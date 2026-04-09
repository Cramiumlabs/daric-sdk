/**
 ******************************************************************************
 * @file    pke_curve.h
 * @author  SCE Team
 * @brief   Header file for PKE curve driver.
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

#ifndef __PKE_H__
#define __PKE_H__
#include <stdbool.h>
#include <stdint.h>
#include "pke_rsa.h"

// #define PKE_DEBUG

// #define PKE_LITTLE // Hardware endianness indicator, define this macro, the hardware will automatically flip the input big-endian data to little-endian for calculation, and the
// output result will be automatically flipped to big-endian data

#define littleEnd 0 // Point coordinate data stream uses little-endian format
#define bigEnd    1 // Point coordinate data stream uses big-endian format

extern const uint32_t ED25519_P_D[];
extern const uint32_t ED25519_L[];
extern const uint32_t ED25519_BASE_POINT[];

typedef struct
{
    uint8_t x[32];
    uint8_t y[32];
} pointDef;

typedef struct
{
    uint32_t x[17];
    uint32_t y[17];
} ec_point;

void sc_reduce(unsigned char *s);

// Calculate K's bitLen, input data is K's little-endian BYTE data stream
uint16_t count_one_bits(const uint8_t *inData, uint32_t inLen, uint8_t mode);
void     pke_calc_H(uint32_t *outBuf, uint32_t bitLen);
uint32_t pke_dataformat(const uint8_t *inData, uint8_t *outData, uint32_t len, uint32_t expectLen);

void sce_Sysinit_Pke(void);

void ed25519_init(uint16_t modLen);
void ed25519_PointAdd(const pointDef *pointA, const pointDef *pointB, pointDef *pointOut);
void ed25519_PointDouble(const pointDef *pointIn, pointDef *pointOut);
void ed25519_PointMul(const uint8_t *k, const pointDef *pointIn, pointDef *pointOut);
void ed25519_PointMul_Base(const uint8_t *in, uint32_t inLen, uint8_t inEndian, pointDef *pointOut, uint8_t mode);
void x25519_calc(const uint8_t *in, uint32_t inLen, pointDef *pointOut, uint8_t mode);

// #define brainpoolP256r1

#define SECP192K1 0x00
#define SECP192R1 0x80

#define SECP224K1 0x01
#define SECP224R1 0x81

#define SECP256K1 0x02
#define SECP256R1 0x82

#define SECP384R1 0x83

#define SECP521R1 0x84

extern const uint32_t *ec_basePonitPtr;
bool                   ecc_init(uint16_t modLen, uint8_t curveType);
void                   ecc_PointAdd(const uint32_t *pointA, const uint32_t *pointB, uint32_t *pointOut, uint16_t curveBitLen);
void                   ecc_PointDouble(const uint32_t *pointIn, uint32_t *pointOut, uint16_t curveBitLen);
void                   ecc_PointMul(const uint8_t *k, const uint32_t *pointIn, uint32_t *pointOut, uint16_t curveBitLen);
void                   ecc_PointMul_Base(const uint8_t *in, uint32_t *pointOut, uint16_t curveBitLen);

#define CURVE_192 192
#define CURVE_224 224
#define CURVE_256 256
#define CURVE_384 384
#define CURVE_521 544

void curve_modular_expo(const uint8_t *n,
                        uint32_t       nLen,
                        uint8_t        nEndian,
                        const uint8_t *x,
                        uint32_t       xLen,
                        uint8_t        xEndian,
                        const uint8_t *y,
                        uint32_t       yLen,
                        uint8_t        yEndian,
                        uint8_t       *out,
                        uint8_t        mode);
void curve_modular_inverse(const uint8_t *p, uint32_t pLen, const uint8_t *x, uint32_t xLen, uint8_t *out, uint16_t curveType, uint8_t mode); // 硬件未实现
void curve_modular_add(const uint32_t *n, const uint32_t *x, const uint32_t *y, uint32_t *out, uint16_t curveType);
void curve_modular_sub(const uint32_t *n, const uint32_t *x, const uint32_t *y, uint32_t *out, uint16_t curveType);
void curve_modular_mul(const uint32_t *n, const uint32_t *x, const uint32_t *y, uint32_t *out, uint16_t curveType);
void curve_rmodL(const uint8_t *in, uint32_t inLen, uint8_t *p, uint32_t pLen, uint8_t *outData);
void curve_rmodL2(const uint8_t *in, uint32_t inLen, uint8_t *p, uint32_t pLen, uint8_t *out);
void ed25519_init_EX(uint8_t *p, uint32_t pLen, uint8_t *d, uint32_t dLen, uint32_t modLen);

void x25519_init(uint16_t modLen);
void x25519_mul_base(const uint8_t *k, uint8_t *outbuf);
void x25519_mul(const uint8_t *k, const uint8_t *inbuf, uint8_t *outbuf);
void x25519_publickey(const uint8_t *private_key, uint8_t *public_key);
void x25519_sharekey(const uint8_t *private_key, const uint8_t *other_public_key, uint8_t *sharekey);

void secp256k1_init(uint16_t modLen);
void secp256r1_init(uint16_t modLen);

// The following calls the RSA interface
// void bn_expo_mod(bn_context_t *ctx, const uint32_t *x, const uint32_t *y, uint32_t xlen, uint32_t ylen, uint32_t *result);
void mdelay_systick(unsigned int ms);
#endif
