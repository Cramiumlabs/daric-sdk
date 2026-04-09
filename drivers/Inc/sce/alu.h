/**
 ******************************************************************************
 * @file    alu.h
 * @author  SCE Team
 * @brief   Header file of ALU driver module.
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

#ifndef __SCE_ALU_H__
#define __SCE_ALU_H__
#include <stdbool.h>
#include <stdint.h>

#define ALU_INBIG              0x33
#define ALU_INBIG_OUTBIG       0xFF
#define ALU_INBIG_OUTLITTLE    0x33
#define ALU_INLITTLE_OUTBIG    0xCC
#define ALU_INLITTLE_OUTLITTLE 0x00

#define ALU_ENDIAN_MODE ALU_INLITTLE_OUTLITTLE

void printBuffer_be(char *name, uint8_t *ptr, uint32_t len);
void printBuffer_le(char *name, uint8_t *ptr, uint32_t len);

uint32_t alu_division(const uint32_t *X, const uint32_t *Y, uint32_t xlen, uint32_t ylen, uint32_t *Q, uint32_t *qlen, uint32_t *R, uint32_t *rlen);
uint32_t alu_sub(const uint32_t *X, const uint32_t *Y, uint32_t xlen, uint32_t ylen, uint32_t *subtraction, uint32_t *carry);
uint32_t alu_add(const uint32_t *X, const uint32_t *Y, uint32_t xlen, uint32_t ylen, uint32_t *addition, uint32_t *carry);
uint32_t alu_sft(const uint32_t *in, uint32_t inLen, uint32_t bitNum, uint32_t *out);
uint32_t alu_sftword(const uint32_t *in, uint32_t inLen, uint32_t wordNum, uint32_t *out);
uint32_t alu_blg(const uint32_t *X, const uint32_t *Y, uint32_t xlen, uint32_t ylen, uint32_t *outData);
uint32_t alu_bex(const uint32_t *in, uint32_t inLen, uint32_t *out);
uint32_t alu_bex_ex(uint32_t *in, uint32_t inLen, uint32_t *out);
#endif