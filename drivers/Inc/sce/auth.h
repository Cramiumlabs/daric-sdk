/**
 ******************************************************************************
 * @file    auth.h
 * @author  SCE Team
 * @brief   SCE register definitions and memory map header file.
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

#ifndef __AUTH_H__
#define __AUTH_H__
// #include "types.h"
#include "daric_util.h"
#include "daric.h"
#define NEW_VERSION // New version algorithm ADDR_PKE_SEG_PKB 512 BYTE increased to 1024 BYTE

// #define ALG_TEST_MODE  // Algorithm parallel test mode, remember to turn it off in the official version

// SCE_INTERNEAL_RAM 0x1000--0x3000 mapped to SCE_RAM_ADDR 0x40024000 -- 0x40026000

#define SCE_BASE_ADDR 0x40020000

/*----------------------SCE_RAM------------------------*/
#define SCE_RAM_ADDR       SCE_BASE_ADDR
#define ADDR_HASH_SEG_LKEY ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0))    // HMAC LONG KEY            256 BYTE   FIFO-0
#define ADDR_HASH_SEG_KEY  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0100)) // HMAC KEY                 256 BYTE
#define ADDR_HASH_SEG_SKEY ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0200)) // HMAC SKEY                256 BYTE
#define ADDR_HASH_SEG_SCRT ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0300)) // HMAC SECRET              256 BYTE
#define ADDR_HASH_SEG_MSG  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0400)) // HMAC MSG                 256 BYTE   FIFO-1
#define ADDR_HASH_SEG_HOUT ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0600)) // HMAC OUT                 256 BYTE
#define ADDR_HASH_SEG_SOB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0700)) // HMAC SECURE OUT BUFFER   256 BYTE

// #define ADDR_PKE_SEG_PCON           ((volatile uint32_t*)(SCE_RAM_ADDR+0x0800))     //PKE CONST                256 BYTE
// #define ADDR_PKE_SEG_PKB            ((volatile uint32_t*)(SCE_RAM_ADDR+0x0900))     //PKE PKEY BUFFER          256 BYTE
#define ADDR_PKE_SEG_PCON ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0800)) // PKE CONST                1024 BYTE (reusing PKB and PIB space)
#define ADDR_PKE_SEG_PKB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0800)) // PKE PKEY BUFFER          512 BYTE increase to 1024 BYTE

#ifdef NEW_VERSION

#define ADDR_PKE_SEG_PIB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0C00)) // PKE INPUT BUFFER        1024 BYTE
#define ADDR_PKE_SEG_PSIB ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1000)) // PKE SECURE INPUT BUFFER 1024 BYTE  This address space has been reused for PIB, no segid 10
#define ADDR_PKE_SEG_POB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1400)) // PKE OUTPUT BUFFER       1024 BYTE
#define ADDR_PKE_SEG_PSOB ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1800)) // PKE SECURE OUT BUFFER   1024 BYTE

#define ADDR_AES_SEG_AKEY ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1C00)) // AES KEY                  256 BYTE
#define ADDR_AES_SEG_AIB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1D00)) // AES INPUT BUFFER         256 BYTE   FIFO-2
#define ADDR_AES_SEG_AOB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1E00)) // AES OUTPUT BUFFER        256 BYTE   FIFO-3

#define ADDR_RNG_SEG_RNGA ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1F00)) // RNG BUFFER A            1024 BYTE   FIFO-4
#define ADDR_RNG_SEG_RNGB ((volatile uint32_t *)(SCE_RAM_ADDR + 0x2300)) // RNG BUFFER B            1024 BYTE   FIFO-5

#else
#define ADDR_PKE_SEG_PIB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0A00)) // PKE INPUT BUFFER        1024 BYTE
#define ADDR_PKE_SEG_PSIB ((volatile uint32_t *)(SCE_RAM_ADDR + 0x0E00)) // PKE SECURE INPUT BUFFER 1024 BYTE
#define ADDR_PKE_SEG_POB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1200)) // PKE OUTPUT BUFFER       1024 BYTE
#define ADDR_PKE_SEG_PSOB ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1600)) // PKE SECURE OUT BUFFER   1024 BYTE

#define ADDR_AES_SEG_AKEY ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1A00)) // AES KEY                  256 BYTE
#define ADDR_AES_SEG_AIB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1B00)) // AES INPUT BUFFER         256 BYTE   FIFO-2
#define ADDR_AES_SEG_AOB  ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1C00)) // AES OUTPUT BUFFER        256 BYTE   FIFO-3

#define ADDR_RNG_SEG_RNGA ((volatile uint32_t *)(SCE_RAM_ADDR + 0x1D00)) // RNG BUFFER A            1024 BYTE   FIFO-4
#define ADDR_RNG_SEG_RNGB ((volatile uint32_t *)(SCE_RAM_ADDR + 0x2100)) // RNG BUFFER B            1024 BYTE   FIFO-5

#endif

/*----------------------SCE_CTRL------------------------*/
#define SCE_REG_CTRL          SCE_BASE_ADDR + 0x8000
#define REG_SCE_CTRL_SCEMODE  (*(volatile uint32_t *)(SCE_REG_CTRL + 0x0))
#define REG_SCE_CTRL_SUBCLKEN (*(volatile uint32_t *)(SCE_REG_CTRL + 0x04))
#define REG_SCE_CTRL_AHBSOPT  (*(volatile uint32_t *)(SCE_REG_CTRL + 0x08))
#define REG_SCE_CTRL_BUSYSR   (*(volatile uint32_t *)(SCE_REG_CTRL + 0x10))
#define REG_SCE_CTRL_DONEFR   (*(volatile uint32_t *)(SCE_REG_CTRL + 0x14))
#define REG_SCE_CTRL_ERRFR    (*(volatile uint32_t *)(SCE_REG_CTRL + 0x18))
#define REG_SCE_CTRL_RESETRAM (*(volatile uint32_t *)(SCE_REG_CTRL + 0x1C))
#define REG_SCE_CTRL_TICKCYC  (*(volatile uint32_t *)(SCE_REG_CTRL + 0x20))
#define REG_SCE_CTRL_TICKCNT  (*(volatile uint32_t *)(SCE_REG_CTRL + 0x24))
#define REG_SCE_CTRL_FIFOEN   (*(volatile uint32_t *)(SCE_REG_CTRL + 0x30))
#define REG_SCE_CTRL_FIFOCLR  (*(volatile uint32_t *)(SCE_REG_CTRL + 0x34))

