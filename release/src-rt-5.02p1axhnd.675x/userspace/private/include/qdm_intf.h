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


#ifndef _QDM_INTF_H_
#define _QDM_INTF_H_


/*!\file qdm_intf.h
 * \brief This file contains declarations for Interface related functions.
 *
 */

/** Get the full path name from the Linux interface name.
 *
 * @param intfname   (IN) the Linux interface name.
 * @param layer2     (IN) boolean to indicate whether intfname is a layer 2 or layer 3 interface name.
 * @param mdmPath    (OUT)the full path name. Caller is responsible for freeing the memory.
 *
 * @return CmsRet enum.
 */
CmsRet qdmIntf_intfnameToFullPathLocked(const char *intfname, UBOOL8 layer2, char **mdmPath);

CmsRet qdmIntf_intfnameToFullPathLocked_igd(const char *intfname, UBOOL8 layer2, char **mdmPath);

CmsRet qdmIntf_intfnameToFullPathLocked_dev2(const char *intfname, UBOOL8 layer2, char **mdmPath);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIntf_intfnameToFullPathLocked(a, b, c)  qdmIntf_intfnameToFullPathLocked_igd((a), (b), (c))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIntf_intfnameToFullPathLocked(a, b, c)  qdmIntf_intfnameToFullPathLocked_igd((a), (b), (c))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIntf_intfnameToFullPathLocked(a, b, c)  qdmIntf_intfnameToFullPathLocked_dev2((a), (b), (c))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIntf_intfnameToFullPathLocked(a, b, c)  (cmsMdm_isDataModelDevice2() ? \
                    qdmIntf_intfnameToFullPathLocked_dev2((a), (b), (c)) : \
                    qdmIntf_intfnameToFullPathLocked_igd((a), (b), (c)))
#endif




/** Get the Linux interface name from the full path name.
 *
 * @param mdmPath    (IN) the full path name.
 * @param intfname   (OUT) On success, this buffer will be filled with
 *              the Linux interface name.  The buffer must be at least
 *              CMS_IFNAME_LENGTH bytes long.
 *
 * @return CmsRet enum.
 */
CmsRet qdmIntf_fullPathToIntfnameLocked(const char *mdmPath, char *intfname);

CmsRet qdmIntf_fullPathToIntfnameLocked_igd(const char *mdmPath, char *intfname);

CmsRet qdmIntf_fullPathToIntfnameLocked_dev2(const char *mdmPath, char *intfname);


#if defined(SUPPORT_DM_LEGACY98)
#define qdmIntf_fullPathToIntfnameLocked(a, b)  qdmIntf_fullPathToIntfnameLocked_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIntf_fullPathToIntfnameLocked(a, b)  qdmIntf_fullPathToIntfnameLocked_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIntf_fullPathToIntfnameLocked(a, b)  qdmIntf_fullPathToIntfnameLocked_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIntf_fullPathToIntfnameLocked(a, b)  (cmsMdm_isDataModelDevice2() ? \
                          qdmIntf_fullPathToIntfnameLocked_dev2((a), (b)) : \
                          qdmIntf_fullPathToIntfnameLocked_igd((a), (b)))
#endif




/** Get the interface value of the Name parameter in object pointed to by fullPath.
  * only dev2 versions of these functions are needed right now.
  *
  * @param fullPath    (IN) the fullpath to the object.
  * @param intfname    (OUT) a buffer to hold the returned interface value.
  * @param intfnameLen (IN) Length of intfname.
  *
  * @return CmsRet.
  */
CmsRet qdmIntf_getIntfnameFromFullPathLocked_dev2(const char *fullPath,
                                                  char *intfname, UINT32 intfnameLen);


/** Get the interface value of the Name parameter in object pointed to by pathDescIn.
  * only dev2 versions of these functions are needed right now.
  *
  * @param pathDescIn   (IN) the path descriptor to the object.
  * @param intfname     (OUT) a buffer to hold the returned interface value.
  * @param intfnameLen  (IN) Length of intfname.
  *
  * @return CmsRet.
  */
CmsRet qdmIntf_getIntfnameFromPathDescLocked_dev2(const MdmPathDescriptor *pathDescIn,
                                                  char *intfname, UINT32 intfnameLen);


