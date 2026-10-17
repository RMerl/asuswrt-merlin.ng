// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <command.h>
#include <config.h>
#include <dm.h>
#include <hang.h>
#include <i2c.h>
#include <ram.h>
#include <time.h>
#include <asm/global_data.h>

#include <asm/sections.h>
#include <linux/io.h>

#include <mach/octeon_ddr.h>

#define CFG_REF_HERTZ	50000000

DECLARE_GLOBAL_DATA_PTR;

/* Sign of an integer */
static s64 _sign(s64 v)
{
	return (v < 0);
}

#ifndef DDR_NO_DEBUG
char *lookup_env(struct ddr_priv *priv, const char *format, ...)
{
	char *s;
	unsigned long value;
	va_list args;
	char buffer[64];

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	s = ddr_getenv_debug(priv, buffer);
	if (s) {
		value = simple_strtoul(s, NULL, 0);
		printf("Parameter found in environment %s=\"%s\" 0x%lx (%ld)\n",
		       buffer, s, value, value);
	}

	return s;
}

char *lookup_env_ull(struct ddr_priv *priv, const char *format, ...)
{
	char *s;
	u64 value;
	va_list args;
	char buffer[64];

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	s = ddr_getenv_debug(priv, buffer);
	if (s) {
		value = simple_strtoull(s, NULL, 0);
		printf("Parameter found in environment. %s = 0x%016llx\n",
		       buffer, value);
	}

	return s;
}
#else
char *lookup_env(struct ddr_priv *priv, const char *format, ...)
{
	return NULL;
}

char *lookup_env_ull(struct ddr_priv *priv, const char *format, ...)
{
	return NULL;
}
#endif

/* Number of L2C Tag-and-data sections (TADs) that are connected to LMC. */
#define CVMX_L2C_TADS  ((OCTEON_IS_MODEL(OCTEON_CN68XX) ||		\
			 OCTEON_IS_MODEL(OCTEON_CN73XX) ||		\
			 OCTEON_IS_MODEL(OCTEON_CNF75XX)) ? 4 :		\
			(OCTEON_IS_MODEL(OCTEON_CN78XX)) ? 8 : 1)

/* Number of L2C IOBs connected to LMC. */
#define CVMX_L2C_IOBS  ((OCTEON_IS_MODEL(OCTEON_CN68XX) ||		\
			 OCTEON_IS_MODEL(OCTEON_CN78XX) ||		\
			 OCTEON_IS_MODEL(OCTEON_CN73XX) ||		\
			 OCTEON_IS_MODEL(OCTEON_CNF75XX)) ? 2 : 1)

#define CVMX_L2C_MAX_MEMSZ_ALLOWED (OCTEON_IS_OCTEON2() ?		\
				    (32 * CVMX_L2C_TADS) :		\
				    (OCTEON_IS_MODEL(OCTEON_CN70XX) ?	\
				     512 : (OCTEON_IS_OCTEON3() ? 1024 : 0)))

/**
 * Initialize the BIG address in L2C+DRAM to generate proper error
 * on reading/writing to an non-existent memory location.
 *
 * @param node      OCX CPU node number
 * @param mem_size  Amount of DRAM configured in MB.
 * @param mode      Allow/Disallow reporting errors L2C_INT_SUM[BIGRD,BIGWR].
 */
static void cvmx_l2c_set_big_size(struct ddr_priv *priv, u64 mem_size, int mode)
{
	if ((OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) &&
	    !OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_X)) {
		union cvmx_l2c_big_ctl big_ctl;
		int bits = 0, zero_bits = 0;
		u64 mem;

		if (mem_size > (CVMX_L2C_MAX_MEMSZ_ALLOWED * 1024ull)) {
			printf("WARNING: Invalid memory size(%lld) requested, should be <= %lld\n",
			       mem_size,
			       (u64)CVMX_L2C_MAX_MEMSZ_ALLOWED * 1024);
			mem_size = CVMX_L2C_MAX_MEMSZ_ALLOWED * 1024;
		}

		mem = mem_size;
		while (mem) {
			if ((mem & 1) == 0)
				zero_bits++;
			bits++;
			mem >>= 1;
		}

		if ((bits - zero_bits) != 1 || (bits - 9) <= 0) {
			printf("ERROR: Invalid DRAM size (%lld) requested, refer to L2C_BIG_CTL[maxdram] for valid options.\n",
			       mem_size);
			return;
		}

		/*
		 * The BIG/HOLE is logic is not supported in pass1 as per
		 * Errata L2C-17736
		 */
		if (mode == 0 && OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			mode = 1;

		big_ctl.u64 = 0;
		big_ctl.s.maxdram = bits - 9;
		big_ctl.cn61xx.disable = mode;
		l2c_wr(priv, CVMX_L2C_BIG_CTL_REL, big_ctl.u64);
	}
}

static u32 octeon3_refclock(u32 alt_refclk, u32 ddr_hertz,
			    struct dimm_config *dimm_config)
{
	u32 ddr_ref_hertz = CFG_REF_HERTZ;
	int ddr_type;
	int spd_dimm_type;

	debug("%s(%u, %u, %p)\n", __func__, alt_refclk, ddr_hertz, dimm_config);

	/* Octeon 3 case... */

	/* we know whether alternate refclk is always wanted
	 * we also know already if we want 2133 MT/s
	 * if alt refclk not always wanted, then probe DDR and
	 * DIMM type if DDR4 and RDIMMs, then set desired refclk
	 * to 100MHz, otherwise to default (50MHz)
	 * depend on ddr_initialize() to do the refclk selection
	 * and validation/
	 */
	if (alt_refclk) {
		/*
		 * If alternate refclk was specified, let it override
		 * everything
		 */
		ddr_ref_hertz = alt_refclk * 1000000;
		printf("%s: DRAM init: %d MHz refclk is REQUESTED ALWAYS\n",
		       __func__, alt_refclk);
	} else if (ddr_hertz > 1000000000) {
		ddr_type = get_ddr_type(dimm_config, 0);
		spd_dimm_type = get_dimm_module_type(dimm_config, 0, ddr_type);

		debug("ddr type: 0x%x, dimm type: 0x%x\n", ddr_type,
		      spd_dimm_type);
		/* Is DDR4 and RDIMM just to be sure. */
		if (ddr_type == DDR4_DRAM &&
		    (spd_dimm_type == 1 || spd_dimm_type == 5 ||
		     spd_dimm_type == 8)) {
			/* Yes, we require 100MHz refclk, so set it. */
			ddr_ref_hertz = 100000000;
			puts("DRAM init: 100 MHz refclk is REQUIRED\n");
		}
	}

	debug("%s: speed: %u\n", __func__, ddr_ref_hertz);
	return ddr_ref_hertz;
}

int encode_row_lsb_ddr3(int row_lsb)
{
	int row_lsb_start = 14;

	/* Decoding for row_lsb        */
	/* 000: row_lsb = mem_adr[14]  */
	/* 001: row_lsb = mem_adr[15]  */
	/* 010: row_lsb = mem_adr[16]  */
	/* 011: row_lsb = mem_adr[17]  */
	/* 100: row_lsb = mem_adr[18]  */
	/* 101: row_lsb = mem_adr[19]  */
	/* 110: row_lsb = mem_adr[20]  */
	/* 111: RESERVED               */

	if (octeon_is_cpuid(OCTEON_CN6XXX) ||
	    octeon_is_cpuid(OCTEON_CNF7XXX) || octeon_is_cpuid(OCTEON_CN7XXX))
		row_lsb_start = 14;
	else
		printf("ERROR: Unsupported Octeon model: 0x%x\n",
		       read_c0_prid());

	return row_lsb - row_lsb_start;
}

int encode_pbank_lsb_ddr3(int pbank_lsb)
{
	/* Decoding for pbank_lsb                                        */
	/* 0000:DIMM = mem_adr[28]    / rank = mem_adr[27] (if RANK_ENA) */
	/* 0001:DIMM = mem_adr[29]    / rank = mem_adr[28]      "        */
	/* 0010:DIMM = mem_adr[30]    / rank = mem_adr[29]      "        */
	/* 0011:DIMM = mem_adr[31]    / rank = mem_adr[30]      "        */
	/* 0100:DIMM = mem_adr[32]    / rank = mem_adr[31]      "        */
	/* 0101:DIMM = mem_adr[33]    / rank = mem_adr[32]      "        */
	/* 0110:DIMM = mem_adr[34]    / rank = mem_adr[33]      "        */
	/* 0111:DIMM = 0              / rank = mem_adr[34]      "        */
	/* 1000-1111: RESERVED                                           */

	int pbank_lsb_start = 0;

	if (octeon_is_cpuid(OCTEON_CN6XXX) ||
	    octeon_is_cpuid(OCTEON_CNF7XXX) || octeon_is_cpuid(OCTEON_CN7XXX))
		pbank_lsb_start = 28;
	else
		printf("ERROR: Unsupported Octeon model: 0x%x\n",
		       read_c0_prid());

	return pbank_lsb - pbank_lsb_start;
}

static void set_ddr_clock_initialized(struct ddr_priv *priv, int if_num,
				      bool inited_flag)
{
	priv->ddr_clock_initialized[if_num] = inited_flag;
}

static int ddr_clock_initialized(struct ddr_priv *priv, int if_num)
{
	return priv->ddr_clock_initialized[if_num];
}

static void set_ddr_memory_preserved(struct ddr_priv *priv)
{
	priv->ddr_memory_preserved = true;
}

bool ddr_memory_preserved(struct ddr_priv *priv)
{
	return priv->ddr_memory_preserved;
}

static void cn78xx_lmc_dreset_init(struct ddr_priv *priv, int if_num)
{
	union cvmx_lmcx_dll_ctl2 dll_ctl2;

	/*
	 * The remainder of this section describes the sequence for LMCn.
	 *
	 * 1. If not done already, write LMC(0..3)_DLL_CTL2 to its reset value
	 * (except without changing the LMC(0..3)_DLL_CTL2[INTF_EN] value from
	 * that set in the prior Step 3), including
	 * LMC(0..3)_DLL_CTL2[DRESET] = 1.
	 *
	 * 2. Without changing any other LMC(0..3)_DLL_CTL2 fields, write
	 * LMC(0..3)_DLL_CTL2[DLL_BRINGUP] = 1.
	 */

	dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(if_num));
	dll_ctl2.cn78xx.dll_bringup = 1;
	lmc_wr(priv, CVMX_LMCX_DLL_CTL2(if_num), dll_ctl2.u64);

	/*
	 * 3. Read LMC(0..3)_DLL_CTL2 and wait for the result.
	 */

	lmc_rd(priv, CVMX_LMCX_DLL_CTL2(if_num));

	/*
	 * 4. Wait for a minimum of 10 LMC CK cycles.
	 */

	udelay(1);

	/*
	 * 5. Without changing any other fields in LMC(0..3)_DLL_CTL2, write
	 * LMC(0..3)_DLL_CTL2[QUAD_DLL_ENA] = 1.
	 * LMC(0..3)_DLL_CTL2[QUAD_DLL_ENA] must not change after this point
	 * without restarting the LMCn DRESET initialization sequence.
	 */

	dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(if_num));
	dll_ctl2.cn78xx.quad_dll_ena = 1;
	lmc_wr(priv, CVMX_LMCX_DLL_CTL2(if_num), dll_ctl2.u64);

	/*
	 * 6. Read LMC(0..3)_DLL_CTL2 and wait for the result.
	 */

	lmc_rd(priv, CVMX_LMCX_DLL_CTL2(if_num));

	/*
	 * 7. Wait a minimum of 10 us.
	 */

	udelay(10);

	/*
	 * 8. Without changing any other fields in LMC(0..3)_DLL_CTL2, write
	 * LMC(0..3)_DLL_CTL2[DLL_BRINGUP] = 0.
	 * LMC(0..3)_DLL_CTL2[DLL_BRINGUP] must not change after this point
	 * without restarting the LMCn DRESET initialization sequence.
	 */

	dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(if_num));
	dll_ctl2.cn78xx.dll_bringup = 0;
	lmc_wr(priv, CVMX_LMCX_DLL_CTL2(if_num), dll_ctl2.u64);

	/*
	 * 9. Read LMC(0..3)_DLL_CTL2 and wait for the result.
	 */

	lmc_rd(priv, CVMX_LMCX_DLL_CTL2(if_num));

	/*
	 * 10. Without changing any other fields in LMC(0..3)_DLL_CTL2, write
	 * LMC(0..3)_DLL_CTL2[DRESET] = 0.
	 * LMC(0..3)_DLL_CTL2[DRESET] must not change after this point without
	 * restarting the LMCn DRESET initialization sequence.
	 *
	 * After completing LMCn DRESET initialization, all LMC CSRs may be
	 * accessed.  Prior to completing LMC DRESET initialization, only
	 * LMC(0..3)_DDR_PLL_CTL, LMC(0..3)_DLL_CTL2, LMC(0..3)_RESET_CTL, and
	 * LMC(0..3)_COMP_CTL2 LMC CSRs can be accessed.
	 */

	dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(if_num));
	dll_ctl2.cn78xx.dreset = 0;
	lmc_wr(priv, CVMX_LMCX_DLL_CTL2(if_num), dll_ctl2.u64);
}

