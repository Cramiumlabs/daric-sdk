#ifndef __NFC_TYPE_H__
#define __NFC_TYPE_H__
#include "port.h"
// typedef unsigned char uint8_t;
// typedef unsigned short int uint16_t;
// typedef unsigned int uint32_t;

#ifndef TRUE
#define TRUE (0x01) /* Logical True Value */
#endif
#ifndef FALSE
#define FALSE (0x00) /* Logical False Value */
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif



#define UINT32_TO_STREAM(p, u32)         \
	{                                    \
		*(p)++ = (uint8_t)(u32);         \
		*(p)++ = (uint8_t)((u32) >> 8);  \
		*(p)++ = (uint8_t)((u32) >> 16); \
		*(p)++ = (uint8_t)((u32) >> 24); \
	}
#define UINT24_TO_STREAM(p, u24)         \
	{                                    \
		*(p)++ = (uint8_t)(u24);         \
		*(p)++ = (uint8_t)((u24) >> 8);  \
		*(p)++ = (uint8_t)((u24) >> 16); \
	}
#define UINT16_TO_STREAM(p, u16)        \
	{                                   \
		*(p)++ = (uint8_t)(u16);        \
		*(p)++ = (uint8_t)((u16) >> 8); \
	}
#define UINT8_TO_STREAM(p, u8)  \
	{                           \
		*(p)++ = (uint8_t)(u8); \
	}
#define INT8_TO_STREAM(p, u8)  \
	{                          \
		*(p)++ = (int8_t)(u8); \
	}
#define ARRAY32_TO_STREAM(p, a)            \
	{                                      \
		register int ijk;                  \
		for (ijk = 0; ijk < 32; ijk++)     \
			*(p)++ = (uint8_t)a[31 - ijk]; \
	}
#define ARRAY16_TO_STREAM(p, a)            \
	{                                      \
		register int ijk;                  \
		for (ijk = 0; ijk < 16; ijk++)     \
			*(p)++ = (uint8_t)a[15 - ijk]; \
	}
#define ARRAY8_TO_STREAM(p, a)            \
	{                                     \
		register int ijk;                 \
		for (ijk = 0; ijk < 8; ijk++)     \
			*(p)++ = (uint8_t)a[7 - ijk]; \
	}
#define BDADDR_TO_STREAM(p, a)                          \
	{                                                   \
		register int ijk;                               \
		for (ijk = 0; ijk < BD_ADDR_LEN; ijk++)         \
			*(p)++ = (uint8_t)a[BD_ADDR_LEN - 1 - ijk]; \
	}
#define LAP_TO_STREAM(p, a)                         \
	{                                               \
		register int ijk;                           \
		for (ijk = 0; ijk < LAP_LEN; ijk++)         \
			*(p)++ = (uint8_t)a[LAP_LEN - 1 - ijk]; \
	}
#define DEVCLASS_TO_STREAM(p, a)                          \
	{                                                     \
		register int ijk;                                 \
		for (ijk = 0; ijk < DEV_CLASS_LEN; ijk++)         \
			*(p)++ = (uint8_t)a[DEV_CLASS_LEN - 1 - ijk]; \
	}
#define ARRAY_TO_STREAM(p, a, len)      \
	{                                   \
		register int ijk;               \
		for (ijk = 0; ijk < len; ijk++) \
			*(p)++ = (uint8_t)a[ijk];   \
	}
#define REVERSE_ARRAY_TO_STREAM(p, a, len)      \
	{                                           \
		register int ijk;                       \
		for (ijk = 0; ijk < len; ijk++)         \
			*(p)++ = (uint8_t)a[len - 1 - ijk]; \
	}

#define STREAM_TO_UINT8(u8, p) \
	{                          \
		u8 = (uint8_t)(*(p));  \
		(p) += 1;              \
	}
#define STREAM_TO_UINT16(u16, p)                                    \
	{                                                               \
		u16 = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)); \
		(p) += 2;                                                   \
	}
#define STREAM_TO_UINT24(u32, p)                                                                             \
	{                                                                                                        \
		u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16)); \
		(p) += 3;                                                                                            \
	}
