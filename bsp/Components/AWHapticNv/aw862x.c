/*
 * File: aw862x.c
 *
 * Author: <wangzhi@awinic.com>
 *
 * Copyright ©2021-2023 awinic.All Rights Reserved
 *
 */
#include "stdio.h"
#include "haptic_nv.h"
#include "haptic_nv_reg.h"

struct aw_haptic_dts_info aw862x_dts = {
	.f0_pre = 1700,
	.f0_cali_percent = 7,
	.cont_drv1_lvl = 0x7D,
	.cont_drv2_lvl = 0x9B,
	.cont_td = 0xF06C,
	.cont_zc_thr = 0x08F8,
	.cont_num_brk = 0x03,
	.f0_coeff = 260,
	.cont_brake[0] = 0x1,
	.cont_brake[1] = 0x1,
	.cont_brake[2] = 0x5A,
	.cont_brake[3] = 0x2A,
	.cont_brake[4] = 0x14,
	.cont_brake[5] = 0x05,
	.cont_brake[6] = 0x02,
	.cont_brake[7] = 0x02,
	.f0_trace_parameter[0] = 0x05,
	.f0_trace_parameter[1] = 0x03,
	.f0_trace_parameter[2] = 0x02,
	.f0_trace_parameter[3] = 0x0F,
	.bemf_config[0] = 0x10,
	.bemf_config[1] = 0x08,
	.bemf_config[2] = 0x23,
	.bemf_config[3] = 0xF8,
	.sw_brake[0] = 0x2C,
	.sw_brake[1] = 0x08,
	.cont_tset = 0x11,
};

static struct trig aw862x_trig1 = {
	.enable = 0,
	.trig_edge = 1,
	.trig_polar = 0,
	.pos_sequence = 1,
	.neg_sequence = 1,
};

static int aw862x_check_qualify(void)
{
	return AW_SUCCESS;
}

static int aw862x_haptic_play_init(void)
{
	uint8_t reg_val = 0;

	if (g_haptic_nv->play_mode == AW_CONT_MODE)
		reg_val = g_haptic_nv->info->sw_brake[0];
	else
		reg_val = g_haptic_nv->info->sw_brake[1];

	haptic_nv_i2c_writes(AW862X_REG_SW_BRAKE, &reg_val, AW_I2C_BYTE_ONE);
	return AW_SUCCESS;
}

static void aw862x_interrupt_clear(void)
{
	uint8_t reg_val = 0;

	haptic_nv_i2c_reads(AW862X_REG_SYSINT, &reg_val, AW_I2C_BYTE_ONE);
	AW_LOGI("reg SYSINT=0x%X", reg_val);
}

static void aw862x_haptic_active(void)
{
	aw862x_haptic_play_init();
	haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_WORK_MODE_MASK,
							 AW862X_BIT_SYSCTRL_ACTIVE);
	aw862x_interrupt_clear();
	haptic_nv_i2c_write_bits(AW862X_REG_SYSINTM, AW862X_BIT_SYSINTM_UVLO_MASK,
							 AW862X_BIT_SYSINTM_UVLO_EN);
}

static void aw862x_play_mode(uint8_t play_mode)
{
	switch (play_mode) {
	case AW_STANDBY_MODE:
		AW_LOGI("enter standby mode");
		g_haptic_nv->play_mode = AW_STANDBY_MODE;
		haptic_nv_i2c_write_bits(AW862X_REG_SYSINTM, AW862X_BIT_SYSINTM_UVLO_MASK,
								 AW862X_BIT_SYSINTM_UVLO_OFF);
		haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_WORK_MODE_MASK,
								 AW862X_BIT_SYSCTRL_STANDBY);
		break;
	case AW_RAM_MODE:
		AW_LOGI("enter ram mode");
		g_haptic_nv->play_mode = AW_RAM_MODE;
		haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_PLAY_MODE_MASK,
								 AW862X_BIT_SYSCTRL_PLAY_MODE_RAM);
		aw862x_haptic_active();
		break;
	case AW_RAM_LOOP_MODE:
		AW_LOGI("enter ram loop mode");
		g_haptic_nv->play_mode = AW_RAM_LOOP_MODE;
		haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_PLAY_MODE_MASK,
								 AW862X_BIT_SYSCTRL_PLAY_MODE_RAM);
		aw862x_haptic_active();
		break;
	case AW_RTP_MODE:
		AW_LOGI("enter rtp mode");
		g_haptic_nv->play_mode = AW_RTP_MODE;
		haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_PLAY_MODE_MASK,
								 AW862X_BIT_SYSCTRL_PLAY_MODE_RTP);
		aw862x_haptic_active();
		break;
	case AW_TRIG_MODE:
		AW_LOGI("enter trig mode");
		g_haptic_nv->play_mode = AW_TRIG_MODE;
		haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_PLAY_MODE_MASK,
								 AW862X_BIT_SYSCTRL_PLAY_MODE_RAM);
		aw862x_haptic_active();
		break;
	case AW_CONT_MODE:
		AW_LOGI("enter cont mode");
		g_haptic_nv->play_mode = AW_CONT_MODE;
		haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_PLAY_MODE_MASK,
								 AW862X_BIT_SYSCTRL_PLAY_MODE_CONT);
		aw862x_haptic_active();
		break;
	default:
		AW_LOGE("play mode %d error", play_mode);
		break;
	}
}

