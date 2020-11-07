/*
* <:copyright-BRCM:2017:proprietary:standard
* 
*    Copyright (c) 2017 Broadcom 
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

#include "tmctl_bdmf_rdpa.h"
#include "rdpactl_api.h"
#if defined(SUPPORT_FLUSH)
#include "eponctl_api.h"
#endif
#include "tmctl_api_runner.h"
#if defined(SUPPORT_DPI)
#include <bcmdpi.h>
#endif

typedef struct
{
    rdpa_drv_ioctl_dev_type rdpa_dev;
    tmctl_devType_e dev_type;
    unsigned int if_id;
    rdpa_traffic_dir dir;
    rdpa_tm_level_type tm_level;
    rdpa_tm_sched_mode tm_mode;
    int sched_caps;
    uint32_t sched_type;
    int num_queues;
    BOOL set_dual_rate;
    int subsidiary_idx;
    int weight;
} set_parm_t;

typedef struct
{
    bdmf_object_handle root_tm;
    rdpa_tm_sched_mode mode;
    rdpa_tm_level_type level;
    uint8_t max_queues;
} get_root_parm_t;

#define SP_SUBSIDIARY_IDX 0
#define WRR_SUBSIDIARY_IDX 1
#define STATIC_CTC_NUM_Q 8 
#define STATIC_CTC_SHARED_TM_IDX 6
#define TOTAL_DSCP_NUM 64


static inline int get_wan_type(tmctl_devType_e dev_type, unsigned int if_id, rdpa_wan_type *wan_type, BOOL *is_ae_mode)
{
    bdmf_object_handle wan_port = BDMF_NULL;
    rdpa_port_dp_cfg_t cfg = {0};
    int ret = 0;

    *is_ae_mode = FALSE;

    switch (dev_type)
    { 
    case  TMCTL_DEV_GPON:
        // For GPON, If_id corresponds to TCONT id. 
        // So, hardcode wan type to GPON
        if_id = rdpa_wan_type_to_if(rdpa_wan_gpon);
        break;
    case TMCTL_DEV_EPON:
        // For EPON, If_id corresponds to LLID. 
        // So, hardcode wan type to EPON
        if_id = rdpa_wan_type_to_if(rdpa_wan_epon);
        break;
    } 

    ret = rdpa_port_get(if_id, &wan_port);
    if (ret)
    {
        tmctl_error("rdpa_port_get failed, ret[%d]" , ret);
        goto Exit;
    }

    ret = rdpa_port_wan_type_get(wan_port, wan_type);
    if (ret)
    {
        tmctl_error("rdpa_port_wan_type_get failed, ret[%d]" , ret);
        goto Exit;
    }

    if (*wan_type == rdpa_wan_epon || *wan_type == rdpa_wan_xepon)
    {
        ret = rdpa_port_cfg_get(wan_port, &cfg);
        *is_ae_mode = cfg.ae_enable;
    }

Exit:
    if (wan_port)
        bdmf_put(wan_port);

    return ret;
}

static rdpa_epon_mode get_epon_mode(void)
{
    static rdpa_epon_mode epon_mode = rdpa_epon_none;
    int ret;

    if (epon_mode == rdpa_epon_none)
    {
        ret = rdpaCtl_get_epon_mode(&epon_mode);
        if (ret)
        {
            tmctl_error("rdpaCtl_get_epon_mode failed, ret[%d]", ret);
            return TMCTL_ERROR;
        }
    }

    return epon_mode;
}

static BOOL is_static_ctc_mode(tmctl_devType_e dev_type, rdpa_traffic_dir dir)
{
    if (((dev_type == TMCTL_DEV_EPON) || 
       (dev_type == TMCTL_DEV_ETH && dir == rdpa_dir_us)) &&
       (get_epon_mode() == rdpa_epon_ctc))
    {
        return 1;
    }

    return 0;
}

static void get_port_tm_caps(int dir, tmctl_devType_e dev_type, unsigned int if_id, tmctl_portTmParms_t *tm)
{
    rdpa_wan_type wan_type;
    BOOL is_ae_mode;
    int ret;

    tm->queueShaper = TRUE;
    tm->dualRate = FALSE;

    if (dir == rdpa_dir_ds) 
    {
        tm->maxQueues = MAX_Q_PER_LAN_TM;
        tm->maxSpQueues = MAX_Q_PER_LAN_TM;
        tm->schedCaps = TMCTL_SP_CAPABLE;
#if defined(BCM_PON_XRDP) || defined(BCM_DSL_XRDP)
        tm->schedCaps |= TMCTL_WRR_CAPABLE | TMCTL_SP_WRR_CAPABLE | TMCTL_1LEVEL_CAPABLE;
#else
        tm->queueShaper = FALSE;
#endif /* BCM_PON_XRDP || BCM_DSL_XRDP */
        tm->portShaper = TRUE;
        if (is_switch_intf(dev_type, if_id)) /* Runner port & queue shapers are not used for switch intf */
        {
            tm->queueShaper = FALSE;
            tm->portShaper = FALSE;
        }
    }
    else
    {
        if (dev_type == TMCTL_DEV_EPON)
            tm->portShaper = TRUE;
        else
        { 
            ret = get_wan_type(dev_type, if_id, &wan_type, &is_ae_mode);
            if (ret) /* function mustn't fail so portShaper will be set to FALSE */
            {
                tm->portShaper = FALSE;
            }
            else
            {
                if (wan_type == rdpa_wan_gpon || wan_type == rdpa_wan_xgpon)
                    tm->portShaper = FALSE;
                else
                    tm->portShaper = dev_type == TMCTL_DEV_ETH ? TRUE : FALSE;
            }
        }

        if (is_static_ctc_mode(dev_type, dir))
        {
            tm->maxQueues = MAX_TMCTL_QUEUES_BASELINE;
            tm->maxSpQueues = MAX_TMCTL_QUEUES_BASELINE;
            tm->schedCaps = RDPA_TM_SP_CAPABLE; 
            tm->queueShaper = FALSE;
        }
        else
        {
            tm->maxQueues = MAX_Q_PER_WAN_TM;
            tm->maxSpQueues = MAX_Q_PER_WAN_TM;
            tm->schedCaps = RDPA_TM_SP_CAPABLE | RDPA_TM_WRR_CAPABLE | RDPA_TM_SP_WRR_CAPABLE;
#if defined(BCM_PON_XRDP) || defined(BCM_DSL_XRDP)
            tm->schedCaps |= RDPA_TM_1LEVEL_CAPABLE;
#endif  /* BCM_PON_XRDP || BCM_DSL_XRDP */
            tm->dualRate = TRUE;
        }
    }

    tmctl_debug("dir[%d], maxQueues[%d], maxSpQueues[%d], portShaper[%d], queueShaper[%d], schedCaps[%d], dualRate[%d]", 
                dir, tm->maxQueues, tm->maxSpQueues, tm->portShaper, tm->queueShaper, tm->schedCaps, tm->dualRate );
}

static int get_tm_owner(tmctl_devType_e dev_type, unsigned int if_id, bdmf_object_handle *owner)
{
    int ret = TMCTL_ERROR;

    tmctl_debug("Devtype[%d], if_id[%d]", dev_type, if_id);

    switch (dev_type)
    {
        case TMCTL_DEV_ETH:
            ret = rdpa_port_get(if_id, owner);
            break;
#if defined(BCM_PON) || defined(CHIP_63158)
        case TMCTL_DEV_GPON:
            /* For T-CONT device type, index 0 is reserved for Default ALLOC Id so we
            need to increment by 1 the index of the TCONT that comes from OMCI userspace application. */
            ret = rdpa_tcont_get(++if_id, owner);
            break;
        case TMCTL_DEV_EPON:
            ret = rdpa_llid_get(if_id, owner);
            break;
#endif
        case TMCTL_DEV_SVCQ:
        default:
            tmctl_error("Device %d not supported", dev_type);
            break;
    }
    
    return ret;
}

static void ignore_negtiv_shaper(tmctl_shaper_t *shaper)
{
    if (shaper->minRate < 0)
        shaper->minRate = 0;
    if (shaper->shapingRate < 0)
        shaper->shapingRate = 0;
    if (shaper->shapingBurstSize < 0)
        shaper->shapingBurstSize = 0;
}

static int set_queue_rl_multi_level(bdmf_object_handle root_tm, bdmf_object_handle queue_tm, tmctl_shaper_t *shaper)
{
    rdpa_tm_rl_rate_mode rl;
    rdpa_tm_rl_cfg_t rl_cfg = {0};
    int ret;
  
    ignore_negtiv_shaper(shaper);
    ret = rdpa_egress_tm_rl_rate_mode_get(root_tm, &rl);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_rl_rate_mode_get failed, ret[%d]" , ret);
        return TMCTL_ERROR;
    }

    if (rl == rdpa_tm_rl_dual_rate)
    {
        if (shaper->minRate)
            rl_cfg.af_rate = KBPS_TO_BPS(shaper->minRate);
        else /* for dual rate mode, AF rate must be greater than 0 */
            rl_cfg.af_rate = 1;
        if (shaper->shapingRate) 
            rl_cfg.be_rate = KBPS_TO_BPS(shaper->shapingRate - shaper->minRate);
        else /* for dual rate mode, BE must be set to max for no max rate shaping. */
            rl_cfg.be_rate = 1000000000L;
    }
    else
        rl_cfg.af_rate = KBPS_TO_BPS(shaper->shapingRate);

    if (shaper->shapingBurstSize < rl_cfg.be_rate)
        rl_cfg.burst_size = 12500000L;
    else
        rl_cfg.burst_size = shaper->shapingBurstSize;

    ret = rdpa_egress_tm_rl_set(queue_tm, &rl_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_rl_set failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}


static int setup_tm(set_parm_t *set_parm, bdmf_object_handle owner, bdmf_object_handle *egress_tm)
{
    bdmf_mattr_handle mattr = BDMF_NULL;
    int ret;

    tmctl_debug("Dir[%d], tm_level[%d], tm_mode[%d]", 
                set_parm->dir, set_parm->tm_level, set_parm->tm_mode);
    
    *egress_tm = BDMF_NULL;

    mattr = bdmf_mattr_alloc(rdpa_egress_tm_drv());
    if (!mattr)
    {
        tmctl_error("bdmf_mattr_alloc failed");
        goto error;
    }

    ret = rdpa_egress_tm_dir_set(mattr, set_parm->dir);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_dir_set failed, ret[%d]", ret);
        goto error;
    }

    ret = rdpa_egress_tm_level_set(mattr, set_parm->tm_level);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_level_set failed, ret[%d]", ret);
        goto error;
    }

    ret = rdpa_egress_tm_mode_set(mattr, set_parm->tm_mode);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_mode_set failed, ret[%d]", ret);
        goto error;
    }

    if (IS_SINGLE_LEVEL(set_parm->tm_level) && (set_parm->tm_mode != rdpa_tm_sched_disabled))
    {
        ret = rdpa_egress_tm_num_queues_set(mattr, set_parm->num_queues);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_num_queues_set failed. ret[%d]", ret);
            goto error;
        }
    }

    if (set_parm->set_dual_rate)
    {
        ret = rdpa_egress_tm_rl_rate_mode_set(mattr, rdpa_tm_rl_dual_rate);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_rl_rate_mode_set failed, ret[%d]", ret);
            goto error;
        }
    }
    
    ret = bdmf_new_and_set(rdpa_egress_tm_drv(), owner, mattr, egress_tm);
    mattr = BDMF_NULL;
    if (ret)
    {
        tmctl_error("bdmf_new_and_set failed to create egress_tm obj, ret[%d]", ret);
        goto error;
    }
    if ((set_parm->dev_type == TMCTL_DEV_EPON) && (set_parm->subsidiary_idx == BDMF_INDEX_UNASSIGNED)) 
    {
        ret = rdpa_llid_egress_tm_set(owner, *egress_tm);
        if (ret)
        {
            tmctl_error("rdpa_llid_egress_tm_set failed, ret[%d]", ret);
            goto error;
        }
    }
    if (set_parm->subsidiary_idx != BDMF_INDEX_UNASSIGNED) /* subsidiary egress_tm */
    {
        if (set_parm->weight)
        {
            ret = rdpa_egress_tm_weight_set(*egress_tm, set_parm->weight);
            if (ret)
            {
                tmctl_error("rdpa_egress_tm_weight_set failed, ret[%d]", ret);
                goto error;
            }
        }
        ret = rdpa_egress_tm_subsidiary_set(owner, set_parm->subsidiary_idx, *egress_tm);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_subsidiary_set failed, ret[%d]", ret);
            goto error;
        }
    }
    return TMCTL_SUCCESS;
error:
    if (mattr)
        bdmf_mattr_free(mattr);
    if (*egress_tm)
        bdmf_destroy(*egress_tm);
    return TMCTL_ERROR;
}

static int get_root_tm(tmctl_devType_e dev_type, unsigned int if_id, bdmf_object_handle *egress_tm)
{
    bdmf_object_handle temp_tm = BDMF_NULL;
    bdmf_object_handle owner = BDMF_NULL;
    int ret = TMCTL_ERROR;

    *egress_tm = BDMF_NULL;

    ret = get_tm_owner(dev_type, if_id, &owner);
    if (ret)
    {
        tmctl_error("Failed to get tm owner, ret[%d]", ret);
        return TMCTL_ERROR;
    }
 
    switch (dev_type)
    {
        case TMCTL_DEV_ETH:
            {
                rdpa_port_tm_cfg_t tm_cfg = {0};
                ret = rdpa_port_tm_cfg_get(owner, &tm_cfg);
                if (ret)
                {
                    tmctl_error("Failed to get tm_cfg, if_id[%d], ret[%d]", if_id, ret);
                    goto exit;
                }
                temp_tm = tm_cfg.sched;
            }
            break;
        case TMCTL_DEV_GPON:
            ret = rdpa_tcont_egress_tm_get(owner, &temp_tm);
            if (ret)
            {
                tmctl_error("Failed to get egress_tm, if_id[%d], ret[%d]", if_id, ret);
                goto exit;
            }
            break;
        case TMCTL_DEV_EPON:
            ret = rdpa_llid_egress_tm_get(owner, &temp_tm);
            if (ret)
            {
                tmctl_error("Failed to get egress_tm, if_id[%d], ret[%d]", if_id, ret);
                goto exit;
            }
            break;
        case TMCTL_DEV_SVCQ:
        default:
            tmctl_error("Device %d not supported", dev_type);
            goto exit;
    }

    *egress_tm = temp_tm;
    ret = TMCTL_SUCCESS;
exit:
    bdmf_put(owner);
    return ret;
}

static void print_sched_caps(uint32_t sched_caps)
{
    printf("Sched caps are:\n");

    if (sched_caps & TMCTL_SP_WRR_CAPABLE)              
        printf("SP and WRR [0]\n"); 
    if (sched_caps & TMCTL_SP_CAPABLE) 
        printf("SP [256]\n");
    if (sched_caps & TMCTL_WRR_CAPABLE)              
        printf("WRR [512]\n");
}

static inline int get_shared_mode(tmctl_devType_e dev_type, BOOL *is_shared)
{
    rdpa_epon_mode epon_mode = rdpa_epon_none;

#ifdef BCM_XRDP /* in RDP egress tm is never shared */
    if (dev_type != TMCTL_DEV_EPON)
#endif
    {
        *is_shared = 0;
        return TMCTL_SUCCESS;
    }

    epon_mode = get_epon_mode();
    if (epon_mode == rdpa_epon_none)
    {
        tmctl_error("get_epon_mode failed");
        return TMCTL_ERROR;
    }

    *is_shared = epon_mode == rdpa_epon_dpoe;
    return TMCTL_SUCCESS;
}

