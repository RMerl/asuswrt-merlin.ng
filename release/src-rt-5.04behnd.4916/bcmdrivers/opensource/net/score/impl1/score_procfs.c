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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/bcm_log.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/types.h>
#include "score_hw.h"
#include "score_int.h"
#include "bcm_score.h"
#include "score_qstat.h"
#include "score_procfs.h"


static struct proc_dir_entry *bcm_score_proc_top_dir;
static struct proc_dir_entry *bcm_score_prints_enable;
static struct proc_dir_entry *bcm_score_status;
static struct proc_dir_entry *bcm_score_dcp;
static struct proc_dir_entry *bcm_score_last_report;
static struct proc_dir_entry *bcm_score_global_enable;

static struct proc_dir_entry *bcm_qstat_proc_top_dir;
static struct proc_dir_entry *bcm_qstat_status;
static struct proc_dir_entry *bcm_qstat_last_report;
static struct proc_dir_entry *bcm_qstat_asyncQ;
static struct proc_dir_entry *bcm_qstat_cfg;

score_procfs_data_t score_procfs_data_g = {};
qstat_procfs_data_t qstat_procfs_data_g = {};

static atomic_t procfs_rpt_locked_g;

typedef struct {
    uint32_t pid;
} score_procfs_dcp_data_t;

typedef struct {
    int dump_done;  /* true if done dumping */
    score_procfs_dcp_data_t    dcp[SCORE_MAX_DCP_NUM];
} score_procfs_file_data_t;


typedef struct {
    int dump_done;      /* true if done dumping */
    int cur_stat_idx;   /* current dump stat index */
} qstat_procfs_file_data_t;

static score_procfs_file_data_t score_procfs_status_file_data_g;
static qstat_procfs_file_data_t qstat_procfs_status_file_data_g;

static score_data_t score_procfs_last_rpt_g[SCORE_PROCFS_HISTORY] = {};
static int score_rpt_index = 0;

static score_qstat_data_t qstat_procfs_last_rpt_g[QSTAT_PROCFS_HISTORY] = {};
static int qstat_rpt_index = 0;

static int _score_last_report_get(int *idx_p, char *buf, int size)
{
    int idx = *idx_p;
    int int_idx;
    score_data_t entry;

    if (idx >= SCORE_PROCFS_HISTORY)
        return 0;

    if (idx < 0)
    {
        int_idx = score_rpt_index % SCORE_PROCFS_HISTORY;
        *idx_p = 1;
    }
    else
    {
        int_idx = (idx + score_rpt_index) % SCORE_PROCFS_HISTORY;
        (*idx_p)++;
    }

    entry = score_procfs_last_rpt_g[int_idx];
    if (score_procfs_data_g.msgs_received_total > score_rpt_index)
    {
        return snprintf(buf, size,
                        "MSG_pending_bitmask: %u, irq_cnt: %u\n",
                        entry.pending_msg_bitmask, entry.irq_cnt);
    }

    return 0; /* No more, return zero len*/
}

static int _qstat_last_report_get(int *idx_p, char *buf, int size)
{
    int idx = *idx_p;
    int int_idx;
    score_qstat_data_t entry;

    if (idx >= QSTAT_PROCFS_HISTORY)
        return 0;

    if (idx < 0)
    {
        int_idx = qstat_rpt_index % QSTAT_PROCFS_HISTORY;
        *idx_p = 1;
    }
    else
    {
        int_idx = (idx + qstat_rpt_index) % QSTAT_PROCFS_HISTORY;
        (*idx_p)++;
    }

    entry = qstat_procfs_last_rpt_g[int_idx];
    if (qstat_procfs_data_g.rnr_received_cnt > qstat_rpt_index)
    {
        if ((entry.type == QSTAT_TYPE_WLAN_NIC_INFO) || (entry.type == QSTAT_TYPE_WLAN_DHD_INFO))
        {
            return snprintf(buf, size,
                            "Report: %u, TS: 0x%x, type: %u, radio_idx: %u," \
                            "fr_idx: %u, pkt_latency: %u, bq_latency: %u, bq_amount: %u\n",
                            entry.report_seq_num, entry.timestamp, entry.type,
                            entry.wlan_dhd.radio_idx, entry.wlan_dhd.fr_idx,
                            entry.wlan_dhd.pkt_latency, entry.wlan_dhd.bq_latency, entry.wlan_dhd.bq_amount);
        }
        else if (entry.type == QSTAT_TYPE_WIRED_INFO)
        {
            return snprintf(buf, size,
                            "Report: %u, TS: 0x%x, " \
                            "pkt_count: %u, pkt_length: %u, pkt_latency: %u, " \
                            "aqm_drop: %u, cq_latency_remark: %u, lq_latency_remark: %u\n",
                            entry.report_seq_num, entry.timestamp,
                            entry.wired.pkt_count, entry.wired.pkt_length, entry.wired.pkt_latency,
                            entry.wired.aqm_drop, entry.wired.cq_latency_remark, entry.wired.lq_latency_remark);
        }
    }

    return 0; /* No more, return zero len*/
}

