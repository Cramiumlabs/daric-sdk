/**
 ******************************************************************************
 * @file    daric_hal_reram.h
 * @author  ReRam Team
 * @brief   Header file of ReRam HAL module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DARIC_HAL_RERAM_H
#define __DARIC_HAL_RERAM_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include "daric_hal_def.h"

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
    uint8_t HAL_RERAM_Read(uint32_t srcAddr, uint8_t *pRdBuf, uint32_t rdLen);

    /**
     * @brief  Write the data from the pWtBuf buffer to flash.
     * @param  dstAddr Write the starting address of ReRam.
     * @param  pWtBuf The source address of the buffer to be written,
     *         typically a buffer allocated in SRAM.
     * @param  wtLen The length of data to be write.
     * @retval Return 1 means success, return 0 value means failure.
     *
     * The function write data to a specified address in ReRam.
     */
    uint8_t HAL_RERAM_Write(uint32_t dstAddr, uint8_t *pWtBuf, uint32_t wtLen);

#ifdef __cplusplus
}
#endif

#endif /* __DARIC_HAL_RERAM_H */
