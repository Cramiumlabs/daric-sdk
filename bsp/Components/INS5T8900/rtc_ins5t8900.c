/**
******************************************************************************
* @file    rtc_ins5t8900.c
* @author  PERIPHERIAL BSP Team
* @brief   HAL RTC driver
           This file contains the common defines and functions prototypes for
           the rtc_ins5t8900.c driver.
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

#include <stdbool.h>
#include <sys/_stdint.h>
#define LOG_LEVEL LOG_LEVEL_D
#include "daric_log.h"

#include "daric_errno.h"
#include "daric_hal.h"
#include "daric_hal_i2c.h"
#include "rtc_ins5t8900.h"

//---------------------------------------------------------------------------------------------------------
#define YEAR_OFFSET 2000
#define INT_PORT    CONFIG_RTC_INS5T8900_INT_PORT
#define INT_PIN     CONFIG_RTC_INS5T8900_INT_PIN

// Define the base address and control bits for I2C communication
#define RTC_I2C0_SLAVE 0x32

// Define the register addresses for the RTC chip
#define RTC_SEC_REG        0x00 // Seconds register address
#define RTC_MIN_REG        0x01 // Minutes register address
#define RTC_HOUR_REG       0x02 // Hours register address
#define RTC_WEEK_REG       0x03 // Weekday register address
#define RTC_DAY_REG        0x04 // Day register address
#define RTC_MONTH_REG      0x05 // Month register address
#define RTC_YEAR_REG       0x06 // Year register address
#define RTC_ALARM_MIN_REG  0x08 // Minutes register for alarm
#define RTC_ALARM_HOUR_REG 0x09 // Hours register for alarm
#define RTC_ALARM_DAY_REG  0x0a // Day register for alarm
#define RTC_TIMER_L_REG    0x0b // Seconds register for timer (low byte)
#define RTC_TIMER_H_REG    0x0c // Seconds register for timer (high byte)
#define RTC_EXTENSION_REG  0x0d // controls alarm, timer, etc.
#define RTC_FLAG_REG       0x0e // Flag register
#define RTC_CTRL_REG       0x0f // controls alarm, timer interrupt enable

//---------------------------------------------------------------------------------------------------------
#define BCD2NUM(val) ((((val) >> 4) * 10) + ((val) & 0x0F))
#define NUM2BCD(val) ((((val) / 10) << 4) | ((val) % 10))

static I2C_HandleTypeDef RTC_I2C0; // I2C handle for RTC

static HAL_StatusTypeDef write_reg(uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef status;
    uint8_t           retryCount = 5; // Max retry attempts (adjust if needed)

    do
    {
        HAL_Delay(1); // Short delay before each write (for I2C timing)
        status = HAL_I2C1_Mem_Write(&RTC_I2C0, RTC_I2C0_SLAVE, reg, 1, &value, 1, HAL_MAX_DELAY);

        if (status == HAL_BUSY)
        {
            HAL_Delay(5); // If I2C is busy, wait 5ms before retrying
            retryCount--;
        }
        else
        {
            break; // Exit on success or non-BUSY errors
        }
    } while (retryCount > 0);

    return status; // Return final status (HAL_OK, HAL_ERROR, etc.)
}

static HAL_StatusTypeDef read_reg(uint8_t reg, uint8_t *value)
{
    HAL_StatusTypeDef status;
    uint8_t           retryCount = 5; // Max retry attempts (adjust as needed)

    do
    {
        HAL_Delay(1); // Short delay before each read (for I2C timing)
        status = HAL_I2C1_Mem_Read(&RTC_I2C0, RTC_I2C0_SLAVE, reg, 1, value, 1, HAL_MAX_DELAY);

        if (status == HAL_BUSY)
        {
            HAL_Delay(5); // If I2C is busy, wait 5ms before retrying
            retryCount--;
        }
        else
        {
            break; // Exit on success or non-BUSY errors
        }
    } while (retryCount > 0);

    return status; // Return final status (HAL_OK, HAL_ERROR, etc.)
}

static HAL_StatusTypeDef update_reg(uint8_t reg, uint8_t mask, uint8_t position, uint8_t value)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t           val_org;
    mask   = mask << position;
    value  = mask & (value << position);
    status = read_reg(reg, &val_org);
    val_org &= ~mask;
    val_org |= value;
    status |= write_reg(reg, val_org);
    return status;
}

static void rtc_irq_handler(void *UserData)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t           flag = 0, ie = 0;

    status |= read_reg(RTC_FLAG_REG, &flag);
    status |= read_reg(RTC_CTRL_REG, &ie);
    if (status != HAL_OK)
    {
        return;
    }
    if ((flag & 0x20) && (ie & 0x20))
    {
        // Update flag
        LOGI("RTC update occurred");
    }
    if ((flag & 0x10) && (ie & 0x10))
    {
        // Timer flag
        LOGI("RTC timer occurred.");
        BSP_RTC_DisTimer();
    }
    if ((flag & 0x08) && (ie & 0x08))
    {
        // Alarm flag
        LOGI("RTC alarm occurred.");
        BSP_RTC_DisAlarm();
    }
}

static void init_int_pin(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin              = INT_PIN;
    GPIO_InitStruct.Mode             = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull             = GPIO_NOPULL;
    GPIO_InitStruct.IsrHandler       = rtc_irq_handler;
    GPIO_InitStruct.UserData         = NULL;
    HAL_GPIO_Init(INT_PORT, &GPIO_InitStruct);
}

/**
 * @brief Check if a year is a leap year
 * @param year Full year (e.g., 2024)
 * @return true: leap year, false: common year
 */
