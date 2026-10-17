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
#include "score_archer.h"
#include "bcm_score.h"
#include "archer_gpl_hooks.h"
#include "score_qstat.h" /* TODO - find way to avoid this inclusion */
#include "score_int.h"
#include "bcm_async_queue.h"

#define SCORE_ARCHER_MAX_DHD_RADIO_IDX  4   /* Radio Idx, NOT max num of supported DHD Radios */
static bcm_async_queue_t *score_qstat_wlan_dhd_queue[SCORE_ARCHER_MAX_DHD_RADIO_IDX];

/* Callback function to push DHD latency record to async queue */
static int score_qstat_wlan_dhd_write(void *dhd_rec)
{
    score_hw_archer_qstat_data_dhd_t *rec_p;  /* Input Record in hardware layout */
    struct wlan_dhd_tlm_info *info_p;           /* Output record in SCORE layout */
    static uint32_t score_record_seq_num = 0;
    uint16_t radio_idx;
    
    rec_p = (score_hw_archer_qstat_data_dhd_t*)dhd_rec;
    radio_idx = rec_p->radio_idx;
    score_record_seq_num++;

    if (radio_idx >= SCORE_ARCHER_MAX_DHD_RADIO_IDX ||
        score_qstat_wlan_dhd_queue[radio_idx] == NULL)
    {
        /* TODO : Any error counter to increase ?*/
        return 0;
    }
                                              
    if (bcm_async_queue_not_full (score_qstat_wlan_dhd_queue[radio_idx]))
    {
        score_qstat_data_t *entry_p = (score_qstat_data_t *)
            bcm_async_queue_entry_write (score_qstat_wlan_dhd_queue[radio_idx]);
        uint16_t latency;

        /* Not filling in timestamp for now */
        /* entry_p->timestamp = 0; */
        entry_p->type = QSTAT_TYPE_WLAN_DHD_INFO;
        entry_p->report_seq_num = score_record_seq_num;
        entry_p->score_stream_id = rec_p->score_stream_id;

        info_p = &entry_p->wlan_dhd;
        /* Assuming that both timestamp have same source in ms */
        info_p->radio_idx = radio_idx;
        info_p->fr_idx = rec_p->flow_ring_id;
        /* Timestamps from Archer/Crossbow are uint16_t
         * Simple subtraction and assignment to uint32_t won't work */
        latency = rec_p->egress_ts - rec_p->ingress_ts;
        info_p->pkt_latency = ARCHER_TIMESTAMP_2_MSEC(latency);
        latency = rec_p->flow_ring_ts - rec_p->backup_queue_ts;
        info_p->bq_latency = ARCHER_TIMESTAMP_2_MSEC(latency);
        info_p->bq_amount = rec_p->backup_queue_level;

        bcm_async_queue_entry_enqueue (score_qstat_wlan_dhd_queue[radio_idx]);
        score_qstat_wlan_dhd_queue[radio_idx]->stats.writes++;
    }
    else
    {
        score_qstat_wlan_dhd_queue[radio_idx]->stats.discards++;
    }

    return 0;
}

static int score_qstat_wlan_dhd_notify(void *arg_p)
{
    score_notify_async_q_data();

    return 0;
}

/* Initialize qstat for WLAN DHD */
static int qstat_init_wlan_dhd_queue(uint32_t radio_idx, uint32_t ring_size)
{
    /* Create Async Queue and initialize the global function pointer */
    int ret = 0;
    int queue_entry_size = sizeof(score_qstat_data_t);
    queue_entry_size = (queue_entry_size + 0x3) & ~3; /* Make entry 4-byte multiple */

    score_qstat_wlan_dhd_queue[radio_idx] = 
        (bcm_async_queue_t*)kmalloc(sizeof(bcm_async_queue_t), GFP_KERNEL);

    if (score_qstat_wlan_dhd_queue[radio_idx] == NULL)
    {
        __logError("WLAN DHD Queue mem alloc failed - %d\n", ring_size);
        return -1;
    }

    if ((ret = bcm_async_queue_init(score_qstat_wlan_dhd_queue[radio_idx], ring_size,
                                    queue_entry_size)) != 0)
    {
        __logError("WLAN DHD Queue Init Failed Q Size- %d\n", ring_size);
        return ret;
    }
    return ret;
}

static int qstat_uninit_wlan_dhd_queue(uint32_t radio_idx)
{
    int ret = 0;

    if (score_qstat_wlan_dhd_queue[radio_idx] == NULL)
    {
        return 0;
    }

    bcm_async_queue_uninit(score_qstat_wlan_dhd_queue[radio_idx]);
    kfree(score_qstat_wlan_dhd_queue[radio_idx]);
    score_qstat_wlan_dhd_queue[radio_idx] = NULL;

    return ret;
}

int score_profile_add(uint32_t *dcp_id, const score_profile_cfg_t *dcp_cfg)
{
    return archer_gpl_score_profile_add(dcp_id, dcp_cfg);
}

int score_profile_get(uint32_t dcp_id, score_profile_cfg_t *dcp_cfg)
{
    return archer_gpl_score_profile_get(dcp_id, dcp_cfg);
}

