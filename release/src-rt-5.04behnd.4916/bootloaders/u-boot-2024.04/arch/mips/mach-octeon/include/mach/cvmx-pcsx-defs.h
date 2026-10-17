/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pcsx.
 */

#ifndef __CVMX_PCSX_DEFS_H__
#define __CVMX_PCSX_DEFS_H__

static inline u64 CVMX_PCSX_ANX_ADV_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001010ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001010ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001010ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001010ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_ANX_EXT_ST_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001028ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001028ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001028ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001028ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_ANX_LP_ABIL_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001018ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001018ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001018ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001018ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_ANX_RESULTS_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001020ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001020ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001020ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001020ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_INTX_EN_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001088ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001088ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001088ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001088ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_INTX_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001080ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001080ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001080ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001080ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_LINKX_TIMER_COUNT_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001040ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001040ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001040ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001040ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_LOG_ANLX_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001090ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001090ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001090ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001090ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

#define CVMX_PCSX_MAC_CRDT_CNTX_REG(offset, block_id)                                              \
	(0x00011800B00010B0ull + (((offset) & 3) + ((block_id) & 1) * 0x20000ull) * 1024)
static inline u64 CVMX_PCSX_MISCX_CTL_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001078ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001078ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001078ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001078ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_MRX_CONTROL_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001000ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001000ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001000ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001000ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_MRX_STATUS_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001008ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001008ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001008ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001008ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_RXX_STATES_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001058ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001058ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001058ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001058ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_RXX_SYNC_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001050ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001050ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001050ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001050ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

#define CVMX_PCSX_SERDES_CRDT_CNTX_REG(offset, block_id)                                           \
	(0x00011800B00010A0ull + (((offset) & 3) + ((block_id) & 1) * 0x20000ull) * 1024)
static inline u64 CVMX_PCSX_SGMX_AN_ADV_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001068ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001068ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001068ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001068ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_SGMX_LP_ADV_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001070ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001070ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001070ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001070ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_TXX_STATES_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001060ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001060ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001060ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001060ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

static inline u64 CVMX_PCSX_TX_RXX_POLARITY_REG(unsigned long offset, unsigned long block_id)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001048ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001048ull + ((offset) + (block_id) * 0x20000ull) * 1024;
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0001048ull + ((offset) + (block_id) * 0x4000ull) * 1024;
	}
	return 0x00011800B0001048ull + ((offset) + (block_id) * 0x20000ull) * 1024;
}

/**
 * cvmx_pcs#_an#_adv_reg
 *
 * Bits [15:9] in the Status Register indicate ability to operate as per those signalling specification,
 * when misc ctl reg MAC_PHY bit is set to MAC mode. Bits [15:9] will all, always read 1'b0, indicating
 * that the chip cannot operate in the corresponding modes.
 *
 * Bit [4] RM_FLT is a don't care when the selected mode is SGMII.
 *
 *
 *
 * PCS_AN_ADV_REG = AN Advertisement Register4
 */
union cvmx_pcsx_anx_adv_reg {
	u64 u64;
	struct cvmx_pcsx_anx_adv_reg_s {
		u64 reserved_16_63 : 48;
		u64 np : 1;
		u64 reserved_14_14 : 1;
		u64 rem_flt : 2;
		u64 reserved_9_11 : 3;
		u64 pause : 2;
		u64 hfd : 1;
		u64 fd : 1;
		u64 reserved_0_4 : 5;
	} s;
	struct cvmx_pcsx_anx_adv_reg_s cn52xx;
	struct cvmx_pcsx_anx_adv_reg_s cn52xxp1;
	struct cvmx_pcsx_anx_adv_reg_s cn56xx;
	struct cvmx_pcsx_anx_adv_reg_s cn56xxp1;
	struct cvmx_pcsx_anx_adv_reg_s cn61xx;
	struct cvmx_pcsx_anx_adv_reg_s cn63xx;
	struct cvmx_pcsx_anx_adv_reg_s cn63xxp1;
	struct cvmx_pcsx_anx_adv_reg_s cn66xx;
	struct cvmx_pcsx_anx_adv_reg_s cn68xx;
	struct cvmx_pcsx_anx_adv_reg_s cn68xxp1;
	struct cvmx_pcsx_anx_adv_reg_s cn70xx;
	struct cvmx_pcsx_anx_adv_reg_s cn70xxp1;
	struct cvmx_pcsx_anx_adv_reg_s cnf71xx;
};

