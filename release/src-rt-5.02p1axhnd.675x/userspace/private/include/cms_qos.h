/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

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
 ************************************************************************/

#ifndef __CMS_QOS_H
#define __CMS_QOS_H

/*!\file cms_qos.h
 * \brief Header file for the QoS portion of the CMS Data Abstration Layer API.
 *   This is in the cms_dal library.  Also includes some QoS definitions.
 */



/********************** Include Files ***************************************/
#include "cms.h"
#include "cms_core.h"  // not needed by this file, but breaks wlan if removed


/********************** Global Types ****************************************/


#define MAX_QOS_LEVELS           8

#define XTM_QOS_LEVELS           MAX_QOS_LEVELS

#define MAX_PON_TRANSMIT_QUEUES  8
#define PON_QOS_LEVELS           8

#if defined(SUPPORT_RDPA) || defined(SUPPORT_BCMTM) || defined(SUPPORT_ARCHERCTL)
#define MAX_ETH_TRANSMIT_QUEUES  8
#define ETH_QOS_LEVELS           8
#else /* Default for all other platforms */
#define MAX_ETH_TRANSMIT_QUEUES  4
#define ETH_QOS_LEVELS           4
#endif

#if (defined(SUPPORT_DSL) && defined(SUPPORT_FAPCTL)) || defined(SUPPORT_RDPA) || defined(SUPPORT_BCMTM) || defined(SUPPORT_ARCHERCTL)
#define MAX_ETHWAN_TRANSMIT_QUEUES  8
#define ETHWAN_QOS_LEVELS           8
#else
#define MAX_ETHWAN_TRANSMIT_QUEUES  4
#define ETHWAN_QOS_LEVELS           4
#endif

#define MAX_WLAN_TRANSMIT_QUEUES 8
#define WLAN_QOS_LEVELS          8

#ifdef BRCM_WLAN
#define WIRELESS_PREDEFINE_QUEUE MAX_WLAN_TRANSMIT_QUEUES
#else
#define WIRELESS_PREDEFINE_QUEUE 0
#endif

// max QoS Classification rules
#define QOS_CLS_MAX_ENTRY        32
#define QOS_CLS_INVALID_INDEX    0xffffffff


/* Policer uses TC DSMARK queuing discipline.
 * The <indices> value of a DSMARK qdisc must be a power of 2.
 */
#define QOS_DSMARK_QDISC_INDICES 128 
#define QOS_POLICER_MAX_ENTRY    (QOS_DSMARK_QDISC_INDICES / 4)    //max 32 QoS Policers

#define WIRELESS_QUEUE_INTF      "wireless"
#define DEFAULT_QUEUE_NAME       "Default Queue"
#define TR181_WIFI_INTF_PATH     "Device.WiFi.SSID."

#define QOS_QUEUE_NO_SHAPING        -1

#define QOS_CRITERION_UNUSED        -1
#define QOS_RESULT_NO_CHANGE        -1
#define QOS_RESULT_AUTO_MARK        -2


#define DHCP_VENDOR_CLASS_OPTION    60
#define DHCP_USER_CLASS_OPTION      77


#define DSCP_NO_CHANGE            -1
#define DSCP_AUTO                 -2
#define DSCP_DEFAULT              0x00
#define DSCP_AF13                 0x0E
#define DSCP_AF12                 0x0C
#define DSCP_AF11                 0x0A
#define DSCP_CS1                  0x08
#define DSCP_AF23                 0x16
#define DSCP_AF22                 0x14
#define DSCP_AF21                 0x12
#define DSCP_CS2                  0x10
#define DSCP_AF33                 0x1E
#define DSCP_AF32                 0x1C
#define DSCP_AF31                 0x1A
#define DSCP_CS3                  0x18
#define DSCP_AF43                 0x26
#define DSCP_AF42                 0x24
#define DSCP_AF41                 0x22
#define DSCP_CS4                  0x20
#define DSCP_EF                   0x2E
#define DSCP_CS5                  0x28
#define DSCP_CS6                  0x30
#define DSCP_CS7                  0x38



/* MUST be kept in-sync with those used in qosqueueadd.html */
#define CMS_QUEUE_SCHED_BLOCK      '0'
#define CMS_QUEUE_SCHED_SP         '1'
#define CMS_QUEUE_SCHED_RR         '2'
#define CMS_QUEUE_SCHED_WRR        '3'
#define CMS_QUEUE_SCHED_WFQ        '4'
#define CMS_QUEUE_SCHED_SP_WRR_WFQ '5'
#define CMS_QUEUE_SCHED_SP_WRR     '6'
#define CMS_QUEUE_SCHED_WRR_WFQ    '7'
#define CMS_QUEUE_SCHED_UNSPEC     'z'



