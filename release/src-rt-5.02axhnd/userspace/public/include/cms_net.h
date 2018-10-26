/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
:>
 *
 ************************************************************************/


#ifndef __CMS_NET_H__
#define __CMS_NET_H__

/*!\file cms_net.h
 * \brief Header file for network utilities that do not require
 *  access to the MDM.
 */


#include "cms.h"
#include <arpa/inet.h>  /* mwang_todo: should not include OS dependent file */

/** Get LAN IP address and netmask.
 *
 * @param ifname (IN) name of LAN interface.
 * @param lan_ip (OUT) in_addr of the LAN interface.
 * @param lan_subnetmask (OUT) in_addr of the LAN interface subnet mask.
 *
 * @return CmsRet enum.
 */
CmsRet cmsNet_getLanInfo(const char *ifname, struct in_addr *lan_ip, struct in_addr *lan_subnetmask);


/** Return TRUE if the specified interface is UP.
 *
 *  Note on Linux, this function looks at the IFF_UP flag, which indicates
 *  _administrative_ status, not actual link status.  For link status,
 *  use cmsNet_isInterfaceLinkUp()
 *
 * @param ifname (IN) name of the interface.
 *
 * @return TRUE if the specified interface is UP.
 */
UBOOL8 cmsNet_isInterfaceUp(const char *ifname);


/** Return TRUE if the specified interface's LINK status is UP.
 *
 *  Note on Linux, this function looks at the IFF_RUNNING flag, which
 *  indicates link status.
 *
 * @param ifname (IN) name of the interface
 *
 * @return TRUE if the specified interfaces link is UP.
 */
UBOOL8 cmsNet_isInterfaceLinkUp(const char *ifname);



/** Return TRUE if the specified IP address is on the LAN side.
 *
 * @param ipAddr (IN) IP address in question.
 *
 * @return TRUE if the specified IP address is on the LAN side.
 */
UBOOL8 cmsNet_isAddressOnLanSide(const char *ipAddr);


/** Returns the number of left most one bit in subnet mask.
 *  Ex: 255.255.255.0 -> 24
 * 
 * @param ipMask (IN) IP mask string in dot notation.
 *
 * @return the number of left most one bit in the given subnet mask.
 *         (The function name and description seems inaccurate.  Maybe a
 *         better description is the number of consecutive 1 bits starting
 *         from the left and going towards the right, assuming network
 *         order/big endian layout.)
 */
UINT32 cmsNet_getLeftMostOneBitsInMask(const char *ipMask);


/** Convert an IP address/mask string from CIDR notation into binary address and mask
 *  and store them in the structures that ipAddr and ipMask point to respectively.
 *  Ex: 192.168.1.100/24 -> ipAddr->s_addr=0xCOA80164  ipMask->s_addr=0xFFFFFF00
 * 
 * @param cp (IN) IP address/mask string in CIDR notation.
 * @param ipAddr (OUT) IP address in binary. Set to 0 if invalid.
 * @param ipMask (OUT) IP mask in binary. Set to 0 if invalid.
 *
 * @return void
 */
void cmsNet_inet_cidrton(const char *cp, struct in_addr *ipAddr, struct in_addr *ipMask);



/** Convert IPv4 Addr String and netmask to CIDR notation.
 *
 * @param ipv4Addr    (IN) IPv4 addr in dotted ASCII, e.g. 192.168.1.1
 * @param ipv4Netmask (IN) IPv4 addr in dotted ASCII, e.g. 255.255.255.0
 * @param ipv4Cidr   (OUT) IPv4 addr in CIDR format, e.g. 192.168.1.1/24
 *                    Note that in an non-IPv6 compile, CMS_IPADDR_LENGTH = 16
 *                    but this buffer must be at least 19 bytes long.
 *                    In an IPv6 compile. CMS_IPADDR_LENGTH=46, which is
 *                    long enough to hold IPv6 addr in CIDR format.
 */
void cmsNet_inet_ipv4AddrStrtoCidr4(const char *ipv4Addr,
                                    const char *ipv4Netmask,
                                    char *ipv4Cidr);



/** Return the ifindex of an interface.
 *
 * @param ifname (IN) .
 * @return ifindex of the interface.  -1 indicates error.
 */
int cmsNet_getIfindexByIfname(const char *ifname);


