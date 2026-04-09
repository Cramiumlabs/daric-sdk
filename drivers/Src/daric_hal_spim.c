/**
 ******************************************************************************
 * @file    daric_hal_spim.c
 * @author  SPIM Team
 * @brief   SPIM HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the Serial Peripheral Interface Master (SPIM)
 *          peripheral:
 *           + Initialization and de-initialization functions
 *           + Data transfer functions (read/write)
 *           + Configuration functions
 *           + Interrupt handling functions
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
#include "daric_hal_conf.h"
#include "daric.h"
#include "daric_hal_nvic.h"
#include "daric_hal_spim.h"
#include "daric_pulp_io.h"
#include "daric_udma.h"
#include "daric_udma_spim_v3.h"
#include "daric_util.h"
#include <stdio.h>
#include <string.h>

#ifdef HAL_SPIM_MODULE_ENABLED

/** @cond Private macros
 * @{
 */
/* Private macros ------------------------------------------------------------*/
/* SPIM IRQ Priority define */
#ifdef HAL_SPIM_IRQ_PRIO
#define SPIM_IRQ_PRIO HAL_SPIM_IRQ_PRIO
#else
#define SPIM_IRQ_PRIO 0
#endif

#ifdef HAL_SPIM_IRQ_SUB_PRIO
#define SPIM_IRQ_SUB_PRIO HAL_SPIM_IRQ_SUB_PRIO
#else
#define SPIM_IRQ_SUB_PRIO 0
#endif

#define HAL_SPIM_LOCK   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_SPIM_UNLOCK HAL_UNLOCK

