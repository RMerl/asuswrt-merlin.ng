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

#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdpa_egress_tm_inline.h"
#include "rdpa_rdd_inline.h"
#include "rdpa_pbit_to_queue_ex.h"

/***************************************************************************
 * pbit_to_queue object type
 **************************************************************************/
/* This table holds qos mapping of pbit to queue.
 * Each table should be linked to the ports on which this mapping is valid on
 */

static bdmf_object_handle pbit_to_queue_objects[RDPA_PBIT_TO_PRTY_MAX_TABLES];
static int pbit_to_queue_linked_port[rdpa_if__number_of];

/* this table is used to link both tcont and llid since they never exists together
 * the size of the table is limit by the max tcont which is larger than llid */
static int pbit_to_queue_linked_tcont_llid[RDPA_MAX_TCONT];

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int pbit_to_queue_pre_init(struct bdmf_object *mo)
{
    pbit_to_queue_drv_priv_t *tbl = (pbit_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    static int init_flag;
    int i;

    /* Set defaults */
    for (i = 0; i < sizeof(tbl->pbit)/sizeof(tbl->pbit[0]); i++)
        tbl->pbit[i] = BDMF_INDEX_UNASSIGNED;
    tbl->index = BDMF_INDEX_UNASSIGNED;

    if (!init_flag)
    {
        for (i = 0; i < rdpa_if__number_of; i++)
        {
            pbit_to_queue_linked_port[i] = BDMF_INDEX_UNASSIGNED;
        }
        for (i = 0; i < RDPA_MAX_TCONT; i++)
        {
            pbit_to_queue_linked_tcont_llid[i] = BDMF_INDEX_UNASSIGNED;
        }
        init_flag = 1;
    }
    /* fetch the wan type from the system object */

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
static int pbit_to_queue_post_init(struct bdmf_object *mo)
{
    pbit_to_queue_drv_priv_t *tbl = (pbit_to_queue_drv_priv_t *)bdmf_obj_data(mo);

    /* if index is not set explicitly - assign free */
    if (tbl->index < 0)
    {
        int i;
        /* Find and assign free index */
        for (i = 0; i < RDPA_PBIT_TO_PRTY_MAX_TABLES; i++)
        {
            if (!pbit_to_queue_objects[i])
            {
                tbl->index = i;
                break;
            }
        }
    }
    if ((unsigned)tbl->index >= RDPA_PBIT_TO_PRTY_MAX_TABLES)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Table index %u is out of range\n", (unsigned)tbl->index);
    /* set object name */

    if (pbit_to_queue_objects[tbl->index])
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "%s index %d is already exists\n", mo->name, (int)tbl->index);

    snprintf(mo->name, sizeof(mo->name), "pbit_to_queue/table=%ld", tbl->index);

    pbit_to_queue_objects[tbl->index] = mo;

    return 0;
}

static void pbit_to_queue_destroy(struct bdmf_object *mo)
{
    pbit_to_queue_drv_priv_t *tbl = (pbit_to_queue_drv_priv_t *)bdmf_obj_data(mo);

    if (tbl->index >= RDPA_PBIT_TO_PRTY_MAX_TABLES || pbit_to_queue_objects[tbl->index] != mo)
        return;

    pbit_to_queue_objects[tbl->index] = NULL;
}

/** "pbit_to_queue" attribute's "write" callback */
static int pbit_to_queue_attr_queue_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    pbit_to_queue_drv_priv_t *tbl = (pbit_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle obj = NULL;
    rdpa_if port;
    int rc = 0;
    uint32_t queue_id = *(uint32_t *)val;

    for (port = rdpa_if_wan0; port < rdpa_if__number_of; port++)
    {
        if (pbit_to_queue_linked_port[port] == tbl->index)
        {
            rc = rdpa_port_get(port, &obj);
            if (rc < 0)
            {
                BDMF_TRACE_ERR("failed to get rdpa port object with index of %d", port);
                goto exit;
            }

            rc = rc ? rc : pbit_to_queue_set_single_entry_ex(mo, 1, obj, index, queue_id, 1);
            if (rc < 0)
            {
                BDMF_TRACE_ERR("Can't link pbit %d queue %d to port %s , rdd_error = %d\n",
                    (int)index, (int)(queue_id), bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), rc);
                goto exit;
            }
            
            bdmf_put(obj);
            obj = NULL;
        }
    }
    for (port = 0; port < RDPA_MAX_TCONT; port++)
    {
        if (pbit_to_queue_linked_tcont_llid[port] == tbl->index)
        {
            if (rdpa_is_gpon_or_xgpon_mode())
                rc = rdpa_tcont_get(port, &obj);
            else
                rc = rdpa_llid_get(port, &obj);

            if (rc < 0)
            {
                BDMF_TRACE_ERR("failed to get tcont/llid object with index of %d", port);
                goto exit;
            }

            rc = pbit_to_queue_set_single_entry_ex(mo, 1, obj, index, queue_id, 1);
            if (rc < 0)
            {
                BDMF_TRACE_ERR("Can't link pbit %d queue %d to port %s, rdd_error = %d\n",
                    (int)index, (int)(queue_id), bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), rc);
                goto exit;
            }
            
            bdmf_put(obj);
            obj = NULL;
        }
    }
    tbl->pbit[index] = queue_id;

