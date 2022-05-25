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

#ifndef _RDD_IP_FLOW_H
#define _RDD_IP_FLOW_H

#include "rdd_ip_class_inline.h"

void rdd_fc_context_entry_write(rdd_fc_context_t *ctx, RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry);
void rdd_fc_context_entry_read(rdd_fc_context_t *ctx, RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry);

int rdd_context_entry_get(uint32_t context_index, void *context_entry);
int rdd_context_entry_modify(uint32_t context_index, void *context_entry);
int rdd_clean_context_aging_get(uint32_t context_index, uint8_t *aging_status);

int rdd_ip_flow_add(rdd_module_t *module, rdpa_ip_flow_key_t *key, void *context, uint32_t *context_index);
int rdd_ip_flow_delete(rdd_module_t *module, uint32_t context_index);
int rdd_ip_flow_find(rdd_module_t *module, rdpa_ip_flow_key_t *key, uint32_t *context_index);
int rdd_ip_flow_get(rdd_module_t *module, uint32_t ip_flow_index, rdpa_ip_flow_key_t *key, uint32_t *context_index);
int rdd_ip_flow_counters_get(uint32_t entry_index, uint32_t *hit_count, uint32_t *byte_count);

int rdd_ds_lite_tunnel_cfg(rdpa_ds_lite_tunnel_id dual_stack_lite_tunnel_id, bdmf_ipv6_t *ipv6_src_ip,
    bdmf_ipv6_t *ipv6_dst_ip);
#endif /* _RDD_IP_FLOW_H */
