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
 * rdpa_llid.c
 *
 * EPON LLID driver
 */

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_ingress_class_int.h"
#include "rdd.h"
#include "rdpa_llid_ex.h"
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
#include <rdpa_epon.h>
#endif

/* LLID objects */
bdmf_object_handle llid_objects[RDPA_EPON_MAX_LLID];

/* Total number of available L1 queues */
#define RDPA_EPON_TOTAL_L1_QUEUES 32

#define RDPA_EPON_CTRL_CH_INDEX 0

/* Channel --> LLID mapper. -1=unassigned */
static int16_t epon_l1_channels[RDPA_EPON_TOTAL_L1_QUEUES];

rdpa_epon_mode g_epon_mode = rdpa_epon_ctc;

/* Mapping b/w control and data in DPoE mode */
extern epon_l2_l1_map epon_l2_l1_alloc[RDPA_EPON_MAX_LLID];

/* llid's total channel number */
#ifndef XRDP
int llid_total_chan_num = 9;
#else
int llid_total_chan_num = 8;
#endif

epon_l2_l1_map epon_l2_l1_alloc_dyn_ctc[] = { {1, 0} };

/* get number of llid total channels */
static inline int llid_get_total_channel_num(llid_drv_priv_t *llid)
{
    return llid->num_channels + 1;
}


/* get number of data channels for each llid */
static inline int16_t llid_get_data_channel_num(rdpa_epon_mode mode)
{
    int16_t number = 0;

    switch (mode)
    {
    case rdpa_epon_dpoe:
        /* fall through */
    case rdpa_epon_bcm:
#ifndef XRDP
        number = 1;
#else
        number = 0;
#endif
        break;
    case rdpa_epon_cuc_dyn:
        /* fall through */
    case rdpa_epon_ctc_dyn:
        number = 1;
        break;
    case rdpa_epon_ctc:
        /* fall through */
    case rdpa_epon_cuc:
        /* fall through */
    default:
#ifndef XRDP
        number = 8;
#else
        number = 7;
#endif
        break;
    }

    return number;
}

/* Does this llid has only one channel */
bdmf_boolean llid_has_dedicated_data_channel(llid_drv_priv_t *llid)
{
    return (llid->num_channels > 0);
}

/* Get free L1 channel */
static int llid_get_free_l1_channel(void)
{
    int i;

    for (i = 0; i < RDPA_EPON_TOTAL_L1_QUEUES; i++)
    {
        if (epon_l1_channels[i] == BDMF_INDEX_UNASSIGNED)
            return i;
    }
    return BDMF_INDEX_UNASSIGNED;
}

epon_l2_l1_map *rdpa_llid_l2_l1_map_get(uint8_t l1_queue)
{
    static epon_l2_l1_map *map;

    if (!map)
    {
        if (_rdpa_epon_mode_get() == rdpa_epon_ctc_dyn ||
            _rdpa_epon_mode_get() == rdpa_epon_cuc_dyn)
            map = &epon_l2_l1_alloc_dyn_ctc[0];
        else
            map = &epon_l2_l1_alloc[0];
    }

    return &map[l1_queue];
}

/* Set L1 channel */
int llid_set_l1_channel(llid_drv_priv_t *llid)
{
    if (g_epon_mode == rdpa_epon_dpoe || g_epon_mode == rdpa_epon_bcm ||
        g_epon_mode == rdpa_epon_ctc_dyn || g_epon_mode == rdpa_epon_cuc_dyn)
    {
        epon_l2_l1_map *map = rdpa_llid_l2_l1_map_get(0);

        llid->channels[0] = map[llid->index].l1_channel_control;
        epon_l1_channels[llid->channels[0]] = llid->index;

        if (llid_has_dedicated_data_channel(llid))
        {
            llid->channels[1] = map[llid->index].l1_channel_data;
            epon_l1_channels[llid->channels[1]] = llid->index;
        }
    }
    else
    {
        /* ctc mode */
        int id;

        for (id = 0; id < llid_total_chan_num; id++)
        {
            if (llid->channels[id] == BDMF_INDEX_UNASSIGNED)
            {
                llid->channels[id] = llid_get_free_l1_channel();
                if (llid->channels[id] == BDMF_INDEX_UNASSIGNED)
                    BDMF_TRACE_RET(BDMF_ERR_PARM, "No free channels\n");
            }
            /* Mark channel as busy */
            epon_l1_channels[llid->channels[id]] = llid->index;
        }
    }
    return 0;
}

