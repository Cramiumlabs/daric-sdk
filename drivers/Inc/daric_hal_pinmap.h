/**
 ******************************************************************************
 * @file    daric_hal_pinmap.h
 * @author  PINMAP Team
 * @brief   Header file of PINMAP HAL module.
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
#ifndef DARIC_HAL_PINMAP_H
#define DARIC_HAL_PINMAP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "daric_hal_def.h"

    /**
     * @brief  PINMAP Initialization Structure definition.
     * This structure defines the configuration of function for a pin.
     */
    typedef struct
    {
        GPIO_TypeDef *Port; /**< Pointer to the port (e.g.,PORT_A, PORT_B, etc.) */
        uint16_t      Pin;  /**< Pin number within the specified pin */
        uint8_t       Func; /**< Function mode of the pin, e.g., AF1_PA3_UART0_RX,
                               AF1_PA4_UART0_TX, etc. */
    } PINMAP_InitTypeDef;

/** @defgroup PINMAP_ports_define
 * @{
 */
#define PORT_A ((GPIO_TypeDef *)GPIOA_BASE) ///< Port A selected
#define PORT_B ((GPIO_TypeDef *)GPIOB_BASE) ///< Port B selected
#define PORT_C ((GPIO_TypeDef *)GPIOC_BASE) ///< Port C selected
#define PORT_D ((GPIO_TypeDef *)GPIOD_BASE) ///< Port D selected
#define PORT_E ((GPIO_TypeDef *)GPIOE_BASE) ///< Port E selected
#define PORT_F ((GPIO_TypeDef *)GPIOF_BASE) ///< Port F selected
/**
 * @}
 */

/** @defgroup PINMAP_pins_define
 * @{
 */
#define PIN_0  ((uint16_t)0x0001) ///< Pin 0 selected
#define PIN_1  ((uint16_t)0x0002) ///< Pin 1 selected
#define PIN_2  ((uint16_t)0x0004) ///< Pin 2 selected
#define PIN_3  ((uint16_t)0x0008) ///< Pin 3 selected
#define PIN_4  ((uint16_t)0x0010) ///< Pin 4 selected
#define PIN_5  ((uint16_t)0x0020) ///< Pin 5 selected
#define PIN_6  ((uint16_t)0x0040) ///< Pin 6 selected
#define PIN_7  ((uint16_t)0x0080) ///< Pin 7 selected
#define PIN_8  ((uint16_t)0x0100) ///< Pin 8 selected
#define PIN_9  ((uint16_t)0x0200) ///< Pin 9 selected
#define PIN_10 ((uint16_t)0x0400) ///< Pin 10 selected
#define PIN_11 ((uint16_t)0x0800) ///< Pin 11 selected
#define PIN_12 ((uint16_t)0x1000) ///< Pin 12 selected
#define PIN_13 ((uint16_t)0x2000) ///< Pin 13 selected
#define PIN_14 ((uint16_t)0x4000) ///< Pin 14 selected
#define PIN_15 ((uint16_t)0x8000) ///< Pin 15 selected
/**
 * @}
 */

/** @defgroup AF0 function mappings
 * @{
 */
#define AF0_PA4_ADC_SI0 ((uint8_t)0x00) ///< ADC_SI0   function
#define AF0_PA5_ADC_SI1 ((uint8_t)0x00) ///< ADC_SI1   function
#define AF0_PA6_ADC_SI2 ((uint8_t)0x00) ///< ADC_SI2   function
#define AF0_PA7_ADC_SI3 ((uint8_t)0x00) ///< ADC_SI3   function
/**
 * @}
 */

/** @defgroup AF1 function mappings
 * @{
 */