#define MAX_SEND_SIZE 256

typedef struct optionmsgbuf
{
   SINT32   mtype;
   char     mtext[MAX_SEND_SIZE];
} QosDhcpOptionMsgBuf;



/** Generic structure for holding QoS Queue info.  This contains all the
 *  params that CMS supports from the TR98 and TR181 Queue object.
 */
typedef struct {
   UBOOL8 enable;
   char   queueName[BUFLEN_64];    /**< name of this queue */
   char   intfName[CMS_IFNAME_LENGTH];  /**< intf which this queue belongs to */
   UBOOL8 isWan;              /**< is this queue on a WAN intf */
   UINT32 queueId;
   char   schedulerAlgorithm[BUFLEN_8];
   UINT32 queuePrecedence;
   UINT32 queueWeight;
   char   dropAlgorithm[BUFLEN_8];
   UINT32 loMinThreshold;
   UINT32 loMaxThreshold;
   UINT32 hiMinThreshold;
   UINT32 hiMaxThreshold;
   SINT32 shapingRate;      /**< This applies to this queue only (not intf) */
   UINT32 shapingBurstSize; /**< This applies to this queue only (not intf) */
   SINT32 minBitRate;
   SINT32 dslLatency;
   SINT32 ptmPriority;
} CmsQosQueueInfo;


/** Configure main QoS properties (enable, DSCP, defaultQueue)
 *
 */
CmsRet dalQos_configQosMgmt(UBOOL8 enable, SINT32 dscp, UINT32 defaultQueue);

CmsRet dalQos_configQosMgmt_igd(UBOOL8 enable, SINT32 dscp, UINT32 defaultQueue);

CmsRet dalQos_configQosMgmt_dev2(UBOOL8 enable, SINT32 dscp, UINT32 defaultQueue);

#if defined(SUPPORT_DM_LEGACY98)
#define dalQos_configQosMgmt(e, d, q)  dalQos_configQosMgmt_igd((e), (d), (q))
#elif defined(SUPPORT_DM_HYBRID)
#define dalQos_configQosMgmt(e, d, q)  dalQos_configQosMgmt_igd((e), (d), (q))
#elif defined(SUPPORT_DM_PURE181)
#define dalQos_configQosMgmt(e, d, q)  dalQos_configQosMgmt_dev2((e), (d), (q))
#elif defined(SUPPORT_DM_DETECT)
#define dalQos_configQosMgmt(e, d, q)  (cmsMdm_isDataModelDevice2() ? \
                               dalQos_configQosMgmt_dev2((e), (d), (q)) : \
                               dalQos_configQosMgmt_igd((e), (d), (q)))
#endif




/** This function checks for duplicate QoS classification object instance.
 *
 * @param clsObj      (IN) pointer to the QoS classification object to be checked for.
 * @param isDuplicate (OUT) result of check. TRUE or FALSE.
 * @return CmsRet         enum.
 */
CmsRet dalQos_duplicateClassCheck(const void *clsObj, UBOOL8 *isDuplicate);
CmsRet dalQos_duplicateClassCheck_igd(const void *clsObj, UBOOL8 *isDuplicate);
CmsRet dalQos_duplicateClassCheck_dev2(const void *clsObj, UBOOL8 *isDuplicate);

#if defined(SUPPORT_DM_LEGACY98)
#define dalQos_duplicateClassCheck(e, d)  dalQos_duplicateClassCheck_igd((e), (d))
#elif defined(SUPPORT_DM_HYBRID)
#define dalQos_duplicateClassCheck(e, d)  dalQos_duplicateClassCheck_igd((e), (d))
#elif defined(SUPPORT_DM_PURE181)
#define dalQos_duplicateClassCheck(e, d)  dalQos_duplicateClassCheck_dev2((e), (d))
#elif defined(SUPPORT_DM_DETECT)
#define dalQos_duplicateClassCheck(e, d)  (cmsMdm_isDataModelDevice2() ? \
                               dalQos_duplicateClassCheck_dev2((e), (d)) : \
                               dalQos_duplicateClassCheck_igd((e), (d)))
#endif



