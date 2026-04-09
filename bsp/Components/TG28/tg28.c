/**
 ******************************************************************************
 * @file    tg28.c
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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "daric_hal.h"
#include "daric_hal_def.h"
#include "daric_hal_i2c.h"

#include "daric_errno.h"
#include "daric_log.h"

#include "tg28.h"

typedef struct
{
    uint8_t register_address;
    uint8_t bit_mask;
} TG28_IRQ_Map_T;

// clang-format off
const TG28_IRQ_Map_T irq_status_map[TG28_IRQ_MAX] = {
    [TG28_IRQ_NONE]       = {0x00, 0x00},
    [TG28_IRQ_DROP_WARN]  = {0x48, 0x80},
    [TG28_IRQ_DROP_SHUT]  = {0x48, 0x40},
    [TG28_IRQ_GAUGE_TIME] = {0x48, 0x20},
    [TG28_IRQ_GAUGE_NEW]  = {0x48, 0x10},
    [TG28_IRQ_BAT_OT_CHG] = {0x48, 0x08},
    [TG28_IRQ_BAT_UT_CHG] = {0x48, 0x04},
    [TG28_IRQ_BAT_OT_WRK] = {0x48, 0x02},
    [TG28_IRQ_BAT_UT_WRK] = {0x48, 0x01},
    [TG28_IRQ_VBUS_INSER] = {0x49, 0x80},
    [TG28_IRQ_VBUS_RMV]   = {0x49, 0x40},
    [TG28_IRQ_BAT_INSER]  = {0x49, 0x20},
    [TG28_IRQ_BAT_RMV]    = {0x49, 0x10},
    [TG28_IRQ_PO_SHORT]   = {0x49, 0x08},
    [TG28_IRQ_PO_LONG]    = {0x49, 0x04},
    [TG28_IRQ_PO_NEDG]    = {0x49, 0x02},
    [TG28_IRQ_PO_PEDG]    = {0x49, 0x01},
    [TG28_IRQ_WDG_TIME]   = {0x4A, 0x80},
    [TG28_IRQ_LDO_OC]     = {0x4A, 0x40},
    [TG28_IRQ_RSV]        = {0x4A, 0x20},
    [TG28_IRQ_CHG_DONE]   = {0x4A, 0x10},
    [TG28_IRQ_CHG_STAR]   = {0x4A, 0x08},
    [TG28_IRQ_DIE_OT]     = {0x4A, 0x04},
    [TG28_IRQ_CHG_TIME]   = {0x4A, 0x02},
    [TG28_IRQ_BAT_OT]     = {0x4A, 0x01},
};

const TG28_IRQ_Map_T irq_enable_map[TG28_IRQ_MAX] = {
    [TG28_IRQ_NONE]       = {0x00, 0x00},
    [TG28_IRQ_DROP_WARN]  = {0x40, 0x80},
    [TG28_IRQ_DROP_SHUT]  = {0x40, 0x40},
    [TG28_IRQ_GAUGE_TIME] = {0x40, 0x20},
    [TG28_IRQ_GAUGE_NEW]  = {0x40, 0x10},
    [TG28_IRQ_BAT_OT_CHG] = {0x40, 0x08},
    [TG28_IRQ_BAT_UT_CHG] = {0x40, 0x04},
    [TG28_IRQ_BAT_OT_WRK] = {0x40, 0x02},
    [TG28_IRQ_BAT_UT_WRK] = {0x40, 0x01},
    [TG28_IRQ_VBUS_INSER] = {0x41, 0x80},
    [TG28_IRQ_VBUS_RMV]   = {0x41, 0x40},
    [TG28_IRQ_BAT_INSER]  = {0x41, 0x20},
    [TG28_IRQ_BAT_RMV]    = {0x41, 0x10},
    [TG28_IRQ_PO_SHORT]   = {0x41, 0x08},
    [TG28_IRQ_PO_LONG]    = {0x41, 0x04},
    [TG28_IRQ_PO_NEDG]    = {0x41, 0x02},
    [TG28_IRQ_PO_PEDG]    = {0x41, 0x01},
    [TG28_IRQ_WDG_TIME]   = {0x42, 0x80},
    [TG28_IRQ_LDO_OC]     = {0x42, 0x40},
    [TG28_IRQ_RSV]        = {0x42, 0x20},
    [TG28_IRQ_CHG_DONE]   = {0x42, 0x10},
    [TG28_IRQ_CHG_STAR]   = {0x42, 0x08},
    [TG28_IRQ_DIE_OT]     = {0x42, 0x04},
    [TG28_IRQ_CHG_TIME]   = {0x42, 0x02},
    [TG28_IRQ_BAT_OT]     = {0x42, 0x01},
};
// clang-format on

#define REG_DCDC_ONOFF          0x80
#define REG_DCDC1_VOT           0x82
#define REG_DCDC2_VOT           0x83
#define REG_DCDC3_VOT           0x84
#define REG_DCDC4_VOT           0x85
#define REG_DCDC5_VOT           0x86
#define REG_LDO_ONOFF1          0X90
#define REG_LDO_ONOFF2          0X91
#define REG_ALDO1_VOT           0X92
#define REG_ALDO2_VOT           0X93
#define REG_ALDO3_VOT           0X94
#define REG_ALDO4_VOT           0X95
#define REG_BLDO1_VOT           0X96
#define REG_BLDO2_VOT           0X97
#define REG_CPULDO_VOT          0X98
#define REG_DLDO1_VOT           0X99
#define REG_DLDO2_VOT           0X9A
#define REG_OFF_DISCHARGE_ONOFF 0X10
#define REG_BATFET_ONOFF        0X12

static I2C_HandleTypeDef I2C_Handle;
static bool              I2C_Inited = false;

void TG28_I2C_init(void)
{
    if (I2C_Inited)
    {
        return;
    }
    memset(&I2C_Handle, 0, sizeof(I2C_HandleTypeDef));
    I2C_Handle.instance_id   = CONFIG_PMIC_I2C_ID;
    I2C_Handle.init.wait     = 1;
    I2C_Handle.init.repeat   = 1;
    I2C_Handle.init.baudrate = CONFIG_PMIC_I2C_SPEED;
    I2C_Handle.init.rx_buf   = 0;
    I2C_Handle.init.rx_size  = 0;
    I2C_Handle.init.tx_buf   = 0;
    I2C_Handle.init.tx_size  = 0;
    I2C_Handle.init.cmd_buf  = 0;
    I2C_Handle.init.cmd_size = 0;

    HAL_StatusTypeDef result = HAL_I2C_Init(&I2C_Handle);
    if (result != HAL_OK)
    {
        LOGE("FAILED!\n");
        return;
    }
    I2C_Inited = true;
    LOGI("SUCCEED!\n");
}

static HAL_StatusTypeDef I2C_write(uint8_t reg_address, uint8_t reg_value)
{
    int               i   = 5;
    HAL_StatusTypeDef ret = HAL_OK;
    do
    {
        HAL_Delay(1);
        ret = HAL_I2C1_Mem_Write(&I2C_Handle, CONFIG_PMIC_I2C_ADDR, reg_address, 1, &reg_value, 1, 10);
    } while (ret == HAL_BUSY && i-- >= 0);
    if (ret != HAL_OK)
    {
        LOGE("FAILED! addr=0x%02X, value=0x%02X", reg_address, reg_value);
    }
    return ret;
}

static HAL_StatusTypeDef I2C_read(uint8_t reg_address, uint8_t *pdata)
{
    int               i   = 5;
    HAL_StatusTypeDef ret = HAL_OK;
    do
    {
        HAL_Delay(1);
        ret = HAL_I2C1_Mem_Read(&I2C_Handle, CONFIG_PMIC_I2C_ADDR, reg_address, 1, pdata, 1, 10);
    } while (ret == HAL_BUSY && i-- >= 0);
    if (ret != HAL_OK)
    {
        LOGE("FAILED! addr=0x%02X", reg_address);
    }
    return ret;
}

static HAL_StatusTypeDef update_reg(uint8_t addr, uint8_t mask, uint8_t value)
{
    uint8_t           v   = 0;
    HAL_StatusTypeDef ret = HAL_OK;
    ret                   = I2C_read(addr, &v);
    if (ret != HAL_OK)
    {
        LOGE("Read faile. addr=0x%02X, mask=0x%02X, value=0x%02X", addr, mask, value);
        return ret;
    }
    v &= (~mask);
    v |= (mask & value);
    ret = I2C_write(addr, v);
    if (ret != HAL_OK)
    {
        LOGE("Write faile. addr=0x%02X, mask=0x%02X, value=0x%02X", addr, mask, value);
    }
    return ret;
}

static uint8_t cv_1(uint16_t mv, uint16_t min, uint16_t max, uint16_t len, uint16_t step)
{
    uint8_t ret = 0;
    if (mv <= min)
    {
        ret = 0;
    }
    else if (mv < max)
    {
        ret = (mv - min) / len;
    }
    else
    {
        ret = step - 1;
    }
    return ret;
}

static uint8_t cv_2(uint16_t mv, uint16_t min1, uint16_t max1, uint16_t len1, uint16_t step1, uint16_t min2, uint16_t max2, uint16_t len2, uint16_t step2)
{
    uint8_t ret = 0;
    if (mv <= min1)
    {
        ret = 0;
    }
    else if (mv <= max1)
    {
        ret = (mv - min1) / len1;
    }
    else if (mv <= min2)
    {
        ret = step1 - 1;
    }
    else if (mv <= max2)
    {
        ret = step1 + (mv - min2) / len2;
    }
    else
    {
        ret = step1 + step2 - 1;
    }
    return ret;
}

static uint8_t cv_ch(uint8_t ch, uint16_t mv)
{
    uint8_t ret = 0;
    switch (ch)
    {
        case TG28_CH_DCDC1: // 1.5 - 3.4 V, 100mv/Step, 20 steps
            ret = cv_1(mv, 1500, 3400, 100, 20);
            break;
        case TG28_CH_DCDC2: // 0.5 - 1.2 V, 10mv/step, 71 steps; 1.22 - 1.54 V 20mv/step, 17 steps
        case TG28_CH_DCDC3: // 0.5 - 1.2 V, 10mv/step, 71 steps; 1.22 - 1.54 V 20mv/step, 17 steps
            ret = cv_2(mv, 500, 1200, 10, 71, 1220, 1540, 20, 17);
            break;
        case TG28_CH_DCDC4: // 0.5 - 1.2 V, 10mv/step, 71steps; 1.22 - 1.84 V 20mv/step, 32 steps
            ret = cv_2(mv, 500, 1200, 10, 71, 1220, 1840, 20, 32);
            break;
        case TG28_CH_DCDC5: // 1.4 - 3.7 V, 100mv/step, 24 steps
            ret = cv_1(mv, 1400, 3700, 100, 24);
            break;
        case TG28_CH_ALDO1: // 0.5 - 3.5 V, 100mv/step, 31 steps
        case TG28_CH_ALDO2: // 0.5 - 3.5 V, 100mv/step, 31 steps
        case TG28_CH_ALDO3: // 0.5 - 3.5 V, 100mv/step, 31 steps
        case TG28_CH_ALDO4: // 0.5 - 3.5 V, 100mv/step, 31 steps
        case TG28_CH_BLDO1: // 0.5 - 3.5 V, 100mv/step, 31 steps
        case TG28_CH_BLDO2: // 0.5 - 3.5 V, 100mv/step, 31 steps
            ret = cv_1(mv, 500, 3500, 100, 31);
            break;
        case TG28_CH_DLDO1: // 0.5 - 3.3 V, 100mv/step, 29 steps
            ret = cv_1(mv, 500, 3300, 100, 29);
            break;
        case TG28_CH_DLDO2:   // 0.5 - 1.4 V, 50mv/step, 20 steps
        case TG28_CH_CPUSLDO: // 0.5 - 1.4 V, 50mv/step, 20 steps
            ret = cv_1(mv, 500, 1400, 50, 20);
            break;
    }
    return ret;
}

static void power_set_voltage(uint8_t ch, uint16_t mv)
{
    uint8_t vol = cv_ch(ch, mv);
    switch (ch)
    {
        case TG28_CH_DCDC1:
            update_reg(REG_DCDC1_VOT, 0X1F, vol);
            break;
        case TG28_CH_DCDC2:
            update_reg(REG_DCDC2_VOT, 0X7F, vol);
            break;
        case TG28_CH_DCDC3:
            update_reg(REG_DCDC3_VOT, 0X7F, vol);
            break;
        case TG28_CH_DCDC4:
            update_reg(REG_DCDC4_VOT, 0X7F, vol);
            break;
        case TG28_CH_DCDC5:
            update_reg(REG_DCDC5_VOT, 0X1F, vol);
            break;
        case TG28_CH_ALDO1:
            update_reg(REG_ALDO1_VOT, 0X1F, vol);
            break;
        case TG28_CH_ALDO2:
            update_reg(REG_ALDO2_VOT, 0X1F, vol);
            break;
        case TG28_CH_ALDO3:
            update_reg(REG_ALDO3_VOT, 0X1F, vol);
            break;
        case TG28_CH_ALDO4:
            update_reg(REG_ALDO4_VOT, 0X1F, vol);
            break;
        case TG28_CH_BLDO1:
            update_reg(REG_BLDO1_VOT, 0X1F, vol);
            break;
        case TG28_CH_BLDO2:
            update_reg(REG_BLDO2_VOT, 0X1F, vol);
            break;
        case TG28_CH_CPUSLDO:
            update_reg(REG_CPULDO_VOT, 0X1F, vol);
            break;
        case TG28_CH_DLDO1:
            update_reg(REG_DLDO1_VOT, 0X1F, vol);
            break;
        case TG28_CH_DLDO2:
            update_reg(REG_DLDO2_VOT, 0X1F, vol);
            break;
    }
}

static void power_enable_ch(uint8_t ch)
{
    switch (ch)
    {
        case TG28_CH_DCDC1:
            update_reg(REG_DCDC_ONOFF, 0x01, 0x01);
            break;
        case TG28_CH_DCDC2:
            update_reg(REG_DCDC_ONOFF, 0x02, 0x02);
            break;
        case TG28_CH_DCDC3:
            update_reg(REG_DCDC_ONOFF, 0x04, 0x04);
            break;
        case TG28_CH_DCDC4:
            update_reg(REG_DCDC_ONOFF, 0x08, 0x08);
            break;
        case TG28_CH_DCDC5:
            update_reg(REG_DCDC_ONOFF, 0x10, 0x10);
            break;
        case TG28_CH_ALDO1:
            update_reg(REG_LDO_ONOFF1, 0x01, 0x01);
            break;
        case TG28_CH_ALDO2:
            update_reg(REG_LDO_ONOFF1, 0x02, 0x02);
            break;
        case TG28_CH_ALDO3:
            update_reg(REG_LDO_ONOFF1, 0x04, 0x04);
            break;
        case TG28_CH_ALDO4:
            update_reg(REG_LDO_ONOFF1, 0x08, 0x08);
            break;
        case TG28_CH_BLDO1:
            update_reg(REG_LDO_ONOFF1, 0x10, 0x10);
            break;
        case TG28_CH_BLDO2:
            update_reg(REG_LDO_ONOFF1, 0x20, 0x20);
            break;
        case TG28_CH_CPUSLDO:
            update_reg(REG_LDO_ONOFF1, 0x40, 0x40);
            break;
        case TG28_CH_DLDO1:
            update_reg(REG_LDO_ONOFF1, 0x80, 0x80);
            break;
        case TG28_CH_DLDO2:
            update_reg(REG_LDO_ONOFF2, 0x01, 0x01);
            break;
        case TG28_CH_BATFET:
            update_reg(REG_BATFET_ONOFF, 0X08, 0x08);
            break;
        case TG28_CH_OFF_DISCHARGE:
            update_reg(REG_OFF_DISCHARGE_ONOFF, 0X20, 0x20);
            break;
    }
}

static void power_disable_ch(uint8_t ch)
{
    switch (ch)
    {
        case TG28_CH_DCDC1:
            update_reg(REG_DCDC_ONOFF, 0x01, 0x00);
            break;
        case TG28_CH_DCDC2:
            update_reg(REG_DCDC_ONOFF, 0x02, 0x00);
            break;
        case TG28_CH_DCDC3:
            update_reg(REG_DCDC_ONOFF, 0x04, 0x00);
            break;
        case TG28_CH_DCDC4:
            update_reg(REG_DCDC_ONOFF, 0x08, 0x00);
            break;
        case TG28_CH_DCDC5:
            update_reg(REG_DCDC_ONOFF, 0x10, 0x00);
            break;
        case TG28_CH_ALDO1:
            update_reg(REG_LDO_ONOFF1, 0x01, 0x00);
            break;
        case TG28_CH_ALDO2:
            update_reg(REG_LDO_ONOFF1, 0x02, 0x00);
            break;
        case TG28_CH_ALDO3:
            update_reg(REG_LDO_ONOFF1, 0x04, 0x00);
            break;
        case TG28_CH_ALDO4:
            update_reg(REG_LDO_ONOFF1, 0x08, 0x00);
            break;
        case TG28_CH_BLDO1:
            update_reg(REG_LDO_ONOFF1, 0x10, 0x00);
            break;
        case TG28_CH_BLDO2:
            update_reg(REG_LDO_ONOFF1, 0x20, 0x00);
            break;
        case TG28_CH_CPUSLDO:
            update_reg(REG_LDO_ONOFF1, 0x40, 0x00);
            break;
        case TG28_CH_DLDO1:
            update_reg(REG_LDO_ONOFF1, 0x80, 0x00);
            break;
        case TG28_CH_DLDO2:
            update_reg(REG_LDO_ONOFF2, 0x01, 0x00);
            break;
        case TG28_CH_BATFET:
            update_reg(REG_BATFET_ONOFF, 0X08, 0x00);
            break;
        case TG28_CH_OFF_DISCHARGE:
            update_reg(REG_OFF_DISCHARGE_ONOFF, 0X20, 0x00);
            break;
    }
}

/**
 * @brief  Set channel power output for different channel.
 * @param  ch The selected channel. Should be one of TG28_CH_T.
 * @param  mv The setted output in milli voltage.
 */
