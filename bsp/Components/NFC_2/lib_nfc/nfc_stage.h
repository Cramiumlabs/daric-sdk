#ifndef _NFC_STAGE_H
#define _NFC_STAGE_H
#include "stdint.h"
#include "nfc_config.h"

#define NFCC_HCI_SUPPORT 1
#define NFCC_LOCAL_M1_SUPPORT 1
#define NFCC_ESE_SUPPORT 1
#define NFCC_SURPER_SIM_SUPPORT 1
#define NFCC_RW_SUPPORT 1
#define NFCC_HCE_SUPPORT 1

#define NFCC_MAX_TIMEOUT_MS	6000
/**接口层的结果 */
typedef enum
{
	NFCC_RESULT_SUCCESS	 = 0x00,
	NFCC_RESULT_FAILED	 = 0x01,
	NFCC_RESULT_TIMEOUT	 = 0x02,
	NFCC_RESULT_PARA_ERR = 0x03,
	NFCC_RESULT_DH_GRAMMAR_ERR = 0x04,
	NFCC_RESULT_DH_SEMANTIC_ERR = 0x05,
	NFCC_UNKNOWN_ERROR = 0xff
} t_NFCC_Result;

typedef enum
{
	RFST_IDLE = 0,
	RFST_DISCOVERY,
	RFST_LISTEN_ACTIVE,
	RFST_LISTEN_SLEEP,
	RFST_W4_ALL_DISCOVERIES,
	RFST_POLL_ACTIVE,
	RFST_W4_HOST_SELECT,
} t_NFCC_rf_Status;

typedef enum
{
	NFCC_STATUS_IDLE = 0x0,
	NFCC_STATUS_DISCOVERY_CE,
	NFCC_STATUS_DISCOVERY_RW,
	NFCC_STATUS_ACTIVE_CE,
	NFCC_STATUS_ACTIVE_RW,
	// NFCC_STATUS_IN_FIELD,
	// NFCC_STATUS_OUT_FIELD,
} t_NFCC_Status;

#define MODE_NFC_CE 0x01
#define MODE_NFC_RW 0x02

// #define MODE_NFC  MODE_NFC_CE|MODE_NFC_RW
#define MODE_NFC MODE_NFC_CE
#define MODE_CE 0x81

typedef uint8_t tStatus;
#define STATUS_OK 0 //ok
#define STATUS_REJECTED 0x01 //rejected
#define STATUS_FAILED 0x03 //failed
#define STATUS_NOT_INITIALIZED 0x04 //not initialized
#define STATUS_SYNTAX_ERROR 0x05 //syntax error
#define STATUS_SEMANTIC_ERROR 0x06 //semantic error
#define STATUS_INVALID_PARAM 0x09 //invalid para
#define STATUS_MSG_SIZE_TOO_BIG 0x0A //msg size too big
#define STATUS_NOT_SUPPORTED 0x0B //not supported
#define STATUS_TIMEOUT 0xB2 //time out
#define STATUS_BUFFER_FULL 0xE0 //buff full,over flow
#define STATUS_LOCK_UNBUSY 0xE1 //nfc process is unbusy, the nfc process has been entered
#define STATUS_LOCK_BUSY 0xE2 //nfc process is busy, cannot enter into the nfc process
#define STATUS_BAD_LENGTH 0xFD //bad length

//LocalM1
#define STATUS_APP_NOT_EXISTED 0xD1 // activation executed
#define STATUS_ACTIVATION_EXECUTED 0xD2 // activation executed
#define STATUS_DEACTIVATION_NOT_EXECUTED 0xD3 // activation executed
#define STATUS_DEACTIVATION_EXECUTED 0xD4 // activation executed
#define STATUS_NOT_AUTHORIZED 0xD5 //NOT AUTH

#define NFC_MODE_CE_SIM 0
#define NFC_MODE_CE_LOCALM1 1
#define NFC_MODE_READER 2

