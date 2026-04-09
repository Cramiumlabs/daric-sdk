#include "firmware_update.h"
#include "nfc_config.h"
#include "nci.h"
#include "nfc_drv.h"
#include "port_hardware.h"

/********************FSM MACRO--start************************/
#define FW_UP_FSM_RESET 0
#define FW_UP_FSM_CHECK 1
#define FW_UP_FSM_DATA 2
#define FW_UP_FSM_SUCCESS 3
#define FW_UP_FSM_FAIL 4
/********************FSM MACRO--end************************/

/********************struct used in this file--start************************/
typedef enum
{
	phTmlNfc_e_Invalid = 0,
	phTmlNfc_e_ResetDevice,
	phTmlNfc_e_EnableDownloadMode, /* Do the hardware setting to enter into download mode */
	phTmlNfc_e_EnableNormalMode,   /* Hardware setting for normal mode of operation */
	phTmlNfc_e_PowerReset,
	phTmlNfc_e_SetFwDownloadHdrSize,
} phTmlNfc_ControlCode_t; /* Control code for IOCTL call */

typedef struct
{
	uint8_t is_update_pub_key;
	uint8_t fw_up_fsm;
	uint16_t timer_id;
	uint16_t max_cmd_size;
	uint16_t cmd_len;
	uint8_t *cmd_buff;
	uint16_t max_recv_size;
	uint16_t recv_len;
	uint8_t *recv_buff;
	uint8_t valid_cmd_needed;
	uint8_t is_first_fw_wr;
	uint8_t is_final_frame;
	uint16_t download_frame_cnt;
} firmware_update_local_t;
/********************struct used in this file--end************************/

/********************static variable--start************************/

#if 0 //C98 语法不支持如此初始化
static firmware_update_local_t s_firmware_update_local = {
	.is_update_pub_key = 0,
	.fw_up_fsm		   = FW_UP_FSM_RESET,
	.max_cmd_size	   = FW_UP_MAX_CMD_SIZE,
	.max_recv_size	   = FW_UP_MAX_RECV_SIZE,
	.is_first_fw_wr	   = 1,
	.valid_cmd_needed  = 0,
	.is_final_frame	   = 0};
#else
static firmware_update_local_t s_firmware_update_local;
static void fwdl_data_init(void)
{
	s_firmware_update_local.is_update_pub_key = 0;
	s_firmware_update_local.fw_up_fsm		  = FW_UP_FSM_RESET;
	s_firmware_update_local.max_cmd_size	  = FW_UP_MAX_CMD_SIZE;
	s_firmware_update_local.max_recv_size	  = FW_UP_MAX_RECV_SIZE;
	s_firmware_update_local.is_first_fw_wr	  = 1;
	s_firmware_update_local.valid_cmd_needed  = 0;
	s_firmware_update_local.is_final_frame	  = 0;
}
#endif
/********************static variable--end************************/

/********************static function--start************************/
static uint16_t s_check_is_update_requred(void);
static uint16_t s_firmware_update_loop(void);
static uint16_t s_set_fwdl_flag(uint8_t fw_up_curr_step);
static uint16_t s_get_fwdl_flag(uint8_t *fw_up_curr_step);
static uint16_t s_firmware_update_download(void);
static uint16_t s_set_nfcc_ioctl(phTmlNfc_ControlCode_t eControlCode);
static uint16_t s_firmware_update_download_reset(void);
static uint16_t s_firmware_update_download_check(void);
static uint16_t s_firmware_update_download_fw_data(void);
static uint16_t s_check_is_fw_version_required(uint16_t cur_nfcc_ver, uint16_t up_fw_ver);
static uint16_t s_get_fw_up_version(void);
static uint16_t s_build_frame(uint8_t *cmd, uint16_t cmd_len, bool need_cpy_cmd);
static uint16_t s_fw_up_dwld_withtime(uint16_t overtime_ms);
static uint16_t s_fw_up_get_fw_cmd(uint8_t *temp_cmd_tb, uint16_t *frame_cmd_len);

/******************************need to call extern module, as below **********************************/
static void s_port_gpio_control(uint8_t pin, uint8_t level);
static void s_port_timer_wait_msec(uint16_t msec);
static uint16_t s_fw_up_send_string(uint8_t *cmd_buf, uint16_t cmd_len, uint32_t timeout, uint8_t *recv_reply, uint16_t *recv_len);
static uint16_t s_port_storage_getFlag(uint8_t *value);
static uint16_t s_port_storage_setFlag(uint8_t flag, uint8_t value);
/********************static function--end************************/

uint8_t g_fwdl_test_flag   = 0;
uint8_t g_fwdl_test_status = 0xff; //返回值 1 正在下载 2 下载失败 0 下载成功

