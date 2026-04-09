/**
******************************************************************************
* @file    lcd_ssd1680_spi.c
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the lcd_ssd1680_spi.c driver.
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
#include "daric_gpio.h"
#include "daric_hal_def.h"
#include "daric_hal_spim.h"
#include "daric_hal_gpio.h"
#include "daric_udma_spim_v3.h"
#include "lcd_common.h"

#include <stdio.h>
#include <tx_api.h>

// ASCII font size - 24x48 pixels
#define LCD_ASCII_SIZE_ROW 48
#define LCD_ASCII_SIZE_COL 24

extern BSP_LCD_Cfg_t Lcd_Ctx[LCD_INSTANCES_NBR];

BSP_LCD_Cfg_t lcd_1680_info;

typedef enum
{
    PIC_BLACK = 0,
    PIC_WHITE = 255,
} DISPLAY_MODE;

#define LCD_PWR(enable)   HAL_GPIO_WritePin(lcd_1680_info.vcien_port, lcd_1680_info.vcien_pin, (enable) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define LCD_RST(enable)   HAL_GPIO_WritePin(lcd_1680_info.rst_port, lcd_1680_info.rst_pin, (enable) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define LCD_WR_RS(enable) HAL_GPIO_WritePin(lcd_1680_info.dc_port, lcd_1680_info.dc_pin, (enable) ? GPIO_PIN_SET : GPIO_PIN_RESET)

void SSD1680_Write_Cmd(uint8_t cmd)
{
    LCD_WR_RS(0);
    SPI_WriteByte(&cmd, 1);
}

void SSD1680_Write_Data(uint8_t dat)
{
    LCD_WR_RS(1);
    SPI_WriteByte(&dat, 1);
}

void SSD1680_Read_Data(uint8_t *buf, uint32_t len)
{
    LCD_WR_RS(1);
    SPI_ReadByte(buf, len);
}

static HAL_StatusTypeDef SSD1680_Wait_For_Ready(void)
{
    uint32_t timeout = HAL_GetTick() + 10000; // 10 seconds timeout
    while (HAL_GPIO_ReadPin(lcd_1680_info.busy_port, lcd_1680_info.busy_pin) == GPIO_PIN_SET)
    {
        if (HAL_GetTick() >= timeout)
        {
            return HAL_TIMEOUT;
        }
        HAL_Delay(10);
    }
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1680_Reset(uint32_t Instance)
{
    LCD_RST(0);
    HAL_Delay(20);
    LCD_RST(1);
    HAL_Delay(20);
    return SSD1680_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1680_Initcode_Config(void)
{
    SSD1680_Write_Cmd(0x01); // Driver output control
    SSD1680_Write_Data(0x07);
    SSD1680_Write_Data(0x01);
    SSD1680_Write_Data(0x00);

    SSD1680_Write_Cmd(0x11); // Data entry mode setting
    SSD1680_Write_Data(0x03);

    SSD1680_Write_Cmd(0x44);  // Set Ram X -address
    SSD1680_Write_Data(0x00); // Start 0
    SSD1680_Write_Data(0x15);

    SSD1680_Write_Cmd(0x45);  // set Ram y -address
    SSD1680_Write_Data(0x00); // RAM y address end at 00h;
    SSD1680_Write_Data(0x00);
    SSD1680_Write_Data(0x07);
    SSD1680_Write_Data(0x01);

    SSD1680_Write_Cmd(0x3C);  // Set border
    SSD1680_Write_Data(0x05); // 0x01 border white   0x00 border black   0x07 border rad

    SSD1680_Write_Cmd(0x21);
    SSD1680_Write_Data(0x00);
    SSD1680_Write_Data(0x00); // 80:168X384   00:200X384
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1680_Set_Display_Window(uint32_t Instance, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    // Set X-axis start and end addresses (command 44h)
    SSD1680_Write_Cmd(0x44);                  // Set X-axis range command
    SSD1680_Write_Data((xStart >> 3) & 0xFF); // X start address (8 pixels per byte, so right shift by 3)
    SSD1680_Write_Data((xEnd >> 3) & 0xFF);   // X end address

    // Set Y-axis start and end addresses (command 45h)
    SSD1680_Write_Cmd(0x45);                  // Set Y-axis range command
    SSD1680_Write_Data(yStart & 0xFF);        // Y start address (low 8 bits)
    SSD1680_Write_Data((yStart >> 8) & 0xFF); // Y start address (high 8 bits)
    SSD1680_Write_Data(yEnd & 0xFF);          // Y end address (low 8 bits)
    SSD1680_Write_Data((yEnd >> 8) & 0xFF);   // Y end address (high 8 bits)

    // Set current X address counter (command 4Eh)
    SSD1680_Write_Cmd(0x4E);                  // Set current X address
    SSD1680_Write_Data((xStart >> 3) & 0xFF); // X start address

    // Set current Y address counter (command 4Fh)
    SSD1680_Write_Cmd(0x4F);                  // Set current Y address
    SSD1680_Write_Data(yStart & 0xFF);        // Y end address (low 8 bits)
    SSD1680_Write_Data((yStart >> 8) & 0xFF); // Y end address (high 8 bits)

    SSD1680_Write_Cmd(0x24); // Set RAM write command

    return SSD1680_Wait_For_Ready();
}

/**
 * @brief Puts the LCD into deep sleep mode.
 *
 * @details This function sends the command to put the LCD into deep sleep mode,
 * which reduces power consumption when the display is not in use.
 *
 * @return Returns HAL_OK.
 */
