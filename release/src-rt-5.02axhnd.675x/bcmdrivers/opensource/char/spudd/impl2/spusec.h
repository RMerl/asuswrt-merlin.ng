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
 *  Description: Header file for device API functions
 *  File: spusec.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *  Device header file for IPSec SPU device
 *
 *****************************************************************************/
#ifndef _SPUSEC_H_
#define _SPUSEC_H_

/*
 * Device_Context is handle for all ubsec device operations. It
 * is assigned at initialization time.
 */
typedef void *ubsec_DeviceContext_t, **ubsec_DeviceContext_pt;

#ifdef PAD_ALIGN64_APP32
#define PADIT(name,size)  char name[size];
#else
#define PADIT(name,size)
#endif

/*
 * Cryptographic parameter definitions
 */
#define UBSEC_DES_KEY_LENGTH        2    /* long */
#define UBSEC_3DES_KEY_LENGTH       6    /* long */
#define UBSEC_AES_KEY_LENGTH        8    /* long */
#define UBSEC_MAX_CRYPT_KEY_LENGTH  UBSEC_AES_KEY_LENGTH
#define UBSEC_IV_LENGTH             2    /* long */
#define UBSEC_IV_LENGTH_BYTES       8
#define UBSEC_AES_IV_LENGTH         4    /* long */
#define UBSEC_AES_IV_LENGTH_BYTES   16
#define UBSEC_MAX_IV_LENGTH         UBSEC_AES_IV_LENGTH

#define UBSEC_MAC_KEY_LENGTH        64    /* Bytes */
#define UBSEC_MD5_LENGTH            16    /* Bytes */
#define UBSEC_SHA1_LENGTH           20    /* Bytes */
#define UBSEC_HMAC_LENGTH           20    /* Max of MD5/SHA1 */

/*
 * HMAC State type defines the current (inner/outer)
 * Hash state values.
 */
typedef struct ubsec_HMAC_State_s {
    unsigned char InnerState[UBSEC_HMAC_LENGTH];
    unsigned char OuterState[UBSEC_HMAC_LENGTH];
} ubsec_HMAC_State_t, *ubsec_HMAC_State_pt;

typedef unsigned char *ubsec_MemAddress_t;

/*
 * Generic Fragment information type. Length
 * and physical address of fragment defined
 * here.
 */
#define UBSEC_MAX_FRAGMENTS 20
typedef struct ubsec_FragmentInfo_s {
  unsigned short  FragmentLength;      /* Length of the fragment.     */
  unsigned short  FragmentFlags;       /* For Centurtion */
  PADIT(FragmentAddress_pad,8)
  ubsec_MemAddress_t  FragmentAddress; /* Virtual or Physical address */
} ubsec_FragmentInfo_t, *ubsec_FragmentInfo_pt;

/*
 * HMAC Block type. Used to generate a HMAC state which is
 * passed to the API.
 */
typedef unsigned char ubsec_HMAC_Block_t[UBSEC_MAC_KEY_LENGTH],
    *ubsec_HMAC_Block_pt;

/*
 * HMAC Block type. Used to generate a HMAC state which is
 * passed to the API.
 */
typedef unsigned char ubsec_HMAC_Key_t[UBSEC_MAC_KEY_LENGTH],
    *ubsec_HMAC_Key_pt;

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
    ubsec_CipherCommand_t Command;    /* Operation(s) to perform */
    ubsec_IV_pt InitialVector;        /* IV for CBC operation. */
    ubsec_CryptKey_pt CryptKey;       /* For CBC operation. */
    ubsec_HMAC_State_pt HMACState;    /* Initialized HMAC state for authentication. */
    unsigned NumSource;               /* Number of source fragments. */
    ubsec_FragmentInfo_pt SourceFragments; /* Source fragment list */
    unsigned int NumDestination;           /* Number of Destination fragments. */
    ubsec_FragmentInfo_pt DestinationFragments; /* Destination fragment list */
    ubsec_FragmentInfo_t AuthenticationInfo;    /* Authentication output location . */
    unsigned short CryptHeaderSkip;             /* Size of crypt header to skip. */
    void (*CompletionCallback) (unsigned long Context, 
          ubsec_Status_t Result);    /* Callback routine on completion. */
    unsigned long CommandContext;    /* Context (ID) of this command). */
} ubsec_CipherCommandInfo_t, *ubsec_CipherCommandInfo_pt;

