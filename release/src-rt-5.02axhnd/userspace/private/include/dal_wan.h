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

#ifndef __DAL_WAN_H__
#define __DAL_WAN_H__


/*!\file dal_wan.h
 * \brief Header file for the WAN portion of the CMS Data Abstration Layer API.
 *  This is in the cms_dal library.
 */



/** This function finds out if the wan layer 2 link is up or for the WanIP/PPPConnObject.
 *
 * @param wanConnOid (IN) oid is either MDMOID_WAN_PPP_CONN or MDMOID_WAN_IP_CONN
 * @param iidStack   (IN) iidStack of the WanIP/PPPConnObject.  This iidStack is
 *                        used to find the ancestor WanLinkCfg object *
 * @return UBOOL8
 */
UBOOL8 dalWan_isWanLayer2LinkUp(MdmObjectId wanConnOid, const InstanceIdStack *iidStack);


/** This function finds out if the wan service is up for the WanIP/PPPConnObject.
 *
 * @param ifName  (IN) WAN interface name
 * @param isIPv4  (IN) service type, either IPv4 or IPv6
 * @return UBOOL8
 */
UBOOL8 dalWan_isWanLayer3ServiceUp(const char *ifName, UBOOL8 isIPv4);


/** Get the (layer 2) Wan Ethernet InterfaceConfig object.
 *
 * @param iidStack (OUT) The iidStack of the requested object.
 * @param wanEthIntfObj (OUT) The requested object.
 *
 * @return CmsRet enum.
 */
CmsRet dalWan_getWanEthObject(InstanceIdStack *iidStack,
                              WanEthIntfObject **wanEthIntfObj);


/** Get the (layer 2) Wan Moca InterfaceConfig object.
 *
 * @param iidStack (OUT) The iidStack of the requested object.
 * @param wanMocaIntfObj (OUT) The requested object.
 *
 * @return CmsRet enum.
 */
CmsRet dalWan_getWanMocaObject(InstanceIdStack *iidStack,
                               WanMocaIntfObject **wanMocaIntfObj);


/** Get the WAN L2tpAc InterfaceConfig object.
 *
 * @param iidStack (OUT) The iidStack of the requested object.
 * @param L2tpAcIntfConfigObject (OUT) The requested object.
 *
 * @return CmsRet enum.
 */

CmsRet dalWan_getWanL2tpAcObject(InstanceIdStack *iidStack, void **obj);
CmsRet dalWan_getWanL2tpAcObject_igd(InstanceIdStack *iidStack, void **obj);
CmsRet dalWan_getWanL2tpAcObject_dev2(InstanceIdStack *iidStack, void **obj);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_getWanL2tpAcObject(v, a)  dalWan_getWanL2tpAcObject_igd((v), (a))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_getWanL2tpAcObject(v, a)  dalWan_getWanL2tpAcObject_igd((v), (a))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_getWanL2tpAcObject(v, a)  dalWan_getWanL2tpAcObject_dev2((v), (a))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_getWanL2tpAcObject(v, a)  (cmsMdm_isDataModelDevice2() ? \
                                   dalWan_getWanL2tpAcObject_dev2((v), (a)) : \
                                   dalWan_getWanL2tpAcObject_igd((v), (a)))
#endif

/** Get various adsl flags.
 *
 * @param adslFlags (OUT) contains the various adsl flags.
 *
 * @return CmsRet enum.
 */
CmsRet dalWan_getAdslFlags(UINT32 *adslFlags);


/** Set various adsl flags.
 *
 * @param adslFlags (IN) contains the various adsl flags.
 *
 * @return CmsRet enum.
 */
CmsRet dalWan_setAdslFlags(UINT32 adslFlags);


/** Get current number of used Pvc information.
 *
 * @return number of PVCs currently defined on the modem.
 */
UINT32 dalWan_getNumberOfUsedPvc(void);


/** return string "Bridge" in allBridge if all pvc are bridge for lancfg2.html
 *
 * @param brName (IN) The interface group name (BridgeName in LanDevice object).
 * @param allBridge (OUT) If all PVC's are bridge, then this char buffer
 *                        will be filled with the string "Bridge".
 */
void dalWan_allBridgePrtcl(const char *brName, char *allBridge);



/** Return the first non-bridge protocol in the system.
 *  If there are only bridge wan connections in the system, return CMS_WAN_TYPE_BRIDGE.
 *  If there are no wan connections of any type, return CMS_WAN_TYPE_UNDEFINED.
 * 
 * @return a CmsWanConnectionType enum.
 */