typedef union cvmx_pcsx_anx_adv_reg cvmx_pcsx_anx_adv_reg_t;

/**
 * cvmx_pcs#_an#_ext_st_reg
 *
 * as per IEEE802.3 Clause 22
 *
 */
union cvmx_pcsx_anx_ext_st_reg {
	u64 u64;
	struct cvmx_pcsx_anx_ext_st_reg_s {
		u64 reserved_16_63 : 48;
		u64 thou_xfd : 1;
		u64 thou_xhd : 1;
		u64 thou_tfd : 1;
		u64 thou_thd : 1;
		u64 reserved_0_11 : 12;
	} s;
	struct cvmx_pcsx_anx_ext_st_reg_s cn52xx;
	struct cvmx_pcsx_anx_ext_st_reg_s cn52xxp1;
	struct cvmx_pcsx_anx_ext_st_reg_s cn56xx;
	struct cvmx_pcsx_anx_ext_st_reg_s cn56xxp1;
	struct cvmx_pcsx_anx_ext_st_reg_s cn61xx;
	struct cvmx_pcsx_anx_ext_st_reg_s cn63xx;
	struct cvmx_pcsx_anx_ext_st_reg_s cn63xxp1;
	struct cvmx_pcsx_anx_ext_st_reg_s cn66xx;
	struct cvmx_pcsx_anx_ext_st_reg_s cn68xx;
	struct cvmx_pcsx_anx_ext_st_reg_s cn68xxp1;
	struct cvmx_pcsx_anx_ext_st_reg_cn70xx {
		u64 reserved_16_63 : 48;
		u64 thou_xfd : 1;
		u64 thou_xhd : 1;
		u64 thou_tfd : 1;
		u64 thou_thd : 1;
		u64 reserved_11_0 : 12;
	} cn70xx;
	struct cvmx_pcsx_anx_ext_st_reg_cn70xx cn70xxp1;
	struct cvmx_pcsx_anx_ext_st_reg_s cnf71xx;
};

typedef union cvmx_pcsx_anx_ext_st_reg cvmx_pcsx_anx_ext_st_reg_t;

/**
 * cvmx_pcs#_an#_lp_abil_reg
 *
 * as per IEEE802.3 Clause 37
 *
 */
union cvmx_pcsx_anx_lp_abil_reg {
	u64 u64;
	struct cvmx_pcsx_anx_lp_abil_reg_s {
		u64 reserved_16_63 : 48;
		u64 np : 1;
		u64 ack : 1;
		u64 rem_flt : 2;
		u64 reserved_9_11 : 3;
		u64 pause : 2;
		u64 hfd : 1;
		u64 fd : 1;
		u64 reserved_0_4 : 5;
	} s;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn52xx;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn52xxp1;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn56xx;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn56xxp1;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn61xx;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn63xx;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn63xxp1;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn66xx;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn68xx;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn68xxp1;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn70xx;
	struct cvmx_pcsx_anx_lp_abil_reg_s cn70xxp1;
	struct cvmx_pcsx_anx_lp_abil_reg_s cnf71xx;
};

typedef union cvmx_pcsx_anx_lp_abil_reg cvmx_pcsx_anx_lp_abil_reg_t;

/**
 * cvmx_pcs#_an#_results_reg
 *
 * NOTE:
 * an_results_reg is don't care when AN_OVRD is set to 1. If AN_OVRD=0 and AN_CPT=1
 * the an_results_reg is valid.
 */
union cvmx_pcsx_anx_results_reg {
	u64 u64;
	struct cvmx_pcsx_anx_results_reg_s {
		u64 reserved_7_63 : 57;
		u64 pause : 2;
		u64 spd : 2;
		u64 an_cpt : 1;
		u64 dup : 1;
		u64 link_ok : 1;
	} s;
	struct cvmx_pcsx_anx_results_reg_s cn52xx;
	struct cvmx_pcsx_anx_results_reg_s cn52xxp1;
	struct cvmx_pcsx_anx_results_reg_s cn56xx;
	struct cvmx_pcsx_anx_results_reg_s cn56xxp1;
	struct cvmx_pcsx_anx_results_reg_s cn61xx;
	struct cvmx_pcsx_anx_results_reg_s cn63xx;
	struct cvmx_pcsx_anx_results_reg_s cn63xxp1;
	struct cvmx_pcsx_anx_results_reg_s cn66xx;
	struct cvmx_pcsx_anx_results_reg_s cn68xx;
	struct cvmx_pcsx_anx_results_reg_s cn68xxp1;
	struct cvmx_pcsx_anx_results_reg_s cn70xx;
	struct cvmx_pcsx_anx_results_reg_s cn70xxp1;
	struct cvmx_pcsx_anx_results_reg_s cnf71xx;
};

