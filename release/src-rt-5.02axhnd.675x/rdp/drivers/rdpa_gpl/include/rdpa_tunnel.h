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

/** \defgroup Tunnel
 * @{
 */

#ifdef XRDP
#define RDPA_MAX_TUNNEL_HEADER_LEN	96
#define RDPA_MAX_TUNNELS 2
#else
#define RDPA_MAX_TUNNEL_HEADER_LEN	80
#define RDPA_MAX_TUNNELS 1
#endif


/** Tunnel Type */
typedef enum
{
    rdpa_tunnel_l2gre,
    rdpa_tunnel_l3gre,
    rdpa_tunnel_dslite,
} rdpa_tunnel_type;

/** Tunnel configuration .\n */
typedef struct
{
    rdpa_tunnel_type  tunnel_type;               /**< Tunnel type */
    uint8_t           tunnel_header_length;      /**< Tunnel header length */
    bdmf_ip_t         local_ip;                  /**< Local IP: used for DS packet tunnel validation */
    uint8_t           layer3_offset;             /**< Layer3 offset: used for US packet modification */
    uint8_t           gre_proto_offset;          /**< GRE protocol offset: used for US packet modification (L3_GRE, IPv6 payload) */   
    uint8_t           tunnel_header[RDPA_MAX_TUNNEL_HEADER_LEN];
} rdpa_tunnel_cfg_t;

/* @} end of tunnel Doxygen group */

#endif /* _RDPA_TUNNEL_H_ */