CmsWanConnectionType dalWan_getNoBridgeNtwkPrtcl(void);

/** Return true if any WAN interface has firewall enabled.
 * 
 * @return TRUE if any wan interface has firewall enabled.
 */
UBOOL8 dalWan_isAnyFirewallEnabled(void);




/** Return true if the WAN interface is vlanmux enabled.
 *  Maybe the qdmIntf is a better place for this function.  But for now,
 *  leave it in the DAL.
 * 
 * @param ifcName (IN) The layer 3 WAN interface name.
 *
 * @return TRUE if the wan interface is vlanmux enabled.
 */
UBOOL8 dalWan_isInterfaceVlanEnabled(const char *ifcName);

UBOOL8 dalWan_isInterfaceVlanEnabled_igd(const char *ifcName);

UBOOL8 dalWan_isInterfaceVlanEnabled_dev2(const char *ifcName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_isInterfaceVlanEnabled(i)  dalWan_isInterfaceVlanEnabled_igd((i))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_isInterfaceVlanEnabled(i)  dalWan_isInterfaceVlanEnabled_igd((i))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_isInterfaceVlanEnabled(i)  dalWan_isInterfaceVlanEnabled_dev2((i))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_isInterfaceVlanEnabled(i)   (cmsMdm_isDataModelDevice2() ? \
                              dalWan_isInterfaceVlanEnabled_dev2((i))   : \
                              dalWan_isInterfaceVlanEnabled_igd((i)))
#endif




/* functions defined in dal_wan.c */
UBOOL8 dalWan_getDslLinkCfg(const WEB_NTWK_VAR *webVar, InstanceIdStack *iidStack,
                            WanDslLinkCfgObject **dslLinkCfg);
UBOOL8 dalWan_getErrorSamples(void* obj,long* len, int lineId);
UBOOL8 dalWan_getVceMacAddress(void* vceMacAddress, int lineId);
UBOOL8 dalWan_getIpConn(const WEB_NTWK_VAR *webVar, InstanceIdStack *iidStack,
                        WanIpConnObject **ipConn);
UBOOL8 dalWan_getPppConn(const WEB_NTWK_VAR *webVar, InstanceIdStack *iidStack,
                         WanPppConnObject **pppConn);
UBOOL8 delWan_getAnotherIpConn(SINT32 connIdExcluded, InstanceIdStack *parentIidStack,
                               InstanceIdStack *iidStack, WanIpConnObject **ipConn);
UBOOL8 delWan_getAnotherPppConn(SINT32 connIdExcluded, InstanceIdStack *parentIidStack,
                                InstanceIdStack *iidStack, WanPppConnObject **pppConn);

/** Get the vlanMuxEnable value of a PVC.
 */

/**  Get the wan protocol from IpConn object
 */
SINT32 cmsDal_getWANProtocolFromIpConn(_WanDslLinkCfgObject *dslLinkCfg, _WanIpConnObject *ipConn);

/**  Get the wan protocol from PppConn object
 */
SINT32 cmsDal_getWANProtocolFromPppConn(_WanDslLinkCfgObject *dslLinkCfg);

/** Get protocol in short string format used by httpd
 */
void cmsDal_getWanProtocolName(UINT8 protocol, char *name);

               
/** Get next ppp device name.  Use the rutWan_fillPppIfName.
 * 
 * @param isPPPoE (IN)  TRUE == PPPoE. FALSE == PPPoA
 * @param pppName (OUT) The next ppp name.
 *
 * @return CmsRet enum.
 */
CmsRet  dalWan_fillPppIfName(UBOOL8 isPPPoE, char *pppName);


/** This function finds out if the WanIPConnObject is a IPoA and it calls rutWl2_isIPoA in
 * rut_wanlayer2.c
 *
 * @param iidStack (IN) iidStack of the WanIPConnObject.  This iidStack is
 *                  used to find the ancestor WanDslLinkCfg object what the linkType is
 *
 * @return UBOOL8 FALSE if it is not ATM or the link or linkeType is EOA (IPoE and Bridge)
 */
UBOOL8 dalWan_isIPoA(const InstanceIdStack *iidStack);


/** This function finds out if the WanPppConnObject is either  PPPoA or PPPoE it calls rutWl2_isPPPoA
 * in rut_wanlayer2.c
 *
 * @param iidStack (IN) iidStack of the WanPppConnObject.  This iidStack is
 *                  used to find the ancestor WanDslLinkCfg object what the linkType is
 *
 * @return UBOOL8 FALSE if it is not ATM or the link or linkeType is EOA (PPPoE)
 */
UBOOL8 dalWan_isPPPoA(const InstanceIdStack *iidStack);

/** This function finds calls rutWl2_isVlanMuxEnabled in 
 * in rut_wanlayer2.c to find out if the vlanMux is enabled  for the WanIP/PPPConnObject
 *
 * @param wanConnOid (IN) oid is either MDMOID_WAN_PPP_CONN or MDMOID_WAN_IP_CONN
 * @param iidStack   (IN) iidStack of the WanIP/PPPConnObject.  This iidStack is
 *                        used to find the ancestor layer 2 object *
 * @return UBOOL8
 */
UBOOL8 dalWan_isVlanMuxEnabled(MdmObjectId wanConnOid, const InstanceIdStack *iidStack);


/** This function finds out if the WanConn object is EoA interface or not. 
 *
 * @param iidStack (IN) iidStack of the WanIp/PppConnObject.  This iidStack is
 *                  used to find the ancestor WanDslLinkCfg object what the linkType is
 *
 * @return UBOOL8 FALSE if it is not ATM or the link or linkeType is EOA 
 */
UBOOL8 dalWan_isEoAInterface(const InstanceIdStack *iidStack);


/**  This section is all wrap funcitons for TR-98, hybrid (TR-98/TR-181), and TR-181 
 *
 */


/**
 * Add WAN service functions
 *
 * @param webVar (IN) structure containing the necessary info to add
 *                    a wan interface.
 *
 * @return CmsRet enum.
 */
CmsRet dalWan_addService(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_addService_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_addService_dev2(const WEB_NTWK_VAR *webVar);
#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_addService(w)  dalWan_addService_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_addService(w)  dalWan_addService_dev2((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_addService(w)  dalWan_addService_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_addService(w)  (cmsMdm_isDataModelDevice2() ? \
                               dalWan_addService_dev2((w)) : \
                               dalWan_addService_dev2((w)))
#endif



CmsRet dalWan_addIPv4Service(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_addIPv4Service_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_addIPv4Service_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_addIPv4Service(w)  dalWan_addIPv4Service_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_addIPv4Service(w)  dalWan_addIPv4Service_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_addIPv4Service(w)  dalWan_addIPv4Service_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_addIPv4Service(w)  (cmsMdm_isDataModelDevice2() ? \
                                   dalWan_addIPv4Service_dev2((w)) : \
                                   dalWan_addIPv4Service_igd((w)))
#endif



CmsRet dalWan_addIPv6Service(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_addIPv6Service_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_addIPv6Service_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_addIPv6Service(w)  dalWan_addIPv6Service_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_addIPv6Service(w)  dalWan_addIPv6Service_dev2((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_addIPv6Service(w)  dalWan_addIPv6Service_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_addIPv6Service(w)  (cmsMdm_isDataModelDevice2() ? \
                                   dalWan_addIPv6Service_dev2((w)) : \
                                   dalWan_addIPv6Service_dev2((w)))
#endif



/**
 * Delete WAN service  functions
 *
 * @param webVar (IN) structure containing the necessary info to add
 *                    a wan interface.
 *
 * @return CmsRet enum.
 */
CmsRet dalWan_deleteService(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_deleteService_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_deleteService_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_deleteService(w)  dalWan_deleteService_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_deleteService(w)  dalWan_deleteService_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_deleteService(w)  dalWan_deleteService_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_deleteService(w)  (cmsMdm_isDataModelDevice2() ? \
                                  dalWan_deleteService_dev2((w)) : \
                                  dalWan_deleteService_igd((w)))
#endif


/**  Fill webVar from MDM  for edit purpose
 *
 * @param webVar (IN) structure containing the necessary info to edit
 *                    a wan interface.
 *
 * @return CmsRet enum.
 */
CmsRet dalWan_getWanConInfoForEdit(WEB_NTWK_VAR *webVar);
CmsRet dalWan_getWanConInfoForEdit_igd(WEB_NTWK_VAR *webVar);
CmsRet dalWan_getWanConInfoForEdit_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_getWanConInfoForEdit(w)  dalWan_getWanConInfoForEdit_igd((w)) 
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_getWanConInfoForEdit(w)  dalWan_getWanConInfoForEdit_igd((w))  
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_getWanConInfoForEdit(w)  dalWan_getWanConInfoForEdit_dev2((w)) 
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_getWanConInfoForEdit(w)  (cmsMdm_isDataModelDevice2() ? \
                                         dalWan_getWanConInfoForEdit_dev2((w))   : \
                                         dalWan_getWanConInfoForEdit_igd((w)))
#endif

          


/** edit a wan interface.
 *
 * @param webVar (IN) structure containing the necessary info to edit
 *                    a wan interface.
 *
 * @return CmsRet enum.
 */
 //todo: later
CmsRet dalWan_editInterface(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_editInterface_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalWan_editInterface_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalWan_editInterface(w)  dalWan_editInterface_igd((w)) 
#elif defined(SUPPORT_DM_HYBRID)
#define dalWan_editInterface(w)  dalWan_editInterface_igd((w))  
#elif defined(SUPPORT_DM_PURE181)
#define dalWan_editInterface(w)  dalWan_editInterface_dev2((w)) 
#elif defined(SUPPORT_DM_DETECT)
#define dalWan_editInterface(w)  (cmsMdm_isDataModelDevice2() ? \
                                  dalWan_editInterface_dev2((w))   : \
                                  dalWan_editInterface_igd((w)))
#endif






/** Get available interface for WAN service.
 * 
 * @param ifList    (OUT) the list of available interface.
 * @param skipUsed  (IN) If FALSE, get all the layer 2 interface even if it 
 *                       and in DEFAULT_CONNECTON_MODE.

 *
 * @return CmsRet enum.
 */
CmsRet cmsDal_getAvailableIfForWanService(NameList **ifList, UBOOL8 skipUsed);
CmsRet cmsDal_getAvailableIfForWanService_igd(NameList **ifList, UBOOL8 skipUsed);
CmsRet cmsDal_getAvailableIfForWanService_dev2(NameList **ifList, UBOOL8 skipUsed);

#if defined(SUPPORT_DM_LEGACY98)
#define cmsDal_getAvailableIfForWanService(i, s)  cmsDal_getAvailableIfForWanService_igd((i), (s)) 
#elif defined(SUPPORT_DM_HYBRID)
#define cmsDal_getAvailableIfForWanService(i, s)  cmsDal_getAvailableIfForWanService_igd((i), (s)) 
#elif defined(SUPPORT_DM_PURE181)
#define cmsDal_getAvailableIfForWanService(i, s)  cmsDal_getAvailableIfForWanService_dev2((i), (s)) 
#elif defined(SUPPORT_DM_DETECT)
#define cmsDal_getAvailableIfForWanService(i, s)  (cmsMdm_isDataModelDevice2() ? \
                                                        cmsDal_getAvailableIfForWanService_dev2((i), (s))  : \
                                                        cmsDal_getAvailableIfForWanService_igd((i), (s)))
#endif

void getDefaultWanConnParams(WEB_NTWK_VAR *webVar);
void getDefaultWanConnParams_igd(WEB_NTWK_VAR *webVar);
void getDefaultWanConnParams_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define getDefaultWanConnParams(w)  getDefaultWanConnParams_igd((w)) 
#elif defined(SUPPORT_DM_HYBRID)
#define getDefaultWanConnParams(w)  getDefaultWanConnParams_igd((w)) 
#elif defined(SUPPORT_DM_PURE181)
#define getDefaultWanConnParams(w)  getDefaultWanConnParams_dev2((w)) 
#elif defined(SUPPORT_DM_DETECT)
#define getDefaultWanConnParams(w)   (cmsMdm_isDataModelDevice2() ? \
                                                        getDefaultWanConnParams_dev2((w))   : \
                                                        getDefaultWanConnParams_igd((w)))
#endif


/* The following functions are in dal2_dhcp.c */
CmsRet dalDhcpv4Relay_set_dev2(const char *ipIntfFullPath, UBOOL8 enable, const char *relayServerIpAddr);
CmsRet dalDhcp_addIpIntfClient_dev2(const char * ipIntfPathRef,
                                    const char * dhcpcOp60VID,
                                    const char * dhcpcOp61DUID,
                                    const char * dhcpcOp61IAID,
                                    const char * dhcpcOp77UID,
                                    UBOOL8 dhcpcOp125Enabled,
                                    const char * dhcpcOp50IpAddress,
                                    const char * dhcpcOp54ServerIpAddress,
                                    UINT32 dhcpcOp51LeasedTime);
CmsRet dalDhcp_deleteIpIntfClient_dev2(const char * ipIntffullPath);


#endif  /* __DAL_WAN_H__ */
