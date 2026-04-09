/**
 ******************************************************************************
 * @file    daric_hal_sdio.h
 * @author  SDIO Team
 * @brief   Header file of SDIO HAL module.
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
#ifndef __DARIC_HAL_SDIO_H
#define __DARIC_HAL_SDIO_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "daric_hal_def.h"
    /* Exported Enumeration
     * ------------------------------------------------------------*/
    /**
     * @brief SDIO State Type enumeration.
     */
    typedef enum
    {
        HAL_SDIO_STATE_RESET   = 0x00U, /* I2C is not yet Initialized */
        HAL_SDIO_STATE_READY   = 0x10U, /* I2C Initialized and ready for use */
        HAL_SDIO_STATE_BUSY_TX = 0x20U, /* Data Transmission process is ongoing */
        HAL_SDIO_STATE_BUSY_RX = 0x40U, /* Data Reception process is ongoing */
        HAL_SDIO_STATE_TIMEOUT = 0x80U, /* Timeout state */
        HAL_SDIO_STATE_ERROR   = 0xE0U  /* Error */
    } SDIO_StateTypeDef;

    /* Exported Structure
     * ------------------------------------------------------------*/
    /**
     * @brief  SDIO Initialization Structure definition
     */
    typedef struct
    {
        uint32_t BusWide; /*!< Specifies the bus width of SDIO controller. */
        uint32_t Clock;   /*!< Specifies the clock frequency of the SDIO controller. */
    } SDIO_InitTypeDef;

    /**
     * @brief  SDIO handle Structure definition
     */
    typedef struct
    {
        uint32_t          instance_id;      /*!< SDIO ID */
        SDIO_StateTypeDef state;            /*!< SDIO states */
        SDIO_InitTypeDef  Init;             /*!< SDIO initialization parameters */
        uint32_t          rx_transfer_size; /*!< Record the transfer size of RX */
    } SDIO_HandleTypeDef;

    /**
     * @brief  SDIO Command Control structure
     */
    typedef struct
    {
        uint32_t Argument; /*!< Specifies the SDIO Command Argument which is sent
                                to a card as part of a command message. */

        uint32_t CmdIndex; /*!< Specifies the SDIO command index. It must be Min_Data
                              = 0 and Max_Data = 64 */

        uint32_t Response; /*!< Specifies the types of Response corresponding to the
                              SDIO Command. */

    } SDIO_CmdInitTypeDef;

    /**
     * @brief  SDIO Data Control structure
     */
    typedef struct
    {
        uint32_t DataLength; /*!< Specifies the number of data bytes
                                to be transferred. */

        uint32_t DataBlockSize; /*!< Specifies the data block size
                                for block transfer. */

        uint32_t TransferDir; /*!< Specifies the data transfer direction, whether the
                                transfer is a read or write. */

        uint32_t TransferMode; /*!< Specifies whether data transfer is in stream or
                                block mode. */

    } SDIO_DataInitTypeDef;

/* Exported Macro --------------------------------------------------------*/
/** @defgroup SDIO NUM
 * @{
 */
#define SDIO_MAX_NUM (1)
#define SDIO0_ID     (0)
/**
 * @}
 */

/** @defgroup SDMMC Error Type definition
 * @{
 */
#define SDMMC_ERROR_NONE          0x00000000U /*!< No error */
#define SDMMC_ERROR_CMD_CRC_FAIL  0x00000001U /*!< Command response received (but CRC check failed) */
#define SDMMC_ERROR_DATA_CRC_FAIL 0x00000002U /*!< Data block sent/received (CRC check failed) */
#define SDMMC_ERROR_CMD_RSP_TIMEOUT                                           \
    0x00000004U                                 /*!< Command response timeout \
                                                 */
#define SDMMC_ERROR_DATA_TIMEOUT    0x00000008U /*!< Data timeout */
#define SDMMC_ERROR_TX_UNDERRUN     0x00000010U /*!< Transmit FIFO underrun */
#define SDMMC_ERROR_RX_OVERRUN      0x00000020U /*!< Receive FIFO overrun */
#define SDMMC_ERROR_ADDR_MISALIGNED 0x00000040U /*!< Misaligned address */
#define SDMMC_ERROR_BLOCK_LEN_ERR                                                                                     \
    0x00000080U                                      /*!< Transferred block length is not allowed for the card or the \
                                                           number of transferred bytes does not match the block       \
                                                        length   */