/** This function adds a QoS queue object instance with the given params.
 *
 * @param intf       (IN) default queue interface
 * @param schedulerAlg (IN) default queue scheduler algorithm
 * @param weight       (IN) default queue weight
 * @param precedence (IN) default queue precedence
 * @param portId     (IN) layer 2 interface port ID
 * @param minRate (IN) default queue minimum rate
 * @param shapingRate (IN) default queue shaping rate
 * @param shapingBurstSize (IN) default queue shaphing burst size
 * @return CmsRet         enum.
 */
CmsRet dalQos_queueAdd(const char *intf, const char *schedulerAlg,
                   UBOOL8 enable, const char *queueName, UINT32 queueId,
                   UINT32 weight, UINT32 precedence,
                   SINT32 minRate, SINT32 shapingRate, UINT32 shapingBurstSize,
                   SINT32 dslLatency, SINT32 ptmPriority, const char *dropAlg,
                   UINT32 loMinThreshold, UINT32 loMaxThreshold,
                   UINT32 hiMinThreshold, UINT32 hiMaxThreshold);

CmsRet dalQos_queueAdd_igd(const char *intf, const char *schedulerAlg,
                   UBOOL8 enable, const char *queueName, UINT32 queueId,
                   UINT32 weight, UINT32 precedence,
                   SINT32 minRate, SINT32 shapingRate, UINT32 shapingBurstSize,
                   SINT32 dslLatency, SINT32 ptmPriority, const char *dropAlg,
                   UINT32 loMinThreshold, UINT32 loMaxThreshold,
                   UINT32 hiMinThreshold, UINT32 hiMaxThreshold);

CmsRet dalQos_queueAdd_dev2(const char *intf, const char *schedulerAlg,
                   UBOOL8 enable, const char *queueName, UINT32 queueId,
                   UINT32 weight, UINT32 precedence,
                   SINT32 minRate, SINT32 shapingRate, UINT32 shapingBurstSize,
                   SINT32 dslLatency, SINT32 ptmPriority, const char *dropAlg,
                   UINT32 loMinThreshold, UINT32 loMaxThreshold,
                   UINT32 hiMinThreshold, UINT32 hiMaxThreshold);

#if defined(SUPPORT_DM_LEGACY98)
#define dalQos_queueAdd(i, s, e, n, u, w, p, m, h, b, y, z, d0, d1, d2, d3, d4)  dalQos_queueAdd_igd((i), (s), (e), (n), (u), (w), (p), (m), (h), (b), (y), (z), (d0), (d1), (d2), (d3), (d4))
#elif defined(SUPPORT_DM_HYBRID)
#define dalQos_queueAdd(i, s, e, n, u, w, p, m, h, b, y, z, d0, d1, d2, d3, d4)  dalQos_queueAdd_igd((i), (s), (e), (n), (u), (w), (p), (m), (h), (b), (y), (z), (d0), (d1), (d2), (d3), (d4))
#elif defined(SUPPORT_DM_PURE181)
#define dalQos_queueAdd(i, s, e, n, u, w, p, m, h, b, y, z, d0, d1, d2, d3, d4)  dalQos_queueAdd_dev2((i), (s), (e), (n), (u), (w), (p), (m), (h), (b), (y), (z), (d0), (d1), (d2), (d3), (d4))
#elif defined(SUPPORT_DM_DETECT)
#define dalQos_queueAdd(i, s, e, n, u, w, p, m, h, b, y, z, d0, d1, d2, d3, d4)  (cmsMdm_isDataModelDevice2() ? \
        dalQos_queueAdd_dev2((i), (s), (e), (n), (u), (w), (p), (m), (h), (b), (y), (z), (d0), (d1), (d2), (d3), (d4)) : \
        dalQos_queueAdd_igd((i), (s), (e), (n), (u), (w), (p), (m), (h), (b), (y), (z), (d0), (d1), (d2), (d3), (d4)))
#endif




/** This function checks for duplicate QoS queue object instance.
 *
 *
 * @param isDuplicate (OUT) result of check. TRUE or FALSE.
 * @return CmsRet         enum.
 */
CmsRet dalQos_duplicateQueueCheck(UINT32 queueId, SINT32 dslLatency,
                            SINT32 ptmPriority, const char *intfName,
                            UBOOL8 *isDuplicate);

CmsRet dalQos_duplicateQueueCheck_igd(UINT32 queueId, SINT32 dslLatency,
                            SINT32 ptmPriority, const char *intfName,
                            UBOOL8 *isDuplicate);

CmsRet dalQos_duplicateQueueCheck_dev2(UINT32 queueId, SINT32 dslLatency,
                            SINT32 ptmPriority, const char *intfName,
                            UBOOL8 *isDuplicate);

