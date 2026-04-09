/**
 ******************************************************************************
 * @file    daric_hal_sd.c
 * @author  SD Team
 * @brief   SD HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the Secure Digital Card(SDCard)
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
#include "daric_hal_sd.h"
#include "daric_hal.h"
#include "daric_hal_sdio.h"
#include <stdbool.h>
#include <string.h>

/** @addtogroup SD
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @addtogroup SD_Private_Defines
 * @{
 */
#define SDIO_RX_CH_IRQ  (112)
#define SDIO_TX_CH_IRQ  (113)
#define SDIO_EOT_CH_IRQ (114)
#define SDIO_ERR_CH_IRQ (115)
/**
 * @}
 */

/* SDIO IRQ Priority define */
#ifdef HAL_SD_IRQ_PRIO
#define SD_IRQ_PRIO HAL_SD_IRQ_PRIO
#else
#define SD_IRQ_PRIO 0
#endif

#ifdef HAL_SD_IRQ_SUB_PRIO
#define SD_IRQ_SUB_PRIO HAL_SD_IRQ_SUB_PRIO
#else
#define SD_IRQ_SUB_PRIO 0
#endif

/* Private macro -------------------------------------------------------------*/
#define SD_TRANS_BLOCK_MAX (16)

#define HAL_SD_LOCK()   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_SD_UNLOCK() HAL_UNLOCK
/* Private variables ---------------------------------------------------------*/
static SDIO_HandleTypeDef gSD_SDIO_handle[SD_NUM_MAX] = { 0 };

/* Private functions ---------------------------------------------------------*/
/** @defgroup SD_Private_Functions SD Private Functions
 * @{
 */
static uint32_t SD_InitCard(SD_HandleTypeDef *hsd);
static uint32_t SD_PowerON(SD_HandleTypeDef *hsd);
static uint32_t SD_SendSDStatus(SD_HandleTypeDef *hsd, uint32_t *pSDstatus);
static uint32_t SD_SendStatus(SD_HandleTypeDef *hsd, uint32_t *pCardStatus);
static uint32_t SD_WideBus_Enable(SD_HandleTypeDef *hsd);
static uint32_t SD_WideBus_Disable(SD_HandleTypeDef *hsd);
// static uint32_t SD_FindSCR(SD_HandleTypeDef *hsd, uint32_t *pSCR);
static void HAL_SD_RxIRQHandler(const void *arg);
static void HAL_SD_TxIRQHandler(const void *arg);
static void HAL_SD_EotIRQHandler(const void *arg);
static void HAL_SD_ErrorIRQHandler(const void *arg);

/**
 * @}
 */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SD_Exported_Functions
 * @{
 */

/** @addtogroup SD_Exported_Functions_Group1
 *  @brief      Initialization and de-initialization functions
 *
@verbatim
  ==============================================================================
          ##### Initialization and de-initialization functions #####
  ==============================================================================
  [..]
    This section provides functions allowing to initialize/de-initialize the SD
    card device to be ready for use.

@endverbatim
  * @{
  */

/**
 * @brief Initializes the SD card interface and hardware resources.
 *        Configures the SDIO peripheral, sets up interrupts, and performs
 *        the necessary steps to initialize the SD card. Validates the
 *        card's voltage and readiness for data transfer.
 *
 * @param hsd : Pointer to the SD handle structure.
 *
 * @retval HAL_StatusTypeDef : HAL_OK on successful initialization,
 *                             HAL_ERROR if initialization fails.
 */
HAL_StatusTypeDef HAL_SD_Init(SD_HandleTypeDef *hsd)
{
    SDIO_InitTypeDef Init       = { 0 };
    uint32_t         errorstate = HAL_SD_ERROR_NONE;

    /* Check the SD handle allocation */
    if (hsd == NULL || !(hsd->ID < SDIO_MAX_NUM))
    {
        return HAL_ERROR;
    }

    hsd->Instance = &gSD_SDIO_handle[hsd->ID];

    /* Initialize the Card parameters */
    /* Default SDIO peripheral configuration for SD card initialization */
    Init.BusWide = SDIO_BUS_WIDE_1B;
    Init.Clock   = SDIO_CLK_400K;

    /* Initialize SDIO peripheral interface with default configuration */
    HAL_SDIO_Init(hsd->Instance, Init);

    /* register SDIO IRQ NUM */
    HAL_NVIC_ConnectIRQ(SDIO_RX_CH_IRQ, SD_IRQ_PRIO, SD_IRQ_SUB_PRIO, HAL_SD_RxIRQHandler, (void *)hsd, 0);
    HAL_NVIC_ConnectIRQ(SDIO_TX_CH_IRQ, SD_IRQ_PRIO, SD_IRQ_SUB_PRIO, HAL_SD_TxIRQHandler, (void *)hsd, 0);
    HAL_NVIC_ConnectIRQ(SDIO_EOT_CH_IRQ, SD_IRQ_PRIO, SD_IRQ_SUB_PRIO, HAL_SD_EotIRQHandler, (void *)hsd, 0);
    HAL_NVIC_ConnectIRQ(SDIO_ERR_CH_IRQ, SD_IRQ_PRIO, SD_IRQ_SUB_PRIO, HAL_SD_ErrorIRQHandler, (void *)hsd, 0);

    /* Identify card operating voltage */
    errorstate = SD_PowerON(hsd);
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        hsd->State = errorstate;
        hsd->ErrorCode |= errorstate;
        printf("%s: SD_PowerON Error!\n", __func__);
        return HAL_ERROR;
    }

    /* Card initialization */
    errorstate = SD_InitCard(hsd);
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        hsd->State = errorstate;
        hsd->ErrorCode |= errorstate;
        printf("%s: SD_InitCard Error!\n", __func__);
        return HAL_ERROR;
    }

    /* Initialize the error code */
    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    /* Initialize the SD operation */
    hsd->Context = SD_CONTEXT_NONE;

    /* Initialize the SD state */
    hsd->State = HAL_SD_STATE_READY;

    return HAL_OK;
}

/**
 * @brief Deinitializes the SD card interface and hardware resources.
 *        This function ensures the SD card is in an idle state, stops any
 *        ongoing operations, and resets the peripheral and its configurations.
 *        Card-related information is cleared, and the SD state is reset.
 *
 * @param hsd : Pointer to the SD handle structure.
 *
 * @retval HAL_StatusTypeDef : HAL_OK on success, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef HAL_SD_DeInit(SD_HandleTypeDef *hsd)
{
    int timeout = 0;
    int count   = 0;

    /* Check the parameters */
    if (hsd == NULL)
    {
        return HAL_ERROR;
    }

    if (hsd->State != HAL_SD_ERROR_NONE)
    {
        hsd->State = HAL_SD_STATE_BUSY;

        /* 1. Confirm SD Card is Free */
        while (1)
        {
            if (HAL_SD_GetCardState(hsd) == HAL_SD_CARD_TRANSFER)
            {
                printf("SD Card is Free!\n");
                break;
            }
            else
            {
                if (timeout == 500)
                {
                    if (count == 3)
                    {
                        printf("CMD12 Stop Transfer Failed!\n");
                        break;
                    }
                    printf("Force exit transmission mode\n");
                    HAL_SDMMC_CmdStopTransfer(hsd->Instance);
                    timeout = 0;
                    count++;
                    continue;
                }
                timeout++;
                HAL_Delay(1);
            }
        }

        /* 2. Send CMD7 to DeSelect the Card */
        HAL_SDMMC_CmdSelDesel(hsd->Instance, (uint32_t)(((uint32_t)hsd->SdCard.RelCardAdd) << 16U));
    }

    /* 3. SDIO Deinit */
    HAL_SDIO_DeInit(hsd->Instance);

    /* Clear SD Card Info */
    memset(&hsd->SdCard, 0, sizeof(HAL_SD_CardInfoTypeDef));
    memset(hsd->CSD, 0, sizeof(hsd->CSD));
    memset(hsd->CID, 0, sizeof(hsd->CID));

    hsd->ErrorCode = HAL_SD_ERROR_NONE;

    hsd->State = HAL_SD_STATE_RESET;

    printf("Please Securely Removable SDCard\n");

    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup SD_Exported_Functions_Group2
 *  @brief      Register/Unregister ISR Function
 *
@verbatim
  ==============================================================================
                ##### Register/Unregister ISR Function #####
  ==============================================================================
  [..]
    This section provides functions for user to register and unregister ISR

@endverbatim
 * @{
 */
/**
 * @brief  Register a User SD Callback
 *
 * @param hsd : Pointer to the SD handle structure.
 * @param CallbackId : Id of the callback to be registered
 *        This parameter can be one of the following values:
 *          @arg @ref HAL_SD_TX_CPLT_CB_ID    SD Tx Complete Callback ID
 *          @arg @ref HAL_SD_RX_CPLT_CB_ID    SD Rx Complete Callback ID
 *          @arg @ref HAL_SD_EOT_CB_ID        SD End of Transfer Callback ID
 *          @arg @ref HAL_SD_ERROR_CB_ID      SD Error Callback ID
 * @param pCallback : pointer to the Callback function

 * @retval HAL_StatusTypeDef : HAL_OK on success, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef HAL_SD_RegisterCallback(SD_HandleTypeDef *hsd, HAL_SD_CallbackIDTypeDef CallbackId, pSD_CallbackTypeDef pCallback)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (pCallback == NULL)
    {
        /* Update the error code */
        hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
        return HAL_ERROR;
    }

    HAL_SD_LOCK();

    switch (CallbackId)
    {
        case HAL_SD_TX_CPLT_CB_ID:
            hsd->TxCpltCallback = pCallback;
            break;
        case HAL_SD_RX_CPLT_CB_ID:
            hsd->RxCpltCallback = pCallback;
            break;
        case HAL_SD_EOT_CB_ID:
            hsd->EotCallback = pCallback;
            break;
        case HAL_SD_ERROR_CB_ID:
            hsd->ErrorCallback = pCallback;
            break;
        default:
            /* Update the error code */
            hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
            /* update return status */
            status = HAL_ERROR;
            break;
    }

    HAL_SD_UNLOCK();

    return status;
}

