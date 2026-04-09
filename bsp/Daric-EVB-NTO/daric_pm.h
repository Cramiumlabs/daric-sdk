/**
 ******************************************************************************
 * @file    daric_pm.h
 * @author  PERIPHERIAL BSP Team
 * @brief   This file contains the apis to power manager module
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

#ifndef _DARIC_PM_H_
#define _DARIC_PM_H_

#include "tg28.h"

/**
 * @brief  Initialize the power manager module
 * @retval BSP_ERROR_NONE if successfuly, failed else.
 */
int BSP_PM_init(void);

/**
 * @brief  Reset the power manager module, which will cause the deveice repower
 * on.
 * @retval BSP_ERROR_NONE if successfuly, failed else.
 */
int BSP_PM_reset(void);

/**
 * @brief  Turn off the power manager module, which will cause the deveice power
 * down.
 * @retval BSP_ERROR_NONE if successfuly, failed else.
 */
int BSP_PM_poweroff(void);

/**
 * @brief  Disconnect the battery. The device will enter ship mode.
 */
void BSP_PM_enterShipMode(void);

/**
 * @brief The power channels supported.
 */
typedef enum
{
    PM_PWR_DCDC1   = TG28_CH_DCDC1,   // 3.3V
    PM_PWR_DCDC2   = TG28_CH_DCDC2,   // 0.85V
    PM_PWR_DCDC3   = TG28_CH_DCDC3,   // 0.85V
    PM_PWR_DCDC4   = TG28_CH_DCDC4,   // 1.8V
    PM_PWR_DCDC5   = TG28_CH_DCDC5,   // 3.6V
    PM_PWR_ALDO1   = TG28_CH_ALDO1,   // 3.0V
    PM_PWR_ALDO2   = TG28_CH_ALDO2,   // 2.5V
    PM_PWR_ALDO3   = TG28_CH_ALDO3,   // 3.1V
    PM_PWR_ALDO4   = TG28_CH_ALDO4,   // 0.9V
    PM_PWR_BLDO1   = TG28_CH_BLDO1,   // 3.3V
    PM_PWR_BLDO2   = TG28_CH_BLDO2,   // 3.3V
    PM_PWR_CPUSLDO = TG28_CH_CPUSLDO, // 0.9V
    PM_PWR_DLDO1   = TG28_CH_DLDO1,   // 3.3V Same to DCDC1
    PM_PWR_DLDO2   = TG28_CH_DLDO2,   // 1.8V Same to DCDC4
} BSP_PM_PWR_E;

/**
 * @brief  Enable or disable the power output.
 * @param  ch the selected channel. Should be one of BSP_PM_PWR_E.
 * @param  en enable power output when true, disable otherwise.
 */
void BSP_PM_PWR_en(uint8_t ch, bool en);

/**
 * @brief  Set the channel voltage and enable the channel.
 * @param  ch the selected channel. Should be one of BSP_PM_PWR_E.
 * @param  mv the voltagbe(micro v) setted.
 */
void BSP_PM_PWR_set(uint8_t ch, uint16_t mv);

/**
 * @brief  Get the battery capacity
 * @retval The batter capacity. unit percentage
 */
uint8_t BSP_PM_BAT_getCapacity();

typedef enum
{
    BSP_PM_CHARG_ST_NONE,   // Doesn't charging
    BSP_PM_CHARG_ST_CHGING, // Charging
    BSP_PM_CHARG_ST_DONE,   // Charge completed
} BSP_PM_CHARG_ST;

/**
 * @brief  Get the charging status
 * @retval charge status
 */
BSP_PM_CHARG_ST BSP_PM_BAT_getChargStatus();

#ifdef BIT
#undef BIT
#define BIT(n) (1 << n)
#endif

typedef enum
{
    BSP_PM_EVT_BAT_INSERT      = BIT(1),
    BSP_PM_EVT_BAT_REMOVE      = BIT(2),
    BSP_PM_EVT_BAT_CHARGE      = BIT(3), // Battery charge status changed
    BSP_PM_EVT_BAT_CAPACITY    = BIT(4), // Battery capacity changed
    BSP_PM_EVT_BAT_DAMAGED     = BIT(5), // Battery is damaged
    BSP_PM_EVT_BAT_HOT_TEMP    = BIT(6), // Charging exception hot temprature
    BSP_PM_EVT_BAT_COLD_TEMP   = BIT(7), // Charging exception cold temprature
    BSP_PM_EVT_PEK_SHORT_PRESS = BIT(8),
    BSP_PM_EVT_PEK_LONG_PRESS  = BIT(9),
} BSP_PM_EVT_E;

/**
 * @brief The callback that will be called when the registered event happened.
 */
typedef void (*BSP_PM_EvntListener)(BSP_PM_EVT_E event, uint32_t param);

/**
 * @brief  Register the battery event listener. Must un-register before
 * re-register.
 * @param  listener, callback
 * @param  events, registered events. OR combination of @BSP_PM_EVT_E
 */
void BSP_PM_registerEventListener(BSP_PM_EvntListener listener, uint32_t events);
/**
 * @brief  Un-register the battery event listener
 * @param  listener, callback
 */
void BSP_PM_unRegisterEventListener(BSP_PM_EvntListener listener);
#endif //_DARIC_PM_H_