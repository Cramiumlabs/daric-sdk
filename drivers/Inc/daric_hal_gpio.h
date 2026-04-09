/**
 ******************************************************************************
 * @file    daric_hal_gpio.h
 * @author  GPIO Team
 * @brief   Header file of GPIO HAL module.
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
#ifndef __DARIC_HAL_GPIO_H
#define __DARIC_HAL_GPIO_H

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include <stdbool.h>

/**
 * @brief   GPIO Interrupt Handler definition.
 *
 * This callback function is called when a GPIO interrupt occurs.
 * It should be implemented by the user to handle specific GPIO interrupt
 * events.
 *
 * @param   UserData  A pointer to user-defined data that will be passed to the
 * handler when the interrupt occurs. This allows the user to access additional
 * context or data within the handler function.
 *
 * @note    The function is intended to be registered using HAL_GPIO_Init`.
 * Ensure that the function is implemented to execute quickly,
 * as it will be called within an interrupt context.
 */
typedef void (*GPIO_IsrHandler_t)(void *UserData);

/**
 * @brief  GPIO Initialization Structure definition.
 *
 * This structure defines the configuration for initializing a GPIO pin,
 * including the pin number, mode, pull-up/pull-down settings, and interrupt
 * handling.
 */
typedef struct
{
    uint32_t Pin; /*!< Specifies the GPIO pins to be configured.
                       This parameter can be any value or combination of
                       GPIO_pins_define. */

    uint32_t Mode; /*!< Specifies the operating mode for the selected pins.
                        This parameter can be a value of GPIO_mode_define. */

    uint32_t Pull; /*!< Specifies the Pull-up or Pull-Down activation for the
                        selected pins. This parameter can be a value of
                        GPIO_pull_define. */

    uint32_t Alternate; /*!< Peripheral to be connected to the selected pins. This
                           parameter can be a value of @ref
                           GPIO_Alternate_function_selection */

    GPIO_IsrHandler_t IsrHandler; /*!< Pointer to the interrupt service routine
                                   (ISR) handler function. This function will be
                                   called when the configured GPIO pin triggers
                                   an interrupt. The function must be implemented
                                   by the user and registered through HAL to
                                   handle specific GPIO interrupt events. */

    void *UserData; /*!< A pointer to user-defined data that will be passed to the
                         ISR_Handler function when the interrupt occurs. This can
                         be used to provide context or additional information to
                         the interrupt handler function. */

} GPIO_InitTypeDef;

// clang-format off
/**
 * @brief  GPIO Bit SET and Bit RESET enumeration
 */
typedef enum {
  GPIO_PIN_RESET = 0U, ///< GPIO pin state is reset (low)
  GPIO_PIN_SET         ///< GPIO pin state is set (high)
} GPIO_PinState;

/**
 * @brief  GPIO Interrupt Trigger enumeration
 */
typedef enum {
  GPIO_IT_TRIGGER_RISING, ///< Interrupt trigger on rising edge
  GPIO_IT_TRIGGER_FALLING, ///< Interrupt trigger on falling edge
  GPIO_IT_TRIGGER_HIGH,    ///< Interrupt trigger on high level
  GPIO_IT_TRIGGER_LOW      ///< Interrupt trigger on low level
} GPIO_ITTrigger;
/* Exported constants --------------------------------------------------------*/
/** @defgroup GPIO_ports_define GPIO ports define
  * @{
  */
#define GPIO_PORT_A ((uint32_t)0x00) ///< GPIO Port A selected
#define GPIO_PORT_B ((uint32_t)0x01) ///< GPIO Port B selected
#define GPIO_PORT_C ((uint32_t)0x02) ///< GPIO Port C selected
#define GPIO_PORT_D ((uint32_t)0x03) ///< GPIO Port D selected
#define GPIO_PORT_E ((uint32_t)0x04) ///< GPIO Port E selected
#define GPIO_PORT_F ((uint32_t)0x05) ///< GPIO Port F selected

#define GPIO_PORT_MAX ((uint32_t)0x06) ///< Total number of GPIO ports
/**
  * @}
  */

/** @defgroup GPIO_pins_define GPIO pins define
  * @{
  */
