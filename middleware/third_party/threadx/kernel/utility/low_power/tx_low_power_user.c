/**
 ******************************************************************************
 * @file    tx_low_power_user.c
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

/* Include necessary system files.  */
#include "tx_low_power_user.h"
#include "daric.h"
#include "system_daric.h"
#include "daric_hal_rtc.h"
#include "daric_hal_ao.h"
#include "daric_hal_nvic.h"
#include "daric_hal_wdg.h"
#include "tx_work_item_queue.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Private macro -------------------------------------------------------------*/
/* Number of different device_ids */
#define MAX_DEVICE_HOOKS    16
/* Maximum number of concurrent wakelocks */
#define MAX_WAKELOCKS       16
#define WAKELOCK_NAME_LEN   32

/* 
 * Default layer threshold (ms), 
 * adjustable via low_power_set_transition_ms 
 */
#define TRANSITION_LIMITED_MS   (1)
#define DEFAULT_LIGHT_TO_IP_MS  (5 * 1000UL)
#define DEFAULT_FD_TO_IP_MS     (5 * 1000UL)
#define DEFAULT_IP_TO_PD_MS   (15 * 1000UL)
#define DEFAULT_DEEP_TO_PD_MS   (100 * 1000UL)

/*
 * During system sleep, the SysTick interrupt has been disabled. 
 * Therefore, after the system is awakened, 
 * the timing information from the RTC must be used to compensate 
 * for the SysTick timing information in the RTOS.
 */
extern VOID tx_time_increment(ULONG time_increment);

extern VOID _tx_timer_interrupt(VOID);

/* Device hook record structure */
typedef struct
{
    bool used;
    UINT device_id;
    LP_HOOK_FUN sleep_hook;
    VOID *sleep_param;
    LP_HOOK_FUN wakeup_hook;
    VOID *wakeup_param;
    LP_PROCESS_FUN sleep_process;
    LP_PROCESS_FUN wakeup_process;
} device_hook_t;

/* wakelock bookkeeping wakelock */
typedef struct
{
    UINT count;
    CHAR names[MAX_WAKELOCKS][WAKELOCK_NAME_LEN];
} wakelock_table_t;

static lp_state g_lp_state = LOW_POWER_SLEEP;
static device_hook_t device_hooks[MAX_DEVICE_HOOKS] = {0};
static wakelock_table_t g_wakelocks = {0};
static LP_PROCESS_FUN g_sleep_process = NULL;
static LP_PROCESS_FUN g_wakeup_process = NULL;

static bool g_update_pd_window = true;

/* Low-power timer callback (implementation by the platform)
 * Input in ThreadX ticks (or implementation-converted)
 */
static UINT (*g_lp_timer_setup_ms)(ULONG *ms) = NULL;

/*
 * Returns the actual sleep duration in milliseconds.
 * Returns 0 if tick compensation has already been applied
 */
static ULONG (*g_lp_timer_adjust_ms)(VOID *param) = NULL;

/* Deep-sleep layer threshold (ms) */
static ULONG g_light_to_ip_ms = DEFAULT_LIGHT_TO_IP_MS;
// static ULONG g_fd_to_ip_ms = DEFAULT_FD_TO_IP_MS;
static ULONG g_ip_to_pd_ms = DEFAULT_IP_TO_PD_MS;
// static ULONG g_deep_to_pd_ms = DEFAULT_DEEP_TO_PD_MS;

/**
 * @brief Convert ThreadX timer ticks to milliseconds
 * @param ticks Time interval in ThreadX timer ticks
 * @return Time interval in milliseconds (rounded down)
 * 
 * @note The result is rounded down to the nearest millisecond
 * @note Uses integer arithmetic to avoid floating point operations
 * @note Conversion formula: ms = (ticks * 1000) / TX_TIMER_TICKS_PER_SECOND
 * 
 * This function converts a time interval from ThreadX timer ticks to milliseconds.
 * The conversion is done by scaling the ticks and dividing by the system ticks per second.
 */
static inline ULONG _ticks_to_ms(ULONG ticks)
{
    return (ticks * 1000UL) / TX_TIMER_TICKS_PER_SECOND;
}

