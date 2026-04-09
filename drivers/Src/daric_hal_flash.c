/**
 ******************************************************************************
 * @file    daric_hal_flash.c
 * @author  FLASH Team
 * @brief   FLASH HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the FLASH
 *          peripheral:
 *           + Initialization functions
 *           + Data transfer functions (read/write)
 *           + Block erase function
 *
 ******************************************************************************
 * @attention
 *
 * Copyright 2024-2026 CrossBar, Inc.
 * This file has been modified by CrossBar, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "daric_hal_flash.h"
#include "daric_hal_gpio.h"

#ifdef HAL_FLASH_MODULE_ENABLED

SPIM_HandleTypeDef g_flash_spim_handle;

/** @cond Private macros
 * @{
 */
/* Private macros ------------------------------------------------------------*/

#define W25N01GV_FLASH_SIZE 0x10000000 /* 1G Bits == 1024 * 128KBytes */
#define W25N01GV_BLOCK_SIZE 0x40000    /* 1024 blocks of 128KBytes */
#define W25N01GV_PAGE_SIZE  0x800      /* 65536 pages of 2048 bytes */

#define W25Nx_PAGE_Bytes_SIZE  2048
#define W25Nx_SPARE_Bytes_SIZE 64
#define W25Nx_PAGE_TOTAL_SIZE  (W25Nx_PAGE_Bytes_SIZE + W25Nx_SPARE_Bytes_SIZE)
#define W25Nx_BLOCK_SIZE       1024

#define W25N01GV_BLOCK_ERASE_MAX_TIME 80000
#define W25Nx_TIMEOUT_VALUE           1000

#define RESET_ENABLE_CMD  0xFF
#define READ_JEDEC_ID_CMD 0x9F

#define PAGE_READ_CMD 0x13
#define READ_CMD      0x03

#define WRITE_ENABLE_CMD  0x06
#define WRITE_DISABLE_CMD 0x04

#define READ_STATUS_REG_CMD  0x05
#define WRITE_STATUS_REG_CMD 0x01

#define WRITE_DATA_CMD            0x02
#define WRITE_INPUT_PAGE_ADDR_CMD 0x10

#define BLOCK_ERASE_CMD 0xD8

#define Dummy_Byte 0x00

#define Wirte_Fast_Quad_CMD 0x32
#define Read_Fast_Quad_CMD  0x6B

/*Status Register ADDR*/
#define Protection_Reg_Addr    0xA0
#define Configuration_Reg_Addr 0xB0
#define Status_Reg_3           0xC0

#define OTP_ENABLE 0x40
#define OTP_LOCK   0x80

/* Flag Status Register */
#define W25N01GV_FSR_BUSY            ((uint8_t)0x01) /*!< busy */
#define W25N01GV_FSR_WREN            ((uint8_t)0x02) /*!< write enable labtch*/
#define W25N01GV_FSR_Erase_Failure   ((uint8_t)0x04) /*!< Erase_Failure */
#define W25N01GV_FSR_Program_Failure ((uint8_t)0x08) /*!< Program_Failure */
#define W25N01GV_FSR_ECC_Status      ((uint8_t)0x30) /*!< ECC Status */

#ifdef CONFIG_FLASH_SPI_PORT
#define SPI_NSS_W25N_GPIO_Port CONFIG_FLASH_SPI_PORT
#else
#define SPI_NSS_W25N_GPIO_Port GPIOC
#endif

#ifdef CONFIG_FLASH_SPI_CS
#define SPI_NSS_W25N_Pin CONFIG_FLASH_SPI_CS
#else
#define SPI_NSS_W25N_Pin GPIO_PIN_12
#endif

