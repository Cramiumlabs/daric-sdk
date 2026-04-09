/**
 ******************************************************************************
 * @file    daric_hal_ao.h
 * @author  AO Team
 * @brief   Header file of AO HAL module.
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
#ifndef DARIC_HAL_AO_H
#define DARIC_HAL_AO_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"
#include <stdbool.h>

    /** @cond Peripheral memory map
     * @{
     */
    typedef struct
    {
        __IO uint32_t CLK32K_SEL; ///< 32KHz clock source selection register.
                                  ///< Address offset: 0x0000.

        __IO uint32_t CLK1HZ_FD; ///< 1Hz frequency division configuration register.
                                 ///< Address offset: 0x0004.

        __IO uint32_t WKUP_INTEN; ///< Wakeup mask and interrupt enable register.
                                  ///< Address offset: 0x0008.

        __IO uint32_t RSTCR_MASK; ///< Reset mask control register.
                                  ///< Address offset: 0x000C.

        const uint32_t RESERVED0[9]; ///< PMU block control register.
                                     ///< Address offset: 0x0010.
                                     ///< PMU block control register for sleep/deep-sleep mode.
                                     ///< Address offset: 0x0014.
                                     ///< PMU block control register for power-down mode.
                                     ///< Address offset: 0x0018.
                                     ///< PMU default test register.
                                     ///< Address offset: 0x001C.
                                     ///< PMU trimming configuration low 32-bit register.
                                     ///< Address offset: 0x0020.
                                     ///< PMU trimming configuration high 32-bit register.
                                     ///< Address offset: 0x0024.
                                     ///< PMU trims value0 register for low power.
                                     ///< Address offset: 0x0028.
                                     ///< PMU trims value1 register for low power.
                                     ///< Address offset: 0x002C.
                                     ///< Reserved.
                                     ///< Address offset: 0x0030.

        __IO uint32_t OSC_CR; ///< 32KHz oscillator IP control register.
                              ///< Address offset: 0x0034.

        const uint32_t RESERVED1[2]; ///< PMU Ready Status register.
                                     ///< Address offset: 0x0038.
                                     ///< PMU Error Flag register.
                                     ///< Address offset: 0x003C.

        __IO uint32_t FR; ///< AO domain interrupt flag register.
                          ///< Address offset: 0x0040.

        const uint32_t RESERVED2[3]; ///< PMU power down mode AR register.
                                     ///< Address offset: 0x0044.
                                     ///< Reserved.
                                     ///< Address offset: 0x0048.

        __IO uint32_t PERI_CLR; ///< Wake up interrupt clear AR register.
                                ///< Address offset: 0x0050.

        const uint32_t RESERVED3[3]; ///< Reserved.
                                     ///< Address offset: 0x0054-0x005C.

        __IO uint32_t IOX; ///< AO IO Pad AF selection register.
                           ///< Address offset: 0x0060.

        __IO uint32_t PADPU; ///< AO IO Pad pull up configuration register.
                             ///< Address offset: 0x0064.
    } AON_TypeDef;

/*!< Peripheral memory map */
#define AON_BASE (0x40060000UL)
#define AON      ((AON_TypeDef *)AON_BASE)

/****************** Bits definition for AO_CLK32K_SEL register *****************/
#define AO_CLK32K_SEL_SRC_Pos      (0U)
#define AO_CLK32K_SEL_SRC_Msk      (0x1UL << AO_CLK32K_SEL_SRC_Pos)
#define AO_CLK32K_SEL_SRC          AO_CLK32K_SEL_SRC_Msk
#define AO_CLK32K_SEL_SRC_INTERNAL (0x0UL << AO_CLK32K_SEL_SRC_Pos)
#define AO_CLK32K_SEL_SRC_EXTERNAL (0x1UL << AO_CLK32K_SEL_SRC_Pos)

#define AO_CLK32K_SEL_PDISIEN_Pos (1U)
#define AO_CLK32K_SEL_PDISIEN_Msk (0x1UL << AO_CLK32K_SEL_PDISIEN_Pos)
#define AO_CLK32K_SEL_PDISIEN     AO_CLK32K_SEL_PDISIEN_Msk

