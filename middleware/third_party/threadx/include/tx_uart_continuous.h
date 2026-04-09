/**
 ******************************************************************************
 * @file    tx_uart_continuous.h
 * @author  OS Team
* @brief   Using the CONTINOUS mode to receive data using ThreadX thread 
*          This module implements a UART handling system  with UARTs
*          operating in continuous mode (ring buffer). 
*          Reduces the risk of missing data in the interrupt mode. 
*          The UART hardware will continuously overwrite data in the ring buffer, 
*          so a too-small ring buffer cannot be used.
******************************************************************************
* @attention
*
* © Copyright CrossBar, Inc. 2024.
*
* All rights reserved.
*
* This software is the proprietary property of CrossBar, Inc. and is protected
* by copyright laws. Any unauthorized reproduction, distribution, or
* modification is strictly prohibited.
*
******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TX_UART_CONTINUOUS_H
#define TX_UART_CONTINUOUS_H

#include "tx_api.h"
#include "daric_hal_uart.h"

/** @brief Error codes for UART operations */
typedef enum {
    UART_OK = 0,             /* Operation completed successfully */
    UART_ERROR_PARAM,        /* Invalid parameter */
    UART_ERROR_BUSY,         /* Another request is in progress */
    UART_ERROR_TIMEOUT,      /* Operation timed out */
    UART_ERROR_OTHER         /* Other error occurred */
} t_pullUartStatus;

/**
 * @brief  Initialize the uart readItem queue and create thread for executing uart poll tasks.
 * @param  none.
 * @retval Return UART_OK means success, return other value means failure.
 *
 * The current design uses multiple threads to process read requests 
 * from different UART controllers in parallel.
 * This function initialize the uart readItem queue,
 * and create the thread for executing uart poll tasks.
 * you can modify POLL_THREAD_COUNT to change the number of worker threads.
 */
t_pullUartStatus daricUartContinuousServerInit(void);


/**
 * @brief  Use the CONTINOUS mode to read data from the UART.
 * @param  handler Pointer to a `UART_HandleTypeDef` UART hardware handle 
 * @param  buffer Pointer to user buffer for storing received data.
 *                The length of the buffer must be greater than the size; 
 *                otherwise, it will corrupt the memory.
 * @param  size Size of data to be read.
 * @param  timeOutTicks Timeout duration.
 * @retval Return bytes received.
 *
 * This function sends a poll item to the end of the queue,
 * The thread calling this function will be blocked here 
 * until the data has been read completely or the timeout period has elapsed.
 */
int daricUartContinuReadSync(UART_HandleTypeDef *handler, void *buffer, 
                                uint32_t size, uint32_t timeOutTicks);

#endif /* TX_UART_CONTINUOUS_H */