#define SDMMC_ERROR_ERASE_SEQ_ERR        0x00000100U /*!< An error in the sequence of erase command occurs */
#define SDMMC_ERROR_BAD_ERASE_PARAM      0x00000200U /*!< An invalid selection for erase groups */
#define SDMMC_ERROR_WRITE_PROT_VIOLATION 0x00000400U /*!< Attempt to program a write protect block */
#define SDMMC_ERROR_LOCK_UNLOCK_FAILED                                                                         \
    0x00000800U                                /*!< Sequence or password error has been detected in unlock     \
                                                    command or if there was an attempt to access a locked card \
                                                */
#define SDMMC_ERROR_COM_CRC_FAILED 0x00001000U /*!< CRC check of the previous command failed */
#define SDMMC_ERROR_ILLEGAL_CMD    0x00002000U /*!< Command is not legal for the card state */
#define SDMMC_ERROR_CARD_ECC_FAILED                                                                               \
    0x00004000U                                      /*!< Card internal ECC was applied but failed to correct the \
                                                        data  */
#define SDMMC_ERROR_CC_ERR               0x00008000U /*!< Internal card controller error */
#define SDMMC_ERROR_GENERAL_UNKNOWN_ERR  0x00010000U /*!< General or unknown error */
#define SDMMC_ERROR_STREAM_READ_UNDERRUN 0x00020000U /*!< The card could not sustain data reading in stream rmode */
#define SDMMC_ERROR_STREAM_WRITE_OVERRUN                                                                          \
    0x00040000U                                   /*!< The card could not sustain data programming in stream mode \
                                                   */
#define SDMMC_ERROR_CID_CSD_OVERWRITE 0x00080000U /*!< CID/CSD overwrite error */
#define SDMMC_ERROR_WP_ERASE_SKIP     0x00100000U /*!< Only partial address space was erased */
#define SDMMC_ERROR_CARD_ECC_DISABLED 0x00200000U /*!< Command has been executed without using internal ECC */
#define SDMMC_ERROR_ERASE_RESET                                                                                        \
    0x00400000U                                        /*!< Erase sequence was cleared before executing because an out \
                                                            of erase sequence command was received */
#define SDMMC_ERROR_AKE_SEQ_ERR            0x00800000U /*!< Error in sequence of authentication */
#define SDMMC_ERROR_INVALID_VOLTRANGE      0x01000000U /*!< Error in case of invalid voltage range */
#define SDMMC_ERROR_ADDR_OUT_OF_RANGE      0x02000000U /*!< Error when addressed block is out of range */
#define SDMMC_ERROR_REQUEST_NOT_APPLICABLE 0x04000000U /*!< Error when command request is not applicable */
#define SDMMC_ERROR_INVALID_PARAMETER      0x08000000U /*!< the used parameter is not valid */
#define SDMMC_ERROR_UNSUPPORTED_FEATURE    0x10000000U /*!< Error when feature is not insupported */
#define SDMMC_ERROR_BUSY                                                         \
    0x20000000U                         /*!< Error when transfer process is busy \
                                         */
#define SDMMC_ERROR_DMA     0x40000000U /*!< Error while DMA transfer */
#define SDMMC_ERROR_TIMEOUT 0x80000000U /*!< Timeout error */
/**
 * @}
 */

/**
 * @brief SDIO Commands Index
 */
#define SDMMC_CMD_GO_IDLE_STATE                  \
    ((uint8_t)0) /*!< Resets the SD memory card. \
                  */
#define SDMMC_CMD_SEND_OP_COND                                                  \
    ((uint8_t)1) /*!< Sends host capacity support information and activates the \
                    card's initialization process. */
#define SDMMC_CMD_ALL_SEND_CID                                                                       \
    ((uint8_t)2)                            /*!< Asks any card connected to the host to send the CID \
                                               numbers on the CMD line.             */
