/**@file  comm.c
* @brief  comm interface declearation	 
* @author  liangww
* @date  2022-11-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "comm.h"
#include "util.h"
#include "error.h"
#include "log.h"
//#include "auth.h"
//#include "crypto.h"
//#include "sm4.h"


/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
  * @{
  */

/** @defgroup COMM COMM
  * @brief comm interface api.
  * @{
  */




/**************************************************************************
* Global Variable Declaration
***************************************************************************/
static peripheral_bus_driver *g_drivers[MAX_PERIPHERAL_BUS_DRIVER] = {0};

/* Exported types ------------------------------------------------------------*/
/** @defgroup COMM_Exported_Types comm_driver_holder Exported Types
  * @{
  */

/**
  * @brief  driver_holder Structure definition
  */

struct driver_holder {
    peripheral_bus_driver *driver;
    peripheral *periph;
};

/**
  * @}
  */


#ifdef __MULTITASK
// �����񻷾�
// ÿ�������TLS����һ����ѡ��peripheral_driver����
#ifdef __FREERTOS
#endif

#ifdef __LINUX
#endif
#else
// �����񻷾�
// ͨ��ȫ�ֱ���������ѡ�е�һ��peripheral_driver����
static struct driver_holder g_selected_driver = {NULL, NULL};

#define setSelectedDriver(driver) g_selected_driver.driver = driver
#define setSelectedPeriph(periph) g_selected_driver.periph = periph

#define getSelectedDriver() g_selected_driver.driver
#define getSelectedPeriph() g_selected_driver.periph
#endif

#define checkSelectedDriverAndDevice() \
    do {    \
        if (getSelectedDriver() == NULL || getSelectedDriver() == NULL)   \
            return NULL;    \
    }while(0)

/* Exported functions --------------------------------------------------------*/

/** @defgroup COMM_Exported_Functions COMM Exported Functions
  * @{
  */




void test_se(){
	se_error_t ret;
	uint32_t outlen = 0;
	uint8_t cmd[] = {0x00, 0xA4, 0x04, 0x00, 0x00};
	uint32_t cmdlen = sizeof(cmd);
	unsigned char                g_ucaAppDataBuf[APP_BUF_SIZE];
	unsigned short				g_usAppDataLen;
	g_usAppDataLen = strlen(HOST_DEMO_VERSION);
	memcpy(g_ucaAppDataBuf, HOST_DEMO_VERSION, g_usAppDataLen);
	ret = api_register(PERIPHERAL_ESE, ESE_PERIPHERAL_SE0);
    if(ret != SE_SUCCESS) printf("failed to spi ese api_register");
    
    ret = api_select(PERIPHERAL_ESE, ESE_PERIPHERAL_SE0);
    if(ret != SE_SUCCESS) printf("failed to spi ese api_select");
    
    ret = api_connect_init();
    if(ret != SE_SUCCESS) printf("failed to api_connect_init");
    
    //ret = api_reset(g_ucaAppDataBuf, &outlen);
	ret = api_transceive(cmd, cmdlen, g_ucaAppDataBuf, &outlen);
	printf("ret = %ln", &ret);
    if(ret != SE_SUCCESS) printf("failed to api_reset");
}

/**
* @brief �����豸���
* @param [in] driver	�������������������
* @param [in] periph  ������
* @return �μ�error.h
* @note no
* @see no
*/
static peripheral* add_periph(peripheral_bus_driver *driver, peripheral *periph) {
    int i=0;

    for (; i<MAX_PERIPHERAL_DEVICE; i++) {
        if (driver->periph[i] == periph)
            return periph;
        else
            if (driver->periph[i] == NULL) {
                driver->periph[i] = periph;
                return periph;
            }
    }

    return NULL;
}


/**
* @brief ����������������������豸���
* @param [in] driver	�������������������
* @param [in] periph  ������
* @return �μ�error.h
* @note no
* @see ���� add_periph
*/
static peripheral_bus_driver* add_driver(peripheral_bus_driver *driver, peripheral *periph) {
    int i=0;

    for (; i<MAX_PERIPHERAL_BUS_DRIVER; i++) { 
        if (g_drivers[i] == driver) {
            if (add_periph(driver, periph) != NULL)
                return driver;
            else
                return NULL;
        }
        else {
            if (g_drivers[i] == NULL) {
                g_drivers[i] = driver;
                if (add_periph(driver, periph) != NULL)
                    return driver;
                else
                    return NULL;
            }
        }
    }

    return NULL;
}



/**
* @brief �����������ͣ���g_driver�в������������������
* @param [in] type	�����������Ͷ��������
* @return ���������������
* @note no
* @see no
*/

static peripheral_bus_driver* find_driver(peripheral_type type) {
    int i=0;
	
    for (; i<MAX_PERIPHERAL_BUS_DRIVER; i++) { 
        if (g_drivers[i]->type == type)
            return g_drivers[i];
    }

    return NULL;
}



