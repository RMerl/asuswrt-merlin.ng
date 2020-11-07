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

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdd_runner_defs.h"
#include "rdd_tm.h"
#include "rdpa_policer_ex.h"

static struct bdmf_object *us_policer_objects[RDPA_TM_MAX_US_POLICER];
static struct bdmf_object *ds_policer_objects[RDPA_TM_MAX_DS_POLICER];

int policer_pre_init_ex(policer_drv_priv_t *policer)
{
    return BDMF_ERR_OK;
}

int policer_post_init_ex(struct bdmf_object *mo)
{
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);
    struct bdmf_object **policer_objects = (policer->dir == rdpa_dir_ds) ?
        ds_policer_objects : us_policer_objects;
    int max_policers = (policer->dir == rdpa_dir_ds) ?
        RDPA_TM_MAX_DS_POLICER : RDPA_TM_MAX_US_POLICER;
    int rc; 

    /* If policer index is set - make sure it is unique.
     * Otherwise, assign free
     */
    if (policer->index < 0)
    {
        int i;

        /* Find and assign free index */
        for (i = 0; i < max_policers; i++)
        {
            if (!policer_objects[i])
            {
                policer->index = i;
                break;
            }
        }
    }
    if ((unsigned)policer->index >= max_policers)
        BDMF_TRACE_RET(BDMF_ERR_PARM, "Too many policers or index %ld is out of range\n", policer->index);
    /* set object name */
    snprintf(mo->name, sizeof(mo->name), "policer/dir=%s,index=%ld",
        bdmf_attr_get_enum_text_hlp(&rdpa_traffic_dir_enum_table, policer->dir), policer->index);
    if (policer_objects[policer->index])
        BDMF_TRACE_RET(BDMF_ERR_ALREADY, "%s already exists\n", mo->name);

    rc = policer_rdd_update(mo, &policer->cfg);
    if (rc < 0)
        return rc;

    policer_objects[policer->index] = mo;

    return 0;
}

int policer_rdd_update(struct bdmf_object *mo, rdpa_tm_policer_cfg_t *cfg)
{
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);
    int rc;
    rdd_rate_limit_params_t rdd_budget;
    int min_committed_burst_size;
    int max_committed_burst_size;

    max_committed_burst_size = (0xFFFF << POLICER_EXPONENT); 
#if 0
    /* committed_burst_size resolution in bytes/ms - calculation logic taken from SP5 */
    max_committed_burst_size = _rdpa_system_cfg_get()->mtu_size +
        ((RDPA_POLICER_MAX_SR / 8) / (1000000 / POLICER_TIMER_PERIOD));
#endif

    /* cfg->rdpa_tm_policer_type always rdpa_tm_policer_token_bucket */
    if (cfg != NULL)
    {
        if (cfg->commited_rate < RDPA_POLICER_MIN_SR || cfg->commited_rate > RDPA_POLICER_MAX_SR)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Committed rate %d is invalid\n", cfg->commited_rate);

        if (cfg->committed_burst_size == RDPA_VALUE_UNASSIGNED)
            cfg->committed_burst_size = max_committed_burst_size;

        if (cfg->committed_burst_size > max_committed_burst_size)
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo, "Max. committed burst size is %d\n", max_committed_burst_size);
        
        /* committed_burst_size resolution in bytes/ms - calculation logic taken from SP5 */
        min_committed_burst_size = _rdpa_system_cfg_get()->mtu_size +
            ((cfg->commited_rate / 8) / (1000000 / POLICER_TIMER_PERIOD));
        if (cfg->committed_burst_size < min_committed_burst_size)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
                "Committed burst size must be larger than %d\n", min_committed_burst_size);
        }
        rdd_budget.rate = cfg->commited_rate / 8; /* bytes/s */
        rdd_budget.limit = cfg->committed_burst_size;
    }
    else /* rdpa_tm_policer_action_none */
    {
        rdd_budget.rate = RDPA_POLICER_MAX_SR / 8; /* bytes/s */
        rdd_budget.limit = max_committed_burst_size;
    }
    rc = rdd_policer_cfg(policer->dir, policer->index, &rdd_budget);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_PARM, mo,
            "Failed to configure RDD policer %d, error %d\n", (int)policer->index, rc);
    }
    return 0;
}

void policer_destroy_ex(struct bdmf_object *mo)
{
    policer_drv_priv_t *policer = (policer_drv_priv_t *)bdmf_obj_data(mo);
    struct bdmf_object **policer_objects = (policer->dir == rdpa_dir_ds) ?
        ds_policer_objects : us_policer_objects;
    int max_policers = (policer->dir == rdpa_dir_ds) ?
        RDPA_TM_MAX_DS_POLICER : RDPA_TM_MAX_US_POLICER;
    rdd_rate_limit_params_t rdd_budget = { .rate = 0, .limit = 0 };
    int rc;
    
    if ((unsigned)policer->index >= max_policers || policer_objects[policer->index] != mo)
        return;

    rc = rdd_policer_cfg(policer->dir, policer->index, &rdd_budget);
    if (rc)
        bdmf_trace("Failed to remove policer in RDD, index %d, error %d\n", (int)policer->index, rc);

    policer_objects[policer->index] = NULL;
}

int policer_attr_stat_read_ex(policer_drv_priv_t *policer, rdpa_tm_policer_stat_t *stat)
{
    int rc = 0;
    uint16_t red_packets = 0;

    /* Calling RDD function for mac_unknown_da_forwarding_drop */
    rc = rdd_policer_drop_counter_get(policer->dir, policer->index, &red_packets);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot read policer drop statistics from RDD, rc=%d\n", rc);

    memset(stat, 0, sizeof(rdpa_tm_policer_stat_t));
    stat->red.packets = red_packets;

    return rc;
}

int policer_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_policer_get_ex(const rdpa_policer_key_t *_key_, bdmf_object_handle *_obj_)
{
    struct bdmf_object **policer_objects = (_key_->dir == rdpa_dir_ds) ?
        ds_policer_objects : us_policer_objects;
    int max_policers = (_key_->dir == rdpa_dir_ds) ?
        RDPA_TM_MAX_DS_POLICER : RDPA_TM_MAX_US_POLICER;

    return rdpa_obj_get(policer_objects, max_policers, _key_->index, _obj_);
}