typedef union cvmx_pcsx_anx_results_reg cvmx_pcsx_anx_results_reg_t;

/**
 * cvmx_pcs#_int#_en_reg
 *
 * PCS Interrupt Enable Register
 *
 */
union cvmx_pcsx_intx_en_reg {
	u64 u64;
	struct cvmx_pcsx_intx_en_reg_s {
		u64 reserved_13_63 : 51;
		u64 dbg_sync_en : 1;
		u64 dup : 1;
		u64 sync_bad_en : 1;
		u64 an_bad_en : 1;
		u64 rxlock_en : 1;
		u64 rxbad_en : 1;
		u64 rxerr_en : 1;
		u64 txbad_en : 1;
		u64 txfifo_en : 1;
		u64 txfifu_en : 1;
		u64 an_err_en : 1;
		u64 xmit_en : 1;
		u64 lnkspd_en : 1;
	} s;
	struct cvmx_pcsx_intx_en_reg_cn52xx {
		u64 reserved_12_63 : 52;
		u64 dup : 1;
		u64 sync_bad_en : 1;
		u64 an_bad_en : 1;
		u64 rxlock_en : 1;
		u64 rxbad_en : 1;
		u64 rxerr_en : 1;
		u64 txbad_en : 1;
		u64 txfifo_en : 1;
		u64 txfifu_en : 1;
		u64 an_err_en : 1;
		u64 xmit_en : 1;
		u64 lnkspd_en : 1;
	} cn52xx;
	struct cvmx_pcsx_intx_en_reg_cn52xx cn52xxp1;
	struct cvmx_pcsx_intx_en_reg_cn52xx cn56xx;
	struct cvmx_pcsx_intx_en_reg_cn52xx cn56xxp1;
	struct cvmx_pcsx_intx_en_reg_s cn61xx;
	struct cvmx_pcsx_intx_en_reg_s cn63xx;
	struct cvmx_pcsx_intx_en_reg_s cn63xxp1;
	struct cvmx_pcsx_intx_en_reg_s cn66xx;
	struct cvmx_pcsx_intx_en_reg_s cn68xx;
	struct cvmx_pcsx_intx_en_reg_s cn68xxp1;
	struct cvmx_pcsx_intx_en_reg_s cn70xx;
	struct cvmx_pcsx_intx_en_reg_s cn70xxp1;
	struct cvmx_pcsx_intx_en_reg_s cnf71xx;
};

typedef union cvmx_pcsx_intx_en_reg cvmx_pcsx_intx_en_reg_t;

/**
 * cvmx_pcs#_int#_reg
 *
 * PCS Interrupt Register
 * NOTE: RXERR and TXERR conditions to be discussed with Dan before finalising
 * DBG_SYNC interrupt fires when code group synchronization state machine makes a transition from
 * SYNC_ACQUIRED_1 state to SYNC_ACQUIRED_2 state(See IEEE 802.3-2005 figure 37-9). It is an
 * indication that a bad code group
 * was received after code group synchronizaton was achieved. This interrupt should be disabled
 * during normal link operation.
 * Use it as a debug help feature only.
 */
