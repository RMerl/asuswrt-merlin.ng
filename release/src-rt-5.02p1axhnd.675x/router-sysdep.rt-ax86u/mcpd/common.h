/***********************************************************************
 *
 *  Copyright (c) 2006  Broadcom Corporation
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
/***************************************************************************
 * File Name  : common.h
 *
 * Description: common proto types for kernel and debug interface
 *              
 ***************************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__

/** Initialize the MCPD debug mem utility
 *
 *
 * @return None
 *
 */
void mcpd_init_mem_utility(void);

/** Display the MCPD debug mem utility
 *
 *
 * @return None
 *
 */
void mcpd_display_mem_usage(void);

/** malloc wrapper function for MCPD
 *
 * @param obj_type   (IN) object type of MCPD objects
 *
 * @param size       (IN) size of memory allocation needed by malloc
 *
 * @return pointer to memory on success or NULL on failure
 *
 */
void *mcpd_alloc(int obj_type, int size);

/** free wrapper function for MCPD
 *
 * @param obj_type   (IN) object type of MCPD objects
 *
 * @param ptr        (IN) ptr to memory
 *
 * @return None
 *
 */
void mcpd_free(int obj_type, void *ptr);

/** wait for mcpd interfaces to come up
 *
 *
 * @return None
 *
 */
void mcpd_wait_for_interfaces(void);

/** convert ifindex to if name
 *
 * @param sockfd     (IN) socket descriptor id
 *
 * @param ifindex    (IN) if index of interface
 *
 * @param ifname     (OUT) if name of interface
 *
 * @return if name of interface
 *
 */
char * mcpd_ifindex_to_name(int sockfd, unsigned int ifindex, char *ifname);

/** convert ifname to ifindex
 *
 * @param sockfd     (IN) socket descriptor id
 *
 * @param ifindex    (OUT) if index of interface
 *
 * @param ifname     (IN) if name of interface
 *
 * @return 0 success, -1 failure
 *
 */
int mcpd_ifname_to_index(int sockfd, unsigned int *ifindex, const char *ifname );

/** check to see if interface is in the upstream proxy list
 *
 * @param s          (IN) if name of interface
 *
 * @param proto      (IN) protocol list to check
 *
 * @return MCPD_TRUE or MCPD_FALSE
 *
 */
int mcpd_upstream_interface_lookup(char *s, t_MCPD_PROTO_TYPE proto);

/** In strict WAN mode, determine if WAN service and bridge are associated
 *  In normal mode, always return SUCCESS
 *
 * @param ifp_wan          (IN) WAN service
 * @param ifp_bridge       (IN) bridge group
 *
 * @return MCPD_TRUE or MCPD_FALSE
 *
 */
t_MCPD_BOOL mcpd_is_wan_service_associated_with_bridge(t_MCPD_INTERFACE_OBJ *ifp_wan, t_MCPD_INTERFACE_OBJ *ifp_bridge);

/** In strict WAN mode, see if bridge has an MCAST enabled WAN service
 *
 * @param s          (IN) ifp_bridge
 *
 * @return MCPD_TRUE or MCPD_FALSE
 *
 */
t_MCPD_BOOL mcpd_is_bridge_associated_with_mcast_wan_service (t_MCPD_INTERFACE_OBJ *ifp_bridge, int proto);

/** check to see if interface is in the downstream snooping list
 *
 * @param s          (IN) if name of interface
 *
 * @param proto      (IN) protocol list to check
 *
 * @return MCPD_TRUE or MCPD_FALSE
 *
 */
int mcpd_downstream_interface_lookup(char *s, t_MCPD_PROTO_TYPE proto);

/** check to see if interface is a mutlicast source
 *
 * @param s          (IN) if name of interface
 *
 * @param proto      (IN) protocol list to check
 *
 * @return MCPD_TRUE or MCPD_FALSE
 *
 */
int mcpd_mcast_interface_lookup(char *s, t_MCPD_PROTO_TYPE proto);

/** mcpd debug trace api
 *
 * @param level      (IN) level of tracing debug
 *
 * @param fmt        (IN) format of tracing string
 *
 * @return None
 *
 */
void mcpd_debug(int level, const char* fmt, ...);

/** Compute the inet checksum
 *
 * @param *addr       (IN) buf ptr
 *
 * @param len         (IN) length 
 *
 * @return checksum value
 *
 */
unsigned short mcpd_in_cksum(unsigned short *addr, int len);

