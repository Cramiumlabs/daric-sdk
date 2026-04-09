#include "nci.h"
#include "port.h"
#include "nfc_timer.h"
#include "nfc_stage.h"

extern void hed_nfc_set_ver(uint8_t *pVer);
extern void hed_nfc_set_chip_ver(uint8_t *chip_Ver);

NFC_A_Poll_Para NfcAPollMode;

uint8_t gCurrentTagIndex = 0; //正在处理的卡片
CARD_PARA NfcCardinfo[3];	  //3种组合

uint8_t g_NeedNextPoll = 0;

uint16_t write_buffer(uint8_t *data, uint16_t len)
{
	DRV_Status status;
	uint16_t numWrote = 0;
	NFC_TRACE_BUFFER_INFO("--> ", data, len);
	status = I2CSequentialTx(data, len);
	if (DRV_STATUS_SUCCESS == status)
	{
		NFC_TRACE_DEBUG("I2CSequentialTx success");
		numWrote = len;
	}
	else if (DRV_STATUS_BUSY == status)
	{
		NFC_TRACE_WARNING("I2CSequentialTx error, status: DRV_STATUS_BUSY");
	}
	else if (DRV_STATUS_NO_DATA != status)
	{
		NFC_TRACE_ERROR("I2CSequentialTx error, status: %d", status);
	}
	return numWrote;
}

t_RetStatus Nci_wrap(uint8_t *data, COMMON_NCI_PACKET *const nci_recv_packet)
{
	nci_recv_packet->MT			 = data[0] & 0xe0;
	nci_recv_packet->PBF		 = data[0] & 0x10;
	nci_recv_packet->GID		 = data[0] & 0x0f;
	nci_recv_packet->OID		 = data[1] & 0x3f;
	nci_recv_packet->PAYLOAD_LEN = data[2];
	memcpy(nci_recv_packet->PAYLOAD, &data[3], nci_recv_packet->PAYLOAD_LEN);
	return NO_ERR;
}

t_RetStatus Nci_unwrap(COMMON_NCI_PACKET nci_send_packet, uint8_t mode)
{
    uint8_t *pbuf = (uint8_t *)port_mem_getMem(BUFFER_BLOCK_SIZE);
    if (pbuf == NULL)
        return MT_ERROR;
    uint16_t plen = nci_send_packet.PAYLOAD_LEN + 3; //buf数据长度
    pbuf[0]		  = nci_send_packet.MT | nci_send_packet.PBF | nci_send_packet.GID;
    pbuf[1]		  = nci_send_packet.OID;
    pbuf[2]		  = nci_send_packet.PAYLOAD_LEN;
    memcpy(pbuf + 3, nci_send_packet.PAYLOAD, nci_send_packet.PAYLOAD_LEN);
    // timer_wait_ms(2);//todo ...
    write_buffer(pbuf, plen); //直接发送
    port_mem_freeMem(pbuf);
    return NO_ERR;
}

t_RetStatus Nci_unwrap_fido(COMMON_NCI_PACKET nci_send_packet, uint8_t mode)
{
    uint8_t *pbuf = (uint8_t *)port_mem_getMem(BUFFER_BLOCK_SIZE);
    if (pbuf == NULL)
        return MT_ERROR;
    uint16_t plen = nci_send_packet.PAYLOAD_LEN + 3;

    pbuf[0]		  = 0x00;
    pbuf[1]		  = nci_send_packet.OID;
    pbuf[2]		  = nci_send_packet.PAYLOAD_LEN;
    memcpy(pbuf + 3, nci_send_packet.PAYLOAD, nci_send_packet.PAYLOAD_LEN);
    write_buffer(pbuf, plen);
    port_mem_freeMem(pbuf);
    return NO_ERR;
}

void Nci_Core_Rsp(COMMON_NCI_PACKET *nci_recv_packet)
{
	NFC_TRACE_DEBUG("Nci_Core_Rsp OID = %x", nci_recv_packet->OID);
	switch (nci_recv_packet->OID)
	{
	case OID_CORE_RESET:
		NFC_TRACE_DEBUG("Nci_Core_Reset_Rsp");
		break;
	case OID_CORE_INIT:
		NFC_TRACE_DEBUG("Nci_Core_Init_Rsp");
		break;
	case OID_CORE_SET_CONFIG:
		NFC_TRACE_DEBUG("Nci_Core_Set_Config_Rsp");
		break;
	case OID_CORE_GET_CONFIG:
		NFC_TRACE_DEBUG("Nci_Core_Get_Config_Rsp");
		break;
	case OID_CONN_GREATE:
		NFC_TRACE_DEBUG("Nci_Core_Conn_Create_Rsp");
		break;
	case OID_CONN_CLOSE:
		NFC_TRACE_DEBUG("Nci_Core_Conn_Close_Rsp");
		break;
	default: break;
	}
}

