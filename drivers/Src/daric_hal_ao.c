/**
 ******************************************************************************
 * @file    daric_hal_ao.c
 * @author  AO Team
 * @brief   AO HAL module driver.
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
#include <stdio.h>
#ifdef HAL_AO_MODULE_ENABLED

extern uint32_t _aoram_boot_info_, _aoram_user_start_, _aoram_user_end_;

/* Private defines -----------------------------------------------------------*/
#define AO_RAM_BASE_ADDR      (0x50300000)
#define AO_RAM_SIZE           (0x4000)
#define AO_RAM_BOOT_INFO_SIZE (0x100) // Depends on the size of the section 'aoram_boot_info' in 'xx.ld'.
#define AO_RAM_USER_SIZE      (AO_RAM_SIZE - AO_RAM_BOOT_INFO_SIZE)

#if (AO_RAM_BOOT_INFO_SIZE % 4 != 0)
#error "AO_RAM_BOOT_INFO_SIZE alignment error!"
#elif (AO_RAM_USER_SIZE % 4 != 0)
#error "AO_RAM_BOOT_INFO_SIZE alignment error!"
#endif

/* Boot mode flag */
#define AO_BOOT_NORMAL_MODE_VALUE         (0x5AA55AA5)
#define AO_BOOT_FAST_MODE_VALUE           (0xA55AA55A)
#define AO_BOOT_AFTER_DOWNLOAD_MODE_VALUE (0xB66BB66B)
#define AO_BOOT_FOTA_MODE_VALUE           (0xC77CC77C)

#define AO_BACK_UP_REG_LENGTH (32u)

#ifdef HAL_AOINT_IRQ_PRIO
#define AOINT_IRQ_PRIO HAL_AOINT_IRQ_PRIO
#else
#define AOINT_IRQ_PRIO 0
#endif

#ifdef HAL_AOINT_IRQ_SUB_PRIO
#define AOINT_IRQ_SUB_PRIO HAL_AOINT_IRQ_SUB_PRIO
#else
#define AOINT_IRQ_SUB_PRIO 0
#endif

#ifdef HAL_AOWKUPINT_IRQ_PRIO
#define AOWKUPINT_IRQ_PRIO HAL_AOWKUPINT_IRQ_PRIO
#else
#define AOWKUPINT_IRQ_PRIO 0
#endif

#ifdef HAL_AOWKUPINT_IRQ_SUB_PRIO
#define AOWKUPINT_IRQ_SUB_PRIO HAL_AOWKUPINT_IRQ_SUB_PRIO
#else
#define AOWKUPINT_IRQ_SUB_PRIO 0
#endif

/* Private macro -------------------------------------------------------------*/
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
// C11 或更高版本
#define STATIC_ASSERT _Static_assert
#elif defined(__cplusplus) && __cplusplus >= 201103L
// C++11 或更高版本
#define STATIC_ASSERT static_assert
#elif defined(__GNUC__) || defined(__clang__)
// GCC 或 Clang 编译器
#define STATIC_ASSERT _Static_assert
#elif defined(_MSC_VER) && _MSC_VER >= 1600
// MSVC 2010 或更高版本
#define STATIC_ASSERT static_assert
#else
// 其他编译器回退方案
#define STATIC_ASSERT(cond, msg) typedef char static_assertion_##__LINE__[(cond) ? 1 : -1]
#endif

/* Private typedef -----------------------------------------------------------*/
#pragma pack(push, 1)
typedef struct
{
    uint32_t dummy;
    uint32_t sleepModeFlag;
    uint32_t bootModeFlag;
} AO_BootInfoStruct;
#pragma pack(pop)

typedef union
{
    uint8_t           buf[AO_RAM_BOOT_INFO_SIZE];
    AO_BootInfoStruct bootInfo;
} AO_BootInfoUnion;
STATIC_ASSERT(sizeof(AO_BootInfoUnion) == AO_RAM_BOOT_INFO_SIZE, "AO_BootInfoUnion size out of range");

