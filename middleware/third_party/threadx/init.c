/**
 ******************************************************************************
 * @file    init.c
 * @author  Init Team
 * @brief   The file provides functions for Threadx initialization
 *
 ******************************************************************************
 * @attention
 *
 * © Copyright CrossBar, Inc. 2024.

 * All rights reserved.

 * This software is the proprietary property of CrossBar, Inc. and is protected
 * by copyright laws. Any unauthorized reproduction, distribution, or
 * modification is strictly prohibited.
 *
 ******************************************************************************
 */

#include <stdint.h>

#include "daric.h"
#include "daric_hal.h"

#include "tx_api.h"
#include "tx_initialize.h"
#include "tx_work_item_queue.h"
#include "platform_threads_attrdef.h"
#include "tx_log.h"
#include "daric_hal_ao.h"

#ifndef CONFIG_THREADX_TEST
TX_THREAD maintask;
void MainTask(ULONG arg)
{
    /* Application main */
    extern int main(void);
    (void)main();

    /* Self exit */
}

void tx_application_define(void *first_unused_memory)
{
    /**
     * @warning first_unused_memory must NOT be used!
     */
    (void)(first_unused_memory);

    //static TX_THREAD maintask;

    /* Sets up the logging system by creating a mutex for 
     * thread-safe operations and initializing the log level
     */
    daricLogInit(CONFIG_LOG_LEVEL, CONFIG_LOG_TAG);

    /* This function needs to be called to complete the initialization
     *  of the bottom_half interrupt mechanism before using any drivers.
     */
    daricWorkQueueInit();

    /**
     * Priority of the main task is set to one-level lower
     * than the timer task.
     */
    tx_thread_create(&maintask, 
                     gPltfrmThreads_cfg_table[PLATFORM_THREAD_MAIN].name, 
                     MainTask, 
                     (ULONG)0, 
                     gPltfrmThreads_cfg_table[PLATFORM_THREAD_MAIN].pStack,
                     gPltfrmThreads_cfg_table[PLATFORM_THREAD_MAIN].stackSize, 
                     gPltfrmThreads_cfg_table[PLATFORM_THREAD_MAIN].priority, 
                     gPltfrmThreads_cfg_table[PLATFORM_THREAD_MAIN].preemptThod, 
                     gPltfrmThreads_cfg_table[PLATFORM_THREAD_MAIN].timeSlice,
                     TX_AUTO_START);
}
#else
void external_exit(UINT code)
{
    (void)code;
    while (1)
    {
        /* loop forever */
    }
}
#endif

static ARM_MPU_Region_t mpu_config_table[] = DARIC_MPU_CONFIG;

static void MPU_Config(void)
{
    ARM_MPU_Disable();

    ARM_MPU_Load(&mpu_config_table[0],
                 sizeof(mpu_config_table) / sizeof(mpu_config_table[0]));

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
}

#ifndef CONFIG_DISABLE_CACHE
static void CACHE_Enable(void)
{
    SCB_EnableICache();
    // SCB_EnableDCache();
}
#endif

TX_BYTE_POOL DefaultHeap;

static void HEAP_Init(void)
{
    extern int __HeapBase;
    extern int __HeapLimit;

    tx_byte_pool_create(&DefaultHeap, "Default Heap", &__HeapBase,
                        (uintptr_t)&__HeapLimit - (uintptr_t)&__HeapBase);
}

static int CHAR_Out(int _c)
{
#ifdef CONFIG_USE_SEMIHOSTING_PRINTF
    char c = _c;

    register unsigned long r0 __asm__("r0") = 0x03;
    register void *r1 __asm__("r1") = &c;

    __asm__ __volatile__("bkpt 0xab" : : "r"(r0), "r"(r1) : "memory");
#else
    /* Use Debug UART by default */
    DUART_PutChar(_c);
#endif

    return 0;
}

static void STDOUT_Setup(void)
{
    extern void __stdout_hook_install(int (*hook)(int));
    __stdout_hook_install(CHAR_Out);
}

void _start(void)
{
    MPU_Config();

#ifndef CONFIG_DISABLE_CACHE
    CACHE_Enable();
#endif
    #ifdef CONFIG_ENABLE_STACKCHECK
    /*
     * By default, ThreadX stack filling is enabled,
     * which places an 0xEF pattern in each byte of each thread's stack.  
     * This is used by debuggers with stack checking feature.
     */ 
    //fillStackCheckPattern();
    #endif
    _tx_initialize_kernel_setup();

    HEAP_Init();

    HAL_Init();

    STDOUT_Setup();

    /* Force reset these two AO registers 
       to avoid abnormal power consumption on the PF port
     */
    AON->IOX = AO_GPIOF_AF0_GPIO;
    AON->PADPU = 0;    
    #ifdef CONFIG_SUPPORT_DEEPSLEEP
    extern void LowerPower_Init(void);
    LowerPower_Init();
    #endif

    /* Initialize and enter the ThreadX kernel. */
    tx_kernel_enter();

    /* Should not get here. */
    while (1)
    {
        /* loop */
    }
}

void printExcepStack(uint32_t *stack_frame)
{
    #define MAX_BACK_DEPTH 20
    uint32_t *sp;
    uint32_t cnt = 1;

    
    TX_THREAD *pCurThread;
    //__asm volatile ("mov %0, sp" : "=r"(sp));
    sp = (uint32_t *)(stack_frame + 8);
    pCurThread = tx_thread_identify();

    uint32_t * pStackEnd = (uint32_t *)pCurThread->tx_thread_stack_end;
    for (uint32_t *p = sp; p < pStackEnd && cnt < MAX_BACK_DEPTH; p++) {
        uint32_t val = *p;
        
        if ((val >= 0x60050000 && val < (0x602C0000)) && (val & 0x1)) 
        //if ((val >= 0x61000000 && val < (0x61200000)) && (val & 0x1)) 
        {
            void *addr = (void *)(val & ~0x1);
            
            //printf("  ===>0x%08lx\r\n", (uint32_t)addr);
            printf("  #%02ld pc 0x%08lX\n", cnt, (uint32_t)addr);
            cnt++;
        }
    }

}