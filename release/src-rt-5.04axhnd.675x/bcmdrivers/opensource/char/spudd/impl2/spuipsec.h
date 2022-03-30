/*
<:copyright-BRCM:2007:GPL/GPL:standard

   Copyright (c) 2007 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/******************************************************************************
 *
 *  Broadcom IPsec SPU Driver Common API
 *  Description: Header file for IPSec SPU Device Driver
 *  File: spuipsec.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *  Contains macros and structures for IPSec packet processing.
 *
 *****************************************************************************/
#ifndef _SPUIPSEC_H_
#define _SPUIPSEC_H_

#define MAX_CONTEXT_SIZE        24      /* 1(len+optype) +  
                                           1(context flags+offset/reserved) + 
                                           8(AES Key 32bytes) + 
                                           4(AES IV 16bytes) + 
                                           5(Inner State) +
                                           5(Outer State) */

/*
 * Cipher Command subtype flags.
 */
#define UBSEC_ENCODE            1
#define UBSEC_DECODE            2
#define UBSEC_3DES              4
#define UBSEC_DES               8
#define UBSEC_MAC_MD5           16
#define UBSEC_MAC_SHA1          32
/*
 * flags kept common between ubsio and ubsec so flags
 * from app can be passed accross till SRL.
 * 32 ...2048 taken by ubsio
 */
#define UBSEC_AES               4096
/* Currently only supported by OPERATION_IPSEC_AES */
#define UBSEC_CTR_MODE          8192    
#define UBSEC_AES_128BITKEY     16384
#define UBSEC_AES_192BITKEY     32768
#define UBSEC_AES_256BITKEY     65536
/* Reserved for  BCM_OEM_4      131072   */
#define UBSEC_NO_CRYPTO         262144

/*
 *      Command field definitions.
 */
#define UBSEC_ENCODE_3DES (UBSEC_ENCODE+UBSEC_3DES)
#define UBSEC_DECODE_3DES (UBSEC_DECODE+UBSEC_3DES)
#define UBSEC_ENCODE_DES  (UBSEC_ENCODE+UBSEC_DES)
#define UBSEC_DECODE_DES  (UBSEC_DECODE+UBSEC_DES)
#define UBSEC_ENCODE_AES  (UBSEC_ENCODE+UBSEC_AES)
#define UBSEC_DECODE_AES  (UBSEC_DECODE+UBSEC_AES)

#define UBSEC_ENCODE_3DES_MD5   (UBSEC_ENCODE_3DES+UBSEC_MAC_MD5)
#define UBSEC_DECODE_3DES_MD5   (UBSEC_DECODE_3DES+UBSEC_MAC_MD5)
#define UBSEC_ENCODE_3DES_SHA1  (UBSEC_ENCODE_3DES+UBSEC_MAC_SHA1)
#define UBSEC_DECODE_3DES_SHA1  (UBSEC_DECODE_3DES+UBSEC_MAC_SHA1)
#define UBSEC_ENCODE_DES_MD5    (UBSEC_ENCODE_DES+UBSEC_MAC_MD5)
#define UBSEC_DECODE_DES_MD5    (UBSEC_DECODE_DES+UBSEC_MAC_MD5)
#define UBSEC_ENCODE_DES_SHA1   (UBSEC_ENCODE_DES+UBSEC_MAC_SHA1)
#define UBSEC_DECODE_DES_SHA1   (UBSEC_DECODE_DES+UBSEC_MAC_SHA1)
#define UBSEC_ENCODE_AES_MD5    (UBSEC_ENCODE_AES+UBSEC_MAC_MD5)
#define UBSEC_DECODE_AES_MD5    (UBSEC_DECODE_AES+UBSEC_MAC_MD5)
#define UBSEC_ENCODE_AES_SHA1   (UBSEC_ENCODE_AES+UBSEC_MAC_SHA1)
#define UBSEC_DECODE_AES_SHA1   (UBSEC_DECODE_AES+UBSEC_MAC_SHA1)

#define UBSEC_USING_CRYPT(f) ( (f) & (UBSEC_3DES | UBSEC_DES | UBSEC_AES) )
#define UBSEC_USING_MAC(f)   ( (f) & (UBSEC_MAC_MD5 | UBSEC_MAC_SHA1) )

#define UBSEC_USING_CTR_MODE(f) ( (f) & (UBSEC_CTR_MODE) )

/*
 * Cryptographic parameter definitions
 */
#define UBSEC_DES_KEY_LENGTH       2  /* long */
#define UBSEC_3DES_KEY_LENGTH      6 /* long */
#define UBSEC_AES_KEY_LENGTH       8  /* long */
#define UBSEC_MAX_CRYPT_KEY_LENGTH UBSEC_AES_KEY_LENGTH
#define UBSEC_IV_LENGTH            2       /* long */
#define UBSEC_IV_LENGTH_BYTES      8
#define UBSEC_AES_IV_LENGTH        4       /* long */
#define UBSEC_AES_IV_LENGTH_BYTES  16
#define UBSEC_MAX_IV_LENGTH        UBSEC_AES_IV_LENGTH

#define UBSEC_MAC_KEY_LENGTH       64      /* Bytes */
#define UBSEC_MD5_LENGTH           16      /* Bytes */
#define UBSEC_SHA1_LENGTH          20      /* Bytes */
#define UBSEC_HMAC_LENGTH          20  /* Max of MD5/SHA1 */

/*
 * Status codes
 */
#define UBSEC_STATUS_SUCCESS              0
#define UBSEC_STATUS_NO_DEVICE           -1
#define UBSEC_STATUS_TIMEOUT             -2
#define UBSEC_STATUS_INVALID_PARAMETER   -3
#define UBSEC_STATUS_DEVICE_FAILED       -4
#define UBSEC_STATUS_DEVICE_BUSY         -5
#define UBSEC_STATUS_NO_RESOURCE         -6
#define UBSEC_STATUS_CANCELLED           -7

