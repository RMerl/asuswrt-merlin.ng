/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
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

/*
*******************************************************************************
* File Name  : spdsvc_procfs.c
*
* Description: This file contains procfs implementation
*              of SPDSVC driver.
*
*******************************************************************************
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/bcm_log.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include "spdsvc_procfs.h"

#if defined(TR471_TIME_DATA_DEBUG)

uint64_t tr471_burst_enable_time_ns;    /* Time when ENABLE recevied */
uint64_t tr471_burst_gen_start_time_ns; /* Time when sw/hw burst gen triggered (sw - gen task needs trigger) */
uint64_t tr471_burst_gen_cmpl_time_ns;  /* Time when sw/hw burst gen complete (sw - gen-loop done, hw - interrupt received */
uint64_t tr471_task_wkup_time_ns;       /* Time when spdsvc task woke up (either to handle sw-gen or hw-cmpl) */
uint64_t tr471_burst_cmpl_time_ns;      /* Time when burst cmpl event was triggered to user application */
uint64_t tr471_burst_gen_tx_start_set_time_ns; /* Time when hw burst gen enable is done */
uint64_t tr471_burst_gen_tx_cmpl_set_time_ns; /* Time when /hw burst gen enable function is complete */

tr471_time_record_t tr471_pktgen_time_record;       /* Record of packet gen timing */
tr471_time_record_t tr471_task_wkup_time_record;    /* Record of task wkup timing */
tr471_time_record_t tr471_total_burst_time_record;  /* Record of entire burst timing */
tr471_time_record_t tr471_burst_gen_set_record;     /* Record of burst gen set timing */

/*
 *------------------------------------------------------------------------------
 * Function Name: tr471_update_time_data
 * Description  : Helper function to update time data for a record 
 *------------------------------------------------------------------------------
 */
static inline void tr471_update_time_data(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns,
                                     tr471_time_record_t *pRec)
{
    uint32_t diff_us = ((uint32_t)(cmpl_ns - start_ns))/1000; /* Convert to usec */
    uint32_t idx = diff_us/100;  /* Create 100us based index */
    if (first_burst)
    {
        pRec->first_burst_time_usec = diff_us;
    }
    else
    {
        if (diff_us > pRec->time_usec)
        {
            pRec->time_usec = diff_us;
            pRec->burst_size = burst_size;
        }
        idx = (idx >= NUM_TR471_TIME_RECORDS) ? (NUM_TR471_TIME_RECORDS - 1) : idx;
        pRec->cnts[idx]++;
    }

}
/*
 *------------------------------------------------------------------------------
 * Function Name: tr471_update_pktgen_time
 * Description  : Helper function to update time data for pktgen 
 *------------------------------------------------------------------------------
 */
void tr471_update_pktgen_time(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns)
{
    tr471_update_time_data(first_burst, burst_size, start_ns, cmpl_ns, &tr471_pktgen_time_record);
}
/*
 *------------------------------------------------------------------------------
 * Function Name: tr471_update_task_wkup_time
 * Description  : Helper function to update time data for task wakeup 
 *------------------------------------------------------------------------------
 */
void tr471_update_task_wkup_time(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns)
{
    tr471_update_time_data(first_burst, burst_size, start_ns, cmpl_ns, &tr471_task_wkup_time_record);
}
/*
 *------------------------------------------------------------------------------
 * Function Name: tr471_update_total_time
 * Description  : Helper function to update time data for complete burst 
 *------------------------------------------------------------------------------
 */
void tr471_update_total_time(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns)
{
    tr471_update_time_data(first_burst, burst_size, start_ns, cmpl_ns, &tr471_total_burst_time_record);
}
/*
 *------------------------------------------------------------------------------
 * Function Name: tr471_update_pktgen_time
 * Description  : Helper function to update time data for pktgen 
 *------------------------------------------------------------------------------
 */
void tr471_update_burst_gen_set_time(int first_burst, uint32_t burst_size, uint64_t start_ns, uint64_t cmpl_ns)
{
    tr471_update_time_data(first_burst, burst_size, start_ns, cmpl_ns, &tr471_burst_gen_set_record);
}
/*
 *------------------------------------------------------------------------------
 * Function Name: tr471_procfs_print_record
 * Description  : Helper function to print time data record 
 *------------------------------------------------------------------------------
 */
