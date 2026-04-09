/**
 ******************************************************************************
 * @file    daric_hal_i2c.c
 * @author  I2C Team
 * @brief   I2C HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of I2C
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
#include <string.h>
#include <inttypes.h>

#include "daric_hal.h"
#include "daric_hal_i2c.h"
#include "daric_hal_nvic.h"
#include "daric_hal_udma_v3.h"

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
#include "daric_ifram.h"
#endif

/// @cond PRIVATE_OUTPUT
/* Private macro -------------------------------------------------------------*/
/* I2C register base address */
#define I2C_MAX_NUM 4
/* I2C buffer size */
#define I2C_BUF_SIZE 128

/* BUSY bit is not correct in DARIC NTO */
#define I2C_DEBUG_DISABLE_STATUS_BUSY_BIT (0x1)
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
#define I2C_STATUS_BUSY_MASK (0x1)
#endif
#define I2C_STATUS_ARB_LOST_MASK (0x2)
#define I2C_STATUS_ACK_NACK_MASK (0x4)

#define I2C_CMD_START     0x0UL
#define I2C_CMD_STOP      0x2UL
#define I2C_CMD_RD_ACK    0x4UL
#define I2C_CMD_RD_NACK   0x6UL
#define I2C_CMD_WR        0x8UL
#define I2C_CMD_WAIT      0xAUL
#define I2C_CMD_RPT       0xCUL
#define I2C_CMD_CFG       0xEUL
#define I2C_CMD_WAIT_EV   0x1UL
#define I2C_CMD_SETUP_UCA 0x3UL
#define I2C_CMD_SETUP_UCS 0x5UL
#define I2C_CMD_WRB       0x7UL
#define I2C_CMD_EOT       0x9UL

#define I2C_CMD_RPT_MAX 0xff

#define I2C_CMD_SHIFT  28
#define i2c_cmd_start  (I2C_CMD_START << I2C_CMD_SHIFT)
#define i2c_cmd_stop   (I2C_CMD_STOP << I2C_CMD_SHIFT)
#define i2c_cmd_eot    (I2C_CMD_EOT << I2C_CMD_SHIFT)
#define i2c_cmd_rdack  (I2C_CMD_RD_ACK << I2C_CMD_SHIFT)
#define i2c_cmd_rdnack (I2C_CMD_RD_NACK << I2C_CMD_SHIFT)
#define i2c_cmd_wr     (I2C_CMD_WR << I2C_CMD_SHIFT)

#define i2c_cmd_wait(cycles)         ((I2C_CMD_WAIT << I2C_CMD_SHIFT) | ((cycles) & 0xffff))
#define i2c_cmd_repeat(loops)        ((I2C_CMD_RPT << I2C_CMD_SHIFT) | ((loops) & 0xffff))
#define i2c_cmd_wrb(b)               ((I2C_CMD_WRB << I2C_CMD_SHIFT) | ((b) & 0xff))
#define i2c_cmd_cfg(cfg)             ((I2C_CMD_CFG << I2C_CMD_SHIFT) | ((cfg) & 0xffff))
#define i2c_cmd_setup_uca(tx1, addr) ((I2C_CMD_CFG << I2C_CMD_SHIFT) | ((tx1) << 27) | ((addr) & 0xffffff))
#define i2c_cmd_setup_ucs(tx1, size) ((I2C_CMD_CFG << I2C_CMD_SHIFT) | ((tx1) << 27) | ((size) & 0xffff))
#define i2c_cmd_wait_ev(evntid)      ((I2C_CMD_WAIT_EV << I2C_CMD_SHIFT) | (evntid))

#define i2c_mem_add_msb(addr) ((uint8_t)((uint16_t)(((uint16_t)((addr) & (uint16_t)(0xFF00U))) >> 8U)))
#define i2c_mem_add_lsb(addr) ((uint8_t)((uint16_t)((addr) & (uint16_t)(0x00FFU))))

#define I2C_SETUP_RESET 0x01

#define I2C_CH_CFG_CLEAR (1 << 6)
#define I2C_CH_CFG_EN    (1 << 4)
#define I2C_CH_CFG_CONT  (1 << 0)

/* DMA RX channel receives data full */
#define I2C_RX_CH_EVT(id) (I2C0_0_IRQn + (id << 2))
/* DMA TX channel finish sending data */
#define I2C_TX_CH_EVT(id) (I2C0_0_IRQn + (id << 2) + 1)
/* CMD event */
#define I2C_CMD_CH_EVT(id) (I2C0_0_IRQn + (id << 2) + 2)
/* Eot event */
#define I2C_EOT_CH_EVT(id) (I2C0_0_IRQn + (id << 2) + 3)
/* NACK Evt */
#define I2C_NACK_EVT(id) (184 + id)
/* Error Evt */
#define I2C_ERROR_EVT(id) (188 + id)

#define HAL_I2C_LOCK()   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_I2C_UNLOCK() HAL_UNLOCK
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
typedef enum
{
    I2C_TYPE_CMD = 1,
    I2C_TYPE_TX,
    I2C_TYPE_RX,
} I2C_TranTypeDef;

typedef struct
{
    volatile uint32_t rx_addr; /* RX transfer address of associated buffer */
    volatile uint32_t rx_size; /* RX transfer size of buffer */
    volatile uint32_t rx_cfg;  /* RX transfer configuration */
    volatile uint32_t dummy_1;
    volatile uint32_t tx_addr; /* TX transfer address of associated buffer */
    volatile uint32_t tx_size; /* TX transfer size of buffer */
    volatile uint32_t tx_cfg;  /* TX transfer configuration */
    volatile uint32_t dummy_2;
    volatile uint32_t cmd_addr; /* CMD transfer address of associated buffer */
    volatile uint32_t cmd_size; /* CMD transfer size of buffer */
    volatile uint32_t cmd_cfg;  /* CMD transfer configuration */
    volatile uint32_t dummy_3;
    volatile uint32_t status; /* I2C Status register */
    volatile uint32_t setup;  /* I2C Configuration register */
} I2C_RegTypeDef;

/* I2C IRQ Priority define */
#ifdef HAL_I2C_IRQ_PRIO
#define I2C_IRQ_PRIO HAL_I2C_IRQ_PRIO
#else
#define I2C_IRQ_PRIO 0
#endif

#ifdef HAL_I2C_IRQ_SUB_PRIO
#define I2C_IRQ_SUB_PRIO HAL_I2C_IRQ_SUB_PRIO
#else
#define I2C_IRQ_SUB_PRIO 0
#endif
/* Private functions ---------------------------------------------------------*/
/**
 * Functions definition for IRQ process
 */
static void HAL_I2C_RxIRQHandler(const void *arg);
static void HAL_I2C_TxIRQHandler(const void *arg);
static void HAL_I2C_CmdIRQHandler(const void *arg);
static void HAL_I2C_EotIRQHandler(const void *arg);
static void HAL_I2C_NackIRQHandler(const void *arg);
static void HAL_I2C_ErrorIRQHandler(const void *arg);

/* Private variables ---------------------------------------------------------*/
static uint32_t daric_i2c_base[I2C_MAX_NUM] = { 0x50109000, 0x5010A000, 0x5010B000, 0x5010C000 };
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
static uint32_t *i2c_cmd_buff[I2C_MAX_NUM] = { 0 };
static uint32_t *i2c_rx_buff[I2C_MAX_NUM]  = { 0 };
static uint32_t *i2c_tx_buff[I2C_MAX_NUM]  = { 0 };
#else
static uint32_t i2c_cmd_buff[I2C_MAX_NUM][I2C_BUF_SIZE] __attribute__((section("ifram")));
static uint8_t  i2c_rx_buff[I2C_MAX_NUM][I2C_BUF_SIZE] __attribute__((section("ifram")));
static uint8_t  i2c_tx_buff[I2C_MAX_NUM][I2C_BUF_SIZE] __attribute__((section("ifram")));
#endif

typedef struct
{
    HAL_I2C_StateTypeDef i2c_status;  /* state of I2Cx component */
    uint32_t             usage_count; /* number of init of the I2Cx component */
} I2C_Component;

static I2C_Component i2c_component[I2C_MAX_NUM] = { 0 };

uint32_t i2c_get_div(uint32_t i2cFreqKHz)
{
    uint32_t div;
#if defined(CONFIG_SOC_DARIC_NTO_A)
    volatile double sysCoreClkKHz;
    sysCoreClkKHz = HAL_GetCoreClkMHz() * 1000;
    if (sysCoreClkKHz < 16000)
    {
        div = 48000 * ((double)((DARIC_CGU->fdper & 0xff) + 1) / 0x100) / 9 / i2cFreqKHz - 9;
    }
    else
    {
        /*sysKHz                   fdper divider                        magicdiv     kHz   magic_offset*/
        div = sysCoreClkKHz * ((double)((DARIC_CGU->fdper & 0xff) + 1) / 0x100) / 9 / i2cFreqKHz - 9;
        if (div > 0xffff)
        {
            div = 0xffff;
        }
    }
#else
    div = DARIC_CGU->cgufsfreq3 * 1000 / 4 / i2cFreqKHz;
#endif

#if defined(CONFIG_I2C_FIXED_DIV_TABLE)
    /*
     * To avoid the issue of inaccurate I2C frequency calculations,
     * provide three fixed configurations at a 700 MHz main frequency.
     */
    if (i2cFreqKHz == 400)
    {
        div = 46;
    }
    else if (i2cFreqKHz == 200)
    {
        div = 93;
    }
    else
    {
        div = 192;
    }
#endif
    return div;
}

