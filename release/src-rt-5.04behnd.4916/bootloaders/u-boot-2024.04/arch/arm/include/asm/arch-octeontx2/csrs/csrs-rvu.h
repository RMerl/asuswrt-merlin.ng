/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_RVU_H__
#define __CSRS_RVU_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * RVU.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Enumeration rvu_af_int_vec_e
 *
 * RVU Admin Function Interrupt Vector Enumeration Enumerates the MSI-X
 * interrupt vectors. Internal: RVU maintains the state of these vectors
 * internally, and generates GIB messages for it without accessing the
 * MSI-X table region in LLC/DRAM.
 */
#define RVU_AF_INT_VEC_E_GEN (3)
#define RVU_AF_INT_VEC_E_MBOX (4)
#define RVU_AF_INT_VEC_E_PFFLR (1)
#define RVU_AF_INT_VEC_E_PFME (2)
#define RVU_AF_INT_VEC_E_POISON (0)

/**
 * Enumeration rvu_bar_e
 *
 * RVU Base Address Register Enumeration Enumerates the base address
 * registers. Internal: For documentation only.
 */
#define RVU_BAR_E_RVU_PFX_BAR0(a) (0x840000000000ll + 0x1000000000ll * (a))
#define RVU_BAR_E_RVU_PFX_BAR0_SIZE 0x10000000ull
#define RVU_BAR_E_RVU_PFX_FUNCX_BAR2(a, b)	\
	(0x840200000000ll + 0x1000000000ll * (a) + 0x2000000ll * (b))
#define RVU_BAR_E_RVU_PFX_FUNCX_BAR2_SIZE 0x100000ull
#define RVU_BAR_E_RVU_PFX_FUNCX_BAR4(a, b)	\
	(0x840400000000ll + 0x1000000000ll * (a) + 0x2000000ll * (b))
#define RVU_BAR_E_RVU_PFX_FUNCX_BAR4_SIZE 0x10000ull

/**
 * Enumeration rvu_block_addr_e
 *
 * RVU Block Address Enumeration Enumerates addressing of RVU resource
 * blocks within each RVU BAR, i.e. values of RVU_FUNC_ADDR_S[BLOCK] and
 * RVU_AF_ADDR_S[BLOCK].  CNXXXX may not implement all enumerated blocks.
 * Software can read RVU_PF/RVU_VF_BLOCK_ADDR()_DISC[IMP] to discover
 * which blocks are implemented and enabled.
 */
#define RVU_BLOCK_ADDR_E_CPTX(a) (0xa + (a))
#define RVU_BLOCK_ADDR_E_LMT (1)
#define RVU_BLOCK_ADDR_E_NDCX(a) (0xc + (a))
#define RVU_BLOCK_ADDR_E_NIXX(a) (4 + (a))
#define RVU_BLOCK_ADDR_E_NPA (3)
#define RVU_BLOCK_ADDR_E_NPC (6)
#define RVU_BLOCK_ADDR_E_RX(a) (0 + (a))
#define RVU_BLOCK_ADDR_E_REEX(a) (0x14 + (a))
#define RVU_BLOCK_ADDR_E_RVUM (0)
#define RVU_BLOCK_ADDR_E_SSO (7)
#define RVU_BLOCK_ADDR_E_SSOW (8)
#define RVU_BLOCK_ADDR_E_TIM (9)

/**
 * Enumeration rvu_block_type_e
 *
 * RVU Block Type Enumeration Enumerates values of
 * RVU_PF/RVU_VF_BLOCK_ADDR()_DISC[BTYPE].
 */
#define RVU_BLOCK_TYPE_E_CPT (9)
#define RVU_BLOCK_TYPE_E_DDF (0xb)
#define RVU_BLOCK_TYPE_E_LMT (2)
#define RVU_BLOCK_TYPE_E_NDC (0xa)
#define RVU_BLOCK_TYPE_E_NIX (3)
#define RVU_BLOCK_TYPE_E_NPA (4)
#define RVU_BLOCK_TYPE_E_NPC (5)
#define RVU_BLOCK_TYPE_E_RAD (0xd)
#define RVU_BLOCK_TYPE_E_REE (0xe)
#define RVU_BLOCK_TYPE_E_RVUM (0)
#define RVU_BLOCK_TYPE_E_SSO (6)
#define RVU_BLOCK_TYPE_E_SSOW (7)
#define RVU_BLOCK_TYPE_E_TIM (8)
#define RVU_BLOCK_TYPE_E_ZIP (0xc)

/**
 * Enumeration rvu_bus_lf_e
 *
 * INTERNAL: RVU Bus LF Range Enumeration  Enumerates the LF range for
 * the RVU bus. Internal: This is an enum used in csr3 virtual equations.
 */
#define RVU_BUS_LF_E_RVU_BUS_LFX(a) (0 + 0x2000000 * (a))

/**
 * Enumeration rvu_bus_lf_slot_e
 *
 * INTERNAL: RVU Bus LF Slot Range Enumeration  Enumerates the LF and
 * Slot range for the RVU bus. Internal: This is an enum used in csr3
 * virtual equations.
 */
#define RVU_BUS_LF_SLOT_E_RVU_BUS_LFX_SLOTX(a, b)	\
	(0 + 0x2000000 * (a) + 0x1000 * (b))

/**
 * Enumeration rvu_bus_pf_e
 *
 * INTERNAL: RVU Bus PF Range Enumeration  Enumerates the PF range for
 * the RVU bus. Internal: This is an enum used in csr3 virtual equations.
 */
#define RVU_BUS_PF_E_RVU_BUS_PFX(a) (0ll + 0x1000000000ll * (a))

/**
 * Enumeration rvu_bus_pfvf_e
 *
 * INTERNAL: RVU Bus PFVF Range Enumeration  Enumerates the PF and VF
 * ranges for the RVU bus. Internal: This is an enum used in csr3 virtual
 * equations.
 */
#define RVU_BUS_PFVF_E_RVU_BUS_PFX(a) (0 + 0x2000000 * (a))
#define RVU_BUS_PFVF_E_RVU_BUS_VFX(a) (0 + 0x2000000 * (a))

/**
 * Enumeration rvu_busbar_e
 *
 * INTERNAL: RVU Bus Base Address Region Enumeration  Enumerates the base
 * address region for the RVU bus. Internal: This is an enum used in csr3
 * virtual equations.
 */
#define RVU_BUSBAR_E_RVU_BUSBAR0 (0)
#define RVU_BUSBAR_E_RVU_BUSBAR2 (0x200000000ll)

/**
 * Enumeration rvu_busdid_e
 *
 * INTERNAL: RVU Bus DID Enumeration  Enumerates the DID offset for the
 * RVU bus. Internal: This is an enum used in csr3 virtual equations.
 */
#define RVU_BUSDID_E_RVU_BUSDID (0x840000000000ll)

/**
 * Enumeration rvu_pf_int_vec_e
 *
 * RVU PF Interrupt Vector Enumeration Enumerates the MSI-X interrupt
 * vectors.
 */
#define RVU_PF_INT_VEC_E_AFPF_MBOX (6)
#define RVU_PF_INT_VEC_E_VFFLRX(a) (0 + (a))
#define RVU_PF_INT_VEC_E_VFMEX(a) (2 + (a))
#define RVU_PF_INT_VEC_E_VFPF_MBOXX(a) (4 + (a))

