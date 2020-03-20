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
 *  Decription: Header file common to all API
 *  File: bcmipsec.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *
 *****************************************************************************/

#ifndef BCMIPSEC_H_
#define BCMIPSEC_H_

#include <linux/types.h>
#include <linux/slab.h>    /* for kmalloc/kfree */

typedef unsigned long long EXTLONG;    /* for large statistics */
typedef unsigned long IPADDR_V4;    /* IP v4 network byte order */

#define IPADDR                  IPADDR_V4
#define SIZE_SPI                4
#define SIZE_SEQUENCE_NUM       4
#define SIZE_ESP_HEADER         (SIZE_SEQUENCE_NUM+SIZE_SPI)
#define SIZE_IPSEC_ICV          12
#define SIZE_IPV4_HEADER        20
#define SIZE_IPV6_HEADER        40
#define SIZE_IPV4_ADDR          4
#define SIZE_IPV6_ADDR          16
#define MAX_IPHEADER_SIZE       SIZE_IPV6_HEADER


#ifndef TRUE
#define TRUE            1
#endif
#ifndef FALSE
#define FALSE           0
#endif

#define _BCM5862_

#include "srl5862.h"

#define BCM_MAX_IPSEC_CTX_SIZE   160

/* API Build Directives */
#define USE_SYNC_MODE
/* The following conditions are optional */
#undef  BCMAPI_USE_CACHED_SA
#undef  BCMAPI_USE_UDP_ENCAP
//#define BCMAPI_USE_INBOUND_SPD_EN
#undef BCMAPI_USE_INBOUND_SPD_EN
#define BCMAPI_USE_MAC_CKSUM
/* Fragment uses */
#ifdef  BCMAPI_USE_INBOUND_SPD_EN
#undef  BCMAPI_USE_SPS_FRAGMENT
#else
#define BCMAPI_USE_SPS_FRAGMENT
#endif
#undef  BCMAPI_USE_HASH_FRAGMENT


/* Handles of API objects */
typedef unsigned long BCMDEVH;    /*  Handle of a device */
typedef unsigned long BCMSADH;    /*  Handle of an SA entry */
typedef unsigned long BCMSPDH;    /*  Handle of an SPD entry */
typedef unsigned long BCMFLAGS;   /*  flags */

typedef int BCMDEV_RET;           /*  Device API functions return type */
typedef int BCMPACK_RET;          /*  Packet API functions return type */
typedef int BCMSAD_RET;           /*  SAD API functions return type */
typedef int BCMSPD_RET;           /*  SPD API functions return type */

typedef int BCMSTATUS;            /*  API status type */

#define BCM_STATUS_OK                        0
#define BCM_STATUS_SUCCESS                   0
#define BCM_STATUS_DEVICE_FAIL              -1
#define BCM_STATUS_SRL_DISABLED             -2
#define BCM_STATUS_DEVICE_DISABLED          -3
#define BCM_STATUS_INVALID_INPUT            -4
#define BCM_STATUS_OUTPUT_ERROR             -5
#define BCM_STATUS_ERROR_ALLOC              -6
#define    BCM_STATUS_DATA_ALIGNMENT        -7
#define BCM_STATUS_INVALID_SIGNATURE        -8
#define BCM_STATUS_POLICY_VERIFICATION      -9
#define BCM_STATUS_INTERFACE_INIT           -10
#define BCM_STATUS_DMA_ALLOC                -11
#define BCM_STATUS_RESOURCE                 -12


/* Bits of the BCMPKT_CTX flags */
typedef struct OperationFlags_s {
    uint32_t state:4;    /* state BCMSAD_STATE_XX */
    uint32_t proto:4;    /* protocol suite BCMSAD_PROTOCOL_XX */
    uint32_t dir:2;      /* direction BCMSAD_DIR_XX */
    uint32_t action:4;   /* action BCMSAD_ACT_XX */
    uint32_t mode:4;     /* mode BCMSAD_MODE_XX */
    uint32_t neg:4;      /* key negotiation */
    uint32_t rekey:4;    /* rekeying method */
    uint32_t auth:1;     /* authentication required */
    uint32_t encr:1;     /* encryption required */
    uint32_t ipv4:1;     /* operation is for IPv4 packets */
    uint32_t ipv6:1;     /* operation is for IPv6 packets */
    uint32_t reserved:2; /* reserved bits */
} CRYPTOOPFLAGS;