static bool i2c_rx_is_finish(I2C_RegTypeDef *i2c)
{
    if (i2c->rx_addr != 0)
    {
        return false;
    }

    if (i2c->rx_size != 0)
    {
        return false;
    }

    if (i2c->rx_cfg & I2C_CH_CFG_EN)
    {
        return false;
    }

    return true;
}

static bool i2c_tx_is_finish(I2C_RegTypeDef *i2c)
{
    if (i2c->tx_addr != 0)
    {
        return false;
    }

    if (i2c->tx_size != 0)
    {
        return false;
    }

    if (i2c->tx_cfg & I2C_CH_CFG_EN)
    {
        return false;
    }

    return true;
}

static bool i2c_cmd_is_finish(I2C_RegTypeDef *i2c)
{
    if (i2c->cmd_addr != 0)
    {
        return false;
    }

    if (i2c->cmd_size != 0)
    {
        return false;
    }

    if (i2c->cmd_cfg & I2C_CH_CFG_EN)
    {
        return false;
    }

    return true;
}

/* Private function prototypes -----------------------------------------------*/
static int i2c_wait_trans_done(I2C_RegTypeDef *i2c, I2C_TranTypeDef type, uint32_t start_time, uint32_t timeout)
{
    bool is_finish = false;

    while (1)
    {
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
        /* 1. Check if I2C status is BUSY or not */
        if (0 == (i2c->status & I2C_STATUS_BUSY_MASK))
        {
            /* 2. Check if I2C CMD is finish or not */
            is_finish = i2c_cmd_is_finish(i2c);
            if (is_finish == true)
            {
                if (type == I2C_TYPE_CMD)
                {
                    break;
                }
                else
                {
                    is_finish = false;
                    /* 3. Check if I2C TX/RX is finish or not */
                    if (type == I2C_TYPE_TX)
                    {
                        is_finish = i2c_tx_is_finish(i2c);
                    }
                    else if (type == I2C_TYPE_RX)
                    {
                        is_finish = i2c_rx_is_finish(i2c);
                    }
                    if (is_finish == true)
                    {
                        break;
                    }
                }
            }
        }
        else if (i2c->status & I2C_STATUS_ACK_NACK_MASK)
        {
            printf("%s: slave is nack!\n", __func__);
            return -1;
        }
#else
        /* 2. Check if I2C CMD is finish or not */
        is_finish = i2c_cmd_is_finish(i2c);
        if (is_finish == true)
        {
            if (type == I2C_TYPE_CMD)
            {
                break;
            }
            else
            {
                is_finish = false;
                /* 3. Check if I2C TX/RX is finish or not */
                if (type == I2C_TYPE_TX)
                {
                    is_finish = i2c_tx_is_finish(i2c);
                }
                else if (type == I2C_TYPE_RX)
                {
                    is_finish = i2c_rx_is_finish(i2c);
                }
                if (is_finish == true)
                {
                    break;
                }
            }
        }
#endif

        if ((HAL_GetMs() - start_time) > timeout)
        {
            printf("%s: timeout!\n", __func__);
            return -1;
        }
    }

    return 0;
}
/// @endcond

/* Exported functions --------------------------------------------------------*/
/**
 * @brief  Initializes the I2C peripheral according to the specified parameters.
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * initialization process.
 *
 * This function configures the I2C peripheral according to the parameters
 * specified in the `I2C_HandleTypeDef` structure, including timing, addressing
 * mode, and other relevant settings.
 */
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    I2C_Component *comp = &i2c_component[hi2c->instance_id];
    HAL_I2C_LOCK();
    if (hi2c->state != HAL_I2C_STATE_RESET)
    {
        HAL_I2C_UNLOCK();
        printf("%s: hi2c%" PRIu32 " is already inited\n", __func__, hi2c->instance_id);
        return HAL_OK;
    }

    if (comp->usage_count == 0 && comp->i2c_status == HAL_I2C_STATE_RESET)
    {
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
        i2c_cmd_buff[hi2c->instance_id] = IFRAM_CALLOC(I2C_BUF_SIZE);
        i2c_tx_buff[hi2c->instance_id]  = IFRAM_CALLOC(I2C_BUF_SIZE);
        i2c_rx_buff[hi2c->instance_id]  = IFRAM_CALLOC(I2C_BUF_SIZE);
        if (i2c_cmd_buff[hi2c->instance_id] == NULL || i2c_tx_buff[hi2c->instance_id] == NULL || i2c_rx_buff[hi2c->instance_id] == NULL)
        {
            if (i2c_cmd_buff[hi2c->instance_id] != NULL)
            {
                IframMgr_Free(i2c_cmd_buff[hi2c->instance_id]);
                i2c_cmd_buff[hi2c->instance_id] = NULL;
            }
            if (i2c_tx_buff[hi2c->instance_id] != NULL)
            {
                IframMgr_Free(i2c_tx_buff[hi2c->instance_id]);
                i2c_tx_buff[hi2c->instance_id] = NULL;
            }
            if (i2c_rx_buff[hi2c->instance_id] != NULL)
            {
                IframMgr_Free(i2c_rx_buff[hi2c->instance_id]);
                i2c_rx_buff[hi2c->instance_id] = NULL;
            }
            printf("%s: hi2c%" PRIu32 " malloc ifram fail!\n", __func__, hi2c->instance_id);
            hi2c->state = HAL_I2C_STATE_RESET;
            HAL_I2C_UNLOCK();
            return HAL_ERROR;
        }
#endif
        /* config uDMA clock gate for I2c: CTRL_CFG_CG
         * Bit 8  IIC0 (R/W)      Defines uDMA peripherals clock gate configuration
         * for IIC0 Bit 9  IIC1 (R/W)      Defines uDMA peripherals clock gate
         * configuration for IIC1 Bit 10  IIC2 (R/W)     Defines uDMA peripherals
         * clock gate configuration for IIC2 Bit 11  IIC3 (R/W)     Defines uDMA
         * peripherals clock gate configuration for IIC3
         */
        HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() | (1 << (8 + hi2c->instance_id)));

        /* register i2c interrupts */
        HAL_NVIC_ConnectIRQ(I2C_RX_CH_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_RxIRQHandler, (void *)hi2c, 0);
        HAL_NVIC_ConnectIRQ(I2C_TX_CH_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_TxIRQHandler, (void *)hi2c, 0);
        HAL_NVIC_ConnectIRQ(I2C_CMD_CH_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_CmdIRQHandler, (void *)hi2c, 0);
        HAL_NVIC_ConnectIRQ(I2C_EOT_CH_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_EotIRQHandler, (void *)hi2c, 0);

        /* enable nack & error interrupt */
        HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
        HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

        comp->i2c_status = HAL_I2C_STATE_READY;
    }
    comp->usage_count++;
    hi2c->init.divider = i2c_get_div(hi2c->init.baudrate);
    hi2c->state        = HAL_I2C_STATE_READY;

    /* initialize internal buffer */
    if (!hi2c->init.cmd_buf)
    {
        hi2c->init.cmd_buf  = (uint32_t)(i2c_cmd_buff[hi2c->instance_id]);
        hi2c->init.cmd_size = I2C_BUF_SIZE;
    }
    if (!hi2c->init.tx_buf)
    {
        hi2c->init.tx_buf  = (uint32_t)(i2c_tx_buff[hi2c->instance_id]);
        hi2c->init.tx_size = I2C_BUF_SIZE;
    }
    if (!hi2c->init.rx_buf)
    {
        hi2c->init.rx_buf  = (uint32_t)(i2c_rx_buff[hi2c->instance_id]);
        hi2c->init.rx_size = I2C_BUF_SIZE;
    }

    printf("%s: hi2c initialized, id = %" PRIu32 "\n", __func__, hi2c->instance_id);
    HAL_I2C_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Deinitializes the I2C peripheral.
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * deinitialization process.
 *
 * This function deinitializes the I2C peripheral, resetting it to its default
 * state and freeing any resources that were allocated during initialization.
 */
HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c)
{
    I2C_RegTypeDef *i2c_reg = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    I2C_Component *comp = &i2c_component[hi2c->instance_id];
    HAL_I2C_LOCK();

    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        HAL_I2C_UNLOCK();
        printf("%s: hi2c%" PRIu32 " transfer have not finish!\n", __func__, hi2c->instance_id);
        return HAL_BUSY;
    }

    hi2c->state = HAL_I2C_STATE_BUSY;
    printf("comp->usage_count = %" PRIu32 "\n", comp->usage_count);
    if (comp->usage_count > 0)
    {
        if (comp->usage_count == 1)
        {
            if (comp->i2c_status != HAL_I2C_STATE_READY)
            {
                printf("%s: i2c%" PRIu32 " status  is not ready!\n", __func__, hi2c->instance_id);
                HAL_I2C_UNLOCK();
                return HAL_BUSY;
            }

            i2c_reg = (I2C_RegTypeDef *)daric_i2c_base[hi2c->instance_id];

            HAL_NVIC_DisableIRQ(I2C_NACK_EVT(hi2c->instance_id));
            HAL_NVIC_DisableIRQ(I2C_ERROR_EVT(hi2c->instance_id));

            /* stop transfer: disable RX & TX transfer */
            i2c_reg->tx_cfg |= I2C_CH_CFG_CLEAR;
            i2c_reg->tx_cfg &= ~I2C_CH_CFG_EN;
            i2c_reg->tx_cfg &= ~I2C_CH_CFG_CONT;
            i2c_reg->rx_cfg |= I2C_CH_CFG_CLEAR;
            i2c_reg->rx_cfg &= ~I2C_CH_CFG_EN;
            i2c_reg->rx_cfg &= ~I2C_CH_CFG_CONT;
            /* stop cmd transfer */
            i2c_reg->cmd_cfg |= I2C_CH_CFG_CLEAR;
            i2c_reg->cmd_cfg &= ~I2C_CH_CFG_EN;
            i2c_reg->cmd_cfg &= ~I2C_CH_CFG_CONT;

            /* clear uDMA clock gate for I2c: CTRL_CFG_CG
             * Bit 8  IIC0 (R/W)      Defines uDMA peripherals clock gate
             * configuration for IIC0 Bit 9  IIC1 (R/W)      Defines uDMA peripherals
             * clock gate configuration for IIC1 Bit 10  IIC2 (R/W)     Defines uDMA
             * peripherals clock gate configuration for IIC2 Bit 11  IIC3 (R/W)
             * Defines uDMA peripherals clock gate configuration for IIC3
             */
            HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() & (~(1 << (8 + hi2c->instance_id))));

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
            if (i2c_cmd_buff[hi2c->instance_id] != NULL)
            {
                IframMgr_Free(i2c_cmd_buff[hi2c->instance_id]);
                i2c_cmd_buff[hi2c->instance_id] = NULL;
            }
            if (i2c_tx_buff[hi2c->instance_id] != NULL)
            {
                IframMgr_Free(i2c_tx_buff[hi2c->instance_id]);
                i2c_tx_buff[hi2c->instance_id] = NULL;
            }
            if (i2c_rx_buff[hi2c->instance_id] != NULL)
            {
                IframMgr_Free(i2c_rx_buff[hi2c->instance_id]);
                i2c_rx_buff[hi2c->instance_id] = NULL;
            }
#endif
            comp->i2c_status = HAL_I2C_STATE_RESET;
        }
        comp->usage_count--;
    }
    hi2c->state = HAL_I2C_STATE_RESET;
    HAL_I2C_UNLOCK();

    printf("%s: hi2c deinitialized\n", __func__);

    return HAL_OK;
}

/**
 * @brief  Registers a callback function for a specific I2C event.
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  CallbackID Specifies the callback ID to be registered.
 * @param  pCallback Pointer to the callback function.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * registration process.
 *
 * This function registers a user-defined callback function for a specific I2C
 * event, allowing the application to handle events such as data transmission or
 * reception completion.
 */
HAL_StatusTypeDef HAL_I2C_RegisterCallback(I2C_HandleTypeDef *hi2c, HAL_I2C_CallbackIDTypeDef CallbackID, I2C_CallbackTypeDef pCallback)
{
    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();

    switch (CallbackID)
    {
        case HAL_I2C_TX_COMPLETE_CB_ID:
            hi2c->TxCpltCallback = pCallback;
            break;
        case HAL_I2C_RX_COMPLETE_CB_ID:
            hi2c->RxCpltCallback = pCallback;
            break;
        case HAL_I2C_MEM_TX_COMPLETE_CB_ID:
            hi2c->MemTxCpltCallback = pCallback;
            break;
        case HAL_I2C_MEM_RX_COMPLETE_CB_ID:
            hi2c->MemRxCpltCallback = pCallback;
            break;
        case HAL_I2C_NACK_CB_ID:
            hi2c->NackCallback = pCallback;
            break;
        case HAL_I2C_ERROR_CB_ID:
            hi2c->ErrorCallback = pCallback;
            break;
        default:
            printf("%s: invalid callback index\n", __func__);
            hi2c->error_code = HAL_I2C_ERR_INVALID_PARAM;
            HAL_I2C_UNLOCK();
            return HAL_ERROR;
    }

    HAL_I2C_UNLOCK();

    // printf("%s: callback registered, id: %d\n", __func__, CallbackID);

    return HAL_OK;
}

/**
 * @brief  Unregisters a callback function for a specific I2C event.
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  CallbackID Specifies the callback ID to be unregistered.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * unregistration process.
 *
 * This function unregisters a previously registered callback function for a
 * specific I2C event, disabling the application's handling of that event.
 */
HAL_StatusTypeDef HAL_I2C_UnRegisterCallback(I2C_HandleTypeDef *hi2c, HAL_I2C_CallbackIDTypeDef CallbackID)
{
    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();

    switch (CallbackID)
    {
        case HAL_I2C_TX_COMPLETE_CB_ID:
            hi2c->TxCpltCallback = 0;
            break;
        case HAL_I2C_RX_COMPLETE_CB_ID:
            hi2c->RxCpltCallback = 0;
            break;
        case HAL_I2C_MEM_TX_COMPLETE_CB_ID:
            hi2c->MemTxCpltCallback = 0;
            break;
        case HAL_I2C_MEM_RX_COMPLETE_CB_ID:
            hi2c->MemRxCpltCallback = 0;
            break;
        case HAL_I2C_NACK_CB_ID:
            hi2c->NackCallback = 0;
            break;
        case HAL_I2C_ERROR_CB_ID:
            hi2c->ErrorCallback = 0;
            break;
        default:
            printf("%s: invalid callback index\n", __func__);
            hi2c->error_code = HAL_I2C_ERR_INVALID_PARAM;
            HAL_I2C_UNLOCK();
            return HAL_ERROR;
    }

    HAL_I2C_UNLOCK();

    printf("%s: callback unregistered, id: %d\n", __func__, CallbackID);

    return HAL_OK;
}

/**
 * @brief  Transmits data in master mode via I2C (blocking mode).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  pData Pointer to the data buffer to be transmitted.
 * @param  Size Amount of data to be sent.
 * @param  Timeout Duration for blocking until transmission is complete.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * transmission.
 *
 * This function sends data to the specified slave device in a blocking mode,
 * where the CPU waits until the operation is complete or a timeout occurs.
 */
HAL_StatusTypeDef HAL_I2C_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint32_t Size, uint32_t Timeout)
{
    I2C_RegTypeDef *i2c_reg    = 0;
    uint32_t        seq_idx    = 0;
    uint32_t       *cmd_buf    = 0;
    uint8_t        *tx_buf     = 0;
    uint32_t        start_time = 0;
    uint32_t        tmp_repeat = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();

    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif

    /* enable tx continuous mode of dma */
    // i2c_reg->tx_cfg |= I2C_CH_CFG_CONT;
    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_TX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_TX;
    hi2c->mode                                  = HAL_I2C_MODE_MASTER_WRITE;

    HAL_I2C_UNLOCK();

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(I2C_TX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_CMD_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_EOT_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);
    tx_buf  = (uint8_t *)(hi2c->init.tx_buf);

#if defined(CONFIG_SOC_DARIC_MPW_A)
    HAL_UDMA_clr((uint32_t) & (i2c_reg->tx_addr));
    HAL_UDMA_clr((uint32_t) & (i2c_reg->cmd_addr));
#elif defined(CONFIG_SOC_DARIC_NTO_A)
    HAL_UDMA_RESET_Set(HAL_UDMA_RESET_Get() & (~(1 << (8 + hi2c->instance_id))));
#endif

    cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
    cmd_buf[seq_idx++] = i2c_cmd_start;
    cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) & 0xFE));
    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);

    start_time = HAL_GetMs();
    while (Size)
    {
        if (Size > I2C_BUF_SIZE)
        {
            tmp_repeat = I2C_BUF_SIZE;
        }
        else
        {
            tmp_repeat = Size;
        }

        seq_idx            = 0;
        cmd_buf[seq_idx++] = i2c_cmd_repeat(tmp_repeat);
        cmd_buf[seq_idx++] = i2c_cmd_wr;

        memcpy(tx_buf, pData, tmp_repeat);

        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->tx_addr), (int)tx_buf, tmp_repeat, UDMA_CHANNEL_CFG_EN);
        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);
        if (i2c_wait_trans_done(i2c_reg, I2C_TYPE_TX, start_time, Timeout))
        {
            printf("%s: hi2c tx timeout\n", __func__);
            cmd_buf[0] = i2c_cmd_stop;
            HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, 1 * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);

            i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
            hi2c->state                                 = HAL_I2C_STATE_READY;
            hi2c->error_code                            = HAL_I2C_ERR_TIMEOUT;
            return HAL_TIMEOUT;
        }

        Size -= tmp_repeat;
        pData += tmp_repeat;
    }
    cmd_buf[0] = i2c_cmd_stop;
    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, 1 * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
    hi2c->state                                 = HAL_I2C_STATE_READY;
    hi2c->error_code                            = HAL_I2C_ERR_NONE;

    printf("%s: transmit done!\n", __func__);

    return HAL_OK;
}