exit:
    if (obj)
        bdmf_put(obj);

    return rc;
}

/* "pbit_to_queue" attribute "read" callback */
static int pbit_to_queue_attr_queue_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    pbit_to_queue_drv_priv_t *tbl = (pbit_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t *queue_id = (uint32_t *)val;

    if (tbl->pbit[index] == BDMF_INDEX_UNASSIGNED)
        return BDMF_ERR_NOENT;

    *queue_id = tbl->pbit[index];
    return 0;
}

/** Called when port is linked with port or pbit_to_queue */
static int pbit_to_queue_link_port(struct bdmf_object *mo, struct bdmf_object *other,
    const char *link_attrs)
{
    pbit_to_queue_drv_priv_t *pbit_to_queue = (pbit_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_number port = -1;
    int rc = 0, i;

    if (other->drv != rdpa_port_drv() && other->drv != rdpa_tcont_drv() && other->drv != rdpa_llid_drv())
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "Can't link with %s, linked object must be port or tcont/llid\n", other->name);
    }

    /* case of link with port object */
    if (other->drv == rdpa_port_drv())
    {
        bdmf_attr_get_as_num(other, rdpa_port_attr_index, &port);

        if (pbit_to_queue_linked_port[port] != BDMF_INDEX_UNASSIGNED)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo,
                "Port: %s is already linked to table index %d\n", other->name, (int)pbit_to_queue->index);
        }
    }
    else /* tcont or llid */
    {
        bdmf_attr_get_as_num(other, rdpa_is_gpon_or_xgpon_mode() ? rdpa_tcont_attr_index : rdpa_llid_attr_index, &port);

        if (pbit_to_queue_linked_tcont_llid[port] != BDMF_INDEX_UNASSIGNED)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo,
                "Tcont/Llid: %s is already linked to table index %d\n", other->name, (int)pbit_to_queue->index);
        }
    }
    /* Configure PBIT to priority in RDD */
    /* first loop just check if all queues are configured on the port */
    for (i = 0; i < 8; i++)
    {
        if (pbit_to_queue->pbit[i] != BDMF_INDEX_UNASSIGNED)
            rc = pbit_to_queue_set_single_entry_ex(mo, 0, other, i, pbit_to_queue->pbit[i], 1);

        if (rc)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "Can't link pbit %d queue %d to port %s\n",
                i, (int)(pbit_to_queue->pbit[i]), bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port));
        }
    }

    /* second loop actually configures to rdd */
    for (i = 0; i < 8; i++)
    {
        bdmf_index queue_idx = pbit_to_queue->pbit[i] != BDMF_INDEX_UNASSIGNED ? pbit_to_queue->pbit[i] : 0;

        if (pbit_to_queue->pbit[i] != BDMF_INDEX_UNASSIGNED)
            rc = pbit_to_queue_set_single_entry_ex(mo, 1, other, i, queue_idx, 1);

        if (rc)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo,
                "Can't link pbit %d queue %d to port %s , rdd_error = %d\n", i,
                (int)(pbit_to_queue->pbit[i]), bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port), rc);
        }
    }
    if (other->drv == rdpa_port_drv())
        pbit_to_queue_linked_port[port] = pbit_to_queue->index;
    else
        pbit_to_queue_linked_tcont_llid[port] = pbit_to_queue->index;

    return 0;
}

