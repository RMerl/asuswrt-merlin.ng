/***********************************************************************
 *
 *  Copyright (c) 2006-2018  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2018:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
 ************************************************************************/

#ifndef BCM_MACSEC_CRYPTO_H__
#define BCM_MACSEC_CRYPTO_H__
#include <stdint.h>


#define BCRYPTO_SUCCESS 			0
/* Generic errores */
#define BCRYPTO_ERR_FAIL    			0x8001
#define BCRYPTO_ERR_INVALID_PARAMETER 		0x80022
/* LUT table warnings and errores */
#define BCRYPTO_WRN_ROLLBACK_ONE_VER		0x4101
#define BCRYPTO_WRN_ROLLBACK_TWO_VER		0x4102
#define BCRYPTO_ERR_MACSEC_VER_OUTDATED  	0x8101
#define BCRYPTO_ERR_VER_MISMATCHED_MAC_ADDR_IN_LUT	0x8102
#define BCRYPTO_ERR_VER_MISMATCHED_SW_VER_SAME		0x8103

/*KDF errores */
#define BCRYPTO_ERR_NULL_MACSEC_HANDLE       0x8201
#define BCRYPTO_ERR_NULL_IN_PTR              0x8202
#define BCRYPTO_ERR_NULL_USEKEY_PTR          0x8203
#define BCRYPTO_ERR_BAD_VERSION              0x8204  /* only version 1 supported */
#define BCRYPTO_ERR_BAD_COUNT_RANGE          0x8205  /* count is < 16 or > 128 */
#define BCRYPTO_ERR_BAD_COUNT_DIV4           0x8206  /* byte count has to be multiple of 4 */     

/* constants */
#define BCRYPTO_LUT_SIZE_IN_BYTE	3640
#define BCRYPTO_MSG_HEADER_SIZE_IN_BYTE		16
#define BCRYPTO_NONCE_SIZE_IN_BYTE		16
#define BCRYPTO_SW_VER_SIZE_IN_BYTE		16
#define BCRYPTO_MAC_ADDR_SIZE_IN_BYTE		8	

#define BCRYPTO_MAX_KEY_SIZE_IN_BYTE	32
#define BCRYPTO_MAX_IV_SIZE_IN_BYTE	16
#define BCRYPTO_MAX_SALT_SIZE_IN_BYTE	12
#define BCRYPTO_MAX_SSCI_SIZE_IN_BYTE	4


typedef uint32_t BCryptoStatus;

/***************************************************************************
Summary:
Handle for the MACsec module.
***************************************************************************/
typedef struct BCryptoMACsec_Module      *BCryptoMACsec_Module_Handle;

/***************************************************************************
Summary:
Handle for the MACsec channel.
***************************************************************************/
typedef struct BCryptoMACsec_Channel     *BCryptoMACsec_Channel_Handle;

/***************************************************************************
Summary:
Message format.

Description:
This structure is used to store the overture message or overture response message.
***************************************************************************/
typedef struct Message
{
	char cHeader[BCRYPTO_MSG_HEADER_SIZE_IN_BYTE]; /* Must be either OvertureMsg or OvertureRspMsg type, Version number just in case we need to change the  Overture message format in the future.  Eg “OM00000000010001 or OR00000000010001 where OM stands for overture message and OR stands for overture receiver message  00000000010001:  000000 (reserved)  0001 (Major version) 0001 (minor version) */
	uint32_t uMACsecImpVersion; /*start from 0x 00000001 */
	uint8_t uNonce[BCRYPTO_NONCE_SIZE_IN_BYTE]; /*e.g. 0x11223344556677889900AABBCCDDEEFF, 0x1234567890ABCDEF1234567890ABCDEF*/
	char swVersion[BCRYPTO_SW_VER_SIZE_IN_BYTE];/*padded 0 at the end, if the sw version is less than 16 bytes.  Eg “3.5p5s1_TXB6” for TCH XB6 => “3.5p5s1_TXB60000”   or   for BCA it  could be “ver3.6.112a => ver3.6.112a00000*/
	uint8_t MACaddress[BCRYPTO_MAC_ADDR_SIZE_IN_BYTE];/*  If there are only 6 bytes,  padding zero in front of 6 bytes:  1234567890AB => 1234567890AB0000, FEEDBABE1122 => FEEDBABE11220000*/
	uint32_t uReserved; /*All 0’s*/
}Message;

