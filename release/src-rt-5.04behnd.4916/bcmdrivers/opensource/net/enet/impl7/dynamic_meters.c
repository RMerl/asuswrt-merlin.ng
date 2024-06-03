/*
  <:copyright-BRCM:2018:DUAL/GPL:standard
  
     Copyright (c) 2018 Broadcom 
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

  Author: kosta.sopov@broadcom.com
*/
#include <rdpa_api.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/version.h>

#define RDPA_ACCUMULATED_COUNTERS

typedef struct {
     struct task_struct *poll_thread;
     bdmf_object_handle cpu_obj;
     int watch_qid;
     int index; /* meter object index */
     int meter_no_drop_time; /* time interval there was no meter drops */
     int no_queued_time; /* time interval there was no queued packets */
     uint32_t rate;
     uint32_t prev_rate;
#ifdef RDPA_ACCUMULATED_COUNTERS
     uint32_t dropped_cnt;
     uint32_t queue_dropped_cnt;
#endif

} dyn_meter_data_t;

static dyn_meter_data_t dyn_meter_data = {.index = -1};

typedef struct {
    union {
        struct {
            uint32_t enabled;
            uint32_t poll_intval_ms;
            uint32_t drop_threshold;
            uint32_t queue_threshold;
            uint32_t initial_rate;
            uint32_t max_rate;
            uint32_t min_rate;
            uint32_t meter_remove_timeout_ms;
            uint32_t rate_up_timeout_ms;
            uint32_t debug;
        };
        uint32_t param[8];
    };
} dyn_meter_config_t;

typedef struct {
    uint32_t rate_up;
    uint32_t rate_down;
    uint32_t meter_clear;
} dyn_meter_stats_t;

static dyn_meter_config_t config = {
    .enabled = 1,
    .poll_intval_ms = 500, 
    .drop_threshold = 20,   /* packets */
    .queue_threshold = 5,   /* packets */
    .initial_rate = 250000, /* fps */
    .max_rate = 500000,     /* fps */
    .min_rate = 10000,      /* fps */
    .meter_remove_timeout_ms = 10000,
    .rate_up_timeout_ms = 5000,
    .debug = 0,
};

static dyn_meter_stats_t stats = {};

char *config_names[][2] = {
    {"enabled", "Enable/Disable dynamic meter monitoring"},
    {"poll_intval_ms", "Monitoring polling interval in ms"},
    {"drop_threshold", "Minimal number of RX queue dropped packets considered for dynamic meter set/rate decrease"},
    {"queue_threshold", "Minimal number of RX queue queued packets considered for dynamic meter rate increase"},
    {"initial_rate", "Initial meter rate"},
    {"max_rate", "Maximal meter rate that will not be exceeded by rate search algorithm"},
    {"min_rate", "Minimal meter rate, rate search algorithml won't set lower rate"},
    {"meter_remove_timeout_ms", "Time to pass w/o meter drops for meter removal"},
    {"rate_up_timeout_ms", "Time to pass with RX queued count less than queue_threshold for meter rate up"},
    {"debug", "Debug messages on/off"},
    {NULL}
};

#define CONFIG_NAME_ID 0
#define CONFIG_HELP_ID 1
#define PROC_CMD_MAX_LEN 8
#define MIN_RATE_DIFF 2000
#define MAX_RATE_STEP 25000

#define log_debug(fmt, params...) do { if (config.debug) printk("[%u] " fmt, jiffies_to_msecs(jiffies), params); } while(0)

