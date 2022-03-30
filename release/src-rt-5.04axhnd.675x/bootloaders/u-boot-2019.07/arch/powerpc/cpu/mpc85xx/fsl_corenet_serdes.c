// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
#include <hwconfig.h>
#endif
#include <asm/fsl_serdes.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/fsl_law.h>
#include <linux/errno.h>
#include "fsl_corenet_serdes.h"

/*
 * The work-arounds for erratum SERDES8 and SERDES-A001 are linked together.
 * The code is already very complicated as it is, and separating the two
 * completely would just make things worse.  We try to keep them as separate
 * as possible, but for now we require SERDES8 if SERDES_A001 is defined.
 */
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES_A001
#ifndef CONFIG_SYS_P4080_ERRATUM_SERDES8
#error "CONFIG_SYS_P4080_ERRATUM_SERDES_A001 requires CONFIG_SYS_P4080_ERRATUM_SERDES8"
#endif
#endif

static u32 serdes_prtcl_map;

#ifdef DEBUG
static const char *serdes_prtcl_str[] = {
	[NONE] = "NA",
	[PCIE1] = "PCIE1",
	[PCIE2] = "PCIE2",
	[PCIE3] = "PCIE3",
	[PCIE4] = "PCIE4",
	[SATA1] = "SATA1",
	[SATA2] = "SATA2",
	[SRIO1] = "SRIO1",
	[SRIO2] = "SRIO2",
	[SGMII_FM1_DTSEC1] = "SGMII_FM1_DTSEC1",
	[SGMII_FM1_DTSEC2] = "SGMII_FM1_DTSEC2",
	[SGMII_FM1_DTSEC3] = "SGMII_FM1_DTSEC3",
	[SGMII_FM1_DTSEC4] = "SGMII_FM1_DTSEC4",
	[SGMII_FM1_DTSEC5] = "SGMII_FM1_DTSEC5",
	[SGMII_FM2_DTSEC1] = "SGMII_FM2_DTSEC1",
	[SGMII_FM2_DTSEC2] = "SGMII_FM2_DTSEC2",
	[SGMII_FM2_DTSEC3] = "SGMII_FM2_DTSEC3",
	[SGMII_FM2_DTSEC4] = "SGMII_FM2_DTSEC4",
	[SGMII_FM2_DTSEC5] = "SGMII_FM2_DTSEC5",
	[XAUI_FM1] = "XAUI_FM1",
	[XAUI_FM2] = "XAUI_FM2",
	[AURORA] = "DEBUG",
};
#endif

static const struct {
	int idx;
	unsigned int lpd; /* RCW lane powerdown bit */
	int bank;
} lanes[SRDS_MAX_LANES] = {
	{ 0, 152, FSL_SRDS_BANK_1 },
	{ 1, 153, FSL_SRDS_BANK_1 },
	{ 2, 154, FSL_SRDS_BANK_1 },
	{ 3, 155, FSL_SRDS_BANK_1 },
	{ 4, 156, FSL_SRDS_BANK_1 },
	{ 5, 157, FSL_SRDS_BANK_1 },
	{ 6, 158, FSL_SRDS_BANK_1 },
	{ 7, 159, FSL_SRDS_BANK_1 },
	{ 8, 160, FSL_SRDS_BANK_1 },
	{ 9, 161, FSL_SRDS_BANK_1 },
	{ 16, 162, FSL_SRDS_BANK_2 },
	{ 17, 163, FSL_SRDS_BANK_2 },
	{ 18, 164, FSL_SRDS_BANK_2 },
	{ 19, 165, FSL_SRDS_BANK_2 },
#ifdef CONFIG_ARCH_P4080
	{ 20, 170, FSL_SRDS_BANK_3 },
	{ 21, 171, FSL_SRDS_BANK_3 },
	{ 22, 172, FSL_SRDS_BANK_3 },
	{ 23, 173, FSL_SRDS_BANK_3 },
#else
	{ 20, 166, FSL_SRDS_BANK_3 },
	{ 21, 167, FSL_SRDS_BANK_3 },
	{ 22, 168, FSL_SRDS_BANK_3 },
	{ 23, 169, FSL_SRDS_BANK_3 },
#endif
#if SRDS_MAX_BANK > 3
	{ 24, 175, FSL_SRDS_BANK_4 },
	{ 25, 176, FSL_SRDS_BANK_4 },
#endif
};

