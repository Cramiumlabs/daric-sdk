/**
 ******************************************************************************
 * @file    daric_hal_udma_v3.h
 * @author  UDMA Team
 * @brief   Header file of UDMA HAL module.
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
/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HAL_UDMA_V3_H__
#define __HAL_UDMA_V3_H__

// #include "hal/pulp.h"
#include "daric.h"
#include "daric_pulp_io.h"
#include "daric_udma_v3.h"
#include <stdint.h>

// Robin TODO:
typedef void (*spim_event_t)(void *);

// Robin Van TODO:
#define ARCHI_SOC_PERIPHERALS_ADDR 0x50100000
#define ARCHI_UDMA_OFFSET          0x0
#define ARCHI_UDMA_ADDR            (ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET)

/** @name UDMA HAL
 * The following API can be used to manage the generic part of the UDMA, i.e.
 * for global configuration and channel enqueueing. Other HALs are available for
 * peripheral specific parts. The UDMA is in charge of moving data between
 * peripherals and L2 memory. In order to better pipeline transfers and not
 * loose any data between 2 transfers, 2 transfers at the same time can be
 * enqueued in the UDMA. Once the first one is finished, the UDMA starts the
 * second, while the application should enqueue another one to always have 2
 * pending. By default, all peripherals are clock-gated and must be explicitly
 * activated before being used. The UDMA can use input events to automatically
 * trigger other actions like enqueuing a new transfer.
 */
/**@{*/

/** Tell if a new transfer canbe enqueued to a UDMA channel.
 *
  \param   channelOffset   Offset of the channel
  \return         1 if another transfer can be enqueued, 0 otherwise
  */
static inline int HAL_UDMA_canEnqueue(unsigned int channelOffset);

/** Enqueue a new transfer to a UDMA channel.
 *
  \param   channelOffset Offset of the channel
  \param   l2Addr        Address in L2 memory where the data must be accessed
 (either stored or loaded) \param   size          Size in bytes of the transfer
  \param   cfg           Configuration of the transfer. Can be
 UDMA_CHANNEL_CFG_SIZE_8, UDMA_CHANNEL_CFG_SIZE_16 or UDMA_CHANNEL_CFG_SIZE_32
  */
static inline void HAL_UDMA_Enqueue(unsigned channelOffset, unsigned int l2Addr, unsigned int size, unsigned int cfg);

/** Tell if a channel is busy, i.e. if it already contains at least one on-going
 transfer
 *
  \param   channelOffset Offset of the channel
  \return  1 if the channel is busy, 0 otherwise
  */
static inline int HAL_UDMA_Busy(unsigned channelOffset);

/** Configures peripheral clock-gating.
 *
  \param   value    New configuration. There is 1 bit per peripheral, 0 means
 the peripheral is clock-gated. The bit number corresponds to the channel ID of
 the peripheral.
  */
static inline void HAL_UDMA_CG_Set(unsigned int value);

/** Returns peripheral clock-gating.
 *
  \return The current clock-gating configuration.
  */
static inline unsigned int HAL_UDMA_CG_Get(void);

/** Configures input events
 * 4 input events can be configured. Each input event is made of 8 bits
 specifying which global event this local event refers to. This can then be used
 in some UDMA commands to refer to the global event.
 *
  \param value  The new configuration. Each event is encoded on 8 bits so that
 it can contain a global event ID between 0 and 255.
  */
static inline void HAL_UDMA_Evtin_Set(unsigned int value);

/** Returns input events configuration.
 *
  \return The current input events configuration.
  */
static inline unsigned int HAL_UDMA_Evtin_Get(void);

/** Channel base
 * Returns the channel base from the channel identifier
  \param id The channel identifier
  \return Channel base
 */
static inline unsigned int HAL_UDMA_Channel_Base(int id);

#if 0
/** Channel type (TX or RX)
 * Tells if the channel is a TX channel
  \param id The channel base address
  \return 1 if it is a TX channel
 */
static inline unsigned int hal_udma_channel_isTx(unsigned int addr);
#endif

// UDMA RX/TX Channels HAL Registers Structure
struct plpUdmaRxTxCh_struct_t
{
    uint32_t rx_ch_saddr;
    uint32_t rx_ch_size;
    uint32_t rx_ch_cfg;
    uint32_t rx_ch_initcfg_unused;
    uint32_t tx_ch_saddr;
    uint32_t tx_ch_size;
    uint32_t tx_ch_cfg;
    uint32_t tx_ch_initcfg_unused;
};
typedef volatile struct plpUdmaRxTxCh_struct_t plpUdmaRxTxChHandle_t;

