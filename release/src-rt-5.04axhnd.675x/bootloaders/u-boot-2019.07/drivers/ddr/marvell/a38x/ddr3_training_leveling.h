/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_TRAINING_LEVELING_H_
#define _DDR3_TRAINING_LEVELING_H_

#define MAX_DQ_READ_LEVELING_DELAY 15

int ddr3_tip_print_wl_supp_result(u32 dev_num);
int ddr3_tip_calc_cs_mask(u32 dev_num, u32 if_id, u32 effective_cs,
			  u32 *cs_mask);

#endif /* _DDR3_TRAINING_LEVELING_H_ */
