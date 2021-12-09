/***********************************************************************
 *
 *  Copyright (c) 2019  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2018:DUAL/GPL:standard
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


#if defined __cplusplus
};
#endif
#endif  /* __BCM_NET_H__ */