static int setup_default_queues(rdpa_drv_ioctl_dev_type rdpa_dev, 
                                 set_parm_t *root_parm, 
                                 int cfg_flags,
                                 tmctl_devType_e dev_type)
{
    tmctl_queueCfg_t qcfg_p = {0};
    int divergence = 0;
    int ret;
    BOOL is_shared;

    ret = get_shared_mode(dev_type, &is_shared);
    if (ret)
    {
        tmctl_error("get_shared_mode failed");
        return TMCTL_ERROR;
    }

    qcfg_p.qsize = getDefaultQueueSize(rdpa_dev, root_parm->dir);
    if (root_parm->tm_mode == rdpa_tm_sched_wrr)
    {
        qcfg_p.weight = 1;
        qcfg_p.schedMode = TMCTL_SCHED_WRR;
    }
    else
    {
        qcfg_p.weight = 0;
        qcfg_p.schedMode = TMCTL_SCHED_SP;
    }    
  
    if (is_shared)
        root_parm->num_queues = 8;
    /* move queues to the top of the array if num queues is lower than max possbile queues */
    if (root_parm->tm_level == rdpa_tm_level_egress_tm || is_shared)
        divergence = MAX_Q_PER_WAN_TM - root_parm->num_queues;
    
    /* Initialize the default queues */
    for (qcfg_p.qid = 0; qcfg_p.qid < root_parm->num_queues; qcfg_p.qid++)
    {
#ifdef BCM_DSL_XRDP
        /* DSL platform will always do Q7P7 */
        qcfg_p.priority = divergence + qcfg_p.qid;
#else
        if (cfg_flags & TMCTL_QIDPRIO_MAP_Q7P7)
            qcfg_p.priority = divergence + qcfg_p.qid;
        else
            qcfg_p.priority = divergence + root_parm->num_queues - (uint32)qcfg_p.qid - 1;
#endif

        ret = tmctl_RdpaTmQueueSet(rdpa_dev, root_parm->dev_type, root_parm->if_id, &qcfg_p);
        if (ret)
            return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

#if defined(SUPPORT_DPI)
static int configure_us_dpi_queues(bdmf_object_handle egress_tm, tmctl_queueCfg_t *parent_queue)
{
    int ret = 0;
    int idx;

    /* add queues under service queue scheduler */
    for (idx = 0; idx < EG_PRIO_MAX + 1; idx++)
    {
        rdpa_tm_queue_cfg_t queue_cfg = {
            .queue_id       = idx + DPI_XRDP_US_SQ_OFFSET,
            .drop_alg       = rdpa_tm_drop_alg_dt,
            .drop_threshold = getDeviceQueueSize(parent_queue->qsize),
            .stat_enable    = 1,
            .reserved_packet_buffers = parent_queue->minBufs,
        };

        ret = rdpa_egress_tm_queue_cfg_set(egress_tm, idx, &queue_cfg);

        if (ret)
            break;
    }

    return ret;
}

static int add_us_dpi_queues(bdmf_object_handle owner, tmctl_queueCfg_t *parent_queue)
{
    rdpa_tm_service_queue_t service_queue = {.enable = 1};
    bdmf_object_handle egress_tm;
    bdmf_mattr_handle mattr;

    if (!rdpa_egress_tm_subsidiary_get(owner, 0, &egress_tm))
        return configure_us_dpi_queues(egress_tm, parent_queue);

    mattr = bdmf_mattr_alloc(rdpa_egress_tm_drv());
    if (!mattr)
    {
        tmctl_error("bdmf_mattr_alloc failed");
        goto err;
    }

#define run_or_err(expr, errlabel) {                  \
    int _ret = expr;                                  \
    if (_ret)                                         \
    {                                                 \
        tmctl_error(#expr " failed, ret %d\n", _ret); \
        goto errlabel;                                \
    } }
    /* create queues */
    run_or_err(rdpa_egress_tm_dir_set(mattr, rdpa_dir_us), err_free_mattr);
    run_or_err(rdpa_egress_tm_level_set(mattr, rdpa_tm_level_queue), err_free_mattr);
#if defined(BCM_XRDP)
    run_or_err(rdpa_egress_tm_mode_set(mattr, rdpa_tm_sched_sp_wrr), err_free_mattr);
#else
    run_or_err(rdpa_egress_tm_mode_set(mattr, rdpa_tm_sched_sp), err_free_mattr);
#endif
    run_or_err(rdpa_egress_tm_service_queue_set(mattr, &service_queue), err_free_mattr);
#if defined(BCM_XRDP)
    run_or_err(rdpa_egress_tm_overall_rl_set(mattr, FALSE), err_free_mattr);
    run_or_err(rdpa_egress_tm_num_queues_set(mattr, 8), err_free_mattr);
    run_or_err(rdpa_egress_tm_num_sp_elements_set(mattr, 8), err_free_mattr);
    run_or_err(rdpa_egress_tm_rl_rate_mode_set(mattr, rdpa_tm_rl_dual_rate), err_free_mattr);
#endif
    run_or_err(bdmf_new_and_set(rdpa_egress_tm_drv(), owner, mattr, &egress_tm), err_free_mattr);

    run_or_err(rdpa_egress_tm_subsidiary_set(owner, 0, egress_tm), err_free_sched);
    run_or_err(configure_us_dpi_queues(egress_tm, parent_queue), err_free_sched);

    return TMCTL_SUCCESS;

err_free_sched:
    bdmf_destroy(egress_tm);
err_free_mattr:
    bdmf_mattr_free(mattr);
err:
    return TMCTL_ERROR;
}

#if !defined(BCM_XRDP)
int rdp_add_us_dpi_queues(int parent_tm_id, tmctl_queueCfg_t *parent_queue)
{
      bdmf_object_handle egress_tm = BDMF_NULL;
      rdpa_egress_tm_key_t tm_key = {
          .dir   = rdpa_dir_us,
          .index = parent_tm_id,
      };
      rdpa_tm_queue_location_t location;

      run_or_err(rdpa_egress_tm_get(&tm_key, &egress_tm), err);
      run_or_err(rdpa_egress_tm_queue_location_get(egress_tm, parent_queue->qid, &location), err);
      run_or_err(add_us_dpi_queues(location.queue_tm, parent_queue), err);

      return TMCTL_SUCCESS;

err:
      return TMCTL_ERROR;
}
#endif /* !defined(BCM_XRDP) */
#endif /* defined(SUPPORT_DPI) */

static int check_caps(uint32_t sched_type, uint32_t sched_caps)
{
    if ((sched_type == TMCTL_SCHED_TYPE_SP    && !(sched_caps & TMCTL_SP_CAPABLE)) ||
       (sched_type == TMCTL_SCHED_TYPE_WRR    && !(sched_caps & TMCTL_WRR_CAPABLE)) ||
       (sched_type == TMCTL_SCHED_TYPE_SP_WRR && !(sched_caps & TMCTL_SP_WRR_CAPABLE)))
    {
       tmctl_error("Configured scheduler type %d is not supported", sched_type);
       print_sched_caps(sched_caps);
       return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static int check_cfg(tmctl_portTmParms_t *tm ,uint32_t sched_type, int num_queues, BOOL set_dual_rate)
{
    if (check_caps(sched_type, tm->schedCaps))
       return TMCTL_ERROR;

    if (num_queues > tm->maxQueues)
    {
        tmctl_error("Num queues wanted[%d] is higher than max queues allowed[%d]", num_queues, tm->maxQueues);
        return TMCTL_ERROR;
    }

    if (num_queues > 0 && num_queues != 8 && num_queues != 16 && num_queues != 32)
    {
        tmctl_error("Num queues wanted[%d] is not allowed[8, 16, 32]", num_queues);
        return TMCTL_ERROR;
    }

    if (set_dual_rate && !tm->dualRate)
    {
        tmctl_error("Dual rate is not supported");
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static int get_default_num_queues(rdpa_traffic_dir dir, uint32_t sched_type)
{
#if !defined(CHIP_6846) && !defined(BCM_RDP) 
    if (dir == rdpa_dir_us && sched_type == TMCTL_SCHED_TYPE_SP_WRR)
        return MAX_TMCTL_QUEUES_EXTENDED;
#endif
    return MAX_TMCTL_QUEUES_BASELINE;
}

static int setup_subsidiary_tm(bdmf_object_handle egress_tm)
{
    set_parm_t set_parm = {0};
    bdmf_object_handle sp_kid = BDMF_NULL;
    bdmf_object_handle wrr_kid = BDMF_NULL;
    int ret;

    set_parm.dir = rdpa_dir_us;
    set_parm.tm_level = rdpa_tm_level_egress_tm;
    set_parm.tm_mode = rdpa_tm_sched_sp;
    set_parm.subsidiary_idx = SP_SUBSIDIARY_IDX;    

    ret = setup_tm(&set_parm, egress_tm, &sp_kid);
    if (ret)
    {
        tmctl_error("setup_tm failed to create SP kid TM");
        goto error;
    }

    set_parm.tm_mode = rdpa_tm_sched_wrr;
    set_parm.subsidiary_idx = WRR_SUBSIDIARY_IDX;
    ret = setup_tm(&set_parm, egress_tm, &wrr_kid);
    if (ret)
    {
        tmctl_error("setup_tm failed to create WRR kid TM");
        goto error;
    }
    
    return TMCTL_SUCCESS;
error:
    if (sp_kid)
        bdmf_destroy(sp_kid);
    if (wrr_kid)
        bdmf_destroy(wrr_kid);
    return TMCTL_ERROR;
}

/* remove all queues except the control queue */
static int reset_shared_root(bdmf_object_handle root_tm)
{
    int ret;
    int i;
    uint8_t num_queues;

    ret = rdpa_egress_tm_num_queues_get(root_tm, &num_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_queues_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }
    
    for (i = 1; i < num_queues; ++i)
    {
        ret = rdpa_egress_tm_queue_cfg_delete(root_tm, i);
        if (ret && ret != BDMF_ERR_NOENT)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_delete failed, ret[%d]", ret);
            return TMCTL_ERROR;
        }
    }

    return TMCTL_SUCCESS;
}

static int setup_root_tm(set_parm_t *root_parm, bdmf_object_handle *root_tm, tmctl_devType_e dev_type)
{
    bdmf_object_handle owner = BDMF_NULL;
    BOOL is_shared;
    int ret;

    ret = get_shared_mode(dev_type, &is_shared);
    if (ret)
    {
        tmctl_error("get_shared_mode failed");
        return TMCTL_ERROR;
    }    

    if (*root_tm && is_shared)
    {
        ret = reset_shared_root(*root_tm);
        if (ret)
        {
            tmctl_error("Faile to reset root");
            goto error;
        }
        return TMCTL_SUCCESS;
    }
    else if (*root_tm)
    {
        tmctl_debug("Destroying old Tm object, if_id[%d]", root_parm->if_id);
        ret = bdmf_destroy(*root_tm);
        if (ret)
        {
            tmctl_error("bdmf_destroy failed, ret[%d]", ret);
            return TMCTL_ERROR;
        }
        *root_tm = BDMF_NULL;
    }
    
    root_parm->subsidiary_idx = BDMF_INDEX_UNASSIGNED;

    if (root_parm->sched_caps & RDPA_TM_1LEVEL_CAPABLE || (root_parm->dir == rdpa_dir_ds))
        root_parm->tm_level = rdpa_tm_level_queue;
    else
        root_parm->tm_level = rdpa_tm_level_egress_tm;
    
    switch (root_parm->sched_type)
    {
        case TMCTL_SCHED_TYPE_SP_WRR:
            root_parm->tm_mode = IS_SINGLE_LEVEL(root_parm->tm_level) ? rdpa_tm_sched_sp_wrr : rdpa_tm_sched_sp ;
            break;
        case TMCTL_SCHED_TYPE_SP:
            root_parm->tm_mode = rdpa_tm_sched_sp;
            break;
        case TMCTL_SCHED_TYPE_WRR:
            root_parm->tm_mode = rdpa_tm_sched_wrr;
            break;
        default:
            tmctl_error("Port sched type 0x%x is not supported, if_id[%d]", root_parm->sched_type, root_parm->if_id);
            goto error;
    }
    root_parm->num_queues = (root_parm->num_queues > 0) ? root_parm->num_queues : 
                       get_default_num_queues(root_parm->dir, root_parm->sched_type);
    ret = get_tm_owner(root_parm->dev_type, root_parm->if_id, &owner);
    if (ret)
    {
        tmctl_error("Failed to get tm owner, if_id[%d], ret[%d]", root_parm->if_id, ret);
        goto error;
    }

    ret = setup_tm(root_parm, owner, root_tm);
    bdmf_put(owner);
    if (ret)
    {
        tmctl_error("Failed to setup new tm, if_id[%d]", root_parm->if_id);
        goto error;
    }

    /* generate multi level root for SP+WRR */
    if (!IS_SINGLE_LEVEL(root_parm->tm_level) && 
        (root_parm->sched_type == TMCTL_SCHED_TYPE_SP_WRR))
    {
        ret = setup_subsidiary_tm(*root_tm);
        if (ret)
        {
            tmctl_error("setup_subsidiary_tm failed, if_id[%d]", root_parm->if_id);
            goto error;
        }
    }

    return TMCTL_SUCCESS;
error:
    if (*root_tm)
        bdmf_destroy(*root_tm);
    return TMCTL_ERROR;
}

/*
Currently only epon sfu and hgu support.
*/
static BOOL is_flush_required(rdpa_drv_ioctl_dev_type rdpa_dev, tmctl_devType_e dev_type, unsigned int dev_id)
{
#if defined(SUPPORT_FLUSH)
    rdpa_traffic_dir dir;
    rdpa_wan_type wan_type;
    BOOL is_ae_mode;
    int ret;

    tmctl_debug("dev_type %d:%d, dev_id %d " , rdpa_dev, dev_type, dev_id);
   
    switch (rdpa_dev)
    {
    case RDPA_IOCTL_DEV_LLID:
        return TRUE;

    case RDPA_IOCTL_DEV_PORT:
        dir = getDir(rdpa_dev, dev_id);
        if (dir == rdpa_dir_us)
        {
            ret = get_wan_type(dev_type, dev_id, &wan_type, &is_ae_mode);
            if (ret)
                return FALSE;
            if (((wan_type == rdpa_wan_epon)||(wan_type == rdpa_wan_xepon))
             && (!is_ae_mode))
                return TRUE;
        }
				
    default:
        return FALSE;
    }
#endif
    return FALSE;	
}

#if defined(SUPPORT_FLUSH)
static inline int get_impacted_devices(tmctl_devType_e dev_type, uint32_t dev_id)
{
    /* SFU and DPoE HGU*/
    if (dev_type == TMCTL_DEV_EPON)
    {
        return dev_id;
    }
    /* CTC HGU */
    if (dev_type == TMCTL_DEV_ETH)  
    {
        return 0;
    }
    return 0;
}
#endif

static tmctl_ret_e enable_device_traffic(tmctl_devType_e dev_type, unsigned int dev_id, BOOL enable)
{
    int ret = TMCTL_SUCCESS;

#if defined(SUPPORT_FLUSH)
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_object_handle llid_obj = BDMF_NULL;
    uint8_t llid;

    tmctl_debug("%s traffic of llid %d\n\r", (enable?"enable":"disable"), dev_id);
 
    llid = get_impacted_devices(dev_type, dev_id);
    rc = rdpa_llid_get(llid, &llid_obj);
    if (rc)
    {
        ret = TMCTL_ERROR;
        tmctl_error("rdpa_llid_get failed: llid(%u) rc(%d)", llid, rc);
        goto ENABLE_TRAFFIC_EXIT;
    }
        
    rc = rdpa_llid_data_enable_set(llid_obj, enable);
    if (rc)
    {
        ret = TMCTL_ERROR;
        tmctl_error("rdpa_llid_data_enable_set failed: llid(%u) rc(%d)", llid, rc);
        goto ENABLE_TRAFFIC_EXIT;
    }
    
ENABLE_TRAFFIC_EXIT:
    if (llid_obj)
        bdmf_put(llid_obj);

#endif

    return ret;
}

static tmctl_ret_e mac_flush_queue(tmctl_devType_e dev_type, unsigned int dev_id)
{
    int ret = TMCTL_SUCCESS;

#if defined(SUPPORT_FLUSH)
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t llid;
    
    tmctl_debug("%d, dev_id 0x%x", dev_type, dev_id);
    
    llid = get_impacted_devices(dev_type, dev_id);
    rc = eponStack_CtlFlushLlid(llid);
    if (rc)
    {
        ret = TMCTL_ERROR;
        tmctl_error("eponStack_CtlFlushLlid failed: link (%u) rc(%d)", llid, rc);
    }
#endif
    return ret;
}

tmctl_ret_e tmctl_RdpaTmInit(rdpa_drv_ioctl_dev_type rdpa_dev,
                                    tmctl_devType_e dev_type,
                                    unsigned int if_id,
                                    uint32_t cfg_flags,
                                    int num_queues)
{
    bdmf_object_handle root_tm = BDMF_NULL;
    tmctl_portTmParms_t tm = {0};
    set_parm_t root_parm = {0}; 
    BOOL is_shared;
    int ret;
    
    root_parm.if_id = if_id;
    root_parm.dev_type = dev_type;
    root_parm.num_queues = num_queues;
    root_parm.dir = getDir(rdpa_dev, if_id);
    get_port_tm_caps(root_parm.dir, dev_type, if_id, &tm);
    root_parm.sched_caps = tm.schedCaps;
    root_parm.sched_type = cfg_flags & TMCTL_SCHED_TYPE_MASK;
    root_parm.set_dual_rate = cfg_flags & TMCTL_SET_DUAL_RATE;

    if (check_cfg(&tm , root_parm.sched_type, num_queues, root_parm.set_dual_rate))
       return TMCTL_ERROR;

    ret = get_root_tm(dev_type, if_id, &root_tm);
    if (ret)
    {
        tmctl_error("get_root_tm failed, if_id[%d]", if_id);
        return TMCTL_ERROR;
    }

    ret = get_shared_mode(dev_type, &is_shared);
    if (ret)
    {
        tmctl_error("get_shared_mode failed");
        return TMCTL_ERROR;
    }

    ret = setup_root_tm(&root_parm, &root_tm, dev_type);
    if (ret)
    {
        tmctl_error("setup_root_tm failed, if_id[%d]", if_id);
        goto error;
    }

    if (cfg_flags & TMCTL_INIT_DEFAULT_QUEUES)   /* The default queues initialization is required */
    {
        ret = setup_default_queues(rdpa_dev, &root_parm, cfg_flags, dev_type);
        if (ret)
        {
            tmctl_error("setup_default_queues failed, if_id[%d]", if_id);
            goto error;
        }
    }

    return TMCTL_SUCCESS;
error:
    if (root_tm && !is_shared)
        bdmf_destroy(root_tm);
    return TMCTL_ERROR;
}

tmctl_ret_e tmctl_RdpaTmUninit(rdpa_drv_ioctl_dev_type rdpa_dev,
                                      tmctl_devType_e dev_type, 
                                      unsigned int if_id)
{
    bdmf_object_handle egress_tm = BDMF_NULL;
    BOOL is_shared;
    BOOL flush_required = FALSE;
    int ret;

    flush_required = is_flush_required(rdpa_dev, dev_type, if_id);
    ret = get_root_tm(dev_type, if_id, &egress_tm);
    if (ret)
    {
        tmctl_error("Failed to get egress_tm");
        return TMCTL_ERROR;
    }
    if (!egress_tm)
        return TMCTL_SUCCESS;

    ret = get_shared_mode(dev_type, &is_shared);
    if (ret)
    {
        tmctl_error("get_shared_mode failed, if_id[%d]", if_id);
        return TMCTL_ERROR;
    }

    /* On EPON platform, the delete queue/egress_tm should follow the sequence of 
      disable traffic, flush queue and enable traffic*/
    if (flush_required)
    {
        ret = enable_device_traffic(dev_type, if_id, FALSE);
        if (ret)
        {
            tmctl_error("enable_device_traffic disable dev %d failed", if_id);
            return TMCTL_ERROR;
        }
    
        ret = mac_flush_queue(dev_type, if_id);
        if (ret)
        {
            tmctl_error("mac_flush_queue dev %d failed", if_id);
            goto  UINT_EXIT;
        }
    }

    if (is_shared)
    {
        ret = reset_shared_root(egress_tm);
        if (ret)
            tmctl_error("Failed to reset root");
        goto  UINT_EXIT;
    }

    ret = bdmf_destroy(egress_tm);
    if (ret)
    {
        ret = TMCTL_ERROR;
        tmctl_error("bdmf_destroy failed, ret[%d]", ret);
        goto  UINT_EXIT;
    }

UINT_EXIT:
    if (flush_required)
        ret |= enable_device_traffic(dev_type, if_id, TRUE);

    return ret;
}

static int get_q_idx(bdmf_object_handle egress_tm, int id, int *idx)
{
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    uint8_t max_queues;
    int ret;
    int i;

    ret = rdpa_egress_tm_num_queues_get(egress_tm, &max_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_queues_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    for (i = 0; i < max_queues ; ++i)
    {
        ret = rdpa_egress_tm_queue_cfg_get(egress_tm, i, &queue_cfg);
        if (ret && ret != BDMF_ERR_NOENT)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_get failed, ret[%d]", ret);
            return TMCTL_ERROR;
        }

        if (!ret && (queue_cfg.queue_id == id))
        {
            *idx = i;
            return TMCTL_SUCCESS;
        }
    }
    
    return TMCTL_NOT_FOUND;
}

static int get_tm_mode(bdmf_object_handle root_tm, rdpa_tm_sched_mode *mode)
{
    bdmf_object_handle subsidiary_tm = BDMF_NULL;
    rdpa_tm_level_type level;
    int ret;

    ret = rdpa_egress_tm_mode_get(root_tm, mode);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_level_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }
   
    ret = rdpa_egress_tm_level_get(root_tm, &level);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_level_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    } 

    if (IS_SINGLE_LEVEL(level)) /* in level_queue 'set mode' and 'real mode' are the same */
        return TMCTL_SUCCESS;

    /* in multi level SP+WRR mode is repesented as SP so need to verify wiche one is it 
     * SP+WRR root must have subsidiary set as level egress_tm*/
    if (*mode == rdpa_tm_sched_sp) 
    {
        ret = rdpa_egress_tm_subsidiary_get(root_tm, SP_SUBSIDIARY_IDX, &subsidiary_tm);
        if (ret == BDMF_ERR_NOENT)
            return TMCTL_SUCCESS;
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_subsidiary_get failed, ret[%d]", ret);
            return TMCTL_ERROR; 
        }

        ret = rdpa_egress_tm_level_get(subsidiary_tm, &level);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_level_get failed, ret[%d]", ret);
            return TMCTL_ERROR;
        }
        if (!IS_SINGLE_LEVEL(level))
            *mode = rdpa_tm_sched_sp_wrr;
    }

    return TMCTL_SUCCESS;
}

static int get_queue_tm(bdmf_object_handle root_tm, int qid, bdmf_object_handle *egress_tm, int *idx)
{
    rdpa_tm_queue_location_t location;
    int ret;
   
    ret = rdpa_egress_tm_queue_location_get(root_tm, qid, &location);
    if (ret && ret != BDMF_ERR_NOENT)
    {
        tmctl_error("rdpa_egress_tm_queue_location_get failed, ret[%d]", ret);
        ret = TMCTL_ERROR;
    }
    else if (ret == BDMF_ERR_NOENT)
        ret = TMCTL_NOT_FOUND;
    else
    {
        *idx = location.queue_idx;
        *egress_tm = location.queue_tm;
        ret = TMCTL_SUCCESS;
    }

    return ret;
}

static int get_queue_shaper(bdmf_object_handle root_tm, 
                            bdmf_object_handle egress_tm, 
                            rdpa_tm_sched_mode tm_mode,
                            rdpa_traffic_dir dir,
                            tmctl_devType_e dev_type,
                            unsigned int if_id,
                            rdpa_tm_queue_cfg_t *queue_cfg, 
                            tmctl_queueCfg_t *qcfg_p)
{

    rdpa_tm_rl_rate_mode rl_mode;
    rdpa_tm_rl_cfg_t rl_cfg = {0};
    tmctl_portTmParms_t tm ={0};
    int ret;

    get_port_tm_caps(dir, dev_type, if_id, &tm);
    if(!tm.queueShaper)
    {
        qcfg_p->shaper.shapingRate = -1;
        qcfg_p->shaper.minRate = -1;
        qcfg_p->shaper.shapingBurstSize =-1;
        return TMCTL_SUCCESS;
    }

    ret = rdpa_egress_tm_rl_rate_mode_get(root_tm, &rl_mode);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_rl_rate_mode_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    if (tm_mode != rdpa_tm_sched_disabled)
    {
        if (rl_mode == rdpa_tm_rl_single_rate)
        {
            qcfg_p->shaper.shapingRate = BPS_TO_KBPS(queue_cfg->rl_cfg.af_rate);
            qcfg_p->shaper.shapingBurstSize = queue_cfg->rl_cfg.burst_size;
        }
        else 
        {
            qcfg_p->shaper.shapingRate = BPS_TO_KBPS(queue_cfg->rl_cfg.be_rate);
            qcfg_p->shaper.minRate = BPS_TO_KBPS(queue_cfg->rl_cfg.af_rate);
            qcfg_p->shaper.shapingBurstSize = queue_cfg->rl_cfg.burst_size;
        }
    }
    else
    {
        ret = rdpa_egress_tm_rl_get(egress_tm, &rl_cfg);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_rl_get failed, ret[%d]", ret);
            return TMCTL_ERROR;
        }

        if (rl_mode == rdpa_tm_rl_dual_rate)
        { 
            qcfg_p->shaper.minRate = BPS_TO_KBPS(rl_cfg.af_rate);
            if (rl_cfg.be_rate < 1000000000L)
                qcfg_p->shaper.shapingRate =  BPS_TO_KBPS(rl_cfg.af_rate + rl_cfg.be_rate);
        }
        else
            qcfg_p->shaper.shapingRate = BPS_TO_KBPS(rl_cfg.af_rate);

        qcfg_p->shaper.shapingBurstSize = rl_cfg.burst_size;
    }
    return TMCTL_SUCCESS;
}

