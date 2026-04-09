/**
 ******************************************************************************
 * @file    daric_hal_sdio.c
 * @author  SDIO Team
 * @brief   SDIO HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the Secure Digital Input and Output(SDIO)
 *          peripheral:
 *           + Initialization and de-initialization functions
 *           + IO operation functions
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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "daric_hal.h"
#include "daric_hal_nvic.h"
#include "daric_hal_sdio.h"
#include "daric_hal_udma_v3.h"

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
#include "daric_ifram.h"
#endif

/// @cond PRIVATE_OUTPUT
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    __IO uint32_t rx_addr;    /*!< SDIO rx udma transfer address register,    Address
                                 offset: 0x00 */
    __IO uint32_t rx_size;    /*!< SDIO rx udma transfer size register,    Address
                                 offset: 0x04 */
    __IO uint32_t rx_cfg;     /*!< SDIO rx udma transfer config register,    Address
                                 offset: 0x08 */
    __IO uint32_t rx_initcfg; /*!< SDIO rx udma transfer interrupt cfg register,
                                 Address offset: 0x0c */
    __IO uint32_t tx_addr;    /*!< SDIO tx udma transfer address register,    Address
                                 offset: 0x10 */
    __IO uint32_t tx_size;    /*!< SDIO tx udma transfer size register,    Address
                                 offset: 0x14 */
    __IO uint32_t tx_cfg;     /*!< SDIO tx udma transfer config register,    Address
                                 offset: 0x18 */
    __IO uint32_t tx_initcfg; /*!< SDIO tx udma transfer interrupt cfg register,
                                 Address offset: 0x1c */
    __IO uint32_t cmd_op;     /*!< SDIO command operation code register,    Address
                                 offset: 0x20 */
    __IO uint32_t cmd_arg;    /*!< SDIO command operation arg register,    Address
                                 offset: 0x24 */
    __IO uint32_t data_setup; /*!< SDIO data transfer setup register,    Address
                                 offset: 0x28 */
    __IO uint32_t start;      /*!< SDIO data start register,    Address offset: 0x2c */
    __IO uint32_t rsp0;       /*!< SDIO response0 register,    Address offset: 0x30 */
    __IO uint32_t rsp1;       /*!< SDIO response1 register,    Address offset: 0x34 */
    __IO uint32_t rsp2;       /*!< SDIO response2 register,    Address offset: 0x38 */
    __IO uint32_t rsp3;       /*!< SDIO response3 register,    Address offset: 0x3c */
    __IO uint32_t clk_div;    /*!< SDIO clock divide register,    Address offset: 0x40 */
    __IO uint32_t status;     /*!< SDIO status register,    Address offset: 0x44 */
} SDIO_TypeDef;

/* Private define ------------------------------------------------------------*/
#define SDIO_BASE (0x5010D000)
#define SDIO0     (SDIO_TypeDef *)(SDIO_BASE)

#define SDIO_UDMA_CLK_GATE_BIT (12)
#ifndef SDIO_BLOCK_NUM
#define SDIO_BLOCK_NUM (16)
#endif
#ifndef SDIO_BLOCK_SIZE
#define SDIO_BLOCK_SIZE (512)
#endif
#define SDIO_BUF_SIZE (SDIO_BLOCK_NUM * SDIO_BLOCK_SIZE)

/* Private Register Bit Mask Macro -------------------------------------------*/
/* CFG */
#define SDIO_CFG_DATASIZE_POS (1)
#define SDIO_CFG_DATASIZE_1   (0x0 << SDIO_CFG_DATASIZE_POS)
#define SDIO_CFG_DATASIZE_2   (0x1 << SDIO_CFG_DATASIZE_POS)
#define SDIO_CFG_DATASIZE_4   (0x2 << SDIO_CFG_DATASIZE_POS)

#define SDIO_CFG_EN_POS       (4)
#define SDIO_CFG_EN_TRANSFER  (0x1 << SDIO_CFG_EN_POS)
#define SDIO_CFG_DIS_TRANSFER (~(SDIO_CFG_EN_TRANSFER))

#define SDIO_CFG_PENDING_POS (5)
#define SDIO_CFG_PENDING     (0x1 << SDIO_CFG_PENDING_POS)

#define SDIO_CFG_CLEAR_POS (6)
#define SDIO_CFG_CLEAR     (0x1 << SDIO_CFG_CLEAR_POS)

/* CMD_OP - SDIO command operation register */
#define SDIO_CMD_OP_POS  (8)
#define SDIO_CMD_OP_MASK (0x3f)

#define SDIO_CMD_RSP_POS       (0)
#define SDIO_CMD_RSP_MASK      (0x7)
#define SDIO_CMD_RSP_NONE      (0)
#define SDIO_CMD_RSP_48B_CRC   (1)
#define SDIO_CMD_RSP_48B_NOCRC (2)
#define SDIO_CMD_RSP_136B      (3)
#define SDIO_CMD_RSP_48B_BUSY  (4)

/* DATA_SETUP - SDIO Data transfer Setup Register */
#define SDIO_DATA_SETUP_BLOCK_SIZE_POS  (16)
#define SDIO_DATA_SETUP_BLOCK_SIZE_512  (0x1ff << SDIO_DATA_SETUP_BLOCK_SIZE_POS)
#define SDIO_DATA_SETUP_BLOCK_SIZE_1024 (0x3ff << SDIO_DATA_SETUP_BLOCK_SIZE_POS)
#define SDIO_DATA_SETUP_BLOCK_NUM_POS   (8)
#define SDIO_DATA_SETUP_QUAD_POS        (2)
#define SDIO_DATA_SETUP_QUAD_MASK       (0x1 << SDIO_DATA_SETUP_QUAD_POS)
#define SDIO_DATA_SETUP_QUAD_DISABLE    (0x0 << SDIO_DATA_SETUP_QUAD_POS)
#define SDIO_DATA_SETUP_QUAD_ENABLE     (0x1 << SDIO_DATA_SETUP_QUAD_POS)
#define SDIO_DATA_SETUP_RWN_POS         (1)
#define SDIO_DATA_SETUP_RWN_MASK        (0x1 << SDIO_DATA_SETUP_RWN_POS)
#define SDIO_DATA_SETUP_RWN_WRITE       (0x0 << SDIO_DATA_SETUP_RWN_POS)
#define SDIO_DATA_SETUP_RWN_READ        (0x1 << SDIO_DATA_SETUP_RWN_POS)
#define SDIO_DATA_SETUP_EN_POS          (0)
#define SDIO_DATA_SETUP_EN_MASK         (0x1 << SDIO_DATA_SETUP_EN_POS)
#define SDIO_DATA_SETUP_DISABLE         (0x0 << SDIO_DATA_SETUP_EN_POS)
#define SDIO_DATA_SETUP_ENABLE          (0x1 << SDIO_DATA_SETUP_EN_POS)

