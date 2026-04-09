/**
 ******************************************************************************
 * @file    daric_hal_adc.c
 * @author  ADC Team
 * @brief   ADC HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of the ADC
 *          peripheral:
 *           + Initialization and de-initialization functions
 *           + IO operation functions
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
#include "daric_hal_adc.h"
#include "daric_hal.h"
#include "daric_hal_nvic.h"
#include "daric_hal_udma_v3.h"
#include <stdio.h>
#include <strings.h>

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
#include "daric_ifram.h"
#endif

/// @cond PRIVATE_OUTPUT
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    __IO uint32_t RX_SADDR; /*!< offset: 0x0, RX transfer address of buffer */
    __IO uint32_t RX_SIZE;  /*!< offset: 0x4, RX transfer size of buffer */
    __IO uint32_t RX_CFG;   /*!< offset: 0x8, RX transfer configuration */
    __IO uint32_t DUMMY;
    __IO uint32_t CR_CFG; /*!< offset: 0x10, ADC Configuration Register */
} ADC_TypeDef;

typedef struct
{
    int32_t raw;         /*!< Raw Value of Temperature Channle */
    int32_t temperature; /* Temperature corresponding to the original value */
} ADC_TempTableTypedef;

/* Private defines -----------------------------------------------------------*/
#define ADC0_BASE             (0x50114000)
#define ADC0                  (ADC_TypeDef *)(ADC0_BASE)
#define ADC_UDMA_CLK_GATE_BIT (19)
#define ADC_RX_CHN_IRQ        (121)

#define HAL_ADC_LOCK()   HAL_INTERRUPT_SAVE_AREA HAL_LOCK
#define HAL_ADC_UNLOCK() HAL_UNLOCK

/* ADC IRQ Priority define */
#ifdef HAL_ADC_IRQ_PRIO
#define ADC_IRQ_PRIO HAL_ADC_IRQ_PRIO
#else
#define ADC_IRQ_PRIO 0
#endif

#ifdef HAL_ADC_IRQ_SUB_PRIO
#define ADC_IRQ_SUB_PRIO HAL_ADC_IRQ_SUB_PRIO
#else
#define ADC_IRQ_SUB_PRIO 0
#endif
/* Private macro -------------------------------------------------------------*/
#define ADC_RESOLUTION (10)
#define ADC_VALUE_MAX  ((0x1 << ADC_RESOLUTION) - 1)
#define ADC_VREF       (1208)
#define ADC_CLK_FREQ   (16000000)

#define ADC_SAMPLE_COUNT_MIN (14)
#define ADC_SAMPLE_COUNT_MAX (16)

#define ADC_SAMPLE_FREQ_MIN (200000)
#define ADC_SAMPLE_FREQ_MAX (1600000)

/* RX_CFG_REG */
#define ADC_CFG_CONTINUE_EN    (0x01 << 0)
#define ADC_CFG_CONST          (0x02 << 1)
#define ADC_CFG_RX_START       (0x01 << 4)
#define ADC_CFG_RX_CLEAR       (0x01 << 5)
#define ADC_CFG_RX_PENDING_STA (0x01 << 5)

/* CR_CFG_REG */
#define ADC_CR_ADC_ANA_REG_POS         (0)
#define ADC_CR_ADC_ANA_REG_MASK        (0xff)
#define ADC_CR_ADC_ANA_REG_CLEAR(ADCx) (ADCx->CR_CFG &= ~(ADC_CR_ADC_ANA_REG_MASK << ADC_CR_ADC_ANA_REG_POS))

#define ADC_CR_BANDGAP_CHOPPER_POS     (0)
#define ADC_CR_BANDGAP_CHOPPER_MASK    (0x1)
#define ADC_CR_BANDGAP_CHOPPER_ENABLE  (0x1 << ADC_CR_BANDGAP_CHOPPER_POS)
#define ADC_CR_BANDGAP_CHOPPER_DISABLE (~(ADC_CR_BANDGAP_CHOPPER_ENABLE))

#define ADC_CR_TEMP_RELATED_BUFFER_POS     (1)
#define ADC_CR_TEMP_RELATED_BUFFER_MASK    (0x1)
#define ADC_CR_TEMP_RELATED_BUFFER_ENABLE  (0x1 << ADC_CR_TEMP_RELATED_BUFFER_POS)
#define ADC_CR_TEMP_RELATED_BUFFER_DISABLE (~(ADC_CR_TEMP_RELATED_BUFFER_ENABLE))

#define ADC_CR_BANDGAP_BUFFER_POS     (2)
#define ADC_CR_BANDGAP_BUFFER_MASK    (0x1)
#define ADC_CR_BANDGAP_BUFFER_ENABLE  (0x1 << ADC_CR_BANDGAP_BUFFER_POS)
#define ADC_CR_BANDGAP_BUFFER_DISABLE (~(ADC_CR_BANDGAP_BUFFER_ENABLE))

