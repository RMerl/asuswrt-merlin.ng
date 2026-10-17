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

#ifndef ___SCORE_QUEUE_H___
#define ___SCORE_QUEUE_H___

#include <linux/bcm_log.h>
#include <linux/blog.h>
#include <linux/poll.h>
#include "score_hw.h"
#include "bcm_score.h"

/* module initialiser */
int score_queue_init(struct device * alloc_device);

/* inform data available related to dcp */
void score_signal_dcp(int dcp_id);
/* inform ctrl message available related to dcp */
int score_post_ctrl_message_dcp(uint32_t cmd, void *data, int size, int dcp_id) __attribute__((unused));

/* allocates ctrl queue, shared with userspace */
int score_ctrl_init(struct file *filep, score_cfg_data_t *cfg);

/* allocates sdr, shared with userspace */
int score_sdr_create(struct file *filep, score_dcp_cfg_data_t *score_dcp_cfg_data, score_sdr_cfg_t *sdr);
/* deallocates sdr */
void score_sdr_destroy(uint8_t dcp_id);

/* file_operations callback functions */
__poll_t score_poll(struct file *file, poll_table *wait);
int score_mmap(struct file *filep, struct vm_area_struct *vma);
int score_open(struct inode *inodep, struct file *filep);
int score_release(struct inode *inodep, struct file *filep);
#endif
