/**
******************************************************************************
* @file    Touch_common.c
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the touch_common.c driver.
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
#include "Touch_common.h"

I2C_HandleTypeDef Touch_IIC;

int HAL_I2C_Setup(void)
{
    int ret = 0;
    memset(&Touch_IIC, 0, sizeof(I2C_HandleTypeDef));

    Touch_IIC.instance_id    = CONFIG_TOUCH_IIC_ID;
    Touch_IIC.init.wait      = 1;
    Touch_IIC.init.repeat    = 1;
    Touch_IIC.init.baudrate  = CONFIG_TOUCH_IIC_SPEED;
    Touch_IIC.init.rx_buf    = 0;
    Touch_IIC.init.rx_size   = 0;
    Touch_IIC.init.tx_buf    = 0;
    Touch_IIC.init.tx_size   = 0;
    Touch_IIC.init.cmd_buf   = 0;
    Touch_IIC.init.cmd_size  = 0;
    HAL_StatusTypeDef result = HAL_I2C_Init(&Touch_IIC);
    if (result != HAL_OK)
    {
        ret = -1;
    }
    return ret;
}

int Touch_iic_Init(void)
{
    int ret = -1;
    ret     = HAL_I2C_Setup();
    return ret;
}