static int _qstat_asyncQ_get(int *idx_p, char *buf, int size)
{
    bcm_async_queue_stats_t stats;
    int idx = *idx_p;
    char disp_str[8] = {};

    if (idx < 0) /* First record */
    {
        /* Used for NIC */
        idx = 0;
    }

    if (score_qstats_asyncq_stats_get(&idx, &stats, disp_str, sizeof(disp_str)))
    {
        *idx_p = idx + 1; /* move to next index */
        return snprintf(buf, size,
                        "%s: writes = %u reads = %u discards = %u\n",
                        disp_str, stats.writes, stats.reads, stats.discards);
    }
    /* No more valid rec */
    return 0; /* done */
}

static void* score_seq_start(struct seq_file *s, loff_t *pos)
{
    score_procfs_file_data_t *data_p = (score_procfs_file_data_t *)(s->private);

    if (data_p->dump_done)
        return NULL;
    return pos;
}

static void* score_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    score_procfs_file_data_t *data_p = (score_procfs_file_data_t *)(s->private);

    (*pos)++; /* Increment -- but not really used */

    if (data_p->dump_done)
        return NULL;
    return pos;
}

static void score_seq_stop(struct seq_file *s, void *v)
{
}

static int score_seq_show(struct seq_file *sf, void *v)
{
    int i;
    score_procfs_file_data_t *data_p = (score_procfs_file_data_t *)(sf->private);

    bcm_seq_printf(sf, "\nSCORE Status/Stats\n");
    bcm_seq_printf(sf, "=============================\n");
    bcm_seq_printf(sf, "SCORE global enable         : %u\n", score_global_enable_g);
    bcm_seq_printf(sf, "total rnr msgs received     : %u\n", score_procfs_data_g.msgs_received_total);
    for (i = 0; i < SCORE_MAX_SDRS; i++)
    {
        if (score_procfs_data_g.msgs_received[i])
            bcm_seq_printf(sf, "    rnr msgs received sdr[%d]     : %u\n", i, score_procfs_data_g.msgs_received[i]);

    }
    bcm_seq_printf(sf, "total control msgs sent     : %u\n", score_procfs_data_g.ctrl_sent_app_total);
    for (i = 0; i < SCORE_MAX_SDRS; i++)
    {
        if (score_procfs_data_g.ctrl_sent_app[i])
            bcm_seq_printf(sf, "    control msgs sent app[%d]     : %u\n", i, score_procfs_data_g.ctrl_sent_app[i]);
    }
    bcm_seq_printf(sf, "total control msgs dropped  : %u\n", score_procfs_data_g.ctrl_dropped_app_total);
    for (i = 0; i < SCORE_MAX_SDRS; i++)
    {
        if (score_procfs_data_g.ctrl_dropped_app[i])
            bcm_seq_printf(sf, "    control msgs dropped app[%d]  : %u\n", i, score_procfs_data_g.ctrl_dropped_app[i]);
    }

    /* if seq file buffer overflows, data from this iteration will be discarded */
    if (!seq_has_overflowed(sf))
    {
    }

    data_p->dump_done = 1;
    return 0;
}

static struct seq_operations score_seq_ops = {
    .start = score_seq_start,
    .next  = score_seq_next,
    .stop  = score_seq_stop,
    .show  = score_seq_show,
};

static int score_status_proc_open(struct inode *inode, struct file *file)
{
    int ret;
    struct seq_file *s;

    if ((ret = seq_open(file, &score_seq_ops)) >= 0)
    {
        s = file->private_data;
        s->private = &score_procfs_status_file_data_g;
    }

    score_procfs_status_file_data_g.dump_done = 0;
    return ret;
}