static bool isLeapYear(uint16_t year)
{
    if (year % 400 == 0)
        return true;
    if (year % 100 == 0)
        return false;
    if (year % 4 == 0)
        return true;
    return false;
}

static const uint8_t daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static uint8_t getDaysInMonth(uint16_t year, uint8_t month)
{
    if (isLeapYear(year) && month == 2)
    {
        return 29;
    }
    else
    {
        return daysInMonth[month - 1];
    }
}

static bool isValidDate(uint16_t year, uint8_t month, uint8_t day)
{
    if (year < YEAR_OFFSET || year > 9999)
        return false;
    if (month < 1 || month > 12)
        return false;
    return (1 <= day && day <= getDaysInMonth(year, month));
}

static bool isValidTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    return !(hour >= 24 || minute >= 60 || second >= 60);
}

/**
 * @brief Calculate the weekday from the date. The date must be a validated date
 * @param year Full year (e.g., 2024)
 * @return weekday
 */
static RTC_WEEKDAY_T date2weekday(uint16_t year, uint8_t month, uint8_t day)
{
    const RTC_WEEKDAY_T WEEK[] = { SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY };

    if (month == 1 || month == 2)
    {
        month += 12;
        year--;
    }

    int id = (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400 + 1) % 7;

    return WEEK[id];
}

static void dateTimeAddDays(BSP_RTC_DATE_TIME *dt, uint16_t days)
{
    while (days >= 365)
    {
        bool startIsLeap = isLeapYear(dt->year);
        bool nextIsLeap  = isLeapYear(dt->year + 1);

        int daysInYear;
        if ((dt->month == 1) || (dt->month == 2 && dt->day <= 28))
        {
            daysInYear = startIsLeap ? 366 : 365;
        }
        else
        {
            daysInYear = nextIsLeap ? 366 : 365;
        }

        if (days >= daysInYear)
        {
            days -= daysInYear;
            dt->year++;
            if (startIsLeap && dt->month == 2 && dt->day == 29)
            {
                dt->day = 28;
            }
        }
        else
        {
            break;
        }
    }

    while (days > 0)
    {
        int daysInCurrentMonth   = getDaysInMonth(dt->year, dt->month);
        int daysRemainingInMonth = daysInCurrentMonth - dt->day;

        if (days > daysRemainingInMonth)
        {
            days -= daysRemainingInMonth + 1;
            dt->day = 1;
            dt->month++;

            if (dt->month > 12)
            {
                dt->month = 1;
                dt->year++;
            }
        }
        else
        {
            dt->day += days;
            days = 0;
        }
    }
}

