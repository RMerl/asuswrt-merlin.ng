/***********************************************************************
 *
 *  Copyright (c) 2009  Broadcom Corporation
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

#ifndef __DAL_DSL_H__
#define __DAL_DSL_H__


/*!\file dal_dsl.h
 * \brief Header file for the DSL portion of the Data Aggregation Layer (DAL) API.
 * This API is used by httpd, cli, and snmp and presents an API similar to the
 * old dbAPI.  A lot of code in http, cli, and snmp rely on functions that are
 * more in the old dbAPI style than the new object layer API.
 * 
 * This API is implemented in the cms_dal.so library.
 */


/** find the dslLinkCfg object with the given layer 2 ifName. Uses rut function
 *
 * @param ifName (IN) layer 2 ifName of DslLinkCfg to find.
 * @param iidStack (OUT) iidStack of the DslLinkCfg object found.
 * @param dslLinkCfg (OUT) if not null, this will contain a pointer to the found
 *                         dslLinkCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired DslLinkCfg object was found.
 */
UBOOL8 dalDsl_getDslLinkByIfName(char *ifName, InstanceIdStack *iidStack, WanDslLinkCfgObject **dslLinkCfg);


/** find the ptmLinkCfg object with the given layer 2 ifName. Uses rut function
 *
 * @param ifName (IN) layer 2 ifName of ptmLinkCfg to find.
 * @param iidStack (OUT) iidStack of the ptmLinkCfg object found.
 * @param dslLinkCfg (OUT) if not null, this will contain a pointer to the found
 *                         ptmLinkCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired ptmLinkCfg object was found.
 */
UBOOL8 dalDsl_getPtmLinkByIfName(char *ifName, InstanceIdStack *iidStack, WanPtmLinkCfgObject **ptmLinkCfg);


/** Get the DSL InterfaceConfig object corresponding to the PTM line.
 *
 * @param iidStack (OUT) The iidStack of the requested object.
 * @param wanDslIntfObj (OUT) The requested object.
 *
 * @return CmsRet enum.
 */
CmsRet dalDsl_getPtmDslIntfObject(InstanceIdStack *iidStack,
                               WanDslIntfCfgObject **wanDslIntfObj);

/** Get the DSL InterfaceConfig object corresponding to the ATM line.
 *
 * @param iidStack (OUT) The iidStack of the requested object.
 * @param wanDslIntfObj (OUT) The requested object.
 *
 * @return CmsRet enum.
 */
CmsRet dalDsl_getAtmDslIntfObject(InstanceIdStack *iidStack,
                               WanDslIntfCfgObject **wanDslIntfObj);


/** Return TRUE if DSL bonding is enabled.
 *
 * @return TRUE if DSL bonding is enabled.
 */
UBOOL8 dalDsl_isDslBondingEnabled(void);


/** Get the DSL InterfaceConfig object corresponding to the Bonding PTM line.
 *
 * @param iidStack (OUT) The iidStack of the requested object.
 * @param wanDslIntfObj (OUT) The requested object.
 *
 * @return CmsRet enum.
 */
CmsRet dalDsl_getBondingPtmDslIntfObject(InstanceIdStack *iidStack, WanDslIntfCfgObject **wanDslIntfObj);


/** Get the DSL InterfaceConfig object corresponding to the Bonding ATM line.
 *
 * @param iidStack (OUT) The iidStack of the requested object.
 * @param wanDslIntfObj (OUT) The requested object.
 *
 * @return CmsRet enum.
 */
CmsRet dalDsl_getBondingAtmDslIntfObject(InstanceIdStack *iidStack, WanDslIntfCfgObject **wanDslIntfObj);


CmsRet dalDsl_setDslBonding(int enable);
CmsRet dalDsl_setDslBonding_igd(int enable);
CmsRet dalDsl_setDslBonding_dev2(int enable);
#if defined(SUPPORT_DM_LEGACY98)
#define dalDsl_setDslBonding(w)       dalDsl_setDslBonding_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDsl_setDslBonding(w)       dalDsl_setDslBonding_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDsl_setDslBonding(w)       dalDsl_setDslBonding_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDsl_setDslBonding(w)       (cmsMdm_isDataModelDevice2() ? \
                                       dalDsl_setDslBonding_dev2((w)) : \
                                       dalDsl_setDslBonding_igd((w)))
#endif

CmsRet dalDsl_getDslBonding(int *enable);
CmsRet dalDsl_getDslBonding_igd(int *enable);
CmsRet dalDsl_getDslBonding_dev2(int *enable);
#if defined(SUPPORT_DM_LEGACY98)
#define dalDsl_getDslBonding(w)       dalDsl_getDslBonding_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalDsl_getDslBonding(w)       dalDsl_getDslBonding_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalDsl_getDslBonding(w)       dalDsl_getDslBonding_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalDsl_getDslBonding(w)       (cmsMdm_isDataModelDevice2() ? \
                                       dalDsl_getDslBonding_dev2((w)) : \
                                       dalDsl_getDslBonding_igd((w)))
#endif

#endif /* __DAL_DSL_H__ */