// fifo bit0-4 fifo status   bit5-15  fifo count
#define REG_SCE_CTRL_FIFOCNT0 (*(volatile uint32_t *)(SCE_REG_CTRL + 0x40))
#define REG_SCE_CTRL_FIFOCNT1 (*(volatile uint32_t *)(SCE_REG_CTRL + 0x44))
#define REG_SCE_CTRL_FIFOCNT2 (*(volatile uint32_t *)(SCE_REG_CTRL + 0x48))
#define REG_SCE_CTRL_FIFOCNT3 (*(volatile uint32_t *)(SCE_REG_CTRL + 0x4C))
#define REG_SCE_CTRL_FIFOCNT4 (*(volatile uint32_t *)(SCE_REG_CTRL + 0x50))
#define REG_SCE_CTRL_FIFOCNT5 (*(volatile uint32_t *)(SCE_REG_CTRL + 0x54))

// REG_SCE_CTRL_SCEMODE
#define SCEMODE_NORMAL    0
#define SCEMODE_EXCLUSIVE 1
#define SCEMODE_SECURE    2

#define SET_SECMODE_NORMAL()    (REG_SCE_CTRL_SCEMODE = SCEMODE_NORMAL)
#define SET_SECMODE_EXCLUSIVE() (REG_SCE_CTRL_SCEMODE = SCEMODE_EXCLUSIVE)
#define SET_SECMODE_SECURE()    (REG_SCE_CTRL_SCEMODE = SCEMODE_SECURE)

// REG_SCE_CTRL_SUBCLKEN
#define SUBCLKEN_SDMA_EN (0x01 << 0)
#define SUBCLKEN_AES_EN  (0x01 << 1)
#define SUBCLKEN_PKE_EN  (0x01 << 2)
#define SUBCLKEN_HASH_EN (0x01 << 3)
#define SUBCLKEN_ALU_EN  (0x01 << 4)

#define SCE_SUBCLK_CLR() (REG_SCE_CTRL_SUBCLKEN = 0)

#define SCE_SDMA_CLK_EN()  (REG_SCE_CTRL_SUBCLKEN |= SUBCLKEN_SDMA_EN)
#define SCE_SDMA_CLK_DIS() (REG_SCE_CTRL_SUBCLKEN &= ~SUBCLKEN_SDMA_EN)

#define SCE_AES_CLK_EN()  (REG_SCE_CTRL_SUBCLKEN |= SUBCLKEN_AES_EN)
#define SCE_AES_CLK_DIS() (REG_SCE_CTRL_SUBCLKEN &= ~SUBCLKEN_AES_EN)

#define SCE_PKE_CLK_EN()  (REG_SCE_CTRL_SUBCLKEN |= SUBCLKEN_PKE_EN)
#define SCE_PKE_CLK_DIS() (REG_SCE_CTRL_SUBCLKEN &= ~SUBCLKEN_PKE_EN)

#define SCE_HASH_CLK_EN()  (REG_SCE_CTRL_SUBCLKEN |= SUBCLKEN_HASH_EN)
#define SCE_HASH_CLK_DIS() (REG_SCE_CTRL_SUBCLKEN &= ~SUBCLKEN_HASH_EN)

#define SCE_ALU_CLK_EN()  (REG_SCE_CTRL_SUBCLKEN |= SUBCLKEN_ALU_EN)
#define SCE_ALU_CLK_DIS() (REG_SCE_CTRL_SUBCLKEN &= ~SUBCLKEN_ALU_EN)

// REG_SCE_CTRL_AHBSOPT
#define SCE_AHB_READ_ENDIAN_SWAP() (REG_SCE_CTRL_AHBSOPT |= (0x01 << 4)) // ahb read endian swap
#define SCE_AHB_WRITE_XOR_MODE()   (REG_SCE_CTRL_AHBSOPT |= (0x01 << 3)) // ahb write xor mode
#define SCE_AHB_SWAP_DW()              \
    {                                  \
        REG_SCE_CTRL_AHBSOPT &= ~0x03; \
        REG_SCE_CTRL_AHBSOPT |= 0x00;  \
    } // ahb write endian swap  swap DR  DWX = DW ^ DR
#define SCE_AHB_SWAP_DWX()             \
    {                                  \
        REG_SCE_CTRL_AHBSOPT &= ~0x03; \
        REG_SCE_CTRL_AHBSOPT |= 0x01;  \
    } // swap DWX
#define SCE_AHB_SWAP_DR()              \
    {                                  \
        REG_SCE_CTRL_AHBSOPT &= ~0x03; \
        REG_SCE_CTRL_AHBSOPT |= 0x02;  \
    } // swap DW

// REG_SCE_CTRL_BUSYSR
#define SCE_RAM_CLEAR_BUSY (0x01 << 8)
#define SCE_SDMA_XCH_BUSY  (0x01 << 7)
#define SCE_SDMA_SCH_BUSY  (0x01 << 6)
#define SCE_SDMA_ICH_BUSY  (0x01 << 5)
#define SCE_SDMA_ALU_BUSY  (0x01 << 4)
#define SCE_SDMA_HASH_BUSY (0x01 << 3)
#define SCE_SDMA_PKE_BUSY  (0x01 << 2)
#define SCE_SDMA_AES_BUSY  (0x01 << 1)
#define SCE_SDMA_RNG_BUSY  (0x01 << 0)

#define SCE_IS_BUSY(t) (REG_SCE_CTRL_BUSYSR & t)
#define SCE_RAM_CLEAR_DONE()                             \
    {                                                    \
        while (REG_SCE_CTRL_BUSYSR & SCE_RAM_CLEAR_BUSY) \
            ;                                            \
    }

// REG_SCE_CTRL_DONEFR
#define SCE_SDMA_XCH_DONE  (0x01 << 7)
#define SCE_SDMA_SCH_DONE  (0x01 << 6)
#define SCE_SDMA_ICH_DONE  (0x01 << 5)
#define SCE_SDMA_ALU_DONE  (0x01 << 4)
#define SCE_SDMA_HASH_DONE (0x01 << 3)
#define SCE_SDMA_PKE_DONE  (0x01 << 2)
#define SCE_SDMA_AES_DONE  (0x01 << 1)
#define SCE_SDMA_RNG_DONE  (0x01 << 0)

#define SCE_SDMA_DONE(t)                     \
    {                                        \
        while (!(REG_SCE_CTRL_DONEFR & (t))) \
            ;                                \
        REG_SCE_CTRL_DONEFR |= (t);          \
    }

// REG_SCE_CTRL_ERRFR
#define SCE_SDMA_XCH_ERR  (7)
#define SCE_SDMA_SCH_ERR  (6)
#define SCE_SDMA_ICH_ERR  (5)
#define SCE_SDMA_ALU_ERR  (4)
#define SCE_SDMA_HASH_ERR (3)
#define SCE_SDMA_PKE_ERR  (2)
#define SCE_SDMA_AES_ERR  (1)
#define SCE_SDMA_RNG_ERR  (0)

