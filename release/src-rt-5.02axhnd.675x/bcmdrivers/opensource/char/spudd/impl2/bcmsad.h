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
 *  Description: Header file for BCMSAD SA Database API functions
 *  File: bcmsad.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *
 *****************************************************************************/

#ifndef _BCMSAD_H_
#define _BCMSAD_H_

/* Connection information */
typedef struct {
    IPADDR ipSrcStart;    /* start source IP address */
    IPADDR ipSrcEnd;      /* end source IP address */
    IPADDR ipDstStart;    /* start destination IP address */
    IPADDR ipDstEnd;      /* end destination IP address */
    IPADDR gwSrc;         /* Source gateway IP address */
    IPADDR gwDst;         /* Destination gateway IP address */
    uint16_t flags;
    uint16_t ipProt;      /* IP protocol */
    uint16_t srcPort;     /* source port */
    uint16_t dstPort;     /* destination port */
} BCMCONN;

/* Outbound five-tuple IP flow information */
typedef struct {
    IPADDR src;           /* source IP address */
    IPADDR dst;           /* destination IP address */
    uint16_t prot;        /* IP protocol */
    uint16_t src_prt;     /* source port */
    uint16_t dst_prt;     /* destination port */
    uint16_t flags;
} BCMFLOW;

typedef struct {
    BCMSADH handle;        /* SA handle */
    uint8_t encrAlg;       /* Encryption algorithm BCMSAD_ALG_ENCR_XX */
    uint8_t authAlg;       /* Authentication Algorithm BCMSAD_ALG_AUTH_XX */
    uint16_t encrKeyLen;   /* Encryption algorithm key length */
    uint16_t authKeyLen;   /* Authentication algorithm key length */
    uint16_t hashLen;      /* Authentication truncated hash length */
    uint32_t spi;          /* SPI for the outermost encapsulation */
    uint32_t innerSpi;     /* ESP SPI in case of an AH/ESP bundle */
    uint8_t *encrKey;      /* Encryption key */
    uint8_t *authKey;      /* Authentication key */
    CRYPTOOPFLAGS flags;   /* flags indicating operation/SA specifics */
    BCMCONN conn;          /* Connection information */
    EXTLONG lifeTime;      /* lifetime in seconds */
    EXTLONG lifeBytes;     /* lifetime in Kbytes */
    EXTLONG rekeyTime;     /* soft lifetime in seconds */
    EXTLONG rekeyBytes;    /* soft lifebyte in Kbytes */
    EXTLONG dataBytes;     /* number of Kbytes used by this SA */
    void *hostSA;          /* pointer to a proprietary host SA */
} BCMSAD;

/* SADB states */
#define BCMSAD_STATE_FREE        0    /* Entry is free for use */
#define BCMSAD_STATE_CREATED     1    /* SPI is allocated */
#define BCMSAD_STATE_ACTIVE      2    /* Fully functional */
#define BCMSAD_STATE_REKEY       3    /* Valid and negotiation pending a new SA */
#define BCMSAD_STATE_EXPIRED     4    /* Expired waiting for deletion */
#define BCMSAD_STATE_MAX         4
#define BCMSAD_STATE_MASK        ((uint32_t)0xF<<28)    /* 4 bits */
#define BCMSAD_STATE_SET(x)      (((uint32_t)(x)<<28)&BCMSAD_STATE_MASK)
#define BCMSAD_STATE_GET(x)      (((uint32_t)(x)&BCMSAD_STATE_MASK)>>28)

/* SA protocols */
#define BCMSAD_PROTOCOL_NONE     0    /* Invalid */
#define BCMSAD_PROTOCOL_ESP      1    /* ESP protocol */
#define BCMSAD_PROTOCOL_AH       2    /* AH protocol */
#define BCMSAD_PROTOCOL_ESP_AH   3    /* AH+ESP bundle */
#define BCMSAD_PROTOCOL_MAX      3
#define BCMSAD_PROTOCOL_MASK     ((uint32_t)0xF<<24)    /* 4 bits */
#define BCMSAD_PROTOCOL_SET(x)   (((uint32_t)(x)<<24)&BCMSAD_PROTOCOL_MASK)
#define BCMSAD_PROTOCOL_GET(x)   (((uint32_t)(x)&BCMSAD_PROTOCOL_MASK)>>24)

