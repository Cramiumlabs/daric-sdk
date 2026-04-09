/**
 ******************************************************************************
 * @file    sleep.c
 * @author  LOW_POWER Team
 * @brief   low_power module code.
 *          This file provides firmware functions to manage the following
 *          functionalities of the low_kpower
 *           + Register and unregister hook fun of devices
 *           + Wakelock fun for disable or enable sleep_mode
 *           + set the chip to sleep mode or doze mode
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
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "daric_hal_reram.h"

#if defined(HAL_PMU_MODULE_ENABLED) && defined(HAL_AO_MODULE_ENABLED)

#include "daric.h"
#include "daric_hal.h"
#include "daric_hal_ao.h"
#include "daric_hal_pmu.h"
#include "daric_hal_rtc.h"
#include "daric_hal_wdg.h"
#include "sys_config.h"
#include "tx_low_power_user.h"

static AO_HandleTypeDef s_hao;
static PMU_HandleTypeDef s_hpmu;

extern int TG28_POK_Config(uint8_t enable, uint8_t poweroff_time, uint8_t poweron_time);
extern int TG28_Soft_Power_Off(void);
extern int TG28_BATFET_en(bool enable);
extern void clearTG28Irq(void);
#ifdef CONFIG_SUPPORT_DEEPSLEEP
extern void aw2023_led_off(void);
extern void aw2023_enter_lowpower();
#ifdef CONFIG_PD_RTC_ALARM_TO_POWEROFF
extern int32_t BSP_RTC_SetAlarmDelayMinutes(uint16_t minutes);
#endif
#endif

/**
 * @brief Configure system clock frequencies for low-power operation
 * @param None
 * @return None
 *
 * @note Configures multiple clock domains through CGU (Clock Generation Unit) registers
 *       CPU core clock set to 48MHz (from default higher frequency)
 *       AXI/QFC/SRAM clock set to 48MHz
 *       AHB/DMA/SCE clock set to 48MHz
 *       Interface clock set to 48MHz
 *       APB peripheral clock set to 48MHz (WDT, SYSCtrl, SEC, TIMER, etc.)
 *       AON domain clock (aoclk) reduced to 1.5MHz for minimal power consumption
 *       Peripheral clock (clkper) set to 48MHz
 *
 * This function sets the clock frequencies for various system domains to
 * optimized values for low-power operation. It reduces clock speeds where
 * possible while maintaining necessary functionality.
 */
static void LowPower_ClockConfig(void)
{
    DARIC_CGU->fdfclk = (DARIC_CGU->fdfclk & ~(0xFF00FF00)) | (0xFF00FF00);   // CPU core----48MHz
    DARIC_CGU->fdaclk = (DARIC_CGU->fdaclk & ~(0xFF00FF00)) | (0xFF00FF00);   // AXI/QFC/SRAM----48MHz
    DARIC_CGU->fdhclk = (DARIC_CGU->fdhclk & ~(0xFF00FF00)) | (0xFF00FF00);   // AHB/DMA/SCE----48MHz
    DARIC_CGU->fdiclk = (DARIC_CGU->fdiclk & ~(0xFF00FF00)) | (0xFF00FF00);   // Interface----48MHz
    DARIC_CGU->fdpclk = (DARIC_CGU->fdpclk & ~(0xFF00FF00)) | (0xFF00FF00);   // APB/WDT/SYSCtrl/SEC/TIMER/...----48MHz
    DARIC_CGU->fdaoclk = (DARIC_CGU->fdaoclk & ~(0xFF00FF00)) | (0x00000700); // aoclk----1.5MHz
    DARIC_CGU->fdper = (DARIC_CGU->fdper & ~(0xFF00FF00)) | (0xFF00FF00);     // clkper----48MHz
}