#define STREAM_TO_UINT32(u32, p)                                                                                                                  \
	{                                                                                                                                             \
		u32 = (((uint32_t)(*(p))) + ((((uint32_t)(*((p) + 1)))) << 8) + ((((uint32_t)(*((p) + 2)))) << 16) + ((((uint32_t)(*((p) + 3)))) << 24)); \
		(p) += 4;                                                                                                                                 \
	}
#define STREAM_TO_BDADDR(a, p)                                   \
	{                                                            \
		register int ijk;                                        \
		register uint8_t *pbda = (uint8_t *)a + BD_ADDR_LEN - 1; \
		for (ijk = 0; ijk < BD_ADDR_LEN; ijk++)                  \
			*pbda-- = *p++;                                      \
	}
#define STREAM_TO_ARRAY32(a, p)                    \
	{                                              \
		register int ijk;                          \
		register uint8_t *_pa = (uint8_t *)a + 31; \
		for (ijk = 0; ijk < 32; ijk++)             \
			*_pa-- = *p++;                         \
	}
#define STREAM_TO_ARRAY16(a, p)                    \
	{                                              \
		register int ijk;                          \
		register uint8_t *_pa = (uint8_t *)a + 15; \
		for (ijk = 0; ijk < 16; ijk++)             \
			*_pa-- = *p++;                         \
	}
#define STREAM_TO_ARRAY8(a, p)                    \
	{                                             \
		register int ijk;                         \
		register uint8_t *_pa = (uint8_t *)a + 7; \
		for (ijk = 0; ijk < 8; ijk++)             \
			*_pa-- = *p++;                        \
	}
#define STREAM_TO_DEVCLASS(a, p)                                  \
	{                                                             \
		register int ijk;                                         \
		register uint8_t *_pa = (uint8_t *)a + DEV_CLASS_LEN - 1; \
		for (ijk = 0; ijk < DEV_CLASS_LEN; ijk++)                 \
			*_pa-- = *p++;                                        \
	}
#define STREAM_TO_LAP(a, p)                                  \
	{                                                        \
		register int ijk;                                    \
		register uint8_t *plap = (uint8_t *)a + LAP_LEN - 1; \
		for (ijk = 0; ijk < LAP_LEN; ijk++)                  \
			*plap-- = *p++;                                  \
	}
#define STREAM_TO_ARRAY(a, p, len)      \
	{                                   \
		register int ijk;               \
		for (ijk = 0; ijk < len; ijk++) \
			((uint8_t *)a)[ijk] = *p++; \
	}
#define REVERSE_STREAM_TO_ARRAY(a, p, len)              \
	{                                                   \
		register int ijk;                               \
		register uint8_t *_pa = (uint8_t *)a + len - 1; \
		for (ijk = 0; ijk < len; ijk++)                 \
			*_pa-- = *p++;                              \
	}

/*****************************************************************************
  ** Macros to get and put bytes to and from a field (Little Endian format).
  ** These are the same as to stream, except the pointer is not incremented.
  *****************************************************************************/

#define UINT32_TO_FIELD(p, u32)                         \
	{                                                   \
		*(uint8_t *)(p)		  = (uint8_t)(u32);         \
		*((uint8_t *)(p) + 1) = (uint8_t)((u32) >> 8);  \
		*((uint8_t *)(p) + 2) = (uint8_t)((u32) >> 16); \
		*((uint8_t *)(p) + 3) = (uint8_t)((u32) >> 24); \
	}
#define UINT24_TO_FIELD(p, u24)                         \
	{                                                   \
		*(uint8_t *)(p)		  = (uint8_t)(u24);         \
		*((uint8_t *)(p) + 1) = (uint8_t)((u24) >> 8);  \
		*((uint8_t *)(p) + 2) = (uint8_t)((u24) >> 16); \
	}
#define UINT16_TO_FIELD(p, u16)                        \
	{                                                  \
		*(uint8_t *)(p)		  = (uint8_t)(u16);        \
		*((uint8_t *)(p) + 1) = (uint8_t)((u16) >> 8); \
	}
#define UINT8_TO_FIELD(p, u8)            \
	{                                    \
		*(uint8_t *)(p) = (uint8_t)(u8); \
	}