#define GPIO_PIN_0                 ((uint16_t)0x0001)  ///< Pin 0 selected
#define GPIO_PIN_1                 ((uint16_t)0x0002)  ///< Pin 1 selected
#define GPIO_PIN_2                 ((uint16_t)0x0004)  ///< Pin 2 selected
#define GPIO_PIN_3                 ((uint16_t)0x0008)  ///< Pin 3 selected
#define GPIO_PIN_4                 ((uint16_t)0x0010)  ///< Pin 4 selected
#define GPIO_PIN_5                 ((uint16_t)0x0020)  ///< Pin 5 selected
#define GPIO_PIN_6                 ((uint16_t)0x0040)  ///< Pin 6 selected
#define GPIO_PIN_7                 ((uint16_t)0x0080)  ///< Pin 7 selected
#define GPIO_PIN_8                 ((uint16_t)0x0100)  ///< Pin 8 selected
#define GPIO_PIN_9                 ((uint16_t)0x0200)  ///< Pin 9 selected
#define GPIO_PIN_10                ((uint16_t)0x0400)  ///< Pin 10 selected
#define GPIO_PIN_11                ((uint16_t)0x0800)  ///< Pin 11 selected
#define GPIO_PIN_12                ((uint16_t)0x1000)  ///< Pin 12 selected
#define GPIO_PIN_13                ((uint16_t)0x2000)  ///< Pin 13 selected
#define GPIO_PIN_14                ((uint16_t)0x4000)  ///< Pin 14 selected
#define GPIO_PIN_15                ((uint16_t)0x8000)  ///< Pin 15 selected
#define GPIO_PIN_All               ((uint16_t)0xFFFF)  ///< All pins selected

#define GPIO_PIN_MASK              (0x0000FFFFU) ///< PIN mask for assert test
#define GPIO_PIN_MAX_NUM           (16UL) ///< Maximum number of GPIO pin for each port
/**
  * @}
  */

/** @defgroup GPIO_MODE GPIO Configuration Mode
  * @{
  */
#define GPIO_MODE_INPUT                 MODE_INPUT                                        ///< Input Mode
#define GPIO_MODE_OUTPUT                MODE_OUTPUT                                       ///< Output Mode
#define GPIO_MODE_AF                    MODE_AF                                           ///< Alternate Function Input Mode
#define GPIO_MODE_IT_RISING             (MODE_INPUT | EXTI_IT | TRIGGER_RISING)           ///< External Interrupt Mode with Rising edge trigger detection
#define GPIO_MODE_IT_FALLING            (MODE_INPUT | EXTI_IT | TRIGGER_FALLING)          ///< External Interrupt Mode with Falling edge trigger detection
#define GPIO_MODE_IT_HIGH_LEVEL         (MODE_INPUT | EXTI_IT | TRIGGER_HIGH_LEVEL)       ///< External Interrupt Mode with high level trigger detection
#define GPIO_MODE_IT_LOW_LEVEL          (MODE_INPUT | EXTI_IT | TRIGGER_LOW_LEVEL)        ///< External Interrupt Mode with low level trigger detection
/**
  * @}
  */

/** @defgroup GPIO_PULL GPIO Pull-Up or Pull-Down Activation
  * @{
  */
#define  GPIO_PULLUP        (0x00000001U)   ///< Pull-up activation
#define  GPIO_NOPULL        (0x00000000U)   ///< No Pull activation
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/* Initialization and de-initialization functions *****************************/
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);

/* IO operation functions *****************************************************/
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                       GPIO_PinState PinState);
uint8_t HAL_GPIO_GetPinAF(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void HAL_GPIO_EnableST(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, bool Enable);
/* Interrupt functionss *****************************************************/
HAL_StatusTypeDef HAL_GPIO_EnableIT(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, bool Enable);
HAL_StatusTypeDef HAL_GPIO_SetITTrigger(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, GPIO_ITTrigger Trigger);
HAL_StatusTypeDef HAL_GPIO_UnregisterCallback(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);

/// @cond PRIVATE_OUTPUT
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
#define GPIO_MODE_Pos                           0u
#define GPIO_MODE                               (0x3uL << GPIO_MODE_Pos)
#define MODE_INPUT                              (0x0uL << GPIO_MODE_Pos)
#define MODE_OUTPUT                             (0x1uL << GPIO_MODE_Pos)
#define MODE_AF                                 (0x2uL << GPIO_MODE_Pos)
#define MODE_ANALOG                             (0x3uL << GPIO_MODE_Pos)
#define EXTI_MODE_Pos                           16u
#define EXTI_MODE                               (0x3uL << EXTI_MODE_Pos)
#define EXTI_IT                                 (0x1uL << EXTI_MODE_Pos)
#define EXTI_WAKEUP                             (0x2uL << EXTI_MODE_Pos)
#define TRIGGER_MODE_Pos                        20u
#define TRIGGER_MODE                            (0x3uL << TRIGGER_MODE_Pos)
#define TRIGGER_RISING                          (0x0uL << TRIGGER_MODE_Pos)
#define TRIGGER_FALLING                         (0x1uL << TRIGGER_MODE_Pos)
#define TRIGGER_HIGH_LEVEL                      (0x2uL << TRIGGER_MODE_Pos)
#define TRIGGER_LOW_LEVEL                       (0x3uL << TRIGGER_MODE_Pos)
/* Private macros ------------------------------------------------------------*/
#define HAL_GPIO_LOCK    HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_GPIO_UNLOCK  HAL_UNLOCK
#define IS_GPIO_PIN_ACTION(ACTION)                                             \
  (((ACTION) == GPIO_PIN_RESET) || ((ACTION) == GPIO_PIN_SET))
#define IS_GPIO_PORT(__PORT__) (((__PORT__) == GPIO_PORT_A) ||                 \
   ((__PORT__) == GPIO_PORT_B) ||                                              \
   ((__PORT__) == GPIO_PORT_C) ||                                              \
   ((__PORT__) == GPIO_PORT_D) ||                                              \
   ((__PORT__) == GPIO_PORT_E) ||                                              \
   ((__PORT__) == GPIO_PORT_F))
