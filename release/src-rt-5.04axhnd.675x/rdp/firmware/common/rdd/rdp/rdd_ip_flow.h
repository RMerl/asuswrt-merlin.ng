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
