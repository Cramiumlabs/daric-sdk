/**
 ******************************************************************************
 * @file    platform_threads_table.h
 * @author  OS Team
 * @brief   All properties of the usr threads are defined here, 
 *          and the application uses the corresponding ID to access 
 *          the relevant properties of the threads.
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
/* Genesis,Initial thread, the first thread started by the system
 * where the main function is executed.
 */
THREAD_ITEM(PLATFORM_THREAD_MAIN,       //id
            "Main",              //name
            STACK_SIZE_4KB,             //stackSize
            TX_PRIORITY_SYS_NORMAL,     //Priority
            TX_PRIORITY_SYS_NORMAL,     //PreemptThod
            TX_TIME_SLICE_10)           //Slice

/* 
 * This thread will handle all workitems related to the touchpanel.
 * Each thread dequeue work items from gWorkTpQueueHandle, 
 * The read/write synchronization of the queue is ensured by ThreadX.
 */
THREAD_ITEM(PLATFORM_THREAD_WORKITEMTP, //id
            "Workitem tp",    //name
            STACK_SIZE_4KB,             //stackSize
            TX_PRIORITY_ISR_CRITICAL,   //Priority
            TX_PRIORITY_ISR_CRITICAL,   //PreemptThod
            TX_NO_TIME_SLICE)           //Slice

/* 
 * These threads handle general interrupt half-bottom workitems.
 * Each thread dequeue work items from gWorkNormalQueueHandle, 
 * !!Do not change the order and do not insert any other definitions.
 */
//{BEGIN
THREAD_ITEM(PLATFORM_THREAD_WORKITEMOTHER0, //id
            "Workitem other0",       //name
            STACK_SIZE_4KB,             //stackSize
            TX_PRIORITY_ISR_CRITICAL,   //Priority
            TX_PRIORITY_ISR_CRITICAL,   //PreemptThod
            TX_NO_TIME_SLICE)           //Slice

THREAD_ITEM(PLATFORM_THREAD_WORKITEMOTHER1, //id
            "Workitem other1",       //name
            STACK_SIZE_4KB,             //stackSize
            TX_PRIORITY_ISR_CRITICAL,   //Priority
            TX_PRIORITY_ISR_CRITICAL,   //PreemptThod
            TX_NO_TIME_SLICE)           //Slice
//}END

/* 
 * These threads implements a UART handling system with UARTs
 * operating in continuous mode (ring buffer).
 * Each thread dequeue work items from gUartPullQueue, 
 * !!Do not change the order and do not insert any other definitions.
 */
//{BEGIN
THREAD_ITEM(PLATFORM_THREAD_UARTPOLL0, //id
            "uart pull0",       //name
            STACK_SIZE_2KB,             //stackSize
            TX_PRIORITY_SYS_CRITICAL,   //Priority
            TX_PRIORITY_SYS_CRITICAL,   //PreemptThod
            TX_NO_TIME_SLICE)           //Slice

THREAD_ITEM(PLATFORM_THREAD_UARTPOLL1, //id
            "uart pull1",       //name
            STACK_SIZE_2KB,             //stackSize
            TX_PRIORITY_SYS_CRITICAL,   //Priority
            TX_PRIORITY_SYS_CRITICAL,   //PreemptThod
            TX_NO_TIME_SLICE)           //Slice
//}END