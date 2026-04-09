/**
 ******************************************************************************
 * @file    bn_util.c
 * @author  SCE Team
 * @brief   Big number utility functions implementation.
 *          This file provides a set of utility functions for large number
 *          arithmetic and formatting.
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

#include "bn_util.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "stdbool.h"

#if defined(HAL_API_DEBUG_EN)
#define bu_debug(format, ...)      Debug_Print(format, ##__VA_ARGS__)
#define hal_print_buf(format, ...) bnu_print_mem_duart_be(format, ##__VA_ARGS__)
#else
#define bu_debug(format, ...)
#define hal_print_buf(format, ...)
#endif

#define Debug_Print(format, ...) printf(format, ##__VA_ARGS__)

/**
 * @brief print memory bytes
 *
 * @param name title
 * @param ptr pointer to memory
 * @param len byte length
 */
void bnu_print_mem(const char *name, const void *ptr, uint32_t len)
{
    uint32_t i;
    uint8_t *p = (uint8_t *)ptr;

    printf("[%4" PRIu32 "]%s = ", len, name);
    for (i = 0; i < len; i++)
    {
        printf("%02X", p[i]);
    }
    printf("\r\n");
}

/**
 * @brief print as per u32
 *
 * @param name title
 * @param ptr pointer to memory
 * @param len word length
 */
void bnu_print_mem_u32(const char *name, const uint32_t *ptr, uint32_t len)
{
    uint32_t i;
    printf("[%4" PRIu32 "]%s = ", len, name);
    for (i = 0; i < len; i++)
    {
        printf("0x%08" PRIX32 ",", ptr[i]);
    }
    printf("\n");
}

/**
 * @brief print memory to DUART, form last byte to first byte
 *
 * @param name title
 * @param ptr pointer to memory
 * @param len byte length
 */
void bnu_print_mem_duart_be(char *name, uint8_t *ptr, uint32_t len)
{
    int32_t i;

    Debug_Print("%s = 0x", name);
    for (i = len - 1; i >= 0; i--)
    {
        Debug_Print("%02X", ptr[i]);
    }
    Debug_Print("\r\n");
}

/**
 * @brief print memory to DUART, form first byte to last byte
 *
 * @param name title
 * @param ptr pointer to memory
 * @param len byte length
 */
void bnu_print_mem_duart(char *name, uint8_t *ptr, uint32_t len)
{
    uint32_t i;
    Debug_Print("%s = 0x", name);
    for (i = 0; i < len; i++)
    {
        Debug_Print("%02X", ptr[i]);
    }
    Debug_Print("\r\n");
}

/**
 * @brief print big number (little endian)
 *
 * @param bn
 * @param len
 */
void bnu_print_le(const uint32_t *bn, uint32_t len)
{
    uint32_t al = bnu_get_nzw_le(bn, len);
    if (al == 0)
    {
        bu_debug("0x0\r\n");
        return;
    }
    bu_debug("0x");
    for (uint32_t i = 0; (al - i) >= 1; i++)
    {
        bu_debug("%08X", bn[al - i - 1]);
    }
    bu_debug("\r\n");
}

/**
 * @brief out = endian swaped in
 *
 * @param in input buffer
 * @param[out] out output buffer
 * @param len buffer length.
 */
void bnu_swap_endian_2(const void *in, void *out, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++)
    {
        ((uint8_t *)out)[i] = ((uint8_t *)in)[len - i - 1];
    }
}

/**
 * @brief swap endian of the buffer
 *
 * @param[out] buffer
 * @param len buffer length
 */
void bnu_swap_endian_1(void *buffer, uint32_t len)
{
    uint32_t i;
    uint8_t  tmp;

    for (i = 0; i < len / 2; i++)
    {
        tmp                              = ((uint8_t *)buffer)[i];
        ((uint8_t *)buffer)[i]           = ((uint8_t *)buffer)[len - i - 1];
        ((uint8_t *)buffer)[len - i - 1] = tmp;
    }
}

/**
 * @brief swap endian of buffer, and remove leading zeros
 *
 * @param in input buffer
 * @param out output buffer
 * @param[out] wordlen input buffer length. words length.
 * @return uint32_t
 */
