/**
 ******************************************************************************
 * @file    daric_hal_usb.h
 * @author  USB Team
 * @brief   Header file of USB HAL module.
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
#ifndef __DARIC_HAL_USB_H
#define __DARIC_HAL_USB_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include <stdint.h>

#define USB_SEND (0)
#define USB_RECV (1)
/*modified for two cdc acm port*/
#define UDC_EP_NUM         8 // 6
#define UDC_ERST_SIZE      1
#define UDC_TD_RING_SIZE   (1280)
#define UDC_EVENT_RING_NUM 1

#define UDC_MMIOBASE 0x50202400 /* usb controller base address */

#define UDC_ERSTSIZE                                                         \
    (0x100) /* allocate 0x100 bytes for event ring segment table, each table \
               0x40 bytes */
#define UDC_EVENTRINGSIZE                                                      \
    (0x800 * UDC_EVENT_RING_NUM) /* allocate 0x800 for one event ring, include \
                                    128 event TRBs , each TRB 16 bytes */
#define UDC_EPCXSIZE                                                         \
    (0x200) /* allocate 0x200 for ep context, include 30 ep context, each ep \
               context 16 bytes */
#define UDC_EP0_TRSIZE                                                                                                        \
    (0x400)                                                /* allocate 0x400 for EP0 transfer ring, include 64 transfer TRBs, \
                                                              each TRB 16 bytes */
#define UDC_EP_TRSIZE (UDC_TD_RING_SIZE * UDC_EP_NUM * 16) /* 1280(TRB Num) * 4(EP NUM) * 16(TRB bytes) */
#define UDC_EP0_REQBUFSIZE                                                    \
    (0x400) /* allocate 0x400 bytes for EP0 Buffer, Normally EP0 TRB transfer \
               length will not greater than 1K */
#define UDC_EP_REQBUFSIZE                                                       \
    (0x800 * UDC_EP_NUM) /* allocate 0x800 bytes for EP1/2/3/4 Buffer, Normally \
                            EP TRB transfer length will not greater than 2K */
#define UDC_IFRAM_SIZE (UDC_ERSTSIZE + UDC_EVENTRINGSIZE + UDC_EPCXSIZE + UDC_EP0_TRSIZE + UDC_EP_TRSIZE + UDC_EP0_REQBUFSIZE + UDC_EP_REQBUFSIZE) /* UDC ifram size */

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
#ifdef HAL_USB_UDC_IFRAM_BASE
#if ((HAL_USB_UDC_IFRAM_BASE % 0x100 == 0) && (HAL_USB_UDC_IFRAM_BASE >= 0x50000000) && (HAL_USB_UDC_IFRAM_BASE + UDC_IFRAM_SIZE <= 0x50040000))
#define USB_BASE_AREA (uint8_t *)HAL_USB_UDC_IFRAM_BASE
#else
#error "HAL_USB_UDC_IFRAM_BASE illegal"
#endif
#else
#define USB_BASE_AREA (uint8_t *)(0x50000000)
#endif
#else
extern char usb_base_area[UDC_IFRAM_SIZE];
#define USB_BASE_AREA (uint8_t *)(&usb_base_area)
#endif

/*modified for two cdc acm port*/
#define UDC_MEMBASE     USB_BASE_AREA /* usb memory base address */
#define UDC_UCCR_OFFSET 0
#define UDC_UICR_OFFSET 0x100

#define UDC_EVENT_RING_SIZE 128
#define UDC_ERST_ADDR       UDC_MEMBASE
#define UDC_EVENTRING_ADDR  UDC_ERST_ADDR + UDC_ERSTSIZE
#define UDC_EPCX_ADDR       (uint8_t *)(UDC_EVENTRING_ADDR + UDC_EVENTRINGSIZE)
#define UDC_EP0_TR_ADDR     (uint8_t *)(UDC_EPCX_ADDR + UDC_EPCXSIZE)
#define UDC_EP_TR_ADDR      (uint8_t *)(UDC_EP0_TR_ADDR + UDC_EP0_TRSIZE)
#define UDC_EP0_BUF_ADDR    (uint8_t *)(UDC_EP_TR_ADDR + UDC_EP_TRSIZE)
#define UDC_APP_BUFADDR     (uint8_t *)(UDC_EP0_BUF_ADDR + UDC_EP0_REQBUFSIZE)