/***************************************************************************
Summary:
Callback function to update LUT.

Description:
This callback function should be called to update the LUT table. It should write in_uiLen bytes of data to the LUT table in non-volatile memory.
***************************************************************************/
typedef void (* BCryptoMACsecLutUpdateCb  )(
	unsigned char *inp_ucLut, /*pointer to a allocated buffer to store the LUT */
	uint32_t in_uiLen	  /*size of LUT table, currently only 3640 is allowed */
);

/***************************************************************************
Summary:
MACsec module settings structure.

Description:
This is the settings for BCryptoMACsec_Module_Handle.

See Also:
BCryptoMACsec_Module_Open
***************************************************************************/
typedef struct BcryptoMACsec_Module_Settings
{
	unsigned char 			*pLut; /*pointer to a allocated buffer to store the LUT */
	uint32_t 			uiLutSize;/*size of LUT table, currently only 3640 is allowed */
        BCryptoMACsecLutUpdateCb  	pLutUpdateCb;/*callback function to update LUT */
}BCryptoMACsec_Module_Settings;

/***************************************************************************
Summary:
This enum is to identify algorithms used in a MACsec crypto channel.

Description:
This enumeration defines the supported algorithms for MACsec crypto library, currently only BCRYPTO_MACSEC_ALG_GCM_AES_XPN_128 is allowed.
****************************************************************************/
typedef enum BCRYPTO_MACSEC_ALG
{
	BCRYPTO_MACSEC_ALG_GCM_AES_XPN_128,
	BCRYPTO_MACSEC_ALG_GCM_AES_XPN_256,
	BCRYPTO_MACSEC_ALG_GCM_AES_128,
	BCRYPTO_MACSEC_ALG_GCM_AES_256,
	/* Add new enum before this line */
	BCRYPTO_MACSEC_ALG_MAX
}BCRYPTO_MACSEC_ALG;

/***************************************************************************
Summary:
This structure defines the data returned from BCryptoMACsec_GenKey()

See Also:
BPskCb
****************************************************************************/
typedef struct BCryptoMACsecData
{
	BCRYPTO_MACSEC_ALG algo;
	uint8_t key[BCRYPTO_MAX_KEY_SIZE_IN_BYTE];
	uint8_t size_key;
	uint8_t IV[BCRYPTO_MAX_IV_SIZE_IN_BYTE];
	uint8_t size_iv;
	/* The following fields are only required for GCM_AES_XPN 128 or 256 */
	uint8_t salt[BCRYPTO_MAX_SALT_SIZE_IN_BYTE];  
	uint8_t size_salt;
	uint8_t SSCI[BCRYPTO_MAX_SSCI_SIZE_IN_BYTE];
	uint8_t size_ssci;
	uint8_t hkey[BCRYPTO_MAX_KEY_SIZE_IN_BYTE];
	uint8_t size_hkey;
}BCryptoMACsecData;

/***************************************************************************
Summary:
This defined the callback function used when the key is derived.

Description:
This callback function will be called within BCryptoMACsec_GenKey() to notify the caller that the key, the salt, the SSCI and the hkey are available to use. 

See Also:
BCryptoMACsec_GenKey
***************************************************************************/
typedef void (* BPskCb )( 
	void *inp_Context,          /*eg. SC*/
	void *inp_KeyContext,       /*eg.  SA */
	void *inp_UserData,	    /* user defined data */
	BCryptoMACsecData *inp_sKeyData   /*key data returned from KDF */
);