/* Set llid channel */
static int llid_set_channel(llid_drv_priv_t *llid)
{
    return llid_set_channel_ex(llid);
}

/* Set egress_tm(ctrl or data) for llid */
static int llid_egress_tm_set(bdmf_object_handle tm_obj,
    bdmf_object_handle owner, int group_id, int num_channels,
    const int16_t *channels)
{
    int rc = 0;

    switch (g_epon_mode)
    {
    case rdpa_epon_dpoe:
        /* fall through */
    case rdpa_epon_bcm:
        /* fall through */
    case rdpa_epon_cuc_dyn:
        /* fall through */
    case rdpa_epon_ctc_dyn:
        rc = _rdpa_egress_tm_channel_set(tm_obj, owner, rdpa_wan_epon, *channels);
        break;
    case rdpa_epon_ctc:
        /* fall through */
    case rdpa_epon_cuc:
        /* fall through */
    default:
        rc = _rdpa_egress_tm_channel_group_set(tm_obj, owner, rdpa_wan_epon, group_id,
            num_channels, channels);
        break;
    }

    return rc;
}

/* enable control channel */
static int llid_control_enable_set(struct bdmf_object *mo, bdmf_boolean enable)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);

    if (!llid_has_dedicated_data_channel(llid))
        return _rdpa_egress_tm_channel_queue_enable_set(llid->control_egress_tm, llid->channels[0], 0, enable);
    else
        return _rdpa_egress_tm_enable_set(llid->control_egress_tm, enable, 0);
}

/* enable data channels */
static int llid_data_enable_set(struct bdmf_object *mo, bdmf_boolean enable)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    int rc = 0;
    uint8_t loop = 0;
    uint8_t queue_num = 0;

    if (!llid_has_dedicated_data_channel(llid))
    {
        rc = rdpa_egress_tm_num_queues_get(llid->data_egress_tm, &queue_num);
        for (loop = 1; loop < queue_num; loop++)
            rc |= _rdpa_egress_tm_channel_queue_enable_set(llid->data_egress_tm, llid->channels[0], loop, enable);
    }
    else if ((g_epon_mode == rdpa_epon_dpoe || g_epon_mode == rdpa_epon_bcm ||
        g_epon_mode == rdpa_epon_ctc_dyn || g_epon_mode == rdpa_epon_cuc_dyn) &&
             rdpa_is_car_mode())
    {
        rc = _rdpa_egress_tm_channel_enable_set(llid->data_egress_tm, llid->channels[1], enable);
    }
    else
        rc = _rdpa_egress_tm_enable_set(llid->data_egress_tm, enable, 0);

    if (g_epon_mode == rdpa_epon_ctc || g_epon_mode == rdpa_epon_cuc)
        llid_link_tc_to_queue(mo, enable);

    /* rc == NOENT is ok. It means that egress_tm is not assigned to this
       channel */
    return (rc == BDMF_ERR_NOENT) ? BDMF_ERR_OK : rc;
}

static void llid_remove_def_flow(llid_drv_priv_t *llid)
{
    int i;
    int cnt = 0;

    for (i = 0; i < RDPA_EPON_MAX_LLID; i++)
    {
        if (llid_objects[i])
        {
            llid_drv_priv_t *other_llid =
                (llid_drv_priv_t *)bdmf_obj_data(llid_objects[i]);

            if (other_llid->ds_def_flow == llid->ds_def_flow)
                continue;

            if (other_llid->ds_def_flow == llid->ds_def_flow)
                cnt++;
        }
    }

    if (!cnt)
    {
        rdpa_ic_result_delete(llid->ds_def_flow, rdpa_dir_ds);
        classification_ctx_index_put(rdpa_dir_ds, llid->ds_def_flow);
    }

    llid->ds_def_flow = RDPA_UNMATCHED_DS_IC_RESULT_ID;
}

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int llid_pre_init(struct bdmf_object *mo)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    int i;

    if (!rdpa_is_epon_or_xepon_mode())
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_NOT_SUPPORTED, mo, "llid object is only "
            "supported with EPON uplink\n");
    }
    g_epon_mode = _rdpa_epon_mode_get();

    llid->index = BDMF_INDEX_UNASSIGNED;
    llid->control_enable = 1;
    llid->data_enable = 0;
    llid->fec_overhead = 0;
    llid->sci_overhead = 0;
    llid->q_802_1ae = 0;
    llid->num_channels = llid_get_data_channel_num(g_epon_mode);
    llid->ds_def_flow = RDPA_UNMATCHED_DS_IC_RESULT_ID;
    for (i = 0; i <= RDPA_EPON_LLID_QUEUES; i++)
        llid->channels[i] = BDMF_INDEX_UNASSIGNED;

    return 0;
}

