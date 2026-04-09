/**
******************************************************************************
* @file    nandflash_filex_impl.c
* @author  OS Team
* @brief   This file's contents are the adaptation of Filex for the nandflash storage.
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
#include "nandflash_filex_impl.h"
#include "daric_hal_flash.h"
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "NANDFLASH_FX"


#ifdef ENABLE_PERF_LOG
uint32_t cycnt = 0;
uint32_t totalCycnt = 0;
uint32_t cpuFreq;
#endif

/* Define Nandflash device driver entry.  */
static void nandflashFilexDiskDriver(FX_MEDIA *media_ptr);

static HAL_StatusTypeDef nandflashInitSPI() {

    g_flash_spim_handle.baudrate	= CONFIG_FLASH_SPI_BAUDRATE;
    g_flash_spim_handle.id			= CONFIG_FLASH_SPI_ID; // SPIM1
    g_flash_spim_handle.cs_gpio		= -1;
    g_flash_spim_handle.cs			= 0; // CS0
    g_flash_spim_handle.qspi		= 0;
    g_flash_spim_handle.wordsize	= SPIM_WORDSIZE_8;
    g_flash_spim_handle.big_endian	= 1; // 1 means SPI_CMD_MSB_FIRST
    g_flash_spim_handle.polarity	= 1; // mode 3
    g_flash_spim_handle.phase		= 1; // mode 3

    if (HAL_OK != HAL_SPIM_Init(&g_flash_spim_handle)) {
        LOGE("Nandflash SPIM Init failed!");
        return HAL_ERROR;
    } else {
        return HAL_OK;
    }
}

/**
 * @brief Erases the entire NAND Flash disk by erasing all blocks.
 * @param nandflashDiskInfo Pointer to the NAND Flash disk information structure.
 * @retval HAL_StatusTypeDef 
 *         - HAL_OK: Erase operation successful
 *         - HAL_ERROR: Parameter error or erase failure
 *         - Other HAL status codes returned by HAL_FLASH_Erase_BLOCK
 * 
 * This function performs a full erase of the NAND Flash disk by erasing all blocks
 * It first validates the input parameters 
 * and checks if the block-to-sector calculation is correct before
 * proceeding with the erase operation.
 */
static HAL_StatusTypeDef nandflashEraseDisk(t_NandflashDiskInfo *nandflashDiskInfo)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    nandflashDiskInfo->preBlockIdx = INVALID_BLOCK_INDEX;
    nandflashDiskInfo->bWritePending = WRITE_STATUS_NO;

    if (!nandflashDiskInfo)
    {
        LOGE("nandflashDiskInfo is NULL");
        return HAL_ERROR;
    }

    if (nandflashDiskInfo->numbsOfBlock * nandflashDiskInfo->sectsPerBlock != nandflashDiskInfo->totalSects)
    {
        LOGE("params error, numbsOfBlock * sectsPerBlock != totalSects");
        return HAL_ERROR;
    }

    status = HAL_FLASH_Erase_BLOCK(nandflashDiskInfo->nandStartBlock, nandflashDiskInfo->numbsOfBlock);
    if (status != HAL_OK)
    {
        LOGE("erase block errorNo=%d", status);
    }

    return status;
}

/**
 * @brief Reads data from a specified block in NAND Flash memory
 * @param nandflashDiskInfo Pointer to the NAND Flash disk information structure
 * @param phyBlockIdx Index of the block to read
 * @return HAL_StatusTypeDef 
 *         - HAL_OK: Block read successfully
 *         - HAL_ERROR: Parameter error or read failure
 *         - Other HAL status codes returned by HAL_FLASH_Read_QSPI
 * 
 * This function reads the entire contents of a specified NAND Flash block into the 
 * pre-allocated block buffer. It performs sector-by-sector reading using QSPI interface.
 * The function validates input parameters before proceeding with the read operation.
 * 1. The block buffer must be pre-allocated with sufficient size (sectsPerBlock * bytesPerSect)
 * 2. This is a blocking operation - may take significant time for large blocks
 * 3. Sector data is stored contiguously in the block buffer
 */
static HAL_StatusTypeDef nandflashReadBlockData(t_NandflashDiskInfo *nandflashDiskInfo, UCHAR *blkBuffer, USHORT phyBlockIdx)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    #ifdef ENABLE_PERF_LOG
    DWT->CYCCNT = 0;
    #endif

    if (!nandflashDiskInfo)
    {
        LOGE("nandflashDiskInfo is NULL");
        return HAL_ERROR;
    }

    if (!blkBuffer)
    {
        LOGE("blockBuffer is null");
        return HAL_ERROR;
    }

    //blkBuffer = nandflashDiskInfo->blockBuffer;

    for (int i = 0; i < nandflashDiskInfo->sectsPerBlock; i++)
    {
        status = HAL_FLASH_Read_QSPI(blkBuffer, phyBlockIdx, i, 0, nandflashDiskInfo->bytesPerSect);
        if (HAL_OK != status)
        {
            LOGE("HAL_FLASH_Read_QSPI errorNo=%d", status);
            return status;
        }
        blkBuffer += nandflashDiskInfo->bytesPerSect;
    }

    #ifdef ENABLE_PERF_LOG
    cycnt = DWT->CYCCNT / cpuFreq;
    totalCycnt += cycnt;
    LOGV("nandflashReadBlockData \tidx=%d, ms=%ld", phyBlockIdx, cycnt);
    #endif

    return HAL_OK;
}

