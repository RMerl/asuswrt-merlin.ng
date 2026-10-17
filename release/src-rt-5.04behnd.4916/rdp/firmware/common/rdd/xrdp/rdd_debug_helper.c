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

#if defined(DEBUG_DAEMON) && !defined(RDP_SIM)

#include "rdd.h"
#include "rdd_debug_daemon.h"
#include "rdpa_int.h"
#include "rdpa_port_int.h"
#ifdef CONFIG_DHD_RUNNER
#include "rdpa_dhd_helper_basic.h"
#endif
#include "rdpa_cpu_ex.h"
#include "rdp_drv_system.h"
#include "xrdp_autogens.h"

#ifdef DCL_SUPPORT
#include "rdp_drv_dcl.h"
extern char *dcl_id_enum_table[];
#endif
#ifdef CONFIG_DHD_RUNNER
int dhd_stats_get(uint32_t set_index, uint32_t stats_index, uint8_t *stat_buf);
#endif
int dhd_helper_dhd_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size);

int system_stats_get(uint32_t set_index, uint32_t stats_index, uint8_t *stat_buf);
int system_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);

extern const bdmf_attr_enum_table_t bufmng_enum_table;
int alloc_mon_set_table(int num_elements, monitor_set_struct_t **cur_set);


monitor_event_struct_t dhd_events_set_cfg[] =
    {
        { "fr_flush", "FlowRing flush", INFO_HIGH_EVENT, "FR_id", "", "" },
        { "fr_disable", "FlowRing disable", INFO_HIGH_EVENT, "FR_id", "", "" },
        { "fr_enable", "FlowRing enable", INFO_HIGH_EVENT, "FR_id", "", "" },
        { "fr_update", "FlowRing update", INFO_HIGH_EVENT, "FR_id", "", "" },
    };

