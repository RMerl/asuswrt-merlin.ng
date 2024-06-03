/*
 * Misc utility routines for accessing PMU corerev specific features
 * of the SiliconBackplane-based Broadcom chips.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hndpmu.c 832034 2023-11-01 03:54:25Z $
 */

/**
 * @file
 * Note: this file contains PLL/FLL related functions. A chip can contain multiple PLLs/FLLs.
 * However, in the context of this file the baseband ('BB') PLL/FLL is referred to.
 *
 * Throughout this code, the prefixes 'pmu1_' and 'pmu2_' are used.
 * They refer to different revisions of the PMU (which is at revision 18 @ Apr 25, 2012)
 * pmu1_ marks the transition from PLL to ADFLL (Digital Frequency Locked Loop). It supports
 * fractional frequency generation. pmu2_ does not support fractional frequency generation.
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcm_math.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <hndpmu.h>
#if defined DONGLEBUILD
#include <hndcpu.h>
#ifdef __arm__
#include <hndarm.h>
#endif
#endif /* DONGLEBUILD */
#include <bcmotp.h>
#include <sbgci.h>
#include <sbgci.h>
#include <lpflags.h>
#if defined(DONGLEBUILD) && defined(BCMHWA)
#include <hwa_export.h>
#endif

#if defined(BCMDBG_ERR)
#define	PMU_ERROR(args)	printf args
#else
#define	PMU_ERROR(args)
#endif	/* defined(BCMDBG_ERR) */

#ifdef BCMDBG
#if !defined(PMU_MSG)  /* over-rideable */
#define	PMU_MSG(args)	printf args
#endif /* PMU_MSG */
#else
#define	PMU_MSG(args)
#endif	/* BCMDBG */

/* To check in verbose debugging messages not intended
 * to be on except on private builds.
 */
#define	PMU_NONE(args)
#define flags_shift	14

/** contains resource bit positions for a specific chip */
struct rsc_per_chip_s {
	uint8 ht_avail;
	uint8 macphy_clkavail;
	uint8 ht_start;
	uint8 otp_pu;
	uint8 macphy_aux_clkavail;
};

typedef struct rsc_per_chip_s rsc_per_chip_t;

#if defined(BCMPMU_STATS) && !defined(BCMPMU_STATS_DISABLED)
bool	_pmustatsenab = TRUE;
#else
bool	_pmustatsenab = FALSE;
#endif /* BCMPMU_STATS */

#if defined(DONGLEBUILD)
/* PLL controls/clocks */
static void si_pmu_pll_off(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 *min_mask,
	uint32 *max_mask, uint32 *clk_ctl_st);
static void si_pmu_pll_on(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 min_mask,
	uint32 max_mask, uint32 clk_ctl_st);
static uint32 si_pmu_htclk_mask(si_t *sih);
#endif /* DONGLEBUILD */

void si_pmu_otp_pllcontrol(si_t *sih, osl_t *osh);
void si_pmu_otp_vreg_control(si_t *sih, osl_t *osh);
void si_pmu_otp_chipcontrol(si_t *sih, osl_t *osh);

static uint32 si_pmu1_cpuclk0(si_t *sih, osl_t *osh, pmuregs_t *pmu);
static uint32 si_pmu_xtal_freq(si_t *sih, osl_t *osh, pmuregs_t *pmu);

/* PMU resources */
static uint32 si_pmu_res_deps(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 rsrcs, bool all);
static uint si_pmu_res_uptime(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint8 rsrc);
static void si_pmu_res_masks(si_t *sih, uint32 *pmin, uint32 *pmax);
static void si_pmu_spuravoid_pllupdate(si_t *sih, pmuregs_t *pmu, osl_t *osh, uint8 spuravoid);
static uint32 si_pmu_backplane_clk_hz(si_t *sih, osl_t *osh);
static uint32 si_pmu_get_xtal_or_alp_clock(si_t *sih, osl_t *osh, pmuregs_t *pmu,
                                           enum si_pmu_clk_e clk_type);
static uint32 si_pmu_ilp_clock(si_t *sih, osl_t *osh, pmuregs_t *pmu);  /* returns [Hz] units */
static uint32 si_pmu_cpu_clock(si_t *sih, osl_t *osh, pmuregs_t *pmu);  /* returns [Hz] units */
#ifdef DONGLEBUILD
static uint32 si_pmu_mem_clock(si_t *sih, osl_t *osh, pmuregs_t *pmu);  /* returns [Hz] units */
#endif /* DONGLEBUILD */

#ifndef WLTEST
static int si_pmu_min_res_set(si_t *sih, osl_t *osh, uint min_mask, bool set);
#endif
uint32 si_pmu_get_pmutime_diff(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 *prev);
bool si_pmu_wait_for_res_pending(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint usec,
	bool cond, uint32 *elapsed_time);

/* PMU timer ticks once in 32uS */
#define PMU_US_STEPS (32)

/* FVCO frequency in [KHz] */
#define FVCO_640	640000  /**< 640MHz */
#define FVCO_880	880000	/**< 880MHz */
#define FVCO_1760	1760000	/**< 1760MHz */
#define FVCO_1440	1440000	/**< 1440MHz */
#define FVCO_960	960000	/**< 960MHz */
#define FVCO_960p1	960100  /**< 960.1MHz */
#define FVCO_960010	960010	/**< 960.0098MHz */
#define FVCO_961	961000	/**< 961MHz */
#define FVCO_960p5	960500	/**< 960.5MHz */
#define FVCO_963	963000	/**< 963MHz */
#define FVCO_1600	1600000	/**< 1600MHz */
#define FVCO_1920	1920000	/**< 1920MHz */
#define FVCO_1938	1938000 /**< 1938MHz */
#define FVCO_385	385000  /**< 385MHz */
#define FVCO_400	400000  /**< 420MHz */
#define FVCO_500	500000  /**< 500MHz */
#define FVCO_1000	1000000 /**< 1000MHz */
#define FVCO_2000	2000000 /**< 2000MHz */
#define FVCO_2700	2700000	/**< 2700 MHz */
#define FVCO_2880	2880000	/**< 2880 MHz */
#define FVCO_2907	2907005	/**< 2907 MHz */
#define FVCO_2946	2946000	/**< 2946 MHz */
#define FVCO_2952	2952000	/**< 2952 MHz */
#define FVCO_3000	3000000	/**< 3000 MHz */
#define FVCO_3200	3200000	/**< 3200 MHz */
#define FVCO_3300	3300000	/**< 3300 MHz */
#define FVCO_3600	3600000	/**< 3600 MHz */
#define FVCO_4400	4400000	/**< 4400 MHz */
#define FVCO_4800	4800000	/**< 4800 MHz */
#define FVCO_5130	5130000	/**< 5130 MHz */
#define FVCO_5200	5200000	/**< 5200 MHz */
#define FVCO_5440	5440000	/**< 5440 MHz */
#define FVCO_5814	5814000	/**< 5814 MHz */
#define FVCO_5840	5840000	/**< 5840 MHz */
#define FVCO_6000	6000000	/**< 6000 MHz */

/* defines to make the code more readable */
#define NO_SUCH_RESOURCE	0	/**< means: chip does not have such a PMU resource */

/**
 * Reads/writes a chipcontrol reg. Performes core switching if required, at function exit the
 * original core is restored. Depending on chip type, read/writes to chipcontrol regs in CC core
 * (older chips) or to chipcontrol regs in PMU core (later chips).
 */
uint32
si_pmu_chipcontrol(si_t *sih, uint reg, uint32 mask, uint32 val)
{
	pmu_corereg(sih, SI_CC_IDX, chipcontrol_addr, ~0, reg);
	return pmu_corereg(sih, SI_CC_IDX, chipcontrol_data, mask, val);
}

/**
 * Reads/writes a voltage regulator (vreg) register. Performes core switching if required, at
 * function exit the original core is restored. Depending on chip type, writes to regulator regs
 * in CC core (older chips) or to regulator regs in PMU core (later chips).
 */
uint32
si_pmu_vreg_control(si_t *sih, uint reg, uint32 mask, uint32 val)
{
	pmu_corereg(sih, SI_CC_IDX, regcontrol_addr, ~0, reg);
	return pmu_corereg(sih, SI_CC_IDX, regcontrol_data, mask, val);
}

/**
 * Reads/writes a PLL control register. Performs core switching if required, at function exit the
 * original core is restored. Depending on chip type, writes to PLL control regs in CC core (older
 * chips) or to PLL control regs in PMU core (later chips).
 */
uint32
si_pmu_pllcontrol(si_t *sih, enum pmu_pll_ctrl_reg_e reg, uint32 mask, uint32 val)
{
	pmu_corereg(sih, SI_CC_IDX, pllcontrol_addr, ~0, reg);
	return pmu_corereg(sih, SI_CC_IDX, pllcontrol_data, mask, val);
}

/**
 * The chip has one or more PLLs/FLLs (e.g. baseband PLL, PCIe PHY PLL). The settings of each PLL
 * are contained within one or more 'PLL control' registers. Since the PLL hardware requires that
 * changes for one PLL are committed at once, the PMU has a provision for 'updating' all PLL control
 * registers at once.
 *
 * When software wants to change the any PLL parameters, it withdraws requests for that PLL clock,
 * updates the PLL control registers being careful not to alter any control signals for the other
 * PLLs, and then writes a 1 to PMUCtl.PllCtnlUpdate to commit the changes. Best usage model would
 * be bring PLL down then update the PLL control register.
 */
void
si_pmu_pllupd(si_t *sih)
{
	pmu_corereg(sih, SI_CC_IDX, pmucontrol, PCTL_PLL_PLLCTL_UPD, PCTL_PLL_PLLCTL_UPD);
}

static rsc_per_chip_t rsc_43684 = {RES43684_HT_AVAIL,  RES43684_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

/**
 * 47622 and 6756 uses same top level and hence share resource table.
 * 6765 and 6766:2G/40MHz and 6766:160MHZ cores have the same definitions.
 */
static rsc_per_chip_t rsc_2x2ax = {RES2x2AX_HT_AVAIL, RES2x2AX_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

static rsc_per_chip_t rsc_6710 = {RES6710_HT_AVAIL,  RES6710_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

static rsc_per_chip_t rsc_6715 = {RES6715_HT_AVAIL,  RES6715_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

/* 6717 resource table also used for 6726 */
static rsc_per_chip_t rsc_6717 = {RES6717_HT_AVAIL,  RES6717_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

static rsc_per_chip_t rsc_6711 = {RES6711_HT_AVAIL,  RES6711_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};
/**
* For each chip, location of resource bits (e.g., ht bit) in resource mask registers may differ.
* This function abstracts the bit position of commonly used resources, thus making the rest of the
* code in hndpmu.c cleaner.
*/
static rsc_per_chip_t* BCMRAMFN(si_pmu_get_rsc_positions)(si_t *sih)
{
	rsc_per_chip_t *rsc = NULL;

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
		rsc = &rsc_43684;
		break;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_EMBEDDED_11BE_CORE:
		/* TODO:6766: PMU tables */
		rsc = &rsc_2x2ax;
		break;
	CASE_BCM6710_CHIP:
		rsc = &rsc_6710;
		break;
	CASE_BCM6715_CHIP:
		rsc = &rsc_6715;
		break;
	CASE_BCM6717_CHIP:
	CASE_BCM6726_CHIP:
		rsc = &rsc_6717;
		break;
	CASE_BCM6711_CHIP:
		rsc = &rsc_6711;
		break;
	default:
		ASSERT(0);
		break;
	}

	return rsc;
}; /* si_pmu_get_rsc_positions */

static const char BCMATTACHDATA(rstr_pllD)[] = "pll%d";
static const char BCMATTACHDATA(rstr_regD)[] = "reg%d";
static const char BCMATTACHDATA(rstr_chipcD)[] = "chipc%d";
static const char BCMATTACHDATA(rstr_rDt)[] = "r%dt";
static const char BCMATTACHDATA(rstr_rDd)[] = "r%dd";
static const char BCMATTACHDATA(rstr_xtalfreq)[] = "xtalfreq";

/* The check for OTP parameters for the PLL control registers is done and if found the
 * registers are updated accordingly.
 */

void
BCMATTACHFN(si_pmu_otp_pllcontrol)(si_t *sih, osl_t *osh)
{
	char name[16];
	const char *otp_val;
	uint8 i;
	uint32 val;
	uint8 pll_ctrlcnt = 0;

	pll_ctrlcnt = (sih->pmucaps & PCAP5_PC_MASK) >> PCAP5_PC_SHIFT;

	for (i = 0; i < pll_ctrlcnt; i++) {
		snprintf(name, sizeof(name), rstr_pllD, i);
		if ((otp_val = getvar(NULL, name)) == NULL)
			continue;

		val = (uint32)bcm_strtoul(otp_val, NULL, 0);
		si_pmu_pllcontrol(sih, i, ~0, val);
	}
}

/**
 * The check for OTP parameters for the Voltage Regulator registers is done and if found the
 * registers are updated accordingly.
 */
void
BCMATTACHFN(si_pmu_otp_vreg_control)(si_t *sih, osl_t *osh)
{
	char name[16];
	const char *otp_val;
	uint8 i;
	uint32 val;
	uint8 vreg_ctrlcnt = 0;

	vreg_ctrlcnt = (sih->pmucaps & PCAP5_VC_MASK) >> PCAP5_VC_SHIFT;

	for (i = 0; i < vreg_ctrlcnt; i++) {
		snprintf(name, sizeof(name), rstr_regD, i);
		if ((otp_val = getvar(NULL, name)) == NULL)
			continue;

		val = (uint32)bcm_strtoul(otp_val, NULL, 0);
		si_pmu_vreg_control(sih, i, ~0, val);
	}
}

/**
 * The check for OTP parameters for the chip control registers is done and if found the
 * registers are updated accordingly.
 */
void
BCMATTACHFN(si_pmu_otp_chipcontrol)(si_t *sih, osl_t *osh)
{
	uint32 val, cc_ctrlcnt, i;
	char name[16];
	const char *otp_val;

	cc_ctrlcnt = (sih->pmucaps & PCAP5_CC_MASK) >> PCAP5_CC_SHIFT;

	for (i = 0; i < cc_ctrlcnt; i++) {
		snprintf(name, sizeof(name), rstr_chipcD, i);
		if ((otp_val = getvar(NULL, name)) == NULL)
			continue;

		val = (uint32)bcm_strtoul(otp_val, NULL, 0);
		si_pmu_chipcontrol(sih, i, 0xFFFFFFFF, val); /* writes to PMU chipctrl reg 'i' */
	}
}

/**
 * A chip contains one or more LDOs (Low Drop Out regulators). During chip bringup, it can turn out
 * that the default (POR) voltage of a regulator is not right or optimal.
 * This function is called only by si_pmu_swreg_init() for specific chips
 */
void
si_pmu_set_ldo_voltage(si_t *sih, osl_t *osh, uint8 ldo, uint8 voltage)
{
	/* This function should not be used for 11ax and later (it used to assert), but it is
	 * referenced by PHY Compononent based upon nvram setting (so never really used). Keep
	 * the function, but just ASSERT to make clear this is not implemented.
	 */

	ASSERT(FALSE);
} /* si_pmu_set_ldo_voltage */

/* d11 slow to fast clock transition time in slow clock cycles */
#define D11SCC_SLOW2FAST_TRANSITION	2

/**
 * d11 core has a 'fastpwrup_dly' register that must be written to.
 * This function returns d11 slow to fast clock transition time in [us] units.
 * It does not write to the d11 core.
 */
uint16
BCMINITFN(si_pmu_fast_pwrup_delay)(si_t *sih, osl_t *osh)
{
	uint pmudelay_us = PMU_MAX_TRANSITION_DLY;
	pmuregs_t *pmu;
	uint origidx;
	uint32 ilp;			/* ILP clock frequency in [Hz] */
	rsc_per_chip_t *rsc;		/* chip specific resource bit positions */

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	if (ISSIM_ENAB(sih)) {
		pmudelay_us = 70;
	} else {
		switch (CHIPID(sih->chip)) {
		CASE_BCM43684_CHIP:
		CASE_EMBEDDED_2x2AX_CORE:
		CASE_EMBEDDED_11BE_CORE:
		CASE_BCM6710_CHIP:
		CASE_BCM6715_CHIP:
		CASE_BCM6717_CHIP:
		CASE_BCM6726_CHIP:
		CASE_BCM6711_CHIP:
			rsc = si_pmu_get_rsc_positions(sih);
			/* Retrieve time by reading it out of the hardware */
			ilp = si_ilp_clock(sih);
			if (ilp != 0) {
				pmudelay_us = si_pmu_res_uptime(sih, osh, pmu, rsc->ht_avail) +
					D11SCC_SLOW2FAST_TRANSITION;
				pmudelay_us = (11 * pmudelay_us) / 10;
			}
			break;
		default:
			break;
		}
	} /* if (ISSIM_ENAB(sih)) */

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return (uint16)pmudelay_us;
} /* si_pmu_fast_pwrup_delay */

/** To enable avb timer clock feature */
void si_pmu_avbtimer_enable(si_t *sih, osl_t *osh, bool set_flag)
{
	pmuregs_t *pmu;
	uint origidx;
	uint intr_val;

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);
	BCM_REFERENCE(pmu);

	if (CHIPID(sih->chip) == BCM43684_CHIP_ID) {
		/*
		 * For 43684, bits 13:12 of chipcontrol_data must be set to a value other than 0.
		 * 0 means AVB clock disabled, 01 means AVB clock uses GPIOs for input, and 10
		 * means AVB clock uses GPIOs for output
		 */
		if (set_flag) {
			si_pmu_chipcontrol(sih, PMU_CHIPCTL0, 0x3000, 0x2000);
		}
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	si_intrrestore(sih, intr_val);
}

/**
 * Determines min/max rsrc masks. Normally hardware contains these masks, and software reads the
 * masks from hardware. Note that masks are sometimes dependent on chip straps.
 */
static void
si_pmu_res_masks(si_t *sih, uint32 *pmin, uint32 *pmax)
{
	uint32 min_mask = 0, max_mask = 0;

	/* determine min/max rsrc masks */
	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_EMBEDDED_11BE_CORE:
	CASE_BCM6710_CHIP:
	CASE_BCM6715_CHIP:
	CASE_BCM6717_CHIP:
	CASE_BCM6726_CHIP:
	CASE_BCM6711_CHIP:
		min_mask = pmu_corereg(sih, SI_CC_IDX, min_res_mask, 0, 0);
		max_mask = 0x7FFFFFFF;
		break;
	default:
		PMU_ERROR(("MIN and MAX mask is not programmed\n"));
		break;
	}

	/* nvram override */
	si_nvram_res_masks(sih, &min_mask, &max_mask);

	*pmin = min_mask;
	*pmax = max_mask;
} /* si_pmu_res_masks */

/** Initialize PMU hardware resources. */
void
BCMATTACHFN(si_pmu_res_init)(si_t *sih, osl_t *osh)
{
#if !defined(_CFE_) && !defined(_CFEZ_)
	pmuregs_t *pmu;
	uint origidx;
	uint32 min_mask = 0, max_mask = 0;
	char name[8];
	const char *val;
	uint i, rsrcs;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	/*
	 * Hardware contains the resource updown and dependency tables. Only if a chip has a
	 * hardware problem, software tables can be used to override hardware tables.
	 */
	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_EMBEDDED_11BE_CORE:
	CASE_BCM6710_CHIP:
	CASE_BCM6715_CHIP:
	CASE_BCM6717_CHIP:
		/* TODO:6717:MLO, TODO:6765:MLO, TODO:6711:MLO
		 * under HWALITE needs chip-specific programming ?
		 */
		break;
	CASE_BCM6726_CHIP:
#ifndef DONGLEBUILD
		/* TODO: is_mlo_capable check */
		si_pmu_chipcontrol(sih, PMU_CHIPCTL3,
			PMU_6726_CC3_NIC_MODE, PMU_6726_CC3_NIC_MODE);
#endif /* !DONGLEBUILD */
		break;
	default:
		break;
	}

	/* # resources */
	rsrcs = (sih->pmucaps & PCAP_RC_MASK) >> PCAP_RC_SHIFT;

	/* Apply nvram overrides to up/down timers */
	for (i = 0; i < rsrcs; i ++) {
		uint32 r_val;
		snprintf(name, sizeof(name), rstr_rDt, i);
		if ((val = getvar(NULL, name)) == NULL)
			continue;
		r_val = (uint32)bcm_strtoul(val, NULL, 0);
		if (r_val < (1 << 16)) {
			uint16 up_time = (r_val >> 8) & 0xFF;
			r_val &= 0xFF;
			r_val |= (up_time << 16);
		}
		PMU_MSG(("Applying %s=%s to rsrc %d res_updn_timer\n", name, val, i));
		W_REG(osh, &pmu->res_table_sel, (uint32)i);
		W_REG(osh, &pmu->res_updn_timer, r_val);
	}

	/* Apply nvram overrides to dependencies masks */
	for (i = 0; i < rsrcs; i ++) {
		snprintf(name, sizeof(name), rstr_rDd, i);
		if ((val = getvar(NULL, name)) == NULL)
			continue;
		PMU_MSG(("Applying %s=%s to rsrc %d res_dep_mask\n", name, val, i));
		W_REG(osh, &pmu->res_table_sel, (uint32)i);
		W_REG(osh, &pmu->res_dep_mask, (uint32)bcm_strtoul(val, NULL, 0));
	}

	/* Determine min/max rsrc masks */
	si_pmu_res_masks(sih, &min_mask, &max_mask);

	/* Add min mask dependencies */
	min_mask |= si_pmu_res_deps(sih, osh, pmu, min_mask, FALSE);

#ifdef BCM_BOOTLOADER
	/* Apply nvram override to max mask */
	if ((val = getvar(NULL, "brmax")) != NULL) {
		PMU_MSG(("Applying brmax=%s to max_res_mask\n", val));
		max_mask = (uint32)bcm_strtoul(val, NULL, 0);
	}

	/* Apply nvram override to min mask */
	if ((val = getvar(NULL, "brmin")) != NULL) {
		PMU_MSG(("Applying brmin=%s to min_res_mask\n", val));
		min_mask = (uint32)bcm_strtoul(val, NULL, 0);
	}
#endif /* BCM_BOOTLOADER */

	if (max_mask) {
		/* Ensure there is no bit set in min_mask which is not set in max_mask */
		max_mask |= min_mask;

		/* First set the bits which change from 0 to 1 in max, then update the
		 * min_mask register and then reset the bits which change from 1 to 0
		 * in max. This is required as the bit in MAX should never go to 0 when
		 * the corresponding bit in min is still 1. Similarly the bit in min cannot
		 * be 1 when the corresponding bit in max is still 0.
		 */
		OR_REG(osh, &pmu->max_res_mask, max_mask);
	} else {
		/* First set the bits which change from 0 to 1 in max, then update the
		 * min_mask register and then reset the bits which change from 1 to 0
		 * in max. This is required as the bit in MAX should never go to 0 when
		 * the corresponding bit in min is still 1. Similarly the bit in min cannot
		 * be 1 when the corresponding bit in max is still 0.
		 */
		if (min_mask)
			OR_REG(osh, &pmu->max_res_mask, min_mask);
	}

	/* Program min resource mask */
	if (min_mask) {
		PMU_MSG(("Changing min_res_mask to 0x%x\n", min_mask));
		W_REG(osh, &pmu->min_res_mask, min_mask);
	}

	/* Program max resource mask */
	if (max_mask) {
		PMU_MSG(("Changing max_res_mask to 0x%x\n", max_mask));
		W_REG(osh, &pmu->max_res_mask, max_mask);
	}

	si_pmu_wait_for_steady_state(sih, osh, pmu);
	/* Add some delay; allow resources to come up and settle. */
	OSL_DELAY(2000);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
#endif /* !_CFE_ && !_CFEZ_ */
} /* si_pmu_res_init */

/* setup pll and query clock speed */
typedef struct {
	uint32	fref;	/* x-tal frequency in [hz] */
	uint8	xf;	/* x-tal index as contained in PMU control reg, see PMU programmers guide */
	uint8	p1div;
	uint8	p2div;
	uint8	ndiv_int;
	uint32	ndiv_frac;
} pmu1_xtaltab0_t;

/** how to achieve 2907Mhz BBPLL output using different x-tal frequencies ('fref') */
static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_2907)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac */
	{54000,   0,       1,      1,     0x35,     0xD55AC }, /**< 54 Mhz xtal */
	{0,	  0,       0,      0,     0,        0	    }
};

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_bcm63178_2880)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac */
	{50000,   0,       1,      1,     0x3a,     0x23DD4 }, /**< 50 Mhz xtal */
	{0,	  0,       0,      0,     0,        0	    }
};

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_bcm6715_5814)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac (use ndiv_p, ndiv_q for 6715) */
	{50000,   0,       1,      1,     0x74,     0x0     }, /**< 50 Mhz xtal */
	{0,	  0,       0,      0,     0,        0	    }
};

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_bcm6717_5840)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac (use ndiv_p, ndiv_q for 6717) */
	{80000,   0,       1,      1,     0x49,     0x0     }, /**< 80 Mhz xtal */
	{0,	  0,       0,      0,     0,        0	    }
};

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_bcm6726_5840)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac (use ndiv_p, ndiv_q for 6726) */
	{80000,   0,       2,      0,     0x92,     0x0     }, /**< 80 Mhz xtal */
	{0,	  0,       0,      0,     0,        0	    }
};

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_bcm6765_5440_5840)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac */
	{80000,   0,       2,      0,     0x88,     0x0     }, /**< 80 Mhz xtal, Fvco 5440 */
	{80000,   0,       2,      0,     0x92,     0x0     }, /**< 80 Mhz xtal, Fvco 5840 */
	{0,	  0,       0,      0,     0,        0	    }
};

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_bcm6711_5840)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac */
	{80000,   0,       2,      0,     0x92,     0x0     }, /**< 80 Mhz xtal */
	{0,	  0,       0,      0,     0,        0	    }
};

