/**
 ******************************************************************************
 * @file    trng.c
 * @author  SCE Team
 * @brief   TRNG driver implementation.
 *          This file provides firmware functions to manage the TRNG
 *          peripheral for generating random numbers.
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
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "daric.h"
#include "trng.h"
#include "bn_util.h"

#define TRNG_CONTINUOUS_TIMEOUT 10000

static inline bool trng_raw_buf_ready(trng_t *trng)
{
#ifdef NEW_VERSION
    return trng->sr & (1 << 28);
#else
    return trng->sr & (1 << 24);
#endif
}

static inline void trng_fifo_enable(uint32_t mask)
{
    SCE_FIFO_ENABLE(mask);
}

static inline void trng_fifo_disable(uint32_t mask)
{
    SCE_FIFO_DISABLE(mask);
}

static inline void trng_start(trng_t *trng)
{
    trng->ar = 0x5a;
    __DSB();
}

void trng_stop(trng_t *trng)
{
    trng->ar = 0xa5;
    trng_fifo_disable(SCE_FIFO_5);
    __DSB();
}

static inline void trng_continuous_prepare(trng_t *trng)
{
    SCE_CLR_FIFO_5();

    trng->crsrc                  = 0xffff;
    trng->crana                  = 0xffff;
    trng->opt                    = 0x10040; /* use rngB , gen 0x40*4*4=1024 bytes */
    trng->postproc.postprocValue = 0xf821;
    trng->chain0                 = 0xFFFFFFFE; /* odd value of chain0 makes it more random */
    trng->chain1                 = 0xFFFFFFFC; /* chain1 less than chain0 makes it more random */
    __DSB();
    trng_start(trng);
}

static trng_continuous_t s_trng_ctx = { 0 };

void trng_continuous_contex_init(void)
{
    s_trng_ctx.trng = trng_hw;
    trng_fifo_enable(SCE_FIFO_5);
    trng_continuous_prepare(s_trng_ctx.trng);
    s_trng_ctx.remain = TRNG_CONTINUOUS_MAX_ATTEMPTS;
}

bool trng_continuous_contex_get_buffer(uint32_t *buf, uint32_t size)
{
    ASSERT_4BYTE_ALIGNED(buf);

    if (size == 0 || size > TRNG_CONTINUOUS_MAX_ATTEMPTS)
    {
        printf("TRNG: size not supported! %" PRIu32 "\n", size);
        return false;
    }

    if (s_trng_ctx.trng == NULL)
    {
        trng_continuous_contex_init();
    }

    if (s_trng_ctx.remain < size || SCE_FIFO_5_CNT() < size)
    {
        trng_continuous_prepare(s_trng_ctx.trng);
        s_trng_ctx.remain = TRNG_CONTINUOUS_MAX_ATTEMPTS;
    }

    uint32_t timeout = TRNG_CONTINUOUS_TIMEOUT;
    while (SCE_FIFO_5_CNT() < size && timeout-- > 0)
    {
        asm("nop");
    }

    if (SCE_FIFO_5_CNT() < size)
    {
        printf("TRNG: timeout!\n");
        return false;
    }

    bnu_memcpy_u32(buf, (void *)ADDR_RNG_SEG_RNGB, size);
    s_trng_ctx.remain -= size;
    return true;
}

void rng_raw_data(void)
{
    trng_continuous_t contex;

    contex.trng        = trng_hw;
    contex.trng->crsrc = 0xffff;
    contex.trng->crana = 0xffff;
    contex.trng->opt   = 0x20;

    contex.trng->postproc.bits.cr_reseed_intval  = 0x01;
    contex.trng->postproc.bits.cr_gen_intval     = 0x02;
    contex.trng->postproc.bits.cr_healthtest_len = 0x20;
    contex.trng->postproc.bits.cr_postproc_opt   = 0x00;
    contex.trng->postproc.bits.cr_drng_en        = 0x00;
    contex.trng->postproc.bits.cr_hlthtest_en    = 0x00;
    contex.trng->postproc.bits.cr_pfilter_en     = 0x00;
    contex.trng->postproc.bits.cr_gen_en         = 0x01;

    contex.trng->chain0 = 0xFFFFFFFE; /* odd value of chain0 makes it more random */
    contex.trng->chain1 = 0xFFFFFFFC; /* chain1 less than chain0 makes it more random */

    printf("sfr_crsrc = 0x%" PRIx32 "\r\n", contex.trng->crsrc);
    printf("sfr_crana = 0x%" PRIx32 "\r\n", contex.trng->crana);
    printf("sfr_postproc = 0x%x\r\n", contex.trng->postproc.postprocValue);
    printf("sfr_opt = 0x%" PRIx32 "\r\n", contex.trng->opt);

    trng_raw_buf_ready(contex.trng);

    printf("out\r\n");

    for (int j = 0; j < 32; j++) // 32 blocks
    {
        for (int i = 0; i < (131072 / 4); i++)
        {
            uint32_t val = trng_hw->buf;
            printf("%08" PRIX32, val);
        }
    }
}

void dumpBuf(uint8_t *ptr, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++)
    {
        printf("%02X", ptr[i]);
    }
}

