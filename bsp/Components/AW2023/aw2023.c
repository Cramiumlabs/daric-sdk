/**
 ******************************************************************************
 * @file    aw2023.c
 * @author  PERIPHERIAL BSP Team
 * @brief   aw2023 rgb led driver.
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

/*******************************************************************************
 **** Copyright (C), 2024-2024, Shanghai awinic technology Co.,Ltd.
                                                all rights reserved. ***********
 *******************************************************************************
 * File Name     : aw2023.c
 * Author        : awinic
 * Date          : 2024-01-30
 * Description   : .C file function description
 * Version       : 1.0
 * Function List :
 *
*******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "aw2023.h"
#include "aw_type.h"
#include "daric_hal_i2c.h"
#include "rgb_led.h"
#include "daric_hal.h"

#define AW2023_DRIVER_VERSION "v1.0.0"

static I2C_HandleTypeDef hi2c1;
static AW_BOOL           i2c_inited = AW_FALSE;

/* This is just a rough delay */
#define delay_ms(ms) HAL_Delay(ms)
// static void delay_ms(AW_U32 ms) {
//     AW_U32 i = 0;

//     while (ms--) {
//         for (i = 0; i < 20000; i++) {
//             __NOP();
//         }
//     }
// }

static AW_U8 aw2023_i2c_write_reg(AW_U8 reg_addr, AW_U8 reg_data)
{
    AW_U8 res = 0;
    AW_U8 cnt = 0;

    while (cnt < I2C_RETRY_TIMES)
    {
        res = HAL_I2C_Mem_Write(&hi2c1, AW2023_DEVICE_ADDR, reg_addr, 1, &reg_data, 1, 100);
        if (res != AW_I2C_OK)
        {
            AWLOG("i2c_write cnt = %d, error = %d\n", cnt, res);
        }
        else
        {
            break;
        }
        cnt++;
    }

    return res;
}

static AW_U8 aw2023_i2c_read_reg(AW_U8 reg_addr, AW_U8 *reg_data)
{
    AW_U8 res = 0;
    AW_U8 cnt = 0;

    while (cnt < I2C_RETRY_TIMES)
    {
        res = HAL_I2C_Mem_Read(&hi2c1, AW2023_DEVICE_ADDR, reg_addr, 1, reg_data, 1, 100);
        if (res != AW_I2C_OK)
        {
            AWLOG("i2c_read cnt = %d, error = %d\n", cnt, res);
        }
        else
        {
            break;
        }
        cnt++;
    }

    return res;
}

/* aw2023 software reset */
static void aw2023_soft_rst(void)
{
    aw2023_i2c_write_reg(AW2023_REG_RESET, AW2023_RESET_MASK);
    /* delay 5ms at least */
    delay_ms(5);
}

/* aw2023 chip enable */
static void aw2023_chip_enable(AW_BOOL enable)
{
    AW_U8 reg_val = 0;

    aw2023_i2c_read_reg(AW2023_REG_GCR1, &reg_val);
    reg_val &= ~AW2023_CHIPEN_MASK;
    reg_val |= (((AW_U8)enable << AW2023_CHIPEN_POS) & AW2023_CHIPEN_MASK);
    aw2023_i2c_write_reg(AW2023_REG_GCR1, reg_val);

    /* About 5ms delay is required after chip enable */
    if (enable == AW_TRUE)
    {
        delay_ms(5);
    }
}

/* aw2023 check chipid */
static AW_BOOL aw2023_read_chipid(void)
{
    AW_U8   reg_val = 0;
    AW_BOOL ret     = AW_FALSE;

    aw2023_i2c_read_reg(AW2023_REG_RESET, &reg_val);
    AWLOG("chip_id = 0x%x\n", reg_val);
    if (reg_val == AW2023_CHIP_ID)
    {
        ret = AW_TRUE;
    }

    return ret;
}

/* aw2023 global max output current selection */
static void aw2023_set_imax(AW_U8 imax)
{
    AW_U8 reg_val = 0;

    aw2023_i2c_read_reg(AW2023_REG_GCR2, &reg_val);
    reg_val &= ~AW2023_IMAX_MASK;
    reg_val |= ((imax << AW2023_IMAX_POS) & AW2023_IMAX_MASK);
    aw2023_i2c_write_reg(AW2023_REG_GCR2, reg_val);
}

