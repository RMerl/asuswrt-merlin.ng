/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Gateworks Corporation
 */

#ifndef __LPDDR4_TIMING_H__
#define __LPDDR4_TIMING_H__

#ifdef CONFIG_IMX8MM
extern struct dram_timing_info dram_timing_512mb;
extern struct dram_timing_info dram_timing_1gb;
extern struct dram_timing_info dram_timing_2gb;
extern struct dram_timing_info dram_timing_4gb;
#elif CONFIG_IMX8MN
extern struct dram_timing_info dram_timing_1gb_single_die;
extern struct dram_timing_info dram_timing_2gb_single_die;
extern struct dram_timing_info dram_timing_2gb_dual_die;
#elif CONFIG_IMX8MP
extern struct dram_timing_info dram_timing_1gb_single_die;
extern struct dram_timing_info dram_timing_4gb_dual_die;
#endif

#endif /* __LPDDR4_TIMING_H__ */
