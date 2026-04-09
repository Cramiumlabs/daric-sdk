/**
 ******************************************************************************
 * @file    daric_hal_gpio.c
 * @author  GPIO Team
 * @brief   GPIO HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the General Purpose Input/Output (GPIO)
 *          peripheral:
 *           + Initialization and de-initialization functions
 *           + IO operation functions
 *           + Interrupt functions
 *
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

/* Includes ------------------------------------------------------------------*/
#include "daric_hal.h"

#ifdef HAL_GPIO_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    GPIO_TypeDef     *GPIOx;
    uint32_t          GPIO_Pin;
    GPIO_IsrHandler_t IsrHandler;
    void             *UserData;
} GPIO_ISR_Config_t;
/* Private defines -----------------------------------------------------------*/
/* GPIO IRQ Priority define */
#ifdef HAL_GPIO_IRQ_PRIO
#define GPIO_IRQ_PRIO HAL_GPIO_IRQ_PRIO
#else
#define GPIO_IRQ_PRIO 0
#endif

#ifdef HAL_GPIO_IRQ_SUB_PRIO
#define GPIO_IRQ_SUB_PRIO HAL_GPIO_IRQ_SUB_PRIO
#else
#define GPIO_IRQ_SUB_PRIO 0
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static GPIO_ISR_Config_t gpio_isr_configs[GPIO_INTCR_NUMBER];
/* Constant pointer to GPIO AFSEL registers */
static __IO GPIO_AFSEL_Array_TypeDef *const s_afsel_arr = GPIO_AFSEL_ARRAY;
/* Constant pointer to GPIO INTCR registers */
static __IO GPIO_INTCR_Array_TypeDef *const s_intcr_arr = GPIO_INTCR_ARRAY;
/* Constant pointer to GPIO INTFR register */
static __IO GPIO_INTFR_TypeDef *const s_intfr = GPIO_INTFR;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static void GPIO_InterruptInit(void)
{
    HAL_GPIO_LOCK
    for (uint32_t i = 0; i < GPIO_INTCR_NUMBER; i++)
    {
        gpio_isr_configs[i].IsrHandler = NULL;
        gpio_isr_configs[i].GPIOx      = NULL;
        gpio_isr_configs[i].GPIO_Pin   = (uint32_t)(-1);
        gpio_isr_configs[i].UserData   = NULL;
    }
    HAL_GPIO_UNLOCK
}

