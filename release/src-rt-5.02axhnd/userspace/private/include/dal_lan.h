/***********************************************************************
 *
 *  Copyright (c) 2008  Broadcom Corporation
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

#ifndef __DAL_LAN_H__
#define __DAL_LAN_H__


/*!\file dal_lan.h
 * \brief Header file for the LAN portion of the CMS Data Abstration Layer API.
 *  This is in the cms_dal library.
 */

#include "cms_dal.h"


/** Fill in all fields of the WEB_NTWK_VAR structure that is needed to
 *  display the lancfg2 web page.
 *
 *  Since the lancfg2 web page can display information for only one LANDevice,
 *  and there could be multiple LANDevices on the modem, which LANDevice
 *  to get depends on the current value in WEB_NTWK_VAR.brName.
 *
 * @param (OUT) pointer to WEB_NTWK_VAR
 *
 */
void dalLan_getLanPageInfo(WEB_NTWK_VAR *webVar);


/** Fill in the given ifname buffer with the name of the first
 * interface group name (if TR-098 Layer 2 Bridging is defined)
 * or bridge ifName (if TR-098 Layer 2 Bridging is not defined).
 *
 * @param (OUT) pointer to a buffer of CMS_IFNAME_LENGTH bytes.
 *
 */
void dalLan_getFirstIntfGroupName(char *brName);
void dalLan_getFirstIntfGroupName_igd(char *brName);
void dalLan_getFirstIntfGroupName_dev2(char *brName);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_getFirstIntfGroupName(brName)   dalLan_getFirstIntfGroupName_igd((brName))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_getFirstIntfGroupName(brName)   dalLan_getFirstIntfGroupName_igd((brName))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_getFirstIntfGroupName(brName)   dalLan_getFirstIntfGroupName_dev2((brName))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_getFirstIntfGroupName(brName)   (cmsMdm_isDataModelDevice2() ? \
                             dalLan_getFirstIntfGroupName_dev2((brName)) : \
                             dalLan_getFirstIntfGroupName_igd((brName)))
#endif




/** Get current DHCP server configuration info.
 *
 * param ifName      (IN)  Name of the LAN side bridge interface name to get DHCP server info for.
 * @param enabled    (OUT) If not NULL, will be filled with 0 for disabled and 1 for enabled.
 * @param minAddress (OUT) In not NULL, will be filled with minimum address that DHCP
 *                         server will assign out (in dotted decimal string form).
 * @param maxAddress (OUT) In not NULL, will be filled with maximum address that DHCP
 *                         server will assign out (in dotted decimal string form).
 * @param leaseTime  (OUT) In not NULL, will be filled with lease time that the
 *                         DHCP server will assign an address for.
 * @param relaySrv (OUT) In not NULL, will be filled with DHCP relay server (in dotted decimal string form).
 */
void dalLan_getDhcpServerInfo(const char *ifName, SINT32 *enabled, char *minAddress,
                              char *maxAddress, SINT32 *leaseTime, char *relaySrv);
void dalLan_getDhcpServerInfo_igd(const char *ifName, SINT32 *enabled, char *minAddress,
                                  char *maxAddress, SINT32 *leaseTime, char *relaySrv);
void dalLan_getDhcpServerInfo_dev2(const char *ifName, SINT32 *enabled, char *minAddress,
                                   char *maxAddress, SINT32 *leaseTime, char *relaySrv);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_getDhcpServerInfo(a, b, c, d, e, f)   dalLan_getDhcpServerInfo_igd((a), (b), (c), (d), (e), (f))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_getDhcpServerInfo(a, b, c, d, e, f)   dalLan_getDhcpServerInfo_igd((a), (b), (c), (d), (e), (f))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_getDhcpServerInfo(a, b, c, d, e, f)   dalLan_getDhcpServerInfo_dev2((a), (b), (c), (d), (e), (f))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_getDhcpServerInfo(a, b, c, d, e, f)   (cmsMdm_isDataModelDevice2() ? \
              dalLan_getDhcpServerInfo_dev2((a), (b), (c), (d), (e), (f)) : \
              dalLan_getDhcpServerInfo_igd((a), (b), (c), (d), (e), (f)))