static int score_dcp_proc_show(struct seq_file *sf, void *v)
{
    int i;
    uint32_t pid;
    bcm_seq_printf(sf, "\nSCORE Application/Profile\n");
    bcm_seq_printf(sf, "=============================\n");
    for (i = 0; i < SCORE_MAX_DCP_NUM; i++)
        if ((pid = score_procfs_status_file_data_g.dcp[i].pid))
            bcm_seq_printf(sf, "dcp[%i] owned by pid[%i]\n", i, pid);
    return 0;
}

static int score_dcp_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, score_dcp_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static struct proc_ops score_status_proc_ops = {
    .proc_open = score_status_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = seq_release,
};
#else
static struct file_operations score_status_proc_ops = {
    .open = score_status_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};
#endif

static struct proc_ops score_dcp_proc_ops = {
    .proc_open = score_dcp_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};



static int score_qstat_cfg_show(struct seq_file *sf, void *v)
{
    int i, bin;
    bin_cfg_t bin_cfg;
    uint16_t mon_id;
    uint64_t mon_bitmap;

    bcm_seq_printf(sf, "\nSCORE Qstat active monitors\n");
    bcm_seq_printf(sf, "=================================\n");
    for (i = 0; i < QSTAT_MAX_QUEUES; i++)
    {
        if (qstat_db[i].is_alloc)
        {
            bcm_seq_printf(sf, "qstat_id %u, if_type %d, if_id %d, queue_id %d, bitmap 0x%02x\n",
                           i, qstat_db[i].info.if_type, qstat_db[i].info.if_id,
                           qstat_db[i].info.queue_id, qstat_db[i].mon_bitmap);

            mon_bitmap = qstat_db[i].mon_bitmap;

            while (mon_bitmap)
            {
                mon_id = __ffs64(mon_bitmap);
                mon_bitmap &= (mon_bitmap - 1);

                /* Sanity check */
                if (mon_id >= QSTAT_MAX_PROFILES)
                    break;

                bcm_seq_printf(sf, "%d:(key %u, dcp_id %d, stat_idx %d)\n",
                               mon_id, qstat_db[i].monitor[mon_id].score_stream_key.val,
                               qstat_db[i].monitor[mon_id].dcp_id, qstat_db[i].monitor[mon_id].stat_idx);
            }

            bcm_seq_printf(sf, "\n");
        }
    }

    bcm_seq_printf(sf, "\n\nSCORE Qstat bins configurations\n");
    bcm_seq_printf(sf, "=================================\n");

    for (i = 0; i < QSTAT_MAX_CFG_NUMBER; i++)
    {
        if ((i == 0) || (qstat_cfg_report[i].qstat_cfg_id != 0))
        {
            bin_cfg = qstat_cfg_report[i].bin_cfg;
            bcm_seq_printf(sf, "Cfg %u : [ ", i);
            for (bin = 0; bin < QSTAT_MAX_BIN_LIMITS; bin++)
            {
                if (bin_cfg.limits[bin] != 0)
                {
                    bcm_seq_printf(sf, "%u ", bin_cfg.limits[bin]);
                }
            }
            bcm_seq_printf(sf, "]\n");
        }
    }

    bcm_seq_printf(sf, "\n");
    return 0;
}

static int score_qstat_cfg_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, score_qstat_cfg_show, NULL);
}

static struct proc_ops qstat_cfg_proc_ops = {
    .proc_open = score_qstat_cfg_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static ssize_t score_last_report_proc_read(struct file *file, char *buff, size_t len, loff_t *offset)
{
    char kbuf[256];
    int copied = 0, slen;
    int idx = -1;

    if (*offset)
        return 0;

    atomic_set(&procfs_rpt_locked_g, 1);

    while ((slen = _score_last_report_get(&idx, kbuf, sizeof(kbuf))) != 0)
    {
        if ((copied + slen >= len) || copy_to_user(buff + copied, kbuf, slen))
        {
            atomic_set(&procfs_rpt_locked_g, 0);
            return -EFAULT;
        }
        copied += slen;
    }

    *offset += copied;

    atomic_set(&procfs_rpt_locked_g, 0);
    return *offset;
}


static void* qstat_seq_start(struct seq_file *s, loff_t *pos)
{
    qstat_procfs_file_data_t *data_p = (qstat_procfs_file_data_t *)(s->private);

    if (data_p->dump_done)
        return NULL;
    return pos;
}

static void* qstat_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    qstat_procfs_file_data_t *data_p = (qstat_procfs_file_data_t *)(s->private);

    (*pos)++; /* Increment -- but not really used */

    if (data_p->dump_done)
        return NULL;
    return pos;
}

