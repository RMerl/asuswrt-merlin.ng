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
* :>
*/

#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_dscp_to_pbit_ex.h"

/***************************************************************************
 * dscp_to_pbit object type
 **************************************************************************/
 /* This table holds mapping of dscp to pbit.
  * One qos_mapping = yes table exist and used for qos mapping of untagged packet.
  * Other table are used for vlan action and should be linked to the ports
  * on which this mapping is valid on
  */

/* dscp_to_pbit object private data */
typedef struct {
    bdmf_index index;
    bdmf_boolean qos_mapping;  /**< Yes: qos mapping table, no: vlan action per port table */
    rdpa_pbit dscp[64];
    rdpa_pbit_dei_t dscp_pbit_dei[64];
} dscp_to_pbit_drv_priv_t;

static struct bdmf_object *dscp_to_pbit_objects[RDPA_DSCP_TO_PBIT_MAX_TABLES];
static int dscp_pbit_linked_port[rdpa_if__number_of];

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int dscp_to_pbit_pre_init(struct bdmf_object *mo)
{
    dscp_to_pbit_drv_priv_t *tbl = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(mo);
    int i;
    static int init_flag;

    /* Set defaults */
    for (i = 0; i < sizeof(tbl->dscp)/sizeof(tbl->dscp[0]); i++)
        tbl->dscp[i] = 0;

    for (i = 0; i < sizeof(tbl->dscp_pbit_dei)/sizeof(tbl->dscp_pbit_dei[0]); i++)
    {
        tbl->dscp_pbit_dei[i].pbit = 0;
        tbl->dscp_pbit_dei[i].dei = 0;
    }

    tbl->index = BDMF_INDEX_UNASSIGNED;
#if defined(BCM_DSL_RDP)
    /* DSL RDP uses QoS Mapping table method */
    tbl->qos_mapping = 1;
#else
    tbl->qos_mapping = 0;
#endif

    if (!init_flag)
    {
        for (i = 0; i < rdpa_if__number_of; i++)
        {
            dscp_pbit_linked_port[i] = BDMF_INDEX_UNASSIGNED;
            init_flag = 1;
        }
    }

    return 0;
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
static int dscp_to_pbit_post_init(struct bdmf_object *mo)
{
    dscp_to_pbit_drv_priv_t *tbl = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    /* if index is not set explicitly - assign free */
    if (tbl->index < 0)
    {
        /* Find and assign free index */
        for (i = 0; i < RDPA_DSCP_TO_PBIT_MAX_TABLES; i++)
        {
            if (!dscp_to_pbit_objects[i])
            {
                tbl->index = i;
                break;
            }
        }
    }
    if ((unsigned)tbl->index >= RDPA_DSCP_TO_PBIT_MAX_TABLES)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Table index %u is out of range\n", (unsigned)tbl->index);

    if (dscp_to_pbit_objects[tbl->index])
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "%s is already exists\n", mo->name);

    if (tbl->qos_mapping)
    {
        for (i = 0; i < RDPA_DSCP_TO_PBIT_MAX_TABLES; i++)
        {
            if (dscp_to_pbit_objects[i])
            {
                dscp_to_pbit_drv_priv_t *data = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(dscp_to_pbit_objects[i]);
                if (data->qos_mapping && i != tbl->index)
                {
                    BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                        "global dscp to pbit table is already configured in table %d\n", (int)data->index);
                }
            }
        }
    }

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "dscp_to_pbit/table = %ld", tbl->index);

    dscp_to_pbit_objects[tbl->index] = mo;
    return 0;
}

static void dscp_to_pbit_destroy(struct bdmf_object *mo)
{
    dscp_to_pbit_drv_priv_t *tbl = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    if (tbl->index >= RDPA_DSCP_TO_PBIT_MAX_TABLES || dscp_to_pbit_objects[tbl->index] != mo)
        return;

    if (tbl->qos_mapping)
    {
        for (i = 0; i < 64; i++)
        {
            rdpa_rdd_qos_dscp_pbit_dei_mapping_set(mo, i, 0, 0);
        }
    }

    dscp_to_pbit_objects[tbl->index] = NULL;
}

/*
 * dscp_to_pbit attribute access
 */
