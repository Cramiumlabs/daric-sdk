/**
******************************************************************************
* @file    rtc_ins5t8900.h
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the rtc_ins5t8900.c driver.
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
#ifndef RTC_INS5T8900_H
#define RTC_INS5T8900_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum
{
    SUNDAY    = 0x01,
    MONDAY    = 0x02,
    TUESDAY   = 0x04,
    WEDNESDAY = 0x08,
    THURSDAY  = 0x10,
    FRIDAY    = 0x20,
    SATURDAY  = 0x40,
} RTC_WEEKDAY_T;

typedef struct
{
    uint16_t      year;
    uint8_t       month;
    uint8_t       day;
    uint8_t       hour;
    uint8_t       minute;
    uint8_t       second;
    RTC_WEEKDAY_T weekday;
} BSP_RTC_DATE_TIME;

typedef enum
{
    BSP_RTC_UP_SECOND = 0x00,
    BSP_RTC_UP_MINUTE = 0x01,
} BSP_RTC_UP_T;

int32_t BSP_RTC_Init();   // Initialize the RTC
int32_t BSP_RTC_DeInit(); // Deinitialize the RTC

int32_t BSP_RTC_SetDate(uint8_t year, uint8_t month, uint8_t day, RTC_WEEKDAY_T weekday);
int32_t BSP_RTC_GetDate(char *current_date);
int32_t BSP_RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
int32_t BSP_RTC_GetTime(char *current_time);
int32_t BSP_RTC_SetDateTime(BSP_RTC_DATE_TIME date);
int32_t BSP_RTC_GetDateTime(BSP_RTC_DATE_TIME *date);
int32_t BSP_RTC_SetAlarm(uint8_t days, uint8_t hours, uint8_t minutes);
int32_t BSP_RTC_SetAlarmDelayMinutes(uint16_t minute);
int32_t BSP_RTC_GetAlarm(uint8_t *day, uint8_t *hour, uint8_t *minute);
int32_t BSP_RTC_DisAlarm();
int32_t BSP_RTC_SetTimer(uint16_t count);
int32_t BSP_RTC_GetTimer(uint16_t *count);
int32_t BSP_RTC_DisTimer();
int32_t BSP_RTC_SetUpdate(BSP_RTC_UP_T t);
int32_t BSP_RTC_DisUpdate();

void BSP_RTC_dumpRegister();

#endif
