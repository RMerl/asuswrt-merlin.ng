/*
* <:copyright-BRCM:2007:proprietary:standard
*
*    Copyright (c) 2007 Broadcom
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

#ifndef _GPONCTL_API_H_
#define _GPONCTL_API_H_

#include "bcm_ploam_api.h"
#include "bcm_omci_api.h"
#include "bcm_gpon_api_common.h"
#include "bcmctl_syslogdefs.h"

#define GPONCTL_LOG_NAME "gponctl"

#if defined(CC_GPONCTL_DEBUG)
#define GPONCTL_DEBUGCODE(code)    code
#else
#define GPONCTL_DEBUGCODE(code)
#endif /* CC_GPONCTL_DEBUG */

#define GPONCTL_LOG_ERROR(fmt, arg...) \
{ \
    printf("[ERROR " "%s" "] %-10s, %d: " fmt "\n", \
           GPONCTL_LOG_NAME, __FUNCTION__, __LINE__, ##arg); \
    BCMCTL_SYSLOGCODE(gponCtl, LOG_ERR, fmt, arg); \
}

#define GPONCTL_LOG_INFO(fmt, arg...) \
{ \
    printf("[INFO " "%s" "] %-10s, %d: " fmt "\n", \
           GPONCTL_LOG_NAME, __FUNCTION__, __LINE__, ##arg); \
    BCMCTL_SYSLOGCODE(gponCtl, LOG_INFO, fmt, arg); \
}

#define GPONCTL_LOG_DEBUG(fmt, arg...) \
{ \
    GPONCTL_DEBUGCODE(printf("[DEBUG " "%s" "] %-10s, %d: " fmt "\n", \
           GPONCTL_LOG_NAME, __FUNCTION__, __LINE__, ##arg); )\
    BCMCTL_SYSLOGCODE(gponCtl, LOG_DEBUG, fmt, arg); \
}

/* Return status values. */
typedef enum GPON_CTL_Status
{
    GPON_CTL_STATUS_SUCCESS = 100,
    GPON_CTL_STATUS_INIT_FAILED,
    GPON_CTL_STATUS_ERROR,
    GPON_CTL_STATUS_LOAD_ERROR,
    GPON_CTL_STATUS_STATE_ERROR,
    GPON_CTL_STATUS_PARAMETER_ERROR,
    GPON_CTL_STATUS_ALLOC_ERROR,
    GPON_CTL_STATUS_RESOURCE_ERROR,
    GPON_CTL_STATUS_IN_USE,
    GPON_CTL_STATUS_NOT_FOUND,
    GPON_CTL_STATUS_NOT_SUPPORTED,
    GPON_CTL_STATUS_NOT_REGISTERED,
    GPON_CTL_STATUS_TIMEOUT
} BCM_GPON_CTL_STATUS;

/*
 * Public functions
 */
char *gponCtl_getMsgError(int err);

/*****************************************
 ** gponCtl Event Handling Commands API **
 *****************************************/
int gponCtl_getEventStatus(BCM_Ploam_EventStatusInfo *info);
int gponCtl_maskEvent(BCM_Ploam_EventMaskInfo *info);

/*****************************************
 ** gponCtl Alarm Handling Commands API **
 *****************************************/
int gponCtl_getAlarmStatus(BCM_Ploam_AlarmStatusInfo *info);
int gponCtl_maskAlarm(BCM_Ploam_MaskAlarmInfo *mask);
int gponCtl_setSFSDThreshold(BCM_Ploam_SFSDthresholdInfo *threshold);
int gponCtl_getSFSDThreshold(BCM_Ploam_SFSDthresholdInfo *threshold);

/****************************************
 ** gponCtl State Control Commands API **
 ****************************************/
int gponCtl_startAdminState(BCM_Ploam_StartInfo *info);
int gponCtl_stopAdminState(BCM_Ploam_StopInfo *info);
int gponCtl_getControlStates(BCM_Ploam_StateInfo *info);
int gponCtl_setTO1TO2(BCM_Ploam_TO1TO2Info *info);
int gponCtl_getTO1TO2(BCM_Ploam_TO1TO2Info *info);
int gponCtl_setDWELL_TIMER(BCM_Ploam_DWELL_TIMERInfo *info);
int gponCtl_getDWELL_TIMER(BCM_Ploam_DWELL_TIMERInfo *info);
int gponCtl_getRebootFlags(BCM_Ploam_RebootFlagsInfo *info);
int gponCtl_setTO6(BCM_Ploam_TO6Info *info);
int gponCtl_getTO6(BCM_Ploam_TO6Info *info);

/***********************************
 ** gponCtl Counters Commands API **
 ***********************************/
int gponCtl_getMessageCounters(BCM_Ploam_MessageCounters *counters);
int gponCtl_getGtcCounters(BCM_Ploam_GtcCounters *counters);
int gponCtl_getFecCounters(BCM_Ploam_fecCounters *counters);
int gponCtl_getGemPortCounters(BCM_Ploam_GemPortCounters *counters);
int gponCtl_getStats(BCM_Ploam_StatCounters *stats);

/************************************************
 ** gponCtl GEM Port Provisioning Commands API **
 ************************************************/
int gponCtl_configGemPort(BCM_Ploam_CfgGemPortInfo *info);
int gponCtl_configDsGemPortEncryptionByIX(BCM_Ploam_GemPortEncryption *conf);
int gponCtl_configDsGemPortEncryptionByID(BCM_Ploam_GemPortEncryption *conf);
int gponCtl_deconfigGemPort(BCM_Ploam_DecfgGemPortInfo *info);
int gponCtl_enableGemPort(BCM_Ploam_EnableGemPortInfo *info);
int gponCtl_getGemPort(BCM_Ploam_GemPortInfo *info);
int gponCtl_getAllocIds(BCM_Ploam_AllocIDs *allocIds);
int gponCtl_getOmciPort(BCM_Ploam_OmciPortInfo *info);
int gponCtl_getTcontCfg(BCM_Ploam_TcontInfo *info);
int gponCtl_configTcontAllocId(BCM_Ploam_TcontAllocIdInfo *info);
int gponCtl_deconfigTcontAllocId(BCM_Ploam_TcontAllocIdInfo *info);
int gponCtl_setGemBlockLength(BCM_Ploam_GemBlkLenInfo *info);
int gponCtl_getGemBlockLength(BCM_Ploam_GemBlkLenInfo *info);
int gponCtl_setTodInfo(BCM_Ploam_TimeOfDayInfo *info);
int gponCtl_getTodInfo(BCM_Ploam_TimeOfDayInfo *info);
int gponCtl_getSRIndication(BCM_Ploam_SRIndInfo *info);
int gponCtl_getPowerManagementParams(BCM_Ploam_PowerManagementParams *info);
int gponCtl_setPowerManagementParams(BCM_Ploam_PowerManagementParams *info);

/*****************************************
 ** gponCtl GTC Parameters Commands API **
 *****************************************/
int gponCtl_getOnuId(BCM_Ploam_GetOnuIdInfo *info);
int gponCtl_getFecMode(BCM_Ploam_GetFecModeInfo *info);
int gponCtl_getEncryptionKey(BCM_Ploam_GetEncryptionKeyInfo *info);
int gponCtl_setMcastEncryptionKeys(BCM_Ploam_McastEncryptionKeysInfo *info);

/***************************************************
 ** gponCtl Serial Number & Password Commands API **
 ***************************************************/
int gponCtl_setSerialPasswd(BCM_Ploam_SerialPasswdInfo *info);
int gponCtl_getSerialPasswd(BCM_Ploam_SerialPasswdInfo *info);
int gponCtl_getPloamDriverVersion(BCM_Gpon_DriverVersionInfo *info);

/****************************************
 ** gponCtl Test Functions API **
 ****************************************/
int gponCtl_generatePrbsSequence(BCM_Ploam_GenPrbsInfo *info);

/*******************************
 ** gponCtl OMCI Commands API **
 *******************************/
int gponCtl_getOmciCounters(BCM_Omci_Counters *counters);
int gponCtl_getOmciDriverVersion(BCM_Gpon_DriverVersionInfo *info);
int gponCtl_getEncryptStateUpdate(BCM_PloamGemEncryptUpd *info);
int gponCtl_getKeyEncryptionKey(BCM_Ploam_Kek *info);
int gponCtl_setOmciCtrlMasterSessionKey(BCM_PLoam_OmciCtrlMsk *info);
int gponCtl_cfgOmciMirror(BCM_Omci_PortMirror *info);

/*******************************
 ** gponCtl Logging API **
 *******************************/
#if defined(BCMCTL_SYSLOG_SUPPORTED)
DECLARE_setSyslogLevel(gponCtl);
DECLARE_getSyslogLevel(gponCtl);
DECALRE_isSyslogLevelEnabled(gponCtl);
DECLARE_setSyslogMode(gponCtl);
DECLARE_isSyslogEnabled(gponCtl);
#endif /* BCMCTL_SYSLOG_SUPPORTED */


#endif /* _GPONCTL_API_H_ */

