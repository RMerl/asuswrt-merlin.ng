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

/* Broadcom QSTAT module*/
#include <linux/types.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>

#include "score_int.h"
#include "score_qstat.h"


#if defined(BCM6855) || defined(BCM63146) || defined(BCM4912) || defined(BCM6813)
#define QSTAT_WIRED_MAX_LATENCY_WA     (500)
#endif

uint32_t default_bin_cfg_limits[QSTAT_MAX_BIN_LIMITS] = {8, 16, 24, 32, 40, 48, 56};

static statistics_entry_t stats[QSTAT_MAX_STATS] = {};

/* Association of queue and qstat: interface + queue_id --> qstat_id
 * WLAN uses their interface ID (radio) and flow ring index.
 * WIRED uses QSTAT_IF_WIRED_INDEX and QM Queue Index.
 */
static uint16_t if_queue_id_to_qstat_id[QSTAT_MAX_IF_NUM][QSTAT_MAX_QUEUE_NUM];

/* stream_idx + encarn + dir */
static uint16_t score_stream_key_to_qstat_id[SCORE_NUM_OF_STREAM_KEYS];
qstat_entry_t qstat_db[QSTAT_MAX_QUEUES] = {};

score_qstat_cfg_report_t qstat_cfg_report[QSTAT_MAX_CFG_NUMBER] = {};

static uint32_t max_val(uint32_t a, uint32_t b)
{
    return (a > b) ? a : b;
}

