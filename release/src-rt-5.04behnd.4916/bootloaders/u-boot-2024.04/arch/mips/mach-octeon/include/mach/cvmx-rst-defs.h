/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_RST_DEFS_H__
#define __CVMX_RST_DEFS_H__

#define CVMX_RST_CTLX(offset)	    (0x0001180006001640ull + ((offset) & 3) * 8)
#define CVMX_RST_SOFT_PRSTX(offset) (0x00011800060016C0ull + ((offset) & 3) * 8)

/**
 * cvmx_rst_ctl#
 */
union cvmx_rst_ctlx {
	u64 u64;
	struct cvmx_rst_ctlx_s {
		u64 reserved_10_63 : 54;
		u64 prst_link : 1;
		u64 rst_done : 1;
		u64 rst_link : 1;
		u64 host_mode : 1;
		u64 reserved_4_5 : 2;
		u64 rst_drv : 1;
		u64 rst_rcv : 1;
		u64 rst_chip : 1;
		u64 rst_val : 1;
	} s;
	struct cvmx_rst_ctlx_s cn70xx;
	struct cvmx_rst_ctlx_s cn70xxp1;
	struct cvmx_rst_ctlx_s cn73xx;
	struct cvmx_rst_ctlx_s cn78xx;
	struct cvmx_rst_ctlx_s cn78xxp1;
	struct cvmx_rst_ctlx_s cnf75xx;
};

typedef union cvmx_rst_ctlx cvmx_rst_ctlx_t;

/**
 * cvmx_rst_soft_prst#
 */
union cvmx_rst_soft_prstx {
	u64 u64;
	struct cvmx_rst_soft_prstx_s {
		u64 reserved_1_63 : 63;
		u64 soft_prst : 1;
	} s;
	struct cvmx_rst_soft_prstx_s cn70xx;
	struct cvmx_rst_soft_prstx_s cn70xxp1;
	struct cvmx_rst_soft_prstx_s cn73xx;
	struct cvmx_rst_soft_prstx_s cn78xx;
	struct cvmx_rst_soft_prstx_s cn78xxp1;
	struct cvmx_rst_soft_prstx_s cnf75xx;
};

typedef union cvmx_rst_soft_prstx cvmx_rst_soft_prstx_t;

/**
 * cvmx_rst_soft_rst
 */
union cvmx_rst_soft_rst {
	u64 u64;
	struct cvmx_rst_soft_rst_s {
		u64 reserved_1_63 : 63;
		u64 soft_rst : 1;
	} s;
	struct cvmx_rst_soft_rst_s cn70xx;
	struct cvmx_rst_soft_rst_s cn70xxp1;
	struct cvmx_rst_soft_rst_s cn73xx;
	struct cvmx_rst_soft_rst_s cn78xx;
	struct cvmx_rst_soft_rst_s cn78xxp1;
	struct cvmx_rst_soft_rst_s cnf75xx;
};

typedef union cvmx_rst_soft_rst cvmx_rst_soft_rst_t;

#endif
