/**
 ******************************************************************************
 * @file    daric_uart.h
 * @author  UART Team
 * @brief   Header file of UART HAL module.
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

/***********************************************************************
 * Copyright (c)  2023, XXXX Co.,Ltd .
 * All rights reserved.
 * Filename    : uart.h
 * Description : uart driver
 * Author(s)   : sheng
 * version     : 1.0
 * Modify date : 2023-02-02
 ***********************************************************************/
#ifndef __DARIC_UART_H__
#define __DARIC_UART_H__

#include "daric_hal_conf.h"
#include "daric_udma.h"

#define UDMA_UART_BASEADDR    (UDMA_BASE_ADDR + UDMA_UART_OFFSET)
#define UDMA_UART_PERIPH_SIZE (UDMA_PERIPH_AREA_SIZE)
#define UDMA_UART_RX_ADDR(id) (UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * id + UDMA_CHANNEL_RX_OFFSET)
#define UDMA_UART_TX_ADDR(id) (UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * id + UDMA_CHANNEL_TX_OFFSET)

#define UDMA_UART_RX_EVT      0
#define UDMA_UART_TX_EVT      1
#define UDMA_UART_TX_CHAR_EVT 2
#define UDMA_UART_ERR_EVT     3

//
// UART custom registers offset definition
//
#define UART_RX_SIZE_OFFSET (0x04)
#define UART_RX_CFG_OFFSET  (0x08)
#define UART_TX_CFG_OFFSET  (0x18)
#define UART_STATUS_OFFSET  (0x20)
#define UART_SETUP_OFFSET   (0x24)
#define UART_ERROR_OFFSET   (0x28)
#define UART_IRQ_OFFSET     (0x2C)
#define UART_VALID_OFFSET   (0x30)
#define UART_DATA_OFFSET    (0x34)

//
// UART custom registers bitfields offset, mask, value definition
//

// CFG Register BITFIELD
#define UART_RX_CONTINOUS_OFFSET 0
#define UART_RX_EN_OFFSET        4
#define UART_RX_PENDING_OFFSET   5
#define UART_RX_CLEAR_OFFSET     6

#define UART_TX_CONTINOUS_OFFSET 0
#define UART_TX_EN_OFFSET        4
#define UART_TX_PENDING_OFFSET   5
#define UART_TX_CLEAR_OFFSET     6

// STATUS Register
#define UART_TX_BUSY_OFFSET 0
#define UART_TX_BUSY_WIDTH  1
#define UART_TX_BUSY_MASK   (0x1 << UART_TX_BUSY_OFFSET)
#define UART_TX_BUSY        (0x1 << UART_TX_BUSY_OFFSET)

#define UART_RX_BUSY_OFFSET 1
#define UART_RX_BUSY_WIDTH  1
#define UART_RX_BUSY_MASK   (0x1 << UART_RX_BUSY_OFFSET)
#define UART_RX_BUSY        (0x1 << UART_RX_BUSY_OFFSET)

// SETUP  Register
#define UART_PARITY_OFFSET 0
#define UART_PARITY_WIDTH  1
#define UART_PARITY_MASK   (0x1 << UART_PARITY_OFFSET)
#define UART_PARITY_DIS    (0 << UART_PARITY_OFFSET)
#define UART_PARITY_ENA    (1 << UART_PARITY_OFFSET)

#define UART_BIT_LENGTH_OFFSET 1
#define UART_BIT_LENGTH_WIDTH  2
#define UART_BIT_LENGTH_MASK   (0x3 << UART_BIT_LENGTH_OFFSET)
#define UART_BIT_LENGTH_5      (0 << UART_BIT_LENGTH_OFFSET)
#define UART_BIT_LENGTH_6      (1 << UART_BIT_LENGTH_OFFSET)
#define UART_BIT_LENGTH_7      (2 << UART_BIT_LENGTH_OFFSET)
#define UART_BIT_LENGTH_8      (3 << UART_BIT_LENGTH_OFFSET)

#define UART_STOP_BITS_OFFSET 3
#define UART_STOP_BITS_WIDTH  1
#define UART_STOP_BITS_MASK   (0x1 << UART_STOP_BITS_OFFSET)
#define UART_STOP_BITS_1      (0 << UART_STOP_BITS_OFFSET)
#define UART_STOP_BITS_2      (1 << UART_STOP_BITS_OFFSET)

#define UART_TX_OFFSET 8
#define UART_TX_WIDTH  1
#define UART_TX_MASK   (0x1 << UART_TX_OFFSET)
#define UART_TX_DIS    (0 << UART_TX_OFFSET)
#define UART_TX_ENA    (1 << UART_TX_OFFSET)

#define UART_RX_OFFSET 9
#define UART_RX_WIDTH  1
#define UART_RX_MASK   (0x1 << UART_RX_OFFSET)
#define UART_RX_DIS    (0 << UART_RX_OFFSET)
#define UART_RX_ENA    (1 << UART_RX_OFFSET)

#define UART_CLKDIV_OFFSET 16
#define UART_CLKDIV_WIDTH  16
#define UART_CLKDIV_MASK   (0xffff << UART_CLKDIV_OFFSET)
#define UART_CLKDIV(val)   (val << UART_CLKDIV_OFFSET)

#endif /* __DARIC_UART_H__ */