extern unsigned char g_nfcc_fwdl_flag;
uint16_t foce_try_fwdl(void)
{
	NFC_TRACE_WARNING(" enter");
	uint16_t ret_firmware_success = FW_UD_ERR_FAIL;
//	uint16_t is_update_requred	  = 0;
//	g_nfcc_fwdl_flag			  = 1;

	fwdl_data_init();

	s_firmware_update_local.cmd_buff = (uint8_t *)port_mem_getMem(s_firmware_update_local.max_cmd_size);
	if (s_firmware_update_local.cmd_buff == NULL)
	{
		NFC_TRACE_ERROR("func:%s; line:%d,malloc cmd_buff NULL ERROR", __FUNCTION__, __LINE__);
		return ret_firmware_success;
	}
	s_firmware_update_local.recv_buff = (uint8_t *)port_mem_getMem(s_firmware_update_local.max_recv_size);
	if (s_firmware_update_local.recv_buff == NULL)
	{
		port_mem_freeMem(s_firmware_update_local.cmd_buff);
		s_firmware_update_local.cmd_buff = NULL;
		NFC_TRACE_ERROR("func:%s; line:%d,malloc recv_buff NULL ERROR", __FUNCTION__, __LINE__);
		return ret_firmware_success;
	}

	g_fwdl_test_status		= 1;
	int is_update_requred = 0;
	
	if (is_update_requred) {
	gHedNfcContx.fw_updated = 1;
	NFCC_Rest(1);
	ret_firmware_success = s_firmware_update_loop();
	if (ret_firmware_success == FW_UD_SUCCESS)
	{
		g_fwdl_test_status = 0;		
		NFC_TRACE_DEBUG("func:%s; line:%d, firmware updated success", __FUNCTION__, __LINE__);
	}
	else
	{
		g_fwdl_test_status = 2;
		NFC_TRACE_ERROR("func:%s; line:%d,s_firmware_update_loop firmware update failed, ret:%u", __FUNCTION__, __LINE__, ret_firmware_success);
	}
	g_nfcc_fwdl_flag			  = 1;
	NFCC_Rest(0);
	}
	else {
		ret_firmware_success = FW_UD_SUCCESS;
	}
	port_mem_freeMem(s_firmware_update_local.cmd_buff);
	s_firmware_update_local.cmd_buff = NULL;
	port_mem_freeMem(s_firmware_update_local.recv_buff);
	s_firmware_update_local.recv_buff = NULL;
	return ret_firmware_success;
}

uint16_t check_try_fwdl(void)
{
	NFC_TRACE_WARNING("enter, g_fwdl_test_flag(isCheckingNeeded):%d", g_fwdl_test_flag);
	uint16_t ret_firmware_success = FW_UD_ERR_FAIL;
	uint16_t is_update_requred	  = 0;

	if (g_fwdl_test_flag == 0)
	{
		is_update_requred = s_check_is_update_requred();
	}
	else
	{
		is_update_requred = 1;
	}
	NFC_TRACE_WARNING("is_update_requred:%d", is_update_requred);

	if (is_update_requred)
	{
		ret_firmware_success = foce_try_fwdl();
	}
	else
	{
		// ret_firmware_success = FW_UD_SUCCESS;
		return FW_UD_NO_NEED_SUCCESS;
	}

	return ret_firmware_success;
}