/**
 * @brief  Unregister a User SD Callback
 *         SD Callback is redirected to the weak (surcharged) predefined
 * callback
 *
 * @param hsd : Pointer to the SD handle structure.
 * @param CallbackId : Id of the callback to be unregistered
 *        This parameter can be one of the following values:
 *          @arg @ref HAL_SD_TX_CPLT_CB_ID    SD Tx Complete Callback ID
 *          @arg @ref HAL_SD_RX_CPLT_CB_ID    SD Rx Complete Callback ID
 *          @arg @ref HAL_SD_EOT_CB_ID        SD End of Transfer Callback ID
 *          @arg @ref HAL_SD_ERROR_CB_ID      SD Error Callback ID
 *
 * @retval HAL_StatusTypeDef : HAL_OK on success, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef HAL_SD_UnRegisterCallback(SD_HandleTypeDef *hsd, HAL_SD_CallbackIDTypeDef CallbackId)
{
    HAL_StatusTypeDef status = HAL_OK;

    HAL_SD_LOCK();

    switch (CallbackId)
    {
        case HAL_SD_TX_CPLT_CB_ID:
            hsd->TxCpltCallback = NULL;
            break;
        case HAL_SD_RX_CPLT_CB_ID:
            hsd->RxCpltCallback = NULL;
            break;
        case HAL_SD_EOT_CB_ID:
            hsd->EotCallback = NULL;
            break;
        case HAL_SD_ERROR_CB_ID:
            hsd->ErrorCallback = NULL;
            break;
        default:
            /* Update the error code */
            hsd->ErrorCode |= HAL_SD_ERROR_INVALID_CALLBACK;
            /* update return status */
            status = HAL_ERROR;
            break;
    }

    HAL_SD_UNLOCK();

    return status;
}

/** @addtogroup SD_Exported_Functions_Group3
 *  @brief      Input and Output operation Functions
 *
@verbatim
  ==============================================================================
                ##### Input and Output operation Functions #####
  ==============================================================================
  [..]
    This subsection provides a set of functions allowing to manage the data
    transfer from/to SD card.
@endverbatim
  * @{
  */

/**
 * @brief Reads multiple or single blocks of data from an SD card in polling
 * mode. This function configures the SDIO interface for data transmission,
 *        sends the appropriate read command, and waits for the data to be
 * received into the RXFIFO. The function then reads the data from the FIFO and
 *        stores it in the provided buffer.
 *
 * @param hsd : Pointer to the SD handle structure.
 * @param pData : Pointer to the buffer where the read data will be stored.
 * @param BlockAdd : Block address to start reading from.
 * @param NumberOfBlocks : Number of blocks to read.
 * @param Timeout : Timeout value in milliseconds for the read operation.
 *
 * @retval HAL_StatusTypeDef : HAL_OK if the operation is successful,
 *                             HAL_ERROR if an error occurs,
 *                             HAL_TIMEOUT if the read operation times out.
 */