#define AF1_PA3_UART0_RX    ((uint8_t)0x01) ///< UART0_RX   function
#define AF1_PA4_UART0_TX    ((uint8_t)0x01) ///< UART0_TX   function
#define AF1_PB0_CAM_D0      ((uint8_t)0x01) ///< CAM_D0     function
#define AF1_PB1_CAM_D1      ((uint8_t)0x01) ///< CAM_D1     function
#define AF1_PB2_CAM_D2      ((uint8_t)0x01) ///< CAM_D2     function
#define AF1_PB3_CAM_D3      ((uint8_t)0x01) ///< CAM_D3     function
#define AF1_PB4_CAM_D4      ((uint8_t)0x01) ///< CAM_D4     function
#define AF1_PB5_CAM_D5      ((uint8_t)0x01) ///< CAM_D5     function
#define AF1_PB6_CAM_D6      ((uint8_t)0x01) ///< CAM_D6     function
#define AF1_PB7_CAM_D7      ((uint8_t)0x01) ///< CAM_D7     function
#define AF1_PB8_CAM_HSYNC   ((uint8_t)0x01) ///< CAM_HSYNC  function
#define AF1_PB9_CAM_VSYNC   ((uint8_t)0x01) ///< CAM_VSYNC  function
#define AF1_PB10_CAM_PIXCLK ((uint8_t)0x01) ///< CAM_PIXCLK function
#define AF1_PB11_I2C0_SCL   ((uint8_t)0x01) ///< I2C0_SCL   function
#define AF1_PB12_I2C0_SDA   ((uint8_t)0x01) ///< I2C0_SDA   function
#define AF1_PB13_UART2_RX   ((uint8_t)0x01) ///< UART2_RX   function
#define AF1_PB14_UART2_TX   ((uint8_t)0x01) ///< UART2_RX   function
#define AF1_PC0_SDIO_CLK    ((uint8_t)0x01) ///< SDIO_CLK   function
#define AF1_PC1_SDIO_CMD    ((uint8_t)0x01) ///< SDIO_CMD   function
#define AF1_PC2_SDIO_D0     ((uint8_t)0x01) ///< SDIO_D0    function
#define AF1_PC3_SDIO_D1     ((uint8_t)0x01) ///< SDIO_D1    function
#define AF1_PC4_SDIO_D2     ((uint8_t)0x01) ///< SDIO_D2    function
#define AF1_PC5_SDIO_D3     ((uint8_t)0x01) ///< SDIO_D3    function
#define AF1_PC7_SPIM1_SD0   ((uint8_t)0x01) ///< SPIM1_SD0  function
#define AF1_PC8_SPIM1_SD1   ((uint8_t)0x01) ///< SPIM1_SD1  function
#define AF1_PC9_SPIM1_SD2   ((uint8_t)0x01) ///< SPIM1_SD2  function
#define AF1_PC10_SPIM1_SD3  ((uint8_t)0x01) ///< SPIM1_SD3  function
#define AF1_PC11_SPIM1_CLK  ((uint8_t)0x01) ///< SPIM1_CLK  function
#define AF1_PC12_SPIM1_CSN0 ((uint8_t)0x01) ///< SPIM1_CSN0 function
#define AF1_PC13_SPIM1_CSN1 ((uint8_t)0x01) ///< SPIM1_CSN1 function
#define AF1_PD0_SPIM0_SD0   ((uint8_t)0x01) ///< SPIM0_SD0  function
#define AF1_PD1_SPIM0_SD1   ((uint8_t)0x01) ///< SPIM0_SD1  function
#define AF1_PD2_SPIM0_SD2   ((uint8_t)0x01) ///< SPIM0_SD2  function
#define AF1_PD3_SPIM0_SD3   ((uint8_t)0x01) ///< SPIM0_SD3  function
#define AF1_PD4_SPIM0_CLK   ((uint8_t)0x01) ///< SPIM0_CLK  function
#define AF1_PD5_SPIM0_CSN0  ((uint8_t)0x01) ///< SPIM0_CSN0 function
#define AF1_PD6_SPIM0_CSN1  ((uint8_t)0x01) ///< SPIM0_CSN1 function
#define AF1_PD7_I2SS_SD     ((uint8_t)0x01) ///< I2SS_SD    function
#define AF1_PD8_I2SS_WS     ((uint8_t)0x01) ///< I2SS_WS    function
#define AF1_PD9_I2SS_SCK    ((uint8_t)0x01) ///< I2SS_SCK   function
#define AF1_PD10_I2SM_SD    ((uint8_t)0x01) ///< I2SM_SD    function
#define AF1_PD11_I2SM_WS    ((uint8_t)0x01) ///< I2SM_WS    function
#define AF1_PD12_I2SM_SCK   ((uint8_t)0x01) ///< I2SM_SCK   function
#define AF1_PD13_UART1_RX   ((uint8_t)0x01) ///< UART1_RX   function
#define AF1_PD14_UART1_TX   ((uint8_t)0x01) ///< UART1_TX   function
#ifdef CONFIG_SOC_DARIC_NTO_A
#define AF1_PE0_SPIM0_CLK   ((uint8_t)0x01) ///< SPIM0_CLK  function
#define AF1_PE1_SPIM0_SD0   ((uint8_t)0x01) ///< SPIM0_SD0  function
#define AF1_PE2_SPIM0_SD1   ((uint8_t)0x01) ///< SPIM0_SD1  function
#define AF1_PE3_SPIM0_CSN0  ((uint8_t)0x01) ///< SPIM0_CSN0 function
#define AF1_PE4_UART3_RX    ((uint8_t)0x01) ///< UART3_RX   function
#define AF1_PE5_UART3_TX    ((uint8_t)0x01) ///< UART3_TX   function
#define AF1_PE6_SPIM3_CLK   ((uint8_t)0x01) ///< SPIM3_CLK  function
#define AF1_PE7_SPIM3_SD0   ((uint8_t)0x01) ///< SPIM3_SD0  function
#define AF1_PE8_SPIM3_SD1   ((uint8_t)0x01) ///< SPIM3_SD1  function
#define AF1_PE9_SPIM3_CSN0  ((uint8_t)0x01) ///< SPIM3_CSN0 function
#define AF1_PE10_SPIS0_CLK  ((uint8_t)0x01) ///< SPIS0_CLK  function
#define AF1_PE11_SPIS0_CSN  ((uint8_t)0x01) ///< SPIS0_CSN  function
#define AF1_PE12_SPIS0_MOSI ((uint8_t)0x01) ///< SPIS0_MOSI function
#define AF1_PE13_SPIS0_MISO ((uint8_t)0x01) ///< SPIS0_MISO function
#define AF1_PE14_I2C3_SCL   ((uint8_t)0x01) ///< I2C3_SCL   function
#define AF1_PE15_I2C3_SDA   ((uint8_t)0x01) ///< I2C3_SDA   function
#define AF1_PF2_KPI_0       ((uint8_t)0x01) ///< KPI_0  function
#define AF1_PF3_KPI_1       ((uint8_t)0x01) ///< KPI_1  function
#define AF1_PF4_KPI_2       ((uint8_t)0x01) ///< KPI_2  function
#define AF1_PF5_KPI_3       ((uint8_t)0x01) ///< KPI_3  function
#define AF1_PF6_KPO_0       ((uint8_t)0x01) ///< KPO_0  function
#define AF1_PF7_KPO_1       ((uint8_t)0x01) ///< KPO_1  function
#define AF1_PF8_KPO_2       ((uint8_t)0x01) ///< KPO_2  function
#define AF1_PF9_KPO_3       ((uint8_t)0x01) ///< KPO_3  function
#endif

