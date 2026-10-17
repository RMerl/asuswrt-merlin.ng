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

/*
 *  Created on: Apr/2024
 *      Author: stefan.chulski@broadcom.com
 */ 

#include "port.h"
#include "runner.h"
#include "runner_common.h"
#include <rdpa_api.h>
#include "enet_dbg.h"
#include "runner_egress_tc.h"
#include <bp3_license.h>

#if defined(ENET_EPON)
#include <rdpa_epon.h>
#include "rdpa_ag_epon.h"
#endif

#define EGRESS_TM_100GBIT 100000000000

static int port_runner_validate_egress_tm(egress_tm_node *node, egress_tm_node **ets_node, egress_tm_node **rl_node,
                                          char *port_name);
static int runner_tc_modify_egress_tm(enetx_port_t *self, bdmf_object_handle port_obj, bdmf_object_handle sched,
                                      char *port_name, egress_tm_node *node);
static int tc_rdpa_egress_tm_subsidiary(bdmf_object_handle tm_obj, int is_wan, egress_tm_node *child, char *port_name, bool has_queues);

#define EGRESS_TM_PON_NAME(type) (type) == PORT_TYPE_RUNNER_GPON ? "TCONT" : "LLID"

#if defined(POLICER_SUPPORT)

#define POLICER_TC2RDPA_INDEX_MAP(index)    ((index) - 1)

int port_runner_tc_config_policer(struct flow_action_entry *act)
{
    bdmf_mattr_handle mattr = NULL;
    bdmf_number index = POLICER_TC2RDPA_INDEX_MAP(act->police.index);
    bdmf_object_handle policer = NULL;
    rdpa_tm_policer_cfg_t policer_cfg = {0};
    int rc = 0;

    if (!rdpa_policer_get(index, &policer)) {
        if ((rc = rdpa_policer_cfg_get(policer, &policer_cfg)))
        {
            egress_tm_err("Failed to get RDPA policer config, rc=%d\n", rc);
            bdmf_put(policer);
            return -1;
        }
    }

    policer_cfg.commited_rate = BYTES_TO_BITS(act->police.rate_bytes_ps);
    policer_cfg.committed_burst_size = act->police.burst;
    if (act->police.peakrate_bytes_ps) {
        if (act->police.peakrate_bytes_ps == RDPA_INVALID_KEY_U64){
            policer_cfg.peak_rate = RDPA_INVALID_KEY_U64;
            policer_cfg.type = rdpa_tm_policer_sr_overflow_dual_token_bucket;
        } else {
            policer_cfg.type = rdpa_tm_policer_tr_dual_token_bucket;
            policer_cfg.peak_rate = BYTES_TO_BITS(act->police.peakrate_bytes_ps);
        }
        policer_cfg.peak_burst_size = act->police.mtu;
    } else {
        policer_cfg.type = rdpa_tm_policer_single_token_bucket;
        policer_cfg.peak_rate = RDPA_INVALID_KEY_U64;
        policer_cfg.peak_burst_size = RDPA_INVALID_KEY;
    }
    policer_cfg.dei_mode = RDPA_INVALID_KEY;
    policer_cfg.color_aware_enabled = RDPA_INVALID_KEY;
    if (act->police.overhead)
        policer_cfg.rl_overhead = act->police.overhead;
    else
        policer_cfg.rl_overhead = RDPA_INVALID_KEY;

    if (policer) {
        rc = rdpa_policer_cfg_set(policer, &policer_cfg);
        if (rc)
            egress_tm_err("Failed to set RDPA policer config, rc=%d\n", rc);

        bdmf_put(policer);
    } else {
        mattr = bdmf_mattr_alloc(rdpa_policer_drv());
        if (!mattr)
        {
            rc = -ENOMEM;
            goto error;
        }

        if ((rc = rdpa_policer_index_set(mattr, index)))
        {
            egress_tm_err("Failed to set RDPA policer index, rc=%d\n", rc);
            goto error;
        }

        if ((rc = rdpa_policer_cfg_set(mattr, &policer_cfg)))
        {
            egress_tm_err("Failed to set RDPA policer config, rc=%d\n", rc);
            goto error;
        }

        if (bdmf_new_and_set(rdpa_policer_drv(), NULL, mattr, &policer))
        {
            rc = -EINVAL;
            goto error;
        }
        bdmf_mattr_free(mattr);
    }

    return rc;
error:
    if (mattr)
        bdmf_mattr_free(mattr);
    return rc;
}

int port_runner_tc_remove_policer(int index)
{
    bdmf_object_handle policer = NULL;

    if (!rdpa_policer_get(POLICER_TC2RDPA_INDEX_MAP(index), &policer)) {
        bdmf_put(policer);
        bdmf_destroy(policer);
    } else {
        egress_tm_err("RDPA policer=%d doesn't exist\n", POLICER_TC2RDPA_INDEX_MAP(index));
        return -EEXIST;
    }

    return 0;
}
#endif /* #if defined(POLICER_SUPPORT) */

#if defined(ENET_EPON)
static rdpa_epon_mode runner_tc_get_epon_mode(void)
{
    rdpa_epon_mode epon_mode = rdpa_epon_none;
    bdmf_object_handle system_obj = NULL;

    if (!rdpa_epon_get(&system_obj))
    {
        rdpa_epon_mode_get(system_obj, &epon_mode);
        bdmf_put(system_obj);
    }

    return epon_mode;
}
#endif

static bool runner_tc_is_static_ctc_mode(port_type_t devType, bdmf_boolean is_lan)
{
#if defined(ENET_EPON)
    rdpa_epon_mode epon_mode = runner_tc_get_epon_mode();

    if (((devType == PORT_TYPE_RUNNER_EPON) ||
       (devType == PORT_TYPE_RUNNER_PORT && !is_lan)) &&
       ((epon_mode == rdpa_epon_ctc) || (epon_mode == rdpa_epon_cuc)))
    {
        return 1;
    }
#endif

    return 0;
}

static int tc_rdpa_egress_tm_queues_cfg(bdmf_object_handle tm_obj, void *queue_cfg_start, uint8_t num_queues, uint8_t num_subsidiaries)
{
    rdpa_tm_queue_cfg_t *queue_cfg;
    bdmf_error_t rc = 0;
    int qid;

    for (qid = 0; qid < (num_queues - num_subsidiaries); qid++)
    {
        queue_cfg = queue_cfg_start + sizeof(rdpa_tm_queue_cfg_t) * qid;
        if ((rc = rdpa_egress_tm_queue_cfg_set(tm_obj, queue_cfg->queue_id, queue_cfg)))
        {
            egress_tm_err("Failed to configure RDPA egress tm queue %d. rc=%d\n", qid, rc);
            break;
        }
    }

    return rc;
}

static int runner_prepare_egress_tm_mqprio_queues_cfg(bdmf_object_handle tm_obj, void *queue_cfg_start,
                                                struct tc_mqprio_qopt_offload *mqprio)
{
    bdmf_error_t rc = 0;
    uint8_t num_queues;
    int qid;
    bool def_queue_config;
    rdpa_tm_queue_cfg_t *queue_cfg;

    rdpa_egress_tm_num_queues_get(tm_obj, &num_queues);

    def_queue_config = num_queues != mqprio->qopt.num_tc ? true : false;

    for (qid = 0; qid < mqprio->qopt.num_tc; qid++)
    {
        bdmf_mattr_handle mattr = NULL;

        queue_cfg = queue_cfg_start + sizeof(rdpa_tm_queue_cfg_t) * qid;
        queue_cfg->drop_alg = rdpa_tm_drop_alg_dt;
        if (def_queue_config) {
            queue_cfg->drop_threshold = EGRESS_TM_DF_QUEUE_THRESHOLD;
            queue_cfg->reserved_packet_buffers = EGRESS_TM_DF_RES_BUFFERS;
            queue_cfg->rl_cfg = 0;
        } else {
            if (rdpa_egress_tm_queue_cfg_get(tm_obj, qid, queue_cfg)) {
                queue_cfg->drop_threshold = EGRESS_TM_DF_QUEUE_THRESHOLD;
                queue_cfg->reserved_packet_buffers = EGRESS_TM_DF_RES_BUFFERS;
                queue_cfg->rl_cfg = 0;                
            }
        }

        if (mqprio->flags & TC_MQPRIO_F_SHAPER) {
            rdpa_rl_cfg_t rl_cfg = {0};

            if (queue_cfg->rl_cfg)
                bdmf_destroy(queue_cfg->rl_cfg);

            mattr = bdmf_mattr_alloc(rdpa_rate_limit_drv());
            if (!mattr)
                return -ENOMEM;

            rc = bdmf_new_and_set(rdpa_rate_limit_drv(), NULL, mattr,
                            &queue_cfg->rl_cfg);
            bdmf_mattr_free(mattr);
            if (rc)
                return -ENOMEM;
            if (mqprio->flags & TC_MQPRIO_F_MIN_RATE)
                rl_cfg.af_rate = mqprio->min_rate[qid];
            if (mqprio->flags & TC_MQPRIO_F_MAX_RATE)
                rl_cfg.be_rate = mqprio->max_rate[qid];
            rdpa_rate_limit_cfg_set(queue_cfg->rl_cfg, &rl_cfg);
        }
        queue_cfg->queue_id = qid;
    }

    return 0;
}

int port_runner_tc_config_mqprio(enetx_port_t *self, struct tc_mqprio_qopt_offload *mqprio)
{
    bdmf_object_handle port_obj = self->priv;
    bdmf_mattr_handle tm_attr = NULL;
    bdmf_object_handle tm_obj = NULL;
    void *queue_cfg;
    rdpa_port_tm_cfg_t port_tm_cfg;
    char port_name[IFNAMSIZ];
    int rc = 0;

    if ((rc = rdpa_port_name_get(port_obj, port_name, IFNAMSIZ)))
    {
        egress_tm_err("Failed to get RDPA port name. rc=%d\n", rc);
        return -EINVAL;
    }

    rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg);
    if (!port_tm_cfg.sched) {
        egress_tm_err("There is something wrong, the port %s does not have a configured Egress TM\n", port_name);
        return -EINVAL;
    }

    tm_attr = bdmf_mattr_alloc(rdpa_egress_tm_drv());
    if (!tm_attr)
        return -ENOMEM;
    rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_queue);
    rdpa_egress_tm_mode_set(tm_attr, rdpa_tm_sched_sp);
    rdpa_egress_tm_num_queues_set(tm_attr, mqprio->qopt.num_tc);
    rdpa_egress_tm_rl_rate_mode_set(tm_attr, rdpa_tm_rl_single_rate);

    queue_cfg = kzalloc(mqprio->qopt.num_tc * sizeof(rdpa_tm_queue_cfg_t), GFP_KERNEL);
    if (!queue_cfg) {
        rc = -ENOMEM;
        goto exit;
    }

    if ((rc = runner_prepare_egress_tm_mqprio_queues_cfg(port_tm_cfg.sched, queue_cfg, mqprio)))
    {
        egress_tm_err("Failed to prepare RDPA egress tm for port %s. rc=%d\n", port_name, rc);    
        goto exit;       
    }

    bdmf_destroy(port_tm_cfg.sched);
    if (bdmf_new_and_set(rdpa_egress_tm_drv(), port_obj, tm_attr, &tm_obj))
    {
        rc = -EINVAL;
        goto exit;
    }

    if ((rc = tc_rdpa_egress_tm_queues_cfg(tm_obj, queue_cfg, mqprio->qopt.num_tc, 0)))
    {
        egress_tm_err("Failed to set RDPA egress tm queues for port %s. rc=%d\n", port_name, rc);
    }

