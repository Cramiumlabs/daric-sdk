/**
******************************************************************************
* @file    rgb_led.h
* @author  PERIPHERIAL BSP Team
* @brief   This file define the const value used in rgb led module.
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
#ifndef RGB_LED_H
#define RGB_LED_H

typedef enum
{
    RGB_RED,
    RGB_GREEN,
    RGB_BLUE
} RGB_COLOR;

#ifdef CONFIG_RGB_LED_AW91xxx
#define RGB_LED 1
#endif

#define RGB_AW2023  1
#define RGB_AW91XXX 2

#if CONFIG_RGB_LED_CHIP == RGB_AW91XXX
typedef enum
{
    RGB_FADE_TIME_0000MS = 0X00,
    RGB_FADE_TIME_0315MS = 0X01,
    RGB_FADE_TIME_0630MS = 0X02,
    RGB_FADE_TIME_1260MS = 0X03,
    RGB_FADE_TIME_2520MS = 0X04,
    RGB_FADE_TIME_5040MS = 0X05
} RGB_TIME;
#elif CONFIG_RGB_LED_CHIP == RGB_AW2023
typedef enum
{
    RGB_FADE_TIME_0040MS = 0x0,
    RGB_FADE_TIME_0130MS = 0x1,
    RGB_FADE_TIME_0260MS = 0x2,
    RGB_FADE_TIME_0380MS = 0x3,
    RGB_FADE_TIME_0510MS = 0x4,
    RGB_FADE_TIME_0770MS = 0x5,
    RGB_FADE_TIME_1040MS = 0x6,
    RGB_FADE_TIME_1600MS = 0x7,
    RGB_FADE_TIME_2100MS = 0x8,
    RGB_FADE_TIME_2600MS = 0x9,
    RGB_FADE_TIME_3100MS = 0xA,
    RGB_FADE_TIME_4200MS = 0xB,
    RGB_FADE_TIME_5200MS = 0xC,
    RGB_FADE_TIME_6200MS = 0xD,
    RGB_FADE_TIME_7300MS = 0xE,
    RGB_FADE_TIME_8300MS = 0xF
} RGB_TIME;
#endif

#endif