/**
 ******************************************************************************
 * @file    daric_hal_def.h
 * @author  Def Team
 * @brief   Header file of Def HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DARIC_HAL_DEF
#define __DARIC_HAL_DEF

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric.h"

    /* Exported types ------------------------------------------------------------*/

    /**
     * @brief  HAL Status structures definition
     */
    typedef enum
    {
        HAL_OK      = 0x00U,
        HAL_ERROR   = 0x01U,
        HAL_BUSY    = 0x02U,
        HAL_TIMEOUT = 0x03U
    } HAL_StatusTypeDef;

    /* Exported macro ------------------------------------------------------------*/

#if !defined(UNUSED)
#define UNUSED(X) (void)X /* To avoid gcc/g++ warnings */
#endif                    /* UNUSED */

#define HAL_MAX_DELAY 0xFFFFFFFFU

#define HAL_IS_BIT_SET(REG, BIT) (((REG) & (BIT)) == (BIT))
#define HAL_IS_BIT_CLR(REG, BIT) (((REG) & (BIT)) == 0U)

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) /* ARM Compiler V6 */
#ifndef __weak
#define __weak __attribute__((weak))
#endif
#ifndef __packed
#define __packed __attribute__((packed))
#endif
#elif defined(__GNUC__) && !defined(__CC_ARM) /* GNU Compiler */
#ifndef __weak
#define __weak __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#endif /* __GNUC__ */

/* Macro to get variable aligned on 4-bytes, for __ICCARM__ the directive
 * "#pragma data_alignment=4" must be used instead */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) /* ARM Compiler V6 */
#ifndef __ALIGN_BEGIN
#define __ALIGN_BEGIN
#endif
#ifndef __ALIGN_END
#define __ALIGN_END __attribute__((aligned(4)))
#endif
#elif defined(__GNUC__) && !defined(__CC_ARM) /* GNU Compiler */
#ifndef __ALIGN_END
#define __ALIGN_END __attribute__((aligned(4)))
#endif /* __ALIGN_END */
#ifndef __ALIGN_BEGIN
#define __ALIGN_BEGIN
#endif /* __ALIGN_BEGIN */
#else
#ifndef __ALIGN_END
#define __ALIGN_END
#endif /* __ALIGN_END */
#ifndef __ALIGN_BEGIN
#if defined(__CC_ARM) /* ARM Compiler V5*/
#define __ALIGN_BEGIN __align(4)
#elif defined(__ICCARM__) /* IAR Compiler */
#define __ALIGN_BEGIN
#endif /* __CC_ARM */
#endif /* __ALIGN_BEGIN */
#endif /* __GNUC__ */

/* Macro to get variable aligned on 32-bytes,needed for cache maintenance
 * purpose */
#if defined(__GNUC__) /* GNU Compiler */
#define ALIGN_32BYTES(buf) buf __attribute__((aligned(32)))
#elif defined(__ICCARM__) /* IAR Compiler */
#define ALIGN_32BYTES(buf) _Pragma("data_alignment=32") buf
#elif defined(__CC_ARM) /* ARM Compiler */
#define ALIGN_32BYTES(buf) __align(32) buf
#endif

/**
 * @brief  __RAM_FUNC definition
 */
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
/* ARM Compiler V4/V5 and V6
   --------------------------
   RAM functions are defined using the toolchain options.
   Functions that are executed in RAM should reside in a separate source module.
   Using the 'Options for File' dialog you can simply change the 'Code / Const'
   area of a module to a memory space in physical RAM.
   Available memory areas are declared in the 'Target' tab of the 'Options for
   Target' dialog.
*/
#define __RAM_FUNC

#elif defined(__ICCARM__)
/* ICCARM Compiler
   ---------------
   RAM functions are defined using a specific toolchain keyword "__ramfunc".
*/
#define __RAM_FUNC __ramfunc

#elif defined(__GNUC__)
/* GNU Compiler
   ------------
  RAM functions are defined using a specific toolchain attribute
   "__attribute__((section(".RamFunc")))".
*/
#define __RAM_FUNC __attribute__((section(".RamFunc")))

#endif

/**
 * @brief  __NOINLINE definition
 */
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)) || defined(__GNUC__)
/* ARM V4/V5 and V6 & GNU Compiler
   -------------------------------
*/
#define __NOINLINE __attribute__((noinline))

#elif defined(__ICCARM__)
/* ICCARM Compiler
   ---------------
*/
#define __NOINLINE _Pragma("optimize = no_inline")

#endif

#ifdef __cplusplus
}
#endif

#endif /* ___DARIC_HAL_DEF */