/* usb speed cfg */
#define UDC_CFG0_MAXSPEED_MASK (0xfL << 0)
#define UDC_CFG0_MAXSPEED_FS   (0x1L << 0)
#define UDC_CFG0_MAXSPEED_HS   (0x3L << 0)

/* command register*/
#define UDC_CTRL_RUN       (1L << 0)
#define UDC_CTRL_STOP      (0L << 0)
#define UDC_CTRL_SWRST     (1L << 1)
#define UDC_CTRL_INT_EN    (1L << 2)
#define UDC_CTRL_SYSERR_EN (1L << 3) /*only AXI bus error*/
#define UDC_CTRL_EWE       (1L << 10)
#define UDC_CTRL_KP_CNCT   (1L << 11)

/* status register */
#define UDC_STATUS_DEV_CTRL_HALT (1L << 0)
#define UDC_STATUS_SYS_ERR       (1L << 2)
#define UDC_STATUS_EINT          (1L << 3)

/* event config bit */
#define UDC_CFG1_CSC_EVENT_EN            (0x1L << 0)
#define UDC_CFG1_PEC_EVENT_EN            (0x1L << 1)
#define UDC_CFG1_PPC_EVENT_EN            (0x1L << 3)
#define UDC_CFG1_PRC_EVENT_EN            (0x1L << 4)
#define UDC_CFG1_PLC_EVENT_EN            (0x1L << 5)
#define UDC_CFG1_CEC_EVENT_EN            (0x1L << 6)
#define UDC_CFG1_U3_ENTRY_EN             (0x1L << 8)
#define UDC_CFG1_L1_ENTRY_EN             (0x1L << 9)
#define UDC_CFG1_U3_RESUME_EN            (0x1L << 10)
#define UDC_CFG1_L1_RESUME_EN            (0x1L << 11)
#define UDC_CFG1_INACTIVE_PLC_EN         (0x1L << 12)
#define UDC_CFG1_U3_RESUME_NORESP_PLC_EN (0x1L << 13)
#define UDC_CFG1_U2_RESUME_NORESP_PLC_EN (0x1L << 14)

#define UDC_CFG1_SETUP_EVENT_EN (0x1L << 16)

/* erdp bits*/
#define UDC_ERDPLO_EHB        (1 << 3)
#define UDC_ERDPLO_ADDRLO(fv) ((fv) & 0xfffffff0)

/* iman bits*/
#define UDC_IMAN_INT_PEND (1L << 0)
#define UDC_IMAN_INT_EN   (1L << 1)

/* endpoint context data structure bit */
#define UDC_EP_CX_LOGICAL_EP_NUM_MASK   0x00000078
#define UDC_EP_CX_LOGICAL_EP_NUM_SHIFT  3
#define UDC_EP_CX_MULT_MASK             0x00000300
#define UDC_EP_CX_MULT_SHIFT            8
#define UDC_EP_CX_MAX_PSTREAMS_MASK     0x00007C00
#define UDC_EP_CX_MAX_PSTREAMS_SHIFT    10
#define UDC_EP_LINEAR_STRM_ARR_MASK     0x00008000
#define UDC_EP_LINEAR_STRM_ARR_SHIFT    15
#define UDC_EP_CX_INTERVAL_MASK         0x00FF0000
#define UDC_EP_CX_INTERVAL_SHIFT        16
#define UDC_EP_CX_EP_TYPE_MASK          0x00000038
#define UDC_EP_CX_EP_TYPE_SHIFT         3
#define UDC_EP_CX_MAX_BURST_SIZE_MASK   0x0000FF00
#define UDC_EP_CX_MAX_BURST_SIZE_SHIFT  8
#define UDC_EP_CX_MAX_PACKET_SIZE_MASK  0xFFFF0000
#define UDC_EP_CX_MAX_PACKET_SIZE_SHIFT 16
#define UDC_EP_CX_DEQ_CYC_STATE_MASK    0x00000001
#define UDC_EP_CX_DEQ_CYC_STATE_SHIFT   0
#define UDC_EP_CX_TR_DQPT_LO_MASK       0xFFFFFFF0
#define UDC_EP_CX_TR_DQPT_LO_SHIFT      4

