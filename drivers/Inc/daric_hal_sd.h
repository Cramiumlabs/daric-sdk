/**
 ******************************************************************************
 * @file    daric_hal_sd.h
 * @author  SD Team
 * @brief   Header file of SD HAL module.
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
#ifndef __DARIC_HAL_SD_H
#define __DARIC_HAL_SD_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>

#include "daric_hal_sdio.h"
    /* Exported Enumeration
     * ------------------------------------------------------------ */
    /**
     * @brief SD State enumeration
     */
    typedef enum
    {
        HAL_SD_STATE_RESET       = 0x00000000U, /*!< SD not yet initialized or disabled */
        HAL_SD_STATE_READY       = 0x00000001U, /*!< SD initialized and ready for use */
        HAL_SD_STATE_TIMEOUT     = 0x00000002U, /*!< SD Timeout state */
        HAL_SD_STATE_BUSY        = 0x00000003U, /*!< SD process ongoing */
        HAL_SD_STATE_PROGRAMMING = 0x00000004U, /*!< SD Programming State */
        HAL_SD_STATE_RECEIVING   = 0x00000005U, /*!< SD Receinving State */
        HAL_SD_STATE_TRANSFER    = 0x00000006U, /*!< SD Transfert State */
        HAL_SD_STATE_ERROR       = 0x0000000FU  /*!< SD is in error state */
    } HAL_SD_StateTypeDef;

    /**
     * @brief SD Card State enumeration
     */
    typedef enum
    {
        HAL_SD_CARD_IDLE           = 0,           /*!< Card state is idle */
        HAL_SD_CARD_READY          = 0x00000001U, /*!< Card state is ready */
        HAL_SD_CARD_IDENTIFICATION = 0x00000002U, /*!< Card is in identification state */
        HAL_SD_CARD_STANDBY        = 0x00000003U, /*!< Card is in standby state */
        HAL_SD_CARD_TRANSFER       = 0x00000004U, /*!< Card is in transfer state */
        HAL_SD_CARD_SENDING        = 0x00000005U, /*!< Card is sending an operation */
        HAL_SD_CARD_RECEIVING      = 0x00000006U, /*!< Card is receiving operation information */
        HAL_SD_CARD_PROGRAMMING    = 0x00000007U, /*!< Card is in programming state */
        HAL_SD_CARD_DISCONNECTED   = 0x00000008U, /*!< Card is disconnected */
        HAL_SD_CARD_ERROR          = 0x000000FFU  /*!< Card response Error */
    } HAL_SD_CardStateTypeDef;

    /**
     * @brief SD Callback ID enumeration definition
     */
    typedef enum
    {
        HAL_SD_TX_CPLT_CB_ID = 0x00U, /*!< SD Tx Complete Callback ID */
        HAL_SD_RX_CPLT_CB_ID = 0x01U, /*!< SD Rx Complete Callback ID */
        HAL_SD_EOT_CB_ID     = 0x02U, /*!< SD Eot Callback ID */
        HAL_SD_ERROR_CB_ID   = 0x03U, /*!< SD Error Callback ID */
    } HAL_SD_CallbackIDTypeDef;

    /* Exported Structure
     * ------------------------------------------------------------ */
    /**
     * @brief  SD Card Information Structure definition
     */
    typedef struct
    {
        uint32_t CardType; /*!< Specifies the card Type */

        uint32_t CardVersion; /*!< Specifies the card version */

        uint32_t Class; /*!< Specifies the class of the card class */

        uint32_t RelCardAdd; /*!< Specifies the Relative Card Address */

        uint32_t BlockNbr; /*!< Specifies the Card Capacity in blocks */

        uint32_t BlockSize; /*!< Specifies one block size in bytes */

        uint32_t LogBlockNbr; /*!< Specifies the Card logical Capacity in blocks */

        uint32_t LogBlockSize; /*!< Specifies logical block size in bytes */

    } HAL_SD_CardInfoTypeDef;

    /**
     * @brief  SD handle Structure definition
     */
    typedef struct __SD_HandleTypeDef
    {
        uint32_t ID; /*!< SD ID */

        SDIO_InitTypeDef Init; /*!< SD required parameters */

        SDIO_HandleTypeDef *Instance; /*!< pointer to SDIO Instance */

        uint32_t *pTxBuffPtr; /*!< Pointer to SD Tx transfer Buffer */

        uint32_t TxXferSize; /*!< SD Tx Transfer size */

        uint32_t TxSingleSize; /*!< SD Tx Single Transfer size for IT */

        uint32_t *pRxBuffPtr; /*!< Pointer to SD Rx transfer Buffer */

        uint32_t RxXferSize; /*!< SD Rx Transfer size */

        uint32_t RxSingleSize; /*!< SD Rx Single Transfer size for IT */

        uint32_t XferBlockAdd; /*!< SD Transfer Block Address */

        __IO uint32_t Context; /*!< SD transfer context */

        __IO HAL_SD_StateTypeDef State; /*!< SD card State */

        __IO uint32_t ErrorCode; /*!< SD Card Error codes */

        HAL_SD_CardInfoTypeDef SdCard; /*!< SD Card information */

        uint32_t CSD[4]; /*!< SD card specific data table */

        uint32_t CID[4]; /*!< SD card identification number table */

        void (*TxCpltCallback)(struct __SD_HandleTypeDef *hsd);
        void (*RxCpltCallback)(struct __SD_HandleTypeDef *hsd);
        void (*EotCallback)(struct __SD_HandleTypeDef *hsd);
        void (*ErrorCallback)(struct __SD_HandleTypeDef *hsd);

    } SD_HandleTypeDef;

    /**
     * @brief  SDCard CSD Information Structure definition
     */
    typedef struct
    {
        __IO uint8_t  CSDStruct;           /*!< CSD structure                         */
        __IO uint8_t  SysSpecVersion;      /*!< System specification version          */
        __IO uint8_t  Reserved1;           /*!< Reserved                              */
        __IO uint8_t  TAAC;                /*!< Data read access time 1               */
        __IO uint8_t  NSAC;                /*!< Data read access time 2 in CLK cycles */
        __IO uint8_t  MaxBusClkFrec;       /*!< Max. bus clock frequency              */
        __IO uint16_t CardComdClasses;     /*!< Card command classes                  */
        __IO uint8_t  RdBlockLen;          /*!< Max. read data block length           */
        __IO uint8_t  PartBlockRead;       /*!< Partial blocks for read allowed       */
        __IO uint8_t  WrBlockMisalign;     /*!< Write block misalignment              */
        __IO uint8_t  RdBlockMisalign;     /*!< Read block misalignment               */
        __IO uint8_t  DSRImpl;             /*!< DSR implemented                       */
        __IO uint8_t  Reserved2;           /*!< Reserved                              */
        __IO uint32_t DeviceSize;          /*!< Device Size                           */
        __IO uint8_t  MaxRdCurrentVDDMin;  /*!< Max. read current @ VDD min           */
        __IO uint8_t  MaxRdCurrentVDDMax;  /*!< Max. read current @ VDD max           */
        __IO uint8_t  MaxWrCurrentVDDMin;  /*!< Max. write current @ VDD min          */
        __IO uint8_t  MaxWrCurrentVDDMax;  /*!< Max. write current @ VDD max          */
        __IO uint8_t  DeviceSizeMul;       /*!< Device size multiplier                */
        __IO uint8_t  EraseGrSize;         /*!< Erase group size                      */
        __IO uint8_t  EraseGrMul;          /*!< Erase group size multiplier           */
        __IO uint8_t  WrProtectGrSize;     /*!< Write protect group size              */
        __IO uint8_t  WrProtectGrEnable;   /*!< Write protect group enable            */
        __IO uint8_t  ManDeflECC;          /*!< Manufacturer default ECC              */
        __IO uint8_t  WrSpeedFact;         /*!< Write speed factor                    */
        __IO uint8_t  MaxWrBlockLen;       /*!< Max. write data block length          */
        __IO uint8_t  WriteBlockPaPartial; /*!< Partial blocks for write allowed */
        __IO uint8_t  Reserved3;           /*!< Reserved                              */
        __IO uint8_t  ContentProtectAppli; /*!< Content protection application */
        __IO uint8_t  FileFormatGrouop;    /*!< File format group                     */
        __IO uint8_t  CopyFlag;            /*!< Copy flag (OTP)                       */
        __IO uint8_t  PermWrProtect;       /*!< Permanent write protection            */
        __IO uint8_t  TempWrProtect;       /*!< Temporary write protection            */
        __IO uint8_t  FileFormat;          /*!< File format                           */
        __IO uint8_t  ECC;                 /*!< ECC code                              */
        __IO uint8_t  CSD_CRC;             /*!< CSD CRC                               */
        __IO uint8_t  Reserved4;           /*!< Always 1                              */

    } HAL_SD_CardCSDTypeDef;

    /**
     * @brief  SDCard CID Information Structure definition
     */
    typedef struct
    {
        __IO uint8_t  ManufacturerID; /*!< Manufacturer ID       */
        __IO uint16_t OEM_AppliID;    /*!< OEM/Application ID    */
        __IO uint32_t ProdName1;      /*!< Product Name part1    */
        __IO uint8_t  ProdName2;      /*!< Product Name part2    */
        __IO uint8_t  ProdRev;        /*!< Product Revision      */
        __IO uint32_t ProdSN;         /*!< Product Serial Number */
        __IO uint8_t  Reserved1;      /*!< Reserved1             */
        __IO uint16_t ManufactDate;   /*!< Manufacturing Date    */
        __IO uint8_t  CID_CRC;        /*!< CID CRC               */
        __IO uint8_t  Reserved2;      /*!< Always 1              */

    } HAL_SD_CardCIDTypeDef;

    /**
     * @brief  SD Card Status returned by ACMD13
     */
    typedef struct
    {
        __IO uint8_t  DataBusWidth;      /*!< Shows the currently defined data bus width */
        __IO uint8_t  SecuredMode;       /*!< Card is in secured mode of operation  */
        __IO uint16_t CardType;          /*!< Carries information about card type    */
        __IO uint32_t ProtectedAreaSize; /*!< Carries information about the capacity
                                            of protected area   */
        __IO uint8_t SpeedClass;         /*!< Carries information about
                                            the speed class of the card */
        __IO uint8_t PerformanceMove;    /*!< Carries information about the card's
                                            performance move      */
        __IO uint8_t AllocationUnitSize; /*!< Carries information about the card's
                                            allocation unit size  */
        __IO uint16_t EraseSize;         /*!< Determines the number of AUs to be erased
                                           in one operation */
        __IO uint8_t EraseTimeout;       /*!< Determines the timeout for
                                                  any number of AU erase */
        __IO uint8_t EraseOffset;        /*!< Carries information about the erase offset */

    } HAL_SD_SDStatusTypeDef;