uint32_t bnu_swap_endian(uint32_t *in, uint32_t *out, uint32_t wordlen)
{
    uint8_t *bin  = (uint8_t *)in;
    uint8_t *bout = (uint8_t *)out;
    uint32_t i, tlen;
    for (i = 0; i < wordlen * 4; i++)
    {
        *(bout + i) = *(bin + wordlen * 4 - 1 - i);
    }
    for (tlen = 0; tlen < wordlen; tlen++)
    {
        if (out[tlen] != 0)
        {
            break;
        }
    }
    for (i = tlen; i < wordlen; i++)
    {
        out[i - tlen] = out[i];
    }
    return (wordlen - tlen);
}

/**
 * @brief HEX string to data buffer in little endian; support to 4096 bit; buflen is
 * bytes number; output is bytes number
 *
 * @param instr
 * @param outbuf
 * @param buflen
 * @return uint32_t
 */
uint32_t bnu_hex2buf_le(char *instr, uint8_t *outbuf, uint32_t buflen)
{
    uint8_t tmpbuf[4096 / 8];
    size_t  len = (strlen(instr) + 1) / 2;
    char   *tmpinstr;
    size_t  i = 0;
    if (len > 4096 / 8)
        len = 4096 / 8;
    if (len > buflen)
        len = buflen;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    if (strlen(instr) % 2)
    {
        /* instr is odd */
        tmpbuf[0] = 0;
        if (instr[0] >= '0' && instr[0] <= '9')
            tmpbuf[0] += (instr[0] - '0');
        if ((instr[0] & ~0x20) >= 'A' && (instr[0] & ~0x20) <= 'F')
            tmpbuf[0] += (10 + (instr[0] & ~0x20) - 'A');
        tmpinstr = instr - 1;
        i        = 1;
    }
    else
    {
        /* instr is even */
        tmpinstr = instr;
    }
    for (; i < len; i++)
    {
        uint8_t c = 0;
        if (tmpinstr[i * 2] >= '0' && tmpinstr[i * 2] <= '9')
            c += (tmpinstr[i * 2] - '0') << 4;
        if ((tmpinstr[i * 2] & ~0x20) >= 'A' && (tmpinstr[i * 2] & ~0x20) <= 'F')
            c += (10 + (tmpinstr[i * 2] & ~0x20) - 'A') << 4;
        if (tmpinstr[i * 2 + 1] >= '0' && tmpinstr[i * 2 + 1] <= '9')
            c += (tmpinstr[i * 2 + 1] - '0');
        if ((tmpinstr[i * 2 + 1] & ~0x20) >= 'A' && (tmpinstr[i * 2 + 1] & ~0x20) <= 'F')
            c += (10 + (tmpinstr[i * 2 + 1] & ~0x20) - 'A');
        tmpbuf[i] = c;
    }
    bnu_swap_endian_2(tmpbuf, outbuf, len);
    return len;
}

/**
 * @brief HEX string to data buffer in big endian; support to 4096 bit; buflen is bytes
 * number
 *
 * @param instr
 * @param outbuf
 * @param buflen
 * @return uint32_t
 */
uint32_t bnu_hex2buf_be(char *instr, uint8_t *outbuf, uint32_t buflen)
{
    uint8_t tmpbuf[4096 / 8];
    size_t  len = (strlen(instr) + 1) / 2;
    char   *tmpinstr;
    size_t  i = 0;
    if (len > 4096 / 8)
        len = 4096 / 8;
    if (len > buflen)
        len = buflen;
    memset(tmpbuf, 0, sizeof(tmpbuf));
    if (strlen(instr) % 2)
    {
        /* instr is odd */
        tmpbuf[0] = 0;
        if (instr[0] >= '0' && instr[0] <= '9')
            tmpbuf[0] += (instr[0] - '0');
        if ((instr[0] & ~0x20) >= 'A' && (instr[0] & ~0x20) <= 'F')
            tmpbuf[0] += (10 + (instr[0] & ~0x20) - 'A');
        tmpinstr = instr - 1;
        i        = 1;
    }
    else
    {
        /* instr is even */
        tmpinstr = instr;
    }
    for (; i < len; i++)
    {
        uint8_t c = 0;
        if (tmpinstr[i * 2] >= '0' && tmpinstr[i * 2] <= '9')
            c += (tmpinstr[i * 2] - '0') << 4;
        if ((tmpinstr[i * 2] & ~0x20) >= 'A' && (tmpinstr[i * 2] & ~0x20) <= 'F')
            c += (10 + (tmpinstr[i * 2] & ~0x20) - 'A') << 4;
        if (tmpinstr[i * 2 + 1] >= '0' && tmpinstr[i * 2 + 1] <= '9')
            c += (tmpinstr[i * 2 + 1] - '0');
        if ((tmpinstr[i * 2 + 1] & ~0x20) >= 'A' && (tmpinstr[i * 2 + 1] & ~0x20) <= 'F')
            c += (10 + (tmpinstr[i * 2 + 1] & ~0x20) - 'A');
        tmpbuf[i] = c;
    }
    memcpy(outbuf, tmpbuf, len);
    return len;
}

