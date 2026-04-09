/**
 ******************************************************************************
 * @file    tx_log.h
 * @author  OS Team
 * @brief   This file sets up the logging system by creating a mutex for 
 *          thread-safe operations and initializing the log level. 
 *          Ensures proper setup of logging infrastructure.
 *          Prints log messages with specified information.
 *          Filters messages based on the current log level.
 *          Ensures thread-safe logging using a mutex
******************************************************************************
* @attention
*
* © Copyright CrossBar, Inc. 2024.
*
* All rights reserved.
*
* This software is the proprietary property of CrossBar, Inc. and is protected
* by copyright laws. Any unauthorized reproduction, distribution, or
* modification is strictly prohibited.
*
******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TX_LOG_H__
#define __TX_LOG_H__
/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"
#include "stdarg.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Log levels definition */
typedef enum {
    LOG_VERBOSE = 0,  // Most detailed logging
    LOG_DEBUG,        // Debug-level information
    LOG_INFO,         // General information
    LOG_WARN,         // Warning messages
    LOG_ERROR,        // Error messages
    LOG_CRITICAL,     // Critical errors
    LOG_NONE,         // Disable all logging
} LogLevel;

/* Log ANSI color definition 
 * Colorize terminal log with escape sequences
 */
typedef enum {
    COL_BLACK = 0,
    COL_RED,
    COL_GREEN,
    COL_YELLO,
    COL_BLUE,
    COL_MAGENTA,
    COL_CYAN,
    COL_WHITE,
    COL_MAX
} LogColor;

/**
 * @brief  Initialize the logging framework.
 * @param  level Initial log level to configure
 * @param  tagFilterStr Separated tag string with '|'
 * @retval None
 *
 * Sets up the logging system by creating a mutex for
 * thread-safe operations and initializing the log level.
 * Ensures proper setup of logging infrastructure.
 */
extern void daricLogInit(LogLevel level, const char *tagFilterStr);

/**
 * @brief  Update a filter string and adds tags to the filter
 * @param  tagFilterStr Separated tag string with '|'
 * @return none
 *
 * Free gLogFilter dynamically allocated nodes
 * Splits a separated tag string with '|' and adds each tag
 * Supports multiple tag configurations
 */
extern void logUpdateTagFilter(const char *tagFilterStr);

/**
 * @brief  Dynamically change the current log output level.
 * @param  level New log level to set
 * @retval None
 *
 * Updates the current log level in a thread-safe manner.
 * Allows runtime modification of logging out level.
 */
extern void logUpdateLevel(LogLevel level);

/**
 * @brief  Return the current log level.
 * @param  None
 * @retval LogLevel Current log level in use
 *
 * Returns the current log level without modifying it.
 * Provides a way to check the current logging out level.
 */
extern LogLevel logGetLevel(void);

/**
 * @brief  Print log without a newline character.
 * @param  level Log level of the message
 * @param  tag Log filter string tag
 * @param  ansiColor Colorize terminal log with ANSI color
 * @param  format Printf-style format string
 * @param  ... Variable arguments for the format string
 * @retval None
 *
 * Prints log messages with specified information.
 * Filters messages based on the current log level.
 * Ensures thread-safe logging using a mutex.
 */
extern void logPrintSingle(LogLevel level, const char* tag, LogColor ansiColor, const char *format, ...);

/**
 * @brief  Core logging function with variable arguments.
 * @param  level Log level of the message
 * @param  tag Log filter string tag
 * @param  ansiColor Colorize terminal log with ANSI color
 * @param  file Source file name where log is generated
 * @param  line Line number in the source file
 * @param  format Printf-style format string
 * @param  ... Variable arguments for the format string
 * @retval None
 *
 * Prints log messages with specified information.
 * Filters messages based on the current log level.
 * Ensures thread-safe logging using a mutex.
 */
extern void logPrintDetail(LogLevel level, const char* tag, LogColor ansiColor, const char* file, int line, const char* format, ...);

/**
 * @brief  Core logging function with variable arguments.
 * @param  level Log level of the message
 * @param  tag Log filter string tag
 * @param  ansiColor Colorize terminal log with ANSI color
 * @param  format Printf-style format string
 * @param  ... Variable arguments for the format string
 * @retval None
 *
 * Prints log messages with specified information.
 * Filters messages based on the current log level.
 * Ensures thread-safe logging using a mutex.
 */
extern void logPrint(LogLevel level, const char* tag, LogColor ansiColor, const char* format, ...);

/* Macro definitions for easy logging 
 * Do not use these LOGx macros in the HAL interface, as tx_log depends on ThreadX, 
 * Also, do not use these interfaces in interrupt handlers, 
 * as LOGx call functions that can cause threads to sleep.
 * and the HAL must support bare-metal development 
 * and cannot be based on any OS interface 
 */
