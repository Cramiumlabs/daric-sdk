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
                                               all rights reserved. ************
 *******************************************************************************
 * File Name     : aw2023.h
 * Author        : awinic
 * Date          : 2024-01-30
 * Description   : .h file function description
 * Version       : 1.0
 * Function List :
 *
*******************************************************************************/

#ifndef __AW2023_H__
#define __AW2023_H__
#include "stdio.h"
// #include "stm32h7xx_hal.h"
#include "aw2023_reg.h"
#include "aw_type.h"

/* Declare I2C handle Structure definition */
// extern I2C_HandleTypeDef hi2c1;

/* chip info */
#define AW2023_DEVICE_ADDR (0x45)
#define AW2023_CHIP_ID     (0x09)

/* I2c status */
#define I2C_RETRY_TIMES (0x5)
typedef enum
{
    AW_I2C_OK      = 0x0,
    AW_I2C_ERROR   = 0x1,
    AW_I2C_BUSY    = 0x2,
    AW_I2C_TIMEOUT = 0x3
} AW_I2C_StatusTypeDef;

AW_BOOL aw2023_play(void);

#define AWINIC_UART_DEBUG
#ifdef AWINIC_UART_DEBUG
#define AWINIC_I2C_NAME       "awinic_log"
#define AWLOG(format, arg...) printf("[%s] %s %d: " format, AWINIC_I2C_NAME, __func__, __LINE__, ##arg)
#else
#define AWLOG(format, arg...)
#endif

#endif