/** Get the pathDescriptor of the specified interface name.
  * only dev2 versions of these functions are needed right now.
  *
  * @param intfname     (IN) The interface name
  * @param layer2	    (IN) boolean to indicate whether intfname is a layer 2 or layer 3 interface name.
  * @param pathDesc    (OUT) the path descriptor to the object.
  *
  * @return CmsRet.
  */
CmsRet qdmIntf_getPathDescFromIntfnameLocked_dev2(const char *intfname,
                                     UBOOL8 layer2,
                                     MdmPathDescriptor *pathDesc);


/** Get the Status value of the object in fullPath.
  * only dev2 versions of these functions are needed right now.
  * This is used by ssk interface stack code to get the status value from
  * any object type.
  *
  * @param fullPath     (IN) the fullpath to the object.
  * @param statusBuf   (OUT) a buffer to hold the returned status value.
  * @param statusBufLen (IN) Length of statusBuf.
  *
  * @return CmsRet.
  */
CmsRet qdmIntf_getStatusFromFullPathLocked_dev2(const char *fullPath,
                                    char *statusBuf, UINT32 statusBufLen);


/** Get the Status value of the object pointed to by pathDescIn.
  * only dev2 versions of these functions are needed right now.
  * This is used by ssk interface stack code to get the status value from
  * any object type.
  *
  * @param pathDescIn   (IN) the path descriptor to the object.
  * @param statusBuf   (OUT) a buffer to hold the returned status value.
  * @param statusBufLen (IN) Length of statusBuf.
  *
  * @return CmsRet.
  */
CmsRet qdmIntf_getStatusFromPathDescLocked_dev2(const MdmPathDescriptor *pathDescIn,
                                    char *statusBuf, UINT32 statusBufLen);




/** Given a fullpath, return TRUE if status is UP.
  *
  * @param fullPath     (IN) the fullpath to the object.
  *
  * @return TRUE if fullpath is UP.
  */
UBOOL8 qdmIntf_isStatusUpOnFullPathLocked_dev2(const char *fullPath);



/** Get the LowerLayers param value of the object in fullPath.
  * LowerLayers is a TR181 param, so only the dev2 version of this function
  * is needed.
  *
  * @param fullPath     (IN) the fullpath to the object.
  * @param LowerLayersBuf (OUT) a buffer to hold the LowerLayers value.
  * @param llBufLen     (IN) Length of the lowerLayersBuf.
  *
  * @return CmsRet.
  */
CmsRet qdmIntf_getLowerLayersFromFullPathLocked_dev2(const char *fullPath,
                                    char *lowerLayersBuf, UINT32 llBufLen);


/** Get the LowerLayers param value of the object in fullPath.
  * LowerLayers is a TR181 param, so only the dev2 version of this function
  * is needed.
  *
  * @param pathDescIn   (IN) the path descriptor to the object.
  * @param LowerLayersBuf (OUT) a buffer to hold the LowerLayers value.
  * @param llBufLen     (IN) Length of the lowerLayersBuf.
  *
  * @return CmsRet.
  */
CmsRet qdmIntf_getLowerLayersFromPathDescLocked_dev2(const MdmPathDescriptor *pathDescIn,
                                    char *lowerLayersBuf, UINT32 llBufLen);


/** Get the First LowerLayers param value of the object in fullPath.
  * LowerLayers is a TR181 param, so only the dev2 version of this function
  * is needed.
  *
  * @param fullPath     (IN) the fullpath to the object.
  * @param LowerLayersBuf (OUT) a buffer to hold the First LowerLayers value.
  * @param llBufLen     (IN) Length of the lowerLayersBuf.
  *
  * @return CmsRet.
  */
CmsRet qdmIntf_getFirstLowerLayerFromFullPathLocked_dev2(const char *fullPath,
                                    char *lowerLayersBuf, UINT32 llBufLen);




/** Return TRUE if the given fullpath points to a Layer 2 interface.
 *  In this function, a layer 2 interface is one that has the layer 2
 *  interface name (ptm0, atm0, but not dsl0), and also the upstream param
 *  (wl0, wl0.1, but not the wifi radio object).
 *
 *  This function is only used in TR181 code, so only _dev2 version exists.
 *
 * @param fullPath  (IN) fullpath
 *
 * @return TRUE if the given fullpath points to a Layer 2 interface.
 */
UBOOL8 qdmIntf_isFullPathLayer2Locked_dev2(const char *fullPath);