void TG28_Ch_Power_Set(uint8_t ch, uint16_t mv)
{
    TG28_I2C_init();
    power_set_voltage(ch, mv);
    power_enable_ch(ch);
}

/**
 * @brief  Set channel power output for different channel to default voltage.
 * @param  ch The selected channel. Should be one of TG28_CH_T.
 */
void TG28_Ch_Power_On(uint8_t ch)
{
    TG28_I2C_init();
    power_enable_ch(ch);
}

/**
 * @brief  Stop channel power output.
 * @param  ch the selected channel. Should be one of TG28_CH_T.
 */
void TG28_Ch_Power_Off(uint8_t ch)
{
    TG28_I2C_init();
    power_disable_ch(ch);
}

/**
 * @brief  Set the pre charge current.
 * @param  c the setted current enum.
 */
void TG28_SetChargePreCurrent(TG28_CHARG_PRE_C_T c)
{
    I2C_write(0x61, c);
}

/**
 * @brief  Set the const charge current.
 * @param  c the setted current enum.
 */
void TG28_SetChargeConCurrent(TG28_CHARG_CON_C_T c)
{
    I2C_write(0x62, c);
}

/**
 * @brief  Set the terminal charge current.
 * @param  c the setted current enum.
 */
void TG28_SetChargeTerCurrent(TG28_CHARG_TER_C_T c)
{
    uint8_t v = 0x10 | c;
    I2C_write(0x63, v);
}

