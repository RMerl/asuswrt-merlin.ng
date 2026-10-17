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

/* Broadcom SCORE driver */
#include <linux/types.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/bcm_log.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <net/tcp.h>
#include <linux/blog.h>
#include <linux/skbuff.h>
#include <linux/bcm_skb_defines.h>
#include <linux/cdev.h>


#include "score_int.h"
#include "score_qstat.h"
#include "bp3_license.h"

int score_ct_init(void);
void score_ct_exit(void);

spinlock_t score_drv_lock;

#define SCORE_LOCK()    spin_lock_bh(&score_drv_lock)
#define SCORE_UNLOCK()  spin_unlock_bh(&score_drv_lock)

static struct class *score_cmd_class = NULL;
static struct device *score_cmd_device = NULL;
static struct cdev score_cmd_cdev;
static int score_cmd_major = 0;

int score_prints_enable_g = 0;
int score_global_enable_g = 1;

score_t score_g = { };

score_dcp_cfg_data_t  dcp_db[SCORE_MAX_DCP_NUM] = {};

/* score_msg thread data elements */
typedef struct {
    struct task_struct *thread;
    wait_queue_head_t threadWqh;
    int work_avail;
} _score_msg_thread_t;


__rcu int (*bcm_score_bnl_notify_cb)(uint32_t cmd, void *arg2, void **arg3) = NULL;
EXPORT_SYMBOL(bcm_score_bnl_notify_cb);

__rcu int (*bcm_score_bnl_wakeup)(uint8_t dcp_id) = NULL;
EXPORT_SYMBOL(bcm_score_bnl_wakeup);


static _score_msg_thread_t _score_msg_thread_g;
static int score_alloc_dcp_idx(score_dcp_cfg_data_t *score_dcp_cfg_data)
{
    int i;

    for (i = 0; i < SCORE_MAX_DCP_NUM; i++)
    {
        if (dcp_db[i].info.dcp_id == SCORE_DCP_ID_INVALID)
        {
            __logInfo("allocated dcp_id %i\n",i);
            score_dcp_cfg_data->info.dcp_id = i;
            return 0;
        }
    }

    __logNotice("allocation failed\n");
    return -1;
}

int score_dcp_add(struct file *filep, score_dcp_cfg_data_t *score_dcp_cfg_data)
{
    int rc = 0;
    uint32_t dcp_id;
    score_profile_cfg_t dcp_cfg = {};
    score_sdr_info_t sdr_cfg = {};
    score_dcp_cfg_data_t *dcp;

    switch(score_dcp_cfg_data->info.dcp_type)
    {
        case DCP_TYPE_SCORE:
        {
            if (bcm_license_check_msg(BP3_FEATURE_SCORE) <= 0)
                return SCORE_ERR_SCORE_LICENSE_INVALID;
            break;
        }
        case DCP_TYPE_QSTAT:
        {
            if (bcm_license_check_msg(BP3_FEATURE_QSTAT) <= 0)
                return SCORE_ERR_QSTAT_LICENSE_INVALID;
            break;
        }
        case DCP_TYPE_BNL:
        {
            if (bcm_license_check_msg(BP3_FEATURE_BNL) <= 0)
                return SCORE_ERR_BNL_LICENSE_INVALID;
            break;
        }
        default:
            break;
    }

    rc = score_alloc_dcp_idx(score_dcp_cfg_data);
    if (rc)
        return rc;

    dcp_id = score_dcp_cfg_data->info.dcp_id;
    if (dcp_id >= SCORE_MAX_DCP_NUM)
        return SCORE_ERR_DCP_ID_INVALID;

    dcp = &dcp_db[dcp_id];

    /* will be filled by score_sdr_create */
    score_dcp_cfg_data->queue_info.alloc_size = 0;

    if (filep)
    {
        score_procfs_dcp_pid(dcp_id, current->pid);

        rc = score_sdr_create(filep, score_dcp_cfg_data, &sdr_cfg.cfg);
        if (rc)
            goto fail_sdr_create;
    }
    else
    {
        score_procfs_dcp_pid(dcp_id, 0);
    }

    memcpy(dcp, score_dcp_cfg_data, sizeof(score_dcp_cfg_data_t));
    memcpy(&dcp_cfg.common, &dcp->info, sizeof(score_dcp_common_cfg_data_t));
    memcpy(&dcp_cfg.collection, &dcp->collect, sizeof(score_dcp_data_collection_t));

    rc = score_profile_add(&dcp_id, &dcp_cfg);
    if (rc < 0)
    {
        __logNotice("hw profile add failed, rc: %d\n", rc);
        goto fail_profile_add;
    }

    sdr_cfg.id = dcp_id;

    if (filep)
    {
        rc = score_sdr_add(&sdr_cfg);
        if (rc < 0)
        {
            __logNotice("hw sdr add failed, rc: %d\n", rc);
            goto fail_sdr_add;
        }
    }

    return 0;

fail_sdr_add:
    score_profile_delete(dcp_id);
fail_profile_add:
    score_sdr_destroy(dcp_id);
fail_sdr_create:
    score_procfs_dcp_pid(dcp_id, 0);
    memset(&dcp_db[dcp_id], 0, sizeof(score_dcp_cfg_data_t));
    dcp_db[dcp_id].info.dcp_id = SCORE_DCP_ID_INVALID;
    __logNotice("hw SDR add failed, rc: %d\n", rc);
    return rc;
}

