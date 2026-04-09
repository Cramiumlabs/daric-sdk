/**
 ******************************************************************************
 * @file    daric_hal_mdma.h
 * @author  MDMA Team
 * @brief   Header file of MDMA HAL module.
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
#ifndef DARIC_HAL_MDMA_H
#define DARIC_HAL_MDMA_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include <stdbool.h>

/** @cond Peripheral memory map
 * @{
 */
#define MDMA_CHANNEL_COUNT 8 /*!< Total number of MDMA channels supported */

    typedef struct
    {
        __IO uint32_t STAT; ///< The Register returns the status of the controller.
                            ///< Address offset: 0x00

        __IO uint32_t CFG; ///< The Register controls the configuration of the controller.
                           ///< Address offset: 0x04

        __IO uint32_t CTRL_BASE_PTR; ///< Configure Channel control data base pointer Register.
                                     ///< Address offset: 0x08

        __IO uint32_t ALT_BASE_PTR; ///< The read-only Register returns the base address of the alternate data structure.
                                    ///< Address offset: 0x0C

        __IO uint32_t CH_WREQ_STAT; ///< The read-only Register returns Channel wait on request status.
                                    ///< Address offset: 0x10

        __IO uint32_t CH_SW_REQ; ///< The Register enables you to generate a software DMA request.
                                 ///< Address offset: 0x14

        __IO uint32_t RESERVE[4]; ///< Reserve.
                                  ///< Address offset: 0x18 ~ 0x24

        __IO uint32_t CH_ENSET; ///< The read/write Register enables you to enable a DMA channel.
                                ///< Address offset: 0x28

        __IO uint32_t CH_ENCLR; ///< The write-only Register enables you to disable a DMA channel.
                                ///< Address offset: 0x2C

        __IO uint32_t CH_PRIALT_SET; ///< The read/write Register enables you to configure a DMA channel to use the alternate data structure.
                                     ///< Address offset: 0x30

        __IO uint32_t CH_PRIALT_CLR; ///< The write-only Register enables you to configure a DMA channel to use the primary data structure.
                                     ///< Address offset: 0x34

        __IO uint32_t CH_PRIO_SET; ///< The read/write Register enables you to configure a DMA channel to use the high priority level.
                                   ///< Address offset: 0x38

        __IO uint32_t CH_PRIO_CLR; ///< The write-only Register enables you to configure a DMA channel to use the default priority level.
                                   ///< Address offset: 0x3C
    } MDMA_TypeDef;

    typedef struct
    {
        __IO uint32_t EVESEL; ///< event selection Register for each channel
        __IO uint32_t RESERVE1[MDMA_CHANNEL_COUNT - 1];

        __IO uint32_t REQ; ///< Request enable for each channel.
        __IO uint32_t RESERVE2[MDMA_CHANNEL_COUNT - 1];

        __IO uint32_t SR_REQ; ///< Request status Register for each channel.
    } MDMA_ChannelTypeDef;

    typedef struct
    {
        __IO uint32_t SDEP; ///< The src_data_end_ptr memory location contains a pointer to the end address of the source data.
        __IO uint32_t DDEP; ///< The dst_data_end_ptr memory location contains a pointer to the end address of the destination data.
        __IO uint32_t CDC;  ///< For each DMA transfer, the channel_cfg memory location provides the control information for the controller.
        __IO uint32_t USED; ///< Used.
    } MDMA_NodeTypeDef;

/*!< Peripheral memory map */
#define MDMA_BASE    (0x40011000)
#define MDMA_CH_BASE (MDMA_BASE + 0x1000UL)

#define MDMA0_BASE (MDMA_BASE + 0x00UL)

#define MDMA_CHANNEL0_BASE (MDMA_CH_BASE + 0x00UL)
#define MDMA_CHANNEL1_BASE (MDMA_CH_BASE + 0x04UL)
#define MDMA_CHANNEL2_BASE (MDMA_CH_BASE + 0x08UL)
#define MDMA_CHANNEL3_BASE (MDMA_CH_BASE + 0x0CUL)
#define MDMA_CHANNEL4_BASE (MDMA_CH_BASE + 0x10UL)
#define MDMA_CHANNEL5_BASE (MDMA_CH_BASE + 0x14UL)
#define MDMA_CHANNEL6_BASE (MDMA_CH_BASE + 0x18UL)
#define MDMA_CHANNEL7_BASE (MDMA_CH_BASE + 0x1CUL)

#define MDMA0 ((MDMA_TypeDef *)MDMA0_BASE)

