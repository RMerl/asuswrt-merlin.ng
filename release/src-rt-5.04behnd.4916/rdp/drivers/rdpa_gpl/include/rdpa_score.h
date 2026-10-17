/*
* <:copyright-BRCM:2024:DUAL/GPL:standard
*
*    Copyright (c) 2024 Broadcom
*    All Rights Reserved
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
*
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
*
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
*
* :>
*/

#ifndef _RDPA_SCORE_H_
#define _RDPA_SCORE_H_

#include "bcm_score_defs.h"

/** \defgroup SCORE
 * This object is used to manage the Runner SCORE module \n
 * @{
 */
#define SCORE_MAX_STREAMS_NUMBER    SCORE_NUM_OF_STREAM_IDX
#define SCORE_MAX_PROFILES_NUMBER   SCORE_MAX_DCP_NUM
#define SCORE_MAX_SDRS_NUMBER       SCORE_MAX_PROFILES_NUMBER

typedef void (*score_msg_callback_t)(unsigned long param);

/* Below typedefs can be removed if RDPA code is changed to use score definitions directly */
typedef score_region_t rdpa_score_region_t;
typedef score_stream_cfg_t rdpa_score_stream_cfg_t;
typedef score_stream_info_t rdpa_score_stream_info_t;
typedef score_profile_cfg_t rdpa_score_profile_cfg_t;
typedef score_sdr_info_t rdpa_score_sdr_info_t;
typedef score_data_t rdpa_score_data_t;
typedef score_qstat_data_t rdpa_qstat_data_t;
typedef score_bnl_data_t rdpa_bnl_data_t;

typedef union
{
    uint32_t u32;
    struct {
        uint32_t flowring_idx:16;
        uint32_t radio_idx:2;
        uint32_t enable:1;
        uint32_t reserved:11;
        uint32_t type:2;
    };
} rdpa_qstat_cfg_data_t;

typedef struct _rdpa_score_stats {
    uint32_t total_score_collect_drops;
    uint32_t total_score_report_drops;
    uint32_t total_qstat_report_sent;
    uint32_t total_qstat_report_drops;
    uint32_t total_bnl_report_sent;
    uint32_t total_bnl_report_drops;
    uint32_t total_ctrl_msg_sent;
    uint32_t total_ctrl_msg_drops;
    uint32_t trace_queue_occupancy;
} rdpa_score_stats_t;

/** Create SCORE ring from Runner to drv */
int rdpa_score_msg_ring_create(uint32_t ring_size);

/** Destroy SCORE ring from Runner to drv */
int rdpa_score_msg_ring_destroy(uint32_t ring_size);

/** Return SCORE ring entry */
int rdpa_score_msg_ring_info_get(rdpa_score_data_t *score_msg_info);

/** Create qstat ring from Runner to drv */
int rdpa_score_qstat_ring_create(uint32_t ring_size);

/** Destroy qstat ring from Runner to drv */
int rdpa_score_qstat_ring_destroy(uint32_t ring_size);

/** Return qstat ring entry */
int rdpa_score_qstat_ring_info_get(rdpa_qstat_data_t *score_qstat_info);

/** Create bnl ring from Runner to drv */
int rdpa_score_bnl_ring_create(uint32_t ring_size);

/** Destroy bnl ring from Runner to drv */
int rdpa_score_bnl_ring_destroy(uint32_t ring_size);

/** Return bnl ring entry */
int rdpa_score_bnl_ring_info_get(rdpa_bnl_data_t *score_bnl_info);

/** Current timestamp for calibration */
int rdpa_score_ts_1us_get(uint32_t *ts_1us_p);

#endif /* _RDPA_SCORE_H_ */
