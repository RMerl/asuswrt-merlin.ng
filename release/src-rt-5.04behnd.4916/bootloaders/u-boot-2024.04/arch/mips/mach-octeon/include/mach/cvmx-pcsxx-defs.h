/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2022 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pcsxx.
 */

#ifndef __CVMX_PCSXX_DEFS_H__
#define __CVMX_PCSXX_DEFS_H__

static inline u64 CVMX_PCSXX_10GBX_STATUS_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000828ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000828ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000828ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000828ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_BIST_STATUS_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000870ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000870ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000870ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000870ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_BIT_LOCK_STATUS_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000850ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000850ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000850ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000850ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_CONTROL1_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000800ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000800ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000800ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000800ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_CONTROL2_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000818ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000818ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000818ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000818ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_INT_EN_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000860ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000860ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000860ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000860ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_INT_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000858ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000858ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000858ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000858ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_LOG_ANL_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000868ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000868ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000868ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000868ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_MISC_CTL_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000848ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000848ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000848ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000848ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_RX_SYNC_STATES_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000838ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000838ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000838ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000838ull + (offset) * 0x8000000ull;
}

#define CVMX_PCSXX_SERDES_CRDT_CNT_REG(offset) (0x00011800B0000880ull)
static inline u64 CVMX_PCSXX_SPD_ABIL_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000810ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000810ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000810ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000810ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_STATUS1_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000808ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000808ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000808ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000808ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_STATUS2_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000820ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000820ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000820ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000820ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_TX_RX_POLARITY_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000840ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000840ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000840ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000840ull + (offset) * 0x8000000ull;
}

static inline u64 CVMX_PCSXX_TX_RX_STATES_REG(unsigned long offset)
{
	switch (cvmx_get_octeon_family()) {
	case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000830ull + (offset) * 0x8000000ull;
	case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
	case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000830ull + (offset) * 0x8000000ull;
	case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
		return 0x00011800B0000830ull + (offset) * 0x1000000ull;
	}
	return 0x00011800B0000830ull + (offset) * 0x8000000ull;
}

/**
 * cvmx_pcsx#_10gbx_status_reg
 *
 * PCSX_10GBX_STATUS_REG = 10gbx_status_reg
 *
 */
