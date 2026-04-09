/**
 ******************************************************************************
 * @file    daric_hal_uart.c
 * @author  UART Team
 * @brief   UART HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the Universal Asynchronous Receiver Transmitter
 *          Peripheral (UART).
 *           + Initialization and de-initialization functions
 *           + IO operation functions
 *           + Peripheral Control functions
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

#include <string.h>

#include "daric_hal.h"
#include "daric_hal_def.h"
#include "daric_hal_nvic.h"
#include "daric_hal_uart.h"
#include "daric_hal_udma_v3.h"
#include "daric_pulp_io.h"

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
#include "daric_ifram.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED

/* UART IRQ Priority define */
#ifdef HAL_UART_IRQ_PRIO
#define UART_IRQ_PRIO HAL_UART_IRQ_PRIO
#else
#define UART_IRQ_PRIO 0
#endif

#ifdef HAL_UART_IRQ_SUB_PRIO
#define UART_IRQ_SUB_PRIO HAL_UART_IRQ_SUB_PRIO
#else
#define UART_IRQ_SUB_PRIO 0
#endif

#define reg_write32 pulp_write32
#define reg_read32  pulp_read32

#define UART_DEFAULT_BAUDRATE 115200
#define UART_TX_BUFFER_SIZE   (500)
#define UART_RX_BUFFER_SIZE   (200)
/* The ifram cannot be too small, otherwise,
 * unread data may be overwritten when using a high baudrate.
 */
#define UART_RX_EX_BUFFER_SIZE (2048)

/* DMA RX channel receives data full */
#define UART_RX_FULL_EVT(id) (UART0_0_IRQn + id * 4)
/* DMA TX channel finish sending data */
#define UART_TX_FINISH_EVT(id) (UART0_1_IRQn + id * 4)
/* RX polling event */
#define UART_RX_POLL_EVT(id) (UART0_2_IRQn + id * 4)
/* Error event */
#define UART_ERROR_EVT(id) (UART0_3_IRQn + id * 4)

#define HAL_UART_LOCK()   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_UART_UNLOCK() HAL_UNLOCK

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
static uint8_t *uart_rx_buf[UART_ID_MAX]    = { 0 };
static uint8_t *uart_tx_buf[UART_ID_MAX]    = { 0 };
static uint8_t *uart_rx_ex_buf[UART_ID_MAX] = { 0 };
#else
static uint8_t uart_rx_buf[UART_ID_MAX][UART_RX_BUFFER_SIZE] __attribute__((section("ifram")));
static uint8_t uart_tx_buf[UART_ID_MAX][UART_TX_BUFFER_SIZE] __attribute__((section("ifram")));
#endif

static inline int UART_IsInited(UART_HandleTypeDef *huart)
{
    return huart->state & HAL_UART_INITED;
}

/**
 * Setup UART. The UART defaults to 8 bit character mode with 1 stop bit.
 *
 * parity       Enable/disable parity mode
 * clk_counter  Clock counter value that is used to derive the UART clock.
 *              There is a prescaler in place that already divides the SoC
 *              clock by 16.  Since this is a counter, a value of 0 means that
 *              the SoC clock divided by 16 is used. A value of 31 would mean
 *              that we use the SoC clock divided by 16*32 = 512.
 */
static void UART_SetUp(UART_HandleTypeDef *huart)
{
    UART_InitTypeDef *Init        = NULL;
    uint32_t          clk_counter = 0;
    uint32_t          val         = 0;

    if (huart == NULL)
        return;

    Init = &(huart->init);

// [31:16]: clock divider (from SoC clock)
// [9]: RX enable
// [8]: TX enable
// [3]: stop bits  0 = 1 stop bit
//                 1 = 2 stop bits
// [2:1]: bits     00 = 5 bits
//                 01 = 6 bits
//                 10 = 7 bits
//                 11 = 8 bits
// [0]: parity
#if defined(CONFIG_SOC_DARIC_NTO_A)
    double perClkHz = 0;
    perClkHz        = HAL_GetPerClkHz();
    /* Round-up the divider */
    clk_counter = (perClkHz / Init->BaudRate) - 1;
#else
    clk_counter = DARIC_CGU->cgufsfreq3 * 1000000 / Init->BaudRate;
#endif

    // val = 0x0306 | Init->Parity; // both tx and rx enabled; 8N1 configuration;
    // 1 stop bits
    val = (Init->Tx_En << 8) | (Init->Rx_En << 9) | (Init->Clean_Rx_Fifo << 5) | (Init->Poll_En << 4) | (Init->StopBits << 3) | (Init->WordLength << 1) | (Init->Parity);

    val |= ((clk_counter) << 16);

    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_SETUP_OFFSET, val);
}

static void UART_CLKGateCfg(int uart_id)
{
    int reg_val = HAL_UDMA_CG_Get();

    reg_val |= (1 << uart_id);
    HAL_UDMA_CG_Set(reg_val);
}