extern uint8_t *hed_nfc_get_ver(void);
extern uint8_t *hed_nfc_get_chip_ver(void);
static uint16_t s_check_is_update_requred(void)
{
	uint16_t cur_nfcc_ver		   = 0;
	uint8_t *cur_nfcc_ver_ptr	   = NULL;
	uint16_t ret_is_update_requred = 0;
	uint16_t status = FW_UD_ERR_FAIL;
	uint8_t fw_up_curr_step = 0;
	uint16_t up_fw_ver = s_get_fw_up_version();

	NFC_TRACE_WARNING("func:%s; line:%d, up_fw_ver: %#x", __FUNCTION__, __LINE__, up_fw_ver);
	if (up_fw_ver == 0) 
	{
		NFC_TRACE_WARNING("The firmware array to upgrade is empty");
		return 0;
	}

#ifdef HED_NFCC_FIRMWARE_CHIP_VERSION
	uint8_t *new_chip_ver = fwdl_getChipVersion();
	uint8_t *curr_chip_ver = hed_nfc_get_chip_ver();
	
	if (new_chip_ver == NULL || curr_chip_ver == NULL) {
		//NFC_TRACE_ERROR("NULL ERROR, new_chip_ver:%#x, curr_chip_ver:%#X", new_chip_ver, curr_chip_ver);
		return 0;
	}
	NFC_TRACE_WARNING(" new_chip_ver: %#x, %#x", *new_chip_ver, *(new_chip_ver+1));
	NFC_TRACE_WARNING("curr_chip_ver: %#x, %#x", *curr_chip_ver, *(curr_chip_ver+1));
	if (memcmp(curr_chip_ver, new_chip_ver, 2) != 0) 
	{
		if (*curr_chip_ver == 0xFF && *(curr_chip_ver+1) == 0xFF && *new_chip_ver == 0x06 && *(new_chip_ver+1) == 0x64) 
		{
			NFC_TRACE_WARNING("Some SOC1 chip types are fully 0xFF, continue updating");
		}
		else
		{
			NFC_TRACE_ERROR("chip version is not same, cannot be updated!!");
			return 0;
		}
	}
#endif

	status = s_get_fwdl_flag(&fw_up_curr_step);
	if (FW_UD_SUCCESS != status)
	{
		NFC_TRACE_ERROR("func:%s; line:%d,s_get_fwdl_flag EXECUTE ERROR", __FUNCTION__, __LINE__);
		return 1;
	}
	NFC_TRACE_WARNING("func:%s; line:%d, last firmware update flag: %#x", __FUNCTION__, __LINE__, fw_up_curr_step);
	if (FW_UP_LABLE_UP_DONE != fw_up_curr_step)
	{
		NFC_TRACE_WARNING("func:%s; line:%d,s_get_fwdl_flag indicate fw_up not done", __FUNCTION__, __LINE__);
		return 1;
	}

	cur_nfcc_ver_ptr = hed_nfc_get_ver();
	cur_nfcc_ver	 = (((uint16_t)cur_nfcc_ver_ptr[1]) << 8u) + cur_nfcc_ver_ptr[0];
	if (0 == cur_nfcc_ver)
	{ //if cur_nfcc_ver is 0, indicating that activate the nfcc failly, so need execute firmware_update
		NFC_TRACE_WARNING("func:%s; line:%d, cur_nfcc_ver is 0, indicating nfcc not activated", __FUNCTION__, __LINE__);
		return 1;
	}
	NFC_TRACE_WARNING("func:%s; line:%d,cur_nfcc_ver: %#x", __FUNCTION__, __LINE__, cur_nfcc_ver);
	ret_is_update_requred = s_check_is_fw_version_required(cur_nfcc_ver, up_fw_ver);

	return ret_is_update_requred;
}

static uint16_t s_check_is_fw_version_required(uint16_t cur_nfcc_ver, uint16_t up_fw_ver)
{
	uint16_t ret_check_is_fw_version_required = 0;

	if(0 == up_fw_ver) 
	{
		return 0;
	}

#if FIRMWARE_DOWNLOAD_STRATEGY == FW_UP_STRATEGY_UPPER_VERSION
	ret_check_is_fw_version_required = up_fw_ver > cur_nfcc_ver ? 1 : 0;
#elif FIRMWARE_DOWNLOAD_STRATEGY == FW_UP_STRATEGY_DIFFERENT_VERSION
	ret_check_is_fw_version_required = (up_fw_ver != cur_nfcc_ver) ? 1 : 0;
// #elif FIRMWARE_DOWNLOAD_STRATEGY == FW_UP_STRATEGY_ALWAYS
// 	ret_check_is_fw_version_required = 1;
#else
	ret_check_is_fw_version_required = up_fw_ver > cur_nfcc_ver ? 1 : 0;
#endif
	return ret_check_is_fw_version_required;
}

static uint16_t s_get_fw_up_version(void)
{
	return fwdl_getVersion();
}

static uint16_t s_firmware_update_loop(void)
{
	uint16_t ret_single_update_success = FW_UD_ERR_FAIL;
	uint16_t update_counter			   = 0;

	s_set_fwdl_flag(FW_UP_LABLE_UP_START);

	s_set_nfcc_ioctl(phTmlNfc_e_EnableDownloadMode);

	s_firmware_update_local.valid_cmd_needed = 0;

	for (update_counter = 0; update_counter < FW_UP_CONF_MAX_UP_CNT; update_counter++)
	{
		s_firmware_update_local.is_first_fw_wr = 1;
		s_firmware_update_local.fw_up_fsm	   = FW_UP_FSM_RESET;

		ret_single_update_success = s_firmware_update_download();
		if (FW_UD_SUCCESS == ret_single_update_success)
		{
			NFC_TRACE_DEBUG("func:%s; line:%d, fw_up success", __FUNCTION__, __LINE__);
			break;
		}
	}

	s_set_nfcc_ioctl(phTmlNfc_e_EnableNormalMode);

	if (update_counter < FW_UP_CONF_MAX_UP_CNT && FW_UD_SUCCESS == ret_single_update_success)
	{
		s_set_fwdl_flag(FW_UP_LABLE_UP_DONE);
	}

	return ret_single_update_success;
}