int initialize_ddr_clock(struct ddr_priv *priv, struct ddr_conf *ddr_conf,
			 u32 cpu_hertz, u32 ddr_hertz, u32 ddr_ref_hertz,
			 int if_num, u32 if_mask)
{
	char *s;

	if (ddr_clock_initialized(priv, if_num))
		return 0;

	if (!ddr_clock_initialized(priv, 0)) {	/* Do this once */
		union cvmx_lmcx_reset_ctl reset_ctl;
		int i;

		/*
		 * Check to see if memory is to be preserved and set global
		 * flag
		 */
		for (i = 3; i >= 0; --i) {
			if ((if_mask & (1 << i)) == 0)
				continue;

			reset_ctl.u64 = lmc_rd(priv, CVMX_LMCX_RESET_CTL(i));
			if (reset_ctl.s.ddr3psv == 1) {
				debug("LMC%d Preserving memory\n", i);
				set_ddr_memory_preserved(priv);

				/* Re-initialize flags */
				reset_ctl.s.ddr3pwarm = 0;
				reset_ctl.s.ddr3psoft = 0;
				reset_ctl.s.ddr3psv = 0;
				lmc_wr(priv, CVMX_LMCX_RESET_CTL(i),
				       reset_ctl.u64);
			}
		}
	}

	/*
	 * ToDo: Add support for these SoCs:
	 *
	 * if (octeon_is_cpuid(OCTEON_CN63XX) ||
	 * octeon_is_cpuid(OCTEON_CN66XX) ||
	 * octeon_is_cpuid(OCTEON_CN61XX) || octeon_is_cpuid(OCTEON_CNF71XX))
	 *
	 * and
	 *
	 * if (octeon_is_cpuid(OCTEON_CN68XX))
	 *
	 * and
	 *
	 * if (octeon_is_cpuid(OCTEON_CN70XX))
	 *
	 */

	if (octeon_is_cpuid(OCTEON_CN78XX) || octeon_is_cpuid(OCTEON_CN73XX) ||
	    octeon_is_cpuid(OCTEON_CNF75XX)) {
		union cvmx_lmcx_dll_ctl2 dll_ctl2;
		union cvmx_lmcx_dll_ctl3 ddr_dll_ctl3;
		union cvmx_lmcx_ddr_pll_ctl ddr_pll_ctl;
		struct dimm_config *dimm_config_table =
			ddr_conf->dimm_config_table;
		int en_idx, save_en_idx, best_en_idx = 0;
		u64 clkf, clkr, max_clkf = 127;
		u64 best_clkf = 0, best_clkr = 0;
		u64 best_pll_MHz = 0;
		u64 pll_MHz;
		u64 min_pll_MHz = 800;
		u64 max_pll_MHz = 5000;
		u64 error;
		u64 best_error;
		u64 best_calculated_ddr_hertz = 0;
		u64 calculated_ddr_hertz = 0;
		u64 orig_ddr_hertz = ddr_hertz;
		const int _en[] = { 1, 2, 3, 4, 5, 6, 7, 8, 10, 12 };
		int override_pll_settings;
		int new_bwadj;
		int ddr_type;
		int i;

		/* ddr_type only indicates DDR4 or DDR3 */
		ddr_type = (read_spd(&dimm_config_table[0], 0,
				     DDR4_SPD_KEY_BYTE_DEVICE_TYPE) ==
			    0x0C) ? DDR4_DRAM : DDR3_DRAM;

		/*
		 * 5.9 LMC Initialization Sequence
		 *
		 * There are 13 parts to the LMC initialization procedure:
		 *
		 * 1. DDR PLL initialization
		 *
		 * 2. LMC CK initialization
		 *
		 * 3. LMC interface enable initialization
		 *
		 * 4. LMC DRESET initialization
		 *
		 * 5. LMC CK local initialization
		 *
		 * 6. LMC RESET initialization
		 *
		 * 7. Early LMC initialization
		 *
		 * 8. LMC offset training
		 *
		 * 9. LMC internal Vref training
		 *
		 * 10. LMC deskew training
		 *
		 * 11. LMC write leveling
		 *
		 * 12. LMC read leveling
		 *
		 * 13. Final LMC initialization
		 *
		 * CN78XX supports two modes:
		 *
		 * - two-LMC mode: both LMCs 2/3 must not be enabled
		 * (LMC2/3_DLL_CTL2[DRESET] must be set to 1 and
		 * LMC2/3_DLL_CTL2[INTF_EN]
		 * must be set to 0) and both LMCs 0/1 must be enabled).
		 *
		 * - four-LMC mode: all four LMCs 0..3 must be enabled.
		 *
		 * Steps 4 and 6..13 should each be performed for each
		 * enabled LMC (either twice or four times). Steps 1..3 and
		 * 5 are more global in nature and each must be executed
		 * exactly once (not once per LMC) each time the DDR PLL
		 * changes or is first brought up. Steps 1..3 and 5 need
		 * not be performed if the DDR PLL is stable.
		 *
		 * Generally, the steps are performed in order. The exception
		 * is that the CK local initialization (step 5) must be
		 * performed after some DRESET initializations (step 4) and
		 * before other DRESET initializations when the DDR PLL is
		 * brought up or changed. (The CK local initialization uses
		 * information from some LMCs to bring up the other local
		 * CKs.) The following text describes these ordering
		 * requirements in more detail.
		 *
		 * Following any chip reset, the DDR PLL must be brought up,
		 * and all 13 steps should be executed. Subsequently, it is
		 * possible to execute only steps 4 and 6..13, or to execute
		 * only steps 8..13.
		 *
		 * The remainder of this section covers these initialization
		 * steps in sequence.
		 */

		/* Do the following init only once */
		if (if_num != 0)
			goto not_if0;

		/* Only for interface #0 ... */

		/*
		 * 5.9.3 LMC Interface-Enable Initialization
		 *
		 * LMC interface-enable initialization (Step 3) must be#
		 * performed after Step 2 for each chip reset and whenever
		 * the DDR clock speed changes. This step needs to be
		 * performed only once, not once per LMC. Perform the
		 * following three substeps for the LMC interface-enable
		 * initialization:
		 *
		 * 1. Without changing any other LMC2_DLL_CTL2 fields
		 * (LMC(0..3)_DLL_CTL2 should be at their reset values after
		 * Step 1), write LMC2_DLL_CTL2[INTF_EN] = 1 if four-LMC
		 * mode is desired.
		 *
		 * 2. Without changing any other LMC3_DLL_CTL2 fields, write
		 * LMC3_DLL_CTL2[INTF_EN] = 1 if four-LMC mode is desired.
		 *
		 * 3. Read LMC2_DLL_CTL2 and wait for the result.
		 *
		 * The LMC2_DLL_CTL2[INTF_EN] and LMC3_DLL_CTL2[INTF_EN]
		 * values should not be changed by software from this point.
		 */

		for (i = 0; i < 4; ++i) {
			if ((if_mask & (1 << i)) == 0)
				continue;

			dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(i));

			dll_ctl2.cn78xx.byp_setting = 0;
			dll_ctl2.cn78xx.byp_sel = 0;
			dll_ctl2.cn78xx.quad_dll_ena = 0;
			dll_ctl2.cn78xx.dreset = 1;
			dll_ctl2.cn78xx.dll_bringup = 0;
			dll_ctl2.cn78xx.intf_en = 0;

			lmc_wr(priv, CVMX_LMCX_DLL_CTL2(i), dll_ctl2.u64);
		}

		/*
		 * ###### Interface enable (intf_en) deferred until after
		 * DDR_DIV_RESET=0 #######
		 */

		/*
		 * 5.9.1 DDR PLL Initialization
		 *
		 * DDR PLL initialization (Step 1) must be performed for each
		 * chip reset and whenever the DDR clock speed changes. This
		 * step needs to be performed only once, not once per LMC.
		 *
		 * Perform the following eight substeps to initialize the
		 * DDR PLL:
		 *
		 * 1. If not done already, write all fields in
		 * LMC(0..3)_DDR_PLL_CTL and
		 * LMC(0..1)_DLL_CTL2 to their reset values, including:
		 *
		 * .. LMC0_DDR_PLL_CTL[DDR_DIV_RESET] = 1
		 * .. LMC0_DLL_CTL2[DRESET] = 1
		 *
		 * This substep is not necessary after a chip reset.
		 *
		 */

		ddr_pll_ctl.u64 = lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(0));

		ddr_pll_ctl.cn78xx.reset_n = 0;
		ddr_pll_ctl.cn78xx.ddr_div_reset = 1;
		ddr_pll_ctl.cn78xx.phy_dcok = 0;

		/*
		 * 73XX pass 1.3 has LMC0 DCLK_INVERT tied to 1; earlier
		 * 73xx passes are tied to 0
		 *
		 * 75XX needs LMC0 DCLK_INVERT set to 1 to minimize duty
		 * cycle falling points
		 *
		 * and we default all other chips LMC0 to DCLK_INVERT=0
		 */
		ddr_pll_ctl.cn78xx.dclk_invert =
		    !!(octeon_is_cpuid(OCTEON_CN73XX_PASS1_3) ||
		       octeon_is_cpuid(OCTEON_CNF75XX));

		/*
		 * allow override of LMC0 desired setting for DCLK_INVERT,
		 * but not on 73XX;
		 * we cannot change LMC0 DCLK_INVERT on 73XX any pass
		 */
		if (!(octeon_is_cpuid(OCTEON_CN73XX))) {
			s = lookup_env(priv, "ddr0_set_dclk_invert");
			if (s) {
				ddr_pll_ctl.cn78xx.dclk_invert =
				    !!simple_strtoul(s, NULL, 0);
				debug("LMC0: override DDR_PLL_CTL[dclk_invert] to %d\n",
				      ddr_pll_ctl.cn78xx.dclk_invert);
			}
		}

		lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(0), ddr_pll_ctl.u64);
		debug("%-45s : 0x%016llx\n", "LMC0: DDR_PLL_CTL",
		      ddr_pll_ctl.u64);

		// only when LMC1 is active
		if (if_mask & 0x2) {
			/*
			 * For CNF75XX, both LMC0 and LMC1 use the same PLL,
			 * so we use the LMC0 setting of DCLK_INVERT for LMC1.
			 */
			if (!octeon_is_cpuid(OCTEON_CNF75XX)) {
				int override = 0;

				/*
				 * by default, for non-CNF75XX, we want
				 * LMC1 toggled LMC0
				 */
				int lmc0_dclk_invert =
				    ddr_pll_ctl.cn78xx.dclk_invert;

				/*
				 * FIXME: work-around for DDR3 UDIMM problems
				 * is to use LMC0 setting on LMC1 and if
				 * 73xx pass 1.3, we want to default LMC1
				 * DCLK_INVERT to LMC0, not the invert of LMC0
				 */
				int lmc1_dclk_invert;

				lmc1_dclk_invert =
					((ddr_type == DDR4_DRAM) &&
					 !octeon_is_cpuid(OCTEON_CN73XX_PASS1_3))
					? lmc0_dclk_invert ^ 1 :
					lmc0_dclk_invert;

				/*
				 * allow override of LMC1 desired setting for
				 * DCLK_INVERT
				 */
				s = lookup_env(priv, "ddr1_set_dclk_invert");
				if (s) {
					lmc1_dclk_invert =
						!!simple_strtoul(s, NULL, 0);
					override = 1;
				}
				debug("LMC1: %s DDR_PLL_CTL[dclk_invert] to %d (LMC0 %d)\n",
				      (override) ? "override" :
				      "default", lmc1_dclk_invert,
				      lmc0_dclk_invert);

				ddr_pll_ctl.cn78xx.dclk_invert =
					lmc1_dclk_invert;
			}

			// but always write LMC1 CSR if it is active
			lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(1), ddr_pll_ctl.u64);
			debug("%-45s : 0x%016llx\n",
			      "LMC1: DDR_PLL_CTL", ddr_pll_ctl.u64);
		}

		/*
		 * 2. If the current DRAM contents are not preserved (see
		 * LMC(0..3)_RESET_ CTL[DDR3PSV]), this is also an appropriate
		 * time to assert the RESET# pin of the DDR3/DDR4 DRAM parts.
		 * If desired, write
		 * LMC0_RESET_ CTL[DDR3RST] = 0 without modifying any other
		 * LMC0_RESET_CTL fields to assert the DDR_RESET_L pin.
		 * No action is required here to assert DDR_RESET_L
		 * following a chip reset. Refer to Section 5.9.6. Do this
		 * for all enabled LMCs.
		 */

		for (i = 0; (!ddr_memory_preserved(priv)) && i < 4; ++i) {
			union cvmx_lmcx_reset_ctl reset_ctl;

			if ((if_mask & (1 << i)) == 0)
				continue;

			reset_ctl.u64 = lmc_rd(priv, CVMX_LMCX_RESET_CTL(i));
			reset_ctl.cn78xx.ddr3rst = 0;	/* Reset asserted */
			debug("LMC%d Asserting DDR_RESET_L\n", i);
			lmc_wr(priv, CVMX_LMCX_RESET_CTL(i), reset_ctl.u64);
			lmc_rd(priv, CVMX_LMCX_RESET_CTL(i));
		}

		/*
		 * 3. Without changing any other LMC0_DDR_PLL_CTL values,
		 * write LMC0_DDR_PLL_CTL[CLKF] with a value that gives a
		 * desired DDR PLL speed. The LMC0_DDR_PLL_CTL[CLKF] value
		 * should be selected in conjunction with the post-scalar
		 * divider values for LMC (LMC0_DDR_PLL_CTL[DDR_PS_EN]) so
		 * that the desired LMC CK speeds are is produced (all
		 * enabled LMCs must run the same speed). Section 5.14
		 * describes LMC0_DDR_PLL_CTL[CLKF] and
		 * LMC0_DDR_PLL_CTL[DDR_PS_EN] programmings that produce
		 * the desired LMC CK speed. Section 5.9.2 describes LMC CK
		 * initialization, which can be done separately from the DDR
		 * PLL initialization described in this section.
		 *
		 * The LMC0_DDR_PLL_CTL[CLKF] value must not change after
		 * this point without restarting this SDRAM PLL
		 * initialization sequence.
		 */

		/* Init to max error */
		error = ddr_hertz;
		best_error = ddr_hertz;

		debug("DDR Reference Hertz = %d\n", ddr_ref_hertz);

		while (best_error == ddr_hertz) {
			for (clkr = 0; clkr < 4; ++clkr) {
				for (en_idx =
				     sizeof(_en) / sizeof(int) -
				     1; en_idx >= 0; --en_idx) {
					save_en_idx = en_idx;
					clkf =
					    ((ddr_hertz) *
					     (clkr + 1) * (_en[save_en_idx]));
					clkf = divide_nint(clkf, ddr_ref_hertz)
					    - 1;
					pll_MHz =
					    ddr_ref_hertz *
					    (clkf + 1) / (clkr + 1) / 1000000;
					calculated_ddr_hertz =
					    ddr_ref_hertz *
					    (clkf +
					     1) / ((clkr +
						    1) * (_en[save_en_idx]));
					error =
					    ddr_hertz - calculated_ddr_hertz;

					if (pll_MHz < min_pll_MHz ||
					    pll_MHz > max_pll_MHz)
						continue;
					if (clkf > max_clkf) {
						/*
						 * PLL requires clkf to be
						 * limited
						 */
						continue;
					}
					if (abs(error) > abs(best_error))
						continue;

					debug("clkr: %2llu, en[%d]: %2d, clkf: %4llu, pll_MHz: %4llu, ddr_hertz: %8llu, error: %8lld\n",
					      clkr, save_en_idx,
					      _en[save_en_idx], clkf, pll_MHz,
					     calculated_ddr_hertz, error);

					/* Favor the highest PLL frequency. */
					if (abs(error) < abs(best_error) ||
					    pll_MHz > best_pll_MHz) {
						best_pll_MHz = pll_MHz;
						best_calculated_ddr_hertz =
							calculated_ddr_hertz;
						best_error = error;
						best_clkr = clkr;
						best_clkf = clkf;
						best_en_idx = save_en_idx;
					}
				}
			}

			override_pll_settings = 0;

			s = lookup_env(priv, "ddr_pll_clkr");
			if (s) {
				best_clkr = simple_strtoul(s, NULL, 0);
				override_pll_settings = 1;
			}

			s = lookup_env(priv, "ddr_pll_clkf");
			if (s) {
				best_clkf = simple_strtoul(s, NULL, 0);
				override_pll_settings = 1;
			}

			s = lookup_env(priv, "ddr_pll_en_idx");
			if (s) {
				best_en_idx = simple_strtoul(s, NULL, 0);
				override_pll_settings = 1;
			}

			if (override_pll_settings) {
				best_pll_MHz =
				    ddr_ref_hertz * (best_clkf +
						     1) /
				    (best_clkr + 1) / 1000000;
				best_calculated_ddr_hertz =
				    ddr_ref_hertz * (best_clkf +
						     1) /
				    ((best_clkr + 1) * (_en[best_en_idx]));
				best_error =
				    ddr_hertz - best_calculated_ddr_hertz;
			}

			debug("clkr: %2llu, en[%d]: %2d, clkf: %4llu, pll_MHz: %4llu, ddr_hertz: %8llu, error: %8lld <==\n",
			      best_clkr, best_en_idx, _en[best_en_idx],
			      best_clkf, best_pll_MHz,
			      best_calculated_ddr_hertz, best_error);

			/*
			 * Try lowering the frequency if we can't get a
			 * working configuration
			 */
			if (best_error == ddr_hertz) {
				if (ddr_hertz < orig_ddr_hertz - 10000000)
					break;
				ddr_hertz -= 1000000;
				best_error = ddr_hertz;
			}
		}

		if (best_error == ddr_hertz) {
			printf("ERROR: Can not compute a legal DDR clock speed configuration.\n");
			return -1;
		}

		new_bwadj = (best_clkf + 1) / 10;
		debug("bwadj: %2d\n", new_bwadj);

		s = lookup_env(priv, "ddr_pll_bwadj");
		if (s) {
			new_bwadj = strtoul(s, NULL, 0);
			debug("bwadj: %2d\n", new_bwadj);
		}

		for (i = 0; i < 2; ++i) {
			if ((if_mask & (1 << i)) == 0)
				continue;

			ddr_pll_ctl.u64 =
			    lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));
			debug("LMC%d: DDR_PLL_CTL                             : 0x%016llx\n",
			      i, ddr_pll_ctl.u64);

			ddr_pll_ctl.cn78xx.ddr_ps_en = best_en_idx;
			ddr_pll_ctl.cn78xx.clkf = best_clkf;
			ddr_pll_ctl.cn78xx.clkr = best_clkr;
			ddr_pll_ctl.cn78xx.reset_n = 0;
			ddr_pll_ctl.cn78xx.bwadj = new_bwadj;

			lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(i), ddr_pll_ctl.u64);
			debug("LMC%d: DDR_PLL_CTL                             : 0x%016llx\n",
			      i, ddr_pll_ctl.u64);

			/*
			 * For cnf75xx LMC0 and LMC1 use the same PLL so
			 * only program LMC0 PLL.
			 */
			if (octeon_is_cpuid(OCTEON_CNF75XX))
				break;
		}

		for (i = 0; i < 4; ++i) {
			if ((if_mask & (1 << i)) == 0)
				continue;

			/*
			 * 4. Read LMC0_DDR_PLL_CTL and wait for the result.
			 */

			lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));

			/*
			 * 5. Wait a minimum of 3 us.
			 */

			udelay(3);	/* Wait 3 us */

			/*
			 * 6. Write LMC0_DDR_PLL_CTL[RESET_N] = 1 without
			 * changing any other LMC0_DDR_PLL_CTL values.
			 */

			ddr_pll_ctl.u64 =
			    lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));
			ddr_pll_ctl.cn78xx.reset_n = 1;
			lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(i), ddr_pll_ctl.u64);

			/*
			 * 7. Read LMC0_DDR_PLL_CTL and wait for the result.
			 */

			lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));

			/*
			 * 8. Wait a minimum of 25 us.
			 */

			udelay(25);	/* Wait 25 us */

			/*
			 * For cnf75xx LMC0 and LMC1 use the same PLL so
			 * only program LMC0 PLL.
			 */
			if (octeon_is_cpuid(OCTEON_CNF75XX))
				break;
		}

		for (i = 0; i < 4; ++i) {
			if ((if_mask & (1 << i)) == 0)
				continue;

			/*
			 * 5.9.2 LMC CK Initialization
			 *
			 * DDR PLL initialization must be completed prior to
			 * starting LMC CK initialization.
			 *
			 * Perform the following substeps to initialize the
			 * LMC CK:
			 *
			 * 1. Without changing any other LMC(0..3)_DDR_PLL_CTL
			 * values, write
			 * LMC(0..3)_DDR_PLL_CTL[DDR_DIV_RESET] = 1 and
			 * LMC(0..3)_DDR_PLL_CTL[DDR_PS_EN] with the
			 * appropriate value to get the desired LMC CK speed.
			 * Section 5.14 discusses CLKF and DDR_PS_EN
			 * programmings.  The LMC(0..3)_DDR_PLL_CTL[DDR_PS_EN]
			 * must not change after this point without restarting
			 * this LMC CK initialization sequence.
			 */

			ddr_pll_ctl.u64 = lmc_rd(priv,
						 CVMX_LMCX_DDR_PLL_CTL(i));
			ddr_pll_ctl.cn78xx.ddr_div_reset = 1;
			lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(i), ddr_pll_ctl.u64);

			/*
			 * 2. Without changing any other fields in
			 * LMC(0..3)_DDR_PLL_CTL, write
			 * LMC(0..3)_DDR_PLL_CTL[DDR4_MODE] = 0.
			 */

			ddr_pll_ctl.u64 =
			    lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));
			ddr_pll_ctl.cn78xx.ddr4_mode =
			    (ddr_type == DDR4_DRAM) ? 1 : 0;
			lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(i), ddr_pll_ctl.u64);

			/*
			 * 3. Read LMC(0..3)_DDR_PLL_CTL and wait for the
			 * result.
			 */

			lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));

			/*
			 * 4. Wait a minimum of 1 us.
			 */

			udelay(1);	/* Wait 1 us */

			/*
			 * ###### Steps 5 through 7 deferred until after
			 * DDR_DIV_RESET=0 #######
			 */

			/*
			 * 8. Without changing any other LMC(0..3)_COMP_CTL2
			 * values, write
			 * LMC(0..3)_COMP_CTL2[CK_CTL,CONTROL_CTL,CMD_CTL]
			 * to the desired DDR*_CK_*_P control and command
			 * signals drive strength.
			 */

			union cvmx_lmcx_comp_ctl2 comp_ctl2;
			const struct ddr3_custom_config *custom_lmc_config =
			    &ddr_conf->custom_lmc_config;

			comp_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_COMP_CTL2(i));

			/* Default 4=34.3 ohm */
			comp_ctl2.cn78xx.dqx_ctl =
			    (custom_lmc_config->dqx_ctl ==
			     0) ? 4 : custom_lmc_config->dqx_ctl;
			/* Default 4=34.3 ohm */
			comp_ctl2.cn78xx.ck_ctl =
			    (custom_lmc_config->ck_ctl ==
			     0) ? 4 : custom_lmc_config->ck_ctl;
			/* Default 4=34.3 ohm */
			comp_ctl2.cn78xx.cmd_ctl =
			    (custom_lmc_config->cmd_ctl ==
			     0) ? 4 : custom_lmc_config->cmd_ctl;

			comp_ctl2.cn78xx.rodt_ctl = 0x4;	/* 60 ohm */

			comp_ctl2.cn70xx.ptune_offset =
			    (abs(custom_lmc_config->ptune_offset) & 0x7)
			    | (_sign(custom_lmc_config->ptune_offset) << 3);
			comp_ctl2.cn70xx.ntune_offset =
			    (abs(custom_lmc_config->ntune_offset) & 0x7)
			    | (_sign(custom_lmc_config->ntune_offset) << 3);

			s = lookup_env(priv, "ddr_clk_ctl");
			if (s) {
				comp_ctl2.cn78xx.ck_ctl =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_ck_ctl");
			if (s) {
				comp_ctl2.cn78xx.ck_ctl =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_cmd_ctl");
			if (s) {
				comp_ctl2.cn78xx.cmd_ctl =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_dqx_ctl");
			if (s) {
				comp_ctl2.cn78xx.dqx_ctl =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_ptune_offset");
			if (s) {
				comp_ctl2.cn78xx.ptune_offset =
				    simple_strtoul(s, NULL, 0);
			}

			s = lookup_env(priv, "ddr_ntune_offset");
			if (s) {
				comp_ctl2.cn78xx.ntune_offset =
				    simple_strtoul(s, NULL, 0);
			}

			lmc_wr(priv, CVMX_LMCX_COMP_CTL2(i), comp_ctl2.u64);

			/*
			 * 9. Read LMC(0..3)_DDR_PLL_CTL and wait for the
			 * result.
			 */

			lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));

			/*
			 * 10. Wait a minimum of 200 ns.
			 */

			udelay(1);	/* Wait 1 us */

			/*
			 * 11. Without changing any other
			 * LMC(0..3)_DDR_PLL_CTL values, write
			 * LMC(0..3)_DDR_PLL_CTL[DDR_DIV_RESET] = 0.
			 */

			ddr_pll_ctl.u64 = lmc_rd(priv,
						 CVMX_LMCX_DDR_PLL_CTL(i));
			ddr_pll_ctl.cn78xx.ddr_div_reset = 0;
			lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(i), ddr_pll_ctl.u64);

			/*
			 * 12. Read LMC(0..3)_DDR_PLL_CTL and wait for the
			 * result.
			 */

			lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));

			/*
			 * 13. Wait a minimum of 200 ns.
			 */

			udelay(1);	/* Wait 1 us */
		}

		/*
		 * Relocated Interface Enable (intf_en) Step
		 */
		for (i = (octeon_is_cpuid(OCTEON_CN73XX) ||
			  octeon_is_cpuid(OCTEON_CNF75XX)) ? 1 : 2;
		     i < 4; ++i) {
			/*
			 * This step is only necessary for LMC 2 and 3 in
			 * 4-LMC mode. The mask will cause the unpopulated
			 * interfaces to be skipped.
			 */
			if ((if_mask & (1 << i)) == 0)
				continue;

			dll_ctl2.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL2(i));
			dll_ctl2.cn78xx.intf_en = 1;
			lmc_wr(priv, CVMX_LMCX_DLL_CTL2(i), dll_ctl2.u64);
			lmc_rd(priv, CVMX_LMCX_DLL_CTL2(i));
		}

		/*
		 * Relocated PHY_DCOK Step
		 */
		for (i = 0; i < 4; ++i) {
			if ((if_mask & (1 << i)) == 0)
				continue;
			/*
			 * 5. Without changing any other fields in
			 * LMC(0..3)_DDR_PLL_CTL, write
			 * LMC(0..3)_DDR_PLL_CTL[PHY_DCOK] = 1.
			 */

			ddr_pll_ctl.u64 = lmc_rd(priv,
						 CVMX_LMCX_DDR_PLL_CTL(i));
			ddr_pll_ctl.cn78xx.phy_dcok = 1;
			lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(i), ddr_pll_ctl.u64);
			/*
			 * 6. Read LMC(0..3)_DDR_PLL_CTL and wait for
			 * the result.
			 */

			lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(i));

			/*
			 * 7. Wait a minimum of 20 us.
			 */

			udelay(20);	/* Wait 20 us */
		}

		/*
		 * 5.9.4 LMC DRESET Initialization
		 *
		 * All of the DDR PLL, LMC global CK, and LMC interface
		 * enable initializations must be completed prior to starting
		 * this LMC DRESET initialization (Step 4).
		 *
		 * This LMC DRESET step is done for all enabled LMCs.
		 *
		 * There are special constraints on the ordering of DRESET
		 * initialization (Steps 4) and CK local initialization
		 * (Step 5) whenever CK local initialization must be executed.
		 * CK local initialization must be executed whenever the DDR
		 * PLL is being brought up (for each chip reset* and whenever
		 * the DDR clock speed changes).
		 *
		 * When Step 5 must be executed in the two-LMC mode case:
		 * - LMC0 DRESET initialization must occur before Step 5.
		 * - LMC1 DRESET initialization must occur after Step 5.
		 *
		 * When Step 5 must be executed in the four-LMC mode case:
		 * - LMC2 and LMC3 DRESET initialization must occur before
		 *   Step 5.
		 * - LMC0 and LMC1 DRESET initialization must occur after
		 *   Step 5.
		 */

		if (octeon_is_cpuid(OCTEON_CN73XX)) {
			/* ONE-LMC or TWO-LMC MODE BEFORE STEP 5 for cn73xx */
			cn78xx_lmc_dreset_init(priv, 0);
		} else if (octeon_is_cpuid(OCTEON_CNF75XX)) {
			if (if_mask == 0x3) {
				/*
				 * 2-LMC Mode: LMC1 DRESET must occur
				 * before Step 5
				 */
				cn78xx_lmc_dreset_init(priv, 1);
			}
		} else {
			/* TWO-LMC MODE DRESET BEFORE STEP 5 */
			if (if_mask == 0x3)
				cn78xx_lmc_dreset_init(priv, 0);

			/* FOUR-LMC MODE BEFORE STEP 5 */
			if (if_mask == 0xf) {
				cn78xx_lmc_dreset_init(priv, 2);
				cn78xx_lmc_dreset_init(priv, 3);
			}
		}

		/*
		 * 5.9.5 LMC CK Local Initialization
		 *
		 * All of DDR PLL, LMC global CK, and LMC interface-enable
		 * initializations must be completed prior to starting this
		 * LMC CK local initialization (Step 5).
		 *
		 * LMC CK Local initialization must be performed for each
		 * chip reset and whenever the DDR clock speed changes. This
		 * step needs to be performed only once, not once per LMC.
		 *
		 * There are special constraints on the ordering of DRESET
		 * initialization (Steps 4) and CK local initialization
		 * (Step 5) whenever CK local initialization must be executed.
		 * CK local initialization must be executed whenever the
		 * DDR PLL is being brought up (for each chip reset and
		 * whenever the DDR clock speed changes).
		 *
		 * When Step 5 must be executed in the two-LMC mode case:
		 * - LMC0 DRESET initialization must occur before Step 5.
		 * - LMC1 DRESET initialization must occur after Step 5.
		 *
		 * When Step 5 must be executed in the four-LMC mode case:
		 * - LMC2 and LMC3 DRESET initialization must occur before
		 *   Step 5.
		 * - LMC0 and LMC1 DRESET initialization must occur after
		 *   Step 5.
		 *
		 * LMC CK local initialization is different depending on
		 * whether two-LMC or four-LMC modes are desired.
		 */

		if (if_mask == 0x3) {
			int temp_lmc_if_num = octeon_is_cpuid(OCTEON_CNF75XX) ?
				1 : 0;

			/*
			 * 5.9.5.1 LMC CK Local Initialization for Two-LMC
			 * Mode
			 *
			 * 1. Write LMC0_DLL_CTL3 to its reset value. (Note
			 * that LMC0_DLL_CTL3[DLL_90_BYTE_SEL] = 0x2 .. 0x8
			 * should also work.)
			 */

			ddr_dll_ctl3.u64 = 0;
			ddr_dll_ctl3.cn78xx.dclk90_recal_dis = 1;

			if (octeon_is_cpuid(OCTEON_CNF75XX))
				ddr_dll_ctl3.cn78xx.dll90_byte_sel = 7;
			else
				ddr_dll_ctl3.cn78xx.dll90_byte_sel = 1;

			lmc_wr(priv,
			       CVMX_LMCX_DLL_CTL3(temp_lmc_if_num),
			       ddr_dll_ctl3.u64);

			/*
			 * 2. Read LMC0_DLL_CTL3 and wait for the result.
			 */

			lmc_rd(priv, CVMX_LMCX_DLL_CTL3(temp_lmc_if_num));

			/*
			 * 3. Without changing any other fields in
			 * LMC0_DLL_CTL3, write
			 * LMC0_DLL_CTL3[DCLK90_FWD] = 1.  Writing
			 * LMC0_DLL_CTL3[DCLK90_FWD] = 1
			 * causes clock-delay information to be forwarded
			 * from LMC0 to LMC1.
			 */

			ddr_dll_ctl3.cn78xx.dclk90_fwd = 1;
			lmc_wr(priv,
			       CVMX_LMCX_DLL_CTL3(temp_lmc_if_num),
			       ddr_dll_ctl3.u64);

			/*
			 * 4. Read LMC0_DLL_CTL3 and wait for the result.
			 */

			lmc_rd(priv, CVMX_LMCX_DLL_CTL3(temp_lmc_if_num));
		}

		if (if_mask == 0xf) {
			/*
			 * 5.9.5.2 LMC CK Local Initialization for Four-LMC
			 * Mode
			 *
			 * 1. Write LMC2_DLL_CTL3 to its reset value except
			 * LMC2_DLL_CTL3[DLL90_BYTE_SEL] = 0x7.
			 */

			ddr_dll_ctl3.u64 = 0;
			ddr_dll_ctl3.cn78xx.dclk90_recal_dis = 1;
			ddr_dll_ctl3.cn78xx.dll90_byte_sel = 7;
			lmc_wr(priv, CVMX_LMCX_DLL_CTL3(2), ddr_dll_ctl3.u64);

			/*
			 * 2. Write LMC3_DLL_CTL3 to its reset value except
			 * LMC3_DLL_CTL3[DLL90_BYTE_SEL] = 0x2.
			 */

			ddr_dll_ctl3.u64 = 0;
			ddr_dll_ctl3.cn78xx.dclk90_recal_dis = 1;
			ddr_dll_ctl3.cn78xx.dll90_byte_sel = 2;
			lmc_wr(priv, CVMX_LMCX_DLL_CTL3(3), ddr_dll_ctl3.u64);

			/*
			 * 3. Read LMC3_DLL_CTL3 and wait for the result.
			 */

			lmc_rd(priv, CVMX_LMCX_DLL_CTL3(3));

			/*
			 * 4. Without changing any other fields in
			 * LMC2_DLL_CTL3, write LMC2_DLL_CTL3[DCLK90_FWD] = 1
			 * and LMC2_DLL_CTL3[DCLK90_RECAL_ DIS] = 1.
			 * Writing LMC2_DLL_CTL3[DCLK90_FWD] = 1 causes LMC 2
			 * to forward clockdelay information to LMC0. Setting
			 * LMC2_DLL_CTL3[DCLK90_RECAL_DIS] to 1 prevents LMC2
			 * from periodically recalibrating this delay
			 * information.
			 */

			ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(2));
			ddr_dll_ctl3.cn78xx.dclk90_fwd = 1;
			ddr_dll_ctl3.cn78xx.dclk90_recal_dis = 1;
			lmc_wr(priv, CVMX_LMCX_DLL_CTL3(2), ddr_dll_ctl3.u64);

			/*
			 * 5. Without changing any other fields in
			 * LMC3_DLL_CTL3, write LMC3_DLL_CTL3[DCLK90_FWD] = 1
			 * and LMC3_DLL_CTL3[DCLK90_RECAL_ DIS] = 1.
			 * Writing LMC3_DLL_CTL3[DCLK90_FWD] = 1 causes LMC3
			 * to forward clockdelay information to LMC1. Setting
			 * LMC3_DLL_CTL3[DCLK90_RECAL_DIS] to 1 prevents LMC3
			 * from periodically recalibrating this delay
			 * information.
			 */

			ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(3));
			ddr_dll_ctl3.cn78xx.dclk90_fwd = 1;
			ddr_dll_ctl3.cn78xx.dclk90_recal_dis = 1;
			lmc_wr(priv, CVMX_LMCX_DLL_CTL3(3), ddr_dll_ctl3.u64);

			/*
			 * 6. Read LMC3_DLL_CTL3 and wait for the result.
			 */

			lmc_rd(priv, CVMX_LMCX_DLL_CTL3(3));
		}

		if (octeon_is_cpuid(OCTEON_CNF75XX)) {
			/*
			 * cnf75xx 2-LMC Mode: LMC0 DRESET must occur after
			 * Step 5, Do LMC0 for 1-LMC Mode here too
			 */
			cn78xx_lmc_dreset_init(priv, 0);
		}

		/* TWO-LMC MODE AFTER STEP 5 */
		if (if_mask == 0x3) {
			if (octeon_is_cpuid(OCTEON_CNF75XX)) {
				/*
				 * cnf75xx 2-LMC Mode: LMC0 DRESET must
				 * occur after Step 5
				 */
				cn78xx_lmc_dreset_init(priv, 0);
			} else {
				cn78xx_lmc_dreset_init(priv, 1);
			}
		}

		/* FOUR-LMC MODE AFTER STEP 5 */
		if (if_mask == 0xf) {
			cn78xx_lmc_dreset_init(priv, 0);
			cn78xx_lmc_dreset_init(priv, 1);

			/*
			 * Enable periodic recalibration of DDR90 delay
			 * line in.
			 */
			ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(0));
			ddr_dll_ctl3.cn78xx.dclk90_recal_dis = 0;
			lmc_wr(priv, CVMX_LMCX_DLL_CTL3(0), ddr_dll_ctl3.u64);
			ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(1));
			ddr_dll_ctl3.cn78xx.dclk90_recal_dis = 0;
			lmc_wr(priv, CVMX_LMCX_DLL_CTL3(1), ddr_dll_ctl3.u64);
		}

		/* Enable fine tune mode for all LMCs */
		for (i = 0; i < 4; ++i) {
			if ((if_mask & (1 << i)) == 0)
				continue;
			ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(i));
			ddr_dll_ctl3.cn78xx.fine_tune_mode = 1;
			lmc_wr(priv, CVMX_LMCX_DLL_CTL3(i), ddr_dll_ctl3.u64);
		}

		/*
		 * Enable the trim circuit on the appropriate channels to
		 * adjust the DDR clock duty cycle for chips that support
		 * it
		 */
		if (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X) ||
		    octeon_is_cpuid(OCTEON_CN73XX) ||
		    octeon_is_cpuid(OCTEON_CNF75XX)) {
			union cvmx_lmcx_phy_ctl lmc_phy_ctl;
			int i;

			for (i = 0; i < 4; ++i) {
				if ((if_mask & (1 << i)) == 0)
					continue;

				lmc_phy_ctl.u64 =
				    lmc_rd(priv, CVMX_LMCX_PHY_CTL(i));

				if (octeon_is_cpuid(OCTEON_CNF75XX) ||
				    octeon_is_cpuid(OCTEON_CN73XX_PASS1_3)) {
					/* Both LMCs */
					lmc_phy_ctl.s.lv_mode = 0;
				} else {
					/* Odd LMCs = 0, Even LMCs = 1 */
					lmc_phy_ctl.s.lv_mode = (~i) & 1;
				}

				debug("LMC%d: PHY_CTL                                 : 0x%016llx\n",
				      i, lmc_phy_ctl.u64);
				lmc_wr(priv, CVMX_LMCX_PHY_CTL(i),
				       lmc_phy_ctl.u64);
			}
		}
	}

	/*
	 * 5.9.6 LMC RESET Initialization
	 *
	 * NOTE: this is now done as the first step in
	 * init_octeon3_ddr3_interface, rather than the last step in clock
	 * init. This reorg allows restarting per-LMC initialization should
	 * problems be encountered, rather than being forced to resort to
	 * resetting the chip and starting all over.
	 *
	 * Look for the code in octeon3_lmc.c: perform_lmc_reset().
	 */

	/* Fallthrough for all interfaces... */