union cvmx_pcsx_intx_reg {
	u64 u64;
	struct cvmx_pcsx_intx_reg_s {
		u64 reserved_13_63 : 51;
		u64 dbg_sync : 1;
		u64 dup : 1;
		u64 sync_bad : 1;
		u64 an_bad : 1;
		u64 rxlock : 1;
		u64 rxbad : 1;
		u64 rxerr : 1;
		u64 txbad : 1;
		u64 txfifo : 1;
		u64 txfifu : 1;
		u64 an_err : 1;
		u64 xmit : 1;
		u64 lnkspd : 1;
	} s;
	struct cvmx_pcsx_intx_reg_cn52xx {
		u64 reserved_12_63 : 52;
		u64 dup : 1;
		u64 sync_bad : 1;
		u64 an_bad : 1;
		u64 rxlock : 1;
		u64 rxbad : 1;
		u64 rxerr : 1;
		u64 txbad : 1;
		u64 txfifo : 1;
		u64 txfifu : 1;
		u64 an_err : 1;
		u64 xmit : 1;
		u64 lnkspd : 1;
	} cn52xx;
	struct cvmx_pcsx_intx_reg_cn52xx cn52xxp1;
	struct cvmx_pcsx_intx_reg_cn52xx cn56xx;
	struct cvmx_pcsx_intx_reg_cn52xx cn56xxp1;
	struct cvmx_pcsx_intx_reg_s cn61xx;
	struct cvmx_pcsx_intx_reg_s cn63xx;
	struct cvmx_pcsx_intx_reg_s cn63xxp1;
	struct cvmx_pcsx_intx_reg_s cn66xx;
	struct cvmx_pcsx_intx_reg_s cn68xx;
	struct cvmx_pcsx_intx_reg_s cn68xxp1;
	struct cvmx_pcsx_intx_reg_s cn70xx;
	struct cvmx_pcsx_intx_reg_s cn70xxp1;
	struct cvmx_pcsx_intx_reg_s cnf71xx;
};

typedef union cvmx_pcsx_intx_reg cvmx_pcsx_intx_reg_t;

/**
 * cvmx_pcs#_link#_timer_count_reg
 *
 * PCS_LINK_TIMER_COUNT_REG = 1.6ms nominal link timer register
 *
 */
union cvmx_pcsx_linkx_timer_count_reg {
	u64 u64;
	struct cvmx_pcsx_linkx_timer_count_reg_s {
		u64 reserved_16_63 : 48;
		u64 count : 16;
	} s;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn52xx;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn52xxp1;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn56xx;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn56xxp1;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn61xx;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn63xx;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn63xxp1;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn66xx;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn68xx;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn68xxp1;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn70xx;
	struct cvmx_pcsx_linkx_timer_count_reg_s cn70xxp1;
	struct cvmx_pcsx_linkx_timer_count_reg_s cnf71xx;
};

typedef union cvmx_pcsx_linkx_timer_count_reg cvmx_pcsx_linkx_timer_count_reg_t;

/**
 * cvmx_pcs#_log_anl#_reg
 *
 * PCS Logic Analyzer Register
 * NOTE: Logic Analyzer is enabled with LA_EN for the specified PCS lane only. PKT_SZ is
 * effective only when LA_EN=1
 * For normal operation(sgmii or 1000Base-X), this bit must be 0.
 * See pcsx.csr for xaui logic analyzer mode.
 * For full description see document at .../rtl/pcs/readme_logic_analyzer.txt
 */
union cvmx_pcsx_log_anlx_reg {
	u64 u64;
	struct cvmx_pcsx_log_anlx_reg_s {
		u64 reserved_4_63 : 60;
		u64 lafifovfl : 1;
		u64 la_en : 1;
		u64 pkt_sz : 2;
	} s;
	struct cvmx_pcsx_log_anlx_reg_s cn52xx;
	struct cvmx_pcsx_log_anlx_reg_s cn52xxp1;
	struct cvmx_pcsx_log_anlx_reg_s cn56xx;
	struct cvmx_pcsx_log_anlx_reg_s cn56xxp1;
	struct cvmx_pcsx_log_anlx_reg_s cn61xx;
	struct cvmx_pcsx_log_anlx_reg_s cn63xx;
	struct cvmx_pcsx_log_anlx_reg_s cn63xxp1;
	struct cvmx_pcsx_log_anlx_reg_s cn66xx;
	struct cvmx_pcsx_log_anlx_reg_s cn68xx;
	struct cvmx_pcsx_log_anlx_reg_s cn68xxp1;
	struct cvmx_pcsx_log_anlx_reg_s cn70xx;
	struct cvmx_pcsx_log_anlx_reg_s cn70xxp1;
	struct cvmx_pcsx_log_anlx_reg_s cnf71xx;
};

typedef union cvmx_pcsx_log_anlx_reg cvmx_pcsx_log_anlx_reg_t;