#define SCE_SDMA_IS_ERR(t) (((REG_SCE_CTRL_ERRFR) >> (t)) & 1)

// REG_SCE_CTRL_RESETRAM
#define SCE_CLR_RAM()       (REG_SCE_CTRL_RESETRAM = 0xA5)
#define SCE_CLR_RESET_RAM() (REG_SCE_CTRL_RESETRAM = 0x5A) // Clear SCE algorithm core RAM? SCE_RAM is not cleared

// REG_SCE_CTRL_FIFOEN    BIT5-BIT0 correspond to FIFO5---FIFO0
#define SCE_FIFOEN_CLR() (REG_SCE_CTRL_FIFOEN = 0)

#define SCE_FIFO_0_EN()  (REG_SCE_CTRL_FIFOEN |= (0x01 << 0))
#define SCE_FIFO_0_DIS() (REG_SCE_CTRL_FIFOEN &= ~(0x01 << 0))

#define SCE_FIFO_1_EN()  (REG_SCE_CTRL_FIFOEN |= (0x01 << 1))
#define SCE_FIFO_1_DIS() (REG_SCE_CTRL_FIFOEN &= ~(0x01 << 1))

#define SCE_FIFO_2_EN()  (REG_SCE_CTRL_FIFOEN |= (0x01 << 2))
#define SCE_FIFO_2_DIS() (REG_SCE_CTRL_FIFOEN &= ~(0x01 << 2))

#define SCE_FIFO_3_EN()  (REG_SCE_CTRL_FIFOEN |= (0x01 << 3))
#define SCE_FIFO_3_DIS() (REG_SCE_CTRL_FIFOEN &= ~(0x01 << 3))

#define SCE_FIFO_4_EN()  (REG_SCE_CTRL_FIFOEN |= (0x01 << 4))
#define SCE_FIFO_4_DIS() (REG_SCE_CTRL_FIFOEN &= ~(0x01 << 4))

#define SCE_FIFO_5_EN()  (REG_SCE_CTRL_FIFOEN |= (0x01 << 5))
#define SCE_FIFO_5_DIS() (REG_SCE_CTRL_FIFOEN &= ~(0x01 << 5))

#define SCE_FIFO_0          0x01
#define SCE_FIFO_1          0x02
#define SCE_FIFO_2          0x04
#define SCE_FIFO_3          0x08
#define SCE_FIFO_4          0x10
#define SCE_FIFO_5          0x20
#define SCE_FIFO_ENABLE(t)  (REG_SCE_CTRL_FIFOEN = t)
#define SCE_FIFO_DISABLE(t) (REG_SCE_CTRL_FIFOEN &= ~(t))

// REG_SCE_CTRL_FIFOCLR
#define SCE_CLR_FIFO_0() (REG_SCE_CTRL_FIFOCLR = 0x0000FF00)
#define SCE_CLR_FIFO_1() (REG_SCE_CTRL_FIFOCLR = 0x0000FF01)
#define SCE_CLR_FIFO_2() (REG_SCE_CTRL_FIFOCLR = 0x0000FF02)
#define SCE_CLR_FIFO_3() (REG_SCE_CTRL_FIFOCLR = 0x0000FF03)
#define SCE_CLR_FIFO_4() (REG_SCE_CTRL_FIFOCLR = 0x0000FF04)
#define SCE_CLR_FIFO_5() (REG_SCE_CTRL_FIFOCLR = 0x0000FF05)

// REG_SCE_CTRL_FIFOCNT
// FIFO STATUS
#define FIFO_FULL         0x08
#define FIFO_ALMOST_FULL  0x04
#define FIFO_EMPTY        0x02
#define FIFO_ALMOST_EMPTY 0x01

#define SCE_FIFO_0_CNT() ((REG_SCE_CTRL_FIFOCNT0 >> 4) & 0x00000FFF)
#define SCE_FIFO_1_CNT() ((REG_SCE_CTRL_FIFOCNT1 >> 4) & 0x00000FFF)
#define SCE_FIFO_2_CNT() ((REG_SCE_CTRL_FIFOCNT2 >> 4) & 0x00000FFF)
#define SCE_FIFO_3_CNT() ((REG_SCE_CTRL_FIFOCNT3 >> 4) & 0x00000FFF)
#define SCE_FIFO_4_CNT() ((REG_SCE_CTRL_FIFOCNT4 >> 4) & 0x00000FFF)
#define SCE_FIFO_5_CNT() ((REG_SCE_CTRL_FIFOCNT5 >> 4) & 0x00000FFF)

#define SCE_GET_FIFO_STATS(t) (t & 0x0000000F)

/*----------------------SCE_SDMA------------------------*/
#define SCE_REG_SDMA       SCE_BASE_ADDR + 0x9000
#define REG_SCE_SDMA_START (*(volatile uint32_t *)(SCE_REG_SDMA + 0x0))

#define REG_SCE_SDMA_XCH_FUNC         (*(volatile uint32_t *)(SCE_REG_SDMA + 0x10))
#define REG_SCE_SDMA_XCH_OPT          (*(volatile uint32_t *)(SCE_REG_SDMA + 0x14))
#define REG_SCE_SDMA_XCH_ADDR         (*(volatile uint32_t *)(SCE_REG_SDMA + 0x18))
#define REG_SCE_SDMA_XCH_SEGMENT_ID   (*(volatile uint32_t *)(SCE_REG_SDMA + 0x1C))
#define REG_SCE_SDMA_XCH_SEGMENT_ADDR (*(volatile uint32_t *)(SCE_REG_SDMA + 0x20))
#define REG_SCE_SDMA_XCH_SIZE         (*(volatile uint32_t *)(SCE_REG_SDMA + 0x24))

#define REG_SCE_SDMA_SCH_FUNC         (*(volatile uint32_t *)(SCE_REG_SDMA + 0x30))
#define REG_SCE_SDMA_SCH_OPT          (*(volatile uint32_t *)(SCE_REG_SDMA + 0x34))
#define REG_SCE_SDMA_SCH_ADDR         (*(volatile uint32_t *)(SCE_REG_SDMA + 0x38))
#define REG_SCE_SDMA_SCH_SEGMENT_ID   (*(volatile uint32_t *)(SCE_REG_SDMA + 0x3C))
#define REG_SCE_SDMA_SCH_SEGMENT_ADDR (*(volatile uint32_t *)(SCE_REG_SDMA + 0x40))
#define REG_SCE_SDMA_SCH_SIZE         (*(volatile uint32_t *)(SCE_REG_SDMA + 0x44))