/***************************************************************************
Summary:
MACsec channel settings structure.

Description:
This is the settings for BCryptoMACsec_Channel_Handle.

See Also:
BCryptoMACsec_ChannelHandle_Open
***************************************************************************/
typedef struct BCryptoMACsec_Channel_Settings
{ 
	BCRYPTO_MACSEC_ALG algo;  /* algorithm used in this channel */
	BPskCb	pSessionKeyCb;    /* callback function for key generation */ 
	void *pContext;		  /* user defined context such as SC */
	void *pUserData;	  /* user define data */
}BCryptoMACsec_Channel_Settings;

/***************************************************************************
Summary:
Open a MACsec module.

Description:

See Also:
BCryptoMACsec_Module_Close
***************************************************************************/
BCryptoMACsec_Module_Handle BCryptoMACsec_Module_Open(
	BCryptoMACsec_Module_Settings *inp_sModuleSettings 
);

/***************************************************************************
Summary:
Close a MACsec module.

Description:

See Also:
BCryptoMACsec_Module_Open
***************************************************************************/
void BCryptoMACsec_Module_Close(
	BCryptoMACsec_Module_Handle in_hModuleHandle
);

/***************************************************************************
Summary:
Open a MACsec channel.

Description:

See Also:
BCryptoMACsec_Channel_Open
***************************************************************************/
BCryptoMACsec_Channel_Handle BCryptoMACsec_ChannelHandle_Open(
	BCryptoMACsec_Module_Handle in_hModuleHandle, 
	BCryptoMACsec_Channel_Settings *inp_sChannelSettings
);

/***************************************************************************
Summary:
Close a MACsec channel.

Description:

See Also:
BCryptoMACsec_Channel_Close
***************************************************************************/
void BCryptoMACsec_ChannelHandle_Close(
	BCryptoMACsec_Channel_Handle in_hKeyHandle
);

/***************************************************************************
Summary:
Get a random number from MACsec crypto library.

Description:
This is will generate an in_nLen bytes random number and store in outp_ucRandomNumber.
***************************************************************************/
BCryptoStatus BCryptoMACsec_GetRandomNumber(
	unsigned char *outp_ucRandomNumber, 
	uint32_t in_nLen
);


/***************************************************************************
Summary:
This function is used to check the messages device’s LUT information only. 

Description:
This function just check the LUT against local/remote messages but not updatingthe LUT table 

See Also:
BCryptoMACsec_CheckAndUpdateLUT
***************************************************************************/

BCryptoStatus BCryptoMACsec_CheckLUT(
	BCryptoMACsec_Module_Handle in_hHandle, 
	Message *inoutp_sLocalMsg,   /* must be OR msg */ 
	Message *inp_sRemoteMsg );


/***************************************************************************
Summary:
This function is used to check the messages and update device’s LUT information. 
Description:
This function just check the LUT against local/remote messages and updatingthe LUT table if necessary

***************************************************************************/
BCryptoStatus BCryptoMACsec_UpdateLUT(
	BCryptoMACsec_Module_Handle in_hHandle, 
	Message *inp_sRemoteMsg,  
	uint32_t in_nImplVer
);

/***************************************************************************
Summary:
This function is used to derive session key from the overture and overture response messages. 

Description:
***************************************************************************/
BCryptoStatus BCryptoMACsec_GenKey(
	BCryptoMACsec_Channel_Handle in_hChannelHandle, 
	void *inp_KeyContext, 			/*SA ID*/
	Message *inp_sOvertureMsg, 
	Message *inp_sOvertureRespMsg
);

/***************************************************************************
Summary:
This function is used to fill dedicated buffer with random number
***************************************************************************/
BCryptoStatus BCryptoMACsec_Sanitize(
	unsigned char *inp_ucBuffer, 
	uint32_t in_nByteCount
);

/***************************************************************************
Summary:
This returns the MACsec KDF version.

Description:
***************************************************************************/
uint32_t BCryptoMACsec_GetVersion();
#endif /* BCM_MACSEC_CRYPTO_H__ */

