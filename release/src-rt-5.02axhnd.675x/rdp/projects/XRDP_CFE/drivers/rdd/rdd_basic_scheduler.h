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

#ifndef _RDD_BASIC_SCHEDULER_H
#define _RDD_BASIC_SCHEDULER_H

#include "rdd.h"

#define BASIC_SCHEDULER

#define BASIC_SCHEDULER_NUM_OF_QUEUES       8
#define BASIC_SCHEDULER_FULL_BUDGET_VECTOR  0xff

typedef enum
{
    basic_scheduler_full_dwrr = 0,
    basic_scheduler_2sp_6dwrr = 1,
    basic_scheduler_4sp_4dwrr = 2,
    basic_scheduler_full_sp = 3,
    basic_scheduler_num_of_dwrr_offset
} basic_scheduler_dwrr_offset_t;

typedef struct
{
    uint8_t dwrr_offset;
    uint8_t bbh_queue_index;
} basic_scheduler_cfg_t;

typedef struct
{
    uint8_t qm_queue_index;
    uint8_t queue_scheduler_index;
    quantum_number_t quantum_number;
} basic_scheduler_queue_t;

/* API to RDPA level */
bdmf_error_t rdd_basic_scheduler_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, basic_scheduler_cfg_t *cfg); // call only when basic scheduler under bbh queue
bdmf_error_t rdd_basic_scheduler_queue_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, basic_scheduler_queue_t *queue);
bdmf_error_t rdd_basic_scheduler_queue_remove(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t queue_scheduler_index);
bdmf_error_t rdd_basic_scheduler_dwrr_offset_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t dwrr_offset);

/* API to complex scheduler module */
bdmf_error_t rdd_basic_scheduler_dwrr_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, quantum_number_t quantum_number);
bdmf_error_t rdd_basic_scheduler_block_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, rdd_scheduling_queue_descriptor_t *scheduler_cfg, uint8_t dwrr_offset);

/* API to rate limiter module */
bdmf_error_t rdd_basic_scheduler_rate_limiter_cfg(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t rate_limiter_index);
bdmf_error_t rdd_basic_scheduler_rate_limiter_remove(rdpa_traffic_dir dir, uint8_t basic_scheduler_index);

/* API to queue */
bdmf_error_t rdd_basic_scheduler_rate_set(rdpa_traffic_dir dir, uint8_t basic_scheduler_index, uint8_t queue_bit_mask);

#endif
