/*
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
 * ┌────────────────────────┬───────────────────────┬───────────┬──────────────────────┐
 * │    Address Range       │     Name in DARIC     │   Size    │ Name In Active Card  │
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 6000 0000 ~ 6001 FFFF  │                 Boot 0│     128 KB│              Boot ROM│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 6002 0000 ~ 6004 FFFF  │                 Boot 1│     192 KB│           Boot loader│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 6005 0000 ~ 602B FFFF  │               Firmware│    2496 KB│         User Firmware│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 602C 0000 ~ 602D 9FFF  │             Parameters│     104 KB│           LCD WF Data│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 602D A000 ~ 603D 9FFF  │                   Data│       1 MB│          FAT12 System│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 603D A000 ~ 603D BFFF  │       one-way counters│       8 KB│                      │
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 603D C000 ~ 603D FFFF  │ Access Contorl Setting│      16 KB│                      │
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 603E 0000 ~ 603E FFFF  │          Dataslot area│      64 KB│                      │
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 603F 0000 ~ 603F FFFF  │           Keyslot Area│      64 KB│                      │
 * └────────────────────────┴───────────────────────┴───────────┴──────────────────────┘
 */

// clang-format off
#define DARIC_MPU_CONFIG                                                            \
{                                                                                   \
    /* ITCM: rwx, Normal memory, Non-cacheable. */                                   \
    {                                                                               \
        ARM_MPU_RBAR(0, 0x00000000),                                                \
        ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_256KB)    \
    },                                                                              \
    /* DTCM: rw, Normal memory, Non-cacheable. */                                   \
    {                                                                               \
        ARM_MPU_RBAR(1, 0x20000000),                                                \
        ARM_MPU_RASR(1, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_64KB)   \
    },                                                                              \
    /* IFRAM: rw, Normal memory, Non-cacheable. */                                  \
    {                                                                               \
        ARM_MPU_RBAR(2, 0x50000000),                                                \
        ARM_MPU_RASR(1, ARM_MPU_AP_FULL, 1, 0, 0, 0, 0, ARM_MPU_REGION_SIZE_256KB)  \
    },                                                                              \
    /* ReRAM code(background): rx, Normal memory, Write-Through, no write allocate. */     \
    {                                                                               \
        ARM_MPU_RBAR(3, 0x60000000),                                                \
        ARM_MPU_RASR(1, ARM_MPU_AP_FULL, 0, 0, 1, 0, 0, ARM_MPU_REGION_SIZE_4MB)     \
    },                                                                              \
    /* ReRAM code(boot0/boot1/firmware): rx, Normal memory, Write-Through, no write allocate. */     \
    {                                                                               \
        ARM_MPU_RBAR(4, 0x60000000),                                                \
        ARM_MPU_RASR(0, ARM_MPU_AP_RO, 0, 0, 1, 0, 0, ARM_MPU_REGION_SIZE_2MB)     \
    },                                                                              \
    /* ReRAM code(boot0/boot1/firmware): rx, Normal memory, Write-Through, no write allocate. */     \
    {                                                                               \
        ARM_MPU_RBAR(5, 0x60200000),                                                \
        ARM_MPU_RASR(0, ARM_MPU_AP_RO, 0, 0, 1, 0, 0, ARM_MPU_REGION_SIZE_512KB)      \
    },                                                                              \
    /* ReRAM code(boot0/boot1/firmware): rx, Normal memory, Write-Through, no write allocate. */     \
    {                                                                               \
        ARM_MPU_RBAR(6, 0x60280000),                                                \
        ARM_MPU_RASR(0, ARM_MPU_AP_RO, 0, 0, 1, 0, 0, ARM_MPU_REGION_SIZE_256KB)      \
    },                                                                              \
    /* SRAM: rw, Normal memory, Write-Through, no write allocate, shareable. */     \
    {                                                                               \
        ARM_MPU_RBAR(7, 0x61000000),                                                \
        ARM_MPU_RASR(0, ARM_MPU_AP_FULL, 0, 1, 1, 0, 0, ARM_MPU_REGION_SIZE_2MB)    \
    },                                                                              \
};
// clang-format on