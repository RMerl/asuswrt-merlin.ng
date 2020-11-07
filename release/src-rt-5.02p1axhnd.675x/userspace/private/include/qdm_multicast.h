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


#include "cms.h"
#include "cms_mdm.h"
#include "cms_obj.h"
#include "cms_qdm.h"
#include "cms_util.h"


#ifndef _QDM_MULTICAST_H_
#define _QDM_MULTICAST_H_


/*!\file qdm_multicast.h
 * \brief This file contains declarations for multicast related functions.
 *
 */

/** Return true if IGMP snooping is enabled on the specified bridge name.
 *
 * @param bridgeIfName  (IN) the Linux bridge name, e.g. br0
 *
 * @return true if IGMP snooping is enabled on the specified bridge name.
 */
UBOOL8 qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked(const char *bridgeIfName);

UBOOL8 qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked_igd(const char *bridgeIfName);

UBOOL8 qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked_dev2(const char *bridgeIfName);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked(n)  qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked_igd((n))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked(n)  qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked_igd((n))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked(n)  qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked_dev2((n))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked(n)  (cmsMdm_isDataModelDevice2() ? \
               qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked_dev2((n)) : \
               qdmMulti_isIgmpSnoopingEnabledOnBridgeLocked_igd((n)))

#endif




/** Get a bunch of info about the IPv4 (IGMP) or IPv6 (MLD) snooping config.
 *
 * @param bridgeIfName   (IN) the Linux bridge name, e.g. br0
 * @param isMld          (IN) Are we asking about IPv6 MLD (or IPv4 IGMP)
 * @param mode           (OUT) snooping mode
 * @param lanToLanEnable (OUT) is Lan to Lan mcast enabled
 * @param enabled        (OUT) is snooping enabled
 *
 * @return CmsRet enum.
 */
CmsRet qdmMulti_getSnoopingInfoLocked(const char *brIfName, UBOOL8 isMld,
        UINT32 *mode, int *lanToLanEnable, UBOOL8 * enabled);

CmsRet qdmMulti_getSnoopingInfoLocked_igd(const char *brIfName, UBOOL8 isMld,
        UINT32 *mode, int *lanToLanEnable, UBOOL8 * enabled);

