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

/*! \file
 *
 *  \author 
 *
 *  \brief SPI communication handling implementation.
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "port.h"
#include "nfc_i2c.h"

#define NCI_MT_MASK 0xE0
#define NCI_MT_SHIFT 5
#define NCI_MT_NTF 3
#define NCI_GID_MASK 0x0F
#define NCI_GID_CORE 0x00
#define NCI_OID_MASK 0x3F

//I2C commnication params
static I2C_Params_t I2C_ComParams;
//current i2c commnication mode
ComMode_t I2C_comMode = Mode_NCI;
//i2c reTransmit count
uint8_t I2C_reTxCount;
//i2c reRecieve count
uint8_t I2C_reRxCount;

uint16_t g_i2c_need_retx_len = 0;
volatile uint8_t retryflag = 0;
LastTxData_t NackComLastTxData;
bool isTX = false;
//I2C and SPI use the same send struct
LastTxData_t ComLastTxData;
#define CRC_A 0
#define CRC_B 1
/*************************************************
  Function:	  proto_i2c_crc16
  Description:  cauculate the protocol crc
  Input:	
            CRCType: crc cauculate type
            Length: input data len
            Data: input data addr
  Return:	crc: crc result
  Others:		
*************************************************/
uint16_t crc16(uint32_t CRCType, uint8_t *Data, uint32_t Length)
{
	uint8_t chBlock = 0;
	uint16_t wCrc	= 0;
	uint16_t Crc	= 0;

	wCrc = (CRCType == CRC_A) ? 0x6363 : 0xFFFF; // CRC_A : ITU-V.41 , CRC_B : ISO 3309

	do
	{
		chBlock = *Data++;
		chBlock = (chBlock ^ (uint8_t)(wCrc & 0x00FF));
		chBlock = (chBlock ^ (chBlock << 4));
		wCrc	= (wCrc >> 8) ^ ((uint16_t)chBlock << 8) ^ ((uint16_t)chBlock << 3) ^ ((uint16_t)chBlock >> 4);
	} while (--Length);

	if (CRCType != CRC_A)
	{
		wCrc = ~wCrc; // ISO 3309
	}

	Crc = ((uint8_t)wCrc << 8) + (wCrc >> 8);

	return Crc;
}

uint16_t proto_crc16(uint8_t *Data, uint32_t Length)
{
	return crc16(CRC_B, Data, Length);
}

/********************************************************************************* 
Function:       Firm_Prot_FillCRC
Description:    
Input:          no 
Output:         no  
Return:         no
Others:         no 
*********************************************************************************/
void Firm_Prot_FillCRC(unsigned char *Buf, unsigned int len)
{
	unsigned short t_checkValue;

	t_checkValue = proto_crc16(Buf, len);

	Buf[len]	 = (unsigned char)(t_checkValue >> 8);
	Buf[len + 1] = (unsigned char)t_checkValue;
	return;
}



DRV_Status  I2CPramsSet (I2C_Params_t *params)
{
	memcpy(&I2C_ComParams, params, sizeof(I2C_Params_t));
	return DRV_STATUS_SUCCESS;
}

DRV_Status I2CModeSet(ComMode_t comMode)
{
	if (!(comMode == Mode_CMS || comMode == Mode_NCI))
		return DRV_STATUS_FAILED;
	I2C_comMode = comMode;
	return DRV_STATUS_SUCCESS;
}

/**
* @brief I2C initialize
* @return excute result	
* @note no
*/
DRV_Status I2CInit(void)
{
	// I2C1_init();I2C_HandleTypeDef i2cHandler = I2C1_init();
	if (port_hardware_i2cInit(I2C_ComParams.I2C_Rate_level) != PORT_COM_OK) //&i2cHandler;
		return DRV_STATUS_FAILED;											//the default mode of I2C is NCI mode
	I2C_comMode = Mode_NCI;
	//there is no data to transmit when initialized successfully
	ComLastTxData.TxDataLen = 0;
	return DRV_STATUS_SUCCESS;
}

