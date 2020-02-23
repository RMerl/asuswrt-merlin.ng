/*
   Copyright (c) 2016 Broadcom
   All Rights Reserved

    <:label-BRCM:2016:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#ifndef _RDD_TUNNELS_PARSING_6838_H
#define _RDD_TUNNELS_PARSING_6838_H

void rdd_oren_tunnels_parsing_enable(bdmf_boolean  xi_tunneling_enable);
BL_LILAC_RDD_ERROR_DTE rdd_dual_stack_lite_tunnel_config(bdmf_ipv6_t *xi_ipv6_src_ip, bdmf_ipv6_t *xi_ipv6_dst_ip);

BL_LILAC_RDD_ERROR_DTE rdd_tunnel_cfg_get(uint32_t tunnel_idx, RDD_TUNNEL_ENTRY_DTS *tunnel_entry);
BL_LILAC_RDD_ERROR_DTE rdd_tunnel_cfg_set(uint32_t tunnel_idx, RDD_TUNNEL_ENTRY_DTS *tunnel_entry);

#endif

