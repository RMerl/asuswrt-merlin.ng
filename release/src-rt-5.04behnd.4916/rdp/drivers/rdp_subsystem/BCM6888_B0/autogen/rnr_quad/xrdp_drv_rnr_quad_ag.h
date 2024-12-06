/*
   Copyright (c) 2015 Broadcom
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


#ifndef _XRDP_DRV_RNR_QUAD_AG_H_
#define _XRDP_DRV_RNR_QUAD_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint32_t ip_address;
    uint32_t ip_address_mask;
    bdmf_boolean ip_filter0_dip_en;
    bdmf_boolean ip_filter0_valid;
} rnr_quad_parser_ip0;

typedef struct
{
    uint16_t hard_nest_profile;
} rnr_quad_parser_hardcoded_ethtype_prof0;

typedef struct
{
    uint16_t da_filt_msb;
    uint32_t da_filt_lsb;
    uint16_t da_filt_mask_msb;
    uint32_t da_filt_mask_l;
} rnr_quad_parser_da_filter;

typedef struct
{
    bdmf_boolean da_filt0_valid;
    bdmf_boolean da_filt1_valid;
    bdmf_boolean da_filt2_valid;
    bdmf_boolean da_filt3_valid;
    bdmf_boolean da_filt4_valid;
    bdmf_boolean da_filt5_valid;
    bdmf_boolean da_filt6_valid;
    bdmf_boolean da_filt7_valid;
    bdmf_boolean da_filt8_valid;
} rnr_quad_da_filter_valid;

typedef struct
{
    uint32_t code;
    bdmf_boolean en_rfc1042;
    bdmf_boolean en_8021q;
} rnr_quad_parser_snap_conf;

typedef struct
{
    bdmf_boolean hop_by_hop_match;
    bdmf_boolean routing_eh;
    bdmf_boolean dest_opt_eh;
} rnr_quad_parser_ipv6_filter;

typedef struct
{
    uint8_t ethtype_user_prot_0;
    uint8_t ethtype_user_prot_1;
    uint8_t ethtype_user_prot_2;
    uint8_t ethtype_user_prot_3;
    uint8_t ethtype_user_en;
    uint8_t ethtype_user_offset_0;
    uint8_t ethtype_user_offset_1;
    uint8_t ethtype_user_offset_2;
    uint8_t ethtype_user_offset_3;
} rnr_quad_parser_core_configuration_user_ethtype_config;

typedef struct
{
    uint8_t size_profile_0;
    uint8_t size_profile_1;
    uint8_t size_profile_2;
    bdmf_boolean pre_da_dprofile_0;
    bdmf_boolean pre_da_dprofile_1;
    bdmf_boolean pre_da_dprofile_2;
} rnr_quad_parser_core_configuration_prop_tag_cfg;

typedef struct
{
    uint8_t l2_tos_mask;
    uint8_t l3_tos_mask;
    bdmf_boolean l2_exclude_smac;
    bdmf_boolean tcp_pure_ack_mask;
    bdmf_boolean incude_dei_in_vlans_crc;
    bdmf_boolean key_size;
    uint8_t max_num_of_vlans_in_crc;
    bdmf_boolean l3_tcp_pure_ack_mask;
    uint8_t rsrv;
} rnr_quad_parser_core_configuration_key_cfg;

typedef struct
{
    bdmf_boolean use_fifo_for_ddr_only;
    bdmf_boolean token_arbiter_is_rr;
    bdmf_boolean chicken_no_flowctrl;
    bdmf_boolean flow_ctrl_clear_token;
    uint8_t ddr_congest_threshold;
    uint8_t psram_congest_threshold;
    bdmf_boolean enable_reply_threshold;
    uint8_t ddr_reply_threshold;
    uint8_t psram_reply_threshold;
} rnr_quad_general_config_dma_arb_cfg;

typedef struct
{
    uint8_t counter_lsb_sel;
    bdmf_boolean enable_trace_core_0;
    bdmf_boolean enable_trace_core_1;
    bdmf_boolean enable_trace_core_2;
    bdmf_boolean enable_trace_core_3;
    bdmf_boolean enable_trace_core_4;
    bdmf_boolean enable_trace_core_5;
    bdmf_boolean enable_trace_core_6;
    bdmf_boolean enable_trace_core_7;
    bdmf_boolean enable_trace_core_8;
    bdmf_boolean enable_trace_core_9;
    bdmf_boolean enable_trace_core_10;
    bdmf_boolean enable_trace_core_11;
    bdmf_boolean enable_trace_core_12;
    bdmf_boolean enable_trace_core_13;
} rnr_quad_general_config_profiling_config;

typedef struct
{
    uint8_t time_counter;
    bdmf_boolean enable_powersave_core_0;
    bdmf_boolean enable_powersave_core_1;
    bdmf_boolean enable_powersave_core_2;
    bdmf_boolean enable_powersave_core_3;
    bdmf_boolean enable_powersave_core_4;
    bdmf_boolean enable_powersave_core_5;
    bdmf_boolean enable_powersave_core_6;
    bdmf_boolean enable_powersave_core_7;
    bdmf_boolean enable_powersave_core_8;
    bdmf_boolean enable_powersave_core_9;
    bdmf_boolean enable_powersave_core_10;
    bdmf_boolean enable_powersave_core_11;
    bdmf_boolean enable_powersave_core_12;
    bdmf_boolean enable_powersave_core_13;
    bdmf_boolean enable_cpu_if_clk_gating;
    bdmf_boolean enable_common_reg_clk_gating;
    bdmf_boolean enable_ec_blocks_clk_gating;
} rnr_quad_general_config_powersave_config;

typedef struct
{
    bdmf_boolean acc_status_0;
    bdmf_boolean acc_status_1;
    bdmf_boolean acc_status_2;
    bdmf_boolean acc_status_3;
    bdmf_boolean acc_status_4;
    bdmf_boolean acc_status_5;
    bdmf_boolean acc_status_6;
    bdmf_boolean acc_status_7;
    bdmf_boolean acc_status_8;
    bdmf_boolean acc_status_9;
    bdmf_boolean acc_status_10;
    bdmf_boolean acc_status_11;
    bdmf_boolean acc_status_12;
    bdmf_boolean acc_status_13;
    bdmf_boolean core_status_0;
    bdmf_boolean core_status_1;
    bdmf_boolean core_status_2;
    bdmf_boolean core_status_3;
    bdmf_boolean core_status_4;
    bdmf_boolean core_status_5;
    bdmf_boolean core_status_6;
    bdmf_boolean core_status_7;
    bdmf_boolean core_status_8;
    bdmf_boolean core_status_9;
    bdmf_boolean core_status_10;
    bdmf_boolean core_status_11;
    bdmf_boolean core_status_12;
    bdmf_boolean core_status_13;
} rnr_quad_general_config_powersave_status;

typedef struct
{
    bdmf_boolean psram_hdr_sw_rst_0;
    bdmf_boolean psram_data_sw_rst_0;
    bdmf_boolean ddr_hdr_sw_rst_0;
    bdmf_boolean select_fifos_for_debug;
    bdmf_boolean psram_hdr_sw_rst_1;
    bdmf_boolean psram_data_sw_rst_1;
    bdmf_boolean ddr_hdr_sw_rst_1;
    uint8_t psram_hdr_sw_rd_addr;
    uint8_t psram_data_sw_rd_addr;
    uint8_t ddr_hdr_sw_rd_addr;
} rnr_quad_debug_fifo_config;

typedef struct
{
    bdmf_boolean full;
    bdmf_boolean empty;
    uint8_t push_wr_cntr;
    uint8_t pop_rd_cntr;
    uint8_t used_words;
} rnr_quad_debug_psram_hdr_fifo_status;

typedef struct
{
    bdmf_boolean full;
    bdmf_boolean empty;
    bdmf_boolean almost_full;
    uint8_t push_wr_cntr;
    uint8_t pop_rd_cntr;
    uint8_t used_words;
} rnr_quad_debug_psram_data_fifo_status;

typedef struct
{
    bdmf_boolean full;
    bdmf_boolean empty;
    uint8_t push_wr_cntr;
    uint8_t pop_rd_cntr;
    uint8_t used_words;
} rnr_quad_debug_ddr_hdr_fifo_status;

typedef struct
{
    bdmf_boolean full;
    bdmf_boolean empty;
    bdmf_boolean almost_full;
    uint16_t wr_cntr;
    uint16_t rd_cntr;
} rnr_quad_debug_ddr_data_fifo_status;


/**********************************************************************************************************************
 * vid_0: 
 *     VLAN ID Filter for first VLAN of register
 * vid_0_en: 
 *     VLAND ID Filter 0 Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_vid0_set(uint8_t quad_idx, uint16_t vid_0, bdmf_boolean vid_0_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid0_get(uint8_t quad_idx, uint16_t *vid_0, bdmf_boolean *vid_0_en);

/**********************************************************************************************************************
 * vid_1: 
 *     VLAN ID Filter 1 for second VLAN of register
 * vid_1_en: 
 *     VLAND ID Filter 1 Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_vid1_set(uint8_t quad_idx, uint16_t vid_1, bdmf_boolean vid_1_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid1_get(uint8_t quad_idx, uint16_t *vid_1, bdmf_boolean *vid_1_en);

/**********************************************************************************************************************
 * vid_2: 
 *     VLAN ID Filter for first VLAN of register
 * vid_2_en: 
 *     VLAND ID Filter 2 Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_vid2_set(uint8_t quad_idx, uint16_t vid_2, bdmf_boolean vid_2_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid2_get(uint8_t quad_idx, uint16_t *vid_2, bdmf_boolean *vid_2_en);

/**********************************************************************************************************************
 * vid_3: 
 *     VLAN ID Filter 3 ofr second VLAN of register
 * vid_3_en: 
 *     VLAND ID Filter 3 Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_vid3_set(uint8_t quad_idx, uint16_t vid_3, bdmf_boolean vid_3_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid3_get(uint8_t quad_idx, uint16_t *vid_3, bdmf_boolean *vid_3_en);

/**********************************************************************************************************************
 * vid_4: 
 *     VLAN ID Filter for first VLAN of register
 * vid_4_en: 
 *     VLAND ID Filter 4 Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_vid4_set(uint8_t quad_idx, uint16_t vid_4, bdmf_boolean vid_4_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid4_get(uint8_t quad_idx, uint16_t *vid_4, bdmf_boolean *vid_4_en);

/**********************************************************************************************************************
 * vid_5: 
 *     VLAN ID Filter 5 ofr second VLAN of register
 * vid_5_en: 
 *     VLAND ID Filter 5 Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_vid5_set(uint8_t quad_idx, uint16_t vid_5, bdmf_boolean vid_5_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid5_get(uint8_t quad_idx, uint16_t *vid_5, bdmf_boolean *vid_5_en);

/**********************************************************************************************************************
 * vid_6: 
 *     VLAN ID Filter for first VLAN of register
 * vid_6_en: 
 *     VLAND ID Filter 6 Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_vid6_set(uint8_t quad_idx, uint16_t vid_6, bdmf_boolean vid_6_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid6_get(uint8_t quad_idx, uint16_t *vid_6, bdmf_boolean *vid_6_en);

/**********************************************************************************************************************
 * vid_7: 
 *     VLAN ID Filter 7 ofr second VLAN of register
 * vid_7_en: 
 *     VLAND ID Filter 7 Enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_vid7_set(uint8_t quad_idx, uint16_t vid_7, bdmf_boolean vid_7_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid7_get(uint8_t quad_idx, uint16_t *vid_7, bdmf_boolean *vid_7_en);

/**********************************************************************************************************************
 * ip_address: 
 *     32-bit address to match SIP or DIP (according to predefined configuration in IP_FILTERS_CFG register)
 * ip_address_mask: 
 *     32-bit address mask
 * ip_filter0_dip_en: 
 *     IP Filter0 DIP or SIP selection.
 *     The default is SIP, when the field is set -> DIP selection is enabled
 * ip_filter0_valid: 
 *     IP Filter0 valid bit.
 *     
 *     When the bit valid is set, the IP filter/mask can be applied by hardware.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_ip0_set(uint8_t quad_idx, const rnr_quad_parser_ip0 *parser_ip0);
bdmf_error_t ag_drv_rnr_quad_parser_ip0_get(uint8_t quad_idx, rnr_quad_parser_ip0 *parser_ip0);

/**********************************************************************************************************************
 * ip_address: 
 *     32-bit address to match SIP or DIP (according to predefined configuration in IP_FILTERS_CFG register)
 * ip_address_mask: 
 *     32-bit address mask
 * ip_filter1_dip_en: 
 *     IP Filter1 DIP or SIP selection.
 *     The default is SIP, when the field is set -> DIP selection is enabled
 * ip_filter1_valid: 
 *     IP Filter1 valid bit.
 *     
 *     When the bit valid is set, the IP filter/mask can be applied by hardware.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_ip1_set(uint8_t quad_idx, const rnr_quad_parser_ip0 *parser_ip0);
bdmf_error_t ag_drv_rnr_quad_parser_ip1_get(uint8_t quad_idx, rnr_quad_parser_ip0 *parser_ip0);

/**********************************************************************************************************************
 * hard_nest_profile: 
 *     bit 2-0: Enable 8100 as VLAN for outer, 2nd, and inner VLANs (inner is bit 2).
 *     bit 5-3: Enable 88a8 as VLAN for outer, 2nd, and inner VLANs.
 *     bit 8-6: Enable 9100 as VLAN for outer, 2nd, and inner VLANs.
 *     bit 11-9: Enable 9200 as VLAN for outer, 2nd, and inner VLANs.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(uint8_t quad_idx, const rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(uint8_t quad_idx, rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0);

/**********************************************************************************************************************
 * hard_nest_profile: 
 *     Hard Nest Profile
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(uint8_t quad_idx, const rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(uint8_t quad_idx, rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0);

/**********************************************************************************************************************
 * hard_nest_profile: 
 *     Hard Nest Profile
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(uint8_t quad_idx, const rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(uint8_t quad_idx, rnr_quad_parser_hardcoded_ethtype_prof0 *parser_hardcoded_ethtype_prof0);

/**********************************************************************************************************************
 * qtag_nest_0_profile_0: 
 *     Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)
 * qtag_nest_1_profile_0: 
 *     Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof0_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_0, uint8_t qtag_nest_1_profile_0);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof0_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_0, uint8_t *qtag_nest_1_profile_0);

/**********************************************************************************************************************
 * qtag_nest_0_profile_1: 
 *     Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)
 * qtag_nest_1_profile_1: 
 *     Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof1_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_1, uint8_t qtag_nest_1_profile_1);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof1_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_1, uint8_t *qtag_nest_1_profile_1);

/**********************************************************************************************************************
 * qtag_nest_0_profile_2: 
 *     Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)
 * qtag_nest_1_profile_2: 
 *     Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof2_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_2, uint8_t qtag_nest_1_profile_2);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof2_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_2, uint8_t *qtag_nest_1_profile_2);

/**********************************************************************************************************************
 * qtag_nest_0_profile_2: 
 *     Set to enable Ethertype_qTag 0 as outer (LSB)  2nd VLAN (2nd), 3rd VLAN (MSB)
 * max_num_of_vlans: 
 *     Max number of VLAN tags allowed in the packet.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_max_vlans_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_2, uint8_t max_num_of_vlans);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_max_vlans_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_2, uint8_t *max_num_of_vlans);

/**********************************************************************************************************************
 * user_ip_prot_0: 
 *     User defined IP protocol 0 (value to be matched to IP protocol field)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol0_set(uint8_t quad_idx, uint8_t user_ip_prot_0);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol0_get(uint8_t quad_idx, uint8_t *user_ip_prot_0);

/**********************************************************************************************************************
 * user_ip_prot_1: 
 *     User defined IP protocol 1 (value to be matched to IP protocol field)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol1_set(uint8_t quad_idx, uint8_t user_ip_prot_1);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol1_get(uint8_t quad_idx, uint8_t *user_ip_prot_1);

/**********************************************************************************************************************
 * user_ip_prot_2: 
 *     User defined IP protocol 2 (value to be matched to IP protocol field)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol2_set(uint8_t quad_idx, uint8_t user_ip_prot_2);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol2_get(uint8_t quad_idx, uint8_t *user_ip_prot_2);

/**********************************************************************************************************************
 * user_ip_prot_3: 
 *     User defined IP protocol 3 (value to be matched to IP protocol field)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol3_set(uint8_t quad_idx, uint8_t user_ip_prot_3);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol3_get(uint8_t quad_idx, uint8_t *user_ip_prot_3);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 * da_filt_mask_msb: 
 *     Current DA Filter mask bits 47:32
 * da_filt_mask_l: 
 *     Current DA Filter mask bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter_set(uint8_t quad_idx, const rnr_quad_parser_da_filter *parser_da_filter);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter_get(uint8_t quad_idx, rnr_quad_parser_da_filter *parser_da_filter);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 * da_filt_mask_msb: 
 *     Current DA Filter mask bits 47:32
 * da_filt_mask_l: 
 *     Current DA Filter mask bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter1_set(uint8_t quad_idx, const rnr_quad_parser_da_filter *parser_da_filter);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter1_get(uint8_t quad_idx, rnr_quad_parser_da_filter *parser_da_filter);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter2_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter2_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter3_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter3_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter4_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter4_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter5_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter5_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter6_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter6_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter7_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter7_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);

/**********************************************************************************************************************
 * da_filt_msb: 
 *     Current DA Filter bits 47:32
 * da_filt_lsb: 
 *     DA Filter bits 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_da_filter8_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter8_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);

/**********************************************************************************************************************
 * da_filt0_valid: 
 *     DA Filter0 valid bit
 * da_filt1_valid: 
 *     DA Filter1 valid bit
 * da_filt2_valid: 
 *     DA Filter2 valid bit
 * da_filt3_valid: 
 *     DA Filter3 valid bit
 * da_filt4_valid: 
 *     DA Filter4 valid bit
 * da_filt5_valid: 
 *     DA Filter5 valid bit
 * da_filt6_valid: 
 *     DA Filter6 valid bit
 * da_filt7_valid: 
 *     DA Filter7 valid bit
 * da_filt8_valid: 
 *     DA Filter8 valid bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_da_filter_valid_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_da_filter_valid_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid);

/**********************************************************************************************************************
 * exception_en: 
 *     [0] - IP header length error
 *     [1] - IPv4 header checksum error
 *     [2] - Ethernet multicast
 *     [3] - ip_mcast_match - Multicast Layer 3 Identified by the following filters on IP-DA:
 *     IPv4: 224.0.0.0/28,
 *     IPv6: 0xFF00::/116 and 0xFF30/116
 *     [4] - ip_fragment: any fragment: first middle or last
 *     [5] - ip_version_err
 *     [6] - ip_mcast_control_match - Set when Multicast Layer 3 Control:
 *     Identified IPv4 DA: IPv4 224.0.0.0/8
 *     IPv6: 0xFF0::
 *     [7] - eth_brdcst
 *     [8] - error: not enough bytes in the header to complete parsing of the packet
 *     [9] - ip_length_error
 *     [10]- eth_ipv4_mcast -
 *     multicast Layer 2 Identified by the following DA filter:
 *     01:00:5e:::/23 or 33:33::::/32
 *     [11]- not l4 fast path prtocol
 *     [12]- UDP_1588_flag
 *     [13]- DHCP identified
 *     [14]- DOS attack
 *     [15]- GRE withe unrecgonized version (not 0 or 1) or version 1 with K bit cleared
 *     [16]- DNS request
 *     [17] -UDP Checksum Zero Detected
 *     [18] -TCP flag set
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_exception_bits_set(uint8_t quad_idx, uint32_t exception_en);
bdmf_error_t ag_drv_rnr_quad_exception_bits_get(uint8_t quad_idx, uint32_t *exception_en);

/**********************************************************************************************************************
 * tcp_flags_filt: 
 *     Defines which TCP falgs set will cause TCP_FLAG bit in summary word to be set
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_tcp_flags_set(uint8_t quad_idx, uint8_t tcp_flags_filt);
bdmf_error_t ag_drv_rnr_quad_tcp_flags_get(uint8_t quad_idx, uint8_t *tcp_flags_filt);

/**********************************************************************************************************************
 * profile_us: 
 *     Profile US - Not Applicable for 63146, 4912
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_profile_us_set(uint8_t quad_idx, uint8_t profile_us);
bdmf_error_t ag_drv_rnr_quad_profile_us_get(uint8_t quad_idx, uint8_t *profile_us);

/**********************************************************************************************************************
 * disable_l2tp_source_port_check: 
 *     Disable checking source port number for L2TP identification
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_disable_l2tp_source_port_set(uint8_t quad_idx, bdmf_boolean disable_l2tp_source_port_check);
bdmf_error_t ag_drv_rnr_quad_disable_l2tp_source_port_get(uint8_t quad_idx, bdmf_boolean *disable_l2tp_source_port_check);

/**********************************************************************************************************************
 * code: 
 *     Used defined SNAP organization code
 * en_rfc1042: 
 *     enable RFC1042 0x00000 organization code
 * en_8021q: 
 *     enables 802.1Q 0x0000f8 organization code
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_snap_conf_set(uint8_t quad_idx, const rnr_quad_parser_snap_conf *parser_snap_conf);
bdmf_error_t ag_drv_rnr_quad_parser_snap_conf_get(uint8_t quad_idx, rnr_quad_parser_snap_conf *parser_snap_conf);

/**********************************************************************************************************************
 * hop_by_hop_match: 
 *     hop by hop match filter mask
 * routing_eh: 
 *     Routing extension header option match filter mask
 * dest_opt_eh: 
 *     Destination Options extension header option match filter mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_ipv6_filter_set(uint8_t quad_idx, const rnr_quad_parser_ipv6_filter *parser_ipv6_filter);
bdmf_error_t ag_drv_rnr_quad_parser_ipv6_filter_get(uint8_t quad_idx, rnr_quad_parser_ipv6_filter *parser_ipv6_filter);

/**********************************************************************************************************************
 * cfg: 
 *     eng_cnfg[0] - IP filters on IPV6 on LSByte not MSByte
 *     eng_cnfg[1] - Disable clock gating
 *     eng_cnfg[2] - enable LLC_SNAP at result word
 *     eng_cnfg[3] - defines the protocol of ppp_code_1 (1 - IPv6, 0 - IPv4) PPPOE CODE1 is IPv4 instead of IPv6
 *     eng_cnfg[4] -  disables ip hdr length error check - 0
 *     eng_cnfg[5] - Disable ip_ver_err check -> 1-ip versions is always 4
 *     eng_cnfg[6] - enables checking if ip_version matches the IP version according to L2 - 0
 *     eng_cnfg[7] - enable ICMP(next_prot=1) over IPV6
 *     eng_cnfg[8]  -  ipv6 route with non zero segment orred with ip_hdr_len_err
 *     eng_cnfg[9]  - enable detection of ipv6_hop_by_hop  not directly after ipv6 header
 *     eng_cnfg[10] - Select MAC Mode for result
 *     eng_cnfg[11] - Select IPv6 MCAST control filter FF0::/116 instead of FF02::/112
 *     eng_cnfg[12] - Free
 *     eng_cnfg[13] - ipv4 length error is assered also when packet is padded
 *     eng_cnfg[14] - ipv6 length error is assered also when packet is padded
 *     eng_cnfg[15] - enable old mode of AH at IPV6
 *     eng_cnfg[16] - enable old mode of AH at IPV4
 *     eng_cnfg[17] - enable L2/L3 combined key
 *     eng_cnfg[18] - dont allow 0xFFFF as valid ipv4 header cksum results
 *     eng_cnfg[19] - Disable IPV4_OVER_6 as qualifier for l4_fast_path_protocol
 *     eng_cnfg[20] - Disable IPV6_OVER_4 as qualifier for l4_fast_path_protocol
 *     eng_cnfg[21] - YL mode
 *     eng_cnfg[22] - Enable MC BC bits at IC Key
 *     eng_cnfg[23] - mask L2/L3 valid key logic
 *     eng_cnfg[25:24] - udp zero detect cnfg {ipv6_en, ipv4_en}
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_eng_set(uint8_t quad_idx, uint32_t cfg);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_eng_get(uint8_t quad_idx, uint32_t *cfg);

/**********************************************************************************************************************
 * ppp_code_0: 
 *     PPP Protocol code to identify L3 is IP
 * ppp_code_1: 
 *     PPP Protocol code to identify L3 is IP
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(uint8_t quad_idx, uint16_t ppp_code_0, uint16_t ppp_code_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get(uint8_t quad_idx, uint16_t *ppp_code_0, uint16_t *ppp_code_1);

/**********************************************************************************************************************
 * ethtype_qtag_0: 
 *     Ethertype to identify VLAN QTAG
 * ethtype_qtag_1: 
 *     Ethertype to identify VLAN QTAG
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(uint8_t quad_idx, uint16_t ethtype_qtag_0, uint16_t ethtype_qtag_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(uint8_t quad_idx, uint16_t *ethtype_qtag_0, uint16_t *ethtype_qtag_1);

/**********************************************************************************************************************
 * ethype_0: 
 *     User Ethertype 0
 * ethype_1: 
 *     User Ethertype 1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(uint8_t quad_idx, uint16_t ethype_0, uint16_t ethype_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(uint8_t quad_idx, uint16_t *ethype_0, uint16_t *ethype_1);

/**********************************************************************************************************************
 * ethype_2: 
 *     User Ethertype 2
 * ethype_3: 
 *     User Ethertype 3
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(uint8_t quad_idx, uint16_t ethype_2, uint16_t ethype_3);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(uint8_t quad_idx, uint16_t *ethype_2, uint16_t *ethype_3);

/**********************************************************************************************************************
 * ethtype_user_prot_0: 
 *     Pointer to L3 protocol for User Ethertype 0 (0 - None, 1-IPv4, 2-IPv6)
 * ethtype_user_prot_1: 
 *     Pointer to L3 protocol for User Ethertype 1 (0 - None, 1-IPv4, 2-IPv6)
 * ethtype_user_prot_2: 
 *     Pointer to L3 protocol for User Ethertype 2 (0 - None, 1-IPv4, 2-IPv6)
 * ethtype_user_prot_3: 
 *     Pointer to L3 protocol for User Ethertype 3 (0 - None, 1-IPv4, 2-IPv6)
 * ethtype_user_en: 
 *     Enable user Ethertype 3-0 (LSB is for user ethertype 0)
 * ethtype_user_offset_0: 
 *     4 byte offset for User Ethertype 0 L3
 * ethtype_user_offset_1: 
 *     4 byte offset for User Ethertype 1 L3
 * ethtype_user_offset_2: 
 *     4 byte offset for User Ethertype 2 L3
 * ethtype_user_offset_3: 
 *     4 byte offset for User Ethertype 3 L3
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(uint8_t quad_idx, const rnr_quad_parser_core_configuration_user_ethtype_config *parser_core_configuration_user_ethtype_config);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(uint8_t quad_idx, rnr_quad_parser_core_configuration_user_ethtype_config *parser_core_configuration_user_ethtype_config);

/**********************************************************************************************************************
 * da_filt0_valid: 
 *     DA Filter0 valid bit
 * da_filt1_valid: 
 *     DA Filter1 valid bit
 * da_filt2_valid: 
 *     DA Filter2 valid bit
 * da_filt3_valid: 
 *     DA Filter3 valid bit
 * da_filt4_valid: 
 *     DA Filter4 valid bit
 * da_filt5_valid: 
 *     DA Filter5 valid bit
 * da_filt6_valid: 
 *     DA Filter6 valid bit
 * da_filt7_valid: 
 *     DA Filter7 valid bit
 * da_filt8_valid: 
 *     DA Filter8 valid bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid);

/**********************************************************************************************************************
 * da_filt0_valid: 
 *     DA Filter0 valid bit
 * da_filt1_valid: 
 *     DA Filter1 valid bit
 * da_filt2_valid: 
 *     DA Filter2 valid bit
 * da_filt3_valid: 
 *     DA Filter3 valid bit
 * da_filt4_valid: 
 *     DA Filter4 valid bit
 * da_filt5_valid: 
 *     DA Filter5 valid bit
 * da_filt6_valid: 
 *     DA Filter6 valid bit
 * da_filt7_valid: 
 *     DA Filter7 valid bit
 * da_filt8_valid: 
 *     DA Filter8 valid bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid);

/**********************************************************************************************************************
 * gre_protocol: 
 *     GRE_PROTOCOL
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(uint8_t quad_idx, uint16_t gre_protocol);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get(uint8_t quad_idx, uint16_t *gre_protocol);

/**********************************************************************************************************************
 * size_profile_0: 
 *     profile 0 tag size, valid values are 0,2,4,6,8
 *     
 * size_profile_1: 
 *     profile 1 tag size, valid values are 0,2,4,6,8
 *     
 * size_profile_2: 
 *     profile 2 tag size, valid values are 0,2,4,6,8
 *     
 * pre_da_dprofile_0: 
 *     Pre-DA Profile 0
 * pre_da_dprofile_1: 
 *     Pre-DA Profile 1
 * pre_da_dprofile_2: 
 *     Pre-DA Profile 2
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(uint8_t quad_idx, const rnr_quad_parser_core_configuration_prop_tag_cfg *parser_core_configuration_prop_tag_cfg);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get(uint8_t quad_idx, rnr_quad_parser_core_configuration_prop_tag_cfg *parser_core_configuration_prop_tag_cfg);

/**********************************************************************************************************************
 * mask: 
 *     mask bit per DOS Attack reason. 1 - Attack is enabled. 0 - Attack is disabled
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_dos_attack_set(uint8_t quad_idx, uint16_t mask);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_dos_attack_get(uint8_t quad_idx, uint16_t *mask);

/**********************************************************************************************************************
 * v4_size: 
 *     Max Size for ICMPV4 packet. See DOS Attack detection details
 * v6_size: 
 *     Max Size for ICMPv6 packet. See DOS Attack detection details
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_set(uint8_t quad_idx, uint16_t v4_size, uint16_t v6_size);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_icmp_max_size_get(uint8_t quad_idx, uint16_t *v4_size, uint16_t *v6_size);

/**********************************************************************************************************************
 * l2_tos_mask: 
 *     Mask for L2 KEY TOS Field. Value is ANDed with TOS field
 * l3_tos_mask: 
 *     Mask for L3 KEY TOS Field. Value is ANDed with TOS field
 * l2_exclude_smac: 
 *     Excludes Ethernet Source MAC from L2 Key (field will be set to 0)
 * tcp_pure_ack_mask: 
 *     Mask for Pure ACK field at the result.
 * incude_dei_in_vlans_crc: 
 *     Controls whether DEI bit of VLAN TAG is included or masked before CRC, if masked value of bit is 0
 * key_size: 
 *     Selects 32 Byte or 16 Byte key result mode
 * max_num_of_vlans_in_crc: 
 *     Max number of VLANs in CRC
 * l3_tcp_pure_ack_mask: 
 *     Mask pure_ack at L3 Key. 1 is allow. 0 is blocked.
 * rsrv: 
 *     Reserved
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_key_cfg_set(uint8_t quad_idx, const rnr_quad_parser_core_configuration_key_cfg *parser_core_configuration_key_cfg);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_key_cfg_get(uint8_t quad_idx, rnr_quad_parser_core_configuration_key_cfg *parser_core_configuration_key_cfg);

/**********************************************************************************************************************
 * use_fifo_for_ddr_only: 
 *     Select whether to use DDR FIFO only for DDR accesses
 * token_arbiter_is_rr: 
 *     Scheduling policy for token arbiter
 * chicken_no_flowctrl: 
 *     chicken bit to disable external flow control. Packetw wil always be sent, no matter what token count says
 * flow_ctrl_clear_token: 
 *     Clear token count of external flow control block
 * ddr_congest_threshold: 
 *     Set DDR congestion threshold
 * psram_congest_threshold: 
 *     Set PSRAM congestion threshold
 * enable_reply_threshold: 
 *     Enable reply FIFO occupancy threshold mechanism
 * ddr_reply_threshold: 
 *     Set max reply FIFO occupancy for DDR transactions
 * psram_reply_threshold: 
 *     Set max reply FIFO occupancy for PSRAM transactions
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_dma_arb_cfg_set(uint8_t quad_idx, const rnr_quad_general_config_dma_arb_cfg *general_config_dma_arb_cfg);
bdmf_error_t ag_drv_rnr_quad_general_config_dma_arb_cfg_get(uint8_t quad_idx, rnr_quad_general_config_dma_arb_cfg *general_config_dma_arb_cfg);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_psram0_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram0_base_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_psram1_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram1_base_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_psram2_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram2_base_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_psram3_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram3_base_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_base_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_base_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_psram0_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram0_mask_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_psram1_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram1_mask_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_psram2_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram2_mask_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_psram3_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram3_mask_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_mask_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value for base/mask
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_mask_get(uint8_t quad_idx, uint32_t *val);

/**********************************************************************************************************************
 * counter_lsb_sel: 
 *     Select which 12-bits from 32-bit counter value to be recorded by tracer
 * enable_trace_core_0: 
 *     Enable tracing for core 0
 * enable_trace_core_1: 
 *     Enable tracing for core 1
 * enable_trace_core_2: 
 *     Enable tracing for core 2
 * enable_trace_core_3: 
 *     Enable tracing for core 3
 * enable_trace_core_4: 
 *     Enable tracing for core 4
 * enable_trace_core_5: 
 *     Enable tracing for core 5
 * enable_trace_core_6: 
 *     Enable tracing for core 6
 * enable_trace_core_7: 
 *     Enable tracing for core 7
 * enable_trace_core_8: 
 *     Enable tracing for core 8
 * enable_trace_core_9: 
 *     Enable tracing for core 9
 * enable_trace_core_10: 
 *     Enable tracing for core 10
 * enable_trace_core_11: 
 *     Enable tracing for core 11
 * enable_trace_core_12: 
 *     Enable tracing for core 12
 * enable_trace_core_13: 
 *     Enable tracing for core 13
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_profiling_config_set(uint8_t quad_idx, const rnr_quad_general_config_profiling_config *general_config_profiling_config);
bdmf_error_t ag_drv_rnr_quad_general_config_profiling_config_get(uint8_t quad_idx, rnr_quad_general_config_profiling_config *general_config_profiling_config);

/**********************************************************************************************************************
 * addr: 
 *     Breakpoint address
 * thread: 
 *     Breakpoint address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_0_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);

/**********************************************************************************************************************
 * addr: 
 *     Breakpoint address
 * thread: 
 *     Breakpoint address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_1_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);

/**********************************************************************************************************************
 * addr: 
 *     Breakpoint address
 * thread: 
 *     Breakpoint address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_2_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);

/**********************************************************************************************************************
 * addr: 
 *     Breakpoint address
 * thread: 
 *     Breakpoint address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_3_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);

/**********************************************************************************************************************
 * addr: 
 *     Breakpoint address
 * thread: 
 *     Breakpoint address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_4_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);

/**********************************************************************************************************************
 * addr: 
 *     Breakpoint address
 * thread: 
 *     Breakpoint address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_5_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);

/**********************************************************************************************************************
 * addr: 
 *     Breakpoint address
 * thread: 
 *     Breakpoint address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_6_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);

/**********************************************************************************************************************
 * addr: 
 *     Breakpoint address
 * thread: 
 *     Breakpoint address
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_7_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);

/**********************************************************************************************************************
 * handler_addr: 
 *     Breakpoint handler routine address
 * update_pc_value: 
 *     New PC to be updated by breakpoint handler routine
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(uint8_t quad_idx, uint16_t handler_addr, uint16_t update_pc_value);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(uint8_t quad_idx, uint16_t *handler_addr, uint16_t *update_pc_value);

/**********************************************************************************************************************
 * time_counter: 
 *     Select how many clocks to wait in IDLE condition before enetrin powersave state
 * enable_powersave_core_0: 
 *     Enable powersavingfor core 0
 * enable_powersave_core_1: 
 *     Enable powersave for core 1
 * enable_powersave_core_2: 
 *     Enable powersave for core 2
 * enable_powersave_core_3: 
 *     Enable powersave for core 3
 * enable_powersave_core_4: 
 *     Enable powersave for core 4
 * enable_powersave_core_5: 
 *     Enable powersave for core 5
 * enable_powersave_core_6: 
 *     Enable powersave for core 6
 * enable_powersave_core_7: 
 *     Enable powersave for core 7
 * enable_powersave_core_8: 
 *     Enable powersave for core 8
 * enable_powersave_core_9: 
 *     Enable powersave for core 9
 * enable_powersave_core_10: 
 *     Enable powersave for core 10
 * enable_powersave_core_11: 
 *     Enable powersave for core 11
 * enable_powersave_core_12: 
 *     Enable powersave for core 12
 * enable_powersave_core_13: 
 *     Enable powersave for core 13
 * enable_cpu_if_clk_gating: 
 *     Enable ENABLE_CPU_IF_CLK_GATING (for all cores)
 * enable_common_reg_clk_gating: 
 *     Enable COMMON_REG block clock gating
 * enable_ec_blocks_clk_gating: 
 *     Enable clock gating for EC arbiter and dispatcher
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_powersave_config_set(uint8_t quad_idx, const rnr_quad_general_config_powersave_config *general_config_powersave_config);
bdmf_error_t ag_drv_rnr_quad_general_config_powersave_config_get(uint8_t quad_idx, rnr_quad_general_config_powersave_config *general_config_powersave_config);

/**********************************************************************************************************************
 * acc_status_0: 
 *     Runner 0 accelerators powersaving status
 * acc_status_1: 
 *     Runner 1 accelerators powersaving status
 * acc_status_2: 
 *     Runner 2 accelerators powersaving status
 * acc_status_3: 
 *     Runner 3 accelerators powersaving status
 * acc_status_4: 
 *     Runner 4 accelerators powersaving status
 * acc_status_5: 
 *     Runner 5 accelerators powersaving status
 * acc_status_6: 
 *     Runner 6 accelerators powersaving status
 * acc_status_7: 
 *     Runner 7 accelerators powersaving status
 * acc_status_8: 
 *     Runner 8 accelerators powersaving status
 * acc_status_9: 
 *     Runner 9 accelerators powersaving status
 * acc_status_10: 
 *     Runner 10 accelerators powersaving status
 * acc_status_11: 
 *     Runner 11 accelerators powersaving status
 * acc_status_12: 
 *     Runner 12 accelerators powersaving status
 * acc_status_13: 
 *     Runner 13 accelerators powersaving status
 * core_status_0: 
 *     Runner 0 core powersaving status
 * core_status_1: 
 *     Runner 1 core powersaving status
 * core_status_2: 
 *     Runner 2 core powersaving status
 * core_status_3: 
 *     Runner 3 core powersaving status
 * core_status_4: 
 *     Runner 4 core powersaving status
 * core_status_5: 
 *     Runner 5 core powersaving status
 * core_status_6: 
 *     Runner 6 core powersaving status
 * core_status_7: 
 *     Runner 7 core powersaving status
 * core_status_8: 
 *     Runner 8 core powersaving status
 * core_status_9: 
 *     Runner 9 core powersaving status
 * core_status_10: 
 *     Runner 10 core powersaving status
 * core_status_11: 
 *     Runner 11 core powersaving status
 * core_status_12: 
 *     Runner 12 core powersaving status
 * core_status_13: 
 *     Runner 13 core powersaving status
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_powersave_status_get(uint8_t quad_idx, rnr_quad_general_config_powersave_status *general_config_powersave_status);

/**********************************************************************************************************************
 * data_addr_start: 
 *     Data address start.
 * data_addr_stop: 
 *     Data address stop
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_set(uint8_t quad_idx, uint16_t data_addr_start, uint16_t data_addr_stop);
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_0_cfg_get(uint8_t quad_idx, uint16_t *data_addr_start, uint16_t *data_addr_stop);

/**********************************************************************************************************************
 * data_addr_start: 
 *     Data address start.
 * data_addr_stop: 
 *     Data address stop
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_set(uint8_t quad_idx, uint16_t data_addr_start, uint16_t data_addr_stop);
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_1_cfg_get(uint8_t quad_idx, uint16_t *data_addr_start, uint16_t *data_addr_stop);

/**********************************************************************************************************************
 * data_addr_start: 
 *     Data address start.
 * data_addr_stop: 
 *     Data address stop
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_set(uint8_t quad_idx, uint16_t data_addr_start, uint16_t data_addr_stop);
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_2_cfg_get(uint8_t quad_idx, uint16_t *data_addr_start, uint16_t *data_addr_stop);

/**********************************************************************************************************************
 * data_addr_start: 
 *     Data address start.
 * data_addr_stop: 
 *     Data address stop
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_set(uint8_t quad_idx, uint16_t data_addr_start, uint16_t data_addr_stop);
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_3_cfg_get(uint8_t quad_idx, uint16_t *data_addr_start, uint16_t *data_addr_stop);

/**********************************************************************************************************************
 * thread_0: 
 *     Thread for bkpt 0
 * thread_1: 
 *     Thread for bkpt 1
 * thread_2: 
 *     Thread for bkpt 2
 * thread_3: 
 *     Thread for bkpt 3
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_set(uint8_t quad_idx, uint8_t thread_0, uint8_t thread_1, uint8_t thread_2, uint8_t thread_3);
bdmf_error_t ag_drv_rnr_quad_general_config_data_bkpt_common_cfg_get(uint8_t quad_idx, uint8_t *thread_0, uint8_t *thread_1, uint8_t *thread_2, uint8_t *thread_3);

/**********************************************************************************************************************
 * enable_statistics: 
 *     Enable statistics
 * sw_reset: 
 *     Writing 1 resets all the counters
 * dest_pid: 
 *     Destination PID,controls which destination PID transactions are counted (e.g. can be programmed either to PSRAM
 *     or DDR)
 *     
 * master_select: 
 *     Selects which master is measured
 *     
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_ubus_counter_control_set(uint8_t quad_idx, bdmf_boolean enable_statistics, bdmf_boolean sw_reset, uint8_t dest_pid, uint8_t master_select);
bdmf_error_t ag_drv_rnr_quad_general_config_ubus_counter_control_get(uint8_t quad_idx, bdmf_boolean *enable_statistics, bdmf_boolean *sw_reset, uint8_t *dest_pid, uint8_t *master_select);

/**********************************************************************************************************************
 * downcnt_value: 
 *     Set the size of the window.
 *     When Statistics are enabled this counter counts down, while it is not 0 the statistics are collected - i.e
 *     below statistics are updated as long as this counter counts down.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_ubus_down_counter_set(uint8_t quad_idx, uint32_t downcnt_value);
bdmf_error_t ag_drv_rnr_quad_general_config_ubus_down_counter_get(uint8_t quad_idx, uint32_t *downcnt_value);

/**********************************************************************************************************************
 * counter_value: 
 *     Value
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_all_xfers_cnt_get(uint8_t quad_idx, uint32_t *counter_value);

/**********************************************************************************************************************
 * counter_value: 
 *     Value
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_read_xfers_cnt_get(uint8_t quad_idx, uint32_t *counter_value);

/**********************************************************************************************************************
 * counter_value: 
 *     Value
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_read_data_cnt_get(uint8_t quad_idx, uint32_t *counter_value);

/**********************************************************************************************************************
 * counter_value: 
 *     Value
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_write_data_cnt_get(uint8_t quad_idx, uint32_t *counter_value);

/**********************************************************************************************************************
 * ddr_pid: 
 *     Set value for DDR PID to be used for determining which transactions go to dedicated DDR virtual channel
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_misc_cfg_set(uint8_t quad_idx, uint8_t ddr_pid);
bdmf_error_t ag_drv_rnr_quad_general_config_misc_cfg_get(uint8_t quad_idx, uint8_t *ddr_pid);

/**********************************************************************************************************************
 * enable_counter: 
 *     Enable AQM counter. Disable resets the counter.
 * enable_random: 
 *     Enable AQM random generator. Disable resets the counter.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_aqm_control_set(uint8_t quad_idx, bdmf_boolean enable_counter, bdmf_boolean enable_random);
bdmf_error_t ag_drv_rnr_quad_general_config_aqm_control_get(uint8_t quad_idx, bdmf_boolean *enable_counter, bdmf_boolean *enable_random);

/**********************************************************************************************************************
 * random_value: 
 *     AQM random generator value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_aqm_randm_value_get(uint8_t quad_idx, uint32_t *random_value);

/**********************************************************************************************************************
 * random_seed: 
 *     Set AQM random generator seed
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_aqm_random_seed_set(uint8_t quad_idx, uint32_t random_seed);
bdmf_error_t ag_drv_rnr_quad_general_config_aqm_random_seed_get(uint8_t quad_idx, uint32_t *random_seed);

/**********************************************************************************************************************
 * random_inc: 
 *     Inc AQM random generator (for test) by writing 1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_aqm_random_test_inc_set(uint8_t quad_idx, bdmf_boolean random_inc);

/**********************************************************************************************************************
 * multi_psel_master_sel: 
 *     Select which select will be treated as multiple write. Only single bit can be set to 1 at any given time.
 * multi_psel_mask: 
 *     Select which clients will get write transaction if it arrives on master psel. Must set 1 for Runner which is
 *     configured as master by MULTI_PSEL_MASTER_SEL field
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_general_config_multi_psel_cfg_set(uint8_t quad_idx, uint16_t multi_psel_master_sel, uint16_t multi_psel_mask);
bdmf_error_t ag_drv_rnr_quad_general_config_multi_psel_cfg_get(uint8_t quad_idx, uint16_t *multi_psel_master_sel, uint16_t *multi_psel_mask);

/**********************************************************************************************************************
 * psram_hdr_sw_rst_0: 
 *     Apply software reset to PSRAM header FIFO in EC arbiter
 * psram_data_sw_rst_0: 
 *     Apply software reset to PSRAM data FIFO in EC arbiter
 * ddr_hdr_sw_rst_0: 
 *     Apply software reset to DDR header FIFO in EC arbiter
 * select_fifos_for_debug: 
 *     Select for which arbiter to display FIFO debug data
 * psram_hdr_sw_rst_1: 
 *     Apply software reset to PSRAM header FIFO in EC arbiter
 * psram_data_sw_rst_1: 
 *     Apply software reset to PSRAM data FIFO in EC arbiter
 * ddr_hdr_sw_rst_1: 
 *     Apply software reset to DDR header FIFO in EC arbiter
 * psram_hdr_sw_rd_addr: 
 *     Software read address for PSRAM header FIFO
 * psram_data_sw_rd_addr: 
 *     Software read address for PSRAM data FIFO
 * ddr_hdr_sw_rd_addr: 
 *     Software read address for DDR header FIFO
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_fifo_config_set(uint8_t quad_idx, const rnr_quad_debug_fifo_config *debug_fifo_config);
bdmf_error_t ag_drv_rnr_quad_debug_fifo_config_get(uint8_t quad_idx, rnr_quad_debug_fifo_config *debug_fifo_config);

/**********************************************************************************************************************
 * full: 
 *     FIFO full indication
 * empty: 
 *     FIFO empty indication
 * push_wr_cntr: 
 *     Push write counter value
 * pop_rd_cntr: 
 *     Pop read counter value
 * used_words: 
 *     Used words value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_psram_hdr_fifo_status *debug_psram_hdr_fifo_status);

/**********************************************************************************************************************
 * full: 
 *     FIFO full indication
 * empty: 
 *     FIFO empty indication
 * almost_full: 
 *     Almost FIFO full indication
 * push_wr_cntr: 
 *     Push write counter value
 * pop_rd_cntr: 
 *     Pop read counter value
 * used_words: 
 *     Used words value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_psram_data_fifo_status *debug_psram_data_fifo_status);

/**********************************************************************************************************************
 * full: 
 *     FIFO full indication
 * empty: 
 *     FIFO empty indication
 * push_wr_cntr: 
 *     Push write counter value
 * pop_rd_cntr: 
 *     Pop read counter value
 * used_words: 
 *     Used words value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_ddr_hdr_fifo_status *debug_ddr_hdr_fifo_status);

/**********************************************************************************************************************
 * full: 
 *     FIFO full indication
 * empty: 
 *     FIFO empty indication
 * almost_full: 
 *     Almost FIFO full indication
 * wr_cntr: 
 *     rite counter value
 * rd_cntr: 
 *     Read counter value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_ddr_data_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_ddr_data_fifo_status *debug_ddr_data_fifo_status);

/**********************************************************************************************************************
 * read_addr: 
 *     Current read address
 * used_words: 
 *     Used words
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get(uint8_t quad_idx, uint8_t *read_addr, uint16_t *used_words);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get(uint8_t quad_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get(uint8_t quad_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_data1_get(uint8_t quad_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_data2_get(uint8_t quad_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get(uint8_t quad_idx, uint32_t *data);

/**********************************************************************************************************************
 * data: 
 *     Data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get(uint8_t quad_idx, uint32_t *data);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(uint8_t quad_idx, uint32_t index, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config2_token_val_2_get(uint8_t quad_idx, uint32_t index, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(uint8_t quad_idx, uint32_t index, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(uint8_t quad_idx, uint32_t index, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2_get(uint8_t quad_idx, uint32_t index, uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     Value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2_get(uint8_t quad_idx, uint32_t index, uint32_t *val);

#ifdef USE_BDMF_SHELL
enum
{
    cli_rnr_quad_parser_vid0,
    cli_rnr_quad_parser_vid1,
    cli_rnr_quad_parser_vid2,
    cli_rnr_quad_parser_vid3,
    cli_rnr_quad_parser_vid4,
    cli_rnr_quad_parser_vid5,
    cli_rnr_quad_parser_vid6,
    cli_rnr_quad_parser_vid7,
    cli_rnr_quad_parser_ip0,
    cli_rnr_quad_parser_ip1,
    cli_rnr_quad_parser_hardcoded_ethtype_prof0,
    cli_rnr_quad_parser_hardcoded_ethtype_prof1,
    cli_rnr_quad_parser_hardcoded_ethtype_prof2,
    cli_rnr_quad_parser_qtag_nest_prof0,
    cli_rnr_quad_parser_qtag_nest_prof1,
    cli_rnr_quad_parser_qtag_nest_prof2,
    cli_rnr_quad_parser_qtag_nest_max_vlans,
    cli_rnr_quad_parser_ip_protocol0,
    cli_rnr_quad_parser_ip_protocol1,
    cli_rnr_quad_parser_ip_protocol2,
    cli_rnr_quad_parser_ip_protocol3,
    cli_rnr_quad_parser_da_filter,
    cli_rnr_quad_parser_da_filter1,
    cli_rnr_quad_parser_da_filter2,
    cli_rnr_quad_parser_da_filter3,
    cli_rnr_quad_parser_da_filter4,
    cli_rnr_quad_parser_da_filter5,
    cli_rnr_quad_parser_da_filter6,
    cli_rnr_quad_parser_da_filter7,
    cli_rnr_quad_parser_da_filter8,
    cli_rnr_quad_da_filter_valid,
    cli_rnr_quad_exception_bits,
    cli_rnr_quad_tcp_flags,
    cli_rnr_quad_profile_us,
    cli_rnr_quad_disable_l2tp_source_port,
    cli_rnr_quad_parser_snap_conf,
    cli_rnr_quad_parser_ipv6_filter,
    cli_rnr_quad_parser_core_configuration_eng,
    cli_rnr_quad_parser_core_configuration_ppp_ip_prot_code,
    cli_rnr_quad_parser_core_configuration_qtag_ethtype,
    cli_rnr_quad_parser_core_configuration_user_ethtype_0_1,
    cli_rnr_quad_parser_core_configuration_user_ethtype_2_3,
    cli_rnr_quad_parser_core_configuration_user_ethtype_config,
    cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1,
    cli_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2,
    cli_rnr_quad_parser_core_configuration_gre_protocol_cfg,
    cli_rnr_quad_parser_core_configuration_prop_tag_cfg,
    cli_rnr_quad_parser_core_configuration_dos_attack,
    cli_rnr_quad_parser_core_configuration_icmp_max_size,
    cli_rnr_quad_parser_core_configuration_key_cfg,
    cli_rnr_quad_general_config_dma_arb_cfg,
    cli_rnr_quad_general_config_psram0_base,
    cli_rnr_quad_general_config_psram1_base,
    cli_rnr_quad_general_config_psram2_base,
    cli_rnr_quad_general_config_psram3_base,
    cli_rnr_quad_general_config_ddr0_base,
    cli_rnr_quad_general_config_ddr1_base,
    cli_rnr_quad_general_config_psram0_mask,
    cli_rnr_quad_general_config_psram1_mask,
    cli_rnr_quad_general_config_psram2_mask,
    cli_rnr_quad_general_config_psram3_mask,
    cli_rnr_quad_general_config_ddr0_mask,
    cli_rnr_quad_general_config_ddr1_mask,
    cli_rnr_quad_general_config_profiling_config,
    cli_rnr_quad_general_config_bkpt_0_cfg,
    cli_rnr_quad_general_config_bkpt_1_cfg,
    cli_rnr_quad_general_config_bkpt_2_cfg,
    cli_rnr_quad_general_config_bkpt_3_cfg,
    cli_rnr_quad_general_config_bkpt_4_cfg,
    cli_rnr_quad_general_config_bkpt_5_cfg,
    cli_rnr_quad_general_config_bkpt_6_cfg,
    cli_rnr_quad_general_config_bkpt_7_cfg,
    cli_rnr_quad_general_config_bkpt_gen_cfg,
    cli_rnr_quad_general_config_powersave_config,
    cli_rnr_quad_general_config_powersave_status,
    cli_rnr_quad_general_config_data_bkpt_0_cfg,
    cli_rnr_quad_general_config_data_bkpt_1_cfg,
    cli_rnr_quad_general_config_data_bkpt_2_cfg,
    cli_rnr_quad_general_config_data_bkpt_3_cfg,
    cli_rnr_quad_general_config_data_bkpt_common_cfg,
    cli_rnr_quad_general_config_ubus_counter_control,
    cli_rnr_quad_general_config_ubus_down_counter,
    cli_rnr_quad_general_config_all_xfers_cnt,
    cli_rnr_quad_general_config_read_xfers_cnt,
    cli_rnr_quad_general_config_read_data_cnt,
    cli_rnr_quad_general_config_write_data_cnt,
    cli_rnr_quad_general_config_misc_cfg,
    cli_rnr_quad_general_config_aqm_control,
    cli_rnr_quad_general_config_aqm_randm_value,
    cli_rnr_quad_general_config_aqm_random_seed,
    cli_rnr_quad_general_config_aqm_random_test_inc,
    cli_rnr_quad_general_config_multi_psel_cfg,
    cli_rnr_quad_debug_fifo_config,
    cli_rnr_quad_debug_psram_hdr_fifo_status,
    cli_rnr_quad_debug_psram_data_fifo_status,
    cli_rnr_quad_debug_ddr_hdr_fifo_status,
    cli_rnr_quad_debug_ddr_data_fifo_status,
    cli_rnr_quad_debug_ddr_data_fifo_status2,
    cli_rnr_quad_debug_psram_hdr_fifo_data1,
    cli_rnr_quad_debug_psram_hdr_fifo_data2,
    cli_rnr_quad_debug_psram_data_fifo_data1,
    cli_rnr_quad_debug_psram_data_fifo_data2,
    cli_rnr_quad_debug_ddr_hdr_fifo_data1,
    cli_rnr_quad_debug_ddr_hdr_fifo_data2,
    cli_rnr_quad_ext_flowctrl_config_token_val,
    cli_rnr_quad_ext_flowctrl_config2_token_val_2,
    cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode,
    cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode,
    cli_rnr_quad_ubus_decode_cfg2_psram_ubus_decode2,
    cli_rnr_quad_ubus_decode_cfg2_ddr_ubus_decode2,
};

int bcm_rnr_quad_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_rnr_quad_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