static void dateTimeAddHours(BSP_RTC_DATE_TIME *dt, uint16_t hours)
{
    uint32_t h = dt->hour + hours;
    if (h >= 24)
    {
        dateTimeAddDays(dt, h / 24);
    }
    dt->hour = h % 24;
}

static void dateTimeAddMinuts(BSP_RTC_DATE_TIME *dt, uint16_t minutes)
{
    uint32_t m = dt->minute + minutes;
    if (m >= 60)
    {
        dateTimeAddHours(dt, m / 60);
    }
    dt->minute = m % 60;
}

/**
 * @brief Sets the date in the Real-Time Clock (RTC) module.
 *
 * @param[in] year The year value in binary format. The year must be offset to 2000
 * @param[in] month The month value in binary format.
 * @param[in] day The day value in binary format.
 * @param[in] weekday The weekday value in binary format. Don't care, the weekday is calculated
 * accordint to the date.
 *
 * @return The function returns int32_t, which indicates the status of the operation.
 *         In this case, it always returns BSP_ERROR_NONE.
 *
 * @details This function sets the date in the RTC module by converting the binary date values to
 * BCD format, and then writing them to the corresponding RTC registers.
 */
int32_t BSP_RTC_SetDate(uint8_t year, uint8_t month, uint8_t day, RTC_WEEKDAY_T weekday)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (!isValidDate(year + YEAR_OFFSET, month, day))
    {
        LOGW("Wrong! Date: %02d-%02d-%02d, Weekday: 0x%02X", year, month, day, weekday);
        return BSP_ERROR_WRONG_PARAM;
    }

    LOGD("Date: %02d-%02d-%02d, Weekday: 0x%02X", year, month, day, weekday);
    status |= write_reg(RTC_YEAR_REG, NUM2BCD(year));
    status |= write_reg(RTC_MONTH_REG, NUM2BCD(month));
    status |= write_reg(RTC_DAY_REG, NUM2BCD(day));
    status |= write_reg(RTC_WEEK_REG, date2weekday(year, month, day));

    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Retrieves the current date from the Real-Time Clock (RTC) module.
 *
 * @param[out] current_date A pointer to a character array where the current date will be stored.
 *                          The date will be formatted as "YYYY-MM-DD (Weekday: W)".
 *
 * @return The function returns int32_t, which indicates the status of the operation.
 *         In this case, it always returns BSP_ERROR_NONE.
 *
 * @details This function reads the current date from the RTC module, converts it from BCD format to
 * binary, and then formats it as a string in the specified format. The formatted string is then
 * stored in the memory location pointed to by the 'current_date' parameter.
 */
int32_t BSP_RTC_GetDate(char *current_date)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t           bcd_y, bcd_m, bcd_d, weekday;

    status |= read_reg(RTC_YEAR_REG, &bcd_y);
    status |= read_reg(RTC_MONTH_REG, &bcd_m);
    status |= read_reg(RTC_DAY_REG, &bcd_d);
    status |= read_reg(RTC_WEEK_REG, &weekday);

    // Format the date string
    snprintf(current_date, 50, "%04d-%02d-%02d Weekday: 0x%02X", YEAR_OFFSET + BCD2NUM(bcd_y), BCD2NUM(bcd_m), BCD2NUM(bcd_d), weekday);

    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Sets the time in the Real-Time Clock (RTC) module.
 *
 * @param[in] hours The hours value in binary format.
 * @param[in] minutes The minutes value in binary format.
 * @param[in] seconds The seconds value in binary format.
 *
 * @return The function returns int32_t, which indicates the status of the operation.
 *         In this case, it always returns BSP_ERROR_NONE.
 *
 * @details This function sets the time in the RTC module by converting the binary time values to
 * BCD format, and then writing them to the corresponding RTC registers.
 */