void Nci_Core_Ntf(COMMON_NCI_PACKET *nci_recv_packet)
{
#define NCI_RESET_NTF_COMPLEX_LENGTH	0x27
#define NCI_RESET_NTF_SIMPLE_LENGTH		0x0C
#define HED_CHIP_VERSION_OFFSET_OF_VENDOR_DATA	3
#define HED_VENDOR_DATA_OFFSET		5

	uint8_t fw_version[2] = {0};
	uint8_t chip_version[2] = {0};
	uint8_t reset_trigger = 1;
	NFC_TRACE_DEBUG("Nci_Core_Ntf OID = %x", nci_recv_packet->OID);
	switch (nci_recv_packet->OID)
	{
	case OID_CORE_RESET_NTF:
		if (nci_recv_packet->PAYLOAD_LEN == NCI_RESET_NTF_COMPLEX_LENGTH ||
			nci_recv_packet->PAYLOAD_LEN == NCI_RESET_NTF_SIMPLE_LENGTH ) 
		{
			reset_trigger = nci_recv_packet->PAYLOAD[0];
			if (reset_trigger == 0x01 || 	//NFCC上电
				reset_trigger == 0x02		//CORE_RESET_CMD被接收
				)
			{
				NFC_TRACE_DEBUG("Nci_Core_Reset_Ntf");
				memcpy(fw_version, nci_recv_packet->PAYLOAD + nci_recv_packet->PAYLOAD_LEN - 2, 2);
				NFC_TRACE_BUFFER_INFO("Firmware Version:", fw_version, 2);
				hed_nfc_set_ver(fw_version);
				if (nci_recv_packet->PAYLOAD_LEN == NCI_RESET_NTF_COMPLEX_LENGTH)
				{
					memcpy(chip_version, nci_recv_packet->PAYLOAD + HED_VENDOR_DATA_OFFSET + HED_CHIP_VERSION_OFFSET_OF_VENDOR_DATA, 2);
				}
				else if (nci_recv_packet->PAYLOAD_LEN == NCI_RESET_NTF_SIMPLE_LENGTH)
				{
					memcpy(chip_version, nci_recv_packet->PAYLOAD + nci_recv_packet->PAYLOAD_LEN - 4, 2);
				}
				NFC_TRACE_BUFFER_INFO("Chip Version:", chip_version, 2); //Part of n30a(SOC1) is full ff
				hed_nfc_set_chip_ver(chip_version);
			}
			else //0x00:NFCC中的不可修复错误; 0xA0-0xFF:其他错误
			{
				NFC_TRACE_DEBUG("OID_CORE_RESET_NTF, reset_trigger[%#x] error", reset_trigger);
				if (g_nfc_dm.sdk_callback_fun_handle_event)
				{
					g_nfc_dm.sdk_callback_fun_handle_event(SDK_CB_EVENT_NFCC_FAULT);
				}
			}
		}
		else 
		{
			//NFC_TRACE_DEBUG("OID_CORE_RESET_NTF, payload_len error, nci_recv_packet->PAYLOAD_LEN", nci_recv_packet->PAYLOAD_LEN);
			if (g_nfc_dm.sdk_callback_fun_handle_event)
			{
				g_nfc_dm.sdk_callback_fun_handle_event(SDK_CB_EVENT_NFCC_FAULT);
			}
		}
		break;
	case OID_CORE_CONN_CREDITS_NTF:
		NFC_TRACE_DEBUG("Nci_Core_Conn_Credits_Ntf");
		break;
	case OID_CORE_GENERIC_ERROR_NTF:
		NFC_TRACE_DEBUG("Nci_Core_Generic_Error_Ntf");
		break;
	case OID_CORE_INTERFACE_ERROR_NTF:
		NFC_TRACE_DEBUG("Nci_Core_Interface_Error_Ntf");
		break;
	default: break;
	}
}

