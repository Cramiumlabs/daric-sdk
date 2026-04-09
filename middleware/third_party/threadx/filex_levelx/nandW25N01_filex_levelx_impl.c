/**
******************************************************************************
* @file    nandW25N01IT_filex_levelx_impl.c
* @author  OS Team
* @brief   This file's contents are the adaptation of Filex&Levelx for the nandflash storage.
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
#include "nandW25N01_filex_levelx_impl.h"
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "NAND_FXLX_IMPL"


static void nandFlashLevelxDiskDriver(FX_MEDIA *media_ptr);

/**
 * @brief Initializes and loads a FileX file system on top of LevelX NAND Flash
 * @param pNandflashFilexLevelxInfo Pointer to the composite structure containing
 *                                  both LevelX NAND Flash configuration and FileX
 *                                  file system parameters needed for initialization
 * @return FX_SUCCESS FileX file system successfully initialized and ready for use
 *         FX_INVALID_STATE Attempt to reinitialize an already initialized disk
 *         Other error codes from LevelX (LX_*) or FileX (FX_*) operations
 *
 * @note The function implements a sophisticated recovery flow:
 *       1. First attempts to load existing LevelX file system
 *       2. If load fails, formats LevelX and retries loading
 *       3. If LevelX load succeeds but FileX media open fails, performs
 *          complete reload with formatting
 * @warning This function may format the file system and erase all data without
 *          user confirmation when corruption is detected
 *
 * This function performs the complete initialization sequence for a FileX FAT
 * file system layered on top of a LevelX wear-leveling NAND Flash driver. It
 * handles the complex bootstrapping process including LevelX initialization,
 * file system formatting (if necessary), and media opening with appropriate
 * recovery mechanisms for corrupted or uninitialized file systems.
 */
uint32_t nandFlashFilexLevelxDiskLoad(t_NandflashFilexLevelxInfo *pNandflashFilexLevelxInfo)
{
    //static UCHAR bInited = 0;
    UINT  status;
    t_NandflashDiskInfo *nandFlashDiskInfo = NULL;
    t_NandflashLevelxInfo *nandFlashLevelxInfo = NULL;

    nandFlashDiskInfo   = pNandflashFilexLevelxInfo->nandFlashDiskInfo;
    nandFlashLevelxInfo = pNandflashFilexLevelxInfo->nandFlashLevelxInfo;
    
    if (nandFlashDiskInfo->bInitialed)
    {
        LOGE("NandflashLevelxDisk re-initialization error");
        return FX_INVALID_STATE;
    }

    status =  nandFlashLevelxLoad(nandFlashLevelxInfo);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "NandflashLevelx init error");
        status = nandFlashLevelxFormat(nandFlashLevelxInfo);
        if (status != LX_SUCCESS)
        {
            LOGE_COLOR(COL_RED, "nandflash levelx format error, errorNum:%d", status);
            return status;
        }
        status = nandFlashLevelxLoad(nandFlashLevelxInfo);
        if (status != LX_SUCCESS)
        {
            LOGE_COLOR(COL_RED, "nandflash levelx reopen error, errorNum:%d", status);
            return status;
        }
        /* Format the nandflash disk.
           Levelx only initializes the management data,
           The data from previously existing file system FAT allocation tables might still remain,
           So perform a formatting operation to reset the file system.
        */
        status = fx_media_format(nandFlashDiskInfo->mediaPtr,
                                 nandFlashLevelxDiskDriver,       // Driver entry
                                 pNandflashFilexLevelxInfo,        // RAM disk memory pointer
                                 nandFlashDiskInfo->cache,           // Media buffer pointer
                                 nandFlashDiskInfo->cacheSize,   // Media buffer size
                                 nandFlashDiskInfo->volName,           // Volume Name
                                 nandFlashDiskInfo->numOfFats,       // Number of FATs
                                 nandFlashDiskInfo->dirEntries,   // Directory Entries
                                 nandFlashDiskInfo->hiddenSects,      // Hidden sectors
                                 nandFlashDiskInfo->totalSects,       // Total sectors
                                 nandFlashDiskInfo->bytesPerSect,         // Sector size
                                 nandFlashDiskInfo->sectPerCluster,    // Sectors per cluster
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
    status =  fx_media_open(nandFlashDiskInfo->mediaPtr,
                            nandFlashDiskInfo->volName,
                            nandFlashLevelxDiskDriver,
                            pNandflashFilexLevelxInfo,
                            nandFlashDiskInfo->cache,
                            nandFlashDiskInfo->cacheSize);

    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {
        /* [TODO]This is a temporary solution. 
           The correct approach should be to format the disk within a function like a "setup wizard," 
           and clearly inform the user that data may be lost. 
        */
        status = nandFlashLevelxFormatReload(nandFlashLevelxInfo);
        if (status != LX_SUCCESS)
        {
            return status;
        }
        /* Format the nandflash disk.
           Because the file system's cache needs to be continuously used 
           and does not require frequent allocation and deallocation, 
           so it is recommended to use static allocation for the cache here.  
        */
        status = fx_media_format(nandFlashDiskInfo->mediaPtr,
                        nandFlashLevelxDiskDriver,       // Driver entry
                        pNandflashFilexLevelxInfo,        // RAM disk memory pointer
                        nandFlashDiskInfo->cache,           // Media buffer pointer
                        nandFlashDiskInfo->cacheSize,   // Media buffer size
                        nandFlashDiskInfo->volName,           // Volume Name
                        nandFlashDiskInfo->numOfFats,       // Number of FATs
                        nandFlashDiskInfo->dirEntries,   // Directory Entries
                        nandFlashDiskInfo->hiddenSects,      // Hidden sectors
                        nandFlashDiskInfo->totalSects,       // Total sectors
                        nandFlashDiskInfo->bytesPerSect,         // Sector size
                        nandFlashDiskInfo->sectPerCluster,    // Sectors per cluster
                        1,                      // Heads
                        1);                     // Sectors per track
        if (status != FX_SUCCESS)
        {
            LOGE("fx_media_format return error %d", status);
            return status;
        }

        status =  fx_media_open(nandFlashDiskInfo->mediaPtr,
            nandFlashDiskInfo->volName,
            nandFlashLevelxDiskDriver,
            pNandflashFilexLevelxInfo,
            nandFlashDiskInfo->cache,
            nandFlashDiskInfo->cacheSize);
        if (status != FX_SUCCESS)
        {
            LOGE("fx_media_open return error %d", status);
            return status;
        }

    }

    /* Filex init success */
    nandFlashDiskInfo->bInitialed = 1;

    return FX_SUCCESS;
}