/**
 * @brief  Set the const charge voltage.
 * @param  c the setted voltage enum.
 */
void TG28_SetChargeConVoltage(TG28_CHARG_CON_V_T v)
{
    I2C_write(0x64, v);
}

/**
 * @brief  Enable cell battery charging.
 * @param  enable true to enable, false to disable.
 */
int TG28_SetChargeEnable(bool enable)
{
    HAL_StatusTypeDef ret = 0;
    ret                   = update_reg(0x18, 0x01, enable ? 0x01 : 0x00);
    return -ret;
}

/**
 * @brief  Enable or disable the interrupt.
 * @param  i the selected interrupt type.
 * @param  e enable when true, disable otherwise.
 *
 * Do nothing when the i is TG28_IRQ_NONE. Control all the interrupt when i is
 * TG28_IRQ_MAX
 */
void TG28_IRQ_Enable(TG28_IRQ_T i, bool e)
{
    uint8_t addr = 0;
    uint8_t mask = 0;

    if (i == TG28_IRQ_NONE)
    {
        return;
    }
    else if (i == TG28_IRQ_MAX)
    {
        I2C_write(0x40, e ? 0xFF : 0);
        I2C_write(0x41, e ? 0xFF : 0);
        I2C_write(0x42, e ? 0xFF : 0);
    }
    else
    {
        addr = irq_enable_map[i].register_address;
        mask = irq_enable_map[i].bit_mask;
        update_reg(addr, mask, e ? 0xFF : 0);
    }
}

