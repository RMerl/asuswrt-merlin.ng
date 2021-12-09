/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef _BCM6858_LPORT_XLMAC_AG_H_
#define _BCM6858_LPORT_XLMAC_AG_H_

#include "access_macros.h"
#include "bcmtypes.h"
typedef struct
{
    uint8_t extended_hig2_en;
    uint8_t link_status_select;
    uint8_t sw_link_status;
    uint8_t xgmii_ipg_check_disable;
    uint8_t rs_soft_reset;
    uint8_t rsvd_5;
    uint8_t local_lpbk_leak_enb;
    uint8_t rsvd_4;
    uint8_t soft_reset;
    uint8_t lag_failover_en;
    uint8_t remove_failover_lpbk;
    uint8_t rsvd_1;
    uint8_t local_lpbk;
    uint8_t rx_en;
    uint8_t tx_en;
} lport_xlmac_ctrl;

typedef struct
{
    uint8_t tx_threshold;
    uint8_t ep_discard;
    uint8_t tx_preamble_length;
    uint8_t throt_denom;
    uint8_t throt_num;
    uint8_t average_ipg;
    uint8_t pad_threshold;
    uint8_t pad_en;
    uint8_t tx_any_start;
    uint8_t discard;
    uint8_t crc_mode;
} lport_xlmac_tx_ctrl;

typedef struct
{
    uint8_t rx_pass_pfc;
    uint8_t rx_pass_pause;
    uint8_t rx_pass_ctrl;
    uint8_t rsvd_3;
    uint8_t rsvd_2;
    uint8_t runt_threshold;
    uint8_t strict_preamble;
    uint8_t strip_crc;
    uint8_t rx_any_start;
    uint8_t rsvd_1;
} lport_xlmac_rx_ctrl;

typedef struct
{
    uint8_t reset_flow_control_timers_on_link_down;
    uint8_t drop_tx_data_on_link_interrupt;
    uint8_t drop_tx_data_on_remote_fault;
    uint8_t drop_tx_data_on_local_fault;
    uint8_t link_interruption_disable;
    uint8_t use_external_faults_for_tx;
    uint8_t remote_fault_disable;
    uint8_t local_fault_disable;
} lport_xlmac_rx_lss_ctrl;

typedef struct
{
    uint16_t pause_xoff_timer;
    uint8_t rsvd_2;
    uint8_t rsvd_1;
    uint8_t rx_pause_en;
    uint8_t tx_pause_en;
    uint8_t pause_refresh_en;
    uint16_t pause_refresh_timer;
} lport_xlmac_pause_ctrl;

typedef struct
{
    uint8_t tx_pfc_en;
    uint8_t rx_pfc_en;
    uint8_t pfc_stats_en;
    uint8_t rsvd;
    uint8_t force_pfc_xon;
    uint8_t pfc_refresh_en;
    uint16_t pfc_xoff_timer;
    uint16_t pfc_refresh_timer;
} lport_xlmac_pfc_ctrl;

typedef struct
{
    uint8_t llfc_img;
    uint8_t no_som_for_crc_llfc;
    uint8_t llfc_crc_ignore;
    uint8_t llfc_cut_through_mode;
    uint8_t llfc_in_ipg_only;
    uint8_t rx_llfc_en;
    uint8_t tx_llfc_en;
} lport_xlmac_llfc_ctrl;

typedef struct
{
    uint8_t link_status;
    uint8_t rx_pkt_overflow;
    uint8_t tx_ts_fifo_overflow;
    uint8_t tx_llfc_msg_overflow;
    uint8_t rsvd_2;
    uint8_t tx_pkt_overflow;
    uint8_t tx_pkt_underflow;
    uint8_t rx_msg_overflow;
    uint8_t rsvd_1;
} lport_xlmac_fifo_status;

typedef struct
{
    uint8_t clear_rx_pkt_overflow;
    uint8_t clear_tx_ts_fifo_overflow;
    uint8_t clear_tx_llfc_msg_overflow;
    uint8_t rsvd_2;
    uint8_t clear_tx_pkt_overflow;
    uint8_t clear_tx_pkt_underflow;
    uint8_t clear_rx_msg_overflow;
    uint8_t rsvd_1;
} lport_xlmac_clear_fifo_status;

