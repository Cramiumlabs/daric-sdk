/**
******************************************************************************
* @file    reram_filex_impl.c
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
#include "reram_filex_impl.h"
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "RERAM_FX"


/* Define RAM device driver entry.  */
static void reRamFilexDiskDriver(FX_MEDIA *media_ptr);

/**
 * @brief  Initializes the filex disk info according to the specified parameters.
 * @param  reramDiskInfo Pointer to a t_ReramDiskInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function configures the filex disk according to the parameters specified
 * in the t_ReramDiskInfo structure, including cache, numOfFats, and other
 * relevant settings.
 */
uint32_t reRamFilexDiskLoad(t_ReramDiskInfo *reramDiskInfo)
{
    //static UCHAR bInited = 0;
    UINT  status;
    
    if (reramDiskInfo->bInitialed) {
        LOGE("ReRamDisk re-initialization error");
        return FX_INVALID_STATE;
    }
    
    //fx_system_initialize();
    /* Open the RAM disk.  */
    status =  fx_media_open(reramDiskInfo->mediaPtr, 
                            reramDiskInfo->volName, 
                            reRamFilexDiskDriver, 
                            reramDiskInfo, 
                            reramDiskInfo->cache, 
                            reramDiskInfo->cacheSize);

    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {
        /* [TODO]This is a temporary solution. 
           The correct approach should be to format the disk within a function like a "setup wizard," 
           and clearly inform the user that data may be lost. 
        */
        /* Format the ReRAM disk.
           Because the file system's cache needs to be continuously used 
           and does not require frequent allocation and deallocation, 
           so it is recommended to use static allocation for the cache here.  
        */
        status = fx_media_format(reramDiskInfo->mediaPtr,
                        reRamFilexDiskDriver,       // Driver entry
                        reramDiskInfo,        // RAM disk memory pointer
                        reramDiskInfo->cache,           // Media buffer pointer
                        reramDiskInfo->cacheSize,   // Media buffer size
                        reramDiskInfo->volName,           // Volume Name
                        reramDiskInfo->numOfFats,       // Number of FATs
                        reramDiskInfo->dirEntries,   // Directory Entries
                        reramDiskInfo->hiddenSects,      // Hidden sectors
                        reramDiskInfo->totalSects,       // Total sectors
                        reramDiskInfo->bytesPerSect,         // Sector size
                        reramDiskInfo->sectPerCluster,    // Sectors per cluster
                        1,                      // Heads
                        1);                     // Sectors per track
        if (status != FX_SUCCESS)
        {
            LOGE("fx_media_format return error %d", status);
            return status;
        }
        status =  fx_media_open(reramDiskInfo->mediaPtr,
                                reramDiskInfo->volName,
                                reRamFilexDiskDriver,
                                reramDiskInfo,
                                reramDiskInfo->cache,
                                reramDiskInfo->cacheSize);
        /* Check the media open status.  */
        if (status != FX_SUCCESS)
        {
            LOGE("fx_media_open return error %d", status);
            return status;
        }
    }

    reramDiskInfo->bInitialed = 1;

    return FX_SUCCESS;
}



