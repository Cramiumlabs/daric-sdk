/**
 ******************************************************************************
 * @file    tg28.h
 * @author  PERIPHERIAL BSP Team
 * @brief   tg28 pmu driver.
 *          This file provides some functions for pmu tg28.
 *          DCDC LDO voltage functions
 *          charging functions
 *          fuelgauge functions
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

#ifndef _TG28_H_
#define _TG28_H_

#include <stdbool.h>
#include <stdint.h>

#include "bsp_common.h"

typedef enum
{
    TG28_IRQ_NONE = 0,
    TG28_IRQ_DROP_WARN,
    TG28_IRQ_DROP_SHUT,
    TG28_IRQ_GAUGE_TIME,
    TG28_IRQ_GAUGE_NEW,
    TG28_IRQ_BAT_OT_CHG,
    TG28_IRQ_BAT_UT_CHG,
    TG28_IRQ_BAT_OT_WRK,
    TG28_IRQ_BAT_UT_WRK,

    TG28_IRQ_VBUS_INSER,
    TG28_IRQ_VBUS_RMV,
    TG28_IRQ_BAT_INSER,
    TG28_IRQ_BAT_RMV,
    TG28_IRQ_PO_SHORT,
    TG28_IRQ_PO_LONG,
    TG28_IRQ_PO_NEDG,
    TG28_IRQ_PO_PEDG,

    TG28_IRQ_WDG_TIME,
    TG28_IRQ_LDO_OC,
    TG28_IRQ_RSV,
    TG28_IRQ_CHG_DONE,
    TG28_IRQ_CHG_STAR,
    TG28_IRQ_DIE_OT,
    TG28_IRQ_CHG_TIME,
    TG28_IRQ_BAT_OT,
    TG28_IRQ_MAX
} TG28_IRQ_T;

typedef enum
{
    TG28_CH_DCDC1 = 0, // 1.5 - 3.4 V, 100mv/Step, 20 steps
    TG28_CH_DCDC2,     // 0.5 - 1.2 V, 10mv/step, 71 steps; 1.22 - 1.54 V 20mv/step, 17 steps
    TG28_CH_DCDC3,     // 0.5 - 1.2 V, 10mv/step, 71 steps; 1.22 - 1.54 V 20mv/step, 17 steps
    TG28_CH_DCDC4,     // 0.5 - 1.2 V, 10mv/step, 71 steps; 1.22 - 1.84 V 20mv/step, 32 steps
    TG28_CH_DCDC5,     // 1.4 - 3.7 V, 100mv/step, 24 steps
    TG28_CH_ALDO1,     // 0.5 - 3.5 V, 100mv/step, 31 steps
    TG28_CH_ALDO2,     // 0.5 - 3.5 V, 100mv/step, 31 steps
    TG28_CH_ALDO3,     // 0.5 - 3.5 V, 100mv/step, 31 steps
    TG28_CH_ALDO4,     // 0.5 - 3.5 V, 100mv/step, 31 steps
    TG28_CH_BLDO1,     // 0.5 - 3.5 V, 100mv/step, 31 steps
    TG28_CH_BLDO2,     // 0.5 - 3.5 V, 100mv/step, 31 steps
    TG28_CH_CPUSLDO,   // 0.5 - 1.4 V, 50mv/step, 20 steps
    TG28_CH_DLDO1,     // 0.5 - 3.3 V, 100mv/step, 29 steps
    TG28_CH_DLDO2,     // 0.5 - 1.4 V, 50mv/step, 20 steps
    //    TG28_CH_RTCLDO1,        // fused in factory
    //    TG28_CH_RTCLDO2,        // fused in factory
    TG28_CH_OFF_DISCHARGE,
    TG28_CH_BATFET,
    TG28_CH_MAX,
} TG28_CH_T;

typedef enum TG28PoweroffTime
{
    TG28_POWEROFF_TIME_4S = 0,
    TG28_POWEROFF_TIME_6S,
    TG28_POWEROFF_TIME_8S,
    TG28_POWEROFF_TIME_10S,
} TG28PoweroffTime;

typedef enum TG28PoweronTime
{
    TG28_POWERON_TIME_128MS = 0,
    TG28_POWERON_TIME_512MS,
    TG28_POWERON_TIME_1S,
    TG28_POWERON_TIME_2S,
} TG28PoweronTime;

/**
 * @brief  Initialize TG28 I2C
 */
void TG28_I2C_init(void);

/**
 * @brief  Set channel power output for different channel.
 * @param  ch The selected channel. Should be one of TG28_CH_T.
 * @param  mv The setted output in milli voltage.
 */
void TG28_Ch_Power_Set(uint8_t ch, uint16_t mv);

/**
 * @brief  Set channel power output for different channel to default voltage.
 * @param  ch The selected channel. Should be one of TG28_CH_T.
 */
void TG28_Ch_Power_On(uint8_t ch);

/**
 * @brief  Stop channel power output.
 * @param  ch the selected channel. Should be one of TG28_CH_T.
 */
