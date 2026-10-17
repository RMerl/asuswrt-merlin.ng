/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */
#ifndef __CSRS_NPC_H__
#define __CSRS_NPC_H__

/**
 * @file
 *
 * Configuration and status register (CSR) address and type definitions for
 * NPC.
 *
 * This file is auto generated.  Do not edit.
 *
 */

/**
 * Enumeration npc_errlev_e
 *
 * NPC Error Level Enumeration Enumerates the lowest protocol layer
 * containing an error.
 */
#define NPC_ERRLEV_E_LA (1)
#define NPC_ERRLEV_E_LB (2)
#define NPC_ERRLEV_E_LC (3)
#define NPC_ERRLEV_E_LD (4)
#define NPC_ERRLEV_E_LE (5)
#define NPC_ERRLEV_E_LF (6)
#define NPC_ERRLEV_E_LG (7)
#define NPC_ERRLEV_E_LH (8)
#define NPC_ERRLEV_E_NIX (0xf)
#define NPC_ERRLEV_E_RX(a) (0 + (a))
#define NPC_ERRLEV_E_RE (0)

/**
 * Enumeration npc_intf_e
 *
 * NPC Interface Enumeration Enumerates the NPC interfaces.
 */
#define NPC_INTF_E_NIXX_RX(a) (0 + 2 * (a))
#define NPC_INTF_E_NIXX_TX(a) (1 + 2 * (a))

/**
 * Enumeration npc_lid_e
 *
 * NPC Layer ID Enumeration Enumerates layers parsed by NPC.
 */
#define NPC_LID_E_LA (0)
#define NPC_LID_E_LB (1)
#define NPC_LID_E_LC (2)
#define NPC_LID_E_LD (3)
#define NPC_LID_E_LE (4)
#define NPC_LID_E_LF (5)
#define NPC_LID_E_LG (6)
#define NPC_LID_E_LH (7)

/**
 * Enumeration npc_lkupop_e
 *
 * NPC Lookup Operation Enumeration Enumerates the lookup operation for
 * NPC_AF_LKUP_CTL[OP].
 */
#define NPC_LKUPOP_E_KEY (1)
#define NPC_LKUPOP_E_PKT (0)

/**
 * Enumeration npc_mcamkeyw_e
 *
 * NPC MCAM Search Key Width Enumeration
 */
#define NPC_MCAMKEYW_E_X1 (0)
#define NPC_MCAMKEYW_E_X2 (1)
#define NPC_MCAMKEYW_E_X4 (2)

/**
 * Structure npc_layer_info_s
 *
 * NPC Layer Parse Information Structure This structure specifies the
 * format of NPC_RESULT_S[LA,LB,...,LH].
 */
union npc_layer_info_s {
	u32 u;
	struct npc_layer_info_s_s {
		u32 lptr                             : 8;
		u32 flags                            : 8;
		u32 ltype                            : 4;
		u32 reserved_20_31                   : 12;
	} s;
	/* struct npc_layer_info_s_s cn; */
};

/**
 * Structure npc_layer_kex_s
 *
 * NPC Layer MCAM Search Key Extract Structure This structure specifies
 * the format of each of the NPC_PARSE_KEX_S[LA,LB,...,LH] fields. It
 * contains the subset of NPC_LAYER_INFO_S fields that can be included in
 * the MCAM search key. See NPC_PARSE_KEX_S and NPC_AF_INTF()_KEX_CFG.
 */
union npc_layer_kex_s {
	u32 u;
	struct npc_layer_kex_s_s {
		u32 flags                            : 8;
		u32 ltype                            : 4;
		u32 reserved_12_31                   : 20;
	} s;
	/* struct npc_layer_kex_s_s cn; */
};

/**
 * Structure npc_mcam_key_x1_s
 *
 * NPC MCAM Search Key X1 Structure This structure specifies the MCAM
 * search key format used by an interface when
 * NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X1.
 */
union npc_mcam_key_x1_s {
	u64 u[3];
	struct npc_mcam_key_x1_s_s {
		u64 intf                             : 2;
		u64 reserved_2_63                    : 62;
		u64 kw0                              : 64;
		u64 kw1                              : 48;
		u64 reserved_176_191                 : 16;
	} s;
	/* struct npc_mcam_key_x1_s_s cn; */
};

/**
 * Structure npc_mcam_key_x2_s
 *
 * NPC MCAM Search Key X2 Structure This structure specifies the MCAM
 * search key format used by an interface when
 * NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X2.
 */
union npc_mcam_key_x2_s {
	u64 u[5];
	struct npc_mcam_key_x2_s_s {
		u64 intf                             : 2;
		u64 reserved_2_63                    : 62;
		u64 kw0                              : 64;
		u64 kw1                              : 64;
		u64 kw2                              : 64;
		u64 kw3                              : 32;
		u64 reserved_288_319                 : 32;
	} s;
	/* struct npc_mcam_key_x2_s_s cn; */
};

/**
 * Structure npc_mcam_key_x4_s
 *
 * NPC MCAM Search Key X4 Structure This structure specifies the MCAM
 * search key format used by an interface when
 * NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4.
 */
union npc_mcam_key_x4_s {
	u64 u[8];
	struct npc_mcam_key_x4_s_s {
		u64 intf                             : 2;
		u64 reserved_2_63                    : 62;
		u64 kw0                              : 64;
		u64 kw1                              : 64;
		u64 kw2                              : 64;
		u64 kw3                              : 64;
		u64 kw4                              : 64;
		u64 kw5                              : 64;
		u64 kw6                              : 64;
	} s;
	/* struct npc_mcam_key_x4_s_s cn; */
};

/**
 * Structure npc_parse_kex_s
 *
 * NPC Parse Key Extract Structure This structure contains the subset of
 * NPC_RESULT_S fields that can be included in the MCAM search key. See
 * NPC_AF_INTF()_KEX_CFG.
 */
union npc_parse_kex_s {
	u64 u[2];
	struct npc_parse_kex_s_s {
		u64 chan                             : 12;
		u64 errlev                           : 4;
		u64 errcode                          : 8;
		u64 l2m                              : 1;
		u64 l2b                              : 1;
		u64 l3m                              : 1;
		u64 l3b                              : 1;
		u64 la                               : 12;
		u64 lb                               : 12;
		u64 lc                               : 12;
		u64 ld                               : 12;
		u64 le                               : 12;
		u64 lf                               : 12;
		u64 lg                               : 12;
		u64 lh                               : 12;
		u64 reserved_124_127                 : 4;
	} s;
	/* struct npc_parse_kex_s_s cn; */
};

