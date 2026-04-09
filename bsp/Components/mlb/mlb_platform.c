#include "mlb_platform.h"
#include "mlb_intf.h"
#include <stdarg.h>
#include <stdio.h>

#include "daric_gpio.h"
#include "daric_hal_def.h"
#include "daric_hal_spim.h"
#include "daric_udma_spim_v3.h"
#include "daric_hal_pinmap.h"
#include "daric_hal_gpio.h"
#include "tx_api.h"
#include "daric_filex_app.h"

#define SPI_ID       CONFIG_FP_SPI_ID
#define SPI_CS_GROUP CONFIG_FP_SPI_PORT
#define SPI_CS_PIN   CONFIG_FP_SPI_CS

static SPIM_HandleTypeDef s_hspim;

// PINMAP_InitTypeDef fp_spim1[] = {
//     {PORT_C, PIN_1, AF2_PC1_SPIM2_SD0},    // Configure PC7
//     {PORT_C, PIN_2, AF2_PC2_SPIM2_SD1},    // Configure PC8
//     {PORT_C, PIN_0, AF2_PC0_SPIM2_CLK},  // Configure PC11
//     {PORT_C, PIN_3, AF2_PC3_SPIM2_CSN0}, // Configure PC12
// };

#define MLB_MALLOC_USE_C_LIB

#ifndef MLB_MALLOC_USE_C_LIB
#define MLB_MALLOC_DEBUG    0
#if MLB_MALLOC_DEBUG
static size_t max_cursor = 0;
#endif

#define MLB_SDK_BUF_SZ (108 * 1024)
#define MLB_SDK_BUF_ALIGN	(8)

static uint8_t mlb_sdk_buf[MLB_SDK_BUF_SZ] __attribute__((aligned(MLB_SDK_BUF_ALIGN)));
static size_t mlb_sdk_buf_cursor = 0;
static size_t mlb_sfi_start_addr = 0; 
#endif 

#define MLB_FS_DATA_DIR                 "\\fpdata"
#define MLB_FS_PROPERTY_FILE            "%s\\mlb_sensor_prop_tpl_data"
#define MLB_USER_FID_FILE               "%s\\mlb_sensor_user_list"
//ponit to disk which data stored
FX_MEDIA *gmlb_fp_Disk = &gNandflashDisk;

/**
 * @brief 模块初始化
*/
MRT_T mlb_plat_init(void)
{
    UINT status;
    char fileName[128];

    // HAL_PINMAP_init(fp_spim1, sizeof(fp_spim1) / sizeof(fp_spim1[0]));
    s_hspim.baudrate   = 5000000;
    s_hspim.id         = SPI_ID;
    s_hspim.cs_gpio    = -1;
    s_hspim.cs = 0;
    s_hspim.qspi = 0;
    s_hspim.polarity = SPI_CMD_CFG_CPOL_NEG;
    s_hspim.phase = SPI_CMD_CFG_CPHA_OPP;
    s_hspim.big_endian = 1; // 1 means SPI_CMD_MSB_FIRST
    s_hspim.wordsize = SPIM_WORDSIZE_8;
    if (HAL_OK != HAL_SPIM_Init(&s_hspim)) {
        printf("FP_SPI_Init failed!\n");
    }
    
    GPIO_InitTypeDef GPIO_InitStruct1 = {0};
    GPIO_InitStruct1.Pin = SPI_CS_PIN;
    GPIO_InitStruct1.Mode = GPIO_MODE_OUTPUT;
    GPIO_InitStruct1.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SPI_CS_GROUP, &GPIO_InitStruct1);
    HAL_GPIO_WritePin(SPI_CS_GROUP, SPI_CS_PIN, GPIO_PIN_RESET);

    //create directory for fp data 
    //daricDataDiskLoad();
    // status = daricDataDiskFormat();
    // if (status == FX_SUCCESS) {
    //     printf("format filesystem test SUCCESS\r\n");
    //     fx_media_flush(gmlb_fp_Disk);
    // }
    status = fx_directory_create(gmlb_fp_Disk, MLB_FS_DATA_DIR);
    if (status == FX_SUCCESS) {
        status = fx_media_flush(gmlb_fp_Disk);
        if (status != FX_SUCCESS) {
            printf("file flush FAIL\r\n");
            return MRT_FS_CORRUPT;
        }
    } else if(status == FX_ALREADY_CREATED || status == FX_NO_MORE_SPACE) {
        printf("directory already created\r\n");
    } else {
        printf("create dir FAIL %d\r\n", status);
        return MRT_FS_CORRUPT;
    }
    memset(fileName, 0, 128);
    snprintf(fileName, 128, MLB_FS_PROPERTY_FILE, MLB_FS_DATA_DIR);
    status = fx_file_create(gmlb_fp_Disk, fileName);
    if (status != FX_SUCCESS && status != FX_ALREADY_CREATED && status != FX_NO_MORE_SPACE) {
        printf("fx_file_create FAIL\r\n");
        return MRT_FLASH_WRITE;
    } else {
        printf("file create SUCCESS or already exist\r\n");
        status = fx_media_flush(gmlb_fp_Disk);
        if (status != FX_SUCCESS) {
            printf("file flush FAIL\r\n");
            return MRT_FS_CORRUPT;
        } 
    }
    memset(fileName, 0, 128);
    snprintf(fileName, 128, MLB_USER_FID_FILE, MLB_FS_DATA_DIR);
    status = fx_file_create(gmlb_fp_Disk, fileName);
    if (status != FX_SUCCESS && status != FX_ALREADY_CREATED && status != FX_NO_MORE_SPACE) {
        printf("fx_file_create FAIL\r\n");
        return MRT_FLASH_WRITE;
    } else {
        printf("file create SUCCESS or already exist\r\n");
        status = fx_media_flush(gmlb_fp_Disk);
        if (status != FX_SUCCESS) {
            printf("file flush FAIL\r\n");
            return MRT_FS_CORRUPT;
        } 
    }

    return MRT_OK;
}