#if defined(SUPPORT_DM_LEGACY98)
#define dalQos_duplicateQueueCheck(q, d, p, i, r)  dalQos_duplicateQueueCheck_igd((q), (d), (p), (i), (r))
#elif defined(SUPPORT_DM_HYBRID)
#define dalQos_duplicateQueueCheck(q, d, p, i, r)  dalQos_duplicateQueueCheck_igd((q), (d), (p), (i), (r))
#elif defined(SUPPORT_DM_PURE181)
#define dalQos_duplicateQueueCheck(q, d, p, i, r)  dalQos_duplicateQueueCheck_dev2((q), (d), (p), (i), (r))
#elif defined(SUPPORT_DM_DETECT)
#define dalQos_duplicateQueueCheck(q, d, p, i, r)  (cmsMdm_isDataModelDevice2() ? \
                dalQos_duplicateQueueCheck_dev2((q), (d), (p), (i), (r)) : \
                dalQos_duplicateQueueCheck_igd((q), (d), (p), (i), (r)))
#endif




/** Generic structure for holding QoS Classification info.  This struct
 *  contains all the params that CMS supports from the TR98 and TR181
 *  Classification object.
 */
typedef struct {
   UBOOL8 enable;
   char name[BUFLEN_64];  /* TR98 X_BROADCOM_COM_ClassName */
   UINT32 key;
   UINT32 order;
   SINT32 etherType;
   UBOOL8 etherTypeExclude;
   char ingressIntfFullPath[MDM_SINGLE_FULLPATH_BUFLEN];  /* TR98 classInterface */
   UBOOL8 ingressIsSpecificWan;   /* is ingress intf a specific WAN intf (not including MDMVS_WAN) */
   UBOOL8 ingressIsSpecificLan;   /* is ingress intf a specific LAN intf (not including MDMVS_LAN) */
   char destIP[CMS_IPADDR_LENGTH];
   char destMask[CMS_IPADDR_LENGTH];
   UBOOL8 destIPExclude;
   char sourceIP[CMS_IPADDR_LENGTH];
   char sourceMask[CMS_IPADDR_LENGTH];
   UBOOL8 sourceIPExclude;
   SINT32 protocol;
   UBOOL8 protocolExclude;
   SINT32 destPort;
   SINT32 destPortRangeMax;
   UBOOL8 destPortExclude;
   SINT32 sourcePort;
   SINT32 sourcePortRangeMax;
   UBOOL8 sourcePortExclude;
   char destMACAddress[MAC_STR_LEN+1];
   char destMACMask[MAC_STR_LEN+1];
   UBOOL8 destMACExclude;
   char sourceMACAddress[MAC_STR_LEN+1];
   char sourceMACMask[MAC_STR_LEN+1];
   UBOOL8 sourceMACExclude;
   char sourceVendorClassID[BUFLEN_64];
   UBOOL8 sourceVendorClassIDExclude;
   char sourceUserClassID[BUFLEN_64];  /* TR181 this is HEXBINARY */
   UBOOL8 sourceUserClassIDExclude;
   SINT32 DSCPCheck;
   UBOOL8 DSCPExclude;
   SINT32 DSCPMark;
   SINT32 ethernetPriorityCheck;
   UBOOL8 ethernetPriorityExclude;
   SINT32 ethernetPriorityMark;
   SINT32 vlanIdTag;
   char egressIntfName[CMS_IFNAME_LENGTH]; /* X_BROADCOM_COM_egressInterface (L3 intf) */
   SINT32 egressQueueInstance;  /* instance number of egress Queue obj */
   SINT32 policerInstance;  /* instance number of policer obj */
   SINT32 classRate;        /* class rate limiter value */
} CmsQosClassInfo;




/** Generic structure for holding QoS Policer info.  This contains all the
 *  params that CMS supports from the TR98 and TR181 Policer object.
 */
typedef struct {
   UBOOL8 enable;
   char name[BUFLEN_32];
   char meterType[BUFLEN_32];
   char conformingAction[BUFLEN_32];
   char partialConformingAction[BUFLEN_32];
   char nonConformingAction[BUFLEN_32];
   UINT32 committedRate;
   UINT32 committedBurstSize;
   UINT32 excessBurstSize;
   UINT32 peakRate;
   UINT32 peakBurstSize;
   UINT32 usPolicerInfo;
   UINT32 dsPolicerInfo;
} CmsQosPolicerInfo;


/** This function adds a QoS policer object instance.
 *
 * @param policerInfo   (IN) pointer to Policer Info struct for policer obj
 *                           to be added.
 * @return CmsRet         enum.
 */