/* START - SDIO Start register */
#define SDIO_START_DISABLE (0)
#define SDIO_START_ENABLE  (1)

/* CLK_DIV - SDIO Clock Div register */
#define SDIO_CLK_DIV_POS  (0)
#define SDIO_CLK_DIV_MASK (0xff)
#define SDIO_CLK_DIV(div) ((div & SDIO_CLK_DIV_MASK) << SDIO_CLK_DIV_POS)

#define SDIO_CLK_DIV_VALID_POS  (8)
#define SDIO_CLK_DIV_VALID_MASK (0x1)
#define SDIO_CLK_DIV_VALID      (0x1 << SDIO_CLK_DIV_VALID_POS)

/* STATUS - SDIO Status register */
#define SDIO_DATA_ERR_STATUS_POS            (24)
#define SDIO_DATA_ERR_STATUS_MASK           (0x3f << SDIO_DATA_ERR_STATUS_POS)
#define SDIO_DATA_ERR_STATUS_NO_ERR         (0x0 << SDIO_DATA_ERR_STATUS_POS)
#define SDIO_DATA_ERR_STATUS_TIMEOUT        (0x1 << SDIO_DATA_ERR_STATUS_POS)
#define SDIO_CMD_ERR_STATUS_POS             (16)
#define SDIO_CMD_ERR_STATUS_MASK            (0x3f << SDIO_CMD_ERR_STATUS_POS)
#define SDIO_CMD_ERR_STATUS_NO_ERR          (0x0 << SDIO_CMD_ERR_STATUS_POS)
#define SDIO_CMD_ERR_STATUS_TIMEOUT         (0x1 << SDIO_CMD_ERR_STATUS_POS)
#define SDIO_CMD_ERR_STATUS_WRONG_DIRECTION (0x2 << SDIO_CMD_ERR_STATUS_POS)
#define SDIO_CMD_ERR_STATUS_BUSY_TIMEOUT    (0x4 << SDIO_CMD_ERR_STATUS_POS)
#define SDIO_ERROR_STATUS_POS               (1)
#define SDIO_ERROR_STATUS_MASK              (0x1 << SDIO_ERROR_STATUS_POS)
#define SDIO_ERROR_STATUS_NO_ERR            (0x0 << SDIO_ERROR_STATUS_POS)
#define SDIO_ERROR_STATUS_ERROR             (0x1 << SDIO_ERROR_STATUS_POS)
#define SDIO_EOT_STATUS_POS                 (0)
#define SDIO_EOT_STATUS_MASK                (0x1 << SDIO_EOT_STATUS_POS)
#define SDIO_EOT_STATUS_NO_END              (0x0 << SDIO_EOT_STATUS_POS)
#define SDIO_EOT_STATUS_END                 (0x1 << SDIO_EOT_STATUS_POS)

/* Private variables ---------------------------------------------------------*/
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
static uint8_t *sdio_rx_buff = NULL;
static uint8_t *sdio_tx_buff = NULL;
#else
static uint8_t sdio_rx_buff[SDIO_BUF_SIZE] __attribute__((section("ifram"))) __attribute__((aligned(4)));
static uint8_t sdio_tx_buff[SDIO_BUF_SIZE] __attribute__((section("ifram"))) __attribute__((aligned(4)));
#endif
static SDIO_TypeDef *sdio_reg[SDIO_MAX_NUM] = { SDIO0 };

/* Private function prototypes -----------------------------------------------*/
static void SDIO_Setup_Clear(SDIO_TypeDef *SDIOx)
{
    SDIOx->data_setup = 0;
}

static void SDIO_Status_Clear(SDIO_TypeDef *SDIOx)
{
    SDIOx->status = 0xffffffff;
}

static void SDIO_RSP_Clear(SDIO_TypeDef *SDIOx)
{
    SDIOx->rsp0 = 0xffffffff;
    SDIOx->rsp1 = 0xffffffff;
    SDIOx->rsp2 = 0xffffffff;
    SDIOx->rsp3 = 0xffffffff;
}

static bool SDIO_CMD_isFinish(SDIO_TypeDef *SDIOx)
{
    if (SDIOx->status & SDIO_EOT_STATUS_MASK)
    {
        SDIOx->status = SDIO_EOT_STATUS_MASK;
        return true;
    }

    return false;
}

static bool SDIO_CMD_isERR(SDIO_TypeDef *SDIOx)
{
    if ((SDIOx->status & SDIO_ERROR_STATUS_MASK) != 0)
    {
        printf("CMD ERROR: ");
        if ((SDIOx->status & SDIO_CMD_ERR_STATUS_MASK) == SDIO_CMD_ERR_STATUS_TIMEOUT)
        {
            printf("Response Timeout!\n");
        }
        else if ((SDIOx->status & SDIO_CMD_ERR_STATUS_MASK) == SDIO_CMD_ERR_STATUS_WRONG_DIRECTION)
        {
            printf("Wrong Direction!\n");
        }
        else if ((SDIOx->status & SDIO_CMD_ERR_STATUS_MASK) == SDIO_CMD_ERR_STATUS_BUSY_TIMEOUT)
        {
            printf("Busy Timeout!\n");
        }
        else
        {
            printf("Transfer Error!\n");
        }
        SDIOx->status = SDIO_CMD_ERR_STATUS_MASK | SDIO_ERROR_STATUS_MASK;
        return true;
    }

    return false;
}

static bool SDIO_RX_isFinish(SDIO_TypeDef *SDIOx)
{
    if (SDIOx->rx_addr != 0)
    {
        return false;
    }

    if (SDIOx->rx_size != 0)
    {
        return false;
    }

    if ((SDIOx->rx_cfg & SDIO_CFG_EN_TRANSFER) != 0)
    {
        return false;
    }

    return true;
}

static bool SDIO_TX_isFinish(SDIO_TypeDef *SDIOx)
{
    if (SDIOx->tx_addr != 0)
    {
        return false;
    }

    if (SDIOx->tx_size != 0)
    {
        return false;
    }

    if ((SDIOx->tx_cfg & SDIO_CFG_EN_TRANSFER) != 0)
    {
        return false;
    }

    return true;
}

static bool SDIO_Transfer_isERR(SDIO_TypeDef *SDIOx)
{
    if ((SDIOx->status & SDIO_ERROR_STATUS_ERROR) != 0)
    {
        return true;
    }

    if ((SDIOx->status & SDIO_DATA_ERR_STATUS_TIMEOUT) != 0)
    {
        return true;
    }

    return false;
}