/**
 * @brief Low-power timer interrupt callback function
 * @param param Callback parameter (unused in this implementation)
 * @return None
 *
 * @note Enables RTC interrupt to ensure proper timer functionality after wakeup
 *       Clears RTC flags to prevent spurious interrupt triggers
 *       Notifies wakeup source as low-power timer for proper system resume handling
 *       Critical for coordinated wakeup sequence from deep sleep states
 *
 * This function serves as the callback handler for low-power timer interrupts.
 * It enables the RTC interrupt, clears the RTC flag, and notifies the system
 * that the wakeup source is the low-power timer.
 */
static void LowPower_Timer_IrqCallback(void *param)
{
    (void)param;
    HAL_RTC_IT_Enable(0);
    HAL_RTC_ClearFlag();
    low_power_notify_wakeup_source(true);
}

/**
 * @brief Enter specified low-power mode with hardware configuration
 * @param param Pointer to the low-power state to enter (lp_state type)
 * @return 0 on success, -1 if an invalid mode is specified
 *
 * @note Performs hardware validation to ensure the requested mode is valid
 * @note Each case implements specific hardware configurations for optimal power savings
 * @note For deep sleep modes:
 *        - Enables deep sleep mode in SCB
 *        - Disables SysTick interrupt
 *        - Clears pending SysTick interrupts
 *        - Configures RTC for wakeup timing
 *        - Applies low-power clock configuration
 *        - Sets up CGU (Clock Generation Unit) for low-power operation
 *        - Uses memory barriers for instruction synchronization
 * @note For power-down mode:
 *        - Configures fast boot settings for quick resume
 *        - Sets up power control for complete shutdown
 *        - Handles board-specific configuration variations
 *        - Implements software power-off sequence
 *
 * This function implements the hardware-specific low-power entry sequence for
 * different power states. It configures the appropriate clock, power, and
 * interrupt settings for each low-power mode before entering the actual sleep state.
 */
