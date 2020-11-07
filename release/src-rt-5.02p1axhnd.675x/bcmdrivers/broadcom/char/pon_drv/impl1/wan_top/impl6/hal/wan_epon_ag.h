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

#ifndef _WAN_EPON_AG_H_
#define _WAN_EPON_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    bdmf_boolean cfg_sgb_pon_10g_epon_rx_cgen_rstn;
    bdmf_boolean cfg_sgb_pon_10g_epon_tx_cgen_rstn;
    bdmf_boolean cfg_sgb_pon_10g_epon_rx_gbox_rstn;
    bdmf_boolean cfg_sgb_pon_10g_epon_tx_gbox_rstn;
    bdmf_boolean cfg_sgb_pon_10g_epon_clk_en;
    bdmf_boolean cfg_sgb_pon_10g_epon_rx_data_end;
    bdmf_boolean cfg_sgb_pon_10g_epon_tx2rx_loop_en;
    bdmf_boolean cfg_sgb_pon_10g_epon_tx_fifo_off_ld;
    uint8_t cfg_sgb_pon_10g_epon_tx_fifo_off;
} wan_epon_wan_epon_10g_gearbox;

bdmf_error_t ag_drv_wan_epon_wan_epon_10g_gearbox_set(const wan_epon_wan_epon_10g_gearbox *wan_epon_10g_gearbox);
bdmf_error_t ag_drv_wan_epon_wan_epon_10g_gearbox_get(wan_epon_wan_epon_10g_gearbox *wan_epon_10g_gearbox);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_wan_epon_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

