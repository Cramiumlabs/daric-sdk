/**
 ******************************************************************************
 * @file    daric_hal_wdg.c
 * @author  WDG Team
 * @brief   WDG HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the wdg
 *          peripheral:
 *           + Initialization functions
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
#include "daric_hal.h"
#include <stdio.h>

#ifdef HAL_WDG_MODULE_ENABLED

#define WDG_LOCKCR_LOCK         0x0
#define WDG_LOCKCR_UNLOCK       0x1ACCE551
#define WDG_COUNTER_RESET_VALUE 0xFFFFFFFF
#define WDG_CLEAR_VALUE         0x5A

/* WDG IRQ Priority define */
#ifdef HAL_WDG_IRQ_PRIO
#define WDG_IRQ_PRIO HAL_WDG_IRQ_PRIO
#else
#define WDG_IRQ_PRIO 0
#endif

#ifdef HAL_WDG_IRQ_SUB_PRIO
#define WDG_IRQ_SUB_PRIO HAL_WDG_IRQ_SUB_PRIO
#else
#define WDG_IRQ_SUB_PRIO 0
#endif

static void (*s_wdg_irq_callback)(void *param) = NULL;

static void HAL_WDG_IRQHandler(void *param)
{
    /* User interrupt callback */
    if (s_wdg_irq_callback)
    {
        s_wdg_irq_callback(param);
    }
}

/**
 * @brief Enables the watchdog timer.
 *
 * Initializes and starts the watchdog timer with the specified load value.
 * Configures reset and interrupt options.
 *
 * @param loadValue: Maximum counter value for timeout.
 * @param enableReset: Enable system reset on timeout.
 * @param enableIrq: Enable interrupt on timeout.
 * @return HAL_StatusTypeDef: HAL status.
 */
HAL_StatusTypeDef HAL_WDG_Enable(uint32_t loadValue, bool enableReset, bool enableIrq)
{
    /* Check the parameters */
    assert_param(IS_WDG_LOADVALUE(loadValue));

    /* Unlock the WDT registers */
    WDG_LOCKCR = WDG_LOCKCR_UNLOCK;

    /* Set the initial value of the counter */
    WDT->VAL = loadValue;

    /* Configure WDT mode */
    uint32_t cfgValue = 0;

    if (enableIrq)
    {
        cfgValue |= (1 << 0); // Enable IRQ
                              /* To avoid a known issue with the watchdog,
                               * it keeps generating interrupts until the watchdog reset occurs.
                               * Since we do not need to feed the dog within the interrupt,
                               * it is unnecessary to enable interrupts here.
                               */
                              // NVIC_EnableIRQ(WDT_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(WDT_IRQn);
    }

    if (enableReset)
    {
        cfgValue |= (1 << 1); // Enable RESET
    }

    WDT->CFG = cfgValue;

    /* Relock the WDT registers */
    WDG_LOCKCR = WDG_LOCKCR_LOCK;

    return HAL_OK;
}

/**
 * @brief Disables the watchdog timer.
 *
 * Stops the watchdog timer.
 *
 * @return HAL_StatusTypeDef: HAL status.
 */
HAL_StatusTypeDef HAL_WDG_Disable()
{
    /* Unlock the WDT registers */
    WDG_LOCKCR = WDG_LOCKCR_UNLOCK;

    /* Clear the counter */
    WDT->VAL = WDG_COUNTER_RESET_VALUE; // Reset the counter

    /* Clear the configuration to disable the WDT */
    WDT->CFG = 0x00; // Set configuration to 0 to disable the WDT

    /* Relock the WDT registers */
    WDG_LOCKCR = WDG_LOCKCR_LOCK;

    return HAL_OK;
}

/**
 * @brief Feeds the watchdog timer.
 *
 * Resets the watchdog timer countdown to prevent timeout.
 *
 * @return HAL_StatusTypeDef: HAL status.
 */
HAL_StatusTypeDef HAL_WDG_FeedDog()
{
    /* Check if the watchdog is enabled */
    if (WDT->CFG == 0)
    {
        return HAL_ERROR; // Watchdog is not enabled
    }
    /* Unlock the WDT registers */
    WDG_LOCKCR = WDG_LOCKCR_UNLOCK;

    WDT->CLR = WDG_CLEAR_VALUE;

    /* Relock the WDT registers */
    WDG_LOCKCR = WDG_LOCKCR_LOCK;
    return HAL_OK;
}

/**
 * @brief Init the watchdog timer interrupt.
 *
 * Initializes and starts the watchdog timer with the specified load value.
 * Configures reset and interrupt options.
 *
 * @param pCallback: User interrupt callback function.
 * @param param: User callback parameter.
 * @return HAL_StatusTypeDef: HAL status.
 */
HAL_StatusTypeDef HAL_WDG_IRQInit(void (*pCallback)(void *param), void *param)
{
    /* User interrupt callback register */
    s_wdg_irq_callback = pCallback;

    /* Connect the NVIC */
    HAL_NVIC_ConnectIRQ(WDT_IRQn, WDG_IRQ_PRIO, WDG_IRQ_SUB_PRIO, (void *)HAL_WDG_IRQHandler, param, 0);

    return HAL_OK;
}
#endif /* HAL_WDG_MODULE_ENABLED */