/* For having the pllcontrol data (info)
 * The table with the values of the registers will have one - one mapping.
 */
typedef struct {
	uint16	clock;	/**< x-tal frequency in [KHz] */
	uint8	mode;	/**< spur mode */
	uint8	xf;	/**< corresponds with xf bitfield in PMU control register */
} pllctrl_data_t;

/** returns a table that instructs how to program the BBPLL for a particular xtal frequency */
static const pmu1_xtaltab0_t *
BCMINITFN(si_pmu1_xtaltab0)(si_t *sih)
{
#ifdef BCMDBG
	char chn[8];
#endif
	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
		return pmu1_xtaltab0_2907;
	CASE_EMBEDDED_2x2AX_CORE:
		return pmu1_xtaltab0_bcm63178_2880; /* default BBPLL output frequency is 2880 Mhz */
	CASE_EMBEDDED_11BE_CORE:
		return pmu1_xtaltab0_bcm6765_5440_5840;
	CASE_BCM6715_CHIP:
		return pmu1_xtaltab0_bcm6715_5814;
	CASE_BCM6717_CHIP:
		return pmu1_xtaltab0_bcm6717_5840;
	CASE_BCM6726_CHIP:
		return pmu1_xtaltab0_bcm6726_5840;
	CASE_BCM6711_CHIP:
		return pmu1_xtaltab0_bcm6711_5840;
	default:
		PMU_MSG(("si_pmu1_xtaltab0: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		break;
	}
	ASSERT(0);
	return NULL;
} /* si_pmu1_xtaltab0 */

/** returns chip specific PLL settings for default xtal frequency and VCO output frequency */
static const pmu1_xtaltab0_t *
BCMINITFN(si_pmu1_xtaldef0)(si_t *sih)
{
#ifdef BCMDBG
	char chn[8];
#endif

	switch (CHIPID(sih->chip)) {
	CASE_EMBEDDED_2x2AX_CORE:
		return &pmu1_xtaltab0_bcm63178_2880[0];
	CASE_EMBEDDED_11BE_CORE:
		return &pmu1_xtaltab0_bcm6765_5440_5840[0];
	CASE_BCM6715_CHIP:
		return &pmu1_xtaltab0_bcm6715_5814[0];
	CASE_BCM6717_CHIP:
		return &pmu1_xtaltab0_bcm6717_5840[0];
	CASE_BCM6726_CHIP:
		return &pmu1_xtaltab0_bcm6726_5840[0];
	CASE_BCM6711_CHIP:
		return &pmu1_xtaltab0_bcm6711_5840[0];
	CASE_BCM43684_CHIP:
		return &pmu1_xtaltab0_2907[0];
	default:
		PMU_MSG(("si_pmu1_xtaldef0: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		break;
	}
	ASSERT(0);
	return NULL;
} /* si_pmu1_xtaldef0 */

/** returns chip specific default BaseBand pll fvco frequency in [KHz] units */
static uint32
BCMINITFN(si_pmu_bbpll_fvco0)(si_t *sih)
{
#ifdef BCMDBG
	char chn[8];
#endif
	uint32 xf, ndiv_int, fvco, pll_reg;
	uint32 p, q;
	uint8 p1_div;
	chipcregs_t *cc;
	uint origidx, intr_val;

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
		return FVCO_2907;
	CASE_EMBEDDED_2x2AX_CORE:
		return FVCO_2880;
	CASE_EMBEDDED_11BE_CORE:
		return FVCO_5440;
	CASE_BCM6715_CHIP:
	CASE_BCM6717_CHIP:
	CASE_BCM6726_CHIP:
		break;
	default:
		PMU_MSG(("%s: Unknown chipid %s\n", __FUNCTION__, bcm_chipname(sih->chip, chn, 8)));
		ASSERT(0);
		return 0;
	}

	/* Remember original core before switch to chipc */
	cc = (chipcregs_t *)si_switch_core(sih, CC_CORE_ID, &origidx, &intr_val);
	ASSERT(cc != NULL);
	BCM_REFERENCE(cc);

	xf = si_pmu_get_clock(sih, si_osh(sih), SI_PMU_CLK_XTAL) / 1000;

	switch (CHIPID(sih->chip)) {
	CASE_BCM6715_CHIP:
		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, 0, 0);
		p1_div = ((pll_reg & PMU6715_PLL0_PC0_PDIV_MASK) >>
				PMU6715_PLL0_PC0_PDIV_SHIFT);
		ndiv_int = ((pll_reg & PMU6715_PLL0_PC0_NDIV_INT_MASK) >>
				PMU6715_PLL0_PC0_NDIV_INT_SHIFT);

		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, 0, 0);
		p = ((pll_reg & PMU6715_PLL0_PC1_NDIV_P_MASK) >>
				PMU6715_PLL0_PC1_NDIV_P_SHIFT);
		q = ((pll_reg & PMU6715_PLL0_PC1_NDIV_Q_MASK) >>
				PMU6715_PLL0_PC1_NDIV_Q_SHIFT);

		/* check i_ndiv_frac_mode_sel */
		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, 0, 0);
		if (pll_reg & PMU6715_PLL0_PC2_NDIV_FRAC_MODE_SEL) {
			/* ndiv_frac_mode_sel 1, fractional value = p/q */
			fvco = (xf * ndiv_int);
			fvco += ((xf * p) / q);
		} else {
			/* ndiv_frac_mode_sel 0, fractional value= (p*2^10+q)/2^20 */
			fvco =  xf * ndiv_int;
			fvco += (xf * ((p << 10) + q)) >> 20;
		}

		if (!p1_div) {
			ASSERT(0);
			fvco = 0;
			break;
		}

		fvco /= p1_div;
		fvco /= 1000;
		fvco *= 1000;
		PMU_MSG(("%s: fref %u p %u q %u ndiv_int %u pdiv %u fvco %u\n",
				__FUNCTION__, xf, p, q, ndiv_int, p1_div, fvco));
		break;
	CASE_BCM6717_CHIP:
		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, 0, 0);
		p1_div = ((pll_reg & PMU6717_PLL0_PC0_PDIV_MASK) >>
				PMU6717_PLL0_PC0_PDIV_SHIFT);
		ndiv_int = ((pll_reg & PMU6717_PLL0_PC0_NDIV_INT_MASK) >>
				PMU6717_PLL0_PC0_NDIV_INT_SHIFT);

		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, 0, 0);
		p = ((pll_reg & PMU6717_PLL0_PC1_NDIV_P_MASK) >>
				PMU6717_PLL0_PC1_NDIV_P_SHIFT);
		q = ((pll_reg & PMU6717_PLL0_PC1_NDIV_Q_MASK) >>
				PMU6717_PLL0_PC1_NDIV_Q_SHIFT);

		/* check i_ndiv_frac_mode_sel */
		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, 0, 0);
		if (pll_reg & PMU6717_PLL0_PC1_NDIV_FRAC_MODE_SEL) {
			/* ndiv_frac_mode_sel 1, fractional value = p/q */
			fvco = (xf * ndiv_int);
			fvco += ((xf * p) / q);
		} else {
			/* ndiv_frac_mode_sel 0, fractional value= (p*2^10+q)/2^20 */
			fvco =  xf * ndiv_int;
			fvco += (xf * ((p << 10) + q)) >> 20;
		}

		if (!p1_div) {
			ASSERT(0);
			fvco = 0;
			break;
		}

		fvco /= p1_div;
		fvco /= 1000;
		fvco *= 1000;
		PMU_MSG(("%s: fref %u p %u q %u ndiv_int %u pdiv %u fvco %u\n",
				__FUNCTION__, xf, p, q, ndiv_int, p1_div, fvco));
		break;
	CASE_BCM6726_CHIP:
		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, 0, 0);
		p1_div = ((pll_reg & PMU6726_PLL0_PC0_PDIV_MASK) >>
				PMU6726_PLL0_PC0_PDIV_SHIFT);
		ndiv_int = ((pll_reg & PMU6726_PLL0_PC0_NDIV_INT_MASK) >>
				PMU6726_PLL0_PC0_NDIV_INT_SHIFT);

		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, 0, 0);
		p = ((pll_reg & PMU6726_PLL0_PC1_NDIV_P_MASK) >>
				PMU6726_PLL0_PC1_NDIV_P_SHIFT);
		q = ((pll_reg & PMU6726_PLL0_PC1_NDIV_Q_MASK) >>
				PMU6726_PLL0_PC1_NDIV_Q_SHIFT);

		/* check i_ndiv_frac_mode_sel */
		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, 0, 0);
		if (pll_reg & PMU6726_PLL0_PC2_NDIV_FRAC_MODE_SEL) {
			/* ndiv_frac_mode_sel 1, fractional value = p/q */
			fvco = (xf * ndiv_int);
			fvco += ((xf * p) / q);
		} else {
			/* ndiv_frac_mode_sel 0, fractional value= (p*2^10+q)/2^20 */
			fvco =  xf * ndiv_int;
			fvco += (xf * ((p << 10) + q)) >> 20;
		}

		if (!p1_div) {
			ASSERT(0);
			fvco = 0;
			break;
		}

		fvco /= p1_div;
		fvco /= 1000;
		fvco *= 1000;
		PMU_MSG(("%s: fref %u p %u q %u ndiv_int %u pdiv %u fvco %u\n",
				__FUNCTION__, xf, p, q, ndiv_int, p1_div, fvco));
		break;
	default:
		PMU_MSG(("si_pmu_cal_fvco: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		ASSERT(0);
		return 0;
	}

	/* Return to original core */
	si_restore_core(sih, origidx, intr_val);

	return fvco;
} /* si_pmu_bbpll_fvco0 */

/**
 * @return XTAL clock frequency in [Hz]
 */
static uint32
BCMINITFN(si_pmu_xtal_freq)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	const pmu1_xtaltab0_t *xt;
	uint32 xf, xtal_hz;
	BCM_REFERENCE(sih);

	if (BCM6710_CHIP(sih->chip)) {
		return 50 * 1000 * 1000; /* always 50Mhz */
	}

	/* Find the frequency in the table */
	xf = (R_REG(osh, &pmu->pmucontrol) & PCTL_XTALFREQ_MASK) >>
	        PCTL_XTALFREQ_SHIFT;

	for (xt = si_pmu1_xtaltab0(sih); xt != NULL && xt->fref != 0; xt ++)
		if (xt->xf == xf)
			break;

	/* Could not find it so assign a default value */
	if (xt == NULL || xt->fref == 0) {
		xt = si_pmu1_xtaldef0(sih);
	}

	ASSERT(xt != NULL && xt->fref != 0);
	xtal_hz = xt->fref * 1000;

	return xtal_hz;
}

uint32
si_pmu_get_pmutimer(si_t *sih)
{
	osl_t *osh = si_osh(sih);
	pmuregs_t *pmu;
	uint origidx;
	uint32 ticks;
	uint32 ticks1;
	uint32 ticks2;
	uint32 ticks3;
	int loop_count;

	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	/* There can be two consecutive reads which are invalid, but not three. */
	ticks1 = R_REG(osh, &pmu->pmutimer);
	ticks2 = R_REG(osh, &pmu->pmutimer);
	loop_count = 5;
	while (loop_count) {
		ticks3 = R_REG(osh, &pmu->pmutimer);
		if ((ticks1 == ticks3) || ((ticks1 + 1) == ticks3)) {
			ticks = ticks3;
			break;
		}
		if ((ticks3 > ticks2) && (ticks2 > ticks1)) {
			ticks = ticks2;
			break;
		}
		ticks = ticks3;
		loop_count--;
		ticks1 = ticks2;
		ticks2 = ticks3;
	}
	ASSERT(loop_count);

	si_setcoreidx(sih, origidx);

	return (ticks);
}