/**
 * @brief if A == B return 1; else return 0
 *
 * @param A
 * @param B
 * @param wordlen length
 * @return uint32_t
 */
uint32_t bnu_is_same(uint32_t *A, uint32_t *B, uint32_t wordlen)
{
    uint32_t i;
    uint32_t ret = true;
    for (i = 0; i < wordlen; i++)
    {
        if (A[i] != B[i])
        {
            ret = false;
            break;
        }
    }
    return ret;
}

/**
 * @brief get the no-zero word length of big number (little endian), nlen is words
 * number
 *
 * @param N
 * @param nlen
 * @return uint32_t
 */
uint32_t bnu_get_nzw_le(const uint32_t *N, uint32_t nlen)
{
    uint32_t i;
    for (i = nlen; i > 0; i--)
    {
        if (N[i - 1] != 0)
        {
            break;
        }
    }
    return i;
}

/**
 * @brief get the no-zero word length of big number (big endian), nlen is words number
 *
 * @param N
 * @param nlen
 * @return uint32_t
 */
uint32_t bnu_get_nzw_be(const uint32_t *N, uint32_t nlen)
{
    uint32_t i;
    for (i = 0; i < nlen; i++)
    {
        if (N[i] != 0)
        {
            break;
        }
    }
    return i;
}

/**
 * @brief get the msb of big number (little endian), nlen is words number; N must be
 * odd number
 *
 * @param N
 * @param nlen
 * @return uint32_t
 */
uint32_t bnu_get_msb_le(const uint32_t *N, uint32_t nlen)
{
    uint32_t msb = 0;
    int32_t  i;
    /* Find the highest non-zero word */
    for (i = nlen - 1; i >= 0; i--)
    {
        if (N[i] != 0)
        {
            uint32_t leading_zeros = __builtin_clz(N[i]);
            uint32_t word_msb      = 31 - leading_zeros;
            msb                    = i * 32 + word_msb;
            break;
        }
    }
    return msb;
}

/**
 * @brief get the msb of big number (big endian), nlen is words number; N must be odd
 * number
 *
 * @param N
 * @param nlen
 * @return uint32_t
 */
uint32_t bnu_get_msb_be(const uint32_t *N, uint32_t nlen)
{
    uint32_t msb;
    uint32_t i, msB = 0, lsB = 0;
    uint8_t *Nb = (uint8_t *)N;
    for (i = 1; i <= nlen * 4; i++)
    {
        if (Nb[i - 1] != 0)
        {
            lsB = i;
            msB = (msB == 0) ? i : msB;
        }
    }
    if (msB == 0)
    {
        return 0; // N is 0
    }
    for (i = 0; i < 8; i++)
    {
        if ((Nb[msB - 1] & (1 << (7 - i))) != 0)
        {
            break;
        }
    }
    msb = (lsB - msB) * 8 + 7 - i;
    return msb;
}

/**
 * @brief outlen > inlen
 *
 * @param in
 * @param inlen
 * @param out
 * @param outlen
 */
void bnu_fill_le(const uint32_t *in, uint32_t inlen, uint32_t *out, uint32_t outlen)
{
    uint32_t i;

    if (outlen < inlen)
    {
        return;
    }
    for (i = 0; i < inlen; i++)
    {
        out[i] = in[i];
    }
    if (outlen == inlen)
    {
        return;
    }
    for (i = inlen; i < outlen; i++)
    {
        out[i] = 0;
    }
}

/**
 * @brief outlen > inlen
 *
 * @param in
 * @param inlen
 * @param out
 * @param outlen
 */
void bnu_fill_be(const uint32_t *in, uint32_t inlen, uint32_t *out, uint32_t outlen)
{
    uint32_t i;
    if (outlen < inlen)
    {
        return;
    }
    for (i = 0; i < inlen; i++)
    {
        out[outlen - 1 - i] = in[inlen - 1 - i];
    }
    if (outlen == inlen)
    {
        return;
    }
    for (i = inlen; i < outlen; i++)
    {
        out[outlen - 1 - i] = 0;
    }
}

/**
 * @brief little endian -> big endian, and shift the results. Return length.
 *
 * @param in input little endian number
 * @param out shifted big endian number
 * @param wordlen input number length (words length)
 * @return uint32_t big number length
 */
