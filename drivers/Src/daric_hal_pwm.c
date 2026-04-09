/**
 ******************************************************************************
 * @file    daric_hal_pwm.c
 * @author  PWM Team
 * @brief   PWM HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the Pulse Width Modulation (PWM)
 *          peripheral:
 *           + Initialization and de-initialization functions
 *           + Interrupt Management
 *           + IO operation functions
 *           + Status Information
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

#ifdef HAL_PWM_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* PWM0 IRQ Priority define */
#ifdef HAL_PWM0_IRQ_PRIO
#define PWM0_IRQ_PRIO HAL_PWM0_IRQ_PRIO
#else
#define PWM0_IRQ_PRIO 0
#endif

#ifdef HAL_PWM0_IRQ_SUB_PRIO
#define PWM0_IRQ_SUB_PRIO HAL_PWM0_IRQ_SUB_PRIO
#else
#define PWM0_IRQ_SUB_PRIO 0
#endif

/* PWM1 IRQ Priority define */
#ifdef HAL_PWM1_IRQ_PRIO
#define PWM1_IRQ_PRIO HAL_PWM1_IRQ_PRIO
#else
#define PWM1_IRQ_PRIO 0
#endif

#ifdef HAL_PWM1_IRQ_SUB_PRIO
#define PWM1_IRQ_SUB_PRIO HAL_PWM1_IRQ_SUB_PRIO
#else
#define PWM1_IRQ_SUB_PRIO 0
#endif

/* PWM2 IRQ Priority define */
#ifdef HAL_PWM2_IRQ_PRIO
#define PWM2_IRQ_PRIO HAL_PWM2_IRQ_PRIO
#else
#define PWM2_IRQ_PRIO 0
#endif

#ifdef HAL_PWM2_IRQ_SUB_PRIO
#define PWM2_IRQ_SUB_PRIO HAL_PWM2_IRQ_SUB_PRIO
#else
#define PWM2_IRQ_SUB_PRIO 0
#endif

/* PWM3 IRQ Priority define */
#ifdef HAL_PWM3_IRQ_PRIO
#define PWM3_IRQ_PRIO HAL_PWM3_IRQ_PRIO
#else
#define PWM3_IRQ_PRIO 0
#endif

#ifdef HAL_PWM3_IRQ_SUB_PRIO
#define PWM3_IRQ_SUB_PRIO HAL_PWM3_IRQ_SUB_PRIO
#else
#define PWM3_IRQ_SUB_PRIO 0
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static void PWM_SetConfig(PWM_TypeDef *Instance, PWM_InitTypeDef *Init)
{
    PWM_PREFD_Array_TypeDef *pwmPrefd = PWM_PREFD_ARRAY;

    /* Set the CONFIG register */
    Instance->CONFIG = Init->ClockSelection | (Init->Prescaler << PWM_CONFIG_PRESC_Pos) | Init->CounterMode | Init->TriggerMode;

    /* Set the PREFD register */
    pwmPrefd->PREFD[((Instance == PWM0) ? 0 : (Instance == PWM1) ? 1 : (Instance == PWM2) ? 2 : 3)] = Init->ClockDivision;

    /* Set the AutoReloadPreload */
    Instance->THRESHOLD = (0 << PWM_THRESHOLD_LO_Pos) | (Init->AutoReloadPreload << PWM_THRESHOLD_HI_Pos);
}

static void PWM_SetChannelConfig(PWM_TypeDef *Instance, PWM_OC_InitTypeDef *Init, uint32_t Channel)
{
    __IO uint32_t tmp_reg = Init->OCMode | Init->OCPolarity | Init->Pulse;

    switch (Channel)
    {
        case PWM_CHANNEL_0:
            Instance->TH_CHANNEL0 = tmp_reg;
            break;
        case PWM_CHANNEL_1:
            Instance->TH_CHANNEL1 = tmp_reg;
            break;
        case PWM_CHANNEL_2:
            Instance->TH_CHANNEL2 = tmp_reg;
            break;
        case PWM_CHANNEL_3:
            Instance->TH_CHANNEL3 = tmp_reg;
            break;
        case PWM_CHANNEL_ALL:
            Instance->TH_CHANNEL0 = tmp_reg;
            Instance->TH_CHANNEL1 = tmp_reg;
            Instance->TH_CHANNEL2 = tmp_reg;
            Instance->TH_CHANNEL3 = tmp_reg;
            break;
    }
}

static void HAL_PWM_IRQHandler(PWM_HandleTypeDef *hpwm)
{
    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));

    /* User interrupt callback */
    if (hpwm->IRQHandleCallback)
    {
        hpwm->IRQHandleCallback(hpwm);
    }
}

