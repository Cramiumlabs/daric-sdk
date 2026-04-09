/**
 ******************************************************************************
 * @file    alu.c
 * @author  SCE Team
 * @brief   ALU driver module for large number arithmetic.
 *          This file provides firmware functions to manage the ALU
 *          peripheral.
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
#include <string.h>
#include "daric.h"
#include "auth.h"
#include "hash.h"
#include "sdma.h"
#include "alu.h"
#include "bn_util.h"
#include "pke_curve.h"
#include <assert.h>
#include <errno.h>

/**
 * @brief swap elements in u32 array
 *
 * @param indata input array
 * @param length array length
 */
void swap_elements(uint32_t indata[], int length)
{
    int      i;
    uint32_t temp;

    for (i = 0; i < length; i += 2)
    {
        temp          = indata[i];
        indata[i]     = indata[i + 1];
        indata[i + 1] = temp;
    }
}

/**
 * @brief x / y = q ... r, xlen > ylen. User should ensure the `q` and `r` have sufficient space to store the result.
 *
 * @param x dividend
 * @param y divisor
 * @param xlen word length of x. The last 64-bit word must not be zero.
 * @param ylen word length of y. The last 64-bit word must not be zero.
 * @param[out] q quotient
 * @param[out] qlen word length of q.
 * @param[out] r remainder
 * @param[out] rlen word length of r
 * @return uint32_t error code, 0 if success
 */
uint32_t alu_division(const uint32_t *x, const uint32_t *y, uint32_t xlen, uint32_t ylen, uint32_t *q, uint32_t *qlen, uint32_t *r, uint32_t *rlen)
{
    uint32_t qws, rws;
    uint32_t xlen_p, ylen_p;

    assert(xlen <= 256 && ylen <= 256); /* SEG_PIB & SEG_PSIB is 1024 bytes */
    ASSERT_4BYTE_ALIGNED(x);
    ASSERT_4BYTE_ALIGNED(y);
    ASSERT_4BYTE_ALIGNED(q);
    ASSERT_4BYTE_ALIGNED(r);
    assert(x[xlen - 1] != 0 || x[xlen - 2] != 0); /* The last 64-bit word must not be zero. */
    assert(y[ylen - 1] != 0 || y[ylen - 2] != 0); /* The last 64-bit word must not be zero. */
    assert(bnu_get_msb_le(x, xlen) < 8191);
    assert(bnu_get_msb_le(x, xlen) >= 64);
    assert(bnu_get_msb_le(y, ylen) < 8191);

    if ((q == NULL) && (r == NULL))
    {
        return EINVAL; /* no need to do division */
    }

    if (xlen & 1)
    {
        xlen_p = xlen + 1;
    }
    else
    {
        xlen_p = xlen;
    }

    if (ylen & 1)
    {
        ylen_p = ylen + 1;
    }
    else
    {
        ylen_p = ylen;
    }

    /* When x < y, the quotient is 0 and the remainder is the dividend. Padding is required to ylen. */
    if (xlen < ylen)
    {
        xlen = ylen;
    }

    SCE_ALU_CLK_EN();
    SCE_ALU_DEDS_DWLEN(xlen_p / 2, ylen_p / 2);

#if ALU_ENDIAN_MODE == ALU_INBIG_OUTBIG || ALU_ENDIAN_MODE == ALU_INBIG_OUTLITTLE
    memset((uint8_t *)ADDR_PKE_SEG_PIB, 0, xlen * 4);
    memset((uint8_t *)ADDR_PKE_SEG_PSIB, 0, ylen * 4);

    memcpy((uint8_t *)ADDR_PKE_SEG_PIB + (xlen - xlen) * 4, (uint8_t *)x, xlen * 4);
    memcpy((uint8_t *)ADDR_PKE_SEG_PSIB + (ylen - ylen) * 4, (uint8_t *)y, ylen * 4);
#else
    /* FIXME: below lines can be optimized. (note: 4 bytes aligned) */
    bnu_memcpy_u32((uint32_t *)ADDR_PKE_SEG_PIB, x, xlen);
    if (xlen & 1)
    {
        *(ADDR_PKE_SEG_PIB + xlen) = 0; /* padding */
    }
    bnu_memcpy_u32((uint32_t *)ADDR_PKE_SEG_PSIB, y, ylen);
    if (ylen & 1)
    {
        *(ADDR_PKE_SEG_PSIB + ylen) = 0; /* padding */
    }

    /* fix rtl bug for little endian , world swap in U64 */
    swap_elements((uint32_t *)ADDR_PKE_SEG_PIB, xlen_p);
    swap_elements((uint32_t *)ADDR_PKE_SEG_PSIB, ylen_p);
#endif

    REG_SCE_ALU_OPTLTX = ALU_ENDIAN_MODE;

    /* setting pointers */
    SCE_ALU_SEGPTR_DE(9, 0);   /* segid = 9, offset = 0; dividend(x) is in Seg_PIB */
    SCE_ALU_SEGPTR_DS(9, 256); /* segid = 9, offset = 0; dividend(x) is in Seg_PIB */
    SCE_ALU_SEGPTR_QT(11, 0);  /* segid = 11, offset = 0; quotient(q) is in Seg_POB */
    SCE_ALU_SEGPTR_RM(12, 0);  /* segid = 12, offset = 0; remainder(r) is in Seg_PSOB */

    /* start */
    SCE_ALU_FUNC(ALU_AF_DIV);
    SCE_ALU_AR();
    SCE_ALU_DONE();

    /* get the result */
    qws = SCE_ALU_QT_DWLEN() * 2; /* word length of quotient */
    rws = SCE_ALU_RM_DWLEN() * 2; /* word length of remainder */
    /* if length of diviend is bigger than 127 Words, qws in register should be 0, SW need read 128 Words (4096b) as result */
    if ((qws == 0) && (xlen_p > 126))
    {
        qws = 128;
    }

    if (q != NULL)
    {
        bnu_memcpy_u32(q, (uint32_t *)ADDR_PKE_SEG_POB, qws);
    }

    if (qlen != NULL)
    {
        *qlen = qws;
    }

    if (r != NULL)
    {
        bnu_memcpy_u32(r, (uint32_t *)ADDR_PKE_SEG_PSOB, rws);
    }

    if (rlen != NULL)
    {
        *rlen = rws;
    }

    return 0;
}