/**
* @brief �������������������������IDֵ������������
* @param [in] driver	�������������������
* @param [in] dev_id  ����ID
* @return ������
* @note no
* @see no
*/
static peripheral* find_slave_device(peripheral_bus_driver *driver, uint32_t dev_id) {
    int i=0;
	
    for (; i<MAX_PERIPHERAL_DEVICE; i++) { 
        if (driver->periph[i]->id == dev_id)
            return driver->periph[i];
    }

    return NULL;
}




/**
* @brief �������������������ͺ�����ID ֵ����ע�ᣬ������ʵ����
* @param [in] driver	���������������
* @param [in] periph  ����
* @return �μ�error.h
* @note no
* @see ���� add_driver
*/
se_error_t _api_register(peripheral_bus_driver *driver, peripheral *periph) 
{
	if (driver == NULL || periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}
	if(add_driver(driver, periph) == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	return SE_SUCCESS;
}




/**
* @brief ����ע��Ķ�������У�ѡ����Ҫ����������
* @param [in] type	��������
* @param [in] dev_id  ����IDֵ
* @return �μ�error.h
* @note no
* @see ���� find_driver find_slave_device setSelectedDriver setSelectedPeriph
*/
se_error_t api_select(peripheral_type type, uint32_t dev_id) 
{
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
	{
		return SE_ERR_PARAM_INVALID; //δִ��acl_register ����
	}

	driver = find_driver(type);
	if (driver == NULL)
	{
		return SE_ERR_HANDLE_INVALID; 
	}

	periph = find_slave_device(driver, dev_id);
	if (periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID; 
	}

	setSelectedDriver(driver);
	setSelectedPeriph(periph);

	return SE_SUCCESS;
}



/**
* @brief �������輰��ȡ����ATR
* @param [out] out_buf	�������ʼ��ַ
* @param [out] out_buf_len  ������ݳ��ȵ���ʼ��ַ
* @return �μ�error.h
* @note 1. ������������ѡ����������� 2. ���ݼ��ص���������������init ��ʼ��������open����
* @see ���� getSelectedDriver getSelectedPeriph init open
*/

se_error_t api_connect (uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		if (((out_buf == NULL) && (out_buf_len != NULL))||((out_buf != NULL) && (out_buf_len == NULL)))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}
	
		//1. ������鼰�����������
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //δִ��api_register ����
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //ע���˶�����裬����ѡ������
				break;
			}
			//ֻע����һ������ʱ, ����ѡ��ֱ��ʹ�õ�1 ��ע�������
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//ִ����api_select ������ѡ����ָ�����豸
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}

		//2.��ʼ���豸
		ret_code = driver->init(periph);   
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

		//3.���豸
		ret_code = driver->open(periph, out_buf , out_buf_len); //���豸,��ȡATR
		if(ret_code != SE_SUCCESS)
		{
			break;
		}
	
	}while(0);
	
	return ret_code;
}

se_error_t api_connect_init (void)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		//1. ������鼰�����������
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //δִ��api_register ����
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //ע���˶�����裬����ѡ������
				break;
			}
			//ֻע����һ������ʱ, ����ѡ��ֱ��ʹ�õ�1 ��ע�������
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//ִ����api_select ������ѡ����ָ�����豸
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}

		//2.��ʼ���豸
		ret_code = driver->init(periph);   
		if(ret_code != SE_SUCCESS)
		{
			break;
		}
	
	}while(0);
	
	return ret_code;
}

#ifdef CONNECT_NEED_AUTH
/**
* @brief ���ⲿ��֤���������輰��ȡ����ATR
* @param [in] ekey-> algָ���Գ��㷨���ͣ��ֽ�֧��SM4��
* @param [in] ekey-> val_len ��Կֵ����
* @param [in] ekey-> val ��Կ����
* @param [out] out_buf	�������ʼ��ַ
* @param [out] out_buf_len  ������ݳ��ȵ���ʼ��ַ
* @return �μ�error.h
* @note 1.����ִ���ⲿ��֤ 2. ���ִ�����Ӳ��� 
* @see ���� getSelectedDriver getSelectedPeriph init open
*/

