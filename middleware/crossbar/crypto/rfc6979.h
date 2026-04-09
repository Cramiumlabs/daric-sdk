/**
 ******************************************************************************
 * @file    rfc6979.h
 * @author  SCE Team
 * @brief   Header file for RFC6979 operations.
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

#ifndef __DARIC_RFC6979_H__
#define __DARIC_RFC6979_H__

// #include "drv_conf.h"
// #ifdef DRV_SCE_ENABLED
#include <stdint.h>

typedef struct
{
    uint8_t v[32];
    uint8_t k[32];
    int     retry;
} rfc6979_context;

uint32_t rfc6979_nonce_fn(const uint32_t *msg, const uint32_t *key, const uint32_t *data, uint32_t *nonce);
#endif
// #endif