void TG28_Ch_Power_Off(uint8_t ch);

typedef enum
{
    TG28_PRE_C_0 = 0,
    TG28_PRE_C_25,
    TG28_PRE_C_50,
    TG28_PRE_C_75,
    TG28_PRE_C_100,
    TG28_PRE_C_125, // Default
    TG28_PRE_C_150,
    TG28_PRE_C_175,
    TG28_PRE_C_200
} TG28_CHARG_PRE_C_T;

typedef enum
{
    TG28_CON_C_0 = 0,
    TG28_CON_C_25,
    TG28_CON_C_50, // Default
    TG28_CON_C_75,
    TG28_CON_C_100,
    TG28_CON_C_125,
    TG28_CON_C_150,
    TG28_CON_C_175,
    TG28_CON_C_200,
    TG28_CON_C_300,
    TG28_CON_C_400,
    TG28_CON_C_500,
    TG28_CON_C_600,
    TG28_CON_C_700,
    TG28_CON_C_800,
    TG28_CON_C_900,
    TG28_CON_C_1000
} TG28_CHARG_CON_C_T;

typedef enum
{
    TG28_TER_C_0 = 0,
    TG28_TER_C_25,
    TG28_TER_C_50,
    TG28_TER_C_75,
    TG28_TER_C_100,
    TG28_TER_C_125, // Default
    TG28_TER_C_150,
    TG28_TER_C_175,
    TG28_TER_C_200
} TG28_CHARG_TER_C_T;

typedef enum
{
    TG28_CON_V_5V,
    TG28_CON_V_4V,
    TG28_CON_V_4V1,
    TG28_CON_V_4V2, // Default
    TG28_CON_V_4V35,
    TG28_CON_V_4V4
} TG28_CHARG_CON_V_T;

/**
 * @brief  Set the pre charge current.
 * @param  c the setted current enum.
 */
void TG28_SetChargePreCurrent(TG28_CHARG_PRE_C_T c);

/**
 * @brief  Set the const charge current.
 * @param  c the setted current enum.
 */
void TG28_SetChargeConCurrent(TG28_CHARG_CON_C_T c);

/**
 * @brief  Set the terminal charge current.
 * @param  c the setted current enum.
 */
void TG28_SetChargeTerCurrent(TG28_CHARG_TER_C_T c);

/**
 * @brief  Set the const charge voltage.
 * @param  c the setted voltage enum.
 */
void TG28_SetChargeConVoltage(TG28_CHARG_CON_V_T v);

/**
 * @brief  Enable cell battery charging.
 * @param  enable true to enable, false to disable.
 */
int TG28_SetChargeEnable(bool enable);

/**
 * @brief  Enable or disable the interrupt.
 * @param  i the selected interrupt type.
 * @param  e enable when true, disable otherwise.
 *
 * Do nothing when the i is TG28_IRQ_NONE. Control all the interrupt when i is
 * TG28_IRQ_MAX
 */
void TG28_IRQ_Enable(TG28_IRQ_T i, bool e);

/**
 * @brief  Clear the interrupt.
 * @param  i the selected interrupt type.
 * @retval 0 on success, negative value on failure.
 *
 * Do nothing when the i is TG28_IRQ_NONE. Clear all the interrupt when i is
 * TG28_IRQ_MAX
 */
int TG28_IRQ_Clear(TG28_IRQ_T i);

/**
 * @brief  Read the interrupt status of TG28.
 * @retval The first read interrupt status of TG28, like TG28_IRQ_DROP_WARN.
           If no interrupt is pending, return STATUS_NONE.
           Return negative value when read failed.
 */
int TG28_IRQ_Read(void);

typedef enum
{
    TG28_CHG_ST_NONE,
    TG28_CHG_ST_PRE,
    TG28_CHG_ST_CC,
    TG28_CHG_ST_CV,
    TG28_CHG_ST_TRI,
    TG28_CHG_ST_DONE,
} TG28_CHG_ST;

TG28_CHG_ST TG28_getChargeStatus();