/**
 * @brief  This function is the entry point to the ReRam disk driver.
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
static void  reRamFilexDiskDriver(FX_MEDIA *media_ptr)
{

    //UCHAR   *source_buffer;
    //UCHAR   *destination_buffer;
    UINT    bytes_per_sector;
    UINT    opReRamAddr;
    UINT    opSize;
    t_ReramDiskInfo *lReRamDiskInfo = NULL;

    lReRamDiskInfo = (t_ReramDiskInfo *)media_ptr->fx_media_driver_info;

    /* Process the driver request specified in the media control block.  */
    switch (media_ptr -> fx_media_driver_request)
    {

        case FX_DRIVER_READ:
        {

            /* Calculate the ReRAM disk sector offset. 
            Note the ReRAM disk base address is pointed to by
            the fx_media_driver_info pointer, which is supplied by 
            the application in the call to fx_media_open.  
            */

            opReRamAddr = lReRamDiskInfo->diskBaseAddr +
                          ((media_ptr->fx_media_driver_logical_sector +
                            media_ptr->fx_media_hidden_sectors) *
                           media_ptr->fx_media_bytes_per_sector);
            opSize = media_ptr->fx_media_driver_sectors *
                     media_ptr->fx_media_bytes_per_sector;
            HAL_RERAM_Read(opReRamAddr, media_ptr->fx_media_driver_buffer, opSize);
            /* Successful driver request.  */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        case FX_DRIVER_WRITE:
        {

            /* Calculate the ReRAM disk sector offset. 
            Note the ReRAM disk base address is pointed to by the fx_media_driver_info pointer, 
            which is supplied by the application in the call to fx_media_open.  
            */

            opReRamAddr = lReRamDiskInfo->diskBaseAddr +
                          ((media_ptr->fx_media_driver_logical_sector +
                            media_ptr->fx_media_hidden_sectors) *
                           media_ptr->fx_media_bytes_per_sector);
            opSize = media_ptr->fx_media_driver_sectors *
                     media_ptr->fx_media_bytes_per_sector;
            HAL_RERAM_Write(opReRamAddr, media_ptr->fx_media_driver_buffer, opSize);

            /* Successful driver request.  */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        case FX_DRIVER_FLUSH:
        {

            /* Return driver success.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_ABORT:
        {

            /* Return driver success.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_INIT:
        {

            /* Perform basic initialization here.
            since the boot record is going to be read subsequently 
            and again for volume name requests.  
            */

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_UNINIT:
        {

            /* There is nothing to do in this case for the ReRam driver.
            Since the code is stored in ReRam, 
            so no uninitialization actions are needed here to ensure 
            that other parts of ReRam can function normally.
            */

            /* Successful driver request. */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_BOOT_READ:
        {

            /* Read the boot record and return to the caller.  */

            /* Calculate the ReRam disk boot sector offset, 
            which is at the very beginning of the ReRam disk. 
            the ReRam disk memory is pointed to by the fx_media_driver_info pointer, 
            which is supplied by the application in the call to fx_media_open.  
            */

            /* Ensure that the size of the array exceeds FX_BYTES_SECTOR=0x0B */
            UCHAR tmpBuff[16]; 
            opReRamAddr = lReRamDiskInfo->diskBaseAddr;
            HAL_RERAM_Read(opReRamAddr, tmpBuff, sizeof(tmpBuff));

            /* For ReRam driver, determine if the boot record is valid.  */
            if ((tmpBuff[0] != (UCHAR)0xEB)  ||
                ((tmpBuff[1] != (UCHAR)0x34)  &&
                (tmpBuff[1] != (UCHAR)0x76)) ||
                (tmpBuff[2] != (UCHAR)0x90))
            {

                /* Invalid boot record, return an error!  */
                media_ptr -> fx_media_driver_status =  FX_MEDIA_INVALID;
                return;
            }

            /* For ReRam disk only, pickup the bytes per sector.  */
            bytes_per_sector =  _fx_utility_16_unsigned_read(&tmpBuff[FX_BYTES_SECTOR]);


            /* Ensure this is less than the media memory size.  */
            if (bytes_per_sector > media_ptr -> fx_media_memory_size)
            {
                media_ptr -> fx_media_driver_status =  FX_BUFFER_ERROR;
                break;
            }

            /* Copy the ReRAM boot sector into the destination.  */
            HAL_RERAM_Read(opReRamAddr, media_ptr->fx_media_driver_buffer, bytes_per_sector);

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_BOOT_WRITE:
        {

            /* Write the boot record and return to the caller.  */

            /* Calculate the ReRam disk boot sector offset, 
            which is at the very beginning of the ReRam disk. 
            Note the ReRam disk memory is pointed to by the fx_media_driver_info pointer, 
            which is supplied by the application in the call to fx_media_open.  
            */
            opReRamAddr = lReRamDiskInfo->diskBaseAddr;

            /* Copy the ReRAM boot sector into the destination.  */
            HAL_RERAM_Write(opReRamAddr, media_ptr->fx_media_driver_buffer, media_ptr->fx_media_bytes_per_sector);

            /* Successful driver request.  */
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
uint32_t reRamFilexDiskFormat(t_ReramDiskInfo *reramDiskInfo)
{
    UINT  status;
    
    //fx_system_initialize();
    if (reramDiskInfo->bInitialed)
    {
        reramDiskInfo->bInitialed = 0;
        status = fx_media_close(reramDiskInfo->mediaPtr);
        if (status != FX_SUCCESS)
        {
            LOGE("ReRamFilex fx_media_close error %d", status);
            return status;
        }
    }
    /* Format the ReRAM disk.
        Because the file system's cache needs to be continuously used 
        and does not require frequent allocation and deallocation, 
        so it is recommended to use static allocation for the cache here.  
    */
    status = fx_media_format(reramDiskInfo->mediaPtr,
                    reRamFilexDiskDriver,       // Driver entry
                    reramDiskInfo,        // RAM disk memory pointer
                    reramDiskInfo->cache,           // Media buffer pointer
                    reramDiskInfo->cacheSize,   // Media buffer size
                    reramDiskInfo->volName,           // Volume Name
                    reramDiskInfo->numOfFats,       // Number of FATs
                    reramDiskInfo->dirEntries,   // Directory Entries
                    reramDiskInfo->hiddenSects,      // Hidden sectors
                    reramDiskInfo->totalSects,       // Total sectors
                    reramDiskInfo->bytesPerSect,         // Sector size
                    reramDiskInfo->sectPerCluster,    // Sectors per cluster
                    1,                      // Heads
                    1);                     // Sectors per track
    if (status != FX_SUCCESS)
    {
        LOGE("fx_media_format return error %d", status);
        return status;
    }

    status =  fx_media_open(reramDiskInfo->mediaPtr,
                            reramDiskInfo->volName,
                            reRamFilexDiskDriver,
                            reramDiskInfo,
                            reramDiskInfo->cache,
                            reramDiskInfo->cacheSize);
    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {
        LOGE("fx_media_open return error %d", status);
        return status;
    }

    reramDiskInfo->bInitialed = 1;

    return FX_SUCCESS;
}

void reRamFilexTest(FX_MEDIA *pReRamDisk) {
    UINT  status;
    CHAR  local_buffer[30];
    ULONG actual;
    FX_FILE my_file;

    status =  fx_file_create(pReRamDisk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

    }

    /* Open the test file.  */
    status =  fx_file_open(pReRamDisk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

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
    status =  fx_media_close(pReRamDisk);

    /* Check the media close status.  */
    if (status != FX_SUCCESS)
    {

        /* Error closing the media, break the loop.  */
        return;
    }


    return;
}