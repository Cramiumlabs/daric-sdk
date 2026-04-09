/**
******************************************************************************
* @file    reram_filex_levelx_impl.c
* @author  OS Team
* @brief   This file's contents are the adaptation of Filex&Levelx for the ReRam storage.
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
#include "reram_filex_levelx_impl.h"
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "RERAM_FX_LX"


/* Define RAM device driver entry.  */
static void reRamLevelxDiskDriver(FX_MEDIA *media_ptr);

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
uint32_t reRamFilexLevelxDiskLoad(t_ReramFilexLevelxInfo *pReramFilexLevelxInfo)
{
    //static UCHAR bInited = 0;
    UINT  status;
    t_ReramDiskInfo *reramDiskInfo = NULL;
    t_ReramLevelxInfo *reramLevelxInfo = NULL;

    reramDiskInfo   = pReramFilexLevelxInfo->reramDiskInfo;
    reramLevelxInfo = pReramFilexLevelxInfo->reramLevelxInfo;
    
    if (reramDiskInfo->bInitialed)
    {
        LOGE("ReRamLevelxDisk re-initialization error");
        return FX_INVALID_STATE;
    }

    status =  reramLevelxLoad(reramLevelxInfo);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "ReRamLevelx init error");
        //lx_nor_flash_close(reramLevelxInfo->mediaPtr);
        reramLevelxEraseAllHead(reramLevelxInfo);
        status =  reramLevelxLoad(reramLevelxInfo);
        if (status != LX_SUCCESS)
        {
            LOGE_COLOR(COL_RED, "reram levelx reopen error, errorNum:%d", status);
            return status;
        }
        /* Format the ReRAM disk.
           Levelx only initializes the management data,
           The data from previously existing file system FAT allocation tables might still remain,
           So perform a formatting operation to reset the file system.
        */
        status = fx_media_format(reramDiskInfo->mediaPtr,
                                 reRamLevelxDiskDriver,       // Driver entry
                                 pReramFilexLevelxInfo,        // RAM disk memory pointer
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
            LOGE_COLOR(COL_RED, "fx_media_format return error %d", status);
            return status;
        }
    }

    //fx_system_initialize();
    /* Open the RAM disk.  */
    status =  fx_media_open(reramDiskInfo->mediaPtr,
                            reramDiskInfo->volName, 
                            reRamLevelxDiskDriver, 
                            pReramFilexLevelxInfo, 
                            reramDiskInfo->cache, 
                            reramDiskInfo->cacheSize);

    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {
        /* [TODO]This is a temporary solution. 
           The correct approach should be to format the disk within a function like a "setup wizard," 
           and clearly inform the user that data may be lost. 
        */
        status = reramLevelxCleanReload(reramLevelxInfo);
        if (status != LX_SUCCESS)
        {
            return status;
        }
        /* Format the ReRAM disk.
           Because the file system's cache needs to be continuously used 
           and does not require frequent allocation and deallocation, 
           so it is recommended to use static allocation for the cache here.  
        */
        status = fx_media_format(reramDiskInfo->mediaPtr,
                        reRamLevelxDiskDriver,       // Driver entry
                        pReramFilexLevelxInfo,        // RAM disk memory pointer
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
            reRamLevelxDiskDriver, 
            pReramFilexLevelxInfo, 
            reramDiskInfo->cache, 
            reramDiskInfo->cacheSize);
        if (status != FX_SUCCESS)
        {
            LOGE("fx_media_open return error %d", status);
            return status;
        }

    }

    /* Filex init success */
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
static void  reRamLevelxDiskDriver(FX_MEDIA *media_ptr)
{

    UCHAR   *srcBuffer;
    UCHAR   *destBuffer;
    ULONG   logiSector;
    UINT    status;
    UINT    i;
    UINT    bytes_per_sector;
    LX_NOR_FLASH *pNorFlash = NULL;
    t_ReramLevelxInfo *pReramLevelxInfo = NULL;
    t_ReramFilexLevelxInfo *pReramFilexLevelxInfo = NULL;

    pReramFilexLevelxInfo = (t_ReramFilexLevelxInfo *)media_ptr->fx_media_driver_info;
    pReramLevelxInfo = pReramFilexLevelxInfo->reramLevelxInfo;
    pNorFlash = pReramLevelxInfo->mediaPtr;

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

            /* Setup the destination buffer and logical sector.  */
            logiSector = media_ptr -> fx_media_driver_logical_sector;
            destBuffer = (UCHAR *) media_ptr -> fx_media_driver_buffer;

            /* Loop to read sectors from reram flash.  */
            for (i = 0; i < media_ptr -> fx_media_driver_sectors; i++)
            {
            
                /* Read a sector from reram flash.  */
                status =  lx_nor_flash_sector_read(pNorFlash, logiSector, destBuffer);

                /* Determine if the read was successful.  */
                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    
                    return;
                } 

                /* Move to the next entries.  */
                logiSector++;
                destBuffer =  destBuffer + media_ptr->fx_media_bytes_per_sector;
            }

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

            /* Setup the source buffer and logical sector.  */
            logiSector  = media_ptr->fx_media_driver_logical_sector;
            srcBuffer   = (UCHAR *)media_ptr->fx_media_driver_buffer;

            /* Loop to write sectors to reram flash.  */
            for (i = 0; i < media_ptr->fx_media_driver_sectors; i++)
            {
            
                /* Write a sector to reram flash.  */
                status =  lx_nor_flash_sector_write(pNorFlash, logiSector, srcBuffer);

                /* Determine if the write was successful.  */
                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    
                    return;
                } 

                /* Move to the next entries.  */
                logiSector++;
                srcBuffer =  srcBuffer + media_ptr->fx_media_bytes_per_sector;
            }

            /* Successful driver request.  */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        case FX_DRIVER_FLUSH:
        {

            /* Return driver success.  */
            media_ptr->fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_ABORT:
        {

            /* Return driver success.  */
            media_ptr->fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_INIT:
        {

            /* 
               Perform basic initialization here.
               since the boot record is going to be read subsequently 
               and again for volume name requests.  

               The fx_media_driver_free_sector_update flag is used to instruct
               FileX to inform the driver whenever sectors are not being used.
               This is especially useful for FLASH managers so they don't have 
               maintain mapping for sectors no longer in use.
            */

            /* With flash wear leveling, FileX should tell wear leveling when sectors
               are no longer in use.  
             */
            media_ptr->fx_media_driver_free_sector_update =  FX_TRUE;

            /* Open the NOR flash simulation.  */
            //status =  lx_nor_flash_open(pNorFlash, "reram levelx flash", NULLNULL);
            //status = reramLevelxLoad(pReramFilexLevelxInfo)

   
            /* Successful driver request.  */
            media_ptr->fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_UNINIT:
        {

            /* There is nothing to do in this case for the ReRam driver.
            Since the code is stored in ReRam, 
            so no uninitialization actions are needed here to ensure 
            that other parts of ReRam can function normally.
            */

            /* Close the reram flash.  */
            //status =  lx_nor_flash_close(pNorFlash);

            /* Determine if the flash close was successful.  */
            // if (status != LX_SUCCESS)
            // {
                
            //     /* Return an I/O error to FileX.  */
            //     media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    
            //     return;
            // } 

            /* Successful driver request. */
            media_ptr->fx_media_driver_status =  FX_SUCCESS;
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
            /* Setup the destination buffer.  */
            destBuffer =  (UCHAR *) media_ptr->fx_media_driver_buffer;

            /* Read boot sector from NOR flash.  */
            status =  lx_nor_flash_sector_read(pNorFlash, 0, destBuffer);

            /* For ReRam driver, determine if the boot record is valid.  */
            if ((destBuffer[0] != (UCHAR)0xEB)  ||
                ((destBuffer[1] != (UCHAR)0x34)  &&
                (destBuffer[1] != (UCHAR)0x76)) ||
                (destBuffer[2] != (UCHAR)0x90))
            {

                /* Invalid boot record, return an error!  */
                media_ptr->fx_media_driver_status =  FX_MEDIA_INVALID;
                return;
            }

            /* For ReRam disk only, pickup the bytes per sector.  */
            bytes_per_sector =  _fx_utility_16_unsigned_read(&destBuffer[FX_BYTES_SECTOR]);


            /* Ensure this is less than the media memory size.  */
            if (bytes_per_sector > media_ptr->fx_media_memory_size)
            {
                media_ptr->fx_media_driver_status =  FX_BUFFER_ERROR;
                break;
            }

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_BOOT_WRITE:
        {

            /* Calculate the ReRam disk boot sector offset, 
                which is at the very beginning of the ReRam disk. 
                Note the ReRam disk memory is pointed to by the fx_media_driver_info pointer, 
                which is supplied by the application in the call to fx_media_open.  
            */
            /* Make sure the media bytes per sector equals to the LevelX logical sector size.  */
            if (media_ptr->fx_media_bytes_per_sector != (LX_NOR_SECTOR_SIZE) * sizeof(ULONG))
            {

                /* Sector size mismatch, return error.  */
                media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                break;
            }

            /* Write the boot record and return to the caller.  */

            /* Setup the source buffer.  */
            srcBuffer = (UCHAR *) media_ptr->fx_media_driver_buffer;

            /* Write boot sector to reram flash.  */
            status =  lx_nor_flash_sector_write(pNorFlash, 0, srcBuffer);

            /* Determine if the boot write was successful.  */
            if (status != LX_SUCCESS)
            {
                
                /* Return an I/O error to FileX.  */
                media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                    
                return;
            } 

            /* Successful driver request.  */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        case FX_DRIVER_RELEASE_SECTORS:
        {
        
            /* Setup the logical sector.  */
            logiSector =  media_ptr->fx_media_driver_logical_sector;

            /* Release sectors.  */
            for (i = 0; i < media_ptr->fx_media_driver_sectors; i++)
            {
            
                /* Release reram flash sector. */
                status =  lx_nor_flash_sector_release(pNorFlash, logiSector);

                /* Determine if the sector release was successful.  */
                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    
                    return;
                } 

                /* Move to the next entries.  */
                logiSector++;
            }

            /* Successful driver request.  */
            media_ptr -> fx_media_driver_status =  FX_SUCCESS;
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
uint32_t reRamFilexLevelxDiskFormat(t_ReramFilexLevelxInfo *pReramFilexLevelxInfo)
{
    UINT  status;
    t_ReramDiskInfo *reramDiskInfo = NULL;
    t_ReramLevelxInfo *reramLevelxInfo = NULL;

    reramDiskInfo   = pReramFilexLevelxInfo->reramDiskInfo;
    reramLevelxInfo = pReramFilexLevelxInfo->reramLevelxInfo;

    if (reramLevelxInfo->bInitialed) {
        reramLevelxInfo->bInitialed = 0;
        status = lx_nor_flash_close(reramLevelxInfo->mediaPtr);
        if (status != LX_SUCCESS)
        {
            LOGE("ReRamLevelxDisk close error");
            return status;
        }
    }
    reramLevelxEraseAllHead(reramLevelxInfo);
    status =  reramLevelxLoad(reramLevelxInfo);
    if (status != LX_SUCCESS)
    {
        LOGE("ReRamLevelx initialization error");
        return status;
    }

    if (reramDiskInfo->bInitialed)
    {
        reramDiskInfo->bInitialed = 0;
        status = fx_media_close(reramDiskInfo->mediaPtr);
        if (status != FX_SUCCESS)
        {
            LOGE("ReRamFilex close error");
            return status;
        }
    }
    /* Format the ReRAM disk.
        Because the file system's cache needs to be continuously used 
        and does not require frequent allocation and deallocation, 
        so it is recommended to use static allocation for the cache here.  
    */
    status = fx_media_format(reramDiskInfo->mediaPtr,
                    reRamLevelxDiskDriver,       // Driver entry
                    pReramFilexLevelxInfo,        // RAM disk memory pointer
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
                            reRamLevelxDiskDriver,
                            pReramFilexLevelxInfo,
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

/**
 * @brief  This function initializes the filex.
 * @param  none.
 * @retval none.
 * 
 * This function initializes the various control data structures 
 * for the FileX System component
 */
void reRamFilexLevelxSystemInit(void)
{
    
    fx_system_initialize();

}

void reRamFilexLevelxTest(FX_MEDIA *pReRamDisk) {
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