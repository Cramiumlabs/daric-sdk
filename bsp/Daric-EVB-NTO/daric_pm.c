/**
 ******************************************************************************
 * @file    daric_pm.c
 * @author  PERIPHERIAL BSP Team
 * @brief   This file contains the apis to power manager module
 ******************************************************************************
 * @attention
 *
 * Copyright 2024-2026 CrossBar, Inc.
 * This file has been modified by CrossBar, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *******************************************************************************
 */
#define LOG_LEVEL LOG_LEVEL_D

#include <stdint.h>

#include <tx_api.h>

#include "daric_errno.h"
#include "daric_hal_gpio.h"
#include "daric_log.h"

#include "tg28.h"

#include "daric_pm.h"

#define SIZE_ALIAGNMENT(type, n) ((sizeof(type) + n - 1) / n)
#define ARRY_LEN(arry)           (sizeof(arry) / sizeof(arry[0]))

#define STACK_SIZE         2048
#define MAX_MSG            8
#define TIMER_FIRST_TIME   (CONFIG_SYS_CLOCK_TICKS_PER_SEC)
#define TIMER_REPEAT_CYCLE (CONFIG_SYS_CLOCK_TICKS_PER_SEC * 3)
#define MAX_LISTENERS      2

typedef enum
{
    MSG_TIMER,    // Timer message
    MSG_TG28_INT, // TG28 Interrupt
} MESSAGE;

typedef struct
{
    MESSAGE  msg;
    uint32_t data;
} MSG_T;

/**
 * This enumeration indicate the different charging temperature zone.
 */
typedef enum
{
    CHG_TZ01, // (-8,  0], Cold tz, Disable charging
    CHG_TZ12, // ( 0, 10], Low tz, Enable charging
    CHG_TZ23, // (10, 18], Normal tz, Enable charging
    CHG_TZ34, // (18, 45], Warm tz, Enable charging
    CHG_TZ45, // (45, +8), Hot tz, Disable charging
} CHG_TZ_E;

/**
 * This enumeration indicate the different charging status
 */
typedef enum
{
    CHG_ST_NONE, // Doesn't charging
    CHG_ST_PRE,  // Pre-charging
    CHG_ST_CC,   // Const current charging
    CHG_ST_CV,   // Const voltage charging
    CHG_ST_CHG,  // Charging status. Some charge IC can't differ cc and cv.
    CHG_ST_DONE, // Charge completed
} CHG_ST_E;

typedef struct
{
    BSP_PM_EvntListener listener;
    uint32_t            event;
} LISTENER;

typedef struct
{
    bool inited;

    uint8_t   stack[STACK_SIZE];
    TX_THREAD tcb;

    uint8_t  msg[MAX_MSG * SIZE_ALIAGNMENT(MSG_T, 4)];
    TX_QUEUE msg_que;

    TX_TIMER timer;
    uint8_t  batt_capacity;
    CHG_TZ_E chg_tz;
    CHG_ST_E chg_st;

    LISTENER listeners[MAX_LISTENERS];
} PM_Ctx;

static void init_perpherial();
static void init_batteryCapacity();
static void init_charge_param();

static void timerCallback(ULONG arg);
static void thread_main(ULONG thread_input);

static BSP_PM_CHARG_ST convert_chargStatus(CHG_ST_E st);

static PM_Ctx ctx = { 0 };

int BSP_PM_init(void)
{
    UINT status = TX_SUCCESS;

    if (ctx.inited)
    {
        LOGW("Has initialized before");
        return BSP_ERROR_NONE;
    }

    status = tx_queue_create(&ctx.msg_que, "power manaager queue", SIZE_ALIAGNMENT(MSG_T, 4), &ctx.msg, MAX_MSG * SIZE_ALIAGNMENT(MSG_T, 4));
    if (status != TX_SUCCESS)
    {
        LOGE("Create queue failed status=%d", status);
        return BSP_ERROR_UNKNOWN_FAILURE;
    }
    else
    {
        LOGI("MSG queue created");
    }

    status = tx_thread_create(&ctx.tcb, "power manager task", thread_main, (ULONG)0, (VOID *)ctx.stack, STACK_SIZE, 14, 14, 10, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        LOGE("Create thread failed status=%d", status);
        goto create_thread_failed;
    }
    else
    {
        LOGI("Thread created");
    }

    status = tx_timer_create(&ctx.timer, "batt_capa_timer", timerCallback, (ULONG)0, TIMER_FIRST_TIME, TIMER_REPEAT_CYCLE, TX_AUTO_ACTIVATE);
    if (status != TX_SUCCESS)
    {
        LOGE("Create battery timer failed status=%d", status);
        goto timer_failed;
    }
    else
    {
        LOGI("Battery timer created");
    }

    init_perpherial();
    init_batteryCapacity();
    init_charge_param();

    ctx.inited = true;
    return BSP_ERROR_NONE;

timer_failed:
    // TODO delete thread
create_thread_failed:
    // TODO delete queue

    return BSP_ERROR_UNKNOWN_FAILURE;
}