/**
 * @brief 平台日志打印
*/
void mlb_plat_log(const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    size_t len = (size_t)vsnprintf(NULL, 0, fmt, args);
    char *buf = mlb_plat_malloc(len + 3);
    vsnprintf(buf, len + 3, fmt, args);
    strcat(buf, "\r\n");
    printf(buf, strlen(buf));
    mlb_plat_free(buf);
    va_end (args);
}
void mlb_plat_vlog(const char *fmt, va_list args)
{
    size_t len = (size_t)vsnprintf(NULL, 0, fmt, args);
    char *buf = mlb_plat_malloc(len + 3);
    vsnprintf(buf, len + 3, fmt, args);
    strcat(buf, "\r\n");
    printf(buf, strlen(buf));
    mlb_plat_free(buf);
}

/**
 * @brief 平台msleep
*/
void mlb_plat_msleep(uint32_t ms)
{
	tx_thread_sleep(ms);
}

/**
 * @brief 平台获取时间戳
*/
uint32_t mlb_plat_get_timestamp(void)
{
    return tx_time_get();
}

/**
 * @brief 平台喂狗
*/
void mlb_plat_wdt_feed(void)
{
    HAL_WDG_FeedDog();
}
/**
 * @brief 平台内存分配
*/
void *mlb_plat_malloc(size_t sz)
{
#ifdef MLB_MALLOC_USE_C_LIB
    void * ptr = malloc(sz);
    return ptr;
#else
    size_t rl_sz = sz + sizeof(size_t);
    rl_sz = (rl_sz + MLB_SDK_BUF_ALIGN - 1) / MLB_SDK_BUF_ALIGN * MLB_SDK_BUF_ALIGN;
    uint8_t *ptr = mlb_sdk_buf + mlb_sdk_buf_cursor;
    size_t *ptr_sz = (size_t *)ptr;
    *ptr_sz = rl_sz;
    if(mlb_sdk_buf_cursor + rl_sz > MLB_SDK_BUF_SZ)
    {
        printf("malloc failed, size %d\r\n", rl_sz);
        return NULL;
    }
    mlb_sdk_buf_cursor += rl_sz;
    #if MLB_MALLOC_DEBUG
    if(max_cursor < mlb_sdk_buf_cursor)
    {
        max_cursor = mlb_sdk_buf_cursor;
    }
    printf("cursor %d, max %d\r\n", mlb_sdk_buf_cursor, max_cursor);
    #endif
    return ptr + sizeof(size_t);
#endif
}