#define REG_SCE_SDMA_ICH_SEGMENT_ID     (*(volatile uint32_t *)(SCE_REG_SDMA + 0x54))
#define REG_SCE_SDMA_ICH_OPT            (*(volatile uint32_t *)(SCE_REG_SDMA + 0x50))
#define REG_SCE_SDMA_ICH_SEGMENT_ADDR_R (*(volatile uint32_t *)(SCE_REG_SDMA + 0x58))
#define REG_SCE_SDMA_ICH_SEGMENT_ADDR_W (*(volatile uint32_t *)(SCE_REG_SDMA + 0x5C))
#define REG_SCE_SDMA_ICH_SIZE           (*(volatile uint32_t *)(SCE_REG_SDMA + 0x60))

// REG_SCE_SDMA_START
#define ICH_START 0x5A
#define XCH_START 0xA5
#define SCH_START 0xAA
#define SCE_SDMA_START(t)       \
    {                           \
        REG_SCE_SDMA_START = t; \
        __DSB();                \
    }

// REG_SCE_SDMA_XCH_FUNC   REG_SCE_SDMA_SCH_FUNC   SCE_XCH_SCH_SDMA_FUNC(SEC_XCH_DMA_TYPE,XCH_SCH_AXI_READ)
#define SEC_XCH_DMA_TYPE 0x00
#define SEC_SCH_DMA_TYPE 0x01

#define XCH_SCH_AXI_READ               0x00 // SRAM/RERAM/KEYSLOTS TO SCERAM(segid)
#define XCH_SCH_AXI_WRITE              0x01 // SCERAM(segid) TO SRAM/RERAM/KEYSLOTS
#define SCE_XCH_SCH_SDMA_FUNC(type, t) (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x00) = t)

// REG_SCE_SDMA_XCH_OPT  REG_SCE_SDMA_SCH_OPT
#define SCE_XCH_SCH_SDMA_OPT_CLR(type) (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x04) = 0x00)

#define SCE_XCH_SDMA_OPT_LEN(type, t)                (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x04) |= (t << 5))
#define SCE_XCH_SCH_SDMA_OPT_WRITE_ENDIAN_SWAP(type) (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x04) |= (0x01 << 4))
#define SCE_XCH_SCH_SDMA_OPT_READ_XOR(type)          (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x04) |= (0x01 << 3))
#define SCE_XCH_SCH_SDMA_OPT_READ_ENDIAN_SWAP(type) \
    (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x04) |= (0x01 << 0)) // bit0-2 read eNdian swap   0 -> writedata, 1-> newdata, 2-> olddata

#define SDMA_XCH_SCH_OPT_NORMAL     0
#define SDMA_XCH_SCH_OPT_WRITE_SWAP (0x01 << 4)
#define SDMA_XCH_SCH_OPT_READ_XOR   (0x01 << 3)
#define SDMA_XCH_SCH_OPT_READ_SWAP  (0x01 << 0) // 0 -> writedata, 1-> newdata, 2-> olddata

#define SCE_XCH_SCH_SDMA_OPT_SET(type, t) (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x04) = t)

// REG_SCE_SDMA_XCH_ADDR  REG_SCE_SDMA_SCH_ADDR
#define SCE_XCH_SCH_SDMA_ADDR(type, t) (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x08) = t)

// REG_SCE_SDMA_XCH_SEGMENT_ID  REG_SCE_SDMA_SCH_SEGMENT_ID
#define SCE_XCH_SCH_SDMA_SEG_ID(type, t) (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x0C) = t)

// REG_SCE_SDMA_XCH_SEGMENT_ADDR   REG_SCE_SDMA_SCH_SEGMENT_ADDR
#define SCE_XCH_SCH_SDMA_SEG_ADDR(type, t) (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x10) = t)

// REG_SCE_SDMA_XCH_SIZE  REG_SCE_SDMA_SCH_SIZE
#define SCE_XCH_SCH_SDMA_SIZE(type, t) (*(volatile uint32_t *)((SCE_REG_SDMA + (2 * type + 1) * 0x10) + 0x14) = t)

// REG_SCE_SDMA_ICH_OPT
#define SDMA_ICH_OPT_NORMAL 0
#define SDMA_ICH_OPT_XOR    (0x01 << 0)
#define SDMA_ICH_OPT_SWAP   (0x01 << 1) // bit1-3  01 - swap endian in a word

#define SCE_ICH_SDMA_OPT_CLR()  (REG_SCE_SDMA_ICH_OPT = 0)
#define SCE_ICH_SDMA_OPT_SET(t) (REG_SCE_SDMA_ICH_OPT = (t))

// REG_SCE_SDMA_ICH_SEGMENT_ID
#define SCE_ICH_SDMA_CLR_SEG_ID(t)   (REG_SCE_SDMA_ICH_SEGMENT_ID = 0)
#define SCE_ICH_SDMA_WRITE_SEG_ID(t) (REG_SCE_SDMA_ICH_SEGMENT_ID = (REG_SCE_SDMA_ICH_SEGMENT_ID & 0xFFFFFF00) | (t & 0x000000FF))
#define SCE_ICH_SDMA_READ_SEG_ID(t)  (REG_SCE_SDMA_ICH_SEGMENT_ID = (REG_SCE_SDMA_ICH_SEGMENT_ID & 0xFFFF00FF) | ((t << 8) & 0x0000FF00))

// REG_SCE_SDMA_ICH_SEGMENT_ADDR_R
#define SCE_ICH_SDMA_READ_SEG_ADDR(t) (REG_SCE_SDMA_ICH_SEGMENT_ADDR_R = t)

// REG_SCE_SDMA_ICH_SEGMENT_ADDR_W
#define SCE_ICH_SDMA_WRITE_SEG_ADDR(t) (REG_SCE_SDMA_ICH_SEGMENT_ADDR_W = t)

// REG_SCE_SDMA_ICH_SIZE
#define SCE_ICH_SDMA_SIZE(t) (REG_SCE_SDMA_ICH_SIZE = t)

