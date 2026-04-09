/**
 ******************************************************************************
 * @file    hal_api.h
 * @author  SCE Team
 * @brief   Header file for HAL API operations.
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

#ifndef __HAL_API_H__
#define __HAL_API_H__

// #include "drv_conf.h"
// #ifdef DRV_SCE_ENABLED

#include <stdint.h>
#include <stdbool.h>

#define safePrimeBitLen  (1024)
#define safePrimeByteLen (safePrimeBitLen / 8)
#define safePrimeWordLen (safePrimeBitLen / 32)

typedef struct
{
    uint32_t data[safePrimeWordLen];
} bigprime_t;

void bn_add_uint32_le(const uint32_t *input, uint32_t xlen, uint32_t data, uint32_t *output);
void bn_sub_uint32_le(const uint32_t *input, uint32_t xlen, uint32_t data, uint32_t *output);

int Miller_Rabin(const bigprime_t *n, int32_t reps);
int GetSafePrime(bigprime_t *P, bigprime_t *Q, int32_t reps);

int hal_mod(uint32_t *X, uint32_t *N, uint32_t x_len, uint32_t n_len, uint32_t *result);
int hal_floordiv(uint32_t *X, uint32_t *Y, uint32_t x_len, uint32_t y_len, uint32_t *result);
int hal_add(uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t x_len, uint32_t y_len, uint32_t n_len, uint32_t *result);
int hal_sub(uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t x_len, uint32_t y_len, uint32_t n_len, uint32_t *result);
int hal_multiply(uint32_t *X, uint32_t *Y, uint32_t *N, uint32_t x_len, uint32_t y_len, uint32_t n_len, uint32_t *result);
int hal_inverse(uint32_t *X, uint32_t *N, uint32_t x_len, uint32_t n_len, uint32_t *result);
int hal_exp(uint32_t *B, uint32_t *E, uint32_t *N, uint32_t b_len, uint32_t e_len, uint32_t n_len, uint32_t *result, uint32_t NisPrime);
int hal_sqrt(uint32_t *A, uint32_t awlen, uint32_t *result);

int LegendreSymbol(uint32_t *X, uint32_t *N, uint32_t x_len, uint32_t n_len);

void bn_random_le(uint32_t *rand, uint32_t len, const bigprime_t *lmt);
#endif
// #endif