#define SDMMC_CMD_SET_REL_ADDR ((uint8_t)3) /*!< Asks the card to publish a new relative address (RCA). */
#define SDMMC_CMD_SET_DSR      ((uint8_t)4) /*!< Programs the DSR of all cards. */
#define SDMMC_CMD_SDMMC_SEN_OP_COND                                              \
    ((uint8_t)5) /*!< Sends host capacity support information (HCS) and asks the \
                   accessed card to send its operating condition register (OCR)  \
                   content in the response on the CMD line.                  */
#define SDMMC_CMD_HS_SWITCH                                               \
    ((uint8_t)6) /*!< Checks switchable function (mode 0) and switch card \
                    function (mode 1).                   */
#define SDMMC_CMD_SEL_DESEL_CARD                                            \
    ((uint8_t)7) /*!< Selects the card by its own relative address and gets \
                    deselected by any other address    */
#define SDMMC_CMD_HS_SEND_EXT_CSD                                              \
    ((uint8_t)8) /*!< Sends SD Memory Card interface condition, which includes \
                   host supply voltage information and asks the card whether   \
                   card supports voltage. */
#define SDMMC_CMD_SEND_CSD                                                     \
    ((uint8_t)9) /*!< Addressed card sends its card specific data (CSD) on the \
                    CMD line.                       */
#define SDMMC_CMD_SEND_CID                                                                                         \
    ((uint8_t)10)                                   /*!< Addressed card sends its card identification (CID) on the \
                                                       CMD line.                      */
#define SDMMC_CMD_READ_DAT_UNTIL_STOP ((uint8_t)11) /*!< SD card doesn't support it. */
#define SDMMC_CMD_STOP_TRANSMISSION   ((uint8_t)12) /*!< Forces the card to stop transmission. */
#define SDMMC_CMD_SEND_STATUS         ((uint8_t)13) /*!< Addressed card sends its status register. */
#define SDMMC_CMD_HS_BUSTEST_READ     ((uint8_t)14) /*!< Reserved */
#define SDMMC_CMD_GO_INACTIVE_STATE   ((uint8_t)15) /*!< Sends an addressed card into the inactive state. */
#define SDMMC_CMD_SET_BLOCKLEN                                                   \
    ((uint8_t)16) /*!< Sets the block length (in bytes for SDSC) for all         \
                     following block commands (read, write, lock). Default block \
                     length is fixed to 512 Bytes. Not effective for SDHS and    \
                     SDXC. */
#define SDMMC_CMD_READ_SINGLE_BLOCK                                           \
    ((uint8_t)17) /*!< Reads single block of size selected by SET_BLOCKLEN in \
                     case of SDSC, and a block of fixed 512 bytes in case of  \
                     SDHC and SDXC. */
#define SDMMC_CMD_READ_MULT_BLOCK                                                                              \
    ((uint8_t)18)                                    /*!< Continuously transfers data blocks from card to host \
                                                        until interrupted by STOP_TRANSMISSION command. */
#define SDMMC_CMD_HS_BUSTEST_WRITE     ((uint8_t)19) /*!< 64 bytes tuning pattern is sent for SDR50 and SDR104. */
#define SDMMC_CMD_WRITE_DAT_UNTIL_STOP ((uint8_t)20) /*!< Speed class control command. */
#define SDMMC_CMD_SET_BLOCK_COUNT      ((uint8_t)23) /*!< Specify block count for CMD18 and CMD25. */
#define SDMMC_CMD_WRITE_SINGLE_BLOCK                                           \
    ((uint8_t)24) /*!< Writes single block of size selected by SET_BLOCKLEN in \
                     case of SDSC, and a block of fixed 512 bytes in case of   \
                     SDHC and SDXC. */
#define SDMMC_CMD_WRITE_MULT_BLOCK                                                             \
    ((uint8_t)25)                              /*!< Continuously writes blocks of data until a \
                                                  STOP_TRANSMISSION follows.                    */
