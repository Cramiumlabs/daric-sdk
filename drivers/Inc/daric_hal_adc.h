/**
 ******************************************************************************
 * @file    daric_hal_adc.h
 * @author  ADC Team
 * @brief   Header file of adc HAL module.
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

#ifndef DARIC_HAL_ADC_H
#define DARIC_HAL_ADC_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>

#include "daric_hal_def.h"

    /* Exported types ------------------------------------------------------------*/
    /**
     * @brief HAL ADC States enumeration.
     */
    typedef enum
    {
        HAL_ADC_STATE_RESET = 0, /*!< ADC is not yet Initialized */
        HAL_ADC_STATE_READY,     /*!< ADC Initialized and ready for use */
        HAL_ADC_STATE_BUSY,      /*!< ADC Sample process is ongoing */
        HAL_ADC_STATE_TIMEOUT    /*!< ADC Timeout state */
    } ADC_StateTypeDef;

    /**
     * @brief HAL ADC Error Code enumeration.
     */
    typedef enum
    {
        HAL_ADC_ERROR_NONE = 0,     /*!< ADC None Error */
        HAL_ADC_ERROR_INVALID_PARAM /*!< ADC Invalid Param Error */
    } ADC_ErrorTypeDef;

    /**
     * @brief HAL ADC Channel enumeration.
     */
    typedef enum
    {
        HAL_ADC_CHANNEL_0 = 0, /*!< ADC Channel 0 */
        HAL_ADC_CHANNEL_1,     /*!< ADC Channel 1 */
        HAL_ADC_CHANNEL_2,     /*!< ADC Channel 2 */
        HAL_ADC_CHANNEL_3,     /*!< ADC Channel 3 */
        HAL_ADC_CHANNEL_MAX    /*!< ADC Channle Max number */
    } ADC_ChannelTypeDef;

    /**
     * @brief HAL ADC Sample Mode enumeration.
     */
    typedef enum
    {
        HAL_ADC_SAMPLE_SINGLE = 0, /*!< ADC Sample Single Mode */
        HAL_ADC_SAMPLE_CONTINUOUS  /*!< ADC Sample Continuous Mode */
    } ADC_SampleModeTypeDef;

    /**
     * @brief HAL ADC Sample Type enumeration.
     */
    typedef enum
    {
        HAL_ADC_SAMPLE_TYPE_NONE = 0,   /*!< ADC Sample Type: Not Init */
        HAL_ADC_SAMPLE_TYPE_RAW,        /*!< ADC Sample Type: RAW */
        HAL_ADC_SAMPLE_TYPE_VOLTAGE,    /*!< ADC Sample Type: Voltage */
        HAL_ADC_SAMPLE_TYPE_TEMPERATURE /*!< ADC Sample Type: Temperature */
    } ADC_SampleTypeTypeDef;

    /**
     * @brief HAL ADC Callback ID enumeration definition
     */
    typedef enum
    {
        HAL_ADC_CONVERSION_COMPLETE_CB_ID = 0x00U, /*!< ADC conversion complete callback ID */
    } ADC_CallbackIDTypeDef;

    /* Exported Struct
     * ------------------------------------------------------------*/
    /**
     * @brief  ADC Initialization Structure definition
     */
    typedef struct
    {
        uint32_t              SampleRate; /*!< ADC Sample Rate */
        ADC_SampleModeTypeDef SampleMode; /*!< ADC Sample Mode */
    } ADC_InitTypeDef;

    /**
     * @brief  ADC WorkInfo Structure definition
     */
    typedef struct
    {
        ADC_SampleTypeTypeDef type; /*!< ADC Sample Type */
        ADC_ChannelTypeDef    chan; /*!< ADC Sample Channel */
    } ADC_WorkInfoTypeDef;

    /**
     * @brief  ADC handle Structure definition
     */
    typedef struct __ADC_HandleTypeDef
    {
        int                   ID;        /*!< ADC ID */
        ADC_InitTypeDef       Init;      /*!< ADC initialization parameters */
        ADC_WorkInfoTypeDef   WorkInfo;  /*!< ADC WorkInfo parameters */
        __IO ADC_StateTypeDef State;     /*!< ADC states */
        __IO ADC_ErrorTypeDef ErrorCode; /*!< ADC Error code */

        void (*ConvCplt_Callback)(struct __ADC_HandleTypeDef *hadc); /*!< ADC Conversion Complete Callback */
    } ADC_HandleTypeDef;

/* Exported constants --------------------------------------------------------*/
/** @defgroup ADC Instance ID define
 * @{
 */
#define ADC0_ID     (0)
#define ADC_MAX_NUM (1)
/**
 * @}
 */

/** @defgroup ADC SAMPLE RATE
 * @{
 */
#define ADC_SAMPLE_RATE_MIN     (12500)
#define ADC_SAMPLE_RATE_MAX     (114285)
#define ADC_SAMPLE_RATE_DEFAULT (100000)
    /**
     * @}
     */

    /* Exported Function ---------------------------------------------------------*/
    /** @addtogroup ADC_Exported_Functions_Group1
     *  @brief      Init/Deinit Peripheral Function
     * @{
     */
    /* Initialization and de-initialization functions  ****************************/
    HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef *hadc);
    HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef *hadc);

    /**
     * @}
     */

    /** @addtogroup ADC_Exported_Functions_Group2
     *  @brief      Register/Unregister ISR Function
     * @{
     */

    /**
     * @brief  HAL I2C Callback pointer definition
     */
    typedef void (*ADC_CallbackTypeDef)(ADC_HandleTypeDef *hadc);

    HAL_StatusTypeDef HAL_ADC_RegisterCallback(ADC_HandleTypeDef *hadc, ADC_CallbackIDTypeDef CallbackID, ADC_CallbackTypeDef pCallback);
    HAL_StatusTypeDef HAL_ADC_UnRegisterCallback(ADC_HandleTypeDef *hadc, ADC_CallbackIDTypeDef CallbackID);

    /**
     * @}
     */

    /** @addtogroup ADC_Exported_Functions_Group3
     *  @brief      Peripheral Control functions
     * @{
     */
    /* Peripheral Control functions ***********************************************/
    HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc, ADC_WorkInfoTypeDef WorkInfo);

    /**
     * @}
     */

    /** @addtogroup ADC_Exported_Functions_Group4
     *  @brief      IO operation functions
     * @{
     */
    /* IO operation functions *****************************************************/

    /* Blocking mode: Polling */
    HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc);
    HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc);
    /* Non-blocking mode: Interruption */
    HAL_StatusTypeDef HAL_ADC_Start_IT(ADC_HandleTypeDef *hadc);
    HAL_StatusTypeDef HAL_ADC_Stop_IT(ADC_HandleTypeDef *hadc);

    /* ADC conversion value from polling or interrupt sampling */
    uint32_t HAL_ADC_GetRaw(ADC_HandleTypeDef *hadc);
    uint32_t HAL_ADC_GetVoltage(ADC_HandleTypeDef *hadc);
    int32_t  HAL_ADC_GetTemperature(ADC_HandleTypeDef *hadc);

    /**
     * @}
     */

    /** @addtogroup ADC_Exported_Functions_Group5
     *  @brief      Peripheral State functions
     * @{
     */
    /* Peripheral State functions *************************************************/
    uint32_t HAL_ADC_GetState(ADC_HandleTypeDef *hadc);
    uint32_t HAL_ADC_GetError(ADC_HandleTypeDef *hadc);

    /**
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /* DARIC_HAL_ADC_H */