/**
 * @}
 */

/** @defgroup AF2 function mappings
 * @{
 */
#define AF2_PA0_I2C1_SCL    ((uint8_t)0x02) ///< I2C1_SCL   function
#define AF2_PA1_I2C1_SDA    ((uint8_t)0x02) ///< I2C1_SDA   function
#define AF2_PA5_I2C0_SCL    ((uint8_t)0x02) ///< I2C0_SCL   function
#define AF2_PA6_I2C0_SDA    ((uint8_t)0x02) ///< I2C0_SDA   function
#define AF2_PB8_SPIM2_CLK   ((uint8_t)0x02) ///< SPIM2_CLK  function
#define AF2_PB9_SPIM2_SD0   ((uint8_t)0x02) ///< SPIM2_SD0  function
#define AF2_PB10_SPIM2_SD1  ((uint8_t)0x02) ///< SPIM2_SD1  function
#define AF2_PB11_SPIM2_CSN0 ((uint8_t)0x02) ///< SPIM2_CSN0 function
#define AF2_PB12_SPIM2_CSN1 ((uint8_t)0x02) ///< SPIM2_CSN1 function
#ifdef CONFIG_SOC_DARIC_MPW_A
#define AF2_PB13_SCIF_SCK ((uint8_t)0x02) ///< SCIF_SCK   function
#define AF2_PB14_SCIF_DAT ((uint8_t)0x02) ///< SCIF_DAT   function
#endif
#define AF2_PC0_SPIM2_CLK   ((uint8_t)0x02) ///< SPIM2_CLK  function
#define AF2_PC1_SPIM2_SD0   ((uint8_t)0x02) ///< SPIM2_SD0  function
#define AF2_PC2_SPIM2_SD1   ((uint8_t)0x02) ///< SPIM2_SD1  function
#define AF2_PC3_SPIM2_CSN0  ((uint8_t)0x02) ///< SPIM2_CSN0 function
#define AF2_PC4_SPIM2_CSN1  ((uint8_t)0x02) ///< SPIM2_CSN1 function
#define AF2_PC5_I2C3_SCL    ((uint8_t)0x02) ///< I2C3_SCL   function
#define AF2_PC6_I2C3_SDA    ((uint8_t)0x02) ///< I2C3_SDA   function
#define AF2_PC7_SDDC_D0     ((uint8_t)0x02) ///< SDDC_D0    function
#define AF2_PC8_SDDC_D1     ((uint8_t)0x02) ///< SDDC_D1    function
#define AF2_PC9_SDDC_D2     ((uint8_t)0x02) ///< SDDC_D2    function
#define AF2_PC10_SDDC_D3    ((uint8_t)0x02) ///< SDDC_D3    function
#define AF2_PC11_SDDC_CLK   ((uint8_t)0x02) ///< SDDC_CLK   function
#define AF2_PC12_SDDC_CMD   ((uint8_t)0x02) ///< SDDC_CMD   function
#define AF2_PD0_SPIS0_MOSI  ((uint8_t)0x02) ///< SPIS0_MOSI function
#define AF2_PD1_SPIS0_MISO  ((uint8_t)0x02) ///< SPIS0_MISO function
#define AF2_PD2_UART0_RX    ((uint8_t)0x02) ///< UART0_RX   function
#define AF2_PD3_UART0_TX    ((uint8_t)0x02) ///< UART0_TX   function
#define AF2_PD4_SPIS0_CLK   ((uint8_t)0x02) ///< SPIS0_CLK  function
#define AF2_PD5_SPIS0_CSN   ((uint8_t)0x02) ///< SPIS0_CSN  function
#define AF2_PD7_SPIM1_CLK   ((uint8_t)0x02) ///< SPIM1_CLK  function
#define AF2_PD8_SPIM1_SD0   ((uint8_t)0x02) ///< SPIM1_SD0  function
#define AF2_PD9_SPIM1_SD1   ((uint8_t)0x02) ///< SPIM1_SD1  function
#define AF2_PD10_SPIM1_CSN0 ((uint8_t)0x02) ///< SPIM1_CSN0 function
#define AF2_PD11_SPIM1_CSN1 ((uint8_t)0x02) ///< SPIM1_CSN1 function
#define AF2_PD12_SPIS1_CLK  ((uint8_t)0x02) ///< SPIS1_CLK  function
#define AF2_PD13_SPIS1_CSN  ((uint8_t)0x02) ///< SPIS1_CSN  function
#define AF2_PD14_SPIS1_MOSI ((uint8_t)0x02) ///< SPIS1_MOSI function
#define AF2_PD15_SPIS1_MISO ((uint8_t)0x02) ///< SPIS1_MISO function
#ifdef CONFIG_SOC_DARIC_NTO_A
#define AF2_PE0_I2C0_SCL   ((uint8_t)0x02) ///< I2C0_SCL function
#define AF2_PE1_I2C0_SDA   ((uint8_t)0x02) ///< I2C0_SDA function
#define AF2_PE2_UART1_RX   ((uint8_t)0x02) ///< UART1_RX function
#define AF2_PE3_UART1_TX   ((uint8_t)0x02) ///< UART1_TX function
#define AF2_PE6_SPIS1_CLK  ((uint8_t)0x02) ///< SPIS1_CLK function
#define AF2_PE7_SPIS1_CSN  ((uint8_t)0x02) ///< SPIS1_CSN function
#define AF2_PE8_SPIS1_MOSI ((uint8_t)0x02) ///< SPIS1_MOSI function
#define AF2_PE9_SPIS1_MISO ((uint8_t)0x02) ///< SPIS1_MISO function
#define AF2_PE10_I2C2_SCL  ((uint8_t)0x02) ///< I2C2_SCL function
#define AF2_PE11_I2C2_SDA  ((uint8_t)0x02) ///< I2C2_SDA function
#define AF2_PE12_UART1_RX  ((uint8_t)0x02) ///< UART1_RX function
#define AF2_PE13_UART1_TX  ((uint8_t)0x02) ///< UART1_TX function
#endif
/**
 * @}
 */

