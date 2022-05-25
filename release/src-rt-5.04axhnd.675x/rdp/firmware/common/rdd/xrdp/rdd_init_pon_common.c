/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#include "rdd_init_pon_common.h"

/*  Classification chain: Flow based
 *
 *   Module list
 *   -----------
 *  1. Ingress Filter
 *  2. Ingress flow classifier
 *  3. QoS
 *  4. tunnel parsing (just for DS)
 *  5. natc lookup
 *  6. IPTV
 *
 */

#if !defined(RDP_UFC)
static rdd_tcam_table_parm_t tcam_ic_flow_us_params =
{
    .module_id = TCAM_IC_MODULE_FLOW_US,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_flow_us_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
                      offsetof(RULE_BASED_CONTEXT_STRUCT, tcam_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_FLOW * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,   /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_flow_us_params
};

static rdd_tcam_table_parm_t tcam_ic_flow_ds_params =
{
    .module_id = TCAM_IC_MODULE_FLOW_DS,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_flow_ds_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
                      offsetof(RULE_BASED_CONTEXT_STRUCT, tcam_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_FLOW * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,   /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_flow_ds_params
};

static rdd_tcam_table_parm_t tcam_ic_qos_us_params =
{
    .module_id = TCAM_IC_MODULE_QOS_US,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_qos_us_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(RULE_BASED_CONTEXT_STRUCT, tcam_qos_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_QOS * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_qos_us_params
};

static rdd_tcam_table_parm_t tcam_ic_qos_ds_params =
{
    .module_id = TCAM_IC_MODULE_QOS_DS,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_qos_ds_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(RULE_BASED_CONTEXT_STRUCT, tcam_qos_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_QOS * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_qos_ds_params
};

static rdd_tcam_table_parm_t tcam_ic_acl_ds_params =
{
    .module_id = TCAM_IC_MODULE_ACL_DS,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_acl_ds_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(RULE_BASED_CONTEXT_STRUCT, tcam_acl_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_ACL * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_acl_ds_params
};

static rdd_tcam_table_parm_t tcam_ic_acl_us_params =
{
    .module_id = TCAM_IC_MODULE_ACL_US,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_acl_us_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(RULE_BASED_CONTEXT_STRUCT, tcam_acl_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_ACL * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_acl_us_params
};

static rdd_tcam_table_parm_t tcam_ic_ip_flow_ds_params =
{
    .module_id = TCAM_IC_MODULE_IP_FLOW_DS,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_ip_flow_ds_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(FLOW_BASED_CONTEXT_STRUCT, tcam_ip_flow_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_IP_FLOW * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_ip_flow_ds_params
};

static rdd_tcam_table_parm_t tcam_ic_ip_flow_us_params =
{
    .module_id = TCAM_IC_MODULE_IP_FLOW_US,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_ip_flow_us_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(FLOW_BASED_CONTEXT_STRUCT, tcam_ip_flow_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_IP_FLOW * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_ip_flow_us_params
};

static rdd_tcam_table_parm_t tcam_ic_ip_flow_ds_miss_params =
{
    .module_id = TCAM_IC_MODULE_IP_FLOW_MISS_DS,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_ip_flow_ds_miss_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(FLOW_BASED_CONTEXT_STRUCT, tcam_ip_flow_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_IP_FLOW * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_ip_flow_ds_miss_params
};

static rdd_tcam_table_parm_t tcam_ic_ip_flow_us_miss_params =
{
    .module_id = TCAM_IC_MODULE_IP_FLOW_MISS_US,
    .scratch_offset = offsetof(PACKET_BUFFER_STRUCT, scratch) + TCAM_IC_SCRATCH_KEY_OFFSET,
};

static rdd_module_t tcam_ic_ip_flow_us_miss_module =
{
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(FLOW_BASED_CONTEXT_STRUCT, tcam_ip_flow_result),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TCAM_IC_IP_FLOW * 4)),
    .cfg_ptr = RDD_TCAM_IC_CFG_TABLE_ADDRESS_ARR,  /* Instance offset will be added at init time */
    .init = rdd_tcam_module_init,
    .params = &tcam_ic_ip_flow_us_miss_params
};

static rdd_module_t ingress_filter_module =
{
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_INGRESS_FILTERS * 4)),
    .cfg_ptr = RDD_INGRESS_FILTER_CFG_ADDRESS_ARR,
    .init = rdd_ingress_filter_module_init,
    .params = NULL
};

#if !defined(OPERATION_MODE_PRV) || defined(RULE_BASED_GRE) 
/* Tunnels parser */
static tunnels_parsing_params_t tunnels_parsing_params =
{
    .tunneling_enable = 0
};
#endif

#ifdef RULE_BASED_GRE
rdd_module_t tunnels_ic_parsing =
{
    .init = rdd_tunnels_parsing_init,
    .cfg_ptr = RDD_TUNNELS_PARSING_CFG_ADDRESS_ARR,
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TUNNELS_IC_PARSING * 4)),
    .params = (void *)&tunnels_parsing_params
};
#endif


/* Bridge */

static rdd_bridge_module_param_t bridge_params_ds =
{
    .bridge_lkps_ready = 0,
    .aggregation_en = 0,
    .bridge_module_actions.hit = 1,
    .bridge_module_actions.bridge_fw_failed_action = 0,
    .bridge_module_actions.vlan_aggregation_action = 0,
    .module_id = BRIDGE_FLOW_DS
};

static rdd_bridge_module_param_t bridge_params_us =
{
    .bridge_lkps_ready = 1,
    .aggregation_en = 0,
    .bridge_module_actions.hit = 1,
    .bridge_module_actions.bridge_fw_failed_action = 0,
    .bridge_module_actions.vlan_aggregation_action = 0,
    .module_id = BRIDGE_FLOW_US
};

static rdd_module_t bridge_module_ds =
{
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_BRIDGE * 4)),
    .cfg_ptr = RDD_BRIDGE_CFG_TABLE_ADDRESS_ARR,
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) + 
        offsetof(RULE_BASED_CONTEXT_STRUCT, bridge_port_vector),
    .init = rdd_bridge_module_init,
    .params = &bridge_params_ds
};