#define MDMA_CHANNEL0 ((MDMA_ChannelTypeDef *)MDMA_CHANNEL0_BASE)
#define MDMA_CHANNEL1 ((MDMA_ChannelTypeDef *)MDMA_CHANNEL1_BASE)
#define MDMA_CHANNEL2 ((MDMA_ChannelTypeDef *)MDMA_CHANNEL2_BASE)
#define MDMA_CHANNEL3 ((MDMA_ChannelTypeDef *)MDMA_CHANNEL3_BASE)
#define MDMA_CHANNEL4 ((MDMA_ChannelTypeDef *)MDMA_CHANNEL4_BASE)
#define MDMA_CHANNEL5 ((MDMA_ChannelTypeDef *)MDMA_CHANNEL5_BASE)
#define MDMA_CHANNEL6 ((MDMA_ChannelTypeDef *)MDMA_CHANNEL6_BASE)
#define MDMA_CHANNEL7 ((MDMA_ChannelTypeDef *)MDMA_CHANNEL7_BASE)

/******************  Bits definition for MDMA register  *****************/
#define MDMA_STAT_ENABLE_Pos (0U)
#define MDMA_STAT_ENABLE_Msk (0x1UL << MDMA_STAT_ENABLE_Pos)
#define MDMA_STAT_ENABLE     MDMA_STAT_ENABLE_Msk

#define MDMA_STAT_STATE_Pos (4U)
#define MDMA_STAT_STATE_Msk (0xFUL << MDMA_STAT_STATE_Pos)
#define MDMA_STAT_STATE     MDMA_STAT_STATE_Msk
#define MDMA_STAT_STATE_0   (0x01UL << MDMA_STAT_STATE_Pos)
#define MDMA_STAT_STATE_1   (0x02UL << MDMA_STAT_STATE_Pos)
#define MDMA_STAT_STATE_2   (0x04UL << MDMA_STAT_STATE_Pos)
#define MDMA_STAT_STATE_3   (0x08UL << MDMA_STAT_STATE_Pos)

#define MDMA_STAT_MINUS_Pos (16U)
#define MDMA_STAT_MINUS_Msk (0x1FUL << MDMA_STAT_MINUS_Pos)
#define MDMA_STAT_MINUS     MDMA_STAT_MINUS_Msk

#define MDMA_CFG_ENABLE_Pos (0U)
#define MDMA_CFG_ENABLE_Msk (0x1UL << MDMA_CFG_ENABLE_Pos)
#define MDMA_CFG_ENABLE     MDMA_CFG_ENABLE_Msk

#define MDMA_CTRL_BASE_PTR_Pos (0U)
#define MDMA_CTRL_BASE_PTR_Msk (0xFFFFFFFFUL << MDMA_CTRL_BASE_PTR_Pos)
#define MDMA_CTRL_BASE_PTR     MDMA_CTRL_BASE_PTR_Msk

#define MDMA_ALT_BASE_PTR_Pos (0U)
#define MDMA_ALT_BASE_PTR_Msk (0xFFFFFFFFUL << MDMA_ALT_BASE_PTR_Pos)
#define MDMA_ALT_BASE_PTR     MDMA_ALT_BASE_PTR_Msk

#define MDMA_REG_CHANNEL0_Msk (0x01UL)
#define MDMA_REG_CHANNEL1_Msk (0x02UL)
#define MDMA_REG_CHANNEL2_Msk (0x04UL)
#define MDMA_REG_CHANNEL3_Msk (0x08UL)
#define MDMA_REG_CHANNEL4_Msk (0x10UL)
#define MDMA_REG_CHANNEL5_Msk (0x20UL)
#define MDMA_REG_CHANNEL6_Msk (0x40UL)
#define MDMA_REG_CHANNEL7_Msk (0x80UL)

#define MDMA_CH_WREQ_STAT_Pos (0U)
#define MDMA_CH_WREQ_STAT_Msk (0xFFUL << MDMA_CH_WREQ_STAT_Pos)
#define MDMA_CH_WREQ_STAT     MDMA_CH_WREQ_STAT_Msk

#define MDMA_CH_SW_REQ_Pos (0U)
#define MDMA_CH_SW_REQ_Msk (0xFFUL << MDMA_CH_SW_REQ_Pos)
#define MDMA_CH_SW_REQ     MDMA_CH_SW_REQ_Msk

#define MDMA_CH_ENSET_Pos (0U)
#define MDMA_CH_ENSET_Msk (0xFFUL << MDMA_CH_ENSET_Pos)
#define MDMA_CH_ENSET     MDMA_CH_ENSET_Msk

#define MDMA_CH_ENCLR_Pos (0U)
#define MDMA_CH_ENCLR_Msk (0xFFUL << MDMA_CH_ENCLR_Pos)
#define MDMA_CH_ENCLR     MDMA_CH_ENCLR_Msk

#define MDMA_CH_CH_PRIALT_SET_Pos (0U)
#define MDMA_CH_CH_PRIALT_SET_Msk (0xFFUL << MDMA_CH_CH_PRIALT_SET_Pos)
#define MDMA_CH_CH_PRIALT_SET     MDMA_CH_CH_PRIALT_SET_Msk

#define MDMA_CH_PRIALT_CLR_Pos (0U)
#define MDMA_CH_PRIALT_CLR_Msk (0xFFUL << MDMA_CH_PRIALT_CLR_Pos)
#define MDMA_CH_PRIALT_CLR     MDMA_CH_PRIALT_CLR_Msk

