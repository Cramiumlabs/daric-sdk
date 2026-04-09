/**
 ******************************************************************************
 * @file    platform_threads_attrdef.h
 * @author  OS Team
 * @brief   This file defines the basic structure needed to create a thread. 
 *          Threads in the project will use a unified structure for management, 
 *          The purpose is to facilitate adjust various thread attributes 
 *          and monitor thread stack usage.
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
#ifndef __PLATFORM_THREADS_ATTRDEF_H__
#define __PLATFORM_THREADS_ATTRDEF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_thread_attrdef.h"
/* Exported types ------------------------------------------------------------*/
/**
 * @brief  The thread enum definition
 */

#define THREAD_ITEM(id, trdName, trdStkSize, trdPriority, trdPreemptThod, trdSlice) id,
typedef enum {
    #include "platform_threads_table.h"
  PLATFORM_THREAD_MAXNUM,
} T_PlatformThreadEnum;
#undef THREAD_ITEM

extern T_ThreadAttrType gPltfrmThreads_cfg_table[PLATFORM_THREAD_MAXNUM];

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Fill stack with pattern 0xEF.
 * @param  None
 * @retval None.
 *
 * Set the thread stack to a pattern prior to creating the initial
 * stack.  This pattern 0xEF is used by the stack checking routines
 * to see how much has been used.
 * By default, ThreadX stack filling is enabled,
 * which places an 0xEF pattern in each byte of each thread's stack.  
 * This is used by debuggers with stack checking feature.
 */
extern void fillStackCheckPattern(void);

/**
 * @brief  Dump the priority of the thread and stack usage.
 * @param  None
 * @retval None.
 *
 * This function analyzes the stack to calculate the highest stack
 * pointer in the thread's stack. This can then be used to derive the
 * minimum amount of stack left for any given thread.
 * ThreadX also provides stack tracing feature,
 * When the TX_ENABLE_STACK_CHECKING is defined, ThreadX thread stack checking is enabled.  
 * However, the stack tracing feature in ThreadX has a bug that also affects thread switching performance, 
 * so we are not using the ThreadX method.
 */
extern void sysDumpTaskInfo(void);
#ifdef __cplusplus
}
#endif

#endif