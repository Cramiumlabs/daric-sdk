/*
 * aw8623x.c
 *
 * Author: <wangzhi@awinic.com>
 *
 * Copyright ©2022-2023 awinic.All Rights Reserved
 *
 */
#include "stdio.h"
#include "haptic_nv.h"
#include "haptic_nv_reg.h"

struct aw_haptic_dts_info aw8623x_dts = {
	.f0_pre = 2350,
	.f0_cali_percent = 20,
	.cont_drv1_lvl = 0x7F,
	.lra_vrms = 1800, /* Motor rated voltage, mV */
	.cont_brk_time = 0x06,
	.cont_brk_gain = 0x08,
	.cont_drv1_time = 0x04,
	.cont_drv2_time = 0x14,
	.cont_track_margin = 0x0F,
	.d2s_gain = 0x05,
	.is_enabled_auto_brk = AW_FALSE,
};

static struct trig aw8623x_trig1 = {
	.trig_brk = 0,
	.trig_level = 0,
	.trig_polar = 0,
	.pos_enable = 0,
	.neg_enable = 0,
	.pos_sequence = 1,
	.neg_sequence = 2,
};

static struct trig aw8623x_trig2 = {
	.trig_brk = 0,
	.trig_level = 0,
	.trig_polar = 0,
	.pos_enable = 0,
	.neg_enable = 0,
	.pos_sequence = 1,
	.neg_sequence = 2,
};

static struct trig aw8623x_trig3 = {
	.trig_brk = 0,
	.trig_level = 0,
	.trig_polar = 0,
	.pos_enable = 0,
	.neg_enable = 0,
	.pos_sequence = 1,
	.neg_sequence = 2,
};

static void aw8623x_reg_unlock(aw_bool flag)
{
	uint8_t reg_val = 0;

	if (flag) {
		/* Unlock register */
		reg_val = AW8623X_BIT_TMCFG_TM_UNLOCK;
		haptic_nv_i2c_writes(AW8623X_REG_TRIMCFG5, &reg_val, AW_I2C_BYTE_ONE);
	} else {
		/* Lock register */
		reg_val = AW8623X_BIT_TMCFG_TM_LOCK;
		haptic_nv_i2c_writes(AW8623X_REG_TRIMCFG5, &reg_val, AW_I2C_BYTE_ONE);
	}
}

static int aw8623x_check_qualify(void)
{
	uint8_t reg = 0;
	int ret = AW_SUCCESS;

	ret = haptic_nv_i2c_reads(AW8623X_REG_EFCFG6, &reg, AW_I2C_BYTE_ONE);
	if (ret != AW_SUCCESS)
		return ret;
	if ((reg & 0x80) == 0x80) {
		AW_LOGI("check quailify success.");
		return AW_SUCCESS;
	}
	AW_LOGE("register 0x64 error: 0x%02X", reg);
	return AW_ERROR;
}

static void aw8623x_raminit(aw_bool flag)
{
	if (flag) {
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL1, AW8623X_BIT_SYSCTRL1_RAMINIT_MASK,
								 AW8623X_BIT_SYSCTRL1_RAMINIT_ON);
		haptic_nv_udelay(500);
	} else {
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL1,  AW8623X_BIT_SYSCTRL1_RAMINIT_MASK,
								 AW8623X_BIT_SYSCTRL1_RAMINIT_OFF);
	}
}

static void aw8623x_play_stop(void)
{
	aw_bool force_flag = AW_TRUE;
	uint8_t val = 0;
	int cnt = 40;

	g_haptic_nv->play_mode = AW_STANDBY_MODE;
	val = AW8623X_BIT_PLAYCFG4_GO_ON;
	aw8623x_raminit(AW_TRUE);
	haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_PLAY_MODE_MASK,
				 AW8623X_BIT_PLAYCFG3_PLAY_MODE_STOP);
	haptic_nv_i2c_writes(AW8623X_REG_PLAYCFG4, &val, AW_I2C_BYTE_ONE);
	aw8623x_raminit(AW_FALSE);
	while (cnt) {
		haptic_nv_i2c_reads(AW8623X_REG_GLBRD5, &val, AW_I2C_BYTE_ONE);
		if ((val & AW_BIT_GLBRD_STATE_MASK) == AW_BIT_STATE_STANDBY) {
			force_flag = AW_FALSE;
			AW_LOGI("entered standby! glb_state=0x%02X", val);
			break;
		} else {
			cnt--;
			AW_LOGI("wait for standby, glb_state=0x%02X", val);
		}
		haptic_nv_mdelay(AW_STOP_DELAY);
	}
	if (force_flag) {
		AW_LOGE("force to enter standby mode!");
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL2, AW8623X_BIT_SYSCTRL2_STANDBY_MASK,
							 	 AW8623X_BIT_SYSCTRL2_STANDBY_ON);
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL2, AW8623X_BIT_SYSCTRL2_STANDBY_MASK,
							 	 AW8623X_BIT_SYSCTRL2_STANDBY_OFF);
	}
}

static void aw8623x_sram_size(uint8_t size_flag)
{
	switch (size_flag) {
	case AW8623X_HAPTIC_SRAM_1K:
		haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG1, AW8623X_BIT_RTPCFG1_SRAM_SIZE_2K_MASK,
								 AW8623X_BIT_RTPCFG1_SRAM_SIZE_2K_DIS);
		haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG1, AW8623X_BIT_RTPCFG1_SRAM_SIZE_1K_MASK,
								 AW8623X_BIT_RTPCFG1_SRAM_SIZE_1K_EN);
		break;
	case AW8623X_HAPTIC_SRAM_2K:
		haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG1, AW8623X_BIT_RTPCFG1_SRAM_SIZE_2K_MASK,
								 AW8623X_BIT_RTPCFG1_SRAM_SIZE_2K_EN);
		haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG1, AW8623X_BIT_RTPCFG1_SRAM_SIZE_1K_MASK,
								 AW8623X_BIT_RTPCFG1_SRAM_SIZE_1K_DIS);
		break;
	case AW8623X_HAPTIC_SRAM_3K:
		haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG1, AW8623X_BIT_RTPCFG1_SRAM_SIZE_1K_MASK,
								 AW8623X_BIT_RTPCFG1_SRAM_SIZE_1K_EN);
		haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG1, AW8623X_BIT_RTPCFG1_SRAM_SIZE_2K_MASK,
								 AW8623X_BIT_RTPCFG1_SRAM_SIZE_2K_EN);
		break;
	default:
		AW_LOGE("size_flag is error");
		break;
	}
}

static void aw8623x_auto_brk_config(uint8_t flag)
{
	if (flag) {
		haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_BRK_EN_MASK,
								 AW8623X_BIT_PLAYCFG3_BRK_ENABLE);
	} else {
		haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_BRK_EN_MASK,
								 AW8623X_BIT_PLAYCFG3_BRK_DISABLE);
	}
}

