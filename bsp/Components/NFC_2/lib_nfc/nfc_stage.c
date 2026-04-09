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
#include "nci.h"
#include "nfc_drv.h"
#include "nfc_stage.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-function"

#ifndef CONFIG_TESTSUITE
extern  uint8_t hce_callback(uint8_t *p_data, uint16_t data_length);
#endif

//2F 03 04 5A A5 A5 5A
static void NFCC_Activate(void);
//20 00 01 00
static void NFCC_CoreReset(void);
//20 00 01 01
static void NFCC_CoreResetAfterFwdl(void);
//20 01 02 00 00
static void NFCC_CoreInit(void);
//20 03 03 01 A0 38
static void NFCC_CoreGetConfigForRfWtxTime(void);
//20 01 02 00 00
static void NFCC_CoreConfigForNfceeAction(void);
static void NFCC_CoreConfigForSeSsionEse(void);
static void NFCC_CoreConfigForSeSsionSim(void);
//20 02 21 0A 30 01 04 31 01 00 58 01 C4 32 01 20 38 01 00 50 01 00 85 01 01 00 02 F4 01 02 01 01 A0 07 01 01
static void NFCC_CoreConfigForNfc(void);
//20 02 04 01 80 01 00
static void NFCC_CoreConfigForField(void);
//2F 02 00
static void NFCC_HedUpdateFlash(void);
static void NFCC_CoreConfigForPowSub(void);
static void NFCC_CoreConfigForGdTimeAndPhase(void);
static void NFCC_CoreConfigForPhase(void);

#if NFCC_USE_SDK_RF_PARA
static void NFCC_CoreConfigRfPara(void);
#endif
#if NFCC_SOC2_SUPPORT
static void NFCC_CoreConfigCeDlma(void);
#endif
static void NFCC_CoreConfigForSurperSIM(void);
#if NFCC_FW_CONFIG_NFCC_PARA
static void NFCC_CoreConfigForEnableRfSet_para(void);
#else
static void NFCC_CoreConfigForDisableRfSet_para(void);
#endif
static void NFCC_CoreConfigForDisableRawData(void);
static void NFCC_CoreGetConfig(void);
static void NFCC_RfSetRoute(void);
//static void NFCC_RfSetRouteForHce(void);
static void NFCC_RfDeactiveSleep(void);
static void NFCC_RfDeactiveSleepAf(void);
static void NFCC_RfDeactiveDiscovery(void);
static void NFCC_RfDeactiveIdle(void);
static void NFCC_RfDiscoverMap(void);
static void NFCC_RfDiscover(void);
static void NFCC_NfceeDiscover(void);
#if NFCC_ESE_SUPPORT
static void NFCC_NfceeEseEnable(void);
static void NFCC_NfceeEseDisable(void);
#endif
#if NFCC_SURPER_SIM_SUPPORT
static void NFCC_NfceeSurperSimEnable(void);
static void NFCC_NfceeSurperSimDisable(void);
static void NFCC_NfceePower(void);
#endif

#if NFCC_HCI_SUPPORT
static void NFCC_HciCreateEsePipe(void);
static void NFCC_HciCreateSuperSimPipe(void);
static void NFCC_HciOpenEsePipe(void);
static void NFCC_HciGetEseApdugateWtx(void);
static void NFCC_HciGetEsimApdugateWtx(void);
static void NFCC_HciOpenSuperSimPipe(void);
static void NFCC_HciCloseEsePipe(void);
static void NFCC_HciCloseSuperSimPipe(void);
static void NFCC_HciTransApdu(void);
static void NFCC_HciClearPipe(void);
static void NFCC_HciSetPara(void);
static void NFCC_HciSetWhite(void);
static void NFCC_HciResponseOk(void);
static void NFCC_HciWakeup(void);
static void NFCC_HciEndTranstion(void);
#endif

#if NFCC_HCE_SUPPORT
static void NFCC_HceTranstion(void);
#endif

#if NFCC_LOCAL_M1_SUPPORT
static void NFCC_NfceeLocalM1Enable(void);
static void NFCC_Localm1Query(void);
static void NFCC_Localm1Create(void);
static void NFCC_Localm1Delete(void);
static void NFCC_Localm1Activate(void);
static void NFCC_Localm1WriteSector(void);
static void NFCC_Localm1WriteSectorCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_Localm1ReadSector(void);
static void NFCC_Localm1ReadSectorCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_Localm1Deactive(void);
static t_NFCC_Result NFCC_M1InfoPack(uint8_t app_type, uint8_t *system_data, uint8_t *initial_key_set, uint8_t *res_date);
static t_NFCC_Result NFCC_LocalM1CardInfoChange(uint8_t app_type, uint8_t *pucInData, uint16_t usLen); //create or change info
#endif

#if NFCC_RW_SUPPORT
static void NFCC_RwProcessCallback(COMMON_NCI_PACKET *pData);
static void NFCC_RwProcess(void);
static void NFCC_RwSelectCard(void);

//m1卡
static void NFCC_RwAuthM1Sector(void);
static void NFCC_RwReadM1Sector(void);
static void NFCC_RwWriteM1Sector(void);
//CPU卡
// static void NFCC_RwSelectCard(CARD_PARA *const dataout);
static void NFCC_RwReaderSendData(SL_APUD_TRAN *const dataout, SL_APUD_TRAN *const datain);
#endif
static void NFCC_LpcdCheckCard(void);

static t_RetStatus NFCC_NciCmdProcess_(uint8_t Index);
static t_RetStatus NFCC_NciCmdProcess(uint8_t Index);

static void NFCC_CoreGetConfigForRfWtxTimeCallBack(COMMON_NCI_PACKET *pData);

static void NFCC_RfDiscoverCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_NfceeSurperSimEnableCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_NfceeEseEnableCallback(COMMON_NCI_PACKET *pData);
static void NFCC_RfDeactiveIdleCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_RfDeactiveSleepCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_RfDeactiveDiscoveryCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_HciGetEseApdugateWtxCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_HciGetEsimApdugateWtxCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_HciTransChainingApduCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_HciTransApduCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_CoreGetConfigCallBack(COMMON_NCI_PACKET *pData);
static void NFCC_Localm1QueryCallback(COMMON_NCI_PACKET *pData);
static void NFCC_Localm1ActivateCallback(COMMON_NCI_PACKET *pData);
static void NFCC_Localm1DeactiveCallback(COMMON_NCI_PACKET *pData);
static void NFCC_Localm1DeleteCallback(COMMON_NCI_PACKET *pData);
static void NFCC_Localm1CreateCallback(COMMON_NCI_PACKET *pData);

static void check_hci_pipe_id(COMMON_NCI_PACKET * pack);


NFC_CMD_LIST nfcCmdList[] = {
	{NCI_ACTIVE, NFCC_Activate, NULL, NULL, 1, NCI_ACK_TIMEOUT},
	{NCI_HED_UPDATE_FLASH, NFCC_HedUpdateFlash, NULL, NULL, 1, 2000}, /***/

	{NCI_CORE_RESET, NFCC_CoreReset, NULL, NULL, 2, NCI_ACK_TIMEOUT},						  /**会拿到版本号*/
	{NCI_CORE_RESET_DW, NFCC_CoreResetAfterFwdl, NULL, NULL, 2, NCI_ACK_TIMEOUT},			  /**会拿到版本号*/
	{NCI_CORE_INIT, NFCC_CoreInit, NULL, NULL, 1, NCI_ACK_TIMEOUT},						  /***/
	{NCI_CORE_GET_CONFIG_FOR_RF_WTX_TIME, NFCC_CoreGetConfigForRfWtxTime, NULL, NFCC_CoreGetConfigForRfWtxTimeCallBack, 1, 4000},	/***/
	{NCI_CORE_CONFIG_FOR_NFCEE_ACTION, NFCC_CoreConfigForNfceeAction, NULL, NULL, 1, 4000}, /***/
	{NCI_CORE_CONFIG_FOR_SESSIONID_ESE, NFCC_CoreConfigForSeSsionEse, NULL, NULL, 1, 4000}, /***/
	{NCI_CORE_CONFIG_FOR_SESSIONID_SIM, NFCC_CoreConfigForSeSsionSim, NULL, NULL, 1, 4000}, /***/
	{NCI_CORE_CONFIG_FOR_NFCC, NFCC_CoreConfigForNfc, NULL, NULL, 1, 2000},				  /***/
	{NCI_CORE_CONFIG_FOR_FIELD_INFO, NFCC_CoreConfigForField, NULL, NULL, 1, 2000},		  /***/
	{NCI_CORE_CONFIG_FOR_POWER_SUB, NFCC_CoreConfigForPowSub, NULL, NULL, 1, 2000},		  /***/
#if NFCC_SOC2_SUPPORT
	{NCI_CORE_CONFIG_FOR_GD_PHASE, NFCC_CoreConfigForGdTimeAndPhase, NULL, NULL, 1, 2000}, /***/
#else
	{NCI_CORE_CONFIG_FOR_PHASE, NFCC_CoreConfigForPhase, NULL, NULL, 1, 2000}, /***/
#endif
#if NFCC_USE_SDK_RF_PARA
	{NCI_CORE_CONFIG_FOR_RF_PARA, NFCC_CoreConfigRfPara, NULL, NULL, 1, 2000}, /***/
#endif

#if NFCC_SOC2_SUPPORT
	{NCI_CORE_CONFIG_FOR_CE_DLMA, NFCC_CoreConfigCeDlma, NULL, NULL, 1, 2000}, /***/
#endif
	{NCI_CORE_CONFIG_FOR_SUPERSIM, NFCC_CoreConfigForSurperSIM, NULL, NULL, 1, 2000}, /***/
#if NFCC_FW_CONFIG_NFCC_PARA
	{NCI_CORE_CONFIG_FOR_ENABLE_RF_SET_PARA, NFCC_CoreConfigForEnableRfSet_para, NULL, NULL, 1, 2000}, /***/
#else
	{NCI_CORE_CONFIG_FOR_DISABLE_RF_SET_PARA, NFCC_CoreConfigForDisableRfSet_para, NULL, NULL, 1, 2000}, /***/
#endif
	{NCI_CORE_CONFIG_FOR_DISABLE_RAW_DATA, NFCC_CoreConfigForDisableRawData, NULL, NULL, 1, 2000},   /***/
	{NCI_CORE_GET_CONFIG, NFCC_CoreGetConfig, NULL, NFCC_CoreGetConfigCallBack, 1, 2000},			   /***/
	{NCI_RF_DISCOVER_MAP, NFCC_RfDiscoverMap, NULL, NULL, 1, NCI_ACK_TIMEOUT},					   /***/
	{NCI_RF_SET_ROUTE, NFCC_RfSetRoute, NULL, NULL, 1, 2000},										   /***/
	{NCI_RF_DISCOVER, NFCC_RfDiscover, NULL, NFCC_RfDiscoverCallBack, 1, 2000},					   /***/
	{NCI_RF_DEACTIVE_IDEL, NFCC_RfDeactiveIdle, NULL, NFCC_RfDeactiveIdleCallBack, 1, 1000},		   /* IDLE RF_DEACTIVATE_CMD/RSP **/
	{NCI_RF_DEACTIVE_IDEL_PLUS, NFCC_RfDeactiveIdle, NULL, NFCC_RfDeactiveIdleCallBack, 2, 1000},	   /* IDLE RF_DEACTIVATE_CMD/RSP/NTF **/
	{NCI_RF_DEACTIVE_SLEEP, NFCC_RfDeactiveSleep, NULL, NFCC_RfDeactiveSleepCallBack, 1, 1000},	   /* SLEEP RF_DEACTIVATE_CMD/RSP **/
	{NCI_RF_DEACTIVE_SLEEP_PLUS, NFCC_RfDeactiveSleep, NULL, NFCC_RfDeactiveSleepCallBack, 2, 1000}, /* SLEEP RF_DEACTIVATE_CMD/RSP/NTF **/
	{NCI_RF_DEACTIVE_SLEEP_AF, NFCC_RfDeactiveSleepAf, NULL, NFCC_RfDeactiveSleepCallBack, 1, 1000},
	/* SLEEP_AF RF_DEACTIVATE_CMD/RSP**/ //P2P
	{NCI_RF_DEACTIVE_SLEEP_AF_PLUS, NFCC_RfDeactiveSleepAf, NULL, NFCC_RfDeactiveSleepCallBack, 2, 1000},
	/* SLEEP_AF RF_DEACTIVATE_CMD/RSP/NTF **/															   //P2P
	{NCI_RF_DEACTIVE_DISCOVERY, NFCC_RfDeactiveDiscovery, NULL, NFCC_RfDeactiveDiscoveryCallBack, 1, 1000},	   /* DISCOVERY RF_DEACTIVATE_CMD/RSP**/
	{NCI_RF_DEACTIVE_DISCOVERY_PLUS, NFCC_RfDeactiveDiscovery, NULL, NFCC_RfDeactiveDiscoveryCallBack, 2, 1000}, /* DISCOVERY RF_DEACTIVATE_CMD/RSP/NTF **/

	//nfcee
	{NCI_NFCEE_DISCOVER, NFCC_NfceeDiscover, NULL, NULL, 3, 10000}, /***/
#if NFCC_ESE_SUPPORT
	{NCI_NFCEE_ESE_ENABLE, NFCC_NfceeEseEnable, NULL, NFCC_NfceeEseEnableCallback, 2, 3000}, /** */
	{NCI_NFCEE_ESE_DISABLE, NFCC_NfceeEseDisable, NULL, NULL, 2, 1000},					   /** */
#endif
#if NFCC_SURPER_SIM_SUPPORT
	{NCI_NFCEE_SUPERSIM_ENABLE, NFCC_NfceeSurperSimEnable, NULL, NFCC_NfceeSurperSimEnableCallBack, 2, 3000}, /***/
	{NCI_NFCEE_SUPERSIM_DISABLE, NFCC_NfceeSurperSimDisable, NULL, NULL, 2, 1000},							/***/
	{NCI_NFCEE_POWER, NFCC_NfceePower, NULL, NULL, 1, 1000},													/***/
#endif
#if NFCC_HCI_SUPPORT
	{NCI_HCI_CREATE_ESE_PIPE, NFCC_HciCreateEsePipe, NULL, NULL, 2, 2000},					  /***/
	{NCI_HCI_CREATE_SURPER_SIM_PIPE, NFCC_HciCreateSuperSimPipe, NULL, NULL, 2, 2000},		  /***/
	{NCI_HCI_OPEN_ESE_PIPE, NFCC_HciOpenEsePipe, NULL, NULL, 3, 4000},						  /***/	
	{NCI_HCI_GET_PARA_ESE_APDUGATE_WTX, NFCC_HciGetEseApdugateWtx, NULL, NFCC_HciGetEseApdugateWtxCallBack, 2, 2000}, /***/
	{NCI_HCI_GET_PARA_ESIM_APDUGATE_WTX, NFCC_HciGetEsimApdugateWtx, NULL, NFCC_HciGetEsimApdugateWtxCallBack, 2, 2000}, /***/
	
	{NCI_HCI_OPEN_SUPER_SIM_PIPE, NFCC_HciOpenSuperSimPipe, NULL, NULL, 3, 4000},				  /***/
	{NCI_HCI_CLOSE_ESE_PIPE, NFCC_HciCloseEsePipe, NULL, NULL, 2, 2000},						  /***/
	{NCI_HCI_CLOSE_SUPER_SIM_PIPE, NFCC_HciCloseSuperSimPipe, NULL, NULL, 2, 2000},			  /***/
	{NCI_HCI_TRANS_APDU, NFCC_HciTransApdu, NFCC_HciTransChainingApduCallBack, NFCC_HciTransApduCallBack, 2, 2000}, /***/
	{NCI_HCI_TRANS_APDU_NEED_NEXT, NFCC_HciTransApdu, NULL, NULL, 1, 2000}, /***/

	
	{NCI_HCI_CLEAR_PIPE, NFCC_HciClearPipe, NULL, NULL, 2, 2000},								  /***/
	{NCI_HCI_SET_PARA, NFCC_HciSetPara, NULL, NULL, 2, 2000},									  /***/
	{NCI_HCI_SET_WHITE, NFCC_HciSetWhite, NULL, NULL, 2, 2000},								  /***/
	{NCI_HCI_RESPONESE_OK, NFCC_HciResponseOk, NULL, NULL, 1, 2000},							  /***/
	{NCI_HCI_WAKUP, NFCC_HciWakeup, NULL, NULL, 1, 2000},										  /***/
	{NCI_HCI_END_TRANSACTION, NFCC_HciEndTranstion, NULL, NULL, 1, 2000},						  /***/
#endif

#if NFCC_HCE_SUPPORT
	{NCI_HCE_TRANSTION, NFCC_HceTranstion, NULL, NULL, 0, 2000}, /***/
#endif

#if NFCC_LOCAL_M1_SUPPORT
	{NCI_NFCEE_LOCAL_M1_ENABLE, NFCC_NfceeLocalM1Enable, NULL, NULL, 2, 500},						   /***/
	{NCI_LOCAL_M1_QUERY, NFCC_Localm1Query, NULL, NFCC_Localm1QueryCallback, 1, 400},				   /***/
	{NCI_LOCAL_M1_CREATE, NFCC_Localm1Create, NULL, NFCC_Localm1CreateCallback, 1, 400},			   /***/
	{NCI_LOCAL_M1_DELETE, NFCC_Localm1Delete, NULL, NFCC_Localm1DeleteCallback, 1, 400},			   /***/
	{NCI_LOCAL_M1_ACTIVATE, NFCC_Localm1Activate, NULL, NFCC_Localm1ActivateCallback, 1, 400},	   /***/
	{NCI_LOCAL_M1_DEACTIVE, NFCC_Localm1Deactive, NULL, NFCC_Localm1DeactiveCallback, 1, 400},	   /***/
	{NCI_LOCAL_M1_W_SECTOR, NFCC_Localm1WriteSector, NULL, NFCC_Localm1WriteSectorCallBack, 1, 400}, /***/
	{NCI_LOCAL_M1_R_SECTOR, NFCC_Localm1ReadSector, NULL, NFCC_Localm1ReadSectorCallBack, 1, 400},   /***/
#endif

#if NFCC_RW_SUPPORT
	{NCI_RW_PROCESS_NEED_NEXT, NFCC_RwProcess, NULL, NULL, 1, 2000}, /***/
	{NCI_RW_PROCESS, NFCC_RwProcess, NFCC_RwProcessCallback, NFCC_RwProcessCallback, 2, 2000}, /***/
	{NCI_RW_SELECT_CARD, NFCC_RwSelectCard, NULL, NULL, 1, 400},			  /***/
	{NCI_RW_AUTH_M1_SECTOR, NFCC_RwAuthM1Sector, NULL, NULL, 2, 400},		  /***/
	{NCI_RW_READ_M1_SECTOR, NFCC_RwReadM1Sector, NULL, NULL, 2, 400},		  /***/
	{NCI_RW_WRITE_M1_SECTOR, NFCC_RwWriteM1Sector, NULL, NULL, 2, 400},	  /***/
#endif

#if NFCC_SOC2_SUPPORT
	{NCI_LPCD_CHECK_CARD, NFCC_LpcdCheckCard, NULL, NULL, 1, 400}, /***/
#endif
};

static COMMON_NCI_PACKET nci_send_packet;
static COMMON_NCI_PACKET nci_rev_packet;
static uint8_t g_nfcc_buffer[261] = {0};
// static uint8_t g_rev_apdu_buffer[261] = {0};
// static uint16_t g_rev_apdu_len		  = 0;
static uint8_t g_CmdInDataBuf[300]	= {0};
static uint8_t g_CmdOutDataBuf[300] = {0};

static uint8_t g_cmdMaxBuf[APP_MAX_DATA_SIZE] = {0}; //2K
uint16_t g_cmdMaxBufIndex			  = 0;

//LOCAL M1 INDEX
static uint8_t g_LocalM1CurrentIndex = 0;

const char sdk_version[] = {"RTOS NFCC HOST SDK[V2.1.0-Beta5]"};

//4字节的UID，1个校验字节，1个字节SIZE，2 个字节的卡片类型
uint8_t appSysData[4][8] = {{0xA0, 0x9A, 0xA4, 0x3F, 0xA1, 0x08, 0x04, 0x00},
							{0xA0, 0x9A, 0xA4, 0x3F, 0xA1, 0x08, 0x04, 0x00},
							{0xA0, 0x9A, 0xA4, 0x3F, 0xA1, 0x08, 0x04, 0x00},
							{0xA0, 0x9A, 0xA4, 0x3F, 0xA1, 0x08, 0x04, 0x00}};