int score_dcp_release(void * data)
{
    uint32_t dcp_id = (uint32_t) (uintptr_t) data;
    int rc;

    __logInfo("dcp_id %i\n", (int) dcp_id);

    if (dcp_id >= SCORE_MAX_DCP_NUM || dcp_db[dcp_id].info.dcp_id == SCORE_DCP_ID_INVALID)
    {
        __logError("invalid dcp %u\n", dcp_id);
        return SCORE_ERR_DCP_ID_INVALID;
    }
    /* flush stream_id's for this dcp */
    score_id_tbl_flush_by_dcp(dcp_id);

    if (dcp_db[dcp_id].queue_info.alloc_size)
    {
        rc = score_sdr_delete(dcp_id);
        if (rc < 0)
        {
            __logNotice("hw SDR delete failed, rc: %d\n", rc);
        }

        score_sdr_destroy(dcp_id);
    }

    rc = score_profile_delete(dcp_id);
    if (rc < 0)
    {
        __logNotice("hw profile delete failed, rc: %d\n", rc);
    }

    score_procfs_dcp_pid(dcp_id, 0);
    memset(&dcp_db[dcp_id], 0, sizeof(score_dcp_cfg_data_t));
    dcp_db[dcp_id].info.dcp_id = SCORE_DCP_ID_INVALID;

    return rc;
}

int score_monitor_start(score_mon_start_data_t *score_mon_start_data)
{
    int rc;

    score_stream_info_t stream_cfg = {};
    uint32_t stream_id = score_mon_start_data->score_stream_id;
    uint8_t dcp_id = score_mon_start_data->dcp_id;

    stream_cfg.id = stream_id;

    /* TODO need to extract existing bitmask and add dcp_id bit */
    stream_cfg.cfg.dcp_bitmask = 1 << dcp_id;

    __logDebug("score_monitor_start stream_id =0x%x dcp_id = 0x%x \n",
                stream_id, dcp_id);

    rc = score_stream_add(&stream_id, &stream_cfg);
    if (rc < 0)
    {
        __logNotice("hw stream add failed, rc: %d\n", rc);
        return rc;
    }

    if (score_mon_start_data->notification_req)
    {
        rc = score_post_ctrl_message_dcp(SCORE_MONITOR_START_NOTIF, score_mon_start_data,
                                         sizeof(score_mon_start_data_t), dcp_id);
    }
    return 0;
}

int score_monitor_stop(score_mon_stop_data_t *score_mon_stop_data)
{
    int rc;
    uint32_t stream_id = score_mon_stop_data->score_stream_id;
    uint8_t dcp_id = score_mon_stop_data->dcp_id;

    __logDebug("score_monitor_stop stream_id =0x%x dcp_id = 0x%x \n",
                stream_id, dcp_id);

    rc = score_stream_delete(stream_id, dcp_id);
    if (rc < 0)
    {
        __logNotice("hw stream delete failed, rc: %d\n", rc);
        return rc;
    }
    if (score_mon_stop_data->notification_req)
    {
        rc = score_post_ctrl_message_dcp(SCORE_MONITOR_STOP_NOTIF, score_mon_stop_data,
                                         sizeof(score_mon_stop_data_t), score_mon_stop_data->dcp_id);
    }
    return 0;
}