/**
 * @brief  Receives data in master mode via I2C (blocking mode).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  pData Pointer to the data buffer to store received data.
 * @param  Size Amount of data to be received.
 * @param  Timeout Duration for blocking until reception is complete.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * reception.
 *
 * This function reads data from the specified slave device in a blocking mode,
 * where the CPU waits until the operation is complete or a timeout occurs.
 */
HAL_StatusTypeDef HAL_I2C_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint32_t Size, uint32_t Timeout)
{

    I2C_RegTypeDef *i2c_reg    = 0;
    uint32_t        seq_idx    = 0;
    uint32_t       *cmd_buf    = 0;
    uint32_t        start_time = 0;
    uint8_t        *rx_buf     = 0;
    uint32_t        tmp_repeat = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();

    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(I2C_TX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_CMD_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_EOT_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_RX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_RX;
    hi2c->mode                                  = HAL_I2C_MODE_MASTER_READ;

    HAL_I2C_UNLOCK();

    cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);
    rx_buf  = (uint8_t *)(hi2c->init.rx_buf);

#if defined(CONFIG_SOC_DARIC_MPW_A)
    HAL_UDMA_clr((uint32_t) & (i2c_reg->rx_addr));
    HAL_UDMA_clr((uint32_t) & (i2c_reg->cmd_addr));
#elif defined(CONFIG_SOC_DARIC_NTO_A)
    HAL_UDMA_RESET_Set(HAL_UDMA_RESET_Get() & (~(1 << (8 + hi2c->instance_id))));
#endif

    while (Size)
    {
        if (Size > I2C_BUF_SIZE)
        {
            tmp_repeat = I2C_BUF_SIZE;
        }
        else
        {
            tmp_repeat = Size;
        }
        cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
        cmd_buf[seq_idx++] = i2c_cmd_start;
        cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) | 0x1));
        if (tmp_repeat > 1)
        {
            cmd_buf[seq_idx++] = i2c_cmd_repeat(tmp_repeat - 1);
            cmd_buf[seq_idx++] = i2c_cmd_rdack;
        }
        cmd_buf[seq_idx++] = i2c_cmd_rdnack;
        cmd_buf[seq_idx++] = i2c_cmd_stop;

        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->rx_addr), (int)rx_buf, tmp_repeat, UDMA_CHANNEL_CFG_EN);
        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);
        start_time = HAL_GetMs();
        if (i2c_wait_trans_done(i2c_reg, I2C_TYPE_RX, start_time, Timeout))
        {
            printf("%s: hi2c rx timeout\n", __func__);
            i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
            hi2c->state                                 = HAL_I2C_STATE_READY;
            hi2c->error_code                            = HAL_I2C_ERR_TIMEOUT;
            return HAL_TIMEOUT;
        }

        memcpy(pData, rx_buf, tmp_repeat);
        pData += tmp_repeat;
        Size -= tmp_repeat;
        seq_idx = 0;
    }

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
    hi2c->state                                 = HAL_I2C_STATE_READY;
    hi2c->error_code                            = HAL_I2C_ERR_NONE;

    printf("%s: receive done!\n", __func__);

    return HAL_OK;
}

/**
 * @brief  Writes data to a specific memory address in a slave device (blocking
 * mode).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  MemAddress Internal memory address in the target device.
 * @param  MemAddSize Size of the internal memory address (in bytes).
 * @param  pData Pointer to the data buffer to be written.
 * @param  Size Amount of data to be written.
 * @param  Timeout Duration for blocking until the write operation is complete.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * write operation.
 *
 * This function writes data to a specific memory address in a slave device
 * in a blocking mode, where the CPU waits until the operation is complete or a
 * timeout occurs.
 */
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{

    I2C_RegTypeDef *i2c_reg    = 0;
    uint32_t        seq_idx    = 0;
    uint32_t       *cmd_buf    = 0;
    uint32_t        start_time = 0;
    uint8_t        *tx_buf     = 0;
    uint32_t        tmp_repeat = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();
    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif
    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_TX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_TX;
    hi2c->mode                                  = HAL_I2C_MODE_MEMORY_WRITE;

    HAL_I2C_UNLOCK();

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(I2C_TX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_CMD_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_EOT_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);
    tx_buf  = (uint8_t *)(hi2c->init.tx_buf);

#if defined(CONFIG_SOC_DARIC_MPW_A)
    HAL_UDMA_clr((uint32_t) & (i2c_reg->tx_addr));
    HAL_UDMA_clr((uint32_t) & (i2c_reg->cmd_addr));
#elif defined(CONFIG_SOC_DARIC_NTO_A)
    HAL_UDMA_RESET_Set(HAL_UDMA_RESET_Get() & (~(1 << (8 + hi2c->instance_id))));
#endif

    while (Size)
    {
        if (Size > I2C_BUF_SIZE)
        {
            tmp_repeat = I2C_BUF_SIZE;
        }
        else
        {
            tmp_repeat = Size;
        }

        if (MemAddSize == 1)
        { /* 8Bit */
            tx_buf[0] = i2c_mem_add_lsb(MemAddress);
        }
        else if (MemAddSize == 2)
        {
            tx_buf[0] = i2c_mem_add_msb(MemAddress);
            tx_buf[1] = i2c_mem_add_lsb(MemAddress);
        }
        else
        {
            printf("%s: MemAddSize[%d] invalid", __func__, MemAddSize);
            return HAL_ERROR;
        }

        /* config command */
        cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
        cmd_buf[seq_idx++] = i2c_cmd_start;
        cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) & 0xFE));
        cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[0]);
        if (MemAddSize == 2)
        {
            cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[1]);
        }
        cmd_buf[seq_idx++] = i2c_cmd_repeat(tmp_repeat);
        cmd_buf[seq_idx++] = i2c_cmd_wr;
        cmd_buf[seq_idx++] = i2c_cmd_stop;

        memcpy(tx_buf, pData, tmp_repeat);

        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->tx_addr), (int)tx_buf, tmp_repeat, UDMA_CHANNEL_CFG_EN);
        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);
        start_time = HAL_GetMs();
        if (i2c_wait_trans_done(i2c_reg, I2C_TYPE_TX, start_time, Timeout))
        {
            printf("%s: hi2c tx timeout\n", __func__);
            i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
            hi2c->state                                 = HAL_I2C_STATE_READY;
            hi2c->error_code                            = HAL_I2C_ERR_TIMEOUT;
            return HAL_TIMEOUT;
        }

        pData += tmp_repeat;
        MemAddress += tmp_repeat;
        Size -= tmp_repeat;
        seq_idx = 0;
    }

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
    hi2c->state                                 = HAL_I2C_STATE_READY;
    hi2c->error_code                            = HAL_I2C_ERR_NONE;

    return HAL_OK;
}