#define SDMMC_CMD_PROG_CID       ((uint8_t)26) /*!< Reserved for manufacturers. */
#define SDMMC_CMD_PROG_CSD       ((uint8_t)27) /*!< Programming of the programmable bits of the CSD. */
#define SDMMC_CMD_SET_WRITE_PROT ((uint8_t)28) /*!< Sets the write protection bit of the addressed group. */
#define SDMMC_CMD_CLR_WRITE_PROT                                               \
    ((uint8_t)29) /*!< Clears the write protection bit of the addressed group. \
                   */
#define SDMMC_CMD_SEND_WRITE_PROT                                               \
    ((uint8_t)30) /*!< Asks the card to send the status of the write protection \
                     bits.                           */
#define SDMMC_CMD_SD_ERASE_GRP_START                                           \
    ((uint8_t)32) /*!< Sets the address of the first write block to be erased. \
                     (For SD card only).              */
#define SDMMC_CMD_SD_ERASE_GRP_END                                     \
    ((uint8_t)33) /*!< Sets the address of the last write block of the \
                     continuous range to be erased.           */
#define SDMMC_CMD_ERASE_GRP_START                                              \
    ((uint8_t)35) /*!< Sets the address of the first write block to be erased. \
                     Reserved for each command system set by switch function   \
                     command (CMD6). */
#define SDMMC_CMD_ERASE_GRP_END                                                                          \
    ((uint8_t)36)                            /*!< Sets the address of the last write block of the        \
                                                continuous range to be erased. Reserved for each command \
                                                system set by switch function command (CMD6). */
#define SDMMC_CMD_ERASE        ((uint8_t)38) /*!< Reserved for SD security applications. */
#define SDMMC_CMD_FAST_IO      ((uint8_t)39) /*!< SD card doesn't support it (Reserved). */
#define SDMMC_CMD_GO_IRQ_STATE ((uint8_t)40) /*!< SD card doesn't support it (Reserved). */
#define SDMMC_CMD_LOCK_UNLOCK                                                    \
    ((uint8_t)42) /*!< Sets/resets the password or lock/unlock the card. The     \
                     size of the data block is set by the SET_BLOCK_LEN command. \
                   */
#define SDMMC_CMD_APP_CMD                                                \
    ((uint8_t)55) /*!< Indicates to the card that the next command is an \
                     application specific command rather than a standard \
                     command. */
#define SDMMC_CMD_GEN_CMD                                                                          \
    ((uint8_t)56)                      /*!< Used either to transfer a data block to the card or to \
                                          get a data block from the card for general               \
                                          purpose/application specific commands. */
#define SDMMC_CMD_NO_CMD ((uint8_t)64) /*!< No command */

/**
 * @brief Following commands are SD Card Specific commands.
 *        SDMMC_APP_CMD should be sent before sending these commands.
 */
#define SDMMC_CMD_APP_SD_SET_BUSWIDTH                                                                     \
    ((uint8_t)6)                              /*!< (ACMD6) Defines the data bus width to be used for data \
                                                 transfer. The allowed data bus widths are given in SCR   \
                                                 register. */
#define SDMMC_CMD_SD_APP_STATUS ((uint8_t)13) /*!< (ACMD13) Sends the SD status. */
#define SDMMC_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS                                   \
    ((uint8_t)22) /*!< (ACMD22) Sends the number of the written (without errors) \
                     write blocks. Responds with 32bit+CRC data block. */
#define SDMMC_CMD_SD_APP_OP_COND                                                \
    ((uint8_t)41) /*!< (ACMD41) Sends host capacity support information (HCS)   \
                     and asks the accessed card to send its operating condition \
                     register (OCR) content in the response on the CMD line. */
#define SDMMC_CMD_SD_APP_SET_CLR_CARD_DETECT                                                                  \
    ((uint8_t)42)                               /*!< (ACMD42) Connect/Disconnect the 50 KOhm pull-up resistor \
                                                   on CD/DAT3 (pin 1) of the card  */
#define SDMMC_CMD_SD_APP_SEND_SCR ((uint8_t)51) /*!< Reads the SD Configuration Register (SCR). */
#define SDMMC_CMD_SDMMC_RW_DIRECT                                  \
    ((uint8_t)52) /*!< For SD I/O card only, reserved for security \
                     specification.                               */
