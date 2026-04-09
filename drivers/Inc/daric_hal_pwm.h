/**
 ******************************************************************************
 * @file    daric_hal_pwm.h
 * @author  PWM Team
 * @brief   Header file of PWM HAL module.
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
#ifndef DARIC_HAL_PWM_H
#define DARIC_HAL_PWM_H

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
  __IO uint32_t CMD;         ///< PWM Timer Command register.
                             ///< Address offset: 0x00.

  __IO uint32_t CONFIG;      ///< PWM Timer Configuration register.
                             ///< Address offset: 0x04.

  __IO uint32_t THRESHOLD;   ///< PWM Threshold Configuration register.
                             ///< Address offset: 0x08.

  __IO uint32_t TH_CHANNEL0; ///< PWM Channel 0 threshold configuration register.
                             ///< Address offset: 0x0C.

  __IO uint32_t TH_CHANNEL1; ///< PWM Channel 1 threshold configuration register.
                             ///< Address offset: 0x10.

  __IO uint32_t TH_CHANNEL2; ///< PWM Channel 2 threshold configuration register.
                             ///< Address offset: 0x14.

  __IO uint32_t TH_CHANNEL3; ///< PWM Channel 3 threshold configuration register.
                             ///< Address offset: 0x18.

  __IO uint32_t RESERVE[4];  ///< RESERVE.
                             ///< Address offset: 0x1C~0x28

  __IO uint32_t COUNTER;     ///< PWM Timer counter register.
                             ///< Address offset: 0x2C.
} PWM_TypeDef;

typedef struct {
  __IO uint32_t EVENT_CFG;
} PWM_EVENT_CFG_TypeDef;

typedef struct {
  __IO uint32_t CG;
} PWM_CG_TypeDef;

typedef struct {
  __IO uint32_t PREFD[4];
} PWM_PREFD_Array_TypeDef;

/*!< Peripheral memory map */
#define PWM_BASE (0x50120000)
#define PWM_EVENT_CFG_BASE  (PWM_BASE + 0x100UL)
#define PWM_CG_BASE         (PWM_BASE + 0x104UL)
#define PWM_PREFD_BASE      (PWM_BASE + 0x140UL)

#define PWM_TIMER0_BASE     (PWM_BASE + 0x000UL)
#define PWM_TIMER1_BASE     (PWM_BASE + 0x040UL)
#define PWM_TIMER2_BASE     (PWM_BASE + 0x080UL)
#define PWM_TIMER3_BASE     (PWM_BASE + 0x0C0UL)

#define PWM0                ((PWM_TypeDef *)PWM_TIMER0_BASE)
#define PWM1                ((PWM_TypeDef *)PWM_TIMER1_BASE)
#define PWM2                ((PWM_TypeDef *)PWM_TIMER2_BASE)
#define PWM3                ((PWM_TypeDef *)PWM_TIMER3_BASE)

#define PWM0_PREFD          ((__IO uint32_t *)PWM_PREFD_BASE + 0x0UL)
#define PWM1_PREFD          ((__IO uint32_t *)PWM_PREFD_BASE + 0x4UL)
#define PWM2_PREFD          ((__IO uint32_t *)PWM_PREFD_BASE + 0x8UL)
#define PWM3_PREFD          ((__IO uint32_t *)PWM_PREFD_BASE + 0xCUL)

#define PWM_EVENT_CFG       ((PWM_EVENT_CFG_TypeDef *)PWM_EVENT_CFG_BASE)
#define PWM_CG              ((PWM_CG_TypeDef *)PWM_CG_BASE)
#define PWM_PREFD_ARRAY     ((PWM_PREFD_Array_TypeDef *)PWM_PREFD_BASE)

/******************  Bits definition for PWM_CMD register  *****************/
#define PWM_CMD_START_Pos           (0U)
#define PWM_CMD_START_Msk           (0x1UL << PWM_CMD_START_Pos)
#define PWM_CMD_START               PWM_CMD_START_Msk

#define PWM_CMD_STOP_Pos            (1U)
#define PWM_CMD_STOP_Msk            (0x1UL << PWM_CMD_STOP_Pos)
#define PWM_CMD_STOP                PWM_CMD_STOP_Msk

