/**
 ******************************************************************************
 * @file    eddsa.h
 * @author  SCE Team
 * @brief   Header file for EdDSA operations.
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

#ifndef __DARIC_EDDSA_H__
#define __DARIC_EDDSA_H__

#include "pke_curve.h"
#include "ll_api.h"
#include "hal_api.h"

cr_status eddsa_gen_pubkey(curve_type curve, uint32_t *privkey, uint32_t *pubkey);
cr_status eddsa_sign(curve_type curve, uint32_t *privkey, uint32_t *pubkey, uint32_t *msg, uint32_t msgLen, uint32_t *sig);
cr_status eddsa_verify(curve_type curve, uint32_t *pubkey, uint32_t *sig, uint32_t *msg, uint32_t msgLen);
#endif
