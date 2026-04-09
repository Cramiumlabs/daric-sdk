/**
 ******************************************************************************
 * @file    nandflash_filex_app.h
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
#ifndef _NANDFLASH_FILEX_APP_H_
#define _NANDFLASH_FILEX_APP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"
#include "nandflash_filex_impl.h"



/**
 * @brief  Initializes the data disk info according to the specified parameters.
 * @param  none.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 *
 * This function configures the filex disk according to the parameters specified
 * in the gNandflashDiskInfo structure, including cache, numOfFats, and other
 * relevant settings.
 */
extern  uint32_t appNandflashFilexDiskLoad(void);

/**
 * @brief  Format the data partition specified by the parameters.
 * @param  none.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 *
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system.
 * Make sure that the data on the disk is no longer needed
 * before calling this interface.
 */
extern uint32_t appNandflashFilexDiskFormat(void);

#ifdef __cplusplus
}
#endif

#endif