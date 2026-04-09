/**
 ******************************************************************************
 * @file    daric_hal_pmu.h
 * @author  PMU Team
 * @brief   Header file of PMU HAL module.
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
#ifndef DARIC_HAL_PMU_H
#define DARIC_HAL_PMU_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include <stdbool.h>

/** @addtogroup PMU_Exported_Types
 * @{
 */

/* Peripheral memory map */
#define PMU_BASE (0x40060000UL) /*!< PMU base address */
#define PMU      ((PMU_TypeDef *)PMU_BASE)

    /**
     * @brief  PMU Register structure definition
     */
    typedef struct
    {
        const uint32_t RESERVED0[4]; /*!<  32KHz clock source selection register,          Address offset: 0x0000. */
                                     /*!<  1Hz frequency division configuration register,  Address offset: 0x0004. */
                                     /*!<  Wakeup mask and interrupt enable register,      Address offset: 0x0008. */
                                     /*!<  Reset mask control register,                    Address offset: 0x000C. */
        __IO uint32_t  CR;           /*!< PMU block control register,                  Address offset: 0x0010 */
        __IO uint32_t  CRLP;         /*!< PMU block control register for low power,    Address offset: 0x0014 */
        __IO uint32_t  CRPD;         /*!< PMU block control register for power down,   Address offset: 0x0018 */
        __IO uint32_t  DFT;          /*!< PMU default test register,                   Address offset: 0x001C */
        __IO uint32_t  TRM0;         /*!< PMU trimming configuration low 32-bit,       Address offset: 0x0020 */
        __IO uint32_t  TRM1;         /*!< PMU trimming configuration high 32-bit,      Address offset: 0x0024 */
        __IO uint32_t  TRMLP0;       /*!< PMU trims value0 for low power,              Address offset: 0x0028 */
        __IO uint32_t  TRMLP1;       /*!< PMU trims value1 for low power,              Address offset: 0x002C */
        const uint32_t RESERVED1[2]; /*!< Reserved,                                    Address offset: 0x0030 */
                                     /*!< 32KHz oscillator control register,           Address offset: 0x0034 */
        __IO uint32_t  SR;           /*!< PMU Ready Status register,                   Address offset: 0x0038 */
        __IO uint32_t  FR;           /*!< PMU Error Flag register,                     Address offset: 0x003C */
        const uint32_t RESERVED2[1]; /*!< AO domain interrupt flag register,           Address offset: 0x0040 */
        __IO uint32_t  PDAR;         /*!< PMU power down mode AR register,             Address offset: 0x0044 */
    } PMU_TypeDef;

/** @defgroup PMU_Register_Bits PMU Register Bits
 * @{
 */

/****************** Bits definition for CR register ********************/
#define PMU_CR_VDD85D_VOL_SEL_Pos (0U)
#define PMU_CR_VDD85D_VOL_SEL_Msk (0x1UL << PMU_CR_VDD85D_VOL_SEL_Pos)
#define PMU_CR_VDD85D_VOL_SEL     PMU_CR_VDD85D_VOL_SEL_Msk

#define PMU_CR_VDD85A_VOL_SEL_Pos (1U)
#define PMU_CR_VDD85A_VOL_SEL_Msk (0x1UL << PMU_CR_VDD85A_VOL_SEL_Pos)
#define PMU_CR_VDD85A_VOL_SEL     PMU_CR_VDD85A_VOL_SEL_Msk

#define PMU_CR_POC_EN_Pos (2U)
#define PMU_CR_POC_EN_Msk (0x1UL << PMU_CR_POC_EN_Pos)
#define PMU_CR_POC_EN     PMU_CR_POC_EN_Msk

#define PMU_CR_IOUT_EN_Pos (3U)
#define PMU_CR_IOUT_EN_Msk (0x1UL << PMU_CR_IOUT_EN_Pos)
#define PMU_CR_IOUT_EN     PMU_CR_IOUT_EN_Msk

#define PMU_CR_VDD85D_REG_EN_Pos (4U)
#define PMU_CR_VDD85D_REG_EN_Msk (0x1UL << PMU_CR_VDD85D_REG_EN_Pos)
#define PMU_CR_VDD85D_REG_EN     PMU_CR_VDD85D_REG_EN_Msk