CmsRet dalQos_policerAdd(const CmsQosPolicerInfo *policerInfo);
CmsRet dalQos_policerAdd_igd(const CmsQosPolicerInfo *policerInfo);
CmsRet dalQos_policerAdd_dev2(const CmsQosPolicerInfo *policerInfo);

#if defined(SUPPORT_DM_LEGACY98)
#define dalQos_policerAdd(p)           dalQos_policerAdd_igd((p))
#elif defined(SUPPORT_DM_HYBRID)
#define dalQos_policerAdd(p)           dalQos_policerAdd_igd((p))
#elif defined(SUPPORT_DM_PURE181)
#define dalQos_policerAdd(p)           dalQos_policerAdd_dev2((p))
#elif defined(SUPPORT_DM_DETECT)
#define dalQos_policerAdd(p)  (cmsMdm_isDataModelDevice2() ? \
                                       dalQos_policerAdd_dev2((p)) : \
                                       dalQos_policerAdd_igd((p)))
#endif



/** This function checks for duplicate QoS policer object instance.
 *
 * @param policerInfo  (IN) pointer to the QoS policer info to be checked for.
 * @param isDuplicate (OUT) result of check. TRUE or FALSE.
 * @return CmsRet         enum.
 */
CmsRet dalQos_duplicatePolicerCheck(const CmsQosPolicerInfo *policerInfo, UBOOL8 *isDuplicate);
CmsRet dalQos_duplicatePolicerCheck_igd(const CmsQosPolicerInfo *policerInfo, UBOOL8 *isDuplicate);
CmsRet dalQos_duplicatePolicerCheck_dev2(const CmsQosPolicerInfo *policerInfo, UBOOL8 *isDuplicate);

#if defined(SUPPORT_DM_LEGACY98)
#define dalQos_duplicatePolicerCheck(e, d)  dalQos_duplicatePolicerCheck_igd((e), (d))
#elif defined(SUPPORT_DM_HYBRID)
#define dalQos_duplicatePolicerCheck(e, d)  dalQos_duplicatePolicerCheck_igd((e), (d))
#elif defined(SUPPORT_DM_PURE181)
#define dalQos_duplicatePolicerCheck(e, d)  dalQos_duplicatePolicerCheck_dev2((e), (d))
#elif defined(SUPPORT_DM_DETECT)
#define dalQos_duplicatePolicerCheck(e, d)  (cmsMdm_isDataModelDevice2() ? \
                               dalQos_duplicatePolicerCheck_dev2((e), (d)) : \
                               dalQos_duplicatePolicerCheck_igd((e), (d)))
#endif



/** This function converts queue precedence to priority for the queue interface.
 *
 * @param qIntfPath (IN)  queue interface path.
 * @param prec      (IN)  queue precedence.
 * @param prio      (OUT) queue priority.
 * @return CmsRet         enum.
 */
CmsRet dalQos_convertPrecedenceToPriority(const char *qIntfPath, UINT32 prec, UINT32 *prio);

/** This function returns an unused queue ID for the layer2 interface.
 *
 * @param l2Ifname (IN)  layer2 interface name for the queue.
 * @param prio     (IN)  queue priority.
 * @param alg      (IN)  scheduling algorithm for the queue.
 * @param qId      (OUT) unused queue ID
 * @return CmsRet         enum.
 */
CmsRet dalQos_getAvailableQueueId(const char *l2Ifname,
                                  UINT32 prio, const char *alg, UINT32 *qId);


/** Get the number of Tx Queues and Levels on this given interface.
 *
 * @param l2IfName  (IN)  layer2 interface name for the queue.
 * @param numQueues (OUT) If pointer is not NULL, will be filled in with num
 *                        TX queues available on this intf.
 * @param numLevels (OUT) If pointer is not NULL, will be filled in with num
 *                        Levels available on this intf.
 *
 */
void dalQos_getIntfNumQueuesAndLevels(const char *l2IfName,
                                      UINT32 *numQueues, UINT32 *numLevels);


/** This function returns an unused class key.
 *
 * @param clsKey      (OUT) unused class key
 * @return CmsRet         enum.
 */
CmsRet dalQos_getAvailableClsKey(UINT32 *clsKey);


CmsRet dalQos_addAccordingQueueStatsobject(const _QMgmtQueueObject *qObj);

CmsRet dalQos_delAccordingQueueStatsobject(const _QMgmtQueueObject *qObj);

#endif   /* __CMS_QOS_H */