/**
 * @brief  This function is the entry point to the nandflash disk driver.
 * @param  media_ptr FX_MEDIA structure defines everything required to manage
 *         a media device. This structure contains all media information,
 *         including the media-specific I/O driver and associated parameters
 *         for passing information and status between the driver and FileX.
 * @return The I/O driver communicates the success or failure of the request
 *         through the fx_media_driver_status member of FX_MEDIA. 
 *         If the driver request was successful, 
 *         FX_SUCCESS is placed in this field before the driver returns. 
 *         Otherwise, if an error is detected, FX_IO_ERROR is placed in this field.
 * 
 * This function serves as the FileX low-level driver interface that bridges
 * FileX file system operations with the underlying LevelX NAND Flash wear-leveling
 * layer. It handles all FileX driver requests including read, write, initialization,
 * boot sector operations, and sector release for garbage collection. The driver
 * translates FileX logical sector operations into LevelX sector operations while
 * managing the complex interactions between the FAT file system and wear-leveling
 * flash translation layer.
 */
static void  nandFlashLevelxDiskDriver(FX_MEDIA *media_ptr)
{

    UCHAR   *srcBuffer;
    UCHAR   *destBuffer;
    ULONG   logiSector;
    UINT    status;
    UINT    i;
    UINT    bytes_per_sector;
    LX_NAND_FLASH *pNandFlash = NULL;
    t_NandflashLevelxInfo *pNandflashLevelxInfo = NULL;
    t_NandflashDiskInfo *pNandFlashDiskInfo = NULL;
    t_NandflashFilexLevelxInfo *pNandflashFilexLevelxInfo = NULL;

    pNandflashFilexLevelxInfo = (t_NandflashFilexLevelxInfo *)media_ptr->fx_media_driver_info;
    pNandflashLevelxInfo = pNandflashFilexLevelxInfo->nandFlashLevelxInfo;
    pNandFlashDiskInfo = pNandflashFilexLevelxInfo->nandFlashDiskInfo;
    pNandFlash = pNandflashLevelxInfo->mediaPtr;

    /* Process the driver request specified in the media control block.  */
    switch (media_ptr -> fx_media_driver_request)
    {

        case FX_DRIVER_READ:
        {

            /* Calculate the nandflash disk sector offset.
            Note the nandflash disk info is pointed to by
            the fx_media_driver_info pointer, which is supplied by 
            the application in the call to fx_media_open.  
            */

            /* Setup the destination buffer and logical sector.  */
            logiSector = media_ptr -> fx_media_driver_logical_sector;
            destBuffer = (UCHAR *) media_ptr -> fx_media_driver_buffer;

            /* Loop to read sectors from nand flash.  */
            for (i = 0; i < media_ptr -> fx_media_driver_sectors; i++)
            {
            
                status =  lx_nand_flash_sector_read(pNandFlash, logiSector, destBuffer);

                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr -> fx_media_driver_status =  FX_IO_ERROR;
                    return;
                } 

                logiSector++;
                destBuffer =  destBuffer + media_ptr->fx_media_bytes_per_sector;
            }

            /* Successful driver request.  */
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        case FX_DRIVER_WRITE:
        {

            /* Calculate the nandflash disk sector offset.
            Note the nandflash disk info is pointed to by the fx_media_driver_info pointer,
            which is supplied by the application in the call to fx_media_open.  
            */

            /* Setup the source buffer and logical sector.  */
            logiSector  = media_ptr->fx_media_driver_logical_sector;
            srcBuffer   = (UCHAR *)media_ptr->fx_media_driver_buffer;

            /* Loop to write sectors to nand flash.  */
            for (i = 0; i < media_ptr->fx_media_driver_sectors; i++)
            {
            
                status =  lx_nand_flash_sector_write(pNandFlash, logiSector, srcBuffer);

                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    return;
                } 

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
            media_ptr->fx_media_driver_write_protect = pNandFlashDiskInfo->bWrtieProtected;

            //status =  lx_nand_flash_open(pNandFlash, "nandflash levelx flash", NULLNULL);
            //status = nandFlashLevelxLoad(pNandflashFilexLevelxInfo)

   
            /* Successful driver request.  */
            media_ptr->fx_media_driver_status =  FX_SUCCESS;
            break;
        }

        case FX_DRIVER_UNINIT:
        {

            /* There is nothing to do in this case for the nandflash driver.
            Since the code is stored in nandflash,
            so no uninitialization actions are needed here to ensure 
            that other parts of nandflash can function normally.
            */

            /* Close the nand flash.  */
            //status =  lx_nand_flash_close(pNandFlash);

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

            /* Calculate the nandflash disk boot sector offset,
            which is at the very beginning of the nandflash disk.
            the nandflash disk memory is pointed to by the fx_media_driver_info pointer,
            which is supplied by the application in the call to fx_media_open.  
            */

            /* Ensure that the size of the array exceeds FX_BYTES_SECTOR=0x0B */
            /* Setup the destination buffer.  */
            destBuffer =  (UCHAR *) media_ptr->fx_media_driver_buffer;

            /* Read boot sector from nand flash.  */
            status =  lx_nand_flash_sector_read(pNandFlash, 0, destBuffer);

            /* For nandflash driver, determine if the boot record is valid.  */
            if ((destBuffer[0] != (UCHAR)0xEB)  ||
                ((destBuffer[1] != (UCHAR)0x34)  &&
                (destBuffer[1] != (UCHAR)0x76)) ||
                (destBuffer[2] != (UCHAR)0x90))
            {

                /* Invalid boot record, return an error!  */
                media_ptr->fx_media_driver_status =  FX_MEDIA_INVALID;
                return;
            }

            /* For nandflash disk only, pickup the bytes per sector.  */
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

            /* Calculate the nandflash disk boot sector offset,
                which is at the very beginning of the nandflash disk.
                Note the nandflash disk memory is pointed to by the fx_media_driver_info pointer,
                which is supplied by the application in the call to fx_media_open.  
            */
            /* Make sure the media bytes per sector equals to the LevelX logical sector size.  */
            if (media_ptr->fx_media_bytes_per_sector != W25N01IT_BYTES_PER_PAGE)
            {

                /* Sector size mismatch, return error.  */
                media_ptr->fx_media_driver_status =  FX_IO_ERROR;
                break;
            }

            srcBuffer = (UCHAR *) media_ptr->fx_media_driver_buffer;

            status =  lx_nand_flash_sector_write(pNandFlash, 0, srcBuffer);

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

            for (i = 0; i < media_ptr->fx_media_driver_sectors; i++)
            {
            
                status =  lx_nand_flash_sector_release(pNandFlash, logiSector);

                if (status != LX_SUCCESS)
                {
                
                    /* Return an I/O error to FileX.  */
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    
                    return;
                } 

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
 * @brief  Formats the FileX file system on LevelX-managed NAND Flash.
 * @param  pNandflashFilexLevelxInfo Pointer to a t_NandflashFilexLevelxInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @return FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 *
 * This function performs a complete format operation of the FileX FAT file system
 * layered on top of LevelX wear-leveling NAND Flash. It properly closes any
 * open file system instances, reformats the underlying LevelX flash layer,
 * and then creates a fresh FileX file system with the specified parameters.
 * This is a destructive operation that erases all existing data.
 */
uint32_t nandFlashFilexLevelxDiskFormat(t_NandflashFilexLevelxInfo *pNandflashFilexLevelxInfo)
{
    UINT  status;
    t_NandflashDiskInfo *nandFlashDiskInfo = NULL;
    t_NandflashLevelxInfo *nandFlashLevelxInfo = NULL;

    nandFlashDiskInfo   = pNandflashFilexLevelxInfo->nandFlashDiskInfo;
    nandFlashLevelxInfo = pNandflashFilexLevelxInfo->nandFlashLevelxInfo;

    if (nandFlashDiskInfo->bInitialed)
    {
        nandFlashDiskInfo->bInitialed = 0;
        status = fx_media_close(nandFlashDiskInfo->mediaPtr);
        if (status != FX_SUCCESS)
        {
            LOGE("NandflashFilex close error");
            return status;
        }
    }

    if (nandFlashLevelxInfo->bInitialed) {
        nandFlashLevelxInfo->bInitialed = 0;
        status = lx_nand_flash_close(nandFlashLevelxInfo->mediaPtr);
        if (status != LX_SUCCESS)
        {
            LOGE("NandflashLevelxDisk close error");
            return status;
        }
    }

    status = nandFlashLevelxFormat(nandFlashLevelxInfo);
    if (status != LX_SUCCESS)
    {
        LOGE("NandflashLevelx levelx format error");
        return status;
    }

    status = nandFlashLevelxLoad(nandFlashLevelxInfo);
    if (status != LX_SUCCESS)
    {
        LOGE("NandflashLevelx initialization error");
        return status;
    }

    /* Format the nandflash disk.
        Because the file system's cache needs to be continuously used 
        and does not require frequent allocation and deallocation, 
        so it is recommended to use static allocation for the cache here.  
    */
    status = fx_media_format(nandFlashDiskInfo->mediaPtr,
                    nandFlashLevelxDiskDriver,       // Driver entry
                    pNandflashFilexLevelxInfo,        // RAM disk memory pointer
                    nandFlashDiskInfo->cache,           // Media buffer pointer
                    nandFlashDiskInfo->cacheSize,   // Media buffer size
                    nandFlashDiskInfo->volName,           // Volume Name
                    nandFlashDiskInfo->numOfFats,       // Number of FATs
                    nandFlashDiskInfo->dirEntries,   // Directory Entries
                    nandFlashDiskInfo->hiddenSects,      // Hidden sectors
                    nandFlashDiskInfo->totalSects,       // Total sectors
                    nandFlashDiskInfo->bytesPerSect,         // Sector size
                    nandFlashDiskInfo->sectPerCluster,    // Sectors per cluster
                    1,                      // Heads
                    1);                     // Sectors per track
    if (status != FX_SUCCESS)
    {
        LOGE("fx_media_format return error %d", status);
        return status;
    }

    status =  fx_media_open(nandFlashDiskInfo->mediaPtr,
                            nandFlashDiskInfo->volName,
                            nandFlashLevelxDiskDriver,
                            pNandflashFilexLevelxInfo,
                            nandFlashDiskInfo->cache,
                            nandFlashDiskInfo->cacheSize);

    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {
        LOGE("fx_media_open return error %d", status);
        return status;
    }

    nandFlashDiskInfo->bInitialed = 1;

    return FX_SUCCESS;
}

/**
 * @brief  This function initializes the filex.
 * @param  none.
 * @return none.
 * 
 * This function initializes the various control data structures 
 * for the FileX System component
 */
void nandFlashFilexLevelxSystemInit(void)
{
    
    fx_system_initialize();

}

void nandFlashFilexLevelxTest(FX_MEDIA *pNandFlashDisk) {
    UINT  status;
    CHAR  local_buffer[30];
    ULONG actual;
    FX_FILE my_file;

    status =  fx_file_create(pNandFlashDisk, "TEST.TXT");

    /* Check the create status.  */
    if (status != FX_SUCCESS)
    {

    }

    /* Open the test file.  */
    status =  fx_file_open(pNandFlashDisk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

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
    status =  fx_media_close(pNandFlashDisk);

    /* Check the media close status.  */
    if (status != FX_SUCCESS)
    {

        /* Error closing the media, break the loop.  */
        return;
    }


    return;
}