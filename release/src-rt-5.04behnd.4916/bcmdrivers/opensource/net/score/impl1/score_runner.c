/*
    <:copyright-BRCM:2024:DUAL/GPL:standard

    Copyright (c) 2024 Broadcom
    All Rights Reserved

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
 */

#include <linux/types.h>
#include "score_int.h"
#include "bcm_score.h"
#include "score_runner.h"
#include "score_qstat.h"
#include <rdpa_mw_blog_parse.h>

#if (HZ != 1000)
#define USE_BCM_EXT_TIMER
#endif

#if defined(USE_BCM_EXT_TIMER)
#include "bcm_ext_timer.h"
static EXT_TIMER_NUMBER ext_tmr_idx = EXT_TIMER_INVALID;
#endif

#define CALIBRATE_TIME_SHIFT    16    // 1.09min

#if defined(SCORE_SUPPORT)
static uint32_t last_calibrated_ts;
static uint16_t host_ts_offset;
#endif

score_hw_t score_hw_g = {  .score_class = NULL };

#if defined(SCORE_SUPPORT)
static int _score_qstat_wired_cfg_set(qstat_queue_info_t *queue_info, uint32_t enable)
{
    rdpa_tm_queue_cfg_t queue_cfg;
    int rc;

    rc = rdpa_egress_tm_queue_cfg_get(queue_info->egress_tm_obj, queue_info->queue_index, &queue_cfg);
    if (rc)
    {
        __logError("unable to get egress tm queue object");
        return BDMF_ERR_INTERNAL;
    }

    queue_cfg.qstat_enable = !!enable;

    rc = rdpa_egress_tm_queue_cfg_set(queue_info->egress_tm_obj, queue_info->queue_index, &queue_cfg);
    if (rc)
    {
        __logError("unable to set egress tm queue");
    }

    return BDMF_ERR_OK;
}
#endif

int score_qstat_cfg_set(qstat_queue_info_t *queue_info, uint32_t enable)
{
#if defined(SCORE_SUPPORT)
    rdpa_qstat_cfg_data_t qstat_cfg_data = {.u32 = 0};

    if (queue_info->if_type == IF_WLAN_DHD)
    {
        qstat_cfg_data.type = queue_info->if_type;
        qstat_cfg_data.radio_idx = queue_info->if_id;
        qstat_cfg_data.flowring_idx = queue_info->queue_id;
        qstat_cfg_data.enable = enable;

        return rdpa_score_qstat_cfg_set(score_hw_g.score_class, (bdmf_number)qstat_cfg_data.u32);
    }
    else if (queue_info->if_type == IF_WIRED)
    {
        return _score_qstat_wired_cfg_set(queue_info, enable);
    }

    return BDMF_ERR_OK;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

#define score_runner_ts_get(ts_p)   rdpa_score_ts_1us_get(ts_p)

int score_profile_add(uint32_t* dcp_id, const score_profile_cfg_t * profile_)
{
#if defined(SCORE_SUPPORT)
    bdmf_index ai;
    int rc;
    rc = rdpa_score_profile_add(score_hw_g.score_class, &ai, profile_);

    return rc;
#else
    return BDMF_ERR_OK;
#endif
}

int score_profile_get(uint32_t dcp_id, score_profile_cfg_t * dcp_cfg)
{
#if defined(SCORE_SUPPORT)
    bdmf_index ai = (bdmf_index)dcp_id;
    int rc;
    rc = rdpa_score_profile_get(score_hw_g.score_class, ai, dcp_cfg);
    return rc;
#else
    return BDMF_ERR_OK;
#endif
}

int score_sdr_add(const score_sdr_info_t * sdr_)
{
#if defined(SCORE_SUPPORT)
    bdmf_index ai;
    int rc;
    rc = rdpa_score_sdr_add(score_hw_g.score_class, &ai, sdr_);

    return rc;
#else
    return BDMF_ERR_OK;
#endif
}

int score_sdr_delete(uint32_t dcp_id)
{
#if defined(SCORE_SUPPORT)
    bdmf_index ai = (bdmf_index)dcp_id;
    int rc;
    rc = rdpa_score_sdr_delete(score_hw_g.score_class, ai);
    return rc;
#else
    return BDMF_ERR_OK;
#endif
}

int score_profile_delete(uint32_t dcp_id)
{
#if defined(SCORE_SUPPORT)
    bdmf_index ai = (bdmf_index)dcp_id;
    int rc;
    rc = rdpa_score_profile_delete(score_hw_g.score_class, ai);
    return rc;
#else
    return BDMF_ERR_OK;
#endif
}

int score_stream_add(uint32_t* stream_id, score_stream_info_t * stream_cfg)
{
#if defined(SCORE_SUPPORT)
    bdmf_index ai = (bdmf_index)*stream_id;
    uint16_t add_dcp_bitmask;
    int rc;
    rc = rdpa_score_stream_find(score_hw_g.score_class, &ai, stream_cfg);
    if (rc == BDMF_ERR_NOENT)
    {
        /* not found, add new stream */
        rc = rdpa_score_stream_add(score_hw_g.score_class, &ai, stream_cfg);
    }
    else
    {
        /* add dcp and update */
        add_dcp_bitmask = stream_cfg->cfg.dcp_bitmask;
        rc = rc ? rc : rdpa_score_stream_get(score_hw_g.score_class, ai, stream_cfg);
        stream_cfg->cfg.dcp_bitmask |= add_dcp_bitmask;
        rc = rc ? rc : rdpa_score_stream_set(score_hw_g.score_class, ai, stream_cfg);
    }

    return rc;
#else
    return BDMF_ERR_OK;
#endif
}

int score_stream_delete(uint32_t stream_id, uint8_t dcp_id)
{
#if defined(SCORE_SUPPORT)
    bdmf_index ai = (bdmf_index)stream_id;
    int rc;
    score_stream_info_t stream_cfg={};

    stream_cfg.id = stream_id;
    stream_cfg.cfg.dcp_bitmask = dcp_id;

    rc = rdpa_score_stream_find(score_hw_g.score_class, &ai, &stream_cfg);
    /* get old dcp bitmask */
    rc = rc ? rc : rdpa_score_stream_get(score_hw_g.score_class, ai, &stream_cfg);
    CLR_BIT(stream_cfg.cfg.dcp_bitmask, dcp_id);
    if (stream_cfg.cfg.dcp_bitmask)
    {
        /* just update dcp bitmask */
        rc = rc ? rc : rdpa_score_stream_set(score_hw_g.score_class, ai, &stream_cfg);
    }
    else
    {
        rc = rc ? rc : rdpa_score_stream_delete(score_hw_g.score_class, ai);
    }
    return rc;
#else
    return BDMF_ERR_OK;
#endif
}

int score_qstat_ring_info_get(score_qstat_data_t *score_qstat_info)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_qstat_ring_info_get(score_qstat_info);
#else
    return BDMF_ERR_NO_MORE;
#endif
}