// STATUS Bit 1 RX_BUSY / Bit 0 TX_BUSY
static void UART_RXFullIRQHander(UART_HandleTypeDef *huart)
{
    uint32_t base = 0;
    uint32_t cfg  = 0;
    uint8_t *buf  = uart_rx_buf[huart->id];

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (!UART_IsInited(huart)) || (huart->pRxBuffPtr == NULL) || (huart->Rx_mode == 0))
    {
        return;
    }

    for (int i = 0; i < huart->RxXferCount; i++)
    {
        huart->pRxBuffPtr[huart->RxBuffIndex + i] = buf[i];
    }

    if (huart->RxXferSize)
    {
        base = UDMA_BASE_ADDR + UDMA_PERIPH_OFFSET(huart->id) + UDMA_CHANNEL_RX_OFFSET;

        if (huart->RxXferSize > UART_RX_BUFFER_SIZE)
        {
            huart->RxXferCount = UART_RX_BUFFER_SIZE;
        }
        else
        {
            huart->RxXferCount = huart->RxXferSize;
        }
        if (huart->RxContinuous)
        {
            cfg |= (1 << UART_RX_CONTINOUS_OFFSET);
        }
        huart->RxXferSize -= huart->RxXferCount;
        huart->RxBuffIndex += huart->RxXferCount;
        HAL_UDMA_Enqueue(base, (unsigned int)buf, huart->RxXferCount, cfg | UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);
    }
    else
    {
        if (huart->RxContinuous && huart->RxReceiveLength > UART_RX_BUFFER_SIZE)
        {
            huart->RxXferSize = huart->RxReceiveLength - huart->RxXferCount;
        }
        huart->RxBuffIndex = 0;
        huart->RxState     = HAL_UART_STATE_READY;
        huart->state &= ~HAL_UART_RX_ONGOING;

        if (huart->RxFullCallback)
            huart->RxFullCallback(huart);
    }
}

static void UART_TXFinishIRQHander(UART_HandleTypeDef *huart)
{
    uint32_t base = 0;
    uint8_t *buf  = uart_tx_buf[huart->id];

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (!UART_IsInited(huart)) || (huart->Tx_mode == 0))
    {
        return;
    }

    if (huart->TxXferSize)
    {
        if (huart->TxXferSize > UART_TX_BUFFER_SIZE)
        {
            huart->TxXferCount = UART_TX_BUFFER_SIZE;
        }
        else
        {
            huart->TxXferCount = huart->TxXferSize;
        }
        huart->TxXferSize -= huart->TxXferCount;

        for (int i = 0; i < huart->TxXferCount; i++)
        {
            buf[i] = huart->pTxBuffPtr[huart->TxBuffIndex + i];
        }
        huart->TxBuffIndex += huart->TxXferCount;

        HAL_UDMA_Enqueue(base, (unsigned int)buf, huart->TxXferCount, UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);
    }
    else
    {
        huart->TxState = HAL_UART_STATE_READY;
        huart->state &= ~HAL_UART_TX_ONGOING;

        if (huart->TxFinishCallback)
            huart->TxFinishCallback(huart);
    }
}

static void UART_RXPollingIRQHander(UART_HandleTypeDef *huart)
{
    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
        return;

    if (huart->RxPollingCallback)
        huart->RxPollingCallback(huart);
}

static void UART_ErrorIRQHander(UART_HandleTypeDef *huart)
{
    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
        return;

    huart->state |= HAL_UART_ERROR;

    if (huart->ErrorCallback)
        huart->ErrorCallback(huart);
}

static inline int UART_TransmitBusy(int uart_id)
{
    return reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * uart_id + UART_STATUS_OFFSET) & 1;
}

static inline int UART_ReceiveBusy(int uart_id)
{
    return (reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * uart_id + UART_STATUS_OFFSET) >> 1) & 1;
}