/* aw2023 led enable control */
static void aw2023_led_enable_ctrl(AW_U8 led_mask, AW_BOOL led_enable)
{
    AW_U8 reg_val = 0;

    aw2023_i2c_read_reg(AW2023_REG_LCTR, &reg_val);
    if (led_enable == AW_TRUE)
    {
        reg_val |= ((led_mask << AW2023_LED_ENABLE_CTRL_POS) & AW2023_LED_ENABLE_CTRL_MASK);
    }
    else
    {
        reg_val &= ~((led_mask << AW2023_LED_ENABLE_CTRL_POS) & AW2023_LED_ENABLE_CTRL_MASK);
    }
    aw2023_i2c_write_reg(AW2023_REG_LCTR, reg_val);
}

/* aw2023 pwm transition mode selection: exponential or linear */
static void aw2023_set_pwm_mode(AW_U8 mode)
{
    AW_U8 reg_val = 0;

    aw2023_i2c_read_reg(AW2023_REG_LCTR, &reg_val);
    reg_val &= ~AW2023_PWM_MODE_SELECTION_MASK;
    reg_val |= ((mode << AW2023_PWM_MODE_SELECTION_POS) & AW2023_PWM_MODE_SELECTION_MASK);
    aw2023_i2c_write_reg(AW2023_REG_LCTR, reg_val);
}

/* aw2023 pwm carrier frequency selection: 250Hz or 125Hz */
static void aw2023_set_pwm_freq(AW_U8 freq)
{
    AW_U8 reg_val = 0;

    aw2023_i2c_read_reg(AW2023_REG_LCTR, &reg_val);
    reg_val &= ~AW2023_PWM_FREQ_MASK;
    reg_val |= ((freq << AW2023_PWM_FREQ_POS) & AW2023_PWM_FREQ_MASK);
    aw2023_i2c_write_reg(AW2023_REG_LCTR, reg_val);
}

/* aw2023 set individual control mode or sync control mode */
static void aw2023_set_sync_mode(AW_BOOL sync_enable)
{
    AW_U8 reg_val = 0;

    aw2023_i2c_read_reg(AW2023_REG_LCFG0, &reg_val);
    reg_val &= ~AW2023_LED_SYNC_MASK;
    reg_val |= (((AW_U8)sync_enable << AW2023_LED_SYNC_POS) & AW2023_LED_SYNC_MASK);
    aw2023_i2c_write_reg(AW2023_REG_LCFG0, reg_val);
}

/* aw2023 set led output current */
static void aw2023_set_led_current(AW_U8 led_index, AW_U8 current)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LCFG0 + led_index, &reg_val);
        reg_val &= ~AW2023_LED_CUR_MASK;
        reg_val |= ((current << AW2023_LED_CUR_POS) & AW2023_LED_CUR_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LCFG0 + led_index, reg_val);
    }
}

/* aw2023 led operating mode select: manual mode or pattern mode */
static void aw2023_set_led_operating_mode(AW_U8 led_index, AW_U8 mode)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LCFG0 + led_index, &reg_val);
        reg_val &= ~AW2023_LED_MD_MASK;
        reg_val |= ((mode << AW2023_LED_MD_POS) & AW2023_LED_MD_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LCFG0 + led_index, reg_val);
    }
}

/* aw2023 fade-in enable control, only active in manual mode */
static void aw2023_fade_in_enable_ctrl(AW_U8 led_index, AW_BOOL enable)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LCFG0 + led_index, &reg_val);
        reg_val &= ~AW2023_LED_FI_MASK;
        reg_val |= (((AW_U8)enable << AW2023_LED_FI_POS) & AW2023_LED_FI_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LCFG0 + led_index, reg_val);
    }
}