HAL_StatusTypeDef HAL_SD_ReadBlocks(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout)
{
    SDIO_DataInitTypeDef config;
    uint32_t             errorstate = HAL_SD_ERROR_NONE;
    // uint32_t tickstart = HAL_GetTick();
    uint32_t *tempbuff = (uint32_t *)pData;

    if (NULL == pData)
    {
        hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
        return HAL_ERROR;
    }

    HAL_SD_LOCK();

    if (hsd->State == HAL_SD_STATE_READY)
    {
        hsd->ErrorCode = HAL_SD_ERROR_NONE;

        if ((BlockAdd + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
        {
            hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            HAL_SD_UNLOCK();
            return HAL_ERROR;
        }

        hsd->State = HAL_SD_STATE_BUSY;

        HAL_SD_UNLOCK();

        HAL_NVIC_DisableIRQ(SDIO_RX_CH_IRQ);

        if (hsd->SdCard.CardType != CARD_SDHC_SDXC)
        {
            BlockAdd *= 512U;
        }

        config.DataLength    = NumberOfBlocks * BLOCKSIZE;
        config.DataBlockSize = BLOCKSIZE;
        config.TransferDir   = SDIO_TRANSFER_DIR_TO_SDIO;
        config.TransferMode  = SDIO_TRANSFER_MODE_BLOCK;
        HAL_SDIO_ConfigData(hsd->Instance, &config);

        errorstate = HAL_SDIO_RXFIFO_Setup(hsd->Instance, NumberOfBlocks * BLOCKSIZE);

        /* Read block(s) in polling mode */
        if (NumberOfBlocks > 1U)
        {
            hsd->Context = SD_CONTEXT_READ_MULTIPLE_BLOCK;

            /* Read Multi Block command */
            errorstate = HAL_SDMMC_CmdReadMultiBlock(hsd->Instance, BlockAdd);
        }
        else
        {
            hsd->Context = SD_CONTEXT_READ_SINGLE_BLOCK;

            /* Read Single Block command */
            errorstate = HAL_SDMMC_CmdReadSingleBlock(hsd->Instance, BlockAdd);
        }
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            hsd->ErrorCode |= errorstate;
            hsd->State = HAL_SD_STATE_READY;
            return HAL_ERROR;
        }

        errorstate = HAL_SDIO_WaitRX(hsd->Instance, 5000);
        if (errorstate == HAL_TIMEOUT)
        {
            printf("Read Block Timeout!\n");
            hsd->ErrorCode = HAL_SD_ERROR_TIMEOUT;
            // hsd->State = HAL_SD_STATE_READY;
            return HAL_TIMEOUT;
        }
        else if (errorstate == HAL_ERROR)
        {
            printf("Read Block Error!\n");
            // hsd->State = HAL_SD_STATE_READY;
            return HAL_ERROR;
        }

        if (NumberOfBlocks > 1U)
        {
            errorstate = HAL_SDMMC_CmdStopTransfer(hsd->Instance);
            if (errorstate != HAL_OK)
            {
                printf("stop MultiBlock Read Failed!\n");
                return HAL_ERROR;
            }
        }
        HAL_SDIO_RXFIFO_Read(hsd->Instance, tempbuff, config.DataLength);

        hsd->State = HAL_SD_STATE_READY;

        return HAL_OK;
    }
    else
    {
        hsd->ErrorCode |= HAL_SD_ERROR_BUSY;
        HAL_SD_UNLOCK();
        return HAL_ERROR;
    }
}

/**
 * @brief Writes multiple or single blocks of data to an SD card in polling
 * mode. This function configures the SDIO interface for data transmission,
 *        sends the appropriate write command, and waits for the data to be
 * transferred from the FIFO to the SD card. The function then writes data from
 * the provided buffer to the TXFIFO.
 *
 * @param hsd : Pointer to the SD handle structure.
 * @param pData : Pointer to the buffer containing data to be written to the SD
 * card.
 * @param BlockAdd : Block address to start writing to.
 * @param NumberOfBlocks : Number of blocks to write.
 * @param Timeout : Timeout value in milliseconds for the write operation.
 *
 * @retval HAL_StatusTypeDef : HAL_OK if the operation is successful,
 *                             HAL_ERROR if an error occurs.
 */
HAL_StatusTypeDef HAL_SD_WriteBlocks(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout)
{
    SDIO_DataInitTypeDef config     = { 0 };
    uint32_t             errorstate = HAL_SD_ERROR_NONE;
    uint32_t            *tempbuff   = (uint32_t *)pData;

    if (NULL == pData)
    {
        hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
        return HAL_ERROR;
    }

    HAL_SD_LOCK();

    if (hsd->State == HAL_SD_STATE_READY)
    {
        hsd->ErrorCode = HAL_SD_ERROR_NONE;

        if ((BlockAdd + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
        {
            hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            HAL_SD_UNLOCK();
            return HAL_ERROR;
        }

        hsd->State = HAL_SD_STATE_BUSY;

        HAL_SD_UNLOCK();

        HAL_NVIC_DisableIRQ(SDIO_TX_CH_IRQ);

        if (hsd->SdCard.CardType != CARD_SDHC_SDXC)
        {
            BlockAdd *= 512U;
        }

        config.DataLength    = NumberOfBlocks * BLOCKSIZE;
        config.DataBlockSize = BLOCKSIZE;
        config.TransferDir   = SDIO_TRANSFER_DIR_TO_CARD;
        config.TransferMode  = SDIO_TRANSFER_MODE_BLOCK;
        HAL_SDIO_ConfigData(hsd->Instance, &config);

        errorstate = HAL_SDIO_TXFIFO_Write(hsd->Instance, tempbuff, NumberOfBlocks * BLOCKSIZE);

        /* Write Blocks in Polling mode */
        if (NumberOfBlocks > 1U)
        {
            hsd->Context = SD_CONTEXT_WRITE_MULTIPLE_BLOCK;

            /* Write Multi Block command */
            errorstate = HAL_SDMMC_CmdWriteMultiBlock(hsd->Instance, BlockAdd);
        }
        else
        {
            hsd->Context = SD_CONTEXT_WRITE_SINGLE_BLOCK;

            /* Write Single Block command */
            errorstate = HAL_SDMMC_CmdWriteSingleBlock(hsd->Instance, BlockAdd);
        }
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            hsd->ErrorCode |= errorstate;
            hsd->State = HAL_SD_STATE_READY;
            return HAL_ERROR;
        }

        errorstate = HAL_SDIO_WaitTX(hsd->Instance, Timeout);
        if (errorstate != HAL_OK)
        {
            return HAL_ERROR;
        }

        if (NumberOfBlocks > 1U)
        {
            errorstate = HAL_SDMMC_CmdStopTransfer(hsd->Instance);
            if (errorstate != HAL_OK)
            {
                printf("stop MultiBlock Write Failed!\n");
                return HAL_ERROR;
            }
        }

        hsd->State = HAL_SD_STATE_READY;

        return HAL_OK;
    }
    else
    {
        hsd->ErrorCode |= HAL_SD_ERROR_BUSY;
        HAL_SD_UNLOCK();
        return HAL_ERROR;
    }
}

/**
 * @brief Reads multiple or single blocks of data from an SD card in interrupt
 * mode. This function configures the SDIO interface for data transmission, sets
 * up the RXFIFO, and then triggers the read operation. It uses interrupts to
 *        handle the data once it is transferred into the FIFO.
 *
 * @param hsd : Pointer to the SD handle structure.
 * @param pData : Pointer to the buffer where data will be stored after read
 * from the SD card.
 * @param BlockAdd : Block address to start reading from.
 * @param NumberOfBlocks : Number of blocks to read.
 *
 * @retval HAL_StatusTypeDef : HAL_OK if the operation is successfully
 * initiated, HAL_ERROR if an error occurs, HAL_BUSY if the SD card is currently
 * busy.
 */
HAL_StatusTypeDef HAL_SD_ReadBlocks_IT(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
    SDIO_DataInitTypeDef config;
    uint32_t             errorstate = HAL_SD_ERROR_NONE;

    if (NULL == pData)
    {
        hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
        return HAL_ERROR;
    }

    HAL_SD_LOCK();

    if (hsd->State == HAL_SD_STATE_READY)
    {
        hsd->ErrorCode = HAL_SD_ERROR_NONE;

        if ((BlockAdd + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
        {
            hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            HAL_SD_UNLOCK();
            return HAL_ERROR;
        }

        hsd->State = HAL_SD_STATE_BUSY;

        HAL_SD_UNLOCK();

        HAL_NVIC_ClearPendingIRQ(SDIO_RX_CH_IRQ);
        HAL_NVIC_EnableIRQ(SDIO_RX_CH_IRQ);

        hsd->pRxBuffPtr   = (uint32_t *)pData;
        hsd->RxXferSize   = BLOCKSIZE * NumberOfBlocks;
        hsd->XferBlockAdd = BlockAdd;
        if (hsd->RxXferSize > SD_TRANS_BLOCK_MAX * BLOCKSIZE)
        {
            hsd->RxSingleSize = SD_TRANS_BLOCK_MAX * BLOCKSIZE;
        }
        else
        {
            hsd->RxSingleSize = hsd->RxXferSize;
        }

        if (hsd->SdCard.CardType != CARD_SDHC_SDXC)
        {
            BlockAdd *= 512U;
        }

        config.DataLength    = hsd->RxSingleSize;
        config.DataBlockSize = BLOCKSIZE;
        config.TransferDir   = SDIO_TRANSFER_DIR_TO_SDIO;
        config.TransferMode  = SDIO_TRANSFER_MODE_BLOCK;
        HAL_SDIO_ConfigData(hsd->Instance, &config);

        errorstate = HAL_SDIO_RXFIFO_Setup(hsd->Instance, hsd->RxSingleSize);

        /* Read Blocks in IT mode */
        if (NumberOfBlocks > 1U)
        {
            hsd->Context = (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_IT);

            /* Read Multi Block command */
            errorstate = HAL_SDMMC_CmdReadMultiBlock(hsd->Instance, BlockAdd);
        }
        else
        {
            hsd->Context = (SD_CONTEXT_READ_SINGLE_BLOCK | SD_CONTEXT_IT);

            /* Read Single Block command */
            errorstate = HAL_SDMMC_CmdReadSingleBlock(hsd->Instance, BlockAdd);
        }
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            hsd->ErrorCode |= errorstate;
            hsd->State = HAL_SD_STATE_READY;
            return HAL_ERROR;
        }

        return HAL_OK;
    }
    else
    {
        hsd->ErrorCode |= HAL_SD_ERROR_BUSY;
        HAL_SD_UNLOCK();
        return HAL_BUSY;
    }
}

/**
 * @brief Writes multiple or single blocks of data to an SD card in interrupt
 * mode. This function configures the SDIO interface for data transmission,
 *        sends the appropriate write command, and enables the interrupt for the
 *        transmission process. The function then writes the data to the SDIO
 *        FIFO in preparation for transmission to the SD card.
 *
 * @param hsd : Pointer to the SD handle structure.
 * @param pData : Pointer to the buffer containing the data to be written to the
 * SD card.
 * @param BlockAdd : Block address to start writing to.
 * @param NumberOfBlocks : Number of blocks to write.
 *
 * @retval HAL_StatusTypeDef : HAL_OK if the operation is successfully
 * completed, HAL_ERROR if an error occurs, HAL_BUSY if the SD is not ready for
 * the operation.
 */
HAL_StatusTypeDef HAL_SD_WriteBlocks_IT(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
    SDIO_DataInitTypeDef config     = { 0 };
    uint32_t            *tempbuff   = (uint32_t *)pData;
    uint32_t             errorstate = HAL_SD_ERROR_NONE;

    if (NULL == pData)
    {
        hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
        return HAL_ERROR;
    }

    HAL_SD_LOCK();

    if (hsd->State == HAL_SD_STATE_READY)
    {
        hsd->ErrorCode = HAL_SD_ERROR_NONE;

        if ((BlockAdd + NumberOfBlocks) > (hsd->SdCard.LogBlockNbr))
        {
            hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            HAL_SD_UNLOCK();
            return HAL_ERROR;
        }

        hsd->State = HAL_SD_STATE_BUSY;

        HAL_SD_UNLOCK();

        HAL_NVIC_ClearPendingIRQ(SDIO_TX_CH_IRQ);
        HAL_NVIC_EnableIRQ(SDIO_TX_CH_IRQ);

        if (hsd->SdCard.CardType != CARD_SDHC_SDXC)
        {
            BlockAdd *= 512U;
        }

        hsd->pTxBuffPtr   = (uint32_t *)pData;
        hsd->TxXferSize   = BLOCKSIZE * NumberOfBlocks;
        hsd->XferBlockAdd = BlockAdd;
        if (NumberOfBlocks > SD_TRANS_BLOCK_MAX)
        {
            hsd->TxSingleSize = SD_TRANS_BLOCK_MAX * BLOCKSIZE;
        }
        else
        {
            hsd->TxSingleSize = hsd->TxXferSize;
        }

        /* Configure the SD DPSM (Data Path State Machine) */
        config.DataLength    = hsd->TxSingleSize;
        config.DataBlockSize = BLOCKSIZE;
        config.TransferDir   = SDIO_TRANSFER_DIR_TO_CARD;
        config.TransferMode  = SDIO_TRANSFER_MODE_BLOCK;
        HAL_SDIO_ConfigData(hsd->Instance, &config);

        errorstate = HAL_SDIO_TXFIFO_Write(hsd->Instance, tempbuff, hsd->TxSingleSize);

        /* Write Blocks in Polling mode */
        if (NumberOfBlocks > 1U)
        {
            hsd->Context = (SD_CONTEXT_WRITE_MULTIPLE_BLOCK | SD_CONTEXT_IT);

            /* Write Multi Block command */
            errorstate = HAL_SDMMC_CmdWriteMultiBlock(hsd->Instance, BlockAdd);
        }
        else
        {
            hsd->Context = (SD_CONTEXT_WRITE_SINGLE_BLOCK | SD_CONTEXT_IT);

            /* Write Single Block command */
            errorstate = HAL_SDMMC_CmdWriteSingleBlock(hsd->Instance, BlockAdd);
        }
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            hsd->ErrorCode |= errorstate;
            hsd->State = HAL_SD_STATE_READY;
            return HAL_ERROR;
        }

        return HAL_OK;
    }
    else
    {
        hsd->ErrorCode |= HAL_SD_ERROR_BUSY;
        HAL_SD_UNLOCK();
        return HAL_BUSY;
    }
}

/**
 * @brief Erases a range of blocks on the SD card.
 *        This function issues the necessary commands to erase a range of blocks
 *        from `BlockStartAdd` to `BlockEndAdd` on the SD card. Sends the erase
 * commands (CMD32, CMD33, CMD38) for the SD card, and waits for the operation
 * to complete. If the card does not support the erase command or if there are
 * any errors, the function will return an error.
 *
 * @param hsd : Pointer to the SD handle structure.
 * @param BlockStartAdd : The starting block address for the erase operation.
 * @param BlockEndAdd : The ending block address for the erase operation.
 *
 * @retval HAL_StatusTypeDef : HAL_OK if the erase operation is successful,
 *                             HAL_ERROR if an error occurs (e.g., parameter
 * error, address out of range, card locked).
 */
HAL_StatusTypeDef HAL_SD_Erase(SD_HandleTypeDef *hsd, uint32_t BlockStartAdd, uint32_t BlockEndAdd)
{
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    HAL_SD_LOCK();

    if (hsd->State == HAL_SD_STATE_READY)
    {
        hsd->ErrorCode = HAL_SD_ERROR_NONE;

        if (BlockEndAdd < BlockStartAdd)
        {
            hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
            HAL_SD_UNLOCK();
            return HAL_ERROR;
        }

        if (BlockEndAdd > (hsd->SdCard.LogBlockNbr))
        {
            hsd->ErrorCode |= HAL_SD_ERROR_ADDR_OUT_OF_RANGE;
            HAL_SD_UNLOCK();
            return HAL_ERROR;
        }

        hsd->State = HAL_SD_STATE_BUSY;

        HAL_SD_UNLOCK();

        /* Check if the card command class supports erase command */
        if (((hsd->SdCard.Class) & SDIO_CCCC_ERASE) == 0U)
        {
            hsd->ErrorCode |= HAL_SD_ERROR_REQUEST_NOT_APPLICABLE;
            hsd->State = HAL_SD_STATE_READY;
            return HAL_ERROR;
        }

        if ((HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
        {
            hsd->ErrorCode |= HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
            hsd->State = HAL_SD_STATE_READY;
            return HAL_ERROR;
        }

        /* Get start and end block for high capacity cards */
        if (hsd->SdCard.CardType != CARD_SDHC_SDXC)
        {
            BlockStartAdd *= 512U;
            BlockEndAdd *= 512U;
        }

        /* According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and
         * erase_group_end(CMD33) */
        if (hsd->SdCard.CardType != CARD_SECURED)
        {
            /* Send CMD32 SD_ERASE_GRP_START with argument as addr  */
            errorstate = HAL_SDMMC_CmdSDEraseStartAdd(hsd->Instance, BlockStartAdd);
            if (errorstate != HAL_SD_ERROR_NONE)
            {
                hsd->ErrorCode |= errorstate;
                hsd->State = HAL_SD_STATE_READY;
                return HAL_ERROR;
            }

            /* Send CMD33 SD_ERASE_GRP_END with argument as addr  */
            errorstate = HAL_SDMMC_CmdSDEraseEndAdd(hsd->Instance, BlockEndAdd);
            if (errorstate != HAL_SD_ERROR_NONE)
            {
                hsd->ErrorCode |= errorstate;
                hsd->State = HAL_SD_STATE_READY;
                return HAL_ERROR;
            }
        }

        /* Send CMD38 ERASE */
        errorstate = HAL_SDMMC_CmdSDErase(hsd->Instance);
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            hsd->ErrorCode |= errorstate;
            hsd->State = HAL_SD_STATE_READY;
            return HAL_ERROR;
        }

        hsd->State = HAL_SD_STATE_READY;

        return HAL_OK;
    }
    else
    {
        hsd->ErrorCode |= HAL_SD_ERROR_BUSY;
        HAL_SD_UNLOCK();
        return HAL_BUSY;
    }
}

/**
 * @brief Aborts the ongoing SD card data transfer.
 *        This function stops the ongoing data transfer (either receiving or
 * sending) on the SD card. It disables the relevant interrupts, checks the
 * current card state, and if the card is in the middle of a transfer, issues a
 * stop command to halt the operation. After the operation, it returns the
 * result of the abort operation and resets the state of the SD handle to ready.
 *
 * @param hsd : Pointer to the SD handle structure.
 *
 * @retval HAL_StatusTypeDef : HAL_OK if the abort operation is successful,
 *                             HAL_ERROR if an error occurs (e.g., stop transfer
 * command failure).
 */
HAL_StatusTypeDef HAL_SD_Abort(SD_HandleTypeDef *hsd)
{
    HAL_SD_CardStateTypeDef CardState;

    /* DIsable All interrupts */
    HAL_NVIC_DisableIRQ(SDIO_RX_CH_IRQ);
    HAL_NVIC_DisableIRQ(SDIO_TX_CH_IRQ);
    HAL_NVIC_DisableIRQ(SDIO_EOT_CH_IRQ);
    HAL_NVIC_DisableIRQ(SDIO_ERR_CH_IRQ);

    hsd->State = HAL_SD_STATE_READY;
    CardState  = HAL_SD_GetCardState(hsd);
    if ((CardState == HAL_SD_CARD_RECEIVING) || (CardState == HAL_SD_CARD_SENDING))
    {
        hsd->ErrorCode = HAL_SDMMC_CmdStopTransfer(hsd->Instance);
    }
    if (hsd->ErrorCode != HAL_SD_ERROR_NONE)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup SD_Exported_Functions_Group4
 *  @brief      Peripheral Control Functions
 *
@verbatim
  ==============================================================================
                      ##### Peripheral Control functions #####
  ==============================================================================
  [..]
    This section provides functions allowing to control the SD card
    operations
@endverbatim
  * @{
  */
/**
 * @brief Configures the SD card for wide bus operation (4-bit or 1-bit).
 *        This function configures the SD card to use either a 4-bit wide bus
 *        or a 1-bit wide bus for data transfer. The wide bus mode can improve
 *        data transfer speed, but the card must support the requested bus
 * width.
 *
 * @param hsd : Pointer to the SD handle structure.
 * @param WideMode : The desired bus width. This can be one of the following:
 *         - SDIO_BUS_WIDE_4B : 4-bit wide bus mode.
 *         - SDIO_BUS_WIDE_1B : 1-bit wide bus mode.
 *
 * @retval HAL_StatusTypeDef : HAL_OK if the operation is successful,
 *                             HAL_ERROR if an error occurs (e.g., unsupported
 * feature or invalid parameter).
 */
HAL_StatusTypeDef HAL_SD_ConfigWideBusOperation(SD_HandleTypeDef *hsd, uint32_t WideMode)
{
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    HAL_SD_LOCK();

    /* Change State */
    if (hsd->State != HAL_SD_STATE_READY)
    {
        hsd->ErrorCode |= HAL_SD_STATE_BUSY;
        HAL_SD_UNLOCK();
        return HAL_BUSY;
    }
    else
    {
        hsd->State = HAL_SD_STATE_BUSY;
        HAL_SD_UNLOCK();
    }

    if (hsd->SdCard.CardType != CARD_SECURED)
    {
        if (WideMode == SDIO_BUS_WIDE_4B)
        {
            errorstate = SD_WideBus_Enable(hsd);

            hsd->ErrorCode |= errorstate;
        }
        else if (WideMode == SDIO_BUS_WIDE_1B)
        {
            errorstate = SD_WideBus_Disable(hsd);

            hsd->ErrorCode |= errorstate;
        }
        else
        {
            /* WideMode is not a valid argument*/
            hsd->ErrorCode |= HAL_SD_ERROR_PARAM;
        }
    }
    else
    {
        /* MMC Card does not support this feature */
        hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
    }

    if (hsd->ErrorCode != HAL_SD_ERROR_NONE)
    {
        hsd->State = HAL_SD_STATE_READY;
        return HAL_ERROR;
    }
    else
    {
        /* Configure the SDIO peripheral */
        HAL_SDIO_SetBusWidth(hsd->Instance, WideMode);
    }

    /* Change State */
    hsd->State = HAL_SD_STATE_READY;

    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup SD_Exported_Functions_Group4
 *  @brief      SD Card Related Functions
 *
@verbatim
  ==============================================================================
                ##### SD Card Related Functions #####
  ==============================================================================
  [..]
    This section provides functions allowing to get the related information
    from SD card.
@endverbatim
  * @{
  */

/**
 * @brief  Returns the information of the card .
 *
 * @param  hsd : Pointer to the SD handle structure.
 * @param  pCID : Pointer to a HAL_SD_CardCIDTypeDef structure that
 *         contains all CID register parameters
 *
 * @retval HAL_StatusTypeDef : HAL_OK
 */
HAL_StatusTypeDef HAL_SD_GetCardCID(SD_HandleTypeDef *hsd, HAL_SD_CardCIDTypeDef *pCID)
{
    uint32_t tmp = 0U;

    /* Byte 0 */
    tmp                  = (uint8_t)((hsd->CID[0U] & 0xFF000000U) >> 24U);
    pCID->ManufacturerID = tmp;

    /* Byte 1 */
    tmp               = (uint8_t)((hsd->CID[0U] & 0x00FF0000U) >> 16U);
    pCID->OEM_AppliID = tmp << 8U;

    /* Byte 2 */
    tmp = (uint8_t)((hsd->CID[0U] & 0x000000FF00U) >> 8U);
    pCID->OEM_AppliID |= tmp;

    /* Byte 3 */
    tmp             = (uint8_t)(hsd->CID[0U] & 0x000000FFU);
    pCID->ProdName1 = tmp << 24U;

    /* Byte 4 */
    tmp = (uint8_t)((hsd->CID[1U] & 0xFF000000U) >> 24U);
    pCID->ProdName1 |= tmp << 16;

    /* Byte 5 */
    tmp = (uint8_t)((hsd->CID[1U] & 0x00FF0000U) >> 16U);
    pCID->ProdName1 |= tmp << 8U;

    /* Byte 6 */
    tmp = (uint8_t)((hsd->CID[1U] & 0x0000FF00U) >> 8U);
    pCID->ProdName1 |= tmp;

    /* Byte 7 */
    tmp             = (uint8_t)(hsd->CID[1U] & 0x000000FFU);
    pCID->ProdName2 = tmp;

    /* Byte 8 */
    tmp           = (uint8_t)((hsd->CID[2U] & 0xFF000000U) >> 24U);
    pCID->ProdRev = tmp;

    /* Byte 9 */
    tmp          = (uint8_t)((hsd->CID[2U] & 0x00FF0000U) >> 16U);
    pCID->ProdSN = tmp << 24U;

    /* Byte 10 */
    tmp = (uint8_t)((hsd->CID[2U] & 0x0000FF00U) >> 8U);
    pCID->ProdSN |= tmp << 16U;

    /* Byte 11 */
    tmp = (uint8_t)(hsd->CID[2U] & 0x000000FFU);
    pCID->ProdSN |= tmp << 8U;

    /* Byte 12 */
    tmp = (uint8_t)((hsd->CID[3U] & 0xFF000000U) >> 24U);
    pCID->ProdSN |= tmp;

    /* Byte 13 */
    tmp = (uint8_t)((hsd->CID[3U] & 0x00FF0000U) >> 16U);
    pCID->Reserved1 |= (tmp & 0xF0U) >> 4U;
    pCID->ManufactDate = (tmp & 0x0FU) << 8U;

    /* Byte 14 */
    tmp = (uint8_t)((hsd->CID[3U] & 0x0000FF00U) >> 8U);
    pCID->ManufactDate |= tmp;

    /* Byte 15 */
    tmp             = (uint8_t)(hsd->CID[3U] & 0x000000FFU);
    pCID->CID_CRC   = (tmp & 0xFEU) >> 1U;
    pCID->Reserved2 = 1U;

    return HAL_OK;
}

/**
 * @brief  Returns the information of the card.
 *
 * @param  hsd : Pointer to the SD handle structure.
 * @param  pCSD : Pointer to a HAL_SD_CardCSDTypeDef structure that
 *         contains all CSD register parameters
 *
 * @retval HAL_StatusTypeDef :
 *         - HAL_OK if the CSD is retrieved and parsed successfully.
 *         - HAL_ERROR if an error occurs (e.g., unsupported card type).
 */
HAL_StatusTypeDef HAL_SD_GetCardCSD(SD_HandleTypeDef *hsd, HAL_SD_CardCSDTypeDef *pCSD)
{
    uint32_t tmp = 0U;

    /* Byte 0 */
    tmp                  = (hsd->CSD[0U] & 0xFF000000U) >> 24U;
    pCSD->CSDStruct      = (uint8_t)((tmp & 0xC0U) >> 6U); /* bit[7:6] */
    pCSD->SysSpecVersion = (uint8_t)((tmp & 0x3CU) >> 2U); /* bit[5:2] */
    pCSD->Reserved1      = tmp & 0x03U;                    /* bit[1:0] */

    /* Byte 1 */
    tmp        = (hsd->CSD[0U] & 0x00FF0000U) >> 16U;
    pCSD->TAAC = (uint8_t)tmp;

    /* Byte 2 */
    tmp        = (hsd->CSD[0U] & 0x0000FF00U) >> 8U;
    pCSD->NSAC = (uint8_t)tmp;

    /* Byte 3 */
    tmp                 = hsd->CSD[0U] & 0x000000FFU;
    pCSD->MaxBusClkFrec = (uint8_t)tmp;

    /* Byte 4 */
    tmp                   = (hsd->CSD[1U] & 0xFF000000U) >> 24U;
    pCSD->CardComdClasses = (uint16_t)(tmp << 4U); /* Higher 8bits of CCC */

    /* Byte 5 */
    tmp = (hsd->CSD[1U] & 0x00FF0000U) >> 16U;
    pCSD->CardComdClasses |= (uint16_t)((tmp & 0xF0U) >> 4U); /* Lower 4bits of CCC */
    pCSD->RdBlockLen = (uint8_t)(tmp & 0x0FU);

    /* Byte 6 */
    tmp                   = (hsd->CSD[1U] & 0x0000FF00U) >> 8U;
    pCSD->PartBlockRead   = (uint8_t)((tmp & 0x80U) >> 7U); /* bit7 */
    pCSD->WrBlockMisalign = (uint8_t)((tmp & 0x40U) >> 6U); /* bit6 */
    pCSD->RdBlockMisalign = (uint8_t)((tmp & 0x20U) >> 5U); /* bit5 */
    pCSD->DSRImpl         = (uint8_t)((tmp & 0x10U) >> 4U); /* bit4 */
    pCSD->Reserved2       = 0U;                             /*!< Reserved */

    if (hsd->SdCard.CardType == CARD_SDSC)
    {
        pCSD->DeviceSize = (tmp & 0x03U) << 10U;

        /* Byte 7 */
        tmp = (uint8_t)(hsd->CSD[1U] & 0x000000FFU);
        pCSD->DeviceSize |= (tmp) << 2U;

        /* Byte 8 */
        tmp = (uint8_t)((hsd->CSD[2U] & 0xFF000000U) >> 24U);
        pCSD->DeviceSize |= (tmp & 0xC0U) >> 6U;

        pCSD->MaxRdCurrentVDDMin = (tmp & 0x38U) >> 3U;
        pCSD->MaxRdCurrentVDDMax = (tmp & 0x07U);

        /* Byte 9 */
        tmp                      = (uint8_t)((hsd->CSD[2U] & 0x00FF0000U) >> 16U);
        pCSD->MaxWrCurrentVDDMin = (tmp & 0xE0U) >> 5U;
        pCSD->MaxWrCurrentVDDMax = (tmp & 0x1CU) >> 2U;
        pCSD->DeviceSizeMul      = (tmp & 0x03U) << 1U;
        /* Byte 10 */
        tmp = (uint8_t)((hsd->CSD[2U] & 0x0000FF00U) >> 8U);
        pCSD->DeviceSizeMul |= (tmp & 0x80U) >> 7U;

        hsd->SdCard.BlockNbr = (pCSD->DeviceSize + 1U);
        hsd->SdCard.BlockNbr *= (1U << (pCSD->DeviceSizeMul + 2U));
        hsd->SdCard.BlockSize = 1U << (pCSD->RdBlockLen);

        hsd->SdCard.LogBlockNbr  = (hsd->SdCard.BlockNbr) * ((hsd->SdCard.BlockSize) / 512U);
        hsd->SdCard.LogBlockSize = 512U;
    }
    else if (hsd->SdCard.CardType == CARD_SDHC_SDXC)
    {
        /* Byte 7 */
        tmp              = (uint8_t)(hsd->CSD[1U] & 0x000000FFU);
        pCSD->DeviceSize = (tmp & 0x3FU) << 16U; /* C_SIZE: [21:16] */

        /* Byte 8 */
        tmp = (uint8_t)((hsd->CSD[2U] & 0xFF000000U) >> 24U);
        pCSD->DeviceSize |= (tmp << 8U); /* C_SIZE: [15:8] */

        /* Byte 9 */
        tmp = (uint8_t)((hsd->CSD[2U] & 0x00FF0000U) >> 16U);
        pCSD->DeviceSize |= (tmp); /* C_SIZE: [7:0] */
        /* Capacity = (pCSD->DeviceSize + 1) * BlockSize * 1024 */
        /* SDHC Card BlockSize = 512byte */
        /* the Number of Block = Capacity / BlockSize */
        hsd->SdCard.LogBlockNbr = hsd->SdCard.BlockNbr = (((uint64_t)pCSD->DeviceSize + 1U) * 1024U);
        hsd->SdCard.LogBlockSize = hsd->SdCard.BlockSize = 512U;

        /* Byte 10 */
        tmp = (uint8_t)((hsd->CSD[2U] & 0x0000FF00U) >> 8U);
    }
    else
    {
        hsd->ErrorCode |= HAL_SD_ERROR_UNSUPPORTED_FEATURE;
        hsd->State = HAL_SD_STATE_READY;
        return HAL_ERROR;
    }

    pCSD->EraseGrSize = (tmp & 0x40U) >> 6U; /* Bit6 */
    pCSD->EraseGrMul  = (tmp & 0x3FU) << 1U; /* Higher 6bits of SECTOR_SIZE: [5:1] */

    /* Byte 11 */
    tmp = (uint8_t)(hsd->CSD[2U] & 0x000000FFU);
    pCSD->EraseGrMul |= (tmp & 0x80U) >> 7U; /* Lowest bits of SECTOR_SIZE: bit0 */
    pCSD->WrProtectGrSize = (tmp & 0x7FU);

    /* Byte 12 */
    tmp                     = (uint8_t)((hsd->CSD[3U] & 0xFF000000U) >> 24U);
    pCSD->WrProtectGrEnable = (tmp & 0x80U) >> 7U;
    pCSD->ManDeflECC        = (tmp & 0x60U) >> 5U;
    pCSD->WrSpeedFact       = (tmp & 0x1CU) >> 2U;
    pCSD->MaxWrBlockLen     = (tmp & 0x03U) << 2U;

    /* Byte 13 */
    tmp = (uint8_t)((hsd->CSD[3U] & 0x00FF0000U) >> 16U);
    pCSD->MaxWrBlockLen |= (tmp & 0xC0U) >> 6U;
    pCSD->WriteBlockPaPartial = (tmp & 0x20U) >> 5U;
    pCSD->Reserved3           = 0U;
    pCSD->ContentProtectAppli = (tmp & 0x01U);

    /* Byte 14 */
    tmp                    = (uint8_t)((hsd->CSD[3U] & 0x0000FF00U) >> 8U);
    pCSD->FileFormatGrouop = (tmp & 0x80U) >> 7U;
    pCSD->CopyFlag         = (tmp & 0x40U) >> 6U;
    pCSD->PermWrProtect    = (tmp & 0x20U) >> 5U;
    pCSD->TempWrProtect    = (tmp & 0x10U) >> 4U;
    pCSD->FileFormat       = (tmp & 0x0CU) >> 2U;
    pCSD->ECC              = (tmp & 0x03U);

    /* Byte 15 */
    tmp             = (uint8_t)(hsd->CSD[3U] & 0x000000FFU);
    pCSD->CSD_CRC   = (tmp & 0xFEU) >> 1U;
    pCSD->Reserved4 = 1U;

    return HAL_OK;
}

/**
 * @brief  Gets the SD status information(SD status register).
 *
 * @param  hsd : Pointer to the SD handle structure.
 * @param  pStatus Pointer to the HAL_SD_CardStatusTypeDef structure that
 *         will contain the SD card status information
 *
 * @retval HAL_StatusTypeDef: HAL_OK if successful, HAL_ERROR if failure occurs.
 */
HAL_StatusTypeDef HAL_SD_GetSDStatus(SD_HandleTypeDef *hsd, HAL_SD_SDStatusTypeDef *pStatus)
{
    uint32_t tmp = 0U;
    uint32_t sd_status[16U];
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    errorstate = SD_SendSDStatus(hsd, sd_status);
    if (errorstate != HAL_OK)
    {
        hsd->ErrorCode |= errorstate;
        hsd->State = HAL_SD_STATE_READY;
        return HAL_ERROR;
    }
    else
    {
        /* Byte 0 */
        tmp                   = (sd_status[0U] & 0xC0U) >> 6U;
        pStatus->DataBusWidth = (uint8_t)tmp;

        /* Byte 0 */
        tmp                  = (sd_status[0U] & 0x20U) >> 5U;
        pStatus->SecuredMode = (uint8_t)tmp;

        /* Byte 2 */
        tmp               = (sd_status[0U] & 0x00FF0000U) >> 16U;
        pStatus->CardType = (uint16_t)(tmp << 8U);

        /* Byte 3 */
        tmp = (sd_status[0U] & 0xFF000000U) >> 24U;
        pStatus->CardType |= (uint16_t)tmp;

        /* Byte 4 */
        tmp                        = (sd_status[1U] & 0xFFU);
        pStatus->ProtectedAreaSize = (uint32_t)(tmp << 24U);

        /* Byte 5 */
        tmp = (sd_status[1U] & 0xFF00U) >> 8U;
        pStatus->ProtectedAreaSize |= (uint32_t)(tmp << 16U);

        /* Byte 6 */
        tmp = (sd_status[1U] & 0xFF0000U) >> 16U;
        pStatus->ProtectedAreaSize |= (uint32_t)(tmp << 8U);

        /* Byte 7 */
        tmp = (sd_status[1U] & 0xFF000000U) >> 24U;
        pStatus->ProtectedAreaSize |= (uint32_t)tmp;

        /* Byte 8 */
        tmp                 = (sd_status[2U] & 0xFFU);
        pStatus->SpeedClass = (uint8_t)tmp;

        /* Byte 9 */
        tmp                      = (sd_status[2U] & 0xFF00U) >> 8U;
        pStatus->PerformanceMove = (uint8_t)tmp;

        /* Byte 10 */
        tmp                         = (sd_status[2U] & 0xF00000U) >> 20U;
        pStatus->AllocationUnitSize = (uint8_t)tmp;

        /* Byte 11 */
        tmp                = (sd_status[2U] & 0xFF000000U) >> 24U;
        pStatus->EraseSize = (uint16_t)(tmp << 8U);

        /* Byte 12 */
        tmp = (sd_status[3U] & 0xFFU);
        pStatus->EraseSize |= (uint16_t)tmp;

        /* Byte 13 */
        tmp                   = (sd_status[3U] & 0xFC00U) >> 10U;
        pStatus->EraseTimeout = (uint8_t)tmp;

        /* Byte 13 */
        tmp                  = (sd_status[3U] & 0x0300U) >> 8U;
        pStatus->EraseOffset = (uint8_t)tmp;
    }

    return HAL_OK;
}

/**
 * @brief  Get the information of the SD card, including its type, version,
 * class, address, and block size. The retrieved information is stored in the
 * provided card info structure.
 *
 * @param  hsd : Pointer to the SD handle structure.
 * @param  pCardInfo: Pointer to a structure that will be filled with the SD
 * card information.
 *
 * @retval HAL_StatusTypeDef: HAL_OK
 */
HAL_StatusTypeDef HAL_SD_GetCardInfo(SD_HandleTypeDef *hsd, HAL_SD_CardInfoTypeDef *pCardInfo)
{
    pCardInfo->CardType     = (uint32_t)(hsd->SdCard.CardType);
    pCardInfo->CardVersion  = (uint32_t)(hsd->SdCard.CardVersion);
    pCardInfo->Class        = (uint32_t)(hsd->SdCard.Class);
    pCardInfo->RelCardAdd   = (uint32_t)(hsd->SdCard.RelCardAdd);
    pCardInfo->BlockNbr     = (uint32_t)(hsd->SdCard.BlockNbr);
    pCardInfo->BlockSize    = (uint32_t)(hsd->SdCard.BlockSize);
    pCardInfo->LogBlockNbr  = (uint32_t)(hsd->SdCard.LogBlockNbr);
    pCardInfo->LogBlockSize = (uint32_t)(hsd->SdCard.LogBlockSize);

    return HAL_OK;
}

/**
 * @brief  Retrieves the current state of the SD card by sending a status
 * command and extracting the state from the response.
 *
 * @param  hsd : Pointer to the SD handle structure.
 *
 * @retval HAL_SD_CardStateTypeDef: The current state of the SD card.
 */
HAL_SD_CardStateTypeDef HAL_SD_GetCardState(SD_HandleTypeDef *hsd)
{
    HAL_SD_CardStateTypeDef cardstate  = HAL_SD_CARD_TRANSFER;
    uint32_t                errorstate = HAL_SD_ERROR_NONE;
    uint32_t                resp1      = 0;

    errorstate = SD_SendStatus(hsd, &resp1);
    if (errorstate != HAL_OK)
    {
        hsd->ErrorCode |= errorstate;
    }

    cardstate = (HAL_SD_CardStateTypeDef)((resp1 >> 9U) & 0x0FU);

    return cardstate;
}

/** @defgroup SD_Exported_Functions_Group6
 * @brief     Peripheral State and Errors Functions
 * @{
 */
/**
 * @brief return the SD state
 *
 * @param  hsd : Pointer to the SD handle structure.
 *
 * @retval HAL_SD_StateTypeDef: SD Handle Current State.
 */
HAL_SD_StateTypeDef HAL_SD_GetState(SD_HandleTypeDef *hsd)
{
    return hsd->State;
}

/**
 * @brief  Return the SD error code
 *
 * @param  hsd : Pointer to the SD handle structure.
 *
 * @retval uint32_t: SD Error Code
 */
uint32_t HAL_SD_GetError(SD_HandleTypeDef *hsd)
{
    return hsd->ErrorCode;
}
/**
 * @}
 */

/**
 * @}
 */
/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Initializes the SD card by sending various SD commands to retrieve
 *         the card's CID, CSD, and other configuration parameters, and setting
 *         the appropriate card settings.
 *
 * @param  hsd : Pointer to the SD handle structure.
 *
 * @retval uint32_t: Error state (HAL_SD_ERROR_NONE if successful, otherwise an
 * error code).
 */
static uint32_t SD_InitCard(SD_HandleTypeDef *hsd)
{
    HAL_SD_CardCSDTypeDef CSD;
    uint32_t              errorstate = HAL_SD_ERROR_NONE;
    uint16_t              sd_rca     = 1U;

    if (hsd->SdCard.CardType != CARD_SECURED)
    {
        /* Send CMD2 ALL_SEND_CID */
        errorstate = HAL_SDMMC_CmdSendCID(hsd->Instance);
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            return errorstate;
        }
        else
        {
            /* Get Card identification number data */
            hsd->CID[0U] = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1);
            hsd->CID[1U] = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP2);
            hsd->CID[2U] = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP3);
            hsd->CID[3U] = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP4);
        }
    }

    if (hsd->SdCard.CardType != CARD_SECURED)
    {
        /* Send CMD3 SET_REL_ADDR with argument 0 */
        /* SD Card publishes its RCA. */
        errorstate = HAL_SDMMC_CmdSetRelAdd(hsd->Instance, &sd_rca);
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            return errorstate;
        }
    }
    if (hsd->SdCard.CardType != CARD_SECURED)
    {
        /* Get the SD card RCA */
        hsd->SdCard.RelCardAdd = sd_rca;

        /* Send CMD9 SEND_CSD with argument as card's RCA */
        errorstate = HAL_SDMMC_CmdSendCSD(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            return errorstate;
        }
        else
        {
            /* Get Card Specific Data */
            hsd->CSD[0U] = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP4); /* CSD 127 - 96 */
            hsd->CSD[1U] = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP3); /* CSD 95 - 64  */
            hsd->CSD[2U] = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP2); /* CSD 63 - 32  */
            hsd->CSD[3U] = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1); /* CSD 31 - 0   */
        }
    }

    /* Get the Card Class */
    hsd->SdCard.Class = (HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP3) >> 20U);

    /* Get CSD parameters */
    HAL_SD_GetCardCSD(hsd, &CSD);

    /* Send CMD7 to Select the Card */
    errorstate = HAL_SDMMC_CmdSelDesel(hsd->Instance, (uint32_t)(((uint32_t)hsd->SdCard.RelCardAdd) << 16U));
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        return errorstate;
    }

    /* Configure SDIO peripheral interface */
    // SDIO_Init(hsd->Instance, hsd->Init);
    /* Set Clock */
    // hsd->Init.Clock = SDIO_CLK_25M;
    // SDIO_SetClock(hsd->Instance, hsd->Init.Clock);

    /* Set Block Size for Card */
    errorstate = HAL_SDMMC_CmdBlockLength(hsd->Instance, BLOCKSIZE);
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        hsd->ErrorCode |= errorstate;
        hsd->State = HAL_SD_STATE_READY;
        return HAL_ERROR;
    }

    /* All cards are initialized */
    return HAL_SD_ERROR_NONE;
}

