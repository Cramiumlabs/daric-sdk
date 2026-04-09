#include "nfc_drv.h"
#include "nfc_timer.h"
#include "port.h"
#include "nfc_i2c.h"

SDKCtx gHedNfcContx = {0};

void hed_nfc_set_ver(uint8_t *pVer)
{
	SDKCtx *pContx = &gHedNfcContx;
	memcpy(pContx->fw_version, pVer, 2);
}

uint8_t *hed_nfc_get_ver(void)
{
	return gHedNfcContx.fw_version;
}
void hed_nfc_set_chip_ver(uint8_t *chip_Ver)
{
	SDKCtx *pContx = &gHedNfcContx;
	memcpy(pContx->chip_version, chip_Ver, 2);
}

uint8_t *hed_nfc_get_chip_ver(void)
{
	return gHedNfcContx.chip_version;
}
void NFCC_Set_BootMode(uint8_t mode)
{
	NFC_TRACE_DEBUG("SetBootMode %d", mode);
	if (mode)
	{ //固件升级模式
		port_hardware_gpioDwlPullup();
		timer_wait_ms(10);
		I2CModeSet(Mode_CMS);
	}
	else
	{ //正常工作模式
		port_hardware_gpioDwlPulldown();
		I2CModeSet(Mode_NCI);
	}
}

void NFCC_Rest(uint8_t mode)
{ //0 正常工作模式;  1 固件升级模式
	port_hardware_gpioVenPulldown();
	timer_wait_ms(10);
	NFCC_Set_BootMode(mode);
	port_hardware_gpioVenPullup();
	timer_wait_ms(10);
}

void nfc_hw_init(void)
{
	//port_nfcc_io_init();
	i2c_drv_Init();
	NFCC_Rest(0);
}