static void GPIO_EXTI_IRQHandler(void)
{
    uint32_t intfr      = s_intfr->INTFR; // Read the interrupt flags from INTFR register
    uint32_t clear_bits = 0;

    HAL_GPIO_LOCK
    for (int i = 0; i < GPIO_INTCR_NUMBER; i++)
    {
        /* Determine the corresponding bit in INTFR */
        int intfr_bit = GPIO_INTCR_NUMBER - 1 - i; // Bit 0 of INTFR corresponds to INTCR7, and so on.

        /* Check if the interrupt flag for INTCRx is set */
        if (intfr & (1 << intfr_bit))
        {
            /* Call the corresponding ISR handler if it is set */
            if (gpio_isr_configs[i].IsrHandler != NULL)
            {
                gpio_isr_configs[i].IsrHandler(gpio_isr_configs[i].UserData);
                clear_bits |= (1 << intfr_bit); // Mark the bit to be cleared
            }
        }
    }
    HAL_GPIO_UNLOCK

    /* Clear the interrupt flags by writing 1 to the appropriate bits */
    if (clear_bits != 0)
    {
        s_intfr->INTFR = clear_bits;
    }
}
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Initializes the GPIOx peripheral according to the specified
 * parameters in the GPIO_Init.
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Init: pointer to a GPIO_InitTypeDef structure that contains
 *         the configuration information for the specified GPIO peripheral.
 * @retval None
 */
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
    uint32_t position = 0x00U;
    uint32_t iocurrent;
    uint32_t temp;

    /* Check the parameters */
    assert_param(GPIO_Init != NULL);
    assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Init->Pin));
    assert_param(IS_GPIO_MODE(GPIO_Init->Mode));

    /* Configure the port pins */
    while (((GPIO_Init->Pin) >> position) != 0x00U)
    {
        /* Get current io position */
        iocurrent = (GPIO_Init->Pin) & (1UL << position);

        if (iocurrent != 0x00U)
        {
            /* Get Port Index */
            uint32_t index     = GPIO_GET_INDEX(GPIOx);
            uint32_t alternate = GPIO_AFSELL_PX0_GPIO; // default GPIO

            assert_param(index < GPIO_PORT_MAX);
            /*--------------------- GPIO Mode Configuration ------------------------*/
            if ((GPIO_Init->Mode & GPIO_MODE) != MODE_ANALOG)
            {
                /* Check the Pull parameter */
                assert_param(IS_GPIO_PULL(GPIO_Init->Pull));

                /* Activate the Pull-up resistor for the current IO */
                temp = GPIOx->PUCR;
                temp &= ~(GPIO_PUCR_PUPD0 << position);
                temp |= ((GPIO_Init->Pull) << position);
                GPIOx->PUCR = temp;
            }
            /* In case of Alternate function mode selection */
            if ((GPIO_Init->Mode & GPIO_MODE) == MODE_AF)
            {
                /* Check the Alternate function parameters */
                assert_param(IS_GPIO_AF(GPIO_Init->Alternate));
                alternate = GPIO_Init->Alternate;
            }
            /* Configure Alternate function mapped with the current IO */
            temp = s_afsel_arr->PORTS[index * 2 + (position >> 3UL)];
            temp &= ~(GPIO_AFSELL_PX0 << ((position & 0x07U) * 2U));
            temp |= ((alternate) << ((position & 0x07U) * 2U));
            s_afsel_arr->PORTS[index * 2 + (position >> 3UL)] = temp;

            /* Configure IO Direction mode (Input, Output) */
            if ((GPIO_Init->Mode & GPIO_MODE) == MODE_INPUT || (GPIO_Init->Mode & GPIO_MODE) == MODE_OUTPUT)
            {
                /* Configure output enable register for the current IO */
                temp = GPIOx->OER;
                temp &= ~(GPIO_OE_D0 << position);
                temp |= (((GPIO_Init->Mode & GPIO_MODE) >> GPIO_MODE_Pos) << position);
                GPIOx->OER = temp;
            }
            /*--------------------- EXTI Mode Configuration ------------------------*/
            /* Configure the External Interrupt for the current IO */
            HAL_GPIO_LOCK
            if ((GPIO_Init->Mode & EXTI_MODE) != 0x00U)
            {
                int32_t intcrx = -1;

                /* Find the index of the interrupt configuration for the specified GPIO */
                for (int32_t i = 0; i < GPIO_INTCR_NUMBER; i++)
                {
                    if (gpio_isr_configs[i].GPIOx == GPIOx && gpio_isr_configs[i].GPIO_Pin == iocurrent)
                    {
                        intcrx                         = i;
                        gpio_isr_configs[i].IsrHandler = GPIO_Init->IsrHandler;
                        gpio_isr_configs[i].UserData   = GPIO_Init->UserData;
                        break;
                    }
                }
                if (intcrx != -1)
                {
                    /* Already initialized before */
                    goto set_intcr;
                }

                /* Find an empty INTCR slot for the specified GPIO */
                for (int32_t i = 0; i < GPIO_INTCR_NUMBER; i++)
                {
                    if (gpio_isr_configs[i].GPIOx == NULL)
                    {
                        intcrx = i;
                        /* occupy the slot */
                        gpio_isr_configs[i].GPIOx      = GPIOx;
                        gpio_isr_configs[i].GPIO_Pin   = iocurrent;
                        gpio_isr_configs[i].IsrHandler = GPIO_Init->IsrHandler;
                        gpio_isr_configs[i].UserData   = GPIO_Init->UserData;
                        break;
                    }
                }

                assert_param(intcrx != -1);

            set_intcr:
                /* Configure INTCR */
                temp = 0;
                if ((GPIO_Init->Mode & EXTI_MODE) & EXTI_IT)
                {
                    temp |= GPIO_INTCR_INTEN;
                }
                if ((GPIO_Init->Mode & EXTI_MODE) & EXTI_WAKEUP)
                {
                    temp |= GPIO_INTCR_WKUPE;
                }
                temp |= (((GPIO_Init->Mode & TRIGGER_MODE) >> TRIGGER_MODE_Pos) << GPIO_INTCR_INTMOD_Pos);
                temp |= (index * GPIO_PIN_MAX_NUM) + position;
                s_intcr_arr->INTCR[intcrx] = temp;
            }
            HAL_GPIO_UNLOCK
        }
        position++;
    }
}

