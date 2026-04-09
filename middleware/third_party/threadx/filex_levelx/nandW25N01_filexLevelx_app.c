/**
******************************************************************************
* @file    nandW25N01IT_filexLevelx_app.c
* @author  OS Team
* @brief   This file's contents are the adaptation of Levelx for the nandflash storage.
******************************************************************************
* @attention
*
* © Copyright CrossBar, Inc. 2024.
*
* All rights reserved.
*
* This software is the proprietary property of CrossBar, Inc. and is protected
* by copyright laws. Any unauthorized reproduction, distribution, or
* modification is strictly prohibited.
*
******************************************************************************
*/

/* Include necessary system files. */
#include "stdio.h"
#include "daric_hal_flash.h"
#include "nandW25N01_filex_levelx_impl.h"
#include "sys_config.h"
#include "tx_log.h"

#include "tx_log.h"
#undef LOG_TAG
#define LOG_TAG "NAND_FXLX_APP"
/*
 * ┌────────────────────────┬───────────────────────┬───────────┬──────────────────────┐
 * │    Address Range       │     Name in DARIC     │   Size    │ Name In Active Card  │
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 6000 0000 ~ 6001 FFFF  │                 Boot 0│     128 KB│              Boot ROM│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 6002 0000 ~ 6004 FFFF  │                 Boot 1│     192 KB│           Boot loader│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 6005 0000 ~ 602B FFFF  │               Firmware│    2496 KB│         User Firmware│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 602C 0000 ~ 602D 9FFF  │             Parameters│     104 KB│           LCD WF Data│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 602D A000 ~ 603D 9FFF  │                   Data│       1 MB│          FAT12 System│
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 603D A000 ~ 603D BFFF  │       one-way counters│       8 KB│                      │
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 603D C000 ~ 603D FFFF  │ Access Contorl Setting│      16 KB│                      │
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 603E 0000 ~ 603E FFFF  │          Dataslot area│      64 KB│                      │
 * ├────────────────────────┼───────────────────────┼───────────┼──────────────────────┤
 * │ 603F 0000 ~ 603F FFFF  │           Keyslot Area│      64 KB│                      │
 * └────────────────────────┴───────────────────────┴───────────┴──────────────────────┘
 */

#if defined(CONFIG_SUPPORT_NANDLEVELX)
/* Private macro -------------------------------------------------------------*/
#define LX_NAND_SPARE_DATA1_OFFSET      4
#define LX_NAND_SPARE_DATA1_LENGTH      4
#define LX_NAND_SPARE_DATA2_OFFSET      2
#define LX_NAND_SPARE_DATA2_LENGTH      2

/*
 * Below are all configurations of gNandflashP1Disk.
 * gNandflashP1Disk will be a read-only, non-encrypted file system
 * primarily used to store data such as LCD parameters.
 */
#define LX_NAND_P1_START_BLOCK          0
#define LX_NAND_P1_TOTAL_BLOCK          512
#define LX_NAND_P1_DISK_NAME            "NandFlashLevelxP1Disk"

static unsigned char gLevelxP1FrameBuffer[W25N01IT_BYTES_PER_PAGE * 4
                                          + (W25N01IT_BYTES_PER_PAGE
                                             +W25N01IT_SPARE_BYTES_PER_PAGE) * 2]
                                        __attribute__((aligned(4)));

/* Buffer for levelx LX_NAND_FLASH sector cache.
   This must be large enough for at least one sector.
 */
static unsigned char gLevelxP1SectorCache[W25N01IT_BYTES_PER_PAGE
                                          + W25N01IT_SPARE_BYTES_PER_PAGE]
                                        __attribute__((aligned(4)));

/* Define Levelx global data structures.  */

static LX_NAND_FLASH gNandFlashP1Levelx;

