/**
******************************************************************************
* @file    tx_work_item_queue.c
* @author  OS Team
* @brief   To speed up the interrupt service routine processing in the driver, 
*          a bottom_half interrupt handling mechanism has been implemented. 
*          Interrupt responses are placed in a queue for later processing, 
*          allowing the interrupt service routine to return immediately after 
*          queuing the work item, which will be handled by other threads.
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
#include "tx_initialize.h"
#include "tx_work_item_queue.h"
#include "platform_threads_attrdef.h"
#include "tx_log.h"
#include "tx_uart_continuous.h"

/* Private macro -------------------------------------------------------------*/
#define WORK_QUEUE_SIZE 40
#define WORK_THREAD_STACK_SIZE  2048
#define WORK_THREAD_COUNT       2

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
    T_WorkItemCallback  handler;
    void                *param;
    uint32_t            devId;
    uint32_t            dummy;
} t_work_item;

/* The work queue can hold up to WORK_QUEUE_SIZE pending work items.
 */
static t_work_item gWorkTpQueue[WORK_QUEUE_SIZE];
static TX_QUEUE gWorkTpQueueHandle;
static t_work_item gWorkNormalQueue[WORK_QUEUE_SIZE];
static TX_QUEUE gWorkNormalQueueHandle;

/* The current design allows multiple threads to process tasks in the work item queue in parallel. 
 * Modify WORK_THREAD_COUNT to change the number of worker threads.
 */
static TX_THREAD gWorkTpThreadHandle;
static TX_THREAD gWorkNormalThreadHandle[WORK_THREAD_COUNT];

/* Private function ----------------------------------------------------------*/
static void workTpItemThreadEntry(uint32_t thread_input);
static void workNormalItemThreadEntry(uint32_t thread_input);

/* 
 * All worker threads will use this function to process work items. 
 * Each thread dequeue work items from gWorkNormalQueueHandle, 
 * The read/write synchronization of the queue is ensured by ThreadX.
 */
static void workNormalItemThreadEntry(uint32_t thread_input)
{
    t_work_item workItem;
    UINT status;

    while (1) {
        status = tx_queue_receive(&gWorkNormalQueueHandle, &workItem, TX_WAIT_FOREVER);
        //LOGV("%s: receive msg  from queue!", __func__);
        if (status == TX_SUCCESS) {
            workItem.handler(workItem.param);
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

/*
 * Initialize the work item queue and the thread for executing queue tasks.
 */
static void initWorkItemQueue(void)
{
    uint32_t status;

    status = tx_queue_create(&gWorkTpQueueHandle, "Work tp Queue", sizeof(t_work_item) / sizeof(uint32_t),
                       gWorkTpQueue, sizeof(gWorkTpQueue));
    if (status != TX_SUCCESS) {
        LOGE("%s,%d: queue create fail,errno=%ld!", __func__, __LINE__, status);
        return;
    }

    status = tx_queue_create(&gWorkNormalQueueHandle, "Work Item Queue", sizeof(t_work_item) / sizeof(uint32_t),
                       gWorkNormalQueue, sizeof(gWorkNormalQueue));

    if (status == TX_SUCCESS) {
        status = tx_thread_create(&gWorkTpThreadHandle, 
                                  gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMTP].name, 
                                  workTpItemThreadEntry, 
                                  0,
                                  gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMTP].pStack,
                                  gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMTP].stackSize,
                                  gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMTP].priority,
                                  gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMTP].preemptThod,
                                  gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMTP].timeSlice,
                                  TX_AUTO_START);
        if (status != TX_SUCCESS) {
            LOGE("%s,%d: thread create fail,errno=%ld!", __func__, __LINE__, status);
            return;
        }
        #if WORK_THREAD_COUNT != 2
            #error "WORK_THREAD_COUNT must be 2 and must be consistent with the normal workitems in the platform_threads_table.h."
        #endif
        for (int i = 0; i < WORK_THREAD_COUNT; i++)
        {
            status = tx_thread_create(&gWorkNormalThreadHandle[i], 
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMOTHER0+i].name, 
                                      workNormalItemThreadEntry, 
                                      i,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMOTHER0+i].pStack,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMOTHER0+i].stackSize,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMOTHER0+i].priority,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMOTHER0+i].preemptThod,
                                      gPltfrmThreads_cfg_table[PLATFORM_THREAD_WORKITEMOTHER0+i].timeSlice,
                                      TX_AUTO_START);

            if (status != TX_SUCCESS) {
                LOGE("%s,%d: thread create fail,errno=%ld!", __func__, __LINE__, status);
                return;
            }
        }
    }
    else
    {
        LOGE("%s,%d: queue create fail,errno=%ld!", __func__, __LINE__, status);
    }
}