/**
 * @brief Initializes the PWM according to the specified parameters in the
 * PWM_InitTypeDef and create the associated handle.
 *
 * @param hpwm: Pointer to a PWM_HandleTypeDef structure that contains
 *              the configuration information for the specified PWM timer.
 * @return HAL status.
 */
HAL_StatusTypeDef HAL_PWM_Init(PWM_HandleTypeDef *hpwm)
{
    /* Check the pwm handle allocation */
    if (hpwm == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));
    assert_param(IS_PWM_CLK_SELECTION(hpwm->Init.ClockSelection));
    assert_param(IS_PWM_CLK_DIVISION(hpwm->Init.ClockDivision));
    assert_param(IS_PWM_PRESCALER(hpwm->Init.Prescaler));
    assert_param(IS_PWM_COUNTER_MODE(hpwm->Init.CounterMode));
    assert_param(IS_PWM_ATUO_RELOAD_PRELOAD(hpwm->Init.AutoReloadPreload));
    assert_param(IS_PWM_REPETITION_COUNTER(hpwm->Init.RepetitionCounter));

    /* Process Locked */
    HAL_PWM_LOCK();

    if (hpwm->State == HAL_PWM_STATE_RESET)
    {
        /* User MCU support package init callback function */
        if (hpwm->MspInitCallback)
        {
            hpwm->MspInitCallback(hpwm);
        }
    }

    /* Set the PWM state */
    hpwm->State = HAL_PWM_STATE_BUSY;

    /* Init the base time for the PWM */
    PWM_SetConfig(hpwm->Instance, &hpwm->Init);

    /* Initialize the PWM channels state */
    PWM_CHANNEL_STATE_SET_ALL(hpwm, HAL_PWM_CHANNEL_STATE_RESET);

    /* Initialize the PWM state*/
    hpwm->State = HAL_PWM_STATE_READY;

    /* Process UnLocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief DeInitializes the PWM peripheral.
 *
 * @param hpwm: Pointer to a PWM_HandleTypeDef structure that contains the
 *              configuration information for the specified PWM timer.
 * @return HAL status.
 */
HAL_StatusTypeDef HAL_PWM_DeInit(PWM_HandleTypeDef *hpwm)
{
    /* Check the pwm handle allocation */
    if (hpwm == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));

    /* Stop output and disable interrupt */
    HAL_PWM_Stop_IT(hpwm, PWM_CHANNEL_ALL);

    /* Process Locked */
    HAL_PWM_LOCK();

    hpwm->State = HAL_PWM_STATE_BUSY;

    /* User MCU support package de-init callback function */
    if (hpwm->MspDeInitCallback)
    {
        hpwm->MspDeInitCallback(hpwm);
    }

    /* Reset the register */
    __HAL_PWM_RESET_REGISTER(hpwm);

    /* Initialize the PWM channels state */
    PWM_CHANNEL_STATE_SET_ALL(hpwm, HAL_PWM_CHANNEL_STATE_RESET);

    /* Set the TIM state */
    hpwm->State = HAL_PWM_STATE_RESET;

    /* Process UnLocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief Configures the PWM according to the specified parameters in the
 * PWM_InitTypeDef. After using HAL_PWM_Start() or HAL_PWM_Start_IT(), you need
 * to call HAL_PWM_Update() to make the configuration take effect.
 *
 * @param hpwm: Pointer to a PWM_HandleTypeDef structure that contains
 *              the configuration information for the specified PWM timer.
 * @return HAL status.
 */
HAL_StatusTypeDef HAL_PWM_Config(PWM_HandleTypeDef *hpwm)
{
    /* Check the pwm handle allocation */
    if (hpwm == NULL)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));
    assert_param(IS_PWM_CLK_SELECTION(hpwm->Init.ClockSelection));
    assert_param(IS_PWM_CLK_DIVISION(hpwm->Init.ClockDivision));
    assert_param(IS_PWM_PRESCALER(hpwm->Init.Prescaler));
    assert_param(IS_PWM_COUNTER_MODE(hpwm->Init.CounterMode));
    assert_param(IS_PWM_ATUO_RELOAD_PRELOAD(hpwm->Init.AutoReloadPreload));
    assert_param(IS_PWM_REPETITION_COUNTER(hpwm->Init.RepetitionCounter));

    /* Set the PWM state */
    hpwm->State = HAL_PWM_STATE_BUSY;

    /* Init the base time for the PWM */
    PWM_SetConfig(hpwm->Instance, &hpwm->Init);

    /* Initialize the PWM state*/
    hpwm->State = HAL_PWM_STATE_READY;

    /* Process UnLocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief Configures the PWM channel to the specified parameters in the
 * PWM_OC_InitTypeDef. After using HAL_PWM_Start() or HAL_PWM_Start_IT(), you
 * need to call HAL_PWM_Update() to make the configuration take effect.
 *
 * @param hpwm: PWM handle.
 * @param sConfig: Pointer to a PWM_OC_InitTypeDef structure that contains the
 *                 configuration information for the specified PWM channel.
 * @param Channel: PWM channel to be configured.
 *                 This parameter can be one of @ref PWM_Channel.
 * @return HAL status.
 */
