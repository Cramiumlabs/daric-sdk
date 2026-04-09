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
#include "firmware.h"
#include "nfc_stage.h"

//#pragma	diag_suppress 170
//const uint8_t newFwFile[] = NFCC_FIRMWARE_BIN;
const uint8_t newFwFile[] = {0x00, 0x00};

static uint32_t fwdl_offset;
static uint8_t dlcmd_index;
static uint32_t totalOffset;

extern uint8_t port_hardware_flashReadU8(uint32_t dest_addr);
/*******************************************************************************
  **
  ** Function         fwdl_getVersion
  **
  ** Description      get the version of the new firmware 
  **                  
  ** Parameters       
  ** Returns          firmware version
  **
  *******************************************************************************/

uint16_t fwdl_getVersion(void)
{
	return port_hardware_flashReadU8((uint32_t)newFwFile) << 8 | port_hardware_flashReadU8((uint32_t)newFwFile + 1);
}

/*******************************************************************************
  **
  ** Function         fwdl_getChipVersion
  **
  ** Description      get the chip version of the new firmware 
  **                  
  ** Parameters       
  ** Returns          chip version
  **
  *******************************************************************************/
#ifdef HED_NFCC_FIRMWARE_CHIP_VERSION
uint8_t *fwdl_getChipVersion(void)
{
	uint8_t* fwFile;
	uint16_t downloadCmdLen;
	uint8_t downloadCmdNum1, downloadCmdNum2;
	uint8_t isPubKeyExsit, isTransKeyExsit, isAuxiKeyExsit;
	uint32_t fw_file_total_size_including_chipversion = 0;
	uint32_t down_last_cmd_offset = 0;
	uint16_t last_len_inback1 = 0;
	uint16_t last_len_inback2 = 0;
		
	//if version equals 0,failed
	if (fwdl_getVersion() == 0)
	{
		return NULL;
	}
	
	fwFile = (uint8_t *)newFwFile;
	
	downloadCmdLen = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_LEN) << 8 | port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_LEN + 1);	

	//initialize the offset
	down_last_cmd_offset = OFFSET_FWDL_CHANGEPUBKEY_CMD;
	totalOffset =0;
	downloadCmdNum1 = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_NUM);
	downloadCmdNum2 = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_NUM + 1);
	
	if (downloadCmdNum2 == 0)
	{
		totalOffset = OFFSET_FWDL_CHANGEPUBKEY_CMD + (downloadCmdLen * downloadCmdNum1) + LEN_FWDL_SETBANK_CMD + LEN_FWDL_CHECKDATA_CMD;//fix total len
	}
	else
	{		
		totalOffset = OFFSET_FWDL_CHANGEPUBKEY_CMD + (downloadCmdLen * downloadCmdNum1) + (downloadCmdLen * downloadCmdNum2) + (LEN_FWDL_SETBANK_CMD * 2) + (LEN_FWDL_CHECKDATA_CMD * 2);//fix total len
	}

	//add change para len to total length
	if (downloadCmdLen == 0x1047)
	{
		totalOffset += LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
	}
	
		
	isPubKeyExsit = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEPUBKEY);
	isTransKeyExsit = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGETRANSTRANSKEY);
	isAuxiKeyExsit = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEAUXIKEY);
	
	
	//check data or valid cmd
	if (isPubKeyExsit)
	{
		totalOffset += LEN_FWDL_CHANGEPUBKEY_CMD;
	}
	
	if (isTransKeyExsit)
	{
		totalOffset += LEN_FWDL_CHANGETRANSTRANSKEY_CMD;
	}
	
	if (isAuxiKeyExsit)
	{
		totalOffset += LEN_FWDL_CHANGEAUXIKEY_CMD;
	}	
	if (port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEPUBKEY))
	{
		down_last_cmd_offset += LEN_FWDL_CHANGEPUBKEY_CMD;
	}
	

	
	if (port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGETRANSTRANSKEY))
	{
			down_last_cmd_offset += LEN_FWDL_CHANGETRANSTRANSKEY_CMD;

	}
	
	if (port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEAUXIKEY))
	{
			down_last_cmd_offset += LEN_FWDL_CHANGEAUXIKEY_CMD;
	}
	
	//change para is the first cmd
	if (downloadCmdLen == 0x1047)
	{
			down_last_cmd_offset += LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
	}
	
	
	//setBank is the first cmd

	down_last_cmd_offset += LEN_FWDL_SETBANK_CMD;

	
	down_last_cmd_offset += (downloadCmdLen * (downloadCmdNum1 - 1));
		
	last_len_inback1 = ((port_hardware_flashReadU8((uint32_t)fwFile + down_last_cmd_offset + 5) << 8) |(port_hardware_flashReadU8((uint32_t)fwFile + down_last_cmd_offset + 6))) + 7;

	totalOffset -= downloadCmdLen;
	totalOffset += last_len_inback1;
	if (downloadCmdNum2 != 0){
		down_last_cmd_offset += last_len_inback1;
		down_last_cmd_offset += LEN_FWDL_SETBANK_CMD;
		down_last_cmd_offset += (downloadCmdLen * (downloadCmdNum2 - 1));
		last_len_inback2 = ((port_hardware_flashReadU8((uint32_t)fwFile + down_last_cmd_offset + 5) << 8) |(port_hardware_flashReadU8((uint32_t)fwFile + down_last_cmd_offset + 6))) + 7;
	
		totalOffset -= downloadCmdLen;
		totalOffset += last_len_inback2;
	}
		
	fw_file_total_size_including_chipversion = sizeof(newFwFile);
	if (fw_file_total_size_including_chipversion > totalOffset + 1){
		return (uint8_t*)&(newFwFile[totalOffset]);//the first two bytes of the last 10 bytes is chip_type
	}
	else {
		return NULL;//the array donot contain the byte of the chip version
	}
}
#endif

