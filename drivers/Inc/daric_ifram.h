/**
 *******************************************************************************
 * @file    daric_ifram.h
 * @author  IFRAM Team
 * @brief   Header file for IFRAM module.
 *******************************************************************************
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
#ifndef DARIC_IFRAM_H
#define DARIC_IFRAM_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include <stdint.h>

    /* Initialization functions  ****************************/
    void IframMgr_Init(void *base, uint32_t size);

    /* Space malloc functions  ****************************/
    void *IframMgr_Malloc(uint32_t size);
    void *IframMgr_MallocAlign(uint32_t size, uint32_t Alignsize);
    void *IframMgr_Realloc(void *ptr, uint32_t size);
#define IFRAM_CALLOC(size)                 \
    ({                                     \
        void *ptr = IframMgr_Malloc(size); \
        if (ptr != NULL)                   \
        {                                  \
            memset(ptr, 0, size);          \
        }                                  \
        ptr;                               \
    })

    /* Space release functions  ****************************/
    void IframMgr_Free(void *ptr);

    /* IFRAM information function  ****************************/
    uint32_t IframMgr_Getleftsize(void);
    uint32_t IframMgr_Getusedmaxsize(void);

#ifdef __cplusplus
}
#endif

#endif // DARIC_IFRAM_H