/**
 * @brief  Reads data from a specific memory address in a slave device (blocking
 * mode).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  MemAddress Internal memory address in the target device.
 * @param  MemAddSize Size of the internal memory address (in bytes).
 * @param  pData Pointer to the data buffer to store received data.
 * @param  Size Amount of data to be read.
 * @param  Timeout Duration for blocking until the read operation is complete.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * read operation.
 *
 * This function reads data from a specific memory address in a slave device
 * in a blocking mode, where the CPU waits until the operation is complete or a
 * timeout occurs.
 */
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{

    I2C_RegTypeDef *i2c_reg    = 0;
    uint32_t        seq_idx    = 0;
    uint32_t       *cmd_buf    = 0;
    uint32_t        start_time = 0;
    uint8_t        *rx_buf = 0, *tx_buf = 0;
    uint32_t        tmp_repeat = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();

    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif
    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_RX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_RX;
    hi2c->mode                                  = HAL_I2C_MODE_MEMORY_READ;

    HAL_I2C_UNLOCK();

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(I2C_TX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_CMD_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_EOT_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);
    tx_buf  = (uint8_t *)(hi2c->init.tx_buf);
    rx_buf  = (uint8_t *)(hi2c->init.rx_buf);

#if defined(CONFIG_SOC_DARIC_MPW_A)
    HAL_UDMA_clr((uint32_t) & (i2c_reg->rx_addr));
    HAL_UDMA_clr((uint32_t) & (i2c_reg->cmd_addr));
#elif defined(CONFIG_SOC_DARIC_NTO_A)
    HAL_UDMA_RESET_Set(HAL_UDMA_RESET_Get() & (~(1 << (8 + hi2c->instance_id))));
#endif

    while (Size)
    {
        if (Size > I2C_BUF_SIZE)
        {
            tmp_repeat = I2C_BUF_SIZE;
        }
        else
        {
            tmp_repeat = Size;
        }

        if (MemAddSize == 1)
        { /* 8Bit */
            tx_buf[0] = i2c_mem_add_lsb(MemAddress);
        }
        else if (MemAddSize == 2)
        {
            tx_buf[0] = i2c_mem_add_msb(MemAddress);
            tx_buf[1] = i2c_mem_add_lsb(MemAddress);
        }
        else
        {
            printf("%s: MemAddSize[%d] invalid", __func__, MemAddSize);
            return HAL_ERROR;
        }

        /* config command */
        cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
        cmd_buf[seq_idx++] = i2c_cmd_start;
        cmd_buf[seq_idx++] = i2c_cmd_wrb((DevAddress << 1) & 0xFE);
        cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[0]);
        if (MemAddSize == 2)
        {
            cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[1]);
        }
        cmd_buf[seq_idx++] = i2c_cmd_start;
        cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) | 0x1));
        if (tmp_repeat > 1)
        {
            cmd_buf[seq_idx++] = i2c_cmd_repeat(Size - 1);
            cmd_buf[seq_idx++] = i2c_cmd_rdack;
        }
        cmd_buf[seq_idx++] = i2c_cmd_rdnack;
        cmd_buf[seq_idx++] = i2c_cmd_stop;

        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->rx_addr), (int)rx_buf, tmp_repeat, UDMA_CHANNEL_CFG_EN);
        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);
        start_time = HAL_GetMs();
        if (i2c_wait_trans_done(i2c_reg, I2C_TYPE_RX, start_time, Timeout))
        {
            printf("%s: hi2c rx timeout\n", __func__);
            i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
            hi2c->state                                 = HAL_I2C_STATE_READY;
            hi2c->error_code                            = HAL_I2C_ERR_TIMEOUT;
            return HAL_TIMEOUT;
        }

        memcpy(pData, rx_buf, tmp_repeat);
        pData += tmp_repeat;
        MemAddress += tmp_repeat;
        Size -= tmp_repeat;
        seq_idx = 0;
    }

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
    hi2c->state                                 = HAL_I2C_STATE_READY;
    hi2c->error_code                            = HAL_I2C_ERR_NONE;
    // printf("%s: receive done!\n", __func__);

    return HAL_OK;
}

/**
 * @brief  Writes data to a specific memory address in a slave device (blocking
 * mode).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  MemAddress Internal memory address in the target device.
 * @param  MemAddSize Size of the internal memory address (in bytes).
 * @param  pData Pointer to the data buffer to be written.
 * @param  Size Amount of data to be written.
 * @param  Timeout Duration for blocking until the write operation is complete.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * write operation.
 *
 * This function writes data to a specific memory address in a slave device
 * in a blocking mode, where the CPU waits until the operation is complete or a
 * timeout occurs.
 */
HAL_StatusTypeDef HAL_I2C1_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{

    I2C_RegTypeDef *i2c_reg    = 0;
    uint32_t        seq_idx    = 0;
    uint32_t       *cmd_buf    = 0;
    uint32_t        start_time = 0;
    uint8_t        *tx_buf     = 0;
    uint32_t        tmp_repeat = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();
    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif
    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_TX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_TX;
    hi2c->mode                                  = HAL_I2C_MODE_MEMORY_WRITE;

    // HAL_I2C_UNLOCK();

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(I2C_TX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_CMD_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_EOT_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);
    tx_buf  = (uint8_t *)(hi2c->init.tx_buf);

#if defined(CONFIG_SOC_DARIC_MPW_A)
    HAL_UDMA_clr((uint32_t) & (i2c_reg->tx_addr));
    HAL_UDMA_clr((uint32_t) & (i2c_reg->cmd_addr));
#elif defined(CONFIG_SOC_DARIC_NTO_A)
    HAL_UDMA_RESET_Set(HAL_UDMA_RESET_Get() & (~(1 << (8 + hi2c->instance_id))));
#endif

    while (Size)
    {
        if (Size > I2C_BUF_SIZE)
        {
            tmp_repeat = I2C_BUF_SIZE;
        }
        else
        {
            tmp_repeat = Size;
        }

        if (MemAddSize == 1)
        { /* 8Bit */
            tx_buf[0] = i2c_mem_add_lsb(MemAddress);
        }
        else if (MemAddSize == 2)
        {
            tx_buf[0] = i2c_mem_add_msb(MemAddress);
            tx_buf[1] = i2c_mem_add_lsb(MemAddress);
        }
        else
        {
            printf("%s: MemAddSize[%d] invalid", __func__, MemAddSize);
            HAL_I2C_UNLOCK();
            return HAL_ERROR;
        }

        /* config command */
        cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
        cmd_buf[seq_idx++] = i2c_cmd_start;
        cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) & 0xFE));
        cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[0]);
        if (MemAddSize == 2)
        {
            cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[1]);
        }
        cmd_buf[seq_idx++] = i2c_cmd_repeat(tmp_repeat);
        cmd_buf[seq_idx++] = i2c_cmd_wr;
        cmd_buf[seq_idx++] = i2c_cmd_stop;

        memcpy(tx_buf, pData, tmp_repeat);

        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->tx_addr), (int)tx_buf, tmp_repeat, UDMA_CHANNEL_CFG_EN);
        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);
        start_time = HAL_GetMs();
        if (i2c_wait_trans_done(i2c_reg, I2C_TYPE_TX, start_time, Timeout))
        {
            printf("%s: hi2c tx timeout\n", __func__);
            i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
            hi2c->state                                 = HAL_I2C_STATE_READY;
            hi2c->error_code                            = HAL_I2C_ERR_TIMEOUT;
            HAL_I2C_UNLOCK();
            return HAL_TIMEOUT;
        }

        pData += tmp_repeat;
        MemAddress += tmp_repeat;
        Size -= tmp_repeat;
        seq_idx = 0;
    }

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
    hi2c->state                                 = HAL_I2C_STATE_READY;
    hi2c->error_code                            = HAL_I2C_ERR_NONE;
    HAL_I2C_UNLOCK();
    return HAL_OK;
}

/**
 * @brief  Reads data from a specific memory address in a slave device (blocking
 * mode).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  MemAddress Internal memory address in the target device.
 * @param  MemAddSize Size of the internal memory address (in bytes).
 * @param  pData Pointer to the data buffer to store received data.
 * @param  Size Amount of data to be read.
 * @param  Timeout Duration for blocking until the read operation is complete.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * read operation.
 *
 * This function reads data from a specific memory address in a slave device
 * in a blocking mode, where the CPU waits until the operation is complete or a
 * timeout occurs.
 */
HAL_StatusTypeDef HAL_I2C1_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{

    I2C_RegTypeDef *i2c_reg    = 0;
    uint32_t        seq_idx    = 0;
    uint32_t       *cmd_buf    = 0;
    uint32_t        start_time = 0;
    uint8_t        *rx_buf = 0, *tx_buf = 0;
    uint32_t        tmp_repeat = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();

    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif
    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_RX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_RX;
    hi2c->mode                                  = HAL_I2C_MODE_MEMORY_READ;

    /* disable interrupts */
    HAL_NVIC_DisableIRQ(I2C_TX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_CMD_CH_EVT(hi2c->instance_id));
    HAL_NVIC_DisableIRQ(I2C_EOT_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);
    tx_buf  = (uint8_t *)(hi2c->init.tx_buf);
    rx_buf  = (uint8_t *)(hi2c->init.rx_buf);

#if defined(CONFIG_SOC_DARIC_MPW_A)
    HAL_UDMA_clr((uint32_t) & (i2c_reg->rx_addr));
    HAL_UDMA_clr((uint32_t) & (i2c_reg->cmd_addr));
#elif defined(CONFIG_SOC_DARIC_NTO_A)
    HAL_UDMA_RESET_Set(HAL_UDMA_RESET_Get() & (~(1 << (8 + hi2c->instance_id))));
#endif

    while (Size)
    {
        if (Size > I2C_BUF_SIZE)
        {
            tmp_repeat = I2C_BUF_SIZE;
        }
        else
        {
            tmp_repeat = Size;
        }

        if (MemAddSize == 1)
        { /* 8Bit */
            tx_buf[0] = i2c_mem_add_lsb(MemAddress);
        }
        else if (MemAddSize == 2)
        {
            tx_buf[0] = i2c_mem_add_msb(MemAddress);
            tx_buf[1] = i2c_mem_add_lsb(MemAddress);
        }
        else
        {
            printf("%s: MemAddSize[%d] invalid", __func__, MemAddSize);
            HAL_I2C_UNLOCK();
            return HAL_ERROR;
        }

        /* config command */
        cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
        cmd_buf[seq_idx++] = i2c_cmd_start;
        cmd_buf[seq_idx++] = i2c_cmd_wrb((DevAddress << 1) & 0xFE);
        cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[0]);
        if (MemAddSize == 2)
        {
            cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[1]);
        }
        cmd_buf[seq_idx++] = i2c_cmd_start;
        cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) | 0x1));
        if (tmp_repeat > 1)
        {
            cmd_buf[seq_idx++] = i2c_cmd_repeat(Size - 1);
            cmd_buf[seq_idx++] = i2c_cmd_rdack;
        }
        cmd_buf[seq_idx++] = i2c_cmd_rdnack;
        cmd_buf[seq_idx++] = i2c_cmd_stop;

        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->rx_addr), (int)rx_buf, tmp_repeat, UDMA_CHANNEL_CFG_EN);
        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);
        start_time = HAL_GetMs();
        if (i2c_wait_trans_done(i2c_reg, I2C_TYPE_RX, start_time, Timeout))
        {
            printf("%s: hi2c rx timeout\n", __func__);
            i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
            hi2c->state                                 = HAL_I2C_STATE_READY;
            hi2c->error_code                            = HAL_I2C_ERR_TIMEOUT;
            HAL_I2C_UNLOCK();
            return HAL_TIMEOUT;
        }

        memcpy(pData, rx_buf, tmp_repeat);
        pData += tmp_repeat;
        MemAddress += tmp_repeat;
        Size -= tmp_repeat;
        seq_idx = 0;
    }

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
    hi2c->state                                 = HAL_I2C_STATE_READY;
    hi2c->error_code                            = HAL_I2C_ERR_NONE;
    // printf("%s: receive done!\n", __func__);

    HAL_I2C_UNLOCK();
    return HAL_OK;
}