int score_profile_delete(uint32_t dcp_id)
{
    return archer_gpl_score_profile_delete(dcp_id);
}

int score_sdr_add(const score_sdr_info_t * sdr_)
{
    return 0;
}

int score_sdr_delete(uint32_t dcp_id)
{
    return 0;
}

int score_stream_add(uint32_t* stream_id, score_stream_info_t * stream_)
{
    return 0;
}

int score_stream_delete(uint32_t stream_id, uint8_t dcp_id)
{
    return 0;
}

int score_qstat_cfg_set(qstat_queue_info_t *queue_info, uint32_t enable)
{
    return 0;
}

int score_qstat_ring_create(uint32_t ring_size)
{
    /* This function is called during SCORE module init
     * For now, initialize the max possible radio indexes --- wastage of memory
     * Build a handshake with Archer/Crossbow to know about DHDs 
     * Another option is to build these queues when we get the first record */ 

    uint32_t radio_idx;

    for (radio_idx = 0; radio_idx < SCORE_ARCHER_MAX_DHD_RADIO_IDX; radio_idx++)
    {
        qstat_init_wlan_dhd_queue(radio_idx, ring_size);
    }
    /* Initialize the callback function */
    bcmFun_reg(BCM_FUN_ID_ARCHER_SCORE_QSTAT_DHD_WRITE, score_qstat_wlan_dhd_write);
    bcmFun_reg(BCM_FUN_ID_ARCHER_SCORE_QSTAT_DHD_NOTIFY, score_qstat_wlan_dhd_notify);
    return 0;
}

int score_qstat_ring_destroy(uint32_t ring_size)
{
    uint32_t radio_idx;

    for (radio_idx = 0; radio_idx < SCORE_ARCHER_MAX_DHD_RADIO_IDX; radio_idx++)
    {
        qstat_uninit_wlan_dhd_queue(radio_idx);
    }
    /* Uninitialize the callback function */
    bcmFun_dereg(BCM_FUN_ID_ARCHER_SCORE_QSTAT_DHD_WRITE);
    bcmFun_dereg(BCM_FUN_ID_ARCHER_SCORE_QSTAT_DHD_NOTIFY);
    return 0;
}

int score_qstat_ring_info_get(score_qstat_data_t *score_qstat_info)
{
    return -1; /* Return no data for now */
}

int score_get_wired_queue_info(blog_score_flow_info_t *score_flow_info, qstat_queue_info_t *queue_info)
{
    return -1;
}

/* Process DHD rings */
int process_qstat_ring(int bound)
{
    uint32_t radio_idx;
    int done = 1; /* Assume no records to process */
    for (radio_idx = 0; radio_idx < SCORE_ARCHER_MAX_DHD_RADIO_IDX; radio_idx++)
    {
        done &= process_qstat_async_q(score_qstat_wlan_dhd_queue[radio_idx], bound);
    }

    return done;
}

int score_qstats_asyncq_stats_get(int *idx_p, bcm_async_queue_stats_t *stats_p, 
                                  char *disp_str_p, int str_sz)
{
    /* Returns the valid rec index or the last index tried */
    int valid_rec = 0;
    if (*idx_p == 0) /* NIC */
    {
        valid_rec = 1;
        snprintf(disp_str_p, str_sz,"%s", "NIC  ");
        disp_str_p[str_sz-1] = '\0';
        qstat_get_nic_asyncq_stats(stats_p);
    }
    else /* DHD */
    {
        int radio_idx = (*idx_p) - 1; /* offset for nic & start with zero */
        for (; radio_idx >= 0 && radio_idx < SCORE_ARCHER_MAX_DHD_RADIO_IDX; radio_idx++)
        {
            if (score_qstat_wlan_dhd_queue[radio_idx]) /* DHD registered */
            {
                valid_rec = 1;
                qstat_get_asyncq_stats(score_qstat_wlan_dhd_queue[radio_idx], stats_p);
                snprintf(disp_str_p, str_sz,"%s#%d", "DHD",radio_idx);
                disp_str_p[str_sz-1] = '\0';
                break;
            }
        }
        (*idx_p) = radio_idx + 1; /* adjust for nic */
    }

    return valid_rec;
}

static bcm_async_queue_t score_hw_bnl_async_queue_g;

static int score_hw_archer_bnl_data_write(score_bnl_data_t *bnl_pkt_info)
{
    if(likely(bcm_async_queue_not_full(&score_hw_bnl_async_queue_g)))
    {
        score_bnl_data_t *bnl_data_p = (score_bnl_data_t *)
            bcm_async_queue_entry_write(&score_hw_bnl_async_queue_g);

        *bnl_data_p = *bnl_pkt_info;

        bcm_async_queue_entry_enqueue(&score_hw_bnl_async_queue_g);

        score_hw_bnl_async_queue_g.stats.writes++;

        return 0;
    }

    score_hw_bnl_async_queue_g.stats.discards++;

    return -1;
}

static void score_hw_archer_bnl_data_notify(void)
{
    score_hw_msg_callback(0);
}

