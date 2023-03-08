/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
************************************************************************/

#ifndef __BCM_NET_H__
#define __BCM_NET_H__

#if defined __cplusplus
extern "C" {
#endif

/*!\file bcm_net.h
 * \brief Broadcom Userspace Network helper utility.
 *
 * This utility leverages on Linux's networking/routing tools to help
 * retrieving networking and routing information of the system.
 */


/** Retrieve the local interface (and its properties) that the system
 * can use to reach the given server.
 *
 * This function returns the name and IP address of the local interface that
 * the system can use to reach the given server IP address. In addition,
 * the maximum data rates at which the gateway can transmit/receive to/from
 * the given server IP addresses at the determined local interface are also
 * returned.
 *
 * @param serverIp (IN) destination server IP in dotted decimal format
 *  (IPv4 or IPv6)
 * @param intfName (OUT) outbound interface name to use. The buffer must be
 *  at least IFNAMSZ (16 bytes as defined in net/if.h)
 * @param intfIp (OUT) outbound interface IP in dotted decimal format. The
 *  buffer must be at least INET_ADDRSTRLEN (16 bytes) for IPv4, or 
 *  INET6_ADDRSTRLEN (46 bytes) for IPv6
 * @param upSpeed (OUT) the uplink speed in kbps
 * @param downSpeed (OUT) the downlink speed kbps
 * @return 0 on success, -1 on error
*/
int bcmNet_getRouteInfoToServer(const char *serverIp, char* intfName,
                                char *intfIp, int *upSpeed, int *downSpeed);


/* This function gets the uboot ipaddr/netmask environment parameters.
 * If the netmaks isn't specified, it will assigned it accordingly.
 * e.g.  192.168.1.x->255.255.255.0; 10.x.x.x->255.0.0.0; 172.x->255.240.0.0; others->255.255.0.0
 * If failed to get the values from uboot, it assigned the default setting which is 
 * 192.168.1.1/255.255.255.0
 */
void bcmNet_getDefaultLanIpInfo(char *ipaddr, int iplen, char* netmask, int masklen);

#if defined __cplusplus
};
#endif
#endif  /* __BCM_NET_H__ */