int qstat_get_results(score_qstat_result_t *score_qstat_result)
{
    int rc, dir;
    uint8_t dcp_id;
    statistics_entry_t stat_results;
    score_stream_key_t streamkey = {};

    dcp_id = score_qstat_result->dcp_id;
    if (dcp_id >= SCORE_MAX_DCP_NUM)
        return SCORE_ERR_DCP_ID_INVALID;

    if (dcp_db[dcp_id].info.dcp_type != DCP_TYPE_QSTAT)
        return SCORE_ERR_DCP_TYPE_INVALID;

    /* get results for orig and reply directions */
    for (dir = 0; dir < 2; dir++)
    {
        score_qstat_result->dir_type[dir] = IF_INVALID;
        streamkey.id = score_qstat_result->score_stream_id;
        streamkey.dir = dir;
        rc = qstat_get_result_by_streamkey(streamkey.val, dcp_id, score_qstat_result->reset_on_read, &stat_results);
        if (!rc)
        {
            score_qstat_result->dir_type[dir] = stat_results.type;
            switch(stat_results.type)
            {
                case IF_WLAN_DHD:
                {
                    score_qstat_result->dir_results[dir].wlan_stats.pkt_latency = stat_results.dhd_info.pkt_latency;
                    score_qstat_result->dir_results[dir].wlan_stats.pkt_latency_hist = stat_results.dhd_info.pkt_latency_hist;
                    break;
                }
                case IF_WLAN_NIC:
                {
                    score_qstat_result->dir_results[dir].wlan_stats.pkt_latency = stat_results.nic_info.pkt_latency;
                    score_qstat_result->dir_results[dir].wlan_stats.pkt_latency_hist = stat_results.nic_info.pkt_latency_hist;
                    break;
                }
                case IF_WIRED:
                {
                    score_qstat_result->dir_results[dir].wired_stats.transmitted = stat_results.wired_info.transmitted;
                    score_qstat_result->dir_results[dir].wired_stats.aqm_dropped = stat_results.wired_info.aqm_dropped;
                    score_qstat_result->dir_results[dir].wired_stats.latency = stat_results.wired_info.latency;
                    score_qstat_result->dir_results[dir].wired_stats.latency_hist = stat_results.wired_info.latency_hist;
                    score_qstat_result->dir_results[dir].wired_stats.cq_latency_remark = stat_results.wired_info.cq_latency_remark;
                    score_qstat_result->dir_results[dir].wired_stats.lq_latency_remark = stat_results.wired_info.lq_latency_remark;
                    break;
                }
                default:
                    break;
            }
        }
    }
    return 0;
}

static inline void score_wake_up_msg_thread(void)
{
    _score_msg_thread_g.work_avail = 1;
    wake_up_interruptible(&(_score_msg_thread_g.threadWqh));
}

void score_hw_msg_callback(int param)
{
    score_wake_up_msg_thread();
}

void score_notify_async_q_data(void)
{
    score_wake_up_msg_thread();
}

static inline bool is_bnl_notify_needed(uint8_t dcp_id)
{
    return (bcm_score_bnl_notify_cb && (dcp_db[dcp_id].info.dcp_type == DCP_TYPE_BNL));
}

int score_dcp_notify(score_dcp_stream_cmd_t cmd, void *dcp_stream_cfg,
        void **dcp_priv)
{
    int ret = SCORE_ERR_OK;
    uint8_t dcp_id;

    if (cmd == SCORE_DCP_CMD_STREAM_ADD) {
        dcp_id = ((score_mon_start_data_t *)dcp_stream_cfg)->dcp_id;

        if (is_bnl_notify_needed(dcp_id))
            ret = bcm_score_bnl_notify_cb(cmd, dcp_stream_cfg, dcp_priv);

        if( ret == SCORE_ERR_OK) {
            ret = score_monitor_start(dcp_stream_cfg);

            if ((ret != SCORE_ERR_OK) & is_bnl_notify_needed(dcp_id)) {
                score_mon_stop_data_t stop_cfg = {};
                /*clean up */
                stop_cfg.dcp_id = dcp_id;
                stop_cfg.score_stream_id = ((score_mon_start_data_t *)dcp_stream_cfg)->score_stream_id;
                bcm_score_bnl_notify_cb(SCORE_DCP_CMD_STREAM_DELETE, &stop_cfg, dcp_priv);
            }
        }
    } else if (cmd == SCORE_DCP_CMD_STREAM_DELETE) {
        dcp_id = ((score_mon_stop_data_t *)dcp_stream_cfg)->dcp_id;
        if (is_bnl_notify_needed(dcp_id))
            ret = bcm_score_bnl_notify_cb(cmd, dcp_stream_cfg, dcp_priv); 
        /* proceed irrespective of ret from bnl*/
        ret = score_monitor_stop(dcp_stream_cfg);
    } else  {
        ret = SCORE_ERR_INTERNAL;
        __logNotice("dcp stream cmd =%u not supported \n", cmd);
    }
    return ret;
}