/* aw2023 fade-out enable control, only active in manual mode */
static void aw2023_fade_out_enable_ctrl(AW_U8 led_index, AW_BOOL enable)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LCFG0 + led_index, &reg_val);
        reg_val &= ~AW2023_LED_FO_MASK;
        reg_val |= (((AW_U8)enable << AW2023_LED_FO_POS) & AW2023_LED_FO_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LCFG0 + led_index, reg_val);
    }
}

/* aw2023 set led pwm dimming level */
static void aw2023_set_pwm_dimming(AW_U8 led_index, AW_U8 dim)
{
    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_write_reg(AW2023_REG_PWM0 + led_index, dim);
    }
}

/* aw2023 pattern or manual mode rise-time */
static void aw2023_set_rise_time(AW_U8 led_index, AW_U8 time)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LED0T0 + led_index * AW2023_LED_NUM, &reg_val);
        reg_val &= ~AW2023_RISE_TIME_MASK;
        reg_val |= ((time << AW2023_RISE_TIME_POS) & AW2023_RISE_TIME_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LED0T0 + led_index * AW2023_LED_NUM, reg_val);
    }
}

/* aw2023 pattern mode on-time */
static void aw2023_set_on_time(AW_U8 led_index, AW_U8 time)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LED0T0 + led_index * AW2023_LED_NUM, &reg_val);
        reg_val &= ~AW2023_ON_TIME_MASK;
        reg_val |= ((time << AW2023_ON_TIME_POS) & AW2023_ON_TIME_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LED0T0 + led_index * AW2023_LED_NUM, reg_val);
    }
}

/* aw2023 pattern or manual mode fall-time */
static void aw2023_set_fall_time(AW_U8 led_index, AW_U8 time)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LED0T1 + led_index * AW2023_LED_NUM, &reg_val);
        reg_val &= ~AW2023_FALL_TIME_MASK;
        reg_val |= ((time << AW2023_FALL_TIME_POS) & AW2023_FALL_TIME_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LED0T1 + led_index * AW2023_LED_NUM, reg_val);
    }
}

/* aw2023 pattern mode off-time */
static void aw2023_set_off_time(AW_U8 led_index, AW_U8 time)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LED0T1 + led_index * AW2023_LED_NUM, &reg_val);
        reg_val &= ~AW2023_OFF_TIME_MASK;
        reg_val |= ((time << AW2023_OFF_TIME_POS) & AW2023_OFF_TIME_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LED0T1 + led_index * AW2023_LED_NUM, reg_val);
    }
}

/* aw2023 delay time of pattern startup */
static void aw2023_set_delay_time(AW_U8 led_index, AW_U8 time)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LED0T2 + led_index * AW2023_LED_NUM, &reg_val);
        reg_val &= ~AW2023_DELAY_TIME_MASK;
        reg_val |= ((time << AW2023_DELAY_TIME_POS) & AW2023_DELAY_TIME_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LED0T2 + led_index * AW2023_LED_NUM, reg_val);
    }
}

/* aw2023 pattern repeat time */
static void aw2023_set_repeat_time(AW_U8 led_index, AW_U8 time)
{
    AW_U8 reg_val = 0;

    if (led_index < AW2023_LED_NUM)
    {
        aw2023_i2c_read_reg(AW2023_REG_LED0T2 + led_index * AW2023_LED_NUM, &reg_val);
        reg_val &= ~AW2023_REPEAT_TIME_MASK;
        reg_val |= ((time << AW2023_REPEAT_TIME_POS) & AW2023_REPEAT_TIME_MASK);
        aw2023_i2c_write_reg(AW2023_REG_LED0T2 + led_index * AW2023_LED_NUM, reg_val);
    }
}

static void aw2023_I2C_init(void);

static AW_BOOL initialized = AW_FALSE;
AW_BOOL        aw2023_inited()
{
    return initialized;
}

/* aw2023 chip init */
AW_BOOL aw2023_init(void)
{
    AW_BOOL ret = AW_FALSE;

    if (initialized)
    {
        return AW_TRUE;
    }

    // IIC inita
    aw2023_I2C_init();
    delay_ms(50);

    /* 1.Soft reset */
    aw2023_soft_rst();
    /* 2.Chip enable */
    aw2023_chip_enable(AW_TRUE);
    /* 3.Read and check that the chip ID is correct */
    ret = aw2023_read_chipid();
    if (ret == AW_TRUE)
    {
        /* 4.Set the global maximum output current */
        aw2023_set_imax(AW2023_IMAX_5MA);
        /* 5.Set the PWM output frequency */
        aw2023_set_pwm_freq(AW2023_PWM_FREQ_250HZ);
    }

    initialized = AW_TRUE;

    return ret;
}