/**
 * Enumeration rvu_vf_int_vec_e
 *
 * RVU VF Interrupt Vector Enumeration Enumerates the MSI-X interrupt
 * vectors.
 */
#define RVU_VF_INT_VEC_E_MBOX (0)

/**
 * Structure rvu_af_addr_s
 *
 * RVU Admin Function Register Address Structure Address format for
 * accessing shared Admin Function (AF) registers in RVU PF BAR0. These
 * registers may be accessed by all RVU PFs whose
 * RVU_PRIV_PF()_CFG[AF_ENA] bit is set.
 */
union rvu_af_addr_s {
	u64 u;
	struct rvu_af_addr_s_s {
		u64 addr                             : 28;
		u64 block                            : 5;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct rvu_af_addr_s_s cn; */
};

/**
 * Structure rvu_func_addr_s
 *
 * RVU Function-unique Address Structure Address format for accessing
 * function-unique registers in RVU PF/FUNC BAR2.
 */
union rvu_func_addr_s {
	u32 u;
	struct rvu_func_addr_s_s {
		u32 addr                             : 12;
		u32 lf_slot                          : 8;
		u32 block                            : 5;
		u32 reserved_25_31                   : 7;
	} s;
	/* struct rvu_func_addr_s_s cn; */
};

/**
 * Structure rvu_msix_vec_s
 *
 * RVU MSI-X Vector Structure Format of entries in the RVU MSI-X table
 * region in LLC/DRAM. See RVU_PRIV_PF()_MSIX_CFG.
 */
union rvu_msix_vec_s {
	u64 u[2];
	struct rvu_msix_vec_s_s {
		u64 addr                             : 64;
		u64 data                             : 32;
		u64 mask                             : 1;
		u64 pend                             : 1;
		u64 reserved_98_127                  : 30;
	} s;
	/* struct rvu_msix_vec_s_s cn; */
};

/**
 * Structure rvu_pf_func_s
 *
 * RVU PF Function Identification Structure Identifies an RVU PF/VF, and
 * format of *_PRIV_LF()_CFG[PF_FUNC] in RVU resource blocks, e.g.
 * NPA_PRIV_LF()_CFG[PF_FUNC].  Internal: Also used for PF/VF
 * identification on inter-coprocessor hardware interfaces (NPA, SSO,
 * CPT, ...).
 */
union rvu_pf_func_s {
	u32 u;
	struct rvu_pf_func_s_s {
		u32 func                             : 10;
		u32 pf                               : 6;
		u32 reserved_16_31                   : 16;
	} s;
	/* struct rvu_pf_func_s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) rvu_af_afpf#_mbox#
 *
 * RVU Admin Function AF/PF Mailbox Registers
 */
union rvu_af_afpfx_mboxx {
	u64 u;
	struct rvu_af_afpfx_mboxx_s {
		u64 data                             : 64;
	} s;
	/* struct rvu_af_afpfx_mboxx_s cn; */
};

static inline u64 RVU_AF_AFPFX_MBOXX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_AFPFX_MBOXX(u64 a, u64 b)
{
	return 0x2000 + 0x10 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_bar2_alias#
 *
 * INTERNAL: RVU Admin Function  BAR2 Alias Registers  These registers
 * alias to the RVU BAR2 registers for the PF and function selected by
 * RVU_AF_BAR2_SEL[PF_FUNC].  Internal: Not implemented. Placeholder for
 * bug33464.
 */
union rvu_af_bar2_aliasx {
	u64 u;
	struct rvu_af_bar2_aliasx_s {
		u64 data                             : 64;
	} s;
	/* struct rvu_af_bar2_aliasx_s cn; */
};

static inline u64 RVU_AF_BAR2_ALIASX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_BAR2_ALIASX(u64 a)
{
	return 0x9100000 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_bar2_sel
 *
 * INTERNAL: RVU Admin Function BAR2 Select Register  This register
 * configures BAR2 accesses from the RVU_AF_BAR2_ALIAS() registers in
 * BAR0. Internal: Not implemented. Placeholder for bug33464.
 */
union rvu_af_bar2_sel {
	u64 u;
	struct rvu_af_bar2_sel_s {
		u64 alias_pf_func                    : 16;
		u64 alias_ena                        : 1;
		u64 reserved_17_63                   : 47;
	} s;
	/* struct rvu_af_bar2_sel_s cn; */
};

static inline u64 RVU_AF_BAR2_SEL(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_BAR2_SEL(void)
{
	return 0x9000000;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_blk_rst
 *
 * RVU Master Admin Function Block Reset Register
 */
union rvu_af_blk_rst {
	u64 u;
	struct rvu_af_blk_rst_s {
		u64 rst                              : 1;
		u64 reserved_1_62                    : 62;
		u64 busy                             : 1;
	} s;
	/* struct rvu_af_blk_rst_s cn; */
};

static inline u64 RVU_AF_BLK_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_BLK_RST(void)
{
	return 0x30;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_bp_test
 *
 * INTERNAL: RVUM Backpressure Test Registers
 */
union rvu_af_bp_test {
	u64 u;
	struct rvu_af_bp_test_s {
		u64 lfsr_freq                        : 12;
		u64 reserved_12_15                   : 4;
		u64 bp_cfg                           : 16;
		u64 enable                           : 8;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct rvu_af_bp_test_s cn; */
};

static inline u64 RVU_AF_BP_TEST(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_BP_TEST(void)
{
	return 0x4000;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_eco
 *
 * INTERNAL: RVU Admin Function ECO Register
 */
union rvu_af_eco {
	u64 u;
	struct rvu_af_eco_s {
		u64 eco_rw                           : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct rvu_af_eco_s cn; */
};

static inline u64 RVU_AF_ECO(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_ECO(void)
{
	return 0x20;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_gen_int
 *
 * RVU Admin Function General Interrupt Register This register contains
 * General interrupt summary bits.
 */
union rvu_af_gen_int {
	u64 u;
	struct rvu_af_gen_int_s {
		u64 unmapped                         : 1;
		u64 msix_fault                       : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct rvu_af_gen_int_s cn; */
};

static inline u64 RVU_AF_GEN_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_GEN_INT(void)
{
	return 0x120;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_gen_int_ena_w1c
 *
 * RVU Admin Function General Interrupt Enable Clear Register This
 * register clears interrupt enable bits.
 */
union rvu_af_gen_int_ena_w1c {
	u64 u;
	struct rvu_af_gen_int_ena_w1c_s {
		u64 unmapped                         : 1;
		u64 msix_fault                       : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct rvu_af_gen_int_ena_w1c_s cn; */
};

static inline u64 RVU_AF_GEN_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_GEN_INT_ENA_W1C(void)
{
	return 0x138;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_gen_int_ena_w1s
 *
 * RVU Admin Function General Interrupt Enable Set Register This register
 * sets interrupt enable bits.
 */
union rvu_af_gen_int_ena_w1s {
	u64 u;
	struct rvu_af_gen_int_ena_w1s_s {
		u64 unmapped                         : 1;
		u64 msix_fault                       : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct rvu_af_gen_int_ena_w1s_s cn; */
};

static inline u64 RVU_AF_GEN_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_GEN_INT_ENA_W1S(void)
{
	return 0x130;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_gen_int_w1s
 *
 * RVU Admin Function General Interrupt Set Register This register sets
 * interrupt bits.
 */
union rvu_af_gen_int_w1s {
	u64 u;
	struct rvu_af_gen_int_w1s_s {
		u64 unmapped                         : 1;
		u64 msix_fault                       : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct rvu_af_gen_int_w1s_s cn; */
};

static inline u64 RVU_AF_GEN_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_GEN_INT_W1S(void)
{
	return 0x128;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_hwvf_rst
 *
 * RVU Admin Function Hardware VF Reset Register
 */
union rvu_af_hwvf_rst {
	u64 u;
	struct rvu_af_hwvf_rst_s {
		u64 hwvf                             : 8;
		u64 reserved_8_11                    : 4;
		u64 exec                             : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct rvu_af_hwvf_rst_s cn; */
};

static inline u64 RVU_AF_HWVF_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_HWVF_RST(void)
{
	return 0x2850;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_msixtr_base
 *
 * RVU Admin Function MSI-X Table Region Base-Address Register
 */
union rvu_af_msixtr_base {
	u64 u;
	struct rvu_af_msixtr_base_s {
		u64 reserved_0_6                     : 7;
		u64 addr                             : 46;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct rvu_af_msixtr_base_s cn; */
};

static inline u64 RVU_AF_MSIXTR_BASE(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_MSIXTR_BASE(void)
{
	return 0x10;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pf#_vf_bar4_addr
 *
 * RVU Admin Function PF/VF BAR4 Address Registers
 */
union rvu_af_pfx_vf_bar4_addr {
	u64 u;
	struct rvu_af_pfx_vf_bar4_addr_s {
		u64 reserved_0_15                    : 16;
		u64 addr                             : 48;
	} s;
	/* struct rvu_af_pfx_vf_bar4_addr_s cn; */
};

static inline u64 RVU_AF_PFX_VF_BAR4_ADDR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFX_VF_BAR4_ADDR(u64 a)
{
	return 0x1000 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pf_bar4_addr
 *
 * RVU Admin Function PF BAR4 Address Registers
 */
union rvu_af_pf_bar4_addr {
	u64 u;
	struct rvu_af_pf_bar4_addr_s {
		u64 reserved_0_15                    : 16;
		u64 addr                             : 48;
	} s;
	/* struct rvu_af_pf_bar4_addr_s cn; */
};

static inline u64 RVU_AF_PF_BAR4_ADDR(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PF_BAR4_ADDR(void)
{
	return 0x40;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pf_rst
 *
 * RVU Admin Function PF Reset Register
 */
union rvu_af_pf_rst {
	u64 u;
	struct rvu_af_pf_rst_s {
		u64 pf                               : 4;
		u64 reserved_4_11                    : 8;
		u64 exec                             : 1;
		u64 reserved_13_63                   : 51;
	} s;
	/* struct rvu_af_pf_rst_s cn; */
};

static inline u64 RVU_AF_PF_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PF_RST(void)
{
	return 0x2840;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfaf_mbox_int
 *
 * RVU Admin Function PF to AF Mailbox Interrupt Registers
 */
union rvu_af_pfaf_mbox_int {
	u64 u;
	struct rvu_af_pfaf_mbox_int_s {
		u64 mbox                             : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfaf_mbox_int_s cn; */
};

static inline u64 RVU_AF_PFAF_MBOX_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFAF_MBOX_INT(void)
{
	return 0x2880;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfaf_mbox_int_ena_w1c
 *
 * RVU Admin Function PF to AF Mailbox Interrupt Enable Clear Registers
 * This register clears interrupt enable bits.
 */
union rvu_af_pfaf_mbox_int_ena_w1c {
	u64 u;
	struct rvu_af_pfaf_mbox_int_ena_w1c_s {
		u64 mbox                             : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfaf_mbox_int_ena_w1c_s cn; */
};

static inline u64 RVU_AF_PFAF_MBOX_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFAF_MBOX_INT_ENA_W1C(void)
{
	return 0x2898;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfaf_mbox_int_ena_w1s
 *
 * RVU Admin Function PF to AF Mailbox Interrupt Enable Set Registers
 * This register sets interrupt enable bits.
 */
union rvu_af_pfaf_mbox_int_ena_w1s {
	u64 u;
	struct rvu_af_pfaf_mbox_int_ena_w1s_s {
		u64 mbox                             : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfaf_mbox_int_ena_w1s_s cn; */
};

static inline u64 RVU_AF_PFAF_MBOX_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFAF_MBOX_INT_ENA_W1S(void)
{
	return 0x2890;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfaf_mbox_int_w1s
 *
 * RVU Admin Function PF to AF Mailbox Interrupt Set Registers This
 * register sets interrupt bits.
 */
union rvu_af_pfaf_mbox_int_w1s {
	u64 u;
	struct rvu_af_pfaf_mbox_int_w1s_s {
		u64 mbox                             : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfaf_mbox_int_w1s_s cn; */
};

static inline u64 RVU_AF_PFAF_MBOX_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFAF_MBOX_INT_W1S(void)
{
	return 0x2888;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfflr_int
 *
 * RVU Admin Function PF Function Level Reset Interrupt Registers
 */
union rvu_af_pfflr_int {
	u64 u;
	struct rvu_af_pfflr_int_s {
		u64 flr                              : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfflr_int_s cn; */
};

static inline u64 RVU_AF_PFFLR_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFFLR_INT(void)
{
	return 0x28a0;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfflr_int_ena_w1c
 *
 * RVU Admin Function PF Function Level Reset Interrupt Enable Clear
 * Registers This register clears interrupt enable bits.
 */
union rvu_af_pfflr_int_ena_w1c {
	u64 u;
	struct rvu_af_pfflr_int_ena_w1c_s {
		u64 flr                              : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfflr_int_ena_w1c_s cn; */
};

static inline u64 RVU_AF_PFFLR_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFFLR_INT_ENA_W1C(void)
{
	return 0x28b8;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfflr_int_ena_w1s
 *
 * RVU Admin Function PF Function Level Reset Interrupt Enable Set
 * Registers This register sets interrupt enable bits.
 */
union rvu_af_pfflr_int_ena_w1s {
	u64 u;
	struct rvu_af_pfflr_int_ena_w1s_s {
		u64 flr                              : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfflr_int_ena_w1s_s cn; */
};

static inline u64 RVU_AF_PFFLR_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFFLR_INT_ENA_W1S(void)
{
	return 0x28b0;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfflr_int_w1s
 *
 * RVU Admin Function PF Function Level Reset Interrupt Set Registers
 * This register sets interrupt bits.
 */
union rvu_af_pfflr_int_w1s {
	u64 u;
	struct rvu_af_pfflr_int_w1s_s {
		u64 flr                              : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfflr_int_w1s_s cn; */
};

static inline u64 RVU_AF_PFFLR_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFFLR_INT_W1S(void)
{
	return 0x28a8;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfme_int
 *
 * RVU Admin Function PF Bus Master Enable Interrupt Registers
 */
union rvu_af_pfme_int {
	u64 u;
	struct rvu_af_pfme_int_s {
		u64 me                               : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfme_int_s cn; */
};

static inline u64 RVU_AF_PFME_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFME_INT(void)
{
	return 0x28c0;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfme_int_ena_w1c
 *
 * RVU Admin Function PF Bus Master Enable Interrupt Enable Clear
 * Registers This register clears interrupt enable bits.
 */
union rvu_af_pfme_int_ena_w1c {
	u64 u;
	struct rvu_af_pfme_int_ena_w1c_s {
		u64 me                               : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfme_int_ena_w1c_s cn; */
};

static inline u64 RVU_AF_PFME_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFME_INT_ENA_W1C(void)
{
	return 0x28d8;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfme_int_ena_w1s
 *
 * RVU Admin Function PF Bus Master Enable Interrupt Enable Set Registers
 * This register sets interrupt enable bits.
 */
union rvu_af_pfme_int_ena_w1s {
	u64 u;
	struct rvu_af_pfme_int_ena_w1s_s {
		u64 me                               : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfme_int_ena_w1s_s cn; */
};

static inline u64 RVU_AF_PFME_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFME_INT_ENA_W1S(void)
{
	return 0x28d0;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfme_int_w1s
 *
 * RVU Admin Function PF Bus Master Enable Interrupt Set Registers This
 * register sets interrupt bits.
 */
union rvu_af_pfme_int_w1s {
	u64 u;
	struct rvu_af_pfme_int_w1s_s {
		u64 me                               : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfme_int_w1s_s cn; */
};

static inline u64 RVU_AF_PFME_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFME_INT_W1S(void)
{
	return 0x28c8;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pfme_status
 *
 * RVU Admin Function PF Bus Master Enable Status Registers
 */
union rvu_af_pfme_status {
	u64 u;
	struct rvu_af_pfme_status_s {
		u64 me                               : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pfme_status_s cn; */
};

static inline u64 RVU_AF_PFME_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFME_STATUS(void)
{
	return 0x2800;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pftrpend
 *
 * RVU Admin Function PF Transaction Pending Registers
 */
union rvu_af_pftrpend {
	u64 u;
	struct rvu_af_pftrpend_s {
		u64 trpend                           : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pftrpend_s cn; */
};

static inline u64 RVU_AF_PFTRPEND(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFTRPEND(void)
{
	return 0x2810;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_pftrpend_w1s
 *
 * RVU Admin Function PF Transaction Pending Set Registers This register
 * reads or sets bits.
 */
union rvu_af_pftrpend_w1s {
	u64 u;
	struct rvu_af_pftrpend_w1s_s {
		u64 trpend                           : 16;
		u64 reserved_16_63                   : 48;
	} s;
	/* struct rvu_af_pftrpend_w1s_s cn; */
};

static inline u64 RVU_AF_PFTRPEND_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_PFTRPEND_W1S(void)
{
	return 0x2820;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_ras
 *
 * RVU Admin Function RAS Interrupt Register This register is intended
 * for delivery of RAS events to the SCP, so should be ignored by OS
 * drivers.
 */
union rvu_af_ras {
	u64 u;
	struct rvu_af_ras_s {
		u64 msix_poison                      : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_af_ras_s cn; */
};

static inline u64 RVU_AF_RAS(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_RAS(void)
{
	return 0x100;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_ras_ena_w1c
 *
 * RVU Admin Function RAS Interrupt Enable Clear Register This register
 * clears interrupt enable bits.
 */
union rvu_af_ras_ena_w1c {
	u64 u;
	struct rvu_af_ras_ena_w1c_s {
		u64 msix_poison                      : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_af_ras_ena_w1c_s cn; */
};

static inline u64 RVU_AF_RAS_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_RAS_ENA_W1C(void)
{
	return 0x118;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_ras_ena_w1s
 *
 * RVU Admin Function RAS Interrupt Enable Set Register This register
 * sets interrupt enable bits.
 */
union rvu_af_ras_ena_w1s {
	u64 u;
	struct rvu_af_ras_ena_w1s_s {
		u64 msix_poison                      : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_af_ras_ena_w1s_s cn; */
};

static inline u64 RVU_AF_RAS_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_RAS_ENA_W1S(void)
{
	return 0x110;
}

/**
 * Register (RVU_PF_BAR0) rvu_af_ras_w1s
 *
 * RVU Admin Function RAS Interrupt Set Register This register sets
 * interrupt bits.
 */
union rvu_af_ras_w1s {
	u64 u;
	struct rvu_af_ras_w1s_s {
		u64 msix_poison                      : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_af_ras_w1s_s cn; */
};

static inline u64 RVU_AF_RAS_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_AF_RAS_W1S(void)
{
	return 0x108;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_block_addr#_disc
 *
 * RVU PF Block Address Discovery Registers These registers allow each PF
 * driver to discover block resources that are provisioned to its PF. The
 * register's BLOCK_ADDR index is enumerated by RVU_BLOCK_ADDR_E.
 */
union rvu_pf_block_addrx_disc {
	u64 u;
	struct rvu_pf_block_addrx_disc_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_10                    : 2;
		u64 imp                              : 1;
		u64 rid                              : 8;
		u64 btype                            : 8;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct rvu_pf_block_addrx_disc_s cn; */
};

static inline u64 RVU_PF_BLOCK_ADDRX_DISC(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_BLOCK_ADDRX_DISC(u64 a)
{
	return 0x200 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_int
 *
 * RVU PF Interrupt Registers
 */
union rvu_pf_int {
	u64 u;
	struct rvu_pf_int_s {
		u64 mbox                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_pf_int_s cn; */
};

static inline u64 RVU_PF_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_INT(void)
{
	return 0xc20;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_int_ena_w1c
 *
 * RVU PF Interrupt Enable Clear Register This register clears interrupt
 * enable bits.
 */
union rvu_pf_int_ena_w1c {
	u64 u;
	struct rvu_pf_int_ena_w1c_s {
		u64 mbox                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_pf_int_ena_w1c_s cn; */
};

static inline u64 RVU_PF_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_INT_ENA_W1C(void)
{
	return 0xc38;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_int_ena_w1s
 *
 * RVU PF Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union rvu_pf_int_ena_w1s {
	u64 u;
	struct rvu_pf_int_ena_w1s_s {
		u64 mbox                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_pf_int_ena_w1s_s cn; */
};

static inline u64 RVU_PF_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_INT_ENA_W1S(void)
{
	return 0xc30;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_int_w1s
 *
 * RVU PF Interrupt Set Register This register sets interrupt bits.
 */
union rvu_pf_int_w1s {
	u64 u;
	struct rvu_pf_int_w1s_s {
		u64 mbox                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_pf_int_w1s_s cn; */
};

static inline u64 RVU_PF_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_INT_W1S(void)
{
	return 0xc28;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_msix_pba#
 *
 * RVU PF MSI-X Pending-Bit-Array Registers This register is the MSI-X PF
 * PBA table.
 */
union rvu_pf_msix_pbax {
	u64 u;
	struct rvu_pf_msix_pbax_s {
		u64 pend                             : 64;
	} s;
	/* struct rvu_pf_msix_pbax_s cn; */
};

static inline u64 RVU_PF_MSIX_PBAX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_MSIX_PBAX(u64 a)
{
	return 0xf0000 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_msix_vec#_addr
 *
 * RVU PF MSI-X Vector-Table Address Registers These registers and
 * RVU_PF_MSIX_VEC()_CTL form the PF MSI-X vector table. The number of
 * MSI-X vectors for a given PF is specified by
 * RVU_PRIV_PF()_MSIX_CFG[PF_MSIXT_SIZEM1] (plus 1).  Software must do a
 * read after any writes to the MSI-X vector table to ensure that the
 * writes have completed before interrupts are generated to the modified
 * vectors.
 */
union rvu_pf_msix_vecx_addr {
	u64 u;
	struct rvu_pf_msix_vecx_addr_s {
		u64 secvec                           : 1;
		u64 reserved_1                       : 1;
		u64 addr                             : 51;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct rvu_pf_msix_vecx_addr_s cn; */
};

static inline u64 RVU_PF_MSIX_VECX_ADDR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_MSIX_VECX_ADDR(u64 a)
{
	return 0x80000 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_msix_vec#_ctl
 *
 * RVU PF MSI-X Vector-Table Control and Data Registers These registers
 * and RVU_PF_MSIX_VEC()_ADDR form the PF MSI-X vector table.
 */
union rvu_pf_msix_vecx_ctl {
	u64 u;
	struct rvu_pf_msix_vecx_ctl_s {
		u64 data                             : 32;
		u64 mask                             : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct rvu_pf_msix_vecx_ctl_s cn; */
};

static inline u64 RVU_PF_MSIX_VECX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_MSIX_VECX_CTL(u64 a)
{
	return 0x80008 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_pfaf_mbox#
 *
 * RVU PF/AF Mailbox Registers
 */
union rvu_pf_pfaf_mboxx {
	u64 u;
	struct rvu_pf_pfaf_mboxx_s {
		u64 data                             : 64;
	} s;
	/* struct rvu_pf_pfaf_mboxx_s cn; */
};

static inline u64 RVU_PF_PFAF_MBOXX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_PFAF_MBOXX(u64 a)
{
	return 0xc00 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vf#_pfvf_mbox#
 *
 * RVU PF/VF Mailbox Registers
 */
union rvu_pf_vfx_pfvf_mboxx {
	u64 u;
	struct rvu_pf_vfx_pfvf_mboxx_s {
		u64 data                             : 64;
	} s;
	/* struct rvu_pf_vfx_pfvf_mboxx_s cn; */
};

static inline u64 RVU_PF_VFX_PFVF_MBOXX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFX_PFVF_MBOXX(u64 a, u64 b)
{
	return 0 + 0x1000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vf_bar4_addr
 *
 * RVU PF VF BAR4 Address Registers
 */
union rvu_pf_vf_bar4_addr {
	u64 u;
	struct rvu_pf_vf_bar4_addr_s {
		u64 reserved_0_15                    : 16;
		u64 addr                             : 48;
	} s;
	/* struct rvu_pf_vf_bar4_addr_s cn; */
};

static inline u64 RVU_PF_VF_BAR4_ADDR(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VF_BAR4_ADDR(void)
{
	return 0x10;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfflr_int#
 *
 * RVU PF VF Function Level Reset Interrupt Registers
 */
union rvu_pf_vfflr_intx {
	u64 u;
	struct rvu_pf_vfflr_intx_s {
		u64 flr                              : 64;
	} s;
	/* struct rvu_pf_vfflr_intx_s cn; */
};

static inline u64 RVU_PF_VFFLR_INTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFFLR_INTX(u64 a)
{
	return 0x900 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfflr_int_ena_w1c#
 *
 * RVU PF VF Function Level Reset Interrupt Enable Clear Registers This
 * register clears interrupt enable bits.
 */
union rvu_pf_vfflr_int_ena_w1cx {
	u64 u;
	struct rvu_pf_vfflr_int_ena_w1cx_s {
		u64 flr                              : 64;
	} s;
	/* struct rvu_pf_vfflr_int_ena_w1cx_s cn; */
};

static inline u64 RVU_PF_VFFLR_INT_ENA_W1CX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFFLR_INT_ENA_W1CX(u64 a)
{
	return 0x960 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfflr_int_ena_w1s#
 *
 * RVU PF VF Function Level Reset Interrupt Enable Set Registers This
 * register sets interrupt enable bits.
 */
union rvu_pf_vfflr_int_ena_w1sx {
	u64 u;
	struct rvu_pf_vfflr_int_ena_w1sx_s {
		u64 flr                              : 64;
	} s;
	/* struct rvu_pf_vfflr_int_ena_w1sx_s cn; */
};

static inline u64 RVU_PF_VFFLR_INT_ENA_W1SX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFFLR_INT_ENA_W1SX(u64 a)
{
	return 0x940 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfflr_int_w1s#
 *
 * RVU PF VF Function Level Reset Interrupt Set Registers This register
 * sets interrupt bits.
 */
union rvu_pf_vfflr_int_w1sx {
	u64 u;
	struct rvu_pf_vfflr_int_w1sx_s {
		u64 flr                              : 64;
	} s;
	/* struct rvu_pf_vfflr_int_w1sx_s cn; */
};

static inline u64 RVU_PF_VFFLR_INT_W1SX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFFLR_INT_W1SX(u64 a)
{
	return 0x920 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfme_int#
 *
 * RVU PF VF Bus Master Enable Interrupt Registers
 */
union rvu_pf_vfme_intx {
	u64 u;
	struct rvu_pf_vfme_intx_s {
		u64 me                               : 64;
	} s;
	/* struct rvu_pf_vfme_intx_s cn; */
};

static inline u64 RVU_PF_VFME_INTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFME_INTX(u64 a)
{
	return 0x980 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfme_int_ena_w1c#
 *
 * RVU PF VF Bus Master Enable Interrupt Enable Clear Registers This
 * register clears interrupt enable bits.
 */
union rvu_pf_vfme_int_ena_w1cx {
	u64 u;
	struct rvu_pf_vfme_int_ena_w1cx_s {
		u64 me                               : 64;
	} s;
	/* struct rvu_pf_vfme_int_ena_w1cx_s cn; */
};

static inline u64 RVU_PF_VFME_INT_ENA_W1CX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFME_INT_ENA_W1CX(u64 a)
{
	return 0x9e0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfme_int_ena_w1s#
 *
 * RVU PF VF Bus Master Enable Interrupt Enable Set Registers This
 * register sets interrupt enable bits.
 */
union rvu_pf_vfme_int_ena_w1sx {
	u64 u;
	struct rvu_pf_vfme_int_ena_w1sx_s {
		u64 me                               : 64;
	} s;
	/* struct rvu_pf_vfme_int_ena_w1sx_s cn; */
};

static inline u64 RVU_PF_VFME_INT_ENA_W1SX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFME_INT_ENA_W1SX(u64 a)
{
	return 0x9c0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfme_int_w1s#
 *
 * RVU PF VF Bus Master Enable Interrupt Set Registers This register sets
 * interrupt bits.
 */
union rvu_pf_vfme_int_w1sx {
	u64 u;
	struct rvu_pf_vfme_int_w1sx_s {
		u64 me                               : 64;
	} s;
	/* struct rvu_pf_vfme_int_w1sx_s cn; */
};

static inline u64 RVU_PF_VFME_INT_W1SX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFME_INT_W1SX(u64 a)
{
	return 0x9a0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfme_status#
 *
 * RVU PF VF Bus Master Enable Status Registers
 */
union rvu_pf_vfme_statusx {
	u64 u;
	struct rvu_pf_vfme_statusx_s {
		u64 me                               : 64;
	} s;
	/* struct rvu_pf_vfme_statusx_s cn; */
};

static inline u64 RVU_PF_VFME_STATUSX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFME_STATUSX(u64 a)
{
	return 0x800 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfpf_mbox_int#
 *
 * RVU VF to PF Mailbox Interrupt Registers
 */
union rvu_pf_vfpf_mbox_intx {
	u64 u;
	struct rvu_pf_vfpf_mbox_intx_s {
		u64 mbox                             : 64;
	} s;
	/* struct rvu_pf_vfpf_mbox_intx_s cn; */
};

static inline u64 RVU_PF_VFPF_MBOX_INTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFPF_MBOX_INTX(u64 a)
{
	return 0x880 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfpf_mbox_int_ena_w1c#
 *
 * RVU VF to PF Mailbox Interrupt Enable Clear Registers This register
 * clears interrupt enable bits.
 */
union rvu_pf_vfpf_mbox_int_ena_w1cx {
	u64 u;
	struct rvu_pf_vfpf_mbox_int_ena_w1cx_s {
		u64 mbox                             : 64;
	} s;
	/* struct rvu_pf_vfpf_mbox_int_ena_w1cx_s cn; */
};

static inline u64 RVU_PF_VFPF_MBOX_INT_ENA_W1CX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFPF_MBOX_INT_ENA_W1CX(u64 a)
{
	return 0x8e0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfpf_mbox_int_ena_w1s#
 *
 * RVU VF to PF Mailbox Interrupt Enable Set Registers This register sets
 * interrupt enable bits.
 */
union rvu_pf_vfpf_mbox_int_ena_w1sx {
	u64 u;
	struct rvu_pf_vfpf_mbox_int_ena_w1sx_s {
		u64 mbox                             : 64;
	} s;
	/* struct rvu_pf_vfpf_mbox_int_ena_w1sx_s cn; */
};

static inline u64 RVU_PF_VFPF_MBOX_INT_ENA_W1SX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFPF_MBOX_INT_ENA_W1SX(u64 a)
{
	return 0x8c0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vfpf_mbox_int_w1s#
 *
 * RVU VF to PF Mailbox Interrupt Set Registers This register sets
 * interrupt bits.
 */
union rvu_pf_vfpf_mbox_int_w1sx {
	u64 u;
	struct rvu_pf_vfpf_mbox_int_w1sx_s {
		u64 mbox                             : 64;
	} s;
	/* struct rvu_pf_vfpf_mbox_int_w1sx_s cn; */
};

static inline u64 RVU_PF_VFPF_MBOX_INT_W1SX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFPF_MBOX_INT_W1SX(u64 a)
{
	return 0x8a0 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vftrpend#
 *
 * RVU PF VF Transaction Pending Registers
 */
union rvu_pf_vftrpendx {
	u64 u;
	struct rvu_pf_vftrpendx_s {
		u64 trpend                           : 64;
	} s;
	/* struct rvu_pf_vftrpendx_s cn; */
};

static inline u64 RVU_PF_VFTRPENDX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFTRPENDX(u64 a)
{
	return 0x820 + 8 * a;
}

/**
 * Register (RVU_PF_BAR2) rvu_pf_vftrpend_w1s#
 *
 * RVU PF VF Transaction Pending Set Registers This register reads or
 * sets bits.
 */
union rvu_pf_vftrpend_w1sx {
	u64 u;
	struct rvu_pf_vftrpend_w1sx_s {
		u64 trpend                           : 64;
	} s;
	/* struct rvu_pf_vftrpend_w1sx_s cn; */
};

static inline u64 RVU_PF_VFTRPEND_W1SX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PF_VFTRPEND_W1SX(u64 a)
{
	return 0x840 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_active_pc
 *
 * RVU Active Program Counter Register
 */
union rvu_priv_active_pc {
	u64 u;
	struct rvu_priv_active_pc_s {
		u64 active_pc                        : 64;
	} s;
	/* struct rvu_priv_active_pc_s cn; */
};

static inline u64 RVU_PRIV_ACTIVE_PC(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_ACTIVE_PC(void)
{
	return 0x8000030;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_block_type#_rev
 *
 * RVU Privileged Block Type Revision Registers These registers are used
 * by configuration software to specify the revision ID of each block
 * type enumerated by RVU_BLOCK_TYPE_E, to assist VF/PF software
 * discovery.
 */
union rvu_priv_block_typex_rev {
	u64 u;
	struct rvu_priv_block_typex_rev_s {
		u64 rid                              : 8;
		u64 reserved_8_63                    : 56;
	} s;
	/* struct rvu_priv_block_typex_rev_s cn; */
};

static inline u64 RVU_PRIV_BLOCK_TYPEX_REV(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_BLOCK_TYPEX_REV(u64 a)
{
	return 0x8000400 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_clk_cfg
 *
 * RVU Privileged General Configuration Register
 */
union rvu_priv_clk_cfg {
	u64 u;
	struct rvu_priv_clk_cfg_s {
		u64 blk_clken                        : 1;
		u64 ncbi_clken                       : 1;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct rvu_priv_clk_cfg_s cn; */
};

static inline u64 RVU_PRIV_CLK_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_CLK_CFG(void)
{
	return 0x8000020;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_const
 *
 * RVU Privileged Constants Register This register contains constants for
 * software discovery.
 */
union rvu_priv_const {
	u64 u;
	struct rvu_priv_const_s {
		u64 max_msix                         : 20;
		u64 hwvfs                            : 12;
		u64 pfs                              : 8;
		u64 max_vfs_per_pf                   : 8;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct rvu_priv_const_s cn; */
};

static inline u64 RVU_PRIV_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_CONST(void)
{
	return 0x8000000;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_gen_cfg
 *
 * RVU Privileged General Configuration Register
 */
union rvu_priv_gen_cfg {
	u64 u;
	struct rvu_priv_gen_cfg_s {
		u64 lock                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_priv_gen_cfg_s cn; */
};

static inline u64 RVU_PRIV_GEN_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_GEN_CFG(void)
{
	return 0x8000010;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_hwvf#_cpt#_cfg
 *
 * RVU Privileged Hardware VF CPT Configuration Registers Similar to
 * RVU_PRIV_HWVF()_NIX()_CFG, but for CPT({a}) block.
 */
union rvu_priv_hwvfx_cptx_cfg {
	u64 u;
	struct rvu_priv_hwvfx_cptx_cfg_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct rvu_priv_hwvfx_cptx_cfg_s cn; */
};

static inline u64 RVU_PRIV_HWVFX_CPTX_CFG(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_HWVFX_CPTX_CFG(u64 a, u64 b)
{
	return 0x8001350 + 0x10000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_hwvf#_int_cfg
 *
 * RVU Privileged Hardware VF Interrupt Configuration Registers
 */
union rvu_priv_hwvfx_int_cfg {
	u64 u;
	struct rvu_priv_hwvfx_int_cfg_s {
		u64 msix_offset                      : 11;
		u64 reserved_11                      : 1;
		u64 msix_size                        : 8;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct rvu_priv_hwvfx_int_cfg_s cn; */
};

static inline u64 RVU_PRIV_HWVFX_INT_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_HWVFX_INT_CFG(u64 a)
{
	return 0x8001280 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_hwvf#_nix#_cfg
 *
 * RVU Privileged Hardware VF NIX Configuration Registers These registers
 * are used to assist VF software discovery. For each HWVF, if the HWVF
 * is mapped to a VF by RVU_PRIV_PF()_CFG[FIRST_HWVF,NVF], software
 * writes NIX block's resource configuration for the VF in this register.
 * The VF driver can read RVU_VF_BLOCK_ADDR()_DISC to discover the
 * configuration.
 */
union rvu_priv_hwvfx_nixx_cfg {
	u64 u;
	struct rvu_priv_hwvfx_nixx_cfg_s {
		u64 has_lf                           : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_priv_hwvfx_nixx_cfg_s cn; */
};

static inline u64 RVU_PRIV_HWVFX_NIXX_CFG(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_HWVFX_NIXX_CFG(u64 a, u64 b)
{
	return 0x8001300 + 0x10000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_hwvf#_npa_cfg
 *
 * RVU Privileged Hardware VF NPA Configuration Registers Similar to
 * RVU_PRIV_HWVF()_NIX()_CFG, but for NPA block.
 */
union rvu_priv_hwvfx_npa_cfg {
	u64 u;
	struct rvu_priv_hwvfx_npa_cfg_s {
		u64 has_lf                           : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_priv_hwvfx_npa_cfg_s cn; */
};

static inline u64 RVU_PRIV_HWVFX_NPA_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_HWVFX_NPA_CFG(u64 a)
{
	return 0x8001310 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_hwvf#_sso_cfg
 *
 * RVU Privileged Hardware VF SSO Configuration Registers Similar to
 * RVU_PRIV_HWVF()_NIX()_CFG, but for SSO block.
 */
union rvu_priv_hwvfx_sso_cfg {
	u64 u;
	struct rvu_priv_hwvfx_sso_cfg_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct rvu_priv_hwvfx_sso_cfg_s cn; */
};

static inline u64 RVU_PRIV_HWVFX_SSO_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_HWVFX_SSO_CFG(u64 a)
{
	return 0x8001320 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_hwvf#_ssow_cfg
 *
 * RVU Privileged Hardware VF SSO Work Slot Configuration Registers
 * Similar to RVU_PRIV_HWVF()_NIX()_CFG, but for SSOW block.
 */
union rvu_priv_hwvfx_ssow_cfg {
	u64 u;
	struct rvu_priv_hwvfx_ssow_cfg_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct rvu_priv_hwvfx_ssow_cfg_s cn; */
};

static inline u64 RVU_PRIV_HWVFX_SSOW_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_HWVFX_SSOW_CFG(u64 a)
{
	return 0x8001330 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_hwvf#_tim_cfg
 *
 * RVU Privileged Hardware VF SSO Work Slot Configuration Registers
 * Similar to RVU_PRIV_HWVF()_NIX()_CFG, but for TIM block.
 */
union rvu_priv_hwvfx_tim_cfg {
	u64 u;
	struct rvu_priv_hwvfx_tim_cfg_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct rvu_priv_hwvfx_tim_cfg_s cn; */
};

static inline u64 RVU_PRIV_HWVFX_TIM_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_HWVFX_TIM_CFG(u64 a)
{
	return 0x8001340 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_cfg
 *
 * RVU Privileged PF Configuration Registers
 */
union rvu_priv_pfx_cfg {
	u64 u;
	struct rvu_priv_pfx_cfg_s {
		u64 first_hwvf                       : 12;
		u64 nvf                              : 8;
		u64 ena                              : 1;
		u64 af_ena                           : 1;
		u64 me_flr_ena                       : 1;
		u64 pf_vf_io_bar4                    : 1;
		u64 reserved_24_63                   : 40;
	} s;
	struct rvu_priv_pfx_cfg_cn96xxp1 {
		u64 first_hwvf                       : 12;
		u64 nvf                              : 8;
		u64 ena                              : 1;
		u64 af_ena                           : 1;
		u64 me_flr_ena                       : 1;
		u64 reserved_23_63                   : 41;
	} cn96xxp1;
	/* struct rvu_priv_pfx_cfg_s cn96xxp3; */
	/* struct rvu_priv_pfx_cfg_cn96xxp1 cnf95xx; */
};

static inline u64 RVU_PRIV_PFX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_CFG(u64 a)
{
	return 0x8000100 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_cpt#_cfg
 *
 * RVU Privileged PF CPT Configuration Registers Similar to
 * RVU_PRIV_PF()_NIX()_CFG, but for CPT({a}) block.
 */
union rvu_priv_pfx_cptx_cfg {
	u64 u;
	struct rvu_priv_pfx_cptx_cfg_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct rvu_priv_pfx_cptx_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_CPTX_CFG(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_CPTX_CFG(u64 a, u64 b)
{
	return 0x8000350 + 0x10000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_id_cfg
 *
 * RVU Privileged PF ID Configuration Registers
 */
union rvu_priv_pfx_id_cfg {
	u64 u;
	struct rvu_priv_pfx_id_cfg_s {
		u64 pf_devid                         : 8;
		u64 vf_devid                         : 8;
		u64 class_code                       : 24;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct rvu_priv_pfx_id_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_ID_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_ID_CFG(u64 a)
{
	return 0x8000120 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_int_cfg
 *
 * RVU Privileged PF Interrupt Configuration Registers
 */
union rvu_priv_pfx_int_cfg {
	u64 u;
	struct rvu_priv_pfx_int_cfg_s {
		u64 msix_offset                      : 11;
		u64 reserved_11                      : 1;
		u64 msix_size                        : 8;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct rvu_priv_pfx_int_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_INT_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_INT_CFG(u64 a)
{
	return 0x8000200 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_msix_cfg
 *
 * RVU Privileged PF MSI-X Configuration Registers These registers
 * specify MSI-X table sizes and locations for RVU PFs and associated
 * VFs. Hardware maintains all RVU MSI-X tables in a contiguous memory
 * region in LLC/DRAM called the MSI-X table region. The table region's
 * base AF IOVA is specified by RVU_AF_MSIXTR_BASE, and its size as a
 * multiple of 16-byte RVU_MSIX_VEC_S structures must be less than or
 * equal to RVU_PRIV_CONST[MAX_MSIX].  A PF's MSI-X table consists of the
 * following range of RVU_MSIX_VEC_S structures in the table region: *
 * First index: [PF_MSIXT_OFFSET]. * Last index: [PF_MSIXT_OFFSET] +
 * [PF_MSIXT_SIZEM1].  If a PF has enabled VFs (associated
 * RVU_PRIV_PF()_CFG[NVF] is nonzero), then each VF's MSI-X table
 * consumes the following range of RVU_MSIX_VEC_S structures: * First
 * index: [VF_MSIXT_OFFSET] + N*([VF_MSIXT_SIZEM1] + 1). * Last index:
 * [VF_MSIXT_OFFSET] + N*([VF_MSIXT_SIZEM1] + 1) + [VF_MSIXT_SIZEM1].
 * N=0 for the first VF, N=1 for the second VF, etc.  Different PFs and
 * VFs must have non-overlapping vector ranges, and the last index of any
 * range must be less than RVU_PRIV_CONST[MAX_MSIX].
 */
union rvu_priv_pfx_msix_cfg {
	u64 u;
	struct rvu_priv_pfx_msix_cfg_s {
		u64 vf_msixt_sizem1                  : 12;
		u64 vf_msixt_offset                  : 20;
		u64 pf_msixt_sizem1                  : 12;
		u64 pf_msixt_offset                  : 20;
	} s;
	/* struct rvu_priv_pfx_msix_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_MSIX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_MSIX_CFG(u64 a)
{
	return 0x8000110 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_nix#_cfg
 *
 * RVU Privileged PF NIX Configuration Registers These registers are used
 * to assist PF software discovery. For each enabled RVU PF, software
 * writes the block's resource configuration for the PF in this register.
 * The PF driver can read RVU_PF_BLOCK_ADDR()_DISC to discover the
 * configuration.
 */
union rvu_priv_pfx_nixx_cfg {
	u64 u;
	struct rvu_priv_pfx_nixx_cfg_s {
		u64 has_lf                           : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_priv_pfx_nixx_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_NIXX_CFG(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_NIXX_CFG(u64 a, u64 b)
{
	return 0x8000300 + 0x10000 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_npa_cfg
 *
 * RVU Privileged PF NPA Configuration Registers Similar to
 * RVU_PRIV_PF()_NIX()_CFG, but for NPA block.
 */
union rvu_priv_pfx_npa_cfg {
	u64 u;
	struct rvu_priv_pfx_npa_cfg_s {
		u64 has_lf                           : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_priv_pfx_npa_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_NPA_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_NPA_CFG(u64 a)
{
	return 0x8000310 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_sso_cfg
 *
 * RVU Privileged PF SSO Configuration Registers Similar to
 * RVU_PRIV_PF()_NIX()_CFG, but for SSO block.
 */
union rvu_priv_pfx_sso_cfg {
	u64 u;
	struct rvu_priv_pfx_sso_cfg_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct rvu_priv_pfx_sso_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_SSO_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_SSO_CFG(u64 a)
{
	return 0x8000320 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_ssow_cfg
 *
 * RVU Privileged PF SSO Work Slot Configuration Registers Similar to
 * RVU_PRIV_PF()_NIX()_CFG, but for SSOW block.
 */
union rvu_priv_pfx_ssow_cfg {
	u64 u;
	struct rvu_priv_pfx_ssow_cfg_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct rvu_priv_pfx_ssow_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_SSOW_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_SSOW_CFG(u64 a)
{
	return 0x8000330 + 0x10000 * a;
}

/**
 * Register (RVU_PF_BAR0) rvu_priv_pf#_tim_cfg
 *
 * RVU Privileged PF SSO Work Slot Configuration Registers Similar to
 * RVU_PRIV_PF()_NIX()_CFG, but for TIM block.
 */
union rvu_priv_pfx_tim_cfg {
	u64 u;
	struct rvu_priv_pfx_tim_cfg_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct rvu_priv_pfx_tim_cfg_s cn; */
};

static inline u64 RVU_PRIV_PFX_TIM_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_PRIV_PFX_TIM_CFG(u64 a)
{
	return 0x8000340 + 0x10000 * a;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_block_addr#_disc
 *
 * RVU VF Block Address Discovery Registers These registers allow each VF
 * driver to discover block resources that are provisioned to its VF. The
 * register's BLOCK_ADDR index is enumerated by RVU_BLOCK_ADDR_E.
 */
union rvu_vf_block_addrx_disc {
	u64 u;
	struct rvu_vf_block_addrx_disc_s {
		u64 num_lfs                          : 9;
		u64 reserved_9_10                    : 2;
		u64 imp                              : 1;
		u64 rid                              : 8;
		u64 btype                            : 8;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct rvu_vf_block_addrx_disc_s cn; */
};

static inline u64 RVU_VF_BLOCK_ADDRX_DISC(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_BLOCK_ADDRX_DISC(u64 a)
{
	return 0x200 + 8 * a;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_int
 *
 * RVU VF Interrupt Registers
 */
union rvu_vf_int {
	u64 u;
	struct rvu_vf_int_s {
		u64 mbox                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_vf_int_s cn; */
};

static inline u64 RVU_VF_INT(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_INT(void)
{
	return 0x20;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_int_ena_w1c
 *
 * RVU VF Interrupt Enable Clear Register This register clears interrupt
 * enable bits.
 */
union rvu_vf_int_ena_w1c {
	u64 u;
	struct rvu_vf_int_ena_w1c_s {
		u64 mbox                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_vf_int_ena_w1c_s cn; */
};

static inline u64 RVU_VF_INT_ENA_W1C(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_INT_ENA_W1C(void)
{
	return 0x38;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_int_ena_w1s
 *
 * RVU VF Interrupt Enable Set Register This register sets interrupt
 * enable bits.
 */
union rvu_vf_int_ena_w1s {
	u64 u;
	struct rvu_vf_int_ena_w1s_s {
		u64 mbox                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_vf_int_ena_w1s_s cn; */
};

static inline u64 RVU_VF_INT_ENA_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_INT_ENA_W1S(void)
{
	return 0x30;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_int_w1s
 *
 * RVU VF Interrupt Set Register This register sets interrupt bits.
 */
union rvu_vf_int_w1s {
	u64 u;
	struct rvu_vf_int_w1s_s {
		u64 mbox                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct rvu_vf_int_w1s_s cn; */
};

static inline u64 RVU_VF_INT_W1S(void)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_INT_W1S(void)
{
	return 0x28;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_msix_pba#
 *
 * RVU VF MSI-X Pending-Bit-Array Registers This register is the MSI-X VF
 * PBA table.
 */
union rvu_vf_msix_pbax {
	u64 u;
	struct rvu_vf_msix_pbax_s {
		u64 pend                             : 64;
	} s;
	/* struct rvu_vf_msix_pbax_s cn; */
};

static inline u64 RVU_VF_MSIX_PBAX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_MSIX_PBAX(u64 a)
{
	return 0xf0000 + 8 * a;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_msix_vec#_addr
 *
 * RVU VF MSI-X Vector-Table Address Registers These registers and
 * RVU_VF_MSIX_VEC()_CTL form the VF MSI-X vector table. The number of
 * MSI-X vectors for a given VF is specified by
 * RVU_PRIV_PF()_MSIX_CFG[VF_MSIXT_SIZEM1] (plus 1).  Software must do a
 * read after any writes to the MSI-X vector table to ensure that the
 * writes have completed before interrupts are generated to the modified
 * vectors.
 */
union rvu_vf_msix_vecx_addr {
	u64 u;
	struct rvu_vf_msix_vecx_addr_s {
		u64 secvec                           : 1;
		u64 reserved_1                       : 1;
		u64 addr                             : 51;
		u64 reserved_53_63                   : 11;
	} s;
	/* struct rvu_vf_msix_vecx_addr_s cn; */
};

static inline u64 RVU_VF_MSIX_VECX_ADDR(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_MSIX_VECX_ADDR(u64 a)
{
	return 0x80000 + 0x10 * a;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_msix_vec#_ctl
 *
 * RVU VF MSI-X Vector-Table Control and Data Registers These registers
 * and RVU_VF_MSIX_VEC()_ADDR form the VF MSI-X vector table.
 */
union rvu_vf_msix_vecx_ctl {
	u64 u;
	struct rvu_vf_msix_vecx_ctl_s {
		u64 data                             : 32;
		u64 mask                             : 1;
		u64 reserved_33_63                   : 31;
	} s;
	/* struct rvu_vf_msix_vecx_ctl_s cn; */
};

static inline u64 RVU_VF_MSIX_VECX_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_MSIX_VECX_CTL(u64 a)
{
	return 0x80008 + 0x10 * a;
}

/**
 * Register (RVU_VF_BAR2) rvu_vf_vfpf_mbox#
 *
 * RVU VF/PF Mailbox Registers
 */
union rvu_vf_vfpf_mboxx {
	u64 u;
	struct rvu_vf_vfpf_mboxx_s {
		u64 data                             : 64;
	} s;
	/* struct rvu_vf_vfpf_mboxx_s cn; */
};

static inline u64 RVU_VF_VFPF_MBOXX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 RVU_VF_VFPF_MBOXX(u64 a)
{
	return 0 + 8 * a;
}

#endif /* __CSRS_RVU_H__ */
