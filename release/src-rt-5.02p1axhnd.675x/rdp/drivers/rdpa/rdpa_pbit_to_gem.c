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

/*
 * rdpa_pbit_to_gem.c
 *
 * QoS mapping table
 *
 *  Created on: Sep 17, 2013
 *      Author: hila
 */


#include <bdmf_dev.h>
#include <rdpa_api.h>
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdd.h"
#ifdef XRDP
#include "rdd_qos_mapper.h"
#endif

#define RDPA_PBIT_TO_GEM_MAX_TABLES 16 /* Max number of PBIT-to-GEM mapping tables */
#define RDPA_PBIT_NUM 8  
#define RDPA_LAST_GEM_INDEX  255

/***************************************************************************
 * pbit_to_gem object type
 **************************************************************************/

static bdmf_object_handle pbit_to_gem_objects[RDPA_PBIT_TO_GEM_MAX_TABLES] = {};

/* pbit_to_queue object private data */
typedef struct {
    bdmf_index index;
    struct bdmf_object *gem[RDPA_PBIT_NUM];   
} pbit_to_gem_drv_priv_t;

/* This optional callback is called at object init time before initial attributes are set.
 * If function returns error code != 0, object creation is aborted
 */
static int pbit_to_gem_pre_init(struct bdmf_object *mo)
{
    pbit_to_gem_drv_priv_t *tbl = (pbit_to_gem_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    /* Set defaults */
    for (i = 0; i < RDPA_PBIT_NUM; i++)
        tbl->gem[i] = NULL;
    tbl->index = BDMF_INDEX_UNASSIGNED;

    return 0;
}

/* This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int pbit_to_gem_post_init(struct bdmf_object *mo)
{
    pbit_to_gem_drv_priv_t *tbl = (pbit_to_gem_drv_priv_t *)bdmf_obj_data(mo);
    int rc, i;
    bdmf_number gem_idx;

    if (!rdpa_is_gpon_or_xgpon_mode())
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Pbit to gem table is only supported in GPON mode\n");
    }

    /* if index is not set explicitly - assign free */
    if (tbl->index < 0)
    {
        int i;

        /* Find and assign free index */
        for (i = 0; i < RDPA_PBIT_TO_GEM_MAX_TABLES; i++)
        {
            if (!pbit_to_gem_objects[i])
            {
                tbl->index = i;
                break;
            }
        }
    }

    if ((unsigned)tbl->index >= RDPA_PBIT_TO_GEM_MAX_TABLES)
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Table index %u is out of range\n", (unsigned)tbl->index);

    if (pbit_to_gem_objects[tbl->index])
        BDMF_TRACE_RET_OBJ(BDMF_ERR_ALREADY, mo, "%s index %d is already exists\n", mo->name, (int)tbl->index);

    /* RDD configuration */
    for (i = 0; i < RDPA_PBIT_NUM; i++)
    {
        if (tbl->gem[i])
        {
            rc = rdpa_gem_index_get(tbl->gem[i], &gem_idx);
            if (rc) 
                BDMF_TRACE_RET(BDMF_ERR_PARM, "GEM Object does not exist\n");
            rc = rdd_us_pbits_to_wan_flow_entry_cfg((uint8_t)tbl->index, (uint8_t)i, (uint8_t)gem_idx);
            if (rc)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "RDD GEM configuration failed, error: %d\n", rc);
        }
        else
        {
            rc = rdd_us_pbits_to_wan_flow_entry_cfg((uint8_t)tbl->index, (uint8_t)i, RDPA_LAST_GEM_INDEX);
            if (rc)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "RDD GEM configuration disable failed, error: %d\n", rc);
        }
    }

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "pbit_to_gem/index=%ld", tbl->index);

    pbit_to_gem_objects[tbl->index] = mo;

    return 0;
}

static void pbit_to_gem_destroy(struct bdmf_object *mo)
{
    uint8_t i;
    int rc;
    pbit_to_gem_drv_priv_t *tbl = (pbit_to_gem_drv_priv_t *)bdmf_obj_data(mo);

    if (tbl->index >= RDPA_PBIT_TO_GEM_MAX_TABLES || pbit_to_gem_objects[tbl->index] != mo)
        return;

    for (i = 0; i < RDPA_PBIT_NUM; i++)
    {
        rc = rdd_us_pbits_to_wan_flow_entry_cfg((int)tbl->index, i, RDPA_LAST_GEM_INDEX);
        if (rc)
            BDMF_TRACE_ERR("Failed to destroy pbit_to_gem for index %u, error: %d\n", i, rc);
    }
    pbit_to_gem_objects[tbl->index] = NULL;
}

