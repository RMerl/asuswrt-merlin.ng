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

#ifndef ___SCORE_QSTAT_H___
#define ___SCORE_QSTAT_H___

#include <linux/bcm_log.h>
#include <bcm_score.h>
#include "score_hw.h"

#define QSTAT_HIST_BIN_WIDTH        8
#define QSTAT_MAX_STATS             64   /* max calculated statistics */
#define QSTAT_MAX_QUEUES            64   /* max monitored queues */

#define QSTAT_MAX_PROFILES          64

#define QSTAT_MAX_IF_NUM            (4 /* WLAN */+ 1 /* WIRED*/)
#define QSTAT_IF_WIRED_INDEX        (QSTAT_MAX_IF_NUM - 1)
#define QSTAT_MAX_QUEUE_NUM         1024

#define QSTAT_INVALID               (0xFFFF)


typedef struct {
    uint8_t radio_idx;
    uint8_t queue_idx;
    qstat_telemetry_data_t bq_occupancy;
    qstat_telemetry_data_t pkt_latency;
    qstat_telemetry_data_t bq_latency;
    qstat_hist_t pkt_latency_hist;
} qstat_wlan_dhd_info_t;

typedef struct {
    qstat_telemetry_data_t pkt_latency;
    qstat_hist_t pkt_latency_hist;
} qstat_wlan_nic_info_t;

typedef struct {
    qstat_counters_t transmitted;
    qstat_counters_t aqm_dropped;

    qstat_telemetry_data_t latency;
    qstat_hist_t latency_hist;
    qstat_counters_t cq_latency_remark;
    qstat_counters_t lq_latency_remark;
} qstat_wired_info_t;

typedef struct {
    qstat_if_type_t if_type;
    uint16_t if_id;
    uint16_t queue_id;

    /* Wired fields */
    void *egress_tm_obj;            /* abstracted bdmf_object_handle of the egress_tm */
    uint8_t queue_index;            /* Queue's index/priority in the egress_tm */
} qstat_queue_info_t;

typedef struct {
    uint8_t ref_cnt;
    uint8_t cfg_id;
    qstat_if_type_t type;
    union {
        qstat_wlan_dhd_info_t dhd_info;
        qstat_wlan_nic_info_t nic_info;
        qstat_wired_info_t wired_info;
    };
} statistics_entry_t;

typedef struct {
    score_stream_key_t score_stream_key;  /* stream_idx + encarn + dir */
    uint16_t dcp_id;
    uint16_t stat_idx;
} mon_entry_t;

typedef struct {
    uint8_t is_alloc;
    qstat_queue_info_t info;
    uint64_t mon_bitmap;
    mon_entry_t monitor[QSTAT_MAX_PROFILES];
} qstat_entry_t;

extern score_qstat_cfg_report_t qstat_cfg_report[QSTAT_MAX_CFG_NUMBER];
extern qstat_entry_t qstat_db[QSTAT_MAX_QUEUES];

void qstat_init(void);
void qstat_uninit(void);
int qstat_cfg_queue(qstat_queue_info_t *queue_info, uint16_t score_stream_key, uint8_t dcp_id, uint8_t enable);
void qstat_process_data(score_qstat_data_t *score_qstat_info);
int qstat_get_stat_results(uint16_t stat_idx, uint8_t reset_on_read, statistics_entry_t *stat_results);
int qstat_get_result_by_streamkey(uint16_t score_stream_key, uint8_t dcp_id,
                                  uint8_t reset_on_read, statistics_entry_t *stat_results);

#endif /* ___SCORE_QSTAT_H___ */
