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


#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "bdmf_errno.h"
#include "rdd_defs.h"
#include "rdp_platform.h"
#include "rdpa_rate_limit.h"


typedef struct
{
    uint8_t dwrr_offset;
    uint8_t bbh_queue_index;
    uint8_t hw_bbh_qid;
    uint16_t first_q_index;
    uint8_t is_sq;
    /* secondary scheduler fields */
    uint8_t primary_scheduler_index;
    uint8_t primary_scheduler_slot_index;
    rdpa_tm_rl_rate_mode rl_rate_mode;
} scheduler_cfg_t;

typedef struct
{
    uint16_t qm_queue_index;
    uint8_t queue_scheduler_index;
    quantum_number_t quantum_number;
} scheduler_queue_t;

typedef enum
{
    rdd_basic_rl_queue = 0,
    rdd_basic_rl_basic_scheduler = 1,
    num_of_rdd_basic_rl_block = 2
} rdd_basic_rl_block_t;

typedef struct
{
    uint32_t rate;
    uint32_t limit;
    uint32_t residue;
    rdd_basic_rl_block_t type;
    uint32_t block_index;
} rdd_basic_rl_cfg_t;

typedef struct
{
    uint32_t scheduler_table_size;
    uint8_t *scheduler_table_p;
    uint32_t secondary_scheduler_table_size;
    uint8_t *secondary_scheduler_table_p;
    uint32_t rl_params_table_size;
    uint8_t *rl_params_table_p;
    uint32_t rl_budget_valid_max;
    uint8_t *rl_budget_valid_table_p;
    uint8_t *scheduler_pool_p;
    uint32_t rl_budget_desc_table_size;
    uint8_t *rl_budget_desc_table_p;
    uint32_t bbh_queue_descriptor_table_size;
    uint8_t *bbh_queue_descriptor_table_p;
    uint8_t *probability_calc_desc_table_p;
    uint8_t *flush_table_p;
    uint8_t *enabled_queues_table_p;
    uint8_t *pi2_stats_table_p;
    uint8_t *lq_stats_table_p;
    uint32_t queue_descriptor_table_size;
    uint8_t *queue_descriptor_table_p;
    uint8_t *aqm_queue_descriptor_table_p;
    uint8_t *queue_pool_p;
    uint32_t qm_queue_start;
    uint32_t qm_queue_end;
    uint8_t  bbh_bound;
    uint8_t *cbcm_p;
} rdd_tm_entity_info;

#if CHIP_VER >= RDP_GEN_62
/* generic API to reset ingress counters for all bbh queues for tm_identity */
bdmf_error_t rdd_wan_bbh_queue_init_counters(bdmf_boolean restore_old_values);
#endif


/* API to RDPA level */
void rdd_set_queue_enable(uint32_t qm_queue_index, bdmf_boolean enable);


/* initialize a general timer action */
void rdd_general_timer_set(uint16_t core_index, uint16_t func_pointer, uint32_t timer_period, uint32_t timer_action);

/* init epon in case epon active */
bdmf_error_t rdd_tm_epon_cfg(void);


/* init scheduling tables */
bdmf_error_t rdd_scheduling_init(void);

static inline int channel_is_eth(tm_identifier_e tm_identity)
{
    return (tm_identity >= TM_ETH_START) && (tm_identity <= TM_ETH_END);
}

static inline int channel_is_pon(tm_identifier_e tm_identity)
{
    return (tm_identity == TM_PON_DSL_AE);
}

static inline int channel_is_eth_sq(tm_identifier_e tm_identity)
{
    return (tm_identity == TM_ETH_SQ);
}

static inline rdpa_traffic_dir channel_to_dir(tm_identifier_e tm_identity)
{
    if ((tm_identity >= TM_ETH_START) && (tm_identity <= TM_ETH_END))
        return rdpa_dir_ds;
    else
        return rdpa_dir_us;
}

void rdd_init_rnr_tm(void);
void rdd_set_aqm_q_desc_offset(uint16_t gsu_idx, uint16_t rnr_base_q_qm);
void rdd_set_max_used_relative_q_num(uint16_t gsu_idx, uint16_t max_queue_idx);
void rdd_enable_timer_tasks(uint32_t global_gsu_idx, uint8_t enable);


#endif /* _SCHEDULING_H*/