/**
 * @brief  Clear the interrupt.
 * @param  i the selected interrupt type.
 * @retval 0 on success, negative value on failure.
 *
 * Do nothing when the i is TG28_IRQ_NONE. Clear all the interrupt when i is
 * TG28_IRQ_MAX
 */
int TG28_IRQ_Clear(TG28_IRQ_T i)
{
    HAL_StatusTypeDef ret  = 0;
    uint8_t           addr = 0;
    uint8_t           mask = 0;

    if (i == TG28_IRQ_NONE)
    {
        return 0;
    }
    else if (i == TG28_IRQ_MAX)
    {
        ret += I2C_write(0x48, 0xFF);
        ret += I2C_write(0x49, 0xFF);
        ret += I2C_write(0x4A, 0xFF);
    }
    else
    {
        addr = irq_status_map[i].register_address;
        mask = irq_status_map[i].bit_mask;
        ret  = update_reg(addr, mask, mask);
    }
    return ret == HAL_OK ? 0 : -ret;
}

/**
 * @brief  Reads the interrupt status of TG28.
 * @retval The first read interrupt status of TG28, like TG28_IRQ_DROP_WARN.
           If no interrupt is pending, return STATUS_NONE.
           Return negative value when read failed.
 */
int TG28_IRQ_Read(void)
{
    uint8_t           status  = 0;
    uint8_t           reg_pre = 0;
    HAL_StatusTypeDef ret;
    for (uint8_t status_index = TG28_IRQ_NONE + 1; status_index < TG28_IRQ_MAX; status_index++)
    {
        if (irq_status_map[status_index].register_address != reg_pre)
        {
            ret = I2C_read(irq_status_map[status_index].register_address, (uint8_t *)&status);
            if (ret != HAL_OK)
            {
                return -ret;
            }
            reg_pre = irq_status_map[status_index].register_address;
        }
        if (status & irq_status_map[status_index].bit_mask)
        {
            return status_index;
        }
    }
    return TG28_IRQ_NONE;
}