#define PWM_CMD_UPDATE_Pos          (2U)
#define PWM_CMD_UPDATE_Msk          (0x1UL << PWM_CMD_UPDATE_Pos)
#define PWM_CMD_UPDATE              PWM_CMD_UPDATE_Msk

#define PWM_CMD_RESET_Pos           (3U)
#define PWM_CMD_RESET_Msk           (0x1UL << PWM_CMD_RESET_Pos)
#define PWM_CMD_RESET               PWM_CMD_RESET_Msk

#define PWM_CMD_ARM_Pos             (4U)
#define PWM_CMD_ARM_Msk             (0x1UL << PWM_CMD_ARM_Pos)
#define PWM_CMD_ARM                 PWM_CMD_ARM_Msk

/******************  Bits definition for PWM_CONFIG register  *****************/
#define PWM_CONFIG_INSEL_Pos        (0U)
#define PWM_CONFIG_INSEL_Msk        (0xFFUL << PWM_CONFIG_INSEL_Pos)
#define PWM_CONFIG_INSEL            PWM_CONFIG_INSEL_Msk
#define PWM_CONFIG_INSEL_0          (0x01UL << PWM_CONFIG_INSEL_Pos)
#define PWM_CONFIG_INSEL_1          (0x02UL << PWM_CONFIG_INSEL_Pos)
#define PWM_CONFIG_INSEL_2          (0x04UL << PWM_CONFIG_INSEL_Pos)
#define PWM_CONFIG_INSEL_3          (0x08UL << PWM_CONFIG_INSEL_Pos)
#define PWM_CONFIG_INSEL_4          (0x10UL << PWM_CONFIG_INSEL_Pos)
#define PWM_CONFIG_INSEL_5          (0x20UL << PWM_CONFIG_INSEL_Pos)
#define PWM_CONFIG_INSEL_6          (0x40UL << PWM_CONFIG_INSEL_Pos)
#define PWM_CONFIG_INSEL_7          (0x80UL << PWM_CONFIG_INSEL_Pos)

#define PWM_CONFIG_MODE_Pos         (8U)
#define PWM_CONFIG_MODE_Msk         (0x7UL << PWM_CONFIG_MODE_Pos)
#define PWM_CONFIG_MODE             PWM_CONFIG_MODE_Msk
#define PWM_CONFIG_MODE_0           (0x1UL << PWM_CONFIG_MODE_Pos)
#define PWM_CONFIG_MODE_1           (0x2UL << PWM_CONFIG_MODE_Pos)
#define PWM_CONFIG_MODE_2           (0x4UL << PWM_CONFIG_MODE_Pos)

#define PWM_CONFIG_CLKSEL_Pos       (11U)
#define PWM_CONFIG_CLKSEL_Msk       (0x1UL << PWM_CONFIG_CLKSEL_Pos)
#define PWM_CONFIG_CLKSEL           PWM_CONFIG_CLKSEL_Msk

#define PWM_CONFIG_UPDOWNSEL_Pos    (12U)
#define PWM_CONFIG_UPDOWNSEL_Msk    (0x1UL << PWM_CONFIG_UPDOWNSEL_Pos)
#define PWM_CONFIG_UPDOWNSEL        PWM_CONFIG_UPDOWNSEL_Msk

#define PWM_CONFIG_PRESC_Pos        (16U)
#define PWM_CONFIG_PRESC_Msk        (0xFFUL << PWM_CONFIG_PRESC_Pos)
#define PWM_CONFIG_PRESC            PWM_CONFIG_PRESC_Msk

/******************  Bits definition for PWM_THRESHOLD register  ******************/
#define PWM_THRESHOLD_LO_Pos        (0U)
#define PWM_THRESHOLD_LO_Msk        (0xFFFFUL << PWM_THRESHOLD_LO_Pos)
#define PWM_THRESHOLD_LO            PWM_THRESHOLD_LO_Msk

#define PWM_THRESHOLD_HI_Pos        (16U)
#define PWM_THRESHOLD_HI_Msk        (0xFFFFUL << PWM_THRESHOLD_HI_Pos)
#define PWM_THRESHOLD_HI            PWM_THRESHOLD_HI_Msk

