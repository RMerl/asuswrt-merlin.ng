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
 * rdpa_tcont.c
 *
 *  Created on: June 22, 2014
 *      Author: Yoni Itah
 */


#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#ifndef XRDP
#include "rdp_drv_bbh.h"
#include "rdd_ih_defs.h"
#endif
#include "rdd.h"
#include "rdpa_tcont_ex.h"

extern int (*f_rdpa_tcont_sr_dba_callback)(uint32_t tcont_id, uint32_t *runner_ddr_occupancy);

#ifdef XRDP
extern int (*f_rdpa_wan_tx_bbh_flush_status_get)(uint8_t tcont_id, bdmf_boolean *bbh_flush_done_p);
#endif

/* tcont object private data */
typedef struct 
{
    bdmf_index index; /* TCONT index */
    bdmf_index channel;
    bdmf_boolean mgmt; /* YES:OMCI Management tcont */	
    uint16_t alloc_id; /* Alloc id */
    bdmf_boolean assign_ploam_flag; /* true if assign alloc id message arrived */
    bdmf_object_handle egress_tm; /* Scheduler object instance */
    bdmf_boolean enable; /* enable TCONT */
    rdpa_tm_orl_prty orl_prty; /* priority for overall rate limiter */
} tcont_drv_priv_t;

static struct bdmf_object *tcont_objects[RDPA_MAX_TCONT];
static uint32_t max_tcont_number;
static uint32_t data_tcont_number; /* Number of created non-management tconts. */

/* "orl_prty" attribute "write" callback */
static int tcont_attr_orl_prty_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    tcont_drv_priv_t *tcont = (tcont_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_orl_prty orl_prty = *(rdpa_tm_orl_prty *)val;

    if (mo->state == bdmf_state_active && tcont->egress_tm)
    {
        int rc;
        rc = _rdpa_egress_tm_orl_prty_set(tcont->egress_tm, orl_prty);
        if (rc < 0)
            return rc;
    }

    tcont->orl_prty = orl_prty;

    return 0;
}

/** This optional callback is called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int tcont_pre_init(struct bdmf_object *mo)
{
    tcont_drv_priv_t *priv = (tcont_drv_priv_t *)bdmf_obj_data(mo);

    priv->index = BDMF_INDEX_UNASSIGNED;
    priv->channel = BDMF_INDEX_UNASSIGNED;
    priv->alloc_id = (uint16_t)RDPA_VALUE_UNASSIGNED;
    priv->enable = 1;
    priv->mgmt = 0;
    max_tcont_number = (rdpa_is_car_mode()) ?
      (RDPA_US_QOS_AND_SCHEDULE_METHOD_ETH_MAX_TCONT_NUM + 1) : RDPA_MAX_TCONT;

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
static int tcont_post_init(struct bdmf_object *mo)
{
    tcont_drv_priv_t *priv = (tcont_drv_priv_t *)bdmf_obj_data(mo);
    int i, rc = 0;

    /* Assign TCONT if not assigned */

    /* find empty index */
    if (priv->index < 0)
    {
        for (i = 0; i < max_tcont_number; i++)
        {
            if (!tcont_objects[i])
            {
                priv->index = i;
                break;
            }
        }
    }

    if ((unsigned)priv->index >= max_tcont_number)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many TCONTs or index %ld is out of range\n", priv->index);
    if (tcont_objects[priv->index])
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "TCONT %ld already exists\n", priv->index);
    if ((rdpa_is_car_mode()) &&  (priv->mgmt == 0) &&
      ((data_tcont_number + 1) >= max_tcont_number))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many TCONTs\n");

    priv->channel = rdpa_tcont_tcont_id_to_channel_id(priv->index);
    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "tcont/index=%ld", priv->index);

    /* Auto-assign egress_tm in CAR mode */
    if (!priv->egress_tm && rdpa_is_car_mode() && (priv->mgmt != 1))
    {
        bdmf_object_handle wan_port = NULL;
        rdpa_port_get(rdpa_wan_type_to_if(rdpa_wan_gpon), &wan_port);
        if (wan_port)
        {
            rdpa_port_tm_cfg_t tm_cfg = {};
            rc = rdpa_port_tm_cfg_get(wan_port, &tm_cfg);
            rc = rc ? rc : rdpa_tcont_egress_tm_set(mo, tm_cfg.sched);

            bdmf_put(wan_port);
            if (rc)
                BDMF_TRACE_RET_OBJ(rc, mo, "Can't auto-assign egress_tm\n");
        }
    }

    if (priv->mgmt == 0)
    {
        data_tcont_number++;
    }

    /* Do cleanup in destroy past this point */
    tcont_objects[priv->index] = mo;
    if (priv->egress_tm)
    {
        rc = _rdpa_egress_tm_channel_set(priv->egress_tm, mo, rdpa_wan_gpon, priv->channel);
        rc = rc ? rc : _rdpa_egress_tm_orl_prty_set(priv->egress_tm, priv->orl_prty);
        if (priv->enable)
            rc = rc ? rc : _rdpa_egress_tm_enable_set(priv->egress_tm, 1, 0);
        if (rc)
            BDMF_TRACE_RET_OBJ(rc, mo, "Can't set channel or enable egress_tm\n");
    }
