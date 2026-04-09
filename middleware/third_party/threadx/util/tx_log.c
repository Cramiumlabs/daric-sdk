/**
 ******************************************************************************
 * @file    tx_log.c
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

/* Includes --------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <tx_api.h>
#include "tx_log.h"
#include "daric_hal.h"

/* Maximum length per line */
#define MAX_LINE_LENGTH 200
#define MAX_HASH_BUCKET 128
#define BITS_PER_WORD 32
#define MAX_BITMAP_MASK (MAX_HASH_BUCKET / BITS_PER_WORD)
#define MAX_TAG_LENGTH 16
#define END_ANSICOLOR_ESCAPE "\033[0m"

/* Static variables for log management */
static TX_MUTEX logMutex; /* ThreadX mutex for thread-safe logging */

#pragma GCC diagnostic ignored "-Wunused-variable"
/* Log level string representations */
static const char *logLevelStrings[] = {
    "V",
    "D",
    "I",
    "W",
    "E",
    "C",
    "NONE",
};

/* Log ANSI color definition 
 * Colorize terminal log with escape sequences
 */
static const char * gLogColorArr[COL_MAX] = {
    [COL_BLACK]     = "\033[1;30m",
    [COL_RED]       = "\033[1;31m",
    [COL_GREEN]     = "\033[1;32m",
    [COL_YELLO]     = "\033[1;33m",
    [COL_BLUE]      = "\033[1;34m",
    [COL_MAGENTA]   = "\033[1;35m",
    [COL_CYAN]      = "\033[1;36m",
    [COL_WHITE]     = "\033[1;37m",
};
#pragma GCC diagnostic pop

/* Store the content of a single line log. */
static char gStrLineBuff[MAX_LINE_LENGTH];

/**
 * Represents a single tag node in the hash table
 * Implements a linked list node for handling hash collisions
 * Each node stores a tag string and a pointer to the next node
 */
typedef struct TagNode
{
    char tag[MAX_TAG_LENGTH]; // Storage for the tag string
    struct TagNode *next;     // Pointer to next node in the collision chain
} TagNode;

/**
 * Manages log filtering with hash table and bitmap
 * Provides efficient tag-based log filtering mechanism
 * Supports multiple tags
 */
typedef struct
{
    TagNode *buckets[MAX_HASH_BUCKET]; /* Hash buckets for tag storage */
    uint32_t bitmap[MAX_BITMAP_MASK];  /* Bitmap for quick tag presence check */
    uint32_t tagNum;                   /* The total number of log tags */
    LogLevel logLevel;                 /* Filter level */
} LogFilter;

static LogFilter gLogFilter = {
    .buckets    = {0},
    .bitmap     = {0},
    .logLevel   = CONFIG_LOG_LEVEL,
    .tagNum     = 0
};

/**
 * @brief  Generates a hash value for a given tag
 *         Uses the DJB hash algorithm to convert a string to a hash value
 * @param  tag Input tag string to be hashed
 * @return Calculated hash value within MAX_HASH_BUCKET range
 *
 * DJBX33A (Daniel J. Bernstein, Times 33 with Addition)
 * It basically uses a function
 * like "hash(i) = hash(i-1) * 33 + str[i]".
 * This is one of the best known hash functions for strings.
 * Because it is both computed very fast and distributes very well.
 * The number 5381 in the calculation can improve the dispersion of the hash values.
 */
