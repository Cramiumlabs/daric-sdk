/**
 ******************************************************************************
 * @file    sdma.h
 * @author  SCE Team
 * @brief   Header file for SCE SDMA driver.
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

#ifndef __SDMA_H__
#define __SDMA_H__

void Xch_Sch_TranData(uint8_t type, uint8_t opmode, uint8_t opt, uint32_t address, uint8_t segId, uint8_t segId_off, uint32_t size);
void Ich_TranData(uint8_t opt, uint8_t segId_r, uint16_t segId_r_off, uint8_t segId_w, uint16_t segId_w_off, uint16_t size);

void memcpy_u32(void *dest, const void *src, size_t n);
#endif