/**
 * @brief diff = x - y; note: xlen >= ylen
 *
 * @param x minuend
 * @param y subtrahend
 * @param xlen world length
 * @param ylen world length
 * @param diff difference
 * @param carry Reduction indicator. 1 if borrow occurs, 0 otherwise.
 * @return uint32_t error code, 0 if success
 */
uint32_t alu_sub(const uint32_t *x, const uint32_t *y, uint32_t xlen, uint32_t ylen, uint32_t *diff, uint32_t *carry)
{
    assert(xlen <= 256 && ylen <= 256); /* SEG_PIB & SEG_PSIB is 1024 bytes */
    ASSERT_4BYTE_ALIGNED(x);
    ASSERT_4BYTE_ALIGNED(y);
    ASSERT_4BYTE_ALIGNED(diff);
    assert(xlen % 2 == 0);
    assert(ylen % 2 == 0);
    assert(xlen >= ylen); /* xlen >= ylen otherwise, the result is wrong */
    assert(bnu_get_msb_le(x, xlen) < 8191);
    assert(bnu_get_msb_le(y, ylen) < 8191);

    SCE_ALU_CLK_EN();

    /* U64 length */
    SCE_ALU_DEDS_DWLEN(xlen / 2, ylen / 2);

#if ALU_ENDIAN_MODE == ALU_INBIG_OUTBIG || ALU_ENDIAN_MODE == ALU_INBIG_OUTLITTLE
    memset((uint8_t *)ADDR_PKE_SEG_PIB, 0, xlen * 4);
    memset((uint8_t *)ADDR_PKE_SEG_PSIB, 0, xlen * 4);

    memcpy((uint8_t *)ADDR_PKE_SEG_PIB + (xlen - xlen) * 4, (uint8_t *)x, xlen * 4);
    memcpy((uint8_t *)ADDR_PKE_SEG_PSIB + (xlen - ylen) * 4, (uint8_t *)y, ylen * 4); /* x  y LSB aligned */
#else
    /* FIXME: below lines can be optimized. */
    bnu_memcpy_u32((uint32_t *)ADDR_PKE_SEG_PIB, x, xlen);
    bnu_memcpy_u32((uint32_t *)ADDR_PKE_SEG_PSIB, y, ylen);

    /* fix rtl bug for little endian , world swap in U64 */
    swap_elements((uint32_t *)ADDR_PKE_SEG_PIB, xlen);
    swap_elements((uint32_t *)ADDR_PKE_SEG_PSIB, ylen);
#endif

    REG_SCE_ALU_OPTLTX = ALU_ENDIAN_MODE;

    /* setting pointers */
    SCE_ALU_SEGPTR_DE(9, 0); /* segid = 9, offset = 0; dividend(x) is in Seg_PIB */
    SCE_ALU_SEGPTR_DS(9, 256);
    SCE_ALU_SEGPTR_RM(11, 0);
    SCE_ALU_SEGPTR_QT(12, 0); /* for sub result */

    /* start */
    SCE_ALU_FUNC(ALU_AF_SUB);
    SCE_ALU_AR();
    SCE_ALU_DONE();

    /* borrow indicator */
    if (carry != NULL)
    {
        if ((REG_SCE_ALU_SRMFSM & 0x100) == 0x100)
        {
            *carry = 1;
        }
        else
        {
            *carry = 0;
        }
    }

    bnu_memcpy_u32(diff, (uint32_t *)ADDR_PKE_SEG_PSOB, xlen);

    return 0;
}

