/*
    <:copyright-BRCM:2015:DUAL/GPL:standard

       Copyright (c) 2015 Broadcom
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

#ifndef DRV_RNR_H_INCLUDED
#define DRV_RNR_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "rdd_runner_proj_defs.h"
#include "xrdp_autogens.h"
#include "rdpa_types.h"
#include "bcm_rdp_arch.h"


/* parser and runner*/
#define ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_gen_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_sen_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_gen_cfg_set
#define ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get ag_drv_all_rnr_cmn_parser_core_configuration_qtag_ethtype_get
#define ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set ag_drv_all_rnr_cmn_parser_core_configuration_qtag_ethtype_set
#define ag_drv_rnr_quad_parser_core_configuration_dos_attack_get ag_drv_all_rnr_cmn_parser_core_configuration_dos_attack_get
#define ag_drv_rnr_quad_parser_core_configuration_dos_attack_set ag_drv_all_rnr_cmn_parser_core_configuration_dos_attack_set
#define ag_drv_rnr_quad_tcp_flags_set ag_drv_all_rnr_cmn_tcp_flags_set
#define ag_drv_rnr_quad_profile_us_set ag_drv_all_rnr_cmn_profile_us_set
#define ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get ag_drv_all_rnr_cmn_parser_core_configuration_user_ethtype_0_1_get
#define ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get ag_drv_all_rnr_cmn_parser_core_configuration_user_ethtype_2_3_get
#define ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set ag_drv_all_rnr_cmn_parser_core_configuration_user_ethtype_0_1_set
#define ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set ag_drv_all_rnr_cmn_parser_core_configuration_user_ethtype_2_3_set
#define rnr_quad_parser_core_configuration_user_ethtype_config all_rnr_cmn_parser_core_configuration_user_ethtype_config
#define ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get ag_drv_all_rnr_cmn_parser_core_configuration_user_ethtype_config_get
#define ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set ag_drv_all_rnr_cmn_parser_core_configuration_user_ethtype_config_set
#define rnr_quad_parser_ip0 all_rnr_cmn_parser_ip0
#define ag_drv_rnr_quad_parser_ip0_set ag_drv_all_rnr_cmn_parser_ip0_set
#define ag_drv_rnr_quad_parser_ip0_get ag_drv_all_rnr_cmn_parser_ip0_get
#define ag_drv_rnr_quad_parser_ip1_set ag_drv_all_rnr_cmn_parser_ip1_set
#define ag_drv_rnr_quad_parser_ip1_get ag_drv_all_rnr_cmn_parser_ip1_get
#define ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set ag_drv_all_rnr_cmn_parser_core_configuration_ppp_ip_prot_code_set
#define ag_drv_rnr_quad_parser_core_configuration_eng_set ag_drv_all_rnr_cmn_parser_core_configuration_eng_set
#define ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_set ag_drv_all_rnr_cmn_parser_core_configuration_icmp_max_size_set
#define rnr_quad_general_config_profiling_config all_rnr_cmn_general_config_profiling_config
#define ag_drv_rnr_quad_parser_ipv6_filter_set ag_drv_all_rnr_cmn_parser_ipv6_filter_set
#define ag_drv_rnr_regs_rnr_freq_set ag_drv_all_rnr_regs_rnr_freq_set
#define ag_drv_rnr_regs_clear_trace_fifo_overrun_set ag_drv_all_rnr_regs_clear_trace_fifo_overrun_set
#define ag_drv_rnr_quad_parser_qtag_nest_max_vlans_set ag_drv_crdp_rcg_rnr_cmn_parser_qtag_nest_max_vlans_set
#define ag_drv_rnr_quad_parser_snap_conf_set ag_drv_all_rnr_cmn_parser_snap_conf_set
#define ag_drv_rnr_regs_rnr_core_cntrs_get ag_drv_all_rnr_regs_rnr_core_cntrs_get
#define rnr_regs_cfg_int_ctrl all_rnr_regs_cfg_int_ctrl
#define rnr_regs_cfg_bkpt_cfg all_rnr_regs_cfg_bkpt_cfg
#define ag_drv_rnr_regs_trace_config_get ag_drv_all_rnr_regs_trace_config_get
#define ag_drv_rnr_regs_trace_config_set ag_drv_all_rnr_regs_trace_config_set
#define ag_drv_rnr_regs_cfg_bkpt_cfg_get ag_drv_all_rnr_regs_cfg_bkpt_cfg_get
#define ag_drv_rnr_regs_cfg_bkpt_cfg_set ag_drv_all_rnr_regs_cfg_bkpt_cfg_set
#define ag_drv_rnr_regs_rnr_enable_set ag_drv_all_rnr_regs_rnr_enable_set
#define ag_drv_rnr_regs_rnr_enable_get ag_drv_all_rnr_regs_rnr_enable_get
#define ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_set ag_drv_all_rnr_cmn_general_config_data_bkpt_common_cfg_set
#define ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_get ag_drv_all_rnr_cmn_general_config_data_bkpt_common_cfg_get
#define ag_drv_rnr_regs_cfg_bkpt_sts_get ag_drv_all_rnr_regs_cfg_bkpt_sts_get
#define ag_drv_rnr_regs_cfg_bkpt_sts_set ag_drv_all_rnr_regs_cfg_bkpt_sts_set
#define ag_drv_rnr_regs_cfg_int_ctrl_get ag_drv_all_rnr_regs_cfg_int_ctrl_get
#define ag_drv_rnr_regs_cfg_int_ctrl_set ag_drv_all_rnr_regs_cfg_int_ctrl_set
#define ag_drv_rnr_regs_prediction_overrun_get ag_drv_all_rnr_regs_prediction_overrun_get
#define cli_rnr_regs_cfg_psram_cfg cli_all_rnr_regs_cfg_psram_cfg
#define cli_rnr_regs_rnr_freq cli_all_rnr_regs_rnr_freq
#define cli_rnr_regs_cfg_ddr_cfg cli_all_rnr_regs_cfg_ddr_cfg
#define cli_rnr_regs_cfg_sch_cfg cli_all_rnr_regs_cfg_sch_cfg
#define cli_rnr_regs_cfg_pc_sts cli_all_rnr_regs_cfg_pc_sts
#define cli_rnr_regs_reset_trace_fifo cli_all_rnr_regs_reset_trace_fifo
#define ag_drv_rnr_quad_general_config_bkpt_0_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_0_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_1_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_1_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_2_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_2_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_3_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_3_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_4_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_4_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_5_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_5_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_6_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_6_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_7_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_7_cfg_get
#define ag_drv_rnr_quad_general_config_bkpt_0_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_0_cfg_set
#define ag_drv_rnr_quad_general_config_bkpt_1_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_1_cfg_set
#define ag_drv_rnr_quad_general_config_bkpt_2_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_2_cfg_set
#define ag_drv_rnr_quad_general_config_bkpt_3_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_3_cfg_set
#define ag_drv_rnr_quad_general_config_bkpt_4_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_4_cfg_set
#define ag_drv_rnr_quad_general_config_bkpt_5_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_5_cfg_set
#define ag_drv_rnr_quad_general_config_bkpt_6_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_6_cfg_set
#define ag_drv_rnr_quad_general_config_bkpt_7_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_7_cfg_set
#define ag_drv_rnr_regs_reset_trace_fifo_set ag_drv_all_rnr_regs_reset_trace_fifo_set
#define ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_set ag_drv_all_rnr_cmn_general_config_data_bkpt_0_cfg_set
#define ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_set ag_drv_all_rnr_cmn_general_config_data_bkpt_1_cfg_set
#define ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_set ag_drv_all_rnr_cmn_general_config_data_bkpt_2_cfg_set
#define ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_set ag_drv_all_rnr_cmn_general_config_data_bkpt_3_cfg_set
#define ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_get ag_drv_all_rnr_cmn_general_config_data_bkpt_0_cfg_get
#define ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_get ag_drv_all_rnr_cmn_general_config_data_bkpt_1_cfg_get
#define ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_get ag_drv_all_rnr_cmn_general_config_data_bkpt_2_cfg_get
#define ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_get ag_drv_all_rnr_cmn_general_config_data_bkpt_3_cfg_get
#define ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get ag_drv_crdp_rcg_rnr_cmn_parser_core_configuration_qtag_hard_nest_0_get
#define ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set ag_drv_crdp_rcg_rnr_cmn_parser_core_configuration_qtag_hard_nest_0_set
#define ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get ag_drv_crdp_rcg_rnr_cmn_parser_core_configuration_qtag_hard_nest_1_get
#define ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set ag_drv_crdp_rcg_rnr_cmn_parser_core_configuration_qtag_hard_nest_1_set
#define ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get ag_drv_crdp_rcg_rnr_cmn_parser_core_configuration_qtag_hard_nest_2_get
#define ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set ag_drv_crdp_rcg_rnr_cmn_parser_core_configuration_qtag_hard_nest_2_set
#define ag_drv_rnr_quad_parser_qtag_nest_prof0_set ag_drv_crdp_rcg_rnr_cmn_parser_qtag_nest_prof0_set
#define ag_drv_rnr_quad_parser_qtag_nest_prof0_get ag_drv_crdp_rcg_rnr_cmn_parser_qtag_nest_prof0_get
#define ag_drv_rnr_quad_parser_qtag_nest_prof1_set ag_drv_crdp_rcg_rnr_cmn_parser_qtag_nest_prof1_set
#define ag_drv_rnr_quad_parser_qtag_nest_prof1_get ag_drv_crdp_rcg_rnr_cmn_parser_qtag_nest_prof1_get
#define ag_drv_rnr_quad_parser_qtag_nest_prof2_set ag_drv_crdp_rcg_rnr_cmn_parser_qtag_nest_prof2_set
#define ag_drv_rnr_quad_parser_qtag_nest_prof2_get ag_drv_crdp_rcg_rnr_cmn_parser_qtag_nest_prof2_get
#define ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set ag_drv_all_rnr_cmn_parser_core_configuration_prop_tag_cfg_set
#define ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get ag_drv_all_rnr_cmn_parser_core_configuration_prop_tag_cfg_get
#define bcm_rnr_regs_cli_get bcm_all_rnr_cmn_cli_get
#define ag_drv_rnr_quad_general_config_powersave_config_set ag_drv_all_rnr_cmn_general_config_powersave_config_set
#define ag_drv_rnr_quad_general_config_powersave_config_get ag_drv_all_rnr_cmn_general_config_powersave_config_get
#define ag_drv_rnr_quad_general_config_profiling_config_get ag_drv_all_rnr_cmn_general_config_profiling_config_get
#define ag_drv_rnr_quad_general_config_profiling_config_set ag_drv_all_rnr_cmn_general_config_profiling_config_set
#define RNR_REGS_CFG_GLOBAL_CTRL ALL_RNR_REGS_CFG_GLOBAL_CTRL
#define RNR_CNTXT_MEM_ENTRY_REG ALL_RNR_CNTXT_MEM_ENTRY_REG
#define RNR_CNTXT_BLOCK ALL_RNR_CNTXT_BLOCK
#define RNR_MEM_BLOCK ALL_RNR_MEM_BLOCK
#define RNR_MEM_HIGH_REG_RAM_CNT ALL_RNR_MEM_MEM_ENTRY_REG_RAM_CNT
#define RNR_CNTXT_MEM_ENTRY_REG_RAM_CNT ALL_RNR_CNTXT_MEM_ENTRY_REG_RAM_CNT
#define ag_drv_rnr_quad_da_filter_valid_get ag_drv_all_rnr_cmn_da_filter_valid_get
#define ag_drv_rnr_quad_da_filter_valid_set ag_drv_all_rnr_cmn_da_filter_valid_set
#define ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set ag_drv_all_rnr_cmn_parser_core_configuration_da_filt_valid_cfg_1_set
#define ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set ag_drv_all_rnr_cmn_parser_core_configuration_da_filt_valid_cfg_2_set
#define rnr_regs_trace_config all_rnr_regs_trace_config
#define rnr_quad_general_config_powersave_config all_rnr_cmn_general_config_powersave_config
#define ag_drv_rnr_quad_profile_us_set ag_drv_all_rnr_cmn_profile_us_set
#define rnr_quad_parser_core_configuration_key_cfg all_rnr_cmn_parser_core_configuration_key_cfg
#define rnr_quad_parser_ipv6_filter all_rnr_cmn_parser_ipv6_filter
#define rnr_quad_parser_snap_conf all_rnr_cmn_parser_snap_conf
#define rnr_regs_cfg_data_bkpt_cfg all_rnr_regs_cfg_data_bkpt_cfg
#define ag_drv_rnr_regs_cfg_data_bkpt_cfg_get ag_drv_all_rnr_regs_cfg_data_bkpt_cfg_get
#define ag_drv_rnr_regs_cfg_data_bkpt_cfg_set ag_drv_all_rnr_regs_cfg_data_bkpt_cfg_set
#define cli_rnr_quad_general_config_bkpt_gen_cfg cli_all_rnr_cmn_general_config_bkpt_gen_cfg
#define ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set ag_drv_all_rnr_cmn_general_config_bkpt_gen_cfg_set
#define ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get ag_drv_all_rnr_cmn_general_config_bkpt_gen_cfg_get
#define rnr_quad_parser_core_configuration_prop_tag_cfg all_rnr_cmn_parser_core_configuration_prop_tag_cfg
#define ag_drv_rnr_quad_parser_core_configuration_key_cfg_get ag_drv_all_rnr_cmn_parser_core_configuration_key_cfg_get
#define ag_drv_rnr_quad_parser_core_configuration_key_cfg_set ag_drv_all_rnr_cmn_parser_core_configuration_key_cfg_set
#define ag_drv_rnr_regs_dma_illegal_get ag_drv_all_rnr_regs_dma_illegal_get
#define ag_drv_rnr_regs_dma_illegal_set ag_drv_all_rnr_regs_dma_illegal_set
#define ag_drv_rnr_quad_parser_da_filter_set ag_drv_all_rnr_cmn_parser_da_filter_set
#define ag_drv_rnr_quad_parser_da_filter1_set ag_drv_all_rnr_cmn_parser_da_filter1_set
#define ag_drv_rnr_quad_parser_da_filter2_set ag_drv_all_rnr_cmn_parser_da_filter2_set
#define ag_drv_rnr_quad_parser_da_filter3_set ag_drv_all_rnr_cmn_parser_da_filter3_set
#define ag_drv_rnr_quad_parser_da_filter4_set ag_drv_all_rnr_cmn_parser_da_filter4_set
#define ag_drv_rnr_quad_parser_da_filter5_set ag_drv_all_rnr_cmn_parser_da_filter5_set
#define ag_drv_rnr_quad_parser_da_filter6_set ag_drv_all_rnr_cmn_parser_da_filter6_set
#define ag_drv_rnr_quad_parser_da_filter7_set ag_drv_all_rnr_cmn_parser_da_filter7_set
#define ag_drv_rnr_quad_parser_da_filter8_set ag_drv_all_rnr_cmn_parser_da_filter8_set
#define ag_drv_rnr_regs_cfg_sch_cfg_set ag_drv_all_rnr_regs_cfg_sch_cfg_set
#define ag_drv_rnr_regs_cfg_profiling_cfg_0_set ag_drv_all_rnr_regs_cfg_profiling_cfg_0_set
#define ag_drv_rnr_regs_cfg_psram_cfg_set ag_drv_all_rnr_regs_cfg_psram_cfg_set
#define ag_drv_rnr_regs_cfg_psram_cfg_get ag_drv_all_rnr_regs_cfg_psram_cfg_get
#define ag_drv_rnr_regs_cfg_ddr_cfg_set ag_drv_all_rnr_regs_cfg_ddr_cfg_set
#define ag_drv_rnr_regs_cfg_ddr_cfg_get ag_drv_all_rnr_regs_cfg_ddr_cfg_get
#define ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_0_get ag_drv_all_rnr_cmn_parser_core_configuration_da_filt_valid_cfg_0_get
#define ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get ag_drv_all_rnr_cmn_parser_core_configuration_da_filt_valid_cfg_1_get
#define ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get ag_drv_all_rnr_cmn_parser_core_configuration_da_filt_valid_cfg_2_get
#define rnr_quad_da_filter_valid all_rnr_cmn_da_filter_valid
#define rnr_quad_parser_da_filter all_rnr_cmn_parser_da_filter
#define bcm_rnr_quad_cli_get bcm_all_rnr_cmn_cli_get
#define ag_drv_rnr_regs_cfg_cpu_wakeup_set ag_drv_all_rnr_regs_cfg_cpu_wakeup_set
#define ag_drv_rnr_regs_trace_config_set ag_drv_all_rnr_regs_trace_config_set
#define ag_drv_rnr_quad_general_config_all_xfers_cnt_get ag_drv_all_rnr_cmn_general_config_all_xfers_cnt_get
#define ag_drv_rnr_quad_general_config_ubus_counter_control_set ag_drv_all_rnr_cmn_general_config_ubus_counter_control_set
#define ag_drv_rnr_quad_general_config_ubus_down_counter_get ag_drv_all_rnr_cmn_general_config_ubus_down_counter_get
#define ag_drv_rnr_quad_general_config_read_xfers_cnt_get ag_drv_all_rnr_cmn_general_config_read_xfers_cnt_get
#define ag_drv_rnr_quad_general_config_read_data_cnt_get ag_drv_all_rnr_cmn_general_config_read_data_cnt_get
#define ag_drv_rnr_quad_general_config_write_data_cnt_get ag_drv_all_rnr_cmn_general_config_write_data_cnt_get
#define ag_drv_rnr_regs_cfg_cpu_wakeup_set ag_drv_all_rnr_regs_cfg_cpu_wakeup_set
#define ag_drv_rnr_quad_general_config_multi_psel_cfg_set ag_drv_all_rnr_cmn_general_config_multi_psel_cfg_set