int BSP_PM_reset(void)
{
    TG28_Soft_Reset();
    return BSP_ERROR_NONE;
}

int BSP_PM_poweroff(void)
{
    TG28_Soft_Power_Off();
    return BSP_ERROR_NONE;
}

void BSP_PM_PWR_en(uint8_t ch, bool en)
{
    if (en)
    {
        TG28_Ch_Power_On(ch);
    }
    else
    {
        TG28_Ch_Power_Off(ch);
    }
}

void BSP_PM_PWR_set(uint8_t ch, uint16_t mv)
{
    TG28_Ch_Power_Set(ch, mv);
}

uint8_t BSP_PM_BAT_getCapacity()
{
    return ctx.batt_capacity;
}

BSP_PM_CHARG_ST BSP_PM_BAT_getChargStatus()
{
    return (convert_chargStatus(ctx.chg_st));
}

void BSP_PM_registerEventListener(BSP_PM_EvntListener listener, uint32_t event)
{
    LISTENER *p = NULL;

    if (listener == NULL)
    {
        LOGE("The listener is NULL");
        return;
    }

    for (int i = 0; i < MAX_LISTENERS; i++)
    {
        if (ctx.listeners[i].listener == NULL)
        {
            p = &ctx.listeners[i];
            break;
        }
    }
    if (p == NULL)
    {
        LOGE("Listener is full. Max listener count is  %d", MAX_LISTENERS);
        return;
    }

    p->listener = listener;
    p->event    = event;
}

void BSP_PM_unRegisterEventListener(BSP_PM_EvntListener listener)
{
    LISTENER *p = NULL;
    for (int i = 0; i < MAX_LISTENERS; i++)
    {
        if (ctx.listeners[i].listener == listener)
        {
            p = &ctx.listeners[i];
        }
    }
    if (p != NULL)
    {
        p->listener = NULL;
        p->event    = 0;
    }
    else
    {
        LOGW("The listener hasn't been registered");
    }
}

//-----------------------------------------------------------------------------
// The static function begin
//-----------------------------------------------------------------------------
#define TG28_INT_PIN  GPIO_PIN_0
#define TG28_INT_PORT GPIOF

#define CHG_THRE_T1 2272 ///<  0 degrees ntc voltages
#define CHG_THRE_T2 1973 ///< 10 degrees ntc voltages
#define CHG_THRE_T3 1312 ///< 18 degrees ntc voltages
#define CHG_THRE_T4 492  ///< 45 degrees ntc voltages

#define CHG_RISE_D1 (-52) ///<  0 degrees rise hysteresis voltages
#define CHG_RISE_D2 (-33) ///< 10 degrees rise hysteresis voltages
#define CHG_RISE_D3 (-22) ///< 18 degrees rise hysteresis voltages
#define CHG_RISE_D4 (-30) ///< 45 degrees rise hysteresis voltages

#define CHG_DROP_D1 67 ///<  0 degrees drop hysteresis voltages
#define CHG_DROP_D2 41 ///< 10 degrees drop hysteresis voltages
#define CHG_DROP_D3 26 ///< 18 degrees drop hysteresis voltages
#define CHG_DROP_D4 37 ///< 45 degrees drop hysteresis voltages

#define CHG_RISE_T1 (CHG_THRE_T1 + CHG_RISE_D1)
#define CHG_RISE_T2 (CHG_THRE_T2 + CHG_RISE_D2)
#define CHG_RISE_T3 (CHG_THRE_T3 + CHG_RISE_D3)
#define CHG_RISE_T4 (CHG_THRE_T4 + CHG_RISE_D4)

#define CHG_DROP_T1 (CHG_THRE_T1 + CHG_DROP_D1)
#define CHG_DROP_T2 (CHG_THRE_T2 + CHG_DROP_D2)
#define CHG_DROP_T3 (CHG_THRE_T3 + CHG_DROP_D3)
#define CHG_DROP_T4 (CHG_THRE_T4 + CHG_DROP_D4)

