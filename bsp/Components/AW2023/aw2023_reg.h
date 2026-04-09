/*******************************************************************************
 **** Copyright (C), 2024-2024, Shanghai awinic technology Co.,Ltd.
                                               all rights reserved. ************
 *******************************************************************************
 * File Name     : aw2023_reg.h
 * Author        : awinic
 * Date          : 2024-01-30
 * Description   : .h file function description
 * Version       : 1.0
 * Function List :
 *
*******************************************************************************/

#ifndef __AW2023_REG_H__
#define __AW2023_REG_H__

/* register address */
#define AW2023_REG_RESET					(0x00)
#define AW2023_REG_GCR1						(0x01)
#define AW2023_REG_ISR						(0x02)
#define AW2023_REG_PATST					(0x03)
#define AW2023_REG_GCR2						(0x04)
#define AW2023_REG_LCTR						(0x30)
#define AW2023_REG_LCFG0					(0x31)
#define AW2023_REG_LCFG1					(0x32)
#define AW2023_REG_LCFG2					(0x33)
#define AW2023_REG_PWM0						(0x34)
#define AW2023_REG_PWM1						(0x35)
#define AW2023_REG_PWM2						(0x36)
#define AW2023_REG_LED0T0					(0x37)
#define AW2023_REG_LED0T1					(0x38)
#define AW2023_REG_LED0T2					(0x39)
#define AW2023_REG_LED1T0					(0x3A)
#define AW2023_REG_LED1T1					(0x3B)
#define AW2023_REG_LED1T2					(0x3C)
#define AW2023_REG_LED2T0					(0x3D)
#define AW2023_REG_LED2T1					(0x3E)
#define AW2023_REG_LED2T2					(0x3F)

/* aw2023 led num */
#define AW2023_LED0							(0)
#define AW2023_LED1							(1)
#define AW2023_LED2							(2)
#define AW2023_LED_NUM 						(3)

/* aw2023 reset */
#define AW2023_RESET_MASK					(0x55)

/* aw2023 chip enable */
#define AW2023_CHIPEN_POS					(0)
#define AW2023_CHIPEN_MASK					(0x1U << AW2023_CHIPEN_POS)
#define AW2023_CHIP_ENABLE					(0x01)
#define AW2023_CHIP_DISABLE					(0x00)

/* aw2023 imax */
#define AW2023_IMAX_POS						(0)
#define AW2023_IMAX_MASK					(0x3U << AW2023_IMAX_POS)
#define AW2023_IMAX_15MA					(0x00)
#define AW2023_IMAX_30MA					(0x01)
#define AW2023_IMAX_5MA						(0x02)
#define AW2303_IMAX_10MA					(0x03)

/* aw2023 led enable ctrl */
#define AW2023_LED_ENABLE_CTRL_POS			(0)
#define AW2023_LED_ENABLE_CTRL_MASK			(0x7U << AW2023_LED_ENABLE_CTRL_POS)
#define AW2023_LED0_EN						(0x01)
#define AW2023_LED1_EN						(0x02)
#define AW2023_LED2_EN						(0x04)

/* aw2303 pwm transition mode select */
#define AW2023_PWM_MODE_SELECTION_POS		(3)
#define AW2023_PWM_MODE_SELECTION_MASK		(0x1U << AW2023_PWM_MODE_SELECTION_POS)
#define AW2023_PWM_EXPONENTIAL				(0x0U)
#define AW2023_PWM_LINEAR					(0x1U)

/* aw2023 pwm freq select */
#define AW2023_PWM_FREQ_POS					(5)
#define AW2023_PWM_FREQ_MASK				(0x1U << AW2023_PWM_FREQ_POS)
#define AW2023_PWM_FREQ_250HZ				(0x0U)
#define AW2023_PWM_FREQ_125HZ				(0x1U)

/* aw2023 led mode configure */
#define AW2023_LED_CUR_POS					(0)
#define AW2023_LED_CUR_MASK					(0xFU << AW2023_LED_CUR_POS)
#define AW2023_LED_MD_POS					(4)
#define AW2023_LED_MD_MASK					(0x1U << AW2023_LED_MD_POS)
#define AW2023_LED_FI_POS					(5)
#define AW2023_LED_FI_MASK					(0x1U << AW2023_LED_FI_POS)
#define AW2023_LED_FO_POS					(6)
#define AW2023_LED_FO_MASK					(0x1U << AW2023_LED_FO_POS)
#define AW2023_LED_SYNC_POS					(7)
#define AW2023_LED_SYNC_MASK				(0x1U << AW2023_LED_SYNC_POS)

/* aw2023 led operating mode select */
#define AW2023_MANUAL_MODE					(0)
#define AW2023_PATTERN_MODE					(1)

/* aw2023 T1&T2 configure */
#define AW2023_ON_TIME_POS					(0)
#define AW2023_ON_TIME_MASK					(0xFU << AW2023_ON_TIME_POS)
#define AW2023_RISE_TIME_POS				(4)
#define AW2023_RISE_TIME_MASK				(0xFU << AW2023_RISE_TIME_POS)

/* aw2023 T3&T4 configure */
#define AW2023_OFF_TIME_POS					(0)
#define AW2023_OFF_TIME_MASK				(0xFU << AW2023_OFF_TIME_POS)
#define AW2023_FALL_TIME_POS				(4)
#define AW2023_FALL_TIME_MASK				(0xFU << AW2023_FALL_TIME_POS)

/* aw2023 T0&REPEAT configure */
#define AW2023_REPEAT_TIME_POS				(0)
#define AW2023_REPEAT_TIME_MASK				(0xFU << AW2023_REPEAT_TIME_POS)
#define AW2023_DELAY_TIME_POS				(4)
#define AW2023_DELAY_TIME_MASK				(0xFU << AW2023_DELAY_TIME_POS)

/* aw2023 pattern time */
#define AW2023_PAT_TIME_0_04				(0x0)
#define AW2023_PAT_TIME_0_13				(0x1)
#define AW2023_PAT_TIME_0_26				(0x2)
#define AW2023_PAT_TIME_0_38				(0x3)
#define AW2023_PAT_TIME_0_51				(0x4)
#define AW2023_PAT_TIME_0_77				(0x5)
#define AW2023_PAT_TIME_1_04				(0x6)
#define AW2023_PAT_TIME_1_60				(0x7)
#define AW2023_PAT_TIME_2_10				(0x8)
#define AW2023_PAT_TIME_2_60				(0x9)
#define AW2023_PAT_TIME_3_10				(0xA)
#define AW2023_PAT_TIME_4_20				(0xB)
#define AW2023_PAT_TIME_5_20				(0xC)
#define AW2023_PAT_TIME_6_20				(0xD)
#define AW2023_PAT_TIME_7_30				(0xE)
#define AW2023_PAT_TIME_8_30				(0xF)

#define AW2023_BREATH_REPEAT_FOREVER		(0)

#endif /* #ifndef __AW2023_REG_H__ */