not_if0:

	/*
	 * Start the DDR clock so that its frequency can be measured.
	 * For some chips we must activate the memory controller with
	 * init_start to make the DDR clock start to run.
	 */
	if ((!octeon_is_cpuid(OCTEON_CN6XXX)) &&
	    (!octeon_is_cpuid(OCTEON_CNF7XXX)) &&
	    (!octeon_is_cpuid(OCTEON_CN7XXX))) {
		union cvmx_lmcx_mem_cfg0 mem_cfg0;

		mem_cfg0.u64 = 0;
		mem_cfg0.s.init_start = 1;
		lmc_wr(priv, CVMX_LMCX_MEM_CFG0(if_num), mem_cfg0.u64);
		lmc_rd(priv, CVMX_LMCX_MEM_CFG0(if_num));
	}

	set_ddr_clock_initialized(priv, if_num, 1);

	return 0;
}

static void octeon_ipd_delay_cycles(u64 cycles)
{
	u64 start = csr_rd(CVMX_IPD_CLK_COUNT);

	while (start + cycles > csr_rd(CVMX_IPD_CLK_COUNT))
		;
}

static void octeon_ipd_delay_cycles_o3(u64 cycles)
{
	u64 start = csr_rd(CVMX_FPA_CLK_COUNT);

	while (start + cycles > csr_rd(CVMX_FPA_CLK_COUNT))
		;
}