static int llid_attr_egress_tm_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size);

/** This optional callback is called at object init time
 * after initial attributes are set.
 * Its work is:
 * - make sure that all necessary attributes are set and make sense
 * - allocate dynamic resources if any
 * - assign object name if not done in pre_init
 * - finalise object creation
 * If function returns error code !=0, object creation is aborted
 */
static int llid_post_init(struct bdmf_object *mo)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    int i, rc = 0;

    /* Assign LLID if not assigned */

    /* find empty index */
    if (llid->index < 0)
    {
        for (i = 0; i < RDPA_EPON_MAX_LLID; i++)
        {
            if (!llid_objects[i])
            {
                llid->index = i;
                break;
            }
        }
    }
    if ((unsigned)llid->index >= RDPA_EPON_MAX_LLID)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many LLIDs or index %ld is out of "
            "range\n", llid->index);
    }
    if (llid_objects[llid->index])
    {
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "LLID %ld already exists\n",
            llid->index);
    }

    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "llid/index=%ld", llid->index);

    /* Verify channel assignment */
    if ((unsigned)llid->num_channels > RDPA_EPON_LLID_QUEUES)
    {
        BDMF_TRACE_RET(BDMF_ERR_PARM, "num_channels must be set in range "
            "0..%d\n", RDPA_EPON_LLID_QUEUES);
    }

    /* Do cleanup in destroy past this point */
    llid_objects[llid->index] = mo;

    llid_total_chan_num = llid_get_total_channel_num(llid);

    /* Assign channels */
    rc = llid_set_channel(llid);
    if (rc)
        BDMF_TRACE_RET_OBJ(rc, mo, "llid_set_channel() fail\n");

    /* Auto-assign egress_tm in CAR mode */
    if (!llid->data_egress_tm && rdpa_is_car_mode())
    {
        bdmf_object_handle wan_port = NULL;
        rdpa_port_get(rdpa_wan_type_to_if(rdpa_wan_epon), &wan_port);
        if (wan_port)
        {
            rdpa_port_tm_cfg_t tm_cfg = {};

            rc = rdpa_port_tm_cfg_get(wan_port, &tm_cfg);
            rc = rc ? rc : rdpa_llid_egress_tm_set(mo, tm_cfg.sched);
            bdmf_put(wan_port);
            if (rc)
                BDMF_TRACE_RET_OBJ(rc, mo, "Can't auto-assign egress_tm\n");
        }
    }

    if (llid->control_egress_tm)
    {
        rc = llid_egress_tm_set(llid->control_egress_tm, mo, llid->index, 1,
            &llid->channels[0]);
        if (llid->control_enable)
        {
            rc = rc ? rc : _rdpa_egress_tm_enable_set(llid->control_egress_tm, 1, 0);
        }
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(rc, mo, "Can't set control channel or enable "
                "control egress_tm\n");
        }
    }
    if (llid->data_egress_tm && llid_has_dedicated_data_channel(llid))
    {
        rc = llid_egress_tm_set(llid->data_egress_tm, mo, llid->index,
            llid->num_channels, &llid->channels[1]);
        if (llid->data_enable)
            rc = rc ? rc : _rdpa_egress_tm_enable_set(llid->data_egress_tm, 1, 0);
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(rc, mo, "Can't set channels or enable data "
                "egress_tm\n");
        }
    }
    /* set data enable differently from control */
    if (llid->data_egress_tm && !llid_has_dedicated_data_channel(llid))
    {
        if (!llid->data_enable &&
            (llid->control_enable != llid->data_enable))
        {
           llid_data_enable_set(mo, 0);
        }
    }