/**
 * @brief  Powers on the SD card and initializes it by sending various SD
 * commands to check for card version, operating voltage, and card type
 * (SDSC/SDHC). This function handles both V1.0 and V2.0 cards.
 *
 * @param  hsd : Pointer to the SD handle structure.
 *
 * @retval uint32_t: Error state (HAL_SD_ERROR_NONE if successful, otherwise an
 * error code).
 */

static uint32_t SD_PowerON(SD_HandleTypeDef *hsd)
{
    __IO uint32_t count    = 0U;
    uint32_t      response = 0U, validvoltage = 0U;
    uint32_t      errorstate = HAL_SD_ERROR_NONE;
    // SDIO_CmdInitTypeDef Command = {0};

    /* CMD0: GO_IDLE_STATE */
    errorstate = HAL_SDMMC_CmdGoIdleState(hsd->Instance);
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        return errorstate;
    }

    /* CMD8: SEND_IF_COND: Command available only on V2.0 cards */
    errorstate = HAL_SDMMC_CmdOperCond(hsd->Instance);
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        hsd->SdCard.CardVersion = CARD_V1_X;

        /* Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
        while (validvoltage == 0U)
        {
            if (count++ == SDMMC_MAX_VOLT_TRIAL)
            {
                return HAL_SD_ERROR_INVALID_VOLTRANGE;
            }

            /* SEND CMD55 APP_CMD with RCA as 0 */
            errorstate = HAL_SDMMC_CmdAppCommand(hsd->Instance, 0U);
            /* No Check CMD55 for workaround */
            // if (errorstate != HAL_SD_ERROR_NONE) {
            //   return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
            // }

            /* Send CMD41 */
            errorstate = HAL_SDMMC_CmdAppOperCommand(hsd->Instance, SDMMC_STD_CAPACITY);
            if (errorstate != HAL_SD_ERROR_NONE)
            {
                return HAL_SD_ERROR_UNSUPPORTED_FEATURE;
            }

            /* Get command response */
            response = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1);

            /* Get operating voltage*/
            validvoltage = (((response >> 31U) == 1U) ? 1U : 0U);

            HAL_Delay(1);
        }
        /* Card type is SDSC */
        hsd->SdCard.CardType = CARD_SDSC;
    }
    else
    {
        hsd->SdCard.CardVersion = CARD_V2_X;

        /* Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
        while (1)
        {
            if (count++ == SDMMC_MAX_VOLT_TRIAL)
            {
                return HAL_SD_ERROR_INVALID_VOLTRANGE;
            }

            /* SEND CMD55 APP_CMD with RCA as 0 */
            // errorstate = HAL_SDMMC_CmdAppCommand(hsd->Instance, 0U);
            // if (errorstate != HAL_SD_ERROR_NONE) {
            //   return HAL_ERROR;
            // }
            HAL_SDMMC_CmdAppCommand(hsd->Instance, 0U);

            /* Send ACMD41 */
            errorstate = HAL_SDMMC_CmdAppOperCommand(hsd->Instance, SDMMC_HIGH_CAPACITY);
            if (errorstate != HAL_SD_ERROR_NONE)
            {
                return HAL_ERROR;
            }

            /* Get command response */
            response = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1);

            /* Get operating voltage*/
            validvoltage = (((response >> 31U) == 1U) ? 1U : 0U);
            if (validvoltage == 1)
            {
                break;
            }
            else
            {
                HAL_Delay(5);
            }
        }

        if ((response & SDMMC_HIGH_CAPACITY) == SDMMC_HIGH_CAPACITY) /* (response &= SD_HIGH_CAPACITY) */
        {
            hsd->SdCard.CardType = CARD_SDHC_SDXC;
        }
        else
        {
            hsd->SdCard.CardType = CARD_SDSC;
        }
    }

    return HAL_SD_ERROR_NONE;
}