int score_hw_get_queue_info(blog_score_flow_info_t *score_flow_info, qstat_queue_info_t *queue_info)
{
    int rc = -1;

    __logDebug("phyHdrType =%u is_wfd: %u, is_chain: %u, hw_flow_enabled: %u \n",
                score_flow_info->tx_info.phyHdrType,
                score_flow_info->wfd.nic_ucast.is_wfd,
                score_flow_info->wfd.nic_ucast.is_chain,
                score_flow_info->hw_flow_enabled);

    memset(queue_info, 0, sizeof(qstat_queue_info_t)); 
    if (score_flow_info->tx_info.phyHdrType == BLOG_WLANPHY)
    {
        if (!score_flow_info->wfd.nic_ucast.is_wfd)
        {
            queue_info->if_type = IF_WLAN_DHD;
            queue_info->if_id = score_flow_info->rnr.radio_idx;
            queue_info->queue_id = score_flow_info->rnr.flowring_idx;
            rc = 0;
        }
        else if (score_flow_info->wfd.nic_ucast.is_chain && 
                 score_flow_info->hw_flow_enabled)
        {
            /* WFD = 1 >> Relying on above if check
               NIC = is_chain >> Otherwise DHD via WFD
               HW-Flow = 1 >> No support yet for software accelerated flow
               */
            queue_info->if_type = IF_WLAN_NIC;
            queue_info->if_id = 0; /* NA */
            queue_info->queue_id = 0; /* NA */
            rc = 0;
        }
    }
    else if ((score_flow_info->tx_info.phyHdrType == BLOG_ENETPHY) ||
             (score_flow_info->tx_info.phyHdrType == BLOG_GPONPHY) ||
             (score_flow_info->tx_info.phyHdrType == BLOG_EPONPHY))
    {
        rc = score_get_wired_queue_info(score_flow_info, queue_info);
    }

    return rc;
}

int score_hw_qstat_cfg_queue(uint16_t streamkey, qstat_queue_info_t *queue_info,
        uint16_t dcp_add_bmap, uint16_t dcp_del_bmap)
{
    int i, rc = 0;

    if (dcp_del_bmap)
    {
        /* first delete all to free resources */
        for (i = 0; i < SCORE_MAX_DCP_NUM; i++)
        {
            if (IS_BIT_ON(dcp_del_bmap, i))
            {
                if (dcp_db[i].info.dcp_type == DCP_TYPE_QSTAT)
                {
                    rc = qstat_cfg_queue(queue_info, streamkey, i, 0);
                    __logDebug("qstat deleted, rc:%d, stream_key:%d, dcp_id:%d, if_id:%d, queue_id:%d\n",
                                rc, streamkey, i, queue_info->if_id, queue_info->queue_id);
                }
            }
        }
    }

    if (dcp_add_bmap)
    {
        for (i = 0; i < SCORE_MAX_DCP_NUM; i++)
        {
            if (IS_BIT_ON(dcp_add_bmap, i))
            {
                if (dcp_db[i].info.dcp_type == DCP_TYPE_QSTAT)
                {
                    rc = qstat_cfg_queue(queue_info, streamkey, i, 1);
                    __logDebug("qstat added, rc:%d, stream_key:%d, dcp_id:%d, if_id:%d, queue_id:%d\n",
                                rc, streamkey, i, queue_info->if_id, queue_info->queue_id);
                }
            }
        }
    }

    return rc;
}