static int switch_priority_and_idx(int prio_or_idx, int max_queues, tmctl_devType_e dev_type)
{
    BOOL is_shared;
    int ret;

    ret = get_shared_mode(dev_type, &is_shared);
    if (ret)
    {
        tmctl_error("is_shared_tm failed");
        return TMCTL_ERROR;
    }
 
#ifdef HIGH_P_LOW_IDX
    prio_or_idx = max_queues - prio_or_idx - 1;
#endif
    return (is_shared) ?  ++prio_or_idx : prio_or_idx;
}

#define get_priority_by_index(idx, max_queues, dev_type) switch_priority_and_idx(idx, max_queues, dev_type)
#define get_index_by_priority(priority, max_queues, dev_type) switch_priority_and_idx(priority, max_queues, dev_type)

tmctl_ret_e tmctl_RdpaQueueCfgGet(rdpa_drv_ioctl_dev_type rdpa_dev,
                                  tmctl_devType_e dev_type,
                                  unsigned int if_id,
                                  int queue_id,
                                  tmctl_queueCfg_t* qcfg_p)
{
    bdmf_object_handle root_tm = BDMF_NULL;
    bdmf_object_handle egress_tm = BDMF_NULL;
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    rdpa_tm_sched_mode tm_mode;
    rdpa_tm_level_type level;
    uint8_t num_queues;
    bdmf_index idx = 0;
    int ret;

    memset(qcfg_p, 0, sizeof(tmctl_queueCfg_t));
    
    get_root_tm(dev_type, if_id, &root_tm);
    if (!root_tm)
    {
        tmctl_error("Failed to get root_tm, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }

    ret = get_queue_tm(root_tm, queue_id, &egress_tm, &idx);
    if (ret == TMCTL_NOT_FOUND)
        return TMCTL_NOT_FOUND;
    else if (ret)
    {
        tmctl_error("get_queue_tm failed, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_mode_get(egress_tm, &tm_mode);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_mode_get failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
        return TMCTL_ERROR;
    } 
    
    ret = rdpa_egress_tm_level_get(root_tm, &level);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_mode_get failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
        return TMCTL_ERROR;
    }  
 
    ret = rdpa_egress_tm_queue_cfg_get(egress_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_get failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
        return TMCTL_ERROR;
    }

    ret = get_queue_shaper(root_tm, egress_tm, tm_mode, getDir(rdpa_dev, if_id), dev_type, if_id, &queue_cfg, qcfg_p);
    if (ret)
    {
        tmctl_error("get_queue_shaper failed, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }

    qcfg_p->qid = queue_id;
    qcfg_p->qsize = getUsrQueueSize(queue_cfg.drop_threshold);
    qcfg_p->minBufs = queue_cfg.reserved_packet_buffers;
    if (IS_SINGLE_LEVEL(level))
        qcfg_p->weight = queue_cfg.weight;
    else
    {
        bdmf_number weight;

        ret = rdpa_egress_tm_weight_get(egress_tm, &weight);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_weight_get failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
            return TMCTL_ERROR;
        }
        qcfg_p->weight = weight;
    }

    if (qcfg_p->weight)
    {
        qcfg_p->schedMode = TMCTL_SCHED_WRR;
        qcfg_p->priority = TMCTL_INVALID_KEY;
    }
    else
    {
        qcfg_p->schedMode = TMCTL_SCHED_SP;
        if (!IS_SINGLE_LEVEL(level))
        {
            int tm_idx = 0;

            ret = rdpa_egress_tm_subsidiary_find(root_tm, &tm_idx, &egress_tm);
            if (ret)
            {
                tmctl_error("rdpa_egress_tm_subsidiary_find failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
                return TMCTL_ERROR;
            }
            if (is_static_ctc_mode(dev_type, getDir(rdpa_dev, if_id)))
            {
                num_queues = STATIC_CTC_NUM_Q;
#ifdef BCM_XRDP
                /* in static ctc mode the lowest priority will be in index 1 the rest will be in index 0 */
                if (idx == 1)
                    idx = 7;
                else 
                    idx = tm_idx;
#else
                idx = tm_idx;
#endif
            }
            else
            {
                num_queues = MAX_Q_PER_TM_MULTI_LEVEL;
                idx = tm_idx;
            }
        }
        else
        {
            ret = rdpa_egress_tm_num_queues_get(egress_tm, &num_queues);
            if (ret)
            {
                tmctl_error("rdpa_egress_tm_num_queues_get failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
                return TMCTL_ERROR;
            }
        }
        qcfg_p->priority = get_priority_by_index(idx, num_queues, dev_type);
        if (qcfg_p->priority < 0)
        {
            tmctl_error("get_priority_by_index failed, if_id[%d]", if_id);
            return TMCTL_ERROR;
        }
    }

    return TMCTL_SUCCESS;
}


static int get_highest_wrr_q(bdmf_object_handle egress_tm, int ignord_id)
{
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    rdpa_tm_num_sp_elem num_sp_queues;
    uint8_t max_queues;
    int ret;
    int i;

    ret = rdpa_egress_tm_num_queues_get(egress_tm, &max_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_queues_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_num_sp_elements_get(egress_tm, &num_sp_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_sp_elements_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }
    
    for (i = num_sp_queues; i < max_queues ; ++i)
    {
        ret = rdpa_egress_tm_queue_cfg_get(egress_tm, i, &queue_cfg);
        if (ret && ret != BDMF_ERR_NOENT)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_get failed, ret[%d]", ret);
            return TMCTL_ERROR;
        }

        if (!ret && IS_ACTIV_Q(queue_cfg) && queue_cfg.queue_id != ignord_id)
            break;
    }

    return i;
}

static rdpa_tm_num_sp_elem get_num_sp_queues(int highest_sp_q)
{
    if (highest_sp_q < 0)
        return rdpa_tm_num_sp_elem_0;
    if (highest_sp_q < 2)
        return rdpa_tm_num_sp_elem_2;
    if (highest_sp_q < 4)
        return rdpa_tm_num_sp_elem_4;
    if (highest_sp_q < 8)
        return rdpa_tm_num_sp_elem_8;
    if (highest_sp_q < 16)
        return rdpa_tm_num_sp_elem_16;
    return rdpa_tm_num_sp_elem_32;
}

/*
 * return the minimum number of SP queues if current queue(idx) will be remove   
 */
static int num_sp_queues_after_rm(bdmf_object_handle egress_tm, int idx, rdpa_tm_num_sp_elem *new_num_sp_queues)
{
    rdpa_tm_num_sp_elem curr_num_sp_queues;
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    int highst_sp_q = -1;
    int ret;
    int i;

    ret = rdpa_egress_tm_num_sp_elements_get(egress_tm, &curr_num_sp_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_sp_elements_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    for (i = 0; i < curr_num_sp_queues; ++i)
    {
        /* skip current queue it will be removed */
        if (i != idx)
        {
            ret = rdpa_egress_tm_queue_cfg_get(egress_tm, i, &queue_cfg);
            if (ret && ret != BDMF_ERR_NOENT)
            {
                tmctl_error("rdpa_egress_tm_queue_cfg_get failed, ret[%d]", ret);
                return TMCTL_ERROR;
            }

            if (!ret && IS_ACTIV_Q(queue_cfg))
                highst_sp_q = i;
        }
    }

    *new_num_sp_queues = get_num_sp_queues(highst_sp_q);
    return TMCTL_SUCCESS;
}

/*
 * return the minimum number of SP queues if current queue(idx) will be add   
 */
static int num_sp_queues_after_add(bdmf_object_handle egress_tm, int idx, rdpa_tm_num_sp_elem *new_num_sp_queues)
{
    rdpa_tm_num_sp_elem curr_num_sp_queues;
    int ret;

    ret = rdpa_egress_tm_num_sp_elements_get(egress_tm, &curr_num_sp_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_sp_elements_set failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }
    
    *new_num_sp_queues = idx < curr_num_sp_queues ? curr_num_sp_queues : get_num_sp_queues(idx);
    return TMCTL_SUCCESS;
}

static void print_allowed_sched_mode(int mode)
{
    switch (mode)
    {
        case rdpa_tm_sched_sp_wrr:
            printf("Allowed sched modes are: SP and WRR.\n");
            break;
        case rdpa_tm_sched_sp:
             printf("Allowed sched mode is: SP.\n");
            break;
        case rdpa_tm_sched_wrr:
            printf("Allowed sched mode is: WRR.\n");
            break;
        case rdpa_tm_sched_disabled:
            printf("Sched mode is disabled.\n");
            break;
        default:
            tmctl_error("Sched mode %d, unknown.\n", mode);
            break;
    }
}

static int get_wrr_queue_idx(bdmf_object_handle egress_tm, int max, int min, int qid)
{
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    rdpa_tm_queue_location_t location;
    int ret;
    int i;

    ret = rdpa_egress_tm_queue_location_get(egress_tm, qid, &location);
    if (ret && ret != BDMF_ERR_NOENT)
    {
        tmctl_error("rdpa_egress_tm_queue_location_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    if(!ret && location.queue_idx >= min)
       return location.queue_idx;

    for (i = max - 1 ; i >= min ; --i)
    {
        ret = rdpa_egress_tm_queue_cfg_get(egress_tm, i, &queue_cfg);
        if (ret && ret != BDMF_ERR_NOENT)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_get failed, ret[%d]", ret);
            return -1;
        }

        if (ret || !IS_ACTIV_Q(queue_cfg))
            return i;
    }

    tmctl_error("No free queue index between min[%d] and max[%d]", min, max);
    return -1;
}

static int prepare_set_q_in_sp_mode_singel_level(bdmf_object_handle egress_tm, tmctl_queueCfg_t *qcfg_p, int current_idx)
{
    rdpa_tm_queue_cfg_t new_queue_cfg = {0};
    uint8_t max_queues;
    int idx;
    int ret;

    ret = rdpa_egress_tm_num_queues_get(egress_tm, &max_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_queues_get failed, qid[%d], ret[%d]",qcfg_p->qid, ret);
        return TMCTL_ERROR;
    }

    idx = get_index_by_priority(qcfg_p->priority, max_queues, 0);
    ret = rdpa_egress_tm_queue_cfg_get(egress_tm, idx, &new_queue_cfg);
    if (ret && ret != BDMF_ERR_NOENT)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_get failed, qid[%d], index[%d], ret [%d]",qcfg_p->qid, idx, ret);
        return TMCTL_ERROR;
    }
    
    if (!ret && IS_ACTIV_Q(new_queue_cfg) && new_queue_cfg.queue_id != qcfg_p->qid)
    {
        tmctl_error("Priority[%d](index[%d]) allready in use, qid[%d] current_qid[%d]", 
                qcfg_p->priority, idx, qcfg_p->qid, new_queue_cfg.queue_id);
        return TMCTL_ERROR;
    }

    /* if changing priority delete existing queue */
    if (idx != current_idx && (current_idx >= 0))
    {
        ret = rdpa_egress_tm_queue_cfg_delete(egress_tm, current_idx);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_delete failed, qid[%d], currnet index[%d], ret[%d]",qcfg_p->qid, current_idx, ret);
            return TMCTL_ERROR;
        }
    }

    return idx;
}

static int prepare_set_q_in_wrr_mode_singel_level(bdmf_object_handle egress_tm, tmctl_queueCfg_t *qcfg_p, int idx)
{
    uint8_t max_queues;
    int ret;

    /* queue exist */
    if (idx >= 0)
        goto exit;

    ret = rdpa_egress_tm_num_queues_get(egress_tm, &max_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_queues_get failed, qid[%d], ret[%d]",qcfg_p->qid, ret);
        return TMCTL_ERROR;
    }

    idx = get_wrr_queue_idx(egress_tm, max_queues, 0, qcfg_p->qid);

exit:
    return idx;
}

static int prepare_set_wrr_q_in_sp_wrr_mode_single_level(bdmf_object_handle egress_tm, 
                                                tmctl_queueCfg_t *qcfg_p, 
                                                int idx, 
                                                rdpa_tm_num_sp_elem *num_sp_queues_, 
                                                uint8_t max_queues)
{
    rdpa_tm_num_sp_elem num_sp_queues;
    int new_idx;
    int ret;

    ret = num_sp_queues_after_rm(egress_tm, idx, &num_sp_queues);
    if (ret)
    {
        tmctl_error("Failed to get new number of SP queues, qid[%d]",  qcfg_p->qid);
        return TMCTL_ERROR;
    }

    new_idx = get_wrr_queue_idx(egress_tm, max_queues, num_sp_queues, qcfg_p->qid);
    if (new_idx == -1)
    {
        tmctl_error("No place for new WRR queue, qid[%d]",  qcfg_p->qid);
        return TMCTL_ERROR;
    }

    *num_sp_queues_ = num_sp_queues;
    return new_idx;
}

static int is_queue_index_resereved(tmctl_devType_e dev_type, int idx)
{
    BOOL is_shared;
    int ret;

    ret = get_shared_mode(dev_type, &is_shared);
    if (ret)
    {
        tmctl_error("get_shared_mode failed");
        return TMCTL_ERROR;
    }

    if (is_shared && (idx == 0))
        return TRUE;
    
    return FALSE;
}

static int prepare_set_sp_q_in_sp_wrr_mode_single_level(bdmf_object_handle egress_tm, 
                                                        tmctl_queueCfg_t *qcfg_p,
                                                        int idx, 
                                                        rdpa_tm_num_sp_elem *num_sp_queues_, 
                                                        uint8_t max_queues, 
                                                        tmctl_devType_e dev_type)
{
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    rdpa_tm_num_sp_elem num_sp_queues;
    int highest_wrr_q;
    int new_idx;
    int ret;

    if (is_queue_index_resereved(dev_type, idx))
    {
        tmctl_error("queue is resereved");
        return TMCTL_ERROR;
    }
    new_idx = get_index_by_priority(qcfg_p->priority, max_queues, dev_type);
    ret = rdpa_egress_tm_queue_cfg_get(egress_tm, new_idx, &queue_cfg);
    if (ret && ret != BDMF_ERR_NOENT)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_get failed, qid[%d], index[%d], ret[%d]",  qcfg_p->qid, new_idx, ret);
        return TMCTL_ERROR;
    }

    if (!ret && IS_ACTIV_Q(queue_cfg) && queue_cfg.queue_id != qcfg_p->qid)
    {
        tmctl_error("No place for new SP queue, qid[%d] index[%d], priority[%d] is taken by currnet qid[%d]",  
                qcfg_p->qid, new_idx, qcfg_p->priority, queue_cfg.queue_id);
        return TMCTL_ERROR;
    }  

    ret = num_sp_queues_after_add(egress_tm, new_idx, &num_sp_queues);
    if (ret)
    {
        tmctl_error("Fail to get new number of SP queues, qid[%d]", qcfg_p->qid);
        return TMCTL_ERROR;
    }

    /* check that there are no WRR queues in new SP queues area */
    highest_wrr_q = get_highest_wrr_q(egress_tm, qcfg_p->qid);
    if (highest_wrr_q < 0)
    {
        tmctl_error("Fail to get highest WRR queue, qid[%d]", qcfg_p->qid);
        return TMCTL_ERROR;
    }

    if (highest_wrr_q < num_sp_queues)
    {
        tmctl_error("No place for new SP queue, qid[%d], priority[%d] highest WRR queue[%d] "
                  "num SP queues[%d], max queues[%d]", 
                  qcfg_p->qid, qcfg_p->priority, highest_wrr_q, num_sp_queues, max_queues);
        return TMCTL_ERROR;
    }

    *num_sp_queues_ = num_sp_queues;
    return new_idx;
}

static int prepare_set_q_in_sp_wrr_mode_singel_level(bdmf_object_handle egress_tm, 
                                                     tmctl_queueCfg_t *qcfg_p, 
                                                     int current_idx, 
                                                     tmctl_devType_e dev_type)
{
    rdpa_tm_num_sp_elem num_sp_queues;
    rdpa_tm_queue_cfg_t cur_queue_cfg = {0};
    uint8_t max_queues;
    int new_idx;
    int ret;
    
    ret = rdpa_egress_tm_num_queues_get(egress_tm, &max_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_queues_get failed, qid[%d], ret[%d]",  qcfg_p->qid, ret);
        return TMCTL_ERROR;
    }

    if (qcfg_p->schedMode == TMCTL_SCHED_WRR)
        new_idx = prepare_set_wrr_q_in_sp_wrr_mode_single_level(egress_tm, qcfg_p, current_idx, &num_sp_queues, max_queues);
    else 
        new_idx = prepare_set_sp_q_in_sp_wrr_mode_single_level(egress_tm, qcfg_p, current_idx, &num_sp_queues, max_queues, dev_type); 

    if (new_idx < 0)
        return TMCTL_ERROR;

    if (current_idx >= 0)
    {
        ret = rdpa_egress_tm_queue_cfg_get(egress_tm, current_idx, &cur_queue_cfg);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_get failed, qid[%d], index[%d], ret[%d]",  qcfg_p->qid, new_idx, ret);
            return TMCTL_ERROR;
        }
    }

    /* if changing position or sched mode delete queue in current position */
    if ((current_idx >= 0 && new_idx != current_idx) || ((current_idx >= 0) &&
        ((cur_queue_cfg.weight && qcfg_p->schedMode == TMCTL_SCHED_SP) ||  
        (!cur_queue_cfg.weight && qcfg_p->schedMode == TMCTL_SCHED_WRR))))
    {
        ret = rdpa_egress_tm_queue_cfg_delete(egress_tm, current_idx);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_delete failed, qid[%d], currnet index[%d], ret[%d]",qcfg_p->qid, current_idx, ret);
            return TMCTL_ERROR;
        }
    }

    ret = rdpa_egress_tm_num_sp_elements_set(egress_tm, num_sp_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_sp_elements_set failed, qid[%d], index[%d], ret[%d]",qcfg_p->qid, new_idx, ret);
        return TMCTL_ERROR;
    }

    return new_idx;
}

static int check_sched_mode(int wanted_sched_mode, int allowd_sched_mode)
{
    if ((wanted_sched_mode == TMCTL_SCHED_SP &&
        allowd_sched_mode != rdpa_tm_sched_sp  && allowd_sched_mode != rdpa_tm_sched_sp_wrr) ||
        (wanted_sched_mode == TMCTL_SCHED_WRR &&
        allowd_sched_mode != rdpa_tm_sched_wrr && allowd_sched_mode != rdpa_tm_sched_sp_wrr) ||
        allowd_sched_mode == rdpa_tm_sched_disabled || 
        (wanted_sched_mode != TMCTL_SCHED_SP && wanted_sched_mode != TMCTL_SCHED_WRR))
    {
        tmctl_error("Queue sched mode %d is not supported", wanted_sched_mode);
        print_allowed_sched_mode(allowd_sched_mode);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static int is_cfg_supported(tmctl_queueCfg_t *qcfg_p, get_root_parm_t *root_parm, tmctl_devType_e dev_type, unsigned int if_id, rdpa_traffic_dir dir)
{
    tmctl_portTmParms_t tm = {0};

    if (IS_SINGLE_LEVEL(root_parm->level))
    {
        if (qcfg_p->schedMode != TMCTL_SCHED_WRR && (qcfg_p->priority > root_parm->max_queues || qcfg_p->priority < 0))
        {
            tmctl_error("Priority of SP queue must < %d and >= 0, qid[%d] priority[%d]",
                    root_parm->max_queues, qcfg_p->qid, qcfg_p->priority);
            return FALSE;
        }
    }
    else
    {
        if (qcfg_p->schedMode != TMCTL_SCHED_WRR && (qcfg_p->priority > MAX_Q_PER_TM_MULTI_LEVEL || qcfg_p->priority < 0))
        {
            tmctl_error("Priority of SP queue must < %d and >= 0, qid[%d] priority[%d]",
                    MAX_Q_PER_TM_MULTI_LEVEL, qcfg_p->qid, qcfg_p->priority);
            return FALSE;
        }

        if (qcfg_p->shaper.shapingRate < qcfg_p->shaper.minRate)
        { 
            tmctl_error("Shaper configuration not allowed maxRate(%u) < minRate(%u)",
                        qcfg_p->shaper.shapingRate, qcfg_p->shaper.minRate);
            return FALSE;
        }
    }

    if ((qcfg_p->schedMode == TMCTL_SCHED_WRR) &&
            (qcfg_p->weight == 0))
    {
        tmctl_error("Weight of WRR/WFQ queue must be non-zero. qid[%d]", qcfg_p->qid);
        return FALSE;
    }

    if (check_sched_mode(qcfg_p->schedMode, root_parm->mode))
        return FALSE;

    get_port_tm_caps(dir, dev_type, if_id, &tm);
    if (!tm.queueShaper && (qcfg_p->shaper.shapingRate || qcfg_p->shaper.minRate || qcfg_p->shaper.shapingBurstSize))
    {
        tmctl_error("Queue shaper is not supported on this port");
        return FALSE;
    }

    return TRUE;
}

static int setup_queue_tm(bdmf_object_handle root_tm, 
                           int index, 
                           int weight, 
                           tmctl_queueCfg_t *qcfg_p, 
                           bdmf_object_handle *queue_tm)
{
    set_parm_t set_parm = {0};
    int ret;

    set_parm.dir = rdpa_dir_us;
    set_parm.tm_level = rdpa_tm_level_queue;
    set_parm.tm_mode = rdpa_tm_sched_disabled;
    set_parm.subsidiary_idx = index;
    set_parm.weight = weight;
    
    ret = setup_tm(&set_parm, root_tm, queue_tm);
    if (ret)
    {
        tmctl_error("Failed to setup queue tm");
        return TMCTL_ERROR;
    }
    return TMCTL_SUCCESS;
}

static int find_free_subsidiary_idx(bdmf_object_handle root_tm , int *idx)
{
    bdmf_object_handle subsidiary_tm = BDMF_NULL;
    int ret;
    int i;

    for (i = 0 ; i < MAX_Q_PER_TM_MULTI_LEVEL ; ++i)
    {
        ret = rdpa_egress_tm_subsidiary_get(root_tm, i, &subsidiary_tm);
        if (ret == BDMF_ERR_NOENT)
        {
            *idx = i;
            return TMCTL_SUCCESS;
        }
        else if(ret)
        {
            tmctl_error("rdpa_egress_tm_subsidiary_get failed, ret[%d]", ret);
            return TMCTL_ERROR;
        }
    }

    tmctl_error("No free subsidiary idx for new queue");
    return TMCTL_NOT_FOUND;
}

static int move_tm_to_new_index(bdmf_object_handle root_tm, bdmf_object_handle queue_tm, int new_idx)
{
    bdmf_object_handle parent_tm = queue_tm;
    bdmf_index old_idx;
    int ret;
    
    ret = rdpa_egress_tm_subsidiary_find(root_tm, &old_idx, &parent_tm);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_subsidiary_find failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }
    
    ret = rdpa_egress_tm_subsidiary_set(root_tm, old_idx, BDMF_NULL);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_subsidiary_set failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_subsidiary_set(root_tm, new_idx, queue_tm);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_subsidiary_set failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }
    return TMCTL_SUCCESS;
}

static int prepare_set_q_wrr_multi_level(bdmf_object_handle root_tm, tmctl_queueCfg_t *qcfg_p, bdmf_object_handle *egress_tm)
{
    int idx;
    int ret = 0;

    if (!*egress_tm)
    {
        ret = get_queue_tm(root_tm, qcfg_p->qid, egress_tm, &idx); 
        if (ret && ret != TMCTL_NOT_FOUND)
        {
            tmctl_error("get_queue_tm failed, qid[%d]", qcfg_p->qid);
            return TMCTL_ERROR;
        }
    }
    if (ret == TMCTL_NOT_FOUND)
    {
        ret = find_free_subsidiary_idx(root_tm, &idx);
        if (ret == TMCTL_NOT_FOUND)
        {
            tmctl_error("No free subsidiary index for new queue, qid[%d]", qcfg_p->qid);
            return TMCTL_ERROR;
        }
        else if (ret)
        {
            tmctl_error("find_free_subsidiary_idx failed, qid[%d]", qcfg_p->qid);
            return TMCTL_ERROR;
        }
        return setup_queue_tm(root_tm, idx, qcfg_p->weight, qcfg_p, egress_tm);
    }

    ret = rdpa_egress_tm_weight_set(*egress_tm, qcfg_p->weight);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_weight_set failed, qid[%d], ret[%d]", qcfg_p->qid, ret);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static inline int set_queue_tm_on_empty_index(bdmf_object_handle root_tm,
                                              tmctl_queueCfg_t *qcfg_p,
                                              int new_idx, 
                                              bdmf_object_handle *egress_tm,
                                              BOOL is_static_ctc)
{
    int ret;

    if (*egress_tm)
    {
        if (is_static_ctc)
        {
            tmctl_error("Can't change priorty of queues in static ctc mode");
            return TMCTL_ERROR;
        }

        ret = move_tm_to_new_index(root_tm, *egress_tm, new_idx);
        if (ret)
        {
            tmctl_error("move_tm_to_new_index failed, qid[%d]", qcfg_p->qid);
            return TMCTL_ERROR;
        }
    }
    else
        return setup_queue_tm(root_tm, new_idx, 0, qcfg_p, egress_tm);

    return TMCTL_SUCCESS;
}

static int prepare_set_q_sp_multi_level(bdmf_object_handle root_tm, tmctl_queueCfg_t *qcfg_p, bdmf_object_handle *egress_tm, BOOL is_static_ctc)
{
    bdmf_object_handle current_tm = BDMF_NULL;
    int new_idx;
    int queue_idx;
    int ret;
    int max_queues;

    if (!*egress_tm)
    { /* if egress_tm wasn't given search for it */
        ret = get_queue_tm(root_tm, qcfg_p->qid, egress_tm, &queue_idx); 
        if (ret && ret != TMCTL_NOT_FOUND)
        {
            tmctl_error("get_queue_tm failed, qid[%d]", qcfg_p->qid);
            return TMCTL_ERROR;
        }
    }

    max_queues = is_static_ctc ? STATIC_CTC_NUM_Q : MAX_Q_PER_TM_MULTI_LEVEL;
    new_idx = get_index_by_priority(qcfg_p->priority, max_queues, 0);

    ret = rdpa_egress_tm_subsidiary_get(root_tm, new_idx, &current_tm);
    if (ret && ret != BDMF_ERR_NOENT)
    {
        tmctl_error("rdpa_egress_tm_subsidiary_get failed, qid[%d], ret[%d]", qcfg_p->qid, ret);
        return TMCTL_ERROR;
    }

    if (ret == BDMF_ERR_NOENT)
        return set_queue_tm_on_empty_index(root_tm, qcfg_p, new_idx, egress_tm, is_static_ctc);

    /* if queue in wanted index not the same as configured queue priority is taken */
    if (current_tm != *egress_tm)
    {
        tmctl_error("Priority[%d] allready in use", qcfg_p->priority);
        return BDMF_ERR_ALREADY;
    }

    return TMCTL_SUCCESS;
}

static int get_subsidiary_tm(bdmf_object_handle root_tm, tmctl_sched_e sched_mode, bdmf_object_handle *subsidiary_tm)
{
    int ret;
    int idx = (sched_mode == TMCTL_SCHED_SP)? SP_SUBSIDIARY_IDX : WRR_SUBSIDIARY_IDX;

    ret = rdpa_egress_tm_subsidiary_get(root_tm, idx ,subsidiary_tm);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_subsidiary_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }
    
    return TMCTL_SUCCESS;
}

static int get_current_parent(bdmf_object_handle root_tm, bdmf_object_handle queue_tm, bdmf_object_handle *parent)
{
    rdpa_tm_sched_mode mode;
    bdmf_number weight = 0;
    bdmf_index sub_id; 
    
    int ret;

    ret = get_tm_mode(root_tm, &mode);
    if (ret)
    {
        tmctl_error("get_tm_mode failed");
        return TMCTL_ERROR;
    }

    if (mode != rdpa_tm_sched_sp_wrr) /* if mode is wrr or sp parent is root */
    {
        *parent = root_tm;
        return TMCTL_SUCCESS;
    }

    ret = rdpa_egress_tm_weight_get(queue_tm, &weight);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_weight_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    /* WRR queues must have weight and SP queues can't */
    sub_id = (weight) ? WRR_SUBSIDIARY_IDX : SP_SUBSIDIARY_IDX;
    ret = rdpa_egress_tm_subsidiary_get(root_tm, sub_id, parent);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_subsidiary_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static int remove_if_switch_tm(bdmf_object_handle root_tm, bdmf_object_handle new_parent, tmctl_queueCfg_t *qcfg_p, bdmf_object_handle *queue_tm)
{
    bdmf_object_handle old_parent = BDMF_NULL;
    int idx;
    int ret;

    ret = get_queue_tm(root_tm, qcfg_p->qid, queue_tm, &idx);
    if (ret == TMCTL_ERROR)
    {
        tmctl_error("get_queue_tm failed, qid[%d], ret[%d]", qcfg_p->qid, ret);
        return TMCTL_ERROR;
    }
    else if (!ret)
    {
        ret = get_current_parent(root_tm, *queue_tm, &old_parent);
        if (ret)
        {
            tmctl_error("get_current_parent failed, qid[%d], ret[%d]", qcfg_p->qid, ret);
            return TMCTL_ERROR;
        }

        if (new_parent != old_parent)
        {
            ret = bdmf_destroy(*queue_tm);
            if (ret)
            {
                tmctl_error("bdmf_destroy failed, qid[%d], ret[%d]", qcfg_p->qid, ret);
                return TMCTL_ERROR;
            }
            *queue_tm = BDMF_NULL;
        }
    }

    return TMCTL_SUCCESS;
}

static int prepare_set_q_in_sp_wrr_mode_level_multi_level(bdmf_object_handle root_tm, tmctl_queueCfg_t *qcfg_p, bdmf_object_handle *egress_tm)
{
    bdmf_object_handle subsidiary_tm = BDMF_NULL;
    int ret;
  
    ret = get_subsidiary_tm(root_tm, qcfg_p->schedMode, &subsidiary_tm);
    if (ret)
    {
        tmctl_error("get_subsidiary_tm failed, qid[%d]", qcfg_p->qid);
        return TMCTL_ERROR;
    } 

    /* if queue change mode remove it before seting new mode */
    ret = remove_if_switch_tm(root_tm, subsidiary_tm, qcfg_p, egress_tm);
    if (ret)
    {
        tmctl_error("remove_if_switch_tm failed, qid[%d]", qcfg_p->qid);
        return TMCTL_ERROR;
    }

    if (qcfg_p->schedMode == TMCTL_SCHED_SP)
       ret = prepare_set_q_sp_multi_level(subsidiary_tm, qcfg_p, egress_tm, FALSE); 
    else
       ret = prepare_set_q_wrr_multi_level(subsidiary_tm, qcfg_p, egress_tm); 
    if (ret)
    {
        tmctl_error("Failed to prepare set queue, qid[%d], ret[%d]", qcfg_p->qid, ret);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static int static_ctc_shared_tm_set(bdmf_object_handle root_tm, tmctl_queueCfg_t *qcfg_p, bdmf_object_handle *egress_tm)
{
    bdmf_object_handle shared_tm = BDMF_NULL;
    set_parm_t set_parm = {0};
    int ret;

    ret = rdpa_egress_tm_subsidiary_get(root_tm, STATIC_CTC_SHARED_TM_IDX, &shared_tm);
    if (ret && ret != BDMF_ERR_NOENT)
    {
        tmctl_error("rdpa_egress_tm_subsidiary_get failed, ret[%d]", ret);
        return TMCTL_ERROR; 
    }
    if (ret)
    {
        set_parm.dir = rdpa_dir_us;
        set_parm.tm_level = rdpa_tm_level_queue;
        set_parm.tm_mode = rdpa_tm_sched_sp;
        set_parm.num_queues = STATIC_CTC_NUM_Q;
        set_parm.subsidiary_idx = STATIC_CTC_SHARED_TM_IDX;

        ret = setup_tm(&set_parm, root_tm, &shared_tm);
        if (ret)
        {
           tmctl_error("setup_tm failed");
           return TMCTL_ERROR;
        }
    }
    else
    {
        int index = qcfg_p->priority == 0 ? 1 : 0;
        rdpa_tm_queue_cfg_t queue_cfg;

        ret = rdpa_egress_tm_queue_cfg_get(shared_tm, index ,&queue_cfg);
        if (ret && ret != BDMF_ERR_NOENT)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_get failed, ret[%d]", ret);
            return TMCTL_ERROR; 
        }
        else if (!ret && queue_cfg.queue_id != qcfg_p->qid)
        {
            tmctl_error("Priority[%d] already taken by qid[%d]", qcfg_p->priority, queue_cfg.queue_id);
            return TMCTL_ERROR; 
        }
    }
    *egress_tm = shared_tm;
    return TMCTL_SUCCESS;
}

/***************************************************************
 * XRDP static CTC egress_tm configuration.
 * Maximum 8 queues. the two lowest priority queues share one queue_tm.
 * The lowest priority(0) will be in queue index 1 and the second(1) in index 0.
 * The rest of the queues will act the same as multi level RDP, 
 * each queue will get its own queue_tm and the index will be 0.
 * Hight prirty will be in the lowest subsidiart index.
 *
 *                            ______ 
 * ____     |--sub_index 0 ---|Q TM|-- q index 0 prio 7
 * |TM|     |                 |____|
 * |__|-----|                 ______ 
 *          |--sub_index 1----|Q TM|-- q index 0 prio 6
 *          |                 |____|     
 *          |                 ______
 *          |--sub_index 6----|Q TM|-- q index 0 prio 1
 *                            |____|-- q index 1 prio 0
 *          
 ****************************************************************/
static int static_ctc_queue_set(bdmf_object_handle root_tm, tmctl_queueCfg_t *qcfg_p, bdmf_object_handle *egress_tm, int *idx)
{
    int ret;
    
#ifdef BCM_XRDP
    *idx = (qcfg_p->priority == 0) ? 1 : 0;
    if (qcfg_p->priority == 1 || qcfg_p->priority == 0)
    {
        ret = static_ctc_shared_tm_set(root_tm, qcfg_p, egress_tm);
        if (ret)
        {
           tmctl_error("static_ctc_shared_tm_set failed");
           return TMCTL_ERROR;
        }
        return TMCTL_SUCCESS;
    }
#endif

    ret = prepare_set_q_sp_multi_level(root_tm, qcfg_p, egress_tm, TRUE);
    if (ret)
    {
        tmctl_error("prepare_set_q_sp_multi_level failed, qid[%d]", qcfg_p->qid);
        return TMCTL_ERROR;
    }
    return TMCTL_SUCCESS;
}


static inline int prepare_set_q_in_multi_level(bdmf_object_handle root_tm, 
                                        rdpa_tm_sched_mode mode, 
                                        tmctl_queueCfg_t *qcfg_p, 
                                        bdmf_object_handle *egress_tm)
{
    int ret;

    switch (mode)
    {
        case rdpa_tm_sched_sp:
            ret = prepare_set_q_sp_multi_level(root_tm, qcfg_p, egress_tm, FALSE);
            if (ret)
            {
                tmctl_error("prepare_set_q_sp_multi_level failed, qid[%d]", qcfg_p->qid);
                return TMCTL_ERROR;
            }
            break;
        case rdpa_tm_sched_wrr:
            ret = prepare_set_q_wrr_multi_level(root_tm, qcfg_p, egress_tm);
            if (ret)
            {
                tmctl_error("prepare_set_q_wrr_multi_level failed, qid[%d]", qcfg_p->qid);
                return TMCTL_ERROR;
            }
            break;
        case rdpa_tm_sched_sp_wrr:
            ret = prepare_set_q_in_sp_wrr_mode_level_multi_level(root_tm, qcfg_p, egress_tm);
            if (ret)
            {
                tmctl_error("prepare_set_q_in_sp_wrr_mode_level_multi_level failed, qid[%d]", qcfg_p->qid);
                return TMCTL_ERROR;
            }
            break;
        default:
            tmctl_error("Failed to get TM mode, qid[%d]", qcfg_p->qid);
            return TMCTL_ERROR;
    }

    ret = set_queue_rl_multi_level(root_tm, *egress_tm, &qcfg_p->shaper);
    if (ret)
    {
        tmctl_error("set_queue_rl_multi_level failed, qid[%d]", qcfg_p->qid);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static inline int prepare_set_q_in_singel_level(bdmf_object_handle egress_tm, 
                                         tmctl_queueCfg_t *qcfg_p, 
                                         rdpa_tm_sched_mode mode, 
                                         int *idx,
                                         tmctl_devType_e dev_type)
{
    int current_idx = TMCTL_INVALID_KEY;
    int ret;

    ret = get_q_idx(egress_tm, qcfg_p->qid, &current_idx);
    if (ret && ret != TMCTL_NOT_FOUND)
    {
        tmctl_error("get_q_idx failed, qid[%d]", qcfg_p->qid);
        return TMCTL_ERROR;
    }
    
    switch (mode)
    {
        case rdpa_tm_sched_sp:
            *idx = prepare_set_q_in_sp_mode_singel_level(egress_tm, qcfg_p, current_idx);
            if (*idx < 0)
            {
                tmctl_error("prepare_set_q_in_sp_mode_singel_level failed, qid[%d]", qcfg_p->qid);
                return TMCTL_ERROR;
            }
            break;
        case rdpa_tm_sched_wrr:
            *idx = prepare_set_q_in_wrr_mode_singel_level(egress_tm, qcfg_p, current_idx);
            if (*idx < 0)
            {
                tmctl_error("prepare_set_q_in_wrr_mode_singel_level failed, qid[%d]", qcfg_p->qid);
                return TMCTL_ERROR;
            }
            break;
        case rdpa_tm_sched_sp_wrr:
            *idx = prepare_set_q_in_sp_wrr_mode_singel_level(egress_tm, qcfg_p, current_idx, dev_type);
            if (*idx < 0)
            {
                tmctl_error("prepare_set_q_in_sp_wrr_mode_singel_level failed, qid[%d]", qcfg_p->qid);
                return TMCTL_ERROR;
            }
            break;
        default:
            tmctl_error("Mode %d not supported, qid[%d]", mode, qcfg_p->qid);
            return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static int get_root_parm(tmctl_devType_e dev_type, unsigned int if_id, get_root_parm_t *root_parm)
{
    int ret;

    get_root_tm(dev_type, if_id, &root_parm->root_tm);
    if (!root_parm->root_tm)
    {
        tmctl_error("Failed to get egress_tm, if_id[%d]", if_id);
        return TMCTL_ERROR;
    }

    ret = get_tm_mode(root_parm->root_tm, &root_parm->mode);
    if (ret)
    {
        tmctl_error("get_tm_mode failed, if_id[%d], ret[%d]", if_id, ret);
        return TMCTL_ERROR;
    }
    
    ret = rdpa_egress_tm_level_get(root_parm->root_tm, &root_parm->level);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_level_get failed, if_id[%d], ret[%d]", if_id, ret);
        return TMCTL_ERROR;
    }
    
    if (IS_SINGLE_LEVEL(root_parm->level))
    {
        ret = rdpa_egress_tm_num_queues_get(root_parm->root_tm, &root_parm->max_queues);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_num_queues_get failed, if_id[%d], ret[%d]", if_id, ret);
            return TMCTL_ERROR;
        }
    }
  
    return TMCTL_SUCCESS;
}

static int prepare_set_q(get_root_parm_t *root_parm, 
                         tmctl_queueCfg_t *qcfg_p, 
                         int *idx, 
                         bdmf_object_handle *egress_tm, 
                         tmctl_devType_e dev_type,
                         BOOL is_static_ctc)
{
    int ret;

    if (!IS_SINGLE_LEVEL(root_parm->level))
    {
        *idx = 0;
        if (is_static_ctc)
            return static_ctc_queue_set(root_parm->root_tm, qcfg_p, egress_tm, idx);

        ret = prepare_set_q_in_multi_level(root_parm->root_tm, root_parm->mode, qcfg_p, egress_tm);
        if (ret)
        {
            tmctl_error("prepare_set_q_in_multi_level, qid[%d]", qcfg_p->qid);
            return TMCTL_ERROR;
        }
    }
    else
    {
        *egress_tm = root_parm->root_tm;
        ret = prepare_set_q_in_singel_level(*egress_tm, qcfg_p, root_parm->mode, idx, dev_type);
        if (ret)
        {
            tmctl_error("prepare_set_q_in_singel_level, qid[%d]", qcfg_p->qid);
            return TMCTL_ERROR;
        }
    }

    return TMCTL_SUCCESS;
}

static inline void queue_shaper_cfg_set(rdpa_tm_rl_rate_mode rl_mode, 
                                        tmctl_shaper_t *shaper_p, 
                                        rdpa_tm_queue_cfg_t *queue_cfg)
{
    if (rl_mode == rdpa_tm_rl_dual_rate)
    {
        queue_cfg->rl_cfg.be_rate = KBPS_TO_BPS(shaper_p->shapingRate);
        queue_cfg->rl_cfg.af_rate = KBPS_TO_BPS(shaper_p->minRate);
        queue_cfg->rl_cfg.burst_size = shaper_p->shapingBurstSize;
    }
    else
    {
        /* Single mode, variable af_rate used as best effort rate*/
        queue_cfg->rl_cfg.af_rate = KBPS_TO_BPS(shaper_p->shapingRate);
        queue_cfg->rl_cfg.burst_size = shaper_p->shapingBurstSize;
    }
}

static inline void prepare_queue_struct(tmctl_queueCfg_t *qcfg_p, 
                                        rdpa_tm_queue_cfg_t *queue_cfg, 
                                        rdpa_tm_rl_rate_mode rl_rate_mode)
{
    queue_cfg->queue_id = qcfg_p->qid;
    queue_cfg->drop_alg = rdpa_tm_drop_alg_dt;
    queue_cfg->drop_threshold = getDeviceQueueSize(qcfg_p->qsize);
#if defined(BCM_PON_XRDP) || defined(BCM_DSL_XRDP) /* in rdp shaper is set on queue tm */
    queue_shaper_cfg_set(rl_rate_mode, &qcfg_p->shaper, queue_cfg);
#endif
    queue_cfg->stat_enable = 1;
    /* change configured weight to 0 for SP */
    queue_cfg->weight = qcfg_p->schedMode == TMCTL_SCHED_SP ? 0 : qcfg_p->weight;
    queue_cfg->reserved_packet_buffers = qcfg_p->minBufs;
    queue_cfg->best_effort = qcfg_p->bestEffort;
}

tmctl_ret_e tmctl_RdpaTmQueueSet(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          unsigned int if_id,
                          tmctl_queueCfg_t *qcfg_p)
{
    bdmf_object_handle egress_tm = BDMF_NULL;
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    get_root_parm_t root_parm = {0};
    tmctl_queueCfg_t saved_qcfg_p = {0};
    BOOL restore_queue = FALSE;
    rdpa_tm_rl_rate_mode rl_rate_mode = rdpa_tm_rl_single_rate;
    static int depth = 0;
    int idx;
    int ret;
    BOOL is_static_ctc = is_static_ctc_mode(dev_type, getDir(rdpa_dev, if_id));

    ret = get_root_parm(dev_type, if_id, &root_parm);
    if (ret)
    {
        tmctl_error("get_root_parm failed, if_id[%d], qid[%d]", if_id, qcfg_p->qid);
        return TMCTL_ERROR;
    }

    ignore_negtiv_shaper(&qcfg_p->shaper);
    ret = !is_cfg_supported(qcfg_p, &root_parm, dev_type, if_id, getDir(rdpa_dev, if_id));
    if (ret)
    {
        tmctl_error("is_cfg_supported failed, if_id[%d], qid[%d]", if_id, qcfg_p->qid);
        return TMCTL_ERROR;
    }

    ret = tmctl_RdpaQueueCfgGet(rdpa_dev, dev_type, if_id, qcfg_p->qid, &saved_qcfg_p);
    if (ret && ret != TMCTL_NOT_FOUND)
    {
        tmctl_error("tmctl_RdpaQueueCfgGet failed, if_id[%d], qid[%d]", if_id, qcfg_p->qid);
        goto error;
    }
    else if (!ret)
        restore_queue = TRUE;

    /* prepare egress_tm for new queue configuration. update num_sp_queues, update egress_tm weight, create egress_tm, etc.. */
    ret = prepare_set_q(&root_parm, qcfg_p, &idx, &egress_tm, dev_type, is_static_ctc);
    if (ret)
    {
        tmctl_error("prepare_set_q failed, if_id[%d], qid[%d]", if_id, qcfg_p->qid);
        goto error;
    }
    if (IS_SINGLE_LEVEL(root_parm.level) && getDir(rdpa_dev, if_id) == rdpa_dir_us)
    {
        ret = rdpa_egress_tm_rl_rate_mode_get(root_parm.root_tm, &rl_rate_mode);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_rl_rate_mode_get failed, if_id[%d], qid[%d], ret[%d]", if_id, qcfg_p->qid, ret);
            goto error;
        }
    }

    prepare_queue_struct(qcfg_p, &queue_cfg, rl_rate_mode);
    ret = rdpa_egress_tm_queue_cfg_set(egress_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_set failed, if_id[%d], qid[%d], index[%d], ret[%d]", if_id, qcfg_p->qid, idx, ret);
        goto error;
    }

#if defined(SUPPORT_DPI) && !defined(CHIP_6846)
    if (qcfg_p->bestEffort && getDir(rdpa_dev, if_id) == rdpa_dir_us && rdpa_if_is_wan(if_id))
    {
        ret = add_us_dpi_queues(egress_tm, qcfg_p);
        if (ret)
            goto error;
    }
#endif

    return TMCTL_SUCCESS;

error:
    if (egress_tm && !IS_SINGLE_LEVEL(root_parm.level) && !is_static_ctc)
         bdmf_destroy(egress_tm);
    /* rollback previous config if available, prevent infinit loop */
    if (restore_queue)
    {
        if (!depth)
        {
            depth = 1;
            ret = tmctl_RdpaTmQueueSet(rdpa_dev, dev_type, if_id, &saved_qcfg_p);
            if (ret)
                tmctl_error("Failed to set old cfg back, if_id[%d], qid[%d]", if_id, qcfg_p->qid);
        }
        depth = 0;
    }
    return TMCTL_ERROR;
}

tmctl_ret_e tmctl_RdpaTmQueueDel(rdpa_drv_ioctl_dev_type rdpa_dev,
                                 tmctl_devType_e dev_type,
                                 unsigned int if_id,
                                 int queue_id)
{
    bdmf_object_handle root_tm = BDMF_NULL;
    bdmf_object_handle egress_tm = BDMF_NULL;
    BOOL flush_required = FALSE;
    int idx;
    int ret;

    flush_required = is_flush_required(rdpa_dev, dev_type, if_id);

    if (is_static_ctc_mode(dev_type, getDir(rdpa_dev, if_id)))
    {
        tmctl_error("Deleting queues is not allowed in static CTC mode");
        return TMCTL_ERROR;
    }

    get_root_tm(dev_type, if_id, &root_tm);
    if (!root_tm)
    {
        tmctl_error("Failed to get egress_tm, qid[%d]", queue_id);
        return TMCTL_ERROR;
    }

    ret = get_queue_tm(root_tm, queue_id, &egress_tm, &idx);
    if (ret)
    {
        tmctl_error("get_queue_tm failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
        return ret;
    }

    if (flush_required)
    {
        ret = enable_device_traffic(dev_type, if_id, FALSE);
        if (ret)
        {
            tmctl_error("enable_device_traffic disable dev %d failed", if_id);
            return TMCTL_ERROR;
        }
    
        ret = mac_flush_queue(dev_type, if_id);
        if (ret)
        {
            tmctl_error("mac_flush_queue dev %d failed", if_id);
            goto  DEL_EXIT;
        }
    }

    if (root_tm == egress_tm) /* single level */
    {    
        if (is_queue_index_resereved(dev_type, idx))
        {
            tmctl_error("Can't delete reserve queue, qid[%d]", queue_id);
            ret = TMCTL_ERROR;
            goto  DEL_EXIT;
        }     
            
        ret = rdpa_egress_tm_queue_cfg_delete(egress_tm, idx);
        if (ret)
        {
            tmctl_error("rdpa_egress_tm_queue_cfg_delete failed, qid[%d], index[%d], ret[%d]", queue_id, idx, ret);
            ret = TMCTL_ERROR;
            goto  DEL_EXIT;
        }
    }
    else /* multi level */
    { 
        ret = bdmf_destroy(egress_tm);
        if (ret)
        {
            tmctl_error("bdmf_destroy failed, qid[%d], ret[%d]", queue_id, ret);
            ret = TMCTL_ERROR;
            goto  DEL_EXIT;
        }
    }

DEL_EXIT:
    if (flush_required)
        ret |= enable_device_traffic(dev_type, if_id, TRUE);

    return ret;
}

tmctl_ret_e tmctl_RdpaGetPortShaper(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          unsigned int if_id,
                          tmctl_shaper_t *shaper_p)
{
    bdmf_object_handle egress_tm = BDMF_NULL;
    rdpa_tm_rl_cfg_t rl;
    tmctl_portTmParms_t tm = {0};
    int ret;

    get_port_tm_caps(getDir(rdpa_dev, if_id), dev_type, if_id, &tm);
    if (!tm.portShaper)
    {
        tmctl_error("Port shaper is not supported on this port");
        return TMCTL_ERROR;
    }

    get_root_tm(dev_type, if_id, &egress_tm);
    if (!egress_tm)
    {
        tmctl_error("Failed to get egress_tm if_id[%d]", if_id);
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_rl_get(egress_tm, &rl);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_rl_get failed, if_id[%d], ret[%d]", if_id, ret);
        return TMCTL_ERROR;
    }
   
    shaper_p->shapingRate = BPS_TO_KBPS(rl.af_rate); /* Best Effort: shaping_rate is in kbit/s: 1 kilobit = 1000 bits */
    shaper_p->shapingBurstSize = rl.burst_size;

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_RdpaSetPortShaper(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          unsigned int if_id,
                          tmctl_shaper_t *shaper_p)
{
    bdmf_object_handle egress_tm = BDMF_NULL;
    rdpa_tm_rl_cfg_t rl = {0};
    tmctl_portTmParms_t tm = {0};
    int ret;

    get_port_tm_caps(getDir(rdpa_dev, if_id), dev_type, if_id, &tm);
    if (!tm.portShaper)
    {
        tmctl_error("Port shaper is not supported on this port");
        return TMCTL_ERROR;
    }

    rl.af_rate = KBPS_TO_BPS(shaper_p->shapingRate); /* Best Effort: shaping_rate is in kbit/s: 1 kilobit = 1000 bits */
    rl.burst_size = shaper_p->shapingBurstSize;

    get_root_tm(dev_type, if_id, &egress_tm);
    if (!egress_tm)
    {
        tmctl_error("Failed to get egress_tm, if_id[%d]", if_id);
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_rl_set(egress_tm, &rl);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_rl_set failed, if_id[%d], ret[%d]", if_id, ret);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}
    
tmctl_ret_e tmctl_RdpaGetQueueDropAlg(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          rdpa_if if_id,
                          int queue_id,
                          tmctl_queueDropAlg_t *dropAlg_p)
{
    bdmf_object_handle queue_tm = BDMF_NULL;
    bdmf_object_handle root_tm = BDMF_NULL;
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    int idx;
    int ret;

    get_root_tm(dev_type, if_id, &root_tm);
    if (!root_tm)
    {
        tmctl_error("Failed to get egress_tm, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }

    ret = get_queue_tm(root_tm, queue_id, &queue_tm, &idx);
    if (ret)
    {
        tmctl_error("get_queue_tm failed, qid[%d]", queue_id);
        return TMCTL_ERROR;
    } 

    /* if singel level check if not reserved queue */
    if ((root_tm == queue_tm) && is_queue_index_resereved(dev_type, idx))
    {
        tmctl_error("queue is resereved");
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_queue_cfg_get(queue_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_get failed, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }   


    if (queue_cfg.drop_alg == rdpa_tm_drop_alg_red)
    {
        dropAlg_p->dropAlgorithm = TMCTL_DROP_RED;

        if (queue_cfg.low_class.min_threshold)
            dropAlg_p->dropAlgLo.redMinThreshold = getUsrQueueSize(queue_cfg.low_class.min_threshold);
        if (queue_cfg.low_class.max_threshold)
            dropAlg_p->dropAlgLo.redMaxThreshold = getUsrQueueSize(queue_cfg.low_class.max_threshold);
        dropAlg_p->dropAlgLo.redPercentage = RDPA_WRED_MAX_DROP_PROBABILITY;
    }
    else if (queue_cfg.drop_alg == rdpa_tm_drop_alg_wred)
    {
        dropAlg_p->dropAlgorithm = TMCTL_DROP_WRED;

        if (queue_cfg.low_class.min_threshold)
            dropAlg_p->dropAlgLo.redMinThreshold = getUsrQueueSize(queue_cfg.low_class.min_threshold);
        if (queue_cfg.low_class.max_threshold)
            dropAlg_p->dropAlgLo.redMaxThreshold = getUsrQueueSize(queue_cfg.low_class.max_threshold);
        if (queue_cfg.high_class.min_threshold)
            dropAlg_p->dropAlgHi.redMinThreshold = getUsrQueueSize(queue_cfg.high_class.min_threshold);
        if (queue_cfg.high_class.max_threshold)
            dropAlg_p->dropAlgHi.redMaxThreshold = getUsrQueueSize(queue_cfg.high_class.max_threshold);

        dropAlg_p->dropAlgLo.redPercentage = RDPA_WRED_MAX_DROP_PROBABILITY; 
        dropAlg_p->dropAlgHi.redPercentage = RDPA_WRED_MAX_DROP_PROBABILITY; 
        dropAlg_p->priorityMask0 = queue_cfg.priority_mask_0;
        dropAlg_p->priorityMask1 = queue_cfg.priority_mask_1;
    }
    else
        dropAlg_p->dropAlgorithm = TMCTL_DROP_DT;

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_RdpaSetQueueDropAlg(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          unsigned int if_id,
                          int queue_id,
                          tmctl_queueDropAlg_t *dropAlg_p)
{
    bdmf_object_handle queue_tm = BDMF_NULL;
    bdmf_object_handle root_tm = BDMF_NULL;
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    int idx;
    int ret;

    get_root_tm(dev_type, if_id, &root_tm);
    if (!root_tm)
    {
        tmctl_error("Failed to get egress_tm, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }

    ret = get_queue_tm(root_tm, queue_id, &queue_tm, &idx);
    if (ret)
    {
        tmctl_error("get_queue_tm failed, qid[%d]", queue_id);
        return TMCTL_ERROR;
    } 

    /* if singel level check if not reserved queue */
    if ((root_tm == queue_tm) && is_queue_index_resereved(dev_type, idx))
    {
        tmctl_error("queue is resereved");
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_queue_cfg_get(queue_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_get failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
        return TMCTL_ERROR;
    }   

    queue_cfg.drop_alg = convertDropAlg(dropAlg_p->dropAlgorithm);

    if (queue_cfg.drop_alg == rdpa_tm_drop_alg_red)
    {
        queue_cfg.low_class.min_threshold  = getDeviceQueueSize(dropAlg_p->dropAlgLo.redMinThreshold);
        queue_cfg.low_class.max_threshold  = getDeviceQueueSize(dropAlg_p->dropAlgLo.redMaxThreshold);
        queue_cfg.high_class.min_threshold = queue_cfg.drop_threshold;
        queue_cfg.high_class.max_threshold = queue_cfg.drop_threshold;
    }
    else if (queue_cfg.drop_alg == rdpa_tm_drop_alg_wred)
    {
        queue_cfg.low_class.min_threshold  = getDeviceQueueSize(dropAlg_p->dropAlgLo.redMinThreshold);
        queue_cfg.low_class.max_threshold  = getDeviceQueueSize(dropAlg_p->dropAlgLo.redMaxThreshold);
        queue_cfg.high_class.min_threshold = getDeviceQueueSize(dropAlg_p->dropAlgHi.redMinThreshold);
        queue_cfg.high_class.max_threshold = getDeviceQueueSize(dropAlg_p->dropAlgHi.redMaxThreshold);
        queue_cfg.priority_mask_0 = dropAlg_p->priorityMask0;
        queue_cfg.priority_mask_1 = dropAlg_p->priorityMask1;
    }
    else /* DT */
    {
        queue_cfg.low_class.min_threshold  = 0;   
        queue_cfg.low_class.max_threshold  = 0;   
        queue_cfg.high_class.min_threshold = 0;
        queue_cfg.high_class.max_threshold = 0;
    }

    ret = rdpa_egress_tm_queue_cfg_set(queue_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_set failed, if_id[%d], qid[%d], ret[%d]", if_id, queue_id, ret);
        return TMCTL_ERROR;
    }   

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_RdpaGetQueueStats(rdpa_drv_ioctl_dev_type rdpa_dev,
                          tmctl_devType_e dev_type,
                          unsigned int if_id,
                          int queue_id, 
                          tmctl_queueStats_t *stats_p)
{
    bdmf_object_handle root_tm = BDMF_NULL;
    bdmf_object_handle queue_tm = BDMF_NULL;
    rdpa_tm_queue_index_t q_stat_index = {0};
    rdpa_stat_1way_t queue_stat = {0};
    int ret;
    int idx;
    
    q_stat_index.channel = if_id - rdpa_if_wan1;
    q_stat_index.queue_id = queue_id; 

    get_root_tm(dev_type, if_id, &root_tm);
    if (!root_tm)
    {
        tmctl_error("Failed to get egress_tm, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }

    ret = get_queue_tm(root_tm, queue_id, &queue_tm, &idx);
    if (ret)
    {
        tmctl_error("get_queue_tm failed, qid[%d]", queue_id);
        return TMCTL_ERROR;
    } 
    
    ret = rdpa_egress_tm_queue_stat_get(queue_tm, &q_stat_index, &queue_stat);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_stat_get failed, qid[%d], ret[%d]", queue_id, ret);
        return TMCTL_ERROR;
    }

    stats_p->txPackets = queue_stat.passed.packets;
    stats_p->txBytes = queue_stat.passed.bytes;
    stats_p->droppedPackets = queue_stat.discarded.packets;
    stats_p->droppedBytes = queue_stat.discarded.bytes;

    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_RdpaSetQueueSize(rdpa_drv_ioctl_dev_type rdpa_dev,        
                                   tmctl_devType_e dev_type,
                                   int if_id,
                                   int queue_id,
                                   int size)
{
    bdmf_object_handle root_tm = BDMF_NULL;
    bdmf_object_handle queue_tm = BDMF_NULL;
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    int idx;
    int ret;

    get_root_tm(dev_type, if_id, &root_tm);
    if (!root_tm)
    {
        tmctl_error("Failed to get egress_tm, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }

    ret = get_queue_tm(root_tm, queue_id, &queue_tm, &idx);
    if (ret)
    {
        tmctl_error("get_queue_tm failed, qid[%d]", queue_id);
        return TMCTL_ERROR;
    } 
    
    /* if singel level check if not reserved queue */
    if ((root_tm == queue_tm) && is_queue_index_resereved(dev_type, idx))
    {
        tmctl_error("queue is resereved");
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_queue_cfg_get(queue_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_get failed, qid[%d], ret[%d]", queue_id, ret);
        return TMCTL_ERROR;
    }

    queue_cfg.drop_threshold = getDeviceQueueSize(size);
    
    ret = rdpa_egress_tm_queue_cfg_set(queue_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_set failed, qid[%d], ret[%d]", queue_id, ret);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

static int set_queue_rl_single_level(bdmf_object_handle egress_tm, 
                                     int idx, 
                                     tmctl_shaper_t *shaper_p)
{
    rdpa_tm_queue_cfg_t queue_cfg = {0};
    rdpa_tm_rl_rate_mode rl_mode;
    int ret;

    ret = rdpa_egress_tm_queue_cfg_get(egress_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    ret = rdpa_egress_tm_rl_rate_mode_get(egress_tm, &rl_mode);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_rl_rate_mode_get failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    queue_shaper_cfg_set(rl_mode, shaper_p, &queue_cfg);
    ret = rdpa_egress_tm_queue_cfg_set(egress_tm, idx, &queue_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_queue_cfg_set failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}


tmctl_ret_e tmctl_RdpaSetQueueShaper(rdpa_drv_ioctl_dev_type rdpa_dev,        
                                   tmctl_devType_e dev_type,
                                   int if_id,
                                   int queue_id,
                                   tmctl_shaper_t *shaper_p)
{
    bdmf_object_handle egress_tm = BDMF_NULL;
    bdmf_object_handle root_tm = BDMF_NULL;
    tmctl_portTmParms_t tm ={0};
    int idx;
    int ret;

    get_port_tm_caps(getDir(rdpa_dev, if_id), dev_type, if_id, &tm);
    if (!tm.queueShaper)
    {
        tmctl_error("Queue shaper is not supported on this port");
        return TMCTL_ERROR;
    }

    get_root_tm(dev_type, if_id, &root_tm);
    if (!root_tm)
    {
        tmctl_error("Failed to get egress_tm, if_id[%d], qid[%d]", if_id, queue_id);
        return TMCTL_ERROR;
    }

    ret = get_queue_tm(root_tm, queue_id, &egress_tm, &idx); 
    if (ret)
    {
        tmctl_error("get_queue_tm failed, qid[%d]", queue_id);
        return TMCTL_ERROR;
    }

    /* if singel level check if not reserved queue */
    if ((root_tm == egress_tm) && is_queue_index_resereved(dev_type, idx))
    {
        tmctl_error("queue is resereved");
        return TMCTL_ERROR;
    }

    if (root_tm == egress_tm)
        ret = set_queue_rl_single_level(egress_tm, idx, shaper_p);
    else
        ret = set_queue_rl_multi_level(root_tm, egress_tm, shaper_p);
    if (ret)
    {
        tmctl_error("Failed to set queue shpaer, qid[%d]", queue_id);
        return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS; 
}

tmctl_ret_e tmctl_RdpaGetPortTmParms(rdpa_drv_ioctl_dev_type rdpa_dev,        
                                    tmctl_devType_e dev_type,
                                    int if_id,
                                    tmctl_portTmParms_t *tm_parms)
{
    bdmf_object_handle egress_tm = BDMF_NULL;
    rdpa_tm_sched_mode mode;
    uint8_t num_queues;
    int ret;
    
    get_port_tm_caps(getDir(rdpa_dev, if_id), dev_type, if_id, tm_parms);

    ret = get_root_tm(dev_type, if_id, &egress_tm);
    if (ret)
    {
        tmctl_error("get_root_tm failed, if_id[%d], dev_type[%d]", if_id, dev_type);
        return TMCTL_ERROR; 
    }
    if (!egress_tm)
    {
        tmctl_debug("No egress_tm obj, if_id[%d]", if_id);
        tm_parms->cfgFlags = 0;
        return TMCTL_SUCCESS;
    }

    ret = rdpa_egress_tm_num_queues_get(egress_tm, &num_queues);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_num_queues_get failed, ret[%d]", ret);
        return TMCTL_ERROR; 
    }
    tm_parms->numQueues = num_queues;

    ret = get_tm_mode(egress_tm, &mode);
    if (ret)
    {
        tmctl_error("get_tm_mode failed, if_id[%d]", if_id);
        return TMCTL_ERROR; 
    }

    switch (mode)
    {
        case rdpa_tm_sched_sp_wrr:
            tm_parms->cfgFlags = TMCTL_SCHED_TYPE_SP_WRR;
            break;
        case rdpa_tm_sched_sp:
            tm_parms->cfgFlags = TMCTL_SCHED_TYPE_SP;
            break;
        case rdpa_tm_sched_wrr:
            tm_parms->cfgFlags = TMCTL_SCHED_TYPE_WRR;
            break;
        default:
            tmctl_error("Mode %d is not supported", mode);
            return TMCTL_ERROR;
    }

    return TMCTL_SUCCESS;
}

#if defined(BCM_PON)
static BOOL is_obj_linked(bdmf_object_handle bdmf_obj, bdmf_object_handle dev_obj)
{
    bdmf_link_handle obj_link = NULL;
    
    obj_link = bdmf_get_next_us_link(bdmf_obj, NULL);
    if (obj_link)
    {
        do {
            if (bdmf_us_link_to_object(obj_link) == dev_obj)
            {
                return TRUE;
            }
            obj_link = bdmf_get_next_us_link(bdmf_obj, obj_link);
        } while(obj_link);
    }

    /* compare DS links obj*/
    obj_link = bdmf_get_next_ds_link(bdmf_obj, NULL);
    if (obj_link)
    {
        do {
            if (bdmf_ds_link_to_object(obj_link) == dev_obj)
            {
                return TRUE;
            }
            obj_link = bdmf_get_next_ds_link(bdmf_obj, obj_link);
        } while(obj_link);
    }

    return FALSE;
}

static BOOL is_dscp_to_pbit_same_map(BOOL remark, bdmf_object_handle dscp_to_pbit_obj, 
                                        uint32_t dscp_pbit_map[])
{
    int i = 0, rc = BDMF_ERR_OK;
    rdpa_pbit dscp_pbit;
    bdmf_boolean qos_map;

    rdpa_dscp_to_pbit_qos_mapping_get(dscp_to_pbit_obj, &qos_map);
    if (qos_map == remark)
    {
        return FALSE;
    }

    for (i = 0; i < TOTAL_DSCP_NUM; i++)
    {
        /* masked, ignore it */
        if (dscp_pbit_map[i] == U32_MAX)
            continue;

        rc = rdpa_dscp_to_pbit_dscp_map_get(dscp_to_pbit_obj, i, &dscp_pbit);
        if (0 != rc)
        {
            tmctl_error(
                "rdpa_pbit_to_queue_pbit_map_get() failed:" \
                " pbit(%u) rc(%d)", i, rc);
            return FALSE;
        }
        
        if (dscp_pbit != dscp_pbit_map[i])
            break;
    }

    if (i == TOTAL_DSCP_NUM)
    {
        return TRUE;
    }

    return FALSE;
}

static int create_dscp_to_pbit_table(BOOL remark, bdmf_object_handle * d_to_p_obj)
{
    int rc = BDMF_ERR_OK;
    bdmf_mattr_handle mattr;

    mattr = bdmf_mattr_alloc(rdpa_dscp_to_pbit_drv());
    
    if (!remark)
    {
        rdpa_dscp_to_pbit_qos_mapping_set(mattr, TRUE);
        rdpa_dscp_to_pbit_table_set(mattr, 0);
    }

    rc = bdmf_new_and_set(rdpa_dscp_to_pbit_drv(), NULL, mattr, d_to_p_obj);
    if (rc)
    {
        tmctl_error("create_dscp_to_pbit_table failed: rc(%d)", rc);
    }

    return rc;
}

static int set_dscp_to_pbit_map(bdmf_object_handle d_to_p_obj, 
                                        uint32_t dscp_pbit_map[])
{
    int i = 0, rc = BDMF_ERR_OK;
    
    for (i = 0; i < TOTAL_DSCP_NUM; i++)
    {
        if (dscp_pbit_map[i] == U32_MAX)
            continue;
        
        rc = rdpa_dscp_to_pbit_dscp_map_set(
            d_to_p_obj, i, dscp_pbit_map[i]);
        
        if (rc)
        {
            tmctl_error("rdpa_dscp_to_pbit_dscp_map_set()" \
                " failed: dscp(%u) pbit(%u) rc(%d)", \
                i, dscp_pbit_map[i], rc);
            break;
        }
    }

    return rc;
}

tmctl_ret_e tmctl_RdpaGetDscpToPbit(rdpa_drv_ioctl_dev_type rdpa_dev,
                        int if_id, tmctl_dscpToPbitCfg_t* cfg_p)
{
   bdmf_object_handle d_to_p_obj = NULL;
   bdmf_object_handle dev_obj = NULL;
   int tbl_id, rc = BDMF_ERR_OK, i;
   BOOL qos_map = FALSE, tbl_found = FALSE;
   rdpa_pbit dscp_map;

   for (tbl_id = 0; tbl_id < RDPA_DSCP_TO_PBIT_MAX_TABLES; tbl_id++)
    {
        rc = rdpa_dscp_to_pbit_get(tbl_id, &d_to_p_obj);
        if (rc)
            continue;
        
        rc = rdpa_dscp_to_pbit_qos_mapping_get(d_to_p_obj, &qos_map);
        if (!rc)
        {
          /* qos map only can use table 0 */
           if (qos_map && !cfg_p->remark && (tbl_id==0))
            {
                tbl_found = TRUE;
            }
            else if (cfg_p->remark && !qos_map)
            {
                if (cfg_p->devType == TMCTL_DEV_NONE)
                    tbl_found = TRUE;
                else
                {
                    if (!(get_tm_owner(rdpa_dev, if_id, &dev_obj)))
                    {
                        if (is_obj_linked(d_to_p_obj, dev_obj))
                            tbl_found = TRUE;
                        bdmf_put(dev_obj);
                    }
                }
            }
            if (tbl_found == TRUE)
                break;
        }
        else
        {
            tmctl_error("rdpa_dscp_to_pbit_qos_mapping_get()" \
                " failed: table(%u) rc(%d)", tbl_id, rc);
         }

        bdmf_put(d_to_p_obj);
        d_to_p_obj = NULL;
    }

    if (tbl_found)
    {
        for (i = 0; i < TOTAL_DSCP_NUM; i++)
        {
            rc = rdpa_dscp_to_pbit_dscp_map_get(d_to_p_obj, i, &dscp_map);
            
            if (rc)
            {
                tmctl_error("rdpa_dscp_to_pbit_dscp_map_get()" \
                    " failed: table(%u) dscp(%u) rc(%d)", tbl_id, i, rc);
                break;
            }
            else
            {
                cfg_p->dscp[i]= dscp_map;
            }
        }
    }

    if (d_to_p_obj)
        bdmf_put(d_to_p_obj);

    if (rc == BDMF_ERR_NOENT)
    {
        tmctl_debug("no table found, ignore this error\n");
        rc  = BDMF_ERR_OK;
    }
    
    if (rc)
        return TMCTL_ERROR;

    return TMCTL_SUCCESS;
} 


tmctl_ret_e tmctl_RdpaSetDscpToPbit(rdpa_drv_ioctl_dev_type rdpa_dev,
                        int if_id, tmctl_dscpToPbitCfg_t* cfg_p)
{
    bdmf_object_handle d_to_p_obj = NULL;
    bdmf_object_handle dev_obj = NULL;
    bdmf_boolean qos_map;
    int tbl_id, rc = BDMF_ERR_OK;

    if (cfg_p->remark == 0)
    {
        /* only table 0 is used as qos table */
        rc = rdpa_dscp_to_pbit_get(0, &d_to_p_obj);
        if (!rc)
        {
            rdpa_dscp_to_pbit_qos_mapping_get(d_to_p_obj, &qos_map);
            if (!qos_map)
            {
                bdmf_put(d_to_p_obj);
                bdmf_destroy(d_to_p_obj);
                rc = BDMF_ERR_NOENT;
            }
            else
            {
               rc = set_dscp_to_pbit_map(d_to_p_obj, cfg_p->dscp);
            }
        }

        if (rc == BDMF_ERR_NOENT)
        {
            rc = create_dscp_to_pbit_table(0, &d_to_p_obj);
            if (!rc)
            {
                set_dscp_to_pbit_map(d_to_p_obj, cfg_p->dscp);
                d_to_p_obj = NULL;
            }
        }
    }
    else
    {
        bdmf_number tbl_id_avail = -1;
        BOOL link_check =  TRUE;

        for (tbl_id = 0; tbl_id < RDPA_DSCP_TO_PBIT_MAX_TABLES; tbl_id++)
        {
            rc = rdpa_dscp_to_pbit_get(tbl_id, &d_to_p_obj);
            if (rc)
                continue;

            rdpa_dscp_to_pbit_qos_mapping_get(d_to_p_obj, &qos_map);
            if (qos_map)
                continue;
            
            /* override all valid tables */
            if (cfg_p->devType == TMCTL_DEV_NONE)
            {
                set_dscp_to_pbit_map(d_to_p_obj, cfg_p->dscp);
                if (tbl_id_avail == -1)
                    tbl_id_avail = tbl_id;
            }
            else
            {
                /* XXX: BUG? dev_obj is set to NULL */
                /* unlink current table */
                if (link_check && is_obj_linked(d_to_p_obj, dev_obj))
                {
                    bdmf_unlink(d_to_p_obj, dev_obj);
                    link_check = FALSE;
                }

                if ((tbl_id_avail == -1) && is_dscp_to_pbit_same_map(1, d_to_p_obj,cfg_p->dscp))
                    tbl_id_avail = tbl_id;
            }

            bdmf_put(d_to_p_obj);
            d_to_p_obj = NULL;
            rc = BDMF_ERR_OK;
        }

        if (rc == BDMF_ERR_NOENT)
            rc = BDMF_ERR_OK;
        
        if (tbl_id_avail == -1)
        {
            rc = create_dscp_to_pbit_table(1, &d_to_p_obj);
            if (!rc)
            {
                set_dscp_to_pbit_map(d_to_p_obj, cfg_p->dscp);
                rdpa_dscp_to_pbit_table_get(d_to_p_obj, &tbl_id_avail);
            }
            d_to_p_obj = NULL;
        }
        
        if ((cfg_p->devType != TMCTL_DEV_NONE) && (tbl_id_avail != -1))
        {
            bdmf_object_handle tm_owner_obj = NULL;
            rc = get_tm_owner(rdpa_dev, if_id, &tm_owner_obj);
            if (!rc)
            {
                rc = rdpa_dscp_to_pbit_get(tbl_id_avail, &d_to_p_obj);
                if (!rc)
                    bdmf_link(d_to_p_obj, tm_owner_obj, NULL);
                else
                    tmctl_error("rdpa_dscp_to_pbit_get() failed rc(%d)", rc);
                bdmf_put(tm_owner_obj);
            }
            else
            {
                tmctl_error("get_tm_owner() failed dev(%d), if(%d), rc(%d)", rdpa_dev, if_id, rc);
            }
        }
     }
    
    /* XXX: BUG? dev_obj is set to NULL */
    if (dev_obj)
        bdmf_put(dev_obj);
    if (d_to_p_obj)
        bdmf_put(d_to_p_obj);

    if (rc)
        return TMCTL_ERROR;
    
    return TMCTL_SUCCESS;
}  

tmctl_ret_e tmctl_RdpaCreatePolicer(tmctl_policer_t *policer_p)
{
    bdmf_mattr_handle mattr = BDMF_NULL;
    bdmf_number index= 0;
    bdmf_object_handle policer = BDMF_NULL;
    rdpa_tm_policer_cfg_t policer_cfg = {0};
    int ret;
    
    tmctl_debug("dir[%d], id[%d], type[%d]", policer_p->dir, policer_p->policerId, policer_p->type);

    index = policer_p->policerId;
    policer_cfg.type = rdpa_tm_policer_single_token_bucket;

    switch (policer_p->type)
    {
        case TMCTL_POLICER_SINGLE_TOKEN_BUCKET:
            policer_cfg.type = rdpa_tm_policer_single_token_bucket;
            break;
#if defined(BCM_PON_XRDP) 			
        case TMCTL_POLICER_SINGLE_RATE_TCM:
            policer_cfg.type = rdpa_tm_policer_sr_overflow_dual_token_bucket;
            break;
        case TMCTL_POLICER_TWO_RATE_TCM:
            policer_cfg.type = rdpa_tm_policer_tr_dual_token_bucket;
            break;
        case TMCTL_POLICER_TWO_RATE_WITH_OVERFLOW:
            policer_cfg.type = rdpa_tm_policer_tr_overflow_dual_token_bucket;
            break;
#endif			
        default:
            tmctl_error("policer type %d is not supported", policer_p->type);
            return TMCTL_UNSUPPORTED;
    }
    policer_cfg.commited_rate = policer_p->cir;
    policer_cfg.committed_burst_size = policer_p->cbs;
    policer_cfg.peak_rate = policer_p->pir;
    policer_cfg.peak_burst_size = (policer_p->type == TMCTL_POLICER_SINGLE_RATE_TCM)?policer_p->ebs:policer_p->pbs;

    mattr = bdmf_mattr_alloc(rdpa_policer_drv());
    if (!mattr)
    {
        tmctl_error("bdmf_mattr_alloc failed");
        goto error;
    }

    if (TMCTL_INVALID_KEY != index)
    {
        ret = rdpa_policer_index_set(mattr, index);
        if (ret)
        {
            tmctl_error("rdpa_policer_index_set failed, ret[%d]", ret);
            goto error;
        }
    }
    
    ret = rdpa_policer_dir_set(mattr, policer_p->dir);
    if (ret)
    {
        tmctl_error("rdpa_policer_dir_set failed, ret[%d]", ret);
        goto error;
    }
    
    ret = rdpa_policer_cfg_set(mattr, &policer_cfg);
    if (ret)
    {
        tmctl_error("rdpa_policer_cfg_set failed, ret[%d]", ret);
        goto error;
    }
    
    ret = bdmf_new_and_set(rdpa_policer_drv(), NULL, mattr, &policer);
    mattr = BDMF_NULL;
    if (ret)
    {
        tmctl_error("bdmf_new_and_set failed to create policer obj, ret[%d]", ret);
        goto error;
    }
	
    if (TMCTL_INVALID_KEY == index)
    {
        ret = rdpa_policer_index_get(policer, &index);
        if (ret)
        {
            tmctl_error("rdpa_policer_index_get failed, ret[%d]", ret);
            goto error;
        }
		policer_p->policerId = index;
		
        tmctl_debug("return policer index as %d", policer_p->policerId);
    }
	
    return TMCTL_SUCCESS;
error:
    if (mattr)
        bdmf_mattr_free(mattr);
    if (policer)
        bdmf_destroy(policer);
    return TMCTL_ERROR;
}                                  

tmctl_ret_e tmctl_RdpaModifyPolicer(tmctl_policer_t *policer_p)
{
    rdpa_policer_key_t key = {0};
    bdmf_object_handle policer = BDMF_NULL;
    rdpa_tm_policer_cfg_t policer_cfg = {0};
    int ret;
    
    tmctl_debug("dir[%d], id[%d], type[%d]", policer_p->dir, policer_p->policerId, policer_p->type);
	
    key.dir = policer_p->dir;
    key.index = policer_p->policerId;
    
    ret = rdpa_policer_get(&key, &policer);
    if (ret)
    {
        tmctl_error("rdpa_policer_get failed, ret[%d]", ret);
        goto error;
    }
    ret = rdpa_policer_cfg_get(policer, &policer_cfg);
    if (ret)
    {
        tmctl_error("rdpa_policer_cfg_set failed, ret[%d]", ret);
        goto error;
    }
    policer_cfg.commited_rate = (policer_p->cir == TMCTL_INVALID_KEY)?policer_cfg.commited_rate:policer_p->cir;
    policer_cfg.committed_burst_size = (policer_p->cbs == TMCTL_INVALID_KEY)?policer_cfg.committed_burst_size:policer_p->cbs;
    policer_cfg.peak_rate = (policer_p->pir == TMCTL_INVALID_KEY)?policer_cfg.peak_rate:policer_p->pir;
    policer_cfg.peak_burst_size = (policer_cfg.type == rdpa_tm_policer_sr_overflow_dual_token_bucket)?
                                  (policer_p->ebs == TMCTL_INVALID_KEY?policer_cfg.peak_burst_size:policer_p->ebs):
                                  (policer_p->pbs == TMCTL_INVALID_KEY?policer_cfg.peak_burst_size:policer_p->pbs);
	
    ret = rdpa_policer_cfg_set(policer, &policer_cfg);
    if (ret)
    {
        tmctl_error("rdpa_policer_cfg_set failed, ret[%d]", ret);
        goto error;
    }
    tmctl_debug("cir[%d], cbs[%d]", policer_cfg.commited_rate, policer_cfg.commited_rate);
    
    bdmf_put(policer);    	
    return TMCTL_SUCCESS;
error:
    if (policer)
        bdmf_put(policer); 
    return TMCTL_ERROR;
}                                  

tmctl_ret_e tmctl_RdpaDeletePolicer(tmctl_dir_e dir, 
                                   int policerId)
{
    rdpa_policer_key_t key = {0};
    bdmf_object_handle policer = BDMF_NULL;
    int ret;
    
    tmctl_debug("dir[%d], policerId[%d]", dir, policerId);
    
    key.dir = dir;
    key.index = policerId;
    
    ret = rdpa_policer_get(&key, &policer);
    if (ret)
    {
        tmctl_debug("rdpa_policer_get failed, ret[%d]", ret);
        return TMCTL_SUCCESS;
    }
    
    bdmf_put(policer);
    
    ret = bdmf_destroy(policer);
    if (ret)
    {
        tmctl_error("bdmf_destroy failed, ret[%d]", ret);
        return TMCTL_ERROR;
    }
    policer = BDMF_NULL;
    return TMCTL_SUCCESS;
}

static int get_us_orl_tm(bdmf_object_handle *egress_tm_obj)
{
    rdpa_egress_tm_key_t tm_key = {};
    int ret;
    tm_key.dir   = rdpa_dir_us;
    tm_key.index = TMCTL_ORL_TM_id;

    ret = rdpa_egress_tm_get(&tm_key, egress_tm_obj);
    if (ret)
    {
        return TMCTL_NOT_FOUND;
    }
    
    return TMCTL_SUCCESS;
} 

static int setup_us_orl_tm(tmctl_shaper_t *shaper_p)
{
    int  ret;
    rdpa_tm_rl_cfg_t     rl_cfg        = {}; 
    bdmf_mattr_handle    mattr         = BDMF_NULL;
    bdmf_object_handle   egress_tm_obj = BDMF_NULL;
    
    mattr = bdmf_mattr_alloc(rdpa_egress_tm_drv());
    if (!mattr)
    {
        tmctl_error("bdmf_mattr_alloc failed");
        goto error;
    }

    ret = rdpa_egress_tm_dir_set(mattr, rdpa_dir_us);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_dir_set failed, ret[%d]", ret);
        goto error;
    }

    ret = rdpa_egress_tm_level_set(mattr, rdpa_tm_level_egress_tm);
        if (ret)
    {
        tmctl_error("rdpa_egress_tm_level_set failed, ret[%d]", ret);
        goto error;
    }
        
    ret = rdpa_egress_tm_mode_set(mattr, rdpa_tm_sched_disabled);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_mode_set failed, ret[%d]", ret);
        goto error;
    }
    
    ret = rdpa_egress_tm_overall_rl_set(mattr, TRUE);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_overall_rl_set failed, ret[%d]", ret);
        goto error;
    }

    ret = bdmf_new_and_set(rdpa_egress_tm_drv(), BDMF_NULL, mattr, &egress_tm_obj);
    mattr = BDMF_NULL;
    if (ret)
    {
        tmctl_error("bdmf_new_and_set failed to create egress_tm obj, ret[%d]", ret);
        goto error;
    }

    rl_cfg.af_rate = KBPS_TO_BPS(shaper_p->shapingRate);/* rate is in kbit/s: 1 kilobit = 1000 bits */
    rl_cfg.be_rate = 0;
    rl_cfg.burst_size = shaper_p->shapingBurstSize;

    ret = rdpa_egress_tm_rl_set(egress_tm_obj, &rl_cfg);
    if (ret)
    {
        tmctl_error("rdpa_egress_tm_rl_set failed, ret[%d]", ret);
        goto error;
    }
 
    return TMCTL_SUCCESS;
    
error:
    if (mattr)
        bdmf_mattr_free(mattr);
    if (egress_tm_obj)
        bdmf_destroy(egress_tm_obj);
    
    return TMCTL_ERROR;
}    

static BOOL is_linked(bdmf_object_handle tcont_obj, bdmf_object_handle egress_tm_obj)
{
    bdmf_link_handle link = BDMF_NULL;

    while ((link = bdmf_get_next_us_link(tcont_obj, link)))
    {
        if (bdmf_us_link_to_object(link) == egress_tm_obj)
            return 1;
    }

    return 0;
}

tmctl_ret_e tmctl_RdpaSetOverAllShaper(tmctl_shaper_t *shaper_p)
{
    int ret;
    bdmf_object_handle   egress_tm_obj = BDMF_NULL;
    rdpa_tm_rl_cfg_t     rl_cfg        = {};  

    if (get_us_orl_tm(&egress_tm_obj) == TMCTL_SUCCESS)
    {
        if (!shaper_p->shapingRate) /*if rate=0 disable us overall shaper */
        {
            bdmf_put(egress_tm_obj);
            ret = bdmf_destroy(egress_tm_obj);
            if (ret)
            {
                tmctl_error("bdmf_destroy() failed ret(%d)\n",ret);
                return TMCTL_ERROR;
            }

            return TMCTL_SUCCESS;
        }
        else 
        {
            rl_cfg.af_rate = KBPS_TO_BPS(shaper_p->shapingRate);/*shaping_rate is in kbit/s: 1 kilobit = 1000 bits */
            rl_cfg.be_rate = 0;
            rl_cfg.burst_size = shaper_p->shapingBurstSize;
            
            ret = rdpa_egress_tm_rl_set(egress_tm_obj, &rl_cfg);
            if (ret)
            {
                tmctl_error("rdpa_egress_tm_rl_set() failed: af(%llu) ret(%d)\n",(uint64_t)rl_cfg.af_rate, ret);
                bdmf_put(egress_tm_obj);                
                return TMCTL_ERROR;
            }
        }

        bdmf_put(egress_tm_obj);
    }
    else 
    {
        if (!shaper_p->shapingRate)
        {
            return TMCTL_SUCCESS;
        }
        
        ret = setup_us_orl_tm(shaper_p);
        if (ret)
        {
            tmctl_error("tmctl_RdpaCreateUsOrlTm() failed: af(%llu) ret(%d)\n",(uint64_t)rl_cfg.af_rate, ret);
            return TMCTL_ERROR;
        }
    }
    
    return TMCTL_SUCCESS;
}

tmctl_ret_e tmctl_RdpaGetOverAllShaper(tmctl_shaper_t *shaper_p, uint32_t *tcont_map)
{   
    int                  rc;
    int                  ret           = TMCTL_SUCCESS;
    bdmf_object_handle   egress_tm_obj = BDMF_NULL;
    bdmf_number          tcont_id      = TMCTL_INVALID_KEY;   
    rdpa_tm_rl_cfg_t     rl_cfg        = {};
    bdmf_boolean         mgmt;
    bdmf_type_handle     tcont_drv     = rdpa_tcont_drv();
    bdmf_object_handle   tcont_obj     = BDMF_NULL;
    uint32_t             bit_map       = 0;
    
    shaper_p->shapingRate = 0;
    shaper_p->shapingBurstSize = 0;
    *tcont_map = 0;
    
    ret = get_us_orl_tm(&egress_tm_obj);
    if (ret)
    {
        return ret;
    }

    rc = rdpa_egress_tm_rl_get(egress_tm_obj, &rl_cfg);
    if (rc)
    {
        tmctl_error("rdpa_egress_tm_rl_get() failed: rc(%d)\n", rc);
        ret = TMCTL_ERROR;
        goto exit;
    }

    shaper_p->shapingRate = BPS_TO_KBPS(rl_cfg.af_rate);
    shaper_p->shapingBurstSize = rl_cfg.burst_size;

    /* get linked object */
    while ((tcont_obj = bdmf_get_next(tcont_drv, tcont_obj, NULL)))
    {
        rc = rdpa_tcont_management_get(tcont_obj, &mgmt);
        if (rc)
        {
            tmctl_error("rdpa_tcont_management_get() failed: rc(%d)\n",rc);
            ret = TMCTL_ERROR;
            continue;
        }

        if (mgmt)
            continue;

        if (is_linked(tcont_obj, egress_tm_obj))
        {
            rc = rdpa_tcont_index_get(tcont_obj, &tcont_id);
            if (rc)
            {
                tmctl_error("get_us_orl_tm()failed: rc(%d)\n",rc);
                ret = TMCTL_ERROR;
                continue;
            }
            
            bit_map |= (1U << ((uint32_t)tcont_id - 1));  /*RDPA_OMCI_TCONT_ID = 0, data tcont id begin from 1*/
        }      
    }

    *tcont_map = bit_map;

exit:
    if (tcont_obj)
        bdmf_put(tcont_obj);
        
    if (egress_tm_obj)
        bdmf_put(egress_tm_obj);
    
    return ret;
}

tmctl_ret_e tmctl_RdpaLinkOverAllShaper(int tcont_id, BOOL do_link)
{
    int                  rc;
    int                  ret           = TMCTL_SUCCESS;
    bdmf_boolean         mgmt;
    bdmf_type_handle     tcont_drv     = rdpa_tcont_drv();
    bdmf_object_handle   tcont_obj     = BDMF_NULL;
    bdmf_object_handle   egress_tm_obj = BDMF_NULL;
    bdmf_number          tmp_id;
	
    ret = get_us_orl_tm(&egress_tm_obj);
    if (ret)
    {
        return ret;
    }

    if (tcont_id == TMCTL_ALL_TCONT_ID)
    {
          /* link all data TCONTs */
        while ((tcont_obj = bdmf_get_next(tcont_drv, tcont_obj, BDMF_NULL)))
        {
            rc = rdpa_tcont_management_get(tcont_obj, &mgmt);
            if (rc)
            {
                tmctl_error("rdpa_tcont_management_get() failed: rc(%d)\n",rc);
                ret = TMCTL_ERROR;
                continue;
            }

            if (mgmt)
                continue;

            if (is_linked(tcont_obj, egress_tm_obj) != do_link)
            {
                if (do_link)
                    rc = bdmf_link(tcont_obj, egress_tm_obj, NULL);
                else 
                    rc = bdmf_unlink(tcont_obj, egress_tm_obj);
                
                if (rc)
                {
                    rdpa_tcont_index_get(tcont_obj, &tmp_id);
                    tmctl_error("%s tcont(%d) failed: rc(%d)\n", do_link ? "bdmf_link()" : "bdmf_unlink()",  tmp_id, rc);
                    ret = TMCTL_ERROR;
                }
            }
        }
    }
    else         /* link the data TCONTs */
    {
        rc = rdpa_tcont_get((bdmf_number)tcont_id, &tcont_obj);
        if (rc)
        {
            tmctl_error("rdpa_tcont_get() failed: rc(%d)\n", rc);
            ret = TMCTL_ERROR;
            goto exit;
        }
        
        rc = rdpa_tcont_management_get(tcont_obj, &mgmt);
        if (rc)
        {
            tmctl_error("get_us_orl_tm()failed: rc(%d)\n",rc);
            ret = TMCTL_ERROR;
            goto exit;
        }
        
        if (!mgmt)
        {
            if (is_linked(tcont_obj, egress_tm_obj) != do_link)
            {
                if (do_link)
                    rc = bdmf_link(tcont_obj, egress_tm_obj, NULL);
                else 
                    rc = bdmf_unlink(tcont_obj, egress_tm_obj);

                if (rc)
                {
                    tmctl_error("%s tcont(%d) failed: rc(%d)\n", do_link ? "bdmf_link()" : "bdmf_unlink()",  tcont_id, rc);
                    ret = TMCTL_ERROR;
                    goto exit;
                }
            }
        }
    }
    
exit:
    if (tcont_obj)
        bdmf_put(tcont_obj);
    
    if (egress_tm_obj)
        bdmf_put(egress_tm_obj);
    
    return ret;
}
#endif
