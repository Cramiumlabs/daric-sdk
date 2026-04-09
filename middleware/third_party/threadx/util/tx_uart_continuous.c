/**
******************************************************************************
* @file    tx_uart_continuous.c
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

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "tx_api.h"
#include "platform_threads_attrdef.h"
#include "tx_log.h"
#include "tx_uart_continuous.h"
#include "daric_pulp_io.h"

#undef LOG_TAG
#define LOG_TAG "UART_CONTI"

/* Private macro -------------------------------------------------------------*/
#define POLL_QUEUE_SIZE     8
#define POLL_THREAD_COUNT   2
#define DARIC_UART_COUNT    4

/* Wait event definition */
typedef enum {
    WAIT_EVENT_UART0 = 1 << 0,  // wait uart0 event
    WAIT_EVENT_UART1 = 1 << 1,  // wait uart1 event
    WAIT_EVENT_UART2 = 1 << 2,  // wait uart2 event
    WAIT_EVENT_UART3 = 1 << 3,  // wait uart3 event
} WaitEventUart;


/** @brief Uart item structure */
typedef struct {
    void* buffer;                      /* User buffer for current request */
    uint32_t received;                 /* Bytes received so far */
    uint8_t result;                    /* Error codes for UART operations */
} t_uart_item;

/** @brief Uart controller structure */
static struct {
    TX_THREAD pollThreads[POLL_THREAD_COUNT];   /* Thread for polling UART buffer */
    TX_MUTEX mutex;                             /* Mutex for protecting state */
    TX_QUEUE msgQueueHandle;
    TX_EVENT_FLAGS_GROUP events;                /* Event flags for synchronization */
    t_uart_item uarts[DARIC_UART_COUNT];
    uint8_t isInited;                           /* Driver initialization state */
} gUartControl;

/*
    Specifies the size of each message in the queue. 
    Message sizes range from 1 32-bit word to 16 
    32-bit words. Valid message size options are 
    defined as follows:
    TX_1_ULONG (0x01)
    TX_2_ULONG (0x02)
    TX_4_ULONG (0x04)
    TX_8_ULONG (0x08)
    TX_16_ULONG (0x10)
*/
typedef struct {
    UART_HandleTypeDef  *huart;
    void                *buffer;
    uint32_t            size;
    uint32_t            timeOutTicks;
} t_read_item;

static t_read_item gUartPullQueue[POLL_QUEUE_SIZE];

/**
 * @brief Receive an amount of data in continous.
 * @param huart   UART handle
 * @param pData   Pointer to data buffer (u8 or u16 data elements).
 * @param Size    Amount of data elements (u8 or u16) to be received.
 * @param Timeout Timeout duration.
 * @retval The number of bytes received, 0: timeout, -1: error.
 */
