/**
******************************************************************************
* @file    daric_evb_nto_rgb_led.h
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the daric_evb_nto_rgb_led.h driver.
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
#ifndef DARIC_EVB_NTO_RGB_LED_H
#define DARIC_EVB_NTO_RGB_LED_H
#include "rgb_led.h"
#include <stdint.h>

void BSP_RGB_LED_On(uint8_t led_id, RGB_COLOR color, uint8_t dim);
void BSP_RGB_LED_Off(uint8_t led_id);
void BSP_RGB_LED_Blink(uint8_t led_id, RGB_COLOR color, RGB_TIME full_on, RGB_TIME full_off);
void BSP_RGB_LED_Breath(uint8_t led_id, RGB_COLOR color, RGB_TIME fade_on, RGB_TIME full_on, RGB_TIME fade_off, RGB_TIME full_off);

#endif /* DARIC_EVB_NTO_RGB_LED_H */