/** @defgroup AF3 function mappings
 * @{
 */
#define AF3_PA0_TIM0_PWM0  ((uint8_t)0x03) ///< TIM0_PWM0 function
#define AF3_PA1_TIM0_PWM1  ((uint8_t)0x03) ///< TIM0_PWM1 function
#define AF3_PA2_TIM0_PWM2  ((uint8_t)0x03) ///< TIM0_PWM2 function
#define AF3_PA3_TIM0_PWM3  ((uint8_t)0x03) ///< TIM0_PWM3 function
#define AF3_PB0_TIM1_PWM0  ((uint8_t)0x03) ///< TIM1_PWM0 function
#define AF3_PB1_TIM1_PWM1  ((uint8_t)0x03) ///< TIM1_PWM1 function
#define AF3_PB2_TIM1_PWM2  ((uint8_t)0x03) ///< TIM1_PWM2 function
#define AF3_PB3_TIM1_PWM3  ((uint8_t)0x03) ///< TIM1_PWM3 function
#define AF3_PC0_TIM2_PWM0  ((uint8_t)0x03) ///< TIM2_PWM0 function
#define AF3_PC1_TIM2_PWM1  ((uint8_t)0x03) ///< TIM2_PWM1 function
#define AF3_PC2_TIM2_PWM2  ((uint8_t)0x03) ///< TIM2_PWM2 function
#define AF3_PC3_TIM2_PWM3  ((uint8_t)0x03) ///< TIM2_PWM3 function
#define AF3_PD0_TIM3_PWM0  ((uint8_t)0x03) ///< TIM3_PWM0 function
#define AF3_PD1_TIM3_PWM1  ((uint8_t)0x03) ///< TIM3_PWM1 function
#define AF3_PD2_TIM3_PWM2  ((uint8_t)0x03) ///< TIM3_PWM2 function
#define AF3_PD3_TIM3_PWM3  ((uint8_t)0x03) ///< TIM3_PWM3 function
#define AF3_PD8_TIM0_PWM0  ((uint8_t)0x03) ///< TIM0_PWM0 function
#define AF3_PD9_TIM0_PWM1  ((uint8_t)0x03) ///< TIM0_PWM1 function
#define AF3_PD10_TIM0_PWM2 ((uint8_t)0x03) ///< TIM0_PWM2 function
#define AF3_PD11_TIM0_PWM3 ((uint8_t)0x03) ///< TIM0_PWM3 function
#ifdef CONFIG_SOC_DARIC_NTO_A
#define AF3_PE0_TIM1_PWM0  ((uint8_t)0x03) ///< TIM1_PWM0 function
#define AF3_PE1_TIM1_PWM1  ((uint8_t)0x03) ///< TIM1_PWM1 function
#define AF3_PE2_TIM1_PWM2  ((uint8_t)0x03) ///< TIM1_PWM2 function
#define AF3_PE3_TIM1_PWM3  ((uint8_t)0x03) ///< TIM1_PWM3 function
#define AF3_PE10_TIM3_PWM0 ((uint8_t)0x03) ///< TIM3_PWM0 function
#define AF3_PE11_TIM3_PWM1 ((uint8_t)0x03) ///< TIM3_PWM1 function
#define AF3_PE12_TIM3_PWM2 ((uint8_t)0x03) ///< TIM3_PWM2 function
#define AF3_PE13_TIM3_PWM3 ((uint8_t)0x03) ///< TIM3_PWM3 function
#endif
/**
 * @}
 */

