/**
 ******************************************************************************
 * @file    daric_hal_uart.h
 * @author  UART Team
 * @brief   Header file of UART HAL module.
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
#ifndef DARIC_HAL_UART_H
#define DARIC_HAL_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "daric_hal_conf.h"
#include "daric_hal_def.h"
#include "daric_uart.h"

#define USE_HAL_UART_REGISTER_CALLBACKS (1) ///< enable Interrupt Callback function

#define HAL_UART_IDLE                  (0)      ///< UART State: IDLE
#define HAL_UART_INITED                (1 << 0) ///< UART State: INITED
#define HAL_UART_TX_ONGOING            (1 << 1) ///< UART State: TX ONGOING
#define HAL_UART_RX_ONGOING            (1 << 2) ///< UART State: RX ONGOING
#define HAL_UART_ERROR                 (1 << 3) ///< UART State: ERROR OCCUR
#define HAL_UART_RX_CONTINUOUS_ENABLE  (1)      ///< RX CONTINUOUS ENABLE
#define HAL_UART_RX_CONTINUOUS_DISABLE (0)      ///< RX CONTINUOUS DISABLE

    /**
     * @brief UART States enumeration.
     */
    typedef enum
    {
        HAL_UART_STATE_RESET = 0, /*!< UART is not yet Initialized */
        HAL_UART_STATE_READY,     /*!< UART Initialized and ready for use */
        HAL_UART_STATE_BUSY,      /*!< UART Transfer is ongoing */
        HAL_UART_STATE_TIMEOUT    /*!< UART Timeout state */
    } UART_StateTypeDef;

    /**
     * @brief UART Init Structure definition
     */
    typedef struct
    {
        uint32_t BaudRate;          /*!< This member configures the UART communication baud rate. */
        uint32_t Rx_En : 1;         /*!< RX transceiver configuration bitfield. */
        uint32_t Tx_En : 1;         /*!< TX transceiver configuration bitfield. */
        uint32_t Clean_Rx_Fifo : 1; /*!< In all mode clean the RX fifo, set 1 then set
                                       0 to realize a reset fifo. */
                                    /*!<  0: Stop Clean the RX FIFO.*/
                                    /*!<  1: Clean the RX FIFO.*/
        uint32_t Poll_En : 1;       /*!< When in uart read, use polling method to read the
                                       data, read interrupt enable flag will be ignored. */
                                    /*!< 0: Do not using polling method to read data */
        /*!< 1: Using polling method to read data. Interrupt enable flag will be
         * ignored. */
        uint32_t StopBits : 1;   /*!< Stop bits length bitfield. */
                                 /*!< 0: 1 stop bit */
                                 /*!< 1: 2 stop bit */
        uint32_t WordLength : 2; /*!< Character length bitfield. */
                                 /*!< 00: 5 bits */
                                 /*!< 01: 6 bits */
                                 /*!< 10: 7 bits */
                                 /*!< 11: 8 bits */
        uint32_t Parity : 1;     /*!< Parity bit generation and check configuration
                                    bitfield; */
                                 /*!< 0: disabled */
                                 /*!< 1: enabled */
        uint32_t reverse : 24;
    } UART_InitTypeDef;

    /**
     * @brief  UART handle Structure definition
     */
    typedef struct __UART_HandleTypeDef
    {
        uint8_t           id;           /*!< UART id */
        uint8_t           state;        /*!< UART state flag */
        uint8_t           Tx_mode;      /*!< Tx transfer mode 0-poll 1-interrupt mode*/
        uint8_t           Rx_mode;      /*!< Rx transfer mode 0-poll 1-interrupt mode*/
        uint8_t           RxContinuous; /*!< UART RX continuous mode */
        UART_InitTypeDef  init;         /*!< UART communication parameters */
        const uint8_t    *pTxBuffPtr;   /*!< UART read buffer */
        uint32_t          TxXferSize;   /*!< UART Tx Transfer size */
        uint32_t          TxXferCount;  /*!< UART Tx Transfer Counter */
        uint32_t          TxBuffIndex;  /*!< UART TX buffer index */
        UART_StateTypeDef TxState;      /*!< UART TX Transfer State */

        uint8_t          *pRxBuffPtr;          /*!< UART read buffer */
        uint32_t          RxXferSize;          /*!< UART Rx Transfer size */
        uint32_t          RxXferCount;         /*!< UART Rx Transfer Counter */
        uint32_t          RxReceiveLength;     /*!< UART Rx Receive Length */
        uint32_t          RxBuffIndex;         /*!< UART RX buffer index */
        uint32_t          RxBuffIndexOut;      /*!< UART RX buffer index out */
        bool              RxContinuousEx_Flag; /*!< UART RX continuous ex flag */
        UART_StateTypeDef RxState;             /*!< UART RX Transfer State */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        void (*RxFullCallback)(struct __UART_HandleTypeDef *huart);    /*!< UART RX channel receives data
                                                                          full Callback		 */
        void (*TxFinishCallback)(struct __UART_HandleTypeDef *huart);  /*!< UART TX channel finish sending
                                                                          data Callback		 */
        void (*RxPollingCallback)(struct __UART_HandleTypeDef *huart); /*!< UART Rx Data received in Polling
                                                                          mode Callback		 */
        void (*ErrorCallback)(struct __UART_HandleTypeDef *huart);     /*!< UART Error occurred when sending and
                                                                          receiving data Callback	*/
#endif                                                                 /* USE_HAL_UART_REGISTER_CALLBACKS */
        void *UserData;                                                /*!< pointer to user data */
    } UART_HandleTypeDef;

    /**
     * @brief  HAL UART Callback ID enumeration definition
     */
    typedef enum
    {
        HAL_UART_RX_FULL_CB_ID    = 0x00U, /*!< UART RX channel receives data full Callback ID        */
        HAL_UART_TX_FINISH_CB_ID  = 0x01U, /*!< UART TX channel finish sending data Callback ID             */
        HAL_UART_RX_POLLING_CB_ID = 0x02U, /*!< UART Rx Data received in Polling mode Callback ID        */
        HAL_UART_ERROR_CB_ID      = 0x03U, /*!< UART Error occurred when sending and
                                              receiving data Callback ID             */
    } HAL_UART_CallbackIDTypeDef;

