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

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* ip_address: IP_address - 32-bit address to match SIP or DIP (according to predefined configura */
/*             tion in IP_FILTERS_CFG register)                                                   */
/* ip_address_mask: IP_address_mask - 32-bit address mask                                         */
/* ip_filter0_dip_en: IP_FILTER0_DIP_EN - IP Filter0 DIP or SIP selection.The default is SIP, whe */
/*                    n the field is set -> DIP selection is enabled                              */
/* ip_filter0_valid: IP_FILTER0_VALID - IP Filter0 valid bit.When the bit valid is set, the IP fi */
/*                   lter/mask can be applied by hardware.                                        */
/**************************************************************************************************/
typedef struct
{
    uint32_t ip_address;
    uint32_t ip_address_mask;
    bdmf_boolean ip_filter0_dip_en;
    bdmf_boolean ip_filter0_valid;
} rnr_quad_parser_ip0;


/**************************************************************************************************/
/* da_filt_msb: DA_FILT_MSB - Current DA Filter bits 47:32                                        */
/* da_filt_lsb: DA_FILT_LSB - DA Filter bits 31:0                                                 */
/* da_filt_mask_msb: DA_FILT_MASK_MSB - Current DA Filter mask bits 47:32                         */
/* da_filt_mask_l: DA_FILT_MASK_L - Current DA Filter mask bits 31:0                              */
/**************************************************************************************************/
typedef struct
{
    uint16_t da_filt_msb;
    uint32_t da_filt_lsb;
    uint16_t da_filt_mask_msb;
    uint32_t da_filt_mask_l;
} rnr_quad_parser_da_filter;


/**************************************************************************************************/
/* da_filt0_valid: DA_FILT0_VALID - DA Filter0 valid bit                                          */
/* da_filt1_valid: DA_FILT1_VALID - DA Filter1 valid bit                                          */
/* da_filt2_valid: DA_FILT2_VALID - DA Filter2 valid bit                                          */
/* da_filt3_valid: DA_FILT3_VALID - DA Filter3 valid bit                                          */
/* da_filt4_valid: DA_FILT4_VALID - DA Filter4 valid bit                                          */
/* da_filt5_valid: DA_FILT5_VALID - DA Filter5 valid bit                                          */
/* da_filt6_valid: DA_FILT6_VALID - DA Filter6 valid bit                                          */
/* da_filt7_valid: DA_FILT7_VALID - DA Filter7 valid bit                                          */
/* da_filt8_valid: DA_FILT8_VALID - DA Filter8 valid bit                                          */
/**************************************************************************************************/
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


/**************************************************************************************************/
/* code: Organization_Code - Used defined SNAP organization code                                  */
/* en_rfc1042: RFC1042_ethernet_encapsulation_enable - enable RFC1042 0x00000 organization code   */
/* en_8021q: 802.1Q_ehternet_encapsulation - enables 802.1Q 0x0000f8 organization code            */
/**************************************************************************************************/
typedef struct
{
    uint32_t code;
    bdmf_boolean en_rfc1042;
    bdmf_boolean en_8021q;
} rnr_quad_parser_snap_conf;


/**************************************************************************************************/
/* hop_by_hop_match: hop_by_hop_match - hop by hop match filter mask                              */
/* routing_eh: routing_eh - Routing extension header option match filter mask                     */
/* dest_opt_eh: dest_opt_eh - Destination Options extension header option match filter mask       */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean hop_by_hop_match;
    bdmf_boolean routing_eh;
    bdmf_boolean dest_opt_eh;
} rnr_quad_parser_ipv6_filter;


/**************************************************************************************************/
/* mask_id: mask_id - Mask id. masks the port_id entering the bridge (ubus2)                      */
/* repin_eswap: repin_eswap - repin endian swap                                                   */
/* reqout_eswap: reqout_eswap - reqout endian swap                                                */
/* dev_clken: dev_clk_en - device clock enable                                                    */
/* dev_err: dev_error - indicate an error on Ubus                                                 */
/* dev_timeout_en: dev_timeout_en - enables timeout                                               */
/* dev_timeout: dev_timeout - device timeout                                                      */
/**************************************************************************************************/
typedef struct
{
    uint8_t mask_id;
    bdmf_boolean repin_eswap;
    bdmf_boolean reqout_eswap;
    bdmf_boolean dev_clken;
    bdmf_boolean dev_err;
    bdmf_boolean dev_timeout_en;
    uint16_t dev_timeout;
} rnr_quad_ubus_slv_res_cntrl;