// clang-format off
// #define CHARG_PC   3    // Pre-charge current 0.01C, 3mA
// #define CHARG_TC   11   // Termination current 0.05C, 11mA
// #define CHARG_CC12 48   //  0~10 deg, Const current 0.2C, 48mA
// #define CHARG_CC23 112  // 10~18 deg, Const current 0.5C, 112mA
// #define CHARG_CC34 224  // 18~45 deg, Const current 1C, 224mA

#define CHARG_PC   TG28_PRE_C_25
#define CHARG_TC   TG28_TER_C_25
#define CHARG_CC12 TG28_CON_C_50
#define CHARG_CC23 TG28_CON_C_125
#define CHARG_CC34 TG28_CON_C_200
// clang-format on

static void tg28_int_handle(void *UserData)
{
    uint32_t irq = 0;
    do
    {
        irq |= TG28_readIrq();
        TG28_clearIrq(irq);
    } while (HAL_GPIO_ReadPin(TG28_INT_PORT, TG28_INT_PIN) == GPIO_PIN_RESET);
    MSG_T msg = { MSG_TG28_INT, irq };
    tx_queue_send(&ctx.msg_que, &msg, TX_NO_WAIT);
}

static void tg28_int_init()
{
    GPIO_InitTypeDef init = { 0 };
    init.Pin              = TG28_INT_PIN;
    init.Mode             = GPIO_MODE_IT_LOW_LEVEL;
    init.Pull             = GPIO_PULLUP;
    init.IsrHandler       = tg28_int_handle;
    init.UserData         = NULL;
    HAL_GPIO_Init(TG28_INT_PORT, &init);
}

// clang-format off
static const REG8_MAP tg28_regs[] = {
    {0x18, 0x0A}, // Disable button battery charge function
    {0x27, 0x0B}, // OFFLEVEL 8s, ONLEVEL 2s
    {0x50, 0x0E}, // Set TS current 50uA
};
// clang-format on

static void init_tg28()
{
    TG28_I2C_init();
    // TG28_dumpRegister();
    TG28_initRegister(tg28_regs, ARRY_LEN(tg28_regs));
    TG28_disableIrq(TG28_IRQ_MASK_ALL);
    uint32_t irq_mask = TG28_IRQ_MASK_BAT_OT_CHG | TG28_IRQ_MASK_BAT_UT_CHG | TG28_IRQ_MASK_VBUS_INSER | TG28_IRQ_MASK_VBUS_RMV | TG28_IRQ_MASK_PO_SHORT | TG28_IRQ_MASK_PO_LONG
                        | TG28_IRQ_MASK_CHG_DONE | TG28_IRQ_MASK_CHG_STAR;
    TG28_enableIrq(irq_mask);
    TG28_clearIrq(TG28_IRQ_MASK_ALL);
    tg28_int_init();
}

static void init_perpherial()
{
    init_tg28();
}

#define CAPA_FILT_LEN 1 // Battery capacity filter length
static uint8_t s_capacity[CAPA_FILT_LEN] = { 0 };
static uint8_t s_capa_idx                = 0;

static uint8_t filter_batterCapacity()
{
    uint16_t capa = 0;
    for (int i = 0; i < CAPA_FILT_LEN; i++)
    {
        capa += s_capacity[i];
    }
    return capa / CAPA_FILT_LEN;
}

static void init_batteryCapacity()
{
    uint8_t capa     = 0;
    uint8_t try_cnt  = 0;
    int     read_cnt = 0;
    do
    {
        if (BSP_ERROR_NONE == TG28_BAT_CAP_Read(&capa))
        {
            s_capacity[read_cnt++] = capa;
        }
        else
        {
            LOGW("Failed, try count: %d, read count: %d", try_cnt, read_cnt);
        }
        try_cnt++;
    } while ((read_cnt < CAPA_FILT_LEN) && try_cnt < 50);

    if (read_cnt < CAPA_FILT_LEN)
    {
        LOGW("Read capacity failed.");
    }

    s_capa_idx        = 0;
    ctx.batt_capacity = filter_batterCapacity();
}

static CHG_TZ_E get_ChargeTempZone(uint16_t mv)
{
    CHG_TZ_E range;
    if (mv > CHG_THRE_T1)
    {
        range = CHG_TZ01;
    }
    else if (mv > CHG_THRE_T2)
    {
        range = CHG_TZ12;
    }
    else if (mv > CHG_THRE_T3)
    {
        range = CHG_TZ23;
    }
    else if (mv > CHG_THRE_T4)
    {
        range = CHG_TZ34;
    }
    else
    {
        range = CHG_TZ45;
    }
    return range;
}

