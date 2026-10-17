/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_SRIOX_DEFS_H__
#define __CVMX_SRIOX_DEFS_H__

#define CVMX_SRIOX_STATUS_REG(offset) (0x00011800C8000100ull + ((offset) & 3) * 0x1000000ull)

/**
 * cvmx_srio#_status_reg
 *
 * The SRIO field displays if the port has been configured for SRIO operation.  This register
 * can be read regardless of whether the SRIO is selected or being reset.  Although some other
 * registers can be accessed while the ACCESS bit is zero (see individual registers for details),
 * the majority of SRIO registers and all the SRIOMAINT registers can be used only when the
 * ACCESS bit is asserted.
 *
 * This register is reset by the coprocessor-clock reset.
 */
union cvmx_sriox_status_reg {
	u64 u64;
	struct cvmx_sriox_status_reg_s {
		u64 reserved_9_63 : 55;
		u64 host : 1;
		u64 spd : 4;
		u64 run_type : 2;
		u64 access : 1;
		u64 srio : 1;
	} s;
	struct cvmx_sriox_status_reg_cn63xx {
		u64 reserved_2_63 : 62;
		u64 access : 1;
		u64 srio : 1;
	} cn63xx;
	struct cvmx_sriox_status_reg_cn63xx cn63xxp1;
	struct cvmx_sriox_status_reg_cn63xx cn66xx;
	struct cvmx_sriox_status_reg_s cnf75xx;
};

typedef union cvmx_sriox_status_reg cvmx_sriox_status_reg_t;

#endif
