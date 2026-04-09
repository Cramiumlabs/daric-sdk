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
  

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PORT_HARDWARE_H__
#define __PORT_HARDWARE_H__
#include "nfc_type.h"
#include "daric_hal_gpio.h"
#include "daric_hal_i2c.h"
#include "daric_hal_uart.h"
#include "daric_hal.h"


    
//#define I2C_DEBUG    1

/**
* IO define about NFCC
*/    
#define PORT_PIN_IRQ 0
#define PIN_IRQ 04
#define PORT_PIN_VEN 0
#define PIN_VEN 28
#define PORT_PIN_DNLD 0
#define PIN_DNLD 29//10
#define PORT_PIN_4V2 0
#define PIN_4V2 		31

// #define PIN_4V2_CTL NRF_GPIO_PIN_MAP(0, 31)
// #define PIN_1V8_CTL NRF_GPIO_PIN_MAP(1, 8)
// #define PIN_PMUVCC1_CTL NRF_GPIO_PIN_MAP(1, 9)

// #define I2C_NPC_SLAVE_ADDR 0x29

// //#define PIN_I2C_SCL 27      /* I2C2 */
// //#define PIN_I2C_SDA 26      /* I2C2 */

// #define PIN_I2C_SCL  ARDUINO_6_PIN      /* I2C2 */
// #define PIN_I2C_SDA  ARDUINO_4_PIN      /* I2C2 */

// #define PORT_PIN_I2C 0

// #define I2C_ADDR_7BIT       I2C_NPC_SLAVE_ADDR
// #define LPC_I2C_PORT         LPC_I2C2
// #define LPC_I2C_INTHAND      I2C2_IRQHandler
// #define LPC_IRQNUM           I2C2_IRQn
// #define LPC_I2CM_CLOCK       SYSCON_CLOCK_I2C2
// #define LPC_I2CM_RESET       RESET_I2C2
// #else                         
//                              //可穿戴IO
// //#define PORT_PIN_IRQ         0
// #define PIN_IRQ              ARDUINO_AREF_PIN
// //#define PORT_PIN_VEN         0
// #define PIN_VEN              ARDUINO_1_PIN
// //#define PORT_PIN_DNLD        0
// #ifdef PN80T_CD_ENABLE
// #define PIN_DNLD             ARDUINO_0_PIN /* Download Pin for PN80T CD */
// #else
// #define PIN_DNLD             22
// #endif



/*
 * P0_23 and P0_24 lines of I2C0
 * used to talk to pn66t over I2C
 */
// #define PIN_I2C_SCL            ARDUINO_SCL_PIN        
// #define PIN_I2C_SDA            ARDUINO_SDA_PIN

// #define PORT_PIN_I2C 0

// #define I2C_ADDR_7BIT        I2C_NPC_SLAVE_ADDR
// #define LPC_I2C_PORT         LPC_I2C0
// #define LPC_I2C_INTHAND      I2C0_IRQHandler
// #define LPC_IRQNUM           I2C0_IRQn
// #define LPC_I2CM_CLOCK       SYSCON_CLOCK_I2C0
// #define LPC_I2CM_RESET       RESET_I2C0
// #endif



/**
*gpio control GPIO define
*/   

/*	NFCC IO Control	*/
#define PIN_VEN_NUM 				0
#define PIN_DWL_REQ_NUM			1
#define PIN_LEVEL_LOW				0
#define PIN_LEVEL_HIGH			1

#define NFCC_VEN_Pin 			PIN_VEN
#define NFCC_DWL_REQ_Pin 	PIN_DNLD

    
/**
*Uart GPIO define
*/    
#define LOG_UART				USART2
#define LOG_UART_TX_PORT		GPIOD
#define LOG_UART_TX_PIN			GPIO_PIN_5
#define LOG_UART_RX_PORT		GPIOD
#define LOG_UART_RX_PIN			GPIO_PIN_6


/**
*I2C GPIO define
*/ 
// #define I2CIRQ_GPO_PIN                                            PIN_IRQ

// #define I2CIRQ_GPO_CLK_ENABLE()                                   __HAL_RCC_GPIOE_CLK_ENABLE()
// #define I2CIRQ_GPO_CLK_DISABLE()                                  __HAL_RCC_GPIOE_CLK_DISABLE()
// #define I2CIRQ_GPO_EXTI                                           PIN_IRQ
// #define I2CIRQ_GPO_EXTIHandler                                    EXTI15_10_IRQHandler