exit:
    if(queue_cfg)
        kfree(queue_cfg);
    bdmf_mattr_free(tm_attr);
    return rc;
}

static struct egress_tm_sched_dict sched_dict[] = {
   {rdpa_tm_sched_sp},
   {rdpa_tm_sched_wrr},
   {rdpa_tm_sched_sp_wrr}
};

static struct qid_list_entry *port_runner_find_in_list(struct list_head *qid_list, int key)
{
    struct qid_list_entry *entry;

    list_for_each_entry(entry, qid_list, list) {
        if (entry->key == key)
            return entry;
    }
    return NULL;
}

static int port_runner_insert_into_list(struct list_head *qid_list, int key, bool print_error)
{
    struct qid_list_entry *new_entry;

    if (port_runner_find_in_list(qid_list, key)) {
        if (print_error)
            egress_tm_err("Key %d already in list", key);
        return -EINVAL;
    }

    new_entry = kmalloc(sizeof(*new_entry), GFP_KERNEL);
    if (!new_entry)
        return -ENOMEM;

    new_entry->key = key;
    list_add(&new_entry->list, qid_list);

    return 0;
}

static void port_runner_cleanup_list(struct list_head *qid_list)
{
    struct qid_list_entry *entry, *tmp;

    list_for_each_entry_safe(entry, tmp, qid_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }
}

egress_tm_node *port_runner_htb2ets(egress_tm_node *node, char *port_name)
{
    if (!node || node->num_of_childs > 1)
    {
        egress_tm_err("Port %s: single HTB class supported", port_name);
        return NULL;
    }

    node = list_first_entry(&node->childs, egress_tm_node, my_list);
    if (!node || node->num_of_childs > 1)
    {
        egress_tm_err("Port %s: single HTB class child supported", port_name);
        return NULL;
    }

    node = list_first_entry(&node->childs, egress_tm_node, my_list);
    if (!node)
        return NULL;

    if (node->type != EGRESS_TM_ETS_PARENT)
    {
        return NULL;
    }

    return node;
}

static unsigned int port_runner_nex_power_of_two(unsigned int n)
{
    unsigned int power = 1;

    if (n == 1)
        return 2;

    if ((n & (n - 1)) == 0) {
        return n;
    }

    while (power < n) {
        power <<= 1;
    }

    return power;
}

static int port_runner_validate_ets(egress_tm_node *node, struct list_head *qid_list, char *port_name)
{
    struct list_head piomap_list;
    bool apply_priomap = true;
    egress_tm_node *child;
#ifndef TM_C_CODE
    bool has_queues = false, has_subsidiaries = false;
#endif

    if (!node)
    {
        egress_tm_err("Port=%s NULL ETS node", port_name);
        return -EINVAL;
    }

    if (node->ets.level > 2) {
        egress_tm_err("Port=%s: Only two level queues supported", port_name);
        return -EINVAL;
    } else if (node->ets.level == 1) {
        if (bcm_license_check(BP3_FEATURE_SERVICE_QUEUE) <= 0) {
            egress_tm_err("Port=%s: there is no valid license for secondary level queues", port_name);
            return -EINVAL; 
        }

        if (node->ets.tm_sched_mode == EGRESS_TM_SP_WRR) {
            egress_tm_err("Port=%s: SP_WRR mode not supported on secondary Egress TM", port_name);
            return -EINVAL;           
        }

        if (node->ets.num_queues > 4) {
            egress_tm_err("Port=%s: up to 4 queues supported on secondary Egress TM", port_name);
            return -EINVAL;           
        }
    }

    if (node->ets.tm_sched_mode == EGRESS_TM_WRR) 
    {
        list_for_each_entry_reverse(child, &node->childs, my_list)
        {
            if (child->queue.weights < RDPA_MIN_WEIGHT || child->queue.weights > RDPA_MAX_WEIGHT)
            {
                egress_tm_err("Port=%s qid=%d illegal weight=%d", port_name, child->queue.qid, child->queue.weights);
                return -EINVAL;
            }
        }
    } else if (node->ets.tm_sched_mode == EGRESS_TM_SP_WRR)
    {
        if (!node->ets.num_sp_queues)
        {
            egress_tm_err("Port=%s Illegal number of sp queues, should be at least one SP queue in SP_WRR mode\n", port_name);
            return -EINVAL;
        }

        list_for_each_entry_reverse(child, &node->childs, my_list)
        {
            if (child->queue.weights > RDPA_MAX_WEIGHT)
            {
                egress_tm_err("Port=%s qid=%d illegal weight=%d", port_name, child->queue.qid, child->queue.weights);
                return -EINVAL;
            }
            if (apply_priomap && child->queue.weights && (child->queue.priomap < node->ets.num_sp_queues))
            {
                apply_priomap = false;
                egress_tm_info("Port=%s qid=%d: WRR priority is less than SP priority use default mapping\n", port_name, child->queue.qid); 
            }  
        }
    } else if (node->ets.tm_sched_mode != EGRESS_TM_SP) {
        egress_tm_err("Port=%s Illegal scheduler mode", port_name);
        return -EINVAL;
    }

    INIT_LIST_HEAD(&piomap_list);
    /* Check if secondary Egress TM exist and valid */
    list_for_each_entry_reverse(child, &node->childs, my_list)
    {
        egress_tm_node *node, *rl_node, *ets_node;

#ifndef TM_C_CODE
        if (child->queue.has_subsidiaries)
            has_subsidiaries = true;
        else
            has_queues = true;
#endif

        if (child->queue.priomap >= RDPA_MAX_EGRESS_QUEUES) {
            apply_priomap = false;
            egress_tm_info("Port=%s: priomap=%d larger than supported use default mapping\n", port_name, child->queue.priomap);           
        }

        if (apply_priomap && port_runner_insert_into_list(&piomap_list, child->queue.priomap, false)) {
            apply_priomap = false;
            egress_tm_info("Port=%s: duplicate priomap=%d use default mapping\n", port_name, child->queue.priomap);
        }

        if (!child->num_of_childs) {
            if (port_runner_insert_into_list(qid_list, child->queue.qid, true)) {
                egress_tm_err("Port=%s: duplicate qid=%d", port_name, child->queue.qid);
                return -EINVAL;
            }
            continue;
        }

        node = list_first_entry(&child->childs, egress_tm_node, my_list);
        /* Drop algo is last level, skip */
        if (EGRESS_IS_DROP_ALGO(node->type)) {
            if (port_runner_insert_into_list(qid_list, child->queue.qid, true)) {
                egress_tm_err("Port=%s: duplicate qid=%d", port_name, child->queue.qid);
                return -EINVAL;
            }
            continue;
        } else if (node->type == EGRESS_TM_ETS_PARENT)
        {
            if (port_runner_validate_ets(node, qid_list, port_name))
                return -EINVAL;
        } else if (node->type == EGRESS_TM_HTB_PARENT)
        {
            rl_node = list_first_entry(&node->childs, egress_tm_node, my_list);
            ets_node = port_runner_htb2ets(node, port_name);
            if (!ets_node) {
                if (port_runner_insert_into_list(qid_list, child->queue.qid, true)) {
                    egress_tm_err("Port=%s: duplicate qid=%d", port_name, child->queue.qid);
                    return -EINVAL;
                }
                continue; /* no secondary ETS, continue */
            }
            if (port_runner_validate_ets(ets_node, qid_list, port_name))
                return -EINVAL;
        } else
        {
            egress_tm_err("Port %s: Wrong(%d) root node type\n", port_name, node->type);
            return -EINVAL;
        }
    }

    if (apply_priomap) {
        /* Apply priomap config */
        list_for_each_entry_reverse(child, &node->childs, my_list)
        {
            child->queue.priority = child->queue.priomap;
        }
    }

#ifndef TM_C_CODE
    if (has_subsidiaries && has_queues)
    {
        egress_tm_err("Port=%s: GEN4 doesn't support both subsidiary TM's and queues in primary TM\n", port_name);
        return -EINVAL;        
    }
#endif

    if (node->ets.tm_sched_mode == EGRESS_TM_SP_WRR)
    {
        unsigned int wrr_index_offset, new_num_sp_queues;

        if (node->ets.num_sp_queues == rdpa_tm_num_sp_elem_2 || node->ets.num_sp_queues == rdpa_tm_num_sp_elem_4 || node->ets.num_sp_queues == rdpa_tm_num_sp_elem_8 ||
            node->ets.num_sp_queues == rdpa_tm_num_sp_elem_16)
        {
            return 0;
        }

        new_num_sp_queues = port_runner_nex_power_of_two(node->ets.num_sp_queues);
        if (new_num_sp_queues > rdpa_tm_num_sp_elem_16)
        {
            egress_tm_err("Port=%s: doesn't support %d SP queues in SP_WRR mode\n", port_name, node->ets.num_sp_queues);
            return -EINVAL;
        }

        wrr_index_offset = new_num_sp_queues - node->ets.num_sp_queues;
        node->ets.num_sp_queues = new_num_sp_queues;

        list_for_each_entry_reverse(child, &node->childs, my_list)
        {
            if (child->queue.weights)
                child->queue.priority += wrr_index_offset;
        }
        node->ets.num_queues += wrr_index_offset;
    }

    return 0;
}

void port_runner_ets2childs(egress_tm_node *node, egress_tm_node **rl_node, egress_tm_node **drop_node, char *port_name)
{
    *rl_node = NULL;
    *drop_node = NULL;

    if (!node || node->num_of_childs > 1)
    {
        egress_tm_err("Port %s: single ets child supported", port_name);
        return;
    }

    if (!node->num_of_childs)
        return;

    node = list_first_entry(&node->childs, egress_tm_node, my_list);
    if (node->type != EGRESS_TM_HTB_PARENT)
    {
        if (EGRESS_IS_DROP_ALGO(node->type)) {
            *drop_node = node;
            return;
        } else if (node->type != EGRESS_TM_HTB_PARENT) {
            egress_tm_err("Port %s: only HTB/RED supported as ETS childs", port_name);
            return;
        }
    }

    if (node->num_of_childs > 1)
    {
        egress_tm_err("Port %s: single HTB class supported", port_name);
        return;
    }

    node = list_first_entry(&node->childs, egress_tm_node, my_list);
    *rl_node = node;

    if (!node->num_of_childs)
        return;

    if (node->num_of_childs > 1)
    {
        egress_tm_err("Port %s: single HTB class child supported", port_name);
        return;
    }

    node = list_first_entry(&node->childs, egress_tm_node, my_list);
    if (EGRESS_IS_DROP_ALGO(node->type))
        *drop_node = node;
}

