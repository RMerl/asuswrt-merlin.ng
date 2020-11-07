/***********************************************************************
 *
 *  Copyright (c) 2013  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2013:proprietary:standard

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


#ifndef _QDM_QOS_H_
#define _QDM_QOS_H_

#include "cms.h"
#include "cms_qos.h"



/*!\file qdm_qos.h
 * \brief This file contains declarations for QoS related functions.
 *
 */


/** Check if QoS is enabled.
 *  Note: TR-181 does not define QoS enable. Therefore, _dev2 function always
 *        returns TRUE.
 *
 * @return TRUE or FALSE.
 */
UBOOL8 qdmQos_isQosEnabled(void);
UBOOL8 qdmQos_isQosEnabled_igd(void);
UBOOL8 qdmQos_isQosEnabled_dev2(void);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmQos_isQosEnabled()  qdmQos_isQosEnabled_igd()
#elif defined(SUPPORT_DM_HYBRID)
#define qdmQos_isQosEnabled()  qdmQos_isQosEnabled_igd()
#elif defined(SUPPORT_DM_PURE181)
#define qdmQos_isQosEnabled()  qdmQos_isQosEnabled_dev2()
#elif defined(SUPPORT_DM_DETECT)
#define qdmQos_isQosEnabled(o)  (cmsMdm_isDataModelDevice2() ? \
           qdmQos_isQosEnabled_dev2() : \
           qdmQos_isQosEnabled_igd())

#endif




/** Get information about the QoS Queue specified by classQueue.
 *  This function replaces rutQos_getClassQueueInfo
 *
 * @param classQueue  (IN) Identifier of the QoS queue.  Currently, we use
 *              the instance id of the queue.  (This is how TR98 code does it.)
 *              TR181 does the same thing for now, but that may change.  If/
 *              when it does change, the classQueue param can be used as
 *              a cookie to identify the desired queue.
 *
 * @param enabled   (OUT) If not NULL, will be set to TRUE or FALSE of the
 *                        specified target queue.
 * @param qidMark   (OUT) If not NULL, will be set to the qidMark of the
 *                        queue as calculated by rutQos_getQidMark.
 * @param L2IntfName (OUT) If not NULL, will contain the layer 2 IntfName of
 *                         the specified target queue.  Buffer must be at
 *                         least CMS_IFNAME_LENGTH bytes long.
 *
 * @return CmsRet enum.
 */
CmsRet qdmQos_getQueueInfoByClassQueueLocked(SINT32 classQueue,
                         UBOOL8 *enabled, UINT32 *qidMark, char *l2IntfName);

CmsRet qdmQos_getQueueInfoByClassQueueLocked_igd(SINT32 classQueue,
                         UBOOL8 *enabled, UINT32 *qidMark, char *l2IntfName);

CmsRet qdmQos_getQueueInfoByClassQueueLocked_dev2(SINT32 classQueue,
                         UBOOL8 *enabled, UINT32 *qidMark, char *l2IntfName);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmQos_getQueueInfoByClassQueueLocked(a, b, c, d)  qdmQos_getQueueInfoByClassQueueLocked_igd((a), (b), (c), (d))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmQos_getQueueInfoByClassQueueLocked(a, b, c, d)  qdmQos_getQueueInfoByClassQueueLocked_igd((a), (b), (c), (d))
#elif defined(SUPPORT_DM_PURE181)
#define qdmQos_getQueueInfoByClassQueueLocked(a, b, c, d)  qdmQos_getQueueInfoByClassQueueLocked_dev2((a), (b), (c), (d))
#elif defined(SUPPORT_DM_DETECT)
#define qdmQos_getQueueInfoByClassQueueLocked(a, b, c, d)  (cmsMdm_isDataModelDevice2() ? \
           qdmQos_getQueueInfoByClassQueueLocked_dev2((a), (b), (c), (d)) : \
           qdmQos_getQueueInfoByClassQueueLocked_igd((a), (b), (c), (d)))

#endif




/** Convert a TR98 or TR181 QoS Queue object into CmsQoSQueueInfo struct.
 *
 */
void qdmQos_convertDmQueueObjToCmsQueueInfoLocked(const void *obj,
                                             CmsQosQueueInfo *qInfo);

