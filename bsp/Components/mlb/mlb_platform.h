#ifndef __MLB_PLATFORM_H__
#define __MLB_PLATFORM_H__


#include "mlb_error_code.h"

#include "mlb_cfg.h"

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdarg.h>

/**
 * @brief 模块初始化
*/
MRT_T mlb_plat_init(void);


/**
 * @brief 平台日志打印
*/
void mlb_plat_log(const char *fmt, ...);
void mlb_plat_vlog(const char *fmt, va_list args);

/**
 * @brief 平台msleep
*/
void mlb_plat_msleep(uint32_t ms);

/**
 * @brief 平台获取时间戳
*/
uint32_t mlb_plat_get_timestamp(void);

/**
 * @brief 平台喂狗
*/
void mlb_plat_wdt_feed(void);

/**
 * @brief 平台内存分配
*/
void *mlb_plat_malloc(size_t sz);

/**
 * @brief 平台内存分配
*/
void *mlb_plat_realloc(void*ptr, size_t sz);

/**
 * @brief 平台内存释放
*/
void mlb_plat_free(void *ptr);


/**
 * SPI命令格式
 * spi_buf[insz + outsz]
 * |----in----|----out-----|
 * in一般都很小,可以使用fifo
 * out > 1024的时候可以是用dma读取
 */
/**
 * @brief 平台SPI先写后读
 * @param in 写入buf, [in = spi_buf]
 * @param insz 写入大小
 * @param out 读取buf, [out = ((uint8_t *)spi_buf) + insz]
 * @param outsz 读取大小
 * @return MRT_T
 * @retval MRT_OK spi传输成功
 * @retval MRT_SPI_ERROR spi传输失败
*/
MRT_T mlb_plat_spi_write_then_read(void *in, size_t insz, void *out, size_t outsz);

/**
 * @brief 平台SPI CS 拉高
*/
void mlb_plat_spi_cs_high(void);

/**
 * @brief 平台SPI CS 拉低
*/
void mlb_plat_spi_cs_low(void);

/**
 * @brief 平台flash block大小
 * @return block大小
*/
size_t mlb_plat_flash_block_size(void);

/**
 * @brief 获取flash可用空间大小
 * @return size_t
 */
size_t mlb_plat_flash_flash_size(void);

/**
 * @brief flash擦除
 * @param offset 擦除偏移, block对齐
 * @param size 擦除大小, block_size 对齐
 * @return MRT_T
 * @retval MRT_OK 擦除成功
 * @retval MRT_FLASH_ERASE Flash 擦除失败
 */
MRT_T mlb_plat_flash_block_erase(size_t offset, size_t size);

/**
 * @brief 平台flash 写入
 * @param offset 写入偏移, block对齐
 * @param buf 写入指针
 * @param size 写入大小, block_size 对齐
 * @return MRT_T
 * @retval MRT_OK 写入成功
 * @retval MRT_FLASH_WRITE Flash 写入失败
*/
MRT_T mlb_plat_flash_block_write(size_t offset, void *buf, size_t size);

/**
 * @brief flush
 * @return MRT_T
 * @retval MRT_OK 同步成功
 * @retval MRT_FLASH_SYNC Flash 同步失败
 */
MRT_T mlb_plat_flash_block_sync(void);

/**
 * @brief 平台flash 读取
 * @param offset 读取偏移, block对齐
 * @param buf 读取buf
 * @param size 读取大小, 非对齐
 * @return MRT_T
 * @retval MRT_OK 读取成功
 * @retval MRT_FLASH_READ Flash 读取失败
*/
MRT_T mlb_plat_flash_read(size_t offset, void *buf, size_t size);


#if defined(__cplusplus)
}
#endif

#endif