int score_dcp_flow_notify(uint16_t stream_id, uint16_t dcp_add_bmap, uint16_t dcp_del_bmap,
        blog_score_flow_info_t *score_flow_info)
{
    qstat_queue_info_t queue_info;
    score_stream_key_t stream_key;

    stream_key.id = stream_id;
    stream_key.dir = score_flow_info->nf_dir;
    /* this is needed only for qstat */
    /*TODO either check for QSTAT dcp type here or put every think inside qstat fucntion */
    /*stream_id.idx is the index */
    if (!score_hw_get_queue_info(score_flow_info, &queue_info))
    {

        __logDebug("qstat: streamkey:%d, if_id:%d, queue_id:%d, dcp_add_bmap:%d, dcp_del_bmap:%d\n",
                   stream_key.val, queue_info.if_id, queue_info.queue_id, dcp_add_bmap, dcp_del_bmap);

        score_hw_qstat_cfg_queue(stream_key.val, &queue_info, dcp_add_bmap, dcp_del_bmap);
    }
    else
    {
        __logDebug("qstat not supported for streamkey:%d\n", stream_key.val);
    }
    return 0;
}

static int process_msg_ring(int bound, int *qstat_avail, int *bnl_avail)
{
    int pkts = 0, done = 0;
    int rc = 0;
    uint16_t msg_bitmask = 0;
    score_data_t score_msg_info = {};

    *qstat_avail = 0;
    *bnl_avail = 0;

    while (!bound || (pkts < bound))
    {
        /* Fetch next message */
        rc = score_msg_ring_info_get(&score_msg_info);

        if (rc)
        {
            done = 1;
            break;
        }

        msg_bitmask |= score_msg_info.pending_msg_bitmask;

        score_procfs_entry_queue(&score_msg_info);

        if (score_prints_enable_g)
        {
             __print("SCORE_info_get: pending_msg_bitmask: %x, irq_cnt: %d\n",
                     score_msg_info.pending_msg_bitmask, score_msg_info.irq_cnt);
        }

        pkts++;
    }

    while (msg_bitmask)
    {
        uint16_t queue_id = ffs(msg_bitmask) - 1;
        msg_bitmask &= (msg_bitmask - 1);

        if (queue_id == SCORE_MSG_QSTAT_ID)
        {
            *qstat_avail = 1;
            continue;
        }

        if (queue_id == SCORE_MSG_BNL_ID)
        {
            *bnl_avail = 1;
            continue;
        }

        if (queue_id >= SCORE_MAX_DCP_NUM || dcp_db[queue_id].info.dcp_id == SCORE_DCP_ID_INVALID)
        {
            __logError("getting report indication for invalid dcp %i\n", queue_id);
            continue;
        }

        score_signal_dcp(queue_id);
    }
    return done;
}

static int _score_msg_thread(void *data)
{
    int qstat_ring_done = 0, msg_ring_done = 0, nic_ring_done = 0;
    int qstat_avail = 0, bnl_avail = 0;

    while (1)
    {
        wait_event_interruptible(_score_msg_thread_g.threadWqh,
                         _score_msg_thread_g.work_avail || kthread_should_stop());
        do
        {       
            if (kthread_should_stop())
            {
                __logInfo("kthread_should_stop detected on _score_msg_thread\n");
                return 0;
            }

            msg_ring_done = process_msg_ring(SCORE_MSG_RING_BATCH, &qstat_avail, &bnl_avail);

            if ((qstat_avail) || (!qstat_ring_done))
            {
                /* Process DHD (& Wired) qstat data */
                qstat_ring_done = process_qstat_ring(SCORE_QSTAT_RING_BATCH);
            }

            if (bcm_score_bnl_wakeup && bnl_avail)
            {
                bcm_score_bnl_wakeup(0);
            }

            /* Process NIC qstat data */
            nic_ring_done = process_qstat_nic(SCORE_QSTAT_RING_BATCH);

            if (msg_ring_done & qstat_ring_done & nic_ring_done)
            {
                _score_msg_thread_g.work_avail = 0;
            }
            else
            {
                cond_resched();
            }
        } while (_score_msg_thread_g.work_avail);
    } /* while (1) */

    return 0;
}

static int _score_msg_thread_construct(void)
{
    memset(&_score_msg_thread_g, 0, sizeof(_score_msg_thread_t));

    _score_msg_thread_g.work_avail = 0;

    init_waitqueue_head(&_score_msg_thread_g.threadWqh);

    _score_msg_thread_g.thread = kthread_create(_score_msg_thread, (void *)&score_g, "score_msg_th");
    if (_score_msg_thread_g.thread == NULL)
    {
        __logError("SCORE_drv: could not kthread_create: _score_msg_thread");
        return -1;
    }

    wake_up_process(_score_msg_thread_g.thread);

    return 0;
}