#define IS_GPIO_PIN(__PIN__)                                                   \
  ((((uint32_t)(__PIN__) & GPIO_PIN_MASK) != 0x00U) &&                         \
   (((uint32_t)(__PIN__) & ~GPIO_PIN_MASK) == 0x00U))
#define IS_GPIO_MODE(MODE)                                                     \
  (((MODE) == GPIO_MODE_INPUT) ||                                              \
   ((MODE) == GPIO_MODE_OUTPUT) ||                                             \
   ((MODE) == GPIO_MODE_AF) ||                                                 \
   ((MODE) == GPIO_MODE_IT_RISING) ||                                          \
   ((MODE) == GPIO_MODE_IT_FALLING) ||                                         \
   ((MODE) == GPIO_MODE_IT_HIGH_LEVEL) ||                                      \
   ((MODE) == GPIO_MODE_IT_LOW_LEVEL))
#define IS_GPIO_PULL(PULL)                                                     \
  (((PULL) == GPIO_PULLUP) || ((PULL) == GPIO_NOPULL))
#define GPIO_GET_INDEX(__GPIOx__)                                              \
  (((__GPIOx__) == (GPIOA))   ? GPIO_PORT_A                                    \
   : ((__GPIOx__) == (GPIOB)) ? GPIO_PORT_B                                    \
   : ((__GPIOx__) == (GPIOC)) ? GPIO_PORT_C                                    \
   : ((__GPIOx__) == (GPIOD)) ? GPIO_PORT_D                                    \
   : ((__GPIOx__) == (GPIOE)) ? GPIO_PORT_E                                    \
                              : GPIO_PORT_F)
#define GET_GPIO_PIN_INDEX(GPIO_Pin) (                                         \
    (GPIO_Pin == GPIO_PIN_0)  ? 0 :                                            \
    (GPIO_Pin == GPIO_PIN_1)  ? 1 :                                            \
    (GPIO_Pin == GPIO_PIN_2)  ? 2 :                                            \
    (GPIO_Pin == GPIO_PIN_3)  ? 3 :                                            \
    (GPIO_Pin == GPIO_PIN_4)  ? 4 :                                            \
    (GPIO_Pin == GPIO_PIN_5)  ? 5 :                                            \
    (GPIO_Pin == GPIO_PIN_6)  ? 6 :                                            \
    (GPIO_Pin == GPIO_PIN_7)  ? 7 :                                            \
    (GPIO_Pin == GPIO_PIN_8)  ? 8 :                                            \
    (GPIO_Pin == GPIO_PIN_9)  ? 9 :                                            \
    (GPIO_Pin == GPIO_PIN_10) ? 10 :                                           \
    (GPIO_Pin == GPIO_PIN_11) ? 11 :                                           \
    (GPIO_Pin == GPIO_PIN_12) ? 12 :                                           \
    (GPIO_Pin == GPIO_PIN_13) ? 13 :                                           \
    (GPIO_Pin == GPIO_PIN_14) ? 14 :                                           \
    (GPIO_Pin == GPIO_PIN_15) ? 15 : -1)
#define IS_GPIO_IT_TRIGGER(TRIGGER)                                            \
  (((TRIGGER) == GPIO_IT_TRIGGER_RISING) ||                                    \
   ((TRIGGER) == GPIO_IT_TRIGGER_FALLING) ||                                   \
   ((TRIGGER) == GPIO_IT_TRIGGER_HIGH) ||                                      \
   ((TRIGGER) == GPIO_IT_TRIGGER_LOW))
/* Private functions ---------------------------------------------------------*/
/* DO NOT invoke the following APIs outsides */
void HAL_GPIO_InitComponent(void);
/// @endcond

#endif /* __DARIC_HAL_GPIO_H */