#define ADC_CR_EXTERNAL_BUFFER_POS     (3)
#define ADC_CR_EXTERNAL_BUFFER_MASK    (0x1)
#define ADC_CR_EXTERNAL_BUFFER_ENABLE  (0x1 << ADC_CR_EXTERNAL_BUFFER_POS)
#define ADC_CR_EXTERNAL_BUFFER_DISABLE (~(ADC_CR_EXTERNAL_BUFFER_ENABLE))

#define ADC_CR_DATA_COUNT_POS         (8)
#define ADC_CR_DATA_COUNT_MASK        (0x1f)
#define ADC_CR_DATA_COUNT_CLEAR(ADCx) (ADCx->CR_CFG &= ~(ADC_CR_DATA_COUNT_MASK << ADC_CR_DATA_COUNT_POS))
#define ADC_CR_SET_DATA_COUNT(count)  ((count & ADC_CR_DATA_COUNT_MASK) << ADC_CR_DATA_COUNT_POS)

#define ADC_CR_TSEN_POS         (13)
#define ADC_CR_TSEN_ENABLE      (0x1 << ADC_CR_TSEN_POS)
#define ADC_CR_TSEN_CLEAR(ADCx) (ADCx->CR_CFG &= ~(ADC_CR_TSEN_ENABLE))

#define ADC_CR_ADCEN_POS         (14)
#define ADC_CR_ADCEN_ENABLE      (0x1 << ADC_CR_ADCEN_POS)
#define ADC_CR_ADCEN_CLEAR(ADCx) (ADCx->CR_CFG &= ~(ADC_CR_ADCEN_ENABLE))

#define ADC_CR_ADCRST_POS         (15)
#define ADC_CR_ADCRST_CLEAR(ADCx) (ADCx->CR_CFG &= ~(0x1 << ADC_CR_ADCRST_POS))
#define ADC_CR_ADCRST(ADCx)       (ADCx->CR_CFG |= 0x1 << ADC_CR_ADCRST_POS)

#define ADC_CR_CLKFD_POS         (16)
#define ADC_CR_CLKFD_MASK        (0xff)
#define ADC_CR_CLKFD_MIN         (10)
#define ADC_CR_CLKFD_MAX         (80)
#define ADC_CR_CLKFD_CLEAR(ADCx) (ADCx->CR_CFG &= ~(ADC_CR_CLKFD_MASK << ADC_CR_CLKFD_POS))
#define ADC_CR_SET_CLKFD(clkfd)  ((clkfd & ADC_CR_CLKFD_MASK) << ADC_CR_CLKFD_POS)

#define ADC_CR_ADC_SEL_POS         (24)
#define ADC_CR_ADC_SEL_MASK        (0x3)
#define ADC_CR_ADC_SEL_CLEAR(ADCx) (ADCx->CR_CFG &= ~((ADC_CR_ADC_SEL_MASK) << ADC_CR_ADC_SEL_POS))

#define ADC_CR_ADC_SEL_TEMPERATURE(ADCx)       ADC_CR_ADC_SEL_CLEAR(ADCx)
#define ADC_CR_ADC_SEL_EXT_VOLTAGE(ADCx, chan) (ADCx->CR_CFG |= (0x1 << ADC_CR_ADC_SEL_POS) | ((chan & ADC_CR_VINSEL_MASK) << ADC_CR_VINSEL_POS))

#define ADC_CR_VINSEL_POS         (26)
#define ADC_CR_VINSEL_MASK        (0x3)
#define ADC_CR_VINSEL_CLEAR(ADCx) (ADCx->CR_CFG &= ~((ADC_CR_VINSEL_MASK) << ADC_CR_VINSEL_POS))
#define ADC_CR_VINSEL(ADCx, chan)                                         \
    do                                                                    \
    {                                                                     \
        ADC_CR_VINSEL_CLEAR(ADCx);                                        \
        ADCx->CR_CFG |= (chan & ADC_CR_VINSEL_MASK) << ADC_CR_VINSEL_POS; \
    } while (0)

#define ADC_CR_REG_CLEAR(ADCx)          \
    do                                  \
    {                                   \
        ADC_CR_ADC_ANA_REG_CLEAR(ADCx); \
        ADC_CR_TSEN_CLEAR(ADCx);        \
        ADC_CR_ADCEN_CLEAR(ADCx);       \
        ADC_CR_ADCRST_CLEAR(ADCx);      \
        ADC_CR_ADC_SEL_CLEAR(ADCx);     \
        ADC_CR_VINSEL_CLEAR(ADCx);      \
    } while (0)

#define ADC_WAIT_RX_TIMEOUT (10)
/* Private variables
 * ---------------------------------------------------------*/
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
static uint32_t *adc_rx_buff[ADC_MAX_NUM] = { 0 };
#else
static uint32_t adc_rx_buff[ADC_MAX_NUM] __attribute__((section("ifram")));
#endif
static ADC_TypeDef *adc_reg[ADC_MAX_NUM] = { ADC0 };