#define MDMA_CH_PRIO_SET_Pos (0U)
#define MDMA_CH_PRIO_SET_Msk (0xFFUL << MDMA_CH_PRIO_SET_Pos)
#define MDMA_CH_PRIO_SET     MDMA_CH_PRIO_SET_Msk

#define MDMA_CH_PRIO_CLR_Pos (0U)
#define MDMA_CH_PRIO_CLR_Msk (0xFFUL << MDMA_CH_PRIO_CLR_Pos)
#define MDMA_CH_PRIO_CLR     MDMA_CH_PRIO_CLR_Msk

#define MDMA_CH_EVSEL_Pos (0U)
#define MDMA_CH_EVSEL_Msk (0x01UL << MDMA_CH_EVSEL_Pos)
#define MDMA_CH_EVSEL     MDMA_CH_EVSEL_Msk

#define MDMA_CH_REQ_CREN_Pos (0U)
#define MDMA_CH_REQ_CREN_Msk (0x01UL << MDMA_CH_REQ_CREN_Pos)
#define MDMA_CH_REQ_CREN     MDMA_CH_REQ_CREN_Msk

#define MDMA_CH_REQ_CRMODE_Pos (1U)
#define MDMA_CH_REQ_CRMODE_Msk (0x01UL << MDMA_CH_REQ_CRMODE_Pos)
#define MDMA_CH_REQ_CRMODE     MDMA_CH_REQ_CRMODE_Msk

#define MDMA_CH_REQ_REQEN_Pos (2U)
#define MDMA_CH_REQ_REQEN_Msk (0x01UL << MDMA_CH_REQ_REQEN_Pos)
#define MDMA_CH_REQ_REQEN     MDMA_CH_REQ_REQEN_Msk

#define MDMA_CH_REQ_SREQEN_Pos (3U)
#define MDMA_CH_REQ_SREQEN_Msk (0x01UL << MDMA_CH_REQ_SREQEN_Pos)
#define MDMA_CH_REQ_SERQEN     MDMA_CH_REQ_SREQEN_Msk

#define MDMA_CH_REQ_WAITON_Pos (4U)
#define MDMA_CH_REQ_WAITON_Msk (0x01UL << MDMA_CH_REQ_WAITON_Pos)
#define MDMA_CH_REQ_WAITON     MDMA_CH_REQ_WAITON_Msk

#define MDMA_CH_SR_REQ_QLEN_Pos (0U)
#define MDMA_CH_SR_REQ_QLEN_Msk (0x07UL << MDMA_CH_SR_REQ_QLEN_Pos)
#define MDMA_CH_SR_REQ_QLEN     MDMA_CH_SR_REQ_QLEN_Msk

#define MDMA_CH_SR_REQ_QOVERFLOW_Pos (4U)
#define MDMA_CH_SR_REQ_QOVERFLOW_Msk (0x01UL << MDMA_CH_SR_REQ_QLEN_Pos)
#define MDMA_CH_SR_REQ_QOVERFLOW     MDMA_CH_SR_REQ_QLEN_Msk

#define MDMA_NODE_SDEP_Pos (0U)
#define MDMA_NODE_SDEP_Msk (0xFFFFFFFFUL << MDMA_NODE_SDEP_Pos)
#define MDMA_NODE_SDEP     MDMA_NODE_SDEP_Msk

#define MDMA_NODE_DDEP_Pos (0U)
#define MDMA_NODE_DDEP_Msk (0xFFFFFFFFUL << MDMA_NODE_DDEP_Pos)
#define MDMA_NODE_DDEP     MDMA_NODE_DDEP_Msk

#define MDMA_NODE_CDC_CYCLECTRL_Pos (0U)
#define MDMA_NODE_CDC_CYCLECTRL_Msk (0x07UL << MDMA_NODE_CDC_CYCLECTRL_Pos)
#define MDMA_NODE_CDC_CYCLECTRL     MDMA_NODE_CDC_CYCLECTRL_Msk
#define MDMA_NODE_CDC_CYCLECTRL_0   (0x01UL << MDMA_NODE_CDC_CYCLECTRL_Pos)
#define MDMA_NODE_CDC_CYCLECTRL_1   (0x02UL << MDMA_NODE_CDC_CYCLECTRL_Pos)
#define MDMA_NODE_CDC_CYCLECTRL_2   (0x04UL << MDMA_NODE_CDC_CYCLECTRL_Pos)

#define MDMA_NODE_CDC_NMINUS_Pos (4U)
#define MDMA_NODE_CDC_NMINUS_Msk (0x3FFUL << MDMA_NODE_CDC_NMINUS_Pos)
#define MDMA_NODE_CDC_NMINUS     MDMA_NODE_CDC_NMINUS_Msk

