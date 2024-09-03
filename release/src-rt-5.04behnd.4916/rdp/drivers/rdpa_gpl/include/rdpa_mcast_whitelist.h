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

#include "rdpa_mcast_basic.h"

/** \defgroup mcast Multicast Whitelist Management
 * @{
 */

/** The maximum number of multicast whitelist entry */
#define RDPA_MCAST_MAX_WHITELIST       1024

/** Multicast whitelist definition.\n
 */
typedef struct rdpa_iptv_channel_key rdpa_mcast_whitelist_t;

/** Multicast whitelist global statstics */
typedef struct {
    uint32_t rx_pkt;        /**< received packets */
    uint64_t rx_byte;       /**< received bytes */
    uint32_t dropped_pkt;   /**< dropped packets */
    uint64_t dropped_byte;   /**< dropped bytes */
} rdpa_mcast_whitelist_stat_t;

#endif /* _RDPA_MCAST_WHITELIST_H_ */
