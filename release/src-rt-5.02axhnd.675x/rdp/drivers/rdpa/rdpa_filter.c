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

#include "rdpa_filter_ex.h"
#include "rdpa_int.h"
#include "rdpa_rdd_inline.h"
#include "rdd.h"

uint8_t port_to_profile[rdpa_if__number_of] = {};
uint8_t profile_counter[NUM_OF_FILTER_PROFILES] = {};
rdpa_filter_ctrl_t filters_profiles[RDPA_FILTERS_QUANT][NUM_OF_FILTER_PROFILES] = {};

void _rdpa_filter_profiles_pre_init(void)
{
    uint8_t profile, filter;
    rdpa_if port;

    for (port = rdpa_if_first; port < rdpa_if__number_of; port++)
        port_to_profile[port] = INVALID_PROFILE_IDX;

    memset(profile_counter, 0, sizeof(profile_counter));

    for (filter = RDPA_FILTERS_BEGIN; filter < RDPA_FILTERS_QUANT; filter++)
    {
        for (profile = 0; profile < NUM_OF_FILTER_PROFILES; profile++)
        {
            filters_profiles[filter][profile].enabled = 0;
            filters_profiles[filter][profile].action = rdpa_forward_action_none;
        }
    }
}

static struct bdmf_object *filter_object;

static int filter_attr_global_cfg_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_global_cfg_t *global_cfg = (rdpa_filter_global_cfg_t *)val;

    /* Update framework */
    *global_cfg = priv->global_cfg;
    return 0;
}

static int filter_attr_etype_udef_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t *etype_udef = (uint16_t *)val;

    *etype_udef = priv->etype_udefs[index];
    return 0;
}

static int filter_attr_etype_udef_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    uint16_t *etype_udef = (uint16_t *)val;
    int rc;

    if (*etype_udef == (uint16_t)RDPA_VALUE_UNASSIGNED)
        *etype_udef = RDPA_FILTER_ETYPE_DUMMY;

    rc = filter_attr_etype_udef_write_ex(mo, ad, index, val, size);
    if (rc)
        return rc;

    priv->etype_udefs[index] = *etype_udef;
    return 0;
}

static int _filter_attr_etype_udef_get_next(struct bdmf_object *mo,
    uint8_t * const udef)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);

    for (; *udef < RDPA_FILTER_ETYPE_UDEFS_QUANT; ++(*udef))
    {
        if (priv->etype_udefs[*udef] != RDPA_FILTER_ETYPE_DUMMY)
            return 0;
    }

    return BDMF_ERR_NO_MORE;
}

static int filter_attr_etype_udef_get_next(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index *index)
{
    static uint8_t udef;
    int err;

    /* 1-st time: Locate first element */
    if (*index == BDMF_INDEX_UNASSIGNED)
        udef = 0;

    /* Find next element */
    err = _filter_attr_etype_udef_get_next(mo, &udef);
    if (err != 0) /* Not found */
        return err; /* End */

    /* Update framework */
    *((bdmf_index *)index) = udef;

    /* Locate next element */
    ++udef;
    return 0;
}

static int filter_attr_etype_udef_s_to_val(struct bdmf_object *mo,
    struct bdmf_attr *ad, const char *sbuf, void *val, uint32_t size)
{
    uint16_t *etype_udef = (uint16_t *)val;
    unsigned int etype_udef_ui;
    int err;

    if (!sbuf)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Filter: Ether-Type; Value - String is not empty\n");
    }

    if (strcmp(sbuf, RDPA_FILTER_VAL_DISABLE) == 0)
    {
        *etype_udef = (uint16_t) RDPA_VALUE_UNASSIGNED;
        return 0;
    }

    err = sscanf(sbuf, "%x", &etype_udef_ui);
    if (err != 1)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Filter: Ether-Type; Failed to convert value: String -> Integer\n");
    }

    *etype_udef = (uint16_t) etype_udef_ui;
    return 0;
}

static int filter_attr_tpid_vals_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_filter_tpid_vals_t *tpid_vals = (rdpa_filter_tpid_vals_t *)val;

    *tpid_vals = priv->tpid_vals;
    return 0;
}

static int filter_attr_tpid_val_ds_s_to_val(struct bdmf_object *mo,
    struct bdmf_attr *ad, const char *sbuf, void *val, uint32_t size)
{
    uint16_t *val_ds = (uint16_t *)val;
    unsigned int val_ds_ui;
    int err;

