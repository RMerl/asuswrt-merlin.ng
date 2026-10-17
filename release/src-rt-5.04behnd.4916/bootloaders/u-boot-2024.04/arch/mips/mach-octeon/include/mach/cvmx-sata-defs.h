/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_SATA_DEFS_H__
#define __CVMX_SATA_DEFS_H__

#define CVMX_SATA_UCTL_CTL	   (0x000118006C000000ull)
#define CVMX_SATA_UCTL_SHIM_CFG	   (0x000118006C0000E8ull)
#define CVMX_SATA_UCTL_BIST_STATUS (0x000118006C000008ull)

#define CVMX_SATA_UAHC_GBL_PI	    (0x00016C000000000Cull)
#define CVMX_SATA_UAHC_GBL_TIMER1MS (0x00016C00000000E0ull)
#define CVMX_SATA_UAHC_GBL_CAP	    (0x00016C0000000000ull)

#define CVMX_SATA_UAHC_PX_CMD(offset)  (0x00016C0000000118ull + ((offset) & 1) * 128)
#define CVMX_SATA_UAHC_PX_SCTL(offset) (0x00016C000000012Cull + ((offset) & 1) * 128)
#define CVMX_SATA_UAHC_PX_SERR(offset) (0x00016C0000000130ull + ((offset) & 1) * 128)
#define CVMX_SATA_UAHC_PX_IS(offset)   (0x00016C0000000110ull + ((offset) & 1) * 128)
#define CVMX_SATA_UAHC_PX_SSTS(offset) (0x00016C0000000128ull + ((offset) & 1) * 128)
#define CVMX_SATA_UAHC_PX_TFD(offset)  (0x00016C0000000120ull + ((offset) & 1) * 128)

/**
 * cvmx_sata_uctl_ctl
 *
 * This register controls clocks, resets, power, and BIST for the SATA.
 *
 * Accessible always.
 *
 * Reset by IOI reset.
 */
union cvmx_sata_uctl_ctl {
	u64 u64;
	struct cvmx_sata_uctl_ctl_s {
		u64 clear_bist : 1;
		u64 start_bist : 1;
		u64 reserved_31_61 : 31;
		u64 a_clk_en : 1;
		u64 a_clk_byp_sel : 1;
		u64 a_clkdiv_rst : 1;
		u64 reserved_27_27 : 1;
		u64 a_clkdiv_sel : 3;
		u64 reserved_5_23 : 19;
		u64 csclk_en : 1;
		u64 reserved_2_3 : 2;
		u64 sata_uahc_rst : 1;
		u64 sata_uctl_rst : 1;
	} s;
	struct cvmx_sata_uctl_ctl_s cn70xx;
	struct cvmx_sata_uctl_ctl_s cn70xxp1;
	struct cvmx_sata_uctl_ctl_s cn73xx;
};

typedef union cvmx_sata_uctl_ctl cvmx_sata_uctl_ctl_t;

/**
 * cvmx_sata_uctl_bist_status
 *
 * Results from BIST runs of SATA's memories.
 * Wait for NDONE==0, then look at defect indication.
 *
 * Accessible always.
 *
 * Reset by IOI reset.
 */
union cvmx_sata_uctl_bist_status {
	u64 u64;
	struct cvmx_sata_uctl_bist_status_s {
		u64 reserved_42_63 : 22;
		u64 uctl_xm_r_bist_ndone : 1;
		u64 uctl_xm_w_bist_ndone : 1;
		u64 reserved_36_39 : 4;
		u64 uahc_p0_rxram_bist_ndone : 1;
		u64 uahc_p1_rxram_bist_ndone : 1;
		u64 uahc_p0_txram_bist_ndone : 1;
		u64 uahc_p1_txram_bist_ndone : 1;
		u64 reserved_10_31 : 22;
		u64 uctl_xm_r_bist_status : 1;
		u64 uctl_xm_w_bist_status : 1;
		u64 reserved_4_7 : 4;
		u64 uahc_p0_rxram_bist_status : 1;
		u64 uahc_p1_rxram_bist_status : 1;
		u64 uahc_p0_txram_bist_status : 1;
		u64 uahc_p1_txram_bist_status : 1;
	} s;
	struct cvmx_sata_uctl_bist_status_s cn70xx;
	struct cvmx_sata_uctl_bist_status_s cn70xxp1;
	struct cvmx_sata_uctl_bist_status_s cn73xx;
};