static uint16_t s_set_fwdl_flag(uint8_t fw_up_curr_step)
{
	uint16_t ret_set_fwdl_flag = FW_UD_ERR_FAIL;
	uint16_t status			   = FW_UD_ERR_FAIL;

	status = s_port_storage_setFlag(0, fw_up_curr_step); //first para is unused
	NFC_TRACE_DEBUG("func:%s; line:%d, fw_up set flag:%u", __FUNCTION__, __LINE__, fw_up_curr_step);

	ret_set_fwdl_flag = status; //to map
	return ret_set_fwdl_flag;
}

static uint16_t s_get_fwdl_flag(uint8_t *fw_up_curr_step)
{
	uint16_t ret_get_fwdl_flag = FW_UD_SUCCESS;

	ret_get_fwdl_flag = s_port_storage_getFlag(fw_up_curr_step);

	return ret_get_fwdl_flag;
}

static uint16_t s_set_nfcc_ioctl(phTmlNfc_ControlCode_t eControlCode)
{
	uint16_t ret_set_nfcc_ioctl = FW_UD_ERR_FAIL;

	switch (eControlCode)
	{
	case phTmlNfc_e_EnableDownloadMode:
		NFC_TRACE_DEBUG(" fw_up phTmlNfc_e_EnableDownloadMode ");
		s_port_gpio_control(FW_UP_PIN_DWL_REQ, FW_UP_PIN_LEVEL_HIGH);
		s_port_timer_wait_msec(10);
		s_port_gpio_control(FW_UP_PIN_VEN, FW_UP_PIN_LEVEL_LOW);
		s_port_timer_wait_msec(10);
		s_port_gpio_control(FW_UP_PIN_VEN, FW_UP_PIN_LEVEL_HIGH);
		s_port_timer_wait_msec(10);
		s_port_timer_wait_msec(100);
		break;
	case phTmlNfc_e_EnableNormalMode:
		NFC_TRACE_DEBUG(" fw_up  phTmlNfc_e_EnableNormalMode complete with VEN RESET ");
		s_port_gpio_control(FW_UP_PIN_DWL_REQ, FW_UP_PIN_LEVEL_LOW);
		s_port_gpio_control(FW_UP_PIN_VEN, FW_UP_PIN_LEVEL_LOW);
		s_port_timer_wait_msec(10);
		s_port_gpio_control(FW_UP_PIN_DWL_REQ, FW_UP_PIN_LEVEL_LOW);
		s_port_gpio_control(FW_UP_PIN_VEN, FW_UP_PIN_LEVEL_HIGH);
		s_port_timer_wait_msec(100);
		break;
	default:

		break;
	}

	return ret_set_nfcc_ioctl;
}