void Nci_Rf_Management_Rsp(COMMON_NCI_PACKET *nci_recv_packet)
{
	NFC_TRACE_DEBUG("Nci_Rf_Management_Rsp OID = %x", nci_recv_packet->OID);
	switch (nci_recv_packet->OID)
	{
	case OID_RF_DISCOVER_MAP:
		NFC_TRACE_DEBUG("Nci_Rf_Discover_Map_Rsp");
		break;
	case OID_RF_SET_ROUTING:
		NFC_TRACE_DEBUG("Nci_Rf_Set_Routing_Rsp");
		break;
	case OID_RF_GET_ROUTING:
		NFC_TRACE_DEBUG("Nci_Rf_Get_Routing_Rsp");
		break;
	case OID_RF_DISCOVER:
		NFC_TRACE_DEBUG("Nci_Rf_Get_Discover_Rsp");
		break;
	case OID_RF_DISCOVER_SELECT:
		NFC_TRACE_DEBUG("Nci_Rf_Discover_Select_Rsp");
		break;
	case OID_RF_DEACTIVATE:
		NFC_TRACE_DEBUG("Nci_Rf_Deactivate_Rsp");
		break;

	default: break;
	}
}

extern unsigned char g_pollNtfFlag;
void Nci_Rf_Management_Ntf(COMMON_NCI_PACKET *nci_recv_packet)
{
	t_NFCC_rf_Status rf_status = NFCC_GetRFGlobalStatus();
	t_NFCC_Config *pNfcConfig  = NFCC_GetConfig();

	NFC_TRACE_DEBUG("Nci_Rf_Management_Rsp OID = %x", nci_recv_packet->OID);
	switch (nci_recv_packet->OID)
	{
	case OID_RF_GET_ROUTING_NTF:
		NFC_TRACE_DEBUG("Nci_Rf_Get_Routing_Ntf");
		break;
	case OID_RF_DISCOVER_NTF: //6103 rw slect success
		NFC_TRACE_DEBUG("Nci_Rf_Discover_Ntf");
		if (nci_recv_packet->PAYLOAD[0] < 4)
		{ //大于4不处理
			Process_card_para_6103(&NfcCardinfo[nci_recv_packet->PAYLOAD[0] - 1], nci_recv_packet);
		}
		break;
	case OID_RF_INTF_ACTIVATED_NTF: //6105 表示寻到卡片了
		// NFC_TRACE_DEBUG("Nci_Rf_Intf_Activated_Ntf =0x%02x 射频接口激活通知", nci_recv_packet->PAYLOAD[3]);
		NFC_TRACE_DEBUG("Nci_Rf_Intf_Activated_Ntf =0x%02x Rf interface activation notification", nci_recv_packet->PAYLOAD[3]);

		if (rf_status == RFST_DISCOVERY) //rf status change
		{
			if (pNfcConfig->nfc_mode == MODE_NFC_RW)
			{
				NFCC_SetRFGlobalStatus(RFST_POLL_ACTIVE);
				NFCC_SetGlobalStatus(NFCC_STATUS_ACTIVE_RW);
			}
			else
			{
				NFCC_SetRFGlobalStatus(RFST_LISTEN_ACTIVE);
				NFCC_SetGlobalStatus(NFCC_STATUS_ACTIVE_CE);
			}
		}
		else if (rf_status == RFST_W4_HOST_SELECT)
		{
			NFCC_SetRFGlobalStatus(RFST_POLL_ACTIVE);
			NFCC_SetGlobalStatus(NFCC_STATUS_ACTIVE_RW);
		}
		else if (rf_status == RFST_LISTEN_SLEEP)
		{
			NFCC_SetRFGlobalStatus(RFST_LISTEN_ACTIVE);
			NFCC_SetGlobalStatus(NFCC_STATUS_ACTIVE_CE);
		}
		else
		{
			NFCC_SetRFGlobalStatus(RFST_LISTEN_ACTIVE);
			NFCC_SetGlobalStatus(NFCC_STATUS_ACTIVE_CE);
		}

		switch (nci_recv_packet->PAYLOAD[3])
		{
		case NFC_A_PASSIVE_POLL_MODE:
		case NFC_B_PASSIVE_POLL_MODE:
		case NFC_F_PASSIVE_POLL_MODE:
		case NFC_V_PASSIVE_POLL_MODE:
			if (nci_recv_packet->PAYLOAD[0] < 4)
			{ //大于4不处理
				Process_card_para_6105(&NfcCardinfo[nci_recv_packet->PAYLOAD[0] - 1], nci_recv_packet);
			}
			break;
		case NFC_A_PASSIVE_LISTEN_MODE:
			break;
		case NFC_B_PASSIVE_LISTEN_MODE:
			break;
		case NFC_F_PASSIVE_LISTEN_MODE:
			break;
		case 0x83:
			break;
		}
		break;
	case OID_RF_DEACTIVATE_NTF:
		if (g_pollNtfFlag == 1) //表示是 poll 中自动产生的NTF
		{
			if (nci_recv_packet->PAYLOAD[0] == 0x01) //sleep
			{
				NFCC_SetRFGlobalStatus(RFST_LISTEN_SLEEP);
			}
			else if (nci_recv_packet->PAYLOAD[0] == 0x03) //discovery mode
			{
				if ((rf_status == RFST_LISTEN_ACTIVE) || (rf_status == RFST_LISTEN_SLEEP)) //rf status change
				{
					NFCC_SetRFGlobalStatus(RFST_DISCOVERY);
					NFCC_SetGlobalStatus(NFCC_STATUS_DISCOVERY_CE);
				}
				else if (rf_status == RFST_POLL_ACTIVE)
				{
					NFCC_SetRFGlobalStatus(RFST_DISCOVERY);
					NFCC_SetGlobalStatus(NFCC_STATUS_DISCOVERY_RW);
				}
				else
				{
					;
				}
			}
		}
		NFC_TRACE_DEBUG("Nci_Rf_Intf_DeActivated_Ntf");
		break;
	case OID_RF_FIELD_INFO_NTF:
		if (nci_recv_packet->PAYLOAD[0] == 0)
		{
			NFC_TRACE_DEBUG("&&&&& test  ntf   &&&&&&&");
			// NFCC_SetGlobalStatus(NFCC_STATUS_OUT_FIELD); //
			NFCC_SetRFGlobalStatus(RFST_DISCOVERY);
			NFCC_SetGlobalStatus(NFCC_STATUS_DISCOVERY_CE);
		}
		else
		{
			// NFCC_SetGlobalStatus(NFCC_STATUS_IN_FIELD); //
		}
		NFC_TRACE_DEBUG("Nci_Rf_Field_Info_Ntf");
		break;
	case OID_RF_NFCEE_ACTION_NTF:

		NFC_TRACE_DEBUG("Nci_Rf_NFCEE_Action_Ntf");
		break;
	case OID_RF_NFCEE_DISCOVERY_REQ_NTF:
		NFC_TRACE_DEBUG("Nci_Rf_NFCEE_Discovery_Req_Ntf");
		break;
	default: break;
	}
}
//add for local M1 handle
void Nci_localM1_Rsp(COMMON_NCI_PACKET *nci_recv_packet)
{
	NFC_TRACE_DEBUG("Nci_localM1_Rsp OID = %x", nci_recv_packet->OID);
	switch (nci_recv_packet->OID)
	{
	case CREATE_LOCAL_M1:
		// NFC_TRACE_DEBUG("OID_PRY_CREATE_LOCAL_M1 在LocalM1中创建一个APP应用");
		NFC_TRACE_DEBUG("OID_PRY_CREATE_LOCAL_M1 Create M1 APP");
		break;
	case DELETE_LOCAL_M1:
		// NFC_TRACE_DEBUG("OID_RF_DELETE_LOCAL_M1 在LocalM1中删除一个APP应用");
		NFC_TRACE_DEBUG("OID_RF_DELETE_LOCAL_M1 Delete M1 APP");
		break;
	case QUERY_LOCAL_M1:
		// NFC_TRACE_DEBUG("OID_RF_QUERY_LOCAL_M1 在LocalM1中 查询 一个APP应用");
		NFC_TRACE_DEBUG("OID_RF_QUERY_LOCAL_M1 Query M1 APP");
		break;
	case ACTIVATE_LOCAL_M1:
		// NFC_TRACE_DEBUG("OID_RF_ACTIVATE_LOCAL_M1 在LocalM1中 激活 一个APP应用");
		NFC_TRACE_DEBUG("OID_RF_ACTIVATE_LOCAL_M1 Activate M1 APP");
		break;
	case DEACTIVE_LOCAL_M1:
		// NFC_TRACE_DEBUG("OID_DEACTIVE_LOCAL_M1  在LocalM1中  去激活  一个APP应用");
		NFC_TRACE_DEBUG("OID_DEACTIVE_LOCAL_M1  Deactive M1 APP");
		break;
	case WRITE_M1_SECTOR:
		// NFC_TRACE_DEBUG("OID_WRITE_M1_SECTOR 向LocalM1 指定的APP应用的扇区写入扇区数据");
		NFC_TRACE_DEBUG("OID_WRITE_M1_SECTOR Write M1 BANK DATA");
		break;
	case READ_M1_SECTOR:
		// NFC_TRACE_DEBUG("OID_READ_M1_SECTOR 向LocalM1 指定的APP应用的扇区 读出扇区数据");
		NFC_TRACE_DEBUG("OID_READ_M1_SECTOR Read M1 BANK DATA");
		break;
	case RESET_M1_SECTOR:
		// NFC_TRACE_DEBUG("OID_RESET_M1_SECTOR 复位LocalM1 指定的APP应用的扇区扇区数据");
		NFC_TRACE_DEBUG("OID_RESET_M1_SECTOR Reset M1 BANK DATA");
		break;
	default: break;
	}
}
void Nci_Nfcee_Management_Rsp(COMMON_NCI_PACKET *nci_recv_packet)
{
	NFC_TRACE_DEBUG("Nci_Nfcee_Management_Rsp OID = %x", nci_recv_packet->OID);
	switch (nci_recv_packet->OID)
	{
	default: break;
	}
}

