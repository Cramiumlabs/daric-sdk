#ifndef FIRMWARE_UPDATE_H_
#define FIRMWARE_UPDATE_H_

#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "trace.h"
#include "nci.h"
#include "port.h"
#include "nfc_timer.h"

#include "firmware.h"

/*	NFCC IO Control	*/
#define PIN_VEN_NUM 0
#define PIN_DWL_REQ_NUM 1
#define PIN_LEVEL_LOW 0
#define PIN_LEVEL_HIGH 1

#define FW_UP_NCI_VERSION_2_0 0x20
#define FW_UP_NCI_VERSION_1_0 0x10

#define FW_UP_STRATEGY_UPPER_VERSION 1
#define FW_UP_STRATEGY_DIFFERENT_VERSION 2
// #define FW_UP_STRATEGY_ALWAYS 3

/*******configurable item start********/
#define FW_UP_CONF_MAX_UP_CNT 10 //max update count for nfcc firmware
#define FW_UP_NCI_VERSION FW_UP_NCI_VERSION_1_0
/*******configurable item end********/

/*******error codes start********/
#define FW_UD_SUCCESS 0x0000
#define FW_UD_SUCCESS_CHANGE_PUB_KEY 0x0001
#define FW_UD_SUCCESS_FINAL_FRAME 0x0002
#define FW_UD_ERR_INSUFFICIENT_SPACE 0x0003
#define FW_UD_ERR_TIME_OUT 0x0009	//NFCSTATUS_RF_TIMEOUT    //0x0009
#define FW_UD_ERR_FAIL 0x00ff		//NFCSTATUS_FAILED        //0x00ff
#define FW_UP_ERR_VALID_PARA 0x09ff //CID_NFC_DNLD << 8 | NFCSTATUS_FAILED

#define FW_UD_NO_NEED_SUCCESS 0x000A

#define FW_UP_ERR_SIGNATURE_ERROR 0x6400 //PH_DL_STATUS_SIGNATURE_ERROR //0x6400
/*******error codes end********/

/*******lable value start********/
#define FW_UP_LABLE_UP_START 0xA5
#define FW_UP_LABLE_UP_DONE 0x5A

#define FW_UP_PIN_VEN 0
#define FW_UP_PIN_DWL_REQ 1
#define FW_UP_PIN_LEVEL_LOW 0
#define FW_UP_PIN_LEVEL_HIGH 1

#define FW_UP_FRAME_HEADER_LEN 3 //PHDNLDNFC_FRAME_HEADER_LEN//3
#define FW_UP_MAX_CMD_SIZE (0x1047 + 7 + 3 + 2)
#define FW_UP_MAX_RECV_SIZE 258	 //as present, length of command INS=48 is most long, is 0x9D
#define FW_UP_DL_Para_Valid 0x46 //PH_DL_Para_Valid  //      0x46

#define FW_UP_MAX_RECV_OVERTIME_MS 2000 //2000ms

#define FW_UP_CUR_VERSION_ERR 0xFFFF
/*******lable value end********/

uint16_t foce_try_fwdl(void); //强制更新
uint16_t check_try_fwdl(void);

#endif
