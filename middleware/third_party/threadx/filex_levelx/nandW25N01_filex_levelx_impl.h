/**
 ******************************************************************************
 * @file    nandW25N01_filex_levelx_impl.h
 * @author  OS Team
 * @brief   Header file of filex&levelx module.
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
#ifndef _NANDW25N01_FILEX_LEVELX_IMPL_H_
#define _NANDW25N01_FILEX_LEVELX_IMPL_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "tx_api.h"
#include "fx_api.h"
#include "lx_api.h"
#include "nandflash_filex_impl.h"
#include "nandW25N01_levelx_app.h"

/**
 * @brief Use the t_NandflashFilexLevelxInfo structure to manage an independent file system.
 *
 */
typedef struct {
    t_NandflashDiskInfo     *nandFlashDiskInfo;     /* Pointer to NandflashDisk control block */
    t_NandflashLevelxInfo   *nandFlashLevelxInfo;   /* Pointer to NandflashLevelx control block */
} t_NandflashFilexLevelxInfo;


/**
 * @brief  This function initializes the filex.
 * @param  none.
 * @return none.
 * 
 * This function initializes the various control data structures 
 * for the FileX System component
 */
extern void nandFlashFilexLevelxSystemInit(void);

/**
 * @brief  Initializes the filex disk info according to the specified parameters.
 * @param  pNandFlashFilexLevelxInfo Pointer to a t_NandflashFilexLevelxInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @return FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function configures the filex disk according to the parameters specified
 * in the t_NandflashFilexLevelxInfo structure, including cache, numOfFats, and other
 * relevant settings.
 */
extern uint32_t nandFlashFilexLevelxDiskLoad(t_NandflashFilexLevelxInfo *pNandFlashFilexLevelxInfo);

/**
 * @brief  Format the file system specified by the parameters.
 * @param  pNandFlashFilexLevelxInfo Pointer to a t_NandflashFilexLevelxInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @return FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system. 
 * Make sure that the data on the disk is no longer needed 
 * before calling this interface.
 */
extern uint32_t nandFlashFilexLevelxDiskFormat(t_NandflashFilexLevelxInfo *pNandFlashFilexLevelxInfo);

#ifdef __cplusplus
}
#endif

#endif