/* SA directions */
#define BCMSAD_DIR_NONE            0    /* Invalid */
#define BCMSAD_DIR_INBOUND         1    /* Inbound SA */
#define BCMSAD_DIR_OUTBOUND        2    /* Outbound SA */
#define BCMSAD_DIR_MAX             2
#define BCMSAD_DIR_MASK          ((uint32_t)0x3<<22)    /* 2 bits */
#define BCMSAD_DIR_SET(x)        (((uint32_t)(x)<<22)&BCMSAD_DIR_MASK)
#define BCMSAD_DIR_GET(x)        (((uint32_t)(x)&BCMSAD_DIR_MASK)>>22)

/* SA rekey */
#define BCMSAD_REKEY_NONE         0    /* Invalid */
#define BCMSAD_REKEY_NEVER        1    /* Never rekey */
#define BCMSAD_REKEY_ALWAYS       2    /* Always rekey */
#define BCMSAD_REKEY_TRAFFIC      3    /* Traffic causes rekey */
#define BCMSAD_REKEY_MAX          3
#define BCMSAD_REKEY_MASK       ((uint32_t)0xF<<6)    /* 4 bits */
#define BCMSAD_REKEY_SET(x)     (((uint32_t)(x)<<6)&BCMSAD_REKEY_MASK)
#define BCMSAD_REKEY_GET(x)     (((uint32_t)(x)&BCMSAD_REKEY_MASK)>>6)

/* SA action */
#define BCMSAD_ACT_NONE           0    /* Invalid */
#define BCMSAD_ACT_SECURE         1    /* Secure all packets that match this SA */
#define BCMSAD_ACT_DROP           2    /* Drop all packets that match this SA */
#define BCMSAD_ACT_BYPASS         3    /* Bypass all packets that match this SA */
#define BCMSAD_ACT_MAX            3
#define BCMSAD_ACT_MASK         ((uint32_t)0xF<<18)    /* 4 bits */
#define BCMSAD_ACT_SET(x)       (((uint32_t)(x)<<18)&BCMSAD_ACT_MASK)
#define BCMSAD_ACT_GET(x)       (((uint32_t)(x)&BCMSAD_ACT_MASK)>>18)

/* SA mode */
#define BCMSAD_MODE_NONE          0    /* Invalid */
#define BCMSAD_MODE_TUNNEL        1    /* Tunnel mode */
#define BCMSAD_MODE_TRANSPORT     2    /* Transport mode */
#define BCMSAD_MODE_MAX           2
#define BCMSAD_MODE_MASK        ((uint32_t)0xF<<14)    /* 4 bits */
#define BCMSAD_MODE_SET(x)      (((uint32_t)(x)<<14)&BCMSAD_MODE_MASK)
#define BCMSAD_MODE_GET(x)      (((uint32_t)(x)&BCMSAD_MODE_MASK)>>14)

/* SA Encryption Algorithm */
#define BCMSAD_ENCR_ALG_NONE    0    /* None */
#define BCMSAD_ENCR_ALG_NULL    1    /* Null encryption */
#define BCMSAD_ENCR_ALG_DES     2    /* DES encryption */
#define BCMSAD_ENCR_ALG_3DES    3    /* 3DES encryption */
#define BCMSAD_ENCR_ALG_AES     4    /* AES (Rijndael) encryption */
#define BCMSAD_ENCR_ALG_AES_CTR 5    /* AES counter mode */
#define BCMSAD_ENCR_ALG_MAX     5    /* 16 bits */

/* SA Authentication Algorithm */
#define BCMSAD_AUTH_ALG_NONE      0    /* None */
#define BCMSAD_AUTH_ALG_SHA1      1    /* SHA1 */
#define BCMSAD_AUTH_ALG_MD5       2    /* MD5 */
#define BCMSAD_AUTH_ALG_AES       3    /* AES XCBC */
#define BCMSAD_AUTH_ALG_MAX       4    /* 3 bits */

