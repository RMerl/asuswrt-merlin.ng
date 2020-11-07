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

#ifndef _SERDES_STATUS_AG_H_
#define _SERDES_STATUS_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    bdmf_boolean pmd_pll0_lock;
    bdmf_boolean pmd_rx_lock_0_invert;
    bdmf_boolean pmd_rx_clk_vld_0;
    bdmf_boolean pmd_tx_clk_vld;
    bdmf_boolean pmd_rx_lock_0;
    bdmf_boolean pmd_energy_detect_0;
    bdmf_boolean pmd_signal_detect_0;
    bdmf_boolean pmi_lp_acknowledge;
    bdmf_boolean pmi_lp_error;
    bdmf_boolean o_laser_burst_en;
    bdmf_boolean pmd_pll1_lock;
} serdes_status_serdes_status_status;

bdmf_error_t ag_drv_serdes_status_serdes_status_status_get(serdes_status_serdes_status_status *serdes_status_status);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_serdes_status_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

