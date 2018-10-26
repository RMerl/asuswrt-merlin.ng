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

#include "xrdp_drv_dsptchr_ag.h"
#include "rdd_data_structures_auto.h"
#include "rdd_runner_proj_defs.h"

#define DSPTCHR_VIRTUAL_QUEUE_NUM  32
#define RUNNER_GROUP_MAX_TASK_NUM  8
#define RUNNER_GROUP_MAX_NUM       8
#define RUNNER_MAX_TASKS_NUM       16
#define DSPTCHR_WAKEUP_THRS        8
#define DSPTCHR_NORMAL_GUARANTEED_BUFFERS 16
#define DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS 4
#define RUNNER_GROUP_MAX_TASKS_NUM 256
#define TASK_REGS_NUM              (RUNNER_GROUP_MAX_TASKS_NUM / 32)

#define DISP_REOR_CREDIT_CFG_TARGET_ADDRESS(ram_target_address, thread_num) ((ram_target_address >> 3) | (thread_num << 12))

typedef enum
{
    dsptchr_viq_dest_disp = 0,
    dsptchr_viq_dest_reor = 1
} dsptchr_viq_dest_t;

typedef enum
{
    dsptchr_viq_non_delayed = 0,
    dsptchr_viq_delayed = 1
} dsptchr_viq_delayed_t;

typedef enum
{
    dsptchr_viq_bbh_target_addr_normal = 2,
    dsptchr_viq_bbh_target_addr_excl = 3
} dsptchr_viq_bbh_target_addr_t;

typedef struct
{
    dsptchr_mask_msk_tsk_255_0	tasks_mask;
	uint32_t			        queues_mask;
} dsptchr_runner_group;

typedef struct
{
	dsptchr_cngs_params		ingress_cngs;
	dsptchr_cngs_params 	egress_cngs;
	uint8_t 				wakeup_thrs;
	bbh_id_e				bb_id;
	uint16_t				bbh_target_address;
	uint8_t 				queue_dest; /* 0-disp, 1-reor */
	uint8_t 				delayed_queue; /* 0-non delayed, 1-delayed */
	uint16_t 				common_max_limit;
	uint16_t 				guaranteed_max_limit;
	bdmf_boolean			coherency_en;
	uint16_t				coherency_ctr;
} dsptchr_ingress_queue;

typedef struct
{
	uint8_t					rnr_num;
	dsptchr_rnr_dsptch_addr	rnr_addr_cfg;
} dsptchr_rnr_addr_cfg;

typedef struct
{
	dsptchr_pools_limits	pools_limits;
#ifdef BCM63158
	uint8_t 				viq_num;
#endif
	dsptchr_ingress_queue   dsptchr_viq_list[DSPTCHR_VIRTUAL_QUEUE_NUM];
	uint8_t 				rnr_grp_num;
	dsptchr_runner_group  	dsptchr_rnr_group_list[RUNNER_GROUP_MAX_NUM];
	uint16_t 				rnr_disp_en_vector;
	dsptchr_rnr_addr_cfg	dsptchr_rnr_cfg[NUM_OF_RUNNER_CORES];
	uint8_t	                rg_available_tasks[RUNNER_GROUP_MAX_TASK_NUM];
	uint32_t 				queue_en_vec;
	dsptchr_glbl_cngs_params glbl_congs_init;
	dsptchr_glbl_cngs_params glbl_egress_congs_init;
	uint16_t                total_viq_guaranteed_buf;
} dsptchr_config;

typedef struct
{
    bdmf_boolean is_isolate;
    uint32_t queue_mask;
    dsptchr_mask_msk_tsk_255_0 task_mask;
} dsptchr_isolate_restore_t;

int  drv_dis_reor_queues_init(void);
int  drv_dis_reor_tasks_to_rg_init(void);
int  drv_dis_reor_free_linked_list_init(void); /* buffers linked list initialization */
int  drv_dis_reor_runner_group_cfg(uint8_t grp_idx, dsptchr_runner_group * rnr_grp);
int  drv_dis_reor_viq_cfg(uint8_t q_idx, dsptchr_ingress_queue * viq);
int  drv_dis_reor_cfg(dsptchr_config * cfg);
int  drv_dis_reor_viq_queue_mapping_modify(rdd_disp_reor_viq viq, uint16_t new_target_address, uint8_t new_bb_id);

int drv_dis_reor_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val);

#ifdef USE_BDMF_SHELL
void drv_dis_reor_cli_init(bdmfmon_handle_t driver_dir);
void drv_dis_reor_cli_exit(bdmfmon_handle_t driver_dir);
int drv_dis_reor_restore_isolate_set(uint8_t rnr_grp_idx);
int drv_dis_reor_isolate_set(uint8_t core_idx, uint8_t task_idx, uint8_t rnr_grp_idx);
#endif

#ifdef __cplusplus
}
#endif

#endif
