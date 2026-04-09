/**
 ******************************************************************************
 * @file    trng.h
 * @author  SCE Team
 * @brief   Header file for TRNG driver.
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

#ifndef __RNG_H__
#define __RNG_H__
#include "auth.h"
#include <stdbool.h>

void trng_continuous_contex_init(void);
bool trng_continuous_contex_get_buffer(uint32_t *buf, uint32_t size);
void trng_stop(trng_t *trng);
void rng_raw_data(void);

void getRng(uint32_t crsrc, uint32_t crana, uint32_t postProc, uint32_t opt, uint32_t chain0, uint32_t chain1, uint32_t *buf, uint32_t size, uint32_t sfr_ar);
#endif