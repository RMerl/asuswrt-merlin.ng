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
 *  Description: Header file for Generic Contexts and Generic Headers 
 *  File: cnctx.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *
 *****************************************************************************/
#ifndef __CNCTX_H_
#define __CNCTX_H_

#pragma pack(1)

/* Notes :
 * This file duplicates the bit/fields defined in the BCM63xx Datasheet 
 * The names[caps/underscore in names] are kept almost similar to the Datasheet
 * union's where avoided in because the big/little make the file too unreadable.
 * Moreover all fields had unique access and never expected to be accessible 
 * in any other data type  -personal perference 
 */

/* Command Context Header - inlines */
/* field flag in CCH */

typedef struct _CCH_field_flag {
    uint8_t SCTX_PR:1;          /* [31] */
    uint8_t BCT_PR_KPARAM_PR:1; /* [30] */
    uint8_t BDESC_PR_HS_PR:1;   /* [29] */
    uint8_t MFM_PR:1;           /* [28] */
    uint8_t BD_PR:1;            /* [27] */
    uint8_t HASH_PR:1;          /* [26] */
    uint8_t SPS_PR:1;           /* [25] */
    uint8_t SUPDT_PR:1;         /* [24] */
} CCH_field_flag;

/* Command Context Header */

typedef struct _CCH {
    union {
        CCH_field_flag flag; /* [31:24] */
        uint8_t flag_bits;
    } field;
    uint8_t opCode;          /* [23:16] */
    uint16_t reserved:4;     /* [15:12] */
    uint16_t length:12;      /* [11:0] */
    uint32_t ech;
} CCH;

/* Generic Mode Security Context Structure [SCTX] -inlines */

typedef struct _SCTX_ICV_MAC_flags {
    uint8_t insert_icv_mac:1;    /* [13] */
    uint8_t check_icv_mac:1;     /* [12] */
} SCTX_ICV_MAC_flags;

typedef struct _SCTX_IV_flags {
    uint8_t SCTX_IV:1;     /* [7] */
    uint8_t explicit_IV:1; /* [6] */
    uint8_t gen_IV:1;      /* [5] */
} SCTX_IV_flags;

typedef struct {
    uint32_t type:2;         /* [31:30] */
    uint32_t reserved:22;    /* [29:8] */
    uint32_t SCTX_size:8;    /* [7:0] */
} SCTX_generic_protocol_flags;

typedef struct {
    uint32_t type:2;           /* [31:30] */
    uint32_t cacheable:1;      /* [29] */
    uint32_t update_en:1;      /* [28] */
    uint32_t lock_en:1;        /* [27] */
    uint32_t cap_en:1;         /* [26] */
    uint32_t ctype:2;          /* [25:24] */
    uint32_t role:1;           /* [23] */
    uint32_t protocol:2;       /* [22:21] */
    uint32_t reserved:1;       /* [20] */
    uint32_t pad_en_pad_chk:1; /* [19] */
    uint32_t reserved_01:9;    /* [18:10] */
    uint32_t export:2;         /* [9:8] */
    uint32_t SCTX_size:8;      /* [7:0] */
} SCTX_ssl_protocol_flags;

typedef struct {
    uint32_t type:2;           /* [31:30] */
    uint32_t cacheable:1;      /* [29] */
    uint32_t update_en:1;      /* [28] */
    uint32_t lock_en:1;        /* [27] */
    uint32_t cap_en:1;         /* [26] */
    uint32_t reserved:2;       /* [25:24] */
    uint32_t transport:1;      /* [23] */
    uint32_t AH:1;             /* [22] */
    uint32_t ESP:1;            /* [21] */
    uint32_t authSeq64:1;      /* [20] */
    uint32_t UDP_en:1;         /* [19] */
    uint32_t reserved_1:3;     /* [18:16] */
    uint32_t pad_en_pad_chk:1; /* [15] */
    uint32_t gen_ID:1;         /* [14] */
    uint32_t twoDestOpts:1;    /* [13] */
    uint32_t IPv4ChkSum:1;     /* [12] */
    uint32_t decrement_TTL:1;  /* [11] */
    uint32_t copy_TOS:1;       /* [10] */
    uint32_t copy_Flow:1;      /* [9] */
    uint32_t DF_Copy:1;        /* [8] */
    uint32_t SCTX_size:8;      /* [7:0] */
} SCTX_ipsec_protocol_flags;

typedef struct {
    uint32_t PK:1;             /* [31] */
    uint32_t KEK:1;            /* [30] */
    uint32_t reserved_2:2;     /* [29:28] */
    uint32_t AKC_handle:8;     /* [27:20] */
    uint32_t reserved_21:6;    /* [19:14] */
} bsafe_fields;

typedef struct {
    uint32_t inbound:1;     /* [31] */
    uint32_t order:1;       /* [30] */
    uint32_t reserved_1:6;  /* [29:24] */
    uint32_t cryptoAlg:3;   /* [23:21] */
    uint32_t cryptoMode:3;  /* [20:18] */
    uint32_t cryptoType:2;  /* [17:16] */
    uint32_t hashAlg:3;     /* [15:13] */
    uint32_t hashMode:3;    /* [12:10] */
    uint32_t hashType:2;    /* [9:8] */
    uint32_t UPDT_ofst:8;   /* [7:0] */
} SCTX_cipher_flags;
typedef struct {
    uint32_t PK:1;               /* [31] */
    uint32_t KEK:1;              /* [30] */
    uint32_t reserved_2:2;       /* [29:28] */
    uint32_t AKC_handle:8;       /* [27:20] */
    uint32_t reserved_21:6;      /* [19:14] */
    uint32_t insert_icv_mac:1;   /* [13] */
    uint32_t check_icv_mac:1;    /* [12] */
    uint32_t ICV_MAC_size:4;     /* [11:8] */
    uint32_t SCTX_IV:1;          /* [7] */
    uint32_t explicit_IV:1;      /* [6] */
    uint32_t gen_IV:1;           /* [5] */
    uint32_t reserved_22:2;      /* [4:3] */
    uint32_t explicit_IV_size:3; /* [2:0] */
} SCTX_extended_cipher_flags;

