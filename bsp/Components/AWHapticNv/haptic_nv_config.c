#include "haptic_nv.h"

#include "daric_hal.h"
#include "daric_hal_def.h"
#include "daric_hal_i2c.h"
#include "daric_log.h"
#include "system_daric.h"
#include "tx_api.h"

extern TX_TIMER vibrator_timer_handle;
extern I2C_HandleTypeDef vibrator_i2c_handle;
I2C_HandleTypeDef *hi2c = &vibrator_i2c_handle;
#ifdef HAPTIC_NV_DOUBLE
extern I2C_HandleTypeDef hi2c2;
#endif
/*****************************************************
 * @brief i2c read function
 * @param reg_addr: register address
 * @param reg_data: register data
 * @param len: Number of read registers
 * @retval i2c read status: 0->success, 1->error
 *****************************************************/
int haptic_nv_i2c_reads(uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	uint8_t cnt = 0;
#ifdef HAPTIC_NV_DOUBLE
	if(!strcmp(g_haptic_nv->mark, "left"))
		hi2c = &hi2c1;
	if(!strcmp(g_haptic_nv->mark, "right"))
		hi2c = &hi2c2;
#endif
	while (cnt < AW_I2C_RETRIES) {
		if (HAL_I2C_Mem_Read(hi2c, g_haptic_nv->i2c_addr, reg_addr,
			1, reg_data, len, 1000) == HAL_OK)
			return AW_SUCCESS;
		cnt ++;
	}

	AW_LOGE("i2c read 0x%02X err!", reg_addr);
	return AW_ERROR;
}

/*****************************************************
 * @brief i2c write function
 * @param reg_addr: register address
 * @param reg_data: register data
 * @param len: Number of write registers
 * @retval i2c write status: 0->success, 1->error
 *****************************************************/
int haptic_nv_i2c_writes(uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	uint8_t cnt = 0;
#ifdef HAPTIC_NV_DOUBLE
	if(!strcmp(g_haptic_nv->mark, "left"))
		hi2c = &hi2c1;
	if(!strcmp(g_haptic_nv->mark, "right"))
		hi2c = &hi2c2;
#endif
	while (cnt < AW_I2C_RETRIES) {
		if (HAL_I2C_Mem_Write(hi2c, g_haptic_nv->i2c_addr, reg_addr,
			1, reg_data, len, 1000) == HAL_OK)
			return AW_SUCCESS;
		cnt ++;
	}

	AW_LOGE("i2c write 0x%02X err!", reg_addr);
	return AW_ERROR;
}

/*****************************************************
 * @brief i2c write bits function
 * @param reg_addr: register address
 * @param reg_addr: register mask
 * @param reg_data: register data
 * @retval NULL
 *****************************************************/
void haptic_nv_i2c_write_bits(uint8_t reg_addr, uint32_t mask, uint8_t reg_data)
{
	uint8_t reg_val = 0;
	uint8_t reg_mask = (uint8_t)mask;

	haptic_nv_i2c_reads(reg_addr, &reg_val, AW_I2C_BYTE_ONE);
	reg_val &= reg_mask;
	reg_val |= (reg_data & (~reg_mask));
	haptic_nv_i2c_writes(reg_addr, &reg_val, AW_I2C_BYTE_ONE);
}

/*****************************************************
 * @brief read chip id function
 * @param reg_addr: chip id
 * @param type: 0->first try, 1->last try
 * @retval 0->success, 1->error
 *****************************************************/
int haptic_nv_read_chipid(uint32_t *val, uint8_t type)
{
	uint8_t cnt = 0;
	uint8_t reg_val[2] = {0};
	int ret = AW_ERROR;

	while (cnt < AW_I2C_RETRIES) {
		ret = haptic_nv_i2c_reads(AW_REG_CHIPIDH, &reg_val[0], AW_I2C_BYTE_ONE);
		if (reg_val[0] == AW8623X_CHIP_ID_H) {
			ret = haptic_nv_i2c_reads(AW8623X_REG_CHIPIDL, &reg_val[1], AW_I2C_BYTE_ONE);
			*val = reg_val[1] | reg_val[0] << 8;
		} else if (reg_val[0] == AW8624X_CHIP_ID_H) {
			ret = haptic_nv_i2c_reads(AW8624X_REG_CHIPIDL, &reg_val[1], AW_I2C_BYTE_ONE);
			*val = reg_val[1] | reg_val[0] << 8;
		} else {
			ret = haptic_nv_i2c_reads(AW_REG_ID, &reg_val[0], AW_I2C_BYTE_ONE);
			*val = reg_val[0];
		}
		if (ret == AW_ERROR) {
			if (type == AW_FIRST_TRY)
				AW_LOGI("reading chip id");
			else if (type == AW_LAST_TRY)
				AW_LOGE("i2c_read cnt=%d error=%d", cnt, ret);
			else
				AW_LOGE("type is error");
		} else {
			break;
		}
		cnt++;
	}

	return ret;
}

/*****************************************************
 * @brief stop hrtimer
 * @param None
 * @retval None
 *****************************************************/
