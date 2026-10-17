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

#ifndef DRV_DIS_REOR_H_INCLUDED
#define DRV_DIS_REOR_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "rdp_drv_bbh_rx.h"
#include "xrdp_autogens.h"
#define RUNNER_GROUP_MAX_NUM       8
#define RUNNER_MAX_TASKS_NUM       16

#define DSPTCHR_WAKEUP_THRS        8

typedef struct
{
    dsptchr_cngs_params ingress_cngs;
    dsptchr_cngs_params egress_cngs;
    uint8_t             wakeup_thrs;
    uint16_t            bb_id;
    uint16_t            bbh_target_address;
    uint8_t             queue_dest; /* 0-disp, 1-reor */
    uint8_t             delayed_queue; /* 0-non delayed, 1-delayed */
    uint16_t            common_max_limit;
    uint16_t            guaranteed_max_limit;
    bdmf_boolean        coherency_en;
    uint16_t            coherency_ctr;
} dsptchr_ingress_queue;

typedef struct
{
    uint16_t base_add;
    uint16_t offset_add;
} dsptchr_rnr_dsptch_addr;

typedef enum
{
    dsptchr_viq_dest_disp = 0,
    dsptchr_viq_dest_reor = 1
} dsptchr_viq_dest_t;

typedef enum
{
    dsptchr_viq_bbh_target_addr_normal = 2,
    dsptchr_viq_bbh_target_addr_excl = 3
} dsptchr_viq_bbh_target_addr_t;

typedef struct
{
    uint8_t                 rnr_num;
    dsptchr_rnr_dsptch_addr rnr_addr_cfg[2];
} dsptchr_rnr_addr_cfg;

typedef struct
{
    dsptchr_pools_limits        pools_limits[NUM_OF_DISPATCHERS];
    dsptchr_cngs_params         cngs_params;
    dsptchr_glbl_cngs_params    glbl_congs_init;
    dsptchr_fll_entry           fll_entry[NUM_OF_DISPATCHERS];
    uint8_t                     disp_num_of_credits;
    dsptchr_ingress_queue       dsptchr_viq_list[NUM_OF_DISPATCHERS][NUM_OF_BBH_RX_VIQS];
    runner_task_group_idx_e     rnr_group_idx[NUM_OF_DISPATCHERS][NUM_OF_BBH_RX_VIQS];
    dsptchr_dsptcher_reordr_top_intr_ctrl_0_isr dsptcher_reordr_top_intr_ctrl_0_isr;
    uint8_t                     rg_available_tasks[RUNNER_TASK_GROUP_NUM];
    uint16_t                    total_viq_guaranteed_buf[NUM_OF_DISPATCHERS];
} dsptchr_config;


bdmf_error_t drv_dispatcher_reorder_viq_status(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t nParms);
bdmf_error_t drv_dispatcher_reorder_available_tasks(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t nParms);
bdmf_error_t drv_dispatcher_init (uint8_t dsptchr_index, uint8_t num_viqs, dsptchr_config* cfg);
bdmf_error_t drv_dispatcher_cmn_init(dsptchr_config* cfg, uint16_t pd_base_addr, uint16_t pd_offset_addr);
bdmf_error_t drv_dispatcher_chan_viq_cfg_set(int dsptchr_chan_index, uint8_t viq_index, dsptchr_config* dsptchr_cfg, bdmf_boolean reset);


void drv_dispatcher_reorder_viq_init(uint8_t dsptchr_index, dsptchr_config *cfg, dsptchr_cngs_params *ingress_congs_init,
    dsptchr_cngs_params *egress_congs_init, uint8_t bb_id, uint32_t target_address,
    bdmf_boolean dest, uint8_t viq_num, uint8_t guaranteed_limit,
    uint16_t common_max_limit, bdmf_boolean is_bbh_queue, runner_task_group_idx_e rnr_group_idx);

int dispatcher_reorder_init(int dsptchr_index, int num_viqs);

#ifdef __cplusplus
}
#endif

#endif