#endif




/** Get IP Interface info for the specified interface.
 *
 * @param getDefaults (IN) If true, get the default values.  Otherwise, get curent values
 *                         for the specified interface name.
 * @param ifName (IN) IfName of the ip interface. Usually this is br0, br1, br0:1, etc.
 * @param ipAddr (OUT) Upon successful return, this will be filled in with the
 *                     ipaddress of the specified IfName.  User is responsible for
 *                     allocating a buffer big enough to hold the IP address string.
 * @param subnetMask (OUT) Upon successful return, this will be filled in with the
 *                     subnet mask of the specified IfName.  User is responsible for
 *                     allocating a buffer big enough to hold the subnet mask string.
 * @param lanFirewallEnabled (OUT) Upon successful return, this will be filled in with 
 *                     whether the LAN side firewall of the specified IfName is enabled or not.
 *
 */
void dalLan_getIpIntfInfo(UBOOL8 getDefaults, const char *ifName, char *ipAddr, char *subnetMask, UBOOL8 *lanFirewallEnabled);

void dalLan_getIpIntfInfo_igd(UBOOL8 getDefaults, const char *ifName, char *ipAddr, char *subnetMask, UBOOL8 *lanFirewallEnabled);

void dalLan_getIpIntfInfo_dev2(UBOOL8 getDefaults, const char *ifName, char *ipAddr, char *subnetMask, UBOOL8 *lanFirewallEnabled);


#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_getIpIntfInfo(a, b, c, d, e)   dalLan_getIpIntfInfo_igd((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_getIpIntfInfo(a, b, c, d, e)   dalLan_getIpIntfInfo_igd((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_getIpIntfInfo(a, b, c, d, e)   dalLan_getIpIntfInfo_dev2((a), (b), (c), (d), (e))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_getIpIntfInfo(a, b, c, d, e)  (cmsMdm_isDataModelDevice2() ? \
                        dalLan_getIpIntfInfo_dev2((a), (b), (c), (d), (e)) : \
                        dalLan_getIpIntfInfo_igd((a), (b), (c), (d), (e)))
#endif




/** Get second IP Interface info for the specified interface(primary interface).
 *
 * @param primaryIfName  (IN) Primary bridge interface name.
 * @param enable       (OUT)Pointer to second IP Interface enable flag
 * @param ipAddr       (OUT)Pointer to second IP Interface address
 * @param enable       (OUT)Pointer to second IP Interface mask
 *
 */
void dalLan_get2IpIntfInfo( const char *primaryIfName, SINT32 *enable, char *ipAddr, char *subnetMask);




/** Configure a specific LAN subnet.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_setLanDevice(const WEB_NTWK_VAR *webVar);
CmsRet dalLan_setLanDevice_igd(const WEB_NTWK_VAR *webVar);
CmsRet dalLan_setLanDevice_dev2(const WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_setLanDevice(w)   dalLan_setLanDevice_igd((w))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_setLanDevice(w)   dalLan_setLanDevice_igd((w))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_setLanDevice(w)   dalLan_setLanDevice_dev2((w))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_setLanDevice(w)   (cmsMdm_isDataModelDevice2() ? \
                                  dalLan_setLanDevice_dev2((w)) : \
                                  dalLan_setLanDevice_igd((w)))
#endif




/** enable or disable dhcpd
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_ConfigDhcpd(UBOOL8 enable);


/** Add a new subnet which the dhcpd will serve.
 *  Basically, this will add a new DEV2_DHCPV4_SERVER_POOL obj and fill it out.
 *
 */
