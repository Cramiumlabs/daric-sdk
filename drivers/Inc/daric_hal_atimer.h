/**
 ******************************************************************************
 * @file    daric_hal_atimer.h
 * @author  ATimer Team
 * @brief   Header file of ATimer HAL module.
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
#ifndef DARIC_HAL_ATIMER_H
#define DARIC_HAL_ATIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include <stdbool.h>

    // clang-format off
/** @cond Peripheral memory map
 * @{
 */
typedef struct {
  __IO uint32_t CFG;          ///< Timer Configuration register.
                              ///< Address offset: 0x00.

  __IO uint32_t RESERVE0[1];  ///< RESERVE0.
                              ///< Address offset: 0x04.

  __IO uint32_t CNT;          ///< Timer counter value register.
                              ///< Address offset: 0x08

  __IO uint32_t RESERVE1[1];  ///< RESERVE1.
                              ///< Address offset: 0x0C.

  __IO uint32_t CMP;          ///< Timer comparator value register.
                              ///< Address offset: 0x10.

  __IO uint32_t RESERVE2[1];  ///< RESERVE2.
                              ///< Address offset: 0x14.

  __IO uint32_t START;        ///< Start Timer counting register.
                              ///< Address offset: 0x18.

  __IO uint32_t RESERVE3[1];  ///< RESERVE3.
                              ///< Address offset: 0x1C.

  __IO uint32_t RESET;        ///< Reset Timer counter register.
                              ///< Address offset: 0x20.
} ATIMER_TypeDef;

/*!< Peripheral memory map */
#define ATIMER_BASE (0x40043000)
#define ATIMER_AO_BASE (0x40063000)

#define ATIMER_LO_BASE (ATIMER_BASE + 0x00UL)
#define ATIMER_HI_BASE (ATIMER_BASE + 0x04UL)

#define ATIMER0 ((ATIMER_TypeDef *)ATIMER_LO_BASE)
#define ATIMER1 ((ATIMER_TypeDef *)ATIMER_HI_BASE)
#define ATIMER_AO ((ATIMER_TypeDef *)ATIMER_AO_BASE)

/******************  Bits definition for ATimer_CFG register  *****************/
#define ATIMER_CFG_EN_Pos     (0U)
#define ATIMER_CFG_EN_Msk     (0x1UL << ATIMER_CFG_EN_Pos)
#define ATIMER_CFG_EN         ATIMER_CFG_EN_Msk

#define ATIMER_CFG_RST_Pos    (1U)
#define ATIMER_CFG_RST_Msk    (0x1UL << ATIMER_CFG_RST_Pos)
#define ATIMER_CFG_RST        ATIMER_CFG_RST_Msk

#define ATIMER_CFG_IRQEN_Pos  (2U)
#define ATIMER_CFG_IRQEN_Msk  (0x1UL << ATIMER_CFG_IRQEN_Pos)
#define ATIMER_CFG_IRQEN      ATIMER_CFG_IRQEN_Msk

#define ATIMER_CFG_MODE_Pos   (4U)
#define ATIMER_CFG_MODE_Msk   (0x1UL << ATIMER_CFG_MODE_Pos)
#define ATIMER_CFG_MODE       ATIMER_CFG_MODE_Msk

#define ATIMER_CFG_ONES_Pos   (5U)
#define ATIMER_CFG_ONES_Msk   (0x1UL << ATIMER_CFG_ONES_Pos)
#define ATIMER_CFG_ONES       ATIMER_CFG_ONES_Msk

#define ATIMER_CFG_PEN_Pos    (6U)
#define ATIMER_CFG_PEN_Msk    (0x1UL << ATIMER_CFG_PEN_Pos)
#define ATIMER_CFG_PEN        ATIMER_CFG_PEN_Msk

#define ATIMER_CFG_CCFG_Pos   (7U)
#define ATIMER_CFG_CCFG_Msk   (0x1UL << ATIMER_CFG_CCFG_Pos)
#define ATIMER_CFG_CCFG       ATIMER_CFG_CCFG_Msk

#define ATIMER_CFG_PVAL_Pos   (8U)
#define ATIMER_CFG_PVAL_Msk   (0x1UL << ATIMER_CFG_PVAL_Pos)
#define ATIMER_CFG_PVAL       ATIMER_CFG_PVAL_Msk

#define ATIMER_CFG_CASC_Pos   (31U)
#define ATIMER_CFG_CASC_Msk   (0x1UL << ATIMER_CFG_CASC_Pos)
#define ATIMER_CFG_CASC       ATIMER_CFG_CASC_Msk

