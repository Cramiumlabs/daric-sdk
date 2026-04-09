/**@file  error.h
* @brief  Header file of uitl error
* @author  liangww
* @date  2021-5-12
* @version  V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef  _ERROR_H_
#define _ERROR_H_

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup UTIL
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup UTIL_Error_Exported_Types UTIL_Error Exported Types
  * @{
  */


/**
  * @brief  Error Code Structure definition
  */
enum error_code
{
		SE_SUCCESS =                           0x00000000,  ///< Success 

		SE_ERR_EXPECT_LENGTH =	               0x10006100,	///< ISO7816 T0 Expect length
		SE_ERR_RETURN_DATA =                   0x10006281,	///< Return Data Error
		SE_ERR_FILE_KEY_INVALID =              0x10006283,	///< Select File Invalid/File/Key CRC ERROR
		SE_ERR_NUM_RETRY =                     0x10006300,	///< last 1-bytes-Retry Num
		SE_ERR_DATA_REMAINING =	               0x10006100,	///< Data Remainning
		SE_ERR_FLAG_UNCHANGED =	               0x10006400,	///< Status Flag No Change
		SE_ERR_WRITE_FLASH =	               0x10006581,	///< Write FLASH Failure
		SE_ERR_CODE_RUN =                      0x10006582,	///< Code Run Error
		SE_ERR_LENGTH =	                       0x10006700,	///< Length Error
		SE_ERR_CLA_NOT_MATCH =	               0x10006900,	///< CLA Error
		SE_ERR_STATUS_INVALID =	               0x10006901,	///< Status Invalid	
		SE_ERR_STRUCTURSE_NOT_INVALID =	       0x10006981,	///< Structure Not Invalid
		SE_ERR_SECURITY_VIOLATION =	           0x10006982,	///< Security Violation
		SE_ERR_AUTHENTICATION_LOCKED =         0x10006983,	///< Key Locked
		SE_ERR_CHALLENGSE_NOT_GET =	           0x10006984,	///< No Challenge
		SE_ERR_OPERATION_VIOLATION =	       0x10006985,	///< Operation Violation
		SE_ERR_FILE_NOT_SELECTED =	           0x10006986,	///< File No Selected
		SE_ERR_SECURITY_MESSAGE =	           0x10006987,	///< No Security Data
		SE_ERR_SECURITY_DATA_INCORRECT =       0x10006988,	///< Security Data Incorrect
		SE_ERR_VERIFY_FAILURE =           	   0x10006A80,	///< Data Structure Incorrect/Verify Failure
		SE_ERR_FUNCTION_UNSUPPORTED =	       0x10006A81,	///< Function Unsupported
		SE_ERR_FILE_NOT_FOUND =	               0x10006A82,	///< File Not Found
		SE_ERR_RECORD_NOT_FOUND =	           0x10006A83,	///< Record Not Found
		SE_ERR_SPACSE_NOT_ENOUGH =	           0x10006A84,	///< Space Not Enough
		SE_ERR_P1P2_INVALID =	               0x10006A86,	///< P1 P2 Error
		SE_ERR_OUT_OF_BOUND =	               0x10006B00,	///< Offset Error
		SE_ERR_DATA_RETURN_LENGTH =	           0x10006C00,	///< Le Error/last 1-bytes-Return Length
		SE_ERR_APDU_UNSUPPORTED =	           0x10006D00,	///< APDU Unsupported
		SE_ERR_CLA_INVALID =	               0x10006E00,	///< CLA Invalid
		SE_ERR_COMMAND_TURN_INVALID =          0x10006E01,	///< Command Turn Invalid 
		SE_ERR_SECURITY_INVALID =	           0x10006E02,	///< Security Invaild
		SE_ERR_REFERENCE_DATA_INVALID =	       0x10006F00,	///< Data Invaild
		SE_ERR_APPLICATION_LOCKED =	           0x10009303,	///< Application Locked
		SE_ERR_ALG_UNSUPPORTED =	           0x10009401,	///< Alg Unsupported
		SE_ERR_KEYTYPSE_UNSUPPORTED =	       0x10009402,	///< Key Type Unsupported
		SE_ERR_KEY_NOT_FOUND =	               0x10009403,	///< Key Not Found
		SE_ERR_ID_EXISTED =	                   0x10009404,	///< ID Existed
		SE_ERR_KEYTYPE_EXISTED =	           0x10009405,	///< Key Type Existed
		SE_ERR_MAC_INVALID =	               0x10009406,	///< MAC Invaild
		SE_ERR_DATA_TRANSMIT_LENGTH =	       0x10009500,	///< last 1-bytes reference Data length
	
		SE_ERR_HANDLE_INVALID =                0x20000000,  ///< Function Handle Error
		SE_ERR_PARAM_INVALID =                 0x20000001,  ///< Function Param Error
		
		SE_ERR_INIT = 		                   0x30000000,  ///< Init error
		SE_ERR_BUSY=			               0x30000001,	///< Communication busy
		SE_ERR_TIMEOUT = 	                   0x30000002,	///< Over Time
		SE_ERR_ATR = 	                       0x30000003,	///< ATR Error
		SE_ERR_LRC_CRC =		               0x30000004,  ///< LRC/CRC Error
		SE_ERR_LEN = 	                       0x30000005,  ///< Data Len Error
		SE_ERR_DATA = 	                       0x30000006,  ///< Data Error
		SE_ERR_COMM = 	                       0x30000007,  ///< Communication Error
		SE_ERR_NO_SELECT = 	                   0x30000008,  ///< No Select
		SE_ERR_UPDATE = 	                   0x30000009,	///< UPDATE Error
		SE_ERR_SOF = 	                   	   0x3000000A,  ///< SOF Error
		SE_ERR_ESE_ABORT = 	             	   0x3000000B,  ///< ese abort Error
		SE_ERR_RETRY = 	             	   	   0x3000000C,  ///< ese RETRY
		ESE_TRANS_MOREDATA = 	               0x3000000D,  ///< ese chain
};

/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */


#endif/**/