#define MAX_CONTEXT_SIZE     24    /* 1(len+optype) +  
                                      1(context flags+offset/reserved) + 
                                      8(AES Key 32bytes) + 
                                      4(AES IV 16bytes) + 
                                      5(Inner State) +
                                      5(Outer State) */

typedef struct ubsec_PacketContext_s {
    VOLATILE unsigned int ContextArray[MAX_CONTEXT_SIZE];
    VOLATILE unsigned int PhysicalAddress;
} ubsec_PacketContext_t;

typedef struct ubsec_CipherContextInfo_s {
    ubsec_HMAC_Key_t Key;
    ubsec_CryptKey_t CryptKey;
    unsigned short CryptHeaderSkip;
} ubsec_CipherContextInfo_t;

typedef struct ubsec_CipherCommandInfo_withSC_s {
    ubsec_CipherCommand_t Command;      /* Operation(s) to perform */
    ubsec_PacketContext_t *pContext;    /* Context */
    unsigned NumSource;                 /* Number of source fragments. */
    ubsec_FragmentInfo_pt SourceFragments; /* Source fragment list */
    unsigned int NumDestination;           /* Number of Destination fragments. */
    ubsec_FragmentInfo_pt DestinationFragments; /* Destination fragment list */
    ubsec_FragmentInfo_t AuthenticationInfo;    /* Authentication output location . */
    void (*CompletionCallback) (unsigned long Context, 
          ubsec_Status_t Result);    /* Callback routine on completion. */
    unsigned long CommandContext;    /* Context (ID) of this command). */
} ubsec_CipherCommandInfo_withSC_t;

/*
 * Cipher Command subtype flags.
 */
#define UBSEC_ENCODE        1
#define UBSEC_DECODE        2
#define UBSEC_3DES          4
#define UBSEC_DES           8
#define UBSEC_MAC_MD5       16
#define UBSEC_MAC_SHA1      32

#define UBSEC_AES             4096
#define UBSEC_CTR_MODE        8192    /* Currently only supported by OPERATION_IPSEC_AES */
#define UBSEC_AES_128BITKEY   16384
#define UBSEC_AES_192BITKEY   32768
#define UBSEC_AES_256BITKEY   65536
/* Reserved for  BCM_OEM_4    131072     */
#define UBSEC_NO_CRYPTO       262144

/*
 *    Command field definitions.
 */
#define UBSEC_ENCODE_3DES       (UBSEC_ENCODE+UBSEC_3DES)
#define UBSEC_DECODE_3DES       (UBSEC_DECODE+UBSEC_3DES)
#define UBSEC_ENCODE_DES        (UBSEC_ENCODE+UBSEC_DES)
#define UBSEC_DECODE_DES        (UBSEC_DECODE+UBSEC_DES)
#define UBSEC_ENCODE_AES        (UBSEC_ENCODE+UBSEC_AES)
#define UBSEC_DECODE_AES        (UBSEC_DECODE+UBSEC_AES)

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

#define UBSEC_USING_CRYPT(f)    ( (f) & (UBSEC_3DES | UBSEC_DES | UBSEC_AES) )
#define UBSEC_USING_MAC(f)      ( (f) & (UBSEC_MAC_MD5 | UBSEC_MAC_SHA1) )

#define UBSEC_USING_CTR_MODE(f) ( (f) & (UBSEC_CTR_MODE) )

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

#define UBSEC_STATUS_DMA_ALLOC_FAILED              -500
#define UBSEC_STATUS_ENTER_CRITICAL_SECTION_FAILED -501

  /*
   * SRL  API function prototypes.
   */
#ifndef OS_DeviceInfo_t
#define OS_DeviceInfo_t void *
#endif

#ifndef OS_MemHandle_t
#define OS_MemHandle_t void *
#endif


/*
 *
 * Public key operational definitions.
 *
 */

/*
 * The long key type is used as a generic type to hold public
 * key information.
 * KeyValue points to an array of 32-bit integers. The convention of these keys
 * is such that element[0] of this array holds the least significant part of
 * the total "key" (multi-precision integer).
 * Keylength holds the number of significant bits in the key, i.e. the bit
 * position of the most significant "1" bit, plus 1.
 * For example, the multi-precision integer ("key")
 *    0x0102030405060708090A0B0C0D0E0F00
 * has 121 significant bits (KeyLength), and would be arranged in the N-element
 * array (pointed to by KeyLength) of 32-bit integers as
 *    array[0] = 0x0D0E0F00
 *    array[1] = 0x090A0B0C
 *    array[2] = 0x05060708
 *    array[3] = 0x01020304
 *    array[4] = 0x00000000
 *        ...
 *    array[N-1] = 0x00000000
 */

