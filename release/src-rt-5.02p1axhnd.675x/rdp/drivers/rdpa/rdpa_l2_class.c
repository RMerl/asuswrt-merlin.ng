/*
* <:copyright-BRCM:2013-2017:proprietary:standard
*
*    Copyright (c) 2013-2017 Broadcom
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
#include "rdpa_rdd_map.h"
#include "rdpa_rdd_inline.h"
#include "rdd_defs.h"
#include "rdpa_l2_class_ex.h"
#include "rdpa_l2l3_common_ex.h"
#include "rdpa_port_int.h"
#include "rdpa_common.h"
#include "rdpa_flow_idx_pool.h"

#ifndef CONFIG_BCM_RUNNER_MAX_FLOWS
#define RDPA_MAX_L2_CLASS_FLOWS     (16*1024)
#else
#define RDPA_MAX_L2_CLASS_FLOWS     (CONFIG_BCM_RUNNER_MAX_FLOWS) 
#endif

#if ((defined(RDP)) && ((RDPA_MAX_L2_CLASS_FLOWS) > (16*1024)))
#error " Number of runner flows for RDP platforms cann't exceed 16K !"
#endif

struct bdmf_object *l2_class_object;
extern struct bdmf_object *ip_class_object;
static int is_idx_pool_local;

static DEFINE_BDMF_FASTLOCK(l2_class_lock);

/***************************************************************************
 * l2_class object type
 **************************************************************************/

/* converts from RDPA L2 flow parameters to RDD context */
int l2_class_prepare_rdd_l2_flow_context(rdpa_l2_flow_info_t *const info,
    rdd_fc_context_t *rdd_ip_flow_ctx, bdmf_boolean is_new_flow)
{
    bdmf_boolean is_ecn_remark_en = 0;
#if defined(XRDP)   
    is_ecn_remark_en = (info->key.eth_type == LAYER2_HEADER_ETHER_TYPE_IPV6 && rdd_ecn_remark_enable_get());
#endif
    return l2l3_prepare_rdd_flow_result(1, rdpa_if_is_wan(info->key.ingress_if) ? rdpa_dir_ds : rdpa_dir_us,
        0, /* ip_family is ignored for l2 flow */
        &info->result, rdd_ip_flow_ctx, is_new_flow, is_ecn_remark_en);
}

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int l2_class_pre_init(struct bdmf_object *mo)
{
    int rc;
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);

    rc = l2_class_pre_init_ex(mo);
    if (rc)
        return rc;

    l2_class->num_flows = 0;
    l2_class->key_exclude_fields = 0;

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
static int l2_class_post_init(struct bdmf_object *mo)
{
    rdpa_key_type ip_class_key_type;
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);

    if (ip_class_object)
    {
        rdpa_ip_class_key_type_get(ip_class_object, &ip_class_key_type);
        if (ip_class_key_type == RDPA_IP_CLASS_3TUPLE)
            BDMF_TRACE_RET(BDMF_ERR_NOT_SUPPORTED, "IP Class 3-tuple lookup cannot coexist with L2 Class\n");
    }
    
    is_idx_pool_local = 0;
    l2_class_post_init_ex(mo);
    
    l2_class_object = mo;

        /* Make sure index pool is initialized */
    if (!l2_class->flow_idx_pool_p)
    {
        int err = 0;
        /* initialize the flow_idx_pool */
        l2_class->flow_idx_pool_p = (rdpa_flow_idx_pool_t *)bdmf_alloc(sizeof(rdpa_flow_idx_pool_t));
        if (!l2_class->flow_idx_pool_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for rdpa_flow_idx_pool\n");

            return BDMF_ERR_NOMEM;
        }
        err = rdpa_flow_idx_pool_init(l2_class->flow_idx_pool_p, RDPA_MAX_L2_CLASS_FLOWS, "l2_class");
        if (err)
        {
            bdmf_free(l2_class->flow_idx_pool_p);
            l2_class->flow_idx_pool_p = NULL;
            return err;
        }
        is_idx_pool_local = 1;
    }
    else
    {
        /* Index pool already created; Must make sure it is created with enough indexes */
        if (RDPA_MAX_L2_CLASS_FLOWS != rdpa_flow_idx_pool_get_pool_size(l2_class->flow_idx_pool_p))
        {
            BDMF_TRACE_ERR("Index pool does not have enough indexes %u > %u\n", 
                           RDPA_MAX_L2_CLASS_FLOWS, rdpa_flow_idx_pool_get_pool_size(l2_class->flow_idx_pool_p));

            return BDMF_ERR_INTERNAL;
        }
    }

    l2_class->l2_flow_data_p = (l2_flow_data_entry_t *)bdmf_alloc(sizeof(l2_flow_data_entry_t)*RDPA_MAX_L2_CLASS_FLOWS);
    if (!l2_class->l2_flow_data_p)
    {
        BDMF_TRACE_ERR("Memory allocation failure for l2_flow_data_entry_t pool\n");
        if (is_idx_pool_local)
        {
            rdpa_flow_idx_pool_exit(l2_class->flow_idx_pool_p);
            bdmf_free(l2_class->flow_idx_pool_p);
        }
        return BDMF_ERR_NOMEM;
    }

    snprintf(mo->name, sizeof(mo->name), "l2_class");
    return 0;
}