uint8_t appInitKeySet[4][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
								{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
								{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
								{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

uint8_t g_SuperSimUid[8]	  = {0};
uint8_t g_get_config_cmd[128] = {0};
uint8_t g_get_config_cmd_len  = 0;

t_NFCC_Info g_NfcInfo	   = {0};
t_NFCC_API_cmd g_apiCmd	   = {0};
t_NFCC_Config g_nfc_config = {
	MODE_NFC_CE,		  /**nfc_mode */
	DEFAULT_OFFHOST_ROUTE /** ce_mode-route*/
};
tNFC_dm g_nfc_dm = {
	.RF_wtx_time_ms = 500 + 50,
	.HCI_ese_apdugate_wtx_time_ms = 1000 + 50,
	.HCI_esim_apdugate_wtx_time_ms = 1000 + 50,
};

static void Clear_g_CmdOutDataBuf(void)
{
	memset(g_CmdOutDataBuf, 0, sizeof(g_CmdOutDataBuf));
}
char *NFCC_GetVersion(void)
{
	return (char *)sdk_version;
}

/**
 * 
 * 提供给应用层维护
 */
void NFCC_SetGlobalStatus(t_NFCC_Status status)
{
	if (status < NFCC_STATUS_IDLE || status > NFCC_STATUS_ACTIVE_RW)
	{
		NFC_TRACE_ERROR("para error, status:%#x", status);
		return;
	}
	
	// if (status == NFCC_STATUS_IN_FIELD)
	// {
	// 	//入场，需要发送通知；；；
	// 	return;
	// }
	// else if (status == NFCC_STATUS_OUT_FIELD)
	// {
	// 	//离场，需要发送通知；；；
	// 	return;
	// }

	g_NfcInfo.nfcc_status = status;
}

t_NFCC_Status NFCC_GetGlobalStatus(void)
{
	return g_NfcInfo.nfcc_status;
}

/**
 * 
 * 底层的RF 状态机机制
 */
void NFCC_SetRFGlobalStatus(t_NFCC_rf_Status status)
{
	if (status < RFST_IDLE || status > RFST_W4_HOST_SELECT)
	{
		NFC_TRACE_ERROR("para error, status:%#x", status);
		return;
	}
	g_NfcInfo.nfcc_status_m = status;
	// NFC_TRACE_DEBUG("--set g_NfcInfo.nfcc_status_m = %d", g_NfcInfo.nfcc_status_m);
}
t_NFCC_rf_Status NFCC_GetRFGlobalStatus(void)
{
	// NFC_TRACE_DEBUG("--get g_NfcInfo.nfcc_status_m = %d", g_NfcInfo.nfcc_status_m);
	return g_NfcInfo.nfcc_status_m;
}

static void set_route_addr(uint8_t route)
{
	g_nfc_config.ce_route = route;
}

static void set_ce_mode(uint8_t route)
{
	g_nfc_config.nfc_mode = MODE_NFC_CE;
	set_route_addr(route);
	NFC_TRACE_DEBUG("set global ce mode,route = 0x%02x", route);
}
static void set_rw_mode(void)
{
	g_nfc_config.nfc_mode = MODE_NFC_RW;
	NFC_TRACE_DEBUG("set global rw mode");
}
static void set_auto_mode(uint8_t route)
{
	g_nfc_config.nfc_mode = MODE_NFC_CE | MODE_NFC_RW;
	set_route_addr(route);
}
t_NFCC_Config *NFCC_GetConfig(void)
{
	return &g_nfc_config;
}

unsigned char g_nfcc_fwdl_flag = 0; //复位后也增发，确保主控复位能够重新设置超级SIM
#if NFCC_SOC2_SUPPORT
static uint8_t g_factory_init_flag = 0;
#endif

extern void nfc_hw_init(void);
static void NFCC_Start(void)
{
	t_RetStatus retStatus = CMD_RECE_OK;
	uint16_t ret_fwup_success = FW_UD_ERR_FAIL;
	trace_level			  = DEFAULT_DEBUG_LEVEL; //打开串口日志
	uint8_t fwdl_cnt = 0;
	const uint8_t fwdl_cnt_max = 2;
	nfc_hw_init();
__NFC_START:
	// foce_try_fwdl();
	NFCC_SetGlobalStatus(NFCC_STATUS_IDLE); //
	NFCC_SetRFGlobalStatus(RFST_IDLE);
	retStatus = NFCC_NciCmdProcess(NCI_ACTIVE);
	
	if (retStatus == CMD_RECE_OK)
	{
		if (g_nfcc_fwdl_flag == 0)
		{
			retStatus = NFCC_NciCmdProcess(NCI_CORE_RESET); //reset NTF中获取到了版本号

			if (retStatus == CMD_RECE_OK)
			{
				ret_fwup_success = check_try_fwdl();
				if (FW_UD_NO_NEED_SUCCESS == ret_fwup_success)
				{
					NFC_TRACE_INFO("check_try_fwdl donnot need fwdl yet");
				}
				else if ((FW_UD_SUCCESS == ret_fwup_success))
				{
					NFC_TRACE_INFO("check_try_fwdl successfully");
					goto __NFC_START;
				}
				else
				{			
					if (fwdl_cnt++ < fwdl_cnt_max)
					{
						goto __FORCE_FWDL;
					}
					else
					{
						NFC_TRACE_ERROR("check_try_fwdl fwdl_cnt:%d to max", fwdl_cnt);
					}
				}
			}
			else 
			{
				NFC_TRACE_ERROR("goto __FORCE_FWDL");
				goto __FORCE_FWDL;
			}
		}		
	}
	else
	{
__FORCE_FWDL:
		ret_fwup_success = foce_try_fwdl();
		if (fwdl_cnt++ < fwdl_cnt_max)
		{
			goto __NFC_START;
		}
		else
		{
			NFC_TRACE_ERROR("foce_try_fwdl fwdl_cnt:%d to max", fwdl_cnt);
			retStatus = CMD_RECE_OK; //强制不再升级
		}
	}
	
	if (g_nfcc_fwdl_flag == 1) //确保固件更新后，参数恢复---增发两条
	{
		fwdl_cnt = 0;
		g_nfcc_fwdl_flag = 0;
		if (retStatus == CMD_RECE_OK)
		{
			retStatus = NFCC_NciCmdProcess(NCI_CORE_RESET_DW); //reset NTF中获取到了版本号
		}
		else
		{
			NFC_TRACE_ERROR("goto __FORCE_FWDL");
			goto __FORCE_FWDL;
		}
		
//		if (retStatus == CMD_RECE_OK)
//		{
//			if (FW_UD_NO_NEED_SUCCESS != check_try_fwdl()) //判断是否需要更新固件
//			{
//				goto __NFC_START;
//			}
//		}
		if (retStatus == CMD_RECE_OK)
		{
			retStatus = NFCC_NciCmdProcess(NCI_CORE_INIT);
		}

		if (retStatus == CMD_RECE_OK)
		{
			retStatus = NFCC_NciCmdProcess(NCI_CORE_RESET); //reset NTF中获取到了版本号
		}
		else
		{
			ret_fwup_success = foce_try_fwdl();
			if (FW_UD_SUCCESS != ret_fwup_success) //强制更新固件
			{
				NFC_TRACE_ERROR("foce_try_fwdl failed");
			}

			retStatus = NFCC_NciCmdProcess(NCI_ACTIVE); //强制更新完，强制激活
			
			retStatus = NFCC_NciCmdProcess(NCI_CORE_RESET_DW); //reset NTF中获取到了版本号

			retStatus = NFCC_NciCmdProcess(NCI_CORE_INIT);

			retStatus = NFCC_NciCmdProcess(NCI_CORE_RESET); //reset NTF中获取到了版本号
		}
	}
	
	if (retStatus == CMD_RECE_OK)
	{
		retStatus = NFCC_NciCmdProcess(NCI_CORE_INIT);
	}

	if (retStatus == CMD_RECE_OK)
	{
		retStatus = NFCC_NciCmdProcess(NCI_CORE_GET_CONFIG_FOR_RF_WTX_TIME);
	}
	
	if (retStatus == CMD_RECE_OK)
	{
		// NCI_CORE_CONFIG_FOR_NFCC
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_NFCC);
	}
#if 1
	if (retStatus == CMD_RECE_OK)
	{
		// NCI_CORE_CONFIG_FOR_SUPERSIM
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_SUPERSIM);
	}
#endif
	if (retStatus == CMD_RECE_OK)
	{
		// CORE_SET_CONFIG_CMD : RF_NFCEE_ACTION
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_NFCEE_ACTION);
	}

#if NFCC_SOC2_SUPPORT //SOC2 支持 GD 和 动态相位
	if (retStatus == CMD_RECE_OK)
	{
		// CORE_SET_CONFIG_CMD : NCI_CORE_CONFIG_FOR_GD_PHASE
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_GD_PHASE);
	}
#else //SOC1 仅仅打开动态相位  SOC1 不支持
	// if (retStatus == CMD_RECE_OK)
	// {
	// 	// CORE_SET_CONFIG_CMD : NCI_CORE_CONFIG_FOR_PHASE
	// 	retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_PHASE);
	// }
#endif

#if NFCC_USE_SDK_RF_PARA
	if (retStatus == CMD_RECE_OK)
	{
		// CORE_SET_CONFIG_CMD : NCI_CORE_CONFIG_FOR_RF_PARA
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_RF_PARA);
	}
#endif

#if NFCC_ESE_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		// CORE_SET_CONFIG_CMD : SESSIONID_UICC1
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_SESSIONID_ESE);
	}
#endif

#if NFCC_SURPER_SIM_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		// CORE_SET_CONFIG_CMD : SESSIONID_UICC2
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_SESSIONID_SIM);
	}
#endif

	if (retStatus == CMD_RECE_OK)
	{
		// NFCEE_DISCOVER_CMD
		retStatus = NFCC_NciCmdProcess(NCI_NFCEE_DISCOVER);
	}

#if NFCC_SOC2_SUPPORT
#if NFCC_ESE_SUPPORT | NFCC_SURPER_SIM_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		retStatus = NFCC_NciCmdProcess(NCI_NFCEE_POWER);//eSE
	}
#endif
#endif

#if NFCC_ESE_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		// NFCC_CMD_NFCEE_MODE_SET : enable 0xc0
		retStatus = NFCC_NciCmdProcess(NCI_NFCEE_ESE_ENABLE);
	}
#endif

#if NFCC_SURPER_SIM_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		// NFCC_CMD_NFCEE_MODE_SET : enable 0x81
		retStatus = NFCC_NciCmdProcess(NCI_NFCEE_SUPERSIM_ENABLE);
	}
#endif

#if NFCC_LOCAL_M1_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		// NFCC_CMD_NFCEE_MODE_SET : enable 0x10
		retStatus = NFCC_NciCmdProcess(NCI_NFCEE_LOCAL_M1_ENABLE);
	}
#endif

	if (retStatus == CMD_RECE_OK)
	{
		// RF_DISCOVER_MAP_CMD
		retStatus = NFCC_NciCmdProcess(NCI_RF_DISCOVER_MAP);
	}

#if NFCC_SOC2_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		if (g_factory_init_flag == 1)
		{
			g_factory_init_flag = 0;
			// NCI_LPCD_CHECK_CARD   ---这个出厂的时候,在工厂干净环境下，调用一次
			retStatus = NFCC_NciCmdProcess(NCI_LPCD_CHECK_CARD); //
		}
	}
#endif

	if (retStatus == CMD_RECE_OK)
	{
		// NCI_CORE_CONFIG_FOR_FIELD_INFO
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_FIELD_INFO);
	}

	if (retStatus == CMD_RECE_OK)
	{
		// NCI_HED_UPDATE_FLASH
		retStatus = NFCC_NciCmdProcess(NCI_HED_UPDATE_FLASH);
	}

#if NFCC_HCI_SUPPORT //test apdu gate 功能
	//todo ... nfcee  gate 功能
	if (retStatus == CMD_RECE_OK)
	{
		//NCI_HCI_CLEAR_PIPE
		//retStatus = NFCC_NciCmdProcess(NCI_HCI_CLEAR_PIPE);
	}

#if 1
	if (g_NfcInfo.EseGateIsReady == 1)
	{
		if (retStatus == CMD_RECE_OK)
		{
			//创建PIPE  ESE
			retStatus = NFCC_NciCmdProcess(NCI_HCI_CREATE_ESE_PIPE);
		}
		if (retStatus == CMD_RECE_OK)
		{
			//OPEN PIPE
			retStatus = NFCC_NciCmdProcess(NCI_HCI_OPEN_ESE_PIPE); //会收到ATR
		}
		if (retStatus == CMD_RECE_OK)
		{
			//get EVT_WTX MAX_WAIT_TIME Maximum waiting time for the execution of a command APDU in ms
			retStatus = NFCC_NciCmdProcess(NCI_HCI_GET_PARA_ESE_APDUGATE_WTX); //会收到WTS
		}		
	}

	if (g_NfcInfo.SuperSimGateIsReady == 1)
	{
		if (retStatus == CMD_RECE_OK)
		{
			//创建PIPE  SURPER_SIM
			retStatus = NFCC_NciCmdProcess(NCI_HCI_CREATE_SURPER_SIM_PIPE);
		}
		if (retStatus == CMD_RECE_OK)
		{
			//OPEN PIPE
			retStatus = NFCC_NciCmdProcess(NCI_HCI_OPEN_SUPER_SIM_PIPE); //会收到ATR
		}
		if (retStatus == CMD_RECE_OK)
		{
			//get EVT_WTX MAX_WAIT_TIME Maximum waiting time for the execution of a command APDU in ms
			retStatus = NFCC_NciCmdProcess(NCI_HCI_GET_PARA_ESIM_APDUGATE_WTX); //会收到wts
		}	
	}

	//增加APDU测试
	if (retStatus != CMD_RECE_OK)
	{
		NFC_TRACE_DEBUG("&&&&&&&&&&&&&&&&&&&& apdu gate failed !!!!!!!!!!!!!!!!!!!");
		retStatus = CMD_RECE_OK; /*清除错误，确保下面的NFCC discovery能够执行*/
	}
#endif

#endif

#if NFCC_LOCAL_M1_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		g_LocalM1CurrentIndex = 0; //查询 APP1
		// NCI_LOCAL_M1_QUERY
		retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_QUERY);
	}
	if (retStatus == CMD_RECE_OK)
	{
		g_LocalM1CurrentIndex = 1; //查询 APP2
		// NCI_LOCAL_M1_QUERY
		retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_QUERY);
	}
	if (retStatus == CMD_RECE_OK)
	{
		g_LocalM1CurrentIndex = 2; //查询 APP3
		// NCI_LOCAL_M1_QUERY
		retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_QUERY);
	}
	if (retStatus == CMD_RECE_OK)
	{
		g_LocalM1CurrentIndex = 3; //查询 APP4
		// NCI_LOCAL_M1_QUERY
		retStatus			  = NFCC_NciCmdProcess(NCI_LOCAL_M1_QUERY);
		g_LocalM1CurrentIndex = 0;
	}

	if (g_NfcInfo.LocalM1IsExit == 0) //没有LocalM1的时候创建一张默认卡,并激活
	{
		// if (retStatus == CMD_RECE_OK)
		// {
		// 	// NCI_LOCAL_M1_DELETE
		// 	retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_DELETE);
		// }
		g_LocalM1CurrentIndex = 0;
		if (retStatus == CMD_RECE_OK)
		{
			// NCI_LOCAL_M1_CREATE
			retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_CREATE);
		}

		// if (retStatus == CMD_RECE_OK) //测试--这里就不再重复发送了
		// {
		// 	// NFCEE_DISCOVER_CMD
		// 	retStatus = NFCC_NciCmdProcess(NCI_NFCEE_DISCOVER);
		// }
	}

	else if ((g_NfcInfo.LocalM1IsExit & 0x01) == 0x01) //type 0 存在，则直接激活
	{
		if (retStatus == CMD_RECE_OK)
		{
			// NCI_LOCAL_M1_ACTIVATE
			retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_ACTIVATE);
		}
	}

#endif

	// debug_io(2);

	if (retStatus == CMD_RECE_OK)
	{
		// NFCC_CMD_RF_SET_ROUTE
		retStatus = NFCC_NciCmdProcess(NCI_RF_SET_ROUTE);
		// NFC_TRACE_DEBUG("&&&&&&&&&&&&&&&&&&&&set Route OK!!!!!!!!!!!!!!!!!!!");
	}

#if NFCC_USE_SDK_RF_PARA
	//如果SDK中设置了射频参数，射频参数中会包含此项设置，这里就不再设置了

#else
#if NFCC_SOC2_SUPPORT
	if (retStatus == CMD_RECE_OK)
	{
		// NCI_CORE_CONFIG_FOR_CE_DLMA
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_CE_DLMA); /**动态相位调整，输出功率 */
		NFC_TRACE_DEBUG("&&&&&&&&&&&&&&&&&&&&NCI_CORE_CONFIG_FOR_CE_DLMA OK333!!!!!!!!!!!!!!!!!!!");
	}
#endif
#endif

	// debug_io(3);

#if NFCC_SOC2_SUPPORT
#if NFCC_FW_CONFIG_NFCC_PARA
	if (retStatus == CMD_RECE_OK) /**非接调试射频参数*/
	{
		// NFCC_CMD_RF_DISCOVER
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_ENABLE_RF_SET_PARA);
	}
#else
	if (retStatus == CMD_RECE_OK) /**禁止非接调试射频参数*/
	{
		// NFCC_CMD_RF_DISCOVER
		retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_DISABLE_RF_SET_PARA);
	}
#endif
#endif

	// if (retStatus == CMD_RECE_OK)  /**禁止RW DATA*/
	// {
	// 	// NCI_CORE_CONFIG_FOR_DISABLE_RAW_DATA
	// 	retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_DISABLE_RAW_DATA);
	// }

	// debug_io(4);

	if (retStatus == CMD_RECE_OK)
	{
		// NFCC_CMD_RF_DISCOVER
		retStatus = NFCC_NciCmdProcess(NCI_RF_DISCOVER);
	}

	if (retStatus == CMD_RECE_OK)
	{
		NFC_TRACE_DEBUG("func:%s; line:%d, nfc init OK!!!", __FUNCTION__, __LINE__);
	}
	else
	{
		//不应该进入此处，需要核实硬件及相关配置
		NFC_TRACE_DEBUG("nfc init ERR!!!");
		NFC_TRACE_DEBUG("func:%s; line:%d, nfc init ERR!!!", __FUNCTION__, __LINE__);
	}
	// debug_io(10);
}

static void NFCC_DataHcePacket(COMMON_NCI_PACKET *nci_recv_packet)
{
	t_RetStatus retStatus = CMD_RECE_OK;
	// NFC_TRACE_DEBUG("NFCC_DataHcePacket");

	memcpy(&g_cmdMaxBuf[g_cmdMaxBufIndex], nci_recv_packet->PAYLOAD, nci_recv_packet->PAYLOAD_LEN);
	g_cmdMaxBufIndex += nci_recv_packet->PAYLOAD_LEN;

	if (nci_recv_packet->PBF == 0) //最后一个包
	{
		uint16_t data_len = g_cmdMaxBufIndex;
		g_cmdMaxBufIndex = 0;
		NFCC_HceCallBack(g_cmdMaxBuf, data_len);
	}

	if (g_cmdMaxBufIndex >= APP_MAX_DATA_SIZE)
	{
		NFC_TRACE_ERROR("func:%s; line:%d,buff  is full", __FUNCTION__, __LINE__);
		g_cmdMaxBufIndex = 0;
		return;
	}
}

static void NFCC_DataHciPacket(COMMON_NCI_PACKET *nci_recv_packet)
{
	t_RetStatus retStatus = CMD_RECE_OK;
	// NFC_TRACE_DEBUG("NFCC_DataHciPacket");

	if (nci_recv_packet->GID == 0x01) //HCI的处理
	{
		if ((nci_recv_packet->PAYLOAD_LEN == 0x03) && (nci_recv_packet->PAYLOAD[0] == 0x81) && (nci_recv_packet->PAYLOAD[1] == 0x15))
		{
			//03 00 03 81 15 02  //清除所有PIPE
			retStatus = NFCC_NciCmdProcess(NCI_HCI_RESPONESE_OK);
		}
		else if ((nci_recv_packet->PAYLOAD_LEN == 0x07) && (nci_recv_packet->PAYLOAD[0] == 0x81) && (nci_recv_packet->PAYLOAD[1] == 0x12))
		{
			//01 00 07 81 12 81 41 01 41 10 //creation of a dynamic pipe
			retStatus = NFCC_NciCmdProcess(NCI_HCI_RESPONESE_OK);
		}
		else if ((nci_recv_packet->PAYLOAD_LEN == 0x02) && (nci_recv_packet->PAYLOAD[1] == 0x03))
		{
			nci_recv_packet->PAYLOAD_LEN = 0x03;
			nci_recv_packet->PAYLOAD[1]	 = 0x80;
			nci_recv_packet->PAYLOAD[2]	 = 0x00;
			Nci_unwrap(*nci_recv_packet, 0); //直接发送
		}
	}
	else if (nci_recv_packet->GID == 0x00)
	{
		NFCC_DataHcePacket(nci_recv_packet); //HCE的处理
	}
}

unsigned char g_pollNtfFlag = 0;
uint16_t NFCC_Poll(void)
{
	uint16_t recv_cnt = 0;
	while (NfcCmdI2cRecvData(g_nfcc_buffer, BUFFER_BLOCK_SIZE - 1) != 0)
	{
		recv_cnt++;
		g_pollNtfFlag = 1;
		Nci_wrap(g_nfcc_buffer, &nci_rev_packet);
//		NFC_TRACE_DEBUG("NFCC_Poll -- , MT:%#x, GID:%#x, OID:%#x", nci_rev_packet.MT, nci_rev_packet.GID, nci_rev_packet.OID);
		
		if ((nci_rev_packet.MT == MT_CTL_RES_PACKET))
		{
			Nci_Rsp_Packet(&nci_rev_packet);
			// NFC_TRACE_DEBUG("receive ok777");
		}
		else if (nci_rev_packet.MT == MT_DATA_PACKET)
		{
			// NFC_TRACE_DEBUG("receive ok888");
			NFCC_DataHciPacket(&nci_rev_packet);
		}
		if (nci_rev_packet.MT == MT_CTL_NTF_PACKET)
		{
			// NFC_TRACE_DEBUG("receive ok999");
			Nci_Ntf_Packet(&nci_rev_packet);
		}
		g_pollNtfFlag = 0;
	}
	return recv_cnt;
}

static bool g_apdu_gate_first_packet = 0;
static bool g_copy_m1_flag			 = 0;
static bool is_fido_packet = 0;

