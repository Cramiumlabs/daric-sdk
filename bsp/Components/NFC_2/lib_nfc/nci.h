/**
 ****************************************************************
 * @file nci.h
 *
 * @brief
 *
 * @author
 *
 *
 ****************************************************************
 */
#ifndef __NCI_H__
#define __NCI_H__

#include "nfc_type.h"

#define DISCOVER_NTF_TIMEOUT 10 //sal 100//5ms
#define NCI_ACK_TIMEOUT 200		//sal100 //5ms  500
#define DATA_ACK_TIMEOUT 15		//sal100//20ms
#define LOCAL_M1_TIMEOUT 400
/*********************************************************************************************
    控制 Packet 格式:(NCI 包组成)
        MT(3bit)+PBF(1bit)+GID(4bit)+RFU(2bit)+OID(6bit)+payloadlen(8bit)+payload(nbits)
    MT:
        001b Control Packet - Command Message      （命令 , 首字节通常为 20)
        010b Control Packet - Response Message     （响应， 首字节通常为 40）
        011b Control Packet - Notification Message （通知， 首字节通常为 60）
    PBF:
        0b: 完整消息或者是分组消息的最后一个，无后续消息
        1b: 有后续消息
    GID:
        0000b  NCI Core
        0001b  RF Management
        0010b  NFCEE Management
        0011b  NFCC Management
    OID:
        命令ID
    payloadlen:
        一个字节，因此最大负载长度为255字节
*********************************************************************************************
    数据 Packet 格式:(NCI 包组成)
        MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
    MT:
         000b Data Packet - Data Message      （首字节通常为 00)
    Conn ID:
        连接ID, 表示数据属于哪个逻辑通道，创建逻辑通道的时候分配 Conn ID
    CR:
        Credits， 取值为0~3
    payloadlen:
        数据负载的长度，最大为255
**********************************************************************************************/
//Message Type (MT)
//GROUP IDENTIFIER(GID)
//Opcode IDENTIFIER(OID)
//NCI_CORE
#define DATA_RFU (uint8_t)(0x00)
/******************************************************************************************************/
//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
//Message Type (MT)
#define MT_DATA_PACKET (uint8_t)((0x00 << 5) & 0xe0) //0x00  //数据消息
#define RF_DATA_CONN_ID (uint8_t)(0x00)				 //需要解析出来
#define HCI_DATA_CONN_ID (uint8_t)(0x01)			 //主控采用终端connet ID 01

#define PBF_NCI_SET		(uint8_t)((1 << 4) & 0x10)
#define PBF_NCI_RESET	(uint8_t)((0 << 4) & 0x10)

#define HCI_PIPEID_MASK	(uint8_t)(0x7F)
//hci cmd
#define HCI_SET_PARA 0x01
#define HCI_GET_PARA 0x02
#define HCI_OPEN_PIPE 0x03
#define HCI_CLOSE_PIPE 0x04
#define HCI_CREATE_PIPE 0x10
#define HCI_DELETE_PIPE 0x11
#define HCI_CREATED_PIPE_NTF 0x12
#define HCI_DELETED_PIPE_NTF 0x13
#define HCI_CLEAR_ALL_PIPE 0x14
#define HCI_CLEAR_ALL_PIPE_NTF 0x15
#define HCI_GATE_ID 0x30
#define HCI_RESPONSE_OK 0x80

//hci apdugate  registry
#define HCI_REGISTRY_MAX_C_APDU_SIZE 0x01
#define HCI_REGISTRY_MAX_WAIT_TIME 0x02

//控制包：MT(3bit)+PBF(1bit)+GID(4bit)+RFU(2bit)+OID(6bit)+payloadlen(8bit)+payload(nbits)
//Message Type (MT)
#define MT_CTL_CMD_PACKET (uint8_t)((0x01 << 5) & 0xe0) //0x20  //控制信息 命令
#define MT_CTL_RES_PACKET (uint8_t)((0x02 << 5) & 0xe0) //0x40  //控制信息 响应
#define MT_CTL_NTF_PACKET (uint8_t)((0x03 << 5) & 0xe0) //0x60  //控制信息 通知