UINT LowPower_User_Enter(VOID *param)
{
    lp_state mode = *(lp_state *)param;

    if (mode > LOW_POWER_POWER_DOWN)
    {
        return -1;
    }

    switch (mode)
    {
    case LOW_POWER_SLEEP:
        HAL_PMU_EnterLowPowerMode(&s_hpmu, PMU_MODE_SLEEP);
        break;

    case LOW_POWER_DEEP_SLEEP_FD:
        printf("=====>Enter LOW_POWER_DEEP_SLEEP_FD Mode\r\n");
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk; // clear systick pending
        HAL_RTC_ClearFlag();
        HAL_RTC_IT_Enable(1);
        HAL_NVIC_EnableIRQ(AOWKUPINT_IRQn);
        LowPower_ClockConfig();
        DARIC_CGU->cgulp = 0x01; // clktop --> lpcfg
        DARIC_CGU->cgusel0 = 0x01;
        DARIC_CGU->cguset = 0x32;
        __DSB();
        HAL_PMU_EnterLowPowerMode(&s_hpmu, PMU_MODE_DEEP_SLEEP);
        __DSB();
        __ISB();
        break;

    case LOW_POWER_DEEP_SLEEP_IP:
        printf("=====>Enter LOW_POWER_DEEP_SLEEP_IP Mode\r\n");
        HAL_PMU_ConfigLDO(&s_hpmu, PMU_MODE_DEEP_SLEEP, PMU_TRM_VDD85D, PMU_TRM_VOLTAGE_0_80V);
        HAL_PMU_ConfigLDO(&s_hpmu, PMU_MODE_DEEP_SLEEP, PMU_TRM_VDD85A, PMU_TRM_VOLTAGE_0_80V);
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk; // clear systick pending
        HAL_RTC_ClearFlag();
        HAL_RTC_IT_Enable(1);
        HAL_NVIC_EnableIRQ(AOWKUPINT_IRQn);
        LowPower_ClockConfig();
        /* Turn off the watchdog before enter deep sleep. */
        HAL_WDG_Disable();
        DARIC_CGU->cgulp = 0x03; // Bit[0]=1, clktop --> lpcfg
                                 // Bit[1]=1, ip --> iplpcfg
        DARIC_CGU->cgusel0 = 0x01;
        DARIC_CGU->cguset = 0x32;
        __DSB();
        DARIC_IPC->lpen = 0x0F; // pmu --> pmulpcfg
        __DSB();
        DARIC_IPC->ar = 0x32; // enable config
        __DSB();
        HAL_PMU_EnterLowPowerMode(&s_hpmu, PMU_MODE_DEEP_SLEEP);
        __DMB();
        DARIC_IPC->ar = 0x57; // enter lp flow
        __DSB();
        __ISB();
        break;

    case LOW_POWER_DEEP_SLEEP_DEEP:
        printf("=====>Enter LOW_POWER_DEEP_SLEEP_DEEP Mode\r\n");
        HAL_PMU_ConfigLDO(&s_hpmu, PMU_MODE_DEEP_SLEEP, PMU_TRM_VDD85D, PMU_TRM_VOLTAGE_0_80V);
        HAL_PMU_ConfigLDO(&s_hpmu, PMU_MODE_DEEP_SLEEP, PMU_TRM_VDD85A, PMU_TRM_VOLTAGE_0_80V);
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk; // clear systick pending
        HAL_RTC_ClearFlag();
        HAL_RTC_IT_Enable(1);
        HAL_NVIC_EnableIRQ(AOWKUPINT_IRQn);
        LowPower_ClockConfig();
        DARIC_CGU->cgulp = 0x07; // Bit[0]=1, clktop --> lpcfg
                                 // Bit[1]=1, ip --> iplpcfg
                                 // Bit[2]=1, clksys --> no, clktop --> no
        DARIC_CGU->cgusel0 = 0x01;
        DARIC_CGU->cguset = 0x32; // enable config
        __DSB();
        DARIC_IPC->lpen = 0x0F; // pmu --> pmulpcfg
        __DSB();
        DARIC_IPC->ar = 0x32; // enable config
        __DSB();
        HAL_PMU_EnterLowPowerMode(&s_hpmu, PMU_MODE_DEEP_SLEEP);
        __DSB();
        DARIC_IPC->ar = 0x57; // enter lp flow
        __DSB();
        __ISB();
        break;

    case LOW_POWER_POWER_DOWN:
        HAL_RTC_ClearFlag();
        HAL_RTC_IT_Enable(0);
        if (get_update_pd_mode_window_state())
        {
            printf("=====>LowPower_User_Enter(LOW_POWER_POWER_DOWN) update pd window\r\n");
            extern VOID tx_low_power_exit(VOID);
            tx_low_power_exit();
            extern void bsp_pm_send_update_pd_window_event();
            bsp_pm_send_update_pd_window_event();
            set_update_pd_mode_window_state(false);
            return 0;
        }
        printf("=====>Enter Power Down Mode\r\n");
        // /* The purpose of adding NOP instructions is to avoid incomplete log output issues. */
        // for (int i = 0; i < 500000; i++)
        // {
        //   __asm volatile ("nop");
        // }
#ifdef CONFIG_BOARD_ACTIVECARD_NTO_DVT2
        HAL_AO_BootModeStruct boot_mode;
        boot_mode.SleepMode = HAL_AO_BOOT_FAST_MODE;
        boot_mode.BootMode = HAL_AO_BOOT_FAST_MODE;
        HAL_AO_BootInfo_Set(HAL_AO_BOOT_INFO_TYPE_BOOTMODE, (void *)&boot_mode);
        HAL_AO_BootModeStruct type;
        HAL_AO_BootInfo_Get(HAL_AO_BOOT_INFO_TYPE_BOOTMODE, &type);
        printf("Power Down Mode AO fastboot flag: %d, boot mode: %d\r\n", type.SleepMode, type.BootMode);
#else
        HAL_AO_BootModeTypeDef boot_mode = HAL_AO_BOOT_FAST_MODE;
        HAL_RERAM_Write(0x603E2A20, (uint8_t *)&boot_mode, sizeof(boot_mode));
        HAL_AO_BootModeTypeDef type;
        HAL_RERAM_Read(0x603E2A20, (uint8_t *)&type, sizeof(type));
        printf("Power Down Mode fastboot flag: %d\r\n", type);
#endif
        #ifdef CONFIG_SUPPORT_DEEPSLEEP
        aw2023_led_off();
        aw2023_enter_lowpower();
        #ifdef CONFIG_PD_RTC_ALARM_TO_POWEROFF
        BSP_RTC_SetAlarmDelayMinutes(1);
        #endif
        #endif
        TG28_BATFET_en(false);
        TG28_POK_Config(true, 3, 0);
        for (int i = 0; i < 4000000; i++)//400MHZ:10 ms
        {
          __asm volatile ("nop");
        }
        TG28_Soft_Power_Off();
        // HAL_PMU_EnterLowPowerMode(&s_hpmu, PMU_MODE_POWER_DOWN);
        break;

    default:
        break;
    }
    return 0;
}

