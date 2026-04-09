#ifndef __MLB_INTF_H__
#define __MLB_INTF_H__

#include "mlb_error_code.h"

#if defined(__cplusplus)
extern "C" {
#endif

//MLB 接口
typedef enum{
    MLB_CHIPID_UNKNOW   = 0,
    MLB_CHIPID_E036N    = 0x03678,
    MLB_CHIPID_E064N    = 0x06478,
    MLB_CHIPID_E088N    = 0x08878,
    MLB_CHIPID_E160N    = 0x16078,
    MLB_CHIPID_E256N    = 0x25678,
    MLB_CHIPID_E256P    = 0x25680,
    MLB_CHIPID_E112P    = 0x11280,
    MLB_CHIPID_E088P    = 0x08880,
    
}MLB_CHIPID;

enum navigation_result {
    NAVI_NONE          = 0,
    NAVI_LEFT          = 1,
    NAVI_DOWN          = 1 << 1,
    NAVI_RIGHT         = 1 << 2,
    NAVI_UP            = 1 << 3,
    NAVI_TAP_DOWN      = 1 << 4,
    NAVI_TAP_UP        = 1 << 5,
};

//Sensor类型
typedef enum
{
    MLB_SENSOR_TYPE_COATING = 0,  // Coating
    MLB_SENSOR_TYPE_COVER = 1     // 盖板
}mlb_sensor_type_t;


//图像参数
typedef struct 
{
    uint16_t img_width; //宽
    uint16_t img_height;//高
    uint16_t img_bit;   //1pix占用字节数
    uint16_t org_img_bit;//原图1pix占用字节数
}mlb_img_info_t;

#define MLB_USER_DATA_FLASH_OFFSET (0xffffffff)

#define MLB_USER_MAX_NUM     3
#define MLB_MAX_FID_PER_USER 5

#define MLB_MAX_ENROLL_STEP     10
#define MLB_MAX_FP_NUMBER       (MLB_USER_MAX_NUM * MLB_MAX_FID_PER_USER)

/**
 * @brief 注册platform接口,第一个需要调用
 * @param sdk_info 
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_platform_init(void);


typedef struct 
{
   const char *version;//sdk 版本
   const char *commit_id;//sdk 版本记录
}mlb_sdk_info_t;

/**
 * @brief 获取SDK信息, 
 * @param sdk_info 
 * @return MRT_T
 */
MLB_CALL MRT_T mlb_intf_get_sdk_info(mlb_sdk_info_t *sdk_info);

/**
 * @brief 注册sensor E112P
 */
MLB_CALL void mlb_sensor_reg_e112p(void);

/**
 * @brief 注册sensor E088P
 */
MLB_CALL void mlb_sensor_reg_e088p(void);

/**
 * @brief 注册sensor
 */
MLB_CALL void mlb_sensor_reg_e036n(void);
MLB_CALL void mlb_sensor_reg_e064n(void);
MLB_CALL void mlb_sensor_reg_e088n(void);
MLB_CALL void mlb_sensor_reg_e256p(void);
MLB_CALL void mlb_sensor_reg_e256n(void);

/**
 * @brief 注册sensor E160N
 */
MLB_CALL void mlb_sensor_reg_e160n(void);

/**
 * @brief 注册空设备, 将图像传入SDK来注册匹配
 */
MLB_CALL void mlb_sensor_reg_simulator(void);

//SDK初始化参数
typedef struct 
{
    mlb_sensor_type_t sensor_type;      //Sensor 类型
    
    size_t fp_tpl_size;                 //单个手指指纹占用Flash空间, 2K, 4K, 8K, 12K, 16K, 20K,
    size_t max_fp_number;               //指纹支持最大手指数量
    size_t max_enroll_steps;            //最大注册次数,实际注册次数可以少于最大次数
    size_t max_vip_cnt;                 //最大优先匹配个数, 最近匹配成功会加入vip, 默认5
    uint8_t duplicate_area_th;          //允许的重复区域阈值 0 - 100, 值越小越严格, 默认80
    uint8_t duplicate_area_start;       //第几次开始检查重复区域

    uint8_t algo_ram_level;             // LOW = 0, NORMAL = 1
    uint8_t algo_time_level;            // SLOW = 0, NORMAL = 1
    uint8_t algo_far_leval;             // 50K = 0, 100K = 1, 200K = 2, 500K = 3, 1M = 4, 5M = 5, 10M = 6
    uint8_t algo_run_mode;              // NORAML = 0, DSM = 1
    uint8_t is_fp_temp_use_ram;         // 使用RAM空间来存储临时指纹模板
    uint8_t dbglvl;                     // 0: 不打印log, 1: 打印error log, 2: 打印error和debug log
    uint8_t reserved;            
}mlb_init_param_t;

/**
 * @brief SDK初始化函数
 * @param init_parma 初始化参数
 * @return MRT_T
 */
MLB_CALL MRT_T mlb_intf_init(mlb_init_param_t *init_param);

/**
 * @brief SDK 释放资源
 */
MLB_CALL MRT_T mlb_intf_deinit(void);


/**
 * @brief 获取图像信息
 * @param info 图像信息
 * @return MRT_T
*/
MLB_CALL MRT_T mlb_intf_get_image_info(mlb_img_info_t *info);

/**
 * @brief Sensor DETECT 中断模式, 上升沿触发
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_detect_mode(void);
/**
 * @brief Sensor DETECT 中断模式, 上升沿触发(最低功耗模式)
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_detect_mode_low_power(void);

MLB_CALL MRT_T mlb_intf_power_down_mode(void);


/**
 * @brief 检测手指按压
 * @retval MRT_OK 手指按压
 * @retval MRT_SENSOR_FDT_NO_DET 手指未按压
 */
MLB_CALL MRT_T mlb_intf_finger_detect(void);

/**
 * @brief 检测手指抬起
 * @return MRT_T
 * @retval MRT_OK 手指按压
 * @retval MRT_SENSOR_FDT_NO_DET 手指未按压
 */
MLB_CALL MRT_T mlb_intf_finger_detect_leave(void);


/**
 * @brief 获取finger_detect图像数据
 * @param img img指针, 大小(img_width * img_height * img_bit)
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_get_img(void *img);

/**
 * @brief 获取finger_detect图像数据指针
 * @param img img指针, 大小(img_width * img_height * img_bit)
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_get_pimg(void **img);

/**
 * @brief 传入图像,用来注册或者匹配
 * 
 * @param img 图像指针
 * @param info 图像信息
 * E112P img_width = 96, img_height = 112, img_bit = 1, org_img_bit = 2
 * @return MLB_CALL 
 */
MLB_CALL MRT_T mlb_intf_set_img(void *img, mlb_img_info_t *info);

/**
 * @brief 芯片校准
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_calc_param(void);

/**
 * @brief 申请空闲的指纹id
 * @param fid 空闲的指纹ID
 * @return MRT_T
 * @retval MRT_OK 生成OK
 * @retval MRT_TPL_FULL 模板已满
 */
MLB_CALL MRT_T mlb_intf_generate_new_fid(uint16_t *fid);

/**
 * @brief 检查fid是否被使用
 * @param fid 
 * @return MRT_T 
 * @retval MRT_OK fid未被使用
 * @retval MRT_TPL_FID_USED fid已被占用
 */
MLB_CALL MRT_T mlb_intf_check_fid_used(uint16_t fid);

/**
 * @brief 注册开始
 * @param fid 空闲的指纹ID
 * @return MRT_T
 * @retval MRT_OK 注册开始OK
 * @retval MRT_TPL_FID_USED 指纹ID已经使用
 */
MLB_CALL MRT_T mlb_intf_enroll_start(uint16_t fid);

/**
 * @brief 将按压的图像注册到临时模板中
 * @param fp_duplicate (=0)不检查重复手指;(=1)检查重复手指, 与指纹库所有指纹查重复
 * @param is_fp_tpl_ok 单个手指的模板数量足够
 * @return MRT_T
 * @retval MRT_OK 注册单步OK
 * @retval MRT_ENROLL_DUP_AREA 重复区域
 * @retval MRT_ENROLL_DUP_FINGER 重复手指
 */
MLB_CALL MRT_T mlb_intf_enroll_update(uint8_t fp_duplicate, uint8_t *is_fp_tpl_ok);

/**
 * @brief 同mlb_intf_enroll_update, 并与指定的fids列表做查重
 * @param fids 待查重的fid列表
 * @param fids_len fid列表长度
 * @param is_fp_tpl_ok 单个手指的模板数量足够
 * @return MRT_T 
 * @retval MRT_OK 注册单步OK
 * @retval MRT_ENROLL_DUP_AREA 重复区域
 * @retval MRT_ENROLL_DUP_FINGER 重复手指
 */
MLB_CALL MRT_T mlb_intf_enroll_update_and_check_duplicate(uint16_t *fids, size_t fids_len, uint8_t *is_fp_tpl_ok);

/**
 * @brief 提交注册
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_enroll_commit(void);

/**
 * @brief 取消注册
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_enroll_discard(void);

/**
 * @brief 匹配所有指纹
 * @param fid 匹配上的指纹
 * @param need_update 指纹模板匹配成功同时指纹模板待更新
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_identify_all(uint16_t *fid, uint8_t *need_update);

/**
 * @brief 单次匹配开始, 只能调用一次
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_verify_start(void);
/**
 * @brief 单次匹配指定指纹
 * @param fid 指定需要匹配的fid
 * @param need_update 指纹模板匹配成功同时指纹模板待更新
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_verify_fid(uint16_t fid, uint8_t *need_update);

/**
 * @brief 指纹模板学习, 当need_update为1的时候调用
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_study(void);

/**
 * @brief 删除指定指纹
 * @param fid 待删除的指纹id
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_tpl_remove(uint16_t fid);

/**
 * @brief 批量删除指定指纹
 * @param fids 待删除的指纹id列表
 * @param count 待删除的指纹数量
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_tpl_remove_fids(uint16_t *fids, uint16_t count);

/**
 * @brief 清空指纹库
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_tpl_remove_all(void);

/**
 * @brief 枚举指纹
 * @param fids 指纹id buffer, 类型uint16_t, 当fids为空的时候只返回real_len
 * @param fids_len 指纹id个数
 * @param real_len 实际指纹id个数
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_tpl_enumerate(uint16_t *fids, size_t fids_len, size_t *real_len);

/**
 * @brief 获取指纹个数
 * @param count 已经存在的指纹个数
 * @param max_count 最大可以注册的指纹个数
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_tpl_count(size_t *count, size_t *max_count);

/**
 * @brief 读取芯片id
 * @param chipid 读出的chipid
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_sensor_read_chipid(MLB_CHIPID *chipid);

/**
 * @brief 设置当前用户
 * @param uid 用户id
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_set_active_user(uint16_t uid);


/**
 * @brief 获取当前用户uid
 * @return  uid 用户id
 */
MLB_CALL MRT_T mlb_intf_get_current_user(void);

/**
 * @brief 获取所有用户uid
 * @param uids 用户uid列表
 * @param uid_num 用户uid数量
 * @return  void
 */
void mlb_intf_get_all_users(uint16_t *uids, uint16_t *uid_num);

/**
 * @brief 获取当前用户fids
 * @param fids 指纹id列表
 * @param fid_num 指纹id数量
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_get_current_user_fids(uint16_t *fids, uint8_t *fid_num);

/**
 * @brief 获取某个用户fids
 * @param uid  用户id
 * @param fids 指纹id列表
 * @param fid_num 指纹id数量
 * @return MRT_T 
 */
MRT_T mlb_intf_get_a_user_fids(uint16_t uid, uint16_t *fids, uint8_t *fid_num);

/**
 * @brief 设置当前用户的指纹id
 * @param fid 指纹id
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_set_user_fid(uint16_t fid);

/**
 * @brief 删除当前用户的指纹id
 * @param fid 指纹id
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_delete_user_fid(uint16_t fid);

/**
 * @brief 删除当前用户的所有指纹id
 * @return MRT_T 
 */
MRT_T mlb_intf_delete_user_all_fids();

/**
 * @brief 删除用户
 * @param uid 用户id
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_delete_user(uint16_t uid);

/**
 * @brief 导航初始化
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_navigation_init(void);

/**
 * @brief 导航结束
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_navigation_deinit(void);

/**
 * @brief 导航 Tap
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_navigation_tap(uint32_t *navi_result);

/**
 * @brief 导航方向
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_navigation_direction(uint32_t *navi_result);

/**
 * @brief 刷新中断参数
 * @param easy_trigger 1: 定时刷新，计算的参数比原有参数小则更新; 0:误触发时调用，会检测手指是否按压，按压时参数不更新; 
 * @return MRT_T 
 */
MLB_CALL MRT_T mlb_intf_priv_refresh_interrupt(int easy_trigger);

#if defined(__cplusplus)
}
#endif

#endif