int32_t BSP_RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (!isValidTime(hours, minutes, seconds))
    {
        LOGW("Wrong! Time: %02d:%02d%02d", hours, minutes, seconds);
        return BSP_ERROR_WRONG_PARAM;
    }

    LOGD("Time: %02d:%02d:%02d", hours, minutes, seconds);
    status |= write_reg(RTC_SEC_REG, NUM2BCD(seconds));
    status |= write_reg(RTC_MIN_REG, NUM2BCD(minutes));
    status |= write_reg(RTC_HOUR_REG, NUM2BCD(hours));
    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Retrieves the current time from the Real-Time Clock (RTC) module.
 *
 * @param[out] current_time A pointer to a character array where the current time will be stored.
 *                          The time will be formatted as "HH:MM:SS".
 *
 * @return The function returns int32_t, which indicates the status of the operation.
 *         In this case, it always returns BSP_ERROR_NONE.
 *
 * @details This function reads the current time from the RTC module, converts it from BCD format to
 * binary, and then formats it as a string in the specified format. The formatted string is then
 * stored in the memory location pointed to by the 'current_time' parameter.
 */
int32_t BSP_RTC_GetTime(char *current_time)
{
    uint8_t           bcd_seconds, bcd_minutes, bcd_hours;
    HAL_StatusTypeDef status = HAL_OK;

    status |= read_reg(RTC_SEC_REG, &bcd_seconds);
    status |= read_reg(RTC_MIN_REG, &bcd_minutes);
    status |= read_reg(RTC_HOUR_REG, &bcd_hours);
    sprintf(current_time, "%02d:%02d:%02d", BCD2NUM(bcd_hours), BCD2NUM(bcd_minutes), BCD2NUM(bcd_seconds));
    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Sets the date and time.
 *
 * @param[in] date_time The date and time. The year must greate or erqual to 2000. Don't care the
 * weekday, it is calculated according to the date.
 *
 * @return BSP_ERROR_NONE if succeed, others else.
 */
int32_t BSP_RTC_SetDateTime(BSP_RTC_DATE_TIME date_time)
{
    HAL_StatusTypeDef status = HAL_OK;
    if (!isValidDate(date_time.year, date_time.month, date_time.day) || !isValidTime(date_time.hour, date_time.minute, date_time.second))
    {
        LOGW("Wrong! Date: %04d-%02d-%02d, Weekday: 0x%02X, Time: %02d:%02d:%02d", date_time.year, date_time.month, date_time.day, date_time.weekday, date_time.hour,
             date_time.minute, date_time.second);
        return BSP_ERROR_WRONG_PARAM;
    }
    LOGD("Date: %04d-%02d-%02d, Weekday: 0x%02X, Time: %02d:%02d:%02d", date_time.year, date_time.month, date_time.day, date_time.weekday, date_time.hour, date_time.minute,
         date_time.second);

    status |= write_reg(RTC_YEAR_REG, NUM2BCD(date_time.year - YEAR_OFFSET));
    status |= write_reg(RTC_MONTH_REG, NUM2BCD(date_time.month));
    status |= write_reg(RTC_DAY_REG, NUM2BCD(date_time.day));
    status |= write_reg(RTC_WEEK_REG, date2weekday(date_time.year, date_time.month, date_time.day));
    status |= write_reg(RTC_HOUR_REG, NUM2BCD(date_time.hour));
    status |= write_reg(RTC_MIN_REG, NUM2BCD(date_time.minute));
    status |= write_reg(RTC_SEC_REG, NUM2BCD(date_time.second));

    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Get the date and time.
 *
 * @param[out] date_time The date and time.
 *
 * @return BSP_ERROR_NONE if succeed, others else.
 */
int32_t BSP_RTC_GetDateTime(BSP_RTC_DATE_TIME *date_time)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t           year, month, day, hour, minute, second, weekday;

    status |= read_reg(RTC_YEAR_REG, &year);
    status |= read_reg(RTC_MONTH_REG, &month);
    status |= read_reg(RTC_DAY_REG, &day);
    status |= read_reg(RTC_WEEK_REG, &weekday);
    status |= read_reg(RTC_HOUR_REG, &hour);
    status |= read_reg(RTC_MIN_REG, &minute);
    status |= read_reg(RTC_SEC_REG, &second);

    date_time->year    = YEAR_OFFSET + BCD2NUM(year);
    date_time->month   = BCD2NUM(month);
    date_time->day     = BCD2NUM(day);
    date_time->weekday = weekday;
    date_time->hour    = BCD2NUM(hour);
    date_time->minute  = BCD2NUM(minute);
    date_time->second  = BCD2NUM(second);

    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Sets the alarm in the Real-Time Clock (RTC) module.
 *
 * @param[in] days The day value in binary format.
 * @param[in] hours The hours value in binary format.
 * @param[in] minutes The minutes value in binary format.
 *
 * @return The function returns int32_t, which indicates the status of the
 * operation. In this case, it always returns BSP_ERROR_NONE.
 *
 * @details This function sets the alarm in the RTC module by converting the
 * binary time values to BCD format, and then writing them to the corresponding
 * RTC registers. It also enables the alarm interrupt and alarm module.
 */
int32_t BSP_RTC_SetAlarm(uint8_t day, uint8_t hour, uint8_t minute)
{
    HAL_StatusTypeDef status = HAL_OK;
    if ((day == 0 || day > 31) || (hour >= 24) || (minute >= 60))
    {
        LOGD("Wrong! Date: %02d, Time: %02d:%02d", day, hour, minute);
        return BSP_ERROR_WRONG_PARAM;
    }

    LOGD("Date: %02d, Time: %02d:%02d", day, hour, minute);

    status |= update_reg(RTC_CTRL_REG, 0x01, 3, 0);          // Disable interrupt
    status |= update_reg(RTC_FLAG_REG, 0x01, 3, 0);          // Clear flage
    status |= write_reg(RTC_ALARM_MIN_REG, NUM2BCD(minute)); // Set alarm minute
    status |= write_reg(RTC_ALARM_HOUR_REG, NUM2BCD(hour));  // Set alarm hours
    status |= write_reg(RTC_ALARM_DAY_REG, NUM2BCD(day));    // Set alarm day
    status |= update_reg(RTC_EXTENSION_REG, 0x01, 6, 1);     // Configure day alarm
    status |= update_reg(RTC_CTRL_REG, 0x01, 3, 1);          // Enable interrupt

    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Sets the alarm in delay minutes base on current time.
 *
 * @param[in] minutes The minutes delayed.
 *
 * @return BSP_ERROR_NONE if succeed, others esle.
 */
int32_t BSP_RTC_SetAlarmDelayMinutes(uint16_t minutes)
{
    BSP_RTC_DATE_TIME dt;

    LOGD("Delay minutes: %d", minutes);

    int32_t status = BSP_RTC_GetDateTime(&dt);
    if (status != BSP_ERROR_NONE)
    {
        return BSP_ERROR_COMPONENT_FAILURE;
    }
    LOGD("Current Date: %04d-%02d-%02d, Weekday: 0x%02X, Time: %02d:%02d:%02d", dt.year, dt.month, dt.day, dt.weekday, dt.hour, dt.minute, dt.second);
    dateTimeAddMinuts(&dt, minutes);
    LOGD("Alarm Date: %04d-%02d-%02d, Weekday: 0x%02X, Time: %02d:%02d:%02d", dt.year, dt.month, dt.day, dt.weekday, dt.hour, dt.minute, dt.second);
    return BSP_RTC_SetAlarm(dt.day, dt.hour, dt.minute);
}

/**
 * @brief Retrieves the setted alarm.
 *
 * @param[out] day the day of alarm
 * @param[out] hour the hour of alarm
 * @param[out] minute the minute of alarm
 *
 * @return BSP_ERROR_NONE if succeed, others else.
 */
int32_t BSP_RTC_GetAlarm(uint8_t *day, uint8_t *hour, uint8_t *minute)
{
    uint8_t           bcd_m, bcd_h, bcd_d;
    HAL_StatusTypeDef status = HAL_OK;

    status |= read_reg(RTC_ALARM_MIN_REG, &bcd_m);
    status |= read_reg(RTC_ALARM_HOUR_REG, &bcd_h);
    status |= read_reg(RTC_ALARM_DAY_REG, &bcd_d);

    *day    = BCD2NUM(bcd_d);
    *hour   = BCD2NUM(bcd_h);
    *minute = BCD2NUM(bcd_m);

    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Disable the alarm.
 *
 * @return The function returns int32_t, which indicates the status of the
 * operation.
 */
int32_t BSP_RTC_DisAlarm()
{
    HAL_StatusTypeDef status = HAL_OK;
    LOGD("Disable");
    status |= update_reg(RTC_CTRL_REG, 0x01, 3, 0);       // Disable interrupt
    status |= update_reg(RTC_ALARM_MIN_REG, 0x01, 7, 0);  // Disable alarm
    status |= update_reg(RTC_ALARM_HOUR_REG, 0x01, 7, 0); // Disable alarm
    status |= update_reg(RTC_ALARM_DAY_REG, 0x01, 7, 0);  // Disable alarm
    status |= update_reg(RTC_FLAG_REG, 0x01, 3, 0);       // Clear flag
    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Set countdown timer. Count every second.
 *
 * @param[in] count the countdown value. Must big than 0 and less than 4096
 *
 * @return BSP_ERROR_NONE if succeed, others else.
 */
int32_t BSP_RTC_SetTimer(uint16_t count)
{
    HAL_StatusTypeDef status = HAL_OK;
    if (count == 0 || count >= 4096)
    {
        LOGW("Wrong! Count: %d", count);
        return BSP_ERROR_WRONG_PARAM;
    }
    LOGD("Count: %d", count);

    uint8_t count_h = count >> 8 & 0xFF;
    uint8_t count_l = count & 0xFF;

    status |= update_reg(RTC_CTRL_REG, 0x01, 4, 0);      // Disable interrup
    status |= update_reg(RTC_FLAG_REG, 0x01, 4, 0);      // Clear flag
    status |= update_reg(RTC_EXTENSION_REG, 0x01, 4, 0); // Disable timer
    status |= update_reg(RTC_EXTENSION_REG, 0x03, 0, 2); // 1 Hz
    status |= write_reg(RTC_TIMER_L_REG, count_l);       // Set countdown value
    status |= write_reg(RTC_TIMER_H_REG, count_h);       // Set countdown value
    status |= update_reg(RTC_CTRL_REG, 0x01, 4, 1);      // Enable interrup
    status |= update_reg(RTC_EXTENSION_REG, 0x01, 4, 1); // Enable timer

    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Get the current countdown value.
 *
 * @param count The remained countdown value.
 *
 * @return BSP_ERROR_NONE if succeed, others else.
 */
int32_t BSP_RTC_GetTimer(uint16_t *count)
{
    uint8_t count_l = 0;
    uint8_t count_h = 0;
    int32_t status  = HAL_OK;

    status |= read_reg(RTC_TIMER_L_REG, &count_l);
    status |= read_reg(RTC_TIMER_H_REG, &count_h);

    *count = count_h << 8 | count_l;
    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Disable the countdown timer.
 *
 * @return BSP_ERROR_NONE if succeed, others else.
 */
int32_t BSP_RTC_DisTimer()
{
    HAL_StatusTypeDef status = HAL_OK;
    LOGD("Disable");
    status = update_reg(RTC_CTRL_REG, 0x01, 4, 0);       // Disable interrup
    status |= update_reg(RTC_EXTENSION_REG, 0x01, 4, 0); // Disable timer
    status |= update_reg(RTC_FLAG_REG, 0x01, 4, 0);      // Clear flag
    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Set update interval and enable update.
 *
 * @param[in] t the update interval.
 *
 * @return BSP_ERROR_NONE if succeed, others else.
 */
int32_t BSP_RTC_SetUpdate(BSP_RTC_UP_T t)
{
    HAL_StatusTypeDef status = HAL_OK;
    LOGI("Update %s", t ? "MINUTE" : "SECOND");

    status |= update_reg(RTC_CTRL_REG, 0x01, 5, 0);      // Disable interrup
    status |= update_reg(RTC_FLAG_REG, 0x01, 5, 0);      // Clear flag
    status |= update_reg(RTC_EXTENSION_REG, 0x01, 5, t); // Configure update interval
    status |= update_reg(RTC_CTRL_REG, 0x01, 5, 1);      // Enable interrup

    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Disable the update.
 *
 * @return BSP_ERROR_NONE if succeed, others else.
 */
int32_t BSP_RTC_DisUpdate()
{
    HAL_StatusTypeDef status = HAL_OK;
    LOGD("Disable");

    status |= update_reg(RTC_CTRL_REG, 0x01, 5, 0); // Disable interrup
    status |= update_reg(RTC_FLAG_REG, 0x01, 5, 0); // Clear flag
    return (status == HAL_OK) ? BSP_ERROR_NONE : BSP_ERROR_COMPONENT_FAILURE;
}

/**
 * @brief Initializes the Real-Time Clock (RTC) module.
 *
 * @return The function returns int32_t, which indicates the status of the
 * operation. In this case, it always returns BSP_ERROR_NONE.
 *
 * @details This function initializes the RTC module by calling the
 * RTC_I2C0_Init() function. It also prints a message to the console indicating
 * the start of the initialization process.
 */
int32_t BSP_RTC_Init()
{
    LOGI("Init");
    memset(&RTC_I2C0, 0, sizeof(I2C_HandleTypeDef));

    RTC_I2C0.instance_id   = CONFIG_RTC_I2C_ID;
    RTC_I2C0.init.wait     = 1;
    RTC_I2C0.init.repeat   = 1;
    RTC_I2C0.init.baudrate = CONFIG_RTC_I2C_SPEED;
    RTC_I2C0.init.rx_buf   = 0;
    RTC_I2C0.init.rx_size  = 0;
    RTC_I2C0.init.tx_buf   = 0;
    RTC_I2C0.init.tx_size  = 0;
    RTC_I2C0.init.cmd_buf  = 0;
    RTC_I2C0.init.cmd_size = 0;

    HAL_I2C_Init(&RTC_I2C0);
    init_int_pin();
    return BSP_ERROR_NONE;
}

/**
 * @brief Deinitializes the Real-Time Clock (RTC) module.
 *
 * @return The function returns int32_t, which indicates the status of the
 * operation. In this case, it always returns BSP_ERROR_NONE.
 *
 * @details This function deinitializes the RTC module by calling the
 * RTC_I2C0_DeInit() function. It also prints a message to the console
 * indicating the start of the deinitialization process.
 */
int32_t BSP_RTC_DeInit()
{
    LOGI("Deinit");
    HAL_I2C_DeInit(&RTC_I2C0);
    return BSP_ERROR_NONE;
}

// clang-format off
void BSP_RTC_dumpRegister() {
    const static uint8_t addr0[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    const static uint8_t addr1[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    const static uint8_t addr2[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F};

    const static uint8_t* addrs[] = {addr0, addr1, addr2};
    const static uint8_t counts[] = {sizeof(addr0), sizeof(addr1), sizeof(addr2)};
    
    for (int i=0; i<sizeof(counts); i++) {
        char str[132] = {0};
        int16_t idx = 0;
        for (int j=0, k=0; k<16; k++) {
            uint8_t v = 0;
            if (j>=counts[i] || (addrs[i][j] & 0xF) != k) {
                idx += snprintf(str+idx, sizeof(str)-idx, "[--:--] ");
            } else {
                read_reg(addrs[i][j], &v);
                idx += snprintf(str+idx, sizeof(str)-idx, "[%02X:%02X] ", addrs[i][j], v);
                j++;
            }
        }
        LOGI("%s", str);
    }
}
// clang-format on
