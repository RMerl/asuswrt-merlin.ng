/*
    <:copyright-BRCM:2015:DUAL/GPL:standard

       Copyright (c) 2015 Broadcom
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

#if defined(DEBUG_DAEMON) && !defined(RDP_SIM)

#include "rdd.h"
#include "rdd_debug_daemon.h"
#include "rdp_mm.h"
#include "bdmf_session.h"
#include "rdp_drv_sbpm.h"

struct task_struct *runner_debug_daemon_task;

uint8_t *monitor_data[DEBUG_MAX_NUMBER_OF_SETS];
uint8_t *prev_monitor_data[DEBUG_MAX_NUMBER_OF_SETS];
void *debug_daemon_logs_buffer;
uint32_t *cur_logs_buffer_ptr;
uint32_t debug_daemon_period = 1000;

monitor_cfg_struct_t monitor_cfg[DEBUG_MAX_NUMBER_OF_SETS];
void *monitor_set_cfg[DEBUG_MAX_NUMBER_OF_SETS];
monitor_set_info_t monitor_set_info[DEBUG_MAX_NUMBER_OF_SETS];

uint32_t total_mon_sets_num = 0;
uint32_t total_cfg_set_num = 0;

typedef int get_stats_func(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf);

int debug_file = 0;
int total_bytes = 0;
bdmf_file data_file = 0;

bdmf_session_handle dummy_session_handle = NULL;

/*  Save dynamically built configurations to files
 *
 *   rlog_cfg.txt - configuration file
 *   -----------
 *   rlog_data.txt - binary output file
 *
 */

