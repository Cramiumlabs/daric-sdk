/**@file  proto_spi_ese.c
* @brief  SPI �ӿ���·ͨ��Э�����������( ����SPI HED eSE ͨ��Э��淶ʵ��)
* @author  chenfeng
* @date  2023-07-20
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include "proto_spi_ese.h"
#include "error.h"
#include "log.h"
 
eSE_iBlockInfo_t  		g_eSE_sendiBlkInfo;
eSE_iBlockInfo_t  		g_eSE_recviBlkInfo;

uint8_t G_SPISendBuf[ESE_IFSD_MAX+BLOCK_MIN_SIZE];
uint8_t G_SPIRecvBuf[ESE_IFSD_MAX+BLOCK_MIN_SIZE];

/**************************************************************************
* Variable Declaration
***************************************************************************/

static eSEspi_param_t g_esespi_param[MAX_PERIPHERAL_DEVICE]={
    {ESE_IFSD_DEFAULT,ESE_IFSC_DEFAULT,ESE_IFSD_MAX,SEQNO_INIT,(eSE_sBlockTypes_t)0},
    {ESE_IFSD_DEFAULT,ESE_IFSC_DEFAULT,ESE_IFSD_MAX,SEQNO_INIT,(eSE_sBlockTypes_t)0},
	{ESE_IFSD_DEFAULT,ESE_IFSC_DEFAULT,ESE_IFSD_MAX,SEQNO_INIT,(eSE_sBlockTypes_t)0},
    {ESE_IFSD_DEFAULT,ESE_IFSC_DEFAULT,ESE_IFSD_MAX,SEQNO_INIT,(eSE_sBlockTypes_t)0}};

static peripheral_bus_driver g_proto_spi_ese= {
    PERIPHERAL_ESE,
   {NULL},
    proto_spi_ese_init,
    proto_spi_ese_deinit,
    proto_spi_ese_open,
    proto_spi_ese_close,
    proto_spi_ese_transceive,		//I-block
    proto_spi_ese_reset,			//inf reset
    proto_spi_ese_control,
    proto_spi_ese_delay,
    proto_spi_ese_ratr,				//ATR
    proto_spi_ese_endApdu			//EndApdu	
};


PERIPHERAL_BUS_DRIVER_REGISTER(PERIPHERAL_ESE, g_proto_spi_ese);


