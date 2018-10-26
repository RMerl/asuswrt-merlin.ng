/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
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

#ifndef _DRV_RUNNER_BRIDGE_H
#define _DRV_RUNNER_BRIDGE_H

void rdd_egress_ethertype_config(uint16_t ether_type_1, uint16_t ether_type_2);
void rdd_vlan_command_always_egress_ether_type_config(uint16_t ether_type_3);
int rdd_ds_pbits_to_qos_entry_cfg(rdd_rdd_vport virtual_port, uint32_t pbits, rdd_tx_queue_id_t qos);
int rdd_ds_tc_to_queue_entry_cfg(rdd_rdd_vport virtual_port, uint8_t tc, rdd_tx_queue_id_t queue);
int rdd_dscp_to_pbits_global_cfg(uint32_t dscp, uint32_t pbits);
int rdd_pbits_to_pbits_config(uint32_t table_number, uint32_t input_pbits, uint32_t output_pbits);

#if !defined(WL4908)
int rdd_tpid_overwrite_table_cfg(uint16_t *tpid_overwrite_array, rdpa_traffic_dir direction);
int rdd_vlan_cmd_cfg(rdpa_traffic_dir direction, rdd_vlan_cmd_param_t *vlan_command_params);
#endif
#endif /* _DRV_RUNNER_BRIDGE_H */