#define MDMA_NODE_CDC_RPWR_Pos (14U)
#define MDMA_NODE_CDC_RPWR_Msk (0x0FUL << MDMA_NODE_CDC_RPWR_Pos)
#define MDMA_NODE_CDC_RPWR     MDMA_NODE_CDC_RPWR_Msk
#define MDMA_NODE_CDC_RPWR_0   (0x01UL << MDMA_NODE_CDC_RPWR_Pos)
#define MDMA_NODE_CDC_RPWR_1   (0x02UL << MDMA_NODE_CDC_RPWR_Pos)
#define MDMA_NODE_CDC_RPWR_2   (0x04UL << MDMA_NODE_CDC_RPWR_Pos)
#define MDMA_NODE_CDC_RPWR_3   (0x08UL << MDMA_NODE_CDC_RPWR_Pos)

#define MDMA_NODE_CDC_SSIZE_Pos (24U)
#define MDMA_NODE_CDC_SSIZE_Msk (0x03UL << MDMA_NODE_CDC_SSIZE_Pos)
#define MDMA_NODE_CDC_SSIZE     MDMA_NODE_CDC_SSIZE_Msk
#define MDMA_NODE_CDC_SSIZE_0   (0x01UL << MDMA_NODE_CDC_SSIZE_Pos)
#define MDMA_NODE_CDC_SSIZE_1   (0x02UL << MDMA_NODE_CDC_SSIZE_Pos)

#define MDMA_NODE_CDC_SINC_Pos (26U)
#define MDMA_NODE_CDC_SINC_Msk (0x03UL << MDMA_NODE_CDC_SINC_Pos)
#define MDMA_NODE_CDC_SINC     MDMA_NODE_CDC_SINC_Msk
#define MDMA_NODE_CDC_SINC_0   (0x01UL << MDMA_NODE_CDC_SINC_Pos)
#define MDMA_NODE_CDC_SINC_1   (0x02UL << MDMA_NODE_CDC_SINC_Pos)

#define MDMA_NODE_CDC_DSIZE_Pos (28U)
#define MDMA_NODE_CDC_DSIZE_Msk (0x03UL << MDMA_NODE_CDC_DSIZE_Pos)
#define MDMA_NODE_CDC_DSIZE     MDMA_NODE_CDC_DSIZE_Msk
#define MDMA_NODE_CDC_DSIZE_0   (0x01UL << MDMA_NODE_CDC_DSIZE_Pos)
#define MDMA_NODE_CDC_DSIZE_1   (0x02UL << MDMA_NODE_CDC_DSIZE_Pos)

#define MDMA_NODE_CDC_DINC_Pos (30U)
#define MDMA_NODE_CDC_DINC_Msk (0x03UL << MDMA_NODE_CDC_DINC_Pos)
#define MDMA_NODE_CDC_DINC     MDMA_NODE_CDC_DINC_Msk
#define MDMA_NODE_CDC_DINC_0   (0x01UL << MDMA_NODE_CDC_DINC_Pos)
#define MDMA_NODE_CDC_DINC_1   (0x02UL << MDMA_NODE_CDC_DINC_Pos)
    /** @endcond
     * @}
     */

    /* Exported types ------------------------------------------------------------*/
    /*! \brief
     *  MDMA channel configure structure definition
     */
    typedef struct
    {
        uint32_t NodeSelection; /*!< Specifies the transfer node for the MDMA channelx.
                                  This parameter can be a value of @ref MDMA_Node_selection */

        uint32_t Priority; /*!< Specifies the software priority for the MDMAy channelx.
                             This parameter can be a value of @ref MDMA_Priority_level */

        uint32_t Request; /*!< Specifies the MDMA request for the MDMAy channelx.
                            This parameter can be a value of @ref MDMA_Request_selection*/

        uint32_t TransferTriggerMode; /*!< Specifies the Trigger Transfer mode for the MDMA channelx.
                                        This parameter can be a value of @ref MDMA_Transfer_TriggerMode  */

    } MDMA_ChannelConfTypeDef;

    /*! \brief
     *  MDMA node configure structure definition
     */
    typedef struct
    {
        uint32_t NodeSelection; /*!< Specifies the transfer node for the MDMA channelx.
                                  This parameter can be a value of @ref MDMA_Node_selection */

        uint32_t SrcDataSize; /*!< Specifies the source data size.
                                This parameter can be a value of @ref MDMA_Src_Data_size */

        uint32_t DstDataSize; /*!< Specifies the destination data size.
                                This parameter can be a value of @ref MDMA_Dst_Data_size */

        uint32_t DataCount; /*!< Specifies the transfer data count.
                              This parameter can be a value of */

        uint32_t SrcAddress; /*!< Specifies the source data size.
                               This parameter can be a value of @ref  */

        uint32_t DstAddress; /*!< Specifies the destination data size.
                               This parameter can be a value of @ref MDMA_Data_size */

        uint32_t SrcIncrement; /*!< Specifies the Source increment mode.
                                 This parameter can be a value of @ref MDMA_Source_increment_mode */

        uint32_t DstIncrement; /*!< Specifies the destination increment mode.
                                 This parameter can be a value of @ref MDMA_Destination_increment_mode */

        uint32_t CycleCtrl; /*!< Specifies the transfer cycle control.
                              This parameter can be a value of @ref MDMA_Cycle_ctrl */
    } MDMA_NodeConfTypeDef;

    /**
     * @brief  HAL MDMA State structure definition
     */
    typedef enum
    {
        HAL_MDMA_STATE_RESET = 0x00U, /*!< MDMA not yet initialized or disabled */
        HAL_MDMA_STATE_READY = 0x01U, /*!< MDMA initialized and ready for use   */
        HAL_MDMA_STATE_BUSY  = 0x02U, /*!< MDMA process is ongoing              */
        HAL_MDMA_STATE_ERROR = 0x03U, /*!< MDMA error state                     */
        HAL_MDMA_STATE_ABORT = 0x04U, /*!< MDMA Abort state                     */

    } HAL_MDMA_StateTypeDef;

    /**
     * @brief  HAL MDMA Callbacks IDs structure definition
     */
    typedef enum
    {
        HAL_MDMA_XFER_CPLT_CB_ID = 0x00U, /*!< Transfer complete. */
    } HAL_MDMA_CallbackIDTypeDef;

    /**
     * @brief  MDMA handle Structure definition
     */
    typedef struct __MDMA_HandleTypeDef
    {
        MDMA_TypeDef        *Instance; /*!< Register base address                  */
        MDMA_ChannelTypeDef *Channel;  /*!< Register base address                  */

        MDMA_ChannelConfTypeDef ChannelConfig; /*!< Register base address                  */
        MDMA_NodeConfTypeDef    NodeConfig;    /*!< Register base address                  */

        __IO HAL_MDMA_StateTypeDef State; /*!< MDMA transfer state                    */

        void (*XferCpltCallback)(struct __MDMA_HandleTypeDef *hmdma); /*!< MDMA transfer complete callback        */

    } MDMA_HandleTypeDef;