/******************  Bits definition for PWM_CHANNEL_TH register  ******************/
#define PWM_CHANNEL_TH_TH_Pos       (0U)
#define PWM_CHANNEL_TH_TH_Msk       (0xFFFFUL << PWM_CHANNEL_TH_TH_Pos)
#define PWM_CHANNEL_TH_TH           PWM_CHANNEL_TH_TH_Msk

#define PWM_CHANNEL_TH_MODE_Pos     (16U)
#define PWM_CHANNEL_TH_MODE_Msk     (0x7UL << PWM_CHANNEL_TH_MODE_Pos)
#define PWM_CHANNEL_TH_MODE         PWM_CHANNEL_TH_MODE_Msk
#define PWM_CHANNEL_TH_MODE_0       (0x1UL << PWM_CHANNEL_TH_MODE_Pos)
#define PWM_CHANNEL_TH_MODE_1       (0x2UL << PWM_CHANNEL_TH_MODE_Pos)
#define PWM_CHANNEL_TH_MODE_2       (0x4UL << PWM_CHANNEL_TH_MODE_Pos)

/******************  Bits definition for PWM_COUNTER register *****************/
#define PWM_COUNTER_COUNTER_Pos     (0U)
#define PWM_COUNTER_COUNTER_Msk     (0xFFFFUL << PWM_COUNTER_COUNTER_Pos)
#define PWM_COUNTER_COUNTER         PWM_COUNTER_COUNTER_Msk

/******************  Bits definition for PWM_EVENT_CFG register  ******************/
#define PWM_EVT_CFG_RX_SADDR_Pos    (0)
#define PWM_EVT_CFG_RX_SADDR_Msk    (0xFFFFUL << PWM_EVT_CFG_RX_SADDR_Pos)
#define PWM_EVT_CFG_RX_SADDR        PWM_EVT_CFG_RX_SADDR

/******************  Bits definition for PWM_CG register  *****************/
#define PWM_CG_ENA_Pos              (0U)
#define PWM_CG_ENA_Msk              (0xFFFFUL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA                  PWM_CG_ENA_Msk
#define PWM_CG_ENA_0                (0x0001UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_1                (0x0002UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_2                (0x0004UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_3                (0x0008UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_4                (0x0010UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_5                (0x0020UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_6                (0x0040UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_7                (0x0080UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_8                (0x0100UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_9                (0x0200UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_10               (0x0400UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_11               (0x0800UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_12               (0x1000UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_13               (0x2000UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_14               (0x4000UL << PWM_CG_ENA_Pos)
#define PWM_CG_ENA_15               (0x8000UL << PWM_CG_ENA_Pos)

/******************  Bits definition for PWM_PREFD register  *****************/
#define PWM_PREFD_CLKFD_Pos         (0U)
#define PWM_PREFD_CLKFD_Msk         (0x3FFUL << PWM_PREFD_CLKFD_Pos)
#define PWM_PREFD_CLKFD             PWM_PREFD_CLKFD_Msk
/** @endcond
 * @}
 */

/*! \brief
 *  PWM state definition
 */
typedef enum {
  HAL_PWM_STATE_RESET   = 0x00U,          ///< Peripheral not yet initialized or disabled.
  HAL_PWM_STATE_READY   = 0x01U,          ///< Peripheral Initialized and ready for use.
  HAL_PWM_STATE_BUSY    = 0x02U,          ///< An internal process is ongoing.
  HAL_PWM_STATE_TIMEOUT = 0x03U,          ///< Timeout state.
  HAL_PWM_STATE_ERROR   = 0x04U,          ///< Reception process is ongoing.
} HAL_PWM_StateTypeDef;

/*! \brief
 *  PWM channel state definition
 */
typedef enum {
  HAL_PWM_CHANNEL_STATE_RESET = 0x00U,    ///< TIM Channel initial state.
  HAL_PWM_CHANNEL_STATE_READY = 0x01U,    ///< TIM Channel ready for use.
  HAL_PWM_CHANNEL_STATE_BUSY  = 0x02U,    ///< An internal process is ongoing on the TIM channel.
} HAL_PWM_ChannelStateTypeDef;

/*! \brief
 *  PWM Activate channel definition
 */
