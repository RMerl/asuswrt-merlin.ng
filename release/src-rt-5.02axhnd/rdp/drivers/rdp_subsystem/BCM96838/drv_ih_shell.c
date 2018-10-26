/*
* <:copyright-BRCM:2012:proprietary:standard
* 
*    Copyright (c) 2012 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :> 
*/

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of UT shell commands for the Lilac   */
/* IH driver.                                                                 */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#define BDMF_SESSION_DISABLE_FORMAT_CHECK
#include "rdp_drv_ih.h"
#include "bdmf_shell.h"
#include "rdpa_api.h"

#ifdef USE_BDMF_SHELL
/*****************************************************************************/
/*                                                                           */
/* Automatically generated unit test code - 3/3/2010   11:09:36              */
/*                                                                           */
/*****************************************************************************/

char * f_drv_ih_error_code_to_string ( DRV_IH_ERROR xi_error_code ) ;

static void p_dump_class_configuration ( bdmf_session_handle session, const DRV_IH_CLASS_CONFIG * xi_class_config ) ;
static void p_dump_classifier_configuration ( bdmf_session_handle session, const DRV_IH_CLASSIFIER_CONFIG * xi_classifier_config ) ;



DRV_IH_ERROR fi_get_lut_all_parameters ( uint8_t xi_table_index ,
                                                      DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * const xo_lookup_table_60_bit_key_config ,
                                                      bdmf_boolean * const xo_five_tupple_enable ) ;

static struct bdmfmon_enum_val rnr_max_buf_enum_table[] = {
    {"16", DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_16},
    {"24", DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_24},
    {"32", DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_32},
    {"48", DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_48},
    {"64", DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_64},
    {NULL, 0},
};

static struct bdmfmon_enum_val parse_layer_depth_enum_table[] = {
    {"VLAN", DRV_IH_PARSING_LAYER_DEPTH_VLAN},
    {"L2", DRV_IH_PARSING_LAYER_DEPTH_LAYER2},
    {"L3", DRV_IH_PARSING_LAYER_DEPTH_LAYER3},
    {"L4", DRV_IH_PARSING_LAYER_DEPTH_LAYER4},
    {NULL, 0},
};

static struct bdmfmon_enum_val prop_tag_size_enum_table[] = {
    {"0", DRV_IH_PROPRIETARY_TAG_SIZE_0},
    {"4", DRV_IH_PROPRIETARY_TAG_SIZE_4},
    {"6", DRV_IH_PROPRIETARY_TAG_SIZE_6},
    {"8", DRV_IH_PROPRIETARY_TAG_SIZE_8},
    {NULL, 0},
};

static struct bdmfmon_enum_val lookup_table_size_enum_table[] = {
    {"32", DRV_IH_LOOKUP_TABLE_SIZE_32_ENTRIES},
    {"64", DRV_IH_LOOKUP_TABLE_SIZE_64_ENTRIES},
    {"128", DRV_IH_LOOKUP_TABLE_SIZE_128_ENTRIES},
    {"256", DRV_IH_LOOKUP_TABLE_SIZE_256_ENTRIES},
    {"512", DRV_IH_LOOKUP_TABLE_SIZE_512_ENTRIES},
    {"1024", DRV_IH_LOOKUP_TABLE_SIZE_1024_ENTRIES},
    {"2048", DRV_IH_LOOKUP_TABLE_SIZE_2048_ENTRIES},
    {"4096", DRV_IH_LOOKUP_TABLE_SIZE_4096_ENTRIES},
    {NULL, 0},
};

static struct bdmfmon_enum_val search_depth_enum_table[] = {
    {"1", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_1_STEP},
    {"2", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_2_STEPS},
    {"4", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_4_STEPS},
    {"8", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_8_STEPS},
    {"16", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_16_STEPS},
    {"32", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_32_STEPS},
    {"64", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_64_STEPS},
    {"128", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_128_STEPS},
    {"256", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_256_STEPS},
    {"512", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_512_STEPS},
    {"1024", DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_1024_STEPS},
    {NULL, 0},
};

static struct bdmfmon_enum_val hash_type_enum_table[] = {
    {"incremental_keys", DRV_IH_LOOKUP_TABLE_HASH_TYPE_HASH_FOR_INCREMENTAL_KEYS},
    {"crc16", DRV_IH_LOOKUP_TABLE_HASH_TYPE_CRC16},
    {NULL, 0},
};

static struct bdmfmon_enum_val ctx_entry_size_enum_table[] = {
    {"1", DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE_1_BYTE},
    {"2", DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE_2_BYTES},
    {"4", DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE_4_BYTES},
    {"internal", DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE_INTERNAL_ENTRY},
    {NULL, 0},
};

static struct bdmfmon_enum_val lookup_key_ext_enum_table[] = {
    {"disable", DRV_IH_LOOKUP_KEY_EXTENSION_DISABLE},
    {"src_port", DRV_IH_LOOKUP_KEY_EXTENSION_SOURCE_PORT},
    {"gem_flow", DRV_IH_LOOKUP_KEY_EXTENSION_GEM_FLOW_ID},
    {"wan", DRV_IH_LOOKUP_KEY_EXTENSION_WAN},
    {NULL, 0},
};

static struct bdmfmon_enum_val class_search_enum_table[] = {
    {"tbl0", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_0},
    {"tbl1", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_1},
    {"tbl2", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_2},
    {"tbl3", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_3},
    {"tbl4", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_4},
    {"tbl5", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_5},
    {"tbl6", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_6},
    {"tbl7", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_7},
    {"tbl8", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_8},
    {"tbl9", DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_9},
    {"disable", DRV_IH_CLASS_SEARCH_DISABLED},
    {NULL, 0},
};

static struct bdmfmon_enum_val op_based_on_class_search_enum_table[] = {
    {"search1", DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1},
    {"search3", DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH3},
    {"disable", DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_OPERATION_DISABLED},
    {NULL, 0},
};

static struct bdmfmon_enum_val target_memory_enum_table[] = {
    {"ddr", DRV_IH_TARGET_MEMORY_DDR},
    {"sram", DRV_IH_TARGET_MEMORY_SRAM},
    {NULL, 0},
};

static struct bdmfmon_enum_val ingress_qos_enum_table[] = {
    {"low", DRV_IH_INGRESS_QOS_LOW},
    {"high", DRV_IH_INGRESS_QOS_HIGH},
    {"exclusive", DRV_IH_INGRESS_QOS_EXCLUSIVE},
    {NULL, 0},
};

static struct bdmfmon_enum_val runner_cluster_enum_table[] = {
    {"cluster_a", DRV_IH_RUNNER_CLUSTER_A},
    {"cluster_b", DRV_IH_RUNNER_CLUSTER_B},
    {NULL, 0},
};

static struct bdmfmon_enum_val l2_protocol_enum_table[] = {
    {"unknown",     DRV_IH_L2_PROTOCOL_UNKNOWN},
    {"pppoed",      DRV_IH_L2_PROTOCOL_PPPOE_DISCOVERY},
    {"pppoes",      DRV_IH_L2_PROTOCOL_PPPOE_SESSION},
    {"ipv4",        DRV_IH_L2_PROTOCOL_IPV4OE},
    {"ipv6",        DRV_IH_L2_PROTOCOL_IPV6OE},
    {"udef0",       DRV_IH_L2_PROTOCOL_USER_DEFINED_0},
    {"udef1",       DRV_IH_L2_PROTOCOL_USER_DEFINED_1},
    {"udef2",       DRV_IH_L2_PROTOCOL_USER_DEFINED_2},
    {"udef3",       DRV_IH_L2_PROTOCOL_USER_DEFINED_3},
    {"arp",         DRV_IH_L2_PROTOCOL_ARP},
    {"1588",        DRV_IH_L2_PROTOCOL_TYPE1588},
    {"8021x",       DRV_IH_L2_PROTOCOL_TYPE8021X},
    {"8011agcfm",   DRV_IH_L2_PROTOCOL_TYPE8011AGCFM},
    {NULL, 0},
};

static struct bdmfmon_enum_val l3_protocol_enum_table[] = {
    {"none",        DRV_IH_L3_PROTOCOL_NONE},
    {"ipv4",        DRV_IH_L3_PROTOCOL_IPV4},
    {"ipv6",        DRV_IH_L3_PROTOCOL_IPV6},
    {NULL, 0},
};

static struct bdmfmon_enum_val l4_protocol_enum_table[] = {
    {"none",        DRV_IH_L4_PROTOCOL_NONE},
    {"tcp",         DRV_IH_L4_PROTOCOL_TCP},
    {"udp",         DRV_IH_L4_PROTOCOL_UDP},
    {"igmp",        DRV_IH_L4_PROTOCOL_IGMP},
    {"icmp",        DRV_IH_L4_PROTOCOL_ICMP},
    {"icmpv6",      DRV_IH_L4_PROTOCOL_ICMPV6},
    {"gre",         DRV_IH_L4_PROTOCOL_GRE},
    {"ipv6",        DRV_IH_L4_PROTOCOL_IPV6},
    {"ah",          DRV_IH_L4_PROTOCOL_AH},
    {"not_parsed",  DRV_IH_L4_PROTOCOL_NOT_PARSED},
    {"udef0",       DRV_IH_L4_PROTOCOL_USER_DEFINED_0},
    {"udef1",       DRV_IH_L4_PROTOCOL_USER_DEFINED_1},
    {"udef2",       DRV_IH_L4_PROTOCOL_USER_DEFINED_2},
    {"udef3",       DRV_IH_L4_PROTOCOL_USER_DEFINED_3},
    {NULL, 0},
};

static struct bdmfmon_enum_val logical_port_enum_table[] = {
    {"eth0",        DRV_IH_LOGICAL_PORT_ETH0},
    {"eth1",        DRV_IH_LOGICAL_PORT_ETH1},
    {"eth2",        DRV_IH_LOGICAL_PORT_ETH2},
    {"eth3",        DRV_IH_LOGICAL_PORT_ETH3},
    {"eth4",        DRV_IH_LOGICAL_PORT_ETH4},
    {"gpon",        DRV_IH_LOGICAL_PORT_GPON},
    {"rnr_a",       DRV_IH_LOGICAL_PORT_RUNNER_A},
    {"rnr_b",       DRV_IH_LOGICAL_PORT_RUNNER_B},
    {"pcie0",       DRV_IH_LOGICAL_PORT_PCIE0},
    {"pcie1",       DRV_IH_LOGICAL_PORT_PCIE1},
    {NULL, 0},
};

static struct bdmfmon_enum_val target_matrix_src_port_enum_table[] = {
    {"eth0",        DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0},
    {"eth1",        DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH1},
    {"eth2",        DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH2},
    {"eth3",        DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH3},
    {"eth4",        DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH4},
    {"gpon",        DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON},
    {"pcie0",       DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0},
    {"pcie1",       DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE1},
    {NULL, 0},
};

static struct bdmfmon_enum_val target_matrix_dst_port_enum_table[] = {
    {"eth0",        DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0},
    {"eth1",        DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1},
    {"eth2",        DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2},
    {"eth3",        DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3},
    {"eth4",        DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4},
    {"gpon",        DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON},
    {"pcie0",       DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0},
    {"pcie1",       DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1},
    {"cpu",         DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU},
    {"mcast",       DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST},
    {"ddr",         DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR},
    {"sram",        DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM},
    {"spare",       DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE},
    {NULL, 0},
};

static struct bdmfmon_enum_val qtag_nesting_enum_table[] = {
    {"udef0",       DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_0},
    {"udef1",       DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_1},
    {"8100",        DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_8100},
    {"88A8",        DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_88A8},
    {"9100",        DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9100},
    {"9200",        DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9200},
    {NULL, 0},
};

static struct bdmfmon_enum_val ip_filter_enum_table[] = {
    {"src_ip",      DRV_IH_IP_FILTER_SELECTION_SOURCE_IP},
    {"dst_ip",      DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP},
    {NULL, 0},
};

