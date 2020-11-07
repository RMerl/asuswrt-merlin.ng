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

#ifndef _BCM6858_TEN_G_GEARBOX_AG_H_
#define _BCM6858_TEN_G_GEARBOX_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif

/**************************************************************************************************/
/* cfg_sgb_pon_10g_epon_tx_fifo_off:  - TX FIFO offset value.                                     */
/* cfg_sgb_pon_10g_epon_tx_fifo_off_ld:  - 0: Normal operation. 1: Load new TX FIFO offset.       */
/* cfg_sgb_pon_10g_epon_tx2rx_loop_en:  - 0: Normal operation. 1: Enable TX to RX loopback.       */
/* cfg_sgb_pon_10g_epon_rx_data_end:  - 0: Normal operation. 1: Enable endian flip.               */
/* cfg_sgb_pon_10g_epon_clk_en:  - 0: Disable clock. 1: Enable clock.                             */
/* cfg_sgb_pon_10g_epon_tx_gbox_rstn:  - 0: Reset TX gearbox logic. 1: Enable TX gearbox logic.   */
/* cfg_sgb_pon_10g_epon_rx_gbox_rstn:  - 0: Reset RX gearbox logic. 1: Enable RX gearbox logic.   */
/* cfg_sgb_pon_10g_epon_tx_cgen_rstn:  - 0: Reset TX clock generator. 1: Enable TX clock generato */
/*                                    r.                                                          */
/* cfg_sgb_pon_10g_epon_rx_cgen_rstn:  - 0: Reset RX clock generator. 1: Enable RX clock generato */
/*                                    r.                                                          */
/**************************************************************************************************/
typedef struct
{
    uint8_t cfg_sgb_pon_10g_epon_tx_fifo_off;
    bdmf_boolean cfg_sgb_pon_10g_epon_tx_fifo_off_ld;
    bdmf_boolean cfg_sgb_pon_10g_epon_tx2rx_loop_en;
    bdmf_boolean cfg_sgb_pon_10g_epon_rx_data_end;
    bdmf_boolean cfg_sgb_pon_10g_epon_clk_en;
    bdmf_boolean cfg_sgb_pon_10g_epon_tx_gbox_rstn;
    bdmf_boolean cfg_sgb_pon_10g_epon_rx_gbox_rstn;
    bdmf_boolean cfg_sgb_pon_10g_epon_tx_cgen_rstn;
    bdmf_boolean cfg_sgb_pon_10g_epon_rx_cgen_rstn;
} ten_g_gearbox_gearbox;

bdmf_error_t ag_drv_ten_g_gearbox_gearbox_set(const ten_g_gearbox_gearbox *gearbox);
bdmf_error_t ag_drv_ten_g_gearbox_gearbox_get(ten_g_gearbox_gearbox *gearbox);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_ten_g_gearbox_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