/**
 * returns
 * a) diff between a 'prev' value of pmu timer and current value
 * b) the current pmutime value in 'prev'
 * 	So, 'prev' is an IO parameter.
 */
uint32
si_pmu_get_pmutime_diff(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 *prev)
{
	uint32 pmutime_diff = 0, pmutime_val = 0;
	uint32 prev_val = *prev;
	BCM_REFERENCE(osh);
	BCM_REFERENCE(pmu);
	/* read current value */
	pmutime_val = si_pmu_get_pmutimer(sih);
	/* diff btween prev and current value, take on wraparound case as well. */
	pmutime_diff = (pmutime_val >= prev_val) ?
		(pmutime_val - prev_val) :
		(~prev_val + pmutime_val + 1);
	*prev = pmutime_val;
	return pmutime_diff;
}

/**
 * wait for usec for the res_pending register to change.
 * NOTE: usec SHOULD be > 32uS
 * if cond = TRUE, res_pending will be read until it becomes == 0;
 * If cond = FALSE, res_pending will be read until it becomes != 0;
 * returns TRUE if timedout.
 * returns elapsed time in this loop in elapsed_time
 */
bool
si_pmu_wait_for_res_pending(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint usec,
	bool cond, uint32 *elapsed_time)
{
	/* add 32uSec more */
	uint countdown = usec;
	uint32 pmutime_prev = 0, pmutime_elapsed = 0, res_pend;
	bool pending = FALSE;

	/* store current time */
	pmutime_prev = si_pmu_get_pmutimer(sih);
	while (1) {
		res_pend = R_REG(osh, &pmu->res_pending);

		/* based on the condition, check */
		if (cond == TRUE) {
			if (res_pend == 0) break;
		} else {
			if (res_pend != 0) break;
		}

		/* if required time over */
		if ((pmutime_elapsed * PMU_US_STEPS) >= countdown) {
			/* timeout. so return as still pending */
			pending = TRUE;
			break;
		}

		/* get elapsed time after adding diff between prev and current
		* pmutimer value
		*/
		pmutime_elapsed += si_pmu_get_pmutime_diff(sih, osh, pmu, &pmutime_prev);
	}

	*elapsed_time = pmutime_elapsed * PMU_US_STEPS;
	return pending;
} /* si_pmu_wait_for_res_pending */

/**
 *	The algorithm for pending check is that,
 *	step1: 	wait till (res_pending !=0) OR pmu_max_trans_timeout.
 *			if max_trans_timeout, flag error and exit.
 *			wait for 1 ILP clk [64uS] based on pmu timer,
 *			polling to see if res_pending again goes high.
 *			if res_pending again goes high, go back to step1.
 *	Note: res_pending is checked repeatedly because, in between switching
 *	of dependent
 *	resources, res_pending resets to 0 for a short duration of time before
 *	it becomes 1 again.
 *	Note: return 0 is GOOD, 1 is BAD [mainly timeout].
 */
int si_pmu_wait_for_steady_state(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	int stat = 0;
	bool timedout = FALSE;
	uint32 elapsed = 0, pmutime_total_elapsed = 0;

	while (1) {
		/* wait until all resources are settled down [till res_pending becomes 0] */
		timedout = si_pmu_wait_for_res_pending(sih, osh, pmu,
			PMU_MAX_TRANSITION_DLY, TRUE, &elapsed);

		if (timedout) {
			stat = 1;
			break;
		}

		pmutime_total_elapsed += elapsed;
		/* wait to check if any resource comes back to non-zero indicating
		* that it pends again. The res_pending goes 0 for 1 ILP clock before
		* getting set for next resource in the sequence , so if res_pending
		* is 0 for more than 1 ILP clk it means nothing is pending
		* to indicate some pending dependency.
		*/
		timedout = si_pmu_wait_for_res_pending(sih, osh, pmu,
			64, FALSE, &elapsed);

		pmutime_total_elapsed += elapsed;
		/* Here, we can also check timedout, but we make sure that,
		* we read the res_pending again.
		*/
		if (timedout) {
			stat = 0;
			break;
		}

		/* Total wait time for all the waits above added should be
		* less than  PMU_MAX_TRANSITION_DLY
		*/
		if (pmutime_total_elapsed >= PMU_MAX_TRANSITION_DLY) {
			/* timeout. so return as still pending */
			stat = 1;
			break;
		}
	}
	return stat;
} /* si_pmu_wait_for_steady_state */

/**
 * returns the CPU clock frequency. Does this by determining current Fvco and the setting of the
 * clock divider that leads up to the ARM. Returns value in [Hz] units.
 */
static uint32
BCMINITFN(si_pmu1_cpuclk0)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 tmp, tmp1, mdiv = 1;
	uint32 ndiv_int, ndiv_frac, p2div, p1div, fvco, p = 0, q = 0;
	uint32 fref_khz;
#ifdef BCMDBG
	char chn[8];
#endif

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP: /* Uses CPU PLL instead of BB PLL to derive CPU clock */
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG12, 0, 0);
		mdiv = (tmp & PMU1_PLL0_PC1_M1DIV_MASK) >> PMU1_PLL0_PC1_M1DIV_SHIFT;
		break; /* default backplane frequency is 240MHz */

	CASE_BCM6715_CHIP: /* Uses CPU PLL instead of BB PLL to derive CPU clock */
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG9, 0, 0);
		mdiv = (tmp & PMU6715_PLL1_PC9_CH0_MDIV_MASK) >> PMU6715_PLL1_PC9_CH0_MDIV_SHIFT;
		break;

	CASE_BCM6717_CHIP: /* Uses CPU PLL instead of BB PLL to derive CPU clock */
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, 0, 0); /* use i_pdiv */
		mdiv = (tmp & PMU6717_PLL1_PC7_PDIV_MASK) >> PMU6717_PLL1_PC7_PDIV_SHIFT;
		break;

	CASE_BCM6726_CHIP: /* Uses CPU PLL instead of BB PLL to derive CPU clock */
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG9, 0, 0); /* use i_ch0_mdiv */
		mdiv = (tmp & PMU6726_PLL1_PC9_CH0_MDIV_MASK) >> PMU6726_PLL1_PC9_CH0_MDIV_SHIFT;
		break;

	default:
		PMU_MSG(("si_pmu1_cpuclk0: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		ASSERT(0);
		break;
	}

	ASSERT(mdiv != 0);
	if (!mdiv)
		return 0;

	/* Read p2div/p1div from pllcontrol[0] */
	tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, 0, 0);
	p2div = (tmp & PMU1_PLL0_PC0_P2DIV_MASK) >> PMU1_PLL0_PC0_P2DIV_SHIFT;
	p1div = (tmp & PMU1_PLL0_PC0_P1DIV_MASK) >> PMU1_PLL0_PC0_P1DIV_SHIFT;

	/* Calculate fvco based on xtal freq and ndiv and pdiv */
	if (BCM43684_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG10;
	} else if (BCM6715_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG6;
	} else if (BCM6717_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG7;
	} else if (BCM6726_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG6;
	} else {
		tmp = PMU1_PLL0_PLLCTL2;
	}

	tmp = si_pmu_pllcontrol(sih, tmp, 0, 0);

	if (BCM43684_CHIP(sih->chip) || BCM6715_CHIP(sih->chip)) {
		ndiv_int = (tmp >> 4) & 0x3ff;
		p2div = 1;
		p1div = (tmp & 7);
	} else if (BCM6717_CHIP(sih->chip)) {
		p2div = 1;
		p1div = (tmp & PMU6717_PLL1_PC7_PDIV_MASK);
		ndiv_int = (tmp & PMU6717_PLL1_PC7_NDIV_INT_MASK) >>
			PMU6717_PLL1_PC7_NDIV_INT_SHIFT;
	} else if (BCM6726_CHIP(sih->chip)) {
		p2div = 1;
		p1div = (tmp & PMU6726_PLL1_PC6_PDIV_MASK);
		ndiv_int = (tmp & PMU6726_PLL1_PC6_NDIV_INT_MASK) >>
			PMU6726_PLL1_PC6_NDIV_INT_SHIFT;
	} else {
		ndiv_int = (tmp & PMU1_PLL0_PC2_NDIV_INT_MASK) >> PMU1_PLL0_PC2_NDIV_INT_SHIFT;
	}

	ASSERT(p1div != 0);
	if (!p1div)
		return 0;

	if (BCM43684_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG11;
	} else if (BCM6715_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG7;
	} else if (BCM6717_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG8;
	} else if (BCM6726_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG7;
	} else {
		tmp = PMU1_PLL0_PLLCTL3;
	}

	tmp = si_pmu_pllcontrol(sih, tmp, 0, 0);
	fref_khz = si_pmu_xtal_freq(sih, osh, pmu) / 1000;

	if (BCM43684_CHIP(sih->chip)) {
		/* PLL has no second pdiv bitfield */
		ndiv_frac = (tmp & PMU4347_PLL0_PC3_NDIV_FRAC_MASK) >>
			PMU4347_PLL0_PC3_NDIV_FRAC_SHIFT;

		fvco = (fref_khz * ndiv_int) << 8;
		fvco += (fref_khz * ((ndiv_frac & 0xfffff) >> 4)) >> 8;
		fvco >>= 8;
		fvco *= p1div;
		fvco /= 1000;
		fvco *= 1000;
	} else if (BCM6715_CHIP(sih->chip)) {
		p = ((tmp & PMU6715_PLL1_PC7_NDIV_P_MASK) >> PMU6715_PLL1_PC7_NDIV_P_SHIFT);
		q = ((tmp & PMU6715_PLL1_PC7_NDIV_Q_MASK) >> PMU6715_PLL1_PC7_NDIV_Q_SHIFT);

		/* check i_ndiv_frac_mode_sel */
		tmp1 = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
		if (tmp1 & PMU6715_PLL1_PC8_NDIV_FRAC_MODE_SEL) {
			/* ndiv_frac_mode_sel 1, fractional value = p/q */
			ndiv_frac = p / q;
			fvco = (fref_khz * ndiv_int);
			fvco += ((fref_khz * p) / q);
		} else {
			/* ndiv_frac_mode_sel 0, fractional value= (p*2^10+q)/2^20 */
			ndiv_frac = ((p << 10) + q) >> 20;
			fvco =  fref_khz * ndiv_int;
			fvco += (fref_khz * ((p << 10) + q)) >> 20;
		}
		fvco /= p1div;
		fvco /= 1000;
		fvco *= 1000;
	} else if (BCM6717_CHIP(sih->chip)) {
		p = ((tmp & PMU6717_PLL1_PC8_NDIV_P_MASK) >> PMU6717_PLL1_PC8_NDIV_P_SHIFT);
		q = ((tmp & PMU6717_PLL1_PC8_NDIV_Q_MASK) >> PMU6717_PLL1_PC8_NDIV_Q_SHIFT);

		/* check i_ndiv_frac_mode_sel */
		if (tmp & PMU6717_PLL1_PC8_NDIV_FRAC_MODE_SEL) {
			/* ndiv_frac_mode_sel 1, fractional value = p/q */
			ndiv_frac = p / q;
			fvco = (fref_khz * ndiv_int);
			fvco += ((fref_khz * p) / q);
		} else {
			/* ndiv_frac_mode_sel 0, fractional value= (p*2^10+q)/2^20 */
			ndiv_frac = ((p << 10) + q) >> 20;
			fvco =  fref_khz * ndiv_int;
			fvco += (fref_khz * ((p << 10) + q)) >> 20;
		}
		fvco /= p1div;
		fvco /= 1000;
		fvco *= 1000;
	} else if (BCM6726_CHIP(sih->chip)) {
		p = ((tmp & PMU6726_PLL1_PC7_NDIV_P_MASK) >> PMU6726_PLL1_PC7_NDIV_P_SHIFT);
		q = ((tmp & PMU6726_PLL1_PC7_NDIV_Q_MASK) >> PMU6726_PLL1_PC7_NDIV_Q_SHIFT);

		/* check i_ndiv_frac_mode_sel */
		tmp1 = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
		if (tmp1 & PMU6726_PLL1_PC8_NDIV_FRAC_MODE_SEL) {
			/* ndiv_frac_mode_sel 1, fractional value = p/q */
			ndiv_frac = p / q;
			fvco = (fref_khz * ndiv_int);
			fvco += ((fref_khz * p) / q);
		} else {
			/* ndiv_frac_mode_sel 0, fractional value= (p*2^10+q)/2^20 */
			ndiv_frac = ((p << 10) + q) >> 20;
			fvco =  fref_khz * ndiv_int;
			fvco += (fref_khz * ((p << 10) + q)) >> 20;
		}
		fvco /= p1div;
		fvco /= 1000;
		fvco *= 1000;
	} else {
		ndiv_frac =
			(tmp & PMU1_PLL0_PC3_NDIV_FRAC_MASK) >> PMU1_PLL0_PC3_NDIV_FRAC_SHIFT;
		fvco = (fref_khz * ndiv_int) << 8;
		fvco += (fref_khz * (ndiv_frac >> 12)) >> 4;
		fvco += (fref_khz * (ndiv_frac & 0xfff)) >> 12;
		fvco >>= 8;
		fvco *= p2div;
		fvco /= p1div;
		fvco /= 1000;
		fvco *= 1000;
	}

	PMU_MSG(("si_pmu1_cpuclk0: ndiv_int %u ndiv_frac %u p2div %u p1div %u fvco %u\n",
	         ndiv_int, ndiv_frac, p2div, p1div, fvco));

	if (BCM6715_CHIP(sih->chip) || BCM6717_CHIP(sih->chip) || BCM6726_CHIP(sih->chip)) {
		PMU_MSG(("%s: p %u q %u mdiv %u alpclk0 %u\n", __FUNCTION__,
			p, q, mdiv, fref_khz));
	}

	return fvco / mdiv * 1000; /* Return CPU clock in [Hz] */
} /* si_pmu1_cpuclk0 */

#if defined(DONGLEBUILD)
#define NDIV_INT_2000	0x251
#define NDIV_INT_2700	0x321
#define NDIV_INT_3000	0x371
#define NDIV_INT_3200	0x3b1
#define NDIV_INT_3300	0x3d1
#define NDIV_INT_3600	0x421
#define I_NDIV_INT_4400 0x6E
#define I_NDIV_INT_4800	0x60
#define I_NDIV_INT_5200	0x82
#define I_NDIV_INT_6000	0x78
#define NDIV_FRAC_2000		0x9858
#define NDIV_FRAC_2700		0x8e396
#define NDIV_FRAC_3000		0x8e396
#define NDIV_FRAC_3200		0x42680
#define NDIV_FRAC_3300		0x1c746
#define NDIV_FRAC_3600		0xab086
#define NDIV_P_4400		0x0
#define NDIV_Q_4400		0x0
#define NDIV_P_4800		0x0
#define NDIV_Q_4800		0x0
#define NDIV_P_5200		0x0
#define NDIV_Q_5200		0x0
#define NDIV_P_6000		0x0
#define NDIV_Q_6000		0x0

/**
 * Before the PLL is switched off, the HT clocks need to be deactivated, and reactivated
 * when the PLL is switched on again.
 * This function returns the chip specific HT clock resources (HT and MACPHY clocks).
 */
static uint32
si_pmu_htclk_mask(si_t *sih)
{
	/* chip specific bit position of various resources */
	rsc_per_chip_t *rsc = si_pmu_get_rsc_positions(sih);

	uint32 ht_req = (PMURES_BIT(rsc->ht_avail) | PMURES_BIT(rsc->macphy_clkavail));

	switch (CHIPID(sih->chip))
	{
		CASE_BCM43684_CHIP:
		CASE_BCM6715_CHIP:
		CASE_BCM6717_CHIP:
		CASE_BCM6726_CHIP:
		CASE_EMBEDDED_2x2AX_CORE:
		CASE_EMBEDDED_11BE_CORE:
			ht_req |= PMURES_BIT(rsc->ht_start);
			break;
		default:
			ASSERT(0);
			break;
	}

	return ht_req;
} /* si_pmu_htclk_mask */

/** Turn Off the PLL - Required before setting the PLL registers */
static void
si_pmu_pll_off(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 *min_mask,
	uint32 *max_mask, uint32 *clk_ctl_st)
{
	uint32 ht_req;

	/* Save the original register values */
	*min_mask = R_REG(osh, &pmu->min_res_mask);
	*max_mask = R_REG(osh, &pmu->max_res_mask);
	*clk_ctl_st = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0);

	ht_req = si_pmu_htclk_mask(sih);
	if (ht_req == 0)
		return;

	if (EMBEDDED_2x2AX_CORE(sih->chip) ||
		EMBEDDED_11BE_CORE(sih->chip) ||
		BCM6715_CHIP(sih->chip) ||
		BCM6717_CHIP(sih->chip) ||
		BCM6726_CHIP(sih->chip) ||
		BCM43684_CHIP(sih->chip)) {
		/* If HT_AVAIL is not set, wait to see if any resources are availing HT. */
		if (((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL) != CCS_HTAVAIL))
			si_pmu_wait_for_steady_state(sih, osh, pmu);
	} else {
		OR_REG(osh,  &pmu->max_res_mask, ht_req);
		/* wait for HT to be ready before taking the HT away...HT could be coming up... */
		SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL) != CCS_HTAVAIL), PMU_MAX_TRANSITION_DLY);
		ASSERT((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL));
	}

	AND_REG(osh, &pmu->min_res_mask, ~ht_req);
	AND_REG(osh, &pmu->max_res_mask, ~ht_req);

#if defined(DONGLEBUILD) && (defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_8A__))
	asm volatile("isb");
#endif /* defined(DONGLEBUILD) && (defined(__ARM_ARCH_7A__)  || defined(__ARM_ARCH_8A__)) */

	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL) == CCS_HTAVAIL), PMU_MAX_TRANSITION_DLY);
	ASSERT(!(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL));
	OSL_DELAY(100);
} /* si_pmu_pll_off */

