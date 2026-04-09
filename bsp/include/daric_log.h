/**
******************************************************************************
* @file    daric_log.h
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the LOG macro
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
#ifndef DARIC_LOG_H
#define DARIC_LOG_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LOG_LEVEL_E 4
#define LOG_LEVEL_W 3
#define LOG_LEVEL_I 2
#define LOG_LEVEL_D 1
#define LOG_LEVEL_V 0

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_I
#endif

#define RMV_PTH(file)           (strrchr(file, '/') + 1)
#define LOG(LEVEL, format, ...) printf("[%s:%d][%s] " LEVEL ": " format "\n", RMV_PTH(__FILE__), __LINE__, __FUNCTION__, ##__VA_ARGS__)

#if (LOG_LEVEL_E >= LOG_LEVEL)
#define LOGE(format, ...) LOG("E", format, ##__VA_ARGS__)
#else
#define LOGE(format, ...)
#endif

#if (LOG_LEVEL_W >= LOG_LEVEL)
#define LOGW(format, ...) LOG("W", format, ##__VA_ARGS__)
#else
#define LOGW(format, ...)
#endif

#if (LOG_LEVEL_I >= LOG_LEVEL)
#define LOGI(format, ...) LOG("I", format, ##__VA_ARGS__)
#else
#define LOGI(format, ...)
#endif

#if (LOG_LEVEL_D >= LOG_LEVEL)
#define LOGD(format, ...) LOG("D", format, ##__VA_ARGS__)
#else
#define LOGD(format, ...)
#endif

#if (LOG_LEVEL_V >= LOG_LEVEL)
#define LOGV(format, ...) LOG("V", format, ##__VA_ARGS__)
#else
#define LOGV(format, ...)
#endif

    // clang-format off
#ifdef LOG_STACK
#define STACK_STUB(stk, size) do {                                                                                                          \
        uint32_t btm = (uint32_t)stk + (uint32_t)size;                                                                                      \
        uint32_t use = btm - (uint32_t)&btm;                                                                                                \
        printf("================================================================================================ Stack used: %ld\n", use);  \
    } while(0) 

#define STACK_DUMP(stk, size) do {                                                                                                          \
        char    *s     = (char*)stk;                                                                                                        \
        uint32_t sz    = (uint32_t)size;                                                                                                    \
        uint16_t block = 0;                                                                                                                 \
        uint16_t i     = 0;                                                                                                                 \
        while (sz > 0) {                                                                                                                    \
            printf("------------------------------------------------------------------------------------------------  BLOCK %d\n", block);  \
            for (int r = 0; r < 16; r++) {                                                                                                  \
                printf(                                                                                                                     \
                    "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "                                      \
                    "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",                                    \
                    s[i + 0],  s[i + 1],  s[i + 2],  s[i + 3],  s[i + 4],  s[i + 5],  s[i + 6],  s[i + 7],                                  \
                    s[i + 8],  s[i + 9],  s[i + 10], s[i + 11], s[i + 12], s[i + 13], s[i + 14], s[i + 15],                                 \
                    s[i + 16], s[i + 17], s[i + 18], s[i + 19], s[i + 20], s[i + 21], s[i + 22], s[i + 23],                                 \
                    s[i + 24], s[i + 25], s[i + 26], s[i + 27], s[i + 28], s[i + 29], s[i + 30], s[i + 31]);                                \
                i += 32;                                                                                                                    \
            }                                                                                                                               \
            sz -= 32 * 16;                                                                                                                  \
            block++;                                                                                                                        \
        }                                                                                                                                   \
        STACK_STUB(stk, size);                                                                                                              \
    } while(0)
#else
#define  STACK_STUB(stk, size)
#define  STACK_DUMP(stk, size)
#endif
    // clang-format on

#ifdef __cplusplus
}
#endif
#endif // DARIC_LOG_H