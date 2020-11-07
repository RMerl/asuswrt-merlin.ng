/*
 * <:copyright-BRCM:2015:proprietary:standard
 * 
 *    Copyright (c) 2015 Broadcom 
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
#include "rdpa_tc_to_queue_ex.h"
#include "rdd.h"

#ifdef XRDP
#include "rdd_qos_mapper.h"
#endif

/***************************************************************************
 * tc_to_queue object type
 **************************************************************************/
/* This table holds qos mapping of traffic class to queue.
 * Each table should be linked to the ports on which this mapping is valid on
 */

static int tc_to_queue_linked_port[rdpa_if__number_of];
static int tc_to_queue_linked_tcont_llid[RDPA_MAX_TCONT];

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int tc_to_queue_pre_init(struct bdmf_object *mo)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    static int init_flag;
    int i;

    /* Set defaults */
    tbl->index = BDMF_INDEX_UNASSIGNED;
    tbl->entries = 0;
    DLIST_INIT(&tbl->mapping_list);

    if (!init_flag)
    {
        /* init the linked port list */
        for (i = 0; i < rdpa_if__number_of; i++)
        {
            tc_to_queue_linked_port[i] = BDMF_INDEX_UNASSIGNED;
        }

        /*init the linked tcont list*/
        for (i = 0; i < RDPA_MAX_TCONT; i++)
        {
            tc_to_queue_linked_tcont_llid[i] = BDMF_INDEX_UNASSIGNED;
        }
        init_flag = 1;
    }

    return 0;
}

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalize object creation
 * If function returns error code !=0, object creation is aborted
 */
static int tc_to_queue_post_init(struct bdmf_object *mo)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    int i, max_tables;
    bdmf_object_handle *container = 0;

    tbl->size = 0;
    rdpa_tc_to_queue_obj_init_ex(&container, &max_tables, &(tbl->size), tbl->dir);
    /* if index is not set explicitly - assign free */
    if (tbl->index < 0)
    {
        /* Find and assign free index */
        for (i = 0; i < max_tables; i++)
        {
            if (!container[i])
            {
                tbl->index = i;
                break;
            }
        }
    }
    if ((unsigned)tbl->index >= max_tables)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Index %u is out of range for %s table\n",
            (unsigned)tbl->index, (tbl->dir == rdpa_dir_ds) ? "ds" : "us");
    }
    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "tc_to_queue/dir=%s,table=%ld",
        (tbl->dir == rdpa_dir_ds) ? "ds" : "us", tbl->index);

    if (container[tbl->index])
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "%s is index %d already exists\n", mo->name, (int)tbl->index);
    container[tbl->index] = mo;
    return 0;
}

static void tc_to_queue_destroy(struct bdmf_object *mo)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    tc_to_queue_t *tmp_entry, *entry = NULL;

    int max_tables;
    bdmf_object_handle *container = 0;
    rdpa_tc_to_queue_obj_init_ex(&container, &max_tables, &(tbl->size), tbl->dir);

    if (tbl->index >= max_tables || container[tbl->index] != mo || tbl->index < 0)
    {
        BDMF_TRACE_ERR_OBJ(mo, "Index is out of range %d for %s table\n",
            (int)tbl->index, (tbl->dir == rdpa_dir_ds) ? "ds" : "us");
        return;
    }

    DLIST_FOREACH_SAFE(entry, &tbl->mapping_list, list, tmp_entry)
    {
        /* removing of the configuration from rdd is done by unlink function */
        DLIST_REMOVE(entry, list);
        bdmf_free(entry);
    }
    BUG_ON(!DLIST_EMPTY(&tbl->mapping_list));
    container[tbl->index] = NULL;
}

/*************************************************************************************************
 * _find_and_update_linked_to_tc_to_queue_objects - check if tc_to_queue is linked to port or llid and update
 *    changed values on this port/llid.
 *    Input Params:
 *         mo -  pointer to tc_to_queue table
 *         index - what element in tc_map to change
 *         queue new tc_map value
 *         tc_to_queue_linked_obects table for each object to what tc_to_queue table is connected (-1 not assignment)
 *         first - from this object id start to search
 *         last  - search up to (from first to last )
 *         if_port - when 1 object is port, 0 llid or tcont
 *   Output:
 *     rc - 0 no error
 *          other; error code
 *          (if no object was linked to tc_to_queue no error will be returned:
 *           inner tc_map value will be updated ok).
 ************************************************************************************************/