static int port_runner_validate_egress_tm(egress_tm_node *node, egress_tm_node **ets_node, egress_tm_node **rl_node,
                                          char *port_name)
{
    struct list_head qid_list;
    int rc;
 
    if (!node)
    {
        egress_tm_err("Port %s: Egress TM node not set\n", port_name);
        return -EINVAL;
    }

    INIT_LIST_HEAD(&qid_list);
    if (node->type == EGRESS_TM_ETS_PARENT)
    {
        *ets_node = node;
        rc = port_runner_validate_ets(node, &qid_list, port_name);
    } else if (node->type == EGRESS_TM_HTB_PARENT)
    {
        *ets_node = port_runner_htb2ets(node, port_name);
        *rl_node = list_first_entry(&node->childs, egress_tm_node, my_list);
        rc = port_runner_validate_ets(*ets_node, &qid_list, port_name);
    } else
    {
        egress_tm_err("Port %s: Wrong(%d) root node type\n", port_name, node->type);
        return -EINVAL;
    }

    port_runner_cleanup_list(&qid_list);
    return rc;
}

static int port_runner_validate_modify_egress_tm(bdmf_object_handle sched, egress_tm_node *ets_node, egress_tm_node *rl_node,
                                          char *port_name)
{
    rdpa_tm_sched_mode tm_mode;
    rdpa_tm_num_sp_elem num_sp_queues;
    uint8_t num_queues;
#ifndef TM_C_CODE
    egress_tm_node *child;
    bool has_queues = false, has_subsidiaries = false;

   list_for_each_entry_reverse(child, &ets_node->childs, my_list)
   {
        if (child->queue.has_subsidiaries)
            has_subsidiaries = true;
        else
            has_queues = true;
   }
#endif

    if (rl_node) {
        rdpa_tm_rl_rate_mode rl_mode;

        if (rl_node->state == EGRESS_NODE_NEW) {
            egress_tm_err("TM new RL configuration not supported for modify");
            egress_tm_err("For LAN port execute init first, for TCONT recreate TCONT node");
            return -EINVAL;
        }

        if (rdpa_egress_tm_rl_rate_mode_get(sched, &rl_mode)) {
            egress_tm_err("Failed to get TM rl rate mode");
            return -EINVAL;
        }
    } else {
        bdmf_object_handle rate_limit_obj = NULL;

        /* check if we already have rate limit object */
        rdpa_egress_tm_rate_limit_get(sched, &rate_limit_obj);
        if (rate_limit_obj) {
            egress_tm_err("TM mode has RL, it was removed from config, consider re-init TM");
            return -EINVAL;            
        }
    }

    if (rdpa_egress_tm_mode_get(sched, &tm_mode)) {
        egress_tm_err("Failed to get TM mode");
        return -EINVAL;
    }

    /* FIXME after Firmware API changed */
    if (sched_dict[ets_node->ets.tm_sched_mode].mode != tm_mode) {
        egress_tm_err("Change TM mode in modify not supported, consider re-init TM");
        return -EINVAL;       
    }

    if (rdpa_egress_tm_num_queues_get(sched, &num_queues))  {
        egress_tm_err("Failed to get TM num_queues");
        return -EINVAL;
    }

#ifdef TM_C_CODE
    if (ets_node->ets.num_queues != num_queues) {
        egress_tm_err("Num of queues change not supported conf(%d)/req(%d), consider re-init TM", num_queues, ets_node->ets.num_queues);
        return -EINVAL;       
    }
#else
    if (has_subsidiaries && (has_queues || num_queues))
    {
        egress_tm_err("Port=%s: GEN4 doesn't support both subsidiary TM's and queues in primary TM\n", port_name);
        return -EINVAL;        
    } else if (!has_subsidiaries && (ets_node->ets.num_queues != num_queues)) {
        egress_tm_err("Num of queues change not supported conf(%d)/req(%d), consider re-init TM", num_queues, ets_node->ets.num_queues);
        return -EINVAL;       
    }
#endif

    if (ets_node->ets.tm_sched_mode == EGRESS_TM_SP_WRR) {
        if (rdpa_egress_tm_num_sp_elements_get(sched, &num_sp_queues)) {
            egress_tm_err("Failed to get TM num_sp_elements");
            return -EINVAL;
        }

        if (ets_node->ets.num_sp_queues != num_sp_queues) {
            egress_tm_err("Num of SP queues change not supported, consider re-init TM");
            return -EINVAL;
        }
    }

    return 0;
}

static int port_runner_rl_config(bdmf_object_handle sched, egress_tm_node *rl_node, bdmf_object_handle rate_limit_obj)
{
    rdpa_rl_cfg_t rl_cfg = {0};

    rl_cfg.af_rate = BYTES_TO_BITS(rl_node->htb.rate);
    rl_cfg.be_rate = BYTES_TO_BITS(rl_node->htb.ceil) - rl_cfg.af_rate;

    /* If Rate Limit is above 100Gbit skip rate limit configuration */
    if (rl_cfg.af_rate >= EGRESS_TM_100GBIT) {
        egress_tm_info("Skip Rate Limit configuration\n");
        if (rate_limit_obj)
            bdmf_destroy(rate_limit_obj);
        rate_limit_obj = NULL;
        goto out;
    }

    if (rl_cfg.be_rate >= EGRESS_TM_100GBIT) {
        egress_tm_info("Skip BE rate configuration\n");
        rl_cfg.be_rate = 0;
    }

    if (rate_limit_obj == NULL)
    {
        /* need to create new rate limit object */
        bdmf_mattr_handle mattr = NULL;
        int ret;

        mattr = bdmf_mattr_alloc(rdpa_rate_limit_drv());
        if (!mattr)
        {
            egress_tm_err("bdmf_mattr_alloc failed");
            return -ENOMEM;
        }
  
        ret = bdmf_new_and_set(rdpa_rate_limit_drv(), NULL, mattr, &rate_limit_obj);
        rdpa_rate_limit_index_get(rate_limit_obj, &rl_node->htb.index);
        bdmf_mattr_free(mattr);
        if (ret)
        {
            if (rate_limit_obj)
                bdmf_destroy(rate_limit_obj);
            egress_tm_err("bdmf_new_and_set failed to create rate limit obj, ret[%d]", ret);
            return -EINVAL;
        }
    }

    /* here we have for sure rate limit object */
    rdpa_rate_limit_cfg_set(rate_limit_obj, &rl_cfg);
out:
    return rdpa_egress_tm_rate_limit_set(sched, rate_limit_obj);
}

static int port_runner_egress_tm_rl_set(bdmf_object_handle tm_get_attr, bdmf_object_handle tm_set_attr,
                                        egress_tm_node *rl_node)
{
    bdmf_object_handle rate_limit_obj = NULL;

    /* check if we already have rate limit object */
    if (tm_get_attr)
        if (rdpa_egress_tm_rate_limit_get(tm_get_attr, &rate_limit_obj))
            rate_limit_obj = NULL;
    if (!rl_node)
    {
        rdpa_egress_tm_rl_rate_mode_set(tm_set_attr, rdpa_tm_rl_single_rate);
        if (rate_limit_obj)
            bdmf_destroy(rate_limit_obj);
        return 0;
    }

    if ((rl_node->htb.ceil - rl_node->htb.rate))
        rdpa_egress_tm_rl_rate_mode_set(tm_set_attr, rdpa_tm_rl_dual_rate);
    else
        rdpa_egress_tm_rl_rate_mode_set(tm_set_attr, rdpa_tm_rl_single_rate);

    return port_runner_rl_config(tm_set_attr, rl_node, rate_limit_obj);
}

static int port_runner_egress_tm_rl_modify(bdmf_object_handle sched, egress_tm_node *rl_node)
{
    bdmf_object_handle rate_limit_obj = NULL;

    if (!rl_node)
        return 0;

    /* if RL in changed state, set already exist */
    if (rl_node->state == EGRESS_NODE_CHANGED && !rdpa_egress_tm_rate_limit_get(sched, &rate_limit_obj)) {
        return port_runner_rl_config(sched, rl_node, rate_limit_obj);
    }

    return 0;
}

static bool tc_rdpa_egress_queue_changed(rdpa_tm_queue_cfg_t queue_cfg, egress_tm_node *node, char *port_name)
{
    egress_tm_node *rl_node, *drop_node;

    /* New queue/changed queue parameters */
    if (node->state != EGRESS_NODE_CONF)
        return true;

    port_runner_ets2childs(node, &rl_node, &drop_node, port_name);
    if (rl_node) {
        /* New queue RL/changed queue RL parameters */
        if (!(rl_node->state == EGRESS_NODE_CONF))
            return true;
    } else if (queue_cfg.rl_cfg) {
        /* queue RL removed */
        return true;
    }

    if (drop_node) {
        /* New queue Drop algo/changed queue Drop algo parameters */
        if (!(drop_node->state == EGRESS_NODE_CONF))
            return true;
    } else if (queue_cfg.drop_alg != rdpa_tm_drop_alg_dt) {
        /* queue Drop algo removed */
        return true;
    }

    return false;
}

