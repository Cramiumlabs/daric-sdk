/**
******************************************************************************
* @file    daric_fingerprint.c
* @author  PERIPHERIAL BSP Team
* @brief   This file includes the driver for fingerprint module.
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
#define LOG_LEVEL LOG_LEVEL_I

#include "daric_fingerprint.h"
#include "daric_errno.h"
#include "daric_hal_gpio.h"
#include "daric_log.h"
#include "lcd_common.h"
#include "mlb_error_code.h"
#include "mlb_intf.h"
#include "mlb_platform.h"
#include "tx_port.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/_intsup.h>
#include <sys/types.h>
#include <tx_api.h>

#define FP_MAX_USER_UID MLB_USER_MAX_NUM     // Max users.
#define FP_MAX_USER_FID MLB_MAX_FID_PER_USER // Max fingerprints every user.

#define FP_MAX_VIP              5
#define FP_MAX_ENROLL_SAMPLE    MLB_MAX_ENROLL_STEP // Max steps when enroll
#define FP_ENROLL_DUPAREA_TH    80
#define FP_ENROLL_DUPAREA_START 2
#define FP_TPL_SIZE             (12 * 1024)

#define FP_TACK_SIZE 8192 // Task stack size
#define FP_MSG_MAX   8    // Message queue size

typedef enum
    : uint16_t
{
    TA_E_ENROLL_START    = 0,
    TA_E_ENROLL_CONTINUE = 1,
    TA_E_ENROLL_CANCEL   = 2,
    TA_E_DETECT_HAPPEN   = 3,
    TA_E_DETECT_START    = 4,
    TA_E_DETECT_CONTINUE = 5,
    TA_E_DETECT_STOP     = 6,
} TASK_ACTION;

#define ACT_STR(action) #action

char *act_str[] = {
    [TA_E_ENROLL_START] = ACT_STR(TA_E_ENROLL_START),   [TA_E_ENROLL_CONTINUE] = ACT_STR(TA_E_ENROLL_CONTINUE), [TA_E_ENROLL_CANCEL] = ACT_STR(TA_E_ENROLL_CANCEL),
    [TA_E_DETECT_HAPPEN] = ACT_STR(TA_E_DETECT_HAPPEN), [TA_E_DETECT_START] = ACT_STR(TA_E_DETECT_START),       [TA_E_DETECT_CONTINUE] = ACT_STR(TA_E_DETECT_CONTINUE),
    [TA_E_DETECT_STOP] = ACT_STR(TA_E_DETECT_STOP),
};

typedef struct
{
    TASK_ACTION action;
    uint16_t    fid;
} MSG_T;

static void fingerprint_entry(ULONG arg);
static void send_msg(TASK_ACTION action, uint16_t fid);
static void interrupt_enable(bool enable);
static void switch_detect();
static void irq_handler(void *data);

#define INT_PORT CONFIG_FP_INT_PORT
#define INT_PIN  CONFIG_FP_INT_PIN
static void interrup_init()
{
    GPIO_InitTypeDef init = { .Pin = INT_PIN, .Mode = GPIO_MODE_IT_RISING, .Pull = GPIO_NOPULL, .IsrHandler = irq_handler };
    HAL_GPIO_Init(INT_PORT, &init);
}
static void interrupt_enable(bool enable)
{
    HAL_GPIO_EnableIT(INT_PORT, INT_PIN, enable);
}

#ifdef CONFIG_FP_PW_CTRL
#define PW_PORT CONFIG_FP_PW_PORT
#define PW_PIN  CONFIG_FP_PW_PIN
static void power_init()
{
    GPIO_InitTypeDef init = { .Pin = PW_PIN, .Mode = GPIO_MODE_OUTPUT, .Pull = GPIO_NOPULL };
    HAL_GPIO_Init(PW_PORT, &init);
}
static void power_enable(bool enable)
{
    HAL_GPIO_WritePin(PW_PORT, PW_PIN, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
#endif

typedef struct
{
    bool    inited;
    bool    thread_started;
    bool    detecting;
    bool    detect_continue;
    bool    enrolling;
    uint8_t enroll_update_times;

    uint16_t user_fids[FP_MAX_USER_FID];
    uint8_t  user_fid_count;

    BSP_FP_LISENTER_T listener;

    uint8_t   msg_buf[FP_MSG_MAX * sizeof(MSG_T)];
    TX_QUEUE  msg_que;
    TX_THREAD tcb;
    char      stack[FP_TACK_SIZE];
} Context;

static Context ctx = { 0 };
#define CHECK_MODULE_INITED           \
    do                                \
    {                                 \
        if (!ctx.inited)              \
        {                             \
            LOGE("Has't inited");     \
            return BSP_ERROR_NO_INIT; \
        }                             \
    } while (0)

int16_t BSP_FP_Init(uint16_t uid)
{
    mlb_img_info_t   img_info = { 0 };
    mlb_sdk_info_t   sdk_info = { 0 };
    MLB_CHIPID       chipid   = 0;
    mlb_init_param_t param    = { 0 };
    MRT_T            ret      = MRT_OK;

    if (ctx.inited)
    {
        LOGI("Has inited before");
        return BSP_ERROR_NONE;
    }

// Turn on fingerprint sensor power
#ifdef CONFIG_FP_PW_CTRL
    power_init();
    power_enable(true);
#endif

    ret = mlb_intf_platform_init();
    if (ret != MRT_OK)
    {
        LOGE("Platform init failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("Platform inited");

    ret = mlb_intf_get_sdk_info(&sdk_info);
    if (ret != MRT_OK)
    {
        LOGE("Get sdk info failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("SDK ver: %s, com-id: %s", sdk_info.version, sdk_info.commit_id);

    ret = mlb_intf_sensor_read_chipid(&chipid);
    if (ret != MRT_OK)
    {
        LOGE("Read chipid failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("chipid: 0x%05X", chipid);
    if (chipid != MLB_CHIPID_E088N)
    {
        LOGE("Unknow chip");
        return FP_ERR_UNKNOW_CHIP;
    }

    mlb_sensor_reg_e088n();
    param.sensor_type          = MLB_SENSOR_TYPE_COATING;
    param.fp_tpl_size          = FP_TPL_SIZE;
    param.max_fp_number        = FP_MAX_USER_UID * FP_MAX_USER_FID;
    param.max_enroll_steps     = FP_MAX_ENROLL_SAMPLE;
    param.max_vip_cnt          = FP_MAX_VIP;
    param.duplicate_area_th    = FP_ENROLL_DUPAREA_TH;
    param.duplicate_area_start = FP_ENROLL_DUPAREA_START;
    param.algo_ram_level       = 1;
    param.algo_time_level      = 1;
    param.algo_far_leval       = 1;
    param.algo_run_mode        = 0;
#if LOG_LEVEL > LOG_LEVEL_D
    param.dbglvl = 1;
#else
    param.dbglvl = 2;
#endif
    ret = mlb_intf_init(&param);
    if (ret != MRT_OK)
    {
        LOGE("Interface init failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("Interface inited");

    ret = mlb_intf_get_image_info(&img_info);
    if (ret != MRT_OK)
    {
        LOGE("Get image info failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("img-inf, w: %d, h: %d, img-bit: %d, org-img-bit: %d", img_info.img_width, img_info.img_height, img_info.img_bit, img_info.org_img_bit);

    ret = mlb_intf_set_active_user(uid);
    if (ret != MRT_OK)
    {
        LOGE("Set uid failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("Setted uid: %d", uid);

    ret = mlb_intf_get_current_user_fids(ctx.user_fids, &ctx.user_fid_count);
    // TODO list current user fids

    LOGI("Create fingerprint thread");
    if (!ctx.thread_started)
    {
        UINT status;
        status = tx_thread_create(&ctx.tcb, "fingerprint_entry", fingerprint_entry, (ULONG)0, (VOID *)ctx.stack, FP_TACK_SIZE, 14, 14, 10, TX_AUTO_START);
        LOGI("Creat thread: %s", status == TX_SUCCESS ? "succeed" : "failed");
    }

    interrup_init();
    interrupt_enable(false);

    ctx.inited = true;
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_DeInit()
{
    MRT_T ret;

    if (!ctx.inited)
    {
        LOGI("Has't inited");
        return BSP_ERROR_NONE;
    }

    ret = mlb_intf_deinit();
    if (ret != MRT_OK)
    {
        LOGE("Interface deinit failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    LOGI("interface deinited");
    ctx.inited = false;
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_Calibrate()
{
    MRT_T ret = MRT_OK;
    CHECK_MODULE_INITED;

    ret = mlb_intf_calc_param();
    if (ret != MRT_OK)
    {
        LOGE("Calibrate failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("calibrated");

    return BSP_ERROR_NONE;
}

int16_t BSP_FP_SetUser(int16_t uid)
{
    uint16_t org_uid;
    MRT_T    ret;
    (void)org_uid;
    CHECK_MODULE_INITED;

    org_uid = mlb_intf_get_current_user();
    if (org_uid == uid)
    {
        LOGI("uid not changed, uid=%d", uid);
        return BSP_ERROR_NONE;
    }
    ret = mlb_intf_set_active_user(uid);
    if (ret != MRT_OK)
    {
        LOGE("Set user failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("org-uid: %d, new-uid=%d", org_uid, uid);

    ret = mlb_intf_get_current_user_fids(ctx.user_fids, &ctx.user_fid_count);
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_GetUser(int16_t *uid)
{
    CHECK_MODULE_INITED;

    if (uid == NULL)
    {
        LOGE("Wrong param");
        return BSP_ERROR_WRONG_PARAM;
    }

    *uid = mlb_intf_get_current_user();
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_GetAllUsers(uint16_t *uids, uint16_t *uid_num)
{
    CHECK_MODULE_INITED;
    mlb_intf_get_all_users(uids, uid_num);
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_DltUser(int16_t uid)
{
    MRT_T ret;
    CHECK_MODULE_INITED;

    ret = mlb_intf_delete_user(uid);
    if (ret != MRT_OK)
    {
        LOGE("Delete user failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("Delete user %d", uid);
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_GetCurrentUserFid(uint16_t *fids, uint8_t *fid_num)
{
    CHECK_MODULE_INITED;

    mlb_intf_get_current_user_fids(fids, fid_num);
    LOGD("Get fids count=%d", *fid_num);
    for (int i = 0; i < *fid_num; i++)
    {
        LOGD("fid %d", fids[i]);
    }
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_GetAUserFid(uint16_t uid, uint16_t *fids, uint8_t *fid_num)
{
    MRT_T ret;
    CHECK_MODULE_INITED;

    ret = mlb_intf_get_a_user_fids(uid, fids, fid_num);
    if (ret != MRT_OK)
    {
        LOGW("Get a user fids failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }

    LOGD("Get a user fids, uid=%d, count=%d", uid, *fid_num);
    for (int i = 0; i < *fid_num; i++)
    {
        LOGD("fid %d", fids[i]);
    }
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_DltUsrFid(uint16_t fid)
{
    MRT_T ret;
    CHECK_MODULE_INITED;

    ret = mlb_intf_delete_user_fid(fid);
    if (ret != MRT_OK)
    {
        LOGE("Delete current user fid failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("Delete current user fid: %d", fid);

    mlb_intf_get_current_user_fids(ctx.user_fids, &ctx.user_fid_count);
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_EnrollStart(uint16_t *fid)
{
    uint16_t tmp_fid;
    MRT_T    ret;
    CHECK_MODULE_INITED;

    if (ctx.user_fid_count >= FP_MAX_USER_FID)
    {
        LOGW("User template is full. fid_cnt=%d, max=%d", ctx.user_fid_count, FP_MAX_USER_FID);
        return FP_ERR_ENROLL_FULL;
    }

    ret = mlb_intf_generate_new_fid(&tmp_fid);
    if (ret != MRT_OK)
    {
        LOGW("Generate new fid failed, ret=%d", ret);
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGI("new fid: %d", tmp_fid);

    *fid = tmp_fid;
    LOGD("Send msg: %s, fid: %d", act_str[TA_E_ENROLL_START], tmp_fid);
    send_msg(TA_E_ENROLL_START, tmp_fid);

    return BSP_ERROR_NONE;
}

int16_t BSP_FP_EnrollCancel(uint16_t fid)
{
    CHECK_MODULE_INITED;
    LOGD("Send msg: %s, fid: %d", act_str[TA_E_ENROLL_CANCEL], fid);
    send_msg(TA_E_ENROLL_CANCEL, fid);
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_DetectStart(bool continoues)
{
    CHECK_MODULE_INITED;
    if (continoues)
    {
        LOGD("Send msg: %s, fid: %d", act_str[TA_E_DETECT_CONTINUE], 0xFF);
        send_msg(TA_E_DETECT_CONTINUE, 0xFF);
    }
    else
    {
        LOGD("Send msg: %s, fid: %d", act_str[TA_E_DETECT_START], 0xFF);
        send_msg(TA_E_DETECT_START, 0xFF);
    }
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_DetectStop()
{
    CHECK_MODULE_INITED;
    LOGD("Send msg: %s, fid: %d", act_str[TA_E_DETECT_STOP], 0xFF);
    send_msg(TA_E_DETECT_STOP, 0xFF);
    return BSP_ERROR_NONE;
}

int16_t BSP_FP_Reg(BSP_FP_LISENTER_T listenor)
{
    CHECK_MODULE_INITED;
    ctx.listener = listenor;
    return BSP_ERROR_NONE;
}

static inline void send_msg(TASK_ACTION action, uint16_t fid)
{
    MSG_T msg = { action, fid };
    tx_queue_send(&ctx.msg_que, &msg, TX_NO_WAIT);
}

static bool read_finger()
{
#define MAX_READ_COUNT 5
    uint8_t i = 0;
    MRT_T   ret;

    while (i++ <= MAX_READ_COUNT)
    {
        interrupt_enable(false);
        ret = mlb_intf_finger_detect();
        if (ret == MRT_OK)
        {
            break;
        }
        tx_thread_sleep(10);
    }
    LOGD("Read finger, ret=%d, read times: %d", ret, i);

    return ret == MRT_OK;
}

static inline void notify_event(uint16_t evt, uint16_t fid)
{
    if (ctx.listener != NULL)
    {
        ctx.listener(evt, fid);
    }
}

static inline void switch_detect()
{
    if (ctx.detecting && !ctx.enrolling)
    {
        LOGD("Enable interrupt");
        mlb_intf_detect_mode();
        interrupt_enable(true);
    }
    else
    {
        LOGD("Disable interrupt");
        interrupt_enable(false);
    }
}

static inline void wait_finger_left()
{
    while (!mlb_intf_finger_detect_leave())
    {
        tx_thread_sleep(10);
    };
    LOGD("Finger left");
}

static void irq_handler(void *data)
{
    interrupt_enable(false);
    send_msg(TA_E_DETECT_HAPPEN, 0xFF);
}

static void enroll_start(uint16_t fid)
{
    MRT_T ret;
    ret = mlb_intf_enroll_start(fid);
    if (ret != MRT_OK)
    {
        LOGW("Enroll start failed, ret=%d", ret);
        notify_event(FP_EVENT_ENROLL_FAILED, fid);
        return;
    }

    ctx.enrolling = true;

    ctx.enroll_update_times = 0;
    LOGD("Send msg: %s, fid: %d", act_str[TA_E_ENROLL_CONTINUE], fid);
    send_msg(TA_E_ENROLL_CONTINUE, fid);
}

static void enroll_continue(uint16_t fid)
{
    uint8_t tpl_ok = 0;
    MRT_T   ret;

    if (!ctx.enrolling)
    {
        return;
    }

    // Read finger from sensor
    LOGD("Before read finger");
    if (!read_finger())
    {
        notify_event(FP_EVENT_ENROLL_NO_FINGER, fid);
        goto label_continue;
    }

    // Update finger template
    LOGD("Before finger update, fid count=%d", ctx.user_fid_count);
#if (FP_DUPLICATE_CHECK_T == FP_DUPLICATE_CHECK_USER)
    if (ctx.user_fid_count == 0)
    {
        ret = mlb_intf_enroll_update(0, &tpl_ok);
    }
    else
    {
        ret = mlb_intf_enroll_update_and_check_duplicate(ctx.user_fids, ctx.user_fid_count, &tpl_ok);
    }
#elif (FP_DUPLICATE_CHECK_T == FP_DUPLICATE_CHECK_ALL)
    ret = mlb_intf_enroll_update(1, &tpl_ok);
#elif (FP_DUPLICATE_CHECK_T == FP_DUPLICATE_CHECK_NONE)
    ret = mlb_intf_enroll_update(0, &tpl_ok);
#endif
    LOGD("Finger update, ret=%d, tpl_ok=%d", ret, tpl_ok);
    if (ret == MRT_ENROLL_DUP_AREA)
    {
        LOGW("Finger update failed, duplicate area");
        notify_event(FP_EVENT_ENROLL_AREA_DUP, fid);
        goto label_continue;
    }
    else if (ret == MRT_ENROLL_DUP_FINGER)
    {
        LOGW("Finger update failed, duplicate finger");
        notify_event(FP_EVENT_ENROLL_FINGER_DUP, fid);
        goto label_continue;
    }
    else if (ret != MRT_OK)
    {
        mlb_intf_enroll_discard();
        LOGW("Finger update failed");
        ctx.enrolling = false;
        notify_event(FP_EVENT_ENROLL_FAILED, fid);
        goto label_stop;
    }
    ctx.enroll_update_times++;

    if (!tpl_ok)
    {
        LOGD("Finger update succeed, continue times: %d", ctx.enroll_update_times);
        notify_event(FP_EVENT_ENROLL_CONTINUE, fid);
        wait_finger_left();
        goto label_continue;
    }

    // Commit finger
    ret = mlb_intf_enroll_commit();
    LOGD("Finger commit, ret=%d", ret);
    if (ret != MRT_OK)
    {
        LOGW("Finger commit failed");
        ctx.enrolling = false;
        notify_event(FP_EVENT_ENROLL_FAILED, fid);
        goto label_stop;
    }

    // Set finger to user
    ret = mlb_intf_set_user_fid(fid);
    LOGD("Set user fid, ret=%d", ret);
    if (ret != MRT_OK)
    {
        LOGW("Set user fid failed");
        mlb_intf_tpl_remove(fid);
        ctx.enrolling = false;
        notify_event(FP_EVENT_ENROLL_FAILED, fid);
        goto label_stop;
    }

    // Update user fid info
    mlb_intf_get_current_user_fids(ctx.user_fids, &ctx.user_fid_count);

    ctx.enrolling = false;
    LOGI("Enroll succeed. fid=%d", fid);
    notify_event(FP_EVENT_ENROLL_DONE, fid);

    wait_finger_left();

label_stop:
    switch_detect();
    return;

label_continue:
    LOGD("Send msg: %s, fid: %d", act_str[TA_E_ENROLL_CONTINUE], fid);
    send_msg(TA_E_ENROLL_CONTINUE, fid);
}

static inline void enroll_cancel(uint16_t fid)
{
    if (ctx.enrolling)
    {
        mlb_intf_enroll_discard();
        ctx.enrolling = false;
    }
    notify_event(FP_EVENT_ENROLL_CANCEL, fid);
}

static void inline detect_start()
{
    ctx.detecting       = true;
    ctx.detect_continue = false;
    switch_detect();
}

static void inline detect_continue_start()
{
    ctx.detecting       = true;
    ctx.detect_continue = true;
    switch_detect();
}

static void inline detect_stop()
{
    ctx.detecting       = false;
    ctx.detect_continue = false;
    switch_detect();
}

static void detect_happen()
{
    uint8_t  update       = 0;
    bool     detected     = false;
    uint16_t detected_fid = 0;
    MRT_T    ret;

    if (!ctx.detecting)
    {
        goto label_exit;
    }

    if (!read_finger())
    {
        LOGW("Read finger failed");
        goto label_exit;
    }

    ret = mlb_intf_verify_start();
    if (ret != MRT_OK)
    {
        LOGW("Start verify failed, ret=%d", ret);
        goto label_exit;
    }

    for (int i = 0; i < ctx.user_fid_count; i++)
    {
        ret = mlb_intf_verify_fid(ctx.user_fids[i], &update);
        if (ret == MRT_OK)
        {
            // if (update) {
            //     mlb_intf_study(); // This api is called in liberary
            // }
            detected     = true;
            detected_fid = ctx.user_fids[i];
            break;
        }
        else
        {
            LOGD("verify failed, ret=%d, fid=%d", ret, ctx.user_fids[i]);
        }
    }

    if (detected)
    {
        notify_event(FP_EVENT_DETECT_SUCCEED, detected_fid);
        if (!ctx.detect_continue)
        {
            ctx.detecting = false;
        }
    }
    else
    {
        notify_event(FP_EVENT_DETECT_FAILED, detected_fid);
    }

label_exit:
    switch_detect();
}

static void fingerprint_entry(ULONG arg)
{
    MSG_T message;
    UINT  status = TX_SUCCESS;

    LOGI("Thread start");
    ctx.thread_started = true;

    status = tx_queue_create(&ctx.msg_que, "fingerprint queue", sizeof(MSG_T) / 4, &ctx.msg_buf, FP_MSG_MAX * sizeof(MSG_T));
    if (status != TX_SUCCESS)
    {
        LOGE("Create queue failed status=%d", status);
    }
    else
    {
        LOGI("MSG queue created");
    }

    while (1)
    {
        status = tx_queue_receive(&ctx.msg_que, &message, TX_WAIT_FOREVER);
        LOGD("Receive msg, action=%s, fid=%d, status=%d", act_str[message.action], message.fid, status);
        if (status == TX_SUCCESS)
        {
            switch (message.action)
            {
                case TA_E_ENROLL_START:
                    enroll_start(message.fid);
                    break;
                case TA_E_ENROLL_CONTINUE:
                    enroll_continue(message.fid);
                    break;
                case TA_E_ENROLL_CANCEL:
                    enroll_cancel(message.fid);
                    break;
                case TA_E_DETECT_START:
                    detect_start();
                    break;
                case TA_E_DETECT_CONTINUE:
                    detect_continue_start();
                    break;
                case TA_E_DETECT_STOP:
                    detect_stop();
                    break;
                case TA_E_DETECT_HAPPEN:
                    detect_happen();
                    break;
                default:
                    break;
            }
        }
    }

    ctx.thread_started = false;
}