/** Return the name of the interface corresponding to the given index.
 *
 * @param index     (IN) Linux interface index
 * @param intfName (OUT) On successful return, this buffer will contain
 *                       the interface name.  Buffer must be at least
 *                       CMS_IFNAME_LENGTH bytes.
 * @return 0 on success, -1 on error.
 */
int cmsNet_getIfnameByIndex(int index, char *intfName);


/** Return 1 if the given interface name exists
 *
 * @param intfName (IN) interface name
 *
 * @return 1 if the given interface name exists.
 */
int cmsNet_isInterfaceExist(const char *intfName);


/** Return the mac address of an interface.
 *
 * @param ifname  (IN)  Linux interface name, e.g. br0, eth0
 * @param macAddr (OUT) On success, this buffer contains the mac address in
 *                      binary form (not string form: "aa:bb:cc:11:22:33").
 *                      The buffer must be at least MAC_ADDR_LEN (6) bytes.
 *                      Since this buffer contains binary data, it is NOT null terminated.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getMacAddrByIfname(const char *ifname, unsigned char *macAddr);


/** Return the mac address string of an interface.
 *
 * @param ifname  (IN)  Linux interface name, e.g. br0, eth0
 * @param macAddr (OUT) On success, this buffer contains the mac address
 *                      in string format, e.g. "aa:bb:cc:11:22:33".
 *               The buffer must be at least MAC_STR_LEN+1 (17+1) bytes.
 *               The extra 1 byte is for NULL termination.  Returned
 *               macAddrStr will be NULL terminated.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getMacAddrStringByIfname(const char *ifname, char *macAddrStr);


/** Return Ethernet interface link status.
 * This API is similar to CLI "ethctl <interface> media-type [port <port id>]"
 *
 * @param ifname  (IN)  Ethernet interface name.
 * @param port    (IN)  Port Id is required if interface is Ethernet Switch and
 *                      virtual interfaces (per switch port) is not supported.
 * @param speed   (OUT) Speed in Mbps.
 * @param fullDuplex (OUT) TRUE if full duplex. FALSE if half duplex.
 * @param linkUp     (OUT) TRUE if link is Up. FALSE if link is Down.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getEthLinkStatus(const char *ifname, int port, int *speed, UBOOL8 *fullDuplex, UBOOL8 *linkUp);

/** Get a list of interface names in the system.
 *
 * @param ifNameList (OUT) Pointer to char * of ifnames.  This function will allocate
 *                         a buffer long enough to hold a list of all the interface
 *                         names in the system, separated by comma.  e.g. atm0,ppp0,eth0,eth1
 *                         Caller is responsible for freeing the buffer.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getIfNameList(char **ifNameList);


/** Get a list of WAN capable ethernet interface names in the system.
 *  This list only includes eth interface names.
 *
 * @param wanCapIfNameList (OUT) Pointer to char *.
 *                               This function will allocate a buffer long
 *                               enough to hold a list of all the WAN capable
 *                               ethernet interface names in the system,
 *                               separated by comma.  e.g. eth0,eth1
 *                               Caller is responsible for freeing the buffer.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getWanCapableIfNameList(char **wanCapIfNameList);


/** Get a list of GMAC Eth interface names in the system.
 *
 * @param GMACPortIfNameList (OUT) Pointer to char * of GMACPortIfNameList.  
 *                                      This function will allocate
 *                                      a buffer long enough to hold a list of all the interface
 *                                      names in the system, separated by comma.  e.g. eth0,eth1
 *                                      Caller is responsible for freeing the buffer.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getGMACPortIfNameList(char **GMACPortIfNameList);

/** Get a list of WAN Only Eth interface names in the system.
 *
 * @param pWANOnlyPortList (OUT) Pointer to char * of pWANOnlyPortList.  
 *                                      This function will allocate
 *                                      a buffer long enough to hold a list of all the interface
 *                                      names in the system, separated by comma.  e.g. eth0,eth1
 *                                      Caller is responsible for freeing the buffer.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getWANOnlyEthPortIfNameList(char **pWANOnlyPortList);;

/** Get a list of LAN Only Eth interface names in the system.
 *
 * @param pLANOnlyPortList (OUT) Pointer to char * of pLANOnlyPortList.  
 *                                      This function will allocate
 *                                      a buffer long enough to hold a list of all the interface
 *                                      names in the system, separated by comma.  e.g. eth0,eth1
 *                                      Caller is responsible for freeing the buffer.
 *
 * @return CmsRet
 */