static HAL_StatusTypeDef SDIO_WaitCMD(SDIO_HandleTypeDef *hsdio, uint8_t cmd_index, uint32_t timeout);

static uint32_t SDMMC_GetCmdError(SDIO_HandleTypeDef *hsdio);
static uint32_t SDMMC_GetCmdResp1(SDIO_HandleTypeDef *hsdio);
static uint32_t SDMMC_GetCmdResp2(SDIO_HandleTypeDef *hsdio);
static uint32_t SDMMC_GetCmdResp3(SDIO_HandleTypeDef *hsdio);
static uint32_t SDMMC_GetCmdResp6(SDIO_HandleTypeDef *hsdio, uint16_t *pRCA);
static uint32_t SDMMC_GetCmdResp7(SDIO_HandleTypeDef *hsdio);

/// @endcond

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SDIO_Exported_Functions_Group1
 * @brief       Init/Deinit Peripheral Function
 * @{
 */
/**
 * @brief  Initializes the SDIO peripheral.
 *         This function configures the SDIO peripheral, including setting up
 *         the uDMA clock gate, configuring the SDIO clock, and setting the bus
 *         width.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 *                configuration information for the SDIO peripheral.
 * @param  Init:  Structure containing the configuration parameters for the SDIO
 *                peripheral, including clock settings and bus width.
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the initialization is successful,
 *                            HAL_ERROR if any error occurs.
 */
HAL_StatusTypeDef HAL_SDIO_Init(SDIO_HandleTypeDef *hsdio, SDIO_InitTypeDef Init)
{
    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    /* 1. config uDMA clock gate for SDIO */
    HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() | (1 << SDIO_UDMA_CLK_GATE_BIT));

    /* 2. Set SDIO clock divide */
    HAL_SDIO_SetClock(hsdio, Init.Clock);
    HAL_SDIO_SetBusWidth(hsdio, Init.BusWide);

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    sdio_rx_buff = IFRAM_CALLOC(SDIO_BUF_SIZE);
    sdio_tx_buff = IFRAM_CALLOC(SDIO_BUF_SIZE);
    if (sdio_rx_buff == NULL || sdio_tx_buff == NULL)
    {
        IframMgr_Free(sdio_rx_buff);
        IframMgr_Free(sdio_tx_buff);
        printf("%s: malloc ifram fail!\n", __func__);
        return HAL_ERROR;
    }
#endif
    return HAL_OK;
}

/**
 * @brief  De-initializes the SDIO peripheral.
 *         This function disables the SDIO peripheral, clears data transfers,
 *         and resets the necessary configurations to their default state.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the de-initialization is successful,
 *                            HAL_ERROR if any error occurs.
 */
HAL_StatusTypeDef HAL_SDIO_DeInit(SDIO_HandleTypeDef *hsdio)
{
    SDIO_TypeDef *SDIOx = NULL;

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    /* Clear Data Setup for RX and TX */
    SDIO_Setup_Clear(SDIOx);

    /* Clear Start for CMD */
    SDIOx->start = SDIO_START_DISABLE;

    /* Disable SDIO uDMA clock gate */
    HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() & (~(1 << SDIO_UDMA_CLK_GATE_BIT)));

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    IframMgr_Free(sdio_rx_buff);
    IframMgr_Free(sdio_tx_buff);