typedef union cvmx_sata_uctl_bist_status cvmx_sata_uctl_bist_status_t;

/**
 * cvmx_sata_uctl_shim_cfg
 *
 * This register allows configuration of various shim (UCTL) features.
 *
 * Fields XS_NCB_OOB_* are captured when there are no outstanding OOB errors indicated in INTSTAT
 * and a new OOB error arrives.
 *
 * Fields XS_BAD_DMA_* are captured when there are no outstanding DMA errors indicated in INTSTAT
 * and a new DMA error arrives.
 *
 * Accessible only when SATA_UCTL_CTL[A_CLK_EN].
 *
 * Reset by IOI reset or SATA_UCTL_CTL[SATA_UCTL_RST].
 */
union cvmx_sata_uctl_shim_cfg {
	u64 u64;
	struct cvmx_sata_uctl_shim_cfg_s {
		u64 xs_ncb_oob_wrn : 1;
		u64 reserved_60_62 : 3;
		u64 xs_ncb_oob_osrc : 12;
		u64 xm_bad_dma_wrn : 1;
		u64 reserved_44_46 : 3;
		u64 xm_bad_dma_type : 4;
		u64 reserved_14_39 : 26;
		u64 dma_read_cmd : 2;
		u64 reserved_11_11 : 1;
		u64 dma_write_cmd : 1;
		u64 dma_endian_mode : 2;
		u64 reserved_2_7 : 6;
		u64 csr_endian_mode : 2;
	} s;
	struct cvmx_sata_uctl_shim_cfg_cn70xx {
		u64 xs_ncb_oob_wrn : 1;
		u64 reserved_57_62 : 6;
		u64 xs_ncb_oob_osrc : 9;
		u64 xm_bad_dma_wrn : 1;
		u64 reserved_44_46 : 3;
		u64 xm_bad_dma_type : 4;
		u64 reserved_13_39 : 27;
		u64 dma_read_cmd : 1;
		u64 reserved_10_11 : 2;
		u64 dma_endian_mode : 2;
		u64 reserved_2_7 : 6;
		u64 csr_endian_mode : 2;
	} cn70xx;
	struct cvmx_sata_uctl_shim_cfg_cn70xx cn70xxp1;
	struct cvmx_sata_uctl_shim_cfg_s cn73xx;
};

typedef union cvmx_sata_uctl_shim_cfg cvmx_sata_uctl_shim_cfg_t;

/**
 * cvmx_sata_uahc_gbl_cap
 *
 * See AHCI specification v1.3 section 3.1
 *
 */
union cvmx_sata_uahc_gbl_cap {
	u32 u32;
	struct cvmx_sata_uahc_gbl_cap_s {
		u32 s64a : 1;
		u32 sncq : 1;
		u32 ssntf : 1;
		u32 smps : 1;
		u32 sss : 1;
		u32 salp : 1;
		u32 sal : 1;
		u32 sclo : 1;
		u32 iss : 4;
		u32 snzo : 1;
		u32 sam : 1;
		u32 spm : 1;
		u32 fbss : 1;
		u32 pmd : 1;
		u32 ssc : 1;
		u32 psc : 1;
		u32 ncs : 5;
		u32 cccs : 1;
		u32 ems : 1;
		u32 sxs : 1;
		u32 np : 5;
	} s;
	struct cvmx_sata_uahc_gbl_cap_s cn70xx;
	struct cvmx_sata_uahc_gbl_cap_s cn70xxp1;
	struct cvmx_sata_uahc_gbl_cap_s cn73xx;
};

typedef union cvmx_sata_uahc_gbl_cap cvmx_sata_uahc_gbl_cap_t;

/**
 * cvmx_sata_uahc_p#_sctl
 */
union cvmx_sata_uahc_px_sctl {
	u32 u32;
	struct cvmx_sata_uahc_px_sctl_s {
		u32 reserved_10_31 : 22;
		u32 ipm : 2;
		u32 reserved_6_7 : 2;
		u32 spd : 2;
		u32 reserved_3_3 : 1;
		u32 det : 3;
	} s;
	struct cvmx_sata_uahc_px_sctl_s cn70xx;
	struct cvmx_sata_uahc_px_sctl_s cn70xxp1;
	struct cvmx_sata_uahc_px_sctl_s cn73xx;
};