typedef struct ubsec_LongKey_s {
    unsigned int KeyLength;     /* length in bits */
     PADIT(KeyValue_pad, 8)
    OS_MemHandle_t KeyValue;    /* pointer to 32-bit integer "key" array */
} ubsec_LongKey_t, *ubsec_LongKey_pt;

/*
 * Diffie-Hellman parameter type definition.
 */
typedef struct ubsec_DH_Params_t {
    ubsec_LongKey_t Y; /* Public value, in (UBSEC_DH_SHARED), out (UBSEC_DH_PUBLIC) */
    ubsec_LongKey_t X; /* Secret value, in (UBSEC_DH_SHARED), out (UBSEC_DH_PUBLIC) */
    ubsec_LongKey_t K; /* Shared secret value, out (UBSEC_DH_SHARED) */
    ubsec_LongKey_t N; /* Modulus, in (UBSEC_DH_SHARED), out (UBSEC_DH_PUBLIC) */
    ubsec_LongKey_t G; /* Generator, in (UBSEC_DH_PUBLIC) */
    ubsec_LongKey_t UserX; /* Optional user supplied secret value, 
                              in (UBSEC_DH_PUBLIC) */

    unsigned short RandomKeyLen; /* Random key length */
    unsigned short RNGEnable;    /* Generate random secret value if set, 
                                    ignore user supplied. */
} ubsec_DH_Params_t, *ubsec_DH_Params_pt;

/*
 * RSA parameter type definition.
 */
typedef struct ubsec_RSA_Params_t {
    ubsec_LongKey_t OutputKeyInfo;    /* Output data. */
    ubsec_LongKey_t InputKeyInfo;     /* Input data. */
    ubsec_LongKey_t ModN;             /* Modulo N value to be applied */
    ubsec_LongKey_t ExpE;             /* BaseG value to be applied. */
    ubsec_LongKey_t PrimeP;           /* Prime P value */
    ubsec_LongKey_t PrimeQ;           /* Prime Q value */
    ubsec_LongKey_t PrimeEdp;         /* Private exponent edp. */
    ubsec_LongKey_t PrimeEdq;         /* Private exponent edq.  */
    ubsec_LongKey_t Pinv;             /* Pinv value. */
} ubsec_RSA_Params_t, *ubsec_RSA_Params_pt;

/*
 * DSA parameter type definition.
 */
typedef struct ubsec_DSA_Params_t {
    unsigned int NumInputFragments;       /* Number of source fragments. */
     PADIT(InputFragments_pad, 8)
    ubsec_FragmentInfo_pt InputFragments; /* Source fragment list for 
                                             unhashed message */
    ubsec_LongKey_t SigR;     /* Signature R value (input on verify, 
                                                    output on sign) */
    ubsec_LongKey_t SigS;     /* Signature S value (input on verify, 
                                                    output on sign) */
    ubsec_LongKey_t ModQ;     /* Modulo Q value to be applied */
    ubsec_LongKey_t ModP;     /* Modulo P value to be applied */
    ubsec_LongKey_t BaseG;    /* BaseG value to be applied. */
    ubsec_LongKey_t Key;      /* User supplied public (verify) or 
                                 private (sign) key. */
    ubsec_LongKey_t Random;   /* Random value optionally provided by user (sign) */
    ubsec_LongKey_t V;        /* Verification value (verify) */
    unsigned short RandomLen; /* Random value length (sign) */
    unsigned short RNGEnable; /* Random value generated on-chip. (sign) */
    unsigned short HashEnable;/* Enable Chip hash */
} ubsec_DSA_Params_t, *ubsec_DSA_Params_pt;

/*
 * Generic key command parameters
 */
typedef union ubsec_KeyCommandParams_u {
    ubsec_DH_Params_t DHParams;      /* DH parameters  */
    ubsec_RSA_Params_t RSAParams;    /* RSA Parameters */
    ubsec_DSA_Params_t DSAParams;    /* RSA Parameters */
} ubsec_KeyCommandParams_t, *ubsec_KeyCommandParams_pt;

/* Key command type defines Public key operation. */
typedef long ubsec_KeyCommand_t;

/* Key command types. */
#define UBSEC_DH          0x0001
#define UBSEC_RSA         0x0002
#define UBSEC_DSA         0x0004
#define UBSEC_KEY_PRIVATE 0x0010
#define UBSEC_KEY_PUBLIC  0x0020
#define UBSEC_SIGN        0x0040
#define UBSEC_VERIFY      0x0080