/** Turn ON/restore the PLL based on the mask received */
static void
si_pmu_pll_on(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 min_mask_mask,
	uint32 max_mask_mask, uint32 clk_ctl_st_mask)
{
	uint32 ht_req;

	ht_req = si_pmu_htclk_mask(sih);
	if (ht_req == 0)
		return;

	max_mask_mask &= ht_req;
	min_mask_mask &= ht_req;

	if (max_mask_mask != 0)
		OR_REG(osh, &pmu->max_res_mask, max_mask_mask);

	if (min_mask_mask != 0)
		OR_REG(osh, &pmu->min_res_mask, min_mask_mask);

#if defined(DONGLEBUILD) && (defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_8A__))
	asm volatile("isb");
#endif /* defined(DONGLEBUILD) && (defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_8A__)) */

	if (clk_ctl_st_mask & CCS_HTAVAIL) {
		/* Wait for HT_AVAIL to come back */
		SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL) != CCS_HTAVAIL), PMU_MAX_TRANSITION_DLY);
		ASSERT((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL));
	}
}

static void
BCMATTACHFN(si_set_cpu_vcofreq_43684)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 freq)
{
	uint32 oldpmucontrol, val;

	uint32 ndiv_int = NDIV_INT_3000, ndiv_frac = NDIV_FRAC_3000;
	int change = 0;

	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG10);
	R_REG(osh, &pmu->pllcontrol_addr);
	if (R_REG(osh, &pmu->pllcontrol_data) != NDIV_INT_3000)
		change++;

	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG11);
	R_REG(osh, &pmu->pllcontrol_addr);
	if (R_REG(osh, &pmu->pllcontrol_data) != NDIV_FRAC_3000)
		change++;
	if ((!change) && (freq == FVCO_3000)) {
		return;
	}

	/* Stop CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= PMU_43684_CC1_CPUPLL_HOLD_CH;
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	switch (freq) {
		case FVCO_2000:
			ndiv_int = NDIV_INT_2000;
			ndiv_frac = NDIV_FRAC_2000;
			break;
		case FVCO_2700:
			ndiv_int = NDIV_INT_2700;
			ndiv_frac = NDIV_FRAC_2700;
			break;
		case FVCO_3000:
			ndiv_int = NDIV_INT_3000;
			ndiv_frac = NDIV_FRAC_3000;
			break;
		case FVCO_3200:
			ndiv_int = NDIV_INT_3200;
			ndiv_frac = NDIV_FRAC_3200;
			break;
		case FVCO_3300:
			ndiv_int = NDIV_INT_3300;
			ndiv_frac = NDIV_FRAC_3300;
			break;
		case FVCO_3600:
			ndiv_int = NDIV_INT_3600;
			ndiv_frac = NDIV_FRAC_3600;
			break;
	}
	/* Set ndiv_int to 45, pdiv to 2 */
	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG10);
	R_REG(osh, &pmu->pllcontrol_addr);
	W_REG(osh, &pmu->pllcontrol_data, ndiv_int);
	R_REG(osh, &pmu->pllcontrol_data);

	/* Set ndiv_frac */
	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG11);
	R_REG(osh, &pmu->pllcontrol_addr);
	W_REG(osh, &pmu->pllcontrol_data, ndiv_frac);
	R_REG(osh, &pmu->pllcontrol_data);

	/* Set pllCntlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, (oldpmucontrol | PCTL_PLL_PLLCTL_UPD));
	R_REG(osh, &pmu->pmucontrol);

	/* Change CPU PLL controlled by force_cpupll_pwrdn_ldo,
	 * force_cpupll_areset and  force_cpupll_dreset
	 */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= (PMU_43684_CC1_PMU_CPUPLL_CONTROL_OVR | PMU_43684_CC1_FORCE_CPUPLL_ARESET |
		PMU_43684_CC1_FORCE_CPUPLL_DRESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* De-Assert cpupll_i_areset */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_43684_CC1_FORCE_CPUPLL_ARESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Polling the CPU PLL LOCKED */
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipstatus), 0, 0) &
		CST43684_CPUPLL_LOCKED) != CST43684_CPUPLL_LOCKED), PMU_MAX_TRANSITION_DLY);
	if ((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipstatus), 0, 0) &
		CST43684_CPUPLL_LOCKED) == 0) {
		PMU_ERROR(("%s: PMU CPU PLL is not locked!!\n", __FUNCTION__));
		ASSERT(0);
	} else {
		PMU_MSG(("%s: PMU CPU PLL is locked\n", __FUNCTION__));
	}

	/* De-Assert cpupll_i_dreset */
	val &= ~(PMU_43684_CC1_FORCE_CPUPLL_DRESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Enable CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_43684_CC1_CPUPLL_HOLD_CH);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Recovery CPUPLL setting to default value */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_43684_CC1_PMU_CPUPLL_CONTROL_OVR);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

} /* si_set_cpu_vcofreq_43684 */

static void
BCMATTACHFN(si_set_cpu_vcofreq_6715)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 freq)
{
	uint32 oldpmucontrol, val;
	uint32 i_ndiv_int, i_ndiv_int_tmp, i_pdiv;
	uint32 p, q, p_tmp, q_tmp, fvco_tmp;
	int ndiv_frac_mod_sel = 0, change = 0;

	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6, 0, 0);
	i_pdiv = ((val & PMU6715_PLL1_PC6_PDIV_MASK) >> PMU6715_PLL1_PC6_PDIV_SHIFT);
	i_ndiv_int = ((val & PMU6715_PLL1_PC6_NDIV_INT_MASK) >>
		PMU6715_PLL1_PC6_NDIV_INT_SHIFT);

	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, 0, 0);
	p = ((val & PMU6715_PLL1_PC7_NDIV_P_MASK) >> PMU6715_PLL1_PC7_NDIV_P_SHIFT);
	q = ((val & PMU6715_PLL1_PC7_NDIV_Q_MASK) >> PMU6715_PLL1_PC7_NDIV_Q_SHIFT);

	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
	if (val & PMU6715_PLL1_PC8_NDIV_FRAC_MODE_SEL) {
		ndiv_frac_mod_sel = PMU6715_PLL1_PC8_NDIV_FRAC_MODE_SEL;
	}

	if (CHIPREV(sih->chiprev) < 2) {
		i_ndiv_int_tmp = I_NDIV_INT_6000;
		p_tmp = NDIV_P_6000;
		q_tmp = NDIV_Q_6000;
		fvco_tmp = FVCO_6000;
	} else {
		i_ndiv_int_tmp = I_NDIV_INT_4800;
		p_tmp = NDIV_P_4800;
		q_tmp = NDIV_Q_4800;
		fvco_tmp = FVCO_4800;
	}
	if (!(p == p_tmp && q == q_tmp && ndiv_frac_mod_sel == 0 &&
		i_ndiv_int == i_ndiv_int_tmp && i_pdiv == 1)) {
		change++;
	}

	if ((!change) && (freq == fvco_tmp)) {
		return;
	}

#ifndef NO_ONTHEFLY_FREQ_CHANGE
	ASSERT(0);
#endif /* NO_ONTHEFLY_FREQ_CHANGE */

	PMU_MSG(("%s(%d):cpuclk change:fvco %d ndiv_frac_mod_sel %d p %d q %d ndiv_int %d\n",
		__FUNCTION__, __LINE__, freq, ndiv_frac_mod_sel, p, q, i_ndiv_int));

	/* Stop CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= PMU_6715_CC5_CPUPLL_HOLD_CH;
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	i_pdiv = 1;
	ndiv_frac_mod_sel = 0;
	switch (freq) {
		case FVCO_4800:
			i_ndiv_int = I_NDIV_INT_4800;
			p = NDIV_P_4800;
			q = NDIV_Q_4800;
			break;
		case FVCO_6000:
			i_ndiv_int = I_NDIV_INT_6000;
			p = NDIV_P_6000;
			q = NDIV_Q_6000;
			break;
		default:
			ASSERT(0);
			break;

	}

	/* Set i_ndiv_int, i_pdiv */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6,
		PMU6715_PLL1_PC6_NDIV_INT_MASK | PMU6715_PLL1_PC6_PDIV_MASK,
		(i_ndiv_int << PMU6715_PLL1_PC6_NDIV_INT_SHIFT) | i_pdiv);
	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6, 0, 0);
	PMU_MSG(("%s(%d):PMU_PLL_CTRL_REG6 0x%x\n", __FUNCTION__, __LINE__, val));

	/* Set ndiv_p, ndiv_q  */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7,
		PMU6715_PLL1_PC7_NDIV_P_MASK|PMU6715_PLL1_PC7_NDIV_Q_MASK,
		((p << PMU6715_PLL1_PC7_NDIV_P_SHIFT) |
		(q << PMU6715_PLL1_PC7_NDIV_Q_SHIFT)));
	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, 0, 0);
	PMU_MSG(("%s(%d):PMU_PLL_CTRL_REG7 0x%x\n", __FUNCTION__, __LINE__, val));

	/* Set ndiv_frac_mod_sel */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, PMU6715_PLL1_PC8_NDIV_FRAC_MODE_SEL,
		ndiv_frac_mod_sel);
	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
	PMU_MSG(("%s(%d):PMU_PLL_CTRL_REG8 0x%x\n", __FUNCTION__, __LINE__, val));

	/* Set pllCntlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, (oldpmucontrol | PCTL_PLL_PLLCTL_UPD));
	R_REG(osh, &pmu->pmucontrol);

	/* Change CPU PLL controlled by force_cpupll_pwrdn_ldo,
	 * force_cpupll_areset and  force_cpupll_dreset
	 */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= (PMU_6715_CC5_PMU_CPUPLL_CONTROL_OVR | PMU_6715_CC5_FORCE_CPUPLL_ARESET |
		PMU_6715_CC5_FORCE_CPUPLL_DRESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* De-Assert cpupll_i_areset */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6715_CC5_FORCE_CPUPLL_ARESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Polling the CPU PLL LOCKED */
	SPINWAIT(((si_gci_chipstatus(sih, 0) & CST6715_CPUPLL_LOCKED) != CST6715_CPUPLL_LOCKED),
		PMU_MAX_TRANSITION_DLY);
	if ((si_gci_chipstatus(sih, 0) & CST6715_CPUPLL_LOCKED) == 0) {
		PMU_ERROR(("%s: PMU CPU PLL is not locked!!\n", __FUNCTION__));
		ASSERT(0);
	} else {
		PMU_MSG(("%s: PMU CPU PLL is locked\n", __FUNCTION__));
	}

	/* De-Assert cpupll_i_dreset */
	val &= ~(PMU_6715_CC5_FORCE_CPUPLL_DRESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Enable CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6715_CC5_CPUPLL_HOLD_CH);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Recovery CPUPLL setting to default value */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6715_CC5_PMU_CPUPLL_CONTROL_OVR);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

} /* si_set_cpu_vcofreq_6715 */

static void
BCMATTACHFN(si_set_cpu_vcofreq_6717)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 freq)
{
	uint32 oldpmucontrol, val;
	uint32 i_ndiv_int, i_ndiv_int_tmp, i_pdiv;
	uint32 p, q, p_tmp, q_tmp, fvco_tmp;
	int ndiv_frac_mod_sel = 0, change = 0;

	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, 0, 0);
	i_pdiv = ((val & PMU6717_PLL1_PC7_PDIV_MASK) >> PMU6717_PLL1_PC7_PDIV_SHIFT);
	i_ndiv_int = ((val & PMU6717_PLL1_PC7_NDIV_INT_MASK) >>
		PMU6717_PLL1_PC7_NDIV_INT_SHIFT);

	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
	p = ((val & PMU6717_PLL1_PC8_NDIV_P_MASK) >> PMU6717_PLL1_PC8_NDIV_P_SHIFT);
	q = ((val & PMU6717_PLL1_PC8_NDIV_Q_MASK) >> PMU6717_PLL1_PC8_NDIV_Q_SHIFT);
	if (val & PMU6717_PLL1_PC8_NDIV_FRAC_MODE_SEL) {
		ndiv_frac_mod_sel = PMU6717_PLL1_PC8_NDIV_FRAC_MODE_SEL;
	}

	i_ndiv_int_tmp = I_NDIV_INT_5200;
	p_tmp = NDIV_P_5200;
	q_tmp = NDIV_Q_5200;
	fvco_tmp = FVCO_5200;

	if (!(p == p_tmp && q == q_tmp && ndiv_frac_mod_sel == 0 &&
		i_ndiv_int == i_ndiv_int_tmp && i_pdiv == 2)) {
		change++;
	}

	if ((!change) && (freq == fvco_tmp)) {
		return;
	}

#ifndef NO_ONTHEFLY_FREQ_CHANGE
	ASSERT(0);
#endif /* NO_ONTHEFLY_FREQ_CHANGE */

	PMU_MSG(("%s(%d):cpuclk change:fvco %d ndiv_frac_mod %d p %d q %d pdiv %d ndiv_int %d\n",
		__FUNCTION__, __LINE__, freq, ndiv_frac_mod_sel, p, q, i_pdiv, i_ndiv_int));

	/* Stop CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= PMU_6717_CC5_CPUPLL_HOLD_CH;
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	i_pdiv = 2;
	ndiv_frac_mod_sel = 0;
	switch (freq) {
		case FVCO_5200:
			i_ndiv_int = I_NDIV_INT_5200;
			p = NDIV_P_5200;
			q = NDIV_Q_5200;
			break;
		default:
			ASSERT(0);
			break;

	}

	/* Set i_ndiv_int, i_pdiv */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7,
		PMU6717_PLL1_PC7_NDIV_INT_MASK | PMU6717_PLL1_PC7_PDIV_MASK,
		(i_ndiv_int << PMU6717_PLL1_PC7_NDIV_INT_SHIFT) | i_pdiv);
	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6, 0, 0);
	PMU_MSG(("%s(%d):PMU_PLL_CTRL_REG7 0x%x\n", __FUNCTION__, __LINE__, val));

	/* Set ndiv_p, ndiv_q, ndiv_frac_mod_sel */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8,
		PMU6717_PLL1_PC8_NDIV_P_MASK|PMU6717_PLL1_PC8_NDIV_Q_MASK|
		PMU6717_PLL1_PC8_NDIV_FRAC_MODE_SEL,
		((p << PMU6717_PLL1_PC8_NDIV_P_SHIFT) |
		(q << PMU6717_PLL1_PC8_NDIV_Q_SHIFT) |
		(ndiv_frac_mod_sel)));
	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
	PMU_MSG(("%s(%d):PMU_PLL_CTRL_REG8 0x%x\n", __FUNCTION__, __LINE__, val));

	/* Set pllCntlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, (oldpmucontrol | PCTL_PLL_PLLCTL_UPD));
	R_REG(osh, &pmu->pmucontrol);

	/* Change CPU PLL controlled by force_cpupll_pwrdn_ldo,
	 * force_cpupll_areset and  force_cpupll_dreset
	 */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= (PMU_6717_CC5_PMU_CPUPLL_CONTROL_OVR | PMU_6717_CC5_FORCE_CPUPLL_ARESET |
		PMU_6717_CC5_FORCE_CPUPLL_DRESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* De-Assert cpupll_i_areset */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6717_CC5_FORCE_CPUPLL_ARESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Polling the CPU PLL LOCKED */
	SPINWAIT(((si_gci_chipstatus(sih, 0) & CST6717_CPUPLL_LOCKED) != CST6717_CPUPLL_LOCKED),
		PMU_MAX_TRANSITION_DLY);
	if ((si_gci_chipstatus(sih, 0) & CST6717_CPUPLL_LOCKED) == 0) {
		PMU_ERROR(("%s: PMU CPU PLL is not locked!!\n", __FUNCTION__));
		ASSERT(0);
	} else {
		PMU_MSG(("%s: PMU CPU PLL is locked\n", __FUNCTION__));
	}

	/* De-Assert cpupll_i_dreset */
	val &= ~(PMU_6717_CC5_FORCE_CPUPLL_DRESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Enable CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6717_CC5_CPUPLL_HOLD_CH);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Recovery CPUPLL setting to default value */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6717_CC5_PMU_CPUPLL_CONTROL_OVR);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

} /* si_set_cpu_vcofreq_6717 */

static void
BCMATTACHFN(si_set_cpu_vcofreq_6726)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 freq)
{
	uint32 oldpmucontrol, val;
	uint32 i_ndiv_int, i_ndiv_int_tmp, i_pdiv;
	uint32 p, q, p_tmp, q_tmp, fvco_tmp;
	int ndiv_frac_mod_sel = 0, change = 0;

	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6, 0, 0);
	i_pdiv = ((val & PMU6726_PLL1_PC6_PDIV_MASK) >> PMU6726_PLL1_PC6_PDIV_SHIFT);
	i_ndiv_int = ((val & PMU6726_PLL1_PC6_NDIV_INT_MASK) >>
		PMU6726_PLL1_PC6_NDIV_INT_SHIFT);
	PMU_MSG(("[%s] PMU_PLL_CTRL_REG6: 0x%X\n", __FUNCTION__, val));

	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, 0, 0);
	p = ((val & PMU6726_PLL1_PC7_NDIV_P_MASK) >> PMU6726_PLL1_PC7_NDIV_P_SHIFT);
	q = ((val & PMU6726_PLL1_PC7_NDIV_Q_MASK) >> PMU6726_PLL1_PC7_NDIV_Q_SHIFT);
	PMU_MSG(("[%s] PMU_PLL_CTRL_REG7: 0x%X\n", __FUNCTION__, val));

	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
	if (val & PMU6726_PLL1_PC8_NDIV_FRAC_MODE_SEL) {
		ndiv_frac_mod_sel = PMU6717_PLL1_PC8_NDIV_FRAC_MODE_SEL;
	}
	PMU_MSG(("[%s] PMU_PLL_CTRL_REG8: 0x%X\n", __FUNCTION__, val));

#ifdef BCMQT
	if (BCM6726_CHIP(sih->chip)) {
		/* During 6726 WC veloce test, observed non zero p, q value
		 * occasionally. Stop programming vco as it does not make
		 * sense in warpcore db
		 */
		if ((p != 0) || (q != 0)) {
			/* Ideally we should not hit this condition */
			PMU_MSG(("[%s] non zero p/q: fvco %d ndiv_frac_mod %d p %d q %d "
				    "pdiv %d ndiv_int %d\n",
				    __FUNCTION__, freq, ndiv_frac_mod_sel, p, q, i_pdiv,
				    i_ndiv_int));
			return;
		}
	}