static HAL_StatusTypeDef UART_WaitReceiveDone(int uart_id, uint32_t timeout)
{
    // chech rx channel cfg.en 0-is ongoing
    int          cnt  = HAL_GetMs();
    unsigned int base = UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * uart_id + UDMA_CHANNEL_RX_OFFSET;

    while (HAL_UDMA_Busy(base))
    {
        if (HAL_GetMs() - cnt > timeout)
        {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef UART_WaitTransmitDone(uint32_t uart_id, uint32_t timeout)
{
    int          cnt  = HAL_GetMs();
    unsigned int base = UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * uart_id + UDMA_CHANNEL_TX_OFFSET;

    while (HAL_UDMA_Busy(base) || UART_TransmitBusy(uart_id))
    {
        if (HAL_GetMs() - cnt > timeout)
        {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

static inline void UART_Disable(int uart_id)
{
    // Bit5: CLEAN_FIFO (R/W)  In all mode clean the RX fifo, set 1 then set 0 to realize a reset fifo:
    // 0: Stop Clean the RX FIFO.
    // 1: Clean the RX FIFO.
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * uart_id + UART_SETUP_OFFSET, 0x00500026);

    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * uart_id + UART_SETUP_OFFSET, 0x00500006);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Clear dirty data received in the FIFO of the UART Instance
 * @param huart UART handle.
 */
void HAL_UART_Clr_FIFO(UART_HandleTypeDef *huart)
{
    uint32_t base = 0;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (!UART_IsInited(huart)))
    {
        return;
    }

    HAL_UART_LOCK();
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        base = UDMA_BASE_ADDR + UDMA_PERIPH_OFFSET(huart->id) + UART_SETUP_OFFSET;
        pulp_write32(base, pulp_read32(base) | (1 << 5));
        pulp_write32(base, reg_read32(base) & (~(0x01 << 5)));
    }
    else
    {
        HAL_UART_UNLOCK();
        return;
    }
    HAL_UART_UNLOCK();
}

/**
 * @brief Reset the UART Instance
 * @param huart UART handle.
 */
void HAL_UART_Reset(UART_HandleTypeDef *huart)
{
    HAL_UART_LOCK();
    uint32_t base = UDMA_BASE_ADDR + UDMA_CONF_OFFSET + UDMA_CONF_RESET_OFFSET;
    uint32_t val  = pulp_read32(base);
    pulp_write32(base, val | (1 << (0 + huart->id)));
    HAL_Delay(1);
    val = pulp_read32(base);
    pulp_write32(base, val & ~(1 << (0 + huart->id)));
    HAL_UART_UNLOCK();
}

/**
 * @brief Initialize the UART mode according to the specified parameters in the
 * UART_InitTypeDef and initialize the associated handle.
 * @param huart UART handle.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart)
{
    if ((huart == NULL) || (huart->id >= UART_ID_MAX))
        return HAL_ERROR;
    if (UART_IsInited(huart))
    {
        return HAL_BUSY;
    }

    HAL_UART_LOCK();

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    uart_rx_buf[huart->id] = IFRAM_CALLOC(UART_RX_BUFFER_SIZE);
    uart_tx_buf[huart->id] = IFRAM_CALLOC(UART_TX_BUFFER_SIZE);
    if (uart_rx_buf[huart->id] == NULL)
    {
        printf("%s: uart%d malloc ifram fail!\n", __func__, huart->id);
        HAL_UART_UNLOCK();
        return HAL_ERROR;
    }
    if (uart_tx_buf[huart->id] == NULL)
    {
        IframMgr_Free(uart_rx_buf[huart->id]);
        uart_rx_buf[huart->id] = NULL;
        printf("%s: uart%d malloc ifram fail!\n", __func__, huart->id);
        HAL_UART_UNLOCK();
        return HAL_ERROR;
    }
    uart_rx_ex_buf[huart->id] = IFRAM_CALLOC(UART_RX_EX_BUFFER_SIZE);
    if (uart_rx_ex_buf[huart->id] == NULL)
    {
        IframMgr_Free(uart_rx_buf[huart->id]);
        IframMgr_Free(uart_tx_buf[huart->id]);
        uart_rx_buf[huart->id] = NULL;
        uart_tx_buf[huart->id] = NULL;
        printf("%s: uart%d malloc ifram fail!\n", __func__, huart->id);
        HAL_UART_UNLOCK();
        return HAL_ERROR;
    }
#endif

    huart->state = HAL_UART_INITED;
    if (huart->init.BaudRate == 0)
    {
        huart->init.BaudRate = UART_DEFAULT_BAUDRATE;
    }

    // Defines uDMA peripherals clock gate configuration for UART0
    UART_CLKGateCfg(huart->id);

    UART_SetUp(huart);
    HAL_NVIC_ConnectIRQ(UART_RX_FULL_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_RXFullIRQHander, (void *)huart, 0);
    HAL_NVIC_ConnectIRQ(UART_TX_FINISH_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_TXFinishIRQHander, (void *)huart, 0);
    HAL_NVIC_ConnectIRQ(UART_RX_POLL_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_RXPollingIRQHander, (void *)huart, 0);
    HAL_NVIC_ConnectIRQ(UART_ERROR_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_ErrorIRQHander, (void *)huart, 0);

    huart->TxState             = HAL_UART_STATE_READY;
    huart->RxState             = HAL_UART_STATE_READY;
    huart->RxContinuous        = HAL_UART_RX_CONTINUOUS_DISABLE;
    huart->RxReceiveLength     = 0;
    huart->RxContinuousEx_Flag = false;
    huart->RxBuffIndexOut      = 0;

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief Send an amount of data in blocking mode.
 * @param huart UART handle
 * @param pData   Pointer to data buffer.
 * @param Size    Amount of data elements to be sent.
 * @param Timeout Timeout duration.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint32_t Size, uint32_t Timeout)
{
    uint32_t base         = 0;
    uint32_t transmit_len = 0;
    uint32_t sum_len      = 0;
    int      ret          = HAL_OK;
    int      i            = 0;
    uint8_t *buf          = uart_tx_buf[huart->id];

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (pData == NULL) || (!UART_IsInited(huart)))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();
    if (huart->TxState == HAL_UART_STATE_READY)
    {
        huart->TxState = HAL_UART_STATE_BUSY;
    }
    else
    {
        HAL_UART_UNLOCK();
        return HAL_BUSY;
    }
    HAL_UART_UNLOCK();

    huart->Tx_mode = 0;
    huart->state |= HAL_UART_TX_ONGOING;

    base = UDMA_BASE_ADDR + UDMA_PERIPH_OFFSET(huart->id) + UDMA_CHANNEL_TX_OFFSET;

    while (Size)
    {
        if (Size > UART_TX_BUFFER_SIZE)
        {
            transmit_len = UART_TX_BUFFER_SIZE;
        }
        else
        {
            transmit_len = Size;
        }
        Size -= transmit_len;

        for (i = 0; i < transmit_len; i++)
        {
            buf[i] = ((uint8_t *)pData)[sum_len + i];
        }

        HAL_UDMA_Enqueue(base, (unsigned int)buf, transmit_len, UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);

        if (UART_WaitTransmitDone(huart->id, Timeout) == HAL_TIMEOUT)
        {
            ret = HAL_TIMEOUT;
            break;
        }
        sum_len += transmit_len;
    }

    huart->TxState = HAL_UART_STATE_READY;
    huart->state &= ~HAL_UART_TX_ONGOING;

    return ret;
}

/**
 * @brief Send an amount of data in interrup mode.
 * @param huart UART handle
 * @param pData   Pointer to data buffer.
 * @param Size    Amount of data elements to be sent.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart, const uint8_t *pData, uint32_t Size)
{
    uint32_t base = 0;
    uint8_t *buf  = uart_tx_buf[huart->id];

    if (Size == 0)
        return HAL_OK;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (pData == NULL) || (!UART_IsInited(huart)))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();
    if (huart->TxState == HAL_UART_STATE_READY)
    {
        huart->TxState = HAL_UART_STATE_BUSY;
    }
    else
    {
        HAL_UART_UNLOCK();
        return HAL_BUSY;
    }
    HAL_UART_UNLOCK();
    HAL_NVIC_ClearPendingIRQ(UART_TX_FINISH_EVT(huart->id));
    HAL_NVIC_EnableIRQ(UART_TX_FINISH_EVT(huart->id));

    huart->Tx_mode = 1;
    huart->state |= HAL_UART_TX_ONGOING;

    base = UDMA_BASE_ADDR + UDMA_PERIPH_OFFSET(huart->id) + UDMA_CHANNEL_TX_OFFSET;

    if (Size > UART_TX_BUFFER_SIZE)
    {
        huart->TxXferCount = UART_TX_BUFFER_SIZE;
    }
    else
    {
        huart->TxXferCount = Size;
    }
    huart->TxXferSize = Size - huart->TxXferCount;
    huart->pTxBuffPtr = pData;

    for (int i = 0; i < huart->TxXferCount; i++)
    {
        buf[i] = ((uint8_t *)pData)[i];
    }
    huart->TxBuffIndex = huart->TxXferCount;

    HAL_UDMA_Enqueue(base, (unsigned int)buf, huart->TxXferCount, UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);

    return HAL_OK;
}

/**
 * @brief Receive an amount of data in continous.
 * @param huart   UART handle
 * @param pData   Pointer to data buffer (u8 or u16 data elements).
 * @param Size    Amount of data elements (u8 or u16) to be received.
 * @param Timeout Timeout duration.
 * @retval The number of bytes received, 0: timeout, -1: error.
 */
uint32_t HAL_UART_ReceiveContinous_ex(UART_HandleTypeDef *huart, uint8_t *pData, uint32_t Size, uint32_t Timeout)
{
    uint32_t index_in;
    uint32_t count     = 0;
    uint32_t count_sum = 0;
    uint32_t data_size;
    uint32_t udma_rx_size;
    uint32_t buff_size = UART_RX_BUFFER_SIZE;
    uint8_t *buf       = uart_rx_buf[huart->id];
    int      err       = -1;

    int cnt = HAL_GetMs();

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (pData == NULL) || (!UART_IsInited(huart)) || (Size == 0))
    {
        return err;
    }
    if (UART_ReceiveBusy(huart->id))
    {
        return err;
    }

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    if (uart_rx_ex_buf[huart->id] != NULL)
    {
        buf       = uart_rx_ex_buf[huart->id];
        buff_size = UART_RX_EX_BUFFER_SIZE;
    }
    else
    {
        return err;
    }
#endif

    HAL_UART_LOCK();

    if (huart->RxState == HAL_UART_STATE_READY)
    {
        huart->RxState = HAL_UART_STATE_BUSY;
    }
    else
    {
        HAL_UART_UNLOCK();
        return err;
    }

    huart->Rx_mode = 0;
    huart->state |= HAL_UART_RX_ONGOING;
    uint32_t base = UDMA_BASE_ADDR + UDMA_PERIPH_OFFSET(huart->id) + UDMA_CHANNEL_RX_OFFSET;

    if (!huart->RxContinuousEx_Flag)
    {
        HAL_UDMA_Enqueue(base, (unsigned int)buf, buff_size, (1 << UART_RX_CONTINOUS_OFFSET) | UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);
        huart->RxContinuousEx_Flag = true;
        huart->RxBuffIndexOut      = 0;
    }
    HAL_UART_UNLOCK();

    while (Size > 0)
    {
        if (HAL_GetMs() - cnt > Timeout)
        {
            break;
        }
        udma_rx_size = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_RX_SIZE_OFFSET);
        index_in     = buff_size - udma_rx_size;

        if (index_in == huart->RxBuffIndexOut || (index_in == 0 && huart->RxBuffIndexOut == buff_size))
        {
            continue;
        }

        if (index_in > huart->RxBuffIndexOut)
        {
            data_size = index_in - huart->RxBuffIndexOut;
            if (Size < data_size)
            {
                data_size = Size;
                memcpy(pData, &buf[huart->RxBuffIndexOut], data_size);
                huart->RxBuffIndexOut += data_size;
                count += data_size;
                count_sum += count;
                break;
            }
            memcpy(pData, &buf[huart->RxBuffIndexOut], data_size);
            pData += data_size;
            huart->RxBuffIndexOut += data_size;
            count += data_size;
            Size -= data_size;
        }
        else
        {
            data_size = buff_size - huart->RxBuffIndexOut;
            if (Size <= data_size)
            {
                count = Size;
                memcpy(pData, &buf[huart->RxBuffIndexOut], count);
                huart->RxBuffIndexOut += data_size;
                count_sum += count;
                break;
            }
            else
            {
                if (data_size)
                {
                    memcpy(pData, &buf[huart->RxBuffIndexOut], data_size);
                    Size -= data_size;
                    pData += data_size;
                    count = data_size;
                }
                data_size = index_in;
                if (Size < data_size)
                {
                    data_size = Size;
                    memcpy(pData, &buf[huart->RxBuffIndexOut], data_size);
                    huart->RxBuffIndexOut += data_size;
                    count += data_size;
                    count_sum += count;
                    break;
                }
                memcpy(pData, &buf[0], data_size);
                pData += data_size;
                count += data_size;
                Size -= data_size;
                huart->RxBuffIndexOut = data_size;
            }
        }
        count_sum += count;
        count = 0;
    }
    huart->RxState = HAL_UART_STATE_READY;
    return count_sum;
}

/**
 * @brief Receive an amount of data by poll mode.
 * @param huart UART handle
 * @param pData   Pointer to data buffer (u8 or u16 data elements).
 * @param Size    Amount of data elements (u8 or u16) to be received.
 * @param Timeout Timeout duration.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint32_t Size, uint32_t Timeout)
{
    uint32_t base        = 0;
    uint32_t receive_len = 0;
    uint32_t sum_len     = 0;
    int      ret         = HAL_OK;
    int      i           = 0;
    uint32_t cfg         = 0;
    uint8_t *buf         = uart_rx_buf[huart->id];

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (pData == NULL) || (!UART_IsInited(huart)))
    {
        return HAL_ERROR;
    }
    // clear fifo to ensure no messup
    // HAL_UART_Clr_FIFO(huart);

    HAL_UART_LOCK();
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        huart->RxState = HAL_UART_STATE_BUSY;
    }
    else
    {
        HAL_UART_UNLOCK();
        return HAL_BUSY;
    }
    HAL_UART_UNLOCK();

    huart->Rx_mode = 0;
    huart->state |= HAL_UART_RX_ONGOING;

    base    = UDMA_BASE_ADDR + UDMA_PERIPH_OFFSET(huart->id) + UDMA_CHANNEL_RX_OFFSET;
    sum_len = 0;

    while (Size)
    {
        if (Size > UART_RX_BUFFER_SIZE)
        {
            receive_len = UART_RX_BUFFER_SIZE;
        }
        else
        {
            receive_len = Size;
        }
        Size -= receive_len;
        if (huart->RxContinuous)
        {
            cfg |= (1 << UART_RX_CONTINOUS_OFFSET);
        }

        HAL_UDMA_Enqueue(base, (unsigned int)buf, receive_len, cfg | UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);

        if (UART_WaitReceiveDone(huart->id, Timeout) == HAL_TIMEOUT)
        {
            ret = HAL_TIMEOUT;
            break;
        }

        for (i = 0; i < receive_len; i++)
        {
            ((uint8_t *)pData)[sum_len + i] = buf[i];
        }
        sum_len += receive_len;
    }

    huart->RxState = HAL_UART_STATE_READY;
    HAL_UART_ReceiveStop(huart);
    return ret;
}

/**
 * @brief Receive an amount of data by interrup mode.
 * @param huart uart handle
 * @param pData   Pointer to data buffer (u8 or u16 data elements).
 * @param Size    Amount of data elements (u8 or u16) to be received.
 * @retval HAL status
 * @note This HAL_UART_IRQCfg can be used if it is necessary to turn off the RX_FULL interrupt
 */
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint32_t Size)
{
    uint32_t base = 0;
    uint32_t cfg  = 0;
    uint8_t *buf  = uart_rx_buf[huart->id];

    if (Size == 0)
        return HAL_OK;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (pData == NULL) || (!UART_IsInited(huart)))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        huart->RxState = HAL_UART_STATE_BUSY;
    }
    else
    {
        HAL_UART_UNLOCK();
        return HAL_BUSY;
    }
    HAL_UART_UNLOCK();

    HAL_NVIC_ClearPendingIRQ(UART_RX_FULL_EVT(huart->id));
    HAL_NVIC_EnableIRQ(UART_RX_FULL_EVT(huart->id));

    huart->Rx_mode = 1;
    huart->state |= HAL_UART_RX_ONGOING;

    base = UDMA_BASE_ADDR + UDMA_PERIPH_OFFSET(huart->id) + UDMA_CHANNEL_RX_OFFSET;

    if (Size > UART_RX_BUFFER_SIZE)
    {
        huart->RxXferCount = UART_RX_BUFFER_SIZE;
    }
    else
    {
        huart->RxXferCount = Size;
    }
    huart->RxXferSize      = Size - huart->RxXferCount;
    huart->RxBuffIndex     = 0;
    huart->pRxBuffPtr      = pData;
    huart->RxReceiveLength = Size;
    if (huart->RxContinuous)
    {
        cfg |= (1 << UART_RX_CONTINOUS_OFFSET);
    }

    HAL_UDMA_Enqueue(base, (unsigned int)buf, huart->RxXferCount, cfg | UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);
    return HAL_OK;
}

