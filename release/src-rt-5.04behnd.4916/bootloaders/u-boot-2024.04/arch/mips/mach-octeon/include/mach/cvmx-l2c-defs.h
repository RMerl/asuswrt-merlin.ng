/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_L2C_DEFS_H_
#define __CVMX_L2C_DEFS_H_

#define CVMX_L2C_CFG 0x0001180080000000ull
#define CVMX_L2C_CTL 0x0001180080800000ull

/*
 * Mapping is done starting from 0x11800.80000000
 * Use _REL for relative mapping
 */
#define CVMX_L2C_CTL_REL	 0x00800000
#define CVMX_L2C_BIG_CTL_REL	 0x00800030
#define CVMX_L2C_TADX_INT_REL(i) (0x00a00028 + (((i) & 7) * 0x40000))
#define CVMX_L2C_MCIX_INT_REL(i) (0x00c00028 + (((i) & 3) * 0x40000))

/**
 * cvmx_l2c_cfg
 *
 * Specify the RSL base addresses for the block
 *
 *                  L2C_CFG = L2C Configuration
 *
 * Description:
 */
union cvmx_l2c_cfg {
	u64 u64;
	struct cvmx_l2c_cfg_s {
		u64 reserved_20_63 : 44;
		u64 bstrun : 1;
		u64 lbist : 1;
		u64 xor_bank : 1;
		u64 dpres1 : 1;
		u64 dpres0 : 1;
		u64 dfill_dis : 1;
		u64 fpexp : 4;
		u64 fpempty : 1;
		u64 fpen : 1;
		u64 idxalias : 1;
		u64 mwf_crd : 4;
		u64 rsp_arb_mode : 1;
		u64 rfb_arb_mode : 1;
		u64 lrf_arb_mode : 1;
	} s;
};

/**
 * cvmx_l2c_ctl
 *
 * L2C_CTL = L2C Control
 *
 *
 * Notes:
 * (1) If MAXVAB is != 0, VAB_THRESH should be less than MAXVAB.
 *
 * (2) L2DFDBE and L2DFSBE allows software to generate L2DSBE, L2DDBE, VBFSBE,
 * and VBFDBE errors for the purposes of testing error handling code.  When
 * one (or both) of these bits are set a PL2 which misses in the L2 will fill
 * with the appropriate error in the first 2 OWs of the fill. Software can
 * determine which OW pair gets the error by choosing the desired fill order
 * (address<6:5>).  A PL2 which hits in the L2 will not inject any errors.
 * Therefore sending a WBIL2 prior to the PL2 is recommended to make a miss
 * likely (if multiple processors are involved software must be careful to be
 * sure no other processor or IO device can bring the block into the L2).
 *
 * To generate a VBFSBE or VBFDBE, software must first get the cache block
 * into the cache with an error using a PL2 which misses the L2.  Then a
 * store partial to a portion of the cache block without the error must
 * change the block to dirty.  Then, a subsequent WBL2/WBIL2/victim will
 * trigger the VBFSBE/VBFDBE error.
 */
union cvmx_l2c_ctl {
	u64 u64;
	struct cvmx_l2c_ctl_s {
		u64 reserved_29_63 : 35;
		u64 rdf_fast : 1;
		u64 disstgl2i : 1;
		u64 l2dfsbe : 1;
		u64 l2dfdbe : 1;
		u64 discclk : 1;
		u64 maxvab : 4;
		u64 maxlfb : 4;
		u64 rsp_arb_mode : 1;
		u64 xmc_arb_mode : 1;
		u64 reserved_2_13 : 12;
		u64 disecc : 1;
		u64 disidxalias : 1;
	} s;

	struct cvmx_l2c_ctl_cn73xx {
		u64 reserved_32_63 : 32;
		u64 ocla_qos : 3;
		u64 reserved_28_28 : 1;
		u64 disstgl2i : 1;
		u64 reserved_25_26 : 2;
		u64 discclk : 1;
		u64 reserved_16_23 : 8;
		u64 rsp_arb_mode : 1;
		u64 xmc_arb_mode : 1;
		u64 rdf_cnt : 8;
		u64 reserved_4_5 : 2;
		u64 disldwb : 1;
		u64 dissblkdty : 1;
		u64 disecc : 1;
		u64 disidxalias : 1;
	} cn73xx;

	struct cvmx_l2c_ctl_cn73xx cn78xx;
};

/**
 * cvmx_l2c_big_ctl
 *
 * L2C_BIG_CTL = L2C Big memory control register
 *
 *
 * Notes:
 * (1) BIGRD interrupts can occur during normal operation as the PP's are
 * allowed to prefetch to non-existent memory locations.  Therefore,
 * BIGRD is for informational purposes only.
 *
 * (2) When HOLEWR/BIGWR blocks a store L2C_VER_ID, L2C_VER_PP, L2C_VER_IOB,
 * and L2C_VER_MSC will be loaded just like a store which is blocked by VRTWR.
 * Additionally, L2C_ERR_XMC will be loaded.
 */
union cvmx_l2c_big_ctl {
	u64 u64;
	struct cvmx_l2c_big_ctl_s {
		u64 reserved_8_63 : 56;
		u64 maxdram : 4;
		u64 reserved_0_3 : 4;
	} s;
	struct cvmx_l2c_big_ctl_cn61xx {
		u64 reserved_8_63 : 56;
		u64 maxdram : 4;
		u64 reserved_1_3 : 3;
		u64 disable : 1;
	} cn61xx;
	struct cvmx_l2c_big_ctl_cn61xx cn63xx;
	struct cvmx_l2c_big_ctl_cn61xx cn66xx;
	struct cvmx_l2c_big_ctl_cn61xx cn68xx;
	struct cvmx_l2c_big_ctl_cn61xx cn68xxp1;
	struct cvmx_l2c_big_ctl_cn70xx {
		u64 reserved_8_63 : 56;
		u64 maxdram : 4;
		u64 reserved_1_3 : 3;
		u64 disbig : 1;
	} cn70xx;
	struct cvmx_l2c_big_ctl_cn70xx cn70xxp1;
	struct cvmx_l2c_big_ctl_cn70xx cn73xx;
	struct cvmx_l2c_big_ctl_cn70xx cn78xx;
	struct cvmx_l2c_big_ctl_cn70xx cn78xxp1;
	struct cvmx_l2c_big_ctl_cn61xx cnf71xx;
	struct cvmx_l2c_big_ctl_cn70xx cnf75xx;
};

struct rlevel_byte_data {
	int delay;
	int loop_total;
	int loop_count;
	int best;
	u64 bm;
	int bmerrs;
	int sqerrs;
	int bestsq;
};

#endif