typedef enum {
  HAL_TIM_ACTIVE_CHANNEL_0 = 0x00U,       ///< The active channel is 0.
  HAL_TIM_ACTIVE_CHANNEL_1 = 0x01U,       ///< The active channel is 1.
  HAL_TIM_ACTIVE_CHANNEL_2 = 0x02U,       ///< The active channel is 2.
  HAL_TIM_ACTIVE_CHANNEL_3 = 0x04U,       ///< The active channel is 3.
} HAL_PWM_ActivateChannel;


typedef enum {
  HAL_PWM_EVENT_CB_ID = 0x00U,  ///< PWM event callback.
} HAL_PWM_CallbackIDTypeDef;

/*! \brief
 *  PWM init structure definition
 */
typedef struct {
  uint32_t ClockSelection;      ///< Clock source selection.
                                ///< This parameter can be a value of @ref PWM_Clock_selection.

  uint32_t ClockDivision;       ///< Specifies the clock division.
                                ///< This parameter can be a number between Min_Data = 0x000 and Max_Data = 0x3FF.

  uint32_t Prescaler;           ///< Specifies the prescaler value used to divide the PWM timer clock.
                                ///< This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF.

  uint32_t CounterMode;         ///< Specifies the counter mode.
                                ///< This parameter can be a value of @ref PWM_Counter_Mode.
  
  uint32_t AutoReloadPreload;   ///< PWM counter end-of-count value.
                                ///< This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF.

  uint32_t TriggerMode;         ///< PWM output trigger mode.
                                ///< This parameter can be a value of @ref PWM_Trigger_Mode.

} PWM_InitTypeDef;

/*! \brief
 *  PWM handle structure definition
 */
typedef struct __PWM_HandleTypeDef {
  PWM_TypeDef *Instance;                                            ///< Register base address.
  PWM_InitTypeDef Init;                                             ///< Timer base required parameters.
  HAL_PWM_ActivateChannel Channel;                                  ///< Compare channel.

  // HAL_LockTypeDef Lock;
  __IO HAL_PWM_StateTypeDef State;                                  ///< PWM operation state.
  __IO HAL_PWM_ChannelStateTypeDef ChannelState[4];                 ///< PWM channel operation state.

  void (* MspInitCallback)( struct __PWM_HandleTypeDef * hpwm);     ///< User MSP init callback function pointer.
  void (* MspDeInitCallback)( struct __PWM_HandleTypeDef * hpwm);   ///< User MSP init callback function pointer.
  void (* IRQHandleCallback)( struct __PWM_HandleTypeDef * hpwm);   ///< User interrupt callback function pointer.
} PWM_HandleTypeDef;

/*! \brief
 *  PWM output/compare init structure definition
 */
typedef struct {
  uint32_t OCMode;        ///< Output or compare mode.
                          ///< This parameter can be a value of @ref PWM_Output_Compare_Mode.
  uint32_t Pulse;         ///< Pulse value at active level.
  uint32_t OCPolarity;    ///< Polarity of output or comparison active level.
                          ///< This parameter can be a value of @ref PWM_Output_Compare_Polarity.
} PWM_OC_InitTypeDef;

/* Exported constants --------------------------------------------------------*/
/** @defgroup PWM_Channel PWM Channel
 * @{
 */
#define PWM_CHANNEL_0     0x00000000U   ///< PWM channel 0 identifier.
#define PWM_CHANNEL_1     0x00000001U   ///< PWM channel 1 identifier.
#define PWM_CHANNEL_2     0x00000002U   ///< PWM channel 2 identifier.
#define PWM_CHANNEL_3     0x00000003U   ///< PWM channel 3 identifier.
#define PWM_CHANNEL_ALL   0xFFFFFFFFU   ///< PWM all channels.
/**
 * @}
 */

/** @defgroup PWM_Clock_selection PWM Counter Mode
 * @{
 */
#define PWM_CLOCK_SELECTION_PCLK       0x00000000U         ///< PCLK.
#define PWM_CLOCK_SELECTION_ISCLK      PWM_CONFIG_CLKSEL   ///< System clock after ClockDivision divider.
/**
 * @}
 */

/** @defgroup PWM_Counter_Mode PWM Counter Mode
 * @{
 */
#define PWM_COUNTERMODE_CYCLE         0x00000000U           ///< The counter counts up and down alternatively.
#define PWM_COUNTERMODE_EMPTY         PWM_CONFIG_UPDOWNSEL  ///< The counter counts up and resets to 0 when reach threshold.
/**
 * @}
 */