void tr471_procfs_print_record(struct seq_file *sf, tr471_time_record_t *pRec)
{
    uint32_t idx = 0;
    bcm_seq_printf(sf, "first_burst: %3uus MAX (Time: %4uus for Pkts: %3u)\n", pRec->first_burst_time_usec, pRec->time_usec, pRec->burst_size);
    for (idx=0; idx < NUM_TR471_TIME_RECORDS; idx++)
    {
        if (pRec->cnts[idx])
        {
            bcm_seq_printf(sf, "[%3u:%3u]us = %4u\n", idx*100, ((idx+1)*100) - 1, pRec->cnts[idx]);
        }
    }
    /* Reset after dump */
    memset(pRec, 0, sizeof(*pRec));
}
/*
 *------------------------------------------------------------------------------
 * Function Name: tr471_procfs_print_time_data
 * Description  : Helper function to print time data record for all types
 *------------------------------------------------------------------------------
 */
void tr471_procfs_print_time_data(struct seq_file *sf)
{
    bcm_seq_printf( sf, "\nComplete Burst handling: \n");
    tr471_procfs_print_record(sf, &tr471_total_burst_time_record);

    bcm_seq_printf( sf, "\nBurst Generation: \n");
    tr471_procfs_print_record(sf, &tr471_pktgen_time_record);

    bcm_seq_printf( sf, "\nTask Wakeup: \n");
    tr471_procfs_print_record(sf, &tr471_task_wkup_time_record);

    bcm_seq_printf( sf, "\nBurst Generation Setup: \n");
    tr471_procfs_print_record(sf, &tr471_burst_gen_set_record);

}
#else
void tr471_procfs_print_time_data(struct seq_file *sf)
{
}
#endif

#define SPDSVC_PROCFS_DIR_PATH       "spdsvc"   /* dir: /procfs/spdsvc    */

static int spdsvc_clear_on_read = 1;
static spdsvc_procfs_data_t last_procfs_data;
static spdsvc_procfs_data_t last_burst_procfs_data;
spdsvc_procfs_data_t spdsvc_procfs_data_g;

typedef enum {
    SPDSVC_PROCFS_FILE_STATUS = 1,
    SPDSVC_PROCFS_FILE_LAST_BURST,
    SPDSVC_PROCFS_FILE_MAX,
}spdsvc_procfs_file_t;

typedef struct {
    spdsvc_procfs_file_t file_type;
    int dump_done;
}spdsvc_procfs_file_data_t;

spdsvc_procfs_file_data_t spdsvc_procfs_status_file_data_g;
spdsvc_procfs_file_data_t spdsvc_procfs_last_burst_file_data_g;

/* Note: Not accounting for wrap around*/
#define SPDSVC_GET_STATS_CNT(last, field)         (spdsvc_procfs_data_g.field - last->field)
#define SPDSVC_TR471_GET_STATS_CNT(last, field)   (spdsvc_procfs_data_g.tr471_cnts.field - last->tr471_cnts.field)

/* Extern to avoid including spdsvc.h */
extern char *spdsvc_get_state(void);
extern char *spdsvc_get_mode(void);
extern int spdsvc_get_next_host_dev(int *idx_p, char *buf, int size);
extern char *spdsvc_tr471_get_state(void);

void spdsvc_procfs_handle_enable(void)
{
    /* Copy over current global stats on enable */
    memcpy(&last_burst_procfs_data, &spdsvc_procfs_data_g, sizeof(last_burst_procfs_data));
}
/*
 *------------------------------------------------------------------------------
 * Function Name: spdsvc_seq_start
 * Description  : 
 *------------------------------------------------------------------------------
 */