/**
 * @brief Convert milliseconds to ThreadX timer ticks with ceiling rounding
 * @param ms Time interval in milliseconds
 * @return Time interval in ThreadX timer ticks (rounded up)
 * 
 * @note The result is rounded up to ensure minimum time requirement is met
 * @note Uses integer arithmetic to avoid floating point operations
 * @note Conversion formula: ticks = (ms * TX_TIMER_TICKS_PER_SECOND + 999) / 1000
 * @note The +999 term ensures ceiling rounding behavior
 * 
 * This function converts a time interval from milliseconds to ThreadX timer ticks.
 * The conversion uses ceiling rounding to ensure the resulting ticks are at least
 * the duration specified in milliseconds.
 */
static inline ULONG _ms_to_ticks_ceil(ULONG ms)
{
    return (ms * TX_TIMER_TICKS_PER_SECOND + 999UL) / 1000UL;
}

/**
 * @brief Set the current low-power state
 * @param st The low-power state to set
 * 
 * @note This function modifies the global variable g_lp_state
 * @note The state change affects system power management behavior
 * 
 * This function updates the global low-power state variable with the specified value.
 * It is used to control and track the system's current power management state.
 */
static void _set_lp_state(lp_state st)
{
    g_lp_state = st;
}

/**
 * @brief Get the current low-power state
 * @param None
 * 
 * @return The current low-power state
 * 
 * @note This function provides read-only access to g_lp_state
 * @note The returned state reflects the system's current power management status
 * 
 * This function returns the current value of the global low-power state variable.
 * It is used to query the system's current power management state.
 */
lp_state _get_lp_state(VOID)
{
    return g_lp_state;
}

/**
 * @brief Execute all registered sleep hooks for low-power transition
 * @param None
 * @return None
 * 
 * @note The function will skip unused hooks and hooks without registered sleep functions
 * @note Each hook receives a parameter structure containing:
 *       - Current low-power state
 *       - Device-specific user parameter
 * @note The iteration stops after processing MAX_DEVICE_HOOKS entries
 * 
 * This function iterates through the registered device hooks and invokes 
 * their sleep hook functions to prepare devices for low-power state transition.
 * It passes the current low-power state and device-specific parameters to each hook.
 */
void _exec_sleep_hooks(void)
{
    lp_func_param_t sleep_param = {0};

    sleep_param.state = _get_lp_state();
    for (int i = 0; i < MAX_DEVICE_HOOKS; ++i)
    {
        if (device_hooks[i].used && device_hooks[i].sleep_hook)
        {
            sleep_param.user_param = device_hooks[i].sleep_param;
            device_hooks[i].sleep_hook(&sleep_param);
        }
    }
}

/**
 * @brief Execute all registered wakeup hooks for system resume
 * @param None
 * @return None
 * 
 * @note The function will skip unused hooks and hooks without registered wakeup functions
 *       Each hook receives a parameter structure containing:
 *       - Previous low-power state
 *       - Device-specific user parameter
 *       The iteration stops after processing MAX_DEVICE_HOOKS entries
 *       This function is typically called after system wakeup to restore device states
 * 
 * This function iterates through the registered device hooks and invokes 
 * their wakeup hook functions to restore devices to normal operation after
 * exiting low-power state. It passes the previous low-power state and 
 * device-specific parameters to each hook.
 */
void _exec_wakeup_hooks(void)
{
    lp_func_param_t wakeup_param = {0};

    wakeup_param.state = _get_lp_state();
    for (int i = 0; i < MAX_DEVICE_HOOKS; ++i)
    {
        if (device_hooks[i].used && device_hooks[i].wakeup_hook)
        {
            wakeup_param.user_param = device_hooks[i].wakeup_param;
            device_hooks[i].wakeup_hook(&wakeup_param);
        }
    }
}

/**
 * @brief  Register sleep/wakeup hook callbacks for a specific device.
 * @note   Each device can register its own pre-sleep and post-wakeup handlers.
 *         When entering or exiting low power mode, all registered hooks will be
 *         called in order.
 * @param  device_id        Unique identifier for the device.
 * @param  sleep_hook_fun   Function pointer called before sleep (can be NULL).
 * @param  sleep_param_ptr  Parameter passed to the sleep hook.
 * @param  wakeup_hook_fun  Function pointer called after wakeup (can be NULL).
 * @param  wakeup_param_ptr Parameter passed to the wakeup hook.
 * @retval TX_SUCCESS       Registration succeeded.
 * @retval TX_PTR_ERROR     Both sleep and wakeup hooks are NULL.
 * @retval TX_NO_MEMORY     No available hook entry slot.
 */
