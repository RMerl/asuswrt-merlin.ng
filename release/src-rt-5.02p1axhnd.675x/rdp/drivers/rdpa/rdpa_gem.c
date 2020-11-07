/*
* <:copyright-BRCM:2014-2015:proprietary:standard
* 
*    Copyright (c) 2014-2015 Broadcom 
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

/*
 *    rdpa_gem.c
 *    Created on: June 18, 2014
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdpa_gem_ex.h"

#define RDPA_GEM_STAT_NUM_USERS 2
#define RDPA_PORT_ID_DONT_CARE 0xFFFF

struct bdmf_object *gem_objects[RDPA_MAX_GEM_FLOW];

/*
 *  Attribute Access Functions
 */

/* "us_cfg" attribute "write" callback. */
static int gem_attr_us_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    int rc = 0;
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_flow_us_cfg_t *cfg = (rdpa_gem_flow_us_cfg_t *)val;
    bdmf_number tcont_index;

    /* Remove GEM */
    if (!cfg || !cfg->tcont)
    {
        rc = _cfg_us_gem_flow_hw(0, priv->index, priv->us_cfg.tcont, priv->port,
           (priv->type == rdpa_gem_flow_ethernet ? 1 : 0), priv->us_cfg.enc);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "remove us gem flow %ld error\n", priv->index);
        priv->us_cfg.tcont = NULL;
    }
    else
    {
        rc = rdpa_tcont_index_get(cfg->tcont, &tcont_index);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "gem %ld: tcont index %ld doesn't exists\n", 
                (long int)priv->index, (long int)tcont_index);
            return BDMF_ERR_NOENT;
        }

        if (mo->state == bdmf_state_active)
        {
            if (gem_objects[priv->index] != mo)
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Received gem %ld: doesn't exist\n", priv->index);

            rc = _cfg_us_gem_flow_hw(1, priv->index, cfg->tcont, priv->port,
                priv->type == rdpa_gem_flow_ethernet ? 1 : 0, cfg->enc);
            if (rc < 0)
                return rc;
        }
        priv->us_cfg = *cfg;
    }

    return rc;
}

/* "stat" attribute "read" callback */
static int gem_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return gem_attr_stat_read_ex(mo, ad, index, val, size);
}

/* "ds_def_flow" attribute "read" callback */
static int gem_attr_ds_def_flow_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    return gem_attr_ds_def_flow_read_ex(mo, ad, index, val, size);
}

/* "ds_def_flow" attribute "write" callback. */
static int gem_attr_ds_def_flow_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    return gem_attr_ds_def_flow_write_ex(mo, ad, index, val, size);
}

/* "ds_cfg" attribute "write" callback. */
static int gem_attr_ds_cfg_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_flow_ds_cfg_t *cfg = (rdpa_gem_flow_ds_cfg_t *)val;
    int rc = 0;

   if (cfg == NULL)
    {
        /* Delete ds cfg */
        rc = _cfg_ds_gem_flow_hw(0, priv->index, priv->port, 0, 0, priv->ds_def_flow);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "remove ds gem flow %ld error\n", priv->index);
        }

        /* return to default value */
        priv->ds_cfg.destination = rdpa_flow_dest_eth;
        priv->ds_cfg.discard_prty = rdpa_discard_prty_low;
    }
    else
    {
        if (priv->index > RDPA_GEM_HIGH_PRIO_GEM_MAX && (cfg->destination != rdpa_flow_dest_eth ||
            cfg->discard_prty != rdpa_discard_prty_low))
        {
            BDMF_TRACE_RET(BDMF_ERR_PARM, "ds gem %ld must have eth destination and low priority", priv->index);
        }

        if (priv->ds_cfg.destination != rdpa_flow_dest_none)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't reconfigure ds gem");

        if (mo->state == bdmf_state_active)
        {
            if (gem_objects[priv->index] != mo)
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Received gem %ld: doesn't exist\n", priv->index);

            rc = _cfg_ds_gem_flow_hw(1, priv->index, priv->port, cfg->destination,
                cfg->discard_prty, priv->ds_def_flow);
            if (rc < 0)
                return rc;
        }
        priv->ds_cfg = *cfg;
    }

    return rc;
}

