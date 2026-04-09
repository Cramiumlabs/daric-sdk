/**
 ******************************************************************************
 * @file    daric_hal_i2c.h
 * @author  I2C Team
 * @brief   Header file of I2C HAL module.
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
#ifndef __DARIC_HAL_I2C_H
#define __DARIC_HAL_I2C_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include <stdint.h>
#include <stdio.h>

/** @defgroup I2C Instance ID
 * @{
 */
#define DARIC_I2C0_ID 0 ///< I2C0 ID
#define DARIC_I2C1_ID 1 ///< I2C0 ID
#define DARIC_I2C2_ID 2 ///< I2C0 ID
#define DARIC_I2C3_ID 3 ///< I2C0 ID

#define DARIC_I2C_MAX 4 ///< DARIC I2C NUM
    /**
     * @}
     */

    /**
     * @brief  I2C Initialization Structure definition.
     */
    typedef struct
    {
        uint32_t wait;     /*!< Indicates I2C wait cycles following I2C_CMD_WAIT command */
        uint32_t repeat;   /*!< Indicates number of timers to repeat next command */
        uint32_t baudrate; /*!< baudrate for the I2C bitstream which can be used with
                              the opened device . */
        uint32_t divider;  /*!< Indicates I2C divider 16bits values releated to SoC
                              Clock  freq  */
        uint32_t rx_buf;   /*!< Rx transfer address of associated buffer */
        uint32_t rx_size;  /*!< Rx buffer size */
        uint32_t tx_buf;   /*!< Tx transfer address of associated buffer */
        uint32_t tx_size;  /*!< Tx buffer size */
        uint32_t cmd_buf;  /*!< Command transfer size of buffer */
        uint32_t cmd_size; /*!< Command buffer size */
    } I2C_InitTypeDef;

    /**
     * @brief  I2C State enumeration
     */
    typedef enum
    {
        HAL_I2C_STATE_RESET   = 0x00U, /*!< I2C is not yet Initialized */
        HAL_I2C_STATE_READY   = 0x10U, /*!< I2C Initialized and ready for use */
        HAL_I2C_STATE_BUSY    = 0x20U, /*!< internal process is ongoing */
        HAL_I2C_STATE_BUSY_TX = 0x40U, /*!< Data Transmission process is ongoing */
        HAL_I2C_STATE_BUSY_RX = 0x80U, /*!< Data Reception process is ongoing */
        HAL_I2C_STATE_TIMEOUT = 0xA0U, /*!< Timeout state */
        HAL_I2C_STATE_ERROR   = 0xE0U  /*!< Error */
    } HAL_I2C_StateTypeDef;

    /**
     * @brief  I2C Transfer Mode enumeration
     */
    typedef enum
    {
        HAL_I2C_MODE_NONE         = 0x00U, /*!< No I2C communication on going             */
        HAL_I2C_MODE_MASTER_WRITE = 0x10U, /*!< I2C communication is in Master Mode */
        HAL_I2C_MODE_MASTER_READ  = 0x20U, /*!< I2C communication is in Master Mode  */
        HAL_I2C_MODE_MEMORY_WRITE = 0x30U, /*!< I2C communication is in Memory Mode */
        HAL_I2C_MODE_MEMORY_READ  = 0x40U  /*!< I2C communication is in Memory Mode   */
    } HAL_I2C_ModeTypeDef;

    /**
     * @brief  I2C Error enumeration
     */
    typedef enum
    {
        HAL_I2C_ERR_NONE = 0,      /*!< None Error */
        HAL_I2C_ERR_INVALID_PARAM, /*!< Invalid Param */
        HAL_I2C_ERR_SLAVE_NACK,    /*!< Slave Nack */
        HAL_I2C_ERR_TIMEOUT,       /*!< Transfer Timeout */
        HAL_I2C_ERR_ARB_LOST,      /*!< Arbitration Lost */
        HAL_I2C_ERR_STATE_WRONG,   /*!< Controller State Wrong */
    } HAL_I2C_ErrorTypeDef;

    typedef struct
    {
        uint8_t *trans_buff;  /*!< Pointer to transfer buffer  */
        uint32_t trans_size;  /*!< Transfer buffer size */
        uint32_t trans_count; /*!<  Count of single Transfer */
        uint32_t DevAddress;  /*!< Target device address */
        uint32_t MemAddress;  /*!< Internal memory address in the target device */
        uint32_t MemAddSize;  /*!< Size of the internal memory address (in bytes). */
    } HAL_I2C_TransferTypedef;

    /**
     * @brief  I2C handle Structure definition
     */
    typedef struct __I2C_HandleTypeDef
    {
        uint32_t                instance_id;                         /*!< I2C ID */
        I2C_InitTypeDef         init;                                /*!< I2C communication parameters  */
        HAL_I2C_StateTypeDef    state;                               /*!< I2C comuunication state */
        HAL_I2C_ModeTypeDef     mode;                                /*!< I2C comuunication mode */
        HAL_I2C_ErrorTypeDef    error_code;                          /*!< I2C error code */
        HAL_I2C_TransferTypedef transfer_info;                       /*!< I2C Transfer info for non-blocking */
        void (*TxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);    /*!< I2C Tx Transfer completed callback */
        void (*RxCpltCallback)(struct __I2C_HandleTypeDef *hi2c);    /*!< I2C Rx Transfer completed callback */
        void (*MemTxCpltCallback)(struct __I2C_HandleTypeDef *hi2c); /*!< I2C Memory Tx Transfer completed callback */
        void (*MemRxCpltCallback)(struct __I2C_HandleTypeDef *hi2c); /*!< I2C Memory Rx Transfer completed callback */
        void (*NackCallback)(struct __I2C_HandleTypeDef *hi2c);      /*!< I2C NACK callback */
        void (*ErrorCallback)(struct __I2C_HandleTypeDef *hi2c);     /*!< I2C Error callback */
    } I2C_HandleTypeDef;

    /**
     * @brief  I2C Callback ID enumeration
     */
    typedef enum
    {
        HAL_I2C_TX_COMPLETE_CB_ID     = 0x00U, /*!< I2C Tx Transfer completed callback ID */
        HAL_I2C_RX_COMPLETE_CB_ID     = 0x01U, /*!< I2C Rx Transfer completed callback ID */
        HAL_I2C_MEM_TX_COMPLETE_CB_ID = 0x02U, /*!< I2C Memory Tx Transfer callback ID */
        HAL_I2C_MEM_RX_COMPLETE_CB_ID = 0x03U, /*!< I2C Memory Rx Transfer completed callback ID */
        HAL_I2C_NACK_CB_ID            = 0x04U, /*!< I2C Nack callback ID */
        HAL_I2C_ERROR_CB_ID           = 0x05U, /*!< I2C Error callback ID */
    } HAL_I2C_CallbackIDTypeDef;

    /**
     * @brief  HAL I2C Callback pointer definition
     */
    typedef void (*I2C_CallbackTypeDef)(I2C_HandleTypeDef *hi2c);

    /**
     * @brief  Initializes the I2C peripheral according to the specified parameters.
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * initialization process.
     *
     * This function configures the I2C peripheral according to the parameters
     * specified in the `I2C_HandleTypeDef` structure, including timing, addressing
     * mode, and other relevant settings.
     */
    HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);

    /**
     * @brief  Deinitializes the I2C peripheral.
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * deinitialization process.
     *
     * This function deinitializes the I2C peripheral, resetting it to its default
     * state and freeing any resources that were allocated during initialization.
     */
    HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c);

    /**
     * @brief  Registers a callback function for a specific I2C event.
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  CallbackID Specifies the callback ID to be registered.
     * @param  pCallback Pointer to the callback function.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * registration process.
     *
     * This function registers a user-defined callback function for a specific I2C
     * event, allowing the application to handle events such as data transmission or
     * reception completion.
     */
    HAL_StatusTypeDef HAL_I2C_RegisterCallback(I2C_HandleTypeDef *hi2c, HAL_I2C_CallbackIDTypeDef CallbackID, I2C_CallbackTypeDef pCallback);

    /**
     * @brief  Unregisters a callback function for a specific I2C event.
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  CallbackID Specifies the callback ID to be unregistered.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * unregistration process.
     *
     * This function unregisters a previously registered callback function for a
     * specific I2C event, disabling the application's handling of that event.
     */
    HAL_StatusTypeDef HAL_I2C_UnRegisterCallback(I2C_HandleTypeDef *hi2c, HAL_I2C_CallbackIDTypeDef CallbackID);

    /**
     * @brief  Transmits data in master mode via I2C (blocking mode).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  pData Pointer to the data buffer to be transmitted.
     * @param  Size Amount of data to be sent.
     * @param  Timeout Duration for blocking until transmission is complete.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * transmission.
     *
     * This function sends data to the specified slave device in a blocking mode,
     * where the CPU waits until the operation is complete or a timeout occurs.
     */
    HAL_StatusTypeDef HAL_I2C_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint32_t Size, uint32_t Timeout);

    /**
     * @brief  Receives data in master mode via I2C (blocking mode).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  pData Pointer to the data buffer to store received data.
     * @param  Size Amount of data to be received.
     * @param  Timeout Duration for blocking until reception is complete.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * reception.
     *
     * This function reads data from the specified slave device in a blocking mode,
     * where the CPU waits until the operation is complete or a timeout occurs.
     */
    HAL_StatusTypeDef HAL_I2C_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint32_t Size, uint32_t Timeout);

    /**
     * @brief  Writes data to a specific memory address in a slave device (blocking
     * mode).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  MemAddress Internal memory address in the target device.
     * @param  MemAddSize Size of the internal memory address (in bytes).
     * @param  pData Pointer to the data buffer to be written.
     * @param  Size Amount of data to be written.
     * @param  Timeout Duration for blocking until the write operation is complete.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * write operation.
     *
     * This function writes data to a specific memory address in a slave device
     * in a blocking mode, where the CPU waits until the operation is complete or a
     * timeout occurs.
     */
    HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

    /**
     * @brief  Reads data from a specific memory address in a slave device (blocking
     * mode).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  MemAddress Internal memory address in the target device.
     * @param  MemAddSize Size of the internal memory address (in bytes).
     * @param  pData Pointer to the data buffer to store received data.
     * @param  Size Amount of data to be read.
     * @param  Timeout Duration for blocking until the read operation is complete.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * read operation.
     *
     * This function reads data from a specific memory address in a slave device
     * in a blocking mode, where the CPU waits until the operation is complete or a
     * timeout occurs.
     */
    HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

    /**
     * @brief  Writes data to a specific memory address in a slave device (blocking
     * mode).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  MemAddress Internal memory address in the target device.
     * @param  MemAddSize Size of the internal memory address (in bytes).
     * @param  pData Pointer to the data buffer to be written.
     * @param  Size Amount of data to be written.
     * @param  Timeout Duration for blocking until the write operation is complete.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * write operation.
     *
     * This function writes data to a specific memory address in a slave device
     * in a blocking mode, where the CPU waits until the operation is complete or a
     * timeout occurs.
     */
    HAL_StatusTypeDef HAL_I2C1_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

    /**
     * @brief  Reads data from a specific memory address in a slave device (blocking
     * mode).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  MemAddress Internal memory address in the target device.
     * @param  MemAddSize Size of the internal memory address (in bytes).
     * @param  pData Pointer to the data buffer to store received data.
     * @param  Size Amount of data to be read.
     * @param  Timeout Duration for blocking until the read operation is complete.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * read operation.
     *
     * This function reads data from a specific memory address in a slave device
     * in a blocking mode, where the CPU waits until the operation is complete or a
     * timeout occurs.
     */
    HAL_StatusTypeDef HAL_I2C1_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

    /**
     * @brief  Checks if the I2C device is ready for communication.
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * operation.
     *
     * This function checks if the specified I2C device is ready for communication.
     */
    HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c);

    /**
     * @brief  Transmits data in master mode via I2C using interrupt mode
     * (non-blocking).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  pData Pointer to the data buffer to be transmitted.
     * @param  Size Amount of data to be sent.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * transmission.
     *
     * This function sends data to the specified slave device in a non-blocking mode
     * using interrupts. The CPU is free to perform other tasks while the data is
     * being transmitted, and a callback is invoked upon completion.
     */
    HAL_StatusTypeDef HAL_I2C_Transmit_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);

    /**
     * @brief  Receives data in master mode via I2C using interrupt mode
     * (non-blocking).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  pData Pointer to the data buffer to store received data.
     * @param  Size Amount of data to be received.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * reception.
     *
     * This function reads data from the specified slave device in a non-blocking
     * mode using interrupts. The CPU is free to perform other tasks while the data
     * is being received, and a callback is invoked upon completion.
     */
    HAL_StatusTypeDef HAL_I2C_Receive_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);

    /**
     * @brief  Writes data to a specific memory address in a slave device using
     * interrupt mode (non-blocking).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  MemAddress Internal memory address in the target device.
     * @param  MemAddSize Size of the internal memory address (in bytes).
     * @param  pData Pointer to the data buffer to be written.
     * @param  Size Amount of data to be written.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * write operation.
     *
     * This function writes data to a specific memory address in a slave device
     * in a non-blocking mode using interrupts. The CPU is free to perform other
     * tasks while the data is being written, and a callback is invoked upon
     * completion.
     */
    HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);

    /**
     * @brief  Reads data from a specific memory address in a slave device using
     * interrupt mode (non-blocking).
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @param  DevAddress Target device address.
     * @param  MemAddress Internal memory address in the target device.
     * @param  MemAddSize Size of the internal memory address (in bytes).
     * @param  pData Pointer to the data buffer to store received data.
     * @param  Size Amount of data to be read.
     * @retval HAL_StatusTypeDef HAL status indicating the success or error of the
     * read operation.
     *
     * This function reads data from a specific memory address in a slave device
     * in a non-blocking mode using interrupts. The CPU is free to perform other
     * tasks while the data is being read, and a callback is invoked upon
     * completion.
     */
    HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);

    /**
     * @brief  Initializes the I2C peripheral according to the specified parameters.
     * @param  hi2c Pointer to an `I2C_HandleTypeDef` structure that contains
     *         the configuration information for the specified I2C module.
     * @retval none.
     *
     * This function used to reset I2C manually for I2C issue workaround.
     */
    void HAL_I2C_Reset(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif /* __DARIC_HAL_I2C_H */