/* Crypto Operation Data Structure */
typedef struct {
    uint8_t encrAlg;        /* Encryption algorithm BCMSAD_ALG_ENCR_XX */
    uint8_t authAlg;        /* Authentication Algorithm BCMSAD_ALG_AUTH_XX */
    uint16_t encrKeyLen;    /* Encryption algorithm key length */
    uint16_t authKeyLen;    /* Authentication algorithm key length */
    uint16_t hashLen;       /* Authentication hash length */
    uint32_t spi;           /* SPI for the outermost encapsulation */
    uint32_t innerSpi;      /* ESP SPI in case of an AH/ESP bundle */
    uint8_t *encrKey;       /* Encryption key */
    uint8_t *authKey;       /* Authentication key */
    void *hostsa;           /* pointer to a proprietary host context */
    void *callback;         /* Completion callback function */
    CRYPTOOPFLAGS flags;    /* flags indicating operation/SA specifics */
} BCM_CRYPTOOP;

/* Bits of the generic BCMPKT_CTX flags */
typedef struct PacketContextFlags_s {
    uint32_t dir:2;            /* Direction as in BCMSAD_DIR_XX */
    uint32_t order:2;          /* order of authen/encr BCMSAD_ORDER_XX */
    uint32_t authProcess:2;    /* authent processing BCMSAD_AUTH_OP_XX */
    uint32_t nextproto:8;      /* next protocol, for IPSec tunnels */
    uint32_t contextWords:3;   /* number of context words */
    uint32_t reserved:15;      /* reserved bits */
} BCMPKT_CTXFLAGS;

/* Generic Packet Context Data Structure */
typedef struct {
    uint16_t encrOffset;    /* Offset for start of data to be (en/de)crypted */
    uint16_t encrLen;       /* Length of data be encrypted/decrypted */
    uint16_t authOffset;    /* Offset for start of data to be hashed */
    uint16_t authLen;       /* Length of data to be hashed */
    uint16_t padLen;        /* Length of added padding and next proto */
    uint8_t *odata;         /* pointer to output data for in-place crypto */
    uint8_t *authDigest;    /* Location of the authentication digest */
    uint8_t *iv;            /* Crypto initialization vector */
    void *hostctx;          /* host context */
    BCMPKT_CTXFLAGS flags;  /* flags indicating operation/SA specifics */
    void *srcpkt;
    void *dstpkt;
    void *next;             /* for a linked list of pre-formatted contexts */
    void *prev;
} BCMPKTCTX;

/* Context used only by outbound SA */
typedef struct outsactx_t {
    uint32_t *seqnumPtr;  /* pointer to the packet sequence number */
    uint16_t tunnelID;    /* tunnel ID */
    uint16_t unused;
    uint8_t tunnelHeader[MAX_IPHEADER_SIZE];/* pre-formatted tunnel header */
} OUTCTX;

/* Context used only by inbound SA */
typedef struct insactx_t {
    uint32_t pol_src[4];    /* policy verification source IP */
    uint32_t srcMask[4];    /* policy verification source IP mask */
    uint32_t pol_dst[4];    /* policy verification dest IP */
    uint32_t dstMask[4];    /* policy verification dest IP mask */
    uint32_t pol_proto;     /* policy verification protocol */
} INCTX;

