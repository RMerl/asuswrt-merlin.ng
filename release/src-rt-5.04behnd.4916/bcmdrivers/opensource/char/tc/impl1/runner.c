/*
   <:copyright-BRCM:2024:DUAL/GPL:standard
   
      Copyright (c) 2024 Broadcom 
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

#include <net/pkt_cls.h>
#include "tc_int.h"
#include <rdpa_api.h>

static int runner_create_ic_object(rdpa_traffic_dir dir, rdpa_ic_cfg_t *cfg, bdmf_object_handle *ingress_class_obj)
{
    int rc;
    BDMF_MATTR_ALLOC(ingress_class_attrs, rdpa_ingress_class_drv());

    rc = rdpa_ingress_class_dir_set(ingress_class_attrs, dir);
    rc = rc ? : rdpa_ingress_class_cfg_set(ingress_class_attrs, cfg);
    rc = rc ? : bdmf_new_and_set(rdpa_ingress_class_drv(), NULL, ingress_class_attrs, ingress_class_obj);

    BDMF_MATTR_FREE(ingress_class_attrs);
    return rc;
}

static int runner_find_ic_rule(rdpa_traffic_dir rule_dir, rdpa_ic_cfg_t *rule_cfg,
    bdmf_object_handle *ingress_class_obj)
{
    int rc;
    bdmf_object_handle obj = NULL;
    rdpa_traffic_dir dir;
    rdpa_ic_cfg_t cfg;

    __debug("dir %d, type %d, mask hi 0x%08X, mask lo 0x%08X, prty %d\n", (int)rule_dir, (int)rule_cfg->type,
        (uint32_t)(rule_cfg->field_mask >> 32), (uint32_t)rule_cfg->field_mask, (int)rule_cfg->prty);

    while ((obj = bdmf_get_next(rdpa_ingress_class_drv(), obj, NULL)))
    {
        rc = rdpa_ingress_class_dir_get(obj, &dir);
        if (rc)
            continue;

        if (dir != rule_dir)
            continue;

        rc = rdpa_ingress_class_cfg_get(obj, &cfg);
        if (rc)
            continue;

        if (cfg.type != rule_cfg->type ||
            cfg.field_mask != rule_cfg->field_mask ||
            cfg.gen_rule_cfg1.type != rule_cfg->gen_rule_cfg1.type ||
            cfg.gen_rule_cfg1.offset != rule_cfg->gen_rule_cfg1.offset ||
            cfg.gen_rule_cfg1.mask != rule_cfg->gen_rule_cfg1.mask ||
            cfg.gen_rule_cfg2.type != rule_cfg->gen_rule_cfg2.type ||
            cfg.gen_rule_cfg2.offset != rule_cfg->gen_rule_cfg2.offset ||
            cfg.gen_rule_cfg2.mask != rule_cfg->gen_rule_cfg2.mask ||
            cfg.generic_filter_location != rule_cfg->generic_filter_location ||
            (rule_cfg->prty != 0xFF && cfg.prty != rule_cfg->prty))
        {
            continue;
        }

        *ingress_class_obj = obj;
        return 1;
    }

    return 0;
}

int tc_runner_create_ic_object(rdpa_traffic_dir dir, rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow)
{
    int rc = BDMF_ERR_OK;
    bdmf_object_handle ingress_class_obj = NULL;
    bdmf_index flow_idx;
    int ic_added = 0;

    __debug("dir %d, type %d, mask hi 0x%08X, mask lo 0x%08X, prty %d, location %d, action %d (reason %d)\n",
        (int)dir,(int)cfg->type, (uint32_t)(cfg->field_mask >> 32), (uint32_t)cfg->field_mask, (int)cfg->prty,
        (int)cfg->generic_filter_location, (int)flow->result.action, flow->result.trap_reason);

    bdmf_lock();

    if (!runner_find_ic_rule(dir, cfg, &ingress_class_obj))
    {
        rc = runner_create_ic_object(dir, cfg, &ingress_class_obj);
        ic_added = rc ? 0 : 1;
    }

    if (!rc)
    {
        rc = rdpa_ingress_class_flow_find(ingress_class_obj, &flow_idx, flow);
        if (rc)
        {
            __debug("Add ic flow\n");
            rc = rdpa_ingress_class_flow_add(ingress_class_obj, &flow_idx, flow);
            if (rc == BDMF_ERR_ALREADY)
                rc = BDMF_ERR_OK;
        }
    }

    if (ingress_class_obj)
    {
        if (!ic_added)
            bdmf_put(ingress_class_obj);
        else if (rc)
            bdmf_destroy(ingress_class_obj);
    }

    bdmf_unlock();
    return rc;
}

int tc_runner_del_ic_rule(rdpa_traffic_dir dir, rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow)
{
    int rc;
    bdmf_object_handle ingress_class_obj;
    bdmf_index flow_idx;
    bdmf_number nflows = BDMF_INDEX_UNASSIGNED;

    __debug("dir %d, type %d, mask hi 0x%08X, mask lo 0x%08X, prty %d, action %d\n", (int)dir, (int)cfg->type,
        (uint32_t)(cfg->field_mask >> 32), (uint32_t)cfg->field_mask, (int)cfg->prty, (int)flow->result.action);

    bdmf_lock();

    if (!runner_find_ic_rule(dir, cfg, &ingress_class_obj)) 
    {
        bdmf_unlock();
        return BDMF_ERR_NOENT;
    }

    rc = rdpa_ingress_class_flow_find(ingress_class_obj, &flow_idx, flow);
    if (!rc)
    {
        rc = rdpa_ingress_class_flow_delete(ingress_class_obj, flow_idx);
        if (!rc)
            rc = rdpa_ingress_class_nflow_get(ingress_class_obj, &nflows);
    }

    bdmf_put(ingress_class_obj);
    if (nflows == 0) 
        bdmf_destroy(ingress_class_obj);
    bdmf_unlock();
    return rc;
}

int tc_runner_get_ic_rule_stats(rdpa_traffic_dir dir, rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow,
    uint64_t *bytes, uint64_t *packets)
{
    int rc;
    rdpa_stat_t flow_stat;
    bdmf_object_handle ingress_class_obj;
    bdmf_index flow_idx;

    __debug("dir %d, type %d, mask hi 0x%08X, mask lo 0x%08X, prty %d, action %d\n", (int)dir, (int)cfg->type,
        (uint32_t)(cfg->field_mask >> 32), (uint32_t)cfg->field_mask, (int)cfg->prty, (int)flow->result.action);

    bdmf_lock();

    if (!runner_find_ic_rule(dir, cfg, &ingress_class_obj)) 
    {
        bdmf_unlock();
        return BDMF_ERR_NOENT;
    }

    rc = rdpa_ingress_class_flow_find(ingress_class_obj, &flow_idx, flow);
    if (!rc)
    {
        rc = rdpa_ingress_class_flow_stat_get(ingress_class_obj, flow_idx, &flow_stat);
        if (!rc)
        {
            *bytes = flow_stat.bytes;
            *packets = flow_stat.packets;
        }
    }

    bdmf_put(ingress_class_obj);
    bdmf_unlock();
    return rc;
}

int tc_runner_set_if_rule(bdmf_object_handle port, int filter_idx, rdpa_filter_ctrl_t *filter_ctrl, uint32_t filter_opaque)
{
    int rc;
    bdmf_object_handle filter_obj;
    bdmf_index udef_idx;
    int is_etype_udef = filter_idx >= RDPA_FILTER_ETYPE_UDEF_0 && filter_idx <= RDPA_FILTER_ETYPE_UDEF_3;

    __debug("enable %d, filter %d, action %d\n", (int)filter_ctrl->enabled, filter_idx,
        (int)filter_ctrl->action);

    if (is_etype_udef)
    {
        rc = rdpa_filter_get(&filter_obj);
        if (rc)
            return rc;

        udef_idx = filter_idx - RDPA_FILTER_ETYPE_UDEF_0;

        if (!filter_ctrl->enabled)
            filter_opaque = RDPA_FILTER_ETYPE_UNSET;
    }

    if (port)
    {
        if (is_etype_udef)
            rc = rdpa_filter_etype_udef_set(filter_obj, udef_idx, filter_opaque);
        rc = rc ? rc : rdpa_port_ingress_filter_set(port, filter_idx, filter_ctrl);
    }
    else
    {

        while (!rc && (port = bdmf_get_next(rdpa_port_drv(), port, NULL)))
        {
            if (is_etype_udef)
                /* Call for each port to raise the refcnt as each port can be considered
                   a "user" of the filter.
                   This yields the same state for all ports being added at once, 
                   or 1 by 1. */
                rc = rdpa_filter_etype_udef_set(filter_obj, udef_idx, filter_opaque);
            rc = rc ? rc : rdpa_port_ingress_filter_set(port, filter_idx, filter_ctrl);
        }
    }

    return rc;
}
