/**
 ******************************************************************************
 * @file    daric_hal_rtc.c
 * @author  RTC Team
 * @brief   RTC HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the Pulse Width Modulation (PWM)
 *          peripheral:
 *           + Initialization and de-initialization functions
 *           + Interrupt Management
 *           + IO operation functions
 *           + Status Information
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
#include "daric_hal_rtc.h"
#include "daric_hal_ao.h"
#define HAL_RTC_MODULE_ENABLED
#ifdef HAL_RTC_MODULE_ENABLED

void HAL_RTC_Start(void)
{
    RTC->CR |= 0x01;
}

void HAL_RTC_Stop(void)
{
    RTC->CR &= ~0x01;
}

uint32_t HAL_RTC_GetState(void)
{
    return RTC->CR;
}

uint32_t HAL_RTC_GetTime(void)
{
    return RTC->DR;
}

uint32_t HAL_RTC_GetRawInt(void)
{
    return RTC->RIS;
}

void HAL_RTC_IT_Enable(uint8_t en)
{
    if (en)
    {
        RTC->IMSC |= 0x01;
    }
    else
    {
        RTC->IMSC &= ~0x01;
    }
}

void HAL_RTC_SetTimer(uint32_t second)
{
    uint32_t current_time = HAL_RTC_GetTime();
    RTC->LR               = current_time;
    RTC->MR               = current_time + second;
}

uint32_t HAL_RTC_GetDuration(void)
{
    return (HAL_RTC_GetTime() - RTC->LR);
}

void HAL_RTC_ClearFlag(void)
{
    RTC->ICR |= 0x01;
    AON->PERI_CLR = AO_PERI_CLR_VALUE;
    NVIC_ClearPendingIRQ(AOWKUPINT_IRQn);
    __ISB();
    __DSB();
}

#endif /* HAL_RTC_MODULE_ENABLED */
