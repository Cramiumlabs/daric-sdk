/**@file  util.c
* @brief  提供双端队列或其它公用函数
* @author  zhengwd
* @date  2021-5-12
* @version  V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
//#include "port_bcm2711_util.h"
#include "util.h"


/** @addtogroup SE_Driver
  * @{
  */

/** @defgroup UTIL UTIL
  * @brief util module driver.
  * @{
  */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup UTIL_Queue_Exported_Functions UTIL_Queue Exported Functions
  * @{
  */



/**
* @brief 双端队列初始化
* @param [in] d_queue  双端队列实例地址
* @return 双端队列实例地址	
* @note 双端队列初始化后，队列相关参数恢复为默认值
*/
double_queue util_queue_init(double_queue d_queue)
{
	d_queue ->q_buf_len =0;
	d_queue ->capacity=DEQUE_MAX_SIZE;
	d_queue ->front_node=20;
	d_queue ->rear_node =20;
	memset(d_queue->q_buf,0x00,DEQUE_MAX_SIZE);
	return d_queue;
}


/**
* @brief 双端队列头增加数据
* @param [in] in_buf  输入数据地址
* @param [in] in_buf_len  输入数据长度
* @param [in] d_queue 双端队列实例地址
* @return no	
* @note no
* @see 函数 util_queue_front_pop
*/
void util_queue_front_push(uint8_t*in_buf,uint32_t in_buf_len, double_queue d_queue)
{
	uint32_t off = 0;
	
	if(d_queue ->front_node==0)
	{
		d_queue ->front_node = d_queue ->capacity- in_buf_len;
	}
	else
	{
		d_queue ->front_node-= in_buf_len;
	}
	
	off = d_queue ->front_node;
	memcpy(d_queue->q_buf+off, in_buf, in_buf_len); 
	d_queue ->q_buf_len += in_buf_len;
}


/**
* @brief 双端队列头删除数据
* @param [in] in_buf_len 删除数据长度
* @param [in] d_queue 双端队列实例地址
* @return no	
* @note no
* @see 函数 util_queue_front_push
*/
void util_queue_front_pop(uint32_t in_buf_len ,double_queue d_queue)
{
	uint8_t item[DEQUE_MAX_SIZE] = {0x00};
	uint32_t off = d_queue->front_node;  
	
	if(d_queue ->front_node ==(d_queue ->capacity- in_buf_len))
	{
		memcpy(item,d_queue->q_buf,in_buf_len);
		d_queue ->front_node =0;
		
	}
	else 
	{
		memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->front_node += in_buf_len;
	}
	
	d_queue ->q_buf_len -= in_buf_len;
}



/**
* @brief 双端队列尾增加数据
* @param [in] in_buf 输入数据地址
* @param [in] in_buf_len 输入数据长度
* @param [in] d_queue 双端队列实例地址
* @return no	
* @note no
* @see 函数 util_queue_rear_pop
*/
void util_queue_rear_push(uint8_t*in_buf,uint32_t in_buf_len, double_queue d_queue)
{
	uint32_t off = d_queue->rear_node;
	
	memcpy(d_queue->q_buf+off, in_buf, in_buf_len); 
	if(d_queue ->rear_node== d_queue ->capacity- in_buf_len)
	{	
		d_queue ->rear_node = 0;
	}
	else
	{
		d_queue ->rear_node += in_buf_len;
	}

	d_queue ->q_buf_len += in_buf_len;
}


/**
* @brief 双端队列尾删除数据
* @param [in] in_buf_len 删除数据长度
* @param [in] d_queue 双端队列实例地址
* @return no	
* @note no
* @see 函数 util_queue_rear_push
*/
void util_queue_rear_pop(uint32_t in_buf_len ,double_queue d_queue)
{
	uint8_t item[DEQUE_MAX_SIZE] = {0x00};
	uint32_t off = d_queue->rear_node;	
	
	if(d_queue ->rear_node == 0)
	{
		off=d_queue ->capacity-in_buf_len;
		memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->capacity-= in_buf_len;
	}
	else 
	{
		memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->rear_node -= in_buf_len;
	}

	d_queue ->q_buf_len -= in_buf_len;
}

/**
* @brief 获取双端队列中存储数据的长度
* @param [in] d_queue 双端队列实例地址
* @return 双端队列中当前存储数据的长度	
* @note no
*/
uint32_t util_queue_size (double_queue d_queue)
{
	if(d_queue ->rear_node> d_queue ->front_node)
	{
		return (d_queue ->rear_node- d_queue ->front_node);
	}
	else
	{
		return (d_queue ->front_node- d_queue ->rear_node);
	}
}



/**
  * @}
  */



/**
  * @}
  */

/**
  * @}
  */