static void aw862x_set_pwm(uint8_t mode)
{
	switch (mode) {
	case AW_PWM_48K:
		haptic_nv_i2c_write_bits(AW862X_REG_PWMDBG, AW862X_BIT_PWMDBG_PWM_MODE_MASK,
								 AW862X_BIT_PWMDBG_PWM_48K);
		break;
	case AW_PWM_24K:
		haptic_nv_i2c_write_bits(AW862X_REG_PWMDBG, AW862X_BIT_PWMDBG_PWM_MODE_MASK,
								 AW862X_BIT_PWMDBG_PWM_24K);
		break;
	case AW_PWM_12K:
		haptic_nv_i2c_write_bits(AW862X_REG_PWMDBG, AW862X_BIT_PWMDBG_PWM_MODE_MASK,
								 AW862X_BIT_PWMDBG_PWM_12K);
		break;
	default:
		break;
	}
}

static void aw862x_protect_config(uint8_t prtime, uint8_t prlvl)
{
	uint8_t reg_val = 0;

	haptic_nv_i2c_write_bits(AW862X_REG_PWMPRC,
				 AW862X_BIT_PWMPRC_PRC_EN_MASK,
				 AW862X_BIT_PWMPRC_PRC_DISABLE);
	if (prlvl != 0) {
		/* Enable protection mode */
		AW_LOGI("enable protection mode");
		reg_val = AW862X_BIT_PRLVL_PR_ENABLE |
			  (prlvl & (~AW862X_BIT_PRLVL_PRLVL_MASK));
		haptic_nv_i2c_writes(AW862X_REG_PRLVL, &reg_val,
				     AW_I2C_BYTE_ONE);
		haptic_nv_i2c_writes(AW862X_REG_PRTIME, &prtime,
				     AW_I2C_BYTE_ONE);
	} else {
		/* Disable */
		AW_LOGI("disable protection mode");
		haptic_nv_i2c_write_bits(AW862X_REG_PRLVL,
					 AW862X_BIT_PRLVL_PR_EN_MASK,
					 AW862X_BIT_PRLVL_PR_DISABLE);
	}
}

static void aw862x_misc_para_init(void)
{
	int ret = 0;
	uint8_t reg_val = 0;
	uint8_t reg_flag = 0;
	uint8_t reg_array[8] = {0};

	/* Get chipid_flag */
	ret = haptic_nv_i2c_reads(AW862X_REG_EF_RDATAH, &reg_flag, AW_I2C_BYTE_ONE);
	if ((ret >= 0) && ((reg_flag & 0x1) == 1))
		g_haptic_nv->chipid_flag = 1;
	else
		AW_LOGE("to read register AW862X_REG_EF_RDATAH: %d", ret);
	/* r_spare */
	haptic_nv_i2c_write_bits(AW862X_REG_R_SPARE, AW862X_BIT_R_SPARE_MASK,
							 AW862X_BIT_R_SPARE_ENABLE);
	/* LRA trim source select register */
	haptic_nv_i2c_write_bits(AW862X_REG_ANACTRL, AW862X_BIT_ANACTRL_LRA_SRC_MASK,
							 AW862X_BIT_ANACTRL_LRA_SRC_REG);
	/* brake */
	reg_val =g_haptic_nv->info->sw_brake[0];
	haptic_nv_i2c_writes(AW862X_REG_SW_BRAKE, &reg_val, AW_I2C_BYTE_ONE);
	reg_val = 0x00;
	haptic_nv_i2c_writes(AW862X_REG_THRS_BRA_END, &reg_val, AW_I2C_BYTE_ONE);
	haptic_nv_i2c_write_bits(AW862X_REG_WAVECTRL, AW862X_BIT_WAVECTRL_NUM_OV_DRIVER_MASK,
							 AW862X_BIT_WAVECTRL_NUM_OV_DRIVER);
	/* zero cross */
	reg_array[0] =g_haptic_nv->info->cont_zc_thr >> 8;
	reg_array[1] =g_haptic_nv->info->cont_zc_thr >> 0;
	haptic_nv_i2c_writes(AW862X_REG_ZC_THRSH_H, reg_array, AW_I2C_BYTE_TWO);
	/* cont_tset */
	reg_val =g_haptic_nv->info->cont_tset;
	haptic_nv_i2c_writes(AW862X_REG_TSET, &reg_val, AW_I2C_BYTE_ONE);
	/* bemf */
	reg_array[0] =g_haptic_nv->info->bemf_config[0];
	reg_array[1] =g_haptic_nv->info->bemf_config[1];
	reg_array[2] =g_haptic_nv->info->bemf_config[2];
	reg_array[3] =g_haptic_nv->info->bemf_config[3];
	haptic_nv_i2c_writes(AW862X_REG_BEMF_VTHH_H, reg_array, AW_I2C_BYTE_FOUR);
	aw862x_set_pwm(AW_PWM_24K);
	aw862x_protect_config(AW862X_REG_PRTIME_DEFAULT_VALUE,
				AW862X_BIT_PRLVL_PRLVL_DEFAULT_VALUE);
}

static void aw862x_raminit(aw_bool flag)
{
	if (flag) {
		haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_RAMINIT_MASK,
								 AW862X_BIT_SYSCTRL_RAMINIT_EN);
	} else {
		haptic_nv_i2c_write_bits(AW862X_REG_SYSCTRL, AW862X_BIT_SYSCTRL_RAMINIT_MASK,
								 AW862X_BIT_SYSCTRL_RAMINIT_OFF);
	}
}

static int aw862x_offset_cali(void)
{
	uint8_t reg_val = 0;
	int  cont = 2000;

	AW_LOGI("enter");
	aw862x_raminit(AW_TRUE);
	haptic_nv_i2c_write_bits(AW862X_REG_DETCTRL, AW862X_BIT_DETCTRL_DIAG_GO_MASK,
							 AW862X_BIT_DETCTRL_DIAG_GO_ENABLE);
	while (cont) {
		haptic_nv_i2c_reads(AW862X_REG_DETCTRL, &reg_val, AW_I2C_BYTE_ONE);
		if ((reg_val & 0x01) == 0 || cont == 0)
			break;
		cont--;
	}
	aw862x_raminit(AW_FALSE);
	if (cont == 0) {
		AW_LOGE("calibration offset failed!");
		return AW_ERROR;
	}

	return AW_SUCCESS;
}