int score_msg_ring_info_get(score_data_t *score_msg_info)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_msg_ring_info_get(score_msg_info);
#else
    return BDMF_ERR_NO_MORE;
#endif
}

int score_msg_ring_create(uint32_t ring_size)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_msg_ring_create(ring_size);
#else
    return BDMF_ERR_OK;
#endif
}
int score_qstat_ring_create(uint32_t ring_size)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_qstat_ring_create(ring_size);
#else
    return BDMF_ERR_OK;
#endif
}
int score_msg_ring_destroy(uint32_t ring_size)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_msg_ring_destroy(ring_size);
#else
    return BDMF_ERR_OK;
#endif
}
int score_qstat_ring_destroy(uint32_t ring_size)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_qstat_ring_destroy(ring_size);
#else
    return BDMF_ERR_OK;
#endif
}

int score_qstats_asyncq_stats_get(int *idx_p, bcm_async_queue_stats_t *stats_p, 
                                  char *disp_str_p, int str_sz)
{
#if defined(SCORE_SUPPORT)
    /* Returns the valid rec index or the last index tried */
    int valid_rec = 0;
    if (*idx_p == 0) /* NIC */
    {
        valid_rec = 1;
        snprintf(disp_str_p, str_sz,"%s", "NIC");
        disp_str_p[str_sz-1] = '\0';
        qstat_get_nic_asyncq_stats(stats_p);
    }
    /* DHD qstat for runner not using async queue */
    return valid_rec;
#else
    return 0;
#endif
}

int process_qstat_ring(int bound)
{
#if defined(SCORE_SUPPORT)
    int pkts = 0, done = 0;
    int rc = 0;
    score_qstat_data_t score_qstat_info;

    while (!bound || (pkts < bound))
    {
        /* Fetch next message */
        rc = score_qstat_ring_info_get(&score_qstat_info);

        if (rc)
        {
            done = 1;
            break;
        }

        qstat_process_data(&score_qstat_info);
        qstat_procfs_entry_queue(&score_qstat_info);

        if (score_prints_enable_g)
        {
             __print("SCORE qstat_runner_ring: ts : 0x%x, seq_num : 0x%x, stream_id: %d, type : 0x%x\n",
                        score_qstat_info.timestamp,
                        score_qstat_info.report_seq_num,
                        score_qstat_info.score_stream_id,
                        score_qstat_info.type);
        }
        pkts++;
    }

    return done;
#else
    return 1;
#endif
}

int score_hw_bnl_ring_create(uint32_t ring_size)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_bnl_ring_create(ring_size);
#else
    return BDMF_ERR_OK;
#endif
}
EXPORT_SYMBOL(score_hw_bnl_ring_create);

int score_hw_bnl_ring_destroy(uint32_t ring_size)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_bnl_ring_destroy(ring_size);
#else
    return BDMF_ERR_OK;
#endif
}
EXPORT_SYMBOL(score_hw_bnl_ring_destroy);

int score_hw_bnl_ring_info_get(score_bnl_data_t *bnl_pkt_info)
{
#if defined(SCORE_SUPPORT)
    return rdpa_score_bnl_ring_info_get(bnl_pkt_info);
#else
    return BDMF_ERR_NO_MORE;
#endif
}
EXPORT_SYMBOL(score_hw_bnl_ring_info_get);

