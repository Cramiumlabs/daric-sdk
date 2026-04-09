/**
 ******************************************************************************
 * @file    daric_hal_nvic.c
 * @author  NVIC Team
 * @brief   NVIC HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of NVIC
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

/** @addtogroup Daric_HAL_Driver
 * @{
 */

/** @defgroup NVIC NVIC
 * @brief NVIC HAL module driver
 * @{
 */

#ifdef HAL_NVIC_MODULE_ENABLED

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup NVIC_Exported_Functions NVIC Exported Functions
 * @{
 */

/** @defgroup NVIC_Exported_Functions_Group1 Initialization and
de-initialization functions
 *  @brief    Initialization and Configuration functions
 *
@verbatim
  ==============================================================================
              ##### Initialization and de-initialization functions #####
  ==============================================================================
    [..]
      This section provides the NVIC HAL driver functions allowing to configure
Interrupts Systick functionalities

@endverbatim
  * @{
  */

/**
 * @brief  Sets the priority grouping field (preemption priority and
 * subpriority) using the required unlock sequence.
 * @param  PriorityGroup The priority grouping bits length.
 *         This parameter can be one of the following values:
 *         @arg NVIC_PRIORITYGROUP_0: 0 bits for preemption priority
 *                                    4 bits for subpriority
 *         @arg NVIC_PRIORITYGROUP_1: 1 bits for preemption priority
 *                                    3 bits for subpriority
 *         @arg NVIC_PRIORITYGROUP_2: 2 bits for preemption priority
 *                                    2 bits for subpriority
 *         @arg NVIC_PRIORITYGROUP_3: 3 bits for preemption priority
 *                                    1 bits for subpriority
 *         @arg NVIC_PRIORITYGROUP_4: 4 bits for preemption priority
 *                                    0 bits for subpriority
 * @note   When the NVIC_PriorityGroup_0 is selected, IRQ preemption is no more
 * possible. The pending IRQ priority will be managed only by the subpriority.
 * @retval None
 */
void HAL_NVIC_SetPriorityGrouping(uint32_t PriorityGroup)
{
    /* Check the parameters */
    assert_param(IS_NVIC_PRIORITY_GROUP(PriorityGroup));

    /* Set the PRIGROUP[10:8] bits according to the PriorityGroup parameter value
     */
    NVIC_SetPriorityGrouping(PriorityGroup);
}

/**
 * @brief  Sets the priority of an interrupt.
 * @param  IRQn External interrupt number.
 *         This parameter can be an enumerator of IRQn_Type enumeration
 * @param  PreemptPriority The preemption priority for the IRQn channel.
 *         This parameter can be a value between 0 and 15
 *         A lower priority value indicates a higher priority
 * @param  SubPriority the subpriority level for the IRQ channel.
 *         This parameter can be a value between 0 and 15
 *         A lower priority value indicates a higher priority.
 * @retval None
 */
void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority)
{
    uint32_t prioritygroup = 0x00;

    /* Check the parameters */
    assert_param(IS_NVIC_SUB_PRIORITY(SubPriority));
    assert_param(IS_NVIC_PREEMPTION_PRIORITY(PreemptPriority));

    prioritygroup = NVIC_GetPriorityGrouping();

    NVIC_SetPriority(IRQn, NVIC_EncodePriority(prioritygroup, PreemptPriority, SubPriority));
}

/**
 * @brief  Enables a device specific interrupt in the NVIC interrupt controller.
 * @note   To configure interrupts priority correctly, the
 * NVIC_PriorityGroupConfig() function should be called before.
 * @param  IRQn External interrupt number.
 *         This parameter can be an enumerator of IRQn_Type enumeration
 * @retval None
 */
void HAL_NVIC_EnableIRQ(IRQn_Type IRQn)
{
    /* Check the parameters */
    assert_param(IS_NVIC_DEVICE_IRQ(IRQn));

    /* Enable interrupt */
    NVIC_EnableIRQ(IRQn);
}

/**
 * @brief  Disables a device specific interrupt in the NVIC interrupt
 * controller.
 * @param  IRQn External interrupt number.
 *         This parameter can be an enumerator of IRQn_Type enumeration
 * @retval None
 */
void HAL_NVIC_DisableIRQ(IRQn_Type IRQn)
{
    /* Check the parameters */
    assert_param(IS_NVIC_DEVICE_IRQ(IRQn));

    /* Disable interrupt */
    NVIC_DisableIRQ(IRQn);
}

/**
 * @brief  Gets the priority grouping field from the NVIC Interrupt Controller.
 * @retval Priority grouping field (SCB->AIRCR [10:8] PRIGROUP field)
 */
uint32_t HAL_NVIC_GetPriorityGrouping(void)
{
    /* Get the PRIGROUP[10:8] field value */
    return NVIC_GetPriorityGrouping();
}

/**
 * @brief  Gets the priority of an interrupt.
 * @param  IRQn External interrupt number.
 *         This parameter can be an enumerator of IRQn_Type enumeration
 * @param   PriorityGroup the priority grouping bits length.
 *         This parameter can be one of the following values:
 *           @arg NVIC_PRIORITYGROUP_0: 0 bits for preemption priority
 *                                      4 bits for subpriority
 *           @arg NVIC_PRIORITYGROUP_1: 1 bits for preemption priority
 *                                      3 bits for subpriority
 *           @arg NVIC_PRIORITYGROUP_2: 2 bits for preemption priority
 *                                      2 bits for subpriority
 *           @arg NVIC_PRIORITYGROUP_3: 3 bits for preemption priority
 *                                      1 bits for subpriority
 *           @arg NVIC_PRIORITYGROUP_4: 4 bits for preemption priority
 *                                      0 bits for subpriority
 * @param  pPreemptPriority Pointer on the Preemptive priority value (starting
 * from 0).
 * @param  pSubPriority Pointer on the Subpriority value (starting from 0).
 * @retval None
 */