static HAL_StatusTypeDef SSD1680_Enter_Deep_Sleep(void)
{
    SSD1680_Write_Cmd(0x10);  // Deep sleep mode command
    SSD1680_Write_Data(0x01); // Enter deep sleep mode
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1680_Display_String(uint32_t Instance, const char *str, uint16_t currentX, uint16_t currentY)
{
    HAL_StatusTypeDef ret = HAL_OK;
    printf("Entering %s function\r\n", __FUNCTION__);
    // Write data for the entire string
    while (*str)
    {
        uint8_t      ascii = (uint8_t)*str;
        unsigned int pcnt  = (ascii - 32) * (LCD_ASCII_SIZE_ROW * (LCD_ASCII_SIZE_COL >> 3));
        // printf("Display character: %c, ASCII: %d, pcnt: %d\n", *str, ascii-32, pcnt);
        //  Check if line break is needed
        if (currentX + LCD_ASCII_SIZE_COL > lcd_1680_info.width)
        {
            currentX = 0;
            currentY += LCD_ASCII_SIZE_ROW;
        }
        // Set display window
        ret = SSD1680_Set_Display_Window(Instance, currentX, currentY, currentX + LCD_ASCII_SIZE_COL - 1, currentY + LCD_ASCII_SIZE_ROW - 1);
        if (ret != HAL_OK)
        {
            printf("SSD1680_Set_Display_Window failed!\n");
            return ret;
        }
        // Write data
        for (int col = 0; col < LCD_ASCII_SIZE_ROW; col++)
        {
            for (int row = 0; row < (LCD_ASCII_SIZE_COL >> 3); row++)
            {
                SSD1680_Write_Data(~ASCII_24X48[pcnt]); // Restore inversion operation
                pcnt++;
            }
        }
        // Move to next character position
        currentX += LCD_ASCII_SIZE_COL;
        str++;
    }

    // Update display
    SSD1680_Write_Cmd(0x18);
    SSD1680_Write_Data(0X80);
    SSD1680_Write_Cmd(0x22);
    SSD1680_Write_Data(0xFF);
    SSD1680_Write_Cmd(0x20);
    HAL_Delay(1);
    return SSD1680_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1680_Display_Image(uint32_t Instance, DISPLAY_MODE mode)
{
    unsigned int row, col;
    unsigned int pcnt = 0;
    SSD1680_Write_Cmd(0x24);
    for (col = 0; col < lcd_1680_info.height; col++)
    {
        for (row = 0; row < (lcd_1680_info.width >> 3); row++)
        {
            switch (mode)
            {
                case PIC_WHITE:
                    SSD1680_Write_Data(0xff);
                    break;
                case PIC_BLACK:
                    SSD1680_Write_Data(0x00);
                    break;
                default:
                    SSD1680_Write_Data(0xff);
                    break;
            }
            pcnt++;
        }
    }
    SSD1680_Write_Cmd(0x26);
    pcnt = 0;
    for (col = 0; col < lcd_1680_info.height; col++)
    {
        for (row = 0; row < (lcd_1680_info.width >> 3); row++)
        {
            switch (mode)
            {
                case PIC_WHITE:
                    SSD1680_Write_Data(0xff);
                    break;
                case PIC_BLACK:
                    SSD1680_Write_Data(0x00);
                    break;
                default:
                    SSD1680_Write_Data(0xff);
                    break;
            }
            pcnt++;
        }
    }
    // Set display update control
    SSD1680_Write_Cmd(0x18);
    SSD1680_Write_Data(0X80);

    // Display update control 2
    SSD1680_Write_Cmd(0x22);
    SSD1680_Write_Data(0xF7);
    SSD1680_Write_Cmd(0x20);
    HAL_Delay(1);
    return SSD1680_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1680_Draw_Bitmap(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp)
{
    printf("Entering %s function\r\n", __FUNCTION__);

    // Check parameter validity
    if (pBmp == NULL)
    {
        return HAL_ERROR;
    }

    // Set display window
    HAL_StatusTypeDef status = SSD1680_Set_Display_Window(Instance, Xpos, Ypos, lcd_1680_info.width, lcd_1680_info.height);
    if (status != HAL_OK)
    {
        return status;
    }

    // Write bitmap data
    SSD1680_Write_Cmd(0x24); // Write to RAM
    for (uint32_t i = 0; i < (lcd_1680_info.width * lcd_1680_info.height / 8); i++)
    {
        SSD1680_Write_Data(pBmp[i]);
    }

    // Refresh display
    SSD1680_Write_Cmd(0x22); // Display update control
    SSD1680_Write_Data(0xF7);
    SSD1680_Write_Cmd(0x20); // Activate display update sequence

    return SSD1680_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1680_Draw_HLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
    printf("Entering %s function\r\n", __FUNCTION__);

    // Check parameter validity
    if (Xpos >= lcd_1680_info.width || Ypos >= lcd_1680_info.height || (Xpos + Length) > lcd_1680_info.width)
    {
        printf("Invalid parameters\r\n");
        return HAL_ERROR;
    }

    // Set display window
    HAL_StatusTypeDef status = SSD1680_Set_Display_Window(Instance, Xpos, Ypos, Xpos + Length, Ypos + 1);
    if (status != HAL_OK)
    {
        printf("Failed to set display window\r\n");
        return status;
    }

    // Prepare data
    uint8_t data = (Color == LCD_COLOR_BLACK) ? 0x00 : 0xFF;
    printf("data: 0x%x\r\n", data);
    // Write data
    SSD1680_Write_Cmd(0x24); // Write to RAM
    for (uint32_t i = 0; i < Length; i++)
    {
        SSD1680_Write_Data(data);
    }

    // Refresh display
    SSD1680_Write_Cmd(0x22); // Display update control
    SSD1680_Write_Data(0xFF);
    SSD1680_Write_Cmd(0x20); // Activate display update sequence

    return SSD1680_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1680_Draw_VLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
    printf("Entering %s function\r\n", __FUNCTION__);

    // Check parameter validity
    if (Xpos >= lcd_1680_info.width || Ypos >= lcd_1680_info.height || (Ypos + Length) > lcd_1680_info.height)
    {
        printf("Invalid parameters\r\n");
        return HAL_ERROR;
    }

    // Set display window
    HAL_StatusTypeDef status = SSD1680_Set_Display_Window(Instance, Xpos, Ypos, Xpos + 1, Ypos + Length);
    if (status != HAL_OK)
    {
        printf("Failed to set display window\r\n");
        return status;
    }

    // Prepare data
    uint8_t data = (Color == LCD_COLOR_BLACK) ? 0xEF : 0xFF; // Control black line thickness. 0x00 for 8 pixels thick, 0xFE for 1 pixel thick.
    printf("data: 0x%x\r\n", data);

    // Write data
    SSD1680_Write_Cmd(0x24); // Write to RAM
    for (uint32_t i = 0; i < Length; i++)
    {
        SSD1680_Write_Data(data);
    }

    // Refresh display
    SSD1680_Write_Cmd(0x22); // Display update control
    SSD1680_Write_Data(0xFF);
    SSD1680_Write_Cmd(0x20); // Activate display update sequence

    return SSD1680_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1680_Fill_Rect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color)
{
    printf("Entering %s function\r\n", __FUNCTION__);

    // Check parameter validity
    if (Xpos >= lcd_1680_info.width || Ypos >= lcd_1680_info.height || (Xpos + Width) > lcd_1680_info.width || (Ypos + Height) > lcd_1680_info.height)
    {
        printf("Invalid parameters\r\n");
        return HAL_ERROR;
    }

    // Set display window
    HAL_StatusTypeDef status = SSD1680_Set_Display_Window(Instance, Xpos, Ypos, Width, Height);
    if (status != HAL_OK)
    {
        printf("Failed to set display window\r\n");
        return status;
    }

    // Prepare data
    uint8_t data = (Color == LCD_COLOR_BLACK) ? 0x00 : 0xFF;
    printf("data: 0x%x\r\n", data);

    // Write data
    SSD1680_Write_Cmd(0x24); // Write to RAM
    for (uint32_t i = 0; i < Width * Height; i++)
    {
        SSD1680_Write_Data(data);
    }

    // Refresh display
    SSD1680_Write_Cmd(0x22); // Display update control
    SSD1680_Write_Data(0xFF);
    SSD1680_Write_Cmd(0x20); // Activate display update sequence

    return SSD1680_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1680_Power(uint32_t Instance, bool power)
{
    if (power)
    {
        LCD_PWR(0);
        HAL_Delay(10);
        LCD_PWR(1);
    }
    else
    {
        LCD_PWR(0);
    }
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1680_ReadId(uint32_t Instance, uint32_t *id)
{
    // return HAL_OK;
    uint8_t readId = 0;

    SSD1680_Write_Cmd(0x2E);       // Read ID register
    LCD_WR_RS(1);                  // Set to data mode
    SSD1680_Read_Data(&readId, 1); // Use SSD1680_Read_Data interface to read ID
    printf("readId: 0x%x\r\n", readId);
    *id = readId;

    if (readId == 0xff)
    {
        return HAL_OK;
    }
    else
    {
        return HAL_ERROR;
    }
}

static HAL_StatusTypeDef SSD1680_Init(uint32_t Instance)
{
    printf("Enter %s\r\n", __FUNCTION__);
    HAL_Delay(20);
    //--------------------------------------------------------
    // Full refresh with white color.
    SSD1680_Reset(Instance);
    SSD1680_Write_Cmd(0x12); // Software reset
    SSD1680_Wait_For_Ready();
    SSD1680_Initcode_Config(); // Initialize LCD
    SSD1680_Display_Image(Instance, PIC_WHITE);
    SSD1680_Enter_Deep_Sleep();

    HAL_Delay(5000); // 1S
    __NOP();
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1680_DeInit(uint32_t Instance)
{
    printf("Enter %s\r\n", __FUNCTION__);
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1680_Display_On(uint32_t Instance)
{
    printf("Enter %s\r\n", __FUNCTION__);
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1680_Display_Off(uint32_t Instance)
{
    printf("Enter %s\r\n", __FUNCTION__);
    return HAL_OK;
}

static BSP_LCD_Ops_t SSD1680_ops = {
    .Init          = SSD1680_Init,
    .DeInit        = SSD1680_DeInit,
    .Power         = SSD1680_Power,
    .SetWindow     = SSD1680_Set_Display_Window,
    .ReadID        = SSD1680_ReadId,
    .Reset         = SSD1680_Reset,
    .DisplayOn     = SSD1680_Display_On,
    .DisplayOff    = SSD1680_Display_Off,
    .DisplayImage  = SSD1680_Display_Image,
    .DisplayString = SSD1680_Display_String,
    .DrawBitmap    = SSD1680_Draw_Bitmap,
    .DrawHLine     = SSD1680_Draw_HLine,
    .DrawVLine     = SSD1680_Draw_VLine,
    .FillRect      = SSD1680_Fill_Rect,
};

BSP_LCD_Cfg_t lcd_1680_info = { .lcd_name   = "ink_ssd1680_xingtai_spi_qvga",
                                .width      = 176,
                                .height     = 264,
                                .bpp        = 1,
                                .dc_port    = GPIOD,
                                .dc_pin     = GPIO_PIN_9,
                                .vcien_port = GPIOD,
                                .vcien_pin  = GPIO_PIN_11,
                                .rst_port   = GPIOD,
                                .rst_pin    = GPIO_PIN_12,
                                .busy_port  = GPIOC,
                                .busy_pin   = GPIO_PIN_13 };

BSP_LCD_Driver lcd_1680_driver = {
    .info = &lcd_1680_info,
    .ops  = &SSD1680_ops,
};