static const ADC_TempTableTypedef ADC_Temperature_Table[] = {
    { 627, -40 }, //-40°C
    { 619, -35 }, //-35°C
    { 610, -30 }, //-30°C
    { 601, -25 }, //-25°C
    { 593, -20 }, //-20°C
    { 584, -15 }, //-15°C
    { 576, -10 }, //-10°C
    { 568, -5 },  //-5°C
    { 560, 0 },   // 0°C
    { 551, 5 },   // 5°C
    { 542, 10 },  // 10°C
    { 534, 15 },  // 15°C
    { 525, 20 },  // 20°C
    { 516, 25 },  // 25°C
    { 508, 30 },  // 30°C
    { 500, 35 },  // 35°C
    { 492, 40 },  // 40°C
    { 483, 45 },  // 45°C
    { 474, 50 },  // 50°C
    { 466, 55 },  // 55°C
    { 458, 60 },  // 60°C
    { 449, 65 },  // 65°C
    { 440, 70 },  // 70°C
    { 432, 75 },  // 75°C
    { 423, 80 },  // 80°C
    { 415, 85 },  // 85°C
    { 407, 90 },  // 90°C
    { 498, 95 },  // 95°C
    { 389, 100 }, // 100°C
    { 380, 105 }, // 105°C
    { 372, 110 }, // 110°C
    { 363, 115 }, // 115°C
    { 354, 120 }, // 120°C
    { 346, 125 }, // 125°C
};

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief  Configures the ADC sample rate by adjusting data count and clock
 * divider settings.
 *
 * This function calculates and sets the data count and clock divider values to
 * achieve the desired ADC sample rate. If an exact match for the requested
 * sample rate is found, it applies those settings. Otherwise, it configures the
 * ADC with the closest achievable sample rate.
 *
 * @param  ADCx        Pointer to the ADC_TypeDef structure that contains the
 *                     configuration information for the specified ADC module.
 * @param  SampleRate  Target sample rate in Hertz. Must be within the range
 *                     defined by ADC_SAMPLE_RATE_MIN and ADC_SAMPLE_RATE_MAX.
 *
 * @retval None
 */
static void HAL_ADC_SetSampleRate(ADC_TypeDef *ADCx, uint32_t SampleRate)
{
    int tmp_samplerate = 0, tmp_clk = 0, tmp_count = 0, tmp_clkfd = 0;
    int error_value = 0, error_value_min = SampleRate;

    ADC_CR_DATA_COUNT_CLEAR(ADCx);
    ADC_CR_CLKFD_CLEAR(ADCx);

    if (SampleRate == ADC_SAMPLE_RATE_MIN)
    {
        tmp_count = ADC_SAMPLE_COUNT_MAX;
        tmp_clkfd = ADC_CR_CLKFD_MIN;
    }
    else if (SampleRate == ADC_SAMPLE_RATE_MAX)
    {
        tmp_count = ADC_SAMPLE_COUNT_MIN;
        tmp_clkfd = ADC_CR_CLKFD_MAX;
    }
    else
    {
        for (int i = ADC_CR_CLKFD_MIN; i < ADC_CR_CLKFD_MAX; i++)
        {
            tmp_clk = ADC_CLK_FREQ / i;
            if (tmp_clk < ADC_SAMPLE_FREQ_MIN || tmp_clk > ADC_SAMPLE_FREQ_MAX)
            {
                continue;
            }

            for (int j = ADC_SAMPLE_COUNT_MIN; j <= ADC_SAMPLE_COUNT_MAX; j++)
            {
                tmp_samplerate = tmp_clk / j;
                if (tmp_samplerate == SampleRate)
                {
                    tmp_count = j;
                    tmp_clkfd = i;
                    goto out;
                }

                /* The actual sample rate must be larger than expected. */
                if (tmp_samplerate > SampleRate)
                {
                    error_value = tmp_samplerate - SampleRate;

                    /* Select the sample rate that is closest to the expected. */
                    if (error_value < error_value_min)
                    {
                        tmp_count       = j;
                        tmp_clkfd       = i;
                        error_value_min = error_value;
                    }
                }
            }
        }
    }

out:
    ADCx->CR_CFG |= ADC_CR_SET_CLKFD(tmp_clkfd) | ADC_CR_SET_DATA_COUNT(tmp_count);
    return;
}

/**
 * @brief  ADC receive interrupt handler.
 *
 * This function handles the ADC receive interrupt by retrieving the ADC
 * conversion value based on the current sample type configuration. It checks
 * the sample type in `WorkInfo` to determine whether to fetch raw data,
 * voltage, or temperature. If a conversion complete callback is registered, it
 * is invoked with the conversion result. Finally, the ADC state is updated to
 * indicate readiness.
 *
 * @param  arg   Pointer to the ADC handle (ADC_HandleTypeDef) for which the
 *               interrupt is being handled.
 *
 * @retval None
 */
