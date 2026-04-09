/**
******************************************************************************
* @file    daric_evb_nto_usb.h
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the daric_evb_nto_usb.h driver.
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

#ifndef DARIC_EVB_NTO_USB_H
#define DARIC_EVB_NTO_USB_H

#include <stdint.h>

/**
 * @brief BSP USB Status Type enumeration.
 */
typedef enum
{
    BSP_USB_INSERT, /*!< USB is inserted */
    BSP_USB_REMOVE, /*!< USB is removed */
} BSP_USB_STATUS;

/**
 * @brief The callback type to notify USB status. The parameter wil be BSP_USB_STATUS.
 */
typedef void (*USB_DETECT_CB)(int16_t status);

int16_t BSP_USB_Detect_Register(USB_DETECT_CB cb);
int16_t BSP_USB_Detect_UnRegister();
int16_t BSP_USB_GetStatus();

#endif