static void aw8623x_set_pwm(uint8_t mode)
{
	switch (mode) {
	case AW_PWM_48K:
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL2, AW8623X_BIT_SYSCTRL2_WAVDAT_MODE_MASK,
								 AW8623X_BIT_SYSCTRL2_RATE_48K);
		break;
	case AW_PWM_24K:
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL2, AW8623X_BIT_SYSCTRL2_WAVDAT_MODE_MASK,
								 AW8623X_BIT_SYSCTRL2_RATE_24K);
		break;
	case AW_PWM_12K:
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL2, AW8623X_BIT_SYSCTRL2_WAVDAT_MODE_MASK,
								 AW8623X_BIT_SYSCTRL2_RATE_12K);
		break;
	default:
		break;
	}
}

static void aw8623x_vbat_ref_config(uint16_t vbat_ref)
{
	uint8_t reg_val = 0;

	AW_LOGI("enter");
	if (vbat_ref <= 3450)
		reg_val = AW8623X_BIT_DETCFG1_VBAT_REF_3_3;
	else if (vbat_ref <= 3800)
		reg_val = AW8623X_BIT_DETCFG1_VBAT_REF_3_6;
	else if (vbat_ref <= 4100)
		reg_val = AW8623X_BIT_DETCFG1_VBAT_REF_4_0;
	else if (vbat_ref <= 4350)
		reg_val = AW8623X_BIT_DETCFG1_VBAT_REF_4_2;
	else if (vbat_ref <= 4700)
		reg_val = AW8623X_BIT_DETCFG1_VBAT_REF_4_5;
	else if (vbat_ref <= 4900)
		reg_val = AW8623X_BIT_DETCFG1_VBAT_REF_4_8;
	else if (vbat_ref <= 5250)
		reg_val = AW8623X_BIT_DETCFG1_VBAT_REF_5_0;
	else
		reg_val = AW8623X_BIT_DETCFG1_VBAT_REF_5_5;

	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1, AW8623X_BIT_DETCFG1_VBAT_REF_MASK, reg_val);
}

static void aw8623x_vbat_mode_config(uint8_t flag)
{
	AW_LOGI("enter");
	if (flag == AW_CONT_VBAT_HW_COMP_MODE) {
		haptic_nv_i2c_write_bits(AW8623X_REG_VBATCTRL,
					 AW8623X_BIT_VBATCTRL_VBAT_MODE_MASK,
					 AW8623X_BIT_VBATCTRL_VBAT_MODE_HW);
	} else {
		haptic_nv_i2c_write_bits(AW8623X_REG_VBATCTRL,
					 AW8623X_BIT_VBATCTRL_VBAT_MODE_MASK,
					 AW8623X_BIT_VBATCTRL_VBAT_MODE_SW);
	}
}

static void aw8623x_play_mode(uint8_t play_mode)
{
	switch (play_mode) {
	case AW_STANDBY_MODE:
		AW_LOGI("enter standby mode");
		g_haptic_nv->play_mode = AW_STANDBY_MODE;
		aw8623x_play_stop();
		break;
	case AW_RAM_MODE:
		AW_LOGI("enter ram mode");
		g_haptic_nv->play_mode = AW_RAM_MODE;
		aw8623x_set_pwm(AW_PWM_12K);
		aw8623x_auto_brk_config(AW_FALSE);
		haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_PLAY_MODE_MASK,
								 AW8623X_BIT_PLAYCFG3_PLAY_MODE_RAM);
		aw8623x_vbat_mode_config(AW_CONT_VBAT_SW_COMP_MODE);
		break;
	case AW_RAM_LOOP_MODE:
		AW_LOGI("enter ram loop mode");
		g_haptic_nv->play_mode = AW_RAM_LOOP_MODE;
		aw8623x_set_pwm(AW_PWM_12K);
		aw8623x_auto_brk_config(AW_TRUE);
		haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_PLAY_MODE_MASK,
								 AW8623X_BIT_PLAYCFG3_PLAY_MODE_RAM);
		aw8623x_vbat_mode_config(AW_CONT_VBAT_HW_COMP_MODE);
		break;
	case AW_CONT_MODE:
		AW_LOGI("enter cont mode");
		g_haptic_nv->play_mode = AW_CONT_MODE;
		aw8623x_auto_brk_config(g_haptic_nv->info->is_enabled_auto_brk);
		haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_PLAY_MODE_MASK,
								 AW8623X_BIT_PLAYCFG3_PLAY_MODE_CONT);
		aw8623x_vbat_mode_config(AW_CONT_VBAT_HW_COMP_MODE);
		break;
	case AW_RTP_MODE:
		AW_LOGI("enter rtp mode");
		g_haptic_nv->play_mode = AW_RTP_MODE;
		aw8623x_set_pwm(AW_PWM_24K);
		aw8623x_auto_brk_config(AW_TRUE);
		haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_PLAY_MODE_MASK,
								 AW8623X_BIT_PLAYCFG3_PLAY_MODE_RTP);
		aw8623x_vbat_mode_config(AW_CONT_VBAT_SW_COMP_MODE);
		break;
	case AW_TRIG_MODE:
		AW_LOGI("enter trig mode");
		g_haptic_nv->play_mode = AW_TRIG_MODE;
		aw8623x_set_pwm(AW_PWM_12K);
		haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_PLAY_MODE_MASK,
								 AW8623X_BIT_PLAYCFG3_PLAY_MODE_RAM);
		aw8623x_vbat_mode_config(AW_CONT_VBAT_SW_COMP_MODE);
		break;
	default:
		AW_LOGE("play mode %d error", play_mode);
		break;
	}
}

static void aw8623x_protect_config(uint8_t prtime,uint8_t prlvl)
{
	uint8_t reg_val = 0;

	haptic_nv_i2c_write_bits(AW8623X_REG_PWMCFG1,
				 AW8623X_BIT_PWMCFG1_PRC_EN_MASK,
				 AW8623X_BIT_PWMCFG1_PRC_DISABLE);
	if (prlvl != 0) {
		/* Enable protection mode */
		AW_LOGI("enable protection mode");
		reg_val = AW8623X_BIT_PWMCFG3_PR_ENABLE |
			  (prlvl & (~AW8623X_BIT_PWMCFG3_PRLVL_MASK));
		haptic_nv_i2c_writes(AW8623X_REG_PWMCFG3, &reg_val,
				     AW_I2C_BYTE_ONE);
		haptic_nv_i2c_writes(AW8623X_REG_PWMCFG4, &prtime,
				     AW_I2C_BYTE_ONE);
	} else {
		/* Disable */
		AW_LOGI("disable protection mode");
		haptic_nv_i2c_write_bits(AW8623X_REG_PWMCFG3,
					 AW8623X_BIT_PWMCFG3_PR_EN_MASK,
					 AW8623X_BIT_PWMCFG3_PR_DISABLE);
	}
}