#endif

    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup ADC_Exported_Functions_Group2
 * @brief       Peripheral Control Functions
 * @{
 */
/**
 * @brief  Sets the bus width for the SDIO peripheral.
 *         This function configures the SDIO bus width by updating the
 * corresponding field in the SDIO handle structure.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  BusWide: The bus width to set. Possible values:
 *                  - SDIO_BUS_WIDE_1B: 1-bit wide bus
 *                  - SDIO_BUS_WIDE_4B: 4-bit wide bus
 *
 * @retval None
 */
void HAL_SDIO_SetBusWidth(SDIO_HandleTypeDef *hsdio, uint32_t BusWide)
{
    hsdio->Init.BusWide = BusWide;
}

/**
 * @brief  Sets the clock frequency for the SDIO peripheral.
 *         This function calculates the clock divider based on the desired clock
 *         frequency and configures the SDIO peripheral's clock.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  Clock: The desired clock frequency. Supported values:
 *                - SDIO_CLK_400K: 400 kHz
 *                - SDIO_CLK_25M: 25 MHz
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the clock is set successfully,
 *                            HAL_ERROR if an error occurs.
 */
HAL_StatusTypeDef HAL_SDIO_SetClock(SDIO_HandleTypeDef *hsdio, uint32_t Clock)
{
    uint32_t      clk_val = 0, clk_div = 0;
    SDIO_TypeDef *SDIOx = NULL;

    /* 1. Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    /* 2. Calculating the clock divider value */
    if (Clock == SDIO_CLK_400K)
    {
        clk_val = 400000;
    }
    else if (Clock == SDIO_CLK_25M)
    {
        clk_val = 25000000;
    }
    else
    {
        printf("%s: Only Support 400K and 25M\n", __func__);
        return HAL_ERROR;
    }

    clk_div = DARIC_CGU->cgufsfreq3 * 1000000 / clk_val / 2;
    if (clk_div == 0)
    {
        printf("%s: Don't Support %lu Frequency\n", __func__, clk_val);
        return HAL_ERROR;
    }

    /* 3. Set clk_div to SDIO CLK_DIV Register */
    SDIOx->clk_div = SDIO_CLK_DIV_VALID | SDIO_CLK_DIV(clk_div);

    /* 4. Save the Clock */
    hsdio->Init.Clock = Clock;

    return HAL_OK;
}

/**
 * @}
 */

/** @defgroup SDIO_Exported_Functions_Group3
 *  @brief    I/O Operation Functions
 * @{
 */
/**
 * @brief  Sets up the SDIO RX for data reception.
 *         This function configures the RX to receive data, sets the appropriate
 *         transfer size, and enables the data reception.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  Size: The size of the data to be received (in bytes).
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the setup is successful,
 *                            HAL_ERROR if an error occurs.
 */
HAL_StatusTypeDef HAL_SDIO_RXFIFO_Setup(SDIO_HandleTypeDef *hsdio, uint32_t Size)
{
    int           ret   = HAL_OK;
    SDIO_TypeDef *SDIOx = NULL;

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    /* 1. Set Transfer Flag */
    hsdio->state = HAL_SDIO_STATE_BUSY_RX;

    hsdio->rx_transfer_size = Size;

    /* 2. Config TX and Enable uDMA */
    SDIOx->rx_addr = (uint32_t)sdio_rx_buff;
    SDIOx->rx_size = Size;
    SDIOx->rx_cfg  = SDIO_CFG_EN_TRANSFER | SDIO_CFG_DATASIZE_4;

    return ret;
}

/**
 * @brief  Reading data after SDIO RX has finished receiving data.
 *         This function reads data from the sdio_rx_buff(ifram) into the
 * provided buffer.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  pReadData: Pointer to the buffer where the received data will be
 * stored.
 * @param  Size: The number of bytes to read from the sdio_rx_buff(ifram).
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the data is read successfully,
 *                            HAL_ERROR if an error occurs.
 */
HAL_StatusTypeDef HAL_SDIO_RXFIFO_Read(SDIO_HandleTypeDef *hsdio, uint32_t *pReadData, uint32_t Size)
{
    uint32_t cp_size = 0;

    if (hsdio == NULL || Size > hsdio->rx_transfer_size)
    {
        return HAL_ERROR;
    }

    if (Size < hsdio->rx_transfer_size)
    {
        cp_size = Size;
    }
    else
    {
        cp_size = hsdio->rx_transfer_size;
    }

    memcpy(pReadData, sdio_rx_buff, cp_size);

    return HAL_OK;
}

/**
 * @brief  Writes data to the SDIO TX for transmission.
 *         This function writes data from a user buffer to the SDIO TX
 *         and sets up the transfer to transmit the data over the SDIO
 * interface.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  pWriteData: Pointer to the buffer containing the data to be
 * transmitted.
 * @param  Size: The size of the data to be transmitted (in bytes).
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the data is written and transfer is set
 * up successfully, HAL_ERROR if an error occurs.
 */
HAL_StatusTypeDef HAL_SDIO_TXFIFO_Write(SDIO_HandleTypeDef *hsdio, uint32_t *pWriteData, uint32_t Size)
{
    int           ret   = HAL_OK;
    SDIO_TypeDef *SDIOx = NULL;

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    /* 1. Set Transfer Flag */
    hsdio->state = HAL_SDIO_STATE_BUSY_TX;

    /* 2. Copy Data to ifram buff */
    memcpy(sdio_tx_buff, pWriteData, Size);

    /* 3. Config TX and Enable uDMA */
    SDIOx->tx_addr = (uint32_t)sdio_tx_buff;
    SDIOx->tx_size = Size;
    SDIOx->tx_cfg  = SDIO_CFG_EN_TRANSFER | SDIO_CFG_DATASIZE_4;

    return ret;
}

/**
 * @brief  Waits for the RX transfer to complete or a timeout to occur.
 *         This function blocks until the RX transfer is finished or a timeout
 * occurs.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  timeout: Timeout duration in milliseconds.
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the transfer is completed successfully,
 *                            HAL_ERROR if an error occurs during the transfer,
 *                            HAL_TIMEOUT if the operation times out.
 */
HAL_StatusTypeDef HAL_SDIO_WaitRX(SDIO_HandleTypeDef *hsdio, uint32_t timeout)
{
    uint32_t      start_time = HAL_GetMs();
    SDIO_TypeDef *SDIOx      = NULL;
    uint8_t       state      = 0;

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    while (1)
    {
        if (SDIO_RX_isFinish(SDIOx) == true)
        {
            state = 0;
            break;
        }

        if (SDIO_Transfer_isERR(SDIOx) == true)
        {
            state = 1;
            break;
        }

        if ((HAL_GetMs() - start_time) > timeout)
        {
            state = 2;
            printf("%s: timeout!\n", __func__);
            break;
        }
    }

    SDIO_Setup_Clear(SDIOx);

    /* Finish Transfer, reset Flag */
    if (hsdio->state == HAL_SDIO_STATE_BUSY_RX)
    {
        hsdio->state = HAL_SDIO_STATE_READY;
    }

    if (state == 0)
    {
        return HAL_OK;
    }
    else if (state == 1)
    {
        return HAL_ERROR;
    }
    else
    {
        return HAL_TIMEOUT;
    }
}

/**
 * @brief  Waits for the TX transfer to complete or a timeout to occur.
 *         This function blocks until the TX transfer is finished or a timeout
 * occurs.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  timeout: Timeout duration in milliseconds.
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the transfer is completed successfully,
 *                            HAL_ERROR if an error occurs during the transfer,
 *                            HAL_TIMEOUT if the operation times out.
 */
HAL_StatusTypeDef HAL_SDIO_WaitTX(SDIO_HandleTypeDef *hsdio, uint32_t timeout)
{
    SDIO_TypeDef *SDIOx      = NULL;
    uint32_t      start_time = HAL_GetMs();

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    while (1)
    {
        if (SDIO_TX_isFinish(SDIOx) == true)
        {
            break;
        }

        if (SDIO_Transfer_isERR(SDIOx) == true)
        {
            return HAL_ERROR;
        }

        if ((HAL_GetMs() - start_time) > timeout)
        {
            printf("%s: timeout!\n", __func__);
            return HAL_TIMEOUT;
        }
    }

    HAL_Delay(1);
    SDIO_Setup_Clear(SDIOx);

    /* Finish Transmit, Reset Flag */
    if (hsdio->state == HAL_SDIO_STATE_BUSY_TX)
    {
        hsdio->state = HAL_SDIO_STATE_READY;
    }

    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup SDIO_Exported_Functions_Group3
 *  @brief    Peripheral Control functions
 * @{
 */

/**
 * @brief  Sends a command to the Card in SDIO peripheral.
 *         This function configures and sends a command to the Card in SDIO
 * peripheral. It waits for the command to be sent successfully or a timeout to
 * occur.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  Command: Pointer to the SDIO_CmdInitTypeDef structure containing the
 * command parameters (e.g., command index, argument, response type).
 *
 * @retval HAL_StatusTypeDef: HAL_OK if the command was sent successfully,
 *                            HAL_ERROR if there was an error in the parameters
 * or during execution.
 */
HAL_StatusTypeDef HAL_SDIO_SendCommand(SDIO_HandleTypeDef *hsdio, SDIO_CmdInitTypeDef *Command)
{
    SDIO_TypeDef     *SDIOx      = NULL;
    HAL_StatusTypeDef ret        = HAL_OK;
    uint32_t          cmd_opcode = 0;

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    /* 1. clear status and rep */
    SDIO_Status_Clear(SDIOx);
    SDIO_RSP_Clear(SDIOx);

    /* 2. Config Command */
    cmd_opcode = ((Command->CmdIndex & SDIO_CMD_OP_MASK) << SDIO_CMD_OP_POS);
    cmd_opcode |= (Command->Response & 0x7);
    SDIOx->cmd_op  = cmd_opcode;
    SDIOx->cmd_arg = Command->Argument;

    /* 3. Send Command */
    SDIOx->start = SDIO_START_ENABLE;

    /* 4. Wait for Command to be sent */
    ret = SDIO_WaitCMD(hsdio, Command->CmdIndex, SDIO_CMDTIMEOUT);
    if (ret != HAL_OK)
    {
        return ret;
    }

    return HAL_OK;
}

/**
 * @brief  Return the command index of last command for which response received
 * @param  SDIOx Pointer to SDMMC register base
 * @retval Command index of the last command response received
 */
// to do, delete later
#if 0
uint8_t HAL_SDIO_GetCommandResponse(SDIO_HandleTypeDef *hsdio)
{
  SDIO_TypeDef *SDIOx = NULL;

  /* Check the parameters */
  if(!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM)) {
    return HAL_ERROR;
  }

  SDIOx = sdio_reg[hsdio->instance_id];

  return (uint8_t)((SDIOx->cmd_op >> SDIO_CMD_OP_POS) & 0x3f);
}
#endif

/**
 * @brief  Return the response received from the card for the last command.
 *         This function reads a specific response register (rsp0, rsp1, rsp2,
 * rsp3) based on the given response index.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  Response: The response index from which the response data is to be
 * retrieved. This parameter can be one of the following values:
 *                      @arg SDIO_RESP1: Response Register 1
 *                      @arg SDIO_RESP2: Response Register 2
 *                      @arg SDIO_RESP3: Response Register 3
 *                      @arg SDIO_RESP4: Response Register 4
 *
 * @retval uint32_t: The value of the specified response register, or HAL_ERROR
 * if there is an error.
 */
uint32_t HAL_SDIO_GetResponse(SDIO_HandleTypeDef *hsdio, uint32_t Response)
{
    __IO uint32_t tmp   = 0U;
    SDIO_TypeDef *SDIOx = NULL;

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    /* Get the response */
    tmp = (uint32_t) & (SDIOx->rsp0) + Response;

    return (*(__IO uint32_t *)tmp);
}

/**
 * @brief  Configures the SDIO data transfer settings, including block size,
 *         block count, bus width, and transfer direction.
 *         This function prepares the SDIO peripheral to transfer data according
 *         to the specified configuration.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 * @param  Data: Pointer to the SDIO_DataInitTypeDef structure that contains
 *               the data transfer configuration parameters.
 *
 * @retval HAL_StatusTypeDef: Status of the configuration (HAL_OK or HAL_ERROR).
 */
HAL_StatusTypeDef HAL_SDIO_ConfigData(SDIO_HandleTypeDef *hsdio, SDIO_DataInitTypeDef *Data)
{
    uint32_t      tmpreg    = 0U;
    uint32_t      block_num = 0;
    SDIO_TypeDef *SDIOx     = NULL;

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    SDIO_Setup_Clear(SDIOx);

    /* 1. Set Block Size*/
    if (Data->DataBlockSize > 1024)
    {
        return HAL_ERROR;
    }
    else
    {
        tmpreg |= (Data->DataBlockSize - 1) << SDIO_DATA_SETUP_BLOCK_SIZE_POS;
    }

    /* 2. Set Block Num */
    block_num = Data->DataLength / Data->DataBlockSize;
    if (block_num == 0)
    {
        return HAL_ERROR;
    }
    tmpreg |= (block_num - 1) << SDIO_DATA_SETUP_BLOCK_NUM_POS;

    /* 3. Set BusWide */
    if (hsdio->Init.BusWide == SDIO_BUS_WIDE_1B)
    {
        tmpreg |= (SDIO_DATA_SETUP_QUAD_DISABLE);
    }
    else if (hsdio->Init.BusWide == SDIO_BUS_WIDE_4B)
    {
        tmpreg |= (SDIO_DATA_SETUP_QUAD_ENABLE);
    }
    else
    {
        return HAL_ERROR;
    }

    /* 4. Set Transfer Direction */
    if (Data->TransferDir == SDIO_TRANSFER_DIR_TO_CARD)
    {
        tmpreg |= SDIO_DATA_SETUP_RWN_WRITE;
    }
    else
    {
        tmpreg |= SDIO_DATA_SETUP_RWN_READ;
    }

    /* 5. Enable Transfer for Current CMD */
    tmpreg |= SDIO_DATA_SETUP_ENABLE;

    SDIOx->data_setup = tmpreg;

    return HAL_OK;
}

/**
 * @brief  Clear the SDIO Data Configuration, When tx or rx transfer is
 * complete.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the
 * configuration information for the SDIO peripheral.
 *
 * @retval HAL_StatusTypeDef: Status of the configuration (HAL_OK or HAL_ERROR).
 */
HAL_StatusTypeDef HAL_SDIO_Setup_Clear(SDIO_HandleTypeDef *hsdio)
{
    SDIO_TypeDef *SDIOx = NULL;

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    SDIO_Setup_Clear(SDIOx);

    return HAL_OK;
}

/**
 * @}
 */

/** @defgroup SDIO_Exported_Functions_Group4
 *  @brief    Command management functions
 * @{
 */

/**
 * @brief  Configures the block size of the SD card by sending the CMD16 command
 *         to set the block length.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  BlockSize: The block size to set for the SD card.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdBlockLength(SDIO_HandleTypeDef *hsdio, uint32_t BlockSize)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Set Block Size for Card: CMD16 */
    sdmmc_cmdinit.Argument = (uint32_t)BlockSize;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SET_BLOCKLEN;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD17 command to read a single block of data from the SD card.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  ReadAdd: The address to read the single block from on the SD card.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdReadSingleBlock(SDIO_HandleTypeDef *hsdio, uint32_t ReadAdd)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Read Single Block from Card: CMD17 */
    sdmmc_cmdinit.Argument = (uint32_t)ReadAdd;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_READ_SINGLE_BLOCK;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD18 command to read multiple blocks of data from the SD card.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  ReadAdd: The address to start reading multiple blocks from on the SD
 * card.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdReadMultiBlock(SDIO_HandleTypeDef *hsdio, uint32_t ReadAdd)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Read Multi Block from Card: CMD18 */
    sdmmc_cmdinit.Argument = (uint32_t)ReadAdd;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_READ_MULT_BLOCK;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD24 command to write a single block of data to the SD card.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  WriteAdd: The address to write a single block of data to on the SD
 * card.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdWriteSingleBlock(SDIO_HandleTypeDef *hsdio, uint32_t WriteAdd)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Write Single Block to Card: CMD24 */
    sdmmc_cmdinit.Argument = (uint32_t)WriteAdd;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_WRITE_SINGLE_BLOCK;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD25 command to write multiple blocks of data to the SD card.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  WriteAdd: The starting address to write multiple blocks of data to on
 * the SD card.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdWriteMultiBlock(SDIO_HandleTypeDef *hsdio, uint32_t WriteAdd)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Write Multi Block to Card: CMD25 */
    sdmmc_cmdinit.Argument = (uint32_t)WriteAdd;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_WRITE_MULT_BLOCK;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD32 command to set the start address for the SD card erase
 * operation.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  StartAdd: The starting address for the erase operation on the SD
 * card.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSDEraseStartAdd(SDIO_HandleTypeDef *hsdio, uint32_t StartAdd)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Set Erase Start Address: CMD32 */
    sdmmc_cmdinit.Argument = (uint32_t)StartAdd;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SD_ERASE_GRP_START;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD33 command to set the End address for the SD card erase
 * operation.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  EndAdd: The Ending address for the erase operation on the SD card.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSDEraseEndAdd(SDIO_HandleTypeDef *hsdio, uint32_t EndAdd)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Set Erase End Address: CMD33 */
    sdmmc_cmdinit.Argument = (uint32_t)EndAdd;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SD_ERASE_GRP_END;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD38 command to start the SD card erase operation.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSDErase(SDIO_HandleTypeDef *hsdio)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Start to Erase selected Block: CMD38 */
    sdmmc_cmdinit.Argument = 0U;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_ERASE;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_BUSY;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD12 command to Stop the transfer with SD card.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdStopTransfer(SDIO_HandleTypeDef *hsdio)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send CMD12 STOP_TRANSMISSION  */
    sdmmc_cmdinit.Argument = 0U;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_STOP_TRANSMISSION;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_BUSY;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD7 command to Select or Deselect the SD card.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSelDesel(SDIO_HandleTypeDef *hsdio, uint64_t Addr)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send CMD7 SDMMC_SEL_DESEL_CARD */
    sdmmc_cmdinit.Argument = (uint32_t)Addr;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SEL_DESEL_CARD;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_BUSY;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD0 command to reset the SD card to idle state.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdGoIdleState(SDIO_HandleTypeDef *hsdio)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit = { 0 };
    uint32_t            errorstate    = SDMMC_ERROR_NONE;

    /* Send Go Idle State Command for All Card: CMD0 */
    sdmmc_cmdinit.Argument = 0U;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_GO_IDLE_STATE;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_NONE;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* CMD0 Don't Need to Check Response */
    errorstate = SDMMC_GetCmdError(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD8 command to check the operating conditions of an SD card.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdOperCond(SDIO_HandleTypeDef *hsdio)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit = { 0 };
    uint32_t            errorstate    = SDMMC_ERROR_NONE;

    /* Send CMD8 to verify SD card interface operating condition */
    /* Argument:
    - [31:12]: Reserved (shall be set to '0')
    - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
    - [7:0]: Check Pattern (recommended 0xAA) */
    sdmmc_cmdinit.Argument = SDMMC_CHECK_PATTERN;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_HS_SEND_EXT_CSD;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp7(hsdio);
    /* If there is a HAL_ERROR, it is a MMC card or SD card 1.x,
       else it is a SD card 2.0 */

    return errorstate;
}

/**
 * @brief  Sends CMD55(Application Command) to the SD card to select the
 * application-specific command set.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  Argument: The argument to be sent with the CMD55 command.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdAppCommand(SDIO_HandleTypeDef *hsdio, uint32_t Argument)
{

    uint32_t errorstate = SDMMC_ERROR_NONE;

    SDIO_CmdInitTypeDef sdmmc_cmdinit = { 0 };
    /* Send APP CMD to verify the command is an application specific CMD: CMD55*/
    sdmmc_cmdinit.Argument = (uint32_t)Argument;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_APP_CMD;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Send the ACMD41(SD application operation command) to the SD card.
 *         This function sends the ACMD41 command to the SD card,
 *         which is used for initializing the SD card and setting
 *         the operating conditions.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  SdType: The SD card type to be initialized (e.g., SDHC, SDXC).
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdAppOperCommand(SDIO_HandleTypeDef *hsdio, uint32_t SdType)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send ACMD41 */
    sdmmc_cmdinit.Argument = SDMMC_VOLTAGE_RECOMMEND | SdType;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SD_APP_OP_COND;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC; // to do, theoretically set to NOCRC
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    errorstate = SDMMC_GetCmdResp3(hsdio);

    return errorstate;
}

/**
 * @brief  Send the ACMD6 to Configures the SD card bus width.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  BusWidth: The desired bus width to configure (0 for 1-bit, 2 for
 * 4-bit).
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdBusWidth(SDIO_HandleTypeDef *hsdio, uint32_t BusWidth)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send the Bus Width command: ACMD6 */
    sdmmc_cmdinit.Argument = (uint32_t)BusWidth;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_APP_SD_SET_BUSWIDTH;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends the CMD51 command to retrieve the SD card's SCR (SD
 * Configuration Register).
 *
 * This function sends the CMD51 (SD_APP_SEND_SCR) command to the SD card to
 * obtain its SCR register. The SCR contains information about the card's
 * capabilities, such as its supported voltage ranges and other features.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSendSCR(SDIO_HandleTypeDef *hsdio)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send CMD51 SD_APP_SEND_SCR */
    sdmmc_cmdinit.Argument = 0U;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SD_APP_SEND_SCR;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends the CMD2 command to retrieve the Card Identification (CID)
 * register from the SD card.
 *
 * This function sends the CMD2 (ALL_SEND_CID) command to the SD card to
 * retrieve its CID register. The CID register contains information that
 * uniquely identifies the card, such as manufacturer information, product name,
 * and card serial number.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSendCID(SDIO_HandleTypeDef *hsdio)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send CMD2 ALL_SEND_CID */
    sdmmc_cmdinit.Argument = 0U;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_ALL_SEND_CID;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_136B;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp2(hsdio);

    return errorstate;
}

