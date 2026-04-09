/**
 ******************************************************************************
 * @file    reram_levelx_app.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _RERAM_LEVELX_APP_H_
#define _RERAM_LEVELX_APP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "tx_api.h"
#include "lx_api.h"

/**
 * @brief Use the t_ReramLevelxInfo structure to manage an independent wear leveling area.
 *
 */
typedef struct {
    LX_NOR_FLASH    *mediaPtr;      /* Pointer to media control block */
    UINT        diskBaseAddr;       /* start address of reram disk */
    UCHAR       *cache;             /* Media buffer pointer */
    UINT        cacheSize;          /* Media buffer size */
    UINT        totalBlocks;        /* Media total blocks */
    UINT        SectsPerBlock;      /* sectors per block */
    UINT        wordsPerSect;       /* Sector size */
    UINT        bInitialed;         /* Initialization has been completed */
} t_ReramLevelxInfo;


/**
 * @brief  Initializes and loads the ReRAM LevelX storage system.
 * @param  reramLevelxInfo Pointer to a t_ReramLevelxInfo structure containing
 *         the configuration parameters for the ReRAM LevelX storage.
 * @retval LX status code indicating the success of the operation,
 *         LX_SUCCESS means initialization was successful,
 *         LX_ERROR means the storage was already initialized or other errors occurred.
 *
 * This function performs the following operations:
 * 1. Checks if the storage is already initialized (returns error if true)
 * 2. Marks the storage as initialized
 * 3. Stores the configuration info in a global variable
 * 4. Initializes the underlying NOR flash interface
 * 
 * Note: The function uses global state (sReramLevelxInfo) to maintain storage information.
 * The initialization can only be performed once unless the bInitialed flag is reset.
 */
extern ULONG  reramLevelxLoad(t_ReramLevelxInfo *reramLevelxInfo);

/**
 * @brief  Erases all data in the ReRAM LevelX storage by writing 0xFF to head sector of blocks.
 * @param  reramLevelxInfo Pointer to a t_ReramLevelxInfo structure containing
 *         the configuration and parameters of the ReRAM LevelX storage.
 * @retval LX status code indicating the success of the operation,
 *         LX_SUCCESS means all blocks were successfully erased,
 *         other values indicate errors during the erase process.
 *
 * This function performs a complete erase of the ReRAM LevelX storage by:
 * Iterating through all blocks in the storage
 * The operation uses the hardware abstraction layer (HAL) for ReRAM writes.
 */
extern ULONG  reramLevelxEraseAllHead(t_ReramLevelxInfo *reramLevelxInfo);

/**
 * @brief  Clean data and loads the ReRAM LevelX storage system.
 * @param  reramLevelxInfo Pointer to a t_ReramLevelxInfo structure containing
 *         the configuration parameters for the ReRAM LevelX storage.
 * @retval LX status code indicating the success of the operation,
 *         LX_SUCCESS means initialization was successful,
 *         LX_ERROR means the storage was already initialized or other errors occurred.
 *
 * This function performs the following operations:
 * 1. Checks if the storage is already initialized (returns error if true)
 * 2. Marks the storage as initialized
 * 3. Stores the configuration info in a global variable
 * 4. Initializes the underlying NOR flash interface
 *
 * Note: The function uses global state (sReramLevelxInfo) to maintain storage information.
 * The initialization can only be performed once unless the bInitialed flag is reset.
 */
extern ULONG  reramLevelxCleanReload(t_ReramLevelxInfo *reramLevelxInfo);
#ifdef __cplusplus
}
#endif

#endif