/** Return TRUE if specified Layer 2 Linux interface name is a WAN (upstream)
 *  interface.  This function is particularly useful for eth interfaces
 *  because they could be WAN or LAN.  But it works for any layer 2 interface
 *  with an "Upstream" param.  If the specified interface does not have an
 *  Upstream param, FALSE is returned.
 *
 */
UBOOL8 qdmIntf_isLayer2IntfNameUpstreamLocked(const char *l2IntfName);

UBOOL8 qdmIntf_isLayer2IntfNameUpstreamLocked_igd(const char *l2IntfName);

UBOOL8 qdmIntf_isLayer2IntfNameUpstreamLocked_dev2(const char *l2IntfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIntf_isLayer2IntfNameUpstreamLocked(i)  qdmIntf_isLayer2IntfNameUpstreamLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIntf_isLayer2IntfNameUpstreamLocked(i)  qdmIntf_isLayer2IntfNameUpstreamLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIntf_isLayer2IntfNameUpstreamLocked(i)  qdmIntf_isLayer2IntfNameUpstreamLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIntf_isLayer2IntfNameUpstreamLocked(i)  (cmsMdm_isDataModelDevice2() ? \
                          qdmIntf_isLayer2IntfNameUpstreamLocked_dev2((i)) : \
                          qdmIntf_isLayer2IntfNameUpstreamLocked_igd((i)))
#endif


/** Return TRUE if the specified layer 2 fullPath is upstream.
 *  This is a helper function to qdmIntf_isLayer2IntfNameUpstreamLocked_dev2,
 *  but it can also be called directly.
 */
UBOOL8 qdmIntf_isLayer2FullPathUpstreamLocked_dev2(const char *l2FullPath);




/** This function get IfName from the bottom layer
 *
 * @param isPPP             (IN) is ppp connection or not
 * @param fullPath          (IN) full path of upper layer object
 * @param ifName           (OUT) ifName
 *
 * @return CmsRet enum.
 */
CmsRet qdmIntf_getIfNameFromBottomLayer(UBOOL8 isPPP, const char *fullPath, char *ifName);

/** This function get path descriptor from the bottom layer
 *
 * @param fullPath             (IN) full path of upper layer object
 * @param bottomLayerPathDesc (OUT) path descriptor of bottom layer
 * @param oid                  (IN) target object ID
 *
 * @return CmsRet enum.
 */
CmsRet qdmIntf_getBottomLayerPathDescLocked_dev2(const char *fullPath,
                          MdmPathDescriptor *bottomLayerPathDesc, MdmObjectId oid);


/* Get the InterfaceGroupName by matching the bridge key (e.g. X of brX)
 * @param brName         (IN)  InterfaceGroupName 
 * @param brKey          (OUT) bridge key
 *
 * @return CmsRet enum.
 */
CmsRet qdmIntf_getIntfKeyByGroupName(char *brName, UINT32 *brKey);
CmsRet qdmIntf_getIntfKeyByGroupName_igd(char *brName, UINT32 *brKey);
CmsRet qdmIntf_getIntfKeyByGroupName_dev2(char *brName, UINT32 *brKey);

/* Get the bridge key (e.g. X of brX) by matching the InterfaceGroupName. 
 * @param brKey          (IN)  bridge key
 * @param brName         (OUT) InterfaceGroupName 
 */
CmsRet qdmIntf_getIntfGroupNameByBrKey(char *brName, UINT32 brKey);
CmsRet qdmIntf_getIntfGroupNameByBrKey_igd(char *brName, UINT32 brKey);
CmsRet qdmIntf_getIntfGroupNameByBrKey_dev2(char *brName, UINT32 brKey);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIntf_getIntfGroupNameByBrKey(a, b)  qdmIntf_getIntfGroupNameByBrKey_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIntf_getIntfGroupNameByBrKey(a, b)  qdmIntf_getIntfGroupNameByBrKey_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIntf_getIntfGroupNameByBrKey(a, b)  qdmIntf_getIntfGroupNameByBrKey_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIntf_getIntfGroupNameByBrKey(a, b)  (cmsMdm_isDataModelDevice2() ? \
                  qdmIntf_getIntfGroupNameByBrKey_dev2((a), (b)) : \
                  qdmIntf_getIntfGroupNameByBrKey_igd((a), (b)))
#endif



UBOOL8 qdmIntf_isInterfaceWANOnly(const char *ifName);
UBOOL8 qdmIntf_isInterfaceWANOnly_igd(const char *ifName);
UBOOL8 qdmIntf_isInterfaceWANOnly_dev2(const char *ifName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIntf_isInterfaceWANOnly(i)    qdmIntf_isInterfaceWANOnly_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIntf_isInterfaceWANOnly(i)    qdmIntf_isInterfaceWANOnly_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIntf_isInterfaceWANOnly(i)    qdmIntf_isInterfaceWANOnly_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIntf_isInterfaceWANOnly(i)   (cmsMdm_isDataModelDevice2() ? \
                                   qdmIntf_isInterfaceWANOnly_dev2((i)) : \
                                   qdmIntf_isInterfaceWANOnly_igd((i)))
#endif



/** Get all LAN (layer 2) interface names in the given buffer (comma separated).
 *  @return number of interface names in buffer.
 */
UINT32 qdmIntf_getAllLanIntfNames(char *ifNamesBuf, UINT32 len);
UINT32 qdmIntf_getAllLanIntfNames_igd(char *ifNamesBuf, UINT32 len);
UINT32 qdmIntf_getAllLanIntfNames_dev2(char *ifNamesBuf, UINT32 len);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIntf_getAllLanIntfNames(b, l)  qdmIntf_getAllLanIntfNames_igd((b), (l))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIntf_getAllLanIntfNames(b, l)  qdmIntf_getAllLanIntfNames_igd((b), (l))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIntf_getAllLanIntfNames(b, l)  qdmIntf_getAllLanIntfNames_dev2((b), (l))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIntf_getAllLanIntfNames(b, l)   (cmsMdm_isDataModelDevice2() ? \
                                   qdmIntf_getAllLanIntfNames_dev2((b), (l)) : \
                                   qdmIntf_getAllLanIntfNames_igd((b), (l)))
#endif


/** Get all WAN (layer 2) interface names in the given buffer (comma separated).
 *  @return number of interface names in buffer.
 */
UINT32 qdmIntf_getAllWanL2IntfNames(char *ifNamesBuf, UINT32 len);
UINT32 qdmIntf_getAllWanL2IntfNames_igd(char *ifNamesBuf, UINT32 len);
UINT32 qdmIntf_getAllWanL2IntfNames_dev2(char *ifNamesBuf, UINT32 len);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIntf_getAllWanL2IntfNames(b, l)  qdmIntf_getAllWanL2IntfNames_igd((b), (l))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIntf_getAllWanL2IntfNames(b, l)  qdmIntf_getAllWanL2IntfNames_igd((b), (l))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIntf_getAllWanL2IntfNames(b, l)  qdmIntf_getAllWanL2IntfNames_dev2((b), (l))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIntf_getAllWanL2IntfNames(b, l)   (cmsMdm_isDataModelDevice2() ? \
                                   qdmIntf_getAllWanL2IntfNames_dev2((b), (l)) : \
                                   qdmIntf_getAllWanL2IntfNames_igd((b), (l)))
#endif


/** Get Bridge interface name in the given interface name.
 *  @return Bridge interface name in buffer.
 */

CmsRet qdmIntf_getBridgeNameByIntfName(const char *intfname, char *bridgeifcName);
CmsRet qdmIntf_getBridgeNameByIntfName_igd(const char *intfname, char *bridgeifcName);
CmsRet qdmIntf_getBridgeNameByIntfName_dev2(const char *intfname, char *bridgeifcName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmIntf_getBridgeNameByIntfName(b, l)  qdmIntf_getBridgeNameByIntfName_igd((b), (l))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmIntf_getBridgeNameByIntfName(b, l)  qdmIntf_getBridgeNameByIntfName_igd((b), (l))
#elif defined(SUPPORT_DM_PURE181)
#define qdmIntf_getBridgeNameByIntfName(b, l)  qdmIntf_getBridgeNameByIntfName_dev2((b), (l))
#elif defined(SUPPORT_DM_DETECT)
#define qdmIntf_getBridgeNameByIntfName(b, l)   (cmsMdm_isDataModelDevice2() ? \
                                   qdmIntf_getBridgeNameByIntfName_dev2((b), (l)) : \
                                   qdmIntf_getBridgeNameByIntfName_igd((b), (l)))
#endif


#endif /* _QDM_INTF_H_ */