static uint16_t s_firmware_update_download(void)
{
	uint16_t ret_firmware_update_download = FW_UD_ERR_FAIL;

	while ((FW_UP_FSM_FAIL != s_firmware_update_local.fw_up_fsm) && (FW_UP_FSM_SUCCESS != s_firmware_update_local.fw_up_fsm))
	{
		switch (s_firmware_update_local.fw_up_fsm)
		{
		case FW_UP_FSM_RESET:
		{
			ret_firmware_update_download = s_firmware_update_download_reset();
			if (ret_firmware_update_download == FW_UD_SUCCESS)
			{
				s_firmware_update_local.fw_up_fsm = FW_UP_FSM_CHECK;
				NFC_TRACE_DEBUG("func:%s; line:%d, fw_up nfcc reset success", __FUNCTION__, __LINE__);
			}
			else
			{
				s_firmware_update_local.fw_up_fsm = FW_UP_FSM_FAIL;
				NFC_TRACE_DEBUG("func:%s; line:%d, fw_up nfcc reset fail", __FUNCTION__, __LINE__);
			}
		}
		break;
		case FW_UP_FSM_CHECK:
		{
			if (s_firmware_update_local.is_update_pub_key == 0)
			{
				ret_firmware_update_download = s_firmware_update_download_check();
				NFC_TRACE_DEBUG("func:%s; line:%d, fw_up nfcc check result:%u", __FUNCTION__, __LINE__, (ret_firmware_update_download == FW_UD_SUCCESS ? 1 : 0));
			}
			else
			{
				NFC_TRACE_DEBUG("func:%s; line:%d, fw_up pub key change, not need check", __FUNCTION__, __LINE__);
			}

			if (ret_firmware_update_download == FW_UD_SUCCESS)
			{
				s_firmware_update_local.fw_up_fsm = FW_UP_FSM_DATA;
			}
			else
			{
				s_firmware_update_local.fw_up_fsm = FW_UP_FSM_FAIL;
			}
		}
		break;
		case FW_UP_FSM_DATA:
		{
			s_firmware_update_local.download_frame_cnt = 0;
			ret_firmware_update_download			   = s_firmware_update_download_fw_data();
			if (ret_firmware_update_download == FW_UD_SUCCESS)
			{
				NFC_TRACE_DEBUG("func:%s; line:%d, fw_up download data complete", __FUNCTION__, __LINE__);
				s_firmware_update_local.fw_up_fsm = FW_UP_FSM_SUCCESS;
			}
			else if (ret_firmware_update_download == FW_UD_SUCCESS_CHANGE_PUB_KEY)
			{ //need execute fw_up again
				NFC_TRACE_DEBUG("func:%s; line:%d, fw_up download data, already change pub key ", __FUNCTION__, __LINE__);
				s_firmware_update_local.fw_up_fsm = FW_UP_FSM_FAIL;
			}
			else
			{
				NFC_TRACE_ERROR("func:%s; line:%d, fw_up download data failed", __FUNCTION__, __LINE__);
				s_firmware_update_local.fw_up_fsm = FW_UP_FSM_FAIL;
			}
		}
		break;
		case FW_UP_FSM_SUCCESS:
		{
			ret_firmware_update_download = FW_UD_SUCCESS;
			NFC_TRACE_DEBUG("firmware update success");
		}
		break;
		case FW_UP_FSM_FAIL:
		{
			ret_firmware_update_download = FW_UD_ERR_FAIL;
			NFC_TRACE_ERROR("func:%s; line:%d,firmware update failed EEROR", __FUNCTION__, __LINE__);
		}
		break;
		default:
		{
			s_firmware_update_local.fw_up_fsm = FW_UP_FSM_FAIL;
			NFC_TRACE_ERROR("func:%s; line:%d,s_firmware_update_local.fw_up_fsm para error EEROR", __FUNCTION__, __LINE__);
		}
		break;
		}
	}

	return ret_firmware_update_download;
}

static uint16_t s_build_frame(uint8_t *cmd, uint16_t cmd_len, bool need_cpy_cmd)
{
	if ((cmd_len + FW_UP_FRAME_HEADER_LEN) > FW_UP_MAX_CMD_SIZE)
	{
		NFC_TRACE_ERROR("wFrameLen exceeds the limit");
		return FW_UD_ERR_FAIL;
	}

	if (true == need_cpy_cmd)
	{
		memset(s_firmware_update_local.cmd_buff, 0, s_firmware_update_local.max_cmd_size);
	}

	s_firmware_update_local.cmd_buff[0] = 0x00;
	s_firmware_update_local.cmd_buff[1] = (uint8_t)(cmd_len >> 8);
	s_firmware_update_local.cmd_buff[2] = (uint8_t)cmd_len;

	s_firmware_update_local.cmd_len = cmd_len + FW_UP_FRAME_HEADER_LEN;
	if (true == need_cpy_cmd)
	{
		memcpy(&s_firmware_update_local.cmd_buff[FW_UP_FRAME_HEADER_LEN], cmd, cmd_len);
	}

	return FW_UD_SUCCESS;
}

static uint16_t s_fw_up_dwld_withtime(uint16_t overtime_ms)
{
	uint16_t ret_fw_up_dwld_withtime = FW_UD_ERR_FAIL;
	uint16_t sw;

	s_firmware_update_local.recv_len = s_firmware_update_local.max_recv_size;
	memset(s_firmware_update_local.recv_buff, 0, s_firmware_update_local.max_recv_size);

	ret_fw_up_dwld_withtime = s_fw_up_send_string(s_firmware_update_local.cmd_buff,
												  s_firmware_update_local.cmd_len,
												  overtime_ms,
												  s_firmware_update_local.recv_buff,
												  &s_firmware_update_local.recv_len);
	if (s_firmware_update_local.recv_len < 2)
	{
		NFC_TRACE_ERROR("s_fw_up_dwld_withtime, recv_len less than 2 error");
		return FW_UD_ERR_FAIL;
	}

	if (ret_fw_up_dwld_withtime == FW_UD_SUCCESS)
	{
		sw = ((uint16_t)(s_firmware_update_local.recv_buff[s_firmware_update_local.recv_len - 2]) << 8u) + s_firmware_update_local.recv_buff[s_firmware_update_local.recv_len - 1];
		if (0x9000 != sw)
		{
			NFC_TRACE_ERROR("s_fw_up_dwld_withtime, sw is not 0x9000 error, sw:%u !", sw);
			return FW_UD_ERR_FAIL;
		}
	}

	return ret_fw_up_dwld_withtime;
}

