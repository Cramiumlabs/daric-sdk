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

/**@file  port_spi_ese.c
* @brief  SPI �ӿ�Ӳ������ʵ����������
* @author  zhengwd
* @date  2021-4-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "port_spi_ese.h"
#include "error.h"
#include "util.h"
#include "log.h"
//#include "port_config.h"
//#include "global_var.h"   //Э��ջ�����ļ�, ��ֲʱ��ɾ��



/**************************************************************************
* Global Variable Declaration
***************************************************************************/
SPIM_HandleTypeDef spi_ese_comm_handle_se0={0};
spi_ese_comm_param_t spi_ese_comm_parm_se0={&spi_ese_comm_handle_se0, ESE_PERIPHERAL_SE0, PORT_SPI_ESE_NSS_MODE};
static uint8_t g_spi_ese_device_init[MAX_PERIPHERAL_DEVICE]={FALSE};

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PORT 
  * @brief hardware  portable layer .
  * @{
  */


/** @defgroup PORT_SPI_ESE PORT_SPI_ESE
  * @brief hardware portable layer spi interface driver.
  * @{
  */



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function  -----------------------------------------------*/


/**
* @brief ������SE ��gpio���г�ʼ��
* @return no	
* @note no
*/
// void port_spi_ese_gpio_init(void)
// {
// 	GPIO_InitTypeDef  GPIO_InitStruct = {0};

// 	GPIO_InitStruct.Pull  = GPIO_PULLUP;
// 	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP; 
// 	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

// 	//---SE RST ����IO----
// 	GPIO_InitStruct.Pin   = PORT_SPI_ESE_SE0_RST_IO_PIN;
// 	PORT_SPI_ESE_SE0_RST_IO_CLK_ENABLE();
// 	HAL_GPIO_Init(PORT_SPI_ESE_SE0_RST_IO_PORT, &GPIO_InitStruct);
// }


// void port_spi_ese_se0_cs_init(void)
// {
// 	GPIO_InitTypeDef  GPIO_InitStruct = {0};

// 	GPIO_InitStruct.Pull  = GPIO_PULLUP;
// 	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP; 
// 	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

// 	//---SE CS ����IO----
// 	GPIO_InitStruct.Pin  = SPI_ESE_CS_PIN;
// 	PORT_SPI_ESE_SE0_CS_OFF();
// 	HAL_GPIO_Init(SPI_ESE_CS_GPIO_PORT, &GPIO_InitStruct);
// }


/**
* @brief SPI�ӿڳ�ʼ��
* @param [in] hspi  spi �ӿھ��
* @param [in] spi_instance  �ӿ�ʵ��
* @param [in] nss_mode  NSSģʽ
* @return ��������״̬��
* @note no
*/
se_error_t port_spi_ese_init(void *handle, void *spi_instance, uint8_t nss_mode)
{
    // GPIO_InitTypeDef  GPIO_InitStruct = {0};
	 SPIM_HandleTypeDef *HEDSEspiHandle = (SPIM_HandleTypeDef *)handle;
    
    // /*##-1- Enable peripherals and GPIO Clocks #################################*/
	// /* Enable GPIO TX/RX clock */
	// SPI_ESE_CS_GPIO_CLK_ENABLE();
	// SPI_ESE_SCK_GPIO_CLK_ENABLE();
	// SPI_ESE_MISO_GPIO_CLK_ENABLE();
	// SPI_ESE_MOSI_GPIO_CLK_ENABLE();
	// /* Enable SPI clock */
	// SPI_ESE_CLK_ENABLE(); 
    
    // /*##-2- Configure peripheral GPIO ##########################################*/  
	// /* SPI SCK GPIO pin configuration  */
	// GPIO_InitStruct.Pin         = SPI_ESE_SCK_PIN;
	// GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
	// GPIO_InitStruct.Pull        = GPIO_PULLUP;	//debug  �����ܽ���powerdownʱ��ʹ��control��io����spi��������
	// GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_VERY_HIGH;
	// GPIO_InitStruct.Alternate   = SPI_ESE_SCK_AF;
	// HAL_GPIO_Init(SPI_ESE_SCK_GPIO_PORT, &GPIO_InitStruct);

	// /* SPI MISO GPIO pin configuration  */
	// GPIO_InitStruct.Pin         = SPI_ESE_MISO_PIN;
	// GPIO_InitStruct.Alternate   = SPI_ESE_MISO_AF;
	// HAL_GPIO_Init(SPI_ESE_MISO_GPIO_PORT, &GPIO_InitStruct);

	// /* SPI MOSI GPIO pin configuration  */
	// GPIO_InitStruct.Pin         = SPI_ESE_MOSI_PIN;
	// GPIO_InitStruct.Alternate   = SPI_ESE_MOSI_AF;
	// HAL_GPIO_Init(SPI_ESE_MOSI_GPIO_PORT, &GPIO_InitStruct);

	// /* SPI CS GPIO pin configuration  */
	// GPIO_InitStruct.Pin         = SPI_ESE_CS_PIN;
	// GPIO_InitStruct.Alternate   = SPI_ESE_CS_AF;
	// HAL_GPIO_Init(SPI_ESE_CS_GPIO_PORT, &GPIO_InitStruct);
    
	// /* Set the SPI parameters */
	// (*HEDSEspiHandle).Instance               = (SPI_TypeDef *)spi_instance;
	  
	// (*HEDSEspiHandle).Init.Direction         = SPI_DIRECTION_2LINES;
	// (*HEDSEspiHandle).Init.CLKPhase          = SPI_PHASE_1EDGE;
	// (*HEDSEspiHandle).Init.CLKPolarity       = SPI_POLARITY_LOW;
	// (*HEDSEspiHandle).Init.DataSize          = SPI_DATASIZE_8BIT;
	// (*HEDSEspiHandle).Init.FirstBit          = SPI_FIRSTBIT_MSB;
	// (*HEDSEspiHandle).Init.TIMode            = SPI_TIMODE_DISABLE;
	// (*HEDSEspiHandle).Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	// (*HEDSEspiHandle).Init.CRCPolynomial     = 7;
	// (*HEDSEspiHandle).Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
	// (*HEDSEspiHandle).Init.NSSPMode          = SPI_NSS_PULSE_DISABLE;
	// (*HEDSEspiHandle).Init.Mode = SPI_MODE_MASTER;

	// if(nss_mode == PORT_SPI_ESE_NSS_SOFT)
	// {
	// 	(*HEDSEspiHandle).Init.NSS               = SPI_NSS_SOFT;
		
	// }
	// else if(nss_mode == PORT_SPI_ESE_NSS_HARD_OUTPUT)
	// {
	// 	(*HEDSEspiHandle).Init.NSS               = SPI_NSS_HARD_OUTPUT;
	// }
    
    // /*
    //  * ��switchΪЭ��ջ������룬������Ҫ��ѡSPIƵ�ʣ��ɶ�BaudRatePrescaler��ʼ��Ϊ�̶���Ƶֵ
    //  */
    // switch(g_eSeSpiFreq)
    // {
    //     case SE_SPI_FREQUENCY_1p25:
    //         (*HEDSEspiHandle).Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    //         break;
        
    //     case SE_SPI_FREQUENCY_2p5:
    //         (*HEDSEspiHandle).Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    //         break;
        
    //     case SE_SPI_FREQUENCY_5:
    //         (*HEDSEspiHandle).Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    //         break;
        
    //     case SE_SPI_FREQUENCY_10:
    //         (*HEDSEspiHandle).Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    //         break;
        
    //     case SE_SPI_FREQUENCY_20:
    //         (*HEDSEspiHandle).Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    //         break;
    // }
    HEDSEspiHandle->baudrate = 20000000;                // Set baud rate to 32MHz
    HEDSEspiHandle->id = 2;                             // SPI interface ID
    HEDSEspiHandle->cs_gpio = -1;                       // Don't use GPIO as chip select
    HEDSEspiHandle->cs = 0;
    HEDSEspiHandle->qspi = 0;                           // Don't use QSPI mode
    HEDSEspiHandle->polarity = 0;    // Clock polarity is positive
    HEDSEspiHandle->phase = 0;       // Clock phase is standard mode
    HEDSEspiHandle->big_endian = 1;                     // Use big endian mode
    HEDSEspiHandle->wordsize = SPIM_WORDSIZE_8;         // 8-bit word length
	if(HAL_SPIM_Init(HEDSEspiHandle) != HAL_OK)
	{
		/* Initialization Error */
		return SE_ERR_INIT;
	}
		
	return SE_SUCCESS;
}


/**
* @brief SPI�ӿ��Ƴ�����ȡ�豸������
* @param [in] hspi	spi �ӿھ��
* @return ��������״̬��
* @note no
*/
se_error_t port_spi_ese_deinit(void * handle)
{
    // /*##-1- Reset peripherals ##################################################*/
	// SPI_ESE_FORCE_RESET();
	// SPI_ESE_RELEASE_RESET();

	// /*##-2- Disable peripherals and GPIO Clocks ################################*/
	// /* Configure SPI SCK as alternate function  */
	// HAL_GPIO_DeInit(SPI_ESE_SCK_GPIO_PORT, SPI_ESE_SCK_PIN);
	// /* Configure SPI MISO as alternate function  */
	// HAL_GPIO_DeInit(SPI_ESE_MISO_GPIO_PORT, SPI_ESE_MISO_PIN);
	// /* Configure SPI MOSI as alternate function  */
	// HAL_GPIO_DeInit(SPI_ESE_MOSI_GPIO_PORT, SPI_ESE_MOSI_PIN);
	// /* Configure SPI CS as alternate function  */	
	// HAL_GPIO_DeInit(SPI_ESE_CS_GPIO_PORT, SPI_ESE_CS_PIN);
    
    if(HAL_SPIM_Deinit((SPIM_HandleTypeDef *)handle) != HAL_OK)
    {
        /* Initialization Error */
         return SE_ERR_INIT;
    }

    return SE_SUCCESS;
}

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/

/** @defgroup Port_Spi_Ese_Exported_Functions Port_Spi_Ese Exported Functions
  * @{
  */

/**
* @brief ���豸periph��ͨ�ų�ʼ��
* -# ������SE��gpio���г�ʼ��
* -# ��ʼ��SPI�ӿ�
* -# ����RST����IOΪ�ߵ�ƽ
* @param [in] periph  �豸���
* @return ��������״̬��	
* @note no
* @see ����  port_spi_ese_gpio_init  port_spi_ese_init
*/
se_error_t port_spi_ese_periph_init (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph) 
{	  
	se_error_t ret_code = SE_SUCCESS;
	void *spim1 = NULL;
	spi_ese_comm_param_pointer p_comm_param = (spi_ese_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		//port_spi_ese_gpio_init();
	
		if(g_spi_ese_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = port_spi_ese_init(p_comm_param->spi_ese_handle, spim1, p_comm_param->nss_mode);
			if(ret_code != SE_SUCCESS)
			{
				break;
			}
			g_spi_ese_device_init[p_comm_param->slave_id] = TRUE;
		}

		// if(p_comm_param->slave_id == ESE_PERIPHERAL_SE0)
		// {
		// 	PORT_SPI_ESE_SE0_RST_HIGH();   //�ߵ�ƽ
		// 	if(p_comm_param->nss_mode == PORT_SPI_ESE_NSS_SOFT)
		// 	{
		// 		PORT_SPI_ESE_SE0_CS_OFF();
		// 		port_spi_ese_se0_cs_init();
		// 	}
		// }	
	}while(0);

	return ret_code;
}


/**
* @brief ���豸periph ��ͨ����ֹ��
* -# ��ֹ��SPI�ӿ�
* -# ����RST����IOΪ�͵�ƽ
* @param [in] periph  �豸���
* @return ��������״̬��	
* @note no
* @see ����  port_spi_ese_deinit
*/
se_error_t port_spi_ese_periph_deinit (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph) 
{
	se_error_t ret_code = SE_SUCCESS;
	spi_ese_comm_param_pointer p_comm_param = (spi_ese_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if(p_comm_param->slave_id == ESE_PERIPHERAL_SE0)
		{
			if(g_spi_ese_device_init[ESE_PERIPHERAL_SE0] == TRUE)
			{
				ret_code = port_spi_ese_deinit(p_comm_param->spi_ese_handle);
			}
			//PORT_SPI_ESE_SE0_RST_LOW();   //�͵�ƽ	
		}

		g_spi_ese_device_init[p_comm_param->slave_id] = FALSE;

		// HAL_GPIO_WritePin(SPI_ESE_SCK_GPIO_PORT, SPI_ESE_SCK_PIN, GPIO_PIN_RESET);
		// HAL_GPIO_WritePin(SPI_ESE_MISO_GPIO_PORT, SPI_ESE_MISO_PIN, GPIO_PIN_RESET);
		// HAL_GPIO_WritePin(SPI_ESE_MOSI_GPIO_PORT, SPI_ESE_MOSI_PIN, GPIO_PIN_RESET);
		// HAL_GPIO_WritePin(SPI_ESE_CS_GPIO_PORT, SPI_ESE_CS_PIN, GPIO_PIN_RESET);  

	}while(0);

    return ret_code;
}


/**
* @brief ���豸periphѡ��ʹ�ܻ�ȥʹ��
* @param [in] enable  ʹ��: TRUE,  ȥʹ��: FALSE
* @return ��������״̬��	
* @note ������STM32 Hal��ķ��ͺ���HAL_SPI_Transmit�У�����ú���__HAL_SPI_ENABLE��
*	      ��ʹ��ʱ���ɲ����ٵ��ô˺���������STM32 Hal��Ľ��պ���HAL_SPI_Receive
*	      δ����__HAL_SPI_DISABLE����ȥʹ��ʱ����Ҫ���ô˺���
*/
se_error_t port_spi_ese_periph_chip_select (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, bool enable) 
{
	se_error_t ret_code = SE_SUCCESS;
	spi_ese_comm_param_pointer p_comm_param = (spi_ese_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}

		if(g_spi_ese_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = SE_ERR_COMM;
			break;
		}
				
		// if(enable == TRUE)
		// {	
        //     //__HAL_SPI_ENABLE(p_comm_param->spi_ese_handle);
		// 	if(p_comm_param->nss_mode == PORT_SPI_ESE_NSS_SOFT)
		// 	{
		// 	//	__HAL_SPI_ENABLE(p_comm_param->spi_ese_handle);
		// 		if(p_comm_param->slave_id == ESE_PERIPHERAL_SE0)
		// 		{
		// 			PORT_SPI_ESE_SE0_CS_ON();
		// 		}	
		// 	}
		// }
		// else
		// {
		// 	if(p_comm_param->nss_mode == PORT_SPI_ESE_NSS_SOFT)
		// 	{
		// 		if(p_comm_param->slave_id == ESE_PERIPHERAL_SE0)
		// 		{
		// 			PORT_SPI_ESE_SE0_CS_OFF();
		// 		}	
		// 	}
		// 	__HAL_SPI_DISABLE (p_comm_param->spi_ese_handle);  
		// }
	}while(0);

	return ret_code;
}


