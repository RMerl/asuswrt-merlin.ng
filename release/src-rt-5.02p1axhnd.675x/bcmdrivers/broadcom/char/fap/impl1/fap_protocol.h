#ifndef __FAP_PROTOCOL_H_INCLUDED__
#define __FAP_PROTOCOL_H_INCLUDED__

/*
<:copyright-BRCM:2011:proprietary:standard 

   Copyright (c) 2011 Broadcom 
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
 *
 * File Name    : fapProtocol.h
 * Description  : This file provides NATed flow configurations in the FAP.
 *
 *  Dynamically Learnt Flows:
 *  =========================
 *  The FAP Protocol layer is responsible for configuring the learnt NATed flows,
 *  with the assistance of the Broadcom Packet Flow Cache subsystem.
 *  The interface between the Flow Cache and the FAP Protocol layer is
 *  defined by 3 hooks and one functional interface as follows:
 *
 *  Flow Cache hooks to which FAP binds:
 *      fc_activate_cmf_hook_g   <-- fapProtoActivate()
 *      fc_deactivate_cmf_hook_g <-- fapProtoDeactivate()
 *      fc_refresh_cmf_hook_g    <-- fapProtoRefresh()
 *
 *******************************************************************************
 */

/*
 *-----------------------------------------------------------------------------
 * Constructor and Destructor for the FAP Protocol layer.
 *-----------------------------------------------------------------------------
 */

/* FAP_PROTO_LOCK() is used to protect the shared global variables of the
   Protocol layer (fapState_g) and Packet layer (flowAlloc_g), as well as
   accesses to the shared memory between the Host MIPS and the FAP MIPS
   (pktTablesHost) */
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define FAP_PROTO_LOCK() spin_lock_bh ( &fapProto_lock_g )
#define FAP_PROTO_UNLOCK() spin_unlock_bh ( &fapProto_lock_g )
#else
#define FAP_PROTO_LOCK()       do {} while(0)
#define FAP_PROTO_UNLOCK()     do {} while(0)
#endif

void fapStatus(void);
void fapPrint(int16 sourceChannel, int16 destChannel);
void fapEnable(void);
void fapDisable(void);
void fapReset(void);
fapRet fapDebug(int logLevel);
extern void fapProtoConstruct(void);
extern void fapProtoDestruct(void);

void decrement_Fapfailures(void);
void update_Fapdeactivates(void);

#endif  /* defined(__FAP_PROTOCOL_H_INCLUDED__) */