/* Exported constants --------------------------------------------------------*/
/** @defgroup SD_Exported_Constants Exported Constants
 * @{
 */
#define SD_NUM_MAX (1)
#define SD0_ID     (0)

#define BLOCKSIZE 512U /*!< Block size is 512 bytes */

/** @defgroup SD_Exported_Constansts_Group1
 * @brief     SD Error status
 * @{
 */
#define HAL_SD_ERROR_NONE SDMMC_ERROR_NONE /*!< No error */
#define HAL_SD_ERROR_CMD_CRC_FAIL                                          \
    SDMMC_ERROR_CMD_CRC_FAIL /*!< Command response received (but CRC check \
                                failed)              */
#define HAL_SD_ERROR_DATA_CRC_FAIL                                                                                \
    SDMMC_ERROR_DATA_CRC_FAIL                                    /*!< Data block sent/received (CRC check failed) \
                                                                  */
#define HAL_SD_ERROR_CMD_RSP_TIMEOUT SDMMC_ERROR_CMD_RSP_TIMEOUT /*!< Command response timeout */
#define HAL_SD_ERROR_DATA_TIMEOUT                                                  \
    SDMMC_ERROR_DATA_TIMEOUT                                     /*!< Data timeout \
                                                                  */
#define HAL_SD_ERROR_TX_UNDERRUN     SDMMC_ERROR_TX_UNDERRUN     /*!< Transmit FIFO underrun */
#define HAL_SD_ERROR_RX_OVERRUN      SDMMC_ERROR_RX_OVERRUN      /*!< Receive FIFO overrun */
#define HAL_SD_ERROR_ADDR_MISALIGNED SDMMC_ERROR_ADDR_MISALIGNED /*!< Misaligned address */
#define HAL_SD_ERROR_BLOCK_LEN_ERR                                             \
    SDMMC_ERROR_BLOCK_LEN_ERR /*!< Transferred block length is not allowed for \
                                 the card or the number of transferred bytes   \
                                 does not match the block length   */
