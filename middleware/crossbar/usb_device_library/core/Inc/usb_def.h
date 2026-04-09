/**
 ******************************************************************************
 * @file    usb_def.h
 * @author  USB Team
 * @brief   Header file of USB Core module.
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
#ifndef __USB_DEF_H
#define __USB_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * USB recipients, the third of three bRequestType fields
 */
#define USB_RECIP_MASK      0x1f
#define USB_RECIP_DEVICE    0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT  0x02
#define USB_RECIP_OTHER     0x03

#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_DESCRIPTOR    0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE     0x0A
#define USB_REQ_SET_INTERFACE     0x0B
#define USB_REQ_SYNCH_FRAME       0x0C
#define USB_REQ_SET_SEL           0x30
#define USB_REQ_SET_ISOCH_DELAY   0x31

#define USB_DT_DEVICE             0x01
#define USB_DT_CONFIG             0x02
#define USB_DT_STRING             0x03
#define USB_DT_INTERFACE          0x04
#define USB_DT_ENDPOINT           0x05
#define USB_DT_DEVICE_QUALIFIER   0x06
#define USB_DT_OTHER_SPEED_CONFIG 0x07
#define USB_DT_INTERFACE_POWER    0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG                   0x09
#define USB_DT_DEVICE_CAPABILITY     0x10
#define USB_DT_DEBUG                 0x0a
#define USB_DT_INTERFACE_ASSOCIATION 0x0b
#define USB_DT_BOS                   0x0f
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE 0x24

#define USB_DT_BOS_SIZE 5

/* Endpoint Transfer Types */
#define USB_EP_XFER_CONTROL   0x00
#define USB_EP_XFER_ISOC      0x01
#define USB_EP_XFER_BULK      0x02
#define USB_EP_XFER_INTERRUPT 0x03

#define USB_DT_DEVICE_SIZE    18
#define USB_DT_ENDPOINT_SIZE  7
#define USB_DT_INTERFACE_SIZE 9
#define USB_DT_CONFIG_SIZE    9

    /* USB Descriptor Header */
    struct usb_descriptor_header
    {
        uint8_t bLength;
        uint8_t bDescriptorType;
    } __attribute__((packed));

    /* USB Device Descriptor */
    struct usb_device_descriptor
    {
        uint8_t  bLength;
        uint8_t  bDescriptorType;
        uint16_t bcdUSB;
        uint8_t  bDeviceClass;
        uint8_t  bDeviceSubClass;
        uint8_t  bDeviceProtocol;
        uint8_t  bMaxPacketSize0;
        uint16_t idVendor;
        uint16_t idProduct;
        uint16_t bcdDevice;
        uint8_t  iManufacturer;
        uint8_t  iProduct;
        uint8_t  iSerialNumber;
        uint8_t  bNumConfigurations;
    } __attribute__((packed));

    /* USB Configuration Descriptor */
    struct usb_configuration_descriptor
    {
        uint8_t  bLength;
        uint8_t  bDescriptorType;
        uint16_t wTotalLength;
        uint8_t  bNumInterfaces;
        uint8_t  bConfigurationValue;
        uint8_t  iConfiguration;
        uint8_t  bmAttributes;
        uint8_t  bMaxPower;
    } __attribute__((packed));

    /* USB Interface Descriptor */
    struct usb_interface_descriptor
    {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bInterfaceNumber;
        uint8_t bAlternateSetting;
        uint8_t bNumEndpoints;
        uint8_t bInterfaceClass;
        uint8_t bInterfaceSubClass;
        uint8_t bInterfaceProtocol;
        uint8_t iInterface;
    } __attribute__((packed));

    /* USB Interface Association Descriptor（IAD）*/
    struct usb_interface_association_descriptor
    {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bFirstInterface;
        uint8_t bInterfaceCount;
        uint8_t bFunctionClass;
        uint8_t bFunctionSubClass;
        uint8_t bFunctionProtocol;
        uint8_t iFunction;
    } __attribute__((packed));

    /* USB_DT_BOS:  group of device-level capabilities */
    struct usb_bos_descriptor
    {
        uint8_t bLength;
        uint8_t bDescriptorType;

        uint16_t wTotalLength;
        uint8_t  bNumDeviceCaps;
    } __attribute__((packed));

    /* USB_DT_DEVICE_QUALIFIER: Device Qualifier descriptor */
    struct usb_qualifier_descriptor
    {
        uint8_t bLength;
        uint8_t bDescriptorType;

        uint16_t bcdUSB;
        uint8_t  bDeviceClass;
        uint8_t  bDeviceSubClass;
        uint8_t  bDeviceProtocol;
        uint8_t  bMaxPacketSize0;
        uint8_t  bNumConfigurations;
        uint8_t  bRESERVED;
    } __attribute__((packed));

    /* USB Endpoint Descriptor */
    struct usb_endpoint_descriptor
    {
        uint8_t  bLength;
        uint8_t  bDescriptorType;
        uint8_t  bEndpointAddress;
        uint8_t  bmAttributes;
        uint16_t wMaxPacketSize;
        uint8_t  bInterval;
    } __attribute__((packed));

    /* USB Control Request */
    struct usb_control_request
    {
        uint8_t  bmRequestType;
        uint8_t  bRequest;
        uint16_t wValue;
        uint16_t wIndex;
        uint16_t wLength;
    } __attribute__((packed));

    /* USB_DT_STRING: String descriptor */
    struct usb_string_descriptor
    {
        uint8_t bLength;
        uint8_t bDescriptorType;

        uint16_t wData[1];
    } __attribute__((packed));

    struct usb_interface_config
    {
        int                              num_endpoints;
        struct usb_endpoint_descriptor  *endpoints;
        struct usb_interface_descriptor *interface;
    };

    struct usb_desc_lang
    {
        struct usb_descriptor_header header;
        uint16_t                     wLANGID;
    } __attribute__((packed));

    struct usb_desc_str
    {
        struct usb_descriptor_header header;
        uint16_t                     unicode_string[64];
    } __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif /* __USB_DEF_H */