#define PMU_CR_VDD85A_REG_EN_Pos (5U)
#define PMU_CR_VDD85A_REG_EN_Msk (0x1UL << PMU_CR_VDD85A_REG_EN_Pos)
#define PMU_CR_VDD85A_REG_EN     PMU_CR_VDD85A_REG_EN_Msk

#define PMU_CR_VDD25_REG_EN_Pos (6U)
#define PMU_CR_VDD25_REG_EN_Msk (0x1UL << PMU_CR_VDD25_REG_EN_Pos)
#define PMU_CR_VDD25_REG_EN     PMU_CR_VDD25_REG_EN_Msk

/****************** Bits definition for DFT register *******************/
#define PMU_DFT_TEST_MODE_Pos (0U)
#define PMU_DFT_TEST_MODE_Msk (0x7UL << PMU_DFT_TEST_MODE_Pos)
#define PMU_DFT_TEST_MODE     PMU_DFT_TEST_MODE_Msk

#define PMU_DFT_TEST_SEL_Pos (3U)
#define PMU_DFT_TEST_SEL_Msk (0x7UL << PMU_DFT_TEST_SEL_Pos)
#define PMU_DFT_TEST_SEL     PMU_DFT_TEST_SEL_Msk

/****************** Bits definition for TRM0/TRMLP0 register ****/
#define PMU_TRM_VDDAO_Pos (0U)
#define PMU_TRM_VDDAO_Msk (0x7UL << PMU_TRM_VDDAO_Pos)
#define PMU_TRM_VDDAO     PMU_TRM_VDDAO_Msk ///< Bits[2:0]: VDDAO trim

#define PMU_TRM_DIG_REF_Pos (3U)
#define PMU_TRM_DIG_REF_Msk (0x1FUL << PMU_TRM_DIG_REF_Pos)
#define PMU_TRM_DIG_REF     PMU_TRM_DIG_REF_Msk ///< Bits[7:3]: digital reg ref

#define PMU_TRM_ANA_REF_Pos (8U)
#define PMU_TRM_ANA_REF_Msk (0x1FUL << PMU_TRM_ANA_REF_Pos)
#define PMU_TRM_ANA_REF     PMU_TRM_ANA_REF_Msk ///< Bits[12:8]: analog reg ref

#define PMU_TRM_25V_REF_Pos (13U)
#define PMU_TRM_25V_REF_Msk (0x1FUL << PMU_TRM_25V_REF_Pos)
#define PMU_TRM_25V_REF     PMU_TRM_25V_REF_Msk ///< Bits[17:13]: 2.5V reg ref

#define PMU_TRM_BG_CTAT_Pos (18U)
#define PMU_TRM_BG_CTAT_Msk (0x1FUL << PMU_TRM_BG_CTAT_Pos)
#define PMU_TRM_BG_CTAT     PMU_TRM_BG_CTAT_Msk ///< Bits[22:18]: bandgap CTAT

#define PMU_TRM_BG_PTAT_Pos (23U)
#define PMU_TRM_BG_PTAT_Msk (0x1FUL << PMU_TRM_BG_PTAT_Pos)
#define PMU_TRM_BG_PTAT     PMU_TRM_BG_PTAT_Msk ///< Bits[27:23]: bandgap PTAT

#define PMU_TRM_IBIAS_LSB_Pos (28U)
#define PMU_TRM_IBIAS_LSB_Msk (0xFUL << PMU_TRM_IBIAS_LSB_Pos)
#define PMU_TRM_IBIAS_LSB     PMU_TRM_IBIAS_LSB_Msk ///< Bits[31:28]: ibias trimming bits[3:0]

/****************** Bits definition for TRM1/TRMLP1 register ****/
#define PMU_TRM_IBIAS_MSB_Pos (0U)
#define PMU_TRM_IBIAS_MSB_Msk (0x3UL << PMU_TRM_IBIAS_MSB_Pos)
#define PMU_TRM_IBIAS_MSB     PMU_TRM_IBIAS_MSB_Msk ///< Bits[1:0]: ibias trimming bits[5:4]

