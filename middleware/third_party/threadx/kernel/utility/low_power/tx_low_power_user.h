/**
 ******************************************************************************
 * @file    tx_low_power_user.h
 * @author  LOW_POWER Team
 * @brief   Header file of low_power module.
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

#ifndef TX_LOW_POWER_USER_H
#define TX_LOW_POWER_USER_H

#include "tx_api.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  LOW_POWER_SLEEP, 
  LOW_POWER_DEEP_SLEEP_FD, 
                           
  LOW_POWER_DEEP_SLEEP_IP,
  LOW_POWER_DEEP_SLEEP_DEEP,
  LOW_POWER_POWER_DOWN, 
  LOW_POWER_MAX
} lp_state;

typedef struct {
  lp_state state;
  void *user_param;
} lp_func_param_t;

typedef UINT (*LP_HOOK_FUN)(VOID *param);
typedef UINT (*LP_PROCESS_FUN)(VOID *param);

/* Follow, APIs available for user access. */
UINT low_power_register_hook(UINT device_id, LP_HOOK_FUN sleep_hook_fun,
                             VOID *sleep_param_ptr, LP_HOOK_FUN wakeup_hook_fun,
                             VOID *wakeup_param_ptr);
UINT low_power_unregister_hook(UINT device_id);
VOID low_power_wake_lock(CHAR *lock_name);
VOID low_power_wake_unlock(CHAR *lock_name);
bool low_power_if_locked(VOID);
VOID low_power_set_transition_ms(ULONG light_to_ip_ms, ULONG ip_to_dp_ms);
VOID low_power_notify_wakeup_source(bool wakeup_from_lp_timer);

/* Ignore, chip platform uses. */
UINT low_power_register_process(UINT device_id,
                                LP_PROCESS_FUN sleep_process_fun,
                                VOID *sleep_param_ptr,
                                LP_PROCESS_FUN wakeup_process_fun,
                                VOID *wakeup_param_ptr);
UINT low_power_register_timer_funcs(UINT (*setup_ms)(ULONG *ms),
                                    ULONG (*adjust_ms)(VOID *param));
/* Ignore, 'tx_low_power.c' uses */
void low_power_timer_config(ULONG ticks);
ULONG low_power_timer_adjust(void);
UINT low_power_user_enter(VOID);

#define LIGHT_SLEEP_TIME_MS (60 * 1000)
#define DEEPIP_SLEEP_TIME_MS (5 * 60 * 1000)
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
VOID low_power_user_exit(VOID);

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
void clearAonIrq(void);

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
void switchDffV33AO(bool on);

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
bool low_power_has_locked(CHAR *lock_name);

/**
 * @brief Get the state of updating PD (Power Delivery) mode window
 * 
 * @return true  - update PD mode window
 * @return false - does not update PD mode update window
 */
bool get_update_pd_mode_window_state();

/**
 * @brief Control the updating PD mode window state
 * 
 * @param state whether need to update PD mode window
 */
void set_update_pd_mode_window_state(bool state);
#endif /* TX_LOW_POWER_USER_H */