#define HAL_SD_ERROR_ERASE_SEQ_ERR                                                                              \
    SDMMC_ERROR_ERASE_SEQ_ERR                                    /*!< An error in the sequence of erase command \
                                                                    occurs              */
#define HAL_SD_ERROR_BAD_ERASE_PARAM SDMMC_ERROR_BAD_ERASE_PARAM /*!< An invalid selection for erase groups */
#define HAL_SD_ERROR_WRITE_PROT_VIOLATION                                    \
    SDMMC_ERROR_WRITE_PROT_VIOLATION /*!< Attempt to program a write protect \
                                        block                      */
#define HAL_SD_ERROR_LOCK_UNLOCK_FAILED                                                                      \
    SDMMC_ERROR_LOCK_UNLOCK_FAILED                             /*!< Sequence or password error has been      \
                                                                  detected in unlock command or if there was \
                                                                  an attempt to access a locked card    */
#define HAL_SD_ERROR_COM_CRC_FAILED SDMMC_ERROR_COM_CRC_FAILED /*!< CRC check of the previous command failed */
#define HAL_SD_ERROR_ILLEGAL_CMD    SDMMC_ERROR_ILLEGAL_CMD    /*!< Command is not legal for the card state */
#define HAL_SD_ERROR_CARD_ECC_FAILED                                                                                      \
    SDMMC_ERROR_CARD_ECC_FAILED                                          /*!< Card internal ECC was applied but failed to \
                                                                            correct the data  */