/****************** Bits definition for SR register ********************/
#define PMU_SR_POR_RDY_Pos (0U)
#define PMU_SR_POR_RDY_Msk (0x1UL << PMU_SR_POR_RDY_Pos)
#define PMU_SR_POR_RDY     PMU_SR_POR_RDY_Msk

#define PMU_SR_VDD85D_REG_RDY_Pos (1U)
#define PMU_SR_VDD85D_REG_RDY_Msk (0x1UL << PMU_SR_VDD85D_REG_RDY_Pos)
#define PMU_SR_VDD85D_REG_RDY     PMU_SR_VDD85D_REG_RDY_Msk

#define PMU_SR_VDD85A_REG_RDY_Pos (2U)
#define PMU_SR_VDD85A_REG_RDY_Msk (0x1UL << PMU_SR_VDD85A_REG_RDY_Pos)
#define PMU_SR_VDD85A_REG_RDY     PMU_SR_VDD85A_REG_RDY_Msk

#define PMU_SR_VDD25_REG_RDY_Pos (3U)
#define PMU_SR_VDD25_REG_RDY_Msk (0x1UL << PMU_SR_VDD25_REG_RDY_Pos)
#define PMU_SR_VDD25_REG_RDY     PMU_SR_VDD25_REG_RDY_Msk

#define PMU_SR_BANDGAP_RDY_Pos (4U)
#define PMU_SR_BANDGAP_RDY_Msk (0x1UL << PMU_SR_BANDGAP_RDY_Pos)
#define PMU_SR_BANDGAP_RDY     PMU_SR_BANDGAP_RDY_Msk

/****************** Bits definition for FR register ********************/
#define PMU_FR_POR_ERR_Pos (0U)
#define PMU_FR_POR_ERR_Msk (0x1UL << PMU_FR_POR_ERR_Pos)
#define PMU_FR_POR_ERR     PMU_FR_POR_ERR_Msk

#define PMU_FR_VDD85D_REG_ERR_Pos (1U)
#define PMU_FR_VDD85D_REG_ERR_Msk (0x1UL << PMU_FR_VDD85D_REG_ERR_Pos)
#define PMU_FR_VDD85D_REG_ERR     PMU_FR_VDD85D_REG_ERR_Msk

#define PMU_FR_VDD85A_REG_ERR_Pos (2U)
#define PMU_FR_VDD85A_REG_ERR_Msk (0x1UL << PMU_FR_VDD85A_REG_ERR_Pos)
#define PMU_FR_VDD85A_REG_ERR     PMU_FR_VDD85A_REG_ERR_Msk

#define PMU_FR_VDD25_ERR_Pos (3U)
#define PMU_FR_VDD25_ERR_Msk (0x1UL << PMU_FR_VDD25_ERR_Pos)
#define PMU_FR_VDD25_ERR     PMU_FR_VDD25_ERR_Msk

#define PMU_FR_BANDGAP_ERR_Pos (4U)
#define PMU_FR_BANDGAP_ERR_Msk (0x1UL << PMU_FR_BANDGAP_ERR_Pos)
#define PMU_FR_BANDGAP_ERR     PMU_FR_BANDGAP_ERR_Msk

/****************** Bits definition for PDAR register ******************/
#define PMU_PDAR_PD_VALUE (0x0000005AU) /*!< Value to write to enter power down mode */

    /**
     * @}
     */

    /**
     * @brief  PMU Init structure definition
     */
    typedef struct
    {
        uint32_t PowerMode;       /*!< Default power mode (ACTIVE, SLEEP, DEEP_SLEEP, POWER_DOWN) */
        uint32_t LDO_Config;      /*!< LDO configuration flags */
        uint32_t LowPowerConfig;  /*!< Low power mode configuration */
        uint32_t PowerDownConfig; /*!< Power down mode configuration */
    } PMU_InitTypeDef;

    /**
     * @brief  HAL PMU State structure definition
     */
    typedef enum
    {
        HAL_PMU_STATE_RESET = 0x00U, /*!< PMU not yet initialized or disabled */
        HAL_PMU_STATE_READY = 0x01U, /*!< PMU initialized and ready for use   */
        HAL_PMU_STATE_BUSY  = 0x02U, /*!< PMU process is ongoing              */
        HAL_PMU_STATE_ERROR = 0x03U  /*!< PMU error state                     */
    } HAL_PMU_StateTypeDef;

    /**
     * @brief  PMU handle Structure definition
     */
    typedef struct __PMU_HandleTypeDef
    {
        PMU_TypeDef         *Instance;                               /*!< Register base address */
        PMU_InitTypeDef      Init;                                   /*!< PMU initialization parameters */
        HAL_PMU_StateTypeDef State;                                  /*!< PMU state */
        void (*MspInitCallback)(struct __PMU_HandleTypeDef *hpmu);   /*!< MSP init callback */
        void (*MspDeInitCallback)(struct __PMU_HandleTypeDef *hpmu); /*!< MSP de-init callback */
    } PMU_HandleTypeDef;