CmsRet qdmMulti_getSnoopingInfoLocked_dev2(const char *brIfName, UBOOL8 isMld,
        UINT32 *mode, int *lanToLanEnable, UBOOL8 * enabled);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_getSnoopingInfoLocked(a, b, c, d, e)  qdmMulti_getSnoopingInfoLocked_igd((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_getSnoopingInfoLocked(a, b, c, d, e)  qdmMulti_getSnoopingInfoLocked_igd((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_getSnoopingInfoLocked(a, b, c, d, e)  qdmMulti_getSnoopingInfoLocked_dev2((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_getSnoopingInfoLocked(a, b, c, d, e)  (cmsMdm_isDataModelDevice2() ? \
           qdmMulti_getSnoopingInfoLocked_dev2((a), (b), (c), (d), (e)) : \
           qdmMulti_getSnoopingInfoLocked_igd((a), (b), (c), (d), (e)))

#endif




/** Given a fullpath to an IGMPsnoopingObj or MLDsnoopingObj, return the
 *  associated bridge interface name.
 *
 * @param snoopFullPath (IN) fullpath to the IGMP or MLD snooping object.
 * @param bridgeIntfName (OUT) On successful return, buffer is filled with
 *            the bridge interface name associated with this snooping object.
 *            Buffer must be at least CMS_IFNAME_LENGTH bytes.
 */
CmsRet qdmMulti_getAssociatedBridgeIntfNameLocked(const char *snoopFullPath,
                                                  char *bridgeIntfName);

CmsRet qdmMulti_getAssociatedBridgeIntfNameLocked_igd(const char *snoopFullPath,
                                                      char *bridgeIntfName);

CmsRet qdmMulti_getAssociatedBridgeIntfNameLocked_dev2(const char *snoopFullPath,
                                                       char *bridgeIntfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_getAssociatedBridgeIntfNameLocked(a, b)  qdmMulti_getAssociatedBridgeIntfNameLocked_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_getAssociatedBridgeIntfNameLocked(a, b)  qdmMulti_getAssociatedBridgeIntfNameLocked_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_getAssociatedBridgeIntfNameLocked(a, b)  qdmMulti_getAssociatedBridgeIntfNameLocked_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_getAssociatedBridgeIntfNameLocked(a, b)  (cmsMdm_isDataModelDevice2() ? \
           qdmMulti_getAssociatedBridgeIntfNameLocked_dev2((a), (b)) : \
           qdmMulti_getAssociatedBridgeIntfNameLocked_igd((a), (b)))

#endif



/** Given a fullpath to an IGMPsnoopingObj or MLDsnoopingObj, return the
 *  associated bridge mode.
 *
 * @param snoopFullPath (IN) fullpath to the IGMP or MLD snooping object.
 * @param mode          (OUT) On successful return, return bridge mode
 */
CmsRet qdmMulti_getAssociatedBridgeModeLocked(const char *snoopFullPath,
                                              int *mode);

CmsRet qdmMulti_getAssociatedBridgeModeLocked_igd(const char *snoopFullPath,
                                              int *mode);

CmsRet qdmMulti_getAssociatedBridgeModeLocked_dev2(const char *snoopFullPath,
                                              int *mode);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_getAssociatedBridgeModeLocked(a, b)  qdmMulti_getAssociatedBridgeModeLocked_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_getAssociatedBridgeModeLocked(a, b)  qdmMulti_getAssociatedBridgeModeLocked_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_getAssociatedBridgeModeLocked(a, b)  qdmMulti_getAssociatedBridgeModeLocked_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_getAssociatedBridgeModeLocked(a, b)  (cmsMdm_isDataModelDevice2() ? \
           qdmMulti_getAssociatedBridgeModeLocked_dev2((a), (b)) : \
           qdmMulti_getAssociatedBridgeModeLocked_igd((a), (b)))

#endif



/** Given a bridge interface name, return the fullpath to the associated
 *  IGMP snooping object.
 *
 * @param brIntfName (IN) bridge linux interface name, e.g. br0
 * @param fullPath  (OUT) on successful return, the fullpath to the associated
 *                        IGMP snooping object.  Caller is responsible for
 *                        freeing this buffer.
 */
CmsRet qdmMulti_getAssociatedIgmpSnoopingFullPathLocked(const char *brIntfName,
                                                        char **fullPath);

CmsRet qdmMulti_getAssociatedIgmpSnoopingFullPathLocked_igd(const char *brIntfName,
                                                            char **fullPath);

CmsRet qdmMulti_getAssociatedIgmpSnoopingFullPathLocked_dev2(const char *brIntfName,
                                                             char **fullPath);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_getAssociatedIgmpSnoopingFullPathLocked(a, b)  qdmMulti_getAssociatedIgmpSnoopingFullPathLocked_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_getAssociatedIgmpSnoopingFullPathLocked(a, b)  qdmMulti_getAssociatedIgmpSnoopingFullPathLocked_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_getAssociatedIgmpSnoopingFullPathLocked(a, b)  qdmMulti_getAssociatedIgmpSnoopingFullPathLocked_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_getAssociatedIgmpSnoopingFullPathLocked(a, b)  (cmsMdm_isDataModelDevice2() ? \
           qdmMulti_getAssociatedIgmpSnoopingFullPathLocked_dev2((a), (b)) : \
           qdmMulti_getAssociatedIgmpSnoopingFullPathLocked_igd((a), (b)))

#endif



/** Given a bridge interface name, return the fullpath to the associated
 *  MLD snooping object.
 *
 * @param brIntfName (IN) bridge linux interface name, e.g. br0
 * @param fullPath  (OUT) on successful return, the fullpath to the associated
 *                        MLD snooping object.  Caller is responsible for
 *                        freeing this buffer.
 */
CmsRet qdmMulti_getAssociatedMldSnoopingFullPathLocked(const char *brIntfName,
                                                        char **fullPath);

CmsRet qdmMulti_getAssociatedMldSnoopingFullPathLocked_igd(const char *brIntfName,
                                                            char **fullPath);

CmsRet qdmMulti_getAssociatedMldSnoopingFullPathLocked_dev2(const char *brIntfName,
                                                             char **fullPath);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_getAssociatedMldSnoopingFullPathLocked(a, b)  qdmMulti_getAssociatedMldSnoopingFullPathLocked_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_getAssociatedMldSnoopingFullPathLocked(a, b)  qdmMulti_getAssociatedMldSnoopingFullPathLocked_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_getAssociatedMldSnoopingFullPathLocked(a, b)  qdmMulti_getAssociatedMldSnoopingFullPathLocked_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_getAssociatedMldSnoopingFullPathLocked(a, b)  (cmsMdm_isDataModelDevice2() ? \
           qdmMulti_getAssociatedMldSnoopingFullPathLocked_dev2((a), (b)) : \
           qdmMulti_getAssociatedMldSnoopingFullPathLocked_igd((a), (b)))

#endif




/** Return TRUE if IGMP proxy is enabled on the specified intfName.
 *
 *  @param intfName (IN) WAN IP interface name
 *
 *  @return TRUE if specified intf name has IGMP proxy enabled
 */
UBOOL8 qdmMulti_isIgmpProxyEnabledOnIntfNameLocked(const char *intfName);
UBOOL8 qdmMulti_isIgmpProxyEnabledOnIntfNameLocked_igd(const char *intfName);
UBOOL8 qdmMulti_isIgmpProxyEnabledOnIntfNameLocked_dev2(const char *intfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_isIgmpProxyEnabledOnIntfNameLocked(i)   qdmMulti_isIgmpProxyEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_isIgmpProxyEnabledOnIntfNameLocked(i)   qdmMulti_isIgmpProxyEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_isIgmpProxyEnabledOnIntfNameLocked(i)   qdmMulti_isIgmpProxyEnabledOnIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_isIgmpProxyEnabledOnIntfNameLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                   qdmMulti_isIgmpProxyEnabledOnIntfNameLocked_dev2((i)) : \
                   qdmMulti_isIgmpProxyEnabledOnIntfNameLocked_igd((i)))
#endif

/** Return TRUE if IGMP multicast source is enabled on the specified intfName.
 *
 *  @param intfName (IN) WAN IP interface name
 *
 *  @return TRUE if specified intf name has IGMP multicast source enabled
 */
UBOOL8 qdmMulti_isIgmpSourceEnabledOnIntfNameLocked(const char *intfName);
UBOOL8 qdmMulti_isIgmpSourceEnabledOnIntfNameLocked_igd(const char *intfName);
UBOOL8 qdmMulti_isIgmpSourceEnabledOnIntfNameLocked_dev2(const char *intfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_isIgmpSourceEnabledOnIntfNameLocked(i)   qdmMulti_isIgmpSourceEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_isIgmpSourceEnabledOnIntfNameLocked(i)   qdmMulti_isIgmpSourceEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_isIgmpSourceEnabledOnIntfNameLocked(i)   qdmMulti_isIgmpSourceEnabledOnIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_isIgmpSourceEnabledOnIntfNameLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                   qdmMulti_isIgmpSourceEnabledOnIntfNameLocked_dev2((i)) : \
                   qdmMulti_isIgmpSourceEnabledOnIntfNameLocked_dev2((i)))
#endif


/** Return TRUE if MLD proxy is enabled on the specified intfName.
 *  Note that in Hybrid mode, we look in the TR181 part of the data model
 *  for IPv6 Enabled.
 *
 *  @param intfName (IN) WAN IP interface name
 *
 *  @return TRUE if specified intf name has MLD proxy enabled
 */
UBOOL8 qdmMulti_isMldProxyEnabledOnIntfNameLocked(const char *intfName);
UBOOL8 qdmMulti_isMldProxyEnabledOnIntfNameLocked_igd(const char *intfName);
UBOOL8 qdmMulti_isMldProxyEnabledOnIntfNameLocked_dev2(const char *intfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_isMldProxyEnabledOnIntfNameLocked(i)   qdmMulti_isMldProxyEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_isMldProxyEnabledOnIntfNameLocked(i)   qdmMulti_isMldProxyEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_isMldProxyEnabledOnIntfNameLocked(i)   qdmMulti_isMldProxyEnabledOnIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_isMldProxyEnabledOnIntfNameLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                   qdmMulti_isMldProxyEnabledOnIntfNameLocked_dev2((i)) : \
                   qdmMulti_isMldProxyEnabledOnIntfNameLocked_dev2((i)))
#endif


/** Return TRUE if MLD multicast source is enabled on the specified intfName.
 *  Note that in Hybrid mode, we look in the TR181 part of the data model
 *  for IPv6 Enabled.
 *
 *  @param intfName (IN) WAN IP interface name
 *
 *  @return TRUE if specified intf name has MLD multicast source enabled
 */
UBOOL8 qdmMulti_isMldSourceEnabledOnIntfNameLocked(const char *intfName);
UBOOL8 qdmMulti_isMldSourceEnabledOnIntfNameLocked_igd(const char *intfName);
UBOOL8 qdmMulti_isMldSourceEnabledOnIntfNameLocked_dev2(const char *intfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmMulti_isMldSourceEnabledOnIntfNameLocked(i)   qdmMulti_isMldSourceEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmMulti_isMldSourceEnabledOnIntfNameLocked(i)   qdmMulti_isMldSourceEnabledOnIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmMulti_isMldSourceEnabledOnIntfNameLocked(i)   qdmMulti_isMldSourceEnabledOnIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmMulti_isMldSourceEnabledOnIntfNameLocked(i)   (cmsMdm_isDataModelDevice2() ? \
                   qdmMulti_isMldSourceEnabledOnIntfNameLocked_dev2((i)) : \
                   qdmMulti_isMldSourceEnabledOnIntfNameLocked_dev2((i)))
#endif

#endif /* _QDM_MULTICAST_H_ */