static void aw862x_vbat_mode_config(uint8_t flag)
{
	AW_LOGI("enter");
	if (flag == AW_CONT_VBAT_HW_COMP_MODE) {
		haptic_nv_i2c_write_bits(AW862X_REG_ADCTEST, AW862X_BIT_DETCTRL_VBAT_MODE_MASK,
								 AW862X_BIT_DETCTRL_VBAT_HW_COMP);
	} else {
		haptic_nv_i2c_write_bits(AW862X_REG_ADCTEST, AW862X_BIT_DETCTRL_VBAT_MODE_MASK,
								 AW862X_BIT_DETCTRL_VBAT_SW_COMP);
	}
}

static void aw862x_set_trim_lra(uint8_t val)
{
	uint8_t reg_val = 0;

	reg_val = val & AW862X_BIT_TRIM_LRA;
	haptic_nv_i2c_writes(AW862X_REG_TRIM_LRA, &reg_val, AW_I2C_BYTE_ONE);
}

static void aw862x_read_f0(void)
{
	uint8_t val[2] = {0};
	uint32_t f0_reg = 0;
	uint64_t f0_tmp = 0;

	haptic_nv_i2c_reads(AW862X_REG_F_LRA_F0_H, val, AW_I2C_BYTE_TWO);
	f0_reg = (val[0] << 8) | val[1];
	if (!f0_reg || !g_haptic_nv->info->f0_coeff) {
		g_haptic_nv->f0 = 0;
		AW_LOGE("get f0 failed with the value becoming 0!");
		return;
	}
	f0_tmp = AW862X_F0_FORMULA(f0_reg, g_haptic_nv->info->f0_coeff);
	g_haptic_nv->f0 = (uint32_t)f0_tmp;
	AW_LOGI("f0=%ld", g_haptic_nv->f0);
}

static void aw862x_read_beme(void)
{
	uint8_t val[2] = {0};

	haptic_nv_i2c_reads(AW862X_REG_WAIT_VOL_MP, val, AW_I2C_BYTE_TWO);
	g_haptic_nv->max_pos_beme = val[0];
	g_haptic_nv->max_neg_beme = val[1];
	AW_LOGI("max_pos_beme=%d", g_haptic_nv->max_pos_beme);
	AW_LOGI("max_neg_beme=%d", g_haptic_nv->max_neg_beme);
}

static uint8_t aw862x_get_glb_state(void)
{
	uint8_t state = 0;

	haptic_nv_i2c_reads(AW862X_REG_GLB_STATE, &state, AW_I2C_BYTE_ONE);
	AW_LOGI("reg:0x%02X=0x%02X", AW862X_REG_GLB_STATE, state);
	return state;
}

static void aw862x_play_go(aw_bool flag)
{
	AW_LOGI("enter");
	if (flag) {
		haptic_nv_i2c_write_bits(AW862X_REG_GO, AW862X_BIT_GO_MASK, AW862X_BIT_GO_ENABLE);
		haptic_nv_mdelay(AW_PLAY_DELAY);
	} else {
		haptic_nv_i2c_write_bits(AW862X_REG_GO, AW862X_BIT_GO_MASK, AW862X_BIT_GO_DISABLE);
	}
	aw862x_get_glb_state();
}

static void aw862x_set_f0_preset(uint32_t f0_pre)
{
	uint32_t f0_reg = 0;
	uint8_t reg_array[2] = {0};

	f0_reg = 1000000000 / (f0_pre * g_haptic_nv->info->f0_coeff);
	reg_array[0] = (uint8_t)((f0_reg >> 8) & 0xff);
	reg_array[1] = (uint8_t)((f0_reg >> 0) & 0xff);
	haptic_nv_i2c_writes(AW862X_REG_F_PRE_H, reg_array, AW_I2C_BYTE_TWO);
}

static int aw862x_stop_delay(void)
{
	uint8_t reg_val = 0;
	int cnt = 100;

	while (cnt--) {
		haptic_nv_i2c_reads(AW862X_REG_GLB_STATE, &reg_val, AW_I2C_BYTE_ONE);
		if ((reg_val & AW_BIT_GLBRD_STATE_MASK) == AW_BIT_STATE_STANDBY) {
			AW_LOGI("enter standby,reg glb_state=0x%02X", reg_val);
			return AW_SUCCESS;
		}
		haptic_nv_mdelay(AW_STOP_DELAY);
		AW_LOGI("wait for standby,reg glb_state=0x%02X", reg_val);
	}
	AW_LOGE("do not enter standby automatically");
	return AW_ERROR;
}

static void aw862x_play_stop()
{
	AW_LOGI("enter");
	aw862x_play_go(AW_FALSE);
	aw862x_stop_delay();
	aw862x_play_mode(AW_STANDBY_MODE);
}

