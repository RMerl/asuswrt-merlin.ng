/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pki.
 */

#ifndef __CVMX_PKI_DEFS_H__
#define __CVMX_PKI_DEFS_H__

#define CVMX_PKI_ACTIVE0	     (0x0001180044000220ull)
#define CVMX_PKI_ACTIVE1	     (0x0001180044000230ull)
#define CVMX_PKI_ACTIVE2	     (0x0001180044000240ull)
#define CVMX_PKI_AURAX_CFG(offset)   (0x0001180044900000ull + ((offset) & 1023) * 8)
#define CVMX_PKI_BIST_STATUS0	     (0x0001180044000080ull)
#define CVMX_PKI_BIST_STATUS1	     (0x0001180044000088ull)
#define CVMX_PKI_BIST_STATUS2	     (0x0001180044000090ull)
#define CVMX_PKI_BPIDX_STATE(offset) (0x0001180044B00000ull + ((offset) & 1023) * 8)
#define CVMX_PKI_BUF_CTL	     (0x0001180044000100ull)
#define CVMX_PKI_CHANX_CFG(offset)   (0x0001180044A00000ull + ((offset) & 4095) * 8)
#define CVMX_PKI_CLKEN		     (0x0001180044000410ull)
#define CVMX_PKI_CLX_ECC_CTL(offset) (0x000118004400C020ull + ((offset) & 3) * 0x10000ull)
#define CVMX_PKI_CLX_ECC_INT(offset) (0x000118004400C010ull + ((offset) & 3) * 0x10000ull)
#define CVMX_PKI_CLX_INT(offset)     (0x000118004400C000ull + ((offset) & 3) * 0x10000ull)
#define CVMX_PKI_CLX_PCAMX_ACTIONX(a, b, c)                                                        \
	(0x0001180044708000ull + ((a) << 16) + ((b) << 12) + ((c) << 3))
#define CVMX_PKI_CLX_PCAMX_MATCHX(a, b, c)                                                         \
	(0x0001180044704000ull + ((a) << 16) + ((b) << 12) + ((c) << 3))
#define CVMX_PKI_CLX_PCAMX_TERMX(a, b, c)                                                          \
	(0x0001180044700000ull + ((a) << 16) + ((b) << 12) + ((c) << 3))