//GROUP IDENTIFIER(GID)
#define GID_NCI_CORE (uint8_t)(0x00)	 //定义设备主机（DH）和NFC控制器（NFCC）之间所需的基本NCI功能。
#define GID_RF_MANAGE (uint8_t)(0x01)	 //射频管理
#define GID_NFCEE_MANAGE (uint8_t)(0x02) //NFCEE管理
#define GID_NFCCC_MANAGE (uint8_t)(0x03)
#define GID_TEST_MANAGE (uint8_t)(0x04)
#define GID_PROPRIETARY (uint8_t)(0x0F) //私有命令

//Opcode IDENTIFIER(OID)
//NCI_CORE(20) 响应
#define OID_CORE_RESET (uint8_t)(0x00)		//重启NFCC的控制消息
#define OID_CORE_INIT (uint8_t)(0x01)		//初始化NFCC的控制消息
#define OID_CORE_SET_CONFIG (uint8_t)(0x02) //配置参数设置控制消息
#define OID_CORE_GET_CONFIG (uint8_t)(0x03) //读取当前配置的控制消息
#define OID_CONN_GREATE (uint8_t)(0x04)		//DH连接创建控制消息
#define OID_CONN_CLOSE (uint8_t)(0x05)		//连接关闭的控制消息
//NCI_CORE(60) 通知
#define OID_CORE_RESET_NTF (uint8_t)(0x00)			 //重启NFCC的通知消息
#define OID_CORE_CONN_CREDITS_NTF (uint8_t)(0x06)	 //连接信用管理的通知消息
#define OID_CORE_GENERIC_ERROR_NTF (uint8_t)(0x07)	 //通用错误的控制消息
#define OID_CORE_INTERFACE_ERROR_NTF (uint8_t)(0x08) //接口错误的控制消息
//RF_MANAGE(21) 响应
#define OID_RF_DISCOVER_MAP (uint8_t)(0x00)		//射频接口映射配置的控制消息
#define OID_RF_SET_ROUTING (uint8_t)(0x01)		//控制消息配置监听模式路由
#define OID_RF_GET_ROUTING (uint8_t)(0x02)		//读取NFCC监听模式路由的控制消息
#define OID_RF_DISCOVER (uint8_t)(0x03)			//开始发现的控制消息
#define OID_RF_DISCOVER_SELECT (uint8_t)(0x04)	//控制消息选择已发现的目标
#define OID_RF_DEACTIVATE (uint8_t)(0x06)		//射频接口去激活控制信息
#define OID_RF_T3T_POLLING (uint8_t)(0x08)		//控制消息请求NFCC发送Type 3标签轮询命令
#define OID_RF_PARAMETER_UPDATE (uint8_t)(0x0b) //控制消息的RF参数更新
//RF_MANAGE(61) 通知
#define OID_RF_GET_ROUTING_NTF (uint8_t)(0x02)		   //读取NFCC监听模式路由的控制消息
#define OID_RF_DISCOVER_NTF (uint8_t)(0x03)			   //开始发现的控制消息
#define OID_RF_INTF_ACTIVATED_NTF (uint8_t)(0x05)	   //射频接口激活通知
#define OID_RF_DEACTIVATE_NTF (uint8_t)(0x06)		   //射频接口去激活控制信息
#define OID_RF_FIELD_INFO_NTF (uint8_t)(0x07)		   //射频FIELD信息
#define OID_RF_T3T_POLLING_NTF (uint8_t)(0x08)		   //控制消息请求NFCC发送Type 3标签轮询命令
#define OID_RF_NFCEE_ACTION_NTF (uint8_t)(0x09)		   //报告NFCEE动作的通知
#define OID_RF_NFCEE_DISCOVERY_REQ_NTF (uint8_t)(0x0a) //NFCEE发现请求通知
//PROP
#define OID_DROP_SEND_WTX_NTF	(uint8_t)(0x01) //wtx
#define OID_PROP_ACTIVATE (uint8_t)(0x03) //NFCC激活

//NFCEE(22) 响应
#define OID_NFCEE_DISCOVER (uint8_t)(0x00)			   //
#define OID_NFCEE_MODE_SET (uint8_t)(0x01)			   //
#define OID_NFCEE_STATUS_NTF (uint8_t)(0x02)		   //
#define OID_NFCEE_POWER_AND_LINK_CNTRL (uint8_t)(0x03) //

//私有
#define OID_PROPRIETARY_UPDATA_FLASH (uint8_t)(0x02) //
#define OID_PROPRIETARY_LPCD_CHECK (uint8_t)(0x08)	 //
/******************************************************************************************************/

