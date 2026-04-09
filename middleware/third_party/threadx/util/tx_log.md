# CrossBar tx_log User Guide

## 1. Overview

The logging system is thread-safe logging system designed for embedded systems using ThreadX RTOS. This logging infrastructure provides fine-grained control over log output through multiple filtering mechanisms, ensuring efficient and configurable logging across different application components.

The default level is set to LOG_VERBOSE. This means that any level above and including LOG_VERBOSE will be logged. Before you make any calls to a logging method you should check to see if your tag should be logged. You can change the default level by setting CONFIG_LOG_LEVEL.

The default tag is an empty string, which means the tag filtering feature is not enabled. You can change the default level by setting CONFIG_LOG_TAG.

`Do not use these LOGx macros in the HAL interface, as tx_log depends on ThreadX, and the HAL must support bare-metal development and cannot be based on any OS interface, Also, do not use these interfaces in interrupt handlers, as LOGx call functions that can cause threads to sleep.`.

## 2. Key Features

### 2.1. Log Level Filtering
The logging system supports multiple log levels to control the verbosity of log messages,Supports multiple log levels from LOG_VERBOSE to LOG_CRITICAL, Can set a default log level at compile-time, Allows dynamic log level changes at runtime, Filters log based on the current log level:
- `LOG_VERBOSE`: Most detailed logging for comprehensive debugging
- `LOG_DEBUG`: Detailed debug information
- `LOG_INFO`: General informational messages
- `LOG_WARN`: Warning messages indicating potential issues
- `LOG_ERROR`: Error messages highlighting significant problems
- `LOG_CRITICAL`: Critical errors requiring immediate attention
- `LOG_NONE`: Completely disable logging

### 2.2. Tag-Based Filtering
The logging system implements a tag-based filtering mechanism using:
- Support for an unlimited number of log tags
- A hash table for efficient tag management
- A bitmap for quick tag presence checking
- Support for multiple tags with collision resolution
- Dynamic tag filtering at runtime
```c
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "MMI_TAG"

LOGV("[%d] Sleeping for 3s ...", cnt);
```
- The filtered TAG can be specified either through the LOG_TAG macro in each file or directly via the LOGx_TAG interface parameters.
```c
#include "tx_log.h"

LOGV_TAG("MAIN_TAG", "[%d] Sleeping for 3s ...", cnt);
```

### 2.3. Thread Safety
Ensures thread-safe logging operations through:
- ThreadX mutex for protecting critical logging sections
- Mutex-guarded log level and tag filter modifications
- Prevention of race conditions during log message generation

### 2.4. Colorized Terminal Output
Provides terminal log colorization with ANSI escape sequences:
Supports multiple color options (Black, Red, Green, Yellow, Blue, Magenta, Cyan, White), Optional color selection for different log levels, Improves log readability and visual distinction
- `COL_BLACK`
- `COL_RED`
- `COL_GREEN`
- `COL_YELLO`
- `COL_BLUE`
- `COL_MAGENTA`
- `COL_CYAN`
- `COL_WHITE`

### 2.5. Flexible Logging Macros:
   - LOGV(), LOGD(), LOGI(), LOGW(), LOGE(), LOGC() macros
   - Support capture file and line information
   - Support variable arguments like printf()

## 3. Initialization and Configuration

### 3.1. Log Initialization
```c
void daricLogInit(LogLevel level, const char *tagFilterStr);
```
- Sets initial log level
- Configures tag filters, Separated tag string with '|'
- Creates a thread-safe mutex for logging operations

### 3.2. Runtime Configuration Methods
- `logUpdateTagFilter()`: Dynamically update log tags at runtime
- `logUpdateLevel()`: Dynamically update log level at runtime

## 4. Logging Macros

### 4.1. Detailed Logging Macros (When `CONFIG_LOG_DETAIL` is defined)
- `LOGV()`: Verbose logging with file and line information
- `LOGD()`: Debug logging with file and line information
- `LOGI()`: Information logging with file and line information
- `LOGW()`: Warning logging with file and line information
- `LOGE()`: Error logging with file and line information
- `LOGC()`: Critical logging with file and line information

### 4.2. Compact Logging Macros
Similar to detailed macros but without file and line information, optimized for reduced code size.

### 4.3. Colored Logging Variants
Each log level has a color-specific variant (e.g., `LOGD_COLOR()`) for enhanced visual logging.
- `LOGV_COLOR()`
- `LOGD_COLOR()`
- `LOGI_COLOR()`
- `LOGW_COLOR()`
- `LOGE_COLOR()`
- `LOGC_COLOR()`

### 4.4. Flexibly specify your own TAG
Do not use LOG_TAG; instead, specify the TAG in the parameters.
- `LOGV_TAG()`
- `LOGD_TAG()`
- `LOGI_TAG()`
- `LOGW_TAG()`
- `LOGE_TAG()`
- `LOGC_TAG()`

### 4.5. Print the raw data,without any decoration
Print the raw data without adding a prefix or a newline at the end,You can think of it as printf.
- `LOGV_RAW()`
- `LOGD_RAW()`
- `LOGI_RAW()`
- `LOGW_RAW()`
- `LOGE_RAW()`
- `LOGC_RAW()`

## 5. Compilation notes:
   - `CONFIG_LOG_LEVEL` Default logging level
   - `CONFIG_LOG_TAG`: Default tag configuration
   - `CONFIG_LOG_DETAIL`: Switch between detailed and compact logging
   - Easy to configure without modifying source code
   - Link with threadx library
   - Include the header tx_log.h in your project
   - Compile with `./build.sh -- -DCONFIG_LOG_LEVEL=LOG_VERBOSE` to change default level
   - Compile with `./build.sh -- -DCONFIG_LOG_TAG="\"MMI_TAG|GUI_TAG|DRV_TAG\""` to change default TAG
   - `./build.sh -- -DCONFIG_LOG_LEVEL=LOG_VERBOSE -DCONFIG_LOG_TAG="\"MMI_TAG|GUI_TAG|DRV_TAG\""`

## 6. Example usage:
```c
#include "tx_log.h"

#undef LOG_TAG
#define LOG_TAG "MMI_TAG"

LOGI("Application starting, version %d.%d", MAJOR_VERSION, MINOR_VERSION);
LOGD("Debug info: x = %d, y = %f", x, y);

/* Dynamically change log level 
 * Now debug logs will print
 */
logUpdateLevel(LOG_DEBUG);
LOGW("This is a warning message");
/* Dynamically update log tags */
logUpdateTagFilter("CUST_TAG|DRV_TAG|GUI_TAG|SCE_TAG");
LOGV_TAG("CUST_TAG", "Do not use LOG_TAG; instead, specify the TAG in the parameters");
LOGE("An error occurred: %s", errorMsg);
/* Colorize terminal log with escape sequences */
LOGE_COLOR(COL_RED, "An error occurred: %s", errorMsg);
/* Print the raw data without adding a prefix or a newline at the end. */
for (int i = 0; i < 10; i++)
{
    LOGV_RAW("%d,", i);
}
/* Print a newline without logLevel prefix*/
LOGV_RAW("\r\n");
```