/**
 * @brief  De-initializes the GPIOx peripheral registers to their default
 * reset values.
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to be written.
 *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
 * @retval None
 */
void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
    uint32_t position = 0x00U;
    uint32_t iocurrent;
    uint32_t temp;

    /* Check the parameters */
    assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin));

    /* Configure the port pins */
    while ((GPIO_Pin >> position) != 0x00U)
    {
        /* Get current io position */
        iocurrent = GPIO_Pin & (1UL << position);

        if (iocurrent != 0x00U)
        {
            /* Get Port Index */
            uint32_t index = GPIO_GET_INDEX(GPIOx);

            /*------------------------- EXTI Mode Configuration --------------------*/
            HAL_GPIO_UnregisterCallback(GPIOx, iocurrent);

            /*------------------------- GPIO Mode Configuration --------------------*/
            /* Revert alternate function to GPIO for the current IO */
            temp = s_afsel_arr->PORTS[index * 2 + (position >> 3UL)];
            temp &= ~(GPIO_AFSELL_PX0 << ((position & 0x07U) * 2U));
            temp |= ((GPIO_AFSELL_PX0_GPIO) << ((position & 0x07U) * 2U));
            s_afsel_arr->PORTS[index * 2 + (position >> 3UL)] = temp;

            /* set NO PULL in GPIOPU for the current IO */
            temp = GPIOx->PUCR;
            temp &= ~(GPIO_PUCR_PUPD0 << position);
            temp |= ((GPIO_NOPULL) << position);
            GPIOx->PUCR = temp;
            /* Reset GPIO output enable register */
            temp = GPIOx->OER;
            temp &= ~(1UL << position); // Disable the output for this pin
            GPIOx->OER = temp;

            /* Reset the output control register for the pin to default (input mode) */
            temp = GPIOx->OCR;
            temp &= ~(1UL << position);
            GPIOx->OCR = temp;

            /* Reset input schmitter trigger enable registers for the pin to default */
            temp = GPIOx->CISTER;
            temp &= ~(1UL << position);
            GPIOx->CISTER = temp;
        }
        position++;
    }
}

/**
 * @brief  Reads the specified input port pin.
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to read.
 *         This parameter can be GPIO_PIN_x where x can be (0..15).
 * @retval The input port pin value.
 */
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIO_PinState bitstatus;

    /* Check the parameters */
    assert_param(IS_GPIO_PIN(GPIO_Pin));

    if ((GPIOx->ISR & GPIO_Pin) != 0x00U)
    {
        bitstatus = GPIO_PIN_SET;
    }
    else
    {
        bitstatus = GPIO_PIN_RESET;
    }
    return bitstatus;
}

/**
 * @brief  Sets or clears the selected data port bit.
 *
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to be written.
 *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
 * @param  PinState: specifies the value to be written to the selected bit.
 *          This parameter can be one of the GPIO_PinState enum values:
 *            @arg GPIO_PIN_RESET: to clear the port pin
 *            @arg GPIO_PIN_SET: to set the port pin
 * @retval None
 */
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    /* Check the parameters */
    assert_param(IS_GPIO_PIN(GPIO_Pin));
    assert_param(IS_GPIO_PIN_ACTION(PinState));

    if (PinState == GPIO_PIN_SET)
    {
        GPIOx->OCR |= GPIO_Pin;
    }
    else
    {
        GPIOx->OCR &= ~GPIO_Pin;
    }
}