static u32 measure_octeon_ddr_clock(struct ddr_priv *priv,
				    struct ddr_conf *ddr_conf, u32 cpu_hertz,
				    u32 ddr_hertz, u32 ddr_ref_hertz,
				    int if_num, u32 if_mask)
{
	u64 core_clocks;
	u64 ddr_clocks;
	u64 calc_ddr_hertz;

	if (ddr_conf) {
		if (initialize_ddr_clock(priv, ddr_conf, cpu_hertz,
					 ddr_hertz, ddr_ref_hertz, if_num,
					 if_mask) != 0)
			return 0;
	}

	/* Dynamically determine the DDR clock speed */
	if (OCTEON_IS_OCTEON2() || octeon_is_cpuid(OCTEON_CN70XX)) {
		core_clocks = csr_rd(CVMX_IPD_CLK_COUNT);
		ddr_clocks = lmc_rd(priv, CVMX_LMCX_DCLK_CNT(if_num));
		/* How many cpu cycles to measure over */
		octeon_ipd_delay_cycles(100000000);
		core_clocks = csr_rd(CVMX_IPD_CLK_COUNT) - core_clocks;
		ddr_clocks =
		    lmc_rd(priv, CVMX_LMCX_DCLK_CNT(if_num)) - ddr_clocks;
		calc_ddr_hertz = ddr_clocks * gd->bus_clk / core_clocks;
	} else if (octeon_is_cpuid(OCTEON_CN7XXX)) {
		core_clocks = csr_rd(CVMX_FPA_CLK_COUNT);
		ddr_clocks = lmc_rd(priv, CVMX_LMCX_DCLK_CNT(if_num));
		/* How many cpu cycles to measure over */
		octeon_ipd_delay_cycles_o3(100000000);
		core_clocks = csr_rd(CVMX_FPA_CLK_COUNT) - core_clocks;
		ddr_clocks =
		    lmc_rd(priv, CVMX_LMCX_DCLK_CNT(if_num)) - ddr_clocks;
		calc_ddr_hertz = ddr_clocks * gd->bus_clk / core_clocks;
	} else {
		core_clocks = csr_rd(CVMX_IPD_CLK_COUNT);
		/*
		 * ignore overflow, starts counting when we enable the
		 * controller
		 */
		ddr_clocks = lmc_rd(priv, CVMX_LMCX_DCLK_CNT_LO(if_num));
		/* How many cpu cycles to measure over */
		octeon_ipd_delay_cycles(100000000);
		core_clocks = csr_rd(CVMX_IPD_CLK_COUNT) - core_clocks;
		ddr_clocks =
		    lmc_rd(priv, CVMX_LMCX_DCLK_CNT_LO(if_num)) - ddr_clocks;
		calc_ddr_hertz = ddr_clocks * cpu_hertz / core_clocks;
	}

	debug("core clocks: %llu, ddr clocks: %llu, calc rate: %llu\n",
	      core_clocks, ddr_clocks, calc_ddr_hertz);
	debug("LMC%d: Measured DDR clock: %lld, cpu clock: %u, ddr clocks: %llu\n",
	      if_num, calc_ddr_hertz, cpu_hertz, ddr_clocks);

	/* Check for unreasonable settings. */
	if (calc_ddr_hertz < 10000) {
		udelay(8000000 * 100);
		printf("DDR clock misconfigured on interface %d. Resetting...\n",
		       if_num);
		do_reset(NULL, 0, 0, NULL);
	}

	return calc_ddr_hertz;
}

