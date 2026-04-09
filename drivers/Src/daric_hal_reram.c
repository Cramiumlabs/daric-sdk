/**
 ******************************************************************************
 * @file    daric_hal_reram.c
 * @author  ReRam Team
 * @brief   ReRam HAL module driver.
 *          This file provides firmware functions to support
 *          read and write functionality for the reram storage.
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "daric_util.h"
#include "daric_hal_reram.h"

/* Private macro -------------------------------------------------------------*/
#define TRUE  1
#define FALSE 0

#define USE_LOOP_WRITE

#define REG8(addr)       (*(volatile uint8_t *)(addr))
#define RERAM_END        (CONFIG_RERAM_START + CONFIG_RERAM_SIZE)
#define RERAM_PAGE_SIZE  0x20          // A block equals 32 bytes
#define RERAM_BLOCK_SIZE (0x20 * 0x20) // 32*32
#define RERAM_BLOCK_MAX  32            // 32*32

/**
 * @brief  Write single-page data to ReRam and flush to storage unit.
 * @param  dstAddr Write the starting address of ReRam.
 * @param  pWtBuf The source address of the buffer to be written,
 *         typically a buffer allocated in SRAM.
 * @retval Return 1 means success, return 0 value means failure.
 *
 * The function write single-page data to a specified address in ReRam.
 * Write to Buffer1->load to Buffer2->flush to storage unit
 */
static uint8_t Reram_Page_Write(uint32_t dstAddr, uint8_t *pWtBuf)
{
    uint8_t i, j;

    // This condition imposes a write limit in the bootloader to prevent damage to the bootloader.
    // if (wtLen != RERAM_PAGE_SIZE || (dstAddr < USR_RUNCOS_ADDR && g_writeFlag == FALSE) || dstAddr > RERAM_END)
    // {
    //     return FALSE;
    // }

    if ((dstAddr < CONFIG_RERAM_START) || (dstAddr >= RERAM_END))
    {
        return FALSE;
    }

    for (i = 0, j = 0; i < 4; i++)
    {
        *((volatile uint64_t *)(dstAddr + (i * 0x8))) = *(volatile uint64_t *)(pWtBuf + j);
        j += 8;
        __DSB();
    }

    // step 2  config  reram write command mode
    DARIC_RERAM->RRC_CR = 0x2; // config write command mode
    __DSB();

    // step 2-1  send load command
    *((volatile uint32_t *)dstAddr) = 0x5200; // 5200  load cammand
    __DSB();

    // step 2-2  send write command
    *((volatile uint32_t *)dstAddr) = 0x9528; // 9528  write cammond
    __DSB();

    DARIC_RERAM->RRC_CR = 0x0; // clear write command mode
    __DSB();

    return TRUE;
}

/**
 * @brief  Write single-page data to ReRam but do not flush to storage unit.
 * @param  dstAddr Write the starting address of ReRam.
 * @param  pWtBuf The source address of the buffer to be written,
 *         typically a buffer allocated in SRAM.
 * @retval Return 1 means success, return 0 value means failure.
 *
 * The function write single-page data to a specified address in ReRam.
 * Write to Buffer1->load to Buffer2
 * A maximum of RERAM_BLOCK_MAX(32)PAGE of data can be loaded at one time.
 */
static uint8_t Reram_Write_LoadBuffer(uint32_t dstAddr, uint8_t *pWtBuf)
{
    uint8_t i, j;

    // This condition imposes a write limit in the bootloader to prevent damage to the bootloader.
    // if (wtLen != RERAM_PAGE_SIZE || (dstAddr < USR_RUNCOS_ADDR && g_writeFlag == FALSE) || dstAddr > RERAM_END)
    // {
    //     return FALSE;
    // }

    if ((dstAddr < CONFIG_RERAM_START) || (dstAddr >= RERAM_END))
    {
        return FALSE;
    }

    for (i = 0, j = 0; i < 4; i++)
    {
        *((volatile uint64_t *)(dstAddr + (i * 0x8))) = *(volatile uint64_t *)(pWtBuf + j);
        j += 8;
        __DSB();
    }

    // step 2  config  reram write command mode
    DARIC_RERAM->RRC_CR = 0x2; // config write command mode
    __DSB();

    // step 2-1  send load command
    *((volatile uint32_t *)dstAddr) = 0x5200; // 5200  load cammand
    __DSB();

    DARIC_RERAM->RRC_CR = 0x0; // clear write command mode
    __DSB();

    return TRUE;
}

/**
 * @brief  Flush buffer of ReRam to storage unit.
 * @param  dstAddr Write the starting address of ReRam.
 * @retval Return 1 means success, return 0 value means failure.
 *
 * The function flush buffer2 of ReRam to storage unit.
 * Buffer2->storage unit
 * A maximum of RERAM_BLOCK_MAX(32)PAGE of data can be loaded at one time.
 */
static uint8_t Reram_Flush_Buffer(uint32_t dstAddr)
{
    if ((dstAddr < CONFIG_RERAM_START) || (dstAddr >= RERAM_END))
    {
        return FALSE;
    }

    // step 2  config  reram write command mode
    DARIC_RERAM->RRC_CR = 0x2; // config write command mode
    __DSB();

    // step 2-2  send write command
    *((volatile uint32_t *)dstAddr) = 0x9528; // 9528  write cammond
    __DSB();

    DARIC_RERAM->RRC_CR = 0x0; // clear write command mode
    __DSB();

    return TRUE;
}