void TG28_enableIrq(uint32_t irq_mask)
{
    uint8_t irq_mask0 = (irq_mask & 0xFF);
    uint8_t irq_mask1 = ((irq_mask >> 8) & 0xFF);
    uint8_t irq_mask2 = ((irq_mask >> 16) & 0xFF);
    update_reg(0x40, irq_mask0, 0xFF);
    update_reg(0x41, irq_mask1, 0xFF);
    update_reg(0x42, irq_mask2, 0xFF);
}

void TG28_disableIrq(uint32_t irq_mask)
{
    uint8_t irq_mask0 = (irq_mask & 0xFF);
    uint8_t irq_mask1 = ((irq_mask >> 8) & 0xFF);
    uint8_t irq_mask2 = ((irq_mask >> 16) & 0xFF);
    update_reg(0x40, irq_mask0, 0);
    update_reg(0x41, irq_mask1, 0);
    update_reg(0x42, irq_mask2, 0);
}

uint32_t TG28_readIrq()
{
    uint32_t irq  = 0;
    uint8_t  irq0 = 0;
    uint8_t  irq1 = 0;
    uint8_t  irq2 = 0;
    I2C_read(0x48, &irq0);
    I2C_read(0x49, &irq1);
    I2C_read(0x4A, &irq2);

    irq = (irq2 << 16) | (irq1 << 8) | irq0;
    return irq;
}