u64 lmc_ddr3_rl_dbg_read(struct ddr_priv *priv, int if_num, int idx)
{
	union cvmx_lmcx_rlevel_dbg rlevel_dbg;
	union cvmx_lmcx_rlevel_ctl rlevel_ctl;

	rlevel_ctl.u64 = lmc_rd(priv, CVMX_LMCX_RLEVEL_CTL(if_num));
	rlevel_ctl.s.byte = idx;

	lmc_wr(priv, CVMX_LMCX_RLEVEL_CTL(if_num), rlevel_ctl.u64);
	lmc_rd(priv, CVMX_LMCX_RLEVEL_CTL(if_num));

	rlevel_dbg.u64 = lmc_rd(priv, CVMX_LMCX_RLEVEL_DBG(if_num));
	return rlevel_dbg.s.bitmask;
}

u64 lmc_ddr3_wl_dbg_read(struct ddr_priv *priv, int if_num, int idx)
{
	union cvmx_lmcx_wlevel_dbg wlevel_dbg;

	wlevel_dbg.u64 = 0;
	wlevel_dbg.s.byte = idx;

	lmc_wr(priv, CVMX_LMCX_WLEVEL_DBG(if_num), wlevel_dbg.u64);
	lmc_rd(priv, CVMX_LMCX_WLEVEL_DBG(if_num));

	wlevel_dbg.u64 = lmc_rd(priv, CVMX_LMCX_WLEVEL_DBG(if_num));
	return wlevel_dbg.s.bitmask;
}

int validate_ddr3_rlevel_bitmask(struct rlevel_bitmask *rlevel_bitmask_p,
				 int ddr_type)
{
	int i;
	int errors = 0;
	u64 mask = 0;		/* Used in 64-bit comparisons */
	u8 mstart = 0;
	u8 width = 0;
	u8 firstbit = 0;
	u8 lastbit = 0;
	u8 bubble = 0;
	u8 tbubble = 0;
	u8 blank = 0;
	u8 narrow = 0;
	u8 trailing = 0;
	u64 bitmask = rlevel_bitmask_p->bm;
	u8 extras = 0;
	u8 toolong = 0;
	u64 temp;

	if (bitmask == 0) {
		blank += RLEVEL_BITMASK_BLANK_ERROR;
	} else {
		/* Look for fb, the first bit */
		temp = bitmask;
		while (!(temp & 1)) {
			firstbit++;
			temp >>= 1;
		}

		/* Look for lb, the last bit */
		lastbit = firstbit;
		while ((temp >>= 1))
			lastbit++;

		/*
		 * Start with the max range to try to find the largest mask
		 * within the bitmask data
		 */
		width = MASKRANGE_BITS;
		for (mask = MASKRANGE; mask > 0; mask >>= 1, --width) {
			for (mstart = lastbit - width + 1; mstart >= firstbit;
			     --mstart) {
				temp = mask << mstart;
				if ((bitmask & temp) == temp)
					goto done_now;
			}
		}
done_now:
		/* look for any more contiguous 1's to the right of mstart */
		if (width == MASKRANGE_BITS) {	// only when maximum mask
			while ((bitmask >> (mstart - 1)) & 1) {
				// slide right over more 1's
				--mstart;
				// count the number of extra bits only for DDR4
				if (ddr_type == DDR4_DRAM)
					extras++;
			}
		}

		/* Penalize any extra 1's beyond the maximum desired mask */
		if (extras > 0)
			toolong =
			    RLEVEL_BITMASK_TOOLONG_ERROR * ((1 << extras) - 1);

		/* Detect if bitmask is too narrow. */
		if (width < 4)
			narrow = (4 - width) * RLEVEL_BITMASK_NARROW_ERROR;

		/*
		 * detect leading bubble bits, that is, any 0's between first
		 * and mstart
		 */
		temp = bitmask >> (firstbit + 1);
		i = mstart - firstbit - 1;
		while (--i >= 0) {
			if ((temp & 1) == 0)
				bubble += RLEVEL_BITMASK_BUBBLE_BITS_ERROR;
			temp >>= 1;
		}

		temp = bitmask >> (mstart + width + extras);
		i = lastbit - (mstart + width + extras - 1);
		while (--i >= 0) {
			if (temp & 1) {
				/*
				 * Detect 1 bits after the trailing end of
				 * the mask, including last.
				 */
				trailing += RLEVEL_BITMASK_TRAILING_BITS_ERROR;
			} else {
				/*
				 * Detect trailing bubble bits, that is,
				 * any 0's between end-of-mask and last
				 */
				tbubble += RLEVEL_BITMASK_BUBBLE_BITS_ERROR;
			}
			temp >>= 1;
		}
	}

	errors = bubble + tbubble + blank + narrow + trailing + toolong;

	/* Pass out useful statistics */
	rlevel_bitmask_p->mstart = mstart;
	rlevel_bitmask_p->width = width;

	debug_bitmask_print("bm:%08lx mask:%02lx, width:%2u, mstart:%2d, fb:%2u, lb:%2u (bu:%2d, tb:%2d, bl:%2d, n:%2d, t:%2d, x:%2d) errors:%3d %s\n",
			    (unsigned long)bitmask, mask, width, mstart,
			    firstbit, lastbit, bubble, tbubble, blank,
			    narrow, trailing, toolong, errors,
			    (errors) ? "=> invalid" : "");

	return errors;
}

