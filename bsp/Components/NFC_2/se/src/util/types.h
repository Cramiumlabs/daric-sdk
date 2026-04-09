/**@file  types.h
* @brief  Header file of uitl types
* @author  ade
* @date  2021-5-24
* @version  V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

#ifdef _ASSERT_DEBUG
#include <assert.h>
#define  ASSERT(int) assert(int)
#endif

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup UTIL
  * @{
  */

/** @defgroup UTIL_Exported_Types UTIL Exported Types
  * @{
  */
#ifndef __cplusplus  
typedef char    bool;
#endif

typedef  int32_t se_error_t;

#define  ASSERT(int) 

#ifndef NULL
#define NULL  0
#endif

#ifndef FALSE
#define FALSE 0
#define false 0

#endif

#ifndef TRUE
#define TRUE  1
#define true  1
#endif

/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */


#endif /* _TYPES_H_ */
