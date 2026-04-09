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

/* Includes ------------------------------------------------------------------*/
#include "port.h"
#include "port_hardware.h"
#include "nfc_drv.h"
#include "port_os.h"
#include "test_hce_mode.h"
#include "nfc_stage.h"

#define CONFIG_NFCC_TASK_STACK_SIZE 10000

extern void start_hce_mode();

volatile uint8_t NfcIrqFlag = 0;
bool nfcclose = false;
bool i2c_init_flag = false;
bool spi_init_flag = false;
extern bool isTX;
static uint8_t hce_aid[8]= {0xA0,0x00,0x11,0x22,0x33,0x44,0x55,0x66};
static uint8_t fido_hce_aid[8]= {0xA0,0x00,0x00,0x06,0x47,0x2F,0x00,0x01};
/*static uint8_t g_TestSectionData[65]		  = {
	 0x11, 0x23, 0x45, 0x00, 0xEE, 0xDC, 0xBA, 0xFF, 0x11, 0x23, 0x45, 0x00, 0x09, 0xF6, 0x09, 0xF6,
	 0x11, 0x23, 0x45, 0x00, 0xEE, 0xDC, 0xBA, 0xFF, 0x11, 0x23, 0x45, 0x00, 0x09, 0xF6, 0x09, 0xF6,
	 0x11, 0x23, 0x45, 0x00, 0xEE, 0xDC, 0xBA, 0xFF, 0x11, 0x23, 0x45, 0x00, 0x09, 0xF6, 0x09, 0xF6,
	 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};*/
uint8_t g_keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static I2C_HandleTypeDef hi2c;
static UART_HandleTypeDef huart;
portSemaphoreId evtSignal;

static TX_THREAD nfcc_thread;
static TX_THREAD nfcc_rw_thread;
static char nfcc_stack[CONFIG_NFCC_TASK_STACK_SIZE];
static char nfcc_rw_stack[CONFIG_NFCC_TASK_STACK_SIZE];
/**
* IRQ functions
*/
// static GPIO_TypeDef *const Gi2c_scl = GPIOA;
// static uint32_t Pini2c_scl = GPIO_PIN_0;
// static GPIO_TypeDef *const Gi2c_sda = GPIOA;
// static uint32_t Pini2c_sda = GPIO_PIN_1;
static GPIO_TypeDef *const Gven = CONFIG_NFC_VEN_PORT;
static uint32_t Pinven = CONFIG_NFC_VEN_PIN;
static GPIO_TypeDef *const Gdwl = CONFIG_NFC_DWL_PORT;
static uint32_t Pindwl = CONFIG_NFC_DWL_PIN;
static GPIO_TypeDef *const Girq = CONFIG_NFC_IRQ_PORT;
static uint32_t Pinirq = CONFIG_NFC_IRQ_PIN;
static GPIO_TypeDef *const Gvddio = CONFIG_NFC_VDDIO_PORT;
static uint32_t Pinvddio = CONFIG_NFC_VDDIO_PIN;
static TX_EVENT_FLAGS_GROUP nfc_data_event_flags;
#define NFC_DATA_EVENT 0x1
//nfc LDO
// static GPIO_TypeDef *const Gnfc_ldo = GPIOA;
// static uint32_t Pinnfc_ldo = GPIO_PIN_6;


// extern void phTmlNfc_TriggerRead();

void port_set_irq_flag(void)
{
	NfcIrqFlag = 1;
    tx_event_flags_set(&nfc_data_event_flags, NFC_DATA_EVENT, TX_OR);
}

/***
 * 
 * RTOS 采用接受信号量  ---此工程中标志等待超时时间未生效
 */