typedef struct
{
    uint32_t InterruptMask;
    void (*Callback)(void *param);
    void *param;
} AO_CallbackTypeDef;
#define HAL_AO_IRQ_SRC_NUM    6
#define HAL_AO_WAKEUP_SRC_NUM 8

/* Private variables ---------------------------------------------------------*/
static AO_BootInfoUnion  *s_boot_info_ptr = (AO_BootInfoUnion *)&_aoram_boot_info_;
volatile uint8_t          aoram_user_stack[AO_RAM_USER_SIZE] __attribute__((section("aoram_user")));
static AO_CallbackTypeDef AO_Callback[HAL_AO_IRQ_SRC_NUM]          = { 0 };
static AO_CallbackTypeDef AO_WakeupCallback[HAL_AO_WAKEUP_SRC_NUM] = { 0 };

static int32_t AO_BootFlag_Set(HAL_AO_BootModeStruct bootModeStruct)
{
    int32_t                ret       = -1;
    uint32_t               value     = AO_BOOT_NORMAL_MODE_VALUE;
    HAL_AO_BootModeTypeDef sleepMode = bootModeStruct.SleepMode;
    HAL_AO_BootModeTypeDef bootMode  = bootModeStruct.BootMode;
    switch (sleepMode)
    {
        case HAL_AO_BOOT_NORMAL_MODE:
            value = AO_BOOT_NORMAL_MODE_VALUE;
            ret   = 0;
            break;
        case HAL_AO_BOOT_FAST_MODE:
            value = AO_BOOT_FAST_MODE_VALUE;
            ret   = 0;
            break;
        case HAL_AO_BOOT_AFTER_DOWNLOAD_MODE:
            value = AO_BOOT_AFTER_DOWNLOAD_MODE_VALUE;
            ret   = 0;
            break;
        case HAL_AO_BOOT_FOTA_MODE:
            value = AO_BOOT_FOTA_MODE_VALUE;
            ret   = 0;
            break;
        default:
            break;
    }
    s_boot_info_ptr->bootInfo.sleepModeFlag = value;

    switch (bootMode)
    {
        case HAL_AO_BOOT_NORMAL_MODE:
            value = AO_BOOT_NORMAL_MODE_VALUE;
            ret   = 0;
            break;
        case HAL_AO_BOOT_FAST_MODE:
            value = AO_BOOT_FAST_MODE_VALUE;
            ret   = 0;
            break;
        case HAL_AO_BOOT_AFTER_DOWNLOAD_MODE:
            value = AO_BOOT_AFTER_DOWNLOAD_MODE_VALUE;
            ret   = 0;
            break;
        case HAL_AO_BOOT_FOTA_MODE:
            value = AO_BOOT_FOTA_MODE_VALUE;
            ret   = 0;
            break;
        default:
            break;
    }
    s_boot_info_ptr->bootInfo.bootModeFlag = value;
    return ret;
}

static HAL_AO_BootModeStruct AO_BootFlag_Get(void)
{
    uint32_t              sleepModeFlag  = s_boot_info_ptr->bootInfo.sleepModeFlag;
    uint32_t              bootModeFlag   = s_boot_info_ptr->bootInfo.bootModeFlag;
    HAL_AO_BootModeStruct bootModeStruct = { 0 };
    switch (sleepModeFlag)
    {
        case AO_BOOT_NORMAL_MODE_VALUE:
            bootModeStruct.SleepMode = HAL_AO_BOOT_NORMAL_MODE;
            break;
        case AO_BOOT_FAST_MODE_VALUE:
            bootModeStruct.SleepMode = HAL_AO_BOOT_FAST_MODE;
            break;
        case AO_BOOT_AFTER_DOWNLOAD_MODE_VALUE:
            bootModeStruct.SleepMode = HAL_AO_BOOT_AFTER_DOWNLOAD_MODE;
            break;
        case AO_BOOT_FOTA_MODE_VALUE:
            bootModeStruct.SleepMode = HAL_AO_BOOT_FOTA_MODE;
            break;
        default:
            bootModeStruct.SleepMode = HAL_AO_BOOT_UNDEFINED_MODE;
            break;
    }

    switch (bootModeFlag)
    {
        case AO_BOOT_NORMAL_MODE_VALUE:
            bootModeStruct.BootMode = HAL_AO_BOOT_NORMAL_MODE;
            break;
        case AO_BOOT_FAST_MODE_VALUE:
            bootModeStruct.BootMode = HAL_AO_BOOT_FAST_MODE;
            break;
        case AO_BOOT_AFTER_DOWNLOAD_MODE_VALUE:
            bootModeStruct.BootMode = HAL_AO_BOOT_AFTER_DOWNLOAD_MODE;
            break;
        case AO_BOOT_FOTA_MODE_VALUE:
            bootModeStruct.BootMode = HAL_AO_BOOT_FOTA_MODE;
            break;
        default:
            bootModeStruct.BootMode = HAL_AO_BOOT_UNDEFINED_MODE;
            break;
    }
    return bootModeStruct;
}