/*******************************************************************************
  **
  ** Function         fwdl_getCmd
  **
  ** Description      get the first cmd of the specific firmware 
  **                  
  ** Parameters       cmd: buffer used to get the cmd
  **                  len:length of the cmd buffer   
  ** Returns          PORT_STATUS_SUCCESS, PORT_STATUS_FAILED, PORT_STATUS_INSUFFICIENT_SPACE 
  **
  *******************************************************************************/
PORT_Status_t fwdl_getCmd(uint8_t *cmd, uint16_t *len)
{
	uint8_t* fwFile;
	uint16_t downloadCmdLen;
	uint8_t downloadCmdNum1, downloadCmdNum2;
	uint8_t isPubKeyExsit, isTransKeyExsit, isAuxiKeyExsit;

	//if version equals 0,failed
	if (fwdl_getVersion() == 0)
	{
		return PORT_STATUS_FAILED;
	}
	
	//cmd should not be null
	if (cmd == NULL)
	{
		return PORT_STATUS_FAILED;
	}
	
	fwFile = (uint8_t *)newFwFile;
	
	downloadCmdLen = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_LEN) << 8 | port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_LEN + 1);	

	//initialize the offset
	fwdl_offset = OFFSET_FWDL_CHANGEPUBKEY_CMD;
	dlcmd_index = 0;
	totalOffset =0;
	downloadCmdNum1 = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_NUM);
	downloadCmdNum2 = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_NUM + 1);
	
	if (downloadCmdNum2 == 0)
	{
		totalOffset = OFFSET_FWDL_CHANGEPUBKEY_CMD + (downloadCmdLen * downloadCmdNum1) + LEN_FWDL_SETBANK_CMD + LEN_FWDL_CHECKDATA_CMD;//fix total len
	}
	else
	{
		totalOffset = OFFSET_FWDL_CHANGEPUBKEY_CMD + (downloadCmdLen * downloadCmdNum1) + (downloadCmdLen * downloadCmdNum2) + (LEN_FWDL_SETBANK_CMD * 2) + (LEN_FWDL_CHECKDATA_CMD * 2);//fix total len
	}

	//add change para len to total length
	if (downloadCmdLen == 0x1047)
	{
		totalOffset += LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
	}
	
		
	isPubKeyExsit = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEPUBKEY);
	isTransKeyExsit = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGETRANSTRANSKEY);
	isAuxiKeyExsit = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEAUXIKEY);
	
	
	//check data or valid cmd
	if (isPubKeyExsit)
	{
		totalOffset += LEN_FWDL_CHANGEPUBKEY_CMD;
	}
	
	if (isTransKeyExsit)
	{
		totalOffset += LEN_FWDL_CHANGETRANSTRANSKEY_CMD;
	}
	
	if (isAuxiKeyExsit)
	{
		totalOffset += LEN_FWDL_CHANGEAUXIKEY_CMD;
	}	
	if (port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEPUBKEY))
	{
		fwdl_offset += LEN_FWDL_CHANGEPUBKEY_CMD;
	}
	

	
	if (port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGETRANSTRANSKEY))
	{
		//change Transkey is the first cmd
		if (*len < LEN_FWDL_CHANGETRANSTRANSKEY_CMD)
		{
			return PORT_STATUS_INSUFFICIENT_SPACE;
		}
		else 
		{
			memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHANGETRANSTRANSKEY_CMD);
			fwdl_offset += LEN_FWDL_CHANGETRANSTRANSKEY_CMD;
			*len = LEN_FWDL_CHANGETRANSTRANSKEY_CMD;
			return PORT_STATUS_SUCCESS;
		}
	}
	
	if (port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEAUXIKEY))
	{
		//change AuxiKey is the first cmd
		if (*len < LEN_FWDL_CHANGEAUXIKEY_CMD)
		{
			return PORT_STATUS_INSUFFICIENT_SPACE;
		}
		else 
		{
			memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHANGEAUXIKEY_CMD);
			fwdl_offset += LEN_FWDL_CHANGEAUXIKEY_CMD;
			*len = LEN_FWDL_CHANGEAUXIKEY_CMD;
			return PORT_STATUS_SUCCESS;
		}
	}
	
	//change para is the first cmd
	if (downloadCmdLen == 0x1047)
	{
		if (*len < LEN_FWDL_CHANGEPARA_SIGFLAG_CMD)
		{
			return PORT_STATUS_INSUFFICIENT_SPACE;
		}
		else 
		{
			memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHANGEPARA_SIGFLAG_CMD);
			fwdl_offset += LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
			*len = LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
			return PORT_STATUS_SUCCESS;	
		}
	
	}
	
	
	//setBank is the first cmd
	if (*len < LEN_FWDL_CHANGEPARA_SIGFLAG_CMD)
	{
		return PORT_STATUS_INSUFFICIENT_SPACE;
	}
	else
	{
		memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
		fwdl_offset += LEN_FWDL_SETBANK_CMD;
		*len = LEN_FWDL_SETBANK_CMD;
		return PORT_STATUS_SUCCESS;
	}

}