uint8_t port_get_irq_flag(uint32_t time_out)
{
	uint32_t i = time_out;
	if (i == 0)
	{
		if ((NfcIrqFlag == 1) || ((port_hardware_irqIsComIrqHigh() == TRUE)))
		{
			NfcIrqFlag = 0;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	while (i--)
	{
		if ((NfcIrqFlag == 1) || ((port_hardware_irqIsComIrqHigh() == TRUE)))
		{
			NfcIrqFlag = 0;
			return TRUE;
		}
		else
		{
			port_timer_delay_ms(1000);
		}
	}

	return FALSE;
}

void port_hardware_irqCallback(void)
{
	port_set_irq_flag();
}

/*******************************************************************************
  **
  ** Function         sdk_callback_fun
  **
  ** Description      call back function used for NFCC reset
  **                  
	** Parameters       event
  ** Returns          None
  **
  *******************************************************************************/
void sdk_callback_fun(uint8_t event)
{
    NFC_TRACE_DEBUG("sdk_callback_fun, event = %x", event);
}

bool nfc_semaphore_create()
{
    PORT_Status_t status = port_semaphore_create(&evtSignal, 1, "evtSignal");
    if (status == PORT_STATUS_SUCCESS)
    {
        return true;
    }
    return false;
}

void set_nfc_route()
{
    t_RoutTablePra routeList[2];
    routeList[0].qualifier_type = 0x42;
    routeList[0].nfcee_id		 = DH_HOST;
    routeList[0].power_state	 = 0x01;
    memcpy(routeList[0].aid, fido_hce_aid, 8);
    routeList[0].aid_len = 8;

    routeList[1].qualifier_type = 0x42;
    routeList[1].nfcee_id		 = DH_HOST;
    routeList[1].power_state	 = 0x01;
    memcpy(routeList[1].aid, hce_aid, 8);
    routeList[1].aid_len = 8;
    NFCC_SetRouteList(routeList, 2);
}

void nfc_change_route(ULONG entry)
{
    tx_event_flags_create(&nfc_data_event_flags, "usb_rx_event");
    NFCC_Init(); //初始路由到超级SIM

    NFCC_LocalM1SetUid(0, 0x11223344);
    NFCC_ChangeRoute(LOCAL_M1, NULL); //验证通过

    set_nfc_route();
    //NFCC_ChangeRoute(DH_HOST, hce_aid); //验证通过

    tx_event_flags_set(&nfc_data_event_flags, NFC_DATA_EVENT, TX_OR);
    nfcclose = false;
    while (1)
    {  
        ULONG actual_nfc_data_flags = 0;
        tx_event_flags_get(&nfc_data_event_flags,
                           NFC_DATA_EVENT, TX_OR_CLEAR,
                           &actual_nfc_data_flags, TX_WAIT_FOREVER);
        if (TRUE == port_get_irq_flag(0) && nfcclose != true)
        {
            NFCC_Poll(); //其他NTF处理
        }
    }
}

TX_QUEUE nfc_api_queue;
bool create_nfc_api_queue()
{
    VOID *pointer = NULL;
    pointer = malloc(10 * sizeof(NFC_API_MESSAGE));

    UINT status = tx_queue_create(&nfc_api_queue, "NFC api Queue", sizeof(NFC_API_MESSAGE) / 4, pointer, 10 * sizeof(NFC_API_MESSAGE));
    if (status == TX_SUCCESS)
    {
        NFC_TRACE_INFO("create NFC api Queue success");
    }
    else
    {
        NFC_TRACE_INFO("create NFC api Queue fail");
        return false;
    }
    return true;
}

void nfc_rw_entry(ULONG Timeout)
{
    NFCC_Init();
    NFCC_OpenRwMode();
    NFCC_Poll();
    create_nfc_api_queue();
}

bool start_nfc_rw_thread()
{
    UINT status;
    status = tx_thread_create(&nfcc_rw_thread,
                              "nfcc_rw_thread",
                              nfc_rw_entry,
                              0,
                              nfcc_rw_stack,
                              CONFIG_NFCC_TASK_STACK_SIZE, 14, 14, 10,
                              TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        printf("Create nfc rw hardware thread failed, statue : %d", status);
        return false;
    }
    return true;
}

bool start_nfc_thread(){
    UINT status;
    status = tx_thread_create(&nfcc_thread,
                              "nfcc_thread",
                              nfc_change_route,
                              0,
                              nfcc_stack,
                              CONFIG_NFCC_TASK_STACK_SIZE, 14, 14, 10,
                              TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        printf("Create nfc hardware thread failed, statue : %d", status);
        return false;
    }
    return true;
}

uint8_t BSP_nfc_close()
{
    nfcclose = true;
    uint8_t status = NFCC_DeInit();
    HAL_GPIO_WritePin(Gvddio, Pinvddio, GPIO_PIN_RESET);
    //port_hardware_irqComIrqDisable();
    tx_thread_terminate(&nfcc_thread);
    status = tx_thread_delete(&nfcc_thread);
    printf("nfc close\n");
    return status;
}

uint8_t BSP_nfc_rw_close()
{
    nfcclose = true;
    uint8_t status = NFCC_DeInit();
    HAL_GPIO_WritePin(Gvddio, Pinvddio, GPIO_PIN_RESET);
    //port_hardware_irqComIrqDisable();
    tx_thread_terminate(&nfcc_rw_thread);
    status = tx_thread_delete(&nfcc_rw_thread);
    printf("nfc rw close\n");
    return status;
}

static void NFC_IRQHandler(void *UserData)
{
    //nfc_event_req();
    port_hardware_irqCallback();
    //Disable_GPIO0_IRQ();
}

void port_hardware_irqComIrqEnable (void)
{
    // GPIO_InitTypeDef GPIO_i2csdaStruct = {0};
    // GPIO_i2csdaStruct.Pin = Pini2c_sda;
    // GPIO_i2csdaStruct.Mode = GPIO_MODE_OUTPUT;
    // GPIO_i2csdaStruct.Pull = GPIO_PULLUP;
    // GPIO_InitTypeDef GPIO_i2csclStruct = {0};
    // GPIO_i2csclStruct.Pin = Pini2c_scl;
    // GPIO_i2csclStruct.Mode = GPIO_MODE_OUTPUT;
    // GPIO_i2csclStruct.Pull = GPIO_PULLUP;
    GPIO_InitTypeDef GPIO_nfcvenStruct = {0};
    GPIO_nfcvenStruct.Pin = Pinven;
    GPIO_nfcvenStruct.Mode = GPIO_MODE_OUTPUT;
    GPIO_nfcvenStruct.Pull = GPIO_NOPULL;
    GPIO_InitTypeDef GPIO_nfcdwlStruct = {0};
    GPIO_nfcdwlStruct.Pin = Pindwl;
    GPIO_nfcdwlStruct.Mode = GPIO_MODE_OUTPUT;
    GPIO_nfcdwlStruct.Pull = GPIO_NOPULL;
    GPIO_InitTypeDef GPIO_nfcirqStruct = {0};
    GPIO_nfcirqStruct.Pin = Pinirq;
    GPIO_nfcirqStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_nfcirqStruct.Pull = GPIO_NOPULL;
    GPIO_nfcirqStruct.IsrHandler = NFC_IRQHandler;
    GPIO_nfcirqStruct.UserData = NULL;
    GPIO_InitTypeDef GPIO_nfcvddioStruct = {0};
    GPIO_nfcvddioStruct.Pin = Pinvddio;
    GPIO_nfcvddioStruct.Mode = GPIO_MODE_OUTPUT;
    GPIO_nfcvddioStruct.Pull = GPIO_NOPULL;
    // GPIO_InitTypeDef GPIO_nfcldoStruct = {0};
    // GPIO_nfcldoStruct.Pin = Pinnfc_ldo;
    // GPIO_nfcldoStruct.Mode = GPIO_MODE_OUTPUT;
    // GPIO_nfcldoStruct.Pull = GPIO_PULLUP;
    // HAL_GPIO_Init(Gi2c_scl, &GPIO_i2csclStruct);
	// HAL_GPIO_Init(Gi2c_sda, &GPIO_i2csdaStruct);
    HAL_GPIO_Init(Gven, &GPIO_nfcvenStruct);
    HAL_GPIO_Init(Gdwl, &GPIO_nfcdwlStruct);
    HAL_GPIO_Init(Girq, &GPIO_nfcirqStruct);
    HAL_GPIO_Init(Gvddio, &GPIO_nfcvddioStruct);
    // HAL_GPIO_Init(Gi2c_sda, &GPIO_i2csdaStruct);
    // HAL_GPIO_Init(Gi2c_scl, &GPIO_i2csclStruct);
    // HAL_GPIO_Init(Gnfc_ldo, &GPIO_nfcldoStruct);
}
void port_hardware_irqComIrqDisable(void)
{
    // HAL_GPIO_DeInit(Gi2c_scl, Pini2c_scl);
    // HAL_GPIO_DeInit(Gi2c_sda, Pini2c_sda);
    HAL_GPIO_DeInit(Gdwl, Pinven);
    HAL_GPIO_DeInit(Gdwl, Pindwl);
    HAL_GPIO_DeInit(Girq, Pinirq);
    HAL_GPIO_DeInit(Gvddio, Pinvddio);
    // HAL_GPIO_DeInit(Gnfc_ldo, Pinnfc_ldo);
}

//check if the communication(SPI/I2C) irq flag is valid ,use value
bool port_hardware_irqComIrqCheck()
{
   if((HAL_GPIO_ReadPin(Girq, Pinirq) != GPIO_PIN_RESET))
       return true;
   else
       return false;
}

//Clear the communication(SPI/I2C) irq flag former
void port_hardware_irqComIrqClear()
{
	//nrf_gpiote_event_clear(I2CIRQ_GPO_PIN);
}

/**
* GPIO functions
*/

//is i2c/spi IRQ pull up
bool port_hardware_irqIsComIrqHigh()
{
    return (HAL_GPIO_ReadPin(Girq, Pinirq) != GPIO_PIN_RESET);
}

#ifdef CONFIG_NFC_DARIC_ACTIVECARD
//IOControl functions
void port_hardware_gpioVenPullup()
{
    HAL_GPIO_WritePin(Gven, Pinven, GPIO_PIN_RESET);
}
void port_hardware_gpioVenPulldown()
{
    HAL_GPIO_WritePin(Gven, Pinven, GPIO_PIN_SET);
}
#endif

#ifdef CONFIG_NFC_DARIC_EVB
void port_hardware_gpioVenPullup()
{
    HAL_GPIO_WritePin(Gven, Pinven, GPIO_PIN_SET);
}
void port_hardware_gpioVenPulldown()
{
    HAL_GPIO_WritePin(Gven, Pinven, GPIO_PIN_RESET);
}
#endif

void port_hardware_gpioDwlPullup()
{
    HAL_GPIO_WritePin(Gdwl, Pindwl, GPIO_PIN_SET);
}
void port_hardware_gpioDwlPulldown()
{
    HAL_GPIO_WritePin(Gdwl, Pindwl, GPIO_PIN_RESET);
}

/**
*I2C functions
*/
bool port_hardware_i2cIsInitialized(void)
{
    return i2c_init_flag;

}


PORT_Com_status port_hardware_i2cInit(uint8_t rate_level)
{
    memset(&hi2c, 0, sizeof(I2C_HandleTypeDef));

    hi2c.instance_id = CONFIG_NFC_I2C_ID;
    hi2c.init.wait = 1;
    hi2c.init.repeat = 1;
    hi2c.init.baudrate = 100;
    hi2c.init.rx_buf = 0;
    hi2c.init.rx_size = 0;
    hi2c.init.tx_buf = 0;
    hi2c.init.tx_size = 0;
    hi2c.init.cmd_buf = 0;
    hi2c.init.cmd_size = 0;
		i2c_init_flag = false;
		//twi interface init
		//HAL_I2C_DeInit(&hi2c);
		HAL_I2C_Init(&hi2c);
        HAL_I2C_RegisterCallback(&hi2c, HAL_I2C_NACK_CB_ID,
                           I2C_Nack_cb);
		//enalbe the irq event-in
		port_hardware_irqComIrqEnable();
        HAL_GPIO_WritePin(Gvddio, Pinvddio, GPIO_PIN_SET);
//        return PORT_COM_ERROR;
		i2c_init_flag = true;
    return PORT_COM_OK;
}


PORT_Com_status port_hardware_i2cMasterTransmit(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
        isTX = true;
		if(HAL_I2C_Transmit(&hi2c,(uint8_t) DevAddress, pData,(uint32_t)Size, Timeout) == HAL_OK)
			return PORT_COM_OK;
		else
			return PORT_COM_ERROR;
}

PORT_Com_status port_hardware_i2cMasterReceive(uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
        isTX = false;
		if(HAL_I2C_Receive(&hi2c, DevAddress, pData, (uint32_t)Size, Timeout) == HAL_OK)
			return PORT_COM_OK;
		else
			return PORT_COM_ERROR;
}

/**
*Uart functions
*/

/**
  * @brief  This function initalize the UART handle.
	* @param	none
  * @retval uart init result
  */
PORT_Com_status port_hardware_uartInit()
{
      memset(&huart, 0, sizeof(UART_HandleTypeDef));

     huart.id = 0;
  huart.init.BaudRate = 115200;
  huart.init.Rx_En = 1;
  huart.init.Tx_En = 1;
  huart.init.Clean_Rx_Fifo = 0;
  huart.init.Poll_En = 0;
  huart.init.StopBits = 0;   // 1BIT
  huart.init.WordLength = 3; // 8BIT
  huart.init.Parity = 0;
	if(HAL_UART_Init(&huart) != HAL_OK)
		return PORT_COM_ERROR;
	else
		return PORT_COM_OK;
}

PORT_Com_status port_hardware_uartTransmit(uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	if(HAL_UART_Transmit(&huart, pData, (uint32_t)Size, Timeout) != HAL_OK)
		return PORT_COM_ERROR;
	else
		return PORT_COM_OK;
}


void port_hardware_delayUs(uint32_t time_us)
{
	tx_thread_sleep((ULONG)time_us);
}


void port_hardware_delayMs(uint32_t time_ms)
{
	tx_thread_sleep(time_ms*(uint32_t)1000);
}



typedef enum
{
    PORT_FLASH_OK   = 0,
    PORT_FLASH_ERR,
}PORT_Flash_status;

#define FWDL_Flag						0
#define FWDL_Start					0xA5
#define FWDL_Done						0x5A


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!这个地址仅在nordic平台适用，移植到其他平台务必修改!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#define PORT_FLASH_END_ADDR              0x100000
#define PORT_FLASH_PAGE_SIZE             0x1000
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!这个地址仅在nordic平台适用，移植到其他平台务必修改!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#define FWDL_FLAG                     (PORT_FLASH_END_ADDR - PORT_FLASH_PAGE_SIZE)

volatile uint8_t fwdl_flag_default[1] = {0x5A};

uint8_t port_hardware_flashWriteU8(uint32_t dest_addr, uint8_t value)
{
	*(__IO uint8_t *)(dest_addr) = value;
	return PORT_FLASH_OK;
}

uint8_t port_hardware_flashReadU8(uint32_t dest_addr)
{
    return *(__IO uint8_t*)(dest_addr);
}
