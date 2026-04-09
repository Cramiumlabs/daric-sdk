/**
 ******************************************************************************
 * @file    daric_filex_app.h
 * @author  OS Team
 * @brief   Header file of filex module.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _DARIC_FILEX_APP_H_
#define _DARIC_FILEX_APP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include   "tx_api.h"
#include   "fx_api.h"

extern FX_MEDIA gReramDisk;
extern FX_MEDIA gNandflashDisk;
extern FX_MEDIA gNandflashP1Disk;
extern FX_MEDIA gNandflashP2Disk;
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
extern uint32_t daricDataDiskLoad(void);

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
extern uint32_t daricDataDiskFormat(void);

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
 extern uint32_t daricNandflashDiskFormat(void);

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
extern uint32_t daricNandflashDiskP1Format(void);

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
extern uint32_t daricNandflashDiskP2Format(void);

/**
 * @brief  This function initializes the filex.
 * @param  none.
 * @retval none.
 *
 * This function initializes the various control data structures
 * for the FileX System component
 */
extern void daricFilesystemInit(void);
#ifdef __cplusplus
}
#endif

#endif