void NFCC_OtherCmdPoll(void)
{
	t_RetStatus retStatus	   = CMD_RECE_OK;
	t_NFCC_Config *pNfcConfig  = NULL;
	pNfcConfig				   = NFCC_GetConfig();
	t_NFCC_rf_Status rf_status = NFCC_GetRFGlobalStatus();
	uint8_t i				   = 0;
	g_apiCmd.cmd_status 	   = NFCC_RESULT_FAILED;

	if (g_apiCmd.cmd_start_flag == 1)
	{
		g_apiCmd.cmd_start_flag = 0;
		NFC_TRACE_DEBUG("func:%s; line:%d, rf_status == 0x%02x", __FUNCTION__, __LINE__, rf_status);
		switch (g_apiCmd.cmd)
		{
		case NFCC_CMD_RESET:
			NFCC_Start();
			break;
		case NFCC_CMD_REGISTER_CB:
			g_nfc_dm.sdk_callback_fun_handle_event = (tSdkEventCallback)g_apiCmd.impl_intf;
			break;
		case NFCC_CMD_CHANGE_ROUT:
			if (retStatus == CMD_RECE_OK)
			{
				if ((rf_status == RFST_POLL_ACTIVE) || (rf_status == RFST_LISTEN_ACTIVE))
				{
					retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_IDEL_PLUS);
				}
				else
				{
					retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_IDEL);
				}
			}
			if (retStatus == CMD_RECE_OK)
			{
				set_route_addr(g_apiCmd.para1);

				uint16_t next_routing_len = 0;
				uint16_t this_frame_len = 2;// more:1  Number of Routing Entries :1
				uint8_t  frame_num = 0;
				uint8_t  num = 0;
				uint16_t total_index = 0;

				if (g_apiCmd.p_in_data[1] == 0)
				{
					NFC_TRACE_ERROR("NFCC_CMD_CHANGE_ROUT: routing 0 ERROR");
					g_apiCmd.cmd_status = NFCC_RESULT_PARA_ERR;
					break;
				}

				this_frame_len = 2;//每一帧头两个字节存放 more 和 Number of Routing Entries
				total_index = 2;
				next_routing_len = g_apiCmd.p_in_data[total_index+1] + 2;
				while (num <= g_apiCmd.p_in_data[1]) 
				{
					
					if (this_frame_len + next_routing_len < NCI_SINGLE_FRAME_MAX_LEN)
					{
						memcpy(&nci_send_packet.PAYLOAD[this_frame_len], g_apiCmd.p_in_data + total_index, next_routing_len);
						this_frame_len += next_routing_len;
						total_index += next_routing_len;
						num++;
						frame_num++;
						if (num < g_apiCmd.p_in_data[1])
						{
							next_routing_len = g_apiCmd.p_in_data[total_index + 1] + 2;
							continue;
						}
						else
						{
							break;
						}
					}
					else 
					{
						nci_send_packet.PBF = PBF_NCI_RESET;
						nci_send_packet.PAYLOAD_LEN = this_frame_len;
						nci_send_packet.PAYLOAD[0] = 1; //"more", 1: More Message(s) to follow
						nci_send_packet.PAYLOAD[1] = frame_num;
						retStatus = NFCC_NciCmdProcess(NCI_RF_SET_ROUTE); //
						this_frame_len = 2;
						frame_num = 0;
					}
				}

				nci_send_packet.PBF = PBF_NCI_RESET;
				nci_send_packet.PAYLOAD_LEN = this_frame_len;
				nci_send_packet.PAYLOAD[0] = 0; //Last Message
				nci_send_packet.PAYLOAD[1] = frame_num;
				retStatus = NFCC_NciCmdProcess(NCI_RF_SET_ROUTE); //
				
			}
			if (retStatus == CMD_RECE_OK)
			{
				retStatus = NFCC_NciCmdProcess(NCI_RF_DISCOVER);
			}
			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_POWDOWN_CE ERROR", __FUNCTION__, __LINE__);
			}
			break;
		case NFCC_CMD_POWDOWN_CE:
			if (retStatus == CMD_RECE_OK)
			{
				if ((rf_status == RFST_POLL_ACTIVE) || (rf_status == RFST_LISTEN_ACTIVE))
				{
					retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_IDEL_PLUS);
				}
				else
				{
					retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_IDEL);
				}
			}
			if (retStatus == CMD_RECE_OK)
			{
				retStatus = NFCC_NciCmdProcess(NCI_HED_UPDATE_FLASH); //reset NTF中获取到了版本号
			}
			if (retStatus == CMD_RECE_OK)
			{
				retStatus = NFCC_NciCmdProcess(NCI_CORE_RESET); //固件要求，进入关机模拟，需要
			}
			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_POWDOWN_CE ERROR", __FUNCTION__, __LINE__);
			}
			else
			{
				//执行NFCC的关机时序---VEN 拉低   随后3ms内VDDIO拉低
				port_hardware_gpioVenPulldown();
				//port_hardware_gpio1V8Pulldown();
				/**
				 * NFCC 进Low Power模式时序
				 * 1.VEN拉低
				 * 2.VDDIO在VEN拉低后的3MS拉低
				 * 
				 * 若上述3ms时序难满足的话。可以通过现拉低VDDIO，再拉低VEN操作。由于内部电路原因此操作实际波形为VDDIO和VEN同时拉低，实际满足关机时序。
				 * 
				 */
			}
			break;
		case NFCC_CMD_SEND_APDU_GATE:
			if (retStatus == CMD_RECE_OK)
			{
				if (g_apdu_gate_first_packet == true)
				{
					if (g_apiCmd.para1 == ESE_HOST)
					{
						retStatus = NFCC_NciCmdProcess(NCI_NFCEE_ESE_ENABLE); //
					}
					else if (g_apiCmd.para1 == SUPER_SIM_HOST)
					{
						retStatus = NFCC_NciCmdProcess(NCI_NFCEE_SUPERSIM_ENABLE); //
					}
				}
			}
			if (retStatus == CMD_RECE_OK)
			{
				i = 0;
				uint8_t pipe = 0;

				if (g_apiCmd.para1 == ESE_HOST)
				{
					pipe = g_NfcInfo.ese_gate_pipe; //
				}
				else if (g_apiCmd.para1 == SUPER_SIM_HOST)
				{
					pipe = g_NfcInfo.supersim_gate_pipe; //
				}

				if (g_apiCmd.in_data_len <= (NCI_SINGLE_FRAME_MAX_LEN -2 ))
				{
					nci_send_packet.PAYLOAD[i++] = (0x80 | pipe); //
					nci_send_packet.PAYLOAD[i++] = 0x50; //1 << 6 | 0x10, event | EVT_C-APDU
					memcpy(&nci_send_packet.PAYLOAD[i], g_apiCmd.p_in_data, g_apiCmd.in_data_len);
					i += g_apiCmd.in_data_len;
					nci_send_packet.PAYLOAD_LEN = i;
					retStatus					= NFCC_NciCmdProcess(NCI_HCI_TRANS_APDU); //
				}
				else 
				{
					nci_send_packet.PAYLOAD[i++] = (0x00 | pipe); //
					nci_send_packet.PAYLOAD[i++] = 0x50; //1 << 6 | 0x10, event | EVT_C-APDU
					memcpy(&nci_send_packet.PAYLOAD[i], g_apiCmd.p_in_data, NCI_SINGLE_FRAME_MAX_LEN -2);	
					i += (NCI_SINGLE_FRAME_MAX_LEN -2);
					nci_send_packet.PAYLOAD_LEN = i;
					retStatus					= NFCC_NciCmdProcess(NCI_HCI_TRANS_APDU_NEED_NEXT); //
					g_apiCmd.in_data_len -= (NCI_SINGLE_FRAME_MAX_LEN - 2);
					g_apiCmd.p_in_data += NCI_SINGLE_FRAME_MAX_LEN - 2;

					while (g_apiCmd.in_data_len > NCI_SINGLE_FRAME_MAX_LEN - 1)
					{
						i = 0;
						nci_send_packet.PAYLOAD[i++] = (0x00 | pipe); //
						memcpy(&nci_send_packet.PAYLOAD[i], g_apiCmd.p_in_data, NCI_SINGLE_FRAME_MAX_LEN - 1);
						i += NCI_SINGLE_FRAME_MAX_LEN - 1;
						nci_send_packet.PAYLOAD_LEN = i;
						retStatus					= NFCC_NciCmdProcess(NCI_HCI_TRANS_APDU_NEED_NEXT); //
						g_apiCmd.in_data_len -= NCI_SINGLE_FRAME_MAX_LEN - 1;
						g_apiCmd.p_in_data += NCI_SINGLE_FRAME_MAX_LEN - 1;
					}

					i = 0;
					nci_send_packet.PAYLOAD[i++] = (0x80 | pipe); //
					memcpy(&nci_send_packet.PAYLOAD[i], g_apiCmd.p_in_data, g_apiCmd.in_data_len);
					i += g_apiCmd.in_data_len;
					nci_send_packet.PAYLOAD_LEN = i;
					retStatus					= NFCC_NciCmdProcess(NCI_HCI_TRANS_APDU); //
				}			
			}

			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_SEND_APDU_GATE ERROR", __FUNCTION__, __LINE__);
			}
			break;
		case NFCC_CMD_LOCAL_M1_CREATE:
			memcpy(appSysData[0], (uint8_t *)g_apiCmd.p_in_data, 8);
			memcpy(appInitKeySet[0], (uint8_t *)&g_apiCmd.p_in_data[8], 16);

			if ((g_NfcInfo.LocalM1IsAcitive == 1)
				&& (g_NfcInfo.LocalM1ActiveAppId == g_apiCmd.para1)) //去激活当前激活的卡片
			{
				if (retStatus == CMD_RECE_OK)
				{
					g_LocalM1CurrentIndex = g_NfcInfo.LocalM1ActiveAppId; //
					// NCI_LOCAL_M1_DEACTIVE
					retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_DEACTIVE);
				}
			}

			if ((g_NfcInfo.LocalM1IsExit >> g_apiCmd.para1) & 0x01) //待修改信息的卡片存在 则需要删除一下
			{
				if (retStatus == CMD_RECE_OK)
				{
					g_LocalM1CurrentIndex = g_apiCmd.para1; //
					// NCI_LOCAL_M1_DELETE
					retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_DELETE);
				}
			}

			if (retStatus == CMD_RECE_OK)
			{
				g_LocalM1CurrentIndex = g_apiCmd.para1; //
				// NCI_LOCAL_M1_CREATE
				retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_CREATE);
			}

			if ((g_NfcInfo.LocalM1ActiveAppId == g_apiCmd.para1)) //恢复激活的卡片
			{
				if (retStatus == CMD_RECE_OK)
				{
					g_LocalM1CurrentIndex = g_NfcInfo.LocalM1ActiveAppId; //
					// NCI_LOCAL_M1_ACTIVATE
					retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_ACTIVATE);
				}
			}
			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_LOCAL_M1_CREATE ERROR", __FUNCTION__, __LINE__);
			}
			break;

		case NFCC_CMD_LOCAL_M1_CHANGE:
			if (retStatus == CMD_RECE_OK)
			{
				for (i = 0; i < 4; i++) //去激活其他所有卡片
				{
					g_LocalM1CurrentIndex = i;
					if (g_NfcInfo.LocalM1IsExit & (0x01 << g_LocalM1CurrentIndex)) //卡片存在,所有存在的卡片均去激活
					{
						// NCI_LOCAL_M1_DEACTIVE
						retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_DEACTIVE);
					}
					g_LocalM1CurrentIndex = g_NfcInfo.LocalM1ActiveAppId; //
				}
			}
			g_LocalM1CurrentIndex = g_apiCmd.para1;
			if (retStatus == CMD_RECE_OK) //激活目标卡片
			{
				// NCI_LOCAL_M1_ACTIVATE
				retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_ACTIVATE);
			}
			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_LOCAL_M1_CREATE ERROR", __FUNCTION__, __LINE__);
			}
			break;

		case NFCC_CMD_LOCAL_M1_DELETE:
			g_LocalM1CurrentIndex = g_apiCmd.para1;
			if (retStatus == CMD_RECE_OK) //激活目标卡片
			{
				// NCI_LOCAL_M1_DELETE
				retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_DELETE);
			}
			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_LOCAL_M1_DELETE ERROR", __FUNCTION__, __LINE__);
			}
			break;
		case NFCC_CMD_LOCAL_M1_QUERY:
			g_LocalM1CurrentIndex = g_apiCmd.para1;
			if (retStatus == CMD_RECE_OK) //激活目标卡片
			{
				// NCI_LOCAL_M1_DELETE
				retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_QUERY);
			}
			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_LOCAL_M1_DELETE ERROR", __FUNCTION__, __LINE__);
			}

			break;

		case NFCC_CMD_LOCAL_M1_WRITE_SECTOR:

			if (g_copy_m1_flag == false) /**为了节省M1卡复制时间，此处先不恢复ACTIVE,复制完成后统一ACTIVE */
			{
				if ((g_NfcInfo.LocalM1IsAcitive == 1)
					&& (g_NfcInfo.LocalM1ActiveAppId == g_apiCmd.p_in_data[0])) //要写的卡片是当前激活的卡片
				{
					if (retStatus == CMD_RECE_OK)
					{
						g_LocalM1CurrentIndex = g_NfcInfo.LocalM1ActiveAppId; //
						// NCI_LOCAL_M1_ACTIVATE
						retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_DEACTIVE);
					}
				}
			}

			if (retStatus == CMD_RECE_OK)
			{
				g_LocalM1CurrentIndex = g_apiCmd.p_in_data[0]; //

				// NCI_LOCAL_M1_W_SECTOR
				retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_W_SECTOR);
			}

			if (g_copy_m1_flag == false) /**为了节省M1卡复制时间，此处先不恢复ACTIVE,复制完成后统一ACTIVE */
			{
				if (g_NfcInfo.LocalM1ActiveAppId == g_apiCmd.p_in_data[0]) //恢复之前的激活状态
				{
					g_LocalM1CurrentIndex = g_NfcInfo.LocalM1ActiveAppId;
					if (retStatus == CMD_RECE_OK)
					{
						// NCI_LOCAL_M1_ACTIVATE
						retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_ACTIVATE);
					}
				}
			}

			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_LOCAL_M1_WRITE_SECTOR ERROR", __FUNCTION__, __LINE__);
			}

			break;

		case NFCC_CMD_LOCAL_M1_READ_SECTOR:
			if (retStatus == CMD_RECE_OK)
			{
				// NCI_LOCAL_M1_R_SECTOR
				retStatus = NFCC_NciCmdProcess(NCI_LOCAL_M1_R_SECTOR);
			}
			if (retStatus != CMD_RECE_OK)
			{
				NFC_TRACE_ERROR("func:%s; line:%d,NFCC_CMD_LOCAL_M1_READ_SECTOR ERROR", __FUNCTION__, __LINE__);
			}
			break;

		case NFCC_CMD_GET_SURPERSIM_UID:
		case NFCC_CMD_GET_ESE_UID:
			memcpy(g_get_config_cmd, (uint8_t *)g_apiCmd.p_in_data, g_apiCmd.in_data_len);
			g_get_config_cmd_len = g_apiCmd.in_data_len;

			if (retStatus == CMD_RECE_OK)
			{
				if ((g_apiCmd.cmd == NFCC_CMD_GET_SURPERSIM_UID)) //超级SIM UID
				{
					memset(g_NfcInfo.super_sim_uid, 0, sizeof(g_NfcInfo.super_sim_uid));
					g_NfcInfo.super_sim_uid_size = 0;
				}
				else if (g_apiCmd.cmd == NFCC_CMD_GET_ESE_UID)
				{
					memset(g_NfcInfo.ese_uid, 0, sizeof(g_NfcInfo.ese_uid));
					g_NfcInfo.ese_uid_size = 0;
				}
				// NCI_CORE_GET_CONFIG
				retStatus = NFCC_NciCmdProcess(NCI_CORE_GET_CONFIG);
			}
			if (g_apiCmd.p_out_data)
			{
				if ((g_apiCmd.cmd == NFCC_CMD_GET_SURPERSIM_UID)) //超级SIM UID
				{
					memcpy((uint8_t *)g_apiCmd.p_out_data, g_NfcInfo.super_sim_uid, g_NfcInfo.super_sim_uid_size);
					g_apiCmd.out_data_len = g_NfcInfo.super_sim_uid_size;
				}
				else if (g_apiCmd.cmd == NFCC_CMD_GET_ESE_UID)
				{
					memcpy((uint8_t *)g_apiCmd.p_out_data, g_NfcInfo.ese_uid, g_NfcInfo.ese_uid_size);
					g_apiCmd.out_data_len = g_NfcInfo.ese_uid_size;
				}
			}
			break;

		case NFCC_CMD_SURPER_SIM_IN: //热插拔
			NFC_TRACE_ERROR("test:%s; line:%d,&&&&&&&&&&&&&&&&&&&&&@@@@@ test @@@@@@@@@&&&&&&&&&", __FUNCTION__, __LINE__);
			if (retStatus == CMD_RECE_OK)
			{
				if ((rf_status == RFST_POLL_ACTIVE) || (rf_status == RFST_LISTEN_ACTIVE))
				{
					retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_IDEL_PLUS);
				}
				else
				{
					retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_IDEL);
				}
			}
			if (retStatus == CMD_RECE_OK)
			{
				// NCI_NFCEE_SUPERSIM_ENABLE
				retStatus = NFCC_NciCmdProcess(NCI_NFCEE_SUPERSIM_DISABLE);
			}

			if (retStatus == CMD_RECE_OK)
			{
				// NCI_CORE_CONFIG_FOR_SESSIONID_SIM
				retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_SESSIONID_SIM);
			}
			if (retStatus == CMD_RECE_OK)
			{
				// NCI_NFCEE_SUPERSIM_ENABLE
				retStatus = NFCC_NciCmdProcess(NCI_NFCEE_SUPERSIM_ENABLE);
			}

#if NFCC_ESE_SUPPORT
			if (retStatus == CMD_RECE_OK)
			{
				// NCI_NFCEE_SUPERSIM_ENABLE
				retStatus = NFCC_NciCmdProcess(NCI_NFCEE_ESE_DISABLE);
			}

			if (retStatus == CMD_RECE_OK)
			{
				// CORE_SET_CONFIG_CMD : SESSIONID_UICC1
				retStatus = NFCC_NciCmdProcess(NCI_CORE_CONFIG_FOR_SESSIONID_ESE);
			}

			if (retStatus == CMD_RECE_OK)
			{
				// NFCC_CMD_NFCEE_MODE_SET : enable 0xc0
				retStatus = NFCC_NciCmdProcess(NCI_NFCEE_ESE_ENABLE);
			}
#endif
			// if (retStatus == CMD_RECE_OK)
			// {
			// 	// NCI_NFCEE_SUPERSIM_ENABLE
			// 	retStatus = NFCC_NciCmdProcess(NCI_HED_UPDATE_FLASH);
			// }
			if (retStatus == CMD_RECE_OK)
			{
				// NCI_RF_SET_ROUTE
				retStatus = NFCC_NciCmdProcess(NCI_RF_SET_ROUTE);
			}
			if (retStatus == CMD_RECE_OK)
			{
				// NFCC_RF_DISCOVER
				retStatus = NFCC_NciCmdProcess(NCI_RF_DISCOVER);
			}
			break;

		case NFCC_CMD_OPEN_RW: //todo ...需要优化
		case NFCC_CMD_CLOSE_RW:

			if (retStatus == CMD_RECE_OK)
			{
				if ((rf_status == RFST_POLL_ACTIVE) || (rf_status == RFST_LISTEN_ACTIVE))
				{
					retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_IDEL_PLUS);
				}
				else
				{
					retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_IDEL);
				}

				if (NFCC_CMD_OPEN_RW == g_apiCmd.cmd)
				{
					set_rw_mode();
				}
				else
				{
					set_ce_mode(g_apiCmd.para1);
				}
			}
			if (retStatus == CMD_RECE_OK)
			{
				// NCI_RF_SET_ROUTE
				retStatus = NFCC_NciCmdProcess(NCI_RF_SET_ROUTE);
			}
			if (retStatus == CMD_RECE_OK)
			{
				// NCI_RF_DISCOVER
				retStatus = NFCC_NciCmdProcess(NCI_RF_DISCOVER);
			}
			break;
		case NFCC_CMD_SELECT_CARD:
			if (rf_status == RFST_POLL_ACTIVE)
			{
				retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_DISCOVERY_PLUS);
			}

			if (retStatus == CMD_RECE_OK)
			{
				retStatus = NFCC_NciCmdProcess(NCI_RW_SELECT_CARD);
			}
			break;
		case NFCC_CMD_RE_DISCOVERY:

			if (rf_status == RFST_POLL_ACTIVE)
			{
				retStatus = NFCC_NciCmdProcess(NCI_RF_DEACTIVE_DISCOVERY_PLUS);
			}
			break;
		case NFCC_CMD_HCE_RESPONSE:

			if (rf_status == RFST_LISTEN_ACTIVE)
			{
				while (g_apiCmd.in_data_len > NCI_SINGLE_FRAME_MAX_LEN)
				{
					nci_send_packet.PBF = PBF_NCI_SET; //有后续分包
					memcpy(nci_send_packet.PAYLOAD, g_apiCmd.p_in_data, NCI_SINGLE_FRAME_MAX_LEN);
					nci_send_packet.PAYLOAD_LEN = NCI_SINGLE_FRAME_MAX_LEN;
					g_apiCmd.p_in_data += NCI_SINGLE_FRAME_MAX_LEN;
					g_apiCmd.in_data_len -= NCI_SINGLE_FRAME_MAX_LEN;
					retStatus = NFCC_NciCmdProcess(NCI_HCE_TRANSTION);
					//NFCC_Poll(); //其他NTF处理
				}

				nci_send_packet.PBF = 0x00; //最后一包
				memcpy(nci_send_packet.PAYLOAD, g_apiCmd.p_in_data, g_apiCmd.in_data_len);
				nci_send_packet.PAYLOAD_LEN = g_apiCmd.in_data_len;
				retStatus = NFCC_NciCmdProcess(NCI_HCE_TRANSTION);
                g_apiCmd.cmd_end_flag = 1;
                return;
				//NFCC_Poll(); //其他NTF处理
				//break;
			}
			break;
        case NFCC_CMD_HCE_FIDO_RESPONSE:
            if (rf_status == RFST_LISTEN_ACTIVE)
            {
                while (g_apiCmd.in_data_len > FIDO_NCI_SINGLE_FRAME_MAX_LEN)
                {
                    nci_send_packet.PBF = PBF_NCI_SET;
                    memcpy(nci_send_packet.PAYLOAD, g_apiCmd.p_in_data, FIDO_NCI_SINGLE_FRAME_MAX_LEN);
                    nci_send_packet.PAYLOAD[NCI_SINGLE_FRAME_MAX_LEN-2] = 0x61;
                    nci_send_packet.PAYLOAD[NCI_SINGLE_FRAME_MAX_LEN-1] = 0x00;
                    nci_send_packet.PAYLOAD_LEN = NCI_SINGLE_FRAME_MAX_LEN;
                    g_apiCmd.p_in_data += FIDO_NCI_SINGLE_FRAME_MAX_LEN;
                    g_apiCmd.in_data_len -= FIDO_NCI_SINGLE_FRAME_MAX_LEN;
                    retStatus = NFCC_NciCmdProcess(NCI_HCE_TRANSTION);
                    NFCC_Poll();
                }

                nci_send_packet.PBF = 0x00;
                memcpy(nci_send_packet.PAYLOAD, g_apiCmd.p_in_data, g_apiCmd.in_data_len);
                nci_send_packet.PAYLOAD_LEN = g_apiCmd.in_data_len;
                retStatus = NFCC_NciCmdProcess(NCI_HCE_TRANSTION);
                is_fido_packet = false;
                NFCC_Poll();
                break;
            }
            break;
		case NFCC_CMD_RW_PROCESS:
			if (rf_status == RFST_POLL_ACTIVE)
			{
				g_apiCmd.out_data_len = 0; //之后的回调中会用到该变量，此时需要清零
				// NCI_RW_PROCESS
				while (g_apiCmd.in_data_len > NCI_SINGLE_FRAME_MAX_LEN)
				{
					nci_send_packet.PBF = PBF_NCI_SET; //有后续分包
					memcpy(nci_send_packet.PAYLOAD, g_apiCmd.p_in_data, NCI_SINGLE_FRAME_MAX_LEN);
					nci_send_packet.PAYLOAD_LEN = NCI_SINGLE_FRAME_MAX_LEN;
					retStatus					= NFCC_NciCmdProcess(NCI_RW_PROCESS_NEED_NEXT); //
					g_apiCmd.in_data_len -= NCI_SINGLE_FRAME_MAX_LEN;
					g_apiCmd.p_in_data += NCI_SINGLE_FRAME_MAX_LEN;
					NFCC_Poll(); //其他NTF处理
				}
				
				nci_send_packet.PBF = PBF_NCI_RESET; 
				memcpy(nci_send_packet.PAYLOAD, g_apiCmd.p_in_data, g_apiCmd.in_data_len);
				nci_send_packet.PAYLOAD_LEN = g_apiCmd.in_data_len;
				retStatus					= NFCC_NciCmdProcess(NCI_RW_PROCESS); //
				NFCC_Poll(); //其他NTF处理
			}

			if (retStatus != CMD_RECE_OK)
			{
				g_apiCmd.out_data_len = 0; //表示RW 失败
				NFC_TRACE_WARNING("func:%s; line:%d,retStatus:%#x", __FUNCTION__, __LINE__, retStatus);
//				retStatus = CMD_RECE_OK; //有可能是卡片离场导致的失败，此时没有必要报错给应用层
			}

			break;
		case NFCC_CMD_HCI_SIM_ENABLE:
			if (retStatus == CMD_RECE_OK)
			{
				retStatus = NFCC_NciCmdProcess(NCI_NFCEE_SUPERSIM_ENABLE); //
			}
			break;
		case NFCC_CMD_HCI_SE_ENABLE:
			if (retStatus == CMD_RECE_OK)
			{
				retStatus = NFCC_NciCmdProcess(NCI_NFCEE_ESE_ENABLE); //
			}
			break;
		}
		
		if (g_apiCmd.UserTimeoutMask == 0xFFFFFFFF) 
		{
			g_apiCmd.UserTimeoutMask = 0;//只有NFCC_RwTransceive会使用该参数传递用户设置的超时时间，其他接口暂不使用，用完后必须清空
		}

		if (retStatus != CMD_RECE_OK)
		{
			NFC_TRACE_ERROR("func:%s; line:%d,&&&&&&&&&&&&&&&&&&&&&@@@@@ ERROR @@@@@@@@@&&&&&&&&&", __FUNCTION__, __LINE__);
			// NFCC_Start(); /**出错处理，调试时可以注释掉 */  //提供给客户时要打开//SDK不再自己调用，而是用户程序根据返回值外部调用
			//g_apiCmd.cmd_status = NFCC_RESULT_FAILED;//函数入口设置该状态值为NFCC_RESULT_FAILED, 注册函数中如果把该状态赋值了，此处不应再次赋值
		}
		else
		{
			g_apiCmd.cmd_status = NFCC_RESULT_SUCCESS; 
		}
		g_apiCmd.cmd_end_flag = 1;
		NFCC_Poll(); //其他NTF处理
	}
}