/**************************************************************************************************/
/* ethtype_user_prot_0: User_Ethertype_0_protocol - Pointer to L3 protocol for User Ethertype 0 ( */
/*                      0 - None, 1-IPv4, 2-IPv6)                                                 */
/* ethtype_user_prot_1: User_Ethertype_1 - Pointer to L3 protocol for User Ethertype 1 (0 - None, */
/*                       1-IPv4, 2-IPv6)                                                          */
/* ethtype_user_prot_2: User_Ethertype_2 - Pointer to L3 protocol for User Ethertype 2 (0 - None, */
/*                       1-IPv4, 2-IPv6)                                                          */
/* ethtype_user_prot_3: User_Ethertype_3 - Pointer to L3 protocol for User Ethertype 3 (0 - None, */
/*                       1-IPv4, 2-IPv6)                                                          */
/* ethtype_user_en: User_Ethertype_Enable - Enable user Ethertype 3-0 (LSB is for user ethertype  */
/*                  0)                                                                            */
/* ethtype_user_offset_0: User_Ethertype_0_L3_Offset - 4 byte offset for User Ethertype 0 L3      */
/* ethtype_user_offset_1: User_Ethertype_1_L3_Offset - 4 byte offset for User Ethertype 1 L3      */
/* ethtype_user_offset_2: User_Ethertype_2_L3_Offset - 4 byte offset for User Ethertype 2 L3      */
/* ethtype_user_offset_3: User_Ethertype_2_L3_Offset - 4 byte offset for User Ethertype 3 L3      */
/**************************************************************************************************/
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


/**************************************************************************************************/
/* size_profile_0: SIZE_PROFILE_0 - profile 0 tag size, valid values are 0,2,4,6,8                */
/* size_profile_1: SIZE_PROFILE_1 - profile 1 tag size, valid values are 0,2,4,6,8                */
/* size_profile_2: SIZE_PROFILE_2 - profile 2 tag size, valid values are 0,2,4,6,8                */
/* pre_da_dprofile_0: PRE_DA_DPROFILE_0 - Pre-DA Profile 0                                        */
/* pre_da_dprofile_1: PRE_DA_DPROFILE_1 - Pre-DA Profile 1                                        */
/* pre_da_dprofile_2: PRE_DA_DPROFILE_2 - Pre-DA Profile 2                                        */
/**************************************************************************************************/
typedef struct
{
    uint8_t size_profile_0;
    uint8_t size_profile_1;
    uint8_t size_profile_2;
    bdmf_boolean pre_da_dprofile_0;
    bdmf_boolean pre_da_dprofile_1;
    bdmf_boolean pre_da_dprofile_2;
} rnr_quad_parser_core_configuration_prop_tag_cfg;


/**************************************************************************************************/
/* use_fifo_for_ddr_only: USE_FIFO_FOR_DDR_ONLY - Select whether to use DDR FIFO only for DDR acc */
/*                        esses                                                                   */
/* token_arbiter_is_rr: TOKEN_ARBITER_IS_RR - Scheduling policy for token arbiter                 */
/* chicken_no_flowctrl: CHICKEN_NO_FLOWCTRL - chicken bit to disable external flow control. Packe */
/*                      tw wil always be sent, no matter what token count says                    */
/* ddr_congest_threshold: DDR_CONGEST_THRESHOLD - Set DDR congestion threshold                    */
/* psram_congest_threshold: PSRAM_CONGEST_THRESHOLD - Set PSRAM congestion threshold              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean use_fifo_for_ddr_only;
    bdmf_boolean token_arbiter_is_rr;
    bdmf_boolean chicken_no_flowctrl;
    uint8_t ddr_congest_threshold;
    uint8_t psram_congest_threshold;
} rnr_quad_general_config_dma_arb_cfg;


/**************************************************************************************************/
/* counter_lsb_sel: COUNTER_LSB_SEL - Select which 12-bits from 32-bit counter value to be record */
/*                  ed by tracer                                                                  */
/* enable_trace_core_0: ENABLE_TRACE_CORE_0 - Enable tracing for core 0                           */
/* enable_trace_core_1: ENABLE_TRACE_CORE_1 - Enable tracing for core 1                           */
/* enable_trace_core_2: ENABLE_TRACE_CORE_2 - Enable tracing for core 2                           */
/* enable_trace_core_3: ENABLE_TRACE_CORE_3 - Enable tracing for core 3                           */
/* enable_trace_core_4: ENABLE_TRACE_CORE_4 - Enable tracing for core 4                           */
/* enable_trace_core_5: ENABLE_TRACE_CORE_5 - Enable tracing for core 5                           */
/**************************************************************************************************/
typedef struct
{
    uint8_t counter_lsb_sel;
    bdmf_boolean enable_trace_core_0;
    bdmf_boolean enable_trace_core_1;
    bdmf_boolean enable_trace_core_2;
    bdmf_boolean enable_trace_core_3;
    bdmf_boolean enable_trace_core_4;
    bdmf_boolean enable_trace_core_5;
} rnr_quad_general_config_profiling_config;