/**
 * @brief 平台内存分配
*/
void *mlb_plat_realloc(void*ptr, size_t sz)
{
#ifdef MLB_MALLOC_USE_C_LIB
    void *_ptr = realloc(ptr, sz);
    return _ptr;
#else
    size_t rl_sz = sz + sizeof(size_t);
    rl_sz = (rl_sz + MLB_SDK_BUF_ALIGN - 1) / MLB_SDK_BUF_ALIGN * MLB_SDK_BUF_ALIGN;
    uint8_t *_ptr = (uint8_t *)ptr;
    size_t *ptr_sz = (size_t *)(_ptr - sizeof(size_t));
    if(mlb_sdk_buf_cursor - *ptr_sz + rl_sz > MLB_SDK_BUF_SZ)
    {
        printf("realloc failed, size %d\r\n", rl_sz);
        return NULL;
    }
    mlb_sdk_buf_cursor -= *ptr_sz;
    mlb_sdk_buf_cursor += rl_sz;
    *ptr_sz = rl_sz;
    #if MLB_MALLOC_DEBUG
    if(max_cursor < mlb_sdk_buf_cursor)
    {
        max_cursor = mlb_sdk_buf_cursor;
    }
    printf("cursor %d, max %d\r\n", mlb_sdk_buf_cursor, max_cursor);
    #endif
    return ptr;
#endif
}

/**
 * @brief 平台内存释放
*/
void mlb_plat_free(void *ptr)
{
#ifdef MLB_MALLOC_USE_C_LIB
    free(ptr);
#else
    uint8_t *_ptr = (uint8_t *)ptr;
    size_t *ptr_sz = (size_t *)(_ptr - sizeof(size_t));
    mlb_sdk_buf_cursor -= *ptr_sz;
    #if MLB_MALLOC_DEBUG
    printf("cursor %d\r\n", mlb_sdk_buf_cursor);
    #endif
#endif
}

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
MRT_T mlb_plat_spi_write_then_read(void *in, size_t insz, void *out, size_t outsz)
{
    int32_t ret = MRT_OK;

    mlb_plat_spi_cs_low();

    ret = HAL_SPIM_Send(&s_hspim, in, insz, SPIM_CS_NONE, HAL_MAX_DELAY);
    if(ret != HAL_OK) {
       return MRT_SPI_ERROR;
    }

    if(out) {
        ret = HAL_SPIM_Receive(&s_hspim, out, outsz, SPIM_CS_NONE, HAL_MAX_DELAY);
        if(ret != HAL_OK) {
            return MRT_SPI_ERROR;
        }
    }

    tx_thread_sleep(1);
    mlb_plat_spi_cs_high();
    
    return ret;
}

/**
 * @brief 平台SPI CS 拉高
*/
void mlb_plat_spi_cs_high(void)
{
    HAL_GPIO_WritePin(SPI_CS_GROUP, SPI_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief 平台SPI CS 拉低
*/
void mlb_plat_spi_cs_low(void)
{
    HAL_GPIO_WritePin(SPI_CS_GROUP, SPI_CS_PIN, GPIO_PIN_RESET);
}

#define SPIFLASH_PAGE_SIZE              256
#define SPIFLASH_SECTOR_SIZE            (4*1024)
#define SPIFLASH_PAGE_PER_SECTOR        (16)
#define SPIFLASH_PAGE_PER_BLOCK         (256)
/**
 * @brief 平台flash block大小
 * @return block大小
*/
size_t mlb_plat_flash_block_size(void)
{
    return SPIFLASH_SECTOR_SIZE;
}

/**
 * @brief 获取flash可用空间大小
 * @return size_t
 */
size_t mlb_plat_flash_flash_size(void)
{
	// return (1 * 1000 * 1024);
    return (MLB_MAX_ENROLL_STEP + 3 * MLB_MAX_FP_NUMBER + 20) * SPIFLASH_SECTOR_SIZE;
}

/**
 * @brief flash擦除
 * @param offset 擦除偏移, block对齐
 * @param size 擦除大小, block_size 对齐
 * @return MRT_T
 * @retval MRT_OK 擦除成功
 * @retval MRT_FLASH_ERASE Flash 擦除失败
 */
MRT_T mlb_plat_flash_block_erase(size_t offset, size_t size)
{
    UINT status;

    uint8_t *tmp_buf = malloc(size);
    if(tmp_buf == NULL)
    {
        printf("fx malloc fail\r\n");
        return MRT_FLASH_ERASE;
    }
    memset(tmp_buf, 0xff, size);
    status = mlb_plat_flash_block_write(offset, tmp_buf, size);
    if(status != MRT_OK)
    {
        free(tmp_buf);
        printf("fx erase fail\r\n");
        return MRT_FLASH_ERASE;
    }
    free(tmp_buf);

    return MRT_OK;
}

/**
 * @brief 平台flash 写入
 * @param offset 写入偏移, block对齐
 * @param buf 写入指针
 * @param size 写入大小, block_size 对齐
 * @return MRT_T
 * @retval MRT_OK 写入成功
 * @retval MRT_FLASH_WRITE Flash 写入失败
*/
MRT_T mlb_plat_flash_block_write(size_t offset, void *buf, size_t size)
{
    UINT status;
    FX_FILE fop_file;
    char fileName[128];
    ULONG actual;
    ULONG file_size;

    (void) actual;

    if(offset == MLB_USER_DATA_FLASH_OFFSET)
    {
        memset(fileName, 0, 128);
        snprintf(fileName, 128, MLB_USER_FID_FILE, MLB_FS_DATA_DIR);
        offset = 0;
    } else {
        memset(fileName, 0, 128);
        snprintf(fileName, 128, MLB_FS_PROPERTY_FILE, MLB_FS_DATA_DIR);
    }

    status = fx_file_open(gmlb_fp_Disk, &fop_file, fileName, FX_OPEN_FOR_WRITE);
    if (status != FX_SUCCESS) {
        printf("file open FAIL\r\n");
        return MRT_FLASH_WRITE;
    }
    
    //make sure write position is correct if file size rather than offset
    fx_directory_information_get(gmlb_fp_Disk, fileName, NULL, &file_size, NULL, NULL, NULL, NULL, NULL, NULL);
    if(offset > file_size)
    {
        uint32_t tmp_size = offset - file_size;
        uint8_t *tmp_buf = malloc(tmp_size); 
        if(tmp_buf == NULL)
        {
            fx_file_close(&fop_file);
            printf("fx malloc fail\r\n");
            return MRT_FLASH_WRITE;
        }
        memset(tmp_buf, 0xff, tmp_size);
        fx_file_seek(&fop_file, file_size);
        fx_file_write(&fop_file, tmp_buf, tmp_size);
        free(tmp_buf);
    }
    
    status = fx_file_seek(&fop_file, offset);
    if (status != FX_SUCCESS) {
        printf("file seek FAIL\r\n");
        return MRT_FLASH_WRITE;
    }

    status = fx_file_write(&fop_file, buf, size);
    if (status != FX_SUCCESS) {
        printf("file write FAIL %d %d\r\n", status, offset);
        return MRT_FLASH_WRITE;
    }
    status = fx_file_close(&fop_file);
    if (status != FX_SUCCESS) {
        printf("closing file FAIL\r\n");
        return MRT_FLASH_WRITE;
    }
    status = fx_media_flush(gmlb_fp_Disk);
    if (status != FX_SUCCESS) {
        printf("file flush FAIL\r\n");
        return MRT_FLASH_WRITE;
    }

    return MRT_OK;
}

/**
 * @brief flush
 * @return MRT_T
 * @retval MRT_OK 同步成功
 * @retval MRT_FLASH_SYNC Flash 同步失败
 */
MRT_T mlb_plat_flash_block_sync(void)
{
    return MRT_OK;
}

/**
 * @brief 平台flash 读取
 * @param offset 读取偏移, block对齐
 * @param buf 读取buf
 * @param size 读取大小, 非对齐
 * @return MRT_T
 * @retval MRT_OK 读取成功
 * @retval MRT_FLASH_READ Flash 读取失败
*/
MRT_T mlb_plat_flash_read(size_t offset, void *buf, size_t size)
{
    UINT status;
    ULONG actual;
    FX_FILE fop_file;
    char fileName[128];

    if(offset == MLB_USER_DATA_FLASH_OFFSET)
    {
        memset(fileName, 0, 128);
        snprintf(fileName, 128, MLB_USER_FID_FILE, MLB_FS_DATA_DIR);
        offset = 0;
    } else {
        memset(fileName, 0, 128);
        snprintf(fileName, 128, MLB_FS_PROPERTY_FILE, MLB_FS_DATA_DIR);
    }

    status = fx_file_open(gmlb_fp_Disk, &fop_file, fileName, FX_OPEN_FOR_READ);
    if(status == FX_NOT_FOUND) {
        return MRT_OK;
    } else if (status != FX_SUCCESS) {
        printf("file open FAIL ret %d\r\n", status);
        return MRT_FLASH_READ;
    } 
    status = fx_file_seek(&fop_file, offset);
    if (status != FX_SUCCESS) {
        printf("file seek FAIL\r\n");
        return MRT_FLASH_READ;
    }
    status = fx_file_read(&fop_file, buf, size, &actual);
    if (status != FX_SUCCESS && status != FX_END_OF_FILE) {
        printf("reading file FAIL\r\n ret %d, offset %d, size %d\r\n", status, offset, size);
        return MRT_FLASH_READ;
    }
    status = fx_file_close(&fop_file);
    if (status != FX_SUCCESS) {
        printf("closing file FAIL\r\n");
        return MRT_FLASH_READ;
    }

    return MRT_OK;
}