/**
 * @brief  Checks if the I2C device is ready for communication.
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * operation.
 *
 * This function checks if the specified I2C device is ready for communication.
 */
HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c)
{
    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    return i2c_component[hi2c->instance_id].i2c_status == HAL_I2C_STATE_READY ? HAL_OK : HAL_BUSY;
}

/**
 * @brief  Transmits data in master mode via I2C using interrupt mode
 * (non-blocking).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  pData Pointer to the data buffer to be transmitted.
 * @param  Size Amount of data to be sent.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * transmission.
 *
 * This function sends data to the specified slave device in a non-blocking mode
 * using interrupts. The CPU is free to perform other tasks while the data is
 * being transmitted, and a callback is invoked upon completion.
 */
HAL_StatusTypeDef HAL_I2C_Transmit_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{
    I2C_RegTypeDef *i2c_reg  = NULL;
    uint32_t        seq_idx  = 0;
    uint32_t       *cmd_buf  = NULL;
    uint8_t        *tx_buf   = NULL;
    uint32_t        tmp_Size = Size, tmp_repeat = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();
    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif

    /* enable interrupts */
    HAL_NVIC_ClearPendingIRQ(I2C_TX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_TX_CH_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_TxIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_TX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_TX;
    hi2c->mode                                  = HAL_I2C_MODE_MASTER_WRITE;

    HAL_I2C_UNLOCK();

    hi2c->transfer_info.trans_buff = pData;
    hi2c->transfer_info.trans_size = Size;

    /* config command */
    cmd_buf            = (uint32_t *)(hi2c->init.cmd_buf);
    cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
    cmd_buf[seq_idx++] = i2c_cmd_start;
    cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) & 0xFE));
    while (tmp_Size > 0)
    {
        if (tmp_Size > I2C_BUF_SIZE)
        {
            tmp_repeat = I2C_BUF_SIZE;
        }
        else
        {
            tmp_repeat = tmp_Size;
        }
        cmd_buf[seq_idx++] = i2c_cmd_repeat(tmp_repeat);
        cmd_buf[seq_idx++] = i2c_cmd_wr;
        tmp_Size -= tmp_repeat;
    }
    cmd_buf[seq_idx++] = i2c_cmd_stop;

    if (hi2c->transfer_info.trans_size > I2C_BUF_SIZE)
    {
        hi2c->transfer_info.trans_count = I2C_BUF_SIZE;
    }
    else
    {
        hi2c->transfer_info.trans_count = hi2c->transfer_info.trans_size;
    }
    tx_buf = (uint8_t *)(hi2c->init.tx_buf);
    memcpy(tx_buf, pData, hi2c->transfer_info.trans_count);

    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->tx_addr), (int)tx_buf, hi2c->transfer_info.trans_count, 0);
    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);

    printf("%s: data is transmitting!\n", __func__);

    return HAL_OK;
}

/**
 * @brief  Receives data in master mode via I2C using interrupt mode
 * (non-blocking).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  pData Pointer to the data buffer to store received data.
 * @param  Size Amount of data to be received.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * reception.
 *
 * This function reads data from the specified slave device in a non-blocking
 * mode using interrupts. The CPU is free to perform other tasks while the data
 * is being received, and a callback is invoked upon completion.
 */
HAL_StatusTypeDef HAL_I2C_Receive_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size)
{

    I2C_RegTypeDef *i2c_reg = 0;
    uint32_t        seq_idx = 0;
    uint32_t       *cmd_buf = NULL;
    uint8_t        *rx_buf  = NULL;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();
    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif

    /* enable IRQ */
    HAL_NVIC_ClearPendingIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_RX_CH_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_RxIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_RX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_RX;
    hi2c->mode                                  = HAL_I2C_MODE_MASTER_READ;

    HAL_I2C_UNLOCK();

    hi2c->transfer_info.trans_buff = pData;
    hi2c->transfer_info.trans_size = Size;

    /* config command */
    cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);
    rx_buf  = (uint8_t *)(hi2c->init.rx_buf);

    if (hi2c->transfer_info.trans_size > I2C_BUF_SIZE)
    {
        hi2c->transfer_info.trans_count = I2C_BUF_SIZE;
    }
    else
    {
        hi2c->transfer_info.trans_count = hi2c->transfer_info.trans_size;
    }

    cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
    cmd_buf[seq_idx++] = i2c_cmd_start;
    cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) | 0x1));
    if (hi2c->transfer_info.trans_count > 1)
    {
        cmd_buf[seq_idx++] = i2c_cmd_repeat(hi2c->transfer_info.trans_count - 1);
        cmd_buf[seq_idx++] = i2c_cmd_rdack;
    }
    cmd_buf[seq_idx++] = i2c_cmd_rdnack;
    cmd_buf[seq_idx++] = i2c_cmd_stop;

    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->rx_addr), (int)rx_buf, hi2c->transfer_info.trans_count, UDMA_CHANNEL_CFG_EN);
    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);

    printf("%s: receiving request is rised!\n", __func__);

    return HAL_OK;
}

/**
 * @brief  Writes data to a specific memory address in a slave device using
 * interrupt mode (non-blocking).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  MemAddress Internal memory address in the target device.
 * @param  MemAddSize Size of the internal memory address (in bytes).
 * @param  pData Pointer to the data buffer to be written.
 * @param  Size Amount of data to be written.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * write operation.
 *
 * This function writes data to a specific memory address in a slave device
 * in a non-blocking mode using interrupts. The CPU is free to perform other
 * tasks while the data is being written, and a callback is invoked upon
 * completion.
 */
HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{

    I2C_RegTypeDef *i2c_reg  = 0;
    uint32_t        seq_idx  = 0;
    uint32_t       *cmd_buf  = 0;
    uint8_t        *tx_buf   = 0;
    uint32_t        tmp_Size = Size, tmp_repeat = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();
    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif

    /* enable interrupts */
    HAL_NVIC_ClearPendingIRQ(I2C_TX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_TX_CH_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_TxIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_TX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_TX;
    hi2c->mode                                  = HAL_I2C_MODE_MEMORY_WRITE;

    HAL_I2C_UNLOCK();

    tx_buf = (uint8_t *)(hi2c->init.tx_buf);

    if (MemAddSize == 1)
    { /* 8Bit */
        tx_buf[0] = i2c_mem_add_lsb(MemAddress);
    }
    else if (MemAddSize == 2)
    {
        tx_buf[0] = i2c_mem_add_msb(MemAddress);
        tx_buf[1] = i2c_mem_add_lsb(MemAddress);
    }
    else
    {
        printf("%s: MemAddSize[%d] invalid", __func__, MemAddSize);
        return HAL_ERROR;
    }

    hi2c->transfer_info.trans_buff = pData;
    hi2c->transfer_info.trans_size = Size;

    /* config command */
    cmd_buf            = (uint32_t *)(hi2c->init.cmd_buf);
    cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
    cmd_buf[seq_idx++] = i2c_cmd_start;
    cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) & 0xFE));
    cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[0]);
    if (MemAddSize == 2)
        cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[1]);
    cmd_buf[seq_idx++] = i2c_cmd_repeat(Size);
    while (tmp_Size > 0)
    {
        if (tmp_Size > I2C_BUF_SIZE)
        {
            tmp_repeat = I2C_BUF_SIZE;
        }
        else
        {
            tmp_repeat = tmp_Size;
        }
        cmd_buf[seq_idx++] = i2c_cmd_repeat(tmp_repeat);
        cmd_buf[seq_idx++] = i2c_cmd_wr;
        tmp_Size -= tmp_repeat;
    }
    cmd_buf[seq_idx++] = i2c_cmd_stop;

    if (hi2c->transfer_info.trans_size > I2C_BUF_SIZE)
    {
        hi2c->transfer_info.trans_count = I2C_BUF_SIZE;
    }
    else
    {
        hi2c->transfer_info.trans_count = hi2c->transfer_info.trans_size;
    }
    memcpy(tx_buf, pData, hi2c->transfer_info.trans_count);

    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->tx_addr), (int)tx_buf, hi2c->transfer_info.trans_count, 0);
    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), 0);

    printf("%s: data[%d] is writing\n", __func__, Size);

    return HAL_OK;
}