/**************************************************************************************************/
/* time_counter: TIME_COUNTER - Select how many clocks to wait in IDLE condition before enetrin p */
/*               owersave state                                                                   */
/* enable_powersave_core_0: ENABLE_POWERSAVE_CORE_0 - Enable powersavingfor core 0                */
/* enable_powersave_core_1: ENABLE_POWERSAVE_CORE_1 - Enable powersave for core 1                 */
/* enable_powersave_core_2: ENABLE_POWERSAVE_CORE_2 - Enable powersave for core 2                 */
/* enable_powersave_core_3: ENABLE_POWERSAVE_CORE_3 - Enable powersave for core 3                 */
/* enable_powersave_core_4: ENABLE_POWERSAVE_CORE_4 - Enable powersave for core 4                 */
/* enable_powersave_core_5: ENABLE_POWERSAVE_CORE_5 - Enable powersave for core 5                 */
/**************************************************************************************************/
typedef struct
{
    uint8_t time_counter;
    bdmf_boolean enable_powersave_core_0;
    bdmf_boolean enable_powersave_core_1;
    bdmf_boolean enable_powersave_core_2;
    bdmf_boolean enable_powersave_core_3;
    bdmf_boolean enable_powersave_core_4;
    bdmf_boolean enable_powersave_core_5;
} rnr_quad_general_config_powersave_config;


/**************************************************************************************************/
/* psram_hdr_sw_rst: PSRAM_HDR_SW_RST - Apply software reset to PSRAM header FIFO in EC arbiter   */
/* psram_data_sw_rst: PSRAM_DATA_SW_RST - Apply software reset to PSRAM data FIFO in EC arbiter   */
/* ddr_hdr_sw_rst: DDR_HDR_SW_RST - Apply software reset to DDR header FIFO in EC arbiter         */
/* psram_hdr_sw_rd_addr: PSRAM_HDR_SW_RD_ADDR - Software read address for PSRAM header FIFO       */
/* psram_data_sw_rd_addr: PSRAM_DATA_SW_RD_ADDR - Software read address for PSRAM data FIFO       */
/* ddr_hdr_sw_rd_addr: DDR_HDR_SW_RD_ADDR - Software read address for DDR header FIFO             */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean psram_hdr_sw_rst;
    bdmf_boolean psram_data_sw_rst;
    bdmf_boolean ddr_hdr_sw_rst;
    uint8_t psram_hdr_sw_rd_addr;
    uint8_t psram_data_sw_rd_addr;
    uint8_t ddr_hdr_sw_rd_addr;
} rnr_quad_debug_fifo_config;


/**************************************************************************************************/
/* full: FULL - FIFO full indication                                                              */
/* empty: EMPTY - FIFO empty indication                                                           */
/* push_wr_cntr: PUSH_WR_CNTR - Push write counter value                                          */
/* pop_rd_cntr: POP_RD_CNTR - Pop read counter value                                              */
/* used_words: USED_WORDS - Used words value                                                      */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean full;
    bdmf_boolean empty;
    uint8_t push_wr_cntr;
    uint8_t pop_rd_cntr;
    uint8_t used_words;
} rnr_quad_debug_psram_hdr_fifo_status;