/**
 * @brief IRQ config
 * @param huart UART handle
 * @param RXFull_EN (R/W) DMA RX channel receives data full interrupt enable
 * flag 0: RXFull IRQ disable 1: RXFull IRQ enable
 * @param TXFinishIRQ_EN (R/W) DMA TX channel finish sending data interrupt
 * enable flag 0: TXFinish IRQ disable 1: TXFinish IRQ enable
 * @param ErrorIRQ_EN ERROR (R/W) Error interrupt in enable flag
 *    0: Error IRQ disable
 *    1: Error IRQ enable
 * @param RXPollingIRQ_EN RX (R/W) Rx interrupt in Polling mode  enable flag:
 *    0: RX Polling IRQ disable
 *    1: RX Polling IRQ enable
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_IRQCfg(UART_HandleTypeDef *huart, int RXFullIRQ_EN, int TXFinishIRQ_EN, int RXPollingIRQ_EN, int ErrorIRQ_EN)
{
    if ((huart == NULL) || (huart->id >= UART_ID_MAX))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();

    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_IRQ_OFFSET, (ErrorIRQ_EN << 1) | RXPollingIRQ_EN);

    HAL_UART_UNLOCK();

    if (RXFullIRQ_EN)
    {
        HAL_NVIC_ConnectIRQ(UART_RX_FULL_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_RXFullIRQHander, (void *)huart, 1);
    }
    else
    {
        HAL_NVIC_ConnectIRQ(UART_RX_FULL_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_RXFullIRQHander, (void *)huart, 0);
    }
    if (TXFinishIRQ_EN)
    {
        HAL_NVIC_ConnectIRQ(UART_TX_FINISH_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_TXFinishIRQHander, (void *)huart, 1);
    }
    else
    {
        HAL_NVIC_ConnectIRQ(UART_TX_FINISH_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_TXFinishIRQHander, (void *)huart, 0);
    }
    if (RXPollingIRQ_EN)
    {
        HAL_NVIC_ConnectIRQ(UART_RX_POLL_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_RXPollingIRQHander, (void *)huart, 1);
    }
    else
    {
        HAL_NVIC_ConnectIRQ(UART_RX_POLL_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_RXPollingIRQHander, (void *)huart, 0);
    }
    if (ErrorIRQ_EN)
    {
        HAL_NVIC_ConnectIRQ(UART_ERROR_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_ErrorIRQHander, (void *)huart, 1);
    }
    else
    {
        HAL_NVIC_ConnectIRQ(UART_ERROR_EVT(huart->id), UART_IRQ_PRIO, UART_IRQ_SUB_PRIO, (void *)UART_ErrorIRQHander, (void *)huart, 0);
    }
    return HAL_OK;
}

/**
 * @brief UART Receive data in a continuous mode.
 * @param huart UART handle
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_ReceiveContinous(UART_HandleTypeDef *huart)
{
    uint32_t data = 0;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();

    huart->RxContinuous = HAL_UART_RX_CONTINUOUS_ENABLE;
    data                = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_RX_CFG_OFFSET);
    data |= (1 << UART_RX_CONTINOUS_OFFSET);
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_RX_CFG_OFFSET, data);

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief UART Transmit Continuous.
 * @param  huart UART handle
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_TransmitContinous(UART_HandleTypeDef *huart)
{
    uint32_t data = 0;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();

    data = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_TX_CFG_OFFSET);
    data |= (1 << UART_TX_CONTINOUS_OFFSET);
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_TX_CFG_OFFSET, data);

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief UART Receive Resume.
 * @param  huart UART handle
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_ReceiveResume(UART_HandleTypeDef *huart)
{
    uint32_t data = 0;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();

    data = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_RX_CFG_OFFSET);
    data |= (1 << UART_RX_EN_OFFSET);
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_RX_CFG_OFFSET, data);

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief UART Transmit Resume.
 * @param  huart UART handle
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_TransmitResume(UART_HandleTypeDef *huart)
{
    uint32_t data = 0;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();

    data = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_TX_CFG_OFFSET);
    data |= (1 << UART_TX_EN_OFFSET);
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_TX_CFG_OFFSET, data);

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief stop and clean the UART Receive.
 * @param  huart UART handle
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_ReceiveStop(UART_HandleTypeDef *huart)
{
    uint32_t data = 0;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();

    data = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_RX_CFG_OFFSET);
    data |= (1 << UART_RX_CLEAR_OFFSET);
    data &= ~(1 << UART_RX_CONTINOUS_OFFSET);
    data &= ~(1 << UART_RX_EN_OFFSET);
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_RX_CFG_OFFSET, data);

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief stop and clean the UART Transmit.
 * @param  huart uart handle
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_TransmitStop(UART_HandleTypeDef *huart)
{
    uint32_t data = 0;

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
    {
        return HAL_ERROR;
    }

    HAL_UART_LOCK();

    data = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_TX_CFG_OFFSET);
    data |= (1 << UART_TX_CLEAR_OFFSET);
    data &= ~(1 << UART_TX_CONTINOUS_OFFSET);
    data &= ~(1 << UART_TX_EN_OFFSET);
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_TX_CFG_OFFSET, data);

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief DeInitialize the UART peripheral.
 * @param  huart UART handle
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart)
{

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || !UART_IsInited(huart))
    {
        return HAL_ERROR;
    }

    UART_WaitTransmitDone(huart->id, 500);

    UART_Disable(huart->id);

    HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() & ~(1 << huart->id));

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    if (uart_rx_buf[huart->id] != NULL)
    {
        IframMgr_Free(uart_rx_buf[huart->id]);
        uart_rx_buf[huart->id] = NULL;
    }
    if (uart_tx_buf[huart->id] != NULL)
    {
        IframMgr_Free(uart_tx_buf[huart->id]);
        uart_tx_buf[huart->id] = NULL;
    }
    if (uart_rx_ex_buf[huart->id] != NULL)
    {
        IframMgr_Free(uart_rx_ex_buf[huart->id]);
        uart_rx_ex_buf[huart->id] = NULL;
    }
#endif
    huart->RxContinuousEx_Flag = false;
    huart->state               = HAL_UART_IDLE;

    return HAL_OK;
}

/**
 * @brief  Register a User UART Callback
 * @note   The HAL_UART_RegisterCallback() should be called after
 * HAL_UART_Init()
 * @param  huart UART handle
 * @param  CallbackID ID of the callback to be registered
 * @param  pCallback pointer to the Callback function
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_RegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID, pUART_CallbackTypeDef pCallback)
{
    HAL_StatusTypeDef status = HAL_OK;
    if ((huart == NULL) || (pCallback == NULL))
        return HAL_ERROR;

    HAL_UART_LOCK();

    if (UART_IsInited(huart))
    {
        switch (CallbackID)
        {
            case HAL_UART_RX_FULL_CB_ID:
                huart->RxFullCallback = pCallback;
                break;

            case HAL_UART_TX_FINISH_CB_ID:
                huart->TxFinishCallback = pCallback;
                break;

            case HAL_UART_RX_POLLING_CB_ID:
                huart->RxPollingCallback = pCallback;
                break;

            case HAL_UART_ERROR_CB_ID:
                huart->ErrorCallback = pCallback;
                break;
            default:
                status = HAL_ERROR;
                break;
        }
    }

    HAL_UART_UNLOCK();

    return status;
}

/**
 * @brief  Unregister a User UART Callback
 * @note   The HAL_UART_RegisterCallback() should be called after
 * HAL_UART_Init()
 * @param  huart UART handle
 * @param  CallbackID ID of the callback to be registered
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_UART_UnRegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (huart == NULL)
        return HAL_ERROR;

    HAL_UART_LOCK();

    if (UART_IsInited(huart))
    {
        switch (CallbackID)
        {
            case HAL_UART_RX_FULL_CB_ID:
                huart->RxFullCallback = NULL;
                break;

            case HAL_UART_TX_FINISH_CB_ID:
                huart->TxFinishCallback = NULL;
                break;

            case HAL_UART_RX_POLLING_CB_ID:
                huart->RxPollingCallback = NULL;
                break;

            case HAL_UART_ERROR_CB_ID:
                huart->ErrorCallback = NULL;
                break;

            default:
                status = HAL_ERROR;
                break;
        }
    }

    HAL_UART_UNLOCK();

    return status;
}

/**
 * @brief Start uart receive data action in continous mode.
 * @param huart   UART handle
 * @retval HAL status
 *
 * This function only initiates the continuous mode of UART for receiving data,
 * and the action of copying data from the ifram is placed in the polling thread.
 */