#define assert_SPIM_param(expr)                                    \
    do                                                             \
    {                                                              \
        if (!(expr))                                               \
        {                                                          \
            printf("SPIM parameter assertion failed:%s\n", #expr); \
            return HAL_ERROR;                                      \
        }                                                          \
    } while (0);

#define IS_SPI_HANDLE(__CONFIG__)     ((__CONFIG__) != NULL)
#define IS_SPI_ID(__CONFIG__)         (((__CONFIG__) == SPIM0) || ((__CONFIG__) == SPIM1) || ((__CONFIG__) == SPIM2) || ((__CONFIG__) == SPIM3))
#define IS_SPI_NOT_ACTIVE(__CONFIG__) ((__CONFIG__) == 0)
#define IS_SPI_ACTIVE(__CONFIG__)     ((__CONFIG__) == 1)

#define IS_SPI_CS(__CONFIG__) (((__CONFIG__) == SPI_CMD_SOT_CS0) || ((__CONFIG__) == SPI_CMD_SOT_CS1) || ((__CONFIG__) == SPI_CMD_SOT_CS2) || ((__CONFIG__) == SPI_CMD_SOT_CS3))

#define IS_SPI_WORDSIZE(__CONFIG__) ((__CONFIG__ >= SPIM_WORDSIZE_1) && (__CONFIG__ <= SPIM_WORDSIZE_32))

#define IS_SPI_ENDIAN(__CONFIG__) ((__CONFIG__ == SPI_CMD_MSB_FIRST) || (__CONFIG__ == SPI_CMD_LSB_FIRST))

#define IS_SPI_CPOL(__CONFIG__) ((__CONFIG__ == SPI_CMD_CFG_CPOL_POS) || (__CONFIG__ == SPI_CMD_CFG_CPOL_NEG))

#define IS_SPI_CPHA(__CONFIG__) ((__CONFIG__ == SPI_CMD_CFG_CPHA_STD) || (__CONFIG__ == SPI_CMD_CFG_CPHA_OPP))

#define SPIM_DEFAULT_BAUDRATE 1000000
#define SPIM_MAX_BAUDRATE     30000000
#define SPIM_RX_BUFFER_SIZE   (1024)
#define SPIM_TX_BUFFER_SIZE   (1024)

/** @endcond
 * @}
 */

static SPIM_HandleTypeDef *g_spim_handle[ARCHI_UDMA_NB_SPIM];
typedef struct
{
    uint32_t cmd[4];
} spim_cmd_t;

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
spim_cmd_t *g_cmd[ARCHI_UDMA_NB_SPIM] = { 0 };
uint8_t    *spim_rx_buf[ARCHI_UDMA_NB_SPIM];
uint8_t    *spim_tx_buf[ARCHI_UDMA_NB_SPIM];
#else
spim_cmd_t g_cmd[ARCHI_UDMA_NB_SPIM] __attribute__((section("ifram")));
uint8_t    spim_rx_buf[ARCHI_UDMA_NB_SPIM][SPIM_RX_BUFFER_SIZE] __attribute__((section("ifram"))) = { 0 };
uint8_t    spim_tx_buf[ARCHI_UDMA_NB_SPIM][SPIM_TX_BUFFER_SIZE] __attribute__((section("ifram"))) = { 0 };
#endif

#if defined(CONFIG_SOC_DARIC_NTO_A)
static uint8_t SPIM_Get_Div(uint32_t spi_freq)
{
    uint8_t div      = 0;
    double  perClkHz = 0;

    perClkHz = HAL_GetPerClkHz();
    if (perClkHz > SPIM_MAX_BAUDRATE)
    {
        if (spi_freq >= SPIM_MAX_BAUDRATE)
        {
            spi_freq = SPIM_MAX_BAUDRATE;
        }
    }
    else
    {
        if (spi_freq >= perClkHz)
        {
            /* 0 means no frequency division */
            return 0;
        }
    }
    /* Round-up the divider */
    div = (perClkHz / spi_freq / 2);
    return div;
}
#else
static uint8_t SPIM_Get_Div(uint32_t spi_freq)
{

    // Robin Van TODO:
    uint8_t  div         = 0;
    uint32_t periph_freq = HAL_UDMA_Get_Perh_Clock();

    // asic confirm that the MAX SPI communication frequency does not exceed 30M
    if (periph_freq > SPIM_MAX_BAUDRATE)
    {

        if (spi_freq >= SPIM_MAX_BAUDRATE)
        {
            div = (periph_freq + SPIM_MAX_BAUDRATE - 1) / SPIM_MAX_BAUDRATE;
        }
        else
        {
            div = (periph_freq + spi_freq - 1) / spi_freq;
        }
    }
    else
    {

        if (spi_freq >= periph_freq)
        {
            return 0;
        }
        else
        {
            // Round-up the divider to obtain an SPI frequency which is below the
            // maximum
            div = (periph_freq + spi_freq - 1) / spi_freq;
        }
    }
    // The SPIM always divide by 2 once we activate the divider, thus increase
    // by 1 in case it is even to not go avove the max frequency.
    if (div & 1)
        div += 1;
    div >>= 1;

    return div;
}
#endif

static uint8_t SPIM_CHECKLEN(uint8_t wordsize, uint32_t len)
{
    if (len == 0)
        return HAL_ERROR;

    if (wordsize <= 8)
    {
        return HAL_OK;
    }
    else if (wordsize <= 16)
    {
        if (len % 2 != 0)
            return HAL_ERROR;
    }
    else if (wordsize <= 32)
    {
        if (len % 4 != 0)
            return HAL_ERROR;
    }
    return HAL_OK;
}

// STATUS Bit 1 RX_BUSY / Bit 0 TX_BUSY
static void SPIM_RX_IRQ_Handler(SPIM_HandleTypeDef *handle)
{

    if ((handle == NULL) || (handle->id >= ARCHI_UDMA_NB_SPIM) || (handle->active_flg == 0))
        return;
}

static void SPIM_TX_IRQ_Handler(SPIM_HandleTypeDef *handle)
{

    if ((handle == NULL) || (handle->id >= ARCHI_UDMA_NB_SPIM) || (handle->active_flg == 0))
        return;
}

static void SPIM_CMD_IRQ_Handler(SPIM_HandleTypeDef *handle)
{

    if ((handle == NULL) || (handle->id >= ARCHI_UDMA_NB_SPIM) || (handle->active_flg == 0))
        return;
}

static void SPIM_EOT_IRQ_Handlerr(SPIM_HandleTypeDef *handle)
{
    uint32_t cb = 0;
    if ((handle == NULL) || (handle->id >= ARCHI_UDMA_NB_SPIM) || (handle->active_flg == 0))
        return;

    if (handle->sending_size)
    {
        if (handle->sending_size < SPIM_TX_BUFFER_SIZE)
            cb = 1;
        handle->sending_size = 0;
    }

    if (handle->recving_size)
    {
        memcpy(handle->recving_addr, spim_rx_buf[handle->id], handle->recving_size);
        if (handle->recving_size < SPIM_RX_BUFFER_SIZE)
            cb = 1;
        handle->recving_size = 0;
        handle->recving_addr = NULL;
    }

    if (cb)
    {
        if ((handle->CpltCallback) != NULL)
            handle->CpltCallback(handle);
    }
}

HAL_StatusTypeDef HAL_SPIM_WaitCanEnqueue(uint32_t channel_base, uint32_t *Timeout)
{
    uint32_t tickstart, tickend, tickgap = 0;
    tickstart        = HAL_GetMs();
    uint32_t timeout = *Timeout;
    while (!HAL_UDMA_canEnqueue(channel_base))
    {
        if ((((HAL_GetMs() - tickstart) >= timeout) && (timeout != HAL_MAX_DELAY)) || (timeout == 0U))
        {
            return HAL_TIMEOUT;
        }
    }
    tickend = HAL_GetMs();
    tickgap = tickend - tickstart;
    if (tickgap > timeout)
    {
        *Timeout = 0;
        return HAL_TIMEOUT;
    }
    else
        *Timeout -= tickgap;

    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPIM_WaitSendDone(uint8_t id, uint32_t *Timeout)
{
    uint32_t tickstart, tickend, tickgap = 0;
    tickstart        = HAL_GetMs();
    uint32_t timeout = *Timeout;

    while (HAL_UDMA_Busy(HAL_UDMA_Periph_Base(g_spim_handle[id]->channel) + UDMA_CHANNEL_TX_OFFSET)
           || HAL_UDMA_Busy(HAL_UDMA_Periph_Base(g_spim_handle[id]->channel) + UDMA_CHANNEL_CUSTOM_OFFSET))
    {
        if ((((HAL_GetMs() - tickstart) >= timeout) && (timeout != HAL_MAX_DELAY)) || (timeout == 0U))
        {
            return HAL_TIMEOUT;
        }
    }
    tickend = HAL_GetMs();
    tickgap = tickend - tickstart;
    if (tickgap > timeout)
    {
        *Timeout = 0;
        return HAL_TIMEOUT;
    }
    else
        *Timeout -= tickgap;

    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPIM_WaitReceiveDone(uint8_t id, uint32_t *Timeout)
{
    uint32_t tickstart, tickend, tickgap = 0;
    tickstart        = HAL_GetMs();
    uint32_t timeout = *Timeout;

    while (HAL_UDMA_Busy(HAL_UDMA_Periph_Base(g_spim_handle[id]->channel) + UDMA_CHANNEL_RX_OFFSET)
           || HAL_UDMA_Busy(HAL_UDMA_Periph_Base(g_spim_handle[id]->channel) + UDMA_CHANNEL_CUSTOM_OFFSET))
    {
        if ((((HAL_GetMs() - tickstart) >= timeout) && (timeout != HAL_MAX_DELAY)) || (timeout == 0U))
        {
            return HAL_TIMEOUT;
        }
    }
    tickend = HAL_GetMs();
    tickgap = tickend - tickstart;
    if (tickgap > timeout)
    {
        *Timeout = 0;
        return HAL_TIMEOUT;
    }
    else
        *Timeout -= tickgap;

    return HAL_OK;
}

HAL_StatusTypeDef HAL_SPIM_TransferDone(uint8_t id, uint32_t *Timeout)
{
    if (HAL_SPIM_WaitSendDone(id, Timeout) != HAL_OK)
        return HAL_TIMEOUT;
    if (HAL_SPIM_WaitReceiveDone(id, Timeout) != HAL_OK)
        return HAL_TIMEOUT;
    return HAL_OK;
}

static HAL_StatusTypeDef do_spim_send(uint8_t id, void *data, uint16_t len, uint32_t qspi, SPIM_CS_TypeDef cs_mode, uint8_t word_size, uint32_t Timeout)
{
    uint32_t    periph_base  = 0;
    uint32_t    cmd_base     = 0;
    uint32_t    channel_base = 0;
    spim_cmd_t *cmd          = NULL;
    uint32_t    cfg          = 0;
    uint8_t     periph_id    = 0;
    uint32_t    wordlen      = 0;
    uint32_t    onceTimeout  = 0;
    onceTimeout              = Timeout;

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    cmd = g_cmd[id];
#else
    cmd = &g_cmd[id];
#endif
    periph_id    = g_spim_handle[id]->channel;
    periph_base  = (uint32_t)(HAL_UDMA_Periph_Base(periph_id));
    cmd_base     = periph_base + ARCHI_SPIM_CMD_OFFSET;
    channel_base = periph_base + UDMA_CHANNEL_TX_OFFSET;

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(SPIM0_0_IRQn + id * 4); // RX
    HAL_NVIC_DisableIRQ(SPIM0_1_IRQn + id * 4); // TX
    HAL_NVIC_DisableIRQ(SPIM0_2_IRQn + id * 4); // CMD
    HAL_NVIC_DisableIRQ(SPIM0_3_IRQn + id * 4); // EOT

    if (HAL_SPIM_WaitCanEnqueue(channel_base, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait Send Can Enqueue( timeout\n", id);
        return HAL_TIMEOUT;
    }
    if (HAL_SPIM_WaitSendDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait send timeout\n", id);
        return HAL_TIMEOUT;
    }

    HAL_SPIM_LOCK
    // len == buf bytes
    // word_size ==each word bits
    // wordlen == word nums
    cfg = UDMA_CHANNEL_CFG_EN;
    if (word_size <= 8)
    {
        wordlen = len;
        cfg |= UDMA_CHANNEL_CFG_SIZE_8;
    }
    else if (word_size <= 16)
    {
        wordlen = len / 2;
        cfg |= UDMA_CHANNEL_CFG_SIZE_16;
    }
    else if (word_size <= 32)
    {
        wordlen = len / 4;
        cfg |= UDMA_CHANNEL_CFG_SIZE_32;
    }

    cmd->cmd[0] = g_spim_handle[id]->cfg;
    cmd->cmd[1] = (uint32_t)(SPI_CMD_SOT(g_spim_handle[id]->cs));
    cmd->cmd[3] = (uint32_t)(SPI_CMD_EOT(1, cs_mode == SPIM_CS_KEEP));
    cmd->cmd[2] = SPI_CMD_TX_DATA(wordlen, SPI_CMD_1_WORD_PER_TRANSF, word_size, qspi, g_spim_handle[id]->big_endian ? SPI_CMD_MSB_FIRST : SPI_CMD_LSB_FIRST);

    memcpy(spim_tx_buf[id], data, len);

    HAL_UDMA_Enqueue(channel_base, (uint32_t)spim_tx_buf[id], len, cfg);
    cfg = UDMA_CHANNEL_CFG_SIZE_32 | UDMA_CHANNEL_CFG_EN;
    HAL_UDMA_Enqueue(cmd_base, (uint32_t)cmd, 4 * 4, cfg);
    if (HAL_SPIM_WaitSendDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] send timeout\n", id);
        return HAL_TIMEOUT;
    }
    HAL_SPIM_UNLOCK

    return HAL_OK;
}

static HAL_StatusTypeDef do_spim_send_it(uint8_t id, const uint8_t *data, uint16_t len, uint32_t qspi, SPIM_CS_TypeDef cs_mode, uint8_t word_size)
{
    uint32_t    periph_base  = 0;
    uint32_t    cmd_base     = 0;
    uint32_t    channel_base = 0;
    spim_cmd_t *cmd          = NULL;
    uint32_t    cfg          = 0;
    uint8_t     periph_id    = 0;
    uint32_t    wordlen      = 0;
    uint32_t    onceTimeout  = 0;
    onceTimeout              = HAL_MAX_DELAY - 1;

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    cmd = g_cmd[id];
#else
    cmd = &g_cmd[id];
#endif

    periph_id    = g_spim_handle[id]->channel;
    periph_base  = (uint32_t)(HAL_UDMA_Periph_Base(periph_id));
    cmd_base     = periph_base + ARCHI_SPIM_CMD_OFFSET;
    channel_base = periph_base + UDMA_CHANNEL_TX_OFFSET;

    /* Enable interrupts */
    HAL_NVIC_EnableIRQ(SPIM0_1_IRQn + id * 4); // TX
    HAL_NVIC_EnableIRQ(SPIM0_2_IRQn + id * 4); // CMD
    HAL_NVIC_EnableIRQ(SPIM0_3_IRQn + id * 4); // EOT

    if (HAL_SPIM_WaitCanEnqueue(channel_base, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait Send Can Enqueue( timeout\n", id);
        return HAL_TIMEOUT;
    }
    if (HAL_SPIM_WaitSendDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait send timeout\n", id);
        return HAL_TIMEOUT;
    }

    HAL_SPIM_LOCK
    // len == buf bytes
    // word_size ==each word bits
    // wordlen == word nums
    cfg = UDMA_CHANNEL_CFG_EN;
    if (word_size <= 8)
    {
        wordlen = len;
        cfg |= UDMA_CHANNEL_CFG_SIZE_8;
    }
    else if (word_size <= 16)
    {
        wordlen = len / 2;
        cfg |= UDMA_CHANNEL_CFG_SIZE_16;
    }
    else if (word_size <= 32)
    {
        wordlen = len / 4;
        cfg |= UDMA_CHANNEL_CFG_SIZE_32;
    }
    cmd->cmd[0] = g_spim_handle[id]->cfg;
    cmd->cmd[1] = (uint32_t)(SPI_CMD_SOT(g_spim_handle[id]->cs));
    cmd->cmd[3] = (uint32_t)(SPI_CMD_EOT(1, cs_mode == SPIM_CS_KEEP));
    cmd->cmd[2] = SPI_CMD_TX_DATA(wordlen, SPI_CMD_1_WORD_PER_TRANSF, word_size, qspi, g_spim_handle[id]->big_endian ? SPI_CMD_MSB_FIRST : SPI_CMD_LSB_FIRST);

    memcpy(spim_tx_buf[id], data, len);
    g_spim_handle[id]->sending_size = len;

    HAL_UDMA_Enqueue(channel_base, (uint32_t)spim_tx_buf[id], len, cfg);
    cfg = UDMA_CHANNEL_CFG_SIZE_32 | UDMA_CHANNEL_CFG_EN;
    HAL_UDMA_Enqueue(cmd_base, (uint32_t)cmd, 4 * 4, cfg);
    HAL_SPIM_UNLOCK
    return HAL_OK;
}

static HAL_StatusTypeDef do_spim_receive(uint8_t id, void *data, uint32_t len, uint32_t qspi, SPIM_CS_TypeDef cs_mode, uint8_t word_size, uint32_t Timeout)
{
    uint32_t    periph_base  = 0;
    uint32_t    cmd_base     = 0;
    uint32_t    channel_base = 0;
    spim_cmd_t *cmd          = NULL;
    uint32_t    cfg          = 0;
    uint8_t     periph_id    = 0;
    uint32_t    wordlen      = 0;
    uint32_t    onceTimeout  = 0;
    onceTimeout              = Timeout;

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    cmd = g_cmd[id];
#else
    cmd = &g_cmd[id];
#endif

    periph_id    = g_spim_handle[id]->channel;
    periph_base  = (uint32_t)(HAL_UDMA_Periph_Base(periph_id));
    cmd_base     = periph_base + ARCHI_SPIM_CMD_OFFSET;
    channel_base = periph_base + UDMA_CHANNEL_RX_OFFSET;

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(SPIM0_0_IRQn + id * 4); // RX
    HAL_NVIC_DisableIRQ(SPIM0_1_IRQn + id * 4); // TX
    HAL_NVIC_DisableIRQ(SPIM0_2_IRQn + id * 4); // CMD
    HAL_NVIC_DisableIRQ(SPIM0_3_IRQn + id * 4); // EOT

    if (HAL_SPIM_WaitCanEnqueue(channel_base, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait receive Can Enqueue( timeout\n", id);
        return HAL_TIMEOUT;
    }

    if (HAL_SPIM_WaitReceiveDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] receive timeout\n", id);
        return HAL_TIMEOUT;
    }

    HAL_SPIM_LOCK
    // len == buf bytes
    // word_size ==each word bits
    // wordlen == word nums
    cfg = UDMA_CHANNEL_CFG_EN;
    if (word_size <= 8)
    {
        wordlen = len;
        cfg |= UDMA_CHANNEL_CFG_SIZE_8;
    }
    else if (word_size <= 16)
    {
        wordlen = len / 2;
        cfg |= UDMA_CHANNEL_CFG_SIZE_16;
    }
    else if (word_size <= 32)
    {
        wordlen = len / 4;
        cfg |= UDMA_CHANNEL_CFG_SIZE_32;
    }
    cmd->cmd[0] = g_spim_handle[id]->cfg;
    cmd->cmd[1] = (uint32_t)(SPI_CMD_SOT(g_spim_handle[id]->cs));
    cmd->cmd[3] = (uint32_t)(SPI_CMD_EOT(1, cs_mode == SPIM_CS_KEEP));
    cmd->cmd[2] = SPI_CMD_RX_DATA(wordlen, SPI_CMD_1_WORD_PER_TRANSF, word_size, qspi, g_spim_handle[id]->big_endian ? SPI_CMD_MSB_FIRST : SPI_CMD_LSB_FIRST);

    HAL_UDMA_Enqueue(channel_base, (uint32_t)spim_rx_buf[id], len, cfg);
    cfg = UDMA_CHANNEL_CFG_SIZE_32 | UDMA_CHANNEL_CFG_EN;
    HAL_UDMA_Enqueue(cmd_base, (uint32_t)cmd, 4 * 4, cfg);
    if (HAL_SPIM_WaitReceiveDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] receive timeout\n", id);
        HAL_SPIM_UNLOCK
        return HAL_TIMEOUT;
    }
    memcpy(data, spim_rx_buf[id], len);
    HAL_SPIM_UNLOCK
    return HAL_OK;
}

static HAL_StatusTypeDef do_spim_receive_it(uint8_t id, const uint8_t *data, uint16_t len, uint32_t qspi, SPIM_CS_TypeDef cs_mode, uint8_t word_size)
{
    uint32_t    periph_base  = 0;
    uint32_t    cmd_base     = 0;
    uint32_t    channel_base = 0;
    spim_cmd_t *cmd          = NULL;
    uint32_t    cfg          = 0;
    uint8_t     periph_id    = 0;
    uint32_t    wordlen      = 0;
    uint32_t    onceTimeout  = 0;
    onceTimeout              = HAL_MAX_DELAY - 1;

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    cmd = g_cmd[id];
#else
    cmd = &g_cmd[id];
#endif

    periph_id    = g_spim_handle[id]->channel;
    periph_base  = (uint32_t)(HAL_UDMA_Periph_Base(periph_id));
    cmd_base     = periph_base + ARCHI_SPIM_CMD_OFFSET;
    channel_base = periph_base + UDMA_CHANNEL_RX_OFFSET;

    /* Enable interrupts */
    HAL_NVIC_EnableIRQ(SPIM0_0_IRQn + id * 4); // RX
    HAL_NVIC_EnableIRQ(SPIM0_2_IRQn + id * 4); // CMD
    HAL_NVIC_EnableIRQ(SPIM0_3_IRQn + id * 4); // EOT

    if (HAL_SPIM_WaitCanEnqueue(channel_base, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait receive Can Enqueue( timeout\n", id);
        return HAL_TIMEOUT;
    }
    if (HAL_SPIM_WaitReceiveDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait receive timeout\n", id);
        return HAL_TIMEOUT;
    }

    HAL_SPIM_LOCK
    // len == buf bytes
    // word_size ==each word bits
    // wordlen == word nums
    cfg = UDMA_CHANNEL_CFG_EN;
    if (word_size <= 8)
    {
        wordlen = len;
        cfg |= UDMA_CHANNEL_CFG_SIZE_8;
    }
    else if (word_size <= 16)
    {
        wordlen = len / 2;
        cfg |= UDMA_CHANNEL_CFG_SIZE_16;
    }
    else if (word_size <= 32)
    {
        wordlen = len / 4;
        cfg |= UDMA_CHANNEL_CFG_SIZE_32;
    }
    cmd->cmd[0] = g_spim_handle[id]->cfg;
    cmd->cmd[1] = (uint32_t)(SPI_CMD_SOT(g_spim_handle[id]->cs));
    cmd->cmd[3] = (uint32_t)(SPI_CMD_EOT(1, cs_mode == SPIM_CS_KEEP));
    cmd->cmd[2] = SPI_CMD_RX_DATA(wordlen, SPI_CMD_1_WORD_PER_TRANSF, word_size, qspi, g_spim_handle[id]->big_endian ? SPI_CMD_MSB_FIRST : SPI_CMD_LSB_FIRST);

    g_spim_handle[id]->recving_size = len;
    g_spim_handle[id]->recving_addr = (uint8_t *)data;

    HAL_UDMA_Enqueue(channel_base, (uint32_t)spim_rx_buf[id], len, cfg);
    cfg = UDMA_CHANNEL_CFG_SIZE_32 | UDMA_CHANNEL_CFG_EN;
    HAL_UDMA_Enqueue(cmd_base, (uint32_t)cmd, 4 * 4, cfg);
    HAL_SPIM_UNLOCK
    return HAL_OK;
}

static HAL_StatusTypeDef do_spim_xfer(uint8_t id, void *txdata, void *rxdata, uint32_t len, SPIM_CS_TypeDef cs_mode, uint8_t word_size, uint32_t Timeout)
{
    uint32_t    periph_base     = 0;
    uint32_t    cmd_base        = 0;
    uint32_t    rx_channel_base = 0;
    uint32_t    tx_channel_base = 0;
    spim_cmd_t *cmd             = NULL;
    uint32_t    cfg             = 0;
    uint8_t     periph_id       = 0;
    uint32_t    wordlen         = 0;
    uint32_t    onceTimeout     = 0;
    onceTimeout                 = Timeout;

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    cmd = g_cmd[id];
#else
    cmd = &g_cmd[id];
#endif

    periph_id       = g_spim_handle[id]->channel;
    periph_base     = (uint32_t)(HAL_UDMA_Periph_Base(periph_id));
    cmd_base        = periph_base + ARCHI_SPIM_CMD_OFFSET;
    rx_channel_base = periph_base + UDMA_CHANNEL_RX_OFFSET;
    tx_channel_base = periph_base + UDMA_CHANNEL_TX_OFFSET;

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(SPIM0_0_IRQn + id * 4); // RX
    HAL_NVIC_DisableIRQ(SPIM0_1_IRQn + id * 4); // TX
    HAL_NVIC_DisableIRQ(SPIM0_2_IRQn + id * 4); // CMD
    HAL_NVIC_DisableIRQ(SPIM0_3_IRQn + id * 4); // EOT

    if (HAL_SPIM_WaitCanEnqueue(tx_channel_base, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait send Can Enqueue( timeout\n", id);
        return HAL_TIMEOUT;
    }
    if (HAL_SPIM_WaitCanEnqueue(rx_channel_base, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait recv Can Enqueue( timeout\n", id);
        return HAL_TIMEOUT;
    }
    if (HAL_SPIM_TransferDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] xfer timeout\n", id);
        return HAL_TIMEOUT;
    }

    HAL_SPIM_LOCK
    // len == buf bytes
    // word_size ==each word bits
    // wordlen == word nums
    cfg = UDMA_CHANNEL_CFG_EN;
    if (word_size <= 8)
    {
        wordlen = len;
        cfg |= UDMA_CHANNEL_CFG_SIZE_8;
    }
    else if (word_size <= 16)
    {
        wordlen = len / 2;
        cfg |= UDMA_CHANNEL_CFG_SIZE_16;
    }
    else if (word_size <= 32)
    {
        wordlen = len / 4;
        cfg |= UDMA_CHANNEL_CFG_SIZE_32;
    }
    cmd->cmd[0] = g_spim_handle[id]->cfg;
    cmd->cmd[1] = (uint32_t)(SPI_CMD_SOT(g_spim_handle[id]->cs));
    cmd->cmd[3] = (uint32_t)(SPI_CMD_EOT(1, cs_mode == SPIM_CS_KEEP));
    cmd->cmd[2] = SPI_CMD_FUL(wordlen, SPI_CMD_1_WORD_PER_TRANSF, word_size, g_spim_handle[id]->big_endian ? SPI_CMD_MSB_FIRST : SPI_CMD_LSB_FIRST);

    memcpy(spim_tx_buf[id], txdata, len);

    HAL_UDMA_Enqueue(tx_channel_base, (uint32_t)spim_tx_buf[id], len, cfg);
    HAL_UDMA_Enqueue(rx_channel_base, (uint32_t)spim_rx_buf[id], len, cfg);

    cfg = UDMA_CHANNEL_CFG_SIZE_32 | UDMA_CHANNEL_CFG_EN;
    HAL_UDMA_Enqueue(cmd_base, (uint32_t)cmd, 4 * 4, cfg);
    if (HAL_SPIM_TransferDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] xfer timeout\n", id);
        HAL_SPIM_UNLOCK
        return HAL_TIMEOUT;
    }
    memcpy(rxdata, spim_rx_buf[id], len);
    HAL_SPIM_UNLOCK
    return HAL_OK;
}

static HAL_StatusTypeDef do_spim_xfer_it(uint8_t id, const uint8_t *txdata, const uint8_t *rxdata, uint32_t len, SPIM_CS_TypeDef cs_mode, uint8_t word_size)
{
    uint32_t    periph_base     = 0;
    uint32_t    cmd_base        = 0;
    uint32_t    rx_channel_base = 0;
    uint32_t    tx_channel_base = 0;
    spim_cmd_t *cmd             = NULL;
    uint32_t    cfg             = 0;
    uint8_t     periph_id       = 0;
    uint32_t    wordlen         = 0;
    uint32_t    onceTimeout     = 0;
    onceTimeout                 = HAL_MAX_DELAY - 1;

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    cmd = g_cmd[id];
#else
    cmd = &g_cmd[id];
#endif

    periph_id       = g_spim_handle[id]->channel;
    periph_base     = (uint32_t)(HAL_UDMA_Periph_Base(periph_id));
    cmd_base        = periph_base + ARCHI_SPIM_CMD_OFFSET;
    rx_channel_base = periph_base + UDMA_CHANNEL_RX_OFFSET;
    tx_channel_base = periph_base + UDMA_CHANNEL_TX_OFFSET;

    /* Enable interrupts */
    HAL_NVIC_EnableIRQ(SPIM0_0_IRQn + id * 4); // RX
    HAL_NVIC_EnableIRQ(SPIM0_1_IRQn + id * 4); // TX
    HAL_NVIC_EnableIRQ(SPIM0_2_IRQn + id * 4); // CMD
    HAL_NVIC_EnableIRQ(SPIM0_3_IRQn + id * 4); // EOT

    if (HAL_SPIM_WaitCanEnqueue(tx_channel_base, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait send Can Enqueue( timeout\n", id);
        return HAL_TIMEOUT;
    }
    if (HAL_SPIM_WaitCanEnqueue(rx_channel_base, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] Wait recv Can Enqueue( timeout\n", id);
        return HAL_TIMEOUT;
    }
    if (HAL_SPIM_TransferDone(id, &onceTimeout) == HAL_TIMEOUT)
    {
        printf("spim[%d] xfer timeout\n", id);
        return HAL_TIMEOUT;
    }

    HAL_SPIM_LOCK
    // len == buf bytes
    // word_size ==each word bits
    // wordlen == word nums
    cfg = UDMA_CHANNEL_CFG_EN;
    if (word_size <= 8)
    {
        wordlen = len;
        cfg |= UDMA_CHANNEL_CFG_SIZE_8;
    }
    else if (word_size <= 16)
    {
        wordlen = len / 2;
        cfg |= UDMA_CHANNEL_CFG_SIZE_16;
    }
    else if (word_size <= 32)
    {
        wordlen = len / 4;
        cfg |= UDMA_CHANNEL_CFG_SIZE_32;
    }
    cmd->cmd[0] = g_spim_handle[id]->cfg;
    cmd->cmd[1] = (uint32_t)(SPI_CMD_SOT(g_spim_handle[id]->cs));
    cmd->cmd[3] = (uint32_t)(SPI_CMD_EOT(1, cs_mode == SPIM_CS_KEEP));
    cmd->cmd[2] = SPI_CMD_FUL(wordlen, SPI_CMD_1_WORD_PER_TRANSF, word_size, g_spim_handle[id]->big_endian ? SPI_CMD_MSB_FIRST : SPI_CMD_LSB_FIRST);

    memcpy(spim_tx_buf[id], txdata, len);

    g_spim_handle[id]->sending_size = len;
    g_spim_handle[id]->recving_size = len;
    g_spim_handle[id]->recving_addr = (uint8_t *)rxdata;

    HAL_UDMA_Enqueue(rx_channel_base, (uint32_t)spim_rx_buf[id], len, cfg);
    HAL_UDMA_Enqueue(tx_channel_base, (uint32_t)spim_tx_buf[id], len, cfg);

    cfg = UDMA_CHANNEL_CFG_SIZE_32 | UDMA_CHANNEL_CFG_EN;
    HAL_UDMA_Enqueue(cmd_base, (uint32_t)cmd, 4 * 4, cfg);

    HAL_SPIM_UNLOCK
    return HAL_OK;
}

static HAL_StatusTypeDef SPIM_Send_Async(uint8_t id, void *data, uint32_t len, uint32_t *Real_Size, uint32_t qspi, SPIM_CS_TypeDef cs_mode, uint32_t Timeout)
{
    uint32_t bytes_to_sent = 0;
    uint32_t ret           = HAL_OK;
    uint8_t  word_size     = 0;
    uint32_t transmit_len  = 0;
    uint32_t sum_len       = 0;

    bytes_to_sent = len;
    word_size     = g_spim_handle[id]->wordsize;

    while (bytes_to_sent)
    {
        if (bytes_to_sent > SPIM_TX_BUFFER_SIZE)
        {
            transmit_len = SPIM_TX_BUFFER_SIZE;
        }
        else
        {
            transmit_len = bytes_to_sent;
        }

        bytes_to_sent -= transmit_len;

        ret = do_spim_send(id, (uint8_t *)data + sum_len, transmit_len, qspi, cs_mode, word_size, Timeout);
        if (ret == HAL_TIMEOUT)
        {
            return ret;
        }

        sum_len += transmit_len;
    }

    *Real_Size = sum_len;
    return ret;
}

static HAL_StatusTypeDef SPIM_Transfer_Async(uint8_t id, void *tx_data, void *rx_data, uint32_t len, SPIM_CS_TypeDef cs_mode, uint32_t Timeout)
{
    uint32_t bytes_to_transfer = 0;
    uint32_t ret               = HAL_OK;
    uint32_t transfer_len      = 0;
    uint32_t transfer_len_max  = 0;
    uint32_t sum_len           = 0;
    uint8_t  word_size         = 0;

    bytes_to_transfer = len;
    word_size         = g_spim_handle[id]->wordsize;

    transfer_len_max = (SPIM_RX_BUFFER_SIZE < SPIM_TX_BUFFER_SIZE) ? SPIM_RX_BUFFER_SIZE : SPIM_TX_BUFFER_SIZE;

    while (bytes_to_transfer)
    {
        if (bytes_to_transfer > transfer_len_max)
        {
            transfer_len = transfer_len_max;
        }
        else
        {
            transfer_len = bytes_to_transfer;
        }
        bytes_to_transfer -= transfer_len;

        ret = do_spim_xfer(id, tx_data + sum_len, rx_data + sum_len, transfer_len, cs_mode, word_size, Timeout);
        if (ret == HAL_TIMEOUT)
        {
            return ret;
        }
        sum_len += transfer_len;
    }

    return ret;
}

static HAL_StatusTypeDef SPIM_Receive_Async(uint8_t id, void *data, uint32_t len, uint32_t *Real_Size, uint32_t qspi, SPIM_CS_TypeDef cs_mode, uint32_t Timeout)
{
    uint32_t bytes_to_recv = 0;
    uint32_t ret           = HAL_OK;
    uint8_t  word_size     = 0;
    uint32_t receive_len   = 0;
    uint32_t sum_len       = 0;

    bytes_to_recv = len;
    word_size     = g_spim_handle[id]->wordsize;

    while (bytes_to_recv)
    {
        if (bytes_to_recv > SPIM_RX_BUFFER_SIZE)
        {
            receive_len = SPIM_RX_BUFFER_SIZE;
        }
        else
        {
            receive_len = bytes_to_recv;
        }
        bytes_to_recv -= receive_len;

        ret = do_spim_receive(id, data + sum_len, receive_len, qspi, cs_mode, word_size, Timeout);
        if (ret == HAL_TIMEOUT)
        {
            return ret;
        }
        sum_len += receive_len;
    }
    *Real_Size = sum_len;
    return ret;
}

static HAL_StatusTypeDef SPIM_Send_Async_IT(uint8_t id, const uint8_t *data, uint32_t len, uint32_t *Real_Size, uint32_t qspi, SPIM_CS_TypeDef cs_mode)
{
    uint32_t bytes_to_sent = 0;
    uint32_t ret           = HAL_OK;
    uint8_t  word_size     = 0;
    uint32_t transmit_len  = 0;
    uint32_t sum_len       = 0;

    bytes_to_sent = len;
    word_size     = g_spim_handle[id]->wordsize;

    while (bytes_to_sent)
    {
        if (bytes_to_sent > SPIM_TX_BUFFER_SIZE)
        {
            transmit_len = SPIM_TX_BUFFER_SIZE;
        }
        else
        {
            transmit_len = bytes_to_sent;
        }
        bytes_to_sent -= transmit_len;

        ret = do_spim_send_it(id, data + sum_len, transmit_len, qspi, cs_mode, word_size);
        if (ret == HAL_OK)
        {
            *Real_Size += transmit_len;
        }
        else
        {
            return ret;
        }
        sum_len += transmit_len;
    }

    return ret;
}

static HAL_StatusTypeDef SPIM_Receive_Async_IT(uint8_t id, const uint8_t *data, uint32_t len, uint32_t *Real_Size, uint32_t qspi, SPIM_CS_TypeDef cs_mode)
{
    uint32_t bytes_to_recv = 0;
    uint32_t ret           = HAL_OK;
    uint8_t  word_size     = 0;
    uint32_t receive_len   = 0;
    uint32_t sum_len       = 0;

    bytes_to_recv = len;
    word_size     = g_spim_handle[id]->wordsize;

    while (bytes_to_recv)
    {
        if (bytes_to_recv > SPIM_RX_BUFFER_SIZE)
        {
            receive_len = SPIM_RX_BUFFER_SIZE;
        }
        else
        {
            receive_len = bytes_to_recv;
        }
        bytes_to_recv -= receive_len;

        ret = do_spim_receive_it(id, data + sum_len, receive_len, qspi, cs_mode, word_size);
        if (ret == HAL_OK)
        {
            *Real_Size += receive_len;
        }
        else
        {
            return ret;
        }
        sum_len += receive_len;
    }
    return ret;
}

static HAL_StatusTypeDef SPIM_Transfer_Async_IT(uint8_t id, const uint8_t *tx_data, const uint8_t *rx_data, uint32_t len, uint32_t *Real_Size, SPIM_CS_TypeDef cs_mode)
{
    uint32_t bytes_to_transfer = 0;
    uint32_t ret               = HAL_OK;
    uint32_t transfer_len      = 0;
    uint32_t transfer_len_max  = 0;
    uint32_t sum_len           = 0;
    uint8_t  word_size         = 0;

    bytes_to_transfer = len;
    word_size         = g_spim_handle[id]->wordsize;

    transfer_len_max = (SPIM_RX_BUFFER_SIZE < SPIM_TX_BUFFER_SIZE) ? SPIM_RX_BUFFER_SIZE : SPIM_TX_BUFFER_SIZE;

    while (bytes_to_transfer)
    {
        if (bytes_to_transfer > transfer_len_max)
        {
            transfer_len = transfer_len_max;
        }
        else
        {
            transfer_len = bytes_to_transfer;
        }
        bytes_to_transfer -= transfer_len;

        ret = do_spim_xfer_it(id, tx_data + sum_len, rx_data + sum_len, transfer_len, cs_mode, word_size);
        if (ret == HAL_OK)
        {
            *Real_Size += transfer_len;
        }
        else
        {
            return ret;
        }
        sum_len += transfer_len;
    }

    return ret;
}

/** @brief Initialize an SPI master configuration with defined values.
 * @param handle SPIM_HandleTypeDef
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Init(SPIM_HandleTypeDef *handle)
{

    /* Check the parameters */
    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_NOT_ACTIVE(handle->active_flg));
    assert_SPIM_param(IS_SPI_CS(handle->cs));
    assert_SPIM_param(IS_SPI_WORDSIZE(handle->wordsize));
    assert_SPIM_param(IS_SPI_ENDIAN(handle->big_endian));
    assert_SPIM_param(IS_SPI_CPOL(handle->polarity));
    assert_SPIM_param(IS_SPI_CPHA(handle->phase));

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    g_cmd[handle->id]       = IFRAM_CALLOC(sizeof(spim_cmd_t));
    spim_tx_buf[handle->id] = IFRAM_CALLOC(SPIM_TX_BUFFER_SIZE);
    spim_rx_buf[handle->id] = IFRAM_CALLOC(SPIM_RX_BUFFER_SIZE);
    if ((g_cmd[handle->id] == NULL) || (spim_tx_buf[handle->id] == NULL) || (spim_rx_buf[handle->id] == NULL))
    {
        IframMgr_Free(g_cmd[handle->id]);
        IframMgr_Free(spim_tx_buf[handle->id]);
        IframMgr_Free(spim_rx_buf[handle->id]);
        printf("Ifram malloc failed \n");
        return HAL_ERROR;
    }
#endif
    HAL_SPIM_LOCK
    if (g_spim_handle[handle->id] && (handle != g_spim_handle[handle->id]))
    {
        printf("spim[%d] handle was inited \n", handle->id);
        HAL_SPIM_UNLOCK
        return HAL_BUSY;
    }
    g_spim_handle[handle->id] = handle;
    HAL_SPIM_UNLOCK

    handle->active_flg   = 1;
    handle->channel      = ARCHI_UDMA_SPIM_ID(handle->id);
    handle->sending_size = 0;
    handle->recving_size = 0;
    handle->recving_addr = 0;

    if (handle->baudrate == 0)
    {
        handle->baudrate = SPIM_DEFAULT_BAUDRATE;
    }

    handle->div = (uint8_t)SPIM_Get_Div(handle->baudrate);
    handle->cfg = (uint32_t)(SPI_CMD_CFG(handle->div, handle->polarity, handle->phase));

    HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() | (1 << handle->channel));

    HAL_NVIC_ConnectIRQ(SPIM0_0_IRQn + handle->id * 4, SPIM_IRQ_PRIO, SPIM_IRQ_SUB_PRIO, (void *)SPIM_RX_IRQ_Handler, (void *)handle, 1);
    HAL_NVIC_ConnectIRQ(SPIM0_1_IRQn + handle->id * 4, SPIM_IRQ_PRIO, SPIM_IRQ_SUB_PRIO, (void *)SPIM_TX_IRQ_Handler, (void *)handle, 1);
    HAL_NVIC_ConnectIRQ(SPIM0_2_IRQn + handle->id * 4, SPIM_IRQ_PRIO, SPIM_IRQ_SUB_PRIO, (void *)SPIM_CMD_IRQ_Handler, (void *)handle, 1);
    HAL_NVIC_ConnectIRQ(SPIM0_3_IRQn + handle->id * 4, SPIM_IRQ_PRIO, SPIM_IRQ_SUB_PRIO, (void *)SPIM_EOT_IRQ_Handlerr, (void *)handle, 1);
    return HAL_OK;
}

/** @brief Dynamically change the device configuration.
 *
 * This function can be called to change part of the device configuration after
 * it has been opened.
 *
 * @param handle SPIM handle.
 * @param cmd       The command which specifies which parameters of the driver
 * to modify and for some of them also their values.
 * @param wordsize       The wordsize of each word to send
 * @param arg       An additional value which is required for some parameters
 * when they are set.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Control(SPIM_HandleTypeDef *handle, SPIM_ControlTypeDef cmd, SPIM_WordSizeTypeDef wordsize, uint32_t arg)
{
    uint32_t polarity   = (cmd >> SPIM_CTRL_CPOL_BIT) & 3;
    uint32_t phase      = (cmd >> SPIM_CTRL_CPHA_BIT) & 3;
    uint32_t set_freq   = (uint32_t)((cmd >> SPIM_CTRL_SET_BAUDRATE_BIT) & 1);
    uint32_t big_endian = (cmd >> SPIM_CTRL_ENDIANNESS_BIT) & 3;
    uint32_t qspi_en    = (uint32_t)((cmd >> SPIM_CTRL_SET_QSPI_ENABLE_BIT) & 1);
    uint32_t qspi_dis   = (uint32_t)((cmd >> SPIM_CTRL_SET_QSPI_DISABLE_BIT) & 1);

    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_WORDSIZE(wordsize));
    assert_SPIM_param(IS_SPI_ACTIVE(handle->active_flg));

    HAL_SPIM_LOCK
    if (set_freq)
    {
        handle->baudrate = (uint32_t)arg;
        handle->div      = (uint8_t)(SPIM_Get_Div((uint32_t)arg));
    }
    if (polarity)
        handle->polarity = (uint8_t)(polarity >> 1) ? SPI_CMD_CFG_CPOL_POS : SPI_CMD_CFG_CPOL_NEG;
    if (phase)
        handle->phase = (uint8_t)(phase >> 1) ? SPI_CMD_CFG_CPHA_STD : SPI_CMD_CFG_CPHA_OPP;
    if (wordsize)
        handle->wordsize = wordsize;
    if (big_endian)
        handle->big_endian = (uint8_t)(big_endian >> 1) ? SPI_CMD_MSB_FIRST : SPI_CMD_LSB_FIRST;
    if (qspi_en)
        handle->qspi = 1;
    if (qspi_dis)
        handle->qspi = 0;
    handle->cfg = (uint32_t)(SPI_CMD_CFG(handle->div, handle->polarity, handle->phase));
    HAL_SPIM_UNLOCK
    return HAL_OK;
}

/** @brief Close an opened SPI device.
 *
 * This function can be called to close an opened SPI device once it is not
 * needed anymore, in order to free all allocated resources. Once this function
 * is called, the device is not accessible anymore and must be init again before
 * being used. This operation is asynchronous and its termination can be managed
 * through an event.
 *
 * @param handle SPIM handle.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Deinit(SPIM_HandleTypeDef *handle)
{

    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_ACTIVE(handle->active_flg));

    HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() & ~(1 << (handle->channel)));

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    IframMgr_Free(g_cmd[handle->id]);
    IframMgr_Free(spim_tx_buf[handle->id]);
    IframMgr_Free(spim_rx_buf[handle->id]);
#endif
    handle->active_flg        = 0;
    g_spim_handle[handle->id] = NULL;

    return HAL_OK;
}

/** @brief Enqueue a write copy to the SPI (from Chip to SPI device).
 *
 * This function can be used to send data to the SPI device.
 * The copy will make an asynchronous transfer between the SPI and one of the
 * chip memory. This is using classic SPI transfer with MOSI and MISO lines.
 * Note that the event attached to this call is triggered when the chip is ready
 * to send another spi stream not when the spi stream has been fully sent. There
 * can be some buffering effects which make the chip send a few bytes after the
 * event is triggered. Due to hardware constraints, the address of the buffer
 * must be aligned on 4 bytes and the size must be a multiple of 4.
 *
 * @param handle SPIM handle.
 * @param data     The address in the chip where the data to be sent must be
 * read.
 * @param len         The size in bytes of the copy.
 * @param cs_mode     The mode for managing the chip select.
 * @param  Timeout Timeout duration
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Send(SPIM_HandleTypeDef *handle, void *data, uint32_t len, SPIM_CS_TypeDef cs_mode, uint32_t Timeout)
{
    uint32_t Real_Size;
    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_ACTIVE(handle->active_flg));

    // check the byte len according to the wordsize
    if ((data == NULL) || SPIM_CHECKLEN(handle->wordsize, len))
        return HAL_ERROR;

    return SPIM_Send_Async(handle->id, data, len, &Real_Size, handle->qspi, cs_mode, Timeout);
}

/** @brief Enqueue a write copy to the SPI (from Chip to SPI device).
 *
 * This function can be used to send data to the SPI device.
 * The copy will make an asynchronous transfer between the SPI and one of the
 * chip memory. This is using classic SPI transfer with MOSI and MISO lines.
 * Note that the event attached to this call is triggered when the chip is ready
 * to send another spi stream not when the spi stream has been fully sent. There
 * can be some buffering effects which make the chip send a few bytes after the
 * event is triggered. Due to hardware constraints, the address of the buffer
 * must be aligned on 4 bytes and the size must be a multiple of 4.
 *
 * @param handle SPIM handle.
 * @param data     The address in the chip where the data to be sent must be
 * read.
 * @param len         The size in bytes of the copy.
 * @param  Real_Size Using a pointer to track the real send bytes
 * @param cs_mode     The mode for managing the chip select.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Send_IT(SPIM_HandleTypeDef *handle, const uint8_t *data, uint32_t len, uint32_t *Real_Size, SPIM_CS_TypeDef cs_mode)
{
    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_ACTIVE(handle->active_flg));

    // check the byte len according to the wordsize
    if ((data == NULL) || SPIM_CHECKLEN(handle->wordsize, len))
        return HAL_ERROR;

    return SPIM_Send_Async_IT(handle->id, data, len, Real_Size, handle->qspi, cs_mode);
}

/** @brief Enqueue a read copy to the SPI (from Chip to SPI device).
 *
 * This function can be used to receive data from the SPI device.
 * The copy will make an asynchronous transfer between the SPI and one of the
 * chip memory. This is using classic SPI transfer with MOSI and MISO lines. Due
 * to hardware constraints, the address of the buffer must be aligned on 4 bytes
 * and the size must be a multiple of 4.
 * @param handle SPIM id.
 * @param data        The address in the chip where the received data must be
 * written.
 * @param len         The size in bytes of the copy.
 * @param cs_mode     The mode for managing the chip select.
 * @param  Timeout Timeout duration
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Receive(SPIM_HandleTypeDef *handle, void *data, uint32_t len, SPIM_CS_TypeDef cs_mode, uint32_t Timeout)
{
    uint32_t Real_Size;
    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_ACTIVE(handle->active_flg));

    // check the byte len according to the wordsize
    if ((data == NULL) || SPIM_CHECKLEN(handle->wordsize, len))
        return HAL_ERROR;

    return SPIM_Receive_Async(handle->id, data, len, &Real_Size, handle->qspi, cs_mode, Timeout);
}

/** @brief Enqueue a read copy to the SPI in interrupt mode (from Chip to SPI
 * device).
 *
 * This function can be used to receive data from the SPI device.
 * The copy will make an asynchronous transfer between the SPI and one of the
 * chip memory. This is using classic SPI transfer with MOSI and MISO lines. Due
 * to hardware constraints, the address of the buffer must be aligned on 4 bytes
 * and the size must be a multiple of 4.
 * @param handle SPIM handle.
 * @param data        The address in the chip where the received data must be
 * written.
 * @param len         The size in bytes of the copy.
 * @param Real_Size   The read size can be read.
 * @param cs_mode     The mode for managing the chip select.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Receive_IT(SPIM_HandleTypeDef *handle, uint8_t *data, uint32_t len, uint32_t *Real_Size, SPIM_CS_TypeDef cs_mode)
{
    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_ACTIVE(handle->active_flg));

    // check the byte len according to the wordsize
    if ((data == NULL) || SPIM_CHECKLEN(handle->wordsize, len))
        return HAL_ERROR;

    return SPIM_Receive_Async_IT(handle->id, data, len, Real_Size, handle->qspi, cs_mode);
}

/** @brief Enqueue a read and write copy to the SPI (using full duplex mode).
 *
 * This function can be used to send and receive data with the SPI device using
 * full duplex mode. The copy will make an asynchronous transfer between the SPI
 * and one of the chip memory. An event can be specified in order to be notified
 * when the transfer is finished. This is using classic SPI transfer with MOSI
 * and MISO lines. Due to hardware constraints, the address of the buffer must
 * be aligned on 4 bytes and the size must be a multiple of 4.
 * @param handle SPIM handle.
 * @param tx_data     The address in the chip where the data to be sent must be
 * read.
 * @param rx_data     The address in the chip where the received data must be
 * written.
 * @param len         The size in bytes of the copy.
 * @param cs_mode     The mode for managing the chip select.
 * @param  Timeout Timeout duration
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Transfer(SPIM_HandleTypeDef *handle, void *tx_data, void *rx_data, uint16_t len, SPIM_CS_TypeDef cs_mode, uint32_t Timeout)
{
    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_ACTIVE(handle->active_flg));

    if ((tx_data == NULL) || (rx_data == NULL) || SPIM_CHECKLEN(handle->wordsize, len))
        return HAL_ERROR;

    return SPIM_Transfer_Async(handle->id, tx_data, rx_data, len, cs_mode, Timeout);
}

/** @brief Enqueue a read and write copy to the SPI (using full duplex mode).
 *
 * This function can be used to send and receive data with the SPI device using
 * full duplex mode. The copy will make an asynchronous transfer between the SPI
 * and one of the chip memory. An event can be specified in order to be notified
 * when the transfer is finished. This is using classic SPI transfer with MOSI
 * and MISO lines. Due to hardware constraints, the address of the buffer must
 * be aligned on 4 bytes and the size must be a multiple of 4.
 * @param handle SPIM handle.
 * @param tx_data     The address in the chip where the data to be sent must be
 * read.
 * @param rx_data     The address in the chip where the received data must be
 * written.
 * @param  Real_Size Using a pointer to track the real transfer bytes
 * @param len         The size in bytes of the copy.
 * @param cs_mode     The mode for managing the chip select.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_SPIM_Transfer_IT(SPIM_HandleTypeDef *handle, const uint8_t *tx_data, uint8_t *rx_data, uint32_t len, uint32_t *Real_Size, SPIM_CS_TypeDef cs_mode)
{
    assert_SPIM_param(IS_SPI_HANDLE(handle));
    assert_SPIM_param(IS_SPI_ID(handle->id));
    assert_SPIM_param(IS_SPI_ACTIVE(handle->active_flg));

    if ((tx_data == NULL) || (rx_data == NULL) || SPIM_CHECKLEN(handle->wordsize, len))
        return HAL_ERROR;

    return SPIM_Transfer_Async_IT(handle->id, tx_data, rx_data, len, Real_Size, cs_mode);
}

#endif /* HAL_SPIM_MODULE_ENABLED */