// //SPI PINs
// #define SPI_SS_PIN                ARDUINO_3_PIN
// #define SPI_MISO_PIN              ARDUINO_4_PIN  //26//
// #define SPI_MOSI_PIN              ARDUINO_5_PIN
// #define SPI_SCK_PIN               ARDUINO_6_PIN  //27//
// /**
// *SPI GPIO define
// */
// #define ST25R_SS_PIN                            GPIO_PIN_4
// #define ST25R_SS_PORT          	                GPIOA

/**
*IRQ define
*/ 
#define            PORT_IRQn_Type           IRQn_Type

/**
*GPIO define
*/ 
#define PORT_GPIO_TypeDef GPIO_TypeDef



#define SET_OUT               1
#define SET_IN                0

#define HIGH              	  1
#define LOW               	  0


typedef enum
{
  PORT_PIN_RESET = 0,
  PORT_PIN_SET
}PORT_PinState;


typedef enum
{
  PORT_COM_OK       = 0x00,
  PORT_COM_ERROR    = 0x01,
  PORT_COM_BUSY     = 0x02,
  PORT_COM_TIMEOUT  = 0x03
} PORT_Com_status;


#define SRI_CONFIG_OK 0
#define SRI_CONFIG_FAILED 1

#define PAYLOAD_HEAD_SIZE   3


// #define PORT_APP_ERROR_CHECK(ERR_CODE)	APP_ERROR_CHECK(ERR_CODE) 
// #define PORT_nrf_drv_gpiote_in_init		nrf_drv_gpiote_in_init
// #define PORT_GPIOTE_CONFIG_IN_SENSE_LOTOHI	GPIOTE_CONFIG_IN_SENSE_LOTOHI
// #define PORT_nrf_drv_gpiote_init	nrf_drv_gpiote_init 


// typedef nrfx_gpiote_in_config_t	PORT_nrfx_gpiote_in_config_t;




/*   
*Interrupt functions 
*
*/
//enable the commnication IRQ interrupt
extern void port_hardware_irqComIrqEnable (void);
//disable the commnication IRQ interrupt
extern void port_hardware_irqComIrqDisable(void);

//Clear the communication(SPI/I2C) irq flag former
extern void port_hardware_irqComIrqClear(void);

/*   
*I2C functions
*
*/

//process i2c initialize
extern PORT_Com_status port_hardware_i2cInit(uint8_t rate_level);
//Master transmit data to Slave
extern PORT_Com_status port_hardware_i2cMasterTransmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//Master receive data from Slave
extern PORT_Com_status port_hardware_i2cMasterReceive(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);


/*   
*GPIO functions
*
*/

//check if the value of communication IRQ is high
extern bool port_hardware_irqIsComIrqHigh(void);
// IOControl functions
extern void port_hardware_gpioVenPullup(void);
extern void port_hardware_gpioVenPulldown(void);
extern void port_hardware_gpioDwlPullup(void);
extern void port_hardware_gpioDwlPulldown(void);
// extern void port_nrf_gpio_pin_write(uint32_t pin_number, uint32_t value);
// extern uint32_t port_nrf_gpio_pin_read(uint32_t pin_number);
// extern void port_nrf_gpio_cfg_output(uint32_t pin_number);
// extern nrfx_err_t port_nrfx_gpiote_init(void);
// extern void port_gpioInit(void);

/**
*Uart functions
*
*/
extern PORT_Com_status port_hardware_uartInit(void);
extern PORT_Com_status port_hardware_uartTransmit(uint8_t *pData, uint16_t Size, uint32_t Timeout);
 

//timer functions
extern void port_hardware_delayUs(uint32_t time_us);

uint8_t port_hardware_flashWriteU8(uint32_t dest_addr, uint8_t value);
uint8_t port_hardware_flashReadU8(uint32_t dest_addr);

extern uint8_t port_get_irq_flag(uint32_t time_out);

typedef struct {
    uint32_t msg_type;
    uint32_t data_len;
    uint8_t *out_data;
} NFC_API_MESSAGE;

#endif /* __PORT_HARDWARE_H__ */