    /* Validate: Value - String is not empty */
    if (!sbuf)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Filter: TPID, Downstream; Value - String is not empty\n");
    }

    /* Handle 'Disable' indication */
    if (!strcmp(sbuf, RDPA_FILTER_VAL_DISABLE))
    {
        *val_ds = (uint16_t) RDPA_VALUE_UNASSIGNED;
        return 0;
    }

    /* Convert value: String -> Integer */
    err = sscanf(sbuf, "%x", &val_ds_ui);
    if (err != 1) /* Variables filled: One */
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Filter: TPID, Downstream; "
            "Failed to convert value: String -> Integer\n");
    }

    *val_ds = (uint16_t) val_ds_ui;
    return 0;
}

static int filter_attr_tpid_val_us_s_to_val(struct bdmf_object *mo,
    struct bdmf_attr *ad, const char *sbuf, void *val, uint32_t size)
{
    uint16_t *val_us = (uint16_t *)val;
    unsigned int val_us_ui;
    int err;

    /* Validate: Value - String is not empty */
    if (!sbuf)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
            "Filter: TPID, Upstream; Value - String is not empty\n");
    }

    /* Handle 'Disable' indication */
    if (strcmp(sbuf, RDPA_FILTER_VAL_DISABLE) == 0)
    {
        *val_us = (uint16_t) RDPA_VALUE_UNASSIGNED;
        return 0;
    }

    /* Convert value: String -> Integer */
    err = sscanf(sbuf, "%x", &val_us_ui);
    if (err != 1) /* Variables filled: One */
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Filter: TPID, Upstream; "
            "Failed to convert value: String -> Integer\n");
    }

    *val_us = (uint16_t) val_us_ui;
    return 0;
}

static int filter_attr_stats_get_next(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index *index)
{
    rdpa_filter_stats_key_t *key = (rdpa_filter_stats_key_t *)index;

    if (*index == BDMF_INDEX_UNASSIGNED)
    {
        key->filter = 0;
        key->dir = rdpa_dir_ds;
    }
    else
    {
        if (key->dir == rdpa_dir_us)
        {
            ++(key->filter);
            key->dir = rdpa_dir_ds;
        }
        else
            key->dir = rdpa_dir_us;
    }
    if (key->filter == RDPA_FILTERS_QUANT)
        return BDMF_ERR_NO_MORE;

    return 0;
}

