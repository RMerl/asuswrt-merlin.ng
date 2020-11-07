/*
* <:copyright-BRCM:2012-2015:proprietary:standard
* 
*    Copyright (c) 2012-2015 Broadcom 
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
 * rdpa_xtm.c
 *
 *  Created on: Aug 23, 2012
 *      Author: igort
 */

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_platform.h"
#include "rdpa_xtm_ex.h"
#include "rdd.h"

static int _cfg_us_xtmflow_hw(bdmf_boolean cfg_xtmflow, bdmf_index xtmflow,
    bdmf_object_handle xtmchannel, uint16_t hdr_type, uint16_t fstat, bdmf_boolean calc_crc,
    int ptm_bonding);


/***************************************************************************
 * xtm_object type
 **************************************************************************/

/* xtm object private data */
typedef struct {
    bdmf_index index;                 /**< XTM Index */
} xtm_drv_priv_t;

static struct bdmf_object *xtm_object;
static struct bdmf_object *xtmchannel_objects[RDPA_MAX_XTMCHANNEL];
static struct bdmf_object *xtmflow_objects[RDPA_MAX_XTMFLOW];

static uint32_t max_xtmchannel_number;

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int xtm_pre_init(struct bdmf_object *mo)
{
    xtm_drv_priv_t *priv = (xtm_drv_priv_t *)bdmf_obj_data(mo);

    priv->index = 0;

    max_xtmchannel_number = RDPA_MAX_XTMCHANNEL;

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
static int xtm_post_init(struct bdmf_object *mo)
{
    int rc = 0, i;

    xtm_object = mo;

    snprintf(mo->name, sizeof(mo->name), "xtm");

    for (i = 0; i < max_xtmchannel_number; i++)
    {
        rc = _cfg_ds_xtm_channel(i);
        if (rc)
            BDMF_TRACE_RET(rc, "XTMCHANNEL %d configuration failed\n", i);
    }

    return rc;
}

static void xtm_destroy(struct bdmf_object *mo)
{
    if (xtm_object != mo)
        return;

    /* ToDo: do cleanups here */
    xtm_object = NULL;
}

/** find onu object */
static int xtm_get(struct bdmf_type *drv,
           struct bdmf_object *owner, const char *discr,
           struct bdmf_object **pmo)
{
    if (!xtm_object)
        return BDMF_ERR_NOENT;
    *pmo = xtm_object;
    return 0;
}

/*
 * enum tables
 */

/*
 * aggregate descriptors
 */

/*
 * aggregate descriptors
 */

/*
 * aggregate descriptors
 */

/*
 * Attribute access functions
 */

/* Object attribute descriptors */
static struct bdmf_attr xtm_attrs[] = {
    { .name = "index", .help = "XTM Index",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_number, .size = sizeof(bdmf_index),
        .offset = offsetof(xtm_drv_priv_t, index)
    },
    BDMF_ATTR_LAST
};

static int xtm_drv_init(struct bdmf_type *drv);
static void xtm_drv_exit(struct bdmf_type *drv);

struct bdmf_type xtm_drv = {
    .name = "xtm",
    .parent = "system",
    .description = "xDSL Link / CPE",
    .drv_init = xtm_drv_init,
    .drv_exit = xtm_drv_exit,
    .pre_init = xtm_pre_init,
    .post_init = xtm_post_init,
    .destroy = xtm_destroy,
    .get = xtm_get,
    .extra_size = sizeof(xtm_drv_priv_t),
    .aattr = xtm_attrs,
    .max_objs = 1,
};
DECLARE_BDMF_TYPE(rdpa_xtm, xtm_drv);

/***************************************************************************
 * xtmchannel object type
 **************************************************************************/

/* xtmchannel object private data */
typedef struct {
    bdmf_index index;                   /* XTMCHANNEL index */
    bdmf_index channel;                 /* egress_tm channel index */
    bdmf_object_handle egress_tm;       /* Scheduler object instance */
    bdmf_boolean enable;                /* enable XTMCHANNEL */
    rdpa_tm_orl_prty orl_prty;          /* priority for overall rate limiter */
} xtmchannel_drv_priv_t;

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int xtmchannel_pre_init(struct bdmf_object *mo)
{
    xtmchannel_drv_priv_t *priv = (xtmchannel_drv_priv_t *)bdmf_obj_data(mo);

    priv->index = BDMF_INDEX_UNASSIGNED;
    priv->channel = BDMF_INDEX_UNASSIGNED;
    priv->enable = 1;

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
static int xtmchannel_post_init(struct bdmf_object *mo)
{
    xtmchannel_drv_priv_t *priv = (xtmchannel_drv_priv_t *)bdmf_obj_data(mo);
    int i, rc = 0;

    /* Assign XTMCHANNEL if not assigned */

    /* find empty index */
    if (priv->index < 0)
    {
        for (i = 0; i < max_xtmchannel_number; i++)
        {
            if (!xtmchannel_objects[i])
            {
                priv->index = i;
                break;
            }
        }
    }
    if ((unsigned)priv->index >= max_xtmchannel_number)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many XTMCHANNELs or index %ld is out of range\n", priv->index);
    if (xtmchannel_objects[priv->index])
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "XTMCHANNEL %ld already exists\n", priv->index);
    priv->channel = rdpa_xtm_xtmchannel_id_to_tm_channel_id(priv->index);

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "xtmchannel/index=%ld", priv->index);

    /* Do cleanup in destroy past this point */
    xtmchannel_objects[priv->index] = mo;
    if (priv->egress_tm)
    {
        rc = _rdpa_egress_tm_channel_set(priv->egress_tm, mo, rdpa_wan_dsl, priv->channel);
        rc = rc ? rc : _rdpa_egress_tm_orl_prty_set(priv->egress_tm, priv->orl_prty);
        if (priv->enable)
            rc = rc ? rc : _rdpa_egress_tm_enable_set(priv->egress_tm, 1, 0);
        if (rc)
            BDMF_TRACE_RET_OBJ(rc, mo, "Can't set channel or enable egress_tm\n");
    }

    return rc;
}