/**
 * @brief  Send a work item to the end of the queue.
 * @param  handler Pointer to a `void (*handler)(void *)` function point type 
 *         where the task that needs to be completed by the work item is done in this function.
 * @param  param Parameters to be passed to the callback function.
 * @param  devId Device ID, reserved for future expansion, 
 *         as there may be a need for special handling of special devices.
 * @retval Return 0 means success, return a non-zero value means failure.
 *
 * This function sends a work item to the end of the queue,
 * The goal is to simulate a FIFO work item queue.
 */
int submitWorkItem(T_WorkItemCallback handler, void *param, T_DevIdType devId)
{
    uint32_t status;
    t_work_item workItem;
    workItem.handler = handler;
    workItem.param = param;
    workItem.devId = devId;

    switch(devId)
    {
        case DEV_ID_TP:
            /* Send a message to the gWorkTpQueueHandle.
            Return immediately, regardless of success. 
            the TX_NO_WAIT wait option is used for calls from 
            initialization, timers, and ISRs. 
            */
            status = tx_queue_send(&gWorkTpQueueHandle, &workItem, TX_NO_WAIT);
            break;

        case DEV_ID_OTHER:
        default:
            /* Send a message to the gWorkNormalQueueHandle.
            Return immediately, regardless of success. 
            the TX_NO_WAIT wait option is used for calls from 
            initialization, timers, and ISRs. 
            */
            status = tx_queue_send(&gWorkNormalQueueHandle, &workItem, TX_NO_WAIT);
            break;

    }

    if (status != TX_SUCCESS) {
        LOGE("%s: queue send fail,errno=%ld!", __func__, status);
        return -1;
    }
    else
    {
        //LOGE("%s: queue send success,errno=%ld!", __func__, status);
    }

    return 0;
}

/**
 * @brief  Send a work item to the front of the queue.
 * @param  handler Pointer to a `void (*handler)(void *)` function point type 
 *         where the task that needs to be completed by the work item is done in this function.
 * @param  param Parameters to be passed to the callback function.
 * @param  devId Device ID, reserved for future expansion, 
 *         as there may be a need for special handling of special devices.
 * @retval Return 0 means success, return a non-zero value means failure.
 *
 * This function sends a work item to the front of the queue,
 * The goal is to simulate a high-priority work item.
 */
int submitWorkItemFront(T_WorkItemCallback handler, void *param, T_DevIdType devId)
{
    uint32_t status;
    t_work_item workItem;
    workItem.handler = handler;
    workItem.param = param;
    workItem.devId = devId;

    switch(devId)
    {
        case DEV_ID_TP:
            /* Send a message to the gWorkTpQueueHandle.
            Return immediately, regardless of success. 
            the TX_NO_WAIT wait option is used for calls from 
            initialization, timers, and ISRs. 
            */
            status = tx_queue_front_send(&gWorkTpQueueHandle, &workItem, TX_NO_WAIT);
            break;

        case DEV_ID_OTHER:
        default:
            /* Send a message to the gWorkNormalQueueHandle.
            Return immediately, regardless of success. 
            the TX_NO_WAIT wait option is used for calls from 
            initialization, timers, and ISRs. 
            */
            status = tx_queue_front_send(&gWorkNormalQueueHandle, &workItem, TX_NO_WAIT);
            break;

    }

    if (status != TX_SUCCESS) {
        LOGE("%s: queue send front fail,errno=%ld!", __func__, status);
        return -1;
    }
    else
    {
        //LOGE("%s: queue send front success,errno=%ld!", __func__, status);
    }

    return 0;
}

/* 
 * All worker threads will use this function to process work items. 
 * Each thread dequeue work items from gWorkTpQueueHandle, 
 * The read/write synchronization of the queue is ensured by ThreadX.
 */
static void workTpItemThreadEntry(uint32_t thread_input)
{
    t_work_item workItem;
    UINT status;

    while (1) {
        status = tx_queue_receive(&gWorkTpQueueHandle, &workItem, TX_WAIT_FOREVER);
        //LOGE("%s: receive msg  from queue!", __func__);
        if (status == TX_SUCCESS) {
            workItem.handler(workItem.param);
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

/**
 * @brief  Initialize the work item queue and the thread for executing queue tasks.
 * @param  none.
 * @retval none.
 *
 * The current design allows multiple threads to process tasks in the work item queue in parallel. 
 * The default configuration uses a single thread mode, 
 * but you can modify WORK_THREAD_COUNT to change the number of worker threads.
 */
void daricWorkQueueInit(void)
{
    initWorkItemQueue();
    daricUartContinuousServerInit();
}