static void aw8623x_misc_para_init(void)
{
	uint8_t val = 0;

	AW_LOGI("enter");
	/* GAIN_BYPASS config */
	haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL1,
				 AW8623X_BIT_SYSCTRL1_GAIN_BYPASS_MASK,
				 AW8623X_BIT_SYSCTRL1_GAIN_CHANGE_ENABLE);
	/* Open the input OVP function of ADC */
	haptic_nv_i2c_write_bits(AW8623X_REG_ANACFG3,
				 AW8623X_BIT_ANACFG3_EN_OVP_SAR_MASK,
				 AW8623X_BIT_ANACFG3_EN_OVP_SAR_ENABLE);
	haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL1,
				 AW8623X_BIT_SYSCTRL1_TRIG1_MODE_MASK,
				 AW8623X_BIT_SYSCTRL1_TRIG1_TRIG);
	aw8623x_reg_unlock(AW_TRUE);
	/* uvlo adj default 2.4V */
	haptic_nv_i2c_write_bits(AW8623X_REG_ANACFG5,
				AW8623X_BIT_ANACFG5_UVLO_ADJ_MASK,
				AW8623X_BIT_ANACFG5_UVLO_ADJ_2_4_V);
	aw8623x_reg_unlock(AW_FALSE);

	if (g_haptic_nv->info->cont_brk_time) {
		val = (uint8_t)g_haptic_nv->info->cont_brk_time;
		haptic_nv_i2c_writes(AW8623X_REG_CONTCFG10,
				     &val, AW_I2C_BYTE_ONE);
	}

	if (g_haptic_nv->info->cont_brk_gain) {
		haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG5,
					 AW8623X_BIT_CONTCFG5_BRK_GAIN_MASK,
					 g_haptic_nv->info->cont_brk_gain);
	}

	g_haptic_nv->info->cont_drv2_lvl = AW8623X_DRV2_LVL_FORMULA(g_haptic_nv->info->f0_pre, g_haptic_nv->info->lra_vrms);
	if (g_haptic_nv->info->cont_drv2_lvl > AW8623X_DRV2_LVL_MAX)
		g_haptic_nv->info->cont_drv2_lvl = AW8623X_DRV2_LVL_MAX;
	AW_LOGI("lra_vrms=%ld, cont_drv2_lvl=0x%02X", g_haptic_nv->info->lra_vrms,
			g_haptic_nv->info->cont_drv2_lvl);

	aw8623x_vbat_ref_config(AW_VBAT_REFER);
	aw8623x_sram_size(AW8623X_HAPTIC_SRAM_3K);
	aw8623x_set_pwm(AW_PWM_12K);
	aw8623x_protect_config(AW8623X_PWMCFG4_PRTIME_DEFAULT_VALUE,
			       AW8623X_BIT_PWMCFG3_PRLVL_DEFAULT_VALUE);
}

static int aw8623x_select_d2s_gain(uint8_t reg)
{
	int d2s_gain = 0;

	switch (reg) {
	case AW8623X_BIT_DETCFG2_D2S_GAIN_1:
		d2s_gain = 1;
		break;
	case AW8623X_BIT_DETCFG2_D2S_GAIN_2:
		d2s_gain = 2;
		break;
	case AW8623X_BIT_DETCFG2_D2S_GAIN_4:
		d2s_gain = 4;
		break;
	case AW8623X_BIT_DETCFG2_D2S_GAIN_5:
		d2s_gain = 5;
		break;
	case AW8623X_BIT_DETCFG2_D2S_GAIN_8:
		d2s_gain = 8;
		break;
	case AW8623X_BIT_DETCFG2_D2S_GAIN_10:
		d2s_gain = 10;
		break;
	case AW8623X_BIT_DETCFG2_D2S_GAIN_20:
		d2s_gain = 20;
		break;
	case AW8623X_BIT_DETCFG2_D2S_GAIN_40:
		d2s_gain = 40;
		break;
	default:
		d2s_gain = -1;
		break;
	}
	return d2s_gain;
}

static int aw8623x_offset_cali(void)
{
	uint8_t reg_val[2] = {0};
	int os_code = 0;
	int d2s_gain = 0;

	AW_LOGI("enter");
	if (!g_haptic_nv->info->d2s_gain) {
		AW_LOGE("dts_info->d2s_gain = 0!");
	} else {
		haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG2,
				AW8623X_BIT_DETCFG2_D2S_GAIN_MASK,
				g_haptic_nv->info->d2s_gain);
	}
	d2s_gain = aw8623x_select_d2s_gain(g_haptic_nv->info->d2s_gain);
	if (d2s_gain < 0) {
		AW_LOGE("d2s_gain is error");
		return AW_ERROR;
	}
	aw8623x_raminit(AW_TRUE);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG2,
				 AW8623X_BIT_DETCFG2_DET_SEQ0_MASK,
				 AW8623X_BIT_DETCFG2_DET_SEQ0_OS);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1,
				 AW8623X_BIT_DETCFG1_DET_GO_MASK,
				 AW8623X_BIT_DETCFG1_DET_GO_ON);
	haptic_nv_mdelay(AW_CALI_DELAY);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1,
				 AW8623X_BIT_DETCFG1_DET_GO_MASK,
				 AW8623X_BIT_DETCFG1_DET_GO_OFF);
	haptic_nv_i2c_reads(AW8623X_REG_DETRD1, reg_val,
			    AW_I2C_BYTE_TWO);
	aw8623x_raminit(AW_FALSE);
	os_code = reg_val[0] & (~AW8623X_BIT_DETRD1_AVG_DATA_H_MASK);
	os_code = (os_code << 8) | reg_val[1];
	os_code = AW8623X_OS_FORMULA(os_code, d2s_gain);
	AW_LOGI("os_code is %d mV", os_code);
	if (os_code > 15 || os_code < -15)
		return AW_ERROR;

	return AW_SUCCESS;
}

static void aw8623x_set_trim_lra(uint8_t val)
{
	haptic_nv_i2c_write_bits(AW8623X_REG_TRIMCFG2, AW8623X_BIT_TRIMCFG2_TRIM_LRA_MASK, val);
}

static uint8_t aw8623x_get_glb_state(void)
{
	uint8_t state = 0;

	haptic_nv_i2c_reads(AW8623X_REG_GLBRD5, &state, AW_I2C_BYTE_ONE);
	AW_LOGI("reg:0x%02X=0x%02X", AW8623X_REG_GLBRD5, state);
	return state;
}

static void aw8623x_set_wav_seq(uint8_t wav, uint8_t seq)
{
	haptic_nv_i2c_writes(AW8623X_REG_WAVCFG1 + wav, &seq, AW_I2C_BYTE_ONE);
}

static void aw8623x_set_wav_loop(uint8_t wav, uint8_t loop)
{
	uint8_t tmp = 0;

	if (wav % 2) {
		tmp = loop << 0;
		haptic_nv_i2c_write_bits(AW8623X_REG_WAVCFG9 + (wav / 2),
								 AW8623X_BIT_WAVLOOP_SEQ_EVEN_MASK, tmp);
	} else {
		tmp = loop << 4;
		haptic_nv_i2c_write_bits(AW8623X_REG_WAVCFG9 + (wav / 2),
								 AW8623X_BIT_WAVLOOP_SEQ_ODD_MASK, tmp);
	}
}