#define UBSEC_DH_PUBLIC        (UBSEC_DH+UBSEC_KEY_PUBLIC)
#define UBSEC_DH_SHARED        (UBSEC_DH+UBSEC_KEY_PRIVATE)
#define UBSEC_RSA_PUBLIC       (UBSEC_RSA+UBSEC_KEY_PUBLIC)
#define UBSEC_RSA_PRIVATE      (UBSEC_RSA+UBSEC_KEY_PRIVATE)
#define UBSEC_DSA_SIGN         (UBSEC_DSA+UBSEC_SIGN)
#define UBSEC_DSA_VERIFY       (UBSEC_DSA+UBSEC_VERIFY)

/*
 * Key command struture defines the parameters of a cipher
 * command, its input and output data areas along with the
 * context.
 */
typedef struct ubsec_KeyCommandInfo_s {
    ubsec_KeyCommand_t Command;          /* Operation(s) to perform */
    ubsec_KeyCommandParams_t Parameters; /* Associated parameters. */
    void (*CompletionCallback) (unsigned long Context, 
          ubsec_Status_t Result);    /* Callback routine on completion. */
    unsigned long CommandContext;    /* Context (ID) of this command). */
} ubsec_KeyCommandInfo_t, *ubsec_KeyCommandInfo_pt;

/*
 * SSL/TLS/ARC4 Command prototype definitions
 */

/*
 *  SSL Command definitions. These commands will be ored in with
 * crypto commands.
 */

#define UBSEC_SSL_HMAC         (0x01000)
#define UBSEC_SSL_MAC         (0x01000)
#define UBSEC_SSL_CRYPTO      (0x02000)
#define UBSEC_TLS             (0x04000)
#define UBSEC_ARC4            (0x08000)
#define UBSEC_HASH            (0x10000)
#define UBSEC_SSL_NEWMCR      (0x20000)

#define UBSEC_SSL_3DES_ENCODE (UBSEC_SSL_CRYPTO+UBSEC_ENCODE+UBSEC_3DES)
#define UBSEC_SSL_3DES_DECODE (UBSEC_SSL_CRYPTO+UBSEC_DECODE+UBSEC_3DES)
#define UBSEC_ARC4_ENCODE     (UBSEC_ARC4)

#define UBSEC_SSL_HMAC_MD5    (UBSEC_SSL_HMAC+UBSEC_MAC_MD5)
#define UBSEC_SSL_HMAC_SHA1   (UBSEC_SSL_HMAC+UBSEC_MAC_SHA1)
#define UBSEC_SSL_MAC_MD5     (UBSEC_SSL_MAC+UBSEC_MAC_MD5)
#define UBSEC_SSL_MAC_SHA1    (UBSEC_SSL_MAC+UBSEC_MAC_SHA1)
#define UBSEC_TLS_HMAC_MD5    (UBSEC_TLS+UBSEC_MAC_MD5)
#define UBSEC_TLS_HMAC_SHA1   (UBSEC_TLS+UBSEC_MAC_SHA1)
#define UBSEC_HASH_SHA1       (UBSEC_HASH | UBSEC_MAC_SHA1)
#define UBSEC_HASH_MD5        (UBSEC_HASH | UBSEC_MAC_MD5)

#define UBSEC_ARC4_STATE_WRITEBACK  0x0001
#define UBSEC_ARC4_STATE_STATEKEY   0x0002
#define UBSEC_ARC4_STATE_NULL_DATA  0x0004

#define UBSEC_SSL_COMMAND_MASK (UBSEC_SSL_MAC+UBSEC_SSL_CRYPTO+UBSEC_TLS+UBSEC_ARC4+UBSEC_HASH+UBSEC_SSL_NEWMCR)
#define UBSEC_SSL_COMMAND(command) (command & (UBSEC_SSL_COMMAND_MASK))

/*
 * Type Definitions:
 */
typedef unsigned char ubsec_SSLMAC_key_t[UBSEC_HMAC_LENGTH],
    *ubsec_SSLMAC_key_pt;

/* Sequence number type. Double DWORD. */
typedef struct ubsec_DoubleSequenceNumber_s {
    unsigned int HighWord;
    unsigned int LowWord;
} ubsec_DoubleSequenceNumber_t, *ubsec_DoubleSequenceNumber_pt;