UINT low_power_register_hook(UINT device_id, LP_HOOK_FUN sleep_hook_fun,
                             VOID *sleep_param_ptr, LP_HOOK_FUN wakeup_hook_fun,
                             VOID *wakeup_param_ptr)
{
    TX_INTERRUPT_SAVE_AREA
    UINT status = TX_SUCCESS;

    if (sleep_hook_fun == NULL && wakeup_hook_fun == NULL)
    {
        return TX_PTR_ERROR;
    }

    TX_DISABLE;

    /* Attempts to find an existing device_id entry for replacement */
    for (int i = 0; i < MAX_DEVICE_HOOKS; ++i)
    {
        if (device_hooks[i].used && device_hooks[i].device_id == device_id)
        {
            if (sleep_hook_fun)
            {
                device_hooks[i].sleep_hook = sleep_hook_fun;
                device_hooks[i].sleep_param = sleep_param_ptr;
            }
            if (wakeup_hook_fun)
            {
                device_hooks[i].wakeup_hook = wakeup_hook_fun;
                device_hooks[i].wakeup_param = wakeup_param_ptr;
            }
            TX_RESTORE;
            return TX_SUCCESS;
        }
    }

    /* Otherwise, find an available slot. */
    for (int i = 0; i < MAX_DEVICE_HOOKS; ++i)
    {
        if (!device_hooks[i].used)
        {
            device_hooks[i].used = true;
            device_hooks[i].device_id = device_id;
            device_hooks[i].sleep_hook = sleep_hook_fun;
            device_hooks[i].sleep_param = sleep_param_ptr;
            device_hooks[i].wakeup_hook = wakeup_hook_fun;
            device_hooks[i].wakeup_param = wakeup_param_ptr;
            device_hooks[i].sleep_process = NULL;
            device_hooks[i].wakeup_process = NULL;
            TX_RESTORE;
            return TX_SUCCESS;
        }
    }

    /* No available slot, returns an error */
    status = TX_NO_MEMORY;
    TX_RESTORE;
    return status;
}

/**
 * @brief Unregister a device hook by device ID
 * @param device_id The device ID of the hook to unregister
 * @return TX_SUCCESS if the hook was successfully found and removed
 *         TX_DELETE_ERROR if no matching device ID was found
 * 
 * @note This function operates with interrupts disabled for thread safety
 *       The entire device_hook_t structure is zeroed out upon successful removal
 *       Only the first matching device ID is removed if multiple entries exist
 *       The search stops at the first match within MAX_DEVICE_HOOKS entries
 * 
 * This function searches for and removes a registered device hook based on the 
 * specified device ID. The operation is performed with interrupts disabled to 
 * ensure thread-safe access to the device hooks array.
 */
UINT low_power_unregister_hook(UINT device_id)
{
    TX_INTERRUPT_SAVE_AREA
    UINT status = TX_DELETE_ERROR;

    TX_DISABLE;
    for (int i = 0; i < MAX_DEVICE_HOOKS; ++i)
    {
        if (device_hooks[i].used && device_hooks[i].device_id == device_id)
        {
            memset(&device_hooks[i], 0, sizeof(device_hook_t));
            status = TX_SUCCESS;
            break;
        }
    }
    TX_RESTORE;
    return status;
}

/**
 * @brief Acquire a wake lock with the specified name
 * @param lock_name Name of the wake lock to acquire
 * @return None
 * 
 * @note This function is thread-safe and operates with interrupts disabled
 *       If lock_name is NULL, the function returns without any action
 *       If the wake lock already exists, no action is taken (idempotent)
 *       Wake lock names are truncated to WAKELOCK_NAME_LEN - 1 characters
 *       If MAX_WAKELOCKS is reached, the operation is ignored (no error triggered)
 * 
 * This function adds a wake lock to prevent the system from entering low-power states.
 * If the lock already exists, the function returns immediately. If the maximum number
 * of wake locks is reached, the operation is silently ignored to ensure system continuity.
 */
VOID low_power_wake_lock(CHAR *lock_name)
{
    if (lock_name == NULL)
        return;

    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;

    /* If it already exists, return immediately */
    for (UINT i = 0; i < g_wakelocks.count; ++i)
    {
        if (strncmp(g_wakelocks.names[i], lock_name, WAKELOCK_NAME_LEN) == 0)
        {
            TX_RESTORE;
            return;
        }
    }

    /* Insert a new name */
    if (g_wakelocks.count < MAX_WAKELOCKS)
    {
        strncpy(g_wakelocks.names[g_wakelocks.count], lock_name,
                WAKELOCK_NAME_LEN - 1);
        g_wakelocks.names[g_wakelocks.count][WAKELOCK_NAME_LEN - 1] = '\0';
        g_wakelocks.count++;
    }
    else
    {
        /* If the upper limit is reached, the action is either ignored or an error is logged.
         * This design ensures continued operation by avoiding assertion triggers.
         */
    }

    TX_RESTORE;
}

