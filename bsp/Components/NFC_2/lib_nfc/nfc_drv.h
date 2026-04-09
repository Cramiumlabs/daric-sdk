#ifndef __NFC_DRV_H__
#define __NFC_DRV_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "nci.h"
#include "port.h"

typedef struct
{
	uint8_t uid[4];
	uint8_t fw_version[2];
	uint8_t chip_version[2];
	int nIrq;
	uint8_t fw_updated;
	uint8_t nMode;
	bool busy;
} SDKCtx;

extern SDKCtx gHedNfcContx;

void NFCC_Rest(uint8_t mode);

#endif
