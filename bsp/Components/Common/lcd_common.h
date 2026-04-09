/**
******************************************************************************
* @file    lcd_common.h
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the lcd_common.c driver.
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
#ifndef LCD_COMMON_H
#define LCD_COMMON_H

#include "lcd_array_data.h"
#include <stdint.h>
#include "daric_gpio.h"
#include "daric_hal_def.h"
#include "daric_hal_spim.h"
#include "daric_hal_gpio.h"
#include "daric_udma_spim_v3.h"
#include <stdio.h>
#include <tx_api.h>

#define LCD_INSTANCES_NBR 1U
#define LUTD_SIZE         16385
#define LUTC_SIZE         64
#define LUTR_SIZE         256

/**
 * @brief LCD color enumeration
 */
typedef enum
{
    LCD_COLOR_BLACK = 0,
    LCD_COLOR_WHITE = 1,
} LCD_Color;

typedef enum
{
    INIT,
    A2,
    DU2,
    GL16,
    GC16,
    LCD_MODE_INVALID = -1 // Invalid mode
} LCD_Mode;

/**
 * @brief LCD color format
 */
typedef enum
{
    LCD_RGB565,
    LCD_RGB666,
    LCD_RGB888
} LCD_ColorFormat_t;

/**
 * @brief LCD panel interface type
 */
typedef enum
{
    LCD_PANEL_IF_TYPE_SPI,
    LCD_PANEL_IF_TYPE_MIPI,
    LCD_PANEL_IF_TYPE_EDP,
    LCD_PANEL_IF_TYPE_DSI,
} LCD_PanelIfType_t;

/**
 * @brief LCD LUT data structure
 */
typedef struct
{
    uint8_t  lutd[LUTD_SIZE];
    uint8_t  lutc[LUTC_SIZE];
    uint8_t  lutr[LUTR_SIZE];
    uint32_t lutd_count;
    bool     loaded;
} LUT_Data_t;

/**
 * @brief LCD GPIO configuration structure
 */
const typedef struct
{
    uint32_t     *pin;
    GPIO_TypeDef *port;
    uint32_t      mode;
    uint32_t      pull;
    const char   *name;
    bool          is_required; // true: required, false: not required
} LCD_GPIO_Config_t;

/**
 * @brief LCD context structure
 */
typedef struct
{
    const char   *lcd_name;      /**< LCD name */
    uint16_t      width;         /**< LCD width */
    uint16_t      height;        /**< LCD height */
    uint8_t       bpp;           /**< Bits per pixel */
    uint8_t       panel_if_type; /**< Panel interface type */
    GPIO_TypeDef *dc_port;       /**< Data/command select port */
    uint32_t      dc_pin;        /**< Data/command select pin */
    GPIO_TypeDef *vcien_port;    /**< Power enable port */
    uint32_t      vcien_pin;     /**< Power enable pin */
    GPIO_TypeDef *rst_port;      /**< Reset port */
    uint32_t      rst_pin;       /**< Reset pin */
    GPIO_TypeDef *busy_port;     /**< Busy status port */
    uint32_t      busy_pin;      /**< Busy status pin */
    GPIO_TypeDef *vdd1_port;     /**< VDD1 port */
    uint32_t      vdd1_pin;      /**< VDD1 pin */
    GPIO_TypeDef *vdd2_port;     /**< VDD2 port */
    uint32_t      vdd2_pin;      /**< VDD2 pin */
    GPIO_TypeDef *vpp1_port;     /**< VPP1 port */
    uint32_t      vpp1_pin;      /**< VPP1 pin */
    GPIO_TypeDef *vpp2_port;     /**< VPP2 port */
    uint32_t      vpp2_pin;      /**< VPP2 pin */

    uint32_t one_frame_data_size; /**< One Frame data size */
} BSP_LCD_Cfg_t;

/**
 * @brief LCD operations structure
 */