static uint32_t logTagHash(const char *tag)
{
    uint32_t hash = 5381;
    int c;
    while ((c = *tag++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % MAX_HASH_BUCKET;
}

/**
 * @brief  Initializes a log filter
 * @param  filter Pointer to the LogFilter to be initialized
 * @retval None
 *
 * Resets all buckets and bitmap to zero
 * Prepares the filter for tag management
 */
static void logTagFilterInit(LogFilter *filter)
{
    tx_mutex_get(&logMutex, TX_WAIT_FOREVER);
    memset(filter->buckets, 0, sizeof(filter->buckets));
    memset(filter->bitmap, 0, sizeof(filter->bitmap));
    filter->tagNum = 0;
    tx_mutex_put(&logMutex);
}

/**
 * @brief  Adds a tag to the log filter
 * @param  filter Pointer to the LogFilter
 * @param  tag Tag string to be added
 * @return none
 *
 * Handles tag insertion with collision resolution
 * Prevents duplicate tags and manages hash table
 */
static void addOneTag(LogFilter *filter, const char *tag)
{
    TagNode *newNode = NULL;
    TagNode *current = NULL;
    uint32_t bitmapIndex = 0;
    uint32_t bitPosition = 0;
    uint32_t index = 0;
    char tagstr[MAX_TAG_LENGTH];

    if (tag == NULL || strlen(tag) == 0)
    {
        return;
    }

    memset(tagstr, 0, sizeof(tagstr));
    strncpy(tagstr, tag, sizeof(tagstr) - 1);

    index = logTagHash(tagstr);
    /* Check for existing tag */
    current = filter->buckets[index];
    while (current != NULL)
    {
        if (strncmp(current->tag, tagstr, sizeof(current->tag)) == 0)
        {
            return; /* Tag already exists */
        }
        current = current->next;
    }

    /* Create new tag node */
    newNode = malloc(sizeof(TagNode));
    if (newNode == NULL)
    {
        /* not enough memory, LOGx macro cannot be used in this location. */
        printf ("addOneTag Error: not enough memory\r\n");
        return;
    }
    memset(newNode, 0, sizeof(TagNode));
    strncpy(newNode->tag, tagstr, sizeof(newNode->tag) - 1);

    /* Insert at bucket head */
    newNode->next = filter->buckets[index];
    filter->buckets[index] = newNode;

    /* Set bitmap */
    bitmapIndex = index / BITS_PER_WORD;
    bitPosition = index % BITS_PER_WORD;
    filter->bitmap[bitmapIndex] |= (1ULL << bitPosition);

    filter->tagNum++;

    return;
}

/**
 * @brief  Checks if a tag is enabled in the log filter
 * @param  filter Pointer to the LogFilter
 * @param  tag Tag string to check
 * @return 1 if tag is enabled, false otherwise
 *
 * Performs two-stage verification:
 * 1. Quick bitmap check
 * 2. Precise string matching
 * 3. Prevents duplicate tags
 */
static int isTagEnabled(LogFilter *filter, const char *tag)
{
    uint32_t index = 0;
    /* Quick bitmap check */
    uint32_t bitmapIndex = 0;
    uint32_t bitPosition = 0;

    if (tag == NULL || strlen(tag) == 0)
    {
        return 0;
    }

    index = logTagHash(tag);
    bitmapIndex = index / BITS_PER_WORD;
    bitPosition = index % BITS_PER_WORD;

    if (!(filter->bitmap[bitmapIndex] & (1ULL << bitPosition)))
    {
        return 0;
    }


    /* Precise tag matching */
    TagNode *current = filter->buckets[index];
    while (current != NULL)
    {
        if (strncmp(current->tag, tag, sizeof(current->tag)) == 0)
        {
            return 1;
        }
        current = current->next;
    }

    return 0;
}

/**
 * @brief  Parses a filter string and adds tags to the filter
 * @param  filter Pointer to the LogFilter
 * @param  tagFilterStr Separated tag string with '|'
 * @return none
 *
 * Splits a separated tag string with '|' and adds each tag
 * Supports multiple tag configurations
 */
static void parseLogTagFilter(LogFilter *filter, const char *tagFilterStr)
{
    char tmpFilter[256];
    char *token = NULL;

    if (tagFilterStr == NULL || strlen(tagFilterStr) == 0)
    {
        return;
    }

    memset(tmpFilter, 0, sizeof(tmpFilter));
    strncpy(tmpFilter, tagFilterStr, sizeof(tmpFilter) - 1);

    tx_mutex_get(&logMutex, TX_WAIT_FOREVER);
    token = strtok(tmpFilter, "|");
    while (token != NULL)
    {
        addOneTag(filter, token);
        token = strtok(NULL, "|");
    }
    tx_mutex_put(&logMutex);
}

/**
 * @brief  Frees all resources allocated by the log filter
 * @param  filter Pointer to the LogFilter to be cleaned up
 * @return none
 *
 * Walks through all buckets and frees dynamically allocated nodes
 * Prevents memory leaks
 */
static void freeLogTagFilter(LogFilter *filter)
{
    TagNode *temp = NULL;
    TagNode *current = NULL;

    tx_mutex_get(&logMutex, TX_WAIT_FOREVER);
    for (int i = 0; i < MAX_HASH_BUCKET; i++)
    {
        current = filter->buckets[i];
        while (current != NULL)
        {
            temp = current;
            current = current->next;
            free(temp);
        }
        filter->buckets[i] = NULL;
    }
    tx_mutex_put(&logMutex);
}

/**
 * @brief  Print log timestamp before each log line.
 * @param  None
 * @retval None
 *
 * The purpose of this function is to print a timestamp before each log line.
 * However, since the daric does not have a in-chip RTC.
 * reading the timestamp from an external RTC via I2C takes too long time.
 * Therefore, we will temporarily keep an empty implementation of the interface
 */
static void printLogTimestamp(void)
{
    /*  */
    return;
}

/**
 * @brief  Prints log messages with specified information
 * @param  str Printf-style format string
 * @retval None
 *
 * Prints log messages with specified information.
 * All log content is output here.
 * A unified output for logs, 
 * allowing printf to be replaced with other interfaces such as UART or USB.
 */
static void printLog(const char *str)
{
    printf(str);
}

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
void daricLogInit(LogLevel level, const char *tagFilterStr)
{
    /* Initialize mutex for thread-safe logging */
    UINT status = tx_mutex_create(&logMutex, "logMutex", TX_INHERIT);
    if (status != TX_SUCCESS)
    {
        printf("Mutex creation error\r\n");
        return;
    }
    logTagFilterInit(&gLogFilter);
    gLogFilter.logLevel = level;

    parseLogTagFilter(&gLogFilter, tagFilterStr);
}

/**
 * @brief  Update a filter string and adds tags to the filter
 * @param  tagFilterStr Separated tag string with '|'
 * @return none
 *
 * Free gLogFilter dynamically allocated nodes
 * Splits a separated tag string with '|' and adds each tag
 * Supports multiple tag configurations
 */
void logUpdateTagFilter(const char *tagFilterStr)
{
    freeLogTagFilter(&gLogFilter);
    logTagFilterInit(&gLogFilter);

    parseLogTagFilter(&gLogFilter, tagFilterStr);
}

/**
 * @brief  Dynamically change the current log output level.
 * @param  level New log level to set
 * @retval None
 *
 * Updates the current log level in a thread-safe manner.
 * Allows runtime modification of logging out level.
 */
void logUpdateLevel(LogLevel level)
{
    tx_mutex_get(&logMutex, TX_WAIT_FOREVER);
    gLogFilter.logLevel = level;
    tx_mutex_put(&logMutex);
}

/**
 * @brief  Return the current log level.
 * @param  None
 * @retval LogLevel Current log level in use
 *
 * Returns the current log level without modifying it.
 * Provides a way to check the current logging out level.
 */
LogLevel logGetLevel(void)
{
    return gLogFilter.logLevel;
}

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
void logPrintSingle(LogLevel level, const char* tag, LogColor ansiColor, const char *format, ...)
{
    char tagstr[MAX_TAG_LENGTH];

    /* Avoid being called in interrupt functions. */
    if (__get_IPSR() != 0){
        return;
    }

    /* Skip logging if current level is higher than set level */
    if (level < gLogFilter.logLevel)
    {
        return;
    }

    memset(tagstr, 0, sizeof(tagstr));
    strncpy(tagstr, tag, sizeof(tagstr) - 1);

    if (gLogFilter.tagNum > 0)
    {
        if (!isTagEnabled(&gLogFilter, tagstr))
        {
            return;
        }
    }

    /* Acquire mutex for thread-safe logging */
    tx_mutex_get(&logMutex, TX_WAIT_FOREVER);

    va_list args;
    va_start(args, format);

    /* Colorize terminal log with escape sequences */
    if (ansiColor < COL_MAX)
    {
        printLog(gLogColorArr[ansiColor]);
    }

    /* Print formatted message */
    memset(gStrLineBuff, 0, sizeof(gStrLineBuff));
    vsnprintf(gStrLineBuff, sizeof(gStrLineBuff), format, args);
    printLog(gStrLineBuff);

    /* Closing the escape sequence */
    if (ansiColor < COL_MAX)
    {
        printLog(END_ANSICOLOR_ESCAPE);
    }

    va_end(args);

    /* Release mutex */
    tx_mutex_put(&logMutex);
}

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
void logPrintDetail(LogLevel level, const char* tag, LogColor ansiColor, const char *file, int line, const char *format, ...)
{
    char tagstr[MAX_TAG_LENGTH];

    /* Avoid being called in interrupt functions. */
    if (__get_IPSR() != 0){
        return;
    }

    /* Skip logging if current level is higher than set level */
    if (level < gLogFilter.logLevel)
    {
        return;
    }

    memset(tagstr, 0, sizeof(tagstr));
    strncpy(tagstr, tag, sizeof(tagstr) - 1);

    if (gLogFilter.tagNum > 0)
    {
        if (!isTagEnabled(&gLogFilter, tagstr))
        {
            return;
        }
    }

    /* Acquire mutex for thread-safe logging */
    tx_mutex_get(&logMutex, TX_WAIT_FOREVER);

    va_list args;
    va_start(args, format);

    /* Colorize terminal log with escape sequences */
    if (ansiColor < COL_MAX)
    {
        printLog(gLogColorArr[ansiColor]);
    }

    /* Print log timestamp */
    printLogTimestamp();

    /* Print log header with level, file, and line */
    memset(gStrLineBuff, 0, sizeof(gStrLineBuff));
    snprintf(gStrLineBuff, sizeof(gStrLineBuff), "[%s][%s] %s:%d: ",
           logLevelStrings[level], tag,
           file,
           line);
    printLog(gStrLineBuff);

    /* Print formatted message */
    memset(gStrLineBuff, 0, sizeof(gStrLineBuff));
    vsnprintf(gStrLineBuff, sizeof(gStrLineBuff), format, args);
    printLog(gStrLineBuff);

    /* Closing the escape sequence */
    if (ansiColor < COL_MAX)
    {
        printLog(END_ANSICOLOR_ESCAPE);
    }

    printLog("\r\n");

    va_end(args);

    /* Release mutex */
    tx_mutex_put(&logMutex);
}

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
void logPrint(LogLevel level, const char* tag, LogColor ansiColor, const char *format, ...)
{
    char tagstr[MAX_TAG_LENGTH];

    /* Avoid being called in interrupt functions. */
    if (__get_IPSR() != 0){
        return;
    }

    /* Skip logging if current level is higher than set level */
    if (level < gLogFilter.logLevel)
    {
        return;
    }

    memset(tagstr, 0, sizeof(tagstr));
    strncpy(tagstr, tag, sizeof(tagstr) - 1);

    if (gLogFilter.tagNum > 0)
    {
        if (!isTagEnabled(&gLogFilter, tagstr))
        {
            return;
        }
    }

    /* Acquire mutex for thread-safe logging */
    tx_mutex_get(&logMutex, TX_WAIT_FOREVER);

    va_list args;
    va_start(args, format);

    /* Colorize terminal log with escape sequences */
    if (ansiColor < COL_MAX)
    {
        printLog(gLogColorArr[ansiColor]);
    }

    /* Print log timestamp */
    printLogTimestamp();

    /* Print log header with level */
    memset(gStrLineBuff, 0, sizeof(gStrLineBuff));
    snprintf(gStrLineBuff, sizeof(gStrLineBuff), "[%s][%s]:", logLevelStrings[level], tag);
    printLog(gStrLineBuff);

    /* Print formatted message */
    memset(gStrLineBuff, 0, sizeof(gStrLineBuff));
    vsnprintf(gStrLineBuff, sizeof(gStrLineBuff), format, args);
    printLog(gStrLineBuff);

    /* Closing the escape sequence */
    if (ansiColor < COL_MAX)
    {
        printLog(END_ANSICOLOR_ESCAPE);
    }

    printLog("\r\n");

    va_end(args);

    /* Release mutex */
    tx_mutex_put(&logMutex);
}
/************************************* END OF FILE ************************************/