#if defined(BCM_PON_XRDP)
    if (!llid_get_data_channel_num(g_epon_mode))
        RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(llid->index, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, llid->index);
    else
    {
        for (i = 0; i <= llid_get_data_channel_num(g_epon_mode); i++)
        {
            RDD_BBH_QUEUE_DESCRIPTOR_HW_BBH_QID_WRITE_G(i, RDD_US_TM_BBH_QUEUE_TABLE_ADDRESS_ARR, i);
        }
    }
    memset(&(llid->deleted_channels[0]), 0, sizeof(int16_t)*(RDPA_EPON_LLID_QUEUES+1));
#endif

    rc = _cfg_ds_gem_flow_hw(1, llid->index, (uint16_t)RDPA_VALUE_UNASSIGNED, rdpa_flow_dest_eth,
        rdpa_discard_prty_low, llid->ds_def_flow);

    if (rc)
        BDMF_TRACE_RET_OBJ(rc, mo, "Can't create default DS flows\n");

    return BDMF_ERR_OK;
}

void llid_epon_l1_channels_set(uint8_t index, int16_t val)
{
    epon_l1_channels[index] = val;
}

int16_t llid_epon_l1_channels_get(uint8_t index)
{
    return epon_l1_channels[index];
}

static void llid_destroy(struct bdmf_object *mo)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);

    /* Only cleanup if tcont setup passed the point of config in gpon stack */
    if ((unsigned)llid->index >= RDPA_EPON_MAX_LLID ||
        llid_objects[llid->index] != mo)
        return;

    /* Scheduler cleanup */
    if (llid->control_egress_tm)
        llid_egress_tm_set(llid->control_egress_tm, NULL, llid->index, 1,
            llid->channels);
    if (llid->data_egress_tm && llid_has_dedicated_data_channel(llid))
        llid_egress_tm_set(llid->data_egress_tm, NULL, llid->index,
            llid->num_channels, &llid->channels[1]);

    /* Release channels */
    llid_destroy_ex(llid);

    _cfg_ds_gem_flow_hw(0, llid->index, (uint16_t)RDPA_VALUE_UNASSIGNED, rdpa_flow_dest_none,
         rdpa_discard_prty_low, RDPA_UNMATCHED_DS_IC_RESULT_ID);
    if (llid->ds_def_flow != RDPA_UNMATCHED_DS_IC_RESULT_ID)
    {
        rdpa_rdd_default_flow_del(llid);
        llid_remove_def_flow(llid);
    }

    llid_objects[llid->index] = NULL;
}

/*
 * Attribute access functions
 */
static bdmf_error_t rdpa_llid_set_channel_attr(llid_drv_priv_t *llid, bdmf_object_handle tm)
{
    channel_attr attr = {};
    int i, rc = 0;

    attr.fec_overhead = llid->fec_overhead;
    attr.sci_overhead = llid->sci_overhead;
    attr.q_802_1ae = llid->q_802_1ae;

    for (i = 0; i < llid_total_chan_num; ++i)
        rc = rc ? rc : _rdpa_egress_tm_set_channel_attr(tm, &attr, llid->channels[i]);

    return rc;
}

/* "egress_tm" attribute "write" callback */
static int llid_attr_egress_tm_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle egress_tm = *(bdmf_object_handle *)val;
    bdmf_object_handle old_egress_tm = llid->data_egress_tm;
    int rc = 0;

    if (mo->state != bdmf_state_init &&
        egress_tm != llid->data_egress_tm &&
        llid_has_dedicated_data_channel(llid))
    {
        if (llid->data_egress_tm)
        {
            /* Unbind old egress_tm */
            llid_egress_tm_set(llid->data_egress_tm, NULL, llid->index,
                llid->num_channels, &llid->channels[1]);
        }

        if (egress_tm)
        {
            /* check if this llid object is linked to pbit_to_queue table */
            bdmf_link_handle link = NULL;
            while ((link = bdmf_get_next_us_link(mo, link)))
            {
#ifndef XRDP
                if (bdmf_us_link_to_object(link)->drv == rdpa_pbit_to_queue_drv())
                {
                    BDMF_TRACE_RET_OBJ(BDMF_ERR_PERM, mo, "can't change "
                        "egress_tm when llid is linked with pbit_to_queue "
                        "table, first unlink");
                }
#endif
            }
            /* Bind new egress_tm */
            rc = llid_egress_tm_set(egress_tm, mo, llid->index,
                llid->num_channels, &llid->channels[1]);
            if (llid->data_enable)
            {
                rc = rc ? rc : _rdpa_egress_tm_enable_set(egress_tm, 1, 0);
                rc = rc ? rc : rdpa_llid_set_channel_attr(llid, egress_tm);
            }
            if (rc)
            {
                llid_egress_tm_set(egress_tm, NULL, llid->index,
                    llid->num_channels, &llid->channels[1]);
                llid_egress_tm_set(llid->data_egress_tm, mo, llid->index,
                    llid->num_channels, &llid->channels[1]);
                _rdpa_egress_tm_enable_set(llid->data_egress_tm, llid->data_enable, 0);
            }
        }
        if (rc < 0)
            return rc;
    }

    llid->data_egress_tm = egress_tm;

    if (!llid_has_dedicated_data_channel(llid))
    {
        /* rollback and return error*/
        if (egress_tm != llid->control_egress_tm)
        {
            llid->data_egress_tm = llid->control_egress_tm;
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PERM, mo, "can't set different tm from "
                        "control tm when use single channel ");
        }
        else
        {
            /* set data enable differently from control*/
            if (mo->state != bdmf_state_init &&
                !llid->data_enable &&
                (llid->control_enable != llid->data_enable))
            {
                llid_data_enable_set(mo, 0);
            }
        }
    }

    if (!rdpa_is_car_mode())
        replace_ownership(old_egress_tm, egress_tm, mo);

    return 0;
}