/**
 * @brief  Get the current SD's status.
 *
 * @param  hsd : Pointer to the SD handle structure.
 * @param  pSDstatus: Pointer to a buffer to store the SD status data.
 *
 * @retval uint32_t: Error state (HAL_SD_ERROR_NONE if successful, otherwise an
 * error code).
 */
static uint32_t SD_SendSDStatus(SD_HandleTypeDef *hsd, uint32_t *pSDstatus)
{
    SDIO_DataInitTypeDef config;
    uint32_t             errorstate = HAL_SD_ERROR_NONE;

    /* Check SD response */
    if ((HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
    {
        return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
    }

    /* Set block size for card if it is not equal to current block size for card
     */
    errorstate = HAL_SDMMC_CmdBlockLength(hsd->Instance, 64U);
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        hsd->ErrorCode |= HAL_SD_ERROR_NONE;
        return errorstate;
    }

    /* Send CMD55 */
    errorstate = HAL_SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        hsd->ErrorCode |= HAL_SD_ERROR_NONE;
        return errorstate;
    }

    /* Configure the Data Transfer */
    config.DataLength    = 64U;
    config.DataBlockSize = 64;
    config.TransferDir   = SDIO_TRANSFER_DIR_TO_SDIO;
    config.TransferMode  = SDIO_TRANSFER_MODE_BLOCK;
    HAL_SDIO_ConfigData(hsd->Instance, &config);

    HAL_SDIO_RXFIFO_Setup(hsd->Instance, config.DataLength);

    /* Send ACMD13 (SD_APP_STAUS)  with argument as card's RCA */
    errorstate = HAL_SDMMC_CmdStatusRegister(hsd->Instance);
    if (errorstate != HAL_SD_ERROR_NONE)
    {
        hsd->ErrorCode |= HAL_SD_ERROR_NONE;
        return errorstate;
    }

    errorstate = HAL_SDIO_WaitRX(hsd->Instance, 5000);
    if (errorstate == HAL_TIMEOUT)
    {
        printf("Read Block Timeout!\n");
        hsd->ErrorCode = HAL_SD_ERROR_TIMEOUT;
        // hsd->State = HAL_SD_STATE_READY;
        return HAL_TIMEOUT;
    }
    else if (errorstate == HAL_ERROR)
    {
        printf("Read Block Error!\n");
        // hsd->State = HAL_SD_STATE_READY;
        return HAL_ERROR;
    }
    HAL_SDIO_RXFIFO_Read(hsd->Instance, pSDstatus, config.DataLength);

    return HAL_SD_ERROR_NONE;
}

