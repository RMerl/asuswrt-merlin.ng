
#ifndef __FAP4KE_IQ_H_INCLUDED__
#define __FAP4KE_IQ_H_INCLUDED__

/*
<:copyright-BRCM:2009:proprietary:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

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
*/


/*
 *******************************************************************************
 * File Name  : fap4ke_iq.h
 *
 *******************************************************************************
 */

/*----- Defines -----*/
#define CC_FAP_IQ_STATS

//#define CC_FAP4KE_IQ_DEBUG

#if defined(CC_FAP4KE_IQ_DEBUG)
#define FAP4KE_IQ_ASSERT(condition) fap4kePrt_Assert(condition)
#define FAP4KE_IQ_DEBUG(fmt, arg...) fap4kePrt_Debug(fmt, ##arg)
#define FAP4KE_IQ_INFO(fmt, arg...) fap4kePrt_Info(fmt, ##arg)
#define FAP4KE_IQ_NOTICE(fmt, arg...) fap4kePrt_Notice(fmt, ##arg)
#define FAP4KE_IQ_PRINT(fmt, arg...) fap4kePrt_Print(fmt, ##arg)
#define FAP4KE_IQ_ERROR(fmt, arg...) fap4kePrt_Error(fmt, ##arg)
#define FAP4KE_IQ_DUMP_PACKET(_packet_p, _length) dumpHeader(_packet_p)
#else
#define FAP4KE_IQ_ASSERT(condition)
#define FAP4KE_IQ_DEBUG(fmt, arg...)
#define FAP4KE_IQ_INFO(fmt, arg...)
#define FAP4KE_IQ_NOTICE(fmt, arg...)
#define FAP4KE_IQ_PRINT(fmt, arg...)
#define FAP4KE_IQ_ERROR(fmt, arg...)
#define FAP4KE_IQ_DUMP_PACKET(_packet_p, _length)
#endif

typedef enum {
    FAP_IQ_PRIO_LOW,
    FAP_IQ_PRIO_HIGH,
    FAP_IQ_PRIO_MAX
} fap4ke_iq_prio_t;

typedef enum {
    FAP_IQ_IPPROTO_TCP = 6,
    FAP_IQ_IPPROTO_UDP = 17,
    FAP_IQ_IPPROTO_ESP = 50,
    FAP_IQ_IPPROTO_MAX
} fap4ke_iq_ipproto_t;

typedef enum {
    FAP_IQ_ENT_DYN,
    FAP_IQ_ENT_STAT,
    FAP_IQ_ENT_MAX
} fap4ke_iq_ent_t;

typedef enum {
    FAP_IQ_CONG_STATUS_LO,
    FAP_IQ_CONG_STATUS_HI,
    FAP_IQ_CONG_STATUS_MAX
} fap_iq_cong_status_t;

typedef enum {
    FAP_IQ_IF_ENET,
    FAP_IQ_IF_ENET_RXCHNL0 = FAP_IQ_IF_ENET,
    FAP_IQ_IF_ENET_RXCHNL1,
    FAP_IQ_IF_ENET_RXCHNL2,
    FAP_IQ_IF_ENET_RXCHNL3,
    FAP_IQ_IF_XTM,
    FAP_IQ_IF_XTM_RXCHNL0 = FAP_IQ_IF_XTM,
    FAP_IQ_IF_XTM_RXCHNL1,
    FAP_IQ_IF_XTM_RXCHNL2,
    FAP_IQ_IF_XTM_RXCHNL3,
    FAP_IQ_IF_MAX,
} fap_iq_if_t;

void enetIqSetThresh( int channel, uint16 loThresh, uint16 hiThresh );
void enetIqSetDqmThresh( int channel, uint16 loThresh, uint16 hiThresh );

void xtmIqSetThresh( int channel, uint16 loThresh, uint16 hiThresh );
void xtmIqSetDqmThresh( int channel, uint16 loThresh, uint16 hiThresh );

void fap4keIq_dumpStatus(void);
void fap4keIq_init( void );
void fap4keIq_setStatus( int status );

#define FAP_IQ_ERROR               (-1)
#define FAP_IQ_SUCCESS             0

#define FAP_IQ_INVALID_NEXT_IX      0
#define FAP_IQ_INVALID_PORT         0

uint8_t fap4ke_iq_add_L4port_hook_t( fap4ke_iq_ipproto_t ipProto, 
        uint16_t destPort, fap4ke_iq_ent_t ent, fap4ke_iq_prio_t prio );

uint8_t fap4ke_iq_rem_L4port_hook_t( fap4ke_iq_ipproto_t ipProto, 
        uint16_t destPort, fap4ke_iq_ent_t ent );

int fap4ke_iq_prio_L4port_hook_t( fap4ke_iq_ipproto_t ipProto, 
        uint16_t destPort );

uint8_t fap4ke_iq_add_L4port( fap4ke_iq_ipproto_t ipProto, uint16_t destPort, 
        fap4ke_iq_ent_t ent, fap4ke_iq_prio_t prio );

uint8_t fap4ke_iq_rem_L4port( fap4ke_iq_ipproto_t ipProto, uint16_t destPort, 
        fap4ke_iq_ent_t ent );

int fap4ke_iq_prio_L4port( fap4ke_iq_ipproto_t ipProto, uint16_t destPort );

int fap4ke_iq_add_proto_prio( fap4ke_iq_prototype_t protoType, 
    uint8_t protoval, fap4ke_iq_prio_t prio );

int fap4ke_iq_rem_proto_prio( fap4ke_iq_prototype_t protoType, 
    uint8_t protoval );

void fap4ke_iq_dump_porttbl( fap4ke_iq_ipproto_t ipProto );

#endif /*  __FAP4KE_IQ_H_INCLUDED__ */

