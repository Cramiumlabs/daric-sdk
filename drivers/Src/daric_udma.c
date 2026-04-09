/**
 ******************************************************************************
 * @file    daric_udma.c
 * @author  UDMA Team
 * @brief   UDMA HAL module driver.
 *          This file provides firmware functions to manage the following
 *          functionalities of UDMA
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

/*
 * Copyright (C) 2018 ETH Zurich, University of Bologna and GreenWaves
 * Technologies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Authors: Germain Haugou, GreenWaves Technologies
 * (germain.haugou@greenwaves-technologies.com)
 */
#include "daric_udma.h"

#ifdef HAL_UDMA_MODULE_ENABLED
uint32_t HAL_UDMA_Get_Perh_Clock(void)
{
    // return 48000000;
    // volatile uint32_t per_clk = *((uint32_t *)(0x4004004C)) * 1000000;
    volatile uint32_t per_clk = DARIC_CGU->cgufsfreq3 * 1000000;

    return per_clk;
}

#endif /* DARIC_UDMA_DRV_ENABLED */