static void reason_meter_set(dyn_meter_data_t *meter)
{
    rdpa_cpu_reason_index_t reason_cfg_idxes[] = {
          {.reason = rdpa_cpu_rx_reason_ip_flow_miss},
          {.reason = rdpa_cpu_rx_reason_unknown_sa},
          {.reason = rdpa_cpu_rx_reason_ip_frag}
        };
    rdpa_cpu_reason_cfg_t reason_cfg = {};
    bdmf_object_handle port = NULL;
    int rc;
    int i;

    for (i=0; i < sizeof(reason_cfg_idxes)/sizeof(rdpa_cpu_reason_index_t); i++)
    {
        if ((rc = rdpa_cpu_reason_cfg_get(meter->cpu_obj, &reason_cfg_idxes[i], &reason_cfg)))
        {
            printk("%s: failed to get CPU RX reason cfg; rc %d\n", __func__, rc);
            return;
        }
        reason_cfg.meter = meter->index;
        if ((rc = rdpa_cpu_reason_cfg_set(meter->cpu_obj, &reason_cfg_idxes[i], &reason_cfg)))
        {
            printk("%s: failed to set CPU rx reason cfg; rc %d\n", __func__, rc);
            return;
        }
    }
    /* set port-port meter for unknown_da */
    while((port = bdmf_get_next(rdpa_port_drv(), port, NULL)))
    {
        rdpa_port_cpu_meter_set(port, rdpa_port_meter_unknown_da, meter->index);
    }
}

static void new_meter_set(dyn_meter_data_t *meter)
{
    rdpa_cpu_meter_cfg_t dummy;
    rdpa_cpu_meter_cfg_t meter_cfg = {};
    bdmf_index idx = 0;
    int rc = BDMF_ERR_OK;

    for (idx = 0; idx < RDPA_CPU_MAX_METERS; idx++)
    {
        rc = rdpa_cpu_meter_cfg_get(meter->cpu_obj, idx, &dummy);
        if (rc == BDMF_ERR_NOENT)
            break; /* Found available slot */
    }
    if (idx == RDPA_CPU_MAX_METERS)
    {
        printk_once("%s: can't find free meter object\n", __func__);
        return;
    }
    log_debug("found free meter id %ld\n", idx);

    meter->prev_rate = meter->rate = meter_cfg.sir = meter_cfg.burst_size = config.initial_rate;
    meter->meter_no_drop_time = 0;
    meter->index = idx;
    rdpa_cpu_meter_cfg_set(meter->cpu_obj, idx , &meter_cfg);

    reason_meter_set(meter);
}

static void meter_rate_modify(dyn_meter_data_t *meter, uint32_t rate)
{
    rdpa_cpu_meter_cfg_t meter_cfg = {};
    int ret;

    rate -= (rate % RDPA_CPU_METER_SR_QUANTA); /* align */
    meter_cfg.sir = meter_cfg.burst_size = rate;

    if ((ret = rdpa_cpu_meter_cfg_set(meter->cpu_obj, meter->index , &meter_cfg)))
    {
        printk_once("%s: rdpa_cpu_meter_cfg_set failed with %d\n", __func__, ret);
        return;
    }

    if (rate)
    {
        if (rate > meter->rate)
            stats.rate_up++;
        else
            stats.rate_down++;

        log_debug("prev rate %u; rate %s %u -> %u\n", meter->prev_rate, rate > meter->rate ? "up" : "down", meter->rate, rate);
    }
    else
        stats.meter_clear++;

    meter->prev_rate = meter->rate;
    meter->rate = rate;
}

static uint32_t meter_drop_counter_get(dyn_meter_data_t *meter)
{
    bdmf_number dropped = 0;
    int rc;

    if ((rc = rdpa_cpu_meter_stat_get(meter->cpu_obj, meter->index, &dropped)))
        printk("%s: failed to get meter stats; rc %d\n", __func__, rc);

#ifdef RDPA_ACCUMULATED_COUNTERS
    {
        uint32_t prev_dropped = meter->dropped_cnt;

        meter->dropped_cnt = (uint32_t)dropped;
        return dropped - prev_dropped;
    }
#else
    return (uint32_t)dropped;
#endif
}

static uint32_t queue_counters_get(dyn_meter_data_t *meter, uint32_t *dropped, uint32_t *queued)
{
     rdpa_cpu_rx_stat_t stat = {0};
     int ret;

     if ((ret = rdpa_cpu_rxq_stat_get(meter->cpu_obj, meter->watch_qid, &stat)))
     {
         printk_once("%s: rdpa_cpu_rxq_stat_get failed with %d\n", __func__, ret);
         return -1;
     }

#ifdef RDPA_ACCUMULATED_COUNTERS
    {
        uint32_t prev_dropped = meter->queue_dropped_cnt;

        meter->queue_dropped_cnt = stat.dropped;
        *dropped = stat.dropped - prev_dropped;
    }
#else
    *dropped = stat.dropped;
#endif
    *queued = stat.queued;
    return 0;
}

