/**
 ******************************************************************************
 * @file    daric_hal_rtc.h
 * @author  RTC Team
 * @brief   Header file of PWM HAL module.
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
#ifndef DARIC_HAL_RTC_H
#define DARIC_HAL_RTC_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include <stdbool.h>

    typedef struct
    {
        __IO uint32_t DR;
        __IO uint32_t MR;
        __IO uint32_t LR;
        __IO uint32_t CR;
        __IO uint32_t IMSC;
        __IO uint32_t RIS;
        __IO uint32_t MIS;
        __IO uint32_t ICR;
    } RTC_TypeDef;

#define RTC_BASE (0x40061000UL)
#define RTC      ((RTC_TypeDef *)RTC_BASE)

    void     HAL_RTC_Start(void);
    void     HAL_RTC_Stop(void);
    uint32_t HAL_RTC_GetState(void);
    uint32_t HAL_RTC_GetRawInt(void);
    uint32_t HAL_RTC_GetTime(void);
    void     HAL_RTC_IT_Enable(uint8_t en);
    void     HAL_RTC_SetTimer(uint32_t second);
    uint32_t HAL_RTC_GetDuration(void);
    void     HAL_RTC_ClearFlag(void);

#ifdef __cplusplus
}
#endif

#endif
