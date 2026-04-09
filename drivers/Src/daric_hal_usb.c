/**
 ******************************************************************************
 * @file    daric_hal_usb.c
 * @author  USB Team
 * @brief   USB HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the General Purpose Input/Output (USB)
 *          peripheral:
 *           + Initialization and de-initialization functions
 *           + IO operation functions
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

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_usb.h"
#include "daric_hal_nvic.h"
#include "daric_hal.h"
#include <stdio.h>
#include <string.h>

/* USB IRQ Priority define */
#ifdef HAL_USB_IRQ_PRIO
#define USB_IRQ_PRIO HAL_USB_IRQ_PRIO
#else
#define USB_IRQ_PRIO 0
#endif

#ifdef HAL_USB_IRQ_SUB_PRIO
#define USB_IRQ_SUB_PRIO HAL_USB_IRQ_SUB_PRIO
#else
#define USB_IRQ_SUB_PRIO 0
#endif

/*modified for two cdc acm port*/
#ifndef HAL_IFRAMMGR_MODULE_ENABLED
char usb_base_area[UDC_IFRAM_SIZE] __attribute__((aligned(0x100))) __attribute__((section("ifram")));
#endif

#define UDC_MAX_BURST    (15)
#define MAX_TRB_XFER_LEN (64 * 1024)
#define UDC_ISO_INTERVAL 3

#define UDC_EVENT_RING_NUM   1
#define UDC_EP0_TD_RING_SIZE 16

/* usb transfer flags */
#define XFER_NO_INTR   (1 << 0) /* no interrupt after this transfer */
#define XFER_NO_DB     (1 << 1) /* will not knock doorbell */
#define XFER_SET_CHAIN (1 << 2) /* set chain bit at the last trb in this transfer */
#define XFER_AZP       (1 << 4) /* append zero length packet after a max packet */

#define USB_MAX_PACKET_SIZE_F (64)
#define USB_MAX_PACKET_SIZE_H (64)

#define TRB_TRANSFER_LEN_MASK           0x0001FFFF
#define TRB_TRANSFER_LEN_SHIFT          0
#define TRB_TD_SIZE_MASK                0x003E0000
#define TRB_TD_SIZE_SHIFT               17
#define TRB_INTR_TARGET_MASK            0xFFC00000
#define TRB_INTR_TARGET_SHIFT           22
#define TRB_CYCLE_BIT_MASK              0x00000001
#define TRB_CYCLE_BIT_SHIFT             0
#define TRB_LINK_TOGGLE_CYCLE_MASK      0x00000002
#define TRB_LINK_TOGGLE_CYCLE_SHIFT     1
#define TRB_INTR_ON_SHORT_PKT_MASK      0x00000004
#define TRB_INTR_ON_SHORT_PKT_SHIFT     2
#define TRB_NO_SNOOP_MASK               0x00000008
#define TRB_NO_SNOOP_SHIFT              3
#define TRB_CHAIN_BIT_MASK              0x00000010
#define TRB_CHAIN_BIT_SHIFT             4
#define TRB_INTR_ON_COMPLETION_MASK     0x00000020
#define TRB_INTR_ON_COMPLETION_SHIFT    5
#define TRB_APPEND_ZLP_MASK             0x00000080
#define TRB_APPEND_ZLP_SHIFT            7
#define TRB_BLOCK_EVENT_INT_MASK        0x00000200
#define TRB_BLOCK_EVENT_INT_SHIFT       9
#define TRB_TYPE_MASK                   0x0000FC00
#define TRB_TYPE_SHIFT                  10
#define TRB_DIR_MASK                    0x00010000
#define TRB_DIR_SHIFT                   16
#define TRB_SETUP_TAG_MASK              0x00060000
#define TRB_SETUP_TAG_SHIFT             17
#define STATUS_STAGE_TRB_STALL_MASK     0x00080000
#define STATUS_STAGE_TRB_STALL_SHIFT    19
#define STATUS_STAGE_TRB_SET_ADDR_MASK  0x00100000
#define STATUS_STAGE_TRB_SET_ADDR_SHIFT 20
#define ISOC_TRB_FRAME_ID_MASK          0x7FF00000
#define ISOC_TRB_FRAME_ID_SHIFT         20
#define ISOC_TRB_SIA_MASK               0x80000000
#define ISOC_TRB_SIA_SHIFT              31
#define EVE_TRB_TRAN_LEN_MASK           0x0001FFFF
#define EVE_TRB_TRAN_LEN_SHIFT          0
#define EVE_TRB_COMPL_CODE_MASK         0xFF000000
#define EVE_TRB_COMPL_CODE_SHIFT        24
#define EVE_TRB_CYCLE_BIT_MASK          0x00000001
#define EVE_TRB_CYCLE_BIT_SHIFT         0
#define EVE_TRB_TYPE_MASK               0x0000FC00
#define EVE_TRB_TYPE_SHIFT              10
#define EVE_TRB_ENDPOINT_ID_MASK        0x001F0000
#define EVE_TRB_ENDPOINT_ID_SHIFT       16
#define EVE_TRB_SETUP_TAG_MASK          0x00600000
#define EVE_TRB_SETUP_TAG_SHIFT         21

/*command params*/
/*command0 init ep0*/
#define CMD0_0_DQPTRLO_SHIFT (4)
#define CMD0_0_DQPTRLO_MASK  (0x0fffffff << CMD0_0_DQPTRLO_SHIFT)

#define CMD0_0_DCS_SHIFT (0)
#define CMD0_0_DCS_MASK  (0x1 << CMD0_0_DCS_SHIFT)
#define CMD0_0_DCS(fv)   (MAKEF_VAR(CMD0_0_DCS, (fv)))

/*command1 update ep0 */
#define CMD1_0_MPS_SHIFT (16)
#define CMD1_0_MPS_MASK  (0xffff << CMD1_0_MPS_SHIFT)
#define CMD1_0_MPS(fv)   (MAKEF_VAR(CMD1_0_MPS, (fv)))

/*command2 set addr */
#define CMD2_0_DEV_ADDR_SHIFT (0)
#define CMD2_0_DEV_ADDR_MASK  (0xff << CMD2_0_DEV_ADDR_SHIFT)
#define CMD2_0_DEV_ADDR(fv)   (MAKEF_VAR(CMD2_0_DEV_ADDR, (fv)))

#define SETF_VAR(field, var, fieldval) (var = (((var) & ~(field##_MASK)) | (((fieldval) << field##_SHIFT) & (field##_MASK))))

#define GETF(field, val) (((val) & (field##_MASK)) >> (field##_SHIFT))

#define MAKEF_VAR(field, fieldval) (((fieldval) << field##_SHIFT) & (field##_MASK))

/*portsc register*/
#define UDC_PORTSC_CCS (1L << 0)
#define UDC_PORTSC_PED (1L << 1)
#define UDC_PORTSC_PP  (1L << 3)
#define UDC_PORTSC_PR  (1L << 4)

#define UDC_PORTSC_PLS_SHIFT  (5)
#define UDC_PORTSC_PLS_MASK   (0xf << UDC_PORTSC_PLS_SHIFT)
#define UDC_PORTSC_PLS(fv)    (MAKEF_VAR(UDC_PORTSC_PLS, (fv)))
#define UDC_PORTSC_PLS_GET(v) (GETF(UDC_PORTSC_PLS, (v)))

#define UDC_PORTSC_SPEED_SHIFT      (10)
#define UDC_PORTSC_SPEED_MASK       (0xf << UDC_PORTSC_SPEED_SHIFT)
#define UDC_PORTSC_SPEED(fv)        (MAKEF_VAR(UDC_PORTSC_SPEED, (fv)))
#define UDC_PORTSC_SPEED_GET(v)     (GETF(UDC_PORTSC_SPEED, (v)))
#define UDC_PORTSC_SPEED_FS         (0x1)
#define UDC_PORTSC_SPEED_LS         (0x2)
#define UDC_PORTSC_SPEED_HS         (0x3)
#define UDC_PORTSC_SPEED_SS         (0x4)
#define UDC_PORTSC_SPEED_SSP_GEN2X1 (0x5)
#define UDC_PORTSC_SPEED_SSP_GEN1X2 (0x6)
#define UDC_PORTSC_SPEED_SSP_GEN2X2 (0x7)

#define UDC_PORTSC_LWS (1L << 16)
#define UDC_PORTSC_CSC (1L << 17)
#define UDC_PORTSC_PEC (1L << 18)
#define UDC_PORTSC_PPC (1L << 20)
#define UDC_PORTSC_PRC (1L << 21)
#define UDC_PORTSC_PLC (1L << 22)
#define UDC_PORTSC_CEC (1L << 23)
#define UDC_PORTSC_WCE (1L << 25)
#define UDC_PORTSC_WDE (1L << 26)
#define UDC_PORTSC_WPR (1L << 31)

#define PORTSC_W1C_MASK   (UDC_PORTSC_CSC | UDC_PORTSC_PEC | UDC_PORTSC_PPC | UDC_PORTSC_PRC | UDC_PORTSC_PLC | UDC_PORTSC_CEC)
#define PORTSC_WRITE_MASK (~PORTSC_W1C_MASK)

/* u3portpmsc */
#define UDC_U3PORTPM_U1TMOUT_SHIFT (0)
#define UDC_U3PORTPM_U1TMOUT_MASK  (0xff << UDC_U3PORTPM_U1TMOUT_SHIFT)
#define UDC_U3PORTPM_U1TMOUT(fv)   (MAKEF_VAR(UDC_U3PORTPM_U1TMOUT, (fv)))

#define UDC_U3PORTPM_U2TMOUT_SHIFT (8)
#define UDC_U3PORTPM_U2TMOUT_MASK  (0xff << UDC_U3PORTPM_U2TMOUT_SHIFT)
#define UDC_U3PORTPM_U2TMOUT(fv)   (MAKEF_VAR(UDC_U3PORTPM_U2TMOUT, (fv)))

#define UDC_U3PORTPM_FLA (1L << 16)

#define UDC_U3PORTPM_U1IEN_SHIFT (20)
#define UDC_U3PORTPM_U1IEN       (1L << UDC_U3PORTPM_U1IEN_SHIFT)

#define UDC_U3PORTPM_U2IEN_SHIFT (21)
#define UDC_U3PORTPM_U2IEN       (1L << UDC_U3PORTPM_U2IEN_SHIFT)

#define UDC_U3PORTPM_U1AEN_SHIFT (22)
#define UDC_U3PORTPM_U1AEN       (1L << UDC_U3PORTPM_U1AEN_SHIFT)

