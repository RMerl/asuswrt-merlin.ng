/*
* <:copyright-BRCM:2013-2015:proprietary:standard
*
*    Copyright (c) 2013-2015 Broadcom
*    All Rights Reserved
*
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
*
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
*
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
 :>
*/

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdd.h"
#include "rdpa_int.h"
#ifndef G9991
#include "rdd_tunnels_parsing.h"
#endif
#ifndef XRDP
#include "rdd_data_structures.h"
#include "rdpa_egress_tm_inline.h"
#ifndef LEGACY_RDP
#include "rdd_ip_flow.h"
#include "rdd_l4_filters.h"
#include "rdpa_rdd_map.h"
#else
#include "rdd_ih_defs.h"
#include "rdpa_rdd_inline.h"
#endif
#else /* XRDP */
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_map.h"
#include "rdpa_rdd_inline.h"
#include "rdd_defs.h"
#include "rdpa_filter.h"
#include "rdd_stubs.h"
#include "rdpa_natc_common_ex.h"
#endif
#include "rdpa_ip_class_int.h"
#include "rdpa_l2l3_common_ex.h"
#include "rdpa_port_int.h"
#include "rdpa_common.h"

/* TODO: Not sure why this define is needed; Hopefully we can find
 * some way to create the ip_class object with max_flows attribute
 * That will make it flexible enough and can be different for differet 
 * platforms and/or customer requirements */ 
#ifndef CONFIG_BCM_RUNNER_MAX_FLOWS
#define RDPA_MAX_IP_CLASS_FLOWS     (16*1024)
#else
#define RDPA_MAX_IP_CLASS_FLOWS     (CONFIG_BCM_RUNNER_MAX_FLOWS) 
#endif

#if ((defined(RDP)) && ((RDPA_MAX_IP_CLASS_FLOWS) > (16*1024)))
#error " Number of runner flows for RDP platforms cann't exceed 16K !"
#endif

struct bdmf_object *ip_class_object;
static int is_idx_pool_local;

static DEFINE_BDMF_FASTLOCK(ip_class_lock);

extern int ip_class_attr_fc_bypass_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
extern int ip_class_attr_key_type_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);    
int ip_class_attr_routed_mac_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
extern void ip_class_pre_init_ex(struct bdmf_object *mo);
extern void ip_class_post_init_ex(struct bdmf_object *mo);
extern void ip_class_destroy_ex(struct bdmf_object *mo);
extern int  ip_class_attr_l4_filter_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size);
extern int ip_class_attr_l4_filter_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size);
extern int ip_class_attr_l4_filter_cfg_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size);

extern const bdmf_attr_enum_table_t rdpa_l4_filter_index_enum_table;

static const bdmf_attr_enum_table_t rdpa_fc_bypass_fields_enum_table =
{
    .type_name = "fc_bypass", .help = "FlowCache Bypass Modes",
    .values = {
        {"pure_mac", RDPA_IP_CLASS_PURE_MAC},
        {"multicast_ip", RDPA_IP_CLASS_MC_IP},
        {"pure_ip", RDPA_IP_CLASS_PURE_IP},
        {"us_wlan", RDPA_IP_CLASS_US_WLAN},
        {"bypass_all", RDPA_IP_CLASS_BP_ALL},
        {NULL, 0}
    }
};

static const bdmf_attr_enum_table_t rdpa_key_type_fields_enum_table =
{
    .type_name = "rdpa_key_type", .help = "Ip Flow Key mask",
    .values = {
        {"five_tuple", RDPA_IP_CLASS_5TUPLE},
        {"six_tuple", RDPA_IP_CLASS_6TUPLE},
        {"three_tuple", RDPA_IP_CLASS_3TUPLE},
        {NULL, 0}
    }
};

/***************************************************************************
 * ip_class object type
 **************************************************************************/