static int _find_and_update_linked_to_tc_to_queue_objects(struct bdmf_object *mo, bdmf_index index, uint32_t queue,
    int first, int last, int *tc_to_queue_linked_obects, bdmf_boolean if_port)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle obj = NULL;
    int inst = 0;
    int rc = BDMF_ERR_OK;

    for (inst = first; inst <= last; inst++)
    {
        if (tc_to_queue_linked_obects[inst] == tbl->index)
        {
            if (if_port)
                rc = rdpa_port_get(inst, &obj);
            else
            {
                rc = rdpa_tcont_get(inst, &obj);
            	if (rc)
            	    rc = rdpa_llid_get(inst, &obj);
            }
            if (rc)
            {
                BDMF_TRACE_ERR("failed to get port/llid object with index of %d, rc %d\n", (int)inst, rc);
                goto exit;
            }

            rc = rdpa_tc_to_queue_set_single_entry_ex(mo, 1, obj, index, queue, 1);
            if (rc)
            {
                BDMF_TRACE_ERR("tc_to_queue_set_single_entry rc %d\n", rc);
                goto exit;
            }
            bdmf_put(obj);
            obj = NULL;
        }
    }
exit:
    if (obj)
        bdmf_put(obj);

    return rc;
}

/*
 * tc_to_queue attribute access
 */

/** "tc_queue" attribute's "write" callback */
static int tc_to_queue_attr_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    tc_to_queue_t *tmp_entry, *list_entry = NULL, *entry = NULL;
    bdmf_boolean modify_flag = 0;
    uint8_t table_size = tbl->size;
    uint32_t queue = *(uint32_t *)val;
    int rc = BDMF_ERR_OK;

    if (tbl->dir == rdpa_dir_ds && (index > 7 || index < 0))
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "DS table index %d out of range: should be between 0 to 7", (int)index);

    DLIST_FOREACH_SAFE(list_entry, &tbl->mapping_list, list, tmp_entry)
    {
        if (list_entry->tc == index)
        {
            entry = list_entry;
            modify_flag = 1;
            break;
        }
    }

    if (!modify_flag && table_size == tbl->entries)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "%s table index %d is full\n",
            (tbl->dir == rdpa_dir_ds) ? "ds" : "us", (int)tbl->index);
    }

    if (!modify_flag)
    {
        entry = bdmf_alloc(sizeof(tc_to_queue_t));
        if (!entry)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't allocate tc to queue entry");
    }

    /*check if any port is linked to the table and change configuration*/
    if (tbl->dir == rdpa_dir_ds)
    {
        rc = _find_and_update_linked_to_tc_to_queue_objects(mo, index, queue, rdpa_if_lan0, rdpa_if__number_of - 1,
            tc_to_queue_linked_port, 1);
    }
    else
    {
        /*check if Active ethernet or GBE modes*/
        if (rdpa_is_epon_ae_mode() || rdpa_is_gbe_mode())
        {
            rc = _find_and_update_linked_to_tc_to_queue_objects(mo, index, queue, rdpa_if_wan0, rdpa_if_wan2,
                tc_to_queue_linked_port, 1);
        }
        else
        {
            /* Check if any tcont is linked to the table and change configuration */
            rc = rdpa_tc_to_queue_realloc_table_ex(tc_to_queue_linked_tcont_llid, tbl);
            if (rc)
            {
                BDMF_TRACE_ERR("Can't reallocate %d sized tables for all tconts , failed on tcont %s",
                    (int)tbl->size, bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, 0));
                goto exit;
            }
            rc = _find_and_update_linked_to_tc_to_queue_objects(mo, index, queue, 0, RDPA_MAX_TCONT - 1,
            		tc_to_queue_linked_tcont_llid, 0);
        }
    }
    if (rc)
        goto exit;

    entry->queue = queue;
    entry->tc = index;

    if (!modify_flag)
    {
        DLIST_INSERT_HEAD(&tbl->mapping_list, entry, list);
        tbl->entries++;
    }

exit:
    if (rc && (!modify_flag) && entry)
        bdmf_free(entry);

    return rc;
}


/* "tc_to_queue" attribute "read" callback */
static int tc_to_queue_attr_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t  *queue = (uint32_t  *)val;
    tc_to_queue_t *tmp_entry, *entry = NULL;

    DLIST_FOREACH_SAFE(entry, &tbl->mapping_list, list, tmp_entry)
    {
        if (entry->tc == index)
        {
            *queue = entry->queue;
            return 0;
        }
    }
    BDMF_TRACE_RET_OBJ(BDMF_ERR_NOENT, mo, "Can't find tc %d in table dir %s index %d\n", (int)index,
        (tbl->dir == rdpa_dir_ds) ? "ds" : "us", (int)tbl->index);

    return 0;
}