static rdd_module_t bridge_module_us =
{
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_BRIDGE * 4)),
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) + 
        offsetof(RULE_BASED_CONTEXT_STRUCT, bridge_port_vector),
    .cfg_ptr = RDD_BRIDGE_CFG_TABLE_ADDRESS_ARR,
    .init = rdd_bridge_module_init,
    .params = &bridge_params_us
};

/* IPTV */
static iptv_params_t iptv_params =
{
    .key_offset = offsetof(PACKET_BUFFER_STRUCT, scratch),
    .hash_tbl_idx = 1
};

static rdd_module_t iptv_module =
{
    .init = rdd_iptv_module_init,
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_IPTV * 4)),
    .cfg_ptr = RDD_IPTV_CFG_TABLE_ADDRESS_ARR,
    .params = (void *)&iptv_params
};

/* IP CLASS */
static natc_params_t natc_params =
{
    .connection_key_offset = offsetof(PACKET_BUFFER_STRUCT, scratch),
};

static rdd_module_t ip_flow =
{
    .init = rdd_nat_cache_init,
    /* NTAC returns 60B (first 4B are control) */
    /* For 8B alignment, 4B are added */
    /* LD_CONTEXT macro adds 8B (control) */
    .context_offset = offsetof(PACKET_BUFFER_STRUCT, classification_contexts_list) +
        offsetof(FLOW_BASED_CONTEXT_STRUCT, flow_cache),
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_NAT_CACHE * 4)),
    .cfg_ptr = RDD_NAT_CACHE_CFG_ADDRESS_ARR,
    .params = (void *)&natc_params
};

#ifndef OPERATION_MODE_PRV
rdd_module_t tunnels_parsing =
{
    .init = rdd_tunnels_parsing_init,
    .cfg_ptr = RDD_TUNNELS_PARSING_CFG_ADDRESS_ARR,
    .res_offset = (offsetof(PACKET_BUFFER_STRUCT, classification_results) +
        (CLASSIFICATION_RESULT_INDEX_TUNNELS_PARSING * 4)),
    .params = (void *)&tunnels_parsing_params
};
#endif