static void meter_remove(dyn_meter_data_t *meter)
{
    meter_rate_modify(meter, 0);
    meter->index = -1;
    reason_meter_set(meter);
    meter->meter_no_drop_time = 0;
}

static void rate_decrease(dyn_meter_data_t *meter)
{
     uint32_t rate, target, diff = 0;

     if (meter->index == -1)
     {
         new_meter_set(meter);
         return;
     }

     if (meter->rate <= config.min_rate)
         return;

     target = (meter->rate > meter->prev_rate) ? meter->prev_rate : config.min_rate;
     diff = meter->rate - target;

     if (diff < MIN_RATE_DIFF)
         rate = target;
     else if (diff > MAX_RATE_STEP*2)
         rate = meter->rate - MAX_RATE_STEP;
     else
         rate = meter->rate - diff/2;

     meter_rate_modify(meter, rate);
}

static void rate_increase(dyn_meter_data_t *meter, uint32_t queued)
{
      uint32_t rate, target, diff = 0;

      if (meter->rate >= config.max_rate || queued > config.queue_threshold)
      {
          meter->no_queued_time = 0;
          return;
      }

      meter->no_queued_time += config.poll_intval_ms;
      if (meter->no_queued_time < config.rate_up_timeout_ms)
          return;

      log_debug("rate_increase: queued %d; time %d\n", queued,
              meter->no_queued_time);

      meter->no_queued_time = 0;
      /* The queue isn't full, try to increase the rate */
      target = (meter->rate < meter->prev_rate) ? meter->prev_rate : config.max_rate;
      diff = target - meter->rate;

      if (diff < MIN_RATE_DIFF)
          rate = target;
      else if (diff > MAX_RATE_STEP*2)
          rate = meter->rate + MAX_RATE_STEP;
      else
          rate = meter->rate + diff/4;


      meter_rate_modify(meter, rate);
}

static void rate_decrease_on_drops(dyn_meter_data_t *meter)
{
    meter->meter_no_drop_time = 0;
    meter->no_queued_time = 0;

    rate_decrease(meter);
}

static void rate_increase_on_no_drops(dyn_meter_data_t *meter, uint32_t queued)
{
    meter->meter_no_drop_time = 0;
    rate_increase(meter, queued);
}

static void no_drop_time_upd(dyn_meter_data_t *meter)
{
    meter->no_queued_time = 0;
    meter->meter_no_drop_time += config.poll_intval_ms;  
}

static int dyn_meter_poll_handler(void *arg)
{
    dyn_meter_data_t *meter = &dyn_meter_data;

    while (!kthread_should_stop())
    {
        uint32_t dropped, queued;

        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(msecs_to_jiffies(config.poll_intval_ms));

        if (!config.enabled)
            continue;

        if (queue_counters_get(meter, &dropped, &queued))  /* both meters monitor the same queue */
            continue;

        if (dropped > config.drop_threshold)
        {
            rate_decrease_on_drops(meter);

            log_debug("Drops %d", dropped);
        }
        else
        {
            /* No queue drops - try to increase the rate or to remove the meter */
            if (meter->index == -1)
                continue;

            if (!meter_drop_counter_get(meter))
            {
                no_drop_time_upd(meter);

                if (meter->meter_no_drop_time < config.meter_remove_timeout_ms)
                    continue;

                meter_remove(meter);
                log_debug("Removing meter; meter_no_drop_time %d\n", meter->meter_no_drop_time);
            }
            else
            {
                rate_increase_on_no_drops(meter, queued);
            }
        }
    }

    printk("%s: thread exits!\n", __func__);
    return 0;
}

static int config_param_index_get(char *name)
{
    int i;

    for (i = 0; config_names[i][CONFIG_NAME_ID] && strcmp(config_names[i][CONFIG_NAME_ID], name); i++);

    return config_names[i][CONFIG_NAME_ID] ? i : -1;
}