#define AO_CLK32K_SEL_PCLKICG_Pos (2U)
#define AO_CLK32K_SEL_PCLKICG_Msk (0x1UL << AO_CLK32K_SEL_PCLKICG_Pos)
#define AO_CLK32K_SEL_PCLKICG     AO_CLK32K_SEL_PCLKICG_Msk

/****************** Bits definition for AO_CLK1HZ_FD register *****************/
#define AO_CLK1HZ_FD_VALUE_Pos (0U)
#define AO_CLK1HZ_FD_VALUE_Msk (0x3FFFUL << AO_CLK1HZ_FD_VALUE_Pos)
#define AO_CLK1HZ_FD_VALUE     AO_CLK1HZ_FD_VALUE_Msk

/****************** Bits definition for AO_WKUP_INTEN register ****************/
/* Interrupt enable bits */
#define AO_INTEN_WDT_RST_Pos (0U)
#define AO_INTEN_WDT_RST_Msk (0x1UL << AO_INTEN_WDT_RST_Pos)
#define AO_INTEN_WDT_RST     AO_INTEN_WDT_RST_Msk

#define AO_INTEN_RTC_EVT_Pos (1U)
#define AO_INTEN_RTC_EVT_Msk (0x1UL << AO_INTEN_RTC_EVT_Pos)
#define AO_INTEN_RTC_EVT     AO_INTEN_RTC_EVT_Msk

#define AO_INTEN_ATIMER_Pos (2U)
#define AO_INTEN_ATIMER_Msk (0x1UL << AO_INTEN_ATIMER_Pos)
#define AO_INTEN_ATIMER     AO_INTEN_ATIMER_Msk

#define AO_INTEN_WDT_EVT_Pos (3U)
#define AO_INTEN_WDT_EVT_Msk (0x1UL << AO_INTEN_WDT_EVT_Pos)
#define AO_INTEN_WDT_EVT     AO_INTEN_WDT_EVT_Msk

#define AO_INTEN_KPC_EVT_Pos (4U)
#define AO_INTEN_KPC_EVT_Msk (0x1UL << AO_INTEN_KPC_EVT_Pos)
#define AO_INTEN_KPC_EVT     AO_INTEN_KPC_EVT_Msk

#define AO_INTEN_KPC_ASYNC_Pos (5U)
#define AO_INTEN_KPC_ASYNC_Msk (0x1UL << AO_INTEN_KPC_ASYNC_Pos)
#define AO_INTEN_KPC_ASYNC     AO_INTEN_KPC_ASYNC_Msk

/* Wakeup mask bits */
#define AO_WKUP_INTEN_WDT_RST_MASK_Pos (8U)
#define AO_WKUP_INTEN_WDT_RST_MASK_Msk (0x1UL << AO_WKUP_INTEN_WDT_RST_MASK_Pos)
#define AO_WKUP_INTEN_WDT_RST_MASK     AO_WKUP_INTEN_WDT_RST_MASK_Msk

#define AO_WKUP_INTEN_RTC_EVT_MASK_Pos (9U)
#define AO_WKUP_INTEN_RTC_EVT_MASK_Msk (0x1UL << AO_WKUP_INTEN_RTC_EVT_MASK_Pos)
#define AO_WKUP_INTEN_RTC_EVT_MASK     AO_WKUP_INTEN_RTC_EVT_MASK_Msk

#define AO_WKUP_INTEN_ATIMER_MASK_Pos (10U)
#define AO_WKUP_INTEN_ATIMER_MASK_Msk (0x1UL << AO_WKUP_INTEN_ATIMER_MASK_Pos)
#define AO_WKUP_INTEN_ATIMER_MASK     AO_WKUP_INTEN_ATIMER_MASK_Msk

#define AO_WKUP_INTEN_WDT_EVT_MASK_Pos (11U)
#define AO_WKUP_INTEN_WDT_EVT_MASK_Msk (0x1UL << AO_WKUP_INTEN_WDT_EVT_MASK_Pos)
#define AO_WKUP_INTEN_WDT_EVT_MASK     AO_WKUP_INTEN_WDT_EVT_MASK_Msk