/**
 * @brief addition = x + y
 *
 * @param x addend
 * @param y addend
 * @param xlen TBD. word length. should be even number, because alu is 64bit aligned.
 * @param ylen TBD. word length. should be even number, because alu is 64bit aligned.
 * @param[out] addition
 * @param[out] carry
 * @return uint32_t
 */
uint32_t alu_add(const uint32_t *x, const uint32_t *y, uint32_t xlen, uint32_t ylen, uint32_t *addition, uint32_t *carry)
{
    ASSERT_4BYTE_ALIGNED(x);
    ASSERT_4BYTE_ALIGNED(y);
    ASSERT_4BYTE_ALIGNED(addition);
    assert(xlen <= 256 && ylen <= 256); /* SEG_PIB & SEG_PSIB is 1024 bytes */
    assert(xlen % 2 == 0);
    assert(ylen % 2 == 0);
    assert(bnu_get_msb_le(x, xlen) < 8191);
    assert(bnu_get_msb_le(y, ylen) < 8191);

    SCE_ALU_CLK_EN();

    /* U64 length */
    SCE_ALU_DEDS_DWLEN(xlen / 2, ylen / 2);

#if ALU_ENDIAN_MODE == ALU_INBIG_OUTBIG || ALU_ENDIAN_MODE == ALU_INBIG_OUTLITTLE
    memset((uint8_t *)ADDR_PKE_SEG_PIB, 0, xlen * 4);
    memset((uint8_t *)ADDR_PKE_SEG_PSIB, 0, ylen_align * 4);

    memcpy((uint8_t *)ADDR_PKE_SEG_PIB + (xlen - xlen) * 4, (uint8_t *)x, xlen * 4);
    memcpy((uint8_t *)ADDR_PKE_SEG_PSIB + (xlen - ylen) * 4, (uint8_t *)y, ylen * 4);
#else
    /* FIXME: below lines can be optimized. */
    bnu_memcpy_u32((uint32_t *)ADDR_PKE_SEG_PIB, x, xlen);
    bnu_memcpy_u32((uint32_t *)ADDR_PKE_SEG_PSIB, y, ylen);

    /* fix rtl bug for little endian , world swap in U64 */
    swap_elements((uint32_t *)ADDR_PKE_SEG_PIB, xlen);
    swap_elements((uint32_t *)ADDR_PKE_SEG_PSIB, ylen);
#endif

    REG_SCE_ALU_OPTLTX = ALU_ENDIAN_MODE;

    /* setting pointers */
    SCE_ALU_SEGPTR_DE(9, 0);
    SCE_ALU_SEGPTR_DS(9, 256);
    SCE_ALU_SEGPTR_RM(11, 0);
    SCE_ALU_SEGPTR_QT(12, 0); /* for add result */

    /* start */
    SCE_ALU_FUNC(ALU_AF_ADD);
    SCE_ALU_AR();
    SCE_ALU_DONE();

    /* carry indicator */
    if (carry != NULL)
    {
        if ((REG_SCE_ALU_SRMFSM & 0x100) == 0x100)
        {
            *carry = 1;
        }
        else
        {
            *carry = 0;
        }
    }

    bnu_memcpy_u32(addition, (uint32_t *)ADDR_PKE_SEG_PSOB, xlen > ylen ? xlen : ylen);

    return 0;
}