typedef struct
{
    /* LCD generic APIs: Initialization and control */
    HAL_StatusTypeDef (*Init)(uint32_t Instance);
    HAL_StatusTypeDef (*FastInit)(uint32_t Instance, uint8_t *pBmp);
    HAL_StatusTypeDef (*DeInit)(uint32_t Instance);
    HAL_StatusTypeDef (*Power)(uint32_t Instance, bool power);
    HAL_StatusTypeDef (*Reset)(uint32_t Instance);
    HAL_StatusTypeDef (*ReadID)(uint32_t Instance, uint32_t *id);
    HAL_StatusTypeDef (*EnterDeepSleep)(uint32_t Instance);
    HAL_StatusTypeDef (*ExitDeepSleep)(uint32_t Instance);
    HAL_StatusTypeDef (*Clear)(uint32_t Instance, uint32_t color);

    /* LCD generic APIs: Display control */
    HAL_StatusTypeDef (*DisplayOn)(uint32_t Instance);
    HAL_StatusTypeDef (*DisplayOff)(uint32_t Instance);
    HAL_StatusTypeDef (*SetBrightness)(uint32_t Instance, uint8_t brightness);
    HAL_StatusTypeDef (*GetBrightness)(uint32_t Instance, uint8_t *brightness);
    HAL_StatusTypeDef (*GetXSize)(uint32_t Instance, uint32_t *XSize);
    HAL_StatusTypeDef (*GetYSize)(uint32_t Instance, uint32_t *YSize);
    HAL_StatusTypeDef (*GetResolution)(uint32_t Instance, uint32_t *GetResolution);

    /* LCD generic APIs: Draw operations */
    HAL_StatusTypeDef (*ReadPixel)(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t *color);
    HAL_StatusTypeDef (*WritePixel)(uint32_t Instance, uint16_t x, uint16_t y, uint32_t color);
    HAL_StatusTypeDef (*SetOrientation)(uint32_t Instance, uint8_t orientation);
    HAL_StatusTypeDef (*SetWindow)(uint32_t Instance, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
    HAL_StatusTypeDef (*DisplayImage)(uint32_t Instance, uint8_t display_mode);
    HAL_StatusTypeDef (*DisplayString)(uint32_t Instance, const char *str, uint16_t currentX, uint16_t currentY);
    HAL_StatusTypeDef (*DrawBitmap)(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp);
    HAL_StatusTypeDef (*DrawHLine)(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
    HAL_StatusTypeDef (*DrawVLine)(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
    HAL_StatusTypeDef (*FillRect)(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);
    HAL_StatusTypeDef (*FillRGBRect)(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint8_t *pData);
    HAL_StatusTypeDef (*SwitchMode)(uint32_t Instance, LCD_Mode Mode);
} BSP_LCD_Ops_t;

/**
 * @brief LCD driver structure
 */
typedef struct
{
    BSP_LCD_Cfg_t *info; /**< Pointer to LCD context */
    BSP_LCD_Ops_t *ops;  /**< Pointer to LCD operations */
} BSP_LCD_Driver;

/**
 * @brief LCD panel configuration structure
 */
typedef struct
{
    uint32_t        lcd_id; /**< LCD ID */
    BSP_LCD_Driver *drv;    /**< Pointer to LCD driver */
} BSP_LCD_Panel_Cfg;

// External declarations
extern SPIM_HandleTypeDef s_hspim_lcd;
extern BSP_LCD_Cfg_t      Lcd_Ctx[LCD_INSTANCES_NBR];
extern BSP_LCD_Driver    *daric_panel_driver;
void                      SPI_WriteByte(uint8_t *buf, uint32_t len);                  // SPI write byte function
void                      SPI_ReadByte(uint8_t *buf, uint32_t len);                   // SPI read byte function
void                      SPI_Write_Mul_Byte(uint8_t *txbuf, uint32_t txlen);         // SPI write byte with chip select function
void                      SPI_Transmit(uint8_t *txbuf, uint8_t *rxbuf, uint32_t len); // SPI transmit function
HAL_StatusTypeDef         lcd_common_probe(uint32_t Instance);                        // LCD common probe function
HAL_StatusTypeDef         lcd_common_remove(uint32_t Instance);                       // LCD common remove function
// end of external declarations

// define log level
typedef enum
{
    LCD_LOG_LEVEL_NONE = 0,
    LCD_LOG_LEVEL_ERROR,
    LCD_LOG_LEVEL_WARN,
    LCD_LOG_LEVEL_INFO,
    LCD_LOG_LEVEL_DEBUG
} LCD_LOG_LEVEL;

// set current log level, can be modified at compile time or runtime
#ifndef LCD_CURRENT_LOG_LEVEL
#define LCD_CURRENT_LOG_LEVEL LCD_LOG_LEVEL_INFO
#endif
// log output macro
#define LCD_LOG(level, fmt, args...)                                      \
    do                                                                    \
    {                                                                     \
        if (level <= LCD_CURRENT_LOG_LEVEL)                               \
        {                                                                 \
            const char *level_str;                                        \
            switch (level)                                                \
            {                                                             \
                case LCD_LOG_LEVEL_ERROR:                                 \
                    level_str = "error";                                  \
                    break;                                                \
                case LCD_LOG_LEVEL_WARN:                                  \
                    level_str = "warn";                                   \
                    break;                                                \
                case LCD_LOG_LEVEL_INFO:                                  \
                    level_str = "info";                                   \
                    break;                                                \
                case LCD_LOG_LEVEL_DEBUG:                                 \
                    level_str = "debug";                                  \
                    break;                                                \
                default:                                                  \
                    level_str = "unknown";                                \
                    break;                                                \
            }                                                             \
            printf("[lcd_%s]%s: " fmt "\n", level_str, __func__, ##args); \
        }                                                                 \
    } while (0)

// log macro for each level
#define LCD_ERROR(fmt, args...) LCD_LOG(LCD_LOG_LEVEL_ERROR, fmt, ##args)
#define LCD_WARN(fmt, args...)  LCD_LOG(LCD_LOG_LEVEL_WARN, fmt, ##args)
#define LCD_INFO(fmt, args...)  LCD_LOG(LCD_LOG_LEVEL_INFO, fmt, ##args)
#define LCD_DEBUG(fmt, args...) LCD_LOG(LCD_LOG_LEVEL_DEBUG, fmt, ##args)

// log macro for function enter and exit
#define LCD_FUNC_ENTER() LCD_LOG(LCD_LOG_LEVEL_DEBUG, "Enter")
#define LCD_FUNC_EXIT()  LCD_LOG(LCD_LOG_LEVEL_DEBUG, "Exit(%d)", __LINE__)

#endif // LCD_COMMON_H