#define AO_WKUP_INTEN_KPC_EVT_MASK_Pos (12U)
#define AO_WKUP_INTEN_KPC_EVT_MASK_Msk (0x1UL << AO_WKUP_INTEN_KPC_EVT_MASK_Pos)
#define AO_WKUP_INTEN_KPC_EVT_MASK     AO_WKUP_INTEN_KPC_EVT_MASK_Msk

#define AO_WKUP_INTEN_KPC_ASYNC_MASK_Pos (13U)
#define AO_WKUP_INTEN_KPC_ASYNC_MASK_Msk (0x1UL << AO_WKUP_INTEN_KPC_ASYNC_MASK_Pos)
#define AO_WKUP_INTEN_KPC_ASYNC_MASK     AO_WKUP_INTEN_KPC_ASYNC_MASK_Msk

#define AO_WKUP_INTEN_PF1_PD_MASK_Pos (16U)
#define AO_WKUP_INTEN_PF1_PD_MASK_Msk (0x1UL << AO_WKUP_INTEN_PF1_PD_MASK_Pos)
#define AO_WKUP_INTEN_PF1_PD_MASK     AO_WKUP_INTEN_PF1_PD_MASK_Msk

#define AO_WKUP_INTEN_PF0_PD_MASK_Pos (17U)
#define AO_WKUP_INTEN_PF0_PD_MASK_Msk (0x1UL << AO_WKUP_INTEN_PF0_PD_MASK_Pos)
#define AO_WKUP_INTEN_PF0_PD_MASK     AO_WKUP_INTEN_PF0_PD_MASK_Msk

/****************** Bits definition for AO_RSTCR_MASK register ****************/
#define AO_RSTCR_MASK_Msk     (0x1FUL)
#define AO_RSTCR_MASK_POR_Pos (0U)
#define AO_RSTCR_MASK_POR_Msk (0x1UL << AO_RSTCR_MASK_POR_Pos)
#define AO_RSTCR_MASK_POR     AO_RSTCR_MASK_POR_Msk

#define AO_RSTCR_MASK_DIG_REG_Pos (1U)
#define AO_RSTCR_MASK_DIG_REG_Msk (0x1UL << AO_RSTCR_MASK_DIG_REG_Pos)
#define AO_RSTCR_MASK_DIG_REG     AO_RSTCR_MASK_DIG_REG_Msk

#define AO_RSTCR_MASK_ANA_REG_Pos (2U)
#define AO_RSTCR_MASK_ANA_REG_Msk (0x1UL << AO_RSTCR_MASK_ANA_REG_Pos)
#define AO_RSTCR_MASK_ANA_REG     AO_RSTCR_MASK_ANA_REG_Msk

#define AO_RSTCR_MASK_25V_REG_Pos (3U)
#define AO_RSTCR_MASK_25V_REG_Msk (0x1UL << AO_RSTCR_MASK_25V_REG_Pos)
#define AO_RSTCR_MASK_25V_REG     AO_RSTCR_MASK_25V_REG_Msk

#define AO_RSTCR_MASK_BG_Pos (4U)
#define AO_RSTCR_MASK_BG_Msk (0x1UL << AO_RSTCR_MASK_BG_Pos)
#define AO_RSTCR_MASK_BG     AO_RSTCR_MASK_BG_Msk

/****************** Bits definition for AO_OSC_CR register ********************/
#define AO_OSC_CR_Msk           (0x1FFFFUL)
#define AO_OSC_CR_ACTIVE_EN_Pos (0U)
#define AO_OSC_CR_ACTIVE_EN_Msk (0x1UL << AO_OSC_CR_ACTIVE_EN_Pos)
#define AO_OSC_CR_ACTIVE_EN     AO_OSC_CR_ACTIVE_EN_Msk ///< Bit 0: Active mode enable