/* aw2023 all led full on */
static AW_BOOL aw2023_all_led_full_on(void)
{
    AW_BOOL ret = AW_FALSE;

    /* 1.Chip initialization */
    ret = aw2023_init();
    if (ret == AW_TRUE)
    {
        /* 2.Select pwm transition mode */
        aw2023_set_pwm_mode(AW2023_PWM_LINEAR);
        /* 3.Set the LED port output current */
        aw2023_set_led_current(AW2023_LED0, 0x0f);
        aw2023_set_led_current(AW2023_LED1, 0x0f);
        aw2023_set_led_current(AW2023_LED2, 0x0f);
        /* 4.Set led pwm dimming level */
        aw2023_set_pwm_dimming(AW2023_LED0, 0xff);
        aw2023_set_pwm_dimming(AW2023_LED1, 0xff);
        aw2023_set_pwm_dimming(AW2023_LED2, 0xff);
        /* 5.Three LED channels were enabled */
        aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_TRUE);
    }

    return ret;
}

/* aw2023 all led breath (pattern mode) */
static AW_BOOL aw2023_all_led_breath(void)
{
    AW_BOOL ret = AW_FALSE;

    /* 1.Chip initialization */
    ret = aw2023_init();
    if (ret == AW_TRUE)
    {
        /* 2.Select pwm transition mode */
        aw2023_set_pwm_mode(AW2023_PWM_EXPONENTIAL);
        /* 3.Set the LED port output current */
        aw2023_set_led_current(AW2023_LED0, 0x0f);
        aw2023_set_led_current(AW2023_LED1, 0x0f);
        aw2023_set_led_current(AW2023_LED2, 0x0f);
        /* 4.Set led pwm dimming level */
        aw2023_set_pwm_dimming(AW2023_LED0, 0xff);
        aw2023_set_pwm_dimming(AW2023_LED1, 0xff);
        aw2023_set_pwm_dimming(AW2023_LED2, 0xff);
        /* 5.Set rise-time */
        aw2023_set_rise_time(AW2023_LED0, AW2023_PAT_TIME_1_04);
        aw2023_set_rise_time(AW2023_LED1, AW2023_PAT_TIME_2_10);
        aw2023_set_rise_time(AW2023_LED2, AW2023_PAT_TIME_3_10);
        /* 6.Set on-time */
        aw2023_set_on_time(AW2023_LED0, AW2023_PAT_TIME_0_13);
        aw2023_set_on_time(AW2023_LED1, AW2023_PAT_TIME_0_26);
        aw2023_set_on_time(AW2023_LED2, AW2023_PAT_TIME_0_38);
        /* 7.Set fall-time */
        aw2023_set_fall_time(AW2023_LED0, AW2023_PAT_TIME_1_04);
        aw2023_set_fall_time(AW2023_LED1, AW2023_PAT_TIME_2_10);
        aw2023_set_fall_time(AW2023_LED2, AW2023_PAT_TIME_3_10);
        /* 8.Set off-time */
        aw2023_set_off_time(AW2023_LED0, AW2023_PAT_TIME_0_13);
        aw2023_set_off_time(AW2023_LED1, AW2023_PAT_TIME_0_26);
        aw2023_set_off_time(AW2023_LED2, AW2023_PAT_TIME_0_38);
        /* 9.Set delay time of pattern startup */
        aw2023_set_delay_time(AW2023_LED0, AW2023_PAT_TIME_0_13);
        aw2023_set_delay_time(AW2023_LED1, AW2023_PAT_TIME_0_26);
        aw2023_set_delay_time(AW2023_LED2, AW2023_PAT_TIME_0_38);
        /* 10.Set breath repeat time */
        aw2023_set_repeat_time(AW2023_LED0, AW2023_BREATH_REPEAT_FOREVER);
        aw2023_set_repeat_time(AW2023_LED1, AW2023_BREATH_REPEAT_FOREVER);
        aw2023_set_repeat_time(AW2023_LED2, AW2023_BREATH_REPEAT_FOREVER);
        /* 11.Set operating mode: pattern mode*/
        aw2023_set_led_operating_mode(AW2023_LED0, AW2023_PATTERN_MODE);
        aw2023_set_led_operating_mode(AW2023_LED1, AW2023_PATTERN_MODE);
        aw2023_set_led_operating_mode(AW2023_LED2, AW2023_PATTERN_MODE);
        /* 12.All led enable */
        aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_TRUE);
    }

    return ret;
}