/**
 * @brief  Sends the CMD9 command to retrieve the Card Specific Data (CSD)
 * register from the SD card.
 *
 * This function sends the CMD9 (SEND_CSD) command to the SD card to retrieve
 * its CSD register. The CSD register contains information that defines the
 * card's capacity, access conditions, and specific parameters, such as the data
 * bus width, maximum clock frequency, and card capacity.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  Argument: The argument to be passed with the CMD9 command, usually
 * the card's relative address (RCA) to identify the card.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSendCSD(SDIO_HandleTypeDef *hsdio, uint32_t Argument)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send CMD9 SEND_CSD */
    sdmmc_cmdinit.Argument = (uint32_t)Argument;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SEND_CSD;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_136B;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp2(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD3 (SD_CMD_SET_REL_ADDR) to set the Relative Card Address
 * (RCA) for the SD card.
 *
 * This function sends CMD3 to the SD card to assign a relative address (RCA) to
 * the card. The card is selected using the RCA, and subsequent commands refer
 * to the card using this address.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  pRCA: Pointer to a 16-bit variable where the relative card address
 * (RCA) will be stored.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSetRelAdd(SDIO_HandleTypeDef *hsdio, uint16_t *pRCA)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send CMD3 SD_CMD_SET_REL_ADDR */
    sdmmc_cmdinit.Argument = 0U;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SET_REL_ADDR;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp6(hsdio, pRCA);

    return errorstate;
}