static void save_cfg_tables(void)
{
    int i, j;
    bdmf_file cfg_file = 0;
    char cfg_fname[] = "/data/rlog_cfg.txt";
    char data_fname[] = "/data/rlog_data.txt";
    char data_buf[256] = {};
    monitor_set_struct_t *stat_cfg_ptr;
    monitor_event_struct_t *event_cfg_ptr;
    struct timespec64 cur_time;

    cfg_file = bdmf_file_open(cfg_fname, BDMF_FMODE_WRONLY | BDMF_FMODE_CREATE | BDMF_FMODE_TRUNCATE);
    data_file = bdmf_file_open(data_fname, BDMF_FMODE_WRONLY | BDMF_FMODE_CREATE | BDMF_FMODE_TRUNCATE);

    if ((!cfg_file) || (!data_file))
    {
        bdmf_trace("output files creation failed!\n");
        if (data_file)
            bdmf_file_close(data_file);
        if (cfg_file)
            bdmf_file_close(cfg_file);
        return;
    }

    ktime_get_real_ts64(&cur_time);

    snprintf(data_buf, sizeof(data_buf) - 1, "Runner debug daemon configuration, version: %d.%d\n", DAEMON_MAJOR_VER, DAEMON_MINOR_VER);
    bdmf_file_write(cfg_file, data_buf, strlen(data_buf));
    DEBUG_TRACE(data_buf);

    snprintf(data_buf, sizeof(data_buf) - 1, "Time(sec): %lld\n", cur_time.tv_sec);
    bdmf_file_write(cfg_file, data_buf, strlen(data_buf));
    DEBUG_TRACE(data_buf);

    snprintf(data_buf, sizeof(data_buf) - 1, "==== Monitor table ====\n");
    bdmf_file_write(cfg_file, data_buf, strlen(data_buf));

    DEBUG_TRACE(data_buf);
    DEBUG_TRACE("id valid set_id set_index superset_id type description       amount size func_addr\n");

    for (i = 0; i < total_mon_sets_num; i++)
    {
        snprintf(data_buf, sizeof(data_buf) - 1, "%d %d %d %d %d %d \"%s\" %d\n",
            i,
            monitor_cfg[i].valid,
            monitor_cfg[i].stat_event_id,
            monitor_cfg[i].set_index,
            monitor_cfg[i].superset,
            monitor_cfg[i].set_type,
            monitor_cfg[i].description,
            monitor_cfg[i].stats_amount);
        bdmf_file_write(cfg_file, data_buf, strlen(data_buf));

        DEBUG_TRACE("%-2d %-2d %-2d %-2d %-2d %-2d %-20s %-3d %-3d %p\n",
            i,
            monitor_cfg[i].valid,
            monitor_cfg[i].stat_event_id,
            monitor_cfg[i].set_index,
            monitor_cfg[i].superset,
            monitor_cfg[i].set_type,
            monitor_cfg[i].description,
            monitor_cfg[i].stats_amount,
            monitor_cfg[i].buf_length,
            monitor_cfg[i].helper_func);
    }

    DEBUG_TRACE("\n==== Configs sets table ====\n");
    for (i = 0; i < total_cfg_set_num; i++)
    {
        DEBUG_TRACE("\n** Configs set table %d, type %d name: %s, num_fields: %d\n",
            i, monitor_set_info[i].set_type, monitor_set_info[i].name,
            monitor_set_info[i].num_of_fields);

        snprintf(data_buf, sizeof(data_buf) - 1, "**Config %d type %d name \"%s\" num_fields %d\n",
            i, monitor_set_info[i].set_type, monitor_set_info[i].name,
            monitor_set_info[i].num_of_fields);
        bdmf_file_write(cfg_file, data_buf, strlen(data_buf));

        DEBUG_TRACE("id   name   description   field_size field_type\n");
        for (j = 0; j < monitor_set_info[i].num_of_fields; j++)
        {
            if (monitor_set_info[i].set_type == stats_type)
            {
                /* stats configuration type */
                stat_cfg_ptr = (monitor_set_struct_t *)monitor_set_cfg[i];

                snprintf(data_buf, sizeof(data_buf) - 1, "%d \"%s\" \"%s\" %d %d\n",
                    j,
                    stat_cfg_ptr[j].name,
                    stat_cfg_ptr[j].description,
                    stat_cfg_ptr[j].field_size,
                    stat_cfg_ptr[j].field_type);
                bdmf_file_write(cfg_file, data_buf, strlen(data_buf));

                DEBUG_TRACE("%-2d %-32s %-32s %-2d   %-2d\n",
                    j,
                    stat_cfg_ptr[j].name,
                    stat_cfg_ptr[j].description,
                    stat_cfg_ptr[j].field_size,
                    stat_cfg_ptr[j].field_type);
            }
            else
            {
                /* events configuration type */
                event_cfg_ptr = (monitor_event_struct_t *)monitor_set_cfg[i];

                snprintf(data_buf, sizeof(data_buf) - 1, "%d \"%s\" \"%s\" %d \"%s\" \"%s\" \"%s\"\n",
                    j,
                    event_cfg_ptr[j].name,
                    event_cfg_ptr[j].description,
                    event_cfg_ptr[j].lvl,
                    event_cfg_ptr[j].param_1,
                    event_cfg_ptr[j].param_2,
                    event_cfg_ptr[j].param_3);
                bdmf_file_write(cfg_file, data_buf, strlen(data_buf));

                DEBUG_TRACE("%-2d %-32s %-32s %-2d %-32s %-32s %-32s\n",
                    j,
                    event_cfg_ptr[j].name,
                    event_cfg_ptr[j].description,
                    event_cfg_ptr[j].lvl,
                    event_cfg_ptr[j].param_1,
                    event_cfg_ptr[j].param_2,
                    event_cfg_ptr[j].param_3);
            }
        }
    }

    bdmf_file_close(cfg_file);
}

static uint32_t get_monbuf_len(int cfg_id)
{
    return monitor_cfg[cfg_id].buf_length * monitor_cfg[cfg_id].stats_amount;
}

/*  Built dynamic configurations
 *  based on selected supersets
 *
 */