//TOUTING BASE
#define Technology_based (uint8_t)(0x00)
#define Protocol_based (uint8_t)(0x01)
#define AID_based (uint8_t)(0x02)
//NFCEE_IDS
#define DH_NFCEE_ID (uint8_t)(0x00)
//POWER STATES
#define SWITCHED_ON (uint8_t)(0x01)
#define SWITCHED_OFF (uint8_t)(0x02)
#define BATTERY_OFF (uint8_t)(0x04)
//RF TECHNOLOGY
#define NFC_TECHNOLOGY_A (uint8_t)(0x00)
#define NFC_TECHNOLOGY_B (uint8_t)(0x01)
#define NFC_TECHNOLOGY_F (uint8_t)(0x02)
#define NFC_TECHNOLOGY_15693 (uint8_t)(0x03)
#define NFC_TECHNOLOGY_RFU_05 (uint8_t)(0x05)
//RF Protocols

#define NCI_TOTAL_DURATION 0x00
#define NCI_CON_DEVICES_LIMIT 0x01
#define NCI_PN_NFC_DEP_SPEED 0x28
#define NCI_PI_BIT_RATE 0x21
#define NCI_RF_FIELD_INFO 0x80
#define NCI_RF_NFCEE_ACTION 0x81
#define NCI_PF_BIT_RATE 0x18
#define NCI_PROTOCOL_ISO_DEP 0x04
#define NCI_PROTOCOL_NFC_DEP 0x05
#define NCI_NFC_A_ACTIVE_POLL_MODE 0x03

#define NCI_SDD_VALUE 0x30
#define NCI_PLATFORM_CONFIG 0x31
#define NCI_LA_SEL_INFO 0x32
#define NCI_NFC_ID 0x33
#define NCI_LB_SENSEB_INFO 0x38
#define NCI_LF_PROTOCOL_TYPE 0x50
#define NCI_LF_CON_BITR_F 0x54
#define NCI_LI_BIT_RATEC 0x5B
#define NCI_LN_WT 0x60
#define NCI_LN_ATR_RES_GEN_BYTES 0x61
#define NCI_LA_PASSIVE 0x80
#define NCI_LB_PASSIVE 0x81
#define NCI_RF_NFCDEP_OP 0x82
#define NCI_A_ACTIVE_LISTEN_MODE 0x83
//FIELD INFO
#define NTF_RF_FIELD_ON 0x01
#define NTF_RF_FIELD_OFF 0x00
//RF DEACTIVATE MODE
#define IDEL_MODE 0x00
#define SLEEP_MODE 0x01
#define SLEEP_AF_MODE 0x02
#define DISCOVERY_MODE 0x03
#define CREATE_LOCAL_M1 0x0B
#define DELETE_LOCAL_M1 0x0C
#define QUERY_LOCAL_M1 0x0D
#define ACTIVATE_LOCAL_M1 0x0E
#define DEACTIVE_LOCAL_M1 0x0F
#define WRITE_M1_SECTOR 0x10
#define READ_M1_SECTOR 0x11
#define RESET_M1_SECTOR 0x12

//寻卡类别、方式参数设置
typedef struct _APUD_TRAN
{
	uint8_t LEN;
	uint8_t TYPE;
	uint8_t DATA[256];
} SL_APUD_TRAN;

//NCI 包组成：MT(3bit)+PBF(1bit)+GID(4bit)+OID(8bit)+payloadlen(8bit)+payload(nbits)
typedef struct _COMMON_NCI_PACKET
{
	uint8_t MT;
	uint8_t PBF;
	uint8_t GID;
	uint8_t OID;
	uint8_t PAYLOAD_LEN;
	uint8_t PAYLOAD[256];
} COMMON_NCI_PACKET;

#define PROTOCOL_UNDETERMINED 0x00
#define PROTOCOL_ISO_DEP 0x04
#define PROTOCOL_NFC_DEP 0x05

#define NCI_NFC_A_PASSIVE_POLL_MODE 0x00

