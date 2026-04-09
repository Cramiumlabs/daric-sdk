/**
 ******************************************************************************
 * @file    reram_filexLevelx_app.h
 * @author  OS Team
 * @brief   Header file of levelx module.
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
#ifndef _RERAM_FILEXLEVELX_APP_H_
#define _RERAM_FILEXLEVELX_APP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include   "tx_api.h"
#include   "reram_levelx_app.h"


extern LX_NOR_FLASH gReramLevelx;

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
extern  uint32_t appReramFilexLevelxDiskLoad(void);

/**
 * @brief  Format the data partition specified by the parameters.
 * @param  reramDiskInfo Pointer to a t_ReramDiskInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 *
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system.
 * Make sure that the data on the disk is no longer needed
 * before calling this interface.
 */
extern uint32_t appReramFilexLevelxDiskFormat(void);

/**
 * @brief  Initializes the reram levelx structure with the specified configuration.
 * @retval LX status code indicating the success of initialization,
 *         LX_SUCCESS means success, other values indicate errors.
 *
 * This function sets up the base address, geometry, driver functions, and sector buffer
 * for the reram. It configures the read, write, erase, and verify operations
 * with specific implementations.
 */
extern uint32_t appReramLevelxInit(void);

/**
 * @brief  Erases all data in the ReRAM LevelX storage by writing 0xFF to head sector of blocks.
 * @retval LX status code indicating the success of the operation,
 *         LX_SUCCESS means all blocks were successfully erased,
 *         other values indicate errors during the erase process.
 *
 * This function performs a complete erase of the ReRAM LevelX storage by:
 * Iterating through all blocks in the storage
 * The operation uses the hardware abstraction layer (HAL) for ReRAM writes.
 */
extern uint32_t appDataLevelxEraseAll(void);
extern ULONG  appReramLevelxPrintAllHead(void);
#ifdef __cplusplus
}
#endif

#endif