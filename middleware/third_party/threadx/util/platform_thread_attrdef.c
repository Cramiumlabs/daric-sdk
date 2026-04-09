/**
 ******************************************************************************
 * @file    platform_thread_attrdef.c
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

/* Includes --------------------------------------------------------------------------*/
#include "platform_threads_attrdef.h"
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "THRD_INFO"

#define THREAD_ITEM(id, trdName, trdStkSize, trdPriority, trdPreemptThod, trdSlice) \
    static char gStack##id[trdStkSize] __attribute__((aligned(8)));
#include "platform_threads_table.h"
#undef THREAD_ITEM

#define THREAD_ITEM(id, trdName, trdStkSize, trdPriority, trdPreemptThod, trdSlice) \
    [id] = { \
        .stackSize      = trdStkSize, \
        .priority       = trdPriority, \
        .preemptThod    = trdPreemptThod, \
        .timeSlice      = trdSlice, \
        .pStack         = gStack##id, \
        .name           = trdName \
    },

T_ThreadAttrType gPltfrmThreads_cfg_table[PLATFORM_THREAD_MAXNUM] = {
    #include "platform_threads_table.h"
};
#undef THREAD_ITEM

#ifdef CONFIG_ENABLE_STACKCHECK
#include "user_threads_attrdef.h"
#include <stdint.h>
extern TX_THREAD maintask;

/**
 * @brief  Return whether the stack information has been statically initialized.
 * @param  None
 * @retval None.
 *
 * All thread information for resident memory should be initialized 
 * in two static structures, gPltfrmThreads_cfg_table and gUsrThreads_cfg_table, 
 * which contain information such as thread priority, thread stack, 
 * and other related content.
 * The function serves to determine whether the specified thread 
 * has already been initialized in the two structures, 
 * gUsrThreads_cfg_table and gPltfrmThreads_cfg_table.
 */
static int isThreadStackFilled(VOID *pStack)
{
    int i;
    /* By default, ThreadX stack filling is enabled */
    return 1;

    /* fill platform thread stack pattern */
    for (i = 0; i < PLATFORM_THREAD_MAXNUM; i++)
    {
        if (pStack == gPltfrmThreads_cfg_table[i].pStack)
        {
            return 1;
        }
    }

    /* fill user thread stack pattern */
    for (i = 0; i < USER_THREAD_MAXNUM; i++)
    {
        if (pStack == gUsrThreads_cfg_table[i].pStack)
        {
            return 1;
        }
    }    

    return 0;
}

/**
 * @brief  Count the number of bytes that have never been used in the stack.
 * @param  None
 * @retval None.
 *
 * Search from the start to the end of the stack for the first 
 * location that does not meet the specified pattern 0xEFEFEFEF.
 * This pattern 0xEF is used by the stack checking routines
 * to see how much has been used.
 */
static uint32_t getStackNeverBeenUsed(VOID *pStack, int stackSize)
{
    int i;
    int wSize = stackSize / 4;
    uint32_t *pScan = (uint32_t *)pStack;
    uint32_t emptyCnt = 0;
    for (i = 0;  i < wSize; i++)
    {
        if (*pScan != 0xEFEFEFEF)
        {
            return emptyCnt;
        }
        else
        {
            emptyCnt += 4;
            pScan++;
        }
    }
    return emptyCnt;
}

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
void fillStackCheckPattern(void)
{
    int i;

    /* fill platform thread stack pattern */
    for (i = 0; i < PLATFORM_THREAD_MAXNUM; i++)
    {
        memset(gPltfrmThreads_cfg_table[i].pStack, 0xEF, gPltfrmThreads_cfg_table[i].stackSize);
    }

    /* fill user thread stack pattern */
    for (i = 0; i < USER_THREAD_MAXNUM; i++)
    {
        memset(gUsrThreads_cfg_table[i].pStack, 0xEF, gUsrThreads_cfg_table[i].stackSize);
    }    
}

/**
 * @brief Dumps system task state information to logs
 * @param None
 * @return None
 * 
 * This function iterates through all created threads and logs their current state,
 * priority, and run count. It provides a comprehensive view of thread status 
 * for system debugging and monitoring purposes.
 * 
 * Log Format:
 * ===== Thread Status =====
 * Thread Name           State          Priority RunCount
 * ------------------------------------------------------------
 * main_thread          TX_READY             8       1234 <-- BLOCKING LOW POWER!
 * ====================================
 */
