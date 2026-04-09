/**
 ******************************************************************************
 * @file    daric_hal_mdma.c
 * @author  MDMA Team
 * @brief   MDMA HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the Multi-channel Direct Memory Access (MDMA)
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
#include "daric_hal.h"

#ifdef HAL_MDMA_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* MDMA IRQ Priority define */
#ifdef HAL_MDMA_IRQ_PRIO
#define MDMA_IRQ_PRIO HAL_MDMA_IRQ_PRIO
#else
#define MDMA_IRQ_PRIO 0
#endif

#ifdef HAL_MDMA_IRQ_SUB_PRIO
#define MDMA_IRQ_SUB_PRIO HAL_MDMA_IRQ_SUB_PRIO
#else
#define MDMA_IRQ_SUB_PRIO 0
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
__IO static MDMA_NodeTypeDef *s_channel_ctrl = NULL;
#else
__IO static MDMA_NodeTypeDef s_channel_ctrl[MDMA_CHANNEL_COUNT * 2] __attribute__((aligned(0x100))) __attribute__((section("ifram")));
#endif
static uint32_t s_channel_status = 0;
/* Private functions ---------------------------------------------------------*/
static void MDMA_SetConfig(MDMA_TypeDef *Instance)
{
    Instance->CFG |= MDMA_CFG_ENABLE;
    Instance->CTRL_BASE_PTR = (uint32_t)s_channel_ctrl;
}

static void MDMA_ChannelConfig(MDMA_TypeDef *Instance, MDMA_ChannelTypeDef *Channel, MDMA_ChannelConfTypeDef *ChannelConfig)
{
    uint32_t chennel_bit = ((uint32_t)Channel - (uint32_t)MDMA_CHANNEL0) / 4;

    /* Control data structure setting */
    switch (ChannelConfig->NodeSelection)
    {
        case HAL_MDMA_PRIMARY_NODE:
            Instance->CH_PRIALT_CLR |= (1 << chennel_bit); // Selects the primary data structure for channel x.
            break;
        case HAL_MDMA_ALTERNATE_NODE:
            Instance->CH_PRIALT_SET |= (1 << chennel_bit); // Selects the alternate data structure for channel x.
            break;
    }

    /* Priority level setting */
    if (ChannelConfig->Priority == MDMA_PRIORITY_HIGH)
    {
        Instance->CH_PRIO_SET |= (1 << chennel_bit); // Channel x uses the high priority level.
    }
    else
    {
        Instance->CH_PRIO_CLR |= (1 << chennel_bit); // Channel x uses the default priority level.
    }

    /* Trigger source setting */
    Channel->EVESEL = ChannelConfig->Request;

    /* CR_Mode setting */
    switch (ChannelConfig->TransferTriggerMode)
    {
        case MDMA_SIGNAL_MODE:
            Channel->REQ &= ~MDMA_CH_REQ_CRMODE;
            break;
        case MDMA_RISE_MODE:
            Channel->REQ |= MDMA_CH_REQ_CRMODE;
            break;
    }
}