/*******************************************************************************
  **
  ** Function         fwdl_getNextCmd
  **
  ** Description      get the next cmd of the specific firmware 
  **                  
  ** Parameters       cmd: buffer used to get the cmd
  **                  len:length of the cmd buffer
  **                  isNew :true if get cmd from the new firmware, false if get cmd from the old firmare
  ** Returns          PORT_STATUS_SUCCESS, PORT_STATUS_FAILED, PORT_STATUS_SUCCESS_EXT, PORT_STATUS_INSUFFICIENT_SPACE 
  **
  *******************************************************************************/
PORT_Status_t fwdl_getNextCmd(uint8_t *cmd, uint16_t *len)
{
	uint8_t* fwFile;
	uint16_t downloadCmdLen;
	uint8_t downloadCmdNum1, downloadCmdNum2;
	uint8_t isTransKeyExsit, isAuxiKeyExsit;

	//if version equals 0,failed
	if (fwdl_getVersion() == 0)
	{
		return PORT_STATUS_FAILED;
	}
	
	//cmd should not be null
	if (cmd == NULL)
	{
		return PORT_STATUS_FAILED;
	}
	
	fwFile = (uint8_t *)newFwFile;
	
	downloadCmdLen = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_LEN) << 8 | port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_LEN + 1);
	downloadCmdNum1 = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_NUM);
	downloadCmdNum2 = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_DOWNLOADCMD_NUM + 1);	

	isTransKeyExsit = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGETRANSTRANSKEY);
	isAuxiKeyExsit = port_hardware_flashReadU8((uint32_t)fwFile + OFFSET_FWDL_IF_CHANGEAUXIKEY);
	
	if (isTransKeyExsit)
	{
		//change Transkey is the first cmd
		if (isAuxiKeyExsit)
		{

			if ((fwdl_offset == OFFSET_FWDL_CHANGEAUXIKEY_CMD_CASE3)|| (fwdl_offset == OFFSET_FWDL_CHANGEAUXIKEY_CMD_CASE4))
			{
				//change AuxiKey is the second cmd
				if (*len < LEN_FWDL_CHANGEAUXIKEY_CMD)
				{
					return PORT_STATUS_INSUFFICIENT_SPACE;
				}
				else 
				{
					memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHANGEAUXIKEY_CMD);
					fwdl_offset += LEN_FWDL_CHANGEAUXIKEY_CMD;
					*len = LEN_FWDL_CHANGEAUXIKEY_CMD;
					return PORT_STATUS_SUCCESS;
				}
			}
			
			
			if (downloadCmdLen == 0x1047)
			{
				//change para is the third cmd
				if ((fwdl_offset == OFFSET_FWDL_CHANGEPARA_CMD_CASE5) || (fwdl_offset == OFFSET_FWDL_CHANGEPARA_CMD_CASE6))
				{
					if (*len < LEN_FWDL_CHANGEPARA_SIGFLAG_CMD)
					{
						return PORT_STATUS_INSUFFICIENT_SPACE;
					}
					else 
					{
						memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHANGEPARA_SIGFLAG_CMD);
						fwdl_offset += LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
						*len = LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
						return PORT_STATUS_SUCCESS;	
					}
				}	
				else if ((fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE11) || (fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE12))
				{
					//setBank is the fourth cmd
					if (*len < LEN_FWDL_SETBANK_CMD)
					{
						return PORT_STATUS_INSUFFICIENT_SPACE;
					}
					else 
					{
						memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
						fwdl_offset += LEN_FWDL_SETBANK_CMD;
						*len = LEN_FWDL_SETBANK_CMD;
						return PORT_STATUS_SUCCESS;
					}					
					
				}
			}
			else if ((fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE5) || (fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE6))
			{
				//setBank is the third cmd
				if (*len < LEN_FWDL_SETBANK_CMD)
				{
					return PORT_STATUS_INSUFFICIENT_SPACE;
				}
				else 
				{
					memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
					fwdl_offset += LEN_FWDL_SETBANK_CMD;
					*len = LEN_FWDL_SETBANK_CMD;
					return PORT_STATUS_SUCCESS;
				}				
			}

		}	
		else
		{
			if (downloadCmdLen == 0x1047)
			{
				//change para is the second cmd
				if ((fwdl_offset == OFFSET_FWDL_CHANGEPARA_CMD_CASE3)|| (fwdl_offset == OFFSET_FWDL_CHANGEPARA_CMD_CASE4))
				{
					if (*len < LEN_FWDL_CHANGEPARA_SIGFLAG_CMD)
					{
						return PORT_STATUS_INSUFFICIENT_SPACE;
					}
					else 
					{
						memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHANGEPARA_SIGFLAG_CMD);
						fwdl_offset += LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
						*len = LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
						return PORT_STATUS_SUCCESS;	
					}

				}
				else if ((fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE9)|| (fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE10))
				{
					//setBank is the third cmd
					if (*len < LEN_FWDL_SETBANK_CMD)
					{
						return PORT_STATUS_INSUFFICIENT_SPACE;
					}
					else 
					{
						memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
						fwdl_offset += LEN_FWDL_SETBANK_CMD;
						*len = LEN_FWDL_SETBANK_CMD;
						return PORT_STATUS_SUCCESS;
					}	
				}
			}
			else if ((fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE3)|| (fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE4))
			{
				//setBank is the second cmd
				if (*len < LEN_FWDL_SETBANK_CMD)
				{
					return PORT_STATUS_INSUFFICIENT_SPACE;
				}
				else 
				{
					memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
					fwdl_offset += LEN_FWDL_SETBANK_CMD;
					*len = LEN_FWDL_SETBANK_CMD;
					return PORT_STATUS_SUCCESS;
				}	
			}

		}
		
	}
	
	if ((!isTransKeyExsit) && (isAuxiKeyExsit))
	{
		if (downloadCmdLen == 0x1047)
		{
			//change para is the second cmd
			if ((fwdl_offset == OFFSET_FWDL_CHANGEPARA_CMD_CASE3) || (fwdl_offset == OFFSET_FWDL_CHANGEPARA_CMD_CASE4))
			{
				if (*len < LEN_FWDL_CHANGEPARA_SIGFLAG_CMD)
				{
					return PORT_STATUS_INSUFFICIENT_SPACE;
				}
				else 
				{
					memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHANGEPARA_SIGFLAG_CMD);
					fwdl_offset += LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
					*len = LEN_FWDL_CHANGEPARA_SIGFLAG_CMD;
					return PORT_STATUS_SUCCESS;	
				}

			}			
			else if ((fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE9) || (fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE10))
			{
				if (*len < LEN_FWDL_SETBANK_CMD)
				{
					return PORT_STATUS_INSUFFICIENT_SPACE;
				}
				memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
				fwdl_offset += LEN_FWDL_SETBANK_CMD;
				*len = LEN_FWDL_SETBANK_CMD;
				return PORT_STATUS_SUCCESS;
			}
		}
		else
		{
			if ((fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE3) || (fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE4))
			{
				if (*len < LEN_FWDL_SETBANK_CMD)
				{
					return PORT_STATUS_INSUFFICIENT_SPACE;
				}
				memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
				fwdl_offset += LEN_FWDL_SETBANK_CMD;
				*len = LEN_FWDL_SETBANK_CMD;
				return PORT_STATUS_SUCCESS;
							
			}
		}

	}

	if (downloadCmdLen == 0x1047)
	{
		//change para is the first cmd, set bank is the second cmd
		if ((fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE7) || (fwdl_offset == OFFSET_FWDL_SETBANK_CMD_CASE8))
		{
			if (*len < LEN_FWDL_SETBANK_CMD)
			{
				return PORT_STATUS_INSUFFICIENT_SPACE;
			}
			memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
			fwdl_offset += LEN_FWDL_SETBANK_CMD;
			*len = LEN_FWDL_SETBANK_CMD;
			return PORT_STATUS_SUCCESS;
						
		}
	}
	
	if (((dlcmd_index < (downloadCmdNum1 - 1))) || (((dlcmd_index > downloadCmdNum1) && (downloadCmdNum2 != 0) && (dlcmd_index < (downloadCmdNum1 + downloadCmdNum2)))))
	{
		if (*len < downloadCmdLen)
		{
			return PORT_STATUS_INSUFFICIENT_SPACE;
		}
		else
		{
			//download is the next cmd
			memcpy(cmd, fwFile + fwdl_offset, downloadCmdLen);
			fwdl_offset += downloadCmdLen;
			*len = downloadCmdLen;
			dlcmd_index++;
			return PORT_STATUS_SUCCESS;	
		}

	}
	
	if (((dlcmd_index > downloadCmdNum1) && (downloadCmdNum2 != 0) && (dlcmd_index == (downloadCmdNum1 + downloadCmdNum2))) || ((dlcmd_index == (downloadCmdNum1 - 1))))
	{
		if (*len < downloadCmdLen)
		{
			return PORT_STATUS_INSUFFICIENT_SPACE;
		}
		else
		{
			uint16_t tmplen = ((port_hardware_flashReadU8((uint32_t)fwFile + fwdl_offset + 5) << 8) |(port_hardware_flashReadU8((uint32_t)fwFile + fwdl_offset + 6))) + 7;
			//download is the next cmd, this cmd len may be 1040 or 1000
			memcpy(cmd, fwFile + fwdl_offset, tmplen);
			fwdl_offset += tmplen;
			*len = tmplen;
			dlcmd_index++;
			totalOffset = totalOffset - downloadCmdLen + tmplen;
			return PORT_STATUS_SUCCESS;	
		}		
	}
	
	if (((dlcmd_index == downloadCmdNum1) && (downloadCmdNum2 == 0)) || ((dlcmd_index == (downloadCmdNum1 + downloadCmdNum2 + 1)) && (downloadCmdNum2 != 0)))
	{


		if ((fwdl_offset < totalOffset) && (downloadCmdLen == 0X1007))
		{
			if (*len < LEN_FWDL_CHECKDATA_CMD)
			{
				return PORT_STATUS_INSUFFICIENT_SPACE;
			}
			
			memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHECKDATA_CMD);
			fwdl_offset += LEN_FWDL_CHECKDATA_CMD;
			*len = LEN_FWDL_CHECKDATA_CMD;

			if (fwdl_offset == totalOffset)//Local m1 data keep, no valid cmd
			{
				return PORT_STATUS_SUCCESS_EXT;// PORT_STATUS_NO_CMD;	
			}
			else
			{
				return PORT_STATUS_SUCCESS;
			}				
			
		}
		else	if ((fwdl_offset < totalOffset) && (downloadCmdLen == 0X1047))
		{
			if (*len < LEN_FWDL_CHECKDATA_CMD)
			{
				return PORT_STATUS_INSUFFICIENT_SPACE;
			}
			
			memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_CHECKDATA_CMD);
			fwdl_offset += LEN_FWDL_CHECKDATA_CMD;
			*len = LEN_FWDL_CHECKDATA_CMD;
			if (fwdl_offset == totalOffset)//Local m1 data keep, no valid cmd
			{
				return PORT_STATUS_SUCCESS_EXT;// PORT_STATUS_NO_CMD;	
			}
			else//add by haojm
			{
				return PORT_STATUS_SUCCESS;
			}				

		}
		else if (fwdl_offset == totalOffset)
		{
			if (*len < LEN_FWDL_VALID_CMD)
			{
				return PORT_STATUS_INSUFFICIENT_SPACE;
			}
			
			memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_VALID_CMD);
			fwdl_offset += LEN_FWDL_VALID_CMD;
			*len = LEN_FWDL_VALID_CMD;
			return PORT_STATUS_SUCCESS_EXT;//PORT_STATUS_NO_CMD;	
		}
	}

	//data segment2 exist
	if ((dlcmd_index == downloadCmdNum1) && (downloadCmdNum2 != 0))
	{
		//second set bank cmd
		if (*len < LEN_FWDL_SETBANK_CMD)
		{
			return PORT_STATUS_INSUFFICIENT_SPACE;
		}
		
		memcpy(cmd, fwFile + fwdl_offset, LEN_FWDL_SETBANK_CMD);
		fwdl_offset += LEN_FWDL_SETBANK_CMD;
		*len = LEN_FWDL_SETBANK_CMD;
		dlcmd_index++;
		return PORT_STATUS_SUCCESS;
	}	
	
	return PORT_STATUS_FAILED;
}
/*******************************************************************************
  **
  ** Function         fwdl_getChangeKeyCmd
  **
  ** Description      get the change key cmd 
  **                  
  ** Parameters       cmd: buffer used to get the cmd
  **                  len:length of the cmd buffer
  ** Returns          PORT_STATUS_SUCCESS, PORT_STATUS_FAILED, PORT_STATUS_INSUFFICIENT_SPACE 
  **
  *******************************************************************************/
PORT_Status_t fwdl_getChangeKeyCmd(uint8_t *cmd, uint16_t *len)
{
	//cmd should not be null
	if (cmd == NULL)
	{
		return PORT_STATUS_FAILED;
	}
	
	if (*len < LEN_FWDL_CHANGEPUBKEY_CMD)
	{
		return PORT_STATUS_INSUFFICIENT_SPACE;
	}
	
	if (port_hardware_flashReadU8((uint32_t)newFwFile + OFFSET_FWDL_IF_CHANGEPUBKEY) == FALSE)
	{
		return PORT_STATUS_FAILED;
	}

	memcpy(cmd, newFwFile + OFFSET_FWDL_CHANGEPUBKEY_CMD, LEN_FWDL_CHANGEPUBKEY_CMD);

	*len = LEN_FWDL_CHANGEPUBKEY_CMD;
	return PORT_STATUS_SUCCESS;
}