int score_hw_bnl_ring_info_get(score_bnl_data_t *bnl_pkt_info)
{
    if(likely(bcm_async_queue_not_empty(&score_hw_bnl_async_queue_g)))
    {
        score_bnl_data_t *bnl_data_p = (score_bnl_data_t *)
            bcm_async_queue_entry_read(&score_hw_bnl_async_queue_g);

        *bnl_pkt_info = *bnl_data_p;

        bcm_async_queue_entry_dequeue(&score_hw_bnl_async_queue_g);

        score_hw_bnl_async_queue_g.stats.reads++;

        return 0;
    }

    return -1;
}
EXPORT_SYMBOL(score_hw_bnl_ring_info_get);

static inline void score_hw_archer_async_queue_dump(char *name,
                                                    bcm_async_queue_t *queue_p)
{
    bcm_print("%s: Level %u/%u, Writes %u, Reads %u, Discards %u, Writes+Discards %u\n",
              name, bcm_async_queue_avail_entries(queue_p), queue_p->depth,
              queue_p->stats.writes, queue_p->stats.reads, queue_p->stats.discards,
              queue_p->stats.writes + queue_p->stats.discards);
}

static void score_hw_archer_bnl_queue_dump(void)
{
    score_hw_archer_async_queue_dump("\tBNL", &score_hw_bnl_async_queue_g);
}

int score_hw_bnl_ring_create(uint32_t ring_size)
{
    archer_gpl_score_callbacks_t callbacks;
    int ret;

    bcm_print("SCORE BNL Ring Create: ring size %u\n", ring_size);

    ret = bcm_async_queue_init(&score_hw_bnl_async_queue_g,
                               ring_size, sizeof(score_bnl_data_t));
    if(ret)
    {
        __logError("Could not bcm_async_queue_init");

        return ret;
    }

    callbacks.bnl_data_write = score_hw_archer_bnl_data_write;
    callbacks.bnl_data_notify = score_hw_archer_bnl_data_notify;
    callbacks.bnl_queue_dump = score_hw_archer_bnl_queue_dump;

    archer_gpl_score_callbacks_bind(&callbacks);

    return 0;
}
EXPORT_SYMBOL(score_hw_bnl_ring_create);

int score_hw_bnl_ring_destroy(uint32_t ring_size)
{
    bcm_async_queue_uninit(&score_hw_bnl_async_queue_g);

    return 0;
}
EXPORT_SYMBOL(score_hw_bnl_ring_destroy);

extern int host_ts_factor;

static bcmFun_t *get_hw_ts_g;

static inline uint32_t score_qstat_get_host_ts(void)
{
    uint32_t hw_ts_in_us;

    if (!get_hw_ts_g) {
        bcmFun_t *get_hw_ts_info = bcmFun_get(BCM_FUN_ID_PKT_TLM_HW_TS_INFO_GET);
        if (get_hw_ts_info) {
            get_hw_ts_info(&hw_ts_in_us);
            host_ts_factor = 1000 / hw_ts_in_us;  // how many ticks in one msec
            get_hw_ts_g = bcmFun_get(BCM_FUN_ID_PKT_TLM_HW_TS_GET);
        }
    }
    return (uint32_t) get_hw_ts_g(NULL);
}

static int score_qstat_wlan_nic_set_egress_ts(void *sob)
{
    score_hw_qstat_record_wlan_nic_t *rec_p;

    rec_p = (score_hw_qstat_record_wlan_nic_t*)sob;
    rec_p->egress_ts = score_qstat_get_host_ts();
    return 0;
}

int score_msg_ring_create(uint32_t ring_size)
{
    /* For crossbow, there is no msg ring */
    return 0;
}

int score_msg_ring_destroy(uint32_t ring_size)
{
    return 0;
}

int score_msg_ring_info_get(score_data_t *score_msg_info)
{
    uint32_t radio_idx;

    score_msg_info->pending_msg_bitmask = 0;
    score_msg_info->irq_cnt = 0;

    if(likely(bcm_async_queue_not_empty(&score_hw_bnl_async_queue_g)))
    {
        score_msg_info->pending_msg_bitmask |= (1 << SCORE_MSG_BNL_ID);
    }

    for(radio_idx=0; radio_idx<SCORE_ARCHER_MAX_DHD_RADIO_IDX; ++radio_idx)
    {
        bcm_async_queue_t *queue_p = score_qstat_wlan_dhd_queue[radio_idx];

        if(queue_p)
        {
            if(likely(bcm_async_queue_not_empty(queue_p)))
            {
                score_msg_info->pending_msg_bitmask |= (1 << SCORE_MSG_QSTAT_ID);
            }
        }
    }

    return (score_msg_info->pending_msg_bitmask) ? 0 : -1;
}

int score_hw_init(void)
{
    memset(&score_hw_bnl_async_queue_g, 0, sizeof(bcm_async_queue_t));

    bcm_score_qstat_wlan_nic_set_egress_ts = score_qstat_wlan_nic_set_egress_ts;

    return 0;
}

extern int default_wlan_nic_set_egress_ts(void *sob);
void score_hw_destroy(void)
{
    bcm_score_qstat_wlan_nic_set_egress_ts = default_wlan_nic_set_egress_ts;
    return;
}