/**
 * @brief Release a wake lock with the specified name
 * @param lock_name Name of the wake lock to release
 * @return None
 * 
 * @note This function is thread-safe and operates with interrupts disabled
 *       The last entry in the array is cleared after shifting
 *       The wake lock count is decremented upon successful removal
 *       If the wake lock is not found, no action is taken
 * 
 * This function removes a wake lock, allowing the system to enter low-power states
 * if no other wake locks are active. The function searches for the specified lock
 * by name and removes it from the wake locks array.
 */
VOID low_power_wake_unlock(CHAR *lock_name)
{
    if (lock_name == NULL)
        return;

    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;

    for (UINT i = 0; i < g_wakelocks.count; ++i)
    {
        if (strncmp(g_wakelocks.names[i], lock_name, WAKELOCK_NAME_LEN) == 0)
        {
            /* Remove the lock,
             * and shift the subsequent elements left to fill the gap.
             */
            for (UINT j = i; j + 1 < g_wakelocks.count; ++j)
            {
                strncpy(g_wakelocks.names[j], g_wakelocks.names[j + 1],
                        WAKELOCK_NAME_LEN);
            }
            /* Clear the last entry */
            memset(g_wakelocks.names[g_wakelocks.count - 1], 0, WAKELOCK_NAME_LEN);
            g_wakelocks.count--;
            break;
        }
    }

    TX_RESTORE;
}

/**
 * @brief Check if any wake locks are currently active
 * @param None
 * @return true if one or more wake locks are active
 *         false if no wake locks are active
 * 
 * @note This function is thread-safe and operates with interrupts disabled
 *       The check is based on the wake lock count (g_wakelocks.count > 0)
 *       A true return value prevents system from entering low-power states
 *       A false return value indicates system can safely enter low-power states
 * 
 * This function checks the current state of wake locks and returns whether
 * any wake locks are currently held. A return value of true indicates that
 * the system should avoid entering low-power states due to active wake locks.
 */
bool low_power_if_locked(VOID)
{
    bool locked;
    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;
    locked = (g_wakelocks.count > 0);
    TX_RESTORE;
    return locked;
}

/**
 * @brief Checks if any wake lock, other than a specified one, is currently active.
 *
 * @param[in] lock_name Pointer to a null-terminated string specifying the name
 *                      of a wake lock to be *excluded* from the check. This is
 *                      often the name of the lock held by the caller itself.
 *                      If this is the only active lock, the function returns
 *                      `false`, allowing the caller to proceed.
 *
 * @retval true  The system's low-power mode is locked. This means either:
 *               - Multiple wake locks are active, OR
 *               - A single wake lock is active whose name does NOT match `lock_name`.
 * @retval false The system's low-power mode is NOT locked, and may proceed.
 *               This means either:
 *               - No wake locks are active (`g_wakelocks.count <= 0`), OR
 *               - Only one wake lock is active, and its name matches `lock_name`.
 *
 * @note
 * - **Thread Safety & Critical Section:** This function disables interrupts
 *   (`TX_DISABLE`) for atomic access to the shared `g_wakelocks` structure.
 *   The critical section is essential in a preemptive, multi-threaded environment.
 * - **Name Comparison:** The comparison uses `strncmp` with `WAKELOCK_NAME_LEN`,
 *   ensuring bounds-safe comparison. It assumes names are stored consistently.
 */
bool low_power_has_locked(CHAR *lock_name)
{
    bool locked = false;
    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;
    if (g_wakelocks.count <= 0)
    {
        locked = false;
    }
    else if (g_wakelocks.count == 1
        && strncmp(g_wakelocks.names[0], lock_name, WAKELOCK_NAME_LEN) == 0)
    {
        locked = false;
    }
    else
    {
        locked = true;
    }
    TX_RESTORE;
    return locked;
}

/**
 * @brief  Configure thresholds (ms) for progressive deep sleep levels.
 * @param  light_to_ip_ms:   Transition threshold from light to IP.
 * @param  ip_to_dp_ms: Transition threshold from IP to POWER_DOWN.
 * @retval None
 * 
 * @note   The system transitions deeper based on accumulated idle time:
 *         - light → IP after light_to_ip_ms
 *         - IP → POWER_DOWN after ip_to_pd_ms
 * 
 * This function configures the minimum duration thresholds (in milliseconds) required 
 * for transitions between different low-power states. Each threshold must meet or 
 * exceed the minimum limited value defined by TRANSITION_LIMITED_MS.
 */