typedef struct ubsec_SSLMACParams_s {
    ubsec_FragmentInfo_t OutputHMAC; /* output MAC */
    ubsec_SSLMAC_key_t key;          /* MAC key */
    ubsec_DoubleSequenceNumber_t SequenceNumber;    /* sequence number */
    unsigned char ContentType;       /* content type */
    unsigned short DataLength;
} ubsec_SSLMACParams_t, *ubsec_SSLMACParams_pt;

typedef struct ubsec_TLSHMACParams_s {
    ubsec_FragmentInfo_t OutputHMAC;    /* output MAC */
    ubsec_HMAC_State_pt HMACState;      /* HMAC State */
    ubsec_DoubleSequenceNumber_t SequenceNumber;    /* sequence number */
    unsigned char ContentType;    /* content type */
    unsigned short Version;       /* Version */
    unsigned short DataLength;
} ubsec_TLSHMACParams_t, *ubsec_TLSHMACParams_pt;

typedef struct ubsec_SSLCipherParams_t {
    ubsec_IV_t InitialVector;    /* initial vector */
    unsigned int CryptKey[UBSEC_3DES_KEY_LENGTH];
} ubsec_SSLCipherParams_t, *ubsec_SSLCipherParams_pt;

#define UBSEC_ARC4_KEYSTATE_BYTES (260)
typedef unsigned char ubsec_ARC4_State_t[UBSEC_ARC4_KEYSTATE_BYTES],
    *ubsec_ARC4_State_pt;

typedef struct ubsec_SSLARC4Params_t {
    ubsec_ARC4_State_pt KeyStateIn; /* key or state data */
    unsigned int KeyStateFlag;      /* start with key or start from flag */
    ubsec_FragmentInfo_t state_out; /* state upon completing this arc4 operation */
} ubsec_ARC4Params_t, *ubsec_ARC4Params_pt;

typedef struct ubsec_HashParams_t {
    ubsec_FragmentInfo_t OutputHMAC;    /* output MAC */
} ubsec_HashParams_t, *ubsec_HashParams_pt;

typedef union ubsec_SSLParams_u {
    ubsec_SSLMACParams_t SSLMACParams;
    ubsec_SSLCipherParams_t SSLCipherParams;
    ubsec_TLSHMACParams_t TLSHMACParams;
    ubsec_ARC4Params_t ARC4Params;
    ubsec_HashParams_t HashParams;
} ubsec_SSLCommandParams_t, *ubsec_SSLCommandParams_pt;

typedef unsigned int ubsec_SSLCommand_t;

typedef struct ubsec_SSLCommandInfo_s {
    ubsec_SSLCommand_t Command;
    ubsec_SSLCommandParams_t Parameters;
    unsigned long CommandContext;
    unsigned int NumSource;                     /* Number of source fragments. */
    ubsec_FragmentInfo_pt SourceFragments;      /* Source fragment list */
    unsigned int NumDestination;                /* Number of Destination fragments. */
    ubsec_FragmentInfo_pt DestinationFragments; /* Destination fragment list */
    void (*CompletionCallback) (unsigned long Context,
                    ubsec_Status_t Result);
} ubsec_SSLCommandInfo_t, *ubsec_SSLCommandInfo_pt;

  /* SSL command execute function. */
UBSECAPI ubsec_Status_t
ubsec_SSLCommand(ubsec_DeviceContext_t Context,
         ubsec_SSLCommandInfo_pt command, int *NumCommands);

/* Register access routines */
UBSECAPI ubsec_Status_t
ubsec_rw_register(void *Context, unsigned int Offset, unsigned char * UserData,
          unsigned int Size, unsigned int Read, unsigned int Hammer);

/*
 * Extended ChipInfo prototype definitions
 */

/* Parameter structure, used at IOCTL and in SRL */
typedef struct ubsec_chipinfo_io_s {
    unsigned int Status;
    unsigned int CardNum;
    unsigned int MaxKeyLen;
    unsigned short DeviceID;
    unsigned int BaseAddress;
    int IRQ;
    int NumDevices;
    unsigned int Features;
} ubsec_chipinfo_io_t, *ubsec_chipinfo_io_pt;

