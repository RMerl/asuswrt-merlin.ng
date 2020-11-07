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

#ifndef _GPON_AG_H_
#define _GPON_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    uint8_t sw_reset_txpg_reset;
    uint8_t sw_reset_txfifo_reset;
    uint8_t fifo_cfg_0_clear_txfifo_drifted;
    uint8_t fifo_cfg_0_loopback_rx;
    uint8_t fifo_cfg_0_clear_txfifo_collision;
    uint8_t fifo_cfg_0_tx_wr_ptr_adv;
    uint8_t fifo_cfg_0_tx_wr_ptr_dly;
    uint8_t fifo_cfg_0_tx_bit_inv;
    uint8_t fifo_cfg_0_tx_ptr_dist_max;
    uint8_t fifo_cfg_0_asym_loopback;
    uint8_t fifo_cfg_0_tx_ptr_dist_min;
    uint8_t fifo_cfg_0_tx_20bit_order;
    uint8_t fifo_cfg_0_tx_8bit_order;
    uint8_t fifo_cfg_0_rx_16bit_order;
    uint8_t fifo_cfg_0_txlbe_bit_order;
    uint8_t fifo_status_sel;
    uint8_t ptg_status1_sel;
    uint8_t ptg_status2_sel;
} gpon_gearbox_0;

typedef struct
{
    uint8_t pg_mode;
    uint8_t header;
    uint8_t payload;
    uint8_t filler;
} gpon_gearbox_pattern_cfg1;

typedef struct
{
    uint8_t fifo_cfg_1_tx_rd_pointer;
    uint8_t fifo_cfg_1_tx_wr_pointer;
    uint8_t tx_vld_delay_cyc;
    uint8_t config_burst_delay_cyc;
} gpon_gearbox_2;

int ag_drv_gpon_gearbox_0_set(const gpon_gearbox_0 *gearbox_0);
int ag_drv_gpon_gearbox_0_get(gpon_gearbox_0 *gearbox_0);
int ag_drv_gpon_gearbox_pattern_cfg1_set(const gpon_gearbox_pattern_cfg1 *gearbox_pattern_cfg1);
int ag_drv_gpon_gearbox_pattern_cfg1_get(gpon_gearbox_pattern_cfg1 *gearbox_pattern_cfg1);
int ag_drv_gpon_gearbox_pattern_cfg2_set(uint8_t gap_size, uint8_t burst_size);
int ag_drv_gpon_gearbox_pattern_cfg2_get(uint8_t *gap_size, uint8_t *burst_size);
int ag_drv_gpon_gearbox_2_set(const gpon_gearbox_2 *gearbox_2);
int ag_drv_gpon_gearbox_2_get(gpon_gearbox_2 *gearbox_2);
int ag_drv_gpon_gearbox_status_get(uint32_t *cr_rd_data_clx);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_gpon_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