void haptic_nv_stop_hrtimer(void)
{
	tx_timer_deactivate(&vibrator_timer_handle);
}

/*****************************************************
 * @brief start hrtimer
 * @param None
 * @retval None
 *****************************************************/
void haptic_nv_start_hrtimer(void)
{
	tx_timer_activate(&vibrator_timer_handle);
}

/*****************************************************
 * @brief hrtimer callback function, it's used to long vibrator stop. should called by HAL_TIM_PeriodElapsedCallback
 * @param htim: hrtimer
 * @retval None
 *****************************************************/
void haptic_nv_tim_periodelapsedcallback(ULONG timer_id)
{
#ifdef HAPTIC_NV_DOUBLE
	struct haptic_nv *haptic_nv_t = g_haptic_nv;

	if (htim->Instance == htim3.Instance) {
		haptic_nv_change_motor(MOTOR_L);
		g_haptic_nv->timer_ms_cnt++;
		if (g_haptic_nv->timer_ms_cnt == g_haptic_nv->duration) {
			AW_LOGI("timer over, g_haptic_nv->duration:%d", g_haptic_nv->duration);
			g_haptic_nv->duration = 0;
			g_haptic_nv->timer_ms_cnt = 0;
			g_func_haptic_nv->play_stop();
			haptic_nv_change_motor(MOTOR_R);
			g_haptic_nv->duration = 0;
			g_haptic_nv->timer_ms_cnt = 0;
			g_func_haptic_nv->play_stop();
			haptic_nv_stop_hrtimer();
		}
		g_haptic_nv = haptic_nv_t;
	}
#else
	// if (htim->Instance == htim3.Instance) {
	g_haptic_nv->timer_ms_cnt++;
	if (g_haptic_nv->timer_ms_cnt == g_haptic_nv->duration) {
		AW_LOGI("timer over, g_haptic_nv->duration:%ld", g_haptic_nv->duration);
		haptic_nv_stop_hrtimer();
		g_haptic_nv->duration = 0;
		g_haptic_nv->timer_ms_cnt = 0;
		g_func_haptic_nv->play_stop();
	}
	// }
#endif
}


/*****************************************************
 * @brief delay function
 * @param ms: millisecond
 * @retval None
 *****************************************************/
void haptic_nv_mdelay(uint32_t ms)
{
	// HAL_Delay(ms);
	tx_thread_sleep((ms) * CONFIG_SYS_CLOCK_TICKS_PER_SEC / 1000);
}

/**
  * @brief delay function
  * @param us: microsecond
  * @retval None
  */
void haptic_nv_udelay(uint32_t us)
{
	// HAL_DelayUs(us);
	// not support us delay, delay 1ms here
	tx_thread_sleep(CONFIG_SYS_CLOCK_TICKS_PER_SEC / 1000);
}

/**
  * @brief factory F0 calibration value can be stored in flash
  * @retval None
  */
void haptic_nv_set_cali_to_flash(void)
{
	AW_LOGI("f0 cali data is 0x%02x", g_haptic_nv->f0_cali_data);
}

/**
  * @brief update calibration values to driver
  * @retval None
  */
void haptic_nv_get_cali_from_flash(void)
{
	AW_LOGI("f0 cali data is 0x%02x", g_haptic_nv->f0_cali_data);
	/* g_haptic_nv->f0_cali_data = val; */
}


#ifdef AW_IRQ_CONFIG
/*****************************************************
 * @brief interrupt callback function, should called by HAL_GPIO_EXTI_Callback
 * @param GPIO_Pin: irq gpio pin
 * @retval None
 *****************************************************/
void haptic_nv_gpio_exti_callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == CONFIG_AW_IRQ_Pin)
		g_haptic_nv->irq_handle = AW_IRQ_ON;
}

#endif

/*****************************************************
 * @brief disable interrput gpio function
 * @param None
 * @retval None
 *****************************************************/
void haptic_nv_disable_irq(void)
{
	HAL_GPIO_EnableIT(CONFIG_AW_IRQ_GPIO_PORT, CONFIG_AW_IRQ_Pin, false);
}

/*****************************************************
 * @brief enable interrput gpio function
 * @param None
 * @retval None
 *****************************************************/
void haptic_nv_enable_irq(void)
{
	HAL_GPIO_EnableIT(CONFIG_AW_IRQ_GPIO_PORT, CONFIG_AW_IRQ_Pin, true);
}

/*****************************************************
 * @brief pin control function
 * @param GPIO_Pin: gpio pin
 * @param status: Pin status
 * @retval NULL
 *****************************************************/
void haptic_nv_pin_control(uint16_t GPIO_Pin, uint8_t status)
{
	if(status == AW_PIN_LOW)
		HAL_GPIO_WritePin(CONFIG_AW_RST_GPIO_PORT, GPIO_Pin, GPIO_PIN_RESET);
	else if (status == AW_PIN_HIGH)
		HAL_GPIO_WritePin(CONFIG_AW_RST_GPIO_PORT, GPIO_Pin, GPIO_PIN_SET);
}