static int p_bl_drv_ih_set_general_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_GENERAL_CONFIG ih_general_config ; /* input */
	DRV_IH_ERROR error ;
	uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	ih_general_config.runner_a_ih_response_address = parm[parameter_index++].value.unumber;
    ih_general_config.runner_b_ih_response_address = parm[parameter_index++].value.unumber;
    ih_general_config.runner_a_ih_congestion_report_address = parm[parameter_index++].value.unumber;
    ih_general_config.runner_b_ih_congestion_report_address = parm[parameter_index++].value.unumber;
    ih_general_config.runner_a_ih_congestion_report_enable = parm[parameter_index++].value.unumber;
    ih_general_config.runner_b_ih_congestion_report_enable = parm[parameter_index++].value.unumber;
    ih_general_config.lut_searches_enable_in_direct_mode = parm[parameter_index++].value.unumber;
    ih_general_config.sn_stamping_enable_in_direct_mode = parm[parameter_index++].value.unumber;
    ih_general_config.header_length_minimum = parm[parameter_index++].value.unumber;
    ih_general_config.congestion_discard_disable = parm[parameter_index++].value.unumber;
    ih_general_config.cam_search_enable_upon_invalid_lut_entry = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_general_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_general_configuration ( & ih_general_config ) ;
	/* -------------------------------------------------------------------- */

	/* Translate and print the error code */
	bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

	return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_ih_get_general_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_GENERAL_CONFIG ih_general_config ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_general_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_general_configuration ( & ih_general_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "runner_a_ih_response_address: 0x%lX\n\r" , ih_general_config.runner_a_ih_response_address ) ;
        bdmf_session_print(session, "runner_b_ih_response_address: 0x%lX\n\r" , ih_general_config.runner_b_ih_response_address ) ;
        bdmf_session_print(session, "runner_a_ih_congestion_report_address: 0x%lX\n\r" , ih_general_config.runner_a_ih_congestion_report_address ) ;
        bdmf_session_print(session, "runner_b_ih_congestion_report_address: 0x%lX\n\r" , ih_general_config.runner_b_ih_congestion_report_address ) ;
        bdmf_session_print(session, "runner_a_ih_congestion_report_enable: %lu\n\r" , ih_general_config.runner_a_ih_congestion_report_enable ) ;
        bdmf_session_print(session, "runner_b_ih_congestion_report_enable: %lu\n\r" , ih_general_config.runner_b_ih_congestion_report_enable ) ;
        bdmf_session_print(session, "lut_searches_enable_in_direct_mode: %lu\n\r" , ih_general_config.lut_searches_enable_in_direct_mode ) ;
        bdmf_session_print(session, "sn_stamping_enable_in_direct_mode: %lu\n\r" , ih_general_config.sn_stamping_enable_in_direct_mode ) ;
        bdmf_session_print(session, "header_length_minimum: %lu\n\r" , ih_general_config.header_length_minimum ) ;
        bdmf_session_print(session, "congestion_discard_disable: %lu\n\r" , ih_general_config.congestion_discard_disable ) ;
        bdmf_session_print(session, "cam_search_enable_upon_invalid_lut_entry: %lu\n\r" , ih_general_config.cam_search_enable_upon_invalid_lut_entry ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

	/* Translate and print the error code */
	bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

	return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_ih_set_packet_header_offsets_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_PACKET_HEADER_OFFSETS packet_header_offsets ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	packet_header_offsets.eth0_packet_header_offset = parm[parameter_index++].value.unumber;
	packet_header_offsets.eth1_packet_header_offset = parm[parameter_index++].value.unumber;
	packet_header_offsets.eth2_packet_header_offset = parm[parameter_index++].value.unumber;
	packet_header_offsets.eth3_packet_header_offset = parm[parameter_index++].value.unumber;
	packet_header_offsets.eth4_packet_header_offset = parm[parameter_index++].value.unumber;
	packet_header_offsets.gpon_packet_header_offset = parm[parameter_index++].value.unumber;
	packet_header_offsets.runner_a_packet_header_offset = parm[parameter_index++].value.unumber;
	packet_header_offsets.runner_b_packet_header_offset = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_packet_header_offsets'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_packet_header_offsets ( & packet_header_offsets ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_packet_header_offsets_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_PACKET_HEADER_OFFSETS packet_header_offsets ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_packet_header_offsets'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_packet_header_offsets ( & packet_header_offsets ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "eth0_packet_header_offset: %lu\n\r" , packet_header_offsets.eth0_packet_header_offset ) ;
        bdmf_session_print(session, "eth1_packet_header_offset: %lu\n\r" , packet_header_offsets.eth1_packet_header_offset ) ;
        bdmf_session_print(session, "eth2_packet_header_offset: %lu\n\r" , packet_header_offsets.eth2_packet_header_offset ) ;
        bdmf_session_print(session, "eth3_packet_header_offset: %lu\n\r" , packet_header_offsets.eth3_packet_header_offset ) ;
        bdmf_session_print(session, "eth4_packet_header_offset: %lu\n\r" , packet_header_offsets.eth4_packet_header_offset ) ;
        bdmf_session_print(session, "gpon_packet_header_offset: %lu\n\r" , packet_header_offsets.gpon_packet_header_offset ) ;
        bdmf_session_print(session, "runner_a_packet_header_offset: %lu\n\r" , packet_header_offsets.runner_a_packet_header_offset ) ;
        bdmf_session_print(session, "runner_b_packet_header_offset: %lu\n\r" , packet_header_offsets.runner_b_packet_header_offset ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_runner_buffers_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_RUNNER_BUFFERS_CONFIG runner_buffers_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	runner_buffers_config.runner_a_ih_managed_rb_base_address = parm[parameter_index++].value.unumber;
	runner_buffers_config.runner_b_ih_managed_rb_base_address = parm[parameter_index++].value.unumber;
	runner_buffers_config.runner_a_runner_managed_rb_base_address = parm[parameter_index++].value.unumber;
	runner_buffers_config.runner_b_runner_managed_rb_base_address = parm[parameter_index++].value.unumber;
	runner_buffers_config.runner_a_maximal_number_of_buffers = parm[parameter_index++].value.unumber;
	runner_buffers_config.runner_b_maximal_number_of_buffers = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_runner_buffers_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_runner_buffers_configuration ( & runner_buffers_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_runner_buffers_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_RUNNER_BUFFERS_CONFIG runner_buffers_config ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_runner_buffers_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_runner_buffers_configuration ( & runner_buffers_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "runner_a_ih_managed_rb_base_address: 0x%lX\n\r" , runner_buffers_config.runner_a_ih_managed_rb_base_address ) ;
        bdmf_session_print(session, "runner_b_ih_managed_rb_base_address: 0x%lX\n\r" , runner_buffers_config.runner_b_ih_managed_rb_base_address ) ;
        bdmf_session_print(session, "runner_a_runner_managed_rb_base_address: 0x%lX\n\r" , runner_buffers_config.runner_a_runner_managed_rb_base_address ) ;
        bdmf_session_print(session, "runner_b_runner_managed_rb_base_address: 0x%lX\n\r" , runner_buffers_config.runner_b_runner_managed_rb_base_address ) ;
        bdmf_session_print(session, "runner_a_maximal_number_of_buffers: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(rnr_max_buf_enum_table, runner_buffers_config.runner_a_maximal_number_of_buffers),
            runner_buffers_config.runner_a_maximal_number_of_buffers ) ;
        bdmf_session_print(session, "runner_b_maximal_number_of_buffers: %s (%lu)\n\r",
            bdmfmon_enum_stringval(rnr_max_buf_enum_table, runner_buffers_config.runner_b_maximal_number_of_buffers),
            runner_buffers_config.runner_b_maximal_number_of_buffers ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_runners_load_thresholds_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_RUNNERS_LOAD_THRESHOLDS runners_load_thresholds ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	runners_load_thresholds.runner_a_high_congestion_threshold = parm[parameter_index++].value.unumber;
	runners_load_thresholds.runner_b_high_congestion_threshold = parm[parameter_index++].value.unumber;
	runners_load_thresholds.runner_a_exclusive_congestion_threshold = parm[parameter_index++].value.unumber;
	runners_load_thresholds.runner_b_exclusive_congestion_threshold = parm[parameter_index++].value.unumber;
	runners_load_thresholds.runner_a_load_balancing_threshold = parm[parameter_index++].value.unumber;
	runners_load_thresholds.runner_b_load_balancing_threshold = parm[parameter_index++].value.unumber;
	runners_load_thresholds.runner_a_load_balancing_hysteresis = parm[parameter_index++].value.unumber;
	runners_load_thresholds.runner_b_load_balancing_hysteresis = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_runners_load_thresholds'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_runners_load_thresholds ( & runners_load_thresholds ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_runners_load_thresholds_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_RUNNERS_LOAD_THRESHOLDS runners_load_thresholds ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_runners_load_thresholds'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_runners_load_thresholds ( & runners_load_thresholds ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "runner_a_high_congestion_threshold: %lu\n\r" , runners_load_thresholds.runner_a_high_congestion_threshold ) ;
        bdmf_session_print(session, "runner_b_high_congestion_threshold: %lu\n\r" , runners_load_thresholds.runner_b_high_congestion_threshold ) ;
        bdmf_session_print(session, "runner_a_exclusive_congestion_threshold: %lu\n\r" , runners_load_thresholds.runner_a_exclusive_congestion_threshold ) ;
        bdmf_session_print(session, "runner_b_exclusive_congestion_threshold: %lu\n\r" , runners_load_thresholds.runner_b_exclusive_congestion_threshold ) ;
        bdmf_session_print(session, "runner_a_load_balancing_threshold: %lu\n\r" , runners_load_thresholds.runner_a_load_balancing_threshold ) ;
        bdmf_session_print(session, "runner_b_load_balancing_threshold: %lu\n\r" , runners_load_thresholds.runner_b_load_balancing_threshold ) ;
        bdmf_session_print(session, "runner_a_load_balancing_hysteresis: %lu\n\r" , runners_load_thresholds.runner_a_load_balancing_hysteresis ) ;
        bdmf_session_print(session, "runner_b_load_balancing_hysteresis: %lu\n\r" , runners_load_thresholds.runner_b_load_balancing_hysteresis ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_route_addresses_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_ROUTE_ADDRESSES route_addresses ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	route_addresses.eth0_route_address = parm[parameter_index++].value.unumber;
	route_addresses.eth1_route_address = parm[parameter_index++].value.unumber;
	route_addresses.eth2_route_address = parm[parameter_index++].value.unumber;
	route_addresses.eth3_route_address = parm[parameter_index++].value.unumber;
	route_addresses.eth4_route_address = parm[parameter_index++].value.unumber;
	route_addresses.gpon_route_address = parm[parameter_index++].value.unumber;
	route_addresses.runner_a_route_address = parm[parameter_index++].value.unumber;
	route_addresses.runner_b_route_address = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_route_addresses'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_route_addresses ( & route_addresses ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_ih_get_route_addresses_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_ROUTE_ADDRESSES route_addresses ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_route_addresses'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_route_addresses ( & route_addresses ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "eth0_route_address: 0x%lX\n\r" , route_addresses.eth0_route_address ) ;
        bdmf_session_print(session, "eth1_route_address: 0x%lX\n\r" , route_addresses.eth1_route_address ) ;
        bdmf_session_print(session, "eth2_route_address: 0x%lX\n\r" , route_addresses.eth2_route_address ) ;
        bdmf_session_print(session, "eth3_route_address: 0x%lX\n\r" , route_addresses.eth3_route_address ) ;
        bdmf_session_print(session, "eth4_route_address: 0x%lX\n\r" , route_addresses.eth4_route_address ) ;
        bdmf_session_print(session, "gpon_route_address: 0x%lX\n\r" , route_addresses.gpon_route_address ) ;
        bdmf_session_print(session, "runner_a_route_address: 0x%lX\n\r" , route_addresses.runner_a_route_address ) ;
        bdmf_session_print(session, "runner_b_route_address: 0x%lX\n\r" , route_addresses.runner_b_route_address ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_logical_ports_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_LOGICAL_PORTS_CONFIG logical_ports_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	logical_ports_config.eth0_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
	logical_ports_config.eth0_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.eth1_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.eth1_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.eth2_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.eth2_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.eth3_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.eth3_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.eth4_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.eth4_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.gpon_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.gpon_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.runner_a_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.runner_a_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.runner_b_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.runner_b_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.pcie0_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.pcie0_config.proprietary_tag_size = parm[parameter_index++].value.unumber;
    logical_ports_config.pcie1_config.parsing_layer_depth = parm[parameter_index++].value.unumber;
    logical_ports_config.pcie1_config.proprietary_tag_size = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_logical_ports_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_ih_set_logical_ports_configuration ( & logical_ports_config ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_logical_ports_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_LOGICAL_PORTS_CONFIG logical_ports_config ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_logical_ports_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_logical_ports_configuration ( & logical_ports_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {

        bdmf_session_print(session, "eth0_config.parsing_layer_depth: %s (%lu)\n\r",
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.eth0_config.parsing_layer_depth),
            logical_ports_config.eth0_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "eth0_config.proprietary_tag_size: %s (%lu)\n\r",
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.eth0_config.proprietary_tag_size),
            logical_ports_config.eth0_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "eth1_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.eth1_config.parsing_layer_depth ),
            logical_ports_config.eth1_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "eth1_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.eth1_config.proprietary_tag_size ),
            logical_ports_config.eth1_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "eth2_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.eth2_config.parsing_layer_depth ),
            logical_ports_config.eth2_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "eth2_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.eth2_config.proprietary_tag_size ),
            logical_ports_config.eth2_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "eth3_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.eth3_config.parsing_layer_depth ),
            logical_ports_config.eth3_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "eth3_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.eth3_config.proprietary_tag_size ),
            logical_ports_config.eth3_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "eth4_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.eth4_config.parsing_layer_depth ),
            logical_ports_config.eth4_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "eth4_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.eth4_config.proprietary_tag_size ),
            logical_ports_config.eth4_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "gpon_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.gpon_config.parsing_layer_depth ),
            logical_ports_config.gpon_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "gpon_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.gpon_config.proprietary_tag_size ),
            logical_ports_config.gpon_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "runner_a_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.runner_a_config.parsing_layer_depth ),
            logical_ports_config.runner_a_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "runner_a_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.runner_a_config.proprietary_tag_size ),
            logical_ports_config.runner_a_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "runner_b_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.runner_b_config.parsing_layer_depth ),
            logical_ports_config.runner_b_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "runner_b_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.runner_b_config.proprietary_tag_size ),
            logical_ports_config.runner_b_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "pcie0_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.pcie0_config.parsing_layer_depth ),
            logical_ports_config.pcie0_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "pcie0_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.pcie0_config.proprietary_tag_size ),
            logical_ports_config.pcie0_config.proprietary_tag_size ) ;
        bdmf_session_print(session, "pcie1_config.parsing_layer_depth: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(parse_layer_depth_enum_table, logical_ports_config.pcie1_config.parsing_layer_depth ),
            logical_ports_config.pcie1_config.parsing_layer_depth ) ;
        bdmf_session_print(session, "pcie1_config.proprietary_tag_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(prop_tag_size_enum_table, logical_ports_config.pcie1_config.proprietary_tag_size ),
            logical_ports_config.pcie1_config.proprietary_tag_size ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_configure_lut_60_bit_key_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG lookup_table_60_bit_key_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	table_index = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.table_base_address_in_8_byte = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.table_size = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.maximal_search_depth = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.hash_type = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.sa_search_enable = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.aging_enable = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.cam_enable = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.cam_base_address_in_8_byte = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.context_table_entry_size = parm[parameter_index++].value.unumber;
	lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.key_extension = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.part_0_mask_low = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.part_0_mask_high = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.part_1_mask_low = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.part_1_mask_high = parm[parameter_index++].value.unumber;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_lut_60_bit_key'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_lut_60_bit_key ( table_index, & lookup_table_60_bit_key_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_ih_get_lut_60_bit_key_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG lookup_table_60_bit_key_config ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	table_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_lut_60_bit_key_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_lut_60_bit_key_configuration ( table_index, & lookup_table_60_bit_key_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "table_base_address_in_8_byte: 0x%lX\n\r" , lookup_table_60_bit_key_config.table_base_address_in_8_byte ) ;
        bdmf_session_print(session, "table_size: %s (%lu)\n\r" , bdmfmon_enum_stringval(lookup_table_size_enum_table, lookup_table_60_bit_key_config.table_size) , lookup_table_60_bit_key_config.table_size ) ;
        bdmf_session_print(session, "maximal_search_depth: %s (%lu)\n\r" , bdmfmon_enum_stringval(search_depth_enum_table, lookup_table_60_bit_key_config.maximal_search_depth ) , lookup_table_60_bit_key_config.maximal_search_depth ) ;
        bdmf_session_print(session, "hash_type: %s (%lu)\n\r" , bdmfmon_enum_stringval(hash_type_enum_table, lookup_table_60_bit_key_config.hash_type ) , lookup_table_60_bit_key_config.hash_type ) ;
        bdmf_session_print(session, "sa_search_enable: %lu\n\r" , lookup_table_60_bit_key_config.sa_search_enable ) ;
        bdmf_session_print(session, "aging_enable: %lu\n\r" , lookup_table_60_bit_key_config.aging_enable ) ;
        bdmf_session_print(session, "cam_enable: %lu\n\r" , lookup_table_60_bit_key_config.cam_enable ) ;
        bdmf_session_print(session, "cam_base_address_in_8_byte: 0x%lX\n\r" , lookup_table_60_bit_key_config.cam_base_address_in_8_byte ) ;
        bdmf_session_print(session, "context_table_base_address_in_8_byte: 0x%lX\n\r" , lookup_table_60_bit_key_config.context_table_base_address_in_8_byte ) ;
        bdmf_session_print(session, "context_table_entry_size: %s (%lu)\n\r" , bdmfmon_enum_stringval(ctx_entry_size_enum_table, lookup_table_60_bit_key_config.context_table_entry_size ) , lookup_table_60_bit_key_config.context_table_entry_size ) ;
        bdmf_session_print(session, "cam_context_base_address_in_8_byte: 0x%lX\n\r" , lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte ) ;
        bdmf_session_print(session, "part_0_start_offset_in_4_byte: %lu\n\r" , lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte ) ;
        bdmf_session_print(session, "part_0_shift_offset_in_4_bit: %lu\n\r" , lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit ) ;
        bdmf_session_print(session, "part_1_start_offset_in_4_byte: %lu\n\r" , lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte ) ;
        bdmf_session_print(session, "part_1_shift_offset_in_4_bit: %lu\n\r" , lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit ) ;
        bdmf_session_print(session, "key_extension: %s (%lu)\n\r" , bdmfmon_enum_stringval(lookup_key_ext_enum_table, lookup_table_60_bit_key_config.key_extension ) , lookup_table_60_bit_key_config.key_extension ) ;
        bdmf_session_print(session, "part_0_mask_low: 0x%lX\n\r" , lookup_table_60_bit_key_config.part_0_mask_low ) ;
        bdmf_session_print(session, "part_0_mask_high: 0x%lX\n\r" , lookup_table_60_bit_key_config.part_0_mask_high ) ;
        bdmf_session_print(session, "part_1_mask_low: 0x%lX\n\r" , lookup_table_60_bit_key_config.part_1_mask_low ) ;
        bdmf_session_print(session, "part_1_mask_high: 0x%lX\n\r" , lookup_table_60_bit_key_config.part_1_mask_high ) ;
        bdmf_session_print(session, "global_mask_in_4_bit: 0x%lX\n\r" , lookup_table_60_bit_key_config.global_mask_in_4_bit ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_ih_configure_lut_120_bit_key_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t primary_table_index ; /* input */
	uint8_t secondary_table_index ; /* input */
	DRV_IH_LOOKUP_TABLE_120_BIT_KEY_CONFIG lookup_table_120_bit_key_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	primary_table_index = parm[parameter_index++].value.unumber;
	secondary_table_index = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.table_base_address_in_8_byte = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.table_size = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.maximal_search_depth = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.hash_type = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.aging_enable = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.cam_enable = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.cam_base_address_in_8_byte = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.context_table_base_address_in_8_byte = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.context_table_entry_size = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.cam_context_base_address_in_8_byte = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_part_0_start_offset_in_4_byte = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_part_0_shift_offset_in_4_bit = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_part_1_start_offset_in_4_byte = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_part_1_shift_offset_in_4_bit = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_extension = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_part_0_mask_low = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_part_0_mask_high = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_part_1_mask_low = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.primary_key_part_1_mask_high = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_part_0_start_offset_in_4_byte = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_part_0_shift_offset_in_4_bit = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_part_1_start_offset_in_4_byte = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_part_1_shift_offset_in_4_bit = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_extension = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_part_0_mask_low = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_part_0_mask_high = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_part_1_mask_low = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.secondary_key_part_1_mask_high = parm[parameter_index++].value.unumber;
    lookup_table_120_bit_key_config.global_mask_in_4_bit = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_lut_120_bit_key'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_lut_120_bit_key ( primary_table_index, secondary_table_index, & lookup_table_120_bit_key_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_lut_120_bit_key_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t primary_table_index ; /* input */
	uint8_t secondary_table_index ; /* input */
	DRV_IH_LOOKUP_TABLE_120_BIT_KEY_CONFIG lookup_table_120_bit_key_config ; /* output */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	primary_table_index = parm[parameter_index++].value.unumber;
	secondary_table_index = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_lut_120_bit_key_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_lut_120_bit_key_configuration ( primary_table_index, secondary_table_index, & lookup_table_120_bit_key_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "table_base_address_in_8_byte: 0x%lX\n\r" , lookup_table_120_bit_key_config.table_base_address_in_8_byte ) ;
        bdmf_session_print(session, "table_size: %s (%lu) - Notice that in HW a double value is configured!\n\r" ,
            bdmfmon_enum_stringval(lookup_table_size_enum_table, lookup_table_120_bit_key_config.table_size ) , lookup_table_120_bit_key_config.table_size ) ;
        bdmf_session_print(session, "maximal_search_depth: %s (%lu)- Notice that in HW a double value is configured!\n\r",
            bdmfmon_enum_stringval(search_depth_enum_table, lookup_table_120_bit_key_config.maximal_search_depth ) , lookup_table_120_bit_key_config.maximal_search_depth ) ;
        bdmf_session_print(session, "hash_type: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(hash_type_enum_table, lookup_table_120_bit_key_config.hash_type ) , lookup_table_120_bit_key_config.hash_type ) ;
        bdmf_session_print(session, "aging_enable: %lu\n\r" , lookup_table_120_bit_key_config.aging_enable ) ;
        bdmf_session_print(session, "cam_enable: %lu\n\r" , lookup_table_120_bit_key_config.cam_enable ) ;
        bdmf_session_print(session, "cam_base_address_in_8_byte: 0x%lX\n\r" , lookup_table_120_bit_key_config.cam_base_address_in_8_byte ) ;
        bdmf_session_print(session, "context_table_base_address_in_8_byte: 0x%lX\n\r" , lookup_table_120_bit_key_config.context_table_base_address_in_8_byte ) ;
        bdmf_session_print(session, "context_table_entry_size: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(ctx_entry_size_enum_table, lookup_table_120_bit_key_config.context_table_entry_size ) , lookup_table_120_bit_key_config.context_table_entry_size ) ;
        bdmf_session_print(session, "cam_context_base_address_in_8_byte: 0x%lX\n\r" , lookup_table_120_bit_key_config.cam_context_base_address_in_8_byte ) ;
        bdmf_session_print(session, "primary_key_part_0_start_offset_in_4_byte: %lu\n\r" , lookup_table_120_bit_key_config.primary_key_part_0_start_offset_in_4_byte ) ;
        bdmf_session_print(session, "primary_key_part_0_shift_offset_in_4_bit: %lu\n\r" , lookup_table_120_bit_key_config.primary_key_part_0_shift_offset_in_4_bit ) ;
        bdmf_session_print(session, "primary_key_part_1_start_offset_in_4_byte: %lu\n\r" , lookup_table_120_bit_key_config.primary_key_part_1_start_offset_in_4_byte ) ;
        bdmf_session_print(session, "primary_key_part_1_shift_offset_in_4_bit: %lu\n\r" , lookup_table_120_bit_key_config.primary_key_part_1_shift_offset_in_4_bit ) ;
        bdmf_session_print(session, "primary_key_extension: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(lookup_key_ext_enum_table, lookup_table_120_bit_key_config.primary_key_extension ) , lookup_table_120_bit_key_config.primary_key_extension ) ;
        bdmf_session_print(session, "primary_key_part_0_mask_low: 0x%lX\n\r" , lookup_table_120_bit_key_config.primary_key_part_0_mask_low ) ;
        bdmf_session_print(session, "primary_key_part_0_mask_high: 0x%lX\n\r" , lookup_table_120_bit_key_config.primary_key_part_0_mask_high ) ;
        bdmf_session_print(session, "primary_key_part_1_mask_low: 0x%lX\n\r" , lookup_table_120_bit_key_config.primary_key_part_1_mask_low ) ;
        bdmf_session_print(session, "primary_key_part_1_mask_high: 0x%lX\n\r" , lookup_table_120_bit_key_config.primary_key_part_1_mask_high ) ;
        bdmf_session_print(session, "secondary_key_part_0_start_offset_in_4_byte: %lu\n\r" , lookup_table_120_bit_key_config.secondary_key_part_0_start_offset_in_4_byte ) ;
        bdmf_session_print(session, "secondary_key_part_0_shift_offset_in_4_bit: %lu\n\r" , lookup_table_120_bit_key_config.secondary_key_part_0_shift_offset_in_4_bit ) ;
        bdmf_session_print(session, "secondary_key_part_1_start_offset_in_4_byte: %lu\n\r" , lookup_table_120_bit_key_config.secondary_key_part_1_start_offset_in_4_byte ) ;
        bdmf_session_print(session, "secondary_key_part_1_shift_offset_in_4_bit: %lu\n\r" , lookup_table_120_bit_key_config.secondary_key_part_1_shift_offset_in_4_bit ) ;
        bdmf_session_print(session, "secondary_key_extension: %s (%lu)\n\r" ,
            bdmfmon_enum_stringval(lookup_key_ext_enum_table, lookup_table_120_bit_key_config.secondary_key_extension ) , lookup_table_120_bit_key_config.secondary_key_extension ) ;
        bdmf_session_print(session, "secondary_key_part_0_mask_low: 0x%lX\n\r" , lookup_table_120_bit_key_config.secondary_key_part_0_mask_low ) ;
        bdmf_session_print(session, "secondary_key_part_0_mask_high: 0x%lX\n\r" , lookup_table_120_bit_key_config.secondary_key_part_0_mask_high ) ;
        bdmf_session_print(session, "secondary_key_part_1_mask_low: 0x%lX\n\r" , lookup_table_120_bit_key_config.secondary_key_part_1_mask_low ) ;
        bdmf_session_print(session, "secondary_key_part_1_mask_high: 0x%lX\n\r" , lookup_table_120_bit_key_config.secondary_key_part_1_mask_high ) ;
        bdmf_session_print(session, "global_mask_in_4_bit: 0x%lX\n\r" , lookup_table_120_bit_key_config.global_mask_in_4_bit ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_configure_class_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t class_index ; /* input */
	DRV_IH_CLASS_CONFIG class_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	class_index = parm[parameter_index++].value.unumber;
    class_config.class_search_1 = parm[parameter_index++].value.unumber;
    class_config.class_search_2 = parm[parameter_index++].value.unumber;
    class_config.class_search_3 = parm[parameter_index++].value.unumber;
    class_config.class_search_4 = parm[parameter_index++].value.unumber;
    class_config.destination_port_extraction = parm[parameter_index++].value.unumber;
    class_config.drop_on_miss = parm[parameter_index++].value.unumber;
    class_config.dscp_to_tci_table_index = parm[parameter_index++].value.unumber;
    class_config.direct_mode_default = parm[parameter_index++].value.unumber;
    class_config.direct_mode_override = parm[parameter_index++].value.unumber;
    class_config.target_memory_default = parm[parameter_index++].value.unumber;
    class_config.target_memory_override = parm[parameter_index++].value.unumber;
    class_config.ingress_qos_default = parm[parameter_index++].value.unumber;
    class_config.ingress_qos_override = parm[parameter_index++].value.unumber;
    class_config.target_runner_default = parm[parameter_index++].value.unumber;
    class_config.target_runner_override_in_direct_mode = parm[parameter_index++].value.unumber;
    class_config.target_runner_for_direct_mode = parm[parameter_index++].value.unumber;
    class_config.load_balancing_enable = parm[parameter_index++].value.unumber;
    class_config.preference_load_balancing_enable = parm[parameter_index++].value.unumber;


	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_class'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_class ( class_index, & class_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_ih_get_class_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t class_index ; /* input */
	DRV_IH_CLASS_CONFIG class_config ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	class_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_class_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_class_configuration ( class_index, & class_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        p_dump_class_configuration ( session, & class_config ) ;
        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_configure_classifier_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t classifier_index ; /* input */
	DRV_IH_CLASSIFIER_CONFIG classifier_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	classifier_index = parm[parameter_index++].value.unumber;
    classifier_config.l2_protocol = parm[parameter_index++].value.unumber;
    classifier_config.l3_protocol = parm[parameter_index++].value.unumber;
    classifier_config.l4_protocol = parm[parameter_index++].value.unumber;
    classifier_config.da_filter_any_hit = parm[parameter_index++].value.unumber;
    classifier_config.matched_da_filter = parm[parameter_index++].value.unumber;
    classifier_config.multicast_da_indication = parm[parameter_index++].value.unumber;
    classifier_config.broadcast_da_indication = parm[parameter_index++].value.unumber;
    classifier_config.vid_filter_any_hit = parm[parameter_index++].value.unumber;
    classifier_config.matched_vid_filter = parm[parameter_index++].value.unumber;
    classifier_config.ip_filter_any_hit = parm[parameter_index++].value.unumber;
    classifier_config.matched_ip_filter = parm[parameter_index++].value.unumber;
    classifier_config.wan_indication = parm[parameter_index++].value.unumber;
    classifier_config.five_tuple_valid = parm[parameter_index++].value.unumber;
    classifier_config.logical_source_port = parm[parameter_index++].value.unumber;
    classifier_config.error = parm[parameter_index++].value.unumber;
    classifier_config.mask = parm[parameter_index++].value.unumber;
    classifier_config.resulting_class = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_classifier'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_classifier ( classifier_index, & classifier_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_classifier_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t classifier_index ; /* input */
	DRV_IH_CLASSIFIER_CONFIG classifier_config ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	classifier_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_classifier_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_classifier_configuration ( classifier_index, & classifier_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        p_dump_classifier_configuration (session, &classifier_config ) ;
        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_remove_classifier_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t classifier_index ; /* input */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	classifier_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_remove_classifier'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_remove_classifier ( classifier_index ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_source_port_to_ingress_queue_mapping_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING source_port_to_ingress_queue_mapping ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
    source_port_to_ingress_queue_mapping.eth0_ingress_queue = parm[parameter_index++].value.unumber;
    source_port_to_ingress_queue_mapping.eth1_ingress_queue = parm[parameter_index++].value.unumber;
    source_port_to_ingress_queue_mapping.eth2_ingress_queue = parm[parameter_index++].value.unumber;
    source_port_to_ingress_queue_mapping.eth3_ingress_queue = parm[parameter_index++].value.unumber;
    source_port_to_ingress_queue_mapping.eth4_ingress_queue = parm[parameter_index++].value.unumber;
    source_port_to_ingress_queue_mapping.gpon_ingress_queue = parm[parameter_index++].value.unumber;
    source_port_to_ingress_queue_mapping.runner_a_ingress_queue = parm[parameter_index++].value.unumber;
    source_port_to_ingress_queue_mapping.runner_b_ingress_queue = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping ( & source_port_to_ingress_queue_mapping ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_source_port_to_ingress_queue_mapping_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING source_port_to_ingress_queue_mapping ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_source_port_to_ingress_queue_mapping'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_source_port_to_ingress_queue_mapping ( & source_port_to_ingress_queue_mapping ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "eth0_ingress_queue: %lu\n\r" , source_port_to_ingress_queue_mapping.eth0_ingress_queue ) ;
        bdmf_session_print(session, "eth1_ingress_queue: %lu\n\r" , source_port_to_ingress_queue_mapping.eth1_ingress_queue ) ;
        bdmf_session_print(session, "eth2_ingress_queue: %lu\n\r" , source_port_to_ingress_queue_mapping.eth2_ingress_queue ) ;
        bdmf_session_print(session, "eth3_ingress_queue: %lu\n\r" , source_port_to_ingress_queue_mapping.eth3_ingress_queue ) ;
        bdmf_session_print(session, "eth4_ingress_queue: %lu\n\r" , source_port_to_ingress_queue_mapping.eth4_ingress_queue ) ;
        bdmf_session_print(session, "gpon_ingress_queue: %lu\n\r" , source_port_to_ingress_queue_mapping.gpon_ingress_queue ) ;
        bdmf_session_print(session, "runner_a_ingress_queue: %lu\n\r" , source_port_to_ingress_queue_mapping.runner_a_ingress_queue ) ;
        bdmf_session_print(session, "runner_b_ingress_queue: %lu\n\r" , source_port_to_ingress_queue_mapping.runner_b_ingress_queue ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_configure_ingress_queue_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t ingress_queue_index ; /* input */
	DRV_IH_INGRESS_QUEUE_CONFIG ingress_queue_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	ingress_queue_index = parm[parameter_index++].value.unumber;
    ingress_queue_config.base_location = parm[parameter_index++].value.unumber;
    ingress_queue_config.size = parm[parameter_index++].value.unumber;
    ingress_queue_config.priority = parm[parameter_index++].value.unumber;
    ingress_queue_config.weight = parm[parameter_index++].value.unumber;
    ingress_queue_config.congestion_threshold = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_ingress_queue'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_ingress_queue ( ingress_queue_index, & ingress_queue_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_ingress_queue_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t ingress_queue_index ; /* input */
	DRV_IH_INGRESS_QUEUE_CONFIG ingress_queue_config ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	ingress_queue_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_ingress_queue_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_ingress_queue_configuration ( ingress_queue_index, & ingress_queue_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "base_location: %lu\n\r" , ingress_queue_config.base_location ) ;
        bdmf_session_print(session, "size: %lu (Note: in HW, the value 0 means 16)\n\r" , ingress_queue_config.size ) ;
        bdmf_session_print(session, "priority: %lu (in HW: %lu)\n\r" , ingress_queue_config.priority , 1 << ingress_queue_config.priority ) ;
        bdmf_session_print(session, "weight: %lu\n\r" , ingress_queue_config.weight ) ;
        bdmf_session_print(session, "congestion_threshold: %lu\n\r" , ingress_queue_config.congestion_threshold ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_target_matrix_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port ; /* input */
	DRV_IH_TARGET_MATRIX_PER_SP_CONFIG per_sp_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	source_port = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0 ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0 ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1 ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1 ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2 ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2 ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3 ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3 ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4 ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4 ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0 ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0 ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1 ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1 ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM ].direct_mode = parm[parameter_index++].value.unumber;

    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE ].target_memory = parm[parameter_index++].value.unumber;
    per_sp_config.entry [ DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE ].direct_mode = parm[parameter_index++].value.unumber;


	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_target_matrix'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_target_matrix ( source_port, & per_sp_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_target_matrix_entry_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port ; /* input */
	DRV_IH_TARGET_MATRIX_DESTINATION_PORT destination_port ; /* input */
	DRV_IH_TARGET_MATRIX_ENTRY entry ; /* output */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	source_port = parm[parameter_index++].value.unumber;
	destination_port = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_target_matrix_entry_'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_target_matrix_entry ( source_port, destination_port, & entry ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "target_memory: %s (%lu)\n\r" , bdmfmon_enum_stringval(target_memory_enum_table, entry.target_memory), entry.target_memory ) ;
        bdmf_session_print(session, "direct_mode: %lu\n\r" , entry.direct_mode ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_forward_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port ; /* input */
	DRV_IH_TARGET_MATRIX_DESTINATION_PORT destination_port ; /* input */
	bdmf_boolean forward ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	source_port = parm[parameter_index++].value.unumber;
	destination_port = parm[parameter_index++].value.unumber;
	forward = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_forward'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_forward ( source_port, destination_port, forward ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_forward_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port ; /* input */
	DRV_IH_TARGET_MATRIX_DESTINATION_PORT destination_port ; /* input */
	bdmf_boolean forward ; /* output */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	source_port = parm[parameter_index++].value.unumber;
	destination_port = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_forward'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_forward ( source_port, destination_port, (int32_t * const) & forward ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "forward: %lu\n\r" , forward ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_configure_wan_ports_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_WAN_PORTS_CONFIG wan_ports_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
    wan_ports_config.eth0 = parm[parameter_index++].value.unumber;
    wan_ports_config.eth1 = parm[parameter_index++].value.unumber;
    wan_ports_config.eth2 = parm[parameter_index++].value.unumber;
    wan_ports_config.eth3 = parm[parameter_index++].value.unumber;
    wan_ports_config.eth4 = parm[parameter_index++].value.unumber;
    wan_ports_config.gpon = parm[parameter_index++].value.unumber;
    wan_ports_config.runner_a = parm[parameter_index++].value.unumber;
    wan_ports_config.runner_b = parm[parameter_index++].value.unumber;
    wan_ports_config.pcie0 = parm[parameter_index++].value.unumber;
    wan_ports_config.pcie1 = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_wan_ports'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_wan_ports ( & wan_ports_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_wan_ports_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_WAN_PORTS_CONFIG wan_ports_config ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_wan_ports_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_wan_ports_configuration ( & wan_ports_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "eth0: %lu\n\r" , wan_ports_config.eth0 ) ;
        bdmf_session_print(session, "eth1: %lu\n\r" , wan_ports_config.eth1 ) ;
        bdmf_session_print(session, "eth2: %lu\n\r" , wan_ports_config.eth2 ) ;
        bdmf_session_print(session, "eth3: %lu\n\r" , wan_ports_config.eth3 ) ;
        bdmf_session_print(session, "eth4: %lu\n\r" , wan_ports_config.eth4 ) ;
        bdmf_session_print(session, "gpon: %lu\n\r" , wan_ports_config.gpon ) ;
        bdmf_session_print(session, "runner_a: %lu\n\r" , wan_ports_config.runner_a ) ;
        bdmf_session_print(session, "runner_b: %lu\n\r" , wan_ports_config.runner_b ) ;
        bdmf_session_print(session, "pcie0: %lu\n\r" , wan_ports_config.pcie0 ) ;
        bdmf_session_print(session, "pcie1: %lu\n\r" , wan_ports_config.pcie1 ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_allocated_runner_buffers_counters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint32_t runner_a_counter ; /* output */
	uint32_t runner_b_counter ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_allocated_runner_buffers_counters'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_allocated_runner_buffers_counters ( & runner_a_counter, & runner_b_counter ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "runner_a_counter: %lu\n\r" , runner_a_counter ) ;
        bdmf_session_print(session, "runner_b_counter: %lu\n\r" , runner_b_counter ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_critical_bits_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_CRITICAL_BITS critical_bits ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_critical_bits'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_critical_bits ( & critical_bits ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "iq_fifo_full: %lu\n\r" , critical_bits.iq_fifo_full ) ;
        bdmf_session_print(session, "iq0_fifo_full: %lu\n\r" , critical_bits.iq0_fifo_full ) ;
        bdmf_session_print(session, "iq1_fifo_full: %lu\n\r" , critical_bits.iq1_fifo_full ) ;
        bdmf_session_print(session, "iq2_fifo_full: %lu\n\r" , critical_bits.iq2_fifo_full ) ;
        bdmf_session_print(session, "iq3_fifo_full: %lu\n\r" , critical_bits.iq3_fifo_full ) ;
        bdmf_session_print(session, "iq4_fifo_full: %lu\n\r" , critical_bits.iq4_fifo_full ) ;
        bdmf_session_print(session, "iq5_fifo_full: %lu\n\r" , critical_bits.iq5_fifo_full ) ;
        bdmf_session_print(session, "iq6_fifo_full: %lu\n\r" , critical_bits.iq6_fifo_full ) ;
        bdmf_session_print(session, "iq7_fifo_full: %lu\n\r" , critical_bits.iq7_fifo_full ) ;
        bdmf_session_print(session, "lookup_1_stuck: %lu (Notice that in HW it is active-low)\n\r" , critical_bits.lookup_1_stuck ) ;
        bdmf_session_print(session, "lookup_2_stuck: %lu (Notice that in HW it is active-low)\n\r" , critical_bits.lookup_2_stuck ) ;
        bdmf_session_print(session, "lookup_3_stuck: %lu (Notice that in HW it is active-low)\n\r" , critical_bits.lookup_3_stuck ) ;
        bdmf_session_print(session, "lookup_4_stuck: %lu (Notice that in HW it is active-low)\n\r" , critical_bits.lookup_4_stuck ) ;
        bdmf_session_print(session, "look_up_packet_command_fifo_full: %lu\n\r" , critical_bits.look_up_packet_command_fifo_full ) ;
        bdmf_session_print(session, "egress_tx_data_fifo_full: %lu\n\r" , critical_bits.egress_tx_data_fifo_full ) ;
        bdmf_session_print(session, "egress_tx_message_fifo_full: %lu\n\r" , critical_bits.egress_tx_message_fifo_full ) ;
        bdmf_session_print(session, "egress_queue_packet_command_fifo_full: %lu\n\r" , critical_bits.egress_queue_packet_command_fifo_full ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_configure_parser_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_PARSER_CONFIG parser_config ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
    parser_config.tcp_flags = parm[parameter_index++].value.unumber;
    parser_config.exception_status_bits = parm[parameter_index++].value.unumber;
    parser_config.ppp_code_1_ipv6 = parm[parameter_index++].value.unumber;
    parser_config.ipv6_extension_header_bitmask = parm[parameter_index++].value.unumber;
    parser_config.snap_user_defined_organization_code = parm[parameter_index++].value.unumber;
    parser_config.snap_rfc1042_encapsulation_enable = parm[parameter_index++].value.unumber;
    parser_config.snap_802_1q_encapsulation_enable = parm[parameter_index++].value.unumber;
    parser_config.gre_protocol = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_parser'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_parser ( & parser_config ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_parser_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_PARSER_CONFIG parser_config ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;

    /* zero the bitmask to clear irrelevant bits, so they won't show in the printing */
    parser_config.ipv6_extension_header_bitmask =  0 ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_parser_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_parser_configuration ( & parser_config ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "tcp_flags: 0x%lX\n\r" , parser_config.tcp_flags ) ;
        bdmf_session_print(session, "exception_status_bits: 0x%lX\n\r" , parser_config.exception_status_bits ) ;
        bdmf_session_print(session, "ppp_code_1_ipv6: %lu\n\r" , parser_config.ppp_code_1_ipv6 ) ;
        bdmf_session_print(session, "ipv6_extension_header_bitmask: 0x%lX\n\r" , parser_config.ipv6_extension_header_bitmask ) ;
        bdmf_session_print(session, "snap_user_defined_organization_code: 0x%lX\n\r" , parser_config.snap_user_defined_organization_code ) ;
        bdmf_session_print(session, "snap_rfc1042_encapsulation_enable: %lu\n\r" , parser_config.snap_rfc1042_encapsulation_enable ) ;
        bdmf_session_print(session, "snap_802_1q_encapsulation_enable: %lu\n\r" , parser_config.snap_802_1q_encapsulation_enable ) ;
        bdmf_session_print(session, "gre_protocol: 0x%lX\n\r" , parser_config.gre_protocol ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_da_filter_with_mask_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	uint8_t mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ; /* input */
	uint8_t mask [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
    filter_index = parm[parameter_index++].value.unumber;
    memcpy(mac_address, parm[parameter_index++].value.mac, sizeof(mac_address));
    memcpy(mask, parm[parameter_index++].value.mac, sizeof(mask));

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_da_filter_with_mask'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_da_filter_with_mask ( filter_index, mac_address, mask ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_da_filter_with_mask_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	uint8_t mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ; /* output */
	uint8_t mask [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ; /* output */
	DRV_IH_ERROR error ;
    uint32_t byte_index ;

	/* Get the parameters */
	filter_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_da_filter_with_mask'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_da_filter_with_mask ( filter_index, mac_address, mask ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "mac_address: " ) ;
        for ( byte_index = 0 ; byte_index < DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ; byte_index ++ )
        {
            bdmf_session_print(session, "%02X ", mac_address [ byte_index ] ) ;
        }
        bdmf_session_print(session, "\n\r" ) ;

        bdmf_session_print(session, "mask: " ) ;
        for ( byte_index = 0 ; byte_index < DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ; byte_index ++ )
        {
            bdmf_session_print(session, "%02X ", mask [ byte_index ] ) ;
        }
        bdmf_session_print(session, "\n\r" ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_da_filter_without_mask_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	uint8_t mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

    parameter_index = 0 ;
    filter_index = parm[parameter_index++].value.unumber;
    memcpy(mac_address, parm[parameter_index++].value.mac, sizeof(mac_address));

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_da_filter_without_mask'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_da_filter_without_mask ( filter_index, mac_address ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_da_filter_without_mask_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
    uint8_t mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ; /* output */
	DRV_IH_ERROR error ;
    uint32_t byte_index ;

	/* Get the parameters */
	filter_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_da_filter_without_mask'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_da_filter_without_mask ( filter_index, mac_address ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "mac_address: " ) ;
        for ( byte_index = 0 ; byte_index < DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ; byte_index ++ )
        {
            bdmf_session_print(session, "%02X ", mac_address [ byte_index ] ) ;
        }
        bdmf_session_print(session, "\n\r" ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_enable_da_filter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	bdmf_boolean enable ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	filter_index = parm[parameter_index++].value.unumber;
	enable = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_enable_da_filter'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_enable_da_filter ( filter_index, enable ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_da_filter_enable_status_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	bdmf_boolean enable ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	filter_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_da_filter_enable_status'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_da_filter_enable_status ( filter_index, (int32_t * const) & enable ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "enable: %lu\n\r" , enable ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_ethertypes_for_qtag_identification_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint16_t ethertype_0 ; /* input */
	uint16_t ethertype_1 ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	ethertype_0 = parm[parameter_index++].value.unumber;
	ethertype_1 = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_ethertypes_for_qtag_identification'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_ethertypes_for_qtag_identification ( ethertype_0, ethertype_1 ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_ethertypes_for_qtag_identification_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint16_t ethertype_0 ; /* output */
	uint16_t ethertype_1 ; /* output */

	/* Define parameters handling variables */
	DRV_IH_ERROR error ;
	
	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_ethertypes_for_qtag_identification'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_ethertypes_for_qtag_identification ( & ethertype_0, & ethertype_1 ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "ethertype_0: 0x%lX\n\r" , ethertype_0 ) ;
        bdmf_session_print(session, "ethertype_1: 0x%lX\n\r" , ethertype_1 ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_configure_qtag_nesting_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_ETHERTYPE_FOR_QTAG_NESTING ethertype_index ; /* input */
	bdmf_boolean use_as_outer ; /* input */
	bdmf_boolean use_as_inner ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	ethertype_index = parm[parameter_index++].value.unumber;
	use_as_outer = parm[parameter_index++].value.unumber;
	use_as_inner = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_qtag_nesting'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_qtag_nesting ( ethertype_index, use_as_outer, use_as_inner ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_qtag_nesting_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	DRV_IH_ETHERTYPE_FOR_QTAG_NESTING ethertype_index ; /* input */
	bdmf_boolean use_as_outer ; /* output */
	bdmf_boolean use_as_inner ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	ethertype_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_qtag_nesting_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_qtag_nesting_configuration ( ethertype_index, (int32_t * const) & use_as_outer, (int32_t * const) & use_as_inner ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "use_as_outer: %lu\n\r" , use_as_outer ) ;
        bdmf_session_print(session, "use_as_inner: %lu\n\r" , use_as_inner ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_configure_user_ethertype_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t ethertype_index ; /* input */
	uint16_t ethertype_value ; /* input */
	DRV_IH_L3_PROTOCOL l3_protocol ; /* input */
	uint8_t l3_offset ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	ethertype_index = parm[parameter_index++].value.unumber;
	ethertype_value = parm[parameter_index++].value.unumber;
	l3_protocol = parm[parameter_index++].value.unumber;
	l3_offset = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_configure_user_ethertype'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_configure_user_ethertype ( ethertype_index, ethertype_value, l3_protocol, l3_offset ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_user_ethertype_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t ethertype_index ; /* input */
	uint16_t ethertype_value ; /* output */
	DRV_IH_L3_PROTOCOL l3_protocol ; /* output */
	uint8_t l3_offset ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	ethertype_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_user_ethertype_configuration'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_user_ethertype_configuration ( ethertype_index, & ethertype_value, & l3_protocol, & l3_offset ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "ethertype_value: 0x%lX\n\r" , ethertype_value ) ;
        bdmf_session_print(session, "l3_protocol: %s (%lu)\n\r" , bdmfmon_enum_stringval(l3_protocol_enum_table, l3_protocol ) , l3_protocol ) ;
        bdmf_session_print(session, "l3_offset: %lu\n\r" , l3_offset ) ;
        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_enable_user_ethertype_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t ethertype_index ; /* input */
	bdmf_boolean enable ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	ethertype_index = parm[parameter_index++].value.unumber;
	enable = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_enable_user_ethertype'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_enable_user_ethertype ( ethertype_index, enable ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_user_ethertype_enable_status_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t ethertype_index ; /* input */
	bdmf_boolean enable ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	ethertype_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_user_ethertype_enable_status'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_user_ethertype_enable_status ( ethertype_index, (int32_t * const) & enable ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "enable: %lu\n\r" , enable ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_user_ip_l4_protocol_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t l4_protocol_index ; /* input */
	uint8_t l4_protocol_value ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	l4_protocol_index = parm[parameter_index++].value.unumber;
	l4_protocol_value = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_user_ip_l4_protocol'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_user_ip_l4_protocol ( l4_protocol_index, l4_protocol_value ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_user_ip_l4_protocol_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t l4_protocol_index ; /* input */
	uint8_t l4_protocol_value ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	l4_protocol_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_user_ip_l4_protocol'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_user_ip_l4_protocol ( l4_protocol_index, & l4_protocol_value ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "l4_protocol_value: 0x%lX\n\r" , l4_protocol_value ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_ppp_code_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t ppp_code_index ; /* input */
	uint16_t ppp_code ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	ppp_code_index = parm[parameter_index++].value.unumber;
	ppp_code = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_ppp_code'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_ppp_code ( ppp_code_index, ppp_code ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_ppp_code_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t ppp_code_index ; /* input */
	uint16_t ppp_code ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	ppp_code_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_ppp_code'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_ppp_code ( ppp_code_index, & ppp_code ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "ppp_code: 0x%lX\n\r" , ppp_code ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_vid_filter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	uint16_t vid ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	filter_index = parm[parameter_index++].value.unumber;
	vid = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_vid_filter'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_vid_filter ( filter_index, vid ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_vid_filter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	uint16_t vid ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	filter_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_vid_filter'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_vid_filter ( filter_index, & vid ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "vid: %lu\n\r" , vid ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_enable_vid_filter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	bdmf_boolean enable ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	filter_index = parm[parameter_index++].value.unumber;
	enable = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_enable_vid_filter'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_enable_vid_filter ( filter_index, enable ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_vid_filter_enable_status_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	bdmf_boolean enable ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	filter_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_vid_filter_enable_status'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_vid_filter_enable_status ( filter_index, (int32_t * const) & enable ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "enable: %lu\n\r" , enable ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_ip_filter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	uint32_t ip_address ; /* input */
	uint32_t ip_address_mask ; /* input */
	DRV_IH_IP_FILTER_SELECTION selection ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	filter_index = parm[parameter_index++].value.unumber;
	ip_address = parm[parameter_index++].value.unumber;
	ip_address_mask = parm[parameter_index++].value.unumber;
	selection = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_ip_filter'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_ip_filter ( filter_index, ip_address, ip_address_mask, selection ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_ip_filter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	uint32_t ip_address ; /* output */
	uint32_t ip_address_mask ; /* output */
	DRV_IH_IP_FILTER_SELECTION selection ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	filter_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_ip_filter'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_ip_filter ( filter_index, (uint32_t * const)&ip_address,
	    (uint32_t * const)&ip_address_mask, &selection ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "ip_address: %d.%d.%d.%d\n\r",
            (ip_address >> 24) & 0xff,
            (ip_address >> 16) & 0xff,
            (ip_address >> 8) & 0xff,
            ip_address & 0xff) ;

        bdmf_session_print(session, "ip_address_mask: %d.%d.%d.%d\n\r",
            (ip_address_mask >> 24) & 0xff,
            (ip_address_mask >> 16) & 0xff,
            (ip_address_mask >> 8) & 0xff,
            ip_address_mask & 0xff) ;

        bdmf_session_print(session, "selection: %s (%lu)\n\r" , bdmfmon_enum_stringval(ip_filter_enum_table, selection ) , selection ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_enable_ip_filter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	bdmf_boolean enable ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	filter_index = parm[parameter_index++].value.unumber;
	enable = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_enable_ip_filter'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_enable_ip_filter ( filter_index, enable ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_ip_filter_enable_status_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t filter_index ; /* input */
	bdmf_boolean enable ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	filter_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_ip_filter_enable_status'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_ip_filter_enable_status ( filter_index, (int32_t * const) & enable ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "enable: %lu\n\r" , enable ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_dscp_to_tci_table_entry_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	uint8_t dscp ; /* input */
	uint8_t tci ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	table_index = parm[parameter_index++].value.unumber;
	dscp = parm[parameter_index++].value.unumber;
	tci = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_dscp_to_tci_table_entry'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_dscp_to_tci_table_entry ( table_index, dscp, tci ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_dscp_to_tci_table_entry_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	uint8_t dscp ; /* input */
	uint8_t tci ; /* output */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	table_index = parm[parameter_index++].value.unumber;
	dscp = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_dscp_to_tci_table_entry'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_dscp_to_tci_table_entry ( table_index, dscp, & tci ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "tci: %lu\n\r" , tci ) ;
        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_set_default_tci_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	uint8_t default_tci ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	table_index = parm[parameter_index++].value.unumber;
	default_tci = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_set_default_tci'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_set_default_tci ( table_index, default_tci ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_default_tci_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	uint8_t default_tci ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	table_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_default_tci'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_default_tci ( table_index, & default_tci ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "default_tci: %lu\n\r" , default_tci ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_enable_dscp_to_tci_table_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	bdmf_boolean enable ; /* input */
	DRV_IH_ERROR error ;
    uint8_t parameter_index ;

	/* Get the parameters */
    parameter_index = 0 ;
	table_index = parm[parameter_index++].value.unumber;
	enable = parm[parameter_index++].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_enable_dscp_to_tci_table'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_enable_dscp_to_tci_table ( table_index, enable ) ;
	/* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_dscp_to_tci_table_enable_status_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	bdmf_boolean enable ; /* output */
	DRV_IH_ERROR error ;

	/* Get the parameters */
	table_index = parm[0].value.unumber;

	/* Call the under test routine */
	bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_ih_get_dscp_to_tci_table_enable_status'\n\r" ) ;
	/* -------------------------------------------------------------------- */
	error = fi_bl_drv_ih_get_dscp_to_tci_table_enable_status ( table_index, (int32_t * const) & enable ) ;
	/* -------------------------------------------------------------------- */

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "enable: %lu\n\r" , enable ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_get_lut_five_tuple_enable_status_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	/* Define I/O variables */
	uint8_t table_index ; /* input */
	bdmf_boolean five_tuple_enable ; /* output */
	DRV_IH_ERROR error ;
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG dummy ;

	/* Get the parameters */
	table_index = parm[0].value.unumber;

	error = fi_get_lut_all_parameters ( table_index, & dummy , & five_tuple_enable ) ;

    if ( error == DRV_IH_NO_ERROR )
    {
        bdmf_session_print(session, "five_tuple_enable: %lu\n\r" , five_tuple_enable ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_ih_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_dump_all_configured_classes_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	DRV_IH_CLASS_CONFIG class_config ;
	DRV_IH_ERROR error = 0;
    uint8_t class_index ;

    for ( class_index = 0 ; class_index < DRV_IH_NUMBER_OF_CLASSES ; ++ class_index )
    {
        if ( fi_is_class_configured ( class_index ) == 1 )
        {
            bdmf_session_print(session, "Class %d:\n\r" , class_index ) ;

            error = fi_bl_drv_ih_get_class_configuration ( class_index , & class_config ) ;
            if ( error == DRV_IH_NO_ERROR )
            {
                p_dump_class_configuration( session, & class_config ) ;
                bdmf_session_print(session, "\n\r" ) ;
            }
            else
            {
                bdmf_session_print(session, "get_class_configuration failed, error = '%s'\n\r" ,
                                  f_drv_ih_error_code_to_string ( error ) ) ;
            }
        }
    }

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_ih_dump_all_configured_classifiers_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	DRV_IH_CLASSIFIER_CONFIG classifier_configuration ;
	DRV_IH_ERROR error = 0;
    uint8_t classifier_index ;

    for ( classifier_index = 0 ; classifier_index < DRV_IH_NUMBER_OF_CLASSIFIERS ; ++ classifier_index )
    {
        if ( fi_is_classifier_configured ( classifier_index ) == 1 )
        {
            bdmf_session_print(session, "Classifier %d:\n\r" , classifier_index ) ;

            error = fi_bl_drv_ih_get_classifier_configuration ( classifier_index , & classifier_configuration ) ;
            if ( error == DRV_IH_NO_ERROR )
            {
                p_dump_classifier_configuration( session, &classifier_configuration ) ;
                bdmf_session_print(session, "\n\r" ) ;
            }
            else
            {
                bdmf_session_print(session, "get_classifier_configuration failed, error = '%s'\n\r" ,
                                  f_drv_ih_error_code_to_string ( error ) ) ;
            }
        }
    }

    return error ? BDMF_ERR_PARM : 0;
}


static bdmfmon_handle_t ih_dir;


void pi_bl_initialize_drv_ih_shell(bdmfmon_handle_t driver_dir)
{
    ih_dir = bdmfmon_dir_add(driver_dir, "ihd", "IH Driver", BDMF_ACCESS_ADMIN, NULL );
    if (!ih_dir)
    {
        bdmf_session_print(NULL, "Can't create ihd directory\n");
        return;
    }

	BDMFMON_MAKE_CMD(ih_dir, "sgc", "Set general configuration", p_bl_drv_ih_set_general_configuration_command,
        BDMFMON_MAKE_PARM_RANGE("rnr_a_ra", "runner_a_ih_response_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_ra", "runner_b_ih_response_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("rnr_a_cra", "runner_a_ih_congestion_report_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_cra", "runner_b_ih_congestion_report_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_ENUM("rnr_a_cr_enable", "runner_a_ih_congestion_report_enable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("rnr_b_cr_enable", "runner_b_ih_congestion_report_enable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("luts_enable", "lut_searches_enable_in_direct_mode", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("sns_enable", "sn_stamping_enable_in_direct_mode", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM("min_hdr_len", "header_length_minimum", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_ENUM("cng_discard_disable", "congestion_discard_disable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("cams_enable_if_inv_lut", "cam_search_enable_upon_invalid_lut_entry", bdmfmon_enum_bool_table, 0));

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "ggc", "Get general configuration", p_bl_drv_ih_get_general_configuration_command);

	BDMFMON_MAKE_CMD(ih_dir, "spho", "Set packet header offsets", p_bl_drv_ih_set_packet_header_offsets_command,
        BDMFMON_MAKE_PARM("eth0_pho", "eth0_packet_header_offset", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("eth1_pho", "eth1_packet_header_offset", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("eth2_pho", "eth2_packet_header_offset", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("eth3_pho", "eth3_packet_header_offset", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("eth4_pho", "eth4_packet_header_offset", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("gpon_pho", "gpon_packet_header_offset", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("rnr_a_pho", "runner_a_packet_header_offset", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("rnr_b_pho", "runner_b_packet_header_offset", BDMFMON_PARM_NUMBER, 0));

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "gpho", "Get packet header offsets", p_bl_drv_ih_get_packet_header_offsets_command);

	BDMFMON_MAKE_CMD(ih_dir, "srbc", "Set Runner Buffers configuration", p_bl_drv_ih_set_runner_buffers_configuration_command,
        BDMFMON_MAKE_PARM_RANGE("rnr_a_ih_rb_base", "runner_a_ih_managed_rb_base_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_ih_rb_base", "runner_b_ih_managed_rb_base_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("rnr_a_rnr_rb_base", "runner_a_runner_managed_rb_base_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_rnr_rb_base", "runner_b_runner_managed_rb_base_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_ENUM("rnr_a_max_buf", "runner_a_maximal_number_of_buffers", rnr_max_buf_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("rnr_b_max_buf", "runner_b_maximal_number_of_buffers", rnr_max_buf_enum_table, 0));

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "grbc", "Get Runner Buffers configuration", p_bl_drv_ih_get_runner_buffers_configuration_command);

	BDMFMON_MAKE_CMD(ih_dir, "srlt", "Set Runners Load Thresholds", p_bl_drv_ih_set_runners_load_thresholds_command,
        BDMFMON_MAKE_PARM_RANGE("rnr_a_hc_thresh", "runner_a_high_congestion_threshold", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_hc_thresh", "runner_b_high_congestion_threshold", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS),
        BDMFMON_MAKE_PARM_RANGE("rnr_a_ec_thresh", "runner_a_exclusive_congestion_threshold", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_ec_thresh", "runner_b_exclusive_congestion_threshold", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS),
        BDMFMON_MAKE_PARM_RANGE("rnr_a_lb_thresh", "runner_a_load_balancing_threshold", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_lb_thresh", "runner_b_load_balancing_threshold", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS),
        BDMFMON_MAKE_PARM_RANGE("rnr_a_lb_hyst", "runner_a_load_balancing_hysteresis", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_lb_hyst", "runner_b_load_balancing_hysteresis", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS));

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "grlt", "Get Runners Load Thresholds", p_bl_drv_ih_get_runners_load_thresholds_command);

	BDMFMON_MAKE_CMD(ih_dir, "sra", "Set Route Addresses\n"
	    "This function sets route address for each physical port.\n"
	    "The route address is used for broad-bus access for sending responses, message and data.\n",
	    p_bl_drv_ih_set_route_addresses_command,
        BDMFMON_MAKE_PARM_RANGE("eth0_route_addr", "eth0_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("eth1_route_addr", "eth1_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("eth2_route_addr", "eth2_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("eth3_route_addr", "eth3_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("eth4_route_addr", "eth4_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("gpon_route_addr", "gpon_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("rnr_a_route_addr", "runner_a_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_route_addr", "runner_b_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f));

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "gra", "Get Route Addresses\n"
        "This function prints route address for each physical port.\n"
        "The route address is used for broad-bus access for sending responses, message and data.\n",
	    p_bl_drv_ih_get_route_addresses_command);

	BDMFMON_MAKE_CMD(ih_dir, "slpc", "Set Logical Ports Configuration\n"
	    "This function sets configuration of the following logical ports:\n"
	    "  Ethernet 0-4, GPON, Runner A, Runner B and PCIE 0-1.\n"
	    "The following parameters are configured per port:\n"
	    "  Parsing layer depth, Proprietary tag size.",
	    p_bl_drv_ih_set_logical_ports_configuration_command,
        BDMFMON_MAKE_PARM_ENUM("eth0_pld", "eth0_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth0_pts", "eth0_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth1_pld", "eth1_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth1_pts", "eth1_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth2_pld", "eth2_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth2_pts", "eth2_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth3_pld", "eth3_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth3_pts", "eth3_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth4_pld", "eth4_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth4_pts", "eth4_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("gpon_pld", "gpon_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("gpon_pts", "gpon_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("rnr_a_pld", "rnr_a_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("rnr_a_pts", "rnr_a_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("rnr_b_pld", "rnr_b_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("rnr_b_pts", "rnr_b_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("pcie0_pld", "pcie0_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("pcie0_pts", "pcie0_proprietary_tag_size", prop_tag_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("pcie1_pld", "pcie1_parsing_layer_depth", parse_layer_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("pcie1_pts", "pcie1_proprietary_tag_size", prop_tag_size_enum_table, 0));

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "glpc", "Get Logical Ports Configuration\n"
        "This function sets configuration of the following logical ports:\n"
        "  Ethernet 0-4, GPON, Runner A, Runner B and PCIE 0-1.\n", p_bl_drv_ih_get_logical_ports_configuration_command);

	BDMFMON_MAKE_CMD(ih_dir, "cl60bk", "Configure Lookup Table 60 bit key\n"
	    "This function configures a lookup table with 60-bit key.\n"
	    "There is a total of 10 tables.\n"
	    "Note that when configuring a lookup table with 120-bit key (with dedicated API), it occupies 2 tables.\n"
	    "The lookup key is obtained by ORing two 60-bit parts taken from the parser result.\n"
	    "Each part has a configurable offset, and optionally a shift & rotate operation.\n"
	    "Initially 64 bits are taken from the configured offset,\n"
	    "then shift & rotate is optionally done, then the 4 MS-bits are omitted.\n"
	    "Then each part is masked with its own mask.\n"
	    "Then ORing the left 60 bits of the 2 parts yields the key.\n"
	    "Then a key-extension can optionally be done,\n"
	    "i.e. ORing the MS-bits of the key with one of the following values:\n"
	    "  (1) 5-bit Source Port from Header Descriptor.\n"
	    "  (2) 8-bit GEM Flow ID from Header Descriptor.\n"
	    "  (3) 1-bit WAN/LAN indication extracted from configuration of the source port.\n"
	    "The global mask is applied on both key & LUT entry when comparing between them.\n"
	    "Move indication: When 'Source port search enable' parameter is enabled,\n"
	    "additional comparison will be done, between source-port (from Header descriptor)\n"
	    " and bits 56:52 of LUT entry, where source-port value should reside.\n"
	    "In this case, the global mask must mask these bits for the regular comparison\n"
	    "between the key and LUT entry, which would be a MAC address comparison.\n"
	    "If both comparisons match, the result would be 'hit'.\n"
	    "If only MAC address comparison matches, the result would be 'move'.",
	    p_bl_drv_ih_configure_lut_60_bit_key_command,
        BDMFMON_MAKE_PARM_RANGE("tbl_index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_LOOKUP_TABLES - 1),
        BDMFMON_MAKE_PARM_RANGE("tbl_ba", "table_base_address_in_8_byte", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_ENUM("tbl_size", "table_size", lookup_table_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("search_depth", "maximal_search_depth", search_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("hash_type", "hash_type", hash_type_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("sa_search_enable", "sa_search_enable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("aging_enable", "aging_enable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("cam_enable", "cam_enable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_RANGE("cam_ba", "cam_base_address_in_8_byte", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_ba", "context_table_base_address_in_8_byte", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_ENUM("ctx_entry_size", "context_table_entry_size", ctx_entry_size_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("cam_ctx_ba", "cam_context_base_address_in_8_byte", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("part0_so", "part_0_start_offset_in_4_byte", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("part0_sfo", "part_0_shift_offset_in_4_bit", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("part1_so", "part_1_start_offset_in_4_byte", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("part1_sfo", "part_1_shift_offset_in_4_bit", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_ENUM("key_ext", "key_extension", lookup_key_ext_enum_table, 0),
        BDMFMON_MAKE_PARM("part0_mask_low", "part_0_mask_low", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("part0_mask_high", "part_0_mask_high", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM("part1_mask_low", "part_1_mask_low", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("part1_mask_high", "part_1_mask_high", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM_RANGE("global_mask", "global_mask_in_4_bit", BDMFMON_PARM_HEX, 0, 0, 0x7fff));


	BDMFMON_MAKE_CMD(ih_dir, "gl60bkc", "Get Lookup Table 60 bit key configuration", p_bl_drv_ih_get_lut_60_bit_key_configuration_command,
        BDMFMON_MAKE_PARM_RANGE("tbl_index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_LOOKUP_TABLES - 1));

	BDMFMON_MAKE_CMD(ih_dir, "cl120bk", "Configure Lookup Table 120 bit key\n"
	    "Configuring a lookup table with 120-bit key occupies 2 tables\n"
	    "(out of total of 10): primary table and secondary table.\n"
	    "The secondary table is used only for generation of the secondary key (see below).\n"
	    "The primary table must be defined as search 1 or 3 of a class\n"
	    "and the secondary must be 2 of 4 respectively.\n"
	    "Two 60 bit keys are defined per table: primary key and secondary key.\n"
	    "Each one of these keys is generated the same way as explained in\n"
	    "Configure Lookup Table 60 bit key API description.\n"
	    "The 120 bit key is composed of these two 60 bit keys.\n"
	    "The 60 bit global mask is applied on secondary key only.\n"
	    "SA search (for Move indication) is not supported for 120 bit key.",
	    p_bl_drv_ih_configure_lut_120_bit_key_command,
        BDMFMON_MAKE_PARM_RANGE("prim_tbl_index", "primary_table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_LOOKUP_TABLES - 1),
        BDMFMON_MAKE_PARM_RANGE("sec_tbl_index", "secondary_table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_LOOKUP_TABLES - 1),
        BDMFMON_MAKE_PARM_RANGE("tbl_ba", "table_base_address_in_8_byte", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_ENUM("tbl_size", "table_size", lookup_table_size_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("search_depth", "maximal_search_depth", search_depth_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("hash_type", "hash_type", hash_type_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("aging_enable", "aging_enable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("cam_enable", "cam_enable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_RANGE("cam_ba", "cam_base_address_in_8_byte", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_ba", "context_table_base_address_in_8_byte", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_ENUM("ctx_entry_size", "context_table_entry_size", ctx_entry_size_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("cam_ctx_ba", "cam_context_base_address_in_8_byte", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("key1_part0_so", "primary_key_part_0_start_offset_in_4_byte", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("key1_part0_sfo", "primary_key_part_0_shift_offset_in_4_bit", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("key1_part1_so", "primary_key_part_1_start_offset_in_4_byte", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("key1_part1_sfo", "primary_key_part_1_shift_offset_in_4_bit", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_ENUM("key1_key_ext", "primary_key_key_extension", lookup_key_ext_enum_table, 0),
        BDMFMON_MAKE_PARM("key1_part0_mask_low", "primary_part_0_mask_low", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("key1_part0_mask_high", "primary_part_0_mask_high", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM("key1_part1_mask_low", "primary_part_1_mask_low", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("key1_part1_mask_high", "primary_part_1_mask_high", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM_RANGE("key2_part0_so", "secondary_key_part_0_start_offset_in_4_byte", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("key2_part0_sfo", "secondary_key_part_0_shift_offset_in_4_bit", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("key2_part1_so", "secondary_key_part_1_start_offset_in_4_byte", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_RANGE("key2_part1_sfo", "secondary_key_part_1_shift_offset_in_4_bit", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART),
        BDMFMON_MAKE_PARM_ENUM("key2_key_ext", "secondary_key_key_extension", lookup_key_ext_enum_table, 0),
        BDMFMON_MAKE_PARM("key2_part0_mask_low", "secondary_part_0_mask_low", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("key2_part0_mask_high", "secondary_part_0_mask_high", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM("key2_part1_mask_low", "secondary_part_1_mask_low", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("key2_part1_mask_high", "secondary_part_1_mask_high", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM_RANGE("global_mask", "global_mask_in_4_bit", BDMFMON_PARM_HEX, 0, 0, 0x7fff));

	BDMFMON_MAKE_CMD(ih_dir, "gl120bkc", "Get Lookup Table 120 bit key configuration", p_bl_drv_ih_get_lut_120_bit_key_configuration_command,
        BDMFMON_MAKE_PARM_RANGE("prim_tbl_index", "primary_table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_LOOKUP_TABLES - 1),
        BDMFMON_MAKE_PARM_RANGE("sec_tbl_index", "secondary_table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_LOOKUP_TABLES - 1));

	BDMFMON_MAKE_CMD(ih_dir, "cc", "Configure Class\n"
	    "This function configures an IH class.\n"
	    "Ingress handler class is type of ingress traffic, e.g. IPTV, bridged, routed.\n"
	    "Each class includes predefined set of settings, such as:\n"
	    "  target runner, destination memory, QoS, definition of look-up searches.\n"
	    "There are up to 16 classes.\n"
	    "Each physical port has a default class (For GPON port, default class is per GEM flow).\n"
	    "IH may override the default class according to enable-override configuration\n"
	    " and to classification based on reduced Parser results, called 'Classifier Key Word'\n"
	    " (Parser Summary Word plus\nsource port).\n"
	    "Default classes and override-enable configurations are in BBH.\n"
	    "In runner flow, runner assigns an initial class (which can be overridden by IH).\n"
	    "Class override is done using classifiers, which are configured using Configure Classifier API.",
	    p_bl_drv_ih_configure_class_command,
        BDMFMON_MAKE_PARM_RANGE("class_index", "class_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_CLASSES - 1),
        BDMFMON_MAKE_PARM_ENUM("class_search_1", "class_search_1", class_search_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("class_search_2", "class_search_1", class_search_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("class_search_3", "class_search_3", class_search_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("class_search_4", "class_search_4", class_search_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("dst_port_extraction", "destination_port_extraction", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("class_search_4", "class_search_4", op_based_on_class_search_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("dcsp_to_tci_tbl", "dscp_to_tci_table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES - 1),
        BDMFMON_MAKE_PARM("direct_mode_default", "direct_mode_default", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_ENUM("direct_mode_override", "direct_mode_override", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("target_memory_default", "target_memory_default", target_memory_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("target_memory_override", "target_memory_override", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("ingress_qos_default", "ingress_qos_default", ingress_qos_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("ingress_qos_override", "ingress_qos_override", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("runner_default", "target_runner_default", runner_cluster_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("runner_override_direct", "target_runner_override_in_direct_mode", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("runner_direct", "target_runner_for_direct_mode", runner_cluster_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("load_balance", "load_balancing_enable", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("pref_load_balance", "preference_load_balancing_enable", bdmfmon_enum_bool_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gcc", "Get Class configuration", p_bl_drv_ih_get_class_configuration_command,
        BDMFMON_MAKE_PARM_RANGE("class_index", "class_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_CLASSES - 1));

	BDMFMON_MAKE_CMD(ih_dir, "cc1", "Configure Classifier\n"
	    "This function configures a classifier. There are up to 16 classifiers.\n"
	    "A classifier is a pair of key and mask, and a resulting class.\n"
	    "When class override is enabled, the key is compared to the masked Classifier Key Word\n"
	    "(the mask is NOT applied on the key, so user is responsible to set 0 at the masked \n"
	    "fields/bits in the key).\n"
	    "In case of match, the default class is overridden by the classifier's resulting class\n."
	    "If there is a match in more than one classifier, the one with the lower index is chosen.",
	    p_bl_drv_ih_configure_classifier_command,
        BDMFMON_MAKE_PARM_RANGE("index", "classifier_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_CLASSIFIERS - 1),
        BDMFMON_MAKE_PARM_ENUM("l2_prot", "L2 protocol", l2_protocol_enum_table, 0),
	    BDMFMON_MAKE_PARM_ENUM("l3_prot", "L3 protocol", l3_protocol_enum_table, 0),
	    BDMFMON_MAKE_PARM_ENUM("l4_prot", "L4 protocol", l4_protocol_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("da_filter_any_hit", "da_filter_any_hit", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_RANGE("matched_da_filter", "matched_da_filter", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DA_FILTERS - 1),
        BDMFMON_MAKE_PARM("mcast_da_indication", "mcast_da_indication", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("bcast_da_indication", "bcast_da_indication", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_ENUM("vid_filter_any_hit", "vid_filter_any_hit", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_RANGE("matched_vid_filter", "matched_vid_filter", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_VID_FILTERS - 1),
        BDMFMON_MAKE_PARM_ENUM("ip_filter_any_hit", "ip_filter_any_hit", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_RANGE("matched_ip_filter", "matched_ip_filter", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_IP_FILTERS - 1),
        BDMFMON_MAKE_PARM("wan_indication", "wan_indication", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_ENUM("five_tuple_valid", "five_tuple_valid", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("l4_prot", "L4 protocol", logical_port_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("error", "error", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM("result", "resulting_class", BDMFMON_PARM_NUMBER, 0));


	BDMFMON_MAKE_CMD(ih_dir, "gcc1", "Get Classifier configuration", p_bl_drv_ih_get_classifier_configuration_command,
        BDMFMON_MAKE_PARM_RANGE("index", "classifier_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_CLASSIFIERS - 1));

    BDMFMON_MAKE_CMD(ih_dir, "rc", "Remove Classifier", p_bl_drv_ih_remove_classifier_command,
        BDMFMON_MAKE_PARM_RANGE("index", "classifier_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_CLASSIFIERS - 1));

	BDMFMON_MAKE_CMD(ih_dir, "ssptiqm", "Set source port to ingress queue mapping\n"
	    "This function sets the mapping of physical source ports\n"
	    "  (eth0-4, GPON,\nrunner A, runner B) to ingress queues.\n"
	    "There are 8 ingress queues. BBH or runner (in case of runner flow)\n"
	    "writes the Header Descriptor to one of these queues, according to \n"
	    "the configuration of the source port.",
	    p_bl_drv_ih_set_source_port_to_ingress_queue_mapping_command,
        BDMFMON_MAKE_PARM_RANGE("eth0", "eth0_ingress_queue", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1),
        BDMFMON_MAKE_PARM_RANGE("eth1", "eth1_ingress_queue", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1),
        BDMFMON_MAKE_PARM_RANGE("eth2", "eth2_ingress_queue", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1),
        BDMFMON_MAKE_PARM_RANGE("eth3", "eth3_ingress_queue", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1),
        BDMFMON_MAKE_PARM_RANGE("eth4", "eth4_ingress_queue", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1),
        BDMFMON_MAKE_PARM_RANGE("gpon", "gpon_ingress_queue", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1),
        BDMFMON_MAKE_PARM_RANGE("rnr_a", "runner_a_ingress_queue", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1),
        BDMFMON_MAKE_PARM_RANGE("rnr_b", "runner_b_ingress_queue", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1));


	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "gsptiqm", "Get source port to ingress queue mapping", p_bl_drv_ih_get_source_port_to_ingress_queue_mapping_command);

	BDMFMON_MAKE_CMD(ih_dir, "ciq", "Configure ingress queue\n"
	    "This function configures an ingress queue.\n"
	    "There are 8 queues. All of them reside in the same \n"
	    "  Ingress-queue (IQ) array of 16 entries.\n"
	    "E.g.\nqueue 0 occupies entries 0-1, queue 1 occupies entries 2-3, etc.",
	    p_bl_drv_ih_configure_ingress_queue_command,
        BDMFMON_MAKE_PARM_RANGE("index", "ingress_queue_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1),
        BDMFMON_MAKE_PARM_RANGE("base", "base_location", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_INGRESS_QUEUES_ARRAY_SIZE - 1),
        BDMFMON_MAKE_PARM_RANGE("size", "size", BDMFMON_PARM_NUMBER, 0, DRV_IH_MINIMAL_INGRESS_QUEUE_SIZE, DRV_IH_MINIMAL_INGRESS_QUEUE_SIZE),
        BDMFMON_MAKE_PARM_RANGE("priority", "priority", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_INGRESS_QUEUE_PRIORITY),
        BDMFMON_MAKE_PARM_RANGE("weight", "weight", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_INGRESS_QUEUE_WEIGHT),
        BDMFMON_MAKE_PARM_RANGE("congestion_threshold", "congestion_threshold", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS));

	BDMFMON_MAKE_CMD(ih_dir, "giqc", "Get ingress queue configuration", p_bl_drv_ih_get_ingress_queue_configuration_command,
        BDMFMON_MAKE_PARM_RANGE("index", "ingress_queue_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_INGRESS_QUEUES - 1));

	BDMFMON_MAKE_CMD(ih_dir, "stm", "Set Target Matrix\n"
	        "This function sets the per-source-port configuration in the target matrix,\n"
	        " i.e. all entries which belong to the given source port.\n"
	        "Each entry consists of the following parameters:\n"
	        "  target memory (DDR/SRAM), direct mode (true/false).\n"
	        "The function will fail when trying to configure an \n"
	        " 'Always DDR' entry with Target memory = SRAM, or \n"
	        " 'Always SRAM' entry with Target memory = DDR.",
	        p_bl_drv_ih_set_target_matrix_command,
	        BDMFMON_MAKE_PARM_ENUM("src_port", "source port", target_matrix_src_port_enum_table, 0),
	        BDMFMON_MAKE_PARM_ENUM("eth0_target_memory", "eth0_target_memory", target_memory_enum_table, 0),
	        BDMFMON_MAKE_PARM_ENUM("eth0_direct_mode", "eth0_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("eth1_target_memory", "eth1_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("eth1_direct_mode", "eth1_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("eth2_target_memory", "eth2_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("eth2_direct_mode", "eth2_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("eth3_target_memory", "eth3_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("eth3_direct_mode", "eth3_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("eth4_target_memory", "eth4_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("eth4_direct_mode", "eth4_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("gpon_target_memory", "gpon_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("gpon_direct_mode", "gpon_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("pcie0_target_memory", "pcie0_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("pcie0_direct_mode", "pcie0_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("pcie1_target_memory", "pcie1_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("pcie1_direct_mode", "pcie1_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("cpu_target_memory", "cpu_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("cpu_direct_mode", "cpu_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("mc_target_memory", "mc_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("mc_direct_mode", "mc_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("ddr_target_memory", "ddr_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("ddr_direct_mode", "ddr_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("sram_target_memory", "sram_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("sram_direct_mode", "sram_direct_mode", bdmfmon_enum_bool_table, 0),
            BDMFMON_MAKE_PARM_ENUM("spare_target_memory", "spare_target_memory", target_memory_enum_table, 0),
            BDMFMON_MAKE_PARM_ENUM("spare_direct_mode", "spare_direct_mode", bdmfmon_enum_bool_table, 0));


	BDMFMON_MAKE_CMD(ih_dir, "gtme", "Get Target Matrix entry", p_bl_drv_ih_get_target_matrix_entry_command,
        BDMFMON_MAKE_PARM_ENUM("src_port", "source port", target_matrix_src_port_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("dst_port", "destination port", target_matrix_dst_port_enum_table, 0));

    BDMFMON_MAKE_CMD(ih_dir, "sf", "Set Forward\n"
        "This function sets the 'forward-enable' bit for the given source port\n"
        " and destination port.\n"
        "The 'forward-enable' is only indication to FW\n"
        "(IH doesn't drop if forwarding is disabled).",
        p_bl_drv_ih_set_forward_command,
        BDMFMON_MAKE_PARM_ENUM("src_port", "source port", target_matrix_src_port_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("dst_port", "destination port", target_matrix_dst_port_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("forward", "forward", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD(ih_dir, "gf", "Get Forward", p_bl_drv_ih_get_forward_command,
        BDMFMON_MAKE_PARM_ENUM("src_port", "source port", target_matrix_src_port_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("dst_port", "destination port", target_matrix_dst_port_enum_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "cwp", "Configure WAN ports\n"
	    "This function configures, for each logical port, whether it\n"
	    " belongs to WAN traffic. IH uses this configuration for WAN indication\n"
	    " in the parser result (and Classifier Key Word).",
	    p_bl_drv_ih_configure_wan_ports_command,
        BDMFMON_MAKE_PARM_ENUM("eth0", "eth0", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth1", "eth1", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth2", "eth2", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth3", "eth3", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth4", "eth4", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("gpon", "gpon", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("rnr_a", "runner a", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("rnr_b", "runner b", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("pcie0", "pcie0", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("pcie1", "pcie1", bdmfmon_enum_bool_table, 0));


	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "gwpc", "Get WAN ports configuration", p_bl_drv_ih_get_wan_ports_configuration_command);

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "garbc", "Get allocated runner buffers counters", p_bl_drv_ih_get_allocated_runner_buffers_counters_command);

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "gcb", "Get critical bits", p_bl_drv_ih_get_critical_bits_command);

	BDMFMON_MAKE_CMD(ih_dir, "cp1", "Configure Parser\n"
	    "This function configures general parameters in the parser accelerator in IH",
	    p_bl_drv_ih_configure_parser_command,
        BDMFMON_MAKE_PARM_RANGE("tcp_flags", "tcp_flags", BDMFMON_PARM_HEX, 0, 0, 0xff),
        BDMFMON_MAKE_PARM_RANGE("exc_status", "exception_status_bits", BDMFMON_PARM_HEX, 0, 0, 0xf),
        BDMFMON_MAKE_PARM_RANGE("ppp_code_1_ipv6", "ppp_code_1_ipv6", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("ipv6_ext_hdr", "ipv6_extension_header_bitmask", BDMFMON_PARM_HEX, 0, 0, 0x7),
        BDMFMON_MAKE_PARM_RANGE("snap_ud_org_code", "snap_user_defined_organization_code", BDMFMON_PARM_HEX, 0, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("rfc1042_enable", "snap_rfc1042_encapsulation_enable", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("802_1q_enable", "snap_802_1q_encapsulation_enable", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("gre_protocol", "gre_protocol", BDMFMON_PARM_HEX, 0, 0, 0xffff));

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "gpc1", "Get Parser configuration", p_bl_drv_ih_get_parser_configuration_command);

	BDMFMON_MAKE_CMD(ih_dir, "sdfwm", "Set DA Filter with Mask\n"
	    "This function sets a DA filter with mask.\n"
	    "Allowed filter index: 0-1 (only these filters has mask).\n"
	    "The filter should be enabled using Enable DA Filter API in order to take effect.",
	    p_bl_drv_ih_set_da_filter_with_mask_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK - 1),
        BDMFMON_MAKE_PARM("mac_address", "mac_address", BDMFMON_PARM_MAC, 0),
        BDMFMON_MAKE_PARM("mask", "mask", BDMFMON_PARM_MAC, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gdfwm", "Get DA Filter with Mask", p_bl_drv_ih_get_da_filter_with_mask_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK - 1));

	BDMFMON_MAKE_CMD(ih_dir, "sdfwm1", "Set DA Filter without mask\n"
	    "This function sets DA filter without mask.\n"
	    "Allowed filter index: 2-7 (only these filters don't have mask).\n"
	    "The filter should be enabled using Enable Filter API in order to take effect.",
	    p_bl_drv_ih_set_da_filter_without_mask_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK, DRV_IH_NUMBER_OF_DA_FILTERS - 1),
        BDMFMON_MAKE_PARM("mac_address", "mac_address", BDMFMON_PARM_MAC, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gdfwm1", "Get DA Filter without mask", p_bl_drv_ih_get_da_filter_without_mask_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK, DRV_IH_NUMBER_OF_DA_FILTERS - 1));

	BDMFMON_MAKE_CMD(ih_dir, "edf", "Enable DA Filter\n"
	    "This function enables/disables a DA filter.\n"
	    "Before enabling a DA filter, it has to be configured by\n"
	    " Configure DA Filter with Mask API or Configure DA Filter without Mask API.",
	    p_bl_drv_ih_enable_da_filter_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DA_FILTERS - 1),
        BDMFMON_MAKE_PARM_ENUM("enable", "enable", bdmfmon_enum_bool_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gdfes", "Get DA filter enable status", p_bl_drv_ih_get_da_filter_enable_status_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DA_FILTERS - 1));

	BDMFMON_MAKE_CMD(ih_dir, "sefqi", "Set Ethertypes for QTAG identification", p_bl_drv_ih_set_ethertypes_for_qtag_identification_command,
        BDMFMON_MAKE_PARM_RANGE("etype0", "User-defined ethertype_0", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("etype1", "User-defined ethertype_1", BDMFMON_PARM_HEX, 0, 0, 0xffff));

	BDMFMON_MAKE_CMD_NOPARM(ih_dir, "gefqi", "Get Ethertypes for QTAG identification", p_bl_drv_ih_get_ethertypes_for_qtag_identification_command);

	BDMFMON_MAKE_CMD(ih_dir, "cqn", "Configure QTAG Nesting\n"
        "This function configures, for 6 possible Ethertypes,\n"
        "whether each Ethertype can be used for QTAG identification,\n"
        "as inner and as outer tag.\n"
        "Note that when packet has a single tag, parser treats it as outer tag.\n"
        "The first 2 Ethertypes indices are for the user defined Ethertypes\n"
        "configured by Set Ethertypes for QTAG Identification API.",
        p_bl_drv_ih_configure_qtag_nesting_command,
        BDMFMON_MAKE_PARM_ENUM("ethertype_index", "ethertype_index", qtag_nesting_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("use_as_outer", "use_as_outer", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("use_as_inner", "use_as_inner", bdmfmon_enum_bool_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gqnc", "Get QTAG Nesting configuration", p_bl_drv_ih_get_qtag_nesting_configuration_command,
        BDMFMON_MAKE_PARM_ENUM("ethertype_index", "ethertype_index", qtag_nesting_enum_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "cue", "Configure User Ethertype\n"
	    "This function configures user defined Ethertype.\n"
	    "There are up to 4 user Ethertypes.\n"
	    "For such an Ethertype, the API configures which L3 protocol comes after it,\n"
	    " and its offset (for L3 parsing).\n"
	    "In order to take effect, the user Ethertype should be enabled\n"
	    " using Enable User Ethertype\nAPI.",
	    p_bl_drv_ih_configure_user_ethertype_command,
	    BDMFMON_MAKE_PARM_RANGE("index", "ethertype_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES - 1),
        BDMFMON_MAKE_PARM_RANGE("etype", "User-defined ethertype value", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_ENUM("l3_prot", "L3 protocol", l3_protocol_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("l3_offset", "l3_offset", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_L3_OFFSET));

    BDMFMON_MAKE_CMD(ih_dir, "guec", "Get User Ethertype configuration", p_bl_drv_ih_get_user_ethertype_configuration_command,
        BDMFMON_MAKE_PARM_RANGE("index", "ethertype_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES - 1));

	BDMFMON_MAKE_CMD(ih_dir, "eue", "Enable User Ethertype\n"
	    "This function enables/disables a user-defined Ethertype\n"
	    "which was configured by Configure User Ethertype API.",
	    p_bl_drv_ih_enable_user_ethertype_command,
        BDMFMON_MAKE_PARM_RANGE("index", "ethertype_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES - 1),
        BDMFMON_MAKE_PARM_ENUM("enable", "enable", bdmfmon_enum_bool_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "guees", "Get User Ethertype enable status", p_bl_drv_ih_get_user_ethertype_enable_status_command,
        BDMFMON_MAKE_PARM_RANGE("index", "ethertype_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES - 1));

	BDMFMON_MAKE_CMD(ih_dir, "suilp", "Set user IP L4 protocol\n"
	    "This function sets a user-defined L4 Protocol ID to be matched to\n"
	    "Protocol field in IP header and to be indicated in the output summary word.\n"
	    "There are up to 4 user-defined L4 protocols.",
	    p_bl_drv_ih_set_user_ip_l4_protocol_command,
        BDMFMON_MAKE_PARM_RANGE("index", "l4_protocol_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_USER_DEFINED_IP_L4_PROTOCOLS - 1),
        BDMFMON_MAKE_PARM_RANGE("value", "l4_protocol_value", BDMFMON_PARM_HEX, 0, 0, 0xffff));

	BDMFMON_MAKE_CMD(ih_dir, "guilp", "Get user IP L4 protocol", p_bl_drv_ih_get_user_ip_l4_protocol_command,
        BDMFMON_MAKE_PARM_RANGE("index", "l4_protocol_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_USER_DEFINED_IP_L4_PROTOCOLS - 1));

	BDMFMON_MAKE_CMD(ih_dir, "spc", "Set PPP code\n"
	    "This function sets PPP Protocol Code to indicate L3 is IP.",
	    p_bl_drv_ih_set_ppp_code_command,
        BDMFMON_MAKE_PARM_RANGE("index", "ppp_code_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_PPP_PROTOCOL_CODES - 1),
        BDMFMON_MAKE_PARM_RANGE("code", "ppp_code", BDMFMON_PARM_HEX, 0, 0, 0xffff));

	BDMFMON_MAKE_CMD(ih_dir, "gpc", "Get PPP code", p_bl_drv_ih_get_ppp_code_command,
        BDMFMON_MAKE_PARM_RANGE("index", "ppp_code_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_PPP_PROTOCOL_CODES - 1));

	BDMFMON_MAKE_CMD(ih_dir, "svf", "Set VID filter\n"
	    "This function sets a VID filter. There are up to 12 VID filters.\n"
	    "The filter has to be enabled by Enable VID filter API in order to take effect.",
	    p_bl_drv_ih_set_vid_filter_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_VID_FILTERS - 1),
        BDMFMON_MAKE_PARM_RANGE("vid", "vid", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_VID_VALUE));

	BDMFMON_MAKE_CMD(ih_dir, "gvf", "Get VID filter", p_bl_drv_ih_get_vid_filter_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_VID_FILTERS - 1));

	BDMFMON_MAKE_CMD(ih_dir, "evf", "Enable VID filter\n"
	    "This function enables/disables a VID filter.\n"
	    "There are up to 12 VID filters.\n"
	    "Before enabling a filter, it should be configured by Set VID filter API.",
	    p_bl_drv_ih_enable_vid_filter_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_VID_FILTERS - 1),
        BDMFMON_MAKE_PARM_ENUM("enable", "enable", bdmfmon_enum_bool_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gvfes", "Get VID filter enable status", p_bl_drv_ih_get_vid_filter_enable_status_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_VID_FILTERS - 1));

	BDMFMON_MAKE_CMD(ih_dir, "sif", "Set IP filter\n"
	    "This function sets an IP filter. There are up to 4 IP filters.\n"
	    "The filter has to be enabled by Enable IP filter API in order to take effect.\n",
	    p_bl_drv_ih_set_ip_filter_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_IP_FILTERS - 1),
        BDMFMON_MAKE_PARM("ip_address", "ip_address", BDMFMON_PARM_IP, 0),
        BDMFMON_MAKE_PARM("ip_address_mask", "ip_address_mask", BDMFMON_PARM_IP, 0),
        BDMFMON_MAKE_PARM_ENUM("selection", "selection", ip_filter_enum_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gif", "Get IP filter", p_bl_drv_ih_get_ip_filter_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_IP_FILTERS - 1));

	BDMFMON_MAKE_CMD(ih_dir, "eif", "Enable IP filter\n"
	    "This function enables/disables an IP filter.\n"
	    "There are up to 4 IP filters.\n"
	    "Before enabling a filter, it should be configured by Set IP filter API.",
	    p_bl_drv_ih_enable_ip_filter_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_IP_FILTERS - 1),
        BDMFMON_MAKE_PARM_ENUM("enable", "enable", bdmfmon_enum_bool_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gifes", "Get IP filter enable status", p_bl_drv_ih_get_ip_filter_enable_status_command,
        BDMFMON_MAKE_PARM_RANGE("index", "filter_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_IP_FILTERS - 1));

	BDMFMON_MAKE_CMD(ih_dir, "sdttte", "Set DSCP to TCI table entry\n"
	    "This function sets an entry in a DSCP to TCI table.\n"
	    "There are 2 such tables.\n"
	    "Each class is configured with one of these tables.\n"
	    "The table has to be enabled by Enable DSCP to TCI table API\n"
	    " in order to take effect.\n"
	    "If a class is configured with a table which is not enabled, the TCI will be 0.",
	    p_bl_drv_ih_set_dscp_to_tci_table_entry_command,
	    BDMFMON_MAKE_PARM_RANGE("index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES - 1),
	    BDMFMON_MAKE_PARM_RANGE("dscp", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_DSCP_VALUE),
	    BDMFMON_MAKE_PARM_RANGE("tci", "tci", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_TCI_VALUE));

	BDMFMON_MAKE_CMD(ih_dir, "gdttte", "Get DSCP to TCI table entry", p_bl_drv_ih_get_dscp_to_tci_table_entry_command,
        BDMFMON_MAKE_PARM_RANGE("index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES - 1),
        BDMFMON_MAKE_PARM_RANGE("dscp", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_DSCP_VALUE));

	BDMFMON_MAKE_CMD(ih_dir, "sdt", "Set default TCI\n"
	    "This function sets default TCI, per DSCP to TCI table.\n"
	    "The default is used in case of non-IP untagged packet.\n"
	    "The default TCI will take effect only after enabling the DSCP to TCI table,\n"
	    " by Enable DSCP to TCI table API.\n"
	    "If a class is configured with a table which is not enabled, the TCI will be 0.",
	    p_bl_drv_ih_set_default_tci_command,
        BDMFMON_MAKE_PARM_RANGE("index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES - 1),
        BDMFMON_MAKE_PARM_RANGE("tci", "default_tci", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_MAXIMAL_TCI_VALUE));

	BDMFMON_MAKE_CMD(ih_dir, "gdt", "Get default TCI", p_bl_drv_ih_get_default_tci_command,
        BDMFMON_MAKE_PARM_RANGE("index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES - 1));

	BDMFMON_MAKE_CMD(ih_dir, "edttt", "Enable DSCP to TCI table\n"
	    "This function enables/disables a DSCP to TCI table.\n"
	    "There are 2 such tables. Each class is configured with one of these tables.\n"
	    "Before enabling a table, it should be configured by\n"
	    "Set DSCP to TCI table API and Set default TCI API.",
	    p_bl_drv_ih_enable_dscp_to_tci_table_command,
        BDMFMON_MAKE_PARM_RANGE("index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES - 1),
        BDMFMON_MAKE_PARM_ENUM("enable", "enable", bdmfmon_enum_bool_table, 0));

	BDMFMON_MAKE_CMD(ih_dir, "gdtttes", "Get DSCP to TCI table enable status", p_bl_drv_ih_get_dscp_to_tci_table_enable_status_command,
        BDMFMON_MAKE_PARM_RANGE("index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES - 1));

    /* more advanced commands: */
    BDMFMON_MAKE_CMD(ih_dir, "glftes", "Get LUT Five Tuple Enable Status", p_bl_drv_ih_get_lut_five_tuple_enable_status_command,
        BDMFMON_MAKE_PARM_RANGE("index", "table_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_IH_NUMBER_OF_LOOKUP_TABLES - 1));

    BDMFMON_MAKE_CMD_NOPARM(ih_dir, "dacc", "Dump All Configured Classes", p_bl_drv_ih_dump_all_configured_classes_command);
    BDMFMON_MAKE_CMD_NOPARM(ih_dir, "dacc1", "Dump All Configured Classifiers", p_bl_drv_ih_dump_all_configured_classifiers_command) ;
}

void pi_bl_exit_drv_ih_shell ( void )
{
    if (ih_dir)
    {
        bdmfmon_token_destroy(ih_dir);
        ih_dir = NULL;
    }
}

char * f_drv_ih_error_code_to_string ( DRV_IH_ERROR xi_error_code )
{
    switch ( xi_error_code )
    {
    case DRV_IH_NO_ERROR:
        return "DRV_IH_NO_ERROR" ;
        break ;
    case DRV_IH_ERROR_INVALID_INDEX:
        return "DRV_IH_ERROR_INVALID_INDEX" ;
        break ;
    case DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS:
        return "DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS" ;
        break ;
    case DRV_IH_ERROR_INVALID_INGRESS_QUEUE:
        return "DRV_IH_ERROR_INVALID_INGRESS_QUEUE" ;
        break ;
    case DRV_IH_ERROR_INVALID_INGRESS_QUEUE_BASE_LOCATION:
        return "DRV_IH_ERROR_INVALID_INGRESS_QUEUE_BASE_LOCATION" ;
        break ;
    case DRV_IH_ERROR_INVALID_INGRESS_QUEUE_SIZE:
        return "DRV_IH_ERROR_INVALID_INGRESS_QUEUE_SIZE" ;
        break ;
    case DRV_IH_ERROR_INVALID_INGRESS_QUEUE_PRIORITY:
        return "DRV_IH_ERROR_INVALID_INGRESS_QUEUE_PRIORITY" ;
        break ;
    case DRV_IH_ERROR_INVALID_INGRESS_QUEUE_WEIGHT:
        return "DRV_IH_ERROR_INVALID_INGRESS_QUEUE_WEIGHT" ;
        break ;
    case DRV_IH_ERROR_DESTINATION_PORT_AND_TARGET_MEMORY_MISMATCH:
        return "DRV_IH_ERROR_DESTINATION_PORT_AND_TARGET_MEMORY_MISMATCH" ;
        break ;
    case DRV_IH_ERROR_INVALID_L3_OFFSET:
        return "DRV_IH_ERROR_INVALID_L3_OFFSET" ;
        break ;
    case DRV_IH_ERROR_INVALID_PORT:
        return "DRV_IH_ERROR_INVALID_PORT" ;
        break ;
    case DRV_IH_ERROR_INVALID_VID:
        return "DRV_IH_ERROR_INVALID_VID" ;
        break ;
    case DRV_IH_ERROR_INVALID_DSCP:
        return "DRV_IH_ERROR_INVALID_DSCP" ;
        break ;
    case DRV_IH_ERROR_INVALID_TCI:
        return "DRV_IH_ERROR_INVALID_TCI" ;
        break ;
    case DRV_IH_ERROR_INVALID_TABLE_SIZE:
        return "DRV_IH_ERROR_INVALID_TABLE_SIZE" ;
        break ;
    case DRV_IH_ERROR_INVALID_MAXIMAL_SEARCH_DEPTH:
        return "DRV_IH_ERROR_INVALID_MAXIMAL_SEARCH_DEPTH" ;
        break ;
    case DRV_IH_ERROR_INVALID_START_OFFSET_SEATCH_KEY_PART:
        return "DRV_IH_ERROR_INVALID_START_OFFSET_SEATCH_KEY_PART" ;
        break ;
    case DRV_IH_ERROR_INVALID_SHIFT_OFFSET_SEATCH_KEY_PART:
        return "DRV_IH_ERROR_INVALID_SHIFT_OFFSET_SEATCH_KEY_PART" ;
        break ;
    case DRV_IH_ERROR_TABLE_IS_NOT_60_BIT_KEY:
        return "DRV_IH_ERROR_TABLE_IS_NOT_60_BIT_KEY" ;
        break ;
    case DRV_IH_ERROR_TABLE_IS_NOT_120_BIT_KEY:
        return "DRV_IH_ERROR_TABLE_IS_NOT_120_BIT_KEY" ;
        break ;
    case DRV_IH_ERROR_CLASS_SEARCH_AND_LUT_LOCATION_MISMATCH:
        return "DRV_IH_ERROR_CLASS_SEARCH_AND_LUT_LOCATION_MISMATCH" ;
        break ;
    case DRV_IH_ERROR_VALUE_IS_WRITE_ONLY:
        return "DRV_IH_ERROR_VALUE_IS_WRITE_ONLY" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static void p_dump_class_configuration ( bdmf_session_handle session, const DRV_IH_CLASS_CONFIG * xi_class_config )
{
    bdmf_session_print(session, "class_search_1: %s (%lu)\n\r" , bdmfmon_enum_stringval(class_search_enum_table, xi_class_config->class_search_1 ) , xi_class_config->class_search_1 ) ;
    bdmf_session_print(session, "class_search_2: %s (%lu)\n\r" , bdmfmon_enum_stringval(class_search_enum_table, xi_class_config->class_search_2 ) , xi_class_config->class_search_2 ) ;
    bdmf_session_print(session, "class_search_3: %s (%lu)\n\r" , bdmfmon_enum_stringval(class_search_enum_table, xi_class_config->class_search_3 ) , xi_class_config->class_search_3 ) ;
    bdmf_session_print(session, "class_search_4: %s (%lu)\n\r" , bdmfmon_enum_stringval(class_search_enum_table, xi_class_config->class_search_4 ) , xi_class_config->class_search_4 ) ;
    bdmf_session_print(session, "destination_port_extraction: %s (%lu)\n\r" , bdmfmon_enum_stringval(op_based_on_class_search_enum_table, xi_class_config->destination_port_extraction ) , xi_class_config->destination_port_extraction ) ;
    bdmf_session_print(session, "drop_on_miss: %s (%lu)\n\r" , bdmfmon_enum_stringval(op_based_on_class_search_enum_table, xi_class_config->drop_on_miss ) , xi_class_config->drop_on_miss ) ;
    bdmf_session_print(session, "dscp_to_tci_table_index: %lu\n\r" , xi_class_config->dscp_to_tci_table_index ) ;
    bdmf_session_print(session, "direct_mode_default: %lu\n\r" , xi_class_config->direct_mode_default ) ;
    bdmf_session_print(session, "direct_mode_override: %lu\n\r" , xi_class_config->direct_mode_override ) ;
    bdmf_session_print(session, "target_memory_default: %s (%lu)\n\r" , bdmfmon_enum_stringval(target_memory_enum_table, xi_class_config->target_memory_default ) , xi_class_config->target_memory_default ) ;
    bdmf_session_print(session, "target_memory_override: %lu\n\r" , xi_class_config->target_memory_override ) ;
    bdmf_session_print(session, "ingress_qos_default: %s (%lu)\n\r" , bdmfmon_enum_stringval(ingress_qos_enum_table, xi_class_config->ingress_qos_default ) , xi_class_config->ingress_qos_default ) ;
    bdmf_session_print(session, "ingress_qos_override: %s (%lu)\n\r" , bdmfmon_enum_stringval(op_based_on_class_search_enum_table, xi_class_config->ingress_qos_override ) , xi_class_config->ingress_qos_override ) ;
    bdmf_session_print(session, "target_runner_default: %s (%lu)\n\r" , bdmfmon_enum_stringval(runner_cluster_enum_table, xi_class_config->target_runner_default ) , xi_class_config->target_runner_default ) ;
    bdmf_session_print(session, "target_runner_override_in_direct_mode: %lu\n\r" , xi_class_config->target_runner_override_in_direct_mode ) ;
    bdmf_session_print(session, "target_runner_for_direct_mode: %s (%lu)\n\r" , bdmfmon_enum_stringval(runner_cluster_enum_table, xi_class_config->target_runner_for_direct_mode ) , xi_class_config->target_runner_for_direct_mode ) ;
    bdmf_session_print(session, "load_balancing_enable: %lu\n\r" , xi_class_config->load_balancing_enable ) ;
    bdmf_session_print(session, "preference_load_balancing_enable: %lu\n\r" , xi_class_config->preference_load_balancing_enable ) ;
}


static void p_dump_classifier_configuration ( bdmf_session_handle session, const DRV_IH_CLASSIFIER_CONFIG * xi_classifier_config )
{
    bdmf_session_print(session, "l2_protocol: %s (%lu)\n\r" , bdmfmon_enum_stringval(l2_protocol_enum_table, xi_classifier_config->l2_protocol ) , xi_classifier_config->l2_protocol ) ;
    bdmf_session_print(session, "l3_protocol: %s (%lu)\n\r" , bdmfmon_enum_stringval(l3_protocol_enum_table, xi_classifier_config->l3_protocol ) , xi_classifier_config->l3_protocol ) ;
    bdmf_session_print(session, "l4_protocol: %s (%lu)\n\r" , bdmfmon_enum_stringval(l4_protocol_enum_table, xi_classifier_config->l4_protocol ) , xi_classifier_config->l4_protocol ) ;
    bdmf_session_print(session, "da_filter_any_hit: %lu\n\r" , xi_classifier_config->da_filter_any_hit ) ;
    bdmf_session_print(session, "matched_da_filter: %lu\n\r" , xi_classifier_config->matched_da_filter ) ;
    bdmf_session_print(session, "multicast_da_indication: %lu\n\r" , xi_classifier_config->multicast_da_indication ) ;
    bdmf_session_print(session, "broadcast_da_indication: %lu\n\r" , xi_classifier_config->broadcast_da_indication ) ;
    bdmf_session_print(session, "vid_filter_any_hit: %lu\n\r" , xi_classifier_config->vid_filter_any_hit ) ;
    bdmf_session_print(session, "matched_vid_filter: %lu\n\r" , xi_classifier_config->matched_vid_filter ) ;
    bdmf_session_print(session, "ip_filter_any_hit: %lu\n\r" , xi_classifier_config->ip_filter_any_hit ) ;
    bdmf_session_print(session, "matched_ip_filter: %lu\n\r" , xi_classifier_config->matched_ip_filter ) ;
    bdmf_session_print(session, "wan_indication: %lu\n\r" , xi_classifier_config->wan_indication ) ;
    bdmf_session_print(session, "five_tuple_valid: %lu\n\r" , xi_classifier_config->five_tuple_valid ) ;
    bdmf_session_print(session, "logical_source_port: %s (%lu)\n\r" , bdmfmon_enum_stringval(logical_port_enum_table, xi_classifier_config->logical_source_port ) , xi_classifier_config->logical_source_port ) ;
    bdmf_session_print(session, "error: %lu\n\r" , xi_classifier_config->error ) ;
    bdmf_session_print(session, "mask: 0x%lX\n\r" , xi_classifier_config->mask ) ;
    bdmf_session_print(session, "resulting_class: %lu\n\r" , xi_classifier_config->resulting_class ) ;
}


#endif