/**
 * cvmx_pcs#_mac_crdt_cnt#_reg
 *
 * PCS MAC Credit Count
 *
 */
union cvmx_pcsx_mac_crdt_cntx_reg {
	u64 u64;
	struct cvmx_pcsx_mac_crdt_cntx_reg_s {
		u64 reserved_5_63 : 59;
		u64 cnt : 5;
	} s;
	struct cvmx_pcsx_mac_crdt_cntx_reg_s cn70xx;
	struct cvmx_pcsx_mac_crdt_cntx_reg_s cn70xxp1;
};

typedef union cvmx_pcsx_mac_crdt_cntx_reg cvmx_pcsx_mac_crdt_cntx_reg_t;

/**
 * cvmx_pcs#_misc#_ctl_reg
 *
 * SGMII Misc Control Register
 * SGMII bit [12] is really a misnomer, it is a decode  of pi_qlm_cfg pins to indicate SGMII or
 * 1000Base-X modes.
 * Note: MODE bit
 * When MODE=1,  1000Base-X mode is selected. Auto negotiation will follow IEEE 802.3 clause 37.
 * When MODE=0,  SGMII mode is selected and the following note will apply.
 * Repeat note from SGM_AN_ADV register
 * NOTE: The SGMII AN Advertisement Register above will be sent during Auto Negotiation if the
 * MAC_PHY mode bit in misc_ctl_reg
 * is set (1=PHY mode). If the bit is not set (0=MAC mode), the tx_config_reg[14] becomes ACK bit
 * and [0] is always 1.
 * All other bits in tx_config_reg sent will be 0. The PHY dictates the Auto Negotiation results.
 */
union cvmx_pcsx_miscx_ctl_reg {
	u64 u64;
	struct cvmx_pcsx_miscx_ctl_reg_s {
		u64 reserved_13_63 : 51;
		u64 sgmii : 1;
		u64 gmxeno : 1;
		u64 loopbck2 : 1;
		u64 mac_phy : 1;
		u64 mode : 1;
		u64 an_ovrd : 1;
		u64 samp_pt : 7;
	} s;
	struct cvmx_pcsx_miscx_ctl_reg_s cn52xx;
	struct cvmx_pcsx_miscx_ctl_reg_s cn52xxp1;
	struct cvmx_pcsx_miscx_ctl_reg_s cn56xx;
	struct cvmx_pcsx_miscx_ctl_reg_s cn56xxp1;
	struct cvmx_pcsx_miscx_ctl_reg_s cn61xx;
	struct cvmx_pcsx_miscx_ctl_reg_s cn63xx;
	struct cvmx_pcsx_miscx_ctl_reg_s cn63xxp1;
	struct cvmx_pcsx_miscx_ctl_reg_s cn66xx;
	struct cvmx_pcsx_miscx_ctl_reg_s cn68xx;
	struct cvmx_pcsx_miscx_ctl_reg_s cn68xxp1;
	struct cvmx_pcsx_miscx_ctl_reg_cn70xx {
		u64 reserved_12_63 : 52;
		u64 gmxeno : 1;
		u64 loopbck2 : 1;
		u64 mac_phy : 1;
		u64 mode : 1;
		u64 an_ovrd : 1;
		u64 samp_pt : 7;
	} cn70xx;
	struct cvmx_pcsx_miscx_ctl_reg_cn70xx cn70xxp1;
	struct cvmx_pcsx_miscx_ctl_reg_s cnf71xx;
};

typedef union cvmx_pcsx_miscx_ctl_reg cvmx_pcsx_miscx_ctl_reg_t;

/**
 * cvmx_pcs#_mr#_control_reg
 *
 * NOTE:
 * Whenever AN_EN bit[12] is set, Auto negotiation is allowed to happen. The results
 * of the auto negotiation process set the fields in the AN_RESULTS reg. When AN_EN is not set,
 * AN_RESULTS reg is don't care. The effective SPD, DUP etc.. get their values
 * from the pcs_mr_ctrl reg.
 */