#define HAL_SD_ERROR_CC_ERR              SDMMC_ERROR_CC_ERR              /*!< Internal card controller error */
#define HAL_SD_ERROR_GENERAL_UNKNOWN_ERR SDMMC_ERROR_GENERAL_UNKNOWN_ERR /*!< General or unknown error */
#define HAL_SD_ERROR_STREAM_READ_UNDERRUN                                 \
    SDMMC_ERROR_STREAM_READ_UNDERRUN /*!< The card could not sustain data \
                                        reading in stream rmode       */
#define HAL_SD_ERROR_STREAM_WRITE_OVERRUN                                                                 \
    SDMMC_ERROR_STREAM_WRITE_OVERRUN                                 /*!< The card could not sustain data \
                                                                        programming in stream mode    */
#define HAL_SD_ERROR_CID_CSD_OVERWRITE SDMMC_ERROR_CID_CSD_OVERWRITE /*!< CID/CSD overwrite error */
#define HAL_SD_ERROR_WP_ERASE_SKIP     SDMMC_ERROR_WP_ERASE_SKIP     /*!< Only partial address space was erased */
#define HAL_SD_ERROR_CARD_ECC_DISABLED                                         \
    SDMMC_ERROR_CARD_ECC_DISABLED /*!< Command has been executed without using \
                                     internal ECC          */
#define HAL_SD_ERROR_ERASE_RESET                                                                                      \
    SDMMC_ERROR_ERASE_RESET                                          /*!< Erase sequence was cleared before executing \
                                                                        because an out of erase sequence command was  \
                                                                        received                        */
#define HAL_SD_ERROR_AKE_SEQ_ERR       SDMMC_ERROR_AKE_SEQ_ERR       /*!< Error in sequence of authentication */
#define HAL_SD_ERROR_INVALID_VOLTRANGE SDMMC_ERROR_INVALID_VOLTRANGE /*!< Error in case of invalid voltage range */
#define HAL_SD_ERROR_ADDR_OUT_OF_RANGE                                      \
    SDMMC_ERROR_ADDR_OUT_OF_RANGE /*!< Error when addressed block is out of \
                                     range                    */
#define HAL_SD_ERROR_REQUEST_NOT_APPLICABLE                                                     \
    SDMMC_ERROR_REQUEST_NOT_APPLICABLE                   /*!< Error when command request is not \
                                                            applicable                  */
#define HAL_SD_ERROR_PARAM SDMMC_ERROR_INVALID_PARAMETER /*!< the used parameter is not valid */
#define HAL_SD_ERROR_UNSUPPORTED_FEATURE                                                    \
    SDMMC_ERROR_UNSUPPORTED_FEATURE              /*!< Error when feature is not insupported \
                                                  */
#define HAL_SD_ERROR_BUSY    SDMMC_ERROR_BUSY    /*!< Error when transfer process is busy */
#define HAL_SD_ERROR_DMA     SDMMC_ERROR_DMA     /*!< Error while DMA transfer */
#define HAL_SD_ERROR_TIMEOUT SDMMC_ERROR_TIMEOUT /*!< Timeout error */

#define HAL_SD_ERROR_INVALID_CALLBACK SDMMC_ERROR_INVALID_PARAMETER /*!< Invalid callback error */

/**
 * @}
 */

/** @defgroup SD_Exported_Constansts_Group2
 * @brief     SD Operation context
 * @{
 */
#define SD_CONTEXT_NONE                 0x00000000U /*!< None                             */
#define SD_CONTEXT_READ_SINGLE_BLOCK    0x00000001U /*!< Read single block operation      */
#define SD_CONTEXT_READ_MULTIPLE_BLOCK  0x00000002U /*!< Read multiple blocks operation   */
#define SD_CONTEXT_WRITE_SINGLE_BLOCK   0x00000010U /*!< Write single block operation     */
#define SD_CONTEXT_WRITE_MULTIPLE_BLOCK 0x00000020U /*!< Write multiple blocks operation  */
#define SD_CONTEXT_IT                   0x00000008U /*!< Process in Interrupt mode        */
#define SD_CONTEXT_DMA                  0x00000080U /*!< Process in DMA mode              */