#define UDC_U3PORTPM_U2AEN_SHIFT (23)
#define UDC_U3PORTPM_U2AEN       (1L << UDC_U3PORTPM_U2AEN_SHIFT)

#define UDC_U3PORTPM_U1U2TMOUT_SHIFT (24)
#define UDC_U3PORTPM_U1U2TMOUT_MASK  (0xff << UDC_U3PORTPM_U1U2TMOUT_SHIFT)

/* u2portpmsc */
#define UDC_U2PORTPM_RJ_TH_SHIFT (0)
#define UDC_U2PORTPM_RJ_TH_MASK  (0xf << UDC_U2PORTPM_RJ_TH_SHIFT)
#define UDC_U2PORTPM_RJ_TH(fv)   (MAKEF_VAR(UDC_U2PORTPM_RJ_TH, (fv)))

#define UDC_U2PORTPM_DS_TH_SHIFT (4)
#define UDC_U2PORTPM_DS_TH_MASK  (0xf << UDC_U2PORTPM_DS_TH_SHIFT)
#define UDC_U2PORTPM_DS_TH(fv)   (MAKEF_VAR(UDC_U2PORTPM_DS_TH, (fv)))

#define UDC_U2PORTPM_LPM_EN    (0x1 << 8)
#define UDC_U2PORTPM_RJ_TH_EN  (0x1 << 9)
#define UDC_U2PORTPM_DS_EN     (0x1 << 10)
#define UDC_U2PORTPM_SLP_EN    (0x1 << 11)
#define UDC_U2PORTPM_LPM_FACK  (0x1 << 12)
#define UDC_U2PORTPM_L1_AEX    (0x1 << 13)
#define UDC_U2PORTPM_H_B_SHIFT (16)
#define UDC_U2PORTPM_H_B_MASK  (0xf << UDC_U2PORTPM_H_B_SHIFT)
#define UDC_U2PORTPM_H_B(fv)   (MAKEF_VAR(UDC_U2PORTPM_H_B, (fv)))

#define UDC_U2PORTPM_RWE (0x1 << 20)

#define UDC_U2PORTPM_TM_SHIFT (28)
#define UDC_U2PORTPM_TM_MASK  (0xf << UDC_U2PORTPM_TM_SHIFT)
#define UDC_U2PORTPM_TM(fv)   (MAKEF_VAR(UDC_U2PORTPM_TM, (fv)))

/* doorbell register*/
#define UDC_DB_TARGET_SHIFT (0)
#define UDC_DB_TARGET_MASK  (0x1f << UDC_DB_TARGET_SHIFT)
#define UDC_DB_TARGET(fv)   (MAKEF_VAR(UDC_DB_TARGET, (fv)))

/* odb registers*/
#define UDC_ODBCFG_2N_OFFSET_SHIFT (0)
#define UDC_ODBCFG_2N_OFFSET_MASK  (0x3ff << UDC_ODBCFG_2N_OFFSET_SHIFT)
#define UDC_ODBCFG_2N_OFFSET(fv)   (MAKEF_VAR(UDC_ODBCFG_2N_OFFSET, (fv)))

#define UDC_ODBCFG_2N_SIZE_SHIFT (10)
#define UDC_ODBCFG_2N_SIZE_MASK  (0x7 << UDC_ODBCFG_2N_SIZE_SHIFT)
#define UDC_ODBCFG_2N_SIZE(fv)   (MAKEF_VAR(UDC_ODBCFG_2N_SIZE, (fv)))

#define UDC_ODBCFG_2N1_OFFSET_SHIFT (16)
#define UDC_ODBCFG_2N1_OFFSET_MASK  (0x3ff << UDC_ODBCFG_2N1_OFFSET_SHIFT)
#define UDC_ODBCFG_2N1_OFFSET(fv)   (MAKEF_VAR(UDC_ODBCFG_2N1_OFFSET, (fv)))

#define UDC_ODBCFG_2N1_SIZE_SHIFT (26)
#define UDC_ODBCFG_2N1_SIZE_MASK  (0x7 << UDC_ODBCFG_2N1_SIZE_SHIFT)
#define UDC_ODBCFG_2N1_SIZE(fv)   (MAKEF_VAR(UDC_ODBCFG_2N1_SIZE, (fv)))

/* command control register*/
#define UDC_CMD_CTRL_ACTIVE_SHIFT (0)
#define UDC_CMD_CTRL_ACTIVE       (1L << UDC_CMD_CTRL_ACTIVE_SHIFT)
#define UDC_CMD_CTRL_IOC_SHIFT    (1)
#define UDC_CMD_CTRL_IOC_EN       (1L << UDC_CMD_CTRL_IOC_SHIFT)

#define UDC_CMD_CTRL_TYPE_SHIFT (4)
#define UDC_CMD_CTRL_TYPE_MASK  (0xf << UDC_CMD_CTRL_TYPE_SHIFT)
#define UDC_CMD_CTRL_TYPE(fv)   (MAKEF_VAR(UDC_CMD_CTRL_TYPE, (fv)))

#define UDC_CMD_CTRL_STATUS_SHIFT  (16)
#define UDC_CMD_CTRL_STATUS_MASK   (0xf << UDC_CMD_CTRL_STATUS_SHIFT)
#define UDC_CMD_CTRL_STATUS(fv)    (MAKEF_VAR(UDC_CMD_CTRL_STATUS, (fv)))
#define UDC_CMD_CTRL_STATUS_GET(v) (GETF(UDC_CMD_CTRL_STATUS, (v)))

#define UDC_CMD_INIT_EP0        (0L)
#define UDC_CMD_UPDATE_EP0      (1L)
#define UDC_CMD_SET_ADDRESS     (2L)
#define UDC_CMD_SEND_DEV_NOTIFY (3L)
#define UDC_CMD_CONFIG_EP       (4L)
#define UDC_CMD_SET_HALT        (5L)
#define UDC_CMD_CLR_HALT        (6L)
#define UDC_CMD_RST_SEQNUM      (7L)
#define UDC_CMD_STOP_EP         (8L)
#define UDC_CMD_SET_TR_DQPTR    (9L)
#define UDC_CMD_FORCE_FLOW_CTRL (10L)
#define UDC_CMD_REQ_LDM_EXCHAG  (11L)

/* int register*/
/* iman bits*/
#define UDC_IMAN_INT_PEND (1L << 0)
#define UDC_IMAN_INT_EN   (1L << 1)

/* erdp bits*/
#define UDC_ERDPLO_EHB        (1 << 3)
#define UDC_ERDPLO_ADDRLO(fv) ((fv) & 0xfffffff0)

// #define DEVDBG
#ifdef DEVDBG
#define USB_LOG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define USB_LOG(format, ...)
#endif

struct erst_s
{
    uint32_t seg_addr_lo;
    uint32_t seg_addr_hi;
    uint32_t seg_size;
    uint32_t rsvd;
};

/* endpoint state */
enum EP_STATE_E
{
    EP_STATE_DISABLED = 0,
    EP_STATE_RUNNING  = 1,
    EP_STATE_HALTED   = 2,
    EP_STATE_STOPPED  = 3
};

/* endpoint type */
enum EP_TYPE_E
{
    EP_TYPE_INVALID = 0,
    EP_TYPE_ISOCH_OUTBOUND,
    EP_TYPE_BULK_OUTBOUND,
    EP_TYPE_INTR_OUTBOUND,
    EP_TYPE_INVALID2,
    EP_TYPE_ISOCH_INBOUND,
    EP_TYPE_BULK_INBOUND,
    EP_TYPE_INTR_INBOUND
};

/* TRB type coding */
enum TRB_TYPE_E
{
    TRB_TYPE_RSVD = 0,
    TRB_TYPE_XFER_NORMAL,
    TRB_TYPE_RSVD2,
    TRB_TYPE_DATA_STAGE,
    TRB_TYPE_STATUS_STAGE,
    TRB_TYPE_DATA_ISOCH, /* 5*/
    TRB_TYPE_LINK,

    TRB_TYPE_EVENT_TRANSFER           = 32,
    TRB_TYPE_EVENT_CMD_COMPLETION     = 33,
    TRB_TYPE_EVENT_PORT_STATUS_CHANGE = 34,
    TRB_TYPE_EVENT_MFINDEX_WRAP       = 39,
    TRB_TYPE_EVENT_SETUP_PKT          = 40,
};

/* TRB completion coding */
enum TRB_CODES_E
{
    CMPL_INVALID = 0,
    CMPL_SUCCESS,
    CMPL_DATA_BUFFER_ERR,
    CMPL_BABBLE_DETECTED_ERR,
    CMPL_USB_TRANS_ERR,
    CMPL_TRB_ERR, /*5*/
    CMPL_TRB_STALL,
    CMPl_INVALID_STREAM_TYPE_ERR = 10,
    CMPL_SHORT_PKT               = 13,
    CMPL_RING_UNDERRUN,
    CMPL_RING_OVERRUN, /*15*/
    CMPL_EVENT_RING_FULL_ERR    = 21,
    CMPL_MISSED_SERVICE_ERR     = 23,
    CMPL_STOPPED                = 26,
    CMPL_STOPPED_LENGTH_INVALID = 27,
    CMPL_ISOCH_BUFFER_OVERRUN   = 31,
    /*192-224 vendor defined error*/
    CMPL_PROTOCOL_STALL          = 192,
    CMPL_SETUP_TAG_MISMATCH      = 193,
    CMPL_HALTED                  = 194,
    CMPL_HALTED_LENGTH_INVALID   = 195,
    CMPL_DISABLED                = 196,
    CMPL_DISABLED_LENGTH_INVALID = 197,
};

/* command type */
enum CMD_TYPE_E
{
    CMD_INIT_EP0              = 0,
    CMD_UPDATE_EP0_CFG        = 1,
    CMD_SET_ADDR              = 2,
    CMD_SEND_DEV_NOTIFICATION = 3,
    CMD_CONFIG_EP             = 4,
    CMD_SET_HALT              = 5,
    CMD_CLEAR_HALT            = 6,
    CMD_RESET_SEQNUM          = 7,
    CMD_STOP_EP               = 8,
    CMD_SET_TR_DQPTR          = 9,
    CMD_FORCE_FLOW_CONTROL    = 10,
    CMD_REQ_LDM_EXCHANGE      = 11
};

static HAL_USB_CtxTypeDef udc_ctx = { 0 };