#define RNR_REGS_BLOCK ALL_RNR_REGS_BLOCK
#define RNR_REGS ALL_RNR_REGS
#define RNR_MEM_BLOCK ALL_RNR_MEM_BLOCK
#define RNR_MEM ALL_RNR_MEM
#define RNR_QUAD_BLOCK ALL_RNR_CMN_BLOCK
#define RNR_QUAD ALL_RNR_CMN
#define RNR_PRED_BLOCK ALL_RNR_PRED_BLOCK
#define RNR_PRED ALL_RNR_PRED
#define RNR_INST_BLOCK ALL_RNR_INST_BLOCK
#define RNR_INST ALL_RNR_INST


typedef enum
{
    DRV_RNR_2SP_14RR = 0,
    DRV_RNR_4SP_12RR = 1,
    DRV_RNR_8SP_8RR = 2,
    DRV_RNR_16RR = 3,
    DRV_RNR_16SP = 4,
} drv_rnr_sch_mode_t;

typedef enum
{
    DRV_PARSER_QTAG_PROFILE_0 = 0,
    DRV_PARSER_QTAG_PROFILE_1 = 1,
    DRV_PARSER_QTAG_PROFILE_2 = 2,
    DRV_PARSER_NUM_OF_QTAG_PROFILE = 3,
} drv_parser_qtag_profile_t;