void rdd_proj_init(rdd_init_params_t *init_params)
{
    /* Classification modules initialization */
#ifndef OPERATION_MODE_PRV
    _rdd_module_init(&tunnels_parsing);
#else
#ifdef RULE_BASED_GRE
    _rdd_module_init(&tunnels_ic_parsing);
#endif
#endif    
    _rdd_module_init(&ip_flow);
    if (init_params->is_basic)
        return;

    _rdd_module_init(&tcam_ic_flow_us_module);
    _rdd_module_init(&tcam_ic_qos_us_module);
    _rdd_module_init(&tcam_ic_flow_ds_module);
    _rdd_module_init(&tcam_ic_qos_ds_module);
    /* ACL exist only in none mode and use the same module ids of generic filter */
    if ( init_params->is_gateway == 0)
    {
        _rdd_module_init(&tcam_ic_acl_us_module);
        _rdd_module_init(&tcam_ic_acl_ds_module);
    }
    else
    {
        _rdd_module_init(&tcam_ic_ip_flow_ds_module);
        _rdd_module_init(&tcam_ic_ip_flow_us_module);
    }
    _rdd_module_init(&tcam_ic_ip_flow_us_miss_module);
    _rdd_module_init(&tcam_ic_ip_flow_ds_miss_module);

    _rdd_module_init(&ingress_filter_module);
    _rdd_module_init(&iptv_module);
    _rdd_module_init(&bridge_module_ds);
    _rdd_module_init(&bridge_module_us);
}

#endif /* !defined(RDP_UFC) */

int rdd_init(void)
{
#ifdef RDP_SIM
    if (rdd_sim_alloc_segments())
        return -1;
#endif
    return 0;
}

void rdd_exit(void)
{
#ifdef RDP_SIM
    rdd_sim_free_segments();
#endif
}

#if defined(DUAL_ISSUE)
void rdd_global_registers_init(uint32_t core_index, uint32_t local_regs[NUM_OF_MAIN_RUNNER_THREADS][NUM_OF_LOCAL_REGS], uint32_t last_thread)
{
     uint32_t i;
    /* in dual_issue we don't have really global, so will set all registers that should be global for all threads */
    for (i = 0; i <= last_thread; ++i)
    {

        local_regs[i][31] = 1; /* CONST_1 is 1, in 6878 its r31 */
#if !defined(RDP_UFC)
        /* VPORT_CFG_EX address is here just to save a mov command in FW and can be replaced if necessary */
        local_regs[i][4] = (RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR[core_index] << 16) + RDD_VPORT_CFG_TABLE_ADDRESS_ARR[core_index];
#endif
    }
}
#else
void rdd_global_registers_init(uint32_t core_index)
{
    static uint32_t global_regs[8] = {};
    uint32_t i;

    /********** Reserved global registers **********/
    /* R6 - ingress qos don't drop counter in processing cores */

    /********** common to all runners **********/
    global_regs[1] = 1; /* R1 = 1 */

    
    /* VPORT_CFG_EX address is here just to save a mov command in FW and can be replaced if necessary */
    global_regs[4] = (RDD_VPORT_CFG_EX_TABLE_ADDRESS_ARR[core_index] << 16) + RDD_VPORT_CFG_TABLE_ADDRESS_ARR[core_index];

    /* R7 is used in US/DS scheduling */

    for (i = 0; i < 8; ++i)
        RDD_BYTES_4_BITS_WRITE(global_regs[i], (uint8_t *)RDD_RUNNER_GLOBAL_REGISTERS_INIT_PTR(core_index) + (sizeof(BYTES_4_STRUCT) * i));
}
#endif

int rdd_cpu_proj_init(void)
{
    uint8_t def_idx = (uint8_t)BDMF_INDEX_UNASSIGNED;
    int rc = 0;

    rdd_cpu_tc_to_rqx_init(def_idx);
    rdd_cpu_vport_cpu_obj_init(def_idx);
    rdd_cpu_rx_meters_init();
    rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(cpu_rx_runner_image),
        CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER);

    return rc;
}

void rdd_write_action(uint8_t core_index, uint16_t *action_arr, uint8_t size_of_array, uint8_t *ptr, uint8_t tbl_size)
{
    uint32_t action_index;
    for (action_index = 0; action_index < tbl_size; action_index++)
    {
        if (action_index < size_of_array)
            RDD_BYTES_2_BITS_WRITE(action_arr[action_index], ptr + (sizeof(action_arr[0]) * action_index));
        else
            BDMF_TRACE_ERR("Empty action left in actions table! action_index=%d\n", action_index);
    }
}