static void HAL_ADC_RxIRQHandler(const void *arg)
{
    ADC_HandleTypeDef *hadc = (ADC_HandleTypeDef *)arg;

    if (hadc->ConvCplt_Callback)
    {
        hadc->ConvCplt_Callback(hadc);
    }
    hadc->State = HAL_ADC_STATE_READY;
}

static int ADC_WaitRX(ADC_TypeDef *ADCx, uint32_t timeout)
{
    uint32_t start_time = HAL_GetMs();

    while (1)
    {
        if ((HAL_UDMA_Busy((uint32_t)&ADCx->CR_CFG) == 0) && (ADCx->RX_SIZE == 0))
        {
            break;
        }

        if ((HAL_GetMs() - start_time) > timeout)
        {
            printf("%s: timeout!\n", __func__);
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}
/// @endcond

/* Exported functions --------------------------------------------------------*/
/** @addtogroup ADC_Exported_Functions_Group1
 * @brief      Init/Deinit Peripheral Function
 * @{
 */
/* Initialization and de-initialization functions  ****************************/
/**
 * @brief  Initializes the ADC peripheral according to the specified parameters
 *         in the ADC_InitStruct, including setting sample rate and connecting
 *         the ADC IRQ.
 *
 * @note   This function configures the ADC clock gate, registers the ADC IRQ,
 *         and sets the ADC sample rate. It also clears and resets the ADC
 *         controller register for a fresh start.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC instance.
 *
 * @retval HAL status. Returns HAL_OK if initialization is successful,
 *         or HAL_ERROR if there is a parameter or configuration error.
 */
HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef *hadc)
{
    ADC_TypeDef *ADCx = NULL;

    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    if (hadc->Init.SampleRate < ADC_SAMPLE_RATE_MIN || hadc->Init.SampleRate > ADC_SAMPLE_RATE_MAX)
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    if (hadc->State != HAL_ADC_STATE_RESET)
    {
        printf("%s: adc%d is already inited!\n", __func__, hadc->ID);
        HAL_ADC_UNLOCK();
        return HAL_OK;
    }

    hadc->State = HAL_ADC_STATE_BUSY;

    HAL_ADC_UNLOCK();

    ADCx = adc_reg[hadc->ID];

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    adc_rx_buff[hadc->ID] = IFRAM_CALLOC(sizeof(uint32_t));
    if (adc_rx_buff[hadc->ID] == NULL)
    {
        printf("%s: adc%d malloc ifram faild\n", __func__, hadc->ID);
        hadc->State = HAL_ADC_STATE_RESET;
        return HAL_ERROR;
    }
#endif

    /* 1. config uDMA clock gate for ADC */
    HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() | (1 << ADC_UDMA_CLK_GATE_BIT));

    /* 2. register ADC IRQ */
    HAL_NVIC_ConnectIRQ(ADC_RX_CHN_IRQ, ADC_IRQ_PRIO, ADC_IRQ_SUB_PRIO, HAL_ADC_RxIRQHandler, (void *)hadc, 0);

    /* 3. Clear CR_REG and Reset ADC */
    ADC_CR_REG_CLEAR(ADCx);
    ADC_CR_ADCRST(ADCx);

    /* 4. Set ADC Sample Rate */
    HAL_ADC_SetSampleRate(ADCx, hadc->Init.SampleRate);

    hadc->State     = HAL_ADC_STATE_READY;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;

    printf("%s: ADC%d init Done!\n", __func__, hadc->ID);
    return HAL_OK;
}

/**
 * @brief  Deinitializes the ADC peripheral, resetting it to its default state
 *         and disabling associated resources such as IRQ and clock gate.
 *
 * @note   This function performs a reset on the ADC, disables the ADC IRQ,
 *         and clears the clock gate for the ADC. After deinitialization, the
 *         ADC is in the HAL_ADC_STATE_RESET state.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that represents
 *         the configuration information for the specified ADC instance.
 *
 * @retval HAL status. Returns HAL_OK if deinitialization is successful,
 *         or HAL_ERROR if the ADC handle or instance ID is invalid.
 */
HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef *hadc)
{
    ADC_TypeDef *ADCx = NULL;

    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    ADCx = adc_reg[hadc->ID];

    /* 1. Reset ADC and stop transfer */
    ADC_CR_ADCRST(ADCx);

    /* 2. Disable ADC IRQ */
    HAL_NVIC_ClearPendingIRQ(ADC_RX_CHN_IRQ);
    HAL_NVIC_DisableIRQ(ADC_RX_CHN_IRQ);

    /* 3. clear uDMA clock gate for ADC */
    HAL_UDMA_CG_Set(HAL_UDMA_CG_Get() & (~(1 << ADC_UDMA_CLK_GATE_BIT)));

    hadc->State = HAL_ADC_STATE_RESET;

    HAL_ADC_UNLOCK();

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    IframMgr_Free(adc_rx_buff[hadc->ID]);
#endif

    printf("%s: ADC%d Deinit Done!\n", __func__, hadc->ID);
    return HAL_OK;
}
/**
 * @}
 */