/* ubsec_chipinfo_io_pt->features bit definitions */
#define UBSEC_EXTCHIPINFO_SRL_BE         0x00000001
#define UBSEC_EXTCHIPINFO_CPU_BE         0x00000002
#define UBSEC_EXTCHIPINFO_ARC4_NULL      0x00000004
#define UBSEC_EXTCHIPINFO_ARC4           0x00000008
#define UBSEC_EXTCHIPINFO_3DES           0x00000010
#define UBSEC_EXTCHIPINFO_RNG            0x00000020
#define UBSEC_EXTCHIPINFO_DBLMODEXP      0x00000040
#define UBSEC_EXTCHIPINFO_KEY_OVERRIDE   0x00000080
#define UBSEC_EXTCHIPINFO_SSL            0x00000100
#define UBSEC_EXTCHIPINFO_AES            0x00000200
#define UBSEC_EXTCHIPINFO_KEY            0x00000400
#define UBSEC_EXTCHIPINFO_MATH           0x00000800
#define UBSEC_EXTCHIPINFO_RESERVED1      0x00001000

/*
 * DVT prototype definitions
 */


/* Raw packet command structure. */
typedef long ubsec_DVTCommand_t;

#define MAX_PACKETINFO_SIZE     20    
typedef struct ubsec_RawCommandInfo_s {
  ubsec_DVTCommand_t Command;      /* Operation(s) to perform */
  ubsec_PacketContext_t *pContext; /* Context */
  unsigned ChannelNum;             /* MCR channel */
  unsigned NumSource;              /* Number of source fragments. */
  ubsec_FragmentInfo_t SourceFragments[UBSEC_MAX_FRAGMENTS];/* Source fragment list */
  unsigned NumDestination;    /* Number of Destination fragments. */
  ubsec_FragmentInfo_t DestinationFragments[UBSEC_MAX_FRAGMENTS];/* Destination 
                                                                    fragment list */
  ubsec_FragmentInfo_t PhysContext;    /* Physical address and size of context */
  void (*CompletionCallback) (unsigned long Context, 
        ubsec_Status_t Result);    /* Callback routine on completion. */
  unsigned long CommandContext;    /* Context (ID) of this command). */
  unsigned int  PacketInfo[MAX_PACKETINFO_SIZE];
} ubsec_RawCommandInfo_t, *ubsec_RawCommandInfo_pt;


/* Parameter structure, used at IOCTL and in SRL */
typedef struct DVT_Params_s {
    int Command;
    int CardNum;
    unsigned int InParameter;
    unsigned int OutParameter;
    unsigned long Status;
    /* Add new structure members to end of existing member list */
} DVT_Params_t, *DVT_Params_pt;

/* DVT Command codes */
#define UBSEC_DVT_MCR1_SUSPEND    1
#define UBSEC_DVT_MCR1_RESUME     2
#define UBSEC_DVT_MCR2_SUSPEND    3
#define UBSEC_DVT_MCR2_RESUME     4
#define UBSEC_DVT_NEXT_MCR1       5
#define UBSEC_DVT_NEXT_MCR2       6
#define UBSEC_DVT_PAGESIZE        7
#define UBSEC_DVT_ALL_MCR_RESUME  8
#define UBSEC_DVT_VERY_RAW_COMMMAND  9
/* Out if order but maintain for posterity */
#define UBSEC_DVT_MCR3_SUSPEND    9
#define UBSEC_DVT_MCR3_RESUME     10
#define UBSEC_DVT_MCR4_SUSPEND    11
#define UBSEC_DVT_MCR4_RESUME     12
#define UBSEC_DVT_NEXT_MCR3       13
#define UBSEC_DVT_NEXT_MCR4       14
#define UBSEC_DVT_ALL_SUSPEND     15
#define UBSEC_DVT_ALL_RESUME      16


/* pDevice->DVTOptions bit definitions */
#define UBSEC_SUSPEND_MCR1        0x00000001
#define UBSEC_SUSPEND_MCR2        0x00000002
#define UBSEC_SUSPEND_MCR3        0x00000004
#define UBSEC_SUSPEND_MCR4        0x00000008

ubsec_Status_t
ubsec_DVTControl(void *context, ubsec_DVTCommand_t Command);

ubsec_Status_t
ubsec_GetOptions(void *context, unsigned int *Options);

ubsec_Status_t
ubsec_SetOptions(void *Context, unsigned int Options);

ubsec_Status_t
ubsec_RawCommand(ubsec_DeviceContext_t Context,
         ubsec_RawCommandInfo_pt pCommand);

ubsec_Status_t
ubsec_dvtregister(void *Context, unsigned int Offset, unsigned char * UserData,
          unsigned int Size, unsigned int Read, unsigned int Hammer);

#ifndef BCM_OEM_4
#define BCM_OEM_4
#endif

#ifdef BCM_OEM_4
#include  "bcm_oem_4_ext.h"
#endif

#endif                /* _UBSEC_H_ */