/**
 * @}
 */

/** @defgroup PMU_Exported_Constants PMU Exported Constants
 * @{
 */

/** @defgroup PMU_Power_Mode PMU Power Mode
 * @{
 */
#define PMU_MODE_ACTIVE     (0x00000000U) /*!< Active mode */
#define PMU_MODE_SLEEP      (0x00000001U) /*!< Sleep mode */
#define PMU_MODE_DEEP_SLEEP (0x00000002U) /*!< Deep sleep mode */
#define PMU_MODE_POWER_DOWN (0x00000003U) /*!< Power down mode */
/**
 * @}
 */

/** @defgroup PMU_Regulator_Voltage PMU Regulator
 * @{
 */
#define PMU_VDDAO_VOLTAGE_CFG (0x00000000U)
#define PMU_TRM_VDD85D        (0x00000001U)
#define PMU_TRM_VDD85A        (0x00000002U)
#define PMU_TRM_VDD25         (0x00000004U)
#define PMU_TRM_CTAT          (0x00000008U)
#define PMU_TRM_PTAT          (0x00000010U)
#define PMU_TRM_CUR           (0x00000020U)
/**
 * @}
 */

/** @defgroup PMU_Regulator_Voltage PMU Regulator Voltage
 * @{
 */
/* PMU_VDDAO_VOLTAGE_CFG */
#define PMU_TRM_VDDAO_VOLTAGE_0_80V (0U)
#define PMU_TRM_VDDAO_VOLTAGE_0_75V (1U)
#define PMU_TRM_VDDAO_VOLTAGE_0_70V (2U)
#define PMU_TRM_VDDAO_VOLTAGE_0_65V (3U)
#define PMU_TRM_VDDAO_VOLTAGE_0_60V (4U)
/* PMU_TRM_VDD85D/PMU_TRM_VDD85A/PMU_TRM_CTAT/PMU_TRM_PTAT */
#define PMU_TRM_VOLTAGE_0_80V (10000U)
#define PMU_TRM_VOLTAGE_0_95V (10000U)
/* PMU_TRM_VDD25 */
#define PMU_TRM_VOLTAGE_2_50V (10000U)
/* PMU_TRM_CUR */
#define PMU_TRM_CURRENT_10_002UA (100000U)
/**
 * @}
 */

/** @defgroup PMU_LDO_Type PMU LDO Type
 * @{
 */
#define PMU_POC        (PMU_CR_POC_EN_Msk)        /*!< Reram Power on control signal */
#define PMU_IOUT       (PMU_CR_IOUT_EN_Msk)       /*!< iout circuit */
#define PMU_LDO_VDD85D (PMU_CR_VDD85D_REG_EN_Msk) /*!< VDD85D LDO */
#define PMU_LDO_VDD85A (PMU_CR_VDD85A_REG_EN_Msk) /*!< VDD85A LDO */
#define PMU_LDO_VDD25  (PMU_CR_VDD25_REG_EN_Msk)  /*!< VDD25 LDO */
/**
 * @}
 */

/** @defgroup PMU_Status_Type PMU Status Type
 * @{
 */