static void xtmchannel_destroy(struct bdmf_object *mo)
{
    xtmchannel_drv_priv_t *priv = (xtmchannel_drv_priv_t *)bdmf_obj_data(mo);

    /* Only cleanup if xtmchannel setup passed the point of config in DSL stack */
    if ((unsigned)priv->index >= max_xtmchannel_number || xtmchannel_objects[priv->index] != mo)
        return;

    /* Scheduler cleanup */
    if (priv->egress_tm)
        _rdpa_egress_tm_channel_set(priv->egress_tm, NULL, rdpa_wan_none, priv->channel);

    xtmchannel_objects[priv->index] = NULL;
}


/* "egress_tm" attribute "write" callback */
static int xtmchannel_attr_egress_tm_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    xtmchannel_drv_priv_t *xtmchannel = (xtmchannel_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle egress_tm = *(bdmf_object_handle *)val;
    int rc = 0;

    if (mo->state == bdmf_state_active)
    {
        if (egress_tm)
        {
            rc = _rdpa_egress_tm_channel_set(egress_tm, mo, rdpa_wan_dsl, xtmchannel->channel);
            rc = rc ? rc : _rdpa_egress_tm_orl_prty_set(egress_tm, xtmchannel->orl_prty);
            if (xtmchannel->enable)
                rc = rc ? rc : _rdpa_egress_tm_enable_set(egress_tm, 1, 0);
        }
        else
        {
            rc = _rdpa_egress_tm_channel_set(xtmchannel->egress_tm, NULL, rdpa_wan_none, xtmchannel->channel);
            xtmchannel->channel = BDMF_INDEX_ANY;
        }
        if (rc < 0)
            return rc;
    }

    xtmchannel->egress_tm = egress_tm;

    return 0;
}