/**
* @brief ͨ��SPI�ӿڴ�����periph���Ͷ��ֽ�����
* -# ����mcu hal���HAL_SPI_Transmit�������Ͷ��ֽ�����
* @param [in] periph  �豸���
* @param [in] inbuf  ���������ݵ���ʼ��ַ
* @param [in] inbuf_len ���������ݵĳ���    
* @return ��������״̬��	
* @note no
*/
se_error_t port_spi_ese_periph_transmit(HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf, uint32_t  inbuf_len) 
{
	se_error_t ret_code = SE_SUCCESS;
	spi_ese_comm_param_pointer p_comm_param = (spi_ese_comm_param_pointer)periph->extra;
	
	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if((inbuf == NULL) || (inbuf_len == 0U))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		if(g_spi_ese_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = SE_ERR_COMM;
			break;
		}

		ret_code = HAL_SPIM_Send(p_comm_param->spi_ese_handle, inbuf, inbuf_len, SPIM_CS_AUTO, HAL_MAX_DELAY);
		if(ret_code == HAL_BUSY)
		{
			ret_code = SE_ERR_BUSY;
			break;
		}
		else if(ret_code == HAL_TIMEOUT)
		{
			ret_code = SE_ERR_TIMEOUT;
			break;
		}
		else if(ret_code != HAL_OK)
		{
			ret_code = SE_ERR_COMM;
			break;
		}

	}while(0);
	
	return ret_code;
}


