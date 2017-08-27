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
/* BBH driver.                                                                */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#define BDMF_SESSION_DISABLE_FORMAT_CHECK
#include "rdp_drv_bbh.h"
#include "bdmf_shell.h"

#ifdef USE_BDMF_SHELL
/*****************************************************************************/
/*                                                                           */
/* Automatically generated unit test code - 3/3/2010   11:09:36              */
/*                                                                           */
/*****************************************************************************/

char * f_drv_bbh_error_code_to_string ( DRV_BBH_ERROR xi_error_code ) ;

static void p_print_tx_internal_unit_enum_help(bdmf_session_handle session);
static void p_print_rx_internal_unit_enum_help(bdmf_session_handle session);
static void p_print_rx_flow_control_trigger_enum_help(bdmf_session_handle session);
static void p_print_rx_drop_trigger_enum_help(bdmf_session_handle session);

static char * f_bbh_port_index_enum_to_string ( DRV_BBH_PORT_INDEX xi_enum_value ) ;
static char * f_ddr_buffer_size_enum_to_string ( DRV_BBH_PORT_INDEX xi_enum_value ) ;
static char * f_payload_offset_resolution_enum_to_string ( DRV_BBH_PORT_INDEX xi_enum_value ) ;
static char * f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT xi_enum_value ) ;
static char * f_rx_flow_control_trigger_enum_to_string ( DRV_BBH_RX_FLOW_CONTROL_TRIGGER xi_enum_value ) ;
static char * f_rx_drop_trigger_enum_to_string ( DRV_BBH_PORT_INDEX xi_enum_value ) ;
static char * f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT xi_enum_value ) ;