typedef enum
{
    PROP_TAG_SIZE_0 = 0,
    PROP_TAG_SIZE_2 = 2,
    PROP_TAG_SIZE_4 = 4,
    PROP_TAG_SIZE_6 = 6,
    PROP_TAG_SIZE_8 = 8,
} drv_rnr_prop_tag_size_t;

typedef enum
{
    RNR_PROFILING_IDLE_CYCLES_COUNTER,
    RNR_PROFILING_IDLE_PWRSAVE_MODE_COUNTER,
} drv_rnr_profiling_idle_counter_t;

typedef enum
{
	BRIDGE_DA_MAC_FILTER_GROUP = 0,
	ROUTER_DA_MAC_FILTER_GROUP = 1,
	DA_MAC_FILTER_GROUP_SIZE   = 2,
} drv_rnr_da_flt_grp_t;

/* qtag mask */
#define DRV_PARSER_OUTER_QTAG_USER_OUTER_BIT        0
#define DRV_PARSER_OUTER_QTAG_8100_OUTER_BIT        0
#define DRV_PARSER_OUTER_QTAG_88A8_OUTER_BIT        3
#define DRV_PARSER_OUTER_QTAG_9100_OUTER_BIT        6
#define DRV_PARSER_OUTER_QTAG_9200_OUTER_BIT        9
#define DRV_PARSER_OUTER_QTAG_USER_INNER_BIT        1
#define DRV_PARSER_OUTER_QTAG_8100_INNER_BIT        1
#define DRV_PARSER_OUTER_QTAG_88A8_INNER_BIT        4
#define DRV_PARSER_OUTER_QTAG_9100_INNER_BIT        7
#define DRV_PARSER_OUTER_QTAG_9200_INNER_BIT        10
#define DRV_PARSER_OUTER_QTAG_USER_3RD_BIT          2
#define DRV_PARSER_OUTER_QTAG_8100_3RD_BIT          2
#define DRV_PARSER_OUTER_QTAG_88A8_3RD_BIT          5
#define DRV_PARSER_OUTER_QTAG_9100_3RD_BIT          8
#define DRV_PARSER_OUTER_QTAG_9200_3RD_BIT          11
#define MAX_NUM_OF_PROFILES                         3
#define NUM_OF_RNR_QUAD                             NUM_OF_RNR_QUADS