static void MDMA_NodeConfig(MDMA_ChannelTypeDef *Channel, MDMA_NodeConfTypeDef *NodeConfig)
{
    uint32_t          chennel_pos = ((uint32_t)Channel - (uint32_t)MDMA_CHANNEL0) / 4;
    uint32_t          src_data_end;
    uint32_t          dst_data_end;
    uint32_t          data_size;
    MDMA_NodeTypeDef *Node;

    assert_param((NodeConfig->DstDataSize == NodeConfig->SrcDataSize));

    data_size = NodeConfig->DataCount - 1;
    switch (NodeConfig->SrcIncrement)
    {
        case MDMA_SRC_INC_BYTE:
            src_data_end = NodeConfig->SrcAddress + data_size;
            break;
        case MDMA_SRC_INC_HALFWORD:
            src_data_end = NodeConfig->SrcAddress + (data_size << 1);
            break;
        case MDMA_SRC_INC_WORD:
            src_data_end = NodeConfig->SrcAddress + (data_size << 2);
            break;
        case MDMA_SRC_INC_NOINC:
            src_data_end = NodeConfig->SrcAddress;
            break;
    }

    switch (NodeConfig->DstIncrement)
    {
        case MDMA_DST_INC_BYTE:
            dst_data_end = NodeConfig->DstAddress + data_size;
            break;
        case MDMA_DST_INC_HALFWORD:
            dst_data_end = NodeConfig->DstAddress + (data_size << 1);
            break;
        case MDMA_DST_INC_WORD:
            dst_data_end = NodeConfig->DstAddress + (data_size << 2);
            break;
        case MDMA_DST_INC_NOINC:
            dst_data_end = NodeConfig->DstAddress;
            break;
    }

    assert_param(IS_MDMA_SRC_ADDRESS_DOMAIN(src_data_end));
    assert_param(IS_MDMA_DST_ADDRESS_DOMAIN(dst_data_end));
    assert_param(IS_MDMA_NODE_ADDRESS_DOMAIN(s_channel_ctrl));

    if (NodeConfig->NodeSelection == HAL_MDMA_PRIMARY_NODE)
    {
        Node = (MDMA_NodeTypeDef *)s_channel_ctrl + chennel_pos;
    }
    else
    {
        Node = (MDMA_NodeTypeDef *)s_channel_ctrl + MDMA_CHANNEL_COUNT + chennel_pos;
    }

    Node->SDEP = src_data_end;
    Node->DDEP = dst_data_end;

    Node->CDC
        = NodeConfig->DstIncrement | NodeConfig->SrcIncrement | NodeConfig->SrcDataSize | NodeConfig->DstDataSize | NodeConfig->CycleCtrl | (data_size << MDMA_NODE_CDC_NMINUS_Pos);
}

static void HAL_MDMA_IRQHandler(MDMA_HandleTypeDef *hmdma)
{
    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));

    /* User interrupt callback */
    if (hmdma->XferCpltCallback)
    {
        hmdma->XferCpltCallback(hmdma);
    }
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Initializes the MDMA according to the specified
 *         parameters in the MDMA_InitTypeDef and create the associated handle.
 * @param  hmdma: Pointer to a MDMA_HandleTypeDef structure that contains
 *         the configuration information for the specified MDMA Channel.
 * @retval None
 */
void HAL_MDMA_IRQInit(MDMA_HandleTypeDef *hmdma)
{
    HAL_MDMA_LOCK();
    HAL_NVIC_ConnectIRQ(MDMAIRQ_IRQn, MDMA_IRQ_PRIO, MDMA_IRQ_SUB_PRIO, (void *)HAL_MDMA_IRQHandler, (void *)hmdma, 0);
    HAL_MDMA_UNLOCK();
}

