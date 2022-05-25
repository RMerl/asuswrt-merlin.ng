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

#ifdef XRDP
#define RDPA_PORT_TO_CTX_IDX(rdpa_port)  ((!(rdpa_if_id(rdpa_port) & RDPA_PORT_ALL_WLAN)) ? rdpa_port : rdpa_if_lan_max + rdpa_port - rdpa_if_wlan0)
#else
#define RDPA_PORT_TO_CTX_IDX(port)       (port)
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
