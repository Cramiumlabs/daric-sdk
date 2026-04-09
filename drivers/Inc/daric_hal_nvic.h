/**
 ******************************************************************************
 * @file    daric_hal_nvic.h
 * @author  NVIC Team
 * @brief   Header file of NVIC HAL module.
 ******************************************************************************
 * @attention
 *
 * Copyright 2024-2026 CrossBar, Inc.
 * This file has been modified by CrossBar, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DARIC_HAL_NVIC_H
#define __DARIC_HAL_NVIC_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"

/** @addtogroup DARIC_HAL_Driver
 * @{
 */

/** @addtogroup NVIC
 * @{
 */
/* Exported types ------------------------------------------------------------*/
/** @defgroup NVIC_Exported_Types Cortex Exported Types
 * @{
 */

/**
 * @}
 */

/* Exported constants --------------------------------------------------------*/

/** @defgroup NVIC_Exported_Constants NVIC Exported Constants
 * @{
 */

/** @defgroup NVIC_Preemption_Priority_Group NVIC Preemption Priority Group
 * @{
 */
#define NVIC_PRIORITYGROUP_0                                     \
    ((uint32_t)0x00000007U) /*!< 0 bits for pre-emption priority \
                                4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1                                     \
    ((uint32_t)0x00000006U) /*!< 1 bits for pre-emption priority \
                                3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2                                     \
    ((uint32_t)0x00000005U) /*!< 2 bits for pre-emption priority \
                                2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3                                     \
    ((uint32_t)0x00000004U) /*!< 3 bits for pre-emption priority \
                                1 bits for subpriority */
#define NVIC_PRIORITYGROUP_4                                     \
    ((uint32_t)0x00000003U) /*!< 4 bits for pre-emption priority \
                                0 bits for subpriority */
    /**
     * @}
     */

    /**
     * @}
     */

    /**
     * @}
     */

    /* Exported Macros -----------------------------------------------------------*/

    /* Exported functions --------------------------------------------------------*/
    /** @addtogroup NVIC_Exported_Functions
     * @{
     */

    /** @addtogroup NVIC_Exported_Functions_Group1
     * @{
     */
    /* Initialization and de-initialization functions *****************************/
    void HAL_NVIC_SetPriorityGrouping(uint32_t PriorityGroup);
    void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);
    void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);
    void HAL_NVIC_DisableIRQ(IRQn_Type IRQn);

    uint32_t HAL_NVIC_GetPriorityGrouping(void);
    void     HAL_NVIC_GetPriority(IRQn_Type IRQn, uint32_t PriorityGroup, uint32_t *pPreemptPriority, uint32_t *pSubPriority);
    void     HAL_NVIC_ClearPendingIRQ(IRQn_Type IRQn);
    void     HAL_NVIC_SetPendingIRQ(IRQn_Type IRQn);
    uint32_t HAL_NVIC_GetPendingIRQ(IRQn_Type IRQn);
    uint32_t HAL_NVIC_GetActive(IRQn_Type IRQn);
    void     HAL_NVIC_ConnectIRQ(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority, void (*Handler)(const void *Arg), const void *Arg, uint32_t Enable);
    void     HAL_NVIC_SystemReset(void);
/**
 * @}
 */

/**
 * @}
 */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/** @defgroup NVIC_Private_Macros NVIC Private Macros
 * @{
 */
#define IS_NVIC_PRIORITY_GROUP(GROUP)                                                                                                                 \
    (((GROUP) == NVIC_PRIORITYGROUP_0) || ((GROUP) == NVIC_PRIORITYGROUP_1) || ((GROUP) == NVIC_PRIORITYGROUP_2) || ((GROUP) == NVIC_PRIORITYGROUP_3) \
     || ((GROUP) == NVIC_PRIORITYGROUP_4))

#define IS_NVIC_PREEMPTION_PRIORITY(PRIORITY) ((PRIORITY) < 0x10U)

#define IS_NVIC_SUB_PRIORITY(PRIORITY) ((PRIORITY) < 0x10U)

#define IS_NVIC_DEVICE_IRQ(IRQ) ((IRQ) >= 0x00)

    /**
     * @}
     */

    /**
     * @}
     */

    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /* __DARIC_HAL_NVIC_H */