uint32_t HAL_AO_BootInfo_Length(HAL_AO_InfoTypeDef infoType)
{
    uint32_t length = 0;

    switch (infoType)
    {
        case HAL_AO_BOOT_INFO_TYPE_BOOTMODE:
            length = sizeof(s_boot_info_ptr->bootInfo.bootModeFlag);
            break;
        default:
            break;
    }

    return length;
}

static void AO_SetConfig(AON_TypeDef *Instance, AO_InitTypeDef *Init)
{
    /* Clock config */
    Instance->CLK32K_SEL = Init->ClockSelection;
    Instance->OSC_CR     = Init->Oscillator;
    Instance->CLK1HZ_FD  = Init->ClockDivision;
    /* Wakeup interrupt cofig */
    Instance->WKUP_INTEN = Init->InterruptMask;
    /* Power reset config */
    Instance->RSTCR_MASK = Init->ResetMask;
    /* AO GPIO config */
    Instance->IOX   = Init->AltFunction;
    Instance->PADPU = Init->PullUpConfig;
}

/**
 * @brief  Handle PMU interrupt request.
 * @param  hao: Pointer to AO_HandleTypeDef structure.
 * @retval None
 */
static void HAL_AO_IRQHandler(AO_HandleTypeDef *hao)
{
    uint32_t interrupt_flags;
    uint32_t src_index;

    /* Get interrupt and error flags */
    interrupt_flags = __HAL_AO_GET_IRQ_FLAG(hao);

    /* Handle interrupt */
    if (interrupt_flags != 0)
    {
        for (src_index = 0; src_index < HAL_AO_IRQ_SRC_NUM; src_index++)
        {
            if ((AO_Callback[src_index].InterruptMask & interrupt_flags) && (AO_Callback[src_index].Callback != NULL))
            {
                AO_Callback[src_index].Callback(AO_Callback[src_index].param);
            }
        }

        /* Clear interrupt flags */
        __HAL_AO_CLEAR_IRQ_FLAG(hao, interrupt_flags);
    }
}

/**
 * @brief  Handle PMU interrupt request.
 * @param  hao: Pointer to AO_HandleTypeDef structure.
 * @retval None
 */
static void HAL_AO_WakeupIRQHandler(AO_HandleTypeDef *hao)
{
    uint32_t interrupt_flags;
    uint32_t src_index;

    /* Get wakeup and error flags */
    interrupt_flags = __HAL_AO_GET_IRQ_FLAG(hao);
    printf("AO_FR=0x%lx\r\n", interrupt_flags);
    /* Handle wakeup events */
    if (interrupt_flags != 0)
    {
        for (src_index = 0; src_index < HAL_AO_WAKEUP_SRC_NUM; src_index++)
        {
            if ((AO_WakeupCallback[src_index].InterruptMask & interrupt_flags) && (AO_WakeupCallback[src_index].Callback != NULL))
            {
                AO_WakeupCallback[src_index].Callback(AO_WakeupCallback[src_index].param);
            }
        }

        /* Clear wakeup flags */
        __HAL_AO_CLEAR_IRQ_FLAG(hao, interrupt_flags);
    }
    __HAL_AO_CLEAR_WKUP_FLAG(hao);
    __ISB();
    __DSB();
}