/**
 * @brief  Get the Alternate Function (AF) mapping of a specified GPIO pin.
 * @note   This function retrieves the alternate function configuration
 *         for a specified GPIO pin based on its position and configuration registers.
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the GPIO pin to be read.
 *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
 * @retval uint8_t: The alternate function value (0..3) assigned to the pin.
 */
uint8_t HAL_GPIO_GetPinAF(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    /* Check the parameters */
    assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin));
    uint32_t position = 0; // Variable to store the position of the pin

    uint32_t index = GPIO_GET_INDEX(GPIOx);
    /* Find the position of the lowest set bit in GPIO_Pin */
    while (((GPIO_Pin >> position) & 0x1U) == 0U)
    {
        position++;
    }
    /* Retrieve the Alternate Function configuration for the specified pin */
    uint32_t temp  = s_afsel_arr->PORTS[index * 2 + (position >> 3U)]; // Access the appropriate AFSEL register
    uint8_t  PinAF = (temp >> ((position & 0x07U) * 2U)) & 0x3U;

    return PinAF;
}

/**
 * @brief  Enable or disable the Schmitt Trigger for a specific GPIO pin
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit for Schmitt Trigger control.
 *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
 * @param  Enable: true to enable Schmitt Trigger, false to disable.
 * @retval None
 */
void HAL_GPIO_EnableST(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, bool Enable)
{
    /* Check the parameters */
    assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin));

    if (Enable)
    {
        GPIOx->CISTER |= GPIO_Pin;
    }
    else
    {
        GPIOx->CISTER &= ~GPIO_Pin;
    }
}

/**
 * @brief  Sets the interrupt trigger condition for a specified GPIO pin.
 * @note   This API can temporarily change the interrupt trigger mode for the specified GPIO pin.
 *         The specified pin must have already been configured using HAL_GPIO_Init.
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to be written.
 *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
 * @param  Trigger: Specifies the new interrupt trigger condition.
 *         This parameter can be a value of @ref GPIO_ITTrigger.
 * @retval HAL_StatusTypeDef: HAL status.
 */
HAL_StatusTypeDef HAL_GPIO_SetITTrigger(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, GPIO_ITTrigger Trigger)
{
    uint32_t temp;

    /* Validate the parameters */
    assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin));
    assert_param(IS_GPIO_IT_TRIGGER(Trigger));

    int32_t index = -1;
    HAL_GPIO_LOCK
    /* Find the index of the interrupt configuration for the specified GPIO */
    for (int32_t i = 0; i < GPIO_INTCR_NUMBER; i++)
    {
        if (gpio_isr_configs[i].GPIOx == GPIOx && gpio_isr_configs[i].GPIO_Pin == GPIO_Pin)
        {
            index = i;
            break;
        }
    }
    HAL_GPIO_UNLOCK

    /* Return error if no matching interrupt configuration is found */
    if (index == -1)
    {
        return HAL_ERROR; // No matching interrupt configuration found
    }

    assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin));
    assert_param(IS_GPIO_IT_TRIGGER(Trigger));

    /* Configure the alternate function to map the GPIO with the current IO */
    temp = s_intcr_arr->INTCR[index];
    temp &= ~GPIO_INTCR_INTMOD;                 // Clear the current interrupt mode bits
    temp |= (Trigger << GPIO_INTCR_INTMOD_Pos); // Set the new interrupt trigger mode
    s_intcr_arr->INTCR[index] = temp;

    return HAL_OK; // Successfully updated the interrupt trigger condition
}

/**
 * @brief  Enables or disables the interrupt for a specified GPIO pin.
 * @note   This API can temporarily enable or disable the interrupt for the specified GPIO pin.
 *         The specified pin must have already been configured using HAL_GPIO_Init.
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to be written.
 *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
 * @param  Enable: Specifies whether to enable or disable the interrupt.
 *         This parameter is of type bool:
 *           true: Enable interrupt
 *           false: Disable interrupt
 * @retval HAL_StatusTypeDef: HAL status.
 */
