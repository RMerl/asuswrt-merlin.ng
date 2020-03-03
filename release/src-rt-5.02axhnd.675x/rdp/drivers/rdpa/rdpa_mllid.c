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

/*
 * rdpa_mllid.c
 *
 * EPON MLLID driver
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdpa_gem_ex.h"
#include "rdpa_mllid.h"
#include "rdpa_mllid_ex.h"


/* MLLID objects */
bdmf_object_handle mllid_objects[RDPA_EPON_MLLID_NUM];

static bdmf_boolean find_free_mllid_flow_and_save_index(bdmf_index *index, bdmf_index *flow)
{
    bdmf_index i;

/* check range */
    if ((unsigned)*flow >= RDPA_MAX_GEM_FLOW)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "flow id %ld is out of range\n", *flow);

    if (!(*index >= 0 && *index < RDPA_EPON_MLLID_NUM) && *index != BDMF_INDEX_UNASSIGNED)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "index %ld is out of range\n", *index);
        goto error;
    }

    for (i = 0; i < RDPA_EPON_MLLID_NUM; i++)
    {
        if (mllid_objects[i] && ((mllid_drv_priv_t *)bdmf_obj_data(mllid_objects[i]))->flow_id == *flow)
        {
            BDMF_TRACE_RET(BDMF_ERR_ALREADY, "MLLID already exists, flow_id: %ld\n", *flow);
            goto error;
        }
    }

/* find save index */
    if (*index == BDMF_INDEX_UNASSIGNED)
    {
        for (i = 0; i < RDPA_EPON_MLLID_NUM; i++)
        {
            if (!mllid_objects[i])
            {
                *index = i;
                return BDMF_ERR_OK;
            }
        }
    }
    else if (mllid_objects[*index])
    {
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "MLLID index %ld already exists\n", *index);
        goto error;
    }
    else
        return BDMF_ERR_OK;

error:
    return BDMF_ERR_PARM;
}

/* "enable" attribute "write" callback */
static int mllid_attr_enable_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    mllid_drv_priv_t *mllid = (mllid_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;
    int rc = 0;

    if (mo->state == bdmf_state_active)
    {
        if (mllid_objects[mllid->index] != mo)
            BDMF_TRACE_RET(BDMF_ERR_PARM, "Received mllid %ld: doesn't exist\n", mllid->index);

        if (enable != mllid->enable)
        {
            rc = _cfg_ds_gem_flow_hw(enable, mllid->flow_id, (uint16_t)RDPA_VALUE_UNASSIGNED, rdpa_flow_dest_eth,
                 rdpa_discard_prty_low, RDPA_UNMATCHED_DS_IC_RESULT_ID);
            if (rc < 0)
                return rc;

            mllid->enable = enable;
        }
    }

    return BDMF_ERR_OK;
}



