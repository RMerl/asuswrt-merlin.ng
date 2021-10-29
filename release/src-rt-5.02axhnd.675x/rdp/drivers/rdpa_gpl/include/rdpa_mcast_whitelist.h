/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
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