HAL_StatusTypeDef HAL_UART_ReceiveContinousForPoll(UART_HandleTypeDef *huart)
{
    uint32_t buff_size = UART_RX_BUFFER_SIZE;
    uint8_t *buf       = uart_rx_buf[huart->id];

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (!UART_IsInited(huart)))
    {
        return HAL_ERROR;
    }
    if (UART_ReceiveBusy(huart->id))
    {
        return HAL_BUSY;
    }

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    if (uart_rx_ex_buf[huart->id] != NULL)
    {
        buf       = uart_rx_ex_buf[huart->id];
        buff_size = UART_RX_EX_BUFFER_SIZE;
    }
    else
    {
        return HAL_ERROR;
    }
#endif

    HAL_UART_LOCK();

    if (huart->RxState == HAL_UART_STATE_READY)
    {
        huart->RxState = HAL_UART_STATE_BUSY;
    }
    else
    {
        HAL_UART_UNLOCK();
        return HAL_ERROR;
    }

    huart->Rx_mode = 0;
    huart->state |= HAL_UART_RX_ONGOING;
    huart->RxBuffIndexOut      = 0;
    huart->RxContinuousEx_Flag = true;
    huart->RxContinuous        = HAL_UART_RX_CONTINUOUS_ENABLE;

    //{Mitigate the risk of RX_SIZE corruption when enable UART continuous mode
    uint32_t val = 0;
    val          = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_SETUP_OFFSET);
    val |= (1 << 5); // set CLEAN_FIFO bit
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_SETUP_OFFSET, val);
    __DSB();
    val &= ~(1 << 5); // unset CLEAN_FIFO bit
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_SETUP_OFFSET, val);
    __DSB();
    //}

    uint32_t base = UDMA_BASE_ADDR + UDMA_PERIPH_OFFSET(huart->id) + UDMA_CHANNEL_RX_OFFSET;

    HAL_UDMA_Enqueue(base, (unsigned int)buf, buff_size, (1 << UART_RX_CONTINOUS_OFFSET) | UDMA_CHANNEL_CFG_EN);

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief Deactivate UART continuous reception mode and empty the FIFO to ensure clean state.
 * @param huart   UART handle
 * @retval HAL status
 *
 * This function Deactivate UART continuous mode,
 * empty the FIFO to ensure clean state to mitigate potential conflicts.
 */