/* aw2023 all led breath (pattern mode with sync) */
static AW_BOOL aw2023_all_led_breath_with_sync(void)
{
    AW_BOOL ret = AW_FALSE;

    /* 1.Chip initialization */
    ret = aw2023_init();
    if (ret == AW_TRUE)
    {
        /* 2.Select pwm transition mode */
        aw2023_set_pwm_mode(AW2023_PWM_EXPONENTIAL);
        /* 3.Synchronous control mode enable */
        aw2023_set_sync_mode(AW_TRUE);
        /* 4.Set the LED port output current */
        aw2023_set_led_current(AW2023_LED0, 0x0f);
        aw2023_set_led_current(AW2023_LED1, 0x0f);
        aw2023_set_led_current(AW2023_LED2, 0x0f);
        /* 5.Set led pwm dimming level */
        aw2023_set_pwm_dimming(AW2023_LED0, 0xff);
        /* 6.Set breath timing parameter */
        aw2023_set_rise_time(AW2023_LED0, AW2023_PAT_TIME_1_04);
        aw2023_set_on_time(AW2023_LED0, AW2023_PAT_TIME_0_13);
        aw2023_set_fall_time(AW2023_LED0, AW2023_PAT_TIME_1_04);
        aw2023_set_off_time(AW2023_LED0, AW2023_PAT_TIME_0_13);
        aw2023_set_delay_time(AW2023_LED0, AW2023_PAT_TIME_0_13);
        aw2023_set_repeat_time(AW2023_LED0, AW2023_BREATH_REPEAT_FOREVER);
        /* 7.Set operating mode: pattern mode*/
        aw2023_set_led_operating_mode(AW2023_LED0, AW2023_PATTERN_MODE);
        /* 8.All led enable */
        aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_TRUE);
    }

    return ret;
}

/* aw2023 manual mode */
static AW_BOOL aw2023_all_led_manual_ctrl(void)
{
    AW_BOOL ret = AW_FALSE;

    /* 1.Chip initialization */
    ret = aw2023_init();
    if (ret == AW_TRUE)
    {
        /* 2.Select pwm transition mode */
        aw2023_set_pwm_mode(AW2023_PWM_EXPONENTIAL);
        /* 3.PWM fade-in enable */
        aw2023_fade_in_enable_ctrl(AW2023_LED0, AW_TRUE);
        aw2023_fade_in_enable_ctrl(AW2023_LED1, AW_TRUE);
        aw2023_fade_in_enable_ctrl(AW2023_LED2, AW_TRUE);
        /* 4.PWM fade-out enable */
        aw2023_fade_out_enable_ctrl(AW2023_LED0, AW_TRUE);
        aw2023_fade_out_enable_ctrl(AW2023_LED1, AW_TRUE);
        aw2023_fade_out_enable_ctrl(AW2023_LED2, AW_TRUE);
        /* 5.Set PWM fade-in time */
        aw2023_set_rise_time(AW2023_LED0, AW2023_PAT_TIME_0_51);
        aw2023_set_rise_time(AW2023_LED1, AW2023_PAT_TIME_0_77);
        aw2023_set_rise_time(AW2023_LED2, AW2023_PAT_TIME_1_04);
        /* 6.Set PWM fade-out time */
        aw2023_set_fall_time(AW2023_LED0, AW2023_PAT_TIME_0_51);
        aw2023_set_fall_time(AW2023_LED1, AW2023_PAT_TIME_0_77);
        aw2023_set_fall_time(AW2023_LED2, AW2023_PAT_TIME_1_04);
        /* 7.Set the LED port output current */
        aw2023_set_led_current(AW2023_LED0, 0x0f);
        aw2023_set_led_current(AW2023_LED1, 0x0f);
        aw2023_set_led_current(AW2023_LED2, 0x0f);
        /* 8.Select led operating mode: manual mode */
        aw2023_set_led_operating_mode(AW2023_LED0, AW2023_MANUAL_MODE);
        aw2023_set_led_operating_mode(AW2023_LED1, AW2023_MANUAL_MODE);
        aw2023_set_led_operating_mode(AW2023_LED2, AW2023_MANUAL_MODE);
        /* 9.All led enable */
        aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_TRUE);
        /* 10.Set led pwm dimming level */
        aw2023_set_pwm_dimming(AW2023_LED0, 0xff);
        aw2023_set_pwm_dimming(AW2023_LED1, 0xff);
        aw2023_set_pwm_dimming(AW2023_LED2, 0xff);
        /* Delay 1000ms */
        delay_ms(1000);
        /* 11.Set led pwm dimming level */
        aw2023_set_pwm_dimming(AW2023_LED0, 0x00);
        aw2023_set_pwm_dimming(AW2023_LED1, 0x00);
        aw2023_set_pwm_dimming(AW2023_LED2, 0x00);
    }

    return ret;
}

