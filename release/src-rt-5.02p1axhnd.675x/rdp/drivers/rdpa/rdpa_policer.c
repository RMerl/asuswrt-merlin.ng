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
 * rdpa_policer.c
 *
 *  Created on: Aug 17, 2012
 *      Author: igort
 */


#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_policer_ex.h"

/** This optional callback is called called at object init time
 *  before initial attributes are set.
 *  If function returns error code !=0, object creation is aborted
 */
static int policer_pre_init(struct bdmf_object *mo)
{
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);

    /* ToDo: Set defaults */
    policer->index = BDMF_INDEX_UNASSIGNED;

    return policer_pre_init_ex(policer);
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
static int policer_post_init(struct bdmf_object *mo)
{
    return policer_post_init_ex(mo);
}

static void policer_destroy(struct bdmf_object *mo)
{
    policer_destroy_ex(mo);
}

/* "cfg" attribute "write" callback */
static int policer_attr_cfg_write(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_tm_policer_cfg_t *cfg = (rdpa_tm_policer_cfg_t *)val;
    int rc;

    if (mo->state == bdmf_state_active)
    {
        rc = policer_rdd_update(mo, cfg);
        if (rc)
            return rc;
    }

    memcpy(&policer->cfg, cfg, sizeof(*cfg));

    return 0;
}

/* "stat" attribute "read" callback */
static int policer_attr_stat_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    rdpa_tm_policer_stat_t *stat = (rdpa_tm_policer_stat_t *)val;
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);

    return policer_attr_stat_read_ex(policer, stat);
}

/* Policer type enum values */
static bdmf_attr_enum_table_t tm_policer_type_enum_table = {
    .type_name = "rdpa_tm_policer_type", .help = "Policer type",
    .values = {
        {"single_bucket", rdpa_tm_policer_single_token_bucket},
        {"dual_bucket_sr", rdpa_tm_policer_sr_overflow_dual_token_bucket},
        {"dual_bucket_tr", rdpa_tm_policer_tr_dual_token_bucket},
        {"dual_bucket_tr_overflow", rdpa_tm_policer_tr_overflow_dual_token_bucket},
        {NULL, 0}
    }
};

static bdmf_attr_enum_table_t policer_factor_bytes_enum_table = {
    .type_name = "policer_factor_bytes", .help = "egress policer factor bytes",
    .values = {
        {"disable", rdpa_policer_factor_bytes_0},
        {"plus_4", rdpa_policer_factor_bytes_4},
        {"plus_8", rdpa_policer_factor_bytes_8},
        {"minus_4", rdpa_policer_factor_bytes_neg_4},
        {"minus_8", rdpa_policer_factor_bytes_neg_8},
        {NULL, 0}
    }
};

