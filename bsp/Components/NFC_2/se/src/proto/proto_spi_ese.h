/**@file  proto_spi_ese.h
* @brief  proto_spi interface declearation (about SPI HED eSE)
* @author  chenfeng
* @date  2023-07-20
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/
#ifndef _PROTO_SPI_ESE_H_
#define _PROTO_SPI_ESE_H_

/***************************************************************************
* Include Header Files
***************************************************************************/
#include <stdint.h>
#include "types.h"
#include "config.h"
#include "peripheral.h"
#include "ial_spi_ese.h"
#include "util.h"

/**************************************************************************
* Global Variable Declaration
***************************************************************************/
////extern peripheral_bus_driver g_proto_spi;
PERIPHERAL_BUS_DRIVER_DECLARE(PERIPHERAL_ESE); 



/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PROTO 
  * @brief link protocol layer.
  * @{
  */


/** @addtogroup PROTO_SPI 
  * @{
  */


/* Private constants --------------------------------------------------------*/
/** @defgroup Proto_Spi_Private_Constants Proto_Spi Private Constants
  * @{
  */


/** @defgroup HED_eSE_SPI_TIME_PARAMS    HED eSE SPI Communication Protocol Time Params
  * @{
  */
#define SPI_ESE_SEND_CS_WAKEUP_TSS_TIME         	   2  /*!< ����ʱ��T0: 2+4.5 =6.5us tss*/  

#define SPI_ESE_SEND_DATA_OVER_WAIT_TIME              20 	/*!< Э�����T3: 200us */ 
#define SPI_ESE_RESET_POLL_SLAVE_INTERVAL_TIME        20   /*!< Э�����T4: 200us */ 
#define SPI_ESE_BEFORE_RECEIVE_DATA_WAIT_TIME         20   /*!< Э�����T5: 200us */ 
#define SPI_ESE_SEND_BGT_TIME                         100  /*!< ֡����ʱ��: 1000us */ 
	
#define SPI_ESE_RECEVIE_FRAME_WAIT_TIME               100000000   /*!< ֡�ȴ�ʱ��BWT: 700ms */ 

#define SPI_ESE_COMM_MUTEX_WAIT_TIME                  		20000000   /*!< ͨ������ʶ�ȴ���ʱʱ��: 200ms */ 
#define SPI_ESE_PROTO_SE_RST_DELAY                 			350000   /*!< �ӳ�ȷ��SE���� */ 
/**
  * @}
  */

/** @defgroup HED_SPI_FRAME_DEFINE    HED SPI  Protocol Frame Define
  * @{
  */

////֡���Ͷ���
//#define PIB_ACTIVE_FRAME                0x03   /*!< ����֡*/        
//#define PIB_INFORMATION_FRAME    0x0E  /*!< ��Ϣ֡*/      
//#define PIB_PROCESS_FRAME             0x09  /*!< ����֡*/

////����֡���Ͷ���
//#define PIB_ACTIVE_FRAME_RESET          0xD3  /*!<RESET ����֡*/        
//#define PIB_ACTIVE_FRAME_RATR           0xE2  /*!<RATR ����֡*/   
//#define PIB_ACTIVE_FRAME_RATR_RESPONSE  0x3B 

////����֡���Ͷ���
//#define PIB_PROCESS_FRAME_NAK_CRC_INFO      0x3C   /*!<NAK ����֡*/
//#define PIB_PROCESS_FRAME_NAK_OTHER_INFO      0x3D   /*!<NAK ����֡*/      

//#define PIB_PROCESS_FRAME_WTX_INFO      0x60  /*!<WTX ����֡*/     

#define getShort(src)			( (unsigned short)(((*((unsigned char *)(src)))<<8)|(*((unsigned char *)(src) + 1))) )
#define setShort(dst, value)   	(*(unsigned char*)(dst)) = ((unsigned short)(value)>>8);(*(unsigned char*)((dst)+1)) = ((unsigned char)(value)); 

#define switchByte(tByte)		(tByte<<4|tByte>>4)	  //�����ֽڵĸߵͰ��ֽ�

//�ֶ���buf�е�ƫ��
//#define PIB_OFFSET        0
//#define LEN_OFFSET        1
//#define DATA_OFFSET      3
#define APDU_CLA_OFFSET 3
#define APDU_INS_OFFSET 4
#define APDU_P1_OFFSET  5

#define ESE_I_BLOCK                 	  0 
#define ESE_R_BLOCK                     2
#define ESE_S_BLOCK                     3

#define ESE_S_BLOCK_REQ                 0xC0
#define ESE_S_BLOCK_RSP                 0xE0

#define PCB_MSB_BIT    			(7)
#define PCB_NS_BIT    			(6)
#define PCB_M_BIT    			  (5)
#define PCB_NR_BIT    			(4)

#define PCB_BIT_NS    			(1<<PCB_NS_BIT)
#define PCB_BIT_M    			(1<<PCB_M_BIT)
#define PCB_BIT_NR    			(1<<PCB_NR_BIT)

//PCB Block Type
#define BLOCK_MASK(pcb)    		(pcb>>PCB_NS_BIT) 
#define SET_BLOCK_TYPE(BLOCK_TYPE)    (BLOCK_TYPE<<PCB_NS_BIT) 

#define IS_I_BLOCK(PCB)    		(CHECK_BIT(PCB,PCB_MSB_BIT) == 0)   		//BIT8 ���λ
#define IS_R_BLOCK(PCB)      	((BLOCK_MASK(PCB)&R_BLOCK) == R_BLOCK)
#define IS_S_BLOCK(PCB)       	((BLOCK_MASK(PCB)&S_BLOCK) == S_BLOCK)
//Э���ֶγ��ȶ���
//Block flag bytes define
#define NAD_S 					(0x5)  
#define NAD_M 					(0xA)  
#define NAD_MS 					(0xA5)  
#define NAD_SM 					(0x5A) 

#define NAD_SIZE 				(1)
#define PCB_SIZE				(1)
#define LEN_POLLING				(NAD_SIZE+PCB_SIZE)

#define LEN_NOMAL_SIZE			(1)

#define LEN_NOMAL_TAG			(0x0)
#define LEN_EXT_TAG				(0xFF)
#define LEN_NOMAL_MAX			(LEN_EXT_TAG-1)
#define LEN_EXT_TAG_SIZE		(LEN_NOMAL_SIZE)
#define LEN_EXT_SIZE			(3)

#define LEN_EXT_LEN_SIZE		(3-LEN_NOMAL_SIZE)

#define ESE_BLOCK_NAD         	NAD_SM  	//��������
#define ESE_REV_SE_NAD 			NAD_MS		//se����

#define ESE_NAD_OFFS   			(0x00)
#define ESE_PCB_OFFS   			(ESE_NAD_OFFS+NAD_SIZE)
#define ESE_LEN_OFFS   			(ESE_PCB_OFFS+PCB_SIZE)

#define PROLOGUE_NOMAL_SIZE 	(NAD_SIZE+PCB_SIZE+LEN_NOMAL_SIZE)
#define PROLOGUE_EXT_SIZE 		(NAD_SIZE+PCB_SIZE+LEN_EXT_SIZE)
#define LEN_EXT_VALUE_OFFS      (ESE_LEN_OFFS+LEN_EXT_TAG_SIZE)

#define BLOCK_MIN_SIZE       	(1+PROLOGUE_NOMAL_SIZE)
#define G_EPILOGUE_SIZE 		(1)  //error detection code to use,1:LRC;2:CRC
 
 
 #define ESE_BLOCK_INF_FIELD_MAX_SIZE       	MAX_IFS
#define ESE_BLOCK_PROLOGUE_NOMAL_SIZE 		(NAD_SIZE+PCB_SIZE+LEN_NOMAL_SIZE)
#define ESE_BLOCK_PROLOGUE_EXT_SIZE 		(NAD_SIZE+PCB_SIZE+LEN_EXT_SIZE)

//ʹ��g_abPrologueȫ��������Ϊ��������ͷ��Ϣ
#define G_LEN_NOMAL_VALUE(buf)		buf[ESE_LEN_OFFS]
#define G_LEN_EXT_VALUE(buf)		getShort(buf+LEN_EXT_VALUE_OFFS)
		 
#define IS_EXT_LEN_FORMAT(buf)     	(LEN_EXT_TAG==G_LEN_NOMAL_VALUE(buf))

#define G_PROLOGUE_SIZE(buf)  		(IS_EXT_LEN_FORMAT(buf) ? PROLOGUE_EXT_SIZE:PROLOGUE_NOMAL_SIZE)

#define G_INFO_OFFS   			(G_PROLOGUE_SIZE) 

#define G_BLOCK_PRO_AND_EPI_SIZE(buf)  	(G_PROLOGUE_SIZE(buf)+G_EPILOGUE_SIZE)
#define G_BLOCK_PRO_AND_EPI_EXT_SIZE 	(PROLOGUE_EXT_SIZE+G_EPILOGUE_SIZE)

#define G_ESE_NAD(buf)        		buf[ESE_NAD_OFFS]
#define G_ESE_PCB(buf)        		buf[ESE_PCB_OFFS]
#define G_ESE_NOMAL_LEN(buf)     	G_LEN_NOMAL_VALUE(buf)

#define getLEN(buf)				(IS_EXT_LEN_FORMAT(buf) ? G_LEN_EXT_VALUE(buf):G_LEN_NOMAL_VALUE(buf))

#define ESE_BLOCK_OK       		0  
#define ESE_BLOCK_ERR     		1    
#define ESE_R_BLOCK_OK      	0 
#define ESE_R_BLOCK_CRC_ERR   	1 
#define ESE_R_BLOCK_OTHER_ERR 	2 

#define ESE_BLOCK_LAST      	0  
#define ESE_BLOCK_MORE       	1  
 

//extern uint16_t g_sIfsd;		 		 
//extern uint16_t g_sIfsc;		 
//extern uint8_t 	g_bRetryNum; 				 
//extern uint16_t g_sMaxIfsc;	
	
/*!
 * \brief S-Block  types used in 7816-3 protocol stack
 */
typedef enum eSE_sBlockTypes {
  ESE_RESYNCH_REQ = 0xC0, 		/*!< Re-synchronisation request between host and ESE */
  ESE_RESYNCH_RSP = 0xE0, 		/*!< Re-synchronisation response between host and ESE */
  ESE_IFS_REQ = 0xC1,    		/*!< IFSC size request */
  ESE_IFS_RSP = 0xE1,    		/*!< IFSC size response */
  ESE_ABORT_REQ = 0xC2,   		/*!< Abort request */
  ESE_ABORT_RSP = 0xE2,   		/*!< Abort response */
  ESE_WTX_REQ = 0xC3,     		/*!< WTX request */
  ESE_WTX_RSP = 0xE3,     		/*!< WTX response */
  ESE_INTF_RESET_REQ = 0xC4,    /*!< Interface reset request */
  ESE_INTF_RESET_RSP = 0xE4,    /*!< Interface reset response */
  ESE_PROP_END_APDU_REQ = 0xC5, /*!< Proprietary Enf of APDU request */
  ESE_PROP_END_APDU_RSP = 0xE5, /*!< Proprietary Enf of APDU response */
  ESE_HARD_RESET_REQ = 0xC6,	/*!< Chip reset request */
  ESE_HARD_RESET_RSP = 0xE6,	/*!< Chip reset request */
  ESE_ATR_REQ = 0xC7,    		/*!< ATR request */
  ESE_ATR_RSP = 0xE7,    		/*!< ATR response */
  INVALID_REQ_RES           	/*!< Invalid request */
} eSE_sBlockTypes_t;


/**
  * @brief  ese SPI Param Structure definition
  */
typedef struct  {
	uint16_t 	g_sIfsd;		//infomation feild size in I block sent by card
	uint16_t 	g_sIfsc;		//infomation feild size in I block received by card
	uint16_t 	g_sMaxIfsc;	
	
	uint8_t 	g_bRetryNum; 				 
	eSE_sBlockTypes_t	 type;  ///<S-block����
} eSEspi_param_t;


/*!
 * \brief R-Block types used in 7816-3 protocol stack
 */
typedef enum rBlockTypes {
  RACK = 0x01, 		/*!< R-frame Acknowledgement frame indicator */
  RNACK = 0x02 		/*!< R-frame Negative-Acknowledgement frame indicator */
} rBlockTypes_t;

/*!
 * \brief R-Block error types used 7816-3 protocol stack
 */
typedef enum rBlockErrorTypes {
  NO_ERROR=0,         /*!< R-Block received with success RACK */
  PARITY_ERROR,     /*!< R-Block received with parity error */
  OTHER_ERROR,      /*!< R-Block received with Other error */
  SOF_MISSED_ERROR, /*!< R-Block received with frame missing error */
  UNDEFINED_ERROR   /*!< R-Block received with some undefined error */
} rBlockErrorTypes_t;

/*!
 * \brief Frame types used in 7816-3 protocol stack
 */
typedef enum BlockTypes {
  IBLOCK,  			/*!< Block type: I-frame */
  SBLOCK,  			/*!< Block type: S-frame */
  RBLOCK,  			/*!< Block type: R-frame */
  INVALID, 			/*!< Block type: Invalid */
  UNKNOWN  			/*!< Block type: Unknown */
} BlockTypes_t; 


/*!
 * \brief I-Block information structure for ISO 7816-3
 *
 * This structure holds the  information of I-Block used for sending
 * and receiving the Block packet.
 *
 */
typedef struct eSE_iBlockInfo {
  bool isChained; 				/*!< I-Block: Indicates if more Blocks to follow in the same
									data packet or not */
  uint8_t seqNo;    			/*!< I-Block: Sequence number of the I-Block */
	
  uint8_t* p_data;  			/*!< I-Block: Actual data (Information field (INF)) */

  uint32_t dataOffset;   		/*!< I-Block: Offset to the actual data(INF) for the
								current Block of the packet */
  uint32_t totalDataLen; 		/*!< I-Block: Total data left in the packet, used to
									set the chained flag/calculating offset */
  uint32_t crtDataLen;  		/*!< I-Block: the length of the current I-Block actual data */
} eSE_iBlockInfo_t;


/*!
 * \brief S-Block information structure for ISO 7816-3
 *
 * This structure holds the  information of S-Block used for sending
 * and receiving the Block packet.
 *
 */
typedef struct eSE_sBlockInfo {
  eSE_sBlockTypes_t sBlockType; 	/*!< S-Block: Type of S-Block cmd/rsp */
  uint8_t* p_data; 					/*!< S-Block: Actual data (Information field (INF)) */
  uint8_t len; 						/*!< S-Block: the length of the I-Block actual data */
} eSE_sBlockInfo_t;


//#define  FRAME_HEAD_LEN                3   /*!<֡ͷ����*/    
//#define  ACTIVE_FRAME_DATA_EDC_LEN     4  /*!<����֡���ݼ�EDC�ܳ���*/  
//#define  ACTIVE_REQ_FRAME_LEN          7  /*!<����֡�ܳ���*/  
//#define  PROCESS_FRAME_LEN             6  /*!<����֡�ܳ���*/  
//#define  EDC_LEN                       2  /*!< EDC ����*/  
//#define  WAKEUP_DATA_LEN               1 /*!<�����ַ����ȳ���*/ 

//#define FRAME_LEN_MAX                 4101   /*!< ֡��ʽ�ֶε���󳤶�: 3+4096+2 */ 
////#define FRAME_LEN_MAX                 265   /*!< ֡��ʽ�ֶε���󳤶�: 3+5+255+1+1 */ 
//#define FRAME_DATA_LEN_MAX  (FRAME_LEN_MAX - EDC_LEN-FRAME_HEAD_LEN)


//#define PROTO_SPI_PFSM_DEFAULT          0   /*!<PFSM Ĭ��ֵ*/ 
//#define PROTO_SPI_PFSS_DEFAULT          0     /*!<PFSS Ĭ��ֵ*/ 
//#define PROTO_SPI_HBSM_DEFAULT          0   /*!<HBSM Ĭ��ֵ*/ 
//#define PROTO_SPI_HBSS_DEFAULT          0     /*!<HBSS Ĭ��ֵ*/ 

//ATR COMMU Para
#define COMM_ATR_LEN			(19)
#define COMM_WTX_LEN			(1)
#define COMM_CAPACITY			(0x0C)

#define COMM_EDC_OFF			(12)
#define COMM_DIFSC_OFF			(13)
#define COMM_MIFSC_OFF			(15)

#define SEQNO_INIT 				(0)
#define ESE_IFSC_DEFAULT      	(0xFE)
#define ESE_IFSD_DEFAULT        (0xFE) 
#define ESE_IFSD_MAX	        (0x02FD) 	

#define MAXBUFLEN               4089
#define PHY_ESE_INFMAX_SIZE		MAXBUFLEN   //4089	//����CRC-16��Χ T=1'ʱ��4089   MAXBUFLEN

#define PHY_MIFSC_DEFAULT   			ESE_IFSD_MAX
#define PHY_SEND_FIRST_FRAG_LEN         6     		 
#define PHY_SEAL_DEFAULT				PHY_MIFSC_DEFAULT

#define PHY_WAKE_UP_TIME_DEFAULT     	200   //us  WPT  
#define PHY_SE_GUARD_TIME_DEFAULT     	200   //us  SEGT T2 


#define PROTO_SPI_ESE_RETRY_NUM           3    /*!<EDCУ���������ط�����*/ 
 
extern eSE_iBlockInfo_t  		g_eSE_sendiBlkInfo;
extern eSE_iBlockInfo_t  		g_eSE_recviBlkInfo;
#define g_bSendSeqNum    		(g_eSE_sendiBlkInfo.seqNo)    //mark the N(S) in I block sent by HD
#define g_bRecvSeqNum    		(g_eSE_recviBlkInfo.seqNo)    //mark the N(S) in I block sent by SE
   
extern uint8_t G_SPISendBuf[ESE_IFSD_MAX+BLOCK_MIN_SIZE];
extern uint8_t G_SPIRecvBuf[ESE_IFSD_MAX+BLOCK_MIN_SIZE];
 
/**
  * @}
  */




/**
  * @}
  */



/* Exported types ------------------------------------------------------------*/
/** @defgroup Proto_Spi_Impl_Exported_Types Proto_Spi_Impl Exported Types
  * @{
  */

//typedef struct
//{
//    uint32_t  start;	
//    uint32_t  interval;		
//}spi_timer_t;


/**
  * @brief  Proto SPI  Control Structure definition
  */
enum  PROTO_SPI_ESE_CTRL
{
	PROTO_SPI_ESE_CTRL_RST =		0x00000010,    ///< RST ��λ����
	PROTO_SPI_ESE_CTRL_OTHER =		0x00000011    ///< ��������
} ;



/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup Proto_Spi_Exported_Functions Proto_Spi Exported Functions
  * @{
  */

extern se_error_t proto_spi_ese_init(peripheral *periph);
extern se_error_t proto_spi_ese_deinit(peripheral *periph) ;
extern se_error_t proto_spi_ese_open(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) ;
extern se_error_t proto_spi_ese_close(peripheral *periph) ;
//extern se_error_t proto_spi_power_on(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) ;
extern se_error_t proto_spi_ese_transceive(peripheral *periph, uint8_t *sbuf, uint32_t	slen, uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_spi_ese_reset(peripheral *periph , uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_spi_ese_ratr(peripheral *periph , uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_spi_ese_control(peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);
extern se_error_t proto_spi_ese_delay(peripheral *periph , uint32_t us) ;
extern se_error_t proto_spi_ese_endApdu(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) ;

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



#endif //_PROTO_SPI_H_