static int aw862x_get_f0(void)
{
	uint8_t i = 0;
	uint8_t reg_val = 0;
	uint8_t reg_array[3] = {0};
	uint8_t f0_pre_num = 0;
	uint8_t f0_wait_num = 0;
	uint8_t f0_repeat_num = 0;
	uint8_t f0_trace_num = 0;
	uint32_t t_f0_ms = 0;
	uint32_t t_f0_trace_ms = 0;
	uint32_t f0_cali_cnt = 50;
	int ret = 0;

	AW_LOGI("enter");
	/* f0 calibrate work mode */
	aw862x_play_stop();
	aw862x_play_mode(AW_CONT_MODE);
	haptic_nv_i2c_write_bits(AW862X_REG_CONT_CTRL,
			   (AW862X_BIT_CONT_CTRL_EN_CLOSE_MASK & AW862X_BIT_CONT_CTRL_F0_DETECT_MASK),
			   (AW862X_BIT_CONT_CTRL_OPEN_PLAYBACK | AW862X_BIT_CONT_CTRL_F0_DETECT_ENABLE));
	/* LPF */
	haptic_nv_i2c_write_bits(AW862X_REG_DATCTRL,
			   (AW862X_BIT_DATCTRL_FC_MASK & AW862X_BIT_DATCTRL_LPF_ENABLE_MASK),
			   (AW862X_BIT_DATCTRL_FC_1000HZ | AW862X_BIT_DATCTRL_LPF_ENABLE));
	/* preset f0 */
	aw862x_set_f0_preset(g_haptic_nv->f0_pre);
	/* f0 driver level */
	reg_val = (uint8_t)(g_haptic_nv->info->cont_drv1_lvl);
	haptic_nv_i2c_writes(AW862X_REG_DRV_LVL, &reg_val, AW_I2C_BYTE_ONE);
	/* f0 trace parameter */
	if (!g_haptic_nv->f0_pre) {
		AW_LOGE("fail to get t_f0_ms");
		return AW_ERROR;
	}
	f0_pre_num = g_haptic_nv->info->f0_trace_parameter[0];
	f0_wait_num = g_haptic_nv->info->f0_trace_parameter[1];
	f0_repeat_num = g_haptic_nv->info->f0_trace_parameter[2];
	f0_trace_num = g_haptic_nv->info->f0_trace_parameter[3];
	reg_array[0] = (f0_pre_num << 4) | (f0_wait_num << 0);
	reg_array[1] = f0_repeat_num << 0;
	reg_array[2] = f0_trace_num << 0;
	haptic_nv_i2c_writes(AW862X_REG_NUM_F0_1, reg_array, AW_I2C_BYTE_THREE);
	/* clear aw862x interrupt */
	ret = haptic_nv_i2c_reads(AW862X_REG_SYSINT, &reg_val, AW_I2C_BYTE_ONE);
	/* play go and start f0 calibration */
	aw862x_play_go(AW_TRUE);
	/* f0 trace time */
	t_f0_ms = 1000 * 10 / g_haptic_nv->f0_pre;
	t_f0_trace_ms = t_f0_ms * (f0_pre_num + f0_wait_num +
					(f0_trace_num + f0_wait_num) * (f0_repeat_num - 1));
	AW_LOGI("t_f0_trace_ms = %ldms", t_f0_trace_ms);
	haptic_nv_mdelay(t_f0_trace_ms);
	for (i = 0; i < f0_cali_cnt; i++) {
		ret = haptic_nv_i2c_reads(AW862X_REG_GLB_STATE, &reg_val, AW_I2C_BYTE_ONE);
		/* f0 calibrate done */
		if (reg_val == AW862X_BIT_GLBRD5_STATE_STANDBY) {
			aw862x_read_f0();
			aw862x_read_beme();
			break;
		}
		haptic_nv_mdelay(AW_F0_DELAY);
		AW_LOGI("f0 cali sleep 10ms,glb_state=0x%02X", reg_val);
	}

	if (i == f0_cali_cnt)
		ret = AW_ERROR;
	else
		ret = AW_SUCCESS;
	haptic_nv_i2c_write_bits(AW862X_REG_CONT_CTRL,
			   (AW862X_BIT_CONT_CTRL_EN_CLOSE_MASK & AW862X_BIT_CONT_CTRL_F0_DETECT_MASK),
			   (AW862X_BIT_CONT_CTRL_CLOSE_PLAYBACK | AW862X_BIT_CONT_CTRL_F0_DETECT_DISABLE));

	return ret;
}

#ifdef AW862X_MUL_GET_F0
static int aw862x_multiple_get_f0(void)
{
	int i = 0;
	int ret = 0;
	int f0_max = g_haptic_nv->info->f0_pre + AW862X_MUL_GET_F0_RANGE;
	int f0_min = g_haptic_nv->info->f0_pre - AW862X_MUL_GET_F0_RANGE;

	g_haptic_nv->f0_pre = g_haptic_nv->info->f0_pre;
	for (i = 0; i < AW862X_MUL_GET_F0_NUM; i++) {
		AW_LOGI("f0_pre=%d", g_haptic_nv->f0_pre);
		ret = aw862x_get_f0();
		if (ret)
			return ret;
		if (g_haptic_nv->f0 >= f0_max || g_haptic_nv->f0 <= f0_min)
			break;
		g_haptic_nv->f0_pre = g_haptic_nv->f0;
		haptic_nv_mdelay(4);
	}
	return AW_SUCCESS;
}
#endif

static void aw862x_calculate_cali_data(void)
{
	char f0_cali_lra = 0;
	int f0_cali_step = 0;
	uint8_t chipid_flag = g_haptic_nv->chipid_flag;

	f0_cali_step = 100000 * ((int)g_haptic_nv->f0 - (int)g_haptic_nv->info->f0_pre) /
				   ((int)g_haptic_nv->info->f0_pre * AW862X_F0_CALI_ACCURACY);
	if (f0_cali_step >= 0) {
		if (f0_cali_step % 10 >= 5) {
			f0_cali_step = f0_cali_step / 10 + 1 + (chipid_flag == 1 ? 32 : 16);
		} else {
			f0_cali_step = f0_cali_step / 10 + (chipid_flag == 1 ? 32 : 16);
		}
	} else {
		if (f0_cali_step % 10 <= -5) {
			f0_cali_step = (chipid_flag == 1 ? 32 : 16) + (f0_cali_step / 10 - 1);
		} else {
			f0_cali_step = (chipid_flag == 1 ? 32 : 16) + f0_cali_step / 10;
		}
	}
	if (chipid_flag == 1) {
		if (f0_cali_step > 31)
			f0_cali_lra = (char)f0_cali_step - 32;
		else
			f0_cali_lra = (char)f0_cali_step + 32;
	} else {
		if (f0_cali_step < 16 || (f0_cali_step > 31 && f0_cali_step < 48)) {
			f0_cali_lra = (char)f0_cali_step + 16;
		} else {
			f0_cali_lra = (char)f0_cali_step - 16;
		}
	}
	if ((chipid_flag != 1) && (g_haptic_nv->f0 <= g_haptic_nv->info->f0_pre * 97 / 100))
			f0_cali_lra = 16;
	g_haptic_nv->f0_cali_data = (int)f0_cali_lra;
	AW_LOGI("f0_cali_data=0x%02X", g_haptic_nv->f0_cali_data);
}