/* Endpoint types */
#define USB_CONTROL_ENDPOINT     (0)
#define USB_ISOCHRONOUS_ENDPOINT (1)
#define USB_BULK_ENDPOINT        (2)
#define USB_INTERRUPT_ENDPOINT   (3)

/* USB power state */
#define USB_POWER_STATE_U3     3
#define USB_POWER_STATE_U2     2
#define USB_POWER_STATE_U0     0
#define USB_POWER_SATTE_RESUME 15

    /* device speed */
    typedef enum
    {
        USB_SPEED_UNKNOWN = 0,
        USB_SPEED_LOW,
        USB_SPEED_FULL,
        USB_SPEED_HIGH,
        USB_SPEED_WIRELESS,
        USB_SPEED_SUPER,
        USB_SPEED_SUPER_PLUS,
    } HAL_USB_SpeedTypeDef;

    typedef enum
    {
        USB_TRANSFER_BULK,
        USB_TRANSFER_INTERRUPT,
        USB_TRANSFER_ISOCHRONOUS
    } USB_TransferTypeDef;

    /* corigine usb 3.1 device core register macros */
    typedef volatile struct
    {
        uint32_t capability; /* 0x00: device capability */
        uint32_t resv0[3];
        uint32_t devconfig; /* 0x10: config0 */
        uint32_t evtconfig; /* 0x14: config1 */
        uint32_t resv1[2];
        uint32_t command;    /* 0x20: USB command */
        uint32_t status;     /* 0x24: USB status */
        uint32_t dcbaplo;    /* 0x28: device context base address (DCBA) Pointer Low */
        uint32_t dcbaphi;    /* 0x2C: device context base address (DCBA) Pointer High */
        uint32_t portsc;     /* 0x30: PORT Status and Control */
        uint32_t u3portpmsc; /* 0x34: USB3 Port PM Status and Control, not used */
        uint32_t u2portpmsc; /* 0x38: USB2 Port PM Status and Control */
        uint32_t u3portli;   /* 0x3C: USB3 Port Link Info, not used */
        uint32_t doorbell;   /* 0x40: Door Bell Register */
        uint32_t mfindex;    /* 0x44: Microframe Index */
        uint32_t ptmctr;     /* 0x48: PTM control */
        uint32_t ptmsts;     /* 0x4C: PTM status */
        uint32_t resv3[4];
        uint32_t ep_enable;  /* 0x60: ep enable */
        uint32_t ep_running; /* 0x64: ep running */
        uint32_t resv4[2];
        uint32_t cmd_param0;  /* 0x70: Command Parameter 0 */
        uint32_t cmd_param1;  /* 0x74: Command Parameter 1 */
        uint32_t cmd_control; /* 0x78: Command Control */
        uint32_t resv5[1];
        uint32_t odb_cap; /* 0x80: odb capability */
        uint32_t resv6[3];
        uint32_t odb_config[8]; /* Command Control 90-a0*/
        uint32_t debug0;        /* 0xB0: debug register */
    } HAL_USB_CtlRegTypeDef;

    typedef volatile struct
    {
        uint32_t iman;   /* 0x100：Interrupter Management */
        uint32_t imod;   /* 0x104: Interrupter Moderation */
        uint32_t erstsz; /* 0x108：Event Ring Segment Table Size */
        uint32_t resv0;
        uint32_t erstbalo; /* 0x110: Event Ring Segment Table Base Address */
        uint32_t erstbahi; /* 0x114: Event Ring Segment Table Base Address */
        uint32_t erdplo;   /* 0x118: Event Ring Dequeue Pointer */
        uint32_t erdphi;   /* 0x11C: Event Ring Dequeue Pointer */
    } HAL_USB_IntRegTypeDef;

    /* buffer info data structure */
    typedef struct
    {
        void    *vaddr;
        uint64_t dma;
        uint32_t len;
    } HAL_USB_BufInfoDef;

    /* trb data structure */
    typedef struct
    {
        uint32_t dw0;
        uint32_t dw1;
        uint32_t dw2;
        uint32_t dw3;
    } HAL_USB_EvtTrbDef, HAL_USB_TransTrbDef;

    /* udc endpoint context data structure */
    typedef struct
    {
        uint32_t dw0;
        uint32_t dw1;
        uint32_t dw2;
        uint32_t dw3;
    } HAL_USB_EpCtxDef;

    /* udc endpoint data structure */
    typedef struct
    {
        uint8_t  ep_num;    /* Endpoint number */
        uint8_t  direction; /* Endpoint direction */
        uint8_t  type;
        uint16_t max_packet_size;

        HAL_USB_BufInfoDef   tran_ring_info;
        HAL_USB_TransTrbDef *first_trb;
        HAL_USB_TransTrbDef *last_trb;
        HAL_USB_TransTrbDef *enq_pt;
        HAL_USB_TransTrbDef *deq_pt;

        uint8_t     pcs;
        int         tran_ring_full;
        struct udc *udc;
        int         ep_state;
        unsigned    wedge : 1;
    } HAL_USB_EpTypeDef;

    /* udc event data structure */
    typedef struct
    {
        HAL_USB_BufInfoDef event_ring;
        HAL_USB_EvtTrbDef *evt_dq_pt;
        HAL_USB_EvtTrbDef *evt_seg0_last_trb;
        uint8_t            ccs;
    } HAL_USB_EvtTypeDef;

    /* USB device controller power management data structure */
    typedef struct
    {
        uint16_t u2_pel_value;
        uint16_t u2_sel_value;
        uint8_t  u1_pel_value;
        uint8_t  u1_sel_value;
    } HAL_USB_SelValueTypeDef;

    /* call back for epx complete function */
    typedef void (*HAL_USB_EpxCmplCB)(int endpoint_id, uint8_t direction, uint8_t *buf, uint32_t len, uint8_t error);
    /* call back for ep0 complete function */
    typedef void (*HAL_USB_Ep0CmplCB)(uint8_t *buf, uint32_t len, uint8_t error);
    /* call back for setup function */
    typedef void (*HAL_USB_SetupCB)(uint8_t *setup);
    /* callback for power state */
    typedef void (*HAL_USB_PowerState)(uint8_t state);

    /* USB device controller data structure */
    typedef struct
    {
        HAL_USB_CtlRegTypeDef  *ctl_reg;
        HAL_USB_IntRegTypeDef  *int_reg;
        HAL_USB_EvtTypeDef      udc_event[UDC_EVENT_RING_NUM];
        HAL_USB_EpCtxDef       *ep_ctx;
        HAL_USB_EpTypeDef       ep[UDC_EP_NUM + 2];
        HAL_USB_SpeedTypeDef    max_speed;
        HAL_USB_SelValueTypeDef sel_value;
        uint32_t                feature_u1_enable : 1;
        uint32_t                feature_u2_enable : 1;
        uint8_t                 setup_tag;
        uint8_t                 setup[8];

        HAL_USB_Ep0CmplCB  ep0_compl_func_ptr;
        HAL_USB_SetupCB    handle_setup;
        HAL_USB_PowerState power_state_notify;
        HAL_USB_EpxCmplCB  cmpl_func_ptr[30];
    } HAL_USB_CtxTypeDef;

    /**
     * @brief Initializes the USB controller.
     * @param USB context.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function performs the necessary hardware initialization of the USB
     * controller, setting up the controller to handle USB events and data
     * transfers.
     */
    HAL_StatusTypeDef HAL_USB_Initialize(HAL_USB_CtxTypeDef *ctx);

    /**
     * @brief Uninitializes the USB controller.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function releases resources and performs cleanup for the USB controller,
     * restoring it to an uninitialized state.
     */
    HAL_StatusTypeDef HAL_USB_Uninitialize(void);

    /**
     * @brief  Sets the USB device address.
     * @param  address The address to set (1-127).
     * @param  intr_target which interrupter target will be use for completion event
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function assigns a USB device address as part of the enumeration
     * process. The address is assigned by the host and is used for all future
     * communications between the host and the device.
     */
    HAL_StatusTypeDef HAL_USB_SetAddress(uint8_t address, int intr_taraget);

    /**
     * @brief  Enables USB controller interrupts.
     * @param  None.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function enables the interrupt system for the USB controller, allowing
     * it to respond to events such as data transfers, resets, and suspend/resume
     * conditions.
     */
    HAL_StatusTypeDef HAL_USB_EnableInt(void);

    /**
     * @brief  Disables USB controller interrupts.
     * @param  None.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function disables the interrupt system for the USB controller,
     * preventing it from responding to USB events. This may be used in low-power
     * states or during critical operations where interrupts must be masked.
     */
    HAL_StatusTypeDef HAL_USB_DisableInt(void);

    /**
     * @brief  Sets or clears a stall condition on a specific endpoint.
     * @param  endpoint_id ID of the endpoint.
     * @param  stall 1 to set stall, 0 to clear stall.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function stalls or clears the stall on a specific endpoint. Stalling an
     * endpoint is typically used to indicate an error or to signify that the
     * endpoint is not ready to transfer data. Clearing the stall allows the
     * endpoint to resume normal operation.
     */
    HAL_StatusTypeDef HAL_USB_Ep_SetStall(int endpoint_id, int stall);

    /**
     * @brief  Clears a stall condition on a specific endpoint.
     * @param  endpoint_id ID of the endpoint.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function clears the stall condition on a specific endpoint, allowing it
     * to resume normal operation. It is commonly used to recover from error
     * conditions.
     */
    HAL_StatusTypeDef HAL_USB_Ep_ClearStall(int endpoint_id);

    /**
     * @brief  Registers a completion callback for an endpoint.
     * @param  endpoint_id ID of the endpoint.
     * @param  direction of endpoint
     * @param  callback Pointer to the callback function.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function registers a callback function that will be called upon the
     * completion of a transfer on the specified endpoint. The callback provides a
     * mechanism for the application to respond to completed transfers, errors, or
     * other endpoint events.
     */
    HAL_StatusTypeDef HAL_USB_Ep_CompRegister(int endpoint_id, uint8_t direction, HAL_USB_EpxCmplCB cb);

    /**
     * @brief  Sets power management mode for the USB controller.
     * @param  mode Power management mode to set (e.g., USB_PM_ACTIVE).
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function configures the power management mode of the USB controller.
     * Different modes may include active, suspend, or sleep states, depending
     * on the desired power consumption and operational requirements.
     */
    HAL_StatusTypeDef HAL_USB_SetPm(int mode);

    /**
     * @brief  Clears power management settings, returning the controller to the
     * default state.
     * @param  None.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function clears any power management settings that have been applied,
     * returning the USB controller to its default power state. It is typically used
     * to reset the power management configuration after an operation or event.
     */
    HAL_StatusTypeDef HAL_USB_ClearPm(void);

    /**
     * @brief  Enables a specific endpoint, allowing data transfers.
     * @param  endpoint_id ID of the endpoint.
     * @param  type The type of transfer (control, bulk, interrupt, isochronous).
     * @param  direction The direction of data flow (IN or OUT).
     * @param  max packet size of ep
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function enables the specified endpoint, making it ready for data
     * transfers. Enabling an endpoint is typically done after it has been
     * configured and is ready to handle data transactions.
     */
    HAL_StatusTypeDef HAL_USB_EpxEnable(int endpoint_id, int type, int direction, uint16_t max_packet_size);

    /**
     * @brief  Disables a specific endpoint, preventing further data transfers.
     * @param  endpoint_id ID of the endpoint.
     * @param  direction The direction of data flow (IN or OUT).
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function disables the specified endpoint, preventing any further data
     * transfers. Disabling an endpoint is typically done when it is no longer
     * needed or when an error condition has been detected.
     */
    HAL_StatusTypeDef HAL_USB_EpxDisable(int endpoint_id, int direction);

    /**
     * @brief  Sends data on the control endpoint (EP0).
     * @param  buf Pointer to the data buffer.
     * @param  length Length of the data to send.
     * @param  intr_target is interrupter target index that will be used for
     * completion event
     * @retval int The number of bytes actually sent, or a negative value on error.
     *
     * This function sends data on the control endpoint (EP0). It is typically used
     * to respond to standard device requests from the host, such as sending
     * descriptors or status information.
     */
    HAL_StatusTypeDef HAL_USB_Ep0Send(void *buf, uint32_t length, int intr_target);

    /**
     * @brief  Receives data on the control endpoint (EP0).
     * @param  buf Pointer to the data buffer.
     * @param  length Length of the data buffer.
     * @param  intr_target is interrupter target index that will be used for
     * completion event
     * @retval int The number of bytes actually received, or a negative value on
     * error.
     *
     * This function receives data on the control endpoint (EP0). It is typically
     * used to handle incoming control transfers from the host, such as receiving
     * requests or configuration commands.
     */
    HAL_StatusTypeDef HAL_USB_Ep0Receive(void *buf, uint32_t length, int intr_target);

    /**
     * @brief  Sends data through a specific endpoint.
     * @param  endpoint_id ID of the endpoint.
     * @param  buf Pointer to the buffer containing data to be sent.
     * @param  length Length of the data to be sent.
     * @param  type transfer type of ep.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function sends data through the specified endpoint, which can be used
     * for bulk, interrupt, or isochronous transfers.
     */
    HAL_StatusTypeDef HAL_USB_EpxSend(int endpoint_id, void *buf, uint32_t length, USB_TransferTypeDef type);

    /**
     * @brief  Receives data from a specific endpoint.
     * @param  endpoint_id ID of the endpoint.
     * @param  buf Pointer to the buffer where received data will be stored.
     * @param  length Length of the data to receive.
     * @param  type transfer type of ep.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function receives data from the specified endpoint, which can be used
     * for bulk, interrupt, or isochronous transfers.
     */
    HAL_StatusTypeDef HAL_USB_EpxReceive(int endpoint_id, void *buf, uint32_t length, USB_TransferTypeDef type);

    /**
     * @brief  Halts a specific endpoint.
     * @param  endpoint_id ID of the endpoint.
     * @param  direction of the endpoint.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function halts the specified endpoint, which is a state similar to stall
     * but may be used in different contexts or implementations.
     */
    HAL_StatusTypeDef HAL_USB_EpxHalt(int endpoint_id, int direction);

    /**
     * @brief  Unhalts a specific endpoint.
     * @param  endpoint_id ID of the endpoint.
     * @param  direction of the endpoint.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function unhalts the specified endpoint, resuming its normal operation
     * after a halt condition has been applied.
     */
    HAL_StatusTypeDef HAL_USB_EpxUnhalt(int endpoint_id, int direction);

    /**
     * @brief  Resets the USB controller and its connected devices.
     * @retval int 0 if successful, negative value if an error occurs.
     *
     * This function performs a USB reset, which typically includes resetting
     * the USB controller and reinitializing connected USB devices. This is
     * commonly used during device initialization or error recovery.
     */
    HAL_StatusTypeDef HAL_USB_Reset(void);

    /**
     * determine if the specific endpoint halt.
     * @param    endpoint_id is endpoint number.
     * @param    direction is endpoint direction
     * @return   1 halted or 0 unhalted
     * @note     None.
     *
     * This function is using for check the ep if it it halted.
     */
    uint8_t HAL_USB_EpIsHalted(int endpoint_id, int direction);

    /**
     * Configure the USB port's power management settings.
     *
     * This function sets specific timing parameters for the U1 and U2 power
     * management states, which are used to reduce power consumption during idle
     * periods. The parameters specify the exit latency from these low-power states
     * for both the system and the device.
     *
     * - U1 and U2 are USB link power management states that allow selective
     *   suspend/resume operations for greater power efficiency.
     *
     * @param   u1_sel  The U1 System Exit Latency, in microseconds (us). This is
     *                  the time required for the system to exit the U1 low-power
     * state.
     *
     * @param   u1_pel  The U1 Device-to-Host Exit Latency, in microseconds (us).
     *                  This indicates the time for the device to exit U1 state and
     *                  signal the host.
     *
     * @param   u2_sel  The U2 System Exit Latency, in microseconds (us). This
     * represents the time it takes the system to exit the U2 low-power state.
     *
     * @param   u2_pel  The U2 Device-to-Host Exit Latency, in microseconds (us).
     * This is the time for the device to exit the U2 state and notify the host.
     *
     * @return  None.
     *
     * @note    This function is typically used in USB 3.0 or later to manage
     *          power-saving modes. The exit latencies should be configured based on
     *          system and device capabilities to ensure proper timing and
     * synchronization.
     */
    void HAL_USB_Set_Sel(uint8_t u1_sel, uint8_t u1_pel, uint16_t u2_sel, uint16_t u2_pel);

    /**
     * Reset the sequence number of a USB endpoint in the protocol layer.
     *
     * This function resets the data sequence number used for a specific USB
     * endpoint, effectively reinitializing its data transmission or reception
     * state. This is typically done when re-establishing communication with the
     * host or recovering from a stall condition.
     *
     * @param   endpoint_id  The endpoint number (1-15) whose sequence number needs
     * to be reset. Endpoint 0 is reserved for control transfers and is not handled
     * here.
     *
     * @param   direction    The data direction of the endpoint:
     *                       - USB_SEND (IN direction, device to host)
     *                       - USB_RECV (OUT direction, host to device)
     *
     * @return  None.
     *
     * @note    This function should be used when resetting an endpoint's
     * communication state, typically after an error or reinitialization. It ensures
     * the protocol layer sequence numbering is synchronized with the host.
     */
    void HAL_USB_ResetSeqNumber(int endpoint_id, int direction);

    /**
     * Retrieve the current operating speed of the USB device.
     *
     * This function returns the current speed at which the USB device is
     * communicating with the host. The speed can vary depending on the USB standard
     * in use (e.g., USB 2.0, USB 3.0) and the negotiation between the host and
     * device during enumeration.
     *
     * The possible speeds are:
     * - USB_SPEED_LOW:    Low-Speed (1.5 Mbps)
     * - USB_SPEED_FULL:   Full-Speed (12 Mbps)
     * - USB_SPEED_HIGH:   High-Speed (480 Mbps)
     *
     * @return  The current USB device speed, represented as a
     * `HAL_USB_SpeedTypeDef` enum value.
     *
     * @note    This function is useful for determining the actual speed of
     * communication, especially in scenarios where performance tuning or
     * troubleshooting is required.
     */
    HAL_USB_SpeedTypeDef HAL_USB_GetSpeed(void);

    /**
     * Set the USB test mode.
     *
     * This function configures the USB controller to enter a specific
     * test mode as defined by the USB 2.0/3.x specification. Test modes
     * are typically used for compliance testing, allowing the device
     * to exhibit specific signal patterns or behavior for signal integrity
     * and protocol validation.
     *
     * Supported test modes might include:
     * - Test_J: Forces the device to output a continuous 'J' state on the USB bus.
     * - Test_K: Forces the device to output a continuous 'K' state on the USB bus.
     * - Test_SE0_NAK: Forces the device to continuously issue SE0 (Single Ended
     * Zero) and respond with NAK.
     * - Test_Packet: Sends a predefined packet for signal quality testing.
     *
     * @param   tm  Test mode identifier, defined by the USB specification.
     *              Typical values include:
     *              - 0x01: Test_J
     *              - 0x02: Test_K
     *              - 0x03: Test_SE0_NAK
     *              - 0x04: Test_Packet
     *
     * @return  None
     *
     * @note    This function should only be used in a test environment and
     *          not during normal device operation. Entering test modes
     *          could disrupt the regular functioning of the USB device,
     *          so ensure it is used appropriately in compliance testing setups.
     */
    void HAL_USB_SetTestMode(int tm);

    /**
     * @brief Handles the USB interrupt request.
     *
     * This function is called when a USB interrupt occurs. It processes
     * the USB events, including handling control transfers, data transfers,
     * and any errors that may arise during USB communication. The function
     * should identify the source of the interrupt and execute the necessary
     * handlers for each USB endpoint as needed. Proper handling ensures
     * smooth data flow and error management within the USB subsystem.
     *
     * @note Ensure that the USB peripheral is correctly configured
     *       before invoking this function.
     */
    void HAL_USB_IRQ_Handle(void);

    /**
     * @brief Start the USB peripheral.
     *
     * This function configures the USB hardware settings, including enabling
     * the necessary clocks and initializing the USB stack. It prepares the
     * USB device for operation and ensures that the device is ready to respond
     * to USB bus events.
     *
     * @note This function should be called before any USB operations can be
     *       performed, such as data transmission or reception. It must be called
     *       only once during the device's lifecycle.
     *
     * @retval None
     */
    void HAL_USB_Start(void);

    /**
     * @brief Stops the USB subsystem and releases any resources.
     *
     * This function disables the USB hardware interface and stops all ongoing USB
     * operations. It ensures that no data transfers are occurring and that the
     * USB controller is safely shut down. Any allocated resources, such as buffers
     * and handles, are freed, preventing memory leaks.
     *
     * @note This function should be called when the USB subsystem is no longer
     *       needed, such as during system shutdown or when the USB interface
     *       is being reinitialized.
     *
     * @retval None
     */
    void HAL_USB_Stop(void);

#ifdef __cplusplus
}
#endif

#endif /* __DARIC_HAL_USB_H */