monitor_event_struct_t sanity_stats_set_cfg[] =
    {
        { "sbpm_buf_underrun", "SBPM: buffer underrun", ERROR_EVENT, "", "", "" },
        { "sbpm_multicast_overflow", "SBPM: multicast overflow", ERROR_EVENT, "", "", "" },
        { "sbpm_free_with_context_error", "SBPM: free with context error", ERROR_EVENT, "", "", "" },
        { "sbpm_free_without_context_error", "SBPM: free without context error", ERROR_EVENT, "", "", "" },
        { "invalid_in2e_underflow", "SBPM: invalid ingress to egress command invalid_in2e_underflow", ERROR_EVENT, "", "", "" },
        { "invalid_in2e_overflow", "SBPM: invalid ingress to egress command invalid_in2e_overflow", WARNING_EVENT, "", "", "" },
        { "invalid_in2e", "SBPM: invalid ingress to egress command invalid_in2e", ERROR_EVENT, "", "", "" },
        { "alloc_null", "SBPM: alloc_null", ERROR_EVENT, "", "", "" },
        { "bac_underrun_ug0", "SBPM: bac_underrun_ug0", WARNING_EVENT, "", "", "" },
        { "bac_underrun_ug1", "SBPM: bac_underrun_ug1", WARNING_EVENT, "", "", "" },
        { "got_null_on_multi_get_next", "SBPM: got null on multi get next", ERROR_EVENT, "", "", "" },
        { "connect_null_buffer", "SBPM: connect null buffer", ERROR_EVENT, "", "", "" },
        { "dma_illegal_status", "RNR: dma_illegal_status", ERROR_EVENT, "core_id", "dma_illegal_status", "" },
        { "prediction_overrun", "RNR: prediction_overrun", WARNING_EVENT, "core_id", "prediction_overrun", "" },
        { "fit_fail_sts", "RNR: fit_fail_sts", ERROR_EVENT, "core_id", "fit_fail_sts", "" },
        { "qm_dqm_pop_on_empty", "QM: qm_dqm_pop_on_empty", ERROR_EVENT, "", "", "" },
        { "qm_dqm_push_on_full", "QM: qm_dqm_push_on_full", ERROR_EVENT, "", "", "" },
        { "qm_cpu_pop_on_empty", "QM: qm_cpu_pop_on_empty", ERROR_EVENT, "", "", "" },
        { "qm_cpu_push_on_full", "QM: qm_cpu_push_on_full", ERROR_EVENT, "", "", "" },
        { "qm_normal_queue_pd_no_credit", "QM: qm_normal_queue_pd_no_credit", ERROR_EVENT, "", "", "" },
        { "qm_non_delayed_queue_pd_no_cre", "QM: qm_non_delayed_queue_pd_no_cre", ERROR_EVENT, "", "", "" },
        { "qm_1588_multicast_contradictio", "QM: qm_1588_multicast_contradictio", ERROR_EVENT, "", "", "" },
        { "qm_agg_coherent_inconsistency", "QM: qm_agg_coherent_inconsistency", ERROR_EVENT, "", "", "" },
        { "qm_force_copy_on_non_delayed", "QM: qm_force_copy_on_non_delayed", ERROR_EVENT, "", "", "" },
        { "qm_fpm_pool_size_nonexistent", "QM: qm_fpm_pool_size_nonexistent", ERROR_EVENT, "", "", "" },
        { "qm_total_byte_cnt_underrun", "QM: qm_total_byte_cnt_underrun", ERROR_EVENT, "", "", "" },
        { "qm_total_pkt_cnt_underrun", "QM: qm_total_pkt_cnt_underrun", ERROR_EVENT, "", "", "" },
        { "qm_fpm_ug0_underrun", "QM: qm_fpm_ug0_underrun", ERROR_EVENT, "", "", "" },
        { "qm_fpm_ug1_underrun", "QM: qm_fpm_ug1_underrun", ERROR_EVENT, "", "", "" },
        { "qm_fpm_ug2_underrun", "QM: qm_fpm_ug2_underrun", ERROR_EVENT, "", "", "" },
        { "qm_fpm_ug3_underrun", "QM: qm_fpm_ug3_underrun", ERROR_EVENT, "", "", "" },
        { "fpm_overflow", "FPM: overflow", ERROR_EVENT, "", "", "" },
        { "fpm_underflow", "FPM: underflow", ERROR_EVENT, "", "", "" },
        { "fpm_invalid_token_freed", "FPM: invalid token freed", ERROR_EVENT, "", "", "" },
        { "fpm_invalid_mcast_token", "FPM: invalid mcast token freed", ERROR_EVENT, "", "", "" },
        { "bbh_tx_pddrop", "BBH tx: pddrop", ERROR_EVENT, "pddrop", "", "" },
        { "bbh_tx_aggrlenerr", "BBH tx: aggrlenerr", ERROR_EVENT, "aggrlenerr", "", "" },
        { "bbh_tx_msgdrop", "BBH tx: msgdrop", ERROR_EVENT, "msgdrop", "", "" },
        { "bbh_tx_stsdrop", "BBH tx: stsdrop", ERROR_EVENT, "stsdrop", "", "" },
        { "bbh_tx_lenerr", "BBH tx: lenerr", ERROR_EVENT, "lenerr", "", "" },
        { "c_stack_overflow", "Runner C stack overflow", ERROR_EVENT, "core_id", "base_addr", "" },
        { "bufmng_neg_err", "Bufmng negative error", ERROR_EVENT, "cnt bitmask", "cnt", "val" },
    };

monitor_set_struct_t qm_hw_stats_set_cfg[] =
    {
        { "qm_good_lvl1_pkts", "QM: good_lvl1_pkts", 4, clear_on_read_counter },
        { "qm_good_lvl1_bytes", "QM: good_lvl1_bytes", 4, clear_on_read_counter },
        { "qm_good_lvl2_pkts", "QM: good_lvl2_pkts", 4, clear_on_read_counter },
        { "qm_good_lvl2_bytes", "QM: good_lvl2_bytes", 4, clear_on_read_counter },
        { "qm_copied_pkts", "QM: copied_pkts", 4, clear_on_read_counter },
        { "qm_copied_bytes", "QM: copied_bytes", 4, clear_on_read_counter },
        { "qm_agg_pkts", "QM: agg_pkts", 4, clear_on_read_counter },
        { "qm_agg_bytes", "QM: agg_bytes", 4, clear_on_read_counter },
        { "qm_agg1_pkts", "QM: agg1_pkts", 4, clear_on_read_counter },
        { "qm_agg2_pkts", "QM: agg2_pkts", 4, clear_on_read_counter },
        { "qm_agg3_pkts", "QM: agg3_pkts", 4, clear_on_read_counter },
        { "qm_agg4_pkts", "QM: agg4_pkts", 4, clear_on_read_counter },
        { "qm_wred_drop", "QM: wred_drop", 4, clear_on_read_counter },
        { "qm_fpm_cong", "QM: fpm_cong", 4, clear_on_read_counter },
        { "qm_ddr_pd_cong_drop", "QM: ddr_pd_cong_drop", 4, clear_on_read_counter },
        { "qm_ddr_cong_byte_drop", "QM: ddr_cong_byte_drop", 4, clear_on_read_counter },
        { "qm_pd_congestion_drop", "QM: qm_pd_congestion_drop", 4, clear_on_read_counter },
#if (CHIP_VER < RDP_GEN_100)
        { "qm_abs_requeue_cnt", "QM: abs_requeue_cnt", 4, clear_on_read_counter },
#endif
    };