/* converts from RDPA ip flow parameters to RDD ip flow parameters */
int ip_class_prepare_rdd_ip_flow_params(rdpa_ip_flow_info_t *const info,
    rdd_fc_context_t *rdd_ip_flow_ctx, bdmf_boolean is_new_flow)
{
    bdmf_boolean is_ecn_remark_en = 0;
#if defined(XRDP)   
    is_ecn_remark_en = info->key.dst_ip.family && rdd_ecn_remark_enable_get();
#endif
    return l2l3_prepare_rdd_flow_result(0, info->key.dir, info->key.dst_ip.family, &info->result,
        rdd_ip_flow_ctx, is_new_flow, is_ecn_remark_en); 
}

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int ip_class_pre_init(struct bdmf_object *mo)
{
    int i;

    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);

    ip_class_pre_init_ex(mo);

    for (i = 0; i < RDPA_MAX_ROUTED_MAC; i++)
        memset(&ip_class->routed_mac[i], 0, sizeof(bdmf_mac_t));

    ip_class->num_flows = 0;

    return BDMF_ERR_OK;
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int ip_class_post_init(struct bdmf_object *mo)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t idx, max_num_flows = RDPA_MAX_IP_CLASS_FLOWS;
#if !defined(XRDP)
    rdd_ic_context_t context = {};
    int rc;
#endif
    /* get the ip class method from the system */
    ip_class->op_method = _rdpa_system_init_cfg_get()->ip_class_method;

    is_idx_pool_local = 0;

    /* if working with flow cache, configure internal l2 flow in RDD */
    if (ip_class->op_method != rdpa_method_none)
    {
#if !defined(XRDP)
        /* configure internal ingress context to RDD */
        context.action = rdpa_forward_action_forward;
        context.rate_shaper = BDMF_INDEX_UNASSIGNED;
        context.policer = BDMF_INDEX_UNASSIGNED;
        context.subnet_id = RDD_SUBNET_FLOW_CACHE;

        rc = rdd_ic_context_cfg(rdpa_dir_ds, RDPA_FC_DS_IC_RESULT_ID, &context);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Internal ds eth flow RDD configuration failed, error %d\n", rc);
#endif
        /* Configure Pure IP mode: Enabled */
        rdd_3_tupples_ip_flows_enable(1);

        ip_class->fc_bypass_mask |= RDPA_IP_CLASS_MASK_PURE_IP;
    }
    
    /* Make sure index pool is initialized */
    if (!ip_class->flow_idx_pool_p)
    {
        int err = 0;
        /* initialize the flow_idx_pool */
        ip_class->flow_idx_pool_p = (rdpa_flow_idx_pool_t *)bdmf_alloc(sizeof(rdpa_flow_idx_pool_t));
        if (!ip_class->flow_idx_pool_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for rdpa_flow_idx_pool\n");
            return BDMF_ERR_NOMEM;
        }

        err = rdpa_flow_idx_pool_init(ip_class->flow_idx_pool_p, max_num_flows, "ip_class");
        if (err)
        {
            bdmf_free(ip_class->flow_idx_pool_p);
            ip_class->flow_idx_pool_p = NULL;
            return err;
        }

        is_idx_pool_local = 1;
    }
    else
    {
        max_num_flows = rdpa_flow_idx_pool_get_pool_size(ip_class->flow_idx_pool_p);
        /* Index pool already created; Must make sure it is created with enough indexes */
        if (RDPA_MAX_IP_CLASS_FLOWS != max_num_flows)
        {
            BDMF_TRACE_ERR("Index pool does not have enough indexes %u > %u\n", RDPA_MAX_IP_CLASS_FLOWS, max_num_flows);
            return BDMF_ERR_INTERNAL;
        }
    }

    ip_class->ctx_ext_idx = (uint32_t *)bdmf_alloc(sizeof(uint32_t) * max_num_flows);
    if (!ip_class->ctx_ext_idx)
    {
        BDMF_TRACE_ERR("Memory allocation failure for ctx_ext_idx\n");
        if (is_idx_pool_local)
        {
            rdpa_flow_idx_pool_exit(ip_class->flow_idx_pool_p);
            bdmf_free(ip_class->flow_idx_pool_p);
            ip_class->flow_idx_pool_p = NULL;
        }

        return BDMF_ERR_NOMEM;
    }

    for (idx = 0; idx < max_num_flows; idx++)
    {
        ip_class->ctx_ext_idx[idx] = RDD_FLOW_ID_INVALID;
    }

    ip_class_post_init_ex(mo);
    
    ip_class_object = mo;
    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "ip_class");
    /* ToDo: configure the filters  in RDD and IH */
    return 0;
}

