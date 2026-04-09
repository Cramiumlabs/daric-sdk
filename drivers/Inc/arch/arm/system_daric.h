/*
 * Copyright 2024-2026 CrossBar, Inc.
 * Copyright (c) 2009-2021 Arm Limited. All rights reserved.
 *
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
 */

#ifndef SYSTEM_DARIC_H
#define SYSTEM_DARIC_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CONFIG_SYS_CLOCK_TICKS_PER_SEC
#define CONFIG_SYS_CLOCK_TICKS_PER_SEC 1000
#endif

#ifndef CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC
#define CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC 250000000
#endif

    /**
      \brief Exception / Interrupt Handler Function Prototype
    */
    typedef void (*VECTOR_TABLE_Type)(void);

    typedef struct
    {
        uint16_t freq;
        uint16_t voltmV;
        uint16_t voltRegData;
    } DVFS_VOLTAGE_TABLE_t;

#define DARIC_UDMACORE ((volatile DARIC_UDMACORE_TypeDef *)0x50100000)
#define DARIC_DUART    ((volatile DARIC_DUART_TypeDef *)0x40042000)
#define DARIC_IPC      ((volatile DARIC_SYSCTRL_IPC_TypeDef *)0x40040090)
#define DARIC_CGU      ((volatile DARIC_SYSCTRL_CGU_TypeDef *)0x40040000)
#define DARIC_SRAMCFG  ((volatile DARIC_CORE_SRAMCFG_TypeDef *)0x40014000)
#define DARIC_SRAMTRIM ((volatile DARIC_SRAMTRIM_TypeDef *)0x40045000)
#define DARIC_RERAM    ((volatile DARIC_RERAM_TypeDef *)0x40000000)

    /**
      \brief System Clock Frequency (Core Clock)
    */
    extern uint32_t SystemCoreClock;

    /**
      \brief Setup the microcontroller system.

       Initialize the System and update the SystemCoreClock variable.
     */
    extern void SystemInit(void);

    /*! \brief Put a character to the Debug-UART
     *
     * @param c The character to put to the Debug-UART
     */
    void sys_nopDelayMs(uint32_t ms);

    void     DUART_Init(void);
    void     DUART_PutChar(char c);
    void     initClockASIC(uint32_t freqHz, uint32_t targetClk1Hz, bool setVoltage);
    void     clkAnalysis();
    void     dvfs(uint32_t freq);
    uint32_t clkGetClkTop_MHz(void);
    void     sys_nopDelayUs(uint32_t us);
    void     configSramTrim(bool isAbove900mV);
    uint64_t clkGetClkPer_Hz(uint32_t clkTopHz);
    int32_t  initClockSimple(uint32_t freq100MHz);
#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_DARIC_H */
