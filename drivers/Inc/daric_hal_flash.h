/**
 *******************************************************************************
 * @file    daric_hal_flash.h
 * @author  FLASH Team
 * @brief   Header file for FLASH HAL module.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef DARIC_HAL_FLASH_H
#define DARIC_HAL_FLASH_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include "daric_hal_spim.h"

    /**
     * SPIM handle for communication with the Flash device.
     * Before using Flash, the handle and corresponding pin configuration must be
     * initialized.
     */
    extern SPIM_HandleTypeDef g_flash_spim_handle;

    HAL_StatusTypeDef HAL_FLASH_Init(void);
    HAL_StatusTypeDef HAL_FLASH_BadBlock_Record(void);
    HAL_StatusTypeDef HAL_FLASH_Read_ID(uint8_t *ID);
    HAL_StatusTypeDef HAL_FLASH_Read(uint8_t *pData, uint16_t BlockAddr, uint8_t PageAddr, uint16_t BytesAddr, uint16_t Size);
    HAL_StatusTypeDef HAL_FLASH_Write(uint8_t *pData, uint16_t BlockAddr, uint8_t PageAddr, uint16_t BytesAddr, uint16_t Size);
    HAL_StatusTypeDef HAL_FLASH_Erase_BLOCK(uint16_t BlockAddr, uint16_t Size);
    HAL_StatusTypeDef HAL_FLASH_Read_QSPI(uint8_t *pData, uint16_t BlockAddr, uint8_t PageAddr, uint16_t BytesAddr, uint16_t Size);
    HAL_StatusTypeDef HAL_FLASH_Write_QSPI(uint8_t *pData, uint16_t BlockAddr, uint8_t PageAddr, uint16_t BytesAddr, uint16_t Size);

#ifdef __cplusplus
}
#endif

#endif