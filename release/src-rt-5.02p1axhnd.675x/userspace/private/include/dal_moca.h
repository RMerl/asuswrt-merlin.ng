/*
* <:copyright-BRCM:2006:proprietary:standard
* 
*    Copyright (c) 2006 Broadcom 
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
:>
*/

#ifndef __DAL_MOCA_H__
#define __DAL_MOCA_H__


/*!\file dal_dns.h
 * \brief Header file for the Moca functions of the CMS DAL API.
 *  This is in the cms_dal library.
 */

#include "cms.h"




/** Get a list of all Moca interfaces (LAN and WAN).
 *
 * @param ifNameListBuf  (OUT) On return, will contain list of moca intf names
 *                             separated by comma, e.g. moca0,moca1.  Caller
 *                             must provide a buffer.
 * @param bufLen          (IN) Length of buffer.
 */
void dalMoca_getIntfNameList(char *ifNameListBuf, UINT32 bufLen);
void dalMoca_getIntfNameList_igd(char *ifNameListBuf, UINT32 bufLen);
void dalMoca_getIntfNameList_dev2(char *ifNameListBuf, UINT32 bufLen);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_getIntfNameList(v, a)  dalMoca_getIntfNameList_igd((v), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_getIntfNameList(v, a)  dalMoca_getIntfNameList_igd((v), (a))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_getIntfNameList(v, a)  dalMoca_getIntfNameList_dev2((v), (a))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_getIntfNameList(v, a)  (cmsMdm_isDataModelDevice2() ? \
                                   dalMoca_getIntfNameList_dev2((v), (a)) : \
                                   dalMoca_getIntfNameList_igd((v), (a)))
#endif




/** Get the moca interface object associated with ifName.
 *
 * This function will return the TR98 LAN Moca, TR98 WAN Moca, or
 * TR181 Moca interface object.
 *
 * @param ifName (IN) pointer MoCA interface name string
 * @param iidStack  (OUT) iidStack of the mocaIntf object found.
 * @param obj       (OUT) if not null, this will contain a pointer to the found
 *                         Moca object.  Caller is responsible for
 *                         calling cmsObj_free() on this object.
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_getMocaObjByIntfName(const char *ifName,
                                    InstanceIdStack *iidStack, void **obj);
CmsRet dalMoca_getMocaObjByIntfName_igd(const char *ifName,
                                    InstanceIdStack *iidStack, void **obj);
CmsRet dalMoca_getMocaObjByIntfName_dev2(const char *ifName,
                                    InstanceIdStack *iidStack, void **obj);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_getMocaObjByIntfName(v, a, j)  dalMoca_getMocaObjByIntfName_igd((v), (a), (j))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_getMocaObjByIntfName(v, a, j)  dalMoca_getMocaObjByIntfName_igd((v), (a), (j))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_getMocaObjByIntfName(v, a, j)  dalMoca_getMocaObjByIntfName_dev2((v), (a), (j))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_getMocaObjByIntfName(v, a, j)  (cmsMdm_isDataModelDevice2() ? \
                    dalMoca_getMocaObjByIntfName_dev2((v), (a), (j)) : \
                    dalMoca_getMocaObjByIntfName_igd((v), (a), (j)))
#endif




/* Get TR98 or TR181 WanMocaIntfObject by interface name
 *
 * @param ifname     (IN)  The WAN moca interface name.
 * @param iidStack  (OUT) iidStack of the WAN mocaIntf object found.
 * @param mocaIntfCfg (OUT) if not null, this will contain a pointer to the found
 *                         mocaIntfCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired mocaIntfCfg object was found.
 */
UBOOL8 dalMoca_getWanMocaIntfByIfName(const char *ifName,
                                      InstanceIdStack *iidStack,
                                      void **obj);

UBOOL8 dalMoca_getWanMocaIntfByIfName_igd(const char *ifName,
                                      InstanceIdStack *iidStack,
                                      void **obj);

UBOOL8 dalMoca_getWanMocaIntfByIfName_dev2(const char *ifName,
                                      InstanceIdStack *iidStack,
                                      void **obj);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_getWanMocaIntfByIfName(v, a, j)  dalMoca_getWanMocaIntfByIfName_igd((v), (a), (j))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_getWanMocaIntfByIfName(v, a, j)  dalMoca_getWanMocaIntfByIfName_igd((v), (a), (j))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_getWanMocaIntfByIfName(v, a, j)  dalMoca_getWanMocaIntfByIfName_dev2((v), (a), (j))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_getWanMocaIntfByIfName(v, a, j)  (cmsMdm_isDataModelDevice2() ? \
                    dalMoca_getWanMocaIntfByIfName_dev2((v), (a), (j)) : \
                    dalMoca_getWanMocaIntfByIfName_igd((v), (a), (j)))
#endif




/** Get Lan Moca interface by name.
 *
 * @param ifname     (IN)  The LAN moca interface name.
 * @param iidStack  (OUT) iidStack of the WAN mocaIntf object found.
 * @param mocaIntfCfg (OUT) if not null, this will contain a pointer to the found
 *                         mocaIntfCfg object.  Caller is responsible for calling
 *                         cmsObj_free() on this object.
 *
 * @return UBOOL8 indicating whether the desired mocaIntfCfg object was found.
 */
UBOOL8 dalMoca_getLanMocaIntfByIfName(const char *ifName,
                                      InstanceIdStack *iidStack,
                                      void **obj);

UBOOL8 dalMoca_getLanMocaIntfByIfName_igd(const char *ifName,
                                          InstanceIdStack *iidStack,
                                          void **obj);

UBOOL8 dalMoca_getLanMocaIntfByIfName_dev2(const char *ifName,
                                           InstanceIdStack *iidStack,
                                           void **obj);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_getLanMocaIntfByIfName(v, a, j)  dalMoca_getLanMocaIntfByIfName_igd((v), (a), (j))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_getLanMocaIntfByIfName(v, a, j)  dalMoca_getLanMocaIntfByIfName_igd((v), (a), (j))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_getLanMocaIntfByIfName(v, a, j)  dalMoca_getLanMocaIntfByIfName_dev2((v), (a), (j))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_getLanMocaIntfByIfName(v, a, j)  (cmsMdm_isDataModelDevice2() ? \
                    dalMoca_getLanMocaIntfByIfName_dev2((v), (a), (j)) : \
                    dalMoca_getLanMocaIntfByIfName_igd((v), (a), (j)))
#endif




/** Start the MoCA Core if it's currently stopped
 *
 * @param ifName (IN) pointer MoCA interface name string
 * @param newObj (IN) pointer to MoCA object with new init parameters
 * @param initMask (IN) bitmask of fields to set from newObj
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_start(const char *ifName, UINT64 initMask,
                     UBOOL8 autoNwSearch, UBOOL8 privacy,
                     UINT32 lastOperationalFrequency,
                     const char *password, const char *initParmsString);
CmsRet dalMoca_start_igd(const char *ifName, UINT64 initMask,
                     UBOOL8 autoNwSearch, UBOOL8 privacy,
                     UINT32 lastOperationalFrequency,
                     const char *password, const char *initParmsString);
CmsRet dalMoca_start_dev2(const char *ifName, UINT64 initMask,
                     UBOOL8 autoNwSearch, UBOOL8 privacy,
                     UINT32 lastOperationalFrequency,
                     const char *password, const char *initParmsString);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_start(i, m, a, p, f, w, s)  dalMoca_start_igd((i), (m), (a), (p), (f), (w), (s))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_start(i, m, a, p, f, w, s)  dalMoca_start_igd((i), (m), (a), (p), (f), (w), (s))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_start(i, m, a, p, f, w, s)  dalMoca_start_dev2((i), (m), (a), (p), (f), (w), (s))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_start(i, m, a, p, f, w, s)  (cmsMdm_isDataModelDevice2() ? \
                    dalMoca_start_dev2((i), (m), (a), (p), (f), (w), (s)) : \
                    dalMoca_start_igd((i), (m), (a), (p), (f), (w), (s)))
#endif




/** Restart the MoCA Core
 *
 * @param ifName (IN) pointer MoCA interface name string
 * @param newObj (IN) pointer to MoCA object with new init parameters
 * @param reinitMask (IN) bitmask of fields to set from newObj
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_restart(const char *ifName, UINT64 reinitMask,
                       UBOOL8 autoNwSearch, UBOOL8 privacy,
                       UINT32 lastOperationalFrequency,
                       const char *password, const char *initParmsString);
CmsRet dalMoca_restart_igd(const char *ifName, UINT64 reinitMask,
                       UBOOL8 autoNwSearch, UBOOL8 privacy,
                       UINT32 lastOperationalFrequency,
                       const char *password, const char *initParmsString);
CmsRet dalMoca_restart_dev2(const char *ifName, UINT64 reinitMask,
                       UBOOL8 autoNwSearch, UBOOL8 privacy,
                       UINT32 lastOperationalFrequency,
                       const char *password, const char *initParmsString);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_restart(i, m, a, p, f, w, s)  dalMoca_restart_igd((i), (m), (a), (p), (f), (w), (s))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_restart(i, m, a, p, f, w, s)  dalMoca_restart_igd((i), (m), (a), (p), (f), (w), (s))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_restart(i, m, a, p, f, w, s)  dalMoca_restart_dev2((i), (m), (a), (p), (f), (w), (s))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_restart(i, m, a, p, f, w, s)  (cmsMdm_isDataModelDevice2() ? \
                    dalMoca_restart_dev2((i), (m), (a), (p), (f), (w), (s)) : \
                    dalMoca_restart_igd((i), (m), (a), (p), (f), (w), (s)))
#endif




/** Stop the MoCA Core if it's currently started
 *
 * @param ifName (IN) pointer MoCA interface name string
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_stop(const char *ifName);
CmsRet dalMoca_stop_igd(const char *ifName);
CmsRet dalMoca_stop_dev2(const char *ifName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_stop(v)               dalMoca_stop_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_stop(v)               dalMoca_stop_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_stop(v)               dalMoca_stop_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_stop(v)  (cmsMdm_isDataModelDevice2() ? \
                                      dalMoca_stop_dev2((v)) : \
                                      dalMoca_stop_igd((v)))
#endif




/** Set moca configuration parameters from the httpd application
 *
 * @param webVar (IN) pointer to global web data structure
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_setWebParams(const WEB_NTWK_VAR * webVar );




/** Get current moca configuration parameters and store it in the webVar
 *  (for the httpd application)
 *
 * @param webVar (IN/OUT) pointer to global web data structure
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_getCurrentCfg(WEB_NTWK_VAR * webVar);
CmsRet dalMoca_getCurrentCfg_igd(WEB_NTWK_VAR * webVar);
CmsRet dalMoca_getCurrentCfg_dev2(WEB_NTWK_VAR * webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_getCurrentCfg(v)     dalMoca_getCurrentCfg_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_getCurrentCfg(v)     dalMoca_getCurrentCfg_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_getCurrentCfg(v)     dalMoca_getCurrentCfg_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_getCurrentCfg(v)  (cmsMdm_isDataModelDevice2() ? \
                                     dalMoca_getCurrentCfg_dev2((v)) : \
                                     dalMoca_getCurrentCfg_igd((v)))
#endif




/** Get some basic info about the Moca WAN interface (assume there is only
 *  one Moca WAN interface.)
 */
CmsRet dalMoca_getWanIntfInfo(char *intfNameBuf, char *connModeBuf);
CmsRet dalMoca_getWanIntfInfo_igd(char *intfNameBuf, char *connModeBuf);
CmsRet dalMoca_getWanIntfInfo_dev2(char *intfNameBuf, char *connModeBuf);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_getWanIntfInfo(i, v)  dalMoca_getWanIntfInfo_igd((i), (v))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_getWanIntfInfo(i, v)  dalMoca_getWanIntfInfo_igd((i), (v))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_getWanIntfInfo(i, v)  dalMoca_getWanIntfInfo_dev2((i), (v))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_getWanIntfInfo(i, v)  (cmsMdm_isDataModelDevice2() ? \
                    dalMoca_getWanIntfInfo_dev2((i), (v)) : \
                    dalMoca_getWanIntfInfo_igd((i), (v)))
#endif




/** Set Moca Version info in the given buffer
 *
 */
void dalMoca_setVersionString(const char *intfName, char *versionBuf, UINT32 bufLen);
void dalMoca_setVersionString_igd(const char *intfName, char *versionBuf, UINT32 bufLen);
void dalMoca_setVersionString_dev2(const char *intfName, char *versionBuf, UINT32 bufLen);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_setVersionString(v, a, j)  dalMoca_setVersionString_igd((v), (a), (j))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_setVersionString(v, a, j)  dalMoca_setVersionString_igd((v), (a), (j))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_setVersionString(v, a, j)  dalMoca_setVersionString_dev2((v), (a), (j))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_setVersionString(v, a, j)  (cmsMdm_isDataModelDevice2() ? \
                    dalMoca_setVersionString_dev2((v), (a), (j)) : \
                    dalMoca_setVersionString_igd((v), (a), (j)))
#endif




/** Add the WanMocaIntfObject object (for moving Moca Intf from LAN to WAN)
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_addMocaInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalMoca_addMocaInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalMoca_addMocaInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_addMocaInterface(v)     dalMoca_addMocaInterface_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_addMocaInterface(v)     dalMoca_addMocaInterface_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_addMocaInterface(v)     dalMoca_addMocaInterface_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_addMocaInterface(v)  (cmsMdm_isDataModelDevice2() ? \
                                     dalMoca_addMocaInterface_dev2((v)) : \
                                     dalMoca_addMocaInterface_igd((v)))
#endif



/** Delete the WanMocaIntfObject object (for moving Moca Intf from WAN to LAN)
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_deleteMocaInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalMoca_deleteMocaInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalMoca_deleteMocaInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalMoca_deleteMocaInterface(v)   dalMoca_deleteMocaInterface_igd((v))
#elif defined(SUPPORT_DM_HYBRID)
#define dalMoca_deleteMocaInterface(v)   dalMoca_deleteMocaInterface_igd((v))
#elif defined(SUPPORT_DM_PURE181)
#define dalMoca_deleteMocaInterface(v)   dalMoca_deleteMocaInterface_dev2((v))
#elif defined(SUPPORT_DM_DETECT)
#define dalMoca_deleteMocaInterface(v)  (cmsMdm_isDataModelDevice2() ? \
                                     dalMoca_deleteMocaInterface_dev2((v)) : \
                                     dalMoca_deleteMocaInterface_igd((v)))
#endif




/** Enable MoCA interface
 *
 * @param ifName (IN) pointer MoCA interface name string
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_enable ( char * ifName );


/** Disable MoCA interface
 *
 * @param ifName (IN) pointer MoCA interface name string
 * @param newObj (IN) pointer to MoCA object with new init parameters
 * @param reinitMask (IN) bitmask of fields to set from newObj
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_disable ( char * ifName );


/** Configure MoCA tracing level.
 *
 * @param ifName (IN) pointer MoCA interface name string
 * @param newObj (IN) pointer to MoCA object with new parameters
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_setTrace ( char * ifName, LanMocaIntfObject * newObj );



/** Configure MoCA operating parameters
 *
 * @param ifName (IN) pointer MoCA interface name string
 * @param newObj (IN) pointer to MoCA object with new parameters
 *
 * @return CmsRet enum.
 */
CmsRet dalMoca_setConfig ( char * ifName, LanMocaIntfObject * newObj );



#endif  /* __DAL_MOCA_H__ */