static void _score_msg_thread_destroy(void)
{
    if (kthread_stop(_score_msg_thread_g.thread))
        __logError("SCORE_drv: could not kthread_stop\n");
}

int score_msg_rings_init(void)
{
    int rc;

    rc = score_msg_ring_create(SCORE_MSG_QUEUE_SIZE);

    if (rc < 0)
    {
        __logError("(%d) error %d\n", SCORE_MSG_QUEUE_SIZE, rc);

        return rc;
    }

    rc = score_qstat_ring_create(SCORE_QSTAT_QUEUE_SIZE);

    if (rc < 0)
    {
        __logError("(%d) error %d\n", SCORE_QSTAT_QUEUE_SIZE, rc);

        return rc;
    }

    return 0;
}


void score_msg_rings_uninit(void)
{
    score_msg_ring_destroy(SCORE_MSG_QUEUE_SIZE);
    score_qstat_ring_destroy(SCORE_QSTAT_QUEUE_SIZE);
}

int score_db_init(void)
{
    int i;

    memset(dcp_db, 0, SCORE_MAX_DCP_NUM*sizeof(score_dcp_cfg_data_t));

    for (i = 0; i < SCORE_MAX_DCP_NUM; i++)
    {
        dcp_db[i].info.dcp_id = SCORE_DCP_ID_INVALID;
    }

    return 0;
}

/*******************************************************************************
 *
 * Score Device management
 *
 *******************************************************************************/
#define CLASS_NAME "score_rx_queue"

static ssize_t score_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    __logError("Not supported");
    return -EFAULT;
}

static ssize_t score_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
    __logError("Not supported");
    return -EFAULT;
}

static long score_ioctl(struct file *filep, unsigned int command, unsigned long arg)
{
    int ret = 0;
    int application_id;
    score_ioctl_t cmd = (score_ioctl_t)command;
    score_ioctl_data_t score_data;
    score_ioctl_data_t *user_score_data_p = (score_ioctl_data_t *)arg;
    __logDebug("file %p, command %i, pid %i arg %lu\n",filep, command,current->pid, arg);

    switch (cmd)
    {
        case SCORE_IOCTL_SCORE_CTRL_INIT:
            if (copy_from_user(&score_data.cfg, &user_score_data_p->cfg, sizeof(score_cfg_data_t)))
                return -EFAULT;

            application_id = score_ctrl_init(filep, &score_data.cfg);
            if (application_id < 0)
                return -EFAULT;

            if (copy_to_user(&user_score_data_p->cfg, &score_data.cfg, sizeof(score_cfg_data_t)))
                return -EFAULT;

            __logInfo("INIT: Registering new application\n");

            break;

        case SCORE_IOCTL_SCORE_DCP_ADD:
            if (copy_from_user(&score_data.dcp_cfg, &user_score_data_p->dcp_cfg, sizeof(score_dcp_cfg_data_t)))
                return -EFAULT;

            __logInfo("SCORE_IOCTL_SCORE_DCP_ADD: DCP_ID: %d type %d\n", score_data.dcp_cfg.info.dcp_id, score_data.dcp_cfg.info.dcp_type);
            ret = score_dcp_add(filep, &score_data.dcp_cfg);
            if (ret)
                return ret;
           
            if (copy_to_user(&user_score_data_p->dcp_cfg, &score_data.dcp_cfg, sizeof(score_dcp_cfg_data_t)))
                return -EFAULT;

            break;

        case SCORE_IOCTL_SCORE_MONITOR_START:
            if (copy_from_user(&score_data.mon_start, &user_score_data_p->mon_start, sizeof(score_mon_start_data_t)))
                return -EFAULT;

            __logInfo("MONITOR_START_REQ: DCP_id %d\n", score_data.mon_start.dcp_id);
            ret = score_monitor_start_by_streamtuple(&score_data.mon_start);
            if (!ret)
            {
                if (copy_to_user(&user_score_data_p->mon_start, &score_data.mon_start, sizeof(score_mon_start_data_t)))
                    return -EFAULT;
            }
            break;

        case SCORE_IOCTL_SCORE_MONITOR_STOP:
            if (copy_from_user(&score_data.mon_stop, &user_score_data_p->mon_stop, sizeof(score_mon_stop_data_t)))
                return -EFAULT;

            __logInfo("MONITOR_STOP_REQ: Score_stream_id %d\n", score_data.mon_stop.score_stream_id);
            ret = score_monitor_stop_by_streamid(&score_data.mon_stop);
            break;

        case SCORE_IOCTL_SCORE_QSTAT_REPORT_CFG:
            if (copy_from_user(&score_data.qstat_cfg, &user_score_data_p->qstat_cfg, sizeof(score_qstat_cfg_report_t)))
                return -EFAULT;

            __logDebug("QSTAT_REPORT_CFG: cfg_id %d\n", score_data.qstat_cfg.qstat_cfg_id);
            ret = qstat_config_report(&score_data.qstat_cfg);
            if (!ret)
            {
                if (copy_to_user(&user_score_data_p->qstat_cfg, &score_data.qstat_cfg, sizeof(score_qstat_cfg_report_t)))
                    return -EFAULT;
            }
            break;

        case SCORE_IOCTL_SCORE_QSTAT_REPORT_DEL:
            if (copy_from_user(&score_data.qstat_cfg_id, &user_score_data_p->qstat_cfg_id, sizeof(uint8_t)))
                return -EFAULT;

            __logDebug("QSTAT_REPORT_DEL: cfg_id %d\n", score_data.qstat_cfg_id);
            ret = qstat_delete_report(score_data.qstat_cfg_id);
            break;

        case SCORE_IOCTL_SCORE_QSTAT_RESULT:
            if (copy_from_user(&score_data.qstat_result, &user_score_data_p->qstat_result, sizeof(score_qstat_result_t)))
                return -EFAULT;

            __logDebug("QSTAT_RESULT_REQ: Score_stream_id %d\n", score_data.qstat_result.score_stream_id);
            ret = qstat_get_results(&score_data.qstat_result);
            if (!ret)
            {
                if (copy_to_user(&user_score_data_p->qstat_result, &score_data.qstat_result, sizeof(score_qstat_result_t)))
                    return -EFAULT;
            }
            break;

        default:
            __logError("Invalid cmd[%u]", command);
            ret = -1;
            break;
    }

    return ret;
}