/*----------------------SCE_HASH------------------------*/
#define SCE_REG_HASH        SCE_BASE_ADDR + 0xB000
#define REG_SCE_HASH_CRFUNC (*(volatile uint32_t *)(SCE_REG_HASH + 0x0))
#define REG_SCE_HASH_AR     (*(volatile uint32_t *)(SCE_REG_HASH + 0x04))
#define REG_SCE_HASH_SRMFSM (*(volatile uint32_t *)(SCE_REG_HASH + 0x08))
#define REG_SCE_HASH_FR     (*(volatile uint32_t *)(SCE_REG_HASH + 0x0C))
#define REG_SCE_HASH_OPT1   (*(volatile uint32_t *)(SCE_REG_HASH + 0x10))
#define REG_SCE_HASH_OPT2   (*(volatile uint32_t *)(SCE_REG_HASH + 0x14))
#define REG_SCE_HASH_OPT3   (*(volatile uint32_t *)(SCE_REG_HASH + 0x18)) // endian data indicator
#define REG_SCE_HASH_BLKT0  (*(volatile uint32_t *)(SCE_REG_HASH + 0x1C))
#define REG_SCE_HASH_LKEY   (*(volatile uint32_t *)(SCE_REG_HASH + 0x20))
#define REG_SCE_HASH_KEY    (*(volatile uint32_t *)(SCE_REG_HASH + 0x24))
#define REG_SCE_HASH_SKEY   (*(volatile uint32_t *)(SCE_REG_HASH + 0x28))
#define REG_SCE_HASH_SCRT   (*(volatile uint32_t *)(SCE_REG_HASH + 0x2C))
#define REG_SCE_HASH_MSG    (*(volatile uint32_t *)(SCE_REG_HASH + 0x30))
#define REG_SCE_HASH_HOUT   (*(volatile uint32_t *)(SCE_REG_HASH + 0x34))
#define REG_SCE_HASH_SOB    (*(volatile uint32_t *)(SCE_REG_HASH + 0x38))
#define REG_SCE_HASH_RESULT (*(volatile uint32_t *)(SCE_REG_HASH + 0x3C))

#define HF_SHA256          0x0
#define HF_SHA512          0x01
#define HF_RIPMID          0x02
#define HF_BLK2S           0x03
#define HF_BLK2B           0x04
#define HF_BLK3            0x05
#define HF_SHA3            0x06
#define HF_HMAC256_KEYHASH 0x40
#define HF_HMAC256_PASS1   0x50
#define HF_HMAC256_PASS2   0x60
#define HF_HMAC512_KEYHASH 0x41
#define HF_HMAC512_PASS1   0x51
#define HF_HMAC512_PASS2   0x61
#define HF_INIT            0xFF

// REG_SCE_HASH_CRFUNC
#define SCE_HASH_FUNC(t) (REG_SCE_HASH_CRFUNC = (t & 0x000000FF))

// REG_SCE_HASH_AR
#define SCE_HASH_AR()           \
    {                           \
        REG_SCE_HASH_FR = 0x0F; \
        __DMB();                \
        REG_SCE_HASH_AR = 0x5A; \
        __DSB();                \
    } // Clear completion flag, then start

// REG_SCE_HASH_SRMFSM    for debug

// REG_SCE_HASH_FR   write 1 to clear
#define HASH_INPUT_DONE  (0x01 << 3)
#define HASH_OUTPUT_DONE (0x01 << 2)
#define HASH_DONE        (0x01 << 1)
#define HASH_MFSM_DONE   (0x01 << 0)

#define SCE_HASH_INPUT_DONE()                        \
    {                                                \
        while (!(REG_SCE_HASH_FR & HASH_INPUT_DONE)) \
            ;                                        \
        REG_SCE_HASH_FR |= HASH_INPUT_DONE;          \
        __DMB();                                     \
    }
#define SCE_HASH_OUTPUT_DONE()                        \
    {                                                 \
        while (!(REG_SCE_HASH_FR & HASH_OUTPUT_DONE)) \
            ;                                         \
        REG_SCE_HASH_FR |= HASH_OUTPUT_DONE;          \
        __DMB();                                      \
    }
#define SCE_HASH_DONE()                        \
    {                                          \
        while (!(REG_SCE_HASH_FR & HASH_DONE)) \
            ;                                  \
        REG_SCE_HASH_FR |= HASH_DONE;          \
        __DMB();                               \
    }
#define SCE_HASH_MFSM_DONE()                        \
    {                                               \
        while (!(REG_SCE_HASH_FR & HASH_MFSM_DONE)) \
            ;                                       \
        REG_SCE_HASH_FR |= HASH_MFSM_DONE;          \
        __DMB();                                    \
    } // algorithm completion

// REG_SCE_HASH_OPT1
#define SCE_HASH_OPT1_CNT(t) (REG_SCE_HASH_OPT1 = t)

// REG_SCE_HASH_OPT2

#define HASH_IF_START   (0x01 << 2)
#define HASH_IF_SOB     (0x01 << 1)
#define HASH_CHECK_SCRT (0x01 << 0)

#define SCE_HASH_OPT2_MODE(t) (REG_SCE_HASH_OPT2 = t)

#define SCE_HASH_OPT2_CLR()            (REG_SCE_HASH_OPT2 = 0)
#define SCE_HASH_OPT2_FIRT_BLOCK()     (REG_SCE_HASH_OPT2 |= (0x01 << 2))
#define SCE_HASH_OPT2_FIRT_BLOCK_CLR() (REG_SCE_HASH_OPT2 &= ~(0x01 << 2))

#define SCE_HASH_OPT2_SEBSOB()       (REG_SCE_HASH_OPT2 |= (0x01 << 1)) // result output to SOB encryption buffer
#define SCE_HASH_OPT2_CHECK_SECRET() (REG_SCE_HASH_OPT2 |= (0x01 << 0)) //

// REG_SCE_HASH_OPT3
#define SCE_HASH_OPT3_SEG_LKEY   (0x01 << 0)
#define SCE_HASH_OPT3_SEG_KEY    (0x01 << 1)
#define SCE_HASH_OPT3_SEG_SKEY   (0x01 << 2)
#define SCE_HASH_OPT3_SEG_SCRT   (0x01 << 3)
#define SCE_HASH_OPT3_SEG_MSG    (0x01 << 4)
#define SCE_HASH_OPT3_SEG_HOUT   (0x01 << 5) // HOUT is both output and input, final result output format is configured by SCE_HASH_OPT3_SEG_RESULT
#define SCE_HASH_OPT3_SEG_SOB    (0x01 << 6)
#define SCE_HASH_OPT3_SEG_RESULT (0x01 << 7)

#define SCE_HASH_OPT3_MODE(t) (REG_SCE_HASH_OPT3 = (t))

// REG_SCE_HASH_LKEY
#define SCE_HASH_SET_SEG_LKEY(t) (REG_SCE_HASH_LKEY = t) // bit13-0   base address offset

// REG_SCE_HASH_KEY
#define SCE_HASH_SET_SEG_KEY(t) (REG_SCE_HASH_KEY = t) // bit13-0   base address offset

// REG_SCE_HASH_SKEY
#define SCE_HASH_SET_SEG_SKEY(t) (REG_SCE_HASH_SKEY = t) // bit13-0   base address offset

// REG_SCE_HASH_SCRT
#define SCE_HASH_SET_SEG_SCRT(t) (REG_SCE_HASH_SCRT = t) // bit13-0   base address offset

// REG_SCE_HASH_MSG
#define SCE_HASH_SET_MSG(t) (REG_SCE_HASH_MSG = t) // bit13-0   base address offset

// REG_SCE_HASH_HOUT
#define SCE_HASH_OUTPUT(t) (REG_SCE_HASH_HOUT = t) // bit13-0   base address offset