/** get the interface information from kernel
 *
 * @param flags       (IN) flags for match
 *
 * @param unflags     (IN) don't match these  flags 
 *
 * @return pointer to allocated t_MCPD_IFINFO_OBJ 
 *
 */
t_MCPD_IFINFO_OBJ *mcpd_get_ifinfo(short flags, short unflags);

/** Free a list of interfaces
 *
 * @param ifl         (IN) interface pointer head
 *
 * @return None
 *
 */
void mcpd_free_ifinfo(t_MCPD_IFINFO_OBJ *ifl);

/** Get the value of the flags for a interface 
 *
 * @param *ifname     (IN) pointer to ifname of interface
 *
 * @return flags
 *
 */
short mcpd_get_interface_flags(char *ifname);

/** Set the value of the flags for a interface 
 *
 * @param *ifname     (IN) pointer to ifname of interface
 *
 * @param flags       (IN) flags for interface
 *
 * @return 0 on success or -1 on failure
 *
 */
short mcpd_set_interface_flags(char *ifname, short flags);

/** Get the value of the mtu for a interface 
 *
 * @param *ifname     (IN) pointer to ifname of interface
 *
 * @return interface mtu on success or -1 on failure
 *
 */
int mcpd_get_interface_mtu(char *ifname);

/** checks to see if the interface has valid ip address
 *
 * @param *ifname     (IN) pointer to ifname of interface
 *
 * @return MCPD_TRUE on success or MCPD_FALSE on failure
 *
 */
int mcpd_is_valid_ipaddr(char *ifname);

/** checks to see if a bridge interface has a WAN interface as a member
 *
 * @param *ifname     (IN) pointer to ifname of bridge
 *
 * @return MCPD_TRUE on success or MCPD_FALSE on failure
 *
 */
int mcpd_bridge_includes_wan_interface(char *ifname);

/** checks to see if the WAN interface is a member of a LAN bridge
 *
 * @param *ifindex     (IN) IF index of WAN device
 *
 * @return MCPD_TRUE on success or MCPD_FALSE on failure
 *
 */
int mcpd_wan_is_bridge_member(int ifindex);

/** checks to see if the ifIndex is a member of brName
 *
 * @param *brName (IN) pointer to name of bridge
 *
 * @param ifIndex (IN) interface index of port
 *
 * @return MCPD_TRUE on success or MCPD_FALSE on failure
 *
 */
int mcpd_is_bridge_member(char *brName, int ifIndex);

/** get all ifIndex under the brName
 *
 * @param *brName (IN) pointer to name of bridge
 *
 * @param ifInBridge (IN) pointer to the ifInBridge[]
 *
 * @param numprt (OUT) the num of bridge members
 *
 * @return MCPD_TRUE on success or MCPD_FALSE on failure
 *
 */
int mcpd_get_bridge_members(char *brName, int *ifInBridge, unsigned int *numprt);

/** prints IPv6 address 
 *
 * @param *addr6      (IN) pointer to ipv6 address
 *
 * @return None
 *
 */
void mcpd_print_ipv6_addr(struct in6_addr *addr6);

/** check DAD status by if index of interface
 *
 * @param if_index    (IN) if index of interface
 *
 * @return 0 on success -1 on failure
 *         Fails if TENTATIVE bit is set
 *
 */
int mcpd_check_ipv6_dad_status(int if_index);

/** get IPv6 address by if index of interface
 *
 * @param if_index    (IN) if index of interface
 *
 * @param saddr6      (OUT) IPv6 address
 *
 * @return 0 on success -1 on failure
 *
 */
int mcpd_get_ipv6_addr_by_ifidx(int if_index, struct sockaddr_in6 *saddr6);

/** dump memory for debug
 *
 * @param *buf        (IN) pointer to buffer
 *
 * @param len         (IN) length of buffer
 *
 * @return None
 *
 */
void mcpd_dump_buf(char *buf, int len);

/** check if it is wan interface
 *
 * @param *ifname     (IN) pointer to interface name 
 *
 * @return MCPD_TRUE or MCPD_FALSE
 *
 */
int mcpd_is_wan_interface(char *ifname);

/**
 *
 * @param *portName   (IN) pointer to port name 
 *
 * @return port number or -1 if none found
 *
 */
int mcpd_convertBrPortNameToPortNo(const char *portName);

/**
 *
 * @param *brName   (IN) pointer to bridge name 
 *
 * @return 1 if it is a bridge or 0 otherwise
 *
 */
int mcpd_is_bridge(char *portName);

#endif /* __COMMON_H__ */
