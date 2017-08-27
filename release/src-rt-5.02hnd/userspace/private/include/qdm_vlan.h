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

#ifndef __QDM_VLAN_H__
#define __QDM_VLAN_H__

#include "cms.h"



/** Return TRUE if a VLAN on LAN side is configured/present.
 *
 * @return TRUE if a VLAN on LAN side is configured/present.
 */
UBOOL8 qdmVlan_isLanVlanPresentLocked(void);
UBOOL8 qdmVlan_isLanVlanPresentLocked_igd(void);
UBOOL8 qdmVlan_isLanVlanPresentLocked_dev2(void);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmVlan_isLanVlanPresentLocked()   qdmVlan_isLanVlanPresentLocked_igd()
#elif defined(SUPPORT_DM_HYBRID)
#define qdmVlan_isLanVlanPresentLocked()   qdmVlan_isLanVlanPresentLocked_igd()
#elif defined(SUPPORT_DM_PURE181)
#define qdmVlan_isLanVlanPresentLocked()   qdmVlan_isLanVlanPresentLocked_dev2()
#elif defined(SUPPORT_DM_DETECT)
#define qdmVlan_isLanVlanPresentLocked()   (cmsMdm_isDataModelDevice2() ? \
                                  qdmVlan_isLanVlanPresentLocked_dev2() : \
                                  qdmVlan_isLanVlanPresentLocked_igd())
#endif




/** Return the VLANMUXID of the given WAN intf name.  Currently only
 *  supports WAN intf name, but maybe it can be expanded to cover LAN
 *  intf names as well.
 *
 * @param intfName  (IN) WAN intfName
 *
 * @return -1 if no VLAN is configured on this intf, or if there was an error
 *         Otherwise, return 1-4095
 */
SINT32 qdmVlan_getVlanIdByIntfNameLocked(const char *intfName);
SINT32 qdmVlan_getVlanIdByIntfNameLocked_igd(const char *intfName);
SINT32 qdmVlan_getVlanIdByIntfNameLocked_dev2(const char *intfName);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmVlan_getVlanIdByIntfNameLocked(i)  qdmVlan_getVlanIdByIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmVlan_getVlanIdByIntfNameLocked(i)  qdmVlan_getVlanIdByIntfNameLocked_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define qdmVlan_getVlanIdByIntfNameLocked(i)  qdmVlan_getVlanIdByIntfNameLocked_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define qdmVlan_getVlanIdByIntfNameLocked(i)  (cmsMdm_isDataModelDevice2() ? \
                          qdmVlan_getVlanIdByIntfNameLocked_dev2((i)) : \
                          qdmVlan_getVlanIdByIntfNameLocked_igd((i)))
#endif




/** TR181 function
 * Return the VLANID, vlan801p, vlanTPID of the given WAN intf name.  Currently only
 *  supports WAN intf name, but maybe it can be expanded to cover LAN
 *  intf names as well.
 *
 * @param intfName  (IN) WAN intfName
 *        vlanId    (OUT) VLANID
 *        vlan801p  (OUT) Vlan801p
 *        vlanTPID  (OUT) vlanTPID
 */
CmsRet qdmVlan_getVlanTermInfoByIntfNameLocked_dev2(const char *intfName,
                                                    SINT32 *vlanId,
                                                    SINT32 *vlan801p,
                                                    UINT32 *vlanTPID);

SINT32 qdmVlan_getVlan801pByIntfNameLocked_dev2(const char *intfName);

UINT32 qdmVlan_getVlanTPIDByIntfNameLocked_dev2(const char *intfName);                                                    

#endif  /* __QDM_VLAN_H__ */