/******************  Bits definition for ATimer_START register  *****************/
#define ATIMER_START_EN_Pos   (0U)
#define ATIMER_START_EN_Msk   (0x1UL << ATIMER_START_EN_Pos)
#define ATIMER_START_EN       ATIMER_START_EN_Msk

/******************  Bits definition for ATimer_RESET register  *****************/
#define ATIMER_RESET_RST_Pos  (0U)
#define ATIMER_RESET_RST_Msk  (0x1UL << ATIMER_RESET_RST_Pos)
#define ATIMER_RESET_RST      ATIMER_RESET_RST_Msk
/** @endcond
 * @}
 */

/*! \brief
 *  ATimer state definition
 */
typedef enum {
  HAL_ATIMER_STATE_RESET   = 0x00U, ///< Peripheral not yet initialized or disabled
  HAL_ATIMER_STATE_READY   = 0x01U, ///< Peripheral Initialized and ready for use
  HAL_ATIMER_STATE_BUSY    = 0x02U, ///< An internal process is ongoing
  HAL_ATIMER_STATE_TIMEOUT = 0x03U, ///< Timeout state
  HAL_ATIMER_STATE_ERROR   = 0x04U, ///< Reception process is ongoing
} HAL_ATimer_StateTypeDef;

/*! \brief
 * ATimer init structure definition
 */
typedef struct {
  uint32_t ClockSelection;      ///< Clock source selection.
                                ///< This parameter can be a value of @ref ATimer_Clock_selection.

  uint32_t ClockDivision;       ///< Specifies the clock division.(unused)
                                ///< This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF.

  uint32_t ConterMode;          ///< Specifies the counter mode.
                                ///< This parameter can be a value of @ref ATimer_Counter_Mode.

  uint32_t Prescaler;           ///< Specifies the prescaler value used to divide the PWM timer clock.
                                ///< This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF.'

  uint32_t AutoReloadPreload;   ///< ATimer counter end-of-count value.
                                ///< This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF in 32-bit mode.

  uint32_t RepetitionMode;      ///< ATimer repetition mode.
                                ///< This parameter can be a value of @ref ATimer_Repetition_Mode.

  uint32_t CascadeMode;         ///< ATimer cascaded mode. The clock frequency is based on the low 32bit timer when using this mode.
                                ///< This parameter can be a value of @ref ATimer_Cascade_Mode.

} ATimer_InitTypeDef;

/*! \brief
 * ATimer Handle structure definition
 */
typedef struct __ATimer_HandleTypeDef {
  ATIMER_TypeDef *Instance;                                           ///< Register base address.
                                                                      ///< This parameter can be a value of @ref ATimer_Instance.
  ATimer_InitTypeDef Init;                                            ///< Timer base required parameters.

  HAL_ATimer_StateTypeDef State;                                      ///< ATimer state.

  void (* MspInitCallback)( struct __ATimer_HandleTypeDef *htim);     ///< User MSP init callback function pointer.
  void (* MspDeInitCallback)( struct __ATimer_HandleTypeDef *htim);   ///< User MSP init callback function pointer.
  void (* IRQHandleCallback)( struct __ATimer_HandleTypeDef *htim);   ///< User interrupt callback function pointer.
} ATimer_HandleTypeDef;

/* Exported constants --------------------------------------------------------*/
/** @defgroup ATimer_Clock_selection ATimer Clock Selection
 * @{
 */
#define ATIMER_CLOCK_SELECTION_PCLK       0x00000000U       ///< PCLK.
#define ATIMER_CLOCK_SELECTION_48K        ATIMER_CFG_CCFG   ///< 32 kHz reference clock(Actual test is 48 kHz).
/**
 * @}
 */

/** @defgroup ATimer_Counter_Mode ATimer Counter Mode
 * @{
 */
#define ATIMER_COUNTER_MODE_INC           0x00000000U       ///< Continue incrementing timer low counter after a compare match with ATMER_CMP.
#define ATIMER_COUNTER_MODE_RESET         ATIMER_CFG_MODE   ///< Reset timer to after a compare match with ATIMER_CMP.
/**
 * @}
 */


/** @defgroup ATimer_Repetition_Mode ATimer Repetition Mode
 * @{
 */
#define ATIMER_REPETITION_MODE_REPEAT     0x00000000U       ///< Timer stays enabled after a compare match with ATIMER_CMP.
#define ATIMER_REPETITION_MODE_ONESHOT    ATIMER_CFG_ONES   ///< Timer is disabled after a compare match with ATIMER_CMP.
/**
 * @}
 */

/** @defgroup ATimer_Cascade_Mode ATimer Cascade Mode
 * @{
 */