/**
 * @brief  Returns the current card's status.
 *
 * @param  hsd : Pointer to the SD handle structure.
 * @param  pCardStatus: pointer to the buffer that will contain the SD card
 *         status (Card Status register)
 *
 * @retval error state
 */
static uint32_t SD_SendStatus(SD_HandleTypeDef *hsd, uint32_t *pCardStatus)
{
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    if (pCardStatus == NULL)
    {
        return HAL_SD_ERROR_PARAM;
    }

    /* Send Status command */
    errorstate = HAL_SDMMC_CmdSendStatus(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Get SD card status */
    *pCardStatus = HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1);

    return HAL_SD_ERROR_NONE;
}

/**
 * @brief  Set the SDIO wide bus to 4-bits Mode.
 *
 * @param  hsd : Pointer to the SD handle structure.
 *
 * @retval uint32_t: error state
 */
static uint32_t SD_WideBus_Enable(SD_HandleTypeDef *hsd)
{
    // uint32_t scr[2U] = {0U, 0U};
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    /* It's a bit strange, how do you know that the command to respond to R1 has
     * been sent before calling this function? */
    /* Consider whether to subsequently delete */
    if ((HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
    {
        return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
    }

    /* Get SCR Register */
    // errorstate = SD_FindSCR(hsd, scr);
    // if(errorstate != HAL_OK)
    // {
    //   return errorstate;
    // }

    /* If requested card supports wide bus operation */
    // if((scr[1U] & SDMMC_WIDE_BUS_SUPPORT) != SDMMC_ALLZERO)
    // {
    /* Send CMD55 APP_CMD with argument as card's RCA.*/
    errorstate = HAL_SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
    errorstate = HAL_SDMMC_CmdBusWidth(hsd->Instance, 2U);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    return HAL_SD_ERROR_NONE;
}

/**
 * @brief  Set the SDIO wide bus to 1-bit Mode.
 *
 * @param  hsd : Pointer to the SD handle structure.
 *
 * @retval uint32_t : error state
 */
static uint32_t SD_WideBus_Disable(SD_HandleTypeDef *hsd)
{
    // uint32_t scr[2U] = {0U, 0U};
    uint32_t errorstate = HAL_SD_ERROR_NONE;

    if ((HAL_SDIO_GetResponse(hsd->Instance, SDIO_RESP1) & SDMMC_CARD_LOCKED) == SDMMC_CARD_LOCKED)
    {
        return HAL_SD_ERROR_LOCK_UNLOCK_FAILED;
    }

    /* Get SCR Register */
    // errorstate = SD_FindSCR(hsd, scr);
    // if(errorstate != HAL_OK)
    // {
    //   return errorstate;
    // }

    /* If requested card supports 1 bit mode operation */
    // if((scr[1U] & SDMMC_SINGLE_BUS_SUPPORT) != SDMMC_ALLZERO)
    // {
    /* Send CMD55 APP_CMD with argument as card's RCA */
    errorstate = HAL_SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)(hsd->SdCard.RelCardAdd << 16U));
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    /* Send ACMD6 APP_CMD with argument as 0 for single bus mode */
    errorstate = HAL_SDMMC_CmdBusWidth(hsd->Instance, 0U);
    if (errorstate != HAL_OK)
    {
        return errorstate;
    }

    return HAL_SD_ERROR_NONE;
}