/** Called when port is unlinked from pbit_to_queue */
static void pbit_to_queue_unlink_port(struct bdmf_object *mo, struct bdmf_object *other)
{
    bdmf_number port = -1;
    int rc = 0, i;

    if (other->drv != rdpa_port_drv() && other->drv != rdpa_tcont_drv() && other->drv != rdpa_llid_drv())
    {
        BDMF_TRACE_ERR_OBJ(mo, "Can't unlink with %s, unlinked object must be port or tcont/llid\n",
            other->name);
    }

    for (i = 0; i < 8; i++)
    {
        rc = pbit_to_queue_set_single_entry_ex(mo, 1, other, i, 0, 0);
        if (rc)
            return;
    }

    if (other->drv == rdpa_port_drv())
    {
        bdmf_attr_get_as_num(other, rdpa_port_attr_index, &port);
        pbit_to_queue_linked_port[port] = BDMF_INDEX_UNASSIGNED;
    }
    else
    {
        bdmf_attr_get_as_num(other, rdpa_tcont_attr_index, &port);
        pbit_to_queue_linked_tcont_llid[port] = BDMF_INDEX_UNASSIGNED;
    }
    rdpa_pbit_to_queue_unlink_other_ex(mo, other);
}

/* Object attribute descriptors */
static struct bdmf_attr pbit_to_queue_attrs[] = {
    { .name = "table", .help = "Table index", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index), .offset = offsetof(pbit_to_queue_drv_priv_t, index)
    },
    { .name = "pbit_map", .help = "Priority array", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .size = sizeof(uint32_t), .offset = offsetof(pbit_to_queue_drv_priv_t, pbit),
        .min_val = 0, .array_size = 8,
        .write = pbit_to_queue_attr_queue_write, .read = pbit_to_queue_attr_queue_read
    },

    BDMF_ATTR_LAST
};

static int pbit_to_queue_drv_init(struct bdmf_type *drv);
static void pbit_to_queue_drv_exit(struct bdmf_type *drv);

struct bdmf_type pbit_to_queue_drv = {
    .name = "pbit_to_queue",
    .parent = "system",
    .description = "PBIT to priority mapping table",
    .drv_init = pbit_to_queue_drv_init,
    .drv_exit = pbit_to_queue_drv_exit,
    .pre_init = pbit_to_queue_pre_init,
    .post_init = pbit_to_queue_post_init,
    .destroy = pbit_to_queue_destroy,
    .link_down = pbit_to_queue_link_port,
    .unlink_down = pbit_to_queue_unlink_port,
    .link_up = pbit_to_queue_link_port,
    .unlink_up = pbit_to_queue_unlink_port,
    .extra_size = sizeof(pbit_to_queue_drv_priv_t),
    .aattr = pbit_to_queue_attrs,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN,
    .max_objs = RDPA_PBIT_TO_PRTY_MAX_TABLES,
};
DECLARE_BDMF_TYPE(rdpa_pbit_to_queue, pbit_to_queue_drv);

/* Init/exit module. Cater for GPL layer */
static int pbit_to_queue_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_pbit_to_queue_drv = rdpa_pbit_to_queue_drv;
    f_rdpa_pbit_to_queue_get = rdpa_pbit_to_queue_get;
#endif
    return 0;
}

static void pbit_to_queue_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_pbit_to_queue_drv = NULL;
    f_rdpa_pbit_to_queue_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated headers
 **************************************************************************/

/** Get pbit_to_queu object by key */
int rdpa_pbit_to_queue_get(bdmf_number _table_, bdmf_object_handle *_obj_)
{
    if (!pbit_to_queue_objects[_table_] || pbit_to_queue_objects[_table_]->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;

    bdmf_get(pbit_to_queue_objects[_table_]);

    *_obj_ =  pbit_to_queue_objects[_table_];
    return 0;
}