/**
 * @brief shift right by bit
 *
 * @param in input buffer
 * @param inLen input buffer length
 * @param bitNum shift right bit number
 * @param[out] out output buffer
 * @return uint32_t
 */
uint32_t alu_sft(const uint32_t *in, uint32_t inLen, uint32_t bitNum, uint32_t *out)
{
    uint32_t xlen_align;

    SCE_ALU_CLK_EN();
    xlen_align = inLen % 2 ? inLen + 1 : inLen;

    // U64 length
    SCE_ALU_DEDS_DWLEN(xlen_align / 2, 0);

    memset((uint8_t *)ADDR_PKE_SEG_PIB, 0, xlen_align * 4);
    memcpy((uint8_t *)ADDR_PKE_SEG_PIB + (xlen_align - inLen) * 4, (uint8_t *)in, inLen * 4);

    bnu_print_mem("ADDR_PKE_SEG_PIB ", (uint8_t *)ADDR_PKE_SEG_PIB, xlen_align * 4);

    REG_SCE_ALU_OPT_DIR(ALU_ENDIAN_MODE);
    REG_SCE_ALU_OPT_SFTCOUNT(bitNum);

    REG_SCE_ALU_OPTLTX = 0xFF;
    // setting pointers
    SCE_ALU_SEGPTR_DE(9, 0);
    SCE_ALU_SEGPTR_DS(9, 256);
    SCE_ALU_SEGPTR_RM(11, 0);
    SCE_ALU_SEGPTR_QT(12, 0);

    // start
    SCE_ALU_FUNC(ALU_AF_SFT);
    SCE_ALU_AR();
    SCE_ALU_DONE();

    bnu_print_mem("ADDR_PKE_SEG_PSOB", (uint8_t *)ADDR_PKE_SEG_PSOB, xlen_align * 4);
    memcpy((uint8_t *)out, (uint8_t *)ADDR_PKE_SEG_PSOB, xlen_align * 4);
    return xlen_align;
}

/**
 * @brief shift right by word
 *
 * @param in input buffer
 * @param inLen input buffer length
 * @param wordNum shift right word number
 * @param[out] out output buffer
 * @return uint32_t
 */
uint32_t alu_sftword(const uint32_t *in, uint32_t inLen, uint32_t wordNum, uint32_t *out)
{
    uint32_t xlen_align;
    SCE_ALU_CLK_EN();
    xlen_align = inLen % 2 ? inLen + 1 : inLen;

    /* U64 length */
    SCE_ALU_DEDS_DWLEN(xlen_align / 2, 0);

    memset((uint8_t *)ADDR_PKE_SEG_PIB, 0, xlen_align * 4);
    memcpy((uint8_t *)ADDR_PKE_SEG_PIB + (xlen_align - inLen) * 4, (uint8_t *)in, inLen * 4);

    bnu_print_mem("ADDR_PKE_SEG_PIB", (uint8_t *)ADDR_PKE_SEG_PIB, xlen_align * 4);

    REG_SCE_ALU_OPT_DIR(ALU_ENDIAN_MODE);
    REG_SCE_ALU_OPT_SFTCOUNT(wordNum);

    REG_SCE_ALU_OPTLTX = 0xFF;
    /* setting pointers */
    SCE_ALU_SEGPTR_DE(9, 0);
    SCE_ALU_SEGPTR_DS(9, 256);
    SCE_ALU_SEGPTR_RM(11, 0);
    SCE_ALU_SEGPTR_QT(12, 0);

    /* start */
    SCE_ALU_FUNC(ALU_AF_SFTW);
    SCE_ALU_AR();
    SCE_ALU_DONE();

    bnu_print_mem("ADDR_PKE_SEG_PSOB", (uint8_t *)ADDR_PKE_SEG_PSOB, xlen_align * 4);
    memcpy((uint8_t *)out, (uint8_t *)ADDR_PKE_SEG_PSOB, xlen_align * 4);
    return xlen_align;
}

