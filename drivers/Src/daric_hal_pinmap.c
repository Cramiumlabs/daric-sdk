/**
 ******************************************************************************
 * @file    daric_hal_pinmap.c
 * @author  PINMAP Team
 * @brief   PINMAP HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the pinmap
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

#ifdef HAL_PINMAP_MODULE_ENABLED

/* Exported functions --------------------------------------------------------*/

/**
 *  @brief Initializes the pinmap according to the specified parameters in
 * the PINMAP_InitTypeDef structure.
 *  @param PINMAP_Init: Pointer to an array of PINMAP_InitTypeDef structures
 *          that contains the configuration information for each pin.
 *  @param PinCount: Number of elements in the PINMAP_Init.
 *  @return None
 */
void HAL_PINMAP_init(PINMAP_InitTypeDef *PINMAP_Init, uint32_t PinCount)
{
    /* Check the parameters */
    assert_param(PINMAP_Init != NULL);

    for (uint32_t i = 0; i < PinCount; i++)
    {
        assert_param(IS_PINMAP_PORT(PINMAP_Init[i].Port));
        assert_param(IS_PINMAP_PIN(PINMAP_Init[i].Pin));
        assert_param(IS_GPIO_AF(PINMAP_Init[i].Func));
        GPIO_InitTypeDef GPIO_Init = { 0 };

        GPIO_Init.Pin = PINMAP_Init[i].Pin;
        /* Set GPIO mode to Alternate Function */
        GPIO_Init.Mode      = GPIO_MODE_AF;
        GPIO_Init.Alternate = PINMAP_Init[i].Func;
        /* Initialize GPIO with specified settings */
        HAL_GPIO_Init(PINMAP_Init[i].Port, &GPIO_Init);
    }
}

#endif /* HAL_PINMAP_MODULE_ENABLED */