/* "control_egress_tm" attribute "write" callback */
static int llid_attr_control_egress_tm_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_object_handle egress_tm = *(bdmf_object_handle *)val;
    int rc = 0;

    if (mo->state != bdmf_state_init && egress_tm != llid->control_egress_tm)
    {
        if (llid->control_egress_tm)
        {
            /* Unbind old egress_tm */
            llid_egress_tm_set(llid->control_egress_tm, NULL, llid->index,
                1, llid->channels);
        }

        if (egress_tm)
        {
            /* Bind new egress_tm */
            rc = llid_egress_tm_set(egress_tm, mo, llid->index, 1,
                llid->channels);
            if (llid->control_enable)
            {
                rc = rc ? rc : _rdpa_egress_tm_enable_set(egress_tm, 1, 0);
                rc = rc ? rc : rdpa_llid_set_channel_attr(llid, egress_tm);
            }
            if (rc)
            {
                llid_egress_tm_set(egress_tm, NULL, llid->index, 1,
                    llid->channels);
                llid_egress_tm_set(llid->control_egress_tm, mo, llid->index, 1,
                    llid->channels);
                _rdpa_egress_tm_enable_set(llid->control_egress_tm, llid->control_enable, 0);
            }
        }
        if (rc < 0)
            return rc;
    }

    llid->control_egress_tm = egress_tm;

    if (!llid_has_dedicated_data_channel(llid))
        llid->data_egress_tm = egress_tm;

    return 0;
}

/* "control_enable" attribute "write" callback */
static int llid_attr_control_enable_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;
    int rc = 0;

    if (mo->state == bdmf_state_active && llid->control_egress_tm &&
        enable != llid->control_enable)
    {
        rc = llid_control_enable_set(mo, enable);
        if (rc < 0)
            return rc;
        if (enable)
        {
            rc = rdpa_llid_set_channel_attr(llid, llid->control_egress_tm);
            if (rc < 0)
                return rc;
        }
    }
    llid->control_enable = enable;

    return 0;
}

/* "data_enable" attribute "write" callback */
static int llid_attr_data_enable_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean enable = *(bdmf_boolean *)val;
    int rc = 0;

    if (mo->state == bdmf_state_active && llid->data_egress_tm &&
        enable != llid->data_enable)
    {
        rc = llid_data_enable_set(mo, enable);
        if (rc < 0)
            return rc;
        if (llid_has_dedicated_data_channel(llid) && (enable))
        {
            rc = rdpa_llid_set_channel_attr(llid, llid->data_egress_tm);
            if (rc < 0)
                return rc;
        }
    }
    llid->data_enable = enable;

    return 0;
}

/* channel attribute "read" callback */
static int llid_attr_channel_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);

    if ((unsigned)index >= llid_total_chan_num)
        return BDMF_ERR_NO_MORE;

    *(int16_t *)val = llid->channels[index];

    return 0;
}

/* "ds_def_flow" attribute "read" callback */
static int llid_attr_ds_def_flow_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    return llid_attr_ds_def_flow_read_ex(mo, ad, index, val, size);
}