/**
 * @brief  Initializes the MDMA according to the specified
 *         parameters in the MDMA_InitTypeDef and create the associated handle.
 * @param  hmdma: Pointer to a MDMA_HandleTypeDef structure that contains
 *               the configuration information for the specified MDMA Channel.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_MDMA_Init(MDMA_HandleTypeDef *hmdma)
{
    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));
    assert_param(IS_MDMA_CHANNEL(hmdma->Channel));
    assert_param(IS_MDMA_PRIORITY(hmdma->ChannelConfig.Priority));
    assert_param(IS_MDMA_REQUEST(hmdma->ChannelConfig.Request));
    assert_param(IS_MDMA_TRANSFER_TRIGGER_MODE(hmdma->Init.TransferTriggerMode));
    assert_param(IS_MDMA_SRC_DATASIZE(hmdma->NodeConfig.SrcDataSize));
    assert_param(IS_MDMA_DST_DATASIZE(hmdma->NodeConfig.DstDataSize));
    assert_param(IS_MDMA_DATA_COUNT(hmdma->NodeConfig.DataCount));
    assert_param(IS_MDMA_SRC_ADDRESS_DOMAIN(hmdma->NodeConfig.SrcAddress));
    assert_param(IS_MDMA_DST_ADDRESS_DOMAIN(hmdma->NodeConfig.DstAddress));
    assert_param(IS_MDMA_SOURCE_INC(hmdma->NodeConfig.SourceInc));
    assert_param(IS_MDMA_DESTINATION_INC(hmdma->NodeConfig.DestinationInc));
    assert_param(IS_MDMA_CYCLE_CTRL(hmdma->NodeConfig.CycleCtrl));

    /* Allocate lock resource */
    HAL_MDMA_LOCK();

    /* Malloc channel control data structure */
    if (!s_channel_status)
    {
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
        s_channel_ctrl = (MDMA_NodeTypeDef *)IframMgr_MallocAlign(sizeof(MDMA_NodeTypeDef) * MDMA_CHANNEL_COUNT * 2, 0x100);
        if (!s_channel_ctrl)
        {
            HAL_MDMA_UNLOCK();
            return HAL_ERROR;
        }
#endif
    }

    /* Change MDMA peripheral state */
    hmdma->State = HAL_MDMA_STATE_BUSY;

    /* Disable the MDMA channel */
    __HAL_MDMA_DISABLE(hmdma->Instance, hmdma->Channel);

    /* Initialize the MDMA channel registers */
    MDMA_SetConfig(hmdma->Instance);
    MDMA_ChannelConfig(hmdma->Instance, hmdma->Channel, &hmdma->ChannelConfig);
    MDMA_NodeConfig(hmdma->Channel, &hmdma->NodeConfig);

    /* Initialize the MDMA state */
    hmdma->State = HAL_MDMA_STATE_READY;

    /* Set channel usage status */
    s_channel_status |= __HAL_MDMA_GET_CHANNEL_MASK(hmdma->Channel);

    HAL_MDMA_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  DeInitializes the MDMA peripheral
 * @param  hmdma: pointer to a MDMA_HandleTypeDef structure that contains
 *               the configuration information for the specified MDMA Channel.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_MDMA_DeInit(MDMA_HandleTypeDef *hmdma)
{
    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));
    assert_param(IS_MDMA_CHANNEL(hmdma->Channel));

    /* Allocate lock resource */
    HAL_MDMA_LOCK();

    /* Change MDMA peripheral state */
    hmdma->State = HAL_MDMA_STATE_BUSY;

    /* Reset channel usage status */
    s_channel_status &= ~(__HAL_MDMA_GET_CHANNEL_MASK(hmdma->Channel));

    if (!s_channel_status)
    {
        /* Disable the interrupt */
        HAL_NVIC_DisableIRQ(MDMAIRQ_IRQn);
/* Free channel control data structure */
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
        if (s_channel_ctrl)
        {
            IframMgr_Free((void *)s_channel_ctrl);
            s_channel_ctrl = NULL;
        }
#endif
    }

    /* Disable the selected MDMA Channelx */
    __HAL_MDMA_DISABLE(hmdma->Instance, hmdma->Channel);

    /* Reset MDMA Channel control register */
    __HAL_MDMA_REST_REGISTER(hmdma);

    /* Initialize the MDMA state */
    hmdma->State = HAL_MDMA_STATE_RESET;

    /* Release Lock */
    HAL_MDMA_UNLOCK();

    return HAL_OK;
}

/**
 * @brief Configures the channel.
 * @param hmdma pointer to a MDMA_HandleTypeDef structure that contains
 *               the configuration information for the specified MDMA Channel.
 * @param ChannelConfig pointer to a MDMA_ChannelConfTypeDef structure that
 * contains the configuration information for the specified MDMA channel
 * configuration.
 * @return
 */
HAL_StatusTypeDef HAL_MDMA_ChannelConfig(MDMA_HandleTypeDef *hmdma, MDMA_ChannelConfTypeDef *ChannelConfig)
{
    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }
    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));
    assert_param(IS_MDMA_CHANNEL(hmdma->Channel));
    assert_param(IS_MDMA_PRIORITY(hmdma->ChannelConfig.Priority));
    assert_param(IS_MDMA_REQUEST(hmdma->ChannelConfig.Request));
    assert_param(IS_MDMA_TRANSFER_TRIGGER_MODE(hmdma->Init.TransferTriggerMode));

    /* Allocate lock resource */
    HAL_MDMA_LOCK();

    /* Change MDMA peripheral state */
    hmdma->State = HAL_MDMA_STATE_BUSY;

    /* Disable the selected MDMA Channelx */
    __HAL_MDMA_DISABLE(hmdma->Instance, hmdma->Channel);

    /* Reset MDMA Channel control register */
    MDMA_ChannelConfig(hmdma->Instance, hmdma->Channel, ChannelConfig);

    /* Initialize the MDMA state */
    hmdma->State = HAL_MDMA_STATE_READY;

    /* Release Lock */
    HAL_MDMA_UNLOCK();

    return HAL_OK;
}

