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
#include "daric_filex_app.h"
#include "reram_filexLevelx_app.h"
#include "nandflash_filex_app.h"
#include "nandW25N01_filexLevelx_app.h"
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "NAND_FX_LX"
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

/* Private macro -------------------------------------------------------------*/

/* Define FileX global data structures.  */


/**
 * @brief  Initializes the data disk info according to the specified parameters.
 * @param  none.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function configures the filex disk according to the parameters specified
 * in the gReRAMDiskInfo structure, including cache, numOfFats, and other
 * relevant settings.
 */
uint32_t daricDataDiskLoad(void)
{
    UINT status;

    status = appReramFilexLevelxDiskLoad();
    if (status != FX_SUCCESS)
    {
        LOGE("appReramFilexLevelxDiskLoad return error=%d", status);
    }

    #if defined(CONFIG_SUPPORT_NANDLEVELX)
    status = appNandFlashFilexLevelxDiskLoad();
    #else
    status = appNandflashFilexDiskLoad();
    #endif
    if (status != FX_SUCCESS)
    {
        LOGE("appNandflashFilexDiskLoad return error=%d", status);
    }

    return status;
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
uint32_t daricDataDiskFormat(void)
{
    UINT status;

    status = appReramFilexLevelxDiskFormat();

    return status;
}

#if !defined(CONFIG_SUPPORT_NANDLEVELX)
/**
 * @brief  Format the nandflash partition specified by the parameters.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 *
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system.
 * Make sure that the data on the disk is no longer needed
 * before calling this interface.
 */
uint32_t daricNandflashDiskFormat(void)
{
    UINT status;

    status = appNandflashFilexDiskFormat();

    return status;
}
#else
/**
 * @brief  Format the nandflash partition specified by the parameters.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system. 
 * Make sure that the data on the disk is no longer needed 
 * before calling this interface.
 */
uint32_t daricNandflashDiskP1Format(void)
{
    UINT status;

    status = appNandFlashFilexLevelxDiskP1Format();

    return status;
}

/**
 * @brief  Format the nandflash partition specified by the parameters.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 *
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system.
 * Make sure that the data on the disk is no longer needed
 * before calling this interface.
 */
uint32_t daricNandflashDiskP2Format(void)
{
    UINT status;

    status = appNandFlashFilexLevelxDiskP2Format();

    return status;
}
#endif

/**
 * @brief  This function initializes the filex.
 * @param  none.
 * @retval none.
 *
 * This function initializes the various control data structures
 * for the FileX System component
 */
void daricFilesystemInit(void)
{
    memset(&gReramDisk, 0, sizeof(gReramDisk));
    #if defined(CONFIG_SUPPORT_NANDLEVELX)
    memset(&gNandflashP1Disk, 0, sizeof(gNandflashP1Disk));
    memset(&gNandflashP2Disk, 0, sizeof(gNandflashP2Disk));
    /* Initialize NAND flash.  */
    lx_nand_flash_initialize();
    #else
    memset(&gNandflashDisk, 0, sizeof(gNandflashDisk));
    #endif
    
    /* Initialize NOR flash.  */
    lx_nor_flash_initialize();
    /* Initialize filex.  */
    fx_system_initialize();
}