HAL_StatusTypeDef HAL_UART_DeactivateContinousForPoll(UART_HandleTypeDef *huart)
{

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (!UART_IsInited(huart)))
    {
        return HAL_ERROR;
    }
    /*
       Since the condition here will be met, causing function exit,
       if the peer device continues transmitting.
       we can safely ignore this condition
       when explicitly stopping reception and flushing the buffer.
     */
    // if (UART_ReceiveBusy(huart->id)) {
    //   return HAL_BUSY;
    // }
    int          cnt  = HAL_GetMs();
    unsigned int base = UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UDMA_CHANNEL_RX_OFFSET;

    while (UART_ReceiveBusy(huart->id) || ((!huart->RxContinuous) && HAL_UDMA_Busy(base)))
    {
        if (HAL_GetMs() - cnt > 100)
        {
            break;
        }
    }

    HAL_UART_LOCK();

    huart->RxState = HAL_UART_STATE_READY;

    huart->Rx_mode = 0;
    huart->state &= ~HAL_UART_RX_ONGOING;
    huart->RxBuffIndexOut      = 0;
    huart->RxContinuousEx_Flag = false;
    huart->RxContinuous        = HAL_UART_RX_CONTINUOUS_DISABLE;

    //{Mitigate the risk of RX_SIZE corruption when enable UART continuous mode
    uint32_t val = 0;
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_RX_CFG_OFFSET, 1 << UART_RX_CLEAR_OFFSET);

    val = reg_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_SETUP_OFFSET);
    val |= (1 << 5); // set CLEAN_FIFO bit
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_SETUP_OFFSET, val);
    __DSB();
    val &= ~(1 << 5); // unset CLEAN_FIFO bit
    reg_write32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id + UART_SETUP_OFFSET, val);
    __DSB();
    //}

    HAL_UART_UNLOCK();

    return HAL_OK;
}

/**
 * @brief Obtain the address and size information of the UART ifram.
 * @param huart   UART handle
 * @retval HAL status
 *
 * The information about the ifram would be more appropriately
 * stored in the UART_HandleTypeDef structure for each UART device.
 */
HAL_StatusTypeDef HAL_UART_GetIFramInfoForPoll(UART_HandleTypeDef *huart, uint32_t *iframSize, uint8_t **iframBuff)
{
    uint32_t buff_size = UART_RX_BUFFER_SIZE;
    uint8_t *buf       = uart_rx_buf[huart->id];

    if ((huart == NULL) || (huart->id >= UART_ID_MAX) || (!UART_IsInited(huart)))
    {
        return HAL_ERROR;
    }
    //   if (UART_ReceiveBusy(huart->id)) {
    //     return HAL_BUSY;
    //   }

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    if (uart_rx_ex_buf[huart->id] != NULL)
    {
        buf       = uart_rx_ex_buf[huart->id];
        buff_size = UART_RX_EX_BUFFER_SIZE;
    }
    else
    {
        return HAL_ERROR;
    }
#endif

    *iframSize = buff_size;
    *iframBuff = buf;

    return HAL_OK;
}
#endif /* HAL_UART_MODULE_ENABLED */