static uint32_t min_val(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

static bcm_async_queue_t score_qstat_wlan_nic_queue;
#define SCORE_WLAN_NIC_ASYNC_Q_SIZE   4096

int host_ts_factor;
/* Callback function to push NIC latency record to async queue */
static int score_qstat_wlan_nic_func(void *nic_rec)
{
    score_hw_qstat_record_wlan_nic_t *rec_p;
    static uint32_t score_record_seq_num = 0;
    
    rec_p = (score_hw_qstat_record_wlan_nic_t*)nic_rec;
    score_record_seq_num++;
    /* TODO : Validate the score_id */
                                              
    if (bcm_async_queue_not_full (&score_qstat_wlan_nic_queue))
    {
        score_qstat_data_t *entry_p = (score_qstat_data_t *)
            bcm_async_queue_entry_write (&score_qstat_wlan_nic_queue);
        /* Not filling in timestamp for now */
        /* entry_p->timestamp = 0; */
        entry_p->type = QSTAT_TYPE_WLAN_NIC_INFO;
        entry_p->report_seq_num = score_record_seq_num;
        entry_p->score_stream_id = rec_p->score_stream_key;
        if (!rec_p->egress_ts)
            return 0;    /* no egress_ts */
        /* Assuming that both timestamp have same source in ms */
        entry_p->wlan_nic.pkt_latency = (uint16_t)(rec_p->egress_ts - rec_p->ingress_ts);
        if (host_ts_factor) entry_p->wlan_nic.pkt_latency /= host_ts_factor;
        /* (uint16_t)entry_p->wlan_nic.hw_q_latency = (uint16_t)(rec_p->hw_q_ts - rec_p->ingress_ts); */
        bcm_async_queue_entry_enqueue (&score_qstat_wlan_nic_queue);
        score_qstat_wlan_nic_queue.stats.writes++;
        /* TODO : do coalescing */
        score_notify_async_q_data();
    }
    else
    {
        score_qstat_wlan_nic_queue.stats.discards++;
    }
    return 0;
}

/* Initialize qstat for WLAN NIC */
static int qstat_init_wlan_nic(void)
{
    /* Create Async Queue and initialize the global function pointer */
    int ret = 0;
    int queue_entry_size = sizeof(score_qstat_data_t);
    queue_entry_size = (queue_entry_size + 0x3) & ~3; /* Make entry 4-byte multiple */
    if ((ret = bcm_async_queue_init(&score_qstat_wlan_nic_queue, SCORE_WLAN_NIC_ASYNC_Q_SIZE,
                                    queue_entry_size)) != 0)
    {
        __logError("WLAN NIC Queue Init Failed Q Size- %d\n", SCORE_WLAN_NIC_ASYNC_Q_SIZE);
        return ret;
    }
    /* Initialize the callback function for skb-free-thread */
    bcm_score_qstat_wlan_nic_func = score_qstat_wlan_nic_func;
    return ret;
}

/* Read latency records from async_q and process the data */
int process_qstat_async_q(bcm_async_queue_t *queue_p, int bound)
{
    int pkts = 0, done = 0;
    score_qstat_data_t *score_qstat_info_p;
    if (queue_p == NULL)
    {
        done = 1;
        return done;
    }

    while (!bound || (pkts < bound))
    {
        if (bcm_async_queue_not_empty(queue_p))
        {
            score_qstat_info_p = (score_qstat_data_t *)
                   bcm_async_queue_entry_read(queue_p);
        }
        else
        {
            done = 1;
            break;
        }
        qstat_process_data(score_qstat_info_p);
        qstat_procfs_entry_queue(score_qstat_info_p);
        if (score_prints_enable_g)
        {
             __print("SCORE qstat_async_q: ts : 0x%x, seq_num : 0x%x, stream_id: %d, type : 0x%x\n",
                        score_qstat_info_p->timestamp,
                        score_qstat_info_p->report_seq_num,
                        score_qstat_info_p->score_stream_id,
                        score_qstat_info_p->type);
        }
        bcm_async_queue_entry_dequeue(queue_p);
        queue_p->stats.reads++;
        pkts++;
    }
    return done;
}

int qstat_get_asyncq_stats(bcm_async_queue_t *queue_p, bcm_async_queue_stats_t *stats_p)
{
    /* Atomicity of stats is not gauranteed with runner traffic */
    if (queue_p == NULL)
    {
        return -1;
    }

    memcpy(stats_p, &queue_p->stats, sizeof(*stats_p));

    return 0;
}

int qstat_get_nic_asyncq_stats(bcm_async_queue_stats_t *stats_p)
{
    return qstat_get_asyncq_stats(&score_qstat_wlan_nic_queue, stats_p);
}
int process_qstat_nic(int bound)
{
    return process_qstat_async_q(&score_qstat_wlan_nic_queue, bound);
}

void qstat_init(void)
{
    int i, j;

    for (i = 0; i < QSTAT_MAX_IF_NUM; i++)
    {
        for (j = 0; j < QSTAT_MAX_QUEUE_NUM; j++)
        {
            if_queue_id_to_qstat_id[i][j] = QSTAT_INVALID;
        }
    }

    for (i = 0; i < QSTAT_MAX_QUEUES; i++)
    {
        qstat_db[i].is_alloc = 0;
        qstat_db[i].info.if_type = IF_INVALID;
        qstat_db[i].info.queue_id = QSTAT_INVALID;
        qstat_db[i].info.if_id = QSTAT_INVALID;
        qstat_db[i].mon_bitmap = 0;

        for (j = 0; j < QSTAT_MAX_PROFILES; j++)
        {
            qstat_db[i].monitor[j].score_stream_key.val = SCORE_STREAM_ID_INVALID;
            qstat_db[i].monitor[j].dcp_id = SCORE_DCP_ID_INVALID;
            qstat_db[i].monitor[j].stat_idx = QSTAT_INVALID;
        }
    }

    for (i = 0; i < SCORE_NUM_OF_STREAM_KEYS; i++)
    {
        score_stream_key_to_qstat_id[i] = QSTAT_INVALID;
    }

    for (i = 0; i < QSTAT_MAX_STATS; i++)
    {
        stats[i].ref_cnt = 0;
        stats[i].cfg_id = 0; /* default, always exists */
        stats[i].type = IF_INVALID;
    }

    for (i = 0; i < QSTAT_MAX_CFG_NUMBER; i++)
    {
        /* 0 = default, always exists */
        qstat_cfg_report[i].qstat_cfg_id = 0;
        memcpy(qstat_cfg_report[i].bin_cfg.limits, default_bin_cfg_limits,
                sizeof(qstat_cfg_report[i].bin_cfg.limits));
    }

    /* TODO - check return value and fail module init upon error */
    qstat_init_wlan_nic();
}

void qstat_uninit(void)
{
    bcm_score_qstat_wlan_nic_func = NULL;
}

static int alloc_new_qstat_cfg(void)
{
    int i;

    /* cfg[0] - always allocated = defaults */
    for (i = 1; i < QSTAT_MAX_CFG_NUMBER; i++)
    {
        if (qstat_cfg_report[i].qstat_cfg_id == 0)
        {
            qstat_cfg_report[i].qstat_cfg_id = i;
            return i;
        }
    }

    return 0;
}

static int validate_qstat_cfg(score_qstat_cfg_report_t *qstat_cfg)
{
    int i;
    uint8_t cfg_id = qstat_cfg->qstat_cfg_id;

    if (cfg_id >= QSTAT_MAX_CFG_NUMBER)
    {
        __logNotice("index out of range cfg_id: %d max %d\n", cfg_id, QSTAT_MAX_CFG_NUMBER);
        return -1;
    }

    if (qstat_cfg_report[cfg_id].qstat_cfg_id != cfg_id)
    {
        /* trying to configure without adding first */
        __logNotice("configuring not existing cfg_id: %d\n", cfg_id);
        return -1;
    }

    if (qstat_cfg->bin_cfg.limits[0] == 0)
    {
        __logNotice("bin configuration: bin[0] can't be 0\n");
        return -1;
    }

    for (i = 1; i < QSTAT_MAX_BIN_LIMITS; i++)
    {
        if (qstat_cfg->bin_cfg.limits[i])
        {
            if (qstat_cfg->bin_cfg.limits[i] <= qstat_cfg->bin_cfg.limits[i-1])
            {
                __logNotice("wrong bin configuration: [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
                            qstat_cfg->bin_cfg.limits[0], qstat_cfg->bin_cfg.limits[1],
                            qstat_cfg->bin_cfg.limits[2], qstat_cfg->bin_cfg.limits[3],
                            qstat_cfg->bin_cfg.limits[4], qstat_cfg->bin_cfg.limits[5],
                            qstat_cfg->bin_cfg.limits[6], qstat_cfg->bin_cfg.limits[7],
                            qstat_cfg->bin_cfg.limits[8], qstat_cfg->bin_cfg.limits[9],
                            qstat_cfg->bin_cfg.limits[10], qstat_cfg->bin_cfg.limits[11],
                            qstat_cfg->bin_cfg.limits[12], qstat_cfg->bin_cfg.limits[13],
                            qstat_cfg->bin_cfg.limits[14], qstat_cfg->bin_cfg.limits[15]);
                return -1;
            }
        }
    }

    return 0;
}

int qstat_config_report(score_qstat_cfg_report_t *qstat_cfg)
{
    uint8_t cfg_id, rc;

    if (!qstat_cfg)
        return -1;

    if (!qstat_cfg->qstat_cfg_id)
    {
        cfg_id = alloc_new_qstat_cfg();
        if (cfg_id == 0)
            return -1;
    }
    else
    {
        cfg_id = qstat_cfg->qstat_cfg_id;
    }

    rc = validate_qstat_cfg(qstat_cfg);
    if (rc)
        return rc;

    memcpy(&qstat_cfg_report[cfg_id].bin_cfg, &qstat_cfg->bin_cfg,
                sizeof(qstat_cfg_report[cfg_id].bin_cfg));

    qstat_cfg->qstat_cfg_id = cfg_id;

    return 0;
}

int qstat_delete_report(uint8_t qstat_cfg_id)
{
    if ((qstat_cfg_id) && (qstat_cfg_id < QSTAT_MAX_CFG_NUMBER))
    {
        qstat_cfg_report[qstat_cfg_id].qstat_cfg_id = 0;
    }

    return 0;
}

static int set_queue_map(qstat_queue_info_t *queue_info, uint16_t qstat_id)
{
    if ((queue_info->if_id >= QSTAT_MAX_IF_NUM) || (queue_info->queue_id >= QSTAT_MAX_QUEUE_NUM))
        return -1;

    /* only DHD and WIRED have queue to qstat mapping*/
    if ((queue_info->if_type != IF_WLAN_DHD) && (queue_info->if_type != IF_WIRED))
        return 0;

    if_queue_id_to_qstat_id[queue_info->if_id][queue_info->queue_id] = qstat_id;

    return 0;
}

static int get_queue_map(qstat_queue_info_t *queue_info)
{

    if ((queue_info->if_id >= QSTAT_MAX_IF_NUM) || (queue_info->queue_id >= QSTAT_MAX_QUEUE_NUM))
        return QSTAT_INVALID;

    /* only DHD and WIRED have queue to qstat mapping*/
    if ((queue_info->if_type != IF_WLAN_DHD) && (queue_info->if_type != IF_WIRED))
    {

        return QSTAT_INVALID;
    }


    return if_queue_id_to_qstat_id[queue_info->if_id][queue_info->queue_id];
}

static void qstat_reset_stats(uint16_t stat_idx)
{
    memset(&stats[stat_idx], 0 , sizeof(statistics_entry_t));
}

/* get qstat_id from queue info or score_stream_key*/
/* returns 0 - all valid
           1 - qstat_id not allocated
           -1 - error in parameters
*/
static int get_qstat_id(qstat_queue_info_t *queue_info, uint16_t score_stream_key, uint16_t *qstat_id)
{
    *qstat_id = QSTAT_INVALID;

    if (score_stream_key >= SCORE_NUM_OF_STREAM_KEYS)
        return -1;

    if (score_stream_key)
    {
        *qstat_id = score_stream_key_to_qstat_id[score_stream_key];
    }
    else
    {
        *qstat_id = get_queue_map(queue_info);
    }

    if (*qstat_id > QSTAT_MAX_QUEUES)
        return 1;

    return 0;
}


/* find new index in stat indexes pool */
static int alloc_stat_idx(uint16_t *idx)
{
    int i;

    for (i = 0; i < QSTAT_MAX_STATS; i++)
    {
        if (!(stats[i].ref_cnt))
        {
            /* free index found */
            qstat_reset_stats(i);
            stats[i].ref_cnt = 1;
            *idx = i;
            return 0;
        }
    }

    return -1;
}

/* get new qstat */
static int alloc_qstat_id(uint16_t *id)
{
    int i;

    for (i = 0; i < QSTAT_MAX_QUEUES; i++)
    {
        if (!(qstat_db[i].is_alloc))
        {
            /* free index found */
            qstat_db[i].is_alloc = 1;
            *id = i;
            return 0;
        }
    }

    return -1;
}

static int qstat_info_compare(qstat_queue_info_t *queue_info1, qstat_queue_info_t *queue_info2)
{
    /* NIC qstats based on score_stream_id only */
    if ((queue_info1->if_type == IF_WLAN_NIC) || (queue_info2->if_type == IF_WLAN_NIC))
        return 0;

    if ((queue_info1->if_type == queue_info2->if_type) &&
        (queue_info1->if_id == queue_info2->if_id) &&
        (queue_info1->queue_id == queue_info2->queue_id))
        return 0;

    return -1;
}


static int qstat_add_monitor(qstat_entry_t *qstat_entry, uint16_t score_stream_key, uint8_t dcp_id, uint16_t stat_idx)
{
    int rc = -1;
    uint16_t new_stat_idx = stat_idx;
    uint16_t mon_id;
    uint64_t inverted, mon_bitmap = qstat_entry->mon_bitmap;

    /* allocate and update stat_idx for requested qstat entry */
    while (mon_bitmap)
    {
        mon_id = __ffs64(mon_bitmap);
        mon_bitmap &= (mon_bitmap - 1);

        /* Sanity check */
        if (mon_id >= QSTAT_MAX_PROFILES)
            break;

        if ((qstat_entry->monitor[mon_id].score_stream_key.val == score_stream_key) &&
            (qstat_entry->monitor[mon_id].dcp_id == dcp_id))
        {
            /* already monitored - nothing to do */
            return 0;
        }

    }

    /* get first 0 */
    inverted = ~qstat_entry->mon_bitmap;
    mon_id = __ffs64(inverted);

    if ((inverted) && (mon_id < QSTAT_MAX_PROFILES))
    {
        /* new monitor set available */

        rc = 0;
        if (stat_idx == QSTAT_INVALID)
        {
            rc = alloc_stat_idx(&new_stat_idx);
        }
        else
        {
            stats[stat_idx].ref_cnt++;
        }

        if (!rc)
        {
            uint8_t dir = qstat_entry->monitor[mon_id].score_stream_key.dir;
            qstat_entry->monitor[mon_id].score_stream_key.val = score_stream_key;
            qstat_entry->monitor[mon_id].dcp_id = dcp_id;
            qstat_entry->monitor[mon_id].stat_idx = new_stat_idx;
            stats[new_stat_idx].cfg_id = dcp_db[dcp_id].collect.qstat_cfg_id[dir];
            SET_BIT(qstat_entry->mon_bitmap, mon_id);
            return 0;
        }
    }

    return rc;
}

static int qstat_del_monitor(qstat_entry_t *qstat_entry, uint16_t score_stream_key, uint8_t dcp_id)
{
    int stat_idx;

    uint16_t mon_id;
    uint64_t mon_bitmap = qstat_entry->mon_bitmap;

    while (mon_bitmap)
    {
        mon_id = __ffs64(mon_bitmap);
        mon_bitmap &= (mon_bitmap - 1);

        /* Sanity check */
        if (mon_id >= QSTAT_MAX_PROFILES)
            break;

        if ((qstat_entry->monitor[mon_id].score_stream_key.val == score_stream_key) &&
                (qstat_entry->monitor[mon_id].dcp_id == dcp_id))
        {
            CLR_BIT(qstat_entry->mon_bitmap, mon_id);
            qstat_entry->monitor[mon_id].score_stream_key.val = SCORE_STREAM_ID_INVALID;
            qstat_entry->monitor[mon_id].dcp_id = SCORE_DCP_ID_INVALID;
            stat_idx = qstat_entry->monitor[mon_id].stat_idx;

            if ((stat_idx != QSTAT_INVALID) && (stats[stat_idx].ref_cnt))
            {
                stats[stat_idx].ref_cnt--;
                if (stats[stat_idx].ref_cnt == 0)
                {
                    qstat_reset_stats(stat_idx);
                    stats[mon_id].type = IF_INVALID;
                }
            }
            qstat_entry->monitor[mon_id].stat_idx = QSTAT_INVALID;
        }
    }

    return 0;
}

static int get_stat_idx(qstat_entry_t *qstat_entry, uint16_t score_stream_key, uint8_t dcp_id)
{
    int stat_idx;

    uint16_t mon_id;
    uint64_t mon_bitmap = qstat_entry->mon_bitmap;

    while (mon_bitmap)
    {
        mon_id = __ffs64(mon_bitmap);
        mon_bitmap &= (mon_bitmap - 1);

        /* Sanity check */
        if (mon_id >= QSTAT_MAX_PROFILES)
            break;

        if ((qstat_entry->monitor[mon_id].score_stream_key.val == score_stream_key) &&
            (qstat_entry->monitor[mon_id].dcp_id == dcp_id))
        {
            stat_idx = qstat_entry->monitor[mon_id].stat_idx;

            if ((stat_idx != QSTAT_INVALID) && (stats[stat_idx].ref_cnt))
            {
                return stat_idx;
            }
        }
    }

    return QSTAT_INVALID;
}

static int qstat_add_new(qstat_queue_info_t *queue_info, uint16_t score_stream_key, uint8_t dcp_id, uint16_t stat_idx)
{
    int rc = -1;
    uint16_t qstat_id;

    rc = alloc_qstat_id(&qstat_id);

    if (!rc)
    {
        qstat_db[qstat_id].mon_bitmap = 0;
        rc = qstat_add_monitor(&qstat_db[qstat_id], score_stream_key, dcp_id, stat_idx);
        if (!rc)
        {
            /* update new qstat entry */
            rc = set_queue_map(queue_info, qstat_id);
            score_stream_key_to_qstat_id[score_stream_key] = qstat_id;
            qstat_db[qstat_id].info = *queue_info;
        }
    }

    return rc;
}

static int qstat_add_queue(qstat_queue_info_t *queue_info, uint16_t score_stream_key, uint8_t dcp_id)
{
    int rc = -1;
    uint16_t stat_idx, qstat_id;

    if ((queue_info->if_type == IF_WLAN_DHD) || (queue_info->if_type == IF_WIRED))
    {
        /* queue info available, get by queue only */
        get_qstat_id(queue_info, 0, &qstat_id);
    }
    else
    {
        /* NIC case - add by score_stream_id */
        if (get_qstat_id(queue_info, score_stream_key, &qstat_id) < 0)
            return -1;
    }

    if (qstat_id != QSTAT_INVALID)
    {
        /* qstat_id already monitored */
        /* same queue/stream_id requested , allocate new statistics*/
        rc = qstat_add_monitor(&qstat_db[qstat_id], score_stream_key, dcp_id, QSTAT_INVALID);
        if (!rc)
        {
             score_stream_key_to_qstat_id[score_stream_key] = qstat_id;
        }
        __logInfo("qstat %d exists, rc:%d, if_id:%d, queue_id:%d, new_mask:0x%x\n",
                  qstat_id, rc, qstat_db[qstat_id].info.if_id, qstat_db[qstat_id].info.queue_id,
                  qstat_db[qstat_id].mon_bitmap);
    }
    else
    {
        /* new queue or new score_stream_key for NIC */
        stat_idx = QSTAT_INVALID;

        qstat_id = score_stream_key_to_qstat_id[score_stream_key];
        if (qstat_id != QSTAT_INVALID)
        {
            /* we already have another queue that monitored by that stream_id, reuse stat_idx */
            stat_idx = get_stat_idx(&qstat_db[qstat_id], score_stream_key, dcp_id);
        }

        rc = qstat_add_new(queue_info, score_stream_key, dcp_id, stat_idx);
        __logInfo("add new qstat, rc:%d, if_id:%d, queue_id:%d, key:%d, dcp_id:%d\n",
              rc, queue_info->if_id, queue_info->queue_id,  score_stream_key, dcp_id);
    }

    return rc;
}


static int qstat_reset_entry(qstat_queue_info_t *queue_info, uint16_t score_stream_key, uint16_t qstat_id)
{
    int j;

    set_queue_map(queue_info, QSTAT_INVALID);

    if (score_stream_key < SCORE_NUM_OF_STREAM_KEYS)
    {
        score_stream_key_to_qstat_id[score_stream_key] = QSTAT_INVALID;
    }

    qstat_db[qstat_id].is_alloc = 0;
    qstat_db[qstat_id].info.if_type = IF_INVALID;
    qstat_db[qstat_id].info.queue_id = QSTAT_INVALID;
    qstat_db[qstat_id].info.if_id = QSTAT_INVALID;
    qstat_db[qstat_id].mon_bitmap = 0;

    for (j = 0; j < QSTAT_MAX_PROFILES; j++)
    {
        qstat_db[qstat_id].monitor[j].score_stream_key.val = SCORE_STREAM_ID_INVALID;
        qstat_db[qstat_id].monitor[j].dcp_id = SCORE_DCP_ID_INVALID;
        qstat_db[qstat_id].monitor[j].stat_idx = QSTAT_INVALID;
    }

    return 0;
}

static int qstat_delete_queue(qstat_queue_info_t *queue_info, uint16_t score_stream_key, uint8_t dcp_id)
{
    uint16_t rc = 0;
    uint16_t i;

    for (i = 0; i < QSTAT_MAX_QUEUES; i++)
    {
        if (!qstat_info_compare(queue_info, &qstat_db[i].info))
        {
            rc = qstat_del_monitor(&qstat_db[i], score_stream_key, dcp_id);
             __logInfo("qstat %d deleted, rc:%d, if_id:%d, queue_id:%d, new_mask:0x%x\n",
                      i, rc, qstat_db[i].info.if_id, qstat_db[i].info.queue_id,
                      qstat_db[i].mon_bitmap);
            if (qstat_db[i].mon_bitmap == 0)
            {
                /* qstat_id is free */
                qstat_reset_entry(queue_info, score_stream_key, i);
                rc = 0;
            }
            else
            {
                /* qstat_id still under monitoring of other profiles */
                rc = 1;
            }
        }
    }

    return rc;
}


static int get_streamkey_stat_idx(uint16_t score_stream_key, uint8_t dcp_id)
{
    uint16_t qstat_id;
    qstat_queue_info_t queue_info = {};
    uint16_t mon_id;
    uint64_t mon_bitmap;

    if (get_qstat_id(&queue_info, score_stream_key, &qstat_id))
        return -1;

    mon_bitmap = qstat_db[qstat_id].mon_bitmap;

    while (mon_bitmap)
    {
        mon_id = __ffs64(mon_bitmap);
        mon_bitmap &= (mon_bitmap - 1);

        /* Sanity check */
        if (mon_id >= QSTAT_MAX_PROFILES)
            break;

        if ((qstat_db[qstat_id].monitor[mon_id].dcp_id == dcp_id) &&
            (qstat_db[qstat_id].monitor[mon_id].score_stream_key.val == score_stream_key))
        {
            return qstat_db[qstat_id].monitor[mon_id].stat_idx;
        }
    }

    /* no stat_id avaialble */
    return QSTAT_INVALID;
}


int qstat_get_stat_results(uint16_t stat_idx, uint8_t reset_on_read, statistics_entry_t *stat_results)
{
    if ((stat_idx < QSTAT_MAX_STATS) && (stats[stat_idx].ref_cnt != 0))
    {
        *stat_results = stats[stat_idx];
        if (reset_on_read)
        {
            memset(&stats[stat_idx].dhd_info, 0, sizeof(qstat_wlan_dhd_info_t));
            memset(&stats[stat_idx].nic_info, 0, sizeof(qstat_wlan_nic_info_t));
            memset(&stats[stat_idx].wired_info, 0, sizeof(qstat_wired_info_t));
        }
        return 0;
    }

    return -1;
}

int qstat_get_result_by_streamkey(uint16_t score_stream_key, uint8_t dcp_id,
                                  uint8_t reset_on_read, statistics_entry_t *stat_results)
{
    int rc = -1;
    uint16_t stat_idx = 0;

    stat_idx = get_streamkey_stat_idx(score_stream_key, dcp_id);
    if (stat_idx != QSTAT_INVALID)
    {
        rc = qstat_get_stat_results(stat_idx, reset_on_read, stat_results);
    }
    return rc;
}

/* configures requested interface_id/queue_id/dcp_id */
int qstat_cfg_queue(qstat_queue_info_t *queue_info, uint16_t score_stream_key, uint8_t dcp_id, uint8_t enable)
{
    int rc = -1;


    __logInfo("QSTAT Cfg queue: type: %u, if_id: %u, queue_id: %u, dcp_id: %u, enable: %u\n",
               queue_info->if_type, queue_info->if_id, queue_info->queue_id, dcp_id,enable);

    if (dcp_id >= QSTAT_MAX_PROFILES)
        return -1;


    if (queue_info->if_type != IF_INVALID)
    {
        if (enable)
        {
            rc = qstat_add_queue(queue_info, score_stream_key, dcp_id);
        }
        else
        {
            /* returns 0 only when last profile removed from qstat_id or not found*/
            rc = qstat_delete_queue(queue_info, score_stream_key, dcp_id);
        }

        /* if queue added or last reference to queue removed clear queue mark in HW*/
        rc = rc ? rc : score_qstat_cfg_set(queue_info, enable);
    }

    if (rc < 0)
    {
        __logNotice("QSTAT Cfg queue failed: type: %u, if_id: %u, queue_id: %u, dcp_id: %u, score_stream_key: %u, enable: %u\n",
                   queue_info->if_type, queue_info->if_id, queue_info->queue_id, dcp_id, score_stream_key, enable);
    }

    return rc;
}

/* In order to update multiple packets with the same value at once use the count field */
static inline void qstat_update_telemetry_data(qstat_telemetry_data_t *tlm_p, uint32_t new_val, uint32_t count)
{
    if (tlm_p->reports_num == 0)
    {
        tlm_p->max_value = new_val;
        tlm_p->min_value = new_val;
    }
    tlm_p->reports_num += count;
    tlm_p->last_value = new_val;
    tlm_p->acc_value += new_val * count;
    tlm_p->acc_sq_value += (new_val * new_val) * count;
    tlm_p->max_value = max_val(tlm_p->max_value, new_val);
    tlm_p->min_value = min_val(tlm_p->min_value, new_val);
}

static inline void qstat_update_counters(qstat_counters_t *counters, uint64_t packets, uint64_t bytes)
{
    counters->packets += packets;
    counters->bytes += bytes;
}

/* In order to update multiple packets with the same value at once use the count field */
static inline void qstat_update_histogram_data(qstat_hist_t *hist_p, uint32_t new_val, uint32_t count, uint8_t cfg_id)
{
    int i;
    bin_cfg_t bin_cfg;

    if (cfg_id >= QSTAT_MAX_CFG_NUMBER)
        return;

    bin_cfg = qstat_cfg_report[cfg_id].bin_cfg;

    for (i = 0; i < QSTAT_MAX_BIN_LIMITS; i++)
    {
        if ((new_val < bin_cfg.limits[i]) || (bin_cfg.limits[i] == 0))
        {
            break;
        }
    }
    /* i  <= QSTAT_MAX_BIN_LIMITS < QSTAT_MAX_BIN_NUMBER */
    hist_p->val[i] += count;
}

static int qstat_process_wlan_dhd_latency(score_qstat_data_t *score_qstat_info)
{
    uint16_t stat_idx = 0, cfg_id = 0;
    uint8_t mon_id = 0;
    qstat_queue_info_t queue_info;
    qstat_wlan_dhd_info_t *info_p;
    struct wlan_dhd_tlm_info *tlm_p = &score_qstat_info->wlan_dhd;
    uint16_t qstat_id;
    uint64_t mon_bitmap;

    queue_info.if_type = IF_WLAN_DHD;
    queue_info.if_id = tlm_p->radio_idx;
    queue_info.queue_id = tlm_p->fr_idx;

    /* protection */
    if ((tlm_p->radio_idx >= QSTAT_MAX_IF_NUM) ||
        (tlm_p->fr_idx >= QSTAT_MAX_QUEUE_NUM))
        return -1;

    if (get_qstat_id(&queue_info, score_qstat_info->score_stream_id, &qstat_id))
        return -1;

    mon_bitmap = qstat_db[qstat_id].mon_bitmap;

    while (mon_bitmap)
    {
        uint32_t latency_msec;

        mon_id = __ffs64(mon_bitmap);
        mon_bitmap &= (mon_bitmap - 1);

        /* Sanity check */
        if (mon_id >= QSTAT_MAX_PROFILES)
            break;

        stat_idx = qstat_db[qstat_id].monitor[mon_id].stat_idx;
        stats[stat_idx].type = IF_WLAN_DHD;
        info_p = &stats[stat_idx].dhd_info;
        cfg_id = stats[stat_idx].cfg_id;

        info_p->radio_idx = tlm_p->radio_idx;
        info_p->queue_idx = tlm_p->fr_idx;

        latency_msec = MSEC_UNIT_ADJUST(tlm_p->pkt_latency);
        qstat_update_telemetry_data(&info_p->pkt_latency, latency_msec, 1);
        qstat_update_telemetry_data(&info_p->bq_latency, latency_msec, 1);
        qstat_update_telemetry_data(&info_p->bq_occupancy, tlm_p->bq_amount, 1);
        qstat_update_histogram_data(&info_p->pkt_latency_hist, latency_msec, 1, cfg_id);
    }

    return 0;
}

/* Process wired latency record */
static int qstat_process_wired_latency(score_qstat_data_t *score_qstat_info)
{
    uint16_t stat_idx = 0, cfg_id = 0;
    uint8_t mon_id = 0;
    qstat_queue_info_t queue_info = {};
    qstat_wired_info_t *info_p;
    wired_tlm_info_t *tlm_p = &score_qstat_info->wired;
    uint16_t qstat_id;
    uint64_t mon_bitmap;

#if defined(BCM6855) || defined(BCM63146) || defined(BCM4912) || defined(BCM6813)
    /* WA to avoid bad FW latency measurements */
    if (tlm_p->pkt_latency > QSTAT_WIRED_MAX_LATENCY_WA)
    {
        return 0;
    }
#endif

    queue_info.if_type = IF_WIRED;
    queue_info.if_id = QSTAT_IF_WIRED_INDEX;
    queue_info.queue_id = tlm_p->qm_queue;

    if (get_qstat_id(&queue_info, score_qstat_info->score_stream_id, &qstat_id))
        return -1;

    mon_bitmap = qstat_db[qstat_id].mon_bitmap;

    while (mon_bitmap)
    {
        mon_id = __ffs64(mon_bitmap);
        mon_bitmap &= (mon_bitmap - 1);

        /* Sanity check */
        if (mon_id >= QSTAT_MAX_PROFILES)
            break;

        stat_idx = qstat_db[qstat_id].monitor[mon_id].stat_idx;
        stats[stat_idx].type = IF_WIRED; /* TODO: Why do we need to assign this all the time?? */
        info_p = &stats[stat_idx].wired_info;
        cfg_id = stats[stat_idx].cfg_id;

        if (!tlm_p->aqm_drop)
        {
            uint32_t latency_msec;

            /* Transmitted packet */
            qstat_update_counters(&info_p->transmitted, tlm_p->pkt_count, tlm_p->pkt_length);

            /* Latency reported form the FW isn't a round number.
             * The FW counter is in units of 1 usec and it is right-shifted to the latency units relevant
             * to the queue type. This gives units such as 1.024 msec, 128 usec etc.
             * For the 1.024 msec case, which as currently assumed as the default, convert to whole 1msec.
             */
            /* TODO: Support multiple latency units (according to queue type) and display them properly */

            latency_msec = MSEC_UNIT_ADJUST(tlm_p->pkt_latency);
            qstat_update_telemetry_data(&info_p->latency, latency_msec, tlm_p->pkt_count);
            qstat_update_histogram_data(&info_p->latency_hist, latency_msec, tlm_p->pkt_count, cfg_id);

            /* Remarked packet */
            if (tlm_p->cq_latency_remark)
            {
                qstat_update_counters(&info_p->cq_latency_remark, tlm_p->pkt_count, tlm_p->pkt_length);
            }
            else if (tlm_p->lq_latency_remark)
            {
                qstat_update_counters(&info_p->lq_latency_remark, tlm_p->pkt_count, tlm_p->pkt_length);
            }
        }
        else
        {
            /* Dropped packet */
            qstat_update_counters(&info_p->aqm_dropped, tlm_p->pkt_count, tlm_p->pkt_length);
        }

    }

    return 0;
}

/* Process NIC latency record */
static int qstat_process_wlan_nic_latency(score_qstat_data_t *score_qstat_info)
{
    uint16_t stat_idx = 0, cfg_id = 0;
    uint8_t mon_id = 0;
    qstat_queue_info_t queue_info;
    qstat_wlan_nic_info_t *info_p;
    struct wlan_nic_tlm_info *tlm_p = &score_qstat_info->wlan_nic;
    uint16_t qstat_id;
    uint64_t mon_bitmap;

    queue_info.if_type = IF_WLAN_NIC;
    queue_info.if_id = 0; /* NA */
    queue_info.queue_id = 0; /* NA */

    if (get_qstat_id(&queue_info, score_qstat_info->score_stream_id, &qstat_id))
        return -1;

    mon_bitmap = qstat_db[qstat_id].mon_bitmap;

    while (mon_bitmap)
    {
        uint32_t latency_msec;

        mon_id = __ffs64(mon_bitmap);
        mon_bitmap &= (mon_bitmap - 1);

        /* Sanity check */
        if (mon_id >= QSTAT_MAX_PROFILES)
            break;

        stat_idx = qstat_db[qstat_id].monitor[mon_id].stat_idx;
        stats[stat_idx].type = IF_WLAN_NIC; /* ?? Why do we need to assign this all the time?? */
        info_p = &stats[stat_idx].nic_info;
        cfg_id = stats[stat_idx].cfg_id;

        latency_msec = MSEC_UNIT_ADJUST_WLAN_NIC(tlm_p->pkt_latency);
        qstat_update_telemetry_data(&info_p->pkt_latency, latency_msec, 1);
        qstat_update_histogram_data(&info_p->pkt_latency_hist, latency_msec, 1, cfg_id);

    }

    return 0;
}

void qstat_process_data(score_qstat_data_t *score_qstat_info)
{
    switch(score_qstat_info->type)
    {
        case QSTAT_TYPE_WLAN_DHD_INFO:
        {
            qstat_procfs_data_g.wlan_dhd_received_cnt++;
            qstat_process_wlan_dhd_latency(score_qstat_info);
            break;
        }
        case QSTAT_TYPE_WIRED_INFO:
        {
            qstat_procfs_data_g.wired_received_cnt++;
            qstat_process_wired_latency(score_qstat_info);
            break;
        }
        case QSTAT_TYPE_WLAN_NIC_INFO:
        {
            qstat_procfs_data_g.wlan_nic_received_cnt++;
            qstat_process_wlan_nic_latency(score_qstat_info);
            break;
        }
        default:
            break;
    }
}