monitor_set_struct_t sbpm_hw_stats_set_cfg[] =
    {
        { "bac", "SBPM: bac", 4, current_level },
        { "sbpm_ug1bac", "SBPM: ug1bac", 4, current_level },
        { "sbpm_ug0bac", "SBPM: ug0bac", 4, current_level },
    };

monitor_set_struct_t fpm_hw_stats_set_cfg[] =
    {
        { "fpm_ovrfl", "FPM: ovrfl", 4, normal_counter },
        { "fpm_undrfl", "FPM: undrfl", 4, normal_counter },
        { "fpm_pool_full", "FPM: pool_full", 4, current_level },
        { "fpm_free_fifo_full", "FPM: free_fifo_full", 4, current_level },
        { "fpm_free_fifo_empty", "FPM: free_fifo_empty", 4, current_level },
        { "fpm_alloc_fifo_full", "FPM: alloc_fifo_full", 4, current_level },
        { "fpm_alloc_fifo_empty", "FPM: alloc_fifo_empty", 4, current_level },
        { "fpm_num_of_tokens_available", "FPM: num_of_tokens_available", 4, current_level },
        { "fpm_num_of_not_valid_token_frees", "FPM: num_of_not_valid_token_frees", 4, normal_counter },
        { "fpm_num_of_not_valid_token_multi", "FPM: num_of_not_valid_token_multi", 4, normal_counter },
        { "fpm_invalid_free_token", "FPM: invalid_free_token", 4, current_level },
        { "fpm_invalid_mcast_token", "FPM: invalid_mcast_token", 4, current_level },
        { "fpm_tokens_available_low_wtmk", "FPM: tokens_available_low_wtmk", 4, current_level },
    };

#ifdef DCL_SUPPORT
monitor_set_struct_t dcl_stats_set_cfg[] =
    {
        { "dcl_wrr_only", "DCL: wrr only", 4, current_level },
        { "dcl_sp_only", "DCL: sp only", 4, current_level },
        { "dcl_aqm_support", "DCL: aqm support", 4, current_level },
        { "dcl_secondary_support", "DCL: secondary scheduler support", 4, current_level },
        { "dcl_scheduler_rl_support", "DCL: scheduler rl support", 4, current_level },
        { "dcl_queue_rl_support", "DCL: queue rl support", 4, current_level },
        { "dcl_debug_support", "DCL: debug(epon, debug stat, mirror) support", 4, current_level }
    };
#endif

void check_periodic_events(void)
{
    rdp_drv_system_sanity(dummy_session_handle, NULL, 0);
}


static void set_field_type(int set_num, char *field_pattern, debug_field_type_t type)
{
    int i;
    monitor_set_struct_t *stat_cfg_ptr = (monitor_set_struct_t *)monitor_set_cfg[set_num];

    for (i = 0; i < monitor_set_info[set_num].num_of_fields; i++)
    {
        if (strstr(stat_cfg_ptr[i].name, field_pattern))
        {
            stat_cfg_ptr[i].field_type = type;
        }
    }
}

static int parse_attrib(struct bdmf_attr *attr, monitor_set_struct_t *set_ptr, uint32_t *fields_num)
{
    int rc = 0;
    struct bdmf_attr *a;

    if (attr->type != bdmf_attr_aggregate)
        return BDMF_ERR_NOT_SUPPORTED;

    for (a = attr->aggr_type->fields; a->name; a++)
    {
        if (a->type == bdmf_attr_aggregate)
        {
            rc = rc ? rc : parse_attrib(a, set_ptr, fields_num);
        }
        else
        {
            if (set_ptr)
            {
                strlcpy(set_ptr[*fields_num].name, a->name, sizeof(set_ptr[*fields_num].name)-1);
                strlcpy(set_ptr[*fields_num].description, a->help, sizeof(set_ptr[*fields_num].description)-1);
                set_ptr[*fields_num].field_size = a->size;
                /* default - normal counter */
                set_ptr[*fields_num].field_type = normal_counter;
            }

            (*fields_num)++;
        }
    }

    return rc;
}