/**
* @brief ͨ��SPI�ӿڴ�����periph���ն��ֽ�����
* -# ����mcu hal���HAL_SPI_Receive�������ն��ֽ�����
* @param [in] periph  �豸���
* @param [out] outbuf  ���������ݵ���ʼ��ַ
* @param [out] outbuf_len ���������ݵĳ���      
* @return ��������״̬��	
* @note no
*/
se_error_t port_spi_ese_periph_receive(HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf, uint32_t *outbuf_len) 
{
	se_error_t ret_code = SE_ERR_COMM;
	spi_ese_comm_param_pointer p_comm_param = (spi_ese_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if((outbuf == NULL) || (outbuf_len == NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		if(g_spi_ese_device_init[p_comm_param->slave_id] == FALSE)
		{
			ret_code = SE_ERR_COMM;
			break; 
		}
		ret_code = HAL_SPIM_Receive(p_comm_param->spi_ese_handle, outbuf, *outbuf_len, SPIM_CS_KEEP, HAL_MAX_DELAY);
		NFC_TRACE_BUFFER_INFO("--> ", outbuf, *outbuf_len);
		if(ret_code == HAL_BUSY)
		{
			ret_code = SE_ERR_BUSY;
			break;
		}
		else if(ret_code == HAL_TIMEOUT)
		{
			ret_code = SE_ERR_TIMEOUT;
			break;
		}
		else if(ret_code != HAL_OK)
		{
			ret_code = SE_ERR_COMM;
			break;
		}

	}while(0);
	
	return ret_code;
}


/**
* @brief ���ݿ�������������ݣ�������periph ���и�λ���Ʋ���
* @param [in] periph  �豸���
* @param [in] ctrlcode  ������
* @param [in] inbuf  ���Ϳ������ݵ���ʼ��ַ
* @param [in] inbuf_len ���Ϳ������ݵĳ���         
* @return ��������״̬��	
* @note no
*/
se_error_t port_spi_ese_periph_control(HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *inbuf, uint32_t  *inbuf_len) 
{
	se_error_t ret_code = SE_SUCCESS;
	//spi_ese_comm_param_pointer p_comm_param = (spi_ese_comm_param_pointer)periph->extra;

	do
	{
		if(periph == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if(ctrlcode == 0U)
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		// if(ctrlcode == PORT_SPI_ESE_CTRL_RST)
		// {
		// 	if(p_comm_param->slave_id == ESE_PERIPHERAL_SE0)
		// 	{
		// 		PORT_SPI_ESE_SE0_RST_LOW();
		// 		port_spi_ese_periph_delay(PORT_SPI_ESE_SE_RST_LOW_DELAY);  //��λʱ��RST�͵�ƽ����ʱ��
		// 		PORT_SPI_ESE_SE0_RST_HIGH(); 
		// 		port_spi_ese_periph_delay(PORT_SPI_ESE_SE_RST_HIGH_DELAY);  //��λ��RST�ߵ�ƽ����ʱ��
		// 	}
		// }

	}while(0);

	return ret_code;
}

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/

/** (@defgroup Port_Stm32l433_Spi_Exported_Functions Port_Stm32l433_Spi Exported Functions
  * @{
  */


/**
* @brief ����΢���ʱ
* @param [in] us  ��ʱ����ֵ����λΪ΢��
* @return no	
* @note no
*/
se_error_t port_spi_ese_periph_delay(uint32_t us)
{
	//uint32_t i = 0;
	//uint32_t j = 0;
	tx_thread_sleep(1);
	// for(i=0;i<us;i++)
	// {
	// 	//ϵͳʱ��Ϊ48Mhzʱ��forѭ����ֵֹΪ11��
	// 	//ϵͳʱ��Ϊ80Mhzʱ��forѭ����ֵֹΪ18��
	// 	//for(j=0;j<18;j++);
	// }
	return SE_SUCCESS;
}



/**
* @brief ������ʱ
* @param [out]  timer_start  ��ʼʱ��ֵ
* @return ��������״̬��	
* @note no
*/
se_error_t port_spi_ese_periph_timer_start(util_timer_t *timer_start) 
{
	timer_start->tv_msec = HAL_GetTick(); 
	return SE_SUCCESS;			
}


/**
* @brief �жϼ�ʱ�Ƿ�ʱ
* @param [in] timer_differ �������ʼʱ��ֵ 
* @return ��������״̬��	
* @note no
*/
se_error_t port_spi_ese_periph_timer_differ(util_timer_t *timer_differ) 
{
	
	uint32_t start, end;
	uint32_t time_use_ms;

	start = timer_differ->tv_msec;
	end = HAL_GetTick(); 

	if(end >= start)
	{
		time_use_ms = end - start;
	}
	else
	{
		time_use_ms = 0xffffffff -(start - end);
	}

	if(time_use_ms>=timer_differ->interval)
	{
		return SE_ERR_TIMEOUT;	
	}
	return SE_SUCCESS;
}


SPI_ESE_PERIPHERAL_DEFINE_BEGIN(ESE_PERIPHERAL_SE0)
    port_spi_ese_periph_init,
    port_spi_ese_periph_deinit,
    port_spi_ese_periph_delay,
    port_spi_ese_periph_timer_start,
    port_spi_ese_periph_timer_differ,
    port_spi_ese_periph_chip_select,
    port_spi_ese_periph_transmit,
    port_spi_ese_periph_receive,
    port_spi_ese_periph_control,
    &spi_ese_comm_parm_se0,
SPI_ESE_PERIPHERAL_DEFINE_END()

SPI_ESE_PERIPHERAL_REGISTER(ESE_PERIPHERAL_SE0);

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