#define PMU_STATUS_POR_READY     (0x00000001U) /*!< POR ready status */
#define PMU_STATUS_VDD85D_READY  (0x00000002U) /*!< VDD85D LDO ready status */
#define PMU_STATUS_VDD85A_READY  (0x00000004U) /*!< VDD85A LDO ready status */
#define PMU_STATUS_VDD25_READY   (0x00000008U) /*!< VDD25 LDO ready status */
#define PMU_STATUS_BANDGAP_READY (0x00000010U) /*!< Bandgap ready status */
/**
 * @}
 */

/** @defgroup PMU_Flag_Type PMU Flag Type
 * @{
 */
#define PMU_FLAG_POR_ERROR     (0x00000001U) /*!< POR error flag */
#define PMU_FLAG_VDD85D_ERROR  (0x00000002U) /*!< VDD85D LDO error flag */
#define PMU_FLAG_VDD85A_ERROR  (0x00000004U) /*!< VDD85A LDO error flag */
#define PMU_FLAG_VDD25_ERROR   (0x00000008U) /*!< VDD25 LDO error flag */
#define PMU_FLAG_BANDGAP_ERROR (0x00000010U) /*!< Bandgap error flag */
/**
 * @}
 */

/** @defgroup PMU_Callback_ID PMU Callback ID
 * @{
 */
#define HAL_PMU_WAKEUP_CB_ID    (0x01U) /*!< PMU Wakeup callback ID */
#define HAL_PMU_ERROR_CB_ID     (0x02U) /*!< PMU Error callback ID */
#define HAL_PMU_MSPINIT_CB_ID   (0x03U) /*!< PMU MSP Init callback ID */
#define HAL_PMU_MSPDEINIT_CB_ID (0x04U) /*!< PMU MSP DeInit callback ID */
    /**
     * @}
     */

    /**
     * @}
     */

    /* Exported macro ------------------------------------------------------------*/

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
    /* Initialization and de-initialization functions *****************************/
    HAL_StatusTypeDef HAL_PMU_Init(PMU_HandleTypeDef *hpmu);
    HAL_StatusTypeDef HAL_PMU_DeInit(PMU_HandleTypeDef *hpmu);
    /**
     * @}
     */

    /** @addtogroup PMU_Exported_Functions_Group2
     * @{
     */
    /* Voltage regulation functions ***********************************************/
    HAL_StatusTypeDef HAL_PMU_ConfigLDO(PMU_HandleTypeDef *hpmu, uint32_t PowerMode, uint32_t LDO, uint32_t Voltage);
    HAL_StatusTypeDef HAL_PMU_ControlLDO(PMU_HandleTypeDef *hpmu, uint32_t PowerMode, uint32_t LDO, uint32_t State);
    /**
     * @}
     */

    /** @addtogroup PMU_Exported_Functions_Group3
     * @{
     */
    /* Power mode control functions ***********************************************/
    HAL_StatusTypeDef HAL_PMU_EnterLowPowerMode(PMU_HandleTypeDef *hpmu, uint32_t Mode);
    HAL_StatusTypeDef HAL_PMU_ExitLowPowerMode(PMU_HandleTypeDef *hpmu);
    /**
     * @}
     */

    /** @addtogroup PMU_Exported_Functions_Group4
     * @{
     */
    /* Status monitoring functions ************************************************/
    uint32_t HAL_PMU_GetStatus(PMU_HandleTypeDef *hpmu, uint32_t StatusType);
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/* Private macros ------------------------------------------------------------*/
/** @defgroup PMU_Private_Macros PMU Private Macros
 * @{
 */
#define HAL_PMU_LOCK()   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_PMU_UNLOCK() HAL_UNLOCK

#define IS_PMU_POWER_MODE(MODE) (((MODE) == PMU_MODE_ACTIVE) || ((MODE) == PMU_MODE_SLEEP) || ((MODE) == PMU_MODE_DEEP_SLEEP) || ((MODE) == PMU_MODE_POWER_DOWN))

#define IS_PMU_LDO_TYPE(LDO) (((LDO) == PMU_LDO_VDD85D) || ((LDO) == PMU_LDO_VDD85A) || ((LDO) == PMU_LDO_VDD25))