void sysDumpTaskStateInfo(void)
{
    UINT state;
    CHAR *pName;
    UINT priority;
    ULONG runCount;
    ULONG sliceNum;
    TX_THREAD *pThread;
    UINT preemThreshold;
    TX_THREAD *pNextThread;
    TX_THREAD *pSuspThread;
    TX_THREAD *pFirstThread;
    
    pThread = &maintask;
    pFirstThread = pThread;
    
    if (pThread == TX_NULL) {
        LOGV_RAW("No threads created\n");
        return;
    }
    
    LOGV_RAW("\r\n");
    LOGV("===== Thread Status =====");
    LOGV("%-20s %-15s Priority RunCount", "Thread Name", "State");
    LOGV("------------------------------------------------------------");
    
    do {
        tx_thread_info_get(pThread, 
                          &pName,
                          &state, 
                          &runCount,
                          &priority,
                          &preemThreshold,
                          &sliceNum,
                          &pNextThread,
                          &pSuspThread);
        
        const char *state_str;
        switch(state) {
            case TX_READY:
                state_str = "TX_READY";
                break;
            case TX_COMPLETED:
                state_str = "TX_COMPLETED";
                break;
            case TX_TERMINATED:
                state_str = "TX_TERMINATED";
                break;
            case TX_SUSPENDED:
                state_str = "TX_SUSPENDED";
                break;
            case TX_SLEEP:
                state_str = "TX_SLEEP";
                break;
            case TX_QUEUE_SUSP:
                state_str = "TX_QUEUE_SUSP";
                break;
            case TX_SEMAPHORE_SUSP:
                state_str = "TX_SEMAP_SUSP";
                break;
            case TX_EVENT_FLAG:
                state_str = "TX_EVENT_FLAG";
                break;
            case TX_BLOCK_MEMORY:
                state_str = "TX_BLOCK_MEMORY";
                break;
            case TX_BYTE_MEMORY:
                state_str = "TX_BYTE_MEMORY";
                break;
            case TX_MUTEX_SUSP:
                state_str = "TX_MUTEX_SUSP";
                break;
            default:
                state_str = "OTHER";
                break;
        }
        
        
        if (state == TX_READY) {
            LOGV("%-20s %-15s %8u %8lu <-- BLOCKING LOW POWER!", 
                pName ? pName : "NULL",
                state_str,
                priority,
                runCount);

        }
        else
        {
            // LOGV("%-20s %-15s %8u %8lu", 
            //     pName ? pName : "NULL",
            //     state_str,
            //     priority,
            //     runCount);
        }
        
        pThread = pThread->tx_thread_created_next;
        
    } while (pThread != pFirstThread);
    
    LOGV("====================================");
}

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
void sysDumpTaskInfo(void)
{
	TX_THREAD      *p_tcb;
    uint32_t    maxFreeStackSize;
    uint32_t    maxStackSize;
    uint32_t    curStackSize;
    uint32_t    usePercent;

    p_tcb = &maintask;
	LOGV_RAW("\r\n");
	LOGV("   Prio     StackSize   CurStack    MaxStack(%%%%)      Taskname");
	LOGV("===============================================================");

	while (p_tcb != (TX_THREAD *)0) 
	{
        maxFreeStackSize = 0;
        if (isThreadStackFilled(p_tcb->tx_thread_stack_start))
        {
            maxFreeStackSize = getStackNeverBeenUsed(p_tcb->tx_thread_stack_start, p_tcb->tx_thread_stack_size);
            curStackSize = (uint32_t)p_tcb->tx_thread_stack_end - (uint32_t)p_tcb->tx_thread_stack_ptr;
            maxStackSize = p_tcb->tx_thread_stack_size - maxFreeStackSize;
            usePercent = maxStackSize * 100 / p_tcb->tx_thread_stack_size;

            if (usePercent >= 70)
            {
                LOGV_COLOR(COL_RED, "   %2d        %5ld      %5ld       %5ld (%2ld%%%%)      %s", 
                            p_tcb->tx_thread_priority,
                            p_tcb->tx_thread_stack_size,
                            curStackSize,
                            maxStackSize,
                            usePercent,
                            p_tcb->tx_thread_name);

            }
            else
            {
                LOGV("   %2d        %5ld      %5ld       %5ld (%2ld%%%%)      %s", 
                            p_tcb->tx_thread_priority,
                            p_tcb->tx_thread_stack_size,
                            curStackSize,
                            maxStackSize,
                            usePercent,
                            p_tcb->tx_thread_name);
            }
        }
        else
        {
            LOGV("   %2d        %5ld      %5d       %11s      %s", 
                        p_tcb->tx_thread_priority,
                        p_tcb->tx_thread_stack_size,
                        (int)p_tcb->tx_thread_stack_end - (int)p_tcb->tx_thread_stack_ptr,
                        "N/A",
                        p_tcb->tx_thread_name);

        }

        p_tcb = p_tcb->tx_thread_created_next;

        if(p_tcb == &maintask) 
        {
            break;
        }
	}
    LOGV_RAW("\r\n");
}

#endif
/************************************* END OF FILE ************************************/