static int llid_set_def_flow(llid_drv_priv_t *llid, rdpa_ic_result_t *cfg)
{
    return llid_set_def_flow_ex(llid, cfg);
}

/* "ds_def_flow" attribute "write" callback. */
static int llid_attr_ds_def_flow_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    int rc;

    /* first remove previous cfg if exist */
    if (priv->ds_def_flow != RDPA_UNMATCHED_DS_IC_RESULT_ID)
    {
        rdpa_rdd_default_flow_del(priv);
        llid_remove_def_flow(priv);
    }

    if (cfg == NULL)
    {
        rc = rdpa_rdd_default_flow_del(priv);
        if (rc)
        {
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't remove llid %d ds "
                "default flow configuration, error %d", (int)priv->index, rc);
        }
        return 0;
    }

    return llid_set_def_flow(priv, cfg);
}

/* "port_action" attribute "read" callback */
static int llid_attr_port_action_read(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
#if defined(BCM63158)
    return BDMF_ERR_NOT_SUPPORTED;
#else
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_llid_port_action_t *action = (rdpa_llid_port_action_t *)val;

    if (priv->ds_def_flow == RDPA_UNMATCHED_DS_IC_RESULT_ID)
        return BDMF_ERR_NOENT;

    return port_action_read(priv->ds_def_flow, &action->vlan_action, index,
        &action->drop);
#endif
}

/* "port_action" attribute write callback */
static int llid_attr_port_action_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
#if defined(BCM63158)
    return BDMF_ERR_NOT_SUPPORTED;
#else
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_llid_port_action_t *action = (rdpa_llid_port_action_t *)val;

    if (mo->state != bdmf_state_active)
        return BDMF_ERR_INVALID_OP;

    return port_action_write(priv->ds_def_flow, action->vlan_action, index,
        action->drop);
#endif
}

static int set_llid_attr(struct bdmf_object *mo)
{
#ifdef XRDP
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);
    int err = BDMF_ERR_OK;

    if (priv->data_enable && priv->data_egress_tm && mo->state == bdmf_state_active)
    {
        err = rdpa_llid_set_channel_attr(priv, priv->data_egress_tm);
        if (err)
            return err;
    }
    if (priv->control_enable && priv->control_egress_tm && mo->state == bdmf_state_active)
        err = rdpa_llid_set_channel_attr(priv, priv->control_egress_tm);

    return err;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

static int llid_attr_fec_overhead_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *fec_overhead = (bdmf_boolean *)val;

    priv->fec_overhead = *fec_overhead;
    return set_llid_attr(mo);
}

static int llid_attr_sci_overhead_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *sci = (bdmf_boolean *)val;

    priv->sci_overhead = *sci;
    return set_llid_attr(mo);
}

static int llid_attr_802_1ae_overhead_write(struct bdmf_object *mo,
    struct bdmf_attr *ad, bdmf_index index, const void *val, uint32_t size)
{
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);
    bdmf_boolean *q_802_1ae = (bdmf_boolean *)val;

    priv->q_802_1ae = *q_802_1ae;
    return set_llid_attr(mo);
}

/* "is_empty" attribute "read" callback */
static int llid_attr_attr_is_empty_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rc = 0;
    /*uint32_t i;*/

    *((bdmf_boolean *)val) = 1;
    /* first check control channels */

    if ((llid->control_egress_tm) && (llid_has_dedicated_data_channel(llid)))
        rc = egress_tm_is_empty_on_channel(llid->control_egress_tm, llid->channels[0], ((bdmf_boolean *)val));

    if (rc)
        return rc;
    if (*((bdmf_boolean *)val) == 0)
    {
        return 0;
    }

    /* now check for data channels */

    if (!(llid->data_egress_tm))
        return 1;

    /* RDPA_EGRESS_TM_CHANNEL_IS_GROUP_ID is used because of llid is sitting in hash according to group channel id */
    if (!llid_has_dedicated_data_channel(llid))
    {
        /* queue 0 will be the control and the rest data */
        rc = egress_tm_is_empty_on_channel(llid->data_egress_tm, llid->channels[0], ((bdmf_boolean *)val));
    }
    else
        if (((g_epon_mode == rdpa_epon_ctc) || (g_epon_mode ==  rdpa_epon_cuc)) && !rdpa_is_car_mode())
        {
            rc = egress_tm_is_empty_on_channel(llid->data_egress_tm, RDPA_EGRESS_TM_CHANNEL_IS_GROUP_ID, ((bdmf_boolean *)val));
        }
        else
        {
            rc = egress_tm_is_empty_on_channel(llid->data_egress_tm, llid->channels[1], ((bdmf_boolean *)val));
        }

    BDMF_TRACE_DBG("llid is_empty = %d\n", *((bdmf_boolean *)val));
    return rc;
}