#define IS_PMU_WAKEUP_SOURCE(SOURCE)                                                                                                               \
    (((SOURCE) == PMU_WKUP_WDT_RST) || ((SOURCE) == PMU_WKUP_RTC_EVENT) || ((SOURCE) == PMU_WKUP_ATIMER_EVENT) || ((SOURCE) == PMU_WKUP_WDT_EVENT) \
     || ((SOURCE) == PMU_WKUP_KPC_EVENT) || ((SOURCE) == PMU_WKUP_KPC_ASYNC) || ((SOURCE) == PMU_WKUP_PF1_PD) || ((SOURCE) == PMU_WKUP_PF0_PD))

#define IS_PMU_STATUS_TYPE(TYPE) \
    (((TYPE) == PMU_STATUS_LDO85D_READY) || ((TYPE) == PMU_STATUS_LDO85A_READY) || ((TYPE) == PMU_STATUS_LDO25_READY) || ((TYPE) == PMU_STATUS_BANDGAP_READY))

#define IS_PMU_FLAG(FLAG)                                                                                                                             \
    (((FLAG) == PMU_FLAG_LDO85D_ERROR) || ((FLAG) == PMU_FLAG_LDO85A_ERROR) || ((FLAG) == PMU_FLAG_LDO25_ERROR) || ((FLAG) == PMU_FLAG_BANDGAP_ERROR) \
     || ((FLAG) == PMU_FLAG_WAKEUP_EVENT))

#define IS_PMU_REGULATOR_VOLTAGE(VOLTAGE) (((VOLTAGE) == PMU_VOLTAGE_0_80V) || ((VOLTAGE) == PMU_VOLTAGE_0_95V) || ((VOLTAGE) == PMU_VOLTAGE_2_50V))

#define IS_PMU_TRIM_VALUE(VALUE) ((VALUE) <= 0x3FU)

#define __HAL_PMU_POWER_ENABLE(__HANDLE__, __CONFIG__)  ((__HANDLE__)->Instance->CR |= (__CONFIG__))
#define __HAL_PMU_POWER_DISABLE(__HANDLE__, __CONFIG__) ((__HANDLE__)->Instance->CR &= (~__CONFIG__))

#define __HAL_PMU_LP_POWER_ENABLE(__HANDLE__, __CONFIG__)  ((__HANDLE__)->Instance->CRLP |= (__CONFIG__))
#define __HAL_PMU_LP_POWER_DISABLE(__HANDLE__, __CONFIG__) ((__HANDLE__)->Instance->CRLP &= (~__CONFIG__))

#define __HAL_PMU_PD_POWER_ENABLE(__HANDLE__, __CONFIG__)  ((__HANDLE__)->Instance->CRPD |= (__CONFIG__))
#define __HAL_PMU_PD_POWER_DISABLE(__HANDLE__, __CONFIG__) ((__HANDLE__)->Instance->CRPD &= (~__CONFIG__))

#define __HAL_PMU_RESET_REGISTER(__HANDLE__)         \
    do                                               \
    {                                                \
        (__HANDLE__)->Instance->CR     = 0x0000007C; \
        (__HANDLE__)->Instance->CRLP   = 0x0000007C; \
        (__HANDLE__)->Instance->CRPD   = 0x0000007C; \
        (__HANDLE__)->Instance->DFT    = 0x00000002; \
        (__HANDLE__)->Instance->TRM0   = 0x08421080; \
        (__HANDLE__)->Instance->TRM1   = 0x00000002; \
        (__HANDLE__)->Instance->TRMLP0 = 0x08421080; \
        (__HANDLE__)->Instance->TRMLP1 = 0x00000002; \
        (__HANDLE__)->Instance->SR     = 0x0000001F; \
        (__HANDLE__)->Instance->FR     = 0x00000000; \
        (__HANDLE__)->Instance->PDAR   = 0x00000000; \
    } while (0)

#define __HAL_PMU_LDO_IS_STABLE(__HANDLE__, __CONFIG__) (((__HANDLE__)->Instance->SR) & (__CONFIG__))

#define __HAL_PMU_ENTER_PD_MODE(__HANDLE__) ((__HANDLE__)->Instance->PDAR = (PMU_PDAR_PD_VALUE))
#define __HAL_PMU_EXIT_PD_MODE(__HANDLE__)  ((__HANDLE__)->Instance->PDAR = 0)

    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /* DARIC_HAL_PMU_H */