static int tc_to_queue_attr_get_next(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index *index)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    tc_to_queue_t *tmp_entry, *entry = NULL;
    bdmf_boolean flag = 0;

    DLIST_FOREACH_SAFE(entry, &tbl->mapping_list, list, tmp_entry)
    {
        if (*index == BDMF_INDEX_UNASSIGNED)
        {
            *index = entry->tc;
            return 0;
        }
        if (flag)
        {
            *index = entry->tc;
            return 0;
        }
        if (entry->tc == *index)
            flag = 1;
    }
    return BDMF_ERR_NO_MORE;
}

/** Called when port is linked with tc_to_queue table*/
static int tc_to_queue_link_port(struct bdmf_object *mo, struct bdmf_object *other,
    const char *link_attrs)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    tc_to_queue_t *entry, *tmp_entry = NULL;
    bdmf_number other_index = -1;
    int rc;

    if (tbl->dir == rdpa_dir_us)
    {
        if (other->drv != rdpa_tcont_drv() && other->drv != rdpa_llid_drv() && other->drv != rdpa_port_drv())
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                "upstream table can be linked with tcont/llid/port object only\n");
        }
        if (other->drv == rdpa_port_drv())
        {
            bdmf_attr_get_as_num(other, rdpa_port_attr_index, &other_index);
            if ((rdpa_wan_if_to_wan_type(other_index) != rdpa_wan_gbe) && !rdpa_is_car_mode() && !rdpa_is_epon_ae_mode())
            {
                BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                    "upstream table can be linked to port object only in GBE or CAR mode or active ethernet\n");
            }
        }
    }
    else if (other->drv != rdpa_port_drv())
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "downstream table can be linked with port object only\n");

    if (other->drv == rdpa_port_drv())
    {
        bdmf_attr_get_as_num(other, rdpa_port_attr_index, &other_index);

        if (tc_to_queue_linked_port[other_index] != BDMF_INDEX_UNASSIGNED)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "Port: %s is already linked to table index %d\n",
                other->name, tc_to_queue_linked_port[other_index]);
        }
    }
    else if (other->drv == rdpa_llid_drv())
    {
        bdmf_attr_get_as_num(other, rdpa_llid_attr_index, &other_index);

        if (tc_to_queue_linked_tcont_llid[other_index] != BDMF_INDEX_UNASSIGNED)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "Llid: %s is already linked to table index %d\n",
                other->name, tc_to_queue_linked_tcont_llid[other_index]);
        }
    }
    else /* must be tcont */
    {
        bdmf_attr_get_as_num(other, rdpa_tcont_attr_index, &other_index);

        if (tc_to_queue_linked_tcont_llid[other_index] != BDMF_INDEX_UNASSIGNED)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "Tcont: %s is already linked to table index %d\n",
                other->name, tc_to_queue_linked_tcont_llid[other_index]);
        }
    }
    /* Configure TC to queue in RDD */
    /* first loop just check if all queues are configured on the port (set_to_rdd = 0)*/
    DLIST_FOREACH_SAFE(entry, &tbl->mapping_list, list, tmp_entry)
    {
        rc = rdpa_tc_to_queue_set_single_entry_ex(mo, 0, other, entry->tc, entry->queue, 1);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't link tc %d queue %d to port %s\n", (int)entry->tc,
                (int)(entry->queue), bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, other_index));
        }
    }

    /* second loop actually configures (set_to_rdd = 1)*/
    DLIST_FOREACH_SAFE(entry, &tbl->mapping_list, list, tmp_entry)
    {
        rc = rdpa_tc_to_queue_set_single_entry_ex(mo, 1, other, entry->tc, entry->queue, 1);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't link tc %d queue %d to port %s rdd_error = %d\n",
                (int)entry->tc, (int)(entry->queue),
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, other_index), rc);
        }
    }
    if (other->drv == rdpa_port_drv())
        tc_to_queue_linked_port[other_index] = tbl->index;
    else
        tc_to_queue_linked_tcont_llid[other_index] = tbl->index;
    return 0;
}