static void aw8623x_play_go(aw_bool flag)
{
	uint8_t val = 0;

	AW_LOGI("enter");
	if (flag) {
		val = AW8623X_BIT_PLAYCFG4_GO_ON;
		haptic_nv_i2c_writes(AW8623X_REG_PLAYCFG4, &val, AW_I2C_BYTE_ONE);
	} else {
		val = AW8623X_BIT_PLAYCFG4_STOP_ON;
		haptic_nv_i2c_writes(AW8623X_REG_PLAYCFG4, &val, AW_I2C_BYTE_ONE);
	}
	aw8623x_get_glb_state();
}

static void aw8623x_read_lra_f0(void)
{
	uint8_t val[2] = {0};
	uint32_t f0_reg = 0;
	uint64_t f0_tmp = 0;

	AW_LOGI("enter");
	/* F_LRA_F0 */
	haptic_nv_i2c_reads(AW8623X_REG_CONTRD14, val, AW_I2C_BYTE_TWO);
	f0_reg = (f0_reg | val[0]) << 8;
	f0_reg |= (val[1] << 0);
	AW_LOGI("lra f0 reg=0x%04lX", f0_reg);
	if (!f0_reg) {
		AW_LOGE("didn't get lra f0 because f0_reg value is 0!");
		g_haptic_nv->f0 = 0;
		return;
	}
	f0_tmp = AW8623X_F0_FORMULA(f0_reg);
	g_haptic_nv->f0 = (uint32_t)f0_tmp;
	AW_LOGI("lra_f0=%ld", g_haptic_nv->f0);
}

#ifdef AW8623X_RAM_GET_F0
static int aw8623x_ram_get_f0(void)
{
	aw_bool get_f0_flag = AW_FALSE;
	uint8_t brk_en_temp = 0;
	uint8_t val[3] = {0};
	int cnt = 200;
	int ret = 0;

	AW_LOGI("enter");
	g_haptic_nv->f0 = g_haptic_nv->info->f0_pre;
	/* enter standby mode */
	aw8623x_play_stop();
	/* config dts d2s_gain */
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG2, AW8623X_BIT_DETCFG2_D2S_GAIN_MASK,
				 g_haptic_nv->info->d2s_gain);
	/* f0 calibrate work mode */
	aw8623x_play_mode(AW_RAM_MODE);
	/* enable f0 detect */
	haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG1, AW8623X_BIT_CONTCFG1_EN_F0_DET_MASK,
							 AW8623X_BIT_CONTCFG1_F0_DET_ENABLE);
	/* cont config */
	haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG6, AW8623X_BIT_CONTCFG6_TRACK_EN_MASK,
							 AW8623X_BIT_CONTCFG6_TRACK_ENABLE);
	/* enable auto break */
	haptic_nv_i2c_reads(AW8623X_REG_PLAYCFG3, &val[0], AW_I2C_BYTE_ONE);
	brk_en_temp = 0x04 & val[0];
	haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_BRK_EN_MASK,
							 AW8623X_BIT_PLAYCFG3_BRK_ENABLE);
	aw8623x_set_wav_seq(0x00, 0x04);
	aw8623x_set_wav_loop(0x00, 0x0A);
	/* ram play go */
	aw8623x_play_go(AW_TRUE);
	haptic_nv_mdelay(20);
	while (cnt) {
		haptic_nv_i2c_reads(AW8623X_REG_GLBRD5,&val[0], AW_I2C_BYTE_ONE);
		if ((val[0] & AW_BIT_GLBRD_STATE_MASK) == AW_BIT_STATE_STANDBY) {
			cnt = 0;
			get_f0_flag = AW_TRUE;
			AW_LOGI("entered standby mode! glb_state=0x%02X", val[0]);
		} else {
			cnt--;
			AW_LOGI("waitting for standby,glb_state=0x%02X", val[0]);
		}
		haptic_nv_mdelay(AW_F0_DELAY);
	}
	if (get_f0_flag) {
		aw8623x_read_lra_f0();
		ret = AW_SUCCESS;
	} else {
		ret = AW_ERROR;
		AW_LOGE("enter standby mode failed, stop reading f0!");
	}
	/* disable f0 detect */
	haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG1, AW8623X_BIT_CONTCFG1_EN_F0_DET_MASK,
							 AW8623X_BIT_CONTCFG1_F0_DET_DISABLE);
	/* recover auto break config */
	haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_BRK_EN_MASK,
							 brk_en_temp);
	return ret;
}

#else