static int udc_wait_halt_asserted(uint32_t timeout)
{
    HAL_USB_CtlRegTypeDef *ctl_reg  = udc_ctx.ctl_reg;
    uint32_t               cur_time = 0, start_time = 0;

    start_time = HAL_GetMs();
    while (!(ctl_reg->status & UDC_STATUS_DEV_CTRL_HALT))
    {
        cur_time = HAL_GetMs();
        if ((cur_time - start_time) > timeout)
        {
            USB_LOG("%s: timeout!\n", __func__);
            return -1;
        }
    }

    return 0;
}

static int udc_wait_reset_comp(uint32_t timeout)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;
    uint32_t               cur_time, start_time;

    start_time = HAL_GetMs();
    while (ctl_reg->command & UDC_CTRL_SWRST)
    {
        cur_time = HAL_GetMs();
        if ((cur_time - start_time) > timeout)
        {
            USB_LOG("%s: timeout!\n", __func__);
            return -1;
        }
    }

    return 0;
}

static void udc_event_ring_init(int index)
{
    struct erst_s         *p_erst    = 0;
    HAL_USB_IntRegTypeDef *int_reg   = udc_ctx.int_reg;
    HAL_USB_EvtTypeDef    *udc_event = &udc_ctx.udc_event[index];

    /* allocate & initialize the event ring segment table */
    udc_event->event_ring.len = UDC_EVENT_RING_SIZE * sizeof(HAL_USB_EvtTrbDef);

    udc_event->event_ring.vaddr  = (void *)(UDC_EVENTRING_ADDR + UDC_EVENT_RING_SIZE * sizeof(HAL_USB_EvtTrbDef) * index);
    udc_event->evt_dq_pt         = udc_event->event_ring.vaddr;
    udc_event->evt_seg0_last_trb = (HAL_USB_EvtTrbDef *)(udc_event->event_ring.vaddr) + (UDC_EVENT_RING_SIZE - 1);

    udc_event->ccs = 1;

    p_erst              = (struct erst_s *)(UDC_ERST_ADDR + 0x40 * index);
    p_erst->seg_addr_lo = (uint32_t)(udc_event->event_ring.vaddr);
    p_erst->seg_addr_hi = 0;
    p_erst->seg_size    = UDC_EVENT_RING_SIZE;
    p_erst->rsvd        = 0;

    /* clear the event ring, to avoid hw unexpected ops
     * because of dirty data
     */
    memset(udc_event->event_ring.vaddr, 0, udc_event->event_ring.len);

    /* hw related ops ERSTBA && ERSTSZ && ERDP */
    int_reg->erstsz   = UDC_ERST_SIZE;
    int_reg->erstbalo = (uint32_t)p_erst;
    int_reg->erstbahi = 0;
    int_reg->erdplo   = (uint32_t)(udc_event->event_ring.vaddr) | UDC_ERDPLO_EHB;
    int_reg->erdphi   = 0;
    int_reg->iman     = (UDC_IMAN_INT_EN | UDC_IMAN_INT_PEND);
    int_reg->imod     = 0;

    __DMB();

    USB_LOG("%s: event ring initialized!\n", __func__);
}

static void udc_device_context_init(void)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;

    udc_ctx.ep_ctx = (HAL_USB_EpCtxDef *)(UDC_EPCX_ADDR);

    /* hw ops DCBAPLO DCBAPHI */
    ctl_reg->dcbaplo = (uint32_t)udc_ctx.ep_ctx;
    ctl_reg->dcbaphi = 0;

    USB_LOG("%s: dcbaplo[0x%p], %lx\n", __func__, &(ctl_reg->dcbaplo), ctl_reg->dcbaplo);
    USB_LOG("%s: dcbaphi[0x%p], %lx\n", __func__, &(ctl_reg->dcbaphi), ctl_reg->dcbaphi);

    __DMB();

    USB_LOG("%s: udc epx context initialized!\n", __func__);
}

static void udc_setup_link_trb(HAL_USB_TransTrbDef *link_trb, int toggle, uint32_t next_trb)
{
    uint32_t dw = 0;

    link_trb->dw0 = next_trb;
    link_trb->dw1 = 0;

    link_trb->dw2 = 0;

    SETF_VAR(TRB_TYPE, dw, TRB_TYPE_LINK);
    if (toggle)
        SETF_VAR(TRB_LINK_TOGGLE_CYCLE, dw, 1);
    else
        SETF_VAR(TRB_LINK_TOGGLE_CYCLE, dw, 0);

    link_trb->dw3 = dw;
}

static int udc_issue_command(uint32_t type, uint32_t param0, uint32_t param1)
{
    HAL_USB_CtlRegTypeDef *ctl_reg        = udc_ctx.ctl_reg;
    uint32_t               status         = 0;
    int                    check_complete = 0;

    if (ctl_reg->command & UDC_CTRL_RUN)
        check_complete = 1;

    if (check_complete)
    {
        if (ctl_reg->cmd_control & UDC_CMD_CTRL_ACTIVE)
        {
            USB_LOG("%s: prev command is not complete!\n", __func__);
            return -1;
        }
    }

    ctl_reg->cmd_param0 = param0;
    ctl_reg->cmd_param1 = param1;

    ctl_reg->cmd_control = UDC_CMD_CTRL_ACTIVE | UDC_CMD_CTRL_TYPE(type);

    __DMB();

    if (check_complete)
    {
        while (ctl_reg->cmd_control & UDC_CMD_CTRL_ACTIVE)
        {
        };

        USB_LOG("%s: successful\n", __func__);

        status = UDC_CMD_CTRL_STATUS_GET(ctl_reg->cmd_control);
        if (status != 0)
        {
            USB_LOG("%s: fail\n", __func__);
            return -1;
        }
    }

    return 0;
}

static void udc_ep0_init(void)
{
    HAL_USB_EpTypeDef *ep = &udc_ctx.ep[0];
    uint32_t           cmd_param0;
    uint32_t           cmd_param1;

    /* setup transfer ring */
    ep->ep_num          = 0;
    ep->direction       = 0;
    ep->type            = USB_CONTROL_ENDPOINT;
    ep->max_packet_size = USB_MAX_PACKET_SIZE_F;

    ep->tran_ring_info.vaddr = (void *)UDC_EP0_TR_ADDR;
    ep->tran_ring_info.len   = UDC_EP0_TD_RING_SIZE * sizeof(HAL_USB_TransTrbDef);
    ep->first_trb            = (HAL_USB_TransTrbDef *)UDC_EP0_TR_ADDR;
    ep->last_trb             = ep->first_trb + UDC_EP0_TD_RING_SIZE - 1;

    memset(ep->first_trb, 0, ep->tran_ring_info.len);
    ep->enq_pt         = ep->first_trb;
    ep->deq_pt         = ep->first_trb;
    ep->pcs            = 1;
    ep->tran_ring_full = 0;

    udc_setup_link_trb(ep->last_trb, 1, (uint32_t)(ep->tran_ring_info.vaddr));

    /* context related ops */
    cmd_param0 = ((uint32_t)(ep->tran_ring_info.vaddr) & CMD0_0_DQPTRLO_MASK) | CMD0_0_DCS(ep->pcs);
    cmd_param1 = 0;

    udc_issue_command(CMD_INIT_EP0, cmd_param0, cmd_param1);

    USB_LOG("%s: ep0 initialized!\n", __func__);
}

static void udc_control_status_trb(HAL_USB_TransTrbDef *p_trb, uint32_t pcs, int set_addr, int stall, uint8_t tag, int intr_target, int dir)
{
    uint32_t val = 0;

    SETF_VAR(TRB_INTR_TARGET, val, intr_target);
    p_trb->dw2 = val;

    val = 0;
    SETF_VAR(TRB_CYCLE_BIT, val, pcs);
    SETF_VAR(TRB_INTR_ON_COMPLETION, val, 1);
    SETF_VAR(TRB_TYPE, val, TRB_TYPE_STATUS_STAGE);

    SETF_VAR(TRB_DIR, val, dir);

    SETF_VAR(TRB_SETUP_TAG, val, tag);
    SETF_VAR(STATUS_STAGE_TRB_STALL, val, stall);
    SETF_VAR(STATUS_STAGE_TRB_SET_ADDR, val, set_addr);

    p_trb->dw3 = val;
}

static void udc_control_data_trb(HAL_USB_TransTrbDef *trb,
                                 uint32_t             dma,
                                 uint8_t              pcs,
                                 uint32_t             num_trb,
                                 uint32_t             transfer_length,
                                 uint32_t             td_size,
                                 uint8_t              IOC,
                                 uint8_t              AZP,
                                 uint8_t              dir,
                                 uint8_t              setup_tag,
                                 int                  intr_target)
{

    uint32_t val = 0;

    trb->dw0 = dma;
    trb->dw1 = 0;

    /* TRB_Transfer_Length
     *For USB_DIR_OUT, this field is the number of data bytes expected from
     *xhc. For USB_DIR_IN, this field is the number of data bytes the device
     *will send.
     */
    SETF_VAR(TRB_TRANSFER_LEN, val, transfer_length);
    SETF_VAR(TRB_TD_SIZE, val, td_size);
    SETF_VAR(TRB_INTR_TARGET, val, intr_target);
    trb->dw2 = val;

    val = 0;
    SETF_VAR(TRB_CYCLE_BIT, val, pcs);
    SETF_VAR(TRB_INTR_ON_SHORT_PKT, val, 1);
    SETF_VAR(TRB_INTR_ON_COMPLETION, val, IOC);
    SETF_VAR(TRB_TYPE, val, TRB_TYPE_DATA_STAGE);
    SETF_VAR(TRB_APPEND_ZLP, val, AZP);
    SETF_VAR(TRB_DIR, val, dir);
    SETF_VAR(TRB_SETUP_TAG, val, setup_tag);

    trb->dw3 = val;
}

static void udc_knock_doorbell(uint32_t PEI)
{
    uint32_t               reg_val = 0;
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;

    reg_val           = UDC_DB_TARGET(PEI);
    ctl_reg->doorbell = reg_val;

    USB_LOG("%s: PEI=%lx\n", __func__, reg_val);
}

static void udc_stop(void)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;

    /* stop controller and disable interrupt */
    ctl_reg->command &= ~(UDC_CTRL_INT_EN | UDC_CTRL_RUN);
}

