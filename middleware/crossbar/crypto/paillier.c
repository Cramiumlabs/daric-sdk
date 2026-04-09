/**
 ******************************************************************************
 * @file    paillier.c
 * @author  SCE Team
 * @brief   Paillier implementation file.
 *          This file provides firmware functions for Paillier
 *          homomorphic encryption operations.
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

#include <string.h>
#include "paillier.h"
// #ifdef DRV_SCE_ENABLED
#include "ll_api.h"
#include "pke_rsa.h"
#include "bn_util.h"
// #include "daric_printf.h"

// #define PAILLIER_API_DEBUG_EN

#if defined(PAILLIER_API_DEBUG_EN)
#define paillierdebug(format, ...) Debug_Print(format, ##__VA_ARGS__)
#else
#define paillierdebug(format, ...)
#endif

static bigprime_t P, Q;
static paillier_t N, PhiN, LambdaN;
static pcipher_t  N2;

int PaillierGenerateKeyPair(key_paillier_t *keyp)
{
    int         ret = 0;
    bigprime_t  Pn1, Qn1;
    paillier_t  vgcd = { 0 };
    bigprime_t *A, *B;

    if (keyp == NULL)
    {
        return ERR_PARAMETERS;
    }

    // Gen 2 Safe Primes for P, Q; (P-Q) bit length > (safePrimeBitLen - pQBitDiff)
    while (1)
    {
        // Q is prime, P = 2Q + 1 is Safe Prime; use Pn1, Qn1 as temp buffer for group 2
        GetSafePrime(&P, &Q, 20);
        GetSafePrime(&Pn1, &Qn1, 20);
        // Qn1 = |P - Pn1|, check bit length; (A>B)
        if (bnu_cmp_le(P.data, safePrimeWordLen, Pn1.data, safePrimeWordLen) > 0)
        {
            A = &P;
            B = &Pn1;
        }
        else
        {
            A = &Pn1;
            B = &P;
        }
        modulo_sub_le(A->data, B->data, ALLF_DATA, safePrimeWordLen, safePrimeWordLen, safePrimeWordLen, Qn1.data);
        uint32_t bitlen = bnu_get_msb_le(Qn1.data, safePrimeWordLen) + 1;
        paillierdebug("check bitlen of P - Q = %d\r\n", bitlen);
        if (bitlen >= safePrimeBitLen - pQBitDiff)
        {
            // Get the pair!!
            memcpy(Q.data, Pn1.data, safePrimeByteLen);
            break;
        }
    }

    // P, Q are safe prime pair from 2 group
    bn_sub_uint32_le(P.data, safePrimeWordLen, 1, Pn1.data); // Pn1 = P - 1
    bn_sub_uint32_le(Q.data, safePrimeWordLen, 1, Qn1.data); // Qn1 = Q - 1

    // PhiN = (P-1)*(Q-1)
    modulo_multiply_le(Pn1.data, Qn1.data, ALLF_DATA, safePrimeWordLen, safePrimeWordLen, paillierWordLen, PhiN.data);

    // N = P * Q
    modulo_multiply_le(P.data, Q.data, ALLF_DATA, safePrimeWordLen, safePrimeWordLen, paillierWordLen, N.data);

    // N2 = N * N
    modulo_multiply_le(N.data, N.data, ALLF_DATA, paillierWordLen, paillierWordLen, n2WordLen, N2.data);

    // lambdaN = lcm(P-1, Q-1) = (P-1)*(Q-1)/gcd
    gcd_le(Pn1.data, Qn1.data, safePrimeWordLen, safePrimeWordLen, vgcd.data);
    bndivision_le(PhiN.data, paillierWordLen, vgcd.data, paillierWordLen, LambdaN.data, NULL, NULL, NULL);

#if defined(PAILLIER_API_DEBUG_EN)
    bnu_print_mem_duart_be("Q", (uint8_t *)Q.data, safePrimeByteLen);
    bnu_print_mem_duart_be("P", (uint8_t *)P.data, safePrimeByteLen);
    bnu_print_mem_duart_be("Q-1", (uint8_t *)Qn1.data, safePrimeByteLen);
    bnu_print_mem_duart_be("P-1", (uint8_t *)Pn1.data, safePrimeByteLen);
    bnu_print_mem_duart_be("PhiN = (P-1)*(Q-1)", (uint8_t *)PhiN.data, paillierByteLen);
    bnu_print_mem_duart_be("N = P*Q", (uint8_t *)N.data, paillierByteLen);
    bnu_print_mem_duart_be("N2 = N*N", (uint8_t *)N2.data, n2ByteLen);
    bnu_print_mem_duart_be("vgcd = gcd(Pn1, Qn1)", (uint8_t *)vgcd.data, paillierByteLen);
    bnu_print_mem_duart_be("LambdaN", (uint8_t *)LambdaN.data, paillierByteLen);
#endif
    keyp->N       = &N;
    keyp->PhiN    = &PhiN;
    keyp->LambdaN = &LambdaN;
    keyp->N2      = &N2;
    keyp->P       = &P;
    keyp->Q       = &Q;

    return ret;
}

// msg is plain > 64bits; x is random < N, x length is paillierWordLen; cipher is output, length is n2WordLen
int PaillierEncrypt(paillier_t *msg, paillier_t *x, key_paillier_t *keyp, pcipher_t *cipher)
{
    int       ret = ERR_NONE;
    pcipher_t Gm, xN;

    if (bnu_cmp_le(keyp->N->data, paillierWordLen, x->data, paillierWordLen) <= 0)
    {
        return ERR_PARAMETERS;
    }

    // Gm = (N+1)^msg mod N2
    bn_add_uint32_le(keyp->N->data, paillierWordLen, 1, Gm.data); // Gm = N + 1
    modulo_expo_le(Gm.data, msg->data, keyp->N2->data, paillierWordLen, paillierWordLen, n2WordLen, Gm.data);

    // xN = x^N mod N2
    modulo_expo_le(x->data, keyp->N->data, keyp->N2->data, paillierWordLen, paillierWordLen, n2WordLen, xN.data);

    // cipher = Gm * xN mod N2
    modulo_multiply_le(Gm.data, xN.data, keyp->N2->data, n2WordLen, n2WordLen, n2WordLen, cipher->data);

    return ret;
}

// cipher is input, msg is output
int PaillierDecrypt(pcipher_t *cipher, key_paillier_t *keyp, paillier_t *msg)
{
    int       ret = ERR_NONE;
    uint32_t  tmp[n2WordLen];
    pcipher_t Ninv = { 0 }, Lc = { 0 }, Lg = { 0 };

    // 1. Lc = (c^LambdaN-1 mod N2) / N
    modulo_expo_le(cipher->data, keyp->LambdaN->data, keyp->N2->data, n2WordLen, paillierWordLen, n2WordLen, tmp); // tmp=cipher^LambdaN mod N2
    bn_sub_uint32_le(tmp, n2WordLen, 1, tmp);                                                                      // tmp = tmp - 1
    bndivision_le(tmp, n2WordLen, keyp->N->data, paillierWordLen, Lc.data, NULL, NULL, NULL);                      // Lc = tmp/N, length should be paillierWordLen

    // 2. Lg = (Gamma^LambdaN-1 mod N2) / N
    bn_add_uint32_le(keyp->N->data, paillierWordLen, 1, Lg.data);                                                   // Gamma = N + 1, temp in Lg
    modulo_expo_le(Lg.data, keyp->LambdaN->data, keyp->N2->data, paillierWordLen, paillierWordLen, n2WordLen, tmp); // tmp=Gamma^LambdaN mod N2
    bn_sub_uint32_le(tmp, n2WordLen, 1, tmp);                                                                       // Lg = Lg - 1
    bndivision_le(tmp, n2WordLen, keyp->N->data, paillierWordLen, Lg.data, NULL, NULL, NULL);                       // Lg = Lg/N, length should be paillierWordLen

    // 3. Lc/Lg mod N = Lc * (1/Lg mod N) mod N
    modulo_inverse_le(Lg.data, keyp->N->data, paillierWordLen, paillierWordLen, Ninv.data);
    modulo_multiply_le(Lc.data, Ninv.data, keyp->N->data, paillierWordLen, paillierWordLen, paillierWordLen, msg->data);
#if defined(PAILLIER_API_DEBUG_EN)
    bnu_print_mem_duart_be("PaillierDecrypt: Lc", (uint8_t *)Lc.data, paillierByteLen);
    bnu_print_mem_duart_be("PaillierDecrypt: Lg", (uint8_t *)Lg.data, paillierByteLen);
    bnu_print_mem_duart_be("PaillierDecrypt: 1/Lg", (uint8_t *)Ninv.data, paillierByteLen);
    bnu_print_mem_duart_be("PaillierDecrypt: msg", (uint8_t *)msg->data, paillierByteLen);
#endif
    return ret;
}

// result = c1^msg mod N2; msg is plain, c1 is cipher; length of result is n2WordLen
int PaillierHomoMult(uint32_t *msg, uint32_t msglen, uint32_t *c1, uint32_t c1len, key_paillier_t *keyp, uint32_t *result)
{
    int ret = ERR_NONE;
    // cipher^msg mod N2
    modulo_expo_le(c1, msg, keyp->N2->data, c1len, msglen, n2WordLen, result);
    return ret;
}

// result = c1*c2 mod N2; length of result is n2WordLen
int PaillierHomoAdd(uint32_t *c1, uint32_t c1len, uint32_t *c2, uint32_t c2len, key_paillier_t *keyp, uint32_t *result)
{
    int ret = ERR_NONE;
    // c1*c2 mod N2
    modulo_multiply_le(c1, c2, keyp->N2->data, c1len, c2len, n2WordLen, result);
    return ret;
}

// #endif
