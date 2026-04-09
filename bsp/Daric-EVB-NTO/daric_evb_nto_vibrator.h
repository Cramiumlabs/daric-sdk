/**
 ******************************************************************************
 * @file    daric_evb_nto_vibrator.h
 * @author  PERIPHERIAL BSP Team
 * @brief   This file contains the common defines and functions prototypes for
 *          the daric_evb_nto_vibrator.c driver.
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
#ifndef DARIC_EVB_NTO_VIB_H
#define DARIC_EVB_NTO_VIB_H

#include <stdint.h>

/**
 * @brief Initializes the vibrator hardware.
 * @retval BSP status code.
 */
int BSP_Vibrator_Init();

/**
 * @brief Activates the vibrator for a short duration.
 * @retval BSP status code.
 */
int BSP_Vibrator_Short();

/**
 * @brief Activates the vibrator for a specified duration.
 * @param ms Duration in milliseconds for which the vibrator should remain active.
 * @retval BSP status code.
 */
int BSP_Vibrator_Long(uint32_t ms);

/**
 * @brief Plays a predefined RTP example.
 * @retval BSP status code.
 */
int BSP_Vibrator_Rtp_Play(void);

#endif /* DARIC_EVB_NTO_VIB_H */