CmsRet   cmsNet_getLANOnlyEthPortIfNameList(char **pLANOnlyPortList);

/** Get IPv6 address info about the specified ifname.
 *
 * @param ifname  (IN)  desired ifname
 * @param addrIdx (IN)  Since a single ifname can have multiple addresses, addrIdx specifies
 *                      which instance of the address is desired.  Caller will probably 
 *                      have to call this function in a loop to get all instances.
 * @param ipAddr  (OUT) Caller supplies a buffer to hold address.  Must be at least
 *                      CMS_IPADDR_LENGTH (46 bytes) long.
 * @param ifIndex (OUT) Kernel internal index for this interface.
 * @param prefixLen (OUT) Prefix len of the address
 * @param scope   (OUT) Scope value of address, 0=global, 32=link local, 16=host
 * @param ifaFlags(OUT) flags on this interface.
 *
 * @return CmsRet
 */
CmsRet cmsNet_getIfAddr6(const char *ifname, UINT32 addrIdx,
                         char *ipAddr, UINT32 *ifIndex, UINT32 *prefixLen, UINT32 *scope, UINT32 *ifaFlags);


/** Get the Globally Unique IPv6 address for the specified ifname.
 *
 * @param ifname  (IN)  desired ifname
 * @param ipAddr  (OUT) Caller supplies a buffer to hold address.  Must be at least
 *                      CMS_IPADDR_LENGTH (46 bytes) long.
 * @param prefixLen (OUT) Prefix len of the address
 *
 * @return CmsRet
 */
CmsRet cmsNet_getGloballyUniqueIfAddr6(const char *ifname, char *ipAddr, UINT32 *prefixLen);


/** Check if two IPv6 addresses are equal.
 *
 * @param ip6Addr1 (IN) IPv6 address 1.
 * @param ip6Addr2 (IN) IPv6 address 2.
 *
 * @return TRUE if the specified two IPv6 addresses are equal.
 */
UBOOL8 cmsNet_areIp6AddrEqual(const char *ip6Addr1, const char *ip6Addr2);

/** Check if two IPv6 DNS server addresses are equal.
 *
 * @param dnsServers1 (IN) DNS server address 1.
 * @param dnsServers2 (IN) DNS server address 2.
 *
 * @return TRUE if the specified two DNS server addresses are equal.
 */
UBOOL8 cmsNet_areIp6DnsEqual(const char *dnsServers1, const char *dnsServers2);

/** Check if a host IPv6 address is in the same subnet of an address prefix.
 *
 * @param addrHost   (IN) host address to check.
 * @param addrPrefix (IN) the address prefix.
 *
 * @return TRUE if the host address is in the same subnet of the address prefix.
 */
UBOOL8 cmsNet_isHostInSameSubnet(const char *addrHost, const char *addrPrefix);

/** This function will subnet an address prefix based on the subnet id and the
 *  subnet prefix length..
 *
 * @param sp       (IN) the address prefix in CIDR notation.
 * @param subnetId (IN) the subnet id.
 * @param snPlen   (IN) the subnet prefix length.
 * @param snPrefix (OUT) the subnet address prefix.
 *
 * @return CmsRet enum.
 */
CmsRet cmsNet_subnetIp6SitePrefix(const char *sp, UINT8 subnetId, UINT32 snPlen, char *snPrefix);

/** Generate address from MAC (EUI-64)
 *
 * @param prefix     (IN) 
 * @param mac        (IN)
 * @param addr       (OUT)
 *
 * @return CmsRet enum.
 */
CmsRet cmsUtl_prefixMacToAddress(const char *prefix, UINT8 *mac, char *addr);

/** first sub interface number for virtual ports, e.g. eth1.2, eth1.3 */
#define START_PMAP_ID           2

/** Max vendor id string len */
#define DHCP_VENDOR_ID_LEN      64

/** Maximum number of vendor id strings we can support in WebUI for portmapping. 
 * 
 * This is an arbitrary limit from the WebUI, but it propagates through to
 * utility functions dealing with DHCP Vendor Id for port mapping.
 */
#define MAX_PORTMAPPING_DHCP_VENDOR_IDS     5


#endif /* __CMS_NET_H__ */
