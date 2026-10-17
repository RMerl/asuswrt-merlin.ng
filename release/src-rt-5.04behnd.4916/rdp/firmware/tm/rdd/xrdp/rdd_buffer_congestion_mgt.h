/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#ifndef _RDD_BUFFER_CONGESTION_MGT_H
#define _RDD_BUFFER_CONGESTION_MGT_H

#include "rdd.h"
#include "rdp_common.h"
#include "rdd_runner_proj_defs.h"

/* Set default value for timer duration if there is no project-specific override */
#ifndef BUFFER_CONG_MGT_TIMER_DURATION_US
#define BUFFER_CONG_MGT_TIMER_DURATION_US    40 /* in us */
#endif

#define BUFFER_CONG_MGT_DEFAULT_UG_HIGH_THRESHOLD  1024
#define BUFFER_CONG_MGT_HIGH_THRESHOLD_PERCENT     90
#define BUFFER_CONG_MGT_LOW_THRESHOLD_PERCENT      85
#define BUFFER_CONG_MGT_NUM_PDS_TO_FLUSH           64

/* GEN6.2 checks the WIRED counter to see if BCM should be started or not.
 * Now since this is counter is relevant to multiple cores, we may see a scenarios where, e.g.,:
 * DS_0 has an issue but DS_1 and DS_2 are also triggered.
 * Until the WIRED counter will be split per core (WIRED_0, WIRED_1, ...) we use a mechanism so that
 * cores that were triggered with no apparent issue will ignore small queues
 */
#define BUFFER_CONG_MGT_MIN_QUEUE_BYTES            (1*1024*1024)     /* queues with lower occupancy will be ignored */
#define BUFFER_CONG_MGT_MIN_QUEUE_BYTES_MIN        (0.5*1024*1024)   /* minimal value */
#define BUFFER_CONG_MGT_MIN_QUEUE_BYTES_MAX        (8*1024*1024)     /* maximal value */

typedef struct
{
   uint32_t congestion_detected;    /* Number of times congestion state was detected */
   uint32_t congestion_cleared;     /* Number of times congestion state was cleared */
   uint32_t run_time;               /* Duration (us) of the last run where congestion was detected */
   uint16_t cong_queue_idx;         /* Last congested queue index */
   uint16_t cong_queue_occupancy;   /* Last congested queue index */
   int is_enabled;                  /* 1 if enabled */
   int is_congested;                /* 1 if congested */
} rdd_buffer_congestion_mgt_stat;

/* API to RDPA level */
bdmf_error_t rdd_buffer_congestion_mgt_init(fpm_ug_id_e ug);
bdmf_error_t rdd_buffer_congestion_mgt_enable(fpm_ug_id_e ug, bdmf_boolean enable);

/* RDD debug functions */
/* Read debug statistics */
#if (CHIP_VER >= RDP_GEN_62)
bdmf_error_t rdd_buffer_congestion_mgt_stat_get(tm_identifier_e tm_identity, rdd_buffer_congestion_mgt_stat *stats);
#else
bdmf_error_t rdd_buffer_congestion_mgt_stat_get(fpm_ug_id_e ug, rdd_buffer_congestion_mgt_stat *stats);
#if (CHIP_VER >= RDP_GEN_60)
void rdd_buffer_congestion_mgt_stat_get_sq(rdd_buffer_congestion_mgt_stat *stats);
#endif
#endif
/* Read thresholds */
bdmf_error_t rdd_buffer_congestion_mgt_thresholds_get(fpm_ug_id_e ug, uint32_t *p_high_threshold, uint32_t *p_low_threshold);

/* Set thresholds */
bdmf_error_t rdd_buffer_congestion_mgt_thresholds_set(fpm_ug_id_e ug, uint32_t high_threshold, uint32_t low_threshold);

/* Read num PDs to flush */
bdmf_error_t rdd_buffer_congestion_mgt_num_pds_get(fpm_ug_id_e ug, uint8_t *num_pds);

/* Set num PDs to flush */
bdmf_error_t rdd_buffer_congestion_mgt_num_pds_set(fpm_ug_id_e ug, uint8_t num_pds);

/* Get/Set min queue bytes */
bdmf_error_t rdd_buffer_congestion_mgt_min_queue_bytes_set(fpm_ug_id_e ug, uint32_t min_queue_bytes);
bdmf_error_t rdd_buffer_congestion_mgt_min_queue_bytes_get(fpm_ug_id_e ug, uint32_t *min_queue_bytes);

#if (CHIP_VER >= RDP_GEN_60)
bdmf_error_t get_tm_range(fpm_ug_id_e ug, tm_identifier_e *tm_start, tm_identifier_e *tm_end);
#endif

#endif