void HAL_NVIC_GetPriority(IRQn_Type IRQn, uint32_t PriorityGroup, uint32_t *pPreemptPriority, uint32_t *pSubPriority)
{
    /* Check the parameters */
    assert_param(IS_NVIC_PRIORITY_GROUP(PriorityGroup));
    /* Get priority for Cortex-M system or device specific interrupts */
    NVIC_DecodePriority(NVIC_GetPriority(IRQn), PriorityGroup, pPreemptPriority, pSubPriority);
}

/**
 * @brief  Sets Pending bit of an external interrupt.
 * @param  IRQn External interrupt number
 *         This parameter can be an enumerator of IRQn_Type enumeration
 * @retval None
 */
void HAL_NVIC_SetPendingIRQ(IRQn_Type IRQn)
{
    /* Check the parameters */
    assert_param(IS_NVIC_DEVICE_IRQ(IRQn));

    /* Set interrupt pending */
    NVIC_SetPendingIRQ(IRQn);
}

/**
 * @brief  Clears the pending bit of an external interrupt.
 * @param  IRQn External interrupt number.
 *         This parameter can be an enumerator of IRQn_Type enumeration
 * @retval None
 */
void HAL_NVIC_ClearPendingIRQ(IRQn_Type IRQn)
{
    /* Check the parameters */
    assert_param(IS_NVIC_DEVICE_IRQ(IRQn));

    /* Clear pending interrupt */
    NVIC_ClearPendingIRQ(IRQn);
}

/**
 * @brief  Gets Pending Interrupt (reads the pending register in the NVIC
 *         and returns the pending bit for the specified interrupt).
 * @param  IRQn External interrupt number.
 *          This parameter can be an enumerator of IRQn_Type enumeration
 * @retval status: - 0  Interrupt status is not pending.
 *                 - 1  Interrupt status is pending.
 */
uint32_t HAL_NVIC_GetPendingIRQ(IRQn_Type IRQn)
{
    /* Check the parameters */
    assert_param(IS_NVIC_DEVICE_IRQ(IRQn));

    /* Return 1 if pending else 0 */
    return NVIC_GetPendingIRQ(IRQn);
}

/**
 * @brief Gets active interrupt ( reads the active register in NVIC and returns
 * the active bit).
 * @param IRQn External interrupt number
 *         This parameter can be an enumerator of IRQn_Type enumeration
 * @retval status: - 0  Interrupt status is not pending.
 *                 - 1  Interrupt status is pending.
 */
uint32_t HAL_NVIC_GetActive(IRQn_Type IRQn)
{
    /* Check the parameters */
    assert_param(IS_NVIC_DEVICE_IRQ(IRQn));

    /* Return 1 if active else 0 */
    return NVIC_GetActive(IRQn);
}

typedef struct
{
    void (*handler)(const void *);
    const void *arg;
} NVIC_Int_TableTypeDef;

static void IRQ_Default_Handler(const void *arg)
{
    (void)arg;
    while (1)
    {
        /* loop forever */
    }
}

static NVIC_Int_TableTypeDef isr_table[TOTAL_IRQ_NUMS] = { [0 ...(TOTAL_IRQ_NUMS - 1)] = { NULL, IRQ_Default_Handler } };

void EXT_IRQ_Handler(void)
{
    uint32_t irq = __get_IPSR() - 16;

    NVIC_Int_TableTypeDef *entry = &isr_table[irq];
    entry->handler(entry->arg);
}

/**
 * @brief Connect the given handler to IRQn channel.
 *
 * @param IRQn External interrupt number.
 * @param PreemptPriority The preemption priority for the IRQn channel.
 * @param SubPriority SubPriority the subpriority level for the IRQ channel.
 * @param Handler Interrupt handler to connect to the IRQn channel.
 * @param Arg Argument for the handler.
 * @param Enable Set to 1 to enable this IRQn channel, set to 0 will disable it.
 * @retval None
 */
void HAL_NVIC_ConnectIRQ(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority, void (*Handler)(const void *Arg), const void *Arg, uint32_t Enable)
{
    /* Check the parameters */
    assert_param(IS_NVIC_DEVICE_IRQ(IRQn));

    NVIC_DisableIRQ(IRQn);

    isr_table[(int)IRQn].handler = Handler;
    isr_table[(int)IRQn].arg     = Arg;

    HAL_NVIC_SetPriority(IRQn, PreemptPriority, SubPriority);

    if (Enable != 0)
    {
        NVIC_EnableIRQ(IRQn);
    }
}

/**
 * @brief  Initiates a system reset request to reset the MCU.
 * @retval None
 */
void HAL_NVIC_SystemReset(void)
{
    /* System Reset */
    NVIC_SystemReset();
}

#endif /* HAL_NVIC_MODULE_ENABLED */
/**
 * @}
 */

/**
 * @}
 */