static int port_runner_set_queue_cfg(egress_tm_node *child, rdpa_tm_queue_cfg_t *queue_cfg, char *port_name)
{
    egress_tm_node *rl_node, *drop_node;
    bdmf_mattr_handle mattr = NULL;
    rdpa_rl_cfg_t rl_cfg = {0};
    int rc;

    queue_cfg->weight = child->queue.weights;
    queue_cfg->drop_threshold = child->queue.size;
    queue_cfg->reserved_packet_buffers = child->queue.reserved_packet_buffers;
    queue_cfg->queue_id = child->queue.qid;

    port_runner_ets2childs(child, &rl_node, &drop_node, port_name);

    /* Config queue Rate Limiter */
    if (rl_node)
    {
        if (queue_cfg->rl_cfg)
            bdmf_destroy(queue_cfg->rl_cfg);

        mattr = bdmf_mattr_alloc(rdpa_rate_limit_drv());
        if (!mattr)
            return -ENOMEM;

        rc = bdmf_new_and_set(rdpa_rate_limit_drv(), NULL, mattr,
                        &queue_cfg->rl_cfg);
        rdpa_rate_limit_index_get(queue_cfg->rl_cfg, &rl_node->htb.index);
        bdmf_mattr_free(mattr);
        if (rc)
            return -ENOMEM;

        rl_cfg.af_rate = BYTES_TO_BITS(rl_node->htb.rate);
        rl_cfg.be_rate = BYTES_TO_BITS(rl_node->htb.ceil) - rl_cfg.af_rate;

        rdpa_rate_limit_cfg_set(queue_cfg->rl_cfg, &rl_cfg);
    }
    rdpa_rate_limit_cfg_set(queue_cfg->rl_cfg, &rl_cfg);

    /* Config queue Drop algorithm */
    if (!drop_node)
        goto drop_alg;
    
    if (drop_node->type == EGRESS_TM_RED) {
        queue_cfg->drop_alg = rdpa_tm_drop_alg_red;
        queue_cfg->low_class.min_threshold  = drop_node->red.low_class.min;
        queue_cfg->low_class.max_threshold  = drop_node->red.low_class.max;
        queue_cfg->high_class.min_threshold = queue_cfg->drop_threshold;
        queue_cfg->high_class.max_threshold = queue_cfg->drop_threshold;
    } else if (drop_node->type == EGRESS_TM_GRED) {
        queue_cfg->drop_alg = rdpa_tm_drop_alg_wred;
        queue_cfg->low_class.min_threshold  = drop_node->red.low_class.min;
        queue_cfg->low_class.max_threshold  = drop_node->red.low_class.max;
        queue_cfg->low_class.max_drop_probability  = drop_node->red.low_class.drop_prob;
        queue_cfg->high_class.min_threshold = drop_node->red.high_class.min;
        queue_cfg->high_class.max_threshold = drop_node->red.high_class.max;
        queue_cfg->high_class.max_drop_probability  = drop_node->red.high_class.drop_prob;
    } else if (drop_node->type == EGRESS_TM_PIE || drop_node->type == EGRESS_TM_DUALQ_LAQM) {
        if (drop_node->type == EGRESS_TM_PIE)
            queue_cfg->drop_alg = rdpa_tm_drop_alg_pi2;
        else
            queue_cfg->drop_alg = rdpa_tm_drop_alg_dualq_laqm;
        queue_cfg->low_class.min_threshold  = 0;
        queue_cfg->low_class.max_threshold  = 0;
        queue_cfg->high_class.min_threshold = 0;
        queue_cfg->high_class.max_threshold = 0;
    } else {
        egress_tm_warn("Invalid drop algorithm, fallback to DT");
        goto drop_alg;
    }

    return 0;

drop_alg:
        queue_cfg->drop_alg = rdpa_tm_drop_alg_dt;
        queue_cfg->low_class.min_threshold  = 0;
        queue_cfg->low_class.max_threshold  = 0;
        queue_cfg->high_class.min_threshold = 0;
        queue_cfg->high_class.max_threshold = 0;
    return 0;
}

static void port_runner_egress_queue2_sub_rl_and_ets(egress_tm_node *queue, egress_tm_node **ets_node, egress_tm_node **rl_node, char *port_name)
{
    egress_tm_node *child;

    /* if subsidiary queue exist, find ETS and RL nodes */
    child = list_first_entry(&queue->childs, egress_tm_node, my_list);
    if (child->type == EGRESS_TM_ETS_PARENT)
    {
        *ets_node = child;
    } else if (child->type == EGRESS_TM_HTB_PARENT)
    {
        *ets_node = port_runner_htb2ets(child, port_name);
        *rl_node = list_first_entry(&child->childs, egress_tm_node, my_list);
    }
}

static int tc_rdpa_egress_tm_queues_modify(enetx_port_t *self, bdmf_object_handle tm_obj, egress_tm_node *node, int is_wan, char *port_name)
{
    rdpa_tm_queue_cfg_t queue_cfg;
    egress_tm_node *child;
    bdmf_error_t rc = 0;
    uint32_t queue_mappping = 0;
    uint8_t num_queues = -1, qid;
    bool has_queues = false;

    rdpa_egress_tm_num_queues_get(tm_obj, &num_queues);

    list_for_each_entry_reverse(child, &node->childs, my_list)
    {
        if (!child->queue.has_subsidiaries)
        {
            has_queues = true;
            break;
        }
    }

    list_for_each_entry_reverse(child, &node->childs, my_list)
    {
        queue_mappping |= BIT(child->queue.priority);

        if (child->queue.has_subsidiaries) {
            egress_tm_node *ets_node = NULL, *rl_node = NULL;
            bdmf_object_handle egress_tm;

            port_runner_egress_queue2_sub_rl_and_ets(child, &ets_node, &rl_node, port_name);
            /* GEN4 doesn't support subsidiary TM and queues in primary TM, so no need in delete */
#ifdef TM_C_CODE
            rdpa_egress_tm_queue_cfg_delete(tm_obj, child->queue.priority);
#endif
            if (ets_node && (ets_node->state != EGRESS_NODE_NEW) && !rdpa_egress_tm_subsidiary_get(tm_obj, child->queue.priority, &egress_tm))
            {
                bdmf_number conf_weight;

                rdpa_egress_tm_weight_get(egress_tm, &conf_weight);
                if (child->queue.weights != conf_weight)
                    rdpa_egress_tm_weight_set(egress_tm, child->queue.weights);

                runner_tc_modify_egress_tm(self, self->priv, egress_tm, port_name, list_first_entry(&child->childs, egress_tm_node, my_list));
            } else {
                if (child->state == EGRESS_NODE_CONF)
                    continue;

                tc_rdpa_egress_tm_subsidiary(tm_obj, is_wan, child, port_name, has_queues);
            }
            continue;
        }

        if (!rdpa_egress_tm_queue_cfg_get(tm_obj, child->queue.priority, &queue_cfg)) {
            if (!tc_rdpa_egress_queue_changed(queue_cfg, child, port_name))
                continue;
        } else {
            memset(&queue_cfg, 0, sizeof(rdpa_tm_queue_cfg_t));
        }

        if ((rc = port_runner_set_queue_cfg(child, &queue_cfg, port_name)))
        {
            egress_tm_err("Failed to configure RDPA egress tm queue %d. rc=%d\n", child->queue.qid, rc);
            return rc;
        }

        if ((rc = rdpa_egress_tm_queue_cfg_set(tm_obj, child->queue.priority, &queue_cfg)))
        {
            egress_tm_err("Failed to configure RDPA egress tm queue %d. rc=%d\n", child->queue.qid, rc);
        }
    }

    for (qid = 0; qid < RDPA_MAX_EGRESS_QUEUES; qid++)
    {
        if (queue_mappping & BIT(qid))
            continue;

        if (!rdpa_egress_tm_queue_cfg_get(tm_obj, qid, &queue_cfg) && (rc = rdpa_egress_tm_queue_cfg_delete(tm_obj, qid)))
        {
            egress_tm_err("Failed to configure RDPA egress tm queue %d. rc=%d\n", qid, rc);
        }
    }

    return rc;
}

static int tc_rdpa_egress_tm_queues_cfg_subsidiary(bdmf_object_handle tm_obj, void *queue_cfg_start, uint8_t num_queues)
{
    rdpa_tm_queue_cfg_t *queue_cfg;
    bdmf_error_t rc = 0;
    int qid;

    for (qid = 0; qid < num_queues; qid++)
    {
        queue_cfg = queue_cfg_start + sizeof(rdpa_tm_queue_cfg_t) * qid;
        if ((rc = rdpa_egress_tm_queue_cfg_set(tm_obj, qid, queue_cfg)))
        {
            egress_tm_err("Failed to configure RDPA egress tm queue %d queue_id %d. rc=%d\n", qid, queue_cfg->queue_id, rc);
            break;
        }
    }

    return rc;
}

#if defined(ENET_EPON)
static void runner_tc_config_ctc_static_egress_tm(bdmf_mattr_handle tm_attr, uint8_t num_queues, bool is_subsidiary)
{
    if (!is_subsidiary) {
        rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_egress_tm);
    } else {
        rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_queue);
        rdpa_egress_tm_weight_set(tm_attr, 0);
        if (num_queues == 1)
            rdpa_egress_tm_mode_set(tm_attr, rdpa_tm_sched_disabled);
    }
}
#endif

