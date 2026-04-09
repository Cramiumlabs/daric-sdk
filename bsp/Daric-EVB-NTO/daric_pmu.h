/**
******************************************************************************
* @file    daric_pmu.h
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the apis to enable/disable pmu power output
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

#include <stdbool.h>
#include <stdint.h>

#include "tg28.h"

typedef enum
{
    PMU_PWR_DCDC1   = TG28_CH_DCDC1,   // 3.3V
    PMU_PWR_DCDC2   = TG28_CH_DCDC2,   // 0.85V
    PMU_PWR_DCDC3   = TG28_CH_DCDC3,   // 0.85V
    PMU_PWR_DCDC4   = TG28_CH_DCDC4,   // 1.8V
    PMU_PWR_DCDC5   = TG28_CH_DCDC5,   // 3.6V
    PMU_PWR_ALDO1   = TG28_CH_ALDO1,   // 3.0V
    PMU_PWR_ALDO2   = TG28_CH_ALDO2,   // 2.5V
    PMU_PWR_ALDO3   = TG28_CH_ALDO3,   // 3.1V
    PMU_PWR_ALDO4   = TG28_CH_ALDO4,   // 0.9V
    PMU_PWR_BLDO1   = TG28_CH_BLDO1,   // 3.3V
    PMU_PWR_BLDO2   = TG28_CH_BLDO2,   // 3.3V
    PMU_PWR_CPUSLDO = TG28_CH_CPUSLDO, // 0.9V
    PMU_PWR_DLDO1   = TG28_CH_DLDO1,   // 3.3V Same to DCDC1
    PMU_PWR_DLDO2   = TG28_CH_DLDO2,   // 1.8V Same to DCDC4
} BSP_PMU_PWR_E;

/**
 * @brief  Enable or disable the power output.
 * @param  ch the selected channel. Should be one of BSP_PMU_PWR_E.
 * @param  en enable power output when true, disable otherwise.
 */
void BSP_PMU_Power_en(uint8_t ch, bool en);

/**
 * @brief  Set the channel voltage and enable the channel.
 * @param  ch the selected channel. Should be one of BSP_PMU_PWR_E.
 * @param  mv the voltagbe(micro v) setted.
 */
void BSP_PMU_Power_set(uint8_t ch, uint16_t mv);