static int l2_class_attr_flow_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);

    return rdpa_flow_get_next(l2_class->flow_idx_pool_p, index, RDPA_FLOW_TUPLE_L2);
}

static int _l2_remove_all_flows(struct bdmf_object *mo)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_index curr_idx = BDMF_INDEX_UNASSIGNED;
    int rc = 0;

    while (!rc)
    {
        rc = l2_class_attr_flow_get_next(mo, NULL, &curr_idx);
        /* We count on the fact that index even if index is returned to the pool, we still can call for
         * rdpa_flow_idx_pool_next_in_use starting from the same index */
        if (!rc)
            l2_class_attr_flow_delete(mo, NULL, curr_idx);
    }

    BUG_ON(l2_class->num_flows != 0);
   
    return 0;
}

static void l2_class_destroy(struct bdmf_object *mo)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    _l2_remove_all_flows(mo);
    /* destroy rdpa_flow_idx_pool */
    if (is_idx_pool_local)
    {
        rdpa_flow_idx_pool_exit(l2_class->flow_idx_pool_p);
        bdmf_free(l2_class->flow_idx_pool_p);
    }
    l2_class->flow_idx_pool_p = NULL;
    /* Release  L2 data storage as well*/
    bdmf_free(l2_class->l2_flow_data_p);
    l2_class->l2_flow_data_p = NULL;
    l2_class_destroy_ex(mo);

    /* ToDo: do more cleanups here */
    l2_class_object = NULL;
}

/** find l2_class object */
static int l2_class_get(struct bdmf_type *drv, struct bdmf_object *owner, const char *discr, struct bdmf_object **pmo)
{
    if (!l2_class_object)
        return BDMF_ERR_NOENT;
    *pmo = l2_class_object;
    return 0;
}

/* "key_exclude_fields" attribute "write" callback */
static int l2_class_attr_key_exclude_fields_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_l2_flow_key_exclude_fields_t key_exclude_fields = *(rdpa_l2_flow_key_exclude_fields_t *)val;
    int rc;

    if (l2_class->num_flows)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot change flow key type when there are flows configured\n");

    rc = l2_class_attr_key_exclude_fields_write_ex(mo, ad, index, val, size);
    if (rc)
        return rc;
    l2_class->key_exclude_fields = key_exclude_fields;
    return 0;
}

/* "flush" attribute "write" callback */
static int l2_class_attr_flush_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    return _l2_remove_all_flows(mo);
}

#define l2_class_indexes_dump(l2_class) _l2_class_indexes_dump(__FUNCTION__, __LINE__, l2_class)
static void _l2_class_indexes_dump(const char *func, int line, l2_class_drv_priv_t *l2_class)
{
    bdmf_index idx_in_use = BDMF_INDEX_UNASSIGNED;
    uint32_t rdpa_flow_idx = 0;
    uint32_t rdpa_flow_id = 0;
    uint32_t rdd_flow_id = 0;
    int rc;

    if (bdmf_global_trace_level < bdmf_trace_level_debug)
        return;

    bdmf_trace("%s:%d, Total %d indecies\n====================================\n", func, line, l2_class->num_flows);
    while (rdpa_flow_get_next(l2_class->flow_idx_pool_p, &idx_in_use, RDPA_FLOW_TUPLE_L2) == 0)
    {
        rdpa_flow_idx = idx_in_use;
        rc = rdpa_flow_get_ids(l2_class->flow_idx_pool_p, rdpa_flow_idx, &rdpa_flow_id, &rdd_flow_id, RDPA_FLOW_TUPLE_L2);
        if (!rc)
        {
            bdmf_trace("%d => ", (int)rdd_flow_id);
        }
    }
    bdmf_trace("\n");
}

