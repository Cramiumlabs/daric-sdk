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
  

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRACE_H__
#define __TRACE_H__
#include <stdint.h>

extern uint8_t trace_level;
/* Exported functions ------------------------------------------------------- */
void trace_general(const char *format, ...);
void trace_data_buffer(const char *format, const uint8_t* p_data, uint16_t len);

/******************************************************************************
  **
  ** Trace configurable parameters
  **
  ******************************************************************************/

/*****************************************************************************
  ** Define trace levels
  *****************************************************************************/
/* No trace messages to be generated    */
#define TRACE_LEVEL_NONE  0
/* Fatal error condition trace messages */
#define TRACE_LEVEL_FATAL 1
/* Error condition trace messages       */
#define TRACE_LEVEL_ERROR 2
/* Warning condition trace messages     */
#define TRACE_LEVEL_WARNING  3
/* Information trace messages           */
#define TRACE_LEVEL_INFO  4
/* Full debug messages                  */
#define TRACE_LEVEL_DEBUG 5



// static const char *bt_trace_getLevelName(uint8_t module)
// {
// 	if(module == TRACE_LEVEL_FATAL)
// 	{
// 		return "Fatal";
// 	}
// 	else if(module == TRACE_LEVEL_ERROR)
// 	{
// 		return "Error";
// 	}
// 	else if(module == TRACE_LEVEL_WARNING)
// 	{
// 		return "Warning";
// 	}
// 	else if(module == TRACE_LEVEL_INFO)
// 	{
// 		return "Info";
// 	}
// 	else if(module == TRACE_LEVEL_DEBUG)
// 	{
// 		return "Debug";
// 	}
// 	else
// 		return "Other";
// };
char *bt_trace_getLevelName(uint8_t module);
//#define BT_TRACE(t,m,...)                        trace_general("%s : "m"\r\n",bt_trace_getLevelName(t),##__VA_ARGS__)
#define BT_TRACE(t,m,...)                        \
	do {\
		if (t <= TRACE_LEVEL_WARNING) {\
			printf("%s func:%s[line:%d] : "m"\r\n",bt_trace_getLevelName(t),__FUNCTION__,__LINE__,##__VA_ARGS__); \
		}else {\
			printf("%s : "m"\r\n",bt_trace_getLevelName(t),##__VA_ARGS__); \
		}\
	}while(0)


/* Define tracing for the NFC unit
  */
#define NFC_TRACE_FATAL(m,...)                     {if (trace_level >= TRACE_LEVEL_FATAL) BT_TRACE(TRACE_LEVEL_FATAL, m,##__VA_ARGS__);}
#define NFC_TRACE_ERROR(m,...)                     {if (trace_level >= TRACE_LEVEL_ERROR) BT_TRACE(TRACE_LEVEL_ERROR, m,##__VA_ARGS__);}
#define NFC_TRACE_WARNING(m,...)                   {if (trace_level >= TRACE_LEVEL_WARNING) BT_TRACE(TRACE_LEVEL_WARNING, m,##__VA_ARGS__);}
#define NFC_TRACE_INFO(m,...)                      {if (trace_level >= TRACE_LEVEL_INFO) BT_TRACE(TRACE_LEVEL_INFO, m,##__VA_ARGS__);}
#define NFC_TRACE_DEBUG(m,...)                     {if (trace_level >= TRACE_LEVEL_DEBUG) BT_TRACE(TRACE_LEVEL_DEBUG, m,##__VA_ARGS__);}

#define NFC_TRACE_BUFFER_INFO(str,data,len)        {if (trace_level >= TRACE_LEVEL_INFO) trace_data_buffer("Info : "str" %s \r\n",data,len);}
#define NFC_TRACE_BUFFER_DEBUG(str,data,len)       {if (trace_level >= TRACE_LEVEL_DEBUG) trace_data_buffer("Debug : "str" %s \r\n",data,len);}



#endif /* __PORT_TRACE_H__ */