/* Exported constants --------------------------------------------------------*/
/** @defgroup MDMA_Node_selection MDMA Error Codes
 * @brief    MDMA Node Selection
 * @{
 */
#define HAL_MDMA_PRIMARY_NODE   (0x00000000UL)
#define HAL_MDMA_ALTERNATE_NODE (0x00000001UL)

/**
 * @}
 */

/** @defgroup MDMA_Priority_level MDMA Priority level
 * @brief    MDMA Priority level
 * @{
 */
#define MDMA_PRIORITY_DEFAULT (0x00000000UL) /*!< Priority level: Default. */
#define MDMA_PRIORITY_HIGH    (0x00000001UL) /*!< Priority level: High.    */

/**
 * @}
 */

/** @defgroup MDMA_Request_selection MDMA Request selection
 * @brief    MDMA_Request_selection
 * @{
 */
#define MDMA_REQUEST_SW (0x40000000UL) /*!< MDMA SW request. */

/**
 * @}
 */

/** @defgroup MDMA_Transfer_TriggerMode MDMA Transfer Trigger mode
 * @brief    MDMA Transfer Trigger Mode
 * @{
 */
#define MDMA_SIGNAL_MODE (0x00000000UL)       /*!< ev signal as req.    */
#define MDMA_RISE_MODE   (MDMA_CH_REQ_CRMODE) /*!< ev rise edge as req. */

/**
 * @}
 */

/** @defgroup MDMA_Src_Data_size MDMA Source Data Size
 * @brief    MDMA Src Data Size
 * @{
 */
#define MDMA_SRC_DATASIZE_BYTE     (0x00000000UL)          /*!< Source data: byte.     */
#define MDMA_SRC_DATASIZE_HALFWORD (MDMA_NODE_CDC_SSIZE_0) /*!< Source data: halfword. */
#define MDMA_SRC_DATASIZE_WORD     (MDMA_NODE_CDC_SSIZE_1) /*!< Source data: word.     */

/**
 * @}
 */

/** @defgroup MDMA_Dst_Data_size MDMA Transfer Trigger Mode
 * @brief    MDMA Dst Data Size
 * @{
 */
#define MDMA_DST_DATASIZE_BYTE     (0x00000000UL)          /*!< Destination data: byte.     */
#define MDMA_DST_DATASIZE_HALFWORD (MDMA_NODE_CDC_DSIZE_0) /*!< Destination data: halfword. */
#define MDMA_DST_DATASIZE_WORD     (MDMA_NODE_CDC_DSIZE_1) /*!< Destination data: word.     */

/**
 * @}
 */

/** @defgroup MDMA_Source_increment_mode MDMA Source increment mode
 * @brief    MDMA Source Increment Mode
 * @{
 */