VOID low_power_set_transition_ms(ULONG light_to_ip_ms, ULONG ip_to_dp_ms)
{
    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;
    if (light_to_ip_ms >= TRANSITION_LIMITED_MS)
        g_light_to_ip_ms = light_to_ip_ms;
    if (ip_to_dp_ms >= TRANSITION_LIMITED_MS)
        g_ip_to_pd_ms = ip_to_dp_ms;
    // if (deep_to_pd_ms >= TRANSITION_LIMITED_MS)
    //     g_deep_to_pd_ms = deep_to_pd_ms;
    TX_RESTORE;
}

/**
 * @brief Notify the system of the wakeup source after resuming from low-power state
 * @param wakeup_from_lp_timer Indicates if wakeup was triggered by low-power timer
 *        - true: Wakeup was caused by low-power timer
 *        - false: Wakeup was caused by other sources (e.g., external interrupt)
 * @return None
 * 
 * @note This function is thread-safe and operates with interrupts disabled
 *       Always disables and clears the AOWKUPINT_IRQn to prevent interrupt issues
 *       The wakeup_from_lp_timer parameter is currently reserved for future use
 *       Critical for ensuring stable system operation after low-power resume
 *       Must be called early in the wakeup process before re-enabling interrupts
 * 
 * This function handles system wakeup notification and performs necessary hardware
 * cleanup operations. It specifically addresses interrupt handling issues by
 * disabling and clearing the AON wakeup interrupt to prevent spurious interrupts.
 */
VOID low_power_notify_wakeup_source(bool wakeup_from_lp_timer)
{
    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;
    printf("low_power_notify_wakeup_source\r\n");
    /* Avoid the int31 interrupt issue and clear invalid int31 */
    HAL_NVIC_DisableIRQ(AOWKUPINT_IRQn);
    NVIC_ClearPendingIRQ(AOWKUPINT_IRQn);

    if (wakeup_from_lp_timer)
    {
    }
    TX_RESTORE;
}

/**
 * @brief  Register global sleep/wakeup process callbacks.
 * @note   These callbacks are called once globally when the system enters or
 *         exits low power state (in addition to device-specific hooks).
 * @param  device_id            Ignored (kept for compatibility).
 * @param  sleep_process_fun    Called before CPU enters WFI (optional).
 * @param  sleep_param_ptr      Parameter passed to sleep process (unused).
 * @param  wakeup_process_fun   Called after wakeup (optional).
 * @param  wakeup_param_ptr     Parameter passed to wakeup process (unused).
 * @retval TX_SUCCESS           Registered successfully.
 * @retval TX_PTR_ERROR         Both callbacks are NULL.
 */
UINT low_power_register_process(UINT device_id,
                                LP_PROCESS_FUN sleep_process_fun,
                                VOID *sleep_param_ptr,
                                LP_PROCESS_FUN wakeup_process_fun,
                                VOID *wakeup_param_ptr)
{
    if (!sleep_process_fun && !wakeup_process_fun)
    {
        return TX_PTR_ERROR;
    }
    g_sleep_process = sleep_process_fun;
    g_wakeup_process = wakeup_process_fun;
    return TX_SUCCESS;
}

/**
 * @brief  Register platform-specific low power timer setup and adjust
 * callbacks.
 * @note   These functions are typically provided by the RTC or AON timer
 * driver.
 *         - setup_ms:  Configures the RTC/low-power timer to expire after 'ms'.
 *         - adjust_ms: Returns actual elapsed sleep time (in ms).
 * @param  setup_ms   Platform callback to start low power timer.
 * @param  adjust_ms  Platform callback to return actual elapsed time.
 * @retval TX_SUCCESS Always.
 */
UINT low_power_register_timer_funcs(UINT (*setup_ms)(ULONG *ticks),
                                    ULONG (*adjust_ms)(VOID *param))
{
    TX_INTERRUPT_SAVE_AREA
    TX_DISABLE;
    g_lp_timer_setup_ms = setup_ms;
    g_lp_timer_adjust_ms = adjust_ms;
    TX_RESTORE;
    return TX_SUCCESS;
}