/**
 * @brief  Sends CMD13 to retrieve the Card Status Register (CSR) of the SD
 * card.
 *
 * This function sends CMD13 to the SD card to read its status register (CSR),
 * which contains information about the card's current state (e.g., whether the
 * card is ready, busy, etc.).
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  Argument: The argument to be passed to CMD13, typically the relative
 * card address (RCA).
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSendStatus(SDIO_HandleTypeDef *hsdio, uint32_t Argument)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send CMD13 to Get Card Status Register */
    sdmmc_cmdinit.Argument = (uint32_t)Argument;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SEND_STATUS;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends ACMD13 to retrieve the SD card status.
 *
 * This function sends ACMD13 to the SD card to read its status, which provides
 * information about the card's current state and conditions.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdStatusRegister(SDIO_HandleTypeDef *hsdio)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send ACMD13 to Get SD Status */
    sdmmc_cmdinit.Argument = 0U;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_SD_APP_STATUS;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC;
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @brief  Sends CMD6 to switch the function of the SD card (e.g., to change
 * mode or configuration).
 *
 * This function sends CMD6 (Switch Command) to the SD card to switch the card's
 * function, which is used for operations like switching between different modes
 * or configurations.
 *
 * @param  hsdio: Pointer to the SDIO handle structure that contains the
 * configuration information for the SDIO peripheral.
 * @param  Argument: Argument to specify the switch function parameters.
 *
 * @retval uint32_t: The status of the operation (HAL_OK on success, error code
 * on failure).
 */