#if 0
/**
 * @brief  Finds the SD card SCR register value.
 * @param  hsd Pointer to SD handle
 * @param  pSCR pointer to the buffer that will contain the SCR value
 * @retval error state
 */
static uint32_t SD_FindSCR(SD_HandleTypeDef *hsd, uint32_t *pSCR)
{
  SDIO_DataInitTypeDef config;
  uint32_t errorstate = HAL_SD_ERROR_NONE;
  // uint32_t tickstart = HAL_GetTick();
  // uint32_t index = 0U;
  uint32_t tempscr[2U] = {0U, 0U};
  
  /* Set Block Size To 8 Bytes */
  errorstate = HAL_SDMMC_CmdBlockLength(hsd->Instance, 8U);
  if(errorstate != HAL_OK)
  {
    return errorstate;
  }

  /* Send CMD55 APP_CMD with argument as card's RCA */
  errorstate = HAL_SDMMC_CmdAppCommand(hsd->Instance, (uint32_t)((hsd->SdCard.RelCardAdd) << 16U));
  if(errorstate != HAL_OK)
  {
    return errorstate;
  }
  
  /* Config Data transfer: read 8 byte data from Card */
  config.DataLength    = 8U;
  config.DataBlockSize = 8;
  config.TransferDir   = SDIO_TRANSFER_DIR_TO_SDIO;
  config.TransferMode  = SDIO_TRANSFER_MODE_BLOCK;
  HAL_SDIO_ConfigData(hsd->Instance, &config);

  HAL_SDIO_RXFIFO_Setup(hsd->Instance, config.DataLength, 5000);
  
  /* Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
  errorstate = HAL_SDMMC_CmdSendSCR(hsd->Instance);
  if(errorstate != HAL_OK)
  {
    return errorstate;
  }

  errorstate = HAL_SDIO_WaitRX(hsd->Instance, 5000);
  if(errorstate == HAL_TIMEOUT) {
    printf("Read Block Timeout!\n");
    hsd->ErrorCode = HAL_SD_ERROR_TIMEOUT;
    // hsd->State = HAL_SD_STATE_READY;
    return HAL_TIMEOUT;
  } else if (errorstate == HAL_ERROR) {
    printf("Read Block Error!\n");
    // hsd->State = HAL_SD_STATE_READY;
    return HAL_ERROR;
  }
  HAL_SDIO_RXFIFO_Read(hsd->Instance, tempscr, config.DataLength);
  
  *(pSCR + 1U) = ((tempscr[0U] & SDMMC_0TO7BITS) << 24U)  | ((tempscr[0U] & SDMMC_8TO15BITS) << 8U) |\
    ((tempscr[0U] & SDMMC_16TO23BITS) >> 8U) | ((tempscr[0U] & SDMMC_24TO31BITS) >> 24U);
  
  *(pSCR) = ((tempscr[1U] & SDMMC_0TO7BITS) << 24U)  | ((tempscr[1U] & SDMMC_8TO15BITS) << 8U) |\
    ((tempscr[1U] & SDMMC_16TO23BITS) >> 8U) | ((tempscr[1U] & SDMMC_24TO31BITS) >> 24U);

  return HAL_SD_ERROR_NONE;
}
#endif

/**
 * @brief  Handles the interrupt for the SDIO data reception (RX).
 *         This function processes the data transfer, manages the reception
 * buffer, and invokes the completion callback when all data is received. It
 * also handles multi-block and single-block reception.
 *
 * @param  arg: Pointer to the SD handle structure that contains the SD card
 * information.
 *
 * @retval None
 */
static void HAL_SD_RxIRQHandler(const void *arg)
{
    SDIO_DataInitTypeDef config     = { 0 };
    uint32_t             errorstate = HAL_SD_ERROR_NONE;
    SD_HandleTypeDef    *hsd        = (SD_HandleTypeDef *)arg;

    HAL_SDIO_Setup_Clear(hsd->Instance);

    HAL_SDIO_RXFIFO_Read(hsd->Instance, hsd->pRxBuffPtr, hsd->RxSingleSize);

    if (hsd->RxXferSize == hsd->RxSingleSize)
    {
        if (hsd->RxSingleSize / BLOCKSIZE > 1)
        {
            HAL_SDMMC_CmdStopTransfer(hsd->Instance);
        }
        if (hsd->RxCpltCallback)
        {
            hsd->RxCpltCallback(hsd);
        }
        hsd->State = HAL_SD_STATE_READY;
    }
    else
    {
        hsd->RxXferSize -= hsd->RxSingleSize;
        hsd->pRxBuffPtr += hsd->RxSingleSize;
        hsd->XferBlockAdd += hsd->RxSingleSize / BLOCKSIZE;
        if (hsd->RxXferSize > SD_TRANS_BLOCK_MAX * BLOCKSIZE)
        {
            hsd->RxSingleSize = SD_TRANS_BLOCK_MAX * BLOCKSIZE;
        }
        else
        {
            hsd->RxSingleSize = hsd->RxXferSize;
        }

        config.DataLength    = hsd->RxSingleSize;
        config.DataBlockSize = BLOCKSIZE;
        config.TransferDir   = SDIO_TRANSFER_DIR_TO_SDIO;
        config.TransferMode  = SDIO_TRANSFER_MODE_BLOCK;
        HAL_SDIO_ConfigData(hsd->Instance, &config);

        errorstate = HAL_SDIO_RXFIFO_Setup(hsd->Instance, hsd->RxSingleSize);

        /* Read Blocks in IT mode */
        if ((hsd->RxSingleSize / BLOCKSIZE) > 1U)
        {
            hsd->Context = (SD_CONTEXT_READ_MULTIPLE_BLOCK | SD_CONTEXT_IT);

            /* Read Multi Block command */
            errorstate = HAL_SDMMC_CmdReadMultiBlock(hsd->Instance, hsd->XferBlockAdd);
        }
        else
        {
            hsd->Context = (SD_CONTEXT_READ_SINGLE_BLOCK | SD_CONTEXT_IT);

            /* Read Single Block command */
            errorstate = HAL_SDMMC_CmdReadSingleBlock(hsd->Instance, hsd->XferBlockAdd);
        }
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            hsd->ErrorCode |= errorstate;
            hsd->State = HAL_SD_STATE_READY;
            return;
        }
    }
}