static int tc_rdpa_egress_conf_tm_subsidiary(bdmf_object_handle owner, int is_wan, int weight, void *queue_cfg_start, bdmf_index idx,
                                             egress_tm_node *ets_node, egress_tm_node *rl_node, bool has_queues)
{
    rdpa_tm_service_queue_t service_queue;
    bdmf_object_handle egress_tm;
    bdmf_mattr_handle mattr;
    bdmf_error_t rc = 0;

    if (!rdpa_egress_tm_subsidiary_get(owner, idx, &egress_tm)) {
        if (ets_node->state != EGRESS_NODE_NEW) {
            return tc_rdpa_egress_tm_queues_cfg_subsidiary(egress_tm, queue_cfg_start, ets_node->ets.num_queues);
        } else {
            bdmf_destroy(egress_tm);
            egress_tm = NULL;           
        }
    }

    mattr = bdmf_mattr_alloc(rdpa_egress_tm_drv());
    if (!mattr)
    {
        egress_tm_err("bdmf_mattr_alloc failed");
        goto err;
    }

    /* create queues */
    /* set level */
    if ((rc = rdpa_egress_tm_level_set(mattr, RDPA_TM_SECONDARY_LEVEL)))
    {
        egress_tm_err("Fail set rdpa_egress_tm_level_set rc=%d\n", rc);
        goto err_free_mattr;       
    }

    if (ets_node->ets.type == EGRESS_TM_TYPE_DUALQ_PI2)
    {
        if ((rc = rdpa_egress_tm_subsidiary_type_set(mattr, rdpa_subsidiary_l4s)))
        {
            egress_tm_err("Fail set rdpa_subsidiary_type rc=%d\n", rc);
            goto err_free_mattr;       
        }
    }

    if ((rc = rdpa_egress_tm_mode_set(mattr, sched_dict[ets_node->ets.tm_sched_mode].mode)))
    {
        egress_tm_err("Fail set rdpa_egress_tm_mode_set rc=%d\n", rc);
        goto err_free_mattr;       
    }

    service_queue.enable = has_queues ? RDPA_TM_SECONDARY_SERVICE_QUEUE_ENABLE : 0;
    if ((rc = rdpa_egress_tm_service_queue_set(mattr, &service_queue)))
    {
        egress_tm_err("Fail set rdpa_egress_tm_service_queue_set rc=%d\n", rc);
        goto err_free_mattr;       
    }

    if ((rc = rdpa_egress_tm_overall_rl_set(mattr, FALSE)))
    {
        egress_tm_err("Fail set rdpa_egress_tm_overall_rl_set rc=%d\n", rc);
        goto err_free_mattr;       
    }

    if ((rc = rdpa_egress_tm_num_queues_set(mattr, ets_node->ets.num_queues)))
    {
        egress_tm_err("Fail set rdpa_egress_tm_num_queues_set rc=%d\n", rc);
        goto err_free_mattr;       
    }

    if ((rc = rdpa_egress_tm_weight_set(mattr, weight)))
    {
        egress_tm_err("Fail set rdpa_egress_tm_weight_set rc=%d\n", rc);
        goto err_free_mattr;       
    }        

#ifdef RDPA_TM_SECONDARY_SP_ELEMENTS
    if (ets_node->ets.tm_sched_mode == EGRESS_TM_SP_WRR) {
        if ((rc = rdpa_egress_tm_num_sp_elements_set(mattr, ets_node->ets.num_sp_queues)))
        {
            egress_tm_err("Fail set rdpa_egress_tm_num_sp_elements_set rc=%d\n", rc);
            goto err_free_mattr;       
        }
    }
#endif
    if ((rc = rdpa_egress_tm_rl_rate_mode_set(mattr, rdpa_tm_rl_single_rate)))
    {
        egress_tm_err("Fail set rdpa_egress_tm_rl_rate_mode_set rc=%d\n", rc);
        goto err_free_mattr;       
    }

#if defined(ENET_EPON)
    if (is_wan)
    {
        rdpa_epon_mode epon_mode = runner_tc_get_epon_mode();
        
        if (epon_mode == rdpa_epon_ctc || epon_mode == rdpa_epon_cuc)
            runner_tc_config_ctc_static_egress_tm(mattr, ets_node->ets.num_queues, true);
    }
#endif

    if ((rc = bdmf_new_and_set(rdpa_egress_tm_drv(), owner, mattr, &egress_tm)))
    {
        egress_tm_err("Fail set bdmf_new_and_set rc=%d\n", rc);
        goto err_free_mattr;       
    }
    rdpa_egress_tm_index_get(egress_tm, &ets_node->ets.index);

    if ((rc = rdpa_egress_tm_subsidiary_set(owner, idx, egress_tm)))
    {
        egress_tm_err("Fail set rdpa_egress_tm_subsidiary_set rc=%d\n", rc);
        goto err_free_sched;       
    }
    if ((rc = tc_rdpa_egress_tm_queues_cfg_subsidiary(egress_tm, queue_cfg_start, ets_node->ets.num_queues)))
    {
        egress_tm_err("Fail set tc_rdpa_egress_tm_queues_cfg_subsidiary rc=%d\n", rc);
        goto err_free_sched;       
    }

    if (rl_node)
    {
        if ((rc = port_runner_rl_config(egress_tm, rl_node, NULL)))
        {
            egress_tm_err("Failed to set Egress TM RL. rc=%d\n", rc);    
            goto err_free_sched;       
        }
    }

    bdmf_mattr_free(mattr);
    return 0;

err_free_sched:
    bdmf_destroy(egress_tm);
err_free_mattr:
    bdmf_mattr_free(mattr);
err:
    return -EINVAL;
}

static int runner_prepare_egress_tm_queues_cfg(void *queue_cfg_start, egress_tm_node *node, char *port_name)
{
    bdmf_error_t rc = 0;
    rdpa_tm_queue_cfg_t *queue_cfg;
    egress_tm_node *child;
    int num = 0;

    list_for_each_entry_reverse(child, &node->childs, my_list)
    {
        if (child->queue.has_subsidiaries)
            continue;

        queue_cfg = queue_cfg_start + sizeof(rdpa_tm_queue_cfg_t) * num;
        queue_cfg->rl_cfg = 0;

        if ((rc = port_runner_set_queue_cfg(child, queue_cfg, port_name)))
        {
            egress_tm_err("Failed to configure RDPA egress tm queue %d. rc=%d\n", child->queue.priority, rc);
            return rc;
        }
        num++;
    }

    return 0;
}

static int tc_rdpa_egress_tm_prio_queues_cfg(bdmf_object_handle tm_obj, egress_tm_node *ets_node, char *port_name)
{
    rdpa_tm_queue_cfg_t queue_cfg;
    egress_tm_node *child;
    bdmf_error_t rc = 0;

    list_for_each_entry_reverse(child, &ets_node->childs, my_list)
    {
        if (child->queue.has_subsidiaries)
            continue;

        memset(&queue_cfg, 0, sizeof(rdpa_tm_queue_cfg_t));
        if ((rc = port_runner_set_queue_cfg(child, &queue_cfg, port_name)))
        {
            egress_tm_err("Failed to configure RDPA egress tm queue %d. rc=%d\n",child->queue.priority, rc);
            return -EINVAL;
        }

        if ((rc = rdpa_egress_tm_queue_cfg_set(tm_obj, child->queue.priority, &queue_cfg)))
        {
            egress_tm_err("Failed to configure RDPA egress tm queue %d. rc=%d\n", child->queue.priority, rc);
            return -EINVAL;
        }
    }

    return 0;
}

static int tc_rdpa_egress_tm_subsidiary(bdmf_object_handle tm_obj, int is_wan, egress_tm_node *child, char *port_name, bool has_queues)
{
    egress_tm_node *ets_node = NULL, *rl_node = NULL;
    bdmf_error_t rc = 0;
    void *queue_cfg = NULL;

    port_runner_egress_queue2_sub_rl_and_ets(child, &ets_node, &rl_node, port_name);
    if (!ets_node) {
        egress_tm_err("Port %s subsidiary %d ETS node not found\n", port_name, child->queue.qid);
        rc = -EINVAL;  
        goto exit;            
    }

    queue_cfg = kzalloc(ets_node->ets.num_queues * sizeof(rdpa_tm_queue_cfg_t), GFP_KERNEL);
    if (!queue_cfg)
    {
        egress_tm_err("No memory %d\n", ets_node->ets.num_queues);
        rc = -ENOMEM;
        goto exit;
    }

    if ((rc = runner_prepare_egress_tm_queues_cfg(queue_cfg, ets_node, port_name)))
    {
        egress_tm_err("Failed to prepare RDPA egress tm for port %s subsidiary %d. rc=%d\n", port_name, child->queue.qid, rc);
        goto exit;       
    }

    if ((rc = tc_rdpa_egress_conf_tm_subsidiary(tm_obj, is_wan, child->queue.weights, queue_cfg, child->queue.qid, ets_node, rl_node, has_queues)))
    {
        egress_tm_err("Failed to configure RDPA egress tm for port %s subsidiary %d. rc=%d\n", port_name, child->queue.qid, rc);          
    }

exit:
    if (queue_cfg)
        kfree(queue_cfg);
    return rc;
}

static int tc_rdpa_egress_conf_tm_subsidiaries(bdmf_object_handle tm_obj, int is_wan, egress_tm_node *node, char *port_name, bool has_queues)
{
    egress_tm_node *child;

    list_for_each_entry_reverse(child, &node->childs, my_list)
    {
        if (!child->queue.has_subsidiaries)
            continue;

        if (tc_rdpa_egress_tm_subsidiary(tm_obj, is_wan, child, port_name, has_queues)) {
            egress_tm_err("Port %s failed configure subsidiary %d\n", port_name, child->queue.qid);
            return -EINVAL;
        }
    }

    return 0;
}

void runner_tc_remove_rate_limiter(egress_tm_node *node)
{
    bdmf_object_handle rate_limit_obj = NULL;
    int rc;

    if ((node->state == EGRESS_NODE_NEW) || (node->htb.index == -1))
        return;

    rc = rdpa_rate_limit_get(node->htb.index, &rate_limit_obj);
    if (rc)
    {
        egress_tm_err("Failed to remove rate limiter %lld. rc=%d\n", node->htb.index, rc);
        return;
    }
    if (rate_limit_obj) {
        bdmf_put(rate_limit_obj);
        bdmf_destroy(rate_limit_obj);
    }
}

static void runner_tc_clean_up_ratelimiters(egress_tm_node *ets_node, egress_tm_node *rl_node)
{
    egress_tm_node *child;

    if (rl_node)
        runner_tc_remove_rate_limiter(rl_node);

    list_for_each_entry_reverse(child, &ets_node->childs, my_list)
    {
        egress_tm_node *node;

        if (!child->num_of_childs)
            continue;

        node = list_first_entry(&child->childs, egress_tm_node, my_list);
        /* Drop algo is last level, skip */
        if (EGRESS_IS_DROP_ALGO(node->type)) {
            continue;
        } else if (node->type == EGRESS_TM_ETS_PARENT) {
            runner_tc_clean_up_ratelimiters(node, NULL);
        } else if (node->type == EGRESS_TM_HTB_PARENT) {
            rl_node = list_first_entry(&node->childs, egress_tm_node, my_list);
            node = port_runner_htb2ets(node, NULL);
            if (!node) {
                runner_tc_remove_rate_limiter(rl_node);
                continue; /* no secondary ETS, continue */
            }
            runner_tc_clean_up_ratelimiters(node, rl_node);
        }
    }

}

static int runner_tc_config_egress_tm(enetx_port_t *self, bdmf_object_handle port_obj, bdmf_object_handle sched, char *port_name, egress_tm_node *node)
{
#if defined(ENET_EPON)
    rdpa_epon_mode epon_mode = runner_tc_get_epon_mode();
#endif
    bdmf_mattr_handle tm_attr = NULL;
    bdmf_object_handle tm_obj = NULL;
    egress_tm_node *ets_node = NULL, *rl_node = NULL, *child = NULL;
    bdmf_object_handle owner = port_obj;
    bdmf_boolean is_wan = false;
    bool has_queues = false;
    int rc = 0;

    if ((rc = port_runner_validate_egress_tm(node, &ets_node, &rl_node, port_name)))
        return rc;

    runner_tc_clean_up_ratelimiters(ets_node, rl_node);

    tm_attr = bdmf_mattr_alloc(rdpa_egress_tm_drv());
    if (!tm_attr)
        return -ENOMEM;

    list_for_each_entry_reverse(child, &ets_node->childs, my_list)
    {
        if (!child->queue.has_subsidiaries)
        {
            has_queues = true;
            break;
        }
    }

#ifdef TM_C_CODE
    rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_queue);
    rdpa_egress_tm_num_queues_set(tm_attr, ets_node->ets.num_queues);
#else
    if (has_queues) {
        rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_queue);
        rdpa_egress_tm_num_queues_set(tm_attr, ets_node->ets.num_queues);
    } else {
        rdpa_egress_tm_level_set(tm_attr, rdpa_tm_level_egress_tm);
    }