static void aw862x_set_base_addr(void)
{
	uint8_t val[2] = {0};
	uint32_t base_addr = g_haptic_nv->ram.base_addr;

	val[0] = (uint8_t)AW_SET_BASEADDR_H(base_addr);
	val[1] = (uint8_t)AW_SET_BASEADDR_L(base_addr);
	haptic_nv_i2c_writes(AW862X_REG_BASE_ADDRH, val, AW_I2C_BYTE_TWO);
}

static void aw862x_set_fifo_addr(void)
{
	uint8_t val[4] = {0};
	uint32_t base_addr = g_haptic_nv->ram.base_addr;

	val[0] = (uint8_t)AW862X_SET_AEADDR_H(base_addr);
	val[1] = (uint8_t)AW862X_SET_AEADDR_L(base_addr);
	val[2] = (uint8_t)AW862X_SET_AFADDR_H(base_addr);
	val[3] = (uint8_t)AW862X_SET_AFADDR_L(base_addr);
	haptic_nv_i2c_writes(AW862X_REG_FIFO_AEH, val, AW_I2C_BYTE_FOUR);
}

static void aw862x_get_fifo_addr(void)
{
	uint8_t val[4] = {0};

	haptic_nv_i2c_reads(AW862X_REG_FIFO_AEH, val, AW_I2C_BYTE_FOUR);
	AW_LOGI("almost_empty_threshold = %d, almost_full_threshold = %d",
			(uint16_t)((val[0] << 8) | val[1]),
			(uint16_t)((val[2] << 8) | val[3]));
}

static void aw862x_set_ram_addr(void)
{
	uint8_t val[2] = {0};
	uint32_t base_addr = g_haptic_nv->ram.base_addr;

	val[0] = (uint8_t)AW_SET_RAMADDR_H(base_addr);
	val[1] = (uint8_t)AW_SET_RAMADDR_L(base_addr);
	haptic_nv_i2c_writes(AW862X_REG_RAMADDRH, val, AW_I2C_BYTE_TWO);
}

static void aw862x_set_ram_data(uint8_t *data, int len)
{
	haptic_nv_i2c_writes(AW862X_REG_RAMDATA, data, len);
}

static void aw862x_get_ram_data(uint8_t *ram_data, int size)
{
	haptic_nv_i2c_reads(AW862X_REG_RAMDATA, ram_data, size);
}

static void aw862x_interrupt_setup(void)
{
	uint8_t reg_val = 0;

	haptic_nv_i2c_reads(AW862X_REG_SYSINT, &reg_val, AW_I2C_BYTE_ONE);
	AW_LOGI("reg SYSINT=0x%X", reg_val);

	haptic_nv_i2c_write_bits(AW862X_REG_DBGCTRL, AW862X_BIT_DBGCTRL_INT_MODE_MASK,
							 AW862X_BIT_DBGCTRL_INT_MODE_EDGE);

	haptic_nv_i2c_write_bits(AW862X_REG_SYSINTM,
			   (AW862X_BIT_SYSINTM_OV_MASK & AW862X_BIT_SYSINTM_UVLO_MASK &
				AW862X_BIT_SYSINTM_FF_AE_MASK & AW862X_BIT_SYSINTM_FF_AF_MASK &
				AW862X_BIT_SYSINTM_OCD_MASK &  AW862X_BIT_SYSINTM_OT_MASK &
				AW862X_BIT_SYSINTM_DONE_MASK),
			   (AW862X_BIT_SYSINTM_OV_OFF | AW862X_BIT_SYSINTM_UVLO_EN |
				AW862X_BIT_SYSINTM_FF_AE_OFF | AW862X_BIT_SYSINTM_FF_AF_OFF |
				AW862X_BIT_SYSINTM_OCD_EN | AW862X_BIT_SYSINTM_OT_EN |
				AW862X_BIT_SYSINTM_DONE_OFF));
}

static void aw862x_haptic_select_pin(uint8_t pin)
{
	if (pin == AW_TRIG1) {
		haptic_nv_i2c_write_bits(AW862X_REG_DBGCTRL, AW862X_BIT_DBGCTRL_INTN_TRG_SEL_MASK,
								 AW862X_BIT_DBGCTRL_TRG_SEL_ENABLE);
		haptic_nv_i2c_write_bits(AW862X_REG_TRG_CFG2, AW862X_BIT_TRGCFG2_TRG1_ENABLE_MASK,
								 AW862X_BIT_TRGCFG2_TRG1_ENABLE);
		AW_LOGI("select TRIG1 pin");
	} else if (pin == AW_IRQ) {
		haptic_nv_i2c_write_bits(AW862X_REG_DBGCTRL, AW862X_BIT_DBGCTRL_INTN_TRG_SEL_MASK,
								 AW862X_BIT_DBGCTRL_INTN_SEL_ENABLE);
		haptic_nv_i2c_write_bits(AW862X_REG_TRG_CFG2, AW862X_BIT_TRGCFG2_TRG1_ENABLE_MASK,
								 AW862X_BIT_TRGCFG2_TRG1_DISABLE);
		AW_LOGI("select INIT pin");
	} else
		AW_LOGE("There is no such option");
}

