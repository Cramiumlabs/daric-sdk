/**
 ******************************************************************************
 * @file    daric_errno.h
 * @author  PERIPHERIAL BSP Team
 * @brief   This file contains the error number used in bsp
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
#ifndef DARIC_NUCLEO_ERRNO_H
#define DARIC_NUCLEO_ERRNO_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Common Error codes */
#define BSP_ERROR_NONE                  0
#define BSP_ERROR_NO_INIT               -1
#define BSP_ERROR_WRONG_PARAM           -2
#define BSP_ERROR_BUSY                  -3
#define BSP_ERROR_PERIPH_FAILURE        -4
#define BSP_ERROR_COMPONENT_FAILURE     -5
#define BSP_ERROR_UNKNOWN_FAILURE       -6
#define BSP_ERROR_UNKNOWN_COMPONENT     -7
#define BSP_ERROR_BUS_FAILURE           -8
#define BSP_ERROR_CLOCK_FAILURE         -9
#define BSP_ERROR_MSP_FAILURE           -10
#define BSP_ERROR_NOT_SUPPORTED_FEATURE -11

#ifdef __cplusplus
}
#endif

#endif /* DARIC_NUCLEO_ERRNO_H */
