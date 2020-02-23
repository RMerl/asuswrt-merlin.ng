/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
 :>
*/

/*
 * RDPA common helpers -for driver use only
 */

#ifndef _RDPA_RDD_MAP_LEGACY_H_
#define _RDPA_RDD_MAP_LEGACY_H_

#include "bdmf_dev.h"
#include "bdmf_errno.h"
#include "rdd.h"
#include "rdd_legacy_conv.h"

#define RDPA_WIFI_SSID_INVALID   (rdpa_if_ssid15 - rdpa_if_ssid0 + 1)
#define RDPA_VLAN_AGGR_ENTRY_DONT_CARE       0xFF

BL_LILAC_RDD_BRIDGE_PORT_DTE rdpa_if_to_rdd_bridge_mcast_port(rdpa_if port);
BL_LILAC_RDD_BRIDGE_PORT_DTE rdpa_ports2rdd_bridge_mcast_ports_mask(rdpa_ports ports);
rdpa_ports rdd_bridge_mcast_ports2rdpa_ports(BL_LILAC_RDD_BRIDGE_PORT_DTE rdd_bridge_mcast_ports);

rdpa_ports rdpa_rdd_bridge_port_mask_to_ports(BL_LILAC_RDD_BRIDGE_PORT_DTE bridge_port, uint8_t wifi_ssid);
BL_LILAC_RDD_BRIDGE_PORT_DTE rdpa_ports_to_rdd_bridge_port_mask(rdpa_ports ports, uint8_t *wifi_ssid);
BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE rdpa_if_to_rdd_bridge_port_vector(rdpa_if port);


BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE rdpa_ports2rdd_bridge_port_vector(rdpa_ports ports);
rdpa_ports rdpa_rdd_bridge_port_vector2rdpa_ports(BL_LILAC_RDD_BRIDGE_PORT_VECTOR_DTE rdd_vector);

uint32_t rdpa_ports_to_rdd_egress_port_vector(rdpa_ports ports, bdmf_boolean is_iptv);
rdpa_ports rdpa_rdd_egress_port_vector_to_ports(uint32_t egress_port_vector, bdmf_boolean is_iptv);

BL_LILAC_RDD_ETHER_TYPE_FILTER_NUMBER_DTE rdpa_filter_to_rdd_etype_filter(rdpa_filter filter);

int emac_id2cr_dev(rdpa_emac src);
int wan_type2serdes_wan_type(rdpa_wan_type src);
int emac2serdes_emac(rdpa_emac src);
int emac_mode2serdes_emac_mode(rdpa_emac_mode src);
int emac_mode2vps_emac_mode(rdpa_emac_mode src);
int rdpa_wan_emac2rdd_phys_port(rdpa_emac_mode src);
int rdpa_emac2bpm_emac(rdpa_emac src);
int rdpa_emac2bbh_emac(rdpa_emac src);
int rdpa_emac2rdd_emac(rdpa_emac src);
int rdpa_emac2rdd_eth_thread(rdpa_emac src);
int rdpa_dest_cpu2rdd_direct_q(rdpa_emac src);
rdpa_emac bbh_emac2_emac(int src);
int rdpa_if_to_bbh_emac(rdpa_if port);
int rdpa_wan_type2rdd_egress_phy(rdpa_wan_type src);
rdpa_wan_type rdd_egress_phy2rdpa_wan_type(int src);

int rdpa_port_to_ih_class_lookup(rdpa_if port);
int emac_id2rdd_bridge(rdpa_emac src);
int rdd_bridge2emac_id(rdpa_emac src);

/* TODO - fix compilation problem when using RDD_FLOW_CACHE_FORWARD_ACTION, instead of int*/
extern const int rdpa_fwd_act2rdd_fc_fwd_act[];
#endif