#endif
    rdpa_egress_tm_mode_set(tm_attr, sched_dict[ets_node->ets.tm_sched_mode].mode);
    if ((rc = port_runner_egress_tm_rl_set(sched, tm_attr, rl_node)))
    {
        egress_tm_err("Failed to set Egress TM RL for port %s. rc=%d\n", port_name, rc);    
        goto exit;       
    }
    if (ets_node->ets.tm_sched_mode == EGRESS_TM_SP_WRR)
        rdpa_egress_tm_num_sp_elements_set(tm_attr, ets_node->ets.num_sp_queues);

    if (self->port_type == PORT_TYPE_RUNNER_PORT)
        rdpa_port_is_wan_get(port_obj, &is_wan);
    else if (self->port_type == PORT_TYPE_RUNNER_EPON || self->port_type == PORT_TYPE_RUNNER_GPON)
        is_wan = true;

    if (self->port_type == PORT_TYPE_RUNNER_SW)
    {
        rdpa_tm_service_queue_t service_queue = {.enable = 1};

        rc = rdpa_egress_tm_service_queue_set(tm_attr, &service_queue);
        if (rc)
        {
            egress_tm_err("rdpa_egress_tm_service_queue_set failed, ret[%d]", rc);
            goto exit;
        }

        rc = rdpa_egress_tm_index_set(tm_attr, RDPA_EGRESS_TM_SVCQ_INDEX);
        if (rc)
        {
            egress_tm_err("rdpa_egress_tm_index_set (service_queue) failed, ret[%d]", rc);
            goto exit;
        }
    }

    if (sched) {
        if (self->port_type != PORT_TYPE_RUNNER_EPON) {
            if (self->port_type == PORT_TYPE_RUNNER_SW)
                bdmf_put(sched);
            bdmf_destroy(sched);
        } else {
            rdpa_egress_tm_rate_limit_set(sched, NULL);
        }
    }

#if defined(ENET_EPON)
    if ((ets_node->ets.type != EGRESS_TM_TYPE_CONTROL) && (self->port_type == PORT_TYPE_RUNNER_EPON  && !self->port_info.is_epon_ae) && (epon_mode == rdpa_epon_ctc || epon_mode == rdpa_epon_cuc))
        runner_tc_config_ctc_static_egress_tm(tm_attr, ets_node->ets.num_queues, false);

    if ((self->port_type == PORT_TYPE_RUNNER_EPON  && !self->port_info.is_epon_ae) && (ets_node->ets.type != EGRESS_TM_TYPE_CONTROL) && (epon_mode == rdpa_epon_ctc_dyn || epon_mode == rdpa_epon_cuc_dyn))
            owner = self->priv;
#endif
    if (bdmf_new_and_set(rdpa_egress_tm_drv(), owner, tm_attr, &tm_obj))
    {
        rc = -EINVAL;
        goto exit;
    }
    rdpa_egress_tm_index_get(tm_obj, &ets_node->ets.index);

    if ((rc = tc_rdpa_egress_tm_prio_queues_cfg(tm_obj, ets_node, port_name)))
    {
        egress_tm_err("Failed to set RDPA egress tm queues for port %s. rc=%d\n", port_name, rc);
        rc = -EINVAL;
        goto exit;
    }

    if ((rc = tc_rdpa_egress_conf_tm_subsidiaries(tm_obj, is_wan, ets_node, port_name, has_queues)))
    {
        egress_tm_err("Failed to set RDPA egress tm queues for port %s. rc=%d\n", port_name, rc);
    }

#if defined(ENET_EPON)
    if (self->port_type == PORT_TYPE_RUNNER_EPON && !self->port_info.is_epon_ae) {
        if (ets_node->ets.type == EGRESS_TM_TYPE_CONTROL) {
            rdpa_llid_control_egress_tm_set(port_obj, tm_obj);
        } else {
            if (epon_mode == rdpa_epon_dpoe)
                rdpa_llid_control_egress_tm_set(port_obj, tm_obj);
            rdpa_llid_egress_tm_set(port_obj, tm_obj);
        }
        if (sched)
            bdmf_destroy(sched);
    }
#endif

exit:
    bdmf_mattr_free(tm_attr);
    return rc;
}

static int runner_tc_modify_egress_tm(enetx_port_t *self, bdmf_object_handle port_obj, bdmf_object_handle sched, char *port_name, egress_tm_node *node)
{
    egress_tm_node *ets_node = NULL, *rl_node = NULL;
    bdmf_boolean is_wan = false;
    int rc = 0;

    if ((rc = port_runner_validate_egress_tm(node, &ets_node, &rl_node, port_name)))
        return rc;

    if (node->state == EGRESS_NODE_NEW) {
        egress_tm_err("Port %s TM cannot be modify, first run init", port_name);
        return -EINVAL;
    }

    if ((rc = port_runner_validate_modify_egress_tm(sched, ets_node, rl_node, port_name)))
        return rc;

    if ((rc = port_runner_egress_tm_rl_modify(sched, rl_node)))
    {
        egress_tm_err("Failed to set Egress TM RL for port %s. rc=%d", port_name, rc);    
        goto exit;       
    }

    if (self->port_type == PORT_TYPE_RUNNER_PORT)
        rdpa_port_is_wan_get(port_obj, &is_wan);
    else if (self->port_type == PORT_TYPE_RUNNER_EPON || self->port_type == PORT_TYPE_RUNNER_GPON)
        is_wan = true;

    if ((rc = tc_rdpa_egress_tm_queues_modify(self, sched, ets_node, is_wan, port_name)))
    {
        egress_tm_err("Failed to prepare RDPA egress tm for port %s. rc=%d", port_name, rc);    
        goto exit;       
    }

exit:
    return rc;
}

int port_runner_tc_config_egress_tm(enetx_port_t *self, bool init)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_tm_cfg_t port_tm_cfg;
    int rc = 0;

    rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg);
    if (init)
        rc = runner_tc_config_egress_tm(self, port_obj, port_tm_cfg.sched, self->name, self->root_node);
    else
        rc = runner_tc_modify_egress_tm(self, port_obj, port_tm_cfg.sched, self->name, self->root_node);

    if (rc)
        egress_tm_err("Failed to configure egress tm for port %s. rc=%d\n", self->name, rc);    

    return rc;
}

static egress_tm_node *port_runner_get_up_level_ets(egress_tm_node *entry)
{
    egress_tm_node *par_entry = entry->parent; /* parent CLASS */

    /* Check if root node */
    if (!par_entry)
        return NULL;

    /* Check if parent is ETS(queue) and return parent */
    if (par_entry->type == EGRESS_TM_ETS_CHILD)
        return par_entry;

    /* Check if grandparent is ETS(queue) and return parent */
    par_entry = par_entry->parent; /* parent Qdisc */
    par_entry = par_entry->parent; /* parent ETS CLASS? */
    if (par_entry && par_entry->type == EGRESS_TM_ETS_CHILD)
        return par_entry;    

    /* not supported topology */
    return NULL;
}

static void port_runner_egress_tm_update_queues_stat(bdmf_object_handle egress_tm, egress_tm_node *ets, char *port_name)
{
    bdmf_object_handle egress_tm_subsidiary = NULL;
    egress_tm_node *child;
    rdpa_stat_1way_t stat;
    int rc = 0;

    ets->ets.sent.packets = 0;
    ets->ets.sent.bytes = 0;
    ets->ets.dropped.packets = 0; 
    ets->ets.dropped.bytes = 0;

    /* Check if subsidiry Egress TM needed */
    if (ets->ets.level) {
        egress_tm_node *queue_parent = port_runner_get_up_level_ets(ets);

        if (!queue_parent) {
            egress_tm_warn("Port %s: to find parent of subsidiary ETS node(%llx)\n", port_name, ets->handle);
            return;
        }

        if (rdpa_egress_tm_subsidiary_get(egress_tm, queue_parent->queue.qid, &egress_tm_subsidiary)) {
            egress_tm_warn("Port %s: failed update queue %d statistic, subsidiary TM not found\n", port_name, queue_parent->queue.qid);
            return;
        }
        egress_tm = egress_tm_subsidiary;
    }

    list_for_each_entry_reverse(child, &ets->childs, my_list)
    {
        /* queue not initialized yet */
        if (child->state != EGRESS_NODE_CONF)
            continue;

        /* queue not initialized yet */
        if (child->queue.has_subsidiaries) {
            egress_tm_node *grandchild, *ets_subsidiary = NULL;

            /* if subsidiary queue exist, find ETS and RL nodes */
            grandchild = list_first_entry(&child->childs, egress_tm_node, my_list);
            if (grandchild->type == EGRESS_TM_ETS_PARENT)
                ets_subsidiary = grandchild;
            else if (grandchild->type == EGRESS_TM_HTB_PARENT)
                ets_subsidiary = port_runner_htb2ets(grandchild, port_name);

            if (!ets_subsidiary) {
                egress_tm_warn("Port %s: to find parent of subsidiary ETS node(%llx)\n", port_name, ets->handle);
                continue;
            }

            port_runner_egress_tm_update_queues_stat(egress_tm, ets_subsidiary, port_name);

            child->queue.sent.packets = ets_subsidiary->ets.sent.packets;
            child->queue.sent.bytes = ets_subsidiary->ets.sent.bytes;
            child->queue.dropped.packets = ets_subsidiary->ets.dropped.packets; 
            child->queue.dropped.bytes = ets_subsidiary->ets.dropped.bytes;
        } else {
            if ((rc = rdpa_egress_tm_queue_stat_get(egress_tm, child->queue.priority, &stat))) {
                egress_tm_warn("Port %s: failed update queue %d statistic, rc=%d\n", port_name, child->queue.qid, rc);
                continue;
            }

            child->queue.sent.packets = stat.passed.packets;
            child->queue.sent.bytes = stat.passed.bytes;
            child->queue.dropped.packets = stat.discarded.packets; 
            child->queue.dropped.bytes = stat.discarded.bytes;
        }

        ets->ets.sent.packets += child->queue.sent.packets;
        ets->ets.sent.bytes += child->queue.sent.bytes;
        ets->ets.dropped.packets += child->queue.dropped.packets; 
        ets->ets.dropped.bytes += child->queue.dropped.bytes;
    }
}

void port_runner_update_queues_stat(enetx_port_t *self, egress_tm_node *ets)
{
    bdmf_object_handle port_obj = self->priv;
    rdpa_port_tm_cfg_t port_tm_cfg;

    /* ETS not initialized yet */
    if (ets->state == EGRESS_NODE_NEW)
        return;

    rdpa_port_tm_cfg_get(port_obj, &port_tm_cfg);
    if (!port_tm_cfg.sched)
    {
        egress_tm_err("There is something wrong, the port %s does not have a configured Egress TM\n", self->name);
        return;
    }

    port_runner_egress_tm_update_queues_stat(port_tm_cfg.sched, ets, self->name);
}

