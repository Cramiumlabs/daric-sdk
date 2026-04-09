/**@file  hal_spi.h
* @brief  port hal spi interface declearation    
* @author  zhengwd
* @date  2021-04-24
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef _IAL_SPI_ESE_H_
#define _IAL_SPI_ESE_H_

#include "types.h"
#include "peripheral.h"
#include "util.h"
#include "se.h"

/**************************************************************************
* Global Macro Definition
***************************************************************************/

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PORT 
  * @brief hardware  portable layer .
  * @{
  */


/** @addtogroup HAL_SPI 
  * @{
  */



/* peripheral define --------------------------------------------------------*/
/** @defgroup peripheral_spi_comm_declearation peripheral spi communication declearation
  * @{
  */


#define HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER PERIPHERAL_STRUCT_DEFINE(PERIPHERAL_ESE)*


//brief ����ͨ����������ӿ�

PERIPHERAL_STRUCT_DEFINE(PERIPHERAL_ESE) {
    peripheral periph;  // ����ӿ�
    se_error_t (*init)       (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph);  //  ��ʼ��
    se_error_t (*deinit)	  (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph);  //  ��ֹ��
    se_error_t (*delay)(uint32_t us);  //΢���ʱ
    se_error_t (*timer_start)(util_timer_t *timerval);  //��ȡ��ʱ����ʼʱ��
    se_error_t (*timer_differ)(util_timer_t *timerval);  //�Ƚ�ʱ���ֵ������Ƿ�ʱ
    se_error_t (*chip_select)(HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, _Bool enable);  //Ƭѡ
    se_error_t (*transmit)   (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint8_t *data, uint32_t  data_len);   //�������� 
    se_error_t (*receive)    (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint8_t *data, uint32_t *data_len);   // �������� 
    se_error_t (*control)    (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *data, uint32_t  *data_len);  //�շ���������
    void *extra;       //����ͻ��Զ���Ĳ��� 
};

/** ����SPI ese ����ʵ�������� */

#define SPI_ESE_PERIPHERAL_NAME(id) PERIPHERAL_NAME(PERIPHERAL_ESE, id)

/** ����SPI ese ���迪ʼ */

#define SPI_ESE_PERIPHERAL_DEFINE_BEGIN(id) PERIPHERAL_DEFINE_BEGIN(PERIPHERAL_ESE, id)

/** ����SPI ese ������� */
#define SPI_ESE_PERIPHERAL_DEFINE_END() PERIPHERAL_DEFINE_END()

/** ע��SPI ese ����*/
#define SPI_ESE_PERIPHERAL_REGISTER(id) PERIPHERAL_REGISTER(PERIPHERAL_ESE, id, PERIPHERAL_NAME(PERIPHERAL_ESE, id))

/** ����SPI ese ���� */
#define SPI_ESE_PERIPHERAL_DECLARE(id) PERIPHERAL_DECLARE(PERIPHERAL_ESE, id)  

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

#endif