#if defined(USE_BCM_EXT_TIMER) && defined(SCORE_SUPPORT)
static inline void score_qstat_get_host_ts_init(void)
{
    if (ext_tmr_idx == EXT_TIMER_INVALID)
    {
        ext_tmr_idx = ext_timer_alloc_freerun(-1);
        if (ext_tmr_idx == EXT_TIMER_INVALID)
        {
            __logError("can't allocate bcm_ext_timer!!");
            return;
        }
        __logDebug("alloc_timer_idx=%d", ext_tmr_idx);
    }
}
#else // use jiffies
#define score_qstat_get_host_ts_init()
#endif // !USE_BCM_EXT_TIMER

#if defined(SCORE_SUPPORT)
static int score_qstat_wlan_nic_set_egress_ts(void *sob)
{
#if defined(USE_BCM_EXT_TIMER)
    uint32_t cur_host_ts = ext_timer_read_32b_ms(ext_tmr_idx);
#else
    uint32_t cur_host_ts = (uint32_t) jiffies;
#endif
    score_hw_qstat_record_wlan_nic_t *rec_p = (score_hw_qstat_record_wlan_nic_t*)sob;

    cur_host_ts = MS_TO_US128(cur_host_ts);
    if (!host_ts_offset || (cur_host_ts - last_calibrated_ts) >> CALIBRATE_TIME_SHIFT)
    {
        uint32_t hw_ts=0;
        // get ingress hw_ts to compute offset between host_ts and hw_ts
        score_runner_ts_get(&hw_ts);
        hw_ts >>= 7;  // originally in 1usec convert to 128usec
        host_ts_offset = (uint16_t)cur_host_ts - (uint16_t)hw_ts;

        last_calibrated_ts = cur_host_ts;
    }

    cur_host_ts -= host_ts_offset;
    // host is in msec unit, ingress_ts in 128usec unit, 
    // potentially host ts can be before ingress_ts, check for this condition, and adjust
    if ((rec_p->ingress_ts - (uint16_t) cur_host_ts) <= 8)
        cur_host_ts += 8;
    rec_p->egress_ts = (uint16_t) cur_host_ts;
    return 0;
}
#endif

int score_hw_init(void)
{
#if defined(SCORE_SUPPORT)
    int ret = 0;

    BDMF_MATTR_ALLOC(score_attrs, rdpa_score_drv());

    ret = rdpa_score_get(&score_hw_g.score_class);
    if (ret)
    {
        rdpa_score_score_msg_cb_set(score_attrs, (void *)score_hw_msg_callback);
        rdpa_score_score_irq_coalescing_num_set(score_attrs, 64);
        rdpa_score_score_irq_coalescing_timeout_set(score_attrs, 1);
        ret = bdmf_new_and_set(rdpa_score_drv(), NULL, score_attrs, &score_hw_g.score_class);
        if (ret)
        {
            __logError("rdpa SCORE object does not exist and can't be created.\n");
            goto exit;
        }
    }

    score_qstat_get_host_ts_init();
    bcm_score_qstat_wlan_nic_set_egress_ts = score_qstat_wlan_nic_set_egress_ts;

    __logDebug("SCORE Runner Layer Initialized\n");

exit:
    BDMF_MATTR_FREE(score_attrs);

    return ret;
#else
    return BDMF_ERR_OK;
#endif
}

extern int default_wlan_nic_set_egress_ts(void *sob);
void score_hw_destroy(void)
{
#if defined(SCORE_SUPPORT)
    if(score_hw_g.score_class)
    {

        bdmf_destroy(score_hw_g.score_class);
    }
    bcm_score_qstat_wlan_nic_set_egress_ts = default_wlan_nic_set_egress_ts;
#endif
}

int score_get_wired_queue_info(blog_score_flow_info_t *score_flow_info, qstat_queue_info_t *queue_info)
{
#if defined(SCORE_SUPPORT)
    bdmf_object_handle port_egress_obj, egress_tm_obj;
    uint16_t hw_queue_id;
    uint8_t queue_index;
    int rc;

    port_egress_obj = rdpa_mw_get_port_object_by_dev_dir(score_flow_info->tx_dev, RDPA_DIR_TX);
    if (!port_egress_obj)
    {
        __logError("unable to obtain egress port object");
        return -1;
    }

    rc = rdpa_port_get_hw_queue_info(port_egress_obj, score_flow_info->wan_flow, score_flow_info->queue_id, &egress_tm_obj, &queue_index, &hw_queue_id);
    if (rc)
    {
        __logError("unable to obtain unique queue identifier rc %d");
        return -1;
    }

    /* These are used are associated with a qstat as a unique identifier */
    queue_info->if_id = QSTAT_IF_WIRED_INDEX;
    queue_info->if_type = IF_WIRED;
    queue_info->queue_id = hw_queue_id;

    /* These are used later to enable qstat on the port/queue */
    queue_info->queue_index = queue_index;
    queue_info->egress_tm_obj = egress_tm_obj;

    return  0;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}