/**
 * @brief  Initialize the AON according to the specified parameters.
 * @param  hao: Pointer to a AO_HandleTypeDef structure.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_AO_Init(AO_HandleTypeDef *hao)
{
    /* Check AO handle */
    if (hao == NULL)
    {
        return HAL_ERROR;
    }

    /* Check parameters */
    assert_param(IS_AO_CLOCK_SELECTION(hao->Init.ClockSelection));
    assert_param(IS_AO_CLOCK_DIVISION(hao->Init.ClockDivision));
    assert_param(IS_AO_OSCILLATOR(hao->Init.Oscillator));
    assert_param(IS_AO_INTERRUPT_MASK(hao->Init.InterruptMask));
    assert_param(IS_AO_WAKEUP_MASK(hao->Init.WakeupMask));
    assert_param(IS_AO_RESET_MASK(hao->Init.ResetMask));
    assert_param(IS_AO_ALT_FUNCTION(hao->Init.AltFunction));
    assert_param(IS_AO_PULLUP_CONFIG(hao->Init.PullUpConfig));

    /* Allocate lock resource */
    HAL_AO_LOCK();

    /* Initialize low level hardware */
    if (hao->MspInitCallback)
    {
        hao->MspInitCallback(hao);
    }

    /* Change AO peripheral state */
    hao->State = HAL_AO_STATE_BUSY;

    __HAL_AO_CLEAR_IRQ_FLAG(hao, 0xFFFFFFFFUL);
    __HAL_AO_CLEAR_WKUP_FLAG(hao);

    /* Apply default configuration */
    AO_SetConfig(hao->Instance, &hao->Init);

    HAL_NVIC_ConnectIRQ(AOINT_IRQn, AOINT_IRQ_PRIO, AOINT_IRQ_SUB_PRIO, (void *)HAL_AO_IRQHandler, (void *)hao, 0);
    HAL_NVIC_ConnectIRQ(AOWKUPINT_IRQn, AOWKUPINT_IRQ_PRIO, AOWKUPINT_IRQ_SUB_PRIO, (void *)HAL_AO_WakeupIRQHandler, (void *)hao, 0);

    /* Set default power mode */
    hao->State = HAL_AO_STATE_READY;

    /* Release Lock */
    HAL_AO_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Initialize the AON according to the specified parameters.
 * @param  hao: Pointer to a AO_HandleTypeDef structure.
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_AO_DeInit(AO_HandleTypeDef *hao)
{
    /* Check AO handle */
    if (hao == NULL)
    {
        return HAL_ERROR;
    }

    /* Allocate lock resource */
    HAL_AO_LOCK();

    /* Change AO peripheral state */
    hao->State = HAL_AO_STATE_BUSY;

    /* De-initialize low level hardware */
    if (hao->MspDeInitCallback)
    {
        hao->MspDeInitCallback(hao);
    }

    /* Reset AO register */
    __HAL_AO_RESET_REGISTER(hao);

    /* Reset AO state */
    hao->State = HAL_AO_STATE_RESET;

    /* Release Lock */
    HAL_AO_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Registers a callback for specific interrupt/wakeup events.
 * @param  InterruptMask: specifies which interrupt source to register
 * @param  WakeupMask: specifies which wakeup source to register
 * @param  pCallback: pointer to wakeup callback function
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_AO_RegisterCallback(uint32_t InterruptMask, uint32_t WakeupMask, void (*pCallback)(void *param), void *param)
{
    /* Check parameters */
    assert_param(IS_AO_INTERRUPT_MASK(InterruptMask));
    assert_param(IS_AO_WAKEUP_MASK(WakeupMask));

    /* Allocate lock resource */
    HAL_AO_LOCK();

    /* Store callback function pointer */
    for (uint8_t index = 0; index < HAL_AO_WAKEUP_SRC_NUM; index++)
    {
        if (index < HAL_AO_IRQ_SRC_NUM)
        {
            if (InterruptMask & (0x1 << index))
            {
                AO_Callback[index].Callback      = pCallback;
                AO_Callback[index].param         = param;
                AO_Callback[index].InterruptMask = InterruptMask & (0x1 << index);
            }
            if (WakeupMask & (0x1 << (index + AO_WKUP_INTEN_WDT_RST_MASK_Pos)))
            {
                AO_WakeupCallback[index].Callback      = pCallback;
                AO_WakeupCallback[index].param         = param;
                AO_WakeupCallback[index].InterruptMask = (WakeupMask >> AO_WKUP_INTEN_WDT_RST_MASK_Pos) & (0x1 << index);
            }
        }
        else
        {
            if (WakeupMask & (0x1 << (index + AO_WKUP_INTEN_WDT_RST_MASK_Pos + 2)))
            {
                AO_WakeupCallback[index].Callback      = pCallback;
                AO_WakeupCallback[index].param         = param;
                AO_WakeupCallback[index].InterruptMask = (WakeupMask >> AO_WKUP_INTEN_WDT_RST_MASK_Pos) & (0x1 << (index + 2));
            }
        }
    }

    /* Release Lock */
    HAL_AO_UNLOCK();

    return HAL_OK;
}

/**
 * @brief  Unregisters a interrupt/wakeup event callback.
 * @param  InterruptMask: specifies which interrupt source to unregister
 * @param  WakeupMask: specifies which wakeup source to unregister
 * @retval HAL status
 */
HAL_StatusTypeDef HAL_AO_UnRegisterCallback(uint32_t InterruptMask, uint32_t WakeupMask)
{
    /* Check parameters */
    assert_param(IS_AO_INTERRUPT_MASK(InterruptMask));
    assert_param(IS_AO_WAKEUP_MASK(WakeupMask));

    /* Allocate lock resource */
    HAL_AO_LOCK();

    /* Clear callback function pointer */
    for (uint8_t index = 0; index < HAL_AO_WAKEUP_SRC_NUM; index++)
    {
        if (index < HAL_AO_IRQ_SRC_NUM)
        {
            if (InterruptMask & (0x1 << index))
            {
                AO_Callback[index].Callback      = NULL;
                AO_Callback[index].param         = NULL;
                AO_Callback[index].InterruptMask = 0;
            }
            if (WakeupMask & (0x1 << (index + AO_WKUP_INTEN_WDT_RST_MASK_Pos)))
            {
                AO_WakeupCallback[index].Callback      = NULL;
                AO_WakeupCallback[index].param         = NULL;
                AO_WakeupCallback[index].InterruptMask = 0;
            }
        }
        else
        {
            if (WakeupMask & (0x1 << (index + AO_WKUP_INTEN_WDT_RST_MASK_Pos + 2)))
            {
                AO_WakeupCallback[index].Callback      = NULL;
                AO_WakeupCallback[index].param         = NULL;
                AO_WakeupCallback[index].InterruptMask = 0;
            }
        }
    }

    /* Release Lock */
    HAL_AO_UNLOCK();

    return HAL_OK;
}

HAL_StatusTypeDef HAL_AO_BootInfo_Set(HAL_AO_InfoTypeDef infoType, void *infoValue)
{
    HAL_StatusTypeDef ret = HAL_ERROR;

    switch (infoType)
    {
        case HAL_AO_BOOT_INFO_TYPE_BOOTMODE:
            if (AO_BootFlag_Set(*(HAL_AO_BootModeStruct *)infoValue) == 0)
            {
                ret = HAL_OK;
            }
            break;
        default:
            break;
    }

    return ret;
}

HAL_StatusTypeDef HAL_AO_BootInfo_Get(HAL_AO_InfoTypeDef infoType, void *infoValue)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    /* Allocate lock resource */
    HAL_AO_LOCK();
    switch (infoType)
    {
        case HAL_AO_BOOT_INFO_TYPE_BOOTMODE:
            *(HAL_AO_BootModeStruct *)infoValue = AO_BootFlag_Get();
            ret                                 = HAL_OK;
            break;
        default:
            break;
    }
    HAL_AO_UNLOCK();

    return ret;
}
#endif /* HAL_AO_MODULE_ENABLED */