/**
 * @brief Configures the transfer node.
 * @param hmdma pointer to a MDMA_HandleTypeDef structure that contains
 *               the configuration information for the specified MDMA Channel.
 * @param NodeConfig pointer to a MDMA_ChannelConfTypeDef structure that
 * contains the configuration information for the specified MDMA node
 * configuration.
 * @return
 */
HAL_StatusTypeDef HAL_MDMA_TransferNodeConfig(MDMA_HandleTypeDef *hmdma, MDMA_NodeConfTypeDef *NodeConfig)
{
    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));
    assert_param(IS_MDMA_CHANNEL(hmdma->Channel));
    assert_param(IS_MDMA_SRC_DATASIZE(hmdma->NodeConfig.SrcDataSize));
    assert_param(IS_MDMA_DST_DATASIZE(hmdma->NodeConfig.DstDataSize));
    assert_param(IS_MDMA_DATA_COUNT(hmdma->NodeConfig.DataCount));
    assert_param(IS_MDMA_SRC_ADDRESS_DOMAIN(hmdma->NodeConfig.SrcAddress));
    assert_param(IS_MDMA_DST_ADDRESS_DOMAIN(hmdma->NodeConfig.DstAddress));
    assert_param(IS_MDMA_SOURCE_INC(hmdma->NodeConfig.SourceInc));
    assert_param(IS_MDMA_DESTINATION_INC(hmdma->NodeConfig.DestinationInc));
    assert_param(IS_MDMA_CYCLE_CTRL(hmdma->NodeConfig.CycleCtrl));

    /* Allocate lock resource */
    HAL_MDMA_LOCK();

    /* Change MDMA peripheral state */
    hmdma->State = HAL_MDMA_STATE_BUSY;

    /* Disable the selected MDMA Channelx */
    __HAL_MDMA_DISABLE(hmdma->Instance, hmdma->Channel);

    /* Initialize the MDMA channel registers */
    MDMA_NodeConfig(hmdma->Channel, NodeConfig);

    /* Initialize the MDMA state */
    hmdma->State = HAL_MDMA_STATE_READY;

    /* Release Lock */
    HAL_MDMA_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Register callbacks
 * @param  hmdma:                pointer to a MDMA_HandleTypeDef structure that
 * contains the configuration information for the specified MDMA Channel.
 * @param  CallbackID:           User Callback identifier
 * @param  pCallback:            pointer to callbacsk function.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_MDMA_RegisterCallback(MDMA_HandleTypeDef *hmdma, HAL_MDMA_CallbackIDTypeDef CallbackID, void (*pCallback)(MDMA_HandleTypeDef *_hmdma))
{
    HAL_StatusTypeDef status = HAL_OK;

    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));
    assert_param(IS_MDMA_CHANNEL(hmdma->Channel));

    /* Process locked */
    HAL_MDMA_LOCK();

    if (HAL_MDMA_STATE_READY == hmdma->State)
    {
        switch (CallbackID)
        {
            case HAL_MDMA_XFER_CPLT_CB_ID:
                hmdma->XferCpltCallback = pCallback;
                break;

            default:
                break;
        }
    }
    else
    {
        /* Return error status */
        status = HAL_ERROR;
    }

    /* Release Lock */
    HAL_MDMA_UNLOCK();

    return status;
}