static int build_cfg_set_table(bdmf_object_handle obj_h, char *stats_name, char *table_name, int *set_id)
{
    int rc;
    int set_fields_num = 0;
    bdmf_attr_id aid;
    struct bdmf_attr *attr;
    bdmf_object_handle obj = 0;
    monitor_set_struct_t *cur_set = NULL;

    obj = (struct bdmf_object *)obj_h;
    rc = bdmf_attr_by_name(obj->drv, stats_name, &aid);
    if (rc)
        return BDMF_ERR_NOENT;

    attr = bdmf_aid_to_attr(obj->drv, aid);

    if (attr->type != bdmf_attr_aggregate)
        return BDMF_ERR_NOT_SUPPORTED;

    set_fields_num = 0;
    rc = parse_attrib(attr, NULL, &set_fields_num);

    if (rc)
        return rc;

    if (!set_fields_num)
        return BDMF_ERR_NOENT;

    cur_set = bdmf_calloc(sizeof(monitor_set_struct_t) * set_fields_num);

    if (cur_set == NULL)
        return BDMF_ERR_NOMEM;

    set_fields_num = 0;
    monitor_set_cfg[total_cfg_set_num] = cur_set;
    rc = parse_attrib(attr, cur_set, &set_fields_num);

    if (rc)
    {
        bdmf_free(monitor_set_cfg[total_cfg_set_num]);
        monitor_set_cfg[total_cfg_set_num] = NULL;

        bdmf_trace("Build cfg table: %s failed rc=%d\n", table_name, rc);
        return rc;
    }

    strlcpy(monitor_set_info[total_cfg_set_num].name, table_name, sizeof(monitor_set_info[total_cfg_set_num].name)-1);
    monitor_set_info[total_cfg_set_num].set_type = stats_type;
    monitor_set_info[total_cfg_set_num].num_of_fields = set_fields_num;
    monitor_set_info[total_cfg_set_num].is_allocated = 1;

    *set_id = total_cfg_set_num;

    total_cfg_set_num++;
    return 0;
}


static void monitor_cfg_add_entry(uint8_t stat_event_id, uint8_t set_index, debug_superset_t superset_id,
                           debug_set_type_t set_type, char *name, uint8_t stats_amount, uint16_t buf_length, void *helper_func)
{
    monitor_cfg[total_mon_sets_num].valid = 1;
    monitor_cfg[total_mon_sets_num].stat_event_id = stat_event_id;
    monitor_cfg[total_mon_sets_num].set_index = set_index;
    monitor_cfg[total_mon_sets_num].superset = superset_id;
    monitor_cfg[total_mon_sets_num].set_type = set_type;
    strlcpy(monitor_cfg[total_mon_sets_num].description, name, sizeof(monitor_cfg[total_mon_sets_num].description)-1);
    monitor_cfg[total_mon_sets_num].stats_amount = stats_amount;
    monitor_cfg[total_mon_sets_num].buf_length = buf_length;
    monitor_cfg[total_mon_sets_num].helper_func = helper_func;

    total_mon_sets_num++;
}

int alloc_mon_set_table(int num_elements, monitor_set_struct_t **cur_set)
{
    *cur_set = bdmf_calloc(sizeof(monitor_set_struct_t) * num_elements);

    if (*cur_set == NULL)
        return BDMF_ERR_NOMEM;

    return 0;
}

static int cfg_set_add_entry(void *cur_set, char *table_name, debug_set_type_t stats_type,
                       uint8_t set_fields_num, int is_alloc, int *set_id)
{
    monitor_set_cfg[total_cfg_set_num] = cur_set;

    strlcpy(monitor_set_info[total_cfg_set_num].name, table_name, sizeof(monitor_set_info[total_cfg_set_num].name)-1);
    monitor_set_info[total_cfg_set_num].set_type = stats_type;
    monitor_set_info[total_cfg_set_num].num_of_fields = set_fields_num;
    monitor_set_info[total_cfg_set_num].is_allocated = is_alloc;

    *set_id = total_cfg_set_num;
    total_cfg_set_num++;

    return 0;
}

/********************************************************************************************************
 *
 * Statstics get functions
 *
 ********************************************************************************************************/