static void qstat_seq_stop(struct seq_file *s, void *v)
{
}

static uint32_t stat64_to_stat32(uint64_t stat64)
{
#define UINT32_MAX 0xFFFFFFFF
    if (stat64 >= UINT32_MAX)
        return UINT32_MAX;

    return (uint32_t)stat64;
}

static uint32_t do_divide(uint64_t divident, uint32_t divisor, uint32_t *frac_rslt)
{
    uint32_t fraction = 0;

    if (divisor)
    {
        fraction = do_div(divident, divisor);
        fraction = (fraction * 10) / divisor;
    }
    else
    {
        divident = 0;
    }

    if (frac_rslt)
    {
        *frac_rslt = fraction;
    }

    return stat64_to_stat32(divident);
}

static int qstat_show(struct seq_file *sf, int stat_idx)
{
    int rc, i;
    statistics_entry_t stat_results;

    if (stat_idx == 0)
    {
        bcm_seq_printf(sf, "\nQSTAT Status and statistics\n");
        bcm_seq_printf(sf, "=============================\n");
        bcm_seq_printf(sf, "rnr_received_cnt       = %u\n", qstat_procfs_data_g.rnr_received_cnt);
        bcm_seq_printf(sf, "wlan_dhd_received_cnt  = %u\n", qstat_procfs_data_g.wlan_dhd_received_cnt);
        bcm_seq_printf(sf, "wlan_nic_received_cnt  = %u\n", qstat_procfs_data_g.wlan_nic_received_cnt);
        bcm_seq_printf(sf, "wired_received_cnt     = %u\n", qstat_procfs_data_g.wired_received_cnt);
    }

    rc = qstat_get_stat_results(stat_idx, 0, &stat_results);

    if (!rc)
    {
        if (stat_results.type == IF_WLAN_DHD)
        {
            qstat_wlan_dhd_info_t *dhd_info = &stat_results.dhd_info;
            uint32_t aver_pkt_latency, aver_bq_latency, aver_bq_amount;
            uint32_t pkt_latency_frac, bq_latency_frac, bq_occupancy_frac;

            aver_pkt_latency = do_divide(dhd_info->pkt_latency.acc_value, dhd_info->pkt_latency.reports_num, &pkt_latency_frac);
            aver_bq_latency = do_divide(dhd_info->bq_latency.acc_value, dhd_info->bq_latency.reports_num, &bq_latency_frac);
            aver_bq_amount = do_divide(dhd_info->bq_occupancy.acc_value, dhd_info->bq_occupancy.reports_num, &bq_occupancy_frac);


            bcm_seq_printf(sf, "\nWLAN DHD Stats[%u]\n", stat_idx);
            bcm_seq_printf(sf, "=============================\n");
            bcm_seq_printf(sf, "radio_idx                  = %u\n", dhd_info->radio_idx);
            bcm_seq_printf(sf, "queue_idx                  = %u\n", dhd_info->queue_idx);
            bcm_seq_printf(sf, "reports_num                = %u\n", dhd_info->pkt_latency.reports_num);
            bcm_seq_printf(sf, "bq_occupancy_average       = %u.%u\n", aver_bq_amount, bq_occupancy_frac);
            bcm_seq_printf(sf, "bq_latency_average_msec    = %u.%u\n", aver_bq_latency, bq_latency_frac);
            bcm_seq_printf(sf, "pkt_latency_average_msec   = %u.%u\n", aver_pkt_latency, pkt_latency_frac);
            bcm_seq_printf(sf, "pkt_latency_min_value_msec = %u\n", dhd_info->pkt_latency.min_value);
            bcm_seq_printf(sf, "pkt_latency_max_value_msec = %u\n", dhd_info->pkt_latency.max_value);
            bcm_seq_printf(sf, "pkt_latency_hist           = [");
            for (i = 0; i < QSTAT_MAX_BIN_NUMBER; i++)
            {
                bcm_seq_printf(sf, "%u ", dhd_info->pkt_latency_hist.val[i]);
            }
            bcm_seq_printf(sf, "]\n");
        }
        else if (stat_results.type == IF_WLAN_NIC)
        {
            qstat_wlan_nic_info_t *nic_info = &stat_results.nic_info;
            uint32_t aver_pkt_latency;
            uint32_t pkt_latency_frac;

            aver_pkt_latency = do_divide(nic_info->pkt_latency.acc_value, nic_info->pkt_latency.reports_num, &pkt_latency_frac);

            bcm_seq_printf(sf, "\nWLAN NIC Stats[%u]\n", stat_idx);
            bcm_seq_printf(sf, "=============================\n");
            bcm_seq_printf(sf, "reports_num                = %u\n", nic_info->pkt_latency.reports_num);
            bcm_seq_printf(sf, "pkt_latency_average_msec   = %u.%u\n", aver_pkt_latency, pkt_latency_frac);
            bcm_seq_printf(sf, "pkt_latency_min_value_msec = %u\n", nic_info->pkt_latency.min_value);
            bcm_seq_printf(sf, "pkt_latency_max_value_msec = %u\n", nic_info->pkt_latency.max_value);
            bcm_seq_printf(sf, "pkt_latency_hist           = [");
            for (i = 0; i < QSTAT_MAX_BIN_NUMBER; i++)
            {
                bcm_seq_printf(sf, "%u ", nic_info->pkt_latency_hist.val[i]);
            }
            bcm_seq_printf(sf, "]\n");
        }
        else if (stat_results.type == IF_WIRED)
        {
            qstat_wired_info_t *wired_info = &stat_results.wired_info;
            uint32_t avg_integer = 0;
            uint32_t avg_fraction = 0;

            avg_integer = do_divide(wired_info->latency.acc_value, wired_info->latency.reports_num, &avg_fraction);

            bcm_seq_printf(sf, "\nWired stats[%u]\n", stat_idx);
            bcm_seq_printf(sf, "=============================\n");
            bcm_seq_printf(sf, "transmitted_packets               = %llu\n", wired_info->transmitted.packets);
            bcm_seq_printf(sf, "transmitted_bytes                 = %llu\n", wired_info->transmitted.bytes);
            bcm_seq_printf(sf, "aqm_dropped_packets               = %llu\n", wired_info->aqm_dropped.packets);
            bcm_seq_printf(sf, "aqm_dropped_bytes                 = %llu\n", wired_info->aqm_dropped.bytes);
            bcm_seq_printf(sf, "ECN_remarked_due_to_CQ_congestion = %llu\n", wired_info->cq_latency_remark.packets);
            bcm_seq_printf(sf, "ECN_remarked_due_to_LQ_congestion = %llu\n", wired_info->lq_latency_remark.packets);

            /* TODO: support latency units according to queue type */
            bcm_seq_printf(sf, "latency_minimum_msec  = %u\n", wired_info->latency.min_value);
            bcm_seq_printf(sf, "latency_average_msec  = %u.%u\n", avg_integer, avg_fraction);
            bcm_seq_printf(sf, "latency_maximum_msec  = %u\n", wired_info->latency.max_value);
            bcm_seq_printf(sf, "latency histogram = [ ");
            for (i = 0; i < QSTAT_MAX_BIN_NUMBER; i++)
            {
                bcm_seq_printf(sf, "%u ", wired_info->latency_hist.val[i]);
            }
            bcm_seq_printf(sf, "]\n");
        }
    }

    return 0;
}

