#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "tx_api.h"

#include "mlb_intf.h"
#include "mlb_platform.h"

#define LOGD(fmt, ...) printf("[%s:%d] " fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__)
#define LOGE LOGD

#define FP_MAX_NUMBER           MLB_MAX_FP_NUMBER
#define FP_MAX_VIP              5
#define FP_MAX_ENROLL_SAMPLE    MLB_MAX_ENROLL_STEP
#define FP_ENROLL_DUPAREA_TH    80
#define FP_ENROLL_DUPAREA_START 2
#define FP_TPL_SIZE             (12 * 1024)

#define FP_TEST_MAX_NUMBER      2

static int enroll_count = 0;
static int enroll_step = 0;
static uint16_t enroll_fid = 0;
#define DETECT_STATE_NONE           0
#define DETECT_STATE_UP             (1 << 0)
#define DETECT_STATE_DOWN           (1 << 1)

#define ENROLL_CHECK_FOR_USER_FIDS
#define VERIFY_FOR_USER_FIDS

#define FP_TEST_DEFAULT_USER_ID     0

MRT_T test_mafp_run_init(void)
{
    MRT_T ret = MRT_OK;
    mlb_init_param_t param;
    mlb_sdk_info_t sdk_info;

    mlb_intf_get_sdk_info(&sdk_info);
    LOGD("mafp sdk version : %s", sdk_info.version);
    LOGD("mafp sdk commit id : %s", sdk_info.commit_id);

    ret = mlb_intf_platform_init();
    if(ret == MRT_OK) 
    {
        mlb_sensor_reg_e088n();
        param.sensor_type = MLB_SENSOR_TYPE_COATING;
        param.fp_tpl_size = FP_TPL_SIZE;
        param.max_fp_number = FP_MAX_NUMBER;
        param.max_enroll_steps = FP_MAX_ENROLL_SAMPLE;
        param.max_vip_cnt = FP_MAX_VIP;
        param.duplicate_area_th = FP_ENROLL_DUPAREA_TH;
        param.duplicate_area_start = FP_ENROLL_DUPAREA_START;
        param.algo_ram_level = 1;
        param.algo_time_level = 1;
        param.algo_far_leval = 1;
        param.algo_run_mode = 0;
        param.dbglvl = 2;
        ret = mlb_intf_init(&param);
        if(ret != MRT_OK)
        {
            LOGD("init fail");
            return ret;
        }
    }

    mlb_intf_set_active_user(FP_TEST_DEFAULT_USER_ID);

    mlb_intf_detect_mode_low_power();
    
    return ret;
}

MRT_T test_mafp_run_deinit(void)
{
    MRT_T ret = MRT_OK;

    ret = mlb_intf_deinit();
    if(ret != MRT_OK){
        LOGE("mlb_intf_deinit fail %d", ret);
        return ret;
    }

    return ret;
}

MRT_T test_read_chipid(void)
{
    MLB_CHIPID chipid;
    MRT_T ret = MRT_OK;

    extern MRT_T mlb_plat_init(void);
    mlb_plat_init();

    ret = mlb_intf_sensor_read_chipid(&chipid);
    if(ret != MRT_OK)
    {
        LOGE("err read chipid %x", ret);
        return ret;
    }
    LOGD("chipid %x", chipid);


    return MRT_OK;
}

MRT_T test_run_calibration(void)
{
    MRT_T ret = MRT_OK;

    ret = mlb_intf_calc_param();
    if(ret != MRT_OK){
        LOGE("mlb_intf_calc_param fail %d", ret);
        return ret;
    }

    return ret;
}

