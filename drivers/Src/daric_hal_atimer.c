/**
 ******************************************************************************
 * @file    daric_hal_atimer.c
 * @author  ATimer Team
 * @brief   ATimer HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the ATimer.
 *
 *          features:
 *           + 2 general purpose 32-bit upwards counters(ATimer0----ATimer_LO
 *             and ATimer1----ATimer_HI).
 *           + Can be triggered by multiple sources:
 *             - PCLK
 *             - 32 kHz (32K always. Actual test is 48 kHz.)
 *           + 8-bit programmable prescaler (divides clock frequency).
 *           + Different counting modes:
 *             - One shot mode: timer is stopped after the first comparison
 *               match.
 *             - Continuous mode: timer continues couting after a match.
 *             - 64-bit cascaded mode: use both 32-bit timers as a.
 *
 *          peripheral:
 *           + Initialization and de-initialization functions.
 *           + Interrupt Management.
 *           + IO operation functions.
 *           + Status Information.
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

#ifdef HAL_ATIMER_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* ATIMER IRQ Priority define */
#ifdef HAL_ATIMER_IRQ_PRIO
#define ATIMER_IRQ_PRIO HAL_ATIMER_IRQ_PRIO
#else
#define ATIMER_IRQ_PRIO 0
#endif

#ifdef HAL_ATIMER_IRQ_SUB_PRIO
#define ATIMER_IRQ_SUB_PRIO HAL_ATIMER_IRQ_SUB_PRIO
#else
#define ATIMER_IRQ_SUB_PRIO 0
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static void ATimer_SetConfig(ATIMER_TypeDef *Instance, ATimer_InitTypeDef *Init)
{
    __IO uint32_t tmp_reg;

    if (Init->Prescaler > 0)
    {
        tmp_reg = ((Init->ClockSelection) | (Init->ConterMode) | (Init->RepetitionMode) | (Init->CascadeMode) | (ATIMER_CFG_RST) | (Init->Prescaler << ATIMER_CFG_PVAL_Pos)
                   | (ATIMER_CFG_PEN));
    }
    else
    {
        tmp_reg = ((Init->ClockSelection) | (Init->ConterMode) | (Init->RepetitionMode) | (Init->CascadeMode) | (ATIMER_CFG_RST) | (Init->Prescaler << ATIMER_CFG_PVAL_Pos))
                  & (~ATIMER_CFG_PEN);
    }

    Instance->CFG = tmp_reg;
    Instance->CMP = Init->AutoReloadPreload;
}

static void HAL_ATimer_IRQHandler(ATimer_HandleTypeDef *htim)
{
    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* User interrupt callback */
    if (htim->IRQHandleCallback)
    {
        htim->IRQHandleCallback(htim);
    }

    /* Clear the interrupt flag */
    __HAL_ATIMER_CLEAR_INT_FLAG(htim);
}