/** Called when port is unlinked from tc_to_queue table */
static void tc_to_queue_unlink_port(struct bdmf_object *mo, struct bdmf_object *other)
{
    tc_to_queue_drv_priv_t *tbl = (tc_to_queue_drv_priv_t *)bdmf_obj_data(mo);
    tc_to_queue_t *entry, *tmp_entry = NULL;
    bdmf_number port = -1;
    int rc = 0;

    DLIST_FOREACH_SAFE(entry, &tbl->mapping_list, list, tmp_entry)
    {
        rc = rdpa_tc_to_queue_set_single_entry_ex(mo, 1, other, entry->tc, 0, 0);
        if (rc)
            return;
    }

    if (other->drv == rdpa_port_drv())
    {
        bdmf_attr_get_as_num(other, rdpa_port_attr_index, &port);
        tc_to_queue_linked_port[port] = BDMF_INDEX_UNASSIGNED;
    }
    else if (other->drv == rdpa_llid_drv())
    {
        bdmf_attr_get_as_num(other, rdpa_llid_attr_index, &port);
        tc_to_queue_linked_tcont_llid[port] = BDMF_INDEX_UNASSIGNED;
    }
    else
    {
        bdmf_attr_get_as_num(other, rdpa_tcont_attr_index, &port);
        tc_to_queue_linked_tcont_llid[port] = BDMF_INDEX_UNASSIGNED;
    }
    rdpa_tc_to_queue_unlink_port_ex(mo, other, port);
}


/* Object attribute descriptors */
static struct bdmf_attr tc_to_queue_attrs[] = {
    { .name = "table", .help = "Table index", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index), .offset = offsetof(tc_to_queue_drv_priv_t, index)
    },
    { .name = "dir", .help = "Traffic Direction",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_dir_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY | BDMF_ATTR_MANDATORY,
        .size = sizeof(rdpa_traffic_dir), .offset = offsetof(tc_to_queue_drv_priv_t, dir)
    },
    { .name = "tc_map", .help = "TC to queue array", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_RANGE_CHECK,
        .size = sizeof(uint32_t),
        .index_type = bdmf_attr_number, .min_val = 0,
        .array_size = RDPA_CS_TC_TO_QUEUE_TABLE_SIZE, .write = tc_to_queue_attr_write, .read = tc_to_queue_attr_read,
        .get_next = tc_to_queue_attr_get_next
    },
    BDMF_ATTR_LAST
};

static int tc_to_queue_drv_init(struct bdmf_type *drv);
static void tc_to_queue_drv_exit(struct bdmf_type *drv);

struct bdmf_type tc_to_queue_drv = {
    .name = "tc_to_queue",
    .parent = "system",
    .description = "TC to queue mapping table",
    .drv_init = tc_to_queue_drv_init,
    .drv_exit = tc_to_queue_drv_exit,
    .pre_init = tc_to_queue_pre_init,
    .post_init = tc_to_queue_post_init,
    .destroy = tc_to_queue_destroy,
    .link_down = tc_to_queue_link_port,
    .unlink_down = tc_to_queue_unlink_port,
    .link_up = tc_to_queue_link_port,
    .unlink_up = tc_to_queue_unlink_port,
    .extra_size = sizeof(tc_to_queue_drv_priv_t),
    .aattr = tc_to_queue_attrs,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN,
#ifndef XRDP
    .max_objs = RDPA_DS_TC_TO_QUEUE_ID_MAX_TABLES + RDPA_US_TC_TO_QUEUE_ID_MAX_TABLES,
#else
    .max_objs = RDPA_TC_TO_QUEUE_ID_MAX_TABLES,
#endif
};
DECLARE_BDMF_TYPE(rdpa_tc_to_queue, tc_to_queue_drv);


/* Init/exit module. Cater for GPL layer */
static int tc_to_queue_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_tc_to_queue_drv = rdpa_tc_to_queue_drv;
    f_rdpa_tc_to_queue_get = rdpa_tc_to_queue_get;
#endif
    return 0;
}

static void tc_to_queue_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_tc_to_queue_drv = NULL;
    f_rdpa_tc_to_queue_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated headers
 **************************************************************************/

/** Get tc_to_queue object by key */
int rdpa_tc_to_queue_get(const rdpa_tc_to_queue_key_t *_key_, bdmf_object_handle *_obj_)
{
    bdmf_object_handle *container = 0;
    int max_tables;
    uint8_t table_size = RDPA_BS_TC_TO_QUEUE_TABLE_SIZE;

    rdpa_tc_to_queue_obj_init_ex(&container, &max_tables, &table_size, _key_->dir);

    if ((unsigned)_key_->table >= max_tables || _key_->table < 0)
    {
    	BDMF_TRACE_RET(BDMF_ERR_PARM, "Can't get tc to queue obj: index out of range %d\n",
            (int)_key_->table);
    }

    if (!container[_key_->table] || container[_key_->table]->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;

    bdmf_get(container[_key_->table]);

    *_obj_ = container[_key_->table];
    return 0;
}

