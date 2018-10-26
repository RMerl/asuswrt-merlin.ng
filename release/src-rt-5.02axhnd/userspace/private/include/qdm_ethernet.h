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
#include "cms_phl.h"
#include "cms_qdm.h"
#include "cms_util.h"


/*
 * qdmEthlin_getEthlinkLowerLayerFullPathByName functions
 *  
 */
CmsRet qdmEthLink_getEthLinkLowerLayerFullPathByName_igd(const char * ethLinkName, char * lowerLayerBuf, UINT32 bufLen);
CmsRet qdmEthLink_getEthLinkLowerLayerFullPathByName_dev2(const char * ethLinkName, char * lowerLayerBuf, UINT32 bufLen);
#if defined(SUPPORT_DM_LEGACY98)
#define qdmEthLink_getEthLinkLowerLayerFullPathByName(n, l, s)  !ERROR: this function should not be used in this mode
#elif defined(SUPPORT_DM_HYBRID)
#define qdmEthLink_getEthLinkLowerLayerFullPathByName(n, l, s)  qdmEthLink_getEthLinkLowerLayerFullPathByName_igd((n), (l), (s))
#elif defined(SUPPORT_DM_PURE181)
#define qdmEthLink_getEthLinkLowerLayerFullPathByName(n, l, s)  qdmEthLink_getEthLinkLowerLayerFullPathByName_dev2((n), (l), (s))
#elif defined(SUPPORT_DM_DETECT)
#define qdmEthLink_getEthLinkLowerLayerFullPathByName(n, l, s)  (cmsMdm_isDataModelDevice2() ? \
                                       qdmEthLink_getEthLinkLowerLayerFullPathByName_dev2((n), (l), (s)) : \
                                       qdmEthLink_getEthLinkLowerLayerFullPathByName_igd((n), (l), (s)))
#endif

/** Get Ethernet interface info of the specified ifname.
 *
 * @param ifname (IN) The interface name to query.
 * @param status (OUT) Operational state of the interface
 * @param upstream (OUT) FALSE (LAN) TRUE (WAN)
 * @param shapingRate (OUT) Port shaping rate
 * @param shapingBurstSize (OUT) Port shaping burst size
 *
 * @return CmsRet CMSRET_SUCCESS if the given ifname is found.
 *                Other return codes indicate real errors.
 */
CmsRet qdmEth_getIntfInfoByNameLocked(const char   *ifname,
                                      char         *status,
                                      UBOOL8       *upstream,
                                      SINT32       *shapingRate,
                                      UINT32       *shapingBurstSize);
                                      
CmsRet qdmEth_getIntfInfoByNameLocked_igd(const char  *ifname,
                                          char        *status,
                                          UBOOL8      *upstream,
                                          SINT32      *shapingRate,
                                          UINT32      *shapingBurstSize);
                                          
CmsRet qdmEth_getIntfInfoByNameLocked_dev2(const char *ifname,
                                           char       *status,
                                           UBOOL8     *upstream,
                                           SINT32     *shapingRate,
                                           UINT32     *shapingBurstSize);

                                              
#if defined(SUPPORT_DM_LEGACY98)
#define qdmEth_getIntfInfoByNameLocked(i,o1,o2,o3,o4) qdmEth_getIntfInfoByNameLocked_igd((i),(o1),(o2),(o3),(o4))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmEth_getIntfInfoByNameLocked(i,o1,o2,o3,o4) qdmEth_getIntfInfoByNameLocked_igd((i),(o1),(o2),(o3),(o4))
#elif defined(SUPPORT_DM_PURE181)
#define qdmEth_getIntfInfoByNameLocked(i,o1,o2,o3,o4) qdmEth_getIntfInfoByNameLocked_dev2((i),(o1),(o2),(o3),(o4))
#elif defined(SUPPORT_DM_DETECT)
#define qdmEth_getIntfInfoByNameLocked(i,o1,o2,o3,o4) (cmsMdm_isDataModelDevice2() ? \
                             qdmEth_getIntfInfoByNameLocked_dev2((i),(o1),(o2),(o3),(o4)) : \
                             qdmEth_getIntfInfoByNameLocked_igd((i),(o1),(o2),(o3),(o4)))
#endif
                                              
/** Get Ethernet interface info of the specified ifname.
 *
 * @param ifname (IN) The interface name to query.
 * @param status (OUT) Operational state of the interface
 *
 * @return CmsRet CMSRET_SUCCESS if there is any LAN int.
 *                Other return codes indicate real errors.
 */
CmsRet qdmEth_getAllL2EthIntfNameLocked(char    *buffer,
                                        UINT32  bufSize);
                                      
CmsRet qdmEth_getAllL2EthIntfNameLocked_igd(char    *buffer,
                                            UINT32  bufSize);
                                          
CmsRet qdmEth_getAllL2EthIntfNameLocked_dev2(char    *buffer,
                                             UINT32  bufSize);

#if defined(SUPPORT_DM_LEGACY98)
#define qdmEth_getAllL2EthIntfNameLocked(i,j) qdmEth_getAllL2EthIntfNameLocked_igd((i),(j))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmEth_getAllL2EthIntfNameLocked(i,j) qdmEth_getAllL2EthIntfNameLocked_igd((i),(j))
#elif defined(SUPPORT_DM_PURE181)
#define qdmEth_getAllL2EthIntfNameLocked(i,j) qdmEth_getAllL2EthIntfNameLocked_dev2((i),(j))
#elif defined(SUPPORT_DM_DETECT)
#define qdmEth_getAllL2EthIntfNameLocked(i,j) (cmsMdm_isDataModelDevice2() ? \
                             qdmEth_getAllL2EthIntfNameLocked_dev2((i),(j)) : \
                             qdmEth_getAllL2EthIntfNameLocked_igd((i),(j)))
#endif


#ifdef DMP_DEVICE2_VLANTERMINATION_1

/** Get Ethernet vlan termination by matching vlan id and lowerlayer
 * and returning vlan termination fullpath reference.
 *
 * @param vlanId (IN) The vlan id of the object
 * @param lowerLayer (IN) ethernet.link fullPath
 * @param myPathRef (OUT) Full path of this vlan termination object
 * @param bufLen (IN) buffer length for myPathRef
 *
 * @return CmsRet CMSRET_SUCCESS if the given ifname is found.
 *                Other return codes indicate real errors.
 */

CmsRet qdmEth_getEthernetVlanTermination_dev2(SINT32 vlanId,
                                              const char *lowerLayer,
                                              char *myPathRef,
                                              UINT32 bufLen);

#endif /* DMP_DEVICE2_VLANTERMINATION_1 */
                                              