#endif /* BCMQT */

	if (freq == FVCO_4400) {
		i_ndiv_int_tmp = I_NDIV_INT_4400;
		p_tmp = NDIV_P_4400;
		q_tmp = NDIV_Q_4400;
	} else {
		ASSERT(freq == FVCO_4800);
		i_ndiv_int_tmp = I_NDIV_INT_4800;
		p_tmp = NDIV_P_4800;
		q_tmp = NDIV_Q_4800;
	}
	fvco_tmp = ((sih->otpflag & OTPFLAG_6726_HI_VOLT_DIS_BIT) == 0) ? FVCO_4400 : FVCO_4800;

	if (!(p == p_tmp && q == q_tmp && ndiv_frac_mod_sel == 0 &&
		i_ndiv_int == i_ndiv_int_tmp && i_pdiv == 2)) {
		change++;
	}

	if ((!change) && (freq == fvco_tmp)) {
		PMU_MSG(("[%s] No change in vco: fvco %d ndiv_frac_mod %d "
			"p %d q %d pdiv %d ndiv_int %d\n",
			__FUNCTION__, freq, ndiv_frac_mod_sel, p, q, i_pdiv,
			i_ndiv_int));
		return;
	}

#ifndef NO_ONTHEFLY_FREQ_CHANGE
	ASSERT(0);
#endif /* NO_ONTHEFLY_FREQ_CHANGE */

	PMU_MSG(("%s(%d):cpuclk change:fvco %d ndiv_frac_mod %d p %d q %d pdiv %d ndiv_int %d\n",
		__FUNCTION__, __LINE__, freq, ndiv_frac_mod_sel, p, q, i_pdiv, i_ndiv_int));

	/* Stop CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= PMU_6726_CC5_CPUPLL_HOLD_CH;
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	i_pdiv = 2;
	ndiv_frac_mod_sel = 0;
	switch (freq) {
		case FVCO_4400:
			i_ndiv_int = I_NDIV_INT_4400;
			p = NDIV_P_4400;
			q = NDIV_Q_4400;
			break;
		case FVCO_4800:
			i_ndiv_int = I_NDIV_INT_4800;
			p = NDIV_P_4800;
			q = NDIV_Q_4800;
			break;
		default:
			ASSERT(0);
			break;

	}

	/* Set i_ndiv_int, i_pdiv */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6,
		PMU6726_PLL1_PC6_NDIV_INT_MASK | PMU6726_PLL1_PC6_PDIV_MASK,
		(i_ndiv_int << PMU6726_PLL1_PC6_NDIV_INT_SHIFT) | i_pdiv);
	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6, 0, 0);
	PMU_MSG(("%s(%d):PMU_PLL_CTRL_REG6 0x%x\n", __FUNCTION__, __LINE__, val));

	/* Set ndiv_p, ndiv_q */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7,
		PMU6726_PLL1_PC7_NDIV_P_MASK | PMU6726_PLL1_PC7_NDIV_Q_MASK,
		(p << PMU6726_PLL1_PC7_NDIV_P_SHIFT) | (q << PMU6726_PLL1_PC7_NDIV_Q_SHIFT));
	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, 0, 0);
	PMU_MSG(("%s(%d):PMU_PLL_CTRL_REG7 0x%x\n", __FUNCTION__, __LINE__, val));

	/* Set ndiv_frac_mod_sel */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, PMU6726_PLL1_PC8_NDIV_FRAC_MODE_SEL,
		ndiv_frac_mod_sel);
	val = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
	PMU_MSG(("%s(%d):PMU_PLL_CTRL_REG8 0x%x\n", __FUNCTION__, __LINE__, val));

	/* Set pllCntlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, (oldpmucontrol | PCTL_PLL_PLLCTL_UPD));
	R_REG(osh, &pmu->pmucontrol);

	/* Change CPU PLL controlled by force_cpupll_pwrdn_ldo,
	 * force_cpupll_areset and  force_cpupll_dreset
	 */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= (PMU_6726_CC5_PMU_CPUPLL_CONTROL_OVR | PMU_6726_CC5_FORCE_CPUPLL_ARESET |
		PMU_6726_CC5_FORCE_CPUPLL_DRESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* De-Assert cpupll_i_areset */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6726_CC5_FORCE_CPUPLL_ARESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Polling the CPU PLL LOCKED, CST6717_CPUPLL_LOCKED can also be used for 6726 */
	SPINWAIT(((si_gci_chipstatus(sih, 0) & CST6717_CPUPLL_LOCKED) != CST6717_CPUPLL_LOCKED),
		PMU_MAX_TRANSITION_DLY);
	if ((si_gci_chipstatus(sih, 0) & CST6717_CPUPLL_LOCKED) == 0) {
		PMU_ERROR(("%s: PMU CPU PLL is not locked!!\n", __FUNCTION__));
		ASSERT(0);
	} else {
		PMU_MSG(("%s: PMU CPU PLL is locked\n", __FUNCTION__));
	}

	/* De-Assert cpupll_i_dreset */
	val &= ~(PMU_6726_CC5_FORCE_CPUPLL_DRESET);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Enable CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6726_CC5_CPUPLL_HOLD_CH);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* Recovery CPUPLL setting to default value */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(PMU_6726_CC5_PMU_CPUPLL_CONTROL_OVR);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

} /* si_set_cpu_vcofreq_6726 */

#include <bcmnvram.h>
#define CCICLK_43684	500
#define CCICLK_6715A0	750
#define CCICLK_6715B0	800
#define CCICLK_6717	1300
#define CCICLK_6726_LO	800
#define CCICLK_6726_HI	1100
#ifdef BCMQT
#define CPUCLK_43684	500
#define CPUCLK_6715A0	750
#define CPUCLK_6715B0	800
#define CPUCLK_6717	800
#define CPUCLK_6726_LO	800
#define CPUCLK_6726_HI	1100
#else /* !BCMQT */
#define CPUCLK_43684	1500
#define CPUCLK_6715A0	1500
#define CPUCLK_6715B0	1600
#define CPUCLK_6717	2600
#define CPUCLK_6726_LO	1600
#define CPUCLK_6726_HI	2200
#endif /* BCMQT */
#define CFG_CLK31_RATIO_SHIFT 17
#define CFG_CLK31_RATIO_MASK (1 << CFG_CLK31_RATIO_SHIFT)
#define CFG_CLK_RATIO_SHIFT 28

/** DONGLEBUILD specific */
static void
BCMATTACHFN(si_set_cpu_clock_43684)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 val, bp_on_ht, cpuclk = CPUCLK_43684, cciclk = CCICLK_43684, vcofreq = FVCO_3000;
	uint32 saved_clk_st = 0, max_res_mask = 0, min_res_mask = 0;
	uint8 ratio = 1;
	uint8 mdiv = 6;
	char *nvstr;
	char *end;

	/* ARM clock speed override */
	if ((nvstr = nvram_get("cpuclk"))) {
		cpuclk = bcm_strtoul(nvstr, &end, 0);
		if (*end == ',') {
			nvstr = ++end;
			cciclk = bcm_strtoul(nvstr, &end, 0);
			printf("cpuclk %d  cciclk %d \n", cpuclk, cciclk);
		}
	}

	switch (cpuclk) {
		case 1650:
			ratio = 3;
			mdiv = 2;
			vcofreq = FVCO_3300;
			break;
		case 1500:
			if (cciclk == 500) {
				ratio = 3;
			} else {
				ratio = 4;
			}
			mdiv = 2;
			break;
		case 1350:
			ratio = 3;
			mdiv = 2;
			vcofreq = FVCO_2700;
			break;
		case 1200:
			ratio = 3;
			mdiv = 3;
			vcofreq = FVCO_3600;
			break;
		case 1100:
			ratio = 3;
			mdiv = 3;
			vcofreq = FVCO_3300;
			break;
		case 1066:
			ratio = 3;
			mdiv = 3;
			vcofreq = FVCO_3200;
			break;
		case 1000:
			ratio = 3;
			mdiv = 3;
			vcofreq = FVCO_3000;
			break;
		case 900:
			ratio = 3;
			mdiv = 3;
			vcofreq = FVCO_2700;
			break;
		case 750:
			ratio = 2;
			mdiv = 4;
			vcofreq = FVCO_3000;
			break;
		case 500:
		default:
			break;
	}

	if (CHIPREV(sih->chiprev) >= 2) {
		/* For B0 the ratio is 0 or 1, 0 means 1:1, 1 means 3:1 */
		if (cpuclk == 500)
			ratio = 0;
		else
			ratio = 1;
	}

	/* check the current clock */
	bp_on_ht = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT;

	if (bp_on_ht) {
		/* Force BP on ALP  */
		si_pmu_pll_off(sih, osh, pmu, &min_res_mask, &max_res_mask, &saved_clk_st);
		ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_BP_ON_ALP);
	}

	si_set_cpu_vcofreq_43684(sih, osh, pmu, vcofreq);

	/* change mdiv */
	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG12);
	W_REG(osh, &pmu->pllcontrol_data, (uint32)mdiv);

	/* set pll ctrl update */
	W_REG(osh, &pmu->pmucontrol, (R_REG(osh, &pmu->pmucontrol) | (1 << 10)));

	/* change the cpu:cci ratio */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);

	if (CHIPREV(sih->chiprev) < 2) {
		/* For 43684 A0/A1 */
		val = 0x20006 | (ratio << CFG_CLK_RATIO_SHIFT);
		W_REG(osh, &pmu->chipcontrol_data, val);
		val &= ~(0x1 << 17);
		W_REG(osh, &pmu->chipcontrol_data, val);
	} else {
		/* For 43684 B0 */
		val = 0x10000006 | (ratio << CFG_CLK31_RATIO_SHIFT);
		W_REG(osh, &pmu->chipcontrol_data, val);
	}

	if (bp_on_ht) {
		/* Resume BP on HT back */
		si_pmu_pll_on(sih, osh, pmu, min_res_mask, max_res_mask, saved_clk_st);
		ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_BP_ON_HT);
		return;
	}

#ifdef NO_ONTHEFLY_FREQ_CHANGE
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_HTAREQ, CCS_HTAREQ);
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_HTAVAIL);

	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_FORCEHT, CCS_FORCEHT);
	asm volatile("isb");
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_BP_ON_HT);
#endif /* NO_ONTHEFLY_FREQ_CHANGE */
} /* si_set_cpu_clock_43684 */

static void
BCMATTACHFN(si_set_cpu_clock_6715)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 val, bp_on_ht, vcofreq = FVCO_6000, cpuclk = CPUCLK_6715A0, cciclk = CCICLK_6715A0;
	uint32 saved_clk_st = 0, max_res_mask = 0, min_res_mask = 0;
	uint8 ratio = 1; /* 2:1 */
	uint8 mdiv = 2;
	char *nvstr;
	char *end;

	if (CHIPREV(sih->chiprev) >= 2) {
		cpuclk = CPUCLK_6715B0;
		cciclk = CCICLK_6715B0;
	}
	/* ARM clock speed override */
	if ((nvstr = nvram_get("cpuclk"))) {
		cpuclk = bcm_strtoul(nvstr, &end, 0);
		if (*end == ',') {
			nvstr = ++end;
			cciclk = bcm_strtoul(nvstr, &end, 0);
			printf("cpuclk %d  cciclk %d \n", cpuclk, cciclk);
		}
	}

	switch (cpuclk) {
		case 1600:
			if (cciclk == 800) {
				/* 2:1 */
				ratio = 1;
			} else if (cciclk == 533) {
				/* 3:1 */
				ratio = 2;
			} else {
				/* 4:1, 400 MHz */
				ratio = 3;
			}
			vcofreq = FVCO_4800;
			mdiv = 3;
			break;

		case 800:
			/* 1:1 */
			ratio = 0;
			vcofreq = FVCO_4800;
			mdiv = 6;
			break;

		case 1500:
			if (cciclk == 750) {
				/* 2:1 */
				ratio = 1;
			} else if (cciclk == 500) {
				/* 3:1 */
				ratio = 2;
			} else {
				/* 4:1, 375 MHz */
				ratio = 3;
			}
			vcofreq = FVCO_6000;
			mdiv = 4;
			break;

		case 750:
			/* 1:1 */
			ratio = 0;
			vcofreq = FVCO_6000;
			mdiv = 8;
			break;

		default:
			break;
	}

	/* check the current clock */
	bp_on_ht = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT;

	if (bp_on_ht) {
		/* Force BP on ALP  */
		si_pmu_pll_off(sih, osh, pmu, &min_res_mask, &max_res_mask, &saved_clk_st);
		ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_BP_ON_ALP);
	}

	si_set_cpu_vcofreq_6715(sih, osh, pmu, vcofreq);

	/* change mdiv */
	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG9);
	R_REG(osh, &pmu->pllcontrol_addr);
	val = R_REG(osh, &pmu->pllcontrol_data);
	val &= ~PMU6715_PLL1_PC9_CH0_MDIV_MASK;
	val |= mdiv;
	W_REG(osh, &pmu->pllcontrol_data, val);

	/* set pll ctrl update */
	W_REG(osh, &pmu->pmucontrol, (R_REG(osh, &pmu->pmucontrol) | (1 << 10)));

	/* change the cpu:cci ratio */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL5);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val &= ~(0x3 << 17);
	val |= (ratio << 17);
	W_REG(osh, &pmu->chipcontrol_data, val);

	PMU_MSG(("%s: cpuclk %u cciclk %u ratio %u vcofreq %u mdiv %u\n",
		__FUNCTION__, cpuclk, cciclk, ratio, vcofreq, mdiv));

	if (bp_on_ht) {
		/* Resume BP on HT back */
		si_pmu_pll_on(sih, osh, pmu, min_res_mask, max_res_mask, saved_clk_st);
		ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_BP_ON_HT);
		return;
	}

#ifdef NO_ONTHEFLY_FREQ_CHANGE
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_HTAREQ, CCS_HTAREQ);
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_HTAVAIL);

	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_FORCEHT, CCS_FORCEHT);
	asm volatile("isb");
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_BP_ON_HT);
#endif /* NO_ONTHEFLY_FREQ_CHANGE */
} /* si_set_cpu_clock_6715 */

static void
BCMATTACHFN(si_set_cpu_clock_6717)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 val, bp_on_ht, vcofreq = FVCO_5200, cpuclk = CPUCLK_6717, cciclk = CCICLK_6717;
	uint32 saved_clk_st = 0, max_res_mask = 0, min_res_mask = 0;
	uint8 ratio = 1; /* 2:1 */
	char *nvstr;
	char *end;

	/* ARM clock speed override */
	if ((nvstr = nvram_get("cpuclk"))) {
		cpuclk = bcm_strtoul(nvstr, &end, 0);
		if (*end == ',') {
			nvstr = ++end;
			cciclk = bcm_strtoul(nvstr, &end, 0);
			printf("cpuclk %d  cciclk %d \n", cpuclk, cciclk);
		}
	}

	switch (cpuclk) {
		case 2600:
			if (cciclk == 1300) {
				/* 2:1 */
				ratio = 1;
			} else if (cciclk == 866) {
				/* 3:1 */
				ratio = 2;
			} else {
				ASSERT(0);
			}
			break;

		default:
			break;
	}

	/* check the current clock */
	bp_on_ht = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT;

	if (bp_on_ht) {
		/* Force BP on ALP  */
		si_pmu_pll_off(sih, osh, pmu, &min_res_mask, &max_res_mask, &saved_clk_st);
		ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_BP_ON_ALP);
	}

	si_set_cpu_vcofreq_6717(sih, osh, pmu, vcofreq);

	/* set pll ctrl update */
	W_REG(osh, &pmu->pmucontrol, (R_REG(osh, &pmu->pmucontrol) | (1 << 10)));

	/* change the cpu:cci ratio */
	{
		volatile void *regs;
		uint idx = si_coreidx(sih);

		regs = si_setcore(sih, ARMCA53_CORE_ID, 0);
		ASSERT(regs);

		val = R_REG(osh, ARMREG(regs, corecontrol));
		val &= ~(ARMCA53_CLOCKRATIO_MASK);
		val |= (ratio << ARMCA53_CLOCKRATIO_SHIFT);
		W_REG(osh, ARMREG(regs, corecontrol), val);

		si_setcoreidx(sih, idx);
	}

	PMU_MSG(("%s: cpuclk %u cciclk %u ratio %u vcofreq %u \n",
		__FUNCTION__, cpuclk, cciclk, ratio, vcofreq));

	if (bp_on_ht) {
		/* Resume BP on HT back */
		si_pmu_pll_on(sih, osh, pmu, min_res_mask, max_res_mask, saved_clk_st);
		ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_BP_ON_HT);
		return;
	}

#ifdef NO_ONTHEFLY_FREQ_CHANGE
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_HTAREQ, CCS_HTAREQ);
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_HTAVAIL);

	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_FORCEHT, CCS_FORCEHT);
	asm volatile("isb");
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_BP_ON_HT);
#endif /* NO_ONTHEFLY_FREQ_CHANGE */
} /* si_set_cpu_clock_6717 */