/* aw2023 led manual mode with sync */
static AW_BOOL aw2023_all_led_manual_ctrl_with_sync(void)
{
    AW_BOOL ret = AW_FALSE;

    /* 1.Chip initialization */
    ret = aw2023_init();
    if (ret == AW_TRUE)
    {
        /* 2.Select pwm transition mode */
        aw2023_set_pwm_mode(AW2023_PWM_EXPONENTIAL);
        /* 3.Synchronous control mode enable */
        aw2023_set_sync_mode(AW_TRUE);
        /* 4.PWM fade-in and fade-out enable */
        aw2023_fade_in_enable_ctrl(AW2023_LED0, AW_TRUE);
        aw2023_fade_out_enable_ctrl(AW2023_LED0, AW_TRUE);
        /* 5.Set PWM fade-in and fade-out time */
        aw2023_set_rise_time(AW2023_LED0, AW2023_PAT_TIME_0_77);
        aw2023_set_fall_time(AW2023_LED0, AW2023_PAT_TIME_0_77);
        /* 6.Set the LED port output current */
        aw2023_set_led_current(AW2023_LED0, 0x0f);
        aw2023_set_led_current(AW2023_LED1, 0x0f);
        aw2023_set_led_current(AW2023_LED2, 0x0f);
        /* 7.Select led operating mode: manual mode */
        aw2023_set_led_operating_mode(AW2023_LED0, AW2023_MANUAL_MODE);
        /* 8.All led enable */
        aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_TRUE);
        /* 9.Set led pwm dimming level */
        aw2023_set_pwm_dimming(AW2023_LED0, 0xff);
        /* Delay 1000ms */
        delay_ms(1000);
        /* 10.Set led pwm dimming level */
        aw2023_set_pwm_dimming(AW2023_LED0, 0x00);
    }

    return ret;
}

AW_BOOL aw2023_play(void)
{
    AW_BOOL ret = AW_FALSE;

    /* all led manual control with sync */
    ret = aw2023_all_led_manual_ctrl_with_sync();
    delay_ms(5000);

    /* all led full on */
    ret = aw2023_all_led_full_on();
    delay_ms(5000);

    /* all led breath */
    ret = aw2023_all_led_breath();
    delay_ms(5000);

    /* all led breath with sync */
    ret = aw2023_all_led_breath_with_sync();
    delay_ms(5000);
    /* all led manual control */
    ret = aw2023_all_led_manual_ctrl();
    delay_ms(5000);
    /* all led manual control with sync */
    ret = aw2023_all_led_manual_ctrl_with_sync();
    delay_ms(5000);

    return ret;
}