static CHG_TZ_E init_ChargeTempZone()
{
    CHG_TZ_E tz = CHG_TZ01;
    uint16_t mv = 0;
    if (BSP_ERROR_NONE == TG28_getTsVoltage(&mv))
    {
        tz = get_ChargeTempZone(mv);
    }
    return tz;
}

static CHG_TZ_E correct_chargeTempZone(CHG_TZ_E pre_tz, uint16_t temp_vm)
{
    CHG_TZ_E tz = pre_tz;
    switch (pre_tz)
    {
        case CHG_TZ01:
            if (temp_vm > CHG_RISE_T1)
            {
                tz = CHG_TZ01;
            }
            else
            {
                tz = get_ChargeTempZone(temp_vm);
            }
            break;
        case CHG_TZ12:
            if (CHG_RISE_T2 < temp_vm && temp_vm < CHG_DROP_T1)
            {
                tz = CHG_TZ12;
            }
            else
            {
                tz = get_ChargeTempZone(temp_vm);
            }
            break;
        case CHG_TZ23:
            if (CHG_RISE_T3 < temp_vm && temp_vm < CHG_DROP_T2)
            {
                tz = CHG_TZ23;
            }
            else
            {
                tz = get_ChargeTempZone(temp_vm);
            }
            break;
        case CHG_TZ34:
            if (CHG_RISE_T4 < temp_vm && temp_vm < CHG_DROP_T3)
            {
                tz = CHG_TZ34;
            }
            else
            {
                tz = get_ChargeTempZone(temp_vm);
            }
            break;
        case CHG_TZ45:
            if (temp_vm < CHG_DROP_T4)
            {
                tz = CHG_TZ45;
            }
            else
            {
                tz = get_ChargeTempZone(temp_vm);
            }
            break;
    }
    return tz;
}

static CHG_ST_E get_chargeStatus()
{
    CHG_ST_E    status  = CHG_ST_NONE;
    TG28_CHG_ST tg28_st = TG28_getChargeStatus();
    switch (tg28_st)
    {
        case TG28_CHG_ST_NONE:
            status = CHG_ST_NONE;
            break;
        case TG28_CHG_ST_PRE:
            status = CHG_ST_PRE;
            break;
        case TG28_CHG_ST_CC:
            status = CHG_ST_CC;
            break;
        case TG28_CHG_ST_CV:
            status = CHG_ST_CV;
            break;
        case TG28_CHG_ST_TRI:
        case TG28_CHG_ST_DONE:
            status = CHG_ST_DONE;
            break;
    }

    return status;
}

static BSP_PM_CHARG_ST convert_chargStatus(CHG_ST_E st)
{
    BSP_PM_CHARG_ST status;
    switch (st)
    {
        case CHG_ST_NONE:
            status = BSP_PM_CHARG_ST_NONE;
            break;
        case CHG_ST_PRE:
        case CHG_ST_CC:
        case CHG_ST_CV:
        case CHG_ST_CHG:
            status = BSP_PM_CHARG_ST_CHGING;
            break;
        case CHG_ST_DONE:
            status = BSP_PM_CHARG_ST_DONE;
            break;
    }
    return status;
}

static inline void set_charge_pc(CHG_TZ_E tz)
{
    TG28_SetChargePreCurrent(CHARG_PC);
}

static inline void set_charge_tc(CHG_TZ_E tz)
{
    TG28_SetChargeTerCurrent(CHARG_TC);
}

static void set_charge_cc(CHG_TZ_E tz)
{
    switch (tz)
    {
        case CHG_TZ01:
        case CHG_TZ12:
            TG28_SetChargeConCurrent(CHARG_CC12);
            break;
        case CHG_TZ23:
            TG28_SetChargeConCurrent(CHARG_CC23);
            break;
        case CHG_TZ34:
        case CHG_TZ45:
            TG28_SetChargeConCurrent(CHARG_CC34);
            break;
    }
}

static void set_chargeCurrent(CHG_TZ_E tz, CHG_ST_E st)
{
    switch (st)
    {
        case CHG_ST_NONE:
        case CHG_ST_PRE:
            set_charge_pc(tz);
            break;
        case CHG_ST_CC:
        case CHG_ST_CV:
        case CHG_ST_CHG:
        case CHG_ST_DONE:
            set_charge_tc(tz);
            break;
    }
    set_charge_cc(tz);
}