/**
 * @brief Exit from low-power mode and restore system operation
 * @param param Pointer to the low-power state being exited (lp_state type)
 * @return 0 on success, -1 if an invalid mode is specified
 * 
 * @note Common restoration steps for deep sleep modes:
 *        - Disables RTC interrupt (no longer needed after wakeup)
 *        - Clears RTC flags to prevent spurious interrupts
 *        - Restores full system clock frequency (400MHz)
 *        - Clears TG28 interrupt sources
 * @note For power-down mode:
 *        - Only performs RTC cleanup (system was completely powered down)
 * @note Universal restoration steps:
 *        - Exits PMU low-power mode
 *        - Disables deep sleep mode in SCB
 *        - Re-enables SysTick interrupt for RTOS operation
 *       Clock restoration is critical for returning to normal performance levels
 *       Interrupt re-enable ensures proper RTOS scheduling after wakeup
 * 
 * This function handles the hardware-specific restoration sequence when exiting
 * from different low-power states. It reconfigures system clocks, clears wakeup
 * sources, and restores normal operating conditions.
 */
UINT LowPower_User_Exit(VOID *param)
{
    lp_state mode = *(lp_state *)param;

    if (mode > LOW_POWER_POWER_DOWN)
    {
        return -1;
    }

    switch (mode)
    {
    case LOW_POWER_SLEEP:
        break;

    case LOW_POWER_DEEP_SLEEP_FD:
        HAL_RTC_IT_Enable(0);
        HAL_RTC_ClearFlag();
        HAL_ClockConfig(HAL_CPU_FREQSEL_700MHZ);
        /* Avoid the int31 interrupt issue and clear invalid int31 */
        clearTG28Irq();
        break;

    case LOW_POWER_DEEP_SLEEP_IP:
        HAL_RTC_IT_Enable(0);
        HAL_RTC_ClearFlag();
        HAL_ClockConfig(HAL_CPU_FREQSEL_700MHZ);
        /* Avoid the int31 interrupt issue and clear invalid int31 */
        clearTG28Irq();
        break;

    case LOW_POWER_DEEP_SLEEP_DEEP:
        HAL_RTC_IT_Enable(0);
        HAL_RTC_ClearFlag();
        HAL_ClockConfig(HAL_CPU_FREQSEL_700MHZ);
        /* Avoid the int31 interrupt issue and clear invalid int31 */
        clearTG28Irq();
        break;

    case LOW_POWER_POWER_DOWN:
        HAL_RTC_IT_Enable(0);
        HAL_RTC_ClearFlag();
        if (get_update_pd_mode_window_state())
        {
            printf("=====>LowPower_User_Exit(LOW_POWER_POWER_DOWN) update clock\r\n");
            HAL_ClockConfig(HAL_CPU_FREQSEL_700MHZ);
            /* Avoid the int31 interrupt issue and clear invalid int31 */
            clearTG28Irq();
        }
        break;

    default:
        break;
    }

    /* The watchdog should be re-enable after waking up from sleep. */
    HAL_WDG_Enable(WATCHDOG_TIMEOUT_MS, true, true);
    HAL_PMU_ExitLowPowerMode(&s_hpmu);
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    return 0;
}