static uint16_t s_firmware_update_download_reset(void)
{
	uint16_t ret_fw_up_download_reset = FW_UD_ERR_FAIL;
	uint8_t resetCmd[7]				  = {0xBF, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00};

	ret_fw_up_download_reset = s_build_frame(resetCmd, sizeof(resetCmd), true);

	if (ret_fw_up_download_reset == FW_UD_SUCCESS)
	{
		ret_fw_up_download_reset = s_fw_up_dwld_withtime(2000); //max overtime: 2000ms
	}
	if (FW_UD_SUCCESS != ret_fw_up_download_reset)
	{
		NFC_TRACE_ERROR("s_firmware_update_download_reset  s_fw_up_dwld_withtime error, ret:%u", ret_fw_up_download_reset);
	}

	return ret_fw_up_download_reset;
}
static uint16_t s_firmware_update_download_check(void)
{
	uint16_t ret_fw_up_download_check = FW_UD_ERR_FAIL;
	uint8_t checkDmd[7]				  = {0xBF, 0x48, 0xC2, 0x00, 0x00, 0x00, 0x98};
	uint8_t check_data[4]			  = {0x8D, 0xAE, 0x9A, 0x6B};

	ret_fw_up_download_check = s_build_frame(checkDmd, sizeof(checkDmd), true);
	if (ret_fw_up_download_check == FW_UD_SUCCESS)
	{
		ret_fw_up_download_check = s_fw_up_dwld_withtime(2000); //max overtime: 2000ms
	}

	if (ret_fw_up_download_check == FW_UD_SUCCESS)
	{
		if ((memcmp(check_data, s_firmware_update_local.recv_buff + FW_UP_FRAME_HEADER_LEN + 0x18, 4) == 0) && (s_firmware_update_local.recv_len == 0x9D))
		{
			s_firmware_update_local.valid_cmd_needed = 1;
			NFC_TRACE_DEBUG("fw_up check, user completeness verify switch: opend");
		}
	}

	return ret_fw_up_download_check;
}
static uint16_t s_firmware_update_download_fw_data(void)
{
	uint16_t ret_fw_up_download_fw_data = FW_UD_SUCCESS;
	uint16_t sw;
	uint8_t is_change_pub_key = 0;

	if (s_firmware_update_local.max_cmd_size <= FW_UP_FRAME_HEADER_LEN)
	{
		NFC_TRACE_ERROR("s_firmware_update_local.max_cmd_size error");
		return FW_UD_ERR_FAIL;
	}
	s_firmware_update_local.is_final_frame = 0;
	while ((s_firmware_update_local.is_final_frame == 0) && (ret_fw_up_download_fw_data == FW_UD_SUCCESS))
	{
		if (is_change_pub_key == 1)
		{ //exit with FW_UD_SUCCESS_CHANGE_PUB_KEY after send the change_pub_key cmd
			ret_fw_up_download_fw_data = FW_UD_SUCCESS_CHANGE_PUB_KEY;
			break;
		}

		s_firmware_update_local.cmd_len = s_firmware_update_local.max_cmd_size - FW_UP_FRAME_HEADER_LEN;
		ret_fw_up_download_fw_data		= s_fw_up_get_fw_cmd(&s_firmware_update_local.cmd_buff[FW_UP_FRAME_HEADER_LEN], &s_firmware_update_local.cmd_len);
		if (FW_UD_SUCCESS_FINAL_FRAME == ret_fw_up_download_fw_data)
		{
			s_firmware_update_local.is_final_frame = 1;
			ret_fw_up_download_fw_data			   = FW_UD_SUCCESS;
			NFC_TRACE_DEBUG("fw_up download fw_data, final frame begain");
		}
		else if (FW_UD_SUCCESS_CHANGE_PUB_KEY == ret_fw_up_download_fw_data)
		{
			is_change_pub_key		   = 1;
			ret_fw_up_download_fw_data = FW_UD_SUCCESS;
		}
		else if (FW_UD_SUCCESS != ret_fw_up_download_fw_data)
		{
			NFC_TRACE_ERROR("fw_up download fw_data, get cmd failed");
		}

		if (ret_fw_up_download_fw_data == FW_UD_SUCCESS)
		{
			ret_fw_up_download_fw_data = s_build_frame(s_firmware_update_local.cmd_buff, s_firmware_update_local.cmd_len, false);
		}

		if (FW_UD_SUCCESS != ret_fw_up_download_fw_data)
		{
			NFC_TRACE_ERROR("fw_up download fw_data, build frame failed");
		}

		if ((s_firmware_update_local.cmd_buff[FW_UP_FRAME_HEADER_LEN + 1] == FW_UP_DL_Para_Valid) && (s_firmware_update_local.valid_cmd_needed != 1))
		{
			ret_fw_up_download_fw_data = FW_UP_ERR_VALID_PARA;
			NFC_TRACE_ERROR("fw_up download fw_data, user completeness verify switch ERROR");
		}

		if (ret_fw_up_download_fw_data == FW_UD_SUCCESS)
		{
			ret_fw_up_download_fw_data = s_fw_up_dwld_withtime(2000); //max overtime: 2000ms

			if (ret_fw_up_download_fw_data == FW_UD_SUCCESS)
			{
				s_firmware_update_local.download_frame_cnt++;
				if (s_firmware_update_local.is_first_fw_wr == 1)
				{
					s_firmware_update_local.is_first_fw_wr = 0;
				}
			}
			else
			{
				NFC_TRACE_ERROR("fw_up download fw_data failed, ret:%u", ret_fw_up_download_fw_data);
			}

			sw = ((uint16_t)(s_firmware_update_local.recv_buff[s_firmware_update_local.recv_len - 2]) << 8u) + s_firmware_update_local.recv_buff[s_firmware_update_local.recv_len - 1];
			if (sw == FW_UP_ERR_SIGNATURE_ERROR)
			{
				s_firmware_update_local.is_update_pub_key = 1; //if this occur, return_value of last called funciton was FW_UD_ERR_FAIL
				NFC_TRACE_ERROR("fw_up download fw_data failed, 0x6400 SIGNATRUE ERROR");
			}
		}
	}

	return ret_fw_up_download_fw_data;
}

