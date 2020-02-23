/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#ifndef _DHD_DEFS_H_
#define _DHD_DEFS_H_

/* The following Macro is from the DHD Driver file dhd_msgbuf.c
   Where:  BCMPCIE_H2D_COMMON_MSGRINGS  is 2
     and:  BCMPCIE_COMMON_MSGRINGS      is 5
     
    #define DHD_RINGID_TO_FLOWID(ringid) \
        (BCMPCIE_H2D_COMMON_MSGRINGS + ((ringid) - BCMPCIE_COMMON_MSGRINGS))
        
    Adjustment = -3
*/
#define DHD_RINGID_TO_DHD_FLOWID_ADJUSTMENT         3

#define DHD_DOORBELL_IRQ_NUM                        2
#define DHD_RXPOST_IRQ_NUM                          5
#define DHD_MSG_TYPE_TX_POST                        0xF
#define DHD_MSG_TYPE_RX_POST                        0x11
#define DHD_TX_POST_FLOW_RING_DESCRIPTOR_SIZE       48

#if defined(RDP_SIM) && defined(XRDP)

#define INTERRUPT_ID_XRDP_QUEUE_0                    0
#define DONGLE_WAKEUP_REGISTER_0                    (INTERRUPT_ID_XRDP_QUEUE_0 + 28)
#define DONGLE_WAKEUP_REGISTER_1                    (INTERRUPT_ID_XRDP_QUEUE_0 + 29)
#define DONGLE_WAKEUP_REGISTER_2                    (INTERRUPT_ID_XRDP_QUEUE_0 + 30)

#define DHD_RX_POST_FLOW_RING_SIZE                  8
#define DHD_TX_COMPLETE_FLOW_RING_SIZE              8
#define DHD_RX_COMPLETE_FLOW_RING_SIZE              8

#else

#define DHD_RX_POST_FLOW_RING_SIZE                  1024
#define DHD_TX_COMPLETE_FLOW_RING_SIZE              1024
#define DHD_RX_COMPLETE_FLOW_RING_SIZE              1024

#endif




#define DHD_DATA_LEN                                2048
#define DHD_RX_POST_RING_NUMBER                     1  
#define DHD_TX_COMPLETE_RING_NUMBER                 3
#define DHD_RX_COMPLETE_RING_NUMBER                 4
#if !defined(BCM_DSL_RDP)
#if defined(XRDP)
#define DHD_DATA_OFFSET                             88
#else
#define DHD_DATA_OFFSET                             64
#endif
#endif
#define DHD_RX_POST_INT_COUNT                       32

/* The host DHD driver sets the Tx Post descriptor request_id field with the 32 bit
 * aligned SKB/FKB address.  Therefore, the least significant two bits are always 0.
 * These bits are used to indicate packet type.  Value 0 is SKB and value 3 is FKB.
 * The address and packet type are stored in little endian byte ordering.  This
 * causes the packet type to be in bits [25:24] instead of [1:0] on the big endian
 * Runner processor.  DHD offload firmware will use the packet type field to indicate
 * RDD host buffer (1) and Runner BPM buffer (2).
 */
#define DHD_TX_POST_BUFFER_TYPE_WIDTH               2
#define DHD_TX_POST_BUFFER_TYPE_OFFSET              24
#define DHD_TX_POST_HOST_BUFFER_BIT_OFFSET          DHD_TX_POST_BUFFER_TYPE_OFFSET
#define DHD_TX_POST_SKB_BUFFER_VALUE                0   /* 00: possible value in tx complete only */
#define DHD_TX_POST_HOST_BUFFER_VALUE               1   /* 01: possible value in tx post and tx complete */
#define DHD_TX_POST_BPM_BUFFER_VALUE                2   /* 10: possible value in tx post and tx complete */
#define DHD_TX_POST_FPM_BUFFER_VALUE                2   /* 10: possible value in tx post and tx complete */
#define DHD_TX_POST_FKB_BUFFER_VALUE                3   /* 11: possible value in tx complete only */
#define DHD_COMPLETE_OWNERSHIP_RUNNER               2   /* 02: (00/01/11 are reserved for SKB/FKB/HOST cases) queued on DHD Complete CPU ring */