/* Callbacks Register/UnRegister functions  ***********************************/
/**
 * @brief  Registers a callback function for the specified ADC event.
 *
 * @note   This function allows the user to set a custom callback function for
 *         specific ADC events. Currently, it supports the callback for the
 *         conversion complete event (HAL_ADC_CONVERSION_COMPLETE_CB_ID).
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 * @param  CallbackID Specifies the ADC event for which the callback is being
 *         registered. This can be one of the following:
 *         - HAL_ADC_CONVERSION_COMPLETE_CB_ID: Callback for ADC conversion
 *         complete.
 * @param  pCallback Pointer to the callback function to be registered.
 * @param  Callback_Arg Pointer to the argument that will be passed to the
 *         callback function.
 *
 * @retval HAL status. Returns HAL_OK if the callback is successfully
 *         registered, or HAL_ERROR if the ADC handle is invalid or the provided
 *         CallbackID is not recognized.
 */
HAL_StatusTypeDef HAL_ADC_RegisterCallback(ADC_HandleTypeDef *hadc, ADC_CallbackIDTypeDef CallbackID, ADC_CallbackTypeDef pCallback)
{
    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    /* Register Callback function according to the CallbackID */
    switch (CallbackID)
    {
        case HAL_ADC_CONVERSION_COMPLETE_CB_ID:
            hadc->ConvCplt_Callback = pCallback;
            break;
        default:
            printf("%s: invalid CallbackID\n", __func__);
            hadc->ErrorCode = HAL_ADC_ERROR_INVALID_PARAM;
            HAL_ADC_UNLOCK();
            return HAL_ERROR;
    }

    HAL_ADC_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Unregisters a callback function for the specified ADC event.
 *
 * @note   This function allows the user to remove a previously registered
 *         callback function for a specific ADC event. Currently, it supports
 *         unregistering the callback for the conversion complete event
 *         (HAL_ADC_CONVERSION_COMPLETE_CB_ID).
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 * @param  CallbackID Specifies the ADC event for which the callback is being
 *         unregistered. This can be one of the following:
 *         - HAL_ADC_CONVERSION_COMPLETE_CB_ID: Callback for ADC conversion
 *         complete.
 *
 * @retval HAL status. Returns HAL_OK if the callback is successfully
 *         unregistered, or HAL_ERROR if the ADC handle is invalid or the
 * provided CallbackID is not recognized.
 */
HAL_StatusTypeDef HAL_ADC_UnRegisterCallback(ADC_HandleTypeDef *hadc, ADC_CallbackIDTypeDef CallbackID)
{
    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    /* Unregister Callback function according to the CallbackID */
    switch (CallbackID)
    {
        case HAL_ADC_CONVERSION_COMPLETE_CB_ID:
            hadc->ConvCplt_Callback = NULL;
            break;
        default:
            printf("%s: invalid CallbackID\n", __func__);
            hadc->ErrorCode = HAL_ADC_ERROR_INVALID_PARAM;
            HAL_ADC_UNLOCK();
            return HAL_ERROR;
    }

    HAL_ADC_UNLOCK();

    return HAL_OK;
}

/** @addtogroup ADC_Exported_Functions_Group2
 *  @brief      Peripheral Control functions
 * @{
 */
/* Peripheral Control functions ***********************************************/
/**
 * @brief  Configures the specified ADC channel for operation.
 *
 * @note   This function updates the ADC configuration to work with a specified
 * channel and operation type as defined in the WorkInfo parameter.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 * @param  WorkInfo Specifies the work type and channel to be configured.
 *         - WorkInfo.type: Type of ADC operation (e.g., single or continuous
 * mode).
 *         - WorkInfo.chan: ADC channel number to be configured.
 *
 * @retval HAL status. Returns HAL_OK if the channel configuration is
 * successfully applied, or HAL_ERROR if the ADC handle is invalid.
 */
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef *hadc, ADC_WorkInfoTypeDef WorkInfo)
{
    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    if (hadc->State == HAL_ADC_STATE_BUSY)
    {
        HAL_ADC_UNLOCK();
        return HAL_BUSY;
    }

    hadc->WorkInfo.type = WorkInfo.type;
    hadc->WorkInfo.chan = WorkInfo.chan;

    HAL_ADC_UNLOCK();

    return HAL_OK;
}

/**
 * @}
 */

/** @addtogroup ADC_Exported_Functions_Group3
 *  @brief      IO operation functions
 * @{
 */
/* IO operation functions *****************************************************/

