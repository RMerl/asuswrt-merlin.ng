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
* :>
*/

/*
 * RDPA common helpers -for driver use only
 */

#ifndef _RDPA_RDD_MAP_H_
#define _RDPA_RDD_MAP_H_

#include "bdmf_dev.h"
#include "bdmf_errno.h"
#include "rdd_defs.h"

#define RDPA_WIFI_SSID_INVALID (rdpa_if_ssid15 - rdpa_if_ssid0 + 1)
#define RDPA_VLAN_AGGR_ENTRY_DONT_CARE 0xFF

#ifndef G9991
rdd_port_profile_t rdpa_if2rdd_port_profile(rdpa_traffic_dir dir, rdpa_if port);
#endif

/* XXX: These are stubs, which are not implemented yet */
rdd_vport_vector_t rdpa_ports2rdd_vport_vector(rdpa_ports ports);
rdpa_ports rdd_vport_vector2rdpa_ports(rdd_vport_vector_t vport_vector);

uint32_t rdpa_ports_to_rdd_egress_port_vector(rdpa_ports ports, bdmf_boolean is_iptv);
rdpa_ports rdpa_rdd_egress_port_vector_to_ports(uint32_t egress_port_vector, bdmf_boolean is_iptv);

rdd_ether_type_filter_t rdpa_filter2rdd_etype_filter(rdpa_filter filter);

rdd_emac_id_vector_t rdpa_if_to_rdd_emac_id_vector(rdpa_if port);
rdpa_ports rdpa_rdd_emac_id_vector2rdpa_ports(rdd_emac_id_vector_t rdd_vector);
rdd_emac_id_vector_t rdpa_ports2rdd_emac_id_vector(rdpa_ports ports);

#if 0
int emac_id2cr_dev(rdpa_emac src);
int wan_type2serdes_wan_type(rdpa_wan_type src);
int emac2serdes_emac(rdpa_emac src);
int emac_mode2serdes_emac_mode(rdpa_emac_mode src);
int emac_mode2vps_emac_mode(rdpa_emac_mode src);
int rdpa_wan_emac2rdd_phys_port(rdpa_emac_mode src);
int rdpa_emac2bpm_emac(rdpa_emac src);
#endif
int rdpa_emac2bbh_emac(rdpa_emac src);
int rdpa_emac2rdd_emac(rdpa_emac src);
#if 0
int rdpa_emac2rdd_eth_thread(rdpa_emac src);
#endif
int rdpa_dest_cpu2rdd_direct_q(rdpa_emac src);
#if 0
rdpa_emac bbh_emac2_emac(int src);
#endif
int rdpa_if_to_bbh_emac(rdpa_if port);
int rdpa_port_to_ih_class_lookup(rdpa_if port);
#if 0
int emac_id2rdd_bridge(rdpa_emac src);
int rdd_bridge2emac_id(rdpa_emac src);

#endif
/* TODO - fix compilation problem when using RDD_FLOW_CACHE_FORWARD_ACTION, instead of int */
extern const int rdpa_fwd_act2rdd_fc_fwd_act[];

#endif