/* SA Context */
typedef struct sactx_t {
    void *hostSA;               /* host proprietary SA */
    uint16_t ctxSize;           /* size of packet context */
    uint16_t msgHdrSize;    /* Size of message header incl SACTX */
    uint16_t ivLen;             /* block size for padding */
    union {
        INCTX in;
        OUTCTX out;
    };
    uint16_t outputFrags:4;    /* number of extra fragments for output */
    uint16_t authOFrag:1;      /* indicates an output fragment for auth hash */
    uint16_t statusOFrag:1;    /* indicates an output fragment for status */
    uint16_t updateOFrag:1;    /* indicates an output fragment for SCTX updates*/
    uint16_t spsOFrag:1;       /* indicates an output fragment for SPS */
    uint16_t inputFrags:4;     /* number of extra fragments for input */
    uint16_t bctIFrag:1;       /* indicates an input fragement for BCT */
    uint16_t inbound:1;        /* SA is inbound */
    uint16_t unusedflags:2;    /* unused */
    uint16_t bctOffset;        /* Offset of the BCT input payload */
    uint16_t bctLen;           /* Length of the BCT input payload */
    uint16_t updateFragLen;    /* Length of the SUPDT output Fragment */
    uint16_t spsFragLen;       /* Length of the SPS output Fragment */
    uint16_t icvOFrag:1;       /* indicates an Output fragement for ICV of AH */
    uint16_t unused:15;    
    CRYPTOOPFLAGS flags;         /* operation/SA specifics */
    uint32_t SAUPhysicalAddress; /* Fixme Gigi */
    uint32_t SAUPtr;
    void    *dmaStatus;
} SACTX;

/* BDESC */
typedef struct bdesc_t {
    uint16_t offsetMAC;
    uint16_t lengthMAC;
    uint16_t offsetCrypto;
    uint16_t lengthCrypto;
    uint16_t offsetICV;
    uint16_t offsetIV;
} BDESC;

/*
 * IP Headers
 */
typedef struct {
    uint8_t     ip_ver;        /* Version & Header length */
    uint8_t     ip_tos;        /* TOS */
    uint16_t ip_len;           /* IP packet length */
    uint16_t ip_id;            /* ID */
    uint16_t ip_off;           /* Flags & Fragment offset */
    uint8_t  ip_ttl;           /* TTL */
    uint8_t  ip_proto;         /* Protocol */
    uint16_t ip_cksum;         /* Checksum */
    uint32_t ip_src;           /* IP Src Address */
    uint32_t ip_dst;           /* IP Dest Address */
} IPV4_HDR;

typedef struct {
    uint32_t ip_ver:4;        /* Version & Header length */
    uint32_t ip_class:8;      /* Traffic Class */
    uint32_t ip_flabel:20;    /* Flow Label */
    uint16_t ip_len;          /* IP packet length */
    uint8_t  ip_next;         /* Next Header */
    uint8_t  ip_hop;          /* Hop Limit */
    uint32_t ip_src[4];       /* IP Src Address */
    uint32_t ip_dst[4];       /* IP Dest Address */
} IPV6_HDR;


/* Logging Control */
#define CONFIG_BCM_PRINT

/* Logging level */
#define BCM_LOG_FATAL        1
#define BCM_LOG_CRITICAL     2
#define BCM_LOG_ERROR        3
#define BCM_LOG_DEBUG        4
#define BCM_LOG_INFO         5

extern int bcm_error;        /* print flag for error messages */
extern int bcm_debug;        /* print flag for debug messages */
extern int bcm_info;         /* print flag for info messages */

#ifdef CONFIG_BCM_PRINT
#define BCM_PRINT(flag, format, args...) \
        ((flag) ? printk(KERN_INFO format , ## args) : 0)
#else
#define BCM_PRINT(flag, format, args...)
#endif                /* CONFIG_BCM_PRINT */

/* Simple mutex to protect a resource */
#define MUTEX_FREE        0
#define MUTEX_TAKEN       1

#define MUTEX_DECLARE(s)    volatile spinlock_t s
#define MUTEX_INIT(s)       spin_lock_init(s)
#define MUTEX_SET(s)        spin_lock(s)
#define MUTEX_UNSET(s)      spin_unlock(s)

#endif /* BCMIPSEC_H_ */