#define AO_OSC_CR_ACTIVE_TRIM_Pos (1U)
#define AO_OSC_CR_ACTIVE_TRIM_Msk (0x7FUL << AO_OSC_CR_ACTIVE_TRIM_Pos)
#define AO_OSC_CR_ACTIVE_TRIM     AO_OSC_CR_ACTIVE_TRIM_Msk ///< Bits[7:1]: active mode trim

#define AO_OSC_CR_SLEEP_EN_Pos (8U)
#define AO_OSC_CR_SLEEP_EN_Msk (0x1UL << AO_OSC_CR_SLEEP_EN_Pos)
#define AO_OSC_CR_SLEEP_EN     AO_OSC_CR_SLEEP_EN_Msk ///< Bit 8: Sleep mode enable

#define AO_OSC_CR_SLEEP_TRIM_Pos (9U)
#define AO_OSC_CR_SLEEP_TRIM_Msk (0x7FUL << AO_OSC_CR_SLEEP_TRIM_Pos)
#define AO_OSC_CR_SLEEP_TRIM     AO_OSC_CR_SLEEP_TRIM_Msk ///< Bits[15:9]: sleep mode trim

#define AO_OSC_CR_PD_EN_Pos (16U)
#define AO_OSC_CR_PD_EN_Msk (0x1UL << AO_OSC_CR_PD_EN_Pos)
#define AO_OSC_CR_PD_EN     AO_OSC_CR_PD_EN_Msk ///< Bit 16: Power-down mode enable

/****************** Bits definition for AO_FR register ************************/
#define AO_FR_WDT_RST_Pos (0U)
#define AO_FR_WDT_RST_Msk (0x1UL << AO_FR_WDT_RST_Pos)
#define AO_FR_WDT_RST     AO_FR_WDT_RST_Msk

#define AO_FR_RTC_EVT_Pos (1U)
#define AO_FR_RTC_EVT_Msk (0x1UL << AO_FR_RTC_EVT_Pos)
#define AO_FR_RTC_EVT     AO_FR_RTC_EVT_Msk

#define AO_FR_ATIMER_Pos (2U)
#define AO_FR_ATIMER_Msk (0x1UL << AO_FR_ATIMER_Pos)
#define AO_FR_ATIMER     AO_FR_ATIMER_Msk

#define AO_FR_WDT_EVT_Pos (3U)
#define AO_FR_WDT_EVT_Msk (0x1UL << AO_FR_WDT_EVT_Pos)
#define AO_FR_WDT_EVT     AO_FR_WDT_EVT_Msk

#define AO_FR_KPC_EVT_Pos (4U)
#define AO_FR_KPC_EVT_Msk (0x1UL << AO_FR_KPC_EVT_Pos)
#define AO_FR_KPC_EVT     AO_FR_KPC_EVT_Msk

#define AO_FR_KPC_ASYNC_Pos (5U)
#define AO_FR_KPC_ASYNC_Msk (0x1UL << AO_FR_KPC_ASYNC_Pos)
#define AO_FR_KPC_ASYNC     AO_FR_KPC_ASYNC_Msk

#define AO_FR_PF1_PD_Pos (8U)
#define AO_FR_PF1_PD_Msk (0x1UL << AO_FR_PF1_PD_Pos)
#define AO_FR_PF1_PD     AO_FR_PF1_PD_Msk

#define AO_FR_PF0_PD_Pos (9U)
#define AO_FR_PF0_PD_Msk (0x1UL << AO_FR_PF0_PD_Pos)
#define AO_FR_PF0_PD     AO_FR_PF0_PD_Msk

/****************** Bits definition for AO_PERI_CLR register ******************/
#define AO_PERI_CLR_VALUE (0x000000AAUL)

/****************** Bits definition for AO_IOX register ***********************/
#define AO_IOX_AF_SEL_Pos (0U)
#define AO_IOX_AF_SEL_Msk (0x1UL << AO_IOX_AF_SEL_Pos)
#define AO_IOX_AF_SEL     AO_IOX_AF_SEL_Msk
#define AO_IOX_AF0        (0x0UL << AO_IOX_AF_SEL_Pos)
#define AO_IOX_AF1        (0x1UL << AO_IOX_AF_SEL_Pos)