/* "port_action" attribute "read" callback */
static int gem_attr_port_action_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    void *val, uint32_t size)
{
    return gem_attr_port_action_read_ex(mo, ad, index, val, size);
}

/* "port_action" attribute write callback */
static int gem_attr_port_action_write(struct bdmf_object *mo,
        struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    return gem_attr_port_action_write_ex(mo, ad, index, val, size);
}

/*
 * enum tables
 */

/* gem_flow_type enum values */
static bdmf_attr_enum_table_t rdpa_gem_flow_type_enum_table = {
    .type_name = "rdpa_gem_flow_type", .help = "GEM flow type",
    .values = {
        {"ethernet", rdpa_gem_flow_ethernet},
        {"omci", rdpa_gem_flow_omci},
        {NULL, 0}
    }
};


/*
 * aggregate descriptors
 */

/*  gem_us_cfg aggregate type */
struct bdmf_aggr_type gem_us_cfg_type = {
    .name = "gem_us_cfg", .struct_name = "rdpa_gem_flow_us_cfg_t",
    .help = "GEM US Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "tcont", .help = "TCONT",
          .type = bdmf_attr_object, .ts.ref_type_name = "tcont",
          .size = sizeof(bdmf_object_handle), .offset = offsetof(rdpa_gem_flow_us_cfg_t, tcont),
        },
        { .name = "encrpt", .help = "XGPON encryption",
          .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
          .offset = offsetof(rdpa_gem_flow_us_cfg_t, enc)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(gem_us_cfg_type);

/*  ds_cfg aggregate type */
struct bdmf_aggr_type ds_cfg_type = {
    .name = "ds_cfg", .struct_name = "rdpa_gem_flow_ds_cfg_t",
    .help = "GEM DS Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "discard_prty", .help = "Discard priority", .size = sizeof(rdpa_discard_prty),
          .type = bdmf_attr_enum, .ts.enum_table = &rdpa_disc_prty_enum_table,
          .offset = offsetof(rdpa_gem_flow_ds_cfg_t, discard_prty)
        },
        { .name = "destination", .help = "Flow destination", .size = sizeof(rdpa_flow_destination),
          .type = bdmf_attr_enum, .ts.enum_table = &rdpa_flow_dest_enum_table,
          .offset = offsetof(rdpa_gem_flow_ds_cfg_t, destination)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(ds_cfg_type);

/*  gem_stat aggregate type */
struct bdmf_aggr_type gem_stat_type = {
    .name = "gem_stat", .struct_name = "rdpa_gem_stat_t",
    .help = "gem Statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "rx_packets", .help = "Rx Packets", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_gem_stat_t, rx_packets),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        { .name = "rx_bytes", .help = "Rx Bytes", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_gem_stat_t, rx_bytes),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        { .name = "tx_packets", .help = "Tx Packets", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_gem_stat_t, tx_packets),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        { .name = "tx_bytes", .help = "Tx Bytes", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_gem_stat_t, tx_bytes),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        { .name = "rx_packets_discard", .help = "Rx Packet discard", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_gem_stat_t, rx_packets_discard),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        { .name = "tx_packets_discard", .help = "Tx Packets discard", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_gem_stat_t, tx_packets_discard),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(gem_stat_type);

/* gem port_action aggregate type */
struct bdmf_aggr_type gem_port_action_type = {
    .name = "gem_port_action", .struct_name = "rdpa_gem_port_action_t",
    .help = "Gem per port action",
    .fields = (struct bdmf_attr[]) {
        { .name = "vlan_action", .help = "VLAN action object",
            .type = bdmf_attr_object, .size = sizeof(bdmf_object_handle), .ts.ref_type_name = "vlan_action",
            .offset = offsetof(rdpa_gem_port_action_t, vlan_action)
        },
        { .name = "drop", .help = "Drop action - true/false",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_gem_port_action_t, drop)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(gem_port_action_type);

/* Object attribute descriptors */
static struct bdmf_attr gem_attrs[] = {
    { .name = "index", .help = "GEM index", .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY,
        .type = bdmf_attr_number, .offset = offsetof(gem_drv_priv_t, index)
#ifdef XRDP
        , .min_val = 0, .max_val = 127
#endif
    },
    { .name = "gem_port", .help = "GEM port ID", .size = sizeof(uint16_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY | BDMF_ATTR_UNSIGNED,
        .type = bdmf_attr_number, .offset = offsetof(gem_drv_priv_t, port)
    },
    { .name = "flow_type", .help = "GEM flow type", .size = sizeof(rdpa_gem_flow_type),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_gem_flow_type_enum_table,
        .offset = offsetof(gem_drv_priv_t, type)
    },
    { .name = "ds_def_flow", .help = "Downstream Default flow configuration",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_NULLCHECK,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "classification_result",
        .write = gem_attr_ds_def_flow_write, .read = gem_attr_ds_def_flow_read
    },
    { .name = "port_action", .help = "Per port Vlan Action configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "gem_port_action",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_RANGE_CHECK,
        .index_ts.enum_table = &rdpa_if_enum_table, .array_size = rdpa_if__number_of,
        .index_type = bdmf_attr_enum,
        .read = gem_attr_port_action_read, .write = gem_attr_port_action_write
    },
    { .name = "us_cfg", .help = "Upstream GEM configuration",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_NULLCHECK,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "gem_us_cfg",
        .offset = offsetof(gem_drv_priv_t, us_cfg), .write = gem_attr_us_cfg_write
    },
    { .name = "ds_cfg", .help = "downstream GEM configuration",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_NULLCHECK,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "ds_cfg",
        .offset = offsetof(gem_drv_priv_t, ds_cfg), .write = gem_attr_ds_cfg_write
    },
    { .name = "stat", .help = "GEM statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "gem_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT,
        .read = gem_attr_stat_read, .write = gem_attr_stat_write_ex
    },
    BDMF_ATTR_LAST
};

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int gem_pre_init(struct bdmf_object *mo)
{
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);

    priv->index = BDMF_INDEX_UNASSIGNED;
    priv->port = (uint16_t)RDPA_VALUE_UNASSIGNED;
    priv->us_cfg.tcont = NULL;
    priv->ds_cfg.destination = rdpa_flow_dest_none;
    priv->ds_cfg.discard_prty = rdpa_discard_prty_low;
    priv->ds_def_flow = RDPA_UNMATCHED_DS_IC_RESULT_ID;

    return gem_pre_init_ex();
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
static int gem_post_init(struct bdmf_object *mo)
{
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_flow_ds_cfg_t *ds_cfg = &priv->ds_cfg;
    rdpa_gem_flow_us_cfg_t *us_cfg = &priv->us_cfg;
    int i, rc = 0;

    /* Assign gem if not assigned */
    /* find empty index */
    if (priv->index < 0)
    {
        for (i = 0; i < RDPA_MAX_GEM_FLOW; i++)
        {
            if (!gem_objects[i] && _ds_gem_flow_check(i))
            {
                priv->index = i;
                break;
            }
        }
    }
    else if (!_ds_gem_flow_check(priv->index))
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "index %ld is out of range\n", priv->index);
    }

    if ((unsigned)priv->index >= RDPA_MAX_GEM_FLOW)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many gems or index %ld is out of range\n", priv->index);
    if (gem_objects[priv->index])
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "gem %ld already exists\n", priv->index);

    /* cfg ds gem if is configured in initialization */
    if (ds_cfg->destination != rdpa_flow_dest_none)
    {
        rc = _cfg_ds_gem_flow_hw(1, priv->index, priv->port, ds_cfg->destination,
            ds_cfg->discard_prty, priv->ds_def_flow);
        if (rc < 0)
            BDMF_TRACE_RET(rc, "DS gem %ld configuration failed", priv->index);
    }
    if (priv->us_cfg.tcont != NULL)
    {
        rc = _cfg_us_gem_flow_hw(1, priv->index, us_cfg->tcont, priv->port,
            (priv->type == rdpa_gem_flow_ethernet ? 1 : 0), us_cfg->enc);
        if (rc < 0)
            BDMF_TRACE_RET(rc, "US gem %ld configuration failed", priv->index);
    }

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "gem/index=%ld", priv->index);
    gem_objects[priv->index] = mo;

    return 0;
}

static void gem_destroy(struct bdmf_object *mo)
{
    gem_drv_priv_t *priv = (gem_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_flow_ds_cfg_t *ds_cfg = &priv->ds_cfg;
    rdpa_gem_flow_us_cfg_t *us_cfg = &priv->us_cfg;

    if ((unsigned)priv->index >= RDPA_MAX_GEM_FLOW || gem_objects[priv->index] != mo)
        return;

    gem_objects[priv->index] = NULL;

    if (ds_cfg->destination != rdpa_flow_dest_none)
    {
        _cfg_ds_gem_flow_hw(0, priv->index, priv->port, ds_cfg->destination,
            ds_cfg->discard_prty, priv->ds_def_flow);
    }
    if (priv->us_cfg.tcont != NULL)
    {
        _cfg_us_gem_flow_hw(0, priv->index, us_cfg->tcont, priv->port,
            (priv->type == rdpa_gem_flow_ethernet ? 1 : 0), us_cfg->enc);
    }

    if (priv->ds_def_flow != RDPA_UNMATCHED_DS_IC_RESULT_ID)
        remove_def_flow(priv);
}

static int gem_drv_init(struct bdmf_type *drv);
static void gem_drv_exit(struct bdmf_type *drv);

struct bdmf_type gem_drv = {
    .name = "gem",
    .parent = "system",
    .description = "GEM",
    .drv_init = gem_drv_init,
    .drv_exit = gem_drv_exit,
    .pre_init = gem_pre_init,
    .post_init = gem_post_init,
    .destroy = gem_destroy,
    .extra_size = sizeof(gem_drv_priv_t),
    .aattr = gem_attrs,
    .max_objs = RDPA_MAX_GEM_FLOW,
};
DECLARE_BDMF_TYPE(rdpa_gem, gem_drv);

/* Init/exit module. Cater for GPL layer */
static int gem_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_gem_drv = rdpa_gem_drv;
    f_rdpa_gem_get = rdpa_gem_get;
#endif
    return 0;
}

static void gem_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_gem_drv = NULL;
    f_rdpa_gem_get = NULL;
#endif
}

/*
 * Functions declared in auto-generated header
 */

int rdpa_gem_get(bdmf_number index, bdmf_object_handle *gem_obj)
{
    if (!gem_objects[index] || gem_objects[index]->state == bdmf_state_deleted)
        return BDMF_ERR_NODEV;
    bdmf_get(gem_objects[index]);
    *gem_obj = gem_objects[index];

    return 0;
}

/*
 * Extern configuration helpers
 */

/* The function returns TCONT ID associated with given GEM flow.
   The function can be called in interrupt context.
   Returns 0 if OK or error < 0  */
int rdpa_gem_flow_id_to_tcont_id(int gem_flow, int *tcont_id)
{
    struct bdmf_object *gem_obj;
    gem_drv_priv_t *gem_priv;
    bdmf_number index;

    if ((unsigned)gem_flow >= RDPA_MAX_GEM_FLOW)
        return BDMF_ERR_PARM;

    gem_obj = gem_objects[gem_flow];
    if (!gem_obj)
        return BDMF_ERR_NOENT;

    gem_priv = (gem_drv_priv_t *)bdmf_obj_data(gem_obj);
    if (!gem_priv->us_cfg.tcont)
        return BDMF_ERR_NOENT;

    rdpa_tcont_index_get(gem_priv->us_cfg.tcont, &index);
    *tcont_id = (int)index;

    return 0;
}

/* The function returns the Channel ID of the TCONT associated with given GEM flow.
   The function can be called in interrupt context.
   Returns 0 if OK or error < 0  */
int rdpa_gem_flow_id_to_tcont_channel_id(int gem_flow, int *channel_id)
{
    struct bdmf_object *gem_obj;
    gem_drv_priv_t *gem_priv;
    bdmf_number channel_index;
    int rc;

    if ((unsigned)gem_flow >= RDPA_MAX_GEM_FLOW)
        return BDMF_ERR_PARM;

    gem_obj = gem_objects[gem_flow];
    if (!gem_obj)
        return BDMF_ERR_NOENT;

    gem_priv = (gem_drv_priv_t *)bdmf_obj_data(gem_obj);
    if (!gem_priv->us_cfg.tcont)
        return BDMF_ERR_NOENT;

    rc = rdpa_tcont_channel_get(gem_priv->us_cfg.tcont, &channel_index);
    if (rc)
        return rc;

    *channel_id = (int)channel_index;

    return 0;
}