// REG_SCE_HASH_SOB
#define SCE_HASH_SOB_OUTPUT(t) (REG_SCE_HASH_SOB = t) // bit13-0   base address offset

/*----------------------SCE_PKE------------------------*/
#define SCE_REG_PKE         SCE_BASE_ADDR + 0xC000
#define REG_SCE_PKE_CRFUNC  (*(volatile uint32_t *)(SCE_REG_PKE + 0x0))
#define REG_SCE_PKE_AR      (*(volatile uint32_t *)(SCE_REG_PKE + 0x04))
#define REG_SCE_PKE_SRMFSM  (*(volatile uint32_t *)(SCE_REG_PKE + 0x08)) // debug register
#define REG_SCE_PKE_FR      (*(volatile uint32_t *)(SCE_REG_PKE + 0x0C))
#define REG_SCE_PKE_OPTNW   (*(volatile uint32_t *)(SCE_REG_PKE + 0x10))
#define REG_SCE_PKE_OPTEW   (*(volatile uint32_t *)(SCE_REG_PKE + 0x14))
#define REG_SCE_PKE_OPTRW   (*(volatile uint32_t *)(SCE_REG_PKE + 0x18))
#define REG_SCE_PKE_OPTLTX  (*(volatile uint32_t *)(SCE_REG_PKE + 0x1C)) // endian data indicator
#define REG_SCE_PKE_OPTMASK (*(volatile uint32_t *)(SCE_REG_PKE + 0x20))
#define REG_SCE_PKE_MIMMCR  (*(volatile uint32_t *)(SCE_REG_PKE + 0x24))
#define REG_SCE_PKE_PCON    (*(volatile uint32_t *)(SCE_REG_PKE + 0x30))
#define REG_SCE_PKE_PIB0    (*(volatile uint32_t *)(SCE_REG_PKE + 0x34))
#define REG_SCE_PKE_PIB1    (*(volatile uint32_t *)(SCE_REG_PKE + 0x38))
#define REG_SCE_PKE_PKB     (*(volatile uint32_t *)(SCE_REG_PKE + 0x3C))
#define REG_SCE_PKE_POB     (*(volatile uint32_t *)(SCE_REG_PKE + 0x40))

/*
0x103 -- 4 multipliers
0x101 -- 2 multipliers
0x100 -- 1 multiplier
*/
#define PKE_MIMMCR_CONFIG 0x103

enum PIR_CFUN
{
    PIR_ECINIT = 0x01,  // ECC/ECD Init
    PIR_ECMM,           // ECC ModMul
    PIR_ECI2MA,         // ECC Integer To Montgomery  For Add
    PIR_ECI2MD,         // ECC Integer To Montgomery  For Double
    PIR_ECPA,           // ECC Point Add
    PIR_ECPD,           // ECC Point Double
    PIR_ECM2I,          // ECC Montgomery To Integer
    PIR_ECINV,          // ECC ModInv
    PIR_ECMA,           // ECC ModAdd
    PIR_ECMS,           // ECC ModSub
    PIR_ECPM,           // ECC Ponit Multiply
    PIR_RSAINIT = 0x11, // Rsa Init
    PIR_RSAMM,          // Rsa ModMul  a*b mod N
    PIR_RSAME,          // Rsa ModExp  a^b mod N
    PIR_MODINV = 0x18,  // Rsa ModInv
    PIR_RSAMA,          // Rsa ModAdd
    PIR_RSAMS,          // Rsa ModSub
    PIR_EDI2MA = 0x23,  // EDDSA Integer To Montgomery  For Add
    PIR_EDI2MD,         // EDDSA Integer To Montgomery  For Double
    PIR_EDPA,           // EDDSA Point Add
    PIR_EDPD,           // EDDSA Point Double
    PIR_EDM2I,          // EDDSA  Montgomery To Integer
    PIR_EDPM    = 0x2B, // EDDSA Ponit Multiply
    PIR_X25519  = 0x3B, // Curve 25519
    PIR_GCD     = 0x51, // Gcd
    PIR_MODL    = 0x81, // r MOD L used in 25519 signature
    PIR_INVINIT = 0xFE  // Invert Init
};

// REG_SCE_PKE_CRFUNC
#define SCE_PKE_FUNC(t) (REG_SCE_PKE_CRFUNC = ((t) & 0x000000FF))
#define SCE_PKE_FUNC_EX(t)                       \
    {                                            \
        REG_SCE_PKE_CRFUNC = ((t) & 0x000000FF); \
        REG_SCE_PKE_MIMMCR = 0x103;              \
    } // ModMul ModExp acceleration

// REG_SCE_PKE_AR
#define SCE_PKE_AR()           \
    {                          \
        REG_SCE_PKE_FR = 0x0F; \
        __DMB();               \
        REG_SCE_PKE_AR = 0x5A; \
        __DSB();               \
    } // Clear completion flag, then start

// REG_SCE_PKE_FR   write 1 to clear
#define SCE_PKE_DONE()                   \
    {                                    \
        while (!(REG_SCE_PKE_FR & 0x01)) \
            ;                            \
        REG_SCE_PKE_FR |= 0x01;          \
    } // algorithm completion
#define SCE_PKE_INV_READY()                   \
    {                                         \
        while (!(REG_SCE_PKE_SRMFSM & 0x100)) \
            ;                                 \
        REG_SCE_PKE_SRMFSM |= 0x100;          \
    } // bit8 of sfr_sr is mod inv ready

#ifdef NEW_VERSION
// REG_SCE_PKE_OPTNW
#define SCE_PKE_OPTNW(t) (REG_SCE_PKE_OPTNW = ((t) & 0x00003FFF))

// REG_SCE_PKE_OPTEW
#define SCE_PKE_OPTEW(t) (REG_SCE_PKE_OPTEW = ((t) & 0x00003FFF))
#else
// REG_SCE_PKE_OPTNW
#define SCE_PKE_OPTNW(t) (REG_SCE_PKE_OPTNW = ((t) & 0x00001FFF))

// REG_SCE_PKE_OPTEW
#define SCE_PKE_OPTEW(t) (REG_SCE_PKE_OPTEW = ((t) & 0x00001FFF))
#endif

// REG_SCE_PKE_OPTRW
#define SCE_PKE_OPTRW(t) (REG_SCE_PKE_OPTRW = ((t) & 0x000003FF))

// REG_SCE_PKE_OPTLTX
#define REG_SCE_PKE_OPTLTX_PCON       (0x01 << 0)
#define REG_SCE_PKE_OPTLTX_PIB0_PSIB0 (0x01 << 1)
#define REG_SCE_PKE_OPTLTX_PIB1_PSIB1 (0x01 << 2)
#define REG_SCE_PKE_OPTLTX_PKB        (0x01 << 3)
#define REG_SCE_PKE_OPTLTX_POB_PSOB   (0x01 << 4)