#ifdef CONFIG_DHD_RUNNER
int dhd_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    int rc;
    bdmf_object_handle obj = 0;

    rc = rdpa_dhd_helper_get(set_index, &obj);
    rc = rc ? rc : dhd_helper_dhd_stat_read_ex((struct bdmf_object *)obj, NULL, 0, (void *)stat_buf, 0);

    return rc;
}
#endif
int system_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    int rc = BDMF_ERR_IO;
    bdmf_object_handle obj = 0;

    obj = _rdpa_system_get();

    if (obj)
    {
        rc = system_attr_stat_read((struct bdmf_object *)obj, NULL, 0, (void *)stat_buf, 0);
    }

    return rc;
}

int port_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    int rc = BDMF_ERR_IO;
    bdmf_object_handle obj = 0;

    obj = _port_obj_get_by_vport(set_index);

    if (obj)
    {
        rc = port_attr_stat_read((struct bdmf_object *)obj, NULL, 0, (void *)stat_buf, 0);
    }

    return rc;
}

int cpu_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    int i, rc;
    bdmf_object_handle obj = 0;

    rc = rdpa_cpu_get(set_index, &obj);

    if (obj)
    {
        for (i = 0; i < stats_amount; i++)
        {
            rc = cpu_attr_rxq_stat_read_ex((struct bdmf_object *)obj, NULL, i, (void *)stat_buf, 0);
            stat_buf += sizeof(rdpa_cpu_rx_stat_t);
        }
    }

    return rc;
}

#ifdef DCL_SUPPORT
int dcl_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    uint32_t *stat_buf_ptr = (uint32_t *)stat_buf;
#ifdef XEPON_SUPPORT
    DCL_PON_CONFIGURATION_U dcl_stat;
#else
    DCL_ETH0_CONFIGURATION_U dcl_stat;
#endif
    dcl_get_active_data(&dcl_stat, set_index);
    *stat_buf_ptr++ = dcl_stat.DDCL_TM_WRR_ONLY;
    *stat_buf_ptr++ = dcl_stat.DDCL_TM_SP_ONLY;
    *stat_buf_ptr++ = dcl_stat.DDCL_TM_AQM_SUPPORT;
    *stat_buf_ptr++ = dcl_stat.DDCL_TM_SECONDARY_SCHEDULER_SUPPORT;
    *stat_buf_ptr++ = dcl_stat.DDCL_TM_SCHEDULER_RL_SUPPORT;
    *stat_buf_ptr++ = dcl_stat.DDCL_TM_QUEUE_RL_SUPPORT;
    *stat_buf_ptr++ = dcl_stat.DDCL_TM_DEBUG_SUPPORT;
#ifdef XEPON_SUPPORT
    *stat_buf_ptr++ = dcl_stat.DDCL_TM_XEPON_SUPPORT;
#endif
    return 0;    
}
#endif

#if CHIP_VER >= RDP_GEN_60
int bufmng_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    int ctr_idx, rc = 0;
    uint32_t *stat_buf_ptr = (uint32_t *)stat_buf;

    for (ctr_idx = 0; ctr_idx < CNPL_MAX_BUFMNG_INDEX; ctr_idx++)
    {
        rc = rc ? rc : ag_drv_cnpl_cnpl_stat_ctrs_val_get(ctr_idx, stat_buf_ptr++);
    }

    return rc;
}
#endif

int qm_hw_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    int rc = 0;
    uint32_t *stat_buf_ptr = (uint32_t *)stat_buf;

    ag_drv_qm_good_lvl1_pkts_cnt_get(stat_buf_ptr++);
    ag_drv_qm_good_lvl1_bytes_cnt_get(stat_buf_ptr++);
    ag_drv_qm_good_lvl2_pkts_cnt_get(stat_buf_ptr++);
    ag_drv_qm_good_lvl2_bytes_cnt_get(stat_buf_ptr++);
    ag_drv_qm_copied_pkts_cnt_get(stat_buf_ptr++);
    ag_drv_qm_copied_bytes_cnt_get(stat_buf_ptr++);
    ag_drv_qm_agg_pkts_cnt_get(stat_buf_ptr++);
    ag_drv_qm_agg_bytes_cnt_get(stat_buf_ptr++);
    ag_drv_qm_agg_1_pkts_cnt_get(stat_buf_ptr++);
    ag_drv_qm_agg_2_pkts_cnt_get(stat_buf_ptr++);
    ag_drv_qm_agg_3_pkts_cnt_get(stat_buf_ptr++);
    ag_drv_qm_agg_4_pkts_cnt_get(stat_buf_ptr++);
    ag_drv_qm_wred_drop_cnt_get(stat_buf_ptr++);
    ag_drv_qm_fpm_congestion_drop_cnt_get(stat_buf_ptr++);
    ag_drv_qm_ddr_pd_congestion_drop_cnt_get(stat_buf_ptr++);
    ag_drv_qm_ddr_byte_congestion_drop_cnt_get(stat_buf_ptr++);
    ag_drv_qm_qm_pd_congestion_drop_cnt_get(stat_buf_ptr++);
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */
    ag_drv_qm_qm_abs_requeue_cnt_get(stat_buf_ptr++);
