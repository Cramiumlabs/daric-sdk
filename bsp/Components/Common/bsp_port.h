/**
******************************************************************************
* @file    bsp_port.h
* @author  PERIPHERIAL BSP Team
* @brief   This file define the port interface used in the prepherial device.
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
#ifndef BSP_PORT_H
#define BSP_PORT_H

#include <stdint.h>

typedef struct
{
    void (*write)(uint8_t reg, uint8_t data);
    void (*read)(uint8_t reg, uint8_t len, uint8_t *pbuf);
} BSP_PORT_F;

#endif