// UDMA RX/TX Channels HAL Register Fields Structure
typedef struct
{
    uint32_t saddr : 16;
    uint32_t unused : 16;
} plpUdmaRxTxChSaddr_t;

typedef struct
{
    uint32_t size : 17;
    uint32_t unused : 15;
} plpUdmaRxTxChSize_t;

typedef struct
{
    uint32_t continuous : 1;
    uint32_t datasize : 2;
    uint32_t unused0 : 1;
    uint32_t enable : 1; // used to check also if a transfer is under going
    // Robin Van TODO: clear bit 6, pending bit 5 according to spec and rtl code
    uint32_t clear_pending : 1; // clear transfer OR check pending transfer in queue
    uint32_t unused1 : 26;
} plpUdmaRxTxChCfg_t;

typedef struct
{
    uint32_t unused : 32;
} plpUdmaRxTxChInitCfg_t;

typedef union
{
    plpUdmaRxTxChSaddr_t saddr;
    plpUdmaRxTxChSize_t  size;
    plpUdmaRxTxChCfg_t   cfg;
    uint32_t             raw;
} plpUdmaRxTxCh_u;

// UDMA RX/TX Channels HAL functions prototype
static inline void     HAL_UDMA_RxChStartAddrSet(plpUdmaRxTxChHandle_t *handle, uint32_t data);
static inline uint32_t HAL_UDMA_RxChStartAddrGet(plpUdmaRxTxChHandle_t *handle);
static inline void     HAL_UDMA_RxChSizeSet(plpUdmaRxTxChHandle_t *handle, uint32_t data);
static inline uint32_t HAL_UDMA_RxChSizeGet(plpUdmaRxTxChHandle_t *handle);
static inline void     HAL_UDMA_RxChCfgSet(plpUdmaRxTxChHandle_t *handle, uint32_t data);
static inline uint32_t HAL_UDMA_RxChCfgGet(plpUdmaRxTxChHandle_t *handle);
static inline void     HAL_UDMA_TxChStartAddrSet(plpUdmaRxTxChHandle_t *handle, uint32_t data);
static inline uint32_t HAL_UDMA_TxChStartAddrGet(plpUdmaRxTxChHandle_t *handle);
static inline void     HAL_UDMA_TxChSizeSet(plpUdmaRxTxChHandle_t *handle, uint32_t data);
static inline uint32_t HAL_UDMA_TxChSizeGet(plpUdmaRxTxChHandle_t *handle);
static inline void     HAL_UDMA_TxChCfgSet(plpUdmaRxTxChHandle_t *handle, uint32_t data);
static inline uint32_t HAL_UDMA_TxChCfgGet(plpUdmaRxTxChHandle_t *handle);

//!@}

/// @cond IMPLEM

// UDMA RX/TX Channels HAL functions definition
static inline void HAL_UDMA_RxChStartAddrSet(plpUdmaRxTxChHandle_t *handle, uint32_t data)
{
    handle->rx_ch_saddr = data;
}

static inline uint32_t HAL_UDMA_RxChStartAddrGet(plpUdmaRxTxChHandle_t *handle)
{
    return handle->rx_ch_saddr;
}

static inline void HAL_UDMA_RxChSizeSet(plpUdmaRxTxChHandle_t *handle, uint32_t data)
{
    handle->rx_ch_size = data;
}

static inline uint32_t HAL_UDMA_RxChSizeGet(plpUdmaRxTxChHandle_t *handle)
{
    return handle->rx_ch_size;
}

static inline void HAL_UDMA_RxChCfgSet(plpUdmaRxTxChHandle_t *handle, uint32_t data)
{
    handle->rx_ch_cfg = data;
}

static inline uint32_t HAL_UDMA_RxChCfgGet(plpUdmaRxTxChHandle_t *handle)
{
    return handle->rx_ch_cfg;
}

static inline void HAL_UDMA_TxChStartAddrSet(plpUdmaRxTxChHandle_t *handle, uint32_t data)
{
    handle->tx_ch_saddr = data;
}

static inline uint32_t HAL_UDMA_TxChStartAddrGet(plpUdmaRxTxChHandle_t *handle)
{
    return handle->tx_ch_saddr;
}

static inline void HAL_UDMA_TxChSizeSet(plpUdmaRxTxChHandle_t *handle, uint32_t data)
{
    handle->tx_ch_size = data;
}

static inline uint32_t HAL_UDMA_TxChSizeGet(plpUdmaRxTxChHandle_t *handle)
{
    return handle->tx_ch_size;
}