static void *spdsvc_seq_start(struct seq_file *s, loff_t *pos)
{
    spdsvc_procfs_file_data_t *data_p = (spdsvc_procfs_file_data_t *)(s->private);

    if (data_p->dump_done)
    {
        return NULL;
    }
    return pos;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: spdsvc_seq_next
 * Description  : 
 *------------------------------------------------------------------------------
 */
static void *spdsvc_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    return NULL;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: spdsvc_seq_stop
 * Description  : 
 *------------------------------------------------------------------------------
 */
static void spdsvc_seq_stop(struct seq_file *s, void *v)
{
    return;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: spdsvc_procfs_show
 * Description  : Helper function
 *------------------------------------------------------------------------------
 */
static void spdsvc_procfs_show(struct seq_file *sf, spdsvc_procfs_data_t *last_p)
{
    bcm_seq_printf( sf, "SPDSVC Status/Stats [COR:%d]:\n", spdsvc_clear_on_read);
    bcm_seq_printf( sf, "SPDSVC State           = %s\n", spdsvc_get_state());
    bcm_seq_printf( sf, "SPDSVC Mode            = %s\n", spdsvc_get_mode());
    bcm_seq_printf( sf, "ioctl_enable_cnt       = %u\n", SPDSVC_GET_STATS_CNT(last_p, ioctl_enable_cnt));
    bcm_seq_printf( sf, "ioctl_disable_cntt     = %u\n", SPDSVC_GET_STATS_CNT(last_p, ioctl_disable_cnt));
    bcm_seq_printf( sf, "ioctl_result_cnt       = %u\n", SPDSVC_GET_STATS_CNT(last_p, ioctl_result_cnt));
    bcm_seq_printf( sf, "ioctl_ovrhd_cnt        = %u\n", SPDSVC_GET_STATS_CNT(last_p, ioctl_ovrhd_cnt));

    bcm_seq_printf( sf, "\nTR471 Status/Stats:\n");
    bcm_seq_printf( sf, "flow_state             = %s\n", spdsvc_tr471_get_state());
    bcm_seq_printf( sf, "error_cnt              = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, error_cnt));
    bcm_seq_printf( sf, "buf_alloc              = %u\n", SPDSVC_GET_STATS_CNT(last_p, buf_alloc));
    bcm_seq_printf( sf, "buf_free               = %u\n", SPDSVC_GET_STATS_CNT(last_p, buf_free));
    bcm_seq_printf( sf, "reset_cnt              = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, reset_cnt));
    bcm_seq_printf( sf, "pkt_store_cnt          = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, pkt_store_cnt));
    bcm_seq_printf( sf, "pkt_free_cnt           = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, pkt_free_cnt));
    bcm_seq_printf( sf, "netdev_rx_cnt          = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, netdev_rx_cnt));
    bcm_seq_printf( sf, "soft_gen_cnt           = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, soft_gen_cnt));
    bcm_seq_printf( sf, "hw_gen_cnt             = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, hw_gen_cnt));
    bcm_seq_printf( sf, "hw_gen_cmpl_cnt        = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, hw_gen_cmpl_cnt));
    bcm_seq_printf( sf, "hw_gen_chk_status      = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, hw_gen_chk_status));
    bcm_seq_printf( sf, "tx_via_fc_cnt          = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, tx_via_fc_cnt));
    bcm_seq_printf( sf, "nf_out_hk_cnt          = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_out_hk_cnt));
    bcm_seq_printf( sf, "nf_out_hk_reg_cnt      = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_out_hk_reg_cnt));
    bcm_seq_printf( sf, "nf_out_hk_unreg_cnt    = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_out_hk_unreg_cnt));
    bcm_seq_printf( sf, "nf_in_hk_cnt           = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_in_hk_cnt));
    bcm_seq_printf( sf, "nf_in_hk_reg_cnt       = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_in_hk_reg_cnt));
    bcm_seq_printf( sf, "nf_in_hk_unreg_cnt     = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_in_hk_unreg_cnt));
    bcm_seq_printf( sf, "blog_event_cnt         = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, blog_event_cnt));
    bcm_seq_printf( sf, "blog_event_fc_act_cnt  = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, blog_event_fc_act_cnt));
    bcm_seq_printf( sf, "blog_event_fc_deact_cnt= %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, blog_event_fc_deact_cnt));
    bcm_seq_printf( sf, "blog_event_hw_act_cnt  = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, blog_event_hw_act_cnt));
    bcm_seq_printf( sf, "blog_event_hw_deact_cnt= %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, blog_event_hw_deact_cnt));
    bcm_seq_printf( sf, "non_tr471_pkt_cnt      = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, non_tr471_pkt_cnt));
    bcm_seq_printf( sf, "pkt_match_unfrag       = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, pkt_match_unfrag));
    bcm_seq_printf( sf, "pkt_match_frag_first   = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, pkt_match_frag_first));
    bcm_seq_printf( sf, "pkt_match_frag_not_first= %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, pkt_match_frag_not_first));
    bcm_seq_printf( sf, "fwd_lhdr               = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, fwd_lhdr));
    bcm_seq_printf( sf, "nf_invalid_state       = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_invalid_state));
    bcm_seq_printf( sf, "nf_invalid_mode        = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_invalid_mode));
    bcm_seq_printf( sf, "nf_invalid_socket      = %u\n", SPDSVC_TR471_GET_STATS_CNT(last_p, nf_invalid_socket));

    tr471_procfs_print_time_data(sf);
}
/*
 *------------------------------------------------------------------------------
 * Function Name: spdsvc_procfs_file_last_burst_show
 * Description  : Helper function
 *------------------------------------------------------------------------------
 */
static int spdsvc_procfs_file_last_burst_show(struct seq_file *sf)
{
    bcm_seq_printf( sf, "\nStats during LAST burst\n\n");

    spdsvc_procfs_show(sf, &last_burst_procfs_data);

    return 1; /* dump_done */
}

/*
 *------------------------------------------------------------------------------
 * Function Name: spdsvc_procfs_file_status_show
 * Description  : Helper function
 *------------------------------------------------------------------------------
 */
static int spdsvc_procfs_file_status_show(struct seq_file *sf)
{
    bcm_seq_printf( sf, "\nOverall Stats\n\n");

    spdsvc_procfs_show(sf, &last_procfs_data);

    /* Store */
    if (spdsvc_clear_on_read)
    {
        memcpy(&last_procfs_data, &spdsvc_procfs_data_g, sizeof(last_procfs_data));
    }

    return 1; /* dump_done */
}

/*
 *------------------------------------------------------------------------------
 * Function Name: spdsvc_seq_show
 * Description  : 
 *------------------------------------------------------------------------------
 */
static int spdsvc_seq_show(struct seq_file *sf, void *v)
{
    spdsvc_procfs_file_data_t *data_p = (spdsvc_procfs_file_data_t *)(sf->private);

    if (data_p->file_type == SPDSVC_PROCFS_FILE_STATUS)
    {
        data_p->dump_done = spdsvc_procfs_file_status_show(sf);
        return 0;
    }
    else if (data_p->file_type == SPDSVC_PROCFS_FILE_LAST_BURST)
    {
        data_p->dump_done = spdsvc_procfs_file_last_burst_show(sf);
        return 0;
    }

    /* Unknown file -- Mark dump done */
    data_p->dump_done = 1;

    return 0;
}

static struct seq_operations spdsv_seq_ops = {
    .start = spdsvc_seq_start,
    .next  = spdsvc_seq_next,
    .stop  = spdsvc_seq_stop,
    .show  = spdsvc_seq_show,
};

/*
 *------------------------------------------------------------------------------
 * Function Name: spdsv_procfs_open
 * Description  : 
 *------------------------------------------------------------------------------
 */
static int spdsv_procfs_open(struct inode *inode, struct file *file)
{
    struct seq_file *s;
    int ret;
    spdsvc_procfs_status_file_data_g.dump_done = 0;
    if ((ret = seq_open(file, &spdsv_seq_ops)) >= 0)
    {
        s = file->private_data;
        s->private = &spdsvc_procfs_status_file_data_g;
    }
    return ret;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static struct proc_ops spdsvDrvProcfs_status = {
        .proc_open = spdsv_procfs_open,
        .proc_read = seq_read,
        .proc_lseek = seq_lseek,
        .proc_release = seq_release,
};
#else
static struct file_operations spdsvDrvProcfs_status = {
        .open = spdsv_procfs_open,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = seq_release,
};
#endif

/*
 *------------------------------------------------------------------------------
 * Function Name: spdsv_procfs_open
 * Description  : 
 *------------------------------------------------------------------------------
 */
static int spdsv_procfs_last_burst_open(struct inode *inode, struct file *file)
{
    struct seq_file *s;
    int ret;
    spdsvc_procfs_last_burst_file_data_g.dump_done = 0;
    if ((ret = seq_open(file, &spdsv_seq_ops)) >= 0)
    {
        s = file->private_data;
        s->private = &spdsvc_procfs_last_burst_file_data_g;
    }
    return ret;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static struct proc_ops spdsvDrvProcfs_last_burst = {
        .proc_open = spdsv_procfs_last_burst_open,
        .proc_read = seq_read,
        .proc_lseek = seq_lseek,
        .proc_release = seq_release,
};
#else
static struct file_operations spdsvDrvProcfs_last_burst = {
        .open = spdsv_procfs_last_burst_open,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = seq_release,
};
#endif

static ssize_t spdsvcClrOnReadShow(struct file *file, char *buff, size_t len, loff_t *offset)
{
    char kbuf[4];
    int copied;
    if (*offset)
        return 0;

    copied = snprintf(kbuf, sizeof(kbuf), "%d\n",spdsvc_clear_on_read);
    if ((copied >= len) || copy_to_user(buff, kbuf, copied))
    {
        return -EFAULT;
    }

    *offset += copied;

    return *offset;
}

static ssize_t spdsvcClrOnReadWrite(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[2];
    uint32_t enable;

    if (copy_from_user(input, buff, sizeof(input)) != 0)
    {
        bcm_print("spdsvcClrOnReadWrite : failed copy\n");
        return -EFAULT;
    }

    if (sscanf(input, "%u", &enable) < 1)
    {
        bcm_print("spdsvcClrOnReadWrite : failed scanf input=%s\n",input);
        return -EFAULT;
    }

    if (enable != 0 && enable != 1)
    {
        bcm_print("spdsvcClrOnReadWrite : Invalid input value %u\n",enable);
        return -EFAULT;
    }

    /* Any change, clear stored stats */
    if (enable ^ spdsvc_clear_on_read)
    {
        memset(&last_procfs_data, 0, sizeof(last_procfs_data));
    }

    spdsvc_clear_on_read = enable;

    return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static const struct proc_ops spdsvDrvProcfs_COR = {
    .proc_read   = spdsvcClrOnReadShow,
    .proc_write  = spdsvcClrOnReadWrite,
};
#else
static const struct file_operations spdsvDrvProcfs_COR = {
    .owner  = THIS_MODULE,
    .read   = spdsvcClrOnReadShow,
    .write  = spdsvcClrOnReadWrite,
};
#endif

static ssize_t spdsvcHostDevShow(struct file *file, char *buff, size_t len, loff_t *offset)
{
    char kbuf[128];
    int copied = 0, slen;
    int idx = -1;

    if (*offset)
        return 0;

    while ((slen = spdsvc_get_next_host_dev(&idx, kbuf, sizeof(kbuf))) != 0)
    {
        if ((copied + slen >= len) || copy_to_user(buff+copied, kbuf, slen))
        {
            return -EFAULT;
        }
        copied += slen;
    }

    *offset += copied;

    return *offset;
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0))
static const struct proc_ops spdsvDrvProcfs_host_dev = {
    .proc_read   = spdsvcHostDevShow,
};
#else
static const struct file_operations spdsvDrvProcfs_host_dev = {
    .owner  = THIS_MODULE,
    .read   = spdsvcHostDevShow,
};
#endif
/*
 *------------------------------------------------------------------------------
 * Function Name: spdsvc_procfs_construct
 * Description  : 
 *------------------------------------------------------------------------------
 */
int spdsvc_procfs_construct(void)
{
    struct proc_dir_entry *entry;
    proc_mkdir( SPDSVC_PROCFS_DIR_PATH, NULL );
    entry = proc_create_data(SPDSVC_PROCFS_DIR_PATH "/status" , S_IRUGO, NULL, &spdsvDrvProcfs_status, NULL);
    if (!entry)
    {
       bcm_print( CLRerr "%s Unable to create proc entry %s" CLRnl, __FUNCTION__, SPDSVC_PROCFS_DIR_PATH);
       return -1;
    }

    memset(&spdsvc_procfs_status_file_data_g, 0, sizeof(spdsvc_procfs_status_file_data_g));
    spdsvc_procfs_status_file_data_g.file_type = SPDSVC_PROCFS_FILE_STATUS;

    entry = proc_create_data(SPDSVC_PROCFS_DIR_PATH "/last_burst" , S_IRUGO, NULL, &spdsvDrvProcfs_last_burst, NULL);
    if (!entry)
    {
       bcm_print( CLRerr "%s Unable to create proc entry %s" CLRnl, __FUNCTION__, SPDSVC_PROCFS_DIR_PATH);
       return -1;
    }

    memset(&spdsvc_procfs_last_burst_file_data_g, 0, sizeof(spdsvc_procfs_last_burst_file_data_g));
    spdsvc_procfs_last_burst_file_data_g.file_type = SPDSVC_PROCFS_FILE_LAST_BURST;

    entry = proc_create_data(SPDSVC_PROCFS_DIR_PATH "/clear_on_read" , S_IWUSR|S_IRUGO, NULL, &spdsvDrvProcfs_COR, NULL);
    if (!entry)
    {
       bcm_print( CLRerr "%s Unable to create proc entry %s" CLRnl, __FUNCTION__, SPDSVC_PROCFS_DIR_PATH);
       return -1;
    }

    entry = proc_create_data(SPDSVC_PROCFS_DIR_PATH "/host_dev" , S_IRUGO, NULL, &spdsvDrvProcfs_host_dev, NULL);
    if (!entry)
    {
       bcm_print( CLRerr "%s Unable to create proc entry %s" CLRnl, __FUNCTION__, SPDSVC_PROCFS_DIR_PATH);
       return -1;
    }

    memset(&spdsvc_procfs_data_g, 0, sizeof(spdsvc_procfs_data_g));
    memset(&last_procfs_data, 0, sizeof(last_procfs_data));
    memset(&last_burst_procfs_data, 0, sizeof(last_burst_procfs_data));

    return 0;
}