/** @defgroup PWM_Trigger_Mode PWM Trigger Mode
 * @{
 */
#define PWM_TRIGGERMODE_EACH_CLOCK    0x00000000U                                                 ///< 000: Trigger event at each clock cycle.
#define PWM_TRIGGERMODE_RESET         (PWM_CONFIG_MODE_0)                                         ///< 001: Trigger event if input source is 0.
#define PWM_TRIGGERMODE_SET           (PWM_CONFIG_MODE_1)                                         ///< 010: Trigger event if input source is 1.
#define PWM_TRIGGERMODE_RISING_EDGE   (PWM_CONFIG_MODE_1 | PWM_CONFIG_MODE_0)                     ///< 011: Trigger event on input source rising edge.
#define PWM_TRIGGERMODE_FALLING_EDGE  (PWM_CONFIG_MODE_2)                                         ///< 100: Trigger event on input source falling edge.
#define PWM_TRIGGERMODE_EACH_EDGE     (PWM_CONFIG_MODE_2 | PWM_CONFIG_MODE_0)                     ///< 101: Trigger event on input source falling or rising edge.
#define PWM_TRIGGERMODE_RISING_ARMED  (PWM_CONFIG_MODE_2 | PWM_CONFIG_MODE_1)                     ///< 110: Trigger event on input source rising edge when armed.
#define PWM_TRIGGERMODE_FALLING_ARMED (PWM_CONFIG_MODE_2 | PWM_CONFIG_MODE_1 | PWM_CONFIG_MODE_0) ///< 111: Trigger event on input source falling edge when armed.
/**
 * @}
 */

/** @defgroup PWM_Output_Compare_Mode PWM Output Compare Mode
 * @{
 */
#define PWM_OC_MODE_OUTPUT            0x00000000U                                     ///< PWM output.
#define PWM_OC_MODE_COMPARE           0x00000001U                                     ///< PWM compare.
/**
 * @}
 */

/** @defgroup PWM_Output_Compare_Polarity PWM Output Compare Polarity
 * @{
 */
#define PWM_OCPOLARITY_HIGH           0x00000000U                                     ///< 000: Output set.
#define PWM_OCPOLARITY_TOGGLE_CLEAR   PWM_CHANNEL_TH_MODE_0                           ///< 001: Output toggle then next threshold match action is clear.
#define PWM_OCPOLARITY_SET_CLEAR      PWM_CHANNEL_TH_MODE_1                           ///< 010: Output set then next threshold match action is clear.
#define PWM_OCPOLARITY_TOGGLE         (PWM_CHANNEL_TH_MODE_1 | PWM_CHANNEL_TH_MODE_0) ///< 011: Output toggle.
#define PWM_OCPOLARITY_LOW            PWM_CHANNEL_TH_MODE_2                           ///< 100: Output clear.
#define PWM_OCPOLARITY_TOGGLE_SET     (PWM_CHANNEL_TH_MODE_2 | PWM_CHANNEL_TH_MODE_0) ///< 101: Output toggle then next threshold match action is set.
#define PWM_OCPOLARITY_CLEAR_SET      (PWM_CHANNEL_TH_MODE_2 | PWM_CHANNEL_TH_MODE_1) ///< 110: Output clear then next threshold match action is set.
/**
 * @}
 */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
HAL_StatusTypeDef HAL_PWM_Init(PWM_HandleTypeDef *hpwm);
HAL_StatusTypeDef HAL_PWM_DeInit(PWM_HandleTypeDef *hpwm);
HAL_StatusTypeDef HAL_PWM_Config(PWM_HandleTypeDef *hpwm);
HAL_StatusTypeDef HAL_PWM_ConfigChannel(PWM_HandleTypeDef *hpwm, PWM_OC_InitTypeDef *sConfig, uint32_t Channel);

HAL_StatusTypeDef HAL_PWM_IRQInit(PWM_HandleTypeDef *hpwm);
HAL_StatusTypeDef HAL_PWM_RegisterCallback(PWM_HandleTypeDef *hpwm, HAL_PWM_CallbackIDTypeDef CallbackID, void (*pCallback)(PWM_HandleTypeDef *_hpwm));
HAL_StatusTypeDef HAL_PWM_UnRegisterCallback(PWM_HandleTypeDef *hpwm, HAL_PWM_CallbackIDTypeDef CallbackID);

