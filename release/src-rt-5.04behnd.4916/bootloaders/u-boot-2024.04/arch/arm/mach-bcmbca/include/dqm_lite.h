/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#ifndef __THIN_DQM_H__
#define __THIN_DQM_H__

#define SMC_WORDS_PER_TOKEN	4

enum dqm_dev_idx {
	DQM_DEV_SMC,
	NUM_DQM_DEVS
};

void bcm_dqm_init(void);
int dqm_open(enum dqm_dev_idx dev_idx, int q_idx);
int dqm_read(int q_id, unsigned char *buffer, int count);
int dqm_write(int q_id, unsigned char *buffer, int count);

#endif