/**
 * @brief  Handles the interrupt for the SDIO data transmission (TX).
 *         This function processes the data transfer, manages the transmission
 * buffer, and invokes the completion callback when all data is transmitted. It
 * also handles multi-block and single-block transmission.
 *
 * @param  arg: Pointer to the SD handle structure that contains the SD card
 * information.
 *
 * @retval None
 */
static void HAL_SD_TxIRQHandler(const void *arg)
{

    SDIO_DataInitTypeDef config     = { 0 };
    uint32_t             errorstate = HAL_SD_ERROR_NONE;
    SD_HandleTypeDef    *hsd        = (SD_HandleTypeDef *)arg;

    HAL_Delay(1);
    HAL_SDIO_Setup_Clear(hsd->Instance);

    if (hsd->TxXferSize == hsd->TxSingleSize)
    {
        if (hsd->TxSingleSize / BLOCKSIZE > 1)
        {
            HAL_SDMMC_CmdStopTransfer(hsd->Instance);
        }
        if (hsd->TxCpltCallback)
        {
            hsd->TxCpltCallback(hsd);
        }
        hsd->State = HAL_SD_STATE_READY;
    }
    else
    {
        hsd->TxXferSize -= hsd->TxSingleSize;
        hsd->pTxBuffPtr += hsd->TxSingleSize;
        hsd->XferBlockAdd += hsd->TxSingleSize / BLOCKSIZE;
        if (hsd->TxXferSize > SD_TRANS_BLOCK_MAX * BLOCKSIZE)
        {
            hsd->TxSingleSize = SD_TRANS_BLOCK_MAX * BLOCKSIZE;
        }
        else
        {
            hsd->TxSingleSize = hsd->TxXferSize;
        }

        config.DataLength    = hsd->TxSingleSize;
        config.DataBlockSize = BLOCKSIZE;
        config.TransferDir   = SDIO_TRANSFER_DIR_TO_CARD;
        config.TransferMode  = SDIO_TRANSFER_MODE_BLOCK;
        HAL_SDIO_ConfigData(hsd->Instance, &config);

        errorstate = HAL_SDIO_TXFIFO_Write(hsd->Instance, hsd->pTxBuffPtr, hsd->TxSingleSize);
        if ((hsd->TxSingleSize / BLOCKSIZE) > 1)
        {
            hsd->Context = (SD_CONTEXT_WRITE_MULTIPLE_BLOCK | SD_CONTEXT_IT);

            /* Write Multi Block command */
            errorstate = HAL_SDMMC_CmdWriteMultiBlock(hsd->Instance, hsd->XferBlockAdd);
        }
        else
        {
            hsd->Context = (SD_CONTEXT_WRITE_SINGLE_BLOCK | SD_CONTEXT_IT);

            /* Write Single Block command */
            errorstate = HAL_SDMMC_CmdWriteSingleBlock(hsd->Instance, hsd->XferBlockAdd);
        }
        if (errorstate != HAL_SD_ERROR_NONE)
        {
            hsd->ErrorCode |= errorstate;
            hsd->State = HAL_SD_STATE_READY;
            return;
        }
    }
}

static void HAL_SD_EotIRQHandler(const void *arg)
{
    UNUSED(arg);
}

static void HAL_SD_ErrorIRQHandler(const void *arg)
{
    UNUSED(arg);
}

/**
 * @}
 */
