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

#ifndef _DRV_EPON_XPCSTX_AG_H_
#define _DRV_EPON_XPCSTX_AG_H_

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
    bdmf_boolean cfgenrmtfaultdet125;
    bdmf_boolean cfglsrtristateen125;
    bdmf_boolean cfgenseqnum125;
    bdmf_boolean cfgenscrmbcont125;
    bdmf_boolean cfglsrenacthi125;
    bdmf_boolean cfgenlsralways125;
    bdmf_boolean cfgenlsrtilendslot125;
    bdmf_boolean cfgtxoutbyteflip125;
    bdmf_boolean cfgentxout125;
    bdmf_boolean cfgentxscrb125;
    bdmf_boolean cfgentxfec125;
    bdmf_boolean pcstxnotrdy;
    bdmf_boolean pcstxdtportrstn;
    bdmf_boolean pcstxrstn;
} xpcstx_tx_control;

typedef struct
{
    bdmf_boolean laseronmax;
    bdmf_boolean laseroff;
    bdmf_boolean grantlagerr;
    bdmf_boolean back2backgnt;
    bdmf_boolean fecunderrun;
    bdmf_boolean gearboxunderrun;
    bdmf_boolean gnttooshort;
} xpcstx_tx_int_stat;

typedef struct
{
    bdmf_boolean laseronmaxmask;
    bdmf_boolean laseroffmask;
    bdmf_boolean grantlagerrmsk;
    bdmf_boolean back2bckgntmsk;
    bdmf_boolean fecunderrunmsk;
    bdmf_boolean gearboxunderrunmsk;
    bdmf_boolean gnttooshortmsk;
} xpcstx_tx_int_mask;

bdmf_error_t ag_drv_xpcstx_tx_control_set(const xpcstx_tx_control *tx_control);
bdmf_error_t ag_drv_xpcstx_tx_control_get(xpcstx_tx_control *tx_control);
bdmf_error_t ag_drv_xpcstx_tx_int_stat_set(const xpcstx_tx_int_stat *tx_int_stat);
bdmf_error_t ag_drv_xpcstx_tx_int_stat_get(xpcstx_tx_int_stat *tx_int_stat);
bdmf_error_t ag_drv_xpcstx_tx_int_mask_set(const xpcstx_tx_int_mask *tx_int_mask);
bdmf_error_t ag_drv_xpcstx_tx_int_mask_get(xpcstx_tx_int_mask *tx_int_mask);
bdmf_error_t ag_drv_xpcstx_tx_port_command_set(bdmf_boolean dataportbusy, uint8_t portselect, uint8_t portopcode, uint16_t portaddress);
bdmf_error_t ag_drv_xpcstx_tx_port_command_get(bdmf_boolean *dataportbusy, uint8_t *portselect, uint8_t *portopcode, uint16_t *portaddress);
bdmf_error_t ag_drv_xpcstx_tx_data_port_0_set(uint32_t portdata0);
bdmf_error_t ag_drv_xpcstx_tx_data_port_0_get(uint32_t *portdata0);
bdmf_error_t ag_drv_xpcstx_tx_data_port_1_set(uint32_t portdata1);
bdmf_error_t ag_drv_xpcstx_tx_data_port_1_get(uint32_t *portdata1);
bdmf_error_t ag_drv_xpcstx_tx_data_port_2_set(uint32_t portdata2);
bdmf_error_t ag_drv_xpcstx_tx_data_port_2_get(uint32_t *portdata2);
bdmf_error_t ag_drv_xpcstx_tx_sync_patt_cword_lo_set(uint32_t cfgsyncpatcwl);
bdmf_error_t ag_drv_xpcstx_tx_sync_patt_cword_lo_get(uint32_t *cfgsyncpatcwl);
bdmf_error_t ag_drv_xpcstx_tx_sync_patt_cword_hi_set(uint32_t cfgsyncpatcwh);
bdmf_error_t ag_drv_xpcstx_tx_sync_patt_cword_hi_get(uint32_t *cfgsyncpatcwh);
bdmf_error_t ag_drv_xpcstx_tx_start_burst_del_cword_lo_set(uint32_t cfgstrtbrstdlmtrcwl);
bdmf_error_t ag_drv_xpcstx_tx_start_burst_del_cword_lo_get(uint32_t *cfgstrtbrstdlmtrcwl);
bdmf_error_t ag_drv_xpcstx_tx_start_burst_del_cword_hi_set(uint32_t cfgstrtbrstdlmtrcwh);
bdmf_error_t ag_drv_xpcstx_tx_start_burst_del_cword_hi_get(uint32_t *cfgstrtbrstdlmtrcwh);
bdmf_error_t ag_drv_xpcstx_tx_end_burst_del_cword_lo_set(uint32_t cfgendbrstdlmtrcwl);
bdmf_error_t ag_drv_xpcstx_tx_end_burst_del_cword_lo_get(uint32_t *cfgendbrstdlmtrcwl);
bdmf_error_t ag_drv_xpcstx_tx_end_burst_del_cword_hi_set(uint32_t cfgendbrstdlmtrcwh);
bdmf_error_t ag_drv_xpcstx_tx_end_burst_del_cword_hi_get(uint32_t *cfgendbrstdlmtrcwh);
bdmf_error_t ag_drv_xpcstx_tx_idle_cword_lo_set(uint32_t cfgidlecwl);
bdmf_error_t ag_drv_xpcstx_tx_idle_cword_lo_get(uint32_t *cfgidlecwl);
bdmf_error_t ag_drv_xpcstx_tx_idle_cword_hi_set(uint32_t cfgidlecwh);
bdmf_error_t ag_drv_xpcstx_tx_idle_cword_hi_get(uint32_t *cfgidlecwh);
bdmf_error_t ag_drv_xpcstx_tx_burst_patt_cword_lo_set(uint32_t cfgburstpatcwl);
bdmf_error_t ag_drv_xpcstx_tx_burst_patt_cword_lo_get(uint32_t *cfgburstpatcwl);
bdmf_error_t ag_drv_xpcstx_tx_burst_patt_cword_hi_set(uint32_t cfgburstpatcwh);
bdmf_error_t ag_drv_xpcstx_tx_burst_patt_cword_hi_get(uint32_t *cfgburstpatcwh);
bdmf_error_t ag_drv_xpcstx_tx_laser_time_set(uint8_t cfglaserpipe125, uint8_t cfglaseroffdlytq125, uint8_t cfglaserondlytq125);
bdmf_error_t ag_drv_xpcstx_tx_laser_time_get(uint8_t *cfglaserpipe125, uint8_t *cfglaseroffdlytq125, uint8_t *cfglaserondlytq125);
bdmf_error_t ag_drv_xpcstx_tx_mac_mode_set(bdmf_boolean cfgennogntxmt125);
bdmf_error_t ag_drv_xpcstx_tx_mac_mode_get(bdmf_boolean *cfgennogntxmt125);
bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_ctl_set(bdmf_boolean laserenstatus, bdmf_boolean cfglsrmonacthi, bdmf_boolean lasermonrstn);
bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_ctl_get(bdmf_boolean *laserenstatus, bdmf_boolean *cfglsrmonacthi, bdmf_boolean *lasermonrstn);
bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_max_thresh_set(uint32_t cfglsrmonmaxtq);
bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_max_thresh_get(uint32_t *cfglsrmonmaxtq);
bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_burst_len_get(uint32_t *laseronlength);
bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_burst_count_get(uint32_t *burstcnt);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xpcstx_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