CmsRet dalLan_addDhcpdSubnet_dev2(const char *ipIntfFullPath,
                                  const char *gwIpAddr,
                                  const char *minIpAddr,
                                  const char *maxIpAddr,
                                  const char *subnetMask);


/** Delete a subnet from dhcpd service.
 *  Basically, delete the DEV2_DHCPV4_SERVER_POOL obj matching the specified
 *  ipIntfFullPath.
 */
void dalLan_deleteDhcpdSubnet_dev2(const char *ipIntfFullPath);




/* Get static IP lease info
 */
void dalLan_GetStaticIpLease(const char *brName, char *varValue);
void dalLan_GetStaticIpLease_igd(const char *brName, char *varValue);
void dalLan_GetStaticIpLease_dev2(const char *brName, char *varValue);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_GetStaticIpLease(a, b)  dalLan_GetStaticIpLease_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_GetStaticIpLease(a, b)  dalLan_GetStaticIpLease_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_GetStaticIpLease(a, b)  dalLan_GetStaticIpLease_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_GetStaticIpLease(a, b)  (cmsMdm_isDataModelDevice2() ? \
                                  dalLan_GetStaticIpLease_dev2((a), (b)) : \
                                  dalLan_GetStaticIpLease_igd((a), (b)))
#endif


/** Set the static IP lease entry for udhcpd.
 *
 * @param static_ip  (IN) IPv4 address that is binded to mac.
 * @param mac        (IN) MAC address that is assigned to static_ip
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_StaticIPAdd(const char *brName, const char *static_ip, const char *mac);
CmsRet dalLan_StaticIPAdd_igd(const char *brName, const char *static_ip, const char *mac);
CmsRet dalLan_StaticIPAdd_dev2(const char *brName, const char *static_ip, const char *mac);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_StaticIPAdd(a, b, c)  dalLan_StaticIPAdd_igd((a), (b), (c))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_StaticIPAdd(a, b, c)  dalLan_StaticIPAdd_igd((a), (b), (c))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_StaticIPAdd(a, b, c)  dalLan_StaticIPAdd_dev2((a), (b), (c))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_StaticIPAdd(a, b, c)  (cmsMdm_isDataModelDevice2() ? \
                                   dalLan_StaticIPAdd_dev2((a), (b), (c)) : \
                                   dalLan_StaticIPAdd_igd((a), (b), (c)))
#endif


/** Remove the static IP lease entry for udhcpd.
 *
 * @param mac        (IN) MAC address to remove
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_StaticIPRemove(const char *brName, const char *mac);
CmsRet dalLan_StaticIPRemove_igd(const char *brName, const char *mac);
CmsRet dalLan_StaticIPRemove_dev2(const char *brName, const char *mac);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_StaticIPRemove(a, b)  dalLan_StaticIPRemove_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_StaticIPRemove(a, b)  dalLan_StaticIPRemove_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_StaticIPRemove(a, b)  dalLan_StaticIPRemove_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_StaticIPRemove(a, b)  (cmsMdm_isDataModelDevice2() ? \
                                      dalLan_StaticIPRemove_dev2((a), (b)) : \
                                      dalLan_StaticIPRemove_igd((a), (b)))
#endif




/** Call appropriate cmsObj_set function to set the Igmp snooping info.
 *
 * @param brName        (IN) Bridge Name, also known as interface group name
 * @param enblIgmpSnp   (IN) enable igmp snooping
 * @param enblIgmpMode  (IN) 0 means standard mode, 1 means blocking mode
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_setIgmpSnooping(const char *brName, SINT32 enblIgmpSnp, SINT32 enblIgmpMode, SINT32 enblIgmpLanToLanMcast);


/** Get Igmp snooping enabled/disabled
 *
 * @param brName        (IN) Bridge Name, also known as interface group name
 * @param varValue     (OUT) if enabled, varValue will contain a string "1", 
 *                           otherwise, a string of "0"
 */
void dalLan_getIgmpSnoopingEnabled(const char *brName, char *varValue);


