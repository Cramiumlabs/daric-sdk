/** * Copyright 2024-2026 CrossBar, Inc.
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
#include <stdint.h>

#ifndef _BSP_COMMON_H_
#define _BSP_COMMON_H_

typedef struct
{
    uint8_t add;
    uint8_t val;
} REG8_MAP;

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define BIT(n) (1 << (n))

#endif //_BSP_COMMON_H_