#define PARSER_ENG_TRIPLE_TAG_DETECTION_BIT_SHIFT   8
/* number of bytes in MAC address */
#define PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS       6
#define RNR_PROFILING_DEFAULT_COUNTER_LSB           0
#define RNR_PROFILING_DEFAULT_CYCLE_NUM             0xFFFF /* maximum possible */

/* gets bit #i from a given number */
#define MS_GET_BIT_I(number, i)   (((1 << (i)) & (number)) >> (i))
/* sets bit #i of a given number to a given value */
#define MS_SET_BIT_I(number, i, bit_value)   ((number) &= (~(1 << (i))), (number) |= ((bit_value) << (i)))

#define QUAD_TO_PROFILING_CORE_ID(quad_id) (quad_id * 4)

#define drv_rnr_quad_parser_da_filter_set(num, a, b, c) ag_drv_rnr_quad_parser_da_filter## #num ##_set(a,b,c)

#define BCM_HW_POLL_INTERVAL_US 1

typedef struct
{
    uint32_t dma_base;
    uint8_t dma_buf_size;
    uint8_t dma_static_offset;
} rnr_dma_cfg_t;

typedef struct
{
    rnr_dma_cfg_t psram;
    rnr_dma_cfg_t ddr;
} rnr_dma_regs_cfg_t;

