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
    bdmf_boolean xpcsrxrst_n;
    bdmf_boolean xpcstxrst_n;
    bdmf_boolean xifrst_n;
    bdmf_boolean clkprgrst_n;
    bdmf_boolean ncorst_n;
    bdmf_boolean lifrst_n;
    bdmf_boolean epnrst_n;
} epon_top_reset;

typedef struct
{
    bdmf_boolean int_1pps;
    bdmf_boolean int_xpcs_tx;
    bdmf_boolean int_xpcs_rx;
    bdmf_boolean int_xif;
    bdmf_boolean int_nco;
    bdmf_boolean int_lif;
    bdmf_boolean int_epn;
} epon_top_interrupt;

typedef struct
{
    bdmf_boolean int_1pps_mask;
    bdmf_boolean int_xpcs_tx_mask;
    bdmf_boolean int_xpcs_rx_mask;
    bdmf_boolean int_xif_mask;
    bdmf_boolean int_nco_mask;
    bdmf_boolean int_lif_mask;
    bdmf_boolean int_epn_mask;
} epon_top_interrupt_mask;

bdmf_error_t ag_drv_epon_top_scratch_set(uint32_t scratch);
bdmf_error_t ag_drv_epon_top_scratch_get(uint32_t *scratch);
bdmf_error_t ag_drv_epon_top_reset_set(const epon_top_reset *reset);
bdmf_error_t ag_drv_epon_top_reset_get(epon_top_reset *reset);
bdmf_error_t ag_drv_epon_top_interrupt_set(const epon_top_interrupt *interrupt);
bdmf_error_t ag_drv_epon_top_interrupt_get(epon_top_interrupt *interrupt);
bdmf_error_t ag_drv_epon_top_interrupt_mask_set(const epon_top_interrupt_mask *interrupt_mask);
bdmf_error_t ag_drv_epon_top_interrupt_mask_get(epon_top_interrupt_mask *interrupt_mask);
bdmf_error_t ag_drv_epon_top_control_set(bdmf_boolean cfgtwogigpondns, bdmf_boolean cfgtengigponup, bdmf_boolean cfgtengigdns);
bdmf_error_t ag_drv_epon_top_control_get(bdmf_boolean *cfgtwogigpondns, bdmf_boolean *cfgtengigponup, bdmf_boolean *cfgtengigdns);
bdmf_error_t ag_drv_epon_top_one_pps_mpcp_offset_set(uint32_t cfg_1pps_mpcp_offset);
bdmf_error_t ag_drv_epon_top_one_pps_mpcp_offset_get(uint32_t *cfg_1pps_mpcp_offset);
bdmf_error_t ag_drv_epon_top_one_pps_captured_mpcp_time_get(uint32_t *capture_1pps_mpcp_time);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_epon_top_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