static uint16_t s_fw_up_get_fw_cmd(uint8_t *temp_cmd_tb, uint16_t *frame_cmd_len)
{
	uint16_t ret_fw_up_get_cmd = FW_UD_ERR_FAIL;
	uint8_t is_change_pub_key  = 0;
	PORT_Status_t status;
	if (s_firmware_update_local.is_update_pub_key == 0)
	{
		if (s_firmware_update_local.is_first_fw_wr == 1)
		{
			status = fwdl_getCmd(temp_cmd_tb, frame_cmd_len);
		}
		else
		{
			status = fwdl_getNextCmd(temp_cmd_tb, frame_cmd_len);
		}
	}
	else
	{
		s_firmware_update_local.is_update_pub_key = 0;
		is_change_pub_key						  = 1;
		status									  = fwdl_getChangeKeyCmd(temp_cmd_tb, frame_cmd_len);
	}
	if (PORT_STATUS_SUCCESS == status)
	{
		ret_fw_up_get_cmd = FW_UD_SUCCESS;
		if (1 == is_change_pub_key)
		{
			ret_fw_up_get_cmd = FW_UD_SUCCESS_CHANGE_PUB_KEY;
		}
	}
	else if (PORT_STATUS_SUCCESS_EXT == status)
	{
		ret_fw_up_get_cmd = FW_UD_SUCCESS_FINAL_FRAME;
	}
	else if (PORT_STATUS_INSUFFICIENT_SPACE == status)
	{
		ret_fw_up_get_cmd = FW_UD_ERR_INSUFFICIENT_SPACE;
	}
	else
	{
		ret_fw_up_get_cmd = FW_UD_ERR_FAIL;
	}
	return ret_fw_up_get_cmd;
}

/******************************need to call extern module, as below **********************************/

/*******************************************************************************
  **
  ** Function         drv_io_control
  **
  ** Description      control gpio pin level
  **                  
  ** Parameters				pin_num: PIN_VEN_NUM, PIN_DWL_REQ_NUM
  **                  level: PIN_LEVEL_LOW, PIN_LEVEL_HIGH 
  **                    
  ** Returns          none
  **                   
  *******************************************************************************/
void drv_io_control(uint8_t pin_num, uint8_t level)
{
	if (pin_num == PIN_VEN_NUM)
	{
		if (level == PIN_LEVEL_LOW)
		{
			port_hardware_gpioVenPulldown();
		}
		else if (level == PIN_LEVEL_HIGH)
		{
			port_hardware_gpioVenPullup();
		}
	}
	else if (pin_num == PIN_DWL_REQ_NUM)
	{
		if (level == PIN_LEVEL_LOW)
		{
			port_hardware_gpioDwlPulldown();
		}
		else if (level == PIN_LEVEL_HIGH)
		{
			port_hardware_gpioDwlPullup();
		}
	}
}