static inline void HAL_UDMA_TxChCfgSet(plpUdmaRxTxChHandle_t *handle, uint32_t data)
{
    handle->tx_ch_cfg = data;
}

static inline uint32_t HAL_UDMA_TxChCfgGet(plpUdmaRxTxChHandle_t *handle)
{
    return handle->tx_ch_cfg;
}

static inline void HAL_UDMA_RxChEnqueue(plpUdmaRxTxChHandle_t *handle, uint32_t addr, uint32_t size, uint32_t cfg)
{
    handle->rx_ch_saddr = addr;
    handle->rx_ch_size  = size;
    handle->rx_ch_cfg   = cfg | UDMA_CHANNEL_CFG_EN;
}

static inline void HAL_UDMA_TxChEnqueue(plpUdmaRxTxChHandle_t *handle, uint32_t addr, uint32_t size, uint32_t cfg)
{
    handle->tx_ch_saddr = addr;
    handle->tx_ch_size  = size;
    handle->tx_ch_cfg   = cfg | UDMA_CHANNEL_CFG_EN;
}

///////////////////////////////////////////////////
// TODO Obsolete : to be removed cause deprecated
#if 0
static inline unsigned int hal_udma_channel_isTx(unsigned int addr)
{
    return (addr >> UDMA_CHANNEL_SIZE_LOG2) & 1;
}
#endif

static inline int HAL_UDMA_canEnqueue(unsigned int channelBase)
{
    return !(pulp_read32(channelBase + UDMA_CHANNEL_CFG_OFFSET) & UDMA_CHANNEL_CFG_SHADOW);
}

static inline void HAL_UDMA_Enqueue(unsigned channelBase, unsigned int l2Addr, unsigned int size, unsigned int cfg)
{
    pulp_write32(channelBase + UDMA_CHANNEL_SADDR_OFFSET, l2Addr);
    pulp_write32(channelBase + UDMA_CHANNEL_SIZE_OFFSET, size);
    pulp_write32(channelBase + UDMA_CHANNEL_CFG_OFFSET, cfg | UDMA_CHANNEL_CFG_EN);
    __DSB();
}

#ifdef CONFIG_SOC_DARIC_MPW_A
static inline void HAL_UDMA_clr(unsigned channelBase)
{
    ARCHI_WRITE(channelBase, UDMA_CHANNEL_CFG_OFFSET, UDMA_CHANNEL_CFG_CLEAR);
}
#endif

static inline int HAL_UDMA_Busy(unsigned channelBase)
{
    return (pulp_read32(channelBase + UDMA_CHANNEL_CFG_OFFSET) & UDMA_CHANNEL_CFG_EN);
}

static inline int HAL_UDMA_Queue_Pending(unsigned channelBase)
{
    return (pulp_read32(channelBase + UDMA_CHANNEL_CFG_OFFSET) & UDMA_CHANNEL_CFG_SHADOW);
}

static inline void HAL_UDMA_CG_Set(unsigned int value)
{
    pulp_write32(ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_CONF_OFFSET + UDMA_CONF_CG_OFFSET, value);
}

static inline unsigned int HAL_UDMA_CG_Get()
{
    return pulp_read32(ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_CONF_OFFSET + UDMA_CONF_CG_OFFSET);
}

static inline void HAL_UDMA_RESET_Set(unsigned int value)
{
    pulp_write32(ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_CONF_OFFSET + UDMA_CONF_RESET_OFFSET, value);
}

static inline unsigned int HAL_UDMA_RESET_Get()
{
    return pulp_read32(ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_CONF_OFFSET + UDMA_CONF_RESET_OFFSET);
}

static inline void HAL_UDMA_Evtin_Set(unsigned int value)
{
    pulp_write32(ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_CONF_OFFSET + UDMA_CONF_EVTIN_OFFSET, value);
}

static inline unsigned int HAL_UDMA_Evtin_Get()
{
    return pulp_read32(ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_CONF_OFFSET + UDMA_CONF_EVTIN_OFFSET);
}

static inline unsigned int HAL_UDMA_Periph_Base(int id)
{
    return (unsigned int)(ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_PERIPH_OFFSET(id));
}

static inline unsigned int HAL_UDMA_Channel_Base(int id)
{
    return (unsigned int)(ARCHI_SOC_PERIPHERALS_ADDR + ARCHI_UDMA_OFFSET + UDMA_PERIPH_OFFSET(id >> 1) + UDMA_CHANNEL_OFFSET(id & 1) /*only 2 values: 0, 1*/);
}
///////////////////////////////////////////////////

/// @endcond

#endif
