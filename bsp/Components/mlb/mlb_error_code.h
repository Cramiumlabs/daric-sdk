
#ifndef __MLB_ERROR_CODE_H__
#define __MLB_ERROR_CODE_H__
#include "mlb_cfg.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t MRT_T;

#define MRT_OK                                              (    0)  // 执行成功

#define MRT_COM_ERROR                                       (   -1)  // 通用错误
#define MRT_INVALID_PARM                                    (   -2)  // 无效的入参
#define MRT_MALLOC_FAILED                                   (   -3)  // 内存分配失败
#define MRT_NOT_SUPPORTED                                   (   -4)  // 不支持
#define MRT_FS_NOSPAC                                       (   -5)  // 文件系统空间不足
#define MRT_FS_CORRUPT                                      (   -6)  // 文件系统错误
#define MRT_FS_CRC_FAILED                                   (   -7)  // 文件CRC校验失败
#define MRT_INTF_NO_INIT                                    (   -8)  // 接口尚未初始化
#define MRT_FLASH_SMALL                                     (   -9)  // Flash空间不足
#define MRT_FLASH_ERASE                                     (  -10)  // Flash擦除失败
#define MRT_FLASH_WRITE                                     (  -11)  // Flash写入失败
#define MRT_FLASH_READ                                      (  -12)  // Flash读取失败
#define MRT_FLASH_SYNC                                      (  -13)  // Flash同步失败

#define MRT_SPI_ERROR                                       ( -101)  // SPI通信错误
#define MRT_SPI_GET_REG_FAIL                                ( -102)  // 读取寄存器失败
#define MRT_SPI_GET_CHIPID_ERR                              ( -103)  // 读取ID失败

#define MRT_SENSOR_NO_LOAD                                  ( -201)  // 芯片没有加载
#define MRT_SENSOR_NO_INIT_PARAM                            ( -202)  // 芯片没有校准
#define MRT_SENSOR_CALI_CTL_FAIL                            ( -203)  // 芯片电压校准失败
#define MRT_SENSOR_CALI_DETCTL_FAIL                         ( -204)  // 芯片detect电压校准失败
#define MRT_SENSOR_DEAD_PIX_FAIL                            ( -205)  // 芯片坏点检测不过
#define MRT_SENSOR_FDT_NO_DET                               ( -206)  // 未检测到手指
#define MRT_SENSOR_FDT_NOT_FP                               ( -207)  // 非手指类,包括水雾,湿纸巾等非手指
#define MRT_SENSOR_FDT_WET                                  ( -208)  // 手指太乱,包括湿手指
#define MRT_SENSOR_CALI_LINE_FAIL                           ( -209)  // 芯片校准链校准失败
#define MRT_SENSOR_CALI_FAIL                                ( -210)  // 芯片校准失败
#define MRT_SENSOR_FDT_FAST                                 ( -211)  // 手指按压太快

#define MRT_BLK_MOUNT_FAIL                                  ( -301)  //FLASH块mount失败
#define MRT_BLK_CHECK_FAIL                                  ( -302)  //FLASH块检查失败
#define MRT_BLK_CRC_FAIL                                    ( -303)  //FLASH块CRC校验失败
#define MRT_BLK_FULL                                        ( -304)  //FLASH块满
#define MRT_BLK_FILE_NOEXIST                                ( -305)  //FLASH文件不存在
#define MRT_BLK_FILE_CNT_RHMAX                              ( -306)  //FLASH文件超出最大文件数量

#define MRT_TPL_FULL                                        ( -401)  // 指纹模板已满
#define MRT_TPL_FID_USED                                    ( -402)  // 指纹ID已经使用
#define MRT_TPL_FID_NOEXIST                                 ( -403)  // 指纹ID不存在
#define MRT_TPL_UPDATE_STATE_INVALID                        ( -404)  // 指纹更新模板状态错误, 没有有效匹配结果
#define MRT_TPL_UPDATE_NOCHANGE                             ( -405)  // 指纹模板未更新
#define MRT_TPL_UPDATE_FAIL                                 ( -406)  // 指纹模板更新错误


#define MRT_ENROLL_STATE_ERROR                              ( -501)  // 注册状态错误
#define MRT_ENROLL_TMP_TPL_FULL                             ( -502)  // 注册临时模板已满
#define MRT_ENROLL_DUP_AREA                                 ( -503)  // 注册重复区域
#define MRT_ENROLL_DUP_FINGER                               ( -504)  // 注册重复手指
#define MRT_IDENTIFY_FAIL                                   ( -505)  // 匹配失败

#define MRT_USER_FULL                                       ( -601)  // 用户已满
#define MRT_USER_NOT_FOUND                                  ( -602)  // 用户不存在
#define MRT_USER_FID_FULL                                   ( -603)  // 用户指纹已满
#define MRT_USER_FID_NOT_FOUND                              ( -604)  // 用户指纹不存在
#define MRT_USER_DELETE_ERROR                               ( -605)  // 用户删除错误

#define MRT_ALGO_ERROR                                      (-1000)  // 算法调用失败



#ifdef __cplusplus
}
#endif

#endif