static int ip_class_attr_flow_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);

    return rdpa_flow_get_next(ip_class->flow_idx_pool_p, index, RDPA_FLOW_TUPLE_L3);
}

static int _ip_class_remove_all_flows(struct bdmf_object *mo)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_index curr_idx = BDMF_INDEX_UNASSIGNED;
    int rc = 0;

    if (!ip_class->flow_idx_pool_p) /* Pool not created, nothing to do */
        return 0;

    while (!rc)
    {
        rc = ip_class_attr_flow_get_next(mo, NULL, &curr_idx);
        /* We count on the fact that index even if index is returned to the pool, we still can call for
         * rdpa_flow_idx_pool_next_in_use starting from the same index */
        if (!rc)
            ip_class_attr_flow_delete(mo, NULL, curr_idx);
    }

    BUG_ON(ip_class->num_flows != 0);
   
    return 0;
}

static void ip_class_destroy(struct bdmf_object *mo)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    _ip_class_remove_all_flows(mo);
    /* destroy rdpa_flow_idx_pool */
    if (ip_class->flow_idx_pool_p && is_idx_pool_local)
    {
        rdpa_flow_idx_pool_exit(ip_class->flow_idx_pool_p);
        bdmf_free(ip_class->flow_idx_pool_p);
    }
    ip_class->flow_idx_pool_p = NULL;
    bdmf_free(ip_class->ctx_ext_idx);
    ip_class->ctx_ext_idx = NULL;

    ip_class_destroy_ex(mo);

    /* ToDo: do more cleanups here */
    ip_class_object = NULL;
}

/** find ip_class object */
static int ip_class_get(struct bdmf_type *drv, struct bdmf_object *owner, const char *discr, struct bdmf_object **pmo)
{
    if (!ip_class_object)
        return BDMF_ERR_NOENT;
    *pmo = ip_class_object;
    return 0;
}

/* "pathstat" attribute "read" callback */
static int ip_class_attr_pathstat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    return ip_class_attr_pathstat_read_ex(mo, ad, index, val, size);    
}


/* "fc_bypass" attribute "write" callback */
static int ip_class_attr_fc_bypass_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return ip_class_attr_fc_bypass_write_ex(mo, ad, index, val, size);
}


/* "key_type" attribute "write" callback */
static int ip_class_attr_key_type_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t key_type = *(uint32_t *)val;
    int rc;

    if (ip_class->num_flows)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot change flow key type when there are flows configured\n");

    rc = ip_class_attr_key_type_write_ex(mo, ad, index, val, size);
    if (rc)
        return rc;

    ip_class->ip_key_type = key_type;
    return 0;
}


/* "routed_mac" attribute "write" callback */
static int ip_class_attr_routed_mac_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    return ip_class_attr_routed_mac_write_ex(mo, ad, index, val, size);
}

/* "routed_mac" attribute "read" callback */
static int ip_class_attr_routed_mac_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_mac_t *mac = (bdmf_mac_t *)val;

    *mac = ip_class->routed_mac[index];
    return 0;
}

/* "flush" attribute "write" callback */
static int ip_class_attr_flush_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    return _ip_class_remove_all_flows(mo);
}