MRT_T test_run_enroll(uint16_t *fid)
{
    MRT_T ret = MRT_OK;
    size_t max_cnt;
    uint8_t is_fp_tpl_ok = 0;
    uint16_t tmp_fid= 0;
    uint32_t start_stamp;
    const uint32_t enroll_timeout_ms = 30 * 1000;

    mlb_intf_tpl_count(&enroll_count, &max_cnt);
    LOGD("enroll_count = %d, max_cnt = %d", enroll_count, max_cnt);

#ifdef ENROLL_CHECK_FOR_USER_FIDS
    uint16_t fids_list[MLB_MAX_FID_PER_USER];
    uint8_t fid_num;
    mlb_intf_get_current_user_fids(fids_list, &fid_num);
    if(fid_num >= MLB_MAX_FID_PER_USER)
    {
        LOGE("enroll count >= max fid per user");
        return MRT_USER_FID_FULL;
    }
#endif

    ret = mlb_intf_generate_new_fid(&tmp_fid);
    if(ret != MRT_OK)
    {
        LOGE("err generate fid %d", ret);
        return ret;
    }

    ret = mlb_intf_enroll_start(tmp_fid);
    if(ret != MRT_OK)
    {
        LOGE("err enroll start %d", ret);
        return ret;
    }

    LOGD("start to enroll fid %d", tmp_fid);
    start_stamp = mlb_plat_get_timestamp();
    while(1) {
        ret = mlb_intf_finger_detect();
        if(ret == MRT_OK)
        {
#ifdef ENROLL_CHECK_FOR_USER_FIDS
            ret = mlb_intf_enroll_update_and_check_duplicate(fids_list, fid_num, &is_fp_tpl_ok);
#else 
            ret = mlb_intf_enroll_update(1, &is_fp_tpl_ok);
#endif
            if(ret != MRT_OK)
            {
                if(ret == MRT_ENROLL_DUP_AREA)
                {
                    LOGE("MRT_ENROLL_DUP_AREA");
                }else if (ret == MRT_ENROLL_DUP_FINGER)
                {
                    LOGE("MRT_ENROLL_DUP_FINGER");
                }else {
                    LOGE("err enroll update %d", ret);
                }
            }
            
            if(is_fp_tpl_ok)
            {
                LOGD("commit");
                ret = mlb_intf_enroll_commit();
                if(ret != MRT_OK)
                {
                    LOGE("err enroll commit %d", ret);
                    return ret;
                }
                *fid = tmp_fid;
                ret = mlb_intf_set_user_fid(tmp_fid);
                if(ret != MRT_OK)
                {
                    LOGE("err set user fid %d", ret);
                    return ret;
                }
                break;
            }

            while(!mlb_intf_finger_detect_leave()) {
                mlb_plat_msleep(10);
            };
        }

        if(mlb_plat_get_timestamp() - start_stamp > enroll_timeout_ms)
        {
            LOGE("enroll timeout");
            ret = mlb_intf_enroll_discard();
            if(ret != MRT_OK)
            {
                LOGE("err enroll abort %d", ret);
            }
            return MRT_ENROLL_STATE_ERROR;
        }

        mlb_plat_msleep(10);
    }

    return MRT_OK;
}

MRT_T test_run_verify(void)
{
    MRT_T ret = MRT_OK;
    uint8_t need_update;
    uint16_t match_fid;
    uint8_t match_cnt = 3;
    uint16_t uid;
    uint16_t fids_list[MLB_MAX_FID_PER_USER];
    uint8_t fid_num, i;

    while(match_cnt) {
        ret = mlb_intf_finger_detect();
        if(ret == MRT_OK)
        {
            uint32_t tk = mlb_plat_get_timestamp();
            uid = mlb_intf_get_current_user();
            mlb_intf_get_current_user_fids(fids_list, &fid_num);
            if(fid_num == 0) 
            {
                LOGD("user %d no fid enrolled", uid);
                return MRT_OK;
            }

#ifdef VERIFY_FOR_USER_FIDS
            ret = mlb_intf_verify_start();
            if(ret != MRT_OK) 
            {
                LOGE("mlb_intf_verify_start fail %d", ret);
                return ret;
            }
            
            for(i = 0; i < fid_num; i++)
            {
                ret = mlb_intf_verify_fid(fids_list[i], &need_update);
                if(ret == MRT_OK)
                {
                    LOGD("uid %d match ok fid %d tm %d", uid, fids_list[i], mlb_plat_get_timestamp() - tk);
                    break;
                }
            }

            if(i == fid_num) 
            {
               LOGD("uid %d match fail tm %d", uid, mlb_plat_get_timestamp() - tk);
            }
#else
            ret = mlb_intf_identify_all(&match_fid, &need_update);
            if(ret == MRT_OK)
            {
                for(i = 0; i < fid_num; i++)
                {
                    if(match_fid == fids_list[i])
                    {
                        LOGD("uid %d match ok fid %d tm %d", uid, match_fid, mlb_plat_get_timestamp() - tk);
                        break;
                    }
                }
                if(i == fid_num) {
                    LOGD("match id not searched in uid %d, tm %d", uid, mlb_plat_get_timestamp() - tk);
                }
            }else {
                LOGD("uid %d match fail tm %d", uid, mlb_plat_get_timestamp() - tk);
            }
#endif

            while(!mlb_intf_finger_detect_leave()) {
                mlb_plat_msleep(10);
            };

            match_cnt--;
        }

        mlb_plat_msleep(10);
    }

    return MRT_OK;
}