/* Generic Mode Security Context Structure [SCTX] */
typedef struct _SCTX {

/* word 0 *//* protocol flags */
    union {            /* [31:0] */
        uint32_t bits;
        SCTX_generic_protocol_flags generic_flags;
        SCTX_ssl_protocol_flags ssl_flags;
        SCTX_ipsec_protocol_flags ipsec_flags;
    } protocol;

/* word 1 *//* cipher flags */
    union {
        uint32_t bits;
        SCTX_cipher_flags flags;
    } cipher;

/* word 2 *//* Extended cipher flags */
    union {
        uint32_t bits;
        SCTX_extended_cipher_flags flags;
    } ecf;

    /* 
     * Rest of the struct is context specific filled in appropriately 
     * by the formatting routine 
     */
} SCTX;


/* CCH & SCTX context *//* a simple generic context */
typedef struct _CCH_SCTX {
    CCH cch;
    SCTX sctx;
} CCH_SCTX;

typedef struct _BD {
    uint16_t dataSize;
    uint16_t reserved;
} BD;

/* in Bytes */
#define MD5_STATE_SIZE     16
#define SHA1_STATE_SIZE    20

#define ARC4_KEY_SIZE      260

#define DES3_KEY_SIZE      8*3
#define DES_KEY_SIZE       8
#define DES_IV_SIZE        8

#define AES128_KEY_SIZE    16
#define AES192_KEY_SIZE    24
#define AES256_KEY_SIZE    32
#define AES_IV_SIZE        16

/* 
 * Various mode/alg/types from tables from Encryption/authentication 
 * algorithms and modes 
 */
/* CryptoAlgo */
#define CRYPTO_ALG_NULL       0x0
#define CRYPTO_ALG_ARC4       0x1
#define CRYPTO_ALG_DES        0x2
#define CRYPTO_ALG_DES3       0x3
#define CRYPTO_ALG_AES        0x4
#define CRYPTO_ALG_KASUMI_F9  0x5

/* CryptoMode */
#define CRYPTO_MODE_ECB        0x0
#define CRYPTO_MODE_CBC        0x1
#define CRYPTO_MODE_OFB        0x2
#define CRYPTO_MODE_CFB        0x3
#define CRYPTO_MODE_CTR        0x4
#define CRYPTO_MODE_CCM        0x5

/* CryptoType */
#define CRYPTO_TYPE_00        0x0
#define CRYPTO_TYPE_01        0x1
#define CRYPTO_TYPE_10        0x2
#define CRYPTO_TYPE_11        0x3
#define CRYPTO_TYPE_KEY128    0x0
#define CRYPTO_TYPE_KEY192    0x1
#define CRYPTO_TYPE_KEY256    0x2

/* HashAlg */
#define HASH_ALG_NULL       0x0
#define HASH_ALG_MD5        0x1
#define HASH_ALG_SHA1       0x2
#define HASH_ALG_SHA256     0x3
#define HASH_ALG_AES        0x4
#define HASH_ALG_KASUMI_F9  0x5

/* HashMode */
#define HASH_MODE_HASH      0x0
#define HASH_MODE_CTXT      0x1
#define HASH_MODE_HMAC      0x2
#define HASH_MODE_SSLMAC    0x3
#define HASH_MODE_CCM       0x4
#define HASH_MODE_XCBC      0x5

/* HashType */
#define HASH_TYPE_00        0x0
#define HASH_TYPE_01        0x1
#define HASH_TYPE_10        0x2
#define HASH_TYPE_11        0x3
#define HASH_TYPE_KEY128    0x0
#define HASH_TYPE_KEY192    0x1
#define HASH_TYPE_KEY256    0x2
#define HASH_FULL           0x0
#define HASH_INIT           0x1
#define HASH_UPDT           0x2
#define HASH_FIN            0x3

/* Role */
#define SCTX_ROLE_SERVER    0x0
#define SCTX_ROLE_CLIENT    0x1

/* Order */
#define ORDER_OB_ENCAUTH_IB_AUTHDEC        0x0
#define ORDER_OB_AUTHENC_IB_DECAUTH        0x1

/* Protocol */
#define PROTOCOL_SSLV2          0x0
#define PROTOCOL_SSLV3          0x1
#define PROTOCOL_TLSV1          0x2

/* Export */
#define EXPORT_CIPHER_DOMESTIC    0x0
#define EXPORT_CIPHER_40          0x1
#define EXPORT_CIPHER_56          0x2
#define EXPORT_RESERVED           0x3

/* Export key sizes */
#define EXPORT_CIPHER_40_SIZE    40
#define EXPORT_CIPHER_56_SIZE    56

/* Type */
#define TYPE_GENERIC        0x0
#define TYPE_IPSEC          0x1
#define TYPE_SSL_TLS        0x2

/* Inbound */
#define SCTX_OUTBOUND       0x0
#define SCTX_INBOUND        0x1

#pragma pack()

#endif /* __CNCTX_H_ */

