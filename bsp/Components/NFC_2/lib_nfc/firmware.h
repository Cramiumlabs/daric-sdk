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
  

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PORT_FIRMWARE_H__
#define __PORT_FIRMWARE_H__
#include "port.h"


/**
* Firmware file structure
*/
#define LEN_FWDL_CHANGEPUBKEY_CMD               (7 + 128)
#define LEN_FWDL_CHANGETRANSTRANSKEY_CMD        (7 + 34)
#define LEN_FWDL_CHANGEAUXIKEY_CMD              (7 + 34)
#define LEN_FWDL_SETBANK_CMD                    (7 + 4)
#define LEN_FWDL_CHECKDATA_CMD                  (7 + 10)
#define LEN_FWDL_VALID_CMD                      (7 + 6)
#define LEN_FWDL_CHANGEPARA_SIGFLAG_CMD         (7)

#define OFFSET_FWDL_VERSION                     0
#define OFFSET_FWDL_DOWNLOADCMD_NUM             2
#define OFFSET_FWDL_DOWNLOADCMD_LEN             4
#define OFFSET_FWDL_IF_CHANGEPUBKEY             6
#define OFFSET_FWDL_IF_CHANGETRANSTRANSKEY      7
#define OFFSET_FWDL_IF_CHANGEAUXIKEY            8
#define OFFSET_FWDL_CHANGEPUBKEY_CMD            9
#define OFFSET_FWDL_CHANGETRANSTRANSKEY_CMD_CASE1     9 // change pub key not exist     
#define OFFSET_FWDL_CHANGETRANSTRANSKEY_CMD_CASE2     (9 + 7 + 128)  // change pub key exsit
#define OFFSET_FWDL_CHANGEAUXIKEY_CMD_CASE1     9  // change pub key and change transit key not exsit   
#define OFFSET_FWDL_CHANGEAUXIKEY_CMD_CASE2     (9 + 7 + 128)   // change pub key exsit   
#define OFFSET_FWDL_CHANGEAUXIKEY_CMD_CASE3     (9 + 7 + 34)   // change transit key exsit 
#define OFFSET_FWDL_CHANGEAUXIKEY_CMD_CASE4     (9 + 7 + 128 + 7 + 34)  // change pub key and change transit key exsit   

#define OFFSET_FWDL_CHANGEPARA_CMD_CASE1         9 //change pub key /change transit key and change auxikey not exsit 
#define OFFSET_FWDL_CHANGEPARA_CMD_CASE2         (9 + 7 + 128) //change pub key exsit 
#define OFFSET_FWDL_CHANGEPARA_CMD_CASE3         (9 + 7 + 34)   // change transit key or change auxikey exsit 
#define OFFSET_FWDL_CHANGEPARA_CMD_CASE4         (9 + 7 + 128 + 7 + 34)  // change pub key and change transit key or change auxikey exsit   
#define OFFSET_FWDL_CHANGEPARA_CMD_CASE5         (9 + 7 + 128 + 7 + 34 + 7 + 34) //change pub key & change transit key & change auxikey exsit 
#define OFFSET_FWDL_CHANGEPARA_CMD_CASE6         (9 + 7 + 34 + 7 + 34) //change change transit key and change auxikey exsit 

#define OFFSET_FWDL_SETBANK_CMD_CASE1           9 //change pub key /change transit key and change auxikey not exsit 
#define OFFSET_FWDL_SETBANK_CMD_CASE2           (9 + 7 + 128) //change pub key exsit 
#define OFFSET_FWDL_SETBANK_CMD_CASE3           (9 + 7 + 34)   // change transit key or change auxikey exsit 
#define OFFSET_FWDL_SETBANK_CMD_CASE4           (9 + 7 + 128 + 7 + 34)  // change pub key and change transit key or change auxikey exsit   
#define OFFSET_FWDL_SETBANK_CMD_CASE5           (9 + 7 + 128 + 7 + 34 + 7 + 34) //change pub key & change transit key & change auxikey exsit 
#define OFFSET_FWDL_SETBANK_CMD_CASE6           (9 + 7 + 34 + 7 + 34) //change change transit key and change auxikey exsit 

//with signature
#define OFFSET_FWDL_SETBANK_CMD_CASE7           (9 + 7) //change pub key /change transit key and change auxikey not exsit 
#define OFFSET_FWDL_SETBANK_CMD_CASE8           (9 + 7 + 128 + 7) //change pub key exsit 
#define OFFSET_FWDL_SETBANK_CMD_CASE9           (9 + 7 + 34 + 7)   // change transit key or change auxikey exsit 
#define OFFSET_FWDL_SETBANK_CMD_CASE10          (9 + 7 + 128 + 7 + 34 + 7)  // change pub key and change transit key or change auxikey exsit   
#define OFFSET_FWDL_SETBANK_CMD_CASE11          (9 + 7 + 128 + 7 + 34 + 7 + 34 + 7) //change pub key & change transit key & change auxikey exsit 
#define OFFSET_FWDL_SETBANK_CMD_CASE12          (9 + 7 + 34 + 7 + 34 + 7) //change change transit key and change auxikey exsit 

#define OFFSET_FWDL_DOWNLOAD_CMD_CASE1          (9 + 11)
#define OFFSET_FWDL_DOWNLOAD_CMD_CASE2          (9 + 7 + 128 + 11)
#define OFFSET_FWDL_DOWNLOAD_CMD_CASE3          (9 + 7 + 34 + 11) 
#define OFFSET_FWDL_DOWNLOAD_CMD_CASE4          (9 + 7 + 128 + 7 + 34 + 11)
#define OFFSET_FWDL_DOWNLOAD_CMD_CASE5          (9 + 7 + 128 + 7 + 34 + 7 + 34 + 11)
#define OFFSET_FWDL_DOWNLOAD_CMD_CASE6          (9 + 7 + 34 + 7 + 34 + 11)
extern uint16_t fwdl_getVersion(void);
#ifdef HED_NFCC_FIRMWARE_CHIP_VERSION
extern uint8_t* fwdl_getChipVersion(void);
#endif

extern PORT_Status_t fwdl_getCmd(uint8_t *cmd, uint16_t *len);

extern PORT_Status_t fwdl_getNextCmd(uint8_t *cmd, uint16_t *len);

extern PORT_Status_t fwdl_getChangeKeyCmd(uint8_t *cmd, uint16_t *len);


#endif /* __PORT_FIRMWARE_H__ */