#define W25Nx_SetCS()                                            \
    do                                                           \
    {                                                            \
        GPIO_InitTypeDef GPIO_InitStruct = { 0 };                \
        GPIO_InitStruct.Pin              = SPI_NSS_W25N_Pin;     \
        GPIO_InitStruct.Mode             = GPIO_MODE_OUTPUT;     \
        GPIO_InitStruct.Pull             = GPIO_PULLUP;          \
        GPIO_InitStruct.IsrHandler       = NULL;                 \
        GPIO_InitStruct.UserData         = NULL;                 \
        HAL_GPIO_Init(SPI_NSS_W25N_GPIO_Port, &GPIO_InitStruct); \
    } while (0);

#if 1
#define W25Nx_Enable()    HAL_GPIO_WritePin(SPI_NSS_W25N_GPIO_Port, SPI_NSS_W25N_Pin, GPIO_PIN_RESET)
#define W25Nx_Disable()   HAL_GPIO_WritePin(SPI_NSS_W25N_GPIO_Port, SPI_NSS_W25N_Pin, GPIO_PIN_SET)
#define FLASH_SPI_CS_MODE SPIM_CS_KEEP
#else
#define W25Nx_Enable()
#define W25Nx_Disable()
#define FLASH_SPI_CS_MODE SPIM_CS_AUTO
#endif

#define HAL_FLASH_LOCK   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_FLASH_UNLOCK HAL_UNLOCK

#define FLASH_BB_RECORDER_HEADER 0x1234A5A5 // already scanned flag header
#define FLASH_BB_RECORDER_TAIL   0x5A5A5678 // already scanned flag tail
#define FLASH_BB_RECORDER_BLOCK  (0u)       // actually invalid
#define FLASH_BB_RECORDER_PAGE   (2u)       // bad block scan recorder OTP page

extern void sys_nopDelayUs(uint32_t us);
// #define FLASH_SPI_DELAY HAL_Delay(2)
#define FLASH_SPI_DELAY sys_nopDelayUs(20)
// #define FLASH_SPI_DELAY

struct
{
    uint32_t scan_header;
    uint32_t bad_block_nums;
    uint32_t record_bit_map[W25Nx_BLOCK_SIZE / sizeof(uint32_t)];
    uint32_t scan_tail;
} static s_scan_bad_block_recorder;

uint8_t W25Nx_WaitForDataEnd(void);

/** @endcond
 * @}
 */

// PA[15:0]  10bit block num + 6bit page num
// CA[11:0]  buffer address