HAL_StatusTypeDef HAL_PWM_Start(PWM_HandleTypeDef *hpwm, uint32_t Channel);
HAL_StatusTypeDef HAL_PWM_Start_IT(PWM_HandleTypeDef *hpwm, uint32_t Channel);
HAL_StatusTypeDef HAL_PWM_Stop(PWM_HandleTypeDef *hpwm, uint32_t Channel);
HAL_StatusTypeDef HAL_PWM_Stop_IT(PWM_HandleTypeDef *hpwm, uint32_t Channel);
/* The chip doesn't support these functions at the moment. Retained for the time
 * being for possible future use. */
#if 0
HAL_StatusTypeDef HAL_PWM_Update(PWM_HandleTypeDef *hpwm, uint32_t Channel);
HAL_StatusTypeDef HAL_PWM_Reset(PWM_HandleTypeDef *hpwm, uint32_t Channel);
#endif

uint32_t HAL_PWM_GetCounter(PWM_HandleTypeDef *hpwm);

/** @cond Private macros
 * @{
 */
/* Private macros ------------------------------------------------------------*/
#define HAL_PWM_LOCK()    HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_PWM_UNLOCK()  HAL_UNLOCK

/****************** PWM Instances : All supported instances *******************/
#define IS_PWM_INSTANCE(INSTANCE)\
  (((INSTANCE) == PWM0) || ((INSTANCE) == PWM1) || ((INSTANCE) == PWM2) || ((INSTANCE) == PWM3))
#define IS_PWM0_INSTANCE(INSTANCE)  ((INSTANCE) == PWM0)
#define IS_PWM1_INSTANCE(INSTANCE)  ((INSTANCE) == PWM1)
#define IS_PWM2_INSTANCE(INSTANCE)  ((INSTANCE) == PWM2)
#define IS_PWM3_INSTANCE(INSTANCE)  ((INSTANCE) == PWM3)

#define IS_PWM_CLK_SELECTION(__CONFIG__)          ((__CONFIG__ == PWM_CLOCK_SELECTION_ISCLK) || (__CONFIG__ == PWM_CLOCK_SELECTION_PCLK))
#define IS_PWM_CLK_DIVISION(__CONFIG__)           ((__CONFIG__ >= 0) && (__CONFIG__ < 1024))
#define IS_PWM_PRESCALER(__CONFIG__)              ((__CONFIG__ >= 0) && (__CONFIG__ < 256))
#define IS_PWM_COUNTER_MODE(__CONFIG__)           ((__CONFIG__ == PWM_COUNTERMODE_EMPTY) || (__CONFIG__ == PWM_COUNTERMODE_CYCLE))
#define IS_PWM_ATUO_RELOAD_PRELOAD(__CONFIG__)    ((__CONFIG__ >= 0) && (__CONFIG__ < 65536))
#define IS_PWM_REPETITION_COUNTER(__CONFIG__)     ((__CONFIG__) || (1))

#define IS_PWM_CHANNELS(__CHANNEL__)        ((__CHANNEL__ == PWM_CHANNEL_0) || (__CHANNEL__ == PWM_CHANNEL_1)\
                                              || (__CHANNEL__ == PWM_CHANNEL_2) || (__CHANNEL__ == PWM_CHANNEL_3)\
                                              || (__CHANNEL__ == PWM_CHANNEL_ALL))
#define IS_PWM_OC_MODE(__CONFIG__)          ((__CONFIG__ == PWM_OC_MODE_OUTPUT))

#define IS_PWM_OC_POLARITY(__CONFIG__)      ((__CONFIG__ == PWM_OCPOLARITY_HIGH) || (__CONFIG__ == PWM_OCPOLARITY_TOGGLE_CLEAR)\
                                              || (__CONFIG__ == PWM_OCPOLARITY_SET_CLEAR) || (__CONFIG__ == PWM_OCPOLARITY_TOGGLE)\
                                              || (__CONFIG__ == PWM_OCPOLARITY_LOW) || (__CONFIG__ == PWM_OCPOLARITY_TOGGLE_SET)\
                                              || (__CONFIG__ == PWM_OCPOLARITY_CLEAR_SET))

