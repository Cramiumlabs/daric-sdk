/******************************************************************************
 *
 *  Copyright (C) 2015 NXP Semiconductors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License")
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
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
// clang-format off
#ifndef __NDEF_API__H__
#define __NDEF_API__H__
#include "ndef_utils.h"

//BER_TLV flag
#define BER_81    0x81
#define BER_82    0X82

//the max size of ram buffer used in ndef
#define NDEF_RAM_MAX_SIZE 300


/* NDEF Type Name Format */
/** Empty (type/id/payload len =0) */
#define NDEF_TNF_EMPTY          0
/** NFC Forum well-known type/RTD */
#define NDEF_TNF_WELLKNOWN      1
/** Media-type as defined in RFC 2046 */
#define NDEF_TNF_MEDIA          2
/** Absolute URI as defined in RFC 3986 */
#define NDEF_TNF_URI            3
/** NFC Forum external type/RTD */
#define NDEF_TNF_EXT            4
/** Unknown (type len =0) */
#define NDEF_TNF_UNKNOWN        5
/** Unchanged (type len =0) */
#define NDEF_TNF_UNCHANGED      6
/**
 *  \brief friendly NDEF Type Name
 */
typedef enum {
    /**
     * \brief NDEF text: NFC Forum well-known type + RTD: 0x55
     */
    NDEF_FRIENDLY_TYPE_TEXT = 1,
    /*
     * \brief NDEF text: NFC Forum well-known type + RTD: 0x54
     */
    NDEF_FRIENDLY_TYPE_URL = 2,
    
    NDEF_FRIENDLY_TYPE_MIME = 3,
    /*
     * \brief Handover Select package
     */
 //   NDEF_FRIENDLY_TYPE_HS = 2,
    /*
     * \brief Handover Request package
     */
 //   NDEF_FRIENDLY_TYPE_HR = 3,
    /*
     * \brief not able to decode directly
     */
    NDEF_FRIENDLY_TYPE_OTHER = 4,
}nfc_friendly_type_t;

extern tNDEF_STATUS NDEF_readText( uint8_t*ndefBuff, uint32_t ndefBuffLen, char * outText, uint32_t* textLen);

extern tNDEF_STATUS NDEF_readLang( uint8_t*ndefBuff, uint32_t ndefBuffLen, char * outLang, uint16_t* LangLen);

extern tNDEF_STATUS NDEF_readUrl(uint8_t*ndefBuff, uint32_t ndefBuffLen, char * outUrl, uint32_t* urlBufferLen);

extern tNDEF_STATUS NDEF_createUri(uint8_t *uri, uint32_t uriLength, uint8_t*outNdefBuff, uint32_t* outBufferLen);

extern tNDEF_STATUS NDEF_createText(uint8_t *languageCode, uint32_t langLen, uint8_t *text, uint32_t textLen, uint8_t*outNdefBuff, uint32_t* outBufferLen);

extern tNDEF_STATUS NDEF_createMime(uint8_t *mimeType, uint32_t typeLen, uint8_t *mimeData, uint32_t mimeDataLength,
                                    uint8_t*outNdefBuff, uint32_t* outBufferLen);

extern void ndefApiTest(void);


#endif