/**
 * Structure npc_result_s
 *
 * NPC Result Structure This structure contains a packet's parse and flow
 * identification information.
 */
union npc_result_s {
	u64 u[6];
	struct npc_result_s_s {
		u64 intf                             : 2;
		u64 pkind                            : 6;
		u64 chan                             : 12;
		u64 errlev                           : 4;
		u64 errcode                          : 8;
		u64 l2m                              : 1;
		u64 l2b                              : 1;
		u64 l3m                              : 1;
		u64 l3b                              : 1;
		u64 eoh_ptr                          : 8;
		u64 reserved_44_63                   : 20;
		u64 action                           : 64;
		u64 vtag_action                      : 64;
		u64 la                               : 20;
		u64 lb                               : 20;
		u64 lc                               : 20;
		u64 reserved_252_255                 : 4;
		u64 ld                               : 20;
		u64 le                               : 20;
		u64 lf                               : 20;
		u64 reserved_316_319                 : 4;
		u64 lg                               : 20;
		u64 lh                               : 20;
		u64 reserved_360_383                 : 24;
	} s;
	/* struct npc_result_s_s cn; */
};

/**
 * Register (RVU_PF_BAR0) npc_af_active_pc
 *
 * NPC Interrupt-Timer Configuration Register
 */
union npc_af_active_pc {
	u64 u;
	struct npc_af_active_pc_s {
		u64 active_pc                        : 64;
	} s;
	/* struct npc_af_active_pc_s cn; */
};