void TG28_clearIrq(uint32_t irq_mask)
{
    uint8_t irq0 = irq_mask & 0xFF;
    uint8_t irq1 = (irq_mask >> 8) & 0xFF;
    uint8_t irq2 = (irq_mask >> 16) & 0xFF;
    update_reg(0x48, irq0, 0xFF);
    update_reg(0x49, irq1, 0xFF);
    update_reg(0x4A, irq2, 0xFF);
}

/**
 * @brief  Configure the Power On/Off Key (POK) behavior.
 * @param enable A boolean value indicating whether to enable (non-zero) or
 *        disable (zero) the auto power-off feature.
 * @param poweroff_time The long press time before poweroff. Value can be in TG28PoweroffTime.
 * @param poweron_time The long press time before poweron. Value can be in TG28PoweronTime.
 * @retval 0 on success, negative value on failure.
 */
int TG28_POK_Config(uint8_t enable, TG28_POK_OFF_LVL poweroff_time, TG28_POK_ON_LVL poweron_time)
{
    HAL_StatusTypeDef ret     = HAL_OK;
    uint8_t           reg_val = 0;
    reg_val |= ((poweroff_time & 0x03) << 2);
    reg_val |= ((poweron_time & 0x03));
    ret = update_reg(0x27, 0x0F, reg_val);
    return ret == HAL_OK ? 0 : -ret;
}

/**
 * @brief Reads the battery voltage (VBAT) value from the TG28 device via I2C. Unit is mV.
 * @param vbat Pointer to a uint16_t VBAT value.
 * @return 0 on success, negative HAL_StatusTypeDef value on I2C read failure.
 */
int TG28_VBAT_Read(uint16_t *vbat)
{
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t           tmp = 0;
    ret                   = I2C_read(0x34, &tmp);
    if (ret != HAL_OK)
    {
        return -ret;
    }
    LOGD("ch_dbg_en: 0x%02X", (tmp & 0xE0) >> 5);
    ret = I2C_read(0x35, (uint8_t *)vbat);
    if (ret != HAL_OK)
    {
        return -ret;
    }
    *vbat = ((tmp & 0x1F) << 8) | *vbat;
    return 0;
}

/**
 * @brief TG28 read battery capacity.
 * @param  cap Pointer to the battery capacity value. range 0--100.
 * @retval 0 on success, negative value on failure.
 */
int TG28_BAT_CAP_Read(uint8_t *cap)
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret                   = I2C_read(0xA4, cap);
    if (ret != HAL_OK)
    {
        return -ret;
    }
    return 0;
}

/**
 * @brief  System reset. The registers will be reset when PMU is powered on.
 *         At system reset state, all voltage outputs are turned off except RTCLDO and VREF.
 * @retval 0 on success, negative value on failure.
 */
int TG28_Soft_Reset(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret                   = update_reg(0x10, 0x02, 0x02); // Reset the TG28
    if (ret != HAL_OK)
    {
        LOGE("TG28_Soft_Reset failed: %d", ret);
        return -ret;
    }
    return 0;
}

/**
 * @brief  Power off the TG28 PMU.
 * @retval 0 on success, negative value on failure.
 */