uint32_t HAL_SDMMC_CmdSwitch(SDIO_HandleTypeDef *hsdio, uint32_t Argument)
{
    SDIO_CmdInitTypeDef sdmmc_cmdinit;
    uint32_t            errorstate = SDMMC_ERROR_NONE;

    /* Send CMD6 to Switch Function */
    sdmmc_cmdinit.Argument = Argument;
    sdmmc_cmdinit.CmdIndex = SDMMC_CMD_HS_SWITCH;
    sdmmc_cmdinit.Response = SDIO_CMD_RSP_48B_CRC; // Not Sure CMD6 RSP
    errorstate             = HAL_SDIO_SendCommand(hsdio, &sdmmc_cmdinit);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Check for error conditions */
    errorstate = SDMMC_GetCmdResp1(hsdio);

    return errorstate;
}

/**
 * @}
 */

/* Private function ----------------------------------------------------------*/
/** @addtogroup SDIO_and_SD_Private_Functions
 * @{
 */

/**
 * @brief  Waits for the SDIO command to finish or timeout.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the SDIO
 * configuration.
 * @param  timeout: Timeout value in milliseconds. The function will return an
 * error if the command doesn't complete within this time frame.
 *
 * @retval HAL_OK if the command completes successfully,
 *         HAL_ERROR if an error occurs during command execution,
 *         HAL_TIMEOUT if the command times out.
 */