static inline u64 NPC_AF_ACTIVE_PC(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_ACTIVE_PC(void)
{
	return 0x10;
}

/**
 * Register (RVU_PF_BAR0) npc_af_blk_rst
 *
 * NPC AF Block Reset Register
 */
union npc_af_blk_rst {
	u64 u;
	struct npc_af_blk_rst_s {
		u64 rst                              : 1;
		u64 reserved_1_62                    : 62;
		u64 busy                             : 1;
	} s;
	/* struct npc_af_blk_rst_s cn; */
};

static inline u64 NPC_AF_BLK_RST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_BLK_RST(void)
{
	return 0x40;
}

/**
 * Register (RVU_PF_BAR0) npc_af_cfg
 *
 * NPC AF General Configuration Register
 */
union npc_af_cfg {
	u64 u;
	struct npc_af_cfg_s {
		u64 reserved_0_1                     : 2;
		u64 cclk_force                       : 1;
		u64 force_intf_clk_en                : 1;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct npc_af_cfg_s cn; */
};

static inline u64 NPC_AF_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_CFG(void)
{
	return 0;
}

/**
 * Register (RVU_PF_BAR0) npc_af_const
 *
 * NPC AF Constants Register This register contains constants for
 * software discovery.
 */
union npc_af_const {
	u64 u;
	struct npc_af_const_s {
		u64 intfs                            : 4;
		u64 lids                             : 4;
		u64 kpus                             : 5;
		u64 reserved_13_15                   : 3;
		u64 mcam_bank_width                  : 10;
		u64 reserved_26_27                   : 2;
		u64 mcam_bank_depth                  : 16;
		u64 mcam_banks                       : 4;
		u64 match_stats                      : 16;
	} s;
	/* struct npc_af_const_s cn; */
};

static inline u64 NPC_AF_CONST(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_CONST(void)
{
	return 0x20;
}

/**
 * Register (RVU_PF_BAR0) npc_af_const1
 *
 * NPC AF Constants 1 Register This register contains constants for
 * software discovery.
 */
union npc_af_const1 {
	u64 u;
	struct npc_af_const1_s {
		u64 kpu_entries                      : 12;
		u64 pkinds                           : 8;
		u64 cpi_size                         : 16;
		u64 reserved_36_63                   : 28;
	} s;
	/* struct npc_af_const1_s cn; */
};

static inline u64 NPC_AF_CONST1(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_CONST1(void)
{
	return 0x30;
}

/**
 * Register (RVU_PF_BAR0) npc_af_cpi#_cfg
 *
 * NPC AF Channel Parse Index Table Registers
 */
union npc_af_cpix_cfg {
	u64 u;
	struct npc_af_cpix_cfg_s {
		u64 padd                             : 4;
		u64 reserved_4_63                    : 60;
	} s;
	/* struct npc_af_cpix_cfg_s cn; */
};

static inline u64 NPC_AF_CPIX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_CPIX_CFG(u64 a)
{
	return 0x200000 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_dbg_ctl
 *
 * NPC AF Debug Control Register This register controls the capture of
 * debug information in NPC_AF_KPU()_DBG, NPC_AF_MCAM_DBG,
 * NPC_AF_DBG_DATA() and NPC_AF_DBG_RESULT().
 */
union npc_af_dbg_ctl {
	u64 u;
	struct npc_af_dbg_ctl_s {
		u64 continuous                       : 1;
		u64 lkup_dbg                         : 1;
		u64 intf_dbg                         : 4;
		u64 reserved_6_63                    : 58;
	} s;
	/* struct npc_af_dbg_ctl_s cn; */
};

static inline u64 NPC_AF_DBG_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_DBG_CTL(void)
{
	return 0x3000000;
}

/**
 * Register (RVU_PF_BAR0) npc_af_dbg_data#
 *
 * NPC AF Debug Data Registers These registers contain the packet header
 * data of the last packet/lookup whose debug information is captured by
 * NPC_AF_DBG_CTL[INTF_DBG,LKUP_DBG].
 */
union npc_af_dbg_datax {
	u64 u;
	struct npc_af_dbg_datax_s {
		u64 data                             : 64;
	} s;
	/* struct npc_af_dbg_datax_s cn; */
};

static inline u64 NPC_AF_DBG_DATAX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_DBG_DATAX(u64 a)
{
	return 0x3001400 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_dbg_result#
 *
 * NPC AF Debug Result Registers These registers contain the result data
 * of the last packet/lookup whose debug information is captured by
 * NPC_AF_DBG_CTL[INTF_DBG,LKUP_DBG].  Internal: FIXME - add note about
 * coherency of data in continuous packet capture mode.
 */
union npc_af_dbg_resultx {
	u64 u;
	struct npc_af_dbg_resultx_s {
		u64 data                             : 64;
	} s;
	/* struct npc_af_dbg_resultx_s cn; */
};

static inline u64 NPC_AF_DBG_RESULTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_DBG_RESULTX(u64 a)
{
	return 0x3001800 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_dbg_status
 *
 * NPC AF Debug Status Register
 */
union npc_af_dbg_status {
	u64 u;
	struct npc_af_dbg_status_s {
		u64 done                             : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npc_af_dbg_status_s cn; */
};

static inline u64 NPC_AF_DBG_STATUS(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_DBG_STATUS(void)
{
	return 0x3000010;
}

/**
 * Register (RVU_PF_BAR0) npc_af_dv_fc_scratch
 *
 * INTERNAL: NPC AF Scratch Register  Internal: This register is for
 * internal DV purpose.
 */
union npc_af_dv_fc_scratch {
	u64 u;
	struct npc_af_dv_fc_scratch_s {
		u64 it                               : 64;
	} s;
	/* struct npc_af_dv_fc_scratch_s cn; */
};

static inline u64 NPC_AF_DV_FC_SCRATCH(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_DV_FC_SCRATCH(void)
{
	return 0x60;
}

/**
 * Register (RVU_PF_BAR0) npc_af_eco0
 *
 * INTERNAL: ECO 0 Register
 */
union npc_af_eco0 {
	u64 u;
	struct npc_af_eco0_s {
		u64 eco_rw                           : 32;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct npc_af_eco0_s cn; */
};

static inline u64 NPC_AF_ECO0(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_ECO0(void)
{
	return 0x200;
}

/**
 * Register (RVU_PF_BAR0) npc_af_ikpu_err_ctl
 *
 * NPC AF Initial KPU Error Control Registers Similar to
 * NPC_AF_KPU()_ERR_CTL, but specifies values captured in
 * NPC_RESULT_S[ERRLEV,ERRCODE] for errors detected by the PKIND-based
 * initial actions from NPC_AF_PKIND()_ACTION0 and
 * NPC_AF_PKIND()_ACTION1. [DP_OFFSET_ERRCODE] from this register is
 * never used.
 */
union npc_af_ikpu_err_ctl {
	u64 u;
	struct npc_af_ikpu_err_ctl_s {
		u64 errlev                           : 4;
		u64 dp_offset_errcode                : 8;
		u64 ptr_advance_errcode              : 8;
		u64 var_len_offset_errcode           : 8;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct npc_af_ikpu_err_ctl_s cn; */
};

static inline u64 NPC_AF_IKPU_ERR_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_IKPU_ERR_CTL(void)
{
	return 0x3000080;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_kex_cfg
 *
 * NPC AF Interface Key Extract Configuration Registers
 */
union npc_af_intfx_kex_cfg {
	u64 u;
	struct npc_af_intfx_kex_cfg_s {
		u64 parse_nibble_ena                 : 31;
		u64 reserved_31                      : 1;
		u64 keyw                             : 3;
		u64 reserved_35_63                   : 29;
	} s;
	/* struct npc_af_intfx_kex_cfg_s cn; */
};

static inline u64 NPC_AF_INTFX_KEX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_KEX_CFG(u64 a)
{
	return 0x1010 + 0x100 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_ldata#_flags#_cfg
 *
 * NPC AF Interface Layer Data Flags Configuration Registers These
 * registers control the extraction of layer data (LDATA) into the MCAM
 * search key for each interface based on the FLAGS\<3:0\> bits of two
 * layers selected by NPC_AF_KEX_LDATA()_FLAGS_CFG.
 */
union npc_af_intfx_ldatax_flagsx_cfg {
	u64 u;
	struct npc_af_intfx_ldatax_flagsx_cfg_s {
		u64 key_offset                       : 6;
		u64 reserved_6                       : 1;
		u64 ena                              : 1;
		u64 hdr_offset                       : 8;
		u64 bytesm1                          : 4;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npc_af_intfx_ldatax_flagsx_cfg_s cn; */
};

static inline u64 NPC_AF_INTFX_LDATAX_FLAGSX_CFG(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_LDATAX_FLAGSX_CFG(u64 a, u64 b, u64 c)
{
	return 0x980000 + 0x10000 * a + 0x1000 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_lid#_lt#_ld#_cfg
 *
 * NPC AF Interface Layer Data Extract Configuration Registers These
 * registers control the extraction of layer data (LDATA) into the MCAM
 * search key for each interface. Up to two LDATA fields can be extracted
 * per layer (LID(0..7) indexed by NPC_LID_E), with up to 16 bytes per
 * LDATA field. For each layer, the corresponding NPC_LAYER_INFO_S[LTYPE]
 * value in NPC_RESULT_S is used as the LTYPE(0..15) index and select the
 * associated LDATA(0..1) registers.  NPC_LAYER_INFO_S[LTYPE]=0x0 means
 * the corresponding layer not parsed (invalid), so software should keep
 * NPC_AF_INTF()_LID()_LT(0)_LD()_CFG[ENA] clear to disable extraction
 * when LTYPE is zero.
 */
union npc_af_intfx_lidx_ltx_ldx_cfg {
	u64 u;
	struct npc_af_intfx_lidx_ltx_ldx_cfg_s {
		u64 key_offset                       : 6;
		u64 flags_ena                        : 1;
		u64 ena                              : 1;
		u64 hdr_offset                       : 8;
		u64 bytesm1                          : 4;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npc_af_intfx_lidx_ltx_ldx_cfg_s cn; */
};

static inline u64 NPC_AF_INTFX_LIDX_LTX_LDX_CFG(u64 a, u64 b, u64 c, u64 d)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_LIDX_LTX_LDX_CFG(u64 a, u64 b, u64 c, u64 d)
{
	return 0x900000 + 0x10000 * a + 0x1000 * b + 0x20 * c + 8 * d;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_miss_act
 *
 * NPC AF Interface MCAM Miss Action Data Registers When a combination of
 * NPC_AF_MCAME()_BANK()_CAM()_* and NPC_AF_MCAME()_BANK()_CFG[ENA]
 * yields an MCAM miss for a packet, this register specifies the packet's
 * match action captured in NPC_RESULT_S[ACTION].
 */
union npc_af_intfx_miss_act {
	u64 u;
	struct npc_af_intfx_miss_act_s {
		u64 action                           : 64;
	} s;
	/* struct npc_af_intfx_miss_act_s cn; */
};

static inline u64 NPC_AF_INTFX_MISS_ACT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_MISS_ACT(u64 a)
{
	return 0x1a00000 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_miss_stat_act
 *
 * NPC AF Interface MCAM Miss Stat Action Data Registers Used to
 * optionally increment a NPC_AF_MATCH_STAT() counter when a packet
 * misses an MCAM entry.
 */
union npc_af_intfx_miss_stat_act {
	u64 u;
	struct npc_af_intfx_miss_stat_act_s {
		u64 stat_sel                         : 9;
		u64 ena                              : 1;
		u64 reserved_10_63                   : 54;
	} s;
	/* struct npc_af_intfx_miss_stat_act_s cn; */
};

static inline u64 NPC_AF_INTFX_MISS_STAT_ACT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_MISS_STAT_ACT(u64 a)
{
	return 0x1880040 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_miss_tag_act
 *
 * NPC AF Interface MCAM Miss VTag Action Data Registers When a
 * combination of NPC_AF_MCAME()_BANK()_CAM()_* and
 * NPC_AF_MCAME()_BANK()_CFG[ENA] yields an MCAM miss for a packet, this
 * register specifies the packet's match Vtag action captured in
 * NPC_RESULT_S[VTAG_ACTION].
 */
union npc_af_intfx_miss_tag_act {
	u64 u;
	struct npc_af_intfx_miss_tag_act_s {
		u64 vtag_action                      : 64;
	} s;
	/* struct npc_af_intfx_miss_tag_act_s cn; */
};

static inline u64 NPC_AF_INTFX_MISS_TAG_ACT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_MISS_TAG_ACT(u64 a)
{
	return 0x1b00008 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_intf#_stat
 *
 * NPC AF Interface Statistics Registers Statistics per interface. Index
 * enumerated by NPC_INTF_E.
 */
union npc_af_intfx_stat {
	u64 u;
	struct npc_af_intfx_stat_s {
		u64 count                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct npc_af_intfx_stat_s cn; */
};

static inline u64 NPC_AF_INTFX_STAT(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_INTFX_STAT(u64 a)
{
	return 0x2000800 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kcam_scrub_ctl
 *
 * NPC AF KCAM Scrub Control Register
 */
union npc_af_kcam_scrub_ctl {
	u64 u;
	struct npc_af_kcam_scrub_ctl_s {
		u64 ena                              : 1;
		u64 reserved_1_7                     : 7;
		u64 lp_dis                           : 1;
		u64 reserved_9_15                    : 7;
		u64 toth                             : 4;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npc_af_kcam_scrub_ctl_s cn; */
};

static inline u64 NPC_AF_KCAM_SCRUB_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KCAM_SCRUB_CTL(void)
{
	return 0xb0;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kex_ldata#_flags_cfg
 *
 * NPC AF Key Extract Layer Data Flags Configuration Register
 */
union npc_af_kex_ldatax_flags_cfg {
	u64 u;
	struct npc_af_kex_ldatax_flags_cfg_s {
		u64 lid                              : 3;
		u64 reserved_3_63                    : 61;
	} s;
	/* struct npc_af_kex_ldatax_flags_cfg_s cn; */
};

static inline u64 NPC_AF_KEX_LDATAX_FLAGS_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KEX_LDATAX_FLAGS_CFG(u64 a)
{
	return 0x800 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu#_cfg
 *
 * NPC AF KPU Configuration Registers
 */
union npc_af_kpux_cfg {
	u64 u;
	struct npc_af_kpux_cfg_s {
		u64 ena                              : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npc_af_kpux_cfg_s cn; */
};

static inline u64 NPC_AF_KPUX_CFG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPUX_CFG(u64 a)
{
	return 0x500 + 8 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu#_dbg
 *
 * NPC AF KPU Debug Registers This register contains information for the
 * last packet/lookup for which debug is enabled by
 * NPC_AF_DBG_CTL[INTF_DBG,LKUP_DBG]. The register contents are undefined
 * when debug information is captured for a software key lookup
 * (NPC_AF_LKUP_CTL[OP] = NPC_LKUPOP_E::KEY).
 */
union npc_af_kpux_dbg {
	u64 u;
	struct npc_af_kpux_dbg_s {
		u64 hit_entry                        : 8;
		u64 byp                              : 1;
		u64 reserved_9_63                    : 55;
	} s;
	/* struct npc_af_kpux_dbg_s cn; */
};

static inline u64 NPC_AF_KPUX_DBG(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPUX_DBG(u64 a)
{
	return 0x3000020 + 0x100 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu#_entry#_action0
 *
 * NPC AF KPU Entry Action Data 0 Registers When a KPU's search data
 * matches a KPU CAM entry in NPC_AF_KPU()_ENTRY()_CAM(), the
 * corresponding entry action in NPC_AF_KPU()_ENTRY()_ACTION0 and
 * NPC_AF_KPU()_ENTRY()_ACTION1 specifies the next state and operations
 * to perform before exiting the KPU.
 */
union npc_af_kpux_entryx_action0 {
	u64 u;
	struct npc_af_kpux_entryx_action0_s {
		u64 var_len_shift                    : 3;
		u64 var_len_right                    : 1;
		u64 var_len_mask                     : 8;
		u64 var_len_offset                   : 8;
		u64 ptr_advance                      : 8;
		u64 capture_flags                    : 8;
		u64 capture_ltype                    : 4;
		u64 capture_lid                      : 3;
		u64 reserved_43                      : 1;
		u64 next_state                       : 8;
		u64 parse_done                       : 1;
		u64 capture_ena                      : 1;
		u64 byp_count                        : 3;
		u64 reserved_57_63                   : 7;
	} s;
	/* struct npc_af_kpux_entryx_action0_s cn; */
};

static inline u64 NPC_AF_KPUX_ENTRYX_ACTION0(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPUX_ENTRYX_ACTION0(u64 a, u64 b)
{
	return 0x100020 + 0x4000 * a + 0x40 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu#_entry#_action1
 *
 * NPC AF KPU Entry Action Data 0 Registers See
 * NPC_AF_KPU()_ENTRY()_ACTION0.
 */
union npc_af_kpux_entryx_action1 {
	u64 u;
	struct npc_af_kpux_entryx_action1_s {
		u64 dp0_offset                       : 8;
		u64 dp1_offset                       : 8;
		u64 dp2_offset                       : 8;
		u64 errcode                          : 8;
		u64 errlev                           : 4;
		u64 reserved_36_63                   : 28;
	} s;
	/* struct npc_af_kpux_entryx_action1_s cn; */
};

static inline u64 NPC_AF_KPUX_ENTRYX_ACTION1(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPUX_ENTRYX_ACTION1(u64 a, u64 b)
{
	return 0x100028 + 0x4000 * a + 0x40 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu#_entry#_cam#
 *
 * NPC AF KPU Entry CAM Registers KPU comparison ternary data. The field
 * values in NPC_AF_KPU()_ENTRY()_CAM() are ternary, where  each data bit
 * of the search key matches as follows: _ [CAM(1)]\<n\>=0,
 * [CAM(0)]\<n\>=0: Always match; search key data\<n\> don't care. _
 * [CAM(1)]\<n\>=0, [CAM(0)]\<n\>=1: Match when search key data\<n\> ==
 * 0. _ [CAM(1)]\<n\>=1, [CAM(0)]\<n\>=0: Match when search key data\<n\>
 * == 1. _ [CAM(1)]\<n\>=1, [CAM(0)]\<n\>=1: Reserved.  The reserved
 * combination is not allowed. Hardware suppresses any write to CAM(0) or
 * CAM(1) that would result in the reserved combination for any CAM bit.
 * The reset value for all non-reserved fields is all zeros for CAM(1)
 * and all ones for CAM(0), matching a search key of all zeros.  Software
 * must program a default entry for each KPU, e.g. by programming each
 * KPU's last entry {b} (NPC_AF_KPU()_ENTRY({b})_CAM()) to always match
 * all bits.
 */
union npc_af_kpux_entryx_camx {
	u64 u;
	struct npc_af_kpux_entryx_camx_s {
		u64 dp0_data                         : 16;
		u64 dp1_data                         : 16;
		u64 dp2_data                         : 16;
		u64 state                            : 8;
		u64 reserved_56_63                   : 8;
	} s;
	/* struct npc_af_kpux_entryx_camx_s cn; */
};

static inline u64 NPC_AF_KPUX_ENTRYX_CAMX(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPUX_ENTRYX_CAMX(u64 a, u64 b, u64 c)
{
	return 0x100000 + 0x4000 * a + 0x40 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu#_entry_dis#
 *
 * NPC AF KPU Entry Disable Registers See NPC_AF_KPU()_ENTRY()_ACTION0.
 */
union npc_af_kpux_entry_disx {
	u64 u;
	struct npc_af_kpux_entry_disx_s {
		u64 dis                              : 64;
	} s;
	/* struct npc_af_kpux_entry_disx_s cn; */
};

static inline u64 NPC_AF_KPUX_ENTRY_DISX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPUX_ENTRY_DISX(u64 a, u64 b)
{
	return 0x180000 + 0x40 * a + 8 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu#_err_ctl
 *
 * NPC AF KPU Error Control Registers This register specifies values
 * captured in NPC_RESULT_S[ERRLEV,ERRCODE] when errors are detected by a
 * KPU.
 */
union npc_af_kpux_err_ctl {
	u64 u;
	struct npc_af_kpux_err_ctl_s {
		u64 errlev                           : 4;
		u64 dp_offset_errcode                : 8;
		u64 ptr_advance_errcode              : 8;
		u64 var_len_offset_errcode           : 8;
		u64 reserved_28_63                   : 36;
	} s;
	/* struct npc_af_kpux_err_ctl_s cn; */
};

static inline u64 NPC_AF_KPUX_ERR_CTL(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPUX_ERR_CTL(u64 a)
{
	return 0x30000a0 + 0x100 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_kpu_diag
 *
 * INTERNAL : NPC AF Debug Result Registers
 */
union npc_af_kpu_diag {
	u64 u;
	struct npc_af_kpu_diag_s {
		u64 skip_dis                         : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npc_af_kpu_diag_s cn; */
};

static inline u64 NPC_AF_KPU_DIAG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_KPU_DIAG(void)
{
	return 0x3002000;
}

/**
 * Register (RVU_PF_BAR0) npc_af_lkup_ctl
 *
 * NPC AF Software Lookup Control Registers
 */
union npc_af_lkup_ctl {
	u64 u;
	struct npc_af_lkup_ctl_s {
		u64 intf                             : 2;
		u64 pkind                            : 6;
		u64 chan                             : 12;
		u64 hdr_sizem1                       : 8;
		u64 op                               : 3;
		u64 exec                             : 1;
		u64 reserved_32_63                   : 32;
	} s;
	/* struct npc_af_lkup_ctl_s cn; */
};

static inline u64 NPC_AF_LKUP_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_LKUP_CTL(void)
{
	return 0x2000000;
}

/**
 * Register (RVU_PF_BAR0) npc_af_lkup_data#
 *
 * NPC AF Software Lookup Data Registers
 */
union npc_af_lkup_datax {
	u64 u;
	struct npc_af_lkup_datax_s {
		u64 data                             : 64;
	} s;
	/* struct npc_af_lkup_datax_s cn; */
};

static inline u64 NPC_AF_LKUP_DATAX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_LKUP_DATAX(u64 a)
{
	return 0x2000200 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_lkup_result#
 *
 * NPC AF Software Lookup Result Registers
 */
union npc_af_lkup_resultx {
	u64 u;
	struct npc_af_lkup_resultx_s {
		u64 data                             : 64;
	} s;
	/* struct npc_af_lkup_resultx_s cn; */
};

static inline u64 NPC_AF_LKUP_RESULTX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_LKUP_RESULTX(u64 a)
{
	return 0x2000400 + 0x10 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_match_stat#
 *
 * NPC AF Match Statistics Registers
 */
union npc_af_match_statx {
	u64 u;
	struct npc_af_match_statx_s {
		u64 count                            : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct npc_af_match_statx_s cn; */
};

static inline u64 NPC_AF_MATCH_STATX(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MATCH_STATX(u64 a)
{
	return 0x1880008 + 0x100 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcam_bank#_hit#
 *
 * NPC AF MCAM Bank Hit Registers
 */
union npc_af_mcam_bankx_hitx {
	u64 u;
	struct npc_af_mcam_bankx_hitx_s {
		u64 hit                              : 64;
	} s;
	/* struct npc_af_mcam_bankx_hitx_s cn; */
};

static inline u64 NPC_AF_MCAM_BANKX_HITX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAM_BANKX_HITX(u64 a, u64 b)
{
	return 0x1c80000 + 0x100 * a + 0x10 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcam_dbg
 *
 * NPC AF MCAM Debug Register This register contains information for the
 * last packet/lookup for which debug is enabled by
 * NPC_AF_DBG_CTL[INTF_DBG,LKUP_DBG].
 */
union npc_af_mcam_dbg {
	u64 u;
	struct npc_af_mcam_dbg_s {
		u64 hit_entry                        : 10;
		u64 reserved_10_11                   : 2;
		u64 hit_bank                         : 2;
		u64 reserved_14_15                   : 2;
		u64 miss                             : 1;
		u64 reserved_17_63                   : 47;
	} s;
	/* struct npc_af_mcam_dbg_s cn; */
};

static inline u64 NPC_AF_MCAM_DBG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAM_DBG(void)
{
	return 0x3001000;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcam_scrub_ctl
 *
 * NPC AF MCAM Scrub Control Register
 */
union npc_af_mcam_scrub_ctl {
	u64 u;
	struct npc_af_mcam_scrub_ctl_s {
		u64 ena                              : 1;
		u64 reserved_1_7                     : 7;
		u64 lp_dis                           : 1;
		u64 reserved_9_15                    : 7;
		u64 toth                             : 4;
		u64 reserved_20_63                   : 44;
	} s;
	/* struct npc_af_mcam_scrub_ctl_s cn; */
};

static inline u64 NPC_AF_MCAM_SCRUB_CTL(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAM_SCRUB_CTL(void)
{
	return 0xa0;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_action
 *
 * NPC AF MCAM Entry Bank Action Data Registers Specifies a packet's
 * match action captured in NPC_RESULT_S[ACTION].  When an interface is
 * configured to use the NPC_MCAM_KEY_X2_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X2), *
 * NPC_AF_MCAME()_BANK(0)_ACTION/_TAG_ACT/_STAT_ACT are used if the
 * search key matches NPC_AF_MCAME()_BANK(0..1)_CAM()_W*. *
 * NPC_AF_MCAME()_BANK(2)_ACTION/_TAG_ACT/_STAT_ACT are used if the
 * search key matches NPC_AF_MCAME()_BANK(2..3)_CAM()_W*. *
 * NPC_AF_MCAME()_BANK(1,3)_ACTION/_TAG_ACT/_STAT_ACT are not used.  When
 * an interface is configured to use the NPC_MCAM_KEY_X4_S search key
 * format (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4): *
 * NPC_AF_MCAME()_BANK(0)_ACTION/_TAG_ACT/_STAT_ACT are used if the
 * search key matches NPC_AF_MCAME()_BANK(0..3)_CAM()_W*. *
 * NPC_AF_MCAME()_BANK(1..3)_ACTION/_TAG_ACT/_STAT_ACT are not used.
 */
union npc_af_mcamex_bankx_action {
	u64 u;
	struct npc_af_mcamex_bankx_action_s {
		u64 action                           : 64;
	} s;
	/* struct npc_af_mcamex_bankx_action_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_ACTION(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_ACTION(u64 a, u64 b)
{
	return 0x1900000 + 0x100 * a + 0x10 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_intf
 *
 * NPC AF MCAM Entry Bank CAM Data Interface Registers MCAM comparison
 * ternary data interface word. The field values in
 * NPC_AF_MCAME()_BANK()_CAM()_INTF, NPC_AF_MCAME()_BANK()_CAM()_W0 and
 * NPC_AF_MCAME()_BANK()_CAM()_W1 are ternary, where  each data bit of
 * the search key matches as follows: _ [CAM(1)]\<n\>=0, [CAM(0)]\<n\>=0:
 * Always match; search key data\<n\> don't care. _ [CAM(1)]\<n\>=0,
 * [CAM(0)]\<n\>=1: Match when search key data\<n\> == 0. _
 * [CAM(1)]\<n\>=1, [CAM(0)]\<n\>=0: Match when search key data\<n\> ==
 * 1. _ [CAM(1)]\<n\>=1, [CAM(0)]\<n\>=1: Reserved.  The reserved
 * combination is not allowed. Hardware suppresses any write to CAM(0) or
 * CAM(1) that would result in the reserved combination for any CAM bit.
 * The reset value for all non-reserved fields in
 * NPC_AF_MCAME()_BANK()_CAM()_INTF, NPC_AF_MCAME()_BANK()_CAM()_W0 and
 * NPC_AF_MCAME()_BANK()_CAM()_W1 is all zeros for CAM(1) and all ones
 * for CAM(0), matching a search key of all zeros.  When an interface is
 * configured to use the NPC_MCAM_KEY_X1_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X1), the four banks of
 * every MCAM entry are used as individual entries, each of which is
 * independently compared with the search key as follows: _
 * NPC_AF_MCAME()_BANK()_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X1_S[INTF]. _ NPC_AF_MCAME()_BANK()_CAM()_W0[MD]
 * corresponds to NPC_MCAM_KEY_X1_S[KW0]. _
 * NPC_AF_MCAME()_BANK()_CAM()_W1[MD] corresponds to
 * NPC_MCAM_KEY_X1_S[KW1].  When an interface is configured to use the
 * NPC_MCAM_KEY_X2_S search key format (NPC_AF_INTF()_KEX_CFG[KEYW] =
 * NPC_MCAMKEYW_E::X2), banks 0-1 of every MCAM entry are used as one
 * double-wide entry, banks 2-3 as a second double-wide entry, and each
 * double-wide entry is independently compared with the search key as
 * follows: _ NPC_AF_MCAME()_BANK(0,2)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X2_S[INTF]. _ NPC_AF_MCAME()_BANK(0,2)_CAM()_W0[MD]
 * corresponds to NPC_MCAM_KEY_X2_S[KW0]. _
 * NPC_AF_MCAME()_BANK(0,2)_CAM()_W1[MD] corresponds to
 * NPC_MCAM_KEY_X2_S[KW1]\<47:0\>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X2_S[INTF]. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W0[MD]\<15:0\> corresponds to
 * NPC_MCAM_KEY_X2_S[KW1]\<63:48\>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W0[MD]\<63:16\> corresponds to
 * NPC_MCAM_KEY_X2_S[KW2]\<47:0\>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W1[MD]\<15:0\> corresponds to
 * NPC_MCAM_KEY_X2_S[KW2]\<63:48\>. _
 * NPC_AF_MCAME()_BANK(1,3)_CAM()_W1[MD]\<47:16\> corresponds to
 * NPC_MCAM_KEY_X2_S[KW3]\<31:0\>.  When an interface is configured to
 * use the NPC_MCAM_KEY_X4_S search key format
 * (NPC_AF_INTF()_KEX_CFG[KEYW] = NPC_MCAMKEYW_E::X4), the four banks of
 * every MCAM entry are used as a single quad-wide entry that is compared
 * with the search key as follows: _
 * NPC_AF_MCAME()_BANK(0)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _ NPC_AF_MCAME()_BANK(0)_CAM()_W0[MD]
 * corresponds to NPC_MCAM_KEY_X4_S[KW0]. _
 * NPC_AF_MCAME()_BANK(0)_CAM()_W1[MD] corresponds to
 * NPC_MCAM_KEY_X4_S[KW1]\<47:0\>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _ NPC_AF_MCAME()_BANK(1)_CAM()_W0[MD]\<15:0\>
 * corresponds to NPC_MCAM_KEY_X4_S[KW1]\<63:48\>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W0[MD]\<63:16\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW2]\<47:0\>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W1[MD]\<15:0\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW2]\<63:48\>. _
 * NPC_AF_MCAME()_BANK(1)_CAM()_W1[MD]\<47:16\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW3]\<31:0\>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _ NPC_AF_MCAME()_BANK(2)_CAM()_W0[MD]\<31:0\>
 * corresponds to NPC_MCAM_KEY_X4_S[KW3]\<63:32\>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W0[MD]\<63:32\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW4]\<31:0\>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W1[MD]\<31:0\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW4]\<63:32\>. _
 * NPC_AF_MCAME()_BANK(2)_CAM()_W1[MD]\<47:32\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW5]\<15:0\>. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_INTF[INTF] corresponds to
 * NPC_MCAM_KEY_X4_S[INTF]. _ NPC_AF_MCAME()_BANK(3)_CAM()_W0[MD]\<47:0\>
 * corresponds to NPC_MCAM_KEY_X4_S[KW5]\<63:16\>. _
 * NPC_AF_MCAME()_BANK(3)_CAM()_W0[MD]\<63:48\> corresponds to
 * NPC_MCAM_KEY_X4_S[KW6]\<15:0\>. _ NPC_AF_MCAME()_BANK(3)_CAM()_W1[MD]
 * corresponds to NPC_MCAM_KEY_X4_S[KW6]\<63:16\>.  Note that for the X2
 * and X4 formats, a wide entry will not match unless the INTF fields
 * from the associated two or four banks match the INTF value from the
 * search key.  For the X1 and X2 formats, a match in a lower-numbered
 * bank takes priority over a match in any higher numbered banks. Within
 * each bank, the lowest numbered matching entry takes priority over any
 * higher numbered entry.
 */
union npc_af_mcamex_bankx_camx_intf {
	u64 u;
	struct npc_af_mcamex_bankx_camx_intf_s {
		u64 intf                             : 2;
		u64 reserved_2_63                    : 62;
	} s;
	/* struct npc_af_mcamex_bankx_camx_intf_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_INTF(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_INTF(u64 a, u64 b, u64 c)
{
	return 0x1000000 + 0x400 * a + 0x40 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_w0
 *
 * NPC AF MCAM Entry Bank CAM Data Word 0 Registers MCAM comparison
 * ternary data word 0. See NPC_AF_MCAME()_BANK()_CAM()_INTF.
 */
union npc_af_mcamex_bankx_camx_w0 {
	u64 u;
	struct npc_af_mcamex_bankx_camx_w0_s {
		u64 md                               : 64;
	} s;
	/* struct npc_af_mcamex_bankx_camx_w0_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W0(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W0(u64 a, u64 b, u64 c)
{
	return 0x1000010 + 0x400 * a + 0x40 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cam#_w1
 *
 * NPC AF MCAM Entry Bank Data Word 1 Registers MCAM comparison ternary
 * data word 1. See NPC_AF_MCAME()_BANK()_CAM()_INTF.
 */
union npc_af_mcamex_bankx_camx_w1 {
	u64 u;
	struct npc_af_mcamex_bankx_camx_w1_s {
		u64 md                               : 48;
		u64 reserved_48_63                   : 16;
	} s;
	/* struct npc_af_mcamex_bankx_camx_w1_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W1(u64 a, u64 b, u64 c)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CAMX_W1(u64 a, u64 b, u64 c)
{
	return 0x1000020 + 0x400 * a + 0x40 * b + 8 * c;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_cfg
 *
 * NPC AF MCAM Entry Bank Configuration Registers
 */
union npc_af_mcamex_bankx_cfg {
	u64 u;
	struct npc_af_mcamex_bankx_cfg_s {
		u64 ena                              : 1;
		u64 reserved_1_63                    : 63;
	} s;
	/* struct npc_af_mcamex_bankx_cfg_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_CFG(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_CFG(u64 a, u64 b)
{
	return 0x1800000 + 0x100 * a + 0x10 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_stat_act
 *
 * NPC AF MCAM Entry Bank Statistics Action Registers Used to optionally
 * increment a NPC_AF_MATCH_STAT() counter when a packet matches an MCAM
 * entry. See also NPC_AF_MCAME()_BANK()_ACTION.
 */
union npc_af_mcamex_bankx_stat_act {
	u64 u;
	struct npc_af_mcamex_bankx_stat_act_s {
		u64 stat_sel                         : 9;
		u64 ena                              : 1;
		u64 reserved_10_63                   : 54;
	} s;
	/* struct npc_af_mcamex_bankx_stat_act_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_STAT_ACT(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_STAT_ACT(u64 a, u64 b)
{
	return 0x1880000 + 0x100 * a + 0x10 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_mcame#_bank#_tag_act
 *
 * NPC AF MCAM Entry Bank VTag Action Data Registers Specifies a packet's
 * match Vtag action captured in NPC_RESULT_S[VTAG_ACTION]. See also
 * NPC_AF_MCAME()_BANK()_ACTION.
 */
union npc_af_mcamex_bankx_tag_act {
	u64 u;
	struct npc_af_mcamex_bankx_tag_act_s {
		u64 vtag_action                      : 64;
	} s;
	/* struct npc_af_mcamex_bankx_tag_act_s cn; */
};

static inline u64 NPC_AF_MCAMEX_BANKX_TAG_ACT(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_MCAMEX_BANKX_TAG_ACT(u64 a, u64 b)
{
	return 0x1900008 + 0x100 * a + 0x10 * b;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pck_cfg
 *
 * NPC AF Protocol Check Configuration Register
 */
union npc_af_pck_cfg {
	u64 u;
	struct npc_af_pck_cfg_s {
		u64 reserved_0                       : 1;
		u64 iip4_cksum                       : 1;
		u64 oip4_cksum                       : 1;
		u64 reserved_3                       : 1;
		u64 l3b                              : 1;
		u64 l3m                              : 1;
		u64 l2b                              : 1;
		u64 l2m                              : 1;
		u64 reserved_8_23                    : 16;
		u64 iip4_cksum_errcode               : 8;
		u64 oip4_cksum_errcode               : 8;
		u64 reserved_40_63                   : 24;
	} s;
	/* struct npc_af_pck_cfg_s cn; */
};

static inline u64 NPC_AF_PCK_CFG(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PCK_CFG(void)
{
	return 0x600;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pck_def_iip4
 *
 * NPC AF Protocol Check Inner IPv4 Definition Register Provides layer
 * information used by the protocol checker to identify an inner IPv4
 * header.
 */
union npc_af_pck_def_iip4 {
	u64 u;
	struct npc_af_pck_def_iip4_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct npc_af_pck_def_iip4_s cn; */
};

static inline u64 NPC_AF_PCK_DEF_IIP4(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PCK_DEF_IIP4(void)
{
	return 0x640;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pck_def_oip4
 *
 * NPC AF Protocol Check Outer IPv4 Definition Register Provides layer
 * information used by the protocol checker to identify an outer IPv4
 * header.
 */
union npc_af_pck_def_oip4 {
	u64 u;
	struct npc_af_pck_def_oip4_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct npc_af_pck_def_oip4_s cn; */
};

static inline u64 NPC_AF_PCK_DEF_OIP4(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PCK_DEF_OIP4(void)
{
	return 0x620;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pck_def_oip6
 *
 * NPC AF Protocol Check Outer IPv6 Definition Register Provides layer
 * information used by the protocol checker to identify an outer IPv6
 * header. [LID] must have the same value as NPC_AF_PCK_DEF_OIP4[LID].
 */
union npc_af_pck_def_oip6 {
	u64 u;
	struct npc_af_pck_def_oip6_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct npc_af_pck_def_oip6_s cn; */
};

static inline u64 NPC_AF_PCK_DEF_OIP6(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PCK_DEF_OIP6(void)
{
	return 0x630;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pck_def_ol2
 *
 * NPC AF Protocol Check Outer L2 Definition Register Provides layer
 * information used by the protocol checker to identify an outer L2
 * header.
 */
union npc_af_pck_def_ol2 {
	u64 u;
	struct npc_af_pck_def_ol2_s {
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_11_63                   : 53;
	} s;
	/* struct npc_af_pck_def_ol2_s cn; */
};

static inline u64 NPC_AF_PCK_DEF_OL2(void)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PCK_DEF_OL2(void)
{
	return 0x610;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pkind#_action0
 *
 * NPC AF Port Kind Action Data 0 Registers NPC_AF_PKIND()_ACTION0 and
 * NPC_AF_PKIND()_ACTION1 specify the initial parse state and operations
 * to perform before entering KPU 0.
 */
union npc_af_pkindx_action0 {
	u64 u;
	struct npc_af_pkindx_action0_s {
		u64 var_len_shift                    : 3;
		u64 var_len_right                    : 1;
		u64 var_len_mask                     : 8;
		u64 var_len_offset                   : 8;
		u64 ptr_advance                      : 8;
		u64 capture_flags                    : 8;
		u64 capture_ltype                    : 4;
		u64 capture_lid                      : 3;
		u64 reserved_43                      : 1;
		u64 next_state                       : 8;
		u64 parse_done                       : 1;
		u64 capture_ena                      : 1;
		u64 byp_count                        : 3;
		u64 reserved_57_63                   : 7;
	} s;
	/* struct npc_af_pkindx_action0_s cn; */
};

static inline u64 NPC_AF_PKINDX_ACTION0(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PKINDX_ACTION0(u64 a)
{
	return 0x80000 + 0x40 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pkind#_action1
 *
 * NPC AF Port Kind Action Data 1 Registers NPC_AF_PKIND()_ACTION0 and
 * NPC_AF_PKIND()_ACTION1 specify the initial parse state and operations
 * to perform before entering KPU 0.
 */
union npc_af_pkindx_action1 {
	u64 u;
	struct npc_af_pkindx_action1_s {
		u64 dp0_offset                       : 8;
		u64 dp1_offset                       : 8;
		u64 dp2_offset                       : 8;
		u64 errcode                          : 8;
		u64 errlev                           : 4;
		u64 reserved_36_63                   : 28;
	} s;
	/* struct npc_af_pkindx_action1_s cn; */
};

static inline u64 NPC_AF_PKINDX_ACTION1(u64 a)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PKINDX_ACTION1(u64 a)
{
	return 0x80008 + 0x40 * a;
}

/**
 * Register (RVU_PF_BAR0) npc_af_pkind#_cpi_def#
 *
 * NPC AF Port Kind Channel Parse Index Definition Registers These
 * registers specify the layer information and algorithm to compute a
 * packet's channel parse index (CPI), which provides a port to channel
 * adder for calculating NPC_RESULT_S[CHAN].  There are two CPI
 * definitions per port kind, allowing the CPI computation to use two
 * possible layer definitions in the parsed packet, e.g. DiffServ DSCP
 * from either IPv4 or IPv6 header.  CPI pseudocode: \<pre\> for (i = 0;
 * i \< 2; i++) {    cpi_def = NPC_AF_PKIND()_CPI_DEF(i);    LX = LA, LB,
 * ..., or LH as selected by cpi_def[LID];     if (cpi_def[ENA]        &&
 * ((cpi_def[LTYPE_MATCH] & cpi_def[LTYPE_MASK])             ==
 * (NPC_RESULT_S[LX[LTYPE]] & cpi_def[LTYPE_MASK]))        &&
 * ((cpi_def[FLAGS_MATCH] & cpi_def[FLAGS_MASK])             ==
 * (NPC_RESULT_S[LX[FLAGS]] & cpi_def[FLAGS_MASK])))    {       // Found
 * matching layer       nibble_offset = (2*NPC_RESULT_S[LX[LPTR]]) +
 * cpi_def[ADD_OFFSET];       add_byte = byte at nibble_offset from start
 * of packet;       cpi_add = (add_byte & cpi_def[ADD_MASK]) \>\>
 * cpi_def[ADD_SHIFT];       cpi = cpi_def[CPI_BASE] + cpi_add;
 * NPC_RESULT_S[CHAN] += NPC_AF_CPI(cpi)_CFG[PADD];       break;    } }
 * \</pre\>
 */
union npc_af_pkindx_cpi_defx {
	u64 u;
	struct npc_af_pkindx_cpi_defx_s {
		u64 cpi_base                         : 10;
		u64 reserved_10_11                   : 2;
		u64 add_shift                        : 3;
		u64 reserved_15                      : 1;
		u64 add_mask                         : 8;
		u64 add_offset                       : 8;
		u64 flags_mask                       : 8;
		u64 flags_match                      : 8;
		u64 ltype_mask                       : 4;
		u64 ltype_match                      : 4;
		u64 lid                              : 3;
		u64 reserved_59_62                   : 4;
		u64 ena                              : 1;
	} s;
	/* struct npc_af_pkindx_cpi_defx_s cn; */
};

static inline u64 NPC_AF_PKINDX_CPI_DEFX(u64 a, u64 b)
	__attribute__ ((pure, always_inline));
static inline u64 NPC_AF_PKINDX_CPI_DEFX(u64 a, u64 b)
{
	return 0x80020 + 0x40 * a + 8 * b;
}

#endif /* __CSRS_NPC_H__ */
