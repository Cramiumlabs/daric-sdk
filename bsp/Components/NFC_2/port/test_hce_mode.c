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

#include "test_hce_mode.h"


#define DH_NFCEEID					0x00
#define UICC1_NFCEEID				0x81

/* AID matching is allowed when the SELECT AID is longer */
#define ROUTE_QUAL_LONG			0x10
/* AID matching is allowed when the SELECT AID is shorter */
#define ROUTE_QUAL_SHORT		0x20 
/* AID is blocked in unsupported power mode */
#define ROUTE_QUAL_BLOCK		0x40 

#define ROUTE_PWR_STATE_ON	0x01


const uint8_t HCE_AID[] = {0x09, 0x08, 0xA0, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
const uint8_t RSP_SW_9000[] = {0x02, 0x90, 0x00};
uint8_t rspBuf[300];
uint8_t outputBuf[1000];



//void ce_callback(uint8_t event, uint8_t *p_data)
//{
//	NFC_TRACE_INFO("EVENT: ce_callback event: 0x%02X", event);
//	ce_hce_apdu_send_rsp((uint8_t*)RSP_SW_9000);
//}


void (ce_callback)(uint16_t data_length, uint8_t *p_data)
{
	// NFC_TRACE_INFO("EVENT: ce_callback event: 0x%d");
	uint8_t offset = 0;
	uint8_t len;	
	
	// uint8_t cla = *(p_data + offset);//00
	// offset++;
	uint8_t ins = *(p_data + 1);//01
	offset++;
	// uint8_t p1 = *(p_data + offset);//02
	// offset++;
	// uint8_t p2 = *(p_data + offset);//03
	// offset++;
	// uint8_t lc = *(p_data + offset);//04
	// offset++;


	switch(ins)
	{
		case 0xA4:
			*rspBuf = 2;
			*(rspBuf + 1) = 0x90;
			*(rspBuf + 2) = 0;
			break;
		
	  case 0xD6://д������				
		  outputBuf[0] = *(p_data + 4);//Lc
		  len = outputBuf[0];
		  if (outputBuf[0] < 0x80)
			{
				memcpy(outputBuf+1,p_data+5,outputBuf[0]);  //L+V
				
			}else
			{
				outputBuf[0] = 0x81;
				outputBuf[1] = len;
				memcpy(outputBuf+2,p_data+5,outputBuf[1]);			//0x81+L+V			
			}		
			
		  *rspBuf = 2;
			*(rspBuf + 1) = 0x90;
			*(rspBuf + 2) = 0x00;		 
			
			break;
			
		case 0xB0://����д�������			
		  
		 if(outputBuf[0] != 0x81)
			{
				if(outputBuf[0] < 0x7E)//L+V
				{
					*rspBuf = outputBuf[0]+2;
					 memcpy(rspBuf+1,outputBuf+1,outputBuf[0]);	
					*(rspBuf + outputBuf[0]+1) = 0x90;
					*(rspBuf + outputBuf[0]+2) = 0x00;
				}else  //0x81+L+V
				{
					* rspBuf = 0x81;
					* (rspBuf + 1)= outputBuf[0]+2;
					memcpy(rspBuf+2,outputBuf+1,outputBuf[0]);	
					*(rspBuf + outputBuf[0]+2) = 0x90;
					*(rspBuf + outputBuf[0]+3) = 0x00;
				}
				
			}else
			{
				if(outputBuf[1] < 0xFE)  //0x81+00+L2+V
				{
					* rspBuf = outputBuf[0];
					*(rspBuf+1) = outputBuf[1]+2;
					memcpy(rspBuf+2,outputBuf+2,outputBuf[1]);	
					*(rspBuf + outputBuf[1]+2) = 0x90;
					*(rspBuf + outputBuf[1]+3) = 0x00;
					
				}else //0x82+L1+L2+V
				{
					*rspBuf = 0x82;					
					*(rspBuf+1) = (outputBuf[1]+2)>>8;
					*(rspBuf+2) = (outputBuf[1]+2);
					 memcpy(rspBuf+3,outputBuf+2,outputBuf[1]);	
					*(rspBuf + outputBuf[1]+3) = 0x90;
					*(rspBuf + outputBuf[1]+4) = 0x00;					
				}				
			}
			
			break;
			
		default:
			*rspBuf = 2;
			*(rspBuf + 1) = 0x6D;
			*(rspBuf + 2) = 0x00;
			break;
	}
	
	//NFCC_HceSendData(rspBuf, sizeof(rspBuf));
}


void test_hce_mode_start(void)
{
	//nfc_hce_add_aid_route(DH_NFCEEID, (uint8_t*)HCE_AID, ROUTE_PWR_STATE_ON, ROUTE_QUAL_BLOCK, &ce_callback);
}


void test_hce_mode(void)
{

	test_hce_mode_start();

}