int bcm_score_bnl_request(score_ioctl_t cmd, score_ioctl_data_t *score_data)
{
    int ret = SCORE_ERR_INTERNAL;

    __logDebug("command %i, pid %i \n",cmd, current->pid);
    switch (cmd)
    {
        case SCORE_IOCTL_SCORE_CTRL_INIT:
            /* create BNL data ring */
          break;

        case SCORE_IOCTL_SCORE_DCP_ADD:
            ret = score_dcp_add(NULL, &score_data->dcp_cfg);
            break;

        case SCORE_IOCTL_SCORE_DCP_DELETE:
            /*TODO stop monitor of existing streams */
            ret = score_dcp_release((void *)(uintptr_t)score_data->dcp_del.dcp_id);
            break;

        case SCORE_IOCTL_SCORE_MONITOR_START:
            __logDebug("MONITOR_START_REQ: DCP_id %d\n", score_data->mon_start.dcp_id);
            ret = score_monitor_start_by_streamtuple(&score_data->mon_start);
            break;

        case SCORE_IOCTL_SCORE_MONITOR_STOP:
            __logDebug("MONITOR_STOP_REQ: Score_stream_id %d\n", score_data->mon_stop.score_stream_id);
            ret = score_monitor_stop_by_streamid(&score_data->mon_stop);
            break;

        default:
            __logError("Invalid cmd[%u]", cmd);
            ret = SCORE_ERR_INTERNAL;
            break;
    }

    return ret;
}
EXPORT_SYMBOL(bcm_score_bnl_request);



static const struct file_operations score_fops_g =
{
        .owner = THIS_MODULE,
        .open = score_open,
        .read = score_read,
        .write = score_write,
        .release = score_release,
        .mmap = score_mmap,
        .unlocked_ioctl = score_ioctl,
#if defined(CONFIG_COMPAT)
        .compat_ioctl = score_ioctl,
#endif
        .poll = score_poll,
};

