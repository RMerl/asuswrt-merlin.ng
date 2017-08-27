/***********************************************************************
 *
 *  Copyright (c) 2012  Broadcom Corporation
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
#include "cms.h"
#include "cms_mdm.h"
#include "cms_obj.h"
#include "cms_qdm.h"
#include "cms_util.h"

#ifndef _QDM_XTM_H_
#define _QDM_XTM_H_


/*!\file qdm_xtm.h
 * \brief This file contains declarations for XTM Interface related functions.
 *
 */

/** Get ATM interface info of the specified ifname.
 *
 *  @param ifName (IN) The interface name to query.
 *  @param vpi (OUT) VPI value
 *  @param vci (OUT) VCI value
 *  @param interfaceId (OUT) Interface ID
 *
 *  @return CmsRet CMSRET_SUCCESS if the given ifname is found.
 *                Other return codes indicate real errors.
 */
CmsRet qdmXtm_getAtmIntfInfoByNameLocked(char *ifName, 
                                         UINT16 *vpi,
                                         UINT16 *vci,
                                         UINT32 *interfaceId);
                                              
CmsRet qdmXtm_getAtmIntfInfoByNameLocked_igd(char *ifName, 
                                         UINT16 *vpi,
                                         UINT16 *vci,
                                         UINT32 *interfaceId);
                                              
CmsRet qdmXtm_getAtmIntfInfoByNameLocked_dev2(char *ifName, 
                                         UINT16 *vpi,
                                         UINT16 *vci,
                                         UINT32 *interfaceId);
                                              
#if defined(SUPPORT_DM_LEGACY98)
#define qdmXtm_getAtmIntfInfoByNameLocked(i,o1,o2,o3) qdmXtm_getAtmIntfInfoByNameLocked_igd((i),(o1),(o2),(o3))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmXtm_getAtmIntfInfoByNameLocked(i,o1,o2,o3) qdmXtm_getAtmIntfInfoByNameLocked_igd((i),(o1),(o2),(o3))
#elif defined(SUPPORT_DM_PURE181)
#define qdmXtm_getAtmIntfInfoByNameLocked(i,o1,o2,o3) qdmXtm_getAtmIntfInfoByNameLocked_dev2((i),(o1),(o2),(o3))
#elif defined(SUPPORT_DM_DETECT)
#define qdmXtm_getAtmIntfInfoByNameLocked(i,o1,o2,o3) (cmsMdm_isDataModelDevice2() ? \
                             qdmXtm_getAtmIntfInfoByNameLocked_dev2((i),(o1),(o2),(o3)) : \
                             qdmXtm_getAtmIntfInfoByNameLocked_igd((i),(o1),(o2),(o3)))
#endif


/** Get PTM interface info of the specified ifname.
 *
 *  @param ifName (IN) The interface name to query.
 *  @param ptmPortId (OUT) PTM port ID
 *  @param ptmPriorityLow (OUT) Low PTM priority boolean
 *  @param ptmPriorityHigh (OUT) High PTM priority boolean
 *
 *  @return CmsRet CMSRET_SUCCESS if the given ifname is found.
 *                Other return codes indicate real errors.
 */
CmsRet qdmXtm_getPtmIntfInfoByNameLocked(char *ifName, 
                                         UINT32 *ptmPortId,
                                         UBOOL8 *ptmPriorityLow,
                                         UBOOL8 *ptmPriorityHigh);
                                              
CmsRet qdmXtm_getPtmIntfInfoByNameLocked_igd(char *ifName, 
                                             UINT32 *ptmPortId,
                                             UBOOL8 *ptmPriorityLow,
                                             UBOOL8 *ptmPriorityHigh);
                                              
CmsRet qdmXtm_getPtmIntfInfoByNameLocked_dev2(char *ifName, 
                                              UINT32 *ptmPortId,
                                              UBOOL8 *ptmPriorityLow,
                                              UBOOL8 *ptmPriorityHigh);
                                              
#if defined(SUPPORT_DM_LEGACY98)
#define qdmXtm_getPtmIntfInfoByNameLocked(i,o1,o2,o3) qdmXtm_getPtmIntfInfoByNameLocked_igd((i),(o1),(o2),(o3))
#elif defined(SUPPORT_DM_HYBRID)
#define qdmXtm_getPtmIntfInfoByNameLocked(i,o1,o2,o3) qdmXtm_getPtmIntfInfoByNameLocked_igd((i),(o1),(o2),(o3))
#elif defined(SUPPORT_DM_PURE181)
#define qdmXtm_getPtmIntfInfoByNameLocked(i,o1,o2,o3) qdmXtm_getPtmIntfInfoByNameLocked_dev2((i),(o1),(o2),(o3))
#elif defined(SUPPORT_DM_DETECT)
#define qdmXtm_getPtmIntfInfoByNameLocked(i,o1,o2,o3) (cmsMdm_isDataModelDevice2() ? \
                             qdmXtm_getPtmIntfInfoByNameLocked_dev2((i),(o1),(o2),(o3)) : \
                             qdmXtm_getPtmIntfInfoByNameLocked_igd((i),(o1),(o2),(o3)))
#endif


#endif /* _QDM_XTM_H_ */