/**************************************************************************************************/
/* full: FULL - FIFO full indication                                                              */
/* empty: EMPTY - FIFO empty indication                                                           */
/* almost_full: ALMOST_FULL - Almost FIFO full indication                                         */
/* push_wr_cntr: PUSH_WR_CNTR - Push write counter value                                          */
/* pop_rd_cntr: POP_RD_CNTR - Pop read counter value                                              */
/* used_words: USED_WORDS - Used words value                                                      */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean full;
    bdmf_boolean empty;
    bdmf_boolean almost_full;
    uint8_t push_wr_cntr;
    uint8_t pop_rd_cntr;
    uint8_t used_words;
} rnr_quad_debug_psram_data_fifo_status;


/**************************************************************************************************/
/* full: FULL - FIFO full indication                                                              */
/* empty: EMPTY - FIFO empty indication                                                           */
/* push_wr_cntr: PUSH_WR_CNTR - Push write counter value                                          */
/* pop_rd_cntr: POP_RD_CNTR - Pop read counter value                                              */
/* used_words: USED_WORDS - Used words value                                                      */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean full;
    bdmf_boolean empty;
    uint8_t push_wr_cntr;
    uint8_t pop_rd_cntr;
    uint8_t used_words;
} rnr_quad_debug_ddr_hdr_fifo_status;


/**************************************************************************************************/
/* full: FULL - FIFO full indication                                                              */
/* empty: EMPTY - FIFO empty indication                                                           */
/* almost_full: ALMOST_FULL - Almost FIFO full indication                                         */
/* wr_cntr: WR_CNTR - rite counter value                                                          */
/* rd_cntr: RD_CNTR - Read counter value                                                          */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean full;
    bdmf_boolean empty;
    bdmf_boolean almost_full;
    uint16_t wr_cntr;
    uint16_t rd_cntr;
} rnr_quad_debug_ddr_data_fifo_status;

