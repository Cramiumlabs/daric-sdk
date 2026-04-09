/**
******************************************************************************
* @file    reram_levelx_app.c
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
#include "daric_hal_reram.h"
#include "reram_levelx_app.h"
#include "tx_log.h"


static UINT reramLevelxRead(ULONG *reramAddress, ULONG *destination, ULONG words);
static UINT reramLevelxWrite(ULONG *reramAddress, ULONG *source, ULONG words);
static UINT reramLevelxErase(ULONG block, ULONG eraseCount);
static UINT reramLevelxEraseVerify(ULONG block);
static t_ReramLevelxInfo *sReramLevelxInfo;

/**
 * @brief  Initializes the reram levelx structure with the specified configuration.
 * @param  reramLevelxInfo Pointer to a t_ReramLevelxInfo structure that contains
 *         the configuration information for the specified levelx area.
 * @retval LX status code indicating the success of initialization,
 *         LX_SUCCESS means success, other values indicate errors.
 *
 * This function sets up the base address, geometry, driver functions, and sector buffer
 * for the reram. It configures the read, write, erase, and verify operations
 * with specific implementations.
 */
static UINT  reramLevelxInitialize(LX_NOR_FLASH *reramFlash)
{

    /* Setup the base address of the flash memory.  */
    reramFlash -> lx_nor_flash_base_address =               (ULONG *) sReramLevelxInfo->diskBaseAddr;

    /* Setup geometry of the flash.  */
    reramFlash -> lx_nor_flash_total_blocks =               sReramLevelxInfo->totalBlocks;
    reramFlash -> lx_nor_flash_words_per_block =            sReramLevelxInfo->SectsPerBlock 
                                                            * sReramLevelxInfo->wordsPerSect;

    /* Setup function pointers for the reram services.  */
    reramFlash -> lx_nor_flash_driver_read =                reramLevelxRead;
    reramFlash -> lx_nor_flash_driver_write =               reramLevelxWrite;
    reramFlash -> lx_nor_flash_driver_block_erase =         reramLevelxErase;
    reramFlash -> lx_nor_flash_driver_block_erased_verify = reramLevelxEraseVerify;

    /* Setup local buffer for reram operation. This buffer must be the sector size of the reram memory.  */
    reramFlash -> lx_nor_flash_sector_buffer =  (ULONG *)sReramLevelxInfo->cache;

    /* Return success.  */
    return(LX_SUCCESS);
}

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
ULONG  reramLevelxLoad(t_ReramLevelxInfo *reramLevelxInfo)
{
    UINT status;

    if (reramLevelxInfo->bInitialed) {   
        LOGE_COLOR(COL_RED, "ReRamLevelx re-loadload error");
        return LX_ERROR;
    }

    sReramLevelxInfo = reramLevelxInfo;


    status = lx_nor_flash_open(reramLevelxInfo->mediaPtr, "reram flash open", reramLevelxInitialize);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "reram lx_nor_flash_open error, errorNum:%d", status);
        return status;
    }

    /* Init success */
    reramLevelxInfo->bInitialed = 1;

    /* Return success.  */
    return(LX_SUCCESS);    
}

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
ULONG  reramLevelxCleanReload(t_ReramLevelxInfo *reramLevelxInfo)
{
    UINT status;


    sReramLevelxInfo = reramLevelxInfo;

    reramLevelxInfo->bInitialed = 0;

    status =  lx_nor_flash_close(reramLevelxInfo->mediaPtr);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "reram lx_nor_flash_close error, errorNum:%d", status);
        return status;
    }

    reramLevelxEraseAllHead(reramLevelxInfo);
    status = lx_nor_flash_open(reramLevelxInfo->mediaPtr, "reram flash open", reramLevelxInitialize);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "reram lx_nor_flash_open error, errorNum:%d", status);
        return status;
    }

    /* Init success */
    reramLevelxInfo->bInitialed = 1;

    /* Return success.  */
    return(LX_SUCCESS);
}

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
ULONG  reramLevelxEraseAllHead(t_ReramLevelxInfo *reramLevelxInfo)
{
    ULONG reramBaseAddr;
    ULONG bytesPerSector;

    reramBaseAddr = reramLevelxInfo->diskBaseAddr;
    bytesPerSector = reramLevelxInfo->wordsPerSect * sizeof(ULONG);

    /* Preparing a sector-sized buffer filled with 0xFF (erase pattern) */
    memset(reramLevelxInfo->cache, 0xff, bytesPerSector);

    /* Loop to erase block.  */
    for (int i = 0; i < reramLevelxInfo->totalBlocks; i++)
    {
        /* Writing the erase pattern to each head sector in every block */
        HAL_RERAM_Write(reramBaseAddr, reramLevelxInfo->cache, bytesPerSector);
        reramBaseAddr  += reramLevelxInfo->SectsPerBlock * bytesPerSector;
    }

    return(LX_SUCCESS);
}

/**
 * @brief  Reads data from ReRAM LevelX storage at specified address.
 * @param  reramAddress Pointer to the starting address in ReRAM to read from.
 * @param  destination Pointer to the buffer where read data will be stored.
 * @param  words Number of 32-bit words to read.
 * @retval LX status code indicating the success of the operation,
 *         LX_SUCCESS means read was successful,
 *         other values indicate errors during the read operation.
 *
 * This function performs a read operation from ReRAM LevelX storage using
 * the hardware abstraction layer (HAL) for ReRAM access.
 */