/*  mllid stat aggregate type */
struct bdmf_aggr_type mllid_stat_type = {
    .name = "mllid_stat", .struct_name = "rdpa_mllid_stat_t",
    .help = "MLLID Statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "rx_packets", .help = "Rx Packets", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_mllid_stat_t, rx_packets),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        { .name = "rx_bytes", .help = "Rx Bytes", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_mllid_stat_t, rx_bytes),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        { .name = "rx_packets_discard", .help = "Rx Packet discard", .size = sizeof(uint32_t),
          .type = bdmf_attr_number, .offset = offsetof(rdpa_mllid_stat_t, rx_packets_discard),
          .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(mllid_stat_type);


/* Object attribute descriptors */
static struct bdmf_attr mllid_attrs[] =
{
    { .name = "index", .help = "MLLID index", .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY,
        .type = bdmf_attr_number, .offset = offsetof(mllid_drv_priv_t, index)
    },
    { .name = "flow_id", .help = "Flow index", .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
        .type = bdmf_attr_number, .offset = offsetof(mllid_drv_priv_t, flow_id)
    },
    { .name = "enable", .help = "Enable MLLID service",
        .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean,
        .offset = offsetof(mllid_drv_priv_t, enable),
        .write = mllid_attr_enable_write
    },
    { .name = "stat", .help = "MLLID statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "mllid_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT,
        .read = mllid_attr_stat_read_ex, .write = mllid_attr_stat_write_ex
    },
    BDMF_ATTR_LAST
};

/** This optional callback is called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int mllid_pre_init(struct bdmf_object *mo)
{
    mllid_drv_priv_t *mllid = (mllid_drv_priv_t *)bdmf_obj_data(mo);

    if (!rdpa_is_epon_or_xepon_mode())
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "mllid object is only "
            "supported with EPON uplink\n");
    }

    mllid->index = BDMF_INDEX_UNASSIGNED;
    mllid->flow_id = BDMF_INDEX_UNASSIGNED;
    mllid->enable = 1;

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

static int mllid_post_init(struct bdmf_object *mo)
{
    mllid_drv_priv_t *mllid = (mllid_drv_priv_t *)bdmf_obj_data(mo);
    int rc = BDMF_ERR_OK;

    /* Assign MLLID if not assigned */
    if (find_free_mllid_flow_and_save_index(&(mllid->index), &(mllid->flow_id)))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "para check error, flow:%ld, index:%ld\n", mllid->flow_id, mllid->index);

    /* set enable */
    if (mllid->enable)
    {
        rc = _cfg_ds_gem_flow_hw(1, mllid->flow_id, (uint16_t)RDPA_VALUE_UNASSIGNED, rdpa_flow_dest_eth,
            rdpa_discard_prty_low, RDPA_UNMATCHED_DS_IC_RESULT_ID);

        if (rc)
            BDMF_TRACE_RET_OBJ(rc, mo, "mllid configuration failed, index: %ld, flow_id: %ld", mllid->index, mllid->flow_id);
    }

    rc = mllid_post_init_ex(mllid->index, mllid->flow_id);
    if (rc)
        BDMF_TRACE_RET_OBJ(rc, mo, "mllid post init failed, index: %ld, flow_id: %ld", mllid->index, mllid->flow_id);

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "mllid/index=%ld", mllid->index);
    mllid_objects[mllid->index] = mo;

    return rc;
}

static void mllid_destroy(struct bdmf_object *mo)
{
    mllid_drv_priv_t *mllid = (mllid_drv_priv_t *)bdmf_obj_data(mo);

    if ((unsigned)mllid->index >= RDPA_EPON_MLLID_NUM || mllid_objects[mllid->index] != mo)
        return;

    if (mllid->enable)
        _cfg_ds_gem_flow_hw(0, mllid->flow_id, (uint16_t)RDPA_VALUE_UNASSIGNED, rdpa_flow_dest_eth,
            rdpa_discard_prty_low, RDPA_UNMATCHED_DS_IC_RESULT_ID);

    mllid_objects[mllid->index] = NULL;
}

static int mllid_drv_init(struct bdmf_type *drv);
static void mllid_drv_exit(struct bdmf_type *drv);

struct bdmf_type mllid_drv = {
    .name = "mllid",
    .parent = "system",
    .description = "MLLID channel",
    .drv_init = mllid_drv_init,
    .drv_exit = mllid_drv_exit,
    .pre_init = mllid_pre_init,
    .post_init = mllid_post_init,
    .destroy = mllid_destroy,
    .extra_size = sizeof(mllid_drv_priv_t),
    .aattr = mllid_attrs,
    .max_objs = RDPA_EPON_MLLID_NUM,
};
DECLARE_BDMF_TYPE(rdpa_mllid, mllid_drv);

/*
 * RDPA- interface
 */

/* Init/exit module. Cater for GPL layer */
static int mllid_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_mllid_drv = rdpa_mllid_drv;
    f_rdpa_mllid_get = rdpa_mllid_get;
#endif
    return BDMF_ERR_OK;
}

static void mllid_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_mllid_drv = NULL;
    f_rdpa_mllid_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/
int rdpa_mllid_get(bdmf_number index, bdmf_object_handle *mllid_obj)
{
    if ((unsigned)index >= RDPA_EPON_MLLID_NUM)
        return BDMF_ERR_PARM;

    if (!mllid_objects[index] || mllid_objects[index]->state == bdmf_state_deleted)
        return BDMF_ERR_NODEV;

    bdmf_get(mllid_objects[index]);
    *mllid_obj = mllid_objects[index];

    return BDMF_ERR_OK;
}