/**
 * @brief  Configure platform low power timer for upcoming sleep.
 * @note   Called by tx_low_power.c before entering WFI.
 *         The registered setup callback is used to start the RTC timer.
 * @param  ticks: Desired sleep duration in ThreadX ticks.
 * @retval None
 *
 * During the system scheduler's processing of tx_low_power_enter,
 * it first calls low_power_timer_config,
 * then proceeds to low_power_user_enter to handle sleep configuration.
 * Since we have already enabled the low-power RTC for timing within low_power_user_enter,
 * these actions no longer need to be performed in low_power_timer_config.
 */
void low_power_timer_config(ULONG ticks)
{
    lp_state current_state = _get_lp_state();
    // ULONG ms = _ticks_to_ms(ticks);

    if ((g_lp_timer_setup_ms) && (current_state != LOW_POWER_SLEEP))
    {
        switch (current_state)
        {
            case LOW_POWER_SLEEP:
                break;

            case LOW_POWER_DEEP_SLEEP_FD:
                break;

            case LOW_POWER_DEEP_SLEEP_IP:
                break;

            case LOW_POWER_DEEP_SLEEP_DEEP:
                break;

            case LOW_POWER_POWER_DOWN:
                break;

            default:
                break;
        }
    }
}

/**
 * @brief  Adjust ThreadX system ticks based on actual low power duration.
 * @note   Called after wakeup. The platform callback returns actual
 *         elapsed sleep time (ms), which is converted back to ticks and
 *         added to ThreadX tick count.
 * @retval Always 0 (the adjustment is done internally).
 * 
 * During system sleep, the SysTick interrupt has been disabled. 
 * Therefore, after the system is awakened, 
 * the timing information from the RTC must be used to compensate 
 * for the SysTick timing information in the RTOS.
 * Returns the actual sleep duration in milliseconds.
 * Returns 0 if tick compensation has already been applied
 */
ULONG low_power_timer_adjust(void)
{
    lp_state current_state = _get_lp_state();
    ULONG actual_ticks = 0;
    ULONG elapsed_ms = 0;
    uint32_t isAoWkupInt = 0;
    uint32_t isRtcInt = 0;

    isAoWkupInt = NVIC_GetPendingIRQ(AOWKUPINT_IRQn);
    isRtcInt = HAL_RTC_GetRawInt();
    if ((g_lp_timer_adjust_ms) && (current_state != LOW_POWER_SLEEP))
    {
        elapsed_ms = g_lp_timer_adjust_ms(NULL);
        printf("=====>low_power_timer_adjust isAoWkupInt=%ld, isRtcInt=%ld, elapsed_ms= %ld\r\n", isAoWkupInt, isRtcInt,
               elapsed_ms);
        actual_ticks = _ms_to_ticks_ceil(elapsed_ms);
        tx_time_increment(actual_ticks);
        if (!(isAoWkupInt && isRtcInt))
        {
            _set_lp_state(LOW_POWER_SLEEP);
        }
        /*
         * The interrupt handler will wake up the specified thread.
         * The RTOS interface updates the information about the next thread to be scheduled. 
         * If there is a thread that needs to be scheduled, 
         * the system scheduler's __tx_ts_wait will execute this thread first 
         * and will not re-enter WFI. 
         * Therefore, there is no need to explicitly call the RTOS scheduler here.
         */
        //_tx_timer_interrupt();
    }
    return 0;
}

/**
 * @brief Enter user-defined low-power state based on current system conditions
 * @param None
 * @return TX_SUCCESS always indicates successful operation
 * 
 * @note   Called during ThreadX idle context.
 *         - If any wakelock is held, stays in LOW_POWER_SLEEP.
 *         - Otherwise, progressively enters deeper levels based on
 *           cumulative idle time.
 *       State progression: SLEEP → DEEP_SLEEP_IP → POWER_DOWN
 *       Idle time accumulation uses tick counting with rollover protection
 *       AOWKUPINT_IRQn is cleared to prevent spurious interrupt issues
 *       Sleep hooks are executed before entering low-power state
 *       Actual WFI execution occurs in system scheduler's __tx_ts_wait
 * 
 * This function implements the main logic for transitioning between low-power states.
 * It determines the appropriate next power state based on wake locks, idle time, 
 * and current state, then executes sleep hooks and sleep configuration processes.
 */