static int aw8623x_cont_get_f0(void)
{
	aw_bool get_f0_flag = AW_FALSE;
	uint8_t brk_en_temp = 0;
	uint8_t val[3] = {0};
	int drv_width = 0;
	int cnt = 200;
	int ret = 0;

	AW_LOGI("enter");
	g_haptic_nv->f0 = g_haptic_nv->info->f0_pre;
	/* enter standby mode */
	aw8623x_play_stop();
	/* config dts d2s_gain */
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG2, AW8623X_BIT_DETCFG2_D2S_GAIN_MASK,
				 g_haptic_nv->info->d2s_gain);
	/* set cont f0 hw vbat_ref */
	aw8623x_vbat_ref_config(AW_CONT_F0_VBAT_REFER);
	/* f0 calibrate work mode */
	aw8623x_play_mode(AW_CONT_MODE);
	/* enable f0 detect */
	haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG1, AW8623X_BIT_CONTCFG1_EN_F0_DET_MASK,
							 AW8623X_BIT_CONTCFG1_F0_DET_ENABLE);
	/* cont config */
	haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG6, AW8623X_BIT_CONTCFG6_TRACK_EN_MASK,
							 AW8623X_BIT_CONTCFG6_TRACK_ENABLE);
	/* enable auto brake */
	haptic_nv_i2c_reads(AW8623X_REG_PLAYCFG3, &val[0], AW_I2C_BYTE_ONE);
	brk_en_temp = AW8623X_BIT_PLAYCFG3_BRK & val[0];
	haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_BRK_EN_MASK,
							 AW8623X_BIT_PLAYCFG3_BRK_ENABLE);
	/* f0 driver level */
	haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG6, AW8623X_BIT_CONTCFG6_DRV1_LVL_MASK,
			   g_haptic_nv->info->cont_drv1_lvl);
	val[0] = g_haptic_nv->info->cont_drv2_lvl;
	haptic_nv_i2c_writes(AW8623X_REG_CONTCFG7, val, AW_I2C_BYTE_ONE);
	val[0] = g_haptic_nv->info->cont_drv1_time;
	val[1] = g_haptic_nv->info->cont_drv2_time;
	haptic_nv_i2c_writes(AW8623X_REG_CONTCFG8, val, AW_I2C_BYTE_TWO);
	/* TRACK_MARGIN */
	if (!g_haptic_nv->info->cont_track_margin) {
		AW_LOGE("dts_info->cont_track_margin = 0!");
	} else {
		val[0] = g_haptic_nv->info->cont_track_margin;
		haptic_nv_i2c_writes(AW8623X_REG_CONTCFG11, &val[0], AW_I2C_BYTE_ONE);
	}
	/* DRV_WIDTH */
	if (!g_haptic_nv->info->f0_pre)
		return AW_ERROR;
	drv_width = AW8623X_DRV_WIDTH_FORMULA(g_haptic_nv->info->f0_pre,
										  g_haptic_nv->info->cont_track_margin,
										  g_haptic_nv->info->cont_brk_gain);
	if (drv_width < AW8623X_DRV_WIDTH_MIN)
		drv_width = AW8623X_DRV_WIDTH_MIN;
	if (drv_width > AW8623X_DRV_WIDTH_MAX)
		drv_width = AW8623X_DRV_WIDTH_MAX;
	val[0] = (uint8_t)drv_width;
	AW_LOGI("cont_drv_width=0x%02X", val[0]);
	haptic_nv_i2c_writes(AW8623X_REG_CONTCFG3, &val[0], AW_I2C_BYTE_ONE);
	/* cont play go */
	aw8623x_play_go(AW_TRUE);
	haptic_nv_mdelay(20);
	while (cnt) {
		haptic_nv_i2c_reads(AW8623X_REG_GLBRD5, &val[0], AW_I2C_BYTE_ONE);
		if ((val[0] & AW_BIT_GLBRD_STATE_MASK) == AW_BIT_STATE_STANDBY) {
			cnt = 0;
			get_f0_flag = AW_TRUE;
			AW_LOGI("entered standby! glb_state=0x%02X", val[0]);
		} else {
			cnt--;
			AW_LOGI("waitting for standby,glb_state=0x%02X", val[0]);
		}
		haptic_nv_mdelay(AW_F0_DELAY);
	}
	// debug
	haptic_nv_mdelay(100);
	if (get_f0_flag) {
		aw8623x_read_lra_f0();
		ret = AW_SUCCESS;
	} else {
		ret = AW_ERROR;
		AW_LOGE("enter standby mode failed, stop reading f0!");
	}
	/* disable f0 detect */
	haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG1, AW8623X_BIT_CONTCFG1_EN_F0_DET_MASK,
							 AW8623X_BIT_CONTCFG1_F0_DET_DISABLE);
	/* recover auto break config */
	haptic_nv_i2c_write_bits(AW8623X_REG_PLAYCFG3, AW8623X_BIT_PLAYCFG3_BRK_EN_MASK,
						 brk_en_temp);
	/* recover default hw vbat_ref */
	aw8623x_vbat_ref_config(AW_VBAT_REFER);

	return ret;
}
#endif

static void aw8623x_calculate_cali_data(void)
{
	char f0_cali_lra = 0;
	int f0_cali_step = 0;

	f0_cali_step = 100000 * ((int)g_haptic_nv->f0 - (int)g_haptic_nv->info->f0_pre) /
				   ((int)g_haptic_nv->info->f0_pre * AW8623X_F0_CALI_ACCURACY);
	AW_LOGI("f0_cali_step=%d", f0_cali_step);
	if (f0_cali_step >= 0) {	/*f0_cali_step >= 0 */
		if (f0_cali_step % 10 >= 5)
			f0_cali_step = 32 + (f0_cali_step / 10 + 1);
		else
			f0_cali_step = 32 + f0_cali_step / 10;
	} else {	/* f0_cali_step < 0 */
		if (f0_cali_step % 10 <= -5)
			f0_cali_step = 32 + (f0_cali_step / 10 - 1);
		else
			f0_cali_step = 32 + f0_cali_step / 10;
	}
	if (f0_cali_step > 31)
		f0_cali_lra = (char)f0_cali_step - 32;
	else
		f0_cali_lra = (char)f0_cali_step + 32;
	/* update cali step */
	g_haptic_nv->f0_cali_data = (int)f0_cali_lra;
	AW_LOGI("f0_cali_data=0x%02X", g_haptic_nv->f0_cali_data);
}

static void aw8623x_set_base_addr(void)
{
	uint32_t base_addr = 0;
	uint8_t val = 0;

	base_addr = g_haptic_nv->ram.base_addr;
	haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG1, AW8623X_BIT_RTPCFG1_ADDRH_MASK,
							 (uint8_t)AW_SET_BASEADDR_H(base_addr));
	val = (uint8_t)AW_SET_BASEADDR_L(base_addr);
	haptic_nv_i2c_writes(AW8623X_REG_RTPCFG2, &val, AW_I2C_BYTE_ONE);
}

static void aw8623x_set_fifo_addr(void)
{
	uint8_t ae_addr_h = 0;
	uint8_t af_addr_h = 0;
	uint8_t val[3] = {0};
	uint32_t base_addr = g_haptic_nv->ram.base_addr;


	ae_addr_h = (uint8_t)AW8623X_SET_AEADDR_H(base_addr);
	af_addr_h = (uint8_t)AW8623X_SET_AFADDR_H(base_addr);
	val[0] = ae_addr_h | af_addr_h;
	val[1] = (uint8_t)AW8623X_SET_AEADDR_L(base_addr);
	val[2] = (uint8_t)AW8623X_SET_AFADDR_L(base_addr);
	haptic_nv_i2c_writes(AW8623X_REG_RTPCFG3, val, AW_I2C_BYTE_THREE);
}

static void aw8623x_get_fifo_addr(void)
{
#ifdef AWINIC_INFO_LOG
	uint8_t ae_addr_h = 0;
	uint8_t af_addr_h = 0;
	uint8_t ae_addr_l = 0;
	uint8_t af_addr_l = 0;
	uint8_t val[3] = {0};

	haptic_nv_i2c_reads(AW8623X_REG_RTPCFG3, val, AW_I2C_BYTE_THREE);
	ae_addr_h = ((val[0]) & AW8623X_BIT_RTPCFG3_FIFO_AEH) >> 4;
	ae_addr_l = val[1];
	af_addr_h = ((val[0]) & AW8623X_BIT_RTPCFG3_FIFO_AFH);
	af_addr_l = val[2];
	AW_LOGI("almost_empty_threshold = %d,almost_full_threshold = %d",
			(uint16_t)((ae_addr_h << 8) | ae_addr_l),
			(uint16_t)((af_addr_h << 8) | af_addr_l));
#endif
}

static void aw8623x_set_ram_addr(void)
{
	uint8_t val[2] = {0};
	uint32_t base_addr = g_haptic_nv->ram.base_addr;

	val[0] = (uint8_t)AW_SET_RAMADDR_H(base_addr);
	val[1] = (uint8_t)AW_SET_RAMADDR_L(base_addr);
	haptic_nv_i2c_writes(AW8623X_REG_RAMADDRH, val, AW_I2C_BYTE_TWO);
}

