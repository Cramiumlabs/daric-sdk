/**
 ******************************************************************************
 * @file    tx_work_item_queue.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TX_WORK_ITEM_QUEUE_H__
#define __TX_WORK_ITEM_QUEUE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "tx_api.h"
#include "tx_initialize.h"

/* Exported types ------------------------------------------------------------*/
/**
 * @brief  Workitem Callback pointer definition
 */
typedef void (*T_WorkItemCallback)(void *param);

typedef enum {
  DEV_ID_TP = 0x01LU,
  DEV_ID_OTHER = 0x02LU,
} T_DevIdType;



/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Initialize the work item queue and the thread for executing queue tasks.
 * @param  none.
 * @retval none.
 *
 * The current design allows multiple threads to process tasks in the work item queue in parallel. 
 * The default configuration uses a single thread mode, 
 * but you can modify WORK_THREAD_COUNT to change the number of worker threads.
  */
extern void daricWorkQueueInit(void);

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
extern int submitWorkItem(T_WorkItemCallback, void *param, T_DevIdType devId);

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
extern int submitWorkItemFront(T_WorkItemCallback, void *param, T_DevIdType devId);

#ifdef __cplusplus
}
#endif

#endif