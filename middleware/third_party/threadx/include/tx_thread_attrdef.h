/**
 ******************************************************************************
 * @file    tx_thread_attrdef.h
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
#ifndef __TX_THREAD_ATTRDEF_H__
#define __TX_THREAD_ATTRDEF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "tx_api.h"
#include "tx_initialize.h"

/*******************************************************************************
 * Daric Thread Priority Definitions
 *
 * Priority Range: 0-31 (Lower number = Higher priority)
 *
 * The priority levels are grouped into categories:
 *  ISR/Interrupt (0-5):     For interrupt handling and critical responses
 *  Device Drivers (6-10):   For device and peripheral management
 *  System Services (11-15): For core system functionalities
 *  UI/Servers (16-20):      For user interface and server tasks
 *  Background (21-30):      For non-critical background operations
 *  Idle (31):               For system idle thread
*******************************************************************************/

/*******************************************************************************
* Interrupt Service Routine (ISR) Priorities (0-5)
* Handles time-critical interrupt processing and immediate responses
*******************************************************************************/
#define TX_PRIORITY_ISR_CRITICAL    1    /* Most critical interrupt handlers */
#define TX_PRIORITY_ISR_HIGH        2    /* High-priority interrupt processing */
#define TX_PRIORITY_ISR_NORMAL      3    /* Normal interrupt handling tasks */
#define TX_PRIORITY_ISR_LOW         4    /* Low interrupt handling tasks */

/*******************************************************************************
* Device Driver Priorities (6-10)
* Manages hardware interfaces and peripheral operations
*******************************************************************************/
#define TX_PRIORITY_DRIVER_CRITICAL 7       /* Critical driver operations */
#define TX_PRIORITY_DRIVER_HIGH     8       /* High-priority driver management */
#define TX_PRIORITY_DRIVER_NORMAL   9       /* Standard driver operations */
#define TX_PRIORITY_DRIVER_LOW      10      /* Low-priority driver tasks */

/*******************************************************************************
* System Service Priorities (11-15)
* Core system services and maintenance tasks
*******************************************************************************/
#define TX_PRIORITY_SYS_CRITICAL    12   /* Critical system operations */
#define TX_PRIORITY_SYS_HIGH        13   /* High-priority system management */
#define TX_PRIORITY_SYS_NORMAL      14   /* Standard system operations */
#define TX_PRIORITY_SYS_LOW         15   /* Low-priority system tasks */

/*******************************************************************************
* UI and Server Priorities (16-20)
* User interface and general application tasks
*******************************************************************************/
#define TX_PRIORITY_UI              16   /* UI thread priority */
#define TX_PRIORITY_SERVER_HIGH     17   /* High-priority application tasks */
#define TX_PRIORITY_SERVER_NORMAL   18   /* Normal application operations */
#define TX_PRIORITY_SERVER_LOW      19   /* Low-priority application tasks */

/*******************************************************************************
* Background Task Priorities (21-30)
* Non-time-critical background operations
*******************************************************************************/
#define TX_PRIORITY_BKG_HIGH        21  /* High-priority background tasks */
#define TX_PRIORITY_BKG_NORMAL      25  /* Normal background operations */
#define TX_PRIORITY_BKG_LOW         29  /* Low-priority background tasks */

/*******************************************************************************
* Idle Thread Priority
* Reserved for system idle processing
*******************************************************************************/
#define TX_PRIORITY_IDLE           31    /* System idle thread */

/* Number of timer-ticks this thread is allowed to 
 * run before other ready threads of the same 
 * priority are given a chance to run. Note that 
 * using preemption-threshold disables time-slicing. 
 */
#define TX_TIME_SLICE_10        10
#define TX_TIME_SLICE_50        50


#define STACK_SIZE_2KB      2048
#define STACK_SIZE_4KB      4096
#define STACK_SIZE_5KB      5120
#define STACK_SIZE_6KB      6144
#define STACK_SIZE_8KB      8192
#define STACK_SIZE_10KB     10240
#define STACK_SIZE_15KB     15360
#define STACK_SIZE_28KB     28 * 1024

/* Exported types ------------------------------------------------------------*/
/**
 * @brief  The thread attribute definition
 */
typedef struct thread_attr_struct
{
    ULONG   stackSize;      /* Stack size in bytes */
    UINT    priority;       /* Priority of thread (0-31) */
    UINT    preemptThod;    /* Preemption threshold */
    ULONG   timeSlice;      /* Thread time-slice value */
    VOID *  pStack;         /* Pointer to start of stack */
    CHAR *  name;           /* Pointer to thread name string */
} T_ThreadAttrType;

#ifdef __cplusplus
}
#endif

#endif
