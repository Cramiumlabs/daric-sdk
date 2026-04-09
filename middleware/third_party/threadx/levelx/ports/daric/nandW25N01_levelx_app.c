/**
******************************************************************************
* @file    nandW25N01IT_levelx_app.c
* @author  OS Team
* @brief   LevelX NAND flash driver for W25N01GVxxIT
*          This file provides the driver layer between LevelX and the
*          W25N01GVxxIT NAND flash device using the daric_hal_flash interface.
*******************************************************************************
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
#include "daric_hal_flash.h"
#include "nandW25N01_levelx_app.h"
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "NAND_LX_APP"
/* Private defines -----------------------------------------------------------*/

/* LevelX spare area configuration */
#define LX_NAND_SPARE_DATA1_OFFSET      4
#define LX_NAND_SPARE_DATA1_LENGTH      4
#define LX_NAND_SPARE_DATA2_OFFSET      2
#define LX_NAND_SPARE_DATA2_LENGTH      2
#define LX_NAND_SPARE_TOTAL_LENGTH      64

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static UINT nandLevelxInitialize(LX_NAND_FLASH *pNandFlash);

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
ULONG  nandFlashLevelxLoad(t_NandflashLevelxInfo *pNandLevelxInfo)
{
    UINT status;

    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    if (pNandLevelxInfo->bInitialed) {
        LOGE_COLOR(COL_RED, "nandLevelxLoad re-loadload error");
        return LX_ERROR;
    }

    /*
     * When calling lx_nand_flash_open,
     * only the content before lx_nand_flash_open_previous within the LX_NAND_FLASH structure is initialized to 0.
     * The content defined by LX_NAND_FLASH_USER_EXTENSION is not cleared to 0,
     * so it can be retained after the call to lx_nand_flash_open.
     */
    pNandLevelxInfo->mediaPtr->lx_media_driver_info = (void *)pNandLevelxInfo;
    // status = nandflashInitSPI();
    // if (HAL_OK != status)
    // {
    //     LOGE("NandflashDisk SPIM Init errorN=%d", status);
    //     return LX_ERROR;
    // }

    // status = HAL_FLASH_Init();
    // if (HAL_OK != status)
    // {
    //     LOGE("HAL_FLASH_Init Init errorN=%d", status);
    //     return LX_ERROR;
    // }

    status = lx_nand_flash_open(pNandLevelxInfo->mediaPtr,
                                pNandLevelxInfo->diskName,
                                nandLevelxInitialize,
                                (ULONG *)pNandLevelxInfo->pMemBuffer,
                                pNandLevelxInfo->memBufferSize);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "nand lx_nand_flash_open error, errorNum:%d", status);
        return LX_ERROR;
    }

    /* Init success */
    pNandLevelxInfo->bInitialed = 1;

    /* Return success.  */
    return(LX_SUCCESS);
}

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
ULONG  nandFlashInitHalSpi(void)
{
    UINT status;

    status = nandflashInitSPI();
    if (HAL_OK != status)
    {
        LOGE("NandflashDisk SPIM Init errorN=%d", status);
        return LX_ERROR;
    }

    status = HAL_FLASH_Init();
    if (HAL_OK != status)
    {
        LOGE("HAL_FLASH_Init Init errorN=%d", status);
        return LX_ERROR;
    }

    /* Return success.  */
    return(LX_SUCCESS);
}

/**
 * @brief Validates NAND Flash operation parameters for safety and correctness
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure that contains flash 
 *                   geometry information and driver-specific data. This structure
 *                   must be properly initialized before calling this function.
 * @param phyBlkIdx Physical block index (0-based) to be accessed. Represents the
 *                  actual physical block number on the NAND Flash device.
 * @param page Page number (0-based) within the specified physical block. Each block
 *             contains multiple pages that must be written sequentially.
 * @param bytes Number of bytes to read or write, including both main data area
 *              and spare/OOB area. For write operations, this typically includes
 *              ECC data in the spare area.
 * 
 * @return LX_SUCCESS All parameters are within valid ranges and the operation can proceed
 *         LX_ERROR One or more parameters are invalid. Specific error is logged.
 * 
 * This function performs comprehensive parameter validation to ensure all inputs
 * for NAND Flash operations are within acceptable ranges. It checks physical block
 * index, page number within block, and data byte count against the NAND Flash's
 * physical geometry constraints. Proper parameter validation prevents out-of-bounds
 * accesses, data corruption, and potential flash controller errors.
 */
