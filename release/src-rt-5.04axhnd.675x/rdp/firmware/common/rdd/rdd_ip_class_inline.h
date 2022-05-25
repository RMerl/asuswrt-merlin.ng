/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
      All Rights Reserved
   
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