typedef enum
{
	//返回值正确，为0
	NO_ERR = 0x00, //成功
	//定义NCI协议层错误
	MT_ERROR		 = 0x01,
	PBF_ERROR		 = 0x02,
	GID_ERROR		 = 0x03,
	OID_ERROR		 = 0x04,
	DECODE_LEN_ERROR = 0x05,
	ENCODE_LEN_ERROR = 0x06,
	//6320应用层错误
	CORE_RST_ERROR		  = 0x20,
	CORE_INT_ERROR		  = 0x21,
	CORE_SELECT_ERROR	  = 0x22,
	CORE_APDU_ERROR		  = 0x23,
	CORE_CRTL_REG_ERROR	  = 0x24,
	CORE_SET_CONFIG_ERROR = 0x25,
	RF_DISCOVER_MAP_ERROR = 0x26,
	//升级错误
	DOWN_START_ERROR = 0x40,
	DOWN_DATA_ERROR	 = 0x41,
	DOWN_STOP_ERROR	 = 0x42,

	MEM_ERROR = 0xE0,

	CMD_RECE_OK	  = 0xF2, //NCI命令接收成功
	CMD_TIME_OUT  = 0xF3, //NCI命令超时
	CMD_RECE_WAIT = 0xF4, //NCI命令接收等待
} t_RetStatus;

typedef enum
{
	DO_OK	   = 0x00, //处理成功
	DO_FAIL	   = 0x01, //处理失败
	DO_TIMEOUT = 0x02, //处理超时
	DO_6105	   = 0x03, //收到6105消息
	DO_6103	   = 0x04, //收到6103消息
} t_DoStatus;

/*
* RF Protocols
* Table 133, NFCForum-TS-NCI-2.1
*/
typedef enum
{
	NCI_RF_PROTOCOL_UNDETERMINED = 0x00,
	NCI_RF_PROTOCOL_T1T			 = 0x01,
	NCI_RF_PROTOCOL_T2T			 = 0x02,
	NCI_RF_PROTOCOL_T3T			 = 0x03,
	NCI_RF_PROTOCOL_ISO_DEP		 = 0x04, //T4T  TYPEA CPU  TYPEB CPU
	NCI_RF_PROTOCOL_NFC_DEP		 = 0x05,
	NCI_RF_PROTOCOL_T5T			 = 0x06,
	NCI_RF_PROTOCOL_NDEF		 = 0x07, ///< no support
	// RFU                         0x08 - 0x7F
	NCI_RF_PROTOCOL_M1_RW = 0x80,
	NCI_RF_PROTOCOL_M1_CE = 0x81,
	// For proprietary use         0x81 - 0xFE
	// RFU                         0xFF
} NCI_RF_PROTOCOL_t;

/*
* RF Interfaces
* Table 134, NFCForum-TS-NCI-2.1
*/
typedef enum
{
	NCI_RF_INTERFACE_NFCEE_DIRECT = 0x00,
	NCI_RF_INTERFACE_FRAME		  = 0x01,
	NCI_RF_INTERFACE_ISO_DEP	  = 0x02,
	NCI_RF_INTERFACE_NFC_DEP	  = 0x03,
	// RFU                          0x04 - 0x05
	NCI_RF_INTERFACE_NDEF = 0x06, ///< no support
	// RFU                          0x07 - 0x7F
	NCI_RF_INTERFACE_M1 = 0x80,
	// For proprietary use          0x81 - 0xFE
	// RFU                          0xFF
} NCI_RF_INTERFACE_t;

/*
* RF Technologies
* Table 130, NFCForum-TS-NCI-2.1
*/
typedef enum
{
	NFC_RF_TECHNOLOGY_A = 0x00,
	NFC_RF_TECHNOLOGY_B = 0x01,
	NFC_RF_TECHNOLOGY_F = 0x02,
	NFC_RF_TECHNOLOGY_V = 0x03,
	// RFU                        0x04 - 0x7F
	// For proprietary use        0x80 - 0xFE
	// RFU                        0xFF
} NCI_RF_Technologies_t;

/*
* RF Technology and Mode
* Table 131, NFCForum-TS-NCI-2.1
*/
typedef enum
{
	NFC_A_PASSIVE_POLL_MODE = 0x00,
	NFC_B_PASSIVE_POLL_MODE = 0x01,
	NFC_F_PASSIVE_POLL_MODE = 0x02,
	NFC_ACTIVE_POLL_MODE	= 0x03,
	// RFU                        0x04 - 0x05
	NFC_V_PASSIVE_POLL_MODE = 0x6,
	// RFU                        0x07 - 0x7F
	NFC_A_PASSIVE_LISTEN_MODE = 0x80,
	NFC_B_PASSIVE_LISTEN_MODE = 0x81,
	NFC_F_PASSIVE_LISTEN_MODE = 0x82,
	NFC_ACTIVE_LISTEN_MODE	  = 0x83,
	// RFU                        0x84 - 0xEF
	// Reserved for Proprietary Technologies in Listen Mode 0xF0 - 0xFF
} NCI_RF_Tech_Mode_t;

