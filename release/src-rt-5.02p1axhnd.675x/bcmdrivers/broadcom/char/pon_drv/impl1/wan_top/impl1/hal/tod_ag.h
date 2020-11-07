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

#ifndef _BCM6858_TOD_AG_H_
#define _BCM6858_TOD_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif

/**************************************************************************************************/
/* cfg_tod_pps_clear:  - Allow 1PPS pulse to clear the counter if set.  If not set, the 1PPSpulse */
/*                     will have no effect on the TS48.                                           */
/* cfg_ts48_read:  - The TS48 will be captured on the rising edge of this signal.                 */
/* cfg_ts48_offset:  - The lower 10-bits of the timestamp, to be applied aftersynchronizing to th */
/*                  e 250 MHz domain.                                                             */
/* cfg_ts48_enable:  - All TS48 config signals will be sampled on the rising edge of thissignal.  */
/*                   To change any of the other configuration fields, softwaremust clear and asse */
/*                  rt this bit again.                                                            */
/* cfg_ts48_mac_select:  - This field selects the MAC that the timestamp comes from.0: EPON1: 10G */
/*                       EPON2: GPON3: NGPON4: Active Ethernet5-7: Reserved                       */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean cfg_tod_pps_clear;
    bdmf_boolean cfg_ts48_read;
    uint16_t cfg_ts48_offset;
    bdmf_boolean cfg_ts48_enable;
    uint8_t cfg_ts48_mac_select;
} tod_config_0;

bdmf_error_t ag_drv_tod_config_0_set(const tod_config_0 *config_0);
bdmf_error_t ag_drv_tod_config_0_get(tod_config_0 *config_0);
bdmf_error_t ag_drv_tod_config_1_set(bdmf_boolean cfg_tod_load, uint32_t cfg_tod_seconds);
bdmf_error_t ag_drv_tod_config_1_get(bdmf_boolean *cfg_tod_load, uint32_t *cfg_tod_seconds);
bdmf_error_t ag_drv_tod_msb_get(uint16_t *ts48_wan_read_msb);
bdmf_error_t ag_drv_tod_lsb_get(uint32_t *ts48_wan_read_lsb);
bdmf_error_t ag_drv_tod_config_2_set(uint16_t cfg_tx_offset, uint16_t cfg_rx_offset);
bdmf_error_t ag_drv_tod_config_2_get(uint16_t *cfg_tx_offset, uint16_t *cfg_rx_offset);
bdmf_error_t ag_drv_tod_config_3_set(uint16_t cfg_ref_offset);
bdmf_error_t ag_drv_tod_config_3_get(uint16_t *cfg_ref_offset);
bdmf_error_t ag_drv_tod_status_0_get(uint16_t *ts16_tx_read, uint16_t *ts16_rx_read);
bdmf_error_t ag_drv_tod_status_1_get(uint16_t *ts16_rx_synce_read, uint16_t *ts16_ref_synce_read);
bdmf_error_t ag_drv_tod_status_2_get(uint16_t *ts16_mac_tx_read, uint16_t *ts16_mac_rx_read);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_tod_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