/* SA negotiation state */
#define BCMSAD_NEG_NONE           0    /* Invalid */
#define BCMSAD_NEG_DYNAMIC        1    /* dynamically negotiated SA */
#define BCMSAD_NEG_MANUAL         2    /* manually defined SA */
#define BCMSAD_NEG_MAX            2
#define BCMSAD_NEG_MASK         ((uint32_t)0x3<<10)    /* 2 bits */
#define BCMSAD_NEG_SET(x)       (((uint32_t)(x)<<10)&BCMSAD_NEG_MASK)
#define BCMSAD_NEG_GET(x)       (((uint32_t)(x)&BCMSAD_NEG_MASK)>>10)

#define SIZE_SPI                    4
#define SIZE_SEQUENCE_NUM           4
#define SIZE_ESP_HEADER             (SIZE_SEQUENCE_NUM+SIZE_SPI)
#define BCMSAD_AUTH_MD5_KEYLEN      16
#define BCMSAD_AUTH_SHA1_KEYLEN     20
#define BCMSAD_ENCR_KEYLEN_DES      8
#define BCMSAD_ENCR_BLOCKLEN_DES    BCMSAD_ENCR_KEYLEN_DES
#define BCMSAD_ENCR_KEYLEN_3DES     (3*BCMSAD_ENCR_KEYLEN_DES)
#define BCMSAD_ENCR_KEYLEN_AES128   16
#define BCMSAD_ENCR_KEYLEN_AES192   24
#define BCMSAD_ENCR_KEYLEN_AES256   32
#define BCMSAD_ENCR_BLOCKLEN_AES    BCMSAD_ENCR_KEYLEN_AES128
#define BCMSAD_ENCR_KEYLEN_MAX      BCMSAD_ENCR_KEYLEN_AES256    

/* Order of Operations: Only for generic packet processing */
#define BCMSAD_ORDER_NONE           0    /* Invalid */
#define BCMSAD_ORDER_CRYPT_AUTH     1    /* Encrypt then authenticate */
#define BCMSAD_ORDER_AUTH_CRYPT     2    /* Authenticate then encrypt */
#define BCMSAD_ORDER_MAX            3    /* 3 bits */

/* Authentication processing: Only for generic packet processing */
#define BCMSAD_AUTH_OP_NONE         0    /* Invalid */
#define BCMSAD_AUTH_OP_DO           1    /* Perform authentication */
#define BCMSAD_AUTH_OP_CHECK        2    /* Check Authentication */
#define BCMSAD_AUTH_OP_MAX          3    /* 3 bits */

/*************************************************************************
**      To be modified as needed to suit the host environment          **
*************************************************************************/
#define BCMSAD_MAX_SA_COUNT        0x20000ul    /* Maximum size of each SADB */
#define BCMSAD_TUNNEL_TTL_DEFAULT  64    /* Default value of TTL in tunnel */
#define BCMSAD_TUNNEL_TOS_DEFAULT   0    /* Default value of TOS in tunnel */
#define BCMSAD_TUNNEL_OPTIONS_MAX  64    /* Maximum size IP options in tunnel */