#define DHD_TX_POST_FPM_TOKEN_SIZE_OFFSET           26
#define DHD_TX_POST_FPM_TOKEN_SIZE_WIDTH            2
#define DHD_TX_POST_FPM_TOKEN_SIZE_1_VALUE          0
#define DHD_TX_POST_FPM_TOKEN_SIZE_2_VALUE          1
#define DHD_TX_POST_FPM_TOKEN_SIZE_4_VALUE          2
#define DHD_TX_POST_FPM_TOKEN_SIZE_8_VALUE          3

#define DHD_TX_BPM_REF_COUNTER_TAIL_OFFSET          16
#define DHD_TX_POST_EXCLUSIVE_OFFSET                27

#define DHD_RX_POST_VALID_REQ_ID_OFFSET             16
#define DHD_RX_POST_VALID_REQ_ID_MASK               ((1<<DHD_RX_POST_VALID_REQ_ID_OFFSET) - 1)

#define DHD_MSG_TYPE_FLOW_RING_FLUSH                0
#define DHD_MSG_TYPE_FLOW_RING_SET_DISABLED         1

#define DHD_TX_POST_FLOW_RING_CACHE_SIZE            16
#define DHD_FLOW_RING_CACHE_LKP_DEPTH               CAM_SEARCH_DEPTH_16

/* Flow Ring Cache Entry Definitions */
#define DHD_FLOW_RING_DISABLED_BIT                  1 /* (1 << 1) */
#if defined(BCM_DSL_RDP)
#define DHD_FLOW_RING_CACHE_FLAGS_AC_F_OFFSET       4
#define DHD_FLOW_RING_CACHE_FLAGS_AC_F_WIDTH        3
#endif

#define DHD_RADIO_OFFSET_COMMON_A(index)            (DHD_RADIO_INSTANCE_COMMON_A_DATA_ADDRESS + (index * sizeof(RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DTS)))
#define DHD_RADIO_OFFSET_COMMON_B(index)            (DHD_RADIO_INSTANCE_COMMON_B_DATA_ADDRESS + (index * sizeof(RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS)))

/* LLCSNAP definitions */
#define DHD_LLCSNAP_HEADER_SIZE                     8
#define DHD_ETH_LENGTH_TYPE_OFFSET                  12
#define DHD_ETH_FCS_SIZE                            4
#define DHD_ETH_L2_HEADER_SIZE                      14
#define DHD_LLCSNAP_CONTROL_OFFSET                  16
#define DHD_LLCSNAP_PROTOCOL_OFFSET                 20
#define DHD_LLCSNAP_END_OFFSET                      22
#define DHD_ETH_TYPE_MAX_DATA_LEN                   0x05dc
#define DHD_ETH_TYPE_APPLE_ARP                      0x80f3
#define DHD_ETH_TYPE_NOVELL_IPX                     0x8137
#define DHD_LLCSNAP_DSAP_SSAP_VALUE                 0xaaaa
#define DHD_LLCSNAP_CONTROL_VALUE                   0x0300
#define DHD_LLCSNAP_OUI_BRIDGE_TUNNEL_VALUE         0xf8

/* TX Packet aggregation definitions */
#define DHD_TX_POST_PKT_AGGR_TCP_LEN_MIN            40

/* FPM management definitions */
#define DHD_FPM_SIZE_OFFSET                         16
#define DHD_FPM_SIZE_WIDTH                          4

/* Backup queue management definitions bit 22 supposed to fit both RDP and XRDP platforms*/
#define DHD_WAKEUP_REQUIRED_OFFSET                  22
#define DHD_WAKEUP_REQUIRED_WIDTH                   1


/* iDMA defines */
#define DMA_TYPE_SHIFT    4
#define INDEX_VAL_SHIFT   16

#endif