static void aw8623x_set_ram_data(uint8_t *data, int len)
{
	haptic_nv_i2c_writes(AW8623X_REG_RAMDATA, data, len);
}

static void aw8623x_get_ram_data(uint8_t *ram_data, int size)
{
	haptic_nv_i2c_reads(AW8623X_REG_RAMDATA, ram_data, size);
}

static void aw8623x_interrupt_setup(void)
{
	uint8_t reg_val = 0;

	haptic_nv_i2c_reads(AW8623X_REG_SYSINT, &reg_val, AW_I2C_BYTE_ONE);
	AW_LOGI("reg SYSINT=0x%02X", reg_val);
	/* edge int mode */
	haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL3,
				 AW8623X_BIT_SYSCTRL3_INTMODE_MASK,
				 AW8623X_BIT_SYSCTRL3_INTMODE_POS_EDGE);
	/* int enable */
	haptic_nv_i2c_write_bits(AW8623X_REG_SYSINTM,
				 (AW8623X_BIT_SYSINTM_UVLM_MASK &
				  AW8623X_BIT_SYSINTM_FF_AEM_MASK &
				  AW8623X_BIT_SYSINTM_FF_AFM_MASK &
				  AW8623X_BIT_SYSINTM_OCDM_MASK &
				  AW8623X_BIT_SYSINTM_OTM_MASK &
				  AW8623X_BIT_SYSINTM_DONEM_MASK),
				 (AW8623X_BIT_SYSINTM_UVLM_ON |
				  AW8623X_BIT_SYSINTM_FF_AEM_OFF |
				  AW8623X_BIT_SYSINTM_FF_AFM_OFF |
				  AW8623X_BIT_SYSINTM_OCDM_ON |
				  AW8623X_BIT_SYSINTM_OTM_ON |
				  AW8623X_BIT_SYSINTM_DONEM_OFF));
}

static void aw8623x_haptic_trig_param_init(uint8_t pin)
{
	switch (pin) {
	case AW_TRIG1:
		g_haptic_nv->trig[0] = &aw8623x_trig1;
		break;
	case AW_TRIG2:
		g_haptic_nv->trig[1] = &aw8623x_trig2;
		break;
	case AW_TRIG3:
		g_haptic_nv->trig[2] = &aw8623x_trig3;
		break;
	default:
		break;
	}
}

static void aw8623x_haptic_select_pin(uint8_t pin)
{
	if (pin == AW_TRIG1) {
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL1,
					 AW8623X_BIT_SYSCTRL1_INTN_PIN_MASK,
					 AW8623X_BIT_SYSCTRL1_TRIG1);
		AW_LOGI("select TRIG1 pin");
	} else if (pin == AW_IRQ) {
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSCTRL1,
					 AW8623X_BIT_SYSCTRL1_INTN_PIN_MASK,
					 AW8623X_BIT_SYSCTRL1_INTN);
		AW_LOGI("select INIT pin");
	} else
		AW_LOGE("There is no such option");
}

static int aw8623x_haptic_trig_param_config(uint8_t pin)
{
	uint8_t trig_polar_lev_brk = 0x00;
	uint8_t trig_pos_seq = 0x00;
	uint8_t trig_neg_seq = 0x00;

	switch (pin) {
	case AW_TRIG1:
		if (g_haptic_nv->is_used_irq_pin) {
			aw8623x_haptic_trig_param_init(AW_TRIG1);
			aw8623x_haptic_select_pin(AW_IRQ);
			return 0;
		}
		aw8623x_haptic_select_pin(AW_TRIG1);
		trig_polar_lev_brk = g_haptic_nv->trig[0]->trig_polar << 2 |
							 g_haptic_nv->trig[0]->trig_level << 1 |
							 g_haptic_nv->trig[0]->trig_brk;
		haptic_nv_i2c_write_bits(AW8623X_REG_TRGCFG7, AW8623X_BIT_TRGCFG7_TRG1_POR_LEV_BRK_MASK,
								 trig_polar_lev_brk << 3);
		trig_pos_seq = g_haptic_nv->trig[0]->pos_enable << 7 |
					   g_haptic_nv->trig[0]->pos_sequence;
		haptic_nv_i2c_writes(AW8623X_REG_TRGCFG1, &trig_pos_seq, AW_I2C_BYTE_ONE);
		trig_neg_seq = g_haptic_nv->trig[0]->neg_enable << 7 |
					   g_haptic_nv->trig[0]->neg_sequence;
		haptic_nv_i2c_writes(AW8623X_REG_TRGCFG4, &trig_neg_seq, AW_I2C_BYTE_ONE);
		AW_LOGI("trig1 config ok!");
		break;
	case AW_TRIG2:
		trig_polar_lev_brk = g_haptic_nv->trig[1]->trig_polar << 2 |
							 g_haptic_nv->trig[1]->trig_level << 1 |
							 g_haptic_nv->trig[1]->trig_brk;
		haptic_nv_i2c_write_bits(AW8623X_REG_TRGCFG7, AW8623X_BIT_TRGCFG7_TRG2_POR_LEV_BRK_MASK,
				   trig_polar_lev_brk << 0);
		trig_pos_seq = g_haptic_nv->trig[1]->pos_enable << 7 |
					   g_haptic_nv->trig[1]->pos_sequence;
		haptic_nv_i2c_writes(AW8623X_REG_TRGCFG2, &trig_pos_seq, AW_I2C_BYTE_ONE);
		trig_neg_seq = g_haptic_nv->trig[1]->neg_enable << 7 |
					   g_haptic_nv->trig[1]->neg_sequence;
		haptic_nv_i2c_writes(AW8623X_REG_TRGCFG5, &trig_neg_seq, AW_I2C_BYTE_ONE);
		AW_LOGI("trig2 config ok!");
		break;
	case AW_TRIG3:
		trig_polar_lev_brk = g_haptic_nv->trig[2]->trig_polar << 2 |
							 g_haptic_nv->trig[2]->trig_level << 1 |
							 g_haptic_nv->trig[2]->trig_brk;
		haptic_nv_i2c_write_bits(AW8623X_REG_TRGCFG8, AW8623X_BIT_TRGCFG8_TRG3_POR_LEV_BRK_MASK,
				   trig_polar_lev_brk << 3);
		trig_pos_seq = g_haptic_nv->trig[2]->pos_enable << 7 |
					   g_haptic_nv->trig[2]->pos_sequence;
		haptic_nv_i2c_writes(AW8623X_REG_TRGCFG3, &trig_pos_seq, AW_I2C_BYTE_ONE);
		trig_neg_seq = g_haptic_nv->trig[2]->neg_enable << 7 |
					   g_haptic_nv->trig[2]->neg_sequence;
		haptic_nv_i2c_writes(AW8623X_REG_TRGCFG6, &trig_neg_seq, AW_I2C_BYTE_ONE);
		AW_LOGI("trig3 config ok!");
		break;
	default:
		break;
	}
	return AW_SUCCESS;
}