/*****************************************************************************
  ** Macros to get and put bytes to and from a stream (Big Endian format)
  *****************************************************************************/

#define UINT32_TO_BE_STREAM(p, u32)      \
	{                                    \
		*(p)++ = (uint8_t)((u32) >> 24); \
		*(p)++ = (uint8_t)((u32) >> 16); \
		*(p)++ = (uint8_t)((u32) >> 8);  \
		*(p)++ = (uint8_t)(u32);         \
	}
#define UINT24_TO_BE_STREAM(p, u24)      \
	{                                    \
		*(p)++ = (uint8_t)((u24) >> 16); \
		*(p)++ = (uint8_t)((u24) >> 8);  \
		*(p)++ = (uint8_t)(u24);         \
	}
#define UINT16_TO_BE_STREAM(p, u16)     \
	{                                   \
		*(p)++ = (uint8_t)((u16) >> 8); \
		*(p)++ = (uint8_t)(u16);        \
	}
#define UINT8_TO_BE_STREAM(p, u8) \
	{                             \
		*(p)++ = (uint8_t)(u8);   \
	}
#define ARRAY_TO_BE_STREAM(p, a, len)   \
	{                                   \
		register int ijk;               \
		for (ijk = 0; ijk < len; ijk++) \
			*(p)++ = (uint8_t)a[ijk];   \
	}

#define BE_STREAM_TO_UINT8(u8, p) \
	{                             \
		u8 = (uint8_t)(*(p));     \
		(p) += 1;                 \
	}
#define BE_STREAM_TO_UINT16(u16, p)                                         \
	{                                                                       \
		u16 = (uint16_t)(((uint16_t)(*(p)) << 8) + (uint16_t)(*((p) + 1))); \
		(p) += 2;                                                           \
	}
#define BE_STREAM_TO_UINT24(u32, p)                                                                  \
	{                                                                                                \
		u32 = (((uint32_t)(*((p) + 2))) + ((uint32_t)(*((p) + 1)) << 8) + ((uint32_t)(*(p)) << 16)); \
		(p) += 3;                                                                                    \
	}
#define BE_STREAM_TO_UINT32(u32, p)                                                                                                 \
	{                                                                                                                               \
		u32 = ((uint32_t)(*((p) + 3)) + ((uint32_t)(*((p) + 2)) << 8) + ((uint32_t)(*((p) + 1)) << 16) + ((uint32_t)(*(p)) << 24)); \
		(p) += 4;                                                                                                                   \
	}
#define BE_STREAM_TO_ARRAY(p, a, len)   \
	{                                   \
		register int ijk;               \
		for (ijk = 0; ijk < len; ijk++) \
			((uint8_t *)a)[ijk] = *p++; \
	}

/*****************************************************************************
  ** Macros to get and put bytes to and from a field (Big Endian format).
  ** These are the same as to stream, except the pointer is not incremented.
  *****************************************************************************/

#define UINT32_TO_BE_FIELD(p, u32)                      \
	{                                                   \
		*(uint8_t *)(p)		  = (uint8_t)((u32) >> 24); \
		*((uint8_t *)(p) + 1) = (uint8_t)((u32) >> 16); \
		*((uint8_t *)(p) + 2) = (uint8_t)((u32) >> 8);  \
		*((uint8_t *)(p) + 3) = (uint8_t)(u32);         \
	}
#define UINT24_TO_BE_FIELD(p, u24)                      \
	{                                                   \
		*(uint8_t *)(p)		  = (uint8_t)((u24) >> 16); \
		*((uint8_t *)(p) + 1) = (uint8_t)((u24) >> 8);  \
		*((uint8_t *)(p) + 2) = (uint8_t)(u24);         \
	}
#define UINT16_TO_BE_FIELD(p, u16)                     \
	{                                                  \
		*(uint8_t *)(p)		  = (uint8_t)((u16) >> 8); \
		*((uint8_t *)(p) + 1) = (uint8_t)(u16);        \
	}
#define UINT8_TO_BE_FIELD(p, u8)         \
	{                                    \
		*(uint8_t *)(p) = (uint8_t)(u8); \
	}

#endif