extern uintptr_t rdp_runner_core_addr[];

int drv_rnr_dma_cfg(dpi_params_t *dpi_params);
void drv_rnr_cores_addr_init(const ru_block_rec *RNR_BLOCK, uint32_t ru_get_offset_rnr_mem);
void drv_rnr_mem_init(void);
void drv_rnr_load_microcode(void);
void drv_rnr_load_prediction(void);
void drv_rnr_set_sch_cfg(void);
void rdp_rnr_write_context(void *__to, void *__from, unsigned int __n);
bdmf_error_t drv_rnr_quad_parser_configure_inner_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean inner_en, rdpa_tpid_detect_t etype);
bdmf_error_t drv_rnr_quad_parser_configure_outer_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean outer_en, rdpa_tpid_detect_t etype);
bdmf_error_t drv_rnr_quad_parser_configure_3rd_qtag(rnr_quad_id_e quad_id,
    drv_parser_qtag_profile_t profile, bdmf_boolean inner_en, rdpa_tpid_detect_t etype);
bdmf_error_t drv_rnr_quad_parser_pure_ack_set(rnr_quad_id_e quad_id, uint32_t enable);
bdmf_error_t drv_rnr_quad_parser_prop_cfg (rnr_quad_id_e quad_id);
void parser_mac_address_array_to_hw_format(const uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS],
    uint32_t *address_4_ls_bytes, uint16_t *addres_2_ms_bytes);
