/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon dbg.
 */

#ifndef __CVMX_DBG_DEFS_H__
#define __CVMX_DBG_DEFS_H__

#define CVMX_DBG_DATA (0x00011F00000001E8ull)

/**
 * cvmx_dbg_data
 *
 * DBG_DATA = Debug Data Register
 *
 * Value returned on the debug-data lines from the RSLs
 */
union cvmx_dbg_data {
	u64 u64;
	struct cvmx_dbg_data_s {
		u64 reserved_23_63 : 41;
		u64 c_mul : 5;
		u64 dsel_ext : 1;
		u64 data : 17;
	} s;
};

typedef union cvmx_dbg_data cvmx_dbg_data_t;

#endif