#if defined(BCM_PON_XRDP)
    RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(priv->channel, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, priv->channel);
#endif
    return rc;
}

static void tcont_destroy(struct bdmf_object *mo)
{
    tcont_drv_priv_t *priv = (tcont_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle gem = NULL;
    rdpa_gem_flow_us_cfg_t gem_us_cfg;

    /* Only cleanup if tcont setup passed the point of config in gpon stack */
    if ((unsigned)priv->index >= max_tcont_number || tcont_objects[priv->index] != mo)
        return;

    /* Scheduler cleanup */
    if (priv->egress_tm)
        _rdpa_egress_tm_channel_set(priv->egress_tm, NULL, rdpa_wan_none, priv->channel);

    /* Gem ports cleanup */
    while ((gem = bdmf_get_next(rdpa_gem_drv(), gem, NULL)))
    {
        rdpa_gem_us_cfg_get(gem, &gem_us_cfg);

        if (gem_us_cfg.tcont == mo)
            rdpa_gem_us_cfg_set(gem, NULL);
    }

    if (priv->mgmt == 0)
    {
        data_tcont_number--;
    }

    tcont_objects[priv->index] = NULL;
}

/*
 * Attribute access functions
 */

/* "egress_tm" attribute "write" callback */
static int tcont_attr_egress_tm_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    tcont_drv_priv_t *tcont = (tcont_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle egress_tm = *(bdmf_object_handle *)val;
    int rc = 0;

    if (mo->state != bdmf_state_init && egress_tm != tcont->egress_tm)
    {
        if (egress_tm)
        {
            rc = _rdpa_egress_tm_channel_set(egress_tm, mo, rdpa_wan_gpon, tcont->channel);
            rc = rc ? rc : _rdpa_egress_tm_orl_prty_set(egress_tm, tcont->orl_prty);
            if (tcont->enable)
                rc = rc ? rc : _rdpa_egress_tm_enable_set(egress_tm, 1, 0);
            if (rc)
                _rdpa_egress_tm_channel_set(egress_tm, NULL, rdpa_wan_none, tcont->channel);
        }
        else
        {
            rc = _rdpa_egress_tm_channel_set(tcont->egress_tm, NULL, rdpa_wan_none, tcont->channel);
        }
        if (rc < 0)
            return rc;
    }

    if (!rdpa_is_car_mode())
        replace_ownership(tcont->egress_tm, egress_tm, mo);

    tcont->egress_tm = egress_tm;

    return 0;
}

/* "enable" attribute "write" callback */
static int tcont_attr_enable_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    tcont_drv_priv_t *tcont = (tcont_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;
    int rc = 0;

    if (mo->state == bdmf_state_active && tcont->egress_tm && enable != tcont->enable)
    {
        rc = _rdpa_egress_tm_enable_set(tcont->egress_tm, enable, 0);
        if (rc < 0)
            return rc;
    }

    tcont->enable = enable;

    return 0;
}
/* "is_empty" attribute "read" callback */
static int tcont_attr_attr_is_empty_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    tcont_drv_priv_t *tcont = (tcont_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;

    *(bdmf_boolean *)val = 1;
    if (tcont->egress_tm)
    {
        egress_tm_is_empty_on_channel(tcont->egress_tm, tcont->channel, (bdmf_boolean *)val);
        BDMF_TRACE_DBG("tcont is_empty = %d\n", *(bdmf_boolean *)val);
        return rc;
    }
    return 0;
}


/*
 * aggregate descriptors
 */

/* Object attribute descriptors */
static struct bdmf_attr tcont_attrs[] =
{
    { .name = "index", .help = "TCONT index", .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .type = bdmf_attr_number, .offset = offsetof(tcont_drv_priv_t, index)
    },
    { .name = "management", .help = "Yes: OMCI management TCONT", .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean, .offset = offsetof(tcont_drv_priv_t, mgmt),
    },
    { .name = "egress_tm", .help = "US scheduler object", .size = sizeof(bdmf_object_handle),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_object, .ts.ref_type_name = "egress_tm", .offset = offsetof(tcont_drv_priv_t, egress_tm),
        .write = tcont_attr_egress_tm_write
    },
    { .name = "enable", .help = "Enable TCONT", .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG | BDMF_ATTR_NOLOCK,
        .type = bdmf_attr_boolean, .offset = offsetof(tcont_drv_priv_t, enable),
        .write = tcont_attr_enable_write
    },
    { .name = "is_empty", .help = "check if TCONT is empty ",
        .flags = BDMF_ATTR_READ,
        .type = bdmf_attr_boolean,
        .read = tcont_attr_attr_is_empty_read
    },
    { .name = "orl_prty", .help = "Priority for overall rate limiter", .size = sizeof(rdpa_tm_orl_prty),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG, .offset = offsetof(tcont_drv_priv_t, orl_prty),
        .type = bdmf_attr_enum, .ts.enum_table = &orl_prty_enum_table, .write = tcont_attr_orl_prty_write
    },
    BDMF_ATTR_LAST
};

static int tcont_drv_init(struct bdmf_type *drv);
static void tcont_drv_exit(struct bdmf_type *drv);

struct bdmf_type tcont_drv = {
    .name = "tcont",
    .parent = "system",
    .description = "TCONT",
    .drv_init = tcont_drv_init,
    .drv_exit = tcont_drv_exit,
    .pre_init = tcont_pre_init,
    .post_init = tcont_post_init,
    .destroy = tcont_destroy,
    .extra_size = sizeof(tcont_drv_priv_t),
    .aattr = tcont_attrs,
    .flags = BDMF_DRV_FLAG_MUXUP | BDMF_DRV_FLAG_MUXDOWN,
    .max_objs = RDPA_MAX_TCONT,
};
DECLARE_BDMF_TYPE(rdpa_tcont, tcont_drv);

/* Init/exit module. Cater for GPL layer */
static int tcont_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_tcont_drv = rdpa_tcont_drv;
    f_rdpa_tcont_get = rdpa_tcont_get;
    f_rdpa_tcont_sr_dba_callback = rdpa_tcont_sr_dba_callback;
#ifdef XRDP 
    f_rdpa_wan_tx_bbh_flush_status_get = tcont_tx_bbh_flush_status_get;
#endif
#endif
    return 0;
}

static void tcont_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_tcont_drv = NULL;
    f_rdpa_tcont_get = NULL;

#ifdef XRDP 
    f_rdpa_wan_tx_bbh_flush_status_get = NULL;
#endif
#endif
}

int rdpa_tcont_get(bdmf_number _index_, bdmf_object_handle *tcont_obj)
{
    if (_index_ >= RDPA_MAX_TCONT)
        return BDMF_ERR_NODEV;

    if (!tcont_objects[_index_] || tcont_objects[_index_]->state == bdmf_state_deleted)
        return BDMF_ERR_NODEV;

    bdmf_get(tcont_objects[_index_]);
    *tcont_obj = tcont_objects[_index_];

    return 0;
}

int rdpa_tcont_is_mgmt(bdmf_object_handle mo)
{
    tcont_drv_priv_t *tcont = (tcont_drv_priv_t *)bdmf_obj_data(mo);
    return tcont->mgmt;
}

int rdpa_tcont_channel_get(bdmf_object_handle mo, bdmf_number *channel_index)
{
    tcont_drv_priv_t *tcont;

    tcont = (tcont_drv_priv_t *)bdmf_obj_data(mo);
    *channel_index = tcont->channel;

    return 0;
}