void qdmQos_convertDmQueueObjToCmsQueueInfoLocked_igd(const void *obj,
                                                 CmsQosQueueInfo *qInfo);

void qdmQos_convertDmQueueObjToCmsQueueInfoLocked_dev2(const void *obj,
                                                  CmsQosQueueInfo *qInfo);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmQos_convertDmQueueObjToCmsQueueInfoLocked(a, b)  qdmQos_convertDmQueueObjToCmsQueueInfoLocked_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmQos_convertDmQueueObjToCmsQueueInfoLocked(a, b)  qdmQos_convertDmQueueObjToCmsQueueInfoLocked_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmQos_convertDmQueueObjToCmsQueueInfoLocked(a, b)  qdmQos_convertDmQueueObjToCmsQueueInfoLocked_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmQos_convertDmQueueObjToCmsQueueInfoLocked(a, b)  (cmsMdm_isDataModelDevice2() ? \
           qdmQos_convertDmQueueObjToCmsQueueInfoLocked_dev2((a), (b)) : \
           qdmQos_convertDmQueueObjToCmsQueueInfoLocked_igd((a), (b)))

#endif




/** Convert a TR98 or TR181 QoS Classification object into a
 *  CmsQoSClassInfo struct.
 *
 */
void qdmQos_convertDmClassObjToCmsClassInfoLocked(const void *obj,
                                             CmsQosClassInfo *cInfo);

void qdmQos_convertDmClassObjToCmsClassInfoLocked_igd(const void *obj,
                                                 CmsQosClassInfo *cInfo);

void qdmQos_convertDmClassObjToCmsClassInfoLocked_dev2(const void *obj,
                                                  CmsQosClassInfo *cInfo);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmQos_convertDmClassObjToCmsClassInfoLocked(a, b)  qdmQos_convertDmClassObjToCmsClassInfoLocked_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmQos_convertDmClassObjToCmsClassInfoLocked(a, b)  qdmQos_convertDmClassObjToCmsClassInfoLocked_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmQos_convertDmClassObjToCmsClassInfoLocked(a, b)  qdmQos_convertDmClassObjToCmsClassInfoLocked_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmQos_convertDmClassObjToCmsClassInfoLocked(a, b)  (cmsMdm_isDataModelDevice2() ? \
           qdmQos_convertDmClassObjToCmsClassInfoLocked_dev2((a), (b)) : \
           qdmQos_convertDmClassObjToCmsClassInfoLocked_igd((a), (b)))

#endif




typedef enum CmsQosClassRefTarget {
   CMS_QOS_REF_TARGET_POLICER=0,
   CMS_QOS_REF_TARGET_QUEUE=1
} CmsQosClassRefTargetEnum;

/** Check if POLICER or QUEUE object is currently referenced by any
 *  classification entry.
 *
 * @param targ      (IN)  one of the CmsQosClassRefTargetEnum's
 * @param instance  (IN)  instance number of the target policer/queue entry
 * @param isRefered (OUT) TRUE if there is a classification entry which
 *                        refers to the target
 *
 * @return CmsRet enum
 */
CmsRet qdmQos_referenceCheckLocked(CmsQosClassRefTargetEnum targ, SINT32 instance, UBOOL8 *isRefered);
CmsRet qdmQos_referenceCheckLocked_igd(CmsQosClassRefTargetEnum targ, SINT32 instance, UBOOL8 *isRefered);
CmsRet qdmQos_referenceCheckLocked_dev2(CmsQosClassRefTargetEnum targ, SINT32 instance, UBOOL8 *isRefered);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmQos_referenceCheckLocked(a, b, c)  qdmQos_referenceCheckLocked_igd((a), (b), (c))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmQos_referenceCheckLocked(a, b, c)  qdmQos_referenceCheckLocked_igd((a), (b), (c))
#elif defined(SUPPORT_DM_PURE181)
#define qdmQos_referenceCheckLocked(a, b, c)  qdmQos_referenceCheckLocked_dev2((a), (b), (c))
#elif defined(SUPPORT_DM_DETECT)
#define qdmQos_referenceCheckLocked(a, b, c)  (cmsMdm_isDataModelDevice2() ? \
                   qdmQos_referenceCheckLocked_dev2((a), (b), (c)) : \
                   qdmQos_referenceCheckLocked_igd((a), (b), (c)))

#endif




