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


#ifndef _RDPA_TUNNEL_H_
#define _RDPA_TUNNEL_H_

#include <bdmf_interface.h>

/** \defgroup tunnel Tunnel Object
 * Tunnel object use do configure accelration parameters
 * for the supported \ref rdpa_tunnel_type "tunnel types"
 *
 * @{
 */

#ifdef XRDP
#define RDPA_MAX_TUNNEL_HEADER_LEN	96
#ifdef RULE_BASED_GRE
#define RDPA_MAX_TUNNELS 16
#else
#define RDPA_MAX_TUNNELS 4
#endif
#else
#define RDPA_MAX_TUNNEL_HEADER_LEN	80
#define RDPA_MAX_TUNNELS 1
#endif


/** Tunnel Types */
typedef enum
{
    rdpa_tunnel_l2gre,                          /**< L2 GRE  tunnel */
    rdpa_tunnel_l3gre,                          /**< L3 GRE  tunnel */
    rdpa_tunnel_dslite,                         /**< DS Lite tunnel */
    rdpa_tunnel_vxlan = 4,                      /**< VxLAN   tunnel */
    rdpa_tunnel_l2tp = 8                        /**< L2TP    tunnel */ 

} rdpa_tunnel_type;

/** Tunnel configuration .\n */
typedef struct
{
    rdpa_tunnel_type  tunnel_type;               /**< Tunnel type */
    uint8_t           tunnel_header_length;      /**< Tunnel header length */
    bdmf_ip_t         local_ip;                  /**< Local IP: used for DS packet tunnel validation */
    uint8_t           layer3_offset;             /**< Layer3 offset: used for US packet modification */
    uint8_t           gre_proto_offset;          /**< GRE protocol offset: used for US packet modification (L3_GRE, IPv6 payload) */   
    uint8_t           layer4_offset;             /** UDP header offset -  (VxLAN) */ 
    uint32_t          tunnel_info;               /**< VXLAN protocol:  VNI - extended VLAN ID.  */ 
    uint8_t           tunnel_header[RDPA_MAX_TUNNEL_HEADER_LEN]; /**< Tunnel header template. */
#if defined(CONFIG_RNR_L2TP_HW_ACCEL_4GEN) 
    uint8_t           l2tp_header_size;          /** L2TP header size */
    uint8_t           ppp_header_size;           /** PPP header size */
#endif
} rdpa_tunnel_cfg_t;

/* @} end of tunnel Doxygen group */

#endif /* _RDPA_TUNNEL_H_ */
