/**
******************************************************************************
* @file    daric_pmu.c
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

#include "daric_pmu.h"
#include "tg28.h"
#include <stdint.h>

/**
 * @brief  Enable or disable the power output.
 * @param  ch the selected channel. Should be one of BSP_PMU_PWR_E.
 * @param  en enable power output when true, disable otherwise.
 */
void BSP_PMU_Power_en(uint8_t ch, bool en)
{
    if (en)
    {
        TG28_Ch_Power_On(ch);
    }
    else
    {
        TG28_Ch_Power_Off(ch);
    }
}

/**
 * @brief  Set the channel voltage and enable the channel.
 * @param  ch the selected channel. Should be one of BSP_PMU_PWR_E.
 * @param  mv the voltagbe(micro v) setted.
 */
void BSP_PMU_Power_set(uint8_t ch, uint16_t mv)
{
    TG28_Ch_Power_Set(ch, mv);
}