static UINT nandFlashDriverParamCheck(LX_NAND_FLASH *pNandFlash, ULONG phyBlkIdx, ULONG page, ULONG bytes)
{
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;

    if (pNandFlash == NULL)
    {
        LOGE("%s pNandFlash NULL error", __func__);
        return LX_ERROR;
    }
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;

    if (phyBlkIdx >= W25N01IT_TOTAL_BLOCKS)
    {
        LOGE("%s phyBlkIdx error=%d", __func__, phyBlkIdx);
        return LX_ERROR;
    }
    if (page >= pNandLevelxInfo->pagesPerBlock)
    {
        LOGE("%s page error=%d", __func__, page);
        return LX_ERROR;
    }
    if (bytes > (pNandLevelxInfo->bytesPerPage + pNandLevelxInfo->spareLength))
    {
        LOGE("%s bytes error=%d", __func__, bytes);
        return LX_ERROR;
    }
    /*
    if (destination == NULL)
    {
        LOGE("nandFlashDriverParamCheck destination is NULL");
        return LX_ERROR;
    }
    */
    return LX_SUCCESS;
}

/**
 * @brief Reads data from specified NAND Flash block and page
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash 
 *                   configuration and driver information
 * @param block Logical block index (0-based) relative to the start block for 
 *              LevelX file system. This is translated to physical block index.
 * @param page Page number (0-based) within the specified block to read from
 * @param destination Pointer to the buffer where read data will be stored.
 *                    Must have sufficient space for the requested words.
 * @param words Number of 32-bit words to read. Will be converted to bytes 
 *              (words * 4) for the actual read operation.
 * 
 * @return LX_SUCCESS Read operation completed successfully
 * @return LX_ERROR Read operation failed due to invalid parameters, 
 *                  boundary violation, or hardware error
 * 
 * This function reads data from a specified physical block and page of the NAND Flash
 * memory. It handles the translation from logical block address to physical block
 * address using the nandStartBlock offset, validates parameters, and performs the
 * actual read operation through the HAL layer. The function reads the specified
 * number of words (converted to bytes) from the flash memory into the destination
 * buffer.
 */