int compute_ddr3_rlevel_delay(u8 mstart, u8 width,
			      union cvmx_lmcx_rlevel_ctl rlevel_ctl)
{
	int delay;

	debug_bitmask_print("  offset_en:%d", rlevel_ctl.s.offset_en);

	if (rlevel_ctl.s.offset_en) {
		delay = max((int)mstart,
			    (int)(mstart + width - 1 - rlevel_ctl.s.offset));
	} else {
		/* if (rlevel_ctl.s.offset) { *//* Experimental */
		if (0) {
			delay = max(mstart + rlevel_ctl.s.offset, mstart + 1);
			/*
			 * Insure that the offset delay falls within the
			 * bitmask
			 */
			delay = min(delay, mstart + width - 1);
		} else {
			/* Round down */
			delay = (width - 1) / 2 + mstart;
		}
	}

	return delay;
}

/* Default ODT config must disable ODT */
/* Must be const (read only) so that the structure is in flash */
const struct dimm_odt_config disable_odt_config[] = {
	/*   1 */ { 0, 0x0000, {.u64 = 0x0000}, {.u64 = 0x0000}, 0, 0x0000, 0 },
	/*   2 */ { 0, 0x0000, {.u64 = 0x0000}, {.u64 = 0x0000}, 0, 0x0000, 0 },
	/*   3 */ { 0, 0x0000, {.u64 = 0x0000}, {.u64 = 0x0000}, 0, 0x0000, 0 },
	/*   4 */ { 0, 0x0000, {.u64 = 0x0000}, {.u64 = 0x0000}, 0, 0x0000, 0 },
};

/* Memory controller setup function */
static int init_octeon_dram_interface(struct ddr_priv *priv,
				      struct ddr_conf *ddr_conf,
				      u32 ddr_hertz, u32 cpu_hertz,
				      u32 ddr_ref_hertz, int if_num,
				      u32 if_mask)
{
	u32 mem_size_mbytes = 0;
	char *s;

	s = lookup_env(priv, "ddr_timing_hertz");
	if (s)
		ddr_hertz = simple_strtoul(s, NULL, 0);

	if (OCTEON_IS_OCTEON3()) {
		int lmc_restart_retries = 0;
#define DEFAULT_RESTART_RETRIES 3
		int lmc_restart_retries_limit = DEFAULT_RESTART_RETRIES;

		s = lookup_env(priv, "ddr_restart_retries_limit");
		if (s)
			lmc_restart_retries_limit = simple_strtoul(s, NULL, 0);

restart_lmc_init:
		mem_size_mbytes = init_octeon3_ddr3_interface(priv, ddr_conf,
							      ddr_hertz,
							      cpu_hertz,
							      ddr_ref_hertz,
							      if_num, if_mask);
		if (mem_size_mbytes == 0) {	// 0 means restart is possible
			if (lmc_restart_retries < lmc_restart_retries_limit) {
				lmc_restart_retries++;
				printf("N0.LMC%d Configuration problem: attempting LMC reset and init restart %d\n",
				       if_num, lmc_restart_retries);
				goto restart_lmc_init;
			} else {
				if (lmc_restart_retries_limit > 0) {
					printf("INFO: N0.LMC%d Configuration: fatal problem remains after %d LMC init retries - Resetting node...\n",
					       if_num, lmc_restart_retries);
					mdelay(500);
					do_reset(NULL, 0, 0, NULL);
				} else {
					// return an error, no restart
					mem_size_mbytes = -1;
				}
			}
		}
	}

	debug("N0.LMC%d Configuration Completed: %d MB\n",
	      if_num, mem_size_mbytes);

	return mem_size_mbytes;
}

#define WLEVEL_BYTE_BITS	5
#define WLEVEL_BYTE_MSK		((1ULL << 5) - 1)

void upd_wl_rank(union cvmx_lmcx_wlevel_rankx *lmc_wlevel_rank,
		 int byte, int delay)
{
	union cvmx_lmcx_wlevel_rankx temp_wlevel_rank;

	if (byte >= 0 && byte <= 8) {
		temp_wlevel_rank.u64 = lmc_wlevel_rank->u64;
		temp_wlevel_rank.u64 &=
		    ~(WLEVEL_BYTE_MSK << (WLEVEL_BYTE_BITS * byte));
		temp_wlevel_rank.u64 |=
		    ((delay & WLEVEL_BYTE_MSK) << (WLEVEL_BYTE_BITS * byte));
		lmc_wlevel_rank->u64 = temp_wlevel_rank.u64;
	}
}

int get_wl_rank(union cvmx_lmcx_wlevel_rankx *lmc_wlevel_rank, int byte)
{
	int delay = 0;

	if (byte >= 0 && byte <= 8)
		delay =
		    ((lmc_wlevel_rank->u64) >> (WLEVEL_BYTE_BITS *
						byte)) & WLEVEL_BYTE_MSK;

	return delay;
}

void upd_rl_rank(union cvmx_lmcx_rlevel_rankx *lmc_rlevel_rank,
		 int byte, int delay)
{
	union cvmx_lmcx_rlevel_rankx temp_rlevel_rank;

	if (byte >= 0 && byte <= 8) {
		temp_rlevel_rank.u64 =
		    lmc_rlevel_rank->u64 & ~(RLEVEL_BYTE_MSK <<
					     (RLEVEL_BYTE_BITS * byte));
		temp_rlevel_rank.u64 |=
		    ((delay & RLEVEL_BYTE_MSK) << (RLEVEL_BYTE_BITS * byte));
		lmc_rlevel_rank->u64 = temp_rlevel_rank.u64;
	}
}

int get_rl_rank(union cvmx_lmcx_rlevel_rankx *lmc_rlevel_rank, int byte)
{
	int delay = 0;

	if (byte >= 0 && byte <= 8)
		delay =
		    ((lmc_rlevel_rank->u64) >> (RLEVEL_BYTE_BITS *
						byte)) & RLEVEL_BYTE_MSK;

	return delay;
}

void rlevel_to_wlevel(union cvmx_lmcx_rlevel_rankx *lmc_rlevel_rank,
		      union cvmx_lmcx_wlevel_rankx *lmc_wlevel_rank, int byte)
{
	int byte_delay = get_rl_rank(lmc_rlevel_rank, byte);

	debug("Estimating Wlevel delay byte %d: ", byte);
	debug("Rlevel=%d => ", byte_delay);
	byte_delay = divide_roundup(byte_delay, 2) & 0x1e;
	debug("Wlevel=%d\n", byte_delay);
	upd_wl_rank(lmc_wlevel_rank, byte, byte_delay);
}

/* Delay trend: constant=0, decreasing=-1, increasing=1 */
static s64 calc_delay_trend(s64 v)
{
	if (v == 0)
		return 0;
	if (v < 0)
		return -1;

	return 1;
}

/*
 * Evaluate delay sequence across the whole range of byte delays while
 * keeping track of the overall delay trend, increasing or decreasing.
 * If the trend changes charge an error amount to the score.
 */

// NOTE: "max_adj_delay_inc" argument is, by default, 1 for DDR3 and 2 for DDR4

int nonseq_del(struct rlevel_byte_data *rlevel_byte, int start, int end,
	       int max_adj_delay_inc)
{
	s64 error = 0;
	s64 delay_trend, prev_trend = 0;
	int byte_idx;
	s64 seq_err;
	s64 adj_err;
	s64 delay_inc;
	s64 delay_diff;

	for (byte_idx = start; byte_idx < end; ++byte_idx) {
		delay_diff = rlevel_byte[byte_idx + 1].delay -
			rlevel_byte[byte_idx].delay;
		delay_trend = calc_delay_trend(delay_diff);

		/*
		 * Increment error each time the trend changes to the
		 * opposite direction.
		 */
		if (prev_trend != 0 && delay_trend != 0 &&
		    prev_trend != delay_trend) {
			seq_err = RLEVEL_NONSEQUENTIAL_DELAY_ERROR;
		} else {
			seq_err = 0;
		}

		// how big was the delay change, if any
		delay_inc = abs(delay_diff);

		/*
		 * Even if the trend did not change to the opposite direction,
		 * check for the magnitude of the change, and scale the
		 * penalty by the amount that the size is larger than the
		 * provided limit.
		 */
		if (max_adj_delay_inc != 0 && delay_inc > max_adj_delay_inc) {
			adj_err = (delay_inc - max_adj_delay_inc) *
				RLEVEL_ADJACENT_DELAY_ERROR;
		} else {
			adj_err = 0;
		}

		rlevel_byte[byte_idx + 1].sqerrs = seq_err + adj_err;
		error += seq_err + adj_err;

		debug_bitmask_print("Byte %d: %d, Byte %d: %d, delay_trend: %ld, prev_trend: %ld, [%ld/%ld]%s%s\n",
				    byte_idx + 0,
				    rlevel_byte[byte_idx + 0].delay,
				    byte_idx + 1,
				    rlevel_byte[byte_idx + 1].delay,
				    delay_trend,
				    prev_trend, seq_err, adj_err,
				    (seq_err) ?
				    " => Nonsequential byte delay" : "",
				    (adj_err) ?
				    " => Adjacent delay error" : "");

		if (delay_trend != 0)
			prev_trend = delay_trend;
	}

	return (int)error;
}

int roundup_ddr3_wlevel_bitmask(int bitmask)
{
	int shifted_bitmask;
	int leader;
	int delay;

	for (leader = 0; leader < 8; ++leader) {
		shifted_bitmask = (bitmask >> leader);
		if ((shifted_bitmask & 1) == 0)
			break;
	}

	for (leader = leader; leader < 16; ++leader) {
		shifted_bitmask = (bitmask >> (leader % 8));
		if (shifted_bitmask & 1)
			break;
	}

	delay = (leader & 1) ? leader + 1 : leader;
	delay = delay % 8;

	return delay;
}

/* Octeon 2 */
static void oct2_ddr3_seq(struct ddr_priv *priv, int rank_mask, int if_num,
			  int sequence)
{
	char *s;

#ifdef DEBUG_PERFORM_DDR3_SEQUENCE
	static const char * const sequence_str[] = {
		"power-up/init",
		"read-leveling",
		"self-refresh entry",
		"self-refresh exit",
		"precharge power-down entry",
		"precharge power-down exit",
		"write-leveling",
		"illegal"
	};
#endif

	union cvmx_lmcx_control lmc_control;
	union cvmx_lmcx_config lmc_config;
	int save_ddr2t;

	lmc_control.u64 = lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
	save_ddr2t = lmc_control.s.ddr2t;

	if (save_ddr2t == 0 && octeon_is_cpuid(OCTEON_CN63XX_PASS1_X)) {
		/* Some register parts (IDT and TI included) do not like
		 * the sequence that LMC generates for an MRS register
		 * write in 1T mode. In this case, the register part does
		 * not properly forward the MRS register write to the DRAM
		 * parts.  See errata (LMC-14548) Issues with registered
		 * DIMMs.
		 */
		debug("Forcing DDR 2T during init seq. Re: Pass 1 LMC-14548\n");
		lmc_control.s.ddr2t = 1;
	}

	s = lookup_env(priv, "ddr_init_2t");
	if (s)
		lmc_control.s.ddr2t = simple_strtoul(s, NULL, 0);

	lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), lmc_control.u64);

	lmc_config.u64 = lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));

	lmc_config.s.init_start = 1;
	if (OCTEON_IS_OCTEON2())
		lmc_config.cn63xx.sequence = sequence;
	lmc_config.s.rankmask = rank_mask;

#ifdef DEBUG_PERFORM_DDR3_SEQUENCE
	debug("Performing LMC sequence: rank_mask=0x%02x, sequence=%d, %s\n",
	      rank_mask, sequence, sequence_str[sequence]);