typedef enum
{
    TG28_IRQ_MASK_DROP_WARN  = BIT(7),
    TG28_IRQ_MASK_DROP_SHUT  = BIT(6),
    TG28_IRQ_MASK_GAUGE_TIME = BIT(5),
    TG28_IRQ_MASK_GAUGE_NEW  = BIT(4),
    TG28_IRQ_MASK_BAT_OT_CHG = BIT(3),
    TG28_IRQ_MASK_BAT_UT_CHG = BIT(2),
    TG28_IRQ_MASK_BAT_OT_WRK = BIT(1),
    TG28_IRQ_MASK_BAT_UT_WRK = BIT(0),

    TG28_IRQ_MASK_VBUS_INSER = BIT(8 + 7),
    TG28_IRQ_MASK_VBUS_RMV   = BIT(8 + 6),
    TG28_IRQ_MASK_BAT_INSER  = BIT(8 + 5),
    TG28_IRQ_MASK_BAT_RMV    = BIT(8 + 4),
    TG28_IRQ_MASK_PO_SHORT   = BIT(8 + 3),
    TG28_IRQ_MASK_PO_LONG    = BIT(8 + 2),
    TG28_IRQ_MASK_PO_NEDG    = BIT(8 + 1),
    TG28_IRQ_MASK_PO_PEDG    = BIT(8 + 0),

    TG28_IRQ_MASK_WDG_TIME = BIT(16 + 7),
    TG28_IRQ_MASK_LDO_OC   = BIT(16 + 6),
    TG28_IRQ_MASK_RSV      = BIT(16 + 5),
    TG28_IRQ_MASK_CHG_DONE = BIT(16 + 4),
    TG28_IRQ_MASK_CHG_STAR = BIT(16 + 3),
    TG28_IRQ_MASK_DIE_OT   = BIT(16 + 2),
    TG28_IRQ_MASK_CHG_TIME = BIT(16 + 1),
    TG28_IRQ_MASK_BAT_OT   = BIT(16 + 0),

    TG28_IRQ_MASK_ALL = 0x00FFFFFF,
} TG28_IRQ_MASK_T;

/**
 * @brief  Enable the interrup.
 # @param  irq_mask, the OR combination for TG28_IRQ_MASK_T
 */
void TG28_enableIrq(uint32_t irq_mask);

/**
 * @brief  Disable the interrup.
 # @param  irq_mask, the OR combination for TG28_IRQ_MASK_T
 */
void TG28_disableIrq(uint32_t irq_mask);

/**
 * @brief  Read the interrup status.
 # @return The interrupt status. OR combination of TG28_IRQ_MASK_T
 */
uint32_t TG28_readIrq();

/**
 * @brief  Clear the interrup status.
 # @param  irq_mask, the OR combination of TG28_IRQ_MASK_T
 */
void TG28_clearIrq(uint32_t irq_mask);

typedef enum
{
    TG28_POK_OFF_4,
    TG28_POK_OFF_6,
    TG28_POK_OFF_8,
    TG28_POK_OFF_10,
} TG28_POK_OFF_LVL;

typedef enum
{
    TG28_POK_ON_128MS,
    TG28_POK_ON_512MS,
    TG28_POK_ON_1S,
    TG28_POK_ON_2S,
} TG28_POK_ON_LVL;

/**
 * @brief  Configure the Power On/Off Key (POK) behavior.
 * @param enable A boolean value indicating whether to enable (non-zero) or
 *        disable (zero) the auto power-off feature.
 * @param  poweroff_time  Duration (in implementation-defined units) the key must be held to trigger power-off.
 * @param  poweron_time   Duration (in implementation-defined units) the key must be held to trigger power-on.
 * @retval 0 on success, negative value on failure.
 */
int TG28_POK_Config(uint8_t enable, TG28_POK_OFF_LVL poweroff_time, TG28_POK_ON_LVL poweron_time);

/**
 * @brief TG28 read battery capacity.
 * @param  pointer to the battery capacity value. range 0--100.
 * @retval 0 on success, negative value on failure.
 */
int TG28_BAT_CAP_Read(uint8_t *cap);

/**
 * @brief  System reset. The registers will be reset when PMU is powered on.
 *         At system reset state, all voltage outputs are turned off except RTCLDO and VREF.
 * @retval 0 on success, negative value on failure.
 */
int TG28_Soft_Reset(void);

/**
 * @brief  Power off the TG28 PMU.
 * @retval 0 on success, negative value on failure.
 */
int TG28_Soft_Power_Off(void);

/**
 * @brief TG28 read charge status.
 * @param  status Pointer to status.
 * @retval 0 on success, negative value on failure.
 */
int TG28_Charge_Status_Read(uint8_t *status);

/**
 * @brief Initialize the registers.
 * @param map the register value key-values.
 * @param len the configure register count.
 */
void TG28_initRegister(const REG8_MAP *map, uint16_t len);

/**
 * @brief Get the Ts temperature vaule
 * @param[out] mv the ts voltage
 */
int TG28_getTsVoltage(uint16_t *mv);

/**
 * @brief Get the battery present status
 * @param[out] present the battery present status
 */
int TG28_getVbatPresent(bool *present);

/**
 * @brief Get the battery voltage
 * @param[out] mv the battery voltage
 */
int TG28_getVbatVoltage(uint16_t *mv);

/**
 * @brief Get the vBus good status
 * @param[out] good the status
 */
int TG28_getVbusGood(bool *good);

/**
 * @brief Enable or disable the charge feature
 * @param enable enable the charge feature, disable otherwise
 */
void TG28_enableCharge(bool enable);

/**
 * @brief Initialize the batter parameter
 * @param param the battery parameter
 * @param size the parameter size
 */
int TG28_InitBatteryParam(const uint8_t *param, uint8_t size);

/**
 * @brief Dump the battery parameter.
 */
void TG28_dumpBatteryParam();

/**
 * @brief Dump the TG28 registers.
 */
void TG28_dumpRegister();
#endif //_TG28_H_