static void udc_ep0_status(int stall, int intr_target)
{
    HAL_USB_EpTypeDef   *ep     = (HAL_USB_EpTypeDef *)&udc_ctx.ep[0];
    HAL_USB_TransTrbDef *enq_pt = (HAL_USB_TransTrbDef *)udc_ctx.ep[0].enq_pt;
    uint8_t              tag    = udc_ctx.setup_tag;

    udc_control_status_trb(enq_pt, ep->pcs, 0, stall, tag, intr_target, 0);

    enq_pt = ++(ep->enq_pt);
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
        enq_pt = ep->enq_pt = ep->first_trb;
        ep->pcs ^= 0x1;
    }

    udc_knock_doorbell(0);
}

static void udc_update_ep0_maxpacketsize(uint16_t maxpacketsize)
{
    uint32_t cmd_param = CMD1_0_MPS(maxpacketsize);

    udc_issue_command(CMD_UPDATE_EP0_CFG, cmd_param, 0);
}

static void udc_update_current_speed(void)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;
    HAL_USB_SpeedTypeDef   speed;

    switch (UDC_PORTSC_SPEED_GET(ctl_reg->portsc))
    {
        case UDC_PORTSC_SPEED_SSP_GEN2X2:
            speed = USB_SPEED_SUPER_PLUS;
            udc_update_ep0_maxpacketsize(512);
            break;
        case UDC_PORTSC_SPEED_SSP_GEN1X2:
            speed = USB_SPEED_SUPER_PLUS;
            udc_update_ep0_maxpacketsize(512);
            break;
        case UDC_PORTSC_SPEED_SSP_GEN2X1:
            speed = USB_SPEED_SUPER_PLUS;
            udc_update_ep0_maxpacketsize(512);
            break;
        case UDC_PORTSC_SPEED_SS:
            speed = USB_SPEED_SUPER;
            udc_update_ep0_maxpacketsize(512);
            break;
        case UDC_PORTSC_SPEED_HS:
            speed = USB_SPEED_HIGH;
            udc_update_ep0_maxpacketsize(64);
            break;
        case UDC_PORTSC_SPEED_FS:
            speed = USB_SPEED_FULL;
            udc_update_ep0_maxpacketsize(64);
            break;
        case UDC_PORTSC_SPEED_LS:
        default:
            speed = USB_SPEED_UNKNOWN;
    }

    udc_ctx.max_speed = speed;
}

static void udc_update_dequeue_pt(HAL_USB_EvtTrbDef *event, HAL_USB_EpTypeDef *udc_ep)
{
    uint32_t             deq_pt_lo = event->dw0;
    HAL_USB_TransTrbDef *deq_pt;

    deq_pt = (HAL_USB_TransTrbDef *)deq_pt_lo;
    deq_pt++;

    if (GETF(TRB_TYPE, deq_pt->dw3) == TRB_TYPE_LINK)
        deq_pt = udc_ep->first_trb;

    udc_ep->deq_pt = deq_pt;
}

static void udc_ep0_xfer_complete(HAL_USB_EvtTrbDef *event)
{
    HAL_USB_TransTrbDef *trb = (HAL_USB_TransTrbDef *)event->dw0;

    if (udc_ctx.ep0_compl_func_ptr)
        udc_ctx.ep0_compl_func_ptr((uint8_t *)trb->dw0, trb->dw2, 0);
}

static void udc_xfer_complete(HAL_USB_EvtTrbDef *event)
{
    uint8_t              PEI = GETF(EVE_TRB_ENDPOINT_ID, event->dw3);
    HAL_USB_TransTrbDef *trb = (HAL_USB_TransTrbDef *)event->dw0;

    if (udc_ctx.cmpl_func_ptr[PEI])
        udc_ctx.cmpl_func_ptr[PEI](PEI / 2, PEI % 2, (uint8_t *)trb->dw0, trb->dw2, 0);
    else
        USB_LOG("%s: can not find completion function\n", __func__);
}

static void udc_prepare_transfer_trb(HAL_USB_TransTrbDef *trb,
                                     uint32_t             xfer_len,
                                     uint32_t             xfer_buf_addr,
                                     uint8_t              td_size,
                                     uint8_t              pcs,
                                     uint8_t              trb_type,
                                     uint8_t              short_pkt,
                                     uint8_t              chain_bit,
                                     uint8_t              intr_on_compl,
                                     int                  b_setup_stage,
                                     uint8_t              usb_dir,
                                     int                  b_isoc,
                                     uint8_t              tlb_pc,
                                     uint16_t             frame_i_d,
                                     uint8_t              SIA,
                                     uint8_t              AZP,
                                     uint8_t              intr_target)
{
    uint32_t val = 0;

    trb->dw0 = xfer_buf_addr;
    trb->dw1 = 0;

    SETF_VAR(TRB_TRANSFER_LEN, val, xfer_len);
    SETF_VAR(TRB_TD_SIZE, val, td_size);
    SETF_VAR(TRB_INTR_TARGET, val, intr_target);

    trb->dw2 = val;

    val = 0;
    SETF_VAR(TRB_CYCLE_BIT, val, pcs);
    SETF_VAR(TRB_INTR_ON_SHORT_PKT, val, short_pkt);
    SETF_VAR(TRB_CHAIN_BIT, val, chain_bit);
    SETF_VAR(TRB_INTR_ON_COMPLETION, val, intr_on_compl);
    SETF_VAR(TRB_APPEND_ZLP, val, AZP);
    SETF_VAR(TRB_TYPE, val, trb_type);

    if (b_setup_stage)
        SETF_VAR(TRB_DIR, val, usb_dir);

    if (b_isoc)
    {
        SETF_VAR(ISOC_TRB_FRAME_ID, val, frame_i_d);
        SETF_VAR(ISOC_TRB_SIA, val, SIA);
    }

    trb->dw3 = val;
}

static void udc_epx_setup(HAL_USB_EpTypeDef *ep)
{
    int               PEI    = 2 * ep->ep_num + ep->direction;
    HAL_USB_EpCtxDef *ep_ctx = (HAL_USB_EpCtxDef *)(udc_ctx.ep_ctx + PEI - 2);
    enum EP_TYPE_E    ep_type;
    uint16_t          maxburst = UDC_MAX_BURST;
    uint16_t          maxsize;
    uint32_t          dw;

    if (ep->ep_num == 0)
        return;

    USB_LOG("%s: crgudc->ep_ctx %p, ep_ctx %p\n", __func__, udc_ctx.ep_ctx, ep_ctx);
    USB_LOG("%s: PE: %d, sizeof ep_cx %d\n", __func__, PEI, sizeof(HAL_USB_EpCtxDef));

    /*corigine gadget dir should be opposite to host dir*/
    if (ep->direction == USB_RECV)
        ep_type = ep->type + EP_TYPE_INVALID2;
    else
        ep_type = ep->type;

    maxsize = ep->max_packet_size & 0x07ff;

    /* fill ep_dw0 */
    dw = 0;
    SETF_VAR(UDC_EP_CX_LOGICAL_EP_NUM, dw, ep->ep_num);
    if (ep->type == USB_ISOCHRONOUS_ENDPOINT || ep->type == USB_INTERRUPT_ENDPOINT)
    {
        SETF_VAR(UDC_EP_CX_INTERVAL, dw, UDC_ISO_INTERVAL);
    }
    else
        SETF_VAR(UDC_EP_CX_INTERVAL, dw, 0);

    ep_ctx->dw0 = dw;

    /* fill ep_dw1 */
    dw = 0;
    SETF_VAR(UDC_EP_CX_EP_TYPE, dw, ep_type);
    SETF_VAR(UDC_EP_CX_MAX_PACKET_SIZE, dw, maxsize);
    SETF_VAR(UDC_EP_CX_MAX_BURST_SIZE, dw, maxburst);
    ep_ctx->dw1 = dw;

    /* fill ep_dw2 */
    dw = ep->tran_ring_info.dma & UDC_EP_CX_TR_DQPT_LO_MASK;
    SETF_VAR(UDC_EP_CX_DEQ_CYC_STATE, dw, ep->pcs);
    ep_ctx->dw2 = dw;

    /* fill ep_dw3 */
    ep_ctx->dw3 = 0;
}

void udc_bulk_xfer(uint8_t ep_num, uint8_t dir, uint8_t *addr, uint32_t len, uint32_t intr_target, uint32_t transfer_flag)
{
    int                  i;
    uint8_t              IOC = 1;
    uint8_t              AZP = 0;
    uint32_t             tmp_len;
    uint32_t             TD_SIZE   = 1;
    uint32_t             num_trb   = 1;
    uint32_t             chain_bit = 0;
    HAL_USB_EpTypeDef   *ep;
    HAL_USB_TransTrbDef *enq_pt;
    uint32_t             PEI = 2 * ep_num + dir;
    if (len != 0)
        num_trb = len / MAX_TRB_XFER_LEN + ((len % MAX_TRB_XFER_LEN) ? 1 : 0);
    ep     = (HAL_USB_EpTypeDef *)&udc_ctx.ep[PEI];
    enq_pt = (HAL_USB_TransTrbDef *)udc_ctx.ep[PEI].enq_pt;

    for (i = 0; i < num_trb; i++)
    {
        if (num_trb == 1)
        { // only 1 trb
            tmp_len   = len;
            IOC       = 1;
            chain_bit = 0;
        }
        else if (((i != (num_trb - 1)) && (num_trb > 1)))
        { // num_trb > 1,  not last trb
            tmp_len   = MAX_TRB_XFER_LEN;
            IOC       = 0;
            chain_bit = 1;
        }
        else if ((i == (num_trb - 1)) && (num_trb > 1))
        { // num_trb > 1,  last trb
            tmp_len   = (len % MAX_TRB_XFER_LEN) ? (len % MAX_TRB_XFER_LEN) : MAX_TRB_XFER_LEN;
            IOC       = 1;
            chain_bit = 0;
        }

        if (transfer_flag & XFER_NO_INTR)
            IOC = 0;

        if (transfer_flag & XFER_SET_CHAIN)
            chain_bit = 1;

        if (transfer_flag & XFER_AZP)
            AZP = 1;

        udc_prepare_transfer_trb(enq_pt, tmp_len, (uint32_t)addr + MAX_TRB_XFER_LEN * i, TD_SIZE, ep->pcs, TRB_TYPE_XFER_NORMAL, 0, chain_bit, IOC, 0, 0, 0, 0, 0, 0, AZP,
                                 intr_target);

        enq_pt = ++udc_ctx.ep[PEI].enq_pt;
        if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
        {
            SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
            enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
            ep->pcs ^= 0x1;
        }
    }

    if (transfer_flag & XFER_NO_DB)
        return;

    udc_knock_doorbell(PEI);
}