static void aw2023_I2C_init(void)
{
    if (i2c_inited)
    {
        return;
    }
    memset(&hi2c1, 0, sizeof(I2C_HandleTypeDef));
    hi2c1.instance_id   = CONFIG_RGB_LED_I2C_ID;
    hi2c1.init.wait     = 1;
    hi2c1.init.repeat   = 1;
    hi2c1.init.baudrate = CONFIG_RGB_LED_I2C_SPEED;
    hi2c1.init.rx_buf   = 0;
    hi2c1.init.rx_size  = 0;
    hi2c1.init.tx_buf   = 0;
    hi2c1.init.tx_size  = 0;
    hi2c1.init.cmd_buf  = 0;
    hi2c1.init.cmd_size = 0;

    HAL_StatusTypeDef result = HAL_I2C_Init(&hi2c1);

    if (result == HAL_OK)
    {
        i2c_inited = AW_TRUE;
    }
    AWLOG("I2C: %s\n", result == HAL_OK ? "PASSED" : "FAILED");
}

/**
 * @brief Convert time (seconds) to register value, register value range is 0x0
 * to 0xF
 *
 * @param time Input time (mini-seconds)
 * @return uint8_t Returned register value (4 bits, range 0x0 to 0xF)
 */
uint8_t aw2023_convert_time_to_register(uint16_t time)
{
    static const uint16_t time_table[16] = { 40, 130, 260, 380, 510, 770, 1040, 1600, 2100, 2600, 3100, 4200, 5200, 6200, 7300, 8300 };
    uint8_t               time_flag      = 0;

    if (time <= time_table[0])
    {
        time_flag = 0x0; // Minimum register value
    }
    else if (time >= time_table[15])
    {
        time_flag = 0xF; // Maximum register value
    }
    else
    {
        for (uint8_t i = 0; i < 15; ++i)
        {
            if (time >= time_table[i] && time < time_table[i + 1])
            {
                uint16_t midpoint = (time_table[i] + time_table[i + 1]) / 2;
                time_flag         = time < midpoint ? i : i + 1;
                break;
            }
        }
    }
    return time_flag;
}

void aw2023_enter_lowpower()
{
    if (aw2023_inited())
    {
        aw2023_chip_enable(AW_FALSE);
    }
}

void aw2023_exit_lowpower()
{
    initialized = AW_FALSE;
    aw2023_init();
}

#define COMMON_PWM_DIM 0x80

void aw2023_set_rgb_led_always_on(uint32_t color)
{
    AW_BOOL ret = AW_FALSE;
    // Parse red, green, blue components from the color
    AW_U8 red   = (color >> 16) & 0xFF; // Take the high 8 bits (red)
    AW_U8 green = (color >> 8) & 0xFF;  // Take the middle 8 bits (green)
    AW_U8 blue  = color & 0xFF;         // Take the low 8 bits (blue)

    AWLOG("Setting RGB LED with color: R=%d, G=%d, B=%d\n", red, green, blue);

    ret = aw2023_init();
    if (ret == AW_TRUE)
    {
        aw2023_set_pwm_mode(AW2023_PWM_EXPONENTIAL);
        aw2023_set_sync_mode(AW_TRUE);
        // Set the brightness of each LED (RGB corresponds to LED0, LED1, LED2
        // respectively)
        aw2023_set_led_current(AW2023_LED0, red);   // Red
        aw2023_set_led_current(AW2023_LED1, green); // Green
        aw2023_set_led_current(AW2023_LED2, blue);  // Blue

        // Set PWM dimming level
        aw2023_set_pwm_dimming(AW2023_LED0, COMMON_PWM_DIM);
        aw2023_set_led_operating_mode(AW2023_LED0, AW2023_MANUAL_MODE);

        aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_TRUE);
    }
}

/**
 * @brief Set RGB LED flash mode
 *
 * @param on_time Input time (mini-seconds)
 * @return uint8_t Returned register value (4 bits, range 0x0 to 0xF)
 */