void port_runner_svsq_update_queues_stat(enetx_port_t *self, egress_tm_node *ets)
{
    bdmf_object_handle egress_tm = NULL;
    /* ETS not initialized yet */
    if (ets->state == EGRESS_NODE_NEW)
        return;
    if (rdpa_egress_tm_get(RDPA_EGRESS_TM_SVCQ_INDEX, &egress_tm) || egress_tm == NULL) {
        egress_tm_err("There is something wrong, the port %s does not have a configured Egress TM\n", self->name);
        return;
    }
    port_runner_egress_tm_update_queues_stat(egress_tm, ets, self->name);
    bdmf_put(egress_tm);
}

int port_runner_tc_config_svsq_egress_tm(enetx_port_t *self, bool init)
{
    bdmf_object_handle egress_tm;
    int rc = 0;

    if (rdpa_egress_tm_get(RDPA_EGRESS_TM_SVCQ_INDEX, &egress_tm)) {
        egress_tm = NULL;
    }

    if (bcm_license_check(BP3_FEATURE_SERVICE_QUEUE) <= 0)
    {
      egress_tm_err("No valid license for the service queue");
      return -EINVAL;
    }

    if (init) {
       rc = runner_tc_config_egress_tm(self, NULL, egress_tm, self->name, self->root_node);
    } else {
        if (egress_tm == NULL)
        {
            egress_tm_err("Cannot modify, service queue Egress TM not created");
            return -EINVAL;
        }
        rc = runner_tc_modify_egress_tm(self, NULL, egress_tm, self->name, self->root_node);
        bdmf_put(egress_tm);
    }

    return rc;
}

void runner_tc_remove_egress_tm_node(enetx_port_t *self, int index, bool control)
{
    bdmf_object_handle egress_tm = NULL;

    if (rdpa_egress_tm_get(index, &egress_tm) || !egress_tm) {
        return;
    }

    bdmf_put(egress_tm);
    bdmf_destroy(egress_tm);
}

void runner_tc_get_caps(enetx_port_t *self, void *priv)
{
    egress_tm_parms *caps = priv;
    bdmf_boolean is_wan;

    rdpa_port_is_wan_get(self->priv, &is_wan);

    caps->queueShaper = TRUE;
    caps->dualRate = TRUE;
    caps->portShaper = FALSE;
    caps->schedCaps = EGRESS_TM_SP_CAPABLE;
    caps->dpiQueueExt = FALSE;
    if (self->port_type == PORT_TYPE_RUNNER_SW)
    {
        caps->portShaper = TRUE;
        caps->maxQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM;
        caps->maxSpQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM;
        caps->schedCaps |= EGRESS_TM_WRR_CAPABLE | EGRESS_TM_SP_WRR_CAPABLE;
#if defined(BCM_PON_XRDP) || defined(BCM_DSL_XRDP)
        caps->schedCaps |= EGRESS_TM_1LEVEL_CAPABLE;
#endif
        return;
    }

#if defined(RDPA_XRDP_US_DS_AGNOSTIC)
    /* Report the same TM capabilities for LAN and WAN interfaces. */
    caps->portShaper = self->port_type == PORT_TYPE_RUNNER_PORT ? TRUE : FALSE;
    caps->maxQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM;
    caps->maxSpQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM;
    caps->schedCaps |= EGRESS_TM_WRR_CAPABLE | EGRESS_TM_SP_WRR_CAPABLE;
    caps->schedCaps |= EGRESS_TM_1LEVEL_CAPABLE;
    caps->dualRate = TRUE;
    if (is_wan)
        caps->dpiQueueExt = TRUE;
    return;
#endif

    if (!is_wan)
    {
        caps->maxQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM;
        caps->maxSpQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM;
#if defined(BCM_PON_XRDP)
        caps->schedCaps |= EGRESS_TM_WRR_CAPABLE | EGRESS_TM_SP_WRR_CAPABLE | EGRESS_TM_1LEVEL_CAPABLE;
#else
        caps->queueShaper = FALSE;
#endif /* BCM_PON_XRDP */
        caps->portShaper = TRUE;
    }
    else
    {
        if (self->port_type == PORT_TYPE_RUNNER_EPON || self->port_type == PORT_TYPE_RUNNER_PORT)
            caps->portShaper = TRUE;

        if (runner_tc_is_static_ctc_mode(self->port_type, !is_wan))
        {
            caps->maxQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM_CTC;
            caps->maxSpQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM_CTC;
            caps->queueShaper = FALSE;
            caps->dualRate = FALSE;
        }
        else
        {
            caps->maxQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM;
            caps->maxSpQueues = TC_ETS_MAX_Q_SERVICE_QUEUE_TM;
            caps->schedCaps |=  EGRESS_TM_WRR_CAPABLE | EGRESS_TM_SP_WRR_CAPABLE;
#if defined(BCM_PON_XRDP)
            caps->schedCaps |= EGRESS_TM_1LEVEL_CAPABLE;
#endif  /* BCM_PON_XRDP */
            caps->dpiQueueExt = TRUE;
        }
    }
}

#if defined(ENET_GPON) || defined(ENET_EPON)
static egress_tm_node *port_runner_ets2pon(egress_tm_node *ets)
{
    if (ets->parent->type == EGRESS_TM_TCONT)
        return ets->parent;

    /* check if ETS under RL */
    if (ets->parent->type == EGRESS_TM_HTB_CHILD) {
        /* ETS->HTB child->HTB parent->TCONT/LLID node */
        if (ets->parent->parent->parent->type == EGRESS_TM_TCONT)
            return ets->parent->parent->parent;
        else if (ets->parent->parent->parent->type == EGRESS_TM_ETS_CHILD) {
            /* ETS secondary->HTB child->HTB parent->ETS primary child->ETS primary parent */
            return port_runner_ets2pon(ets->parent->parent->parent->parent);
        }
    }

    /* ETS secondary->ETS primary child->ETS primary parent */
    if (ets->parent->type == EGRESS_TM_ETS_CHILD)
        return port_runner_ets2pon(ets->parent->parent);

    return NULL;
}

/* In Car mode ETS or ETS->HTB is root */
static bool port_runner_gpon_is_car_mode(egress_tm_node *ets)
{
    if (!ets->parent)
        return true;

    if (ets->parent->type == EGRESS_TM_HTB_CHILD) {
        /* ETS->HTB child->HTB parent->root */
        if (!ets->parent->parent->parent)
            return true;
    }

    return false;
}

static int port_runner_pon_child_validate_egress_tm(egress_tm_node *node, char *port_name)
{
    if (!node || node->num_of_childs > 1)
    {
        egress_tm_err("Port %s: single TCONT/LLID child supported\n", port_name);
        return -EINVAL;
    }

    if (!node->num_of_childs)
    {
        egress_tm_warn("Port %s: TCONT/LLID(%d) doesn't has childs\n", port_name, node->ponc.id);
        return -EBUSY;
    }
    return 0;
}

static int port_runner_pon_validate_egress_tm(enetx_port_t *self, char *port_name)
{
    if (!self->root_node)
    {
        egress_tm_err("Port %s: Egress TM PON root node not set\n", port_name);
        return -EINVAL;
    }

    if (self->root_node->type == EGRESS_TM_PON)
    {
        if (self->port_type == PORT_TYPE_RUNNER_GPON || self->port_type == PORT_TYPE_RUNNER_EPON)
        {
            egress_tm_node *child;

            list_for_each_entry_reverse(child, &self->root_node->childs, my_list)
            {
                if (child->type != EGRESS_TM_TCONT)
                {
                    egress_tm_err("Port %s: PON child not TCONT\n", port_name);
                    return -EINVAL;                    
                }
            }        
        } else
        {
            egress_tm_err("Port %s: Not GPON/EPON Runner\n", port_name);
            return -EINVAL; 
        }
        return 0;
    }
    
    if (self->port_type == PORT_TYPE_RUNNER_EPON)
    {
        egress_tm_err("Port %s: root node is not EGRESS_TM_PON\n", port_name);
        return -EINVAL;         
    }

    return 0;
}

int runner_pon_get_rdpa_scheduler(enetx_port_t *self, int id, u8 control, bdmf_object_handle *pon_child, bdmf_object_handle *scheduler)
{
    int rc = 0;

    if (self->port_type == PORT_TYPE_RUNNER_GPON)
        rc = rdpa_tcont_get(id, pon_child);
    else
        rc = rdpa_llid_get(id, pon_child);

    if (rc) {
        egress_tm_err("Port %s %s(%d) wasn't created. rc=%d\n", self->name, EGRESS_TM_PON_NAME(self->port_type), id, rc);
        return -EINVAL;
    }

    if (self->port_type == PORT_TYPE_RUNNER_GPON) {
        rdpa_tcont_egress_tm_get(*pon_child, scheduler);
    } else {
        if (control)
            rdpa_llid_control_egress_tm_get(*pon_child, scheduler);
        else
            rdpa_llid_egress_tm_get(*pon_child, scheduler);
    }

    return 0;
}

void port_runner_pon_update_queues_stat(enetx_port_t *self, egress_tm_node *ets)
{
    bdmf_object_handle port_obj = self->priv;
    bdmf_object_handle pon_child = 0;
    egress_tm_node *pon_node;
    bdmf_object_handle scheduler = NULL;
    char port_name[IFNAMSIZ];
    int rc = 0;

    /* ETS not initialized yet */
    if (ets->state == EGRESS_NODE_NEW)
        return;

    if (port_runner_gpon_is_car_mode(ets)) {
        port_runner_update_queues_stat(self, ets);
        return;
    }

    if ((rc = rdpa_port_name_get(port_obj, port_name, IFNAMSIZ)))
    {
        egress_tm_err("Failed to get RDPA port name. rc=%d\n", rc);
        return;
    }

    pon_node = port_runner_ets2pon(ets);
    if (!pon_node)
    {
        egress_tm_err("Port %s failed to find ETS %s parrent\n", port_name, EGRESS_TM_PON_NAME(self->port_type));
        return;
    }

    /* Configure tcont/llid */
    if ((rc = runner_pon_get_rdpa_scheduler(self, pon_node->ponc.id, pon_node->ponc.control, &pon_child, &scheduler)))
    {
        egress_tm_err("There is something wrong, the port %s(%d) failed get Egress TM\n", port_name, pon_node->ponc.id);
        return;
    }

    if (!scheduler)
    {
        egress_tm_err("There is something wrong, the port %s(%d) does not have a configured Egress TM\n", port_name, pon_node->ponc.id);
        return;
    }

    port_runner_egress_tm_update_queues_stat(scheduler, ets, port_name);
}