#endif

	lmc_wr(priv, CVMX_LMCX_CONFIG(if_num), lmc_config.u64);
	lmc_rd(priv, CVMX_LMCX_CONFIG(if_num));
	udelay(600);		/* Wait a while */

	lmc_control.s.ddr2t = save_ddr2t;
	lmc_wr(priv, CVMX_LMCX_CONTROL(if_num), lmc_control.u64);
	lmc_rd(priv, CVMX_LMCX_CONTROL(if_num));
}

/* Check to see if any custom offset values are used */
static int is_dll_offset_provided(const int8_t *dll_offset_table)
{
	int i;

	if (!dll_offset_table)	/* Check for pointer to table. */
		return 0;

	for (i = 0; i < 9; ++i) {
		if (dll_offset_table[i] != 0)
			return 1;
	}

	return 0;
}

void change_dll_offset_enable(struct ddr_priv *priv, int if_num, int change)
{
	union cvmx_lmcx_dll_ctl3 ddr_dll_ctl3;

	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));
	SET_DDR_DLL_CTL3(offset_ena, !!change);
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);
	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));
}

unsigned short load_dll_offset(struct ddr_priv *priv, int if_num,
			       int dll_offset_mode, int byte_offset, int byte)
{
	union cvmx_lmcx_dll_ctl3 ddr_dll_ctl3;
	int field_width = 6;
	/*
	 * byte_sel:
	 * 0x1 = byte 0, ..., 0x9 = byte 8
	 * 0xA = all bytes
	 */
	int byte_sel = (byte == 10) ? byte : byte + 1;

	if (octeon_is_cpuid(OCTEON_CN6XXX))
		field_width = 5;

	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));
	SET_DDR_DLL_CTL3(load_offset, 0);
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);
	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));

	SET_DDR_DLL_CTL3(mode_sel, dll_offset_mode);
	SET_DDR_DLL_CTL3(offset,
			 (abs(byte_offset) & (~(-1 << field_width))) |
			 (_sign(byte_offset) << field_width));
	SET_DDR_DLL_CTL3(byte_sel, byte_sel);
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);
	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));

	SET_DDR_DLL_CTL3(load_offset, 1);
	lmc_wr(priv, CVMX_LMCX_DLL_CTL3(if_num), ddr_dll_ctl3.u64);
	ddr_dll_ctl3.u64 = lmc_rd(priv, CVMX_LMCX_DLL_CTL3(if_num));

	return (unsigned short)GET_DDR_DLL_CTL3(offset);
}

void process_custom_dll_offsets(struct ddr_priv *priv, int if_num,
				const char *enable_str,
				const int8_t *offsets, const char *byte_str,
				int mode)
{
	const char *s;
	int enabled;
	int provided;
	int byte_offset;
	unsigned short offset[9] = { 0 };
	int byte;

	s = lookup_env(priv, enable_str);
	if (s)
		enabled = !!simple_strtol(s, NULL, 0);
	else
		enabled = -1;

	/*
	 * enabled == -1: no override, do only configured offsets if provided
	 * enabled ==  0: override OFF, do NOT do it even if configured
	 *                offsets provided
	 * enabled ==  1: override ON, do it for overrides plus configured
	 *                offsets
	 */

	if (enabled == 0)
		return;

	provided = is_dll_offset_provided(offsets);

	if (enabled < 0 && !provided)
		return;

	change_dll_offset_enable(priv, if_num, 0);

	for (byte = 0; byte < 9; ++byte) {
		// always take the provided, if available
		byte_offset = (provided) ? offsets[byte] : 0;

		// then, if enabled, use any overrides present
		if (enabled > 0) {
			s = lookup_env(priv, byte_str, if_num, byte);
			if (s)
				byte_offset = simple_strtol(s, NULL, 0);
		}

		offset[byte] =
		    load_dll_offset(priv, if_num, mode, byte_offset, byte);
	}

	change_dll_offset_enable(priv, if_num, 1);

	debug("N0.LMC%d: DLL %s Offset 8:0       :  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x  0x%02x\n",
	      if_num, (mode == 2) ? "Read " : "Write",
	      offset[8], offset[7], offset[6], offset[5], offset[4],
	      offset[3], offset[2], offset[1], offset[0]);
}

void ddr_init_seq(struct ddr_priv *priv, int rank_mask, int if_num)
{
	char *s;
	int ddr_init_loops = 1;
	int rankx;

	s = lookup_env(priv, "ddr%d_init_loops", if_num);
	if (s)
		ddr_init_loops = simple_strtoul(s, NULL, 0);

	while (ddr_init_loops--) {
		for (rankx = 0; rankx < 8; rankx++) {
			if (!(rank_mask & (1 << rankx)))
				continue;

			if (OCTEON_IS_OCTEON3()) {
				/* power-up/init */
				oct3_ddr3_seq(priv, 1 << rankx, if_num, 0);
			} else {
				/* power-up/init */
				oct2_ddr3_seq(priv, 1 << rankx, if_num, 0);
			}

			udelay(1000);	/* Wait a while. */

			s = lookup_env(priv, "ddr_sequence1");
			if (s) {
				int sequence1;

				sequence1 = simple_strtoul(s, NULL, 0);

				if (OCTEON_IS_OCTEON3()) {
					oct3_ddr3_seq(priv, 1 << rankx,
						      if_num, sequence1);
				} else {
					oct2_ddr3_seq(priv, 1 << rankx,
						      if_num, sequence1);
				}
			}

			s = lookup_env(priv, "ddr_sequence2");
			if (s) {
				int sequence2;

				sequence2 = simple_strtoul(s, NULL, 0);

				if (OCTEON_IS_OCTEON3())
					oct3_ddr3_seq(priv, 1 << rankx,
						      if_num, sequence2);
				else
					oct2_ddr3_seq(priv, 1 << rankx,
						      if_num, sequence2);
			}
		}
	}
}

static int octeon_ddr_initialize(struct ddr_priv *priv, u32 cpu_hertz,
				 u32 ddr_hertz, u32 ddr_ref_hertz,
				 u32 if_mask,
				 struct ddr_conf *ddr_conf,
				 u32 *measured_ddr_hertz)
{
	u32 ddr_conf_valid_mask = 0;
	int memsize_mbytes = 0;
	char *eptr;
	int if_idx;
	u32 ddr_max_speed = 667000000;
	u32 calc_ddr_hertz = -1;
	int val;
	int ret;

	if (env_get("ddr_verbose") || env_get("ddr_prompt"))
		priv->flags |= FLAG_DDR_VERBOSE;

#ifdef DDR_VERBOSE
	priv->flags |= FLAG_DDR_VERBOSE;
#endif

	if (env_get("ddr_trace_init")) {
		printf("Parameter ddr_trace_init found in environment.\n");
		priv->flags |= FLAG_DDR_TRACE_INIT;
		priv->flags |= FLAG_DDR_VERBOSE;
	}

	priv->flags |= FLAG_DDR_DEBUG;

	val = env_get_ulong("ddr_debug", 10, (u32)-1);
	switch (val) {
	case 0:
		priv->flags &= ~FLAG_DDR_DEBUG;
		printf("Parameter ddr_debug clear in environment\n");
		break;
	case (u32)-1:
		break;
	default:
		printf("Parameter ddr_debug set in environment\n");
		priv->flags |= FLAG_DDR_DEBUG;
		priv->flags |= FLAG_DDR_VERBOSE;
		break;
	}
	if (env_get("ddr_prompt"))
		priv->flags |= FLAG_DDR_PROMPT;

	/* Force ddr_verbose for failsafe debugger */
	if (priv->flags & FLAG_FAILSAFE_MODE)
		priv->flags |= FLAG_DDR_VERBOSE;

#ifdef DDR_DEBUG
	priv->flags |= FLAG_DDR_DEBUG;
	/* Keep verbose on while we are still debugging. */
	priv->flags |= FLAG_DDR_VERBOSE;
#endif

	if ((octeon_is_cpuid(OCTEON_CN61XX) ||
	     octeon_is_cpuid(OCTEON_CNF71XX)) && ddr_max_speed > 533333333) {
		ddr_max_speed = 533333333;
	} else if (octeon_is_cpuid(OCTEON_CN7XXX)) {
		/* Override speed restrictions to support internal testing. */
		ddr_max_speed = 1210000000;
	}

	if (ddr_hertz > ddr_max_speed) {
		printf("DDR clock speed %u exceeds maximum supported DDR speed, reducing to %uHz\n",
		       ddr_hertz, ddr_max_speed);
		ddr_hertz = ddr_max_speed;
	}

	if (OCTEON_IS_OCTEON3()) {	// restrict check
		if (ddr_hertz > cpu_hertz) {
			printf("\nFATAL ERROR: DDR speed %u exceeds CPU speed %u, exiting...\n\n",
			       ddr_hertz, cpu_hertz);
			return -1;
		}
	}

	/* Enable L2 ECC */
	eptr = env_get("disable_l2_ecc");
	if (eptr) {
		printf("Disabling L2 ECC based on disable_l2_ecc environment variable\n");
		union cvmx_l2c_ctl l2c_val;

		l2c_val.u64 = l2c_rd(priv, CVMX_L2C_CTL_REL);
		l2c_val.s.disecc = 1;
		l2c_wr(priv, CVMX_L2C_CTL_REL, l2c_val.u64);
	} else {
		union cvmx_l2c_ctl l2c_val;

		l2c_val.u64 = l2c_rd(priv, CVMX_L2C_CTL_REL);
		l2c_val.s.disecc = 0;
		l2c_wr(priv, CVMX_L2C_CTL_REL, l2c_val.u64);
	}

	/*
	 * Init the L2C, must be done before DRAM access so that we
	 * know L2 is empty
	 */
	eptr = env_get("disable_l2_index_aliasing");
	if (eptr) {
		union cvmx_l2c_ctl l2c_val;

		puts("L2 index aliasing disabled.\n");

		l2c_val.u64 = l2c_rd(priv, CVMX_L2C_CTL_REL);
		l2c_val.s.disidxalias = 1;
		l2c_wr(priv, CVMX_L2C_CTL_REL, l2c_val.u64);
	} else {
		union cvmx_l2c_ctl l2c_val;

		/* Enable L2C index aliasing */

		l2c_val.u64 = l2c_rd(priv, CVMX_L2C_CTL_REL);
		l2c_val.s.disidxalias = 0;
		l2c_wr(priv, CVMX_L2C_CTL_REL, l2c_val.u64);
	}

	if (OCTEON_IS_OCTEON3()) {
		/*
		 * rdf_cnt: Defines the sample point of the LMC response data in
		 * the DDR-clock/core-clock crossing.  For optimal
		 * performance set to 10 * (DDR-clock period/core-clock
		 * period) - 1.  To disable set to 0. All other values
		 * are reserved.
		 */

		union cvmx_l2c_ctl l2c_ctl;
		u64 rdf_cnt;
		char *s;

		l2c_ctl.u64 = l2c_rd(priv, CVMX_L2C_CTL_REL);

		/*
		 * It is more convenient to compute the ratio using clock
		 * frequencies rather than clock periods.
		 */
		rdf_cnt = (((u64)10 * cpu_hertz) / ddr_hertz) - 1;
		rdf_cnt = rdf_cnt < 256 ? rdf_cnt : 255;
		l2c_ctl.cn78xx.rdf_cnt = rdf_cnt;

		s = lookup_env(priv, "early_fill_count");
		if (s)
			l2c_ctl.cn78xx.rdf_cnt = simple_strtoul(s, NULL, 0);

		debug("%-45s : %d, cpu_hertz:%d, ddr_hertz:%d\n",
		      "EARLY FILL COUNT  ", l2c_ctl.cn78xx.rdf_cnt, cpu_hertz,
		      ddr_hertz);
		l2c_wr(priv, CVMX_L2C_CTL_REL, l2c_ctl.u64);
	}

	/* Check for lower DIMM socket populated */
	for (if_idx = 0; if_idx < 4; ++if_idx) {
		if ((if_mask & (1 << if_idx)) &&
		    validate_dimm(priv,
				  &ddr_conf[(int)if_idx].dimm_config_table[0],
				  0))
			ddr_conf_valid_mask |= (1 << if_idx);
	}

	if (octeon_is_cpuid(OCTEON_CN68XX) || octeon_is_cpuid(OCTEON_CN78XX)) {
		int four_lmc_mode = 1;
		char *s;

		if (priv->flags & FLAG_FAILSAFE_MODE)
			four_lmc_mode = 0;

		/* Pass 1.0 disable four LMC mode.
		 *  See errata (LMC-15811)
		 */
		if (octeon_is_cpuid(OCTEON_CN68XX_PASS1_0))
			four_lmc_mode = 0;

		s = env_get("ddr_four_lmc");
		if (s) {
			four_lmc_mode = simple_strtoul(s, NULL, 0);
			printf("Parameter found in environment. ddr_four_lmc = %d\n",
			       four_lmc_mode);
		}

		if (!four_lmc_mode) {
			puts("Forcing two-LMC Mode.\n");
			/* Invalidate LMC[2:3] */
			ddr_conf_valid_mask &= ~(3 << 2);
		}
	} else if (octeon_is_cpuid(OCTEON_CN73XX)) {
		int one_lmc_mode = 0;
		char *s;

		s = env_get("ddr_one_lmc");
		if (s) {
			one_lmc_mode = simple_strtoul(s, NULL, 0);
			printf("Parameter found in environment. ddr_one_lmc = %d\n",
			       one_lmc_mode);
		}

		if (one_lmc_mode) {
			puts("Forcing one-LMC Mode.\n");
			/* Invalidate LMC[1:3] */
			ddr_conf_valid_mask &= ~(1 << 1);
		}
	}

	if (!ddr_conf_valid_mask) {
		printf
		    ("ERROR: No valid DIMMs detected on any DDR interface.\n");
		hang();
		return -1;	// testr-only: no ret negativ!!!
	}

	/*
	 * We measure the DDR frequency by counting DDR clocks.  We can
	 * confirm or adjust the expected frequency as necessary.  We use
	 * the measured frequency to make accurate timing calculations
	 * used to configure the controller.
	 */
	for (if_idx = 0; if_idx < 4; ++if_idx) {
		u32 tmp_hertz;

		if (!(ddr_conf_valid_mask & (1 << if_idx)))
			continue;

try_again:
		/*
		 * only check for alternate refclk wanted on chips that
		 * support it
		 */
		if ((octeon_is_cpuid(OCTEON_CN73XX)) ||
		    (octeon_is_cpuid(OCTEON_CNF75XX)) ||
		    (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X))) {
			// only need do this if we are LMC0
			if (if_idx == 0) {
				union cvmx_lmcx_ddr_pll_ctl ddr_pll_ctl;

				ddr_pll_ctl.u64 =
				    lmc_rd(priv, CVMX_LMCX_DDR_PLL_CTL(0));

				/*
				 * If we are asking for 100 MHz refclk, we can
				 * only get it via alternate, so switch to it
				 */
				if (ddr_ref_hertz == 100000000) {
					ddr_pll_ctl.cn78xx.dclk_alt_refclk_sel =
					    1;
					lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(0),
					       ddr_pll_ctl.u64);
					udelay(1000);	// wait 1 msec
				} else {
					/*
					 * If we are NOT asking for 100MHz,
					 * then reset to (assumed) 50MHz and go
					 * on
					 */
					ddr_pll_ctl.cn78xx.dclk_alt_refclk_sel =
					    0;
					lmc_wr(priv, CVMX_LMCX_DDR_PLL_CTL(0),
					       ddr_pll_ctl.u64);
					udelay(1000);	// wait 1 msec
				}
			}
		} else {
			if (ddr_ref_hertz == 100000000) {
				debug("N0: DRAM init: requested 100 MHz refclk NOT SUPPORTED\n");
				ddr_ref_hertz = CFG_REF_HERTZ;
			}
		}

		tmp_hertz = measure_octeon_ddr_clock(priv, &ddr_conf[if_idx],
						     cpu_hertz, ddr_hertz,
						     ddr_ref_hertz, if_idx,
						     ddr_conf_valid_mask);

		/*
		 * only check for alternate refclk acquired on chips that
		 * support it
		 */
		if ((octeon_is_cpuid(OCTEON_CN73XX)) ||
		    (octeon_is_cpuid(OCTEON_CNF75XX)) ||
		    (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X))) {
			/*
			 * if we are LMC0 and we are asked for 100 MHz refclk,
			 * we must be sure it is available
			 * If not, we print an error message, set to 50MHz,
			 * and go on...
			 */
			if (if_idx == 0 && ddr_ref_hertz == 100000000) {
				/*
				 * Validate that the clock returned is close
				 * enough to the clock desired
				 */
				// FIXME: is 5% close enough?
				int hertz_diff =
				    abs((int)tmp_hertz - (int)ddr_hertz);
				if (hertz_diff > ((int)ddr_hertz * 5 / 100)) {
					// nope, diff is greater than than 5%
					debug("N0: DRAM init: requested 100 MHz refclk NOT FOUND\n");
					ddr_ref_hertz = CFG_REF_HERTZ;
					// clear the flag before trying again!!
					set_ddr_clock_initialized(priv, 0, 0);
					goto try_again;
				} else {
					debug("N0: DRAM Init: requested 100 MHz refclk FOUND and SELECTED\n");
				}
			}
		}

		if (tmp_hertz > 0)
			calc_ddr_hertz = tmp_hertz;
		debug("LMC%d: measured speed: %u hz\n", if_idx, tmp_hertz);
	}

	if (measured_ddr_hertz)
		*measured_ddr_hertz = calc_ddr_hertz;

	memsize_mbytes = 0;
	for (if_idx = 0; if_idx < 4; ++if_idx) {
		if (!(ddr_conf_valid_mask & (1 << if_idx)))
			continue;

		ret = init_octeon_dram_interface(priv, &ddr_conf[if_idx],
						 calc_ddr_hertz,
						 cpu_hertz, ddr_ref_hertz,
						 if_idx, ddr_conf_valid_mask);
		if (ret > 0)
			memsize_mbytes += ret;
	}

	if (memsize_mbytes == 0)
		/* All interfaces failed to initialize, so return error */
		return -1;

	/*
	 * switch over to DBI mode only for chips that support it, and
	 * enabled by envvar
	 */
	if ((octeon_is_cpuid(OCTEON_CN73XX)) ||
	    (octeon_is_cpuid(OCTEON_CNF75XX)) ||
	    (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X))) {
		eptr = env_get("ddr_dbi_switchover");
		if (eptr) {
			printf("DBI Switchover starting...\n");
			cvmx_dbi_switchover(priv);
			printf("DBI Switchover finished.\n");
		}
	}

	/* call HW-assist tuning here on chips that support it */
	if ((octeon_is_cpuid(OCTEON_CN73XX)) ||
	    (octeon_is_cpuid(OCTEON_CNF75XX)) ||
	    (octeon_is_cpuid(OCTEON_CN78XX_PASS2_X)))
		cvmx_maybe_tune_node(priv, calc_ddr_hertz);

	eptr = env_get("limit_dram_mbytes");
	if (eptr) {
		unsigned int mbytes = dectoul(eptr, NULL);

		if (mbytes > 0) {
			memsize_mbytes = mbytes;
			printf("Limiting DRAM size to %d MBytes based on limit_dram_mbytes env. variable\n",
			       mbytes);
		}
	}

	debug("LMC Initialization complete. Total DRAM %d MB\n",
	      memsize_mbytes);

	return memsize_mbytes;
}