void udc_isoc_xfer(uint8_t ep_num, uint8_t dir, uint8_t *addr, int len, int intr_target, int no_intr, int no_knock)
{
    uint32_t             TD_SIZE   = 1;
    uint8_t              IOC       = 1;
    uint8_t              SIA       = 0;
    uint8_t              b_isoc    = 1;
    uint32_t             chain_bit = 0;
    HAL_USB_EpTypeDef   *ep;
    HAL_USB_TransTrbDef *enq_pt;
    uint32_t             PEI = 2 * ep_num + dir;

    ep     = (HAL_USB_EpTypeDef *)&udc_ctx.ep[PEI];
    enq_pt = (HAL_USB_TransTrbDef *)udc_ctx.ep[PEI].enq_pt;

    if (len > MAX_TRB_XFER_LEN)
    {
        chain_bit = 1;
        IOC       = 0;
        b_isoc    = 1;
        SIA       = 1;
        udc_prepare_transfer_trb(enq_pt, MAX_TRB_XFER_LEN, (uint32_t)addr, TD_SIZE, ep->pcs, TRB_TYPE_DATA_ISOCH, 0, chain_bit, IOC, 0, 0, b_isoc, 0, 0, SIA, 0, intr_target);

        enq_pt = ++udc_ctx.ep[PEI].enq_pt;
        if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
        {
            SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
            enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
            ep->pcs ^= 0x1;
        }

        chain_bit = 0;
        IOC       = 1;

        if (no_intr == 1)
        {
            IOC = 0;
        }

        udc_prepare_transfer_trb(enq_pt, len - MAX_TRB_XFER_LEN, (uint32_t)addr + MAX_TRB_XFER_LEN, 0, ep->pcs, TRB_TYPE_XFER_NORMAL, 0, chain_bit, IOC, 0, 0, 0, 0, 0, 0, 0,
                                 intr_target);

        enq_pt = ++udc_ctx.ep[PEI].enq_pt;
        if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
        {
            SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
            enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
            ep->pcs ^= 0x1;
        }
    }
    else
    {
        chain_bit = 0;
        IOC       = 1;
        b_isoc    = 1;
        SIA       = 1;

        if (no_intr == 1)
        {
            IOC = 0;
        }

        udc_prepare_transfer_trb(enq_pt, len, (uint32_t)addr, 0, ep->pcs, TRB_TYPE_DATA_ISOCH, 0, chain_bit, IOC, 0, 0, b_isoc, 0, 0, SIA, 0, intr_target);

        enq_pt = ++udc_ctx.ep[PEI].enq_pt;
        if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
        {
            SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
            enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
            ep->pcs ^= 0x1;
        }
    }

    if (!no_knock)
        udc_knock_doorbell(PEI);
}

void udc_isoc_xfer_sg2(uint8_t ep_num, uint8_t dir, uint8_t *addr1, int len1, uint8_t *addr2, int len2, int intr_target, int no_intr, int no_knock)
{
    uint32_t             TD_SIZE   = 1;
    uint8_t              SIA       = 0;
    uint8_t              IOC       = 0;
    uint8_t              b_isoc    = 1;
    uint8_t              chain_bit = 0;
    HAL_USB_EpTypeDef   *ep;
    HAL_USB_TransTrbDef *enq_pt;
    uint32_t             PEI = 2 * ep_num + dir;

    ep     = (HAL_USB_EpTypeDef *)&udc_ctx.ep[PEI];
    enq_pt = (HAL_USB_TransTrbDef *)udc_ctx.ep[PEI].enq_pt;

    b_isoc = 1;
    SIA    = 1;

    USB_LOG("%s: len1 = %d, len2 = %d, TD_SIZE = %ld\n", __func__, len1, len2, TD_SIZE);

    chain_bit = 1;
    udc_prepare_transfer_trb(enq_pt, len1, (uint32_t)addr1, TD_SIZE, ep->pcs, TRB_TYPE_DATA_ISOCH, 0, chain_bit, IOC, 0, 0, b_isoc, 0, 0, SIA, 0, 0);
    enq_pt = ++udc_ctx.ep[PEI].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
        enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
        ep->pcs ^= 0x1;
    }

    if (no_intr == 1)
    {
        IOC = 0;
    }
    else
        IOC = 1;

    chain_bit = 0;
    udc_prepare_transfer_trb(enq_pt, len2, (uint32_t)addr2, 0, ep->pcs, TRB_TYPE_XFER_NORMAL, 0, chain_bit, IOC, 0, 0, 0, 0, 0, 0, 0, intr_target);

    enq_pt = ++udc_ctx.ep[PEI].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
        enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
        ep->pcs ^= 0x1;
    }

    if (!no_knock)
        udc_knock_doorbell(PEI);
}

void udc_isoc_xfer_sg4(
    uint8_t ep_num, uint8_t dir, uint8_t *addr1, int len1, uint8_t *addr2, int len2, uint8_t *addr3, int len3, uint8_t *addr4, int len4, int intr_target, int no_intr, int no_knock)
{
    uint32_t             TD_SIZE   = 0;
    uint8_t              SIA       = 0;
    uint8_t              IOC       = 0;
    uint8_t              b_isoc    = 1;
    uint8_t              chain_bit = 0;
    uint32_t             len_rem;
    HAL_USB_EpTypeDef   *ep;
    HAL_USB_TransTrbDef *enq_pt;
    uint32_t             PEI = 2 * ep_num + dir;

    ep     = (HAL_USB_EpTypeDef *)&udc_ctx.ep[PEI];
    enq_pt = (HAL_USB_TransTrbDef *)udc_ctx.ep[PEI].enq_pt;

    b_isoc  = 1;
    SIA     = 1;
    TD_SIZE = 1;

    len_rem = ((len1 % 1024) + len2 + len3 + len4);
    TD_SIZE = len_rem / 1024 + (len_rem % 1024 ? 1 : 0);

    if (len2 == 0)
    {
        TD_SIZE++;
    }

    chain_bit = 1;

    USB_LOG("%s: len1 = %d, TD_SIZE = %ld\n", __func__, len1, TD_SIZE);

    udc_prepare_transfer_trb(enq_pt, len1, (uint32_t)addr1, TD_SIZE, ep->pcs, TRB_TYPE_DATA_ISOCH, 0, chain_bit, IOC, 0, 0, b_isoc, 0, 0, SIA, 0, 0);
    enq_pt = ++udc_ctx.ep[PEI].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
        enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
        ep->pcs ^= 0x1;
    }

    len_rem = (len1 % 1024 + len2) % 1024 + len3 + len4;
    TD_SIZE = len_rem / 1024 + (len_rem % 1024 ? 1 : 0);

    chain_bit = 1;
    USB_LOG("%s: len2 = %d, TD_SIZE = %ld\n", __func__, len2, TD_SIZE);

    udc_prepare_transfer_trb(enq_pt, len2, (uint32_t)addr2, 0, ep->pcs, TRB_TYPE_XFER_NORMAL, 0, chain_bit, IOC, 0, 0, 0, 0, 0, 0, 0, intr_target);

    enq_pt = ++udc_ctx.ep[PEI].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
        enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
        ep->pcs ^= 0x1;
    }

    len_rem = ((len1 % 1024 + len2) % 1024 + len3) % 1024 + len4;
    TD_SIZE = len_rem / 1024 + (len_rem % 1024 ? 1 : 0);

    chain_bit = 1;

    USB_LOG("%s: len3 = %d, TD_SIZE = %ld\n", __func__, len3, TD_SIZE);
    udc_prepare_transfer_trb(enq_pt, len3, (uint32_t)addr3, 0, ep->pcs, TRB_TYPE_XFER_NORMAL, 0, chain_bit, IOC, 0, 0, 0, 0, 0, 0, 0, intr_target);

    enq_pt = ++udc_ctx.ep[PEI].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
        enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
        ep->pcs ^= 0x1;
    }

    if (no_intr == 1)
    {
        IOC = 0;
    }
    else
        IOC = 1;

    chain_bit = 0;
    TD_SIZE   = 0;

    USB_LOG("%s: len4 = %d, TD_SIZE = %ld\n", __func__, len4, TD_SIZE);

    udc_prepare_transfer_trb(enq_pt, len4, (uint32_t)addr4, 0, ep->pcs, TRB_TYPE_XFER_NORMAL, 0, chain_bit, IOC, 0, 0, 0, 0, 0, 0, 0, intr_target);

    enq_pt = ++udc_ctx.ep[PEI].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
        enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
        ep->pcs ^= 0x1;
    }

    if (!no_knock)
        udc_knock_doorbell(PEI);
}

void udc_intr_xfer(uint8_t ep_num, uint8_t dir, uint8_t *addr, int len, int intr_target, int no_intr, int no_knock)
{
    int                  i;
    uint32_t             tmp_len;
    uint8_t              IOC       = 1;
    uint32_t             num_trb   = 1;
    uint32_t             chain_bit = 0;
    HAL_USB_EpTypeDef   *ep;
    HAL_USB_TransTrbDef *enq_pt;
    uint32_t             PEI = 2 * ep_num + dir;

    if (len != 0)
        num_trb = len / MAX_TRB_XFER_LEN + ((len % MAX_TRB_XFER_LEN) ? 1 : 0);
    ep     = (HAL_USB_EpTypeDef *)&udc_ctx.ep[PEI];
    enq_pt = (HAL_USB_TransTrbDef *)udc_ctx.ep[PEI].enq_pt;

    for (i = 0; i < num_trb; i++)
    {
        if (num_trb == 1)
        {
            tmp_len   = len;
            IOC       = 1;
            chain_bit = 0;
        }
        else if (((i != (num_trb - 1)) && (num_trb != 1)))
        {
            tmp_len   = MAX_TRB_XFER_LEN;
            IOC       = 0;
            chain_bit = 1;
        }
        else if ((i == (num_trb - 1)) && (num_trb != 1))
        {
            tmp_len   = (len % MAX_TRB_XFER_LEN) ? (len % MAX_TRB_XFER_LEN) : MAX_TRB_XFER_LEN;
            IOC       = 1;
            chain_bit = 0;
        }

        if (no_intr == 1)
        {
            IOC = 0;
        }

        udc_prepare_transfer_trb(enq_pt, tmp_len, (uint32_t)addr + MAX_TRB_XFER_LEN * i, 1, ep->pcs, TRB_TYPE_XFER_NORMAL, 0, chain_bit, IOC, 0, 0, 0, 0, 0, 0, 0, intr_target);

        enq_pt = ++udc_ctx.ep[PEI].enq_pt;
        if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
        {
            SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
            enq_pt = udc_ctx.ep[PEI].enq_pt = ep->first_trb;
            ep->pcs ^= 0x1;
        }
    }

    if (!no_knock)
        udc_knock_doorbell(PEI);
}