HAL_StatusTypeDef HAL_GPIO_EnableIT(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, bool Enable)
{
    int32_t  index = -1;
    uint32_t temp;

    HAL_GPIO_LOCK
    /* Find the index of the interrupt configuration for the specified GPIO */
    for (int32_t i = 0; i < GPIO_INTCR_NUMBER; i++)
    {
        if (gpio_isr_configs[i].GPIOx == GPIOx && gpio_isr_configs[i].GPIO_Pin == GPIO_Pin)
        {
            index = i;
            break;
        }
    }
    HAL_GPIO_UNLOCK

    /* Return error if no matching interrupt configuration is found */
    if (index == -1)
    {
        return HAL_ERROR;
    }

    /* Retrieve the current interrupt configuration */
    temp = s_intcr_arr->INTCR[index];

    if (Enable)
    {
        temp |= GPIO_INTCR_INTEN; /* Enable the interrupt (set the interrupt enable bit) */
    }
    else
    {
        temp &= ~GPIO_INTCR_INTEN; /* Disable the interrupt (clear the interrupt enable bit) */
    }

    /* Write back the updated configuration */
    s_intcr_arr->INTCR[index] = temp;

    return HAL_OK; // Successfully enabled/disabled the interrupt
}

/**
 * @brief  Unregisters a callback for a specified GPIO pin
 * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to be written.
 *          This parameter can be one of GPIO_PIN_x where x can be (0..15).
 * @retval HAL_StatusTypeDef: HAL status.
 */
HAL_StatusTypeDef HAL_GPIO_UnregisterCallback(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
    int32_t index = -1;

    HAL_GPIO_LOCK
    /* Find the index of the interrupt configuration for the specified GPIO */
    for (int32_t i = 0; i < GPIO_INTCR_NUMBER; i++)
    {
        if (gpio_isr_configs[i].GPIOx == GPIOx && gpio_isr_configs[i].GPIO_Pin == GPIO_Pin)
        {
            index = i;
            break;
        }
    }

    /* Return error if no matching interrupt configuration is found */
    if (index == -1)
    {
        HAL_GPIO_UNLOCK
        return HAL_ERROR;
    }

    /* Remove the entry from gpio_isr_configs */
    gpio_isr_configs[index].IsrHandler = NULL;
    gpio_isr_configs[index].GPIOx      = NULL;
    gpio_isr_configs[index].GPIO_Pin   = (uint32_t)(-1);
    gpio_isr_configs[index].UserData   = NULL;

    HAL_GPIO_UNLOCK
    /* Reset the corresponding INTCR register */
    s_intcr_arr->INTCR[index] = 0; // Clear the register to reset all settings

    return HAL_OK; // Successfully unregistered the callback and reset the INTCR
}

/**
 * @brief  Initializes the GPIO component and configures the interrupt.
 *
 * This function initializes the GPIO interrupt settings and connects the
 * GPIO interrupt request (IRQ) handler to the Nested Vectored Interrupt
 * Controller (NVIC). It sets up the NVIC to handle GPIO interrupts with
 * the specified priority and enables the interrupt.
 *
 * @note   This function must be called during the initialization phase
 *         of the application to ensure that GPIO interrupts are properly
 *         configured and enabled.
 *
 * @retval None
 */
void HAL_GPIO_InitComponent(void)
{
    GPIO_InterruptInit(); /**< Initialize GPIO interrupt settings. */

    /* Connect GPIO NVIC IRQ */
    HAL_NVIC_ConnectIRQ(GPIO_IRQn, GPIO_IRQ_PRIO, GPIO_IRQ_SUB_PRIO, (void *)GPIO_EXTI_IRQHandler, (void *)NULL, 1);
}

#endif /* HAL_GPIO_MODULE_ENABLED */