static int qstat_seq_show(struct seq_file *sf, void *v)
{
    qstat_procfs_file_data_t *data_p = (qstat_procfs_file_data_t *)(sf->private);

    qstat_show(sf, data_p->cur_stat_idx);

    /* if seq file buffer overflows, data from this iteration will be discarded */
    if (!seq_has_overflowed(sf))
    {
        /* Move to next connection */
        data_p->cur_stat_idx++;
    }
    /* Check if we are done with dumping all connections */
    data_p->dump_done = (data_p->cur_stat_idx == QSTAT_MAX_STATS);

    return 0;
}

static struct seq_operations qstat_seq_ops = {
    .start = qstat_seq_start,
    .next  = qstat_seq_next,
    .stop  = qstat_seq_stop,
    .show  = qstat_seq_show,
};

static int qstat_status_proc_open(struct inode *inode, struct file *file)
{
    int ret;
    struct seq_file *s;

    qstat_procfs_status_file_data_g.cur_stat_idx = 0;

    if ((ret = seq_open(file, &qstat_seq_ops)) >= 0)
    {
        s = file->private_data;
        s->private = &qstat_procfs_status_file_data_g;
    }

    qstat_procfs_status_file_data_g.dump_done = 0;
    return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static struct proc_ops qstat_status_proc_ops = {
    .proc_open = qstat_status_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = seq_release,
};
#else
static struct file_operations qstat_status_proc_ops = {
    .open = qstat_status_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release,
};
#endif

static ssize_t qstat_last_report_proc_read(struct file *file, char *buff, size_t len, loff_t *offset)
{
    char kbuf[256];
    int copied = 0, slen;
    int idx = -1;

    if (*offset)
        return 0;

    atomic_set(&procfs_rpt_locked_g, 1);

    while ((slen = _qstat_last_report_get(&idx, kbuf, sizeof(kbuf))) != 0)
    {
        if ((copied + slen >= len) || copy_to_user(buff + copied, kbuf, slen))
        {
            atomic_set(&procfs_rpt_locked_g, 0);
            return -EFAULT;
        }
        copied += slen;
    }

    *offset += copied;

    atomic_set(&procfs_rpt_locked_g, 0);
    return *offset;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static const struct proc_ops score_last_report_proc_ops = {
    .proc_read   = score_last_report_proc_read,
};
#else
static const struct file_operations score_last_report_proc_ops = {
    .owner  = THIS_MODULE,
    .read   = score_last_report_proc_read,
};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static const struct proc_ops qstat_last_report_proc_ops = {
    .proc_read   = qstat_last_report_proc_read,
};
#else
static const struct file_operations qstat_last_report_proc_ops = {
    .owner  = THIS_MODULE,
    .read   = qstat_last_report_proc_read,
};
#endif

static ssize_t qstat_asyncQ_proc_read(struct file *file, char *buff, size_t len, loff_t *offset)
{
    char kbuf[256];
    int copied = 0, slen;
    int idx = -1;

    if (*offset)
        return 0;

    while ((slen = _qstat_asyncQ_get(&idx, kbuf, sizeof(kbuf))) != 0)
    {
        if ((copied + slen >= len) || copy_to_user(buff + copied, kbuf, slen))
        {
            return -EFAULT;
        }
        copied += slen;
    }

    *offset += copied;

    return *offset;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static const struct proc_ops qstat_asyncQ_proc_ops = {
    .proc_read   = qstat_asyncQ_proc_read,
};
#else
static const struct file_operations qstat_asyncQ_proc_ops = {
    .owner  = THIS_MODULE,
    .read   = qstat_asyncQ_proc_read,
};
#endif

static ssize_t score_prints_enable_proc_read(struct file *file, char *buff, size_t len, loff_t *offset)
{
    int copied;
    char kbuf[4];

    if (*offset)
        return 0;

    copied = snprintf(kbuf, sizeof(kbuf), "%d\n", score_prints_enable_g);
    if ((copied >= len) || copy_to_user(buff, kbuf, copied))
        return -EFAULT;

    *offset += copied;
    return *offset;
}

static ssize_t score_prints_enable_proc_write(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[2];
    uint32_t enable;

    if (copy_from_user(input, buff, sizeof(input)) != 0)
    {
        bcm_print("%s: failed copy\n", __FUNCTION__);
        return -EFAULT;
    }

    if (sscanf(input, "%u", &enable) < 1)
    {
        bcm_print("%s: failed scanf input=%s\n", __FUNCTION__, input);
        return -EFAULT;
    }

    if (enable != 0 && enable != 1)
    {
        bcm_print("%s: Invalid input value %u\n", __FUNCTION__, enable);
        return -EFAULT;
    }

    score_prints_enable_g = enable;
    return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static const struct proc_ops score_prints_enable_proc_ops = {
    .proc_read   = score_prints_enable_proc_read,
    .proc_write  = score_prints_enable_proc_write,
};
#else
static const struct file_operations score_prints_enable_proc_ops = {
    .owner  = THIS_MODULE,
    .read   = score_prints_enable_proc_read,
    .write  = score_prints_enable_proc_write,
};
#endif

static ssize_t score_global_enable_proc_read(struct file *file, char *buff, size_t len, loff_t *offset)
{
    char kbuf[4];
    int copied;

    if (*offset)
        return 0;

    copied = snprintf(kbuf, sizeof(kbuf), "%d\n", score_global_enable_g);
    if ((copied >= len) || copy_to_user(buff, kbuf, copied))
        return -EFAULT;

    *offset += copied;
    return *offset;
}

static ssize_t score_global_enable_proc_write(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    uint32_t enable;
    char input[2];

    if (copy_from_user(input, buff, sizeof(input)) != 0)
    {
        bcm_print("%s: failed copy\n", __FUNCTION__);
        return -EFAULT;
    }

    if (sscanf(input, "%u", &enable) < 1)
    {
        bcm_print("%s: failed scanf input=%s\n", __FUNCTION__, input);
        return -EFAULT;
    }

    if ((enable != 0) && (enable != 1))
    {
        bcm_print("%s: Invalid input value %u\n", __FUNCTION__, enable);
        return -EFAULT;
    }

    score_global_enable_g = enable;
    return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static const struct proc_ops score_global_enable_proc_ops = {
    .proc_read   = score_global_enable_proc_read,
    .proc_write  = score_global_enable_proc_write,
};
#else
static const struct file_operations score_global_enable_proc_ops = {
    .owner  = THIS_MODULE,
    .read   = score_global_enable_proc_read,
    .write  = score_global_enable_proc_write,
};
#endif


void score_procfs_dcp_pid(int dcp_id, uint32_t pid)
{
    score_procfs_status_file_data_g.dcp[dcp_id].pid = pid;
}

void score_procfs_entry_queue(score_data_t *queue_entry_p)
{
    uint16_t sdr_bitmask = queue_entry_p->pending_msg_bitmask;

    /* note : its not possible to keep an exact counter of score reports here.
              this merely serves as indicator for debug purpose */
    while (sdr_bitmask)
    {
        uint16_t sdr_id = ffs(sdr_bitmask) - 1;
        sdr_bitmask &= (sdr_bitmask - 1);
        score_procfs_data_g.msgs_received[sdr_id]++;
        score_procfs_data_g.msgs_received_total++;
    }

    if (!atomic_read(&procfs_rpt_locked_g))
    {
        score_procfs_last_rpt_g[score_rpt_index++] = *queue_entry_p;
        score_rpt_index = score_rpt_index % SCORE_PROCFS_HISTORY;
    }
}

void qstat_procfs_entry_queue(score_qstat_data_t *queue_entry_p)
{
    qstat_procfs_data_g.rnr_received_cnt++;

    /* TODO check seq_num to find out drops */
    if (!atomic_read(&procfs_rpt_locked_g))
    {
        qstat_procfs_last_rpt_g[qstat_rpt_index++] = *queue_entry_p;
        qstat_rpt_index = qstat_rpt_index % QSTAT_PROCFS_HISTORY;
    }
}

int score_procfs_construct(void)
{
    __debug("Loading procfs");

    atomic_set(&procfs_rpt_locked_g, 0);

    bcm_score_proc_top_dir = proc_mkdir(SCORE_PROCFS_DIR_PATH, NULL);

    if (!bcm_score_proc_top_dir)
    {
        bcm_print(CLRerr "%s Unable to create proc directory %s" CLRnl, __FUNCTION__,
                  SCORE_PROCFS_DIR_PATH);
        return -1;
    }

    bcm_score_status = proc_create_data(SCORE_PROCFS_DIR_PATH "/status", S_IRUGO, NULL,
                             &score_status_proc_ops, NULL);
    if (!bcm_score_status)
    {
        bcm_print(CLRerr "%s Unable to create proc status entry %s" CLRnl, __FUNCTION__,
                  SCORE_PROCFS_DIR_PATH);
        return -1;
    }


    bcm_score_dcp = proc_create_data(SCORE_PROCFS_DIR_PATH "/dcp", S_IRUGO, NULL,
                             &score_dcp_proc_ops, NULL);
    if (!bcm_score_dcp)
    {
        bcm_print(CLRerr "%s Unable to create proc dcp entry %s" CLRnl, __FUNCTION__,
                  SCORE_PROCFS_DIR_PATH);
        return -1;
    }

    memset(&score_procfs_status_file_data_g, 0, sizeof(score_procfs_status_file_data_g));


    bcm_score_prints_enable = proc_create_data(SCORE_PROCFS_DIR_PATH "/prints_enable", S_IWUSR | S_IRUGO,
                             NULL, &score_prints_enable_proc_ops, NULL);
    if (!bcm_score_prints_enable)
    {
        bcm_print(CLRerr "%s Unable to create proc prints_enable entry %s" CLRnl, __FUNCTION__,
                  SCORE_PROCFS_DIR_PATH);
        return -1;
    }

    bcm_score_global_enable = proc_create_data(SCORE_PROCFS_DIR_PATH "/score_enable", S_IWUSR | S_IRUGO,
                             NULL, &score_global_enable_proc_ops, NULL);
    if (!bcm_score_global_enable)
    {
        bcm_print(CLRerr "%s Unable to create proc score_enable entry %s" CLRnl, __FUNCTION__,
                  SCORE_PROCFS_DIR_PATH);
        return -1;
    }

    bcm_score_last_report = proc_create_data(SCORE_PROCFS_DIR_PATH "/last_report", S_IRUGO, NULL,
                             &score_last_report_proc_ops, NULL);
    if (!bcm_score_last_report)
    {
        bcm_print(CLRerr "%s Unable to create proc last_report entry %s" CLRnl, __FUNCTION__,
                  SCORE_PROCFS_DIR_PATH);
        return -1;
    }

    bcm_qstat_proc_top_dir = proc_mkdir(QSTAT_PROCFS_DIR_PATH, NULL);

    if (!bcm_qstat_proc_top_dir)
    {
        bcm_print(CLRerr "%s Unable to create proc directory %s" CLRnl, __FUNCTION__,
                  QSTAT_PROCFS_DIR_PATH);
        return -1;
    }

    bcm_qstat_status = proc_create_data(QSTAT_PROCFS_DIR_PATH "/status", S_IRUGO, NULL,
                             &qstat_status_proc_ops, NULL);
    if (!bcm_qstat_status)
    {
        bcm_print(CLRerr "%s Unable to create proc status entry %s" CLRnl, __FUNCTION__,
                  QSTAT_PROCFS_DIR_PATH);
        return -1;
    }

    memset(&qstat_procfs_status_file_data_g, 0, sizeof(qstat_procfs_status_file_data_g));

    bcm_qstat_last_report = proc_create_data(QSTAT_PROCFS_DIR_PATH "/last_report", S_IRUGO, NULL,
                             &qstat_last_report_proc_ops, NULL);
    if (!bcm_qstat_last_report)
    {
        bcm_print(CLRerr "%s Unable to create proc last_report entry %s" CLRnl, __FUNCTION__,
                  QSTAT_PROCFS_DIR_PATH);
        return -1;
    }

    bcm_qstat_asyncQ = proc_create_data(QSTAT_PROCFS_DIR_PATH "/asyncQ", S_IRUGO, NULL,
                             &qstat_asyncQ_proc_ops, NULL);
    if (!bcm_qstat_asyncQ)
    {
        bcm_print(CLRerr "%s Unable to create proc asyncQ entry %s" CLRnl, __FUNCTION__,
                  QSTAT_PROCFS_DIR_PATH);
        return -1;
    }

    bcm_qstat_cfg = proc_create_data(QSTAT_PROCFS_DIR_PATH "/cfg", S_IRUGO, NULL,
                             &qstat_cfg_proc_ops, NULL);
    if (!bcm_qstat_cfg)
    {
        bcm_print(CLRerr "%s Unable to create proc qstat cfg entry %s" CLRnl, __FUNCTION__,
                  QSTAT_PROCFS_DIR_PATH);
        return -1;
    }

    memset(&score_procfs_data_g, 0, sizeof(score_procfs_data_t));
    memset(&qstat_procfs_data_g, 0, sizeof(qstat_procfs_data_t));

    __debug("procfs loaded");
    return 0;
}


int score_procfs_destroy(void)
{
    if (bcm_score_status)
        remove_proc_entry(SCORE_PROCFS_DIR_PATH "/status", NULL);

    if (bcm_score_dcp)
        remove_proc_entry(SCORE_PROCFS_DIR_PATH "/dcp", NULL);

    if (bcm_score_prints_enable)
        remove_proc_entry(SCORE_PROCFS_DIR_PATH "/prints_enable", NULL);

    if (bcm_score_global_enable)
        remove_proc_entry(SCORE_PROCFS_DIR_PATH "/score_enable", NULL);

    if (bcm_score_last_report)
        remove_proc_entry(SCORE_PROCFS_DIR_PATH "/last_report", NULL);

    if (bcm_qstat_status)
        remove_proc_entry(QSTAT_PROCFS_DIR_PATH "/status", NULL);

    if (bcm_qstat_last_report)
        remove_proc_entry(QSTAT_PROCFS_DIR_PATH "/last_report", NULL);

    if (bcm_qstat_cfg)
        remove_proc_entry(QSTAT_PROCFS_DIR_PATH "/cfg", NULL);

    if (bcm_qstat_asyncQ)
        remove_proc_entry(QSTAT_PROCFS_DIR_PATH "/asyncQ", NULL);

    if (bcm_qstat_proc_top_dir)
        remove_proc_entry(QSTAT_PROCFS_DIR_PATH, NULL);

    if (bcm_score_proc_top_dir)
        remove_proc_entry(SCORE_PROCFS_DIR_PATH, NULL);

    return 0;
}