static int udc_handle_event(HAL_USB_EvtTrbDef *event)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;
    uint32_t               portsc_val;
    uint32_t               comp_code;
    uint32_t               PEI = GETF(EVE_TRB_ENDPOINT_ID, event->dw3);
    HAL_USB_EpTypeDef     *ep  = (HAL_USB_EpTypeDef *)&udc_ctx.ep[PEI];

    switch (GETF(EVE_TRB_TYPE, event->dw3))
    {
        case TRB_TYPE_EVENT_PORT_STATUS_CHANGE: {
            /* handle Port Reset */
            portsc_val      = ctl_reg->portsc;
            ctl_reg->portsc = portsc_val;
            __DMB();

            uint32_t cs = portsc_val & UDC_PORTSC_CCS;
            uint32_t pp = portsc_val & UDC_PORTSC_PP;

            USB_LOG("%s: Current Port Link State is %lx\n", __func__, UDC_PORTSC_PLS_GET(portsc_val));

            if (portsc_val & UDC_PORTSC_CSC)
            {
                if (portsc_val & UDC_PORTSC_CCS)
                {
                    USB_LOG("%s: Port Connection\n", __func__);
                }
                else
                {
                    USB_LOG("%s: Port DisConnection\n", __func__);
                }
            }

            if (portsc_val & UDC_PORTSC_PPC)
            {
                if (portsc_val & UDC_PORTSC_PP)
                    USB_LOG("%s: Power Present\n", __func__);
                else
                    USB_LOG("%s: Power Not Present\n", __func__);
            }

            if ((portsc_val & UDC_PORTSC_CSC) || (portsc_val & UDC_PORTSC_PPC))
            {
                if (cs && pp)
                {
                    USB_LOG("%s: cable connect and power present\n", __func__);
                    udc_update_current_speed();
                }
            }

            if (portsc_val & UDC_PORTSC_PRC)
            {
                if (portsc_val & UDC_PORTSC_PR)
                {
                    USB_LOG("%s: In Port Reset Process\n", __func__);
                }
                else
                {
                    USB_LOG("%s: Port Reset Done\n", __func__);
                    udc_update_current_speed();
                }
            }

            if (portsc_val & UDC_PORTSC_PLC)
            {
                USB_LOG("%s: Port Link State Change\n", __func__);
                if (UDC_PORTSC_PLS_GET(portsc_val) == 3)
                {
                    USB_LOG("%s: Link suspend to U3\n", __func__);
                }
                else if (UDC_PORTSC_PLS_GET(portsc_val) == 2)
                {
                    USB_LOG("%s: Link enter U2\n", __func__);
                }
                else if (UDC_PORTSC_PLS_GET(portsc_val) == 0)
                {
                    USB_LOG("%s: Link enter U0\n", __func__);
                }
                else if (UDC_PORTSC_PLS_GET(portsc_val) == 15)
                {
                    USB_LOG("%s: Link is Resume\n", __func__);
                }
                udc_ctx.power_state_notify(UDC_PORTSC_PLS_GET(portsc_val));
            }

            if (!cs && !pp)
            {
                USB_LOG("%s: cable disconnect and power not present\n", __func__);
            }

            ctl_reg->evtconfig |= UDC_CFG1_SETUP_EVENT_EN;
        }
        break;
        case TRB_TYPE_EVENT_TRANSFER:
            comp_code = GETF(EVE_TRB_COMPL_CODE, event->dw2);
            udc_update_dequeue_pt(event, ep);

            USB_LOG("%s: comp_code = %ld, PEI = %ld\n", __func__, comp_code, PEI);

            if (PEI == 0)
            {
                udc_ep0_xfer_complete(event);
            }
            else if (PEI >= 2)
            {
                if (comp_code == CMPL_SUCCESS || comp_code == CMPL_SHORT_PKT)
                {
                    udc_xfer_complete(event);
                }
                else if (comp_code == CMPL_MISSED_SERVICE_ERR)
                {
                    if (udc_ctx.max_speed >= USB_SPEED_SUPER)
                    {
                        USB_LOG("%s: CMPL_MISSED_SERVICE_ERR\n", __func__);
                    }
                    else
                    {
                        USB_LOG("%s: CMPL_MISSED_SERVICE_ERR\n", __func__);
                    }
                }
                else
                {
                    USB_LOG("%s: ######comp_code = %ld, PEI = %ld\n", __func__, comp_code, PEI);
                }
            }

            break;
        case TRB_TYPE_EVENT_SETUP_PKT: {
            USB_LOG("%s: handle_setup_pkt\n", __func__);

            memcpy((uint8_t *)udc_ctx.setup, (const void *)&event->dw0, 8);

            udc_ctx.setup_tag = GETF(EVE_TRB_SETUP_TAG, event->dw3);
            USB_LOG("%s: setup_pkt = %p, setup_tag = %x\n", __func__, udc_ctx.setup, udc_ctx.setup_tag);

            udc_ctx.handle_setup((uint8_t *)udc_ctx.setup);

            break;
        }
        default:
            USB_LOG("%s: unexpect TRB_TYPE = %lx", __func__, GETF(EVE_TRB_TYPE, event->dw3));
            break;
    }

    return 0;
}

static int udc_process_event_ring(int index)
{
    HAL_USB_IntRegTypeDef *int_reg = udc_ctx.int_reg;
    HAL_USB_EvtTrbDef     *event;
    HAL_USB_EvtTypeDef    *udc_event;
    int                    ret = 0;

    int_reg->iman |= UDC_IMAN_INT_PEND;

    udc_event = (HAL_USB_EvtTypeDef *)&udc_ctx.udc_event[index];
    while (udc_event->evt_dq_pt)
    {
        event = (HAL_USB_EvtTrbDef *)(udc_event->evt_dq_pt);

        if (GETF(EVE_TRB_CYCLE_BIT, event->dw3) != udc_event->ccs)
        {
            break;
        }

        ret = udc_handle_event(event);
        if (ret)
            return ret;

        if (event == udc_event->evt_seg0_last_trb)
        {
            USB_LOG("%s: evt_last_trb = 0x%p\n", __func__, udc_event->evt_seg0_last_trb);
            USB_LOG("%s: evt_dq_pt = 0x%p\n", __func__, udc_event->evt_dq_pt);
            udc_event->ccs       = udc_event->ccs ? 0 : 1;
            udc_event->evt_dq_pt = udc_event->event_ring.vaddr;
        }
        else
        {
            udc_event->evt_dq_pt++;
        }
    }

    /* update dequeue pointer */
    int_reg->erdplo = (uint32_t)(udc_event->evt_dq_pt) | UDC_ERDPLO_EHB;
    int_reg->erdphi = 0;

    __DMB();

    return 0;
}

static void udc_handle_interrupt(const void *param)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;

    int i = 0;

    if (ctl_reg->status & UDC_STATUS_SYS_ERR)
    {
        USB_LOG("%s: System error happens!!!\n", __func__);
        ctl_reg->status = UDC_STATUS_SYS_ERR;

        __DMB();

        USB_LOG("%s: uccr->control: %lx\n", __func__, ctl_reg->command);
    }

    if (ctl_reg->status & UDC_STATUS_EINT)
    {
        ctl_reg->status = UDC_STATUS_EINT;
        __DMB();

        for (i = 0; i < UDC_EVENT_RING_NUM; i++)
            udc_process_event_ring(i);
    }
}

/**
 * @brief Initializes the USB controller.
 * @param USB context.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function performs the necessary hardware initialization of the USB
 * controller, setting up the controller to handle USB events and data
 * transfers.
 */
HAL_StatusTypeDef HAL_USB_Initialize(HAL_USB_CtxTypeDef *ctx)
{
    udc_ctx.ctl_reg = (HAL_USB_CtlRegTypeDef *)(UDC_MMIOBASE + UDC_UCCR_OFFSET);
    udc_ctx.int_reg = (HAL_USB_IntRegTypeDef *)(UDC_MMIOBASE + UDC_UICR_OFFSET);

    /* check if usb is disconnect */
    if (udc_ctx.ctl_reg->command & UDC_CTRL_RUN)
    {
        USB_LOG("%s: usb is initialized, please disconnect it first\n", __func__);
        return HAL_ERROR;
    }

    /* FIXME: disable interrupt ? */
    if (udc_ctx.ctl_reg->command & UDC_CTRL_INT_EN)
        udc_ctx.ctl_reg->command &= ~UDC_CTRL_INT_EN;

    /* register usb interrupt */
    HAL_NVIC_ConnectIRQ(USB_IRQn, USB_IRQ_PRIO, USB_IRQ_SUB_PRIO, udc_handle_interrupt, 0, 0);

    /* udc reset */
    udc_ctx.ctl_reg->command |= UDC_CTRL_SWRST;
    if (udc_wait_reset_comp(5))
    {
        USB_LOG("%s: usb reset failed\n", __func__);
        return HAL_ERROR;
    }

    /* config usb max speed */
    if (ctx->max_speed == USB_SPEED_FULL)
    {
        udc_ctx.ctl_reg->devconfig = 0x80 | (UDC_CFG0_MAXSPEED_FS & UDC_CFG0_MAXSPEED_MASK);
    }
    else if (ctx->max_speed == USB_SPEED_HIGH)
    {
        udc_ctx.ctl_reg->devconfig = 0x80 | (UDC_CFG0_MAXSPEED_HS & UDC_CFG0_MAXSPEED_MASK);
    }
    else
    {
        USB_LOG("%s: usb speed set error\n", __func__);
        return HAL_ERROR;
    }

    USB_LOG("%s: config0: %lx\n", __func__, udc_ctx.ctl_reg->devconfig);

    /* config event */
    udc_ctx.ctl_reg->evtconfig |= (UDC_CFG1_CSC_EVENT_EN | UDC_CFG1_PEC_EVENT_EN | UDC_CFG1_PPC_EVENT_EN | UDC_CFG1_PRC_EVENT_EN | UDC_CFG1_PLC_EVENT_EN | UDC_CFG1_CEC_EVENT_EN
                                   | UDC_CFG1_SETUP_EVENT_EN);

    /* initialize event ring */
    for (int i = 0; i < UDC_EVENT_RING_NUM; i++)
    {
        udc_event_ring_init(i);
    }

    /* init device context and ep context, refer to 7.6.2 */
    udc_device_context_init();

    /* initial ep0 transfer ring */
    udc_ep0_init();

    /* disable u1 u2 */
    udc_ctx.ctl_reg->u3portpmsc = 0;
    /* disable 2.0 LPM */
    udc_ctx.ctl_reg->u2portpmsc = 0;

    udc_ctx.max_speed          = ctx->max_speed;
    udc_ctx.ep0_compl_func_ptr = ctx->ep0_compl_func_ptr;
    udc_ctx.handle_setup       = ctx->handle_setup;
    udc_ctx.power_state_notify = ctx->power_state_notify;

    USB_LOG("%s: usb initialized !\r\n", __func__);

    return HAL_OK;
}

