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
#include "port.h"

// extern void vTaskDelay(const uint32_t xTicksToDelay );
// extern uint32_t xTaskGetTickCount( void );
// extern uint32_t xTaskCreate(	void* pxTaskCode, const char * const pcName, const uint16_t usStackDepth, void * const pvParameters, uint32_t uxPriority, void * const pxCreatedTask );
// extern void vTaskDelete( void* xTaskToDelete );
// extern void* xTaskGetCurrentTaskHandle( void );


// static SemaphoreHandle_t xCanSemaphore = NULL;


// thread related interface 
UINT port_thread_create(const portThreadDef_t *thread_def, void *argument)
{
  //PORT_ThrdHandle_t id;
  UINT status;
  //TX_THREAD *thread_ptr = (TX_THREAD *)port_mem_getMem(sizeof(TX_THREAD)*4);
  status = tx_thread_create(thread_def->threadptr, 
                                          thread_def->name, 
                                          thread_def->pthread, 
                                          (ULONG)argument, 
                                          port_mem_getMem(thread_def->stacksize), 
                                          (ULONG)thread_def->stacksize, 
                                          (UINT)thread_def->tpriority,
                                          (UINT)thread_def->tpriority,  
                                          TX_NO_TIME_SLICE, 
                                          TX_AUTO_START); 
  //id = (PORT_ThrdHandle_t)thread_ptr;    /* create the thread  */
  if (status != TX_SUCCESS)
  {
    /* handle thread creation    */
    printf("%s : fail to create thread", __FUNCTION__);
  }
  else{
    printf("%s :create thread", __FUNCTION__);
    //i++;
  }

  return status;
}

/**
  * @brief Delete a thread.
  * @param TheId Handle of the thread to delete
  * @retval void
  */
void port_thread_delete(PORT_ThrdHandle_t *threadptr)//UINT  _tx_thread_delete(TX_THREAD *thread_ptr)
{
  //TX_THREAD *online = tx_thread_identify();
  tx_thread_delete(threadptr);

}

PORT_ThrdHandle_t* port_thread_getId()
{
 return tx_thread_identify();
}



// semaphore related interface
PORT_Status_t port_semaphore_create( portSemaphoreDef_t *semaphore_id, int32_t count, char *name)
{
  //TX_SEMAPHORE *semaphore_ptr = (TX_SEMAPHORE *)port_mem_getMem(sizeof(TX_SEMAPHORE));
  //semaphore_ptr = (TX_SEMAPHORE *)semaphore_def;
  if(tx_semaphore_create (semaphore_id, name, count) == TX_SUCCESS){
    return PORT_STATUS_SUCCESS;
  }
  else{
    return PORT_STATUS_FAILED;
  }
  //return (portSemaphoreId)semaphore_ptr;
}

PORT_Status_t port_semaphore_wait(portSemaphoreId *semaphore_id, uint32_t millisec)
{
    if(tx_semaphore_get (semaphore_id, millisec) == TX_SUCCESS)
        return PORT_STATUS_SUCCESS;
    else 
        return PORT_STATUS_FAILED;
}

PORT_Status_t port_semaphore_release(portSemaphoreId *semaphore_id)
{
    if(tx_semaphore_put(semaphore_id) == TX_SUCCESS)
       return PORT_STATUS_SUCCESS;
    else 
        return PORT_STATUS_FAILED;
}

PORT_Status_t port_semaphore_delete(portSemaphoreId *semaphore_id)
{
    if(tx_semaphore_delete(semaphore_id) == TX_SUCCESS)
       return PORT_STATUS_SUCCESS;
    else 
        return PORT_STATUS_FAILED;
}



/**
  * @brief Allocate memory according to size provided
  * @param[in] size size to allocate
  * @retval pointer to memory allocated
  */
void *port_mem_getMem(uint32_t size)
{
  void *p;
  // UINT status;
	if(size != 0){
    
		// status = tx_byte_allocate(&byte_pool_nfc, (VOID **) &p, (ULONG)size, TX_NO_WAIT);
    p = malloc(size);
		// if (status != TX_SUCCESS)
    if(p == NULL)
		{
			printf("%s: not enough memory to allocate 0x%04lx bytes\n", __FUNCTION__, size);
		// }
    // else{
    //   printf("%s: enough memory to allocate 0x%04lx bytes\n", __FUNCTION__, size);
    }
	}
	else{
		printf("port_mem_getMem allocate NULL pointer\n");
	}
  return p;
}

/**
  * @brief Free memory
  * @param[in] pMem pointer to memory
  * @retval None
  */
void port_mem_freeMem(void *pMem)
{
  if (NULL == pMem)
  {
    NFC_TRACE_WARNING("port_mem_freeMem NULL pointer");
  }
	else{		
		// tx_byte_release((void*)pMem);
    free(pMem);
	}	
}


void port_timer_delay_ms(ULONG ms)
{
	tx_thread_sleep(ms);
}

PORT_Status_t port_i2c_mutex_init(portMutexId *context, char *name)
{
  PORT_Status_t result;
  UINT handle;
  //TX_MUTEX    *mutex_ptr = (TX_MUTEX *)port_mem_getMem(sizeof(TX_MUTEX));
  //handle = tx_mutex_create(mutex_ptr, " ", TX_NO_INHERIT);
  handle = tx_mutex_create(context, name, TX_NO_INHERIT);
  if (handle != TX_SUCCESS)
  {
    NFC_TRACE_ERROR("%s : fail to create mutex", __FUNCTION__);
    result = PORT_STATUS_FAILED;
  }
  else
  {
    result = PORT_STATUS_SUCCESS;
  }

  //*context = (void *)mutex_ptr;

  return result;
}

PORT_Status_t port_i2c_mutex_wait(portMutexId *context) 
{
	PORT_Status_t status = PORT_STATUS_SUCCESS;
  //void * handle = (void *) context;
  UINT state;
  // NOTE : Give a chance to other task to lock the mutex.
  if (NULL != context)
  {
    state = tx_mutex_get(context, TX_WAIT_FOREVER);
    if (state != TX_SUCCESS)
    {
      NFC_TRACE_ERROR("%s : fail to lock mutex,state = %u\n", __FUNCTION__,state);
			status = PORT_STATUS_FAILED;
    }
  }
  else
  {
    NFC_TRACE_ERROR("%s : NULL context", __FUNCTION__);
		status = PORT_STATUS_FAILED;
  }
	return status;
}

PORT_Status_t port_i2c_mutex_release(portMutexId *context) 
{
	PORT_Status_t status = PORT_STATUS_SUCCESS;
  //void * handle = (void *) context;
  UINT state;
  if (NULL != context)
  {
    state = tx_mutex_put(context);
		if (state != TX_SUCCESS)
		{
			NFC_TRACE_ERROR("%s : fail to unlock mutex,state = %u\n", __FUNCTION__,state);
			status = PORT_STATUS_FAILED;
		}
  }
  else
  {
    NFC_TRACE_ERROR("%s : NULL context", __FUNCTION__);
		status = PORT_STATUS_FAILED;
  }
	return status;
}