static int rdd_cfg_tables_init(int superset_bitmask)
{
    int i, rc;

    total_bytes = 0;

    rc = build_debug_daemon_tables(superset_bitmask);
    if (rc)
        return rc;

    if (total_mon_sets_num > DEBUG_MAX_NUMBER_OF_SETS)
        return BDMF_ERR_RANGE;

    save_cfg_tables();

    for (i = 0; i < total_mon_sets_num; i++)
    {
        if (monitor_cfg[i].valid)
        {
            monitor_data[i] =  bdmf_calloc(get_monbuf_len(i));
            prev_monitor_data[i] = bdmf_calloc(get_monbuf_len(i));

            if ((!monitor_data[i]) || (!prev_monitor_data[i]))
                return BDMF_ERR_NOMEM;

            memset(monitor_data[i], 0, get_monbuf_len(i));
            memset(prev_monitor_data[i], 0, get_monbuf_len(i));
        }
    }

    return 0;
}

int rdd_debug_daemon_init(int superset_bitmask, uint32_t prints_on)
{
    bdmf_phys_addr_t phy_addr;
    int rc = 0;

    debug_file = prints_on;

    if (runner_debug_daemon_task)
    {
        bdmf_trace("Runner debug daemon already on\n");
        return BDMF_ERR_ALREADY;
    }

    /*allocate  buffer for debug daemon - must be non-cacheable memory*/
    debug_daemon_logs_buffer = (void *)rdp_mm_aligned_alloc(DEBUG_DAEMON_DDR_SIZE, &phy_addr);
    cur_logs_buffer_ptr = debug_daemon_logs_buffer;

    if (debug_daemon_logs_buffer == NULL)
    {
        bdmf_trace("failed to allocate memory for debug daemon buffer\n");
        return BDMF_ERR_NOMEM;
    }

    bdmf_dummy_session_open(&dummy_session_handle);

    /*GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phy_addr);*/
    rc = rdd_cfg_tables_init(superset_bitmask);

    if (!rc)
    {
        bdmf_trace("Debug daemon init: MemBase=%p Size= %d\n",
          debug_daemon_logs_buffer, DEBUG_DAEMON_DDR_SIZE);
    }
    else
    {
        rdd_debug_daemon_deinit();
        bdmf_trace("Debug daemon init failed\n");
    }

    return rc;
}

void rdd_debug_daemon_deinit(void)
{
    int i;

    if (data_file)
        bdmf_file_close(data_file);

    if (dummy_session_handle)
    {
        bdmf_dummy_session_close(dummy_session_handle);
        dummy_session_handle = NULL;
    }

    destroy_debug_daemon_tables();

    for (i = 0; i < total_mon_sets_num; i++)
    {
        monitor_cfg[i].valid = 0;

        if (monitor_data[i])
        {
            bdmf_free(monitor_data[i]);
        }
        if (prev_monitor_data[i])
        {
            bdmf_free(prev_monitor_data[i]);
        }
    }

    if (debug_daemon_logs_buffer)
    {
        rdp_mm_aligned_free((void *)debug_daemon_logs_buffer, DEBUG_DAEMON_DDR_SIZE);
        cur_logs_buffer_ptr = NULL;
        debug_daemon_logs_buffer = NULL;
    }
}


static inline uint32_t _usec_clock(void)
{
    struct timespec64 cur_time;

    ktime_get_real_ts64(&cur_time);

    return cur_time.tv_sec * 1000000UL + cur_time.tv_nsec/1000;
}

static inline uint32_t data_buf_length(uint32_t *ptr)
{
    return (uintptr_t)ptr - (uintptr_t)debug_daemon_logs_buffer;
}