/**
 * @brief Uninitializes the USB controller.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function releases resources and performs cleanup for the USB controller,
 * restoring it to an uninitialized state.
 */
HAL_StatusTypeDef HAL_USB_Uninitialize(void)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;

    /* disconenct controller and disable interrupt */
    udc_stop();
    if (udc_wait_halt_asserted(10))
        ctl_reg->command |= UDC_CTRL_KP_CNCT;

    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_SetAddress(uint8_t address, int intr_target)
{
    HAL_USB_EpTypeDef   *ep     = (HAL_USB_EpTypeDef *)&udc_ctx.ep[0];
    HAL_USB_TransTrbDef *enq_pt = (HAL_USB_TransTrbDef *)(ep->enq_pt);

    udc_ctx.feature_u1_enable = 0;
    udc_ctx.feature_u2_enable = 0;

    udc_issue_command(CMD_SET_ADDR, CMD2_0_DEV_ADDR(address), 0);

    udc_control_status_trb(enq_pt, ep->pcs, 1, 0, udc_ctx.setup_tag, intr_target, 0);

    enq_pt = ++udc_ctx.ep[0].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep->pcs);
        enq_pt = udc_ctx.ep[0].enq_pt = ep->first_trb;
        ep->pcs ^= 0x1;
    }

    udc_knock_doorbell(0);

    return HAL_OK;
}

/**
 * @brief  Enables USB controller interrupts.
 * @param  None.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function enables the interrupt system for the USB controller, allowing
 * it to respond to events such as data transfers, resets, and suspend/resume
 * conditions.
 */
HAL_StatusTypeDef HAL_USB_EnableInt(void)
{
    /* enable usb interrupt */
    HAL_NVIC_EnableIRQ(USB_IRQn);
    return HAL_OK;
}

/**
 * @brief  Disables USB controller interrupts.
 * @param  None.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function disables the interrupt system for the USB controller,
 * preventing it from responding to USB events. This may be used in low-power
 * states or during critical operations where interrupts must be masked.
 */
HAL_StatusTypeDef HAL_USB_DisableInt(void)
{
    HAL_NVIC_DisableIRQ(USB_IRQn);
    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_Ep_SetStall(int endpoint_id, int stall)
{
    return 0;
}

/**
 * @brief  Clears a stall condition on a specific endpoint.
 * @param  endpoint_id ID of the endpoint.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function clears the stall condition on a specific endpoint, allowing it
 * to resume normal operation. It is commonly used to recover from error
 * conditions.
 */
HAL_StatusTypeDef HAL_USB_Ep_ClearStall(int endpoint_id)
{
    return 0;
}

/**
 * @brief  Registers a completion callback for an endpoint.
 * @param  endpoint_id ID of the endpoint.
 * @param  callback Pointer to the callback function.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function registers a callback function that will be called upon the
 * completion of a transfer on the specified endpoint. The callback provides a
 * mechanism for the application to respond to completed transfers, errors, or
 * other endpoint events.
 */
HAL_StatusTypeDef HAL_USB_Ep_CompRegister(int endpoint_id, uint8_t direction, HAL_USB_EpxCmplCB cb)
{
    uint8_t index = (endpoint_id * 2) + direction;

    udc_ctx.cmpl_func_ptr[index] = cb;

    return HAL_OK;
}

/**
 * @brief  Sets power management mode for the USB controller.
 * @param  mode Power management mode to set (e.g., USB_PM_ACTIVE).
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function configures the power management mode of the USB controller.
 * Different modes may include active, suspend, or sleep states, depending
 * on the desired power consumption and operational requirements.
 */
HAL_StatusTypeDef HAL_USB_SetPm(int mode)
{
    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_ClearPm(void)
{
    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_EpxEnable(int endpoint_id, int type, int direction, uint16_t max_packet_size)
{
    uint32_t           param0 = 0;
    uint32_t           dma    = 0;
    int                PEI    = 2 * endpoint_id + direction;
    HAL_USB_EpTypeDef *ep     = (HAL_USB_EpTypeDef *)&udc_ctx.ep[PEI];
    int                len    = UDC_TD_RING_SIZE * sizeof(HAL_USB_TransTrbDef);
    uint8_t           *vaddr  = 0;

    if (endpoint_id == 0)
    {
        USB_LOG("%s: endpoint number error:%d\n", __func__, endpoint_id);
        return HAL_ERROR;
    }

    vaddr = (uint8_t *)(UDC_EP_TR_ADDR + (PEI - 2) * len);
    dma   = (uint32_t)vaddr;
    USB_LOG("%s: udc_ep->PEI = %d, transfer ring addr = %lx\n", __func__, PEI, (uint32_t)vaddr);
    if (!vaddr || (dma > (uint32_t)(UDC_EP_TR_ADDR + UDC_EP_TRSIZE)))
    {
        USB_LOG("%s: ########failed to allocate trb ring !!!#######, dma:%lx, %lx\n", __func__, dma, (uint32_t)(UDC_EP_TR_ADDR + UDC_EP_TRSIZE));
        return HAL_ERROR;
    }

    ep->ep_num          = endpoint_id;
    ep->direction       = direction;
    ep->max_packet_size = max_packet_size;
    ep->type            = type;

    ep->tran_ring_info.vaddr = vaddr;
    ep->tran_ring_info.dma   = dma;
    ep->tran_ring_info.len   = len;
    ep->first_trb            = (HAL_USB_TransTrbDef *)vaddr;
    ep->last_trb             = ep->first_trb + UDC_TD_RING_SIZE - 1;

    memset(ep->first_trb, 0, ep->tran_ring_info.len);

    udc_setup_link_trb(ep->last_trb, 1, ep->tran_ring_info.dma);

    ep->enq_pt         = ep->first_trb;
    ep->deq_pt         = ep->first_trb;
    ep->pcs            = 1;
    ep->tran_ring_full = 0;

    udc_epx_setup(ep);

    param0 = (0x1 << PEI);
    udc_issue_command(CMD_CONFIG_EP, param0, 0);
    udc_ctx.ep[PEI].ep_state = EP_STATE_RUNNING;

    USB_LOG("%s: config ep and start, DCI=%d\n", __func__, PEI);

    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_EpxDisable(int endpoint_id, int direction)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;
    int                    PEI     = 2 * endpoint_id + direction;
    HAL_USB_EpCtxDef      *ep_ctx  = (HAL_USB_EpCtxDef *)udc_ctx.ep_ctx + PEI - 2;
    uint32_t               param0  = (0x1 << (2 * endpoint_id + direction));

    /* stop the DMA from HW first */
    if (param0 & ctl_reg->ep_running)
    {
        udc_issue_command(CMD_STOP_EP, param0, 0);

        while ((ctl_reg->ep_running & param0) != 0)
        {
        }
    }

    ctl_reg->ep_enable = 0x1 << PEI;

    memset(ep_ctx, 0, sizeof(HAL_USB_EpCtxDef));
    udc_ctx.ep[PEI].ep_state = EP_STATE_DISABLED;

    USB_LOG("%s: stop and disable ep, PEI=%d\n", __func__, PEI);
    __DMB();

    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_Ep0Send(void *buf, uint32_t length, int intr_target)
{
    HAL_USB_EpTypeDef   *ep0    = (HAL_USB_EpTypeDef *)&udc_ctx.ep[0];
    HAL_USB_TransTrbDef *enq_pt = (HAL_USB_TransTrbDef *)udc_ctx.ep[0].enq_pt;
    uint8_t              tag    = udc_ctx.setup_tag;

    if (length != 0)
    {
        udc_control_data_trb(enq_pt, (uint32_t)buf, ep0->pcs, 1, length, 0, 1, 0, 0, tag, intr_target);

        enq_pt = ++udc_ctx.ep[0].enq_pt;
        if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
        {
            SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep0->pcs);
            enq_pt = udc_ctx.ep[0].enq_pt = ep0->first_trb;
            ep0->pcs ^= 0x1;
        }
    }

    udc_control_status_trb(enq_pt, ep0->pcs, 0, 0, tag, intr_target, 1);

    enq_pt = ++udc_ctx.ep[0].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep0->pcs);
        enq_pt = udc_ctx.ep[0].enq_pt = ep0->first_trb;
        ep0->pcs ^= 0x1;
    }

    udc_knock_doorbell(0);

    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_Ep0Receive(void *buf, uint32_t length, int intr_target)
{
    HAL_USB_EpTypeDef   *ep0    = (HAL_USB_EpTypeDef *)&udc_ctx.ep[0];
    HAL_USB_TransTrbDef *enq_pt = (HAL_USB_TransTrbDef *)(udc_ctx.ep[0].enq_pt);
    uint8_t              tag    = udc_ctx.setup_tag;

    if (length != 0)
    {
        udc_control_data_trb(enq_pt, (uint32_t)buf, ep0->pcs, 1, length, 0, 0, 0, 1, tag, intr_target);

        enq_pt = ++udc_ctx.ep[0].enq_pt;
        if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
        {
            SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep0->pcs);
            enq_pt = udc_ctx.ep[0].enq_pt = ep0->first_trb;
            ep0->pcs ^= 0x1;
        }
    }

    udc_control_status_trb(enq_pt, ep0->pcs, 0, 0, tag, intr_target, 0);

    enq_pt = ++udc_ctx.ep[0].enq_pt;
    if (GETF(TRB_TYPE, enq_pt->dw3) == TRB_TYPE_LINK)
    {
        SETF_VAR(TRB_CYCLE_BIT, enq_pt->dw3, ep0->pcs);
        enq_pt = udc_ctx.ep[0].enq_pt = ep0->first_trb;
        ep0->pcs ^= 0x1;
    }

    udc_knock_doorbell(0);

    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_EpxSend(int endpoint_id, void *buf, uint32_t length, USB_TransferTypeDef type)
{
    if (buf == NULL || length == 0)
    {
        return HAL_ERROR;
    }

    switch (type)
    {
        case USB_TRANSFER_BULK:
            udc_bulk_xfer(endpoint_id, USB_SEND, buf, length, 0, 0);
            break;
        case USB_TRANSFER_INTERRUPT:
            udc_intr_xfer(endpoint_id, USB_SEND, buf, length, 0, 0, 0);
            break;
        case USB_TRANSFER_ISOCHRONOUS:
            udc_isoc_xfer(endpoint_id, USB_SEND, buf, length, 0, 0, 0);
            break;
        default:
            return HAL_ERROR;
    }

    return HAL_OK;
}

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
HAL_StatusTypeDef HAL_USB_EpxReceive(int endpoint_id, void *buf, uint32_t length, USB_TransferTypeDef type)
{
    if (buf == NULL || length == 0)
    {
        return HAL_ERROR;
    }

    switch (type)
    {
        case USB_TRANSFER_BULK:
            udc_bulk_xfer(endpoint_id, USB_RECV, buf, length, 0, 0);
            break;
        case USB_TRANSFER_INTERRUPT:
            udc_intr_xfer(endpoint_id, USB_RECV, buf, length, 0, 0, 0);
            break;
        case USB_TRANSFER_ISOCHRONOUS:
            udc_isoc_xfer(endpoint_id, USB_RECV, buf, length, 0, 0, 0);
            break;
        default:
            return HAL_ERROR;
    }

    return HAL_OK; // 成功
}

/**
 * @brief  Halts a specific endpoint.
 * @param  endpoint_id ID of the endpoint.
 * @param  direction of the endpoint.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function halts the specified endpoint, which is a state similar to stall
 * but may be used in different contexts or implementations.
 */
HAL_StatusTypeDef HAL_USB_EpxHalt(int endpoint_id, int direction)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;
    uint8_t                PEI     = 2 * endpoint_id + direction;
    uint32_t               param0;

    if (PEI == 0 || PEI == 1)
        PEI = 0;

    if (PEI == 0)
    {
        udc_ep0_status(1, 0);
    }
    else if (udc_ctx.ep[PEI].ep_state == EP_STATE_RUNNING)
    {
        param0 = (0x1 << PEI);
        udc_issue_command(CMD_SET_HALT, param0, 0);

        while ((ctl_reg->ep_running & param0) != 0)
        {
        };
        udc_ctx.ep[PEI].ep_state = EP_STATE_HALTED;
    }

    USB_LOG("%s: the %x endpoint is halted\n", __func__, PEI);

    return HAL_OK;
}

/**
 * @brief  Unhalts a specific endpoint.
 * @param  endpoint_id ID of the endpoint.
 * @param  direction of the endpoint.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function unhalts the specified endpoint, resuming its normal operation
 * after a halt condition has been applied.
 */
HAL_StatusTypeDef HAL_USB_EpxUnhalt(int endpoint_id, int direction)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;
    HAL_USB_TransTrbDef   *deq_pt;
    HAL_USB_EpCtxDef      *epcx;
    uint32_t               param0 = 0;
    uint8_t                PEI    = 2 * endpoint_id + direction;

    epcx   = (HAL_USB_EpCtxDef *)udc_ctx.ep_ctx + PEI - 2;
    deq_pt = (HAL_USB_TransTrbDef *)udc_ctx.ep[PEI].deq_pt;
    param0 = (0x1 << PEI);

    udc_issue_command(CMD_CLEAR_HALT, param0, 0);

    /* fill ep_dw2 */
    epcx->dw2 = (uint32_t)deq_pt | udc_ctx.ep[PEI].pcs;

    /* fill ep_dw3 */
    epcx->dw3 = 0;

    udc_issue_command(CMD_SET_TR_DQPTR, param0, 0);

    while ((ctl_reg->ep_running & param0) == 0)
    {
    };

    udc_ctx.ep[PEI].ep_state = EP_STATE_RUNNING;

    udc_knock_doorbell(PEI);

    USB_LOG("%s: the ep %d is unhalted!\n", __func__, PEI);

    return HAL_OK;
}