/*********************************************************/
#define NCI_NFCID1_MAX_LEN 10
#define NCI_T1T_HR_LEN 2
typedef struct
{
	uint8_t sens_res[2];				/* SENS_RES Response (ATQA). Available after Technology
                          Detection */
	uint8_t nfcid1_len;					/* 4, 7 or 10 */
	uint8_t nfcid1[NCI_NFCID1_MAX_LEN]; /* AKA NFCID1 */
	uint8_t sel_rsp;					/* SEL_RSP (SAK) Available after Collision Resolution */
	uint8_t hr_len;						/* 2, if T1T HR0/HR1 is reported */
	uint8_t hr[NCI_T1T_HR_LEN];			/* T1T HR0 is in hr[0], HR1 is in hr[1] */
} tNCI_RF_PA_PARAMS;
typedef tNCI_RF_PA_PARAMS tNFC_RF_PA_PARAMS;

#define NCI_MAX_SENSB_RES_LEN 12
#define NFC_MAX_SENSB_RES_LEN NCI_MAX_SENSB_RES_LEN
#define NFC_NFCID0_MAX_LEN 4
typedef struct
{
	uint8_t sensb_res_len;					  /* Length of SENSB_RES Response (Byte 2 - Byte 12 or
                            13) Available after Technology Detection */
	uint8_t sensb_res[NFC_MAX_SENSB_RES_LEN]; /* SENSB_RES Response (ATQ) */
	uint8_t nfcid0[NFC_NFCID0_MAX_LEN];
} tNFC_RF_PB_PARAMS;

#define NCI_MAX_SENSF_RES_LEN 18
#define NCI_SENSF_RES_OFFSET_PAD0 8
#define NCI_SENSF_RES_OFFSET_RD 16
#define NCI_T3T_PMM_LEN 8
#define NCI_NFCID2_LEN 8
#define NCI_SYSTEMCODE_LEN 2
#define NCI_RF_F_UID_LEN NCI_NFCID2_LEN
#define NCI_MRTI_CHECK_INDEX 13
#define NCI_MRTI_UPDATE_INDEX 14
#define NFC_MAX_SENSF_RES_LEN NCI_MAX_SENSF_RES_LEN
#define NFC_NFCID2_LEN NCI_NFCID2_LEN
typedef struct
{
	uint8_t bit_rate;						  /* NFC_BIT_RATE_212 or NFC_BIT_RATE_424 */
	uint8_t sensf_res_len;					  /* Length of SENSF_RES Response (Byte 2 - Byte 17 or
                            19) Available after Technology Detection */
	uint8_t sensf_res[NFC_MAX_SENSF_RES_LEN]; /* SENSB_RES Response */
	uint8_t nfcid2[NFC_NFCID2_LEN];			  /* NFCID2 generated by the Local NFCC for
                                     NFC-DEP Protocol.Available for Frame
                                     Interface  */
	uint8_t mrti_check;
	uint8_t mrti_update;
} tNFC_RF_PF_PARAMS;

typedef struct
{
	uint8_t nfcid2[NCI_NFCID2_LEN]; /* NFCID2 generated by the Local NFCC for
                                     NFC-DEP Protocol.Available for Frame
                                     Interface  */
} tNCI_RF_LF_PARAMS;
typedef tNCI_RF_LF_PARAMS tNFC_RF_LF_PARAMS;

/* NCI 2.0 - Begin */
#define NFC_V_UID_LEN 8
typedef struct
{
	uint8_t flag;
	uint8_t dsfid;
	uint8_t uid[NFC_V_UID_LEN];
} tNFC_RF_PV_PARAMS;
/* NCI 2.0 - End */