static void aw862x_haptic_trig1_param_init()
{
	g_haptic_nv->trig[0] = &aw862x_trig1;
}

static void aw862x_haptic_trig1_param_config(void)
{
	if (!(g_haptic_nv->trig[0]->enable) || (g_haptic_nv->is_used_irq_pin &&
		  g_haptic_nv->name == AW8624)) {
		aw862x_haptic_trig1_param_init();
		if (g_haptic_nv->name == AW8624)
			aw862x_haptic_select_pin(AW_IRQ);

		return;
	}
	aw862x_haptic_select_pin(AW_TRIG1);
	haptic_nv_i2c_write_bits(AW862X_REG_TRG_CFG1, AW862X_BIT_TRGCFG1_TRG1_EDGE_MASK,
							 g_haptic_nv->trig[0]->trig_edge);
	haptic_nv_i2c_write_bits(AW862X_REG_TRG_CFG1, AW862X_BIT_TRGCFG1_TRG1_POLAR_MASK,
							 g_haptic_nv->trig[0]->trig_polar << 1);
	haptic_nv_i2c_writes(AW862X_REG_TRG1_SEQP, &g_haptic_nv->trig[0]->pos_sequence, AW_I2C_BYTE_ONE);
	haptic_nv_i2c_writes(AW862X_REG_TRG1_SEQN, &g_haptic_nv->trig[0]->neg_sequence, AW_I2C_BYTE_ONE);
}

static void aw862x_haptic_set_trig1()
{
	aw862x_haptic_trig1_param_init();
	aw862x_haptic_trig1_param_config();
}

static void aw862x_trig_init(void)
{
	if (g_haptic_nv->is_used_irq_pin && g_haptic_nv->name == AW8624) {
		aw862x_haptic_select_pin(AW_IRQ);
		return;
	}
	AW_LOGI("enter");
	aw862x_haptic_set_trig1();
}

static void aw862x_set_gain(uint8_t gain)
{
	g_haptic_nv->gain = gain;
	haptic_nv_i2c_writes(AW862X_REG_DATDBG, &gain, AW_I2C_BYTE_ONE);
}

static void aw862x_get_vbat(void)
{
	uint8_t reg_val = 0;

	aw862x_play_stop();
	/*step 1:EN_RAMINIT*/
	aw862x_raminit(AW_TRUE);
	/*step 2 :launch power supply testing */
	haptic_nv_i2c_write_bits(AW862X_REG_DETCTRL, AW862X_BIT_DETCTRL_VBAT_GO_MASK,
							 AW862X_BIT_DETCTRL_VABT_GO_ENABLE);
	haptic_nv_mdelay(AW_VBAT_DELAY);
	haptic_nv_i2c_reads(AW862X_REG_VBATDET, &reg_val, AW_I2C_BYTE_ONE);
	g_haptic_nv->vbat = AW862X_VBAT_FORMULA(reg_val);
	AW_LOGI("get_vbat=%ldmV, vbat_code=0x%02X", g_haptic_nv->vbat, reg_val);
	/*step 3: return val*/
	aw862x_raminit(AW_FALSE);
}

static void aw862x_set_wav_seq(uint8_t wav, uint8_t seq)
{
	haptic_nv_i2c_writes(AW862X_REG_WAVSEQ1 + wav, &seq, AW_I2C_BYTE_ONE);
}

static void aw862x_set_wav_loop(uint8_t wav, uint8_t loop)
{
	uint8_t tmp = 0;

	if (wav % 2) {
		tmp = loop << 0;
		haptic_nv_i2c_write_bits(AW862X_REG_WAVLOOP1 + (wav / 2),
								 AW862X_BIT_WAVLOOP_SEQNP1_MASK, tmp);
	} else {
		tmp = loop << 4;
		haptic_nv_i2c_write_bits(AW862X_REG_WAVLOOP1 + (wav / 2),
								 AW862X_BIT_WAVLOOP_SEQN_MASK, tmp);
	}

}

static void aw862x_haptic_start(void)
{
	aw862x_haptic_active();
	aw862x_play_go(AW_TRUE);
}

static uint8_t aw862x_rtp_get_fifo_afs(void)
{
	uint8_t reg_val = 0;

	haptic_nv_i2c_reads(AW862X_REG_SYSST, &reg_val, AW_I2C_BYTE_ONE);
	reg_val &= AW862X_BIT_SYSST_FF_AFS;
	reg_val = reg_val >> 3;
	return reg_val;
}

static void aw862x_set_rtp_aei(aw_bool flag)
{
	if (flag) {
		haptic_nv_i2c_write_bits(AW862X_REG_SYSINTM, AW862X_BIT_SYSINTM_FF_AE_MASK,
								 AW862X_BIT_SYSINTM_FF_AE_EN);
	} else {
		haptic_nv_i2c_write_bits(AW862X_REG_SYSINTM, AW862X_BIT_SYSINTM_FF_AE_MASK,
								 AW862X_BIT_SYSINTM_FF_AE_OFF);
	}
}