/****************** Bits definition for AO_PADPU register *********************/
#define AO_PADPU_PF_Msk  (0x3FFUL)
#define AO_PADPU_PF0_Pos (0U)
#define AO_PADPU_PF0_Msk (0x1UL << AO_PADPU_PF0_Pos)
#define AO_PADPU_PF0     AO_PADPU_PF0_Msk

#define AO_PADPU_PF1_Pos (1U)
#define AO_PADPU_PF1_Msk (0x1UL << AO_PADPU_PF1_Pos)
#define AO_PADPU_PF1     AO_PADPU_PF1_Msk

#define AO_PADPU_PF2_Pos (2U)
#define AO_PADPU_PF2_Msk (0x1UL << AO_PADPU_PF2_Pos)
#define AO_PADPU_PF2     AO_PADPU_PF2_Msk

#define AO_PADPU_PF3_Pos (3U)
#define AO_PADPU_PF3_Msk (0x1UL << AO_PADPU_PF3_Pos)
#define AO_PADPU_PF3     AO_PADPU_PF3_Msk

#define AO_PADPU_PF4_Pos (4U)
#define AO_PADPU_PF4_Msk (0x1UL << AO_PADPU_PF4_Pos)
#define AO_PADPU_PF4     AO_PADPU_PF4_Msk

#define AO_PADPU_PF5_Pos (5U)
#define AO_PADPU_PF5_Msk (0x1UL << AO_PADPU_PF5_Pos)
#define AO_PADPU_PF5     AO_PADPU_PF5_Msk

#define AO_PADPU_PF6_Pos (6U)
#define AO_PADPU_PF6_Msk (0x1UL << AO_PADPU_PF6_Pos)
#define AO_PADPU_PF6     AO_PADPU_PF6_Msk

#define AO_PADPU_PF7_Pos (7U)
#define AO_PADPU_PF7_Msk (0x1UL << AO_PADPU_PF7_Pos)
#define AO_PADPU_PF7     AO_PADPU_PF7_Msk

#define AO_PADPU_PF8_Pos (8U)
#define AO_PADPU_PF8_Msk (0x1UL << AO_PADPU_PF8_Pos)
#define AO_PADPU_PF8     AO_PADPU_PF8_Msk

#define AO_PADPU_PF9_Pos (9U)
#define AO_PADPU_PF9_Msk (0x1UL << AO_PADPU_PF9_Pos)
#define AO_PADPU_PF9     AO_PADPU_PF9_Msk

    /** @endcond
     * @}
     */

    /*! \brief
     *  Boot info type definition
     */
    typedef enum
    {
        HAL_AO_BOOT_INFO_TYPE_BOOTMODE = 0,
        HAL_AO_BOOT_INFO_TYPE_UNDEFINED,
    } HAL_AO_InfoTypeDef;

    /*! \brief
     *  Boot mode definition
     */
    typedef enum
    {
        HAL_AO_BOOT_NORMAL_MODE = 0,
        HAL_AO_BOOT_FAST_MODE,
        HAL_AO_BOOT_AFTER_DOWNLOAD_MODE,
        HAL_AO_BOOT_FOTA_MODE,
        HAL_AO_BOOT_UNDEFINED_MODE,
    } HAL_AO_BootModeTypeDef;

    /**
     * @brief  AO Init structure definition
     */
    typedef struct
    {
        HAL_AO_BootModeTypeDef SleepMode; /*!< fastboot flag */
        HAL_AO_BootModeTypeDef BootMode;  /*!< boot mode */
    } HAL_AO_BootModeStruct;

    /**
     * @brief  AO Init structure definition
     */
    typedef struct
    {
        uint32_t ClockSelection; /*!< Clock source selection */
        uint32_t ClockDivision;  /*!< Clock division coefficient */
        uint32_t Oscillator;     /*!< Oscillator configuration */
        uint32_t InterruptMask;  /*!< Interrupt mask settings */
        uint32_t WakeupMask;     /*!< Wakeup mask settings */
        uint32_t ResetMask;      /*!< Reset mask settings */
        uint32_t AltFunction;    /*!< GPIO function configuration */
        uint32_t PullUpConfig;   /*!< GPIO pull-up configuration */
    } AO_InitTypeDef;

    /**
     * @brief  HAL AO State structure definition
     */
    typedef enum
    {
        HAL_AO_STATE_RESET = 0x00U, /*!< AO not yet initialized or disabled */
        HAL_AO_STATE_READY = 0x01U, /*!< AO initialized and ready for use   */
        HAL_AO_STATE_BUSY  = 0x02U, /*!< AO process is ongoing              */
        HAL_AO_STATE_ERROR = 0x03U  /*!< AO error state                     */
    } HAL_AO_StateTypeDef;

    /**
     * @brief  AO handle Structure definition
     */
    typedef struct __AO_HandleTypeDef
    {
        AON_TypeDef        *Instance;                               /*!< Register base address */
        AO_InitTypeDef      Init;                                   /*!< AO initialization parameters */
        HAL_AO_StateTypeDef State;                                  /*!< AO state */
        void (*MspInitCallback)(struct __AO_HandleTypeDef *hpmu);   /*!< MSP init callback */
        void (*MspDeInitCallback)(struct __AO_HandleTypeDef *hpmu); /*!< MSP de-init callback */
    } AO_HandleTypeDef;