t_NFCC_Result NFCC_SetRouteList(t_RoutTablePra *pRouteList, uint8_t num)
{
	uint16_t i			= 0;
	t_RoutTablePra *ptr = pRouteList;

	if (pRouteList == NULL || num == 0)
	{
		NFC_TRACE_ERROR("para error, pRouteList:%p, num:%d", (void *)pRouteList, num);
		return NFCC_RESULT_PARA_ERR;
	}
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_CHANGE_ROUT;

	g_apiCmd.p_in_data = g_CmdInDataBuf;

	g_CmdInDataBuf[i++] = 0x00; //Last message indicator
	g_CmdInDataBuf[i++] = num;	//Number of Routing Entries

	while (num--)
	{
		ptr					= pRouteList++;
		g_CmdInDataBuf[i++] = ptr->qualifier_type; //Routing Entry 1

		if ((ptr->qualifier_type & 0x0f) == 0)
		{
			g_CmdInDataBuf[i++] = 0x03;				//len
			g_CmdInDataBuf[i++] = ptr->nfcee_id;	//nfceeid power state technology
			g_CmdInDataBuf[i++] = ptr->power_state; //
			g_CmdInDataBuf[i++] = ptr->technology;	//
		}
		else if ((ptr->qualifier_type & 0x0f) == 0x01)
		{
			g_CmdInDataBuf[i++] = 0x03;				//len
			g_CmdInDataBuf[i++] = ptr->nfcee_id;	//nfceeid power state protocol
			g_CmdInDataBuf[i++] = ptr->power_state; //
			g_CmdInDataBuf[i++] = ptr->protocol;	//
		}
		else if ((ptr->qualifier_type & 0x0f) == 0x02)
		{
			g_CmdInDataBuf[i++] = 0x02 + (ptr->aid_len); //len
			g_CmdInDataBuf[i++] = ptr->nfcee_id;		 //nfceeid power state aid
			g_CmdInDataBuf[i++] = ptr->power_state;		 //
			memcpy(&g_CmdInDataBuf[i], ptr->aid, ptr->aid_len);
			i += ptr->aid_len;
		}
		else if ((ptr->qualifier_type & 0x0f) == 0x03)
		{
			g_CmdInDataBuf[i++] = 0x02 + (ptr->sys_code_len); //len
			g_CmdInDataBuf[i++] = ptr->nfcee_id;			  //
			g_CmdInDataBuf[i++] = ptr->power_state;			  //
			memcpy(&g_CmdInDataBuf[i], ptr->sys_code, ptr->sys_code_len);
			i += ptr->sys_code_len;
		}
	}
	g_apiCmd.in_data_len = i;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_ChangeRoute(uint8_t ucRoute, uint8_t *Aid)
{
	t_RoutTablePra routeList;

	if (ucRoute != LOCAL_M1 && ucRoute != ESE_HOST && ucRoute != SUPER_SIM_HOST && ucRoute != DH_HOST)
	{
		NFC_TRACE_ERROR("para error, ucRoute:%#x", ucRoute);
		return NFCC_RESULT_PARA_ERR;
	}

	if ((ucRoute == DH_HOST) && (Aid == NULL))
	{
		//NFC_TRACE_ERROR("para error, ucRoute:%p, Aid:%p", (void *)ucRoute, (void *)Aid);
		return NFCC_RESULT_PARA_ERR;
	}

	g_apiCmd.para1 = ucRoute;

	if (ucRoute != DH_HOST)
	{
		routeList.qualifier_type = 0x00;
		routeList.nfcee_id		 = ucRoute;
		routeList.power_state	 = 0x3b;
		routeList.technology	 = 0x00;
	}
	else
	{
		routeList.qualifier_type = 0x42;
		routeList.nfcee_id		 = ucRoute;
		routeList.power_state	 = 0x01;
		memcpy(routeList.aid, Aid, 8);
		routeList.aid_len = 8;
	}

	return NFCC_SetRouteList(&routeList, 1);
}

/**
 * 客户应用中调用了此接口后，会在应用中做主控掉电的操作
 */
t_NFCC_Result NFCC_PowDownCe(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_POWDOWN_CE;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_DeInit(void)
{
	//t_NFCC_Result res = NFCC_PowDownCe();

	//port_nfcc_io_deinit(); //关闭IO口

	//port_timer_delay_ms(10); //延时10ms
	port_hardware_gpioVenPulldown();
	// port_timer_delay_ms(10); //延时10ms
	// port_hardware_gpio1V8Pullup();
	// port_timer_delay_ms(10); //延时10ms
	// port_hardware_gpio1V8Pulldown();

	//会进入 Off——plus 模式
	//重新启动NFCC功能需要重新跑NFCC_Init
	return NFCC_RESULT_SUCCESS;
}

t_NFCC_Result NFCC_Init(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RESET;
	g_apiCmd.p_in_data		= g_CmdInDataBuf;
	g_apiCmd.in_data_len	= 0;
	g_apiCmd.p_out_data		= g_CmdInDataBuf;
	g_apiCmd.out_data_len	= 0;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_register_callback(sdk_callback_info_t *sdk_callback)
{
	if (sdk_callback == NULL) 
	{
		NFC_TRACE_ERROR("para error, sdk_callback:%p", (void *)sdk_callback);
		return NFCC_RESULT_PARA_ERR;
	}

	if (sdk_callback->sdk_callback_fun_handle_event == NULL)
	{
		NFC_TRACE_ERROR("para error, sdk_callback->sdk_callback_fun_handle_event:%p", 
			(void *)sdk_callback->sdk_callback_fun_handle_event);
		return NFCC_RESULT_PARA_ERR;
	}
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_REGISTER_CB;
	g_apiCmd.impl_intf		= (void*)sdk_callback;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}


#if NFCC_SOC2_SUPPORT
t_NFCC_Result NFCC_factoryInit(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RESET;
	g_apiCmd.p_in_data		= g_CmdInDataBuf;
	g_apiCmd.in_data_len	= 0;
	g_apiCmd.p_out_data		= g_CmdInDataBuf;
	g_apiCmd.out_data_len	= 0;
	g_factory_init_flag		= 1;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}
#endif

/**
 * 
 * type 1:first packet
 * type 0:next packet
 * 
 * 用于解决性能问题，若是多个apdu_gate指令连续发送，可以只在第一包做set mode处理，后续不用重复处理，避免时间浪费
 */
t_NFCC_Result NFCC_GateSendApdu(uint8_t route, uint8_t *pucInApduData, uint16_t usInLen, uint8_t *pucRes, uint16_t *pOutLen, uint16_t usOutBufSize, bool type)
{
	if ((route != ESE_HOST && route != SUPER_SIM_HOST) || (pucInApduData == NULL) || (usInLen == 0) || (pucRes == NULL) || (pOutLen == NULL))
	{
		//NFC_TRACE_ERROR("para error, route:%#x, pucInApduData:%#x, usInLen:%d, pucRes:%#x, pOutLen:%#x", 
		//							route, pucInApduData, usInLen, pucRes, pOutLen);
		return NFCC_RESULT_PARA_ERR;
	}
	g_apdu_gate_first_packet   = type;
	g_apiCmd.cmd_end_flag	   = 0;
	g_apiCmd.cmd_start_flag	   = 1;
	g_apiCmd.cmd			   = NFCC_CMD_SEND_APDU_GATE;
	g_apiCmd.para1			   = route;
	g_apiCmd.p_in_data		   = pucInApduData;
	g_apiCmd.in_data_len	   = usInLen;
	g_apiCmd.p_out_data		   = pucRes;
	g_apiCmd.out_data_len	   = 0;
	g_apiCmd.out_data_max_size = usOutBufSize;
	NFCC_OtherCmdPoll();
	*pOutLen = g_apiCmd.out_data_len;
	return g_apiCmd.cmd_status;
}

static t_NFCC_Result NFCC_LocalM1CardInfoChange(uint8_t app_type, uint8_t *pucInData, uint16_t usLen) //create or change
{
	if ((pucInData == NULL) || (usLen == 0))
	{
		NFC_TRACE_ERROR("para error, pucInData:%p, usLen:%#x", (void *)pucInData, usLen);
		return NFCC_RESULT_PARA_ERR;
	}
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_LOCAL_M1_CREATE;
	g_apiCmd.para1			= app_type;
	g_apiCmd.p_in_data		= pucInData;
	g_apiCmd.in_data_len	= usLen;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_LocalM1CardChange(uint8_t app_type) //切换卡片
{
	if (((g_NfcInfo.LocalM1IsExit >> app_type) & 0x01) != 0x01)
	{
		NFC_TRACE_ERROR("para error, app_type:%#x", app_type);
		return NFCC_RESULT_FAILED; //卡不存在，需要重新创建，由于不知道用户需要的APP信息，因此需要报告给用户，由应用来创建
	}
	if ((g_NfcInfo.LocalM1ActiveAppId == app_type) && (g_NfcInfo.LocalM1IsAcitive == 1)) //当前使用的就是目标卡片
	{
		return NFCC_RESULT_SUCCESS;
	}

	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_LOCAL_M1_CHANGE;
	g_apiCmd.para1			= app_type;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_LocalM1Delete(uint8_t app_type) //删除卡片
{
	if (((g_NfcInfo.LocalM1IsExit >> app_type) & 0x01) != 0x01)
	{
		NFC_TRACE_ERROR("para error, app_type:%#x", app_type);
		return NFCC_RESULT_FAILED;
	}

	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_LOCAL_M1_DELETE;
	g_apiCmd.para1			= app_type;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_LocalM1Query(uint8_t app_type, uint8_t *pOutData) //查询卡片APP --读取指定应用存储区中sector0的block0块前8字节系统数据块
{
	if ((((g_NfcInfo.LocalM1IsExit >> app_type) & 0x01) != 0x01) || (pOutData == NULL))
	{
		NFC_TRACE_ERROR("para error, app_type:%#x, pOutData:%p", app_type, (void *)pOutData);
		return NFCC_RESULT_FAILED;
	}

	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_LOCAL_M1_QUERY;
	g_apiCmd.para1			= app_type;
	g_apiCmd.p_out_data		= pOutData;
	g_apiCmd.out_data_len	= 0;
	NFCC_OtherCmdPoll();

	if (g_apiCmd.out_data_len == 8)
	{
		return NFCC_RESULT_SUCCESS;
	}
	{
		return NFCC_RESULT_FAILED;
	}
}

t_NFCC_Result NFCC_LocalM1WriteSector(uint8_t app_type, uint8_t sector_number, uint16_t block_bit_map, uint8_t *p_password, uint8_t *p_in_data_lv)
{
	uint8_t i = 0;
	uint8_t k = 0;
	uint8_t bytes_cnt = 0;

	for (k = 0; k < 4; k++)
	{
		if (block_bit_map & (1 << k))
		{
			bytes_cnt += 16; //M1 CLASSIC , 16 byte per block
		}
	}
	if ((p_password == NULL) || (p_in_data_lv == NULL))
	{
		NFC_TRACE_ERROR("para error, p_password:%p, p_in_data_lv:%p", (void*)p_password, (void*)p_in_data_lv);
		return NFCC_RESULT_PARA_ERR;
	}	

	if (sector_number > 15) {
		NFC_TRACE_ERROR("para error, sector_number:%#x", sector_number);
		return NFCC_RESULT_PARA_ERR;
	}
	if ((block_bit_map & 0x0f) == 0) {
		NFC_TRACE_ERROR("para error, all low 4 bits are 0. block_bit_map:%#x", block_bit_map);
		return NFCC_RESULT_PARA_ERR;
	}

	if (*p_in_data_lv != bytes_cnt) {
		//NFC_TRACE_ERROR("para error, bytes[%#x] by block_bit_map donot map array len[%d]:%p", bytes_cnt, *p_in_data_lv);
		return NFCC_RESULT_PARA_ERR;
	}

	if (((g_NfcInfo.LocalM1IsExit >> app_type) & 0x01) != 0x01)
	{
		NFC_TRACE_DEBUG("func:%s; line:%d, test localM1 errr", __FUNCTION__, __LINE__);
		return NFCC_RESULT_FAILED; //卡不存在，需要重新创建，由于不知道用户需要的APP信息，因此需要报告给用户
	}

	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_LOCAL_M1_WRITE_SECTOR;
	// g_apiCmd.para1		  = app_type; //暂不使用
	g_CmdInDataBuf[i++] = app_type;
	g_CmdInDataBuf[i++] = sector_number;
	g_CmdInDataBuf[i++] = block_bit_map;
	memcpy(&g_CmdInDataBuf[i], p_password, 8);
	i += 8;

	memcpy(&g_CmdInDataBuf[i], (p_in_data_lv + 1), bytes_cnt);
	i += bytes_cnt;

	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.p_out_data	  = g_CmdOutDataBuf;
	g_apiCmd.out_data_len = 0;
	// NFC_TRACE_DEBUG("write localM1 sector");
	NFCC_OtherCmdPoll();

	// NFC_TRACE_DEBUG("g_apiCmd.out_data_len = 0x%02x", g_apiCmd.out_data_len);
	// NFC_TRACE_DEBUG("g_apiCmd.p_out_data[0] = 0x%02x", g_apiCmd.p_out_data[0]);

	if ((g_apiCmd.out_data_len == 1) && (g_CmdOutDataBuf[0] == 0))
	{
		Clear_g_CmdOutDataBuf();
		return g_apiCmd.cmd_status;
	}
	else
	{
		Clear_g_CmdOutDataBuf();
		NFC_TRACE_DEBUG("func:%s; line:%d, test localM1 errr", __FUNCTION__, __LINE__);
		return NFCC_RESULT_FAILED;
	}
}
t_NFCC_Result NFCC_LocalM1ReadSector(uint8_t app_type, uint8_t sector_number, uint16_t block_bit_map, uint8_t *p_password, uint8_t *p_out_data_lv)
{
	uint8_t i = 0;
	uint8_t k = 0;
	uint8_t bytes_cnt = 0;

	for (k = 0; k < 4; k++)
	{
		if (block_bit_map & (1 << k))
		{
			bytes_cnt += 16; //M1 CLASSIC , 16 byte per block
		}
	}
	
	if ((p_password == NULL) || (p_out_data_lv == NULL))
	{
		NFC_TRACE_ERROR("para error, p_password:%p, p_out_data_lv:%p", p_password, p_out_data_lv);
		return NFCC_RESULT_PARA_ERR;
	}

	if (sector_number > 15) {
		NFC_TRACE_ERROR("para error, sector_number:%#x", sector_number);
		return NFCC_RESULT_PARA_ERR;
	}
	if ((block_bit_map & 0x0f) == 0) {
		NFC_TRACE_ERROR("para error, all low 4 bits are 0. block_bit_map:%#x", block_bit_map);
		return NFCC_RESULT_PARA_ERR;
	}

	if (((g_NfcInfo.LocalM1IsExit >> app_type) & 0x01) != 0x01)
	{
		NFC_TRACE_DEBUG("func:%s; line:%d, test localM1 errr", __FUNCTION__, __LINE__);
		return NFCC_RESULT_FAILED; //卡不存在，需要重新创建，由于不知道用户需要的APP信息，因此需要报告给用户
	}

	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_LOCAL_M1_READ_SECTOR;
	// g_apiCmd.para1		  = app_type;
	g_CmdInDataBuf[i++] = app_type;
	g_CmdInDataBuf[i++] = sector_number;
	g_CmdInDataBuf[i++] = block_bit_map;
	memcpy(&g_CmdInDataBuf[i], p_password, 8);
	i += 8;
	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.p_out_data	  = g_CmdOutDataBuf; //将数据输出到这个地址
	g_apiCmd.out_data_max_size	  = sizeof(g_CmdOutDataBuf);
	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("read localM1 sector");
	NFCC_OtherCmdPoll();

	if (g_apiCmd.out_data_len == 0)
	{
		Clear_g_CmdOutDataBuf();
		NFC_TRACE_ERROR("PROP_LOCAL_M1_READ_SECTOR_RSP, payload len is 0 errr");
		return NFCC_RESULT_FAILED;
	}
	if ((g_CmdOutDataBuf[0] != 0))
	{
		Clear_g_CmdOutDataBuf();
		NFC_TRACE_ERROR("PROP_LOCAL_M1_READ_SECTOR_RSP, STATUS[%#x] errr", g_CmdOutDataBuf[0]);
		return NFCC_RESULT_FAILED;
	}

	if (g_apiCmd.out_data_len != (1 + bytes_cnt))
	{
		Clear_g_CmdOutDataBuf();
		NFC_TRACE_ERROR("PROP_LOCAL_M1_READ_SECTOR_RSP, payload len[%d] errr", g_apiCmd.out_data_len);
		return NFCC_RESULT_FAILED;
	}

	*p_out_data_lv = bytes_cnt;
	memcpy(p_out_data_lv+1, g_CmdOutDataBuf+1, bytes_cnt);
	Clear_g_CmdOutDataBuf();

	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_GetSuperSimUID(uint8_t *pucSuperSimUid)
{
	static uint8_t g_GetSuperSimUidCmd[] = {0xA0, 0x4B};

	if (pucSuperSimUid == NULL)
	{
		NFC_TRACE_ERROR("para error, pucSuperSimUid:%p", pucSuperSimUid);
		return NFCC_RESULT_PARA_ERR;
	}

	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_GET_SURPERSIM_UID;
	g_apiCmd.para1			= 0x00; //nocare
	g_apiCmd.p_in_data		= g_GetSuperSimUidCmd;
	g_apiCmd.in_data_len	= sizeof(g_GetSuperSimUidCmd);
	g_apiCmd.p_out_data		= pucSuperSimUid;
	//g_apiCmd.out_data_len	= 0x08;
	NFCC_OtherCmdPoll();

	NFC_TRACE_DEBUG("SuperSimUID len:%d", g_apiCmd.out_data_len);
	NFC_TRACE_BUFFER_INFO("SuperSimUID:", g_apiCmd.p_out_data, g_apiCmd.out_data_len);
	return g_apiCmd.cmd_status;
}
t_NFCC_Result NFCC_GetEseUID(uint8_t *pucEseUid)
{
	static uint8_t g_GetEseUidCmd[] = {0xA0, 0x4A};

	if (pucEseUid == NULL)
	{
		NFC_TRACE_ERROR("para error, pucEseUid:%p", pucEseUid);
		return NFCC_RESULT_PARA_ERR;
	}
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_GET_ESE_UID;
	g_apiCmd.para1			= 0x00; //no care
	g_apiCmd.p_in_data		= g_GetEseUidCmd;
	g_apiCmd.in_data_len	= sizeof(g_GetEseUidCmd);
	g_apiCmd.p_out_data		= pucEseUid;
	//g_apiCmd.out_data_len	= 0x08;
	NFCC_OtherCmdPoll();

	NFC_TRACE_DEBUG("EseUID len:%d", g_apiCmd.out_data_len);
	NFC_TRACE_BUFFER_INFO("EseUID:", g_apiCmd.p_out_data, g_apiCmd.out_data_len);
	return g_apiCmd.cmd_status;
}
t_NFCC_Result NFCC_SuperSimIn(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_SURPER_SIM_IN;
	g_apiCmd.para1			= 0x00; //nocare
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_OpenRwMode(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_OPEN_RW;
	g_apiCmd.para1			= g_nfc_config.ce_route; //打开读写
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_OpenCeMode(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_CLOSE_RW;
	g_apiCmd.para1			= g_nfc_config.ce_route; //关闭读写
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_RwReDiscovery(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RE_DISCOVERY;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_HceSendData(uint8_t *pInData, uint16_t usLen)
{
	if (pInData == NULL || usLen == 0) 
	{
		NFC_TRACE_ERROR("error, pInData:%p, usLen:%d", pInData, usLen);
		return NFCC_RESULT_PARA_ERR;
	}
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_HCE_RESPONSE;
	g_apiCmd.p_in_data		= pInData;
	g_apiCmd.in_data_len	= usLen;
	g_apiCmd.is_fido = false;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_HceFIDOSendData(uint8_t *pInData, uint16_t usLen)
{
    if (pInData == NULL || usLen == 0) 
    {
        NFC_TRACE_ERROR("error, pInData:%p, usLen:%d", pInData, usLen);
        return NFCC_RESULT_PARA_ERR;
    }
    is_fido_packet = true;
    g_apiCmd.cmd_end_flag   = 0;
    g_apiCmd.cmd_start_flag = 1;
    g_apiCmd.cmd            = NFCC_CMD_HCE_RESPONSE;
    g_apiCmd.p_in_data      = pInData;
    g_apiCmd.in_data_len    = usLen;
    g_apiCmd.is_fido = true;
    NFCC_OtherCmdPoll();
    return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_RwSelect(uint8_t card_num)
{
	uint8_t test_data[3];
	CARD_PARA *CardPara;
	if (card_num >= 2)
	{
		NFC_TRACE_ERROR("error, card_num:%#x", card_num);
		return NFCC_RESULT_PARA_ERR;
	}
	CardPara				= &NfcCardinfo[card_num];
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_SELECT_CARD;

	NFC_TRACE_DEBUG(" CardPara->RF_Discovery_ID = %02X", CardPara->RF_Discovery_ID);
	NFC_TRACE_DEBUG(" CardPara->RF_Protocol = %02X", CardPara->RF_Protocol);
	NFC_TRACE_DEBUG(" CardPara->RF_Interface = %02X", CardPara->RF_Interface);

	test_data[0]		 = CardPara->RF_Discovery_ID;
	test_data[1]		 = CardPara->RF_Protocol;
	test_data[2]		 = CardPara->RF_Interface;
	g_apiCmd.p_in_data	 = test_data;
	g_apiCmd.in_data_len = 3;
	NFCC_OtherCmdPoll();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_RwMifareAuth(uint8_t type, uint8_t sector_id, uint8_t *p_in_key)
{
#define M1_KEY_LEN 6
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if (p_in_key == NULL) 
	{
		NFC_TRACE_ERROR("para error, p_in_key:%p", p_in_key);
		return NFCC_RESULT_PARA_ERR;
	}
	
	if (sector_id > 16) 
	{
		NFC_TRACE_ERROR("para error, sector_id:%#x", sector_id);
		return NFCC_RESULT_PARA_ERR;
	}

	if((type != 1) && (type != 2))
	{
		NFC_TRACE_ERROR("para error, type:%#x", type);
		return NFCC_RESULT_PARA_ERR;
	}
	
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	g_CmdInDataBuf[i++] = NFCC_RW_M1_AUTH + type;
	g_CmdInDataBuf[i++] = sector_id * 4;
	g_CmdInDataBuf[i++] = 0; //默认key selector 0
	memcpy(&g_CmdInDataBuf[i], p_in_key, M1_KEY_LEN);
	i += M1_KEY_LEN;
	g_apiCmd.p_in_data	  = g_CmdInDataBuf;
	g_apiCmd.in_data_len  = i;
	g_apiCmd.p_out_data	  = g_CmdOutDataBuf;
	g_apiCmd.out_data_max_size = sizeof(g_CmdOutDataBuf);
	g_apiCmd.out_data_len = i;

	// NFC_TRACE_DEBUG("NFCC_RwMifareAuth");
	NFCC_OtherCmdPoll();

	// NFC_TRACE_DEBUG("g_apiCmd.out_data_len = 0x%02x", g_apiCmd.out_data_len);
	// NFC_TRACE_DEBUG("g_apiCmd.p_out_data[0] = 0x%02x", g_apiCmd.p_out_data[0]);

	if ((g_apiCmd.out_data_len == 1) && (g_CmdOutDataBuf[0] == 0))
	{
		Clear_g_CmdOutDataBuf();
		return g_apiCmd.cmd_status;
	}
	else
	{
		Clear_g_CmdOutDataBuf();
		return NFCC_RESULT_FAILED;
	}
}
/**
 * M1扇区读取，返回的数据为17个字节 ：16个字节扇区数据+00
 */
t_NFCC_Result NFCC_RwMifareReadBlock(uint8_t block_id, uint8_t *pOutData)
{
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if (pOutData == NULL) 
	{
		NFC_TRACE_ERROR("error, pOutData:%p", pOutData);
		return NFCC_RESULT_PARA_ERR;
	}

	if (block_id >= 64)
	{
		NFC_TRACE_ERROR("para error, block_id:%#x", block_id);
		return NFCC_RESULT_PARA_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	g_CmdInDataBuf[i++] = NFCC_RW_M1_READ; //read cmd
	g_CmdInDataBuf[i++] = block_id;

	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.p_out_data	  = pOutData;
	g_apiCmd.out_data_max_size = 17;
	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RwMifareReadBlock");
	NFCC_OtherCmdPoll();

	// NFC_TRACE_DEBUG("g_apiCmd.out_data_len = 0x%02x", g_apiCmd.out_data_len);
	// NFC_TRACE_DEBUG("g_apiCmd.p_out_data[0] = 0x%02x", g_apiCmd.p_out_data[0]);

	if (g_apiCmd.out_data_len == 0x11)
	{
		return g_apiCmd.cmd_status;
	}
	else
	{
		return NFCC_RESULT_FAILED;
	}
}

t_NFCC_Result NFCC_RwMifareWriteBlock(uint8_t block_id, uint8_t *p_in_buf)
{
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if (p_in_buf == NULL) 
	{
		NFC_TRACE_ERROR("para error, p_in_buf:%p", p_in_buf);
		return NFCC_RESULT_PARA_ERR;
	}

	if (block_id >= 64)
	{
		NFC_TRACE_ERROR("para error, block_id:%#x", block_id);
		return NFCC_RESULT_PARA_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	g_CmdInDataBuf[i++] = NFCC_RW_M1_WRITE; //write cmd   part 1
	g_CmdInDataBuf[i++] = block_id;


	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.p_out_data	  = g_CmdOutDataBuf;
	g_apiCmd.out_data_max_size = sizeof(g_CmdOutDataBuf);
	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RwMifareWriteBlock1");
	NFCC_OtherCmdPoll();

	if (g_apiCmd.out_data_len == 0)
	{
		Clear_g_CmdOutDataBuf();
		return NFCC_RESULT_FAILED;
	}

	if (!(g_CmdOutDataBuf[0] == 0x0A || g_CmdOutDataBuf[0] == 0x00))
	{
		Clear_g_CmdOutDataBuf();
		return NFCC_RESULT_FAILED;
	}

	i						= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	memcpy(&g_CmdInDataBuf[0], p_in_buf, 0x10); //part 2 data
	i += 0x10;
	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RwMifareReadBlock2");
	NFCC_OtherCmdPoll();

	// NFC_TRACE_DEBUG("g_apiCmd.out_data_len = 0x%02x", g_apiCmd.out_data_len);
	// NFC_TRACE_DEBUG("g_apiCmd.p_out_data[0] = 0x%02x", g_apiCmd.p_out_data[0]);

	if (g_apiCmd.out_data_len == 0)
	{
		Clear_g_CmdOutDataBuf();
		return NFCC_RESULT_FAILED;
	}

	if (!(g_CmdOutDataBuf[0] == 0x0A || g_CmdOutDataBuf[0] == 0x00))
	{
		Clear_g_CmdOutDataBuf();
		return NFCC_RESULT_FAILED;
	}

	Clear_g_CmdOutDataBuf();
	return g_apiCmd.cmd_status;
}

/**
 * T2t 读
 */

t_NFCC_Result NFCC_RwT2tReadBlock(uint8_t block_id, uint8_t *pOutData)
{
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if (pOutData == NULL) 
	{
		NFC_TRACE_ERROR("error, pOutData:%p", pOutData);
		return NFCC_RESULT_PARA_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	g_CmdInDataBuf[i++] = NFCC_RW_T2T_READ; //read cmd
	g_CmdInDataBuf[i++] = block_id;

	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.p_out_data	  = pOutData;
	g_apiCmd.out_data_max_size = 0x11;
	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RwT2tReadBlock");
	NFCC_OtherCmdPoll();

	// NFC_TRACE_DEBUG("g_apiCmd.out_data_len = 0x%02x", g_apiCmd.out_data_len);
	// NFC_TRACE_DEBUG("g_apiCmd.p_out_data[0] = 0x%02x", g_apiCmd.p_out_data[0]);

	if (g_apiCmd.out_data_len == 0x11)
	{
		return g_apiCmd.cmd_status;
	}
	else
	{
		return NFCC_RESULT_FAILED;
	}
}

/**
 * T2t 写
 * 
 */
t_NFCC_Result NFCC_RwT2tWriteBlock(uint8_t block_id, uint8_t *p_in_buf)
{
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if (p_in_buf == NULL) 
	{
		NFC_TRACE_ERROR("error, pOutData:%p", p_in_buf);
		return NFCC_RESULT_PARA_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	g_CmdInDataBuf[i++] = NFCC_RW_T2T_WRITE; //write cmd
	g_CmdInDataBuf[i++] = block_id;
	memcpy(&g_CmdInDataBuf[i], p_in_buf, 4);
	i = i + 4;

	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.p_out_data	  = g_CmdOutDataBuf;
	g_apiCmd.out_data_max_size	  = sizeof(g_CmdOutDataBuf);
	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RwT2tWriteBlock");
	NFCC_OtherCmdPoll();

	if (g_apiCmd.out_data_len == 0)
	{
		Clear_g_CmdOutDataBuf();
		return NFCC_RESULT_FAILED;
	}

	Clear_g_CmdOutDataBuf();
	return g_apiCmd.cmd_status;
}

/**
 * T2t 读
 */

t_NFCC_Result NFCC_RwT1tReadBlock(uint8_t block_id, uint8_t *pOutData)
{
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if (pOutData == NULL) 
	{
		NFC_TRACE_ERROR("error, pOutData:%p", pOutData);
		return NFCC_RESULT_PARA_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	g_CmdInDataBuf[i++] = 0x30; //read cmd
	g_CmdInDataBuf[i++] = block_id;

	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.p_out_data	  = pOutData;
	g_apiCmd.out_data_max_size = 0x11;
	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RwT2tReadBlock");
	NFCC_OtherCmdPoll();

	// NFC_TRACE_DEBUG("g_apiCmd.out_data_len = 0x%02x", g_apiCmd.out_data_len);
	// NFC_TRACE_DEBUG("g_apiCmd.p_out_data[0] = 0x%02x", g_apiCmd.p_out_data[0]);

	if (g_apiCmd.out_data_len == 0x11)
	{
		return g_apiCmd.cmd_status;
	}
	else
	{
		return NFCC_RESULT_FAILED;
	}
}

/**
  * T2t 写
  * 
  */
t_NFCC_Result NFCC_RwT1tWriteBlock(uint8_t block_id, uint8_t *p_in_buf)
{
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if (p_in_buf == NULL) 
	{
		NFC_TRACE_ERROR("error, p_in_buf:%p", p_in_buf);
		return NFCC_RESULT_PARA_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	g_CmdInDataBuf[i++] = 0xA2; //write cmd   part 1
	g_CmdInDataBuf[i++] = block_id;
	memcpy(&g_CmdInDataBuf[i], p_in_buf, 4);
	i = i + 4;

	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = i;

	g_apiCmd.p_out_data	  = g_CmdOutDataBuf;
	g_apiCmd.out_data_max_size	  = sizeof(g_CmdOutDataBuf);

	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RwT2tWriteBlock");
	NFCC_OtherCmdPoll();

	if (g_apiCmd.out_data_len == 0)
	{
		Clear_g_CmdOutDataBuf();
		return NFCC_RESULT_FAILED;
	}

	Clear_g_CmdOutDataBuf();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_RfData(uint8_t *p_in_buf, uint8_t len)
{
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if (p_in_buf == NULL) 
	{
		NFC_TRACE_ERROR("error, p_in_buf:%p", p_in_buf);
		return NFCC_RESULT_PARA_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	memcpy(&g_CmdInDataBuf[0], p_in_buf, len);

	g_apiCmd.p_in_data = g_CmdInDataBuf;
	
	g_apiCmd.p_out_data	  = g_CmdOutDataBuf;
	g_apiCmd.out_data_max_size	  = sizeof(g_CmdOutDataBuf);
	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RfData");
	NFCC_OtherCmdPoll();
	Clear_g_CmdOutDataBuf();
	return g_apiCmd.cmd_status;
}

t_NFCC_Result NFCC_RwT4tApduCmd(uint8_t *pInApduBuf, uint8_t ucInLen, uint8_t *pOutApduRes, uint16_t *pOutLen)
{
    NFC_TRACE_ERROR("NFCC_RwT4tApduCmd pOutLen = %d", *pOutLen);
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	
	if ((pInApduBuf == NULL) || (pOutApduRes == NULL) || (pOutLen == NULL) || ucInLen == 0)
	{
		NFC_TRACE_ERROR("error, pInApduBuf:%p, pOutApduRes:%p, pOutLen:%p, ucInLen:%d", 
			pInApduBuf, pOutApduRes, pOutLen, ucInLen);
		return NFCC_RESULT_PARA_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	memcpy(g_CmdInDataBuf, pInApduBuf, ucInLen);
	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = ucInLen;

	g_apiCmd.p_out_data	  = pOutApduRes;
	g_apiCmd.out_data_max_size	  = *pOutLen;//没有传入最大限制值，以该输入项输入最大输出范围
	g_apiCmd.out_data_len = 0;

	// NFC_TRACE_DEBUG("NFCC_RwT4tApduCmd");
	NFCC_OtherCmdPoll();
    NFC_TRACE_ERROR("NFCC_RwT4tApduCmd len = %d", g_apiCmd.out_data_len);
    *pOutLen = g_apiCmd.out_data_len;
    return g_apiCmd.cmd_status;
}

/**
* para TimeoutMask: 
		0xFFFFFFFF: return right now;
		other: The response data is required according to the command;
*/
t_NFCC_Result NFCC_RwTransceive(uint8_t *pInBuf, uint16_t ucInLen, uint8_t *pOutRes, uint16_t *pOutLen, uint32_t TimeoutMask)
{
	//pTimeoutMs为NULL也是一种正常状态，不能防护其为空
	if ((pInBuf == NULL) || (ucInLen == 0) || (pOutRes == NULL) || (pOutLen == NULL))
	{
		NFC_TRACE_ERROR("error, pInBuf:%p, ucInLen:%d, pOutRes:%p, pOutLen:%p", pInBuf, ucInLen, pOutRes, pOutLen);
		return NFCC_RESULT_PARA_ERR;
	}
	t_NFCC_rf_Status RF_state = NFCC_GetRFGlobalStatus();
	if (RF_state != RFST_POLL_ACTIVE)
	{
		NFC_TRACE_ERROR("NFCC_GetRFGlobalStatus:%d", RF_state);
		return NFCC_RESULT_DH_SEMANTIC_ERR;
	}
	uint8_t i				= 0;
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_RW_PROCESS;
	g_apiCmd.para1			= 0x00; //nocare

	memcpy(g_CmdInDataBuf, pInBuf, ucInLen);
	g_apiCmd.p_in_data	 = g_CmdInDataBuf;
	g_apiCmd.in_data_len = ucInLen;

	g_apiCmd.p_out_data	  = pOutRes;
	g_apiCmd.out_data_len = 0;
	g_apiCmd.out_data_max_size = *pOutLen;
	g_apiCmd.UserTimeoutMask = TimeoutMask;

	// NFC_TRACE_DEBUG("NFCC_RwT4tApduCmd");
	NFCC_OtherCmdPoll();

	*pOutLen = g_apiCmd.out_data_len;
	if (TimeoutMask == 0xFFFFFFFF)
	{
		*pOutLen = 0;

	}
	return g_apiCmd.cmd_status;
}
static t_NFCC_Result NFCC_M1InfoPack(uint8_t app_type, uint8_t *system_data, uint8_t *initial_key_set, uint8_t *res_date)
{
	//COMMON_NCI_PACKET nci_send_packet;
	COMMON_NCI_PACKET nci_recv_packet;
	//param check
	if (app_type > 3)
	{
		NFC_TRACE_ERROR("error, app_type:%#x", app_type);
		return NFCC_RESULT_FAILED; //失败
	}

	//check null
	if ((system_data == NULL) || (initial_key_set == NULL) || (res_date == NULL))
	{
		NFC_TRACE_ERROR("error, system_data:%p, initial_key_set:%p, res_date:%p", system_data, initial_key_set, res_date);
		return NFCC_RESULT_PARA_ERR; //失败
	}

	memcpy((res_date), (system_data), 4);
	*(res_date + 4) = system_data[0] ^ system_data[1] ^ system_data[2] ^ system_data[3];
	*(res_date + 5) = 0x08;
	*(res_date + 6) = 0x04;
	*(res_date + 7) = 0x00;
	memcpy((res_date + 8), (initial_key_set), 16);

	return NFCC_RESULT_SUCCESS;
}

/*
typedef enum
{
    NCI_RF_PROTOCOL_UNDETERMINED = 0x00,
    NCI_RF_PROTOCOL_T1T          = 0x01,
    NCI_RF_PROTOCOL_T2T          = 0x02,
    NCI_RF_PROTOCOL_T3T          = 0x03,
    NCI_RF_PROTOCOL_ISO_DEP      = 0x04,
    NCI_RF_PROTOCOL_NFC_DEP      = 0x05,
    NCI_RF_PROTOCOL_T5T          = 0x06,
    NCI_RF_PROTOCOL_NDEF         = 0x07,        ///< no support
    // RFU                         0x08 - 0x7F
    NCI_RF_PROTOCOL_M1_RW        = 0x80,
    NCI_RF_PROTOCOL_M1_CE        = 0x81,
    // For proprietary use         0x81 - 0xFE
    // RFU                         0xFF
} NCI_RF_PROTOCOL_t;

*/
#ifndef CONFIG_TESTSUITE
uint8_t NFCC_HceCallBack(uint8_t *pInData, uint16_t usInDataLen)
{
	NFC_TRACE_DEBUG("call back successed");
	return hce_callback(pInData, usInDataLen);
}
#endif

/*	uint8_t offset = 0;
	uint16_t len;
	uint8_t rspBuf[100];

	switch (pInData[1])
	{
	case 0xA4:
		*rspBuf		  = 0x90;
		*(rspBuf + 1) = 0x00;
		len			  = 2;
		break;
	default:
		*rspBuf		  = 0x6D;
		*(rspBuf + 1) = 0x00;
		len			  = 2;
		break;
	}

	return (NFCC_HceSendData(rspBuf, len));
}*/
#ifdef CONFIG_TESTSUITE
uint8_t NFCC_HceCallBack(uint8_t *pInData, uint16_t usInDataLen)
{
	NFC_TRACE_DEBUG("call back successed");
	uint8_t offset = 0;
	uint16_t len;
	uint8_t rspBuf[100];

	switch (pInData[1])
	{
	case 0xA4:
		*rspBuf		  = 0x90;
		*(rspBuf + 1) = 0x00;
		len			  = 2;
		break;
	default:
		*rspBuf		  = 0x6D;
		*(rspBuf + 1) = 0x00;
		len			  = 2;
		break;
	}

	return (NFCC_HceSendData(rspBuf, len));
}
#endif

t_NFCC_Result NFCC_RwWaitCard(NCI_RF_PROTOCOL_t CardProtocol, uint16_t Timeout)
{
	uint32_t i = 0;
	if (CardProtocol < NCI_RF_PROTOCOL_UNDETERMINED || (CardProtocol > NCI_RF_PROTOCOL_NDEF && CardProtocol != NCI_RF_PROTOCOL_M1_RW))
	{
		NFC_TRACE_ERROR("error, CardProtocol:%#x", CardProtocol);
		return NFCC_RESULT_PARA_ERR; //失败
	}

	// NFC_TRACE_DEBUG("--CardProtocol = 0x%02x--------------ok ----2222---------", CardProtocol);

	t_NFCC_rf_Status rf_status;

	//NFCC_OpenRwMode();
	memset(&NfcCardinfo, 0, sizeof(NfcCardinfo)); //清空 RW 卡片区数据
	i = 0;
	while (1)
	{
		if (FALSE == port_get_irq_flag(0))
		{
			port_timer_delay_ms(20);
			//_sleep   此处需要平台任务休眠函数，等待IRQ中断唤醒，降低功耗
		}

		NFCC_Poll(); //其他NTF处理

		if (Timeout != NO_TIMEOUT_FOREVER)
		{
			if ((i++) >= Timeout * 50) //超时退出
			{
				NFC_TRACE_DEBUG("----------------err timeout  no M1 card in-------------");
				return NFCC_RESULT_TIMEOUT; //超时
			}
		}

		if (NfcCardinfo[0].RF_Protocol != 0)
		{
			if ((CardProtocol & NfcCardinfo[0].RF_Protocol) == NfcCardinfo[0].RF_Protocol) //普通卡-或者复合卡第一张为所等待的卡片
			{
				// NFC_TRACE_DEBUG("------------test 11111-------------------");
				// NFC_TRACE_DEBUG("------------CardProtocol = 0x%02x-----------------", CardProtocol);
				// NFC_TRACE_DEBUG("------------NfcCardinfo[0].RF_Protocol = 0x%02x----------------", NfcCardinfo[0].RF_Protocol);
				// NFC_TRACE_DEBUG("------------NfcCardinfo[1].RF_Protocol = 0x%02x----------------", NfcCardinfo[1].RF_Protocol);
				rf_status = NFCC_GetRFGlobalStatus();
				if (rf_status == RFST_W4_HOST_SELECT) //此状态需要选卡--复合卡会进入此状态
				{
					NFCC_RwSelect(0);
					while (1)
					{
						if (FALSE == port_get_irq_flag(0))
						{
							port_timer_delay_ms(20);
							//_sleep   此处需要平台任务休眠函数，等待IRQ中断唤醒，降低功耗
						}

						NFCC_Poll(); //其他NTF处理
						if (Timeout != NO_TIMEOUT_FOREVER)
						{
							if ((i++) >= (Timeout * 50)) //超时退出
							{
								NFC_TRACE_DEBUG("----------------err timeout  1111-------------");
								return NFCC_RESULT_TIMEOUT; //超时
							}
						}

						if ((CardProtocol & NfcCardinfo[0].RF_Protocol) == NfcCardinfo[0].RF_Protocol) //表征读到的卡是等待的卡片
						{
							NFC_TRACE_DEBUG("----------------ok ----3333---------");
							return NFCC_RESULT_SUCCESS; //成功;
						}
					}
				}
				else
				{
					NFC_TRACE_DEBUG("----------------ok ----1111---------");
					return NFCC_RESULT_SUCCESS; //成功
				}
			}
			else if (((CardProtocol & NfcCardinfo[1].RF_Protocol) == NfcCardinfo[1].RF_Protocol)
					 && (NfcCardinfo[1].RF_Protocol != 0x00)) //复合卡，第二张为所等待的卡片
			{
				// NFC_TRACE_DEBUG("------------test 2222-----------------");
				// NFC_TRACE_DEBUG("------------CardProtocol = 0x%02x---------------------", CardProtocol);
				// NFC_TRACE_DEBUG("------------NfcCardinfo[0].RF_Protocol = 0x%02x-------------------", NfcCardinfo[0].RF_Protocol);
				// NFC_TRACE_DEBUG("------------NfcCardinfo[1].RF_Protocol = 0x%02x-----------------", NfcCardinfo[1].RF_Protocol);
				NFCC_RwSelect(1); //选择第二张卡片
				while (1)
				{
					if (FALSE == port_get_irq_flag(0))
					{
						port_timer_delay_ms(20);
						//_sleep   此处需要平台任务休眠函数，等待IRQ中断唤醒，降低功耗
					}

					NFCC_Poll(); //其他NTF处理
					if (Timeout != NO_TIMEOUT_FOREVER)
					{
						if ((i++) >= (Timeout * 50)) //超时退出
						{
							NFC_TRACE_DEBUG("----------------err timeout  no M1 card in-------------");
							return NFCC_RESULT_TIMEOUT; //超时
						}
					}

					if ((CardProtocol & NfcCardinfo[1].RF_Protocol) == NfcCardinfo[1].RF_Protocol) //表征读到的卡所等待的卡片
					{
						NFC_TRACE_DEBUG("----------------ok ----2222---------");
						return NFCC_RESULT_SUCCESS; //成功;
					}
				}
			}
			else //不是所等待的卡片
			{
				NFC_TRACE_DEBUG("----------------not card type  NFCC_RwReDiscovery -------------");
				memset(&NfcCardinfo, 0, sizeof(NfcCardinfo)); //清空 RW 卡片区数据
				NFCC_RwReDiscovery();						  //重新探卡
			}
		}
	}
}

static uint8_t keyA[6]						= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t keyB[6]						= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t g_localM1_Sector_password[8] = {0x0B, 0x54, 0x57, 0x07, 0x45, 0xFE, 0x3A, 0xE7}; //全FFFFFFFFFFFFFFFF 当前Sector的KEYA/KAYB生成的Pass word值
static uint8_t g_section_data[65]			= {
	  0x11, 0x23, 0x45, 0x00, 0xEE, 0xDC, 0xBA, 0xFF, 0x11, 0x23, 0x45, 0x00, 0x09, 0xF6, 0x09, 0xF6,
	  0x11, 0x23, 0x45, 0x00, 0xEE, 0xDC, 0xBA, 0xFF, 0x11, 0x23, 0x45, 0x00, 0x09, 0xF6, 0x09, 0xF6,
	  0x11, 0x23, 0x45, 0x00, 0xEE, 0xDC, 0xBA, 0xFF, 0x11, 0x23, 0x45, 0x00, 0x09, 0xF6, 0x09, 0xF6,
	  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};

t_NFCC_Result NFCC_CopyToLocalM1(uint8_t app_type)
{
	t_NFCC_Result ucResult = NFCC_RESULT_SUCCESS;
	uint8_t flag	 = 0;
	ucResult		 = NFCC_RwWaitCard(NCI_RF_PROTOCOL_M1_RW, 100);

	g_copy_m1_flag = true;

	if (((g_NfcInfo.LocalM1IsExit >> app_type) & 0x01) != 0x01) //若是本张卡不存在，就先创建
	{
		ucResult = NFCC_LocalM1SetUid(app_type, 0x11223344); //UID随便写的，复制的时候会进行覆盖
	}

	if (g_NfcInfo.LocalM1ActiveAppId == app_type) //若是本张卡是激活的，就先去激活
	{
		flag				  = 1;
		g_LocalM1CurrentIndex = g_NfcInfo.LocalM1ActiveAppId;
		// NCI_LOCAL_M1_DEACTIVE
		NFCC_NciCmdProcess(NCI_LOCAL_M1_DEACTIVE);
	}

	for (int i = 0; i < 16; i++)
	{
		if (!ucResult)
		{
			ucResult = NFCC_RwMifareAuth(1, i, keyA);
		}

		// if (!ucResult)
		// {
		// 	ucResult = NFCC_RwMifareAuth(2, i, keyB);
		// }

		for (int j = 0; j < 4; j++)
		{
			if (!ucResult)
			{
				ucResult = NFCC_RwMifareReadBlock(i * 4 + j, &g_section_data[1]);
				g_section_data[0] = 0x10; //LV Lenth
			}

			if (j != 3)
			{
				if (!ucResult)
				{
					ucResult = NFCC_LocalM1WriteSector(app_type, i, (0x01 << j), g_localM1_Sector_password, g_section_data);
				}
			}
		}
	}
	g_copy_m1_flag = false;

	if (flag == 1) //恢复之前的激活状态
	{
		g_LocalM1CurrentIndex = app_type;
		if (ucResult == 0)
		{
			// NCI_LOCAL_M1_ACTIVATE
			NFCC_NciCmdProcess(NCI_LOCAL_M1_ACTIVATE);
		}
	}
	return ucResult;
}

/*
0xA0, 
0x9A, 0xA4, 0x3F, 0xA1, 
0x08, //type
0x00, 0x04, //reqa
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x60, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF//key
};
*/
t_NFCC_Result NFCC_LocalM1SetUid(uint8_t app_type, uint32_t Uid)
{
	uint8_t res_date[]	   = {0xA0, 0x9A, 0xA4, 0x3F, 0xA1, 0x08, 0x04, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x60, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t se[]		   = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x80, 0x69, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	t_NFCC_Config *pConfig = NFCC_GetConfig();

	// if (pConfig->nfc_mode & MODE_NFC_RW)
	// {
	// 	NFCC_OpenCeMode();
	// }

	if (app_type > 3)
	{
		NFC_TRACE_ERROR("para error, app_type:%#x", app_type);
		return NFCC_RESULT_PARA_ERR; //失败
	}
	NFCC_M1InfoPack(app_type, (uint8_t *)&Uid, se, res_date);
	return (NFCC_LocalM1CardInfoChange(app_type, res_date, sizeof(res_date))); //验证通过
																			   // NFCC_ChangeRoute(LOCAL_M1,NULL);
}

extern uint8_t g_fwdl_test_flag;   /**1 test mode 0 其他 */
extern uint8_t g_fwdl_test_status; //返回值 1 正在下载 2 下载失败 0 下载成功
t_NFCC_Result NFCC_ForceFwdl(void)
{
	g_fwdl_test_flag = 1;
	uint16_t res	 = check_try_fwdl();
	g_fwdl_test_flag = 0;
	NFCC_Init();
	if (FW_UD_SUCCESS != res)
		return NFCC_RESULT_FAILED;
	return NFCC_RESULT_SUCCESS;
}

//返回值 1 正在下载 2 下载失败 0 下载成功
uint8_t NFCC_GetFwdlStatus(void)
{
	return g_fwdl_test_status;
}

CARD_PARA *NFCC_RwGetInfo(void)
{
	return &NfcCardinfo[0];
}

bool NFCC_SuperSimIsReady(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_HCI_SIM_ENABLE;
	NFCC_OtherCmdPoll();

	if(g_apiCmd.cmd_status == NFCC_RESULT_SUCCESS)
	{
		return g_NfcInfo.SuperSimGateIsReady;
	}
	else
	{
		return false;
	}	
}
bool NFCC_EseIsReady(void)
{
	g_apiCmd.cmd_end_flag	= 0;
	g_apiCmd.cmd_start_flag = 1;
	g_apiCmd.cmd			= NFCC_CMD_HCI_SE_ENABLE;
	NFCC_OtherCmdPoll();

	if(g_apiCmd.cmd_status == NFCC_RESULT_SUCCESS)
	{
		return g_NfcInfo.EseGateIsReady;
	}
	else
	{
		return false;
	}	
}

/**0x30: apdugate ID from HCI spec
*  0xF0: apdugate ID from GP spec*/
static void check_hci_pipe_id(COMMON_NCI_PACKET * pack)
{
	if ((pack->GID == HCI_DATA_CONN_ID) && (pack->PAYLOAD_LEN == 0x07) && 
		(pack->PAYLOAD[0] == 0x81) && (pack->PAYLOAD[1] == 0x80) &&
		(pack->PAYLOAD[2] == 0x01) && (pack->PAYLOAD[3] == 0x30)) //rsp for create a pipe
	{
		//C0/*01 00 07 81 80 01 30 C0 30 07  */
		//81/*01 00 07 81 80 01 30 81 30 13 */
		if (pack->PAYLOAD[4] == ESE_HOST) //ESE
		{
			g_NfcInfo.ese_gate_pipe = pack->PAYLOAD[6]; //记录 gate_pip
		}
		else if (pack->PAYLOAD[4] == SUPER_SIM_HOST)
		{
			g_NfcInfo.supersim_gate_pipe = pack->PAYLOAD[6]; //记录 gate_pip
		}
		else
		{
			NFC_TRACE_ERROR("func:%s; line:%d,data ERROR, pack->PAYLOAD[4]:%#x", __FUNCTION__, __LINE__, pack->PAYLOAD[4]);
		}
	}
	else if ((pack->GID == HCI_DATA_CONN_ID) && (pack->PAYLOAD_LEN == 0x07) && 
		(pack->PAYLOAD[0] == 0x81) && (pack->PAYLOAD[1] == 0x12) &&
		(pack->PAYLOAD[3] == 0xF0) && (pack->PAYLOAD[4] == 0x01))
	{
		//C0/*01 00 07 81 12 C0 F0 01 F0 05  */
		//81/*01 00 07 81 12 81 F0 01 F0 11  */
		if (pack->PAYLOAD[2] == ESE_HOST) //ESE
		{
			g_NfcInfo.ese_gate_pipe = pack->PAYLOAD[6]; //记录 gate_pip
		}
		else if (pack->PAYLOAD[2] == SUPER_SIM_HOST)
		{
			g_NfcInfo.supersim_gate_pipe = pack->PAYLOAD[6]; //记录 gate_pip
		}
		else
		{
			NFC_TRACE_ERROR("func:%s; line:%d,data ERROR, pack->PAYLOAD[2]:%#x", __FUNCTION__, __LINE__, pack->PAYLOAD[2]);
		}
	}
}

static t_RetStatus NFCC_NciCmdProcess(uint8_t Index)
{
	t_RetStatus ret = NO_ERR;
	uint16_t poll_cnt = 0;

	poll_cnt = NFCC_Poll();
	if (poll_cnt > 0) 
	{
		NFC_TRACE_DEBUG("before NFCC_NciCmdProcess_, NFCC_Poll cnt:%d", poll_cnt);
	}
	
	ret = NFCC_NciCmdProcess_(Index);
	/*poll_cnt = NFCC_Poll();
	if (poll_cnt > 0) 
	{
		NFC_TRACE_DEBUG("after NFCC_NciCmdProcess_, NFCC_Poll cnt:%d", poll_cnt);
	}*/
	return ret;
}


/**
 */

static t_RetStatus NFCC_NciCmdProcess_(uint8_t Index)
{
	NFC_CMD_LIST *pNfcCmdList = NULL;
	uint8_t count			  = 3;
	uint8_t mode			  = 0;
	tTIMER_CB recvdata_timeout; //接收超时时间

	uint8_t ucGID	  = 0;
	uint8_t ucOID	  = 0;
	uint8_t ucMT	  = 0;
	uint8_t ucHciPipe = 0;
	uint32_t user_timeout_ms = g_apiCmd.UserTimeoutMask;
	uint32_t timeout = 0;
	uint8_t cmdMode = 0;
	CARD_PARA * rw_cardpara = NULL;

	if (Index < NCI_CMD_END)
	{
		pNfcCmdList = &nfcCmdList[Index];
	}
	else
	{
		NFC_TRACE_ERROR("func:%s; line:%d,Index ERROR", __FUNCTION__, __LINE__);
		g_apiCmd.cmd_status = NFCC_RESULT_DH_SEMANTIC_ERR;
		return MEM_ERROR;
	}

	cmdMode = pNfcCmdList->CmdMode;
	
	if (user_timeout_ms != 0xFFFFFFFF) 
	{
		timeout = pNfcCmdList->timeout;//使用默认超时时间
	}
	else 
	{
		cmdMode = 0;//当用户设置的超时时间为0xFFFFFFFF时，发送完成需要立即退出。使用cmdMode为0完成。
	}

	while (count--)
	{
		if (pNfcCmdList->packetSendFunc != NULL)
		{
			pNfcCmdList->packetSendFunc();	 //打包数据
			ucGID	  = nci_send_packet.GID;
			ucOID	  = nci_send_packet.OID; //保留指令的操作码，避免递归调用覆盖
			ucMT	  = nci_send_packet.MT;
			ucHciPipe = nci_send_packet.PAYLOAD[0]; //记录apdu_gate 的pipe
		}
		else
		{
			NFC_TRACE_ERROR("func:%s; line:%d,malloc packetSendFunc NULL ERROR", __FUNCTION__, __LINE__);
			g_apiCmd.cmd_status = NFCC_RESULT_DH_GRAMMAR_ERR;
			return MEM_ERROR;
		}

        Nci_unwrap(nci_send_packet, 0);
		// debug_io(5);

		if (cmdMode == 0) //不等待响应
			return CMD_RECE_OK;

		timer_set_timeout(&recvdata_timeout, timeout); //超时时间 NCI_ACK_TIMEOUT

		rw_cardpara = NFCC_RwGetInfo();
		mode = 0;
		// debug_io(2);
		while (1)
		{
			if (NfcCmdI2cRecvData(g_nfcc_buffer, BUFFER_BLOCK_SIZE - 1) != 0)
			{
				Nci_wrap(g_nfcc_buffer, &nci_rev_packet);
				NFC_TRACE_DEBUG("$$$mode = %d;pNfcCmdList->CmdMode = %d", mode, cmdMode);
				if ((nci_rev_packet.MT == MT_CTL_RES_PACKET) && (ucOID == nci_rev_packet.OID))
				{
					// debug_io(6);
					mode++;
					Nci_Rsp_Packet(&nci_rev_packet);
					if (mode >= cmdMode)
					{
						if (pNfcCmdList->CallbackCpltFunc != NULL)
						{
							pNfcCmdList->CallbackCpltFunc(&nci_rev_packet);
						}

						NFC_TRACE_DEBUG("receive ok111");
						// debug_io(7);
						return CMD_RECE_OK;
					}
				}
				else if (nci_rev_packet.MT == MT_DATA_PACKET)
				{
					if (rw_cardpara) 
					{
						//T4T使用ISO-DEP RF Interface
						if (rw_cardpara->RF_Protocol == NCI_RF_PROTOCOL_M1_RW)
						{
							if (nci_rev_packet.GID == RF_DATA_CONN_ID && nci_rev_packet.OID == DATA_RFU &&
								nci_rev_packet.PAYLOAD_LEN == 1 && nci_rev_packet.PAYLOAD[0] == 0xB2) //B2: M1卡B2表示超时
							{
								g_apiCmd.cmd_status = NFCC_RESULT_TIMEOUT;
								return CMD_TIME_OUT;
							}
						}
					}
					
					// debug_io(8);
					if (ucMT != MT_DATA_PACKET)
					{
						NFC_TRACE_DEBUG("receive ok222");
						NFCC_DataHciPacket(&nci_rev_packet);
					}
					else
					{
						NFC_TRACE_DEBUG("receive ok444");

						if ((nci_rev_packet.GID == RF_DATA_CONN_ID) && (ucOID == nci_rev_packet.OID) && (0x00 == nci_rev_packet.OID)) //HCE  RW
						{
							if (nci_rev_packet.PBF == 0x00) //分包的情况，最后一包再计数
							{
								mode++;
							}
						}
						else if (nci_rev_packet.GID == HCI_DATA_CONN_ID) //hci pipe
						{
							if (nci_rev_packet.PBF == 0x00) //分包的情况，最后一包再计数
							{
								/**
								 * TYPE: 01    events
								 * INSTRUCTION: 11   
								 */
								if ((nci_rev_packet.PAYLOAD[1] != 0x51)) /**WTX */
								{
									mode++;
								}
								else
								{
									if (((nci_rev_packet.PAYLOAD[0] & HCI_PIPEID_MASK) == g_NfcInfo.ese_gate_pipe)
										&&(g_NfcInfo.ese_gate_pipe != 0))
									{
										timer_set_timeout(&recvdata_timeout, g_nfc_dm.HCI_ese_apdugate_wtx_time_ms);//收到WTX，重新计时，apdugate寄存器时间
									}
									else if (((nci_rev_packet.PAYLOAD[0] & HCI_PIPEID_MASK) == g_NfcInfo.supersim_gate_pipe)
										&&(g_NfcInfo.supersim_gate_pipe != 0))
									{
										timer_set_timeout(&recvdata_timeout, g_nfc_dm.HCI_esim_apdugate_wtx_time_ms);//收到WTX，重新计时，apdugate寄存器时间
									}
									continue;
								}
							}
						}

						check_hci_pipe_id(&nci_rev_packet);						

						if (nci_rev_packet.PBF == PBF_NCI_SET) //分包的情况，非最后一包的回调
						{
							if (pNfcCmdList->CallbackPartFunc != NULL)
							{
								pNfcCmdList->CallbackPartFunc(&nci_rev_packet);
							}
						}
						if ((mode >= cmdMode) && (nci_rev_packet.PBF == 0x00)) //分包的情况，最后一包再计数
						{
							// debug_io(9);
							if (pNfcCmdList->CallbackCpltFunc != NULL)
							{
								pNfcCmdList->CallbackCpltFunc(&nci_rev_packet);
							}

							NFC_TRACE_DEBUG("receive ok555");
							return CMD_RECE_OK;
						}
					}
				}
				if (nci_rev_packet.MT == MT_CTL_NTF_PACKET)
				{					
					if (rw_cardpara) 
					{
						//T4T使用ISO-DEP RF Interface
						if (rw_cardpara->RF_Protocol == NCI_RF_PROTOCOL_ISO_DEP && rw_cardpara->RF_Interface == NCI_RF_INTERFACE_ISO_DEP)
						{
							if (nci_rev_packet.GID == GID_PROPRIETARY && nci_rev_packet.OID == OID_DROP_SEND_WTX_NTF &&
								nci_rev_packet.PAYLOAD_LEN == 1 && nci_rev_packet.PAYLOAD[0] == 0)
							{
								timer_set_timeout(&recvdata_timeout, g_nfc_dm.RF_wtx_time_ms);//收到WTX，重新计时，tagA038时间
								continue;
							}
						}
					}
					if (ucOID != nci_rev_packet.OID)
					{
						NFC_TRACE_DEBUG("nci_send_packet.OID= 0x%02x,nci_rev_packet.OID = 0x%02x", ucOID, nci_rev_packet.OID);
						NFC_TRACE_DEBUG("func:%s; line:%d, unknow NTF need prcess", __FUNCTION__, __LINE__);
						if (ucMT == MT_DATA_PACKET)
						{
							if (nci_rev_packet.GID == GID_NCI_CORE && nci_rev_packet.OID == OID_CORE_CONN_CREDITS_NTF)
							{
								mode++;
							}
						}
					}
					else
					{
						mode++;
					}
					Nci_Ntf_Packet(&nci_rev_packet);
					if (mode >= cmdMode)
					{
						if (pNfcCmdList->CallbackCpltFunc != NULL)
						{
							pNfcCmdList->CallbackCpltFunc(&nci_rev_packet);
						}

						NFC_TRACE_DEBUG("receive ok333");
						// debug_io(10);

						return CMD_RECE_OK;
					}
				}
			}
			// debug_io(12);
			if (timer_wait_timeout(&recvdata_timeout) == 1)
			{
				//return CMD_TIME_OUT;
				NFC_TRACE_DEBUG("func:%s; line:%d,TIMEOUT ERROR", __FUNCTION__, __LINE__);
				break;
			}
			// debug_io(13);
		}
	}
	g_apiCmd.cmd_status = NFCC_RESULT_TIMEOUT;
	return CMD_TIME_OUT;
}

static void NFCC_NciClean(void)
{
	memset((uint8_t *)&nci_send_packet, 0, sizeof(nci_send_packet));
}

static void NFCC_Activate(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= OID_PROP_ACTIVATE;
	nci_send_packet.PAYLOAD_LEN = 0x04;
	nci_send_packet.PAYLOAD[0]	= 0x5A;
	nci_send_packet.PAYLOAD[1]	= 0xA5;
	nci_send_packet.PAYLOAD[2]	= 0xA5;
	nci_send_packet.PAYLOAD[3]	= 0x5A;
	NFC_TRACE_DEBUG("NFCC_Activate");
}

static void NFCC_CoreReset(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_RESET;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= 0x00; //复位并且rf配置也清零
	NFC_TRACE_DEBUG("NFCC_CoreReset");
}

static void NFCC_CoreResetAfterFwdl(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_RESET;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= 0x01; //--固件下载后调用
	NFC_TRACE_DEBUG("NFCC_CoreReset");
}

static void NFCC_CoreInit(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_INIT;
	nci_send_packet.PAYLOAD_LEN = 0x02;
	nci_send_packet.PAYLOAD[0]	= 0x00;
	nci_send_packet.PAYLOAD[1]	= 0x00;
	NFC_TRACE_DEBUG("NFCC_CoreInit");
}

static void NFCC_CoreGetConfigForRfWtxTime(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_GET_CONFIG;
	nci_send_packet.PAYLOAD_LEN = 0x03;
	nci_send_packet.PAYLOAD[0]	= 0x01;
	nci_send_packet.PAYLOAD[1]	= 0xA0;
	nci_send_packet.PAYLOAD[2]	= 0x38;
	NFC_TRACE_DEBUG("NFCC_CoreGetConfigForRfWtxTime");
}

static void NFCC_CoreConfigForNfceeAction(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN = 0x04;
	nci_send_packet.PAYLOAD[0]	= 0x01;
	nci_send_packet.PAYLOAD[1]	= 0x81;
	nci_send_packet.PAYLOAD[2]	= 0x01;
	nci_send_packet.PAYLOAD[3]	= 0x00; /**for test  发送 RF_NFCEE_ACTION_NTF*/
	NFC_TRACE_DEBUG("NFCC_CoreConfigForNfceeAction");
}

static void NFCC_CoreConfigForSeSsionEse(void)
{
	/** 20 02 0C 01 A0 46 08 FF FF FF FF FF FF FF FF*/
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_NCI_CORE;
	nci_send_packet.OID			 = OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN	 = 0x0c;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0xA0;
	nci_send_packet.PAYLOAD[i++] = 0x46;
	nci_send_packet.PAYLOAD[i++] = 0x08;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	NFC_TRACE_DEBUG("NFCC_CoreConfigForSeSsionEse");
}

static void NFCC_CoreConfigForSeSsionSim(void)
{
	/** 20 02 0C 01 A0 47 08 FF FF FF FF FF FF FF FF*/
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_NCI_CORE;
	nci_send_packet.OID			 = OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN	 = 0x0c;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0xA0;
	nci_send_packet.PAYLOAD[i++] = 0x47;
	nci_send_packet.PAYLOAD[i++] = 0x08;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	nci_send_packet.PAYLOAD[i++] = 0xFF;
	NFC_TRACE_DEBUG("NFCC_CoreConfigForSeSsionEse");
}

static void NFCC_CoreConfigForNfc(void)
{
	/**20 02 21 0A 30 01 04 31 01 00 58 01 C4 32 01 20 38 01 00 50 01 00 85 01 01 00 02 F4 01 02 01 01 A0 07 01 01 */
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_NCI_CORE;
	nci_send_packet.OID			 = OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN	 = 0x21;
	nci_send_packet.PAYLOAD[i++] = 0x0A;

	nci_send_packet.PAYLOAD[i++] = 0x30;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x04;

	nci_send_packet.PAYLOAD[i++] = 0x31;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x00;

	nci_send_packet.PAYLOAD[i++] = 0x58;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0xc4;

	nci_send_packet.PAYLOAD[i++] = 0x32;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x20;

	nci_send_packet.PAYLOAD[i++] = 0x38;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x00;

	nci_send_packet.PAYLOAD[i++] = 0x50;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x00;

	nci_send_packet.PAYLOAD[i++] = 0x85;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x01;

	nci_send_packet.PAYLOAD[i++] = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x02;
	nci_send_packet.PAYLOAD[i++] = 0xf4;
	nci_send_packet.PAYLOAD[i++] = 0x01;

	nci_send_packet.PAYLOAD[i++] = 0x02;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x01;

	nci_send_packet.PAYLOAD[i++] = 0xA0;
	nci_send_packet.PAYLOAD[i++] = 0x07;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x01;

	NFC_TRACE_DEBUG("NFCC_CoreConfigForNfc");
}

static void NFCC_CoreConfigForField(void)
{
	/**20 02 04 01 80 01 00 */
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_NCI_CORE;
	nci_send_packet.OID			 = OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN	 = 0x04;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x80;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x01;

	NFC_TRACE_DEBUG("NFCC_CoreConfigForField");
}

static void NFCC_HedUpdateFlash(void)
{
	/**2F 02 00 */
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= OID_PROPRIETARY_UPDATA_FLASH;
	nci_send_packet.PAYLOAD_LEN = 0x00;

	NFC_TRACE_DEBUG("NFCC_HedUpdateFlash");
}

static void NFCC_CoreConfigForPowSub(void)
{
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_NCI_CORE;
	nci_send_packet.OID			 = OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN	 = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x00;

	NFC_TRACE_DEBUG("NFCC_CoreConfigForPowSub");
}
static void NFCC_CoreConfigForGdTimeAndPhase(void)
{
	/**0x20,0x02,0x05,0x01,0xA0,0x0F,0x01,0x02 */
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_NCI_CORE;
	nci_send_packet.OID			 = OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0xA0;
	nci_send_packet.PAYLOAD[i++] = 0x0F;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x03; //打开GD PHASE
	nci_send_packet.PAYLOAD_LEN	 = i;

	NFC_TRACE_DEBUG("NFCC_CoreConfigForPowSub");
}
static void NFCC_CoreConfigForPhase(void)
{
	/**0x20,0x02,0x05,0x01,0xA0,0x0F,0x01,0x02 */
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_NCI_CORE;
	nci_send_packet.OID			 = OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0xA0;
	nci_send_packet.PAYLOAD[i++] = 0x0F;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	nci_send_packet.PAYLOAD[i++] = 0x01; //仅仅打开PHASE
	nci_send_packet.PAYLOAD_LEN	 = i;

	NFC_TRACE_DEBUG("NFCC_CoreConfigForPowSub");
}

#if NFCC_USE_SDK_RF_PARA

const uint8_t cfg_RF_param[] = USER_RF_CONFIG_PARA;
static void NFCC_CoreConfigRfPara(void)
{
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN = cfg_RF_param[2];
	memcpy(&nci_send_packet.PAYLOAD[0], &cfg_RF_param[3], nci_send_packet.PAYLOAD_LEN);
}

#endif

#if NFCC_SOC2_SUPPORT
static uint8_t HED_NFC_CE_DLMA_CFG[] = {
	0x20, 0x02, 0x1F, 0x03,
	0xA1, 0x08, 0x07, 0x05, 0x90, 0x04, 0x0D, 0x0D, 0x0D, 0x0D,
	0xA1, 0x08, 0x07, 0x05, 0xA8, 0x04, 0x0D, 0x0D, 0xFF, 0xFF,
	0xA1, 0x08, 0x07, 0x00, 0xC8, 0x04, 0x05, 0x05, 0x00, 0x00};

static void NFCC_CoreConfigCeDlma(void)
{
	uint8_t i = 0;

	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN = HED_NFC_CE_DLMA_CFG[2];
	memcpy(&nci_send_packet.PAYLOAD[0], &HED_NFC_CE_DLMA_CFG[3], nci_send_packet.PAYLOAD_LEN);
}
#endif

static uint8_t HED_NFC_SWP_CFG[] = {0x20, 0x02, 0x05, 0x01,
									0xA0, 0x4D, 0x01, 0x00};
static void NFCC_CoreConfigForSurperSIM(void)
{
	uint8_t i = 0;

	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN = HED_NFC_SWP_CFG[2];
	memcpy(&nci_send_packet.PAYLOAD[0], &HED_NFC_SWP_CFG[3], nci_send_packet.PAYLOAD_LEN);
}

#if NFCC_FW_CONFIG_NFCC_PARA

#if 0
//static uint8_t HED_NFC_ENABLE_RF_PARA_CFG[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x11, 0x01, 0x01};  //SOC2
static uint8_t HED_NFC_ENABLE_RF_PARA_CFG[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x0B, 0x01, 0x01};  //SOC1
#endif

#if NFCC_SOC2_SUPPORT
static uint8_t HED_NFC_ENABLE_RF_PARA_CFG[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x11, 0x01, 0x01};
#else
//SOC1
static uint8_t HED_NFC_ENABLE_RF_PARA_CFG[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x0B, 0x01, 0x01};
#endif

static void NFCC_CoreConfigForEnableRfSet_para(void)
{
	uint8_t i = 0;

	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN = HED_NFC_ENABLE_RF_PARA_CFG[2];
	memcpy(&nci_send_packet.PAYLOAD[0], &HED_NFC_ENABLE_RF_PARA_CFG[3], nci_send_packet.PAYLOAD_LEN);
}

#else
#if NFCC_SOC2_SUPPORT
static uint8_t HED_NFC_DISABLE_RF_PARA_CFG[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x11, 0x01, 0x00};
#else
//SOC1
static uint8_t HED_NFC_DISABLE_RF_PARA_CFG[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x0B, 0x01, 0x00};
#endif
static void NFCC_CoreConfigForDisableRfSet_para(void)
{
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN = HED_NFC_DISABLE_RF_PARA_CFG[2];
	memcpy(&nci_send_packet.PAYLOAD[0], &HED_NFC_DISABLE_RF_PARA_CFG[3], nci_send_packet.PAYLOAD_LEN);
}
#endif

static uint8_t HED_NFC_DISABLE_RAW_DATA[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x0B, 0x01, 0x00};
static void NFCC_CoreConfigForDisableRawData(void)
{
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NCI_CORE;
	nci_send_packet.OID			= OID_CORE_SET_CONFIG;
	nci_send_packet.PAYLOAD_LEN = HED_NFC_DISABLE_RAW_DATA[2];
	memcpy(&nci_send_packet.PAYLOAD[0], &HED_NFC_DISABLE_RAW_DATA[3], nci_send_packet.PAYLOAD_LEN);
}

static void NFCC_CoreGetConfig(void)
{
	/**2F 02 00 */
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_NCI_CORE;
	nci_send_packet.OID			 = OID_CORE_GET_CONFIG; //get  config cmd
	nci_send_packet.PAYLOAD_LEN	 = 1 + g_get_config_cmd_len;
	nci_send_packet.PAYLOAD[i++] = 0x01;
	memcpy(&nci_send_packet.PAYLOAD[i], g_get_config_cmd, g_get_config_cmd_len);
	NFC_TRACE_DEBUG("NFCC_CoreGetConfig");
}

/**
 *  <--  40 03 4D 00 01 A0 4B 48 02 02 04 A0 58 EF 04 00 00 00 00 00 00 20 00 04 0C 4D 54 70 02 00 00 00 00 A0 58 EF 03 00 00 00 70 01 00 00 00 01 00 02 01 00 00 00 00 00 00 00 00 E4 02 53 5A 00 00 00 00 00 00 00 00 00 00 00
 */
static void NFCC_CoreGetConfigCallBack(COMMON_NCI_PACKET *pData)
{
	if ((g_apiCmd.cmd == NFCC_CMD_GET_SURPERSIM_UID) | (g_apiCmd.cmd == NFCC_CMD_GET_ESE_UID)) //超级SIM UID
	{
		if (pData->PAYLOAD_LEN == 0x4D)
		{
			if ((pData->PAYLOAD[2] == 0xA0) && (pData->PAYLOAD[3] == 0x4B))
			{
				g_NfcInfo.super_sim_uid_size = pData->PAYLOAD[7];
				memcpy(g_NfcInfo.super_sim_uid, &pData->PAYLOAD[8], g_NfcInfo.super_sim_uid_size);
			}
			if ((pData->PAYLOAD[2] == 0xA0) && (pData->PAYLOAD[3] == 0x4A))
			{
				g_NfcInfo.ese_uid_size = pData->PAYLOAD[7];
				memcpy(g_NfcInfo.ese_uid, &pData->PAYLOAD[8], g_NfcInfo.ese_uid_size);
			}
		}
	}
}

static void NFCC_RfSetRoute(void)
{
	uint8_t i = 0;

	if (g_apiCmd.cmd == NFCC_CMD_CHANGE_ROUT)
	{
		nci_send_packet.MT	= MT_CTL_CMD_PACKET;	
		nci_send_packet.GID = GID_RF_MANAGE;
		nci_send_packet.OID = OID_RF_SET_ROUTING;
		//memcpy(nci_send_packet.PAYLOAD, g_apiCmd.p_in_data, g_apiCmd.in_data_len);
		//nci_send_packet.PAYLOAD_LEN = g_apiCmd.in_data_len;
		//考虑到分包，在外部组帧
	}
	else
	{
		NFCC_NciClean();
		nci_send_packet.MT	= MT_CTL_CMD_PACKET;	
		nci_send_packet.GID = GID_RF_MANAGE;
		nci_send_packet.OID = OID_RF_SET_ROUTING;
		nci_send_packet.PBF = 0;
		if (g_nfc_config.ce_route != DH_HOST) /**21 01 07 00 01 00 03 81 3B 00 */ //  10 C0 81
		{
			nci_send_packet.PAYLOAD[i++] = 0x00;
			nci_send_packet.PAYLOAD[i++] = 0x01; //Number of Routing Entries
			nci_send_packet.PAYLOAD[i++] = 0x00; //Qualifier-Type
			nci_send_packet.PAYLOAD[i++] = 0x03; //Length
			nci_send_packet.PAYLOAD[i++] = g_nfc_config.ce_route;
			nci_send_packet.PAYLOAD[i++] = 0x3b;
			nci_send_packet.PAYLOAD[i++] = 0x00;
		}
		else /**21 01 13 00 02 42 0A 00 01 A0 00 11 22 33 44 55 66 00 03 81 3B 00  */
		{
			nci_send_packet.PAYLOAD[i++] = 0x00;
			nci_send_packet.PAYLOAD[i++] = 0x02; //Number of Routing Entries
			nci_send_packet.PAYLOAD[i++] = 0x42; //Qualifier-Type
			nci_send_packet.PAYLOAD[i++] = 0x0A; //Length
			nci_send_packet.PAYLOAD[i++] = 0x00; //Route : 00    DH ID  g_nfc_config.ce_route
			nci_send_packet.PAYLOAD[i++] = 0x01; //Applies to Switched On State(NFC终端开机，亮屏，解锁);

			memcpy(&nci_send_packet.PAYLOAD[i], g_apiCmd.p_in_data, 0x08); //AID
			i += 0x08;

			nci_send_packet.PAYLOAD[i++] = 0x00; //Qualifier-Type
			nci_send_packet.PAYLOAD[i++] = 0x03; //Length
			nci_send_packet.PAYLOAD[i++] = 0x81;
			nci_send_packet.PAYLOAD[i++] = 0x3b;
			nci_send_packet.PAYLOAD[i++] = 0x00;
		}
		nci_send_packet.PAYLOAD_LEN = i;
	}

	NFC_TRACE_DEBUG("NFCC_RfSetRoute");
}

static void NFCC_RfDeactiveSleep(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_RF_MANAGE;
	nci_send_packet.OID			= OID_RF_DEACTIVATE;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= SLEEP_MODE; //Sleep Mode
	NFC_TRACE_DEBUG("NFCC_RfDeactiveSleep");
}
static void NFCC_RfDeactiveSleepAf(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_RF_MANAGE;
	nci_send_packet.OID			= OID_RF_DEACTIVATE;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= SLEEP_AF_MODE; //Sleep Af Mode
	NFC_TRACE_DEBUG("NFCC_RfDeactiveSleepAf");
}

static void NFCC_RfDeactiveDiscovery(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_RF_MANAGE;
	nci_send_packet.OID			= OID_RF_DEACTIVATE;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= DISCOVERY_MODE; //Sleep Mode
	NFC_TRACE_DEBUG("NFCC_RfDeactiveDiscovery");
}

#if 1
static void NFCC_RwProcessCallback(COMMON_NCI_PACKET *pData)
{
	if (g_apiCmd.p_out_data)
	{
		if (g_apiCmd.out_data_max_size >= (g_apiCmd.out_data_len + pData->PAYLOAD_LEN))
		{
			memcpy((uint8_t *)g_apiCmd.p_out_data, pData->PAYLOAD, pData->PAYLOAD_LEN);
			g_apiCmd.out_data_len += (pData->PAYLOAD_LEN);
			g_apiCmd.p_out_data += (pData->PAYLOAD_LEN);
		}
		else
		{
			memcpy((uint8_t *)g_apiCmd.p_out_data, pData->PAYLOAD, g_apiCmd.out_data_max_size - g_apiCmd.out_data_len);
			g_apiCmd.out_data_len = g_apiCmd.out_data_max_size;

		}
	}
}
static void NFCC_RwProcess(void)
{
	//NFCC_NciClean();//外部赋值了，不能在此处清除了
	nci_send_packet.MT			= MT_DATA_PACKET;
	nci_send_packet.GID			= RF_DATA_CONN_ID;
	nci_send_packet.OID			= DATA_RFU;
	NFC_TRACE_DEBUG("NFCC_RwProcess");
}
#endif

#if NFCC_RW_SUPPORT
static void NFCC_RwSelectCard(void)
{
	//210403028080
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_RF_MANAGE;
	nci_send_packet.OID			= OID_RF_DISCOVER_SELECT;
	nci_send_packet.PAYLOAD_LEN = 0x03;
	nci_send_packet.PAYLOAD[0]	= g_apiCmd.p_in_data[0];
	nci_send_packet.PAYLOAD[1]	= g_apiCmd.p_in_data[1];
	nci_send_packet.PAYLOAD[2]	= g_apiCmd.p_in_data[2];

	NFC_TRACE_DEBUG(" CardPara->RF_Discovery_ID = %02X", g_apiCmd.p_in_data[0]);
	NFC_TRACE_DEBUG(" CardPara->RF_Protocol = %02X", g_apiCmd.p_in_data[1]);
	NFC_TRACE_DEBUG(" CardPara->RF_Interface = %02X", g_apiCmd.p_in_data[2]);
	NFC_TRACE_DEBUG("NFCC_RwSelectCard");
}

static void NFCC_RwAuthM1Sector(void)
{
	//0000 09 600300FFFFFFFFFFFF
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = 0x00;
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x60;
	nci_send_packet.PAYLOAD[i++] = 0x03; //todo ... block num 03 07 0b
	nci_send_packet.PAYLOAD[i++] = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0xFF; //todo ... key
	nci_send_packet.PAYLOAD[i++] = 0xFF; //todo ... key
	nci_send_packet.PAYLOAD[i++] = 0xFF; //todo ... key
	nci_send_packet.PAYLOAD[i++] = 0xFF; //todo ... key
	nci_send_packet.PAYLOAD[i++] = 0xFF; //todo ... key
	nci_send_packet.PAYLOAD[i++] = 0xFF; //todo ... key
	nci_send_packet.PAYLOAD_LEN	 = i;

	NFC_TRACE_DEBUG("NFCC_RwAuthM1Sector");
}

static void NFCC_RwReadM1Sector(void)
{
	//0000 02 3004
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = 0x00;
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x30;
	nci_send_packet.PAYLOAD[i++] = 0x00; //todo ... block num
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_RwReadM1Sector");
}
static void NFCC_RwWriteM1Sector(void)
{
	//
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = 0x00;
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x30;
	nci_send_packet.PAYLOAD[i++] = 0x00; //todo ... block num
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_RwWriteM1Sector");
}

static void NFCC_RwReaderSendData(SL_APUD_TRAN *const dataout, SL_APUD_TRAN *const datain)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_DATA_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= RF_DATA_CONN_ID;
	nci_send_packet.OID			= DATA_RFU;
	nci_send_packet.PAYLOAD_LEN = dataout->LEN;
	memcpy(nci_send_packet.PAYLOAD, dataout->DATA, dataout->LEN);
	NFC_TRACE_DEBUG("NFCC_RwReaderSendData");
}
#endif

static void NFCC_LpcdCheckCard(void)
{
	//
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= OID_PROPRIETARY_LPCD_CHECK;
	nci_send_packet.PAYLOAD_LEN = 0;
	NFC_TRACE_DEBUG("NFCC_LpcdCheckCard");
}

static void NFCC_RfDiscoverMap(void)
{
	/**21 00 10 05 04 03 02 80 01 80 05 03 03 03 02 01 81 02 81 */
	uint8_t i = 0;
	NFCC_NciClean();

	nci_send_packet.MT			 = MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = GID_RF_MANAGE;
	nci_send_packet.OID			 = OID_RF_DISCOVER_MAP;
	nci_send_packet.PAYLOAD_LEN	 = 0x10;
	nci_send_packet.PAYLOAD[i++] = 0x05; //配置条数

	nci_send_packet.PAYLOAD[i++] = 0x04; //PROTOCOL_ISO_DEP
	nci_send_packet.PAYLOAD[i++] = 0x03; //the RF Interface is mapped to the RF Protocol in Poll & Listen Mode.
	nci_send_packet.PAYLOAD[i++] = 0x02; //ISO-DEP RF Interface

	nci_send_packet.PAYLOAD[i++] = 0x80; //Mifare
	nci_send_packet.PAYLOAD[i++] = 0x01; //the RF Interface is mapped to the RF Protocol in Poll Mode
	nci_send_packet.PAYLOAD[i++] = 0x80; //Mifare

	nci_send_packet.PAYLOAD[i++] = 0x05; //PROTOCOL_NFC_DEP
	nci_send_packet.PAYLOAD[i++] = 0x03; //the RF Interface is mapped to the RF Protocol in Poll & Listen Mode
	nci_send_packet.PAYLOAD[i++] = 0x03; // NFC-DEP RF Interface

	nci_send_packet.PAYLOAD[i++] = 0x03; //PROTOCOL_T3T
	nci_send_packet.PAYLOAD[i++] = 0x02; // the RF Interface is mapped to the RF Protocol in Listen Mode
	nci_send_packet.PAYLOAD[i++] = 0x01; //Frame RF Interface

	nci_send_packet.PAYLOAD[i++] = 0x81; // RFU/For proprietary use
	nci_send_packet.PAYLOAD[i++] = 0x02; //the RF Interface is mapped to the RF Protocol in Listen Mode
	nci_send_packet.PAYLOAD[i++] = 0x81; //RFU/For proprietary use
	NFC_TRACE_DEBUG("NFCC_Nci_Rf_Discover_map_Cmd");
}

static void NFCC_RfDiscover(void)
{
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT	= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF = 0;
	nci_send_packet.GID = GID_RF_MANAGE;
	nci_send_packet.OID = OID_RF_DISCOVER;

	if (g_nfc_config.nfc_mode == (MODE_NFC_RW | MODE_NFC_CE)) //add for auto mode
	{
		nci_send_packet.PAYLOAD[i++] = 0x04; //配置条数
		nci_send_packet.PAYLOAD[i++] = NFC_A_PASSIVE_LISTEN_MODE;
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD[i++] = NFC_A_PASSIVE_POLL_MODE; //0x00
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD[i++] = NFC_B_PASSIVE_POLL_MODE; //0x00
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD[i++] = NFC_F_PASSIVE_POLL_MODE; //0x00
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD_LEN	 = i;
	}
	else if (g_nfc_config.nfc_mode == MODE_NFC_CE)
	{
		/**21 03 03 01 80 01 */
		nci_send_packet.PAYLOAD[i++] = 0x01; //配置条数
		nci_send_packet.PAYLOAD[i++] = NFC_A_PASSIVE_LISTEN_MODE;
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD_LEN	 = i;
	}
	else //add type A/B/F
	{
		//21 03 09 01 00 01
		nci_send_packet.PAYLOAD[i++] = 0x04;					//A B F V
		nci_send_packet.PAYLOAD[i++] = NFC_A_PASSIVE_POLL_MODE; //0x00
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD[i++] = NFC_B_PASSIVE_POLL_MODE; //0x00
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD[i++] = NFC_F_PASSIVE_POLL_MODE; //0x00
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD[i++] = NFC_V_PASSIVE_POLL_MODE; //0x00
		nci_send_packet.PAYLOAD[i++] = 0x01;
		nci_send_packet.PAYLOAD_LEN	 = i;
	}

	NFC_TRACE_DEBUG("NFCC_RfDiscover");
}

static void NFCC_CoreGetConfigForRfWtxTimeCallBack(COMMON_NCI_PACKET *pData)
{
	//40 03 06 00 01 A0 38 01 xx //xx的单位是100ms， 5表示500ms
	g_nfc_dm.RF_wtx_time_ms = 500 + 50;//默认使用500ms, 由于RTOS计时误差，加大50ms
	if (pData->PAYLOAD_LEN != 6)
	{
		NFC_TRACE_ERROR("NFCC_CoreGetConfigForRfWtxTimeCallBack, PAYLOAD_LEN[%d] error", pData->PAYLOAD_LEN);
		return ; //使用默认值500ms
	}
	if (pData->PAYLOAD[0] != 0)
	{
		NFC_TRACE_ERROR("NFCC_CoreGetConfigForRfWtxTimeCallBack, rsp status[%d] error", pData->PAYLOAD[0]);
		return ; //使用默认值500ms
	}
	if (pData->PAYLOAD[1] != 1)
	{
		NFC_TRACE_ERROR("NFCC_CoreGetConfigForRfWtxTimeCallBack, rsp num[%d] error", pData->PAYLOAD[1]);
		return ; //使用默认值500ms
	}
	if (pData->PAYLOAD[2] != 0xA0 && pData->PAYLOAD[3] != 0x38)
	{
		NFC_TRACE_ERROR("NFCC_CoreGetConfigForRfWtxTimeCallBack, tag[%#x %#x] error", pData->PAYLOAD[2], pData->PAYLOAD[3]);
		return ; //使用默认值500ms
	}
	if (pData->PAYLOAD[4] != 1)
	{
		NFC_TRACE_ERROR("NFCC_CoreGetConfigForRfWtxTimeCallBack, tag len[%d] error", pData->PAYLOAD[4]);
		return ; //使用默认值500ms
	}

	if (pData->PAYLOAD[5] != 0)
	{
		g_nfc_dm.RF_wtx_time_ms = pData->PAYLOAD[5] * 100 + 50; //单位是100ms，由于RTOS计时误差，加大50ms
	}
	else 
	{
		NFC_TRACE_ERROR("NFCC_CoreGetConfigForRfWtxTimeCallBack, tag[a038] value[%d] error, use 500ms", pData->PAYLOAD[5]);
	}
}

static void NFCC_RfDiscoverCallBack(COMMON_NCI_PACKET *pData)
{
	NFCC_SetRFGlobalStatus(RFST_DISCOVERY);

	if (g_nfc_config.nfc_mode == MODE_NFC_CE)
	{
		NFCC_SetGlobalStatus(NFCC_STATUS_DISCOVERY_CE);
	}
	else
	{
		NFCC_SetGlobalStatus(NFCC_STATUS_DISCOVERY_RW);
	}
}

static void NFCC_RfDeactiveIdle(void)
{
	//	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_RF_MANAGE;
	nci_send_packet.OID			= OID_RF_DEACTIVATE;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= IDEL_MODE; //Idel Mode
	NFC_TRACE_DEBUG("NFCC_RfDeactiveIdle");
}

#if NFCC_HCI_SUPPORT
static void NFCC_HciCreateEsePipe(void)
{
	/** 010005811030C030*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x81;
	nci_send_packet.PAYLOAD[i++] = HCI_CREATE_PIPE; //0x10
	nci_send_packet.PAYLOAD[i++] = HCI_GATE_ID;
	nci_send_packet.PAYLOAD[i++] = ESE_HOST;
	nci_send_packet.PAYLOAD[i++] = HCI_GATE_ID;
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciCreateEsePipe");
}
static void NFCC_HciCreateSuperSimPipe(void)
{
	/** 010005811030C030*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x81;
	nci_send_packet.PAYLOAD[i++] = HCI_CREATE_PIPE; //0x10
	nci_send_packet.PAYLOAD[i++] = HCI_GATE_ID;
	nci_send_packet.PAYLOAD[i++] = SUPER_SIM_HOST;
	nci_send_packet.PAYLOAD[i++] = HCI_GATE_ID;
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciCreateSuperSimPipe");
}

static void NFCC_HciOpenEsePipe(void)
{
	/** 0100028903*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.ese_gate_pipe); //pipe id
	nci_send_packet.PAYLOAD[i++] = HCI_OPEN_PIPE;					 //0x03
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciOpenEsePipe");
}

static void NFCC_HciGetEseApdugateWtx(void)
{
	/** 010003890202*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.ese_gate_pipe); //pipe id
	nci_send_packet.PAYLOAD[i++] = HCI_GET_PARA;					 //0x03
	nci_send_packet.PAYLOAD[i++] = HCI_REGISTRY_MAX_WAIT_TIME;
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_NFCC_HciGetApdugateWtx");
}


static void NFCC_HciGetEsimApdugateWtx(void)
{
	/** 010003890202*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF 		 = 0;
	nci_send_packet.GID 		 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID 		 = 0x00;
	nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.supersim_gate_pipe); //pipe id
	nci_send_packet.PAYLOAD[i++] = HCI_GET_PARA;					 //0x03
	nci_send_packet.PAYLOAD[i++] = HCI_REGISTRY_MAX_WAIT_TIME;
	nci_send_packet.PAYLOAD_LEN  = i;
	NFC_TRACE_DEBUG("NFCC_HciGetEsimApdugateWtx");
}



static void NFCC_HciOpenSuperSimPipe(void)
{
	/** 0100028903*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.supersim_gate_pipe); //pipe id
	nci_send_packet.PAYLOAD[i++] = HCI_OPEN_PIPE;						   //0x03
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciOpenSuperSimPipe");
}

static void NFCC_HciCloseEsePipe(void)
{
	/** 0100028903*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.ese_gate_pipe); //pipe id
	nci_send_packet.PAYLOAD[i++] = HCI_CLOSE_PIPE;					 //0x03
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciCloseEsePipe");
}

static void NFCC_HciCloseSuperSimPipe(void)
{
	/** 0100028903*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.supersim_gate_pipe); //pipe id
	nci_send_packet.PAYLOAD[i++] = HCI_CLOSE_PIPE;						   //0x03
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciCloseSuperSimPipe");
}

static void NFCC_HciTransApdu(void) //todo ... 需要和应用结合
{
	/** */
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	// NFCC_NciClean();//外面有赋值，不清除
	nci_send_packet.MT	= MT_DATA_PACKET;
	nci_send_packet.GID = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID = 0x00;
	nci_send_packet.PBF = 0x00; //发送apdugate数据分包时，利用HCI分包，而不是NCI，所以为0

	//由于涉及分包，将赋值放在了otherCmdPoll中

	NFC_TRACE_DEBUG("NFCC_HciTransApdu");
}

static uint8_t hci_chainig_first_rsp_flag = 0;
static void NFCC_HciTransChainingApduCallBack(COMMON_NCI_PACKET *pData)
{
	if (hci_chainig_first_rsp_flag == 0)
	{
		hci_chainig_first_rsp_flag = 1;
		if (g_apiCmd.out_data_max_size >= (g_apiCmd.out_data_len + (pData->PAYLOAD_LEN -2)))
		{
			memcpy((uint8_t *)g_apiCmd.p_out_data, &(pData->PAYLOAD[2]), pData->PAYLOAD_LEN - 2);//2:data offset in hci frame
			g_apiCmd.out_data_len += (pData->PAYLOAD_LEN - 2);
			g_apiCmd.p_out_data += (pData->PAYLOAD_LEN - 2);
		}
		else
		{
			memcpy((uint8_t *)g_apiCmd.p_out_data, &(pData->PAYLOAD[2]), g_apiCmd.out_data_max_size - g_apiCmd.out_data_len);
			g_apiCmd.out_data_len = g_apiCmd.out_data_max_size;
			g_apiCmd.cmd_status = NFCC_RESULT_PARA_ERR;
		}
	}
	else 
	{
		if (g_apiCmd.out_data_max_size >= (g_apiCmd.out_data_len + pData->PAYLOAD_LEN))
		{
			memcpy((uint8_t *)g_apiCmd.p_out_data, pData->PAYLOAD, pData->PAYLOAD_LEN);//
			g_apiCmd.out_data_len += pData->PAYLOAD_LEN;
			g_apiCmd.p_out_data += pData->PAYLOAD_LEN;
		}
		else
		{
			memcpy((uint8_t *)g_apiCmd.p_out_data, pData->PAYLOAD, g_apiCmd.out_data_max_size - g_apiCmd.out_data_len);
			g_apiCmd.out_data_len = g_apiCmd.out_data_max_size;
			g_apiCmd.cmd_status = NFCC_RESULT_PARA_ERR;
		}
	}
	
}

static void NFCC_HciTransApduCallBack(COMMON_NCI_PACKET *pData)
{
	if (hci_chainig_first_rsp_flag == 1) //有级联发生，最后一包不带HCI message header
	{
		if (g_apiCmd.p_out_data)
		{
			if (g_apiCmd.out_data_max_size >= (g_apiCmd.out_data_len + pData->PAYLOAD_LEN))
			{
				memcpy((uint8_t *)g_apiCmd.p_out_data, pData->PAYLOAD, pData->PAYLOAD_LEN);
				g_apiCmd.out_data_len += pData->PAYLOAD_LEN;
				g_apiCmd.p_out_data += pData->PAYLOAD_LEN;
			}
			else
			{
				memcpy((uint8_t *)g_apiCmd.p_out_data, pData->PAYLOAD, g_apiCmd.out_data_max_size - g_apiCmd.out_data_len);
				g_apiCmd.out_data_len = g_apiCmd.out_data_max_size;
				g_apiCmd.cmd_status = NFCC_RESULT_PARA_ERR;
			}
		}
	}
	else //没有级联发生，带HCI message header
	{
		if (pData->PAYLOAD_LEN < 2)
		{
			NFC_TRACE_ERROR("NFCC_HciTransApduCallBack, pData->PAYLOAD_LEN:%d", pData->PAYLOAD_LEN);
			g_apiCmd.cmd_status = NFCC_RESULT_PARA_ERR;
			g_apiCmd.cmd_status = NFCC_RESULT_FAILED;
			return;
		}
		if (g_apiCmd.out_data_max_size >= (g_apiCmd.out_data_len + (pData->PAYLOAD_LEN -2)))
		{
			memcpy((uint8_t *)g_apiCmd.p_out_data, &(pData->PAYLOAD[2]), pData->PAYLOAD_LEN - 2);//2:data offset in hci frame
			g_apiCmd.out_data_len += (pData->PAYLOAD_LEN - 2);
			g_apiCmd.p_out_data += (pData->PAYLOAD_LEN - 2);
		}
		else
		{
			memcpy((uint8_t *)g_apiCmd.p_out_data, &(pData->PAYLOAD[2]), g_apiCmd.out_data_max_size - g_apiCmd.out_data_len);
			g_apiCmd.out_data_len = g_apiCmd.out_data_max_size;
			g_apiCmd.cmd_status = NFCC_RESULT_PARA_ERR;
		}
	}
	
	hci_chainig_first_rsp_flag = 0;
}

static void NFCC_HciClearPipe(void)
{
	/** 01000481140201*/ //clear all pipe
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x81;			   //pipe id  todo ...
	nci_send_packet.PAYLOAD[i++] = HCI_CLEAR_ALL_PIPE; //0x14
	nci_send_packet.PAYLOAD[i++] = 0x02;			   //??
	nci_send_packet.PAYLOAD[i++] = 0x01;			   //??
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciClearPipe");
}

static void NFCC_HciSetPara(void)
{
	/** 01000B81 0101769D6766DBF86566*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x81;		 //
	nci_send_packet.PAYLOAD[i++] = HCI_SET_PARA; //0x01
	nci_send_packet.PAYLOAD[i++] = 0x01;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x76;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x9d;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x67;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x66;		 //??
	nci_send_packet.PAYLOAD[i++] = 0xdb;		 //??
	nci_send_packet.PAYLOAD[i++] = 0xf8;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x65;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x66;		 //??
	nci_send_packet.PAYLOAD_LEN	 = i;

	NFC_TRACE_DEBUG("NFCC_HciSetPara");
}

static void NFCC_HciSetWhite(void)
{
	/** 010006 81 01 03028182*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x81;		 //
	nci_send_packet.PAYLOAD[i++] = HCI_SET_PARA; //0x01
	nci_send_packet.PAYLOAD[i++] = 0x03;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x02;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x81;		 //??
	nci_send_packet.PAYLOAD[i++] = 0x82;		 //??
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciSetPara");
}

static void NFCC_HciResponseOk(void)
{
	/** 010006 81 01 03028182*/
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT			 = MT_DATA_PACKET;
	nci_send_packet.PBF			 = 0;
	nci_send_packet.GID			 = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID			 = 0x00;
	nci_send_packet.PAYLOAD[i++] = 0x81;			//
	nci_send_packet.PAYLOAD[i++] = HCI_RESPONSE_OK; //0x01
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciResponseOk");
}
static void NFCC_HciWakeup(void)
{
	/** -->  01 00 02 89 50  
<--  01 00 04 89 50 68 81 */
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT	= MT_DATA_PACKET;
	nci_send_packet.PBF = 0;
	nci_send_packet.GID = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID = 0x00;
	if (g_apiCmd.para1 == ESE_HOST)
	{
		nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.ese_gate_pipe); //
	}
	else if (g_apiCmd.para1 == SUPER_SIM_HOST)
	{
		nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.supersim_gate_pipe); //
	}
	nci_send_packet.PAYLOAD[i++] = 0x50; //0x01
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciResponseOk");
}
static void NFCC_HciEndTranstion(void)
{
	/** -->  01 00 02 89 61  
<--  01 00 04 89 50 68 81 */
	//数据包：MT(3bit)+PBF(1bit)+Conn ID(4bit)+RFU(6bit)+CR(2bit)+payloadlen(8bit)+payload(nbits)
	uint8_t i = 0;
	NFCC_NciClean();
	nci_send_packet.MT	= MT_DATA_PACKET;
	nci_send_packet.PBF = 0;
	nci_send_packet.GID = HCI_DATA_CONN_ID; //connect id
	nci_send_packet.OID = 0x00;
	if (g_apiCmd.para1 == ESE_HOST)
	{
		nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.ese_gate_pipe); //
	}
	else if (g_apiCmd.para1 == SUPER_SIM_HOST)
	{
		nci_send_packet.PAYLOAD[i++] = (0x80 | g_NfcInfo.supersim_gate_pipe); //
	}
	nci_send_packet.PAYLOAD[i++] = 0x61;
	nci_send_packet.PAYLOAD_LEN	 = i;
	NFC_TRACE_DEBUG("NFCC_HciResponseOk");
}
#endif

#if NFCC_HCE_SUPPORT
static void NFCC_HceTranstion(void)
{
	//00 00 02 90 00
	// NFCC_NciClean();
	nci_send_packet.MT	= MT_DATA_PACKET;
	nci_send_packet.GID = RF_DATA_CONN_ID; //hce
	nci_send_packet.OID = 0x00;
	NFC_TRACE_DEBUG("NFCC_HceTranstion");
	//其他数据由于设计分包，在外面进行赋值
}

#endif

static void NFCC_NfceeDiscover(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NFCEE_MANAGE;
	nci_send_packet.OID			= OID_NFCEE_DISCOVER;
	nci_send_packet.PAYLOAD_LEN = 0x00;
	NFC_TRACE_DEBUG("NFCC_NfceeDiscover");
}

static void NFCC_RfDeactiveIdleCallBack(COMMON_NCI_PACKET *pData)
{
	t_NFCC_rf_Status nfcc_rf_type = NFCC_GetRFGlobalStatus();

	switch (nfcc_rf_type)
	{
	case RFST_DISCOVERY:
	case RFST_W4_ALL_DISCOVERIES:
	case RFST_LISTEN_SLEEP:
	case RFST_W4_HOST_SELECT:
	case RFST_IDLE:
	case RFST_LISTEN_ACTIVE:
	case RFST_POLL_ACTIVE:
		NFCC_SetRFGlobalStatus(RFST_IDLE);
		NFCC_SetGlobalStatus(NFCC_STATUS_IDLE);
		break;
	default:
		NFC_TRACE_ERROR("func:%s; line:%d,NFCC_GetRFGlobalStatus EEROR", __FUNCTION__, __LINE__);
		NFCC_SetRFGlobalStatus(RFST_IDLE); //todo ... 不应该进入此处，需要做断言处理
		NFCC_SetGlobalStatus(NFCC_STATUS_IDLE);
		break;
	}
}
static void NFCC_RfDeactiveSleepCallBack(COMMON_NCI_PACKET *pData)
{
	t_NFCC_rf_Status nfcc_rf_type = NFCC_GetRFGlobalStatus();

	switch (nfcc_rf_type)
	{
	case RFST_DISCOVERY:
	case RFST_W4_ALL_DISCOVERIES:
	case RFST_LISTEN_SLEEP:
	case RFST_W4_HOST_SELECT:
	case RFST_IDLE:
		NFCC_SetRFGlobalStatus(RFST_IDLE);
		NFCC_SetGlobalStatus(NFCC_STATUS_IDLE);
		break;
	case RFST_LISTEN_ACTIVE:
		NFCC_SetRFGlobalStatus(RFST_LISTEN_SLEEP);
		break;
	case RFST_POLL_ACTIVE:
		NFCC_SetRFGlobalStatus(RFST_W4_HOST_SELECT);
		break;
	default:
		NFC_TRACE_ERROR("func:%s; line:%d,NFCC_GetRFGlobalStatus EEROR", __FUNCTION__, __LINE__);
		NFCC_SetRFGlobalStatus(RFST_IDLE); //todo ... 不应该进入此处，需要做断言处理
		NFCC_SetGlobalStatus(NFCC_STATUS_IDLE);
		break;
	}
}

static void NFCC_RfDeactiveDiscoveryCallBack(COMMON_NCI_PACKET *pData)
{
	t_NFCC_rf_Status nfcc_rf_type = NFCC_GetRFGlobalStatus();
	uint8_t ucOID				  = pData->OID;

	switch (nfcc_rf_type)
	{
	case RFST_DISCOVERY:
	case RFST_W4_ALL_DISCOVERIES:
	case RFST_LISTEN_SLEEP:
	case RFST_W4_HOST_SELECT:
	case RFST_IDLE:
		NFCC_SetRFGlobalStatus(RFST_IDLE);
		NFCC_SetGlobalStatus(NFCC_STATUS_IDLE);
		break;
	case RFST_LISTEN_ACTIVE:
		NFCC_SetRFGlobalStatus(RFST_DISCOVERY);
		NFCC_SetGlobalStatus(NFCC_STATUS_DISCOVERY_CE);
		break;
	case RFST_POLL_ACTIVE:
		NFCC_SetRFGlobalStatus(RFST_DISCOVERY);
		NFCC_SetGlobalStatus(NFCC_STATUS_DISCOVERY_RW);
		break;
	default:
		NFC_TRACE_ERROR("func:%s; line:%d,NFCC_GetRFGlobalStatus EEROR", __FUNCTION__, __LINE__);
		NFCC_SetRFGlobalStatus(RFST_IDLE); //todo ... 不应该进入此处，需要做断言处理
		NFCC_SetGlobalStatus(NFCC_STATUS_IDLE);
		break;
	}
}

static void NFCC_HciGetEseApdugateWtxCallBack(COMMON_NCI_PACKET *pData)
{
	g_nfc_dm.HCI_ese_apdugate_wtx_time_ms = 1000 + 50;//HCI规范默认1000ms，由于RTOS计时误差，加大50ms
	if (pData->MT == 0 && pData->GID == 1 && pData->OID == 0 && pData->PAYLOAD_LEN == 4 && pData->PAYLOAD[1] == 0x80)	
	{
		g_nfc_dm.HCI_ese_apdugate_wtx_time_ms = (uint16_t)(pData->PAYLOAD[2] << 8 | pData->PAYLOAD[3]) + 50;
		NFC_TRACE_DEBUG("HCI_ese_apdugate_wtx_time_ms:%d", g_nfc_dm.HCI_ese_apdugate_wtx_time_ms);
	}
	else
	{
		NFC_TRACE_ERROR("NFCC_HciGetEseApdugateWtxCallBack data rsp error, MT:%#x, GID:%#x, OID:%#x, PAYLOAD_LEN:%#x, PAYLOAD[1](status):%#x",
						pData->MT, pData->GID, pData->OID, pData->PAYLOAD_LEN, pData->PAYLOAD[1]);
	}
}

static void NFCC_HciGetEsimApdugateWtxCallBack(COMMON_NCI_PACKET *pData)
{
	g_nfc_dm.HCI_ese_apdugate_wtx_time_ms = 1000 + 50;//HCI规范默认1000ms，由于RTOS计时误差，加大50ms
	if (pData->MT == 0 && pData->GID == 1 && pData->OID == 0 && pData->PAYLOAD_LEN == 4 && pData->PAYLOAD[1] == 0x80)	
	{
		g_nfc_dm.HCI_esim_apdugate_wtx_time_ms = (uint16_t)(pData->PAYLOAD[2] << 8 | pData->PAYLOAD[3]) + 50;
		NFC_TRACE_DEBUG("HCI_esim_apdugate_wtx_time_ms:%d", g_nfc_dm.HCI_esim_apdugate_wtx_time_ms);
	}
	else
	{
		NFC_TRACE_ERROR("NFCC_HciGetEsimApdugateWtxCallBack data rsp error, MT:%#x, GID:%#x, OID:%#x, PAYLOAD_LEN:%#x, PAYLOAD[1](status):%#x",
						pData->MT, pData->GID, pData->OID, pData->PAYLOAD_LEN, pData->PAYLOAD[1]);
	}
}



#if NFCC_ESE_SUPPORT
static void NFCC_NfceeEseEnable(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NFCEE_MANAGE;
	nci_send_packet.OID			= OID_NFCEE_MODE_SET;
	nci_send_packet.PAYLOAD_LEN = 0x02;
	nci_send_packet.PAYLOAD[0]	= ESE_HOST; //route
	nci_send_packet.PAYLOAD[1]	= 0x01;		//enable
	NFC_TRACE_DEBUG("NFCC_NfceeEseEnable");
}

static void NFCC_NfceeEseEnableCallback(COMMON_NCI_PACKET *pData)
{
	if (pData->PAYLOAD[0] == 0)
	{
		g_NfcInfo.EseGateIsReady = true;
	}
	else if (pData->PAYLOAD[0] == 3)
	{
		g_NfcInfo.EseGateIsReady = false;
	}
	else
	{
		g_NfcInfo.EseGateIsReady = false;
	}
	NFC_TRACE_DEBUG("---------NFCC_NfceeEseEnableCallback:g_NfcInfo.EseGateIsReady = 0x%02x------", g_NfcInfo.EseGateIsReady);
}

static void NFCC_NfceeEseDisable(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NFCEE_MANAGE;
	nci_send_packet.OID			= OID_NFCEE_MODE_SET;
	nci_send_packet.PAYLOAD_LEN = 0x02;
	nci_send_packet.PAYLOAD[0]	= ESE_HOST; //route
	nci_send_packet.PAYLOAD[1]	= 0x00;		//disable
	NFC_TRACE_DEBUG("NFCC_NfceeEseDisable");
}
static void NFCC_NfceeEsePower(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NFCEE_MANAGE;
	nci_send_packet.OID			= OID_NFCEE_MODE_SET;
	nci_send_packet.PAYLOAD_LEN = 0x02;
	nci_send_packet.PAYLOAD[0]	= ESE_HOST; //route
	nci_send_packet.PAYLOAD[1]	= 0x01;		//enable
	NFC_TRACE_DEBUG("NFCC_NfceeEsePower");
}
#endif

#if NFCC_SURPER_SIM_SUPPORT
static void NFCC_NfceeSurperSimEnable(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NFCEE_MANAGE;
	nci_send_packet.OID			= OID_NFCEE_MODE_SET;
	nci_send_packet.PAYLOAD_LEN = 0x02;
	nci_send_packet.PAYLOAD[0]	= SUPER_SIM_HOST; //route
	nci_send_packet.PAYLOAD[1]	= 0x01;			  //enable
	NFC_TRACE_DEBUG("NFCC_NfceeSurperSimEnable");
}

static void NFCC_NfceeSurperSimEnableCallBack(COMMON_NCI_PACKET *pData)
{
	if (pData->PAYLOAD[0] == 0)
	{
		g_NfcInfo.SuperSimGateIsReady = true;
	}
	else if (pData->PAYLOAD[0] == 3)
	{
		g_NfcInfo.SuperSimGateIsReady = false;
	}
	else
	{
		g_NfcInfo.SuperSimGateIsReady = false;
	}
	NFC_TRACE_DEBUG("---------NFCC_NfceeSurperSimEnableCallBack:g_NfcInfo.SuperSimGateIsReady = 0x%02x------", g_NfcInfo.SuperSimGateIsReady);
}

static void NFCC_NfceeSurperSimDisable(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NFCEE_MANAGE;
	nci_send_packet.OID			= OID_NFCEE_MODE_SET;
	nci_send_packet.PAYLOAD_LEN = 0x02;
	nci_send_packet.PAYLOAD[0]	= SUPER_SIM_HOST; //route
	nci_send_packet.PAYLOAD[1]	= 0x00;			  //disable
	NFC_TRACE_DEBUG("NFCC_NfceeSurperSimDisable");
}
#endif

#if NFCC_SURPER_SIM_SUPPORT | NFCC_ESE_SUPPORT
static void NFCC_NfceePower(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NFCEE_MANAGE;
	nci_send_packet.OID			= OID_NFCEE_POWER_AND_LINK_CNTRL;
	nci_send_packet.PAYLOAD_LEN = 0x02;
	nci_send_packet.PAYLOAD[0]	= ESE_HOST; //route
	nci_send_packet.PAYLOAD[1]	= 0x03;			  //power on
	NFC_TRACE_DEBUG("NFCC_NfceePower");
}
#endif

#if NFCC_LOCAL_M1_SUPPORT

static void NFCC_NfceeLocalM1Enable(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_NFCEE_MANAGE;
	nci_send_packet.OID			= OID_NFCEE_MODE_SET;
	nci_send_packet.PAYLOAD_LEN = 0x02;
	nci_send_packet.PAYLOAD[0]	= LOCAL_M1; //route
	nci_send_packet.PAYLOAD[1]	= 0x01;		//enable
	NFC_TRACE_DEBUG("NFCC_NfceeLocalM1Enable");
}

static void NFCC_Localm1Query(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= QUERY_LOCAL_M1;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= g_LocalM1CurrentIndex; //app  todo... 最大4张，需要特殊处理
	NFC_TRACE_DEBUG("NFCC_Localm1Query");
}

static void NFCC_Localm1QueryCallback(COMMON_NCI_PACKET *pData)
{
	NFC_TRACE_DEBUG("func:%s; line:%d, g_LocalM1CurrentIndex = %d", __FUNCTION__, __LINE__, g_LocalM1CurrentIndex);

	if (pData->PAYLOAD[0] == 0x00) //查询到了超级SIM卡
	{
		NFC_TRACE_DEBUG("test1111111");
		g_NfcInfo.LocalM1IsExit |= (1 << g_LocalM1CurrentIndex); //查询到LocalM1
		g_apiCmd.out_data_len = 8;
		memcpy(g_apiCmd.p_out_data, &pData->PAYLOAD[1], 8);
	}
	else
	{
		g_NfcInfo.LocalM1IsExit &= (~(1 << g_LocalM1CurrentIndex)); //未查询到LocalM1
		g_apiCmd.out_data_len = 0;
	}
	NFC_TRACE_DEBUG("func:%s; line:%d, g_NfcInfo.LocalM1IsExit = %d", __FUNCTION__, __LINE__, g_NfcInfo.LocalM1IsExit);
}
static void NFCC_Localm1Create(void)
{
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= CREATE_LOCAL_M1;
	nci_send_packet.PAYLOAD_LEN = (16 + 8 + 1);
	nci_send_packet.PAYLOAD[0]	= g_LocalM1CurrentIndex;	   //todo...
	memcpy(&nci_send_packet.PAYLOAD[1], appSysData[0], 8);	   //todo ...
	memcpy(&nci_send_packet.PAYLOAD[9], appInitKeySet[0], 16); //todo ...
	NFC_TRACE_DEBUG("NFCC_Localm1Create");
}

static void NFCC_Localm1CreateCallback(COMMON_NCI_PACKET *pData)
{
	if (pData->PAYLOAD[0] == STATUS_OK)
	{
		g_NfcInfo.LocalM1IsExit |= (1 << g_LocalM1CurrentIndex); //记录存在的ID
	}
}

static void NFCC_Localm1Delete(void)
{
	uint8_t app = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= DELETE_LOCAL_M1;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= g_LocalM1CurrentIndex; //todo...
	NFC_TRACE_DEBUG("NFCC_Localm1Delete");
}

static void NFCC_Localm1DeleteCallback(COMMON_NCI_PACKET *pData)
{
	if (pData->PAYLOAD[0] == STATUS_OK)
	{
		g_NfcInfo.LocalM1IsExit &= (~(1 << g_LocalM1CurrentIndex)); //删除记录的ID
	}
}

static void NFCC_Localm1Activate(void)
{
	uint8_t app = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= ACTIVATE_LOCAL_M1;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= g_LocalM1CurrentIndex; //todo...
	NFC_TRACE_DEBUG("NFCC_Localm1Activate");
}

static void NFCC_Localm1ActivateCallback(COMMON_NCI_PACKET *pData)
{
	if ((pData->PAYLOAD[0] == STATUS_OK)
		|| (pData->PAYLOAD[0] == STATUS_ACTIVATION_EXECUTED))
	{
		g_NfcInfo.LocalM1IsAcitive	 = 1;					  //激活成功
		g_NfcInfo.LocalM1ActiveAppId = g_LocalM1CurrentIndex; //激活成功
	}
}

static void NFCC_Localm1WriteSector(void)
{
	uint8_t app = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= WRITE_M1_SECTOR;
	nci_send_packet.PAYLOAD_LEN = g_apiCmd.in_data_len;
	memcpy(&nci_send_packet.PAYLOAD[0], g_apiCmd.p_in_data, g_apiCmd.in_data_len);
	NFC_TRACE_DEBUG("NFCC_Localm1WriteSector");
}

static void NFCC_Localm1WriteSectorCallBack(COMMON_NCI_PACKET *pData)
{
	if (g_apiCmd.p_out_data != NULL)
	{
		memcpy(g_apiCmd.p_out_data, &pData->PAYLOAD[0], pData->PAYLOAD_LEN);
		g_apiCmd.out_data_len = pData->PAYLOAD_LEN;
		NFC_TRACE_DEBUG("write sector data to buffer");
	}
}

static void NFCC_Localm1ReadSector(void)
{
	uint8_t app = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= READ_M1_SECTOR;
	nci_send_packet.PAYLOAD_LEN = g_apiCmd.in_data_len;
	;
	memcpy(&nci_send_packet.PAYLOAD[0], g_apiCmd.p_in_data, g_apiCmd.in_data_len);
	NFC_TRACE_DEBUG("NFCC_Localm1ReadSector");
}

static void NFCC_Localm1ReadSectorCallBack(COMMON_NCI_PACKET *pData)
{
	if (g_apiCmd.p_out_data != NULL)
	{
		memcpy(g_apiCmd.p_out_data, &pData->PAYLOAD[0], pData->PAYLOAD_LEN);
		g_apiCmd.out_data_len = pData->PAYLOAD_LEN;
		NFC_TRACE_DEBUG("read sector data to buffer");
	}
}

static void NFCC_Localm1Deactive(void)
{
	uint8_t app = 0;
	NFCC_NciClean();
	nci_send_packet.MT			= MT_CTL_CMD_PACKET;
	nci_send_packet.PBF			= 0;
	nci_send_packet.GID			= GID_PROPRIETARY;
	nci_send_packet.OID			= DEACTIVE_LOCAL_M1;
	nci_send_packet.PAYLOAD_LEN = 0x01;
	nci_send_packet.PAYLOAD[0]	= g_LocalM1CurrentIndex; //todo...
	NFC_TRACE_DEBUG("NFCC_Localm1Deactive");
}

static void NFCC_Localm1DeactiveCallback(COMMON_NCI_PACKET *pData)
{
	if ((pData->PAYLOAD[0] == STATUS_OK) || (pData->PAYLOAD[0] == STATUS_DEACTIVATION_EXECUTED))
	{
		g_NfcInfo.LocalM1IsAcitive = 0; //清除激活标志
	}
}

#endif
#pragma GCC diagnostic pop