static int octeon_ddr_probe(struct udevice *dev)
{
	struct ddr_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args l2c_node;
	struct ddr_conf *ddr_conf_ptr;
	u32 ddr_conf_valid_mask = 0;
	u32 measured_ddr_hertz = 0;
	int conf_table_count;
	int def_ddr_freq;
	u32 mem_mbytes = 0;
	u32 ddr_hertz;
	u32 ddr_ref_hertz;
	int alt_refclk;
	const char *eptr;
	fdt_addr_t addr;
	u64 *ptr;
	u64 val;
	int ret;
	int i;

	/* Don't try to re-init the DDR controller after relocation */
	if (gd->flags & GD_FLG_RELOC)
		return 0;

	/*
	 * Dummy read all local variables into cache, so that they are
	 * locked in cache when the DDR code runs with flushes etc enabled
	 */
	ptr = (u64 *)_end;
	for (i = 0; i < (0x100000 / sizeof(u64)); i++)
		val = readq(ptr++);

	/*
	 * The base addresses of LMC and L2C are read from the DT. This
	 * makes it possible to use the DDR init code without the need
	 * of the "node" variable, describing on which node to access. The
	 * node number is already included implicitly in the base addresses
	 * read from the DT this way.
	 */

	/* Get LMC base address */
	priv->lmc_base = dev_remap_addr(dev);
	debug("%s: lmc_base=%p\n", __func__, priv->lmc_base);

	/* Get L2C base address */
	ret = dev_read_phandle_with_args(dev, "l2c-handle", NULL, 0, 0,
					 &l2c_node);
	if (ret) {
		printf("Can't access L2C node!\n");
		return -ENODEV;
	}

	addr = ofnode_get_addr(l2c_node.node);
	if (addr == FDT_ADDR_T_NONE) {
		printf("Can't access L2C node!\n");
		return -ENODEV;
	}

	priv->l2c_base = map_physmem(addr, 0, MAP_NOCACHE);
	debug("%s: l2c_base=%p\n", __func__, priv->l2c_base);

	ddr_conf_ptr = octeon_ddr_conf_table_get(&conf_table_count,
						 &def_ddr_freq);
	if (!ddr_conf_ptr) {
		printf("ERROR: unable to determine DDR configuration\n");
		return -ENODEV;
	}

	for (i = 0; i < conf_table_count; i++) {
		if (ddr_conf_ptr[i].dimm_config_table[0].spd_addrs[0] ||
		    ddr_conf_ptr[i].dimm_config_table[0].spd_ptrs[0])
			ddr_conf_valid_mask |= 1 << i;
	}

	/*
	 * Check for special case of mismarked 3005 samples,
	 * and adjust cpuid
	 */
	alt_refclk = 0;
	ddr_hertz = def_ddr_freq * 1000000;

	eptr = env_get("ddr_clock_hertz");
	if (eptr) {
		ddr_hertz = simple_strtoul(eptr, NULL, 0);
		gd->mem_clk = divide_nint(ddr_hertz, 1000000);
		printf("Parameter found in environment. ddr_clock_hertz = %d\n",
		       ddr_hertz);
	}

	ddr_ref_hertz = octeon3_refclock(alt_refclk,
					 ddr_hertz,
					 &ddr_conf_ptr[0].dimm_config_table[0]);

	debug("Initializing DDR, clock = %uhz, reference = %uhz\n",
	      ddr_hertz, ddr_ref_hertz);

	mem_mbytes = octeon_ddr_initialize(priv, gd->cpu_clk,
					   ddr_hertz, ddr_ref_hertz,
					   ddr_conf_valid_mask,
					   ddr_conf_ptr, &measured_ddr_hertz);
	debug("Mem size in MBYTES: %u\n", mem_mbytes);

	gd->mem_clk = divide_nint(measured_ddr_hertz, 1000000);

	debug("Measured DDR clock %d Hz\n", measured_ddr_hertz);

	if (measured_ddr_hertz != 0) {
		if (!gd->mem_clk) {
			/*
			 * If ddr_clock not set, use measured clock
			 * and don't warn
			 */
			gd->mem_clk = divide_nint(measured_ddr_hertz, 1000000);
		} else if ((measured_ddr_hertz > ddr_hertz + 3000000) ||
			   (measured_ddr_hertz < ddr_hertz - 3000000)) {
			printf("\nWARNING:\n");
			printf("WARNING: Measured DDR clock mismatch!  expected: %lld MHz, measured: %lldMHz, cpu clock: %lu MHz\n",
			       divide_nint(ddr_hertz, 1000000),
			       divide_nint(measured_ddr_hertz, 1000000),
			       gd->cpu_clk);
			printf("WARNING:\n\n");
			gd->mem_clk = divide_nint(measured_ddr_hertz, 1000000);
		}
	}

	if (!mem_mbytes)
		return -ENODEV;

	priv->info.base = CFG_SYS_SDRAM_BASE;
	priv->info.size = MB(mem_mbytes);

	/*
	 * For 6XXX generate a proper error when reading/writing
	 * non-existent memory locations.
	 */
	cvmx_l2c_set_big_size(priv, mem_mbytes, 0);

	debug("Ram size %uMiB\n", mem_mbytes);

	return 0;
}

static int octeon_get_info(struct udevice *dev, struct ram_info *info)
{
	struct ddr_priv *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops octeon_ops = {
	.get_info = octeon_get_info,
};

static const struct udevice_id octeon_ids[] = {
	{.compatible = "cavium,octeon-7xxx-ddr4" },
	{ }
};

U_BOOT_DRIVER(octeon_ddr) = {
	.name = "octeon_ddr",
	.id = UCLASS_RAM,
	.of_match = octeon_ids,
	.ops = &octeon_ops,
	.probe = octeon_ddr_probe,
	.plat_auto = sizeof(struct ddr_priv),
};