#endif
    return rc;
}

#if (CHIP_VER < RDP_GEN_100)
int sbpm_hw_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    int rc = 0;
    uint32_t *stat_buf_ptr = (uint32_t *)stat_buf;
    uint16_t bac, ug1bac, ug0bac;

    ag_drv_sbpm_bac_get(&bac, &ug1bac, &ug0bac);
    *stat_buf_ptr++ = bac;
    *stat_buf_ptr++ = ug1bac;
    *stat_buf_ptr++ = ug0bac;

    return rc;
}
#endif

int fpm_hw_stats_get(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf)
{
    int rc = 0;
    uint32_t *stat_buf_ptr = (uint32_t *)stat_buf;
    fpm_pool_stat pool_stat;

    rc = ag_drv_fpm_pool_stat_get(&pool_stat);

    *stat_buf_ptr++ = pool_stat.ovrfl;
    *stat_buf_ptr++ = pool_stat.undrfl;
    *stat_buf_ptr++ = pool_stat.pool_full;
    *stat_buf_ptr++ = pool_stat.free_fifo_full;
    *stat_buf_ptr++ = pool_stat.free_fifo_empty;
    *stat_buf_ptr++ = pool_stat.alloc_fifo_full;
    *stat_buf_ptr++ = pool_stat.alloc_fifo_empty;
    *stat_buf_ptr++ = pool_stat.num_of_tokens_available;
    *stat_buf_ptr++ = pool_stat.num_of_not_valid_token_frees;
    *stat_buf_ptr++ = pool_stat.num_of_not_valid_token_multi;
    *stat_buf_ptr++ = pool_stat.invalid_free_token;
    *stat_buf_ptr++ = pool_stat.invalid_mcast_token;
    *stat_buf_ptr++ = pool_stat.tokens_available_low_wtmk;

    return rc;
}


/********************************************************************************************************
 *
 * Build sets functions
 *
 ********************************************************************************************************/
#ifdef CONFIG_DHD_RUNNER
static void add_dhd_superset(void)
{
    int i, rc, set_id, set_event_id;
    char name[256];
    bdmf_object_handle obj_h;

    rc = rdpa_dhd_helper_get(0, &obj_h);
    /* Build dhd set table */
    rc = rc ? rc : build_cfg_set_table(obj_h, "dhd_stat", "dhd_stat_table", &set_id);

    /* update fpm_used fields stats type */
    if (!rc)
    {
        set_field_type(set_id, "fpm_used", current_level);
    }

    rc = rc ? rc : cfg_set_add_entry(dhd_events_set_cfg, "dhd_events", event_type,
                                     NUM_OF_ELEMENTS(dhd_events_set_cfg), 0, &set_event_id);

    if (!rc)
    {
        /* Add entries to monitor_cfg_table */
        for (i = 0; i < RDPA_MAX_RADIOS; i++)
        {
            rc = rdpa_dhd_helper_get(i, &obj_h);

            if (!rc)
            {
                snprintf(name, sizeof(name)-1, "DHD stats radio %d", i);
                monitor_cfg_add_entry(set_id, i, dhd_set, stats_type, name, 1, sizeof(rdpa_dhd_data_stat_t), &dhd_stats_get);
                snprintf(name, sizeof(name)-1, "DHD events radio %d", i);
                monitor_cfg_add_entry(set_event_id, i, dhd_set, event_type, name, 1, 0, NULL);
            }
        }
    }
}
#endif