#define UICC1_HOST ((unsigned char)0xC0)
#define UICC2_HOST ((unsigned char)0x81)
#define UICC3_HOST ((unsigned char)0x82)
#define DH_HOST ((unsigned char)0x00)
#define LOCAL_M1 ((unsigned char)0x10)
#define ESE_HOST UICC1_HOST
#define SUPER_SIM_HOST UICC2_HOST
#define SUPER_SIM2_HOST UICC3_HOST

#define NO_TIMEOUT_FOREVER 0xffff

#define NCI_SINGLE_FRAME_MAX_LEN 253 //单帧最大长度 保留CRC两字节
#define APP_MAX_DATA_SIZE 0x800 //2k
#define FIDO_NCI_SINGLE_FRAME_MAX_LEN 251 //fido

typedef enum
{
	NCI_ACTIVE = 0,
	//
	NCI_HED_UPDATE_FLASH,

	//core
	NCI_CORE_RESET,
	NCI_CORE_RESET_DW,
	NCI_CORE_INIT,
	NCI_CORE_GET_CONFIG_FOR_RF_WTX_TIME,
	NCI_CORE_CONFIG_FOR_NFCEE_ACTION,
	NCI_CORE_CONFIG_FOR_SESSIONID_ESE,
	NCI_CORE_CONFIG_FOR_SESSIONID_SIM,
	NCI_CORE_CONFIG_FOR_NFCC,
	NCI_CORE_CONFIG_FOR_FIELD_INFO,
	NCI_CORE_CONFIG_FOR_POWER_SUB,
#if NFCC_SOC2_SUPPORT
	NCI_CORE_CONFIG_FOR_GD_PHASE,
#else
	NCI_CORE_CONFIG_FOR_PHASE,
#endif
#if NFCC_USE_SDK_RF_PARA
	NCI_CORE_CONFIG_FOR_RF_PARA,
#endif
#if DCDC_SUPPORT
	NCI_CORE_CONFIG_FOR_RF_DCDC,
#endif

#if NFCC_SOC2_SUPPORT
	NCI_CORE_CONFIG_FOR_CE_DLMA, /**设置DLMA输出功率，需要小于Vbat 0.3V，否则影响功耗 */
#endif
	NCI_CORE_CONFIG_FOR_SUPERSIM, /** */
#if NFCC_FW_CONFIG_NFCC_PARA
	NCI_CORE_CONFIG_FOR_ENABLE_RF_SET_PARA, /**打开固件非接调试射频参数功能 */
#else
	NCI_CORE_CONFIG_FOR_DISABLE_RF_SET_PARA, /**关闭固件非接调试射频参数功能 */
#endif
	NCI_CORE_CONFIG_FOR_DISABLE_RAW_DATA, /**关闭RAW DATA */
	NCI_CORE_GET_CONFIG,

	//RF
	NCI_RF_DISCOVER_MAP,
	NCI_RF_SET_ROUTE,
	NCI_RF_DISCOVER,

	//CMD FOR RF STATUS
	NCI_RF_DEACTIVE_IDEL,			/**RSP */
	NCI_RF_DEACTIVE_IDEL_PLUS,		/**RSP/NTF */
	NCI_RF_DEACTIVE_SLEEP,			/**RSP */
	NCI_RF_DEACTIVE_SLEEP_PLUS,		/**RSP/NTF */
	NCI_RF_DEACTIVE_SLEEP_AF,		/**RSP */
	NCI_RF_DEACTIVE_SLEEP_AF_PLUS,	/**RSP/NTF */
	NCI_RF_DEACTIVE_DISCOVERY,		/**RSP */
	NCI_RF_DEACTIVE_DISCOVERY_PLUS, /**RSP/NTF */

	//NFCEE
	NCI_NFCEE_DISCOVER,
#if NFCC_ESE_SUPPORT
	NCI_NFCEE_ESE_ENABLE,
	NCI_NFCEE_ESE_DISABLE,
#endif
#if NFCC_SURPER_SIM_SUPPORT
	NCI_NFCEE_SUPERSIM_ENABLE,
	NCI_NFCEE_SUPERSIM_DISABLE,
	NCI_NFCEE_POWER,
#endif

#if NFCC_HCI_SUPPORT
	NCI_HCI_CREATE_ESE_PIPE, /** 拿到PIPE号，后期来使用*/
	NCI_HCI_CREATE_SURPER_SIM_PIPE,
	NCI_HCI_OPEN_ESE_PIPE,		  /** */
	NCI_HCI_GET_PARA_ESE_APDUGATE_WTX,		  /** */
	NCI_HCI_GET_PARA_ESIM_APDUGATE_WTX,		  /** */
	NCI_HCI_OPEN_SUPER_SIM_PIPE,  /** */
	NCI_HCI_CLOSE_ESE_PIPE,		  /** */
	NCI_HCI_CLOSE_SUPER_SIM_PIPE, /** */
	NCI_HCI_TRANS_APDU,	  /** */
	NCI_HCI_TRANS_APDU_NEED_NEXT,	  /** */
	NCI_HCI_CLEAR_PIPE,			  /** */
	NCI_HCI_SET_PARA,			  /** */
	NCI_HCI_SET_WHITE,			  /** */
	NCI_HCI_RESPONESE_OK,
	NCI_HCI_WAKUP,
	NCI_HCI_END_TRANSACTION,
#endif

#if NFCC_HCE_SUPPORT
	NCI_HCE_TRANSTION,
#endif

#if NFCC_LOCAL_M1_SUPPORT
	NCI_NFCEE_LOCAL_M1_ENABLE,
	// NFCC_CORE_CONFIG_FOR_LOCAL_M1,
	NCI_LOCAL_M1_QUERY,
	NCI_LOCAL_M1_CREATE,
	NCI_LOCAL_M1_DELETE,
	NCI_LOCAL_M1_ACTIVATE,
	NCI_LOCAL_M1_DEACTIVE,
	NCI_LOCAL_M1_W_SECTOR,
	NCI_LOCAL_M1_R_SECTOR,
#endif

#if NFCC_RW_SUPPORT
	NCI_RW_PROCESS_NEED_NEXT,
	NCI_RW_PROCESS,
	NCI_RW_SELECT_CARD,
	NCI_RW_AUTH_M1_SECTOR,
	NCI_RW_READ_M1_SECTOR,
	NCI_RW_WRITE_M1_SECTOR,
#endif

#if NFCC_SOC2_SUPPORT
	NCI_LPCD_CHECK_CARD,
#endif

	//test
	NCI_CMD_END,
} NCI_CMD_INDEX;