#if defined(ENET_EPON)
static void runner_pon_tc_fix_queue_id(rdpa_epon_mode epon_mode, egress_tm_node *node, char *port_name)
{
    egress_tm_node *child;

    if (node->type == EGRESS_TM_HTB_PARENT)
        node = port_runner_htb2ets(node, port_name);

    if (epon_mode == rdpa_epon_dpoe) {
        node->ets.num_queues = 32;
        node->ets.num_sp_queues = 16;
        node->ets.tm_sched_mode = EGRESS_TM_SP_WRR;

        /* Fix DPOE queue ID. queue_cfg[0] is queue_id=121 and queue_cfg[n] is queue_id=n-1 */
        list_for_each_entry_reverse(child, &node->childs, my_list)
        {
            if (child->queue.priority == 0)
                child->queue.qid = RDPA_EPON_CONTROL_QUEUE_ID;
            else
                child->queue.qid = child->queue.priority - 1;
        }
    } else {
        egress_tm_node *grandchild, *great_grandchild;

        /* Fix CTC static queue ID order in subsidiary 6. queue_cfg[0] is queue_id=1 and queue_cfg[1] is queue_id=0 */
        list_for_each_entry_reverse(child, &node->childs, my_list)
        {
            if (child->queue.priority == 6) {
                grandchild = list_first_entry(&child->childs, egress_tm_node, my_list);
                list_for_each_entry_reverse(great_grandchild, &grandchild->childs, my_list)
                {
                    if (great_grandchild->queue.priority == 0)
                        great_grandchild->queue.qid = 1;
                    else
                        great_grandchild->queue.qid = 0;
                }
            }
        }
    }
}

static void runner_pon_tc_fix_control_queue_id(rdpa_epon_mode epon_mode, egress_tm_node *node)
{
    node->ets.type = EGRESS_TM_TYPE_CONTROL;
    node = list_first_entry(&node->childs, egress_tm_node, my_list);
    node->queue.qid = RDPA_EPON_CONTROL_QUEUE_ID;
}

static int port_runner_pon_validate_static_ctc_subsidiary_egress_tm(egress_tm_node *node, char *port_name)
{
    egress_tm_node *child;

    if (node->queue.qid >= TC_CTC_NUM_OF_SUBSIDIARIARIES)
    {
        egress_tm_err("Port %s: ETS classes ID should be less than %d\n", port_name, TC_CTC_NUM_OF_SUBSIDIARIARIES);
        return -EINVAL;
    }

    if (!node->queue.has_subsidiaries)
    {
        egress_tm_err("Port %s: ETS classes ID should have subsidiaries\n", port_name);
        return -EINVAL;
    }

    node = list_first_entry(&node->childs, egress_tm_node, my_list);
    if (node->type != EGRESS_TM_ETS_PARENT) {
        egress_tm_err("Port %s: ETS classes child should be ETS\n", port_name);
        return -EINVAL;
    }

    child = list_first_entry(&node->childs, egress_tm_node, my_list);
    if (node->queue.priority == 6) {
        if (child->num_of_childs != 2)
        {
            egress_tm_err("Port %s: subsidiary ETS 6 should has two queues\n", port_name);
            return -EINVAL;
        }
    } else {
        if (child->num_of_childs > 1)
        {
            egress_tm_err("Port %s: subsidiary ETS should has single queue\n", port_name);
            return -EINVAL;
        }
    }

    node = list_first_entry(&child->childs, egress_tm_node, my_list);
    if (node->num_of_childs)
    {
        egress_tm_err("Port %s: subsidiary queue drop algo child node not supported\n", port_name);
        return -EINVAL;
    }

    return 0;
}

static int port_runner_pon_validate_static_ctc_egress_tm(egress_tm_node *node, bool init, char *port_name)
{
    int num_of_control_tm = 0, num_of_data_tm = 0;
    egress_tm_node *child;

    if (!node || node->num_of_childs > 2)
    {
        egress_tm_err("Port %s: up to 2 LLID supported(control + data)\n", port_name);
        return -EINVAL;
    }

    /* Get ETS node */
    list_for_each_entry_reverse(child, &node->childs, my_list)
    {
        egress_tm_node *grandchild, *greatgrandchild;

        if (num_of_control_tm > 1 || num_of_data_tm > 1)
        {
            egress_tm_err("Port %s: only single data(%d) and control(%d) TM supported\n", port_name, num_of_data_tm, num_of_control_tm);
            return -EINVAL;
        }

        if (child->state == EGRESS_NODE_CHANGED)
            goto change_err;

        if (child->ponc.control) {
            num_of_control_tm++;
            continue;
        }

        grandchild = list_first_entry(&child->childs, egress_tm_node, my_list);
        if (grandchild->type != EGRESS_TM_ETS_PARENT)
        {
            egress_tm_err("Port %s: LLID child should be ETS\n", port_name);
            return -EINVAL;
        }

        if (grandchild->state == EGRESS_NODE_CHANGED)
            goto change_err;

        if (grandchild->num_of_childs != TC_CTC_NUM_OF_SUBSIDIARIARIES || grandchild->ets.num_subsidiaries != TC_CTC_NUM_OF_SUBSIDIARIARIES)
        {
            egress_tm_err("Port %s: First level ETS should have %d subsidiaries\n", port_name, TC_CTC_NUM_OF_SUBSIDIARIARIES);
            return -EINVAL;
        }

        list_for_each_entry_reverse(greatgrandchild, &grandchild->childs, my_list)
        {
            if (greatgrandchild->state == EGRESS_NODE_CHANGED)
                goto change_err;
            if (port_runner_pon_validate_static_ctc_subsidiary_egress_tm(greatgrandchild, port_name))
                return -EINVAL;
        }
        num_of_data_tm++;
    }

    return 0;

change_err:
    egress_tm_err("Port %s: CTC/CUC static doesn't support change\n", port_name);
    return -EINVAL;
}

static int port_runner_pon_control_validate_egress_tm(egress_tm_node *node, char *port_name)
{
    if (!node || node->num_of_childs > 1)
    {
        egress_tm_err("Port %s: only single control LLID supported\n", port_name);
        return -EINVAL;
    }

    /* Get ETS node */
    node = list_first_entry(&node->childs, egress_tm_node, my_list);
    if (node->type != EGRESS_TM_ETS_PARENT)
    {
        egress_tm_err("Port %s: control LLID child should be ETS\n", port_name);
        return -EINVAL;
    }

    if (node->num_of_childs != 1)
    {
        egress_tm_err("Port %s: only single queue for control Egress TM\n", port_name);
        return -EINVAL;
    }

    return 0;
}
#endif

int runner_pon_tc_config_egress_tm(enetx_port_t *self, bool init)
{
#if defined(ENET_EPON)
    rdpa_epon_mode epon_mode = runner_tc_get_epon_mode();
#endif
    bdmf_object_handle port_obj = self->priv;
    egress_tm_node *child;
    char port_name[IFNAMSIZ], pon_child_id[IFNAMSIZ];
    int rc = 0;

    if ((rc = rdpa_port_name_get(port_obj, port_name, IFNAMSIZ)))
    {
        egress_tm_err("Failed to get RDPA port name. rc=%d\n", rc);
        return -EINVAL;
    }

    if ((rc = port_runner_pon_validate_egress_tm(self, port_name)))
        return rc;

    if (self->root_node->type != EGRESS_TM_PON)
        return port_runner_tc_config_egress_tm(self, init);

#if defined(ENET_EPON)
    if (epon_mode == rdpa_epon_ctc || epon_mode == rdpa_epon_cuc) {
        if (port_runner_pon_validate_static_ctc_egress_tm(self->root_node, init, port_name)) {
            egress_tm_err("Port %s CTC/CUC static configuration validation failed\n", port_name);
            return -EINVAL;
        }
    }
#endif
    if (!init && self->root_node->state == EGRESS_NODE_NEW) {
        egress_tm_err("Port %s TM cannot be modify, first run init", port_name);
        return -EINVAL;
    }

    list_for_each_entry_reverse(child, &self->root_node->childs, my_list)
    {
        bdmf_object_handle pon_child = 0;
        char pon_child_name[IFNAMSIZ*2 + 1];
        bdmf_object_handle scheduler = NULL;
        egress_tm_node *node;

        rc = port_runner_pon_child_validate_egress_tm(child, port_name);
        if (rc == -EBUSY)
        {
            continue;
        } else if (rc)
        {
            egress_tm_err("Port %s %s validation failed. rc=%d\n", port_name,
                          EGRESS_TM_PON_NAME(self->port_type), rc);
            return rc;
        }

        if (child->ponc.control) {
#if defined(ENET_EPON)
            if ((epon_mode == rdpa_epon_ctc || epon_mode == rdpa_epon_cuc ||
                epon_mode == rdpa_epon_ctc_dyn || epon_mode == rdpa_epon_cuc_dyn))
            {
                if (port_runner_pon_control_validate_egress_tm(child, port_name))
                    return -EINVAL;
            }
            else
#endif
            {
                egress_tm_err("Port %s %s %d doesn't support control\n", port_name,
                            EGRESS_TM_PON_NAME(self->port_type), child->ponc.id);
                return -EINVAL;
            }
        }

        /* Configure tcont/llid */
        if ((rc = runner_pon_get_rdpa_scheduler(self, child->ponc.id, child->ponc.control, &pon_child, &scheduler)))
            return rc;

        if ((rc = rdpa_port_name_get(pon_child, pon_child_id, IFNAMSIZ)))
        {
            egress_tm_err("Failed to find PON(%d). rc=%d\n", child->ponc.id, rc);
            return -EINVAL;
        }
        sprintf(pon_child_name, "%s_%s", port_name, pon_child_id);
        
        node = list_first_entry(&child->childs, egress_tm_node, my_list);

#if defined(ENET_EPON)
        if (child->ponc.control) {
            if (epon_mode == rdpa_epon_ctc || epon_mode == rdpa_epon_ctc_dyn || epon_mode == rdpa_epon_cuc || epon_mode == rdpa_epon_cuc_dyn)
                runner_pon_tc_fix_control_queue_id(epon_mode, node);
        } else {
            if (epon_mode == rdpa_epon_dpoe || epon_mode == rdpa_epon_ctc || epon_mode == rdpa_epon_cuc)
                runner_pon_tc_fix_queue_id(epon_mode, node, port_name);
        }
#endif

        if (init || !scheduler) {
            rc = runner_tc_config_egress_tm(self, pon_child, scheduler, pon_child_name, node);
        } else {
            rc = runner_tc_modify_egress_tm(self, pon_child, scheduler, pon_child_name, node);
        }
        if (rc) {
            egress_tm_err("Failed to configure egress tm for port %s. rc=%d\n", pon_child_name, rc);
            return rc;
        }
    }
    return 0;
}
#endif

void runner_tc_remove_egress_tm(enetx_port_t *port, egress_tm_node *node)
{
    if (node->type == EGRESS_TM_HTB_CHILD) {
        runner_tc_remove_rate_limiter(node);
    } else if (node->type == EGRESS_TM_ETS_PARENT) {
        runner_tc_remove_egress_tm_node(port, node->ets.index, false);
    }
}
