/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_SRIOMAINTX_DEFS_H__
#define __CVMX_SRIOMAINTX_DEFS_H__

static inline u64 CVMX_SRIOMAINTX_PORT_0_CTL2(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		return 0x0000010000000154ull + (offset) * 0x100000000ull;
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x0000000000000154ull;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x0000000000000154ull + (offset) * 0x100000000ull;
	}
	return 0x0000010000000154ull + (offset) * 0x100000000ull;
}

/**
 * cvmx_sriomaint#_port_0_ctl2
 *
 * These registers are accessed when a local processor or an external
 * device wishes to examine the port baudrate information.  The automatic
 * baud rate feature is not available on this device. The SUP_* and ENB_*
 * fields are set directly by the SRIO()_STATUS_REG[SPD] bits as a
 * reference but otherwise have no effect.
 *
 * WARNING!!  Writes to this register will reinitialize the SRIO link.
 */
union cvmx_sriomaintx_port_0_ctl2 {
	u32 u32;
	struct cvmx_sriomaintx_port_0_ctl2_s {
		u32 sel_baud : 4;
		u32 baud_sup : 1;
		u32 baud_enb : 1;
		u32 sup_125g : 1;
		u32 enb_125g : 1;
		u32 sup_250g : 1;
		u32 enb_250g : 1;
		u32 sup_312g : 1;
		u32 enb_312g : 1;
		u32 sub_500g : 1;
		u32 enb_500g : 1;
		u32 sup_625g : 1;
		u32 enb_625g : 1;
		u32 reserved_2_15 : 14;
		u32 tx_emph : 1;
		u32 emph_en : 1;
	} s;
	struct cvmx_sriomaintx_port_0_ctl2_s cn63xx;
	struct cvmx_sriomaintx_port_0_ctl2_s cn63xxp1;
	struct cvmx_sriomaintx_port_0_ctl2_s cn66xx;
	struct cvmx_sriomaintx_port_0_ctl2_s cnf75xx;
};

typedef union cvmx_sriomaintx_port_0_ctl2 cvmx_sriomaintx_port_0_ctl2_t;

#endif
