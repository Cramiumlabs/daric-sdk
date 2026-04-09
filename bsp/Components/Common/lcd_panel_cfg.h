/**
******************************************************************************
* @file    lcd_panel_cfg.h
* @author  PERIPHERIAL BSP Team
* @brief   HAL LCD driver
           This file contains the common defines and functions prototypes for
           the lcd_common.c driver.
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

#ifndef __PANEL_CFG_H__
#define __PANEL_CFG_H__

#include "lcd_common.h"

// Supported LCD panels
extern BSP_LCD_Driver lcd_1680_driver;
extern BSP_LCD_Driver lcd_1685_driver;
extern BSP_LCD_Driver lcd_ed028tc1_driver;
extern BSP_LCD_Driver lcd_1312_driver;

// Supported LCD panels configuration
static BSP_LCD_Panel_Cfg supported_lcd[] = {
#ifdef CONFIG_LCD_SSD1312_SPI
    {
        .lcd_id = 0x1312,
        .drv    = &lcd_1312_driver,
    },
#endif
#ifdef CONFIG_LCD_ED028TC1_SPI
    {
        .lcd_id = 0x281,
        .drv    = &lcd_ed028tc1_driver,
    },
#endif
#ifdef CONFIG_LCD_SSD1685_SPI
    {
        .lcd_id = 0x1685,
        .drv    = &lcd_1685_driver,
    },
#endif
#ifdef CONFIG_LCD_SSD1680_SPI
    {
        .lcd_id = 0x1680,
        .drv    = &lcd_1680_driver,
    },
#endif
    {
        .lcd_id = 0x0000,
        .drv    = NULL,
    },
};

#endif // __PANEL_CFG_H__