static void
BCMATTACHFN(si_set_cpu_clock_6726)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 val, bp_on_ht, vcofreq, cpuclk, cciclk;
	uint32 saved_clk_st = 0, max_res_mask = 0, min_res_mask = 0;
	uint8 ratio = 1; /* 2:1 */
	char *nvstr;
	char *end;
	bool high_performance = ((sih->otpflag & OTPFLAG_6726_HI_VOLT_DIS_BIT) == 0);

	if (high_performance) {
		/* high performance package */
		cpuclk = CPUCLK_6726_HI;
		cciclk = CCICLK_6726_HI;
		vcofreq = FVCO_4400;
	} else {
		cpuclk = CPUCLK_6726_LO;
		cciclk = CCICLK_6726_LO;
		vcofreq = FVCO_4800;
	}

	/* ARM clock speed override */
	if ((nvstr = nvram_get("cpuclk"))) {
		cpuclk = bcm_strtoul(nvstr, &end, 0);
		if (*end == ',') {
			nvstr = ++end;
			cciclk = bcm_strtoul(nvstr, &end, 0);
			printf("cpuclk %d  cciclk %d \n", cpuclk, cciclk);
		}
	} else {
		/* keep HW defaults */
		return;
	}

	switch (cpuclk) {
		/* high - performance modes */
		case 1100:
			if (cciclk == 1100) {
				/* 1:1 */
				ratio = 0;
			} else {
				ASSERT(0);
			}
			if (!high_performance)
				PMU_ERROR(("%s WARNING: %s performance: cpuclk %d, cciclk %d, "
					"ratio:%d\n", __FUNCTION__, high_performance ? "HIGH" :
					"LOW", cpuclk, cciclk, ratio));
			vcofreq = FVCO_4400;
			break;
		case 2200:
			if (cciclk == 1100) {
				/* 2:1 */
				ratio = 1;
			} else if (cciclk == 733) {
				/* 3:1 */
				ratio = 2;
			} else if (cciclk == 550) {
				/* 3:1 */
				ratio = 3;
			} else {
				ASSERT(0);
			}
			if (!high_performance)
				PMU_ERROR(("%s WARNING: %s performance: cpuclk %d, cciclk %d, "
					"ratio:%d\n", __FUNCTION__, high_performance ? "HIGH" :
					"LOW", cpuclk, cciclk, ratio));
			vcofreq = FVCO_4400;
			break;
		/* low - performance modes */
		case 800:
			if (cciclk == 800) {
				/* 1:1 */
				ratio = 0;
			} else {
				ASSERT(0);
			}
			vcofreq = FVCO_4800;
			break;
		case 1600:
			if (cciclk == 800) {
				/* 2:1 */
				ratio = 1;
			} else if (cciclk == 533) {
				/* 3:1 */
				ratio = 2;
			} else if (cciclk == 400) {
				/* 3:1 */
				ratio = 3;
			} else {
				ASSERT(0);
			}
			vcofreq = FVCO_4800;
			break;
		default:
			ASSERT(0);
			break;
	}

	/* check the current clock */
	bp_on_ht = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT;

	if (bp_on_ht) {
		/* Force BP on ALP  */
		si_pmu_pll_off(sih, osh, pmu, &min_res_mask, &max_res_mask, &saved_clk_st);
		ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_BP_ON_ALP);
	}

	si_set_cpu_vcofreq_6726(sih, osh, pmu, vcofreq);

	/* set pll ctrl update */
	W_REG(osh, &pmu->pmucontrol, (R_REG(osh, &pmu->pmucontrol) | (1 << 10)));

	/* change the cpu:cci ratio */
	{
		volatile void *regs;
		uint idx = si_coreidx(sih);

		regs = si_setcore(sih, ARMCA7_CORE_ID, 0);
		ASSERT(regs);

		val = R_REG(osh, ARMREG(regs, corecontrol));
		/* can use CA53 ratio mask/shift also for A7 */
		val &= ~(ARMCA53_CLOCKRATIO_MASK);
		val |= (ratio << ARMCA53_CLOCKRATIO_SHIFT);
		W_REG(osh, ARMREG(regs, corecontrol), val);

		si_setcoreidx(sih, idx);
	}

	PMU_MSG(("%s: cpuclk %u cciclk %u ratio %u vcofreq %u \n",
		__FUNCTION__, cpuclk, cciclk, ratio, vcofreq));

	if (bp_on_ht) {
		/* Resume BP on HT back */
		si_pmu_pll_on(sih, osh, pmu, min_res_mask, max_res_mask, saved_clk_st);
		ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_BP_ON_HT);
		return;
	}

#ifdef NO_ONTHEFLY_FREQ_CHANGE
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_HTAREQ, CCS_HTAREQ);
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_HTAVAIL);

	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_FORCEHT, CCS_FORCEHT);
	asm volatile("isb");
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_BP_ON_HT);
#endif /* NO_ONTHEFLY_FREQ_CHANGE */
} /* si_set_cpu_clock_6726 */
#endif /* defined(DONGLEBUILD) */

/** Enable PMU 1Mhz clock */
static void
si_pmu_enb_slow_clk(si_t *sih, osl_t *osh, uint32 xtalfreq)
{
	uint32 val;
	pmuregs_t *pmu;
	uint origidx;

	if (PMUREV(sih->pmurev) < 24) {
		PMU_ERROR(("%s: Not supported %d\n", __FUNCTION__, PMUREV(sih->pmurev)));
		return;
	}

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	/* twiki PmuRev30, OneMhzToggleEn:31, AlpPeriod[23:0] */
	if (PMUREV(sih->pmurev) >= 30) {
		uint32 alp_clk = xtalfreq;

		/* AlpPeriod = ROUND(POWER(2,26)/ALP_CLK_FREQ_IN_MHz,0) */
		/* Calculation will be accurate for only one decimal of xtal (like 37.4),
		* and will not be accurate for more than one decimal of xtal freq (like 37.43)
		* Also no rounding is done on final result
		*/
		ROMMABLE_ASSERT((alp_clk / 100) * 100 == alp_clk);
		if (BCM6717_CHIP(sih->chip) || BCM6726_CHIP(sih->chip) ||
		    BCM6711_CHIP(sih->chip) || EMBEDDED_11BE_CORE(sih->chip)) {
			alp_clk = xtalfreq / 2; /* these chips contain an ALP clock divider */
		}

		val = ((1 << 26) * 10) / (alp_clk / 100);
		/* set the 32 bit to enable OneMhzToggle
		 * -usec wide toggle signal will be generated
		 */
		val |= PMU30_ALPCLK_ONEMHZ_ENAB;
	} else { /* twiki PmuRev24, OneMhzToggleEn:16, AlpPeriod[15:0] */
		if (xtalfreq == 37400) {
			val = 0x101B6;
		} else if (xtalfreq == 40000) {
			val = 0x10199;
		} else {
			PMU_ERROR(("%s: xtalfreq is not supported, %d\n", __FUNCTION__, xtalfreq));
			return;
		}
	}

	/* Enable Precision Usec Timer */
	OR_REG(osh, &pmu->PrecisionTmrCtrlStatus, PMU_PREC_USEC_TIMER_ENABLE);

	W_REG(osh, &pmu->slowclkperiod, val);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
} /* si_pmu_enb_slow_clk */

/**
 * Initializes PLL given an x-tal frequency.
 * Calls si_pmuX_pllinitY() type of functions, where the reasoning behind 'X' and 'Y' is historical
 * rather than logical.
 *
 * xtalfreq : x-tal frequency in [KHz]
 */
void
BCMATTACHFN(si_pmu_pll_init)(si_t *sih, osl_t *osh, uint xtalfreq)
{
	pmuregs_t *pmu;
	uint origidx;
#ifdef BCMDBG
	char chn[8];
#endif
	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);
	BCM_REFERENCE(pmu);

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
#if defined(DONGLEBUILD)
		si_set_cpu_clock_43684(sih, osh, pmu);
#endif /* DONGLEBUILD */
		break;
	CASE_BCM6715_CHIP:
#if defined(DONGLEBUILD)
		si_set_cpu_clock_6715(sih, osh, pmu);
#endif /* DONGLEBUILD */
		break;
	CASE_BCM6717_CHIP:
#if defined(DONGLEBUILD)
		si_set_cpu_clock_6717(sih, osh, pmu);
#endif /* DONGLEBUILD */
		break;
	CASE_BCM6726_CHIP:
#if defined(DONGLEBUILD)
		si_set_cpu_clock_6726(sih, osh, pmu);
#endif /* DONGLEBUILD */
		break;
	CASE_EMBEDDED_2x2AX_CORE:
		si_pmu_spuravoid(sih, osh, 1);
		break;
	CASE_EMBEDDED_11BE_CORE:
		si_pmu_spuravoid(sih, osh, 1);
		break;
	default:
		PMU_MSG(("No PLL init done for chip %s rev %d pmurev %d\n",
		         bcm_chipname(
			 CHIPID(sih->chip), chn, 8), CHIPREV(sih->chiprev), PMUREV(sih->pmurev)));
		break;
	}

#ifdef BCMDBG_FORCEHT
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_FORCEHT, CCS_FORCEHT);

#if defined(DONGLEBUILD) && (defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_8A__))
	asm volatile("isb");
#endif /* defined(DONGLEBUILD) && defined(__ARM_ARCH_7A__) */

	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_BP_ON_HT) == 0), PMU_MAX_TRANSITION_DLY);
	ASSERT((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0) & CCS_BP_ON_HT)
		== CCS_BP_ON_HT);
#endif /* BCMDBG_FORCEHT */

	si_pmu_enb_slow_clk(sih, osh, xtalfreq);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
} /* si_pmu_pll_init */

/** gets the clock frequency of type clk_type in [Hz] units */
uint32
BCMINITFN(si_pmu_get_clock)(si_t *sih, osl_t *osh, enum si_pmu_clk_e clk_type)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 clk_hz;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);

	ASSERT(pmu != NULL);

	switch (clk_type) {
	case SI_PMU_CLK_XTAL:
	case SI_PMU_CLK_ALP:
		clk_hz = si_pmu_get_xtal_or_alp_clock(sih, osh, pmu, clk_type);
		break;
	case SI_PMU_CLK_ILP:
		clk_hz = si_pmu_ilp_clock(sih, osh, pmu);
		break;
	case SI_PMU_CLK_BACKPLANE:
		clk_hz = si_pmu_backplane_clk_hz(sih, osh);
		break;
	case SI_PMU_CLK_CPU:
		clk_hz = si_pmu_cpu_clock(sih, osh, pmu);
		break;
#ifdef DONGLEBUILD
	case SI_PMU_CLK_MEM:
		clk_hz = si_pmu_mem_clock(sih, osh, pmu);
		break;
#endif /* DONGLEBUILD */
	default:
		ASSERT(0);
		clk_hz = 0;
		break;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return clk_hz;
} /* si_pmu_get_clock */

static uint32
BCMINITFN(si_pmu_get_xtal_or_alp_clock)(si_t *sih, osl_t *osh, pmuregs_t *pmu,
          enum si_pmu_clk_e clk_type)
{
	uint32 clk_hz = ALP_CLOCK;
#ifdef BCMDBG
	char chn[8];
#endif

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP: /* alp clk == xtal clk */
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6715_CHIP:
	CASE_BCM6710_CHIP:
	CASE_BCM6717_CHIP:
	CASE_BCM6726_CHIP:
	CASE_EMBEDDED_11BE_CORE:
	CASE_BCM6711_CHIP:
		clk_hz = si_pmu_xtal_freq(sih, osh, pmu);
		break;
	default:
		PMU_MSG(("No ALP clock specified "
			"for chip %s rev %d pmurev %d, using default %d Hz\n",
			bcm_chipname(
			CHIPID(sih->chip), chn, 8), CHIPREV(sih->chiprev),
			PMUREV(sih->pmurev), clk_hz));
		break;
	}

	if (clk_type == SI_PMU_CLK_ALP) {
		switch (CHIPID(sih->chip)) {
		CASE_BCM6717_CHIP:
		CASE_BCM6726_CHIP:
		CASE_EMBEDDED_11BE_CORE:
		CASE_BCM6711_CHIP:
			/* these chips contain a clock divider */
			clk_hz /= 2;
			break;
		}
	}

	return clk_hz;
} /* si_pmu_get_clock */

/**
 * Find the output of the "m" pll divider given pll controls that start with
 * pllreg "pll0" i.e. 12 for main 6 for phy, 0 for misc.
 */
static uint32
BCMINITFN(si_pmu5_clock)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint pll0, uint m)
{
	uint32 tmp, div, ndiv, p1, p2, fc;

	if ((pll0 & 3) || (pll0 > PMU4716_MAINPLL_PLL0)) {
		PMU_ERROR(("%s: Bad pll0: %d\n", __FUNCTION__, pll0));
		return 0;
	}

	/* Strictly there is an m5 divider, but I'm not sure we use it */
	if ((m == 0) || (m > 4)) {
		PMU_ERROR(("%s: Bad m divider: %d\n", __FUNCTION__, m));
		return 0;
	}

	W_REG(osh, &pmu->pllcontrol_addr, pll0 + PMU5_PLL_P1P2_OFF);
	(void)R_REG(osh, &pmu->pllcontrol_addr);
	tmp = R_REG(osh, &pmu->pllcontrol_data);
	p1 = (tmp & PMU5_PLL_P1_MASK) >> PMU5_PLL_P1_SHIFT;
	p2 = (tmp & PMU5_PLL_P2_MASK) >> PMU5_PLL_P2_SHIFT;

	W_REG(osh, &pmu->pllcontrol_addr, pll0 + PMU5_PLL_M14_OFF);
	(void)R_REG(osh, &pmu->pllcontrol_addr);
	tmp = R_REG(osh, &pmu->pllcontrol_data);
	div = (tmp >> ((m - 1) * PMU5_PLL_MDIV_WIDTH)) & PMU5_PLL_MDIV_MASK;

	W_REG(osh, &pmu->pllcontrol_addr, pll0 + PMU5_PLL_NM5_OFF);
	(void)R_REG(osh, &pmu->pllcontrol_addr);
	tmp = R_REG(osh, &pmu->pllcontrol_data);
	ndiv = (tmp & PMU5_PLL_NDIV_MASK) >> PMU5_PLL_NDIV_SHIFT;

	/* Do calculation in Mhz */
	fc = si_pmu_get_clock(sih, osh, SI_PMU_CLK_ALP) / 1000000;
	fc = (p1 * ndiv * fc) / p2;

	PMU_NONE(("%s: p1=%d, p2=%d, ndiv=%d(0x%x), m%d=%d; fc=%d, clock=%d\n",
	          __FUNCTION__, p1, p2, ndiv, ndiv, m, div, fc, fc / div));

	/* Return clock in Hertz */
	return ((fc / div) * 1000000);
} /* si_pmu5_clock */

/** returns CPU clock frequency in [hz] units */
static uint32
BCMINITFN(si_pmu_cpu_clock)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 clock;	/* in [hz] units */

	ASSERT(sih->cccaps & CC_CAP_PMU);

	if (BCM43684_CHIP(sih->chip) ||
		BCM6715_CHIP(sih->chip) ||
		BCM6717_CHIP(sih->chip) ||
		BCM6726_CHIP(sih->chip)) {
		clock = si_pmu1_cpuclk0(sih, osh, pmu);
	} else {
		clock = si_pmu5_clock(sih, osh, pmu, PMU4716_MAINPLL_PLL0, PMU5_MAINPLL_CPU);
	}

	return clock;
} /* si_pmu_cpu_clock */

/**
 * Get backplane clock frequency, returns a value in [hz] units.
 * For designs that feed the same clock to both backplane and CPU just return the CPU clock speed.
 */
uint32
BCMINITFN(si_pmu_backplane_clk_hz)(si_t *sih, osl_t *osh)
{
	uint32 clock = HT_CLOCK;	/* in [hz] units */
	uint32 mdiv;
#ifdef BCMDBG
	char chn[8];
#endif

	ASSERT(sih->cccaps & CC_CAP_PMU);

	switch (CHIPID(sih->chip)) {
	CASE_BCM43684_CHIP:
	CASE_EMBEDDED_2x2AX_CORE:
		/* BP clock is derived from BB PLL, channel 1 */
		mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, 0, 0);
		mdiv = (mdiv & PMU1_PLL0_PC1_M2DIV_MASK) >> PMU1_PLL0_PC1_M2DIV_SHIFT;
		ASSERT(mdiv != 0);
		clock = si_pmu_bbpll_fvco0(sih) / mdiv * 1000;
		break;
	CASE_BCM6715_CHIP:
		/* BP clock is derived from BB PLL, channel 1 */
		mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, 0, 0);
		mdiv = (mdiv & PMU6715_PLL0_PC3_CH1_MDIV_MASK) >> PMU6715_PLL0_PC3_CH1_MDIV_SHIFT;
		ASSERT(mdiv != 0);
		clock = si_pmu_bbpll_fvco0(sih) / mdiv * 1000;
		break;
	CASE_BCM6717_CHIP:
		/* BP clock is derived from BB PLL, channel 1 */
		mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG5, 0, 0);
		mdiv = (mdiv & PMU6717_PLL0_PC5_CH1_MDIV_MASK) >> PMU6717_PLL0_PC5_CH1_MDIV_SHIFT;
		ASSERT(mdiv != 0);
		clock = si_pmu_bbpll_fvco0(sih) / mdiv * 1000;
		break;
	CASE_EMBEDDED_11BE_CORE: /* reuse 6726 definition */
	CASE_BCM6726_CHIP:
		/* BP clock is derived from BB PLL, channel 1 */
		mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, 0, 0);
		mdiv = (mdiv & PMU6726_PLL0_PC3_CH1_MDIV_MASK) >> PMU6726_PLL0_PC3_CH1_MDIV_SHIFT;
		ASSERT(mdiv != 0);
		clock = si_pmu_bbpll_fvco0(sih) / mdiv * 1000;
		break;
	default:
		PMU_MSG(("No backplane clock specified "
			"for chip %s rev %d pmurev %d, using default %d Hz\n",
			bcm_chipname(CHIPID(sih->chip), chn, 8), CHIPREV(sih->chiprev),
			PMUREV(sih->pmurev), clock));
		break;
	}

	return clock;
} /* si_pmu_backplane_clk_hz */

#ifdef DONGLEBUILD
/**
 * Gets memory clock frequency, which is the same as the HT clock for some chips. Returns [Hz].
 * Other chips derive mem clock from CPU clock. Function is only called for firmware builds.
 */
static uint32
BCMINITFN(si_pmu_mem_clock)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 clock;

	if (BCM6726_CHIP(sih->chip) || BCM6717_CHIP(sih->chip)) { /* CCI clock based on CPU clk */
		volatile void *regs;
		uint idx = si_coreidx(sih);
		uint32 ratio;

		/* read the cpu:cci ratio */
		regs = si_setcore(sih, BCM6717_CHIP(sih->chip)? ARMCA53_CORE_ID :
			ARMCA7_CORE_ID, 0);
		ASSERT(regs);
		/* can use CA53 ratio mask/shift also for A7 */
		ratio = (R_REG(osh, ARMREG(regs, corecontrol)) & ARMCA53_CLOCKRATIO_MASK) >>
			ARMCA53_CLOCKRATIO_SHIFT;
		si_setcoreidx(sih, idx);
		ratio += 1; /* 0 means 1:1 ratio, 1 means 2:1, etc */
		clock = si_pmu1_cpuclk0(sih, osh, pmu) / ratio;
	} else if (!(BCM43684_CHIP(sih->chip) || BCM6715_CHIP(sih->chip))) {
		clock = si_pmu5_clock(sih, osh, pmu, PMU4716_MAINPLL_PLL0, PMU5_MAINPLL_MEM);
	} else {
		clock = si_pmu_backplane_clk_hz(sih, osh); /* mem clk same as backplane clk */
	}

	return clock;
} /* si_pmu_mem_clock */
#endif /* DONGLEBUILD */

/*
 * ilpcycles per sec are now calculated during CPU init in a new way
 * for better accuracy.  We set it here for compatability.
 *
 * On platforms that do not do this we resort to the old way.
 */

#define ILP_CALC_DUR	10	/* ms, make sure 1000 can be divided by it. */

static uint32 ilpcycles_per_sec = 0;

void
BCMINITFN(si_pmu_ilp_clock_set)(uint32 cycles_per_sec)
{
	ilpcycles_per_sec = cycles_per_sec;
}