#define ip_class_indexes_dump(ip_class) _ip_class_indexes_dump(__FUNCTION__, __LINE__, ip_class)
static void _ip_class_indexes_dump(const char *func, int line, ip_class_drv_priv_t *ip_class)
{
    bdmf_index idx_in_use = BDMF_INDEX_UNASSIGNED;
    uint32_t rdpa_flow_idx = 0;
    uint32_t rdpa_flow_id = 0;
    uint32_t rdd_flow_id = 0;
    int rc;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("%s:%d, Total %d indecies\n====================================\n", func, line, ip_class->num_flows);
    while (rdpa_flow_get_next(ip_class->flow_idx_pool_p, &idx_in_use, RDPA_FLOW_TUPLE_L3) == 0)
    {
        rdpa_flow_idx = idx_in_use;
        rc = rdpa_flow_get_ids(ip_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L3);
        if (!rc)
        {
            bdmf_trace("%d => ", (int)rdd_flow_id);
        }
    }
    bdmf_trace("\n");
}

int ip_class_attr_flow_add(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size)
{
    int rc;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;
    bdmf_index ctx_ext_index = RDD_FLOW_ID_INVALID;

    if (rdpa_flow_idx_pool_num_idx_in_use(ip_class->flow_idx_pool_p) >= RDPA_MAX_IP_CLASS_FLOWS)
        BDMF_TRACE_RET(BDMF_ERR_NORES, "Too many connections\n");

    rc = ip_class_attr_flow_add_ex(mo, ad, index, val, size, &ctx_ext_index);
    if (rc)
        return rc;

    if (rdpa_flow_idx_pool_get_index(ip_class->flow_idx_pool_p, &rdpa_flow_idx))
    {
        ip_class_attr_flow_delete_ex(mo, ad, *index);
        return BDMF_ERR_NORES;
    }
    /* set the rdpa_flow_id mapping for rdpa_flow_idx */
    rdpa_flow_id = rdpa_build_flow_id(*index, RDPA_FLOW_TUPLE_L3);
    rdpa_flow_idx_pool_set_id(ip_class->flow_idx_pool_p, rdpa_flow_idx, rdpa_flow_id);
    ip_class->ctx_ext_idx[rdpa_flow_idx] = ctx_ext_index;
    /* return rdpa_flow_idx to caller */
    *index = rdpa_flow_idx;

    bdmf_fastlock_lock(&ip_class_lock);
    ip_class->num_flows++;

    bdmf_fastlock_unlock(&ip_class_lock);
    ip_class_indexes_dump(ip_class);

    return 0;
}

int ip_class_attr_flow_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    int rc;
    ip_class_drv_priv_t *ip_class = (ip_class_drv_priv_t *)bdmf_obj_data(mo);

    uint32_t rdpa_flow_idx = index;

    rc = ip_class_attr_flow_delete_ex(mo, ad, rdpa_flow_idx);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&ip_class_lock);

    if (rdpa_flow_idx_pool_return_index(ip_class->flow_idx_pool_p, rdpa_flow_idx))
    {
        BDMF_TRACE_ERR("rdpa_flow_idx %u return failure\n", rdpa_flow_idx);
    }

    ip_class->ctx_ext_idx[rdpa_flow_idx] = RDD_FLOW_ID_INVALID;
   
    if (ip_class->num_flows == 0)
        goto exit;
    
    ip_class->num_flows--;

exit:
    bdmf_fastlock_unlock(&ip_class_lock);
    ip_class_indexes_dump(ip_class);

    return 0;
}