/**
 * @brief  UnRegister callbacks
 * @param  hmdma:                 pointer to a MDMA_HandleTypeDef structure that
 * contains the configuration information for the specified MDMA Channel.
 * @param  CallbackID:           User Callback identifier
 *                               a HAL_MDMA_CallbackIDTypeDef ENUM as parameter.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_MDMA_UnRegisterCallback(MDMA_HandleTypeDef *hmdma, HAL_MDMA_CallbackIDTypeDef CallbackID)
{
    HAL_StatusTypeDef status = HAL_OK;

    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));
    assert_param(IS_MDMA_CHANNEL(hmdma->Channel));

    /* Process locked */
    HAL_MDMA_LOCK();

    if (HAL_MDMA_STATE_READY == hmdma->State)
    {
        switch (CallbackID)
        {
            case HAL_MDMA_XFER_CPLT_CB_ID:
                hmdma->XferCpltCallback = NULL;
                break;

            default:
                status = HAL_ERROR;
                break;
        }
    }
    else
    {
        status = HAL_ERROR;
    }

    /* Release Lock */
    HAL_MDMA_UNLOCK();

    return status;
}

/**
 * @brief Starts the MDMA Transfer.
 * @param hmdma pointer to a MDMA_HandleTypeDef structure that
 * contains the configuration information for the specified MDMA Channel.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_MDMA_Start(MDMA_HandleTypeDef *hmdma)
{
    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));
    assert_param(IS_MDMA_CHANNEL(hmdma->Channel));

    /* Process locked */
    HAL_MDMA_LOCK();

    if (HAL_MDMA_STATE_READY == hmdma->State)
    {
        /* Change MDMA peripheral state */
        hmdma->State = HAL_MDMA_STATE_BUSY;

        /* Enable the Channel */
        __HAL_MDMA_ENABLE(hmdma->Instance, hmdma->Channel);

        if (hmdma->ChannelConfig.Request == MDMA_REQUEST_SW)
        {
            /* activate If SW request mode*/
            __HAL_MDMA_SWREQUEST(hmdma->Instance, hmdma->Channel);
        }
    }
    else
    {
        /* Process unlocked */
        HAL_MDMA_UNLOCK();

        /* Return error status */
        return HAL_BUSY;
    }

    /* Release Lock */
    HAL_MDMA_UNLOCK();

    return HAL_OK;
}

