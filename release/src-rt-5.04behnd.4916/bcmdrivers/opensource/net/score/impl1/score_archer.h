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

#ifndef ___SCORE_ARCHER_H___
#define ___SCORE_ARCHER_H___

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/blog.h>
#include "bcm_score_defs.h"
#include "score_qstat.h"

/* The queue configuration to be hardware dependent */
#define SCORE_MSG_QUEUE_SIZE        1024
#define SCORE_QSTAT_QUEUE_SIZE      1024
#define SCORE_MSG_RING_BATCH        1
#define SCORE_QSTAT_RING_BATCH      64

typedef struct {
    void* xyz;
} score_hw_t;

extern score_hw_t score_hw_g;

int score_qstat_cfg_set(qstat_queue_info_t *queue_info, uint32_t enable);
int score_profile_add(uint32_t* dcp_id, const score_profile_cfg_t * profile_);
int score_profile_delete(uint32_t dcp_id);
int score_profile_get(uint32_t dsp_id, score_profile_cfg_t * dcp_cfg);
int score_sdr_add(const score_sdr_info_t * sdr_);
int score_sdr_delete(uint32_t dcp_id);
int score_stream_add(uint32_t* stream_id, score_stream_info_t * stream_);
int score_stream_delete(uint32_t stream_id, uint8_t dcp_id);

int score_msg_ring_create(uint32_t ring_size);
int score_msg_ring_destroy(uint32_t ring_size);
int score_msg_ring_info_get(score_data_t *score_msg_info);

int score_qstat_ring_create(uint32_t ring_size);
int score_qstat_ring_destroy(uint32_t ring_size);
int score_qstat_ring_info_get(score_qstat_data_t *score_qstat_info);
int score_get_wired_queue_info(blog_score_flow_info_t *score_flow_info, qstat_queue_info_t *queue_info);

int score_hw_init(void);
void score_hw_destroy(void);

#endif /* ___SCORE_ARCHER_H___ */