void log_event(debug_superset_t superset, int set_idx, int event_id, int param_num, ...)
{
    int set_id, set_cfg_id, i;
    va_list args;

    /* logger not started */
    if ((!total_mon_sets_num) || (!cur_logs_buffer_ptr))
        return;

    if ((param_num == 0) || (param_num > 3))
        return;

    for (set_id = 0; set_id < total_mon_sets_num; set_id++)
    {
        if ((!monitor_cfg[set_id].valid) || (monitor_cfg[set_id].set_type != event_type) ||
            (monitor_cfg[set_id].superset != superset) || (monitor_cfg[set_id].set_index != set_idx))
            continue;

        set_cfg_id = monitor_cfg[set_id].stat_event_id;

        *cur_logs_buffer_ptr++ = htonl((MAGIC_HWORD_NEW_EVENT << 16) | set_id);
        *cur_logs_buffer_ptr++ = htonl(_usec_clock());
        *cur_logs_buffer_ptr++ = htonl((event_id << 16) | param_num);

        va_start(args, param_num);

        for (i = 0; i < param_num; i++)
        {
            *cur_logs_buffer_ptr++ = htonl(va_arg(args, uint32_t));
        }

        va_end(args);
        break;
    }
}

/*  Runs over all configured sets
 *  Saves modified fields in the data file
 *
 */

void rdd_debug_cmds_handle()
{
    int rc, set_id, set_cfg_id;
    int found = 0;
    get_stats_func *get_statistics;
    uint8_t *cur_stat_ptr, *prev_stat_ptr;
    uint32_t elem, elem_db_id, cur_stat_elem, prev_stat_elem;
    monitor_set_struct_t *stat_cfg_ptr;

    if (total_bytes > MAX_FILE_SIZE)
    {
        bdmf_trace("Runner debug daemon: max log size reached\n");
        return;
    }

    check_periodic_events();

    *cur_logs_buffer_ptr++ = htonl(MAGIC_WORD_NEW_BATCH);
    *cur_logs_buffer_ptr++ = htonl(_usec_clock());

    for (set_id = 0; set_id < total_mon_sets_num; set_id++)
    {
        cur_stat_ptr = monitor_data[set_id];
        prev_stat_ptr = prev_monitor_data[set_id];

        if ((!monitor_cfg[set_id].valid) || (monitor_cfg[set_id].set_type != stats_type) ||
            (monitor_cfg[set_id].helper_func == NULL))
            continue;

        get_statistics = (get_stats_func *)(monitor_cfg[set_id].helper_func);
        rc = get_statistics(monitor_cfg[set_id].set_index, monitor_cfg[set_id].stats_amount, monitor_data[set_id]);

        if (rc)
        {
            /*bdmf_trace("Error getting stats for set: %s\n", monitor_cfg[set_id].description);*/
            /* Some disconnected ports return error on get status */
            continue;
        }

        elem = 0;
        found = 0;
        set_cfg_id = monitor_cfg[set_id].stat_event_id;
        stat_cfg_ptr = (monitor_set_struct_t *)monitor_set_cfg[set_cfg_id];

        for (elem = 0; elem < monitor_set_info[set_cfg_id].num_of_fields * monitor_cfg[set_id].stats_amount; elem++)
        {
            elem_db_id = (elem % monitor_set_info[set_cfg_id].num_of_fields);

            if (data_buf_length(cur_logs_buffer_ptr) > DEBUG_DAEMON_DDR_SIZE - 1024)
            {
                *cur_logs_buffer_ptr++ = htonl(MAGIC_WORD_OVERFLOW);
                *cur_logs_buffer_ptr++ = htonl(MAGIC_WORD_OVERFLOW);
                break;
            }
            if (cur_stat_ptr >= monitor_data[set_id] + get_monbuf_len(set_id))
            {
                bdmf_trace("Config incorrect for set: %s, elem: %d, total_elem: %d\n",
                    monitor_cfg[set_id].description, elem, monitor_set_info[set_cfg_id].num_of_fields);
                break;
            }

            prev_stat_elem = 0;
            cur_stat_elem = 0;
            switch (stat_cfg_ptr[elem_db_id].field_size)
            {
            case 1:
                cur_stat_elem = *((uint8_t *)cur_stat_ptr);
                prev_stat_elem = *((uint8_t *)prev_stat_ptr);
                break;
            case 2:
                cur_stat_elem = *((uint16_t *)cur_stat_ptr);
                prev_stat_elem = *((uint16_t *)prev_stat_ptr);
                break;
            case 4:
                cur_stat_elem = *((uint32_t *)cur_stat_ptr);
                prev_stat_elem = *((uint32_t *)prev_stat_ptr);
                break;
            default:
                bdmf_trace("Field size not suported: %d, elem_db_id: %d, set: %s\n",
                           stat_cfg_ptr[elem_db_id].field_size, elem_db_id, monitor_cfg[set_id].description);
                break;
            }

            cur_stat_ptr += stat_cfg_ptr[elem_db_id].field_size;
            prev_stat_ptr += stat_cfg_ptr[elem_db_id].field_size;

            if (cur_stat_elem != prev_stat_elem)
            {
                DEBUG_TRACE("Daemon running found diff in set: %s[%d] name: %s = %d\n",
                           monitor_cfg[set_id].description,
                           elem / monitor_set_info[set_cfg_id].num_of_fields,
                           stat_cfg_ptr[elem_db_id].name,
                           cur_stat_elem);

                if (found == 0)
                {
                    found = 1;
                    *cur_logs_buffer_ptr++ = htonl((MAGIC_HWORD_NEW_SET << 16) | set_id);
                }
                *cur_logs_buffer_ptr++ = htonl(elem);
                *cur_logs_buffer_ptr++ = htonl(cur_stat_elem);
            }
        }

        memcpy(prev_monitor_data[set_id], monitor_data[set_id], get_monbuf_len(set_id));
    }

    if (data_buf_length(cur_logs_buffer_ptr) > 0)
    {
        total_bytes += data_buf_length(cur_logs_buffer_ptr);
        if (data_file)
            bdmf_file_write(data_file, debug_daemon_logs_buffer, data_buf_length(cur_logs_buffer_ptr));
    }

    cur_logs_buffer_ptr = (uint32_t *)debug_daemon_logs_buffer;
}