se_error_t api_connect_auth (sym_key_t *ekey,  uint8_t *out_buf, uint32_t *out_buf_len)
{
	uint8_t random[16]={0};
	uint8_t cipher_buf[32] = {0};
	se_error_t ret_code = SE_SUCCESS;
	sm4_context sm4_ctxt;

	//�������
	if ((ekey->alg == ALG_SM4) && (ekey->val_len != 16))
	{
		LOGE("failed to api_connect_auth input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//ִ�нӿ�����
	ret_code = api_connect(out_buf, out_buf_len);
	
	//��ȡ16�ֽ������
    ret_code = api_get_random (0x10, random);
	if ( ret_code != SE_SUCCESS)
	{
		LOGE("failed to api_get_random!\n");
		return ret_code;
	}
	
    //�����������SM4���ܻ����֤����
    sm4_ctxt.mode = SM4_ENCRYPT;
	sm4_setkey_enc(&sm4_ctxt,ekey->val);
    switch( ekey ->alg )
    {
		case ALG_SM4:
			sm4_crypt_ecb(&sm4_ctxt, SM4_ENCRYPT, 0x10, random, cipher_buf);	
			break;
		default:
			LOGE("failed to support the sym alg!\n");
			ret_code = SE_ERR_PARAM_INVALID;
			return ret_code;
	}		

	//����api_pair_auth�ӿ�����ⲿ��֤
     ret_code = api_pair_auth(cipher_buf,0x10);
    if ( ret_code != SE_SUCCESS)
	{
		LOGE("failed to api_pair_auth!\n");
		return ret_code;
	}


	return ret_code;
}

#endif

/**
* @brief �Ͽ������������
* @param no
* @return �μ�error.h
* @note no
* @see ���� getSelectedDriver getSelectedPeriph deinit close
*/
se_error_t api_disconnect (void)
{
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	se_error_t ret_code = SE_SUCCESS;

	do
	{
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;//δִ��acl_register ����
			break;  
		}

		//1.��������
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			//δִ��acl_select ����, ʹ�õ�1 ��ע����豸
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//ִ����acl_select ������ѡ����ָ�����豸
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}	

		//2.��ֹ���豸
		ret_code = driver->deinit(periph);   
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

		//3.�ر��豸
		ret_code = driver->close(periph);   
		if(ret_code != SE_SUCCESS)
		{
			break;
		}
		
	}while(0);
	
	return ret_code;
}



/**
* @brief ��˫����и�ʽ������豸�������������Ӧ
* @param [in] in_buf	����˫����е���ʼ��ַ
* @param [in] in_buf_len  ����˫����е��������ݳ���
* @param [out] out_buf	���˫����е���ʼ��ַ
* @param [out] out_buf_len  ���˫����е�������ݳ��ȵ���ʼ��ַ
* @return �μ�error.h
* @note 1.��������Ч�� 2.������������������豸��� 3.ͨ�����صľ��������proto��transceive�������������ݼ�����������Ӧ
* @see ���� getSelectedDriver getSelectedPeriph transceive
*/
se_error_t api_transceive_queue(uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		//������
		if ((in_buf == NULL)||(in_buf_len == 0U) || (out_buf == NULL)||(out_buf_len==NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}
		
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;//δִ��acl_register ����
			break;  
		}

		//1.��������
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			//δִ��acl_select ����, ʹ�õ�1 ��ע����豸
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//ִ����acl_select ������ѡ����ָ�����豸
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}	

		//---����proto�е�proto_spi_transceive��proto_i2c_transceive, 
		//��ѡ����豸�������������������Ӧ����
		ret_code = driver->transceive(periph, in_buf, in_buf_len, out_buf, out_buf_len); 
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);	
	
	return ret_code;
}



/**
* @brief SE��������ʵ��
* @param [in] in_buf	�������ݵ���ʼ��ַ
* @param [in] in_buf_len  �������ݳ���
* @param [out] out_buf	�������ʼ��ַ
* @param [out] out_buf_len  ������ݳ��ȵ���ʼ��ַ
* @return �μ�error.h
* @note 1.����洢�浵������˫����� 2.��api_transceive_queue ��������˫����и�ʽ���ͺͽ������� 3.�����˫�����ȡ����Ӧ����
* @see ���� util_queue_init util_queue_rear_push api_transceive_queue
*/
se_error_t api_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	uint32_t out_len = 0;
	uint16_t front_node = 0;

	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;

	do
	{
		//������
		if ((in_buf == NULL) || (out_buf == NULL)||(out_buf_len==NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		if ((in_buf_len<COMM_DATA_LEN_MIN)||(in_buf_len>COMM_DATA_LEN_MAX))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//˫�˶��г�ʼ��
		util_queue_init(&queue_in);
		util_queue_init(&queue_out);

		//�������ݴ���˫�˶���
		util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);

		//������˫������е����ݰ�Э���ʽ���͸��豸��������Ӧ���ݴ洢�����˫�����	
		ret_code = api_transceive_queue((uint8_t *)&queue_in, util_queue_size(&queue_in), (uint8_t *)&queue_out, &out_len);
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

		//��˫�˶��п������������
		front_node = queue_out.front_node;
		memcpy(out_buf,&queue_out.q_buf[front_node],queue_out.q_buf_len);
		*out_buf_len = queue_out.q_buf_len;

	}while(0);

	return ret_code;
}