union cvmx_pcsx_mrx_control_reg {
	u64 u64;
	struct cvmx_pcsx_mrx_control_reg_s {
		u64 reserved_16_63 : 48;
		u64 reset : 1;
		u64 loopbck1 : 1;
		u64 spdlsb : 1;
		u64 an_en : 1;
		u64 pwr_dn : 1;
		u64 reserved_10_10 : 1;
		u64 rst_an : 1;
		u64 dup : 1;
		u64 coltst : 1;
		u64 spdmsb : 1;
		u64 uni : 1;
		u64 reserved_0_4 : 5;
	} s;
	struct cvmx_pcsx_mrx_control_reg_s cn52xx;
	struct cvmx_pcsx_mrx_control_reg_s cn52xxp1;
	struct cvmx_pcsx_mrx_control_reg_s cn56xx;
	struct cvmx_pcsx_mrx_control_reg_s cn56xxp1;
	struct cvmx_pcsx_mrx_control_reg_s cn61xx;
	struct cvmx_pcsx_mrx_control_reg_s cn63xx;
	struct cvmx_pcsx_mrx_control_reg_s cn63xxp1;
	struct cvmx_pcsx_mrx_control_reg_s cn66xx;
	struct cvmx_pcsx_mrx_control_reg_s cn68xx;
	struct cvmx_pcsx_mrx_control_reg_s cn68xxp1;
	struct cvmx_pcsx_mrx_control_reg_s cn70xx;
	struct cvmx_pcsx_mrx_control_reg_s cn70xxp1;
	struct cvmx_pcsx_mrx_control_reg_s cnf71xx;
};

typedef union cvmx_pcsx_mrx_control_reg cvmx_pcsx_mrx_control_reg_t;

/**
 * cvmx_pcs#_mr#_status_reg
 *
 * Bits [15:9] in the Status Register indicate ability to operate as per those signalling
 * specification,
 * when misc ctl reg MAC_PHY bit is set to MAC mode. Bits [15:9] will all, always read 1'b0,
 * indicating
 * that the chip cannot operate in the corresponding modes.
 * Bit [4] RM_FLT is a don't care when the selected mode is SGMII.
 */
union cvmx_pcsx_mrx_status_reg {
	u64 u64;
	struct cvmx_pcsx_mrx_status_reg_s {
		u64 reserved_16_63 : 48;
		u64 hun_t4 : 1;
		u64 hun_xfd : 1;
		u64 hun_xhd : 1;
		u64 ten_fd : 1;
		u64 ten_hd : 1;
		u64 hun_t2fd : 1;
		u64 hun_t2hd : 1;
		u64 ext_st : 1;
		u64 reserved_7_7 : 1;
		u64 prb_sup : 1;
		u64 an_cpt : 1;
		u64 rm_flt : 1;
		u64 an_abil : 1;
		u64 lnk_st : 1;
		u64 reserved_1_1 : 1;
		u64 extnd : 1;
	} s;
	struct cvmx_pcsx_mrx_status_reg_s cn52xx;
	struct cvmx_pcsx_mrx_status_reg_s cn52xxp1;
	struct cvmx_pcsx_mrx_status_reg_s cn56xx;
	struct cvmx_pcsx_mrx_status_reg_s cn56xxp1;
	struct cvmx_pcsx_mrx_status_reg_s cn61xx;
	struct cvmx_pcsx_mrx_status_reg_s cn63xx;
	struct cvmx_pcsx_mrx_status_reg_s cn63xxp1;
	struct cvmx_pcsx_mrx_status_reg_s cn66xx;
	struct cvmx_pcsx_mrx_status_reg_s cn68xx;
	struct cvmx_pcsx_mrx_status_reg_s cn68xxp1;
	struct cvmx_pcsx_mrx_status_reg_s cn70xx;
	struct cvmx_pcsx_mrx_status_reg_s cn70xxp1;
	struct cvmx_pcsx_mrx_status_reg_s cnf71xx;
};

typedef union cvmx_pcsx_mrx_status_reg cvmx_pcsx_mrx_status_reg_t;

/**
 * cvmx_pcs#_rx#_states_reg
 *
 * PCS_RX_STATES_REG = RX State Machines states register
 *
 */