void Nci_Nfcee_Management_Ntf(COMMON_NCI_PACKET *nci_recv_packet)
{
	NFC_TRACE_DEBUG("Nci_Nfcee_Management_Ntf OID = %x,PAYLOAD_LEN:%d", nci_recv_packet->OID, nci_recv_packet->PAYLOAD_LEN);
}

void Nci_Rsp_Packet(COMMON_NCI_PACKET *nci_recv_packet)
{
	switch (nci_recv_packet->GID)
	{
	case GID_NCI_CORE:
		Nci_Core_Rsp(nci_recv_packet);
		break;
	case GID_RF_MANAGE:
		Nci_Rf_Management_Rsp(nci_recv_packet);
		break;
	case GID_NFCEE_MANAGE:
		Nci_Nfcee_Management_Rsp(nci_recv_packet);
		break;
	case GID_PROPRIETARY:
		Nci_localM1_Rsp(nci_recv_packet);
		break;
	default: break;
	}
}

void Nci_Ntf_Packet(COMMON_NCI_PACKET *nci_recv_packet)
{
	NFC_TRACE_DEBUG("Nci_Ntf_Packet GID = %x", nci_recv_packet->GID);
	switch (nci_recv_packet->GID)
	{
	case GID_NCI_CORE:
		Nci_Core_Ntf(nci_recv_packet);
		break;
	case GID_NFCEE_MANAGE:
		Nci_Nfcee_Management_Ntf(nci_recv_packet);
		break;
	case GID_RF_MANAGE:
		Nci_Rf_Management_Ntf(nci_recv_packet);
		break;

	default: break;
	}
}

