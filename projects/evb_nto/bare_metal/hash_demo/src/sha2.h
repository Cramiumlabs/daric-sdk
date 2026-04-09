/**
 *******************************************************************************
 * @file    sha2.h
 * @author  Daric Team
 * @brief   Header file for sha2.h module.
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
#ifndef __SHA2_H__
#define __SHA2_H__

#include <stdint.h>
#include <stddef.h>
#include "byte_order.h"

#define SHA1_BLOCK_LENGTH           64
#define SHA1_DIGEST_LENGTH          20
#define SHA1_DIGEST_STRING_LENGTH   (SHA1_DIGEST_LENGTH * 2 + 1)
#define SHA256_BLOCK_LENGTH         64
#define SHA256_DIGEST_LENGTH        32
#define SHA256_DIGEST_STRING_LENGTH (SHA256_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH         128
#define SHA512_DIGEST_LENGTH        64
#define SHA512_DIGEST_STRING_LENGTH (SHA512_DIGEST_LENGTH * 2 + 1)

typedef struct _SHA1_CTX
{
    uint32_t state[5];
    uint64_t bitcount;
    uint32_t buffer[SHA1_BLOCK_LENGTH / sizeof(uint32_t)];
} SHA1_CTX;
typedef struct _SHA256_CTX
{
    uint32_t state[8];
    uint64_t bitcount;
    uint32_t buffer[SHA256_BLOCK_LENGTH / sizeof(uint32_t)];
} SHA256_CTX;
typedef struct _SHA512_CTX
{
    uint64_t state[8];
    uint64_t bitcount[2];
    uint64_t buffer[SHA512_BLOCK_LENGTH / sizeof(uint64_t)];
} SHA512_CTX;

extern const uint32_t sha256_initial_hash_value[8];
extern const uint64_t sha512_initial_hash_value[8];

void  sha1_Transform(const uint32_t *state_in, const uint32_t *data, uint32_t *state_out);
void  sha1_Init(SHA1_CTX *);
void  sha1_Update(SHA1_CTX *, const uint8_t *, size_t);
void  sha1_Final(SHA1_CTX *, uint8_t[SHA1_DIGEST_LENGTH]);
char *sha1_End(SHA1_CTX *, char[SHA1_DIGEST_STRING_LENGTH]);
void  sha1_Raw(const uint8_t *, size_t, uint8_t[SHA1_DIGEST_LENGTH]);
char *sha1_Data(const uint8_t *, size_t, char[SHA1_DIGEST_STRING_LENGTH]);

void  sha256_Transform(const uint32_t *state_in, const uint32_t *data, uint32_t *state_out);
void  sha256_Init(SHA256_CTX *);
void  sha256_Init_ex(SHA256_CTX *, const uint32_t state[8], uint64_t bitcount);
void  sha256_Update(SHA256_CTX *, const uint8_t *, size_t);
void  sha256_Final(SHA256_CTX *, uint8_t[SHA256_DIGEST_LENGTH]);
char *sha256_End(SHA256_CTX *, char[SHA256_DIGEST_STRING_LENGTH]);
void  sha256_Raw(const uint8_t *, size_t, uint8_t[SHA256_DIGEST_LENGTH]);
char *sha256_Data(const uint8_t *, size_t, char[SHA256_DIGEST_STRING_LENGTH]);

void  sha512_Transform(const uint64_t *state_in, const uint64_t *data, uint64_t *state_out);
void  sha512_Init(SHA512_CTX *);
void  sha512_Update(SHA512_CTX *, const uint8_t *, size_t);
void  sha512_Final(SHA512_CTX *, uint8_t[SHA512_DIGEST_LENGTH]);
char *sha512_End(SHA512_CTX *, char[SHA512_DIGEST_STRING_LENGTH]);
void  sha512_Raw(const uint8_t *, size_t, uint8_t[SHA512_DIGEST_LENGTH]);
char *sha512_Data(const uint8_t *, size_t, char[SHA512_DIGEST_STRING_LENGTH]);

#endif