static int aw862x_get_irq_state(void)
{
	uint8_t reg_val = 0;
	uint8_t glb_st = 0;
	int ret = AW_IRQ_NULL;

	haptic_nv_i2c_reads(AW862X_REG_SYSINT, &reg_val, AW_I2C_BYTE_ONE);
	AW_LOGI("reg SYSINT=0x%02X", reg_val);
	if (reg_val & AW862X_BIT_SYSINT_OCDI) {
		AW_LOGE("chip over current int error");
		ret = AW_IRQ_OCD;
	}
	if (reg_val & AW862X_BIT_SYSINT_OTI) {
		AW_LOGE("chip over temperature int error");
		ret = AW_IRQ_OT;
	}
	if (reg_val & AW862X_BIT_SYSINT_DONEI) {
		AW_LOGI("chip playback done");
		ret = AW_IRQ_DONE;
	}
	if (reg_val & AW862X_BIT_SYSINT_UVLI) {
		AW_LOGE("chip uvlo int error");
		ret = AW_IRQ_UVL;
		haptic_nv_i2c_reads(AW862X_REG_GLB_STATE, &glb_st, AW_I2C_BYTE_ONE);
		if (glb_st == 0) {
			haptic_nv_i2c_write_bits(AW862X_REG_SYSINTM, AW862X_BIT_SYSINTM_UVLO_MASK,
									 AW862X_BIT_SYSINTM_UVLO_OFF);
		}
	}
	if (reg_val & AW862X_BIT_SYSINT_FF_AFI) {
		AW_LOGI("rtp mode fifo full");
		ret = AW_IRQ_ALMOST_FULL;
	}
	if (reg_val & AW862X_BIT_SYSINT_FF_AEI) {
		AW_LOGI("rtp fifo almost empty");
		ret = AW_IRQ_ALMOST_EMPTY;
	}
	return ret;
}

static void aw862x_irq_clear(void)
{
	uint8_t reg_val = 0;

	haptic_nv_i2c_reads(AW862X_REG_SYSINT, &reg_val, AW_I2C_BYTE_ONE);
	AW_LOGI("SYSINT=0x%02X", reg_val);
}

static uint8_t aw862x_judge_rtp_going(void)
{
	uint8_t glb_st = 0;

	haptic_nv_i2c_reads(AW862X_REG_GLB_STATE, &glb_st, AW_I2C_BYTE_ONE);
	if (((glb_st & AW_BIT_GLBRD_STATE_MASK) == AW_BIT_STATE_RTP_GO)) {
		return AW_SUCCESS;
	}
	return glb_st;
}

static void aw862x_get_lra_resistance(void)
{
	uint8_t reg_val = 0;
	uint8_t anactrl = 0;
	uint8_t d2scfg = 0;

	haptic_nv_i2c_reads(AW862X_REG_ANACTRL, &anactrl, AW_I2C_BYTE_ONE);
	haptic_nv_i2c_reads(AW862X_REG_D2SCFG, &d2scfg, AW_I2C_BYTE_ONE);
	aw862x_play_stop();
	aw862x_raminit(AW_TRUE);
	haptic_nv_i2c_write_bits(AW862X_REG_ANACTRL, AW862X_BIT_ANACTRL_EN_IO_PD1_MASK,
							 AW862X_BIT_ANACTRL_EN_IO_PD1_HIGH);
	haptic_nv_i2c_write_bits(AW862X_REG_D2SCFG, AW862X_BIT_D2SCFG_CLK_ADC_MASK,
							 AW862X_BIT_D2SCFG_CLK_ASC_1P5MHZ);
	haptic_nv_i2c_write_bits(AW862X_REG_DETCTRL,
			   (AW862X_BIT_DETCTRL_RL_OS_MASK & AW862X_BIT_DETCTRL_DIAG_GO_MASK),
			   (AW862X_BIT_DETCTRL_RL_DETECT | AW862X_BIT_DETCTRL_DIAG_GO_ENABLE));
	haptic_nv_mdelay(AW_RL_DELAY);
	haptic_nv_i2c_reads(AW862X_REG_RLDET, &reg_val, AW_I2C_BYTE_ONE);
	g_haptic_nv->lra = AW862X_RL_FORMULA(reg_val);
	haptic_nv_i2c_writes(AW862X_REG_D2SCFG, &d2scfg, AW_I2C_BYTE_ONE);
	haptic_nv_i2c_writes(AW862X_REG_ANACTRL, &anactrl, AW_I2C_BYTE_ONE);
	aw862x_raminit(AW_FALSE);
	AW_LOGI("res=%ld", g_haptic_nv->lra);
}

static void aw862x_read_reg_array(uint8_t head_reg_addr, uint8_t tail_reg_addr)
{
	int i = 0;
	int ret = 0;
	int reg_num = 0;
	uint8_t reg_array[AW_REG_MAX] = {0};

	reg_num = tail_reg_addr - head_reg_addr + 1;

	ret = haptic_nv_i2c_reads(head_reg_addr, reg_array, reg_num);
	if (ret == AW_ERROR) {
		AW_LOGE("read reg:0x%02X ~ 0x%02X is failed.", head_reg_addr, tail_reg_addr);
		return;
	}

	for (i = 0 ; i < reg_num; i++)
		AW_LOGI("reg:0x%02X=0x%02X\r\n", head_reg_addr + i, reg_array[i]);
}

static void aw862x_get_reg(void)
{
	aw862x_read_reg_array(AW862X_REG_ID, AW862X_REG_RTP_DATA - 1);
	aw862x_read_reg_array(AW862X_REG_RTP_DATA + 1, AW862X_REG_RAMDATA - 1);
	aw862x_read_reg_array(AW862X_REG_RAMDATA + 1, AW862X_REG_NUM_F0_3);
}

static void aw862x_set_rtp_autosin(uint8_t flag)
{
	AW_LOGI("unsupport rtp_autosin mode\n");
}

static void aw862x_set_rtp_data(uint8_t *data, uint32_t len)
{
	haptic_nv_i2c_writes(AW862X_REG_RTP_DATA, data, len);
}

static void aw862x_set_repeat_seq(uint8_t seq)
{
	aw862x_set_wav_seq(0x00, seq);
	aw862x_set_wav_seq(0x01, 0x00);
	aw862x_set_wav_loop(0x00, AW862X_BIT_WAVLOOP_INIFINITELY);
}