/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PROTO 
  * @brief link protocol layer.
  * @{
  */


/** @defgroup PROTO_SPI PROTO_SPI
  * @brief hed spi communication driver.
  * @{
  */



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function  -----------------------------------------------*/
/** @defgroup Proto_Spi_Private_Functions Proto_Spi Private Functions
 * @{
 */
// 

/**
* @brief ͨ��SPI�ӿڷ�������
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵ�оƬѡ���� chip_select
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵķ��ͺ���transmit
* @param [in] periph  �豸���
* @param [in] inbuf  ���������ݵ���ʼ��ַ
* @param [in] inbuf_len ���������ݵĳ���
* @return ��������״̬��	
* @note no
* @see no
*/
se_error_t proto_spi_ese_transmit(peripheral *periph, uint8_t *inbuf,uint32_t inbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;

	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER pSpiPeriph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	do
	{
		if(periph == NULL)
		{
			ret_code =	SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if ((inbuf == NULL) || (inbuf_len == 0U))
		{
			ret_code =	SE_ERR_PARAM_INVALID;
			break;
		}
		pSpiPeriph->delay(SPI_ESE_SEND_BGT_TIME); 	//��ʱBGTESE_SPI_SEND_BGT_TIME	
		
		pSpiPeriph->chip_select(pSpiPeriph,TRUE);			
		pSpiPeriph->delay(SPI_ESE_SEND_CS_WAKEUP_TSS_TIME);
		ret_code = pSpiPeriph->transmit(pSpiPeriph,inbuf,inbuf_len);	
		pSpiPeriph->chip_select(pSpiPeriph,FALSE);
		
	}while(0);

	return ret_code;
}



/**
* @brief ͨ��SPI�ӿڽ�������
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵ�оƬѡ���� chip_select
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵĽ��պ���receive
* @param [in] periph  �豸���
* @param [out] outbuf  �������ݵ���ʼ��ַ
* @param [out] outbuf_len �������ݵĳ���
* @return ��������״̬��	
*/
se_error_t proto_spi_ese_receive(peripheral *periph, uint8_t *outbuf, uint32_t outbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER pSpiPeriph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	do
	{
		if(periph == NULL)
		{
			ret_code =	SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if ((outbuf == NULL) || (outbuf_len == 0U))
		{
			ret_code =	SE_ERR_PARAM_INVALID;
			break;
		}
		
		memset(outbuf,0xff,outbuf_len);
		pSpiPeriph->chip_select(pSpiPeriph,TRUE);
		ret_code = pSpiPeriph->receive(pSpiPeriph,outbuf,&outbuf_len);	
		pSpiPeriph->chip_select(pSpiPeriph,FALSE);
		
	}while(0);

	return ret_code;
}


/*************************************************
  Function:	  proto_spi_ese_LRC
  Description:  ����ָ������ָ�����ݵ�LRC���
  Input:	
            Buf��Ҫ��������ݵ���ʼ��ַ
            len��Ҫ��������ݳ���			
  Return:	lrc������	
  Others:		
*************************************************/
uint8_t proto_spi_ese_LRC(uint8_t* Buf,uint32_t len)
{
	uint8_t* infoPtr, checkValue;
	uint32_t i;
	
	infoPtr = Buf;
	checkValue = *infoPtr++;
	
	for(i = 1; i!=len; i++)
	{
		checkValue ^= *infoPtr++;
	}
	
	return (checkValue);
}

void eSESPI_ResetProParas(uint8_t dev_id) 
{	
	g_esespi_param[dev_id].g_sIfsd = ESE_IFSD_DEFAULT;
	g_esespi_param[dev_id].g_sIfsc = ESE_IFSC_DEFAULT;
	g_esespi_param[dev_id].g_sMaxIfsc = ESE_IFSD_MAX;
	g_esespi_param[dev_id].g_bRetryNum = SEQNO_INIT;	

	memset(&g_eSE_sendiBlkInfo,0,sizeof(eSE_iBlockInfo_t));
	memset(&g_eSE_recviBlkInfo,0,sizeof(eSE_iBlockInfo_t));				
}

/**
* @brief ����HED SPI ESEͨ��Э���Block��ʽ������BLOCK����
* @param [in] periph  �豸���
* @param [in] param  ͨ�Ų�����Ϣ
* @param [in] inbuf  ���������ݵ���ʼ��ַ
* @param [in] inbuf_len ���������ݵĳ���
* @return ��������״̬��	
* @note Э��ʱ�����WPT~T3 �����SE оƬҪ��
* @see ����  proto_spi_ese_transmit
*/
se_error_t proto_spi_ese_send_frame(peripheral *periph, eSEspi_param_t *param, uint8_t *inbuf, uint32_t inbuf_len)
{
	se_error_t ret_code = SE_SUCCESS;
//	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_ese_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	do
	{
		if(periph == NULL)
		{
			ret_code =	SE_ERR_HANDLE_INVALID;
			break;
		}
		
		if ((param == NULL) || (inbuf == NULL) ||(inbuf_len < BLOCK_MIN_SIZE))
		{
			ret_code =	SE_ERR_PARAM_INVALID;
			break;
		}
		
		//eseЭ��Ĭ�� = 0��û�л����ֽ� ͨ�� SS�ź�delayʱ��ΪG_SPIT0��ʵ��		
//		memset(bData, 0x00, (WAKEUP_DATA_LEN+1));  //debug
//		ret_code = proto_spi_transmit(periph,bData, (WAKEUP_DATA_LEN+1)); //send wakeup data
//		if(ret_code !=SE_SUCCESS)
//		{
//			break;
//		}
//		
//		p_spi_ese_periph->delay(SPI_SEND_CS_WAKEUP_TIME);  //delay WPTret_code = p_spi_periph->gpio_irqwait(gpio_fd, SPI_RECEVIE_FRAME_WAIT_GPIO_IRQ_TIME);
				
		ret_code = proto_spi_ese_transmit(periph,inbuf, inbuf_len);	   //send All Block
		if(ret_code !=SE_SUCCESS)
		{
			break;
		}
		
	}while(0);

	return ret_code;
}

/******************************************************************************
 * Function         proto_spi_ese_sendRblock
 *
 * Description      This internal function is called to send R-Block with all
 *                   updated 7816-3 headers
 *
 * Returns          On success return true or else false.
 *
 ******************************************************************************/
se_error_t proto_spi_ese_sendRblock(peripheral *periph, eSEspi_param_t *param, rBlockErrorTypes_t rBlockErrorTypes) 
{
	se_error_t ret_code = SE_SUCCESS;
	uint8_t bLrc = 0;
	uint8_t abBlockBuf[BLOCK_MIN_SIZE] = {0};  	//��ʱ���飬û�и���ȫ��I-buff
	uint8_t bRecvSeqNum=(g_bRecvSeqNum?0:1);
	
	//NAK���ܷ�ת����Ҫ���½���֮ǰ�յ���Ns��ACK��ת��Ԥ�ڽ�����һ��Ns
	uint8_t bNr=(rBlockErrorTypes?g_bRecvSeqNum:bRecvSeqNum);   

	if(periph == NULL)
	{
		return	SE_ERR_HANDLE_INVALID;
	}

	if (param == NULL)
	{
		return	SE_ERR_PARAM_INVALID;
	}
	
	//1.r-PROLOGUE
	abBlockBuf[ESE_NAD_OFFS] = ESE_BLOCK_NAD;
	abBlockBuf[ESE_PCB_OFFS] = SET_BLOCK_TYPE(ESE_R_BLOCK)|(bNr<<4)|rBlockErrorTypes;
	abBlockBuf[ESE_LEN_OFFS] = 0x00;

	//2.R-EPILOGUE
	bLrc = proto_spi_ese_LRC(abBlockBuf+ESE_PCB_OFFS, (PROLOGUE_NOMAL_SIZE-NAD_SIZE));
	abBlockBuf[PROLOGUE_NOMAL_SIZE] = bLrc;
	
	//3.Send R-block
	ret_code = proto_spi_ese_send_frame(periph, param, abBlockBuf,BLOCK_MIN_SIZE);
	if(ret_code!= SE_SUCCESS)
	{
	
		ret_code = SE_ERR_COMM;   	 
	}
 
	
	return ret_code;	
}


/******************************************************************************
 * Function         proto_spi_ese_sendSblock
 *
 * Description      This internal function is called to send S-Block with all
 *                   updated 7816-3 headers
 *
 * Returns          On success return true or else false.
 *
 ******************************************************************************/
se_error_t proto_spi_ese_sendSblock(peripheral *periph, eSEspi_param_t *param,eSE_sBlockInfo_t sBlockInfo)
{
	se_error_t ret_code = SE_SUCCESS;
	uint16_t sOff = 0;
	uint16_t sInfLen = 0,sIfsd=0;
	uint8_t bLrc = 0,dev_id;	
	uint8_t abBlockBuf[8] = {0}; //��ʱ���飬ĿǰReq INF lenֻ��1��2; 6+2
	
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;
	
	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (param == NULL )
	{
		return SE_ERR_PARAM_INVALID;
	}

	dev_id = p_spi_periph->periph.id;
	sIfsd = g_esespi_param[dev_id].g_sIfsd;
	
	//��Ҫ���� sBlockInfo.p_data ��len  �޸� 
	switch(sBlockInfo.sBlockType)
	{
		case ESE_IFS_REQ:
		{
			if(ESE_IFSD_DEFAULT < sIfsd)
			{
				sInfLen = LEN_EXT_LEN_SIZE;
			}
			else
			{
				sInfLen = LEN_NOMAL_SIZE;
			}

		}break;
		
		case ESE_WTX_RSP:
		{
			if(sBlockInfo.len ==COMM_WTX_LEN)
			{
				sInfLen=COMM_WTX_LEN;
				
				sIfsd = sBlockInfo.p_data[0];  //ʹ�ý��յ�������
			}
			else
			{
				ret_code = SE_ERR_SOF;
				return ret_code;
			}
		}break;
			
		case ESE_ABORT_REQ:  			//SDKҲ��֧��
		case ESE_RESYNCH_REQ:				
		case ESE_INTF_RESET_REQ:
		case ESE_PROP_END_APDU_REQ:		//Ŀǰֻ���� NO-INF
		case ESE_HARD_RESET_REQ:		//Ŀǰֻ���� NO-INF	
		case ESE_ATR_REQ:
			break;
		default:
			ret_code = SE_ERR_SOF;
			return ret_code;
	}
	
	//1.s-PROLOGUE
	abBlockBuf[ESE_NAD_OFFS] = ESE_BLOCK_NAD;
	abBlockBuf[ESE_PCB_OFFS] = sBlockInfo.sBlockType;
	abBlockBuf[ESE_LEN_OFFS] = sInfLen;

	sOff += PROLOGUE_NOMAL_SIZE;

	//2.s-INFO  ĿǰReq INF lenֻ��1��2
	if(sInfLen == 1)
	{
		abBlockBuf[sOff++] = (uint8_t)sIfsd&LEN_EXT_TAG;
	}
	else if(sInfLen == 2)
	{
		abBlockBuf[sOff++] = (uint8_t)((sIfsd>>8)&LEN_EXT_TAG);
		abBlockBuf[sOff++] = (uint8_t)(sIfsd&LEN_EXT_TAG);
	}	

	//3.S-EPILOGUE
	bLrc = proto_spi_ese_LRC(abBlockBuf+ESE_PCB_OFFS, (sOff-NAD_SIZE));
	abBlockBuf[sOff] = bLrc;
	sOff += G_EPILOGUE_SIZE;
	
	//4.Send S-block
	ret_code = proto_spi_ese_send_frame(periph, param, abBlockBuf,sOff);
	if(ret_code!= SE_SUCCESS)
	{
	
		ret_code = SE_ERR_COMM;   	 
	}
 
	
	return ret_code;	
}
/**
* @brief ����HED SPI ESEͨ��Э���i-Block��ʽ������i-BLOCK����
* @param [in] periph  �豸���
* @param [in] param  ͨ�Ų�����Ϣ
* @param [in] eSE_iBlockInfo_t ������I���ݵĽṹ
* @return ��������״̬��	
* @note Э��ʱ�����WPT~T3 �����SE оƬҪ��
* @see ����  proto_spi_ese_transmit
*/
se_error_t proto_spi_ese_sendIblock(peripheral *periph, eSEspi_param_t *param, eSE_iBlockInfo_t iBlockInfo) 
{
	se_error_t ret_code = SE_SUCCESS;
	uint16_t sOff = 0;	
	uint8_t bLrc = 0;	
	
	//���ڲ���������buff ʹ��G_SPISendBufȫ������
	uint8_t *abBlockBuf = (uint8_t*)G_SPISendBuf;    //���޸�Block ���ȵ�ȫ�ֱ���or tmp  ʹ��˫����

	//1.I-PROLOGUE
	abBlockBuf[ESE_NAD_OFFS] = ESE_BLOCK_NAD;
	abBlockBuf[ESE_PCB_OFFS] = ((iBlockInfo.seqNo<<PCB_NS_BIT) |(iBlockInfo.isChained<<PCB_M_BIT));
	sOff = ESE_LEN_OFFS;
	
	if( iBlockInfo.crtDataLen > ESE_IFSD_DEFAULT)  
	{ 	/* Case for frame size > 254 bytes  Exterten Len Format */
		abBlockBuf[sOff++] = LEN_EXT_TAG;

		setShort(abBlockBuf+sOff, iBlockInfo.crtDataLen);
		sOff += LEN_EXT_LEN_SIZE;
	} 
	else 
	{ 	/* Case for frame size < 254 bytes */
		/* store I frame length */
		abBlockBuf[sOff++] = iBlockInfo.crtDataLen;
	}

	//2.I-INFO
	memcpy(abBlockBuf+sOff, iBlockInfo.p_data+iBlockInfo.dataOffset, iBlockInfo.crtDataLen);
	sOff += iBlockInfo.crtDataLen;

	//3.I-EPILOGUE
	bLrc = proto_spi_ese_LRC(abBlockBuf+ESE_PCB_OFFS, (sOff-NAD_SIZE));
	abBlockBuf[sOff] = bLrc;
	sOff += G_EPILOGUE_SIZE;
	
	//4.Send block
	ret_code = proto_spi_ese_send_frame(periph, param, abBlockBuf,sOff);
	if(ret_code!= SE_SUCCESS)
	{
	
		ret_code = SE_ERR_COMM;   	 
	}
	
 	return ret_code;	
}

static se_error_t eSESPI_ProcessSblock(uint8_t dev_id,eSE_sBlockInfo_t sBlockInfo)
{
	se_error_t ret_code = SE_SUCCESS;
	uint16_t sIfsTmp = 0;
	
	switch(sBlockInfo.sBlockType)
	{
		case ESE_WTX_REQ:
			//G_SPITBWT = G_ESESPI_TBWT_DEFAULT * sBlockInfo.p_data[0];   //SDK������ ���ﲻ��ֵ
		break;
		case ESE_IFS_RSP:
		{		
			if (sBlockInfo.len) //inf len
			{
				if (LEN_NOMAL_SIZE == sBlockInfo.len)
				{
					sIfsTmp = sBlockInfo.p_data[0];
				}
				else if (LEN_EXT_LEN_SIZE == sBlockInfo.len)
				{
					sIfsTmp = getShort(sBlockInfo.p_data);
				}
				
				if ( sIfsTmp == g_esespi_param[dev_id].g_sIfsd) //SE����ok
				{
					g_esespi_param[dev_id].g_sIfsc = g_esespi_param[dev_id].g_sIfsd = sIfsTmp; //ͬʱ���� ��ΪĿǰ��֧��ESE_IFS_REQ
				}			
			}		
			else
			{
				ret_code = SE_ERR_SOF;
			}
		}break;
		case ESE_HARD_RESET_RSP:	
			eSESPI_ResetProParas(dev_id);	
			break;
		case ESE_INTF_RESET_RSP:	
			eSESPI_ResetProParas(dev_id);
			break;	
		case ESE_ATR_RSP:	
		{		
			if (COMM_ATR_LEN ==sBlockInfo.len)
			{
//				G_SPICheckLen = ((0==sBlockInfo.p_data[COMM_EDC_OFF])? 1 : 2) ;  //�̶�LRC
				g_esespi_param[dev_id].g_sIfsc = g_esespi_param[dev_id].g_sIfsd = sBlockInfo.p_data[COMM_DIFSC_OFF];   //default
				g_esespi_param[dev_id].g_sMaxIfsc = getShort(sBlockInfo.p_data+COMM_MIFSC_OFF);
				
				//�������������Ƿ����
//				VENDORID				5	��ʾ��ǰSlave�豸OS�汾��Ŀǰȫ0
//				�ӿ��ֽ�				1	���淶�̶�1����ʾT=1
//				�鱣��ʱ�䣨ms��		2	���淶��ΪBGTʱ��
//				��ʱ�ȴ�ʱ�䣨ms��		2	���淶��ΪFWTʱ��
//				�ӿ����Ƶ��MCF��Hz��	2	���淶�����壬ʵ�ʸ���Ӳ���ӿ�����
//				��У������				1	���淶�̶�0��ֻ֧��LRC  
//				DIFSC					1	���淶Ĭ��0xFE
//				�߼�ͨ������			1	���淶Ĭ��1
//				MIFSC					2	���淶Ĭ��02FD
//				����					2	���淶Ĭ��0C��00001100��
//							Bit1: Slave�Ƿ�֧��������Ч�źŹ���
//							Bit2: Slave�Ƿ�֧��֧��������Ч�źŵ�ƽ 1:SE����Ӧ֮ǰ��������
//							Bit3:Slave�Ƿ�֧��S(HARD_RESET_REQ)
//							Bit4:�鳤����չ��ʽ�Ƿ�֧��
//							Bit5:�Ƿ�֧���߼�ͨ����������1��
//							Bit6-16:����	

			}
			else if (!sBlockInfo.len) //SE ��֧��
			{
				g_esespi_param[dev_id].g_sMaxIfsc = ESE_IFSC_DEFAULT;
			}				
		}break;		
		case ESE_RESYNCH_RSP:
		{
			g_bRecvSeqNum=SEQNO_INIT;	
			g_bSendSeqNum=SEQNO_INIT;	
		}break;
		
		case ESE_ABORT_RSP:
			break;
		default:
			break;
	}
	
	return ret_code;
}


/**
* @brief ����HED SPI ese ͨ��Э��Ľ��� blockΪ��λ����ʱ��֡�ȴ�ʱ�������������ݳ�ʱ�ļ�ʱ����
* @param [in] periph  �豸��� 
* @param [in] param  ͨ�Ų�����Ϣ
* @param [out] outbuf  �����ռ���֡����ʼ��ַ
* @param [out] outbuf_len �����ռ���֡�ĳ���
* @return ��������״̬��	
* @note Э��ʱ�����T4 �����SE оƬҪ��
* @see ����  proto_spi_ese_receive_block  eSESPI_RecvCheck_Block
*/
se_error_t proto_spi_ese_receive_block(peripheral *periph, eSEspi_param_t *param, uint8_t *output, uint32_t *output_len)
{
	se_error_t retCode = SE_SUCCESS;
	util_timer_t timer={0};
	uint16_t edc_value ;
//	uint32_t len = 0;

	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;
	uint16_t sRecvLen = 0;
	uint16_t sInfLen = 0;	
	uint8_t bBlockType = 0;		
	
	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((param == NULL) || (output == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}
	
	//���ý�������ʱ֡�ȴ��ĳ�ʱʱ��
	timer.interval = SPI_ESE_RECEVIE_FRAME_WAIT_TIME;  //SPITBWT
	p_spi_periph->timer_start(&timer);

	do
	{
		//receive frame head	
		p_spi_periph->delay(SPI_ESE_SEND_DATA_OVER_WAIT_TIME);   //delay T3
		do
		{
			if(p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
			{
				LOGE("Failed:receive frame overtime,  ErrCode-%d.", SE_ERR_TIMEOUT);
				
				return SE_ERR_TIMEOUT;	
			}		
			
			p_spi_periph->delay(SPI_ESE_RESET_POLL_SLAVE_INTERVAL_TIME);   //delay T4
			//1. receive first fragment of data  LEN_POLLING
			retCode = proto_spi_ese_receive(periph, output, LEN_POLLING); //recieve NAD PCB
			if(retCode != SE_SUCCESS)
				return retCode;

			if(output[ESE_NAD_OFFS] != ESE_REV_SE_NAD)
			{
				continue;
			}
			
			//��ʾ���յ�NAD+PCB
			sRecvLen += LEN_POLLING;
			
			p_spi_periph->delay(SPI_ESE_RESET_POLL_SLAVE_INTERVAL_TIME);   //delay T4
			//2. receive len fragment of data   		
			retCode = proto_spi_ese_receive(periph, output+sRecvLen, LEN_NOMAL_SIZE); 
			if (retCode)
			{
				return retCode;
			}	
			
			//��ʾ���յ�LEN
			sRecvLen += LEN_NOMAL_SIZE;	
			sInfLen = output[ESE_LEN_OFFS]; 
			
			//3-1. �ж��Ƿ�����չ�鳤�ȸ�ʽ	
			if(sInfLen == LEN_EXT_TAG) 
			{
				p_spi_periph->delay(SPI_ESE_RESET_POLL_SLAVE_INTERVAL_TIME);   //delay T4		
				//3-2. receive ��չ�鳤��
				retCode = proto_spi_ese_receive(periph, output+sRecvLen, LEN_EXT_LEN_SIZE); 
				if (retCode)
				{
					return retCode;
				}
				
				//3-3.��ʾ���յ���չlen����
				sInfLen = getShort(output+sRecvLen); 			
				sRecvLen += LEN_EXT_LEN_SIZE;						
			}
			
			//4. �ж�INF Len�Ϸ���
			if(sInfLen > ESE_IFSD_MAX) //1 block
			{
				retCode= SE_ERR_LEN;
				return retCode;			
			}
			
			//5. ����EDC�����ݳ���
			sInfLen += G_EPILOGUE_SIZE;   //sInfLen����������Ҫ����EDC���� ������һ�����
	
			p_spi_periph->delay(SPI_ESE_BEFORE_RECEIVE_DATA_WAIT_TIME); //delay T5			
			retCode = proto_spi_ese_receive(periph, output+sRecvLen, sInfLen); 
			if (retCode)
			{
				return retCode;
			}								
			sRecvLen += sInfLen;	
			break;
		}while(1);

		//4.�������ݸ�ʽ��ȷ��
		sInfLen = getLEN(output);	//��ȡ����
		bBlockType = BLOCK_MASK(G_ESE_PCB(output));	
		if(bBlockType==ESE_R_BLOCK)
		{
			//����ΪR ��ʱ��������ݸ�ʽ
			if( (G_ESE_PCB(output)&0x6C)!=0 )  
			{
				retCode = SE_ERR_DATA;
				break;
			}
			if(sInfLen!=0)
			{
				retCode = SE_ERR_DATA;
				break;
			}
		}
		else if(bBlockType==ESE_S_BLOCK)
		{
			//����ΪS ��ʱ��������ݸ�ʽ
			if((G_ESE_PCB(output)&0x18) != 0) 
			{
				retCode = SE_ERR_DATA;
				break;
			}
		}
		else
		{
			//����ΪI ��ʱ��������ݸ�ʽ
			if((G_ESE_PCB(output)&0x1F)!=0)
			{
				retCode = SE_ERR_DATA;
				break;
			}
		}
		
		//���edc
		edc_value =  proto_spi_ese_LRC(output+ESE_PCB_OFFS,sRecvLen-G_EPILOGUE_SIZE-NAD_SIZE);
		if((output[sRecvLen-G_EPILOGUE_SIZE] != edc_value))
		{
			return SE_ERR_LRC_CRC;		
		} 
		
		*output_len = sRecvLen;
		break;

	}while(1);

	return retCode;
}

/******************************************************************************
 * Function         proto_spi_ese_receive_Block_Handler
 *
 * Description      This internal function is called to process recive Block with all
 *                   updated 7816-3 headers and check 
 *
 * Returns          On success return true or else false.
 *
 ******************************************************************************/
se_error_t proto_spi_ese_receive_Block_Handler(peripheral *periph, eSEspi_param_t *param, uint8_t *output, uint32_t *output_len)
{
	se_error_t retCode = SE_SUCCESS;
//	util_timer_t timer={0};
//	uint32_t len = 0;
	uint8_t bBlockType = 0,dev_id;	
	
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((param == NULL) || (output == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}
	dev_id = p_spi_periph->periph.id;
	do
	{
		//1.���տ�����,�������ݸ�ʽ�Ƿ���ȷ,���Block����
		retCode = proto_spi_ese_receive_block(periph,param,output, output_len);
		if(retCode)
		{
			if(g_esespi_param[dev_id].g_bRetryNum >= PROTO_SPI_ESE_RETRY_NUM)
		{
			retCode = SE_ERR_COMM; 
			g_esespi_param[dev_id].g_bRetryNum =0;
			
			return retCode;
		}		
			if(retCode == SE_ERR_TIMEOUT)
			{
				return retCode;
			}
			
			//send R block to inform error, then turn to receive
			if(retCode == SE_ERR_LRC_CRC)
			{
				proto_spi_ese_sendRblock(periph,param,PARITY_ERROR);  
			}
			else if (retCode == SE_ERR_DATA)
			{
				proto_spi_ese_sendRblock(periph,param,SOF_MISSED_ERROR);  
			}
			else
			{
				proto_spi_ese_sendRblock(periph,param,OTHER_ERROR);
			}

			g_esespi_param[dev_id].g_bRetryNum++;
			continue;
		}
		
		//2.����Ƿ�ΪS �飬���ǣ�������Ӧ����
		bBlockType = BLOCK_MASK(G_ESE_PCB(output));				 //ʹ��ȫ�ֱ���
		if(bBlockType==ESE_S_BLOCK)
		{	
			eSE_sBlockInfo_t sBlockInfoTmp;
			//when receive S request,send S response, then turn to receive
			//when receive S response,return back to up level to handle
			//when receive  nonsense Sblock,send  R block to inform error, then turn to receive
			sBlockInfoTmp.p_data = output+G_PROLOGUE_SIZE(output);  //ָ��INF
			sBlockInfoTmp.len = (*output_len) - G_BLOCK_PRO_AND_EPI_SIZE(output); //inf ����

			if(G_ESE_PCB(output)==ESE_WTX_REQ)
			{
				sBlockInfoTmp.sBlockType = ESE_WTX_RSP;
				proto_spi_ese_sendSblock(periph,param,sBlockInfoTmp);
				
				sBlockInfoTmp.sBlockType = ESE_WTX_REQ;
				eSESPI_ProcessSblock(dev_id,sBlockInfoTmp);
				
				//���½�����������¼�ʱ
				continue;
			}
			else if ( (G_ESE_PCB(output)==ESE_IFS_RSP) || (G_ESE_PCB(output)==ESE_INTF_RESET_RSP) 
				||(G_ESE_PCB(output)==ESE_HARD_RESET_RSP) || (G_ESE_PCB(output)==ESE_ATR_RSP)
				|| (G_ESE_PCB(output)==ESE_PROP_END_APDU_RSP) || (G_ESE_PCB(output)==ESE_RESYNCH_RSP))
			{
				//��ȥ�������Ҫ����S-BLOCK�ٴ���				
				break;
			}
			else if ( G_ESE_PCB(output)==ESE_ABORT_RSP ) // Chain transmitter initiates chain abortion
			{    		
				retCode = SE_ERR_ESE_ABORT;	//Scenario 28
				break;  
			}
			else if ( G_ESE_PCB(output)==ESE_ABORT_REQ ) // Chain receiver initiates chain abortion 
			{
				retCode = SE_ERR_ESE_ABORT;       //Scenario 27
				break;  
			}		
			else				
			{
				if(g_esespi_param[dev_id].g_bRetryNum >= PROTO_SPI_ESE_RETRY_NUM)
				{
					retCode = SE_ERR_COMM; 
					g_esespi_param[dev_id].g_bRetryNum =0;
					
					return retCode;
				}		
				proto_spi_ese_sendRblock(periph,param,OTHER_ERROR);

				g_esespi_param[dev_id].g_bRetryNum++;				
				continue;
			}
		}
		else
		{
			//when receive I or R,return back to up level to handle
			break;
		}
	}while(1);
	
	return retCode;
}

//use periph type instead of sBlockInfo.sBlockType
static se_error_t eSESPI_RecvSBlock(peripheral *periph, eSEspi_param_t *param, uint8_t *output, uint32_t *output_len)
{
	uint8_t abTmp[COMM_ATR_LEN+BLOCK_MIN_SIZE];  //������ʱbuff
	uint32_t sTmpLen = 0;
	se_error_t retCode = SE_SUCCESS;
	eSE_sBlockInfo_t sBlockInfo;
	
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((param == NULL) || (output == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}
	
	sBlockInfo.sBlockType = param->type;
	do
	{
		//1.���� ������,�������ݸ�ʽ�Ƿ���ȷ
		retCode=proto_spi_ese_receive_Block_Handler(periph,param,abTmp,&sTmpLen);
		if(retCode != SE_SUCCESS && (retCode!=SE_ERR_ESE_ABORT))
		{
			break;
		}
		//2.����S-BLOCK����
		if(G_ESE_PCB(abTmp) != (sBlockInfo.sBlockType | ESE_S_BLOCK_RSP) )//��ָ����S-BLOCK RSP
		{
			retCode=SE_ERR_RETRY;
			break;
		}
		else
		{
			sBlockInfo.p_data = abTmp+G_PROLOGUE_SIZE(abTmp);
			sBlockInfo.len = sTmpLen-G_BLOCK_PRO_AND_EPI_SIZE(abTmp);	
			sBlockInfo.sBlockType = (eSE_sBlockTypes_t)G_ESE_PCB(abTmp); //����Ϊ����PCB
			retCode=eSESPI_ProcessSblock(p_spi_periph->periph.id,sBlockInfo);  //���ﴦ��S-Block����֧�ֵ�	
			
			if(retCode==SE_ERR_ESE_ABORT)
			{
				retCode = SE_SUCCESS; //����
			}
			
			if(retCode == SE_SUCCESS)
			{
				output = sBlockInfo.p_data;
				*output_len = sBlockInfo.len;
			}
		}

	}while(0);  

	return retCode;
}



/******************************************************************************
 * Function         eSESPI_RecvRBlock
 *
 * Description      This internal function is called to reveive R-Block with all
 *                  updated 7816-3 headers
 *
 * Returns          On success return true or else false.
 *
 ******************************************************************************/
static se_error_t eSESPI_RecvRBlock(peripheral *periph, eSEspi_param_t *param,uint16_t seq_num )
{
	uint8_t bBlockType = 0;
	uint8_t bNr = 0; //,dev_id;		
	uint8_t abTmp[COMM_ATR_LEN+BLOCK_MIN_SIZE];  //R-block û��INFO����
	uint32_t sTmpLen = 0;
	se_error_t retCode = SE_SUCCESS;
	
//	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (param == NULL)
	{
		return SE_ERR_PARAM_INVALID;
	}
//	dev_id = p_spi_periph->periph.id;	
	do
	{
		//1.���� ������,�������ݸ�ʽ�Ƿ���ȷ
		retCode=proto_spi_ese_receive_Block_Handler(periph,param,abTmp,&sTmpLen);
		if(retCode != SE_SUCCESS && (retCode!=SE_ERR_ESE_ABORT))
		{
			break;
		} 
			
		bBlockType = BLOCK_MASK(G_ESE_PCB(abTmp));				 //ʹ��ȫ�ֱ���		
		if(bBlockType != ESE_R_BLOCK)
		{
			proto_spi_ese_sendRblock(periph,param,OTHER_ERROR);
			continue;
		}
		else
		{
			bNr = (G_ESE_PCB(abTmp)&PCB_BIT_NR) ? 1:0;  
			if( (bNr!=seq_num) || (G_ESE_PCB(abTmp)&0x0F))  //�յ�NAKҲ�ط�
			{
				retCode=SE_ERR_RETRY;
				break;
			}
			else
			{
				break;
			}
		}
	}while(1); //end while(1)
	
	return retCode;
}


/******************************************************************************
 * Function         eSESPI_RecvIBolck
 *
 * Description      This internal function is called to reveive I-Block with all
 *                  updated 7816-3 headers
 *
 * Returns          On success return true or else false.
 *
 ******************************************************************************/
static se_error_t eSESPI_RecvIBolck(peripheral *periph, eSEspi_param_t *param, uint8_t *outbuf , uint32_t *outbuf_len)
{
	uint8_t bBlockType = 0;
	uint8_t bNrs = 0,dev_id;
	se_error_t retCode = SE_SUCCESS;

	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((param == NULL) || (outbuf == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}
	dev_id = p_spi_periph->periph.id;	 	
	do
	{
		//1.���� ������,�������ݸ�ʽ�Ƿ���ȷ  	
		retCode=proto_spi_ese_receive_Block_Handler(periph,param,outbuf,outbuf_len);
		if(retCode != SE_SUCCESS)
		{
			break;
		}
		if(g_esespi_param[dev_id].g_bRetryNum >=PROTO_SPI_ESE_RETRY_NUM)  
		{
			retCode = SE_ERR_COMM;  
			g_esespi_param[dev_id].g_bRetryNum  =0;
			break;
		}

		bBlockType = BLOCK_MASK(G_ESE_PCB(outbuf));				
		if(bBlockType == ESE_S_BLOCK)
		{
			//received not likely S block
			proto_spi_ese_sendRblock(periph,param,OTHER_ERROR);
			g_esespi_param[dev_id].g_bRetryNum ++;
			continue;
		}
		else if(bBlockType == ESE_R_BLOCK)
		{	
			//received R block
			bNrs = (G_ESE_PCB(outbuf)&PCB_BIT_NR) ? 1:0;  
			if(bNrs==g_bSendSeqNum)
			{
				proto_spi_ese_sendRblock(periph,param,OTHER_ERROR);
				g_esespi_param[dev_id].g_bRetryNum ++;
				continue;
			}
			else
			{
				retCode=SE_ERR_RETRY;
				break;
			}	
		}
		else
		{
			//received I block  
			bNrs = (G_ESE_PCB(outbuf)&PCB_BIT_NS) ? 1:0;  
			if(bNrs!=g_bRecvSeqNum)
			{
				//received overlap I block which is not needed
				proto_spi_ese_sendRblock(periph,param,OTHER_ERROR);
				g_esespi_param[dev_id].g_bRetryNum ++;
				continue;
			}

			if(G_ESE_PCB(outbuf)&PCB_BIT_M)
			{
				//received a chaining I block,chain not finished
				retCode=ESE_TRANS_MOREDATA;
				break;
			}
			else
			{
				//received the tail I block of chain,chain finished
				retCode=SE_SUCCESS;
				break;
			}
		}
	}while(1); //end while(1)

	
	return retCode;	
}

/**
* @brief ����S-block������Block ��Ӧ
* @param [in] periph  �豸���
* @param [in] param  ͨ�Ų�����Ϣ������s-block����
* @param [out] rbuf  ������INFO��������ʼ��ַ
* @param [out] rlen ������INFO�ĳ���
* @return ��������״̬��	
* @see ����  proto_spi_ese_exchg_sblock   
*/
se_error_t proto_spi_ese_exchg_sblock(peripheral *periph, eSEspi_param_t *param, uint8_t *rbuf, uint32_t *rlen)
{ 
	se_error_t ret_code = SE_SUCCESS;
	uint32_t bufsize = 0; 
//	uint16_t spi_edc = 0;
//	uint16_t rec_nak_count = 0;
	uint16_t rec_time_out_count = 0;
	eSE_sBlockInfo_t sBlockInfo;
	
	if(periph == NULL)
	{
		ret_code =	SE_ERR_HANDLE_INVALID;
		return ret_code;
	}

	if (param == NULL)
	{
		ret_code =	SE_ERR_PARAM_INVALID;
		return ret_code;
	}

	do
	{
		//������Ӧ��S-Block  
		sBlockInfo.sBlockType = param->type;
		ret_code = proto_spi_ese_sendSblock(periph, param, sBlockInfo);
		if(ret_code!= SE_SUCCESS)
		{
			break;
		}

		//2 RecvS   inf ����
		ret_code=eSESPI_RecvSBlock(periph, param, rbuf, &bufsize);
		if(ret_code!= SE_SUCCESS)
		{
   			if(ret_code == SE_ERR_TIMEOUT)//��ʱ�ط�һ��
			{   
				rec_time_out_count++;
				if(rec_time_out_count < PROTO_SPI_ESE_RETRY_NUM)
				{
					continue;
				}
				break;   
			}
			
			//��������
			return ret_code;
		}
		
		*rlen = bufsize;		
		break;

	}while(1);

	return ret_code;
}


/******************************************************************************
 * Function         proto_spi_ese_send_data
 *
 * Description      This function is called to send AppData with all
 *                  updated 7816-3 headers
 *
 * Returns          On success return true or else false.
 *
 ******************************************************************************/
se_error_t proto_spi_ese_send_data(peripheral *periph, eSEspi_param_t *param, uint8_t *sbuf, uint32_t slen)
{
	se_error_t ret_code = SE_SUCCESS;
	uint16_t sTotalLenSent=0;
	uint16_t sRemainLen=0;	
	uint16_t re_tran_count = 0;	
	uint8_t dev_id;	
	
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;	
	if(periph == NULL)
	{
		ret_code =	SE_ERR_HANDLE_INVALID;
		return ret_code;
	}

	if ((param == NULL) || (sbuf == NULL) || (slen == 0))
	{
		ret_code =	SE_ERR_PARAM_INVALID;
		return ret_code;
	}	
	dev_id = p_spi_periph->periph.id;		
	if(slen < MAXBUFLEN)   //��ֵ�����ٵ��� ���� ĿǰͬG_InterfaceSendBuf[MAXBUFLEN+6] ���� ��������PHY_ESE_MAX_SIZE
	{
		g_eSE_sendiBlkInfo.p_data =sbuf;
		g_eSE_sendiBlkInfo.totalDataLen = slen;
	}
	else
	{
		/* BUFF SIZE  ��Ҫ�޸Ĵ� */
		return SE_ERR_LENGTH;
	}	
	
	do
	{
		sRemainLen = g_eSE_sendiBlkInfo.totalDataLen - sTotalLenSent; 
		g_eSE_sendiBlkInfo.isChained =(sRemainLen>g_esespi_param[dev_id].g_sIfsc)?ESE_BLOCK_MORE:ESE_BLOCK_LAST;
		g_eSE_sendiBlkInfo.crtDataLen=g_eSE_sendiBlkInfo.isChained?g_esespi_param[dev_id].g_sIfsc:sRemainLen;
		g_eSE_sendiBlkInfo.dataOffset = sTotalLenSent;

		ret_code=proto_spi_ese_sendIblock(periph, param, g_eSE_sendiBlkInfo); 
		if(ret_code != SE_SUCCESS) 
		{
			re_tran_count++;
			continue;//loop to retransmit last chained block  
		}
	
		if(!g_eSE_sendiBlkInfo.isChained)  //last 
		{
			g_bSendSeqNum^=1;   						//����Last I-Block��ɷ�תNs (g_eSE_sendiBlkInfo.seqNo) 
			g_eSE_sendiBlkInfo.dataOffset=sTotalLenSent;

			return SE_SUCCESS;  
		}

		do   //ESE_BLOCK_MORE : send chain data
		{
			ret_code=eSESPI_RecvRBlock(periph, param, g_bSendSeqNum^1); //Ԥ��NrΪ��һ��Ns,���޸�g_bSendSeqNum��ֵ
			if(ret_code == SE_SUCCESS) 
			{
				//continue loop to transmit next chained block				
				if(g_eSE_sendiBlkInfo.isChained)
				{	//send not finished
					g_bSendSeqNum^=1;	 				//����R-bock��ɷ�תNs
					sTotalLenSent+=g_eSE_sendiBlkInfo.crtDataLen;
					break;
				}
				else
				{	//send finished  ���������I-block�����յ� R-block �����break��
					g_bSendSeqNum^=1;					//����R-bock��ɷ�תNs
					
					g_eSE_sendiBlkInfo.p_data = NULL;
					g_eSE_sendiBlkInfo.crtDataLen=0;
					g_eSE_sendiBlkInfo.isChained = ESE_BLOCK_LAST;
					
					proto_spi_ese_sendIblock(periph, param, g_eSE_sendiBlkInfo);   //���Ϳ� Last I-Block 
					
					g_bSendSeqNum^=1;					//����Last I-Block��ɷ�תNs
					return SE_SUCCESS;
				}
			}
			else if((ret_code==SE_ERR_TIMEOUT)||(ret_code==SE_ERR_COMM)) 
			{
				return ret_code;
			}
			else if(ret_code==SE_ERR_ESE_ABORT) 
			{
				//received ABORT request S block			Scenario 27
				sTotalLenSent=0;
				continue;
			}
			else
			{
				re_tran_count++;
				break;//loop to retransmit last chained block     
			}
		}while(1);
		
		if(re_tran_count > PROTO_SPI_ESE_RETRY_NUM)
		{
		   return SE_ERR_COMM;	
		}
	
	}while(1); //end while(1)
}


/******************************************************************************
 * Function         eSESPI_RecvAppData
 *
 * Description      This function is called to reveive AppData with all
 *                  updated 7816-3 headers
 *
 * Returns          On success return true or else false.
 *
 ******************************************************************************/
se_error_t proto_spi_ese_recv_data(peripheral *periph, eSEspi_param_t *param, uint8_t *outbuf, uint32_t *outbuf_len)
{
	uint32_t iCrtBlkLen=0;
	se_error_t ret_code = SE_SUCCESS;
//	util_timer_t timer = {0};
//	uint32_t rec_len = 0;
//	uint16_t send_nak_count = 0; 
//	uint16_t rec_nak_count = 0;
//	uint16_t re_tran_count = 0;
	uint8_t dev_id;
	
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	if(periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((param == NULL) || (outbuf == NULL))
	{
		return SE_ERR_PARAM_INVALID;
	}
	dev_id = p_spi_periph->periph.id;		
	
	g_eSE_recviBlkInfo.p_data = outbuf;   
	g_eSE_recviBlkInfo.totalDataLen = 0;  
	g_eSE_recviBlkInfo.dataOffset = 0;  	
	
	do
	{
		//���ڲ���������buff ʹ��G_SPIRecvBufȫ������	
		ret_code=eSESPI_RecvIBolck(periph, param,G_SPIRecvBuf,&iCrtBlkLen);
		if(ret_code==SE_SUCCESS)
		{
			//received the last chained I block
			g_bRecvSeqNum=g_bRecvSeqNum?0:1;

			//only return APDU data to PC
			if(G_SPIRecvBuf[ESE_LEN_OFFS] == LEN_EXT_TAG)   //ʹ��˫�������ͷβ
			{
				g_eSE_recviBlkInfo.crtDataLen = iCrtBlkLen-G_BLOCK_PRO_AND_EPI_EXT_SIZE;
				memcpy(g_eSE_recviBlkInfo.p_data+g_eSE_recviBlkInfo.dataOffset, 
							G_SPIRecvBuf+PROLOGUE_EXT_SIZE, 
							g_eSE_recviBlkInfo.crtDataLen);
			}
			else
			{
				g_eSE_recviBlkInfo.crtDataLen = iCrtBlkLen-G_BLOCK_PRO_AND_EPI_SIZE(G_SPIRecvBuf);
				memcpy(g_eSE_recviBlkInfo.p_data+g_eSE_recviBlkInfo.dataOffset, 
							G_SPIRecvBuf+PROLOGUE_NOMAL_SIZE,
							g_eSE_recviBlkInfo.crtDataLen);
			}

			g_eSE_recviBlkInfo.totalDataLen += g_eSE_recviBlkInfo.crtDataLen;
			g_eSE_recviBlkInfo.dataOffset = g_eSE_recviBlkInfo.totalDataLen;
//			InforLen=g_eSE_recviBlkInfo.totalDataLen;
			
			ret_code = SE_SUCCESS;
			break;
		}
		else if((ret_code==SE_ERR_TIMEOUT)||(ret_code==SE_ERR_COMM)) 
		{
			break;
		}
		else if(ret_code==ESE_TRANS_MOREDATA)
		{
			proto_spi_ese_sendRblock(periph,param,NO_ERROR); // ACK	
			
			//received chaining I block,chain not finished
			g_bRecvSeqNum=g_bRecvSeqNum?0:1;		
			
			//only return APDU data to PC
			if(G_SPIRecvBuf[ESE_LEN_OFFS] == LEN_EXT_TAG)
			{
				g_eSE_recviBlkInfo.crtDataLen = iCrtBlkLen-G_BLOCK_PRO_AND_EPI_EXT_SIZE;
				memcpy(g_eSE_recviBlkInfo.p_data+g_eSE_recviBlkInfo.dataOffset, 
							G_SPIRecvBuf+PROLOGUE_EXT_SIZE, 
							g_eSE_recviBlkInfo.crtDataLen);
			}
			else
			{
				g_eSE_recviBlkInfo.crtDataLen = iCrtBlkLen-G_BLOCK_PRO_AND_EPI_SIZE(G_SPIRecvBuf);
				memcpy(g_eSE_recviBlkInfo.p_data+g_eSE_recviBlkInfo.dataOffset, 
							G_SPIRecvBuf+PROLOGUE_NOMAL_SIZE,
							g_eSE_recviBlkInfo.crtDataLen);
			}

			g_eSE_recviBlkInfo.totalDataLen += g_eSE_recviBlkInfo.crtDataLen;
			g_eSE_recviBlkInfo.dataOffset = g_eSE_recviBlkInfo.totalDataLen;

			continue;
		}
		else if(ret_code==SE_ERR_ESE_ABORT) 
		{
			//received ABORT request S block
			g_eSE_recviBlkInfo.totalDataLen = 0;  
			g_eSE_recviBlkInfo.dataOffset = 0;  
			continue;
		}
		//else if(bStatus==ESE_TRANS_RESYNCH)  //Ŀǰ�����������������������֧�����ʱ���ͬ��֡ eSESPI_Block_Handler������û�д��� 
		//{//received RESYNCH request S block
		//	AppDataAddr-=length;
		//	*pAppDataLen-=length;
		//	continue;
		//}
		else if(ret_code==SE_ERR_RETRY)    
		{			
			g_esespi_param[dev_id].g_bRetryNum++;
			if(g_esespi_param[dev_id].g_bRetryNum >PROTO_SPI_ESE_RETRY_NUM)
			{
				ret_code = SE_ERR_COMM;
				break;
			}
			
			//last APDU response sent failed
			g_bSendSeqNum^=1;   //����Ns �ط���һ��I-block
			proto_spi_ese_sendIblock(periph,param,g_eSE_sendiBlkInfo); 
			g_bSendSeqNum^=1;   //�ط����ٴη�תNs

			continue;
		}
		else
		{
			//received nonsense R block
			proto_spi_ese_sendRblock(periph,param,OTHER_ERROR);	
			continue;
		}
	}while(1); //end while(1)

	*outbuf_len = g_eSE_recviBlkInfo.totalDataLen;
	
	g_esespi_param[dev_id].g_bRetryNum = 0;
	return ret_code;
}

/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/

/** @defgroup Proto_Spi_Exported_Functions Proto_Spi Exported Functions
  * @{
  */


/**
* @brief ����SPI eSEͨ��Э�������ʼ��
* -# IFXC�Ȳ�����ʼ��
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵĳ�ʼ������init
* @param [in] periph  �豸���
* @return ��������״̬��	
* @note no
* @see ����  proto_spi_ese_init
*/

se_error_t proto_spi_ese_init(peripheral *periph) 
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint8_t dev_id = 0;
	
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_ese_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	if(p_spi_ese_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	//�������ȴ��ĳ�ʱʱ��
	timer.interval = SPI_ESE_COMM_MUTEX_WAIT_TIME; 
	p_spi_ese_periph->timer_start(&timer);

	do
	{
		if(p_spi_ese_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;	
			LOGE("Failed:init mutex,  ErrCode-%ld.", ret_code);
			break;
		}

		dev_id = p_spi_ese_periph->periph.id;
		
		eSESPI_ResetProParas(dev_id);
		
		ret_code = p_spi_ese_periph->init(p_spi_ese_periph);	
		if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:spi potocol,  ErrCode-%ld.", ret_code);
		}
		else
		{
			LOGI("Success!");
		}
		break;
	}while(1);
	
    return ret_code;
}

/**
* @brief ����SPI eseͨ��Э�������ֹ��
* -# Ifsc�Ȳ����ָ�Ĭ��ֵ
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵ� ��ֹ������deinit
* @param [in] periph  �豸���
* @return ��������״̬��	
* @note no
* @see ����    proto_spi_ese_deinit
*/
se_error_t proto_spi_ese_deinit(peripheral *periph) 
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};

	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_ese_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	if(p_spi_ese_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	//���õȴ��ĳ�ʱʱ��
	timer.interval = SPI_ESE_COMM_MUTEX_WAIT_TIME; 
	p_spi_ese_periph->timer_start(&timer);

	do
	{
		if(p_spi_ese_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;	
			LOGE("Failed:deinit mutex,  ErrCode-%ld.", ret_code);
			break;
		}
		
		ret_code = p_spi_ese_periph->deinit(p_spi_ese_periph);
		if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:spi potocol,  ErrCode-%ld.", ret_code);
		}
		else
		{
			LOGI("Success!");
		}
		break;

	}while(1);

	
	return ret_code;
}


/**
* @brief ���Ӵ��豸ʱ�����ô˺������Ի�ȡ���豸��ATR
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵ� control������SE���и�λ��׵
* -# ���͸�λ��������
* -# ����RATR�������У���ȡ���豸ATRֵ
* @param [in] periph  �豸���
* @param [out] rbuf  ������ATR����ʼ��ַ
* @param [out] rlen ������ATR�ĳ���
* @return ��������״̬��	
* @note no
* @see ����  port_spi_periph_control   proto_spi_reset_frame proto_spi_ratr_frame
*/
se_error_t proto_spi_ese_open(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) 
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint32_t rec_len = 0;
	uint8_t dev_id = 0;
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;
//	uint8_t index = 0;
	uint8_t reset_buf[COMM_ATR_LEN+BLOCK_MIN_SIZE];  //������ʱbuff
	
	if(p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (((rbuf == NULL) && (rlen != NULL))||((rbuf != NULL) && (rlen == NULL)))
	{
		return  SE_ERR_PARAM_INVALID;
	}

	//���õȴ��ĳ�ʱʱ��
	timer.interval = SPI_ESE_COMM_MUTEX_WAIT_TIME; 
	p_spi_periph->timer_start(&timer);

	do
	{
		if(p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;	
			LOGE("Failed:open periph mutex,  ErrCode-%ld.", ret_code);
			break;
		}
		
		
//		//����SE ��RST���Ž��и�λ����
//		ret_code = p_spi_periph->control(p_spi_periph, PROTO_SPI_ESE_CTRL_RST, NULL, NULL);
//		if(ret_code != SE_SUCCESS)
//			{
//			LOGE("Failed:HW rst io control,  ErrCode-%ld.", ret_code);
//			//break;
//		}
// 		
//        p_spi_periph->delay(SPI_ESE_PROTO_SE_RST_DELAY);//�ӳ�ȷ��SE����
		
		dev_id = p_spi_periph->periph.id;
		g_esespi_param[dev_id].type = ESE_HARD_RESET_REQ;
		
		ret_code = proto_spi_ese_exchg_sblock(periph, &g_esespi_param[dev_id], reset_buf, &rec_len);
		if(ret_code == SE_ERR_BUSY)
		{
			continue;
		}
		else if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol reset  frame,  ErrCode-%ld.", ret_code);
			break;
		}

		*rlen = rec_len;

		LOGI("Open Periph Success!");
		break;
	}while(1);

	return ret_code;
}


/**
* @brief Ӧ�ùر��豸ʱ�����ô˺���
* @param [in] periph  �豸���
* @return ��������״̬��	
* @note �޴���ʵ��
*/
se_error_t proto_spi_ese_close(peripheral *periph) 
{
	se_error_t ret_code = SE_SUCCESS;

	return ret_code;
}


/**
* @brief ͨ��SPI�ӿ�����豸�������������Ӧʱ�����ô˺���
* -# ��˫����зֱ�������ʼ��(PIB, LEN)
* -# ��˫����зֱ�������ֹ��(EDC)
* -# ����proto_spi_handle���������������������Ӧ
* @param [in] periph  �豸���
* @param [in] sbuf ����˫����е���ʼ��ַ
* @param [in] slen ����˫����е��������ݳ���
* @param [out] rbuf  ���˫����е���ʼ��ַ
* @param [out] rlen ���˫����е�������ݳ��ȵ���ʼ��ַ
* @return ��������״̬��	
* @see ����  porto_spi_queue_in   proto_spi_handle
*/
se_error_t proto_spi_ese_transceive(peripheral *periph, uint8_t *sbuf, uint32_t  slen, uint8_t *rbuf, uint32_t *rlen)
{
	se_error_t ret_code = SE_SUCCESS;
//	util_timer_t timer = {0};
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;
	uint32_t rec_len = 0;
	uint8_t dev_id = 0;

	double_queue queue_in = (double_queue)sbuf;
	double_queue queue_out = (double_queue)rbuf;
	uint8_t* p_input = NULL;
	uint8_t* p_output = NULL;	
	
	if(p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if ((sbuf == NULL) || (rbuf == NULL) || (slen == 0U) || (rlen == NULL))
	{
		return  SE_ERR_PARAM_INVALID;
	}
	
	p_input = &queue_in->q_buf[queue_in->front_node];
	p_output = &queue_out->q_buf[queue_out->front_node];
	dev_id = p_spi_periph->periph.id;
	
	//׼���շ������� (peripheral *periph, eSEspi_param_t *param, uint8_t *sbuf, uint32_t slen)	
	ret_code = proto_spi_ese_send_data(periph,&g_esespi_param[dev_id],p_input,util_queue_size(queue_in));
	if(ret_code != SE_SUCCESS)
	{
		return ret_code;
	}	
 
	//G_SPIRecvBuf��������G_SPIRecvBuf 
	ret_code =proto_spi_ese_recv_data(periph,&g_esespi_param[dev_id],p_output,(uint32_t*)&rec_len);
	if(ret_code) 
	{
		LOGE("Failed:protocol communication,  ErrCode-%ld.", ret_code);
	}
	else
	{	
		queue_out->q_buf_len = rec_len;
		queue_out->rear_node =  queue_out->front_node + rec_len;

		*rlen = util_queue_size(queue_out);
		
		LOGI("Communication Success!");
	}
	

	
	return ret_code;	
}



/**
* @brief ���Ӵ��豸ʱ�����ô˺����ӿڸ�λ����ȡ���豸��ATR
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵ� control������SE���и�λ��׵
* -# ���͸�λ��������
* -# ����RATR�������У���ȡ���豸ATRֵ
* @param [in] periph  �豸���
* @param [out] rbuf  ������ATR����ʼ��ַ
* @param [out] rlen ������ATR�ĳ���
* @return ��������״̬��	
* @note no
* @see ����  port_spi_periph_control proto_spi_reset_frame proto_spi_ratr_frame
*/
se_error_t proto_spi_ese_reset(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) 
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint32_t rec_len = 0;
	uint8_t reset_buf[COMM_ATR_LEN+BLOCK_MIN_SIZE];  //������ʱbuff
	
	uint8_t dev_id = 0;
	
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;
//	uint8_t index = 0;

	if(p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (((rbuf == NULL) && (rlen != NULL))||((rbuf != NULL) && (rlen == NULL)))
	{
		return  SE_ERR_PARAM_INVALID;
	}

	//���õȴ��ĳ�ʱʱ��
	timer.interval = SPI_ESE_COMM_MUTEX_WAIT_TIME; 
	p_spi_periph->timer_start(&timer);

	do
	{
		if(p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;	
			LOGE("Failed:open periph mutex,  ErrCode-%ld.", ret_code);
			break;
		}
		
		//����SE ��RST���Ž��и�λ����
		ret_code = p_spi_periph->control(p_spi_periph, PROTO_SPI_ESE_CTRL_RST, NULL, NULL);
		if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol rst io control,  ErrCode-%ld.", ret_code);
			break;
		}

		p_spi_periph->delay(SPI_ESE_PROTO_SE_RST_DELAY);//�ӳ�ȷ��SE����
		
		dev_id = p_spi_periph->periph.id;
		g_esespi_param[dev_id].type = ESE_INTF_RESET_REQ;
		
		ret_code = proto_spi_ese_exchg_sblock(periph, &g_esespi_param[dev_id], reset_buf, &rec_len);
		if(ret_code == SE_ERR_BUSY)
		{
			continue;
		}
		else if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol reset  frame,  ErrCode-%ld.", ret_code);
			break;
		}

		*rlen = rec_len;

		LOGI("Open Periph Success!");
		break;
		
	}while(1);

	return ret_code;
}

/**
* @brief ���Ӵ��豸ʱ���ϵ���ȡ���豸��ATR
* -# ͨ��port���豸ע��ĺ����б�ָ�룬����port��spi�ӿڵ� control������SE���и�λ��׵
* -# ���͸�λ��������
* -# ����RATR�������У���ȡ���豸ATRֵ
* @param [in] periph  �豸���
* @param [out] rbuf  ������ATR����ʼ��ַ
* @param [out] rlen ������ATR�ĳ���
* @return ��������״̬��	
* @note no
* @see ����  proto_spi_ese_ratr  
*/
se_error_t proto_spi_ese_ratr(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) 
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint32_t rec_len = 0;
	uint8_t dev_id = 0;
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;
	uint8_t reset_buf[COMM_ATR_LEN+BLOCK_MIN_SIZE];  //������ʱbuff

	if(p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (((rbuf == NULL) && (rlen != NULL))||((rbuf != NULL) && (rlen == NULL)))
	{
		return  SE_ERR_PARAM_INVALID;
	}

	//���õȴ��ĳ�ʱʱ��
	timer.interval = SPI_ESE_COMM_MUTEX_WAIT_TIME; 
	p_spi_periph->timer_start(&timer);

	do
	{
		if(p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;	
			LOGE("Failed:open periph mutex,  ErrCode-%ld.", ret_code);
			break;
		}

		//����SE ��RST���Ž��и�λ����
		ret_code = p_spi_periph->control(p_spi_periph, PROTO_SPI_ESE_CTRL_RST, NULL, NULL);
		if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol rst io control,  ErrCode-%ld.", ret_code);
			break;
		}

		p_spi_periph->delay(SPI_ESE_PROTO_SE_RST_DELAY);//�ӳ�ȷ��SE����

		dev_id = p_spi_periph->periph.id;
		g_esespi_param[dev_id].type = ESE_ATR_REQ;
		
		ret_code = proto_spi_ese_exchg_sblock(periph, &g_esespi_param[dev_id], reset_buf, &rec_len);
		if(ret_code == SE_ERR_BUSY)
		{
			continue;
		}
		else if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol reset  frame,  ErrCode-%ld.", ret_code);
			break;
		}

		*rlen = rec_len;
            
		LOGI("Open Periph Success!");
		break;
	}while(1);
	return ret_code;
}

se_error_t proto_spi_ese_endApdu(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) 
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	uint32_t rec_len = 0;
	uint8_t dev_id = 0;
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;
	uint8_t end_buf[0x10];  //������ʱbuff

	if(p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (((rbuf == NULL) && (rlen != NULL))||((rbuf != NULL) && (rlen == NULL)))
	{
		return  SE_ERR_PARAM_INVALID;
	}

	//���õȴ��ĳ�ʱʱ��
	timer.interval = SPI_ESE_COMM_MUTEX_WAIT_TIME; 
	p_spi_periph->timer_start(&timer);

	do
	{
		if(p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;	
			LOGE("Failed:open periph mutex,  ErrCode-%ld.", ret_code);
			break;
		}

//		//����SE ��RST���Ž��и�λ����
//		ret_code = p_spi_periph->control(p_spi_periph, PROTO_SPI_ESE_CTRL_RST, NULL, NULL);
//		if(ret_code != SE_SUCCESS)
//		{
//			LOGE("Failed:protocol rst io control,  ErrCode-%ld.", ret_code);
//			break;
//		}

//		p_spi_periph->delay(SPI_ESE_PROTO_SE_RST_DELAY);//�ӳ�ȷ��SE����

		dev_id = p_spi_periph->periph.id;
		g_esespi_param[dev_id].type = ESE_PROP_END_APDU_REQ;
		
		ret_code = proto_spi_ese_exchg_sblock(periph, &g_esespi_param[dev_id], end_buf, &rec_len);
		if(ret_code == SE_ERR_BUSY)
		{
			continue;
		}
		else if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:protocol reset  frame,  ErrCode-%ld.", ret_code);
			break;
		}

		*rlen = rec_len;
            
		LOGI("Open Periph Success!");
		break;
	}while(1);
	return ret_code;
}

/**
* @brief ͨ��SPI�ӿ�����豸����RST�������Ž��и�λ�����Ŀ��������������������
* @param [in] periph  �豸���
* @param [in] ctrlcode �������·
* @param [in] sbuf �������ݵ���ʼ��ַ
* @param [in] slen �������ݵĳ���
* @param [out] rbuf ������ݵ���ʼ��ַ
* @param [out] rlen ������ݳ��ȵ���ʼ��ַ
* @return ��������״̬��	
* @see ����  port_spi_periph_control 
*/
se_error_t proto_spi_ese_control(peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen)
{
	se_error_t ret_code = SE_SUCCESS;
	util_timer_t timer = {0};
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;

	if(p_spi_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	if (ctrlcode == 0U)
	{
		return  SE_ERR_PARAM_INVALID;
	}

	//���õȴ��ĳ�ʱʱ��
	timer.interval = SPI_ESE_COMM_MUTEX_WAIT_TIME; 
	p_spi_periph->timer_start(&timer);

	do
	{
		if(p_spi_periph->timer_differ(&timer) != SE_SUCCESS)
		{
			ret_code = SE_ERR_TIMEOUT;	
			LOGE("Failed:control periph mutex,  ErrCode-%ld.", ret_code);
			break;
		}	
		ret_code = p_spi_periph->control(p_spi_periph, ctrlcode, sbuf,&slen);
		if(ret_code != SE_SUCCESS)
		{
			LOGE("Failed:spi potocol,  ErrCode-%ld.", ret_code);
		}
		else
		{
			LOGI("Success!");
		}
		break;
	}while(1);

	
	return ret_code;
}


/**
* @brief ʵ��΢���ʱ
* @param [in] periph  �豸���
* @param [in] us  ΢��
* @return ��������״̬��	
* @note no
* @see ����  port_spi_periph_delay  
*/
se_error_t proto_spi_ese_delay(peripheral *periph , uint32_t us) 
{
	se_error_t ret_code = SE_SUCCESS;
	HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER p_spi_ese_periph = (HAL_SPI_ESE_PERIPHERAL_STRUCT_POINTER)periph;
	
	if(p_spi_ese_periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}
	//��ʱָ��΢��ʱ��
	p_spi_ese_periph->delay(us);
	return ret_code;
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