static uint32
BCMINITFN(si_pmu_ilp_clock)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	if (ISSIM_ENAB(sih))
		return ILP_CLOCK;

	if (ilpcycles_per_sec == 0) {
		uint32 start, end, delta;
		start = R_REG(osh, &pmu->pmutimer);
		if (start != R_REG(osh, &pmu->pmutimer))
			start = R_REG(osh, &pmu->pmutimer);
		OSL_DELAY(ILP_CALC_DUR * 1000);
		end = R_REG(osh, &pmu->pmutimer);
		if (end != R_REG(osh, &pmu->pmutimer))
			end = R_REG(osh, &pmu->pmutimer);
		delta = end - start;
		ilpcycles_per_sec = delta * (1000 / ILP_CALC_DUR);
	}

	ASSERT(ilpcycles_per_sec != 0);
	return ilpcycles_per_sec;
}

/**
 * Balance between stable SDIO operation and power consumption is achieved using this function.
 * Note that each drive strength table is for a specific VDDIO of the SDIO pads, ideally this
 * function should read the VDDIO itself to select the correct table. For now it has been solved
 * with the 'BCM_SDIO_VDDIO' preprocessor constant.
 *
 * 'drivestrength': desired pad drive strength in mA. Drive strength of 0 requests tri-state (if
 *		    hardware supports this), if no hw support drive strength is not programmed.
 */
void
BCMINITFN(si_sdiod_drive_strength_init)(si_t *sih, osl_t *osh, uint32 drivestrength)
{
	/*
	 * Note:
	 * This function used to set the SDIO drive strength via PMU_CHIPCTL1 for the
	 * 43143, 4330, 4334, 4336, 43362 chips.  These chips are now no longer supported, so
	 * the code has been deleted.
	 * Newer chips have the SDIO drive strength setting via a GCI Chip Control register,
	 * but the bit definitions are chip-specific.  We are keeping this function available
	 * (accessed via DHD 'sdiod_drive' IOVar) in case these newer chips need to provide access.
	 */
	UNUSED_PARAMETER(sih);
	UNUSED_PARAMETER(osh);
	UNUSED_PARAMETER(drivestrength);
}

/** initialize PMU */
void
BCMATTACHFN(si_pmu_init)(si_t *sih, osl_t *osh)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 val;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	OR_REG(osh, &pmu->pmucontrol, PCTL_NOILP_ON_WAIT);

	val = PMU_INTC_ALP_REQ | PMU_INTC_HT_REQ | PMU_INTC_HQ_REQ;
	pmu_corereg(sih, SI_CC_IDX, pmuintctrl0, val, val);

	val = RSRC_INTR_MASK_TIMER_INT_0;
	pmu_corereg(sih, SI_CC_IDX, pmuintmask0, val, val);
	pmu_corereg(sih, SI_CC_IDX, pmuintmask0, 0, 0);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

uint32
si_pmu_rsrc_macphy_clk_deps(si_t *sih, osl_t *osh, int maccore_index)
{
	uint32 deps;
	rsc_per_chip_t *rsc;
	uint origidx;
	pmuregs_t *pmu = NULL;
	uint8 rsc_num;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	rsc = si_pmu_get_rsc_positions(sih);
	if (maccore_index == 0) {
		rsc_num = rsc->macphy_clkavail;
	} else {
		rsc_num = rsc->macphy_aux_clkavail;
	}
	deps = si_pmu_res_deps(sih, osh, pmu, PMURES_BIT(rsc_num), TRUE);
	deps |= PMURES_BIT(rsc_num);

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return deps;
}

uint32
si_pmu_rsrc_ht_avail_clk_deps(si_t *sih, osl_t *osh)
{
	uint32 deps;
	rsc_per_chip_t *rsc;
	uint origidx;
	pmuregs_t *pmu = NULL;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	rsc = si_pmu_get_rsc_positions(sih);
	deps = si_pmu_res_deps(sih, osh, pmu, PMURES_BIT(rsc->ht_avail), FALSE);
	deps |= PMURES_BIT(rsc->ht_avail);

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return deps;
}

void
si_pmu_set_mac_rsrc_req(si_t *sih, int macunit)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	if (macunit == 0) {
		W_REG(osh, &pmu->mac_res_req_timer, PMU32_MAC_MAIN_RSRC_REQ_TIMER);
		W_REG(osh, &pmu->mac_res_req_mask, si_pmu_rsrc_macphy_clk_deps(sih, osh, macunit));
	} else if (macunit == 1) {
		W_REG(osh, &pmu->mac_res_req_timer1, PMU32_MAC_AUX_RSRC_REQ_TIMER);
		W_REG(osh, &pmu->mac_res_req_mask1, si_pmu_rsrc_macphy_clk_deps(sih, osh, macunit));
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

static uint
BCMINITFN(si_pmu_res_uptime)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint8 rsrc)
{
	uint32 deps;
	uint uptime, i, dup, dmax;
	uint32 min_mask = 0;
	uint32 max_mask = 0;

	/* uptime of resource 'rsrc' */
	W_REG(osh, &pmu->res_table_sel, (uint32)rsrc);
	uptime = (R_REG(osh, &pmu->res_updn_timer) >> 16) & 0x7fff;

	/* direct dependencies of resource 'rsrc' */
	deps = si_pmu_res_deps(sih, osh, pmu, PMURES_BIT(rsrc), FALSE);
	for (i = 0; i <= PMURES_MAX_RESNUM; i ++) {
		if (!(deps & PMURES_BIT(i)))
			continue;
		deps &= ~si_pmu_res_deps(sih, osh, pmu, PMURES_BIT(i), TRUE);
	}
	si_pmu_res_masks(sih, &min_mask, &max_mask);
	deps &= ~min_mask;

	/* max uptime of direct dependencies */
	dmax = 0;
	for (i = 0; i <= PMURES_MAX_RESNUM; i ++) {
		if (!(deps & PMURES_BIT(i)))
			continue;
		dup = si_pmu_res_uptime(sih, osh, pmu, (uint8)i);
		if (dmax < dup)
			dmax = dup;
	}

	PMU_NONE(("si_pmu_res_uptime: rsrc %u uptime %u(deps 0x%08x uptime %u)\n",
	         rsrc, uptime, deps, dmax));

	return uptime + dmax + PMURES_UP_TRANSITION;
}

/* Return dependencies (direct or all/indirect) for the given resources */
static uint32
si_pmu_res_deps(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 rsrcs, bool all)
{
	uint32 deps = 0;
	uint32 i;

	for (i = 0; i <= PMURES_MAX_RESNUM; i ++) {
		if (!(rsrcs & PMURES_BIT(i)))
			continue;
		W_REG(osh, &pmu->res_table_sel, i);
		deps |= R_REG(osh, &pmu->res_dep_mask);
	}

	return !all ? deps : (deps ? (deps | si_pmu_res_deps(sih, osh, pmu, deps, TRUE)) : 0);
}

/**
 * OTP is powered down/up as a means of resetting it, or for saving current when OTP is unused.
 * OTP is powered up/down through PMU resources.
 * OTP will turn OFF only if its not in the dependency of any "higher" rsrc in min_res_mask
 * none of the supported chips currently requires this.
 */
void
si_pmu_otp_power(si_t *sih, osl_t *osh, bool on, uint32* min_res_mask)
{
} /* si_pmu_otp_power */

void
si_pmu_spuravoid(si_t *sih, osl_t *osh, uint8 spuravoid)
{
	pmuregs_t *pmu;
	uint origidx, intr_val = 0;

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	/* update the pll changes */
	si_pmu_spuravoid_pllupdate(sih, pmu, osh, spuravoid);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	si_intrrestore(sih, intr_val);
} /* si_pmu_spuravoid */

/* below function are only for BBPLL parallel purpose */
void
si_pmu_spuravoid_isdone(si_t *sih, osl_t *osh, uint32 min_res_mask,
	uint32 max_res_mask, uint32 clk_ctl_st, uint8 spuravoid)
{
	pmuregs_t *pmu;
	uint origidx, intr_val = 0;

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	/* update the pll changes */
	si_pmu_spuravoid_pllupdate(sih, pmu, osh, spuravoid);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	si_intrrestore(sih, intr_val);
} /* si_pmu_spuravoid_isdone */

/* below function are only for BBPLL parallel purpose */

/* For having the pllcontrol data values for spuravoid */
typedef struct {
	uint8	spuravoid_mode;
	uint8	pllctrl_reg;
	uint32	pllctrl_regval;
} pllctrl_spuravoid_t;

/* LCNXN */
/* PLL Settings for spur avoidance on/off mode */

static void
si_pmu_spuravoid_pllupdate(si_t *sih, pmuregs_t *pmu, osl_t *osh, uint8 spuravoid)
{
	uint32 tmp = 0;
#ifdef BCMDBG_ERR
	char chn[8];
#endif

	switch (CHIPID(sih->chip)) {
	CASE_EMBEDDED_2x2AX_CORE:
		/* 63178 / 47622 / 6756 / 6880 */
		/* PLL Settings for spur avoidance on/off mode. XTAL is 50MHz */
		if (spuravoid == 1) {
			/* VCO 2880.51758MHz, BBPLL 320.0575MHz, MAC 240.0431MHz */
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, ~0, 0x00000391);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x0009C400);
		} else {
			/* VCO 2907.00474MHz, BBPLL 323.0005MHz, MAC 242.2504MHz */
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, ~0, 0x000003A1);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x00023DD4);
		}
		tmp = 1 << 10;
		break;
	CASE_EMBEDDED_2x2BE_320MHZ_CORE:
		/* 6765A0 */
		/* PLL Settings for spur avoidance on/off mode. XTAL is 80MHz */
		if (spuravoid == 1) {
			/* VCO 5440MHz, PHY 680MHz, MAC 777MHz */
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, ~0, 0x00000882);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x001c1c08);
			if ((sih->boardflags & BFL_BTCOEX) &&
			    !(sih->boardflags2 & (BFL2_BTCLEGACY | BFL2_BTC3WIREONLY))) {
				/* Allow 52.3MHz GCI clock for 4Mbaud SECI */
				si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x0000d007);
			} else {
				si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x0000ca07);
			}
		} else {
			/* VCO 5840MHz, PHY 649MHz, MAC 730MHz */
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, ~0, 0x00000922);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x00202009);
			if ((sih->boardflags & BFL_BTCOEX) &&
			    !(sih->boardflags2 & (BFL2_BTCLEGACY | BFL2_BTC3WIREONLY))) {
				/* Allow 52.14MHz GCI clock for 4Mbaud SECI */
				si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x0000e008);
			} else {
				si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x0000d808);
			}
		}
		tmp = 1 << 10;
		break;
	CASE_EMBEDDED_4x4BE_160MHZ_CORE:
		/* 6766A0 4x4 core */
		/* PLL Settings for spur avoidance on/off mode. XTAL is 80MHz */
		if (spuravoid == 1) {
			/* VCO 5440MHz, PHY 340MHz, MAC 777.143MHz */
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, ~0, 0x00000882);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x001c2010);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x0000c807);
		} else {
			/* VCO 5840MHz, PHY 343MHz, MAC 730MHz */
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, ~0, 0x00000922);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x00202412);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x0000d808);
		}
		tmp = 1 << 10;
		break;
	CASE_EMBEDDED_2x2BE_40MHZ_CORE:
		/* 6766A0 2x2 core */
		/* PLL Settings for spur avoidance on/off mode. XTAL is 80MHz */
		if (spuravoid == 1) {
			/* VCO 5440MHz, PHY 340MHz, MAC 604.444MHz */
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, ~0, 0x00000882);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x00245810);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x0000c807);
		} else {
			/* VCO 5840MHz, PHY 343MHz, MAC 449.231MHz */
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, ~0, 0x00000922);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x00345e12);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x0000d808);
		}
		tmp = 1 << 10;
		break;
	default:
		PMU_ERROR(("%s: unknown spuravoidance settings for chip %s, not changing PLL\n",
			__FUNCTION__, bcm_chipname(CHIPID(sih->chip), chn, 8)));
		break;
	}

	tmp |= R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, tmp);
} /* si_pmu_spuravoid_pllupdate */

bool
si_pmu_is_otp_powered(si_t *sih, osl_t *osh)
{
	uint idx;
	pmuregs_t *pmu;
	bool st;

	/* Remember original core before switch to chipc/pmu */
	idx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	si_pmu_wait_for_steady_state(sih, osh, pmu);

	switch (CHIPID(sih->chip)) {
	/* These chip doesn't use PMU bit to power up/down OTP. OTP always on.
	 * Use OTP_INIT command to reset/refresh state.
	 */
	CASE_BCM43684_CHIP:
	CASE_BCM6715_CHIP:
	CASE_BCM6717_CHIP:
	CASE_BCM6726_CHIP:
	CASE_BCM6711_CHIP:
		st = TRUE;
		break;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_EMBEDDED_11BE_CORE:
		st = FALSE;
		break;
	default:
		st = TRUE;
		break;
	}

	/* Return to original core */
	si_setcoreidx(sih, idx);
	return st;
} /* si_pmu_is_otp_powered */

/** initialize PMU chip controls and other chip level stuff */
void
BCMATTACHFN(si_pmu_chip_init)(si_t *sih, osl_t *osh)
{
	ASSERT(sih->cccaps & CC_CAP_PMU);

	si_pmu_otp_chipcontrol(sih, osh);

#ifdef CHIPC_UART_ALWAYS_ON
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), CCS_FORCEALP, CCS_FORCEALP);
#endif /* CHIPC_UART_ALWAYS_ON */

	/* Misc. chip control, has nothing to do with PMU */
	switch (CHIPID(sih->chip)) {
	CASE_BCM6715_CHIP:
	CASE_BCM6717_CHIP: /* 6717 PMU CC3 TXSTAT reg/bits same as 6715, reuse existing logic */
	CASE_BCM6726_CHIP: /* 6726 PMU CC3 TXSTAT reg/bits same as 6715, reuse existing logic */

#if defined(DONGLEBUILD)
		/* txstatus_ack_sel: by default is 0.
		 * 0: txstatus_ack from m2mdma (STS_XFER_TXS)
		 * 1: txstatus_ack from hwa (HWA4a)
		 */
#if defined(BCMHWA) && defined(HWA_TXSTAT_BUILD)
		/* Use HWA */
		si_pmu_chipcontrol(sih, PMU_CHIPCTL3, PMU_6715_CC3_TXSTATUS_ACK_SEL_MASK,
			PMU_6715_CC3_TXSTATUS_ACK_SEL_HWA);
#elif defined(STS_XFER_TXS)
		/* Use M2MDMA */
		si_pmu_chipcontrol(sih, PMU_CHIPCTL3, PMU_6715_CC3_TXSTATUS_ACK_SEL_MASK, 0);
#endif /* STS_XFER_TXS */
#endif /* DONGLEBUILD */
		break;
	CASE_BCM6710_CHIP:
	CASE_BCM43684_CHIP:
#ifdef BCM_ROUTER
		/* Force NIC-mode PMU to Power-Up state on warm-resets to
		 * prevent backplane lockups.
		 */
		si_pmu_set_resetcontrol(sih, PCTL_RESETCTL_PU);
#endif /* BCM_ROUTER */
		break;

	default:
		break;
	}
} /* si_pmu_chip_init */

/** Reference: http://confluence.broadcom.com/display/WLAN/Open+loop+Calibration+Sequence */
int
si_pmu_openloop_cal(si_t *sih, uint16 currtemp)
{
	int err = BCME_OK;
	switch (CHIPID(sih->chip)) {
	default:
		PMU_MSG(("%s: chip not supported!\n", __FUNCTION__));
		break;
	}
	return err;
}

void
si_pmu_slow_clk_reinit(si_t *sih, osl_t *osh)
{
	chipcregs_t *cc;
	uint origidx;
	uint32 xtalfreq;

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);
	if (cc == NULL)
		return;

	xtalfreq = getintvar(NULL, rstr_xtalfreq);
	/* If xtalfreq var not available, try to measure it */
	if (xtalfreq == 0)
		xtalfreq = si_pmu_measure_alpclk(sih, osh);
	si_pmu_enb_slow_clk(sih, osh, xtalfreq);
	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

/** initialize PMU registers in case default values proved to be suboptimal */
void
BCMATTACHFN(si_pmu_swreg_init)(si_t *sih, osl_t *osh)
{
	ASSERT(sih->cccaps & CC_CAP_PMU);

	si_pmu_otp_vreg_control(sih, osh);
} /* si_pmu_swreg_init */

/** Wait for a particular clock level to be on the backplane */
uint32
si_pmu_waitforclk_on_backplane(si_t *sih, osl_t *osh, uint32 clk, uint32 delay_val)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 val;

	ASSERT(sih->cccaps & CC_CAP_PMU);
	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	if (delay_val)
		SPINWAIT(((R_REG(osh, &pmu->pmustatus) & clk) != clk), delay_val);
	val = R_REG(osh, &pmu->pmustatus) & clk;

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	return (val);
}

#define EXT_ILP_HZ 32768

/**
 * Measures the ALP clock frequency in KHz.  Returns 0 if not possible.
 * Possible only if PMU rev >= 10 and there is an external LPO 32768Hz crystal.
 */