static t_NandflashLevelxInfo gNandFlashP1LevelxInfo = {
    .mediaPtr           = &gNandFlashP1Levelx,
    .diskName           = LX_NAND_P1_DISK_NAME,
    .nandStartBlock     = LX_NAND_P1_START_BLOCK,
    .numbsOfBlock       = LX_NAND_P1_TOTAL_BLOCK,
    .pagesPerBlock      = W25N01IT_PAGES_PER_BLOCK,
    .bytesPerPage       = W25N01IT_BYTES_PER_PAGE,
    .spareData1Offset   = LX_NAND_SPARE_DATA1_OFFSET,
    .spareData1Length   = LX_NAND_SPARE_DATA1_LENGTH,
    .spareData2Offset   = LX_NAND_SPARE_DATA2_OFFSET,
    .spareData2Length   = LX_NAND_SPARE_DATA2_LENGTH,
    .spareLength        = W25N01IT_SPARE_BYTES_PER_PAGE,

    .pMemBuffer         = gLevelxP1FrameBuffer,
    .memBufferSize      = sizeof(gLevelxP1FrameBuffer),
    .pageCache          = gLevelxP1SectorCache,
    .pageCacheSize      = sizeof(gLevelxP1SectorCache),

    .bInitialed         = 0,
    .bEncrypt           = 0,
};

#define FX_P1_RESERVE_BLOCKS        64
#define FX_P1_NUBER_OF_FATS         1
#define FX_P1_DIRECTORY_ENTRIES     128
#define FX_P1_HIDDEN_SECTORS        0
#define FX_P1_TOTAL_SECTORS         ((LX_NAND_P1_TOTAL_BLOCK - FX_P1_RESERVE_BLOCKS) * W25N01IT_PAGES_PER_BLOCK)
#define FX_P1_SECTOR_SIZE           W25N01IT_BYTES_PER_PAGE
#define FX_P1_SECTORS_PER_CLUSTER   4

/* Buffer for FileX FX_MEDIA sector cache. 
   This must be large enough for at least one sector.
 */
static unsigned char gDataP1SectorCache[W25N01IT_BYTES_PER_PAGE
                                        * FX_MAX_SECTOR_CACHE] __attribute__((aligned(4)));

/* Define FileX global data structures.  */
DTCM_PUT_BUFF_SECTION FX_MEDIA        gNandflashP1Disk;

static t_NandflashDiskInfo gNandFlashP1DiskInfo = {
    .mediaPtr       = &gNandflashP1Disk,
    .cache          = gDataP1SectorCache,
    .cacheSize      = sizeof(gDataP1SectorCache),
    .volName        = "Nandflash_DATAP1",
    .numOfFats      = FX_P1_NUBER_OF_FATS,
    .dirEntries     = FX_P1_DIRECTORY_ENTRIES,
    .hiddenSects    = FX_P1_HIDDEN_SECTORS,
    .totalSects     = FX_P1_TOTAL_SECTORS,
    .bytesPerSect   = FX_P1_SECTOR_SIZE,
    .sectPerCluster = FX_P1_SECTORS_PER_CLUSTER,
    .bInitialed     = 0,
    .bWrtieProtected= 0,
};

t_NandflashFilexLevelxInfo gNandFlashP1FilexLevelxInfo = {
    .nandFlashDiskInfo      = &gNandFlashP1DiskInfo,
    .nandFlashLevelxInfo    = &gNandFlashP1LevelxInfo,
};

/*
 * Below are all configurations of gNandflashP2Disk.
 * gNandflashP2Disk will be a read-only, non-encrypted file system
 * primarily used to store data such as LCD parameters.
 */
#define LX_NAND_P2_START_BLOCK          512
#define LX_NAND_P2_TOTAL_BLOCK          (W25N01IT_TOTAL_BLOCKS - LX_NAND_P2_START_BLOCK)
#define LX_NAND_P2_DISK_NAME            "NandFlashLevelxP2Disk"

static unsigned char gLevelxP2FrameBuffer[W25N01IT_BYTES_PER_PAGE * 4
                                          + (W25N01IT_BYTES_PER_PAGE
                                             +W25N01IT_SPARE_BYTES_PER_PAGE) * 2]
                                        __attribute__((aligned(4)));

/* Buffer for levelx LX_NAND_FLASH sector cache.
   This must be large enough for at least one sector.
 */