/**底层状态机机制 */

typedef struct _T_NFC_INFO
{
	uint8_t version[2];
	t_NFCC_Status nfcc_status; /**全局指示NFCC状态*/
	uint8_t ese_gate_pipe;
	uint8_t supersim_gate_pipe;
	uint8_t LocalM1IsExit; //bit0 app1 bit1 app2 bit2 app3 bit3 app4
	uint8_t LocalM1IsAcitive;
	uint8_t LocalM1ActiveAppId;
	bool SuperSimGateIsReady;
	uint8_t super_sim_uid[10];
	uint8_t super_sim_uid_size;
	bool EseGateIsReady;
	uint8_t ese_uid[10];
	uint8_t ese_uid_size;
	uint8_t hce_aid[8];
	uint8_t hce_aid_size;
	t_NFCC_rf_Status nfcc_status_m;
} t_NFCC_Info;

//参照NCI规范， 路由参数表,需要依照规范来进行赋值初始化
typedef struct _T_ROUT_TABLE_PRA
{
	uint8_t qualifier_type; //
	uint8_t power_state;	//功耗模式同NCI规范的mask
	uint8_t technology;		//技术
	uint8_t protocol;		//协议
	uint8_t nfcee_id;		//NFCEEID
	uint8_t aid[16];		//AID
	uint8_t aid_len;		//AID
	uint8_t sys_code[64];	//SystemCode
	uint8_t sys_code_len;	//SystemCode
} t_RoutTablePra;