uint32_t alu_blg(const uint32_t *x, const uint32_t *y, uint32_t xlen, uint32_t ylen, uint32_t *outData)
{
    uint32_t xlen_align, ylen_align;
    SCE_ALU_CLK_EN();
    xlen_align = xlen % 2 ? xlen + 1 : xlen;
    ylen_align = ylen % 2 ? ylen + 1 : ylen;

    /* U64 length */
    SCE_ALU_DEDS_DWLEN(xlen_align / 2, ylen_align / 2);

    memset((uint8_t *)ADDR_PKE_SEG_PIB, 0, xlen_align * 4);
    memset((uint8_t *)ADDR_PKE_SEG_PSIB, 0, ylen_align * 4);
    memcpy((uint8_t *)ADDR_PKE_SEG_PIB + (xlen_align - xlen) * 4, (uint8_t *)x, xlen * 4);
    memcpy((uint8_t *)ADDR_PKE_SEG_PSIB + (ylen_align - ylen) * 4, (uint8_t *)y, ylen * 4);

    bnu_print_mem("ADDR_PKE_SEG_PIB", (uint8_t *)ADDR_PKE_SEG_PIB, xlen_align * 4);
    bnu_print_mem("ADDR_PKE_SEG_PSIB", (uint8_t *)ADDR_PKE_SEG_PSIB, ylen_align * 4);

    REG_SCE_ALU_OPT_BLG(ALU_ENDIAN_MODE);

    REG_SCE_ALU_OPTLTX = 0xFF;
    /* setting pointers */
    SCE_ALU_SEGPTR_DE(9, 0);
    SCE_ALU_SEGPTR_DS(9, 256);
    SCE_ALU_SEGPTR_RM(11, 0);
    SCE_ALU_SEGPTR_QT(12, 0); /* for add result */

    /* start */
    SCE_ALU_FUNC(ALU_AF_BLG);
    SCE_ALU_AR();
    SCE_ALU_DONE();

    bnu_print_mem("ADDR_PKE_SEG_PSOB", (uint8_t *)ADDR_PKE_SEG_PSOB, xlen_align >= ylen_align ? xlen_align * 4 : ylen_align * 4);

    memcpy((uint8_t *)outData, (uint8_t *)ADDR_PKE_SEG_PSOB, xlen_align >= ylen_align ? xlen_align * 4 : ylen_align * 4);
    return 8 / 4;
}

uint32_t alu_bex(const uint32_t *in, uint32_t inLen, uint32_t *out)
{
    uint32_t xlen_align;
    SCE_ALU_CLK_EN();
    xlen_align = inLen % 2 ? inLen + 1 : inLen;

    /* U64 length */
    SCE_ALU_DEDS_DWLEN(xlen_align / 2 + 1, xlen_align / 2 + 1);

    memset((uint8_t *)ADDR_PKE_SEG_PIB, 0, xlen_align * 4);
    memcpy((uint8_t *)ADDR_PKE_SEG_PIB + (xlen_align - inLen) * 4, (uint8_t *)in, inLen * 4);

    REG_SCE_ALU_OPT_BEX(ALU_ENDIAN_MODE);

    REG_SCE_ALU_OPTLTX = 0x00;
    /* setting pointers */
    SCE_ALU_SEGPTR_DE(9, 0);
    SCE_ALU_SEGPTR_DS(9, 256);
    SCE_ALU_SEGPTR_RM(11, 0);
    SCE_ALU_SEGPTR_QT(12, 0);

    /* start */
    SCE_ALU_FUNC(ALU_AF_BEX);
    SCE_ALU_AR();
    SCE_ALU_DONE();

    memcpy((uint8_t *)out, (uint8_t *)ADDR_PKE_SEG_POB + 8, xlen_align * 4);
    return xlen_align;
}

// alu_bex can only reverse 1008 bytes at a time
uint32_t alu_bex_ex(uint32_t *in, uint32_t inLen, uint32_t *out)
{
    uint32_t tempBuf[8192 / 32], i, loop, remain;

    memset(tempBuf, 0, sizeof(tempBuf));

    loop   = inLen / 0xFC;
    remain = inLen % 0xFC;

    for (i = 0; i < loop; i++)
    {
        alu_bex(in + inLen - 0xFC * (i + 1), 0xFC, tempBuf + 0xFC * i);
    }

    // remainder part
    if (remain)
    {
        alu_bex(in, remain, tempBuf + 0xFC * loop);
    }

    memcpy(out, tempBuf, inLen * 4);
    return inLen;
}
