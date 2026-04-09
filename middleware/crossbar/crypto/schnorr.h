/**
 ******************************************************************************
 * @file    schnorr.h
 * @author  SCE Team
 * @brief   Header file for Schnorr signature operations.
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

#ifndef __DARIC_SCHNORR_H__
#define __DARIC_SCHNORR_H__

#include "ecdsa.h"
// #include "drv_conf.h"
// #ifdef DRV_SCE_ENABLED

int32_t schnorr_sign_digest(curve_type curve, const uint32_t *priv_key, const uint32_t *digest, uint32_t *sig);
int     schnorr_verify_digest(curve_type curve, const uint32_t *pub_key, const uint32_t *digest, const uint32_t *signature);
#endif
// #endif