void getRng(uint32_t crsrc, uint32_t crana, uint32_t postProc, uint32_t opt, uint32_t chain0, uint32_t chain1, uint32_t *buf, uint32_t size, uint32_t sfr_ar)
{
    uint32_t          remain, offset;
    uint8_t           tempBuf[1024];
    trng_continuous_t contex;
    contex.trng      = trng_hw;
    uint32_t genloop = 0;

    if (opt & 0x10000)
    {
        trng_fifo_enable(SCE_FIFO_5); /* RNG B */
    }
    else
    {
        trng_fifo_enable(SCE_FIFO_4); /* RNG A */
    }

    contex.trng->crsrc                  = crsrc;
    contex.trng->crana                  = crana;
    contex.trng->postproc.postprocValue = postProc;
    contex.trng->opt                    = (opt & 0x10000) | 0x40;
    contex.trng->chain0                 = chain0; /* odd value of chain0 makes it more random */
    contex.trng->chain1                 = chain1; /* chain1 less than chain0 makes it more random */

    __DSB();

    printf("sfr_crsrc = 0x%08" PRIx32 "\r\n", contex.trng->crsrc);
    printf("sfr_crana = 0x%08" PRIx32 "\r\n", contex.trng->crana);
    printf("sfr_postproc = 0x%08x\r\n", contex.trng->postproc.postprocValue);
    printf("sfr_opt = 0x%08" PRIx32 "\r\n", contex.trng->opt);
    printf("sfr_chain0 = 0x%08" PRIx32 "\r\n", contex.trng->chain0);
    printf("sfr_chain1 = 0x%08" PRIx32 "\r\n", contex.trng->chain1);

    remain = size;
    offset = 0;
    printf("rng start\r\n");
    while (1)
    {
        if (opt & 0x10000)
        {
            SCE_CLR_FIFO_5();
        }
        else
        {
            SCE_CLR_FIFO_4();
        }

        if (genloop != 0)
        {
            printf("\r\nGEN_LOOP = %" PRIu32 " ,HLTHTEST_ERRCNT (After gen)= %" PRIu32 "\r\n", genloop - 1, ((contex.trng->sr >> 16) & 0x00FF)); /* last gen loop */
        }

        printf("GEN_LOOP = %" PRIu32 " ,HLTHTEST_ERRCNT (Before gen)= %" PRIu32 "\r\n", genloop, ((contex.trng->sr >> 16) & 0x00FF)); /* current gen loop */

        if (sfr_ar == 0x5a)
        {
            trng_start(contex.trng);

            if (remain * 4 < 1024)
            {
                if (opt & 0x10000)
                {
                    while (SCE_FIFO_5_CNT() < remain)
                        ;
                    if (buf != NULL)
                    {
                        memcpy(buf + offset, (void *)ADDR_RNG_SEG_RNGB, remain * sizeof(uint32_t));
                    }

                    memcpy(tempBuf, (void *)ADDR_RNG_SEG_RNGB, remain * sizeof(uint32_t));

                    printf("\r\nGEN_LOOP = %" PRIu32 " ,HLTHTEST_ERRCNT (After gen)= %" PRIu32 "\r\n", genloop, ((contex.trng->sr >> 16) & 0x00FF));
                    trng_stop(contex.trng);
                    break;
                }
                else
                {
                    while (SCE_FIFO_4_CNT() < remain)
                        ;
                    if (buf != NULL)
                    {
                        memcpy(buf + offset, (void *)ADDR_RNG_SEG_RNGA, remain * sizeof(uint32_t));
                    }

                    memcpy(tempBuf, (void *)ADDR_RNG_SEG_RNGA, remain * sizeof(uint32_t));
                    printf("\r\nGEN_LOOP = %" PRIu32 " ,HLTHTEST_ERRCNT (After gen)= %" PRIu32 "\r\n", genloop, ((contex.trng->sr >> 16) & 0x00FF));
                    trng_stop(contex.trng);
                    break;
                }
            }
            else
            {
                if (opt & 0x10000)
                {
                    while (SCE_GET_FIFO_STATS(REG_SCE_CTRL_FIFOCNT5) != FIFO_FULL)
                        ;

                    if (buf != NULL)
                    {
                        memcpy(buf + offset, (void *)ADDR_RNG_SEG_RNGB, 1024);
                    }

                    memcpy(tempBuf, (void *)ADDR_RNG_SEG_RNGB, 1024);
                    dumpBuf(tempBuf, 1024);
                }
                else
                {
                    while (SCE_GET_FIFO_STATS(REG_SCE_CTRL_FIFOCNT4) != FIFO_FULL)
                        ;
                    if (buf != NULL)
                    {
                        memcpy(buf + offset, (void *)ADDR_RNG_SEG_RNGA, 1024);
                    }

                    memcpy(tempBuf, (void *)ADDR_RNG_SEG_RNGA, 1024);
                    dumpBuf(tempBuf, 1024);
                }

                offset += 1024;
                remain -= (1024 / 4);
            }
            if (remain == 0)
            {
                printf("\r\nGEN_LOOP = %" PRIu32 " ,HLTHTEST_ERRCNT (After gen)= %" PRIu32 "\r\n", genloop, ((contex.trng->sr >> 16) & 0x00FF));
                trng_stop(contex.trng);
                break;
            }
        }
        else /* get raw data */
        {
            trng_stop(contex.trng);
            printf("get raw data\r\n");
            while (trng_raw_buf_ready(contex.trng) == 0)
            {
                asm("nop");
            }

            for (uint32_t i = 0; i < remain; i++)
            {
                printf("%08" PRIX32, contex.trng->buf);
            }
            break;
        }

        genloop++;
    }
    if (opt & 0x10000)
    {
        trng_fifo_disable(SCE_FIFO_5);
    }
    else
    {
        trng_fifo_disable(SCE_FIFO_4);
    }
    printf("\r\nrng stop\r\n");
    return;
}