static void s_port_gpio_control(uint8_t pin, uint8_t level)
{
	uint8_t pin_num	  = (pin == FW_UP_PIN_VEN ? PIN_VEN_NUM : PIN_DWL_REQ_NUM);
	uint8_t pin_level = (level == FW_UP_PIN_LEVEL_LOW ? PIN_LEVEL_LOW : PIN_LEVEL_HIGH);
	drv_io_control(pin_num, pin_level);
}

static void s_port_timer_wait_msec(uint16_t msec)
{
	timer_wait_ms(msec);
}

t_RetStatus Fwdl_receive(uint8_t *nci_recv_packet, uint16_t *outlen)
{ //接收响应通知
	uint8_t *pbuf	= (uint8_t *)port_mem_getMem(BUFFER_BLOCK_SIZE);
	uint16_t rcvlen = 0;
	if (pbuf == NULL)
		return MT_ERROR;
	if (NfcCmdI2cRecvData(pbuf, BUFFER_BLOCK_SIZE - 1) != 0)
	{ //接收到数据
		rcvlen = pbuf[2] + 3;
		if (*outlen >= rcvlen)
		{
			memcpy(nci_recv_packet, pbuf, rcvlen);
			*outlen = rcvlen;
		}
		else
		{
			return MEM_ERROR;
		}

		port_mem_freeMem(pbuf);
		return CMD_RECE_OK; //获取到正确响应
	}
	port_mem_freeMem(pbuf);
	return CMD_RECE_WAIT;
}

t_RetStatus Fwdl_SndRcv_Stream(uint8_t *pbuf, uint16_t plen, uint32_t timeout, uint8_t *nci_recv_packet, uint16_t *outlen)
{ //发送命令
	uint8_t count = 3;
	uint8_t rst	  = 0;
	tTIMER_CB recvdata_timeout; //接收超时时间
	while (count--)
	{
		rst = write_buffer(pbuf, plen); //直接发送 0
		if (rst != 0)
		{
			timer_set_timeout(&recvdata_timeout, timeout); //超时时间 NCI_ACK_TIMEOUT
			while (1)
			{
				t_RetStatus rcvRst = Fwdl_receive(nci_recv_packet, outlen);
				if ((rcvRst == CMD_RECE_OK) || (rcvRst == MEM_ERROR))
				{ //接收数据
					return rcvRst;
				}
				if (timer_wait_timeout(&recvdata_timeout) == 1)
				{
					NFC_TRACE_DEBUG("Fwdl_SndRcv_Stream CMD_TIME_OUT");
					break;
				}
			}
		}
	}
	return CMD_TIME_OUT;
}

static uint16_t s_fw_up_send_string(uint8_t *cmd_buf, uint16_t cmd_len, uint32_t timeout, uint8_t *recv_reply, uint16_t *recv_len)
{
	uint16_t status		  = FW_UD_SUCCESS;
	t_RetStatus RetStatus = Fwdl_SndRcv_Stream(cmd_buf, cmd_len, timeout, recv_reply, recv_len); //
	if (CMD_TIME_OUT == RetStatus)
	{
		status = FW_UD_ERR_TIME_OUT;
	}
	else if (CMD_RECE_OK == RetStatus || NO_ERR == RetStatus)
	{
		status = FW_UD_SUCCESS;
	}
	else
	{
		status = FW_UD_ERR_FAIL;
	}
	return status;
}

extern volatile uint8_t fwdl_flag_default[1];

static PORT_Status_t storage_getFlag(uint8_t *value)
{
	*value = port_hardware_flashReadU8((uint32_t)&fwdl_flag_default[0]); //目前仅有此标志，暂放在RAM中,客户接口中，期望是存在NVM区

	// *value = fwdl_flag_default[0];
	return PORT_STATUS_SUCCESS;
}

static PORT_Status_t storage_setFlag(uint8_t flag, uint8_t value)
{
	port_hardware_flashWriteU8((uint32_t)&fwdl_flag_default[0], value); //目前仅有此标志，暂放在RAM中,客户接口中，期望是存在NVM区
	// fwdl_flag_default[0] = value;
	return PORT_STATUS_SUCCESS;
}

static uint16_t s_port_storage_getFlag(uint8_t *value)
{
	PORT_Status_t status;
	status = storage_getFlag(value);

	return (PORT_STATUS_SUCCESS == status ? FW_UD_SUCCESS : FW_UD_ERR_FAIL);
}

static uint16_t s_port_storage_setFlag(uint8_t flag, uint8_t value)
{
	PORT_Status_t status;
	status = storage_setFlag(flag, value); //first is unused

	return (PORT_STATUS_SUCCESS == status ? FW_UD_SUCCESS : FW_UD_ERR_FAIL);
}