#define SCE_PKE_OPTLTX_MODE(t) (REG_SCE_PKE_OPTLTX = (t))

// REG_SCE_PKE_PCON
#define SCE_PKE_SET_PCON(t) (REG_SCE_PKE_PCON = (t)) // bit11-0   base address offset

// REG_SCE_PKE_PIB0
#define SCE_PKE_SET_PIB0(t) (REG_SCE_PKE_PIB0 = (t)) // bit11-0   base address offset

// REG_SCE_PKE_PIB1
#define SCE_PKE_SET_PIB1(t) (REG_SCE_PKE_PIB1 = (t)) // bit11-0   base address offset

// REG_SCE_PKE_PKB
#define SCE_PKE_SET_PKB(t) (REG_SCE_PKE_PKB = (t)) // bit11-0   base address offset

// REG_SCE_PKE_POB
#define SCE_PKE_SET_POB(t) (REG_SCE_PKE_POB = (t)) // bit11-0   base address offset

/*----------------------SCE_AES------------------------*/
#define SCE_REG_AES        SCE_BASE_ADDR + 0xD000
#define REG_SCE_AES_CRFUNC (*(volatile uint32_t *)(SCE_REG_AES + 0x0))
#define REG_SCE_AES_AR     (*(volatile uint32_t *)(SCE_REG_AES + 0x04))
#define REG_SCE_AES_SRMFSM (*(volatile uint32_t *)(SCE_REG_AES + 0x08))
#define REG_SCE_AES_FR     (*(volatile uint32_t *)(SCE_REG_AES + 0x0C))
#define REG_SCE_AES_OPT1   (*(volatile uint32_t *)(SCE_REG_AES + 0x10))
#define REG_SCE_AES_OPT2   (*(volatile uint32_t *)(SCE_REG_AES + 0x14))
#define REG_SCE_AES_OPT3   (*(volatile uint32_t *)(SCE_REG_AES + 0x18)) // endian data indicator
#define REG_SCE_AES_IV     (*(volatile uint32_t *)(SCE_REG_AES + 0x30))
#define REG_SCE_AES_AKEY   (*(volatile uint32_t *)(SCE_REG_AES + 0x34))
#define REG_SCE_AES_AIB    (*(volatile uint32_t *)(SCE_REG_AES + 0x38))
#define REG_SCE_AES_AOB    (*(volatile uint32_t *)(SCE_REG_AES + 0x3C))

#define AF_KS  0x0
#define AF_ENC 0x01
#define AF_DEC 0x02

// REG_SCE_AES_CRFUNC
#define SCE_AES_FUNC(t) (REG_SCE_AES_CRFUNC = (t & 0x000000FF))

// REG_SCE_AES_AR
#define SCE_AES_AR()           \
    {                          \
        REG_SCE_AES_FR = 0x0F; \
        __DMB();               \
        REG_SCE_AES_AR = 0x5A; \
        __DSB();               \
    } // Clear completion flag, then start

// REG_SCE_AES_FR   write 1 to clear
#define SCE_AES_DONE()                   \
    {                                    \
        while (!(REG_SCE_AES_FR & 0x01)) \
            ;                            \
        REG_SCE_AES_FR |= 0x01;          \
    } // algorithm completion

// REG_SCE_AES_OPT1

#define AES_FIRST_SEG 0x100 // Except for ECB, other modes need to use this flag to identify whether it is the first block of data
#define AES_MODE_ECB  0x00
#define AES_MODE_CBC  0x10
#define AES_MODE_CTR  0x20
#define AES_MODE_CFB  0x40
#define AES_MODE_OFB  0x30

#define AES_128              0x00
#define AES_192              0x01
#define AES_256              0x02
#define SCE_AES_OPT1_MODE(t) (REG_SCE_AES_OPT1 = ((t) & 0x000000FF))

// REG_SCE_AES_OPT2
#define SCE_AES_OPT2_CNT(t) (REG_SCE_AES_OPT2 = (t))

// REG_SCE_AES_OPT3
#define SCE_HASH_OPT3_IV       (0x01 << 0)
#define SCE_HASH_OPT3_KEY      (0x01 << 1)
#define SCE_HASH_OPT3_AIB      (0x01 << 2)
#define SCE_HASH_OPT3_AOB      (0x01 << 3)
#define SCE_HASH_OPT3_CTR      (0x01 << 4)
#define SCE_HASH_OPT3_CTR_ZERO (0x01 << 5)

#define SCE_AES_OPT3_MODE(t) (REG_SCE_AES_OPT3 = (t))

// REG_SCE_AES_IV
#define SCE_AES_SET_IV(t) (REG_SCE_AES_IV = t) // bit11-0   base address offset

// REG_SCE_AES_AKEY
#define SCE_AES_SET_KEY(t) (REG_SCE_AES_AKEY = t) // bit11-0   base address offset

// REG_SCE_AES_AIB
#define SCE_AES_SET_MSG(t) (REG_SCE_AES_AIB = t) // bit11-0   base address offset

// REG_SCE_AES_AOB
#define SCE_AES_SET_OUT(t) (REG_SCE_AES_AOB = t) // bit11-0   base address offset

/*----------------------SCE_RNG------------------------*/
#define SCE_TRNG_BASE_ADDR (SCE_BASE_ADDR + 0xE000)
typedef volatile uint32_t io_rw_32;

#define TRNG_CONTINUOUS_MAX_ATTEMPTS (1024 / 4) // Maximum number of random numbers generated at one time (word)

typedef union
{
    struct
    {
        unsigned int cr_gen_en : 1;
        unsigned int cr_pfilter_en : 1;
        unsigned int cr_hlthtest_en : 1;
        unsigned int cr_drng_en : 1;
        unsigned int cr_postproc_opt : 2;
        unsigned int cr_healthtest_len : 6;
        unsigned int cr_gen_intval : 2;
        unsigned int cr_reseed_intval : 2;
        unsigned int rfu : 16;
    } bits;

    unsigned int postprocValue;
} uPostProc;

typedef struct
{
    /* data */
    io_rw_32  crsrc;    // 0x0
    io_rw_32  crana;    // 0x4
    uPostProc postproc; // 0x8
    // io_rw_32 postproc; // 0x8
    io_rw_32 opt;      // 0xc
    io_rw_32 sr;       // 0x10
    io_rw_32 ar;       // 0x14
    io_rw_32 fr;       // 0x18 ??
    io_rw_32 _pad1;    // 0x1c
    io_rw_32 drpsz;    // 0x20
    io_rw_32 drgen;    // 0x24
    io_rw_32 drreseed; // 0x28
    io_rw_32 _pad2;    // 0x2c
    io_rw_32 buf;      // 0x30
    io_rw_32 _pad3;    // 0x34
    io_rw_32 _pad4;    // 0x38
    io_rw_32 _pad5;    // 0x3c
    io_rw_32 chain0;   // 0x40
    io_rw_32 chain1;   // 0x44
} trng_t;

