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

/**@file  port_spi_ese.h
* @brief  port_spi_ese interface declearation 
* @author  zhengwd
* @date  2021-4-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef _PORT_SPI_ESE_H_
#define _PORT_SPI_ESE_H_

/***************************************************************************
* Include Header Files
***************************************************************************/
#include <stdint.h>
#include "types.h"
#include "config.h"
#include "peripheral.h"
#include "ial_spi_ese.h"
#include "daric_hal.h"
#include "daric_hal_spim.h"
#include "daric_hal_gpio.h"
#include "tx_api.h"
#include "trace.h"


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


/** @addtogroup PORT_SPI_ESE
  * @{
  */


/* Private constants --------------------------------------------------------*/
/** @defgroup PORT_SPI_TIME_PARAMS    SPI Communication Time Params
  * @{
  */

#define PORT_SPI_ESE_HAL_TIMEOUT                100     /*!< STM32L433 HAL������ݷ��ͽ��ճ�ʱʱ��: 100ms */   
#define PORT_SPI_ESE_SE_RST_LOW_DELAY           1000    /*!< RST��λʱ�͵�ƽ����ʱ��T7:  1000us */  
#define PORT_SPI_ESE_SE_RST_HIGH_DELAY          10000   /*!< RST��λ��ߵ�ƽ����ʱ��T6:  10000us */  

/**
  * @}
  */


/** @defgroup PORT_SPI_ESE_NSS_MODE    SPI Communication NSS MODE
  * @{
  */

#define PORT_SPI_ESE_NSS_SOFT                   0x00    /*!< ��ģʽ */  
#define PORT_SPI_ESE_NSS_HARD_OUTPUT            0x01    /*!< Ӳ���ģʽ */  

#define PORT_SPI_ESE_NSS_MODE                   PORT_SPI_ESE_NSS_SOFT

/**
  * @}
  */
 

/********************����GPIO ����*******************/
//SE0 RST ����IO
// #define PORT_SPI_ESE_SE0_RST_IO_PORT            GPIOA
// #define PORT_SPI_ESE_SE0_RST_IO_PIN             GPIO_PIN_3
// #define PORT_SPI_ESE_SE0_RST_IO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()

// #define PORT_SPI_ESE_SE0_RST_LOW()              HAL_GPIO_WritePin(PORT_SPI_ESE_SE0_RST_IO_PORT, PORT_SPI_ESE_SE0_RST_IO_PIN, GPIO_PIN_RESET) 
// #define PORT_SPI_ESE_SE0_RST_HIGH()             HAL_GPIO_WritePin(PORT_SPI_ESE_SE0_RST_IO_PORT, PORT_SPI_ESE_SE0_RST_IO_PIN, GPIO_PIN_SET)


// /********************SPI �ӿ�IO ����*******************/
 #define SPI_ESE_INSTANCE                        SPIM1
// #define SPI_ESE_CLK_ENABLE()                    __HAL_RCC_SPI1_CLK_ENABLE()
// #define SPI_ESE_CS_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOA_CLK_ENABLE()
// #define SPI_ESE_SCK_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
// #define SPI_ESE_MISO_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOA_CLK_ENABLE()
// #define SPI_ESE_MOSI_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOA_CLK_ENABLE()

// #define SPI_ESE_FORCE_RESET()                   __HAL_RCC_SPI1_FORCE_RESET()
// #define SPI_ESE_RELEASE_RESET()                 __HAL_RCC_SPI1_RELEASE_RESET()

// /* Definition for SPIx Pins */
// #define SPI_ESE_CS_PIN                          GPIO_PIN_4
// #define SPI_ESE_CS_GPIO_PORT                    GPIOA
// #define SPI_ESE_CS_AF                           GPIO_AF5_SPI1

// #define SPI_ESE_SCK_PIN                         GPIO_PIN_5
// #define SPI_ESE_SCK_GPIO_PORT                   GPIOA
// #define SPI_ESE_SCK_AF                          GPIO_AF5_SPI1

// #define SPI_ESE_MISO_PIN                        GPIO_PIN_6
// #define SPI_ESE_MISO_GPIO_PORT                  GPIOA
// #define SPI_ESE_MISO_AF                         GPIO_AF5_SPI1

// #define SPI_ESE_MOSI_PIN                        GPIO_PIN_7
// #define SPI_ESE_MOSI_GPIO_PORT                  GPIOA
// #define SPI_ESE_MOSI_AF                         GPIO_AF5_SPI1

// #define PORT_SPI_ESE_SE0_CS_ON()                HAL_GPIO_WritePin(SPI_ESE_CS_GPIO_PORT, SPI_ESE_CS_PIN, GPIO_PIN_RESET)
// #define PORT_SPI_ESE_SE0_CS_OFF()               HAL_GPIO_WritePin(SPI_ESE_CS_GPIO_PORT, SPI_ESE_CS_PIN, GPIO_PIN_SET)

/**
  * @}
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup Port_Spi_Ese_Exported_Types Port_Spi_Ese Exported Types
  * @{
  */

/**
  * @brief  Port SPI Ese Control Structure definition
  */
enum  PORT_SPI_ESE_CTRL
{
	PORT_SPI_ESE_CTRL_RST   = 0x00000010,   /*!< RST ��λ����*/
	PORT_SPI_ESE_CTRL_OTHER = 0x00000011    /*!< ��������*/
} ;

/**
  * @brief  Port SPI ESE Communication Params Structure definition
  */
typedef struct   _spi_ese_comm_param_t{
	SPIM_HandleTypeDef *spi_ese_handle;  /*!< SPIͨ�Žӿھ�� */  
	uint8_t slave_id;  /*!< ���豸IDֵ */  
	uint8_t nss_mode; /*!< NSSģʽ */     
} spi_ese_comm_param_t, *spi_ese_comm_param_pointer;

/**
  * @}
  */



/**************************************************************************
* Global Variable Declaration
***************************************************************************/
SPI_ESE_PERIPHERAL_DECLARE(ESE_PERIPHERAL_SE0);


/* Exported functions --------------------------------------------------------*/
/** @defgroup Port_Spi_Ese_Exported_Functions Port_Spi_Ese Exported Functions
  * @{
  */
extern se_error_t port_spi_ese_periph_init (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph) ;
extern se_error_t port_spi_ese_periph_deinit (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph) ;
extern se_error_t port_spi_ese_periph_chip_select (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, bool enable) ;
extern se_error_t port_spi_ese_periph_transmit(HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf, uint32_t  inbuf_len) ;
extern se_error_t port_spi_ese_periph_receive(HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf, uint32_t *outbuf_len) ;
extern se_error_t port_spi_ese_periph_control(HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *inbuf, uint32_t  *inbuf_len) ;
extern se_error_t port_spi_ese_periph_delay(uint32_t us);
extern se_error_t port_spi_ese_periph_timer_start(util_timer_t *timer_start);
extern se_error_t port_spi_ese_periph_timer_differ(util_timer_t *timer_differ);


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


#endif /*_PORT_SPI_ESE_H*/