/**
 * @brief Set up low-power timer for system wakeup
 * @param ms Pointer to the wakeup time interval in milliseconds
 * @return 0 always indicates successful operation
 * 
 * @note Configures the RTC hardware to generate an interrupt after *ms milliseconds
 *       Enables the RTC interrupt to ensure wakeup capability
 *       This timer is typically used for timed wakeups from deep sleep states
 *       The RTC continues running during low-power states for reliable wakeup
 *       Should be called before entering deep sleep modes that require timed wakeup
 * 
 * This function configures the low-power timer (RTC) to generate a wakeup
 * interrupt after the specified time interval. The timer is used to wake the
 * system from deep sleep states when no other wakeup sources are active.
 */
static UINT LowPower_Timer_Setup(ULONG *ms)
{
    HAL_RTC_SetTimer(*ms);
    HAL_RTC_IT_Enable(1);
    return 0;
}

/**
 * @brief Get the duration from low-power timer (RTC)
 * @param param Unused parameter, maintained for interface consistency
 * @return Elapsed time in milliseconds since last timer reference point
 * 
 * This function retrieves the elapsed time measured by the low-power timer (RTC)
 * since the last wakeup or timer start. The duration is typically used for time
 * compensation and system tick recovery after waking from low-power states.
 */
static ULONG LowPower_Timer_Get(VOID *param)
{
    return (ULONG)HAL_RTC_GetDuration();
}

/**
 * @brief Initialize low-power management system
 * @param None
 * @return None
 * 
 * @note Initialization sequence:
 *       1. RTC initialization and start
 *       2. Always-On domain configuration:
 *          - Clock selection (32K external with isolation and PCLK enabled)
 *          - Clock division (1KHz output)
 *          - Oscillator configuration (32K power-down, deep sleep, and active modes)
 *          - GPIO pad configuration (pull-up and alternate function settings)
 *          - Interrupt and reset mask configuration
 *       3. PMU initialization:
 *          - Power mode (active)
 *          - LDO configuration for core and analog supplies
 *          - Low-power and power-down configurations
 *       4. System boot mode detection and reporting
 *       5. Systick pending interrupt clearance
 * 
 * This function performs the complete initialization of the low-power management
 * subsystem, including RTC, Always-On (AO) domain, Power Management Unit (PMU),
 * and registration of low-power callback functions.
 */