/// @cond PRIVATE_OUTPUT
#define IS_PINMAP_PORT(PORT) (((PORT) == PORT_A) || ((PORT) == PORT_B) || ((PORT) == PORT_C) || ((PORT) == PORT_D))

#define IS_PINMAP_PIN(PIN)                                                                                                                                          \
    ((PIN == PIN_0) || (PIN == PIN_1) || (PIN == PIN_2) || (PIN == PIN_3) || (PIN == PIN_4) || (PIN == PIN_5) || (PIN == PIN_6) || (PIN == PIN_7) || (PIN == PIN_8) \
     || (PIN == PIN_9) || (PIN == PIN_10) || (PIN == PIN_11) || (PIN == PIN_12) || (PIN == PIN_13) || (PIN == PIN_14) || (PIN == PIN_15))

    /// @endcond

    /* Exported functions --------------------------------------------------------*/
    /* Initialization functions ***************************************************/
    /**
     *  @brief Initializes the pinmap according to the specified parameters in
     * the PINMAP_InitTypeDef structure.
     *  @param PINMAP_Init: Pointer to an array of PINMAP_InitTypeDef structures
     *          that contains the configuration information for each pin.
     *  @param PinCount: Number of elements in the PINMAP_Init.
     *  @return None
     */
    void HAL_PINMAP_init(PINMAP_InitTypeDef *pinMapArray, uint32_t size);

#ifdef __cplusplus
}
#endif
#endif // DARIC_HAL_PINMAP_H