typedef union cvmx_sata_uahc_px_sctl cvmx_sata_uahc_px_sctl_t;

/**
 * cvmx_sata_uahc_p#_cmd
 */
union cvmx_sata_uahc_px_cmd {
	u32 u32;
	struct cvmx_sata_uahc_px_cmd_s {
		u32 icc : 4;
		u32 asp : 1;
		u32 alpe : 1;
		u32 dlae : 1;
		u32 atapi : 1;
		u32 apste : 1;
		u32 fbscp : 1;
		u32 esp : 1;
		u32 cpd : 1;
		u32 mpsp : 1;
		u32 hpcp : 1;
		u32 pma : 1;
		u32 cps : 1;
		u32 cr : 1;
		u32 fr : 1;
		u32 mpss : 1;
		u32 ccs : 5;
		u32 reserved_5_7 : 3;
		u32 fre : 1;
		u32 clo : 1;
		u32 pod : 1;
		u32 sud : 1;
		u32 st : 1;
	} s;
	struct cvmx_sata_uahc_px_cmd_s cn70xx;
	struct cvmx_sata_uahc_px_cmd_s cn70xxp1;
	struct cvmx_sata_uahc_px_cmd_s cn73xx;
};

typedef union cvmx_sata_uahc_px_cmd cvmx_sata_uahc_px_cmd_t;

/**
 * cvmx_sata_uahc_gbl_pi
 *
 * See AHCI specification v1.3 section 3.1.
 *
 */
union cvmx_sata_uahc_gbl_pi {
	u32 u32;
	struct cvmx_sata_uahc_gbl_pi_s {
		u32 reserved_2_31 : 30;
		u32 pi : 2;
	} s;
	struct cvmx_sata_uahc_gbl_pi_s cn70xx;
	struct cvmx_sata_uahc_gbl_pi_s cn70xxp1;
	struct cvmx_sata_uahc_gbl_pi_s cn73xx;
};

typedef union cvmx_sata_uahc_gbl_pi cvmx_sata_uahc_gbl_pi_t;

/**
 * cvmx_sata_uahc_p#_ssts
 */
union cvmx_sata_uahc_px_ssts {
	u32 u32;
	struct cvmx_sata_uahc_px_ssts_s {
		u32 reserved_12_31 : 20;
		u32 ipm : 4;
		u32 spd : 4;
		u32 det : 4;
	} s;
	struct cvmx_sata_uahc_px_ssts_s cn70xx;
	struct cvmx_sata_uahc_px_ssts_s cn70xxp1;
	struct cvmx_sata_uahc_px_ssts_s cn73xx;
};

typedef union cvmx_sata_uahc_px_ssts cvmx_sata_uahc_px_ssts_t;

/**
 * cvmx_sata_uahc_p#_tfd
 */
union cvmx_sata_uahc_px_tfd {
	u32 u32;
	struct cvmx_sata_uahc_px_tfd_s {
		u32 reserved_16_31 : 16;
		u32 tferr : 8;
		u32 sts : 8;
	} s;
	struct cvmx_sata_uahc_px_tfd_s cn70xx;
	struct cvmx_sata_uahc_px_tfd_s cn70xxp1;
	struct cvmx_sata_uahc_px_tfd_s cn73xx;
};

typedef union cvmx_sata_uahc_px_tfd cvmx_sata_uahc_px_tfd_t;

/**
 * cvmx_sata_uahc_gbl_timer1ms
 */
union cvmx_sata_uahc_gbl_timer1ms {
	u32 u32;
	struct cvmx_sata_uahc_gbl_timer1ms_s {
		u32 reserved_20_31 : 12;
		u32 timv : 20;
	} s;
	struct cvmx_sata_uahc_gbl_timer1ms_s cn70xx;
	struct cvmx_sata_uahc_gbl_timer1ms_s cn70xxp1;
	struct cvmx_sata_uahc_gbl_timer1ms_s cn73xx;
};

typedef union cvmx_sata_uahc_gbl_timer1ms cvmx_sata_uahc_gbl_timer1ms_t;

#endif