static unsigned char gLevelxP2SectorCache[W25N01IT_BYTES_PER_PAGE
                                          + W25N01IT_SPARE_BYTES_PER_PAGE]
                                        __attribute__((aligned(4)));

/* Define Levelx global data structures.  */

static LX_NAND_FLASH gNandFlashP2Levelx;

static t_NandflashLevelxInfo gNandFlashP2LevelxInfo = {
    .mediaPtr           = &gNandFlashP2Levelx,
    .diskName           = LX_NAND_P2_DISK_NAME,
    .nandStartBlock     = LX_NAND_P2_START_BLOCK,
    .numbsOfBlock       = LX_NAND_P2_TOTAL_BLOCK,
    .pagesPerBlock      = W25N01IT_PAGES_PER_BLOCK,
    .bytesPerPage       = W25N01IT_BYTES_PER_PAGE,
    .spareData1Offset   = LX_NAND_SPARE_DATA1_OFFSET,
    .spareData1Length   = LX_NAND_SPARE_DATA1_LENGTH,
    .spareData2Offset   = LX_NAND_SPARE_DATA2_OFFSET,
    .spareData2Length   = LX_NAND_SPARE_DATA2_LENGTH,
    .spareLength        = W25N01IT_SPARE_BYTES_PER_PAGE,

    .pMemBuffer         = gLevelxP2FrameBuffer,
    .memBufferSize      = sizeof(gLevelxP2FrameBuffer),
    .pageCache          = gLevelxP2SectorCache,
    .pageCacheSize      = sizeof(gLevelxP2SectorCache),

    .bInitialed         = 0,
    .bEncrypt           = 0,
};

#define FX_P2_RESERVE_BLOCKS        64
#define FX_P2_NUBER_OF_FATS         1
#define FX_P2_DIRECTORY_ENTRIES     128
#define FX_P2_HIDDEN_SECTORS        0
#define FX_P2_TOTAL_SECTORS         ((LX_NAND_P2_TOTAL_BLOCK - FX_P2_RESERVE_BLOCKS) * W25N01IT_PAGES_PER_BLOCK)
#define FX_P2_SECTOR_SIZE           W25N01IT_BYTES_PER_PAGE
#define FX_P2_SECTORS_PER_CLUSTER   4

/* Buffer for FileX FX_MEDIA sector cache.
   This must be large enough for at least one sector.
 */
static unsigned char gDataP2SectorCache[W25N01IT_BYTES_PER_PAGE
                                        * FX_MAX_SECTOR_CACHE] __attribute__((aligned(4)));

/* Define FileX global data structures.  */
DTCM_PUT_BUFF_SECTION FX_MEDIA        gNandflashP2Disk;

static t_NandflashDiskInfo gNandFlashP2DiskInfo = {
    .mediaPtr       = &gNandflashP2Disk,
    .cache          = gDataP2SectorCache,
    .cacheSize      = sizeof(gDataP2SectorCache),
    .volName        = "Nandflash_DATAP2",
    .numOfFats      = FX_P2_NUBER_OF_FATS,
    .dirEntries     = FX_P2_DIRECTORY_ENTRIES,
    .hiddenSects    = FX_P2_HIDDEN_SECTORS,
    .totalSects     = FX_P2_TOTAL_SECTORS,
    .bytesPerSect   = FX_P2_SECTOR_SIZE,
    .sectPerCluster = FX_P2_SECTORS_PER_CLUSTER,
    .bInitialed     = 0,
    .bWrtieProtected= 0,
};

t_NandflashFilexLevelxInfo gNandFlashP2FilexLevelxInfo = {
    .nandFlashDiskInfo      = &gNandFlashP2DiskInfo,
    .nandFlashLevelxInfo    = &gNandFlashP2LevelxInfo,
};

#if (LX_NAND_P1_TOTAL_BLOCK + LX_NAND_P2_TOTAL_BLOCK) > W25N01IT_TOTAL_BLOCKS
#error "LX_NAND_P1_TOTAL_BLOCK + LX_NAND_P2_TOTAL_BLOCK must be <= W25N01IT_TOTAL_BLOCKS"
#endif