#define MDMA_SRC_INC_BYTE     (0x00000000UL)                                /*!< Source address pointer is incremented by a BYTE (8 bits)          */
#define MDMA_SRC_INC_HALFWORD (MDMA_NODE_CDC_SINC_0)                        /*!< Source address pointer is incremented by a half Word (16 bits)    */
#define MDMA_SRC_INC_WORD     (MDMA_NODE_CDC_SINC_1)                        /*!< Source address pointer is incremented by a Word (32 bits)         */
#define MDMA_SRC_INC_NOINC    (MDMA_NODE_CDC_SINC_0 | MDMA_NODE_CDC_SINC_1) /*!< Source address pointer is fixed                                   */

/**
 * @}
 */

/** @defgroup MDMA_Destination_increment_mode MDMA Destination increment mode
 * @brief    MDMA Destination Increment Mode
 * @{
 */
#define MDMA_DST_INC_BYTE     (0x00000000UL)                                /*!< Destination address pointer is incremented by a BYTE (8 bits)          */
#define MDMA_DST_INC_HALFWORD (MDMA_NODE_CDC_DINC_0)                        /*!< Destination address pointer is incremented by a half Word (16 bits)    */
#define MDMA_DST_INC_WORD     (MDMA_NODE_CDC_DINC_1)                        /*!< Destination address pointer is incremented by a Word (32 bits)         */
#define MDMA_DST_INC_NOINC    (MDMA_NODE_CDC_DINC_0 | MDMA_NODE_CDC_DINC_1) /*!< Destination address pointer is fixed                                   */

/**
 * @}
 */

/** @defgroup MDMA_Cycle_ctrl_mode MDMA Destination increment mode
 * @brief    MDMA Cycle Ctrl Mode
 * @{
 */
#define MDMA_CYCLECTRL_STOP (0x00000000UL) /*!< Indicates that the data structure is invalid. */
#define MDMA_CYCLECTRL_BASIC                                                     \
    (MDMA_NODE_CDC_CYCLECTRL_0) /*!< The controller must receive a new request,  \
                                   prior to it entering the arbitration process, \
                                   to enable the DMA cycle to complete. */
#define MDMA_CYCLECTRL_ATUOREQUEST                                             \
    (MDMA_NODE_CDC_CYCLECTRL_1) /*!< The controller automatically inserts a    \
                                request for the appropriate channel during     \
                                  the arbitration process. This means that the \
                                  initial request is sufficient to enable the  \
                                  DMA cycle to complete.       */
#define MDMA_CYCLECTRL_PINGPONG                                                                              \
    (MDMA_NODE_CDC_CYCLECTRL_1 | MDMA_NODE_CDC_CYCLECTRL_0) /*!< The controller performs a DMA cycle using   \
                                                               one of the data structures. After the DMA     \
                                                               cycle completes, it performs a DMA cycle      \
                                                               using the other data structure. After the DMA \
                                                               cycle completes and provided that the host    \
                                                               processor has updated the original data       \
                                                               structure, it performs a DMA cycle using the  \
                                                               original data structure. The controller       \
                                                               continues to perform DMA cycles until it      \
                                                               either reads an invalid data structure or the \
                                                               host processor changes the cycle_ctrl bits to \
                                                               b001 or b010. */
#define MDMA_CYCLECTRL_MEMORY_PRIMARY                                          \
    (MDMA_NODE_CDC_CYCLECTRL_2) /*!< When the controller operates in memory    \
                                   scatter-gather mode, you must only use this \
                                   value in the primary data structure.*/
#define MDMA_CYCLECTRL_MEMORY_ALTERNATE                                                                    \
    (MDMA_NODE_CDC_CYCLECTRL_2 | MDMA_NODE_CDC_CYCLECTRL_0) /*!< When the controller operates in memory    \
                                                               scatter-gather mode, you must only use this \
                                                               value in the alternate data structure.*/
#define MDMA_CYCLECTRL_PERIPHERAL_PRIMARY                                                                   \
    (MDMA_NODE_CDC_CYCLECTRL_2 | MDMA_NODE_CDC_CYCLECTRL_1) /*!< When the controller operates in peripheral \
                                                               scatter-gather mode, you must only use this  \
                                                               value in the primary data structure.*/
