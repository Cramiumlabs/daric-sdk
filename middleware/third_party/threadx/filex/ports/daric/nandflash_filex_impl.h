/**
 ******************************************************************************
 * @file    nandflash_filex_impl.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _NANDFLASH_FILEX_IMPL_H_
#define _NANDFLASH_FILEX_IMPL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "tx_api.h"
#include "fx_api.h"

typedef enum {
    WRITE_STATUS_NO,
    WRITE_STATUS_PENDING,
    WRITE_STATUS_IN_PROGRESS
} NandWriteStatus;

#define INVALID_BLOCK_INDEX 0xFFFF

/**
 * @brief Use the t_NandflashDiskInfo structure to manage an independent file system.
 *
 */
typedef struct {
    FX_MEDIA    *mediaPtr;          /* Pointer to media control block */
    UINT        nandStartBlock;     /* start block of nandflash */
    UINT        numbsOfBlock;       /* number of blocks occupied by the filesystem */
    UINT        sectsPerBlock;      /* sectors per block */
    UCHAR       *blockBuffer;       /* buffer of block */
    UCHAR       *extBlkBuffer;      /* buffer of extended block */
    UCHAR       *cache;             /* Media buffer pointer */
    UINT        cacheSize;          /* Media buffer size */
    CHAR        *volName;           /* Volume name */
    UINT        numOfFats;          /* Number of FATs */
    UINT        dirEntries;         /* Directory entries */
    UINT        hiddenSects;        /* Hidden sectors */
    ULONG       totalSects;         /* Total sectors */
    UINT        bytesPerSect;       /* Sector size */
    UINT        sectPerCluster;     /* Sectors per cluster */
    UINT        bInitialed;         /* Initialization has been completed */
    UINT        bWrtieProtected;    /* Indication of write protected */
    USHORT      preBlockIdx;        /* Prev access block number */
    USHORT      preExtBlockIdx;     /* extended block number */
    USHORT      extBlockIdx;        /* block index need to be cached */
    NandWriteStatus bWritePending;  /* Indicates the status of the write cache */
    NandWriteStatus bExtWritePending;  /* Indicates the status of the write cache */

} t_NandflashDiskInfo;


/**
 * @brief  Initializes the filex disk info according to the specified parameters.
 * @param  nandflashDiskInfo Pointer to a t_NandflashDiskInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function configures the filex disk according to the parameters specified
 * in the t_NandflashDiskInfo structure, including cache, numOfFats, and other
 * relevant settings.
 */
extern uint32_t nandflashFilexDiskLoad(t_NandflashDiskInfo *nandflashDiskInfo);

/**
 * @brief  Format the file system specified by the parameters.
 * @param  nandflashDiskInfo Pointer to a t_NandflashDiskInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system. 
 * Make sure that the data on the disk is no longer needed 
 * before calling this interface.
 */
extern uint32_t nandflashFilexDiskFormat(t_NandflashDiskInfo *nandflashDiskInfo);

#ifdef __cplusplus
}
#endif

#endif