/* Blocking mode: Polling */
/**
 * @brief  Starts the ADC conversion process based on the specified channel and
 *         settings.
 *
 * @note   This function initiates the ADC conversion for either single or
 *         continuous sampling mode, depending on the configuration in
 *         hadc->Init.SampleMode. It configures ADC parameters, clears previous
 * data, and enables necessary ADC components for the selected work type.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval HAL status:
 *         - HAL_OK if the ADC starts successfully.
 *         - HAL_ERROR if the ADC handle or configuration is invalid.
 *         - HAL_BUSY if the ADC is already busy.
 *         - HAL_TIMEOUT if ADC conversion does not complete within the defined
 *           timeout.
 */
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef *hadc)
{
    ADC_TypeDef *ADCx = NULL;
    int          ret  = 0;
    uint32_t     cfg  = 0;
    uint32_t    *data = NULL;

    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    if (hadc->WorkInfo.type == HAL_ADC_SAMPLE_TYPE_NONE)
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    if (hadc->State != HAL_ADC_STATE_READY)
    {
        HAL_ADC_UNLOCK();
        printf("%s: adc%d is not ready!\n", __func__, hadc->ID);
        return HAL_BUSY;
    }
    else
    {
        hadc->State = HAL_ADC_STATE_BUSY;
    }

    HAL_ADC_UNLOCK();

    ADCx = adc_reg[hadc->ID];
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    *adc_rx_buff[hadc->ID] = 0;
    data                   = adc_rx_buff[hadc->ID];
#else
    adc_rx_buff[hadc->ID] = 0;
    data                  = &adc_rx_buff[hadc->ID];
#endif

    HAL_NVIC_DisableIRQ(ADC_RX_CHN_IRQ);

    /* 1. Clear CR_REG and Reset ADC */
    ADC_CR_REG_CLEAR(ADCx);
    ADC_CR_ADCRST(ADCx);

    /* 2. Config ADC according to WorkInfo */
    /* 2.1: Enable ADC_TSEN */
    ADCx->CR_CFG |= ADC_CR_TSEN_ENABLE;
    /* Delay: 10us */
    HAL_DelayUs(10);

    /* Config ADCANA_CTR 0, 2 */
    cfg = ADC_CR_BANDGAP_CHOPPER_ENABLE | ADC_CR_BANDGAP_BUFFER_ENABLE;

    if (hadc->WorkInfo.type == HAL_ADC_SAMPLE_TYPE_RAW || hadc->WorkInfo.type == HAL_ADC_SAMPLE_TYPE_VOLTAGE)
    {
        /* 2.2: Config ADC_ANA_CTR 3 */
        cfg |= ADC_CR_EXTERNAL_BUFFER_ENABLE;
        ADCx->CR_CFG |= cfg;
        /* Delay: 0us */
        HAL_DelayUs(0);

        /* 2.3: Enable ADC_EN */
        ADCx->CR_CFG |= ADC_CR_ADCEN_ENABLE;
        /* Delay: 90us */
        HAL_DelayUs(90);

        /* 2.4: Select adc_sel to external voltage and Select Input Channel */
        ADC_CR_ADC_SEL_EXT_VOLTAGE(ADCx, hadc->WorkInfo.chan);
    }
    else
    {
        /* 2.2: Config ADC_ANA_CTR 1 */
        cfg |= ADC_CR_TEMP_RELATED_BUFFER_ENABLE;
        ADCx->CR_CFG |= cfg;
        /* Delay: 0us */
        HAL_DelayUs(0);

        /* 2.3: Enable ADC_EN */
        ADCx->CR_CFG |= ADC_CR_ADCEN_ENABLE;
        /* Delay: 90us */
        HAL_DelayUs(90);

        /* 2.4: Select adc_sel to temperature voltage */
        ADC_CR_ADC_SEL_TEMPERATURE(ADCx);
    }

    /* 3. Clear RX Channel and Buffer */
    ADCx->RX_CFG |= ADC_CFG_RX_CLEAR;

    /* 4. Enqueue ADC RX Channel */
    if (hadc->Init.SampleMode == HAL_ADC_SAMPLE_SINGLE)
    {
        cfg = ADC_CFG_CONST;
    }
    else
    {
        cfg = ADC_CFG_CONST | ADC_CFG_CONTINUE_EN;
    }
    HAL_UDMA_Enqueue((uint32_t) & (ADCx->RX_SADDR), (uint32_t)data, sizeof(uint32_t), cfg);

    /* 5. Wait RX Channel Done*/
    if (hadc->Init.SampleMode == HAL_ADC_SAMPLE_SINGLE)
    {
        ret = ADC_WaitRX(ADCx, ADC_WAIT_RX_TIMEOUT);
        if (ret != HAL_OK)
        {
            hadc->State = HAL_ADC_STATE_READY;
            return HAL_TIMEOUT;
        }
    }

    hadc->State = HAL_ADC_STATE_READY;

    return HAL_OK;
}