#define IS_PWM_IRQCB_ID(__IRQCB_ID__)       ((__IRQCB_ID__ == HAL_PWM_EVENT_CB_ID))

#define PWM_CHANNEL_STATE_GET(__HANDLE__, __CHANNEL__)\
  (((__CHANNEL__) == PWM_CHANNEL_0) ? (__HANDLE__)->ChannelState[0] :\
   ((__CHANNEL__) == PWM_CHANNEL_1) ? (__HANDLE__)->ChannelState[1] :\
   ((__CHANNEL__) == PWM_CHANNEL_2) ? (__HANDLE__)->ChannelState[2] :\
   (__HANDLE__)->ChannelState[3])

#define PWM_CHANNEL_STATE_SET(__HANDLE__, __CHANNEL__, __CHANNEL_STATE__) \
  (((__CHANNEL__) == PWM_CHANNEL_0) ? ((__HANDLE__)->ChannelState[0] = (__CHANNEL_STATE__)) :\
   ((__CHANNEL__) == PWM_CHANNEL_1) ? ((__HANDLE__)->ChannelState[1] = (__CHANNEL_STATE__)) :\
   ((__CHANNEL__) == PWM_CHANNEL_2) ? ((__HANDLE__)->ChannelState[2] = (__CHANNEL_STATE__)) :\
   ((__HANDLE__)->ChannelState[3] = (__CHANNEL_STATE__)))

#define PWM_CHANNEL_STATE_SET_ALL(__HANDLE__,  __CHANNEL_STATE__) do { \
                                                                       (__HANDLE__)->ChannelState[0]  = \
                                                                       (__CHANNEL_STATE__);  \
                                                                       (__HANDLE__)->ChannelState[1]  = \
                                                                       (__CHANNEL_STATE__);  \
                                                                       (__HANDLE__)->ChannelState[2]  = \
                                                                       (__CHANNEL_STATE__);  \
                                                                       (__HANDLE__)->ChannelState[3]  = \
                                                                       (__CHANNEL_STATE__);  \
                                                                     } while(0)

#define __HAL_PWM_OUTPUT_ENABLE(__HANDLE__, __CHANNEL__)\
  PWM_CG->CG |= (0x1UL << (((__HANDLE__)->Instance == PWM0) ? 0 :\
                           ((__HANDLE__)->Instance == PWM1) ? 1 :\
                           ((__HANDLE__)->Instance == PWM2) ? 2 :\
                           3))

#define __HAL_PWM_OUTPUT_DISABLE(__HANDLE__, __CHANNEL__)\
  PWM_CG->CG &= ~(0x1UL << (((__HANDLE__)->Instance == PWM0) ? 0 :\
                            ((__HANDLE__)->Instance == PWM1) ? 1 :\
                            ((__HANDLE__)->Instance == PWM2) ? 2 :\
                            3))

#define __HAL_PWM_RESET_REGISTER(__HANDLE__)  do {\
                                                  (__HANDLE__)->Instance->CMD = 0x00000000;\
                                                  (__HANDLE__)->Instance->CONFIG = 0x00000000;\
                                                  (__HANDLE__)->Instance->THRESHOLD = 0x00000000;\
                                                  (__HANDLE__)->Instance->TH_CHANNEL0 = 0x00000000;\
                                                  (__HANDLE__)->Instance->TH_CHANNEL1 = 0x00000000;\
                                                  (__HANDLE__)->Instance->TH_CHANNEL2 = 0x00000000;\
                                                  (__HANDLE__)->Instance->TH_CHANNEL3 = 0x00000000;\
                                                  PWM_PREFD_ARRAY->PREFD[(((__HANDLE__)->Instance == PWM0) ? 0 :\
                                                                          ((__HANDLE__)->Instance == PWM1) ? 1 :\
                                                                          ((__HANDLE__)->Instance == PWM2) ? 2 :\
                                                                          3)] = 0x00000000;\
                                              }while(0)

#define __HAL_PWM_OUTPUT_START(__HANDLE__)    ((__HANDLE__)->Instance->CMD = PWM_CMD_START)
#define __HAL_PWM_OUTPUT_STOP(__HANDLE__)     ((__HANDLE__)->Instance->CMD = PWM_CMD_STOP)
/** @endcond
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
