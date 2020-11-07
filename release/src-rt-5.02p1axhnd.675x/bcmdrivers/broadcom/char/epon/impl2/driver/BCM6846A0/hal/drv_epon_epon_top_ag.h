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

#ifndef _DRV_EPON_EPON_TOP_AG_H_
#define _DRV_EPON_EPON_TOP_AG_H_

#include "access_macros.h"
#if !defined(_CFE_)
#include "bdmf_interface.h"
#else
#include "bdmf_data_types.h"
#include "bdmf_errno.h"
#endif
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
typedef struct
{
    bdmf_boolean todrst_n;
    bdmf_boolean clkprgrst_n;
    bdmf_boolean ncorst_n;
    bdmf_boolean lifrst_n;
    bdmf_boolean epnrst_n;
} epon_top_reset;

typedef struct
{
    bdmf_boolean cfg_tod_load_ns;
    bdmf_boolean cfg_tod_read;
    uint8_t cfg_tod_read_sel;
    bdmf_boolean cfg_tod_pps_clear;
    bdmf_boolean cfg_tod_load;
    uint32_t cfg_tod_seconds;
} epon_top_tod_config;

bdmf_error_t ag_drv_epon_top_scratch_set(uint32_t scratch);
bdmf_error_t ag_drv_epon_top_scratch_get(uint32_t *scratch);
bdmf_error_t ag_drv_epon_top_reset_set(const epon_top_reset *reset);
bdmf_error_t ag_drv_epon_top_reset_get(epon_top_reset *reset);
bdmf_error_t ag_drv_epon_top_interrupt_set(bdmf_boolean int_1pps, bdmf_boolean int_nco, bdmf_boolean int_lif, bdmf_boolean int_epn);
bdmf_error_t ag_drv_epon_top_interrupt_get(bdmf_boolean *int_1pps, bdmf_boolean *int_nco, bdmf_boolean *int_lif, bdmf_boolean *int_epn);
bdmf_error_t ag_drv_epon_top_interrupt_mask_set(bdmf_boolean int_1pps_mask, bdmf_boolean int_nco_mask, bdmf_boolean int_lif_mask, bdmf_boolean int_epn_mask);
bdmf_error_t ag_drv_epon_top_interrupt_mask_get(bdmf_boolean *int_1pps_mask, bdmf_boolean *int_nco_mask, bdmf_boolean *int_lif_mask, bdmf_boolean *int_epn_mask);
bdmf_error_t ag_drv_epon_top_control_set(bdmf_boolean cfgtwogigpondns);
bdmf_error_t ag_drv_epon_top_control_get(bdmf_boolean *cfgtwogigpondns);
bdmf_error_t ag_drv_epon_top_one_pps_mpcp_offset_set(uint32_t cfg_1pps_mpcp_offset);
bdmf_error_t ag_drv_epon_top_one_pps_mpcp_offset_get(uint32_t *cfg_1pps_mpcp_offset);
bdmf_error_t ag_drv_epon_top_one_pps_captured_mpcp_time_get(uint32_t *capture_1pps_mpcp_time);
bdmf_error_t ag_drv_epon_top_tod_config_set(const epon_top_tod_config *tod_config);
bdmf_error_t ag_drv_epon_top_tod_config_get(epon_top_tod_config *tod_config);
bdmf_error_t ag_drv_epon_top_tod_ns_set(uint32_t cfg_tod_ns);
bdmf_error_t ag_drv_epon_top_tod_ns_get(uint32_t *cfg_tod_ns);
bdmf_error_t ag_drv_epon_top_tod_mpcp_set(uint32_t cfg_tod_mpcp);
bdmf_error_t ag_drv_epon_top_tod_mpcp_get(uint32_t *cfg_tod_mpcp);
bdmf_error_t ag_drv_epon_top_ts48_msb_get(uint16_t *ts48_epon_read_msb);
bdmf_error_t ag_drv_epon_top_ts48_lsb_get(uint32_t *ts48_epon_read_lsb);
bdmf_error_t ag_drv_epon_top_tsec_get(uint32_t *tsec_epon_read);
bdmf_error_t ag_drv_epon_top_tns_epon_get(uint32_t *tns_epon_read);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_epon_top_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