/** @defgroup AO_Clock_Source AO Clock Source
 * @{
 */
#define AO_CLOCK_32K_EXT (AO_CLK32K_SEL_SRC_EXTERNAL)
#define AO_CLOCK_ISO_EN  (AO_CLK32K_SEL_PDISIEN)
#define AO_CLOCK_PCLK_EN (AO_CLK32K_SEL_PCLKICG)
/**
 * @}
 */

/** @defgroup AO_Clock_Division AO Clock Division
 * @{
 */
#define AO_CLOCK_DIVISION_DEFAULT  (0x3FFFUL)
#define AO_CLOCK_DIVISION_ACCURATE (0x3E80UL) // 1 Hz
#define AO_CLOCK_DIVISION_1KHZ     (0x000FUL) // 1k Hz
/**
 * @}
 */

/** @defgroup AO_Oscillator_Config AO_Oscillator_Config
 * @{
 */
#define AO_OSC_PD_32K_EN  (AO_OSC_CR_PD_EN)
#define AO_OSC_DS_32K_EN  (AO_OSC_CR_SLEEP_EN)
#define AO_OSC_ACT_32K_EN (AO_OSC_CR_ACTIVE_EN)

#define AO_OSC_DS_32K  (0x00000026UL << AO_OSC_CR_SLEEP_TRIM_Pos)
#define AO_OSC_ACT_32K (0x00000026UL << AO_OSC_CR_ACTIVE_TRIM_Pos)
/**
 * @}
 */

/** @defgroup AO_Interrupt_Mask AO Interrupt_Mask
 * @{
 */
#define AO_IRQ_WDT_RST      (AO_INTEN_WDT_RST)   /*!< WDT reset wakeup */
#define AO_IRQ_RTC_EVENT    (AO_INTEN_RTC_EVT)   /*!< RTC event wakeup */
#define AO_IRQ_ATIMER_EVENT (AO_INTEN_ATIMER)    /*!< ATimer event wakeup */
#define AO_IRQ_WDT_EVENT    (AO_INTEN_WDT_EVT)   /*!< WDT event wakeup */
#define AO_IRQ_KPC_EVENT    (AO_INTEN_KPC_EVT)   /*!< Keypad event wakeup */
#define AO_IRQ_KPC_ASYNC    (AO_INTEN_KPC_ASYNC) /*!< Keypad async wakeup */
/**
 * @}
 */

/** @defgroup AO_Wakeup_Source AO Wakeup Source
 * @{
 */