/**
 * @brief Writes data to a specified block in NAND Flash memory
 * @param nandflashDiskInfo Pointer to the NAND Flash disk information structure
 * @param phyBlockIdx Index of the block to write (0-based)
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Block write successful
 *         - HAL_ERROR: Parameter error or write failure
 *         - Other HAL status codes returned by HAL_FLASH_Write_QSPI
 *
 * This function writes data from the pre-allocated block buffer to a specified NAND Flash block
 * using QSPI interface. It performs sector-by-sector writing and validates input parameters before
 * proceeding with the write operation. The block must be erased before writing.
 * 1. The block must be erased before writing (use nandflashEraseBlockData first)
 * 2. This is a blocking operation - may take significant time for large blocks
 * 3. Data is written sequentially from sector 0 to sector N-1,ref nandflash datasheet 8.2.13
 */
static HAL_StatusTypeDef nandflashWriteBlockData(t_NandflashDiskInfo *nandflashDiskInfo, UCHAR *blkBuffer, USHORT phyBlockIdx)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    #ifdef ENABLE_PERF_LOG
    DWT->CYCCNT = 0;
    #endif
    
    if (!nandflashDiskInfo)
    {
        LOGE("nandflashDiskInfo is NULL");
        return HAL_ERROR;
    }

    if (!blkBuffer)
    {
        LOGE("blockBuffer is null");
        return HAL_ERROR;
    }

    //blkBuffer = nandflashDiskInfo->blockBuffer;

    for (int i = 0; i < nandflashDiskInfo->sectsPerBlock; i++)
    {
        status = HAL_FLASH_Write_QSPI(blkBuffer, phyBlockIdx, i, 0, nandflashDiskInfo->bytesPerSect);
        if (HAL_OK != status)
        {
            LOGE("HAL_FLASH_Write_QSPI errorNo=%d", status);
            return status;
        }
        blkBuffer += nandflashDiskInfo->bytesPerSect;
    }

    #ifdef ENABLE_PERF_LOG
    cycnt = DWT->CYCCNT / cpuFreq;
    totalCycnt += cycnt;
    LOGV("nandflashWriteBlockData \tidx=%d, ms=%ld", phyBlockIdx, cycnt);
    #endif

    return HAL_OK;
}

/**
 * @brief Erases a specified block in NAND Flash memory
 * @param nandflashDiskInfo Pointer to the NAND Flash disk information structure
 * @param phyBlockIdx Index of the block to erase 
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Block erase successful
 *         - HAL_ERROR: Parameter error or erase failure
 *         - Other HAL status codes returned by HAL_FLASH_Erase_BLOCK
 *
 * This function erases a single block in NAND Flash memory, preparing it for new data writes.
 * Block indices must be within valid range for the device
 */
static HAL_StatusTypeDef nandflashEraseBlockData(t_NandflashDiskInfo *nandflashDiskInfo, USHORT phyBlockIdx)
{
    HAL_StatusTypeDef status = HAL_ERROR;

    #ifdef ENABLE_PERF_LOG
    DWT->CYCCNT = 0;
    #endif

    if (!nandflashDiskInfo)
    {
        LOGE("nandflashDiskInfo is NULL");
        return HAL_ERROR;
    }

    if (!nandflashDiskInfo->blockBuffer)
    {
        LOGE("blockBuffer is null");
        return HAL_ERROR;
    }

    status = HAL_FLASH_Erase_BLOCK(phyBlockIdx, 1);
    if (HAL_OK != status)
    {
        LOGE("HAL_FLASH_Erase_BLOCK errorNo=%d", status);
        return status;
    }

    #ifdef ENABLE_PERF_LOG
    cycnt = DWT->CYCCNT / cpuFreq;
    totalCycnt += cycnt;
    LOGV("nandflashEraseBlockData \tidx=%d, ms=%ld", phyBlockIdx, cycnt);
    #endif

    return HAL_OK;
}

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
uint32_t nandflashFilexDiskLoad(t_NandflashDiskInfo *nandflashDiskInfo)
{
    //static UCHAR bInited = 0;
    UINT  status;
    
    if (nandflashDiskInfo->bInitialed)
    {
        LOGE("NandflashDisk re-initialization error");
        return FX_INVALID_STATE;
    }

    nandflashDiskInfo->preBlockIdx = INVALID_BLOCK_INDEX;
    nandflashDiskInfo->bWritePending = WRITE_STATUS_NO;

    status = nandflashInitSPI();
    if (HAL_OK != status)
    {
        LOGE("NandflashDisk SPIM Init errorN=%d", status);
        return FX_INVALID_STATE;
    }

    status = HAL_FLASH_Init();
    if (HAL_OK != status)
    {
        LOGE("HAL_FLASH_Init Init errorN=%d", status);
        return FX_INVALID_STATE;
    }

    //return FX_SUCCESS;
    //fx_system_initialize();
    /* Open the Nandflash disk.  */
    status =  fx_media_open(nandflashDiskInfo->mediaPtr,
                            nandflashDiskInfo->volName,
                            nandflashFilexDiskDriver,
                            nandflashDiskInfo,
                            nandflashDiskInfo->cache,
                            nandflashDiskInfo->cacheSize);

    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {
        /* [TODO]This is a temporary solution. 
           The correct approach should be to format the disk within a function like a "setup wizard," 
           and clearly inform the user that data may be lost. 
        */
        /* Format the nandflash disk.
           Because the file system's cache needs to be continuously used 
           and does not require frequent allocation and deallocation, 
           so it is recommended to use static allocation for the cache here.  
        */
        if (nandflashEraseDisk(nandflashDiskInfo) != HAL_OK)
        {
            LOGE("eraseNandDisk return error");
            return FX_INVALID_STATE;
        }

        status = fx_media_format(nandflashDiskInfo->mediaPtr,
                        nandflashFilexDiskDriver,       // Driver entry
                        nandflashDiskInfo,        // RAM disk memory pointer
                        nandflashDiskInfo->cache,           // Media buffer pointer
                        nandflashDiskInfo->cacheSize,   // Media buffer size
                        nandflashDiskInfo->volName,           // Volume Name
                        nandflashDiskInfo->numOfFats,       // Number of FATs
                        nandflashDiskInfo->dirEntries,   // Directory Entries
                        nandflashDiskInfo->hiddenSects,      // Hidden sectors
                        nandflashDiskInfo->totalSects,       // Total sectors
                        nandflashDiskInfo->bytesPerSect,         // Sector size
                        nandflashDiskInfo->sectPerCluster,    // Sectors per cluster
                        1,                      // Heads
                        1);                     // Sectors per track
        if (status != FX_SUCCESS)
        {
            LOGE("fx_media_format return error %d", status);
            return status;
        }
        status =  fx_media_open(nandflashDiskInfo->mediaPtr,
                                nandflashDiskInfo->volName,
                                nandflashFilexDiskDriver,
                                nandflashDiskInfo,
                                nandflashDiskInfo->cache,
                                nandflashDiskInfo->cacheSize);
        /* Check the media open status.  */
        if (status != FX_SUCCESS)
        {
            LOGE("fx_media_open return error %d", status);
            return status;
        }
    }

    nandflashDiskInfo->bInitialed = 1;

    return FX_SUCCESS;
}