typedef struct _T_NFC_CONFIG
{
	uint8_t nfc_mode; /***BIT0 CE mode  BIT1 RW mode    */
	uint8_t ce_route; /***0xc0/0x81/0x00    */
} t_NFCC_Config;

typedef enum
{
	NFCC_CMD_RESET = 0,
	NFCC_CMD_REGISTER_CB,
	NFCC_CMD_CHANGE_ROUT,
	NFCC_CMD_POWDOWN_CE,
	NFCC_CMD_OPEN_RW,
	NFCC_CMD_CLOSE_RW,
	NFCC_CMD_SEND_APDU_GATE,
	NFCC_CMD_SURPER_SIM_IN, //热插拔

#if NFCC_LOCAL_M1_SUPPORT
	NFCC_CMD_LOCAL_M1_CREATE,
	NFCC_CMD_LOCAL_M1_WRITE_SECTOR,
	NFCC_CMD_LOCAL_M1_READ_SECTOR,
	NFCC_CMD_LOCAL_M1_CHANGE,
	NFCC_CMD_LOCAL_M1_DELETE,
	NFCC_CMD_LOCAL_M1_QUERY,
#endif

	NFCC_CMD_GET_SURPERSIM_UID,
	NFCC_CMD_GET_ESE_UID,
	NFCC_CMD_SELECT_CARD,
	NFCC_CMD_RW_PROCESS,
	NFCC_CMD_RE_DISCOVERY,
	NFCC_CMD_HCE_RESPONSE,
    NFCC_CMD_HCE_FIDO_RESPONSE,

	NFCC_CMD_HCI_SIM_ENABLE,
	NFCC_CMD_HCI_SE_ENABLE,

	NFCC_CMD_END,
} NFCC_CMD_INDEX;

typedef enum
{
	/**T1T */
	NFCC_RW_T1T_RID = 0x78,
} NFCC_CMD_RW_T1T_INDEX;
typedef enum
{
	/**T2T */
	NFCC_RW_T2T_READ		  = 0x30,
	NFCC_RW_T2T_WRITE		  = 0xA2,
	NFCC_RW_T2T_SECTOR_SELECT = 0xC2,
} NFCC_CMD_RW_T2T_INDEX;

typedef enum
{
	/**T5T */
	NFCC_RW_T5T_READ_SINGLE	 = 0x20,
	NFCC_RW_T5T_WRITE_SINGLE = 0x21,
	NFCC_RW_T5T_LOCK_SINGLE	 = 0x22,

	NFCC_RW_T5T_READ_MULTI = 0x23,

	NFCC_RW_T5T_EXTENDED_READ_SINGLE  = 0x30,
	NFCC_RW_T5T_EXTENDED_WRITE_SINGLE = 0x31,
	NFCC_RW_T5T_EXTENDED_LOCK_SINGLE  = 0x32,

	NFCC_RW_T5T_EXTENDED_READ_MULTI = 0x33,

} NFCC_CMD_RW_T5T_INDEX;

typedef enum
{
	/**M1 */
	NFCC_RW_M1_AUTH	 = 0x5F,
	NFCC_RW_M1_READ	 = 0x30,
	NFCC_RW_M1_WRITE = 0xA0,
} NFCC_CMD_RW_M1_INDEX;

typedef struct _T_NFCC_API_CMD
{
	uint8_t cmd_start_flag;	  //
	uint8_t cmd_end_flag;	  //
	uint8_t cmd;			  //
	t_NFCC_Result cmd_status; //
	uint8_t para1;
	uint8_t *p_in_data;	   //apdu gate 和rw mode使用
	uint16_t in_data_len;  //
	uint8_t *p_out_data;   //apdu gate 和rw mode使用
	uint16_t out_data_len; //
	uint16_t out_data_max_size;
	void *impl_intf;
	uint32_t UserTimeoutMask; /*0xFFFFFFFF:Return immediately after sending successfully; other: user default timeout; */
	bool is_fido;
} t_NFCC_API_cmd;