#define AO_WKUP_NOT_WDT_RST      (AO_WKUP_INTEN_WDT_RST_MASK)   /*!< WDT reset can not wakeup */
#define AO_WKUP_NOT_RTC_EVENT    (AO_WKUP_INTEN_RTC_EVT_MASK)   /*!< RTC event can not wakeup */
#define AO_WKUP_NOT_ATIMER_EVENT (AO_WKUP_INTEN_ATIMER_MASK)    /*!< ATimer event can not wakeup */
#define AO_WKUP_NOT_WDT_EVENT    (AO_WKUP_INTEN_WDT_EVT_MASK)   /*!< WDT event can not wakeup */
#define AO_WKUP_NOT_KPC_EVENT    (AO_WKUP_INTEN_KPC_EVT_MASK)   /*!< Keypad event can not wakeup */
#define AO_WKUP_NOT_KPC_ASYNC    (AO_WKUP_INTEN_KPC_ASYNC_MASK) /*!< Keypad async can not wakeup */
#define AO_WKUP_NOT_PF1_PD       (AO_WKUP_INTEN_PF1_PD_MASK)    /*!< PF1 pull-down can not wakeup */
#define AO_WKUP_NOT_PF0_PD       (AO_WKUP_INTEN_PF0_PD_MASK)    /*!< PF0 pull-down can not wakeup */
/**
 * @}
 */

/** @defgroup AO_Reset_Source AO Reset Source
 * @{
 */
#define AO_RESET_NOT_POR     (AO_RSTCR_MASK_POR)     /*!< PMU POR can not reset source */
#define AO_RESET_NOT_VDD85D  (AO_RSTCR_MASK_DIG_REG) /*!< PMU 0.8V digital regulator can not reset source */
#define AO_RESET_NOT_VDD85A  (AO_RSTCR_MASK_ANA_REG) /*!< PMU 0.8V analog regulator can not reset source */
#define AO_RESET_NOT_VDD25   (AO_RSTCR_MASK_25V_REG) /*!< PMU 2.5V regulator can not reset source */
#define AO_RESET_NOT_BANDGAP (AO_RSTCR_MASK_BG)      /*!< PMU bandgap can not reset source */
/**
 * @}
 */

/** @defgroup AO_GPIO_AF AO GPIO AF
 * @{
 */
#define AO_GPIOF_AF0_GPIO   (0x00000000UL)
#define AO_GPIOF_AF1_KEYPAD (0x00000001UL)
/**
 * @}
 */

/** @defgroup AO_GPIO_AF AO GPIO AF
 * @{
 */
#define AO_GPIOF_0_PU (0x00000001UL)
#define AO_GPIOF_1_PU (0x00000002UL)
#define AO_GPIOF_2_PU (0x00000004UL)
#define AO_GPIOF_3_PU (0x00000008UL)
#define AO_GPIOF_4_PU (0x00000010UL)
#define AO_GPIOF_5_PU (0x00000020UL)
#define AO_GPIOF_6_PU (0x00000040UL)
#define AO_GPIOF_7_PU (0x00000080UL)
#define AO_GPIOF_8_PU (0x00000100UL)
#define AO_GPIOF_9_PU (0x00000200UL)
    /**
     * @}
     */

    uint32_t          HAL_AO_BootInfo_Length(HAL_AO_InfoTypeDef infoType);
    HAL_StatusTypeDef HAL_AO_BootInfo_Set(HAL_AO_InfoTypeDef infoType, void *infoValue);
    HAL_StatusTypeDef HAL_AO_BootInfo_Get(HAL_AO_InfoTypeDef infoType, void *infoValue);

    HAL_StatusTypeDef   HAL_AO_Init(AO_HandleTypeDef *hao);
    HAL_StatusTypeDef   HAL_AO_DeInit(AO_HandleTypeDef *hao);
    HAL_StatusTypeDef   HAL_AO_RegisterCallback(uint32_t InterruptMask, uint32_t WakeupMask, void (*pCallback)(void *param), void *param);
    HAL_StatusTypeDef   HAL_AO_UnRegisterCallback(uint32_t InterruptMask, uint32_t WakeupMask);
    HAL_AO_StateTypeDef HAL_AO_GetState(AO_HandleTypeDef *hao);

#define HAL_AO_LOCK()   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_AO_UNLOCK() HAL_UNLOCK