static void aw8623x_set_trig(uint8_t pin)
{
	aw8623x_haptic_trig_param_init(pin);
	aw8623x_haptic_trig_param_config(pin);
}

static void aw8623x_trig_init(void)
{
	AW_LOGI("enter");
	aw8623x_set_trig(AW_TRIG1);
	aw8623x_set_trig(AW_TRIG2);
	aw8623x_set_trig(AW_TRIG3);
}

static void aw8623x_set_gain(uint8_t gain)
{
	g_haptic_nv->gain = gain;
	haptic_nv_i2c_writes(AW8623X_REG_PLAYCFG2, &gain, AW_I2C_BYTE_ONE);
}

static void aw8623x_get_vbat(void)
{
	uint8_t reg_val[2] = {0};
	uint32_t vbat_code = 0;

	aw8623x_play_stop();
	aw8623x_raminit(AW_TRUE);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG2, AW8623X_BIT_DETCFG2_DET_SEQ0_MASK,
						 AW8623X_BIT_DETCFG2_DET_SEQ0_VBAT);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1,AW8623X_BIT_DETCFG1_DET_GO_MASK,
						 AW8623X_BIT_DETCFG1_DET_GO_ON);
	haptic_nv_mdelay(AW_VBAT_DELAY);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1, AW8623X_BIT_DETCFG1_DET_GO_MASK,
						 AW8623X_BIT_DETCFG1_DET_GO_OFF);

	haptic_nv_i2c_reads(AW8623X_REG_DETRD1, reg_val, AW_I2C_BYTE_TWO);
	vbat_code = reg_val[0] & (~AW8623X_BIT_DETRD1_AVG_DATA_H_MASK);
	vbat_code = (vbat_code << 8) | reg_val[1];
	g_haptic_nv->vbat = AW8623X_VBAT_FORMULA(vbat_code);
	if (g_haptic_nv->vbat > AW_VBAT_MAX) {
		g_haptic_nv->vbat = AW_VBAT_MAX;
		AW_LOGI("vbat max limit = %ldmV", g_haptic_nv->vbat);
	}
	if (g_haptic_nv->vbat < AW_VBAT_MIN) {
		g_haptic_nv->vbat = AW_VBAT_MIN;
		AW_LOGI("vbat min limit = %ldmV", g_haptic_nv->vbat);
	}
	AW_LOGI("vbat=%ldmV, vbat_code=0x%02lX", g_haptic_nv->vbat, vbat_code);
	aw8623x_raminit(AW_FALSE);
}

static void aw8623x_haptic_start(void)
{
	aw8623x_play_go(AW_TRUE);
}

static uint8_t aw8623x_rtp_get_fifo_afs(void)
{
	uint8_t reg_val = 0;

	haptic_nv_i2c_reads(AW8623X_REG_SYSST, &reg_val, AW_I2C_BYTE_ONE);
	reg_val &= AW8623X_BIT_SYSST_FF_AFS;
	reg_val = reg_val >> 3;
	return reg_val;
}

static void aw8623x_set_rtp_aei(aw_bool flag)
{
	if (flag) {
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSINTM, AW8623X_BIT_SYSINTM_FF_AEM_MASK,
								 AW8623X_BIT_SYSINTM_FF_AEM_ON);
	} else {
		haptic_nv_i2c_write_bits(AW8623X_REG_SYSINTM, AW8623X_BIT_SYSINTM_FF_AEM_MASK,
								 AW8623X_BIT_SYSINTM_FF_AEM_OFF);
	}
}

static int aw8623x_get_irq_state(void)
{
	uint8_t reg_val = 0;
	int ret = AW_IRQ_NULL;

	haptic_nv_i2c_reads(AW8623X_REG_SYSINT, &reg_val, AW_I2C_BYTE_ONE);
	AW_LOGI("reg SYSINT=0x%02X", reg_val);
	if (reg_val & AW8623X_BIT_SYSINT_UVLI) {
		AW_LOGE("chip uvlo int error");
		ret = AW_IRQ_UVL;
	}
	if (reg_val & AW8623X_BIT_SYSINT_OCDI) {
		AW_LOGE("chip over current int error");
		ret = AW_IRQ_OCD;
	}
	if (reg_val & AW8623X_BIT_SYSINT_OTI) {
		AW_LOGE("chip over temperature int error");
		ret = AW_IRQ_OT;
	}
	if (reg_val & AW8623X_BIT_SYSINT_DONEI) {
		AW_LOGI("chip playback done");
		ret = AW_IRQ_DONE;
	}
	if (reg_val & AW8623X_BIT_SYSINT_FF_AFI) {
		AW_LOGI("rtp mode fifo almost full!");
		ret = AW_IRQ_ALMOST_FULL;
	}
	if (reg_val & AW8623X_BIT_SYSINT_FF_AEI) {
		AW_LOGI("rtp fifo almost empty");
		ret = AW_IRQ_ALMOST_EMPTY;
	}
	return ret;
}

static void aw8623x_irq_clear(void)
{
	uint8_t val = 0;

	haptic_nv_i2c_reads(AW8623X_REG_SYSINT, &val, AW_I2C_BYTE_ONE);
	AW_LOGI("SYSINT=0x%02X", val);
}

static uint8_t aw8623x_judge_rtp_going(void)
{
	uint8_t glb_st = 0;

	haptic_nv_i2c_reads(AW8623X_REG_GLBRD5, &glb_st, AW_I2C_BYTE_ONE);
	if (((glb_st & AW_BIT_GLBRD_STATE_MASK) == AW_BIT_STATE_RTP_GO)) {
		return AW_SUCCESS;
	}
	return glb_st;
}

