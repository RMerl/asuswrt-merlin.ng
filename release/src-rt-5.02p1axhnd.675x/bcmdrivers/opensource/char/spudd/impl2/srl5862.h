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
 *  File: srl5862.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *
 *****************************************************************************/
#ifndef _SRL5862_H_
#define _SRL5862_H_

#include "cnctx.h"

typedef struct sctx_outsau_flags_t {
    uint32_t expired:1;
    uint32_t soft_expired:1;
    uint32_t reserved1:2;
    uint32_t seqnum_en:1;
    uint32_t timer_en:1;
    uint32_t bc_en:1;
    uint32_t reserved2:9;
    uint32_t next_header:8;
    uint32_t UPDT_size:8;
} SCTX_outsau_flags;

typedef struct sctx_outsau_t {
    SCTX_outsau_flags flags;
    uint32_t SALifeTime;
    uint32_t ByteCount[2];
    uint32_t SeqNum[2];
} SCTX_OUTSAU;

typedef struct sctx_insau_flags_t {
    uint32_t expired:1;
    uint32_t soft_expired:1;
    uint32_t reserved1:1;
    uint32_t audit_en:1;
    uint32_t seqnum_en:1;
    uint32_t timer_en:1;
    uint32_t bc_en:1;
    uint32_t spd_en:1;
    uint32_t reserved2:16;
    uint32_t UPDT_size:8;
} SCTX_insau_flags;

#define WINDOW_MASK4     4
#define WINDOW_MASK8     8
#define WINDOW_MASK16    16
#define WINDOW_MASK32    32

typedef struct sctx_insau_t {
    SCTX_insau_flags flags;
    uint32_t SALifeTime;
    uint32_t ByteCount[2];
    uint32_t SeqNum[2];
    uint32_t SlidingWindow[2];
    uint16_t AuthErrorCount;
    uint16_t ReplayCount;
    uint32_t PacketCount;
} SCTX_INSAU;

typedef union sctx_sau_t {
    SCTX_OUTSAU out;
    SCTX_INSAU in;
} SAU;
        
typedef struct BD_header_t {
    uint16_t    size;
    uint16_t    id;
} BD_HEADER;
        
typedef struct SPSV4_field_t {
    uint32_t    SeqNum[2];
    uint32_t    ByteCount;
    uint16_t    DstPort;
    uint16_t    SrcPort;
    uint32_t    DstAddress;
    uint32_t    SrcAddress;
    uint32_t    SPS_Size:8;        
    uint32_t    Protocol:8;        
    uint32_t    reserved:13;        
    uint32_t    SeqNum_Extracted:1;
    uint32_t    reserved_ipv6:1;    
    uint32_t    Extract_Err:1;
} SPSV4_FIELD;
        
typedef struct SPSV6_field_t {
    uint32_t    SeqNum[2];
    uint32_t    ByteCount;
    uint16_t    DstPort;
    uint16_t    SrcPort;
    uint32_t    DstAddress[4];
    uint32_t    SrcAddress[4];
    uint32_t    SPS_Size:8;        
    uint32_t    Protocol:8;        
    uint32_t    reserved:13;        
    uint32_t    SeqNum_Extracted:1;
    uint32_t    reserved_ipv6:1;    
    uint32_t    Extract_Err:1;
} SPSV6_FIELD;


#define SPD_ADDR_WILD    0
#define SPD_ADDR_FIXED   1
#define SPD_ADDR_RANGE   2
#define SPD_ADDR_MASK    3

typedef struct spd_flags_t {
    uint32_t ipv6:1;
    uint32_t dadr_en:2;
    uint32_t sadr_en:2;
    uint32_t dport_en:1;
    uint32_t sport_en:1;
    uint32_t prot_en:1;
    uint32_t reserved1:8;
    uint32_t protocol:8;
    uint32_t reserved2:8;
} SPD_FLAGS;

typedef struct spd_ports_t {
    uint16_t srcPort;
    uint16_t dstPort;
} SPD_PORTS;

typedef struct spd_v4_t {
    SPD_FLAGS flags;
    uint32_t SrcAddr;
    uint32_t SrcMask;
    uint32_t DstAddr;
    uint32_t DstMask;
    SPD_PORTS ports;
} SPD_V4;

typedef struct spd_v6_t {
    SPD_FLAGS flags;
    uint32_t SrcAddr[4];
    uint32_t SrcMask[4];
    uint32_t DstAddr[4];
    uint32_t DstMask[4];
    SPD_PORTS ports;
} SPD_V6;

typedef struct sps_flags_t {
    uint32_t ipv6:1;
    uint32_t dadr_en:2;
    uint32_t sadr_en:2;
    uint32_t dport_en:1;
    uint32_t sport_en:1;
    uint32_t prot_en:1;
    uint32_t reserved1:8;
    uint32_t protocol:8;
    uint32_t reserved2:8;
} SPS_FLAGS;

typedef struct sps_v4_t {
    SPS_FLAGS flags;
    uint32_t SrcAddr;
    uint32_t DstAddr;
    SPD_PORTS ports;
    uint32_t ByteCount;
    uint32_t SeqNum[2];
} SPS_V4;

typedef struct sps_v6_t {
    SPS_FLAGS flags;
    uint32_t SrcAddr[4];
    uint32_t DstAddr[4];
    SPD_PORTS ports;
    uint32_t ByteCount;
    uint32_t SeqNum[2];
} SPS_V6;