static uint32_t uartPollReceiveData(UART_HandleTypeDef *huart, uint8_t *pData,
                             uint32_t Size, uint32_t Timeout)
{
    uint32_t index_in;
    uint32_t count = 0;
    uint32_t count_sum = 0;
    uint32_t data_size;
    uint32_t udma_rx_size;
    uint32_t iframSize;
    uint8_t *iframBuff;
    int err = -1;
    int cnt = tx_time_get();

    if (HAL_UART_GetIFramInfoForPoll(huart, &iframSize, &iframBuff) != HAL_OK)
    {
        return err;
    }
    while (Size > 0)
    {
        if (tx_time_get() - cnt > Timeout)
        {
            break;
        }
        udma_rx_size =
            pulp_read32(UDMA_UART_BASEADDR + UDMA_UART_PERIPH_SIZE * huart->id +
                       UART_RX_SIZE_OFFSET);
        index_in = iframSize - udma_rx_size;

        if (index_in == huart->RxBuffIndexOut ||
            (index_in == 0 && huart->RxBuffIndexOut == iframSize))
        {
            //tx_thread_sleep(2);
            continue;
        }

        if (index_in > huart->RxBuffIndexOut)
        {
            data_size = index_in - huart->RxBuffIndexOut;
            if (Size < data_size)
            {
                data_size = Size;
                memcpy(pData, &iframBuff[huart->RxBuffIndexOut], data_size);
                huart->RxBuffIndexOut += data_size;
                count += data_size;
                count_sum += count;
                break;
            }
            memcpy(pData, &iframBuff[huart->RxBuffIndexOut], data_size);
            pData += data_size;
            huart->RxBuffIndexOut += data_size;
            count += data_size;
            Size -= data_size;
        }
        else
        {
            data_size = iframSize - huart->RxBuffIndexOut;
            if (Size <= data_size)
            {
                count = Size;
                memcpy(pData, &iframBuff[huart->RxBuffIndexOut], count);
                huart->RxBuffIndexOut += data_size;
                count_sum += count;
                break;
            }
            else
            {
                if (data_size)
                {
                    memcpy(pData, &iframBuff[huart->RxBuffIndexOut], data_size);
                    Size -= data_size;
                    pData += data_size;
                    count = data_size;
                }
                data_size = index_in;
                if (Size < data_size)
                {
                    data_size = Size;
                    memcpy(pData, &iframBuff[huart->RxBuffIndexOut], data_size);
                    huart->RxBuffIndexOut += data_size;
                    count += data_size;
                    count_sum += count;
                    break;
                }
                memcpy(pData, &iframBuff[0], data_size);
                pData += data_size;
                count += data_size;
                Size -= data_size;
                huart->RxBuffIndexOut = data_size;
            }
        }
        count_sum += count;
        count = 0;
    }
    huart->RxState = HAL_UART_STATE_READY;
    return count_sum;
}
/* 
 * All pull threads will use this function to process read items. 
 * Each thread dequeue work items from gUartPullQueue, 
 * The read/write synchronization of the queue is ensured by ThreadX.
 */
static void uartPullItemThreadEntry(uint32_t thread_input)
{
    t_read_item readItem;
    UART_HandleTypeDef *handler;
    UINT status;
    uint32_t waitEvent = 0;
    int num = 0;

    while (1) {
        status = tx_queue_receive(&gUartControl.msgQueueHandle, &readItem, TX_WAIT_FOREVER);
        //LOGV("%s: receive msg  from queue!", __func__);
        if (status == TX_SUCCESS) {

            handler = readItem.huart;
            num = uartPollReceiveData(handler, readItem.buffer, readItem.size, readItem.timeOutTicks);
            
            tx_mutex_get(&gUartControl.mutex, TX_WAIT_FOREVER);
            if (num < 0)
            {
                gUartControl.uarts[handler->id].received = 0;
                gUartControl.uarts[handler->id].result = UART_ERROR_OTHER;
            }
            else if (num != readItem.size)
            {
                gUartControl.uarts[handler->id].received = num;
                gUartControl.uarts[handler->id].result = UART_ERROR_TIMEOUT;
            }
            else
            {
                gUartControl.uarts[handler->id].received = num;
                gUartControl.uarts[handler->id].result = UART_OK;
            }

            switch(handler->id)
            {
            case 0:
                waitEvent = WAIT_EVENT_UART0;
                break;
            case 1:
                waitEvent = WAIT_EVENT_UART1;
                break;
            case 2:
                waitEvent = WAIT_EVENT_UART2;
                break;
            case 3:
                waitEvent = WAIT_EVENT_UART3;
                break;
            default:
                LOGE("%s: param fail,errno=%ld!", __func__, status);
            }
            tx_mutex_put(&gUartControl.mutex);

            tx_event_flags_set(&gUartControl.events, waitEvent, TX_OR);

        }
        else if (status == TX_DELETED)
        {
            LOGW("%s: queue be deleted!", __func__);
        }
        else
        {
            LOGE("%s: queue receive fail,errno=%d!", __func__, status);
        }
    }
}