uint32_t bnu_le2be(const uint32_t *in, uint32_t *out, uint32_t wordlen)
{
    const uint8_t *bin  = (uint8_t *)in;
    uint8_t       *bout = (uint8_t *)out;
    uint32_t       i, tlen;
    for (i = 0; i < wordlen * 4; i++)
    {
        *(bout + i) = *(bin + wordlen * 4 - 1 - i);
    }
    for (tlen = 0; tlen < wordlen; tlen++)
    {
        if (out[tlen] != 0)
        {
            break;
        }
    }
    for (i = tlen; i < wordlen; i++)
    {
        out[i - tlen] = out[i];
    }
    return (wordlen - tlen);
}

/**
 * @brief shift big endian number
 *
 * @param buf pointer of big endian number
 * @param wordlen length (words length)
 * @return uint32_t length after shift
 */
uint32_t bnu_shift_be_num(uint32_t *buf, uint32_t wordlen)
{
    uint32_t tlen;
    uint32_t i;

    for (tlen = 0; tlen < wordlen; tlen++)
    {
        if (buf[tlen] != 0)
        {
            break;
        }
    }
    for (i = tlen; i < wordlen; i++)
    {
        buf[i - tlen] = buf[i];
    }
    return (wordlen - tlen);
}

/**
 * @brief big number is zero return 0; else return 1
 *
 * @param bn big number
 * @param size words length
 * @return true zero
 * @return false not zero
 */
bool bnu_is_zero(const uint32_t *bn, uint32_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        if (bn[i] != 0)
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief out = in
 *
 * @param in
 * @param len
 * @param out
 */
void bnu_copy(const uint32_t *in, int32_t len, uint32_t *out)
{
    while (len-- > 0)
    {
        out[len] = in[len];
    }
}

/**
 * @brief Copies memory in 4-byte blocks with unaligned source support.
 *
 * This function copies a specified number of 4-byte blocks from the source address
 * to the destination address. The destination address must be 4-byte aligned,
 * while the source address can be unaligned.
 *
 * @param dest The destination address, which must be 4-byte aligned.
 * @param src The source address, which can be unaligned.
 * @param size The number of bytes to copy.
 * @return int Returns 0 on success; returns -1 if the destination
 *             address is not 4-byte aligned.
 *
 * @note This function uses 4-byte blocks for copying. The caller must ensure that
 *       the destination and source memory regions do not overlap.
 * @warning If the destination address is not 4-byte aligned, this function will return
 *          an error, and no copying will be performed.
 */
int bnu_memcpy_unaligned_src(void *dest, const void *src, size_t size)
{
    if ((uintptr_t)dest % 4 != 0)
    {
        printf("Error: Destination address is not 4-byte aligned\n");
        return -1;
    }

    if ((uintptr_t)src % 4 != 0)
    {
        printf("warning! src is not 4-byte aligned\n");
        // return -1;
    }

    uint32_t      *d  = (uint32_t *)dest;
    const uint8_t *s  = (const uint8_t *)src;
    size_t         sz = size / 4;

    for (size_t i = 0; i < sz; i++)
    {
        uint32_t temp = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);
        d[i]          = temp;
        s += 4;
    }

    return 0;
}

/**
 * @brief copy memory with 4-byte blocks
 *
 * @param dst destination
 * @param src source
 * @param len words number
 */
void bnu_memcpy_u32(uint32_t *dst, const uint32_t *src, uint32_t len)
{
    while (len--)
    {
        *dst++ = *src++;
    }
}

/**
 * @brief unsigned big number (little endian) compare
 * return 0, if A = B; return 1, if A>B; return -1 if A<B; Alen, Blen are words
 * number
 * @param A
 * @param Alen
 * @param B
 * @param Blen
 * @return int32_t
 */