MRT_T test_run_enumerate(void)
{
    MRT_T ret = MRT_OK;
    uint16_t uid, i;
    uint16_t fids_list[MLB_MAX_FID_PER_USER];
    uint8_t fid_num;

    uid = mlb_intf_get_current_user();
    ret = mlb_intf_get_current_user_fids(fids_list, &fid_num);
    if(ret != MRT_OK)
    {
        LOGE("mlb_intf_get_current_user_fids fail %d", ret);
        return ret;
    }

    for(i = 0; i < fid_num; i++)
    {
        ret = mlb_intf_check_fid_used(fids_list[i]);
        if(ret == MRT_TPL_FID_USED){
            LOGD("user %d fid %d", uid, fids_list[i]);
        }
    }

    return MRT_OK;
}

MRT_T test_run_delete(uint16_t fid)
{
    MRT_T ret = MRT_OK;
    uint16_t fids_list[MLB_MAX_FID_PER_USER];
    uint8_t fid_num, i;

    ret = mlb_intf_delete_user_fid(fid);
    if(ret != MRT_OK){
        LOGE("remove fid fail %d", ret);
        return ret;
    }

    return MRT_OK;
}

MRT_T test_run_navigation(void)
{
    MRT_T ret = MRT_OK;
    uint32_t navi_result = NAVI_NONE;
    const uint32_t expected_resutl = NAVI_DOWN | NAVI_RIGHT | NAVI_UP | NAVI_LEFT;
    uint32_t test_resutl = 0;


    ret = mlb_intf_navigation_init();
    if(ret != MRT_OK)
    {
        LOGE("mlb_intf_navigation_init fail %d", ret);
        return ret;
    }

    while(test_resutl != expected_resutl) {
        ret = mlb_intf_navigation_tap(&navi_result);
        if(ret != MRT_OK)
        {
            LOGE("mlb_intf_navigation_tap fail %d", ret);
            return ret;
        }

        if(navi_result == NAVI_TAP_UP)
        {   
            ret = mlb_intf_navigation_direction(&navi_result);
            if(ret != MRT_OK)
            {
                LOGE("mlb_intf_navigation_direction fail %d", ret);
                return ret;
            } 
            switch(navi_result) {
                case NAVI_DOWN:
                    LOGD("navi_result NAVI_DOWN");
                    break;
                case NAVI_UP:
                    LOGD("navi_result NAVI_UP");
                    break;
                case NAVI_RIGHT:
                    LOGD("navi_result NAVI_RIGHT");
                    break;
                case NAVI_LEFT:
                    LOGD("navi_result NAVI_LEFT");
                    break;
                case NAVI_NONE:
                    LOGD("navi_result NAVI_NONE");
                    break;
            }
            
            if(navi_result != NAVI_NONE) 
            {
                test_resutl |= navi_result;
            }
            
            navi_result = NAVI_NONE;
        }
    }

    ret = mlb_intf_navigation_deinit();
    if(ret != MRT_OK)
    {
        LOGE("mlb_intf_navigation_deinit fail %d", ret);
        return ret;
    }

    return MRT_OK;
}