/** @defgroup UART_id
 * @{
 */
#define UART0_ID 0x00000000U ///< UART0 ID
#define UART1_ID 0x00000001U ///< UART1 ID
#define UART2_ID 0x00000002U ///< UART2 ID
#define UART3_ID 0x00000003U ///< UART3 ID

#define UART_ID_MAX 0x00000004U ///< DARIC UART NUM
    /**
     * @}
     */

    /**
     * @brief UART Callback pointer definition
     */
    typedef void (*pUART_CallbackTypeDef)(UART_HandleTypeDef *huart);

    /**
     * @brief Clear dirty data received in the FIFO of the UART Instance
     * @param huart UART handle.
     */
    void HAL_UART_Clr_FIFO(UART_HandleTypeDef *huart);

    /**
     * @brief Reset the UART Instance
     * @param huart UART handle.
     */
    void HAL_UART_Reset(UART_HandleTypeDef *huart);

    /* Initialization and de-initialization functions  ****************************/
    /**
     * @brief Initialize the UART mode according to the specified parameters in the
     * UART_InitTypeDef and initialize the associated handle.
     * @param huart UART handle.
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);

    /**
     * @brief DeInitialize the UART peripheral.
     * @param  huart UART handle
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart);

    /**
     * @brief Send an amount of data in interrup mode.
     * @param huart UART handle
     * @param pData   Pointer to data buffer.
     * @param Size    Amount of data elements to be sent.
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart, const uint8_t *pData, uint32_t Size);

    /**
     * @brief Send an amount of data in blocking mode.
     * @param huart UART handle
     * @param pData   Pointer to data buffer.
     * @param Size    Amount of data elements to be sent.
     * @param Timeout Timeout duration.
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint32_t Size, uint32_t Timeout);

    /**
     * @brief Receive an amount of data by interrup mode.
     * @param huart uart handle
     * @param pData   Pointer to data buffer.
     * @param Size    Amount of data elementsto be received.
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint32_t Size);

    /**
     * @brief Receive an amount of data by poll mode.
     * @param huart UART handle
     * @param pData   Pointer to data buffer.
     * @param Size    Amount of data elements to be received.
     * @param Timeout Timeout duration.
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint32_t Size, uint32_t Timeout);

    /**
     * @brief Receive an amount of data in continous.
     * @param huart   UART handle
     * @param pData   Pointer to data buffer.
     * @param Size    Amount of data elements to be received.
     * @param Timeout Timeout duration.
     * @retval The number of bytes received, 0: timeout, -1: error.
     * @note This HAL_UART_IRQCfg can be used if it is necessary to turn off the RX_FULL interrupt
     */
    uint32_t HAL_UART_ReceiveContinous_ex(UART_HandleTypeDef *huart, uint8_t *pData, uint32_t Size, uint32_t Timeout);

    /**
     * @brief IRQ config
     * @param huart UART handle
     * @param RXFull_EN (R/W) DMA RX channel receives data full interrupt enable
     * flag 0: RXFull IRQ disable 1: RXFull IRQ enable
     * @param TXFinishIRQ_EN (R/W) DMA TX channel finish sending data interrupt
     * enable flag 0: TXFinish IRQ disable 1: TXFinish IRQ enable
     * @param ErrorIRQ_EN ERROR (R/W) Error interrupt in enable flag
     *    0: Error IRQ disable
     *    1: Error IRQ enable
     * @param RXPollingIRQ_EN RX (R/W) Rx interrupt in Polling mode  enable flag:
     *    0: RX Polling IRQ disable
     *    1: RX Polling IRQ enable
     * @retval HAL status
     */
    HAL_StatusTypeDef HAL_UART_IRQCfg(UART_HandleTypeDef *huart, int RXFullIRQ_EN, int TXFinishIRQ_EN, int RXPollingIRQ_EN, int ErrorIRQ_EN);

    /**
     * @brief UART Receive data in a continuous mode.
     * @param huart UART handle
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_ReceiveContinous(UART_HandleTypeDef *huart);

    /**
     * @brief UART Transmit Continuous.
     * @param  huart UART handle
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_TransmitContinous(UART_HandleTypeDef *huart);

    /**
     * @brief UART Receive Resume.
     * @param  huart UART handle
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_ReceiveResume(UART_HandleTypeDef *huart);

    /**
     * @brief UART Transmit Resume.
     * @param  huart UART handle
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_TransmitResume(UART_HandleTypeDef *huart);

    /**
     * @brief stop and clean the UART Receive.
     * @param  huart UART handle
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_ReceiveStop(UART_HandleTypeDef *huart);

    /**
     * @brief stop and clean the UART Transmit.
     * @param  huart uart handle
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_TransmitStop(UART_HandleTypeDef *huart);

    /**
     * @brief  Register a User UART Callback
     * @note   The HAL_UART_RegisterCallback() should be called after
     * HAL_UART_Init()
     * @param  huart UART handle
     * @param  CallbackID ID of the callback to be registered
     * @param  pCallback pointer to the Callback function
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_RegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID, pUART_CallbackTypeDef pCallback);

    /**
     * @brief  Unregister a User UART Callback
     * @note   The HAL_UART_RegisterCallback() should be called after
     * HAL_UART_Init()
     * @param  huart UART handle
     * @param  CallbackID ID of the callback to be registered
     * @retval HAL_StatusTypeDef HAL status
     */
    HAL_StatusTypeDef HAL_UART_UnRegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID);

    /**
     * @brief Start uart receive data action in continous mode.
     * @param huart   UART handle
     * @retval HAL status
     *
     * This function only initiates the continuous mode of UART for receiving data,
     * and the action of copying data from the ifram is placed in the polling thread.
     */
    HAL_StatusTypeDef HAL_UART_ReceiveContinousForPoll(UART_HandleTypeDef *huart);

    /**
     * @brief Deactivate UART continuous reception mode and empty the FIFO to ensure clean state.
     * @param huart   UART handle
     * @retval HAL status
     *
     * This function Deactivate UART continuous mode,
     * empty the FIFO to ensure clean state to mitigate potential conflicts.
     */
    HAL_StatusTypeDef HAL_UART_DeactivateContinousForPoll(UART_HandleTypeDef *huart);

    /**
     * @brief Obtain the address and size information of the UART ifram.
     * @param huart   UART handle
     * @retval HAL status
     *
     * The information about the ifram would be more appropriately
     * stored in the UART_HandleTypeDef structure for each UART device.
     */
    HAL_StatusTypeDef HAL_UART_GetIFramInfoForPoll(UART_HandleTypeDef *huart, uint32_t *iframSize, uint8_t **iframBuff);

#ifdef __cplusplus
}
#endif

#endif /* DARIC_HAL_UART_H */
