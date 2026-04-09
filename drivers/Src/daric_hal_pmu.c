/**
 ******************************************************************************
 * @file    daric_hal_pmu.c
 * @author  PMU Team
 * @brief   PMU HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the Power Management Unit (PMU) peripheral:
 *           + Initialization and de-initialization
 *           + Power mode control functions
 *           + Voltage regulation functions
 *           + Status Monitoring functions
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

#ifdef HAL_PMU_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define PMU_LDO_STABILIZATION_DELAY 10 /* 10ms delay for LDO stabilization */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t s_current_mode = PMU_MODE_ACTIVE;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

static void PMU_SetConfig(PMU_TypeDef *Instance, PMU_InitTypeDef *Init)
{
    Instance->CR   = Init->LDO_Config;
    Instance->CRLP = Init->LowPowerConfig;
    Instance->CRPD = Init->PowerDownConfig;
}

static void PMU_ConfigLDO(PMU_TypeDef *Instance, uint32_t Regulator, uint32_t Setting)
{
    switch (Regulator)
    {
        case PMU_VDDAO_VOLTAGE_CFG:
            Instance->TRM0 &= ~PMU_TRM_VDDAO_Msk;
            Instance->TRM0 |= Setting << PMU_TRM_VDDAO_Pos;
            break;
        case PMU_TRM_VDD85D:
            Instance->TRM0 &= ~PMU_TRM_DIG_REF_Msk;
            Instance->TRM0 |= Setting << PMU_TRM_DIG_REF_Pos;
            break;
        case PMU_TRM_VDD85A:
            Instance->TRM0 &= ~PMU_TRM_ANA_REF_Msk;
            Instance->TRM0 |= Setting << PMU_TRM_ANA_REF_Pos;
            break;
        case PMU_TRM_VDD25:
            Instance->TRM0 &= ~PMU_TRM_25V_REF_Msk;
            Instance->TRM0 |= Setting << PMU_TRM_25V_REF_Pos;
            break;
        case PMU_TRM_CTAT:
            Instance->TRM0 &= ~PMU_TRM_BG_CTAT_Msk;
            Instance->TRM0 |= Setting << PMU_TRM_BG_CTAT_Pos;
            break;
        case PMU_TRM_PTAT:
            Instance->TRM0 &= ~PMU_TRM_BG_PTAT_Msk;
            Instance->TRM0 |= Setting << PMU_TRM_BG_PTAT_Pos;
            break;
        case PMU_TRM_CUR:
            Instance->TRM0 &= ~PMU_TRM_IBIAS_LSB_Msk;
            Instance->TRM0 |= Setting << PMU_TRM_IBIAS_LSB_Pos;
            Instance->TRM1 &= ~PMU_TRM_IBIAS_MSB_Msk;
            Instance->TRM1 |= (Setting >> PMU_TRM_IBIAS_LSB_Pos) << PMU_TRM_IBIAS_MSB_Pos;
            break;

        default:
            break;
    }
}

static void PMU_LP_ConfigLDO(PMU_TypeDef *Instance, uint32_t Regulator, uint32_t Setting)
{
    switch (Regulator)
    {
        case PMU_VDDAO_VOLTAGE_CFG:
            Instance->TRMLP0 &= ~PMU_TRM_VDDAO_Msk;
            Instance->TRMLP0 |= Setting << PMU_TRM_VDDAO_Pos;
            break;
        case PMU_TRM_VDD85D:
            Instance->TRMLP0 &= ~PMU_TRM_DIG_REF_Msk;
            Instance->TRMLP0 |= Setting << PMU_TRM_DIG_REF_Pos;
            break;
        case PMU_TRM_VDD85A:
            Instance->TRMLP0 &= ~PMU_TRM_ANA_REF_Msk;
            Instance->TRMLP0 |= Setting << PMU_TRM_ANA_REF_Pos;
            break;
        case PMU_TRM_VDD25:
            Instance->TRMLP0 &= ~PMU_TRM_25V_REF_Msk;
            Instance->TRMLP0 |= Setting << PMU_TRM_25V_REF_Pos;
            break;
        case PMU_TRM_CTAT:
            Instance->TRMLP0 &= ~PMU_TRM_BG_CTAT_Msk;
            Instance->TRMLP0 |= Setting << PMU_TRM_BG_CTAT_Pos;
            break;
        case PMU_TRM_PTAT:
            Instance->TRMLP0 &= ~PMU_TRM_BG_PTAT_Msk;
            Instance->TRMLP0 |= Setting << PMU_TRM_BG_PTAT_Pos;
            break;
        case PMU_TRM_CUR:
            Instance->TRMLP0 &= ~PMU_TRM_IBIAS_LSB_Msk;
            Instance->TRMLP0 |= Setting << PMU_TRM_IBIAS_LSB_Pos;
            Instance->TRMLP1 &= ~PMU_TRM_IBIAS_MSB_Msk;
            Instance->TRMLP1 |= (Setting >> PMU_TRM_IBIAS_LSB_Pos) << PMU_TRM_IBIAS_MSB_Pos;
            break;

        default:
            break;
    }
}