/** Get Igmp snooping mode
 *
 * @param brName        (IN) Bridge Name, also known as interface group name
 * @param varValue     (OUT) if standard mode, varValue will contain a string "0", 
 *                           otherwise, a string of "1"
 */
void dalLan_getIgmpSnoopingMode(const char *brName, char *varValue);


/** Get Igmp Lan To Lan Multicast Enabled
 *
 * @param brName        (IN) Bridge Name, also known as interface group name
 * @param varValue     (OUT) if disabled, varValue will contain a string "0",
 *                           if enabled, a string of "1"
 *                           if deferred to default, "-1"
 */
void dalLan_getIgmpLanToLanMcastEnable(const char *brName, char *varValue);


/** Return true if any LAN interface has firewall enabled.
 * 
 * @return TRUE if any LAN interface has firewall enabled.
 */
UBOOL8 dalLan_isAnyFirewallEnabled(void);


/** Find the ancestor LANDevice object corresponding to the given bridge ifName.
 *
 * @param brIfName  (IN)  Bridge IfName, such as br0, br1, etc.
 * @param iidStack  (OUT) On successful return, will be the iidStack of the ancestor LANDevice.
 *                        This pointer must not be NULL.
 * @param lanDev    (OUT) On successful return, will be the LanDevice Object.  This pointer
 *                        must not be NULL.
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_getLanDevByBridgeIfName(const char *brIfName, InstanceIdStack *iidStack, LanDevObject **lanDev);


/** Get the bridge interface name (br0, br1, etc) from the interface group name.
 *
 * @param bridgeName (IN) Bridge name or interface group name.
 * @param bridgeIfName (OUT) Bridge interface name, e.g. br0, br1, etc.
 *
 */
void dalLan_getBridgeIfNameFromBridgeName(const char *bridgeName, char *bridgeIfName);

void dalLan_getBridgeIfNameFromBridgeName_igd(const char *bridgeName, char *bridgeIfName);

void dalLan_getBridgeIfNameFromBridgeName_dev2(const char *bridgeName, char *bridgeIfName);


#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_getBridgeIfNameFromBridgeName(a, b)  dalLan_getBridgeIfNameFromBridgeName_igd((a), (b))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_getBridgeIfNameFromBridgeName(a, b)  dalLan_getBridgeIfNameFromBridgeName_igd((a), (b))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_getBridgeIfNameFromBridgeName(a, b)  dalLan_getBridgeIfNameFromBridgeName_dev2((a), (b))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_getBridgeIfNameFromBridgeName(a, b)  (cmsMdm_isDataModelDevice2() ? \
                      dalLan_getBridgeIfNameFromBridgeName_dev2((a), (b)) : \
                      dalLan_getBridgeIfNameFromBridgeName_igd((a), (b)))
#endif




/** Connect lower layer WAN intf to LAN bridge.  Used for adding bridge service.
 *
 * @param fullPath   (IN) fullpath to the interface to add to the bridge
 * @param brIntfName (IN) name of the bridge
 *
 * @return CmsRet
 */
CmsRet dalBridge_addFullPathToBridge_dev2(const char *fullPath,
                                          const char *brIntfName);


/** Delete intf from LAN bridge
 *
 * @param intfName  (IN) name of the interface to remove from bridge
 */
void dalBridge_deleteIntfNameFromBridge_dev2(const char *intfName);