/* 5862 MCR1 IPSEC error codes */
#define ERROR_OB_IPSEC_PROT     0x00
#define ERROR_IB_IPSEC_PROT     0x01
#define ERROR_OB_SSL_PROT       0x02
#define ERROR_IB_SSL_PROT       0x03
#define ERROR_SCTX_FORMAT       0x04
#define ERROR_SCTX_MGMT         0x05
#define ERROR_ANTI_REPLAY       0x06
#define ERROR_INBOUND_POLICY    0x07
#define ERROR_AUTHENT_FAIL      0x10
#define ERROR_CRYTPO_SIZE       0x11
#define ERROR_PADCHECK_FAIL     0x12
#define ERROR_MAX_REAL_ERROR    0x12
#define ERROR_WARNING           0x13

/* Outbound IPSec Protocol Errors */
#define IPSECERR_OB_MAX_IPV6_HEADER         0x20
#define IPSECERR_OB_INNER_IP_VERSION        0x21
#define IPSECERR_OB_OUTER_IP_VERSION        0x22
#define IPSECERR_OB_IP_VERSION              0x23
#define IPSECERR_OB_INNER_IPV6_HEADER       0x24
#define IPSECERR_OB_OUTER_IPV6_HEADER       0x26
#define IPSECERR_OB_IPV4_TUNNEL_HEAD_LEN    0x27
#define IPSECERR_OB_IPV6_TUNNEL_HEAD_LEN    0x28
#define IPSECERR_OB_LAST_PROC_DESC_ERR      0x29
#define IPSECERR_OB_BAD_PROC_DESC_TYPE      0x2B
#define IPSECERR_OB_BAD_ESN_ALGIN           0x2D
#define IPSECERR_OB_BAD_MFM                 0x2F
#define IPSECERR_OB_LAST_PROC_DESC_ERR2     0x66
#define IPSECERR_OB_BAD_PROC_DESC_TYPE2     0x68
#define IPSECERR_OB_BAD_ESN_ALGIN2          0x6A

/* Inbound IPSec Protocol Errors */
#define IPSECERR_IB_MAX_IPV6_HEADER         0x55
#define IPSECERR_IB_INNER_IP_VERSION        0x56
#define IPSECERR_IB_OUTER_IP_VERSION        0x57
#define IPSECERR_IB_IP_VERSION              0x58
#define IPSECERR_IB_OUTER_IPV6_HEADER1      0x59
#define IPSECERR_IB_OUTER_IPV6_HEADER2      0x5A
#define IPSECERR_IB_OUTER_IPV6_HEADER3      0x5B
#define IPSECERR_IB_OUTER_IPV6_HEADER4      0x5C
#define IPSECERR_IB_OUTER_IPV6_HEADER5      0x5D
#define IPSECERR_IB_INNER_IPV6_HEADER       0x60
#define IPSECERR_IB_LAST_PROC_DESC_ERR      0x2A
#define IPSECERR_IB_BAD_PROC_DESC_TYPE      0x2C
#define IPSECERR_IB_BAD_ESN_ALGIN           0x2E
#define IPSECERR_IB_LAST_PROC_DESC_ERR2     0x67
#define IPSECERR_IB_BAD_PROC_DESC_TYPE2     0x69
#define IPSECERR_IB_BAD_ESN_ALGIN2          0x6B

/* SCTX Format Errors */
/* TBD */

/* SCTX Management Errors */
#define SCTXMGNERR_SA_OUT_HARD_TIME_EXPIRE    0x00
#define SCTXMGNERR_SA_OUT_HARD_BYTE_EXPIRE    0x01
#define SCTXMGNERR_SA_OUT_HARD_SEQ_EXPIRE     0x02
#define SCTXMGNERR_SA_IN_HARD_TIME_EXPIRE     0x03
#define SCTXMGNERR_SA_IN_HARD_BYTE_EXPIRE     0x04
#define SCTXMGNERR_SA_IN_HARD_SEQ_EXPIRE      0x05

/* Status warnings */
#define WARNING_SA_OUT_SOFT_TIME_EXPIRE    0x00
#define WARNING_SA_OUT_SOFT_BYTE_EXPIRE    0x01
#define WARNING_SA_OUT_SOFT_SEQ_EXPIRE     0x02
#define WARNING_SA_IN_SOFT_TIME_EXPIRE     0x03
#define WARNING_SA_IN_SOFT_BYTE_EXPIRE     0x04
#define WARNING_SA_IN_SOFT_SEQ_EXPIRE      0x05
#define WARNING_SA_IN_PACKET_LOSS          0x08

#define BCM_MAX_IPSEC_CTX_SIZE             160

#define AUX_SPI_SIZE                       4
#define NEXT_DEVICE                        -1

typedef struct esp_header_t {
    uint32_t spi;
    uint32_t SeqNum;
    uint32_t iv[4];
} ESP_HEADER;

typedef struct ah_header_t {
    uint8_t  NextHeader;
    uint8_t  PayloadLen;
    uint16_t reserved;
    uint32_t spi;
    uint32_t SeqNum;
    uint8_t  AuthData[SIZE_IPSEC_ICV];
} AH_HEADER;

#endif /* _SRL5862_H_ */
