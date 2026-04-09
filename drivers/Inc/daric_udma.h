/**
 ******************************************************************************
 * @file    daric_udma.h
 * @author  UDMA Team
 * @brief   Header file of UDMA HAL module.
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

#ifndef __DRV_UDMA_H__
#define __DRV_UDMA_H__

#include "daric_hal_conf.h"

#ifdef HAL_UDMA_MODULE_ENABLED
#include "daric_udma_v3.h"
#include <stdint.h>

/*
 * uDMA map
 * uDMA-->Periph-->channel(tx/rx/cmd)
Periph id   Periph        offset             event(base Interrupt vector 64)
0           UART0         0x01000            0~3
1           UART1         0x02000            4~7
2           UART2         0x03000            8~11
3           UART3         0x04000            12~15
4           SPI0          0x05000            16~19
5           SPI1          0x06000            20~23
6           SPI2          0x07000            24~27
7           SPI3          0x08000            28~31
8           I2C0          0x09000            32~35
9           I2C1          0x0A000            36~39
10          I2C2          0x0B000            40~43
11          I2C3          0x0C000            44~47
12          SDIO          0x0D000            48~51
13          I2S           0x0E000            52~55
14          Camif         0xOF000            56~59
15          Filter        0x10000            60~63
*/

#define UDMA_BASE_ADDR     0x50100000
#define UDMA_UART_OFFSET   0x01000
#define UDMA_SPIM_OFFSET   0x05000
#define UDMA_I2C_OFFSET    0x09000
#define UDMA_SDIO_OFFSET   0x0D000
#define UDMA_I2S_OFFSET    0x0E000
#define UDMA_CAMIF_OFFSET  0x0F000
#define UDMA_FILTER_OFFSET 0x10000
#define UDMA_SCIF_OFFSET   0x11000
#define UDMA_SPS_OFFSET    0x12000

extern uint32_t HAL_UDMA_Get_Perh_Clock();

#endif /* HAL_UDMA_MODULE_ENABLED */

#endif /* __DRV_UDMA_H__ */
