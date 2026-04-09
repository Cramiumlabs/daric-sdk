/**
******************************************************************************
* @file    daric_gsensor.h
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the daric_gsensor.h driver.
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
#ifndef DARIC_GSENSOR_H
#define DARIC_GSENSOR_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*BSP_GSENSOR_CB)(uint32_t event, void *param);

typedef struct
{
    uint16_t size;
    int16_t *pX;
    int16_t *pY;
    int16_t *pZ;
} BSP_GSENSOR_FIFO_T;

#define BSP_GSENSOR_E_SLEEP  0x01 ///< One time event Sleep event
#define BSP_GSENSOR_E_WAKEUP 0x02 ///< One time event Wake up event
#define BSP_GSENSOR_E_FALL   0x04 ///< Free fall event
#define BSP_GSENSOR_E_FIFO   0x08 ///< Fifo full event
#define BSP_GSENSOR_E_MASK   0x0F
// #define BSP_GSENSOR_E_CLICK  0x10 ///< Click event
// #define BSP_GSENSOR_E_DCLICK 0x20 ///< Double click event
// #define BSP_GSENSOR_E_MASK   0x3F

int16_t BSP_GSENSOR_Init();
int16_t BSP_GSENSOR_DeInit();
int16_t BSP_GSENSOR_GetRawData(int16_t *x, int16_t *y, int16_t *z);
int16_t BSP_GSENSOR_GetFifoData(int16_t *x, int16_t *y, int16_t *z, uint8_t read_cnt);
int16_t BSP_GSENSOR_Register(BSP_GSENSOR_CB cb, uint32_t events);
void    BSP_GSENSOR_UnRegister();
void    BSP_GSENSOR_AddEvent(uint32_t events);
void    BSP_GSENSOR_RemoveEvent(uint32_t events);
#endif