typedef struct _NFC_CMD_LIST
{
	NCI_CMD_INDEX num;							/**序列*/
	void (*packetSendFunc)(void);				/**打包数据函数*/
	void (*CallbackPartFunc)(COMMON_NCI_PACKET *p); /**接收部分数据，即遇到PBF为1时，级联数据的回调函数*/
	void (*CallbackCpltFunc)(COMMON_NCI_PACKET *p); /**接收完成后的回调函数*/
	uint8_t CmdMode;							/**0:不需要等待应答和NTF；1：需要等待应答，不需要等待NTF；2：需要等待应答和NTF */
	uint32_t timeout;							/**需要根据指令的执行时间进行设置 */
} NFC_CMD_LIST;

typedef enum {
	SDK_CB_EVENT_NFCC_FAULT,
}SDK_CB_EVENT_t;

typedef void (*tSdkEventCallback)( SDK_CB_EVENT_t event);
typedef struct {
    tSdkEventCallback  sdk_callback_fun_handle_event;
}sdk_callback_info_t;


typedef struct {
	tSdkEventCallback sdk_callback_fun_handle_event;
	uint32_t RF_wtx_time_ms;
	uint16_t HCI_ese_apdugate_wtx_time_ms;
	uint16_t HCI_esim_apdugate_wtx_time_ms;
}tNFC_dm;

extern t_NFCC_Info g_NfcInfo;
extern t_NFCC_API_cmd g_apiCmd;
extern t_NFCC_Config g_nfc_config;
extern tNFC_dm g_nfc_dm;

/**
 * 
 * NFCC主动上送的NTF解析
 * NFCC主动上送 IRQ会拉高，之后需要调用此函数
 * 
 * 
 * 
 */
uint16_t NFCC_Poll(void); //处理NTF,

/**
 * 下载固件相关接口
 * 
 */
t_NFCC_Result NFCC_ForceFwdl(void);
//返回值 1 正在下载 2 下载失败 0 下载成功
uint8_t NFCC_GetFwdlStatus(void);

/**
 * 管理NFCC相关接口
 * 
 */
char *NFCC_GetVersion(void);
t_NFCC_Result NFCC_Init(void);
t_NFCC_Result NFCC_register_callback(sdk_callback_info_t *sdk_callback);
#if NFCC_SOC2_SUPPORT
t_NFCC_Result NFCC_factoryInit(void); //接口，在无干扰环境下调用一次，用于校准LPCD
#endif

t_NFCC_Result NFCC_SetRouteList(t_RoutTablePra *pRouteList, uint8_t num); //通用接口，客户不调用；客户使用NFCC_ChangeRoute
t_NFCC_Result NFCC_ChangeRoute(uint8_t ucRoute, uint8_t *Aid);			  //接口  改变路由功能
t_NFCC_Result NFCC_PowDownCe(void);										  //接口  关机卡模拟前调用
t_NFCC_Result NFCC_DeInit(void);										  //接口  关闭NFCC功能
t_NFCC_Result NFCC_OpenRwMode(void);									  //接口  打开读写功能
t_NFCC_Result NFCC_OpenCeMode(void);									  //接口  打开CE功能
void NFCC_SetGlobalStatus(t_NFCC_Status status);
t_NFCC_Status NFCC_GetGlobalStatus(void); //接口 获取全局状态
void NFCC_SetRFGlobalStatus(t_NFCC_rf_Status status);
t_NFCC_rf_Status NFCC_GetRFGlobalStatus(void);				//接口 获取RF全局状态
t_NFCC_Config *NFCC_GetConfig(void);						//接口 获取配置信息
t_NFCC_Result NFCC_GetSuperSimUID(uint8_t *pucSuperSimUid); //接口 获取超级SIM UID
t_NFCC_Result NFCC_GetEseUID(uint8_t *pucEseUid);			//接口 获取ESE UID
bool NFCC_SuperSimIsReady(void);
bool NFCC_EseIsReady(void);