void LowerPower_Init(void)
{
    HAL_RTC_ClearFlag();
    if (!HAL_RTC_GetState())
    {
        HAL_RTC_Start();
    }

    /* AO config */
    // Clock config
    s_hao.Init.ClockSelection =
        AO_CLOCK_32K_EXT | AO_CLOCK_ISO_EN | AO_CLOCK_PCLK_EN;
    s_hao.Init.ClockDivision = AO_CLOCK_DIVISION_1KHZ;
    s_hao.Init.Oscillator =
        AO_OSC_PD_32K_EN | AO_OSC_DS_32K_EN | AO_OSC_ACT_32K_EN;
    // IO PAD config
    // s_hao.Init.PullUpConfig = AO_GPIOF_0_PU | AO_GPIOF_1_PU;
    // s_hao.Init.AltFunction = AO_GPIOF_AF0_GPIO;
    s_hao.Init.PullUpConfig = 0;
    s_hao.Init.AltFunction = AO_GPIOF_AF0_GPIO;
    // Intterupt config & Reset config
    s_hao.Init.InterruptMask = 0;
    s_hao.Init.ResetMask = AO_RSTCR_MASK_POR;
    s_hao.Instance = AON;
    s_hao.MspInitCallback = NULL;
    s_hao.MspDeInitCallback = NULL;
    HAL_AO_Init(&s_hao);
    HAL_AO_RegisterCallback(0, AO_WKUP_NOT_PF0_PD | AO_WKUP_NOT_PF1_PD | AO_WKUP_NOT_RTC_EVENT, LowPower_Timer_IrqCallback, NULL);

    // PMU config
    s_hpmu.Instance = PMU;
    s_hpmu.Init.PowerMode = PMU_MODE_ACTIVE,
    s_hpmu.Init.LDO_Config = PMU_LDO_VDD85D | PMU_LDO_VDD85A | PMU_LDO_VDD25,
    s_hpmu.Init.LowPowerConfig = PMU_LDO_VDD85D | PMU_LDO_VDD85A | PMU_LDO_VDD25,
    s_hpmu.Init.PowerDownConfig = 0, s_hpmu.State = HAL_PMU_STATE_RESET;
    s_hpmu.MspInitCallback = NULL;
    s_hpmu.MspDeInitCallback = NULL;
    HAL_PMU_Init(&s_hpmu);

    // HAL_AO_BootModeStruct boot_mode;
    // HAL_AO_BootInfo_Get(HAL_AO_BOOT_INFO_TYPE_BOOTMODE, (void *)&boot_mode);
    // printf("Boot by %s.\n",
    //        (boot_mode.SleepMode == HAL_AO_BOOT_FAST_MODE) ? "fast" : "normal");

    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk; // clear systick pending
    low_power_register_process(0, LowPower_User_Enter, NULL, LowPower_User_Exit,
                               NULL);
    low_power_register_timer_funcs(LowPower_Timer_Setup, LowPower_Timer_Get);
    low_power_set_transition_ms(LIGHT_SLEEP_TIME_MS, DEEPIP_SLEEP_TIME_MS);
}

/**
 * @brief Control D Flip-Flop (DFF) to switch V33_AO power
 * @param on Control signal for V33_AO power
 *        - true: Enable V33_AO power by setting DFF output Q
 *        - false: Disable V33_AO power by clearing DFF output Q
 * @return None
 *
 * @note GPIO_PIN_7 connects to DFF clock input (C)
 * @note GPIO_PIN_8 connects to DFF data input (D)
 *
 * This function controls a D Flip-Flop circuit using GPIO pins PF7 and PF8
 * to drive the clock (C) and data (D) inputs respectively, generating a
 * latched output signal Q that controls the V33_AO power.
 * The function implements the following sequence:
 * 1. Initializes PF7 and PF8 as output pins with no pull-up/down
 * 2. Enables AON domain pull-up on both pins for signal stability
 * 3. Sets initial state: C=0, D=0
 * 4. Waits 1ms for signal stabilization
 * 5. Sets D pin according to the 'on' parameter (1=SET, 0=RESET)
 * 6. Waits 1ms for data signal setup time
 * 7. Triggers clock (C=1) to latch the data value to output Q
 * 8. Waits 1ms for output stabilization
 * 9. Sets D=1 as default safe state
 * 10. Re-enables pull-ups and releases GPIO control
 */
void switchDffV33AO(bool on)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.IsrHandler = NULL;
    GPIO_InitStruct.UserData = NULL;

    //GPIO_PIN_7 is DFF C pin
    //GPIO_PIN_8 is DFF D pin
    AON->PADPU |= AO_GPIOF_7_PU | AO_GPIOF_8_PU; //pullup PF7, PF8
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);    
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);
    __DSB();
    tx_thread_sleep(1); //1ms
    if (on)
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_SET);  //pullup
    }
    else
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_RESET);
    }
    __DSB();
    tx_thread_sleep(1); //1ms
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_7, GPIO_PIN_SET); //pullup C, output Q
    __DSB();
    tx_thread_sleep(1); //1ms
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, GPIO_PIN_SET); //pullup D
    tx_thread_sleep(1);
    AON->PADPU |= AO_GPIOF_7_PU | AO_GPIOF_8_PU; //pullup PF7, PF8
    __DSB();
    HAL_GPIO_DeInit(GPIOF, GPIO_InitStruct.Pin);
    __DSB();

}
#else
void LowerPower_Init(void) {}
#endif