int l2_class_attr_flow_add(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index, const void *val,
    uint32_t size)
{
    int rc;
    l2_flow_data_entry_t *entry;
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_l2_flow_info_t *info = (rdpa_l2_flow_info_t *)val;
    uint32_t rdpa_flow_idx;
    uint32_t rdpa_flow_id;

    if (rdpa_flow_idx_pool_num_idx_in_use(l2_class->flow_idx_pool_p) >= RDPA_MAX_L2_CLASS_FLOWS)
        BDMF_TRACE_RET(BDMF_ERR_NORES, "Too many connections\n");

    rc = l2_class_attr_flow_add_ex(mo, ad, index, val, size);
    if (rc)
        return rc;

    if (rdpa_flow_idx_pool_get_index(l2_class->flow_idx_pool_p, &rdpa_flow_idx))
    {
        l2_class_attr_flow_delete_ex(mo, ad, *index);
        return BDMF_ERR_NORES;
    }
    /* set the rdd_flow_id and rdpa_flow_idx mapping */
    rdpa_flow_id = rdpa_build_flow_id(*index, RDPA_FLOW_TUPLE_L2);
    rdpa_flow_idx_pool_set_id(l2_class->flow_idx_pool_p, rdpa_flow_idx, rdpa_flow_id);
    /* return the rdpa_flow_idx to caller */
    *index = rdpa_flow_idx;

    entry = &(l2_class->l2_flow_data_p[rdpa_flow_idx]);

    memcpy(&entry->src_mac, &info->key.src_mac, sizeof(bdmf_mac_t));
    memcpy(&entry->dst_mac, &info->key.dst_mac, sizeof(bdmf_mac_t));

    bdmf_fastlock_lock(&l2_class_lock);
    l2_class->num_flows++;

    bdmf_fastlock_unlock(&l2_class_lock);
    l2_class_indexes_dump(l2_class);

    return 0;
}

int rdpa_l2_common_get_macs_by_idx(struct bdmf_object *mo, bdmf_index flow_idx, bdmf_mac_t *src_mac, bdmf_mac_t *dst_mac)
{
    l2_flow_data_entry_t *entry;
    uint32_t rdpa_flow_idx = flow_idx;
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);

    entry = &(l2_class->l2_flow_data_p[rdpa_flow_idx]);
    memcpy(src_mac, &entry->src_mac, sizeof(bdmf_mac_t));
    memcpy(dst_mac, &entry->dst_mac, sizeof(bdmf_mac_t));
    return 0;
}

int l2_class_attr_flow_delete(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index)
{
    int rc;
    l2_flow_data_entry_t *entry;
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdpa_flow_idx = index;

    rc = l2_class_attr_flow_delete_ex(mo, ad, rdpa_flow_idx);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&l2_class_lock);

    entry = &(l2_class->l2_flow_data_p[rdpa_flow_idx]);
    memset(entry, 0, sizeof(*entry));

    rdpa_flow_idx_pool_return_index(l2_class->flow_idx_pool_p, rdpa_flow_idx);

    BUG_ON(l2_class->num_flows == 0);
    l2_class->num_flows--;
    bdmf_fastlock_unlock(&l2_class_lock);
    l2_class_indexes_dump(l2_class);

    return 0;
}

int l2_flow_key_is_field_visible(rdpa_l2_flow_key_t *key, struct bdmf_attr *field,
    rdpa_l2_flow_key_exclude_fields_t key_exclude_fields);