typedef struct
{
    uint8_t e2efc_dual_modid_en;
    uint8_t e2ecc_legacy_imp_en;
    uint8_t e2ecc_dual_modid_en;
    uint8_t honor_pause_for_e2e;
    uint8_t e2e_enable;
} lport_xlmac_e2e_ctrl;

typedef struct
{
    uint8_t sum_ts_entry_valid;
    uint8_t sum_link_interruption_status;
    uint8_t sum_remote_fault_status;
    uint8_t sum_local_fault_status;
    uint8_t sum_rx_cdc_double_bit_err;
    uint8_t sum_rx_cdc_single_bit_err;
    uint8_t sum_tx_cdc_double_bit_err;
    uint8_t sum_tx_cdc_single_bit_err;
    uint8_t sum_rx_msg_overflow;
    uint8_t sum_rx_pkt_overflow;
    uint8_t sum_tx_ts_fifo_overflow;
    uint8_t sum_tx_llfc_msg_overflow;
    uint8_t sum_tx_pkt_overflow;
    uint8_t sum_tx_pkt_underflow;
} lport_xlmac_intr_status;

typedef struct
{
    uint8_t en_ts_entry_valid;
    uint8_t en_link_interruption_status;
    uint8_t en_remote_fault_status;
    uint8_t en_local_fault_status;
    uint8_t en_rx_cdc_double_bit_err;
    uint8_t en_rx_cdc_single_bit_err;
    uint8_t en_tx_cdc_double_bit_err;
    uint8_t en_tx_cdc_single_bit_err;
    uint8_t en_rx_msg_overflow;
    uint8_t en_rx_pkt_overflow;
    uint8_t en_tx_ts_fifo_overflow;
    uint8_t en_tx_llfc_msg_overflow;
    uint8_t en_tx_pkt_overflow;
    uint8_t en_tx_pkt_underflow;
} lport_xlmac_intr_enable;