#define SDMMC_CMD_SDMMC_RW_EXTENDED                                \
    ((uint8_t)53) /*!< For SD I/O card only, reserved for security \
                     specification.                               */

/**
 * @brief Following commands are SD Card Specific security commands.
 *        SDMMC_CMD_APP_CMD should be sent before sending these commands.
 */
#define SDMMC_CMD_SD_APP_GET_MKB                     ((uint8_t)43)
#define SDMMC_CMD_SD_APP_GET_MID                     ((uint8_t)44)
#define SDMMC_CMD_SD_APP_SET_CER_RN1                 ((uint8_t)45)
#define SDMMC_CMD_SD_APP_GET_CER_RN2                 ((uint8_t)46)
#define SDMMC_CMD_SD_APP_SET_CER_RES2                ((uint8_t)47)
#define SDMMC_CMD_SD_APP_GET_CER_RES1                ((uint8_t)48)
#define SDMMC_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK  ((uint8_t)18)
#define SDMMC_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK ((uint8_t)25)
#define SDMMC_CMD_SD_APP_SECURE_ERASE                ((uint8_t)38)
#define SDMMC_CMD_SD_APP_CHANGE_SECURE_AREA          ((uint8_t)49)
#define SDMMC_CMD_SD_APP_SECURE_WRITE_MKB            ((uint8_t)48)

/**
 * @brief  Masks for errors Card Status R1 (OCR Register)
 */
#define SDMMC_OCR_ADDR_OUT_OF_RANGE     0x80000000U
#define SDMMC_OCR_ADDR_MISALIGNED       0x40000000U
#define SDMMC_OCR_BLOCK_LEN_ERR         0x20000000U
#define SDMMC_OCR_ERASE_SEQ_ERR         0x10000000U
#define SDMMC_OCR_BAD_ERASE_PARAM       0x08000000U
#define SDMMC_OCR_WRITE_PROT_VIOLATION  0x04000000U
#define SDMMC_OCR_LOCK_UNLOCK_FAILED    0x01000000U
#define SDMMC_OCR_COM_CRC_FAILED        0x00800000U
#define SDMMC_OCR_ILLEGAL_CMD           0x00400000U
#define SDMMC_OCR_CARD_ECC_FAILED       0x00200000U
#define SDMMC_OCR_CC_ERROR              0x00100000U
#define SDMMC_OCR_GENERAL_UNKNOWN_ERROR 0x00080000U
#define SDMMC_OCR_STREAM_READ_UNDERRUN  0x00040000U
#define SDMMC_OCR_STREAM_WRITE_OVERRUN  0x00020000U
#define SDMMC_OCR_CID_CSD_OVERWRITE     0x00010000U
#define SDMMC_OCR_WP_ERASE_SKIP         0x00008000U
#define SDMMC_OCR_CARD_ECC_DISABLED     0x00004000U
#define SDMMC_OCR_ERASE_RESET           0x00002000U
#define SDMMC_OCR_AKE_SEQ_ERROR         0x00000008U
#define SDMMC_OCR_ERRORBITS             0xFDFFE008U

/**
 * @brief  Masks for R6 Response
 */
#define SDMMC_R6_GENERAL_UNKNOWN_ERROR 0x00002000U
#define SDMMC_R6_ILLEGAL_CMD           0x00004000U
#define SDMMC_R6_COM_CRC_FAILED        0x00008000U

/**
 * @brief  Args for ACMD41
 */
#define SDMMC_VOLTAGE_WINDOW_SD 0x80100000U
#define SDMMC_VOLTAGE_RECOMMEND 0x10100000U
#define SDMMC_HIGH_CAPACITY     0x40000000U
#define SDMMC_STD_CAPACITY      0x00000000U

/**
 * @brief  Args for CMD8
 */
#define SDMMC_CHECK_PATTERN 0x000001AAU

#define SDMMC_MAX_VOLT_TRIAL 0x00003FFFU

#define SDMMC_ALLZERO 0x00000000U

#define SDMMC_WIDE_BUS_SUPPORT   0x00040000U
#define SDMMC_SINGLE_BUS_SUPPORT 0x00010000U
#define SDMMC_CARD_LOCKED        0x02000000U