#if LX_NAND_P2_START_BLOCK < (LX_NAND_P1_START_BLOCK + LX_NAND_P1_TOTAL_BLOCK)
#error "LX_NAND_P2_START_BLOCK >= (LX_NAND_P1_START_BLOCK + LX_NAND_P1_TOTAL_BLOCK)"
#endif


/**
 * @brief  Initializes the nandflash levelx structure with the specified configuration.
 * @retval LX status code indicating the success of initialization,
 *         LX_SUCCESS means success, other values indicate errors.
 *
 * This function sets up the base address, geometry, driver functions, and sector buffer
 * for the nandflash. It configures the read, write, erase, and verify operations
 * with specific implementations.
 */
uint32_t appNandFlashLevelxInit(void)
{
    return nandFlashLevelxLoad(&gNandFlashP1LevelxInfo);
}

/**
 * @brief  Initializes the data disk info according to the specified parameters.
 * @param  none.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function configures the filex disk according to the parameters specified
 * in the gNandFlashP1DiskInfo structure, including cache, numOfFats, and other
 * relevant settings.
 */
uint32_t appNandFlashFilexLevelxDiskLoad(void)
{
    UINT status;

//    status = daricNandFlashLevelxLoad(&gNandFlashP1LevelxInfo);
//    if (status != LX_SUCCESS)
//    {
//        printf("nandflash open error, errorNum:%d", status);
//        return status;
//    }
    status = nandFlashInitHalSpi();
    if (HAL_OK != status)
    {
        LOGE("NandflashDisk SPIM Init errorN=%d", status);
        return FX_IO_ERROR;
    }

    status = nandFlashFilexLevelxDiskLoad(&gNandFlashP1FilexLevelxInfo);
    if (status != FX_SUCCESS)
    {
        printf("nandflashP1 filex open error, errorNum:%d", status);
        return FX_IO_ERROR;
    }

    status = nandFlashFilexLevelxDiskLoad(&gNandFlashP2FilexLevelxInfo);
    if (status != FX_SUCCESS)
    {
        printf("nandflashP2 filex open error, errorNum:%d", status);
        return FX_IO_ERROR;
    }

    return(FX_SUCCESS);
}

/**
 * @brief  Format the data partition specified by the parameters.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system. 
 * Make sure that the data on the disk is no longer needed 
 * before calling this interface.
 */
uint32_t appNandFlashFilexLevelxDiskP1Format(void)
{
    UINT status;

//    status = daricNandFlashLevelxLoad(&gNandFlashP1LevelxInfo);
//    if (status != LX_SUCCESS)
//    {
//        printf("nandflash open error, errorNum:%d", status);
//        return status;
//    }

    status = nandFlashFilexLevelxDiskFormat(&gNandFlashP1FilexLevelxInfo);
    if (status != FX_SUCCESS)
    {
        printf("nandflashP1 format error, errorNum:%d", status);
        return status;
    }

    return(FX_SUCCESS);
}

/**
 * @brief  Format the data partition specified by the parameters.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 *
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system.
 * Make sure that the data on the disk is no longer needed
 * before calling this interface.
 */
uint32_t appNandFlashFilexLevelxDiskP2Format(void)
{
    UINT status;

    //    status = daricNandFlashLevelxLoad(&gNandFlashP1LevelxInfo);
    //    if (status != LX_SUCCESS)
    //    {
    //        printf("nandflash open error, errorNum:%d", status);
    //        return status;
    //    }

    status = nandFlashFilexLevelxDiskFormat(&gNandFlashP2FilexLevelxInfo);
    if (status != FX_SUCCESS)
    {
        printf("nandflashP2 format error, errorNum:%d", status);
        return status;
    }

    return(FX_SUCCESS);
}

/**
 * @brief  This function initializes the filex.
 * @param  none.
 * @retval none.
 *
 * This function initializes the various control data structures
 * for the FileX System component
 */
void appNandFlashFilexLevelxFilesystemInit(void)
{
    /* Initialize NAND flash.  */
    lx_nand_flash_initialize();
    /* Initialize filex.  */
    fx_system_initialize();

}
#endif