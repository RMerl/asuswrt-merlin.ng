/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_LMT_H__
#define __CSRS_LMT_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * LMT.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Register (RVU_PFVF_BAR2) lmt_lf_lmtcancel
 *
 * RVU VF LMT Cancel Register
 */
union lmt_lf_lmtcancel {
	u64 u;
	struct lmt_lf_lmtcancel_s {
		u64 data                             : 64;
	} s;
	/* struct lmt_lf_lmtcancel_s cn; */
};

static inline u64 LMT_LF_LMTCANCEL(void)
	__attribute__ ((pure, always_inline));
static inline u64 LMT_LF_LMTCANCEL(void)
{
	return 0x400;
}

/**
 * Register (RVU_PFVF_BAR2) lmt_lf_lmtline#
 *
 * RVU VF LMT Line Registers
 */
union lmt_lf_lmtlinex {
	u64 u;
	struct lmt_lf_lmtlinex_s {
		u64 data                             : 64;
	} s;
	/* struct lmt_lf_lmtlinex_s cn; */
};

static inline u64 LMT_LF_LMTLINEX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 LMT_LF_LMTLINEX(u64 a)
{
	return 0 + 8 * a;
}

#endif /* __CSRS_LMT_H__ */
