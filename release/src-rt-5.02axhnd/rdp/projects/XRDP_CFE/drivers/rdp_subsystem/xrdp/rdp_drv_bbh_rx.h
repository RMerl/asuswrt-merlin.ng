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

#ifndef DRV_BBH_RX_H_INCLUDED
#define DRV_BBH_RX_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "xrdp_drv_bbh_rx_ag.h"

#define DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0 32
#define DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1 33
#define DRV_BBH_RX_MAXIMUM_OTF_SBPM_REQUESTS_DEFAULT_VAL 0xF

typedef struct
{
    bdmf_boolean ploam_en;
    bdmf_boolean user_prio3_en;
    bdmf_boolean pause_en;
    bdmf_boolean pfc_en;
    bdmf_boolean ctrl_en;
    bdmf_boolean exc_en;
    bdmf_boolean pattern_en;
} excl_cfg_t;

typedef struct
{
    uint32_t timer;
    bbh_rx_flow_ctrl_drops_config drops;
} flow_xtrl_cfg_t;

typedef struct
{
    uint8_t first_chunk_idx;
    uint8_t sdma_chunks;
    uint8_t sdma_bb_id;
} bbh_rx_sdma_chunks_cfg_t;

typedef struct
{
    bdmf_boolean is_epon;
    bdmf_boolean is_xgpon;
    bdmf_boolean is_vdsl;
} bbh_rx_mac_mode_cfg_t;

typedef struct
{
    uint8_t disp_bb_id;
    uint8_t sbpm_bb_id;
    uint8_t normal_viq;
    uint8_t excl_viq;
    uint8_t sop_offset;
    uint8_t per_flow_th;
    uint8_t max_otf_sbpm;
    uint8_t min_pkt_size[4];
    uint16_t max_pkt_size[4];
    bdmf_boolean crc_omit_dis;
    bdmf_boolean gint_en;
    bbh_rx_mac_mode_cfg_t mac_mode_cfg;
    bdmf_boolean flow_ctrl_swap;
    flow_xtrl_cfg_t flow_ctrl_cfg;
    bbh_rx_sdma_chunks_cfg_t *sdma_chunks_cfg;
    bbh_rx_pattern_recog pattern_recog;
    excl_cfg_t excl_cfg;
} bbh_rx_config;

typedef struct
{
    bbh_rx_error_pm_counters error_counter;
    bbh_rx_pm_counters pm_counters;
} bbh_rx_counters_t;


int drv_bbh_rx_configuration_set(uint8_t bbh_id, bbh_rx_config *config);
int drv_bbh_rx_configuration_get(uint8_t bbh_id, bbh_rx_config *config);
void drv_bbh_rx_get_stat(uint8_t bbh_id, bbh_rx_counters_t *counters);
int drv_bbh_rx_pkt_size_set(uint8_t bbh_id, int sel, uint8_t min_pkt_size, uint16_t max_pkt_size);
int drv_bbh_rx_pkt_size_get(uint8_t bbh_id, int sel, uint8_t *min_pkt_size, uint16_t *max_pkt_size);
int drv_bbh_rx_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val);

#ifdef USE_BDMF_SHELL
int drv_bbh_rx_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_bbh_rx_cli_init(bdmfmon_handle_t driver_dir);
void drv_bbh_rx_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#ifdef __cplusplus
}
#endif

#endif
