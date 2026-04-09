/**
 ******************************************************************************
 * Copyright 2024-2026 CrossBar, Inc.
 * This file has been modified by CrossBar, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *******************************************************************************
 */

/******************************************************************************
  *
  *  Copyright (c) 2023, CEC Huada Electronic Design Co.,Ltd.
  *
  *  Licensed under the Apache License, Version 2.0 (the "License");
  *  you may not use this file except in compliance with the License.
  *  You may obtain a copy of the License at:
  *
  *  http://www.apache.org/licenses/LICENSE-2.0
  *
  *  Unless required by applicable law or agreed to in writing, software
  *  distributed under the License is distributed on an "AS IS" BASIS,
  *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  *  See the License for the specific language governing permissions and
  *  limitations under the License.
  *
  ******************************************************************************/
#include <stdarg.h>
#include "port.h"
#include "nfc_type.h"

#define LOG_BUFFER_SIZE 256
#define UART_TIMEOUT 1000

/* Global variables ----------------------------------------------------------*/
uint8_t trace_level;
char logger_buffer[LOG_BUFFER_SIZE];



char *bt_trace_getLevelName(uint8_t module)
{
	if(module == TRACE_LEVEL_FATAL)
	{
		return "Fatal";
	}
	else if(module == TRACE_LEVEL_ERROR)
	{
		return "Error";
	}
	else if(module == TRACE_LEVEL_WARNING)
	{
		return "Warning";
	}
	else if(module == TRACE_LEVEL_INFO)
	{
		return "Info";
	}
	else if(module == TRACE_LEVEL_DEBUG)
	{
		return "Debug";
	}
	else
		return "Other";
};
/* Exported functions --------------------------------------------------------*/

/*******************************************************************************
**
** Function         trace_general
**
** Description      general trace function
**
** Returns          None
**
*******************************************************************************/
void trace_general(const char *format, ...)
{
  
	va_list varg;
	va_start(varg, format);
	vsnprintf(logger_buffer, LOG_BUFFER_SIZE, format, varg);
	port_hardware_uartTransmit((uint8_t *)logger_buffer, strlen(logger_buffer), UART_TIMEOUT);
	va_end(varg);
}

/*******************************************************************************
**
** Function         trace_data_buffer
**
** Description      Print packet
**
** Returns          None
**
*******************************************************************************/
void trace_data_buffer(const char *format, const uint8_t *p_data, uint16_t len)
{
	{
		uint32_t i;
		char print_buffer[len * 3 + 1];
		memset(print_buffer, 0, sizeof(print_buffer));
		for (i = 0; i < len; i++)
		{
			snprintf(print_buffer + i * 3, 4, "%02X ", p_data[i]);
		}
		printf(format, print_buffer);
	}
}