int serdes_get_lane_idx(int lane)
{
	return lanes[lane].idx;
}

int serdes_get_bank_by_lane(int lane)
{
	return lanes[lane].bank;
}

int serdes_lane_enabled(int lane)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	serdes_corenet_t *regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;

	int bank = lanes[lane].bank;
	int word = lanes[lane].lpd / 32;
	int bit = lanes[lane].lpd % 32;

	if (in_be32(&regs->bank[bank].rstctl) & SRDS_RSTCTL_SDPD)
		return 0;

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	/*
	 * For banks two and three, use the srds_lpd_b[] array instead of the
	 * RCW, because this array contains the real values of SRDS_LPD_B2 and
	 * SRDS_LPD_B3.
	 */
	if (bank > 0)
		return !(srds_lpd_b[bank] & (8 >> (lane - (6 + 4 * bank))));
#endif

	return !(in_be32(&gur->rcwsr[word]) & (0x80000000 >> bit));
}

int is_serdes_configured(enum srds_prtcl device)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* Is serdes enabled at all? */
	if (!(in_be32(&gur->rcwsr[5]) & FSL_CORENET_RCWSR5_SRDS_EN))
		return 0;

	if (!(serdes_prtcl_map & (1 << NONE)))
		fsl_serdes_init();

	return (1 << device) & serdes_prtcl_map;
}

static int __serdes_get_first_lane(uint32_t prtcl, enum srds_prtcl device)
{
	int i;

	for (i = 0; i < SRDS_MAX_LANES; i++) {
		if (serdes_get_prtcl(prtcl, i) == device)
			return i;
	}

	return -ENODEV;
}

/*
 * Returns the SERDES lane (0..SRDS_MAX_LANES-1) that routes to the given
 * device. This depends on the current SERDES protocol, as defined in the RCW.
 *
 * Returns a negative error code if SERDES is disabled or the given device is
 * not supported in the current SERDES protocol.
 */
