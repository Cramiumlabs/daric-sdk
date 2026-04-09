/**
******************************************************************************
* @file    lcd_common.h
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the lcd_common.c driver.
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
#ifndef TOUCH_COMMON_H
#define TOUCH_COMMON_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "daric_hal_def.h"
#include "daric_hal_i2c.h"
#include "system_daric.h"
#include "tx_api.h"
#include "daric_gpio.h"

extern I2C_HandleTypeDef Touch_IIC;
#endif