/** Get info about specified Qos Policer
 *
 * @param instance  (IN)  instance number of the policer entry
 * @param policerInfo (OUT) Caller passes in a pointer to a CmsQosPolicerInfo
 *                          struct.  This function will fill it out.
 *
 * @return CmsRet enum
 */
CmsRet qdmQos_getClassPolicerInfoLocked(SINT32 instance, CmsQosPolicerInfo *policerInfo);
CmsRet qdmQos_getClassPolicerInfoLocked_igd(SINT32 instance, CmsQosPolicerInfo *policerInfo);
CmsRet qdmQos_getClassPolicerInfoLocked_dev2(SINT32 instance, CmsQosPolicerInfo *policerInfo);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmQos_getClassPolicerInfoLocked(a, b)  qdmQos_getClassPolicerInfoLocked_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmQos_getClassPolicerInfoLocked(a, b)  qdmQos_getClassPolicerInfoLocked_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmQos_getClassPolicerInfoLocked(a, b)  qdmQos_getClassPolicerInfoLocked_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmQos_getClassPolicerInfoLocked(a, b)  (cmsMdm_isDataModelDevice2() ? \
                           qdmQos_getClassPolicerInfoLocked_dev2((a), (b)) : \
                           qdmQos_getClassPolicerInfoLocked_igd((a), (b)))

#endif




/** Return TRUE if there is a classifier which has output queue which is on
 *  an Ethernet switch port.
 *
 * @param excludeClassKey  (IN) If a classification is currently being
 *                unconfigured or deleted, exclude it from consideration.
 *                If all classifications should be considered, pass in
 *                QOS_CLS_INVALID_INDEX
 *
 * @return TRUE if there is a classifier which has output queue which is on
 *              an Ethernet switch port
 */
UBOOL8 qdmQos_isEgressEthPortClassificationPresentLocked(UINT32 excludeClassKey);
UBOOL8 qdmQos_isEgressEthPortClassificationPresentLocked_igd(UINT32 excludeClassKey);
UBOOL8 qdmQos_isEgressEthPortClassificationPresentLocked_dev2(UINT32 excludeClassKey);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmQos_isEgressEthPortClassificationPresentLocked(a)  qdmQos_isEgressEthPortClassificationPresentLocked_igd((a))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmQos_isEgressEthPortClassificationPresentLocked(a)  qdmQos_isEgressEthPortClassificationPresentLocked_igd((a))
#elif defined(SUPPORT_DM_PURE181)
#define qdmQos_isEgressEthPortClassificationPresentLocked(a)  qdmQos_isEgressEthPortClassificationPresentLocked_dev2((a))
#elif defined(SUPPORT_DM_DETECT)
#define qdmQos_isEgressEthPortClassificationPresentLocked(a)  (cmsMdm_isDataModelDevice2() ? \
                           qdmQos_isEgressEthPortClassificationPresentLocked_dev2((a)) : \
                           qdmQos_isEgressEthPortClassificationPresentLocked_igd((a)))

#endif




/** Return TRUE if there is a classifier which has input from a LAN side
 *  Ethernet switch port and output queue which is on a LAN side Ethernet
 *  switch port.
 *
 * @param excludeClassKey  (IN) If a classification is currently being
 *                unconfigured or deleted, exclude it from consideration.
 *                If all classifications should be considered, pass in
 *                QOS_CLS_INVALID_INDEX
 *
 * @return TRUE if there is a classifier which has input from a LAN side
 *              Ethernet switch port and output queue which is on a LAN side
 *              Ethernet switch port.
 */
UBOOL8 qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked(UINT32 excludeClassKey);
UBOOL8 qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked_igd(UINT32 excludeClassKey);
UBOOL8 qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked_dev2(UINT32 excludeClassKey);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked(a)  qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked_igd((a))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked(a)  qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked_igd((a))
#elif defined(SUPPORT_DM_PURE181)
#define qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked(a)  qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked_dev2((a))
#elif defined(SUPPORT_DM_DETECT)
#define qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked(a)  (cmsMdm_isDataModelDevice2() ? \
                           qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked_dev2((a)) : \
                           qdmQos_isLanSwitchPortToLanSwitchPortClassificationPresentLocked_igd((a)))

#endif




#endif /* _QDM_QOS_H_ */