union cvmx_pcsx_rxx_states_reg {
	u64 u64;
	struct cvmx_pcsx_rxx_states_reg_s {
		u64 reserved_16_63 : 48;
		u64 rx_bad : 1;
		u64 rx_st : 5;
		u64 sync_bad : 1;
		u64 sync : 4;
		u64 an_bad : 1;
		u64 an_st : 4;
	} s;
	struct cvmx_pcsx_rxx_states_reg_s cn52xx;
	struct cvmx_pcsx_rxx_states_reg_s cn52xxp1;
	struct cvmx_pcsx_rxx_states_reg_s cn56xx;
	struct cvmx_pcsx_rxx_states_reg_s cn56xxp1;
	struct cvmx_pcsx_rxx_states_reg_s cn61xx;
	struct cvmx_pcsx_rxx_states_reg_s cn63xx;
	struct cvmx_pcsx_rxx_states_reg_s cn63xxp1;
	struct cvmx_pcsx_rxx_states_reg_s cn66xx;
	struct cvmx_pcsx_rxx_states_reg_s cn68xx;
	struct cvmx_pcsx_rxx_states_reg_s cn68xxp1;
	struct cvmx_pcsx_rxx_states_reg_s cn70xx;
	struct cvmx_pcsx_rxx_states_reg_s cn70xxp1;
	struct cvmx_pcsx_rxx_states_reg_s cnf71xx;
};

typedef union cvmx_pcsx_rxx_states_reg cvmx_pcsx_rxx_states_reg_t;

/**
 * cvmx_pcs#_rx#_sync_reg
 *
 * Note:
 * r_tx_rx_polarity_reg bit [2] will show correct polarity needed on the link receive path after code grp synchronization is achieved.
 *
 *
 *  PCS_RX_SYNC_REG = Code Group synchronization reg
 */
union cvmx_pcsx_rxx_sync_reg {
	u64 u64;
	struct cvmx_pcsx_rxx_sync_reg_s {
		u64 reserved_2_63 : 62;
		u64 sync : 1;
		u64 bit_lock : 1;
	} s;
	struct cvmx_pcsx_rxx_sync_reg_s cn52xx;
	struct cvmx_pcsx_rxx_sync_reg_s cn52xxp1;
	struct cvmx_pcsx_rxx_sync_reg_s cn56xx;
	struct cvmx_pcsx_rxx_sync_reg_s cn56xxp1;
	struct cvmx_pcsx_rxx_sync_reg_s cn61xx;
	struct cvmx_pcsx_rxx_sync_reg_s cn63xx;
	struct cvmx_pcsx_rxx_sync_reg_s cn63xxp1;
	struct cvmx_pcsx_rxx_sync_reg_s cn66xx;
	struct cvmx_pcsx_rxx_sync_reg_s cn68xx;
	struct cvmx_pcsx_rxx_sync_reg_s cn68xxp1;
	struct cvmx_pcsx_rxx_sync_reg_s cn70xx;
	struct cvmx_pcsx_rxx_sync_reg_s cn70xxp1;
	struct cvmx_pcsx_rxx_sync_reg_s cnf71xx;
};

typedef union cvmx_pcsx_rxx_sync_reg cvmx_pcsx_rxx_sync_reg_t;

/**
 * cvmx_pcs#_serdes_crdt_cnt#_reg
 *
 * PCS SERDES Credit Count
 *
 */
union cvmx_pcsx_serdes_crdt_cntx_reg {
	u64 u64;
	struct cvmx_pcsx_serdes_crdt_cntx_reg_s {
		u64 reserved_5_63 : 59;
		u64 cnt : 5;
	} s;
	struct cvmx_pcsx_serdes_crdt_cntx_reg_s cn70xx;
	struct cvmx_pcsx_serdes_crdt_cntx_reg_s cn70xxp1;
};

typedef union cvmx_pcsx_serdes_crdt_cntx_reg cvmx_pcsx_serdes_crdt_cntx_reg_t;

/**
 * cvmx_pcs#_sgm#_an_adv_reg
 *
 * SGMII AN Advertisement Register (sent out as tx_config_reg)
 * NOTE: The SGMII AN Advertisement Register above will be sent during Auto Negotiation if the
 * MAC_PHY mode bit in misc_ctl_reg
 * is set (1=PHY mode). If the bit is not set (0=MAC mode), the tx_config_reg[14] becomes ACK bit
 * and [0] is always 1.
 * All other bits in tx_config_reg sent will be 0. The PHY dictates the Auto Negotiation results.
 */
