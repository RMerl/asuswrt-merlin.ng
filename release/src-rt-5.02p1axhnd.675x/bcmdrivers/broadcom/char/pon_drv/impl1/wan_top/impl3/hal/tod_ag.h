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

#ifndef _TOD_AG_H_
#define _TOD_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

typedef struct
{
    uint8_t cfg_ts48_read;
    uint16_t cfg_ts48_offset;
    uint8_t cfg_ts48_enable;
    uint8_t cfg_ts48_mac_select;
} tod_config_0;

int ag_drv_tod_config_0_set(const tod_config_0 *config_0);
int ag_drv_tod_config_0_get(tod_config_0 *config_0);
int ag_drv_tod_msb_get(uint16_t *ts48_wan_read_msb);
int ag_drv_tod_lsb_get(uint32_t *ts48_wan_read_lsb);
int ag_drv_tod_config_2_set(uint16_t cfg_tx_offset, uint16_t cfg_rx_offset);
int ag_drv_tod_config_2_get(uint16_t *cfg_tx_offset, uint16_t *cfg_rx_offset);
int ag_drv_tod_config_3_set(uint8_t cfg_ts48_fifo_dis, uint8_t cfg_ts48_fifo_ld_rate, uint16_t cfg_ref_offset);
int ag_drv_tod_config_3_get(uint8_t *cfg_ts48_fifo_dis, uint8_t *cfg_ts48_fifo_ld_rate, uint16_t *cfg_ref_offset);
int ag_drv_tod_status_0_get(uint16_t *ts16_tx_read, uint16_t *ts16_rx_read);
int ag_drv_tod_status_1_get(uint16_t *ts16_rx_synce_read, uint16_t *ts16_ref_synce_read);
int ag_drv_tod_status_2_get(uint16_t *ts16_mac_tx_read, uint16_t *ts16_mac_rx_read);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_tod_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