int TG28_Soft_Power_Off(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret                   = update_reg(0x10, 0x01, 0x01); // Power off the TG28
    if (ret != HAL_OK)
    {
        LOGE("TG28_Soft_Power_Off failed: %d", ret);
        return -ret;
    }
    return 0;
}

/**
 * @brief TG28 read charge status.
 * @param  status Pointer to status.
 * @retval 0 on success, negative value on failure.
 */
int TG28_Charge_Status_Read(uint8_t *status)
{
    if (status == NULL)
    {
        LOGE("TG28_Charge_Status_Read: status is NULL");
        return -1;
    }
    HAL_StatusTypeDef ret = HAL_OK;
    ret                   = I2C_read(0x01, status);
    if (ret != HAL_OK)
    {
        LOGE("TG28_Charge_Status_Read: I2C read failed: %d", ret);
        return -ret;
    }
    *status &= 0x07;
    return 0;
}

TG28_CHG_ST TG28_getChargeStatus()
{
    TG28_CHG_ST status = TG28_CHG_ST_NONE;
    uint8_t     val    = 0;

    HAL_StatusTypeDef ret = I2C_read(0x01, &val);
    if (HAL_OK == ret)
    {
        val &= 0x07;
        switch (val)
        {
            case 0:
                status = TG28_CHG_ST_TRI;
                break;
            case 1:
                status = TG28_CHG_ST_PRE;
                break;
            case 2:
                status = TG28_CHG_ST_CC;
                break;
            case 3:
                status = TG28_CHG_ST_CV;
                break;
            case 4:
                status = TG28_CHG_ST_DONE;
                break;
            case 5:
                status = TG28_CHG_ST_NONE;
                break;
            default:
                status = TG28_CHG_ST_TRI;
                LOGW("Unknow status");
                break;
        }
    }
    return status;
}

void TG28_initRegister(const REG8_MAP *map, uint16_t len)
{
    for (int i = 0; i < len; i++)
    {
        I2C_write(map[i].add, map[i].val);
    }
}

int TG28_getTsVoltage(uint16_t *mv)
{
    uint8_t ts_h = 0;
    uint8_t ts_l = 0;

    int ret = I2C_read(0x36, &ts_h);
    ret |= I2C_read(0x37, &ts_l);
    if (HAL_OK == ret)
    {
        *mv = ((ts_h & 0x3F) << 8) | ts_l;
        return BSP_ERROR_NONE;
    }
    return BSP_ERROR_COMPONENT_FAILURE;
}