UINT low_power_user_enter(VOID)
{
    ULONG current_ticks = tx_time_get();
    lp_state next_state = LOW_POWER_SLEEP;
    lp_state current_state = _get_lp_state();
    static ULONG idle_time = 0;
    static ULONG last_tick = 0;
    ULONG delta_ticks;

    /* Avoid the int31 interrupt issue and clear invalid int31 */
    NVIC_ClearPendingIRQ(AOWKUPINT_IRQn);
    /*
     * When wakelocks are held,
     * the system enters the lightest LOW_POWER_SLEEP mode using WFI
     */
    if (low_power_if_locked())
    {
        next_state = LOW_POWER_SLEEP;
    }
    else
    {
        switch (current_state)
        {
        case LOW_POWER_SLEEP:
            if (current_ticks >= last_tick)
            {
                delta_ticks = current_ticks - last_tick;
            }
            else
            {
                delta_ticks = (ULONG) ((0xFFFFFFFFUL - last_tick) + current_ticks + 1UL);
            }
            if (delta_ticks == 1)
            {
                idle_time++;
            }
            else
            {
                idle_time = 0;
            }
            HAL_WDG_FeedDog();
            /* Avoid the int31 interrupt issue and clear invalid int31 */
            HAL_RTC_ClearFlag();
            HAL_NVIC_EnableIRQ(AOWKUPINT_IRQn);
            last_tick = current_ticks;
            if (idle_time >= g_light_to_ip_ms)
            {
                idle_time = 0;
                //next_state = LOW_POWER_DEEP_SLEEP_FD;
                next_state = LOW_POWER_DEEP_SLEEP_IP;
                printf("=====>next_state = LOW_POWER_DEEP_SLEEP_IP, g_light_to_ip_ms = %ld, g_ip_to_pd_ms = %ld\r\n", g_light_to_ip_ms, g_ip_to_pd_ms);
                g_lp_timer_setup_ms(&g_ip_to_pd_ms);
            }
            break;

        case LOW_POWER_DEEP_SLEEP_FD:
            next_state = LOW_POWER_DEEP_SLEEP_IP;
            // g_lp_timer_setup_ms(&g_ip_to_deep_ms);
            break;

        case LOW_POWER_DEEP_SLEEP_IP:
            //next_state = LOW_POWER_DEEP_SLEEP_DEEP;
            next_state = LOW_POWER_POWER_DOWN;
            //g_lp_timer_setup_ms(&g_deep_to_pd_ms);
            break;

        case LOW_POWER_DEEP_SLEEP_DEEP:
            next_state = LOW_POWER_POWER_DOWN;
            break;

        case LOW_POWER_POWER_DOWN:
            break;

        default:
            next_state = LOW_POWER_SLEEP;
            break;
        }
    }
    _set_lp_state(next_state);
    /* Invoke all registered sleep hooks to configure devices for low-power entry */
    //_exec_sleep_hooks();

    /*
     * Process the configuration for entering each sleep level.
     * Within the registered process, configure the corresponding registers,
     * enable the ARM deep sleep bit, and note that the WFI instruction is invoked
     * by the system scheduler's __tx_ts_wait function.
     */
    if (g_sleep_process)
    {
        g_sleep_process(&next_state);
    }
    else
    {
        switch (next_state)
        {
        case LOW_POWER_SLEEP:
            break;
        case LOW_POWER_DEEP_SLEEP_FD:
        case LOW_POWER_DEEP_SLEEP_IP:
        case LOW_POWER_DEEP_SLEEP_DEEP:
        case LOW_POWER_POWER_DOWN:
            break;
        default:
            break;
        }
    }

    return TX_SUCCESS;
}

void tempTaskWork(void *obj)
{
    while (1)
    {
        int tmpVal;

        // tx_thread_sleep(1000);
        for (int i = 0; i < 500000; i++)
        {
            tmpVal = 23.5 * i;
        }
        printf("tx_thread_sleep tmpVal=%d\r\n", tmpVal);
    }
}

/**
 * @brief Exit from user-defined low-power state and restore system operation
 * @param None
 * @return None
 * 
 * @note If current state is LOW_POWER_SLEEP, returns immediately (shallow sleep exit)
 *       Checks both AON wakeup interrupt and RTC interrupt status to determine wakeup source
 *       When both interrupts are active, performs minimal system validation
 *       When interrupts don't match expected pattern, performs full wakeup sequence:
 *        - Enables RTC interrupt
 *        - Clears RTC flags
 *        - Disables AON wakeup interrupt
 *        - Calls registered wakeup process function
 *        - Executes all registered wakeup hooks
 *        - Triggers power key event for system resume
 *       Debug clock frequency information is printed for system validation
 * 
 * This function handles the system wakeup process from low-power states deeper than
 * basic sleep. It performs hardware status checks, interrupt management, and
 * executes wakeup hooks to restore devices to normal operation.
 */
