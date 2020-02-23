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

#ifndef _RDD_COMMON_H
#define _RDD_COMMON_H

#include "rdd_platform.h"
#include "rdd_crc.h"

static inline uint32_t rdd_budget_to_alloc_unit(uint32_t budget, uint32_t period, uint32_t exponent)
{
    return ((budget + ((1000000 / period) / 2)) / (1000000 / period)) >> exponent;
}

static inline uint32_t rdd_get_exponent(uint32_t value, uint32_t mantissa_len, uint32_t exponent_list_len,
    uint32_t *exponent_list)
{
    uint32_t  i;

    for (i = exponent_list_len - 1; i > 0; i--)
    {
        if (value > (((1 << mantissa_len) - 1) << exponent_list[i - 1]))
            return i;
    }

    return 0;
}

void rdd_ddr_packet_headroom_size_cfg(uint32_t ddr_packet_headroom_size);
int rdd_packet_headroom_size_cfg(uint32_t ddr_packet_headroom_size, uint32_t psram_packet_headroom_size);
int rdd_4_bytes_counter_get(uint32_t counter_group, uint32_t counter_num, uint32_t *counter);
int rdd_2_bytes_counter_get(uint32_t counter_group, uint32_t counter_num, uint16_t *counter);
void rdd_us_padding_cfg(bdmf_boolean control_enabled, bdmf_boolean cpu_control_enabled, uint16_t size);
int rdd_mtu_cfg(uint16_t mtu_size);
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
int rdd_ds_wan_flow_cfg(uint32_t wan_flow, rdpa_cpu_reason cpu_reason, bdmf_boolean is_pkt_based,
    uint8_t ingress_flow);
void rdd_prop_tag_vport_cfg(rdd_runner_group_t *group, rdpa_traffic_dir dir, uint32_t vector);
#endif /*DSL*/
void rdd_us_wan_flow_cfg(uint32_t wan_flow, rdd_wan_channel_id_t wan_channel, uint32_t wan_port,
    bdmf_boolean crc_calc_en, bdmf_boolean ptm_bonding_enabled, uint8_t pbits_to_queue_table_idx,
    uint8_t tc_to_queue_table_idx);
void rdd_us_wan_flow_get(uint32_t wan_flow, rdd_wan_channel_id_t *wan_channel, uint32_t *wan_port,
    bdmf_boolean *crc_calc_en, uint8_t *pbits_to_queue_table_idx, uint8_t *tc_to_queue_table_idx);
void rdd_dscp_to_pbits_cfg(rdpa_traffic_dir direction, rdd_vport_id_t vport_id, uint32_t dscp, uint32_t pbits);
void rdd_reverse_ffi_table_init(void);
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
void rdd_layer2_header_copy_mapping_init(void);
#endif /*DSL*/
int rdd_timer_task_config(rdpa_traffic_dir direction, uint16_t task_period_in_usec, uint16_t firmware_routine_address_id);
void rdd_interrupt_mask(uint32_t intr, uint32_t sub_intr);
void rdd_interrupt_unmask(uint32_t intr, uint32_t sub_intr);
void rdd_interrupt_vector_get(uint32_t intr, uint8_t *sub_intr_vector);
void rdd_interrupt_clear(uint32_t intr, uint32_t sub_intr);
void rdd_interrupt_mask_get(uint32_t intr, uint8_t *sub_intr_mask);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
int rdd_ipv4_host_address_table_set(uint32_t table_index, 
                                    bdmf_ipv4 ipv4_host_addr,
                                    uint16_t ref_count);
int rdd_ipv4_host_address_table_get(uint32_t table_index, 
                                    bdmf_ipv4 *ipv4_host_addr,
                                    uint16_t *ref_count);
int rdd_ipv6_host_address_table_set(uint32_t table_index, 
                                    const bdmf_ipv6_t *ipv6_host_addr,
                                    uint16_t ref_count);
int rdd_ipv6_host_address_table_get(uint32_t table_index, 
                                    bdmf_ipv6_t *ipv6_host_addr,
                                    uint16_t *ref_count);
int rdd_lan_get_stats(uint32_t lan_port,
                      uint32_t *rx_packet,
                      uint32_t *tx_packet,
                      uint16_t *tx_discard);
#endif /*DSL*/
#endif /* _RDD_COMMON_H */