/* NCI 2.0 - Begin */
#define NCI_MAX_ATR_REQ_LEN 64
#define NFC_MAX_ATR_REQ_LEN NCI_MAX_ATR_REQ_LEN
#ifndef NCI_MAX_GEN_BYTES_LEN
#define NCI_MAX_GEN_BYTES_LEN 48
#endif
typedef struct
{
	uint8_t atr_res_len;				  /* Length of ATR_RES Response (from Byte 3) Available
                          after Technology Detection */
	uint8_t atr_res[NFC_MAX_ATR_REQ_LEN]; /* ATR_RES Response */

	uint8_t max_payload_size;				  /* 64, 128, 192 or 254          */
	uint8_t gen_bytes_len;					  /* len of general bytes         */
	uint8_t gen_bytes[NCI_MAX_GEN_BYTES_LEN]; /* general bytes                */
	uint8_t waiting_time;					  /* WT -> Response Waiting Time
                                               RWT = (256 x 16/fC) x 2WT    */
} tNFC_RF_PACM_PARAMS;
typedef struct
{
	uint8_t atr_req_len;				  /* Length of ATR_REQ Command Available after Technology
                          Detection */
	uint8_t atr_req[NFC_MAX_ATR_REQ_LEN]; /* ATR_REQ Response */

	uint8_t max_payload_size;				  /* 64, 128, 192 or 254          */
	uint8_t gen_bytes_len;					  /* len of general bytes         */
	uint8_t gen_bytes[NCI_MAX_GEN_BYTES_LEN]; /* general bytes                */

} tNFC_RF_LACM_PARAMS;
/* NCI 2.0 - End */

typedef union
{
	tNFC_RF_PA_PARAMS pa;
	tNFC_RF_PB_PARAMS pb;
	tNFC_RF_PF_PARAMS pf;
	tNFC_RF_LF_PARAMS lf;
	/* NCI 2.0 - Begin */
	tNFC_RF_PV_PARAMS pv;
	/* NCI 2.0 - End */
	/* NCI 2.0 - Begin */
	tNFC_RF_PACM_PARAMS pacm;
	tNFC_RF_LACM_PARAMS lacm;
	/* NCI 2.0 - End */
} tNFC_RF_TECH_PARAMU;

//the tech parameter of RF_INTF_ACTIVATED_NTF and RF_DISCOVER_NTF
typedef struct
{
	uint8_t mode;
	tNFC_RF_TECH_PARAMU param;
} tNFC_RF_TECH_PARAMS;

typedef struct
{
	uint16_t SENS_RES;	 //ATQA
	uint8_t NFCID_LEN;	 //uid长度
	uint8_t NFCID[10];	 //uid最多10字节
	uint8_t SEL_RES_LEN; //SAK长度 0 1
	uint8_t SEL_RES;	 //SAK
	uint8_t HRx_LEN;	 //HRx长度 0 2
	uint8_t HRx[2];		 //HRx
} NFC_A_Poll_Para;

typedef struct
{
	uint8_t RF_Discovery_ID;
	NCI_RF_INTERFACE_t RF_Interface;
	NCI_RF_PROTOCOL_t RF_Protocol;
	uint8_t RF_Technology_Mode;
	uint8_t Max_Payload_Size;
	uint8_t Initial_Number_Credits;
	uint8_t Length_Parameters;
	tNFC_RF_TECH_PARAMS RF_Technology_Para; //void  *RF_Technology_Para;
	uint8_t Data_Exchange_Mode;
	uint8_t Data_Transmit_Bit_Rate;
	uint8_t Data_Receive_Bit_Rate;
	uint8_t Length_Activation_Para;
	void *Activation_Para;
	uint8_t uid_length;
	uint8_t uid[10];
} CARD_PARA;

#define STREAM_TO_ARRAY(a, p, len)      \
	{                                   \
		register int ijk;               \
		for (ijk = 0; ijk < len; ijk++) \
			((uint8_t *)a)[ijk] = *p++; \
	}

#define TARGET_TYPE_UNKNOWN (-1)
#define TARGET_TYPE_ISO14443_3A 1
#define TARGET_TYPE_ISO14443_3B 2
#define TARGET_TYPE_ISO14443_4 3
#define TARGET_TYPE_FELICA 4
#define TARGET_TYPE_V 5
#define TARGET_TYPE_NDEF 6
#define TARGET_TYPE_NDEF_FORMATABLE 7
#define TARGET_TYPE_MIFARE_CLASSIC 8