/* BCMSAD return values */
#define BCMSAD_SUCCESS            0    /* success */
#define BCMSAD_ERR_MEMORY_ALLOC   -1000    /* memory allocation error */
#define BCMSAD_ERR_SA_NOT_FOUND   -1001    /* SA not found */
#define BCMSAD_ERR_SA_EXISTS      -1002    /* SA exists */
#define BCMSAD_ERR_SA_FREE        -1003    /* SA is free */
#define BCMSAD_ERR_SADB_FULL      -1004    /* SADB full */
#define BCMSAD_ERR_SA_CORRUPTED   -1005    /* SA entry is corrupted */
#define BCMSAD_ERR_BAD_HANDLE     -1006    /* SA handle is invalid */
#define BCMSAD_ERR_BAD_CONN_INFO  -1007    /* Conn info does not match criteria */
#define BCMSAD_ERR_MISS_CONN      -1008    /* Conn information is incomplete */
#define BCMSAD_ERR_MISS_ENC_KEY   -1009    /* SA has no encryption key */
#define BCMSAD_ERR_MISS_AUTH_KEY  -1010    /* SA has no authentication key */
#define BCMSAD_ERR_MISS_ENC_PROT  -1011    /* SA has no encryption protocol */
#define BCMSAD_ERR_MISS_AUTH_PROT -1012    /* SA has no authentication protocol */
#define BCMSAD_ERR_MISS_LIFETIME  -1013    /* SA has no have lifetime/byte info */
#define BCMSAD_ERR_MISS_TUNNEL    -1014    /* Tunnel information incomplete */
#define BCMSAD_ERR_FLOW_UNKNOWN   -1015    /* No matching flow entry */
#define BCMSAD_ERR_FLOW_BAD_SA    -1016    /* SA does not match flow entry */
#define BCMSAD_ERR_DEV_NOT_FOUND  -1017    /* device not found */
#define BCMSAD_ERR_FLOW_NOT_FOUND -1018    /* device not found */
#define BCMSAD_ERR_FLOW_PROTECTED -1019    /* device not found */
#define BCMSAD_ERR_SA_EXPIRED     -1020    /* SA entry has expired */
#define BCMSAD_ERR_SA_NEGOTIATING -1021    /* SA negotiation in progress */
#define BCMSAD_ERR_BAD_PROTOCOL   -1022    /* SA has invalid protocol type */
#define BCMSAD_ERR_BAD_MODE       -1023    /* SA has invalid mode type */
#define BCMSAD_ERR_BAD_ACTION     -1024    /* SA has invalid policy action */
#define BCMSAD_ERR_BAD_KEY_SIZE   -1025    /* SA has invalid AES key size */
#define BCMSAD_ERR_BAD_NEG_TYPE   -1026    /* SA has invalid Negotiation type */
#define BCMSAD_ERR_BAD_REKEY_TYPE -1027    /* SA has invalid Rekey type */
#define BCMSAD_ERR_BAD_REKEY_INFO -1028    /* SA has invalid Rekey info */
#define BCMSAD_ERR_BAD_DIR        -1029    /* SA direction does not match input */
#define BCMSAD_ERR_BAD_SPI        -1030
#define BCMSAD_ERR_BAD_ENCR_ALG   -1031
#define BCMSAD_ERR_BAD_AUTH_ALG   -1032
#define BCMSAD_ERR_FLOW_FULL      -1044    /* No more Flow Entries */
#define BCMSAD_ERR_TUNNEL_OPT_LEN -1045
#define BCMSAD_ERR_ENET_HDR_LEN   -1046
#define BCMSAD_ERR_BAD_FLAGS      -1047
#define BCMSAD_ERR_BAD_STATE      -1048
#define BCMSAD_ERR_SA_ACTIVE      -1049
#define BCMSAD_ERR_SA_HW_LOAD     -1050    /* failure to load SA to the chip */
#define BCMSAD_ERR_SA_HW_UNLOAD   -1051    /* failure to unload SA to the chip */
#define BCMSAD_ERR_UNDEFINED      -1100

/* SADB flags values for arguments to API functions */

/* SADB flags values for arguments to API functions */
#define BCMSAD_APIFLAG_NONE             0x0000
#define BCMSAD_APIFLAG_ALL_SA           0x0001
#define BCMSAD_APIFLAG_ACTIVE           0x0002
#define BCMSAD_APIFLAG_EXPIRED          0x0004
#define BCMSAD_APIFLAG_SA_ACTIVATE      0x0008
#define BCMSAD_APIFLAG_TTL_NO_DEC       0x0010
#define BCMSAD_APIFLAG_DF_COPY          0x0020
#define BCMSAD_APIFLAG_DF_CLEAR         0x0040
#define BCMSAD_APIFLAG_DF_SET           0x0080
#define BCMSAD_APIFLAG_INBOUND          0x0100
#define BCMSAD_APIFLAG_OUTBOUND         0x0200
#define BCMSAD_APIFLAG_IPV4ADDR         0x0400
#define BCMSAD_APIFLAG_IPV6ADDR         0x0800
#define BCMSAD_APIFLAG_ENET_PREPEND     0x1000

/*************************************************************************
**      To be modified as needed to suit the host environment          **
*************************************************************************/
#define BCMSAD_MAX_SA_COUNT          0x20000ul    /* Maximum size of each SADB */
#define BCMSAD_TUNNEL_TTL_DEFAULT    64    /* Default tunnel TTL value */
#define BCMSAD_TUNNEL_TOS_DEFAULT     0    /* Default tunnel TOS value */
#define BCMSAD_TUNNEL_OPTIONS_MAX    64    /* Maximum size IP options in tunnel */

#endif /* _BCMSAD_H_ */