static void aw8623x_get_lra_resistance(void)
{
	uint8_t reg_val[2] = {0};
	uint8_t adc_fs = 0;
	uint32_t lra_code = 0;
	uint8_t d2s_gain_default = 0;
	int d2s_gain = 0;

	/* enter standby mode */
	aw8623x_play_stop();
	aw8623x_raminit(AW_TRUE);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG2,
				 AW8623X_BIT_DETCFG2_DET_SEQ0_MASK,
				 AW8623X_BIT_DETCFG2_DET_SEQ0_RL);
	haptic_nv_i2c_reads(AW8623X_REG_DETCFG1, &adc_fs, AW_I2C_BYTE_ONE);
	haptic_nv_i2c_reads(AW8623X_REG_DETCFG2, reg_val, AW_I2C_BYTE_ONE);
	d2s_gain_default = (~AW8623X_BIT_DETCFG2_D2S_GAIN_MASK) & reg_val[0];
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG2, AW8623X_BIT_DETCFG2_D2S_GAIN_MASK,
							 AW8623X_BIT_DETCFG2_D2S_GAIN_10);
	d2s_gain = aw8623x_select_d2s_gain(AW8623X_BIT_DETCFG2_D2S_GAIN_10);
	if (d2s_gain < 0) {
		AW_LOGE("d2s_gain is error");
		return;
	}
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1,
				 AW8623X_BIT_DETCFG1_ADC_FS_MASK,
				 AW8623X_BIT_DETCFG1_ADC_FS_24K);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1,
				 AW8623X_BIT_DETCFG1_DET_GO_MASK,
				 AW8623X_BIT_DETCFG1_DET_GO_ON);
	haptic_nv_mdelay(AW_RL_DELAY);
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1,
				 AW8623X_BIT_DETCFG1_DET_GO_MASK,
				 AW8623X_BIT_DETCFG1_DET_GO_OFF);
	aw8623x_raminit(AW_FALSE);
	/* restore adc fs */
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG1,
				 AW8623X_BIT_DETCFG1_ADC_FS_MASK,
				 adc_fs);
	/* restore d2s_gain */
	haptic_nv_i2c_write_bits(AW8623X_REG_DETCFG2,
				 AW8623X_BIT_DETCFG2_D2S_GAIN_MASK,
				 d2s_gain_default);
	haptic_nv_i2c_reads(AW8623X_REG_DETRD1, reg_val,
			    AW_I2C_BYTE_TWO);
	lra_code =reg_val[0] & (~AW8623X_BIT_DETRD1_AVG_DATA_H_MASK);
	lra_code = (lra_code << 8) | reg_val[1];
	/* 2num */
	g_haptic_nv->lra = AW8623X_RL_FORMULA(lra_code, d2s_gain);
	AW_LOGI("res=%ld", g_haptic_nv->lra);
}

static void aw8623x_read_reg_array(uint8_t head_reg_addr, uint8_t tail_reg_addr)
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
		printf("reg:0x%02X=0x%02X\r\n", head_reg_addr + i, reg_array[i]);
}

static void aw8623x_get_reg(void)
{
	aw8623x_read_reg_array(AW8623X_REG_RSTCFG, AW8623X_REG_RTPDATA - 1);
	aw8623x_read_reg_array(AW8623X_REG_RTPDATA + 1,  AW8623X_REG_RAMDATA - 1);
	aw8623x_read_reg_array(AW8623X_REG_RAMDATA + 1, AW8623X_REG_ANACFG10);
}

static void aw8623x_set_rtp_autosin(uint8_t flag)
{
	if (flag) {
		AW_LOGI("enable rtp_autosin\n");
		haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG6,
					 AW8623X_BIT_RTPCFG6_EN_RTP_AUTO_SIN_MASK,
					 AW8623X_BIT_RTPCFG6_EN_RTP_AUTO_SIN_ENABLE);
	} else {
		AW_LOGI("disable rtp_autosin\n");
		haptic_nv_i2c_write_bits(AW8623X_REG_RTPCFG6,
					 AW8623X_BIT_RTPCFG6_EN_RTP_AUTO_SIN_MASK,
					 AW8623X_BIT_RTPCFG6_EN_RTP_AUTO_SIN_DISABLE);
	}
}

static void aw8623x_set_rtp_data(uint8_t *data, uint32_t len)
{
	haptic_nv_i2c_writes(AW8623X_REG_RTPDATA, data, len);
}

static void aw8623x_set_repeat_seq(uint8_t seq)
{
	aw8623x_set_wav_seq(0x00, seq);
	aw8623x_set_wav_seq(0x01, 0x00);
	aw8623x_set_wav_loop(0x00, AW8623X_BIT_WAVLOOP_INIFINITELY);
}

static void aw8623x_cont_config(void)
{
	uint8_t reg_val = 0;

	AW_LOGI("enter");
	/* work mode */
	aw8623x_play_mode(AW_CONT_MODE);
	/* cont config */
	haptic_nv_i2c_write_bits(AW8623X_REG_CONTCFG6,
			   (AW8623X_BIT_CONTCFG6_TRACK_EN_MASK & AW8623X_BIT_CONTCFG6_DRV1_LVL_MASK),
			   (AW8623X_BIT_CONTCFG6_TRACK_ENABLE | g_haptic_nv->info->cont_drv1_lvl));
	reg_val = g_haptic_nv->info->cont_drv2_lvl;
	haptic_nv_i2c_writes(AW8623X_REG_CONTCFG7, &reg_val, AW_I2C_BYTE_ONE);
	/* DRV2_TIME */
	reg_val = 0xFF;
	haptic_nv_i2c_writes(AW8623X_REG_CONTCFG9, &reg_val, AW_I2C_BYTE_ONE);
	/* cont play go */
	aw8623x_play_go(AW_TRUE);
}

struct aw_haptic_func aw8623x_func_list = {
	.ram_init = aw8623x_raminit,
	.trig_init = aw8623x_trig_init,
	.play_mode = aw8623x_play_mode,
	.play_stop = aw8623x_play_stop,
	.irq_clear = aw8623x_irq_clear,
	.cont_config = aw8623x_cont_config,
	.offset_cali = aw8623x_offset_cali,
	.haptic_start = aw8623x_haptic_start,
	.check_qualify = aw8623x_check_qualify,
	.judge_rtp_going = aw8623x_judge_rtp_going,
	.protect_config = aw8623x_protect_config,
	.misc_para_init = aw8623x_misc_para_init,
	.interrupt_setup = aw8623x_interrupt_setup,
	.rtp_get_fifo_afs = aw8623x_rtp_get_fifo_afs,
	.vbat_mode_config = aw8623x_vbat_mode_config,
	.calculate_cali_data = aw8623x_calculate_cali_data,
	.set_gain = aw8623x_set_gain,
	.set_wav_seq = aw8623x_set_wav_seq,
	.set_wav_loop = aw8623x_set_wav_loop,
	.set_ram_data = aw8623x_set_ram_data,
	.get_ram_data = aw8623x_get_ram_data,
	.set_fifo_addr = aw8623x_set_fifo_addr,
	.get_fifo_addr = aw8623x_get_fifo_addr,
	.set_rtp_aei = aw8623x_set_rtp_aei,
	.set_rtp_autosin = aw8623x_set_rtp_autosin,
	.set_rtp_data = aw8623x_set_rtp_data,
	.set_ram_addr = aw8623x_set_ram_addr,
	.set_trim_lra = aw8623x_set_trim_lra,
	.set_base_addr = aw8623x_set_base_addr,
	.set_repeat_seq = aw8623x_set_repeat_seq,
#ifdef AW8623X_RAM_GET_F0
	.get_f0 = aw8623x_ram_get_f0,
#else
	.get_f0 = aw8623x_cont_get_f0,
#endif
	.get_reg = aw8623x_get_reg,
	.get_vbat = aw8623x_get_vbat,
	.get_irq_state = aw8623x_get_irq_state,
	.get_glb_state = aw8623x_get_glb_state,
	.get_lra_resistance = aw8623x_get_lra_resistance,
};
