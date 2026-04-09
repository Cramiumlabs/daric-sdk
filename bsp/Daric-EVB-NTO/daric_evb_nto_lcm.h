/**
 ******************************************************************************
 * @file    daric_evb_nto_lcm.h
 * @author  PERIPHERIAL BSP Team
 * @brief   This file contains the common defines and functions prototypes for
 *          the daric_evb_nto_lcm.c driver.
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
#ifndef DARIC_EVB_NTO_LCM_H
#define DARIC_EVB_NTO_LCM_H

#include "lcd_common.h"

/**
 * @brief Initializes the LCD.
 *
 * @details This function initializes the LCD with the specified orientation.
 *
 * @param Instance The instance of the LCD.
 * @param Orientation The orientation of the LCD.
 *
 * @return Returns BSP_ERROR_NONE if the initialization is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_Init(uint32_t Instance, uint32_t Orientation);

/**
 * @brief Fast initializes the LCD.
 *
 * @details This function fast initializes the LCD.
 *
 * @param Instance The instance of the LCD.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_FastInit(uint32_t Instance, uint8_t *pBmp);

/** * @brief Switches the LCD mode.
 *
 * @details This function switches the LCD mode.
 *
 * @param Mode The mode to switch to.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_SwitchMode(uint32_t Instance, LCD_Mode Mode);

/**
 * @brief Deinitializes the LCD.
 *
 * @details This function deinitializes the LCD.
 *
 * @param Instance The instance of the LCD.
 *
 * @return Returns BSP_ERROR_NONE if the deinitialization is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_DeInit(uint32_t Instance);

/**
 * @brief Turns on the LCD display.
 *
 * @details This function turns on the LCD display.
 *
 * @param Instance The instance of the LCD.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_DisplayOn(uint32_t Instance);

/**
 * @brief Turns off the LCD display.
 *
 * @details This function turns off the LCD display.
 *
 * @param Instance The instance of the LCD.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_DisplayOff(uint32_t Instance);

/**
 * @brief Resets the LCD.
 *
 * @details This function resets the LCD.
 *
 * @param Instance The instance of the LCD.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_Reset(uint32_t Instance);

/**
 * @brief Gets the X size of the LCD.
 *
 * @details This function gets the X size (width) of the LCD.
 *
 * @param Instance The instance of the LCD.
 * @param XSize Pointer to store the X size.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_GetXSize(uint32_t Instance, uint32_t *XSize);

/**
 * @brief Gets the Y size of the LCD.
 *
 * @details This function gets the Y size (height) of the LCD.
 *
 * @param Instance The instance of the LCD.
 * @param YSize Pointer to store the Y size.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_GetYSize(uint32_t Instance, uint32_t *YSize);

/**
 * @brief Displays a string on the LCD.
 *
 * @details This function displays a string at the specified position on the LCD.
 *
 * @param Instance The instance of the LCD.
 * @param str The string to display.
 * @param Xpos The X position to start displaying the string.
 * @param Ypos The Y position to start displaying the string.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_Display_String(uint32_t Instance, const char *str, uint32_t Xpos, uint32_t Ypos);

/**
 * @brief Draws a bitmap on the LCD.
 *
 * @details This function draws a bitmap at the specified position on the LCD.
 *
 * @param Instance The instance of the LCD.
 * @param Xpos The X position to start drawing the bitmap.
 * @param Ypos The Y position to start drawing the bitmap.
 * @param pBmp Pointer to the bitmap data.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_DrawBitmap(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp);

/**
 * @brief Draws a horizontal line on the LCD.
 *
 * @details This function draws a horizontal line at the specified position on the LCD.
 *
 * @param Instance The instance of the LCD.
 * @param Xpos The X position to start drawing the line.
 * @param Ypos The Y position to start drawing the line.
 * @param Length The length of the line.
 * @param Color The color of the line.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_DrawHLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);

/**
 * @brief Draws a vertical line on the LCD.
 *
 * @details This function draws a vertical line at the specified position on the LCD.
 *
 * @param Instance The instance of the LCD.
 * @param Xpos The X position to start drawing the line.
 * @param Ypos The Y position to start drawing the line.
 * @param Length The length of the line.
 * @param Color The color of the line.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_DrawVLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);

/**
 * @brief Fills a rectangle on the LCD.
 *
 * @details This function fills a rectangle at the specified position on the LCD with the specified color.
 *
 * @param Instance The instance of the LCD.
 * @param Xpos The X position to start filling the rectangle.
 * @param Ypos The Y position to start filling the rectangle.
 * @param Width The width of the rectangle.
 * @param Height The height of the rectangle.
 * @param Color The color to fill the rectangle with.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_FillRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);

/**
 * @brief Fills a RGB rectangle on the LCD.
 *
 * @details This function fills a RGB rectangle at the specified position on the LCD with the specified color.
 *
 * @param Instance The instance of the LCD.
 * @param Xpos The X position to start filling the rectangle.
 * @param Ypos The Y position to start filling the rectangle.
 * @param Width The width of the rectangle.
 * @param Height The height of the rectangle.
 * @param pData The pointer to the RGB data.
 *
 * @return Returns BSP_ERROR_NONE if the operation is successful, BSP_ERROR_NO_INIT otherwise.
 */
int32_t BSP_LCD_FillRGBRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint8_t *pData);

#endif // DARIC_LCD_H
