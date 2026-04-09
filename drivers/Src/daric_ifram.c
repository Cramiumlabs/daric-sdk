/**
 ******************************************************************************
 * @file    daric_ifram.c
 * @author  IFRAM Team
 * @brief   IFRAM HAL driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of IFRAM
 *          peripheral:
 *           + Initialization functions
 *           + IFRAM Space malloc and free
 *           + Get IFRAM usage information
 *
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
/* Includes ------------------------------------------------------------------*/
#include "daric_hal.h"
#include "daric_tlsf.h"

/** @cond Private macros
 * @{
 */
/* Private macros ------------------------------------------------------------*/

#define HAL_TLSF_LOCK   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_TLSF_UNLOCK HAL_UNLOCK

#define DARIC_TLSF_MAX(a, b) ((a) > (b) ? (a) : (b))

/** @endcond
 * @}
 */

daric_tlsf_state_t ifram_state;

/** @endcond
 * @}
 */
void IframMgr_Init(void *base, uint32_t size)
{
    ifram_state.tlsf       = daric_tlsf_create_with_pool(base, (uint32_t)size);
    ifram_state.total_size = size - daric_tlsf_size();
    ifram_state.cur_used   = 0;
    ifram_state.max_used   = 0;
}

void *IframMgr_Malloc(uint32_t size)
{

    void *p = NULL;
    if (ifram_state.tlsf)
    {
        HAL_TLSF_LOCK
        p = daric_tlsf_malloc(ifram_state.tlsf, size);
        if (p)
        {
            ifram_state.cur_used += (daric_tlsf_block_size(p) + sizeof(size_t));
            ifram_state.max_used = DARIC_TLSF_MAX(ifram_state.cur_used, ifram_state.max_used);
        }
        HAL_TLSF_UNLOCK
    }
    return p;
}

void *IframMgr_MallocAlign(uint32_t size, uint32_t Alignsize)
{

    void *p = NULL;
    if (ifram_state.tlsf)
    {
        HAL_TLSF_LOCK
        p = daric_tlsf_memalign(ifram_state.tlsf, Alignsize, size);
        if (p)
        {
            ifram_state.cur_used += (daric_tlsf_block_size(p) + sizeof(size_t));
            ifram_state.max_used = DARIC_TLSF_MAX(ifram_state.cur_used, ifram_state.max_used);
        }
        HAL_TLSF_UNLOCK
    }
    return p;
}

void *IframMgr_Realloc(void *ptr, uint32_t size)
{

    void *p     = NULL;
    uint8_t new = 0;
    if (ifram_state.tlsf)
    {
        if (ptr == NULL)
            new = 1;
        HAL_TLSF_LOCK
        size_t old_size = daric_tlsf_block_size(ptr);
        p               = daric_tlsf_realloc(ifram_state.tlsf, ptr, size);
        if (p)
        {
            ifram_state.cur_used -= old_size;
            if (new)
                ifram_state.cur_used += (daric_tlsf_block_size(p) + sizeof(size_t));
            else
                ifram_state.cur_used += daric_tlsf_block_size(p);
            ifram_state.max_used = DARIC_TLSF_MAX(ifram_state.cur_used, ifram_state.max_used);
        }
        HAL_TLSF_UNLOCK
    }
    return p;
}

void IframMgr_Free(void *ptr)
{
    if (ifram_state.tlsf)
    {
        HAL_TLSF_LOCK
        size_t free_size = daric_tlsf_block_size(ptr);
        daric_tlsf_free(ifram_state.tlsf, ptr);
        if (ifram_state.cur_used > (free_size + sizeof(size_t)))
            ifram_state.cur_used -= (free_size + sizeof(size_t));
        else
            ifram_state.cur_used = 0;
        HAL_TLSF_UNLOCK
    }
}

// Get the actual remaining space, minus the control block overhead and block
// overhead
uint32_t IframMgr_Getleftsize(void)
{
    return (uint32_t)(ifram_state.total_size - ifram_state.cur_used);
}

// Get the maximum used size
uint32_t IframMgr_Getusedmaxsize(void)
{
    return (uint32_t)(ifram_state.max_used);
}