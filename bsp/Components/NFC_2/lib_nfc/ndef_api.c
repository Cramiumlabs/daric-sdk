/******************************************************************************
 *
 *  Copyright 2015-2021 NXP
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
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
//#include "comm.h"
#include "nfc_type.h"
#include "ndef_api.h"
#include "trace.h"

static uint8_t RTD_TEXT[1] = {'T'};
static uint8_t RTD_URL[1] = {'U'};
//when construct and parse NDEF message, the buffer is used byte user api, to promis the input and output buffer is diffrent when call the lower api in ndef_utils.
uint8_t gNdefBuffer [NDEF_RAM_MAX_SIZE];

static char *URI_PREFIX_MAP[] = {
            "", // 0x00
            "http://www.", // 0x01
            "https://www.", // 0x02
            "http://", // 0x03
            "https://", // 0x04
            "tel:", // 0x05
            "mailto:", // 0x06
            "ftp://anonymous:anonymous@", // 0x07
            "ftp://ftp.", // 0x08
            "ftps://", // 0x09
            "sftp://", // 0x0A
            "smb://", // 0x0B
            "nfs://", // 0x0C
            "ftp://", // 0x0D
            "dav://", // 0x0E
            "news:", // 0x0F
            "telnet://", // 0x10
            "imap:", // 0x11
            "rtsp://", // 0x12
            "urn:", // 0x13
            "pop:", // 0x14
            "sip:", // 0x15
            "sips:", // 0x16
            "tftp:", // 0x17
            "btspp://", // 0x18
            "btl2cap://", // 0x19
            "btgoep://", // 0x1A
            "tcpobex://", // 0x1B
            "irdaobex://", // 0x1C
            "file://", // 0x1D
            "urn:epc:id:", // 0x1E
            "urn:epc:tag:", // 0x1F
            "urn:epc:pat:", // 0x20
            "urn:epc:raw:", // 0x21
            "urn:epc:", // 0x22
            "urn:nfc:", // 0x23
    };
#define URI_PREFIX_MAP_LENGTH 24

static nfc_friendly_type_t ndef_getFriendlyType(uint8_t tnf, uint8_t *type, uint8_t typeLength)
{
    if(tnf == NDEF_TNF_WELLKNOWN
                && (memcmp(type, RTD_TEXT, typeLength) == 0))
    {
        return NDEF_FRIENDLY_TYPE_TEXT;
    }
    if(tnf == NDEF_TNF_URI 
                || (tnf == NDEF_TNF_WELLKNOWN && (memcmp(type, RTD_URL, typeLength) == 0)))
    {
        return NDEF_FRIENDLY_TYPE_URL;
    }
    if(tnf == NDEF_TNF_MEDIA)   
    {
        return NDEF_FRIENDLY_TYPE_MIME;
    }

    return NDEF_FRIENDLY_TYPE_OTHER;
}

tNDEF_STATUS NDEF_createUri(uint8_t *uri, uint32_t uriLength, uint8_t*outNdefBuff, uint32_t* outBufferLen)
{
    tNDEF_STATUS status = NDEF_OK;
    uint32_t current_size = 0;
    uint32_t max_size = *outBufferLen;
    uint32_t i, prefixLength;

    for (i = 1; i < URI_PREFIX_MAP_LENGTH; i++)
    {
        if (memcmp(URI_PREFIX_MAP[i], uri, strlen(URI_PREFIX_MAP[i])) == 0)
        {
            
            break;
        }
    }
    if (i == URI_PREFIX_MAP_LENGTH)
    {
        i = 0;
    }
    prefixLength = strlen(URI_PREFIX_MAP[i]);
    status = NDEF_MsgAddRec(outNdefBuff, max_size, &current_size, NDEF_TNF_WKT, (uint8_t*)RTD_URL, 1, NULL, 0,
                                    (uint8_t*)&i, 1);
    if(status != NDEF_OK)
    {
        //NFC_TRACE_ERROR("%s: couldn't create Ndef record");
        return status;
    }
    status = NDEF_MsgAppendPayload(outNdefBuff, max_size, &current_size, outNdefBuff,
                                    (uint8_t*)(uri + prefixLength), (uint32_t)(uriLength - prefixLength));

    if (status != NDEF_OK )
    {
        NFC_TRACE_ERROR ("%s: couldn't create Ndef record", __FUNCTION__);
        return status;
    }
    //return output length
    *outBufferLen = current_size;
    return NDEF_OK;
}

tNDEF_STATUS NDEF_createText(uint8_t *languageCode, uint32_t langCodeLength, uint8_t *text, uint32_t textLength, uint8_t*outNdefBuff, uint32_t* outBufferLen)
{
    static char * DEFAULT_LANGUAGE_CODE = "En";
    tNDEF_STATUS status = NDEF_OK;
    uint32_t current_size = 0;
    uint32_t max_size = *outBufferLen ;
    char *langCode = (char *)languageCode;


    if (langCodeLength >= 64)
    {
        NFC_TRACE_ERROR ("%s: language code is too long, must be <64 bytes.", __FUNCTION__);
        return NDEF_OTHER_ERROR;
    }
    if (langCodeLength == 0)
    {
        //set default language to 'EN'
        langCode = DEFAULT_LANGUAGE_CODE;
        langCodeLength = 2;
    }
    //memset (outNdefBuff, 0, max_size);
    status = NDEF_MsgAddRec(outNdefBuff, max_size, &current_size, NDEF_TNF_WKT, (uint8_t*)RTD_TEXT, 1, NULL, 0,
                                    (uint8_t*)(&langCodeLength), 1);
    if(status != NDEF_OK)
    {
        
        NFC_TRACE_ERROR ("%s: couldn't create Ndef record", __FUNCTION__);
        return status;
    }
    status = NDEF_MsgAppendPayload(outNdefBuff, max_size, &current_size, outNdefBuff,
                                    (uint8_t*)langCode, langCodeLength);
    if(status != NDEF_OK)
    {
        
        NFC_TRACE_ERROR ("%s: couldn't create Ndef record", __FUNCTION__);
        return status;
    }
    status = NDEF_MsgAppendPayload(outNdefBuff, max_size, &current_size, outNdefBuff,
                                    (uint8_t*)text, textLength);
    if(status != NDEF_OK)
    {
        NFC_TRACE_ERROR ("%s: couldn't create Ndef record", __FUNCTION__);
        return status;
    }
    *outBufferLen = current_size;

    return NDEF_OK;
}

tNDEF_STATUS NDEF_createMime(uint8_t *mimeType, uint32_t mimeTypeLength, uint8_t *mimeData, uint32_t mimeDataLength,
                                                                uint8_t*outNdefBuff, uint32_t* outBufferLen)
{
    tNDEF_STATUS status = NDEF_OK;
    uint32_t current_size = 0;
    uint32_t max_size = *outBufferLen;
    
    if (mimeTypeLength + mimeDataLength >= max_size)
    {
        NFC_TRACE_ERROR ("%s: data too large.", __FUNCTION__);
        return NDEF_MSG_INSUFFICIENT_MEM;
    }

    status = NDEF_MsgAddRec(outNdefBuff, max_size, &current_size, NDEF_TNF_MEDIA, (uint8_t *)mimeType, mimeTypeLength, NULL, 0,
                                    (uint8_t*)mimeData, (uint32_t)mimeDataLength);

    if (status != NDEF_OK )
    {
        NFC_TRACE_ERROR ("%s: couldn't create Ndef record", __FUNCTION__);
        return status;
    }
    *outBufferLen = current_size;
    return NDEF_OK;
}

tNDEF_STATUS NDEF_readText( uint8_t*ndefBuff, uint32_t ndefBuffLen, char * outText, uint32_t* textLen)
{
    int langCodeLen;
    uint8_t *payload;
    uint32_t payloadLength;
    uint8_t ndef_tnf;
    uint8_t *ndef_type;
    uint8_t ndef_typeLength;
    nfc_friendly_type_t friendly_type;

    ndef_type = NDEF_RecGetType((uint8_t*)ndefBuff, &ndef_tnf, &ndef_typeLength);
    friendly_type = ndef_getFriendlyType(ndef_tnf, ndef_type, ndef_typeLength);
    if (friendly_type != NDEF_FRIENDLY_TYPE_TEXT)
    {
        return NDEF_OTHER_ERROR;
    }
    payload = NDEF_RecGetPayload((uint8_t*)ndefBuff, (uint32_t*)&payloadLength);
    if (payload == NULL)
    {
        return NDEF_OTHER_ERROR;
    }
    langCodeLen = payload[0] & 0x3F;//bit7:UTF-8/UTF-16, bit6:RFU, bit5-bit0:langCodeLen
    if (*textLen < (payloadLength - langCodeLen - 1))
    {
        return NDEF_MSG_INSUFFICIENT_MEM;
    }
    memcpy(outText, payload + langCodeLen + 1, payloadLength - langCodeLen - 1);
    *textLen = (payloadLength - langCodeLen - 1);
    return NDEF_OK;
}

tNDEF_STATUS NDEF_readLang( uint8_t*ndefBuff, uint32_t ndefBuffLen, char * outLang, uint16_t* LangLen)
{
    int langCodeLen;
    uint8_t *payload;
    uint32_t payloadLength;
    uint8_t ndef_tnf;
    uint8_t *ndef_type;
    uint8_t ndef_typeLength;
    nfc_friendly_type_t friendly_type;

    ndef_type = NDEF_RecGetType((uint8_t*)ndefBuff, &ndef_tnf, &ndef_typeLength);
    friendly_type = ndef_getFriendlyType(ndef_tnf, ndef_type, ndef_typeLength);
    if (friendly_type != NDEF_FRIENDLY_TYPE_TEXT)
    {
        return NDEF_OTHER_ERROR;
    }
    payload = NDEF_RecGetPayload((uint8_t*)ndefBuff, &payloadLength);
    if (payload == NULL)
    {
        return NDEF_OTHER_ERROR;
    }
    langCodeLen = payload[0] & 0x3F;
    if (*LangLen < langCodeLen)
    {
        return NDEF_MSG_INSUFFICIENT_MEM;
    }
    memcpy(outLang, payload + 1, langCodeLen);
    *LangLen = langCodeLen;
    return NDEF_OK;
}

tNDEF_STATUS NDEF_readUrl(uint8_t*ndefBuff, uint32_t ndefBuffLen, char * outUrl, uint32_t* urlBufferLen)
{
    uint32_t prefixIdx;
    uint32_t prefixLen;
    uint8_t *payload;
    uint32_t payloadLength;
    uint8_t ndef_tnf;
    uint8_t *ndef_type;
    uint8_t ndef_typeLength;
    nfc_friendly_type_t friendly_type;

    ndef_type = NDEF_RecGetType((uint8_t*)ndefBuff, &ndef_tnf, &ndef_typeLength);
    friendly_type = ndef_getFriendlyType(ndef_tnf, ndef_type, ndef_typeLength);
    if (friendly_type != NDEF_FRIENDLY_TYPE_URL)
    {
        return NDEF_OTHER_ERROR;
    }
    payload = NDEF_RecGetPayload((uint8_t*)ndefBuff, (uint32_t *)&payloadLength);
    if (payload == NULL)
    {
        return NDEF_OTHER_ERROR;
    }

    if( payload[0]  >= URI_PREFIX_MAP_LENGTH )
    {
        prefixIdx = 0;
    }
    else
    {
        prefixIdx = payload[0];
    }
    prefixLen = strlen(URI_PREFIX_MAP[prefixIdx]);
    if (*urlBufferLen < payloadLength + prefixLen)
    {
        return NDEF_MSG_INSUFFICIENT_MEM;
    }
    memcpy(outUrl, URI_PREFIX_MAP[prefixIdx], prefixLen);
    memcpy(outUrl + prefixLen, payload + 1, payloadLength - 1);
    *urlBufferLen = (payloadLength + prefixLen - 1);
    return NDEF_OK;
 }

 //get L field int BER_TLV structure, return V field address
uint8_t* parseBERLen(uint8_t* LenAddr, uint16_t* lenValue)
 {
     uint8_t* dataAddr = LenAddr;
     uint16_t lenByte = *dataAddr++;
     if(lenByte < BER_81){
         *lenValue = lenByte;
     }else if(lenByte == BER_81){
         lenByte = *dataAddr++;
         *lenValue = lenByte;
     }else if(lenByte == BER_82){
         lenByte = *dataAddr++;
         *lenValue = (lenByte << 0x08)| (*dataAddr++);
     }else{
         //format error
         return NULL;
     }
     return dataAddr;
 }
  //construct BER_TLV
uint16_t constructBERLen(uint8_t* LenAddr, uint16_t lenValue)
 {
     if(lenValue < 0x80){
         *LenAddr = lenValue;
         return 1;
     }else if(lenValue < 0x100){
         *LenAddr++ = BER_81;
         *LenAddr++ = lenValue;
         return 2;
     }else if(lenValue <= 0xFFFF){
         *LenAddr++ = BER_82;
         *LenAddr++ = lenValue >> 8;
         *LenAddr++ = lenValue;
         return 3;
     }else{
         //format error
         return 0;
     }
 }

 //p_inout_buf is the input buffer and outbuffer , is LV struct, L is BER_TLV strurct
tNDEF_STATUS ndef_parse(uint16_t buf_max_len, uint8_t *p_inout_buf)
{
    tNDEF_STATUS status;
    uint8_t ndef_tnf;
    uint8_t *ndef_type;
    uint8_t ndef_typeLength;
    nfc_friendly_type_t friendly_type;
    uint32_t outLen = 0;
    uint8_t lenField[3];
    uint8_t paramField[100];
    uint16_t paramLen = 0;
    
    uint8_t *p_ndef;
    uint16_t ndefLen;
    uint8_t lenLen;
     
    uint8_t *payload = NULL;
    
    if(!p_inout_buf)
        return NDEF_OTHER_ERROR;

    p_ndef = parseBERLen(p_inout_buf, &ndefLen);
    if(p_ndef == NULL)//format error
        return NDEF_OTHER_ERROR;
    ndef_type = NDEF_RecGetType((uint8_t*)p_ndef, &ndef_tnf, &ndef_typeLength);
    friendly_type = ndef_getFriendlyType(ndef_tnf, ndef_type, ndef_typeLength);
    
    outLen = NDEF_RAM_MAX_SIZE;//use global buffer gNdefBufferfor outbuffer, check the max length of gNdefBufferfor here
    
    switch(friendly_type)
    {
        case NDEF_FRIENDLY_TYPE_TEXT:
            status = NDEF_readText(p_ndef, ndefLen, (char*)gNdefBuffer, &outLen);
            if(status != NDEF_OK)
                return status;
            paramLen = 64;//the max length of langage is 64
            status = NDEF_readLang( p_ndef, ndefLen, (char*)paramField, &paramLen);
            break;
        case NDEF_FRIENDLY_TYPE_URL:
            status = NDEF_readUrl(p_ndef, ndefLen, (char*)gNdefBuffer, &outLen);
            break;
        case NDEF_FRIENDLY_TYPE_MIME:
            //copy the type before get payload, prevent being covered by payload
            memcpy(paramField, ndef_type, ndef_typeLength);
            paramLen = ndef_typeLength;
            payload = NDEF_RecGetPayload(p_ndef, &outLen);
            if(payload != NULL)
            {
                memcpy(gNdefBuffer, payload, outLen);
                status = NDEF_OK;
            }
            else
                return NDEF_OTHER_ERROR;
            break;
        default:
            return NDEF_MSG_INVALID_TYPE;

    }
    if(status != NDEF_OK)
        return status;
    
    lenLen = constructBERLen(lenField, outLen);
    
    if(buf_max_len < (1 + lenLen + outLen + 1 + paramLen))//the buffer is not enough
        return NDEF_OTHER_ERROR;
    //copy the parsed data
    memcpy(p_inout_buf + 1 + lenLen, gNdefBuffer, outLen);

    *p_inout_buf = friendly_type;
    memcpy((p_inout_buf + 1), lenField, lenLen);
    p_inout_buf[(1 + lenLen + outLen)] = paramLen;
    memcpy((p_inout_buf + 1 + lenLen + outLen + 1), paramField, paramLen);
    return NDEF_OK;
}


tNDEF_STATUS ndef_construct (uint8_t type, uint16_t buf_max_len, uint8_t *p_inout_buf)
{
    tNDEF_STATUS status = NDEF_OK;
    uint16_t dataLen;
    uint16_t paramLen;
    uint8_t* pData;
    uint8_t* pParam;
    uint32_t outLen;
    
    uint8_t lenField[3];
    uint16_t lenLen;
    
    
    if(!p_inout_buf)
        return NDEF_OTHER_ERROR;
    
    pData= parseBERLen(p_inout_buf, &dataLen);
    if(pData == NULL)
        return NDEF_OTHER_ERROR;
    pParam = parseBERLen(pData + dataLen, &paramLen);
    if(pParam == NULL)
        return NDEF_OTHER_ERROR;
    
    outLen = NDEF_RAM_MAX_SIZE;//use global buffer gNdefBufferfor outbuffer, check the max length of gNdefBufferfor here
    switch(type)
    {
        case NDEF_FRIENDLY_TYPE_TEXT:
            status = NDEF_createText(pParam, paramLen, pData, dataLen, gNdefBuffer, &outLen);
            break;
        case NDEF_FRIENDLY_TYPE_URL:
            status = NDEF_createUri(pData, dataLen, gNdefBuffer, &outLen);
            break;
        case NDEF_FRIENDLY_TYPE_MIME:
            status = NDEF_createMime(pParam, paramLen, pData, dataLen, gNdefBuffer, &outLen);
            break;
        default:
            return NDEF_OTHER_ERROR;

    }
    if(status != NDEF_OK)
        return status;
    lenLen = constructBERLen(lenField, outLen);
    if(buf_max_len < (lenLen + outLen))
        return NDEF_OTHER_ERROR;
    //copy the value
    memcpy((p_inout_buf + lenLen), gNdefBuffer, outLen);
    //copy the len 
    memcpy(p_inout_buf, lenField, lenLen);
    return NDEF_OK; 
}

#define TEST_NDEF  1
#ifdef TEST_NDEF
#define MAX_LEN  1024
uint8_t inOutBuffer[MAX_LEN];

//test 支付宝链接------
uint8_t test_alipayNdef[] = {0x91,0x01,0xC1,0x55,0x04,0x72,0x65, 
0x6E,0x64,0x65,0x72,0x2E,0x61,0x6C,0x69,0x70,0x61,0x79,0x2E,0x63,0x6F,0x6D,0x2F, 
0x70,0x2F,0x73,0x2F,0x75,0x6C,0x69,0x6E,0x6B,0x2F,0x3F,0x73,0x63,0x65,0x6E,0x65, 
0x3D,0x6E,0x66,0x63,0x26,0x73,0x63,0x68,0x65,0x6D,0x65,0x3D,0x61,0x6C,0x69,0x70, 
0x61,0x79,0x25,0x33,0x41,0x25,0x32,0x46,0x25,0x32,0x46,0x6E,0x66,0x63,0x25,0x32, 
0x46,0x61,0x70,0x70,0x25,0x33,0x46,0x69,0x64,0x25,0x33,0x44,0x32,0x30,0x30,0x30, 
0x32,0x31,0x32,0x33,0x25,0x32,0x36,0x61,0x70,0x70,0x53,0x63,0x68,0x65,0x6D,0x65, 
0x25,0x33,0x44,0x4E,0x46,0x43,0x5F,0x50,0x41,0x59,0x5F,0x41,0x50,0x50,0x25,0x32, 
0x36,0x70,0x61,0x67,0x65,0x50,0x61,0x72,0x61,0x6D,0x73,0x25,0x33,0x44,0x25,0x32, 
0x35,0x37,0x42,0x25,0x32,0x35,0x32,0x32,0x6B,0x25,0x32,0x35,0x32,0x32,0x25,0x32, 
0x35,0x33,0x41,0x25,0x32,0x35,0x32,0x32,0x32,0x38,0x31,0x37,0x35,0x37,0x32,0x30, 
0x36,0x34,0x39,0x36,0x38,0x34,0x32,0x36,0x38,0x35,0x5F,0x30,0x2E,0x30,0x31,0x5F, 
0x38,0x36,0x30,0x31,0x25,0x32,0x35,0x32,0x32,0x25,0x32,0x35,0x37,0x44,0x14,0x0F, 
0x1B,0x61,0x6E,0x64,0x72,0x6F,0x69,0x64,0x2E,0x63,0x6F,0x6D,0x3A,0x70,0x6B,0x67, 
0x63,0x6F,0x6D,0x2E,0x65,0x67,0x2E,0x61,0x6E,0x64,0x72,0x6F,0x69,0x64,0x2E,0x41, 
0x6C,0x69,0x70,0x61,0x79,0x47,0x70,0x68,0x6F,0x6E,0x65,0x52,0x08,0x1E,0x61,0x69, 
0x72,0x74,0x6F,0x75,0x63,0x68,0x80,0x7C,0x32,0x30,0x30,0x7C,0x7C,0x31,0x35,0x30, 
0x37,0x33,0x30,0x31,0x33,0x35,0x38,0x33,0x38,0x34,0x39,0x37,0x34,0x32,0x30,0x38, 
0x7C,0x7C,0x55,0x7C};
void ndefApiTest(void)
{
    uint8_t* pInput = inOutBuffer;
    uint8_t lenField[3];
    uint16_t len;
//    uint16_t inputLen;
    uint16_t lenLen;
    //tNDEF_STATUS status = NDEF_OK;
    uint8_t testType;
    //Ò»¡¢construct API test
    //1.text type
    /*
    char lang[] = "en-US";
    char myText[] = "This is my application data";
    
    //payLoad
    len = strlen(myText);
    lenLen = constructBERLen(lenField, len);
    memcpy(pInput, lenField, lenLen);
    pInput += lenLen;
    memcpy(pInput, myText, len);
    pInput += len;
    //lang
    len = strlen(lang);
    lenLen = constructBERLen(lenField, len);
    memcpy(pInput, lenField, lenLen);
    pInput += lenLen;
    memcpy(pInput, lang, len);
    testType = NDEF_FRIENDLY_TYPE_TEXT;
    */
    //2.Uri type
    
    char myUri[] = "http://test.com.cn";
    //payLoad
    len = strlen(myUri);
    lenLen = constructBERLen(lenField, len);
    memcpy(pInput, lenField, lenLen);
    pInput += lenLen;
    memcpy(pInput, myUri, len);
    pInput += len;
    testType = NDEF_FRIENDLY_TYPE_URL;
    
    //3¡¢Mime type
    /*
    char mimeType[] = "Appliction/code.jpg";
    uint8_t myData[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A};
    //payLoad
    len = sizeof(myData);
    lenLen = constructBERLen(lenField, len);
    memcpy(pInput, lenField, lenLen);
    pInput += lenLen;
    memcpy(pInput, myData, len);
    pInput += len;
    //lang
    len = strlen(mimeType);
    lenLen = constructBERLen(lenField, len);
    memcpy(pInput, lenField, lenLen);
    pInput += lenLen;
    memcpy(pInput, mimeType, len);
    testType = NDEF_FRIENDLY_TYPE_MIME;
    */
    //µ÷ÓÃconstruct½Ó¿Ú
    ndef_construct (testType, MAX_LEN, inOutBuffer);
    
    
    //¶þ¡¢parse API test
    //1.text type
    //lang£ºApplication/no
    //text£ºThis is my text.
   // uint8_t myNdef[] = {0xD1,0x01,0x1F,0x54,0x0E,0x41,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x2F,0x6E,0x6F,0x54,0x68,0x69,0x73,0x20,0x69,0x73,0x20,0x6D,0x79,0x20,0x74,0x65,0x78,0x74,0x2E};
 
    //2.Uri type
     //http://www.hed.com.cn

//     uint8_t myNdef[] = {0xD1,0x01,0x0B,0x55,0x01,0x68,0x65,0x64,0x2E,0x63,0x6F,0x6D,0X2E,0x63,0x6E};


    //3.Mime type
        //type:Application/no
        //text:1122334455667788
    // uint8_t myNdef[] = {0xD2,0x0E,0x08,0x41,0x70,0x70,0x6C,0x69,0x63,0x61,0x74,0x69,0x6F,0x6E,0x2F,0x6E,0x6F,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
     

    memset(inOutBuffer, 0, MAX_LEN);
    pInput = inOutBuffer;

    len = sizeof(test_alipayNdef);
    lenLen = constructBERLen(lenField, len);
    memcpy(pInput, lenField, lenLen);
    pInput += lenLen;
    memcpy(pInput, test_alipayNdef, len);
    pInput += len;
    ndef_parse(MAX_LEN, inOutBuffer);
   
}

#endif