VOID low_power_user_exit(VOID)
{
    lp_state current_state = _get_lp_state();
    uint32_t isAoWkupInt = 0;
    uint32_t isRtcInt = 0;

    if (current_state == LOW_POWER_SLEEP)
    {
        return;
    }

    isAoWkupInt = NVIC_GetPendingIRQ(AOWKUPINT_IRQn);
    isRtcInt = HAL_RTC_GetRawInt();
    printf("\r\n");
    printf("isAoWkupInt = %ld, isRtcInt = %ld\r\n", isAoWkupInt, isRtcInt);

    if (isAoWkupInt && isRtcInt)
    {
        // printf("CGUFSFREQ0: fclk = %ld MHz, clkpke = %ld MHz\r\n",
        //         (DARIC_CGU->cgufsfreq0 >> 16) & 0x0000FFFF, DARIC_CGU->cgufsfreq0 & 0x0000FFFF);
        // printf("CGUFSFREQ1: clkao = %ld MHz, clkaoram = %ld MHz\r\n",
        //         (DARIC_CGU->cgufsfreq1 >> 16) & 0x0000FFFF, DARIC_CGU->cgufsfreq1 & 0x0000FFFF);
        // printf("CGUFSFREQ2: osc = %ld MHz, XTAL = %ld MHz\r\n",
        //         (DARIC_CGU->cgufsfreq2 >> 16) & 0x0000FFFF, DARIC_CGU->cgufsfreq2 & 0x0000FFFF);
        // printf("CGUFSFREQ3: pll0 = %ld MHz, pll1 = %ld MHz\r\n",
        //        (DARIC_CGU->cgufsfreq3 >> 16) & 0x0000FFFF, 
        //        DARIC_CGU->cgufsfreq3 & 0x0000FFFF
        //       );
    }
    else
    {
        HAL_RTC_IT_Enable(0);
        HAL_RTC_ClearFlag();
        HAL_NVIC_DisableIRQ(AOWKUPINT_IRQn);
        // printf("CGUFSFREQ3: pll0 = %ld MHz, pll1 = %ld MHz\r\n",
        //        (DARIC_CGU->cgufsfreq3 >> 16) & 0x0000FFFF, 
        //        DARIC_CGU->cgufsfreq3 & 0x0000FFFF
        //       );
        if (g_wakeup_process)
        {
            g_wakeup_process(&current_state);
        }
        /* exec wakeup hook */
        //_exec_wakeup_hooks();
        // submitWorkItem(tempTaskWork, NULL, DEV_ID_OTHER);
        #if defined(CONFIG_BOARD_ACTIVECARD_NTO_DVT2) || defined(CONFIG_BOARD_ACTIVECARD_NTO)
        #ifdef CONFIG_DARIC_GUIX
        if (!get_update_pd_mode_window_state())
        {
            extern void bsp_pm_send_short_powerkey_event();
            bsp_pm_send_short_powerkey_event();
        }
        #endif
        #endif
    }
}

/**
 * @brief Clear Always-On domain interrupts (Workaround for IRQ31 issue)
 * @param None
 * @return None
 * 
 * @note This is a critical workaround for hardware IRQ31 interrupt issue
 *       Clears TG28 interrupt sources that may contribute to IRQ31 problem
 *       Clears RTC flags to eliminate another potential source of IRQ31
 *       Should be called during system wakeup and interrupt cleanup phases
 *       The IRQ31 issue manifests as continuous interrupt reporting even after handling
 *       This workaround ensures system stability by preventing interrupt storms
 * 
 * This function serves as a workaround solution to prevent the continuous
 * reporting of IRQ31 interrupts in the system. It clears both TG28 and RTC
 * interrupt flags to eliminate spurious interrupt sources that could trigger
 * the persistent IRQ31 problem.
 */
void clearAonIrq(void)
{
    extern void clearTG28Irq(void);
    clearTG28Irq();
    HAL_RTC_ClearFlag();
    HAL_NVIC_DisableIRQ(AOWKUPINT_IRQn);
}
/**
 * @brief Get the state of updating PD (Power Delivery) mode window
 * 
 * @return true  - update PD mode window
 * @return false - does not update PD mode update window
 */
bool get_update_pd_mode_window_state()
{
    return g_update_pd_window;
}

/**
 * @brief Control the updating PD mode window state
 * 
 * @param state whether need to update PD mode window
 */
void set_update_pd_mode_window_state(bool state)
{
    g_update_pd_window = state;
}