static UINT nandFlashDriverRead(LX_NAND_FLASH *pNandFlash, ULONG block, ULONG page, ULONG *destination, ULONG words)
{
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;
    HAL_StatusTypeDef status;
    UINT phyBlkIndex;
    UINT bytesRead;

    if (pNandFlash == NULL || destination == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash or destination NULL error", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to read */
    bytesRead = words * sizeof(ULONG);
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, page, bytesRead) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    /* Read data from flash */
    status = HAL_FLASH_Read_QSPI((uint8_t *)destination,
                                 (uint16_t)phyBlkIndex,
                                 (uint8_t)page,
                                 0,
                                 (uint16_t)bytesRead
                                );
    if (HAL_OK != status)
    {
        LOGE("%s HAL_FLASH_Read_QSPI errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    return LX_SUCCESS;
}

/**
 * @brief Writes data to specified NAND Flash block and page
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash 
 *                   configuration, geometry information, and driver-specific data
 * @param block Logical block index (0-based) relative to the LevelX file system 
 *              start block. This is converted to physical block index.
 * @param page Page number (0-based) within the specified block where data 
 *             will be written
 * @param source Pointer to the source buffer containing data to be written 
 *               to the flash memory
 * @param words Number of 32-bit words to write from the source buffer. 
 *              This value is converted to bytes (words * 4) for the actual write.
 * 
 * @return LX_SUCCESS Write operation completed successfully
 *         LX_ERROR Write operation failed due to invalid parameters, boundary 
 *                  violation, or hardware write error
 * 
 * This function writes data from a source buffer to a specified physical block 
 * and page of the NAND Flash memory. It translates logical block addresses to 
 * physical addresses using the nandStartBlock offset, validates parameters to 
 * ensure they are within valid ranges, and performs the actual write operation 
 * through the HAL layer. The function converts the requested word count to bytes 
 * for the write operation.
 */
static UINT nandFlashDriverWrite(LX_NAND_FLASH *pNandFlash, ULONG block, ULONG page, ULONG *source, ULONG words)
{
    HAL_StatusTypeDef status;
    UINT phyBlkIndex;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;
    UINT bytesWrite;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to write */
    bytesWrite = words * sizeof(ULONG);
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, page, bytesWrite) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    /* Write data to flash */
    status = HAL_FLASH_Write_QSPI((uint8_t *)source,
                                  (uint16_t)phyBlkIndex,
                                  (uint8_t)page,
                                  0,
                                  (uint16_t)bytesWrite
                                 );
    if (HAL_OK != status)
    {
        LOGE("%s HAL_FLASH_Write_QSPI errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    return LX_SUCCESS;
}

/**
 * @brief Erases a specified block of NAND Flash memory
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash 
 *                   configuration, geometry information, and driver context
 * @param block Logical block index (0-based) to be erased. This index is 
 *              relative to the LevelX file system's logical block addressing.
 * @param eraseCnt Erase count parameter (currently unused and ignored). 
 *                 This parameter is reserved for future wear-leveling 
 *                 or erase cycle tracking implementations.
 * 
 * @return LX_SUCCESS Block erase operation completed successfully
 *         LX_ERROR Erase operation failed due to invalid parameters or 
 *                  hardware erase failure
 * 
 * This function erases a logical block of NAND Flash memory by translating the 
 * logical block address to a physical address and issuing an erase command 
 * through the hardware abstraction layer. Block erasure sets all bits in the 
 * block to 1 (0xFF), which is required before any write operations can be 
 * performed on pages within that block.
 */
static UINT nandFlashDriverBlockErase(LX_NAND_FLASH *pNandFlash, ULONG block, ULONG eraseCnt)
{
    HAL_StatusTypeDef status;
    UINT phyBlkIndex;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;
    (void)(eraseCnt);

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, 0, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    /* Erase the specified block */
    status = HAL_FLASH_Erase_BLOCK(phyBlkIndex, 1);
    if (HAL_OK != status)
    {
        LOGE("%s HAL_FLASH_Erase_BLOCK errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    return LX_SUCCESS;
}

/**
 * @brief Verifies that a NAND Flash block is completely erased
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash 
 *                   configuration, geometry information, and driver context
 * @param block Logical block index (0-based) to verify. This is translated to 
 *              physical block index using the nandStartBlock offset.
 * 
 * @return LX_SUCCESS The block is completely erased (all bytes are 0xFF)
 *         LX_ERROR Verification failed due to invalid parameters, read error,
 *                  or block not properly erased (contains non-0xFF data)
 * 
 * This function performs a comprehensive verification of a NAND Flash block's
 * erase state by reading every page within the specified block and checking
 * that all data bytes contain the erased value (0xFF). The verification is
 * performed by reading each page into a cache buffer and verifying that all
 * 32-bit words in the page are equal to 0xFFFFFFFF. This ensures the block
 * is properly erased and ready for write operations.
 */
static UINT nandFlashDriverBlockEraseVerify(LX_NAND_FLASH *pNandFlash, ULONG block)
{
    UINT page;
    UINT i;
    ULONG *pPageBuffer;
    HAL_StatusTypeDef status;
    UINT phyBlkIndex;
    UINT words;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, 0, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    /* Check all pages in the block */
    for (page = 0; page < pNandLevelxInfo->pagesPerBlock; page++)
    {
        /* Read the page */
        /* Read data from flash */
        status = HAL_FLASH_Read_QSPI((uint8_t *)pNandLevelxInfo->pageCache,
                                     (uint16_t)phyBlkIndex,
                                     (uint8_t)page,
                                     0,
                                     (uint16_t)pNandLevelxInfo->bytesPerPage
                                    );

        if (HAL_OK != status)
        {
            LOGE("%s HAL_FLASH_Read_QSPI errorNo=%d", __func__, status);
            return LX_ERROR;
        }

        pPageBuffer = (ULONG*)pNandLevelxInfo->pageCache;

        /* Verify all bytes are 0xFF */
        words = pNandLevelxInfo->bytesPerPage / sizeof(ULONG);
        for (i = 0; i < words; i++)
        {
            if (pPageBuffer[i] != 0xFFFFFFFF)
            {
                LOGE("%s check error", __func__);
                return LX_ERROR;
            }
        }
    }

    return LX_SUCCESS;
}

/**
 * @brief  Verify if a page is erased
 * @param  block: Block number (0-1023)
 * @param  page: Page number within block (0-63)
 * @return LX_SUCCESS if erased, LX_ERROR if not erased or on failure
 */
static UINT nandFlashDriverPageErasedVerify(LX_NAND_FLASH *pNandFlash, ULONG block, ULONG page)
{
    UINT i;
    UINT words;
    UINT phyBlkIndex;
    ULONG *pPageBuffer;
    ULONG *pSpareBuffer;
    HAL_StatusTypeDef status;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to write */
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, page, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    /* Check all pages in the block */
    /* Read the page */
    /* Read data from flash */
    status = HAL_FLASH_Read_QSPI((uint8_t *)pNandLevelxInfo->pageCache,
                                 (uint16_t)phyBlkIndex,
                                 (uint8_t)page,
                                 0,
                                 (uint16_t)pNandLevelxInfo->bytesPerPage
                                         + pNandLevelxInfo->spareLength
                                );

    if (HAL_OK != status)
    {
        LOGE("%s HAL_FLASH_Read_QSPI errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    pPageBuffer = (ULONG*)pNandLevelxInfo->pageCache;

    /* Verify all page bytes are 0xFF */
    words = pNandLevelxInfo->bytesPerPage / sizeof(ULONG);
    for (i = 0; i < words; i++)
    {
        if (pPageBuffer[i] != 0xFFFFFFFF)
        {
            LOGE("%s page check error", __func__);
            return LX_ERROR;
        }
    }

    pSpareBuffer = (ULONG*)(pNandLevelxInfo->pageCache + pNandLevelxInfo->bytesPerPage);

    /* Verify all spare bytes are 0xFF */
    words = pNandLevelxInfo->spareLength / sizeof(ULONG);
    for (i = 0; i < words; i++)
    {
        if (pSpareBuffer[i] != 0xFFFFFFFF)
        {
            LOGE("%s spare check error", __func__);
            return LX_ERROR;
        }
    }

    return LX_SUCCESS;
}

/**
 * @brief Verifies that a specific page in a NAND Flash block is completely erased
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash 
 *                   configuration, geometry information, and driver context
 * @param block Logical block index (0-based) containing the page to verify
 * @param page Page number (0-based) within the block to verify for erase state
 * 
 * @return LX_SUCCESS The page is completely erased (all main and spare bytes are 0xFF)
 *         LX_ERROR Verification failed due to invalid parameters, read error, 
 *                  or page not properly erased (contains non-0xFF data)
 * 
 * This function verifies the erase state of a single page within a NAND Flash block,
 * checking both the main data area and the spare/OOB area. It reads the specified
 * page into a cache buffer and verifies that all bytes in both main and spare areas
 * contain the erased value (0xFF). This is useful for verifying individual page
 * erase states without checking the entire block.
 */
static UINT nandFlashDriverBlockStatusGet(LX_NAND_FLASH *pNandFlash, ULONG block, UCHAR *blkStatusByte)
{
    HAL_StatusTypeDef status;
    UINT phyBlkIndex;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;
    //UCHAR *pPageBuffer;
    UCHAR *pSpareBuffer;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, 0, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    /* Read data from flash */
    pSpareBuffer = pNandLevelxInfo->pageCache + pNandLevelxInfo->bytesPerPage;
    status = HAL_FLASH_Read_QSPI((uint8_t *)pSpareBuffer,
                                 (uint16_t)phyBlkIndex,
                                 0,
                                 (uint16_t)pNandLevelxInfo->bytesPerPage,
                                 (uint16_t)pNandLevelxInfo->spareLength
                                );

    if (HAL_OK != status)
    {
        LOGE("%s HAL_FLASH_Read_QSPI errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    //pPageBuffer = pNandLevelxInfo->pageCache;
    
    *blkStatusByte = pSpareBuffer[0];

    return LX_SUCCESS;
}

/**
 * @brief Sets block status information in the spare area of the first page
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash 
 *                   configuration, geometry information, and driver context
 * @param block Logical block index (0-based) whose status is to be set
 * @param blkStatusByte Status byte value to write to the block's spare area.
 *                      Typical values include bad block markers (non-0xFF) or
 *                      other metadata flags.
 * 
 * @return LX_SUCCESS Block status was successfully written to the spare area
 *         LX_ERROR Operation failed due to invalid parameters or write error
 * 
 * This function writes block status information (typically bad block markers or 
 * metadata) to the spare/OOB area of the first page (page 0) of a specified block.
 * It prepares a buffer with the status byte and writes it to the appropriate 
 * location in the spare area. This is commonly used for marking blocks as bad
 * or storing block-specific metadata in NAND Flash management.
 */
static UINT nandFlashDriverBlockStatusSet(LX_NAND_FLASH *pNandFlash, ULONG block, UCHAR blkStatusByte)
{
    HAL_StatusTypeDef status;
    UINT phyBlkIndex;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;
    UCHAR *pSpareBuffer;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to write */
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, 0, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    pSpareBuffer = pNandLevelxInfo->pageCache + pNandLevelxInfo->bytesPerPage;
    memset(pSpareBuffer, 0xff, pNandLevelxInfo->spareLength);
    pSpareBuffer[0] = blkStatusByte;

    /* Write data to flash */
    status = HAL_FLASH_Write_QSPI((uint8_t *)pSpareBuffer,
                                  (uint16_t)phyBlkIndex,
                                  0,
                                  (uint16_t)pNandLevelxInfo->bytesPerPage,
                                  (uint16_t)pNandLevelxInfo->spareLength
                                 );

    if (HAL_OK != status)
    {
        LOGE("%s HAL_FLASH_Write_QSPI errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    return LX_SUCCESS;
}

/**
 * @brief Retrieves extra bytes (spare/OOB area) from a specific NAND Flash page
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash 
 *                   configuration, geometry information, and driver context
 * @param block Logical block index (0-based) containing the target page
 * @param page Page number (0-based) within the block whose spare area is to be read
 * @param pDestBuffer Pointer to the destination buffer where spare bytes will be copied
 * @param size Number of spare bytes to retrieve from the beginning of the spare area
 * 
 * @return LX_SUCCESS Spare bytes were successfully read and copied to destination
 *         LX_ERROR Operation failed due to invalid parameters, boundary violation,
 *                  or read error
 * 
 * This function reads the spare/OOB area bytes from a specified page in a NAND 
 * Flash block and copies them to the destination buffer. The spare area typically 
 * contains metadata such as ECC codes, bad block markers, file system metadata, 
 * and wear-leveling information. The function validates parameters, reads the 
 * entire spare area into a cache, and copies the requested number of bytes to 
 * the caller's buffer.
 */
static UINT nandFlashDriverExtraBytesGet(LX_NAND_FLASH *pNandFlash,
                                         ULONG block,
                                         ULONG page,
                                         UCHAR *pDestBuffer,
                                         UINT size)
{
    HAL_StatusTypeDef status;
    UINT phyBlkIndex;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to write */
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, page, size) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    if (size > pNandLevelxInfo->spareLength)
    {
        LOGE("%s spare size error=%d", __func__, size);
        return LX_ERROR;
    }

    /* Read data from flash */
    status = HAL_FLASH_Read_QSPI((uint8_t *)pNandLevelxInfo->pageCache,
                                 (uint16_t)phyBlkIndex,
                                 (uint8_t)page,
                                 (uint16_t)pNandLevelxInfo->bytesPerPage,
                                 (uint16_t)pNandLevelxInfo->spareLength
                                );

    if (HAL_OK != status)
    {
        LOGE("%s HAL_FLASH_Read_QSPI errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    memcpy(pDestBuffer, pNandLevelxInfo->pageCache, size);

    return LX_SUCCESS;
}

/**
 * @brief Writes extra bytes to the spare/OOB area of a specified NAND Flash page
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash
 *                   configuration, geometry information, and driver context
 * @param block Logical block index (0-based) relative to LevelX addressing
 * @param page Page number (0-based) within the block where spare data will be written
 * @param pSrcBuffer Pointer to source buffer containing data to write to spare area
 * @param size Number of bytes to write to the beginning of the spare area
 * 
 * @return LX_SUCCESS Spare bytes were successfully written to the flash
 *         LX_ERROR Operation failed due to invalid parameters, size violation,
 *                  boundary error, or hardware write failure
 * 
 * This function writes data from a source buffer to the spare/OOB area of a 
 * specific page in a NAND Flash block. It is typically used to store metadata
 * such as ECC codes, bad block markers, file system information, or wear-leveling
 * data. The function performs parameter validation including boundary checks
 * and size verification before executing the write operation.
 */
static UINT nandFlashDriverExtraBytesSet(LX_NAND_FLASH *pNandFlash,
                                         ULONG block,
                                         ULONG page,
                                         UCHAR *pSrcBuffer,
                                         UINT size)
{
    HAL_StatusTypeDef status;
    UINT phyBlkIndex;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to write */
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, page, size) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    if (size > pNandLevelxInfo->spareLength)
    {
        LOGE("%s spare size error=%d", __func__, size);
        return LX_ERROR;
    }

    /* Write data to flash */
    status = HAL_FLASH_Write_QSPI((uint8_t *)pSrcBuffer,
                                  (uint16_t)phyBlkIndex,
                                  (uint8_t)page,
                                  (uint16_t)pNandLevelxInfo->bytesPerPage,
                                  (uint16_t)size
                                 );

    if (HAL_OK != status)
    {
        LOGE("%s HAL_FLASH_Write_QSPI errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    return LX_SUCCESS;
}

/**
 * @brief  System error handler
 * @param  error_code: Error code from LevelX
 * @return LX_SUCCESS (always returns success to allow LevelX to continue)
 */
static UINT nandFlashDriverSystemError(LX_NAND_FLASH *pNandFlash, UINT errCode, ULONG block, ULONG page)
{
    (void)(pNandFlash);

    /* Log or handle the error as needed */
    LOGE_COLOR(COL_RED, "%s block=%d, page=%d, errCode=%d",
                __func__,
                block,
                page,
                errCode
              );
    return LX_SUCCESS;
}

/**
 * @brief Reads multiple consecutive pages from a NAND Flash block 
 *        with separate buffers for main data and spare area
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash
 *                   configuration, geometry information, and driver context
 * @param block Logical block index (0-based) relative to LevelX addressing
 * @param page Starting page number (0-based) within the block to begin reading
 * @param pDestPageBuffer Pointer to destination buffer for main data area.
 *                        If NULL, main data is not copied (only spare data is read).
 * @param pDestSpareBuffer Pointer to destination buffer for spare/OOB area.
 *                         If NULL, spare data is not copied (only main data is read).
 * @param pages Number of consecutive pages to read starting from the specified page
 * 
 * @return LX_SUCCESS All requested pages were successfully read and copied
 *         LX_ERROR Operation failed due to invalid parameters, boundary violation,
 *                  hardware read error, or buffer allocation issues
 * 
 * This function performs bulk read operations of multiple consecutive pages from a 
 * specified NAND Flash block. It allows separate destination buffers for main data 
 * area and spare/OOB area, providing flexibility for applications that handle 
 * data and metadata independently. The function reads each page into a temporary 
 * cache buffer and then copies the data to the appropriate destination buffers.
 */
static UINT nandFlashDriverPagesRead(LX_NAND_FLASH *pNandFlash,
                                     ULONG block,
                                     ULONG page,
                                     UCHAR* pDestPageBuffer,
                                     UCHAR* pDestSpareBuffer,
                                     ULONG pages)
{
    UINT i;
    UINT phyBlkIndex;
    UCHAR *pPageBuffer;
    UCHAR *pSpareBuffer;
    USHORT pageAddress;
    USHORT readSize;
    HAL_StatusTypeDef status;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to write */
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    if (pDestPageBuffer == NULL && pDestSpareBuffer == NULL)
    {
        LOGE("%s NULL buffer error", __func__);
        return LX_ERROR;
    }

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, page + pages -1, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    if (pDestPageBuffer == NULL)
    {
        pageAddress = pNandLevelxInfo->bytesPerPage;
        readSize    = 0;
        pSpareBuffer = pNandLevelxInfo->pageCache;
    }
    else
    {
        pageAddress = 0;
        readSize    = pNandLevelxInfo->bytesPerPage;
        pSpareBuffer = pNandLevelxInfo->pageCache + pNandLevelxInfo->bytesPerPage;
    }

    pPageBuffer = pNandLevelxInfo->pageCache;

    if (pDestSpareBuffer != NULL)
    {
        readSize    += pNandLevelxInfo->spareLength;
    }


    for (i = 0; i < pages; i++)
    {
        status = HAL_FLASH_Read_QSPI((uint8_t *)pNandLevelxInfo->pageCache,
                                     (uint16_t)phyBlkIndex,
                                     (uint8_t)page + i,
                                     (uint16_t)pageAddress,
                                     (uint16_t)readSize
                                    );
        if (HAL_OK != status)
        {
            LOGE("%s HAL_FLASH_Read_QSPI errorNo=%d", __func__, status);
            return LX_ERROR;
        }

        if (pDestPageBuffer != NULL)
        {
            memcpy(pDestPageBuffer, pPageBuffer, pNandLevelxInfo->bytesPerPage);
            pDestPageBuffer += pNandLevelxInfo->bytesPerPage;
        }

        if (pDestSpareBuffer != NULL)
        {
            memcpy(pDestSpareBuffer, pSpareBuffer, pNandLevelxInfo->spareLength);
            pDestSpareBuffer += pNandLevelxInfo->spareLength;
        }
    }

    return LX_SUCCESS;
}

/**
 * @brief Writes multiple consecutive pages to a NAND Flash block 
 *        with separate source buffers for main data and spare area
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash
 *                   configuration, geometry information, and driver context
 * @param block Logical block index (0-based) relative to LevelX addressing
 * @param page Starting page number (0-based) within the block to begin writing
 * @param pSrcPageBuffer Pointer to source buffer containing main data area content.
 *                       Must contain data for all requested pages.
 * @param pSrcSpareBuffer Pointer to source buffer containing spare/OOB area content.
 *                        Must contain data for all requested pages.
 * @param pages Number of consecutive pages to write starting from the specified page
 * 
 * @return LX_SUCCESS All requested pages were successfully written to flash
 *         LX_ERROR Operation failed due to invalid parameters, boundary violation,
 *                  hardware write error, or source buffer issues
 * 
 * This function performs bulk write operations of multiple consecutive pages to a 
 * specified NAND Flash block. It accepts separate source buffers for main data 
 * area and spare/OOB area, allowing applications to provide data and metadata 
 * independently. The function copies data from source buffers to a temporary 
 * cache buffer and then writes the combined page data to the flash memory.
 */
static UINT nandFlashDriverPagesWrite(LX_NAND_FLASH *pNandFlash,
                                ULONG block,
                                ULONG page,
                                UCHAR* pSrcPageBuffer,
                                UCHAR* pSrcSpareBuffer,
                                ULONG pages)
{
    UINT i;
    UINT phyBlkIndex;
    UCHAR *pPageBuffer;
    UCHAR *pSpareBuffer;
    HAL_StatusTypeDef status;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to write */
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    phyBlkIndex = pNandLevelxInfo->nandStartBlock + block;

    if (pSrcPageBuffer == NULL || pSrcSpareBuffer == NULL)
    {
        LOGE("%s NULL buffer error", __func__);
        return LX_ERROR;
    }

    /* Ensure we don't exceed page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, phyBlkIndex, page + pages -1, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }


    for (i = 0; i < pages; i++)
    {
        pPageBuffer = pNandLevelxInfo->pageCache;
        pSpareBuffer = pNandLevelxInfo->pageCache + pNandLevelxInfo->bytesPerPage;

        memcpy(pPageBuffer, pSrcPageBuffer, pNandLevelxInfo->bytesPerPage);
        memcpy(pSpareBuffer, pSrcSpareBuffer, pNandLevelxInfo->spareLength);
        /* Write data to flash */
        status = HAL_FLASH_Write_QSPI((uint8_t *)pNandLevelxInfo->pageCache,
                                      (uint16_t)phyBlkIndex,
                                      (uint8_t)page + i,
                                      (uint16_t)0,
                                      (uint16_t)pNandLevelxInfo->bytesPerPage
                                              + pNandLevelxInfo->spareLength
                                     );
        if (HAL_OK != status)
        {
            LOGE("%s HAL_FLASH_Read_QSPI errorNo=%d", __func__, status);
            return LX_ERROR;
        }

        pSrcPageBuffer += pNandLevelxInfo->bytesPerPage;
        pSrcSpareBuffer += pNandLevelxInfo->spareLength;
    }

    return LX_SUCCESS;
}

/**
 * @brief Copies multiple pages between different blocks in NAND Flash
 * @param pNandFlash Pointer to the LX_NAND_FLASH structure containing flash
 *                   configuration, geometry information, and driver context
 * @param srcBlock Logical source block index (0-based) where data is copied from
 * @param srcPage Starting page number (0-based) within source block to begin copying
 * @param destBlock Logical destination block index (0-based) where data is copied to
 * @param destPage Starting page number (0-based) within destination block to begin writing
 * @param pages Number of pages to copy
 * @param pDataBuffer Pointer to an optional data buffer (currently unused and ignored)
 * 
 * @return LX_SUCCESS All requested pages were successfully copied from source to destination
 *         LX_ERROR Operation failed due to invalid parameters, boundary violations,
 *                  hardware read/write errors, or memory issues
 * 
 * This function performs page-level copy operations between source and destination
 * blocks in NAND Flash memory. It reads each page from the source location into a
 * temporary cache buffer and then writes it to the destination location. This is
 * commonly used for wear-leveling, bad block replacement, and data reorganization
 * operations in flash file systems.
 */
static UINT nandFlashDriverPagesCopy(LX_NAND_FLASH *pNandFlash,
                                ULONG srcBlock,
                                ULONG srcPage,
                                ULONG destBlock,
                                ULONG destPage,
                                ULONG pages,
                                UCHAR *pDataBuffer)
{
    UINT i;
    UINT srcPhyBlkIndex;
    UINT destPhyBlkIndex;
    // UCHAR *pPageBuffer;
    // UCHAR *pSpareBuffer;
    HAL_StatusTypeDef status;
    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;

    (void)(pDataBuffer);

    if (pNandFlash == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandFlash is NULL", __func__);
        return LX_ERROR;
    }

    /* Calculate bytes to write */
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;
    if (pNandLevelxInfo == NULL)
    {
        LOGE_COLOR(COL_RED, "%s pNandLevelxInfo is NULL", __func__);
        return LX_ERROR;
    }

    srcPhyBlkIndex = pNandLevelxInfo->nandStartBlock + srcBlock;
    destPhyBlkIndex = pNandLevelxInfo->nandStartBlock + destBlock;

    /* Ensure we don't exceed src page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, srcPhyBlkIndex, srcPage + pages -1, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }
    /* Ensure we don't exceed dest page boundary */
    if (nandFlashDriverParamCheck(pNandFlash, destPhyBlkIndex, destPage + pages -1, 0) != LX_SUCCESS)
    {
        return LX_ERROR;
    }

    for (i = 0; i < pages; i++)
    {
        /* Read data from flash */
        status = HAL_FLASH_Read_QSPI((uint8_t *)pNandLevelxInfo->pageCache,
                                     (uint16_t)srcPhyBlkIndex,
                                     (uint8_t)srcPage + i,
                                     (uint16_t)0,
                                     (uint16_t)pNandLevelxInfo->bytesPerPage
                                             + pNandLevelxInfo->spareLength
                                    );
        if (HAL_OK != status)
        {
            LOGE("%s HAL_FLASH_Read_QSPI errorNo=%d", __func__, status);
            return LX_ERROR;
        }

        /* Write data to flash */
        status = HAL_FLASH_Write_QSPI((uint8_t *)pNandLevelxInfo->pageCache,
                                      (uint16_t)destPhyBlkIndex,
                                      (uint8_t)destPage + i,
                                      (uint16_t)0,
                                      (uint16_t)pNandLevelxInfo->bytesPerPage
                                              + pNandLevelxInfo->spareLength
                                     );
        if (HAL_OK != status)
        {
            LOGE("%s HAL_FLASH_Write_QSPI errorNo=%d", __func__, status);
            return LX_ERROR;
        }

    }

    return LX_SUCCESS;

}

/**
 * @brief  Initialize the LevelX NAND flash driver for W25N01GVxxIT
 * @param  pNandFlash: Pointer to LevelX NAND flash control block
 * @return LX_SUCCESS on success, error code otherwise
 */
static UINT nandLevelxInitialize(LX_NAND_FLASH *pNandFlash)
{

    t_NandflashLevelxInfo *pNandLevelxInfo = NULL;
    pNandLevelxInfo = pNandFlash->lx_media_driver_info;

    /* Setup param of the NAND flash.  */
    pNandFlash->lx_nand_flash_total_blocks =                  pNandLevelxInfo->numbsOfBlock;
    pNandFlash->lx_nand_flash_bytes_per_page =                pNandLevelxInfo->bytesPerPage;
    pNandFlash->lx_nand_flash_pages_per_block =               pNandLevelxInfo->pagesPerBlock;

    /* Setup function pointers for the NAND flash services.  */
    pNandFlash->lx_nand_flash_driver_read =                   nandFlashDriverRead;
    pNandFlash->lx_nand_flash_driver_write =                  nandFlashDriverWrite;
    pNandFlash->lx_nand_flash_driver_pages_copy =             nandFlashDriverPagesCopy;
    pNandFlash->lx_nand_flash_driver_pages_read =             nandFlashDriverPagesRead;
    pNandFlash->lx_nand_flash_driver_pages_write =            nandFlashDriverPagesWrite;
    pNandFlash->lx_nand_flash_driver_block_erase =            nandFlashDriverBlockErase;
    pNandFlash->lx_nand_flash_driver_system_error =           nandFlashDriverSystemError;
    pNandFlash->lx_nand_flash_driver_extra_bytes_get =        nandFlashDriverExtraBytesGet;
    pNandFlash->lx_nand_flash_driver_extra_bytes_set =        nandFlashDriverExtraBytesSet;
    pNandFlash->lx_nand_flash_driver_block_status_get =       nandFlashDriverBlockStatusGet;
    pNandFlash->lx_nand_flash_driver_block_status_set =       nandFlashDriverBlockStatusSet;
    pNandFlash->lx_nand_flash_driver_page_erased_verify =     nandFlashDriverPageErasedVerify;
    pNandFlash->lx_nand_flash_driver_block_erased_verify =    nandFlashDriverBlockEraseVerify;

    pNandFlash->lx_nand_flash_spare_data1_offset =            pNandLevelxInfo->spareData1Offset;
    pNandFlash->lx_nand_flash_spare_data1_length =            pNandLevelxInfo->spareData1Length;
    pNandFlash->lx_nand_flash_spare_data2_offset =            pNandLevelxInfo->spareData2Offset;
    pNandFlash->lx_nand_flash_spare_data2_length =            pNandLevelxInfo->spareData2Length;
    pNandFlash->lx_nand_flash_spare_total_length =            pNandLevelxInfo->spareLength;

    return(LX_SUCCESS);

}

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
ULONG  nandFlashLevelxFormat(t_NandflashLevelxInfo *pNandLevelxInfo)
{
    UINT status;

    status = lx_nand_flash_format(pNandLevelxInfo->mediaPtr,
                                  pNandLevelxInfo->diskName,
                                  nandLevelxInitialize,
                                  (ULONG *)pNandLevelxInfo->pMemBuffer,
                                  pNandLevelxInfo->memBufferSize);
    if (status != LX_SUCCESS)
    {
        LOGE("%s Nandflash format errorNo=%d", __func__, status);
        return LX_ERROR;
    }

    return(LX_SUCCESS);
}

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
ULONG  nandFlashLevelxFormatReload(t_NandflashLevelxInfo *pNandLevelxInfo)
{
    UINT status;

    pNandLevelxInfo->bInitialed = 0;

    status =  lx_nand_flash_close(pNandLevelxInfo->mediaPtr);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "nandflash lx_nand_flash_close error, errorNum:%d", status);
        return status;
    }

    status = nandFlashLevelxFormat(pNandLevelxInfo);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "nandflash format error, errorNum:%d", status);
        return status;
    }

    status = lx_nand_flash_open(pNandLevelxInfo->mediaPtr,
                                pNandLevelxInfo->diskName,
                                nandLevelxInitialize,
                                (ULONG *)pNandLevelxInfo->pMemBuffer,
                                pNandLevelxInfo->memBufferSize);
    if (status != LX_SUCCESS)
    {
        LOGE_COLOR(COL_RED, "nand lx_nand_flash_open error, errorNum:%d", status);
        return LX_ERROR;
    }

    /* Init success */
    pNandLevelxInfo->bInitialed = 1;

    /* Return success.  */
    return(LX_SUCCESS);
}