static ssize_t help_read(struct file *file, char *buff, size_t len, loff_t *offset)
{
    int i;

    if (*offset)
        return 0;

    for (i = 0; config_names[i][CONFIG_NAME_ID] && *offset < (len - 64); i++)
        *offset += sprintf(buff + *offset, "%s: %s\n",  config_names[i][CONFIG_NAME_ID], config_names[i][CONFIG_HELP_ID]);

    return *offset;
}

static ssize_t stats_read(struct file *file, char *buff, size_t len, loff_t *offset)
{
    if (*offset)
        return 0;

    *offset = sprintf(buff, "Dynamic meter statistics (clear on read):\n\tRate down: %u\n\tRate up: %u\n\tMeter remove: %u\n", 
            stats.rate_down, stats.rate_up, stats.meter_clear);

    stats.rate_down = stats.rate_up = stats.meter_clear = 0;

    return *offset;
}

static ssize_t config_get(struct file *file, char *buff, size_t len, loff_t *offset)
{
    int i;

    if (*offset)
        return 0;

    if ((i = config_param_index_get(file->f_path.dentry->d_iname)) == -1)
        return 0;

    *offset = sprintf(buff, "%u\n", config.param[i]);

    return *offset;
}

static ssize_t config_set(struct file *file, const char *buff, size_t len, loff_t *offset)
{
    char input[PROC_CMD_MAX_LEN];
    int ret, i;

    if (len > PROC_CMD_MAX_LEN)
        len = PROC_CMD_MAX_LEN;

    if ((i = config_param_index_get(file->f_path.dentry->d_iname)) == -1)
        return len;

    if (copy_from_user(input, buff, len) != 0)
        return -EFAULT;

    ret = sscanf(input, "%u", &config.param[i]);
    printk("\n%s changed to %d\n", config_names[i][CONFIG_NAME_ID], config.param[i]);

    if (!config.enabled && (dyn_meter_data.index != -1))
    {
        log_debug("Removing meter %d\n", dyn_meter_data.index);
        meter_remove(&dyn_meter_data);
    }

    return len;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,10,0))
static struct proc_ops config_proc_ops = {
    .proc_read  = config_get,
    .proc_write = config_set,
};

static struct proc_ops help_proc_ops = {
    .proc_read  = help_read,
};

static struct proc_ops stats_proc_ops = {
    .proc_read  = stats_read,
};
#else
static struct file_operations config_proc_ops = {
    .read  = config_get,
    .write = config_set,
};

static struct file_operations help_proc_ops = {
    .read  = help_read,
};

static struct file_operations stats_proc_ops = {
    .read  = stats_read,
};
#endif

static void proc_entries_create(void)
{
    struct proc_dir_entry *dir;
    int i;

    dir = proc_mkdir("driver/dyn_meter", NULL);
    for (i = 0; config_names[i][CONFIG_NAME_ID]; i++)
        proc_create(config_names[i][CONFIG_NAME_ID], S_IWUSR | S_IRUSR, dir, &config_proc_ops);

    proc_create("help",  S_IRUSR, dir, &help_proc_ops);
    proc_create("stats", S_IRUSR, dir, &stats_proc_ops);
}

void dynamic_meters_init(bdmf_object_handle cpu_obj, int watch_qid)
{
    rdpa_cpu_port cpu_port;

    /* only 1 CPU object is supported for now */
    if (dyn_meter_data.poll_thread)
        return;

    proc_entries_create();

    dyn_meter_data.cpu_obj = cpu_obj;
    dyn_meter_data.watch_qid = watch_qid;

    rdpa_cpu_index_get(cpu_obj, &cpu_port);

    printk("Creating dynamic meter on cpu/index=%d queue %d\n", cpu_port, watch_qid);
    dyn_meter_data.poll_thread = kthread_run(dyn_meter_poll_handler, cpu_obj, "dyn_meter_poll");
}

void dynamic_meters_uninit(bdmf_object_handle cpu_obj)
{
    kthread_stop(dyn_meter_data.poll_thread);
    dyn_meter_data.poll_thread = NULL;
} 
