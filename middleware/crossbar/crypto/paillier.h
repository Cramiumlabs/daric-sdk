/**
 ******************************************************************************
 * @file    paillier.h
 * @author  SCE Team
 * @brief   Header file for Paillier operations.
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

#ifndef __PAILLIER_H__
#define __PAILLIER_H__

// #include "drv_conf.h"
// #ifdef DRV_SCE_ENABLED
#include "daric_util.h"
#include "daric.h"
#include "hal_api.h"

#define pQBitDiff 3

#define paillierBitLen  (safePrimeBitLen * 2)
#define paillierByteLen (paillierBitLen / 8)
#define paillierWordLen (paillierBitLen / 32)

typedef struct
{
    uint32_t data[paillierWordLen];
} paillier_t;

#define n2BitLen  (paillierBitLen * 2)
#define n2ByteLen (n2BitLen / 8)
#define n2WordLen (n2BitLen / 32)

typedef struct
{
    uint32_t data[n2WordLen];
} pcipher_t;

typedef struct
{
    paillier_t *N;       // N = p*q is Public Key
    paillier_t *LambdaN; // LambdaN = lcm(p-1, q-1)
    paillier_t *PhiN;    // PhiN = (p-1)*(q-1)
    pcipher_t  *N2;      // N2 = N*N
    bigprime_t *P, *Q;   // safe prime
} key_paillier_t;

int PaillierGenerateKeyPair(key_paillier_t *keyp);
int PaillierEncrypt(paillier_t *msg, paillier_t *x, key_paillier_t *keyp, pcipher_t *cipher);
int PaillierDecrypt(pcipher_t *cipher, key_paillier_t *keyp, paillier_t *msg);
int PaillierHomoMult(uint32_t *msg, uint32_t msglen, uint32_t *c1, uint32_t c1len, key_paillier_t *keyp, uint32_t *result);
int PaillierHomoAdd(uint32_t *c1, uint32_t c1len, uint32_t *c2, uint32_t c2len, key_paillier_t *keyp, uint32_t *result);

#endif
// #endif