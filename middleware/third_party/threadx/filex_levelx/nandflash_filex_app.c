/**
******************************************************************************
* @file    daric_filex_app.c
* @author  OS Team
* @brief   This file's contents are the adaptation of Filex for the ReRam storage.
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
#include "nandflash_filex_impl.h"
#include "daric_filex_app.h"
#include "sys_config.h"
#include "tx_log.h"
#undef LOG_TAG
#define LOG_TAG "NFLASH_FILEX_APP"

#if !defined(CONFIG_SUPPORT_NANDLEVELX)
/* Private macro -------------------------------------------------------------*/
#define FX_NANDFLASH_START_GROUP       0
#define FX_NANDFLASH_BLOCK_NUMBERS     512
#define FX_DATA_NUBER_OF_FATS          2
#define FX_DATA_DIRECTORY_ENTRIES      128
#define FX_DATA_HIDDEN_SECTORS         0
#define FX_NANDFLASH_PAGES_PERBLOCK    64
#define FX_DATA_TOTAL_SECTORS          (FX_NANDFLASH_BLOCK_NUMBERS * FX_NANDFLASH_PAGES_PERBLOCK)
#define FX_DATA_SECTOR_SIZE            2048
#define FX_DATA_SECTORS_PER_CLUSTER    1

/* Buffer for FileX FX_MEDIA sector cache. 
   This must be large enough for at least one sector, 
   which are typically 512 bytes in size.  
 */
static unsigned char gDataSectorCache[2048];
//static unsigned char gNandBlockBuffer[64*2048];
#define CONFIG_SUPPORT_FILEX_EXTBUFFER
#ifdef CONFIG_SUPPORT_FILEX_EXTBUFFER
//static unsigned char gextBlockBuffer[64*2048];
#endif
/* Define FileX global data structures.  */

DTCM_PUT_BUFF_SECTION FX_MEDIA        gNandflashDisk;

t_NandflashDiskInfo gNandflashDiskInfo = {
    .mediaPtr       = &gNandflashDisk,
    .nandStartBlock = FX_NANDFLASH_START_GROUP,
    .extBlockIdx    = FX_NANDFLASH_START_GROUP,
    .numbsOfBlock   = FX_NANDFLASH_BLOCK_NUMBERS,
    .sectsPerBlock  = FX_NANDFLASH_PAGES_PERBLOCK,
    .blockBuffer    = NULL,
    #ifdef CONFIG_SUPPORT_FILEX_EXTBUFFER
    .extBlkBuffer   = NULL,
    #else
    .extBlkBuffer   = NULL,
    #endif
    .cache          = gDataSectorCache,
    .cacheSize      = sizeof(gDataSectorCache),
    .volName        = "Nandflash_DATA",
    .numOfFats      = FX_DATA_NUBER_OF_FATS,
    .dirEntries     = FX_DATA_DIRECTORY_ENTRIES,
    .hiddenSects    = FX_DATA_HIDDEN_SECTORS,
    .totalSects     = FX_DATA_TOTAL_SECTORS,
    .bytesPerSect   = FX_DATA_SECTOR_SIZE,
    .sectPerCluster = FX_DATA_SECTORS_PER_CLUSTER,
    .bWritePending  = WRITE_STATUS_NO,
    .bExtWritePending = WRITE_STATUS_NO,
    .preBlockIdx    = INVALID_BLOCK_INDEX,
    .preExtBlockIdx = INVALID_BLOCK_INDEX,
    .bInitialed     = 0,
};

/**
 * @brief  Initializes the data disk info according to the specified parameters.
 * @param  none.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function configures the filex disk according to the parameters specified
 * in the gNandflashDiskInfo structure, including cache, numOfFats, and other
 * relevant settings.
 */
uint32_t appNandflashFilexDiskLoad(void)
{
    if (gNandflashDiskInfo.blockBuffer == NULL)
    {
        gNandflashDiskInfo.blockBuffer = (uint8_t *)malloc(FX_NANDFLASH_PAGES_PERBLOCK * FX_DATA_SECTOR_SIZE);
    }
    if (gNandflashDiskInfo.extBlkBuffer == NULL)
    {
        gNandflashDiskInfo.extBlkBuffer = (uint8_t *)malloc(FX_NANDFLASH_PAGES_PERBLOCK * FX_DATA_SECTOR_SIZE);
    }

    if ((gNandflashDiskInfo.blockBuffer == NULL) || (gNandflashDiskInfo.extBlkBuffer == NULL))
    {
        LOGE("Not enouth sram memory!");
        return FX_NOT_ENOUGH_MEMORY;
    }

    return nandflashFilexDiskLoad(&gNandflashDiskInfo);
}

/**
 * @brief  Format the data partition specified by the parameters.
 * @param  none.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 *
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system.
 * Make sure that the data on the disk is no longer needed
 * before calling this interface.
 */
uint32_t appNandflashFilexDiskFormat(void)
{
    if (gNandflashDiskInfo.blockBuffer == NULL)
    {
        gNandflashDiskInfo.blockBuffer = (uint8_t *)malloc(FX_NANDFLASH_PAGES_PERBLOCK * FX_DATA_SECTOR_SIZE);
    }
    if (gNandflashDiskInfo.extBlkBuffer == NULL)
    {
        gNandflashDiskInfo.extBlkBuffer = (uint8_t *)malloc(FX_NANDFLASH_PAGES_PERBLOCK * FX_DATA_SECTOR_SIZE);
    }

    if ((gNandflashDiskInfo.blockBuffer == NULL) || (gNandflashDiskInfo.extBlkBuffer == NULL))
    {
        LOGE("Not enouth sram memory!");
        return FX_NOT_ENOUGH_MEMORY;
    }

    return nandflashFilexDiskFormat(&gNandflashDiskInfo);
}

/**
 * @brief  This function initializes the filex.
 * @param  none.
 * @retval none.
 *
 * This function initializes the various control data structures
 * for the FileX System component
 */
void appNandflashFilesystemInit(void)
{
    /* Initialize filex.  */
    fx_system_initialize();
}
#endif