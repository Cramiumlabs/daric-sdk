/**
 ******************************************************************************
 * @file    daric_gpio.h
 * @author  GPIO Team
 * @brief   Header file of GPIO HAL module.
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

// See LICENSE file for license

#include <stddef.h>
#include <stdint.h>

#ifndef DARIC_GPIO_H
#define DARIC_GPIO_H

#define DARIC_GPIO_BASE       0x5012F000
#define DARIC_GPIO_OE_BASE    (DARIC_GPIO_BASE + 0x0148)
#define DARIC_GPIO_PORT_BASE  (DARIC_GPIO_BASE + 0x0130)
#define DARIC_GPIO_PU_BASE    (DARIC_GPIO_BASE + 0x0160)
#define DARIC_GPIO_PINS_BASE  (DARIC_GPIO_BASE + 0x0178)
#define DARIC_GPIO_AF_BASE    (DARIC_GPIO_BASE + 0x0000)
#define DARIC_GPIO_INTCR_BASE (DARIC_GPIO_BASE + 0x0100)

typedef struct
{
    uint16_t PA;
    uint16_t PAdummy;
    uint16_t PB;
    uint16_t PBdummy;
    uint16_t PC;
    uint16_t PCdummy;
    uint16_t PD;
    uint16_t PDdummy;
    uint16_t PE;
    uint16_t PEdummy;
    uint16_t PF;
    uint16_t PFdummy;
} DARIC_GPIO_PORT_T;

/* IO port */
enum GPIO_AFSEL_PORT
{
    AFSEL_PAL = DARIC_GPIO_BASE,
    AFSEL_PAH = DARIC_GPIO_BASE + 0x0004,
    AFSEL_PBL = DARIC_GPIO_BASE + 0x0008,
    AFSEL_PBH = DARIC_GPIO_BASE + 0x000c,
    AFSEL_PCL = DARIC_GPIO_BASE + 0x0010,
    AFSEL_PCH = DARIC_GPIO_BASE + 0x0014,
    AFSEL_PDL = DARIC_GPIO_BASE + 0x0018,
    AFSEL_PDH = DARIC_GPIO_BASE + 0x001c,
    AFSEL_PEL = DARIC_GPIO_BASE + 0x0020,
    AFSEL_PEH = DARIC_GPIO_BASE + 0x0024,
    AFSEL_PFL = DARIC_GPIO_BASE + 0x0028,
    AFSEL_PFH = DARIC_GPIO_BASE + 0x002c,
};

enum GPIO_OUT_PORT
{
    OUT_PA = DARIC_GPIO_BASE + 0x0130,
    OUT_PB = DARIC_GPIO_BASE + 0x0134,
    OUT_PC = DARIC_GPIO_BASE + 0x0138,
    OUT_PD = DARIC_GPIO_BASE + 0x013c,
    OUT_PE = DARIC_GPIO_BASE + 0x0140,
    OUT_PF = DARIC_GPIO_BASE + 0x0144,
};
enum GPIO_OUT_EN_PORT
{
    OUT_EN_PA = DARIC_GPIO_BASE + 0x0148,
    OUT_EN_PB = DARIC_GPIO_BASE + 0x014c,
    OUT_EN_PC = DARIC_GPIO_BASE + 0x0150,
    OUT_EN_PD = DARIC_GPIO_BASE + 0x0154,
    OUT_EN_PE = DARIC_GPIO_BASE + 0x0158,
    OUT_EN_PF = DARIC_GPIO_BASE + 0x015c,
};
enum GPIO_PULL_UP_PORT
{
    PULL_UP_PA = DARIC_GPIO_BASE + 0x0160,
    PULL_UP_PB = DARIC_GPIO_BASE + 0x0164,
    PULL_UP_PC = DARIC_GPIO_BASE + 0x0168,
    PULL_UP_PD = DARIC_GPIO_BASE + 0x016c,
    PULL_UP_PE = DARIC_GPIO_BASE + 0x0170,
    PULL_UP_PF = DARIC_GPIO_BASE + 0x0174,
};
enum GPIO_INPUT_STATUS_PORT
{
    IN_PA = DARIC_GPIO_BASE + 0x0178,
    IN_PB = DARIC_GPIO_BASE + 0x017c,
    IN_PC = DARIC_GPIO_BASE + 0x0180,
    IN_PD = DARIC_GPIO_BASE + 0x0184,
    IN_PE = DARIC_GPIO_BASE + 0x0188,
    IN_PF = DARIC_GPIO_BASE + 0x018c,
};
enum GPIO_AFSEL_VALUE
{
    AFSEL_GPIO = 0,
    AFSEL_AF1  = 1,
    AFSEL_AF2  = 2,
    AFSEL_AF3  = 3,
};

static volatile DARIC_GPIO_PORT_T *const GPIO_OUT = (DARIC_GPIO_PORT_T *)DARIC_GPIO_PORT_BASE;
static volatile DARIC_GPIO_PORT_T *const GPIO_IN  = (DARIC_GPIO_PORT_T *)DARIC_GPIO_PINS_BASE;
static volatile DARIC_GPIO_PORT_T *const GPIO_OE  = (DARIC_GPIO_PORT_T *)DARIC_GPIO_OE_BASE;
static volatile DARIC_GPIO_PORT_T *const GPIO_PU  = (DARIC_GPIO_PORT_T *)DARIC_GPIO_PU_BASE;

typedef struct
{
    uint32_t PAL;
    uint32_t PAH;
    uint32_t PBL;
    uint32_t PBH;
    uint32_t PCL;
    uint32_t PCH;
    uint32_t PDL;
    uint32_t PDH;
    uint32_t PEL;
    uint32_t PEH;
    uint32_t PFL;
    uint32_t PFH;
} DARIC_GPIO_PORT_AF_T;
static volatile DARIC_GPIO_PORT_AF_T *const GPIO_AF = (DARIC_GPIO_PORT_AF_T *)DARIC_GPIO_AF_BASE;

typedef struct DARIC_GPIO_INTCR_REG
{
    uint32_t intsel : 7;
    uint32_t intmode : 2;
    uint32_t inten : 1;
    uint32_t wkupe : 1;
    uint32_t rsv : 21;
} DARIC_GPIO_INTCR_REG_T;

typedef struct
{
    DARIC_GPIO_INTCR_REG_T INTCR0;
    DARIC_GPIO_INTCR_REG_T INTCR1;
    DARIC_GPIO_INTCR_REG_T INTCR2;
    DARIC_GPIO_INTCR_REG_T INTCR3;
    DARIC_GPIO_INTCR_REG_T INTCR4;
    DARIC_GPIO_INTCR_REG_T INTCR5;
    DARIC_GPIO_INTCR_REG_T INTCR6;
    DARIC_GPIO_INTCR_REG_T INTCR7;
} DARIC_GPIO_INTCR_T;
static volatile DARIC_GPIO_INTCR_T *const GPIO_INTCR = (DARIC_GPIO_INTCR_T *)DARIC_GPIO_INTCR_BASE;

typedef struct DARIC_GPIO_INTFR_REG
{
    uint32_t IT7 : 1;
    uint32_t IT6 : 1;
    uint32_t IT5 : 1;
    uint32_t IT4 : 1;
    uint32_t IT3 : 1;
    uint32_t IT2 : 1;
    uint32_t IT1 : 1;
    uint32_t IT0 : 1;
    uint32_t rsv : 24;
} DARIC_GPIO_INTFR_REG_T;

#endif // DARIC_GPIO_H