int32_t bnu_cmp_le(const uint32_t *A, uint32_t Alen, const uint32_t *B, uint32_t Blen)
{
    uint32_t al = bnu_get_nzw_le(A, Alen);
    uint32_t bl = bnu_get_nzw_le(B, Blen);

    if (al > bl)
    {
        return 1;
    }
    else if (al < bl)
    {
        return -1;
    }
    /* al == bl */
    for (uint32_t i = 1; i <= al; i++)
    {
        if (A[al - i] > B[bl - i])
        {
            return 1;
        }
        else if (A[al - i] < B[bl - i])
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief unsigned big number (big endian) compare
 * return 0, if A = B; return 1, if A>B; return -1 if A<B;
 *
 * @param A
 * @param Alen
 * @param B
 * @param Blen
 * @return int32_t
 */
int32_t bnu_cmp_be(const uint32_t *A, uint32_t Alen, const uint32_t *B, uint32_t Blen)
{
    uint32_t al  = bnu_get_nzw_be(A, Alen);
    uint32_t bl  = bnu_get_nzw_be(B, Blen);
    uint32_t Ale = 0, Ble = 0;
    uint32_t Ai, Bi;

    if (Alen - al > Blen - bl)
    {
        return 1;
    }
    else if (Alen - al < Blen - bl)
    {
        return -1;
    }
    /* Alen - al == Blen - bl */
    for (Ai = al, Bi = bl; Bi < Blen; Ai++, Bi++)
    {
        bnu_le2be(A + Ai, &Ale, 1);
        bnu_le2be(B + Bi, &Ble, 1);
        if (Ale > Ble)
        {
            return 1;
        }
        else if (Ale < Ble)
        {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief big number (little endian) input left shift stbits; xlen is words length,
 * must >= 2; output can NOT be same with input
 *
 * @param input
 * @param xlen
 * @param stbits
 * @param output
 */
void bnu_leftshift_le(const uint32_t *input, uint32_t xlen, uint32_t stbits, uint32_t *output)
{
    uint64_t tmp64 = 0;
    uint32_t i;

    if (stbits == 0)
    {
        memcpy(output, input, xlen * 4);
        return;
    }
    uint32_t stw = stbits / 32;
    uint32_t stb = stbits % 32;
    memset((void *)output, 0, xlen * 4);
    for (i = stw; i < xlen - 1; i++)
    {
        tmp64 = (uint64_t)(*(input + i - stw + 1));
        tmp64 = *(input + i - stw) + (tmp64 << 32);
        tmp64 <<= stb;
        *(output + i) |= (uint32_t)(tmp64 & 0xFFFFFFFF);
        *(output + i + 1) |= (uint32_t)(tmp64 >> 32);
    }
}

/**
 * @brief big number (little endian) input right shift stbits; xlen is words length,
 * must >= 2; output can NOT be same with input
 *
 * @param input
 * @param xlen
 * @param stbits
 * @param output
 */
void bnu_rightshift_le(const uint32_t *input, uint32_t xlen, uint32_t stbits, uint32_t *output)
{
    uint64_t tmp64 = 0;
    uint32_t i;

    if (xlen < 2)
    {
        return;
    }
    if (stbits == 0)
    {
        memcpy(output, input, xlen * 4);
        return;
    }
    uint32_t stw = stbits / 32;
    uint32_t stb = stbits % 32;
    memset(output, 0, xlen * 4);
    for (i = 1; i < (xlen - stw); i++)
    {
        tmp64 = (uint64_t)(*(input + xlen - i));
        tmp64 = (uint64_t)(*(input + xlen - i - 1)) + (tmp64 << 32);
        tmp64 >>= stb;
        *(output + xlen - i - stw) |= (uint32_t)(tmp64 >> 32);
        *(output + xlen - i - stw - 1) |= (uint32_t)(tmp64 & 0xFFFFFFFF);
    }
}

/**
 * @brief set a bit of big number (little endian)
 *
 * @param bn
 * @param xlen
 * @param stbits
 * @return int
 */
int bnu_setbit_le(uint32_t *bn, uint32_t xlen, uint32_t stbits)
{
    uint32_t stw = stbits / 32;
    uint32_t stb = stbits % 32;
    if (stw >= xlen)
    {
        return -1;
    }
    bn[stw] |= (1 << stb);
    return 0;
}

/**
 * @brief if input is multiple of 3, return 1; or return 0; len < 4096/32
 *
 * @param input
 * @param len
 * @return int
 */
int isMultipleOf3(const uint32_t *input, uint32_t len)
{
    int32_t  odd_num = 0, even_num = 0;
    uint32_t lmtbitlen         = bnu_get_msb_le(input, len) + 1;
    uint32_t bntmp2[4096 / 32] = { 0 };
    bnu_copy(input, len, bntmp2);

    for (uint32_t i = 2; i < lmtbitlen; i += 2)
    {
        if (bntmp2[0] & 1)
        {
            odd_num++;
        }
        if (bntmp2[0] & 2)
        {
            even_num++;
        }
        bnu_rightshift_le(input, len, i, bntmp2);
    }

    if ((odd_num - even_num) % 3 == 0)
    {
        return true;
    }
    return false;
}
