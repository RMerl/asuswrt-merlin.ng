/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
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

#ifndef _DRV_RUNNER_BRIDGE_H
#define _DRV_RUNNER_BRIDGE_H

void rdd_egress_ethertype_config(uint16_t ether_type_1, uint16_t ether_type_2);
void rdd_vlan_command_always_egress_ether_type_config(uint16_t ether_type_3);
int rdd_ds_pbits_to_qos_entry_cfg(rdd_rdd_vport virtual_port, uint32_t pbits, rdd_tx_queue_id_t qos);
int rdd_ds_tc_to_queue_entry_cfg(rdd_rdd_vport virtual_port, uint8_t tc, rdd_tx_queue_id_t queue);
int rdd_dscp_to_pbits_global_cfg(uint32_t dscp, uint32_t pbits);
int rdd_pbits_to_pbits_config(uint32_t table_number, uint32_t input_pbits, uint32_t output_pbits);

#if defined(DSL_63138) || defined(DSL_63148)
int rdd_tpid_overwrite_table_cfg(uint16_t *tpid_overwrite_array, rdpa_traffic_dir direction);
int rdd_vlan_cmd_cfg(rdpa_traffic_dir direction, rdd_vlan_cmd_param_t *vlan_command_params);
#endif
#endif /* _DRV_RUNNER_BRIDGE_H */

