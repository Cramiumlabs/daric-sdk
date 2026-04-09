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
#ifndef __hed_i2c_H
#define __hed_i2c_H

/* Includes ------------------------------------------------------------------*/
#include "port_hardware.h"
#include "nfc_config.h"


#define I2C_TIMEOUT 1000

#define I2C_RECEIVE_TIMEOUT 300
#define I2C_BGT 100
#define I2C_RRT 100
#define I2C_RE_RECEIVE_MAX 3
#define I2C_RE_TRANSMIT_MAX 5
#define I2C_SLAVE_ADDR DEFAULT_I2C_SLAVE_ADDR
#define I2C_RATE_LEVEL 1

#define NFCC_BGT_TIME 40

#define CMS_MODE_CRC_ERROR 0x6500
#define NCI_DATA_NOTIFY 0x0A
#define NCI_DATA_CRC_ERROR 0xE0

#define CMS_FRAME_HEADER_LEN 3
#define CMS_FRAME_DATA_MAX_LEN 0x1047
#define CMS_FRAME_MAX_LEN (CMS_FRAME_DATA_MAX_LEN + 7 + CMS_FRAME_HEADER_LEN + 2)
#define MAX_COM_BUFFER_SIZE CMS_FRAME_MAX_LEN

//i2c通信参数，配置区配置
typedef struct I2C_Params
{
	uint32_t I2C_Rec_TimeOut;
	uint32_t I2C_BGT_Time;
	uint32_t I2C_RRT_Time;

	uint8_t I2C_ReRx_Max;
	uint8_t I2C_ReTx_Max;

	uint8_t I2C_Slave_Addr;
	uint8_t I2C_Rate_level;
} I2C_Params_t;

//commnication mode
typedef enum ComMode
{
	Mode_CMS = 0,
	Mode_NCI,
} ComMode_t;

//save the data transmited
typedef struct LastTxData
{
	uint8_t TxBuffer[MAX_COM_BUFFER_SIZE];
	uint16_t TxDataLen;
} LastTxData_t;

typedef enum driver_Statuss
{
	DRV_STATUS_SUCCESS = 0, /*!< DRV Status SUCCESS    */
	DRV_STATUS_FAILED  = 1, /*!< DRV Status FAILED     */
	DRV_STATUS_NOT_INITIALED,
	DRV_STATUS_CRC_ERROR,
	DRV_STATUS_BUSY,
	DRV_STATUS_NO_DATA,
	DRV_STATUS_TX_OVERFLOW,
	DRV_STATUS_RX_OVERFLOW,
	DRV_STATUS_RETX_ERROR,
} DRV_Status; /*!< DRV Status type       */

/*! 
 *****************************************************************************
 * \brief  Initalize I2C
 * 
 * \param[in]  hi2c : pointer to initalized I2C block
 *
 *****************************************************************************
 */
DRV_Status I2CInit(void);

/*! 
 *****************************************************************************
 * \brief  I2C Sequential Transmit
 *  
 * This method transmits the given txBuf over I2C with support for sequencial
 * transmits, and repeat start condition
 * 
 * \param[in]  address : device address 
 * \param[in]  txBuf   : buffer to be transmitted
 * \param[in]  txLen   : size of txBuffer
 * \param[in]  last    : true if last data to be transmitted
 * \param[in]  txOnly  : true if no reception is to be performed after (STOP)
 *                       false if a reception should happen afterwards with 
 *                       repeated START
 *
 * \return HAL Error code 
 *****************************************************************************
 */
DRV_Status i2cSequentialTx(uint8_t address, const uint8_t *txBuf, uint16_t txLen, bool last, bool txOnly);

DRV_Status I2CSequentialTx(const uint8_t *txBuf, uint16_t txLen);

/*! 
 *****************************************************************************
 * \brief  I2C Sequential Receive
 *  
 * This method receives data over I2C. To be used after i2cSequentialTx()
 * 
 * \param[in]  address : device address 
 * \param[in]  rxBuf   : buffer to be place received data
 * \param[in]  txLen   : size of rxBuffer
 *
 * \return HAL Error code 
 *****************************************************************************
 */
DRV_Status i2cSequentialRx(uint8_t address, uint8_t *rxBuf, uint16_t *rxLen);
DRV_Status I2CSequentialRx(uint8_t *rxBuf, uint16_t *rxLen);

/*! 
 *****************************************************************************
 * \brief  I2C Transceive
 *  
 * This method transmits and receives with a repeated START Condition in between.
 * 
 * \param[in]  address : device address 
 * \param[in]  txBuf   : buffer to be transmitted
 * \param[in]  txLen   : size of txBuffer
 * \param[in]  rxBuf   : buffer to be place received data
 * \param[in]  txLen   : size of rxBuffer: if 0 no reception is performed
 *
 * \return HAL Error code 
 *****************************************************************************
 */
DRV_Status i2cSequentialTxRx(uint8_t address, const uint8_t *txBuf, uint16_t txLen, uint8_t *rxBuf, uint16_t rxLen);

extern DRV_Status I2CPramsSet(I2C_Params_t *params);
extern DRV_Status I2CModeSet(ComMode_t comMode);
extern DRV_Status i2c_drv_Init(void);
extern void I2C_Nack_cb(I2C_HandleTypeDef *hi2c);
#endif /*__i2c_H */