#define IS_AO_CLOCK_SELECTION(__CONFIG__) (((__CONFIG__) == AO_CLOCK_SOURCE_INTERNAL) || ((__CONFIG__) == AO_CLOCK_SOURCE_EXTERNAL))
#define IS_AO_CLOCK_DIVISION(__CONFIG__)  (((__CONFIG__) == AO_CLOCK_DIVISION_DEFAULT) || ((__CONFIG__) == AO_CLOCK_DIVISION_ACCURATE))
#define IS_AO_OSCILLATOR(__CONFIG__)      (__CONFIG__ <= AO_OSC_CR_Msk)
#define IS_AO_INTERRUPT_MASK(__CONFIG__)  (((__CONFIG__) & ~(AO_IRQ_WDT_RST | AO_IRQ_RTC_EVENT | AO_IRQ_ATIMER_EVENT | AO_IRQ_WDT_EVENT | AO_IRQ_KPC_EVENT | AO_IRQ_KPC_ASYNC)) == 0)
#define IS_AO_WAKEUP_MASK(__CONFIG__)                                                                                                                                             \
    (((__CONFIG__) & ~(AO_WKUP_WDT_RST | AO_WKUP_RTC_EVENT | AO_WKUP_ATIMER_EVENT | AO_WKUP_WDT_EVENT | AO_WKUP_KPC_EVENT | AO_WKUP_KPC_ASYNC | AO_WKUP_PF1_PD | AO_WKUP_PF0_PD)) \
     == 0)
#define IS_AO_RESET_MASK(__CONFIG__)    ((__CONFIG__) <= AO_RSTCR_MASK_Msk)
#define IS_AO_ALT_FUNCTION(__CONFIG__)  (((__CONFIG__) == AO_GPIOF_AF0_GPIO) || ((__CONFIG__) == AO_GPIOF_AF1_KEYPAD))
#define IS_AO_PULLUP_CONFIG(__CONFIG__) ((__CONFIG__) <= AO_PADPU_PF_Msk)

#define __HAL_AO_GET_IRQ_FLAG(__HANDLE__)               ((__HANDLE__)->Instance->FR)
#define __HAL_AO_CLEAR_IRQ_FLAG(__HANDLE__, __CONFIG__) ((__HANDLE__)->Instance->FR = (__CONFIG__))
#define __HAL_AO_CLEAR_WKUP_FLAG(__HANDLE__)            ((__HANDLE__)->Instance->PERI_CLR = AO_PERI_CLR_VALUE)

#define __HAL_AO_IRQ_ENABLE(__HANDLE__, __CONFIG__)  ((__HANDLE__)->Instance->WKUP_INTEN |= (__CONFIG__))
#define __HAL_AO_IRQ_DISABLE(__HANDLE__, __CONFIG__) ((__HANDLE__)->Instance->WKUP_INTEN &= ~(__CONFIG__))

#define __HAL_AO_WAKEUP_ENABLE(__HANDLE__, __CONFIG__)  ((void)(__HANDLE__)->Instance->WKUP_INTEN)
#define __HAL_AO_WAKEUP_DISABLE(__HANDLE__, __CONFIG__) ((void)(__HANDLE__)->Instance->WKUP_INTEN)

#define __HAL_AO_RESET_REGISTER(__HANDLE__)              \
    do                                                   \
    {                                                    \
        (__HANDLE__)->Instance->CLK32K_SEL = 0x00000006; \
        (__HANDLE__)->Instance->CLK1HZ_FD  = 0x00003FFF; \
        (__HANDLE__)->Instance->WKUP_INTEN = 0x00000000; \
        (__HANDLE__)->Instance->RSTCR_MASK = 0x00000000; \
        (__HANDLE__)->Instance->OSC_CR     = 0x00014D4D; \
        (__HANDLE__)->Instance->FR         = 0xFFFFFFFF; \
        (__HANDLE__)->Instance->PERI_CLR   = 0;          \
        (__HANDLE__)->Instance->IOX        = 0x00000000; \
        (__HANDLE__)->Instance->PADPU      = 0x000003FF; \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* DARIC_HAL_AO_H */