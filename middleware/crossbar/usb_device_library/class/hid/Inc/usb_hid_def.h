/**
 ******************************************************************************
 * @file    usb_hid_def.h
 * @author  USB Team
 * @brief   Header file of USB HID module.
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
#ifndef __USB_HID_DEF_H
#define __USB_HID_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#define USB_HID_CLASS 0x03U

#define USB_DT_HID    0x21U
#define USB_DT_REPORT 0x22U

/* HID subclass code */
#define USB_HID_SUBCLASS_BOOT_ITF 0x01U

/* HID protocol codes */
#define USB_HID_PROTOCOL_KEYBOARD 0x01U
#define USB_HID_PROTOCOL_MOUSE    0x02U

#define GET_REPORT   0x01U
#define GET_IDLE     0x02U
#define GET_PROTOCOL 0x03U
#define SET_REPORT   0x09U
#define SET_IDLE     0x0AU
#define SET_PROTOCOL 0x0BU

    struct usb_hid_descriptor
    {
        struct usb_descriptor_header header;

        uint16_t bcdHID;
        uint8_t  bCountryCode;
        uint8_t  bNumDescriptors;
        uint8_t  bDescriptorType;
        uint16_t wDescriptorLength;
    } __attribute__((packed));

    struct usb_hid_config_set_descriptor
    {
        struct usb_configuration_descriptor config;
        struct usb_interface_descriptor     hid_itf;
        struct usb_hid_descriptor           hid_vendor;
        struct usb_endpoint_descriptor      hid_epin;
        struct usb_endpoint_descriptor      hid_epout;
    } __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif /* __HID_DEF_H */