union cvmx_pcsxx_10gbx_status_reg {
	u64 u64;
	struct cvmx_pcsxx_10gbx_status_reg_s {
		u64 reserved_13_63 : 51;
		u64 alignd : 1;
		u64 pattst : 1;
		u64 reserved_4_10 : 7;
		u64 l3sync : 1;
		u64 l2sync : 1;
		u64 l1sync : 1;
		u64 l0sync : 1;
	} s;
	struct cvmx_pcsxx_10gbx_status_reg_s cn52xx;
	struct cvmx_pcsxx_10gbx_status_reg_s cn52xxp1;
	struct cvmx_pcsxx_10gbx_status_reg_s cn56xx;
	struct cvmx_pcsxx_10gbx_status_reg_s cn56xxp1;
	struct cvmx_pcsxx_10gbx_status_reg_s cn61xx;
	struct cvmx_pcsxx_10gbx_status_reg_s cn63xx;
	struct cvmx_pcsxx_10gbx_status_reg_s cn63xxp1;
	struct cvmx_pcsxx_10gbx_status_reg_s cn66xx;
	struct cvmx_pcsxx_10gbx_status_reg_s cn68xx;
	struct cvmx_pcsxx_10gbx_status_reg_s cn68xxp1;
	struct cvmx_pcsxx_10gbx_status_reg_s cn70xx;
	struct cvmx_pcsxx_10gbx_status_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_10gbx_status_reg cvmx_pcsxx_10gbx_status_reg_t;

/**
 * cvmx_pcsx#_bist_status_reg
 *
 * PCSX Bist Status Register
 *
 */
union cvmx_pcsxx_bist_status_reg {
	u64 u64;
	struct cvmx_pcsxx_bist_status_reg_s {
		u64 reserved_1_63 : 63;
		u64 bist_status : 1;
	} s;
	struct cvmx_pcsxx_bist_status_reg_s cn52xx;
	struct cvmx_pcsxx_bist_status_reg_s cn52xxp1;
	struct cvmx_pcsxx_bist_status_reg_s cn56xx;
	struct cvmx_pcsxx_bist_status_reg_s cn56xxp1;
	struct cvmx_pcsxx_bist_status_reg_s cn61xx;
	struct cvmx_pcsxx_bist_status_reg_s cn63xx;
	struct cvmx_pcsxx_bist_status_reg_s cn63xxp1;
	struct cvmx_pcsxx_bist_status_reg_s cn66xx;
	struct cvmx_pcsxx_bist_status_reg_s cn68xx;
	struct cvmx_pcsxx_bist_status_reg_s cn68xxp1;
	struct cvmx_pcsxx_bist_status_reg_s cn70xx;
	struct cvmx_pcsxx_bist_status_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_bist_status_reg cvmx_pcsxx_bist_status_reg_t;

/**
 * cvmx_pcsx#_bit_lock_status_reg
 *
 * PCSX Bit Lock Status Register
 *
 */
union cvmx_pcsxx_bit_lock_status_reg {
	u64 u64;
	struct cvmx_pcsxx_bit_lock_status_reg_s {
		u64 reserved_4_63 : 60;
		u64 bitlck3 : 1;
		u64 bitlck2 : 1;
		u64 bitlck1 : 1;
		u64 bitlck0 : 1;
	} s;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn52xx;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn52xxp1;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn56xx;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn56xxp1;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn61xx;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn63xx;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn63xxp1;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn66xx;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn68xx;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn68xxp1;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn70xx;
	struct cvmx_pcsxx_bit_lock_status_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_bit_lock_status_reg cvmx_pcsxx_bit_lock_status_reg_t;

/**
 * cvmx_pcsx#_control1_reg
 *
 * NOTE: Logic Analyzer is enabled with LA_EN for the specified PCS lane only.
 * PKT_SZ is effective only when LA_EN=1
 * For normal operation(sgmii or 1000Base-X), this bit must be 0.
 * See pcsx.csr for xaui logic analyzer mode.
 * For full description see document at .../rtl/pcs/readme_logic_analyzer.txt
 *
 *
 *  PCSX regs follow IEEE Std 802.3-2005, Section: 45.2.3
 *
 *
 *  PCSX_CONTROL1_REG = Control Register1
 */
union cvmx_pcsxx_control1_reg {
	u64 u64;
	struct cvmx_pcsxx_control1_reg_s {
		u64 reserved_16_63 : 48;
		u64 reset : 1;
		u64 loopbck1 : 1;
		u64 spdsel1 : 1;
		u64 reserved_12_12 : 1;
		u64 lo_pwr : 1;
		u64 reserved_7_10 : 4;
		u64 spdsel0 : 1;
		u64 spd : 4;
		u64 reserved_0_1 : 2;
	} s;
	struct cvmx_pcsxx_control1_reg_s cn52xx;
	struct cvmx_pcsxx_control1_reg_s cn52xxp1;
	struct cvmx_pcsxx_control1_reg_s cn56xx;
	struct cvmx_pcsxx_control1_reg_s cn56xxp1;
	struct cvmx_pcsxx_control1_reg_s cn61xx;
	struct cvmx_pcsxx_control1_reg_s cn63xx;
	struct cvmx_pcsxx_control1_reg_s cn63xxp1;
	struct cvmx_pcsxx_control1_reg_s cn66xx;
	struct cvmx_pcsxx_control1_reg_s cn68xx;
	struct cvmx_pcsxx_control1_reg_s cn68xxp1;
	struct cvmx_pcsxx_control1_reg_s cn70xx;
	struct cvmx_pcsxx_control1_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_control1_reg cvmx_pcsxx_control1_reg_t;

/**
 * cvmx_pcsx#_control2_reg
 *
 * PCSX_CONTROL2_REG = Control Register2
 *
 */
union cvmx_pcsxx_control2_reg {
	u64 u64;
	struct cvmx_pcsxx_control2_reg_s {
		u64 reserved_2_63 : 62;
		u64 type : 2;
	} s;
	struct cvmx_pcsxx_control2_reg_s cn52xx;
	struct cvmx_pcsxx_control2_reg_s cn52xxp1;
	struct cvmx_pcsxx_control2_reg_s cn56xx;
	struct cvmx_pcsxx_control2_reg_s cn56xxp1;
	struct cvmx_pcsxx_control2_reg_s cn61xx;
	struct cvmx_pcsxx_control2_reg_s cn63xx;
	struct cvmx_pcsxx_control2_reg_s cn63xxp1;
	struct cvmx_pcsxx_control2_reg_s cn66xx;
	struct cvmx_pcsxx_control2_reg_s cn68xx;
	struct cvmx_pcsxx_control2_reg_s cn68xxp1;
	struct cvmx_pcsxx_control2_reg_s cn70xx;
	struct cvmx_pcsxx_control2_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_control2_reg cvmx_pcsxx_control2_reg_t;

/**
 * cvmx_pcsx#_int_en_reg
 *
 * PCSX Interrupt Enable Register
 *
 */
union cvmx_pcsxx_int_en_reg {
	u64 u64;
	struct cvmx_pcsxx_int_en_reg_s {
		u64 reserved_7_63 : 57;
		u64 dbg_sync_en : 1;
		u64 algnlos_en : 1;
		u64 synlos_en : 1;
		u64 bitlckls_en : 1;
		u64 rxsynbad_en : 1;
		u64 rxbad_en : 1;
		u64 txflt_en : 1;
	} s;
	struct cvmx_pcsxx_int_en_reg_cn52xx {
		u64 reserved_6_63 : 58;
		u64 algnlos_en : 1;
		u64 synlos_en : 1;
		u64 bitlckls_en : 1;
		u64 rxsynbad_en : 1;
		u64 rxbad_en : 1;
		u64 txflt_en : 1;
	} cn52xx;
	struct cvmx_pcsxx_int_en_reg_cn52xx cn52xxp1;
	struct cvmx_pcsxx_int_en_reg_cn52xx cn56xx;
	struct cvmx_pcsxx_int_en_reg_cn52xx cn56xxp1;
	struct cvmx_pcsxx_int_en_reg_s cn61xx;
	struct cvmx_pcsxx_int_en_reg_s cn63xx;
	struct cvmx_pcsxx_int_en_reg_s cn63xxp1;
	struct cvmx_pcsxx_int_en_reg_s cn66xx;
	struct cvmx_pcsxx_int_en_reg_s cn68xx;
	struct cvmx_pcsxx_int_en_reg_s cn68xxp1;
	struct cvmx_pcsxx_int_en_reg_s cn70xx;
	struct cvmx_pcsxx_int_en_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_int_en_reg cvmx_pcsxx_int_en_reg_t;

/**
 * cvmx_pcsx#_int_reg
 *
 * PCSX Interrupt Register
 * Note: DBG_SYNC is a edge triggered interrupt. When set it indicates PCS Synchronization state
 * machine in
 * Figure 48-7 state diagram in IEEE Std 802.3-2005 changes state SYNC_ACQUIRED_1 to
 * SYNC_ACQUIRED_2
 * indicating an invalid code group was received on one of the 4 receive lanes.
 * This interrupt should be always disabled and used only for link problem debugging help.
 */
union cvmx_pcsxx_int_reg {
	u64 u64;
	struct cvmx_pcsxx_int_reg_s {
		u64 reserved_7_63 : 57;
		u64 dbg_sync : 1;
		u64 algnlos : 1;
		u64 synlos : 1;
		u64 bitlckls : 1;
		u64 rxsynbad : 1;
		u64 rxbad : 1;
		u64 txflt : 1;
	} s;
	struct cvmx_pcsxx_int_reg_cn52xx {
		u64 reserved_6_63 : 58;
		u64 algnlos : 1;
		u64 synlos : 1;
		u64 bitlckls : 1;
		u64 rxsynbad : 1;
		u64 rxbad : 1;
		u64 txflt : 1;
	} cn52xx;
	struct cvmx_pcsxx_int_reg_cn52xx cn52xxp1;
	struct cvmx_pcsxx_int_reg_cn52xx cn56xx;
	struct cvmx_pcsxx_int_reg_cn52xx cn56xxp1;
	struct cvmx_pcsxx_int_reg_s cn61xx;
	struct cvmx_pcsxx_int_reg_s cn63xx;
	struct cvmx_pcsxx_int_reg_s cn63xxp1;
	struct cvmx_pcsxx_int_reg_s cn66xx;
	struct cvmx_pcsxx_int_reg_s cn68xx;
	struct cvmx_pcsxx_int_reg_s cn68xxp1;
	struct cvmx_pcsxx_int_reg_s cn70xx;
	struct cvmx_pcsxx_int_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_int_reg cvmx_pcsxx_int_reg_t;

/**
 * cvmx_pcsx#_log_anl_reg
 *
 * PCSX Logic Analyzer Register
 * NOTE: Logic Analyzer is enabled with LA_EN for xaui only. PKT_SZ is effective only when
 * LA_EN=1
 * For normal operation(xaui), this bit must be 0. The dropped lane is used to send rxc[3:0].
 * See pcs.csr  for sgmii/1000Base-X logic analyzer mode.
 * For full description see document at .../rtl/pcs/readme_logic_analyzer.txt
 */
union cvmx_pcsxx_log_anl_reg {
	u64 u64;
	struct cvmx_pcsxx_log_anl_reg_s {
		u64 reserved_7_63 : 57;
		u64 enc_mode : 1;
		u64 drop_ln : 2;
		u64 lafifovfl : 1;
		u64 la_en : 1;
		u64 pkt_sz : 2;
	} s;
	struct cvmx_pcsxx_log_anl_reg_s cn52xx;
	struct cvmx_pcsxx_log_anl_reg_s cn52xxp1;
	struct cvmx_pcsxx_log_anl_reg_s cn56xx;
	struct cvmx_pcsxx_log_anl_reg_s cn56xxp1;
	struct cvmx_pcsxx_log_anl_reg_s cn61xx;
	struct cvmx_pcsxx_log_anl_reg_s cn63xx;
	struct cvmx_pcsxx_log_anl_reg_s cn63xxp1;
	struct cvmx_pcsxx_log_anl_reg_s cn66xx;
	struct cvmx_pcsxx_log_anl_reg_s cn68xx;
	struct cvmx_pcsxx_log_anl_reg_s cn68xxp1;
	struct cvmx_pcsxx_log_anl_reg_s cn70xx;
	struct cvmx_pcsxx_log_anl_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_log_anl_reg cvmx_pcsxx_log_anl_reg_t;

/**
 * cvmx_pcsx#_misc_ctl_reg
 *
 * PCSX Misc Control Register
 * LN_SWAP for XAUI is to simplify interconnection layout between devices
 */
union cvmx_pcsxx_misc_ctl_reg {
	u64 u64;
	struct cvmx_pcsxx_misc_ctl_reg_s {
		u64 reserved_4_63 : 60;
		u64 tx_swap : 1;
		u64 rx_swap : 1;
		u64 xaui : 1;
		u64 gmxeno : 1;
	} s;
	struct cvmx_pcsxx_misc_ctl_reg_s cn52xx;
	struct cvmx_pcsxx_misc_ctl_reg_s cn52xxp1;
	struct cvmx_pcsxx_misc_ctl_reg_s cn56xx;
	struct cvmx_pcsxx_misc_ctl_reg_s cn56xxp1;
	struct cvmx_pcsxx_misc_ctl_reg_s cn61xx;
	struct cvmx_pcsxx_misc_ctl_reg_s cn63xx;
	struct cvmx_pcsxx_misc_ctl_reg_s cn63xxp1;
	struct cvmx_pcsxx_misc_ctl_reg_s cn66xx;
	struct cvmx_pcsxx_misc_ctl_reg_s cn68xx;
	struct cvmx_pcsxx_misc_ctl_reg_s cn68xxp1;
	struct cvmx_pcsxx_misc_ctl_reg_s cn70xx;
	struct cvmx_pcsxx_misc_ctl_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_misc_ctl_reg cvmx_pcsxx_misc_ctl_reg_t;

/**
 * cvmx_pcsx#_rx_sync_states_reg
 *
 * PCSX_RX_SYNC_STATES_REG = Receive Sync States Register
 *
 */
union cvmx_pcsxx_rx_sync_states_reg {
	u64 u64;
	struct cvmx_pcsxx_rx_sync_states_reg_s {
		u64 reserved_16_63 : 48;
		u64 sync3st : 4;
		u64 sync2st : 4;
		u64 sync1st : 4;
		u64 sync0st : 4;
	} s;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn52xx;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn52xxp1;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn56xx;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn56xxp1;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn61xx;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn63xx;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn63xxp1;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn66xx;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn68xx;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn68xxp1;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn70xx;
	struct cvmx_pcsxx_rx_sync_states_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_rx_sync_states_reg cvmx_pcsxx_rx_sync_states_reg_t;

/**
 * cvmx_pcsx#_serdes_crdt_cnt_reg
 *
 * PCSX SERDES Credit Count
 *
 */
union cvmx_pcsxx_serdes_crdt_cnt_reg {
	u64 u64;
	struct cvmx_pcsxx_serdes_crdt_cnt_reg_s {
		u64 reserved_5_63 : 59;
		u64 cnt : 5;
	} s;
	struct cvmx_pcsxx_serdes_crdt_cnt_reg_s cn70xx;
	struct cvmx_pcsxx_serdes_crdt_cnt_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_serdes_crdt_cnt_reg cvmx_pcsxx_serdes_crdt_cnt_reg_t;

/**
 * cvmx_pcsx#_spd_abil_reg
 *
 * PCSX_SPD_ABIL_REG = Speed ability register
 *
 */
union cvmx_pcsxx_spd_abil_reg {
	u64 u64;
	struct cvmx_pcsxx_spd_abil_reg_s {
		u64 reserved_2_63 : 62;
		u64 tenpasst : 1;
		u64 tengb : 1;
	} s;
	struct cvmx_pcsxx_spd_abil_reg_s cn52xx;
	struct cvmx_pcsxx_spd_abil_reg_s cn52xxp1;
	struct cvmx_pcsxx_spd_abil_reg_s cn56xx;
	struct cvmx_pcsxx_spd_abil_reg_s cn56xxp1;
	struct cvmx_pcsxx_spd_abil_reg_s cn61xx;
	struct cvmx_pcsxx_spd_abil_reg_s cn63xx;
	struct cvmx_pcsxx_spd_abil_reg_s cn63xxp1;
	struct cvmx_pcsxx_spd_abil_reg_s cn66xx;
	struct cvmx_pcsxx_spd_abil_reg_s cn68xx;
	struct cvmx_pcsxx_spd_abil_reg_s cn68xxp1;
	struct cvmx_pcsxx_spd_abil_reg_s cn70xx;
	struct cvmx_pcsxx_spd_abil_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_spd_abil_reg cvmx_pcsxx_spd_abil_reg_t;

/**
 * cvmx_pcsx#_status1_reg
 *
 * PCSX_STATUS1_REG = Status Register1
 *
 */
union cvmx_pcsxx_status1_reg {
	u64 u64;
	struct cvmx_pcsxx_status1_reg_s {
		u64 reserved_8_63 : 56;
		u64 flt : 1;
		u64 reserved_3_6 : 4;
		u64 rcv_lnk : 1;
		u64 lpable : 1;
		u64 reserved_0_0 : 1;
	} s;
	struct cvmx_pcsxx_status1_reg_s cn52xx;
	struct cvmx_pcsxx_status1_reg_s cn52xxp1;
	struct cvmx_pcsxx_status1_reg_s cn56xx;
	struct cvmx_pcsxx_status1_reg_s cn56xxp1;
	struct cvmx_pcsxx_status1_reg_s cn61xx;
	struct cvmx_pcsxx_status1_reg_s cn63xx;
	struct cvmx_pcsxx_status1_reg_s cn63xxp1;
	struct cvmx_pcsxx_status1_reg_s cn66xx;
	struct cvmx_pcsxx_status1_reg_s cn68xx;
	struct cvmx_pcsxx_status1_reg_s cn68xxp1;
	struct cvmx_pcsxx_status1_reg_s cn70xx;
	struct cvmx_pcsxx_status1_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_status1_reg cvmx_pcsxx_status1_reg_t;

/**
 * cvmx_pcsx#_status2_reg
 *
 * PCSX_STATUS2_REG = Status Register2
 *
 */
union cvmx_pcsxx_status2_reg {
	u64 u64;
	struct cvmx_pcsxx_status2_reg_s {
		u64 reserved_16_63 : 48;
		u64 dev : 2;
		u64 reserved_12_13 : 2;
		u64 xmtflt : 1;
		u64 rcvflt : 1;
		u64 reserved_3_9 : 7;
		u64 tengb_w : 1;
		u64 tengb_x : 1;
		u64 tengb_r : 1;
	} s;
	struct cvmx_pcsxx_status2_reg_s cn52xx;
	struct cvmx_pcsxx_status2_reg_s cn52xxp1;
	struct cvmx_pcsxx_status2_reg_s cn56xx;
	struct cvmx_pcsxx_status2_reg_s cn56xxp1;
	struct cvmx_pcsxx_status2_reg_s cn61xx;
	struct cvmx_pcsxx_status2_reg_s cn63xx;
	struct cvmx_pcsxx_status2_reg_s cn63xxp1;
	struct cvmx_pcsxx_status2_reg_s cn66xx;
	struct cvmx_pcsxx_status2_reg_s cn68xx;
	struct cvmx_pcsxx_status2_reg_s cn68xxp1;
	struct cvmx_pcsxx_status2_reg_s cn70xx;
	struct cvmx_pcsxx_status2_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_status2_reg cvmx_pcsxx_status2_reg_t;

/**
 * cvmx_pcsx#_tx_rx_polarity_reg
 *
 * RX lane polarity vector [3:0] = XOR_RXPLRT<9:6>  ^  [4[RXPLRT<1>]];
 * TX lane polarity vector [3:0] = XOR_TXPLRT<5:2>  ^  [4[TXPLRT<0>]];
 * In short keep <1:0> to 2'b00, and use <5:2> and <9:6> fields to define per lane polarities
 */
union cvmx_pcsxx_tx_rx_polarity_reg {
	u64 u64;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s {
		u64 reserved_10_63 : 54;
		u64 xor_rxplrt : 4;
		u64 xor_txplrt : 4;
		u64 rxplrt : 1;
		u64 txplrt : 1;
	} s;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn52xx;
	struct cvmx_pcsxx_tx_rx_polarity_reg_cn52xxp1 {
		u64 reserved_2_63 : 62;
		u64 rxplrt : 1;
		u64 txplrt : 1;
	} cn52xxp1;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn56xx;
	struct cvmx_pcsxx_tx_rx_polarity_reg_cn52xxp1 cn56xxp1;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn61xx;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn63xx;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn63xxp1;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn66xx;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn68xx;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn68xxp1;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn70xx;
	struct cvmx_pcsxx_tx_rx_polarity_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_tx_rx_polarity_reg cvmx_pcsxx_tx_rx_polarity_reg_t;

/**
 * cvmx_pcsx#_tx_rx_states_reg
 *
 * PCSX_TX_RX_STATES_REG = Transmit Receive States Register
 *
 */
union cvmx_pcsxx_tx_rx_states_reg {
	u64 u64;
	struct cvmx_pcsxx_tx_rx_states_reg_s {
		u64 reserved_14_63 : 50;
		u64 term_err : 1;
		u64 syn3bad : 1;
		u64 syn2bad : 1;
		u64 syn1bad : 1;
		u64 syn0bad : 1;
		u64 rxbad : 1;
		u64 algn_st : 3;
		u64 rx_st : 2;
		u64 tx_st : 3;
	} s;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn52xx;
	struct cvmx_pcsxx_tx_rx_states_reg_cn52xxp1 {
		u64 reserved_13_63 : 51;
		u64 syn3bad : 1;
		u64 syn2bad : 1;
		u64 syn1bad : 1;
		u64 syn0bad : 1;
		u64 rxbad : 1;
		u64 algn_st : 3;
		u64 rx_st : 2;
		u64 tx_st : 3;
	} cn52xxp1;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn56xx;
	struct cvmx_pcsxx_tx_rx_states_reg_cn52xxp1 cn56xxp1;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn61xx;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn63xx;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn63xxp1;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn66xx;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn68xx;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn68xxp1;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn70xx;
	struct cvmx_pcsxx_tx_rx_states_reg_s cn70xxp1;
};

typedef union cvmx_pcsxx_tx_rx_states_reg cvmx_pcsxx_tx_rx_states_reg_t;

#endif