static int p_bl_drv_bbh_tx_set_misc_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_TX_CONFIGURATION tx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_tx_get_configuration ( port_index ,
                                                       & tx_configuration ) ;
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    tx_configuration.ddr_buffer_size = parm[parameter_index++].value.unumber;
    tx_configuration.payload_offset_resolution = parm[parameter_index++].value.unumber;
    tx_configuration.multicast_header_size = parm[parameter_index++].value.unumber;
    tx_configuration.ddr1_multicast_headers_base_address_in_byte = parm[parameter_index++].value.unumber;
    tx_configuration.dma_read_requests_fifo_base_address = parm[parameter_index++].value.unumber;
    tx_configuration.dma_read_requests_maximal_number = parm[parameter_index++].value.unumber;
    tx_configuration.sdma_read_requests_fifo_base_address = parm[parameter_index++].value.unumber;
    tx_configuration.sdma_read_requests_maximal_number = parm[parameter_index++].value.unumber;
    tx_configuration.tcont_address_in_8_byte = parm[parameter_index++].value.unumber;
    tx_configuration.skb_address = parm[parameter_index++].value.unumber;
    tx_configuration.mdu_mode_enable = parm[parameter_index++].value.unumber;
    tx_configuration.mdu_mode_read_pointer_address_in_8_byte = parm[parameter_index++].value.unumber;
    tx_configuration.ddr1_tm_base_address = parm[parameter_index++].value.unumber;
    tx_configuration.emac_1588_enable = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_tx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_tx_set_configuration ( port_index ,
                                                       & tx_configuration ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_bbh_tx_set_route_addresses_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_TX_CONFIGURATION tx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_tx_get_configuration(port_index, &tx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    tx_configuration.dma_route_address = parm[parameter_index++].value.unumber;
    tx_configuration.bpm_route_address = parm[parameter_index++].value.unumber;
    tx_configuration.sdma_route_address = parm[parameter_index++].value.unumber;
    tx_configuration.sbpm_route_address = parm[parameter_index++].value.unumber;
    tx_configuration.runner_route_address = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_tx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_tx_set_configuration(port_index , &tx_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_tx_set_runner_tasks_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_TX_CONFIGURATION tx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_tx_get_configuration(port_index, &tx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    tx_configuration.task_0 = parm[parameter_index++].value.unumber;
    tx_configuration.task_1 = parm[parameter_index++].value.unumber;
    tx_configuration.task_2 = parm[parameter_index++].value.unumber;
    tx_configuration.task_3 = parm[parameter_index++].value.unumber;
    tx_configuration.task_4 = parm[parameter_index++].value.unumber;
    tx_configuration.task_5 = parm[parameter_index++].value.unumber;
    tx_configuration.task_6 = parm[parameter_index++].value.unumber;
    tx_configuration.task_7 = parm[parameter_index++].value.unumber;
    tx_configuration.task_8_39 = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_tx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_tx_set_configuration(port_index, &tx_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_tx_set_pd_fifos_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_TX_CONFIGURATION tx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_tx_get_configuration(port_index, &tx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    tx_configuration.pd_fifo_size_0 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_1 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_2 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_3 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_4 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_5 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_6 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_7 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_8_15 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_16_23 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_24_31 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_size_32_39 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_0 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_1 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_2 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_3 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_4 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_5 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_6 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_7 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_8_15 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_16_23 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_24_31 = parm[parameter_index++].value.unumber;
    tx_configuration.pd_fifo_base_32_39 = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_tx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_tx_set_configuration(port_index, &tx_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_tx_set_pd_prefetch_byte_thresholds_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_TX_CONFIGURATION tx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_tx_get_configuration(port_index, &tx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    tx_configuration.pd_prefetch_byte_threshold_enable = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_0_in_32_byte = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_1_in_32_byte = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_2_in_32_byte = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_3_in_32_byte = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_4_in_32_byte = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_5_in_32_byte = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_6_in_32_byte = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_7_in_32_byte = parm[parameter_index++].value.unumber;
    tx_configuration.pd_prefetch_byte_threshold_8_39_in_32_byte = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_tx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_tx_set_configuration(port_index, &tx_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_tx_get_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_TX_CONFIGURATION tx_configuration ; /* output */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;


    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_tx_get_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_tx_get_configuration(port_index, &tx_configuration);
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "dma_route_address: 0x%lX\n\r" , tx_configuration.dma_route_address ) ;
        bdmf_session_print(session, "bpm_route_address: 0x%lX\n\r" , tx_configuration.bpm_route_address ) ;
        bdmf_session_print(session, "sdma_route_address: 0x%lX\n\r" , tx_configuration.sdma_route_address ) ;
        bdmf_session_print(session, "sbpm_route_address: 0x%lX\n\r" , tx_configuration.sbpm_route_address ) ;
        bdmf_session_print(session, "runner_route_address: 0x%lX\n\r" , tx_configuration.runner_route_address ) ;
        bdmf_session_print(session, "ddr_buffer_size: %s (%lu)\n\r" , f_ddr_buffer_size_enum_to_string ( tx_configuration.ddr_buffer_size ) , tx_configuration.ddr_buffer_size ) ;
        bdmf_session_print(session, "payload_offset_resolution: %s (%lu)\n\r" , f_payload_offset_resolution_enum_to_string ( tx_configuration.payload_offset_resolution ) , tx_configuration.payload_offset_resolution ) ;
        bdmf_session_print(session, "multicast_header_size: %lu\n\r" , tx_configuration.multicast_header_size ) ;
        bdmf_session_print(session, "ddr1_multicast_headers_base_address_in_byte: 0x%lX\n\r" , tx_configuration.ddr1_multicast_headers_base_address_in_byte ) ;
        bdmf_session_print(session, "ddr2_multicast_headers_base_address_in_byte: 0x%lX\n\r" , tx_configuration.ddr2_multicast_headers_base_address_in_byte ) ;
        bdmf_session_print(session, "task_0: %lu\n\r" , tx_configuration.task_0 ) ;
        bdmf_session_print(session, "task_1: %lu\n\r" , tx_configuration.task_1 ) ;
        bdmf_session_print(session, "task_2: %lu\n\r" , tx_configuration.task_2 ) ;
        bdmf_session_print(session, "task_3: %lu\n\r" , tx_configuration.task_3 ) ;
        bdmf_session_print(session, "task_4: %lu\n\r" , tx_configuration.task_4 ) ;
        bdmf_session_print(session, "task_5: %lu\n\r" , tx_configuration.task_5 ) ;
        bdmf_session_print(session, "task_6: %lu\n\r" , tx_configuration.task_6 ) ;
        bdmf_session_print(session, "task_7: %lu\n\r" , tx_configuration.task_7 ) ;
        bdmf_session_print(session, "task_8_39: %lu\n\r" , tx_configuration.task_8_39 ) ;
        bdmf_session_print(session, "pd_fifo_size_0: %lu\n\r" , tx_configuration.pd_fifo_size_0 ) ;
        bdmf_session_print(session, "pd_fifo_size_1: %lu\n\r" , tx_configuration.pd_fifo_size_1 ) ;
        bdmf_session_print(session, "pd_fifo_size_2: %lu\n\r" , tx_configuration.pd_fifo_size_2 ) ;
        bdmf_session_print(session, "pd_fifo_size_3: %lu\n\r" , tx_configuration.pd_fifo_size_3 ) ;
        bdmf_session_print(session, "pd_fifo_size_4: %lu\n\r" , tx_configuration.pd_fifo_size_4 ) ;
        bdmf_session_print(session, "pd_fifo_size_5: %lu\n\r" , tx_configuration.pd_fifo_size_5 ) ;
        bdmf_session_print(session, "pd_fifo_size_6: %lu\n\r" , tx_configuration.pd_fifo_size_6 ) ;
        bdmf_session_print(session, "pd_fifo_size_7: %lu\n\r" , tx_configuration.pd_fifo_size_7 ) ;
        bdmf_session_print(session, "pd_fifo_size_8_15: %lu\n\r" , tx_configuration.pd_fifo_size_8_15 ) ;
        bdmf_session_print(session, "pd_fifo_size_16_23: %lu\n\r" , tx_configuration.pd_fifo_size_16_23 ) ;
        bdmf_session_print(session, "pd_fifo_size_24_31: %lu\n\r" , tx_configuration.pd_fifo_size_24_31 ) ;
        bdmf_session_print(session, "pd_fifo_size_32_39: %lu\n\r" , tx_configuration.pd_fifo_size_32_39 ) ;
        bdmf_session_print(session, "pd_fifo_base_0: %lu\n\r" , tx_configuration.pd_fifo_base_0 ) ;
        bdmf_session_print(session, "pd_fifo_base_1: %lu\n\r" , tx_configuration.pd_fifo_base_1 ) ;
        bdmf_session_print(session, "pd_fifo_base_2: %lu\n\r" , tx_configuration.pd_fifo_base_2 ) ;
        bdmf_session_print(session, "pd_fifo_base_3: %lu\n\r" , tx_configuration.pd_fifo_base_3 ) ;
        bdmf_session_print(session, "pd_fifo_base_4: %lu\n\r" , tx_configuration.pd_fifo_base_4 ) ;
        bdmf_session_print(session, "pd_fifo_base_5: %lu\n\r" , tx_configuration.pd_fifo_base_5 ) ;
        bdmf_session_print(session, "pd_fifo_base_6: %lu\n\r" , tx_configuration.pd_fifo_base_6 ) ;
        bdmf_session_print(session, "pd_fifo_base_7: %lu\n\r" , tx_configuration.pd_fifo_base_7 ) ;
        bdmf_session_print(session, "pd_fifo_base_8_15: %lu\n\r" , tx_configuration.pd_fifo_base_8_15 ) ;
        bdmf_session_print(session, "pd_fifo_base_16_23: %lu\n\r" , tx_configuration.pd_fifo_base_16_23 ) ;
        bdmf_session_print(session, "pd_fifo_base_24_31: %lu\n\r" , tx_configuration.pd_fifo_base_24_31 ) ;
        bdmf_session_print(session, "pd_fifo_base_32_39: %lu\n\r" , tx_configuration.pd_fifo_base_32_39 ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_enable: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_enable ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_0_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_0_in_32_byte ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_1_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_1_in_32_byte ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_2_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_2_in_32_byte ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_3_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_3_in_32_byte ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_4_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_4_in_32_byte ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_5_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_5_in_32_byte ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_6_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_6_in_32_byte ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_7_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_7_in_32_byte ) ;
        bdmf_session_print(session, "pd_prefetch_byte_threshold_8_39_in_32_byte: %lu\n\r" , tx_configuration.pd_prefetch_byte_threshold_8_39_in_32_byte ) ;
        bdmf_session_print(session, "dma_read_requests_fifo_base_address: 0x%lX\n\r" , tx_configuration.dma_read_requests_fifo_base_address ) ;
        bdmf_session_print(session, "dma_read_requests_maximal_number: %lu\n\r" , tx_configuration.dma_read_requests_maximal_number ) ;
        bdmf_session_print(session, "sdma_read_requests_fifo_base_address: 0x%lX\n\r" , tx_configuration.sdma_read_requests_fifo_base_address ) ;
        bdmf_session_print(session, "sdma_read_requests_maximal_number: %lu\n\r" , tx_configuration.sdma_read_requests_maximal_number ) ;
        bdmf_session_print(session, "tcont_address_in_8_byte: 0x%lX\n\r" , tx_configuration.tcont_address_in_8_byte ) ;
        bdmf_session_print(session, "skb_address: 0x%lX\n\r" , tx_configuration.skb_address ) ;
        bdmf_session_print(session, "mdu_mode_enable: %lu\n\r" , tx_configuration.mdu_mode_enable ) ;
        bdmf_session_print(session, "mdu_mode_read_pointer_address_in_8_byte: 0x%lX\n\r" , tx_configuration.mdu_mode_read_pointer_address_in_8_byte ) ;
        bdmf_session_print(session, "ddr1_tm_base_address: 0x%lX\n\r" , tx_configuration.ddr1_tm_base_address ) ;
        bdmf_session_print(session, "ddr2_tm_base_address: 0x%lX\n\r" , tx_configuration.ddr2_tm_base_address ) ;
        bdmf_session_print(session, "MISC BYOI no_fpm_release=%s\n\r",  tx_configuration.byoi_no_fpm_release ? "yes":"no");
        bdmf_session_print(session, "MISC BYOI direct=%s\n\r",  tx_configuration.byoi_direct ? "yes":"no");
        bdmf_session_print(session, "MISC Buff size select(DRV_BBH_FPM_BUFF_SIZE)=%d\n\r",  tx_configuration.fpm_buff_size_set);
        bdmf_session_print(session, "emac_1588_enable: %lu\n\r" , tx_configuration.emac_1588_enable ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_tx_reset_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    uint16_t units_to_reset_bitmask ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;
    units_to_reset_bitmask = parm[parameter_index++].value.unumber;

    if (!units_to_reset_bitmask)
    {
        p_print_tx_internal_unit_enum_help(session);
        return 0;
    }

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_tx_reset'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_tx_reset(port_index, units_to_reset_bitmask);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_tx_get_counters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_TX_COUNTERS tx_counters ; /* output */
    DRV_BBH_ERROR error ;

    /* Get the parameters */
    port_index = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_tx_get_counters'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_tx_get_counters(port_index, &tx_counters);
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "BBH TX counters for port %d (%s):\n\r" , port_index , f_bbh_port_index_enum_to_string ( port_index ) ) ;
        bdmf_session_print(session, "=====================================\n\r" ) ;

        bdmf_session_print(session, "pd_with_zero_packet_length: %lu\n\r" , tx_counters.pd_with_zero_packet_length ) ;
        bdmf_session_print(session, "tx_packets_from_ddr: %lu\n\r" , tx_counters.tx_packets_from_ddr ) ;

        bdmf_session_print(session, "tx_packets_from_sram: %lu\n\r" , tx_counters.tx_packets_from_sram ) ;
        bdmf_session_print(session, "dropped_pd: %lu\n\r" , tx_counters.dropped_pd ) ;
        bdmf_session_print(session, "get_next_null: %lu\n\r" , tx_counters.get_next_null ) ;


        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_set_misc_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_CONFIGURATION rx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;


    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_rx_get_configuration(port_index, &rx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    rx_configuration.ddr_buffer_size = parm[parameter_index++].value.unumber;
    rx_configuration.ddr1_tm_base_address = parm[parameter_index++].value.unumber;
    rx_configuration.ih_ingress_buffers_bitmask = parm[parameter_index++].value.unumber;
    rx_configuration.packet_header_offset = parm[parameter_index++].value.unumber;
    rx_configuration.flows_32_255_group_divider = parm[parameter_index++].value.unumber;
    rx_configuration.ploam_default_ih_class = parm[parameter_index++].value.unumber;
    rx_configuration.ploam_ih_class_override = parm[parameter_index++].value.unumber;
    rx_configuration.reassembly_offset_in_8_byte = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_set_configuration(port_index, &rx_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_bbh_rx_set_route_addresses_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_CONFIGURATION rx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;


    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_rx_get_configuration(port_index, &rx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    rx_configuration.dma_route_address = parm[parameter_index++].value.unumber;
    rx_configuration.bpm_route_address = parm[parameter_index++].value.unumber;
    rx_configuration.sdma_route_address = parm[parameter_index++].value.unumber;
    rx_configuration.sbpm_route_address = parm[parameter_index++].value.unumber;
    rx_configuration.runner_0_route_address = parm[parameter_index++].value.unumber;
    rx_configuration.runner_1_route_address = parm[parameter_index++].value.unumber;
    rx_configuration.ih_route_address = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_set_configuration(port_index, &rx_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_set_pd_fifos_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_CONFIGURATION rx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;


    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_rx_get_configuration(port_index, &rx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    rx_configuration.pd_fifo_base_address_normal_queue_in_8_byte = parm[parameter_index++].value.unumber;
    rx_configuration.pd_fifo_base_address_direct_queue_in_8_byte = parm[parameter_index++].value.unumber;
    rx_configuration.pd_fifo_size_normal_queue = parm[parameter_index++].value.unumber;
    rx_configuration.pd_fifo_size_direct_queue = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_set_configuration(port_index, &rx_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_set_runner_tasks_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_CONFIGURATION rx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;


    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_rx_get_configuration(port_index, &rx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    rx_configuration.runner_0_task_normal_queue = parm[parameter_index++].value.unumber;
    rx_configuration.runner_0_task_direct_queue = parm[parameter_index++].value.unumber;
    rx_configuration.runner_1_task_normal_queue = parm[parameter_index++].value.unumber;
    rx_configuration.runner_1_task_direct_queue = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_set_configuration(port_index, &rx_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_set_dma_and_sdma_fifos_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_CONFIGURATION rx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_rx_get_configuration(port_index, &rx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    rx_configuration.dma_data_fifo_base_address = parm[parameter_index++].value.unumber;
    rx_configuration.dma_chunk_descriptor_fifo_base_address = parm[parameter_index++].value.unumber;
    rx_configuration.dma_data_and_chunk_descriptor_fifos_size = parm[parameter_index++].value.unumber;
    rx_configuration.dma_exclusive_threshold = parm[parameter_index++].value.unumber;
    rx_configuration.sdma_data_fifo_base_address = parm[parameter_index++].value.unumber;
    rx_configuration.sdma_chunk_descriptor_fifo_base_address = parm[parameter_index++].value.unumber;
    rx_configuration.sdma_data_and_chunk_descriptor_fifos_size = parm[parameter_index++].value.unumber;
    rx_configuration.sdma_exclusive_threshold = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_set_configuration ( port_index ,
                                                       & rx_configuration ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_set_minimum_and_maximum_packet_size_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_CONFIGURATION rx_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;

    /* get all configuration */
    error = fi_bl_drv_bbh_rx_get_configuration(port_index, &rx_configuration);
    if ( error != DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "UT: Failed to get configuration. Error code: '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;
        return BDMF_ERR_PARM;
    }

    /* modify the relevant parameters */
    rx_configuration.minimum_packet_size_0 = parm[parameter_index++].value.unumber;
    rx_configuration.minimum_packet_size_1 = parm[parameter_index++].value.unumber;
    rx_configuration.minimum_packet_size_2 = parm[parameter_index++].value.unumber;
    rx_configuration.minimum_packet_size_3 = parm[parameter_index++].value.unumber;
    rx_configuration.maximum_packet_size_0 = parm[parameter_index++].value.unumber;
    rx_configuration.maximum_packet_size_1 = parm[parameter_index++].value.unumber;
    rx_configuration.maximum_packet_size_2 = parm[parameter_index++].value.unumber;
    rx_configuration.maximum_packet_size_3 = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_set_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_set_configuration ( port_index ,
                                                       & rx_configuration ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_get_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_CONFIGURATION rx_configuration ; /* output */
    DRV_BBH_ERROR error ;
    uint32_t ppidx;
    /* Get the parameters */
    port_index = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_get_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_get_configuration(port_index, &rx_configuration);
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "dma_route_address: 0x%lX\n\r" , rx_configuration.dma_route_address ) ;
        bdmf_session_print(session, "bpm_route_address: 0x%lX\n\r" , rx_configuration.bpm_route_address ) ;
        bdmf_session_print(session, "sdma_route_address: 0x%lX\n\r" , rx_configuration.sdma_route_address ) ;
        bdmf_session_print(session, "sbpm_route_address: 0x%lX\n\r" , rx_configuration.sbpm_route_address ) ;
        bdmf_session_print(session, "runner_0_route_address: 0x%lX\n\r" , rx_configuration.runner_0_route_address ) ;
        bdmf_session_print(session, "runner_1_route_address: 0x%lX\n\r" , rx_configuration.runner_1_route_address ) ;
        bdmf_session_print(session, "ih_route_address: 0x%lX\n\r" , rx_configuration.ih_route_address ) ;
        bdmf_session_print(session, "ddr_buffer_size: %s (%lu)\n\r" , f_ddr_buffer_size_enum_to_string ( rx_configuration.ddr_buffer_size ) , rx_configuration.ddr_buffer_size ) ;
        bdmf_session_print(session, "ddr1_tm_base_address: 0x%lX\n\r" , rx_configuration.ddr1_tm_base_address ) ;
        bdmf_session_print(session, "ddr2_tm_base_address: 0x%lX\n\r" , rx_configuration.ddr2_tm_base_address ) ;
        bdmf_session_print(session, "pd_fifo_base_address_normal_queue_in_8_byte: 0x%lX\n\r" , rx_configuration.pd_fifo_base_address_normal_queue_in_8_byte ) ;
        bdmf_session_print(session, "pd_fifo_base_address_direct_queue_in_8_byte: 0x%lX\n\r" , rx_configuration.pd_fifo_base_address_direct_queue_in_8_byte ) ;
        bdmf_session_print(session, "pd_fifo_size_normal_queue: %lu\n\r" , rx_configuration.pd_fifo_size_normal_queue ) ;
        bdmf_session_print(session, "pd_fifo_size_direct_queue: %lu\n\r" , rx_configuration.pd_fifo_size_direct_queue ) ;
        bdmf_session_print(session, "runner_0_task_normal_queue: %lu\n\r" , rx_configuration.runner_0_task_normal_queue ) ;
        bdmf_session_print(session, "runner_0_task_direct_queue: %lu\n\r" , rx_configuration.runner_0_task_direct_queue ) ;
        bdmf_session_print(session, "runner_1_task_normal_queue: %lu\n\r" , rx_configuration.runner_1_task_normal_queue ) ;
        bdmf_session_print(session, "runner_1_task_direct_queue: %lu\n\r" , rx_configuration.runner_1_task_direct_queue ) ;
        bdmf_session_print(session, "dma_data_fifo_base_address: 0x%lX\n\r" , rx_configuration.dma_data_fifo_base_address ) ;
        bdmf_session_print(session, "dma_chunk_descriptor_fifo_base_address: 0x%lX\n\r" , rx_configuration.dma_chunk_descriptor_fifo_base_address ) ;
        bdmf_session_print(session, "dma_data_and_chunk_descriptor_fifos_size: %lu\n\r" , rx_configuration.dma_data_and_chunk_descriptor_fifos_size ) ;
        bdmf_session_print(session, "dma_exclusive_threshold: %lu\n\r" , rx_configuration.dma_exclusive_threshold ) ;
        bdmf_session_print(session, "sdma_data_fifo_base_address: 0x%lX\n\r" , rx_configuration.sdma_data_fifo_base_address ) ;
        bdmf_session_print(session, "sdma_chunk_descriptor_fifo_base_address: 0x%lX\n\r" , rx_configuration.sdma_chunk_descriptor_fifo_base_address ) ;
        bdmf_session_print(session, "sdma_data_and_chunk_descriptor_fifos_size: %lu\n\r" , rx_configuration.sdma_data_and_chunk_descriptor_fifos_size ) ;
        bdmf_session_print(session, "sdma_exclusive_threshold: %lu\n\r" , rx_configuration.sdma_exclusive_threshold ) ;
        bdmf_session_print(session, "minimum_packet_size_0: %lu\n\r" , rx_configuration.minimum_packet_size_0 ) ;
        bdmf_session_print(session, "minimum_packet_size_1: %lu\n\r" , rx_configuration.minimum_packet_size_1 ) ;
        bdmf_session_print(session, "minimum_packet_size_2: %lu\n\r" , rx_configuration.minimum_packet_size_2 ) ;
        bdmf_session_print(session, "minimum_packet_size_3: %lu\n\r" , rx_configuration.minimum_packet_size_3 ) ;
        bdmf_session_print(session, "maximum_packet_size_0: %lu\n\r" , rx_configuration.maximum_packet_size_0 ) ;
        bdmf_session_print(session, "maximum_packet_size_1: %lu\n\r" , rx_configuration.maximum_packet_size_1 ) ;
        bdmf_session_print(session, "maximum_packet_size_2: %lu\n\r" , rx_configuration.maximum_packet_size_2 ) ;
        bdmf_session_print(session, "maximum_packet_size_3: %lu\n\r" , rx_configuration.maximum_packet_size_3 ) ;
        bdmf_session_print(session, "ih_ingress_buffers_bitmask: 0x%lX\n\r" , rx_configuration.ih_ingress_buffers_bitmask ) ;
        bdmf_session_print(session, "packet_header_offset: %lu\n\r" , rx_configuration.packet_header_offset ) ;
        bdmf_session_print(session, "flow_control_triggers_bitmask: 0x%lX\n\r" , rx_configuration.flow_control_triggers_bitmask ) ;
        bdmf_session_print(session, "drop_triggers_bitmask: 0x%lX\n\r" , rx_configuration.drop_triggers_bitmask ) ;
        bdmf_session_print(session, "flows_32_255_group_divider: %lu\n\r" , rx_configuration.flows_32_255_group_divider ) ;
        bdmf_session_print(session, "ploam_default_ih_class: %lu\n\r" , rx_configuration.ploam_default_ih_class ) ;
        bdmf_session_print(session, "ploam_ih_class_override: %lu\n\r" , rx_configuration.ploam_ih_class_override ) ;
        bdmf_session_print(session, "reassembly_offset_in_8_byte: %lu\n\r" , rx_configuration.reassembly_offset_in_8_byte ) ;
        for(ppidx = 0; ppidx < 7 ;ppidx++ )
        {
            if(rx_configuration.pp_task_enable_bitmap & (1<<ppidx))
                bdmf_session_print(session, "parallel processing task[%d] = 0%02x\n\r" , rx_configuration.pp_task_nums[ppidx] ) ;
        }
        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    uint8_t flow_control_triggers_bitmask ; /* input */
    uint8_t drop_triggers_bitmask ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;
    flow_control_triggers_bitmask = parm[parameter_index++].value.unumber;
    if (!flow_control_triggers_bitmask)
            p_print_rx_flow_control_trigger_enum_help(session);
    drop_triggers_bitmask = parm[parameter_index++].value.unumber;
    if (!drop_triggers_bitmask)
        p_print_rx_drop_trigger_enum_help(session);

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop(port_index, flow_control_triggers_bitmask, drop_triggers_bitmask);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_bbh_rx_get_triggers_of_flow_control_and_drop_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    uint8_t flow_control_triggers_bitmask ; /* output */
    uint8_t drop_triggers_bitmask ; /* output */
    DRV_BBH_ERROR error ;

    /* Get the parameters */
    port_index = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_get_triggers_of_flow_control_and_drop'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_get_triggers_of_flow_control_and_drop ( port_index ,
                                                                           & flow_control_triggers_bitmask ,
                                                                           & drop_triggers_bitmask ) ;
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "flow_control_triggers_bitmask: 0x%lX\n\r" , flow_control_triggers_bitmask ) ;
        bdmf_session_print(session, "drop_triggers_bitmask: 0x%lX\n\r" , drop_triggers_bitmask ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_bbh_rx_set_per_flow_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION flow_index ; /* input */
    DRV_BBH_PER_FLOW_CONFIGURATION per_flow_configuration ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;
    flow_index = parm[parameter_index++].value.unumber;
    per_flow_configuration.minimum_packet_size_selection = parm[parameter_index++].value.unumber;
    per_flow_configuration.maximum_packet_size_selection = parm[parameter_index++].value.unumber;
    per_flow_configuration.default_ih_class = parm[parameter_index++].value.unumber;
    per_flow_configuration.ih_class_override = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_set_per_flow_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_set_per_flow_configuration(port_index, flow_index, &per_flow_configuration);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_bbh_rx_get_per_flow_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION flow_index ; /* input */
    DRV_BBH_PER_FLOW_CONFIGURATION per_flow_configuration ; /* output */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;
    flow_index = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_get_per_flow_configuration'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_get_per_flow_configuration(port_index, flow_index, &per_flow_configuration);
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "minimum_packet_size_selection: %lu\n\r" , per_flow_configuration.minimum_packet_size_selection ) ;
        bdmf_session_print(session, "maximum_packet_size_selection: %lu\n\r" , per_flow_configuration.maximum_packet_size_selection ) ;
        bdmf_session_print(session, "default_ih_class: %lu\n\r" , per_flow_configuration.default_ih_class ) ;
        bdmf_session_print(session, "ih_class_override: %lu\n\r" , per_flow_configuration.ih_class_override ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static int p_bl_drv_bbh_rx_reset_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    uint16_t units_to_reset_bitmask ; /* input */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;
    units_to_reset_bitmask = parm[parameter_index++].value.unumber;
    if (!units_to_reset_bitmask)
    {
        p_print_rx_internal_unit_enum_help(session);
        return 0;
    }
    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_reset'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_reset(port_index, units_to_reset_bitmask);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_get_counters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_COUNTERS rx_counters ; /* output */
    DRV_BBH_ERROR error ;

    /* Get the parameters */
    port_index = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_get_counters'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_get_counters(port_index, &rx_counters);
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "BBH RX counters for port %d (%s):\n\r" , port_index , f_bbh_port_index_enum_to_string ( port_index ) ) ;
        bdmf_session_print(session, "=====================================\n\r" ) ;

        bdmf_session_print(session, "incoming_packets: %lu\n\r" , rx_counters.incoming_packets ) ;
        bdmf_session_print(session, "too_short_error: %lu\n\r" , rx_counters.too_short_error ) ;
        bdmf_session_print(session, "too_long_error: %lu\n\r" , rx_counters.too_long_error ) ;
        bdmf_session_print(session, "crc_error: %lu\n\r" , rx_counters.crc_error ) ;
        bdmf_session_print(session, "runner_congestion: %lu\n\r" , rx_counters.runner_congestion ) ;
        bdmf_session_print(session, "no_bpm_bn_error: %lu\n\r" , rx_counters.no_bpm_bn_error ) ;
        bdmf_session_print(session, "no_sbpm_sbn_error: %lu\n\r" , rx_counters.no_sbpm_sbn_error ) ;
        bdmf_session_print(session, "no_dma_cd_error: %lu\n\r" , rx_counters.no_dma_cd_error ) ;
        bdmf_session_print(session, "no_sdma_cd_error: %lu\n\r" , rx_counters.no_sdma_cd_error ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_get_iptv_filter_counter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    uint32_t iptv_filter_counter ; /* output */
    DRV_BBH_ERROR error ;

    /* Get the parameters */
    port_index = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_get_iptv_filter_counter'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_get_iptv_filter_counter(port_index, &iptv_filter_counter);
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "BBH RX IPTV filter counter for port %d (%s):\n\r" , port_index , f_bbh_port_index_enum_to_string ( port_index ) ) ;
        bdmf_session_print(session, "=====================================\n\r" ) ;

        bdmf_session_print(session, "iptv_filter_counter: %lu\n\r" , iptv_filter_counter ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_get_error_counters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    DRV_BBH_RX_ERROR_COUNTERS rx_error_counters ; /* output */
    DRV_BBH_ERROR error ;

    /* Get the parameters */
    port_index = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_get_error_counters'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_get_error_counters(port_index, &rx_error_counters);
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "BBH RX error counters for port %d (%s):\n\r" , port_index , f_bbh_port_index_enum_to_string ( port_index ) ) ;
        bdmf_session_print(session, "=====================================\n\r" ) ;

        bdmf_session_print(session, "sop_after_sop_error: %lu\n\r" , rx_error_counters.sop_after_sop_error ) ;
        bdmf_session_print(session, "third_flow_error: %lu\n\r" , rx_error_counters.third_flow_error ) ;
        bdmf_session_print(session, "ih_drop_error_for_ploam: %lu\n\r" , rx_error_counters.ih_drop_error_for_ploam ) ;
        bdmf_session_print(session, "no_bpm_bn_error_for_ploam: %lu\n\r" , rx_error_counters.no_bpm_bn_error_for_ploam ) ;
        bdmf_session_print(session, "crc_error_for_ploam: %lu\n\r" , rx_error_counters.crc_error_for_ploam ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bbh_rx_get_per_flow_counters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BBH_PORT_INDEX port_index ; /* input */
    uint8_t flow_index ; /* input */
    uint16_t number_of_packets_dropped_by_ih ; /* output */
    DRV_BBH_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    port_index = parm[parameter_index++].value.unumber;
    flow_index = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bbh_rx_get_per_flow_counters'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bbh_rx_get_per_flow_counters(port_index, flow_index, &number_of_packets_dropped_by_ih);
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BBH_NO_ERROR )
    {
        bdmf_session_print(session, "BBH RX per-flow counter for port %d (%s), flow %d:\n\r" ,
                          port_index , f_bbh_port_index_enum_to_string ( port_index ) , flow_index ) ;
        bdmf_session_print(session, "========================================\n\r" ) ;

        bdmf_session_print(session, "number_of_packets_dropped_by_ih: %lu\n\r" , number_of_packets_dropped_by_ih ) ;

        bdmf_session_print(session, "\n\r" ) ;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bbh_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

/* void bzero(void *s, size_t n); */


static int p_bl_drv_bbh_debug_write_memory_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint32_t* address ; /* input */
    uint32_t number_of_words ; /* input */
    uint32_t value ; /* input */
    uint32_t index ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    address = (uint32_t*)(uintptr_t)parm[parameter_index++].value.unumber64;
    number_of_words = parm[parameter_index++].value.unumber;
    value = parm[parameter_index++].value.unumber;

    for ( index = 0 ; index < number_of_words ; ++ index )
    {
        *address = value ;
        ++address;
    }

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: memory reset done\n\r" ) ;

    return 0;
}


#define MAX_RUNNER_TASK 31

static bdmfmon_handle_t bbh_dir;

void pi_bl_initialize_drv_bbh_shell(bdmfmon_handle_t driver_dir)
{
    static struct bdmfmon_enum_val bbh_port_enum_table[] = {
        {"WAN",   DRV_BBH_WAN},
        {"EMAC_0", DRV_BBH_EMAC_0},
        {"EMAC_1", DRV_BBH_EMAC_1},
        {"EMAC_2", DRV_BBH_EMAC_2},
        {NULL, 0},
    };
    static struct bdmfmon_enum_val ddr_bufsize_enum_table[] = {
        {"2KB",   DRV_BBH_DDR_BUFFER_SIZE_2_KB},
        {"4KB", DRV_BBH_DDR_BUFFER_SIZE_4_KB},
        {"16KB", DRV_BBH_DDR_BUFFER_SIZE_16_KB},
        {NULL, 0},
    };
    static struct bdmfmon_enum_val pl_offset_res_enum_table[] = {
        {"1B",   DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_1_B},
        {"2B", DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_2_B},
        {NULL, 0},
    };

    bbh_dir = bdmfmon_dir_add(driver_dir, "bbhd", "BBH Driver", BDMF_ACCESS_ADMIN, NULL );
    if (!bbh_dir)
    {
        bdmf_session_print(NULL, "Can't create bbhd directory\n");
        return;
    }

    BDMFMON_MAKE_CMD(bbh_dir, "tsmc", "TX set misc configuration", p_bl_drv_bbh_tx_set_misc_configuration_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("ddr_buffer_size",  "DDR buffer size", ddr_bufsize_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("pl_offset_res",  "Payload offset resolution", pl_offset_res_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("mc_hdr_size", "Multicast header size", BDMFMON_PARM_NUMBER, 0, DRV_BBH_MINIMAL_MULTICAST_HEADER_SIZE,
            DRV_BBH_MAXIMAL_MULTICAST_HEADER_SIZE),
        BDMFMON_MAKE_PARM_RANGE("mc_hdr_base", "Multicast header base address (hex)", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM_RANGE("dma_read_base", "dma_read_requests_fifo_base_address (hex)", BDMFMON_PARM_HEX, 0, 0, 0x3f),
        BDMFMON_MAKE_PARM_RANGE("dma_read_max_num", "dma_read_requests_maximal_number", BDMFMON_PARM_NUMBER, 0,
            DRV_BBH_TX_MINIMAL_VALUE_FOR_DMA_READ_REQUESTS_MAXIMAL_NUMBER, DRV_BBH_TX_MAXIMAL_VALUE_FOR_DMA_READ_REQUESTS_MAXIMAL_NUMBER),
        BDMFMON_MAKE_PARM_RANGE("sdma_read_base", "sdma_read_requests_fifo_base_address (hex)", BDMFMON_PARM_HEX, 0, 0, 0x1f),
        BDMFMON_MAKE_PARM_RANGE("sdma_read_max_num", "sdma_read_requests_maximal_number", BDMFMON_PARM_NUMBER, 0,
            DRV_BBH_TX_MINIMAL_VALUE_FOR_SDMA_READ_REQUESTS_MAXIMAL_NUMBER, DRV_BBH_TX_MAXIMAL_VALUE_FOR_SDMA_READ_REQUESTS_MAXIMAL_NUMBER),
        BDMFMON_MAKE_PARM_RANGE("tcont", "tcont_address (hex)", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("skb", "skb_address (hex)", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("mdu_mode_enable", "MDU mode enable (0/1)", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("mdu_mode_read_ptr", "mdu_mode_read_pointer_address", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("ddr_tm_base_address", "ddr_tm_base_address", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM_RANGE("emac_1588_enable", "emac_1588_enable (0/1)", BDMFMON_PARM_NUMBER, 0, 0, 1));


    BDMFMON_MAKE_CMD(bbh_dir, "tsra", "TX set route addresses", p_bl_drv_bbh_tx_set_route_addresses_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("dma", "dma_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("bpm", "bpm_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("sdma", "sdma_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("sbpm", "sbpm_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("runner", "runner_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f));

    BDMFMON_MAKE_CMD(bbh_dir, "tsrt",
            "TX set runner tasks\n"
            "The BBH manages 40 queues (1 for each TCONT). Each of the first 8 Queues\n"
            "may have a unique Runner task number (this task will be woken up when the\n"
            "queue has empty space). Queues 8-39 have all the same configurable task\n"
            "number. In Ethernet case, only Task 0 is relevant.",
            p_bl_drv_bbh_tx_set_runner_tasks_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("task_0", "task_0", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("task_1", "task_1", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("task_2", "task_2", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("task_3", "task_3", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("task_4", "task_4", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("task_5", "task_5", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("task_6", "task_6", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("task_7", "task_7", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("task_8_39", "task_8_39", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK));


    BDMFMON_MAKE_CMD(bbh_dir, "tspf",
            "TX set PD FIFOs\n"
            "The BBH manages 40 queues (1 for each TCONT). For each queue it manages\n"
            "a PD FIFO. A total of 128 PDs is available for all queues. For each Queue\n"
            "the SW configures the base and the size within these 128 PDs. For\n"
            "Ethernet, queue 0 should be configured (and its size is up to 8).\n",
            p_bl_drv_bbh_tx_set_pd_fifos_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("fs_0", "pd_fifo_size_0", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_1", "pd_fifo_size_1", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_2", "pd_fifo_size_2", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_3", "pd_fifo_size_3", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_4", "pd_fifo_size_4", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_5", "pd_fifo_size_5", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_6", "pd_fifo_size_6", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_7", "pd_fifo_size_7", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_8", "pd_fifo_size_8_15", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_16", "pd_fifo_size_16_23", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_24", "pd_fifo_size_24_31", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_32", "pd_fifo_size_32_39", BDMFMON_PARM_NUMBER, 0, DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE,
            DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM("base1", "pd_fifo_base_0", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base2", "pd_fifo_base_2", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base3", "pd_fifo_base_3", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base4", "pd_fifo_base_4", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base5", "pd_fifo_base_5", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base6", "pd_fifo_base_6", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base7", "pd_fifo_base_7", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base8", "pd_fifo_base_8_15", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base16", "pd_fifo_base_16_23", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base24", "pd_fifo_base_24_31", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("base32", "pd_fifo_base_32_39", BDMFMON_PARM_NUMBER, 0));


    BDMFMON_MAKE_CMD(bbh_dir, "tsppbt",
            "TX set PD prefetch byte thresholds\n"
            "The BBH requests PDs from the Runner and maintains a pre-fetch PDs FIFO.\n"
            "The PDs pre fetch is limited either by the PD FIFO configurable size or\n"
            "according to the total number of bytes for preventing HOL blocking (high\n"
            "priority queue in the Runner needs to wait for low priority queues' data\n"
            "that had been already sent to the BBH).The thresholds are relevant if\n"
            "'PD prefetch byte threshold enable' is set\n",
            p_bl_drv_bbh_tx_set_pd_prefetch_byte_thresholds_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("pbt_enable",  "pd_prefetch_byte_threshold_enable: 0/1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("pbt_0", "pd_prefetch_byte_threshold_0_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE),
        BDMFMON_MAKE_PARM_RANGE("pbt_1", "pd_prefetch_byte_threshold_1_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE),
        BDMFMON_MAKE_PARM_RANGE("pbt_2", "pd_prefetch_byte_threshold_2_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE),
        BDMFMON_MAKE_PARM_RANGE("pbt_3", "pd_prefetch_byte_threshold_3_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
        DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE),
        BDMFMON_MAKE_PARM_RANGE("pbt_4", "pd_prefetch_byte_threshold_4_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE),
        BDMFMON_MAKE_PARM_RANGE("pbt_5", "pd_prefetch_byte_threshold_5_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE),
        BDMFMON_MAKE_PARM_RANGE("pbt_6", "pd_prefetch_byte_threshold_6_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE),
        BDMFMON_MAKE_PARM_RANGE("pbt_7", "pd_prefetch_byte_threshold_7_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE),
        BDMFMON_MAKE_PARM_RANGE("pbt_8", "pd_prefetch_byte_threshold_8_39_in_32_byte", BDMFMON_PARM_NUMBER, 0, 0,
            DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE));


    BDMFMON_MAKE_CMD(bbh_dir, "tgc", "TX get configuration", p_bl_drv_bbh_tx_get_configuration_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bbh_dir, "tr1", "TX reset:", p_bl_drv_bbh_tx_reset_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("units", "units_to_reset_bitmask: Hex. 0=help", BDMFMON_PARM_HEX, 0, 0, 0x3ff));

    BDMFMON_MAKE_CMD(bbh_dir, "tgc1", "TX Get counters", p_bl_drv_bbh_tx_get_counters_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bbh_dir, "rsmc", "RX set misc configuration", p_bl_drv_bbh_rx_set_misc_configuration_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("ddr_buffer_size",  "DDR buffer size", ddr_bufsize_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("ddr_tm_base", "ddr_tm_base_address: Hex", BDMFMON_PARM_HEX, 0, 0, 0x0fffffff),
        BDMFMON_MAKE_PARM_RANGE("ih_buf_mask", "ih_ingress_buffers_bitmask: Hex", BDMFMON_PARM_HEX, 0, 0, 0x0ffff),
        BDMFMON_MAKE_PARM_RANGE("pkt_hdr_offset", "packet_header_offset", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET),
        BDMFMON_MAKE_PARM_RANGE("fl_grp_divider", "flows_32_255_group_divider", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MINIMAL_FLOWS_32_255_GROUP_DIVIDER),
        BDMFMON_MAKE_PARM_RANGE("ploam_default_ih_class", "ploam_default_ih_class", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_IH_CLASS),
        BDMFMON_MAKE_PARM_RANGE("ploam_ih_class_override",  "ploam_ih_class_override: 0/1", BDMFMON_PARM_NUMBER, 0, 0, 1),
        BDMFMON_MAKE_PARM_RANGE("reass_offset",  "reassembly_offset_in_8_byte", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_REASSEMBLY_OFFSET_IN_8_BYTE));

    BDMFMON_MAKE_CMD(bbh_dir, "rsra", "RX set route addresses", p_bl_drv_bbh_rx_set_route_addresses_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("dma", "dma_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("bpm", "bpm_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("sdma", "sdma_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("sbpm", "sbpm_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("runner0", "runner_0_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("runner1", "runner_1_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f),
        BDMFMON_MAKE_PARM_RANGE("ih", "ih_route_address", BDMFMON_PARM_HEX, 0, 0, 0x7f));

    BDMFMON_MAKE_CMD(bbh_dir, "rspf", "RX set PD FIFOs", p_bl_drv_bbh_rx_set_pd_fifos_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("fb_normal", "pd_fifo_base_address_normal_queue: Hex", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("fb_direct", "pd_fifo_base_address_direct_queue: Hex", BDMFMON_PARM_HEX, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("fs_normal", "pd_fifo_size_normal_queue", BDMFMON_PARM_NUMBER, 0, DRV_BBH_RX_MINIMAL_PD_FIFO_SIZE, DRV_BBH_RX_MAXIMAL_PD_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("fs_direct", "pd_fifo_size_direct_queue", BDMFMON_PARM_NUMBER, 0, DRV_BBH_RX_MINIMAL_PD_FIFO_SIZE, DRV_BBH_RX_MAXIMAL_PD_FIFO_SIZE));

    BDMFMON_MAKE_CMD(bbh_dir, "rsrt", "RX set runner tasks", p_bl_drv_bbh_rx_set_runner_tasks_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("rnr0_normal", "runner_0_task_normal_queue", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("rnr0_direct", "runner_0_task_direct_queue", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("rnr1_normal", "runner_1_task_normal_queue", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK),
        BDMFMON_MAKE_PARM_RANGE("rnr1_direct", "runner_1_task_direct_queue", BDMFMON_PARM_NUMBER, 0, 0, MAX_RUNNER_TASK));

    BDMFMON_MAKE_CMD(bbh_dir, "rsdasf", "RX set DMA and SDMA FIFOs", p_bl_drv_bbh_rx_set_dma_and_sdma_fifos_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("dma_data_fb", "dma_data_fifo_base_address: Hex", BDMFMON_PARM_HEX, 0, 0, 0x3f),
        BDMFMON_MAKE_PARM_RANGE("dma_chunk_fb", "dma_data_fifo_base_address: Hex", BDMFMON_PARM_HEX, 0, 0, 0x3f),
        BDMFMON_MAKE_PARM_RANGE("dma_data_chunk_fs", "dma_data_and_chunk_descriptor_fifos_size", BDMFMON_PARM_NUMBER, 0,
            DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE, DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("dma_exclusive_threshold", "dma_exclusive_threshold", BDMFMON_PARM_NUMBER, 0,
            DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE, DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("sdma_data_fb", "sdma_data_fifo_base_address: Hex", BDMFMON_PARM_HEX, 0, 0, 0x3f),
        BDMFMON_MAKE_PARM_RANGE("sdma_chunk_fb", "sdma_data_fifo_base_address: Hex", BDMFMON_PARM_HEX, 0, 0, 0x3f),
        BDMFMON_MAKE_PARM_RANGE("sdma_data_chunk_fs", "sdma_data_and_chunk_descriptor_fifos_size", BDMFMON_PARM_NUMBER, 0,
            DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE, DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE),
        BDMFMON_MAKE_PARM_RANGE("sdma_exclusive_threshold", "sdma_exclusive_threshold", BDMFMON_PARM_NUMBER, 0,
            DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE, DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE));

    BDMFMON_MAKE_CMD(bbh_dir, "rsmamps", "RX set minimum and maximum packet size", p_bl_drv_bbh_rx_set_minimum_and_maximum_packet_size_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("min_size_0", "minimum_packet_size_0", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("min_size_1", "minimum_packet_size_1", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("min_size_2", "minimum_packet_size_2", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("min_size_3", "minimum_packet_size_3", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("max_size_0", "maximum_packet_size_0", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("max_size_1", "maximum_packet_size_1", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("max_size_2", "maximum_packet_size_2", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("max_size_3", "maximum_packet_size_3", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE));

    BDMFMON_MAKE_CMD(bbh_dir, "rgc", "RX get configuration", p_bl_drv_bbh_rx_get_configuration_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bbh_dir, "rstofcad", "RX set triggers of flow control and drop", p_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("fc_triggers", "flow_control_triggers_bitmask 0=help: Hex", BDMFMON_PARM_HEX, 0, 0, 0x7),
        BDMFMON_MAKE_PARM_RANGE("drop_triggers", "drop_triggers_bitmask 0=help: Hex", BDMFMON_PARM_HEX, 0, 0, 0x3));

    BDMFMON_MAKE_CMD(bbh_dir, "rgtofcad", "RX get triggers of flow control and drop\n"
        "This function gets the triggers for sending flow control to MAC, and\n"
        "the triggers for dropping packets. For flow control, there are 3 possible\n"
        "triggers: BPM is in exclusive state, SBPM is in exclusive state, Runner\n"
        "request. For drop, there are 2 possible triggers: BPM is in exclusive\n"
        "state, SBPM is in exclusive state. The triggers statuses are given in a\n"
        "bitmask.\n",
        p_bl_drv_bbh_rx_get_triggers_of_flow_control_and_drop_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bbh_dir, "rspfc", "RX set per flow configuration\n"
         "This function sets per-flow configuration in the RX part of BBH block.\n"
         "Each one of flows 0-31 has its own configuration. Flows 32-255 are\n"
         "divided into 2 groups (32 to x, x+1 to 255). Each group has its own\n"
         "configuration. The groups-divider (x) is configured in 'RX Set\n"
         "configuration' API. In Ethernet case, only flow 0 is relevant.\n",
        p_bl_drv_bbh_rx_set_per_flow_configuration_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("flow", "flow_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0-1),
        BDMFMON_MAKE_PARM_RANGE("min_packet_size", "minimum_packet_size_selection", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_SELECTION_FOR_MINIMUM_OR_MAXIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("max_packet_size", "maximum_packet_size_selection", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_SELECTION_FOR_MINIMUM_OR_MAXIMUM_PACKET_SIZE),
        BDMFMON_MAKE_PARM_RANGE("default_ih_class", "default_ih_class", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_MAXIMAL_IH_CLASS),
        BDMFMON_MAKE_PARM_RANGE("ih_class_override",  "ih_class_override: 0/1", BDMFMON_PARM_NUMBER, 0, 0, 1));

    BDMFMON_MAKE_CMD(bbh_dir, "rgpfc", "RX get per flow configuration\n"
        "This function gets the per-flow configuration in the RX part of BBH\n"
        "block. Each one of flows 0-31 has its own configuration. Flows 32-255\n"
        "are divided into 2 groups (32 to x, x+1 to 255). Each group has its own\n"
        "configuration. The groups-divider (x) is configured in 'RX Set per flow\n"
        "configuration' API. In Ethernet case, only flow 0 is relevant.\n",
        p_bl_drv_bbh_rx_get_per_flow_configuration_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("flow", "flow_index", BDMFMON_PARM_NUMBER, 0, 0, DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0-1));

    BDMFMON_MAKE_CMD(bbh_dir, "rr", "RX reset", p_bl_drv_bbh_rx_reset_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("units", "units_to_reset_bitmask: Hex. 0=help", BDMFMON_PARM_HEX, 0, 0, 0x3ff));

    BDMFMON_MAKE_CMD(bbh_dir, "rgc1", "RX get counters", p_bl_drv_bbh_rx_get_counters_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bbh_dir, "rgifc", "RX get IPTV filter counter", p_bl_drv_bbh_rx_get_iptv_filter_counter_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bbh_dir, "rgec", "RX get error counters", p_bl_drv_bbh_rx_get_error_counters_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bbh_dir, "rgpfc1", "RX get per flow counters\n"
        "This function gets, for the specified flow, the number of packets\n"
        "dropped by IH (due to either runner congestion or IPTV filter). These\n"
        "counters are relevant for GPON only. In Ethernet case, this information\n"
        "can be obtained using 'RX Get Counters' API. Each of these counters is\n"
        "cleared when read and freezes when maximum value is reached.\n",
        p_bl_drv_bbh_rx_get_per_flow_counters_command,
        BDMFMON_MAKE_PARM_ENUM("port",  "Port index", bbh_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("flow", "flow_index", BDMFMON_PARM_NUMBER, 0, 0, 255));

    BDMFMON_MAKE_CMD(bbh_dir, "dwm", "Debug write memory", p_bl_drv_bbh_debug_write_memory_command,
        BDMFMON_MAKE_PARM("address", "address: Hex64", BDMFMON_PARM_HEX64, 0),
        BDMFMON_MAKE_PARM("num_words", "number_of_words", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0));

}

void pi_bl_exit_drv_bbh_shell(void)
{
    if (bbh_dir)
    {
        bdmfmon_token_destroy(bbh_dir);
        bbh_dir = NULL;
    }
}


char * f_drv_bbh_error_code_to_string ( DRV_BBH_ERROR xi_error_code )
{
    switch ( xi_error_code )
    {
    case DRV_BBH_NO_ERROR:
        return "DRV_BBH_NO_ERROR" ;
        break ;
    case DRV_BBH_INVALID_PORT_INDEX:
        return "DRV_BBH_INVALID_PORT_INDEX" ;
        break ;
    case DRV_BBH_INVALID_MULTICAST_HEADER_SIZE:
        return "DRV_BBH_INVALID_MULTICAST_HEADER_SIZE" ;
        break ;
    case DRV_BBH_INVALID_TX_PD_FIFO_SIZE:
        return "DRV_BBH_INVALID_TX_PD_FIFO_SIZE" ;
        break ;
    case DRV_BBH_INVALID_PD_PREFETCH_BYTE_THRESHOLD:
        return "DRV_BBH_INVALID_PD_PREFETCH_BYTE_THRESHOLD" ;
        break ;
    case DRV_BBH_INVALID_DMA_READ_REQUESTS_MAXIMAL_NUMBER:
        return "DRV_BBH_INVALID_DMA_READ_REQUESTS_MAXIMAL_NUMBER" ;
        break ;
    case DRV_BBH_INVALID_SDMA_READ_REQUESTS_MAXIMAL_NUMBER:
        return "DRV_BBH_INVALID_SDMA_READ_REQUESTS_MAXIMAL_NUMBER" ;
        break ;
    case DRV_BBH_INVALID_RX_PD_FIFO_SIZE:
        return "DRV_BBH_INVALID_RX_PD_FIFO_SIZE" ;
        break ;
    case DRV_BBH_INVALID_DMA_DATA_AND_CHUNK_DESCRIPTOR_FIFOS_SIZE:
        return "DRV_BBH_INVALID_DMA_DATA_AND_CHUNK_DESCRIPTOR_FIFOS_SIZE" ;
        break ;
    case DRV_BBH_INVALID_DMA_EXCLUSIVE_THRESHOLD:
        return "DRV_BBH_INVALID_DMA_EXCLUSIVE_THRESHOLD" ;
        break ;
    case DRV_BBH_INVALID_SDMA_DATA_AND_CHUNK_DESCRIPTOR_FIFOS_SIZE:
        return "DRV_BBH_INVALID_SDMA_DATA_AND_CHUNK_DESCRIPTOR_FIFOS_SIZE" ;
        break ;
    case DRV_BBH_INVALID_SDMA_EXCLUSIVE_THRESHOLD:
        return "DRV_BBH_INVALID_SDMA_EXCLUSIVE_THRESHOLD" ;
        break ;
    case DRV_BBH_INVALID_MINIMUM_PACKET_SIZE:
        return "DRV_BBH_INVALID_MIN_PKT_SIZE" ;
        break ;
    case DRV_BBH_INVALID_MAXIMUM_PACKET_SIZE:
        return "DRV_BBH_INVALID_MAX_PKT_SIZE" ;
        break ;
    case DRV_BBH_INVALID_PACKET_HEADER_OFFSET:
        return "DRV_BBH_INVALID_PKT_HEADER_OFFSET" ;
        break ;
    case DRV_BBH_INVALID_FLOWS_32_255_GROUP_DIVIDER:
        return "DRV_BBH_INVALID_FLOWS_32_255_GROUP_DIVIDER" ;
        break ;
    case DRV_BBH_INVALID_IH_CLASS:
        return "DRV_BBH_INVALID_IH_CLASS" ;
        break ;
    case DRV_BBH_INVALID_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION:
        return "DRV_BBH_INVALID_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION" ;
        break ;
    case DRV_BBH_ILLEGAL_FLOW_INDEX_FOR_ETHERNET_PORT:
        return "DRV_BBH_ILLEGAL_FLOW_INDEX_FOR_ETH_PORT" ;
        break ;
    case DRV_BBH_INVALID_MINIMUM_PACKET_SIZE_SELECTION:
        return "DRV_BBH_INVALID_MIN_PKT_SIZE_SELECTION" ;
        break ;
    case DRV_BBH_INVALID_MAXIMUM_PACKET_SIZE_SELECTION:
        return "DRV_BBH_INVALID_MAX_PKT_SIZE_SELECTION" ;
        break ;
    case DRV_BBH_API_IS_FOR_GPON_PORT_ONLY:
        return "DRV_BBH_API_IS_FOR_GPON_PORT_ONLY" ;
        break ;
    case DRV_BBH_INVALID_REASSEMBLY_OFFSET:
        return "DRV_BBH_INVALID_REASSEMBLY_OFFSET" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static char * f_bbh_port_index_enum_to_string ( DRV_BBH_PORT_INDEX xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BBH_EMAC_0:
        return "EMAC_0" ;
        break ;
    case DRV_BBH_EMAC_1:
        return "EMAC_1" ;
        break ;
    case DRV_BBH_EMAC_2:
        return "EMAC_2" ;
        break ;
    case DRV_BBH_WAN:
        return "WAN" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static char * f_ddr_buffer_size_enum_to_string ( DRV_BBH_PORT_INDEX xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BBH_DDR_BUFFER_SIZE_2_KB:
        return "DDR_BUFFER_SIZE_2_KB" ;
        break ;
    case DRV_BBH_DDR_BUFFER_SIZE_4_KB:
        return "DDR_BUFFER_SIZE_4_KB" ;
        break ;
    case DRV_BBH_DDR_BUFFER_SIZE_16_KB:
        return "DDR_BUFFER_SIZE_16_KB" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static char * f_payload_offset_resolution_enum_to_string ( DRV_BBH_PORT_INDEX xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_1_B:
        return "PAYLOAD_OFFSET_RESOLUTION_1_B" ;
        break ;
    case DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_2_B:
        return "PAYLOAD_OFFSET_RESOLUTION_2_B" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static char * f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BBH_TX_INTERNAL_UNIT_SEGMENTATION_CONTEXT_TABLE:
        return "TX_INTERNAL_UNIT_SEGMENTATION_CONTEXT_TABLE" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_ALL_40_PDS_FIFOS:
        return "TX_INTERNAL_UNIT_ALL_40_PDS_FIFOS" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_WRITE_POINTER_IN_THE_DMA:
        return "TX_INTERNAL_UNIT_WRITE_POINTER_IN_THE_DMA" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_WRITE_POINTER_IN_THE_SDMA:
        return "TX_INTERNAL_UNIT_WRITE_POINTER_IN_THE_SDMA" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_BPM_RELEASE_FIFO:
        return "TX_INTERNAL_UNIT_BPM_RELEASE_FIFO" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_SBPM_RELEASE_FIFO:
        return "TX_INTERNAL_UNIT_SBPM_RELEASE_FIFO" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_ORDER_KEEPER_FIFO:
        return "TX_INTERNAL_UNIT_ORDER_KEEPER_FIFO" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_DDR_DATA_FIFO:
        return "TX_INTERNAL_UNIT_DDR_DATA_FIFO" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_SRAM_DATA_FIFO:
        return "TX_INTERNAL_UNIT_SRAM_DATA_FIFO" ;
        break ;
    case DRV_BBH_TX_INTERNAL_UNIT_SKB_POINTERS:
        return "TX_INTERNAL_UNIT_SKB_POINTERS" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static void p_print_tx_internal_unit_enum_help(bdmf_session_handle session)
{
    uint8_t bit_number = 0 ;

    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_SEGMENTATION_CONTEXT_TABLE ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_ALL_40_PDS_FIFOS ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_WRITE_POINTER_IN_THE_DMA ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_WRITE_POINTER_IN_THE_SDMA ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_BPM_RELEASE_FIFO ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_SBPM_RELEASE_FIFO ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_ORDER_KEEPER_FIFO ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_DDR_DATA_FIFO ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_SRAM_DATA_FIFO ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_tx_internal_unit_enum_to_string ( DRV_BBH_TX_INTERNAL_UNIT_SKB_POINTERS ) ) ;
}


static void p_print_rx_flow_control_trigger_enum_help(bdmf_session_handle session)
{
    uint8_t bit_number = 0 ;

    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_rx_flow_control_trigger_enum_to_string ( DRV_BBH_RX_FLOW_CONTROL_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_rx_flow_control_trigger_enum_to_string ( DRV_BBH_RX_FLOW_CONTROL_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_rx_flow_control_trigger_enum_to_string ( DRV_BBH_RX_FLOW_CONTROL_TRIGGER_RUNNER_REQUEST ) ) ;
}

static char * f_rx_flow_control_trigger_enum_to_string ( DRV_BBH_RX_FLOW_CONTROL_TRIGGER xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BBH_RX_FLOW_CONTROL_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE:
        return "RX_FLOW_CTRL_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE" ;
        break ;
    case DRV_BBH_RX_FLOW_CONTROL_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE:
        return "RX_FLOW_CTRL_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE" ;
        break ;
    case DRV_BBH_RX_FLOW_CONTROL_TRIGGER_RUNNER_REQUEST:
        return "RX_FLOW_CTRL_TRIGGER_RUNNER_REQUEST" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static void p_print_rx_drop_trigger_enum_help(bdmf_session_handle session)
{
    uint8_t bit_number = 0 ;

    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_rx_drop_trigger_enum_to_string ( DRV_BBH_RX_DROP_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE ) ) ;
    bdmf_session_print(session, "  bit %lu: %s\n\r" , bit_number ++ , f_rx_drop_trigger_enum_to_string ( DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE ) ) ;
}

static char * f_rx_drop_trigger_enum_to_string ( DRV_BBH_PORT_INDEX xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BBH_RX_DROP_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE:
        return "RX_DROP_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE" ;
        break ;
    case DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE:
        return "RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static char * f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BBH_RX_INTERNAL_UNIT_INPUT_BUFFER:
        return "RX_INTERNAL_UNIT_INPUT_BUFFER" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_BURST_BUFFER:
        return "RX_INTERNAL_UNIT_BURST_BUFFER" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_IH_CONTEXT:
        return "RX_INTERNAL_UNIT_IH_CONTEXT" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_IH_BUFFER_ENABLE:
        return "RX_INTERNAL_UNIT_IH_BUFFER_ENABLE" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_REASSEMBLY_FIFO:
        return "RX_INTERNAL_UNIT_REASSEMBLY_FIFO" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_BPM_FIFO:
        return "RX_INTERNAL_UNIT_BPM_FIFO" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_SBPM_FIFO:
        return "RX_INTERNAL_UNIT_SBPM_FIFO" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_IH_RESPONSE_FIFO:
        return "RX_INTERNAL_UNIT_IH_RESPONSE_FIFO" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_PRE_WAKEUP_FIFO:
        return "RX_INTERNAL_UNIT_PRE_WAKEUP_FIFO" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_REASSEMBLY_CONTEXT_TABLE:
        return "RX_INTERNAL_UNIT_REASSEMBLY_CONTEXT_TABLE" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_DMA_WRITE_POINTER:
        return "RX_INTERNAL_UNIT_DMA_WRITE_POINTER" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_SDMA_WRITE_POINTER:
        return "RX_INTERNAL_UNIT_SDMA_WRITE_POINTER" ;
        break ;
    case DRV_BBH_RX_INTERNAL_UNIT_RUNNER_WRITE_POINTER:
        return "RX_INTERNAL_UNIT_RUNNER_WRITE_POINTER" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static void p_print_rx_internal_unit_enum_help(bdmf_session_handle session)
{
    uint8_t bit_number = 0 ;

    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_INPUT_BUFFER ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_BURST_BUFFER ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_IH_CONTEXT ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_IH_BUFFER_ENABLE ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_REASSEMBLY_FIFO ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_BPM_FIFO ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_SBPM_FIFO ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_IH_RESPONSE_FIFO ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_PRE_WAKEUP_FIFO ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_REASSEMBLY_CONTEXT_TABLE ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_DMA_WRITE_POINTER ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_SDMA_WRITE_POINTER ) ) ;
    bdmf_session_print(session, "  bit %2lu: %s\n\r" , bit_number ++ , f_rx_internal_unit_enum_to_string ( DRV_BBH_RX_INTERNAL_UNIT_RUNNER_WRITE_POINTER ) ) ;
}



#endif