/**
 * @brief  Read data from the srcAddr location in ReRam into the sram buffer pRdBuf.
 * @param  srcAddr Read the starting address of ReRam.
 * @param  pRdBuf The destination address of the buffer to be read,
 *         typically a buffer allocated in SRAM.
 * @param  rdLen The length of data to be read.
 * @retval Return 1 means success, return 0 value means failure.
 *
 * This function reads data of a specified length from a specified address in ReRam,
 * and the data will be stored at the location specified by pRdBuf.
 */
uint8_t HAL_RERAM_Read(uint32_t srcAddr, uint8_t *pRdBuf, uint32_t rdLen)
{
    uint32_t i;

    for (i = 0; i < rdLen; i++)
    {
        *(pRdBuf + i) = REG8(srcAddr + i);
    }
    return TRUE;
}

/**
 * @brief  Write the data from the pWtBuf buffer to ReRam.
 * @param  dstAddr Write the starting address of ReRam.
 * @param  pWtBuf The source address of the buffer to be written,
 *         typically a buffer allocated in SRAM.
 * @param  wtLen The length of data to be write.
 * @retval Return 1 means success, return 0 value means failure.
 *
 * The function write data to a specified address in ReRam.
 */
uint8_t HAL_RERAM_Write(uint32_t dstAddr, uint8_t *pWtBuf, uint32_t wtLen)
{
    volatile uint32_t writeAddr, loopAddr, tempAddr;
    uint8_t           oft;
    uint32_t          len;
    uint8_t           buf[RERAM_PAGE_SIZE], loop;
    uint8_t          *p = pWtBuf;

    // Calculate the length of the first write, considering whether it spans across pages.
    oft = dstAddr % RERAM_PAGE_SIZE;
    len = ((oft + wtLen) > RERAM_PAGE_SIZE) ? (RERAM_PAGE_SIZE - oft) : wtLen;

    // Convert the write start address to the page start address.
    writeAddr = (uint32_t)(-1);
    writeAddr = dstAddr & (writeAddr - (RERAM_PAGE_SIZE - 1));

    // Read the data content of the starting page into RAM.
    HAL_RERAM_Read(writeAddr, buf, RERAM_PAGE_SIZE);

    // Write the data that needs to be updated into RAM.
    memcpy(&buf[oft], p, len);
    p += len;

    // Update the data in the ReRam.
    if (Reram_Page_Write(writeAddr, buf) == FALSE)
    {
        return FALSE;
    }

    // If there is a page boundary crossing, continue the operation.
    wtLen = wtLen - len;
    // Avoid the issue of writing data errors on the same page caused by executing code in SRAM.
    if (wtLen == 0)
    {
        HAL_RERAM_Read((writeAddr + RERAM_PAGE_SIZE) >= RERAM_END ? (writeAddr - RERAM_PAGE_SIZE) : (writeAddr + RERAM_PAGE_SIZE), buf, RERAM_PAGE_SIZE);
        return TRUE;
    }
    writeAddr += RERAM_PAGE_SIZE;

#ifdef USE_LOOP_WRITE // Calculate the number of blocks already used in the current block.
    loop     = 0;
    loopAddr = 0;
    tempAddr = (uint32_t)(-1);
    tempAddr = writeAddr & (tempAddr - (RERAM_BLOCK_SIZE - 1)); // Block start address
    loop     = (writeAddr - tempAddr) / RERAM_PAGE_SIZE;        // The number of pages that the data block has been written to.
#endif

    for (; wtLen > RERAM_PAGE_SIZE;)
    {
        // The whole page data to be written.
        memcpy(buf, p, RERAM_PAGE_SIZE);

#ifdef USE_LOOP_WRITE
        if (loopAddr == 0)
        {
            loopAddr = writeAddr;
        }
        // Update the data in the ReRam.
        if (Reram_Write_LoadBuffer(writeAddr, buf) == FALSE)
        {
            return FALSE;
        }
        loop++;
        if (loop == RERAM_BLOCK_MAX)
        {
            if (Reram_Flush_Buffer(loopAddr) == FALSE)
            {
                return FALSE;
            }
            loop     = 0;
            loopAddr = 0;
        }
#else
        // Update the data in the ReRam.
        if (Reram_Page_Write(writeAddr, buf) == FALSE)
        {
            return FALSE;
        }
#endif
        p += RERAM_PAGE_SIZE;
        wtLen -= RERAM_PAGE_SIZE;
        writeAddr += RERAM_PAGE_SIZE;
    }

#ifdef USE_LOOP_WRITE
    if (loop != 0)
    {
        if (loopAddr != 0 && Reram_Flush_Buffer(loopAddr) == FALSE)
        {
            return FALSE;
        }
        loop = 0;
    }
#endif

    // If a tail page exists, continue the operation.
    if (wtLen > 0)
    {
        // Read the data content of the tail page into RAM.
        HAL_RERAM_Read(writeAddr, buf, RERAM_PAGE_SIZE);

        // Write the data that needs to be updated into RAM.
        memcpy(buf, p, wtLen);

        // Update the data in the ReRam.
        if (Reram_Page_Write(writeAddr, buf) == FALSE)
        {
            return FALSE;
        }
    }

    // Avoid the issue of writing data errors on the same page caused by executing code in SRAM.
    HAL_RERAM_Read((writeAddr + RERAM_PAGE_SIZE) >= RERAM_END ? (writeAddr - RERAM_PAGE_SIZE) : (writeAddr + RERAM_PAGE_SIZE), buf, RERAM_PAGE_SIZE);

    return TRUE;
}
