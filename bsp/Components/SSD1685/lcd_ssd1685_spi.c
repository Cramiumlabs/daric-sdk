/**
******************************************************************************
* @file    lcd_ssd1685_spi.c
* @author  PERIPHERIAL BSP Team
* @brief   This file contains the common defines and functions prototypes for
*          the lcd_ssd1685_spi.c driver.
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
#define LCD_DC_PORT        CONFIG_LCD_DC_PORT
#define LCD_DC_PIN         CONFIG_LCD_DC_PIN
#define LCD_VCIEN_PORT     CONFIG_LCD_VCIEN_PORT
#define LCD_VCIEN_PIN      CONFIG_LCD_VCIEN_PIN
#define LCD_RST_PORT       CONFIG_LCD_RST_PORT
#define LCD_RST_PIN        CONFIG_LCD_RST_PIN
#define LCD_BUSY_PORT      CONFIG_LCD_BUSY_PORT
#define LCD_BUSY_PIN       CONFIG_LCD_BUSY_PIN
extern BSP_LCD_Cfg_t Lcd_Ctx[LCD_INSTANCES_NBR];

BSP_LCD_Cfg_t lcd_1685_info;

typedef enum
{
    PIC_BLACK = 0,
    PIC_WHITE = 255,
} DISPLAY_MODE;

#ifdef CONFIG_LCD_VCIEN_IS_REQUIRED
#define LCD_PWR(enable) HAL_GPIO_WritePin(lcd_1685_info.vcien_port, lcd_1685_info.vcien_pin, (enable) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#endif
#ifdef CONFIG_LCD_RST_IS_REQUIRED
#define LCD_RST(enable) HAL_GPIO_WritePin(lcd_1685_info.rst_port, lcd_1685_info.rst_pin, (enable) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#endif
#ifdef CONFIG_LCD_DC_IS_REQUIRED
#define LCD_WR_RS(enable) HAL_GPIO_WritePin(lcd_1685_info.dc_port, lcd_1685_info.dc_pin, (enable) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#endif
static HAL_StatusTypeDef SSD1685_Display_Fast_Init(uint32_t Instance, uint8_t *pBmp);

void SSD1685_Write_Cmd(uint8_t cmd)
{
#ifdef CONFIG_LCD_DC_IS_REQUIRED
    LCD_WR_RS(0);
#endif
    SPI_WriteByte(&cmd, 1);
}

void SSD1685_Write_Data(uint8_t dat)
{
#ifdef CONFIG_LCD_DC_IS_REQUIRED
    LCD_WR_RS(1);
#endif
    SPI_WriteByte(&dat, 1);
}

void SSD1685_Read_Data(uint8_t *buf, uint32_t len)
{
#ifdef CONFIG_LCD_DC_IS_REQUIRED
    LCD_WR_RS(1);
#endif
    SPI_ReadByte(buf, len);
}

static HAL_StatusTypeDef SSD1685_Wait_For_Ready(void)
{
    LCD_FUNC_ENTER();
    uint32_t timeout = HAL_GetTick() + 10000; // 10 seconds timeout
    while (HAL_GPIO_ReadPin(lcd_1685_info.busy_port, lcd_1685_info.busy_pin) == GPIO_PIN_SET)
    {
        if (HAL_GetTick() >= timeout)
        {
            return HAL_TIMEOUT;
        }
        HAL_Delay(10);
    }
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1685_Reset(uint32_t Instance)
{
#ifdef CONFIG_LCD_RST_IS_REQUIRED
    LCD_RST(1);
    HAL_Delay(10);
    LCD_RST(0);
    HAL_Delay(10);
    LCD_RST(1);
    HAL_Delay(10);
#endif
    return SSD1685_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1685_Initcode_Config(void)
{
    LCD_FUNC_ENTER();
    SSD1685_Write_Cmd(0x01); // Driver output control
    SSD1685_Write_Data(0x2B);
    SSD1685_Write_Data(0x01);
    SSD1685_Write_Data(0x00);

    SSD1685_Write_Cmd(0x11); // Data entry mode setting
    SSD1685_Write_Data(0x03);

    SSD1685_Write_Cmd(0x44);  // Set Ram X -address
    SSD1685_Write_Data(0x00); // Start 0
    SSD1685_Write_Data(0x18); // End   0x18=24 (24+1)x8=200

    SSD1685_Write_Cmd(0x45);  // set Ram y -address
    SSD1685_Write_Data(0x00); // RAM y address end at 00h;
    SSD1685_Write_Data(0x00);
    SSD1685_Write_Data(0x2B); // 0x12b = 299
    SSD1685_Write_Data(0x01);

    // SSD1685_Write_Cmd(0x3C);		// Set border
    // SSD1685_Write_Data(0x05);		// 0x01 border white   0x00 border black   0x07 border rad

    // SSD1685_Write_Cmd(0x21);
    // SSD1685_Write_Data(0x00);
    // SSD1685_Write_Data(0x00);   // 80:168X384   00:200X384
    LCD_FUNC_EXIT();
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1685_Set_Display_Window(uint32_t Instance, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    LCD_FUNC_ENTER();
    // Processing edge
    xEnd -= 1;
    yEnd -= 1;
    // Set X-axis start and end addresses (command 44h)
    SSD1685_Write_Cmd(0x44);                  // Set X-axis range command
    SSD1685_Write_Data((xStart >> 3) & 0xFF); // X start address (8 pixels per byte, so right shift by 3)
    SSD1685_Write_Data((xEnd >> 3) & 0xFF);   // X end address

    // Set Y-axis start and end addresses (command 45h)
    SSD1685_Write_Cmd(0x45);                  // Set Y-axis range command
    SSD1685_Write_Data(yStart & 0xFF);        // Y start address (low 8 bits)
    SSD1685_Write_Data((yStart >> 8) & 0xFF); // Y start address (high 8 bits)
    SSD1685_Write_Data(yEnd & 0xFF);          // Y end address (low 8 bits)
    SSD1685_Write_Data((yEnd >> 8) & 0xFF);   // Y end address (high 8 bits)

    // Set current X address counter (command 4Eh)
    SSD1685_Write_Cmd(0x4E);                  // Set current X address
    SSD1685_Write_Data((xStart >> 3) & 0xFF); // X start address

    // Set current Y address counter (command 4Fh)
    SSD1685_Write_Cmd(0x4F);                  // Set current Y address
    SSD1685_Write_Data(yStart & 0xFF);        // Y end address (low 8 bits)
    SSD1685_Write_Data((yStart >> 8) & 0xFF); // Y end address (high 8 bits)

    // SSD1685_Write_Cmd(0x24);                   // Set RAM write command

    return SSD1685_Wait_For_Ready();
}

/**
 * @brief Puts the LCD into deep sleep mode.
 *
 * @details This function sends the command to put the LCD into deep sleep mode,
 * which reduces power consumption when the display is not in use.
 *
 * @return Returns HAL_OK.
 */
