/**
 ******************************************************************************
 * @file    usb_cdc.h
 * @author  USB Team
 * @brief   Header file of USB CDC module.
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
#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>

#include "usb_def.h"

/* communications device class code */
#define USB_CLASS_CDC 0x02U

/* CDC subclass code */
#define USB_CDC_SUBCLASS_DLCM 0x01U
#define USB_CDC_SUBCLASS_ACM  0x02U

/* communications interface class control protocol codes */
#define USB_CDC_PROTOCOL_NONE 0x00U
#define USB_CDC_PROTOCOL_AT   0x01U

#define USB_CLASS_DATA 0x0AU

    struct usb_header_func_descriptor
    {
        struct usb_descriptor_header header;
        uint8_t                      bDescriptorSubtype;
        uint16_t                     bcdCDC;
    } __attribute__((packed));

    struct usb_call_management_func_descriptor
    {
        struct usb_descriptor_header header;
        uint8_t                      bDescriptorSubtype;
        uint8_t                      bmCapabilities;
        uint8_t                      bDataInterface;
    } __attribute__((packed));

    struct usb_acm_func_descriptor
    {
        struct usb_descriptor_header header;
        uint8_t                      bDescriptorSubtype;
        uint8_t                      bmCapabilities;
    } __attribute__((packed));

    struct usb_union_func_descriptor
    {
        struct usb_descriptor_header header;
        uint8_t                      bDescriptorSubtype;
        uint8_t                      bMasterInterface;
        uint8_t                      bSlaveInterface0;
    } __attribute__((packed));

    struct usb_cdc_config_descriptor
    {
        struct usb_configuration_descriptor         config;
        struct usb_interface_association_descriptor itf_ass;
        struct usb_interface_descriptor             cmd_itf;
        struct usb_header_func_descriptor           cdc_header;
        struct usb_call_management_func_descriptor  cdc_call_managment;
        struct usb_acm_func_descriptor              cdc_acm;
        struct usb_union_func_descriptor            cdc_union;
        struct usb_endpoint_descriptor              cdc_cmd_endpoint;
        struct usb_interface_descriptor             cdc_data_interface;
        struct usb_endpoint_descriptor              cdc_out_endpoint;
        struct usb_endpoint_descriptor              cdc_in_endpoint;
    } __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif /* __USB_CDC_H */