// static uint32_t  nandflashFilexDiskDriverWrite(FX_MEDIA *media_ptr)
// {
//     return FX_SUCCESS;
// }

// static uint32_t  nandflashFilexDiskDriverRead(FX_MEDIA *media_ptr)
// {
//     return FX_SUCCESS;
// }

/**
 * @brief  This function is the entry point to the nandflash disk driver.
 * @param  media_ptr FX_MEDIA structure defines everything required to manage
 *         a media device. This structure contains all media information,
 *         including the media-specific I/O driver and associated parameters
 *         for passing information and status between the driver and FileX.
 * @retval The I/O driver communicates the success or failure of the request
 *         through the fx_media_driver_status member of FX_MEDIA. 
 *         If the driver request was successful, 
 *         FX_SUCCESS is placed in this field before the driver returns. 
 *         Otherwise, if an error is detected, FX_IO_ERROR is placed in this field.
 * 
 * FileX calls the I/O driver entry function to 
 * request initialization and boot sector reading. 
 */
static void  nandflashFilexDiskDriver(FX_MEDIA *media_ptr)
{

    ULONG   logiSect;
    ULONG   sectCount;
    UCHAR   *buffer;
    UCHAR   *blockBuffer;
    USHORT  phyBlockIdx;
    UCHAR   pageIdx;
    UINT    bytes_per_sector;
    HAL_StatusTypeDef status;
    t_NandflashDiskInfo *nandflashDiskInfo = NULL;

    nandflashDiskInfo = (t_NandflashDiskInfo *)media_ptr->fx_media_driver_info;

    /* Process the driver request specified in the media control block.  */
    switch (media_ptr -> fx_media_driver_request)
    {

        case FX_DRIVER_READ:
        {

            /* Read sectors from nand flash.
            Note the nandflash disk info is pointed to by
            the fx_media_driver_info pointer, which is supplied by 
            the application in the call to fx_media_open.  
            */
            logiSect    = media_ptr->fx_media_driver_logical_sector;
            sectCount   = media_ptr->fx_media_driver_sectors;
            buffer      = media_ptr->fx_media_driver_buffer;
            
            for (int i = 0; i < sectCount; i++)
            {
                phyBlockIdx    = logiSect / nandflashDiskInfo->sectsPerBlock;
                phyBlockIdx    += nandflashDiskInfo->nandStartBlock;
                pageIdx     = logiSect % nandflashDiskInfo->sectsPerBlock;

                if (nandflashDiskInfo->extBlkBuffer && phyBlockIdx == nandflashDiskInfo->extBlockIdx)
                {
                    if (nandflashDiskInfo->preExtBlockIdx == INVALID_BLOCK_INDEX)
                    {
                        /*
                         * If it is the first access, the specified block needs to be pre-read
                         */
                        status = nandflashReadBlockData(nandflashDiskInfo,
                                                        nandflashDiskInfo->extBlkBuffer,
                                                        phyBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }
                        nandflashDiskInfo->preExtBlockIdx = phyBlockIdx;
                        nandflashDiskInfo->bExtWritePending = WRITE_STATUS_NO;
                    }
                    blockBuffer = nandflashDiskInfo->extBlkBuffer + pageIdx * nandflashDiskInfo->bytesPerSect;
                    memcpy(buffer, blockBuffer, nandflashDiskInfo->bytesPerSect);

                    logiSect++;
                    buffer += media_ptr->fx_media_bytes_per_sector;
                }
                else
                {
                    if (phyBlockIdx != nandflashDiskInfo->preBlockIdx)
                    {
                        if (nandflashDiskInfo->preBlockIdx != INVALID_BLOCK_INDEX
                            && nandflashDiskInfo->bWritePending == WRITE_STATUS_PENDING)
                        {
                            /*
                             * The content of the previous accessed block is still in the cache.
                             * Before reading other blocks,
                             * first erase the previous block,
                             * write the data of the previous block,
                             * and then read the current block into the cache.
                             */
                            status = nandflashEraseBlockData(nandflashDiskInfo, nandflashDiskInfo->preBlockIdx);
                            if (HAL_OK != status)
                            {
                                /* return a io_error to filex */
                                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                                LOGE("nandflashEraseBlockData return errorNo=%d, line=%d", status, __LINE__);
                                return;
                            }

                            status = nandflashWriteBlockData(nandflashDiskInfo,
                                                             nandflashDiskInfo->blockBuffer,
                                                             nandflashDiskInfo->preBlockIdx);
                            if (HAL_OK != status)
                            {
                                /* return a io_error to filex */
                                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                                LOGE("nandflashWriteBlockData return errorNo=%d, line=%d", status, __LINE__);
                                return;
                            }

                            status = nandflashReadBlockData(nandflashDiskInfo,
                                                            nandflashDiskInfo->blockBuffer,
                                                            phyBlockIdx);
                            if (HAL_OK != status)
                            {
                                /* return a io_error to filex */
                                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                                LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                                return;
                            }
                            nandflashDiskInfo->bWritePending = WRITE_STATUS_NO;
                        }
                        else
                        {
                            /*
                             * If it is the first access, the specified block needs to be pre-read
                             */
                            status = nandflashReadBlockData(nandflashDiskInfo,
                                                            nandflashDiskInfo->blockBuffer,
                                                            phyBlockIdx);
                            if (HAL_OK != status)
                            {
                                /* return a io_error to filex */
                                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                                LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                                return;
                            }
                        }

                        nandflashDiskInfo->preBlockIdx = phyBlockIdx;
                    }

                    blockBuffer = nandflashDiskInfo->blockBuffer + pageIdx * nandflashDiskInfo->bytesPerSect;
                    memcpy(buffer, blockBuffer, nandflashDiskInfo->bytesPerSect);

                    // status = HAL_FLASH_Read_QSPI(buffer, phyBlockIdx, pageIdx, 0, nandflashDiskInfo->bytesPerSect);
                    // if (HAL_OK != status)
                    // {
                    //     /* return a io_error to filex */
                    //     media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    //     LOGE("HAL_FLASH_Read_QSPI return errorNo=%d", status);
                    //     return;
                    // }
                    logiSect++;
                    buffer += media_ptr->fx_media_bytes_per_sector;
                }
            }

            /* Successful driver request.  */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        case FX_DRIVER_WRITE:
        {

            /* write sectors to nand flash.
            Note the nandflash disk info is pointed to by the fx_media_driver_info pointer,
            which is supplied by the application in the call to fx_media_open.  
            */

            logiSect    = media_ptr->fx_media_driver_logical_sector;
            sectCount   = media_ptr->fx_media_driver_sectors;
            buffer      = media_ptr->fx_media_driver_buffer;

            for (int i = 0; i < sectCount; i++)
            {
                phyBlockIdx    = logiSect / nandflashDiskInfo->sectsPerBlock;
                phyBlockIdx    += nandflashDiskInfo->nandStartBlock;
                pageIdx     = logiSect % nandflashDiskInfo->sectsPerBlock;

                if (nandflashDiskInfo->extBlkBuffer && phyBlockIdx == nandflashDiskInfo->extBlockIdx)
                {
                    if (nandflashDiskInfo->preExtBlockIdx == INVALID_BLOCK_INDEX)
                    {
                        /*
                         * If it is the first access, the specified block needs to be pre-read
                         */
                        status = nandflashReadBlockData(nandflashDiskInfo,
                                                        nandflashDiskInfo->extBlkBuffer,
                                                        phyBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }
                        nandflashDiskInfo->preExtBlockIdx = phyBlockIdx;
                    }
                    blockBuffer = nandflashDiskInfo->extBlkBuffer + pageIdx * nandflashDiskInfo->bytesPerSect;
                    memcpy(blockBuffer, buffer, nandflashDiskInfo->bytesPerSect);

                    logiSect++;
                    buffer += media_ptr->fx_media_bytes_per_sector;

                    nandflashDiskInfo->bExtWritePending = WRITE_STATUS_PENDING;
                }
                else
                {
                    if (phyBlockIdx != nandflashDiskInfo->preBlockIdx)
                    {
                        if (nandflashDiskInfo->preBlockIdx != INVALID_BLOCK_INDEX
                            && nandflashDiskInfo->bWritePending == WRITE_STATUS_PENDING)
                        {
                            /*
                             * The content of the previous accessed block is still in the cache.
                             * Before reading other blocks,
                             * first erase the previous block,
                             * write the data of the previous block,
                             * and then read the current block into the cache.
                             */
                            status = nandflashEraseBlockData(nandflashDiskInfo, nandflashDiskInfo->preBlockIdx);
                            if (HAL_OK != status)
                            {
                                /* return a io_error to filex */
                                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                                LOGE("nandflashEraseBlockData return errorNo=%d, line=%d", status, __LINE__);
                                return;
                            }

                            status = nandflashWriteBlockData(nandflashDiskInfo,
                                                             nandflashDiskInfo->blockBuffer,
                                                             nandflashDiskInfo->preBlockIdx);
                            if (HAL_OK != status)
                            {
                                /* return a io_error to filex */
                                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                                LOGE("nandflashWriteBlockData return errorNo=%d, line=%d", status, __LINE__);
                                return;
                            }

                            status = nandflashReadBlockData(nandflashDiskInfo,
                                                            nandflashDiskInfo->blockBuffer,
                                                            phyBlockIdx);
                            if (HAL_OK != status)
                            {
                                /* return a io_error to filex */
                                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                                LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                                return;
                            }
                        }
                        else
                        {
                            status = nandflashReadBlockData(nandflashDiskInfo,
                                                            nandflashDiskInfo->blockBuffer,
                                                            phyBlockIdx);
                            if (HAL_OK != status)
                            {
                                /* return a io_error to filex */
                                media_ptr->fx_media_driver_status = FX_IO_ERROR;
                                LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                                return;
                            }
                        }

                        nandflashDiskInfo->preBlockIdx = phyBlockIdx;
                    }

                    blockBuffer = nandflashDiskInfo->blockBuffer + pageIdx * nandflashDiskInfo->bytesPerSect;
                    memcpy(blockBuffer, buffer, nandflashDiskInfo->bytesPerSect);

                    logiSect++;
                    buffer += media_ptr->fx_media_bytes_per_sector;
                    nandflashDiskInfo->bWritePending = WRITE_STATUS_PENDING;

                }
            }


            /* Successful driver request.  */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        case FX_DRIVER_FLUSH:
        {
            if (nandflashDiskInfo->extBlkBuffer)
            {
                if (nandflashDiskInfo->preExtBlockIdx != INVALID_BLOCK_INDEX
                    && nandflashDiskInfo->bExtWritePending == WRITE_STATUS_PENDING)
                {
                    /*
                    * The content of the ext cached block is still in the cache.
                    * first erase the previous block,
                    * write the data of the previous block.
                    */
                    status = nandflashEraseBlockData(nandflashDiskInfo, nandflashDiskInfo->extBlockIdx);
                    if (HAL_OK != status)
                    {
                        /* return a io_error to filex */
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        LOGE("nandflashEraseBlockData return errorNo=%d, line=%d", status, __LINE__);
                        return;
                    }

                    status = nandflashWriteBlockData(nandflashDiskInfo,
                                                     nandflashDiskInfo->extBlkBuffer,
                                                     nandflashDiskInfo->extBlockIdx);
                    if (HAL_OK != status)
                    {
                        /* return a io_error to filex */
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        LOGE("nandflashWriteBlockData return errorNo=%d, line=%d", status, __LINE__);
                        return;
                    }

                }
                nandflashDiskInfo->bExtWritePending = WRITE_STATUS_NO;
            }

            if (nandflashDiskInfo->preBlockIdx != INVALID_BLOCK_INDEX
                && nandflashDiskInfo->bWritePending == WRITE_STATUS_PENDING)
            {
                /*
                * The content of the previous accessed block is still in the cache.
                * first erase the previous block,
                * write the data of the previous block.
                */
                status = nandflashEraseBlockData(nandflashDiskInfo, nandflashDiskInfo->preBlockIdx);
                if (HAL_OK != status)
                {
                    /* return a io_error to filex */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    LOGE("nandflashEraseBlockData return errorNo=%d, line=%d", status, __LINE__);
                    return;
                }

                status = nandflashWriteBlockData(nandflashDiskInfo,
                                                 nandflashDiskInfo->blockBuffer,
                                                 nandflashDiskInfo->preBlockIdx);
                if (HAL_OK != status)
                {
                    /* return a io_error to filex */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    LOGE("nandflashWriteBlockData return errorNo=%d, line=%d", status, __LINE__);
                    return;
                }
            }
            nandflashDiskInfo->bWritePending = WRITE_STATUS_NO;


            /* Return driver success.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_ABORT:
        {
            nandflashDiskInfo->preBlockIdx      = INVALID_BLOCK_INDEX;
            nandflashDiskInfo->preExtBlockIdx   = INVALID_BLOCK_INDEX;
            nandflashDiskInfo->bWritePending    = WRITE_STATUS_NO;
            nandflashDiskInfo->bExtWritePending = WRITE_STATUS_NO;
            /* Return driver success.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_INIT:
        {

            #ifdef ENABLE_PERF_LOG
            cpuFreq = HAL_GetCoreClkMHz() * 1000;
            CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
            DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
            #endif

            /* Perform basic initialization here.
            since the boot record is going to be read subsequently 
            and again for volume name requests.  
            */
            nandflashDiskInfo->preBlockIdx      = INVALID_BLOCK_INDEX;
            nandflashDiskInfo->preExtBlockIdx   = INVALID_BLOCK_INDEX;
            nandflashDiskInfo->bWritePending    = WRITE_STATUS_NO;
            nandflashDiskInfo->bExtWritePending = WRITE_STATUS_NO;

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_UNINIT:
        {
            if (nandflashDiskInfo->extBlkBuffer)
            {
                if (nandflashDiskInfo->preExtBlockIdx != INVALID_BLOCK_INDEX
                    && nandflashDiskInfo->bExtWritePending == WRITE_STATUS_PENDING)
                {
                    /*
                    * The content of the ext cached block is still in the cache.
                    * first erase the previous block,
                    * write the data of the previous block.
                    */
                    status = nandflashEraseBlockData(nandflashDiskInfo, nandflashDiskInfo->extBlockIdx);
                    if (HAL_OK != status)
                    {
                        /* return a io_error to filex */
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        LOGE("nandflashEraseBlockData return errorNo=%d, line=%d", status, __LINE__);
                        return;
                    }

                    status = nandflashWriteBlockData(nandflashDiskInfo,
                                                     nandflashDiskInfo->extBlkBuffer,
                                                     nandflashDiskInfo->extBlockIdx);
                    if (HAL_OK != status)
                    {
                        /* return a io_error to filex */
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        LOGE("nandflashWriteBlockData return errorNo=%d, line=%d", status, __LINE__);
                        return;
                    }

                }
                nandflashDiskInfo->bExtWritePending = WRITE_STATUS_NO;
            }

            if (nandflashDiskInfo->preBlockIdx != INVALID_BLOCK_INDEX
                && nandflashDiskInfo->bWritePending == WRITE_STATUS_PENDING)
            {
                /*
                    * The content of the previous accessed block is still in the cache.
                    * first erase the previous block,
                    * write the data of the previous block.
                    */
                status = nandflashEraseBlockData(nandflashDiskInfo, nandflashDiskInfo->preBlockIdx);
                if (HAL_OK != status)
                {
                    /* return a io_error to filex */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    LOGE("nandflashEraseBlockData return errorNo=%d, line=%d", status, __LINE__);
                    return;
                }

                status = nandflashWriteBlockData(nandflashDiskInfo,
                                                 nandflashDiskInfo->blockBuffer,
                                                 nandflashDiskInfo->preBlockIdx);
                if (HAL_OK != status)
                {
                    /* return a io_error to filex */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    LOGE("nandflashWriteBlockData return errorNo=%d, line=%d", status, __LINE__);
                    return;
                }
            }

            nandflashDiskInfo->preBlockIdx      = INVALID_BLOCK_INDEX;
            nandflashDiskInfo->preExtBlockIdx   = INVALID_BLOCK_INDEX;
            nandflashDiskInfo->bWritePending    = WRITE_STATUS_NO;
            nandflashDiskInfo->bExtWritePending = WRITE_STATUS_NO;

            /* Successful driver request. */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_BOOT_READ:
        {

            /* Read the boot record and return to the caller.  */

            /* Calculate the nandflash disk boot sector offset,
            which is at the very beginning of the nandflash disk.
            the nandflash disk memory is pointed to by the fx_media_driver_info pointer,
            which is supplied by the application in the call to fx_media_open.  
            */

            /* Ensure that the size of the array exceeds FX_BYTES_SECTOR=0x0B */
            UCHAR tmpBuff[16]; 
            buffer      = media_ptr->fx_media_driver_buffer;

            phyBlockIdx    = 0;
            phyBlockIdx    += nandflashDiskInfo->nandStartBlock;
            pageIdx     = 0;

            if (nandflashDiskInfo->extBlkBuffer && phyBlockIdx == nandflashDiskInfo->extBlockIdx)
            {
                if (nandflashDiskInfo->preExtBlockIdx == INVALID_BLOCK_INDEX)
                {
                    /*
                     * If it is the first access, the specified block needs to be pre-read
                     */
                    status = nandflashReadBlockData(nandflashDiskInfo,
                                                    nandflashDiskInfo->extBlkBuffer,
                                                    phyBlockIdx);
                    if (HAL_OK != status)
                    {
                        /* return a io_error to filex */
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                        return;
                    }
                    nandflashDiskInfo->preExtBlockIdx = phyBlockIdx;
                    nandflashDiskInfo->bExtWritePending = WRITE_STATUS_NO;
                }
                blockBuffer = nandflashDiskInfo->extBlkBuffer + pageIdx * nandflashDiskInfo->bytesPerSect;
                memcpy(buffer, blockBuffer, nandflashDiskInfo->bytesPerSect);

            }
            else
            {
                /* Copy the nandflash boot sector into the destination. */
                if (phyBlockIdx != nandflashDiskInfo->preBlockIdx)
                {
                    if (nandflashDiskInfo->preBlockIdx != INVALID_BLOCK_INDEX
                        && nandflashDiskInfo->bWritePending == WRITE_STATUS_PENDING)
                    {
                        /*
                            * The content of the previous accessed block is still in the cache.
                            * Before reading other blocks,
                            * first erase the previous block,
                            * write the data of the previous block,
                            * and then read the current block into the cache.
                            */
                        status = nandflashEraseBlockData(nandflashDiskInfo, nandflashDiskInfo->preBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashEraseBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }

                        status = nandflashWriteBlockData(nandflashDiskInfo,
                                                         nandflashDiskInfo->blockBuffer,
                                                         nandflashDiskInfo->preBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashWriteBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }

                        status = nandflashReadBlockData(nandflashDiskInfo,
                                                        nandflashDiskInfo->blockBuffer,
                                                        phyBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }
                        nandflashDiskInfo->bWritePending = WRITE_STATUS_NO;
                    }
                    else
                    {
                        /*
                            * If it is the first access, the specified block needs to be pre-read
                            */
                        status = nandflashReadBlockData(nandflashDiskInfo,
                                                        nandflashDiskInfo->blockBuffer,
                                                        phyBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }
                    }

                    nandflashDiskInfo->preBlockIdx = phyBlockIdx;
                }

                blockBuffer = nandflashDiskInfo->blockBuffer + pageIdx * nandflashDiskInfo->bytesPerSect;
                memcpy(buffer, blockBuffer, nandflashDiskInfo->bytesPerSect);
            }

            memcpy(tmpBuff, buffer, sizeof(tmpBuff));

            /* For nandflash driver, determine if the boot record is valid.  */
            if ((tmpBuff[0] != (UCHAR)0xEB)  ||
                ((tmpBuff[1] != (UCHAR)0x34)  &&
                (tmpBuff[1] != (UCHAR)0x76)) ||
                (tmpBuff[2] != (UCHAR)0x90))
            {

                /* Invalid boot record, return an error!  */
                media_ptr -> fx_media_driver_status =  FX_MEDIA_INVALID;
                LOGE("invalid fat format");
                return;
            }

            /* For nandflash disk only, pickup the bytes per sector.  */
            bytes_per_sector =  _fx_utility_16_unsigned_read(&tmpBuff[FX_BYTES_SECTOR]);


            /* Ensure this is less than the media memory size.  */
            if (bytes_per_sector > media_ptr -> fx_media_memory_size)
            {
                media_ptr -> fx_media_driver_status =  FX_BUFFER_ERROR;
                break;
            }

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_BOOT_WRITE:
        {

            /* Write the boot record and return to the caller.  */

            /* Calculate the nandflash disk boot sector offset,
            which is at the very beginning of the nandflash disk.
            Note the nandflash disk memory is pointed to by the fx_media_driver_info pointer,
            which is supplied by the application in the call to fx_media_open.  
            */
            buffer      = media_ptr->fx_media_driver_buffer;
            blockBuffer = nandflashDiskInfo->blockBuffer;

            phyBlockIdx    = 0;
            phyBlockIdx    += nandflashDiskInfo->nandStartBlock;
            pageIdx     = 0;

            if (nandflashDiskInfo->extBlkBuffer && phyBlockIdx == nandflashDiskInfo->extBlockIdx)
            {
                if (nandflashDiskInfo->preExtBlockIdx == INVALID_BLOCK_INDEX)
                {
                    /*
                     * If it is the first access, the specified block needs to be pre-read
                     */
                    status = nandflashReadBlockData(nandflashDiskInfo,
                                                    nandflashDiskInfo->extBlkBuffer,
                                                    phyBlockIdx);
                    if (HAL_OK != status)
                    {
                        /* return a io_error to filex */
                        media_ptr->fx_media_driver_status = FX_IO_ERROR;
                        LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                        return;
                    }
                    nandflashDiskInfo->preExtBlockIdx = phyBlockIdx;
                }
                blockBuffer = nandflashDiskInfo->extBlkBuffer + pageIdx * nandflashDiskInfo->bytesPerSect;
                memcpy(blockBuffer, buffer, nandflashDiskInfo->bytesPerSect);

                nandflashDiskInfo->bExtWritePending = WRITE_STATUS_PENDING;
            }
            else
            {
                /* write sectors to nand flash.
                Note the nandflash disk info is pointed to by the fx_media_driver_info pointer,
                which is supplied by the application in the call to fx_media_open.
                */
                if (phyBlockIdx != nandflashDiskInfo->preBlockIdx)
                {
                    if (nandflashDiskInfo->preBlockIdx != INVALID_BLOCK_INDEX
                        && nandflashDiskInfo->bWritePending == WRITE_STATUS_PENDING)
                    {
                        /*
                            * The content of the previous accessed block is still in the cache.
                            * Before reading other blocks,
                            * first erase the previous block,
                            * write the data of the previous block,
                            * and then read the current block into the cache.
                            */
                        status = nandflashEraseBlockData(nandflashDiskInfo, nandflashDiskInfo->preBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashEraseBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }

                        status = nandflashWriteBlockData(nandflashDiskInfo,
                                                         nandflashDiskInfo->blockBuffer,
                                                         nandflashDiskInfo->preBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashWriteBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }

                        status = nandflashReadBlockData(nandflashDiskInfo,
                                                        nandflashDiskInfo->blockBuffer,
                                                        phyBlockIdx);
                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }
                    }
                    else
                    {
                        status = nandflashReadBlockData(nandflashDiskInfo,
                                                        nandflashDiskInfo->blockBuffer,
                                                        phyBlockIdx);

                        if (HAL_OK != status)
                        {
                            /* return a io_error to filex */
                            media_ptr->fx_media_driver_status = FX_IO_ERROR;
                            LOGE("nandflashReadBlockData return errorNo=%d, line=%d", status, __LINE__);
                            return;
                        }
                    }

                    nandflashDiskInfo->preBlockIdx = phyBlockIdx;
                }

                blockBuffer = nandflashDiskInfo->blockBuffer + pageIdx * nandflashDiskInfo->bytesPerSect;
                memcpy(blockBuffer, buffer, nandflashDiskInfo->bytesPerSect);

                nandflashDiskInfo->bWritePending = WRITE_STATUS_PENDING;
            }
            /* Successful driver request. */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        default:
        {

            /* Invalid driver request.  */
            media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
            break;
        }
    }
}

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
uint32_t nandflashFilexDiskFormat(t_NandflashDiskInfo *nandflashDiskInfo)
{
    UINT  status;
    
    //fx_system_initialize();
    if (nandflashDiskInfo->bInitialed)
    {
        nandflashDiskInfo->bInitialed = 0;
        status = fx_media_close(nandflashDiskInfo->mediaPtr);
        if (status != FX_SUCCESS)
        {
            LOGE("nandflash fx_media_close error %d", status);
            return status;
        }
    }
    nandflashDiskInfo->preBlockIdx = INVALID_BLOCK_INDEX;
    nandflashDiskInfo->bWritePending = WRITE_STATUS_NO;

    /* Format the nandflash disk.
        Because the file system's cache needs to be continuously used 
        and does not require frequent allocation and deallocation, 
        so it is recommended to use static allocation for the cache here.  
    */
    if (nandflashEraseDisk(nandflashDiskInfo) != HAL_OK)
    {
        LOGE("eraseNandDisk return error");
        return FX_INVALID_STATE;
    }

    status = fx_media_format(nandflashDiskInfo->mediaPtr,
                    nandflashFilexDiskDriver,       // Driver entry
                    nandflashDiskInfo,        // RAM disk memory pointer
                    nandflashDiskInfo->cache,           // Media buffer pointer
                    nandflashDiskInfo->cacheSize,   // Media buffer size
                    nandflashDiskInfo->volName,           // Volume Name
                    nandflashDiskInfo->numOfFats,       // Number of FATs
                    nandflashDiskInfo->dirEntries,   // Directory Entries
                    nandflashDiskInfo->hiddenSects,      // Hidden sectors
                    nandflashDiskInfo->totalSects,       // Total sectors
                    nandflashDiskInfo->bytesPerSect,         // Sector size
                    nandflashDiskInfo->sectPerCluster,    // Sectors per cluster
                    1,                      // Heads
                    1);                     // Sectors per track
    if (status != FX_SUCCESS)
    {
        LOGE("fx_media_format return error %d", status);
        return status;
    }

    //status = fx_media_flush(nandflashDiskInfo->mediaPtr);

    status = fx_media_open(nandflashDiskInfo->mediaPtr,
                            nandflashDiskInfo->volName,
                            nandflashFilexDiskDriver,
                            nandflashDiskInfo,
                            nandflashDiskInfo->cache,
                            nandflashDiskInfo->cacheSize);
    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {
        LOGE("fx_media_open return error %d", status);
        return status;
    }

    nandflashDiskInfo->bInitialed = 1;

    return FX_SUCCESS;
}

void nandflashFilexTest(FX_MEDIA *pNandflashDisk) {
    UINT  status;
    CHAR  local_buffer[30];
    ULONG actual;
    FX_FILE my_file;

    status =  fx_file_create(pNandflashDisk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

    }

    /* Open the test file.  */
    status =  fx_file_open(pNandflashDisk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

    /* Check the file open status.  */
    if (status != FX_SUCCESS)
    {

        /* Error opening file, break the loop.  */
        return;
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        /* Error performing file seek, break the loop.  */
        return;
    }
    #if 0
    /* Write a string to the test file.  */
    status =  fx_file_write(&my_file, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);

    /* Check the file write status.  */
    if (status != FX_SUCCESS)
    {

        /* Error writing to a file, break the loop.  */
        return;
    }

    /* Seek to the beginning of the test file.  */
    status =  fx_file_seek(&my_file, 0);

    /* Check the file seek status.  */
    if (status != FX_SUCCESS)
    {

        /* Error performing file seek, break the loop.  */
        return;
    }
    #endif

    /* Read the first 28 bytes of the test file.  */
    status =  fx_file_read(&my_file, local_buffer, 28, &actual);

    /* Check the file read status.  */
    if ((status != FX_SUCCESS) || (actual != 28))
    {

        /* Error reading file, break the loop.  */
        return;
    }

    /* Close the test file.  */
    status =  fx_file_close(&my_file);

    /* Check the file close status.  */
    if (status != FX_SUCCESS)
    {

        /* Error closing the file, break the loop.  */
        return;
    }

    /* Close the media.  */
    status =  fx_media_close(pNandflashDisk);

    /* Check the media close status.  */
    if (status != FX_SUCCESS)
    {

        /* Error closing the media, break the loop.  */
        return;
    }


    return;
}