//set parametters from local config based the communication type
DRV_Status i2c_pramsSet(void)
{
	I2C_Params_t i2cParams;
	i2cParams.I2C_BGT_Time	  = I2C_BGT;
	i2cParams.I2C_Rec_TimeOut = I2C_RECEIVE_TIMEOUT;
	i2cParams.I2C_RRT_Time	  = I2C_RRT;
	i2cParams.I2C_ReRx_Max	  = I2C_RE_RECEIVE_MAX;
	i2cParams.I2C_ReTx_Max	  = I2C_RE_TRANSMIT_MAX;
	i2cParams.I2C_Slave_Addr  = I2C_SLAVE_ADDR;
	I2CPramsSet(&i2cParams);
	return DRV_STATUS_SUCCESS;
}
/**
  * @brief Open communication link and initialize the NFC device
  * @param [in] SetReadEvent pointer to callback function to register read request (message from NFC controller)
  * @retval PORT_Status_t PORT_STATUS_SUCCESS
  */
DRV_Status i2c_drv_Init(void)
{
	if (I2CInit() != DRV_STATUS_SUCCESS)
		return DRV_STATUS_FAILED;
	return i2c_pramsSet();
}

//call the port interface to transmit data to the slave, now the data to be transmit is copied in the struct ComLastTxData
DRV_Status i2cTransmit(uint8_t address, bool last, bool txOnly)
{
	DRV_Status status = DRV_STATUS_SUCCESS;
	//wait for the BGT_time //port_timer_waitmsticks(I2C_ComParams.I2C_BGT_Time/1000);
	//the unit of bgt_time is us, now taskDeley is 1 ms, need adjust latter.
	port_hardware_delayUs(I2C_ComParams.I2C_BGT_Time);
	g_i2c_need_retx_len = ComLastTxData.TxDataLen; //记录， 防止IRQ拉高导致未发送

try_transmit:
    if(port_hardware_irqIsComIrqHigh()) //after bgt_time delay, judge the value of irq, if the value is high， means here is data sent from slave， the master can not send data now
        return DRV_STATUS_BUSY;
   // address = address << 1;//the high 7-bits express the slave address

	status = DRV_STATUS_SUCCESS;
	if (port_hardware_i2cMasterTransmit(address, ComLastTxData.TxBuffer, ComLastTxData.TxDataLen, I2C_TIMEOUT) != PORT_COM_OK)
		status = DRV_STATUS_FAILED;

    tx_thread_sleep(1);
    
    if(status != DRV_STATUS_SUCCESS || retryflag == 1)
	{
		port_timer_delay_ms(1); //when send fail(eg. recieve NACK)，maybe slave is in sleep mode, now delay 5ms,try again
		//try send data again if the retry count allowed
		if (I2C_reTxCount >= I2C_ComParams.I2C_ReTx_Max)
		{
			g_i2c_need_retx_len = 0; //清除因IRQ重发
			//if exceed the max retry count, clear the txDatalen and return a failure.
			ComLastTxData.TxDataLen = 0;
			return DRV_STATUS_FAILED;
		}
		else
		{
			I2C_reTxCount++;
            retryflag = 0;
			goto try_transmit;
		}
	}
	else
	{
		g_i2c_need_retx_len = 0; //清除因IRQ重发
	}
	return status;
}


void I2C_Nack_cb(I2C_HandleTypeDef *hi2c) {
    
    // port_timer_waitmsticks(5);//when send fail(eg. recieve NACK)，maybe slave is in sleep mode, now delay 5ms,try again
    //printf("I2C_Nack_cb:isTX = %d\r\n",isTX);
        // //try send data again if the retry count allowed
        // if(I2C_reTxCount >= I2C_ComParams.I2C_ReTx_Max){
        //     //if exceed the max retry count, clear the txDatalen and return a failure.
        //     ComLastTxData.TxDataLen = 0;
        // }else{
        //     I2C_reTxCount++;
        if(isTX != false){
            isTX = false;
            retryflag = 1;
            // hi2c->state = HAL_I2C_STATE_READY;

            // port_hardware_i2cMasterTransmit(I2C_ComParams.I2C_Slave_Addr, NackComLastTxData.TxBuffer, NackComLastTxData.TxDataLen, I2C_TIMEOUT);

            
        }
            //port_hardware_i2cMasterTransmit(I2C_ComParams.I2C_Slave_Addr, NackComLastTxData.TxBuffer, NackComLastTxData.TxDataLen, I2C_TIMEOUT);
        // }
    

}

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
 * \return DRV Error code 
 *****************************************************************************
 */