/**
 * @}
 */

/* Exported functions --------------------------------------------------------*/

/** @addtogroup PMU_Exported_Functions
 * @{
 */

/** @addtogroup PMU_Exported_Functions_Group1
 * @{
 */

/**
 * @brief  Initialize the PMU according to the specified parameters.
 * @param  hpmu: Pointer to a PMU_HandleTypeDef structure.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_PMU_Init(PMU_HandleTypeDef *hpmu)
{
    /* Check PMU handle */
    if (hpmu == NULL)
    {
        return HAL_ERROR;
    }

    /* Check parameters */
    assert_param(IS_PMU_POWER_MODE(hpmu->Init.PowerMode));
    assert_param(IS_PMU_LDO_TYPE(hpmu->Init.LDO_Config));

    /* Allocate lock resource */
    HAL_PMU_LOCK();

    /* Initialize low level hardware */
    if (hpmu->MspInitCallback)
    {
        hpmu->MspInitCallback(hpmu);
    }

    /* Change PMU peripheral state */
    hpmu->State = HAL_PMU_STATE_BUSY;

    /* Apply default configuration */
    PMU_SetConfig(hpmu->Instance, &hpmu->Init);

    /* Set default power mode */
    hpmu->State = HAL_PMU_STATE_READY;

    /* Release Lock */
    HAL_PMU_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  DeInitialize the PMU peripheral.
 * @param  hpmu: Pointer to a PMU_HandleTypeDef structure.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_PMU_DeInit(PMU_HandleTypeDef *hpmu)
{
    /* Check PMU handle */
    if (hpmu == NULL)
    {
        return HAL_ERROR;
    }

    /* Allocate lock resource */
    HAL_PMU_LOCK();

    /* Change PMU peripheral state */
    hpmu->State = HAL_PMU_STATE_BUSY;

    /* De-initialize low level hardware */
    if (hpmu->MspDeInitCallback)
    {
        hpmu->MspDeInitCallback(hpmu);
    }

    /* Reset PMU register */
    __HAL_PMU_RESET_REGISTER(hpmu);

    /* Reset PMU state */
    hpmu->State = HAL_PMU_STATE_RESET;

    /* Release Lock */
    HAL_PMU_UNLOCK();

    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup PMU_Exported_Functions_Group2
 * @{
 */

/**
 * @brief  Configure the specified LDO voltage level.
 * @param  hpmu: Pointer to a PMU_HandleTypeDef structure.
 * @param  LDO: Specifies the LDO to configure.
 *         This parameter can be one of the following values:
 *           @arg PMU_VDDAO_VOLTAGE_CFG
 *           @arg PMU_TRM_VDD85D
 *           @arg PMU_TRM_VDD85A
 *           @arg PMU_TRM_VDD25
 *           @arg PMU_TRM_CTAT
 *           @arg PMU_TRM_PTAT
 *           @arg PMU_TRM_CUR
 * @param  Voltage: Specifies the voltage level.
 *         This parameter can be one of the following values:
 *           @arg 0.8 (V)
 *           @arg 0.95 (V)
 *           @arg 2.5 (V)
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_PMU_ConfigLDO(PMU_HandleTypeDef *hpmu, uint32_t PowerMode, uint32_t LDO, uint32_t Voltage)
{
    /* Check parameters */
    assert_param(IS_PMU_POWER_MODE(PowerMode));
    assert_param(IS_PMU_LDO_TYPE(LDO));
    assert_param(IS_PMU_REGULATOR_VOLTAGE(Voltage));

    /* Check PMU handle state */
    if (hpmu->State != HAL_PMU_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Check PMU current mode */
    if (s_current_mode == PowerMode)
    {
        return HAL_BUSY;
    }

    /* Allocate lock resource */
    HAL_PMU_LOCK();

    /* Change PMU peripheral state */
    hpmu->State = HAL_PMU_STATE_BUSY;

    /* Configure LDO voltage */
    switch (PowerMode)
    {
        case PMU_MODE_ACTIVE:
            PMU_ConfigLDO(hpmu->Instance, LDO, Voltage);
            break;
        case PMU_MODE_SLEEP:
        case PMU_MODE_DEEP_SLEEP:
        case PMU_MODE_POWER_DOWN:
            PMU_LP_ConfigLDO(hpmu->Instance, LDO, Voltage);
            break;
    }

    /* Reset PMU state */
    hpmu->State = HAL_PMU_STATE_READY;

    /* Release Lock */
    HAL_PMU_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Enable or disable the specified LDO.
 * @param  hpmu: Pointer to a PMU_HandleTypeDef structure.
 * @param  LDO: Specifies the LDO to control.
 *         This parameter can be one of the following values:
 *           @arg PMU_POC
 *           @arg PMU_IOUT
 *           @arg PMU_LDO_VDD85D
 *           @arg PMU_LDO_VDD85A
 *           @arg PMU_LDO_VDD25
 * @param  State: New state of the LDO.
 *         This parameter can be: ENABLED or DISABLED.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_PMU_ControlLDO(PMU_HandleTypeDef *hpmu, uint32_t PowerMode, uint32_t LDO, uint32_t State)
{
    /* Check parameters */
    assert_param(IS_PMU_LDO_TYPE(LDO));
    assert_param(IS_FUNCTIONAL_STATE(State));

    /* Check PMU handle state */
    if (hpmu->State != HAL_PMU_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Check PMU current mode */
    if (s_current_mode == PowerMode)
    {
        return HAL_BUSY;
    }

    /* Allocate lock resource */
    HAL_PMU_LOCK();

    /* Change PMU peripheral state */
    hpmu->State = HAL_PMU_STATE_BUSY;

    /* Configure LDO state */
    if (State == 1)
    {
        /* Enable the power  */
        switch (PowerMode)
        {
            case PMU_MODE_ACTIVE:
                __HAL_PMU_POWER_ENABLE(hpmu, LDO);
                break;
            case PMU_MODE_SLEEP:
            case PMU_MODE_DEEP_SLEEP:
                __HAL_PMU_LP_POWER_ENABLE(hpmu, LDO);
                break;
            case PMU_MODE_POWER_DOWN:
                __HAL_PMU_PD_POWER_ENABLE(hpmu, LDO);
                break;
        }
    }
    else
    {
        /* Disable the power  */
        switch (PowerMode)
        {
            case PMU_MODE_ACTIVE:
                __HAL_PMU_POWER_DISABLE(hpmu, LDO);
                break;
            case PMU_MODE_SLEEP:
            case PMU_MODE_DEEP_SLEEP:
                __HAL_PMU_LP_POWER_DISABLE(hpmu, LDO);
                break;
            case PMU_MODE_POWER_DOWN:
                __HAL_PMU_PD_POWER_DISABLE(hpmu, LDO);
                break;
        }
    }

    /* Reset PMU state */
    hpmu->State = HAL_PMU_STATE_READY;

    /* Release Lock */
    HAL_PMU_UNLOCK();

    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup PMU_Exported_Functions_Group3
 * @{
 */

/**
 * @brief  Enter the specified low power mode.
 * @param  hpmu: Pointer to a PMU_HandleTypeDef structure.
 * @param  Mode: Specifies the low power mode to enter.
 *         This parameter can be one of the following values:
 *           @arg PMU_MODE_SLEEP
 *           @arg PMU_MODE_DEEP_SLEEP
 *           @arg PMU_MODE_POWER_DOWN
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_PMU_EnterLowPowerMode(PMU_HandleTypeDef *hpmu, uint32_t Mode)
{
    /* Check parameters */
    assert_param(IS_PMU_POWER_MODE(Mode));

    /* Check PMU handle state */
    if (hpmu->State != HAL_PMU_STATE_READY)
    {
        return HAL_ERROR;
    }

    /* Allocate lock resource */
    HAL_PMU_LOCK();

    /* Set PMU state to busy */
    hpmu->State = HAL_PMU_STATE_BUSY;

    /* Enter requested low power mode */
    switch (Mode)
    {
        case PMU_MODE_SLEEP:
        case PMU_MODE_DEEP_SLEEP:
            s_current_mode = Mode;
            /* Execute WFI/WFE instruction (to be implemented in user application) */
            break;

        case PMU_MODE_POWER_DOWN:
            s_current_mode = Mode;
            /* Write specific value to enter power-down mode */
            __HAL_PMU_ENTER_PD_MODE(hpmu);
            break;

        default:
            hpmu->State = HAL_PMU_STATE_READY;
            /* Release Lock */
            HAL_PMU_UNLOCK();
            return HAL_ERROR;
    }

    hpmu->State = HAL_PMU_STATE_READY;

    /* Release Lock */
    HAL_PMU_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Exit low power mode.
 * @param  hpmu: Pointer to a PMU_HandleTypeDef structure.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_PMU_ExitLowPowerMode(PMU_HandleTypeDef *hpmu)
{
    /* Check PMU handle state */
    if (hpmu->State != HAL_PMU_STATE_READY)
    {
        return HAL_ERROR;
    }
    /* Allocate lock resource */
    HAL_PMU_LOCK();

    /* Restore default configuration */
    __HAL_PMU_EXIT_PD_MODE(hpmu);

    s_current_mode = PMU_MODE_ACTIVE;

    /* Set PMU state to ready */
    hpmu->State = HAL_PMU_STATE_READY;

    /* Release Lock */
    HAL_PMU_UNLOCK();

    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup PMU_Exported_Functions_Group4
 * @{
 */

/**
 * @brief  Get the PMU status.
 * @param  hpmu: Pointer to a PMU_HandleTypeDef structure.
 * @param  StatusType: Specifies the status type to read.
 * @retval Status value
 */
uint32_t HAL_PMU_GetStatus(PMU_HandleTypeDef *hpmu, uint32_t StatusType)
{
    /* Check parameters */
    assert_param(IS_PMU_STATUS_TYPE(StatusType));

    return __HAL_PMU_LDO_IS_STABLE(hpmu, StatusType);
}

/**
 * @}
 */

#endif /* HAL_PMU_MODULE_ENABLED */