typedef struct
{
    trng_t           *trng;
    volatile uint32_t remain;
} trng_continuous_t;

#define trng_hw ((trng_t *)SCE_TRNG_BASE_ADDR)

// REG_SCE_TRNG_CRSRC
#define TRNGLF_EN_MASK (1 << 0)
#define TRNGHF_EN_MASK (1 << 1)

/*----------------------SCE_ALU------------------------*/
#define SCE_REG_ALU           SCE_BASE_ADDR + 0xF000
#define REG_SCE_ALU_CRFUNC    (*(volatile uint32_t *)(SCE_REG_ALU + 0x0))
#define REG_SCE_ALU_AR        (*(volatile uint32_t *)(SCE_REG_ALU + 0x04))
#define REG_SCE_ALU_SRMFSM    (*(volatile uint32_t *)(SCE_REG_ALU + 0x08))
#define REG_SCE_ALU_FR        (*(volatile uint32_t *)(SCE_REG_ALU + 0x0C))
#define REG_SCE_ALU_CRDIVLEN  (*(volatile uint32_t *)(SCE_REG_ALU + 0x10))
#define REG_SCE_ALU_SRDIVLEN  (*(volatile uint32_t *)(SCE_REG_ALU + 0x14))
#define REG_SCE_ALU_OPT       (*(volatile uint32_t *)(SCE_REG_ALU + 0x18))
#define REG_SCE_ALU_OPTLTX    (*(volatile uint32_t *)(SCE_REG_ALU + 0x1C)) // High bit is world internal flip, low bit is entire data flip according to world
#define REG_SCE_ALU_SEGPTR_DE (*(volatile uint32_t *)(SCE_REG_ALU + 0x30))
#define REG_SCE_ALU_SEGPTR_DS (*(volatile uint32_t *)(SCE_REG_ALU + 0x34))
#define REG_SCE_ALU_SEGPTR_QT (*(volatile uint32_t *)(SCE_REG_ALU + 0x38))
#define REG_SCE_ALU_SEGPTR_RM (*(volatile uint32_t *)(SCE_REG_ALU + 0x3C))

// REG_SCE_ALU_CRFUNC
#define ALU_AF_DIV  (0x01)
#define ALU_AF_ADD  (0x02)
#define ALU_AF_SUB  (0x03)
#define ALU_AF_SFT  (0x10)
#define ALU_AF_SFTW (0x11)
#define ALU_AF_BLG  (0x20)
#define ALU_AF_BEX  (0x30)

// REG_SCE_ALU_OPT

// BEX CONFIG
#define REG_SCE_ALU_OPT_WD64   (1 << 19)
#define REG_SCE_ALU_OPT_WD32   (1 << 18)
#define REG_SCE_ALU_OPT_BYTE   (1 << 17)
#define REG_SCE_ALU_OPT_BIT    (1 << 16)
#define REG_SCE_ALU_OPT_BEX(t) (REG_SCE_ALU_OPT = t)

// SFT AND SFTW CONFIG
#define REG_SCE_ALU_OPT_LEFT        (1 << 2)
#define REG_SCE_ALU_OPT_RIGHT       (0 << 2)
#define REG_SCE_ALU_OPT_DIR(t)      (REG_SCE_ALU_OPT = (REG_SCE_ALU_OPT & 0x0000FFF8) | ((t & 0x00000007)))
#define REG_SCE_ALU_OPT_SFTCOUNT(t) (REG_SCE_ALU_OPT = (REG_SCE_ALU_OPT & 0x000000FF) | ((t & 0x000000FF) << 8))

// BLG CONFIG
#define TRUE11                 (1 << 7)
#define TRUE01                 (1 << 6)
#define TRUE10                 (1 << 5)
#define TRUE00                 (1 << 4)
#define REG_SCE_ALU_OPT_BLG(t) (REG_SCE_ALU_OPT = t)

#define SCE_ALU_FUNC(t) (REG_SCE_ALU_CRFUNC = (t & 0x000000FF))

// REG_SCE_ALU_AR
#define SCE_ALU_AR()           \
    {                          \
        REG_SCE_ALU_FR = 0xFF; \
        __DMB();               \
        REG_SCE_ALU_AR = 0x5A; \
        __DSB();               \
    } // clear flag then start
#define SCE_ALU_DONE()                   \
    {                                    \
        while (!(REG_SCE_ALU_FR & 0x01)) \
            ;                            \
        REG_SCE_ALU_FR |= 0x01;          \
        __DMB();                         \
    }

// REG_SCE_ALU_CRDIVLEN
// a is length of divident; b is length of divisor; uint is double word (64b)
#define SCE_ALU_DEDS_DWLEN(a, b) (REG_SCE_ALU_CRDIVLEN = (REG_SCE_ALU_CRDIVLEN & 0xFFFF0000) | ((a) & 0x000000FF) | (((b) & 0x000000FF) << 8))
// REG_SCE_ALU_SRDIVLEN
#define SCE_ALU_QT_DWLEN() (REG_SCE_ALU_SRDIVLEN & 0x000000FF)
#define SCE_ALU_RM_DWLEN() ((REG_SCE_ALU_SRDIVLEN & 0x0000FF00) >> 8)

// REG_SCE_ALU_OPTLTX
#define REG_SCE_ALU_OPTLTX_DE (0x01 << 0)
#define REG_SCE_ALU_OPTLTX_DS (0x01 << 1)
#define REG_SCE_ALU_OPTLTX_QT (0x01 << 2)
#define REG_SCE_ALU_OPTLTX_RM (0x01 << 3)

// REG_SCE_ALU_SEGPTR_
#define SCE_ALU_SEGPTR_DE(t, o) (REG_SCE_ALU_SEGPTR_DE = (o & 0x00000FFF) | ((t & 0x000000FF) << 12))
#define SCE_ALU_SEGPTR_DS(t, o) (REG_SCE_ALU_SEGPTR_DS = (o & 0x00000FFF) | ((t & 0x000000FF) << 12))
#define SCE_ALU_SEGPTR_QT(t, o) (REG_SCE_ALU_SEGPTR_QT = (o & 0x00000FFF) | ((t & 0x000000FF) << 12))
#define SCE_ALU_SEGPTR_RM(t, o) (REG_SCE_ALU_SEGPTR_RM = (o & 0x00000FFF) | ((t & 0x000000FF) << 12))
#endif