/**
 * @brief  Resets the USB controller and its connected devices.
 * @retval int 0 if successful, negative value if an error occurs.
 *
 * This function performs a USB reset, which typically includes resetting
 * the USB controller and reinitializing connected USB devices. This is
 * commonly used during device initialization or error recovery.
 */
HAL_StatusTypeDef HAL_USB_Reset(void)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = (HAL_USB_CtlRegTypeDef *)(UDC_MMIOBASE + UDC_UCCR_OFFSET);

    *(uint32_t *)0x502020fc = 0x00000001;
    __DMB();

#define MAGIC_SIM 1
    // for simulation???
#ifdef MAGIC_MANUAL
    *(uint32_t *)0x50202084 = 0x01401388;
    *(uint32_t *)0x502020f4 = 0x0000f023;
    *(uint32_t *)0x50202088 = 0x06060a09;
    *(uint32_t *)0x5020208c = 0x0d020509;
    *(uint32_t *)0x50202090 = 0x04050603;
    *(uint32_t *)0x50202094 = 0x0303000a;
    *(uint32_t *)0x50202098 = 0x05131304;
    *(uint32_t *)0x5020209c = 0x06070d15;
    *(uint32_t *)0x502020a0 = 0x14160e0b;
    *(uint32_t *)0x502020a4 = 0x18060408;
    *(uint32_t *)0x502020a8 = 0x4b120c0f;
    *(uint32_t *)0x502020ac = 0x03640d05;
    *(uint32_t *)0x502020b0 = 0x08080d09;
    *(uint32_t *)0x502020b4 = 0x20060914;
    *(uint32_t *)0x502020b8 = 0x040a0e0f;
    *(uint32_t *)0x502020bc = 0x44080c09;
#elif defined MAGIC_SIM
    *(uint32_t *)0x50202084 = 0x01401388;
    *(uint32_t *)0x502020f4 = 0x0000f023;
    *(uint32_t *)0x50202088 = 0x3b066409;
    *(uint32_t *)0x5020208c = 0x0d020407;
    *(uint32_t *)0x50202090 = 0x04055050;
    *(uint32_t *)0x50202094 = 0x03030a07;
    *(uint32_t *)0x50202098 = 0x05131304;
    *(uint32_t *)0x5020209c = 0x3b4b0d15;
    *(uint32_t *)0x502020a0 = 0x14168c6e;
    *(uint32_t *)0x502020a4 = 0x18060408;
    *(uint32_t *)0x502020a8 = 0x4b120c0f;
    *(uint32_t *)0x502020ac = 0x03190d05;
    *(uint32_t *)0x502020b0 = 0x08080d09;
    *(uint32_t *)0x502020b4 = 0x20060b03;
    *(uint32_t *)0x502020b8 = 0x040a8c0e;
    *(uint32_t *)0x502020bc = 0x44087d5a;

    *(uint32_t *)0x50202110 = 0x00000000;
#else
    __DMB();
    /*set controller device mode*/
    uccr->debug0 = 0x0400;
    // tmp  = reg_read((uint32_t*)CRG_UHC_MMIOBASE+0x2080+0x7C);
    // reg_write((uint32_t*)(CRG_UHC_MMIOBASE+0x2080+0x7C), tmp | 0x1);
#endif

    /* clear interrupt */
    if (ctl_reg->command & UDC_CTRL_INT_EN)
        ctl_reg->command &= ~UDC_CTRL_INT_EN;

    /* udc reset */
    ctl_reg->command |= UDC_CTRL_SWRST;
    if (udc_wait_reset_comp(5))
    {
        USB_LOG("%s: usb reset failed\n", __func__);
        return HAL_ERROR;
    }

    USB_LOG("%s: usb reset!\n", __func__);

    return HAL_OK;
}

/**
 * determine if the specific endpoint halt.
 * @param    ep_num is endpoint number.
 * @param    direction is endpoint direction
 * @return   1 halted or 0 unhalted
 * @note     None.
 *
 * This function is using for check the ep if it it halted.
 */
uint8_t HAL_USB_EpIsHalted(int endpoint_id, int direction)
{
    uint8_t PEI = 2 * endpoint_id + direction;

    return (udc_ctx.ep[PEI].ep_state == EP_STATE_HALTED);
}

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
void HAL_USB_Set_Sel(uint8_t u1_sel, uint8_t u1_pel, uint16_t u2_sel, uint16_t u2_pel)
{
    udc_ctx.sel_value.u1_sel_value = u1_sel;
    udc_ctx.sel_value.u1_pel_value = u1_pel;
    udc_ctx.sel_value.u2_sel_value = u2_sel;
    udc_ctx.sel_value.u2_pel_value = u2_pel;
}

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
void HAL_USB_ResetSeqNumber(int endpoint_id, int direction)
{
    uint8_t  PEI    = 2 * endpoint_id + direction;
    uint32_t param0 = 0;

    param0 = (0x1 << PEI);
    udc_issue_command(CMD_RESET_SEQNUM, param0, 0);
}

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
HAL_USB_SpeedTypeDef HAL_USB_GetSpeed(void)
{
    return udc_ctx.max_speed;
}

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
void HAL_USB_SetTestMode(int tm)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;
    uint32_t               val     = 0;

    val = ctl_reg->u2portpmsc;
    SETF_VAR(UDC_U2PORTPM_TM, val, tm);
    ctl_reg->u2portpmsc = val;
}

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
void HAL_USB_IRQ_Handle(void)
{
    udc_handle_interrupt(0);
}

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
void HAL_USB_Start(void)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;

    /* interrupt related */
    ctl_reg->devconfig = (UDC_CFG1_CSC_EVENT_EN | UDC_CFG1_PEC_EVENT_EN | UDC_CFG1_PPC_EVENT_EN | UDC_CFG1_PRC_EVENT_EN | UDC_CFG1_PLC_EVENT_EN | UDC_CFG1_CEC_EVENT_EN
                          | UDC_CFG1_INACTIVE_PLC_EN | UDC_CFG1_U3_RESUME_NORESP_PLC_EN | UDC_CFG1_U2_RESUME_NORESP_PLC_EN);

    ctl_reg->command |= (UDC_CTRL_SYSERR_EN | UDC_CTRL_INT_EN | UDC_CTRL_RUN);

    HAL_USB_SetAddress(0, 0);

    USB_LOG("%s: udc start\n", __func__);
}

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
void HAL_USB_Stop(void)
{
    HAL_USB_CtlRegTypeDef *ctl_reg = udc_ctx.ctl_reg;

    ctl_reg->command &= ~(UDC_CTRL_INT_EN | UDC_CTRL_RUN);

    USB_LOG("%s: udc stop\n", __func__);
}