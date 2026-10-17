/*
    <:copyright-BRCM:2015:DUAL/GPL:standard

       Copyright (c) 2015 Broadcom
       All Rights Reserved

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

#include "rdp_subsystem_common.h"
#include "rdp_drv_cnpl.h"
#include "rdp_common.h"
#include "rdd_defs.h"
#include "XRDP_AG.h"
#include "rdp_drv_cntr.h"
#include "rdp_drv_xpm.h"
#include "xrdp_drv_cnpl_ag.h"
#include "bdmf_dev.h"

DEFINE_BDMF_FASTLOCK(counter_read_lock);

/*
    counter configuration example:
        call the function: ag_drv_cnpl_counter_cfg_set to set up a new counter group
            cnt_loc_profile: group number (0-15)
            cn_double: set each counter to be single or double (boolean)
            cn0_byts: number of bytes for each counter (1, 2, 4)
            ba: base address for the counter group (10bit in 8B resolution)
            wrap: freeze or wrap around (boolean)
            clr: clear on read (boolean)

    counter read example:
        1. send read command by calling the function: drv_cnpl_counter_read_command_set
            group: choose the counter group (4bit)
            cntr: choose start counter (14bit)
            size: number of counters to read (8bit)
        2. check status register: ag_drv_cnpl_sw_stat_get
            cn_rd_st: counter read status (0, 1, 2)
        3. read the result from: ag_drv_cnpl_block_sw_if_sw_cnt_rd_get
            rd_idx: read bytes rd_idx*4--rd_idx*4+3 (0-7)
*/

/*
    policer configuration example:
        1. memory reset
        2. call the function: ag_drv_cnpl_policer_cfg_set to set up a new policer
            conf_idx: profile number (0, 1)
            bk_ba: bucket base address (10bit in 8B resolution)
            pa_ba: parameters base address (10bit)
            pl_double: set each policer to be single or double (boolean)
            pl_st: first policer in the group (8bit)
            pl_end: last policer in the group (8bit)
        3. call the function: ag_drv_cnpl_block_policers_configurations_per_up_set to start periodic update
            en: periodic update enable (boolean)
            N: period in 8k cycles quanta (8bit)

    policer read example:
        1. send read command by calling the function: drv_cnpl_policer_read_command_set
            group: choose the policer group (2bit)
            policer_num: choose start policer (8bit)
            reset_after_read: set reset (boolean)
        2. check status register: ag_drv_cnpl_sw_stat_get
            pl_rd_st: policer read status (0, 1, 2)
        3. read the result from: ag_drv_cnpl_block_sw_if_sw_pl_rd_get
            bucket: read bytes bucket*4--bucket*4+3 (0-7)

    policer police example:
        1. send police command by calling the function: drv_cnpl_policer_police_command_set
            group: choose the policer group (2bit)
            policer_num: choose start policer (8bit)
            packet_len: packet length (16bit)
        2. check status register: ag_drv_cnpl_sw_stat_get
            pl_plc_st: policer police status (0, 1, 2)
        3. read the color from: ag_drv_cnpl_block_sw_if_sw_pl_rslt_get
*/

/* cpu_port enum values */
bdmf_attr_enum_table_t bufmng_enum_table = {
    .values = {
        {"CPU RX     ", BUF_MNG_CPU_RX_CNTR},
        {"CPU RX HIGH", BUF_MNG_CPU_RX_HIGH_CNTR},
        {"WLAN_0     ", BUF_MNG_WLAN_0_CNTR},
        {"WLAN_0 HIGH", BUF_MNG_WLAN_HIGH_0_CNTR},
        {"WLAN_0 RX  ", BUF_MNG_WLAN_RX_0_CNTR},
        {"WLAN_1     ", BUF_MNG_WLAN_1_CNTR},
        {"WLAN_1 HIGH", BUF_MNG_WLAN_HIGH_1_CNTR},
        {"WLAN_1 RX  ", BUF_MNG_WLAN_RX_1_CNTR},
        {"WLAN_2     ", BUF_MNG_WLAN_2_CNTR},
        {"WLAN_2 HIGH", BUF_MNG_WLAN_HIGH_2_CNTR},
        {"WLAN_2 RX  ", BUF_MNG_WLAN_RX_2_CNTR},
        {"WLAN_3     ", BUF_MNG_WLAN_3_CNTR},
        {"WLAN_3 HIGH", BUF_MNG_WLAN_HIGH_3_CNTR},
        {"WLAN_3 RX  ", BUF_MNG_WLAN_RX_3_CNTR},
        {"WLAN GROUP ", BUF_MNG_WLAN_TOTAL_CNTR},
        {"BE1 GROUP  ", BUF_MNG_BE1_GROUP_CNTR},
        {"BE2 GROUP  ", BUF_MNG_BE2_GROUP_CNTR},
        {"WIRED GROUP", BUF_MNG_WIRED_GROUP_CNTR},
        {"ALL PASS   ", BUF_MNG_ALL_PASS_CNTR},
        {"TOTAL      ", BUF_MNG_TOTAL_BUDGET_CNTR},
        {NULL,      0}
    }
};

bdmf_error_t drv_cnpl_counter_clr(uint8_t group, uint32_t cntr_id)
{
    return 0;
}

bdmf_error_t drv_cnpl_counter_set(uint8_t group, uint32_t cntr_id, uint32_t value)
{
    return 0;
}

bdmf_error_t drv_cnpl_counter_read(void *counters, uint8_t group, uint16_t start_counter, uint8_t num_of_counters)
{
    return 0;
}

bdmf_error_t drv_cnpl_counter_read_command_set(uint8_t group, uint16_t start_counter, uint8_t num_of_counters)
{
    return 0;
}

bdmf_error_t drv_cnpl_counter_read_command_get(uint32_t *counters, uint8_t num_of_counters, bdmf_boolean cn_double, uint8_t cn0_byts, bdmf_boolean is_start_counter_odd)
{
    return 0;
}

bdmf_error_t drv_cnpl_policer_police_command_set(uint8_t group, uint8_t policer_num, uint16_t packet_len)
{
    return 0;
}

bdmf_error_t drv_cnpl_policer_read_command_set(uint8_t group, uint8_t policer_num, bdmf_boolean reset_after_read)
{
    return 0;
}

uint8_t drv_cnpl_periodic_update_us_to_n_get(uint32_t microseconds)
{
    return 0;
}

bdmf_error_t drv_cnpl_memory_data_init()
{
    return 0;
}

int drv_cnpl_cli_bufmng_sanity_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return 0;
}

#ifdef USE_BDMF_SHELL

int drv_cnpl_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return 0;
}


int drv_cnpl_cli_bufmng_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return 0;
}

void drv_cnpl_cli_init(bdmfmon_handle_t driver_dir)
{
    
}

void drv_cnpl_cli_exit(bdmfmon_handle_t driver_dir)
{
 
}

#endif /* USE_BDMF_SHELL */