DRV_Status i2cSequentialTx(uint8_t address, const uint8_t *txBuf, uint16_t txLen, bool last, bool txOnly) // )//, bool last, bool txOnly
{
	if (txBuf == NULL || txLen == 0)
		return DRV_STATUS_FAILED;

	I2C_reTxCount = 0;
	memcpy(ComLastTxData.TxBuffer, txBuf, txLen);
	//Calculate protocol CRC
	Firm_Prot_FillCRC(ComLastTxData.TxBuffer, txLen);
	txLen += 2;
	ComLastTxData.TxDataLen = txLen;
	return i2cTransmit(address, last, txOnly);
}

DRV_Status I2CSequentialTx(const uint8_t *txBuf, uint16_t txLen) // )//, bool last, bool txOnly
{
	if (txLen > MAX_COM_BUFFER_SIZE - 2) //minus the crc len
		return DRV_STATUS_TX_OVERFLOW;
	DRV_Status status = i2cSequentialTx(I2C_ComParams.I2C_Slave_Addr, txBuf, txLen, true, true);
	return status;
}

static DRV_Status i2cReTx()
{
	DRV_Status status = DRV_STATUS_SUCCESS;
	//check if there is data to be sent
	if (ComLastTxData.TxDataLen == 0)
		return DRV_STATUS_FAILED;
	I2C_reTxCount++;
		status = i2cTransmit(I2C_ComParams.I2C_Slave_Addr, true, true);
	return status;
}

static DRV_Status i2cReTx_for_irq()
{
	DRV_Status status = DRV_STATUS_SUCCESS;
	//check if there is data to be sent
	if (g_i2c_need_retx_len == 0)
		return DRV_STATUS_SUCCESS;

	// debug_io(7);
	//debug_io(3);
	status = i2cTransmit(I2C_ComParams.I2C_Slave_Addr, true, true);
	
	return status;
}

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
 * \return DRV Error code 
 *****************************************************************************
 */
