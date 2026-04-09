/**
******************************************************************************
* @file    hyn_ts_ext.c
* @author  PERIPHERIAL BSP Team
* @brief   HAL TOUCH driver
           This file contains the common defines and functions prototypes for
           the hyn_cst92xx.c driver.
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

#include "hyn_core.h"

void hyn_irq_set(struct hyn_ts_data *ts_data, u8 value)
{
    // HYN_ENTER();
}

void hyn_set_i2c_addr(struct hyn_ts_data *ts_data, u8 addr)
{
    ts_data->salve_addr = addr;
}

u16 hyn_sum16(int val, u8 *buf, u16 len)
{
    u16 sum = val;
    while (len--)
        sum += *buf++;
    return sum;
}

u32 hyn_sum32(int val, u32 *buf, u16 len)
{
    u32 sum = val;
    while (len--)
        sum += *buf++;
    return sum;
}

void hyn_esdcheck_switch(struct hyn_ts_data *ts_data, u8 enable)
{
}

int copy_for_updata(struct hyn_ts_data *ts_data, u8 *buf, u32 offset, u16 len)
{
    int ret = -1;

    return ret;
}