static UINT reramLevelxRead(ULONG *reramAddress, ULONG *destination, ULONG words)
{
    ULONG baseAddr;

    baseAddr = (ULONG)reramAddress;
    HAL_RERAM_Read(baseAddr, (UCHAR *)destination, words * sizeof(ULONG));

    return(LX_SUCCESS);
}


/**
 * @brief  Writes data to ReRAM LevelX storage at specified address.
 * @param  reramAddress Pointer to the starting address in ReRAM to write to.
 * @param  source Pointer to the buffer containing data to be written.
 * @param  words Number of 32-bit words to write.
 * @retval LX status code indicating the success of the operation,
 *         LX_SUCCESS means write was successful,
 *         other values indicate errors during the write operation.
 *
 * This function performs a write operation to ReRAM LevelX storage using
 * the hardware abstraction layer (HAL) for ReRAM access.
 */
static UINT reramLevelxWrite(ULONG *reramAddress, ULONG *source, ULONG words)
{
    ULONG baseAddr;

    baseAddr = (ULONG)reramAddress;

    HAL_RERAM_Write(baseAddr, (UCHAR *)source, words * sizeof(ULONG));

    return(LX_SUCCESS);
}

/**
 * @brief  Erases a specified block in ReRAM LevelX storage.
 * @param  block Index of the block to be erased.
 * @param  eraseCount Number of times the block has been erased.
 * @retval LX status code indicating the success of the operation,
 *         LX_SUCCESS means erase was successful,
 *         LX_NO_MEMORY means memory allocation failed,
 *         other values indicate errors during the erase operation.
 *
 * This function erases a block by writing 0xFF pattern to the block's header sector.
 * It calculates the block's base address using storage geometry information and
 * uses the hardware abstraction layer (HAL) for ReRAM access.
 */
static UINT reramLevelxErase(ULONG block, ULONG eraseCount)
{
    ULONG baseAddr;
    UCHAR *fillBuff = NULL;
    ULONG bytesPerSector;

    baseAddr = sReramLevelxInfo->diskBaseAddr;

    baseAddr += block * sReramLevelxInfo->SectsPerBlock 
                      * sReramLevelxInfo->wordsPerSect 
                      * sizeof(ULONG);

    bytesPerSector = sReramLevelxInfo->wordsPerSect * sizeof(ULONG);
    fillBuff = malloc(bytesPerSector);

    if (fillBuff == NULL)
    {
        LOGE_COLOR(COL_RED, "ReRamLevelx NO_MEMORY error");
        return(LX_NO_MEMORY);
    }
    memset(fillBuff, 0xff, bytesPerSector);

    /* to erase block head */
    HAL_RERAM_Write(baseAddr, fillBuff, bytesPerSector);

    free(fillBuff);
    return(LX_SUCCESS);
}

/**
 * @brief  Verifies if a specified block in ReRAM LevelX storage is properly erased.
 * @param  block Index of the block to be verified.
 * @retval LX status code indicating the verification result,
 *         LX_SUCCESS means block is properly erased,
 *         LX_NO_MEMORY means memory allocation failed,
 *         LX_ERROR means block verification failed (not properly erased),
 *         other values indicate other errors during verification.
 *
 * This function verifies block erase status by comparing the block's content
 * with 0xFF pattern. It allocates temporary buffers for comparison and uses
 * the hardware abstraction layer (HAL) for ReRAM access.
 */
static UINT reramLevelxEraseVerify(ULONG block)
{

    ULONG baseAddr;
    UCHAR *fillBuff = NULL;
    UCHAR *readBuff = NULL;
    ULONG bytesPerSector;

    baseAddr = sReramLevelxInfo->diskBaseAddr;

    baseAddr += block * sReramLevelxInfo->SectsPerBlock 
                      * sReramLevelxInfo->wordsPerSect 
                      * sizeof(ULONG);

    bytesPerSector = sReramLevelxInfo->wordsPerSect * sizeof(ULONG);

    fillBuff = malloc(bytesPerSector);
    if (fillBuff == NULL)
    {
        LOGE_COLOR(COL_RED, "ReRamLevelx NO_MEMORY error");
        return(LX_NO_MEMORY);
    }
    memset(fillBuff, 0xff, bytesPerSector);

    readBuff = malloc(bytesPerSector);
    if (readBuff == NULL)
    {
        LOGE_COLOR(COL_RED, "ReRamLevelx NO_MEMORY error");
        free(fillBuff);
        return(LX_NO_MEMORY);
    }
    HAL_RERAM_Read(baseAddr, (UCHAR *)readBuff, bytesPerSector);

    /* check if the block is erased */
    if (memcmp(fillBuff, readBuff, bytesPerSector) != 0)
    {
        LOGE_COLOR(COL_RED, "ReRamLevelx erased check error");
        free(fillBuff);
        free(readBuff);
        return(LX_ERROR);
    }

    free(fillBuff);
    free(readBuff);
    /* Return success.  */
    return(LX_SUCCESS);
}