/* llid port_action aggregate type */
struct bdmf_aggr_type llid_port_action_type = {
    .name = "llid_port_action", .struct_name = "rdpa_llid_port_action_t",
    .help = "Llid per port action",
    .fields = (struct bdmf_attr[]) {
        { .name = "vlan_action", .help = "VLAN action object",
            .type = bdmf_attr_object,
            .size = sizeof(bdmf_object_handle), .ts.ref_type_name = "vlan_action",
            .offset = offsetof(rdpa_llid_port_action_t, vlan_action)
        },
        { .name = "drop", .help = "Drop action - true/false",
            .type = bdmf_attr_boolean, .size = sizeof(bdmf_boolean),
            .offset = offsetof(rdpa_llid_port_action_t, drop)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(llid_port_action_type);

/*  rdd_pm_counter_type aggregate type */
struct bdmf_aggr_type llid_stat_type = {
    .name = "llid_stat", .struct_name = "rdpa_gem_stat_t",
    .help = "LLID Statistics",
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
DECLARE_BDMF_AGGREGATE_TYPE(llid_stat_type);

/* Object attribute descriptors */
static struct bdmf_attr llid_attrs[] =
{
    { .name = "index", .help = "LLID index", .size = sizeof(bdmf_index),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY,
        .type = bdmf_attr_number, .offset = offsetof(llid_drv_priv_t, index)
    },
    { .name = "num_channels", .help = "Number of data channels",
        .size = sizeof(int16_t),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_NO_AUTO_GEN,
        .min_val = 1, .max_val = RDPA_EPON_LLID_QUEUES,
        .type = bdmf_attr_number, .offset = offsetof(llid_drv_priv_t,
            num_channels)
    },
    { .name = "egress_tm", .help = "US data scheduler object",
        .size = sizeof(bdmf_object_handle),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_object, .ts.ref_type_name = "egress_tm",
        .offset = offsetof(llid_drv_priv_t, data_egress_tm),
        .write = llid_attr_egress_tm_write
    },
    { .name = "control_egress_tm", .help = "US control scheduler object",
        .size = sizeof(bdmf_object_handle),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_object, .ts.ref_type_name = "egress_tm",
        .offset = offsetof(llid_drv_priv_t, control_egress_tm),
        .write = llid_attr_control_egress_tm_write
    },
    { .name = "control_enable", .help = "Enable LLID control channel",
        .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean,
        .offset = offsetof(llid_drv_priv_t, control_enable),
        .write = llid_attr_control_enable_write
    },
    { .name = "data_enable", .help = "Enable LLID data channels",
        .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean,
        .offset = offsetof(llid_drv_priv_t, data_enable),
        .write = llid_attr_data_enable_write
    },
    { .name = "channels", .help = "L1 channels", .size = sizeof(int16_t),
        .array_size = RDPA_EPON_LLID_QUEUES + 1,
        .min_val = 0, .max_val = RDPA_EPON_TOTAL_L1_QUEUES-1,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_AUTO_GEN,
        .type = bdmf_attr_number, .read = llid_attr_channel_read
    },
    { .name = "ds_def_flow", .help = "downstream default flow configuration",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG |
            BDMF_ATTR_NO_NULLCHECK,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "classification_result",
        .write = llid_attr_ds_def_flow_write, .read = llid_attr_ds_def_flow_read
    },
    { .name = "port_action", .help = "Per port vlan action configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "llid_port_action",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG |
            BDMF_ATTR_NO_RANGE_CHECK,
        .index_ts.enum_table = &rdpa_if_enum_table,
        .array_size = rdpa_if__number_of,
        .index_type = bdmf_attr_enum,
        .read = llid_attr_port_action_read, .write = llid_attr_port_action_write
    },
    { .name = "pm_counters", .help = "PM counters",
        .array_size = RDPA_EPON_LLID_QUEUES + 1,
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "llid_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_CONFIG | BDMF_ATTR_NO_AUTO_GEN,
        .read = llid_attr_counters_read
    },
    { .name = "fec_overhead", .help = "FEC overhead for ghost reporting",
        .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean, .offset = offsetof(llid_drv_priv_t,
            fec_overhead),
        .write = llid_attr_fec_overhead_write
    },
    { .name = "sci_overhead", .help = "sci overhead for ghost reporting",
        .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean, .offset = offsetof(llid_drv_priv_t,
            sci_overhead),
        .write = llid_attr_sci_overhead_write
    },
    { .name = "q_802_1ae", .help = "802.1AE overhead for ghost reporting",
        .size = sizeof(bdmf_boolean),
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .type = bdmf_attr_boolean, .offset = offsetof(llid_drv_priv_t,
            q_802_1ae),
        .write = llid_attr_802_1ae_overhead_write
    },
    { .name = "is_empty", .help = "check if LLID is empty ",
        .flags = BDMF_ATTR_READ,
        .type = bdmf_attr_boolean,
        .read = llid_attr_attr_is_empty_read
    },
    BDMF_ATTR_LAST
};

static int llid_drv_init(struct bdmf_type *drv);
static void llid_drv_exit(struct bdmf_type *drv);

struct bdmf_type llid_drv = {
    .name = "llid",
    .parent = "system",
    .description = "LLID channel bundle",
    .drv_init = llid_drv_init,
    .drv_exit = llid_drv_exit,
    .pre_init = llid_pre_init,
    .post_init = llid_post_init,
    .destroy = llid_destroy,
    .extra_size = sizeof(llid_drv_priv_t),
    .aattr = llid_attrs,
    .max_objs = RDPA_EPON_MAX_LLID,
};
DECLARE_BDMF_TYPE(rdpa_llid, llid_drv);

/*
 * RDPA- interface
 */

/* Init/exit module. Cater for GPL layer */
extern epon_l2_l1_map * (*f_rdpa_llid_l2_l1_map_get)(uint8_t l1_queue);
static int llid_drv_init(struct bdmf_type *drv)
{
    int i;
    for (i = 0; i < RDPA_EPON_TOTAL_L1_QUEUES; i++)
        epon_l1_channels[i] = BDMF_INDEX_UNASSIGNED;

#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_llid_drv = rdpa_llid_drv;
    f_rdpa_llid_get = rdpa_llid_get;
    f_rdpa_llid_l2_l1_map_get = rdpa_llid_l2_l1_map_get;
#endif
    return 0;
}

static void llid_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_llid_drv = NULL;
    f_rdpa_llid_get = NULL;
    f_rdpa_llid_l2_l1_map_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/
int rdpa_llid_get(bdmf_number _index_, bdmf_object_handle *llid_obj)
{
    if ((unsigned)_index_ >= RDPA_EPON_MAX_LLID)
        return BDMF_ERR_PARM;
    if (!llid_objects[_index_] || llid_objects[_index_]->state == bdmf_state_deleted)
        return BDMF_ERR_NODEV;
    bdmf_get(llid_objects[_index_]);
    *llid_obj = llid_objects[_index_];
    return 0;
}

/* The function returns the Channel ID associated with given link and queue id.
   The function can be called in interrupt context.
   Returns 0 if OK or error < 0  */
int rdpa_llid_queue_id_to_channel_id(int llid_id, int queue_id, int *channel_id)
{
    struct bdmf_object *llid_obj;
    llid_drv_priv_t *llid_priv;
    bdmf_number channel_index  = 0;

    if ((unsigned)llid_id >= RDPA_EPON_MAX_LLID)
        return BDMF_ERR_PARM;

    llid_obj = llid_objects[llid_id];
    if (!llid_obj)
    {   /* Could be AE mode */
        *channel_id = (int)channel_index;
        return 0;
    }

    llid_priv = (llid_drv_priv_t *)bdmf_obj_data(llid_obj);

    if (!llid_has_dedicated_data_channel(llid_priv))
    {
        channel_index = llid_priv->channels[0];
    }
    else if (llid_priv->num_channels == 1)
    {
#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
        if (queue_id == RDPA_EPON_CONTROL_QUEUE_ID)
        {
            channel_index = llid_priv->channels[0];
        }
        else
        {
            channel_index = llid_priv->channels[1];
        }
#endif
    }
    else
    {}

    *channel_id = (int)channel_index;

    return 0;
}