#define SDMMC_DATATIMEOUT 0xFFFFFFFFU

#define SDMMC_0TO7BITS   0x000000FFU
#define SDMMC_8TO15BITS  0x0000FF00U
#define SDMMC_16TO23BITS 0x00FF0000U
#define SDMMC_24TO31BITS 0xFF000000U

/**
 * @brief  Command Class supported
 */
#define SDIO_CCCC_ERASE 0x00000020U

#define SDIO_CMDTIMEOUT      5000U  /* Command send and response timeout */
#define SDIO_MAXERASETIMEOUT 63000U /* Max erase Timeout 63 s            */

/** @defgroup SDIO_Bus_Wide Bus Width
 * @{
 */
#define SDIO_BUS_WIDE_1B (0x0)
#define SDIO_BUS_WIDE_4B (0x1)
/**
 * @}
 */

/** @defgroup SDIO Response Register Offset
 * @{
 */
#define SDIO_RESP1 0x00000000U
#define SDIO_RESP2 0x00000004U
#define SDIO_RESP3 0x00000008U
#define SDIO_RESP4 0x0000000CU
/**
 * @}
 */

/** @defgroup SDIO Transfer Direction
 * @{
 */
#define SDIO_TRANSFER_DIR_TO_CARD 0x0U
#define SDIO_TRANSFER_DIR_TO_SDIO 0x1U
/**
 * @}
 */

/** @defgroup SDIO Transfer Type
 * @{
 */
#define SDIO_TRANSFER_MODE_BLOCK  0x0U
#define SDIO_TRANSFER_MODE_STREAM 0x1U
/**
 * @}
 */

/** @defgroup SDIO Transfer Clock Frequency
 * @{
 */
#define SDIO_CLK_400K (0)
#define SDIO_CLK_25M  (1)
/**
 * @}
 */

/** @defgroup SDIO_LL_Interrupt_Clock Interrupt And Clock Configuration
 *  @brief macros to handle interrupts and specific clock configurations
 * @{
 */

/**
 * @brief  Enable the SDIO device.
 * @param  __INSTANCE__ SDIO Instance
 * @retval None
 */
#define __SDIO_ENABLE(__INSTANCE__)

/**
 * @brief  Disable the SDIO device.
 * @param  __INSTANCE__ SDIO Instance
 * @retval None
 */