static HAL_StatusTypeDef SSD1685_Enter_Deep_Sleep(uint32_t Instance)
{
    LCD_FUNC_ENTER();
    SSD1685_Write_Cmd(0x10);  // Deep sleep mode command
    SSD1685_Write_Data(0x01); // Enter deep sleep mode
    HAL_Delay(10);
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1685_Display_String(uint32_t Instance, const char *str, uint16_t currentX, uint16_t currentY)
{
    HAL_StatusTypeDef ret = HAL_OK;
    LCD_FUNC_ENTER();
    // Write data for the entire string
    while (*str)
    {
        uint8_t      ascii = (uint8_t)*str;
        unsigned int pcnt  = (ascii - 32) * (LCD_ASCII_SIZE_ROW * (LCD_ASCII_SIZE_COL >> 3));
        LCD_INFO("Display character: %c, ASCII: %d, pcnt: %d\n", *str, ascii - 32, pcnt);
        // Check if line break is needed
        if (currentX + LCD_ASCII_SIZE_COL > lcd_1685_info.width)
        {
            currentX = 0;
            currentY += LCD_ASCII_SIZE_ROW;
        }

        SSD1685_Write_Cmd(0x21);
        SSD1685_Write_Data(0x00);
        SSD1685_Write_Data(0x00);
        // Set display window
        ret = SSD1685_Set_Display_Window(Instance, currentX, currentY, currentX + LCD_ASCII_SIZE_COL - 1, currentY + LCD_ASCII_SIZE_ROW - 1);
        if (ret != HAL_OK)
        {
            LCD_ERROR("SSD1685_Set_Display_Window failed!\n");
            return ret;
        }
        // Write data
        for (int col = 0; col < LCD_ASCII_SIZE_ROW; col++)
        {
            for (int row = 0; row < (LCD_ASCII_SIZE_COL >> 3); row++)
            {
                SSD1685_Write_Data(~ASCII_24X48[pcnt]); // Restore inversion operation
                pcnt++;
            }
        }
        // Move to next character position
        currentX += LCD_ASCII_SIZE_COL;
        str++;
    }

    // Update display
    SSD1685_Write_Cmd(0x3c);
    SSD1685_Write_Data(0Xc1);
    SSD1685_Write_Cmd(0x18);
    SSD1685_Write_Data(0X80);
    SSD1685_Write_Cmd(0x22);
    SSD1685_Write_Data(0xFF);
    SSD1685_Write_Cmd(0x20);
    HAL_Delay(10);
    return SSD1685_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1685_Display_Image(uint32_t Instance, DISPLAY_MODE mode)
{
    unsigned int row, col;
    unsigned int pcnt = 0;
    LCD_FUNC_ENTER();
    SSD1685_Write_Cmd(0x21);
    SSD1685_Write_Data(0x40);
    SSD1685_Write_Data(0x00);
    SSD1685_Set_Display_Window(Instance, 0, 0, lcd_1685_info.width, lcd_1685_info.height);
    SSD1685_Write_Cmd(0x24);
    for (col = 0; col < lcd_1685_info.height; col++)
    {
        for (row = 0; row < (lcd_1685_info.width >> 3); row++)
        {
            switch (mode)
            {
                case PIC_WHITE:
                    SSD1685_Write_Data(0xff);
                    break;
                case PIC_BLACK:
                    SSD1685_Write_Data(0x00);
                    break;
                default:
                    SSD1685_Write_Data(0xff);
                    break;
            }
            pcnt++;
        }
    }
    SSD1685_Write_Cmd(0x26);
    pcnt = 0;
    for (col = 0; col < lcd_1685_info.height; col++)
    {
        for (row = 0; row < (lcd_1685_info.width >> 3); row++)
        {
            switch (mode)
            {
                case PIC_WHITE:
                    SSD1685_Write_Data(0xff);
                    break;
                case PIC_BLACK:
                    SSD1685_Write_Data(0x00);
                    break;
                default:
                    SSD1685_Write_Data(0xff);
                    break;
            }
            pcnt++;
        }
    }
    // Set display update control
    SSD1685_Write_Cmd(0x3c);
    SSD1685_Write_Data(0Xc1);
    SSD1685_Write_Cmd(0x18);
    SSD1685_Write_Data(0X80);

    // Display update control 2
    SSD1685_Write_Cmd(0x22);
    SSD1685_Write_Data(0xF7);
    SSD1685_Write_Cmd(0x20);
    HAL_Delay(1);
    return SSD1685_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1685_Draw_Bitmap(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp)
{
    LCD_FUNC_ENTER();
    //--------------------------
    SSD1685_Reset(Instance);
    SSD1685_Write_Cmd(0x12); // Software reset
    SSD1685_Wait_For_Ready();
    SSD1685_Initcode_Config(); // Initialize LCD
    //---------------------------
    // Check parameter validity
    if (pBmp == NULL)
    {
        return HAL_ERROR;
    }
    SSD1685_Write_Cmd(0x3C);  // Set border
    SSD1685_Write_Data(0x01); // 0x01 border white   0x00 border black   0x07 border rad

    SSD1685_Write_Cmd(0x21);
    SSD1685_Write_Data(0x40);
    SSD1685_Write_Data(0x00);
    // Set display window
    HAL_StatusTypeDef status = SSD1685_Set_Display_Window(Instance, Xpos, Ypos, lcd_1685_info.width, lcd_1685_info.height);
    if (status != HAL_OK)
    {
        return status;
    }

    // Write bitmap data
    SSD1685_Write_Cmd(0x24); // Write to RAM
    LCD_WR_RS(1);
    SPI_WriteByte(pBmp, lcd_1685_info.width * lcd_1685_info.height / 8);

    SSD1685_Write_Cmd(0x18); // Display update control
    SSD1685_Write_Data(0x80);
    // Refresh display
    SSD1685_Write_Cmd(0x22); // Display update control
    SSD1685_Write_Data(0xF7);
    SSD1685_Write_Cmd(0x20); // Activate display update sequence

    return SSD1685_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1685_Draw_HLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
    LCD_FUNC_ENTER();

    // Check parameter validity
    if (Xpos >= lcd_1685_info.width || Ypos >= lcd_1685_info.height || (Xpos + Length) > lcd_1685_info.width)
    {
        LCD_ERROR("Invalid parameters\r\n");
        return HAL_ERROR;
    }

    // Set display window
    HAL_StatusTypeDef status = SSD1685_Set_Display_Window(Instance, Xpos, Ypos, Xpos + Length, Ypos + 1);
    if (status != HAL_OK)
    {
        LCD_ERROR("Failed to set display window\r\n");
        return status;
    }

    // Prepare data
    uint8_t data = (Color == LCD_COLOR_BLACK) ? 0x00 : 0xFF;
    LCD_INFO("data: 0x%x\r\n", data);
    // Write data
    SSD1685_Write_Cmd(0x24); // Write to RAM
    for (uint32_t i = 0; i < Length; i++)
    {
        SSD1685_Write_Data(data);
    }

    // Refresh display
    SSD1685_Write_Cmd(0x22); // Display update control
    SSD1685_Write_Data(0xFF);
    SSD1685_Write_Cmd(0x20); // Activate display update sequence

    return SSD1685_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1685_Draw_VLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
    LCD_FUNC_ENTER();

    // Check parameter validity
    if (Xpos >= lcd_1685_info.width || Ypos >= lcd_1685_info.height || (Ypos + Length) > lcd_1685_info.height)
    {
        LCD_ERROR("Invalid parameters\r\n");
        return HAL_ERROR;
    }

    // Set display window
    HAL_StatusTypeDef status = SSD1685_Set_Display_Window(Instance, Xpos, Ypos, Xpos + 1, Ypos + Length);
    if (status != HAL_OK)
    {
        LCD_ERROR("Failed to set display window\r\n");
        return status;
    }

    // Prepare data
    uint8_t data = (Color == LCD_COLOR_BLACK) ? 0xEF : 0xFF; // Control black line thickness. 0x00 for 8 pixels thick, 0xFE for 1 pixel thick.
    LCD_INFO("data: 0x%x\r\n", data);

    // Write data
    SSD1685_Write_Cmd(0x24); // Write to RAM
    for (uint32_t i = 0; i < Length; i++)
    {
        SSD1685_Write_Data(data);
    }

    // Refresh display
    SSD1685_Write_Cmd(0x22); // Display update control
    SSD1685_Write_Data(0xFF);
    SSD1685_Write_Cmd(0x20); // Activate display update sequence

    return SSD1685_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1685_Fill_Rect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color)
{
    LCD_FUNC_ENTER();

    // Check parameter validity
    if (Xpos >= lcd_1685_info.width || Ypos >= lcd_1685_info.height || (Xpos + Width) > lcd_1685_info.width || (Ypos + Height) > lcd_1685_info.height)
    {
        LCD_ERROR("Invalid parameters\r\n");
        return HAL_ERROR;
    }

    // Set display window
    HAL_StatusTypeDef status = SSD1685_Set_Display_Window(Instance, Xpos, Ypos, Xpos + Width, Ypos + Height);
    if (status != HAL_OK)
    {
        LCD_ERROR("Failed to set display window\r\n");
        return status;
    }

    // Prepare data
    uint8_t data = (Color == LCD_COLOR_BLACK) ? 0x00 : 0xFF;
    LCD_INFO("data: 0x%x\r\n", data);

    // Write data
    SSD1685_Write_Cmd(0x24); // Write to RAM
    for (uint32_t i = 0; i < Width * Height / 8; i++)
    {
        SSD1685_Write_Data(data);
    }

    // Refresh display
    SSD1685_Write_Cmd(0x22); // Display update control
    SSD1685_Write_Data(0xFF);
    SSD1685_Write_Cmd(0x20); // Activate display update sequence

    return SSD1685_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1685_Fill_RGB_Rect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint8_t *pData)
{
    LCD_FUNC_ENTER();

    // Check parameter validity
    if (Xpos >= lcd_1685_info.width || Ypos >= lcd_1685_info.height || (Xpos + Width) > lcd_1685_info.width || (Ypos + Height) > lcd_1685_info.height)
    {
        LCD_ERROR("Invalid parameters\r\n");
        return HAL_ERROR;
    }

    // Set display window
    HAL_StatusTypeDef status = SSD1685_Set_Display_Window(Instance, Xpos, Ypos, Xpos + Width, Ypos + Height);
    if (status != HAL_OK)
    {
        LCD_ERROR("Failed to set display window\r\n");
        return status;
    }

    SSD1685_Write_Cmd(0x24); // Write to RAM

    // Write data
    LCD_WR_RS(1);
    SPI_WriteByte(pData, Width * Height / 8);

    SSD1685_Write_Cmd(0x3C);  // Set border
    SSD1685_Write_Data(0xC1); // 0x01 border white   0x00 border black   0x07 border rad

    SSD1685_Write_Cmd(0x18);
    SSD1685_Write_Data(0x80);
    // Refresh display
    SSD1685_Write_Cmd(0x22); // Display update control
    SSD1685_Write_Data(0xFF);
    SSD1685_Write_Cmd(0x20); // Activate display update sequence
    HAL_Delay(10);
    return SSD1685_Wait_For_Ready();
}

static HAL_StatusTypeDef SSD1685_Power(uint32_t Instance, bool power)
{
    LCD_FUNC_ENTER();
    if (power)
    {
        LCD_INFO("power on\r\n");
#ifdef CONFIG_LCD_VCIEN_IS_REQUIRED
        LCD_PWR(0);
        HAL_Delay(10);
        LCD_PWR(1);
#endif
    }
    else
    {
        LCD_INFO("power off\r\n");
#ifdef CONFIG_LCD_VCIEN_IS_REQUIRED
        LCD_PWR(0);
#endif
    }
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1685_ReadId(uint32_t Instance, uint32_t *id)
{
    return HAL_OK;
    uint8_t readId = 0;
    LCD_FUNC_ENTER();
    SSD1685_Write_Cmd(0x2E);       // Read ID register
    LCD_WR_RS(1);                  // Set to data mode
    SSD1685_Read_Data(&readId, 1); // Use SSD1685_Read_Data interface to read ID
    LCD_INFO("readId: 0x%x\r\n", readId);
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

static HAL_StatusTypeDef SSD1685_Init(uint32_t Instance)
{
    LCD_FUNC_ENTER();
    HAL_Delay(20);
    // Full refresh with white color.
    SSD1685_Reset(Instance);
    SSD1685_Write_Cmd(0x12); // Software reset
    SSD1685_Wait_For_Ready();
    SSD1685_Initcode_Config(); // Initialize LCD
    // SSD1685_Display_Image(Instance, PIC_WHITE);
    // SSD1685_Enter_Deep_Sleep(Instance);

    HAL_Delay(1000); // 1S
    __NOP();
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1685_Display_Fast_Init(uint32_t Instance, uint8_t *pBmp)
{
    LCD_FUNC_ENTER();
    SSD1685_Write_Cmd(0x21);
    SSD1685_Write_Data(0x00);
    SSD1685_Write_Data(0x00);
    SSD1685_Set_Display_Window(Instance, 0, 0, lcd_1685_info.width, lcd_1685_info.height);
    SSD1685_Write_Cmd(0x26);

    LCD_WR_RS(1);
    SPI_WriteByte(pBmp, lcd_1685_info.width * lcd_1685_info.height / 8);

    return HAL_OK;
}

static HAL_StatusTypeDef SSD1685_DeInit(uint32_t Instance)
{
    LCD_FUNC_ENTER();
    SSD1685_Enter_Deep_Sleep(Instance);
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1685_Display_On(uint32_t Instance)
{
    /* do nothing */
    LCD_FUNC_ENTER();
    return HAL_OK;
}

static HAL_StatusTypeDef SSD1685_Display_Off(uint32_t Instance)
{
    /* do nothing */
    LCD_FUNC_ENTER();
    return HAL_OK;
}

static BSP_LCD_Ops_t SSD1685_ops = {
    .Init          = SSD1685_Init,
    .FastInit      = SSD1685_Display_Fast_Init,
    .DeInit        = SSD1685_DeInit,
    .Power         = SSD1685_Power,
    .SetWindow     = SSD1685_Set_Display_Window,
    .ReadID        = SSD1685_ReadId,
    .Reset         = SSD1685_Reset,
    .DisplayOn     = SSD1685_Display_On,
    .DisplayOff    = SSD1685_Display_Off,
    .DisplayImage  = SSD1685_Display_Image,
    .DisplayString = SSD1685_Display_String,
    .DrawBitmap    = SSD1685_Draw_Bitmap,
    .DrawHLine     = SSD1685_Draw_HLine,
    .DrawVLine     = SSD1685_Draw_VLine,
    .FillRect      = SSD1685_Fill_Rect,
    .FillRGBRect   = SSD1685_Fill_RGB_Rect,
};

BSP_LCD_Cfg_t lcd_1685_info = {
    .lcd_name      = "ink_ssd1685_xingtai_spi_qvga",
    .width         = 200,
    .height        = 300,
    .bpp           = 1,
    .panel_if_type = LCD_PANEL_IF_TYPE_SPI,
#ifdef CONFIG_LCD_DC_PORT
    .dc_port = LCD_DC_PORT,
    .dc_pin  = LCD_DC_PIN,
#endif
#ifdef CONFIG_LCD_VCIEN_PORT
    .vcien_port = LCD_VCIEN_PORT,
    .vcien_pin  = LCD_VCIEN_PIN,
#endif
#ifdef CONFIG_LCD_RST_PORT
    .rst_port = LCD_RST_PORT,
    .rst_pin  = LCD_RST_PIN,
#endif
#ifdef CONFIG_LCD_BUSY_PORT
    .busy_port = LCD_BUSY_PORT,
    .busy_pin  = LCD_BUSY_PIN,
#endif
};

BSP_LCD_Driver lcd_1685_driver = {
    .info = &lcd_1685_info,
    .ops  = &SSD1685_ops,
};