static void add_system_superset(void)
{
    int rc = BDMF_ERR_PARM, set_id = 0, set_event_id;
    bdmf_object_handle obj_h;

    obj_h = _rdpa_system_get();

    /* Build system set table */
    if (obj_h)
    {
        rc = build_cfg_set_table(obj_h, "stat", "system_stat_table", &set_id);
        rc = rc ? rc : cfg_set_add_entry(sanity_stats_set_cfg, "sanity_events", event_type,
                                     NUM_OF_ELEMENTS(sanity_stats_set_cfg), 0, &set_event_id);
    }

    if (!rc)
    {
        monitor_cfg_add_entry(set_id, 0, system_set, stats_type, "system drop stat", 1, sizeof(rdpa_system_stat_t), &system_stats_get);
        monitor_cfg_add_entry(set_event_id, 0, system_set, event_type, "sanity events", 1, 0, NULL);
    }
}

static void add_port_superset(void)
{
    int rc = 0, set_id, vport;
    bdmf_object_handle obj_h = NULL;
    char name[256];
    char *port_name = NULL;
    int cfg_built = 0;

    for (vport = 0; vport < PROJ_DEFS_NUMBER_OF_VPORTS; vport++)
    {
        obj_h = NULL;

        port_name = rdpa_port_dev_name_get(vport);

        if (port_name)
        {
            rc = rdpa_port_get(port_name, &obj_h);
        }

        /* Build system set table */
        if (obj_h && !cfg_built)
        {
            rc = build_cfg_set_table(obj_h, "stat", "port_stat_table", &set_id);
            if (!rc)
            {
                cfg_built = 1;
            }
        }

        if (cfg_built && port_name)
        {
            snprintf(name, sizeof(name)-1, "Port stats: %s", port_name);
            monitor_cfg_add_entry(set_id, vport, port_set, stats_type, name, 1, sizeof(rdpa_port_stat_t), &port_stats_get);
        }
    }
}

static void add_cpu_superset(void)
{
    int rc = 0, set_id, i;
    bdmf_object_handle obj_h = NULL;
    char name[128];
    int cfg_built = 0;
    long long int num_queues;

    for (i = 0; i < rdpa_cpu_port__num_of; i++)
    {
        obj_h = NULL;
        rc = rdpa_cpu_get(i, &obj_h);

        /* Build system set table */
        if (obj_h && !cfg_built)
        {
            rc = build_cfg_set_table(obj_h, "rxq_stat", "cpu_rxq_stat_table", &set_id);
            if (!rc)
            {
                cfg_built = 1;
                /* update fields stats type */
                set_field_type(set_id, "queued", current_level);
                set_field_type(set_id, "dropped", current_level);
            }
        }

        if (cfg_built && obj_h)
        {
            snprintf(name, sizeof(name)-1, "CPU rxq stats: %s", ((struct bdmf_object *)obj_h)->name);
            rdpa_cpu_num_queues_get(obj_h, &num_queues);

            monitor_cfg_add_entry(set_id, i, cpu_set, stats_type, name,
                num_queues, sizeof(rdpa_cpu_rx_stat_t), &cpu_stats_get);
        }
    }
}

#if CHIP_VER >= RDP_GEN_60
static int build_bufmng_set_table(int *set_id, int *stat_size)
{
    int ctr_idx, rc = 0;
    const char *name;
    char default_name[64];
    monitor_set_struct_t *cur_set = 0;

    rc = alloc_mon_set_table(CNPL_MAX_BUFMNG_INDEX, &cur_set);
    if (rc)
        return rc;

    for (ctr_idx = 0; ctr_idx < CNPL_MAX_BUFMNG_INDEX; ctr_idx++)
    {
        name = bdmf_attr_get_enum_text_hlp(&bufmng_enum_table, ctr_idx);

        if (!name)
        {
            snprintf(default_name, sizeof(default_name)-1, "bufmng_cnt_%d", ctr_idx);
            name = default_name;
        }
        strlcpy(cur_set[ctr_idx].name, name, sizeof(cur_set[ctr_idx].name)-1);  /* to terminate buffer anyway */
        strlcpy(cur_set[ctr_idx].description, name, sizeof(cur_set[ctr_idx].description)-1);  /* to terminate buffer anyway */
        cur_set[ctr_idx].field_size = sizeof(uint32_t);
        cur_set[ctr_idx].field_type = current_level;
    }

    *stat_size = CNPL_MAX_BUFMNG_INDEX*sizeof(uint32_t);
    cfg_set_add_entry(cur_set, "BUFMNG set", stats_type, CNPL_MAX_BUFMNG_INDEX, 1, set_id);

    return 0;
}
#endif