struct bdmf_aggr_type filter_global_cfg_type =
{
    .name = "filter_global_cfg", .struct_name = "rdpa_filter_global_cfg_t",
    .help = "Global configuration",
    .fields = (struct bdmf_attr[])
    {
        { .name = "ls_enabled", .help = "Local switching enabled",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_filter_global_cfg_t, ls_enabled)
        },
        { .name = "cpu_bypass", .help = "Bypass filter for packets injected from CPU",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_filter_global_cfg_t, cpu_bypass)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(filter_global_cfg_type);

struct bdmf_aggr_type filter_oui_val_key =
{
    .name = "filter_oui_val_key", .struct_name = "rdpa_filter_oui_val_key_t",
    .help = "MAC Address OUI filter, Value: Key",
    .fields = (struct bdmf_attr[])
    {
        { .name = "ports", .help = "Ports (mask)", .type = bdmf_attr_enum_mask,
            .ts.enum_table = &rdpa_lan_wan_wlan_if_enum_table,
            .size = sizeof(rdpa_ports),
            .offset = offsetof(rdpa_filter_oui_val_key_t, ports)
        },
        { .name = "val_id", .help = "Value ID", .type = bdmf_attr_number,
            .size = sizeof(uint8_t),
            .offset = offsetof(rdpa_filter_oui_val_key_t, val_id)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(filter_oui_val_key);

struct bdmf_aggr_type filter_tpid_vals_type =
{
    .name = "filter_tpid_vals", .struct_name = "rdpa_filter_tpid_vals_t",
    .help = "TPID filter, Values",
    .fields = (struct bdmf_attr[])
    {
        { .name = "val_ds", .help = "Value, Downstream",
            .type = bdmf_attr_number, .size = sizeof(uint16_t),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
            .offset = offsetof(rdpa_filter_tpid_vals_t, val_ds),
            .s_to_val = filter_attr_tpid_val_ds_s_to_val
        },
        { .name = "val_us", .help = "Value, Upstream",
            .type = bdmf_attr_number, .size = sizeof(uint16_t),
            .flags = BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
            .offset = offsetof(rdpa_filter_tpid_vals_t, val_us),
            .s_to_val = filter_attr_tpid_val_us_s_to_val
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(filter_tpid_vals_type);

struct bdmf_aggr_type filter_key_type =
{
    .name = "filter_key", .struct_name = "rdpa_filter_key_t",
    .help = "Filter: Key",
    .fields = (struct bdmf_attr[])
    {
        { .name = "filter", .help = "Filter", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_filter_enum_table,
            .size = sizeof(rdpa_filter),
            .offset = offsetof(rdpa_filter_key_t, filter)
        },
        { .name = "ports", .help = "Ports (mask)",
            .type = bdmf_attr_enum_mask,
            .ts.enum_table = &rdpa_lan_wan_wlan_if_enum_table,
            .size = sizeof(rdpa_ports),
            .offset = offsetof(rdpa_filter_key_t, ports)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(filter_key_type);

struct bdmf_aggr_type filter_stats_key_type =
{
    .name = "filter_stats_key", .struct_name = "rdpa_filter_stats_key_t",
    .help = "Drop statistics: Key",
    .fields = (struct bdmf_attr[])
    {
        { .name = "filter", .help = "Filter", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_filter_enum_table,
            .size = sizeof(rdpa_filter),
            .offset = offsetof(rdpa_filter_stats_key_t, filter)
        },
        { .name = "dir", .help = "Direction", .type = bdmf_attr_enum,
            .ts.enum_table = &rdpa_traffic_dir_enum_table,
            .size = sizeof(rdpa_traffic_dir),
            .offset = offsetof(rdpa_filter_stats_key_t, dir)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(filter_stats_key_type);

/* Object attributes */
static struct bdmf_attr filter_attrs[] =
{
    { .name = "global_cfg", .help = "Global configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "filter_global_cfg",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = filter_attr_global_cfg_read, .write = filter_attr_global_cfg_write_ex,
    },
    { .name = "etype_udef", .help = "Ether-Type, User-Defined filter",
        .array_size = RDPA_FILTER_ETYPE_UDEFS_QUANT,
        .index_type = bdmf_attr_number, .type = bdmf_attr_number,
        .size = sizeof(uint16_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG |
            BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
        .read = filter_attr_etype_udef_read, .write = filter_attr_etype_udef_write,
        .get_next = filter_attr_etype_udef_get_next, .s_to_val = filter_attr_etype_udef_s_to_val
    },
    { .name = "oui_val", .help = "MAC Address OUI filter, Value",
        .array_size = RDPA_FILTER_OUI_VALS_SZ,
        .index_type = bdmf_attr_aggregate,
        .index_ts.aggr_type_name = "filter_oui_val_key",
        .type = bdmf_attr_number, .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG |
            BDMF_ATTR_UNSIGNED | BDMF_ATTR_HEX_FORMAT,
        .read = filter_attr_oui_val_read_ex, .write = filter_attr_oui_val_write_ex,
        .get_next = filter_attr_oui_val_get_next_ex, .s_to_val = filter_attr_oui_val_s_to_val_ex
    },
    { .name = "tpid_vals", .help = "TPID filter, Values",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "filter_tpid_vals",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .read = filter_attr_tpid_vals_read, .write = filter_attr_tpid_vals_write_ex,
    },
    { .name = "stats", .help = "Drop statistics",
        .array_size = RDPA_FILTER_STATS_SZ, .index_type = bdmf_attr_aggregate,
        .index_ts.aggr_type_name = "filter_stats_key", .type = bdmf_attr_number,
        .size = sizeof(uint32_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_UNSIGNED | BDMF_ATTR_WRITE,
        .read = filter_attr_stats_read_ex, .get_next = filter_attr_stats_get_next,
        .write = filter_attr_stats_write_ex
    },
    BDMF_ATTR_LAST
};

static int filter_pre_init(struct bdmf_object *mo)
{
    filter_drv_priv_t *priv = (filter_drv_priv_t *)bdmf_obj_data(mo);
    uint8_t cntr1, cntr2;

    /* 'global_cfg' */
    priv->global_cfg.ls_enabled = 0;

    /* 'etype_udef' */
    for (cntr1 = 0; cntr1 < RDPA_FILTER_ETYPE_UDEFS_QUANT; ++cntr1)
        priv->etype_udefs[cntr1] = RDPA_FILTER_ETYPE_DUMMY;

    /* 'oui_val' */
    for (cntr1 = 0; cntr1 < rdpa_if__number_of; ++cntr1)
    {
        for (cntr2 = 0; cntr2 < RDPA_FILTER_OUI_VALS_QUANT; ++cntr2)
            priv->oui_vals[cntr1][cntr2] = RDPA_FILTER_OUI_DUMMY;
    }

    /* 'tpid_vals' */
    priv->tpid_vals.val_ds = RDPA_FILTER_TPID_DUMMY;
    priv->tpid_vals.val_us = RDPA_FILTER_TPID_DUMMY;

    /* Statistics */
    memset(priv->stats, 0, sizeof(priv->stats));

    return filter_pre_init_ex(mo);
}

static int filter_post_init(struct bdmf_object *mo)
{
    filter_object = mo;
    snprintf(mo->name, sizeof(mo->name), "filter");

    return filter_post_init_ex(mo);
}


static void filter_destroy(struct bdmf_object *mo)
{
    filter_destroy_ex(mo);
    filter_object = NULL;
}

static int filter_drv_init(struct bdmf_type *drv);
static void filter_drv_exit(struct bdmf_type *drv);

struct bdmf_type filter_drv =
{
    .name = "filter",
    .parent = "system",
    .description = "Ingress Filters",
    .drv_init = filter_drv_init,
    .drv_exit = filter_drv_exit,
    .pre_init = filter_pre_init,
    .post_init = filter_post_init,
    .destroy = filter_destroy,
    .extra_size = sizeof(filter_drv_priv_t),
    .aattr = filter_attrs,
    .max_objs = 1
};
DECLARE_BDMF_TYPE(rdpa_filter, filter_drv);

static int filter_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_filter_drv = rdpa_filter_drv;
    f_rdpa_filter_get = rdpa_filter_get;
#endif
    return 0;
}

static void filter_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_filter_drv = NULL;
    f_rdpa_filter_get = NULL;
#endif
}

int ingress_filter_ctrl_cfg_get_next(rdpa_filter_ctrl_t *ingress_filters, bdmf_index *index)
{
    rdpa_filter filter;

    if (!filter_object)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Filter object doesn't exist\n");

    if (*index == BDMF_INDEX_UNASSIGNED)
        filter = 0;
    else
        filter = (rdpa_filter)(*index) + 1;
    for (; filter < RDPA_FILTERS_QUANT; filter++)
    {
        if (ingress_filters[filter].enabled)
        {
            *index = filter;
            return 0;
        }
    }
    return BDMF_ERR_NO_MORE;
}

int ingress_filter_ctrl_cfg_read(rdpa_filter_ctrl_t *ingress_filters, bdmf_index index, void *val)
{
    if (!filter_object)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Filter object doesn't exist\n");

    if (!ingress_filters[index].enabled)
        return BDMF_ERR_NOENT;

    memcpy(val, &(ingress_filters[index]), sizeof(rdpa_filter_ctrl_t));
    return 0;
}

int ingress_filter_ctrl_cfg_validate(bdmf_index index, void *val)
{
    rdpa_filter_ctrl_t *ctrl = (rdpa_filter_ctrl_t *)val;

    if (!filter_object)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Filter object doesn't exist\n");

    if (!ctrl->enabled)
        return 0;

    if (ctrl->action != rdpa_forward_action_host && ctrl->action != rdpa_forward_action_drop)
        return BDMF_ERR_PARM;

    return 0;
}

int ingress_filter_entry_set(rdpa_filter filter, struct bdmf_object *owner_obj,
    rdpa_filter_ctrl_t *owner_ctrl_table, rdpa_filter_ctrl_t *ctrl, uint8_t *profile_id)
{
    filter_drv_priv_t *priv;

    if (!filter_object)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Filter object doesn't exist\n");

    priv = (filter_drv_priv_t *)bdmf_obj_data(filter_object);
    return ingress_filter_entry_set_ex(priv, filter, owner_obj, owner_ctrl_table, ctrl, profile_id);
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/

/** Get ip_class object by key
 * \return  Object handle or NULL if not found
 */
int rdpa_filter_get(bdmf_object_handle *_obj_)
{
    if (!filter_object || filter_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(filter_object);
    *_obj_ = filter_object;
    return 0;
}

