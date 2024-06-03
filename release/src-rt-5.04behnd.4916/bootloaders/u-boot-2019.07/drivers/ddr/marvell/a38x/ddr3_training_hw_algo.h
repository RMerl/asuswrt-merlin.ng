/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR3_TRAINING_HW_ALGO_H_
#define _DDR3_TRAINING_HW_ALGO_H_

int ddr3_tip_vref(u32 dev_num);
int ddr3_tip_write_additional_odt_setting(u32 dev_num, u32 if_id);
int ddr3_tip_cmd_addr_init_delay(u32 dev_num, u32 adll_tap);

#endif /* _DDR3_TRAINING_HW_ALGO_H_ */
