/**
 ******************************************************************************
 * @file    daric_evb_nto_ts.h
 * @author  PERIPHERIAL BSP Team
 * @brief   This file contains the common defines and functions prototypes for
 *          the daric_evb_nto_ts.c driver.
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
#ifndef DARIC_EVB_NTO_TS_H
#define DARIC_EVB_NTO_TS_H

#include <stdint.h>
#include "hyn_cfg.h"
#include "tx_api.h"
#include "daric_errno.h"

struct hyn_ts_data;

typedef struct
{
    uint8_t  pos_id;
    uint8_t  event;
    uint16_t pos_x;
    uint16_t pos_y;
    uint16_t pres_z;
} pos_info;

typedef struct
{
    int32_t (*Init)(uint32_t, struct hyn_ts_data *);
    int32_t (*DeInit)(uint32_t);
    int32_t (*GetMultiTouchState)(uint32_t, pos_info *);
    int32_t (*EnableIT)(uint32_t);
    int32_t (*DisableIT)(uint32_t);
    int32_t (*EnterSleep)(uint32_t);
    int32_t (*ExitSleep)(uint32_t);
} TS_Drv_t;

typedef struct
{
    char           *name;
    const TS_Drv_t *driver;

} Ts_Drv_list;

extern const TS_Drv_t cst92xx_fuc_using;

extern TX_EVENT_FLAGS_GROUP gTouchEventGroup;
#define TOUCH_PANEL_EVENT (1UL << 0)

int32_t BSP_TS_Init(uint32_t Instance);
int32_t BSP_TS_DeInit(uint32_t Instance);
int32_t BSP_TS_EnableIT(uint32_t Instance);
int32_t BSP_TS_DisableIT(uint32_t Instance);
int32_t BSP_TS_Get_MultiTouchState(uint32_t Instance, pos_info *);
int32_t BSP_TS_EnterSleep(uint32_t Instance);
int32_t BSP_TS_ExitSleep(uint32_t Instance);
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif
#endif
