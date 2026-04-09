/**
 ******************************************************************************
 * @file    reram_filex_levelx_impl.h
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
#ifndef _RERAM_FILEX_LEVELX_APP_H_
#define _RERAM_FILEX_LEVELX_APP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "tx_api.h"
#include "fx_api.h"
#include "lx_api.h"
#include "reram_filex_impl.h"
#include "reram_levelx_app.h"

/**
 * @brief Use the t_ReramFilexLevelxInfo structure to manage an independent file system.
 *
 */
typedef struct {
    t_ReramDiskInfo     *reramDiskInfo;     /* Pointer to ReramDisk control block */
    t_ReramLevelxInfo   *reramLevelxInfo;   /* Pointer to ReramLevelx control block */
} t_ReramFilexLevelxInfo;


/**
 * @brief  This function initializes the filex.
 * @param  none.
 * @retval none.
 * 
 * This function initializes the various control data structures 
 * for the FileX System component
 */
extern void reRamFilexLevelxSystemInit(void);

/**
 * @brief  Initializes the filex disk info according to the specified parameters.
 * @param  reramFilexLevelxInfo Pointer to a t_ReramFilexLevelxInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function configures the filex disk according to the parameters specified
 * in the t_ReramFilexLevelxInfo structure, including cache, numOfFats, and other
 * relevant settings.
 */
extern uint32_t reRamFilexLevelxDiskLoad(t_ReramFilexLevelxInfo *reramFilexLevelxInfo);

/**
 * @brief  Format the file system specified by the parameters.
 * @param  reramFilexLevelxInfo Pointer to a t_ReramFilexLevelxInfo structure that contains
 *         the configuration information for the specified filex disk.
 * @retval FS status indicating the success or error of the initialization process,
 *         FX_SUCCESS means success, return a non-zero value means failure.
 * 
 * This function format the file system specified by the parameters.
 * Please note that this interface will destroy the data on the file system. 
 * Make sure that the data on the disk is no longer needed 
 * before calling this interface.
 */
extern uint32_t reRamFilexLevelxDiskFormat(t_ReramFilexLevelxInfo *reramFilexLevelxInfo);

#ifdef __cplusplus
}
#endif

#endif