void parser_mac_address_hw_format_to_array(uint32_t address_4_ls_bytes,
    uint16_t addres_2_ms_bytes, uint8_t mac_address[PARSER_NUMBER_OF_BYTES_IN_MAC_ADDRESS]);
bdmf_error_t drv_rnr_quad_parser_da_filter_without_mask_set(const uint8_t *mac_address, bdmf_boolean add);
bdmf_error_t drv_rnr_quad_parser_da_filter_valid_cfg(rnr_quad_id_e quad_id, uint8_t filter_index, uint8_t enable);
void drv_rnr_profiling_clear_trace(uint8_t core_id);
void drv_rnr_quad_profiling_quad_init(rnr_quad_id_e quad_id);
bdmf_error_t drv_rnr_sram_init(void);
bdmf_error_t drv_rnr_power_save_init(void);
void drv_rnr_configure_scheduler(void);
bdmf_error_t drv_rnr_bkpt_init(void);

uint16_t drv_rnr_dynamic_code_start_address_get(void);
uint16_t drv_rnr_dynamic_code_end_address_get(void);

#if !defined(RDP_SIM)
int  drv_rnr_quad_ubus_decode_wnd_cfg(uint32_t win, uint32_t phys_addr, uint32_t size_power_of_2, int port_id, unsigned int cache_bit_en);
#endif
int drv_rnr_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val);
#ifdef USE_BDMF_SHELL
int drv_rnr_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_quad_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_parser_cli_config_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_rnr_cli_sanity_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_rnr_cli_config_trace(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_rnr_cli_set_parser_dos_attack(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_rnr_cli_init(bdmfmon_handle_t driver_dir);
void drv_rnr_cli_exit(bdmfmon_handle_t driver_dir);
#endif

void drv_rnr_num_of_cores_set(int num_of_cores);
int drv_rnr_get_inst_word_size(int core);

bdmf_error_t ag_drv_rnr_quad_exception_bits_set(uint8_t quad_idx, uint32_t exception_en);
bdmf_error_t ag_drv_rnr_quad_exception_bits_get(uint8_t quad_idx, uint32_t *exception_en);

#define rnr_regs_rnr_core_cntrs all_rnr_regs_rnr_core_cntrs

struct xrdp_rnr_profiling_cfg {
    char enable_quads[NUM_OF_RNR_QUAD];
    uint32_t num_cycles;
    uint32_t manual_stop_mode;
    uint32_t trace_mode;
    uint32_t task_id;
    uint32_t address_mode;
    uint32_t address_min;
    uint32_t address_max;
};

struct xrdp_rnr_profiling_res {
    uint32_t total_cnt;
    char profiling_on;
#ifndef BCM6858    
    rnr_regs_rnr_core_cntrs cntrs[NUM_OF_RUNNER_CORES];
#else
    uint32_t idle_cnts[NUM_OF_RUNNER_CORES];
#endif
};

void drv_rnr_load_instructions(uint8_t rnr_idx, uint32_t first_instuct_idx, uint32_t num_of_instructs, uint32_t *instructs_arr);
int xrdp_rnr_profiling_set_config(struct xrdp_rnr_profiling_cfg *cfg);
int xrdp_rnr_profiling_get_result(struct xrdp_rnr_profiling_res *res);
int xrdp_rnr_profiling_stop_manual_mode(void);

/* Data path init helpers */
int drv_rnr_frequency_set(uint16_t freq);
int drv_parser_init(void);

#if defined(RDP_ARCH_SIM) || defined(RDP_ARCH_QEMU_SIM) || defined(CONFIG_BRCM_QEMU_NO_INDIRECT)

static inline void runner_core_mem_finalize(void) {
    drv_rnr_mem_init();
}

#elif defined(RDP_ARCH_BOARD) || defined (RDP_ARCH_BOOT)

static inline void runner_core_mem_finalize(void) {
    uint8_t core_index;
    all_rnr_regs_cfg_gen_cfg cfg_gen_cfg;

    /* wait for core memory to be zero'd */
    for (core_index = 0; core_index < NUM_OF_RUNNER_CORES; core_index++)
    {
         do
         {
             ag_drv_all_rnr_regs_cfg_gen_cfg_get(core_index, &cfg_gen_cfg);
         } while (!cfg_gen_cfg.zero_data_mem_done || !cfg_gen_cfg.zero_context_mem_done);
    }
}



#else /* RDP_ARCH */
#error "no arch defined"
#endif /* RDP_ARCH */


#ifdef __cplusplus
}
#endif

#endif
