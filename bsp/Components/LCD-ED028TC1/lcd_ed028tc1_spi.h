#ifndef __E2501_H__
#define __E2501_H__

/**********************************
Constant set
**********************************/
#define PSR         0x00
#define PWR         0x01
#define POF         0x02
#define PFS         0x03
#define PON         0x04
#define DPC         0x05
#define DSLP        0x06
#define BTST        0x07
#define DTM1        0x10
#define DRF         0x12
#define DTM2        0x13
#define AWM1        0x15
#define AWM2        0x16
#define LUTC        0x20
#define LUTD        0x21
#define LUTR        0x22
#define MISCS       0x27
#define PLL         0x30
#define TSC         0x40
#define TSE         0x41
#define CDI         0x50
#define LPD         0x51
#define TCON        0x60
#define TRES        0x61
#define DAM         0x65
#define REV         0x70
#define FLG         0x71
#define LUT_COL_FLG 0x72
#define LUT_BUSY_FLG 0x73
#define AMV         0x80
#define VV          0x81
#define VDCS        0x82
#define DTMW        0x83
#define EDS         0x84
#define XONS        0x85
#define LEDDS       0x90
#define PBC         0x91
#define PBCS        0x92
#define EXTRS       0x93
#define NTRS        0x94
#define GDOS        0xE0

#define Initial_23_16          0x00
#define Initial_15_0           0x0000 // 4K
#define Temperature            0x001000 // 4K


#define Image_Start  0x006000

#define DEMO_KIT         0x00
#define RA_JIG           0x01
#define TEST_JIG         0x02
#define DEMO_KIT_AV      0x10
#define RA_JIG_AV        0x11
#define TEST_JIG_AV      0x12

#define LUT_CHECK_SUCCESS 0x00
#define LUT_CHECK_FAILURE 0x01

#define Initial_Counter            64
#define Temperature_LUT_Counter   16770 // LUTC + LUTD + LUTR

#endif //#ifndef __E2501_H__
