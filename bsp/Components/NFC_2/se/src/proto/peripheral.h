/**@file  peripheral.h
* @brief  abstraction layer interface declearation    
* @author  zhengwd
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _PERIPHERAL_H_
#define _PERIPHERAL_H_

/**************************************************************************
* Global Macro Definition
***************************************************************************/

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PROTO 
  * @brief link protocol layer.
  * @{
  */


/** @addtogroup PERIPHERAL 
  * @{
  */



/* peripheral define --------------------------------------------------------*/
/** @defgroup peripheral_bus_driver_declearation peripheral bus driver declearation
  * @{
  */


#define MAX_PERIPHERAL_DEVICE                   4     /*!< 每个通信接口最多支持的设备数*/  
#define MAX_PERIPHERAL_BUS_DRIVER         4     /*!< 最多支持的通信接口数*/  
//#define SPI_SUPPORT_GPIO_IRQ_RECEIVE                /*!< SPI接口下协议层支持GPIO中断信号接收SE返回数据*/
//#define I2C_SUPPORT_GPIO_IRQ_RECEIVE                /*!< I2C接口下协议层支持GPIO中断信号接收SE返回数据*/


/**
  * @brief  Peripheral Type Structure definition
  */
typedef enum _peripheral_type {	
	PERIPHERAL_SPI,   ///< SPI通信接口的外设
    PERIPHERAL_ESE, ///< eSE 通信接口的外设 
	PERIPHERAL_I2C  ///< I2C通信接口的外设
}peripheral_type;


/**
  * @brief  Peripheral Structure definition
  */
typedef struct _peripheral {
	peripheral_type type; ///< 外设的通信接口类型
	uint8_t id;	 ///< 外设标识
}peripheral;

/** 定义外设结构类型名 */
#define PERIPHERAL_STRUCT_DEFINE(periph_type) struct peripheral_##periph_type  

/** 定义外设实例变量名 */
#define PERIPHERAL_NAME(periph_type, id) g_peripheral_device_##periph_type##id

/** 定义指向外设实例的指针*/
#define PERIPHERAL_POINTER(periph_type, id) p_peripheral_device_##periph_type##id


/** 定义外设开始 */
#define PERIPHERAL_DEFINE_BEGIN(periph_type, id) PERIPHERAL_STRUCT_DEFINE(periph_type) PERIPHERAL_NAME(periph_type, id) = {	\
	{periph_type, id},

/** 定义外设结束 */
#define PERIPHERAL_DEFINE_END()  };

/** 注册外设*/
//#define PERIPHERAL_REGISTER(periph_type, id, periph) const PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_NAME(periph_type, id)_ = &periph
#define PERIPHERAL_REGISTER(periph_type, id, periph) PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_POINTER(periph_type, id)= &periph


/** 获取外设*/
#define PERIPHERAL_GET(periph_type, id) PERIPHERAL_POINTER(periph_type, id)

/** 声明外设 */
//#define PERIPHERAL_DECLARE(periph_type, id) extern const PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_NAME(periph_type, id)_
#define PERIPHERAL_DECLARE(periph_type, id) extern PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_POINTER(periph_type, id)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*总线类型 */


/**
  * @brief 外设总线驱动抽象接口
  */
typedef struct _peripheral_bus_driver {
	peripheral_type type;  ///< 外设类型
	peripheral    *periph[MAX_PERIPHERAL_DEVICE];  ///< 总线外设
	se_error_t (*init)		 (peripheral *periph);	  ///< 初始化 
	se_error_t (*deinit)	 (peripheral *periph);	  ///< 终止化
	se_error_t (*open)      (peripheral *periph , uint8_t *rbuf, uint32_t *rlen);  ///< 打开
	se_error_t (*close)     (peripheral *periph);	/* 关闭*/
	se_error_t (*transceive)(peripheral *periph ,  uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);  ///< 数据收发
	se_error_t (*reset)      (peripheral *periph , uint8_t *rbuf, uint32_t *rlen);  ///< 复位
	se_error_t (*control)   (peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);	  ///< 控制命令
	se_error_t (*delay)   (peripheral *periph , uint32_t us);	  ///< 延时指定的微秒
    se_error_t (*ratr)      (peripheral *periph , uint8_t *rbuf, uint32_t *rlen);  ///< ratr
	se_error_t (*endapdu)	(peripheral *periph , uint8_t *rbuf, uint32_t *rlen);  ///< endapdu
	void *extra;  ///< 预留给用户使用
}peripheral_bus_driver;

/** 注册总线驱动 */
//#define PERIPHERAL_BUS_DRIVER_REGISTER(periph_type, driver) const  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type = &driver
#define PERIPHERAL_BUS_DRIVER_REGISTER(periph_type, driver) peripheral_bus_driver *g_peripheral_bus_driver_##periph_type = &driver


/** 获取总线外设驱动 */
#define PERIPHERAL_BUS_DRIVER_GET(periph_type) g_peripheral_bus_driver_##periph_type

/** 声明总线外设驱动*/
//#define PERIPHERAL_BUS_DRIVER_DECLARE(periph_type) extern const  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type
#define PERIPHERAL_BUS_DRIVER_DECLARE(periph_type) extern  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


#endif // _PERIPHERAL_DRIVER_H_