/* "enable" attribute "write" callback */
static int xtmchannel_attr_enable_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    xtmchannel_drv_priv_t *xtmchannel = (xtmchannel_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;
    int rc = 0;

    if (mo->state == bdmf_state_active && xtmchannel->egress_tm && enable != xtmchannel->enable)
    {
        rc = _rdpa_egress_tm_enable_set(xtmchannel->egress_tm, enable, 0);
        if (rc < 0)
            return rc;
    }

    xtmchannel->enable = enable;

    return 0;
}

/* "orl_prty" attribute "write" callback */
static int xtmchannel_attr_orl_prty_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    xtmchannel_drv_priv_t *xtmchannel = (xtmchannel_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_orl_prty orl_prty = *(rdpa_tm_orl_prty *)val;

    if (mo->state == bdmf_state_active && xtmchannel->egress_tm)
    {
        int rc;
        rc = _rdpa_egress_tm_orl_prty_set(xtmchannel->egress_tm, orl_prty);
        if (rc < 0)
            return rc;
    }

    xtmchannel->orl_prty = orl_prty;

    return 0;
}

/* Object attribute descriptors */
static struct bdmf_attr xtmchannel_attrs[] =
{
    { .name = "index", .help = "XTMCHANNEL index", .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .type = bdmf_attr_number, .offset = offsetof(xtmchannel_drv_priv_t, index)
    },
    { .name = "egress_tm", .help = "US scheduler object", .size = sizeof(bdmf_object_handle),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_object, .ts.ref_type_name = "egress_tm",
        .offset = offsetof(xtmchannel_drv_priv_t, egress_tm),
        .write = xtmchannel_attr_egress_tm_write
    },
    { .name = "enable", .help = "Enable XTMCHANNEL", .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean, .offset = offsetof(xtmchannel_drv_priv_t, enable),
        .write = xtmchannel_attr_enable_write
    },
    { .name = "orl_prty", .help = "Priority for overall rate limiter", .size = sizeof(rdpa_tm_orl_prty),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(xtmchannel_drv_priv_t, orl_prty),
        .type = bdmf_attr_enum, .ts.enum_table = &orl_prty_enum_table, .write = xtmchannel_attr_orl_prty_write
    },
     BDMF_ATTR_LAST
};

struct bdmf_type xtmchannel_drv = {
    .name = "xtmchannel",
    .parent = "xtm",
    .description = "XTMCHANNEL",
    .pre_init = xtmchannel_pre_init,
    .post_init = xtmchannel_post_init,
    .destroy = xtmchannel_destroy,
    .extra_size = sizeof(xtmchannel_drv_priv_t),
    .aattr = xtmchannel_attrs,
    .max_objs = RDPA_MAX_XTMCHANNEL,
};
DECLARE_BDMF_TYPE(rdpa_xtmchannel, xtmchannel_drv);

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int xtmflow_pre_init(struct bdmf_object *mo)
{
    xtmflow_drv_priv_t *priv = (xtmflow_drv_priv_t *)bdmf_obj_data(mo);

    priv->index = BDMF_INDEX_UNASSIGNED;
    priv->hdr_type = (uint16_t)RDPA_VALUE_UNASSIGNED;
    priv->fstat = (uint16_t)RDPA_VALUE_UNASSIGNED;
    priv->us_cfg.xtmchannel = NULL;
    priv->ptm_bonding = (uint16_t)RDPA_VALUE_UNASSIGNED;
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
static int xtmflow_post_init(struct bdmf_object *mo)
{
    xtmflow_drv_priv_t *priv = (xtmflow_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_xtmflow_us_cfg_t *us_cfg = &priv->us_cfg;
    int i, rc = 0;

    /* Assign xtmflow if not assigned */
    /* find empty index */
    if (priv->index < 0)
    {
        for (i = 0; i < RDPA_MAX_XTMFLOW; i++)
        {
            if (!xtmflow_objects[i])
            {
                priv->index = i;
                break;
            }
        }
    }

    if ((unsigned)priv->index >= RDPA_MAX_XTMFLOW)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many xtmflows or index %ld is out of range\n", priv->index);
    if (xtmflow_objects[priv->index])
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "xtmflow %ld already exists\n", priv->index);

    if (priv->us_cfg.xtmchannel != NULL)
    {
        rc = _cfg_us_xtmflow_hw(1, priv->index, us_cfg->xtmchannel, priv->hdr_type, priv->fstat, 0, priv->ptm_bonding);
        if (rc < 0)
            BDMF_TRACE_RET(rc, "US xtmflow %ld configuration failed", priv->index);
    }

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "xtmflow/index=%ld", priv->index);
    xtmflow_objects[priv->index] = mo;
    return 0;
}