/*! \brief
 * Initializes the ATimer according to the specified parameters in the
 * ATimer_InitTypeDef and create the associated handle.
 *
 * @param htim: Pointer to a ATimer_HandleTypeDef structure that contains
 *              the configuration information for the specified ATimer.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_ATimer_Init(ATimer_HandleTypeDef *htim)
{
    /* Check the atimer handle allocation */
    if (htim == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));
    assert_param(IS_ATIMER_CLK_SELECTION(htim->Init.ClockSelection));
    assert_param(IS_ATIMER_CLK_DIVISION(htim->Init.ClockDivision));
    assert_param(IS_ATIMER_PRESCALER(htim->Init.Prescaler));
    assert_param(IS_ATIMER_COUNTER_MODE(htim->Init.CounterMode));
    assert_param(IS_ATIMER_ATUO_RELOAD_PRELOAD(htim->Init.AutoReloadPreload));
    assert_param(IS_ATIMER_REPETITION_MODE(htim->Init.RepetitionMode));
    assert_param(IS_ATIMER_CASCADE_MODE(htim->Init.RepetitionMode));

    /* Process Locked */
    HAL_ATIMER_LOCK();

    if (htim->State == HAL_ATIMER_STATE_RESET)
    {
        /* User MCU support package init callback function */
        if (htim->MspInitCallback)
        {
            htim->MspInitCallback(htim);
        }
    }

    /* Set the ATimer state */
    htim->State = HAL_ATIMER_STATE_BUSY;

    /* Init the base time for the ATimer */
    ATimer_SetConfig(htim->Instance, &htim->Init);

    /* Initialize the ATimer state*/
    htim->State = HAL_ATIMER_STATE_READY;

    /* Process UnLocked */
    HAL_ATIMER_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/*! \brief
 * DeInitializes the ATimer peripheral.
 *
 * @param htim: ATimer handle.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_ATimer_DeInit(ATimer_HandleTypeDef *htim)
{
    /* Check the ATimer handle allocation */
    if (htim == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* Process Locked */
    HAL_ATIMER_LOCK();

    htim->State = HAL_ATIMER_STATE_BUSY;

    /* User MCU support package de-init callback function */
    if (htim->MspDeInitCallback)
    {
        htim->MspDeInitCallback(htim);
    }

    /* Disable the channel  */
    __HAL_ATIMER_DISABLE(htim);

    /* Disable the Peripheral */
    __HAL_ATIMER_STOP(htim);

    /* Reset ATimer control register */
    __HAL_ATIMER_RESET_REGISTER(htim);

    /* Set the TIM state */
    htim->State = HAL_ATIMER_STATE_RESET;

    /* Process UnLocked */
    HAL_ATIMER_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/*! \brief
 * This function initializes the ATimer interrupt settings and connects the
 * ATimer interrupt request (IRQ) handler to the Nested Vectored Interrupt
 * Controller (NVIC). It sets up the NVIC to handle PWM interrupts with the
 * specified priority and enables the interrupt.
 *
 * @param htim: ATimer handle.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_ATimer_IRQInit(ATimer_HandleTypeDef *htim)
{
    /* Check the ATimer handle allocation */
    if (htim == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* Check the State */
    if (htim->State != HAL_ATIMER_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_ATIMER_LOCK();

    if (htim->Instance == ATIMER0)
    {
        HAL_NVIC_ConnectIRQ(ATIMER0_IRQn, ATIMER_IRQ_PRIO, ATIMER_IRQ_SUB_PRIO, (void *)HAL_ATimer_IRQHandler, (void *)htim, 1);
    }
    else if (htim->Instance == ATIMER1)
    {
        HAL_NVIC_ConnectIRQ(ATIMER1_IRQn, ATIMER_IRQ_PRIO, ATIMER_IRQ_SUB_PRIO, (void *)HAL_ATimer_IRQHandler, (void *)htim, 1);
    }

    /* Process Unlocked */
    HAL_ATIMER_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/*! \brief
 * Starts the ATimer.
 *
 * @param htim: ATimer handle.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_ATimer_Start(ATimer_HandleTypeDef *htim)
{
    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* Check the ATimer state */
    if (htim->State != HAL_ATIMER_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_ATIMER_LOCK();

    /* Enable the channel  */
    __HAL_ATIMER_ENABLE(htim);

    /* Enable the Peripheral */
    __HAL_ATIMER_START(htim);

    /* Process Unlocked */
    HAL_ATIMER_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/*! \brief
 * Starts the ATimer with enable interrupt.
 *
 * @param htim: ATimer handle.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_ATimer_Start_IT(ATimer_HandleTypeDef *htim)
{
    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* Check the ATimer state */
    if (htim->State != HAL_ATIMER_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_ATIMER_LOCK();

    /* Clear the interrupt flag */
    __HAL_ATIMER_CLEAR_INT_FLAG(htim);

    /* Enable the interrupt */
    __HAL_ATIMER_INT_ENABLE(htim);

    /* Enable the channel */
    __HAL_ATIMER_ENABLE(htim);

    /* Enable the Peripheral */
    __HAL_ATIMER_START(htim);

    /* Process Unlocked */
    HAL_ATIMER_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/*! \brief
 * Stops the ATimer.
 *
 * @param htim: ATimer handle.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_ATimer_Stop(ATimer_HandleTypeDef *htim)
{
    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* Process Locked */
    HAL_ATIMER_LOCK();

    /* Disable the channel  */
    __HAL_ATIMER_DISABLE(htim);

    /* Disable the Peripheral */
    __HAL_ATIMER_STOP(htim);

    /* Process Unlocked */
    HAL_ATIMER_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/*! \brief
 * Stops the ATimer with disable interrupt.
 *
 * @param htim: ATimer handle.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_ATimer_Stop_IT(ATimer_HandleTypeDef *htim)
{
    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* Process Locked */
    HAL_ATIMER_LOCK();

    /* Disable the interrupt */
    __HAL_ATIMER_INT_ENABLE(htim);

    /* Disable the channel  */
    __HAL_ATIMER_DISABLE(htim);

    /* Disable the peripheral */
    __HAL_ATIMER_STOP(htim);

    /* Process Unlocked */
    HAL_ATIMER_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/*! \brief
 * Resets the value of ATimer counter.
 *
 * @param htim: ATimer handle.
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HAL_ATimer_Reset(ATimer_HandleTypeDef *htim)
{
    /* check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* Process Locked */
    HAL_ATIMER_LOCK();

    /* Reset the peripheral */
    __HAL_ATIMER_RESET(htim);

    /* Process Unlocked */
    HAL_ATIMER_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/*! \brief
 * Gets the value of ATimer counter.
 *
 * @param htim: ATimer handle.
 * @return Counter value.
 */
uint64_t HAL_ATimer_GetCounter(ATimer_HandleTypeDef *htim)
{
    uint64_t counter;

    /* Check the parameters */
    assert_param(IS_ATIMER_INSTANCE(htim->Instance));

    /* Process Locked */
    HAL_ATIMER_LOCK();

    /* Get the ATimer counter */
    counter = (uint64_t)__HAL_ATIMER_COUNTER(htim);

    /* Process Unlocked */
    HAL_ATIMER_UNLOCK();

    /* Return ATimer counter */
    return counter;
}

#endif /* HAL_ATIMER_MODULE_ENABLED */