static int runner_debug_daemon_thread_func(void *thread_data)
{
    /* wake up periodically for every perodicity_ms */
    unsigned long timeout_jiffies = msecs_to_jiffies(debug_daemon_period);

    while (!kthread_should_stop())
    {
      rdd_debug_cmds_handle();

      /* TODO: handle many logs */
      if (0)
      {
        /* if too many logs */
        yield();
      }
      else
      {
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(timeout_jiffies);
      }
   }
  return 0;
}


int rdd_debug_daemon_start_thread(uint32_t period)
{
    if (runner_debug_daemon_task == NULL)
    {
        debug_daemon_period = period;
        runner_debug_daemon_task = create_runner_debug_daemon_task();
    }
    else
    {
        bdmf_trace("Runner debug daemon task already run\n");
        return BDMF_ERR_ALREADY;
    }

    return 0;
}

void rdd_debug_daemon_stop_thread(void)
{
    if (runner_debug_daemon_task != NULL)
    {
        kthread_stop(runner_debug_daemon_task);
        runner_debug_daemon_task = NULL;
    }
    else
    {
        bdmf_trace("Runner debug daemon task already stopped\n");
    }
}

struct task_struct *create_runner_debug_daemon_task(void)
{
  struct task_struct *tsk;
  /*struct sched_param param;*/

  bdmf_msleep(1000);

  tsk = kthread_create(runner_debug_daemon_thread_func, NULL, "runner_debug_daemon_task");

  if (IS_ERR(tsk))
  {
    bdmf_print("runner_debug_daemon_task creation failed\n");
    return NULL;
  }

  /*param.sched_priority = BCM_RTPRIO_DATA;*/
  /*sched_setscheduler(tsk, SCHED_RR, &param); */
  wake_up_process(tsk);

  return tsk;
}

#endif