#ifdef CONFIG_LOG_DETAIL
    /* use LOG_TAG */
    #define LOGV(...) logPrintDetail(LOG_VERBOSE, LOG_TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGD(...) logPrintDetail(LOG_DEBUG, LOG_TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGI(...) logPrintDetail(LOG_INFO, LOG_TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGW(...) logPrintDetail(LOG_WARN, LOG_TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGE(...) logPrintDetail(LOG_ERROR, LOG_TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGC(...) logPrintDetail(LOG_CRITICAL, LOG_TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)

    /* Do not use LOG_TAG; instead, specify the TAG in the parameters */
    #define LOGV_TAG(TAG, ...) logPrintDetail(LOG_VERBOSE, TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGD_TAG(TAG, ...) logPrintDetail(LOG_DEBUG, TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGI_TAG(TAG, ...) logPrintDetail(LOG_INFO, TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGW_TAG(TAG, ...) logPrintDetail(LOG_WARN, TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGE_TAG(TAG, ...) logPrintDetail(LOG_ERROR, TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGC_TAG(TAG, ...) logPrintDetail(LOG_CRITICAL, TAG, COL_MAX, __FILE__, __LINE__, __VA_ARGS__)

    /* Colorize terminal log with escape sequences, use LOG_TAG */
    #define LOGV_COLOR(COLOR, ...) logPrintDetail(LOG_VERBOSE, LOG_TAG, COLOR, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGD_COLOR(COLOR, ...) logPrintDetail(LOG_DEBUG, LOG_TAG, COLOR, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGI_COLOR(COLOR, ...) logPrintDetail(LOG_INFO, LOG_TAG, COLOR, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGW_COLOR(COLOR, ...) logPrintDetail(LOG_WARN, LOG_TAG, COLOR, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGE_COLOR(COLOR, ...) logPrintDetail(LOG_ERROR, LOG_TAG, COLOR, __FILE__, __LINE__, __VA_ARGS__)
    #define LOGC_COLOR(COLOR, ...) logPrintDetail(LOG_CRITICAL, LOG_TAG, COLOR, __FILE__, __LINE__, __VA_ARGS__)
#else
    /* For reduce elf codesize, use LOG_TAG */
    #define LOGV(...) logPrint(LOG_VERBOSE, LOG_TAG, COL_MAX, __VA_ARGS__)
    #define LOGD(...) logPrint(LOG_DEBUG, LOG_TAG, COL_MAX, __VA_ARGS__)
    #define LOGI(...) logPrint(LOG_INFO, LOG_TAG, COL_MAX, __VA_ARGS__)
    #define LOGW(...) logPrint(LOG_WARN, LOG_TAG, COL_MAX, __VA_ARGS__)
    #define LOGE(...) logPrint(LOG_ERROR, LOG_TAG, COL_MAX, __VA_ARGS__)
    #define LOGC(...) logPrint(LOG_CRITICAL, LOG_TAG, COL_MAX, __VA_ARGS__)

    /* Do not use LOG_TAG; instead, specify the TAG in the parameters */
    #define LOGV_TAG(TAG, ...) logPrint(LOG_VERBOSE, TAG, COL_MAX, __VA_ARGS__)
    #define LOGD_TAG(TAG, ...) logPrint(LOG_DEBUG, TAG, COL_MAX, __VA_ARGS__)
    #define LOGI_TAG(TAG, ...) logPrint(LOG_INFO, TAG, COL_MAX, __VA_ARGS__)
    #define LOGW_TAG(TAG, ...) logPrint(LOG_WARN, TAG, COL_MAX, __VA_ARGS__)
    #define LOGE_TAG(TAG, ...) logPrint(LOG_ERROR, TAG, COL_MAX, __VA_ARGS__)
    #define LOGC_TAG(TAG, ...) logPrint(LOG_CRITICAL, TAG, COL_MAX, __VA_ARGS__)

    /* Colorize terminal log with escape sequences, use LOG_TAG */
    #define LOGV_COLOR(COLOR, ...) logPrint(LOG_VERBOSE, LOG_TAG, COLOR, __VA_ARGS__)
    #define LOGD_COLOR(COLOR, ...) logPrint(LOG_DEBUG, LOG_TAG, COLOR, __VA_ARGS__)
    #define LOGI_COLOR(COLOR, ...) logPrint(LOG_INFO, LOG_TAG, COLOR, __VA_ARGS__)
    #define LOGW_COLOR(COLOR, ...) logPrint(LOG_WARN, LOG_TAG, COLOR, __VA_ARGS__)
    #define LOGE_COLOR(COLOR, ...) logPrint(LOG_ERROR, LOG_TAG, COLOR, __VA_ARGS__)
    #define LOGC_COLOR(COLOR, ...) logPrint(LOG_CRITICAL, LOG_TAG, COLOR, __VA_ARGS__)
#endif
/* Do not print \r & \n */
#define LOGV_RAW(...) logPrintSingle(LOG_VERBOSE, LOG_TAG, COL_MAX, __VA_ARGS__)
#define LOGD_RAW(...) logPrintSingle(LOG_DEBUG, LOG_TAG, COL_MAX, __VA_ARGS__)
#define LOGI_RAW(...) logPrintSingle(LOG_INFO, LOG_TAG, COL_MAX, __VA_ARGS__)
#define LOGW_RAW(...) logPrintSingle(LOG_WARN, LOG_TAG, COL_MAX, __VA_ARGS__)
#define LOGE_RAW(...) logPrintSingle(LOG_ERROR, LOG_TAG, COL_MAX, __VA_ARGS__)
#define LOGC_RAW(...) logPrintSingle(LOG_CRITICAL, LOG_TAG, COL_MAX, __VA_ARGS__)

/* Configuration macro for compile-time log level */
#ifndef CONFIG_LOG_LEVEL
#define CONFIG_LOG_LEVEL LOG_VERBOSE
#endif

#ifndef CONFIG_LOG_TAG
#define CONFIG_LOG_TAG ""
#endif

#define LOG_TAG ""

#ifdef __cplusplus
}
#endif

#endif