/*  ip_flow_key aggregate type */
struct bdmf_aggr_type ip_flow_key_type = {
    .name = "ip_flow_key", .struct_name = "rdpa_ip_flow_key_t",
    .help = "IP Flow Key",
    .fields = (struct bdmf_attr[])
    {
        { .name = "src_ip", .help = "Source IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ip_flow_key_t, src_ip)
        },
        { .name = "dst_ip", .help = "Destination IPv4/IPv6 address", .size = sizeof(bdmf_ip_t),
            .type = bdmf_attr_ip_addr, .offset = offsetof(rdpa_ip_flow_key_t, dst_ip)
        },
        { .name = "prot", .help = "IP protocol",
            .type = bdmf_attr_number, .size = sizeof(uint8_t), .offset = offsetof(rdpa_ip_flow_key_t, prot),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "src_port", .help = "Source port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_key_t, src_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "dst_port", .help = "Destination port", .size = sizeof(uint16_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_key_t, dst_port),
            .flags = BDMF_ATTR_UNSIGNED
        },
        { .name = "dir", .help = "Traffic direction",
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_dir_enum_table,
            .size = sizeof(rdpa_traffic_dir), .offset = offsetof(rdpa_ip_flow_key_t, dir)
        },
        { .name = "ingress_if", .help = "Ingress interface", 
            .type = bdmf_attr_enum, .ts.enum_table = &rdpa_if_enum_table,
            .size = sizeof(rdpa_if), .offset = offsetof(rdpa_ip_flow_key_t, ingress_if)
        },
        { .name = "tcp_pure_ack", .help = "TCP pure ACK flow", .size = sizeof(uint8_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_ip_flow_key_t, tcp_pure_ack),
            .flags = BDMF_ATTR_UNSIGNED
        },
        BDMF_ATTR_LAST
    },
};
DECLARE_BDMF_AGGREGATE_TYPE(ip_flow_key_type);