MRT_T test_run_switch_user(uint16_t uid)
{
    MRT_T ret = MRT_OK;
    uint16_t fids_list[MLB_MAX_FID_PER_USER];
    uint8_t fid_num;
    uint16_t uids_list[MLB_USER_MAX_NUM];
    uint16_t uid_num;

    LOGD("switch user %d to %d", mlb_intf_get_current_user(), uid);

    ret = mlb_intf_set_active_user(uid);
    if(ret != MRT_OK)
    {
        LOGE("mlb_intf_set_active_user fail %d", ret);
        return ret;
    }

    ret = mlb_intf_get_current_user_fids(fids_list, &fid_num);
    if(ret != MRT_OK)
    {
        LOGE("mlb_intf_get_current_user_fids fail %d", ret);
        return ret;
    }
    
    LOGD("curren user %d, has enrolled finger: %d", mlb_intf_get_current_user(), fid_num);

    for(int i = 0; i < fid_num; i++)
    {
        LOGD("fid %d", fids_list[i]);
    }

    mlb_intf_get_all_users(uids_list, &uid_num);
    LOGD("total user num %d", uid_num);
    for(int i = 0; i < uid_num; i++)
    {
        LOGD("uid %d", uids_list[i]);
    }

    return MRT_OK;
}

MRT_T test_run_delete_user(uint16_t uid)
{
    MRT_T ret = MRT_OK;
    uint16_t uids_list[10];
    uint16_t uid_num;

    LOGD("cur user %d delete user %d", mlb_intf_get_current_user(), uid);

    ret = mlb_intf_delete_user(uid);
    if(ret != MRT_OK)
    {
        LOGE("mlb_intf_delete_user fail %d", ret);
        return ret;
    }

    mlb_intf_get_all_users(uids_list, &uid_num);
    LOGD("total user num %d", uid_num);
    for(int i = 0; i < uid_num; i++)
    {
        LOGD("uid %d", uids_list[i]);
    }

    return MRT_OK;
}

extern TX_SEMAPHORE g_fp_irq_sem;
uint8_t g_timer_fresh_flag = 0;
extern void interrupt_enable(uint8_t enable);
extern MRT_T mlb_comm_n_set_mode(uint8_t mode);
#define REFRESH_INTERVAL (5 * 1000)
static void mlb_refresh_timer_callback(ULONG id)
{
    (void)id;

    /* ¶¨Ê±µ÷ÓÃË¢ÐÂÖÐ¶Ï²ÎÊý£¬easy_trigger=0£¨¶¨Ê±Ë¢ÐÂ£© */
    printf("fresh int\r\n");
    g_timer_fresh_flag = 1;
    tx_semaphore_put(&g_fp_irq_sem);

}

static void error_trigger_count(int press) {
	static int trigger_count = 0;

	if (press == MRT_SENSOR_FDT_NO_DET) {
		trigger_count++;
	} else {
		trigger_count = 0;
	}
	if (trigger_count > 50) {
        printf("reset int paras\r\n");
        mlb_intf_priv_refresh_interrupt(0);
        trigger_count = 0;
	}
}