static int initUartContinuMode(void)
{
    uint32_t status;
    int i;

    status = tx_queue_create(&gUartControl.msgQueueHandle, "uart pullQueue", sizeof(t_read_item) / sizeof(uint32_t),
                       gUartPullQueue, sizeof(gUartPullQueue));
    if (status != TX_SUCCESS) {
        LOGE("%s,%d: queue create fail,errno=%ld!", __func__, __LINE__, status);
        return -1;
    }

    /* Create synchronization objects */
    status = tx_mutex_create(&gUartControl.mutex, "uart mutex", TX_INHERIT);
    if (status != TX_SUCCESS) {
        tx_queue_delete(&gUartControl.msgQueueHandle);
        return -1;
    }

    status = tx_event_flags_create(&gUartControl.events, "uart_events");
    if (status != TX_SUCCESS) {
        tx_queue_delete(&gUartControl.msgQueueHandle);
        tx_mutex_delete(&gUartControl.mutex);
        return -1;
    }

    if (status == TX_SUCCESS) {
        #if POLL_THREAD_COUNT != 2
            #error "POLL_THREAD_COUNT must be 2 and must be consistent with the readitems in the platform_threads_table.h."
        #endif
        /* Create polling thread */
        for (i = 0; i < POLL_THREAD_COUNT; i++)
        {
            status = tx_thread_create(&gUartControl.pollThreads[i], 
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_UARTPOLL0+i].name, 
                                      uartPullItemThreadEntry, 
                                      i,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_UARTPOLL0+i].pStack,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_UARTPOLL0+i].stackSize,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_UARTPOLL0+i].priority,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_UARTPOLL0+i].preemptThod,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_UARTPOLL0+i].timeSlice,
                                      TX_AUTO_START);

            if (status != TX_SUCCESS) {
                LOGE("%s,%d: thread create fail,errno=%ld!", __func__, __LINE__, status);
                return -1;
            }
        }
    }
    else
    {
        LOGE("%s,%d: queue create fail,errno=%ld!", __func__, __LINE__, status);
        return -1;
    }

    return 0;
}

/**
 * @brief  Initialize the uart readItem queue and the thread for executing uart poll tasks.
 * @param  none.
 * @retval Return UART_OK means success, return other value means failure.
 *
 * The current design uses multiple threads to process read requests 
 * from different UART controllers in parallel.
 * This function initialize the uart readItem queue,
 * and create the thread for executing uart poll tasks.
 * you can modify POLL_THREAD_COUNT to change the number of worker threads.
 */
t_pullUartStatus daricUartContinuousServerInit(void) 
{

    if (gUartControl.isInited) {
        return UART_OK;
    }

    memset(&gUartControl, 0, sizeof(gUartControl));

    if (initUartContinuMode() < 0)
    {
        return UART_ERROR_OTHER;
    }

    /* Initialize state */
    gUartControl.isInited = 1;

    return UART_OK;
}



/**
 * @brief  Send a work item to the end of the queue.
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
                                uint32_t size, uint32_t timeOutTicks)
{
    uint32_t status;
    uint32_t waitEvent = 0;
    uint32_t actualFlags = 0;
    t_read_item readItem;

    if (handler == NULL || handler->id > 3)
    {
        LOGE("%s: param fail,errno=%ld!", __func__);
        return -1;
    }

    readItem.huart          = handler;
    readItem.buffer         = buffer;
    readItem.size           = size;
    readItem.timeOutTicks   = timeOutTicks;

    status = tx_queue_send(&gUartControl.msgQueueHandle, &readItem, TX_NO_WAIT);

    if (status != TX_SUCCESS) {
        LOGE("%s: queue send fail,errno=%ld!", __func__, status);
        return -1;
    }

    switch(handler->id)
    {
    case 0:
        waitEvent = WAIT_EVENT_UART0;
        break;
    case 1:
        waitEvent = WAIT_EVENT_UART1;
        break;
    case 2:
        waitEvent = WAIT_EVENT_UART2;
        break;
    case 3:
        waitEvent = WAIT_EVENT_UART3;
        break;
    default:
        LOGE("%s: param fail,errno=%ld!", __func__, status);
        return -1;
    }
    status = tx_event_flags_get(&gUartControl.events,
                          waitEvent,
                          TX_OR_CLEAR,
                          &actualFlags,
                          TX_WAIT_FOREVER);
    if (status != TX_SUCCESS) {
        LOGE("%s: event get fail,errno=%ld!", __func__, status);
        return -1;
    }

    return gUartControl.uarts[handler->id].received;
}