/**
* @brief ��λ���輰��ȡ����ATR
* @param [out] out_buf	�������ʼ��ַ
* @param [out] out_buf_len  ������ݳ��ȵ���ʼ��ַ
* @return �μ�error.h
* @note 1.������������ѡ���������������δѡ������裬���ص�һ��ע����豸 2.���ݼ��ص���������������reset��λ����
* @see ���� getSelectedDriver getSelectedPeriph reset
*/
se_error_t api_reset (uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		if (((out_buf == NULL) && (out_buf_len != NULL))||((out_buf != NULL) && (out_buf_len == NULL)))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//1. ������鼰�����������
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //δִ��acl_register ����
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //ע���˶�����裬����ѡ������
				break;
			}
			//ֻע����һ������ʱ, ����ѡ��ֱ��ʹ�õ�1 ��ע�������
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//ִ����acl_select ������ѡ����ָ��������
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}


		//2.��λ����
		ret_code = driver->reset(periph, out_buf , out_buf_len); //��λ����,��ȡATR
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);

	return ret_code;
}


se_error_t api_ratr (uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		if (((out_buf == NULL) && (out_buf_len != NULL))||((out_buf != NULL) && (out_buf_len == NULL)))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//1. ������鼰�����������
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //δִ��acl_register ����
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //ע���˶�����裬����ѡ������
				break;
			}
			//ֻע����һ������ʱ, ����ѡ��ֱ��ʹ�õ�1 ��ע�������
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//ִ����acl_select ������ѡ����ָ��������
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}


		//2.��ȡATR
		ret_code = driver->ratr(periph, out_buf , out_buf_len); //��ȡATR
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);

	return ret_code;
}

/**
* @brief ����ese APDUͨ��
* @param [out] out_buf	�������ʼ��ַ
* @param [out] out_buf_len  ������ݳ��ȵ���ʼ��ַ
* @return �μ�error.h
* @note 1.������������ѡ���������������δѡ������裬���ص�һ��ע����豸 2.���ݼ��ص���������������endApdu����
* @see ���� getSelectedDriver getSelectedPeriph reset
*/
se_error_t api_endapdu(uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		if (((out_buf == NULL) && (out_buf_len != NULL))||((out_buf != NULL) && (out_buf_len == NULL)))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//1. ������鼰�����������
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //δִ��acl_register ����
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //ע���˶�����裬����ѡ������
				break;
			}
			//ֻע����һ������ʱ, ����ѡ��ֱ��ʹ�õ�1 ��ע�������
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//ִ����acl_select ������ѡ����ָ��������
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}


		//2.����ͨ��
		ret_code = driver->endapdu(periph, out_buf , out_buf_len); //����ͨ��
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);

	return ret_code;
}


/**
* @brief ΢���ʱ�ӿ�
* @param [in] us	��ʱ����ֵ����λΪ΢��
* @return �μ�error.h
* @see ���� getSelectedDriver getSelectedPeriph delay 
*/
se_error_t api_delay (uint32_t us)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		if (us == 0)
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//1. ������鼰�����������
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //δִ��acl_register ����
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //ע���˶�����裬����ѡ������
				break;
			}
			//ֻע����һ������ʱ, ����ѡ��ֱ��ʹ�õ�1 ��ע�������
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//ִ����acl_select ������ѡ����ָ��������
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}


		//2.��ʱָ����΢��
		ret_code = driver->delay(periph, us); //�ӳ�ָ��΢����
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);

	return ret_code;
}


/**
* @brief ���Ϳ���������ڵײ�Ӳ���Ʋ���
* @param [in] ctrlcode	���������
* @param [in] in_buf  �������ʼ��ַ
* @param [in] in_buf_len  �������ݳ���
* @param [out] out_buf	�������ʼ��ַ
* @param [out] out_buf_len  ������ݳ��ȵ���ʼ��ַ
* @return �μ�error.h
* @note 1. ������������ѡ����������� 2. ���ݼ��ص���������������control���ƺ���
* @see ���� getSelectedDriver getSelectedPeriph control
*/
se_error_t _api_control(uint32_t ctrlcode, uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t retCode = SE_SUCCESS;

	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
	{
		return SE_ERR_HANDLE_INVALID;  //δִ��acl_register ����
	}

	if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
	{
		//δִ��acl_select ����, ʹ�õ�1 ��ע����豸
		driver = g_drivers[0];
		periph = g_drivers[0]->periph[0];
	}
	else
	{
		//ִ����acl_select ������ѡ����ָ�����豸
		driver = getSelectedDriver();
		periph = getSelectedPeriph();
	}

	
	//��ѡ����豸���Ϳ�����������տ����������Ӧ����
	retCode = driver->control(periph, ctrlcode, in_buf, in_buf_len, out_buf, out_buf_len); 

	return retCode;
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

/**
  * @}
  */
