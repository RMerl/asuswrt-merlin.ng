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
 *  Description: Header file for IPSec SPU Cryptographic functions.
 *  File: spucrypt.h
 *  Author: Pavan Kumar
 *  Date: 11/16/2007
 *  Packet Processing header file for IPSec SPU device
 *
 *****************************************************************************/

#ifndef _SPUCRYPT_H_
#define _SPUCRYPT_H_

#define MAX_CRYPT_CONTEXT       22  /* 8(AES 32bytes) + 4(AES 16bytes) + 
                                     * 5(Inner State) +5(Outer State)   */

/*
 * CryptoFlag Settings
 */
#define CF_ENCODE       0x0000
#define CF_DECODE       0x0040
#define CF_CRYPTO       0x0080
#define CF_3DES         0x0080      /* SSL */
#define CF_MD5          0x0010
#define CF_SHA1         0x0020
#define CF_AES_CBC      0x0000
#define CF_AES_CTR      0x0004
#define CF_AES_128      0x0000
#define CF_AES_192      0x0001
#define CF_AES_256      0x0002

/*
 * Crypto operation types.
 */
#define OPERATION_IPSEC                         0x0000
#define OPERATION_IPSEC_GENERIC                 0x0001
#define OPERATION_SSL_HMAC                      0x0100
#define OPERATION_SSL_MAC                       0x0100
#define OPERATION_TLS_HMAC                      0x0200
#define OPERATION_SSL_CRYPTO                    0x0300
#define OPERATION_ARC4                          0x0400
#define OPERATION_HASH                          0x0500
#define OPERATION_IPSEC_AES                     0x4000
#define OPERATION_IPSEC_OUTBOUND_TRANSPORT      0x9000
#define OPERATION_IPSEC_OUTBOUND_TUNNEL         0x9100
#define OPERATION_IPSEC_INBOUND_TRANSPORT       0x9200
#define OPERATION_IPSEC_INBOUND_TUNNEL          0x9300

typedef struct CipherContext_s {
    volatile unsigned short CryptoOffset;
    volatile unsigned short CryptoFlag;
    volatile unsigned int Context[MAX_CRYPT_CONTEXT];
} CipherContext_t, *CipherContext_pt;

typedef struct SSL_HMACContext_s {
    volatile unsigned short Reserved;
    volatile unsigned short CryptoFlag;
    volatile unsigned char HMACKey[20];
    volatile unsigned char HMACPad[48];
    volatile unsigned int SequenceHigh;
    volatile unsigned int SequenceLow;
    volatile unsigned int ContentType:8;
    volatile unsigned int DataLength:16;
    volatile unsigned int ReservedB:8;
} SSL_HMACContext_t, *SSL_HMACContext_pt, SSL_MACContext_t, *SSL_MACContext_pt;

/* TLS HMAC Context */
typedef struct TLS_HMACContext_s {
    volatile unsigned short Reserved;
    volatile unsigned short CryptoFlag;
    volatile unsigned int HMACInnerState[5];
    volatile unsigned int HMACOuterState[5];
    volatile unsigned int SequenceHigh;
    volatile unsigned int SequenceLow;
    volatile unsigned int ContentType:8;
    volatile unsigned int Version:16;
    volatile unsigned int DataLengthHi:8;
    volatile unsigned int DataLengthLo:8;
    volatile unsigned int ReservedG2:24;
} TLS_HMACContext_t, *TLS_HMACContext_pt;

/* Pure Hash Context */
typedef struct Hash_Context_s {
    volatile unsigned short Reserved;
    volatile unsigned short CryptoFlag;
} Hash_Context_t, *Hash_Context_pt;

/* SSL/TLS DES Context */
typedef struct SSL_CryptoContext_s {
    volatile unsigned short Reserved;
    volatile unsigned short CryptoFlag;
    volatile unsigned int CryptoKey1[2];
    volatile unsigned int CryptoKey2[2];
    volatile unsigned int CryptoKey3[2];
    volatile unsigned int ComputedIV[2];
} SSL_CryptoContext_t, *SSL_CryptoContext_pt;

typedef union CryptoContext_u {
    CipherContext_t Cipher;
    SSL_HMACContext_t SSL_Mac;
    TLS_HMACContext_t TLS_HMac;
    SSL_CryptoContext_t SSL_Crypto;
    Hash_Context_t Hash;
} CryptoContext_t, *CryptoContext_pt;

typedef struct PacketContext_s {
    volatile unsigned short operation_type;
    volatile unsigned short cmd_structure_length;
    volatile CryptoContext_t Context;
    volatile unsigned int PhysicalAddress;
} PacketContext_t, *PacketContext_pt;

typedef struct PacketDataBufChain_new_s {
    volatile unsigned int DataAddress;        /* Physical address. */
    volatile unsigned int pNext;      /* Physical address. */
    volatile unsigned short Reserved:8;
    volatile unsigned short C_flag:1;
    volatile unsigned short nFrags:7;
    volatile unsigned short DataLength;
} PacketDataBufChain_new_t, *PacketDataBufChain_new_pt;

/*
 * Packet
 */
typedef struct Packet_new_s {
    volatile unsigned int PacketContextBuffer;
    volatile PacketDataBufChain_new_t InputHead;
    volatile unsigned short PacketLength;
    volatile unsigned short E_flag:1;       /* evit flag */
    volatile unsigned short rsvd:3;
    volatile unsigned short cctx_len:12;    /* context_len */
    volatile PacketDataBufChain_new_t OutputHead;
} Packet_new_t, *Packet_new_pt;

typedef void DMAHandle_t;

/* This structure would contain the packet and its associated callback */
typedef struct _packet_cmd_info_s{
    DMAHandle_t     *packet_handle; /* A DMA mapped memory handle to Packet_t */
} packet_cmd_info_t;

/* a Logical Processing struct with the required buffers */
#define MAX_PROC_BUF_SAVE_PTRS  4

typedef struct _proc_buf_s {
    void  * pDevice;        /* A back refrence to the device this 
                               proc_buf is allocated for */
    unsigned int command;   /* Store the command for local use
                               command is opaque but is stored when 
                               called getProcesssingBuffer 
                               can be used as a flag for 
                               get/free/ProcessingBuffer also */
                            /* This is because the polldevice 
                               callbacks dones not pass the pdevice back */
    DMAHandle_t     *inputBuffer_handle;    /* a opaque handle for a 
                                               DMA mapped memory */
    DMAHandle_t     *outputBuffer_handle;   /* a opaque handle for a 
                                               DMA mapped memory */
    DMAHandle_t     *inputFragments_handle; /* A DMA mapped memory 
                                               handle to DataBufChain_t */
    DMAHandle_t     *outputFragments_handle;/* A DMA mapped memory handle 
                                               to DataBufChain_t */
    DMAHandle_t     *status_handle;         /* A DMA mapped memory for 
                                               status storage for PRD 
                                               like functions */
    packet_cmd_info_t packetInfo;           /* the packet and its callback */
    void * save_ptrs[MAX_PROC_BUF_SAVE_PTRS]; /* usage is dependent on 
                                                 packet processing path */
} proc_buf_t;

#endif /* _SPUCRYPT_H_ */