#define CVMX_PKI_CLX_PKINDX_CFG(offset, block_id)                                                  \
	(0x0001180044300040ull + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#define CVMX_PKI_CLX_PKINDX_KMEMX(a, b, c)                                                         \
	(0x0001180044200000ull + ((a) << 16) + ((b) << 8) + ((c) << 3))
#define CVMX_PKI_CLX_PKINDX_L2_CUSTOM(offset, block_id)                                            \
	(0x0001180044300058ull + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#define CVMX_PKI_CLX_PKINDX_LG_CUSTOM(offset, block_id)                                            \
	(0x0001180044300060ull + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#define CVMX_PKI_CLX_PKINDX_SKIP(offset, block_id)                                                 \
	(0x0001180044300050ull + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#define CVMX_PKI_CLX_PKINDX_STYLE(offset, block_id)                                                \
	(0x0001180044300048ull + (((offset) & 63) + ((block_id) & 3) * 0x100ull) * 256)
#define CVMX_PKI_CLX_SMEMX(offset, block_id)                                                       \
	(0x0001180044400000ull + (((offset) & 2047) + ((block_id) & 3) * 0x2000ull) * 8)
#define CVMX_PKI_CLX_START(offset) (0x000118004400C030ull + ((offset) & 3) * 0x10000ull)
#define CVMX_PKI_CLX_STYLEX_ALG(offset, block_id)                                                  \
	(0x0001180044501000ull + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8)
#define CVMX_PKI_CLX_STYLEX_CFG(offset, block_id)                                                  \
	(0x0001180044500000ull + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8)
#define CVMX_PKI_CLX_STYLEX_CFG2(offset, block_id)                                                 \
	(0x0001180044500800ull + (((offset) & 63) + ((block_id) & 3) * 0x2000ull) * 8)
#define CVMX_PKI_DSTATX_STAT0(offset)	 (0x0001180044C00000ull + ((offset) & 1023) * 64)
#define CVMX_PKI_DSTATX_STAT1(offset)	 (0x0001180044C00008ull + ((offset) & 1023) * 64)
#define CVMX_PKI_DSTATX_STAT2(offset)	 (0x0001180044C00010ull + ((offset) & 1023) * 64)
#define CVMX_PKI_DSTATX_STAT3(offset)	 (0x0001180044C00018ull + ((offset) & 1023) * 64)
#define CVMX_PKI_DSTATX_STAT4(offset)	 (0x0001180044C00020ull + ((offset) & 1023) * 64)
#define CVMX_PKI_ECC_CTL0		 (0x0001180044000060ull)
#define CVMX_PKI_ECC_CTL1		 (0x0001180044000068ull)
#define CVMX_PKI_ECC_CTL2		 (0x0001180044000070ull)
#define CVMX_PKI_ECC_INT0		 (0x0001180044000040ull)
#define CVMX_PKI_ECC_INT1		 (0x0001180044000048ull)
#define CVMX_PKI_ECC_INT2		 (0x0001180044000050ull)
#define CVMX_PKI_FRM_LEN_CHKX(offset)	 (0x0001180044004000ull + ((offset) & 1) * 8)
#define CVMX_PKI_GBL_PEN		 (0x0001180044000200ull)
#define CVMX_PKI_GEN_INT		 (0x0001180044000020ull)
#define CVMX_PKI_ICGX_CFG(offset)	 (0x000118004400A000ull)
#define CVMX_PKI_IMEMX(offset)		 (0x0001180044100000ull + ((offset) & 2047) * 8)
#define CVMX_PKI_LTYPEX_MAP(offset)	 (0x0001180044005000ull + ((offset) & 31) * 8)
#define CVMX_PKI_PBE_ECO		 (0x0001180044000710ull)
#define CVMX_PKI_PCAM_LOOKUP		 (0x0001180044000500ull)
#define CVMX_PKI_PCAM_RESULT		 (0x0001180044000510ull)
#define CVMX_PKI_PFE_DIAG		 (0x0001180044000560ull)
#define CVMX_PKI_PFE_ECO		 (0x0001180044000720ull)
#define CVMX_PKI_PIX_CLKEN		 (0x0001180044000600ull)
#define CVMX_PKI_PIX_DIAG		 (0x0001180044000580ull)
#define CVMX_PKI_PIX_ECO		 (0x0001180044000700ull)
#define CVMX_PKI_PKINDX_ICGSEL(offset)	 (0x0001180044010000ull + ((offset) & 63) * 8)
#define CVMX_PKI_PKNDX_INB_STAT0(offset) (0x0001180044F00000ull + ((offset) & 63) * 256)
#define CVMX_PKI_PKNDX_INB_STAT1(offset) (0x0001180044F00008ull + ((offset) & 63) * 256)
#define CVMX_PKI_PKNDX_INB_STAT2(offset) (0x0001180044F00010ull + ((offset) & 63) * 256)
#define CVMX_PKI_PKT_ERR		 (0x0001180044000030ull)
#define CVMX_PKI_PTAG_AVAIL		 (0x0001180044000130ull)
#define CVMX_PKI_QPG_TBLBX(offset)	 (0x0001180044820000ull + ((offset) & 2047) * 8)
#define CVMX_PKI_QPG_TBLX(offset)	 (0x0001180044800000ull + ((offset) & 2047) * 8)
#define CVMX_PKI_REASM_SOPX(offset)	 (0x0001180044006000ull + ((offset) & 1) * 8)
#define CVMX_PKI_REQ_WGT		 (0x0001180044000120ull)
#define CVMX_PKI_SFT_RST		 (0x0001180044000010ull)
#define CVMX_PKI_STATX_HIST0(offset)	 (0x0001180044E00000ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_HIST1(offset)	 (0x0001180044E00008ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_HIST2(offset)	 (0x0001180044E00010ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_HIST3(offset)	 (0x0001180044E00018ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_HIST4(offset)	 (0x0001180044E00020ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_HIST5(offset)	 (0x0001180044E00028ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_HIST6(offset)	 (0x0001180044E00030ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT0(offset)	 (0x0001180044E00038ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT1(offset)	 (0x0001180044E00040ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT10(offset)	 (0x0001180044E00088ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT11(offset)	 (0x0001180044E00090ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT12(offset)	 (0x0001180044E00098ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT13(offset)	 (0x0001180044E000A0ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT14(offset)	 (0x0001180044E000A8ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT15(offset)	 (0x0001180044E000B0ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT16(offset)	 (0x0001180044E000B8ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT17(offset)	 (0x0001180044E000C0ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT18(offset)	 (0x0001180044E000C8ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT2(offset)	 (0x0001180044E00048ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT3(offset)	 (0x0001180044E00050ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT4(offset)	 (0x0001180044E00058ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT5(offset)	 (0x0001180044E00060ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT6(offset)	 (0x0001180044E00068ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT7(offset)	 (0x0001180044E00070ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT8(offset)	 (0x0001180044E00078ull + ((offset) & 63) * 256)
#define CVMX_PKI_STATX_STAT9(offset)	 (0x0001180044E00080ull + ((offset) & 63) * 256)
#define CVMX_PKI_STAT_CTL		 (0x0001180044000110ull)
#define CVMX_PKI_STYLEX_BUF(offset)	 (0x0001180044024000ull + ((offset) & 63) * 8)
#define CVMX_PKI_STYLEX_TAG_MASK(offset) (0x0001180044021000ull + ((offset) & 63) * 8)
#define CVMX_PKI_STYLEX_TAG_SEL(offset)	 (0x0001180044020000ull + ((offset) & 63) * 8)
#define CVMX_PKI_STYLEX_WQ2(offset)	 (0x0001180044022000ull + ((offset) & 63) * 8)
#define CVMX_PKI_STYLEX_WQ4(offset)	 (0x0001180044023000ull + ((offset) & 63) * 8)
#define CVMX_PKI_TAG_INCX_CTL(offset)	 (0x0001180044007000ull + ((offset) & 31) * 8)
#define CVMX_PKI_TAG_INCX_MASK(offset)	 (0x0001180044008000ull + ((offset) & 31) * 8)
#define CVMX_PKI_TAG_SECRET		 (0x0001180044000430ull)
#define CVMX_PKI_X2P_REQ_OFL		 (0x0001180044000038ull)

/**
 * cvmx_pki_active0
 */
union cvmx_pki_active0 {
	u64 u64;
	struct cvmx_pki_active0_s {
		u64 reserved_1_63 : 63;
		u64 pfe_active : 1;
	} s;
	struct cvmx_pki_active0_s cn73xx;
	struct cvmx_pki_active0_s cn78xx;
	struct cvmx_pki_active0_s cn78xxp1;
	struct cvmx_pki_active0_s cnf75xx;
};

typedef union cvmx_pki_active0 cvmx_pki_active0_t;

/**
 * cvmx_pki_active1
 */
union cvmx_pki_active1 {
	u64 u64;
	struct cvmx_pki_active1_s {
		u64 reserved_4_63 : 60;
		u64 fpc_active : 1;
		u64 iobp_active : 1;
		u64 sws_active : 1;
		u64 pbtag_active : 1;
	} s;
	struct cvmx_pki_active1_s cn73xx;
	struct cvmx_pki_active1_s cn78xx;
	struct cvmx_pki_active1_s cn78xxp1;
	struct cvmx_pki_active1_s cnf75xx;
};

typedef union cvmx_pki_active1 cvmx_pki_active1_t;

/**
 * cvmx_pki_active2
 */
union cvmx_pki_active2 {
	u64 u64;
	struct cvmx_pki_active2_s {
		u64 reserved_5_63 : 59;
		u64 pix_active : 5;
	} s;
	struct cvmx_pki_active2_s cn73xx;
	struct cvmx_pki_active2_s cn78xx;
	struct cvmx_pki_active2_s cn78xxp1;
	struct cvmx_pki_active2_s cnf75xx;
};

typedef union cvmx_pki_active2 cvmx_pki_active2_t;

/**
 * cvmx_pki_aura#_cfg
 *
 * This register configures aura backpressure, etc.
 *
 */
union cvmx_pki_aurax_cfg {
	u64 u64;
	struct cvmx_pki_aurax_cfg_s {
		u64 reserved_32_63 : 32;
		u64 pkt_add : 2;
		u64 reserved_19_29 : 11;
		u64 ena_red : 1;
		u64 ena_drop : 1;
		u64 ena_bp : 1;
		u64 reserved_10_15 : 6;
		u64 bpid : 10;
	} s;
	struct cvmx_pki_aurax_cfg_s cn73xx;
	struct cvmx_pki_aurax_cfg_s cn78xx;
	struct cvmx_pki_aurax_cfg_s cn78xxp1;
	struct cvmx_pki_aurax_cfg_s cnf75xx;
};

typedef union cvmx_pki_aurax_cfg cvmx_pki_aurax_cfg_t;

/**
 * cvmx_pki_bist_status0
 *
 * This register indicates BIST status.
 *
 */
union cvmx_pki_bist_status0 {
	u64 u64;
	struct cvmx_pki_bist_status0_s {
		u64 reserved_31_63 : 33;
		u64 bist : 31;
	} s;
	struct cvmx_pki_bist_status0_s cn73xx;
	struct cvmx_pki_bist_status0_s cn78xx;
	struct cvmx_pki_bist_status0_s cn78xxp1;
	struct cvmx_pki_bist_status0_s cnf75xx;
};

typedef union cvmx_pki_bist_status0 cvmx_pki_bist_status0_t;

/**
 * cvmx_pki_bist_status1
 *
 * This register indicates BIST status.
 *
 */
union cvmx_pki_bist_status1 {
	u64 u64;
	struct cvmx_pki_bist_status1_s {
		u64 reserved_26_63 : 38;
		u64 bist : 26;
	} s;
	struct cvmx_pki_bist_status1_s cn73xx;
	struct cvmx_pki_bist_status1_s cn78xx;
	struct cvmx_pki_bist_status1_cn78xxp1 {
		u64 reserved_21_63 : 43;
		u64 bist : 21;
	} cn78xxp1;
	struct cvmx_pki_bist_status1_s cnf75xx;
};

typedef union cvmx_pki_bist_status1 cvmx_pki_bist_status1_t;

/**
 * cvmx_pki_bist_status2
 *
 * This register indicates BIST status.
 *
 */
union cvmx_pki_bist_status2 {
	u64 u64;
	struct cvmx_pki_bist_status2_s {
		u64 reserved_25_63 : 39;
		u64 bist : 25;
	} s;
	struct cvmx_pki_bist_status2_s cn73xx;
	struct cvmx_pki_bist_status2_s cn78xx;
	struct cvmx_pki_bist_status2_s cn78xxp1;
	struct cvmx_pki_bist_status2_s cnf75xx;
};

typedef union cvmx_pki_bist_status2 cvmx_pki_bist_status2_t;

/**
 * cvmx_pki_bpid#_state
 *
 * This register shows the current bpid state for diagnostics.
 *
 */
union cvmx_pki_bpidx_state {
	u64 u64;
	struct cvmx_pki_bpidx_state_s {
		u64 reserved_1_63 : 63;
		u64 xoff : 1;
	} s;
	struct cvmx_pki_bpidx_state_s cn73xx;
	struct cvmx_pki_bpidx_state_s cn78xx;
	struct cvmx_pki_bpidx_state_s cn78xxp1;
	struct cvmx_pki_bpidx_state_s cnf75xx;
};

typedef union cvmx_pki_bpidx_state cvmx_pki_bpidx_state_t;

/**
 * cvmx_pki_buf_ctl
 */
union cvmx_pki_buf_ctl {
	u64 u64;
	struct cvmx_pki_buf_ctl_s {
		u64 reserved_11_63 : 53;
		u64 fpa_wait : 1;
		u64 fpa_cac_dis : 1;
		u64 reserved_6_8 : 3;
		u64 pkt_off : 1;
		u64 reserved_3_4 : 2;
		u64 pbp_en : 1;
		u64 reserved_1_1 : 1;
		u64 pki_en : 1;
	} s;
	struct cvmx_pki_buf_ctl_s cn73xx;
	struct cvmx_pki_buf_ctl_s cn78xx;
	struct cvmx_pki_buf_ctl_s cn78xxp1;
	struct cvmx_pki_buf_ctl_s cnf75xx;
};

typedef union cvmx_pki_buf_ctl cvmx_pki_buf_ctl_t;

/**
 * cvmx_pki_chan#_cfg
 *
 * This register configures each channel.
 *
 */
union cvmx_pki_chanx_cfg {
	u64 u64;
	struct cvmx_pki_chanx_cfg_s {
		u64 reserved_17_63 : 47;
		u64 imp : 1;
		u64 reserved_10_15 : 6;
		u64 bpid : 10;
	} s;
	struct cvmx_pki_chanx_cfg_s cn73xx;
	struct cvmx_pki_chanx_cfg_s cn78xx;
	struct cvmx_pki_chanx_cfg_s cn78xxp1;
	struct cvmx_pki_chanx_cfg_s cnf75xx;
};

typedef union cvmx_pki_chanx_cfg cvmx_pki_chanx_cfg_t;

/**
 * cvmx_pki_cl#_ecc_ctl
 *
 * This register configures ECC. All of PKI_CL()_ECC_CTL must be configured identically.
 *
 */
union cvmx_pki_clx_ecc_ctl {
	u64 u64;
	struct cvmx_pki_clx_ecc_ctl_s {
		u64 pcam_en : 1;
		u64 reserved_24_62 : 39;
		u64 pcam1_flip : 2;
		u64 pcam0_flip : 2;
		u64 smem_flip : 2;
		u64 dmem_flip : 1;
		u64 rf_flip : 1;
		u64 reserved_5_15 : 11;
		u64 pcam1_cdis : 1;
		u64 pcam0_cdis : 1;
		u64 smem_cdis : 1;
		u64 dmem_cdis : 1;
		u64 rf_cdis : 1;
	} s;
	struct cvmx_pki_clx_ecc_ctl_s cn73xx;
	struct cvmx_pki_clx_ecc_ctl_s cn78xx;
	struct cvmx_pki_clx_ecc_ctl_s cn78xxp1;
	struct cvmx_pki_clx_ecc_ctl_s cnf75xx;
};

typedef union cvmx_pki_clx_ecc_ctl cvmx_pki_clx_ecc_ctl_t;

/**
 * cvmx_pki_cl#_ecc_int
 */
union cvmx_pki_clx_ecc_int {
	u64 u64;
	struct cvmx_pki_clx_ecc_int_s {
		u64 reserved_8_63 : 56;
		u64 pcam1_dbe : 1;
		u64 pcam1_sbe : 1;
		u64 pcam0_dbe : 1;
		u64 pcam0_sbe : 1;
		u64 smem_dbe : 1;
		u64 smem_sbe : 1;
		u64 dmem_perr : 1;
		u64 rf_perr : 1;
	} s;
	struct cvmx_pki_clx_ecc_int_s cn73xx;
	struct cvmx_pki_clx_ecc_int_s cn78xx;
	struct cvmx_pki_clx_ecc_int_s cn78xxp1;
	struct cvmx_pki_clx_ecc_int_s cnf75xx;
};

typedef union cvmx_pki_clx_ecc_int cvmx_pki_clx_ecc_int_t;

/**
 * cvmx_pki_cl#_int
 */
union cvmx_pki_clx_int {
	u64 u64;
	struct cvmx_pki_clx_int_s {
		u64 reserved_4_63 : 60;
		u64 iptint : 1;
		u64 sched_conf : 1;
		u64 pcam_conf : 2;
	} s;
	struct cvmx_pki_clx_int_s cn73xx;
	struct cvmx_pki_clx_int_s cn78xx;
	struct cvmx_pki_clx_int_s cn78xxp1;
	struct cvmx_pki_clx_int_s cnf75xx;
};

typedef union cvmx_pki_clx_int cvmx_pki_clx_int_t;

/**
 * cvmx_pki_cl#_pcam#_action#
 *
 * This register configures the result side of the PCAM. PKI hardware is opaque as to the use
 * of the 32 bits of CAM result.
 *
 * For each legal j and k, PKI_CL(i)_PCAM(j)_ACTION(k) must be configured identically for i=0..1.
 *
 * With the current parse engine code:
 *
 * Action performed based on PCAM lookup using the PKI_CL()_PCAM()_TERM() and
 * PKI_CL()_PCAM()_MATCH() registers.
 *
 * If lookup data matches no PCAM entries, then no action takes place. No matches indicates
 * normal parsing will continue.
 *
 * If data matches multiple PCAM entries, PKI_WQE_S[ERRLEV,OPCODE] of the processed packet may
 * be set to PKI_ERRLEV_E::RE,PKI_OPCODE_E::RE_PKIPCAM and the PKI_CL()_INT[PCAM_CONF] error
 * interrupt is signaled.  Once a conflict is detected, the PCAM state is unpredictable and is
 * required to be fully reconfigured before further valid processing can take place.
 */
union cvmx_pki_clx_pcamx_actionx {
	u64 u64;
	struct cvmx_pki_clx_pcamx_actionx_s {
		u64 reserved_31_63 : 33;
		u64 pmc : 7;
		u64 style_add : 8;
		u64 pf : 3;
		u64 setty : 5;
		u64 advance : 8;
	} s;
	struct cvmx_pki_clx_pcamx_actionx_s cn73xx;
	struct cvmx_pki_clx_pcamx_actionx_s cn78xx;
	struct cvmx_pki_clx_pcamx_actionx_s cn78xxp1;
	struct cvmx_pki_clx_pcamx_actionx_s cnf75xx;
};

typedef union cvmx_pki_clx_pcamx_actionx cvmx_pki_clx_pcamx_actionx_t;

/**
 * cvmx_pki_cl#_pcam#_match#
 *
 * This register configures the match side of the PCAM. PKI hardware is opaque as to the use
 * of the 32 bits of CAM data.
 *
 * For each legal j and k, PKI_CL(i)_PCAM(j)_MATCH(k) must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pcamx_matchx {
	u64 u64;
	struct cvmx_pki_clx_pcamx_matchx_s {
		u64 data1 : 32;
		u64 data0 : 32;
	} s;
	struct cvmx_pki_clx_pcamx_matchx_s cn73xx;
	struct cvmx_pki_clx_pcamx_matchx_s cn78xx;
	struct cvmx_pki_clx_pcamx_matchx_s cn78xxp1;
	struct cvmx_pki_clx_pcamx_matchx_s cnf75xx;
};

typedef union cvmx_pki_clx_pcamx_matchx cvmx_pki_clx_pcamx_matchx_t;

/**
 * cvmx_pki_cl#_pcam#_term#
 *
 * This register configures the match side of the PCAM. PKI hardware is opaque as to the use
 * of the 16 bits of CAM data; the split between TERM and STYLE is defined by the
 * parse engine.
 *
 * For each legal j and k, PKI_CL(i)_PCAM(j)_TERM(k) must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pcamx_termx {
	u64 u64;
	struct cvmx_pki_clx_pcamx_termx_s {
		u64 valid : 1;
		u64 reserved_48_62 : 15;
		u64 term1 : 8;
		u64 style1 : 8;
		u64 reserved_16_31 : 16;
		u64 term0 : 8;
		u64 style0 : 8;
	} s;
	struct cvmx_pki_clx_pcamx_termx_s cn73xx;
	struct cvmx_pki_clx_pcamx_termx_s cn78xx;
	struct cvmx_pki_clx_pcamx_termx_s cn78xxp1;
	struct cvmx_pki_clx_pcamx_termx_s cnf75xx;
};

typedef union cvmx_pki_clx_pcamx_termx cvmx_pki_clx_pcamx_termx_t;

/**
 * cvmx_pki_cl#_pkind#_cfg
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_CFG must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_cfg {
	u64 u64;
	struct cvmx_pki_clx_pkindx_cfg_s {
		u64 reserved_11_63 : 53;
		u64 lg_custom_layer : 3;
		u64 fcs_pres : 1;
		u64 mpls_en : 1;
		u64 inst_hdr : 1;
		u64 lg_custom : 1;
		u64 fulc_en : 1;
		u64 dsa_en : 1;
		u64 hg2_en : 1;
		u64 hg_en : 1;
	} s;
	struct cvmx_pki_clx_pkindx_cfg_s cn73xx;
	struct cvmx_pki_clx_pkindx_cfg_s cn78xx;
	struct cvmx_pki_clx_pkindx_cfg_s cn78xxp1;
	struct cvmx_pki_clx_pkindx_cfg_s cnf75xx;
};

typedef union cvmx_pki_clx_pkindx_cfg cvmx_pki_clx_pkindx_cfg_t;

/**
 * cvmx_pki_cl#_pkind#_kmem#
 *
 * This register initializes the KMEM, which initializes the parse engine state for each
 * pkind. These CSRs are used only by the PKI parse engine.
 *
 * Inside the KMEM are the following parse engine registers. These registers are the
 * preferred access method for software:
 * * PKI_CL()_PKIND()_CFG.
 * * PKI_CL()_PKIND()_STYLE.
 * * PKI_CL()_PKIND()_SKIP.
 * * PKI_CL()_PKIND()_L2_CUSTOM.
 * * PKI_CL()_PKIND()_LG_CUSTOM.
 *
 * To avoid overlapping addresses, these aliases have address bit 20 set in contrast to
 * this register; the PKI address decoder ignores bit 20 when accessing
 * PKI_CL()_PKIND()_KMEM().
 *
 * Software must reload the PKI_CL()_PKIND()_KMEM() registers upon the detection of
 * PKI_ECC_INT0[KMEM_SBE] or PKI_ECC_INT0[KMEM_DBE].
 *
 * For each legal j and k value, PKI_CL(i)_PKIND(j)_KMEM(k) must be configured
 * identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_kmemx {
	u64 u64;
	struct cvmx_pki_clx_pkindx_kmemx_s {
		u64 reserved_16_63 : 48;
		u64 data : 16;
	} s;
	struct cvmx_pki_clx_pkindx_kmemx_s cn73xx;
	struct cvmx_pki_clx_pkindx_kmemx_s cn78xx;
	struct cvmx_pki_clx_pkindx_kmemx_s cn78xxp1;
	struct cvmx_pki_clx_pkindx_kmemx_s cnf75xx;
};

typedef union cvmx_pki_clx_pkindx_kmemx cvmx_pki_clx_pkindx_kmemx_t;

/**
 * cvmx_pki_cl#_pkind#_l2_custom
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_L2_CUSTOM must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_l2_custom {
	u64 u64;
	struct cvmx_pki_clx_pkindx_l2_custom_s {
		u64 reserved_16_63 : 48;
		u64 valid : 1;
		u64 reserved_8_14 : 7;
		u64 offset : 8;
	} s;
	struct cvmx_pki_clx_pkindx_l2_custom_s cn73xx;
	struct cvmx_pki_clx_pkindx_l2_custom_s cn78xx;
	struct cvmx_pki_clx_pkindx_l2_custom_s cn78xxp1;
	struct cvmx_pki_clx_pkindx_l2_custom_s cnf75xx;
};

typedef union cvmx_pki_clx_pkindx_l2_custom cvmx_pki_clx_pkindx_l2_custom_t;

/**
 * cvmx_pki_cl#_pkind#_lg_custom
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_LG_CUSTOM must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_lg_custom {
	u64 u64;
	struct cvmx_pki_clx_pkindx_lg_custom_s {
		u64 reserved_8_63 : 56;
		u64 offset : 8;
	} s;
	struct cvmx_pki_clx_pkindx_lg_custom_s cn73xx;
	struct cvmx_pki_clx_pkindx_lg_custom_s cn78xx;
	struct cvmx_pki_clx_pkindx_lg_custom_s cn78xxp1;
	struct cvmx_pki_clx_pkindx_lg_custom_s cnf75xx;
};

typedef union cvmx_pki_clx_pkindx_lg_custom cvmx_pki_clx_pkindx_lg_custom_t;

/**
 * cvmx_pki_cl#_pkind#_skip
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_SKIP must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_skip {
	u64 u64;
	struct cvmx_pki_clx_pkindx_skip_s {
		u64 reserved_16_63 : 48;
		u64 fcs_skip : 8;
		u64 inst_skip : 8;
	} s;
	struct cvmx_pki_clx_pkindx_skip_s cn73xx;
	struct cvmx_pki_clx_pkindx_skip_s cn78xx;
	struct cvmx_pki_clx_pkindx_skip_s cn78xxp1;
	struct cvmx_pki_clx_pkindx_skip_s cnf75xx;
};

typedef union cvmx_pki_clx_pkindx_skip cvmx_pki_clx_pkindx_skip_t;

/**
 * cvmx_pki_cl#_pkind#_style
 *
 * This register is inside PKI_CL()_PKIND()_KMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_PKIND(j)_STYLE must be configured identically for i=0..1.
 */
union cvmx_pki_clx_pkindx_style {
	u64 u64;
	struct cvmx_pki_clx_pkindx_style_s {
		u64 reserved_15_63 : 49;
		u64 pm : 7;
		u64 style : 8;
	} s;
	struct cvmx_pki_clx_pkindx_style_s cn73xx;
	struct cvmx_pki_clx_pkindx_style_s cn78xx;
	struct cvmx_pki_clx_pkindx_style_s cn78xxp1;
	struct cvmx_pki_clx_pkindx_style_s cnf75xx;
};

typedef union cvmx_pki_clx_pkindx_style cvmx_pki_clx_pkindx_style_t;

/**
 * cvmx_pki_cl#_smem#
 *
 * This register initializes the SMEM, which configures the parse engine. These CSRs
 * are used by the PKI parse engine and other PKI hardware.
 *
 * Inside the SMEM are the following parse engine registers. These registers are the
 * preferred access method for software:
 * * PKI_CL()_STYLE()_CFG
 * * PKI_CL()_STYLE()_CFG2
 * * PKI_CL()_STYLE()_ALG
 *
 * To avoid overlapping addresses, these aliases have address bit 20 set in contrast to
 * this register; the PKI address decoder ignores bit 20 when accessing
 * PKI_CL()_SMEM().
 *
 * Software must reload the PKI_CL()_SMEM() registers upon the detection of
 * PKI_CL()_ECC_INT[SMEM_SBE] or PKI_CL()_ECC_INT[SMEM_DBE].
 *
 * For each legal j, PKI_CL(i)_SMEM(j) must be configured identically for i=0..1.
 */
union cvmx_pki_clx_smemx {
	u64 u64;
	struct cvmx_pki_clx_smemx_s {
		u64 reserved_32_63 : 32;
		u64 data : 32;
	} s;
	struct cvmx_pki_clx_smemx_s cn73xx;
	struct cvmx_pki_clx_smemx_s cn78xx;
	struct cvmx_pki_clx_smemx_s cn78xxp1;
	struct cvmx_pki_clx_smemx_s cnf75xx;
};

typedef union cvmx_pki_clx_smemx cvmx_pki_clx_smemx_t;

/**
 * cvmx_pki_cl#_start
 *
 * This register configures a cluster. All of PKI_CL()_START must be programmed identically.
 *
 */
union cvmx_pki_clx_start {
	u64 u64;
	struct cvmx_pki_clx_start_s {
		u64 reserved_11_63 : 53;
		u64 start : 11;
	} s;
	struct cvmx_pki_clx_start_s cn73xx;
	struct cvmx_pki_clx_start_s cn78xx;
	struct cvmx_pki_clx_start_s cn78xxp1;
	struct cvmx_pki_clx_start_s cnf75xx;
};

typedef union cvmx_pki_clx_start cvmx_pki_clx_start_t;

/**
 * cvmx_pki_cl#_style#_alg
 *
 * This register is inside PKI_CL()_SMEM(). These CSRs are used only by
 * the PKI parse engine.
 *
 * For each legal j, PKI_CL(i)_STYLE(j)_ALG must be configured identically for i=0..1.
 */
union cvmx_pki_clx_stylex_alg {
	u64 u64;
	struct cvmx_pki_clx_stylex_alg_s {
		u64 reserved_32_63 : 32;
		u64 tt : 2;
		u64 apad_nip : 3;
		u64 qpg_qos : 3;
		u64 qpg_port_sh : 3;
		u64 qpg_port_msb : 4;
		u64 reserved_11_16 : 6;
		u64 tag_vni : 1;
		u64 tag_gtp : 1;
		u64 tag_spi : 1;
		u64 tag_syn : 1;
		u64 tag_pctl : 1;
		u64 tag_vs1 : 1;
		u64 tag_vs0 : 1;
		u64 tag_vlan : 1;
		u64 tag_mpls0 : 1;
		u64 tag_prt : 1;
		u64 wqe_vs : 1;
	} s;
	struct cvmx_pki_clx_stylex_alg_s cn73xx;
	struct cvmx_pki_clx_stylex_alg_s cn78xx;
	struct cvmx_pki_clx_stylex_alg_s cn78xxp1;
	struct cvmx_pki_clx_stylex_alg_s cnf75xx;
};

typedef union cvmx_pki_clx_stylex_alg cvmx_pki_clx_stylex_alg_t;

/**
 * cvmx_pki_cl#_style#_cfg
 *
 * This register is inside PKI_CL()_SMEM(). These CSRs are used by
 * the PKI parse engine and other PKI hardware.
 *
 * For each legal j, PKI_CL(i)_STYLE(j)_CFG must be configured identically for i=0..1.
 */
union cvmx_pki_clx_stylex_cfg {
	u64 u64;
	struct cvmx_pki_clx_stylex_cfg_s {
		u64 reserved_31_63 : 33;
		u64 ip6_udp_opt : 1;
		u64 lenerr_en : 1;
		u64 lenerr_eqpad : 1;
		u64 minmax_sel : 1;
		u64 maxerr_en : 1;
		u64 minerr_en : 1;
		u64 qpg_dis_grptag : 1;
		u64 fcs_strip : 1;
		u64 fcs_chk : 1;
		u64 rawdrp : 1;
		u64 drop : 1;
		u64 nodrop : 1;
		u64 qpg_dis_padd : 1;
		u64 qpg_dis_grp : 1;
		u64 qpg_dis_aura : 1;
		u64 reserved_11_15 : 5;
		u64 qpg_base : 11;
	} s;
	struct cvmx_pki_clx_stylex_cfg_s cn73xx;
	struct cvmx_pki_clx_stylex_cfg_s cn78xx;
	struct cvmx_pki_clx_stylex_cfg_s cn78xxp1;
	struct cvmx_pki_clx_stylex_cfg_s cnf75xx;
};

typedef union cvmx_pki_clx_stylex_cfg cvmx_pki_clx_stylex_cfg_t;

/**
 * cvmx_pki_cl#_style#_cfg2
 *
 * This register is inside PKI_CL()_SMEM(). These CSRs are used by
 * the PKI parse engine and other PKI hardware.
 *
 * For each legal j, PKI_CL(i)_STYLE(j)_CFG2 must be configured identically for i=0..1.
 */
union cvmx_pki_clx_stylex_cfg2 {
	u64 u64;
	struct cvmx_pki_clx_stylex_cfg2_s {
		u64 reserved_32_63 : 32;
		u64 tag_inc : 4;
		u64 reserved_25_27 : 3;
		u64 tag_masken : 1;
		u64 tag_src_lg : 1;
		u64 tag_src_lf : 1;
		u64 tag_src_le : 1;
		u64 tag_src_ld : 1;
		u64 tag_src_lc : 1;
		u64 tag_src_lb : 1;
		u64 tag_dst_lg : 1;
		u64 tag_dst_lf : 1;
		u64 tag_dst_le : 1;
		u64 tag_dst_ld : 1;
		u64 tag_dst_lc : 1;
		u64 tag_dst_lb : 1;
		u64 len_lg : 1;
		u64 len_lf : 1;
		u64 len_le : 1;
		u64 len_ld : 1;
		u64 len_lc : 1;
		u64 len_lb : 1;
		u64 csum_lg : 1;
		u64 csum_lf : 1;
		u64 csum_le : 1;
		u64 csum_ld : 1;
		u64 csum_lc : 1;
		u64 csum_lb : 1;
	} s;
	struct cvmx_pki_clx_stylex_cfg2_s cn73xx;
	struct cvmx_pki_clx_stylex_cfg2_s cn78xx;
	struct cvmx_pki_clx_stylex_cfg2_s cn78xxp1;
	struct cvmx_pki_clx_stylex_cfg2_s cnf75xx;
};

typedef union cvmx_pki_clx_stylex_cfg2 cvmx_pki_clx_stylex_cfg2_t;

/**
 * cvmx_pki_clken
 */
union cvmx_pki_clken {
	u64 u64;
	struct cvmx_pki_clken_s {
		u64 reserved_1_63 : 63;
		u64 clken : 1;
	} s;
	struct cvmx_pki_clken_s cn73xx;
	struct cvmx_pki_clken_s cn78xx;
	struct cvmx_pki_clken_s cn78xxp1;
	struct cvmx_pki_clken_s cnf75xx;
};

typedef union cvmx_pki_clken cvmx_pki_clken_t;

/**
 * cvmx_pki_dstat#_stat0
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat0 {
	u64 u64;
	struct cvmx_pki_dstatx_stat0_s {
		u64 reserved_32_63 : 32;
		u64 pkts : 32;
	} s;
	struct cvmx_pki_dstatx_stat0_s cn73xx;
	struct cvmx_pki_dstatx_stat0_s cn78xx;
	struct cvmx_pki_dstatx_stat0_s cnf75xx;
};

typedef union cvmx_pki_dstatx_stat0 cvmx_pki_dstatx_stat0_t;

/**
 * cvmx_pki_dstat#_stat1
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat1 {
	u64 u64;
	struct cvmx_pki_dstatx_stat1_s {
		u64 reserved_40_63 : 24;
		u64 octs : 40;
	} s;
	struct cvmx_pki_dstatx_stat1_s cn73xx;
	struct cvmx_pki_dstatx_stat1_s cn78xx;
	struct cvmx_pki_dstatx_stat1_s cnf75xx;
};

typedef union cvmx_pki_dstatx_stat1 cvmx_pki_dstatx_stat1_t;

/**
 * cvmx_pki_dstat#_stat2
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat2 {
	u64 u64;
	struct cvmx_pki_dstatx_stat2_s {
		u64 reserved_32_63 : 32;
		u64 err_pkts : 32;
	} s;
	struct cvmx_pki_dstatx_stat2_s cn73xx;
	struct cvmx_pki_dstatx_stat2_s cn78xx;
	struct cvmx_pki_dstatx_stat2_s cnf75xx;
};

typedef union cvmx_pki_dstatx_stat2 cvmx_pki_dstatx_stat2_t;

/**
 * cvmx_pki_dstat#_stat3
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat3 {
	u64 u64;
	struct cvmx_pki_dstatx_stat3_s {
		u64 reserved_32_63 : 32;
		u64 drp_pkts : 32;
	} s;
	struct cvmx_pki_dstatx_stat3_s cn73xx;
	struct cvmx_pki_dstatx_stat3_s cn78xx;
	struct cvmx_pki_dstatx_stat3_s cnf75xx;
};

typedef union cvmx_pki_dstatx_stat3 cvmx_pki_dstatx_stat3_t;

/**
 * cvmx_pki_dstat#_stat4
 *
 * This register contains statistics indexed by PKI_QPG_TBLB()[DSTAT_ID].
 *
 */
union cvmx_pki_dstatx_stat4 {
	u64 u64;
	struct cvmx_pki_dstatx_stat4_s {
		u64 reserved_40_63 : 24;
		u64 drp_octs : 40;
	} s;
	struct cvmx_pki_dstatx_stat4_s cn73xx;
	struct cvmx_pki_dstatx_stat4_s cn78xx;
	struct cvmx_pki_dstatx_stat4_s cnf75xx;
};

typedef union cvmx_pki_dstatx_stat4 cvmx_pki_dstatx_stat4_t;

/**
 * cvmx_pki_ecc_ctl0
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_pki_ecc_ctl0 {
	u64 u64;
	struct cvmx_pki_ecc_ctl0_s {
		u64 reserved_24_63 : 40;
		u64 ldfif_flip : 2;
		u64 ldfif_cdis : 1;
		u64 pbe_flip : 2;
		u64 pbe_cdis : 1;
		u64 wadr_flip : 2;
		u64 wadr_cdis : 1;
		u64 nxtptag_flip : 2;
		u64 nxtptag_cdis : 1;
		u64 curptag_flip : 2;
		u64 curptag_cdis : 1;
		u64 nxtblk_flip : 2;
		u64 nxtblk_cdis : 1;
		u64 kmem_flip : 2;
		u64 kmem_cdis : 1;
		u64 asm_flip : 2;
		u64 asm_cdis : 1;
	} s;
	struct cvmx_pki_ecc_ctl0_s cn73xx;
	struct cvmx_pki_ecc_ctl0_s cn78xx;
	struct cvmx_pki_ecc_ctl0_s cn78xxp1;
	struct cvmx_pki_ecc_ctl0_s cnf75xx;
};

typedef union cvmx_pki_ecc_ctl0 cvmx_pki_ecc_ctl0_t;

/**
 * cvmx_pki_ecc_ctl1
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_pki_ecc_ctl1 {
	u64 u64;
	struct cvmx_pki_ecc_ctl1_s {
		u64 reserved_51_63 : 13;
		u64 sws_flip : 2;
		u64 sws_cdis : 1;
		u64 wqeout_flip : 2;
		u64 wqeout_cdis : 1;
		u64 doa_flip : 2;
		u64 doa_cdis : 1;
		u64 bpid_flip : 2;
		u64 bpid_cdis : 1;
		u64 reserved_30_38 : 9;
		u64 plc_flip : 2;
		u64 plc_cdis : 1;
		u64 pktwq_flip : 2;
		u64 pktwq_cdis : 1;
		u64 reserved_21_23 : 3;
		u64 stylewq2_flip : 2;
		u64 stylewq2_cdis : 1;
		u64 tag_flip : 2;
		u64 tag_cdis : 1;
		u64 aura_flip : 2;
		u64 aura_cdis : 1;
		u64 chan_flip : 2;
		u64 chan_cdis : 1;
		u64 pbtag_flip : 2;
		u64 pbtag_cdis : 1;
		u64 stylewq_flip : 2;
		u64 stylewq_cdis : 1;
		u64 qpg_flip : 2;
		u64 qpg_cdis : 1;
	} s;
	struct cvmx_pki_ecc_ctl1_s cn73xx;
	struct cvmx_pki_ecc_ctl1_s cn78xx;
	struct cvmx_pki_ecc_ctl1_cn78xxp1 {
		u64 reserved_51_63 : 13;
		u64 sws_flip : 2;
		u64 sws_cdis : 1;
		u64 wqeout_flip : 2;
		u64 wqeout_cdis : 1;
		u64 doa_flip : 2;
		u64 doa_cdis : 1;
		u64 bpid_flip : 2;
		u64 bpid_cdis : 1;
		u64 reserved_30_38 : 9;
		u64 plc_flip : 2;
		u64 plc_cdis : 1;
		u64 pktwq_flip : 2;
		u64 pktwq_cdis : 1;
		u64 reserved_18_23 : 6;
		u64 tag_flip : 2;
		u64 tag_cdis : 1;
		u64 aura_flip : 2;
		u64 aura_cdis : 1;
		u64 chan_flip : 2;
		u64 chan_cdis : 1;
		u64 pbtag_flip : 2;
		u64 pbtag_cdis : 1;
		u64 stylewq_flip : 2;
		u64 stylewq_cdis : 1;
		u64 qpg_flip : 2;
		u64 qpg_cdis : 1;
	} cn78xxp1;
	struct cvmx_pki_ecc_ctl1_s cnf75xx;
};

typedef union cvmx_pki_ecc_ctl1 cvmx_pki_ecc_ctl1_t;

/**
 * cvmx_pki_ecc_ctl2
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_pki_ecc_ctl2 {
	u64 u64;
	struct cvmx_pki_ecc_ctl2_s {
		u64 reserved_3_63 : 61;
		u64 imem_flip : 2;
		u64 imem_cdis : 1;
	} s;
	struct cvmx_pki_ecc_ctl2_s cn73xx;
	struct cvmx_pki_ecc_ctl2_s cn78xx;
	struct cvmx_pki_ecc_ctl2_s cn78xxp1;
	struct cvmx_pki_ecc_ctl2_s cnf75xx;
};

typedef union cvmx_pki_ecc_ctl2 cvmx_pki_ecc_ctl2_t;

/**
 * cvmx_pki_ecc_int0
 */
union cvmx_pki_ecc_int0 {
	u64 u64;
	struct cvmx_pki_ecc_int0_s {
		u64 reserved_16_63 : 48;
		u64 ldfif_dbe : 1;
		u64 ldfif_sbe : 1;
		u64 pbe_dbe : 1;
		u64 pbe_sbe : 1;
		u64 wadr_dbe : 1;
		u64 wadr_sbe : 1;
		u64 nxtptag_dbe : 1;
		u64 nxtptag_sbe : 1;
		u64 curptag_dbe : 1;
		u64 curptag_sbe : 1;
		u64 nxtblk_dbe : 1;
		u64 nxtblk_sbe : 1;
		u64 kmem_dbe : 1;
		u64 kmem_sbe : 1;
		u64 asm_dbe : 1;
		u64 asm_sbe : 1;
	} s;
	struct cvmx_pki_ecc_int0_s cn73xx;
	struct cvmx_pki_ecc_int0_s cn78xx;
	struct cvmx_pki_ecc_int0_s cn78xxp1;
	struct cvmx_pki_ecc_int0_s cnf75xx;
};

typedef union cvmx_pki_ecc_int0 cvmx_pki_ecc_int0_t;

/**
 * cvmx_pki_ecc_int1
 */
union cvmx_pki_ecc_int1 {
	u64 u64;
	struct cvmx_pki_ecc_int1_s {
		u64 reserved_34_63 : 30;
		u64 sws_dbe : 1;
		u64 sws_sbe : 1;
		u64 wqeout_dbe : 1;
		u64 wqeout_sbe : 1;
		u64 doa_dbe : 1;
		u64 doa_sbe : 1;
		u64 bpid_dbe : 1;
		u64 bpid_sbe : 1;
		u64 reserved_20_25 : 6;
		u64 plc_dbe : 1;
		u64 plc_sbe : 1;
		u64 pktwq_dbe : 1;
		u64 pktwq_sbe : 1;
		u64 reserved_12_15 : 4;
		u64 tag_dbe : 1;
		u64 tag_sbe : 1;
		u64 aura_dbe : 1;
		u64 aura_sbe : 1;
		u64 chan_dbe : 1;
		u64 chan_sbe : 1;
		u64 pbtag_dbe : 1;
		u64 pbtag_sbe : 1;
		u64 stylewq_dbe : 1;
		u64 stylewq_sbe : 1;
		u64 qpg_dbe : 1;
		u64 qpg_sbe : 1;
	} s;
	struct cvmx_pki_ecc_int1_s cn73xx;
	struct cvmx_pki_ecc_int1_s cn78xx;
	struct cvmx_pki_ecc_int1_s cn78xxp1;
	struct cvmx_pki_ecc_int1_s cnf75xx;
};

typedef union cvmx_pki_ecc_int1 cvmx_pki_ecc_int1_t;

/**
 * cvmx_pki_ecc_int2
 */
union cvmx_pki_ecc_int2 {
	u64 u64;
	struct cvmx_pki_ecc_int2_s {
		u64 reserved_2_63 : 62;
		u64 imem_dbe : 1;
		u64 imem_sbe : 1;
	} s;
	struct cvmx_pki_ecc_int2_s cn73xx;
	struct cvmx_pki_ecc_int2_s cn78xx;
	struct cvmx_pki_ecc_int2_s cn78xxp1;
	struct cvmx_pki_ecc_int2_s cnf75xx;
};

typedef union cvmx_pki_ecc_int2 cvmx_pki_ecc_int2_t;

/**
 * cvmx_pki_frm_len_chk#
 */
union cvmx_pki_frm_len_chkx {
	u64 u64;
	struct cvmx_pki_frm_len_chkx_s {
		u64 reserved_32_63 : 32;
		u64 maxlen : 16;
		u64 minlen : 16;
	} s;
	struct cvmx_pki_frm_len_chkx_s cn73xx;
	struct cvmx_pki_frm_len_chkx_s cn78xx;
	struct cvmx_pki_frm_len_chkx_s cn78xxp1;
	struct cvmx_pki_frm_len_chkx_s cnf75xx;
};

typedef union cvmx_pki_frm_len_chkx cvmx_pki_frm_len_chkx_t;

/**
 * cvmx_pki_gbl_pen
 *
 * This register contains global configuration information that applies to all
 * pkinds. The values are opaque to PKI HW.
 *
 * This is intended for communication between the higher-level software SDK, and the
 * SDK code that loads PKI_IMEM() with the parse engine code. This allows the loader to
 * appropriately select the parse engine code with only those features required, so that
 * performance will be optimized.
 */
union cvmx_pki_gbl_pen {
	u64 u64;
	struct cvmx_pki_gbl_pen_s {
		u64 reserved_10_63 : 54;
		u64 virt_pen : 1;
		u64 clg_pen : 1;
		u64 cl2_pen : 1;
		u64 l4_pen : 1;
		u64 il3_pen : 1;
		u64 l3_pen : 1;
		u64 mpls_pen : 1;
		u64 fulc_pen : 1;
		u64 dsa_pen : 1;
		u64 hg_pen : 1;
	} s;
	struct cvmx_pki_gbl_pen_s cn73xx;
	struct cvmx_pki_gbl_pen_s cn78xx;
	struct cvmx_pki_gbl_pen_s cn78xxp1;
	struct cvmx_pki_gbl_pen_s cnf75xx;
};

typedef union cvmx_pki_gbl_pen cvmx_pki_gbl_pen_t;

/**
 * cvmx_pki_gen_int
 */
union cvmx_pki_gen_int {
	u64 u64;
	struct cvmx_pki_gen_int_s {
		u64 reserved_10_63 : 54;
		u64 bufs_oflow : 1;
		u64 pkt_size_oflow : 1;
		u64 x2p_req_ofl : 1;
		u64 drp_noavail : 1;
		u64 dat : 1;
		u64 eop : 1;
		u64 sop : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} s;
	struct cvmx_pki_gen_int_s cn73xx;
	struct cvmx_pki_gen_int_s cn78xx;
	struct cvmx_pki_gen_int_cn78xxp1 {
		u64 reserved_8_63 : 56;
		u64 x2p_req_ofl : 1;
		u64 drp_noavail : 1;
		u64 dat : 1;
		u64 eop : 1;
		u64 sop : 1;
		u64 bckprs : 1;
		u64 crcerr : 1;
		u64 pktdrp : 1;
	} cn78xxp1;
	struct cvmx_pki_gen_int_s cnf75xx;
};

typedef union cvmx_pki_gen_int cvmx_pki_gen_int_t;

/**
 * cvmx_pki_icg#_cfg
 *
 * This register configures the cluster group.
 *
 */
union cvmx_pki_icgx_cfg {
	u64 u64;
	struct cvmx_pki_icgx_cfg_s {
		u64 reserved_53_63 : 11;
		u64 maxipe_use : 5;
		u64 reserved_36_47 : 12;
		u64 clusters : 4;
		u64 reserved_27_31 : 5;
		u64 release_rqd : 1;
		u64 mlo : 1;
		u64 pena : 1;
		u64 timer : 12;
		u64 delay : 12;
	} s;
	struct cvmx_pki_icgx_cfg_s cn73xx;
	struct cvmx_pki_icgx_cfg_s cn78xx;
	struct cvmx_pki_icgx_cfg_s cn78xxp1;
	struct cvmx_pki_icgx_cfg_s cnf75xx;
};

typedef union cvmx_pki_icgx_cfg cvmx_pki_icgx_cfg_t;

/**
 * cvmx_pki_imem#
 */
union cvmx_pki_imemx {
	u64 u64;
	struct cvmx_pki_imemx_s {
		u64 data : 64;
	} s;
	struct cvmx_pki_imemx_s cn73xx;
	struct cvmx_pki_imemx_s cn78xx;
	struct cvmx_pki_imemx_s cn78xxp1;
	struct cvmx_pki_imemx_s cnf75xx;
};

typedef union cvmx_pki_imemx cvmx_pki_imemx_t;

/**
 * cvmx_pki_ltype#_map
 *
 * This register is the layer type map, indexed by PKI_LTYPE_E.
 *
 */
union cvmx_pki_ltypex_map {
	u64 u64;
	struct cvmx_pki_ltypex_map_s {
		u64 reserved_3_63 : 61;
		u64 beltype : 3;
	} s;
	struct cvmx_pki_ltypex_map_s cn73xx;
	struct cvmx_pki_ltypex_map_s cn78xx;
	struct cvmx_pki_ltypex_map_s cn78xxp1;
	struct cvmx_pki_ltypex_map_s cnf75xx;
};

typedef union cvmx_pki_ltypex_map cvmx_pki_ltypex_map_t;

/**
 * cvmx_pki_pbe_eco
 */
union cvmx_pki_pbe_eco {
	u64 u64;
	struct cvmx_pki_pbe_eco_s {
		u64 reserved_32_63 : 32;
		u64 eco_rw : 32;
	} s;
	struct cvmx_pki_pbe_eco_s cn73xx;
	struct cvmx_pki_pbe_eco_s cn78xx;
	struct cvmx_pki_pbe_eco_s cnf75xx;
};

typedef union cvmx_pki_pbe_eco cvmx_pki_pbe_eco_t;

/**
 * cvmx_pki_pcam_lookup
 *
 * For diagnostic use only, this register performs a PCAM lookup against the provided
 * cluster and PCAM instance and loads results into PKI_PCAM_RESULT.
 */
union cvmx_pki_pcam_lookup {
	u64 u64;
	struct cvmx_pki_pcam_lookup_s {
		u64 reserved_54_63 : 10;
		u64 cl : 2;
		u64 reserved_49_51 : 3;
		u64 pcam : 1;
		u64 term : 8;
		u64 style : 8;
		u64 data : 32;
	} s;
	struct cvmx_pki_pcam_lookup_s cn73xx;
	struct cvmx_pki_pcam_lookup_s cn78xx;
	struct cvmx_pki_pcam_lookup_s cn78xxp1;
	struct cvmx_pki_pcam_lookup_s cnf75xx;
};

typedef union cvmx_pki_pcam_lookup cvmx_pki_pcam_lookup_t;

/**
 * cvmx_pki_pcam_result
 *
 * For diagnostic use only, this register returns PCAM results for the most recent write to
 * PKI_PCAM_LOOKUP. The read will stall until the lookup is completed.
 * PKI_CL()_ECC_CTL[PCAM_EN] must be clear before accessing this register. Read stall
 * is implemented by delaying the PKI_PCAM_LOOKUP write acknowledge until the PCAM is
 * free and the lookup can be issued.
 */
union cvmx_pki_pcam_result {
	u64 u64;
	struct cvmx_pki_pcam_result_s {
		u64 reserved_41_63 : 23;
		u64 match : 1;
		u64 entry : 8;
		u64 result : 32;
	} s;
	struct cvmx_pki_pcam_result_cn73xx {
		u64 conflict : 1;
		u64 reserved_41_62 : 22;
		u64 match : 1;
		u64 entry : 8;
		u64 result : 32;
	} cn73xx;
	struct cvmx_pki_pcam_result_cn73xx cn78xx;
	struct cvmx_pki_pcam_result_cn73xx cn78xxp1;
	struct cvmx_pki_pcam_result_cn73xx cnf75xx;
};

typedef union cvmx_pki_pcam_result cvmx_pki_pcam_result_t;

/**
 * cvmx_pki_pfe_diag
 */
union cvmx_pki_pfe_diag {
	u64 u64;
	struct cvmx_pki_pfe_diag_s {
		u64 reserved_1_63 : 63;
		u64 bad_rid : 1;
	} s;
	struct cvmx_pki_pfe_diag_s cn73xx;
	struct cvmx_pki_pfe_diag_s cn78xx;
	struct cvmx_pki_pfe_diag_s cn78xxp1;
	struct cvmx_pki_pfe_diag_s cnf75xx;
};

typedef union cvmx_pki_pfe_diag cvmx_pki_pfe_diag_t;

/**
 * cvmx_pki_pfe_eco
 */
union cvmx_pki_pfe_eco {
	u64 u64;
	struct cvmx_pki_pfe_eco_s {
		u64 reserved_32_63 : 32;
		u64 eco_rw : 32;
	} s;
	struct cvmx_pki_pfe_eco_s cn73xx;
	struct cvmx_pki_pfe_eco_s cn78xx;
	struct cvmx_pki_pfe_eco_s cnf75xx;
};

typedef union cvmx_pki_pfe_eco cvmx_pki_pfe_eco_t;

/**
 * cvmx_pki_pix_clken
 */
union cvmx_pki_pix_clken {
	u64 u64;
	struct cvmx_pki_pix_clken_s {
		u64 reserved_17_63 : 47;
		u64 mech : 1;
		u64 reserved_4_15 : 12;
		u64 cls : 4;
	} s;
	struct cvmx_pki_pix_clken_s cn73xx;
	struct cvmx_pki_pix_clken_s cn78xx;
	struct cvmx_pki_pix_clken_s cn78xxp1;
	struct cvmx_pki_pix_clken_s cnf75xx;
};

typedef union cvmx_pki_pix_clken cvmx_pki_pix_clken_t;

/**
 * cvmx_pki_pix_diag
 */
union cvmx_pki_pix_diag {
	u64 u64;
	struct cvmx_pki_pix_diag_s {
		u64 reserved_4_63 : 60;
		u64 nosched : 4;
	} s;
	struct cvmx_pki_pix_diag_s cn73xx;
	struct cvmx_pki_pix_diag_s cn78xx;
	struct cvmx_pki_pix_diag_s cn78xxp1;
	struct cvmx_pki_pix_diag_s cnf75xx;
};

typedef union cvmx_pki_pix_diag cvmx_pki_pix_diag_t;

/**
 * cvmx_pki_pix_eco
 */
union cvmx_pki_pix_eco {
	u64 u64;
	struct cvmx_pki_pix_eco_s {
		u64 reserved_32_63 : 32;
		u64 eco_rw : 32;
	} s;
	struct cvmx_pki_pix_eco_s cn73xx;
	struct cvmx_pki_pix_eco_s cn78xx;
	struct cvmx_pki_pix_eco_s cnf75xx;
};

typedef union cvmx_pki_pix_eco cvmx_pki_pix_eco_t;

/**
 * cvmx_pki_pkind#_icgsel
 */
union cvmx_pki_pkindx_icgsel {
	u64 u64;
	struct cvmx_pki_pkindx_icgsel_s {
		u64 reserved_2_63 : 62;
		u64 icg : 2;
	} s;
	struct cvmx_pki_pkindx_icgsel_s cn73xx;
	struct cvmx_pki_pkindx_icgsel_s cn78xx;
	struct cvmx_pki_pkindx_icgsel_s cn78xxp1;
	struct cvmx_pki_pkindx_icgsel_s cnf75xx;
};

typedef union cvmx_pki_pkindx_icgsel cvmx_pki_pkindx_icgsel_t;

/**
 * cvmx_pki_pknd#_inb_stat0
 *
 * This register counts inbound statistics, indexed by pkind.
 *
 */
union cvmx_pki_pkndx_inb_stat0 {
	u64 u64;
	struct cvmx_pki_pkndx_inb_stat0_s {
		u64 reserved_48_63 : 16;
		u64 pkts : 48;
	} s;
	struct cvmx_pki_pkndx_inb_stat0_s cn73xx;
	struct cvmx_pki_pkndx_inb_stat0_s cn78xx;
	struct cvmx_pki_pkndx_inb_stat0_s cn78xxp1;
	struct cvmx_pki_pkndx_inb_stat0_s cnf75xx;
};

typedef union cvmx_pki_pkndx_inb_stat0 cvmx_pki_pkndx_inb_stat0_t;

/**
 * cvmx_pki_pknd#_inb_stat1
 *
 * This register counts inbound statistics, indexed by pkind.
 *
 */
union cvmx_pki_pkndx_inb_stat1 {
	u64 u64;
	struct cvmx_pki_pkndx_inb_stat1_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_pki_pkndx_inb_stat1_s cn73xx;
	struct cvmx_pki_pkndx_inb_stat1_s cn78xx;
	struct cvmx_pki_pkndx_inb_stat1_s cn78xxp1;
	struct cvmx_pki_pkndx_inb_stat1_s cnf75xx;
};

typedef union cvmx_pki_pkndx_inb_stat1 cvmx_pki_pkndx_inb_stat1_t;

/**
 * cvmx_pki_pknd#_inb_stat2
 *
 * This register counts inbound statistics, indexed by pkind.
 *
 */
union cvmx_pki_pkndx_inb_stat2 {
	u64 u64;
	struct cvmx_pki_pkndx_inb_stat2_s {
		u64 reserved_48_63 : 16;
		u64 errs : 48;
	} s;
	struct cvmx_pki_pkndx_inb_stat2_s cn73xx;
	struct cvmx_pki_pkndx_inb_stat2_s cn78xx;
	struct cvmx_pki_pkndx_inb_stat2_s cn78xxp1;
	struct cvmx_pki_pkndx_inb_stat2_s cnf75xx;
};

typedef union cvmx_pki_pkndx_inb_stat2 cvmx_pki_pkndx_inb_stat2_t;

/**
 * cvmx_pki_pkt_err
 */
union cvmx_pki_pkt_err {
	u64 u64;
	struct cvmx_pki_pkt_err_s {
		u64 reserved_7_63 : 57;
		u64 reasm : 7;
	} s;
	struct cvmx_pki_pkt_err_s cn73xx;
	struct cvmx_pki_pkt_err_s cn78xx;
	struct cvmx_pki_pkt_err_s cn78xxp1;
	struct cvmx_pki_pkt_err_s cnf75xx;
};

typedef union cvmx_pki_pkt_err cvmx_pki_pkt_err_t;

/**
 * cvmx_pki_ptag_avail
 *
 * For diagnostic use only.
 *
 */
union cvmx_pki_ptag_avail {
	u64 u64;
	struct cvmx_pki_ptag_avail_s {
		u64 reserved_8_63 : 56;
		u64 avail : 8;
	} s;
	struct cvmx_pki_ptag_avail_s cn73xx;
	struct cvmx_pki_ptag_avail_s cn78xx;
	struct cvmx_pki_ptag_avail_s cnf75xx;
};

typedef union cvmx_pki_ptag_avail cvmx_pki_ptag_avail_t;

/**
 * cvmx_pki_qpg_tbl#
 *
 * These registers are used by PKI BE to indirectly calculate the Portadd/Aura/Group
 * from the Diffsrv, HiGig or VLAN information as described in QPG. See also
 * PKI_QPG_TBLB().
 */
union cvmx_pki_qpg_tblx {
	u64 u64;
	struct cvmx_pki_qpg_tblx_s {
		u64 reserved_60_63 : 4;
		u64 padd : 12;
		u64 grptag_ok : 3;
		u64 reserved_42_44 : 3;
		u64 grp_ok : 10;
		u64 grptag_bad : 3;
		u64 reserved_26_28 : 3;
		u64 grp_bad : 10;
		u64 reserved_12_15 : 4;
		u64 aura_node : 2;
		u64 laura : 10;
	} s;
	struct cvmx_pki_qpg_tblx_s cn73xx;
	struct cvmx_pki_qpg_tblx_s cn78xx;
	struct cvmx_pki_qpg_tblx_s cn78xxp1;
	struct cvmx_pki_qpg_tblx_s cnf75xx;
};

typedef union cvmx_pki_qpg_tblx cvmx_pki_qpg_tblx_t;

/**
 * cvmx_pki_qpg_tblb#
 *
 * This register configures the QPG table. See also PKI_QPG_TBL().
 *
 */
union cvmx_pki_qpg_tblbx {
	u64 u64;
	struct cvmx_pki_qpg_tblbx_s {
		u64 reserved_10_63 : 54;
		u64 dstat_id : 10;
	} s;
	struct cvmx_pki_qpg_tblbx_s cn73xx;
	struct cvmx_pki_qpg_tblbx_s cn78xx;
	struct cvmx_pki_qpg_tblbx_s cnf75xx;
};

typedef union cvmx_pki_qpg_tblbx cvmx_pki_qpg_tblbx_t;

/**
 * cvmx_pki_reasm_sop#
 */
union cvmx_pki_reasm_sopx {
	u64 u64;
	struct cvmx_pki_reasm_sopx_s {
		u64 sop : 64;
	} s;
	struct cvmx_pki_reasm_sopx_s cn73xx;
	struct cvmx_pki_reasm_sopx_s cn78xx;
	struct cvmx_pki_reasm_sopx_s cn78xxp1;
	struct cvmx_pki_reasm_sopx_s cnf75xx;
};

typedef union cvmx_pki_reasm_sopx cvmx_pki_reasm_sopx_t;

/**
 * cvmx_pki_req_wgt
 *
 * This register controls the round-robin weights between each PKI requestor. For diagnostic
 * tuning only.
 */
union cvmx_pki_req_wgt {
	u64 u64;
	struct cvmx_pki_req_wgt_s {
		u64 reserved_36_63 : 28;
		u64 wgt8 : 4;
		u64 wgt7 : 4;
		u64 wgt6 : 4;
		u64 wgt5 : 4;
		u64 wgt4 : 4;
		u64 wgt3 : 4;
		u64 wgt2 : 4;
		u64 wgt1 : 4;
		u64 wgt0 : 4;
	} s;
	struct cvmx_pki_req_wgt_s cn73xx;
	struct cvmx_pki_req_wgt_s cn78xx;
	struct cvmx_pki_req_wgt_s cn78xxp1;
	struct cvmx_pki_req_wgt_s cnf75xx;
};

typedef union cvmx_pki_req_wgt cvmx_pki_req_wgt_t;

/**
 * cvmx_pki_sft_rst
 */
union cvmx_pki_sft_rst {
	u64 u64;
	struct cvmx_pki_sft_rst_s {
		u64 busy : 1;
		u64 reserved_33_62 : 30;
		u64 active : 1;
		u64 reserved_1_31 : 31;
		u64 rst : 1;
	} s;
	struct cvmx_pki_sft_rst_s cn73xx;
	struct cvmx_pki_sft_rst_s cn78xx;
	struct cvmx_pki_sft_rst_s cn78xxp1;
	struct cvmx_pki_sft_rst_s cnf75xx;
};

typedef union cvmx_pki_sft_rst cvmx_pki_sft_rst_t;

/**
 * cvmx_pki_stat#_hist0
 */
union cvmx_pki_statx_hist0 {
	u64 u64;
	struct cvmx_pki_statx_hist0_s {
		u64 reserved_48_63 : 16;
		u64 h1to63 : 48;
	} s;
	struct cvmx_pki_statx_hist0_s cn73xx;
	struct cvmx_pki_statx_hist0_s cn78xx;
	struct cvmx_pki_statx_hist0_s cn78xxp1;
	struct cvmx_pki_statx_hist0_s cnf75xx;
};

typedef union cvmx_pki_statx_hist0 cvmx_pki_statx_hist0_t;

/**
 * cvmx_pki_stat#_hist1
 */
union cvmx_pki_statx_hist1 {
	u64 u64;
	struct cvmx_pki_statx_hist1_s {
		u64 reserved_48_63 : 16;
		u64 h64to127 : 48;
	} s;
	struct cvmx_pki_statx_hist1_s cn73xx;
	struct cvmx_pki_statx_hist1_s cn78xx;
	struct cvmx_pki_statx_hist1_s cn78xxp1;
	struct cvmx_pki_statx_hist1_s cnf75xx;
};

typedef union cvmx_pki_statx_hist1 cvmx_pki_statx_hist1_t;

/**
 * cvmx_pki_stat#_hist2
 */
union cvmx_pki_statx_hist2 {
	u64 u64;
	struct cvmx_pki_statx_hist2_s {
		u64 reserved_48_63 : 16;
		u64 h128to255 : 48;
	} s;
	struct cvmx_pki_statx_hist2_s cn73xx;
	struct cvmx_pki_statx_hist2_s cn78xx;
	struct cvmx_pki_statx_hist2_s cn78xxp1;
	struct cvmx_pki_statx_hist2_s cnf75xx;
};

typedef union cvmx_pki_statx_hist2 cvmx_pki_statx_hist2_t;

/**
 * cvmx_pki_stat#_hist3
 */
union cvmx_pki_statx_hist3 {
	u64 u64;
	struct cvmx_pki_statx_hist3_s {
		u64 reserved_48_63 : 16;
		u64 h256to511 : 48;
	} s;
	struct cvmx_pki_statx_hist3_s cn73xx;
	struct cvmx_pki_statx_hist3_s cn78xx;
	struct cvmx_pki_statx_hist3_s cn78xxp1;
	struct cvmx_pki_statx_hist3_s cnf75xx;
};

typedef union cvmx_pki_statx_hist3 cvmx_pki_statx_hist3_t;

/**
 * cvmx_pki_stat#_hist4
 */
union cvmx_pki_statx_hist4 {
	u64 u64;
	struct cvmx_pki_statx_hist4_s {
		u64 reserved_48_63 : 16;
		u64 h512to1023 : 48;
	} s;
	struct cvmx_pki_statx_hist4_s cn73xx;
	struct cvmx_pki_statx_hist4_s cn78xx;
	struct cvmx_pki_statx_hist4_s cn78xxp1;
	struct cvmx_pki_statx_hist4_s cnf75xx;
};

typedef union cvmx_pki_statx_hist4 cvmx_pki_statx_hist4_t;

/**
 * cvmx_pki_stat#_hist5
 */
union cvmx_pki_statx_hist5 {
	u64 u64;
	struct cvmx_pki_statx_hist5_s {
		u64 reserved_48_63 : 16;
		u64 h1024to1518 : 48;
	} s;
	struct cvmx_pki_statx_hist5_s cn73xx;
	struct cvmx_pki_statx_hist5_s cn78xx;
	struct cvmx_pki_statx_hist5_s cn78xxp1;
	struct cvmx_pki_statx_hist5_s cnf75xx;
};

typedef union cvmx_pki_statx_hist5 cvmx_pki_statx_hist5_t;

/**
 * cvmx_pki_stat#_hist6
 */
union cvmx_pki_statx_hist6 {
	u64 u64;
	struct cvmx_pki_statx_hist6_s {
		u64 reserved_48_63 : 16;
		u64 h1519 : 48;
	} s;
	struct cvmx_pki_statx_hist6_s cn73xx;
	struct cvmx_pki_statx_hist6_s cn78xx;
	struct cvmx_pki_statx_hist6_s cn78xxp1;
	struct cvmx_pki_statx_hist6_s cnf75xx;
};

typedef union cvmx_pki_statx_hist6 cvmx_pki_statx_hist6_t;

/**
 * cvmx_pki_stat#_stat0
 */
union cvmx_pki_statx_stat0 {
	u64 u64;
	struct cvmx_pki_statx_stat0_s {
		u64 reserved_48_63 : 16;
		u64 pkts : 48;
	} s;
	struct cvmx_pki_statx_stat0_s cn73xx;
	struct cvmx_pki_statx_stat0_s cn78xx;
	struct cvmx_pki_statx_stat0_s cn78xxp1;
	struct cvmx_pki_statx_stat0_s cnf75xx;
};

typedef union cvmx_pki_statx_stat0 cvmx_pki_statx_stat0_t;

/**
 * cvmx_pki_stat#_stat1
 */
union cvmx_pki_statx_stat1 {
	u64 u64;
	struct cvmx_pki_statx_stat1_s {
		u64 reserved_48_63 : 16;
		u64 octs : 48;
	} s;
	struct cvmx_pki_statx_stat1_s cn73xx;
	struct cvmx_pki_statx_stat1_s cn78xx;
	struct cvmx_pki_statx_stat1_s cn78xxp1;
	struct cvmx_pki_statx_stat1_s cnf75xx;
};

typedef union cvmx_pki_statx_stat1 cvmx_pki_statx_stat1_t;

/**
 * cvmx_pki_stat#_stat10
 */
union cvmx_pki_statx_stat10 {
	u64 u64;
	struct cvmx_pki_statx_stat10_s {
		u64 reserved_48_63 : 16;
		u64 jabber : 48;
	} s;
	struct cvmx_pki_statx_stat10_s cn73xx;
	struct cvmx_pki_statx_stat10_s cn78xx;
	struct cvmx_pki_statx_stat10_s cn78xxp1;
	struct cvmx_pki_statx_stat10_s cnf75xx;
};

typedef union cvmx_pki_statx_stat10 cvmx_pki_statx_stat10_t;

/**
 * cvmx_pki_stat#_stat11
 */
union cvmx_pki_statx_stat11 {
	u64 u64;
	struct cvmx_pki_statx_stat11_s {
		u64 reserved_48_63 : 16;
		u64 oversz : 48;
	} s;
	struct cvmx_pki_statx_stat11_s cn73xx;
	struct cvmx_pki_statx_stat11_s cn78xx;
	struct cvmx_pki_statx_stat11_s cn78xxp1;
	struct cvmx_pki_statx_stat11_s cnf75xx;
};

typedef union cvmx_pki_statx_stat11 cvmx_pki_statx_stat11_t;

/**
 * cvmx_pki_stat#_stat12
 */
union cvmx_pki_statx_stat12 {
	u64 u64;
	struct cvmx_pki_statx_stat12_s {
		u64 reserved_48_63 : 16;
		u64 l2err : 48;
	} s;
	struct cvmx_pki_statx_stat12_s cn73xx;
	struct cvmx_pki_statx_stat12_s cn78xx;
	struct cvmx_pki_statx_stat12_s cn78xxp1;
	struct cvmx_pki_statx_stat12_s cnf75xx;
};

typedef union cvmx_pki_statx_stat12 cvmx_pki_statx_stat12_t;

/**
 * cvmx_pki_stat#_stat13
 */
union cvmx_pki_statx_stat13 {
	u64 u64;
	struct cvmx_pki_statx_stat13_s {
		u64 reserved_48_63 : 16;
		u64 spec : 48;
	} s;
	struct cvmx_pki_statx_stat13_s cn73xx;
	struct cvmx_pki_statx_stat13_s cn78xx;
	struct cvmx_pki_statx_stat13_s cn78xxp1;
	struct cvmx_pki_statx_stat13_s cnf75xx;
};

typedef union cvmx_pki_statx_stat13 cvmx_pki_statx_stat13_t;

/**
 * cvmx_pki_stat#_stat14
 */
union cvmx_pki_statx_stat14 {
	u64 u64;
	struct cvmx_pki_statx_stat14_s {
		u64 reserved_48_63 : 16;
		u64 drp_bcast : 48;
	} s;
	struct cvmx_pki_statx_stat14_s cn73xx;
	struct cvmx_pki_statx_stat14_s cn78xx;
	struct cvmx_pki_statx_stat14_s cn78xxp1;
	struct cvmx_pki_statx_stat14_s cnf75xx;
};

typedef union cvmx_pki_statx_stat14 cvmx_pki_statx_stat14_t;

/**
 * cvmx_pki_stat#_stat15
 */
union cvmx_pki_statx_stat15 {
	u64 u64;
	struct cvmx_pki_statx_stat15_s {
		u64 reserved_48_63 : 16;
		u64 drp_mcast : 48;
	} s;
	struct cvmx_pki_statx_stat15_s cn73xx;
	struct cvmx_pki_statx_stat15_s cn78xx;
	struct cvmx_pki_statx_stat15_s cn78xxp1;
	struct cvmx_pki_statx_stat15_s cnf75xx;
};

typedef union cvmx_pki_statx_stat15 cvmx_pki_statx_stat15_t;

/**
 * cvmx_pki_stat#_stat16
 */
union cvmx_pki_statx_stat16 {
	u64 u64;
	struct cvmx_pki_statx_stat16_s {
		u64 reserved_48_63 : 16;
		u64 drp_bcast : 48;
	} s;
	struct cvmx_pki_statx_stat16_s cn73xx;
	struct cvmx_pki_statx_stat16_s cn78xx;
	struct cvmx_pki_statx_stat16_s cn78xxp1;
	struct cvmx_pki_statx_stat16_s cnf75xx;
};

typedef union cvmx_pki_statx_stat16 cvmx_pki_statx_stat16_t;

/**
 * cvmx_pki_stat#_stat17
 */
union cvmx_pki_statx_stat17 {
	u64 u64;
	struct cvmx_pki_statx_stat17_s {
		u64 reserved_48_63 : 16;
		u64 drp_mcast : 48;
	} s;
	struct cvmx_pki_statx_stat17_s cn73xx;
	struct cvmx_pki_statx_stat17_s cn78xx;
	struct cvmx_pki_statx_stat17_s cn78xxp1;
	struct cvmx_pki_statx_stat17_s cnf75xx;
};

typedef union cvmx_pki_statx_stat17 cvmx_pki_statx_stat17_t;

/**
 * cvmx_pki_stat#_stat18
 */
union cvmx_pki_statx_stat18 {
	u64 u64;
	struct cvmx_pki_statx_stat18_s {
		u64 reserved_48_63 : 16;
		u64 drp_spec : 48;
	} s;
	struct cvmx_pki_statx_stat18_s cn73xx;
	struct cvmx_pki_statx_stat18_s cn78xx;
	struct cvmx_pki_statx_stat18_s cn78xxp1;
	struct cvmx_pki_statx_stat18_s cnf75xx;
};

typedef union cvmx_pki_statx_stat18 cvmx_pki_statx_stat18_t;

/**
 * cvmx_pki_stat#_stat2
 */
union cvmx_pki_statx_stat2 {
	u64 u64;
	struct cvmx_pki_statx_stat2_s {
		u64 reserved_48_63 : 16;
		u64 raw : 48;
	} s;
	struct cvmx_pki_statx_stat2_s cn73xx;
	struct cvmx_pki_statx_stat2_s cn78xx;
	struct cvmx_pki_statx_stat2_s cn78xxp1;
	struct cvmx_pki_statx_stat2_s cnf75xx;
};

typedef union cvmx_pki_statx_stat2 cvmx_pki_statx_stat2_t;

/**
 * cvmx_pki_stat#_stat3
 */
union cvmx_pki_statx_stat3 {
	u64 u64;
	struct cvmx_pki_statx_stat3_s {
		u64 reserved_48_63 : 16;
		u64 drp_pkts : 48;
	} s;
	struct cvmx_pki_statx_stat3_s cn73xx;
	struct cvmx_pki_statx_stat3_s cn78xx;
	struct cvmx_pki_statx_stat3_s cn78xxp1;
	struct cvmx_pki_statx_stat3_s cnf75xx;
};

typedef union cvmx_pki_statx_stat3 cvmx_pki_statx_stat3_t;

/**
 * cvmx_pki_stat#_stat4
 */
union cvmx_pki_statx_stat4 {
	u64 u64;
	struct cvmx_pki_statx_stat4_s {
		u64 reserved_48_63 : 16;
		u64 drp_octs : 48;
	} s;
	struct cvmx_pki_statx_stat4_s cn73xx;
	struct cvmx_pki_statx_stat4_s cn78xx;
	struct cvmx_pki_statx_stat4_s cn78xxp1;
	struct cvmx_pki_statx_stat4_s cnf75xx;
};

typedef union cvmx_pki_statx_stat4 cvmx_pki_statx_stat4_t;

/**
 * cvmx_pki_stat#_stat5
 */
union cvmx_pki_statx_stat5 {
	u64 u64;
	struct cvmx_pki_statx_stat5_s {
		u64 reserved_48_63 : 16;
		u64 bcast : 48;
	} s;
	struct cvmx_pki_statx_stat5_s cn73xx;
	struct cvmx_pki_statx_stat5_s cn78xx;
	struct cvmx_pki_statx_stat5_s cn78xxp1;
	struct cvmx_pki_statx_stat5_s cnf75xx;
};

typedef union cvmx_pki_statx_stat5 cvmx_pki_statx_stat5_t;

/**
 * cvmx_pki_stat#_stat6
 */
union cvmx_pki_statx_stat6 {
	u64 u64;
	struct cvmx_pki_statx_stat6_s {
		u64 reserved_48_63 : 16;
		u64 mcast : 48;
	} s;
	struct cvmx_pki_statx_stat6_s cn73xx;
	struct cvmx_pki_statx_stat6_s cn78xx;
	struct cvmx_pki_statx_stat6_s cn78xxp1;
	struct cvmx_pki_statx_stat6_s cnf75xx;
};

typedef union cvmx_pki_statx_stat6 cvmx_pki_statx_stat6_t;

/**
 * cvmx_pki_stat#_stat7
 */
union cvmx_pki_statx_stat7 {
	u64 u64;
	struct cvmx_pki_statx_stat7_s {
		u64 reserved_48_63 : 16;
		u64 fcs : 48;
	} s;
	struct cvmx_pki_statx_stat7_s cn73xx;
	struct cvmx_pki_statx_stat7_s cn78xx;
	struct cvmx_pki_statx_stat7_s cn78xxp1;
	struct cvmx_pki_statx_stat7_s cnf75xx;
};

typedef union cvmx_pki_statx_stat7 cvmx_pki_statx_stat7_t;

/**
 * cvmx_pki_stat#_stat8
 */
union cvmx_pki_statx_stat8 {
	u64 u64;
	struct cvmx_pki_statx_stat8_s {
		u64 reserved_48_63 : 16;
		u64 frag : 48;
	} s;
	struct cvmx_pki_statx_stat8_s cn73xx;
	struct cvmx_pki_statx_stat8_s cn78xx;
	struct cvmx_pki_statx_stat8_s cn78xxp1;
	struct cvmx_pki_statx_stat8_s cnf75xx;
};

typedef union cvmx_pki_statx_stat8 cvmx_pki_statx_stat8_t;

/**
 * cvmx_pki_stat#_stat9
 */
union cvmx_pki_statx_stat9 {
	u64 u64;
	struct cvmx_pki_statx_stat9_s {
		u64 reserved_48_63 : 16;
		u64 undersz : 48;
	} s;
	struct cvmx_pki_statx_stat9_s cn73xx;
	struct cvmx_pki_statx_stat9_s cn78xx;
	struct cvmx_pki_statx_stat9_s cn78xxp1;
	struct cvmx_pki_statx_stat9_s cnf75xx;
};

typedef union cvmx_pki_statx_stat9 cvmx_pki_statx_stat9_t;

/**
 * cvmx_pki_stat_ctl
 *
 * This register controls how the PKI statistics counters are handled.
 *
 */
union cvmx_pki_stat_ctl {
	u64 u64;
	struct cvmx_pki_stat_ctl_s {
		u64 reserved_2_63 : 62;
		u64 mode : 2;
	} s;
	struct cvmx_pki_stat_ctl_s cn73xx;
	struct cvmx_pki_stat_ctl_s cn78xx;
	struct cvmx_pki_stat_ctl_s cn78xxp1;
	struct cvmx_pki_stat_ctl_s cnf75xx;
};

typedef union cvmx_pki_stat_ctl cvmx_pki_stat_ctl_t;

/**
 * cvmx_pki_style#_buf
 *
 * This register configures the PKI BE skip amounts and other information.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_buf {
	u64 u64;
	struct cvmx_pki_stylex_buf_s {
		u64 reserved_33_63 : 31;
		u64 pkt_lend : 1;
		u64 wqe_hsz : 2;
		u64 wqe_skip : 2;
		u64 first_skip : 6;
		u64 later_skip : 6;
		u64 opc_mode : 2;
		u64 dis_wq_dat : 1;
		u64 mb_size : 13;
	} s;
	struct cvmx_pki_stylex_buf_s cn73xx;
	struct cvmx_pki_stylex_buf_s cn78xx;
	struct cvmx_pki_stylex_buf_s cn78xxp1;
	struct cvmx_pki_stylex_buf_s cnf75xx;
};

typedef union cvmx_pki_stylex_buf cvmx_pki_stylex_buf_t;

/**
 * cvmx_pki_style#_tag_mask
 *
 * This register configures the PKI BE tag algorithm.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_tag_mask {
	u64 u64;
	struct cvmx_pki_stylex_tag_mask_s {
		u64 reserved_16_63 : 48;
		u64 mask : 16;
	} s;
	struct cvmx_pki_stylex_tag_mask_s cn73xx;
	struct cvmx_pki_stylex_tag_mask_s cn78xx;
	struct cvmx_pki_stylex_tag_mask_s cn78xxp1;
	struct cvmx_pki_stylex_tag_mask_s cnf75xx;
};

typedef union cvmx_pki_stylex_tag_mask cvmx_pki_stylex_tag_mask_t;

/**
 * cvmx_pki_style#_tag_sel
 *
 * This register configures the PKI BE tag algorithm.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_tag_sel {
	u64 u64;
	struct cvmx_pki_stylex_tag_sel_s {
		u64 reserved_27_63 : 37;
		u64 tag_idx3 : 3;
		u64 reserved_19_23 : 5;
		u64 tag_idx2 : 3;
		u64 reserved_11_15 : 5;
		u64 tag_idx1 : 3;
		u64 reserved_3_7 : 5;
		u64 tag_idx0 : 3;
	} s;
	struct cvmx_pki_stylex_tag_sel_s cn73xx;
	struct cvmx_pki_stylex_tag_sel_s cn78xx;
	struct cvmx_pki_stylex_tag_sel_s cn78xxp1;
	struct cvmx_pki_stylex_tag_sel_s cnf75xx;
};

typedef union cvmx_pki_stylex_tag_sel cvmx_pki_stylex_tag_sel_t;

/**
 * cvmx_pki_style#_wq2
 *
 * This register configures the PKI BE WQE generation.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_wq2 {
	u64 u64;
	struct cvmx_pki_stylex_wq2_s {
		u64 data : 64;
	} s;
	struct cvmx_pki_stylex_wq2_s cn73xx;
	struct cvmx_pki_stylex_wq2_s cn78xx;
	struct cvmx_pki_stylex_wq2_s cn78xxp1;
	struct cvmx_pki_stylex_wq2_s cnf75xx;
};

typedef union cvmx_pki_stylex_wq2 cvmx_pki_stylex_wq2_t;

/**
 * cvmx_pki_style#_wq4
 *
 * This register configures the PKI BE WQE generation.
 * It is indexed by final style, PKI_WQE_S[STYLE]<5:0>.
 */
union cvmx_pki_stylex_wq4 {
	u64 u64;
	struct cvmx_pki_stylex_wq4_s {
		u64 data : 64;
	} s;
	struct cvmx_pki_stylex_wq4_s cn73xx;
	struct cvmx_pki_stylex_wq4_s cn78xx;
	struct cvmx_pki_stylex_wq4_s cn78xxp1;
	struct cvmx_pki_stylex_wq4_s cnf75xx;
};

typedef union cvmx_pki_stylex_wq4 cvmx_pki_stylex_wq4_t;

/**
 * cvmx_pki_tag_inc#_ctl
 */
union cvmx_pki_tag_incx_ctl {
	u64 u64;
	struct cvmx_pki_tag_incx_ctl_s {
		u64 reserved_12_63 : 52;
		u64 ptr_sel : 4;
		u64 offset : 8;
	} s;
	struct cvmx_pki_tag_incx_ctl_s cn73xx;
	struct cvmx_pki_tag_incx_ctl_s cn78xx;
	struct cvmx_pki_tag_incx_ctl_s cn78xxp1;
	struct cvmx_pki_tag_incx_ctl_s cnf75xx;
};

typedef union cvmx_pki_tag_incx_ctl cvmx_pki_tag_incx_ctl_t;

/**
 * cvmx_pki_tag_inc#_mask
 */
union cvmx_pki_tag_incx_mask {
	u64 u64;
	struct cvmx_pki_tag_incx_mask_s {
		u64 en : 64;
	} s;
	struct cvmx_pki_tag_incx_mask_s cn73xx;
	struct cvmx_pki_tag_incx_mask_s cn78xx;
	struct cvmx_pki_tag_incx_mask_s cn78xxp1;
	struct cvmx_pki_tag_incx_mask_s cnf75xx;
};

typedef union cvmx_pki_tag_incx_mask cvmx_pki_tag_incx_mask_t;

/**
 * cvmx_pki_tag_secret
 *
 * The source and destination initial values (IVs) in tag generation provide a mechanism for
 * seeding with a random initialization value to reduce cache collision attacks.
 */
union cvmx_pki_tag_secret {
	u64 u64;
	struct cvmx_pki_tag_secret_s {
		u64 dst6 : 16;
		u64 src6 : 16;
		u64 dst : 16;
		u64 src : 16;
	} s;
	struct cvmx_pki_tag_secret_s cn73xx;
	struct cvmx_pki_tag_secret_s cn78xx;
	struct cvmx_pki_tag_secret_s cn78xxp1;
	struct cvmx_pki_tag_secret_s cnf75xx;
};

typedef union cvmx_pki_tag_secret cvmx_pki_tag_secret_t;

/**
 * cvmx_pki_x2p_req_ofl
 */
union cvmx_pki_x2p_req_ofl {
	u64 u64;
	struct cvmx_pki_x2p_req_ofl_s {
		u64 reserved_4_63 : 60;
		u64 x2p_did : 4;
	} s;
	struct cvmx_pki_x2p_req_ofl_s cn73xx;
	struct cvmx_pki_x2p_req_ofl_s cn78xx;
	struct cvmx_pki_x2p_req_ofl_s cn78xxp1;
	struct cvmx_pki_x2p_req_ofl_s cnf75xx;
};

typedef union cvmx_pki_x2p_req_ofl cvmx_pki_x2p_req_ofl_t;

#endif