/*  tm_policer_cfg aggregate type : policer configuration */
struct bdmf_aggr_type tm_policer_cfg_type = {
    .name = "tm_policer_cfg", .struct_name = "rdpa_tm_policer_cfg_t",
    .help = "Policer Configuration",
    .fields = (struct bdmf_attr[]) {
        { .name = "type", .help = "Policer type",
            .type = bdmf_attr_enum, .ts.enum_table = &tm_policer_type_enum_table,
            .size = sizeof(rdpa_tm_policer_type), .offset = offsetof(rdpa_tm_policer_cfg_t, type)
        },
        { .name = "cr", .help = "Committed rate (bps)", .type = bdmf_attr_number,
            .size = sizeof(policer_rate_size_t), .offset = offsetof(rdpa_tm_policer_cfg_t, commited_rate)
        },
        { .name = "cbs", .help = "Committed burst size (bytes)", .type = bdmf_attr_number,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_tm_policer_cfg_t, committed_burst_size)
        },
        { .name = "pr", .help = "Peak rate (bps)", .type = bdmf_attr_number,
            .size = sizeof(policer_rate_size_t), .offset = offsetof(rdpa_tm_policer_cfg_t, peak_rate)
        },
        { .name = "pbs", .help = "Peak burst size (bytes)", .type = bdmf_attr_number,
            .size = sizeof(uint32_t), .offset = offsetof(rdpa_tm_policer_cfg_t, peak_burst_size)
        },
        { .name = "dei_mode", .help = "dei mode", .type = bdmf_attr_boolean,
             .size = sizeof(bdmf_boolean), .offset = offsetof(rdpa_tm_policer_cfg_t, dei_mode)
        },
        { .name = "factor_bytes", .help = "will be added to packet len before policing",
            .type = bdmf_attr_enum, .ts.enum_table = &policer_factor_bytes_enum_table,
            .size = sizeof(rdpa_policer_factor_bytes), .offset = offsetof(rdpa_tm_policer_cfg_t, factor_bytes)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(tm_policer_cfg_type);

/*  tm_policer_stat aggregate type : policer statistic */
struct bdmf_aggr_type tm_policer_stat_type = {
    .name = "tm_policer_stat", .struct_name = "rdpa_tm_policer_stat_t",
    .help = "Policer Statistics",
    .fields = (struct bdmf_attr[]) {
        { .name = "green", .help = "Green Statistics",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
            .offset = offsetof(rdpa_tm_policer_stat_t, green)
        },
        { .name = "yellow", .help = "Yellow Statistics",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
             .offset = offsetof(rdpa_tm_policer_stat_t, yellow)
        },
        { .name = "red", .help = "Red Statistics",
            .type = bdmf_attr_aggregate, .ts.aggr_type_name = "rdpa_stat",
            .offset = offsetof(rdpa_tm_policer_stat_t, red)
        },
        BDMF_ATTR_LAST
    }
};
DECLARE_BDMF_AGGREGATE_TYPE(tm_policer_stat_type);

/* Object attribute descriptors */
static struct bdmf_attr policer_attrs[] = {
    { .name = "dir", .help = "Traffic Direction",
        .type = bdmf_attr_enum, .ts.enum_table = &rdpa_traffic_dir_enum_table,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG |
            BDMF_ATTR_KEY | BDMF_ATTR_MANDATORY,
        .size = sizeof(rdpa_traffic_dir), .offset = offsetof(policer_drv_priv_t, dir)
    },
    { .name = "index", .help = "Policer Index", .type = bdmf_attr_number,
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE_INIT | BDMF_ATTR_CONFIG | BDMF_ATTR_KEY,
        .size = sizeof(bdmf_index), .offset = offsetof(policer_drv_priv_t, index),
        .max_val = (RDPA_TM_MAX_POLICER - 1), .min_val = 0
    },
    { .name = "cfg", .help = "Configuration",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "tm_policer_cfg",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_CONFIG,
        .offset = offsetof(policer_drv_priv_t, cfg),
        .write = policer_attr_cfg_write
    },
    { .name = "stat", .help = "Statistics",
        .type = bdmf_attr_aggregate, .ts.aggr_type_name = "tm_policer_stat",
        .flags = BDMF_ATTR_READ | BDMF_ATTR_WRITE | BDMF_ATTR_STAT,
        .read = policer_attr_stat_read, .write = policer_attr_stat_write_ex
    },

    BDMF_ATTR_LAST
};

static int policer_drv_init(struct bdmf_type *drv);
static void policer_drv_exit(struct bdmf_type *drv);

struct bdmf_type policer_drv = {
    .name = "policer",
    .parent = "system", 
    .description = "Traffic Policer",
    .drv_init = policer_drv_init,
    .drv_exit = policer_drv_exit,
    .pre_init = policer_pre_init,
    .post_init = policer_post_init,
    .destroy = policer_destroy,
    .extra_size = sizeof(policer_drv_priv_t),
    .aattr = policer_attrs,
    .max_objs = RDPA_TM_MAX_POLICER,
};
DECLARE_BDMF_TYPE(rdpa_policer, policer_drv);

/* Init/exit module. Cater for GPL layer */
static int policer_drv_init(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_policer_drv = rdpa_policer_drv;
    f_rdpa_policer_get = rdpa_policer_get;
#endif
    return 0;
}

static void policer_drv_exit(struct bdmf_type *drv)
{
#ifdef BDMF_DRIVER_GPL_LAYER
    f_rdpa_policer_drv = NULL;
    f_rdpa_policer_get = NULL;
#endif
}

/***************************************************************************
 * Functions declared in auto-generated header
 **************************************************************************/


/** Get policer object by key
 * \param[in] policer_key       Object key
 * \param[out] policer_obj     Object handle
 * \return  0=OK or error <0
 */
int rdpa_policer_get(const rdpa_policer_key_t *_key_, bdmf_object_handle *_obj_)
{
    return rdpa_policer_get_ex(_key_, _obj_);
}

