/******************************************************************************
 *
<:copyright-BRCM:2018:proprietary:standard

   Copyright (c) 2018 Broadcom
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
#ifndef _GPONCTL_API_TRACE_H_
#define _GPONCTL_API_TRACE_H_

void gponCtl_getEventStatusTrace(BCM_Ploam_EventStatusInfo *info);
void gponCtl_maskEventTrace(BCM_Ploam_EventMaskInfo *info);

void gponCtl_getAlarmStatusTrace(BCM_Ploam_AlarmStatusInfo *info);
void gponCtl_maskAlarmTrace(BCM_Ploam_MaskAlarmInfo *mask);
void gponCtl_setSFSDThresholdTrace(BCM_Ploam_SFSDthresholdInfo *threshold);
void gponCtl_getSFSDThresholdTrace(BCM_Ploam_SFSDthresholdInfo *threshold);
void gponCtl_startAdminStateTrace(BCM_Ploam_StartInfo *info);
void gponCtl_stopAdminStateTrace(BCM_Ploam_StopInfo *info);
void gponCtl_getControlStatesTrace(BCM_Ploam_StateInfo *info);
void gponCtl_setTO1TO2Trace(BCM_Ploam_TO1TO2Info *info);
void gponCtl_getTO1TO2Trace(BCM_Ploam_TO1TO2Info *info);
void gponCtl_setDWELL_TIMERTrace(BCM_Ploam_DWELL_TIMERInfo *info);
void gponCtl_getDWELL_TIMERTrace(BCM_Ploam_DWELL_TIMERInfo *info);
void gponCtl_getRebootFlagsTrace(BCM_Ploam_RebootFlagsInfo *info);
void gponCtl_setTO6Trace(BCM_Ploam_TO6Info *info);
void gponCtl_getTO6Trace(BCM_Ploam_TO6Info *info);

void gponCtl_getMessageCountersTrace(BCM_Ploam_MessageCounters *counters);
void gponCtl_getGtcCountersTrace(BCM_Ploam_GtcCounters *counters);
void gponCtl_getFecCountersTrace(BCM_Ploam_fecCounters *counters);
void gponCtl_getGemPortCountersTrace(BCM_Ploam_GemPortCounters *counters);
void gponCtl_getStatsTrace(BCM_Ploam_StatCounters *stats);

void gponCtl_configGemPortTrace(BCM_Ploam_CfgGemPortInfo *info);
void gponCtl_configDsGemPortEncryptionByIXTrace(BCM_Ploam_GemPortEncryption *conf);
void gponCtl_configDsGemPortEncryptionByIDTrace(BCM_Ploam_GemPortEncryption *conf);
void gponCtl_deconfigGemPortTrace(BCM_Ploam_DecfgGemPortInfo *info);
void gponCtl_enableGemPortTrace(BCM_Ploam_EnableGemPortInfo *info);
void gponCtl_getGemPortTrace(BCM_Ploam_GemPortInfo *info);
void gponCtl_getAllocIdsTrace(BCM_Ploam_AllocIDs *allocIds);
void gponCtl_getOmciPortTrace(BCM_Ploam_OmciPortInfo *info);
void gponCtl_getTcontCfgTrace(BCM_Ploam_TcontInfo *info);
void gponCtl_configTcontAllocIdTrace(BCM_Ploam_TcontAllocIdInfo *info);
void gponCtl_deconfigTcontAllocIdTrace(BCM_Ploam_TcontAllocIdInfo *info);
void gponCtl_setGemBlockLengthTrace(BCM_Ploam_GemBlkLenInfo *info);
void gponCtl_getGemBlockLengthTrace(BCM_Ploam_GemBlkLenInfo *info);
void gponCtl_setTodInfoTrace(BCM_Ploam_TimeOfDayInfo *info);
void gponCtl_getTodInfoTrace(BCM_Ploam_TimeOfDayInfo *info);
void gponCtl_getSRIndicationTrace(BCM_Ploam_SRIndInfo *info);
void gponCtl_getPowerManagementParamsTrace(BCM_Ploam_PowerManagementParams *info);
void gponCtl_setPowerManagementParamsTrace(BCM_Ploam_PowerManagementParams *info);

void gponCtl_getOnuIdTrace(BCM_Ploam_GetOnuIdInfo *info);
void gponCtl_getFecModeTrace(BCM_Ploam_GetFecModeInfo *info);
void gponCtl_getEncryptionKeyTrace(BCM_Ploam_GetEncryptionKeyInfo *info);
void gponCtl_setMcastEncryptionKeysTrace(BCM_Ploam_McastEncryptionKeysInfo *info);

void gponCtl_setSerialPasswdTrace(BCM_Ploam_SerialPasswdInfo *info);
void gponCtl_getSerialPasswdTrace(BCM_Ploam_SerialPasswdInfo *info);
void gponCtl_getPloamDriverVersionTrace(BCM_Gpon_DriverVersionInfo *info);

void gponCtl_generatePrbsSequenceTrace(BCM_Ploam_GenPrbsInfo *info);

void gponCtl_getOmciCountersTrace(BCM_Omci_Counters *counters);
void gponCtl_getOmciDriverVersionTrace(BCM_Gpon_DriverVersionInfo *info);
void gponCtl_getEncryptStateUpdateTrace(BCM_PloamGemEncryptUpd *info);
void gponCtl_getKeyEncryptionKeyTrace(BCM_Ploam_Kek *info);
void gponCtl_setOmciCtrlMasterSessionKeyTrace(BCM_PLoam_OmciCtrlMsk *info);


#endif /* _GPONCTL_API_TRACE_H_ */