#define ATIMER_CASCADE_MODE_INDEPENDENT   0x00000000U       ///< Timer independent counting.
#define ATIMER_CASCADE_MODE_CASCADED      ATIMER_CFG_CASC   ///< Timer cascade counting.
/**
 * @}
 */

    // clang-format on
    HAL_StatusTypeDef HAL_ATimer_Init(ATimer_HandleTypeDef *htim);
    HAL_StatusTypeDef HAL_ATimer_DeInit(ATimer_HandleTypeDef *htim);
    HAL_StatusTypeDef HAL_ATimer_IRQInit(ATimer_HandleTypeDef *htim);
    HAL_StatusTypeDef HAL_ATimer_Start(ATimer_HandleTypeDef *htim);
    HAL_StatusTypeDef HAL_ATimer_Start_IT(ATimer_HandleTypeDef *htim);
    HAL_StatusTypeDef HAL_ATimer_Stop(ATimer_HandleTypeDef *htim);
    HAL_StatusTypeDef HAL_ATimer_Stop_IT(ATimer_HandleTypeDef *htim);
    HAL_StatusTypeDef HAL_ATimer_Reset(ATimer_HandleTypeDef *htim);
    uint64_t          HAL_ATimer_GetCounter(ATimer_HandleTypeDef *htim);
    // clang-format off

/// @cond Private macros
/* Private macros ------------------------------------------------------------*/
#define HAL_ATIMER_LOCK() HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_ATIMER_UNLOCK() HAL_UNLOCK

#define IS_ATIMER_INSTANCE(INSTANCE)\
  (((INSTANCE) == ATIMER0) || ((INSTANCE) == ATIMER1))

#define IS_ATIMER_CLK_SELECTION(__CONFIG__)          ((__CONFIG__ == ATIMER_CLOCK_SELECTION_PCLK) || (__CONFIG__ == ATIMER_CLOCK_SELECTION_48K))
#define IS_ATIMER_CLK_DIVISION(__CONFIG__)           ((__CONFIG__ >= 0) && (__CONFIG__ < 1024))
#define IS_ATIMER_PRESCALER(__CONFIG__)              ((__CONFIG__ >= 0) && (__CONFIG__ < 256))
#define IS_ATIMER_COUNTER_MODE(__CONFIG__)           ((__CONFIG__ == ATIMER_COUNTER_MODE_INC) || (__CONFIG__ == ATIMER_COUNTER_MODE_RESET))
#define IS_ATIMER_ATUO_RELOAD_PRELOAD(__CONFIG__)    ((__CONFIG__ >= 0) && (__CONFIG__ < 65536))
#define IS_ATIMER_REPETITION_MODE(__CONFIG__)        ((__CONFIG__) || (1))
#define IS_ATIMER_CASCADE_MODE(__CONFIG__)           ((__CONFIG__ == ATIMER_CASCADE_MODE_INDEPENDENT) || (__CONFIG__ == ATIMER_CASCADE_MODE_CASCADED))

#define __HAL_ATIMER_ENABLE(__HANDLE__)     ((__HANDLE__)->Instance->CFG |= (ATIMER_CFG_EN))
#define __HAL_ATIMER_DISABLE(__HANDLE__)    ((__HANDLE__)->Instance->CFG &= (~ATIMER_CFG_EN))

#define __HAL_ATIMER_INT_ENABLE(__HANDLE__)   ((__HANDLE__)->Instance->CFG |= (ATIMER_CFG_IRQEN))
#define __HAL_ATIMER_INT_DISABLE(__HANDLE__)  ((__HANDLE__)->Instance->CFG &= (~ATIMER_CFG_IRQEN))

#define __HAL_ATIMER_CLEAR_INT_FLAG(__HANDLE__) ((void)(__HANDLE__))

#define __HAL_ATIMER_RESET_REGISTER(__HANDLE__)                                \
  do {                                                                         \
    (__HANDLE__)->Instance->CFG = 0x00000000;                                  \
    (__HANDLE__)->Instance->CNT = 0x00000000;                                  \
    (__HANDLE__)->Instance->CMP = 0x00000000;                                  \
    (__HANDLE__)->Instance->START = 0x00000000;                                \
    (__HANDLE__)->Instance->RESET = 0x00000000;                                \
  } while (0)

#define __HAL_ATIMER_START(__HANDLE__)    ((__HANDLE__)->Instance->START |= (ATIMER_START_EN))
#define __HAL_ATIMER_STOP(__HANDLE__)     ((void)(__HANDLE__))
#define __HAL_ATIMER_RESET(__HANDLE__)    ((__HANDLE__)->Instance->RESET |= (ATIMER_RESET_RST))

#define __HAL_ATIMER_COUNTER(__HANDLE__)  ((__HANDLE__)->Instance->CNT)
/// @endcond

#ifdef __cplusplus
}
#endif

#endif