int TG28_getVbatPresent(bool *present)
{
    uint8_t status = 0;
    int     ret    = I2C_read(0x00, &status);
    if (ret != HAL_OK)
    {
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    *present = (status & BIT(3)) ? true : false;
    return BSP_ERROR_NONE;
}

int TG28_getVbatVoltage(uint16_t *mv)
{
    uint8_t bat_h = 0;
    uint8_t bat_l = 0;

    int ret = I2C_read(0x34, &bat_h);
    ret |= I2C_read(0x35, &bat_l);
    if (HAL_OK == ret)
    {
        *mv = ((bat_h & 0x3F) << 8) | bat_l;
        return BSP_ERROR_NONE;
    }
    return BSP_ERROR_COMPONENT_FAILURE;
}

int TG28_getVbusGood(bool *good)
{
    uint8_t status = 0;
    int     ret    = I2C_read(0x00, &status);
    if (ret != HAL_OK)
    {
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    *good = (status & BIT(5)) ? true : false;
    return BSP_ERROR_NONE;
}

void TG28_enableCharge(bool enable)
{
    update_reg(0x18, 0x02, enable ? 0x02 : 0x00);
}

int TG28_InitBatteryParam(const uint8_t *param, uint8_t size)
{
    LOGI("Configure battery parameter begin.");
    uint8_t r_param = 0;
    bool    error   = false;

    update_reg(0xA2, 0x01, 0x00);
    update_reg(0xA2, 0x01, 0x01);
    for (int i = 0; i < size; i++)
    {
        I2C_read(0xA1, &r_param);
        // LOGI("config[%03d]=0x%02X, pre-param: 0x%02X", i, param[i], r_param);
        if (param[i] != r_param)
        {
            LOGW("Pre-parameter incorrect, config[%03d]=0x%02X, pre-param: 0x%02X", i, param[i], r_param);
            error = true;
            break;
        }
    }

    if (!error)
    {
        LOGI("Pre-parameter correct, needn't config");
        update_reg(0xA2, 0x01, 0x00);
        update_reg(0xA2, 0x10, 0x10);
        return BSP_ERROR_NONE;
    }

    error = false;
    update_reg(0x17, 0x04, 0x04);
    update_reg(0x17, 0x04, 0x00);
    update_reg(0xA2, 0x01, 0x00);
    update_reg(0xA2, 0x01, 0x01);
    for (int i = 0; i < size; i++)
    {
        I2C_write(0xA1, param[i]);
    }
    update_reg(0xA2, 0x01, 0x00);
    update_reg(0xA2, 0x01, 0x01);
    for (int i = 0; i < size; i++)
    {
        I2C_read(0xA1, &r_param);
        // LOGI("Config result, config[%03d]=0x%02X, result=0x%02X", i, param[i], r_param);
        if (param[i] != r_param)
        {
            LOGW("Configure failed, config[%03d]=0x%02X, result=0x%02X", i, param[i], r_param);
            error = true;
        }
    }
    if (error)
    {
        LOGW("Configure parameter failed");
        update_reg(0xA2, 0x01, 0x00);
        update_reg(0xA2, 0x10, 0x00);
        update_reg(0x17, 0x04, 0x04);
        update_reg(0x17, 0x04, 0x00);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
        LOGI("Configure parameter successfully");
        update_reg(0xA2, 0x01, 0x00);
        update_reg(0xA2, 0x10, 0x10);
        update_reg(0x17, 0x04, 0x04);
        update_reg(0x17, 0x04, 0x00);
        return BSP_ERROR_NONE;
    }
}

void TG28_dumpBatteryParam()
{
    uint8_t r_param[16] = { 0 };

    update_reg(0xA2, 0x01, 0x00);
    update_reg(0xA2, 0x01, 0x01);

    for (int j = 0; j < 8; j++)
    {
        char    str[128] = { 0 };
        int16_t idx      = 0;
        for (int i = 0; i < 16; i++)
        {
            I2C_read(0xA1, &r_param[i]);
            idx += snprintf(str + idx, sizeof(str) - idx, "0x%02x,", r_param[i]);
        }
        LOGI("%d: %s", j, str);
    }
    update_reg(0xA2, 0x01, 0x00);
}

// clang-format off
void TG28_dumpRegister() {
    const static uint8_t addr0[] = {0x00, 0x01,             0x04, 0x05, 0x06, 0x07,};
    const static uint8_t addr1[] = {0x10,       0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,};
    const static uint8_t addr2[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,};
    const static uint8_t addr3[] = {0x30,                   0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,};
    const static uint8_t addr4[] = {0x40, 0x41, 0x42,                               0x48, 0x49, 0x4A,};
    const static uint8_t addr5[] = {0x50,       0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D,};
    const static uint8_t addr6[] = {      0x61, 0x62, 0x63, 0x64, 0x65,       0x67, 0x68, 0x69, 0x6A,};
    const static uint8_t addr8[] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,};
    const static uint8_t addr9[] = {0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A,};
    const static uint8_t addrA[] = {      0xA1, 0xA2,       0xA4,};

    const static uint8_t* addrs[] = {addr0, addr1, addr2, addr3, addr4, addr5, addr6, addr8, addr9, addrA};
    const static uint8_t counts[] = {sizeof(addr0), sizeof(addr1), sizeof(addr2), sizeof(addr3), sizeof(addr4), sizeof(addr5), sizeof(addr6), sizeof(addr8), sizeof(addr9), sizeof(addrA) };
    
    for (int i=0; i<sizeof(counts); i++) {
        char str[132] = {0};
        int16_t idx = 0;
        for (int j=0, k=0; k<16; k++) {
            uint8_t v = 0;
            if (j>=counts[i] || (addrs[i][j] & 0xF) != k) {
                idx += snprintf(str+idx, sizeof(str)-idx, "[--:--] ");
            } else {
                I2C_read(addrs[i][j], &v);
                idx += snprintf(str+idx, sizeof(str)-idx, "[%02X:%02X] ", addrs[i][j], v);
                j++;
            }
        }
        LOGI("%s", str);
    }
}
// clang-format on

void clearTG28Irq(void)
{
    // printf("TG28_IRQ_Clear\r\n");
    TG28_IRQ_Clear(TG28_IRQ_MAX);
}

/**
 * @brief  Disable BATFET.
 * @retval 0 on success, negative value on failure.
 */
int TG28_BATFET_en(bool enable)
{
    HAL_StatusTypeDef ret = HAL_OK;
    if (enable)
    {
        ret = update_reg(REG_BATFET_ONOFF, 0X08, 0x08); // Enable BATFET
    }
    else
    {
        ret = update_reg(REG_BATFET_ONOFF, 0X08, 0x00); // Disbale BATFET
    }
    if (ret != HAL_OK)
    {
        LOGE("TG28_BATFET_en failed: %d", ret);
        return -ret;
    }
    return 0;
}