uint32
BCMATTACHFN(si_pmu_measure_alpclk)(si_t *sih, osl_t *osh)
{
	uint32 alp_khz;
	uint32 pmustat_lpo = 0;
	pmuregs_t *pmu;
	uint origidx;

	if (PMUREV(sih->pmurev) < 10)
		return 0;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	if (EMBEDDED_2x2AX_CORE(sih->chip) ||
		EMBEDDED_11BE_CORE(sih->chip) ||
		BCM6715_CHIP(sih->chip) ||
		BCM6717_CHIP(sih->chip) ||
		BCM6726_CHIP(sih->chip) ||
		BCM6711_CHIP(sih->chip) ||
		BCM43684_CHIP(sih->chip))
		pmustat_lpo = !(R_REG(osh, &pmu->pmucontrol) & PCTL_LPO_SEL);
	else
		pmustat_lpo = R_REG(osh, &pmu->pmustatus) & PST_EXTLPOAVAIL;

	if (pmustat_lpo) {
		uint32 ilp_ctr, alp_hz;

		/* Enable the reg to measure the freq, in case disabled before */
		W_REG(osh, &pmu->pmu_xtalfreq, 1U << PMU_XTALFREQ_REG_MEASURE_SHIFT);

		/* Delay for well over 4 ILP clocks */
		OSL_DELAY(1000);

		/* Read the latched number of ALP ticks per 4 ILP ticks */
		ilp_ctr = R_REG(osh, &pmu->pmu_xtalfreq) & PMU_XTALFREQ_REG_ILPCTR_MASK;

		/* Turn off the PMU_XTALFREQ_REG_MEASURE_SHIFT bit to save power */
		W_REG(osh, &pmu->pmu_xtalfreq, 0);

		/* Calculate ALP frequency */
		alp_hz = (ilp_ctr * EXT_ILP_HZ) / 4;

		/* Round to nearest 100KHz, and at the same time convert to KHz */
		alp_khz = (alp_hz + 50000) / 100000 * 100;
	} else {
		alp_khz = 0;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return alp_khz;
} /* si_pmu_measure_alpclk */

#ifdef DONGLEBUILD

#define PMUCAP_DUMP_TAG_SIZE_BYTES	4

static uint32 chipc_regs_to_dump[] = {
	OFFSETOF(chipcregs_t, clk_ctl_st),
	OFFSETOF(chipcregs_t, powerctl) };

/** size of the buffer needed to store the PMU register dump specifically PMU indirect registers */
uint32
si_pmu_dump_buf_size_pmucap(si_t *sih)
{
	uint32 buf_size = 0;
	uint32 cnt;

	if (PMUREV(sih->pmurev) < 5)
		return 0;

	/* pmu resources resource mask and resource updown */
	cnt = (sih->pmucaps & PCAP_RC_MASK) >> PCAP_RC_SHIFT;
	if (cnt) {
		buf_size += (cnt * 2 * sizeof(uint32)) + PMUCAP_DUMP_TAG_SIZE_BYTES;
	}
	/* pll controls */
	cnt = (sih->pmucaps & PCAP5_PC_MASK) >> PCAP5_PC_SHIFT;
	if (cnt) {
		buf_size += (cnt * sizeof(uint32)) + PMUCAP_DUMP_TAG_SIZE_BYTES;
	}

	/* voltage controls */
	cnt = (sih->pmucaps & PCAP5_VC_MASK) >> PCAP5_VC_SHIFT;
	if (cnt) {
		buf_size += (cnt * sizeof(uint32)) + PMUCAP_DUMP_TAG_SIZE_BYTES;
	}

	/* chip controls */
	cnt = (sih->pmucaps & PCAP5_CC_MASK) >> PCAP5_CC_SHIFT;
	if (cnt) {
		buf_size += (cnt * sizeof(uint32)) + PMUCAP_DUMP_TAG_SIZE_BYTES;
	}

	/* cnt indicates how many registers, tag_id 0 will say these are address/value */
	if (ARRAYSIZE(chipc_regs_to_dump)) {
		buf_size += PMUCAP_DUMP_TAG_SIZE_BYTES;
		/* address/value pairs */
		buf_size += sizeof(chipc_regs_to_dump);
		buf_size += sizeof(chipc_regs_to_dump);
	}

	return buf_size;
}

/**
 * routine to dump the registers into the user specified buffer
 * needed to store the PMU register dump specifically PMU indirect registers
 * format is sets of count, base regiser, register values
*/
uint32
si_pmu_dump_pmucap_binary(si_t *sih, uchar *p)
{
	uint32 cnt, i;
	osl_t *osh;
	pmuregs_t *pmu;
	uint origidx;
	uint32 *p32 = (uint32 *)p;

	if (PMUREV(sih->pmurev) < 5)
		return 0;

	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	osh = si_osh(sih);

	cnt = (sih->pmucaps & PCAP_RC_MASK) >> PCAP_RC_SHIFT;
	if (cnt) {
		*p32++ = (cnt << 16 | RSRCTABLEADDR);
		for (i = 0; i < cnt; i++) {
			W_REG(osh, &pmu->res_table_sel, i);
			*p32++ = R_REG(osh, &pmu->res_dep_mask);
			*p32++ = R_REG(osh, &pmu->res_updn_timer);
		}
	}

	cnt = (sih->pmucaps & PCAP5_PC_MASK) >> PCAP5_PC_SHIFT;
	if (cnt) {
		*p32++ = (cnt << 16 | PMU_PLL_CONTROL_ADDR);
		for (i = 0; i < cnt; i++) {
			*p32++ = si_pmu_pllcontrol(sih, i, 0, 0);
		}
	}

	cnt = (sih->pmucaps & PCAP5_VC_MASK) >> PCAP5_VC_SHIFT;
	if (cnt) {
		*p32++ = (cnt << 16 | PMU_REG_CONTROL_ADDR);
		for (i = 0; i < cnt; i++) {
			*p32++ = si_pmu_vreg_control(sih, i, 0, 0);
		}
	}
	cnt = (sih->pmucaps & PCAP5_CC_MASK) >> PCAP5_CC_SHIFT;
	if (cnt) {
		*p32++ = (cnt << 16 | CC_CHIPCTL_ADDR);
		for (i = 0; i < cnt; i++) {
			*p32++ = si_pmu_chipcontrol(sih, i, 0, 0);
		}
	}
	if (ARRAYSIZE(chipc_regs_to_dump)) {
		uint32 *addr;
		*p32++ = (ARRAYSIZE(chipc_regs_to_dump) << 16 | 0);
		for (i = 0; i < ARRAYSIZE(chipc_regs_to_dump); i++) {
			addr = (uint32 *)(SI_ENUM_BASE_PA(sih) + chipc_regs_to_dump[i]);
			*p32++ = (uint32)addr;
			*p32++ = R_REG(osh, addr);
		}
	}
	/* Return to original core */
	si_setcoreidx(sih, origidx);
	return 1;
} /* si_pmu_dump_pmucap_binary */

#endif /* DONGLEBUILD */
#ifndef WLTEST
/**
 * Function to enable the min_mask with specified resources along with its dependencies.
 * Also it can be used for bringing back to the default value of the device.
 */
static int
si_pmu_min_res_set(si_t *sih, osl_t *osh, uint min_mask, bool set)
{
	uint32 min_res, max_res;
	uint origidx, intr_val = 0;
	pmuregs_t *pmu;

	/* Block ints and save current core */
	intr_val = si_introff(sih);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	si_pmu_res_masks(sih, &min_res, &max_res);
	min_mask |= si_pmu_res_deps(sih, osh, pmu, min_mask, TRUE);

	/*
	 * If set is enabled, the resources specified in the min_mask is brought up. If not set,
	 * go to the default min_resource of the device.
	 */
	if (set) {
		OR_REG(osh, &pmu->min_res_mask, min_mask);
	} else {
		min_mask &= ~min_res;
		AND_REG(osh, &pmu->min_res_mask, ~min_mask);
	}

	si_pmu_wait_for_steady_state(sih, osh, pmu);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	si_intrrestore(sih, intr_val);

	return min_mask;
}
#endif	/* !WLTEST */

#ifndef WLTEST
int
si_pmu_min_res_otp_pu_set(si_t *sih, osl_t *osh, bool on)
{
	uint32 min_mask = 0;
	rsc_per_chip_t *rsc;

	rsc = si_pmu_get_rsc_positions(sih);
	if (rsc) {
		min_mask = PMURES_BIT(rsc->otp_pu);
	} else {
		return BCME_UNSUPPORTED;
	}
	si_pmu_min_res_set(sih, osh, min_mask, on);
	return BCME_OK;
}
#endif /* !WLTEST */

/*
 * si_pmu_set_resetcontrol
 * Set the PMU's reset-control behavior for NIC-mode devices
 */
void
si_pmu_set_resetcontrol(si_t *sih, uint32 value)
{
	if (PMUREV(sih->pmurev) >= 35) {
		value &= PCTL_RESETCTL_MASK;
		pmu_corereg(sih, SI_CC_IDX, pmucontrol, PCTL_RESETCTL_MASK, value);
	} else {
		PMU_ERROR(("%s not set due to version %d\n", __FUNCTION__, PMUREV(sih->pmurev)));
	}
}

/*
 * si_pmu_get_resetcontrol
 * Get the PMU's reset-control configuration for NIC-mode devices
 */
uint32
si_pmu_get_resetcontrol(si_t *sih)
{
	return pmu_corereg(sih, SI_CC_IDX, pmucontrol, 0, 0) & PCTL_RESETCTL_MASK;
}

#ifdef ATE_BUILD
void hnd_pmu_clr_int_sts_req_active(osl_t *hnd_osh, si_t *sih)
{
	uint32 res_req_timer;
	pmuregs_t *pmu;

	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);
	W_REG(hnd_osh, &pmu->pmuintstatus,
		RSRC_INTR_MASK_TIMER_INT_0);
	(void)R_REG(hnd_osh, &pmu->pmuintstatus);
	res_req_timer = R_REG(hnd_osh, &pmu->res_req_timer);
	W_REG(hnd_osh, &pmu->res_req_timer,
			res_req_timer & ~(PRRT_REQ_ACTIVE << flags_shift));
	(void)R_REG(hnd_osh, &pmu->res_req_timer);
}
#endif /* ATE_BUILD */

void si_pmu_set_min_res_mask(si_t *sih, osl_t *osh, uint min_res_mask)
{
	pmuregs_t *pmu;
	uint origidx;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	W_REG(osh, &pmu->min_res_mask, min_res_mask);
	OSL_DELAY(100);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

#ifdef BCMPMU_STATS
/*
 * 8 pmu statistics timer default map
 *
 * for CORE_RDY_AUX measure, set as below for timer 6 and 7 instead of CORE_RDY_MAIN.
 *	//core-n active duration : pmu_rsrc_state(CORE_RDY_AUX)
 *	{ SRC_CORE_RDY_AUX, FALSE, TRUE, LEVEL_HIGH},
 *	//core-n active duration : pmu_rsrc_state(CORE_RDY_AUX)
 *	{ SRC_CORE_RDY_AUX, FALSE, TRUE, EDGE_RISE}
 */
static pmu_stats_timer_t pmustatstimer[] = {
	{ SRC_LINK_IN_L12, FALSE, TRUE, PMU_STATS_LEVEL_HIGH},	//link_in_l12
	{ SRC_LINK_IN_L23, FALSE, TRUE, PMU_STATS_LEVEL_HIGH},	//link_in_l23
	{ SRC_PM_ST_IN_D0, FALSE, TRUE, PMU_STATS_LEVEL_HIGH},	//pm_st_in_d0
	{ SRC_PM_ST_IN_D3, FALSE, TRUE, PMU_STATS_LEVEL_HIGH},	//pm_st_in_d3
	//deep-sleep duration : pmu_rsrc_state(XTAL_PU)
	{ SRC_XTAL_PU, FALSE, TRUE, PMU_STATS_LEVEL_LOW},
	//deep-sleep entry count : pmu_rsrc_state(XTAL_PU)
	{ SRC_XTAL_PU, FALSE, TRUE, PMU_STATS_EDGE_FALL},
	//core-n active duration : pmu_rsrc_state(CORE_RDY_MAIN)
	{ SRC_CORE_RDY_MAIN, FALSE, TRUE, PMU_STATS_LEVEL_HIGH},
	//core-n active duration : pmu_rsrc_state(CORE_RDY_MAIN)
	{ SRC_CORE_RDY_MAIN, FALSE, TRUE, PMU_STATS_EDGE_RISE}
};

static void
si_pmustatstimer_update(osl_t *osh, pmuregs_t *pmu, uint8 timerid)
{
	uint32 stats_timer_ctrl;

	W_REG(osh, &pmu->pmu_statstimer_addr, timerid);
	stats_timer_ctrl =
		((pmustatstimer[timerid].src_num << PMU_ST_SRC_SHIFT) &
			PMU_ST_SRC_MASK) |
		((pmustatstimer[timerid].cnt_mode << PMU_ST_CNT_MODE_SHIFT) &
			PMU_ST_CNT_MODE_MASK) |
		((pmustatstimer[timerid].enable << PMU_ST_EN_SHIFT) & PMU_ST_EN_MASK) |
		((pmustatstimer[timerid].int_enable << PMU_ST_INT_EN_SHIFT) & PMU_ST_INT_EN_MASK);
	W_REG(osh, &pmu->pmu_statstimer_ctrl, stats_timer_ctrl);
	W_REG(osh, &pmu->pmu_statstimer_N, 0);
}

void
si_pmustatstimer_int_enable(si_t *sih)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	OR_REG(osh, &pmu->pmuintmask0, PMU_INT_STAT_TIMER_INT_MASK);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmustatstimer_int_disable(si_t *sih)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	AND_REG(osh, &pmu->pmuintmask0, ~PMU_INT_STAT_TIMER_INT_MASK);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmustatstimer_init(si_t *sih)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);
	uint32 core_cap_ext;
	uint8 max_stats_timer_num;
	int8 i;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	core_cap_ext = R_REG(osh, &pmu->core_cap_ext);

	max_stats_timer_num = ((core_cap_ext & PCAP_EXT_ST_NUM_MASK) >> PCAP_EXT_ST_NUM_SHIFT) + 1;

	for (i = 0; i < max_stats_timer_num; i++) {
		si_pmustatstimer_update(osh, pmu, i);
	}

	OR_REG(osh, &pmu->pmuintmask0, PMU_INT_STAT_TIMER_INT_MASK);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmustatstimer_dump(si_t *sih)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);
	uint32 core_cap_ext, pmucapabilities, AlpPeriod, ILPPeriod, pmuintmask0, pmuintstatus;
	uint8 max_stats_timer_num, max_stats_timer_src_num;
	uint32 stat_timer_ctrl, stat_timer_N;
	uint8 i;
	uint32 current_time_ms = OSL_SYSUPTIME();

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	pmucapabilities = R_REG(osh, &pmu->pmucapabilities);
	core_cap_ext = R_REG(osh, &pmu->core_cap_ext);
	AlpPeriod = R_REG(osh, &pmu->slowclkperiod);
	ILPPeriod = R_REG(osh, &pmu->ILPPeriod);

	max_stats_timer_num = ((core_cap_ext & PCAP_EXT_ST_NUM_MASK) >>
		PCAP_EXT_ST_NUM_SHIFT) + 1;
	max_stats_timer_src_num = ((core_cap_ext & PCAP_EXT_ST_SRC_NUM_MASK) >>
		PCAP_EXT_ST_SRC_NUM_SHIFT) + 1;

	pmuintstatus = R_REG(osh, &pmu->pmuintstatus);
	pmuintmask0 = R_REG(osh, &pmu->pmuintmask0);

	PMU_ERROR(("%s : TIME %d\n", __FUNCTION__, current_time_ms));

	PMU_ERROR(("\tMAX Timer Num %d, MAX Source Num %d\n",
		max_stats_timer_num, max_stats_timer_src_num));
	PMU_ERROR(("\tpmucapabilities 0x%8x, core_cap_ext 0x%8x, AlpPeriod 0x%8x, ILPPeriod 0x%8x, "
		"pmuintmask0 0x%8x, pmuintstatus 0x%8x, pmurev %d\n",
		pmucapabilities, core_cap_ext, AlpPeriod, ILPPeriod,
		pmuintmask0, pmuintstatus, PMUREV(sih->pmurev)));

	for (i = 0; i < max_stats_timer_num; i++) {
		W_REG(osh, &pmu->pmu_statstimer_addr, i);
		stat_timer_ctrl = R_REG(osh, &pmu->pmu_statstimer_ctrl);
		stat_timer_N = R_REG(osh, &pmu->pmu_statstimer_N);
		PMU_ERROR(("\t Timer %d : control 0x%8x, %d\n",
			i, stat_timer_ctrl, stat_timer_N));
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmustatstimer_start(si_t *sih, uint8 timerid)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	pmustatstimer[timerid].enable = TRUE;

	W_REG(osh, &pmu->pmu_statstimer_addr, timerid);
	OR_REG(osh, &pmu->pmu_statstimer_ctrl, PMU_ST_ENAB << PMU_ST_EN_SHIFT);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmustatstimer_stop(si_t *sih, uint8 timerid)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	pmustatstimer[timerid].enable = FALSE;

	W_REG(osh, &pmu->pmu_statstimer_addr, timerid);
	AND_REG(osh, &pmu->pmu_statstimer_ctrl, ~(PMU_ST_ENAB << PMU_ST_EN_SHIFT));

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmustatstimer_clear(si_t *sih, uint8 timerid)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	W_REG(osh, &pmu->pmu_statstimer_addr, timerid);
	W_REG(osh, &pmu->pmu_statstimer_N, 0);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmustatstimer_clear_overflow(si_t *sih)
{
	uint8 i;
	uint32 core_cap_ext;
	uint8 max_stats_timer_num;
	uint32 timerN;
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	core_cap_ext = R_REG(osh, &pmu->core_cap_ext);
	max_stats_timer_num = ((core_cap_ext & PCAP_EXT_ST_NUM_MASK) >> PCAP_EXT_ST_NUM_SHIFT) + 1;

	for (i = 0; i < max_stats_timer_num; i++) {
		W_REG(osh, &pmu->pmu_statstimer_addr, i);
		timerN = R_REG(osh, &pmu->pmu_statstimer_N);
		if (timerN == 0xFFFFFFFF) {
			PMU_ERROR(("pmustatstimer overflow clear - timerid : %d\n", i));
			si_pmustatstimer_clear(sih, i);
		}
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

uint32
si_pmustatstimer_read(si_t *sih, uint8 timerid)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);
	uint32 stats_timer_N;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	W_REG(osh, &pmu->pmu_statstimer_addr, timerid);
	stats_timer_N = R_REG(osh, &pmu->pmu_statstimer_N);

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return stats_timer_N;
}

void
si_pmustatstimer_cfg_src_num(si_t *sih, uint8 src_num, uint8 timerid)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	pmustatstimer[timerid].src_num = src_num;
	si_pmustatstimer_update(osh, pmu, timerid);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmustatstimer_cfg_cnt_mode(si_t *sih, uint8 cnt_mode, uint8 timerid)
{
	pmuregs_t *pmu;
	uint origidx;
	osl_t *osh = si_osh(sih);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	pmu = si_setcore(sih, PMU_CORE_ID, 0);
	ASSERT(pmu != NULL);

	pmustatstimer[timerid].cnt_mode = cnt_mode;
	si_pmustatstimer_update(osh, pmu, timerid);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}
#endif /* BCMPMU_STATS */

/**
 * On the fly CPU VCo register update (#NO_ONTHEFLY_FREQ_CHANGE) would slow down firmware
 * initialization. Prevent it by clearing the pll_ctrl_reg #8 register (which includes the
 * PMU6715_PLL1_PC8_NDIV_FRAC_MODE_SEL bit) prior to firmware download.
 *
 * Preconditions:
 * - The connected chip is a 6715
 * - The 6715 ARM core is in reset
 */
void
si_pmu_6715_prevent_on_the_fly_arm_vco_change(si_t *sih)
{
	uint origidx;

	origidx = si_coreidx(sih);
	if (si_setcore(sih, PMU_CORE_ID, 0) != NULL) {
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0xFFFFFFFF, 0);
	}

	si_setcoreidx(sih, origidx);
}
