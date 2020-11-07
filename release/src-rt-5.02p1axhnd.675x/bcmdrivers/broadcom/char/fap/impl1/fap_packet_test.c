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

//**************************************************************************
// File Name  : fap4ke_packet_test.c
//
// Description: This file contains the unit test code for the FAP 4ke Packet
//              layer.
//               
//**************************************************************************

#include "fap_hw.h"
#include "bcmtypes.h"
#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include "fap_task.h"
#include "fap_packet.h"
#include "fap_dqm.h"
#include "fap_dqmHost.h"
#include "fap4ke_iopDma.h"

#if defined(CC_FAP4KE_PKT_TEST)

static void flowTest(void)
{
    fapRet ret;
    int flowIdIx;
    fap4kePkt_flowId_t flowId[FAP4KE_PKT_MAX_FLOWS];
    fap4kePkt_flowInfo_t flowInfo;
    uint32 fapIdx = 0;  // TBD_FAP: update this
//    int test = 0;

    iopDma_CmdListAddEndOfCommands(flowInfo.cmdList);

//    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "*********** Test %d ***********", test++);
    flowIdIx = 0;
    flowInfo.ipv4.tuple.ipSa = 0x01000001;
    flowInfo.ipv4.tuple.ipDa = 0x02000001;
    flowInfo.ipv4.tuple.sPort = 1000;
    flowInfo.ipv4.tuple.dPort = 2000;
    flowInfo.ipv4.tuple.flags.isRouted = 1;
    flowInfo.ipv4.source.phy = FAP4KE_PKT_PHY_ENET;
    flowInfo.ipv4.source.channel = 3;
    flowId[flowIdIx] = fapPkt_activate(fapIdx, &flowInfo);
    if(flowId[flowIdIx] == FAP4KE_PKT_INVALID_FLOWID)
    {
//        return;
    }

    flowIdIx = 1;
    flowInfo.ipv4.tuple.ipSa = 0x01000001;
    flowInfo.ipv4.tuple.ipDa = 0x02000001;
    flowInfo.ipv4.tuple.sPort = 1000;
    flowInfo.ipv4.tuple.dPort = 2000;
    flowInfo.ipv4.tuple.flags.isRouted = 1;
    flowInfo.source.phy = FAP4KE_PKT_PHY_XTM;
    flowInfo.source.channel = 14;
    flowId[flowIdIx] = fapPkt_activate(fapIdx, &flowInfo);
    if(flowId[flowIdIx] == FAP4KE_PKT_INVALID_FLOWID)
    {
//        return;
    }

    flowIdIx = 2;
    flowInfo.ipv4.tuple.ipSa = 0x01000101;
    flowInfo.ipv4.tuple.ipDa = 0x02000101;
    flowInfo.ipv4.tuple.sPort = 10000;
    flowInfo.ipv4.tuple.dPort = 20000;
    flowInfo.ipv4.tuple.flags.isRouted = 0;
    flowInfo.source.phy = FAP4KE_PKT_PHY_XTM;
    flowInfo.source.channel = 23;
    flowId[flowIdIx] = fapPkt_activate(fapIdx, &flowInfo);
    if(flowId[flowIdIx] == FAP4KE_PKT_INVALID_FLOWID)
    {
//        return;
    }

    fapPkt_printFlow(FAP4KE_PKT_MAX_FLOWS);

//    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "*********** Test %d ***********", test++);
    ret = fapPkt_deactivate(flowId[1]);
    if(ret == FAP_ERROR)
    {
//        return;
    }

    fapPkt_printFlow(FAP4KE_PKT_MAX_FLOWS);

//    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "*********** Test %d ***********", test++);
    ret = fapPkt_deactivate(flowId[0]);
    if(ret == FAP_ERROR)
    {
//        return;
    }

    fapPkt_printFlow(FAP4KE_PKT_MAX_FLOWS);

//    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "*********** Test %d ***********", test++);
    flowIdIx = 3;
    flowInfo.ipv4.tuple.ipSa = 0x01000001;
    flowInfo.ipv4.tuple.ipDa = 0x02000001;
    flowInfo.ipv4.tuple.sPort = 1000;
    flowInfo.ipv4.tuple.dPort = 2000;
    flowInfo.ipv4.tuple.flags.isRouted = 1;
    flowInfo.source.phy = FAP4KE_PKT_PHY_ENET;
    flowInfo.source.channel = 3;
    flowId[flowIdIx] = fapPkt_activate(fapIdx, &flowInfo);
    if(flowId[flowIdIx] == FAP4KE_PKT_INVALID_FLOWID)
    {
//        return;
    }

    fapPkt_printFlow(FAP4KE_PKT_MAX_FLOWS);

//    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "*********** Test %d ***********", test++);
    fapPkt_deactivate(15);

//    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "*********** Test %d ***********", test++);
    fapPkt_deactivate(flowId[2]);
    fapPkt_deactivate(flowId[3]);

    fapPkt_printFlow(FAP4KE_PKT_MAX_FLOWS);
}