static int __init bcm_score_init(void)
{
    dev_t dev = 0;
    dev_t devno;
    int ret = 0;

    ret = alloc_chrdev_region(&dev, 0, 1, SCORE_DRV_NAME);
    if (ret < 0)
    {
        __logError("alloc_chrdev_region() failed %s class\n", SCORE_DRV_NAME);
        return -ENODEV;
    }
    score_cmd_major = MAJOR(dev);

    score_cmd_class = class_create(THIS_MODULE, SCORE_DRV_NAME);
    if (IS_ERR(score_cmd_class))
    {
        __logError("Unable to class_create() for the device [%s]\n", SCORE_DRV_NAME);
        ret = PTR_ERR(score_cmd_class);
        goto fail_free_chrdev_region;
    }

    devno = MKDEV(score_cmd_major, 0);
    cdev_init(&score_cmd_cdev, &score_fops_g);
    score_cmd_cdev.owner = THIS_MODULE;

    ret = cdev_add(&score_cmd_cdev, devno, 1);
    if (ret)
    {
        __logError("Fail to add cdev %s, ret = %d\n", SCORE_DRV_NAME, ret);
        goto fail_free_class;
    }

    score_cmd_device = device_create(score_cmd_class, NULL, devno, NULL, SCORE_DRV_NAME);
    if (IS_ERR(score_cmd_device))
    {
        ret = PTR_ERR(score_cmd_device);
        __logError("Fail to create device %s, rc = %d\n", SCORE_DRV_NAME, ret);
        goto fail_free_class;
    }

    ret = score_procfs_construct();
    if (ret)
    {
        __logError("Failed to create %s procfs (%d)", SCORE_DRV_NAME, ret);
        goto fail_free_class;
    }

    ret = score_hw_init();
    if (ret)
    {
        __logError("Cannot hw init");
        goto fail_hw_init;
    }

    /* SCORE msg thread - setup */
    ret = _score_msg_thread_construct();
    if (ret)
    {
        __logError("Cannot setup msg thread");
        goto fail_thread_construct;
    }

    ret = score_db_init();
    if (ret)
    {
        __logError("Failed to intialize DBs (%d)", SCORE_DRV_NAME, ret);
        goto fail_db_init;
    }

    ret = score_queue_init(score_cmd_device);
    if (ret)
    {
        __logError("Failed to intialize QUEUE (%d)", SCORE_DRV_NAME, ret);
        goto fail_db_init;
    }

    ret = score_msg_rings_init();
    if (ret)
    {
        __logError("Failed to create %s queues (%d)", SCORE_DRV_NAME, ret);
        goto fail_queue_init;
    }

    /* init QSTAT module */
    qstat_init();

    bcmLog_setLogLevel(BCM_LOG_ID_SCORE, BCM_LOG_LEVEL_NOTICE);

    __logNotice(SCORE_MODNAME " Char Driver " SCORE_VER_STR " Registered Ver:<%d>\n",
            score_cmd_major);

    ret = score_ct_init();
    if (ret)
    {
        __logError("Failed to create Score CT table rc=%d", ret);
        goto fail_queue_init;
        /*TODO free anything allocated till now*/
    }

    return 0;

fail_queue_init:

    qstat_uninit();
    score_msg_rings_uninit();

fail_db_init:

    _score_msg_thread_destroy();

fail_thread_construct:

    score_hw_destroy();

fail_hw_init:

    score_procfs_destroy();

fail_free_class:

    class_destroy(score_cmd_class);

fail_free_chrdev_region:

    unregister_chrdev_region(MKDEV(score_cmd_major, 0), 1);

    return ret;
}

static void __exit bcm_score_exit(void)
{
     __logInfo("_score_msg_thread_destroy start");

    score_ct_exit();

    qstat_uninit();

    _score_msg_thread_destroy();

    score_msg_rings_uninit();

    score_hw_destroy();

    score_procfs_destroy();

    device_destroy(score_cmd_class, MKDEV(score_cmd_major, 0));
    cdev_del(&score_cmd_cdev);
    class_destroy(score_cmd_class);
    unregister_chrdev_region(MKDEV(score_cmd_major, 0), 1);

    __logNotice(SCORE_MODNAME " Char Driver " SCORE_VER_STR " Unregistered <%d>", SCORE_DRV_MAJOR);

    return;
}

module_init(bcm_score_init);
module_exit(bcm_score_exit);

MODULE_DESCRIPTION(SCORE_MODNAME " GPL");
MODULE_VERSION(SCORE_VERSION);
MODULE_LICENSE("GPL");