/** "dscp_to_pbit" attribute's "write" callback */
static int dscp_to_pbit_attr_pbit_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    dscp_to_pbit_drv_priv_t *tbl = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_pbit pbit = *(rdpa_pbit *)val;
    int rc, i;

    /* Make sure that pbit is sane */
    if (pbit > 7)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "pbit %d is out of range\n", pbit);

    if (tbl->qos_mapping)
    {
        /* in case this is a new object don't set configuration if another global table already exist */
        if (mo->state != bdmf_state_active)
        {
            for (i = 0; i < RDPA_DSCP_TO_PBIT_MAX_TABLES; i++)
            {
                if (dscp_to_pbit_objects[i])
                {
                    dscp_to_pbit_drv_priv_t *data = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(dscp_to_pbit_objects[i]);
                    if (data->qos_mapping && i != tbl->index)
                    {
                        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                            "global dscp to pbit table is already configured in table %d\n", (int)data->index);
                    }
                }
            }
        }
        rc = rdpa_rdd_qos_dscp_pbit_mapping_set(mo, index, pbit);
        if (rc)
            return rc;
    }
    else
    {
        rc = rdpa_rdd_vlan_dscp_pbit_mapping_set(mo, tbl->index, index, pbit);
        if (rc)
            return rc;
    }

    tbl->dscp[index] = pbit;

    return 0;
}

/*
 * dscp_to_pbit_dei attribute access
 */
/** "dscp_to_pbit_dei" attribute's "write" callback */
static int dscp_to_pbit_dei_attr_pbit_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    dscp_to_pbit_drv_priv_t *tbl = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_pbit_dei_t *pbit_dei = (rdpa_pbit_dei_t *)val;
    int rc, i;

    /* Make sure that pbit is sane */
    if (pbit_dei->pbit > RDPA_MAX_PBIT)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "pbit %d is out of range\n", pbit_dei->pbit);

    if (pbit_dei->dei > RDPA_MAX_DEI)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "dei %d is out of range\n", pbit_dei->dei);

    if (tbl->qos_mapping)
    {
        /* in case this is a new object don't set configuration if another global table already exist */
        if (mo->state != bdmf_state_active)
        {
            for (i = 0; i < RDPA_DSCP_TO_PBIT_MAX_TABLES; i++)
            {
                if (dscp_to_pbit_objects[i])
                {
                    dscp_to_pbit_drv_priv_t *data = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(dscp_to_pbit_objects[i]);
                    if (data->qos_mapping && i != tbl->index)
                    {
                        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                            "global dscp to pbit table is already configured in table %d\n", (int)data->index);
                    }
                }
            }
        }
        rc = rdpa_rdd_qos_dscp_pbit_dei_mapping_set(mo, index, pbit_dei->pbit, pbit_dei->dei);
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(rc, mo, "Can't configure dscp %d pbit %d dei %d\n",
                (int)index, (int)pbit_dei->pbit, (int)pbit_dei->dei);
        }
    }

    tbl->dscp[index] = pbit_dei->pbit & 0x7;
    tbl->dscp_pbit_dei[index].pbit = pbit_dei->pbit;
    tbl->dscp_pbit_dei[index].dei = pbit_dei->dei;

    return 0;
}

/** Called when dscp_to_pbit is linked with port */
static int dscp_to_pbit_link_port(struct bdmf_object *mo, struct bdmf_object *other,
    const char *link_attrs)
{
    dscp_to_pbit_drv_priv_t *dscp_to_pbit = (dscp_to_pbit_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_if port;
    int rc;

    if (other->drv != rdpa_port_drv())
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Can't link with %s, linked object must be port\n", other->name);

    if (dscp_to_pbit->qos_mapping)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "dscp to queue qos mapping table can't be linked to port\n");

    rdpa_port_index_get(other, &port);

    if (dscp_pbit_linked_port[port] != BDMF_INDEX_UNASSIGNED)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "Port: %s is already linked to table %d\n",
            other->name, dscp_pbit_linked_port[port]);
    }

    /* Configure DSCP to PBIT in RDD */
    rc = rdpa_rdd_port_to_dscp_to_pbit_table_set(mo, port, dscp_to_pbit->index);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(rc, mo,
            "Can't set link port %s with dscp_to_pbit table %d\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), (int)dscp_to_pbit->index);
    }

    dscp_pbit_linked_port[port] = dscp_to_pbit->index;

    return 0;
}