static HAL_StatusTypeDef SDIO_WaitCMD(SDIO_HandleTypeDef *hsdio, uint8_t cmd_index, uint32_t timeout)
{
    SDIO_TypeDef *SDIOx      = NULL;
    uint32_t      start_time = HAL_GetMs();

    /* Check the parameters */
    if (!hsdio || !(hsdio->instance_id < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    SDIOx = sdio_reg[hsdio->instance_id];

    while (1)
    {
        if (SDIO_CMD_isFinish(SDIOx))
        {
            break;
        }

        if (SDIO_CMD_isERR(SDIOx))
        {
            printf("command %d is error!\n", cmd_index);
            return HAL_ERROR;
        }

        if ((HAL_GetMs() - start_time) > timeout)
        {
            printf("%s: command %d timeout!\n", __func__, cmd_index);
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

/**
 * @brief  This function return SDMMC_ERROR_NONE for CMD0.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the SDIO
 * configuration.
 *
 * @retval uint32_t: SDMMC_ERROR_NONE
 */
static uint32_t SDMMC_GetCmdError(SDIO_HandleTypeDef *hsdio)
{
    return SDMMC_ERROR_NONE;
}

/**
 * @brief  This function processes the response R1 from the SDIO command.
 *
 * It checks the error flags in the response and returns the corresponding error
 * code. If no error is found, it returns `SDMMC_ERROR_NONE`.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the SDIO
 * configuration.
 *
 * @retval uint32_t: SD Card error state
 */
static uint32_t SDMMC_GetCmdResp1(SDIO_HandleTypeDef *hsdio)
{
    uint32_t response_r1;

    /* We have received response, retrieve it for analysis  */
    response_r1 = HAL_SDIO_GetResponse(hsdio, SDIO_RESP1);
    if ((response_r1 & SDMMC_OCR_ERRORBITS) == SDMMC_ALLZERO)
    {
        return SDMMC_ERROR_NONE;
    }
    else if ((response_r1 & SDMMC_OCR_ADDR_OUT_OF_RANGE) == SDMMC_OCR_ADDR_OUT_OF_RANGE)
    {
        return SDMMC_ERROR_ADDR_OUT_OF_RANGE;
    }
    else if ((response_r1 & SDMMC_OCR_ADDR_MISALIGNED) == SDMMC_OCR_ADDR_MISALIGNED)
    {
        return SDMMC_ERROR_ADDR_MISALIGNED;
    }
    else if ((response_r1 & SDMMC_OCR_BLOCK_LEN_ERR) == SDMMC_OCR_BLOCK_LEN_ERR)
    {
        return SDMMC_ERROR_BLOCK_LEN_ERR;
    }
    else if ((response_r1 & SDMMC_OCR_ERASE_SEQ_ERR) == SDMMC_OCR_ERASE_SEQ_ERR)
    {
        return SDMMC_ERROR_ERASE_SEQ_ERR;
    }
    else if ((response_r1 & SDMMC_OCR_BAD_ERASE_PARAM) == SDMMC_OCR_BAD_ERASE_PARAM)
    {
        return SDMMC_ERROR_BAD_ERASE_PARAM;
    }
    else if ((response_r1 & SDMMC_OCR_WRITE_PROT_VIOLATION) == SDMMC_OCR_WRITE_PROT_VIOLATION)
    {
        return SDMMC_ERROR_WRITE_PROT_VIOLATION;
    }
    else if ((response_r1 & SDMMC_OCR_LOCK_UNLOCK_FAILED) == SDMMC_OCR_LOCK_UNLOCK_FAILED)
    {
        return SDMMC_ERROR_LOCK_UNLOCK_FAILED;
    }
    else if ((response_r1 & SDMMC_OCR_COM_CRC_FAILED) == SDMMC_OCR_COM_CRC_FAILED)
    {
        return SDMMC_ERROR_COM_CRC_FAILED;
    }
    else if ((response_r1 & SDMMC_OCR_ILLEGAL_CMD) == SDMMC_OCR_ILLEGAL_CMD)
    {
        return SDMMC_ERROR_ILLEGAL_CMD;
    }
    else if ((response_r1 & SDMMC_OCR_CARD_ECC_FAILED) == SDMMC_OCR_CARD_ECC_FAILED)
    {
        return SDMMC_ERROR_CARD_ECC_FAILED;
    }
    else if ((response_r1 & SDMMC_OCR_CC_ERROR) == SDMMC_OCR_CC_ERROR)
    {
        return SDMMC_ERROR_CC_ERR;
    }
    else if ((response_r1 & SDMMC_OCR_STREAM_READ_UNDERRUN) == SDMMC_OCR_STREAM_READ_UNDERRUN)
    {
        return SDMMC_ERROR_STREAM_READ_UNDERRUN;
    }
    else if ((response_r1 & SDMMC_OCR_STREAM_WRITE_OVERRUN) == SDMMC_OCR_STREAM_WRITE_OVERRUN)
    {
        return SDMMC_ERROR_STREAM_WRITE_OVERRUN;
    }
    else if ((response_r1 & SDMMC_OCR_CID_CSD_OVERWRITE) == SDMMC_OCR_CID_CSD_OVERWRITE)
    {
        return SDMMC_ERROR_CID_CSD_OVERWRITE;
    }
    else if ((response_r1 & SDMMC_OCR_WP_ERASE_SKIP) == SDMMC_OCR_WP_ERASE_SKIP)
    {
        return SDMMC_ERROR_WP_ERASE_SKIP;
    }
    else if ((response_r1 & SDMMC_OCR_CARD_ECC_DISABLED) == SDMMC_OCR_CARD_ECC_DISABLED)
    {
        return SDMMC_ERROR_CARD_ECC_DISABLED;
    }
    else if ((response_r1 & SDMMC_OCR_ERASE_RESET) == SDMMC_OCR_ERASE_RESET)
    {
        return SDMMC_ERROR_ERASE_RESET;
    }
    else if ((response_r1 & SDMMC_OCR_AKE_SEQ_ERROR) == SDMMC_OCR_AKE_SEQ_ERROR)
    {
        return SDMMC_ERROR_AKE_SEQ_ERR;
    }
    else
    {
        return SDMMC_ERROR_GENERAL_UNKNOWN_ERR;
    }
}

/**
 * @brief  Checks for error conditions for R2 (CID or CSD) response.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the SDIO
 * configuration.
 *
 * @retval uint32_t: SDMMC_ERROR_NONE
 */
static uint32_t SDMMC_GetCmdResp2(SDIO_HandleTypeDef *hsdio)
{
    return SDMMC_ERROR_NONE;
}

/**
 * @brief  Checks for error conditions for R3 (OCR) response.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the SDIO
 * configuration.
 *
 * @retval uint32_t: SDMMC_ERROR_NONE
 */
static uint32_t SDMMC_GetCmdResp3(SDIO_HandleTypeDef *hsdio)
{
    return SDMMC_ERROR_NONE;
}

/**
 * @brief  This function processes the response R6 from the SDIO command.
 *
 * It checks the error flags in the response and returns the corresponding error
 * code. If no error is found, it extracts the Relative Card Address (RCA) and
 * returns `SDMMC_ERROR_NONE`.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the SDIO
 * configuration.
 * @param  pRCA: Pointer to a variable where the Relative Card Address (RCA)
 * will be stored if no error is found.
 *
 * @retval uint32_t: SD Card error state.
 */
static uint32_t SDMMC_GetCmdResp6(SDIO_HandleTypeDef *hsdio, uint16_t *pRCA)
{
    uint32_t response_r1;

    /* We have received response, retrieve it. */
    response_r1 = HAL_SDIO_GetResponse(hsdio, SDIO_RESP1);

    if ((response_r1 & (SDMMC_R6_GENERAL_UNKNOWN_ERROR | SDMMC_R6_ILLEGAL_CMD | SDMMC_R6_COM_CRC_FAILED)) == SDMMC_ALLZERO)
    {
        *pRCA = (uint16_t)(response_r1 >> 16);

        return SDMMC_ERROR_NONE;
    }
    else if ((response_r1 & SDMMC_R6_ILLEGAL_CMD) == SDMMC_R6_ILLEGAL_CMD)
    {
        return SDMMC_ERROR_ILLEGAL_CMD;
    }
    else if ((response_r1 & SDMMC_R6_COM_CRC_FAILED) == SDMMC_R6_COM_CRC_FAILED)
    {
        return SDMMC_ERROR_COM_CRC_FAILED;
    }
    else
    {
        return SDMMC_ERROR_GENERAL_UNKNOWN_ERR;
    }
}

/**
 * @brief  This function processes the response R7 from the SDIO command.
 *
 * It checks if the response matches the expected pattern (SDMMC_CHECK_PATTERN).
 * If the response does not match, it returns an error indicating an illegal
 * command.
 *
 * @param  hsdio: Pointer to the SDIO handle structure containing the SDIO
 * configuration.
 *
 * @retval uint32_t: SD Card error state.
 */
static uint32_t SDMMC_GetCmdResp7(SDIO_HandleTypeDef *hsdio)
{
    uint32_t response_r1;

    /* We have received response, retrieve it for analysis  */
    response_r1 = HAL_SDIO_GetResponse(hsdio, SDIO_RESP1);
    if (response_r1 != SDMMC_CHECK_PATTERN)
    {
        return SDMMC_ERROR_ILLEGAL_CMD;
    }

    return SDMMC_ERROR_NONE;
}