/* Discovery Types/Detected Technology and Mode */
#define NCI_DISCOVERY_TYPE_POLL_A 0x00
#define NCI_DISCOVERY_TYPE_POLL_B 0x01
#define NCI_DISCOVERY_TYPE_POLL_F 0x02
#define NCI_DISCOVERY_TYPE_POLL_V 0x06
#define NCI_DISCOVERY_TYPE_POLL_A_ACTIVE 0x03
#define NCI_DISCOVERY_TYPE_POLL_ACTIVE 0x03
#define NCI_DISCOVERY_TYPE_POLL_F_ACTIVE 0x05
#define NCI_DISCOVERY_TYPE_LISTEN_A 0x80
#define NCI_DISCOVERY_TYPE_LISTEN_B 0x81
#define NCI_DISCOVERY_TYPE_LISTEN_F 0x82
#define NCI_DISCOVERY_TYPE_LISTEN_A_ACTIVE 0x83
#define NCI_DISCOVERY_TYPE_LISTEN_ACTIVE 0x83
#define NCI_DISCOVERY_TYPE_LISTEN_F_ACTIVE 0x85
#define NCI_DISCOVERY_TYPE_LISTEN_ISO15693 0x86
#define NCI_DISCOVERY_TYPE_LISTEN_V 0x86
#define NCI_DISCOVERY_TYPE_FIELD_DETECT 0xFF

/* Discovery Types/Detected Technology and Mode */
#define NFC_DISCOVERY_TYPE_POLL_A NCI_DISCOVERY_TYPE_POLL_A
#define NFC_DISCOVERY_TYPE_POLL_B NCI_DISCOVERY_TYPE_POLL_B
#define NFC_DISCOVERY_TYPE_POLL_F NCI_DISCOVERY_TYPE_POLL_F
#define NFC_DISCOVERY_TYPE_POLL_A_ACTIVE NCI_DISCOVERY_TYPE_POLL_A_ACTIVE
#define NFC_DISCOVERY_TYPE_POLL_F_ACTIVE NCI_DISCOVERY_TYPE_POLL_F_ACTIVE
#define NFC_DISCOVERY_TYPE_POLL_ACTIVE NCI_DISCOVERY_TYPE_POLL_ACTIVE
#define NFC_DISCOVERY_TYPE_POLL_V NCI_DISCOVERY_TYPE_POLL_V
#define NFC_DISCOVERY_TYPE_LISTEN_A NCI_DISCOVERY_TYPE_LISTEN_A
#define NFC_DISCOVERY_TYPE_LISTEN_B NCI_DISCOVERY_TYPE_LISTEN_B
#define NFC_DISCOVERY_TYPE_LISTEN_F NCI_DISCOVERY_TYPE_LISTEN_F
#define NFC_DISCOVERY_TYPE_LISTEN_A_ACTIVE NCI_DISCOVERY_TYPE_LISTEN_A_ACTIVE
#define NFC_DISCOVERY_TYPE_LISTEN_F_ACTIVE NCI_DISCOVERY_TYPE_LISTEN_F_ACTIVE
#define NFC_DISCOVERY_TYPE_LISTEN_ACTIVE NCI_DISCOVERY_TYPE_LISTEN_ACTIVE
#define NFC_DISCOVERY_TYPE_LISTEN_ISO15693 NCI_DISCOVERY_TYPE_LISTEN_ISO15693
/* NCI 2.0 - Begin */
#define NFC_DISCOVERY_TYPE_LISTEN_V NCI_DISCOVERY_TYPE_LISTEN_V
/* NCI 2.0 - End */
#define NFC_INFO_LEN (34)

#define BUFFER_BLOCK_SIZE (256 + 3)

extern NFC_A_Poll_Para NfcAPollMode;

extern uint8_t gCurrentTagIndex; //当前正在处理的卡片
extern CARD_PARA NfcCardinfo[3];
extern uint8_t g_NeedNextPoll;
uint16_t write_buffer(uint8_t *data, uint16_t len);

t_RetStatus Nci_wrap(uint8_t *data, COMMON_NCI_PACKET *const aptncipackdecode);
t_RetStatus Nci_unwrap(COMMON_NCI_PACKET atpncipackencode, uint8_t mode);
uint16_t NfcCmdI2cRecvData(uint8_t *pbuf, uint16_t nBufferLen);
t_RetStatus Process_card_para_6103(CARD_PARA *CardPara, COMMON_NCI_PACKET *nci_recv_packet);
t_RetStatus Process_card_para_6105(CARD_PARA *CardPara, COMMON_NCI_PACKET *nci_recv_packet);

void Nci_Rsp_Packet(COMMON_NCI_PACKET *nci_recv_packet);
void Nci_Ntf_Packet(COMMON_NCI_PACKET *nci_recv_packet);
t_RetStatus Nci_unwrap_fido(COMMON_NCI_PACKET nci_send_packet, uint8_t mode);

#endif