int ag_drv_lport_xlmac_ctrl_set(uint8_t port_id, const lport_xlmac_ctrl *ctrl);
int ag_drv_lport_xlmac_ctrl_get(uint8_t port_id, lport_xlmac_ctrl *ctrl);
int ag_drv_lport_xlmac_mode_set(uint8_t port_id, uint8_t speed_mode, uint8_t no_sop_for_crc_hg, uint8_t hdr_mode);
int ag_drv_lport_xlmac_mode_get(uint8_t port_id, uint8_t *speed_mode, uint8_t *no_sop_for_crc_hg, uint8_t *hdr_mode);
int ag_drv_lport_xlmac_spare0_set(uint8_t port_id, uint32_t rsvd);
int ag_drv_lport_xlmac_spare0_get(uint8_t port_id, uint32_t *rsvd);
int ag_drv_lport_xlmac_spare1_set(uint8_t port_id, uint8_t rsvd);
int ag_drv_lport_xlmac_spare1_get(uint8_t port_id, uint8_t *rsvd);
int ag_drv_lport_xlmac_tx_ctrl_set(uint8_t port_id, const lport_xlmac_tx_ctrl *tx_ctrl);
int ag_drv_lport_xlmac_tx_ctrl_get(uint8_t port_id, lport_xlmac_tx_ctrl *tx_ctrl);
int ag_drv_lport_xlmac_tx_ctrl_overlay_set(uint8_t port_id, uint16_t xlmac_tx_ctrl_hi, uint32_t xlmac_tx_ctrl_lo);
int ag_drv_lport_xlmac_tx_ctrl_overlay_get(uint8_t port_id, uint16_t *xlmac_tx_ctrl_hi, uint32_t *xlmac_tx_ctrl_lo);
int ag_drv_lport_xlmac_tx_mac_sa_set(uint8_t port_id, uint64_t ctrl_sa);
int ag_drv_lport_xlmac_tx_mac_sa_get(uint8_t port_id, uint64_t *ctrl_sa);
int ag_drv_lport_xlmac_tx_mac_sa_overlay_set(uint8_t port_id, uint16_t sa_hi, uint32_t sa_lo);
int ag_drv_lport_xlmac_tx_mac_sa_overlay_get(uint8_t port_id, uint16_t *sa_hi, uint32_t *sa_lo);
int ag_drv_lport_xlmac_rx_ctrl_set(uint8_t port_id, const lport_xlmac_rx_ctrl *rx_ctrl);
int ag_drv_lport_xlmac_rx_ctrl_get(uint8_t port_id, lport_xlmac_rx_ctrl *rx_ctrl);
int ag_drv_lport_xlmac_rx_mac_sa_set(uint8_t port_id, uint64_t rx_sa);
int ag_drv_lport_xlmac_rx_mac_sa_get(uint8_t port_id, uint64_t *rx_sa);
int ag_drv_lport_xlmac_rx_mac_sa_overlay_set(uint8_t port_id, uint16_t sa_hi, uint32_t sa_lo);
int ag_drv_lport_xlmac_rx_mac_sa_overlay_get(uint8_t port_id, uint16_t *sa_hi, uint32_t *sa_lo);
int ag_drv_lport_xlmac_rx_max_size_set(uint8_t port_id, uint16_t rx_max_size);
int ag_drv_lport_xlmac_rx_max_size_get(uint8_t port_id, uint16_t *rx_max_size);
int ag_drv_lport_xlmac_rx_vlan_tag_set(uint8_t port_id, uint8_t outer_vlan_tag_enable, uint8_t inner_vlan_tag_enable, uint16_t outer_vlan_tag, uint16_t inner_vlan_tag);
int ag_drv_lport_xlmac_rx_vlan_tag_get(uint8_t port_id, uint8_t *outer_vlan_tag_enable, uint8_t *inner_vlan_tag_enable, uint16_t *outer_vlan_tag, uint16_t *inner_vlan_tag);
int ag_drv_lport_xlmac_rx_lss_ctrl_set(uint8_t port_id, const lport_xlmac_rx_lss_ctrl *rx_lss_ctrl);
int ag_drv_lport_xlmac_rx_lss_ctrl_get(uint8_t port_id, lport_xlmac_rx_lss_ctrl *rx_lss_ctrl);
int ag_drv_lport_xlmac_rx_lss_status_get(uint8_t port_id, uint8_t *link_interruption_status, uint8_t *remote_fault_status, uint8_t *local_fault_status);
int ag_drv_lport_xlmac_clear_rx_lss_status_set(uint8_t port_id, uint8_t clear_link_interruption_status, uint8_t clear_remote_fault_status, uint8_t clear_local_fault_status);
int ag_drv_lport_xlmac_clear_rx_lss_status_get(uint8_t port_id, uint8_t *clear_link_interruption_status, uint8_t *clear_remote_fault_status, uint8_t *clear_local_fault_status);
int ag_drv_lport_xlmac_pause_ctrl_set(uint8_t port_id, const lport_xlmac_pause_ctrl *pause_ctrl);
int ag_drv_lport_xlmac_pause_ctrl_get(uint8_t port_id, lport_xlmac_pause_ctrl *pause_ctrl);
int ag_drv_lport_xlmac_pause_ctrl_overlay_set(uint8_t port_id, uint8_t xlmac_pause_ctrl_hi, uint32_t xlmac_pause_ctrl_lo);
int ag_drv_lport_xlmac_pause_ctrl_overlay_get(uint8_t port_id, uint8_t *xlmac_pause_ctrl_hi, uint32_t *xlmac_pause_ctrl_lo);
int ag_drv_lport_xlmac_pfc_ctrl_set(uint8_t port_id, const lport_xlmac_pfc_ctrl *pfc_ctrl);
int ag_drv_lport_xlmac_pfc_ctrl_get(uint8_t port_id, lport_xlmac_pfc_ctrl *pfc_ctrl);
int ag_drv_lport_xlmac_pfc_ctrl_overlay_set(uint8_t port_id, uint8_t llfc_refresh_en, uint16_t llfc_refresh_timer);
int ag_drv_lport_xlmac_pfc_ctrl_overlay_get(uint8_t port_id, uint8_t *llfc_refresh_en, uint16_t *llfc_refresh_timer);
int ag_drv_lport_xlmac_pfc_type_set(uint8_t port_id, uint16_t pfc_eth_type);
int ag_drv_lport_xlmac_pfc_type_get(uint8_t port_id, uint16_t *pfc_eth_type);
int ag_drv_lport_xlmac_pfc_opcode_set(uint8_t port_id, uint16_t pfc_opcode);
int ag_drv_lport_xlmac_pfc_opcode_get(uint8_t port_id, uint16_t *pfc_opcode);
int ag_drv_lport_xlmac_pfc_da_set(uint8_t port_id, uint64_t pfc_macda);
int ag_drv_lport_xlmac_pfc_da_get(uint8_t port_id, uint64_t *pfc_macda);
int ag_drv_lport_xlmac_pfc_da_overlay_set(uint8_t port_id, uint16_t pfc_macda_hi, uint32_t pfc_macda_lo);
int ag_drv_lport_xlmac_pfc_da_overlay_get(uint8_t port_id, uint16_t *pfc_macda_hi, uint32_t *pfc_macda_lo);
int ag_drv_lport_xlmac_llfc_ctrl_set(uint8_t port_id, const lport_xlmac_llfc_ctrl *llfc_ctrl);
int ag_drv_lport_xlmac_llfc_ctrl_get(uint8_t port_id, lport_xlmac_llfc_ctrl *llfc_ctrl);
int ag_drv_lport_xlmac_tx_llfc_msg_fields_set(uint8_t port_id, uint16_t llfc_xoff_time, uint8_t tx_llfc_fc_obj_logical, uint8_t tx_llfc_msg_type_logical);
int ag_drv_lport_xlmac_tx_llfc_msg_fields_get(uint8_t port_id, uint16_t *llfc_xoff_time, uint8_t *tx_llfc_fc_obj_logical, uint8_t *tx_llfc_msg_type_logical);
int ag_drv_lport_xlmac_rx_llfc_msg_fields_set(uint8_t port_id, uint8_t rx_llfc_fc_obj_physical, uint8_t rx_llfc_msg_type_physical, uint8_t rx_llfc_fc_obj_logical, uint8_t rx_llfc_msg_type_logical);
int ag_drv_lport_xlmac_rx_llfc_msg_fields_get(uint8_t port_id, uint8_t *rx_llfc_fc_obj_physical, uint8_t *rx_llfc_msg_type_physical, uint8_t *rx_llfc_fc_obj_logical, uint8_t *rx_llfc_msg_type_logical);
int ag_drv_lport_xlmac_tx_timestamp_fifo_data_get(uint8_t port_id, uint8_t *ts_entry_valid, uint16_t *sequence_id, uint32_t *time_stamp);
int ag_drv_lport_xlmac_tx_timestamp_fifo_status_get(uint8_t port_id, uint8_t *entry_count);
int ag_drv_lport_xlmac_fifo_status_get(uint8_t port_id, lport_xlmac_fifo_status *fifo_status);
int ag_drv_lport_xlmac_clear_fifo_status_set(uint8_t port_id, const lport_xlmac_clear_fifo_status *clear_fifo_status);
int ag_drv_lport_xlmac_clear_fifo_status_get(uint8_t port_id, lport_xlmac_clear_fifo_status *clear_fifo_status);
int ag_drv_lport_xlmac_lag_failover_status_get(uint8_t port_id, uint8_t *rsvd, uint8_t *lag_failover_loopback);
int ag_drv_lport_xlmac_eee_ctrl_set(uint8_t port_id, uint8_t rsvd, uint8_t eee_en);
int ag_drv_lport_xlmac_eee_ctrl_get(uint8_t port_id, uint8_t *rsvd, uint8_t *eee_en);
int ag_drv_lport_xlmac_eee_timers_set(uint8_t port_id, uint16_t eee_ref_count, uint16_t eee_wake_timer, uint32_t eee_delay_entry_timer);
int ag_drv_lport_xlmac_eee_timers_get(uint8_t port_id, uint16_t *eee_ref_count, uint16_t *eee_wake_timer, uint32_t *eee_delay_entry_timer);
int ag_drv_lport_xlmac_eee_timers_overlay_set(uint8_t port_id, uint32_t xlmac_eee_timers_hi, uint32_t xlmac_eee_timers_lo);
int ag_drv_lport_xlmac_eee_timers_overlay_get(uint8_t port_id, uint32_t *xlmac_eee_timers_hi, uint32_t *xlmac_eee_timers_lo);
int ag_drv_lport_xlmac_eee_1_sec_link_status_timer_set(uint8_t port_id, uint32_t one_second_timer);
int ag_drv_lport_xlmac_eee_1_sec_link_status_timer_get(uint8_t port_id, uint32_t *one_second_timer);
int ag_drv_lport_xlmac_higig_hdr_0_set(uint8_t port_id, uint64_t higig_hdr_0);
int ag_drv_lport_xlmac_higig_hdr_0_get(uint8_t port_id, uint64_t *higig_hdr_0);
int ag_drv_lport_xlmac_higig_hdr_0_overlay_set(uint8_t port_id, uint32_t higig_hdr_0_hi, uint32_t higig_hdr_0_lo);
int ag_drv_lport_xlmac_higig_hdr_0_overlay_get(uint8_t port_id, uint32_t *higig_hdr_0_hi, uint32_t *higig_hdr_0_lo);
int ag_drv_lport_xlmac_higig_hdr_1_set(uint8_t port_id, uint64_t higig_hdr_1);
int ag_drv_lport_xlmac_higig_hdr_1_get(uint8_t port_id, uint64_t *higig_hdr_1);
int ag_drv_lport_xlmac_higig_hdr_1_overlay_set(uint8_t port_id, uint32_t higig_hdr_1_hi, uint32_t higig_hdr_1_lo);
int ag_drv_lport_xlmac_higig_hdr_1_overlay_get(uint8_t port_id, uint32_t *higig_hdr_1_hi, uint32_t *higig_hdr_1_lo);
int ag_drv_lport_xlmac_gmii_eee_ctrl_set(uint8_t port_id, uint8_t gmii_lpi_predict_mode_en, uint16_t gmii_lpi_predict_threshold);
int ag_drv_lport_xlmac_gmii_eee_ctrl_get(uint8_t port_id, uint8_t *gmii_lpi_predict_mode_en, uint16_t *gmii_lpi_predict_threshold);
int ag_drv_lport_xlmac_timestamp_adjust_set(uint8_t port_id, uint8_t ts_use_cs_offset, uint8_t ts_tsts_adjust, uint16_t ts_osts_adjust);
int ag_drv_lport_xlmac_timestamp_adjust_get(uint8_t port_id, uint8_t *ts_use_cs_offset, uint8_t *ts_tsts_adjust, uint16_t *ts_osts_adjust);
int ag_drv_lport_xlmac_timestamp_byte_adjust_set(uint8_t port_id, uint8_t rx_timer_byte_adjust_en, uint16_t rx_timer_byte_adjust, uint8_t tx_timer_byte_adjust_en, uint16_t tx_timer_byte_adjust);
int ag_drv_lport_xlmac_timestamp_byte_adjust_get(uint8_t port_id, uint8_t *rx_timer_byte_adjust_en, uint16_t *rx_timer_byte_adjust, uint8_t *tx_timer_byte_adjust_en, uint16_t *tx_timer_byte_adjust);
int ag_drv_lport_xlmac_tx_crc_corrupt_ctrl_set(uint8_t port_id, uint32_t prog_tx_crc, uint8_t tx_crc_corruption_mode, uint8_t tx_crc_corrupt_en, uint8_t tx_err_corrupts_crc);
int ag_drv_lport_xlmac_tx_crc_corrupt_ctrl_get(uint8_t port_id, uint32_t *prog_tx_crc, uint8_t *tx_crc_corruption_mode, uint8_t *tx_crc_corrupt_en, uint8_t *tx_err_corrupts_crc);
int ag_drv_lport_xlmac_tx_crc_corrupt_ctrl_overlay_set(uint8_t port_id, uint8_t xlmac_tx_crc_corrupt_ctrl_hi, uint32_t xlmac_tx_crc_corrupt_ctrl_lo);
int ag_drv_lport_xlmac_tx_crc_corrupt_ctrl_overlay_get(uint8_t port_id, uint8_t *xlmac_tx_crc_corrupt_ctrl_hi, uint32_t *xlmac_tx_crc_corrupt_ctrl_lo);
int ag_drv_lport_xlmac_e2e_ctrl_set(uint8_t port_id, const lport_xlmac_e2e_ctrl *e2e_ctrl);
int ag_drv_lport_xlmac_e2e_ctrl_get(uint8_t port_id, lport_xlmac_e2e_ctrl *e2e_ctrl);
int ag_drv_lport_xlmac_e2ecc_module_hdr_0_set(uint8_t port_id, uint64_t e2ecc_module_hdr_0);
int ag_drv_lport_xlmac_e2ecc_module_hdr_0_get(uint8_t port_id, uint64_t *e2ecc_module_hdr_0);
int ag_drv_lport_xlmac_e2ecc_module_hdr_0_overlay_set(uint8_t port_id, uint32_t e2ecc_module_hdr_0_hi, uint32_t e2ecc_module_hdr_0_lo);
int ag_drv_lport_xlmac_e2ecc_module_hdr_0_overlay_get(uint8_t port_id, uint32_t *e2ecc_module_hdr_0_hi, uint32_t *e2ecc_module_hdr_0_lo);
int ag_drv_lport_xlmac_e2ecc_module_hdr_1_set(uint8_t port_id, uint64_t e2ecc_module_hdr_1);
int ag_drv_lport_xlmac_e2ecc_module_hdr_1_get(uint8_t port_id, uint64_t *e2ecc_module_hdr_1);
int ag_drv_lport_xlmac_e2ecc_module_hdr_1_overlay_set(uint8_t port_id, uint32_t e2ecc_module_hdr_1_hi, uint32_t e2ecc_module_hdr_1_lo);
int ag_drv_lport_xlmac_e2ecc_module_hdr_1_overlay_get(uint8_t port_id, uint32_t *e2ecc_module_hdr_1_hi, uint32_t *e2ecc_module_hdr_1_lo);
int ag_drv_lport_xlmac_e2ecc_data_hdr_0_set(uint8_t port_id, uint64_t e2ecc_data_hdr_0);
int ag_drv_lport_xlmac_e2ecc_data_hdr_0_get(uint8_t port_id, uint64_t *e2ecc_data_hdr_0);
int ag_drv_lport_xlmac_e2ecc_data_hdr_0_overlay_set(uint8_t port_id, uint32_t e2ecc_data_hdr_0_hi, uint32_t e2ecc_data_hdr_0_lo);
int ag_drv_lport_xlmac_e2ecc_data_hdr_0_overlay_get(uint8_t port_id, uint32_t *e2ecc_data_hdr_0_hi, uint32_t *e2ecc_data_hdr_0_lo);
int ag_drv_lport_xlmac_e2ecc_data_hdr_1_set(uint8_t port_id, uint64_t e2ecc_data_hdr_1);
int ag_drv_lport_xlmac_e2ecc_data_hdr_1_get(uint8_t port_id, uint64_t *e2ecc_data_hdr_1);
int ag_drv_lport_xlmac_e2ecc_data_hdr_1_overlay_set(uint8_t port_id, uint32_t e2ecc_data_hdr_1_hi, uint32_t e2ecc_data_hdr_1_lo);
int ag_drv_lport_xlmac_e2ecc_data_hdr_1_overlay_get(uint8_t port_id, uint32_t *e2ecc_data_hdr_1_hi, uint32_t *e2ecc_data_hdr_1_lo);
int ag_drv_lport_xlmac_e2efc_module_hdr_0_set(uint8_t port_id, uint64_t e2efc_module_hdr_0);
int ag_drv_lport_xlmac_e2efc_module_hdr_0_get(uint8_t port_id, uint64_t *e2efc_module_hdr_0);
int ag_drv_lport_xlmac_e2efc_module_hdr_0_overlay_set(uint8_t port_id, uint32_t e2efc_module_hdr_0_hi, uint32_t e2efc_module_hdr_0_lo);
int ag_drv_lport_xlmac_e2efc_module_hdr_0_overlay_get(uint8_t port_id, uint32_t *e2efc_module_hdr_0_hi, uint32_t *e2efc_module_hdr_0_lo);
int ag_drv_lport_xlmac_e2efc_module_hdr_1_set(uint8_t port_id, uint64_t e2efc_module_hdr_1);
int ag_drv_lport_xlmac_e2efc_module_hdr_1_get(uint8_t port_id, uint64_t *e2efc_module_hdr_1);
int ag_drv_lport_xlmac_e2efc_module_hdr_1_overlay_set(uint8_t port_id, uint32_t e2efc_module_hdr_1_hi, uint32_t e2efc_module_hdr_1_lo);
int ag_drv_lport_xlmac_e2efc_module_hdr_1_overlay_get(uint8_t port_id, uint32_t *e2efc_module_hdr_1_hi, uint32_t *e2efc_module_hdr_1_lo);
int ag_drv_lport_xlmac_e2efc_data_hdr_0_set(uint8_t port_id, uint64_t e2efc_data_hdr_0);
int ag_drv_lport_xlmac_e2efc_data_hdr_0_get(uint8_t port_id, uint64_t *e2efc_data_hdr_0);
int ag_drv_lport_xlmac_e2efc_data_hdr_0_overlay_set(uint8_t port_id, uint32_t e2efc_data_hdr_0_hi, uint32_t e2efc_data_hdr_0_lo);
int ag_drv_lport_xlmac_e2efc_data_hdr_0_overlay_get(uint8_t port_id, uint32_t *e2efc_data_hdr_0_hi, uint32_t *e2efc_data_hdr_0_lo);
int ag_drv_lport_xlmac_e2efc_data_hdr_1_set(uint8_t port_id, uint64_t e2efc_data_hdr_1);
int ag_drv_lport_xlmac_e2efc_data_hdr_1_get(uint8_t port_id, uint64_t *e2efc_data_hdr_1);
int ag_drv_lport_xlmac_e2efc_data_hdr_1_overlay_set(uint8_t port_id, uint32_t e2efc_data_hdr_1_hi, uint32_t e2efc_data_hdr_1_lo);
int ag_drv_lport_xlmac_e2efc_data_hdr_1_overlay_get(uint8_t port_id, uint32_t *e2efc_data_hdr_1_hi, uint32_t *e2efc_data_hdr_1_lo);
int ag_drv_lport_xlmac_txfifo_cell_cnt_get(uint8_t port_id, uint8_t *cell_cnt);
int ag_drv_lport_xlmac_txfifo_cell_req_cnt_get(uint8_t port_id, uint8_t *req_cnt);
int ag_drv_lport_xlmac_mem_ctrl_set(uint8_t port_id, uint16_t tx_cdc_mem_ctrl_tm, uint16_t rx_cdc_mem_ctrl_tm);
int ag_drv_lport_xlmac_mem_ctrl_get(uint8_t port_id, uint16_t *tx_cdc_mem_ctrl_tm, uint16_t *rx_cdc_mem_ctrl_tm);
int ag_drv_lport_xlmac_ecc_ctrl_set(uint8_t port_id, uint8_t tx_cdc_ecc_ctrl_en, uint8_t rx_cdc_ecc_ctrl_en);
int ag_drv_lport_xlmac_ecc_ctrl_get(uint8_t port_id, uint8_t *tx_cdc_ecc_ctrl_en, uint8_t *rx_cdc_ecc_ctrl_en);
int ag_drv_lport_xlmac_ecc_force_double_bit_err_set(uint8_t port_id, uint8_t tx_cdc_force_double_bit_err, uint8_t rx_cdc_force_double_bit_err);
int ag_drv_lport_xlmac_ecc_force_double_bit_err_get(uint8_t port_id, uint8_t *tx_cdc_force_double_bit_err, uint8_t *rx_cdc_force_double_bit_err);
int ag_drv_lport_xlmac_ecc_force_single_bit_err_set(uint8_t port_id, uint8_t tx_cdc_force_single_bit_err, uint8_t rx_cdc_force_single_bit_err);
int ag_drv_lport_xlmac_ecc_force_single_bit_err_get(uint8_t port_id, uint8_t *tx_cdc_force_single_bit_err, uint8_t *rx_cdc_force_single_bit_err);
int ag_drv_lport_xlmac_rx_cdc_ecc_status_get(uint8_t port_id, uint8_t *rx_cdc_double_bit_err, uint8_t *rx_cdc_single_bit_err);
int ag_drv_lport_xlmac_tx_cdc_ecc_status_get(uint8_t port_id, uint8_t *tx_cdc_double_bit_err, uint8_t *tx_cdc_single_bit_err);
int ag_drv_lport_xlmac_clear_ecc_status_set(uint8_t port_id, uint8_t clear_tx_cdc_double_bit_err, uint8_t clear_tx_cdc_single_bit_err, uint8_t clear_rx_cdc_double_bit_err, uint8_t clear_rx_cdc_single_bit_err);
int ag_drv_lport_xlmac_clear_ecc_status_get(uint8_t port_id, uint8_t *clear_tx_cdc_double_bit_err, uint8_t *clear_tx_cdc_single_bit_err, uint8_t *clear_rx_cdc_double_bit_err, uint8_t *clear_rx_cdc_single_bit_err);
int ag_drv_lport_xlmac_intr_status_get(uint8_t port_id, lport_xlmac_intr_status *intr_status);
int ag_drv_lport_xlmac_intr_enable_set(uint8_t port_id, const lport_xlmac_intr_enable *intr_enable);
int ag_drv_lport_xlmac_intr_enable_get(uint8_t port_id, lport_xlmac_intr_enable *intr_enable);
int ag_drv_lport_xlmac_version_id_get(uint8_t port_id, uint16_t *xlmac_version);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_lport_xlmac_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