/**
 * @brief  Stops the ADC conversion process and resets the ADC peripheral.
 *
 * @note   This function clears the RX channel, resets the ADC, and disables
 *         the ADC and Bandgap to stop any ongoing ADC conversion and conserve
 *         power when not in use.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval HAL status:
 *         - HAL_OK if the ADC stops successfully.
 *         - HAL_ERROR if the ADC handle is NULL or the ID is invalid.
 */
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef *hadc)
{
    ADC_TypeDef *ADCx = NULL;

    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    ADCx = adc_reg[hadc->ID];

    /* 1. Clear RX Channel */
    ADCx->RX_CFG |= ADC_CFG_RX_CLEAR;
    ADCx->RX_CFG &= ~(ADC_CFG_RX_START);
    if (hadc->Init.SampleMode == HAL_ADC_SAMPLE_CONTINUOUS)
    {
        ADCx->RX_CFG &= ~(ADC_CFG_CONTINUE_EN);
    }

    /* 2. Reset ADC */
    ADC_CR_ADCRST(ADCx);

    /* 3. Disable ADC and Bandgap */
    ADC_CR_TSEN_CLEAR(ADCx);
    ADC_CR_ADCEN_CLEAR(ADCx);

    hadc->State = HAL_ADC_STATE_READY;

    HAL_ADC_UNLOCK();

    return HAL_OK;
}

/* Non-blocking mode: Interruption */
/**
 * @brief  Starts ADC conversion with interrupt.
 *
 * @note   This function configures the ADC for conversion based on the
 *         specified channel and mode, and enables interrupt-based handling
 *         of the conversion complete event.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval HAL status:
 *         - HAL_OK if the ADC starts successfully.
 *         - HAL_ERROR if the ADC handle is NULL, the ID is invalid, or
 *           the sample type is not specified.
 *         - HAL_BUSY if the ADC is currently busy.
 */
HAL_StatusTypeDef HAL_ADC_Start_IT(ADC_HandleTypeDef *hadc)
{
    ADC_TypeDef *ADCx = NULL;
    uint32_t     cfg  = 0;
    uint32_t    *data = NULL;

    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    if (hadc->WorkInfo.type == HAL_ADC_SAMPLE_TYPE_NONE)
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    if (hadc->State != HAL_ADC_STATE_READY)
    {
        HAL_ADC_UNLOCK();
        printf("%s: adc%d is not ready!\n", __func__, hadc->ID);
        return HAL_BUSY;
    }
    else
    {
        hadc->State = HAL_ADC_STATE_BUSY;
    }

    HAL_ADC_UNLOCK();

    ADCx = adc_reg[hadc->ID];
#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    *adc_rx_buff[hadc->ID] = 0;
    data                   = adc_rx_buff[hadc->ID];
#else
    adc_rx_buff[hadc->ID] = 0;
    data                  = &adc_rx_buff[hadc->ID];
#endif

    HAL_NVIC_ClearPendingIRQ(ADC_RX_CHN_IRQ);
    HAL_NVIC_EnableIRQ(ADC_RX_CHN_IRQ);

    /* 1. Reset ADC */
    ADC_CR_REG_CLEAR(ADCx);
    ADC_CR_ADCRST(ADCx);

    /* 2. Config ADC according to WorkInfo */
    /* 2.1: Enable ADC_TSEN */
    ADCx->CR_CFG |= ADC_CR_TSEN_ENABLE;
    /* Delay: 10us */
    HAL_DelayUs(10);

    /* Config ADCANA_CTR 0, 2 */
    cfg = ADC_CR_BANDGAP_CHOPPER_ENABLE | ADC_CR_BANDGAP_BUFFER_ENABLE;

    if (hadc->WorkInfo.type == HAL_ADC_SAMPLE_TYPE_RAW || hadc->WorkInfo.type == HAL_ADC_SAMPLE_TYPE_VOLTAGE)
    {
        /* 2.2: Config ADC_ANA_CTR 3 */
        cfg |= ADC_CR_EXTERNAL_BUFFER_ENABLE;
        ADCx->CR_CFG |= cfg;
        /* Delay: 0us */
        HAL_DelayUs(0);

        /* 2.3: Enable ADC_EN */
        ADCx->CR_CFG |= ADC_CR_ADCEN_ENABLE;
        /* Delay: 90us */
        HAL_DelayUs(90);

        /* 2.4: Select adc_sel to external voltage */
        ADC_CR_ADC_SEL_EXT_VOLTAGE(ADCx, hadc->WorkInfo.chan);
        // ADC_CR_VINSEL(ADCx, hadc->WorkInfo.chan);
    }
    else
    {
        /* 2.2: Config ADC_ANA_CTR 1 */
        cfg |= ADC_CR_TEMP_RELATED_BUFFER_ENABLE;
        ADCx->CR_CFG |= cfg;
        /* Delay: 0us */
        HAL_DelayUs(0);

        /* 2.3: Enable ADC_EN */
        ADCx->CR_CFG |= ADC_CR_ADCEN_ENABLE;
        /* Delay: 90us */
        HAL_DelayUs(90);

        /* 2.4: Select adc_sel to temperature voltage */
        ADC_CR_ADC_SEL_TEMPERATURE(ADCx);
    }

    /* 3. Clear RX Channel and Buffer */
    ADCx->RX_CFG |= ADC_CFG_RX_CLEAR;

    /* 4. Enqueue ADC RX Channel */
    if (hadc->Init.SampleMode == HAL_ADC_SAMPLE_SINGLE)
    {
        cfg = ADC_CFG_CONST;
    }
    else
    {
        cfg = ADC_CFG_CONST | ADC_CFG_CONTINUE_EN;
    }
    HAL_UDMA_Enqueue((uint32_t) & (ADCx->RX_SADDR), (uint32_t)data, sizeof(uint32_t), cfg);

    return HAL_OK;
}