static int _l2_flow_key_is_field_visible(struct bdmf_object *mo, struct bdmf_attr *ad, const void *val,
        struct bdmf_aggr_type *aggr, struct bdmf_attr *field)
{
    l2_class_drv_priv_t *l2_class = (l2_class_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_l2_flow_key_t *key = (rdpa_l2_flow_key_t *)val;

    return l2_flow_key_is_field_visible(key, field, l2_class->key_exclude_fields);
}

/*  l2_flow_info aggregate type */
struct bdmf_aggr_type l2_flow_info_type = {
    .name = "l2_flow_info", .struct_name = "rdpa_l2_flow_info_t",
    .help = "Fast L2 Connection Info (key+result)",
    .fields = (struct bdmf_attr[]) {
        { .name = "hw_id", .help = "HW Flow ID",
            .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_HEX_FORMAT | BDMF_ATTR_UNSIGNED,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_l2_flow_info_t, hw_flow_id)
        },
        { .name = "key", .help = "L2 flow key", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "l2_flow_key", .offset = offsetof(rdpa_l2_flow_info_t, key),
            .is_field_visible = _l2_flow_key_is_field_visible
        },
        { .name = "result", .help = "L2 flow result", .type = bdmf_attr_aggregate,
            .ts.aggr_type_name = "ip_flow_result", .offset = offsetof(rdpa_l2_flow_info_t, result),
            .is_field_visible = l2l3_flow_result_is_field_visible
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(l2_flow_info_type);

/* Object attribute descriptors */
static struct bdmf_attr l2_class_attrs[] = {
    { .name = "key_exclude_fields", .help = "List of fields to exclude from basic L2 class key",
        .type = bdmf_attr_enum_mask, .size = sizeof(uint32_t), .ts.enum_table = &rdpa_l2_flow_key_exclude_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(rdpa_l2_flow_key_exclude_fields_t), .offset = offsetof(l2_class_drv_priv_t , key_exclude_fields),
        .write = l2_class_attr_key_exclude_fields_write, .data_type_name = "rdpa_l2_flow_key_exclude_fields_t"
    },
    { .name = "nflows", .help = "number of configured L2 flows",
        .type = bdmf_attr_number, .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
        .size = sizeof(uint32_t), .offset = offsetof(l2_class_drv_priv_t, num_flows)
    },
    { .name = "flow_idx_pool_ptr", .help = "Flow ID Pool Virtual Address", .size = sizeof(void *),
        .type = bdmf_attr_pointer, .offset = offsetof(l2_class_drv_priv_t, flow_idx_pool_p),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
    },
    { .name = "flow", .help = "L2 flow entry",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "l2_flow_info", .array_size = RDPA_MAX_L2_CLASS_FLOWS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .read = l2_class_attr_flow_read_ex, .write = l2_class_attr_flow_write_ex,
        .add = l2_class_attr_flow_add, .del = l2_class_attr_flow_delete,
        .find = l2_class_attr_flow_find_ex, .get_next = l2_class_attr_flow_get_next
    },
    { .name = "flow_stat", .help = "L2 flow entry statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_MAX_L2_CLASS_FLOWS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_NOLOCK,
        .read = l2_class_attr_flow_stat_read_ex, .get_next = l2_class_attr_flow_get_next
    },
    { .name = "flush", .help = "Flush flows", .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean, .write = l2_class_attr_flush_write
    },
    { .name = "pathstat", .help = "L2 class path entry statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat", .array_size = RDPA_IP_CLASS_MAX_PATHS,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_DEPRECATED | BDMF_ATTR_NOLOCK,
        .read = l2_class_attr_pathstat_read_ex
    },
    BDMF_ATTR_LAST
};


static int l2_class_drv_init(struct bdmf_type *drv);
static void l2_class_drv_exit(struct bdmf_type *drv);

struct bdmf_type l2_class_drv = {
    .name = "l2_class",
    .parent = "system",
    .description = "L2 Flow Classifier",
    .drv_init = l2_class_drv_init,
    .drv_exit = l2_class_drv_exit,
    .pre_init = l2_class_pre_init,
    .post_init = l2_class_post_init,
    .destroy = l2_class_destroy,
    .get = l2_class_get,
    .extra_size = sizeof(l2_class_drv_priv_t),
    .aattr = l2_class_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_l2_class, l2_class_drv);

/* Init/exit module. Cater for GPL layer */
static int l2_class_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_l2_class_drv = rdpa_l2_class_drv;
    f_rdpa_l2_class_get = rdpa_l2_class_get;
#endif
    return 0;
}

static void l2_class_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_l2_class_drv = NULL;
    f_rdpa_l2_class_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get l2_class object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_l2_class_get(bdmf_object_handle *_obj_)
{
    if (!l2_class_object || l2_class_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(l2_class_object);
    *_obj_ = l2_class_object;
    return 0;
}

