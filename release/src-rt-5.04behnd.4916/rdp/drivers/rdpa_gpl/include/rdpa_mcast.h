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

#ifndef _RDPA_MCAST_H_
#define _RDPA_MCAST_H_

#include "rdpa_cmd_list.h"

/** \defgroup mcast Multicast Flow Management
 * @{
 */

#include "rdpa_mcast_basic.h"
#if !defined(CONFIG_BCM_PON)
#include "rdpa_ucast.h"
#endif

/* Multi-flow implementation uses ucast result and different key */
typedef rdpa_fc_mcast_flow_key_t    rdpa_mcast_flow_key_t;
typedef rdpa_ip_flow_result_t       rdpa_mcast_flow_result_t;


/** Multicast flow definition (key + result).\n
 */
typedef struct {
    uint32_t  hw_flow_id;               /**< Multicast flow HW flow ID */
    rdpa_mcast_flow_key_t key;          /**< Multicast flow key */
    rdpa_mcast_flow_result_t result;    /**< Multicast flow result */
} rdpa_mcast_flow_t;

/** @} end of mcast Doxygen group. */
#if !defined(BCM_PON_XRDP) || defined(RDP_UFC)
typedef rdpa_mcast_flow_t rdpa_fc_mcast_flow_t;
#endif

#endif /* _RDPA_MCAST_H_ */