#define MDMA_CYCLECTRL_PERIPHERAL_ALTERNATE                                                                                             \
    (MDMA_NODE_CDC_CYCLECTRL_2 | MDMA_NODE_CDC_CYCLECTRL_1 | MDMA_NODE_CDC_CYCLECTRL_0) /*!< When the controller operates in peripheral \
                                                                                           scatter-gather mode, you must only use this  \
                                                                                           value in the alternate data structure.*/

    /**
     * @}
     */

    /* Exported macro ------------------------------------------------------------*/
    /* Exported functions --------------------------------------------------------*/
    /* Initialization and de-initialization functions *****************************/
    void              HAL_MDMA_IRQInit(MDMA_HandleTypeDef *hmdma);
    HAL_StatusTypeDef HAL_MDMA_Init(MDMA_HandleTypeDef *hmdma);
    HAL_StatusTypeDef HAL_MDMA_DeInit(MDMA_HandleTypeDef *hmdma);
    HAL_StatusTypeDef HAL_MDMA_ChannelConfig(MDMA_HandleTypeDef *hmdma, MDMA_ChannelConfTypeDef *ChannelConfig);
    HAL_StatusTypeDef HAL_MDMA_TransferNodeConfig(MDMA_HandleTypeDef *hmdma, MDMA_NodeConfTypeDef *NodeConfig);
    HAL_StatusTypeDef HAL_MDMA_RegisterCallback(MDMA_HandleTypeDef *hmdma, HAL_MDMA_CallbackIDTypeDef CallbackID, void (*pCallback)(MDMA_HandleTypeDef *_hmdma));
    HAL_StatusTypeDef HAL_MDMA_UnRegisterCallback(MDMA_HandleTypeDef *hmdma, HAL_MDMA_CallbackIDTypeDef CallbackID);
    /* IO operation functions *****************************************************/
    HAL_StatusTypeDef HAL_MDMA_Start(MDMA_HandleTypeDef *hmdma);
    HAL_StatusTypeDef HAL_MDMA_Start_IT(MDMA_HandleTypeDef *hmdma);
    HAL_StatusTypeDef HAL_MDMA_Abort(MDMA_HandleTypeDef *hmdma);
    HAL_StatusTypeDef HAL_MDMA_Abort_IT(MDMA_HandleTypeDef *hmdma);
    HAL_StatusTypeDef HAL_MDMA_GenerateSWRequest(MDMA_HandleTypeDef *hmdma);
    HAL_StatusTypeDef HAL_MDMA_PollForTransfer(MDMA_HandleTypeDef *hmdma, uint32_t Timeout);
    /* Peripheral State and Error functions ***************************************/
    HAL_MDMA_StateTypeDef HAL_MDMA_GetState(MDMA_HandleTypeDef *hmdma);

/** @cond Private macros
 * @{
 */
/* Private macros ------------------------------------------------------------*/
#define HAL_MDMA_LOCK()   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_MDMA_UNLOCK() HAL_UNLOCK

/****************** PWM Instances : All supported instances *******************/
#define IS_MDMA_INSTANCE(INSTANCE) ((INSTANCE) == MDMA0)

#define IS_MDMA_CHANNEL(CHANNEL)                                                                                                                                  \
    (((CHANNEL) == MDMA_CHANNEL0) || ((CHANNEL) == MDMA_CHANNEL1) || ((CHANNEL) == MDMA_CHANNEL2) || ((CHANNEL) == MDMA_CHANNEL3) || ((CHANNEL) == MDMA_CHANNEL4) \
     || ((CHANNEL) == MDMA_CHANNEL5) || ((CHANNEL) == MDMA_CHANNEL6) || ((CHANNEL) == MDMA_CHANNEL7))

#define IS_MDMA_NODE_SELECTON(__CONFIG__) (((__CONFIG__) == HAL_MDMA_PRIMARY_NODE) || ((__CONFIG__) == HAL_MDMA_ALTERNATE_NODE))

#define IS_MDMA_PRIORITY(__CONFIG__) (((__CONFIG) == MDMA_PRIORITY_DEFAULT) || ((__CONFIG) == MDMA_PRIORITY_HIGH))

#define IS_MDMA_REQUEST(__CONFIG__) (((__CONFIG__) == ((__CONFIG) == MDMA_PRIORITY_DEFAULT)))

#define IS_MDMA_TRANSFER_TRIGGER_MODE(__CONFIG__) (((__CONFIG__) == MDMA_SIGNAL_MODE) || ((__CONFIG__) == MDMA_RISE_MODE))

#define IS_MDMA_SRC_DATASIZE(__CONFIG__) (((__CONFIG__) == MDMA_SRC_DATASIZE_BYTE) || ((__CONFIG__) == MDMA_SRC_DATASIZE_HALFWORD) || ((__CONFIG__) == MDMA_SRC_DATASIZE_WORD))

#define IS_MDMA_DST_DATASIZE(__CONFIG__) (((__CONFIG__) == MDMA_DST_DATASIZE_BYTE) || ((__CONFIG__) == MDMA_DST_DATASIZE_HALFWORD) || ((__CONFIG__) == MDMA_DST_DATASIZE_WORD))

#define IS_MDMA_DATA_COUNT(__CONFIG__) (((__CONFIG__) >= 0) && ((__CONFIG__) <= 1024))

#define IS_MDMA_SRC_ADDRESS_DOMAIN(__CONFIG__)                                                                                                    \
    ((((__CONFIG__) >= 0) && ((__CONFIG__) <= 0)) || (((__CONFIG__) >= 0) && ((__CONFIG__) <= 0)) || (((__CONFIG__) >= 0) && ((__CONFIG__) <= 0)) \
     || (((__CONFIG__) >= 0) && ((__CONFIG__) <= 0)))