uint16_t NfcCmdI2cRecvData(uint8_t *pbuf, uint16_t nBufferLen)
{
	uint16_t nLen = 0;

	// if (port_get_irq_flag() == TRUE)
//	{
		if ((port_hardware_irqIsComIrqHigh() == TRUE))
		{
			// debug_io(3);
			DRV_Status status;
			uint16_t maxByte = nBufferLen;
			status			 = I2CSequentialRx(pbuf, (uint16_t *)(&maxByte));
			if (DRV_STATUS_SUCCESS == status)
			{
				NFC_TRACE_BUFFER_INFO("<-- ", pbuf, maxByte);
				nLen = maxByte;
			}
			else if (DRV_STATUS_NO_DATA != status)
			{
				NFC_TRACE_ERROR("I2CSequentialRx status: %d", status);
			}
			return nLen;
		}
		else
		{
			// debug_io(4);
			// timer_wait_ms(5);
			// debug_io(5);
			return 0;
		}
//	}

//	return 0;
}

t_RetStatus Process_card_para_6103(CARD_PARA *CardPara, COMMON_NCI_PACKET *nci_recv_packet)
{
	uint8_t u8, *p;
	uint8_t notification	   = 0;
	t_NFCC_rf_Status rf_status = NFCC_GetRFGlobalStatus();

	NFC_TRACE_DEBUG("%s start", __FUNCTION__);

	//--------------------------num
	//复合卡的第一条数据：61 03 0F 01 04 00 0A 08 00 04 21 64 F2 15 01 20 00 02
	//复合卡的第二条数据：61 03 0F 02 80 00 0A 08 00 04 21 64 F2 15 01 08 00 00

	/**
	 * 
	 * 61 03 0F 
	 * 02 //RF_Discovery_ID
	 * 80 //RF_Protocol
	 * 00 //RF_Technology_Mode
	 * 0A //Length_Parameters
	 * 08 00 //sens_res 04  //uid lenth 21 64 F2 15 //uid 01 // sel 08 00 //sel_rsp
	 * 00
	 */
	notification = nci_recv_packet->PAYLOAD[(nci_recv_packet->PAYLOAD_LEN - 1)];

	if (rf_status == RFST_DISCOVERY) //rf status change
	{
		if (notification == 2)
		{
			NFCC_SetRFGlobalStatus(RFST_W4_ALL_DISCOVERIES);
		}
		else
		{
			NFCC_SetRFGlobalStatus(RFST_W4_HOST_SELECT);
		}
	}
	else if (rf_status == RFST_W4_ALL_DISCOVERIES)
	{
		if (notification == 2)
		{
			// NFCC_SetRFGlobalStatus(RFST_W4_ALL_DISCOVERIES);
		}
		else
		{
			NFCC_SetRFGlobalStatus(RFST_W4_HOST_SELECT);
		}
	}
	else
	{
		;
	}

	CardPara->RF_Discovery_ID = nci_recv_packet->PAYLOAD[0];
	CardPara->RF_Protocol	  = (NCI_RF_PROTOCOL_t)nci_recv_packet->PAYLOAD[1];
	if (CardPara->RF_Protocol == NCI_RF_PROTOCOL_ISO_DEP)
	{
		CardPara->RF_Interface = NCI_RF_INTERFACE_ISO_DEP;
	}
	if (CardPara->RF_Protocol == NCI_RF_PROTOCOL_M1_RW)
	{
		CardPara->RF_Interface = NCI_RF_INTERFACE_M1;
	}
	CardPara->RF_Technology_Mode = nci_recv_packet->PAYLOAD[2];
	CardPara->Length_Parameters	 = nci_recv_packet->PAYLOAD[3]; //09
	p							 = &nci_recv_packet->PAYLOAD[4];

	if (CardPara->RF_Technology_Mode == NFC_DISCOVERY_TYPE_POLL_A)
	{
		//        00112233 4455 66 7788 99 0A0B0C0D 0E0F        11223344
		//6105 14 01808000 ff01 09 0400 04 cef31070 0108        00000000   (mifare V1.0协议)
		//6105 16 01010100 ff01 0b 000c 04 45637700 00   02124c 00000000   (Tag1   V2.1协议)
		//6105 17 01010200 ff01 0c 4400 07 04053e32e33e81 0100  00000000   (Tag2   V1.0协议)
		/*
	    SENS_RES Response   2 bytes Defined in [DIGPROT] Available after Technology
	    Detection
	    NFCID1 length   1 byte  Length of NFCID1 Available after Collision Resolution
	    NFCID1  4, 7, or 10 bytes   Defined in [DIGPROT]Available after Collision
	    Resolution
	    SEL_RES Response    1 byte  Defined in [DIGPROT]Available after Collision
	    Resolution
	    HRx Length  1 Octets    Length of HRx Parameters collected from the response to
	    the T1T RID command.
	    HRx 0 or 2 Octets   If present, the first byte SHALL contain HR0 and the second
	    byte SHALL contain HR1 as defined in [DIGITAL].
	      */

		CardPara->RF_Technology_Para.mode = NFC_DISCOVERY_TYPE_POLL_A;
		STREAM_TO_ARRAY(CardPara->RF_Technology_Para.param.pa.sens_res, p, 2);
		CardPara->RF_Technology_Para.param.pa.nfcid1_len = *p++; //uid长度
		if (CardPara->RF_Technology_Para.param.pa.nfcid1_len > NCI_NFCID1_MAX_LEN)
		{
			CardPara->RF_Technology_Para.param.pa.nfcid1_len = NCI_NFCID1_MAX_LEN;
		}
		STREAM_TO_ARRAY(CardPara->RF_Technology_Para.param.pa.nfcid1, p, CardPara->RF_Technology_Para.param.pa.nfcid1_len);
		u8 = *p++;
		if (u8)
		{
			CardPara->RF_Technology_Para.param.pa.sel_rsp = *p++;
		}
		if (CardPara->Length_Parameters == (7 + CardPara->RF_Technology_Para.param.pa.nfcid1_len + u8)) /* 2(sens_res) + 1(len) +
                                        p_pa->nfcid1_len + 1(len) + u8 + hr
                                        (1:len + 2) */
		{
			// NCI2.0: HRx length always present for POLL A type. Not 0 if T1T, 0
			// otherwise
			CardPara->RF_Technology_Para.param.pa.hr_len = *p++;
			if (CardPara->RF_Technology_Para.param.pa.hr_len == NCI_T1T_HR_LEN)
			{
				CardPara->RF_Technology_Para.param.pa.hr[0] = *p++;
				CardPara->RF_Technology_Para.param.pa.hr[1] = *p;
			}
		}

		CardPara->uid_length = CardPara->RF_Technology_Para.param.pa.nfcid1_len;
		memcpy(CardPara->uid, CardPara->RF_Technology_Para.param.pa.nfcid1, CardPara->RF_Technology_Para.param.pa.nfcid1_len); //拷贝剩余数据
	}
	else if (CardPara->RF_Technology_Mode == NFC_DISCOVERY_TYPE_POLL_B)
	{
		/*
	    SENSB_RES Response length (n)   1 byte  Length of SENSB_RES Response (Byte 2 -
	    Byte 12 or 13)Available after Technology Detection
	    SENSB_RES Response Byte 2 - Byte 12 or 13   11 or 12 bytes  Defined in [DIGPROT]
	    Available after Technology Detection
	      */
		CardPara->RF_Technology_Para.mode					= NFC_DISCOVERY_TYPE_POLL_B;
		CardPara->RF_Technology_Para.param.pb.sensb_res_len = *p++;

		if (CardPara->RF_Technology_Para.param.pb.sensb_res_len > NCI_MAX_SENSB_RES_LEN)
		{
			CardPara->RF_Technology_Para.param.pb.sensb_res_len = NCI_MAX_SENSB_RES_LEN;
		}
		STREAM_TO_ARRAY(CardPara->RF_Technology_Para.param.pb.sensb_res, p, CardPara->RF_Technology_Para.param.pb.sensb_res_len);
		memcpy(CardPara->RF_Technology_Para.param.pb.nfcid0, CardPara->RF_Technology_Para.param.pb.sensb_res, NFC_NFCID0_MAX_LEN); //digital 协议中是从byte1开始的？

		CardPara->uid_length = NFC_NFCID0_MAX_LEN;
		memcpy(CardPara->uid, CardPara->RF_Technology_Para.param.pb.nfcid0, NFC_NFCID0_MAX_LEN); //拷贝剩余数据
	}

	return NO_ERR;
}

