/**
 ******************************************************************************
 * @file    nandW25N01_levelx_app.h
 * @author  OS Team
 * @brief   Header file for LevelX NAND flash driver for W25N01GVxxIT.
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
#ifndef _NANDW25N01_LEVELX_APP_H_
#define _NANDW25N01_LEVELX_APP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "tx_api.h"
#include "lx_api.h"

/* Exported macro ------------------------------------------------------------*/
#define W25N01IT_TOTAL_BLOCKS           1024
#define W25N01IT_PAGES_PER_BLOCK        64
#define W25N01IT_BYTES_PER_PAGE         2048
#define W25N01IT_SPARE_BYTES_PER_PAGE   64

/* Exported types ------------------------------------------------------------*/
/**
 * @brief Use the t_NandLevelxInfo structure to manage an independent wear leveling area.
 *
 */
typedef struct {
    LX_NAND_FLASH    *mediaPtr;     /* Pointer to media control block */
    CHAR        *diskName;          /* Disk name */
    UINT        nandStartBlock;     /* start block of nandflash */
    UINT        numbsOfBlock;       /* number of blocks */
    UINT        pagesPerBlock;      /* pages per block */
    UINT        bytesPerPage;       /* bytes per page */
    UINT        spareData1Offset;   /* flash spare data1 offset */
    UINT        spareData1Length;   /* flash spare data1 length */
    UINT        spareData2Offset;   /* flash spare data2 offset */
    UINT        spareData2Length;   /* flash spare data2 length */
    UINT        spareLength;        /* flash spare total length */

    UCHAR       *pMemBuffer;        /* Media buffer pointer */
    UINT        memBufferSize;      /* Media buffer size */

    UCHAR       *pageCache;         /* Media buffer pointer */
    UINT        pageCacheSize;      /* Media buffer size */
    UINT        bInitialed;         /* Initialization has been completed */
    UINT        bEncrypt;           /* Indicate whether the data needs to be encrypted */
} t_NandflashLevelxInfo;
/* Exported constants --------------------------------------------------------*/


/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Formats a NAND Flash device for use with the LevelX file system
 * @param pNandLevelxInfo Pointer to the NAND Flash LevelX information structure
 *                        containing media configuration, disk identification, and
 *                        memory buffer resources for the formatting operation
 * @return LX_SUCCESS The NAND Flash device was successfully formatted for LevelX
 *         LX_ERROR Formatting operation failed due to invalid parameters,
 *                  insufficient resources, or hardware-related issues
 *
 * This function initiates the formatting process for a NAND Flash device to prepare
 * it for use with the LevelX wear-leveling flash file system. It serves as a wrapper
 * that calls the LevelX library's format function with the appropriate parameters,
 * including the flash media pointer, disk name, and memory buffers required for
 * the formatting operation.
 */
extern ULONG  nandFlashLevelxFormat(t_NandflashLevelxInfo *pNandLevelxInfo);

/**
 * @brief  Initializes and loads the nand LevelX storage system.
 * @param  pNandLevelxInfo Pointer to a t_NandflashLevelxInfo structure containing
 *         the configuration parameters for the nand LevelX storage.
 * @return LX status code indicating the success of the operation,
 *         LX_SUCCESS means initialization was successful,
 *         LX_ERROR means the storage was already initialized or other errors occurred.
 *
 * This function performs the following operations:
 * 1. Checks if the storage is already initialized (returns error if true)
 * 2. Marks the storage as initialized
 * 3. Stores the configuration info in a global variable
 * 4. Initializes the underlying nand flash interface
 *
 * Note: The function uses global state (lx_media_driver_info) to maintain storage information.
 * The initialization can only be performed once unless the bInitialed flag is reset.
 */
extern ULONG  nandFlashLevelxLoad(t_NandflashLevelxInfo *pNandLevelxInfo);

/**
 * @brief Performs a clean reload of the LevelX file system on NAND Flash
 * @param pNandLevelxInfo Pointer to the NAND Flash LevelX information structure
 *                        containing media configuration, disk identification, and
 *                        memory buffer resources for the reload operation
 * @return LX_SUCCESS The clean reload operation completed successfully
 *         LX_ERROR One or more steps in the reload process failed (close, format,
 *                  or open operation)
 *
 * @note The function follows a three-step process:
 *       1. Close the current LevelX file system instance
 *       2. Format the NAND Flash device (erasing all data)
 *       3. Reopen the file system with fresh initialization
 * @warning This operation will permanently erase all data on the NAND Flash device
 * @warning The function does not preserve any data during the reload; all user
 *          data will be lost
 *
 * This function executes a complete restart cycle of the LevelX file system on
 * a NAND Flash device by closing the current file system instance, reformatting
 * the flash, and reopening it with fresh initialization. This process is typically
 * used for recovery from file system corruption, testing purposes, or when a
 * complete reset of the flash storage is required.
 */
extern ULONG  nandFlashLevelxFormatReload(t_NandflashLevelxInfo *pNandLevelxInfo);

/**
 * @brief Initializes the SPI interface and HAL layer for NAND Flash communication
 * @param None
 * @return LX_SUCCESS Both SPI interface and HAL flash initialization completed successfully
 *         LX_ERROR Either SPI initialization or HAL flash initialization failed
 *
 * @note The function performs two distinct initialization steps in sequence:
 *       1. SPI interface initialization via nandflashInitSPI()
 *       2. HAL flash layer initialization via HAL_FLASH_Init()
 *
 * This function performs the initialization sequence required for SPI-based
 * NAND Flash communication by first initializing the SPI interface hardware
 * and then initializing the Hardware Abstraction Layer (HAL) for flash operations.
 * It serves as the entry point for setting up the low-level communication
 * infrastructure needed for NAND Flash access.
 */
extern ULONG  nandFlashInitHalSpi(void);
#ifdef __cplusplus
}
#endif

#endif