/**
 * @brief  Reads data from a specific memory address in a slave device using
 * interrupt mode (non-blocking).
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @param  DevAddress Target device address.
 * @param  MemAddress Internal memory address in the target device.
 * @param  MemAddSize Size of the internal memory address (in bytes).
 * @param  pData Pointer to the data buffer to store received data.
 * @param  Size Amount of data to be read.
 * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
 * read operation.
 *
 * This function reads data from a specific memory address in a slave device
 * in a non-blocking mode using interrupts. The CPU is free to perform other
 * tasks while the data is being read, and a callback is invoked upon
 * completion.
 */
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{

    I2C_RegTypeDef *i2c_reg = 0;
    uint32_t        seq_idx = 0;
    uint32_t       *cmd_buf = 0;
    uint8_t        *tx_buf  = 0;
    uint8_t        *rx_buf  = 0;

    if (!hi2c || hi2c->instance_id > I2C_MAX_NUM || !pData || !Size)
    {
        printf("%s: error parameter\n", __func__);
        return HAL_ERROR;
    }

    HAL_I2C_LOCK();
    if (i2c_component[hi2c->instance_id].i2c_status != HAL_I2C_STATE_READY)
    {
        printf("%s: i2c%" PRIu32 " status is not ready!\n", __func__, hi2c->instance_id);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
    if (hi2c->state != HAL_I2C_STATE_READY)
    {
        printf("%s: hi2c state is not ready!\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if (i2c_reg->status & 0x1)
    {
        printf("%s: i2c is busy\n", __func__);
        HAL_I2C_UNLOCK();
        return HAL_BUSY;
    }
#endif

    /* enable interrupts */
    HAL_NVIC_ClearPendingIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
    HAL_NVIC_ConnectIRQ(I2C_RX_CH_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_RxIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_NACK_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_NackIRQHandler, (void *)hi2c, 1);
    HAL_NVIC_ConnectIRQ(I2C_ERROR_EVT(hi2c->instance_id), I2C_IRQ_PRIO, I2C_IRQ_SUB_PRIO, HAL_I2C_ErrorIRQHandler, (void *)hi2c, 1);

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_BUSY_RX;
    hi2c->state                                 = HAL_I2C_STATE_BUSY_RX;
    hi2c->mode                                  = HAL_I2C_MODE_MEMORY_READ;

    HAL_I2C_UNLOCK();

    hi2c->transfer_info.trans_buff = pData;
    hi2c->transfer_info.trans_size = Size;
    hi2c->transfer_info.DevAddress = DevAddress;
    hi2c->transfer_info.MemAddress = MemAddress;
    hi2c->transfer_info.MemAddSize = MemAddSize;

    tx_buf  = (uint8_t *)(hi2c->init.tx_buf);
    rx_buf  = (uint8_t *)(hi2c->init.rx_buf);
    cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);

    if (MemAddSize == 1)
    { /* 8Bit */
        tx_buf[0] = i2c_mem_add_lsb(MemAddress);
    }
    else if (MemAddSize == 2)
    {
        tx_buf[0] = i2c_mem_add_msb(MemAddress);
        tx_buf[1] = i2c_mem_add_lsb(MemAddress);
    }
    else
    {
        printf("%s: MemAddSize[%d] invalid", __func__, MemAddSize);
        return HAL_ERROR;
    }

    if (hi2c->transfer_info.trans_size > I2C_BUF_SIZE)
    {
        hi2c->transfer_info.trans_count = I2C_BUF_SIZE;
    }
    else
    {
        hi2c->transfer_info.trans_count = hi2c->transfer_info.trans_size;
    }

    cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
    cmd_buf[seq_idx++] = i2c_cmd_start;
    cmd_buf[seq_idx++] = i2c_cmd_wrb((DevAddress << 1) & 0xFE);
    cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[0]);
    if (MemAddSize == 2)
    {
        cmd_buf[seq_idx++] = i2c_cmd_wrb(tx_buf[1]);
    }
    cmd_buf[seq_idx++] = i2c_cmd_start;
    cmd_buf[seq_idx++] = i2c_cmd_wrb(((DevAddress << 1) | 0x1));
    if (hi2c->transfer_info.trans_count > 1)
    {
        cmd_buf[seq_idx++] = i2c_cmd_repeat(hi2c->transfer_info.trans_count - 1);
        cmd_buf[seq_idx++] = i2c_cmd_rdack;
    }
    cmd_buf[seq_idx++] = i2c_cmd_rdnack;
    cmd_buf[seq_idx++] = i2c_cmd_stop;

    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->rx_addr), (int)rx_buf, hi2c->transfer_info.trans_count, UDMA_CHANNEL_CFG_EN);
    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);

    printf("%s: memory read request is rised!\n", __func__);

    return HAL_OK;
}

/**
 * @brief  Initializes the I2C peripheral according to the specified parameters.
 * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 * @retval none.
 *
 * This function used to reset I2C manually for I2C issue workaround.
 */
void HAL_I2C_Reset(I2C_HandleTypeDef *hi2c)
{

    I2C_RegTypeDef *i2c_reg = 0;

    i2c_reg = (I2C_RegTypeDef *)daric_i2c_base[hi2c->instance_id];

    i2c_reg->setup = 1;

    /* FIXME: should be a fixed value */
    for (int i = 0; i < 500000; i++)
    {
    }

    i2c_reg->setup = 0;

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
    hi2c->state                                 = HAL_I2C_STATE_READY;
    hi2c->error_code                            = HAL_I2C_ERR_NONE;

    printf("%s: reset I2C\n", __func__);
}

/**
 * @brief  Handles I2C Rx event interrupt.
 * @param  arg Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 *
 * This function handles I2C event interrupts, managing data reception
 * during I2C communication.
 */
static void HAL_I2C_RxIRQHandler(const void *arg)
{
    I2C_HandleTypeDef *hi2c    = (I2C_HandleTypeDef *)arg;
    I2C_RegTypeDef    *i2c_reg = 0;
    uint8_t           *rx_buf  = (uint8_t *)(hi2c->init.rx_buf);
    uint32_t          *cmd_buf = 0;
    uint32_t           seq_idx = 0;

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if ((i2c_reg->status & I2C_STATUS_BUSY_MASK))
    {
        hi2c->error_code = HAL_I2C_ERR_STATE_WRONG;
    }
#endif

    memcpy(hi2c->transfer_info.trans_buff, rx_buf, hi2c->transfer_info.trans_count);
    if (hi2c->transfer_info.trans_size == hi2c->transfer_info.trans_count)
    {
        hi2c->transfer_info.trans_buff  = NULL;
        hi2c->transfer_info.trans_size  = 0;
        hi2c->transfer_info.trans_count = 0;
        switch (hi2c->mode)
        {
            case HAL_I2C_MODE_MASTER_READ:
                if (hi2c->RxCpltCallback)
                    hi2c->RxCpltCallback(hi2c);
                break;
            case HAL_I2C_MODE_MEMORY_READ:
                if (hi2c->MemRxCpltCallback)
                    hi2c->MemRxCpltCallback(hi2c);
                break;
            default:
                break;
        }
        i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
        hi2c->state                                 = HAL_I2C_STATE_READY;
        hi2c->error_code                            = HAL_I2C_ERR_NONE;
    }
    else
    {
        hi2c->transfer_info.trans_buff += hi2c->transfer_info.trans_count;
        hi2c->transfer_info.trans_size -= hi2c->transfer_info.trans_count;

        cmd_buf = (uint32_t *)(hi2c->init.cmd_buf);

        if (hi2c->transfer_info.trans_size > I2C_BUF_SIZE)
        {
            hi2c->transfer_info.trans_count = I2C_BUF_SIZE;
        }
        else
        {
            hi2c->transfer_info.trans_count = hi2c->transfer_info.trans_size;
        }

        cmd_buf[seq_idx++] = i2c_cmd_cfg(hi2c->init.divider);
        cmd_buf[seq_idx++] = i2c_cmd_start;
        cmd_buf[seq_idx++] = i2c_cmd_wrb(((hi2c->transfer_info.DevAddress << 1) | 0x1));
        if (hi2c->mode == HAL_I2C_MODE_MASTER_READ)
        {
            if (hi2c->transfer_info.MemAddSize == 2)
            {
                cmd_buf[seq_idx++] = i2c_cmd_wrb(i2c_mem_add_msb(hi2c->transfer_info.MemAddress));
            }
            cmd_buf[seq_idx++] = i2c_cmd_wrb(i2c_mem_add_lsb(hi2c->transfer_info.MemAddress));
            cmd_buf[seq_idx++] = i2c_cmd_start;
            cmd_buf[seq_idx++] = i2c_cmd_wrb(((hi2c->transfer_info.DevAddress << 1) | 0x1));
        }
        if (hi2c->transfer_info.trans_count > 1)
        {
            cmd_buf[seq_idx++] = i2c_cmd_repeat(hi2c->transfer_info.trans_count - 1);
            cmd_buf[seq_idx++] = i2c_cmd_rdack;
        }
        cmd_buf[seq_idx++] = i2c_cmd_rdnack;
        cmd_buf[seq_idx++] = i2c_cmd_stop;
        if (hi2c->transfer_info.trans_count == hi2c->transfer_info.trans_size)
        {
            cmd_buf[seq_idx++] = i2c_cmd_eot;
            HAL_NVIC_DisableIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
            HAL_NVIC_EnableIRQ(I2C_EOT_CH_EVT(hi2c->instance_id));
        }
        else
        {
            HAL_NVIC_EnableIRQ(I2C_RX_CH_EVT(hi2c->instance_id));
            HAL_NVIC_DisableIRQ(I2C_EOT_CH_EVT(hi2c->instance_id));
        }

        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->rx_addr), (int)rx_buf, hi2c->transfer_info.trans_count, UDMA_CHANNEL_CFG_EN);
        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->cmd_addr), (int)cmd_buf, seq_idx * sizeof(uint32_t), UDMA_CHANNEL_CFG_EN);
    }
}

