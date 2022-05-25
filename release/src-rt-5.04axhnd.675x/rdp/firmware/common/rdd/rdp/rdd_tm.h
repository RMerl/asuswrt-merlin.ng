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

#ifndef _RDD_TM_H
#define _RDD_TM_H

#ifdef DS_SRAM_TX_QUEUES
void rdd_ds_free_packet_descriptors_pool_init(void);
#endif
#ifdef US_SRAM_TX_QUEUES
void rdd_us_free_packet_descriptors_pool_init(void);
#endif
int rdd_wan_tx_init(void);
void rdd_eth_tx_init(void);
int rdd_wan_channel_cfg(rdd_wan_channel_id_t wan_channel, rdd_wan_channel_schedule_t schedule_mode,
    rdd_peak_schedule_mode_t  peak_schedule_mode);
int rdd_rate_cntrl_cfg(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_rate_cntrl_params_t *rate_cntrl_params);
#if defined(WL4908)
int rdd_rate_cntrl_modify(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_rate_cntrl_params_t *rate_cntrl_params);
void rdd_rate_controller_sustain_budget_limit_config(uint32_t  sustain_budget_limit);
void rdd_us_overall_rate_limiter_cfg(rdd_rate_limit_params_t *budget);
int rdd_wan_channel_rate_limiter_cfg(rdd_wan_channel_id_t channel_id, bdmf_boolean rate_limiter_enabled,
    rdpa_tm_orl_prty prio);
#endif
int rdd_rate_cntrl_remove(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl);
int rdd_wan_tx_queue_cfg(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_tx_queue_id_t tx_queue, uint16_t packet_threshold, rdd_queue_profile_id_t profile_id, uint8_t counter_id);
int rdd_wan_tx_queue_modify(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_tx_queue_id_t tx_queue, uint16_t packet_threshold, rdd_queue_profile_id_t profile_id, uint8_t counter_id);
int rdd_wan_tx_queue_remove(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_tx_queue_id_t tx_queue);

void rdd_wan_tx_queue_get_status(rdd_wan_channel_id_t wan_channel, rdd_rate_cntrl_id_t rate_cntrl,
    rdd_tx_queue_id_t tx_queue, 
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
    uint16_t *number_of_packets
#else
    rdpa_stat_1way_t *stat 
#endif
				 );
int rdd_lan_vport_cfg(rdd_vport_id_t port, rdd_rate_limiter_t rate_limiter);
int rdd_lan_vport_tx_queue_cfg(rdd_vport_id_t port, rdd_tx_queue_id_t queue_id, uint16_t packet_threshold,
    rdd_queue_profile_id_t profile_id);
int rdd_lan_vport_tx_queue_status_get(rdd_vport_id_t port, rdd_tx_queue_id_t queue_id,
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    rdpa_stat_1way_t *stat 
#else
    uint16_t *number_of_packets
#endif
				      );
void rdd_eth_tx_ddr_queue_addr_cfg(rdd_emac_id_t emac_id, rdd_tx_queue_id_t queue_id, uint32_t ddr_addr, uint16_t queue_size, uint8_t counter_id);
void rdd_queue_profile_cfg(rdpa_traffic_dir direction, rdd_queue_profile_id_t profile_id,
    rdd_queue_profile_t *queue_profile);
void rdd_drop_precedence_cfg(rdpa_traffic_dir direction, uint16_t eligibility_vector);
int rdd_mdu_mode_pointer_get(rdd_emac_id_t emac_id, uint16_t *mdu_mode_ptr);
int rdd_wan_tx_queue_flush(rdd_wan_channel_id_t wan_channel_id, rdd_rate_cntrl_id_t rate_controller_id,
    rdd_tx_queue_id_t queue_id, bdmf_boolean is_wait);
int rdd_eth_tx_queue_flush(rdd_vport_id_t port, rdd_tx_queue_id_t queue_id, bdmf_boolean is_wait);
int rdd_wan_channel_byte_counter_read(rdd_wan_channel_id_t wan_channel_id, uint32_t *byte_counter);
int rdd_flow_control_send_xon(rdd_vport_id_t vport);

int rdd_free_packet_descriptors_pool_size_get ( uint32_t  *downstream_size,
						  uint32_t  *upstream_size );

#endif /* _RDD_TM_H */
