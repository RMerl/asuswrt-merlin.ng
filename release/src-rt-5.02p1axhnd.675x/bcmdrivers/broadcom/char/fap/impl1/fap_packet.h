#ifndef __FAP_PACKET_H_INCLUDED__
#define __FAP_PACKET_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
:>
*/

/*
 *******************************************************************************
 * File Name  : fap_packet.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

#include "fap4ke_timers.h"
#include "fap4ke_packet.h"

#define CC_FAP_MANAGE_HW_ARL

/* Special FLOWHANDLE signify an invalid flow */
#define FAP4KE_PKT_INVALID_FLOWHANDLE   ( (uint16)(~0) )

#define FAP_INVALID_SWITCH_PORT 0xFF

/* fapPkt_flowHandle_t MUST be kept in sync with CMF_TUPLE16_MCAST_MASK in fap.h */
typedef union {
    struct {
        uint16 reserved : 6,
               fapIdx   : 1,
               flowId   : 9;
    };
    uint16 u16;
} fapPkt_flowHandle_t;

typedef union {
    struct {
        uint8 isAlloc       : 1,
              isHostValid   : 1,
              reserved      : 6;
    };
    uint8 u8;
} fapPkt_flowAlloc_t;

fapRet __init fapPkt_construct(void);
fapPkt_flowHandle_t fapPkt_mcastActivate(uint32 fapIdx,
                                    fap4kePkt_flowInfo_t *flowInfo, 
                                    uint8 *checksum1, 
                                    uint8 *checksum2,
                                    uint32 mcCfglogIdx);

fapRet fapPkt_mcastDeactivate(fapPkt_flowHandle_t flowHandle,uint32 mcCfglogIdx);
fapRet fapPkt_mcastUpdate(fapPkt_flowHandle_t flowHandle, uint32 msgType,
                          fap4kePkt_flowInfo_t *flowInfo, uint32 mcCfglogIdx);
void fapPkt_mcastSetMissBehavior(int dropMcastMiss);

fapPkt_flowHandle_t fapPkt_activate(uint32 fapIdx,
                                    fap4kePkt_flowInfo_t *flowInfo, 
                                    uint8 *cmdList,
                                    uint16 cmdListSize,
                                    uint8 *checksum1, 
                                    uint8 *checksum2,
                                    fap4kePkt_learnAction_t *learnAction_p);
fapRet fapPkt_deactivate(fapPkt_flowHandle_t flowHandle);
fapRet fapPkt_getFlowInfo(fapPkt_flowHandle_t flowHandle,
                          fap4kePkt_flowInfo_t *flowInfo);
fapRet fapPkt_setFlowInfo(fapPkt_flowHandle_t flowHandle,
                          fap4kePkt_flowInfo_t *flowInfo);
int fapPkt_deactivateAll(void);
fapRet fapPkt_printFlow(fapPkt_flowHandle_t flowHandle);
fap4kePkt_flowStats_t *fapPkt_getFlowStats(fapPkt_flowHandle_t flowHandle);
fapRet fapPkt_resetStats(fapPkt_flowHandle_t flowHandle);
void fapPkt_printAllFlows(int16 sourceChannel, int16 destChannel);
#if defined(CONFIG_BCM_FAP_LAYER2)
void fapPkt_setFloodingMask(uint8 channel, uint8 mask, int drop);
void fapPkt_arlPrint(void);
void fapPkt_arlFlush(uint8 channelMask);
void fapL2flow_defaultVlanTagConfig(int enable);
#if defined(CC_FAP_MANAGE_HW_ARL)
uint16 fapPkt_hwArlRead(fap4keArl_tableEntryKey_t *arlKey_p,
                        fap4keArl_tableEntryKey_t *rsltArlKey_p);
int fapPkt_hwArlWrite(fap4keArl_tableEntryKey_t *arlKey_p, uint32 port, int overwrite);
int fapPkt_hwArlDelete(fap4keArl_tableEntryKey_t *arlKey_p);
void fapPkt_hwArlDump(void);
void fapPkt_hwArlConfig(int enable, uint8 learnPortMask);
#endif
#endif
fapRet fapPkt_dynMemFreeIsr(uint32 fapIdx, DQMQueueDataReg_S msg);
fapRet fapPkt_dynMemFreeBh(uint32 fapIdx, DQMQueueDataReg_S msg);

void fapPktTest_runTests(void);

#endif /* __FAP_PACKET_H_INCLUDED__ */
