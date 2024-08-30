/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2019:DUAL/GPL:standard
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

#ifndef __SYSUTIL_NET_H__
#define __SYSUTIL_NET_H__

/*!\file sysutil_net.h
 * \brief Header file for various utility functions dealing with networking.
 *
 */


/** System networking constants, similar to what is defined in cms.h.  But
  * These constants have nothing to do with CMS or BDK or Broadcom.  They
  * might be defined in some linux header file somewhere, but it is convenient
  * to have them here.
  */

#ifndef MAC_ADDR_LEN
// Mac addr len when it is in an array of 6 bytes.
#define MAC_ADDR_LEN    6
#endif

// Mac addr len when it is in string form separated by : eg: 12:34:56:78:9A:BC
// including the null termination character.
#define SYSUTIL_MAC_ADDR_STR_LEN     18

// Linux IFNAMSIZ is usually 16, but CMS has traditionally used 32.  So keep
// using the longer length, which should be ok.
#define SYSUTL_IFNAME_LEN  32

// IP address length to hold IPv6 in CIDR notation (match INET6_ADDRSTRLEN)
#define SYSUTL_IPV6_ADDR_LEN   46

// IP address length to hold IPv6 prefix plus null termination (from TR181)
#define SYSUTL_IPV6_PREFIX_LEN   50

// IP address buffer to hold IPv4 in CIDR notation (e.g. 192.168.100.201) plus
// null terminating character.
#define SYSUTL_IPV4_ADDR_LEN   16

// This buffer can hold any IP address related string, so use the longest one.
#define SYSUTL_IPVX_ADDR_LEN   SYSUTL_IPV6_PREFIX_LEN

// Invalid ifindex
// On success, if_nametoindex() returns the index number of the network interface;
// on error, 0 is returned
#define INVALID_IFINDEX 0

// Invalid vlan id
#define INVALID_VLANID 4096

/** Open a netlink socket.
 *
 * @param protocol (IN) NETLINK_ROUTE, NETLINK_FIREWALL, NETLINK_ARPD, etc.
 * @param pid (IN) the pid or thread id of the caller.
 * @param groups (IN) the groups to listen to.
 *
 * @return fd on success, -1 on error.
 */
int sysUtl_openNetlinkSocket(int protocol, unsigned int pid, unsigned int groups);


/** Get the kernel interface index for the given interface name.
 *
 * @param ifname (IN) interface name.
 *
 * @return the kernel interface index, or -1 on error.
 */
int sysUtl_getIfindexByIfname(const char *ifname);


/** Get interface name by kernel interface index.
 *
 * @param index (IN) Kernel interface index.
 * @param intfName (OUT) buffer to hold result.  Should be at least
 *        IFNAMESZ (16) bytes but does not hurt to make it bigger.
 *
 * @return 0 on success, -1 on failure.
 */
int sysUtl_getIfnameByIndex(int index, char *intfName);


/** Return 1 if the given interface has Link up.
 */
int sysUtl_isInterfaceLinkUp(const char *ifname);


/** Return 1 if the given interface has been configured up.  Note this does
  * not mean the interface has link up.
 */
int sysUtl_isInterfaceConfigedUp(const char *ifname);


/** Return TRUE if the given interface name is under the specified bridge.
 *
 * @param brName   (IN) bridge interface name, e.g. br0
 * @param intfName (IN) interface name, e.g. eth2.0
 *
 * @return 1 if TRUE, 0 if false or error.
 */
int sysUtl_isInterfaceUnderBridge(const char *brName, const char *intfName);


/** Return TRUE if the given interface name exists.
 *
 * @param ifname   (IN) given interface name
 *
 * @return 1 if the interface exists, 0 if it does not exist.
 */
int sysUtl_interfaceExists(const char *ifname);


/** Return ifindex of the parent device of the given interface.
 *
 * @param ifname   (IN) given interface name
 *
 * @return ifindex.
 */
int sysUtl_getLowerDeviceIfindex(const char *ifname);


/** Return vlanid of the the given interface.
 *
 * @param ifname   (IN) given interface name
 *
 * @return vlanid.
 */
int sysUtl_getVlanId(const char *ifname);

#endif /* __SYSUTIL_NET_H__ */
