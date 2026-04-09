/**@file  log.h
* @brief  Header file of log
* @author  zhengwd
* @date  2021-5-12
* @version  V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/
#ifndef _LOG_H_
#define _LOG_H_


#include <stdio.h>
//#include "port_util.h"


#define port_printf_buf(info, buf, buflen)               \
					printf("\n");                   \
					printf("%s", info);             \
					for(uint16_t i=0; i<buflen; i++)     \
                        printf("%3.2x",  buf[i]);  
#define LOG_PRINTF(...) printf(__VA_ARGS__)

#define LOG_TRACE(format, level, file, line, func, ...) printf("\n%s - %s(%d) - %s: "format, level, file, line, func, ##__VA_ARGS__)



//#define port_printf_buf(name, buf, buflen)   

// #define LOG_PRINTF(...) 
// #define LOG_TRACE(level, format, ...)



#define LOGV(format, ...) LOG_TRACE(format, "V", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define LOGD(format, ...) LOG_TRACE(format, "D", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define LOGI(format, ...) LOG_TRACE(format, "I", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define LOGW(format, ...) LOG_TRACE(format, "W", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOGE(format, ...) LOG_TRACE(format, "E", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define LOGF(format, ...) LOG_TRACE(format, "F", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifndef  LOGV
#define  LOGV(format, ...) 
#endif

#ifndef  LOGD
#define  LOGD(format, ...) 
#endif

#ifndef  LOGI
#define  LOGI(format, ...) 
#endif

#ifndef  LOGW
#define  LOGW(format, ...)  
#endif

#ifndef  LOGE
#define  LOGE(format, ...) 
#endif

#ifndef  LOGF
#define  LOGF(format, ...)  
#endif  
#endif


