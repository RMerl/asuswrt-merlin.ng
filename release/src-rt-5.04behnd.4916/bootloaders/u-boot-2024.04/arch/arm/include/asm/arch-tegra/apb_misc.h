/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2012 The Chromium OS Authors.
 */

#ifndef _GP_PADCTRL_H_
#define _GP_PADCTRL_H_

/* APB_MISC_PP registers */
struct apb_misc_pp_ctlr {
	u32	reserved0[2];
	u32	strapping_opt_a;/* 0x08: APB_MISC_PP_STRAPPING_OPT_A */
	u32	reserved1[6];	/* 0x0c .. 0x20 */
	u32	cfg_ctl;	/* 0x24 */
};

/* bit fields definitions for APB_MISC_PP_STRAPPING_OPT_A register */
#define RAM_CODE_SHIFT		4
#define RAM_CODE_MASK		(0xf << RAM_CODE_SHIFT)

#endif
