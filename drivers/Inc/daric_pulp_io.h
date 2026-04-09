/**
 ******************************************************************************
 * @file    daric_pulp_io.h
 * @author  PULP IO Team
 * @brief   Header file of PULP IO HAL module.
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
/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HAL_PULP_IO__
#define __HAL_PULP_IO__

#include <stdint.h>

// Robin Van TODO:  from pulp-runtime-master/include/archi/utils.h
#define archi_write8(add, val_)  (*(volatile unsigned char *)(long)(add) = val_)
#define archi_write16(add, val_) (*(volatile unsigned short *)(long)(add) = val_)
#define archi_write32(add, val_) (*(volatile unsigned int *)(long)(add) = val_)
#define archi_write(add, val_)   (*(volatile unsigned int *)(long)(add) = val_)

#define archi_read8(add)  (*(volatile unsigned char *)(long)(add))
#define archi_read16(add) (*(volatile unsigned short *)(long)(add))
#define archi_read32(add) (*(volatile unsigned int *)(long)(add))
#define archi_read(add)   (*(volatile unsigned int *)(long)(add))

#define ARCHI_WRITE_VOL(base, offset, value) archi_write32((base) + (offset), (value))
#define ARCHI_WRITE(base, offset, value)     archi_write32((base) + (offset), (value))
#define ARCHI_READ(base, offset)             archi_read32((base) + (offset))

#define pulp_write8(add, val_)  (*(volatile unsigned char *)(long)(add) = val_)
#define pulp_write16(add, val_) (*(volatile unsigned short *)(long)(add) = val_)
#define pulp_write32(add, val_) (*(volatile unsigned int *)(long)(add) = val_)
#define pulp_write(add, val_)   (*(volatile unsigned int *)(long)(add) = val_)

#if 1

#define pulp_read8(add)  (*(volatile unsigned char *)(long)(add))
#define pulp_read16(add) (*(volatile unsigned short *)(long)(add))
#define pulp_read32(add) (*(volatile unsigned int *)(long)(add))
#define pulp_read(add)   (*(volatile unsigned int *)(long)(add))

#else

static inline uint8_t pulp_read8(uint32_t add)
{
    __asm__ __volatile__("" : : : "memory");
    uint8_t result = *(volatile uint8_t *)add;
    asm volatile("l.nop;");
    __asm__ __volatile__("" : : : "memory");
    return result;
}

static inline uint16_t pulp_read16(uint32_t add)
{
    __asm__ __volatile__("" : : : "memory");
    uint16_t result = *(volatile uint16_t *)add;
    asm volatile("nop;");
    __asm__ __volatile__("" : : : "memory");
    return result;
}

static inline uint32_t pulp_read32(uint32_t add)
{
    __asm__ __volatile__("" : : : "memory");
    uint32_t result = *(volatile uint32_t *)add;
    asm volatile("nop;");
    __asm__ __volatile__("" : : : "memory");
    return result;
}

static inline uint32_t pulp_read(uint32_t add)
{
    __asm__ __volatile__("" : : : "memory");
    uint32_t result = *(volatile uint32_t *)add;
    asm volatile("nop;");
    __asm__ __volatile__("" : : : "memory");
    return result;
}

#endif

#endif