int serdes_get_first_lane(enum srds_prtcl device)
{
	u32 prtcl;
	const ccsr_gur_t *gur;

	gur = (typeof(gur))CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Is serdes enabled at all? */
	if (unlikely((in_be32(&gur->rcwsr[5]) & 0x2000) == 0))
		return -ENODEV;

	prtcl = (in_be32(&gur->rcwsr[4]) & FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;

	return __serdes_get_first_lane(prtcl, device);
}

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES9
/*
 * Returns the SERDES bank (1, 2, or 3) that a given device is on for a given
 * SERDES protocol.
 *
 * Returns a negative error code if the given device is not supported for the
 * given SERDES protocol.
 */
static int serdes_get_bank_by_device(uint32_t prtcl, enum srds_prtcl device)
{
	int lane;

	lane = __serdes_get_first_lane(prtcl, device);
	if (unlikely(lane < 0))
		return lane;

	return serdes_get_bank_by_lane(lane);
}

static uint32_t __serdes_get_lane_count(uint32_t prtcl, enum srds_prtcl device,
					int first)
{
	int lane;

	for (lane = first; lane < SRDS_MAX_LANES; lane++) {
		if (serdes_get_prtcl(prtcl, lane) != device)
			break;
	}

	return lane - first;
}

static void __serdes_reset_rx(serdes_corenet_t *regs,
			      uint32_t prtcl,
			      enum srds_prtcl device)
{
	int lane, idx, first, last;

	lane = __serdes_get_first_lane(prtcl, device);
	if (unlikely(lane < 0))
		return;
	first = serdes_get_lane_idx(lane);
	last = first + __serdes_get_lane_count(prtcl, device, lane);

	/*
	 * Set BnGCRy0[RRST] = 0 for each lane in the each bank that is
	 * selected as XAUI to place the lane into reset.
	*/
	for (idx = first; idx < last; idx++)
		clrbits_be32(&regs->lane[idx].gcr0, SRDS_GCR0_RRST);

	/* Wait at least 250 ns */
	udelay(1);

	/*
	 * Set BnGCRy0[RRST] = 1 for each lane in the each bank that is
	 * selected as XAUI to bring the lane out of reset.
	 */
	for (idx = first; idx < last; idx++)
		setbits_be32(&regs->lane[idx].gcr0, SRDS_GCR0_RRST);
}

void serdes_reset_rx(enum srds_prtcl device)
{
	u32 prtcl;
	const ccsr_gur_t *gur;
	serdes_corenet_t *regs;

	if (unlikely(device == NONE))
		return;

	gur = (typeof(gur))CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Is serdes enabled at all? */
	if (unlikely((in_be32(&gur->rcwsr[5]) & 0x2000) == 0))
		return;

	regs = (typeof(regs))CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	prtcl = (in_be32(&gur->rcwsr[4]) & FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;

	__serdes_reset_rx(regs, prtcl, device);
}
#endif

#ifndef CONFIG_SYS_DCSRBAR_PHYS
#define CONFIG_SYS_DCSRBAR_PHYS	0x80000000 /* Must be 1GB-aligned for rev1.0 */
#define CONFIG_SYS_DCSRBAR	0x80000000
#define __DCSR_NOT_DEFINED_BY_CONFIG
#endif

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
/*
 * Enable a SERDES bank that was disabled via the RCW
 *
 * We only call this function for SERDES8 and SERDES-A001 in cases we really
 * want to enable the bank, whether we actually want to use the lanes or not,
 * so make sure at least one lane is enabled.  We're only enabling this one
 * lane to satisfy errata requirements that the bank be enabled.
 *
 * We use a local variable instead of srds_lpd_b[] because we want drivers to
 * think that the lanes actually are disabled.
 */
static void enable_bank(ccsr_gur_t *gur, int bank)
{
	u32 rcw5;
	u32 temp_lpd_b = srds_lpd_b[bank];

	/*
	 * If we're asked to disable all lanes, just pretend we're doing
	 * that.
	 */
	if (temp_lpd_b == 0xF)
		temp_lpd_b = 0xE;

	/*
	 * Enable the lanes SRDS_LPD_Bn.  The RCW bits are read-only in
	 * CCSR, and read/write in DSCR.
	 */
	rcw5 = in_be32(gur->rcwsr + 5);
	if (bank == FSL_SRDS_BANK_2) {
		rcw5 &= ~FSL_CORENET_RCWSRn_SRDS_LPD_B2;
		rcw5 |= temp_lpd_b << 26;
	} else if (bank == FSL_SRDS_BANK_3) {
		rcw5 &= ~FSL_CORENET_RCWSRn_SRDS_LPD_B3;
		rcw5 |= temp_lpd_b << 18;
	} else {
		printf("SERDES: enable_bank: bad bank %d\n", bank + 1);
		return;
	}

	/* See similar code in cpu/mpc85xx/cpu_init.c for an explanation
	 * of the DCSR mapping.
	 */
	{
#ifdef __DCSR_NOT_DEFINED_BY_CONFIG
		struct law_entry law = find_law(CONFIG_SYS_DCSRBAR_PHYS);
		int law_index;
		if (law.index == -1)
			law_index = set_next_law(CONFIG_SYS_DCSRBAR_PHYS,
						 LAW_SIZE_1M, LAW_TRGT_IF_DCSR);
		else
			set_law(law.index, CONFIG_SYS_DCSRBAR_PHYS, LAW_SIZE_1M,
				LAW_TRGT_IF_DCSR);
#endif
		u32 *p = (void *)CONFIG_SYS_DCSRBAR + 0x20114;
		out_be32(p, rcw5);
#ifdef __DCSR_NOT_DEFINED_BY_CONFIG
		if (law.index == -1)
			disable_law(law_index);
		else
			set_law(law.index, law.addr, law.size, law.trgt_id);
#endif
	}
}

/*
 * To avoid problems with clock jitter, rev 2 p4080 uses the pll from
 * bank 3 to clock banks 2 and 3, as well as a limited selection of
 * protocol configurations.  This requires that banks 2 and 3's lanes be
 * disabled in the RCW, and enabled with some fixup here to re-enable
 * them, and to configure bank 2's clock parameters in bank 3's pll in
 * cases where they differ.
 */
static void p4080_erratum_serdes8(serdes_corenet_t *regs, ccsr_gur_t *gur,
				  u32 devdisr, u32 devdisr2, int cfg)
{
	int srds_ratio_b2;
	int rfck_sel;

	/*
	 * The disabled lanes of bank 2 will cause the associated
	 * logic blocks to be disabled in DEVDISR.  We reverse that here.
	 *
	 * Note that normally it is not permitted to clear DEVDISR bits
	 * once the device has been disabled, but the hardware people
	 * say that this special case is OK.
	 */
	clrbits_be32(&gur->devdisr, devdisr);
	clrbits_be32(&gur->devdisr2, devdisr2);

	/*
	 * Some protocols require special handling.  There are a few
	 * additional protocol configurations that can be used, which are
	 * not listed here.  See app note 4065 for supported protocol
	 * configurations.
	 */
	switch (cfg) {
	case 0x19:
		/*
		 * Bank 2 has PCIe which wants BWSEL -- tell bank 3's PLL.
		 * SGMII on bank 3 should still be usable.
		 */
		setbits_be32(&regs->bank[FSL_SRDS_BANK_3].pllcr1,
			     SRDS_PLLCR1_PLL_BWSEL);
		break;

	case 0x0f:
	case 0x10:
		/*
		 * Banks 2 (XAUI) and 3 (SGMII) have different clocking
		 * requirements in these configurations.  Bank 3 cannot
		 * be used and should have its lanes (but not the bank
		 * itself) disabled in the RCW.  We set up bank 3's pll
		 * for bank 2's needs here.
		 */
		srds_ratio_b2 = (in_be32(&gur->rcwsr[4]) >> 13) & 7;

		/* Determine refclock from XAUI ratio */
		switch (srds_ratio_b2) {
		case 1: /* 20:1 */
			rfck_sel = SRDS_PLLCR0_RFCK_SEL_156_25;
			break;
		case 2: /* 25:1 */
			rfck_sel = SRDS_PLLCR0_RFCK_SEL_125;
			break;
		default:
			printf("SERDES: bad SRDS_RATIO_B2 %d\n",
			       srds_ratio_b2);
			return;
		}

		clrsetbits_be32(&regs->bank[FSL_SRDS_BANK_3].pllcr0,
				SRDS_PLLCR0_RFCK_SEL_MASK, rfck_sel);

		clrsetbits_be32(&regs->bank[FSL_SRDS_BANK_3].pllcr0,
				SRDS_PLLCR0_FRATE_SEL_MASK,
				SRDS_PLLCR0_FRATE_SEL_6_25);
		break;
	}

	enable_bank(gur, FSL_SRDS_BANK_3);
}
#endif

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES_A005
/*
 * If PCIe is not selected as a protocol for any lanes driven by a given PLL,
 * that PLL should have SRDSBnPLLCR1[PLLBW_SEL] = 0.
 */
static void p4080_erratum_serdes_a005(serdes_corenet_t *regs, unsigned int cfg)
{
	enum srds_prtcl device;

	switch (cfg) {
	case 0x13:
	case 0x16:
		/*
		 * If SRDS_PRTCL = 0x13 or 0x16, set SRDSB1PLLCR1[PLLBW_SEL]
		 * to 0.
		 */
		clrbits_be32(&regs->bank[FSL_SRDS_BANK_1].pllcr1,
			     SRDS_PLLCR1_PLL_BWSEL);
		break;
	case 0x19:
		/*
		 * If SRDS_PRTCL = 0x19, set SRDSB1PLLCR1[PLLBW_SEL] to 0 and
		 * SRDSB3PLLCR1[PLLBW_SEL] to 1.
		 */
		clrbits_be32(&regs->bank[FSL_SRDS_BANK_1].pllcr1,
			     SRDS_PLLCR1_PLL_BWSEL);
		setbits_be32(&regs->bank[FSL_SRDS_BANK_3].pllcr1,
			     SRDS_PLLCR1_PLL_BWSEL);
		break;
	}

	/*
	 * Set SRDSBnPLLCR1[PLLBW_SEL] to 0 for each bank that selects XAUI
	 * before XAUI is initialized.
	 */
	for (device = XAUI_FM1; device <= XAUI_FM2; device++) {
		if (is_serdes_configured(device)) {
			int bank = serdes_get_bank_by_device(cfg, device);

			clrbits_be32(&regs->bank[bank].pllcr1,
				     SRDS_PLLCR1_PLL_BWSEL);
		}
	}
}
#endif

/*
 * Wait for the RSTDONE bit to get set, or a one-second timeout.
 */
static void wait_for_rstdone(unsigned int bank)
{
	serdes_corenet_t *srds_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	unsigned long long end_tick;
	u32 rstctl;

	/* wait for reset complete or 1-second timeout */
	end_tick = usec2ticks(1000000) + get_ticks();
	do {
		rstctl = in_be32(&srds_regs->bank[bank].rstctl);
		if (rstctl & SRDS_RSTCTL_RSTDONE)
			break;
	} while (end_tick > get_ticks());

	if (!(rstctl & SRDS_RSTCTL_RSTDONE))
		printf("SERDES: timeout resetting bank %u\n", bank + 1);
}


static void __soc_serdes_init(void)
{
	/* Allow for SoC-specific initialization in <SOC>_serdes.c  */
};
void soc_serdes_init(void) __attribute__((weak, alias("__soc_serdes_init")));

void fsl_serdes_init(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int cfg;
	serdes_corenet_t *srds_regs;
#ifdef CONFIG_ARCH_P5040
	serdes_corenet_t *srds2_regs;
#endif
	int lane, bank, idx;
	int have_bank[SRDS_MAX_BANK] = {};
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	u32 serdes8_devdisr = 0;
	u32 serdes8_devdisr2 = 0;
	char srds_lpd_opt[16];
	const char *srds_lpd_arg;
	size_t arglen;
#endif
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES_A001
	int need_serdes_a001;	/* true == need work-around for SERDES A001 */
#endif
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	char buffer[HWCONFIG_BUFFER_SIZE];
	char *buf = NULL;

	/*
	 * Extract hwconfig from environment since we have not properly setup
	 * the environment but need it for ddr config params
	 */
	if (env_get_f("hwconfig", buffer, sizeof(buffer)) > 0)
		buf = buffer;
#endif
	if (serdes_prtcl_map & (1 << NONE))
		return;

	/* Is serdes enabled at all? */
	if (!(in_be32(&gur->rcwsr[5]) & FSL_CORENET_RCWSR5_SRDS_EN))
		return;

	srds_regs = (void *)(CONFIG_SYS_FSL_CORENET_SERDES_ADDR);
	cfg = (in_be32(&gur->rcwsr[4]) & FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;
	debug("Using SERDES configuration 0x%x, lane settings:\n", cfg);

	if (!is_serdes_prtcl_valid(cfg)) {
		printf("SERDES[PRTCL] = 0x%x is not valid\n", cfg);
		return;
	}

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	/*
	 * Display a warning if banks two and three are not disabled in the RCW,
	 * since our work-around for SERDES8 depends on these banks being
	 * disabled at power-on.
	 */
#define B2_B3 (FSL_CORENET_RCWSRn_SRDS_LPD_B2 | FSL_CORENET_RCWSRn_SRDS_LPD_B3)
	if ((in_be32(&gur->rcwsr[5]) & B2_B3) != B2_B3) {
		printf("Warning: SERDES8 requires banks two and "
		       "three to be disabled in the RCW\n");
	}

	/*
	 * Store the values of the fsl_srds_lpd_b2 and fsl_srds_lpd_b3
	 * hwconfig options into the srds_lpd_b[] array.  See README.p4080ds
	 * for a description of these options.
	 */
	for (bank = 1; bank < ARRAY_SIZE(srds_lpd_b); bank++) {
		sprintf(srds_lpd_opt, "fsl_srds_lpd_b%u", bank + 1);
		srds_lpd_arg =
			hwconfig_subarg_f("serdes", srds_lpd_opt, &arglen, buf);
		if (srds_lpd_arg)
			srds_lpd_b[bank] =
				simple_strtoul(srds_lpd_arg, NULL, 0) & 0xf;
	}

	if ((cfg == 0xf) || (cfg == 0x10)) {
		/*
		 * For SERDES protocols 0xF and 0x10, force bank 3 to be
		 * disabled, because it is not supported.
		 */
		srds_lpd_b[FSL_SRDS_BANK_3] = 0xF;
	}
#endif

	/* Look for banks with all lanes disabled, and power down the bank. */
	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl = serdes_get_prtcl(cfg, lane);
		if (serdes_lane_enabled(lane)) {
			have_bank[serdes_get_bank_by_lane(lane)] = 1;
			serdes_prtcl_map |= (1 << lane_prtcl);
		}
	}

#ifdef CONFIG_ARCH_P5040
	/*
	 * Lanes on bank 4 on P5040 are commented-out, but for some SERDES
	 * protocols, these lanes are routed to SATA.  We use serdes_prtcl_map
	 * to decide whether a protocol is supported on a given lane, so SATA
	 * will be identified as not supported, and therefore not initialized.
	 * So for protocols which use SATA on bank4, we add SATA support in
	 * serdes_prtcl_map.
	 */
	switch (cfg) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		serdes_prtcl_map |= 1 << SATA1 | 1 << SATA2;
		break;
	default:
		srds2_regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES2_ADDR;

		/* We don't need bank 4, so power it down */
		setbits_be32(&srds2_regs->bank[0].rstctl, SRDS_RSTCTL_SDPD);
	}
#endif

	soc_serdes_init();

	/* Set the first bit to indicate serdes has been initialized */
	serdes_prtcl_map |= (1 << NONE);

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
	/*
	 * Bank two uses the clock from bank three, so if bank two is enabled,
	 * then bank three must also be enabled.
	 */
	if (have_bank[FSL_SRDS_BANK_2])
		have_bank[FSL_SRDS_BANK_3] = 1;
#endif

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES_A001
	/*
	 * The work-aroud for erratum SERDES-A001 is needed only if bank two
	 * is disabled and bank three is enabled.  The converse is also true,
	 * but SERDES8 ensures that bank 3 is always enabled if bank 2 is
	 * enabled, so there's no point in complicating the code to handle
	 * that situation.
	 */
	need_serdes_a001 =
		!have_bank[FSL_SRDS_BANK_2] && have_bank[FSL_SRDS_BANK_3];
#endif

	/* Power down the banks we're not interested in */
	for (bank = 0; bank < SRDS_MAX_BANK; bank++) {
		if (!have_bank[bank]) {
			printf("SERDES: bank %d disabled\n", bank + 1);
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES_A001
			/*
			 * Erratum SERDES-A001 says bank two needs to be powered
			 * down after bank three is powered up, so don't power
			 * down bank two here.
			 */
			if (!need_serdes_a001 || (bank != FSL_SRDS_BANK_2))
				setbits_be32(&srds_regs->bank[bank].rstctl,
					     SRDS_RSTCTL_SDPD);
#else
			setbits_be32(&srds_regs->bank[bank].rstctl,
				     SRDS_RSTCTL_SDPD);
#endif
		}
	}

#ifdef CONFIG_SYS_FSL_ERRATUM_A004699
	/*
	 * To avoid the situation that resulted in the P4080 erratum
	 * SERDES-8, a given SerDes bank will use the PLLs from the previous
	 * bank if one of the PLL frequencies is a multiple of the other.  For
	 * instance, if bank 3 is running at 2.5GHz and bank 2 is at 1.25GHz,
	 * then bank 3 will use bank 2's PLL.  P5040 Erratum A-004699 says
	 * that, in this situation, lane synchronization is not initiated.  So
	 * when we detect a bank with a "borrowed" PLL, we have to manually
	 * initiate lane synchronization.
	 */
	for (bank = FSL_SRDS_BANK_2; bank <= FSL_SRDS_BANK_3; bank++) {
		/* Determine the first lane for this bank */
		unsigned int lane;

		for (lane = 0; lane < SRDS_MAX_LANES; lane++)
			if (lanes[lane].bank == bank)
				break;
		idx = lanes[lane].idx;

		/*
		 * Check if the PLL for the bank is borrowed.  The UOTHL
		 * bit of the first lane will tell us that.
		 */
		if (in_be32(&srds_regs->lane[idx].gcr0) & SRDS_GCR0_UOTHL) {
			/* Manually start lane synchronization */
			setbits_be32(&srds_regs->bank[bank].pllcr0,
				     SRDS_PLLCR0_PVCOCNT_EN);
		}
	}
#endif

#if defined(CONFIG_SYS_P4080_ERRATUM_SERDES8) || defined (CONFIG_SYS_P4080_ERRATUM_SERDES9)
	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		enum srds_prtcl lane_prtcl;

		idx = serdes_get_lane_idx(lane);
		lane_prtcl = serdes_get_prtcl(cfg, lane);

#ifdef DEBUG
		switch (lane) {
		case 0:
			puts("Bank1: ");
			break;
		case 10:
			puts("\nBank2: ");
			break;
		case 14:
			puts("\nBank3: ");
			break;
		default:
			break;
		}

		printf("%s ", serdes_prtcl_str[lane_prtcl]);
#endif

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES9
		/*
		 * Set BnTTLCRy0[FLT_SEL] = 011011 and set BnTTLCRy0[31] = 1
		 * for each of the SerDes lanes selected as SGMII, XAUI, SRIO,
		 * or AURORA before the device is initialized.
		 *
		 * Note that this part of the SERDES-9 work-around is
		 * redundant if the work-around for A-4580 has already been
		 * applied via PBI.
		 */
		switch (lane_prtcl) {
		case SGMII_FM1_DTSEC1:
		case SGMII_FM1_DTSEC2:
		case SGMII_FM1_DTSEC3:
		case SGMII_FM1_DTSEC4:
		case SGMII_FM2_DTSEC1:
		case SGMII_FM2_DTSEC2:
		case SGMII_FM2_DTSEC3:
		case SGMII_FM2_DTSEC4:
		case SGMII_FM2_DTSEC5:
		case XAUI_FM1:
		case XAUI_FM2:
		case SRIO1:
		case SRIO2:
		case AURORA:
			out_be32(&srds_regs->lane[idx].ttlcr0,
				 SRDS_TTLCR0_FLT_SEL_KFR_26 |
				 SRDS_TTLCR0_FLT_SEL_KPH_28 |
				 SRDS_TTLCR0_FLT_SEL_750PPM |
				 SRDS_TTLCR0_FREQOVD_EN);
			break;
		default:
			break;
		}
#endif

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
		switch (lane_prtcl) {
		case PCIE1:
		case PCIE2:
		case PCIE3:
			serdes8_devdisr |= FSL_CORENET_DEVDISR_PCIE1 >>
					   (lane_prtcl - PCIE1);
			break;
		case SRIO1:
		case SRIO2:
			serdes8_devdisr |= FSL_CORENET_DEVDISR_SRIO1 >>
					   (lane_prtcl - SRIO1);
			break;
		case SGMII_FM1_DTSEC1:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1 |
					    FSL_CORENET_DEVDISR2_DTSEC1_1;
			break;
		case SGMII_FM1_DTSEC2:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1 |
					    FSL_CORENET_DEVDISR2_DTSEC1_2;
			break;
		case SGMII_FM1_DTSEC3:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1 |
					    FSL_CORENET_DEVDISR2_DTSEC1_3;
			break;
		case SGMII_FM1_DTSEC4:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1 |
					    FSL_CORENET_DEVDISR2_DTSEC1_4;
			break;
		case SGMII_FM2_DTSEC1:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_1;
			break;
		case SGMII_FM2_DTSEC2:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_2;
			break;
		case SGMII_FM2_DTSEC3:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_3;
			break;
		case SGMII_FM2_DTSEC4:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_4;
			break;
		case SGMII_FM2_DTSEC5:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2 |
					    FSL_CORENET_DEVDISR2_DTSEC2_5;
			break;
		case XAUI_FM1:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM1	|
					    FSL_CORENET_DEVDISR2_10GEC1;
			break;
		case XAUI_FM2:
			serdes8_devdisr2 |= FSL_CORENET_DEVDISR2_FM2	|
					    FSL_CORENET_DEVDISR2_10GEC2;
			break;
		case AURORA:
			break;
		default:
			break;
		}

#endif
	}
#endif

#ifdef DEBUG
	puts("\n");
#endif

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES_A005
	p4080_erratum_serdes_a005(srds_regs, cfg);
#endif

	for (idx = 0; idx < SRDS_MAX_BANK; idx++) {
		bank = idx;

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
		/*
		 * Change bank init order to 0, 2, 1, so that the third bank's
		 * PLL is established before we start the second bank.  The
		 * second bank uses the third bank's PLL.
		 */

		if (idx == 1)
			bank = FSL_SRDS_BANK_3;
		else if (idx == 2)
			bank = FSL_SRDS_BANK_2;
#endif

		/* Skip disabled banks */
		if (!have_bank[bank])
			continue;

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
		if (idx == 1) {
			/*
			 * Re-enable devices on banks two and three that were
			 * disabled by the RCW, and then enable bank three. The
			 * devices need to be enabled before either bank is
			 * powered up.
			 */
			p4080_erratum_serdes8(srds_regs, gur, serdes8_devdisr,
					      serdes8_devdisr2, cfg);
		} else if (idx == 2) {
			/* Enable bank two now that bank three is enabled. */
			enable_bank(gur, FSL_SRDS_BANK_2);
		}
#endif

		wait_for_rstdone(bank);
	}

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES_A001
	if (need_serdes_a001) {
		/* Bank 3 has been enabled, so now we can disable bank 2 */
		setbits_be32(&srds_regs->bank[FSL_SRDS_BANK_2].rstctl,
			     SRDS_RSTCTL_SDPD);
	}
#endif
}

const char *serdes_clock_to_string(u32 clock)
{
	switch (clock) {
	case SRDS_PLLCR0_RFCK_SEL_100:
		return "100";
	case SRDS_PLLCR0_RFCK_SEL_125:
		return "125";
	case SRDS_PLLCR0_RFCK_SEL_156_25:
		return "156.25";
	case SRDS_PLLCR0_RFCK_SEL_161_13:
		return "161.1328123";
	default:
		return "150";
	}
}

