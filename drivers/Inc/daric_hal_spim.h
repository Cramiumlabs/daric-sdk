/**
 *******************************************************************************
 * @file    daric_hal_spim.h
 * @author  SPIM Team
 * @brief   Header file for SPIM HAL module.
 *******************************************************************************
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
#ifndef DARIC_HAL_SPIM_H
#define DARIC_HAL_SPIM_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal.h"
#include "daric_hal_udma_v3.h"
#include <stdint.h>

    /** @cond SPIM CTRL BIT macros
     * @{
     */

#define SPIM_CTRL_CPOL_BIT             0
#define SPIM_CTRL_CPHA_BIT             2
#define SPIM_CTRL_ENDIANNESS_BIT       4
#define SPIM_CTRL_SET_BAUDRATE_BIT     6
#define SPIM_CTRL_SET_QSPI_ENABLE_BIT  8
#define SPIM_CTRL_SET_QSPI_DISABLE_BIT 9

    /** @endcond
     * @}
     */

    /**
     * @enum SPIM_WordSizeTypeDef
     * @brief Word size of the SPI bitstream elements.
     *
     * The length unit of the 0-8-bit word is 1 byte, the length unit of the 9-16
     * bit word is 2 bytes, and the length unit of the 17-32 bit word is 4 bytes
     */
    typedef enum
    {
        SPIM_WORDSIZE_1 = 1,
        SPIM_WORDSIZE_2,
        SPIM_WORDSIZE_3,
        SPIM_WORDSIZE_4,
        SPIM_WORDSIZE_5,
        SPIM_WORDSIZE_6,
        SPIM_WORDSIZE_7,
        SPIM_WORDSIZE_8,
        SPIM_WORDSIZE_9,
        SPIM_WORDSIZE_10,
        SPIM_WORDSIZE_11,
        SPIM_WORDSIZE_12,
        SPIM_WORDSIZE_13,
        SPIM_WORDSIZE_14,
        SPIM_WORDSIZE_15,
        SPIM_WORDSIZE_16,
        SPIM_WORDSIZE_17,
        SPIM_WORDSIZE_18,
        SPIM_WORDSIZE_19,
        SPIM_WORDSIZE_20,
        SPIM_WORDSIZE_21,
        SPIM_WORDSIZE_22,
        SPIM_WORDSIZE_23,
        SPIM_WORDSIZE_24,
        SPIM_WORDSIZE_25,
        SPIM_WORDSIZE_26,
        SPIM_WORDSIZE_27,
        SPIM_WORDSIZE_28,
        SPIM_WORDSIZE_29,
        SPIM_WORDSIZE_30,
        SPIM_WORDSIZE_31,
        SPIM_WORDSIZE_32
    } SPIM_WordSizeTypeDef;
    /**
     * @enum SPIM_IDTypeDef
     * @brief ID of the SPIM.
     *
     * This is used to know which SPIM to use.
     */
    typedef enum
    {
        SPIM0 = 0,
        SPIM1 = 1,
        SPIM2 = 2,
        SPIM3 = 3,
    } SPIM_IDTypeDef;

    /**
     * @enum SPIM_ControlTypeDef
     * @brief Possible parameters which can be set through the rt_spim_control API
     * function.
     *
     * This is used to reconfigure dynamically some of the parameters of an opened
     * device.
     */
    typedef enum
    {
        SPIM_CTRL_CPOL0      = 1 << SPIM_CTRL_CPOL_BIT,                   /*!< Set the clock polarity to 0. */
        SPIM_CTRL_CPOL1      = 2 << SPIM_CTRL_CPOL_BIT,                   /*!< Set the clock polarity to 1. */
        SPIM_CTRL_CPHA0      = 1 << SPIM_CTRL_CPHA_BIT,                   /*!< Set the clock phase to 0. */
        SPIM_CTRL_CPHA1      = 2 << SPIM_CTRL_CPHA_BIT,                   /*!< Set the clock phase to 1. */
        SPIM_CTRL_BIG_ENDIAN = 1 << SPIM_CTRL_ENDIANNESS_BIT,             /*!< Handle the elements in memory in a
                                                                             big-endian way. */
        SPIM_CTRL_LITTLE_ENDIAN = 2 << SPIM_CTRL_ENDIANNESS_BIT,          /*!< Handle the elements in memory in a
                                                                             little-endian way. */
        SPIM_CTRL_SET_BAUDRATE     = 1 << SPIM_CTRL_SET_BAUDRATE_BIT,     /*!< Change baudrate. */
        SPIM_CTRL_SET_QSPI_ENABLE  = 1 << SPIM_CTRL_SET_QSPI_ENABLE_BIT,  /*!< ENABLE QSPI mode. */
        SPIM_CTRL_SET_QSPI_DISABLE = 1 << SPIM_CTRL_SET_QSPI_DISABLE_BIT, /*!< DISABLE QSPI mode. */
    } SPIM_ControlTypeDef;

    /**
     * @enum SPIM_CS_TypeDef
     * @brief Specifies chip select mode.
     *
     * This is used to set CS TYPE after SPIM transfer.
     */
    typedef enum
    {
        SPIM_CS_AUTO = 0, /*!< Handles the chip select automatically. It is set low
                             just before the transfer is started and set back high
                             when the transfer is finished. */
        SPIM_CS_KEEP = 1, /*!< Handle the chip select manually. It is set low just before the
                             transfer is started and is kept low until the next transfer. */
        SPIM_CS_NONE = 2, /*!< Don't do anything with the chip select. */
    } SPIM_CS_TypeDef;

    /**
     * @struct SPIM_HandleTypeDef
     * @brief SPI handle Structure definition.
     *
     */
    typedef struct __SPIM_HandleTypeDef
    {
        uint32_t baudrate;                                       /*!< baudrate for the SPI bitstream which can be used with
                                                               the opened device . */
        uint8_t id;                                              /*!< If it is different from -1, this specifies on which SPI
                                                                          interface the device is connected. */
        uint8_t qspi;                                            /*!< send by using quad spi or classic SPI transfer with MOSI
                                                                and   MISO lines */
        int8_t cs_gpio;                                          /*!< If it is different from -1, the specified number is used
                                                                  to drive a GPIO which is used as a chip select for the SPI
                                                                  device. The cs field is then ignored. */
        uint8_t cs;                                              /*!< If cs_gpio is -1, the normal chip select pins are used and
                                                                 this field specifies which one to use for the device. */
        uint8_t wordsize;                                        /*!< Wordsize of the elements in the bitstream. Can be
                                                                    RT_SPIM_WORDSIZE_8 for 8 bits data or RT_SPIM_WORDSIZE_32 for
                                                                    32 bits data. This is used to interpret the endianness. */
        uint8_t big_endian;                                      /*!< If 1, the elements are stored in memory in a
                                                                 big-endian way, i.e. the most significant byte is stored
                                                                 at the lowest address. This is taken into account only if
                                                                 the wordsize bigger than 8bits. */
        uint8_t byte_align;                                      /*!< byte alignment. */
        uint8_t polarity;                                        /*!< Polarity of the clock. */
        uint8_t phase;                                           /*!< Phase of the clock. */
        uint8_t div;                                             /*!< div of the clock. */
        uint8_t channel;                                         /*!< peri id in udma . */
        uint8_t active_flg;                                      /*!< Status flag indicating whether the SPIM
                                                                          instance is active . */
        uint32_t cfg;                                            /*!< cfg cmd fiead. */
        uint32_t sending_size;                                   /*!< Sending data size used in interrupt mode. */
        uint32_t recving_size;                                   /*!< receving data size used in interrupt mode. */
        uint8_t *recving_addr;                                   /*!< receving data address used in interrupt mode. */
        void (*CpltCallback)(struct __SPIM_HandleTypeDef *hspi); /*!< SPI Completed callback */

    } SPIM_HandleTypeDef; /*!< SPIM_HandleTypeDef */

    /* Initialization and de-initialization functions  ****************************/
    HAL_StatusTypeDef HAL_SPIM_Init(SPIM_HandleTypeDef *hspim);
    HAL_StatusTypeDef HAL_SPIM_Deinit(SPIM_HandleTypeDef *hspim);
    HAL_StatusTypeDef HAL_SPIM_Control(SPIM_HandleTypeDef *hspim, SPIM_ControlTypeDef cmd, SPIM_WordSizeTypeDef wordsize, uint32_t arg);
    /* IO operation functions *****************************************************/
    HAL_StatusTypeDef HAL_SPIM_Send(SPIM_HandleTypeDef *hspim, void *data, uint32_t len, SPIM_CS_TypeDef cs_mode, uint32_t Timeout);
    HAL_StatusTypeDef HAL_SPIM_Receive(SPIM_HandleTypeDef *hspim, void *data, uint32_t len, SPIM_CS_TypeDef cs_mode, uint32_t Timeout);
    HAL_StatusTypeDef HAL_SPIM_Transfer(SPIM_HandleTypeDef *hspim, void *tx_data, void *rx_data, uint16_t len, SPIM_CS_TypeDef cs_mode, uint32_t Timeout);
    /* Interrupt functionss *****************************************************/
    HAL_StatusTypeDef HAL_SPIM_Send_IT(SPIM_HandleTypeDef *handle, const uint8_t *data, uint32_t len, uint32_t *Real_Size, SPIM_CS_TypeDef cs_modee);
    HAL_StatusTypeDef HAL_SPIM_Receive_IT(SPIM_HandleTypeDef *hspi, uint8_t *pData, uint32_t len, uint32_t *Real_Size, SPIM_CS_TypeDef cs_mode);
    HAL_StatusTypeDef HAL_SPIM_Transfer_IT(SPIM_HandleTypeDef *hspi, const uint8_t *pTxData, uint8_t *pRxData, uint32_t len, uint32_t *Real_Size, SPIM_CS_TypeDef cs_mode);

#ifdef __cplusplus
}
#endif

#endif /* DARIC_HAL_SPIM_H */