/**
 * HCI apdu_gate接口
 */
t_NFCC_Result NFCC_GateSendApdu(uint8_t route, uint8_t *pucInApduData, uint16_t usInLen, uint8_t *pucRes, uint16_t *pOutLen, uint16_t usOutBufSize, bool type);

/**
 * HCE  客户应用处理接口
 */
uint8_t NFCC_HceCallBack(uint8_t *pInData, uint16_t usInDataLen);
t_NFCC_Result NFCC_HceSendData(uint8_t *pInData, uint16_t usLen);

/**
 * Local M1卡接口
 */
t_NFCC_Result NFCC_LocalM1SetUid(uint8_t app_type, uint32_t Uid); //创建对应UID的卡片
t_NFCC_Result NFCC_LocalM1WriteSector(uint8_t app_type, uint8_t sector_number, uint16_t block_bit_map, uint8_t *p_password, uint8_t *p_in_data_lv);
t_NFCC_Result NFCC_LocalM1ReadSector(uint8_t app_type, uint8_t sector_number, uint16_t block_bit_map, uint8_t *p_password, uint8_t *p_out_data_lv);
t_NFCC_Result NFCC_LocalM1CardChange(uint8_t app_type);				  //切换卡片
t_NFCC_Result NFCC_LocalM1Delete(uint8_t app_type);					  //删除卡片
t_NFCC_Result NFCC_LocalM1Query(uint8_t app_type, uint8_t *pOutData); //查询卡片信息

/**
 * RW mode Mifare卡接口
 * 
 */
t_NFCC_Result NFCC_RwMifareAuth(uint8_t type, uint8_t sector_id, uint8_t *p_in_key);
t_NFCC_Result NFCC_RwMifareReadBlock(uint8_t block_id, uint8_t *pOutData);
t_NFCC_Result NFCC_RwMifareWriteBlock(uint8_t block_id, uint8_t *p_in_buf);
t_NFCC_Result NFCC_RwWaitCard(NCI_RF_PROTOCOL_t CardProtocol, uint16_t Timeout); //接口 等待卡片
// void NFCC_RfData(uint8_t *p_in_buf, uint8_t len);
t_NFCC_Result NFCC_RwSelect(uint8_t card_num);
t_NFCC_Result NFCC_RwReDiscovery(void);
t_NFCC_Result NFCC_RwTransceive(uint8_t *pInBuf, uint16_t ucInLen, uint8_t *pOutRes, uint16_t *pOutLen, uint32_t TimeoutMask);			//通用发送接收接口
t_NFCC_Result NFCC_RwT4tApduCmd(uint8_t *pInApduBuf, uint8_t ucInLen, uint8_t *pOutApduRes, uint16_t *pOutLen); //type A cpu card rw
t_NFCC_Result NFCC_RwT2tReadBlock(uint8_t block_id, uint8_t *pOutData);											//type A T2T  rw
t_NFCC_Result NFCC_RwT2tWriteBlock(uint8_t block_id, uint8_t *p_in_buf);
CARD_PARA *NFCC_RwGetInfo(void); //获取读到的卡片的信息

/**
 * 复制M1卡
 */
t_NFCC_Result NFCC_CopyToLocalM1(uint8_t app_type);
/**
 * 热插拔接口
 */
t_NFCC_Result NFCC_SuperSimIn(void);

bool start_nfc_thread();

bool start_nfc_rw_thread();
t_NFCC_Result NFCC_HceFIDOSendData(uint8_t *pInData, uint16_t usLen);

#endif