/*  ip_flow_info aggregate type */
struct bdmf_aggr_type ip_flow_info_type = {
    .name = "ip_flow_info", .struct_name = "rdpa_ip_flow_info_t",
    .help = "Fast IP Connection Info (key+result)",
    .fields = (struct bdmf_attr[]) {
        { .name = "hw_id", .help = "HW Flow ID",
            .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_UNSIGNED,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_ip_flow_info_t, hw_flow_id)
        },
        { .name = "key", .help = "IP flow key", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "ip_flow_key", .offset = offsetof(rdpa_ip_flow_info_t, key)
        },
        { .name = "result", .help = "IP flow result", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "ip_flow_result", .offset = offsetof(rdpa_ip_flow_info_t, result),
            .is_field_visible = l2l3_flow_result_is_field_visible
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(ip_flow_info_type);

/*  l4_filter_cfg aggregate type : l4 filter configuration */
/* not used for XRDP but must be decleared */
struct bdmf_aggr_type l4_filter_cfg_type = {
    .name = "l4_filter_cfg", .struct_name = "rdpa_l4_filter_cfg_t",
    .help = "L4 Protocol Filter Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "action", .help = "Filter action", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_forward_action_enum_table,
            .size = sizeof(rdpa_filter_action), .offset = offsetof(rdpa_l4_filter_cfg_t, action)
        },
        { .name = "protocol", .help = "IP protocol", .type = bdmf_attr_number,
            .flags = BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_HAS_DISABLE, .disable_val = RDPA_INVALID_PROTOCOL,
            .size = sizeof(uint8_t), .offset = offsetof(rdpa_l4_filter_cfg_t, protocol)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(l4_filter_cfg_type);


/* Object attribute descriptors */
static struct bdmf_attr ip_class_attrs[] = {
    { .name = "nflows", .help = "number of configured IP flows (5-tuple, 6-tuple, or 3-tuple)",
        .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
        .size = sizeof(uint32_t), .offset = offsetof(ip_class_drv_priv_t, num_flows)
    },
    { .name = "flow_idx_pool_ptr", .help = "Flow ID Pool Virtual Address", .size = sizeof(void *),
        .type = bdmf_attr_pointer, .offset = offsetof(ip_class_drv_priv_t, flow_idx_pool_p),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    { .name = "flow", .help = "5-tuple based IP flow entry",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "ip_flow_info", .array_size = RDPA_MAX_IP_CLASS_FLOWS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .read = ip_class_attr_flow_read, .write = ip_class_attr_flow_write,
        .add = ip_class_attr_flow_add, .del = ip_class_attr_flow_delete,
        .find = ip_class_attr_flow_find, .get_next = ip_class_attr_flow_get_next
    },
    { .name = "flow_stat", .help = "5-tuple based IP flow entry statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_MAX_IP_CLASS_FLOWS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
        .read = ip_class_attr_flow_stat_read, .get_next = ip_class_attr_flow_get_next
    },
    { .name = "flush", .help = "Flush flows", .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean, .write = ip_class_attr_flush_write
    },
    { .name = "l4_filter", .help = "L4 filter configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "l4_filter_cfg",
        .index_type = bdmf_attr_enum, .index_ts.enum_table = &rdpa_l4_filter_index_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG, .array_size = RDPA_MAX_L4_FILTERS,
        .read = ip_class_attr_l4_filter_cfg_read, .write = ip_class_attr_l4_filter_cfg_write
    },
    { .name = "l4_filter_stat", .help = "L4 filter statistics",
        .index_type = bdmf_attr_enum, .index_ts.enum_table = &rdpa_l4_filter_index_enum_table,
        .type = bdmf_attr_number, .size = sizeof(uint32_t), .data_type_name = "uint32_t",
        .array_size = RDPA_MAX_L4_FILTERS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT, .read = ip_class_attr_l4_filter_stat_read
    },
    { .name = "routed_mac", .help = "Router MAC address", .type = bdmf_attr_ether_addr,
        .array_size = RDPA_MAX_ROUTED_MAC,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_NULLCHECK,
        .size = sizeof(bdmf_mac_t), .offset = offsetof(ip_class_drv_priv_t, routed_mac),
        .read = ip_class_attr_routed_mac_read, .write = ip_class_attr_routed_mac_write
    },
    { .name = "fc_bypass", .help = "FlowCache Bypass Modes", .type = bdmf_attr_enum_mask,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .ts.enum_table = &rdpa_fc_bypass_fields_enum_table,
        .size = sizeof(rdpa_fc_bypass), .offset = offsetof(ip_class_drv_priv_t , fc_bypass_mask),
        .write = ip_class_attr_fc_bypass_write, .data_type_name = "rdpa_fc_bypass"
    },
    { .name = "key_type", .help = "IP class key type", .type = bdmf_attr_enum,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .ts.enum_table = &rdpa_key_type_fields_enum_table,
        .size = sizeof(rdpa_key_type), .offset = offsetof(ip_class_drv_priv_t , ip_key_type),
        .write = ip_class_attr_key_type_write, .data_type_name = "rdpa_key_type"
    },
    { .name = "pathstat", .help = "Ip class path entry statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_IP_CLASS_MAX_PATHS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_DEPRECATED | BDMF_ATTR_NOLOCK,
        .read = ip_class_attr_pathstat_read
    },
    { .name = "tcp_ack_prio", .help = "TCP pure ACK prioritization (common for L3 and L2)", .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_boolean), .offset = offsetof(ip_class_drv_priv_t, tcp_ack_prio),
        .write = ip_class_attr_tcp_ack_prio_write_ex
    },
    BDMF_ATTR_LAST
};


static int ip_class_drv_init(struct bdmf_type *drv);
static void ip_class_drv_exit(struct bdmf_type *drv);

struct bdmf_type ip_class_drv = {
    .name = "ip_class",
    .parent = "system",
    .description = "IP Flow Classifier",
    .drv_init = ip_class_drv_init,
    .drv_exit = ip_class_drv_exit,
    .pre_init = ip_class_pre_init,
    .post_init = ip_class_post_init,
    .destroy = ip_class_destroy,
    .get = ip_class_get,
    .extra_size = sizeof(ip_class_drv_priv_t),
    .aattr = ip_class_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_ip_class, ip_class_drv);

/* Init/exit module. Cater for GPL layer */
static int ip_class_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_ip_class_drv = rdpa_ip_class_drv;
    f_rdpa_ip_class_get = rdpa_ip_class_get;
#endif
    return 0;
}

static void ip_class_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_ip_class_drv = NULL;
    f_rdpa_ip_class_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get ip_class object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_ip_class_get(bdmf_object_handle *_obj_)
{
    if (!ip_class_object || ip_class_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(ip_class_object);
    *_obj_ = ip_class_object;
    return 0;
}