void aw2023_set_rgb_led_flash(uint32_t color, RGB_TIME on_time, RGB_TIME off_time)
{
    AW_BOOL ret = AW_FALSE;
    // Parse red, green, blue components from the color
    AW_U8 red   = (color >> 16) & 0xFF; // Take the high 8 bits (red)
    AW_U8 green = (color >> 8) & 0xFF;  // Take the middle 8 bits (green)
    AW_U8 blue  = color & 0xFF;         // Take the low 8 bits (blue)

    AWLOG("Setting RGB LED with color: R=%d, G=%d, B=%d\n", red, green, blue);

    ret = aw2023_init();
    if (ret == AW_TRUE)
    {
        aw2023_set_pwm_mode(AW2023_PWM_EXPONENTIAL);
        aw2023_set_sync_mode(AW_TRUE);

        // Set the brightness of each LED (RGB corresponds to LED0, LED1, LED2
        // respectively)
        aw2023_set_led_current(AW2023_LED0, red);   // Red
        aw2023_set_led_current(AW2023_LED1, green); // Green
        aw2023_set_led_current(AW2023_LED2, blue);  // Blue

        // Set PWM dimming level
        aw2023_set_pwm_dimming(AW2023_LED0, COMMON_PWM_DIM);

        aw2023_set_on_time(AW2023_LED0, on_time);
        aw2023_set_off_time(AW2023_LED0, off_time);
        aw2023_set_delay_time(AW2023_LED0, AW2023_PAT_TIME_0_13);

        aw2023_set_repeat_time(AW2023_LED0, AW2023_BREATH_REPEAT_FOREVER);

        aw2023_set_led_operating_mode(AW2023_LED0, AW2023_PATTERN_MODE);

        aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_TRUE);
    }
}

/**
 * @brief Set RGB LED breathing mode
 *
 * @param on_time Input time (seconds)
 * @return uint8_t Returned register value (4 bits, range 0x0 to 0xF)
 */
void aw2023_set_rgb_led_breath(uint32_t color, RGB_TIME fade_on, RGB_TIME full_on, RGB_TIME fade_off, RGB_TIME full_off)
{
    AW_BOOL ret = AW_FALSE;
    // Parse red, green, blue components from the color
    AW_U8 red   = (color >> 16) & 0xFF; // Take the high 8 bits (red)
    AW_U8 green = (color >> 8) & 0xFF;  // Take the middle 8 bits (green)
    AW_U8 blue  = color & 0xFF;         // Take the low 8 bits (blue)

    AWLOG("Setting RGB LED with color: R=%d, G=%d, B=%d\n", red, green, blue);

    ret = aw2023_init();
    if (ret == AW_TRUE)
    {
        aw2023_set_pwm_mode(AW2023_PWM_EXPONENTIAL);
        aw2023_set_sync_mode(AW_TRUE);

        // Set the brightness of each LED (RGB corresponds to LED0, LED1, LED2
        // respectively)
        aw2023_set_led_current(AW2023_LED0, red);   // Red
        aw2023_set_led_current(AW2023_LED1, green); // Green
        aw2023_set_led_current(AW2023_LED2, blue);  // Blue

        // Set PWM dimming level
        aw2023_set_pwm_dimming(AW2023_LED0, COMMON_PWM_DIM);

        aw2023_set_on_time(AW2023_LED0, full_on);
        aw2023_set_off_time(AW2023_LED0, full_off);
        aw2023_set_delay_time(AW2023_LED0, AW2023_PAT_TIME_0_13);
        aw2023_set_rise_time(AW2023_LED0, fade_on);
        aw2023_set_fall_time(AW2023_LED0, fade_off);

        aw2023_set_repeat_time(AW2023_LED0, AW2023_BREATH_REPEAT_FOREVER);

        aw2023_set_led_operating_mode(AW2023_LED0, AW2023_PATTERN_MODE);

        aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_TRUE);
    }
}

/**
 * @brief Turn off all LEDs
 *
 * @return void
 */
void aw2023_led_off(void)
{
    if (!initialized)
    {
        return;
    }
    aw2023_led_enable_ctrl(AW2023_LED0_EN | AW2023_LED1_EN | AW2023_LED2_EN, AW_FALSE);
}