static void classificationTest(void)
{
    int flowIdIx;
    fap4kePkt_flowId_t flowId[FAP4KE_PKT_MAX_FLOWS];
    fap4kePkt_flowInfo_t flowInfo;

    iopDma_CmdListAddEndOfCommands(flowInfo.cmdList);

    flowIdIx = 0;
    flowInfo.ipv4.tuple.ipSa = 0x01010201;
    flowInfo.ipv4.tuple.ipDa = 0x01010101;
    flowInfo.ipv4.tuple.sPort = 0x2222;
    flowInfo.ipv4.tuple.dPort = 0x3333;
    flowInfo.ipv4.tuple.flags.isRouted = 1;
    flowInfo.source.phy = flowInfo.dest.phy = FAP4KE_PKT_PHY_ENET;
    flowInfo.source.channel = flowInfo.dest.channel = 2;
    flowInfo.dest.queue = 0;
    flowInfo.txAdjust = 0;
    flowId[flowIdIx] = fapPkt_activate(&flowInfo);
    if(flowId[flowIdIx] == FAP4KE_PKT_INVALID_FLOWID)
    {
//        return;
    }

    flowIdIx = 0;
    flowInfo.ipv4.tuple.ipSa = 0x01010201;
    flowInfo.ipv4.tuple.ipDa = 0x01010101;
    flowInfo.ipv4.tuple.sPort = 0x2222;
    flowInfo.ipv4.tuple.dPort = 0x3333;
    flowInfo.ipv4.tuple.flags.isRouted = 1;
    flowInfo.source.phy = flowInfo.dest.phy = FAP4KE_PKT_PHY_ENET;
    flowInfo.source.channel = flowInfo.dest.channel = 3;
    flowInfo.dest.queue = 0;
    flowInfo.txAdjust = 0;
    flowId[flowIdIx] = fapPkt_activate(&flowInfo);
    if(flowId[flowIdIx] == FAP4KE_PKT_INVALID_FLOWID)
    {
//        return;
    }

    flowIdIx = 0;
    flowInfo.ipv4.tuple.ipSa = 0x01010201;
    flowInfo.ipv4.tuple.ipDa = 0x04040404;
    flowInfo.ipv4.tuple.sPort = 0x2222;
    flowInfo.ipv4.tuple.dPort = 0x3333;
    flowInfo.ipv4.tuple.flags.isRouted = 1;
    flowInfo.source.phy = flowInfo.dest.phy = FAP4KE_PKT_PHY_XTM;
    flowInfo.source.channel = flowInfo.dest.channel = 30;
    flowInfo.dest.queue = 0;
    flowInfo.txAdjust = 0;
    flowId[flowIdIx] = fapPkt_activate(&flowInfo);
    if(flowId[flowIdIx] == FAP4KE_PKT_INVALID_FLOWID)
    {
//        return;
    }

    flowIdIx = 0;
    flowInfo.ipv4.tuple.ipSa = 0x01010201;
    flowInfo.ipv4.tuple.ipDa = 0x04040404;
    flowInfo.ipv4.tuple.sPort = 0x2222;
    flowInfo.ipv4.tuple.dPort = 0x3333;
    flowInfo.ipv4.tuple.flags.isRouted = 1;
    flowInfo.source.phy = flowInfo.dest.phy = FAP4KE_PKT_PHY_XTM;
    flowInfo.source.channel = flowInfo.dest.channel = 20;
    flowInfo.dest.queue = 0;
    flowInfo.txAdjust = 0;
    flowId[flowIdIx] = fapPkt_activate(&flowInfo);
    if(flowId[flowIdIx] == FAP4KE_PKT_INVALID_FLOWID)
    {
//        return;
    }

    fapPkt_printFlow(FAP4KE_PKT_MAX_FLOWS);

    if(dqmXmitAvailableHost(DQM_HOST2FAP_CMD_Q))
    {
        DQMQueueDataReg_S t;

        t.word0 = DQM_FAP2HOST_CMD_Q_PKT_TEST;
        t.word1 = 0;
        t.word2 = 0;
        t.word3 = 0;

        dqmXmitMsgHost(DQM_HOST2FAP_CMD_Q, DQM_HOST2FAP_CMD_Q_SIZE, &t);
    }
    else
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "DQM_HOST2FAP_CMD_Q is full");
    }
}

void fapPktTest_runTests(void)
{
    flowTest();

    classificationTest();
}

#endif /* CC_FAP4KE_PKT_TEST */