static void aw862x_cont_config(void)
{
	uint8_t brake0_level = 0;
	uint8_t time_nzc = 0;
	uint8_t en_brake1 = 0;
	uint8_t brake1_level = 0;
	uint8_t en_brake2 = 0;
	uint8_t brake2_level = 0;
	uint8_t brake2_num = 0;
	uint8_t brake1_num = 0;
	uint8_t brake0_num = 0;
	uint8_t val[4] = {0};

	AW_LOGI("enter");
	aw862x_haptic_active();
	aw862x_play_mode(AW_CONT_MODE);
	/* preset f0 */
	if (g_haptic_nv->f0 <= 0)
		aw862x_set_f0_preset(g_haptic_nv->info->f0_pre);
	else
		aw862x_set_f0_preset(g_haptic_nv->f0);
	/* lpf */
	haptic_nv_i2c_write_bits(AW862X_REG_DATCTRL,
			   (AW862X_BIT_DATCTRL_FC_MASK & AW862X_BIT_DATCTRL_LPF_ENABLE_MASK),
			   (AW862X_BIT_DATCTRL_FC_1000HZ | AW862X_BIT_DATCTRL_LPF_ENABLE));
	/* brake */
	en_brake1 = g_haptic_nv->info->cont_brake[0];
	en_brake2 = g_haptic_nv->info->cont_brake[1];
	brake0_level = g_haptic_nv->info->cont_brake[2];
	brake1_level = g_haptic_nv->info->cont_brake[3];
	brake2_level = g_haptic_nv->info->cont_brake[4];
	brake0_num = g_haptic_nv->info->cont_brake[5];
	brake1_num = g_haptic_nv->info->cont_brake[6];
	brake2_num = g_haptic_nv->info->cont_brake[7];
	val[0] = brake0_level << 0;
	val[1] = (en_brake1 << 7)|(brake1_level << 0);
	val[2] = (en_brake2 << 7)|(brake2_level << 0);
	val[3] = (brake2_num << 6) | (brake1_num << 3) | (brake0_num << 0);
	haptic_nv_i2c_writes(AW862X_REG_BRAKE0_CTRL, val, AW_I2C_BYTE_FOUR);
	/* cont config */
	val[0] = AW862X_BIT_CONT_CTRL_ZC_DETEC_ENABLE |
			 AW862X_BIT_CONT_CTRL_WAIT_1PERIOD |
			 AW862X_BIT_CONT_CTRL_BY_GO_SIGNAL |
			 AW862X_BIT_CONT_CTRL_CLOSE_PLAYBACK |
			 AW862X_BIT_CONT_CTRL_F0_DETECT_DISABLE |
			 AW862X_BIT_CONT_CTRL_O2C_DISABLE;
	haptic_nv_i2c_writes(AW862X_REG_CONT_CTRL, val, AW_I2C_BYTE_ONE);
	/* TD time */
	val[0] = g_haptic_nv->info->cont_td>>8;
	val[1] = g_haptic_nv->info->cont_td>>0;
	haptic_nv_i2c_writes(AW862X_REG_TD_H, val, AW_I2C_BYTE_TWO);
	haptic_nv_i2c_write_bits(AW862X_REG_BEMF_NUM, AW862X_BIT_BEMF_NUM_BRK_MASK,
						 g_haptic_nv->info->cont_num_brk);
	time_nzc = AW862X_BIT_TIME_NZC_DEF_VAL;
	haptic_nv_i2c_writes(AW862X_REG_TIME_NZC, &time_nzc, AW_I2C_BYTE_ONE);
	/* f0 driver level */
	val[0] = g_haptic_nv->info->cont_drv1_lvl;
	val[1] = g_haptic_nv->info->cont_drv2_lvl;
	haptic_nv_i2c_writes(AW862X_REG_DRV_LVL, val, AW_I2C_BYTE_TWO);
	aw862x_play_go(AW_TRUE);
}

struct aw_haptic_func aw862x_func_list = {
	.ram_init = aw862x_raminit,
	.trig_init = aw862x_trig_init,
	.play_mode = aw862x_play_mode,
	.play_stop = aw862x_play_stop,
	.irq_clear = aw862x_irq_clear,
	.cont_config = aw862x_cont_config,
	.offset_cali = aw862x_offset_cali,
	.haptic_start = aw862x_haptic_start,
	.check_qualify = aw862x_check_qualify,
	.judge_rtp_going = aw862x_judge_rtp_going,
	.protect_config = aw862x_protect_config,
	.misc_para_init = aw862x_misc_para_init,
	.interrupt_setup = aw862x_interrupt_setup,
	.rtp_get_fifo_afs = aw862x_rtp_get_fifo_afs,
	.vbat_mode_config = aw862x_vbat_mode_config,
	.calculate_cali_data = aw862x_calculate_cali_data,
	.set_gain = aw862x_set_gain,
	.set_wav_seq = aw862x_set_wav_seq,
	.set_wav_loop = aw862x_set_wav_loop,
	.set_ram_data = aw862x_set_ram_data,
	.get_ram_data = aw862x_get_ram_data,
	.set_fifo_addr = aw862x_set_fifo_addr,
	.get_fifo_addr = aw862x_get_fifo_addr,
	.set_rtp_aei = aw862x_set_rtp_aei,
	.set_rtp_autosin = aw862x_set_rtp_autosin,
	.set_rtp_data = aw862x_set_rtp_data,
	.set_ram_addr = aw862x_set_ram_addr,
	.set_trim_lra = aw862x_set_trim_lra,
	.set_base_addr = aw862x_set_base_addr,
	.set_repeat_seq = aw862x_set_repeat_seq,
#ifdef AW862X_MUL_GET_F0
	.get_f0 = aw862x_multiple_get_f0,
#else
	.get_f0 = aw862x_get_f0,
#endif
	.get_reg = aw862x_get_reg,
	.get_vbat = aw862x_get_vbat,
	.get_irq_state = aw862x_get_irq_state,
	.get_glb_state = aw862x_get_glb_state,
	.get_lra_resistance = aw862x_get_lra_resistance,
};