/** Called when dscp_to_pbit is unlinked from port */
static void dscp_to_pbit_unlink_port(struct bdmf_object *mo, struct bdmf_object *other)
{
    rdpa_if port;
    int rc = 0;

    rdpa_port_index_get(other, &port);

    rc = rdpa_rdd_port_to_dscp_to_pbit_table_set(mo, port, RDPA_RDD_DSCP_MAPPING_TABLE_UNDEFINED);
    if (rc)
    {
        BDMF_TRACE_ERR_OBJ(mo, "Can't unlink from port %s\n",
            bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port));
    }

    dscp_pbit_linked_port[port] = BDMF_INDEX_UNASSIGNED;
}


/* pbit_dei aggregate type */
struct bdmf_aggr_type pbit_dei_type =
{
    .name = "pbit_dei", .struct_name = "rdpa_pbit_dei_t",
    .help = "pbit dei",
    .fields = (struct bdmf_attr[])
    {
        {
            .name = "pbit", .help = "pbit: 0-7 priority bits", .type = bdmf_attr_number,
            .size = sizeof(uint8_t),
            .offset = offsetof(rdpa_pbit_dei_t, pbit)
        },
        {
            .name = "dei", .help = "dei: 0/1 drop eligibility value", .type = bdmf_attr_number,
            .size = sizeof(uint8_t),
            .offset = offsetof(rdpa_pbit_dei_t, dei)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(pbit_dei_type);


/* Object attribute descriptors */
static struct bdmf_attr dscp_to_pbit_attrs[] = {
    { .name = "table", .help = "Table index", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index), .offset = offsetof(dscp_to_pbit_drv_priv_t, index)
    },
    { .name = "qos_mapping", .help = "Yes : qos mapping table, no : vlan action per port table",
        .type = bdmf_attr_boolean,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .size = sizeof(bdmf_boolean), .offset = offsetof(dscp_to_pbit_drv_priv_t, qos_mapping)
    },
    { .name = "dscp_map", .help = "DSCP PBIT array", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG, .data_type_name = "rdpa_pbit",
        .size = sizeof(rdpa_pbit), .offset = offsetof(dscp_to_pbit_drv_priv_t, dscp),
        .min_val = 0, .max_val = 63,
        .array_size = 64, .write = dscp_to_pbit_attr_pbit_write
    },
    { .name = "dscp_pbit_dei_map", .help = "DSCP PBIT/DEI array", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG, .data_type_name = "rdpa_pbit_dei_t",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "pbit_dei",
        .size = sizeof(rdpa_pbit_dei_t), .offset = offsetof(dscp_to_pbit_drv_priv_t, dscp_pbit_dei),
        .array_size = 64, .write = dscp_to_pbit_dei_attr_pbit_write
    },
    BDMF_ATTR_LAST
};

static int dscp_to_pbit_drv_init(struct bdmf_type *drv);
static void dscp_to_pbit_drv_exit(struct bdmf_type *drv);

struct bdmf_type dscp_to_pbit_drv = {
    .name = "dscp_to_pbit",
    .parent = "system",
    .description = "DSCP to PBIT mapping table",
    .drv_init = dscp_to_pbit_drv_init,
    .drv_exit = dscp_to_pbit_drv_exit,
    .pre_init = dscp_to_pbit_pre_init,
    .post_init = dscp_to_pbit_post_init,
    .destroy = dscp_to_pbit_destroy,
    .link_down = dscp_to_pbit_link_port,
    .unlink_down = dscp_to_pbit_unlink_port,
    .link_up = dscp_to_pbit_link_port,
    .unlink_up = dscp_to_pbit_unlink_port,
    .extra_size = sizeof(dscp_to_pbit_drv_priv_t),
    .aattr = dscp_to_pbit_attrs,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN,
    .max_objs = RDPA_DSCP_TO_PBIT_MAX_TABLES,
};
DECLARE_BDMF_TYPE(rdpa_dscp_to_pbit, dscp_to_pbit_drv);

/* Init/exit module. Cater for GPL layer */
static int dscp_to_pbit_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_dscp_to_pbit_drv = rdpa_dscp_to_pbit_drv;
    f_rdpa_dscp_to_pbit_get = rdpa_dscp_to_pbit_get;
#endif
    return rdpa_rdd_dscp_to_pbit_init();
}

static void dscp_to_pbit_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_dscp_to_pbit_drv = NULL;
    f_rdpa_dscp_to_pbit_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated headers
 **************************************************************************/

/** Get dscp_to_pbit object by key */
int rdpa_dscp_to_pbit_get(bdmf_number _table_, bdmf_object_handle *_obj_)
{
    return rdpa_obj_get(dscp_to_pbit_objects, RDPA_DSCP_TO_PBIT_MAX_TABLES, _table_, _obj_);
}