t_RetStatus Process_card_para_6105(CARD_PARA *CardPara, COMMON_NCI_PACKET *nci_recv_packet)
{
	uint8_t u8, *p;

	NFC_TRACE_DEBUG("%s start", __FUNCTION__);

	CardPara->RF_Discovery_ID		 = nci_recv_packet->PAYLOAD[0];
	CardPara->RF_Interface			 = (NCI_RF_INTERFACE_t)nci_recv_packet->PAYLOAD[1];
	CardPara->RF_Protocol			 = (NCI_RF_PROTOCOL_t)nci_recv_packet->PAYLOAD[2];
	CardPara->RF_Technology_Mode	 = nci_recv_packet->PAYLOAD[3];
	CardPara->Max_Payload_Size		 = nci_recv_packet->PAYLOAD[4];
	CardPara->Initial_Number_Credits = nci_recv_packet->PAYLOAD[5];
	CardPara->Length_Parameters		 = nci_recv_packet->PAYLOAD[6]; //09
	CardPara->Data_Exchange_Mode	 = nci_recv_packet->PAYLOAD[7 + CardPara->Length_Parameters];
	CardPara->Data_Transmit_Bit_Rate = nci_recv_packet->PAYLOAD[8 + CardPara->Length_Parameters];
	CardPara->Data_Receive_Bit_Rate	 = nci_recv_packet->PAYLOAD[9 + CardPara->Length_Parameters];
	CardPara->Length_Activation_Para = nci_recv_packet->PAYLOAD[10 + CardPara->Length_Parameters];
	p								 = &nci_recv_packet->PAYLOAD[7];
	if (CardPara->RF_Technology_Mode == NFC_DISCOVERY_TYPE_POLL_A)
	{
		//        00112233 4455 66 7788 99 0A0B0C0D 0E0F        11223344
		//6105 14 01808000 ff01 09 0400 04 cef31070 0108 00000000   (mifare V1.0协议)
		//6105 15 01808000 FF01 0A 0400 04 4E04F825 0108 0000000000 //mifare  card 协议
		//6105 16 01010100 ff01 0b 000c 04 45637700 00   02124c 00000000   (Tag1   V2.1协议)
		//6105 17 01010200 ff01 0c 4400 07 04053e32e33e81 0100  00000000   (Tag2   V1.0协议)
		/*
	    SENS_RES Response   2 bytes Defined in [DIGPROT] Available after Technology
	    Detection
	    NFCID1 length   1 byte  Length of NFCID1 Available after Collision Resolution
	    NFCID1  4, 7, or 10 bytes   Defined in [DIGPROT]Available after Collision
	    Resolution
	    SEL_RES Response    1 byte  Defined in [DIGPROT]Available after Collision
	    Resolution
	    HRx Length  1 Octets    Length of HRx Parameters collected from the response to
	    the T1T RID command.
	    HRx 0 or 2 Octets   If present, the first byte SHALL contain HR0 and the second
	    byte SHALL contain HR1 as defined in [DIGITAL].
	      */
		CardPara->RF_Technology_Para.mode = NFC_DISCOVERY_TYPE_POLL_A;
		STREAM_TO_ARRAY(CardPara->RF_Technology_Para.param.pa.sens_res, p, 2); //偏移7
		CardPara->RF_Technology_Para.param.pa.nfcid1_len = *p++;			   //uid长度   偏移9
		if (CardPara->RF_Technology_Para.param.pa.nfcid1_len > NCI_NFCID1_MAX_LEN)
		{
			CardPara->RF_Technology_Para.param.pa.nfcid1_len = NCI_NFCID1_MAX_LEN;
		}
		STREAM_TO_ARRAY(CardPara->RF_Technology_Para.param.pa.nfcid1, p, CardPara->RF_Technology_Para.param.pa.nfcid1_len);
		u8 = *p++;
		if (u8)
		{
			CardPara->RF_Technology_Para.param.pa.sel_rsp = *p++;
		}
		if (CardPara->Length_Parameters == (7 + CardPara->RF_Technology_Para.param.pa.nfcid1_len + u8)) /* 2(sens_res) + 1(len) +
                                        p_pa->nfcid1_len + 1(len) + u8 + hr
                                        (1:len + 2) */
		{
			// NCI2.0: HRx length always present for POLL A type. Not 0 if T1T, 0
			// otherwise
			CardPara->RF_Technology_Para.param.pa.hr_len = *p++;
			if (CardPara->RF_Technology_Para.param.pa.hr_len == NCI_T1T_HR_LEN)
			{
				CardPara->RF_Technology_Para.param.pa.hr[0] = *p++;
				CardPara->RF_Technology_Para.param.pa.hr[1] = *p;
			}
		}

		CardPara->uid_length = CardPara->RF_Technology_Para.param.pa.nfcid1_len;
		memcpy(CardPara->uid, CardPara->RF_Technology_Para.param.pa.nfcid1, CardPara->RF_Technology_Para.param.pa.nfcid1_len); //拷贝剩余数据
	}
	else if (CardPara->RF_Technology_Mode == NFC_DISCOVERY_TYPE_POLL_B)
	{
		/*
	    SENSB_RES Response length (n)   1 byte  Length of SENSB_RES Response (Byte 2 -
	    Byte 12 or 13)Available after Technology Detection
	    SENSB_RES Response Byte 2 - Byte 12 or 13   11 or 12 bytes  Defined in [DIGPROT]
	    Available after Technology Detection
	      */
		CardPara->RF_Technology_Para.mode					= NFC_DISCOVERY_TYPE_POLL_B;
		CardPara->RF_Technology_Para.param.pb.sensb_res_len = *p++;

		if (CardPara->RF_Technology_Para.param.pb.sensb_res_len > NCI_MAX_SENSB_RES_LEN)
		{
			CardPara->RF_Technology_Para.param.pb.sensb_res_len = NCI_MAX_SENSB_RES_LEN;
		}
		STREAM_TO_ARRAY(CardPara->RF_Technology_Para.param.pb.sensb_res, p, CardPara->RF_Technology_Para.param.pb.sensb_res_len);
		memcpy(CardPara->RF_Technology_Para.param.pb.nfcid0, CardPara->RF_Technology_Para.param.pb.sensb_res, NFC_NFCID0_MAX_LEN); //digital 协议中是从byte1开始的？

		CardPara->uid_length = NFC_NFCID0_MAX_LEN;
		memcpy(CardPara->uid, CardPara->RF_Technology_Para.param.pb.nfcid0, NFC_NFCID0_MAX_LEN); //拷贝剩余数据
	}

	return NO_ERR;
}