/**
 * @brief  Handles I2C Tx event interrupt.
 * @param  arg Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 *
 * This function handles I2C Tx event interrupt, managing data transmission
 * conditions during I2C communication.
 */
static void HAL_I2C_TxIRQHandler(const void *arg)
{
    I2C_HandleTypeDef *hi2c        = (I2C_HandleTypeDef *)arg;
    I2C_RegTypeDef    *i2c_reg     = 0;
    uint8_t           *tx_buf      = (uint8_t *)(hi2c->init.tx_buf);
    uint8_t            tx_max_size = hi2c->init.tx_size;
    // uint32_t trans_size = 0;

#if 0
  i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
  if ((i2c_reg->status & I2C_STATUS_BUSY_MASK) || i2c_reg->tx_size != 0)
    hi2c->state = HAL_I2C_STATE_ERROR;
  else
    hi2c->state = HAL_I2C_STATE_READY;

  if (hi2c->transfer_info.trans_size != 0) {
    if (hi2c->transfer_info.trans_size > tx_max_size)
      trans_size = tx_max_size;
    else
      trans_size = hi2c->transfer_info.trans_size;
    memcpy(tx_buf, hi2c->transfer_info.trans_buff, trans_size);
    hi2c->transfer_info.trans_size -= trans_size;
    hi2c->transfer_info.trans_buff += trans_size;
    HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->tx_addr), (int)tx_buf, trans_size,
                     UDMA_CHANNEL_CFG_EN);
  }
#endif

    if (hi2c->transfer_info.trans_count == hi2c->transfer_info.trans_size)
    {
        hi2c->transfer_info.trans_buff  = NULL;
        hi2c->transfer_info.trans_size  = 0;
        hi2c->transfer_info.trans_count = 0;

        switch (hi2c->mode)
        {
            case HAL_I2C_MODE_MASTER_WRITE:
                if (hi2c->TxCpltCallback)
                    hi2c->TxCpltCallback(hi2c);
                break;
            case HAL_I2C_MODE_MEMORY_WRITE:
                if (hi2c->MemTxCpltCallback)
                    hi2c->MemTxCpltCallback(hi2c);
                break;
            default:
                break;
        }
        i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
        hi2c->state                                 = HAL_I2C_STATE_READY;
        hi2c->error_code                            = HAL_I2C_ERR_NONE;
    }
    else
    {
        hi2c->transfer_info.trans_buff += hi2c->transfer_info.trans_count;
        hi2c->transfer_info.trans_size -= hi2c->transfer_info.trans_count;

        if (hi2c->transfer_info.trans_size > tx_max_size)
        {
            hi2c->transfer_info.trans_count = tx_max_size;
        }
        else
        {
            hi2c->transfer_info.trans_count = hi2c->transfer_info.trans_size;
        }

        memcpy(tx_buf, hi2c->transfer_info.trans_buff, hi2c->transfer_info.trans_count);

        HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->tx_addr), (int)tx_buf, hi2c->transfer_info.trans_count, 0);
    }
}

/**
 * @brief  Handles I2C Cmd event interrupt.
 * @param  arg Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 *
 * This function handles I2C Cmd event interrupt, managing data transmission
 * conditions during I2C communication.
 */
static void HAL_I2C_CmdIRQHandler(const void *arg)
{
    // TODO: need more detailed description in Daric_datasheet v1.6
    I2C_HandleTypeDef *hi2c    = (I2C_HandleTypeDef *)arg;
    I2C_RegTypeDef    *i2c_reg = 0;

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
#ifndef I2C_DEBUG_DISABLE_STATUS_BUSY_BIT
    if ((!(i2c_reg->status & I2C_STATUS_BUSY_MASK) && (i2c_reg->cmd_size == 0)))
    {
#else
    if (i2c_reg->cmd_size == 0)
    {
#endif
        if (hi2c->mode == HAL_I2C_MODE_MASTER_WRITE || hi2c->mode == HAL_I2C_MODE_MEMORY_WRITE)
        {
            uint8_t *tx_buf      = (uint8_t *)(hi2c->init.tx_buf);
            uint8_t  tx_max_size = hi2c->init.tx_size;
            uint32_t trans_size  = 0;

            if (hi2c->transfer_info.trans_size > tx_max_size)
                trans_size = tx_max_size;
            else
                trans_size = hi2c->transfer_info.trans_size;

            hi2c->transfer_info.trans_size -= trans_size;
            memcpy(tx_buf, hi2c->transfer_info.trans_buff, trans_size);
            hi2c->transfer_info.trans_buff += trans_size;

            HAL_UDMA_Enqueue((uint32_t) & (i2c_reg->tx_addr), (int)tx_buf, trans_size, UDMA_CHANNEL_CFG_EN);
        }
    }
    else
    {
        hi2c->error_code = HAL_I2C_ERR_STATE_WRONG;
        if (hi2c->ErrorCallback)
            hi2c->ErrorCallback(hi2c);
    }
}

/**
 * @brief  Handles I2C Eot event interrupt.
 * @param  arg Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 *
 * This function handles I2C Eot event interrupt, managing data transmission
 * conditions during I2C communication.
 */
static void HAL_I2C_EotIRQHandler(const void *arg)
{
    // TODO: need more detailed description in Daric_datasheet v1.6
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)arg;

    i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
    hi2c->state                                 = HAL_I2C_STATE_READY;
    hi2c->error_code                            = HAL_I2C_ERR_NONE;
}

/**
 * @brief  Handles I2C Nack event interrupt.
 * @param  arg Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 *
 * This function handles I2C Nack event interrupt, managing data transmission
 * conditions during I2C communication.
 */
static void HAL_I2C_NackIRQHandler(const void *arg)
{
    // TODO: need more detailed description in Daric_datasheet v1.6
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)arg;

    hi2c->error_code = HAL_I2C_ERR_SLAVE_NACK;
    if (hi2c->NackCallback)
        hi2c->NackCallback(hi2c);
}

/**
 * @brief  Handles I2C Error event interrupt.
 * @param  arg Pointer to an `I2C_HandleTypeDef` structure that contains
 *         the configuration information for the specified I2C module.
 *
 * This function handles I2C Error event interrupt, managing data transmission
 * conditions during I2C communication.
 */
static void HAL_I2C_ErrorIRQHandler(const void *arg)
{
    // TODO: need more detailed description in Daric_datasheet v1.6
    I2C_HandleTypeDef *hi2c    = (I2C_HandleTypeDef *)arg;
    I2C_RegTypeDef    *i2c_reg = 0;

    i2c_reg = (I2C_RegTypeDef *)(daric_i2c_base[hi2c->instance_id]);
    if ((i2c_reg->status & I2C_STATUS_ARB_LOST_MASK))
    {
        /* I2C_STATUS_ARB_LOST_MASK or not ? need spec to be more detailed! */
        i2c_component[hi2c->instance_id].i2c_status = HAL_I2C_STATE_READY;
        hi2c->state                                 = HAL_I2C_STATE_READY;
        hi2c->error_code                            = HAL_I2C_ERR_ARB_LOST;
        if (hi2c->ErrorCallback)
            hi2c->ErrorCallback(hi2c);
    }
    else
    {
        /* unknown error, reset I2C bus */
        // i2c_reg->setup = 1;
    }
}
