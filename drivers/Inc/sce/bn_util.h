/**
 ******************************************************************************
 * @file    bn_util.h
 * @author  SCE Team
 * @brief   Header file for big number utility functions.
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

#ifndef __BN_UTIL_C__
#define __BN_UTIL_C__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#define BNU_DEBUG(format, ...)    printf(format, ##__VA_ARGS__)
#define ASSERT_4BYTE_ALIGNED(ptr) assert(((uintptr_t)(ptr) & 0x3) == 0)

static inline int MAX(int x, int y)
{
    return x > y ? x : y;
}

static inline int MIN(int x, int y)
{
    return x > y ? y : x;
}

void    bnu_print_le(const uint32_t *bn, uint32_t len);
int32_t bnu_cmp_le(const uint32_t *A, uint32_t Alen, const uint32_t *B, uint32_t Blen);
int32_t bnu_cmp_be(const uint32_t *A, uint32_t Alen, const uint32_t *B, uint32_t Blen);
int     bnu_setbit_le(uint32_t *bn, uint32_t xlen, uint32_t stbits);
bool    bnu_is_zero(const uint32_t *bn, uint32_t size);

void bnu_leftshift_le(const uint32_t *input, uint32_t xlen, uint32_t stbits, uint32_t *output);
void bnu_rightshift_le(const uint32_t *input, uint32_t xlen, uint32_t stbits, uint32_t *output);
void bnu_copy(const uint32_t *in, int32_t len, uint32_t *out);
void bnu_fill_be(const uint32_t *in, uint32_t inlen, uint32_t *out, uint32_t outlen);
void bnu_fill_le(const uint32_t *in, uint32_t inlen, uint32_t *out, uint32_t outlen);

uint32_t bnu_le2be(const uint32_t *in, uint32_t *out, uint32_t wordlen);
uint32_t bnu_shift_be_num(uint32_t *buf, uint32_t wordlen);

void     bnu_swap_endian_2(const void *in, void *out, uint32_t len);
void     bnu_swap_endian_1(void *buffer, uint32_t len);
uint32_t bnu_swap_endian(uint32_t *in, uint32_t *out, uint32_t wordlen);
uint32_t bnu_is_same(uint32_t *A, uint32_t *B, uint32_t wordlen);

uint32_t bnu_get_nzw_le(const uint32_t *N, uint32_t nlen);
uint32_t bnu_get_nzw_be(const uint32_t *N, uint32_t nlen);
uint32_t bnu_get_msb_le(const uint32_t *N, uint32_t nlen);
uint32_t bnu_get_msb_be(const uint32_t *N, uint32_t nlen);

void bnu_print_mem(const char *name, const void *ptr, uint32_t len);
void bnu_print_mem_u32(const char *name, const uint32_t *ptr, uint32_t len);
void bnu_print_mem_duart(char *name, uint8_t *ptr, uint32_t len);
void bnu_print_mem_duart_be(char *name, uint8_t *ptr, uint32_t len);

uint32_t bnu_hex2buf_be(char *instr, uint8_t *outbuf, uint32_t buflen);
uint32_t bnu_hex2buf_le(char *instr, uint8_t *outbuf, uint32_t buflen);

int  bnu_memcpy_unaligned_src(void *dest, const void *src, size_t size);
void bnu_memcpy_u32(uint32_t *dst, const uint32_t *src, uint32_t len);
#endif // ! __BN_UTIL_C__