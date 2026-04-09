/**
 ******************************************************************************
 * @file    daric_hal_wdg.h
 * @author  WDG Team
 * @brief   Header file of WDG HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DARIC_HAL_WDG_H
#define DARIC_HAL_WDG_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "daric_hal.h"
#include "daric_hal_def.h"

#define WDT ((WDT_TypeDef *)WDG_BASE_ADDR) /*!< wgd register set access pointer */

#define WDG_LOCKCR (*(volatile uint32_t *)0x40041C00UL) ///< WDT Lock Configuration Register
#define WDG_ITCR   (*(volatile uint32_t *)0x40041F00UL) ///< WDT Internal Test Configuration Register
#define WDG_ITOP   (*(volatile uint32_t *)0x40041F04UL) ///< WDT Internal Test Operation Register
    typedef struct
    {
        __IO uint32_t VAL; ///< WDT Load Value Register
                           ///< Address offset: 0x00

        __IO uint32_t CNT; ///< WDT Counter Register
                           ///< Address offset: 0x04

        __IO uint32_t CFG; ///< WDT Configuration Register
                           ///< Address offset: 0x08

        __IO uint32_t CLR; ///< WDT Clear Counter Register
                           ///< Address offset: 0x0C

        __IO uint32_t INTRAW; ///< WDT Raw Interrupt Status Register
                              ///< Address offset: 0x10

        __IO uint32_t INT; ///< WDT Masked Interrupt Status Register
                           ///< Address offset: 0x14
    } WDT_TypeDef;

/// @cond PRIVATE_OUTPUT
#define WDG_BASE_ADDR           0x40041000
#define IS_WDG_LOADVALUE(VALUE) ((VALUE) <= 0xFFFFFFFF)
// Normal setting
#define GET_WDG_FDPCLK(fdpclk)           \
    (((fdpclk & 0x00FF) == 0xFF)   ? 8   \
     : ((fdpclk & 0x00FF) == 0x7F) ? 8   \
     : ((fdpclk & 0x00FF) == 0x3F) ? 8   \
     : ((fdpclk & 0x00FF) == 0x1F) ? 8   \
     : ((fdpclk & 0x00FF) == 0x0F) ? 16  \
     : ((fdpclk & 0x00FF) == 0x07) ? 32  \
     : ((fdpclk & 0x00FF) == 0x03) ? 64  \
     : ((fdpclk & 0x00FF) == 0x01) ? 128 \
                                   : -1)
#define WDG_CLOCK ((uint64_t)HAL_GetCoreClkMHz() * 1000000 / GET_WDG_FDPCLK(DARIC_CGU->fdpclk))
    /// @endcond

#define WDG_TICKS_PER_MS ((WDG_CLOCK) / 1000) ///< ticks per ms
    /* Exported functions --------------------------------------------------------*/
    HAL_StatusTypeDef HAL_WDG_Enable(uint32_t loadValue, bool enableReset, bool enableIrq);
    HAL_StatusTypeDef HAL_WDG_Disable();
    HAL_StatusTypeDef HAL_WDG_FeedDog();
    HAL_StatusTypeDef HAL_WDG_IRQInit(void (*pCallback)(void *param), void *param);
#ifdef __cplusplus
}
#endif

#endif // DARIC_HAL_WDG_H