static void init_charge_param()
{
    ctx.chg_tz = init_ChargeTempZone();
    ctx.chg_st = get_chargeStatus();
    set_chargeCurrent(ctx.chg_tz, ctx.chg_st);
}

static void timerCallback(ULONG arg)
{
    MSG_T msg = { MSG_TIMER };
    tx_queue_send(&ctx.msg_que, &msg, TX_NO_WAIT);
}

#define NOTIFY_EVENT(e, param)                         \
    do                                                 \
    {                                                  \
        for (int i = 0; i < MAX_LISTENERS; i++)        \
        {                                              \
            LISTENER *p = &ctx.listeners[i];           \
            if (p->listener != NULL && (p->event & e)) \
            {                                          \
                p->listener(e, param);                 \
            }                                          \
        }                                              \
    } while (0)

static void timer_handle_chargeStatus()
{
    uint16_t vm = 0;
    CHG_TZ_E tz;
    CHG_ST_E st;

    int ret = TG28_getTsVoltage(&vm);

    if (BSP_ERROR_NONE != ret)
    {
        LOGW("Get ts voltage failed.");
        return;
    }

    tz = correct_chargeTempZone(ctx.chg_tz, vm);
    st = get_chargeStatus();
    if (tz != ctx.chg_tz || st != ctx.chg_st)
    {
        LOGI("Charge temperature zone: %d, status: %d", tz, st);
        set_chargeCurrent(tz, st);
        BSP_PM_CHARG_ST st1 = convert_chargStatus(ctx.chg_st);
        BSP_PM_CHARG_ST st2 = convert_chargStatus(st);
        if (st1 != st2)
        {
            NOTIFY_EVENT(BSP_PM_EVT_BAT_CHARGE, st2);
        }
        ctx.chg_tz = tz;
        ctx.chg_st = st;
    }
}

static void timer_handle_battCapacity()
{
    uint8_t capa_n;
    uint8_t capa_f;
    int     ret = TG28_BAT_CAP_Read(&capa_n);
    if (ret == BSP_ERROR_NONE)
    {
        s_capacity[s_capa_idx++] = capa_n;
        s_capa_idx %= CAPA_FILT_LEN;
        capa_f = filter_batterCapacity();
        if (capa_f != ctx.batt_capacity)
        {
            LOGI("capacity = %d", capa_f);
            ctx.batt_capacity = capa_f;
            NOTIFY_EVENT(BSP_PM_EVT_BAT_CAPACITY, ctx.batt_capacity);
        }
    }
}

static void timer_process()
{
    timer_handle_chargeStatus();
    timer_handle_battCapacity();
}

static void tg28_iqr(uint32_t irq)
{
    LOGI("IRQ: 0x%06lX", irq);

    if (irq & TG28_IRQ_MASK_BAT_OT_CHG)
    {
        NOTIFY_EVENT(BSP_PM_EVT_BAT_HOT_TEMP, 0);
    }
    if (irq & TG28_IRQ_MASK_BAT_UT_CHG)
    {
        NOTIFY_EVENT(BSP_PM_EVT_BAT_COLD_TEMP, 0);
    }
    if (irq & TG28_IRQ_MASK_VBUS_INSER)
    {
        timer_handle_chargeStatus();
    }
    if (irq & TG28_IRQ_MASK_VBUS_RMV)
    {
        timer_handle_chargeStatus();
    }
    if (irq & TG28_IRQ_MASK_PO_SHORT)
    {
        NOTIFY_EVENT(BSP_PM_EVT_PEK_SHORT_PRESS, 0);
    }
    if (irq & TG28_IRQ_MASK_PO_LONG)
    {
        NOTIFY_EVENT(BSP_PM_EVT_PEK_LONG_PRESS, 0);
    }
    if (irq & TG28_IRQ_MASK_CHG_DONE || irq & TG28_IRQ_MASK_CHG_STAR)
    {
        timer_handle_chargeStatus();
    }
}

static void thread_main(ULONG thread_input)
{
    MSG_T message;
    LOGI("start");

    while (1)
    {
        UINT status = tx_queue_receive(&ctx.msg_que, &message, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
        {
            LOGE("Receive message failed. status:%d", status);
            continue;
        }
        // LOGI("Receive message, msg=%d", message.msg);
        switch (message.msg)
        {
            case MSG_TIMER:
                timer_process();
                break;
            case MSG_TG28_INT:
                tg28_iqr(message.data);
                break;
        }
    }
}