MRT_T test_run_interrupt(void)
{
    uint8_t status;
    int ret = MRT_OK;
    static TX_TIMER s_mlb_refresh_timer;

    status = tx_timer_create(&s_mlb_refresh_timer,
                                     "mlb_refresh",
                                     mlb_refresh_timer_callback,
                                     0,
                                     REFRESH_INTERVAL,   /* initial ticks */
                                     REFRESH_INTERVAL,   /* reschedule ticks */
                                     TX_AUTO_ACTIVATE);
    if (status != TX_SUCCESS) {
        printf("create refresh timer FAIL %u\r\n", status);
    }

    while(1) {
        
        mlb_intf_detect_mode_low_power();
        interrupt_enable(1);
        printf("wait IRQ ...\r\n");
        status = tx_semaphore_get(&g_fp_irq_sem, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS) {
            printf("wait semaphore failed/timeout, status=%u\r\n", status);
        }

        //timer refresh 
        if(g_timer_fresh_flag) {
            g_timer_fresh_flag = 0;
            interrupt_enable(0);
            mlb_intf_priv_refresh_interrupt(1);
        }

        ret = mlb_intf_finger_detect();
        //error trigger refresh
        error_trigger_count(ret);

        //if generate refresh error parameter,comment this line
        while(mlb_intf_finger_detect_leave() != MRT_SENSOR_FDT_NO_DET);
    }

    return 0;
}

static MRT_T mlb_test_work()
{
    MRT_T ret = MRT_OK;
    uint8_t is_fp_tpl_ok = 0;
    uint16_t match_fid;
    uint8_t need_update;
    static uint8_t detect_state = DETECT_STATE_NONE;
    if(detect_state == DETECT_STATE_NONE)
    {

        if(enroll_count < FP_TEST_MAX_NUMBER)
        {
            if(enroll_step == 0)
            {
                ret = mlb_intf_generate_new_fid(&enroll_fid);
                if(ret != MRT_OK)
                {
                    LOGE("err generate fid %d", ret);
                    return ret;
                }
                ret = mlb_intf_enroll_start(enroll_fid);
                if(ret != MRT_OK)
                {
                    LOGE("err enroll start %d", ret);
                    return ret;
                }
            }
        }
        ret = mlb_intf_finger_detect_leave();
        if(ret == MRT_SENSOR_FDT_NO_DET)
        {
            detect_state = DETECT_STATE_UP;
        }
    }else if (detect_state == DETECT_STATE_UP)
    {
        ret = mlb_intf_finger_detect();
        if(ret == MRT_OK)
        {
            detect_state = DETECT_STATE_DOWN;
        }
    }else if (detect_state == DETECT_STATE_DOWN)
    {
        if(enroll_count < FP_TEST_MAX_NUMBER)
        {
            ret = mlb_intf_enroll_update(1, &is_fp_tpl_ok);
            if(ret != MRT_OK)
            {
                if(ret == MRT_ENROLL_DUP_AREA)
                {
                    LOGE("MRT_ENROLL_DUP_AREA");
                    detect_state = DETECT_STATE_NONE;
                    return MRT_OK;
                }else if (ret == MRT_ENROLL_DUP_FINGER)
                {
                    LOGE("MRT_ENROLL_DUP_FINGER");
                    detect_state = DETECT_STATE_NONE;
                    return MRT_OK;
                }else {
                    LOGE("err enroll update %d", ret);
                    return ret;
                }
            }
            
            enroll_step++;

            if(is_fp_tpl_ok)
            {
                LOGD("commit");
                ret = mlb_intf_enroll_commit();
                if(ret != MRT_OK)
                {
                    LOGE("err enroll commit %d", ret);
                    return ret;
                }
                enroll_count++;
                enroll_step = 0;
            }
        }else {
            uint32_t tk = mlb_plat_get_timestamp();
            ret = mlb_intf_identify_all(&match_fid, &need_update);
            if(ret == MRT_OK)
            {
                LOGD("match ok fid %d tm %d", match_fid, mlb_plat_get_timestamp() - tk);
            }else {
                LOGD("match fail tm %d", mlb_plat_get_timestamp() - tk);
            }
        }
        detect_state = DETECT_STATE_NONE;
    }
    return MRT_OK;
}

void mlb_test_run()
{
    MRT_T ret = MRT_OK;

    ret = test_mafp_run_init();
    if(ret != MRT_OK){
        LOGE("test_run_init fail %d", ret);
        return;
    }

    size_t max_cnt;
    mlb_intf_tpl_count(&enroll_count, &max_cnt);

    while(1)
    {
        if(mlb_test_work() != MRT_OK)
        {
            LOGD("Test fail");
            break;
        }
    }


}