#define MAX_RECEIVE_BUFFER_SIZE 258
#define CRC_SIZE 2
DRV_Status i2cSequentialRx(uint8_t address, uint8_t *rxBuf, uint16_t *rxLen)
{
	DRV_Status status = DRV_STATUS_SUCCESS;
	;
	uint16_t crc	 = 0;
	uint8_t tryCount = 0;
	uint8_t myRxBuf[MAX_RECEIVE_BUFFER_SIZE + CRC_SIZE];
	uint16_t maxBufferLen = *rxLen;
	uint16_t payloadDataLen;

	if (!port_hardware_irqIsComIrqHigh()) //call read API only when irq is high, if irq is low,maybe the interrupt is error
		return DRV_STATUS_NO_DATA;

	if (rxBuf == NULL)
		return DRV_STATUS_FAILED;

try_read_head: //read the protocol head(3bytes) in nci mode and cms mode
	status = DRV_STATUS_SUCCESS;
	if (port_hardware_i2cMasterReceive(address, myRxBuf, PAYLOAD_HEAD_SIZE, I2C_TIMEOUT) != PORT_COM_OK)
		status = DRV_STATUS_FAILED;

	if (status != DRV_STATUS_SUCCESS)
	{
		//read failed , check the retry count, to reRead or finish the read process.
		if (tryCount >= I2C_ComParams.I2C_ReRx_Max)
		{
			status = DRV_STATUS_FAILED;
			goto finish_read;
		}
		else
		{
			tryCount++;
			goto try_read_head;
		}
	}

	//according to the mode NCI or CMS，get the payload len.
	if (I2C_comMode == Mode_CMS)
	{													 //mode cms
		payloadDataLen = (myRxBuf[1] << 8) | myRxBuf[2]; //byte1|byte2 means payload len in CMS mode
	}
	else
	{								 //mode nci
		payloadDataLen = myRxBuf[2]; //byte2 means payload len in NCI mode
	}
	//the buffer used for recieved data is not big enough
	if ((PAYLOAD_HEAD_SIZE + payloadDataLen) > maxBufferLen)
	{
		status = DRV_STATUS_RX_OVERFLOW;
		goto finish_read;
	}

	if ((PAYLOAD_HEAD_SIZE + payloadDataLen) > MAX_RECEIVE_BUFFER_SIZE)
	{
		status = DRV_STATUS_RX_OVERFLOW;
		goto finish_read;
	}

//read remaining payload
try_read_data:
	status = DRV_STATUS_SUCCESS;

	if (port_hardware_i2cMasterReceive(address, myRxBuf + PAYLOAD_HEAD_SIZE, payloadDataLen + CRC_SIZE, I2C_TIMEOUT) != PORT_COM_OK) //include crc
		status = DRV_STATUS_FAILED;

	if (status != DRV_STATUS_SUCCESS)
	{
		//if read failed，read again
		if (tryCount >= I2C_ComParams.I2C_ReRx_Max)
		{
			status = DRV_STATUS_FAILED;
			goto finish_read;
		}
		else
		{
			tryCount++;
			goto try_read_data;
		}
	}
	else
	{
		//check CRC
		*rxLen = PAYLOAD_HEAD_SIZE + payloadDataLen;
		crc	   = proto_crc16(myRxBuf, *rxLen);
		if (crc != (myRxBuf[*rxLen] << 8 | myRxBuf[*rxLen + 1]))
		{
			//crc error, read again
			if (tryCount >= I2C_ComParams.I2C_ReRx_Max)
			{
				status = DRV_STATUS_CRC_ERROR;
				goto finish_read;
			}
			else
			{
				tryCount++;
				//read again,shold delay some time which is not shorter the BGT_time and is short than RRT_time
				//here we delay BGT_time
				port_hardware_delayUs(I2C_ComParams.I2C_BGT_Time);
				goto try_read_head;
			}
		}
	}

	//check the recevied data according the mode
	//if the slave receive failed, we need send the last frame again
	if (I2C_comMode == Mode_CMS)
	{
		if (((myRxBuf[*rxLen - 2] << 8) | myRxBuf[*rxLen - 1]) == CMS_MODE_CRC_ERROR) //slave response SW=6500 means receive an error frame
		{
			if (I2C_reTxCount < I2C_ComParams.I2C_ReTx_Max)
			{
				//PORT_HARDWARE_IRQCOMDISABLE();
				if (i2cReTx() == DRV_STATUS_SUCCESS) //send the last fame again
				{
					while (!port_hardware_irqIsComIrqHigh()) //wait the IRQ pull up
					{
						;
					}
					//PORT_HARDWARE_IRQCOMENABLE();//this time donot enable iqr in Simplified SDK
					port_hardware_delayUs(NFCC_BGT_TIME);
					goto try_read_head;
				}
				else
				{
					//PORT_HARDWARE_IRQCOMENABLE();//this time donot enable iqr in Simplified SDK
					status = DRV_STATUS_RETX_ERROR;
					goto finish_read;
				}
			}
		}
	}
	else
	{
		if (((myRxBuf[0] & NCI_MT_MASK) >> NCI_MT_SHIFT) == NCI_MT_NTF) //NOTIFY
		{
			if (((myRxBuf[0] & NCI_GID_MASK) == NCI_GID_CORE) && ((myRxBuf[1] & NCI_OID_MASK) == NCI_DATA_NOTIFY))
			{
				if (myRxBuf[3] == NCI_DATA_CRC_ERROR)
				{
					if (I2C_reTxCount < I2C_ComParams.I2C_ReTx_Max)
					{
						if (i2cReTx() == DRV_STATUS_SUCCESS) //send the last fame again
						{
							while (!port_hardware_irqIsComIrqHigh()) //wait the IRQ pull up
							{
								;
							}
							//PORT_HARDWARE_IRQCOMENABLE();//this time donot enable iqr in Simplified SDK
							port_hardware_delayUs(NFCC_BGT_TIME);
							goto try_read_head;
						}
						else
						{
							//PORT_HARDWARE_IRQCOMENABLE();//this time donot enable iqr in Simplified SDK
							status = DRV_STATUS_RETX_ERROR;
							goto finish_read;
						}
					}
				}
			}
		}
	}
	memcpy(rxBuf, myRxBuf, *rxLen); //copy the data to output buffer

finish_read:
	//finish the read process, open the irq interrupt.
	i2cReTx_for_irq();
	return status;
}

DRV_Status I2CSequentialRx(uint8_t *rxBuf, uint16_t *rxLen)
{
	DRV_Status status = i2cSequentialRx(I2C_ComParams.I2C_Slave_Addr, rxBuf, rxLen);
	return status;
}