HAL_StatusTypeDef HAL_PWM_ConfigChannel(PWM_HandleTypeDef *hpwm, PWM_OC_InitTypeDef *sConfig, uint32_t Channel)
{
    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));
    assert_param(IS_PWM_CHANNELS(Channel));
    assert_param(IS_PWM_OC_MODE(sConfig->OCMode));
    assert_param(IS_PWM_OC_POLARITY(sConfig->OCPolarity));

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Set the channel state */
    PWM_CHANNEL_STATE_SET(hpwm, Channel, HAL_PWM_CHANNEL_STATE_BUSY);

    /* Set the channel register */
    PWM_SetChannelConfig(hpwm->Instance, sConfig, Channel);

    /* Set the channel state */
    PWM_CHANNEL_STATE_SET(hpwm, Channel, HAL_PWM_CHANNEL_STATE_READY);

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief This function initializes the PWM interrupt settings and connects the PWM
 * interrupt request (IRQ) handler to the Nested Vectored Interrupt Controller
 * (NVIC). It sets up the NVIC to handle PWM interrupts with the specified
 * priority and enables the interrupt.
 *
 * @param hpwm: PWM handle.
 * @return HAL status.
 */
HAL_StatusTypeDef HAL_PWM_IRQInit(PWM_HandleTypeDef *hpwm)
{
    /* Check the pwm handle allocation */
    if (hpwm == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));

    /* Check the State */
    if (hpwm->State != HAL_PWM_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Connect the NVIC */
    if (hpwm->Instance == PWM0)
    {
        HAL_NVIC_ConnectIRQ(PWM_0_IRQn, PWM0_IRQ_PRIO, PWM0_IRQ_SUB_PRIO, (void *)HAL_PWM_IRQHandler, (void *)hpwm, 0);
    }
    else if (hpwm->Instance == PWM1)
    {
        HAL_NVIC_ConnectIRQ(PWM_1_IRQn, PWM1_IRQ_PRIO, PWM1_IRQ_SUB_PRIO, (void *)HAL_PWM_IRQHandler, (void *)hpwm, 0);
    }
    else if (hpwm->Instance == PWM2)
    {
        HAL_NVIC_ConnectIRQ(PWM_2_IRQn, PWM2_IRQ_PRIO, PWM2_IRQ_SUB_PRIO, (void *)HAL_PWM_IRQHandler, (void *)hpwm, 0);
    }
    else if (hpwm->Instance == PWM3)
    {
        HAL_NVIC_ConnectIRQ(PWM_3_IRQn, PWM3_IRQ_PRIO, PWM3_IRQ_SUB_PRIO, (void *)HAL_PWM_IRQHandler, (void *)hpwm, 0);
    }

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief Register callbacks.
 *
 * @param hpwm: PWM handle.
 * @param CallbackID: User Callback identifier. This parameter can be one of
 *                    @ref HAL_PWM_CallbackIDTypeDef.
 * @param pCallback: Pointer to user callbacsk function.
 * @return HAL status.
 */
HAL_StatusTypeDef HAL_PWM_RegisterCallback(PWM_HandleTypeDef *hpwm, HAL_PWM_CallbackIDTypeDef CallbackID, void (*pCallback)(PWM_HandleTypeDef *_hpwm))
{
    /* Check the callback function pointer allocation */
    if (pCallback == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));
    assert_param(IS_PWM_IRQCB_ID(CallbackID));

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Set the user callback function pointer */
    switch (CallbackID)
    {
        case HAL_PWM_EVENT_CB_ID:
            hpwm->IRQHandleCallback = pCallback;
            break;

        default:
            break;
    }

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief UnRegister callbacks.
 *
 * @param hpwm: PWM handle.
 * @param CallbackID: User Callback identifier. This parameter can be one of
 *                    @ref HAL_PWM_CallbackIDTypeDef.
 * @return HAL status.
 */
HAL_StatusTypeDef HAL_PWM_UnRegisterCallback(PWM_HandleTypeDef *hpwm, HAL_PWM_CallbackIDTypeDef CallbackID)
{
    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Reset the user callback function pointer */
    switch (CallbackID)
    {
        case HAL_PWM_EVENT_CB_ID:
            hpwm->IRQHandleCallback = NULL;
            break;

        default:
            return HAL_ERROR;
    }

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief Starts the PWM signal generation.
 *
 * @param hpwm: PWM handle.
 * @param Channel: PWM channel to be enabled.
 *                 This parameter can be one of @ref PWM_Channel.
 * @return HAL status.
 */
HAL_StatusTypeDef HAL_PWM_Start(PWM_HandleTypeDef *hpwm, uint32_t Channel)
{
    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));
    assert_param(IS_PWM_CHANNELS(Channel));

    /* Check the PWM state */
    if (hpwm->State != HAL_PWM_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Enable the channel  */
    __HAL_PWM_OUTPUT_ENABLE(hpwm, Channel);

    /* Enable the Peripheral */
    __HAL_PWM_OUTPUT_START(hpwm);

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief Starts the PWM signal generation in interrupt mode.
 *
 * @param hpwm: PWM handle.
 * @param Channel: PWM channel to be enabled.
 *                 This parameter can be one of @ref PWM_Channel.
 * @return HAL status.
 * @return
 */
HAL_StatusTypeDef HAL_PWM_Start_IT(PWM_HandleTypeDef *hpwm, uint32_t Channel)
{
    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));
    assert_param(IS_PWM_CHANNELS(Channel));

    /* Check the PWM state */
    if (hpwm->State != HAL_PWM_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Enable the interrupt */
    if (hpwm->IRQHandleCallback)
    {

        switch ((uint32_t)hpwm->Instance)
        {
            case (uint32_t)PWM0:
                HAL_NVIC_EnableIRQ(PWM_0_IRQn);
                break;
            case (uint32_t)PWM1:
                HAL_NVIC_EnableIRQ(PWM_1_IRQn);
                break;
            case (uint32_t)PWM2:
                HAL_NVIC_EnableIRQ(PWM_2_IRQn);
                break;
            case (uint32_t)PWM3:
                HAL_NVIC_EnableIRQ(PWM_3_IRQn);
                break;

            default:
                break;
        }
    }

    /* Enable the channel */
    __HAL_PWM_OUTPUT_ENABLE(hpwm, Channel);

    /* Enable the Peripheral */
    __HAL_PWM_OUTPUT_START(hpwm);

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief Stops the PWM signal generation.
 *
 * @param hpwm: PWM handle.
 * @param Channel: PWM channel to be disabled.
 *                 This parameter can be one of @ref PWM_Channel.
 * @return
 */
HAL_StatusTypeDef HAL_PWM_Stop(PWM_HandleTypeDef *hpwm, uint32_t Channel)
{
    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));
    assert_param(IS_PWM_CHANNELS(Channel));

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Disable the channel  */
    __HAL_PWM_OUTPUT_DISABLE(hpwm, Channel);

    /* Disable the Peripheral */
    __HAL_PWM_OUTPUT_STOP(hpwm);

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief Stops the PWM signal generation in interrupt mode.
 *
 * @param hpwm: PWM handle.
 * @param Channel: PWM channel to be disabled.
 *                 This parameter can be one of @ref PWM_Channel.
 * @return
 */
HAL_StatusTypeDef HAL_PWM_Stop_IT(PWM_HandleTypeDef *hpwm, uint32_t Channel)
{
    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));
    assert_param(IS_PWM_CHANNELS(Channel));

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Disable the interrupt */
    switch ((uint32_t)hpwm->Instance)
    {
        case (uint32_t)PWM0:
            HAL_NVIC_DisableIRQ(PWM_0_IRQn);
            break;
        case (uint32_t)PWM1:
            HAL_NVIC_DisableIRQ(PWM_1_IRQn);
            break;
        case (uint32_t)PWM2:
            HAL_NVIC_DisableIRQ(PWM_2_IRQn);
            break;
        case (uint32_t)PWM3:
            HAL_NVIC_DisableIRQ(PWM_3_IRQn);
            break;

        default:
            break;
    }

    /* Disable the channel  */
    __HAL_PWM_OUTPUT_DISABLE(hpwm, Channel);

    /* Disable the peripheral */
    __HAL_PWM_OUTPUT_STOP(hpwm);

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return function status */
    return HAL_OK;
}

/**
 * @brief Gets the PWM timer counter value.
 *
 * @param hpwm: PWM handle.
 * @return Counter value.
 */
uint32_t HAL_PWM_GetCounter(PWM_HandleTypeDef *hpwm)
{
    uint32_t counter;

    /* Check the parameters */
    assert_param(IS_PWM_INSTANCE(hpwm->Instance));

    /* Process Locked */
    HAL_PWM_LOCK();

    /* Get the PWM counter */
    counter = hpwm->Instance->COUNTER;

    /* Process Unlocked */
    HAL_PWM_UNLOCK();

    /* Return PWM counter */
    return counter;
}

#endif /* HAL_PWM_MODULE_ENABLED */
