/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
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
* :> 
*/

#ifndef _RDPA_MCAST_WHITELIST_H_
#define _RDPA_MCAST_WHITELIST_H_

/** \defgroup mcast Multicast Whitelist Management
 * @{
 */

/** The maximum number of multicast whitelist entry */
#define RDPA_MCAST_MAX_WHITELIST       1024

/** Multicast whitelist definition.\n
 */
typedef struct {
    bdmf_ip_t src_ip;       /**< Source IP address*/
    bdmf_ip_t dst_ip;       /**< Destination IP address*/
    uint8_t check_src_ip;   /**< Check Source IP for SSM mode */
    uint8_t num_vlan_tags;  /**< Number of VLAN Tags */
    uint16_t outer_vlan_id; /**< Outer VLAN ID */
    uint16_t inner_vlan_id; /**< Inner VLAN ID */
} rdpa_mcast_whitelist_t;

/** Multicast whitelist global statstics */
typedef struct {
    uint32_t rx_pkt;        /**< received packets */
    uint32_t rx_byte;       /**< received bytes */
    uint32_t dropped_pkt;   /**< dropped packets */
} rdpa_mcast_whitelist_stat_t;

#endif /* _RDPA_MCAST_WHITELIST_H_ */