#define __SDIO_DISABLE(__INSTANCE__)

    /* Exported functions --------------------------------------------------------*/
    /* Initialization/de-initialization functions
     * **********************************/
    /** @addtogroup SDIO_Exported_Functions_Group1
     * @{
     */
    HAL_StatusTypeDef HAL_SDIO_Init(SDIO_HandleTypeDef *hsdio, SDIO_InitTypeDef Init);
    HAL_StatusTypeDef HAL_SDIO_DeInit(SDIO_HandleTypeDef *hsdio);
    /**
     * @}
     */

    /* Peripheral Control Functions
     * *****************************************************/
    /** @addtogroup SDIO_Exported_Functions_Group2
     * @{
     */
    void              HAL_SDIO_SetBusWidth(SDIO_HandleTypeDef *hsdio, uint32_t BusWide);
    HAL_StatusTypeDef HAL_SDIO_SetClock(SDIO_HandleTypeDef *hsdio, uint32_t Clock);
    /**
     * @}
     */

    /* I/O Operation Functions
     * *****************************************************/
    /** @addtogroup SDIO_Exported_Functions_Group3
     * @{
     */

    HAL_StatusTypeDef HAL_SDIO_RXFIFO_Setup(SDIO_HandleTypeDef *hsdio, uint32_t Size);
    HAL_StatusTypeDef HAL_SDIO_RXFIFO_Read(SDIO_HandleTypeDef *hsdio, uint32_t *pReadData, uint32_t Size);
    HAL_StatusTypeDef HAL_SDIO_TXFIFO_Write(SDIO_HandleTypeDef *hsdio, uint32_t *pWriteData, uint32_t Size);
    HAL_StatusTypeDef HAL_SDIO_WaitRX(SDIO_HandleTypeDef *hsdio, uint32_t timeout);
    HAL_StatusTypeDef HAL_SDIO_WaitTX(SDIO_HandleTypeDef *hsdio, uint32_t timeout);

    /**
     * @}
     */

    /* Peripheral Communication Functions
     * ************************************************/
    /** @addtogroup SDIO_Exported_Functions_Group4
     * @{
     */

    HAL_StatusTypeDef HAL_SDIO_SendCommand(SDIO_HandleTypeDef *hsdio, SDIO_CmdInitTypeDef *Command);

    // uint8_t SDIO_GetCommandResponse(SDIO_HandleTypeDef *hsdio);
    uint32_t HAL_SDIO_GetResponse(SDIO_HandleTypeDef *hsdio, uint32_t Response);

    HAL_StatusTypeDef HAL_SDIO_ConfigData(SDIO_HandleTypeDef *hsdio, SDIO_DataInitTypeDef *Data);
    HAL_StatusTypeDef HAL_SDIO_Setup_Clear(SDIO_HandleTypeDef *hsdio);
    /* SDMMC Commands management functions */
    uint32_t HAL_SDMMC_CmdBlockLength(SDIO_HandleTypeDef *hsdio, uint32_t BlockSize);
    uint32_t HAL_SDMMC_CmdReadSingleBlock(SDIO_HandleTypeDef *hsdio, uint32_t ReadAdd);
    uint32_t HAL_SDMMC_CmdReadMultiBlock(SDIO_HandleTypeDef *hsdio, uint32_t ReadAdd);
    uint32_t HAL_SDMMC_CmdWriteSingleBlock(SDIO_HandleTypeDef *hsdio, uint32_t WriteAdd);
    uint32_t HAL_SDMMC_CmdWriteMultiBlock(SDIO_HandleTypeDef *hsdio, uint32_t WriteAdd);
    uint32_t HAL_SDMMC_CmdSDEraseStartAdd(SDIO_HandleTypeDef *hsdio, uint32_t StartAdd);
    uint32_t HAL_SDMMC_CmdSDEraseEndAdd(SDIO_HandleTypeDef *hsdio, uint32_t EndAdd);
    uint32_t HAL_SDMMC_CmdSDErase(SDIO_HandleTypeDef *hsdio);
    uint32_t HAL_SDMMC_CmdStopTransfer(SDIO_HandleTypeDef *hsdio);
    uint32_t HAL_SDMMC_CmdSelDesel(SDIO_HandleTypeDef *hsdio, uint64_t Addr);
    uint32_t HAL_SDMMC_CmdGoIdleState(SDIO_HandleTypeDef *hsdio);
    uint32_t HAL_SDMMC_CmdOperCond(SDIO_HandleTypeDef *hsdio);
    uint32_t HAL_SDMMC_CmdAppCommand(SDIO_HandleTypeDef *hsdio, uint32_t Argument);
    uint32_t HAL_SDMMC_CmdAppOperCommand(SDIO_HandleTypeDef *hsdio, uint32_t SdType);
    uint32_t HAL_SDMMC_CmdBusWidth(SDIO_HandleTypeDef *hsdio, uint32_t BusWidth);
    uint32_t HAL_SDMMC_CmdSendSCR(SDIO_HandleTypeDef *hsdio);
    uint32_t HAL_SDMMC_CmdSendCID(SDIO_HandleTypeDef *hsdio);
    uint32_t HAL_SDMMC_CmdSendCSD(SDIO_HandleTypeDef *hsdio, uint32_t Argument);
    uint32_t HAL_SDMMC_CmdSetRelAdd(SDIO_HandleTypeDef *hsdio, uint16_t *pRCA);
    uint32_t HAL_SDMMC_CmdSendStatus(SDIO_HandleTypeDef *hsdio, uint32_t Argument);
    uint32_t HAL_SDMMC_CmdStatusRegister(SDIO_HandleTypeDef *hsdio);
    uint32_t HAL_SDMMC_CmdSwitch(SDIO_HandleTypeDef *hsdio, uint32_t Argument);

#ifdef __cplusplus
}
#endif

#endif