/** Get the specified ethernet interface on the LAN side.
 *
 * @param ifName     (IN) The interface name of the eth interface to get.
 * @param iidStack  (OUT) the iidStack of the requested eth interface.
 * @param lanEthObj (OUT) The requested LanEthernetInterfaceConfig object.  Caller
 *                        is responsible for calling cmsObj_free() on this object.
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_getEthInterface(const char *ifName, InstanceIdStack *iidStack, LanEthIntfObject **lanEthObj);


/** Get the specified Moca interface on the LAN side.
 *
 * @param ifName     (IN) The interface name of the moca interface to get.
 * @param iidStack  (OUT) the iidStack of the requested moca interface.
 * @param lanEthObj (OUT) The requested LanMocaInterfaceConfig object.  Caller
 *                        is responsible for calling cmsObj_free() on this object.
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_getMocaInterface(const char *ifName, InstanceIdStack *iidStack, LanMocaIntfObject **lanMocaObj);



/** Call appropriate cmsObj_set function to set the Mld snooping info.
 *
 * @param brName        (IN) Bridge Name, also known as interface group name
 * @param enblMldSnp    (IN) enable mld snooping
 * @param enblMldMode   (IN) 0 means standard mode, 1 means blocking mode
 *
 * @return CmsRet enum.
 */
CmsRet dalLan_setMldSnooping(const char *brName, SINT32 enblMldSnp, SINT32 enblMldMode, SINT32 enblMldLanToLanMcast);

/** Get Mld snooping enabled/disabled
 *
 * @param brName        (IN) Bridge Name, also known as interface group name
 * @param varValue     (OUT) if enabled, varValue will contain a string "1", 
 *                           otherwise, a string of "0"
 */
void dalLan_getMldSnoopingEnabled(const char *brName, char *varValue);

/** Get Mld snooping mode
 *
 * @param brName        (IN) Bridge Name, also known as interface group name
 * @param varValue     (OUT) if standard mode, varValue will contain a string "0", 
 *                           otherwise, a string of "1"
 */
void dalLan_getMldSnoopingMode(const char *brName, char *varValue);

/** Get Mld Lan To Lan Multicast
 *
 * @param brName        (IN) Bridge Name, also known as interface group name
 * @param varValue     (OUT) if disabled, varValue will contain a string "0",
 *                           if enabled, a string of "1"
 *                           if deferred to default, "-1"
 */
void dalLan_getMldLanToLanMulticastEnable (const char *brName, char *varValue);



#ifdef SUPPORT_LANVLAN
/** get LAN VLAN parameter.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return void
 */
void dalLan_getLanVlan(WEB_NTWK_VAR *webVar);
void dalLan_getLanVlan_igd(WEB_NTWK_VAR *webVar);
void dalLan_getLanVlan_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_getLanVlan(webVar)   dalLan_getLanVlan_igd((webVar))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_getLanVlan(webVar)   dalLan_getLanVlan_igd((webVar))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_getLanVlan(webVar)   dalLan_getLanVlan_dev2((webVar))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_getLanVlan(webVar)   (cmsMdm_isDataModelDevice2() ? \
                             dalLan_getLanVlan_dev2((webVar)) : \
                             dalLan_getLanVlan_igd((webVar)))
#endif
/** set LAN VLAN parameter.
 *
 * @param (IN) pointer to WEB_NTWK_VAR
 *
 * @return void
 */
void dalLan_setLanVlan(WEB_NTWK_VAR *webVar); 
void dalLan_setLanVlan_igd(WEB_NTWK_VAR *webVar);
void dalLan_setLanVlan_dev2(WEB_NTWK_VAR *webVar);

#if defined(SUPPORT_DM_LEGACY98)
#define dalLan_setLanVlan(webVar)   dalLan_setLanVlan_igd((webVar))
#elif defined(SUPPORT_DM_HYBRID)
#define dalLan_setLanVlan(webVar)   dalLan_setLanVlan_igd((webVar))
#elif defined(SUPPORT_DM_PURE181)
#define dalLan_setLanVlan(webVar)   dalLan_setLanVlan_dev2((webVar))
#elif defined(SUPPORT_DM_DETECT)
#define dalLan_setLanVlan(webVar)   (cmsMdm_isDataModelDevice2() ? \
                             dalLan_setLanVlan_dev2((webVar)) : \
                             dalLan_setLanVlan_igd((webVar)))
#endif
#endif /* SUPPORT_LANVLAN */

#endif  /* __DAL_LAN_H__ */