/*
 * Device_Context is handle for all ubsec device operations. It
 * is assigned at initialization time.
 */
typedef void *ubsec_DeviceContext_t, **ubsec_DeviceContext_pt;

/*
 * HMAC State type defines the current (inner/outer)
 * Hash state values.
 */
typedef struct ubsec_HMAC_State_s {
        unsigned char InnerState[UBSEC_HMAC_LENGTH];
        unsigned char OuterState[UBSEC_HMAC_LENGTH];
} ubsec_HMAC_State_t, *ubsec_HMAC_State_pt;

/*
 * Generic Fragment information type. Length
 * and physical address of fragment defined
 * here.
 */
#define UBSEC_MAX_FRAGMENTS 20

typedef struct ubsec_FragmentInfo_s {
  unsigned short  FragmentLength;       /* Length of the fragment.     */
  unsigned short  FragmentFlags;        /* For Centurtion */
  char FragmentAddress_pad[8];
  unsigned char *FragmentAddress;       /* Virtual or Physical address */
} ubsec_FragmentInfo_t, *ubsec_FragmentInfo_pt;

/*
 * Initial Vector type for CBC operations.
 */
typedef long ubsec_IV_t[UBSEC_MAX_IV_LENGTH], *ubsec_IV_pt;

/*
 * Crypt Key type definitions.
 */

/* Crypt key type. */
typedef long ubsec_CryptKey_t[UBSEC_MAX_CRYPT_KEY_LENGTH], *ubsec_CryptKey_pt;

/* Cipher command type defines Cipher/Authentication operation. */
typedef long ubsec_CipherCommand_t;

/* Status code is used by the SRL to indicate status */
typedef long ubsec_Status_t;

/*
 * Cipher command struture defines the parameters of a cipher
 * command, its input and output data areas along with the
 * context.
 */
typedef struct ubsec_CipherCommandInfo_s {
    ubsec_CipherCommand_t Command;  /* Operation(s) to perform */
    ubsec_IV_pt InitialVector;      /* IV for CBC operation. */
    ubsec_CryptKey_pt CryptKey;     /* For CBC operation. */
    ubsec_HMAC_State_pt HMACState;  /*  Initialized HMAC state for authentication. */
    unsigned NumSource;             /* Number of source fragments. */
    ubsec_FragmentInfo_pt SourceFragments;  /* Source fragment list */
    unsigned int NumDestination;            /* Number of Destination fragments. */
    ubsec_FragmentInfo_pt DestinationFragments; /* Destination fragment list */
    ubsec_FragmentInfo_t AuthenticationInfo;    /* Authentication output location . */
    unsigned short CryptHeaderSkip;             /* Size of crypt header to skip. */
    void (*CompletionCallback) (unsigned long Context,
         ubsec_Status_t Result);    /* Callback routine on completion. */
    unsigned long CommandContext;   /* Context (ID) of this command). */
} ubsec_CipherCommandInfo_t, *ubsec_CipherCommandInfo_pt;

/*
 * Cipher Command subtype flags.
 */
#define UBSEC_ENCODE            1
#define UBSEC_DECODE            2
#define UBSEC_3DES              4
#define UBSEC_DES               8
#define UBSEC_MAC_MD5           16
#define UBSEC_MAC_SHA1          32

#define UBSEC_AES               4096
#define UBSEC_CTR_MODE          8192    /* Only supported by OPERATION_IPSEC_AES */
#define UBSEC_AES_128BITKEY     16384
#define UBSEC_AES_192BITKEY     32768
#define UBSEC_AES_256BITKEY     65536
/* Reserved for  BCM_OEM_4      131072   */
#define UBSEC_NO_CRYPTO         262144

#define UBSEC_EXPLICIT_IV       131072

/*
 * HMAC Block type. Used to generate a HMAC state which is
 * passed to the API.
 */
typedef unsigned char ubsec_HMAC_Key_t[UBSEC_MAC_KEY_LENGTH],
    *ubsec_HMAC_Key_pt;

typedef struct ubsec_PacketContext_s {
    volatile unsigned int ContextArray[MAX_CONTEXT_SIZE];
    volatile unsigned int PhysicalAddress;
} ubsec_PacketContext_t;

typedef struct ubsec_CipherContextInfo_s {
    ubsec_HMAC_Key_t Key;
    ubsec_CryptKey_t CryptKey;
    unsigned short CryptHeaderSkip;
} ubsec_CipherContextInfo_t;


typedef struct ubsec_CipherCommandInfo_withSC_s {
    ubsec_CipherCommand_t Command;         /* Operation(s) to perform */
    ubsec_PacketContext_t *pContext;       /* Context */
    unsigned NumSource;                    /* Number of source fragments. */
    ubsec_FragmentInfo_pt SourceFragments; /* Source fragment list */
    unsigned int NumDestination;           /* Number of Destination fragments. */
    ubsec_FragmentInfo_pt DestinationFragments; /* Destination fragment list */
    ubsec_FragmentInfo_t AuthenticationInfo;    /* Authentication output location . */
    void (*CompletionCallback) (unsigned long Context, 
          ubsec_Status_t Result);      /* Callback routine on completion. */
    unsigned long CommandContext;      /* Context (ID) of this command). */
} ubsec_CipherCommandInfo_withSC_t;

ubsec_Status_t ubsec_InitHMACState(  ubsec_HMAC_State_pt HMAC_State,
                                     ubsec_CipherCommand_t type, ubsec_HMAC_Key_pt Key);



#endif // _SPUIPSEC_H_