static void add_hw_mon_set_superset(void)
{
    int set_id;
    
#if CHIP_VER >= RDP_GEN_60
    int stat_size = 0;

    build_bufmng_set_table(&set_id, &stat_size);
    monitor_cfg_add_entry(set_id, 0, hw_mon_set, stats_type, "BUFMNG stats",
        1, stat_size, &bufmng_stats_get);
#endif

    cfg_set_add_entry(qm_hw_stats_set_cfg, "QM HW set", stats_type, NUM_OF_ELEMENTS(qm_hw_stats_set_cfg), 0, &set_id);
    monitor_cfg_add_entry(set_id, 0, hw_mon_set, stats_type, "QM HW stats",
        1, NUM_OF_ELEMENTS(qm_hw_stats_set_cfg)*sizeof(uint32_t), &qm_hw_stats_get);

#if (CHIP_VER < RDP_GEN_100)
    cfg_set_add_entry(sbpm_hw_stats_set_cfg, "SBPM HW set", stats_type, NUM_OF_ELEMENTS(sbpm_hw_stats_set_cfg), 0, &set_id);
    monitor_cfg_add_entry(set_id, 0, hw_mon_set, stats_type, "SBPM HW stats",
        1, NUM_OF_ELEMENTS(sbpm_hw_stats_set_cfg)*sizeof(uint32_t), &sbpm_hw_stats_get);
#endif

    cfg_set_add_entry(fpm_hw_stats_set_cfg, "FPM HW set", stats_type, NUM_OF_ELEMENTS(fpm_hw_stats_set_cfg), 0, &set_id);
    monitor_cfg_add_entry(set_id, 0, hw_mon_set, stats_type, "FPM HW stats",
        1, NUM_OF_ELEMENTS(fpm_hw_stats_set_cfg)*sizeof(uint32_t), &fpm_hw_stats_get);
}

#ifdef DCL_SUPPORT
static void add_dcl_set_superset(void)
{
    int i;
    int set_id;
    for (i = 0; i < DCL_NUM_OF_FEATURES; i++)
    {
        cfg_set_add_entry(dcl_stats_set_cfg, dcl_id_enum_table[i], stats_type, NUM_OF_ELEMENTS(dcl_stats_set_cfg), 0, &set_id);
        monitor_cfg_add_entry(set_id, i, dcl_set, stats_type, dcl_id_enum_table[i] ,
            1, NUM_OF_ELEMENTS(dcl_stats_set_cfg)*sizeof(uint32_t), &dcl_stats_get);
    }
}
#endif


static int is_cfg_set(uint32_t bitmask, uint32_t id)
{
    return (bitmask >> id) & 0x1;
}

int build_debug_daemon_tables(int superset_bitmask)
{
    total_mon_sets_num = 0;
    total_cfg_set_num = 0;

    memset(monitor_cfg, 0, sizeof(monitor_cfg));
    memset(monitor_set_cfg, 0, sizeof(monitor_set_cfg));

#ifdef CONFIG_DHD_RUNNER
    if (is_cfg_set((uint32_t)superset_bitmask, dhd_set))
    {
        add_dhd_superset();
    }
#endif
    if (is_cfg_set((uint32_t)superset_bitmask, system_set))
    {
        add_system_superset();
    }
    if (is_cfg_set((uint32_t)superset_bitmask, port_set))
    {
        add_port_superset();
    }
    if (is_cfg_set((uint32_t)superset_bitmask, cpu_set))
    {
        add_cpu_superset();
    }
    if (is_cfg_set((uint32_t)superset_bitmask, hw_mon_set))
    {
        add_hw_mon_set_superset();
    }

#ifdef DCL_SUPPORT
    if (is_cfg_set((uint32_t)superset_bitmask, dcl_set))
    {
        add_dcl_set_superset();
    }
#endif

    return 0;
}

void destroy_debug_daemon_tables(void)
{
    int i;

    for (i = 0; i < total_cfg_set_num; i++)
    {
        if (monitor_set_cfg[i] && monitor_set_info[i].is_allocated)
        {
            bdmf_free(monitor_set_cfg[i]);
            monitor_set_cfg[i] = NULL;
        }
    }

    for (i = 0; i < total_mon_sets_num; i++)
    {
        monitor_cfg[i].valid = 0;
    }

    total_mon_sets_num = 0;
    total_cfg_set_num = 0;
}
#endif