/**
 * @brief  Stops ADC conversion with interrupt.
 *
 * @note   This function stops any ongoing ADC conversion, disables the
 *         associated interrupt, and clears the RX channel configuration.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval HAL status:
 *         - HAL_OK if the ADC stops successfully.
 *         - HAL_ERROR if the ADC handle is NULL or the ID is invalid.
 */
HAL_StatusTypeDef HAL_ADC_Stop_IT(ADC_HandleTypeDef *hadc)
{
    ADC_TypeDef *ADCx = NULL;

    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    HAL_ADC_LOCK();

    ADCx = adc_reg[hadc->ID];

    HAL_NVIC_ClearPendingIRQ(ADC_RX_CHN_IRQ);
    HAL_NVIC_DisableIRQ(ADC_RX_CHN_IRQ);

    /* 1. Clear RX Channel */
    ADCx->RX_CFG |= ADC_CFG_RX_CLEAR;
    ADCx->RX_CFG &= ~(ADC_CFG_RX_START);
    if (hadc->Init.SampleMode == HAL_ADC_SAMPLE_CONTINUOUS)
    {
        ADCx->RX_CFG &= ~(ADC_CFG_CONTINUE_EN);
    }

    /* 2. Reset ADC */
    ADC_CR_ADCRST(ADCx);

    /* 3. Disable ADC and Bandgap */
    ADC_CR_TSEN_CLEAR(ADCx);
    ADC_CR_ADCEN_CLEAR(ADCx);

    hadc->State = HAL_ADC_STATE_READY;

    HAL_ADC_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Gets the raw ADC conversion value.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval uint32_t ADC raw data value.
 */
uint32_t HAL_ADC_GetRaw(ADC_HandleTypeDef *hadc)
{
    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return 0;
    }

#ifdef HAL_IFRAMMGR_MODULE_ENABLED
    return *adc_rx_buff[hadc->ID];
#else
    return adc_rx_buff[hadc->ID];
#endif
}

/**
 * @brief  Converts the ADC raw value to a corresponding voltage.
 *
 * @note   The function uses the reference voltage (ADC_VREF) and maximum
 *         ADC value (ADC_VALUE_MAX) for conversion.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval uint32_t The converted voltage value in millivolts.
 */
uint32_t HAL_ADC_GetVoltage(ADC_HandleTypeDef *hadc)
{
    int32_t voltage = 0;
    int32_t raw     = 0;

    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    raw = HAL_ADC_GetRaw(hadc);

    voltage = raw * ADC_VREF / ADC_VALUE_MAX;

    return voltage;
}

/**
 * @brief  Converts the ADC raw value to a corresponding temperature.
 *
 * @note   This function compares the ADC value with entries in an
 *         ADC_Temperature_Table to determine the corresponding temperature.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval int32_t The temperature in degrees Celsius, or HAL_ERROR if the
 *         raw does not match any entry in the temperature table.
 */
int32_t HAL_ADC_GetTemperature(ADC_HandleTypeDef *hadc)
{
    int     i           = 0;
    int32_t temperature = 0;
    int32_t raw         = 0;

    if (hadc == NULL || !(hadc->ID < ADC_MAX_NUM))
    {
        return HAL_ERROR;
    }

    raw = HAL_ADC_GetRaw(hadc);

    int32_t num = sizeof(ADC_Temperature_Table) / sizeof(ADC_TempTableTypedef);

    for (i = 0; i < num; i++)
    {
        if (raw >= ADC_Temperature_Table[i].raw)
        {
            temperature = ADC_Temperature_Table[i].temperature;
            break;
        }
    }

    if (i == num)
    {
        printf("raw is invalid, %ld, temperature over 125℃\n", raw);
        return HAL_ERROR;
    }

    return temperature;
}

/**
 * @}
 */

/** @addtogroup ADC_Exported_Functions_Group4
 *  @brief      Peripheral State functions
 * @{
 */
/* Peripheral State functions *************************************************/
/**
 * @brief  Retrieves the current state of the ADC.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval uint32_t Current state of the ADC, represented by a predefined
 *         state value.
 */
uint32_t HAL_ADC_GetState(ADC_HandleTypeDef *hadc)
{
    return hadc->State;
}

/**
 * @brief  Retrieves the error code of the last ADC operation.
 *
 * @param  hadc Pointer to an ADC_HandleTypeDef structure that contains
 *         the configuration information for the specified ADC.
 *
 * @retval uint32_t Error code representing the last encountered error
 *         during an ADC operation.
 */
uint32_t HAL_ADC_GetError(ADC_HandleTypeDef *hadc)
{
    return hadc->ErrorCode;
}

/**
 * @}
 */