/*
 * pbit_to_GEM attribute access
 */

/* "pbit_to_gem" attribute's "write" callback */
static int pbit_to_gem_attr_write(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    pbit_to_gem_drv_priv_t *priv_tbl = (pbit_to_gem_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle gem = *(bdmf_object_handle *)val;
    bdmf_number gem_idx;
    int rc;

    if (index >= RDPA_PBIT_NUM || index < 0)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "pBit %d out of range\n", (int)index);

    if (gem)
    {
        rc = rdpa_gem_index_get(gem, &gem_idx);
        if (rc) 
            BDMF_TRACE_RET(BDMF_ERR_PARM, "GEM Object does not exist\n");

        if (mo->state == bdmf_state_active)
        {
            rc = rdd_us_pbits_to_wan_flow_entry_cfg((uint8_t)priv_tbl->index, (uint8_t)index, (uint8_t)gem_idx);
            if (rc)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "RDD GEM configuration failed, error: %d\n", rc);
        }
    }
    else
    {
            rc = rdd_us_pbits_to_wan_flow_entry_cfg((uint8_t)priv_tbl->index, (uint8_t)index, RDPA_LAST_GEM_INDEX);
            if (rc)
                BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "RDD GEM configuration disable failed, error: %d\n", rc);
    }

    priv_tbl->gem[index] = gem;

    return 0;
}

/* "pbit_to_queue" attribute "read" callback */
static int pbit_to_gem_attr_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val,
    uint32_t size)
{
    pbit_to_gem_drv_priv_t *tbl = (pbit_to_gem_drv_priv_t *)bdmf_obj_data(mo);

    if (index >= RDPA_PBIT_NUM || index < 0)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "pBit %d out of range\n", (int)index);

    if (!tbl->gem[index])
        return BDMF_ERR_NOENT;

    *(bdmf_object_handle *)val = tbl->gem[index];
    return 0;
}

/* Object attribute descriptors */
static struct bdmf_attr pbit_to_gem_attrs[] =
{
    {
        .name = "index",
        .help = "Table index",
        .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index),
        .offset = offsetof(pbit_to_gem_drv_priv_t, index)
    },
    {
        .name = "pbit_map",
        .help = "Priority to GEM array",
        .type = bdmf_attr_object,
        .size = sizeof(bdmf_object_handle),
        .offset = offsetof(pbit_to_gem_drv_priv_t, gem),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .array_size = RDPA_PBIT_NUM,
        .write = pbit_to_gem_attr_write, 
        .read = pbit_to_gem_attr_read
    },
    BDMF_ATTR_LAST
};

static int pbit_to_gem_drv_init(struct bdmf_type *drv);
static void pbit_to_gem_drv_exit(struct bdmf_type *drv);

struct bdmf_type pbit_to_gem_drv =
{
    .name = "pbit_to_gem",
    .parent = "system",
    .description = "PBIT to GEM mapping table",
    .drv_init = pbit_to_gem_drv_init,
    .drv_exit = pbit_to_gem_drv_exit,
    .pre_init = pbit_to_gem_pre_init,
    .post_init = pbit_to_gem_post_init,
    .destroy = pbit_to_gem_destroy,
    .extra_size = sizeof(pbit_to_gem_drv_priv_t),
    .aattr = pbit_to_gem_attrs,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN,
    .max_objs = RDPA_PBIT_TO_PRTY_MAX_TABLES,
};
DECLARE_BDMF_TYPE(rdpa_pbit_to_gem, pbit_to_gem_drv);

/* Init/exit module. Cater for GPL layer */
static int pbit_to_gem_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_pbit_to_gem_drv = rdpa_pbit_to_gem_drv;
    f_rdpa_pbit_to_gem_get = rdpa_pbit_to_gem_get;
#endif
    return 0;
}

static void pbit_to_gem_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_pbit_to_gem_drv = NULL;
    f_rdpa_pbit_to_gem_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/
int rdpa_pbit_to_gem_get(bdmf_number index_, bdmf_object_handle *pbit_to_gem_obj)
{
    if (!pbit_to_gem_objects[index_] || pbit_to_gem_objects[index_]->state == bdmf_state_deleted)
        return BDMF_ERR_NODEV;
    bdmf_get(pbit_to_gem_objects[index_]);
    *pbit_to_gem_obj = pbit_to_gem_objects[index_];
    return 0;
}