static void xtmflow_destroy(struct bdmf_object *mo)
{
    xtmflow_drv_priv_t *priv = (xtmflow_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_xtmflow_us_cfg_t *us_cfg = &priv->us_cfg;

    if ((unsigned)priv->index >= RDPA_MAX_XTMFLOW || xtmflow_objects[priv->index] != mo)
        return;

    xtmflow_objects[priv->index] = NULL;

    if (priv->us_cfg.xtmchannel != NULL)
    {
        _cfg_us_xtmflow_hw(0, priv->index, us_cfg->xtmchannel, priv->hdr_type, priv->fstat, 0, priv->ptm_bonding);
    }
}
/*
 * private enum tables
 */



/*
 * aggregate descriptors
 */

/*  xtmflow_us_cfg aggregate type */
struct bdmf_aggr_type xtmflow_us_cfg_type = {
    .name = "xtmflow_us_cfg", .struct_name = "rdpa_xtmflow_us_cfg_t",
    .help = "xtmflow US Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "xtmchannel", .help = "XTMCHANNEL",
            .type = bdmf_attr_object, .ts.ref_type_name = "xtmchannel",
            .size = sizeof(bdmf_object_handle), .offset = offsetof(rdpa_xtmflow_us_cfg_t, xtmchannel),
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(xtmflow_us_cfg_type);

/*  xtmflow_stat aggregate type */
struct bdmf_aggr_type xtmflow_stat_type = {
    .name = "xtmflow_stat", .struct_name = "rdpa_xtmflow_stat_t",
    .help = "xtmflow Statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "tx_packets", .help = "Tx Packets", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_xtmflow_stat_t, tx_packets),
            .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
        { .name = "tx_bytes", .help = "Tx Bytes", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_xtmflow_stat_t, tx_bytes),
            .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
#if defined(CONFIG_BCM_DSL_RDP)
        { .name = "tx_packets_discard", .help = "Tx Packets discard", .size = sizeof(uint32_t),
            .type = bdmf_attr_number, .offset = offsetof(rdpa_xtmflow_stat_t, tx_packets_discard),
            .flags = BDMF_ATTR_UNSIGNED, .ts.format = "%u"
        },
#endif
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(xtmflow_stat_type);

/*
 * Attribute access functions
 */

/* "stat" attribute "read" callback */
static int xtmflow_attr_stat_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc = 0;

    rdpa_xtmflow_stat_t *stat = (rdpa_xtmflow_stat_t *)val; 
    xtmflow_drv_priv_t *xtmflow = (xtmflow_drv_priv_t *)bdmf_obj_data(mo);

    /* Unconfigured xtmflow - silently return BDMF_ERR_NOENT */
    if ((unsigned)xtmflow->index >= RDPA_MAX_XTMFLOW || xtmflow_objects[xtmflow->index] != mo)
        return BDMF_ERR_NOENT;

    rc = rdpa_flow_pm_counters_get(xtmflow->index, stat);

    return rc;
}

static int _cfg_us_xtmflow_hw(bdmf_boolean cfg_xtmflow, bdmf_index xtmflow,
    bdmf_object_handle xtmchannel, uint16_t hdr_type, uint16_t fstat, bdmf_boolean calc_crc,
    int ptm_bonding)
{
    int rc = 0;
#ifndef BDMF_SYSTEM_SIM
    bdmf_number xtmchannel_index;
    xtmchannel_drv_priv_t *priv;

    if (cfg_xtmflow)
    {
        rdpa_xtmchannel_index_get(xtmchannel, &xtmchannel_index);
        priv = (xtmchannel_drv_priv_t *)bdmf_obj_data(xtmchannel_objects[xtmchannel_index]);
        if (priv == NULL)
            return BDMF_ERR_NOENT;
        rc = rdpa_us_wan_flow_config(xtmflow, (int)priv->channel,
            hdr_type, fstat, calc_crc, ptm_bonding, 0, 0, cfg_xtmflow);
    }
    else
    {
        rc = rdpa_us_wan_flow_config(xtmflow, 0, 0, 0, 0, 0, 0, 0, cfg_xtmflow);
    }

    if (rc)
    {
        BDMF_TRACE_ERR("us xtmflow %ld configuration error\n", xtmflow);
        rc = BDMF_ERR_INTERNAL;
    }
#endif
    return rc;
}

/* "us_cfg" attribute "write" callback. */
static int xtmflow_attr_us_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    xtmflow_drv_priv_t *priv = (xtmflow_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_xtmflow_us_cfg_t *cfg = (rdpa_xtmflow_us_cfg_t *)val;
    struct bdmf_object *xtmchannel_object = NULL;
    bdmf_number xtmchannel_index;
    int rc = 0;

    /* remove xtmflow */
    if (cfg == NULL)
    {
#ifndef BDMF_SYSTEM_SIM
       rc = rdpa_us_wan_flow_config(priv->index, 0, 0, 0, 0, 0, 0, 0, 0);
       if (rc)
          BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "remove us xtmflow %ld error\n", priv->index);
#endif
        priv->us_cfg.xtmchannel = NULL;
    }
    else
    {
        if (priv->us_cfg.xtmchannel)
            return BDMF_ERR_ALREADY;

        rdpa_xtmchannel_index_get(cfg->xtmchannel, &xtmchannel_index);

        xtmchannel_object = xtmchannel_objects[xtmchannel_index];
        if (!xtmchannel_object)
        {
            BDMF_TRACE_RET(BDMF_ERR_NOENT, "xtmflow %ld: xtmchannel/%ld doesn't exists\n",
                (long int)priv->index, (long int)xtmchannel_index);
            return BDMF_ERR_NOENT;
        }

        if (mo->state == bdmf_state_active)
        {
            if (xtmflow_objects[priv->index] != mo)
                BDMF_TRACE_RET(BDMF_ERR_PARM, "Received xtmflow %ld: doesn't exist\n", priv->index);

            rc = _cfg_us_xtmflow_hw(1, priv->index, cfg->xtmchannel, priv->hdr_type, priv->fstat, 0, priv->ptm_bonding);
            if (rc < 0)
                return rc;
        }
        priv->us_cfg = *cfg;
    }
    return rc;
}


/* Object attribute descriptors */
static struct bdmf_attr xtmflow_attrs[] = {
    { .name = "index", .help = "xtmflow index", .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY,
        .type = bdmf_attr_number, .offset = offsetof(xtmflow_drv_priv_t, index)
    },
    { .name = "hdr_type", .help = "xtmflow hdr_type", .size = sizeof(uint16_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
        .type = bdmf_attr_number, .offset = offsetof(xtmflow_drv_priv_t, hdr_type)
    },
    { .name = "fstat", .help = "xtmflow fstat", .size = sizeof(uint16_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
        .type = bdmf_attr_number, .offset = offsetof(xtmflow_drv_priv_t, fstat)
    },
    { .name = "us_cfg", .help = "US configuration",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_NULLCHECK,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "xtmflow_us_cfg",
        .offset = offsetof(xtmflow_drv_priv_t, us_cfg), .write = xtmflow_attr_us_cfg_write
    },
    { .name = "stat", .help = "xtmflow statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "xtmflow_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_STAT | BDMF_ATTR_WRITE,
        .read = xtmflow_attr_stat_read, .write = rdpa_xtmflow_attr_stat_write_ex
    },
    { .name = "ptmBonding", .help = "xtmflow ptmBonding", .size = sizeof(int),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_MANDATORY,
        .type = bdmf_attr_number, .offset = offsetof(xtmflow_drv_priv_t, ptm_bonding)
    },

    BDMF_ATTR_LAST
};

struct bdmf_type xtmflow_drv = {
    .name = "xtmflow",
    .parent = "xtm",
    .description = "xtmflow",
    .pre_init = xtmflow_pre_init,
    .post_init = xtmflow_post_init,
    .destroy = xtmflow_destroy,
    .extra_size = sizeof(xtmflow_drv_priv_t),
    .aattr = xtmflow_attrs,
    .max_objs = RDPA_MAX_XTMFLOW,
};
DECLARE_BDMF_TYPE(rdpa_xtmflow, xtmflow_drv);

/* Init/exit module. Cater for GPL layer */
static int xtm_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_xtm_drv = rdpa_xtm_drv;
    f_rdpa_xtmchannel_drv = rdpa_xtmchannel_drv;
    f_rdpa_xtmflow_drv = rdpa_xtmflow_drv;
    f_rdpa_xtmflow_get = rdpa_xtmflow_get;
    f_rdpa_xtm_get = rdpa_xtm_get;
    f_rdpa_xtmchannel_get = rdpa_xtmchannel_get;
#endif
    return 0;
}

static void xtm_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_xtm_drv = NULL;
    f_rdpa_xtmchannel_drv = NULL;
    f_rdpa_xtmflow_drv = NULL;
    f_rdpa_xtmflow_get = NULL;
    f_rdpa_xtm_get = NULL;
    f_rdpa_xtmchannel_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/
int rdpa_xtmflow_get(bdmf_number _index_, bdmf_object_handle *xtmflow_obj)
{
    if (!xtmflow_objects[_index_] || xtmflow_objects[_index_]->state == bdmf_state_deleted)
        return BDMF_ERR_NODEV;
    bdmf_get(xtmflow_objects[_index_]);
    *xtmflow_obj = xtmflow_objects[_index_];
    return 0;
}

int rdpa_xtmchannel_get(bdmf_number _index_, bdmf_object_handle *xtmchannel_obj)
{
    if (!xtmchannel_objects[_index_] || xtmchannel_objects[_index_]->state == bdmf_state_deleted)
        return BDMF_ERR_NODEV;
    bdmf_get(xtmchannel_objects[_index_]);
    *xtmchannel_obj = xtmchannel_objects[_index_];
    return 0;
}

int rdpa_xtm_get(bdmf_object_handle *xtm_obj)
{
    if (!xtm_object || xtm_object->state == bdmf_state_deleted)
        return BDMF_ERR_NOENT;
    bdmf_get(xtm_object);
    *xtm_obj = xtm_object;
    return 0;
}

/* The function returns xtm channel associated with given xtm flow.
   The function can be called in interrupt context.
   Returns 0 if OK or error < 0  */
int rdpa_xtm_flow_id_to_channel_id(int xtm_flow, int *channel_id)
{
    struct bdmf_object *xtmflow_obj;
    xtmflow_drv_priv_t *xtmflow_priv;
    struct bdmf_object *xtmchannel_object = NULL;
    xtmchannel_drv_priv_t *xtmchannel_priv;
    bdmf_number index;

    if ((unsigned)xtm_flow >= RDPA_MAX_XTMFLOW)
        return BDMF_ERR_PARM;

    xtmflow_obj = xtmflow_objects[xtm_flow];
    if (!xtmflow_obj)
        return BDMF_ERR_NOENT;

    xtmflow_priv = (xtmflow_drv_priv_t *)bdmf_obj_data(xtmflow_obj);
    if (!xtmflow_priv->us_cfg.xtmchannel)
        return BDMF_ERR_NOENT;

    rdpa_xtmchannel_index_get(xtmflow_priv->us_cfg.xtmchannel, &index);
    xtmchannel_object = xtmchannel_objects[index];
    if (!xtmchannel_object)
        return BDMF_ERR_NOENT;

    xtmchannel_priv = (xtmchannel_drv_priv_t *)bdmf_obj_data(xtmchannel_object);

    *channel_id = (int)xtmchannel_priv->channel;

    return 0;
}