// standard SPI MODE
void static W25Nx_SendData(uint8_t *data, uint16_t size)
{
    W25Nx_Enable();
    HAL_SPIM_Send(&g_flash_spim_handle, data, size, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    FLASH_SPI_DELAY;
    W25Nx_Disable();
}

// standard SPI MODE
void static W25Nx_SendWriteData(uint8_t *cmd, uint16_t cmdsize, uint8_t *data, uint16_t datasize)
{
    W25Nx_Enable();
    HAL_SPIM_Send(&g_flash_spim_handle, cmd, cmdsize, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    FLASH_SPI_DELAY;
    HAL_SPIM_Send(&g_flash_spim_handle, data, datasize, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    FLASH_SPI_DELAY;
    W25Nx_Disable();
    W25Nx_WaitForDataEnd();
}
void delaytest()
{
    W25Nx_Enable();
    // HAL_Delay(1);
    extern void sys_nopDelayMs(uint32_t ms);
    sys_nopDelayMs(1);
    W25Nx_Disable();
}
// Half Duplex
void static W25Nx_Send_And_Receive_Data(uint8_t *Txdata, uint16_t Txsize, uint8_t *Rxdata, uint16_t Rxsize)
{
    W25Nx_Enable();
    HAL_SPIM_Send(&g_flash_spim_handle, Txdata, Txsize, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    FLASH_SPI_DELAY;
    HAL_SPIM_Receive(&g_flash_spim_handle, Rxdata, Rxsize, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    W25Nx_Disable();
}

void static W25Nx_Send_And_Receive_Data_QSPI(uint8_t *cmd, uint16_t cmdsize, uint8_t *Rxdata, uint16_t Rxsize)
{
    W25Nx_Enable();
    HAL_SPIM_Send(&g_flash_spim_handle, cmd, cmdsize, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    FLASH_SPI_DELAY;
    HAL_SPIM_Control(&g_flash_spim_handle, SPIM_CTRL_SET_QSPI_ENABLE, 8,
                     0); // enter QSPI MODE
    HAL_SPIM_Receive(&g_flash_spim_handle, Rxdata, Rxsize, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    W25Nx_Disable();
    HAL_SPIM_Control(&g_flash_spim_handle, SPIM_CTRL_SET_QSPI_DISABLE, 8,
                     0); // enter standard MODE
}

/**
 * @brief  This function reset the W25Nx.
 * @retval None
 */
static void W25Nx_Reset(void)
{
    uint8_t cmd = RESET_ENABLE_CMD;

    W25Nx_SendData(&cmd, 1);
    HAL_Delay(2); // tRST = 500us
}

static void W25Nx_READ_ALL_Status(void)
{
    uint8_t cmd1[]    = { READ_STATUS_REG_CMD, Protection_Reg_Addr };
    uint8_t cmd2[]    = { READ_STATUS_REG_CMD, Configuration_Reg_Addr };
    uint8_t cmd3[]    = { READ_STATUS_REG_CMD, Status_Reg_3 };
    uint8_t status[3] = { 0x00, 0x00, 0x00 };

    W25Nx_Send_And_Receive_Data(cmd1, 2, &status[0], 1);
    W25Nx_Send_And_Receive_Data(cmd2, 2, &status[1], 1);
    W25Nx_Send_And_Receive_Data(cmd3, 2, &status[2], 1);
}

/**
 * @brief  Reads current status of the W25Nx.
 * @retval W25N01GV memory status
 */
static uint8_t W25Nx_GetStatus(void)
{
    uint8_t cmd[]  = { READ_STATUS_REG_CMD, Status_Reg_3 };
    uint8_t status = 0xFF;

    // do
    // {
    W25Nx_Send_And_Receive_Data(cmd, 2, &status, 1);
    // }while((status & W25N01GV_FSR_BUSY) != 0);
    /* Check the value of the register */
    if ((status & W25N01GV_FSR_BUSY) != 0)
    {
        return HAL_BUSY;
    }
    else
    {
        return HAL_OK;
    }
}

/**
 * @brief  This function send a Write Enable and wait it is effective.
 * @retval None
 */
static void W25Nx_WriteEnable(void)
{
    uint8_t cmd = WRITE_ENABLE_CMD;

    W25Nx_SendData(&cmd, 1);
}

// /**
//  * @brief  This function send a Write disable and wait it is effective.
//  * @retval None
//  */
// static void W25Nx_WriteDisable(void) {
//   uint8_t cmd = WRITE_DISABLE_CMD;

//   W25Nx_SendData(&cmd, 1);
// }

/*
write Reg,Protection_Reg_Addr，Configuration_Reg_Addr，Status_Reg_3
*/
static void W25Nx_WriteFRS(uint8_t ADDR, uint8_t FRSvalue)
{
    uint8_t cmd[3] = { WRITE_STATUS_REG_CMD, ADDR, FRSvalue };

    W25Nx_SendData(cmd, 3);
}

/*
Read Reg,Protection_Reg_Addr，Configuration_Reg_Addr，Status_Reg_3
*/
static uint8_t W25Nx_Get_FSRStatus(uint8_t Addr)
{
    uint8_t cmd[] = { READ_STATUS_REG_CMD, Addr };
    uint8_t status;

    W25Nx_Send_And_Receive_Data(cmd, 2, &status, 1);
    return status;
}

/*
Scan for factory-defective blocks and record them in the OTP area.
*/
static uint8_t W25Nx_Record_BadBlock(void)
{
    uint8_t  ret    = 0;
    uint8_t *addr   = (uint8_t *)&s_scan_bad_block_recorder;
    uint32_t size   = sizeof(s_scan_bad_block_recorder);
    uint8_t  status = W25Nx_Get_FSRStatus(Configuration_Reg_Addr);

    W25Nx_WriteFRS(Configuration_Reg_Addr, status | OTP_ENABLE);

    if (HAL_FLASH_Read(addr, FLASH_BB_RECORDER_BLOCK, FLASH_BB_RECORDER_PAGE, 0, size) != HAL_OK)
    {
        ret = 1;
    }
    else
    {
        uint8_t data, spare;
        if ((s_scan_bad_block_recorder.scan_header != FLASH_BB_RECORDER_HEADER) || (s_scan_bad_block_recorder.scan_tail != FLASH_BB_RECORDER_TAIL))
        {
            memset(&s_scan_bad_block_recorder, 0, sizeof(s_scan_bad_block_recorder));
            W25Nx_WriteFRS(Configuration_Reg_Addr, status & (~OTP_ENABLE));
            for (uint32_t block = 0; block < W25Nx_BLOCK_SIZE; block++)
            {
                if (HAL_FLASH_Read(&data, block, 0, 0, 1) != HAL_OK || HAL_FLASH_Read(&spare, block, 0, 2048, 1) != HAL_OK)
                {
                    ret = 2;
                    break;
                }
                if ((data & spare) != 0xFF)
                {
                    s_scan_bad_block_recorder.bad_block_nums += 1;
                    s_scan_bad_block_recorder.record_bit_map[block / sizeof(uint32_t)] |= (0x01 << block % sizeof(uint32_t));
                }
            }
            if (ret == 0)
            {
                s_scan_bad_block_recorder.scan_header = FLASH_BB_RECORDER_HEADER;
                s_scan_bad_block_recorder.scan_tail   = FLASH_BB_RECORDER_TAIL;
                W25Nx_WriteFRS(Configuration_Reg_Addr, status | OTP_ENABLE);
                if (HAL_FLASH_Write(addr, FLASH_BB_RECORDER_BLOCK, FLASH_BB_RECORDER_PAGE, 0, size) != HAL_OK)
                {
                    ret = 3;
                }
            }
        }
    }

    W25Nx_WriteFRS(Configuration_Reg_Addr, status & (~OTP_ENABLE));

    return ret;
}

void WB_Enable_Buffer_mode()
{
    uint8_t SR;
    SR = W25Nx_Get_FSRStatus(Configuration_Reg_Addr); // Read status register 2
    SR |= 0x08;                                       // Enable BUF bit
    W25Nx_WriteFRS(Configuration_Reg_Addr, SR);
}

/********************
Function: Disable buffer mode
Argument:
*********************/
void WB_Disable_Buffer_mode()
{
    uint8_t SR;
    SR = W25Nx_Get_FSRStatus(Configuration_Reg_Addr); // Read status register 2
    SR &= 0xF7;                                       // Disable BUF bit
    W25Nx_WriteFRS(Configuration_Reg_Addr, SR);
}

/**
 * @brief  Initializes the FLASH interface.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_FLASH_Init(void)
{
    /* Reset W25Nxxx */
    W25Nx_SetCS();   // Don't use SPIM CS, use GPIO CS
    W25Nx_Disable(); // pull up cs
    HAL_Delay(5);    // give tPUW = 5ms
    /*
     * To avoid the issue of abnormal data transmission caused by manual CS control on SPI,
     * the CLK signal remains low and cannot be changed before CS is pulled low.
     * The CLK level state can only be altered by the first packet of data.
     * Therefore, send the reset command twice.
     */
    W25Nx_Reset(); // every time flash power again,the SR1 value is 0x7C,is a
                   // flash protect status
    W25Nx_Reset();
    // W25Nx_READ_ALL_Status();
    W25Nx_WriteFRS(Protection_Reg_Addr, 0x00); // dis write protect
    WB_Enable_Buffer_mode();                   // W25NxxIG default BUF= 1,mean Buffer read,default
                                               // ECC-E = 1
    // W25Nx_WriteFRS(0xB0 , 0x58);
    W25Nx_READ_ALL_Status(); // fot test
    return W25Nx_GetStatus();
}

/**
 * @brief  Records the FLASH bad block.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_FLASH_BadBlock_Record(void)
{
    HAL_StatusTypeDef ret = HAL_OK;

    HAL_FLASH_LOCK
    if (W25Nx_Record_BadBlock() != 0)
    {
        ret = HAL_ERROR;
    }
    HAL_FLASH_UNLOCK

    return ret;
}

/**
 * @brief  Read FLASH Manufacture/Device ID.
 * @param  return value address
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_FLASH_Read_ID(uint8_t *ID)
{
    uint8_t cmd[2] = { READ_JEDEC_ID_CMD, Dummy_Byte };

    W25Nx_Send_And_Receive_Data(cmd, 2, ID, 3);
    return HAL_OK;
}

/**
 * @brief
 * wait WIP(BUSY) set 0，buf to flash complete
 * @param  none
 * @retval none
 */
uint8_t W25Nx_WaitForDataEnd(void)
{
    uint8_t  FLASH_Status[1] = { 0 };
    uint8_t  cmd[2]          = { READ_STATUS_REG_CMD, Status_Reg_3 };
    uint16_t T_OUT;
    T_OUT = W25Nx_TIMEOUT_VALUE;

    do
    {
        W25Nx_Send_And_Receive_Data(cmd, 2, FLASH_Status, 1);
        {
            if ((T_OUT--) == 0)
            {
                return HAL_TIMEOUT;
            }
        }
    } while ((FLASH_Status[0] & W25N01GV_FSR_BUSY) == 1);

    return HAL_OK;
}

static uint8_t W25Nx_CheckErase_FAIL()
{
    uint8_t cmd[]  = { READ_STATUS_REG_CMD, Status_Reg_3 };
    uint8_t status = 0xFF;

    W25Nx_Send_And_Receive_Data(cmd, 2, &status, 1);
    /* Check the value of the register */
    if ((status & W25N01GV_FSR_Erase_Failure) != 0)
    {
        return HAL_ERROR;
    }
    else
    {
        return HAL_OK;
    }
}

static uint8_t W25Nx_CheckWrite_FAIL()
{
    uint8_t cmd[]  = { READ_STATUS_REG_CMD, Status_Reg_3 };
    uint8_t status = 0xFF;

    W25Nx_Send_And_Receive_Data(cmd, 2, &status, 1);
    /* Check the value of the register */
    if ((status & W25N01GV_FSR_Program_Failure) != 0)
    {
        return HAL_ERROR;
    }
    else
    {
        return HAL_OK;
    }
}

static uint8_t W25Nx_CheckRead_ECC()
{
    uint8_t cmd[]  = { READ_STATUS_REG_CMD, Status_Reg_3 };
    uint8_t status = 0xFF;

    W25Nx_Send_And_Receive_Data(cmd, 2, &status, 1);
    /* Check the value of the register */
    if ((status & W25N01GV_FSR_ECC_Status) == 0x00)
    { // ECC-1:0 = 00,no fault
        return HAL_OK;
    }
    else if ((status & W25N01GV_FSR_ECC_Status) == 0x10)
    { // ECC-1:0 = 01,with 1~4 bit/page ECC corrections
        return HAL_OK;
    }
    else if ((status & W25N01GV_FSR_ECC_Status) == 0x20)
    { // ECC-1:0 = 10,more than 4 bits errors
        return HAL_ERROR;
    }
    else if ((status & W25N01GV_FSR_ECC_Status) == 0x30)
    { // ECC-1:0 = 11
        return HAL_ERROR;
    }
    // record fault page

    return HAL_ERROR;
}

/**
 * @brief  Reads data from a single page (including Spare area).
 * @param  pData: Pointer to data buffer
 * @param  BlockAddr: Block address (0 ~ 1023)
 * @param  PageAddr: Page address (0 ~ 63)
 * @param  BytesAddr: Start byte address (0 ~ 2111, 0x0800~0x083F = Spare area)
 * @param  Size: Size of data to read (must not exceed page boundary)
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_FLASH_Read(uint8_t *pData, uint16_t BlockAddr, uint8_t PageAddr, uint16_t BytesAddr, uint16_t Size)
{
    uint8_t cmd[8];

    // Check if address exceeds page boundary (2112 bytes per page)
    if ((BlockAddr > 1023) || (BytesAddr + Size > W25Nx_PAGE_TOTAL_SIZE))
    {
        return HAL_ERROR;
    }

    cmd[0] = PAGE_READ_CMD; // physical flash to buf
    cmd[1] = Dummy_Byte;
    cmd[2] = (uint8_t)((BlockAddr >> 2) & 0xFF);
    cmd[3] = (uint8_t)(((BlockAddr << 6) | PageAddr) & 0xFF);

    cmd[4] = READ_CMD; // read buf
    cmd[5] = (uint8_t)((BytesAddr >> 8) & 0x0F);
    cmd[6] = (uint8_t)(BytesAddr & 0xFF);
    cmd[7] = Dummy_Byte;

    HAL_FLASH_LOCK

    W25Nx_SendData(cmd, 4);
    W25Nx_WaitForDataEnd();
    if (W25Nx_CheckRead_ECC())
    {
        HAL_FLASH_UNLOCK
        return HAL_ERROR;
    }
    W25Nx_Send_And_Receive_Data(cmd + 4, 4, pData, Size);
    W25Nx_WaitForDataEnd();

    HAL_FLASH_UNLOCK
    return HAL_OK;
}

/**
 * @brief  Writes data to a single page (including Spare area).
 * @param  pData: Pointer to data buffer
 * @param  BlockAddr: Block address (0 ~ 1023)
 * @param  PageAddr: Page address (0 ~ 63)
 * @param  BytesAddr: Start byte address (0 ~ 2111, 0x0800~0x083F = Spare area)
 * @param  Size: Size of data to write (must not exceed page boundary)
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_FLASH_Write(uint8_t *pData, uint16_t BlockAddr, uint8_t PageAddr, uint16_t BytesAddr, uint16_t Size)
{
    uint8_t cmd[7];

    // Check if address exceeds page boundary (2112 bytes per page)
    if ((BlockAddr > 1023) || (BytesAddr + Size > W25Nx_PAGE_TOTAL_SIZE))
    {
        return HAL_ERROR;
    }

    /* Configure the command */
    cmd[0] = WRITE_DATA_CMD;                     // write to temp data buf
    cmd[1] = (uint8_t)((BytesAddr >> 8) & 0x0F); // CA[11:8]
    cmd[2] = (uint8_t)(BytesAddr & 0xFF);        // CA[7:0]

    cmd[3] = WRITE_INPUT_PAGE_ADDR_CMD; // write to physical addr
    cmd[4] = Dummy_Byte;
    cmd[5] = (uint8_t)((BlockAddr >> 2) & 0xFF);              // block addr high
    cmd[6] = (uint8_t)(((BlockAddr << 6) + PageAddr) & 0xFF); // block addr low + page addr

    HAL_FLASH_LOCK

    W25Nx_WriteEnable();
    W25Nx_SendWriteData(cmd, 3, pData, Size); // Send command + address + data
    W25Nx_WriteEnable();
    W25Nx_SendData(cmd + 3, 4);
    W25Nx_WaitForDataEnd();
    if (W25Nx_CheckWrite_FAIL())
    {
        HAL_FLASH_UNLOCK
        return HAL_ERROR;
    }

    HAL_FLASH_UNLOCK
    return HAL_OK;
}

/**
 * @brief  Erases the specified block of the memory.
 * @param  BlockAddress: start Block address to erase,value：0 ~ 1023
           Size: erase block num,if remain block size smaller than Size, erase
 to the end,value:1~1024
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_FLASH_Erase_BLOCK(uint16_t BlockAddr, uint16_t Size)
{
    uint8_t  cmd[4];
    uint16_t i;
    uint32_t tickstart = HAL_GetTick();
    cmd[0]             = BLOCK_ERASE_CMD;
    cmd[1]             = Dummy_Byte;
    cmd[2]             = (uint8_t)((BlockAddr >> 2) & 0xFF); // block addr high
    cmd[3]             = (uint8_t)((BlockAddr << 6) & 0xC0); // block addr low

    if (BlockAddr > 1023)
        return HAL_ERROR;

    HAL_FLASH_LOCK

    // remain blocks > earse block
    if ((W25Nx_BLOCK_SIZE - BlockAddr) >= Size)
    {
        for (i = 0; i < Size; i++)
        {
            while (W25Nx_GetStatus() == HAL_BUSY)
                ; // wait
            W25Nx_WriteEnable();
            // W25Nx_READ_ALL_Status(); //for test
            W25Nx_SendData(cmd, 4); // erase command

            /* Wait the end of Flash Erase */
            while (W25Nx_GetStatus() == HAL_BUSY)
            {
                /* Check for the Timeout */
                if ((HAL_GetTick() - tickstart) > W25N01GV_BLOCK_ERASE_MAX_TIME)
                {
                    HAL_FLASH_UNLOCK
                    return HAL_TIMEOUT;
                }
            }

            if (W25Nx_CheckErase_FAIL())
            {
                HAL_FLASH_UNLOCK
                return HAL_ERROR;
            }

            BlockAddr++;
            cmd[2] = (uint8_t)((BlockAddr >> 2) & 0xFF);
            cmd[3] = (uint8_t)((BlockAddr << 6) & 0xC0);

            tickstart = HAL_GetTick();
        }
    }
    else
    { // earse left blocks
        for (i = 0; i < (W25Nx_BLOCK_SIZE - BlockAddr); i++)
        {
            while (W25Nx_GetStatus() == HAL_BUSY)
                ; // wait

            W25Nx_WriteEnable();
            W25Nx_SendData(cmd, 4); // erase command

            /* Wait the end of Flash Erase */
            while (W25Nx_GetStatus() == HAL_BUSY)
            {
                /* Check for the Timeout */
                if ((HAL_GetTick() - tickstart) > W25N01GV_BLOCK_ERASE_MAX_TIME)
                {
                    HAL_FLASH_UNLOCK
                    return HAL_TIMEOUT;
                }
            }

            if (W25Nx_CheckErase_FAIL())
            {
                HAL_FLASH_UNLOCK
                return HAL_ERROR;
            }

            BlockAddr++;
            cmd[2] = (uint8_t)((BlockAddr >> 2) & 0xFF);
            cmd[3] = (uint8_t)((BlockAddr << 6) & 0xC0);

            tickstart = HAL_GetTick();
        }
    }
    HAL_FLASH_UNLOCK
    return HAL_OK;
}
// QSPI MODE
// /When WP-E bit in the Status Register is set to a 1, all Quad SPI
// instructions are disabled

void static W25Nx_SendData_QSPI(uint8_t *cmd, uint16_t cmdsize, uint8_t *data, uint16_t datasize)
{

    W25Nx_Enable();
    HAL_SPIM_Send(&g_flash_spim_handle, cmd, cmdsize, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    FLASH_SPI_DELAY;
    HAL_SPIM_Control(&g_flash_spim_handle, SPIM_CTRL_SET_QSPI_ENABLE, 8,
                     0); // enter QSPI mode
    HAL_SPIM_Send(&g_flash_spim_handle, data, datasize, FLASH_SPI_CS_MODE, HAL_MAX_DELAY);
    FLASH_SPI_DELAY;
    HAL_SPIM_Control(&g_flash_spim_handle, SPIM_CTRL_SET_QSPI_DISABLE, 8,
                     0); // enter QSPI mode
    W25Nx_Disable();
    W25Nx_WaitForDataEnd();
}

/**
 * @brief  Use QSPI mode to Writes a single page (including Spare area).
 * @param  pData: Pointer to data buffer
 * @param  BlockAddr: Block address (0 ~ 1023)
 * @param  PageAddr: Page address (0 ~ 63)
 * @param  BytesAddr: Start byte address (0 ~ 2111, 0x0800~0x083F = Spare area)
 * @param  Size: Size of data to write (must not exceed page boundary)
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_FLASH_Write_QSPI(uint8_t *pData, uint16_t BlockAddr, uint8_t PageAddr, uint16_t BytesAddr, uint16_t Size)
{

    uint8_t cmd[7];

    // Check if address exceeds page boundary (2112 bytes per page)
    if ((BlockAddr > 1023) || (BytesAddr + Size > W25Nx_PAGE_TOTAL_SIZE))
    {
        return HAL_ERROR;
    }

    /* Configure the command */
    cmd[0] = Wirte_Fast_Quad_CMD;                // write to temp data buf
    cmd[1] = (uint8_t)((BytesAddr >> 8) & 0x0F); // CA[11:8]
    cmd[2] = (uint8_t)(BytesAddr & 0xFF);        // CA[7:0]

    cmd[3] = WRITE_INPUT_PAGE_ADDR_CMD; // write to physical addr
    cmd[4] = Dummy_Byte;
    cmd[5] = (uint8_t)((BlockAddr >> 2) & 0xFF);              // block addr high
    cmd[6] = (uint8_t)(((BlockAddr << 6) + PageAddr) & 0xFF); // block addr low + page addr

    HAL_FLASH_LOCK

    W25Nx_WriteEnable();
    W25Nx_SendData_QSPI(cmd, 3, pData, Size);
    W25Nx_WriteEnable();
    W25Nx_SendData(cmd + 3, 4);
    W25Nx_WaitForDataEnd();
    if (W25Nx_CheckWrite_FAIL())
    {
        HAL_FLASH_UNLOCK
        return HAL_ERROR;
    }

    HAL_FLASH_UNLOCK
    return HAL_OK;
}

/**
 * @brief  Use QSPI mode Read a single page (including Spare area).
 * @param  pData: Pointer to data buffer
 * @param  BlockAddr: Block address (0 ~ 1023)
 * @param  PageAddr: Page address (0 ~ 63)
 * @param  BytesAddr: Start byte address (0 ~ 2111, 0x0800~0x083F = Spare area)
 * @param  Size: Size of data to read (must not exceed page boundary)
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_FLASH_Read_QSPI(uint8_t *pData, uint16_t BlockAddr, uint8_t PageAddr, uint16_t BytesAddr, uint16_t Size)
{
    uint8_t cmd[8];

    // Check if address exceeds page boundary (2112 bytes per page)
    if ((BlockAddr > 1023) || (BytesAddr + Size > W25Nx_PAGE_TOTAL_SIZE))
    {
        return HAL_ERROR;
    }

    cmd[0] = PAGE_READ_CMD; // physical flash to buf
    cmd[1] = Dummy_Byte;
    cmd[2] = (uint8_t)((BlockAddr >> 2) & 0xFF);
    cmd[3] = (uint8_t)(((BlockAddr << 6) | PageAddr) & 0xFF);

    cmd[4] = Read_Fast_Quad_CMD; // read buf
    cmd[5] = (uint8_t)((BytesAddr >> 8) & 0x0F);
    cmd[6] = (uint8_t)(BytesAddr & 0xFF);
    cmd[7] = Dummy_Byte;

    HAL_FLASH_LOCK

    W25Nx_SendData(cmd, 4);
    W25Nx_WaitForDataEnd();
    if (W25Nx_CheckRead_ECC())
    {
        HAL_FLASH_UNLOCK
        return HAL_ERROR;
    }
    W25Nx_Send_And_Receive_Data_QSPI(cmd + 4, 4, pData, Size);
    W25Nx_WaitForDataEnd();

    HAL_FLASH_UNLOCK
    return HAL_OK;
}

#endif /* HAL_FLASH_MODULE_ENABLED */