/**
 * @brief Starts the MDMA Transfer with interrupts enabled.
 * @param hmdma pointer to a MDMA_HandleTypeDef structure that
 * contains the configuration information for the specified MDMA Channel.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_MDMA_Start_IT(MDMA_HandleTypeDef *hmdma)
{
    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_MDMA_INSTANCE(hmdma->Instance));
    assert_param(IS_MDMA_CHANNEL(hmdma->Channel));

    /* Process locked */
    HAL_MDMA_LOCK();

    if (HAL_MDMA_STATE_READY == hmdma->State)
    {
        /* Change MDMA peripheral state */
        hmdma->State = HAL_MDMA_STATE_BUSY;

        /* Enable the channel */
        __HAL_MDMA_ENABLE(hmdma->Instance, hmdma->Channel);

        /* Clear the interrupt flag */
        HAL_NVIC_ClearPendingIRQ(MDMAIRQ_IRQn);

        /* Enable the interrupt */
        if (hmdma->XferCpltCallback)
        {
            HAL_NVIC_EnableIRQ(MDMAIRQ_IRQn);
        }

        if (hmdma->ChannelConfig.Request == MDMA_REQUEST_SW)
        {
            /* activate If SW request mode*/
            __HAL_MDMA_SWREQUEST(hmdma->Instance, hmdma->Channel);
        }
    }
    else
    {
        /* Process unlocked */
        HAL_MDMA_UNLOCK();

        /* Return error status */
        return HAL_BUSY;
    }

    /* Process unlocked */
    HAL_MDMA_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Aborts the MDMA Transfer.
 * @param  hmdma  : pointer to a MDMA_HandleTypeDef structure that contains
 *                 the configuration information for the specified MDMA Channel.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_MDMA_Abort(MDMA_HandleTypeDef *hmdma)
{
    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_MDMA_LOCK();

    if (HAL_MDMA_STATE_BUSY != hmdma->State)
    {
        /* Process Unlocked */
        HAL_MDMA_UNLOCK();

        return HAL_ERROR;
    }
    else
    {
        /* Disable the channel */
        __HAL_MDMA_DISABLE(hmdma->Instance, hmdma->Channel);

        /* Check if the MDMA Channel is effectively disabled */

        /* Change the MDMA state*/
        hmdma->State = HAL_MDMA_STATE_READY;
    }

    /* Process Unlocked */
    HAL_MDMA_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Aborts the MDMA Transfer in Interrupt mode.
 * @param  hmdma  : pointer to a MDMA_HandleTypeDef structure that contains
 *                 the configuration information for the specified MDMA Channel.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_MDMA_Abort_IT(MDMA_HandleTypeDef *hmdma)
{
    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_MDMA_LOCK();

    if (HAL_MDMA_STATE_BUSY != hmdma->State)
    {
        /* Process Unlocked */
        HAL_MDMA_UNLOCK();

        return HAL_ERROR;
    }
    else
    {
        /* Disable the stream */
        __HAL_MDMA_DISABLE(hmdma->Instance, hmdma->Channel);

        HAL_NVIC_DisableIRQ(MDMAIRQ_IRQn);

        /* Check if the MDMA Channel is effectively disabled */

        /* Set Abort State  */
        hmdma->State = HAL_MDMA_STATE_ABORT;
    }

    /* Process Unlocked */
    HAL_MDMA_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Generate an MDMA SW request trigger to activate the request on the
 * given Channel.
 * @param  hmdma:       pointer to a MDMA_HandleTypeDef structure that contains
 *                     the configuration information for the specified MDMA
 * Stream.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_MDMA_GenerateSWRequest(MDMA_HandleTypeDef *hmdma)
{
    uint32_t channel_mask;

    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    HAL_MDMA_LOCK();

    channel_mask = (0x1UL << (((uint32_t)hmdma->Channel - (uint32_t)MDMA_CHANNEL0) / 4));

    if (((hmdma->Instance->STAT & MDMA_STAT_ENABLE) == 0U) || (((hmdma->Instance->CH_ENSET & channel_mask) == 0U)))
    {
        /* if no Transfer on going (MDMA enable chennel_bit not set) return error */
        HAL_MDMA_UNLOCK();

        return HAL_ERROR;
    }
    else if (!(hmdma->Instance->CH_WREQ_STAT & channel_mask) || (hmdma->ChannelConfig.Request != MDMA_REQUEST_SW))
    {
        /* if an MDMA ongoing request has not yet end or if request mode is not SW
         * request return error */
        HAL_MDMA_UNLOCK();

        return HAL_ERROR;
    }
    else
    {
        /* Set the SW request chennel_bit to activate the request on the Channel */
        __HAL_MDMA_SWREQUEST(hmdma->Instance, hmdma->Channel);

        HAL_MDMA_UNLOCK();

        return HAL_OK;
    }
}

/**
 * @brief Polling for transfer complete.
 * @param hmdma pointer to a MDMA_HandleTypeDef structure that
 * contains the configuration information for the specified MDMA Channel.
 * @param Timeout unit: ms.
 * @return
 */
HAL_StatusTypeDef HAL_MDMA_PollForTransfer(MDMA_HandleTypeDef *hmdma, uint32_t Timeout)
{
    uint32_t channel_mask;
    uint32_t tickstart;

    /* Check the MDMA peripheral handle */
    if (hmdma == NULL)
    {
        return HAL_ERROR;
    }

    channel_mask = (0x1UL << (((uint32_t)hmdma->Channel - (uint32_t)MDMA_CHANNEL0) / 4));

    tickstart = HAL_GetMs();

    if (HAL_MDMA_STATE_BUSY != hmdma->State)
    {
        return HAL_ERROR;
    }
    else
    {
        while ((hmdma->Instance->CH_ENSET & channel_mask))
        {
            if (HAL_GetMs() - tickstart >= Timeout)
            {
                HAL_MDMA_Abort(hmdma);
                return HAL_TIMEOUT;
            }
        }

        return HAL_OK;
    }
}

/**
 * @brief  Returns the MDMA state.
 * @param  hmdma: pointer to a MDMA_HandleTypeDef structure that contains
 *               the configuration information for the specified MDMA Channel.
 * @retval HAL state
 */
HAL_MDMA_StateTypeDef HAL_MDMA_GetState(MDMA_HandleTypeDef *hmdma)
{
    return hmdma->State;
}

#endif /* HAL_MDMA_MODULE_ENABLED */