bdmf_error_t ag_drv_rnr_quad_parser_vid0_set(uint8_t quad_idx, uint16_t vid_0, bdmf_boolean vid_0_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid0_get(uint8_t quad_idx, uint16_t *vid_0, bdmf_boolean *vid_0_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid1_set(uint8_t quad_idx, uint16_t vid_1, bdmf_boolean vid_1_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid1_get(uint8_t quad_idx, uint16_t *vid_1, bdmf_boolean *vid_1_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid2_set(uint8_t quad_idx, uint16_t vid_2, bdmf_boolean vid_2_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid2_get(uint8_t quad_idx, uint16_t *vid_2, bdmf_boolean *vid_2_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid3_set(uint8_t quad_idx, uint16_t vid_3, bdmf_boolean vid_3_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid3_get(uint8_t quad_idx, uint16_t *vid_3, bdmf_boolean *vid_3_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid4_set(uint8_t quad_idx, uint16_t vid_4, bdmf_boolean vid_4_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid4_get(uint8_t quad_idx, uint16_t *vid_4, bdmf_boolean *vid_4_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid5_set(uint8_t quad_idx, uint16_t vid_5, bdmf_boolean vid_5_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid5_get(uint8_t quad_idx, uint16_t *vid_5, bdmf_boolean *vid_5_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid6_set(uint8_t quad_idx, uint16_t vid_6, bdmf_boolean vid_6_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid6_get(uint8_t quad_idx, uint16_t *vid_6, bdmf_boolean *vid_6_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid7_set(uint8_t quad_idx, uint16_t vid_7, bdmf_boolean vid_7_en);
bdmf_error_t ag_drv_rnr_quad_parser_vid7_get(uint8_t quad_idx, uint16_t *vid_7, bdmf_boolean *vid_7_en);
bdmf_error_t ag_drv_rnr_quad_parser_ip0_set(uint8_t quad_idx, const rnr_quad_parser_ip0 *parser_ip0);
bdmf_error_t ag_drv_rnr_quad_parser_ip0_get(uint8_t quad_idx, rnr_quad_parser_ip0 *parser_ip0);
bdmf_error_t ag_drv_rnr_quad_parser_ip1_set(uint8_t quad_idx, const rnr_quad_parser_ip0 *parser_ip0);
bdmf_error_t ag_drv_rnr_quad_parser_ip1_get(uint8_t quad_idx, rnr_quad_parser_ip0 *parser_ip0);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_set(uint8_t quad_idx, uint16_t hard_nest_profile);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof0_get(uint8_t quad_idx, uint16_t *hard_nest_profile);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_set(uint8_t quad_idx, uint16_t hard_nest_profile);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof1_get(uint8_t quad_idx, uint16_t *hard_nest_profile);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_set(uint8_t quad_idx, uint16_t hard_nest_profile);
bdmf_error_t ag_drv_rnr_quad_parser_hardcoded_ethtype_prof2_get(uint8_t quad_idx, uint16_t *hard_nest_profile);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof0_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_0, uint8_t qtag_nest_1_profile_0);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof0_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_0, uint8_t *qtag_nest_1_profile_0);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof1_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_1, uint8_t qtag_nest_1_profile_1);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof1_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_1, uint8_t *qtag_nest_1_profile_1);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof2_set(uint8_t quad_idx, uint8_t qtag_nest_0_profile_2, uint8_t qtag_nest_1_profile_2);
bdmf_error_t ag_drv_rnr_quad_parser_qtag_nest_prof2_get(uint8_t quad_idx, uint8_t *qtag_nest_0_profile_2, uint8_t *qtag_nest_1_profile_2);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol0_set(uint8_t quad_idx, uint8_t user_ip_prot_0);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol0_get(uint8_t quad_idx, uint8_t *user_ip_prot_0);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol1_set(uint8_t quad_idx, uint8_t user_ip_prot_1);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol1_get(uint8_t quad_idx, uint8_t *user_ip_prot_1);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol2_set(uint8_t quad_idx, uint8_t user_ip_prot_2);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol2_get(uint8_t quad_idx, uint8_t *user_ip_prot_2);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol3_set(uint8_t quad_idx, uint8_t user_ip_prot_3);
bdmf_error_t ag_drv_rnr_quad_parser_ip_protocol3_get(uint8_t quad_idx, uint8_t *user_ip_prot_3);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter_set(uint8_t quad_idx, const rnr_quad_parser_da_filter *parser_da_filter);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter_get(uint8_t quad_idx, rnr_quad_parser_da_filter *parser_da_filter);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter1_set(uint8_t quad_idx, const rnr_quad_parser_da_filter *parser_da_filter);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter1_get(uint8_t quad_idx, rnr_quad_parser_da_filter *parser_da_filter);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter2_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter2_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter3_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter3_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter4_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter4_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter5_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter5_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter6_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter6_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter7_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter7_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter8_set(uint8_t quad_idx, uint16_t da_filt_msb, uint32_t da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_parser_da_filter8_get(uint8_t quad_idx, uint16_t *da_filt_msb, uint32_t *da_filt_lsb);
bdmf_error_t ag_drv_rnr_quad_da_filter_valid_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_da_filter_valid_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_exception_bits_set(uint8_t quad_idx, uint16_t exception_en);
bdmf_error_t ag_drv_rnr_quad_exception_bits_get(uint8_t quad_idx, uint16_t *exception_en);
bdmf_error_t ag_drv_rnr_quad_tcp_flags_set(uint8_t quad_idx, uint8_t tcp_flags_filt);
bdmf_error_t ag_drv_rnr_quad_tcp_flags_get(uint8_t quad_idx, uint8_t *tcp_flags_filt);
bdmf_error_t ag_drv_rnr_quad_profile_us_set(uint8_t quad_idx, uint8_t profile_us);
bdmf_error_t ag_drv_rnr_quad_profile_us_get(uint8_t quad_idx, uint8_t *profile_us);
bdmf_error_t ag_drv_rnr_quad_parser_snap_conf_set(uint8_t quad_idx, const rnr_quad_parser_snap_conf *parser_snap_conf);
bdmf_error_t ag_drv_rnr_quad_parser_snap_conf_get(uint8_t quad_idx, rnr_quad_parser_snap_conf *parser_snap_conf);
bdmf_error_t ag_drv_rnr_quad_parser_ipv6_filter_set(uint8_t quad_idx, const rnr_quad_parser_ipv6_filter *parser_ipv6_filter);
bdmf_error_t ag_drv_rnr_quad_parser_ipv6_filter_get(uint8_t quad_idx, rnr_quad_parser_ipv6_filter *parser_ipv6_filter);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_res_cntrl_set(uint8_t quad_idx, const rnr_quad_ubus_slv_res_cntrl *ubus_slv_res_cntrl);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_res_cntrl_get(uint8_t quad_idx, rnr_quad_ubus_slv_res_cntrl *ubus_slv_res_cntrl);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_vpb_base_set(uint8_t quad_idx, uint32_t base);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_vpb_base_get(uint8_t quad_idx, uint32_t *base);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_vpb_mask_set(uint8_t quad_idx, uint32_t mask);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_vpb_mask_get(uint8_t quad_idx, uint32_t *mask);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_apb_base_set(uint8_t quad_idx, uint32_t base);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_apb_base_get(uint8_t quad_idx, uint32_t *base);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_apb_mask_set(uint8_t quad_idx, uint32_t mask);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_apb_mask_get(uint8_t quad_idx, uint32_t *mask);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_dqm_base_set(uint8_t quad_idx, uint32_t base);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_dqm_base_get(uint8_t quad_idx, uint32_t *base);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_dqm_mask_set(uint8_t quad_idx, uint32_t mask);
bdmf_error_t ag_drv_rnr_quad_ubus_slv_dqm_mask_get(uint8_t quad_idx, uint32_t *mask);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_eng_set(uint8_t quad_idx, uint32_t cfg);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_eng_get(uint8_t quad_idx, uint32_t *cfg);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(uint8_t quad_idx, uint16_t ppp_code_0, uint16_t ppp_code_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_get(uint8_t quad_idx, uint16_t *ppp_code_0, uint16_t *ppp_code_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_set(uint8_t quad_idx, uint16_t ethtype_qtag_0, uint16_t ethtype_qtag_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_qtag_ethtype_get(uint8_t quad_idx, uint16_t *ethtype_qtag_0, uint16_t *ethtype_qtag_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_set(uint8_t quad_idx, uint16_t ethype_0, uint16_t ethype_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_0_1_get(uint8_t quad_idx, uint16_t *ethype_0, uint16_t *ethype_1);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_set(uint8_t quad_idx, uint16_t ethype_2, uint16_t ethype_3);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_2_3_get(uint8_t quad_idx, uint16_t *ethype_2, uint16_t *ethype_3);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_set(uint8_t quad_idx, const rnr_quad_parser_core_configuration_user_ethtype_config *parser_core_configuration_user_ethtype_config);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_user_ethtype_config_get(uint8_t quad_idx, rnr_quad_parser_core_configuration_user_ethtype_config *parser_core_configuration_user_ethtype_config);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_1_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_set(uint8_t quad_idx, const rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_da_filt_valid_cfg_2_get(uint8_t quad_idx, rnr_quad_da_filter_valid *da_filter_valid);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_set(uint8_t quad_idx, uint16_t gre_protocol);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_gre_protocol_cfg_get(uint8_t quad_idx, uint16_t *gre_protocol);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_set(uint8_t quad_idx, const rnr_quad_parser_core_configuration_prop_tag_cfg *parser_core_configuration_prop_tag_cfg);
bdmf_error_t ag_drv_rnr_quad_parser_core_configuration_prop_tag_cfg_get(uint8_t quad_idx, rnr_quad_parser_core_configuration_prop_tag_cfg *parser_core_configuration_prop_tag_cfg);
bdmf_error_t ag_drv_rnr_quad_general_config_dma_arb_cfg_set(uint8_t quad_idx, const rnr_quad_general_config_dma_arb_cfg *general_config_dma_arb_cfg);
bdmf_error_t ag_drv_rnr_quad_general_config_dma_arb_cfg_get(uint8_t quad_idx, rnr_quad_general_config_dma_arb_cfg *general_config_dma_arb_cfg);
bdmf_error_t ag_drv_rnr_quad_general_config_psram0_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram0_base_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram1_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram1_base_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram2_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram2_base_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram3_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram3_base_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_base_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_base_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_base_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram0_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram0_mask_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram1_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram1_mask_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram2_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram2_mask_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram3_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_psram3_mask_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr0_mask_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_mask_set(uint8_t quad_idx, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_general_config_ddr1_mask_get(uint8_t quad_idx, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_general_config_profiling_config_set(uint8_t quad_idx, const rnr_quad_general_config_profiling_config *general_config_profiling_config);
bdmf_error_t ag_drv_rnr_quad_general_config_profiling_config_get(uint8_t quad_idx, rnr_quad_general_config_profiling_config *general_config_profiling_config);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_0_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_0_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_1_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_1_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_2_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_2_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_3_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_3_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_4_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_4_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_5_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_5_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_6_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_6_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_7_cfg_set(uint8_t quad_idx, uint16_t addr, uint8_t thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_7_cfg_get(uint8_t quad_idx, uint16_t *addr, uint8_t *thread);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_gen_cfg_set(uint8_t quad_idx, uint16_t handler_addr, uint16_t update_pc_value);
bdmf_error_t ag_drv_rnr_quad_general_config_bkpt_gen_cfg_get(uint8_t quad_idx, uint16_t *handler_addr, uint16_t *update_pc_value);
bdmf_error_t ag_drv_rnr_quad_general_config_powersave_config_set(uint8_t quad_idx, const rnr_quad_general_config_powersave_config *general_config_powersave_config);
bdmf_error_t ag_drv_rnr_quad_general_config_powersave_config_get(uint8_t quad_idx, rnr_quad_general_config_powersave_config *general_config_powersave_config);
bdmf_error_t ag_drv_rnr_quad_general_config_powersave_status_get(uint8_t quad_idx, bdmf_boolean *core_0_status, bdmf_boolean *core_1_status, bdmf_boolean *core_2_status, bdmf_boolean *core_3_status);
bdmf_error_t ag_drv_rnr_quad_debug_fifo_config_set(uint8_t quad_idx, const rnr_quad_debug_fifo_config *debug_fifo_config);
bdmf_error_t ag_drv_rnr_quad_debug_fifo_config_get(uint8_t quad_idx, rnr_quad_debug_fifo_config *debug_fifo_config);
bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_psram_hdr_fifo_status *debug_psram_hdr_fifo_status);
bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_psram_data_fifo_status *debug_psram_data_fifo_status);
bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_ddr_hdr_fifo_status *debug_ddr_hdr_fifo_status);
bdmf_error_t ag_drv_rnr_quad_debug_ddr_data_fifo_status_get(uint8_t quad_idx, rnr_quad_debug_ddr_data_fifo_status *debug_ddr_data_fifo_status);
bdmf_error_t ag_drv_rnr_quad_debug_ddr_data_fifo_status2_get(uint8_t quad_idx, uint8_t *read_addr, uint16_t *used_words);
bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_data1_get(uint8_t quad_idx, uint32_t *data);
bdmf_error_t ag_drv_rnr_quad_debug_psram_hdr_fifo_data2_get(uint8_t quad_idx, uint32_t *data);
bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_data1_get(uint8_t quad_idx, uint32_t *data);
bdmf_error_t ag_drv_rnr_quad_debug_psram_data_fifo_data2_get(uint8_t quad_idx, uint32_t *data);
bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_data1_get(uint8_t quad_idx, uint32_t *data);
bdmf_error_t ag_drv_rnr_quad_debug_ddr_hdr_fifo_data2_get(uint8_t quad_idx, uint32_t *data);
bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ext_flowctrl_config_token_val_get(uint8_t quad_idx, uint32_t index, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_psram_ubus_decode_get(uint8_t quad_idx, uint32_t index, uint32_t *val);
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_set(uint8_t quad_idx, uint32_t index, uint32_t val);
bdmf_error_t ag_drv_rnr_quad_ubus_decode_cfg_ddr_ubus_decode_get(uint8_t quad_idx, uint32_t index, uint32_t *val);

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
    cli_rnr_quad_parser_snap_conf,
    cli_rnr_quad_parser_ipv6_filter,
    cli_rnr_quad_ubus_slv_res_cntrl,
    cli_rnr_quad_ubus_slv_vpb_base,
    cli_rnr_quad_ubus_slv_vpb_mask,
    cli_rnr_quad_ubus_slv_apb_base,
    cli_rnr_quad_ubus_slv_apb_mask,
    cli_rnr_quad_ubus_slv_dqm_base,
    cli_rnr_quad_ubus_slv_dqm_mask,
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
    cli_rnr_quad_ubus_decode_cfg_psram_ubus_decode,
    cli_rnr_quad_ubus_decode_cfg_ddr_ubus_decode,
};

int bcm_rnr_quad_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_rnr_quad_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