#define IS_MDMA_DST_ADDRESS_DOMAIN(__CONFIG__)                                                                                                    \
    ((((__CONFIG__) >= 0) && ((__CONFIG__) <= 0)) || (((__CONFIG__) >= 0) && ((__CONFIG__) <= 0)) || (((__CONFIG__) >= 0) && ((__CONFIG__) <= 0)) \
     || (((__CONFIG__) >= 0) && ((__CONFIG__) <= 0)))

#define IS_MDMA_NODE_ADDRESS_DOMAIN(__CONFIG__) \
    (((uint32_t)(__CONFIG__) >= 0x50000000) && (((uint32_t)(__CONFIG__) + sizeof(MDMA_NodeTypeDef) * MDMA_CHANNEL_COUNT * 2) <= 0x50040000) && ((__CONFIG__) % 0x100 == 0))

#define IS_MDMA_SOURCE_INC(__CONFIG__) \
    (((__CONFIG__) == MDMA_SRC_INC_BYTE) || ((__CONFIG__) == MDMA_SRC_INC_HALFWORD) || ((__CONFIG__) == MDMA_SRC_INC_WORD) || ((__CONFIG__) == MDMA_SRC_INC_NOINC))

#define IS_MDMA_DESTINATION_INC(__CONFIG__) \
    (((__CONFIG__) == MDMA_DST_INC_BYTE) || ((__CONFIG__) == MDMA_DST_INC_HALFWORD) || ((__CONFIG__) == MDMA_DST_INC_WORD) || ((__CONFIG__) == MDMA_DST_INC_NOINC))

#define IS_MDMA_CYCLE_CTRL(__CONFIG__)                                                                                                                                            \
    (((__CONFIG__) == MDMA_CYCLECTRL_STOP) || ((__CONFIG__) == MDMA_CYCLECTRL_BASIC) || ((__CONFIG__) == MDMA_CYCLECTRL_ATUOREQUEST) || ((__CONFIG__) == MDMA_CYCLECTRL_PINGPONG) \
     || ((__CONFIG__) == MDMA_CYCLECTRL_MEMORY_PRIMARY) || ((__CONFIG__) == MDMA_CYCLECTRL_MEMORY_ALTERNATE) || ((__CONFIG__) == MDMA_CYCLECTRL_PERIPHERAL_PRIMARY)               \
     || ((__CONFIG__) == MDMA_CYCLECTRL_PERIPHERAL_ALTERNATE))

#define __HAL_MDMA_GET_CHANNEL_MASK(CHANNEL) (0x1UL << (((uint32_t)CHANNEL - (uint32_t)MDMA_CHANNEL0) / 4))

#define __HAL_MDMA_ENABLE(INSTANCE, CHANNEL)                                                                \
    do                                                                                                      \
    {                                                                                                       \
        uint32_t channel_bit = ((uint32_t)CHANNEL - (uint32_t)MDMA_CHANNEL0) / 4;                           \
        (CHANNEL)->REQ |= (MDMA_CH_REQ_CREN | MDMA_CH_REQ_REQEN | MDMA_CH_REQ_SERQEN | MDMA_CH_REQ_WAITON); \
        (INSTANCE)->CH_ENSET |= (0x1UL << channel_bit);                                                     \
    } while (0)

#define __HAL_MDMA_DISABLE(INSTANCE, CHANNEL)                                                                \
    do                                                                                                       \
    {                                                                                                        \
        uint32_t channel_bit = ((uint32_t)CHANNEL - (uint32_t)MDMA_CHANNEL0) / 4;                            \
        (CHANNEL)->REQ &= ~(MDMA_CH_REQ_CREN | MDMA_CH_REQ_REQEN | MDMA_CH_REQ_SERQEN | MDMA_CH_REQ_WAITON); \
        (INSTANCE)->CH_ENCLR &= ~(0x1UL << channel_bit);                                                     \
    } while (0)

#define __HAL_MDMA_SWREQUEST(INSTANCE, CHANNEL)                                   \
    do                                                                            \
    {                                                                             \
        uint32_t channel_bit = ((uint32_t)CHANNEL - (uint32_t)MDMA_CHANNEL0) / 4; \
        (INSTANCE)->CH_SW_REQ |= (0x1UL << channel_bit);                          \
    } while (0)

#define __HAL_MDMA_REST_REGISTER(__HANDLE__)       \
    do                                             \
    {                                              \
        (__HANDLE__)->Instance->CFG           = 0; \
        (__HANDLE__)->Instance->CTRL_BASE_PTR = 0; \
        (__HANDLE__)->Instance->CH_SW_REQ     = 0; \
        (__HANDLE__)->Instance->CH_ENCLR      = 0; \
        (__HANDLE__)->Instance->CH_PRIALT_CLR = 0; \
        (__HANDLE__)->Instance->CH_PRIO_CLR   = 0; \
        (__HANDLE__)->Instance->CH_PRIO_CLR   = 0; \
        (__HANDLE__)->Channel->EVESEL         = 0; \
        (__HANDLE__)->Channel->REQ            = 0; \
    } while (0)

    /** @endcond
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /* DARIC_HAL_MDMA_H */