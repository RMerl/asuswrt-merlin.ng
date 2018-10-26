/******************************************************************************
 *
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
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
 *
******************************************************************************/
#ifndef _VLANCTL_API_TRACE_H_
#define _VLANCTL_API_TRACE_H_

int vlanCtl_initTrace(void);
int vlanCtl_cleanupTrace(void);
int vlanCtl_createVlanInterfaceTrace(const char *realDevName, unsigned int vlanDevId,
                                int isRouted, int isMulticast);
int vlanCtl_createVlanInterfaceExtTrace(const char *realDevName, unsigned int vlanDevId,
                                vlanCtl_createParams_t *createParamsP);
int vlanCtl_createVlanInterfaceByNameTrace(char *realDevName, char *vlanDevName,
                                      int isRouted, int isMulticast);
int vlanCtl_createVlanInterfaceByNameExtTrace(char *realDevName, char *vlanDevName,
                                      vlanCtl_createParams_t *createParamsP);
int vlanCtl_deleteVlanInterfaceTrace(char *vlanDevName);
int vlanCtl_initTagRuleTrace(void);
int vlanCtl_insertTagRuleTrace(const char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags,
                          vlanCtl_ruleInsertPosition_t position, unsigned int posTagRuleId);
int vlanCtl_removeTagRuleTrace(char *realDevName, vlanCtl_direction_t tableDir,
                          unsigned int nbrOfTags, unsigned int tagRuleId);
int vlanCtl_removeAllTagRuleTrace(char *vlanDevName);
int vlanCtl_removeTagRuleByFilterTrace(char *realDevName, vlanCtl_direction_t tableDir,
                                  unsigned int nbrOfTags);
int vlanCtl_dumpRuleTableTrace(char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags);
int vlanCtl_dumpAllRulesTrace(void);
int vlanCtl_getNbrOfRulesInTableTrace(char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags);
int vlanCtl_setDefaultVlanTagTrace(char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags,
                              unsigned int defaultTpid, unsigned int defaultPbits, unsigned int defaultCfi,
                              unsigned int defaultVid);
int vlanCtl_setDscpToPbitsTrace(char *realDevName, unsigned int dscp, unsigned int pbits);
int vlanCtl_dumpDscpToPbitsTrace(char *realDevName, unsigned int dscp);
int vlanCtl_setTpidTableTrace(char *realDevName, unsigned int *tpidTable);
int vlanCtl_dumpTpidTableTrace(char *realDevName);
int vlanCtl_dumpLocalStatsTrace(char *realDevName);
int vlanCtl_setIfSuffixTrace(char *ifSuffix);
int vlanCtl_setDefaultActionTrace(const char *realDevName, vlanCtl_direction_t tableDir, unsigned int nbrOfTags,
                             vlanCtl_defaultAction_t defaultAction, char *defaultRxVlanDevName);
int vlanCtl_setRealDevModeTrace(char *realDevName, bcmVlan_realDevMode_t mode);
int vlanCtl_createVlanFlowsTrace(char *rxVlanDevName, char *txVlanDevName);
int vlanCtl_deleteVlanFlowsTrace(char *rxVlanDevName, char *txVlanDevName);
int vlanCtl_filterOnSkbPriorityTrace(unsigned int priority);
int vlanCtl_filterOnSkbMarkFlowIdTrace(unsigned int flowId);
int vlanCtl_filterOnSkbMarkPortTrace(unsigned int port);
int vlanCtl_filterOnEthertypeTrace(unsigned int etherType);
int vlanCtl_filterOnIpProtoTrace(unsigned int ipProto);
int vlanCtl_filterOnDscpTrace(unsigned int dscp);
int vlanCtl_filterOnDscp2PbitsTrace(unsigned int dscp2pbits);
int vlanCtl_filterOnVlanDeviceMacAddrTrace(unsigned int acceptMulticast);
int vlanCtl_filterOnFlagsTrace(unsigned int flags);
int vlanCtl_filterOnTagPbitsTrace(unsigned int pbits, unsigned int tagIndex);
int vlanCtl_filterOnTagCfiTrace(unsigned int cfi, unsigned int tagIndex);
int vlanCtl_filterOnTagVidTrace(unsigned int vid, unsigned int tagIndex);
int vlanCtl_filterOnTagEtherTypeTrace(unsigned int etherType, unsigned int tagIndex);
int vlanCtl_filterOnRxRealDeviceTrace(char *realDevName);
int vlanCtl_filterOnTxVlanDeviceTrace(char *vlanDevName);
int vlanCtl_cmdPopVlanTagTrace(void);
int vlanCtl_cmdPushVlanTagTrace(void);
int vlanCtl_cmdSetEtherTypeTrace(unsigned int etherType);
int vlanCtl_cmdSetTagPbitsTrace(unsigned int pbits, unsigned int tagIndex);
int vlanCtl_cmdSetTagCfiTrace(unsigned int cfi, unsigned int tagIndex);
int vlanCtl_cmdSetTagVidTrace(unsigned int vid, unsigned int tagIndex);
int vlanCtl_cmdSetTagEtherTypeTrace(unsigned int etherType, unsigned int tagIndex);
int vlanCtl_cmdCopyTagPbitsTrace(unsigned int sourceTagIndex, unsigned int targetTagIndex);
int vlanCtl_cmdCopyTagCfiTrace(unsigned int sourceTagIndex, unsigned int targetTagIndex);
int vlanCtl_cmdCopyTagVidTrace(unsigned int sourceTagIndex, unsigned int targetTagIndex);
int vlanCtl_cmdCopyTagEtherTypeTrace(unsigned int sourceTagIndex, unsigned int targetTagIndex);
int vlanCtl_cmdDscpToPbitsTrace(unsigned int tagIndex);
int vlanCtl_cmdSetDscpTrace(unsigned int dscp);
int vlanCtl_cmdDropFrameTrace(void);
int vlanCtl_cmdSetSkbPriorityTrace(unsigned int priority);
int vlanCtl_cmdSetSkbMarkPortTrace(unsigned int port);
int vlanCtl_cmdSetSkbMarkQueueTrace(unsigned int queue);
int vlanCtl_cmdSetSkbMarkQueueByPbitsTrace(void);
int vlanCtl_cmdSetSkbMarkFlowIdTrace(unsigned int flowId);
int vlanCtl_cmdOvrdLearningVidTrace(unsigned int vid);
int vlanCtl_cmdContinueTrace(void);
int vlanCtl_setReceiveVlanDeviceTrace(char *vlanDevName);
int vlanCtl_setVlanRuleTableTypeTrace(unsigned int type);
int vlanCtl_setDropPrecedenceTrace(bcmVlan_flowDir_t dir, bcmVlan_dpCode_t dpCode);


#endif /* _VLANCTL_API_TRACE_H_ */
