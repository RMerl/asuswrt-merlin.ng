/*
    <:copyright-BRCM:2014-2016:DUAL/GPL:standard
    
       Copyright (c) 2014-2016 Broadcom 
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

#ifndef _RDD_DEBUG_DAEMON_H
#define _RDD_DEBUG_DAEMON_H

#if defined(DEBUG_DAEMON) && !defined(RDP_SIM)

#include <linux/kthread.h>
#include <linux/bcm_realtime.h>
#include "rdpa_types.h"
#include "rdpa_api.h"
#include "bdmf_interface.h"
#include "bdmf_dev.h"

#define  DAEMON_MAJOR_VER  1
#define  DAEMON_MINOR_VER  100

#define  DEBUG_MAX_NUMBER_OF_SETS  256
#define  DEBUG_DAEMON_DDR_SIZE  (64*1024)  /* 64K */
#define  MAGIC_WORD_NEW_BATCH 0xFEEDBABA
#define  MAGIC_HWORD_NEW_SET 0xFFFF
#define  MAGIC_HWORD_NEW_EVENT 0xFFFF
#define  MAGIC_WORD_OVERFLOW 0xDEADBABA
#define  MAX_FILE_SIZE (1*1024*1024)  /* 1MB */

#define NUM_OF_ELEMENTS(X) (sizeof(X)/sizeof(X[0]))

#define DEBUG_TRACE(fmt, args...) do { if (debug_file)  bdmf_trace(fmt, ##args); } while (0)


/* Helper macro to count the number of arguments */
#define NARGS_SEQ(_1, _2, _3, _4, _5, N, ...) N
#define NARGS(...) NARGS_SEQ(, ##__VA_ARGS__, 5, 4, 3, 2, 1)

/* Macro to call the function conditionally based on the number of arguments */
#if defined(LOG_EVENT)
#undef LOG_EVENT
#endif

#define LOG_EVENT(superset, set_idx, event_id, ...) \
    log_event(superset, set_idx, event_id, (NARGS(__VA_ARGS__) - 1), __VA_ARGS__)

typedef enum {
    stats_type,
    event_type,
} debug_set_type_t;

typedef enum {
    normal_counter,
    current_level,
    clear_on_read_counter,
} debug_field_type_t;

typedef enum {
    dhd_set,
    port_set,
    cpu_set,
    system_set,
    hw_mon_set,
    dcl_set,
} debug_superset_t;

typedef enum {
    INFO_LOW_EVENT,
    INFO_MED_EVENT,
    INFO_HIGH_EVENT,
    WARNING_EVENT,
    ERROR_EVENT,
} debug_level_t;

typedef enum {
   ev_fr_flush,
   ev_fr_disable,
   ev_fr_enable,
   ev_fr_update,
} dhd_events_t;

typedef enum {
   ev_sbpm_buf_underrun,
   ev_sbpm_multicast_overflow ,
   ev_sbpm_free_with_context_err,
   ev_sbpm_free_without_context_err,
   ev_invalid_in2e_underflow,
   ev_invalid_in2e_overflow,
   ev_invalid_in2e,
   ev_alloc_null,
   ev_bac_underrun_ug0,
   ev_bac_underrun_ug1,
   ev_got_null_on_multi_get_next,
   ev_connect_null_buffer,
   ev_dma_illegal_status,
   ev_prediction_overrun,
   ev_fit_fail_sts,
   ev_qm_dqm_pop_on_empty,
   ev_qm_dqm_push_on_full,
   ev_qm_cpu_pop_on_empty,
   ev_qm_cpu_push_on_full,
   ev_qm_normal_queue_pd_no_cred,
   ev_qm_non_delayed_queue_pd_no,
   ev_qm_1588_multicast_contradi,
   ev_qm_agg_coherent_inconsiste,
   ev_qm_force_copy_on_non_delay,
   ev_qm_fpm_pool_size_nonexiste,
   ev_qm_total_byte_cnt_underrun,
   ev_qm_total_pkt_cnt_underrun,
   ev_qm_fpm_ug0_underrun,
   ev_qm_fpm_ug1_underrun,
   ev_qm_fpm_ug2_underrun,
   ev_qm_fpm_ug3_underrun,
   ev_fpm_overflow,
   ev_fpm_underflow,
   ev_fpm_invalid_token_freed,
   ev_fpm_invalid_mcast_token,
   ev_bbh_tx_pddrop,
   ev_bbh_tx_aggrlenerr,
   ev_bbh_tx_msgdrop,
   ev_bbh_tx_stsdrop,
   ev_bbh_tx_lenerr,
   ev_c_stack_overflow,
   ev_bufmng_neg_err,
   ev_dcl_failure,
   ev_dcl_update,
} sanity_events_t;

typedef struct
{
    uint8_t valid;
    uint8_t stat_event_id;
    uint8_t set_index;
    debug_superset_t superset;
    debug_set_type_t set_type;
    char description[64];
    uint8_t stats_amount;
    uint16_t buf_length;
    void *helper_func;
} monitor_cfg_struct_t;


typedef struct
{
    char name[64];
    char description[128];
    uint8_t field_size;
    debug_field_type_t field_type;
} monitor_set_struct_t;

typedef struct
{
    char name[64];
    char description[128];
    debug_level_t lvl;
    char param_1[64];
    char param_2[64];
    char param_3[64];
} monitor_event_struct_t;

typedef struct
{
    char name[64];
    debug_set_type_t set_type;
    uint8_t num_of_fields;
    uint8_t is_allocated;
} monitor_set_info_t;


typedef int get_stats_func(uint32_t set_index, uint32_t stats_amount, uint8_t *stat_buf);


extern uint32_t total_mon_sets_num;
extern uint32_t total_cfg_set_num;

extern monitor_cfg_struct_t monitor_cfg[DEBUG_MAX_NUMBER_OF_SETS];
extern void *monitor_set_cfg[DEBUG_MAX_NUMBER_OF_SETS];
extern monitor_set_info_t monitor_set_info[DEBUG_MAX_NUMBER_OF_SETS];

extern struct task_struct *runner_debug_daemon_task;
extern struct task_struct *create_runner_debug_daemon_task(void);
extern void rdd_debug_cmds_handle(void);
extern bdmf_session_handle dummy_session_handle;

int rdd_debug_daemon_init(int superset_bitmask, uint32_t prints_on);
void rdd_debug_daemon_deinit(void);

int rdd_debug_daemon_start_thread(uint32_t period);
void rdd_debug_daemon_stop_thread(void);

int build_debug_daemon_tables(int superset_bitmask);
void destroy_debug_daemon_tables(void);

void log_event(debug_superset_t superset, int set_idx, int event_id, int param_num, ...);
void check_periodic_events(void);

#endif

#endif