union cvmx_pcsx_sgmx_an_adv_reg {
	u64 u64;
	struct cvmx_pcsx_sgmx_an_adv_reg_s {
		u64 reserved_16_63 : 48;
		u64 link : 1;
		u64 ack : 1;
		u64 reserved_13_13 : 1;
		u64 dup : 1;
		u64 speed : 2;
		u64 reserved_1_9 : 9;
		u64 one : 1;
	} s;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn52xx;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn52xxp1;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn56xx;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn56xxp1;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn61xx;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn63xx;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn63xxp1;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn66xx;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn68xx;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn68xxp1;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn70xx;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cn70xxp1;
	struct cvmx_pcsx_sgmx_an_adv_reg_s cnf71xx;
};

typedef union cvmx_pcsx_sgmx_an_adv_reg cvmx_pcsx_sgmx_an_adv_reg_t;

/**
 * cvmx_pcs#_sgm#_lp_adv_reg
 *
 * SGMII LP Advertisement Register (received as rx_config_reg)
 *
 */
union cvmx_pcsx_sgmx_lp_adv_reg {
	u64 u64;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s {
		u64 reserved_16_63 : 48;
		u64 link : 1;
		u64 reserved_13_14 : 2;
		u64 dup : 1;
		u64 speed : 2;
		u64 reserved_1_9 : 9;
		u64 one : 1;
	} s;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn52xx;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn52xxp1;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn56xx;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn56xxp1;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn61xx;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn63xx;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn63xxp1;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn66xx;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn68xx;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn68xxp1;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn70xx;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cn70xxp1;
	struct cvmx_pcsx_sgmx_lp_adv_reg_s cnf71xx;
};

typedef union cvmx_pcsx_sgmx_lp_adv_reg cvmx_pcsx_sgmx_lp_adv_reg_t;

/**
 * cvmx_pcs#_tx#_states_reg
 *
 * PCS_TX_STATES_REG = TX State Machines states register
 *
 */
union cvmx_pcsx_txx_states_reg {
	u64 u64;
	struct cvmx_pcsx_txx_states_reg_s {
		u64 reserved_7_63 : 57;
		u64 xmit : 2;
		u64 tx_bad : 1;
		u64 ord_st : 4;
	} s;
	struct cvmx_pcsx_txx_states_reg_s cn52xx;
	struct cvmx_pcsx_txx_states_reg_s cn52xxp1;
	struct cvmx_pcsx_txx_states_reg_s cn56xx;
	struct cvmx_pcsx_txx_states_reg_s cn56xxp1;
	struct cvmx_pcsx_txx_states_reg_s cn61xx;
	struct cvmx_pcsx_txx_states_reg_s cn63xx;
	struct cvmx_pcsx_txx_states_reg_s cn63xxp1;
	struct cvmx_pcsx_txx_states_reg_s cn66xx;
	struct cvmx_pcsx_txx_states_reg_s cn68xx;
	struct cvmx_pcsx_txx_states_reg_s cn68xxp1;
	struct cvmx_pcsx_txx_states_reg_s cn70xx;
	struct cvmx_pcsx_txx_states_reg_s cn70xxp1;
	struct cvmx_pcsx_txx_states_reg_s cnf71xx;
};

typedef union cvmx_pcsx_txx_states_reg cvmx_pcsx_txx_states_reg_t;

/**
 * cvmx_pcs#_tx_rx#_polarity_reg
 *
 * Note:
 * r_tx_rx_polarity_reg bit [2] will show correct polarity needed on the link receive path after
 * code grp synchronization is achieved.
 */
union cvmx_pcsx_tx_rxx_polarity_reg {
	u64 u64;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s {
		u64 reserved_4_63 : 60;
		u64 rxovrd : 1;
		u64 autorxpl : 1;
		u64 rxplrt : 1;
		u64 txplrt : 1;
	} s;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn52xx;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn52xxp1;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn56xx;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn56xxp1;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn61xx;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn63xx;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn63xxp1;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn66xx;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn68xx;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn68xxp1;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn70xx;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cn70xxp1;
	struct cvmx_pcsx_tx_rxx_polarity_reg_s cnf71xx;
};

typedef union cvmx_pcsx_tx_rxx_polarity_reg cvmx_pcsx_tx_rxx_polarity_reg_t;

#endif
