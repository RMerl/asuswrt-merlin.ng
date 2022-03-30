/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
   :>
 */
#ifndef _RDD_IP_CLASS_INLINE_H
#define _RDD_IP_CLASS_INLINE_H

#include "rdd_defs.h"

static inline uint32_t rdd_checksum_delta_calc(uint16_t checksum_delta,
    uint16_t old_value, uint16_t new_value)
{
    uint32_t calc_delta;

    calc_delta = checksum_delta + (~old_value & 0xFFFF);
    calc_delta += calc_delta >> 16 & 0xFFFF;
    calc_delta &= 0xFFFF;

    calc_delta += new_value;
    calc_delta += calc_delta >> 16 & 0xFFFF;
    calc_delta &= 0xFFFF;

    return calc_delta;
}

static inline void rdd_connection_checksum_delta_calc(rdpa_ip_flow_key_t *key, rdd_fc_context_t *fc_ctx)
{
    uint32_t calc_delta;
    uint32_t nonat_ipv4_addr;
    uint16_t nonat_port;

    if (key->dir == rdpa_dir_ds)
    {
        nonat_ipv4_addr = key->dst_ip.addr.ipv4;
        nonat_port = key->dst_port;
    }
    else
    {
        nonat_ipv4_addr = key->src_ip.addr.ipv4;
        nonat_port = key->src_port;
    }

    /* IP checksum delta */
    calc_delta = rdd_checksum_delta_calc(0, nonat_ipv4_addr, fc_ctx->nat_ip.addr.ipv4);
    calc_delta = rdd_checksum_delta_calc(calc_delta, nonat_ipv4_addr >> 16,
        fc_ctx->nat_ip.addr.ipv4 >> 16);
    fc_ctx->ip_checksum_delta = calc_delta;

    /* TCP/UDP checksum delta */
    calc_delta = rdd_checksum_delta_calc(calc_delta, nonat_port, fc_ctx->nat_port);
    fc_ctx->l4_checksum_delta = calc_delta;
}

#endif /* RDD_IP_CLASS_INLINE_H_ */