/**
 * @}
 */

/** @defgroup SD_Exported_Constansts_Group3
 * @brief     SD Supported Memory Cards
 * @{
 */
#define CARD_SDSC      0x00000000U
#define CARD_SDHC_SDXC 0x00000001U
#define CARD_SECURED   0x00000003U

/**
 * @}
 */

/** @defgroup SD_Exported_Constansts_Group4
 * @brief     SD Supported Version
 * @{
 */
#define CARD_V1_X 0x00000000U
#define CARD_V2_X 0x00000001U
    /**
     * @}
     */

    /**
     * @}
     */

    /* Exported macro ------------------------------------------------------------*/
    /* Exported functions --------------------------------------------------------*/
    /** @defgroup SD_Exported_Functions_Group1
     * @brief     Initialization and De-Initialization functions
     * @{
     */
    HAL_StatusTypeDef HAL_SD_Init(SD_HandleTypeDef *hsd);
    HAL_StatusTypeDef HAL_SD_DeInit(SD_HandleTypeDef *hsd);
    /**
     * @}
     */

    /** @defgroup SD_Exported_Functions_Group2
     * @brief     Register/Unregister ISR Function
     * @{
     */
    typedef void (*pSD_CallbackTypeDef)(SD_HandleTypeDef *hsd);

    HAL_StatusTypeDef HAL_SD_RegisterCallback(SD_HandleTypeDef *hsd, HAL_SD_CallbackIDTypeDef CallbackId, pSD_CallbackTypeDef pCallback);
    HAL_StatusTypeDef HAL_SD_UnRegisterCallback(SD_HandleTypeDef *hsd, HAL_SD_CallbackIDTypeDef CallbackId);
    /**
     * @}
     */

    /** @defgroup SD_Exported_Functions_Group3
     * @brief     Input and Output operation Functions
     * @{
     */
    /* Blocking mode: Polling */
    HAL_StatusTypeDef HAL_SD_ReadBlocks(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout);
    HAL_StatusTypeDef HAL_SD_WriteBlocks(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks, uint32_t Timeout);
    /* Non-Blocking mode: IT */
    HAL_StatusTypeDef HAL_SD_ReadBlocks_IT(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
    HAL_StatusTypeDef HAL_SD_WriteBlocks_IT(SD_HandleTypeDef *hsd, uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
    HAL_StatusTypeDef HAL_SD_Erase(SD_HandleTypeDef *hsd, uint32_t BlockStartAdd, uint32_t BlockEndAdd);
    HAL_StatusTypeDef HAL_SD_Abort(SD_HandleTypeDef *hsd);

    /**
     * @}
     */

    /** @defgroup SD_Exported_Functions_Group4
     * @brief     Peripheral Control Functions
     * @{
     */
    HAL_StatusTypeDef HAL_SD_ConfigWideBusOperation(SD_HandleTypeDef *hsd, uint32_t WideMode);
    /**
     * @}
     */

    /** @defgroup SD_Exported_Functions_Group5
     * @brief     SD Card Related Functions
     * @{
     */
    HAL_StatusTypeDef HAL_SD_SendSDStatus(SD_HandleTypeDef *hsd, uint32_t *pSDstatus);

    HAL_SD_CardStateTypeDef HAL_SD_GetCardState(SD_HandleTypeDef *hsd);
    HAL_StatusTypeDef       HAL_SD_GetCardCID(SD_HandleTypeDef *hsd, HAL_SD_CardCIDTypeDef *pCID);
    HAL_StatusTypeDef       HAL_SD_GetCardCSD(SD_HandleTypeDef *hsd, HAL_SD_CardCSDTypeDef *pCSD);
    HAL_StatusTypeDef       HAL_SD_GetSDStatus(SD_HandleTypeDef *hsd, HAL_SD_SDStatusTypeDef *pStatus);
    HAL_StatusTypeDef       HAL_SD_GetCardInfo(SD_HandleTypeDef *hsd, HAL_SD_CardInfoTypeDef *pCardInfo);
    /**
     * @}
     */

    /** @defgroup SD_Exported_Functions_Group6
     * @brief     Peripheral State and Errors Functions
     * @{
     */
    HAL_SD_StateTypeDef HAL_SD_GetState(SD_HandleTypeDef *hsd);
    uint32_t            HAL_SD_GetError(SD_HandleTypeDef *hsd);
    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif