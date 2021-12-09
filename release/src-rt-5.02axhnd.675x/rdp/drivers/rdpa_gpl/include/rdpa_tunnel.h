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
