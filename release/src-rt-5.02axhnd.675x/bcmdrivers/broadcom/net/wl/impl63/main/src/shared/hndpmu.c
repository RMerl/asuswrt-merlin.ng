/*
 * Misc utility routines for accessing PMU corerev specific features
 * of the SiliconBackplane-based Broadcom chips.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 * $Id: hndpmu.c 788587 2020-07-06 01:46:22Z $
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
#endif // endif
#endif /* DONGLEBUILD */
#if !defined(BCMDONGLEHOST)
#include <bcmotp.h>
#endif // endif
#if defined(SAVERESTORE) && !defined(BCMDONGLEHOST)
#include <saverestore.h>
#endif // endif
#include <hndlhl.h>
#if defined(BCMULP)
#include <ulp.h>
#endif /* defined(BCMULP) */
#include <sbgci.h>
#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif
#include <sbgci.h>
#include <lpflags.h>

#if defined(EVENT_LOG_COMPILE) && defined(BCMDBG_ERR) && defined(ERR_USE_EVENT_LOG)
#if defined(ERR_USE_EVENT_LOG_RA)
#define PMU_ERROR(args) EVENT_LOG_RA(EVENT_LOG_TAG_PMU_ERROR, args)
#else
#define PMU_ERROR(args) EVENT_LOG_COMPACT_CAST_PAREN_ARGS(EVENT_LOG_TAG_PMU_ERROR, args)
#endif /* ERR_USE_EVENT_LOG_RA */
#elif defined(BCMDBG_ERR)
#define	PMU_ERROR(args)	printf args
#else
#define	PMU_ERROR(args)
#endif	/* defined(BCMDBG_ERR) && defined(ERR_USE_EVENT_LOG) */

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

#if !defined(BCMDONGLEHOST)
/* PLL controls/clocks */
static void si_pmu1_pllinit1(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 xtal);
static void si_pmu_pll_off(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 *min_mask,
	uint32 *max_mask, uint32 *clk_ctl_st);
static void si_pmu_pll_off_isdone(si_t *sih, osl_t *osh, pmuregs_t *pmu);
static void si_pmu_pll_on(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 min_mask,
	uint32 max_mask, uint32 clk_ctl_st);
void si_pmu_pll_4364_macfreq_switch(si_t *sih, osl_t *osh, uint8 mode);
void si_pmu_otp_pllcontrol(si_t *sih, osl_t *osh);
void si_pmu_otp_vreg_control(si_t *sih, osl_t *osh);
void si_pmu_otp_chipcontrol(si_t *sih, osl_t *osh);
uint32 si_pmu_def_alp_clock(si_t *sih, osl_t *osh);
bool si_pmu_update_pllcontrol(si_t *sih, osl_t *osh, uint32 xtal, bool update_required);
static uint32 si_pmu_htclk_mask(si_t *sih);

static uint32 si_pmu1_cpuclk0(si_t *sih, osl_t *osh, pmuregs_t *pmu);
static uint32 si_pmu1_alpclk0(si_t *sih, osl_t *osh, pmuregs_t *pmu);

static uint32 si_pmu1_cpuclk0_pll2(si_t *sih, osl_t *osh, pmuregs_t *pmu);

/* PMU resources */
static uint32 si_pmu_res_deps(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 rsrcs, bool all);
static uint si_pmu_res_uptime(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint8 rsrc);
static void si_pmu_res_masks(si_t *sih, uint32 *pmin, uint32 *pmax);
static void si_pmu_spuravoid_pllupdate(si_t *sih, pmuregs_t *pmu, osl_t *osh, uint8 spuravoid);

#ifndef WLTEST
static int si_pmu_min_res_set(si_t *sih, osl_t *osh, uint min_mask, bool set);
#endif // endif
uint32 si_pmu_get_pmutime_diff(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 *prev);
bool si_pmu_wait_for_res_pending(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint usec,
	bool cond, uint32 *elapsed_time);

static uint8 fastlpo_dis_get(void);
static uint8 fastlpo_pcie_dis_get(void);

#ifdef BCMULP

typedef struct si_pmu_ulp_cr_dat {
	uint32 ilpcycles_per_sec;
} si_pmu_ulp_cr_dat_t;

static uint si_pmu_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo);
static int si_pmu_ulp_enter_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data);
static int si_pmu_ulp_exit_cb(void *handle, uint8 *cache_data, uint8 *p2_cache_data);
static const ulp_p1_module_pubctx_t ulp_pmu_ctx = {
	MODCBFL_CTYPE_STATIC,
	si_pmu_ulp_enter_cb,
	si_pmu_ulp_exit_cb,
	si_pmu_ulp_get_retention_size_cb,
	NULL,
	NULL
};

#endif /* BCMULP */

/* PMU timer ticks once in 32uS */
#define PMU_US_STEPS (32)

void *g_si_pmutmr_lock_arg = NULL;
si_pmu_callback_t g_si_pmutmr_lock_cb = NULL, g_si_pmutmr_unlock_cb = NULL;

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
#define FVCO_1938	1938000 /* 1938MHz */
#define FVCO_385	385000  /**< 385MHz */
#define FVCO_400	400000  /**< 420MHz */
#define FVCO_500	500000  /**< 500MHz */
#define FVCO_1000	1000000 /**< 1000MHz */
#define FVCO_2000	2000000 /**< 2000MHz */
#define FVCO_2700	2700000	/**< 2700 MHz */
#define FVCO_2880	2880000	/**< 2880 MHz */
#define FVCO_2907	2907005	/**< 2907 MHz */
#define FVCO_2946	2946000	/**< 2946 MHz */
#define FVCO_3000	3000000	/**< 3000 MHz */
#define FVCO_3200	3200000	/**< 3200 MHz */
#define FVCO_3300	3300000	/**< 3300 MHz */
#define FVCO_3600	3600000	/**< 3600 MHz */
#define FVCO_5760	5760000	/**< 5760 MHz */
#define FVCO_5814	5814000	/**< 5814 MHz */
#define FVCO_6000	6000000	/**< 6000 MHz */

/* defines to make the code more readable */
#define NO_SUCH_RESOURCE	0	/**< means: chip does not have such a PMU resource */

/* uses these defines instead of 'magic' values when writing to register pllcontrol_addr */
#define PMU_PLL_CTRL_REG0	0
#define PMU_PLL_CTRL_REG1	1
#define PMU_PLL_CTRL_REG2	2
#define PMU_PLL_CTRL_REG3	3
#define PMU_PLL_CTRL_REG4	4
#define PMU_PLL_CTRL_REG5	5
#define PMU_PLL_CTRL_REG6	6
#define PMU_PLL_CTRL_REG7	7
#define PMU_PLL_CTRL_REG8	8
#define PMU_PLL_CTRL_REG9	9
#define PMU_PLL_CTRL_REG10	10
#define PMU_PLL_CTRL_REG11	11
#define PMU_PLL_CTRL_REG12	12
#define PMU_PLL_CTRL_REG13	13
#define PMU_PLL_CTRL_REG14	14
#define PMU_PLL_CTRL_REG15	15

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
 * Reads/writes a PLL control register. Performes core switching if required, at function exit the
 * original core is restored. Depending on chip type, writes to PLL control regs in CC core (older
 * chips) or to PLL control regs in PMU core (later chips).
 */
uint32
si_pmu_pllcontrol(si_t *sih, uint reg, uint32 mask, uint32 val)
{
	pmu_corereg(sih, SI_CC_IDX, pllcontrol_addr, ~0, reg);
	return pmu_corereg(sih, SI_CC_IDX, pllcontrol_data, mask, val);
}

/**
 * The chip has one or more PLLs/FLLs (e.g. baseband PLL, USB PHY PLL). The settings of each PLL are
 * contained within one or more 'PLL control' registers. Since the PLL hardware requires that
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
	pmu_corereg(sih, SI_CC_IDX, pmucontrol,
	           PCTL_PLL_PLLCTL_UPD, PCTL_PLL_PLLCTL_UPD);
}

static rsc_per_chip_t rsc_4350 =  {RES4350_HT_AVAIL,  RES4350_MACPHY_CLKAVAIL,
		RES4350_HT_START,  RES4350_OTP_PU, NO_SUCH_RESOURCE};
/* 4360_OTP_PU is used for 4352, not a typo */
static rsc_per_chip_t rsc_4352 =  {NO_SUCH_RESOURCE,  NO_SUCH_RESOURCE,
		NO_SUCH_RESOURCE,  RES4360_OTP_PU, NO_SUCH_RESOURCE};
static rsc_per_chip_t rsc_4360 =  {RES4360_HT_AVAIL,  NO_SUCH_RESOURCE,
		NO_SUCH_RESOURCE,  RES4360_OTP_PU, NO_SUCH_RESOURCE};
static rsc_per_chip_t rsc_43602 = {RES43602_HT_AVAIL, RES43602_MACPHY_CLKAVAIL,
		RES43602_HT_START, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};
static rsc_per_chip_t rsc_4365 = {RES4365_HT_AVAIL, RES4365_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};
static rsc_per_chip_t rsc_4347 = {RES4347_HT_AVAIL, RES4347_MACPHY_MAIN_CLK_AVAIL,
		RES4347_HT_AVAIL, RES4347_AAON, RES4347_MACPHY_AUX_CLK_AVAIL};

static rsc_per_chip_t rsc_43684 = {RES43684_HT_AVAIL,  RES43684_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

/* 63178 and 6878 uses same top level and hence both share same resource table */
static rsc_per_chip_t rsc_63178 = {RES2x2AX_HT_AVAIL, RES2x2AX_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

static rsc_per_chip_t rsc_6710 = {RES6710_HT_AVAIL,  RES6710_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

static rsc_per_chip_t rsc_6715 = {RES6715_HT_AVAIL,  RES6715_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, NO_SUCH_RESOURCE, NO_SUCH_RESOURCE};

static rsc_per_chip_t rsc_53573 = {RES53573_HT_AVAIL, RES53573_MACPHY_CLK_AVAIL,
		NO_SUCH_RESOURCE, RES53573_OTP_PU, NO_SUCH_RESOURCE};

/**
* For each chip, location of resource bits (e.g., ht bit) in resource mask registers may differ.
* This function abstracts the bit position of commonly used resources, thus making the rest of the
* code in hndpmu.c cleaner.
*/
static rsc_per_chip_t* BCMRAMFN(si_pmu_get_rsc_positions)(si_t *sih)
{
	rsc_per_chip_t *rsc = NULL;

	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
		rsc = &rsc_4350;
		break;
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID:	/* usb variant of 4352 */
		rsc = &rsc_4352;
		break;
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
		rsc = &rsc_4360;
		break;
	CASE_BCM43602_CHIP:
		rsc = &rsc_43602;
		break;
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		rsc = &rsc_4365;
		break;
	case BCM4347_CHIP_GRPID:
		rsc = &rsc_4347;
		break;
	case BCM53573_CHIP_GRPID:
		rsc = &rsc_53573;
		break;
	CASE_BCM43684_CHIP:
		rsc = &rsc_43684;
		break;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
		rsc = &rsc_63178;
		break;
	CASE_BCM6710_CHIP:
		rsc = &rsc_6710;
		break;
	CASE_BCM6715_CHIP:
		rsc = &rsc_6715;
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
static const char BCMATTACHDATA(rstr_Invalid_Unsupported_xtal_value_D)[] =
	"Invalid/Unsupported xtal value %d";
static const char BCMATTACHDATA(rstr_dacrate2g)[] = "dacrate2g";
static const char BCMATTACHDATA(rstr_clkreq_conf)[] = "clkreq_conf";
static const char BCMATTACHDATA(rstr_cbuckout)[] = "cbuckout";
static const char BCMATTACHDATA(rstr_cldo_ldo2)[] = "cldo_ldo2";
static const char BCMATTACHDATA(rstr_cldo_pwm)[] = "cldo_pwm";
static const char BCMATTACHDATA(rstr_cldo_burst)[] = "cldo_burst";
static const char BCMATTACHDATA(rstr_lpldo1)[] = "lpldo1";
static const char BCMATTACHDATA(rstr_lnldo1)[] = "lnldo1";
static const char BCMATTACHDATA(rstr_force_pwm_cbuck)[] = "force_pwm_cbuck";
static const char BCMATTACHDATA(rstr_xtalfreq)[] = "xtalfreq";
#if !defined(BCMDONGLEHOST) && defined(DONGLEBUILD)
static const char BCMATTACHDATA(rstr_avs_enab)[] = "avs_enab";
#endif // endif
#if defined(SAVERESTORE) && defined(LDO3P3_MIN_RES_MASK)
static const char BCMATTACHDATA(rstr_ldo_prot)[] = "ldo_prot";
#endif /* SAVERESTORE && LDO3P3_MIN_RES_MASK */
static const char BCMATTACHDATA(rstr_btldo3p3pu)[] = "btldopu";
static const char BCMATTACHDATA(rstr_fastlpo_dis)[] = "fastlpo_dis";
static const char BCMATTACHDATA(rstr_fastlpo_pcie_dis)[] = "fastlpo_pcie_dis";
static const char BCMATTACHDATA(rstr_gpio_drive_strength)[] = "gpio_drive_strength";
static const char BCMATTACHDATA(rstr_memlpldo_volt)[] = "memlpldo_volt";
static const char BCMATTACHDATA(rstr_lpldo_volt)[] = "lpldo_volt";

/* The check for OTP parameters for the PLL control registers is done and if found the
 * registers are updated accordingly.
 */

/**
 * As a hardware bug workaround, OTP can contain variables in the form 'pll%d=%d'.
 * If these variables are present, the corresponding PLL control register(s) are
 * overwritten, but not yet 'updated'.
 */
void
BCMATTACHFN(si_pmu_otp_pllcontrol)(si_t *sih, osl_t *osh)
{
	char name[16];
	const char *otp_val;
	uint8 i;
	uint32 val;
	uint8 pll_ctrlcnt = 0;

	if (PMUREV(sih->pmurev) >= 5) {
		pll_ctrlcnt = (sih->pmucaps & PCAP5_PC_MASK) >> PCAP5_PC_SHIFT;
	} else {
		pll_ctrlcnt = (sih->pmucaps & PCAP_PC_MASK) >> PCAP_PC_SHIFT;
	}

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

	if (PMUREV(sih->pmurev) >= 5) {
		vreg_ctrlcnt = (sih->pmucaps & PCAP5_VC_MASK) >> PCAP5_VC_SHIFT;
	} else {
		vreg_ctrlcnt = (sih->pmucaps & PCAP_VC_MASK) >> PCAP_VC_SHIFT;
	}

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

	if (PMUREV(sih->pmurev) >= 5) {
		cc_ctrlcnt = (sih->pmucaps & PCAP5_CC_MASK) >> PCAP5_CC_SHIFT;
	} else {
		cc_ctrlcnt = (sih->pmucaps & PCAP_CC_MASK) >> PCAP_CC_SHIFT;
	}

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
	uint8 sr_cntl_shift = 0, rc_shift = 0, shift = 0, mask = 0;
	uint8 addr = 0;
	uint8 do_reg2 = 0, rshift2 = 0, rc_shift2 = 0, mask2 = 0, addr2 = 0;

	BCM_REFERENCE(osh);

	ASSERT(sih->cccaps & CC_CAP_PMU);

	switch (CHIPID(sih->chip)) {
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID:
		switch (ldo) {
		case  SET_LDO_VOLTAGE_PAREF:
			addr = 1;
			rc_shift = 0;
			mask = 0xf;
			break;
		default:
			ASSERT(FALSE);
			break;
		}
		break;
	CASE_BCM43602_CHIP:
		switch (ldo) {
		case  SET_LDO_VOLTAGE_PAREF:
			addr = 0;
			rc_shift = 29;
			mask = 0x7;
			do_reg2 = 1;
			addr2 = 1;
			rshift2 = 3;
			mask2 = 0x8;
			break;
		default:
			ASSERT(FALSE);
			break;
		}
		break;
	default:
		ASSERT(FALSE);
		return;
	}

	shift = sr_cntl_shift + rc_shift;

	pmu_corereg(sih, SI_CC_IDX, regcontrol_addr, /* PMU VREG register */
		~0, addr);
	pmu_corereg(sih, SI_CC_IDX, regcontrol_data,
		mask << shift, (voltage & mask) << shift);
	if (do_reg2) {
		/* rshift2 - right shift moves mask2 to bit 0, rc_shift2 - left shift in reg */
		si_pmu_vreg_control(sih, addr2, (mask2 >> rshift2) << rc_shift2,
			((voltage & mask2) >> rshift2) << rc_shift2);
	}
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
	uint pmudelay = PMU_MAX_TRANSITION_DLY;
	pmuregs_t *pmu;
	uint origidx;
	uint32 ilp;			/* ILP clock frequency in [Hz] */
	rsc_per_chip_t *rsc;		/* chip specific resource bit positions */

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	if (ISSIM_ENAB(sih)) {
		pmudelay = 70;
	} else {
		switch (CHIPID(sih->chip)) {
		case BCM43217_CHIP_ID:
		case BCM43460_CHIP_ID:
		case BCM43526_CHIP_ID:
			pmudelay = 3700;
			break;
		case BCM4360_CHIP_ID:
		case BCM4352_CHIP_ID:
			if (CHIPREV(sih->chiprev) < 4) {
				pmudelay = 1500;
			} else {
				pmudelay = 3000;
			}
			break;
		case BCM4363_CHIP_ID:
		case BCM4365_CHIP_ID:
		case BCM4366_CHIP_ID:
		case BCM43664_CHIP_ID:
	        case BCM43666_CHIP_ID:
		case BCM43570_CHIP_ID:
		case BCM53573_CHIP_GRPID:
			rsc = si_pmu_get_rsc_positions(sih);
			/* Retrieve time by reading it out of the hardware */
			ilp = si_ilp_clock(sih);
			if (ilp != 0) {
				pmudelay = (si_pmu_res_uptime(sih, osh, pmu, rsc->ht_avail) +
					D11SCC_SLOW2FAST_TRANSITION) * ((1000000 + ilp - 1) / ilp);
				pmudelay = (11 * pmudelay) / 10;
			}
			break;
		CASE_BCM43602_CHIP:
			rsc = si_pmu_get_rsc_positions(sih);
			/* Retrieve time by reading it out of the hardware */
			ilp = si_ilp_clock(sih);
			if (ilp != 0) {
				pmudelay = (si_pmu_res_uptime(sih, osh, pmu, rsc->macphy_clkavail) +
					D11SCC_SLOW2FAST_TRANSITION) * ((1000000 + ilp - 1) / ilp);
				pmudelay = (11 * pmudelay) / 10;
			}
			break;
		case BCM4347_CHIP_GRPID:
			rsc = si_pmu_get_rsc_positions(sih);
			/* Retrieve time by reading it out of the hardware */
			ilp = si_ilp_clock(sih);
			if (ilp != 0) {
				pmudelay = si_pmu_res_uptime(sih, osh, pmu, rsc->ht_avail) +
					D11SCC_SLOW2FAST_TRANSITION;
				pmudelay = (11 * pmudelay) / 10;
			}
			break;
		CASE_BCM43684_CHIP:
		CASE_EMBEDDED_2x2AX_CORE:
		CASE_BCM6878_CHIP:
		CASE_BCM6710_CHIP:
		CASE_BCM6715_CHIP:
			rsc = si_pmu_get_rsc_positions(sih);
			/* Retrieve time by reading it out of the hardware */
			ilp = si_ilp_clock(sih);
			if (ilp != 0) {
				pmudelay = si_pmu_res_uptime(sih, osh, pmu, rsc->ht_avail) +
					D11SCC_SLOW2FAST_TRANSITION;
				pmudelay = (11 * pmudelay) / 10;
			}
			break;
		default:
			break;
		}
	} /* if (ISSIM_ENAB(sih)) */

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return (uint16)pmudelay;
} /* si_pmu_fast_pwrup_delay */

/*
 * During chip bringup, it can turn out that the 'hard wired' PMU dependencies are not fully
 * correct, or that up/down time values can be optimized. The following data structures and arrays
 * deal with that.
 */

/* Setup resource up/down timers */
typedef struct {
	uint8 resnum;
	uint32 updown;
} pmu_res_updown_t;

/* Change resource dependencies masks */
typedef struct {
	uint32 res_mask;		/* resources (chip specific) */
	int8 action;			/* action, e.g. RES_DEPEND_SET */
	uint32 depend_mask;		/* changes to the dependencies mask */
	bool (*filter)(si_t *sih);	/* action is taken when filter is NULL or return TRUE */
} pmu_res_depend_t;

/* Resource dependencies mask change action */
#define RES_DEPEND_SET		0	/* Override the dependencies mask */
#define RES_DEPEND_ADD		1	/* Add to the  dependencies mask */
#define RES_DEPEND_REMOVE	-1	/* Remove from the dependencies mask */

static const pmu_res_updown_t BCMATTACHDATA(bcm4350_res_updown)[] = {
	{RES4350_XTAL_PU,		0x00260002},
	{RES4350_LQ_AVAIL,		0x00010001},
	{RES4350_LQ_START,		0x00010001},
	{RES4350_WL_CORE_RDY,		0x00010001},
	{RES4350_ALP_AVAIL,		0x00010001},
	{RES4350_SR_CLK_STABLE,		0x00010001},
	{RES4350_SR_SLEEP,		0x00010001},
	{RES4350_HT_AVAIL,		0x00010001},

#ifndef SRFAST
	{RES4350_SR_PHY_PWRSW,		0x00120002},
	{RES4350_SR_VDDM_PWRSW,		0x00120002},
	{RES4350_SR_SUBCORE_PWRSW,	0x00120002},
#else /* SRFAST */
	{RES4350_RSVD_7,	        0x000c0001},
	{RES4350_PMU_BG_PU,		0x00010001},
	{RES4350_PMU_SLEEP,		0x00200004},
	{RES4350_CBUCK_LPOM_PU,		0x00010001},
	{RES4350_CBUCK_PFM_PU,		0x00010001},
	{RES4350_COLD_START_WAIT,	0x00010001},
	{RES4350_LNLDO_PU,		0x00200004},
	{RES4350_XTALLDO_PU,		0x00050001},
	{RES4350_LDO3P3_PU,		0x00200004},
	{RES4350_OTP_PU,		0x00010001},
	{RES4350_SR_CLK_START,		0x000f0001},
	{RES4350_PERST_OVR,		0x00010001},
	{RES4350_WL_CORE_RDY,		0x00010001},
	{RES4350_ALP_AVAIL,		0x00010001},
	{RES4350_MINI_PMU,		0x00010001},
	{RES4350_RADIO_PU,		0x00010001},
	{RES4350_SR_CLK_STABLE,		0x00010001},
	{RES4350_SR_SLEEP,		0x00010001},
	{RES4350_HT_START,		0x000f0001},
	{RES4350_HT_AVAIL,		0x00010001},
	{RES4350_MACPHY_CLKAVAIL,	0x00010001},
#endif /* SRFAST */
};

static const pmu_res_depend_t BCMATTACHDATA(bcm4350_res_depend)[] = {
#ifdef SRFAST
	{
		PMURES_BIT(RES4350_LDO3P3_PU),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4350_PMU_SLEEP),
		NULL
	},
	{
		PMURES_BIT(RES4350_WL_CORE_RDY) |
		PMURES_BIT(RES4350_ALP_AVAIL) |
		PMURES_BIT(RES4350_SR_CLK_STABLE) |
		PMURES_BIT(RES4350_SR_SAVE_RESTORE) |
		PMURES_BIT(RES4350_SR_SUBCORE_PWRSW) |
		PMURES_BIT(RES4350_SR_SLEEP) |
		PMURES_BIT(RES4350_MACPHY_CLKAVAIL) |
		PMURES_BIT(RES4350_MINI_PMU) |
		PMURES_BIT(RES4350_ILP_REQ) |
		PMURES_BIT(RES4350_HT_START) |
		PMURES_BIT(RES4350_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LDO3P3_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_SR_CLK_START) |
		PMURES_BIT(RES4350_WL_CORE_RDY) |
		PMURES_BIT(RES4350_ALP_AVAIL) |
		PMURES_BIT(RES4350_SR_CLK_STABLE) |
		PMURES_BIT(RES4350_SR_SAVE_RESTORE) |
		PMURES_BIT(RES4350_SR_PHY_PWRSW) |
		PMURES_BIT(RES4350_SR_VDDM_PWRSW) |
		PMURES_BIT(RES4350_SR_SUBCORE_PWRSW) |
		PMURES_BIT(RES4350_SR_SLEEP) |
		PMURES_BIT(RES4350_RADIO_PU) |
		PMURES_BIT(RES4350_MACPHY_CLKAVAIL) |
		PMURES_BIT(RES4350_MINI_PMU) |
		PMURES_BIT(RES4350_HT_START) |
		PMURES_BIT(RES4350_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_RSVD_7),
		NULL
	},
	{
		PMURES_BIT(RES4350_SR_CLK_START) |
		PMURES_BIT(RES4350_WL_CORE_RDY) |
		PMURES_BIT(RES4350_SR_CLK_STABLE) |
		PMURES_BIT(RES4350_SR_SAVE_RESTORE) |
		PMURES_BIT(RES4350_SR_SLEEP) |
		PMURES_BIT(RES4350_RADIO_PU) |
		PMURES_BIT(RES4350_CBUCK_PFM_PU) |
		PMURES_BIT(RES4350_MINI_PMU),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_XTAL_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_SR_CLK_START) |
		PMURES_BIT(RES4350_WL_CORE_RDY) |
		PMURES_BIT(RES4350_SR_CLK_STABLE) |
		PMURES_BIT(RES4350_SR_SAVE_RESTORE) |
		PMURES_BIT(RES4350_SR_SLEEP) |
		PMURES_BIT(RES4350_RADIO_PU) |
		PMURES_BIT(RES4350_CBUCK_PFM_PU) |
		PMURES_BIT(RES4350_MINI_PMU),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_XTALLDO_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_COLD_START_WAIT) |
		PMURES_BIT(RES4350_XTALLDO_PU) |
		PMURES_BIT(RES4350_XTAL_PU) |
		PMURES_BIT(RES4350_SR_CLK_START) |
		PMURES_BIT(RES4350_SR_CLK_STABLE) |
		PMURES_BIT(RES4350_SR_PHY_PWRSW) |
		PMURES_BIT(RES4350_SR_VDDM_PWRSW) |
		PMURES_BIT(RES4350_SR_SUBCORE_PWRSW),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4350_CBUCK_PFM_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_RSVD_7),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_XTALLDO_PU) |
		PMURES_BIT(RES4350_COLD_START_WAIT) |
		PMURES_BIT(RES4350_LNLDO_PU) |
		PMURES_BIT(RES4350_PMU_SLEEP) |
		PMURES_BIT(RES4350_CBUCK_LPOM_PU) |
		PMURES_BIT(RES4350_PMU_BG_PU) |
		PMURES_BIT(RES4350_LPLDO_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_LNLDO_PU),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4350_PMU_SLEEP) |
		PMURES_BIT(RES4350_CBUCK_LPOM_PU) |
		PMURES_BIT(RES4350_CBUCK_PFM_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_ALP_AVAIL) |
		PMURES_BIT(RES4350_RADIO_PU) |
		PMURES_BIT(RES4350_HT_START) |
		PMURES_BIT(RES4350_HT_AVAIL) |
		PMURES_BIT(RES4350_MACPHY_CLKAVAIL),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4350_LQ_AVAIL) |
		PMURES_BIT(RES4350_LQ_START),
		NULL
	},
#else
	{0, 0, 0, NULL}
#endif /* SRFAST */
};

#ifndef BCM_BOOTLOADER
static const pmu_res_depend_t BCMATTACHDATA(bcm4350_res_pciewar)[] = {
	{
#ifdef SRFAST
		PMURES_BIT(RES4350_SR_PHY_PWRSW) |
		PMURES_BIT(RES4350_SR_VDDM_PWRSW),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_XTALLDO_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_CBUCK_PFM_PU),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LNLDO_PU) |
		PMURES_BIT(RES4350_COLD_START_WAIT),
		NULL
	},
	{
		PMURES_BIT(RES4350_OTP_PU),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4350_CBUCK_PFM_PU),
		NULL
	},
	{
#endif /* SRFAST */
		PMURES_BIT(RES4350_LQ_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_LQ_START),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_PERST_OVR),
		RES_DEPEND_SET,
		PMURES_BIT(RES4350_LPLDO_PU) |
		PMURES_BIT(RES4350_PMU_BG_PU) |
		PMURES_BIT(RES4350_PMU_SLEEP) |
		PMURES_BIT(RES4350_CBUCK_LPOM_PU) |
		PMURES_BIT(RES4350_CBUCK_PFM_PU) |
		PMURES_BIT(RES4350_COLD_START_WAIT) |
		PMURES_BIT(RES4350_LNLDO_PU) |
		PMURES_BIT(RES4350_XTALLDO_PU) |
#ifdef SRFAST
		PMURES_BIT(RES4350_OTP_PU) |
#endif // endif
		PMURES_BIT(RES4350_XTAL_PU) |
#ifdef SRFAST
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_SR_CLK_START) |
#endif // endif
		PMURES_BIT(RES4350_SR_CLK_STABLE) |
		PMURES_BIT(RES4350_SR_SAVE_RESTORE) |
		PMURES_BIT(RES4350_SR_PHY_PWRSW) |
		PMURES_BIT(RES4350_SR_VDDM_PWRSW) |
		PMURES_BIT(RES4350_SR_SUBCORE_PWRSW) |
		PMURES_BIT(RES4350_SR_SLEEP),
		NULL
	},
	{
		PMURES_BIT(RES4350_WL_CORE_RDY),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_PERST_OVR) |
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_ILP_REQ),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_ALP_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_PERST_OVR) |
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_RADIO_PU),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_PERST_OVR) |
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_SR_CLK_STABLE),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_SR_SAVE_RESTORE),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_SR_SUBCORE_PWRSW),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LDO3P3_PU) |
#ifdef SRFAST
		PMURES_BIT(RES4350_XTALLDO_PU) |
#endif // endif
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_SR_SLEEP),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_HT_START),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_PERST_OVR) |
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_PERST_OVR) |
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4350_MACPHY_CLKAVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4350_PERST_OVR) |
		PMURES_BIT(RES4350_LDO3P3_PU) |
		PMURES_BIT(RES4350_OTP_PU),
		NULL
	}
};
#endif /* BCM_BOOTLOADER */

static const pmu_res_updown_t BCMATTACHDATA(bcm4360_res_updown)[] = {
	{RES4360_BBPLLPWRSW_PU,		0x00200001}
};

static const pmu_res_updown_t BCMATTACHDATA(bcm43602_res_updown)[] = {
	{RES43602_SR_SAVE_RESTORE,	0x00190019},
	{RES43602_XTAL_PU,		0x00280002},
	{RES43602_RFLDO_PU,		0x00430005}
};

/* 53573 res up down times  Need to populate with correct values !!! */
static const pmu_res_updown_t BCMATTACHDATA(bcm53573_res_updown)[] = {
	{RES53573_REGULATOR_PU, 0x00000000},
	{RES53573_XTALLDO_PU, 0x000A0000},
	{RES53573_XTAL_PU, 0x01d50000},
	{RES53573_MINI_PMU, 0x00020000},
	{RES53573_RADIO_PU, 0x00040000},
	{RES53573_ILP_REQ, 0x00020000},
	{RES53573_ALP_AVAIL, 0x00000000},
	{RES53573_CPUPLL_LDO_PU, 0x00000000},
	{RES53573_CPU_PLL_PU, 0x00020000},
	{RES53573_WLAN_BB_PLL_PU, 0x00000000},
	{RES53573_MISCPLL_LDO_PU, 0x00000000},
	{RES53573_MISCPLL_PU, 0x00020000},
	{RES53573_AUDIOPLL_PU, 0x00000000},
	{RES53573_PCIEPLL_LDO_PU, 0x00000000},
	{RES53573_PCIEPLL_PU, 0x00020000},
	{RES53573_DDRPLL_LDO_PU, 0x00000000},
	{RES53573_DDRPLL_PU, 0x00020000},
	{RES53573_HT_AVAIL, 0x00000000},
	{RES53573_MACPHY_CLK_AVAIL, 0x00000000},
	{RES53573_OTP_PU, 0x00000000},
	{RES53573_RSVD20, 0x00000000},
};

/* 53573 Res dependency
Reference: http://confluence.broadcom.com/display/WLAN/
BCM53573+Top+Level+Architecture#BCM53573TopLevelArchitecture-PMUResourcetable
*/
static const pmu_res_depend_t BCMATTACHDATA(bcm53573a0_res_depend)[] = {
	{
/* RSRC 0 0x0 */
		PMURES_BIT(RES53573_REGULATOR_PU),
		RES_DEPEND_SET,
		0x00000000,
		NULL
	},
	{
/* RSRC 1 0x1 */
		PMURES_BIT(RES53573_XTALLDO_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 2 0x3 */
		PMURES_BIT(RES53573_XTAL_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_XTALLDO_PU) | PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 3 0x3 */
		PMURES_BIT(RES53573_MINI_PMU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_XTALLDO_PU) | PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 4 0xF */
		PMURES_BIT(RES53573_RADIO_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_MINI_PMU) | PMURES_BIT(RES53573_XTAL_PU) |
		PMURES_BIT(RES53573_XTALLDO_PU) | PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 5 0x3 */
		PMURES_BIT(RES53573_ILP_REQ),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 6 0x7 */
		PMURES_BIT(RES53573_ALP_AVAIL),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_OTP_PU) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 7 0x7 */
		PMURES_BIT(RES53573_CPUPLL_LDO_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_OTP_PU) | PMURES_BIT(RES53573_ALP_AVAIL) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 8 0x7 */
		PMURES_BIT(RES53573_CPU_PLL_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_CPUPLL_LDO_PU) | PMURES_BIT(RES53573_OTP_PU) |
		PMURES_BIT(RES53573_ALP_AVAIL) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 9 0x87 */
		PMURES_BIT(RES53573_WLAN_BB_PLL_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_ALP_AVAIL) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 10 0x187 */
		PMURES_BIT(RES53573_MISCPLL_LDO_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_ALP_AVAIL) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 11 0x587 */
		PMURES_BIT(RES53573_MISCPLL_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_MISCPLL_LDO_PU) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 12  0x87 */
		PMURES_BIT(RES53573_AUDIOPLL_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_ALP_AVAIL) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 13 0x187 */
		PMURES_BIT(RES53573_PCIEPLL_LDO_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_ALP_AVAIL) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 14 0x2187 */
		PMURES_BIT(RES53573_PCIEPLL_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_PCIEPLL_LDO_PU) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 15  0x187 */
		PMURES_BIT(RES53573_DDRPLL_LDO_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_ALP_AVAIL) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 16  0x8187 */
		PMURES_BIT(RES53573_DDRPLL_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_DDRPLL_LDO_PU) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 17 0x21c7  */
		PMURES_BIT(RES53573_HT_AVAIL),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_OTP_PU) |
		PMURES_BIT(RES53573_CPU_PLL_PU) | PMURES_BIT(RES53573_CPUPLL_LDO_PU) |
		PMURES_BIT(RES53573_ALP_AVAIL) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 18  */
		PMURES_BIT(RES53573_MACPHY_CLK_AVAIL),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_OTP_PU) | PMURES_BIT(RES53573_HT_AVAIL) |
		PMURES_BIT(RES53573_WLAN_BB_PLL_PU) |
		PMURES_BIT(RES53573_CPU_PLL_PU) | PMURES_BIT(RES53573_CPUPLL_LDO_PU) |
		PMURES_BIT(RES53573_ALP_AVAIL) | PMURES_BIT(RES53573_RADIO_PU) |
		PMURES_BIT(RES53573_MINI_PMU) |
		PMURES_BIT(RES53573_XTAL_PU) | PMURES_BIT(RES53573_XTALLDO_PU) |
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC 19 */
		PMURES_BIT(RES53573_OTP_PU),
		RES_DEPEND_SET,
		PMURES_BIT(RES53573_REGULATOR_PU),
		NULL
	},
	{
/* RSRC20  0x0 */
		PMURES_BIT(RES53573_RSVD20),
		RES_DEPEND_SET,
		0x00000000,
		NULL
	}
};

static const pmu_res_depend_t BCMATTACHDATA(bcm43602_res_depend)[] = {
	{
		PMURES_BIT(RES43602_SR_SUBCORE_PWRSW) | PMURES_BIT(RES43602_SR_CLK_STABLE) |
		PMURES_BIT(RES43602_SR_SAVE_RESTORE)  | PMURES_BIT(RES43602_SR_SLEEP) |
		PMURES_BIT(RES43602_LQ_START) | PMURES_BIT(RES43602_LQ_AVAIL) |
		PMURES_BIT(RES43602_WL_CORE_RDY) | PMURES_BIT(RES43602_ILP_REQ) |
		PMURES_BIT(RES43602_ALP_AVAIL) | PMURES_BIT(RES43602_RFLDO_PU) |
		PMURES_BIT(RES43602_HT_START) | PMURES_BIT(RES43602_HT_AVAIL) |
		PMURES_BIT(RES43602_MACPHY_CLKAVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_SERDES_PU),
		NULL
	},
	/* set rsrc  7, 8, 9, 12, 13, 14 & 17 add (1<<10 | 1<<4 )] */
	{
		PMURES_BIT(RES43602_SR_CLK_START) | PMURES_BIT(RES43602_SR_PHY_PWRSW) |
		PMURES_BIT(RES43602_SR_SUBCORE_PWRSW) | PMURES_BIT(RES43602_SR_CLK_STABLE) |
		PMURES_BIT(RES43602_SR_SAVE_RESTORE) | PMURES_BIT(RES43602_SR_SLEEP) |
		PMURES_BIT(RES43602_WL_CORE_RDY),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_XTALLDO_PU) | PMURES_BIT(RES43602_XTAL_PU),
		NULL
	},
	/* set rsrc 11 add (1<<13 | 1<<12 | 1<<9 | 1<<8 | 1<<7 )] */
	{
		PMURES_BIT(RES43602_PERST_OVR),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_SR_CLK_START) | PMURES_BIT(RES43602_SR_PHY_PWRSW) |
		PMURES_BIT(RES43602_SR_SUBCORE_PWRSW) | PMURES_BIT(RES43602_SR_CLK_STABLE) |
		PMURES_BIT(RES43602_SR_SAVE_RESTORE),
		NULL
	},
	/* set rsrc 19, 21, 22, 23 & 24 remove ~(1<<16 | 1<<15 )] */
	{
		PMURES_BIT(RES43602_ALP_AVAIL) | PMURES_BIT(RES43602_RFLDO_PU) |
		PMURES_BIT(RES43602_HT_START) | PMURES_BIT(RES43602_HT_AVAIL) |
		PMURES_BIT(RES43602_MACPHY_CLKAVAIL),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES43602_LQ_START) | PMURES_BIT(RES43602_LQ_AVAIL),
		NULL
	}
};

static const pmu_res_depend_t BCMATTACHDATA(bcm43602_12x12_res_depend)[] = {
	/* set rsrc 19, 21, 22, 23 & 24 remove ~(1<<16 | 1<<15 )] */
	{	/* resources no longer dependent on resource that is going to be removed */
		PMURES_BIT(RES43602_LPLDO_PU)        | PMURES_BIT(RES43602_REGULATOR)        |
		PMURES_BIT(RES43602_PMU_SLEEP)       | PMURES_BIT(RES43602_RSVD_3)           |
		PMURES_BIT(RES43602_XTALLDO_PU)      | PMURES_BIT(RES43602_SERDES_PU)        |
		PMURES_BIT(RES43602_BBPLL_PWRSW_PU)  | PMURES_BIT(RES43602_SR_CLK_START)     |
		PMURES_BIT(RES43602_SR_PHY_PWRSW)    | PMURES_BIT(RES43602_SR_SUBCORE_PWRSW) |
		PMURES_BIT(RES43602_XTAL_PU)         | PMURES_BIT(RES43602_PERST_OVR)        |
		PMURES_BIT(RES43602_SR_CLK_STABLE)   | PMURES_BIT(RES43602_SR_SAVE_RESTORE)  |
		PMURES_BIT(RES43602_SR_SLEEP)        | PMURES_BIT(RES43602_LQ_START)         |
		PMURES_BIT(RES43602_LQ_AVAIL)        | PMURES_BIT(RES43602_WL_CORE_RDY)      |
		PMURES_BIT(RES43602_ILP_REQ)         | PMURES_BIT(RES43602_ALP_AVAIL)        |
		PMURES_BIT(RES43602_RADIO_PU)        | PMURES_BIT(RES43602_RFLDO_PU)         |
		PMURES_BIT(RES43602_HT_START)        | PMURES_BIT(RES43602_HT_AVAIL)         |
		PMURES_BIT(RES43602_MACPHY_CLKAVAIL) | PMURES_BIT(RES43602_PARLDO_PU)        |
		PMURES_BIT(RES43602_RSVD_26),
		RES_DEPEND_REMOVE,
		/* resource that is going to be removed */
		PMURES_BIT(RES43602_LPLDO_PU),
		NULL
	}
};

#ifndef BCM_BOOTLOADER
static const pmu_res_depend_t BCMATTACHDATA(bcm43602_res_pciewar)[] = {
	{
		PMURES_BIT(RES43602_PERST_OVR),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_REGULATOR) |
		PMURES_BIT(RES43602_PMU_SLEEP) |
		PMURES_BIT(RES43602_XTALLDO_PU) |
		PMURES_BIT(RES43602_XTAL_PU) |
		PMURES_BIT(RES43602_RADIO_PU),
		NULL
	},
	{
		PMURES_BIT(RES43602_WL_CORE_RDY),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_PERST_OVR),
		NULL
	},
	{
		PMURES_BIT(RES43602_LQ_START),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_PERST_OVR),
		NULL
	},
	{
		PMURES_BIT(RES43602_LQ_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_PERST_OVR),
		NULL
	},
	{
		PMURES_BIT(RES43602_ALP_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_PERST_OVR),
		NULL
	},
	{
		PMURES_BIT(RES43602_HT_START),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_PERST_OVR),
		NULL
	},
	{
		PMURES_BIT(RES43602_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_PERST_OVR),
		NULL
	},
	{
		PMURES_BIT(RES43602_MACPHY_CLKAVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES43602_PERST_OVR),
		NULL
	}
};
#endif /* BCM_BOOTLOADER */

static const pmu_res_updown_t BCMATTACHDATA(bcm4360B1_res_updown)[] = {
	/* Need to change elements here, should get default values for this - 4360B1 */
	{RES4360_XTAL_PU,               0x00430002}, /* Changed for 4360B1 */
};

static const pmu_res_depend_t BCMATTACHDATA(bcm4347_res_depend)[] = {
#ifndef BCM_BOOTLOADER
	{
		PMURES_BIT(RES4347_MACPHY_AUX_CLK_AVAIL) | PMURES_BIT(RES4347_MINIPMU_AUX_PU) |
		PMURES_BIT(RES4347_RADIO_AUX_PU) | PMURES_BIT(RES4347_CORE_RDY_AUX) |
		PMURES_BIT(RES4347_SLEEP_AUX) | PMURES_BIT(RES4347_SR_AUX),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_PWRSW_MAIN) | PMURES_BIT(RES4347_PWRSW_DIG),
		NULL
	},
	{
		PMURES_BIT(RES4347_MACPHY_MAIN_CLK_AVAIL) | PMURES_BIT(RES4347_MINIPMU_MAIN_PU) |
		PMURES_BIT(RES4347_RADIO_MAIN_PU) | PMURES_BIT(RES4347_CORE_RDY_MAIN) |
		PMURES_BIT(RES4347_SLEEP_MAIN) | PMURES_BIT(RES4347_SR_MAIN),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_PWRSW_AUX) | PMURES_BIT(RES4347_PWRSW_DIG),
		NULL
	},
#else
	{0, 0, 0, NULL}
#endif /* BCM_BOOTLOADER */
};

static const pmu_res_updown_t BCMATTACHDATA(bcm4347_res_updown)[] = {
	{RES4347_XTAL_PU, 0x05dc0022},
	{RES4347_FAST_LPO_AVAIL, 0x01900022},
	{RES4347_SR_DIG, 0x01c401c4},
	{RES4347_SR_AUX, 0x01c401c4},
	{RES4347_SR_MAIN, 0x01c401c4},
	{RES4347_PWRSW_DIG, 0x00c80032},
	{RES4347_PWRSW_AUX, 0x00c80032},
	{RES4347_PWRSW_MAIN, 0x00c80032},
};

#if !defined(_CFE_) && !defined(_CFEZ_)
static pmu_res_depend_t BCMATTACHDATA(bcm4347b0_res_depend)[] = {
#ifndef BCM_BOOTLOADER
	{
		PMURES_BIT(RES4347_FAST_LPO_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_MEMLPLDO_PU),
		NULL
	},
	{
		PMURES_BIT(RES4347_LDO3P3_PU) |
		PMURES_BIT(RES4347_XTAL_PU) |
		PMURES_BIT(RES4347_XTAL_STABLE) |
		PMURES_BIT(RES4347_PWRSW_DIG) |
		PMURES_BIT(RES4347_SR_DIG) |
		PMURES_BIT(RES4347_SLEEP_DIG) |
		PMURES_BIT(RES4347_PWRSW_AUX) |
		PMURES_BIT(RES4347_SR_AUX) |
		PMURES_BIT(RES4347_SLEEP_AUX) |
		PMURES_BIT(RES4347_PWRSW_MAIN) |
		PMURES_BIT(RES4347_SR_MAIN) |
		PMURES_BIT(RES4347_SLEEP_MAIN) |
		PMURES_BIT(RES4347_CORE_RDY_DIG) |
		PMURES_BIT(RES4347_ALP_AVAIL) |
		PMURES_BIT(RES4347_PCIE_EP_PU) |
		PMURES_BIT(RES4347_COLD_START_WAIT) |
		PMURES_BIT(RES4347_ARMHTAVAIL) |
		PMURES_BIT(RES4347_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_FAST_LPO_AVAIL),
		NULL
	},
	{
		PMURES_BIT(RES4347_PCIE_EP_PU),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_XTAL_STABLE) |
		PMURES_BIT(RES4347_XTAL_PU),
		NULL
	},
	{
		PMURES_BIT(RES4347_CORE_RDY_AUX) |
		PMURES_BIT(RES4347_RADIO_AUX_PU) |
		PMURES_BIT(RES4347_MINIPMU_AUX_PU) |
		PMURES_BIT(RES4347_CORE_RDY_MAIN) |
		PMURES_BIT(RES4347_RADIO_MAIN_PU) |
		PMURES_BIT(RES4347_MINIPMU_MAIN_PU),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_FAST_LPO_AVAIL) |
		PMURES_BIT(RES4347_PWRSW_DIG) |
		PMURES_BIT(RES4347_SR_DIG) |
		PMURES_BIT(RES4347_SLEEP_DIG) |
		PMURES_BIT(RES4347_CORE_RDY_DIG),
		NULL
	},
	{
		PMURES_BIT(RES4347_MACPHY_AUX_CLK_AVAIL) |
		PMURES_BIT(RES4347_MACPHY_MAIN_CLK_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_FAST_LPO_AVAIL) |
		PMURES_BIT(RES4347_ARMHTAVAIL),
		NULL
	},
	{
		PMURES_BIT(RES4347_PMU_SLEEP),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4347_AAON),
		NULL
	},
	{
		PMURES_BIT(RES4347_PWRSW_DIG) |
		PMURES_BIT(RES4347_SR_DIG) |
		PMURES_BIT(RES4347_SLEEP_DIG) |
		PMURES_BIT(RES4347_PWRSW_AUX) |
		PMURES_BIT(RES4347_SR_AUX) |
		PMURES_BIT(RES4347_SLEEP_AUX) |
		PMURES_BIT(RES4347_PWRSW_MAIN) |
		PMURES_BIT(RES4347_SR_MAIN) |
		PMURES_BIT(RES4347_SLEEP_MAIN) |
		PMURES_BIT(RES4347_COLD_START_WAIT),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4347_PCIE_EP_PU),
		NULL
	},
	{
		PMURES_BIT(RES4347_CORE_RDY_DIG) |
		PMURES_BIT(RES4347_ALP_AVAIL) |
		PMURES_BIT(RES4347_ARMHTAVAIL) |
		PMURES_BIT(RES4347_HT_AVAIL),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4347_PWRSW_MAIN) |
		PMURES_BIT(RES4347_SR_MAIN) |
		PMURES_BIT(RES4347_SLEEP_MAIN) |
		PMURES_BIT(RES4347_CORE_RDY_MAIN) |
		PMURES_BIT(RES4347_PWRSW_AUX) |
		PMURES_BIT(RES4347_SR_AUX) |
		PMURES_BIT(RES4347_SLEEP_AUX) |
		PMURES_BIT(RES4347_CORE_RDY_AUX),
		NULL
	},
	{
		PMURES_BIT(RES4347_MACPHY_MAIN_CLK_AVAIL),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4347_PWRSW_AUX) |
		PMURES_BIT(RES4347_SR_AUX) |
		PMURES_BIT(RES4347_SLEEP_AUX) |
		PMURES_BIT(RES4347_CORE_RDY_AUX),
		NULL
	},
#ifndef BCM_FASTLPO
	/* Needed if no PCIe fast LPO */
	{
		PMURES_BIT(RES4347_PCIE_EP_PU),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_XTAL_PU) |
		PMURES_BIT(RES4347_XTAL_STABLE),
		NULL
	},
#endif // endif
	/* For closed-loop PLL support */
	{
		PMURES_BIT(RES4347_PWRSW_DIG) |
		PMURES_BIT(RES4347_PWRSW_AUX) |
		PMURES_BIT(RES4347_PWRSW_MAIN),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_XTAL_PU) |
		PMURES_BIT(RES4347_XTAL_STABLE),
		NULL
	},

	/* NOTE: This needs to be last for bcm4347b0_res_depend_war */
	{
		PMURES_BIT(RES4347_MACPHY_AUX_CLK_AVAIL),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4347_PWRSW_MAIN) |
		PMURES_BIT(RES4347_SR_MAIN) |
		PMURES_BIT(RES4347_SLEEP_MAIN) |
		PMURES_BIT(RES4347_CORE_RDY_MAIN),
		NULL
	},
#else
	{0, 0, 0, NULL}
#endif /* BCM_BOOTLOADER */
};
#endif /* !_CFE_ && !_CFEZ_ */

static const pmu_res_depend_t BCMATTACHDATA(bcm4347b0_res_depend_war)[] = {
	{
		PMURES_BIT(RES4347_RADIO_AUX_PU) |
		PMURES_BIT(RES4347_MINIPMU_AUX_PU) |
		PMURES_BIT(RES4347_MACPHY_AUX_CLK_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4347_CORE_RDY_MAIN) |
		PMURES_BIT(RES4347_PWRSW_MAIN) |
		PMURES_BIT(RES4347_SR_MAIN) |
		PMURES_BIT(RES4347_SLEEP_MAIN),
		NULL
	},
};

static const pmu_res_updown_t BCMATTACHDATA(bcm4347b0_res_updown)[] = {
	{RES4347_AAON,			0x00640044},
	{RES4347_PMU_SLEEP,		0x00f00022},
	{RES4347_FAST_LPO_AVAIL,	0x00f00022},
	{RES4347_XTAL_PU,		0x05dc0022},
	{RES4347_PWRSW_DIG,		0x00c80064},
	{RES4347_SR_DIG,		0x01860186},
	{RES4347_SLEEP_DIG,		0x00220022},
	{RES4347_PWRSW_AUX,		0x00c80064},
	{RES4347_SR_AUX,		0x01860186},
	{RES4347_PWRSW_MAIN,		0x00c80064},
	{RES4347_SR_MAIN,		0x01860186},
	{RES4347_RADIO_AUX_PU,		0x006e0022},
	{RES4347_MINIPMU_AUX_PU,	0x00460022},
	{RES4347_RADIO_MAIN_PU,		0x006e0022},
	{RES4347_MINIPMU_MAIN_PU,	0x00460022},
	{RES4347_PCIE_EP_PU,		0x006e0064},
	{RES4347_MACPHY_AUX_CLK_AVAIL,	0x00640022},
	{RES4347_MACPHY_MAIN_CLK_AVAIL,	0x00640022},
};
/** To enable avb timer clock feature */
void si_pmu_avbtimer_enable(si_t *sih, osl_t *osh, bool set_flag)
{
	uint32 min_mask = 0, max_mask = 0;
	pmuregs_t *pmu;
	uint origidx;

	/* Block ints and save current core */
	uint intr_val = si_introff(sih);
	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	if ((CHIPID(sih->chip) == BCM4360_CHIP_ID || CHIPID(sih->chip) == BCM43460_CHIP_ID) &&
		CHIPREV(sih->chiprev) >= 0x3) {
		int cst_ht = CST4360_RSRC_INIT_MODE(sih->chipst) & 0x1;
		if (cst_ht == 0) {
			/* Enable the AVB timers for proxd feature */
			min_mask = R_REG(osh, &pmu->min_res_mask);
			max_mask = R_REG(osh, &pmu->max_res_mask);
			if (set_flag) {
				max_mask |= PMURES_BIT(RES4360_AVB_PLL_PWRSW_PU);
				max_mask |= PMURES_BIT(RES4360_PCIE_TL_CLK_AVAIL);
				min_mask |= PMURES_BIT(RES4360_AVB_PLL_PWRSW_PU);
				min_mask |= PMURES_BIT(RES4360_PCIE_TL_CLK_AVAIL);
				W_REG(osh, &pmu->min_res_mask, min_mask);
				W_REG(osh, &pmu->max_res_mask, max_mask);
			} else {
				AND_REG(osh, &pmu->min_res_mask,
					~PMURES_BIT(RES4360_AVB_PLL_PWRSW_PU));
				AND_REG(osh, &pmu->min_res_mask,
					~PMURES_BIT(RES4360_PCIE_TL_CLK_AVAIL));
				AND_REG(osh, &pmu->max_res_mask,
					~PMURES_BIT(RES4360_AVB_PLL_PWRSW_PU));
				AND_REG(osh, &pmu->max_res_mask,
					~PMURES_BIT(RES4360_PCIE_TL_CLK_AVAIL));
			}
			/* Need to wait 100 millisecond for the uptime */
			OSL_DELAY(100);
		}
	}

	if (((CHIPID(sih->chip) == BCM4366_CHIP_ID) &&
		CHIPREV(sih->chiprev) >= 0x4) ||
		CHIPID(sih->chip) == BCM43684_CHIP_ID) {
		/*
		For Rev C0 4365 (chiprev 0x4) and 43684 chips, bits 13:12 of chipcontrol_data
		must be set to a value other than 0.  0 means AVB clock disabled, 01 means AVB
		clock uses GPIOs for input, and 10 means AVB clock uses GPIOs for output
		*/

		if (set_flag) {
			pmu_corereg(sih, SI_CC_IDX, chipcontrol_addr,
				0xffffffff, 0);
			pmu_corereg(sih, SI_CC_IDX, chipcontrol_data,
				0x3000, 0x2000);
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
	case BCM4360_CHIP_ID:
	case BCM4352_CHIP_ID:
		if (CHIPREV(sih->chiprev) >= 0x4) {
			min_mask = 0x103;
		}
		/* Continue - Don't break */
	case BCM43460_CHIP_ID:
	case BCM43526_CHIP_ID:
		if (CHIPREV(sih->chiprev) >= 0x3) {
			int cst_ht = CST4360_RSRC_INIT_MODE(sih->chipst) & 0x1;
			if (cst_ht == 0)
				max_mask = 0x1ff;
		}
		break;
	CASE_BCM43602_CHIP:
		/* as a bare minimum, have ALP clock running */
		min_mask = PMURES_BIT(RES43602_LPLDO_PU)  | PMURES_BIT(RES43602_REGULATOR)      |
			PMURES_BIT(RES43602_PMU_SLEEP)    | PMURES_BIT(RES43602_XTALLDO_PU)     |
			PMURES_BIT(RES43602_SERDES_PU)    | PMURES_BIT(RES43602_BBPLL_PWRSW_PU) |
			PMURES_BIT(RES43602_SR_CLK_START) | PMURES_BIT(RES43602_SR_PHY_PWRSW)   |
			PMURES_BIT(RES43602_SR_SUBCORE_PWRSW) | PMURES_BIT(RES43602_XTAL_PU)    |
			PMURES_BIT(RES43602_PERST_OVR)    | PMURES_BIT(RES43602_SR_CLK_STABLE)  |
			PMURES_BIT(RES43602_SR_SAVE_RESTORE) | PMURES_BIT(RES43602_SR_SLEEP)    |
			PMURES_BIT(RES43602_LQ_START)     | PMURES_BIT(RES43602_LQ_AVAIL)       |
			PMURES_BIT(RES43602_WL_CORE_RDY)  |
			PMURES_BIT(RES43602_ALP_AVAIL);

		if (sih->chippkg == BCM43602_12x12_PKG_ID) /* LPLDO WAR */
			min_mask &= ~PMURES_BIT(RES43602_LPLDO_PU);

		max_mask = (1<<3) | min_mask          | PMURES_BIT(RES43602_RADIO_PU)        |
			PMURES_BIT(RES43602_RFLDO_PU) | PMURES_BIT(RES43602_HT_START)        |
			PMURES_BIT(RES43602_HT_AVAIL) | PMURES_BIT(RES43602_MACPHY_CLKAVAIL);
		break;
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		/* as a bare minimum, have ALP clock running */
		min_mask = PMURES_BIT(RES4365_REGULATOR_PU) | PMURES_BIT(RES4365_XTALLDO_PU) |
			PMURES_BIT(RES4365_XTAL_PU) | PMURES_BIT(RES4365_CPU_PLLLDO_PU) |
			PMURES_BIT(RES4365_CPU_PLL_PU) | PMURES_BIT(RES4365_WL_CORE_RDY) |
			PMURES_BIT(RES4365_ALP_AVAIL) | PMURES_BIT(RES4365_HT_AVAIL);
		max_mask = min_mask |
			PMURES_BIT(RES4365_BB_PLLLDO_PU) | PMURES_BIT(RES4365_BB_PLL_PU) |
			PMURES_BIT(RES4365_MINIMU_PU) | PMURES_BIT(RES4365_RADIO_PU) |
			PMURES_BIT(RES4365_MACPHY_CLK_AVAIL);
		break;
	case BCM53573_CHIP_GRPID:
		max_mask = 0x1FFFFF;
		/* For now min = max  - later to be changed as per needed */
		min_mask = max_mask;
		break;
	case BCM43570_CHIP_ID:
#ifndef BCM_BOOTLOADER
		if (CST4350_IFC_MODE(sih->chipst) == CST4350_IFC_MODE_PCIE) {
			int L1substate = si_pcie_get_L1substate(sih);
			if (L1substate & 1)	/* L1.2 is enabled */
				min_mask = PMURES_BIT(RES4350_LPLDO_PU) |
					PMURES_BIT(RES4350_PMU_BG_PU) |
					PMURES_BIT(RES4350_PMU_SLEEP);
			else			/* use chip default min resource mask */
				min_mask = 0xfc22f77;
		} else {
#endif /* BCM_BOOTLOADER */

			/* use chip default min resource mask */
			min_mask = pmu_corereg(sih, SI_CC_IDX,
				min_res_mask, 0, 0);
#ifndef BCM_BOOTLOADER
		}
#endif /* BCM_BOOTLOADER */

		/* Set the bits for all resources in the max mask except for the SR Engine */
		max_mask = 0x7FFFFFFF;
		break;
	CASE_BCM43684_CHIP:
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
	CASE_BCM6710_CHIP:
	CASE_BCM6715_CHIP:
		min_mask = pmu_corereg(sih, SI_CC_IDX, min_res_mask, 0, 0);
		max_mask = 0x7FFFFFFF;
		break;
	case BCM4347_CHIP_GRPID:
		min_mask = 0x64fffff;
#if defined(SAVERESTORE)
		if (SR_ENAB() && sr_isenab(sih)) {
			min_mask = PMURES_BIT(RES4347_MEMLPLDO_PU);
		}
#endif /* SAVERESTORE */
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

#if !defined(_CFE_) && !defined(_CFEZ_)
/**
 * resource dependencies can change because of the host interface
 * selected, to work around an issue, or for more optimal power
 * savings after tape out
 */
#ifdef DUAL_PMU_SEQUENCE
static void
si_pmu_resdeptbl_upd(si_t *sih, osl_t *osh, pmuregs_t *pmu,
	const pmu_res_depend_t *restable, uint tablesz)
#else
static void
BCMATTACHFN(si_pmu_resdeptbl_upd)(si_t *sih, osl_t *osh, pmuregs_t *pmu,
	const pmu_res_depend_t *restable, uint tablesz)
#endif /* DUAL_PMU_SEQUENCE */
{
	uint i, rsrcs;

	if (tablesz == 0)
		return;

	ASSERT(restable != NULL);

	rsrcs = (sih->pmucaps & PCAP_RC_MASK) >> PCAP_RC_SHIFT;
	/* Program resource dependencies table */
	while (tablesz--) {
		if (restable[tablesz].filter != NULL &&
		    !(restable[tablesz].filter)(sih))
			continue;
		for (i = 0; i < rsrcs; i ++) {
			if ((restable[tablesz].res_mask &
			     PMURES_BIT(i)) == 0)
				continue;
			W_REG(osh, &pmu->res_table_sel, i);
			switch (restable[tablesz].action) {
				case RES_DEPEND_SET:
					PMU_MSG(("Changing rsrc %d res_dep_mask to 0x%x\n", i,
						restable[tablesz].depend_mask));
					W_REG(osh, &pmu->res_dep_mask,
					      restable[tablesz].depend_mask);
					break;
				case RES_DEPEND_ADD:
					PMU_MSG(("Adding 0x%x to rsrc %d res_dep_mask\n",
						restable[tablesz].depend_mask, i));
					OR_REG(osh, &pmu->res_dep_mask,
					       restable[tablesz].depend_mask);
					break;
				case RES_DEPEND_REMOVE:
					PMU_MSG(("Removing 0x%x from rsrc %d res_dep_mask\n",
						restable[tablesz].depend_mask, i));
					AND_REG(osh, &pmu->res_dep_mask,
						~restable[tablesz].depend_mask);
					break;
				default:
					ASSERT(0);
					break;
			}
		}
	}
} /* si_pmu_resdeptbl_upd */
#endif /* !defined(_CFE_) && !defined(_CFEZ_) */

/** Initialize PMU hardware resources. */
void
BCMATTACHFN(si_pmu_res_init)(si_t *sih, osl_t *osh)
{
#if !defined(_CFE_) && !defined(_CFEZ_)
	pmuregs_t *pmu;
	uint origidx;
	const pmu_res_updown_t *pmu_res_updown_table = NULL;
	uint pmu_res_updown_table_sz = 0;
	const pmu_res_depend_t *pmu_res_depend_table = NULL;
	uint pmu_res_depend_table_sz = 0;
#ifndef BCM_BOOTLOADER
	const pmu_res_depend_t *pmu_res_depend_pciewar_table[2] = {NULL, NULL};
	uint pmu_res_depend_pciewar_table_sz[2] = {0, 0};
#endif /* BCM_BOOTLOADER */
	uint32 min_mask = 0, max_mask = 0;
	char name[8];
	const char *val;
	uint i, rsrcs;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	/*
	 * Hardware contains the resource updown and dependency tables. Only if a chip has a
	 * hardware problem, software tables can be used to override hardware tables.
	 */
	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
		pmu_res_updown_table = bcm4350_res_updown;
		pmu_res_updown_table_sz = ARRAYSIZE(bcm4350_res_updown);
#ifndef BCM_BOOTLOADER
		pmu_res_depend_pciewar_table[0] = bcm4350_res_pciewar;
		pmu_res_depend_pciewar_table_sz[0] = ARRAYSIZE(bcm4350_res_pciewar);
#endif /* BCM_BOOTLOADER */
		break;
	case BCM53573_CHIP_GRPID:
		si_pmu_chipcontrol(sih, PMU_53573_CHIPCTL3, PMU_53573_CC3_ENABLE_CLOSED_LOOP_MASK,
		PMU_53573_CC3_ENABLE_CLOSED_LOOP);

		pmu_res_updown_table = bcm53573_res_updown;
		pmu_res_updown_table_sz = ARRAYSIZE(bcm53573_res_updown);
		pmu_res_depend_table = bcm53573a0_res_depend;
		pmu_res_depend_table_sz = ARRAYSIZE(bcm53573a0_res_depend);
		break;
	case BCM4360_CHIP_ID:
	case BCM4352_CHIP_ID:
		if (CHIPREV(sih->chiprev) < 4) {
			pmu_res_updown_table = bcm4360_res_updown;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4360_res_updown);
		} else {
			/* FOR 4360B1 */
			pmu_res_updown_table = bcm4360B1_res_updown;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4360B1_res_updown);
		}
		break;
	CASE_BCM43602_CHIP:
		pmu_res_updown_table = bcm43602_res_updown;
		pmu_res_updown_table_sz = ARRAYSIZE(bcm43602_res_updown);
		pmu_res_depend_table = bcm43602_res_depend;
		pmu_res_depend_table_sz = ARRAYSIZE(bcm43602_res_depend);
#ifndef BCM_BOOTLOADER
		pmu_res_depend_pciewar_table[0] = bcm43602_res_pciewar;
		pmu_res_depend_pciewar_table_sz[0] = ARRAYSIZE(bcm43602_res_pciewar);
		if (sih->chippkg == BCM43602_12x12_PKG_ID) { /* LPLDO WAR */
			pmu_res_depend_pciewar_table[1] = bcm43602_12x12_res_depend;
			pmu_res_depend_pciewar_table_sz[1] = ARRAYSIZE(bcm43602_12x12_res_depend);
		}
#endif /* !BCM_BOOTLOADER */
		break;
	CASE_BCM43684_CHIP:
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
	CASE_BCM6710_CHIP:
	CASE_BCM6715_CHIP:
		break;
	case BCM4347_CHIP_GRPID:
		if (CHIPREV(sih->chiprev) < 3) {
			pmu_res_updown_table = bcm4347_res_updown;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4347_res_updown);
			pmu_res_depend_table = bcm4347_res_depend;
			pmu_res_depend_table_sz = ARRAYSIZE(bcm4347_res_depend);
		} else {
			pmu_res_updown_table = bcm4347b0_res_updown;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4347b0_res_updown);
			pmu_res_depend_table = bcm4347b0_res_depend;
			pmu_res_depend_table_sz = ARRAYSIZE(bcm4347b0_res_depend);

			if (CHIPREV(sih->chiprev) == 3) {	/* Apply WAR for HW4347-913 */
				memcpy(&bcm4347b0_res_depend[pmu_res_depend_table_sz - 1],
					&bcm4347b0_res_depend_war[0],
					sizeof(bcm4347b0_res_depend_war));
			}
		}
		break;
	default:
		break;
	}

	/* # resources */
	rsrcs = (sih->pmucaps & PCAP_RC_MASK) >> PCAP_RC_SHIFT;

	/* Program up/down timers */
	while (pmu_res_updown_table_sz--) {
		ASSERT(pmu_res_updown_table != NULL);
		PMU_MSG(("Changing rsrc %d res_updn_timer to 0x%x\n",
		         pmu_res_updown_table[pmu_res_updown_table_sz].resnum,
		         pmu_res_updown_table[pmu_res_updown_table_sz].updown));
		W_REG(osh, &pmu->res_table_sel,
		      (uint32)pmu_res_updown_table[pmu_res_updown_table_sz].resnum);
		W_REG(osh, &pmu->res_updn_timer,
		      pmu_res_updown_table[pmu_res_updown_table_sz].updown);
	}

	/* Apply nvram overrides to up/down timers */
	for (i = 0; i < rsrcs; i ++) {
		uint32 r_val;
		snprintf(name, sizeof(name), rstr_rDt, i);
		if ((val = getvar(NULL, name)) == NULL)
			continue;
		r_val = (uint32)bcm_strtoul(val, NULL, 0);
		/* PMUrev = 13, pmu resource updown times are 12 bits(0:11 DT, 16:27 UT) */
		if (PMUREV(sih->pmurev) >= 13) {
			if (r_val < (1 << 16)) {
				uint16 up_time = (r_val >> 8) & 0xFF;
				r_val &= 0xFF;
				r_val |= (up_time << 16);
			}
		}
		PMU_MSG(("Applying %s=%s to rsrc %d res_updn_timer\n", name, val, i));
		W_REG(osh, &pmu->res_table_sel, (uint32)i);
		W_REG(osh, &pmu->res_updn_timer, r_val);
	}

	/* Program resource dependencies table */
	si_pmu_resdeptbl_upd(sih, osh, pmu, pmu_res_depend_table, pmu_res_depend_table_sz);

	/* Apply nvram overrides to dependencies masks */
	for (i = 0; i < rsrcs; i ++) {
		snprintf(name, sizeof(name), rstr_rDd, i);
		if ((val = getvar(NULL, name)) == NULL)
			continue;
		PMU_MSG(("Applying %s=%s to rsrc %d res_dep_mask\n", name, val, i));
		W_REG(osh, &pmu->res_table_sel, (uint32)i);
		W_REG(osh, &pmu->res_dep_mask, (uint32)bcm_strtoul(val, NULL, 0));
	}

#if !defined(BCM_BOOTLOADER)
	/* Initial any chip interface dependent PMU rsrc by looking at the
	 * chipstatus register to figure the selected interface
	 */
	/* XXX: this should be a general change to cover all the chips.
	 * this also should validate the build where the dongle is
	 * built for SDIO but downloaded on PCIE dev
	 */
	if (BUSTYPE(sih->bustype) == PCI_BUS || BUSTYPE(sih->bustype) == SI_BUS) {
		bool is_pciedev = FALSE;

		if (BCM4350_CHIP(sih->chip) && CST4350_CHIPMODE_PCIE(sih->chipst))
			is_pciedev = TRUE;
		else if (BCM43602_CHIP(sih->chip))
			is_pciedev = TRUE;

		for (i = 0; i < ARRAYSIZE(pmu_res_depend_pciewar_table); i++) {
			if (is_pciedev && pmu_res_depend_pciewar_table[i] &&
			    pmu_res_depend_pciewar_table_sz[i]) {
				si_pmu_resdeptbl_upd(sih, osh, pmu,
					pmu_res_depend_pciewar_table[i],
					pmu_res_depend_pciewar_table_sz[i]);
			}
		}
	}
#endif /* !BCM_BOOTLOADER */
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

	/* XXX apply new PLL setting if is ALP strap (need to close out
	 * if possible apply if is HT strap)
	 */
	if (((CHIPID(sih->chip) == BCM4360_CHIP_ID) || (CHIPID(sih->chip) == BCM4352_CHIP_ID)) &&
	    (CHIPREV(sih->chiprev) < 4) &&
	    ((CST4360_RSRC_INIT_MODE(sih->chipst) & 1) == 0)) {
		/* BBPLL */
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6, ~0, 0x09048562);
		/* AVB PLL */
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG14, ~0, 0x09048562);
		si_pmu_pllupd(sih);
	} else if (((CHIPID(sih->chip) == BCM4360_CHIP_ID) ||
		(CHIPID(sih->chip) == BCM4352_CHIP_ID)) &&
		(CHIPREV(sih->chiprev) >= 4) &&
		((CST4360_RSRC_INIT_MODE(sih->chipst) & 1) == 0)) {
		/* Changes for 4360B1 */

		/* Enable REFCLK bit 11 */
		si_pmu_chipcontrol(sih, PMU_CHIPCTL1, 0x800, 0x800);

		/* BBPLL */
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG6, ~0, 0x080004e2);
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, ~0, 0xE);
		/* AVB PLL */
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG14, ~0, 0x080004e2);
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG15, ~0, 0xE);
		si_pmu_pllupd(sih);
	}
	/* disable PLL open loop operation */
	si_pll_closeloop(sih);

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
#if defined(SAVERESTORE) && defined(LDO3P3_MIN_RES_MASK)
	if (SR_ENAB()) {
		/* Set the default state for LDO3P3 protection */
		if (getintvar(NULL, rstr_ldo_prot) == 1) {
			si_pmu_min_res_ldo3p3_set(wlc_hw->sih, wlc_hw->osh, TRUE);
		}
	}
#endif /* SAVERESTORE && LDO3P3_MIN_RES_MASK */

	/* request htavail thru pcie core */
	if (((CHIPID(sih->chip) == BCM4360_CHIP_ID) || (CHIPID(sih->chip) == BCM4352_CHIP_ID)) &&
	    (BUSTYPE(sih->bustype) == PCI_BUS) &&
	    (CHIPREV(sih->chiprev) < 4)) {
		uint32 pcie_clk_ctl_st;

		pcie_clk_ctl_st = si_corereg(sih, 3, 0x1e0, 0, 0);
		si_corereg(sih, 3, 0x1e0, ~0, (pcie_clk_ctl_st | CCS_HTAREQ));
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
	uint16	fref;	/* x-tal frequency in [hz] */
	uint8	xf;	/* x-tal index as contained in PMU control reg, see PMU programmers guide */
	uint8	p1div;
	uint8	p2div;
	uint8	ndiv_int;
	uint32	ndiv_frac;
} pmu1_xtaltab0_t;

/* 'xf' values corresponding to the 'xf' definition in the PMU control register */
enum xtaltab0_640 {
	XTALTAB0_640_12000K = 1,
	XTALTAB0_640_13000K,
	XTALTAB0_640_14400K,
	XTALTAB0_640_15360K,
	XTALTAB0_640_16200K,
	XTALTAB0_640_16800K,
	XTALTAB0_640_19200K,
	XTALTAB0_640_19800K,
	XTALTAB0_640_20000K,
	XTALTAB0_640_24000K,
	XTALTAB0_640_25000K,
	XTALTAB0_640_26000K,
	XTALTAB0_640_30000K,
	XTALTAB0_640_33600K,
	XTALTAB0_640_37400K,
	XTALTAB0_640_38400K,
	XTALTAB0_640_40000K,
	XTALTAB0_640_48000K,
	XTALTAB0_640_52000K
};

/**
 * given an x-tal frequency, this table specifies the PLL params to use to generate a 640Mhz output
 * clock. This output clock feeds the clock divider network. The defines of the form
 * PMU1_XTALTAB0_640_* index into this array.
 */
static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_640)[] = {
/*	fref      xf     p1div   p2div  ndiv_int  ndiv_frac */
	{12000,   1,       1,      1,     0x35,   0x555555}, /* array index 0 */
	{13000,   2,       1,      1,     0x31,   0x3B13B1},
	{14400,   3,       1,      1,     0x2C,   0x71C71C},
	{15360,   4,       1,      1,     0x29,   0xAAAAAA},
	{16200,   5,       1,      1,     0x27,   0x81948B},
	{16800,   6,       1,      1,     0x26,   0x186186},
	{19200,   7,       1,      1,     0x21,   0x555555},
	{19800,   8,       1,      1,     0x20,   0x5ABF5A},
	{20000,   9,       1,      1,     0x20,   0x000000},
	{24000,   10,      1,      1,     0x1A,   0xAAAAAA},
	{25000,   11,      1,      1,     0x19,   0x999999}, /* array index 10 */
	{26000,   12,      1,      1,     0x18,   0x9D89D8},
	{30000,   13,      1,      1,     0x15,   0x555555},
	{33600,   14,      1,      1,     0x13,   0x0C30C3},
	{37400,   15,      1,      1,     0x11,   0x1CBFA8},
	{38400,   16,      1,      1,     0x10,   0xAAAAAA},
	{40000,   17,      1,      1,     0x10,   0x000000},
	{48000,   18,      1,      1,     0x0D,   0x555555},
	{52000,   19,      1,      1,     0x0C,   0x4EC4EC}, /* array index 18 */
	{0,	      0,       0,      0,     0,      0	      }
};

/* Indices into array pmu1_xtaltab0_640[]. Keep array and these defines synchronized. */
#define PMU1_XTALTAB0_640_12000K	0
#define PMU1_XTALTAB0_640_13000K	1
#define PMU1_XTALTAB0_640_14400K	2
#define PMU1_XTALTAB0_640_15360K	3
#define PMU1_XTALTAB0_640_16200K	4
#define PMU1_XTALTAB0_640_16800K	5
#define PMU1_XTALTAB0_640_19200K	6
#define PMU1_XTALTAB0_640_19800K	7
#define PMU1_XTALTAB0_640_20000K	8
#define PMU1_XTALTAB0_640_24000K	9
#define PMU1_XTALTAB0_640_25000K	10
#define PMU1_XTALTAB0_640_26000K	11
#define PMU1_XTALTAB0_640_30000K	12
#define PMU1_XTALTAB0_640_33600K	13
#define PMU1_XTALTAB0_640_37400K	14
#define PMU1_XTALTAB0_640_38400K	15
#define PMU1_XTALTAB0_640_40000K	16
#define PMU1_XTALTAB0_640_48000K	17
#define PMU1_XTALTAB0_640_52000K	18

/* the following table is based on 880Mhz fvco */
static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_880)[] = {
	{12000,	1,	3,	22,	0x9,	0xFFFFEF},
	{13000,	2,	1,	6,	0xb,	0x483483},
	{14400,	3,	1,	10,	0xa,	0x1C71C7},
	{15360,	4,	1,	5,	0xb,	0x755555},
	{16200,	5,	1,	10,	0x5,	0x6E9E06},
	{16800,	6,	1,	10,	0x5,	0x3Cf3Cf},
	{19200,	7,	1,	4,	0xb,	0x755555},
	{19800,	8,	1,	11,	0x4,	0xA57EB},
	{20000,	9,	1,	11,	0x4,	0x0},
	{24000,	10,	3,	11,	0xa,	0x0},
	{25000,	11,	5,	16,	0xb,	0x0},
	{26000,	12,	1,	2,	0x10,	0xEC4EC4},
	{30000,	13,	3,	8,	0xb,	0x0},
	{33600,	14,	1,	2,	0xd,	0x186186},
	{38400,	15,	1,	2,	0xb,	0x755555},
	{40000,	16,	1,	2,	0xb,	0},
	{0,	0,	0,	0,	0,	0}
};

/* indices into pmu1_xtaltab0_880[] */
#define PMU1_XTALTAB0_880_12000K	0
#define PMU1_XTALTAB0_880_13000K	1
#define PMU1_XTALTAB0_880_14400K	2
#define PMU1_XTALTAB0_880_15360K	3
#define PMU1_XTALTAB0_880_16200K	4
#define PMU1_XTALTAB0_880_16800K	5
#define PMU1_XTALTAB0_880_19200K	6
#define PMU1_XTALTAB0_880_19800K	7
#define PMU1_XTALTAB0_880_20000K	8
#define PMU1_XTALTAB0_880_24000K	9
#define PMU1_XTALTAB0_880_25000K	10
#define PMU1_XTALTAB0_880_26000K	11
#define PMU1_XTALTAB0_880_30000K	12
#define PMU1_XTALTAB0_880_37400K	13
#define PMU1_XTALTAB0_880_38400K	14
#define PMU1_XTALTAB0_880_40000K	15

/* the following table is based on 1760Mhz fvco */
static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_1760)[] = {
	{12000,	1,	3,	44,	0x9,	0xFFFFEF},
	{13000,	2,	1,	12,	0xb,	0x483483},
	{14400,	3,	1,	20,	0xa,	0x1C71C7},
	{15360,	4,	1,	10,	0xb,	0x755555},
	{16200,	5,	1,	20,	0x5,	0x6E9E06},
	{16800,	6,	1,	20,	0x5,	0x3Cf3Cf},
	{19200,	7,	1,	18,	0x5,	0x17B425},
	{19800,	8,	1,	22,	0x4,	0xA57EB},
	{20000,	9,	1,	22,	0x4,	0x0},
	{24000,	10,	3,	22,	0xa,	0x0},
	{25000,	11,	5,	32,	0xb,	0x0},
	{26000,	12,	1,	4,	0x10,	0xEC4EC4},
	{30000,	13,	3,	16,	0xb,	0x0},
	{38400,	14,	1,	10,	0x4,	0x955555},
	{40000,	15,	1,	4,	0xb,	0},
	{0,	0,	0,	0,	0,	0}
};

#define XTAL_FREQ_24000MHZ		24000
#define XTAL_FREQ_30000MHZ		30000
#define XTAL_FREQ_37400MHZ		37400
#define XTAL_FREQ_48000MHZ		48000

/* 'xf' values corresponding to the 'xf' definition in the PMU control register */
enum xtaltab0_960 {
	XTALTAB0_960_12000K = 1,
	XTALTAB0_960_13000K,
	XTALTAB0_960_14400K,
	XTALTAB0_960_15360K,
	XTALTAB0_960_16200K,
	XTALTAB0_960_16800K,
	XTALTAB0_960_19200K,
	XTALTAB0_960_19800K,
	XTALTAB0_960_20000K,
	XTALTAB0_960_24000K,
	XTALTAB0_960_25000K,
	XTALTAB0_960_26000K,
	XTALTAB0_960_30000K,
	XTALTAB0_960_33600K,
	XTALTAB0_960_37400K,
	XTALTAB0_960_38400K,
	XTALTAB0_960_40000K,
	XTALTAB0_960_48000K,
	XTALTAB0_960_52000K
};

/**
 * given an x-tal frequency, this table specifies the PLL params to use to generate a 960Mhz output
 * clock. This output clock feeds the clock divider network. The defines of the form
 * PMU1_XTALTAB0_960_* index into this array.
 */
static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_960)[] = {
/*	fref      xf     p1div   p2div  ndiv_int  ndiv_frac */
	{12000,   1,       1,      1,     0x50,   0x0     }, /* array index 0 */
	{13000,   2,       1,      1,     0x49,   0xD89D89},
	{14400,   3,       1,      1,     0x42,   0xAAAAAA},
	{15360,   4,       1,      1,     0x3E,   0x800000},
	{16200,   5,       1,      1,     0x3B,   0x425ED0},
	{16800,   6,       1,      1,     0x39,   0x249249},
	{19200,   7,       1,      1,     0x32,   0x0     },
	{19800,   8,       1,      1,     0x30,   0x7C1F07},
	{20000,   9,       1,      1,     0x30,   0x0     },
	{24000,   10,      1,      1,     0x28,   0x0     },
	{25000,   11,      1,      1,     0x26,   0x666666}, /* array index 10 */
	{26000,   12,      1,      1,     0x24,   0xEC4EC4},
	{30000,   13,      1,      1,     0x20,   0x0     },
	{33600,   14,      1,      1,     0x1C,   0x924924},
	{37400,   15,      2,      1,     0x33,   0x563EF9},
	{38400,   16,      2,      1,     0x32,   0x0	  },
	{40000,   17,      2,      1,     0x30,   0x0     },
	{48000,   18,      2,      1,     0x28,   0x0     },
	{52000,   19,      2,      1,     0x24,   0xEC4EC4}, /* array index 18 */
	{0,	      0,       0,      0,     0,      0	      }
};

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_1600)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac */
	{40000,   0,       1,      1,     0x28,   0x0     },
	{0,	  0,       0,      0,     0,      0	  }
};

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

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_bcm6715_5760)[] = {
/*	fref      xf       p1div   p2div  ndiv_int  ndiv_frac (use ndiv_p, ndiv_q for 6715) */
	{50000,   0,       1,      1,     0x72,     0x0 }, /**< 50 Mhz xtal */
	{0,	  0,       0,      0,     0,        0	    }
};

static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_1938)[] = {
	/*	fref      xf         p1div        p2div    ndiv_int  ndiv_frac */
	{XTAL_FREQ_54MHZ,   0,       1,      1,     0x28,   0x0     },
	{0,	  0,       0,      0,     0,      0	  }
};
/* Indices into array pmu1_xtaltab0_960[]. Keep array and these defines synchronized. */
#define PMU1_XTALTAB0_960_12000K	0
#define PMU1_XTALTAB0_960_13000K	1
#define PMU1_XTALTAB0_960_14400K	2
#define PMU1_XTALTAB0_960_15360K	3
#define PMU1_XTALTAB0_960_16200K	4
#define PMU1_XTALTAB0_960_16800K	5
#define PMU1_XTALTAB0_960_19200K	6
#define PMU1_XTALTAB0_960_19800K	7
#define PMU1_XTALTAB0_960_20000K	8
#define PMU1_XTALTAB0_960_24000K	9
#define PMU1_XTALTAB0_960_25000K	10
#define PMU1_XTALTAB0_960_26000K	11
#define PMU1_XTALTAB0_960_30000K	12
#define PMU1_XTALTAB0_960_33600K	13
#define PMU1_XTALTAB0_960_37400K	14
#define PMU1_XTALTAB0_960_38400K	15
#define PMU1_XTALTAB0_960_40000K	16
#define PMU1_XTALTAB0_960_48000K	17
#define PMU1_XTALTAB0_960_52000K	18

#define PMU15_XTALTAB0_12000K	0
#define PMU15_XTALTAB0_20000K	1
#define PMU15_XTALTAB0_26000K	2
#define PMU15_XTALTAB0_37400K	3
#define PMU15_XTALTAB0_52000K	4
#define PMU15_XTALTAB0_END	5

/* For having the pllcontrol data (info)
 * The table with the values of the registers will have one - one mapping.
 */
typedef struct {
	uint16	clock;	/**< x-tal frequency in [KHz] */
	uint8	mode;	/**< spur mode */
	uint8	xf;	/**< corresponds with xf bitfield in PMU control register */
} pllctrl_data_t;

/*  *****************************  tables for 4350a0 *********************** */

#define XTAL_DEFAULT_4350	37400
/**
 * PLL control register table giving info about the xtal supported for 4350
 * There should be a one to one mapping between pmu1_pllctrl_tab_4350_963mhz[] and this table.
 */
static const pllctrl_data_t pmu1_xtaltab0_4350[] = {
/*       clock  mode xf */
	{37400, 0,   XTALTAB0_960_37400K},
	{40000, 0,   XTALTAB0_960_40000K},
};

/**
 * PLL control register table giving info about the xtal supported for 4335.
 * There should be a one to one mapping between pmu1_pllctrl_tab_4335_963mhz[] and this table.
 */
static const uint32	BCMATTACHDATA(pmu1_pllctrl_tab_4350_963mhz)[] = {
/*	PLL 0       PLL 1       PLL 2       PLL 3       PLL 4       PLL 5       PLL6         */
	0x50800000, 0x18060603, 0x0cb10814, 0x80bfaa00, 0x02600004, 0x00019AB1, 0x04a6c181,
	0x50800000, 0x18060603, 0x0C310814, 0x00133333, 0x02600004, 0x00017FFF, 0x04a6c181
};
static const uint32	BCMATTACHDATA(pmu1_pllctrl_tab_4350C0_963mhz)[] = {
/*	PLL 0       PLL 1       PLL 2       PLL 3       PLL 4       PLL 5       PLL6         */
	0x50800000, 0x08060603, 0x0cb10804, 0xe2bfaa00, 0x02600004, 0x00019AB1, 0x02a6c181,
	0x50800000, 0x08060603, 0x0C310804, 0xe2133333, 0x02600004, 0x00017FFF, 0x02a6c181
};
/*  ************************  tables for 4350a0 END *********************** */

/*  *****************************  tables for 4347a0 *********************** */
/**
 * PLL control register table giving info about the xtal supported for 4347
 * There should be a one to one mapping between pmu1_pllctrl_tab_4347_960p5mhz[] and this table.
 */
static const pllctrl_data_t BCMATTACHDATA(pmu1_xtaltab0_4347)[] = {
/*       clock  mode xf */
	{37400, 0,   XTALTAB0_960_37400K}
};

/**
 * PLL control register table giving info about the xtal supported for 4347.
 * There should be a one to one mapping between pmu1_pllctrl_tab_4347_960p5mhz[] and this table.
 * PLL control5 register is related to HSIC which is not supported in 4347
 */
/**
 * TBD : will update once all the control data is provided by system team
 * freq table : pll1 : fvco 963M, pll2 for arm : 385 MHz
 */
static const uint32	BCMATTACHDATA(pmu1_pllctrl_tab_4347_960p5mhz)[] = {
/* Default values for unused registers 4-7 as sw loop execution will go for 8 times */
/* Fvco is taken as 960.5M */
/*	PLL 0  PLL 1   PLL 2   PLL 3   PLL 4   PLL 5  PLL 6  PLL 7   PLL 8   PLL 9   PLL 10 */
	0x32800000, 0x06060603, 0x0191080C, 0x006AE8BA,
	0x00000800, 0x32800000, 0x2D2D20A5, 0x40800000,
	0x00000000, 0x00000000, 0x00000000
};
/*  ************************  tables for 4347a0 END *********************** */

/*  *****************************  tables for 53573 *********************** */

#define XTAL_DEFAULT_53573    40000
/**
 * PLL control register table giving info about the xtal supported for 53573
 * There should be a one to one mapping between pmu1_pllctrl_tab_53573_640mhz[] and this table.
 */
static const pllctrl_data_t pmu1_xtaltab0_53573[] = {
/*       clock  mode xf */
	{40000, 0,   XTALTAB0_640_40000K}
};

/**
 * PLL control register table giving info about the xtal supported for 4349.
 * There should be a one to one mapping between pmu1_pllctrl_tab_4349_640mhz[] and this table.
 * PLL control5 register is related to HSIC which is not supported in 4349
 */
static const uint32	pmu1_pllctrl_tab_53573_640mhz[] = {
/* Default values for unused registers 5-7 as sw loop execution will go for 8 times */
/* Fvco is taken as 640.1 instead of 640 as per recommendation from chip team */
/*	PLL 0       PLL 1       PLL 2       PLL 3     PLL 4       PLL 5       PLL 6        PLL 7 */
	0x20800000, 0x04020402, 0x8B10608, 0xE1000000,
	0x01600004, 0x00000000, 0x00000000, 0x00000000
};
/*  ************************  tables for 53573 END *********************** */

/** returns a table that instructs how to program the BBPLL for a particular xtal frequency */
static const pmu1_xtaltab0_t *
BCMINITFN(si_pmu1_xtaltab0)(si_t *sih)
{
#ifdef BCMDBG
	char chn[8];
#endif // endif
	switch (CHIPID(sih->chip)) {
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID:
	CASE_BCM43602_CHIP:
	case BCM43570_CHIP_ID:
	case BCM4347_CHIP_GRPID:
		return pmu1_xtaltab0_960;
	case BCM53573_CHIP_GRPID:
		return pmu1_xtaltab0_640;
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		return pmu1_xtaltab0_1600;
	CASE_BCM43684_CHIP:
		return pmu1_xtaltab0_2907;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
		return pmu1_xtaltab0_bcm63178_2880; /* default BBPLL output frequency is 2880 Mhz */
	CASE_BCM6715_CHIP:
		return pmu1_xtaltab0_bcm6715_5760;
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
#endif // endif

	switch (CHIPID(sih->chip)) {
	case BCM4360_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM43526_CHIP_ID:
	case BCM43570_CHIP_ID:
		/* Default to 37400Khz */
		return &pmu1_xtaltab0_960[PMU1_XTALTAB0_960_37400K];
	CASE_BCM43602_CHIP:
		return &pmu1_xtaltab0_960[PMU1_XTALTAB0_960_40000K];
	case BCM53573_CHIP_GRPID:
		return &pmu1_xtaltab0_640[PMU1_XTALTAB0_640_40000K];
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		return &pmu1_xtaltab0_1600[0];
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
		return &pmu1_xtaltab0_bcm63178_2880[0];
	CASE_BCM6715_CHIP:
		return &pmu1_xtaltab0_bcm6715_5760[0];
	CASE_BCM43684_CHIP:
		return &pmu1_xtaltab0_2907[0];
	case BCM4347_CHIP_GRPID:
		return &pmu1_xtaltab0_960[PMU1_XTALTAB0_960_37400K];
	default:
		PMU_MSG(("si_pmu1_xtaldef0: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		break;
	}
	ASSERT(0);
	return NULL;
} /* si_pmu1_xtaldef0 */

static uint32 fvco_4360 = 0;

/**
 * store the val on init, then if func is called during normal operation
 * don't touch core regs anymore
 */
static uint32 si_pmu_pll1_fvco_4360(si_t *sih, osl_t *osh)
{
	uint32 xf, ndiv_int, ndiv_frac, fvco, pll_reg, p1_div_scale;
	uint32 r_high, r_low, int_part, frac_part, rounding_const;
	uint8 p1_div;
	uint origidx = 0, intr_val = 0;

	if (fvco_4360) {
		printf("%s:attempt to query fvco during normal operation\n",
			__FUNCTION__);
		/* this will insure that the func is called only once upon init */
		return fvco_4360;
	}

	/* Remember original core before switch to chipc */
	si_switch_core(sih, CC_CORE_ID, &origidx, &intr_val);

	xf = si_pmu_alp_clock(sih, osh)/1000;

	/* pll reg 10 , p1div, ndif_mode, ndiv_int */
	pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG10, 0, 0);
	p1_div = pll_reg & 0xf;
	ndiv_int = (pll_reg >> 7)  & 0x1f;

	/* pllctrl11 */
	pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG11, 0, 0);
	ndiv_frac = pll_reg & 0xfffff;

	int_part = xf * ndiv_int;

	rounding_const = 1 << (BBPLL_NDIV_FRAC_BITS - 1);
	math_uint64_multiple_add(&r_high, &r_low, ndiv_frac, xf, rounding_const);
	math_uint64_right_shift(&frac_part, r_high, r_low, BBPLL_NDIV_FRAC_BITS);

	if (!p1_div) {
		PMU_ERROR(("p1_div calc returned 0! [%d]\n", __LINE__));
		ROMMABLE_ASSERT(0);
	}

	if (p1_div == 0) {
		ASSERT(p1_div != 0);
		p1_div_scale = 0;
	} else

	p1_div_scale = (1 << P1_DIV_SCALE_BITS) / p1_div;
	rounding_const = 1 << (P1_DIV_SCALE_BITS - 1);

	math_uint64_multiple_add(&r_high, &r_low, (int_part + frac_part),
		p1_div_scale, rounding_const);
	math_uint64_right_shift(&fvco, r_high, r_low, P1_DIV_SCALE_BITS);

	/* Return to original core */
	si_restore_core(sih, origidx, intr_val);

	fvco_4360 = fvco;
	return fvco;
} /* si_pmu_pll1_fvco_4360 */

/** returns chip specific default BaseBand pll fvco frequency in [khz] units */
static uint32
BCMINITFN(si_pmu1_pllfvco0)(si_t *sih)
{
#ifdef BCMDBG
	char chn[8];
#endif // endif

	switch (CHIPID(sih->chip)) {
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID:
		return FVCO_960;

	CASE_BCM43602_CHIP:
		return FVCO_960;
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		return FVCO_1920;	/* For BB PLL */
	case BCM43570_CHIP_ID:
		return FVCO_960010;
	case BCM53573_CHIP_GRPID:
	{
		osl_t *osh;

		osh = si_osh(sih);
		return (si_pmu_cal_fvco(sih, osh));
	}
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	{
		osl_t *osh;
		osh = si_osh(sih);
		return si_pmu_pll1_fvco_4360(sih, osh);
	}
	case BCM4347_CHIP_GRPID:
		return FVCO_960p5;
	case BCM4369_CHIP_GRPID:
		return FVCO_960p1;
	CASE_BCM6715_CHIP:
		return FVCO_5760;
	CASE_BCM43684_CHIP:
		return FVCO_2907;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
		return FVCO_2907;
	default:
		PMU_MSG(("si_pmu1_pllfvco0: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		break;
	}
	ASSERT(0);
	return 0;
} /* si_pmu1_pllfvco0 */

/** returns chip specific default CPU pll vco frequency in [khz] units */
static uint32
BCMINITFN(si_pmu1_pllfvco1)(si_t *sih)
{
#ifdef BCMDBG
	char chn[8];
#endif // endif
	switch (CHIPID(sih->chip)) {
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		return FVCO_1600;
	CASE_BCM43684_CHIP:
		return FVCO_3000;
	CASE_BCM6715_CHIP:
		return FVCO_6000;
	default:
		PMU_MSG(("si_pmu1_pllfvco1: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		break;
	}
	ASSERT(0);
	return 0;
}

/**
 * returns chip specific default pll fvco frequency in [khz] units
 * for second pll for arm clock in 4347 - 420 MHz
 */
static uint32
BCMINITFN(si_pmu1_pllfvco0_pll2)(si_t *sih)
{
#ifdef BCMDBG
	char chn[8];
#endif // endif

	switch (CHIPID(sih->chip)) {
	case BCM4347_CHIP_GRPID:
		return FVCO_385;

	case BCM4369_CHIP_GRPID:
		return FVCO_400;
	default:
		PMU_MSG(("%s : Unknown chipid %s\n",
				__FUNCTION__, bcm_chipname(sih->chip, chn, 8)));
		ASSERT(0);
		break;
	}
	return 0;
} /* si_pmu1_pllfvco0_pll2 */

/** query alp/xtal clock frequency */
static uint32
BCMINITFN(si_pmu1_alpclk0)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	const pmu1_xtaltab0_t *xt;
	uint32 xf;
	BCM_REFERENCE(sih);

	/* Find the frequency in the table */
	xf = (R_REG(osh, &pmu->pmucontrol) & PCTL_XTALFREQ_MASK) >>
	        PCTL_XTALFREQ_SHIFT;
	for (xt = si_pmu1_xtaltab0(sih); xt != NULL && xt->fref != 0; xt ++)
		if (xt->xf == xf)
			break;
	/* Could not find it so assign a default value */
	if (xt == NULL || xt->fref == 0)
		xt = si_pmu1_xtaldef0(sih);
	ASSERT(xt != NULL && xt->fref != 0);

	return xt->fref * 1000;
}

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
		CASE_BCM43602_CHIP:  /* Same HT_ vals as 4350 */
		case BCM53573_CHIP_GRPID: /* ht start is not defined for 53573 */
		case BCM4363_CHIP_ID:
		case BCM4365_CHIP_ID:
		case BCM4366_CHIP_ID:
		case BCM43664_CHIP_ID:
	        case BCM43666_CHIP_ID:
		case BCM43570_CHIP_ID:
		CASE_BCM43684_CHIP:
		CASE_BCM6715_CHIP:
		CASE_EMBEDDED_2x2AX_CORE:
		CASE_BCM6878_CHIP:
			ht_req |= PMURES_BIT(rsc->ht_start);
			break;
		case BCM4347_CHIP_GRPID:
			ht_req |= PMURES_BIT(rsc->ht_start);
			break;
		default:
			ASSERT(0);
			break;
	}

	return ht_req;
} /* si_pmu_htclk_mask */

/** returns ALP frequency in [Hz] */
uint32
BCMATTACHFN(si_pmu_def_alp_clock)(si_t *sih, osl_t *osh)
{
	uint32 clock = ALP_CLOCK;

	BCM_REFERENCE(osh);

	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
	case BCM4347_CHIP_GRPID:
		clock = 37400*1000;
		break;
	case BCM53573_CHIP_GRPID:
		clock = 40000*1000;
		break;
	CASE_BCM43602_CHIP:
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		clock = 40000 * 1000;
		break;
	CASE_BCM43684_CHIP:
		clock = 54000 * 1000;
		break;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
	CASE_BCM6710_CHIP:
	CASE_BCM6715_CHIP:
		clock = 50000 * 1000;
		break;
	}

	return clock;
}

/**
 * The BBPLL register set needs to be reprogrammed because the x-tal frequency is not known at
 * compile time, or a different spur mode is selected. This function writes appropriate values into
 * the BBPLL registers. It returns the 'xf', corresponding to the 'xf' bitfield in the PMU control
 * register.
 * @param[in] xtal                Xtal frequency in [KHz]
 * @param[in] spurmode            0 for default, 'non spur' mode.
 * @param[in] pllctrlreg_update   Contains info on what entries to use in 'pllctrlreg_val' for the
 *                                given x-tal frequency and spur mode.
 * @param[in] array_size          Number of elements in array pllctrlreg_update[].
 * @param[in] pllctrlreg_val      An array of literal BBPLL values to write.
 *
 * Note: if pmu is NULL, this function returns xf, without programming PLL registers.
 * This function is only called for pmu1_ type chips, perhaps we should rename it.
 */
static uint8
si_pmu_pllctrlreg_update(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 xtal,
            uint8 spur_mode, const pllctrl_data_t *pllctrlreg_update, uint32 array_size,
            const uint32 *pllctrlreg_val)
{
	uint8 indx, reg_offset, xf = 0;
	uint8 pll_ctrlcnt = 0;

	ASSERT(pllctrlreg_update);

	if (PMUREV(sih->pmurev) >= 5) {
		pll_ctrlcnt = (sih->pmucaps & PCAP5_PC_MASK) >> PCAP5_PC_SHIFT;
	} else {
		pll_ctrlcnt = (sih->pmucaps & PCAP_PC_MASK) >> PCAP_PC_SHIFT;
	}

	/* Program the PLL control register if the xtal value matches with the table entry value */
	for (indx = 0; indx < array_size; indx++) {
		/* If the entry does not match the xtal and spur_mode just continue the loop */
		if (!((pllctrlreg_update[indx].clock == (uint16)xtal) &&
			(pllctrlreg_update[indx].mode == spur_mode)))
			continue;
		/*
		 * Don't program the PLL registers if register base is NULL.
		 * If NULL just return the xref.
		 */
		if (pmu) {
			for (reg_offset = 0; reg_offset < pll_ctrlcnt; reg_offset++) {
				si_pmu_pllcontrol(sih, reg_offset, ~0,
					pllctrlreg_val[indx*pll_ctrlcnt + reg_offset]);
			}
		}
		xf = pllctrlreg_update[indx].xf;
		break;
	}
	return xf;
} /* si_pmu_pllctrlreg_update */

/**
 * Chip-specific overrides to PLLCONTROL registers during init. If certain conditions (dependent on
 * x-tal frequency and current ALP frequency) are met, an update of the PLL is required.
 *
 * This takes less precedence over OTP PLLCONTROL overrides.
 *
 * @param [in] update_required   The first time this function is called, this parameter is FALSE.
 *                               The function will then read PLL registers, and determine if they
 *                               need to be updated. If so, this function returns TRUE, without
 *                               having written any PLL registers.
 *                               On a return value of 'TRUE', the caller will then take its
 *                               precautions and call this function a second time, this time with
 *                               parameter 'update_required' set to 'TRUE'. In that case, this
 *                               function will update PLL registers.
 *
 * Return value: TRUE if the BBPLL registers 'update' field should be written by the caller.
 *
 * This function is only called for pmu1_ type chips, perhaps we should rename it.
 */
bool
BCMATTACHFN(si_pmu_update_pllcontrol)(si_t *sih, osl_t *osh, uint32 xtal, bool update_required)
{
	pmuregs_t *pmu;
	uint origidx;
	bool write_en = FALSE;
	uint8 xf = 0;
	const pmu1_xtaltab0_t *xt;
	uint32 tmp;
	const pllctrl_data_t *pllctrlreg_update = NULL;
	uint32 array_size = 0;
	/* points at a set of PLL register values to write for a given x-tal frequency: */
	const uint32 *pllctrlreg_val = NULL;
	uint8 ndiv_mode = PMU1_PLL0_PC2_NDIV_MODE_MASH;
	uint32 xtalfreq = 0;

	/* If there is OTP or NVRAM entry for xtalfreq, program the
	 * PLL control register even if it is default xtal.
	 */
	xtalfreq = getintvar(NULL, rstr_xtalfreq);

#if !defined(BCMDONGLEHOST)
	/* Some NIC WL combo-chips, like the 43570, don't have srom support,
	 * and must field the xtal from NVRAM.  However, there could be more
	 * than one xtal entry in nvram, e.g.,
	 *    xtalfreq=25000    <<< motherboard
	 *    1:xtalfreq=40000  <<< WL NIC chip
	 *
	 * Using getintvar() with NULL (above), will return the first found
	 * xtalfreq, i.e., the motherboard's xtal, which could be wrong.
	 * For these chips, retain the xtal value assigned within si_doattach,
	 * and force CASE1, below.
	 */
	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
		if (xtal) {
			xtalfreq = xtal;
		} else {
			PMU_MSG(("%s: Warning: chip %x is using default XTAL %d\n",
				__FUNCTION__, CHIPID(sih->chip), xtalfreq));
		}
		break;
	default:
		/* do nothing */
		break;
	}
#endif /* !defined(BCMDONGLEHOST) */

	/* CASE1 */
	if (xtalfreq) {
		write_en = TRUE;
		xtal = xtalfreq;
	} else {
		/* There is NO OTP value */
		if (xtal) {
			/* CASE2: If the xtal value was calculated, program the PLL control
			 * registers only if it is not default xtal value.
			 */
			if (xtal != (si_pmu_def_alp_clock(sih, osh)/1000))
				write_en = TRUE;
		} else {
			/* CASE3: If the xtal obtained is "0", ie., clock is not measured, then
			 * leave the PLL control register as it is but program the xf in
			 * pmucontrol register with the default xtal value.
			 */
			xtal = si_pmu_def_alp_clock(sih, osh)/1000;
		}
	}

	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
		pllctrlreg_update = pmu1_xtaltab0_4350;
		array_size = ARRAYSIZE(pmu1_xtaltab0_4350);

		if (CHIPID(sih->chip) == BCM43570_CHIP_ID ||
			(CHIPREV(sih->chiprev) >= 3)) {
			pllctrlreg_val = pmu1_pllctrl_tab_4350C0_963mhz;
		} else {
			pllctrlreg_val = pmu1_pllctrl_tab_4350_963mhz;
		}

		/* If PMU1_PLL0_PC2_MxxDIV_MASKxx have to change,
		 * then set write_en to true.
		 */
#ifdef BCMUSB_NODISCONNECT
		write_en = FALSE;
#else
		write_en = TRUE;
#endif // endif
#ifdef BCM_BOOTLOADER
		/* Bootloader need to change pll if it is not default 37.4M */
		if (xtal == XTAL_DEFAULT_4350)
			write_en = FALSE;
#endif /*  BCM_BOOTLOADER */
		break;

	case BCM53573_CHIP_GRPID:
		pllctrlreg_update = pmu1_xtaltab0_53573;
		array_size = ARRAYSIZE(pmu1_xtaltab0_53573);
		pllctrlreg_val = pmu1_pllctrl_tab_53573_640mhz;
		/* If PMU1_PLL0_PC2_MxxDIV_MASKxx have to change,
		 * then set write_en to true.
		 */
		 break;

	case BCM4347_CHIP_GRPID:
		pllctrlreg_update = pmu1_xtaltab0_4347;
		array_size = ARRAYSIZE(pmu1_xtaltab0_4347);
		pllctrlreg_val = pmu1_pllctrl_tab_4347_960p5mhz;
		break;
	CASE_BCM43602_CHIP:
		/*
		 * XXX43602 has only 1 x-tal value, possibly insert case when an other BBPLL
		 * frequency than 960Mhz is required (e.g., for spur avoidance)
		 */
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
	CASE_BCM43684_CHIP:
	CASE_BCM6715_CHIP:
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
		 /* fall through */
	default:
		/* write_en is FALSE in this case. So returns from the function */
		write_en = FALSE;
		break;
	}

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	/* Check if the table has PLL control register values for the requested xtal */
	if (!update_required && pllctrlreg_update) {
		/* Here the chipcommon register base is passed as NULL, so that we just get
		 * the xf for the xtal being programmed but don't program the registers now
		 * as the PLL is not yet turned OFF.
		 */
		xf = si_pmu_pllctrlreg_update(sih, osh, NULL, xtal, 0, pllctrlreg_update,
			array_size, pllctrlreg_val);

		/* Program the PLL based on the xtal value. */
		if (xf != 0) {
			/* Write XtalFreq. Set the divisor also. */
			tmp = R_REG(osh, &pmu->pmucontrol) &
				~(PCTL_ILP_DIV_MASK | PCTL_XTALFREQ_MASK);
			tmp |= (((((xtal + 127) / 128) - 1) << PCTL_ILP_DIV_SHIFT) &
				PCTL_ILP_DIV_MASK) |
				((xf << PCTL_XTALFREQ_SHIFT) & PCTL_XTALFREQ_MASK);
			W_REG(osh, &pmu->pmucontrol, tmp);
		} else {
			write_en = FALSE;
			printf(rstr_Invalid_Unsupported_xtal_value_D, xtal);
		}
	}

	/* If its a check sequence or if there is nothing to write, return here */
	if ((update_required == FALSE) || (write_en == FALSE)) {
		goto exit;
	}

	/* Update the PLL control register based on the xtal used. */
	if (pllctrlreg_val) {
		si_pmu_pllctrlreg_update(sih, osh, pmu, xtal, 0, pllctrlreg_update, array_size,
			pllctrlreg_val);
	}

	/* Program the PLL based on the xtal value. */
	if (xtal != 0) {
		/* Find the frequency in the table */
		for (xt = si_pmu1_xtaltab0(sih); xt != NULL && xt->fref != 0; xt ++)
			if (xt->fref == xtal)
				break;

		/* Check current PLL state, bail out if it has been programmed or
		 * we don't know how to program it. But we might still have some programming
		 * like changing the ARM clock, etc. So cannot return from here.
		 */
		if (xt == NULL || xt->fref == 0)
			goto exit;

		/* If the PLL is already programmed exit from here. */
		if (((R_REG(osh, &pmu->pmucontrol) &
			PCTL_XTALFREQ_MASK) >> PCTL_XTALFREQ_SHIFT) == xt->xf)
			goto exit;

		PMU_MSG(("XTAL %d.%d MHz (%d)\n", xtal / 1000, xtal % 1000, xt->xf));
		PMU_MSG(("Programming PLL for %d.%d MHz\n", xt->fref / 1000, xt->fref % 1000));

		if (BCM4369_CHIP(sih->chip)) {

			/* Write pdiv (Actually it is mapped to p1div in the struct)
			 to pllcontrol[2]
			 */
			tmp = ((xt->p1div << PMU4369_PLL0_PC2_PDIV_SHIFT) &
				PMU4369_PLL0_PC2_PDIV_MASK);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2,
				(PMU4369_PLL0_PC2_PDIV_MASK), tmp);

			/* Write ndiv_int to pllcontrol[2] */
			tmp = ((xt->ndiv_int << PMU4369_PLL0_PC2_NDIV_INT_SHIFT)
					& PMU4369_PLL0_PC2_NDIV_INT_MASK);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2,
				(PMU4369_PLL0_PC2_NDIV_INT_MASK), tmp);

			/* Write ndiv_frac to pllcontrol[3] */
			tmp = ((xt->ndiv_frac << PMU4369_PLL0_PC3_NDIV_FRAC_SHIFT) &
				PMU4369_PLL0_PC3_NDIV_FRAC_MASK);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3,
				PMU4369_PLL0_PC3_NDIV_FRAC_MASK, tmp);

		} else {
			/* Write p1div and p2div to pllcontrol[0] */
			tmp = ((xt->p1div << PMU1_PLL0_PC0_P1DIV_SHIFT) &
				PMU1_PLL0_PC0_P1DIV_MASK) |
				((xt->p2div << PMU1_PLL0_PC0_P2DIV_SHIFT) &
				PMU1_PLL0_PC0_P2DIV_MASK);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0,
				(PMU1_PLL0_PC0_P1DIV_MASK | PMU1_PLL0_PC0_P2DIV_MASK), tmp);

			/* Write ndiv_int and ndiv_mode to pllcontrol[2] */
			tmp = ((xt->ndiv_int << PMU1_PLL0_PC2_NDIV_INT_SHIFT)
					& PMU1_PLL0_PC2_NDIV_INT_MASK) |
					((ndiv_mode << PMU1_PLL0_PC2_NDIV_MODE_SHIFT)
					& PMU1_PLL0_PC2_NDIV_MODE_MASK);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2,
				(PMU1_PLL0_PC2_NDIV_INT_MASK | PMU1_PLL0_PC2_NDIV_MODE_MASK), tmp);
			/* Write ndiv_frac to pllcontrol[3] */
			if (BCM4347_CHIP(sih->chip)) {
				tmp = ((xt->ndiv_frac << PMU4347_PLL0_PC3_NDIV_FRAC_SHIFT) &
					PMU4347_PLL0_PC3_NDIV_FRAC_MASK);
				si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3,
					PMU4347_PLL0_PC3_NDIV_FRAC_MASK, tmp);
			} else
			{
				tmp = ((xt->ndiv_frac << PMU1_PLL0_PC3_NDIV_FRAC_SHIFT) &
					PMU1_PLL0_PC3_NDIV_FRAC_MASK);
				si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3,
					PMU1_PLL0_PC3_NDIV_FRAC_MASK, tmp);
			}
		}

		/* Write XtalFreq. Set the divisor also. */
		tmp = R_REG(osh, &pmu->pmucontrol) &
			~(PCTL_ILP_DIV_MASK | PCTL_XTALFREQ_MASK);
		tmp |= (((((xt->fref + 127) / 128) - 1) << PCTL_ILP_DIV_SHIFT) &
			PCTL_ILP_DIV_MASK) |
			((xt->xf << PCTL_XTALFREQ_SHIFT) & PCTL_XTALFREQ_MASK);
		W_REG(osh, &pmu->pmucontrol, tmp);
	}

exit:
	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return write_en;
} /* si_pmu_update_pllcontrol */

/* returns current value of PMUTimer.
	XXX: also taking care of PR88659 by multiple reads.
*/
uint32
si_pmu_get_pmutimer(si_t *sih)
{
	osl_t *osh = si_osh(sih);
	pmuregs_t *pmu;
	uint origidx;
	uint32 start;
	BCM_REFERENCE(sih);

	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	start = R_REG(osh, &pmu->pmutimer);
	if (start != R_REG(osh, &pmu->pmutimer))
		start = R_REG(osh, &pmu->pmutimer);

	si_setcoreidx(sih, origidx);

	return (start);
}

/* Get current pmu time API */
uint32
si_cur_pmu_time(si_t *sih)
{
	uint origidx;
	uint32 pmu_time;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);

	pmu_time = si_pmu_get_pmutimer(sih);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	return (pmu_time);
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

	if (BCM4365_CHIP(sih->chip) ||
		(BCM4347_CHIP(sih->chip)) ||
		BCM43602_CHIP(sih->chip) ||
		BCM4350_CHIP(sih->chip) ||
		BCM53573_CHIP(sih->chip) ||
		EMBEDDED_2x2AX_CORE(sih->chip) ||
		BCM6715_CHIP(sih->chip) ||
		BCM43684_CHIP(sih->chip)) {
		/* slightly different way for 4335, but this could be applied for other chips also.
		* If HT_AVAIL is not set, wait to see if any resources are availing HT.
		*/
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

#if defined(DONGLEBUILD) && defined(__ARM_ARCH_7A__)
	asm volatile("isb");
#endif /* defined(DONGLEBUILD) && defined(__ARM_ARCH_7A__) */

	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL) == CCS_HTAVAIL), PMU_MAX_TRANSITION_DLY);
	ASSERT(!(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL));
	OSL_DELAY(100);
} /* si_pmu_pll_off */

/* below function are for BBPLL parallel purpose */
/** Turn Off the PLL - Required before setting the PLL registers */
void
si_pmu_pll_off_PARR(si_t *sih, osl_t *osh, uint32 *min_mask,
uint32 *max_mask, uint32 *clk_ctl_st)
{
	pmuregs_t *pmu;
	uint origidx, intr_val;
	uint32 ht_req;

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	/* Save the original register values */
	*min_mask = R_REG(osh, &pmu->min_res_mask);
	*max_mask = R_REG(osh, &pmu->max_res_mask);
	*clk_ctl_st = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0);
	ht_req = si_pmu_htclk_mask(sih);
	if (ht_req == 0) {
		/* Return to original core */
		si_setcoreidx(sih, origidx);
		si_intrrestore(sih, intr_val);
		return;
	}

	if (BCM4365_CHIP(sih->chip) ||
		(BCM4347_CHIP(sih->chip)) ||
		BCM43602_CHIP(sih->chip) ||
		BCM4350_CHIP(sih->chip) ||
		BCM53573_CHIP(sih->chip) ||
		EMBEDDED_2x2AX_CORE(sih->chip) ||
		BCM6715_CHIP(sih->chip) ||
		BCM43684_CHIP(sih->chip)) {
		/* slightly different way for 4335, but this could be applied for other chips also.
		* If HT_AVAIL is not set, wait to see if any resources are availing HT.
		*/
		if (((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL)
			!= CCS_HTAVAIL))
			si_pmu_wait_for_steady_state(sih, osh, pmu);
	} else {
		OR_REG(osh, &pmu->max_res_mask, ht_req);
		/* wait for HT to be ready before taking the HT away...HT could be coming up... */
		SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL) != CCS_HTAVAIL), PMU_MAX_TRANSITION_DLY);
		ASSERT((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL));
	}

	AND_REG(osh, &pmu->min_res_mask, ~ht_req);
	AND_REG(osh, &pmu->max_res_mask, ~ht_req);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	si_intrrestore(sih, intr_val);
} /* si_pmu_pll_off_PARR */

static void
si_pmu_pll_off_isdone(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 ht_req;

	ht_req = si_pmu_htclk_mask(sih);
	SPINWAIT(((R_REG(osh, &pmu->res_state) & ht_req) != 0),
	PMU_MAX_TRANSITION_DLY);
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL) == CCS_HTAVAIL), PMU_MAX_TRANSITION_DLY);
	ASSERT(!(si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL));
}

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

#if defined(DONGLEBUILD) && defined(__ARM_ARCH_7A__)
	asm volatile("isb");
#endif /* defined(DONGLEBUILD) && defined(__ARM_ARCH_7A__) */

	if (clk_ctl_st_mask & CCS_HTAVAIL) {
		/* Wait for HT_AVAIL to come back */
		SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL) != CCS_HTAVAIL), PMU_MAX_TRANSITION_DLY);
		ASSERT((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HTAVAIL));
	}
}

/**
 * This function controls the PLL register update while
 * switching MAC Clock Frequency dynamically between 120MHz and 160MHz
 * in the case of 4364
 */
void
si_pmu_pll_4364_macfreq_switch(si_t *sih, osl_t *osh, uint8 mode)
{
	uint32 max_res_mask = 0, min_res_mask = 0, clk_ctl_st = 0;
	uint origidx = si_coreidx(sih);
	pmuregs_t *pmu = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(pmu != NULL);
	si_pmu_pll_off(sih, osh, pmu, &min_res_mask, &max_res_mask, &clk_ctl_st);
	if (mode == PMU1_PLL0_SWITCH_MACCLOCK_120MHZ) { /* 120MHz */
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, PMU1_PLL0_PC1_M2DIV_MASK,
			(PMU1_PLL0_PC1_M2DIV_VALUE_120MHZ << PMU1_PLL0_PC1_M2DIV_SHIFT));
	} else if (mode == PMU1_PLL0_SWITCH_MACCLOCK_160MHZ) {  /* 160MHz */
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, PMU1_PLL0_PC1_M2DIV_MASK,
			(PMU1_PLL0_PC1_M2DIV_VALUE_160MHZ << PMU1_PLL0_PC1_M2DIV_SHIFT));
	}
	W_REG(osh, &pmu->pmucontrol,
		R_REG(osh, &pmu->pmucontrol) | PCTL_PLL_PLLCTL_UPD);
	si_pmu_pll_on(sih, osh, pmu, min_res_mask, max_res_mask, clk_ctl_st);
	si_setcoreidx(sih, origidx);
}

/**
 * Set up PLL registers in the PMU as per the (optional) OTP values, or, if no OTP values are
 * present, optionally update with POR override values contained in firmware. Enables the BBPLL
 * when done.
 */
static void
BCMATTACHFN(si_pmu1_pllinit1)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 xtal)
{
	char name[16];
	const char *otp_val;
	uint8 i, otp_entry_found = FALSE;
	uint32 pll_ctrlcnt;
	uint32 min_mask = 0, max_mask = 0, clk_ctl_st = 0;

	if (PMUREV(sih->pmurev) >= 5) {
		pll_ctrlcnt = (sih->pmucaps & PCAP5_PC_MASK) >> PCAP5_PC_SHIFT;
	} else {
		pll_ctrlcnt = (sih->pmucaps & PCAP_PC_MASK) >> PCAP_PC_SHIFT;
	}

	/* Check if there is any otp enter for PLLcontrol registers */
	for (i = 0; i < pll_ctrlcnt; i++) {
		snprintf(name, sizeof(name), rstr_pllD, i);
		if ((otp_val = getvar(NULL, name)) == NULL)
			continue;

		/* If OTP entry is found for PLL register, then turn off the PLL
		 * and set the status of the OTP entry accordingly.
		 */
		otp_entry_found = TRUE;
		break;
	}

	/* If no OTP parameter is found and no chip-specific updates are needed, return. */
	if ((otp_entry_found == FALSE) &&
		(si_pmu_update_pllcontrol(sih, osh, xtal, FALSE) == FALSE))
		return;

	/* Make sure PLL is off */
	si_pmu_pll_off(sih, osh, pmu, &min_mask, &max_mask, &clk_ctl_st);

	/* Update any chip-specific PLL registers. Does not write PLL 'update' bit yet. */
	si_pmu_update_pllcontrol(sih, osh, xtal, TRUE);

	/* Update the PLL register if there is a OTP entry for PLL registers */
	si_pmu_otp_pllcontrol(sih, osh);

	/* Flush ('update') the deferred pll control registers writes */
	if (PMUREV(sih->pmurev) >= 2)
		OR_REG(osh, &pmu->pmucontrol, PCTL_PLL_PLLCTL_UPD);

	/* Restore back the register values. This ensures PLL remains on if it
	 * was originally on and remains off if it was originally off.
	 */
	si_pmu_pll_on(sih, osh, pmu, min_mask, max_mask, clk_ctl_st);
} /* si_pmu1_pllinit1 */

/**
 * returns the CPU clock frequency. Does this by determining current Fvco and the setting of the
 * clock divider that leads up to the ARM. Returns value in [Hz] units.
 */
static uint32
BCMINITFN(si_pmu1_cpuclk0)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 tmp, tmp1, mdiv = 1;
	uint32 ndiv_int, ndiv_frac, p2div, p1div, fvco, p = 0, q = 0;
	uint32 fref;
#ifdef BCMDBG
	char chn[8];
#endif // endif
	uint32 FVCO;	/* in [khz] units */

	if (BCM4365_CHIP(sih->chip) || BCM43684_CHIP(sih->chip) ||
		BCM6715_CHIP(sih->chip)) {
		/* these chips derive CPU clock from CPU PLL instead of BB PLL */
		FVCO = si_pmu1_pllfvco1(sih);
	} else {
		FVCO = si_pmu1_pllfvco0(sih);
	}

	if (BCM43602_CHIP(sih->chip)) {
		/* CR4 running on backplane_clk */
		uint32 clock = si_pmu_si_clock(sih, osh);	/* in [hz] units */
#if defined(DONGLEBUILD) && defined(__ARM_ARCH_7R__)
		if (si_arm_clockratio(sih, 0) == 2) {
			clock *= 2;				/* running at double frequency */
		}
#endif /* DONGLEBUILD && __ARM_ARCH_7R__ */
		return clock;
	}

	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, 0, 0);
		mdiv = (tmp & PMU1_PLL0_PC2_M5DIV_MASK) >> PMU1_PLL0_PC2_M5DIV_SHIFT;
		break;
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM43526_CHIP_ID:
	case BCM4352_CHIP_ID:
		/* Read m6div from pllcontrol[5] */
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG5, 0, 0);
		mdiv = (tmp & PMU1_PLL0_PC2_M6DIV_MASK) >> PMU1_PLL0_PC2_M6DIV_SHIFT;
		break;
#ifdef DONGLEBUILD
	CASE_BCM43602_CHIP:
#ifdef __arm__
		ASSERT(si_arm_clockratio(sih, 0) == 2);
#endif // endif
		/* CR4 running on armcr4_clk (Ch5). Read 'bbpll_i_m5div' from pllctl[5] */
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG5, 0, 0);
		mdiv = (tmp & PMU1_PLL0_PC2_M5DIV_MASK) >> PMU1_PLL0_PC2_M5DIV_SHIFT;
		break;
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG11, 0, 0);
		mdiv = (tmp & PMU1_PLL0_PC1_M1DIV_MASK) >> PMU1_PLL0_PC1_M1DIV_SHIFT;
		break;
#endif /* DONGLEBUILD */

	CASE_BCM43684_CHIP: /* Uses CPU PLL instead of BB PLL to derive CPU clock */
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG12, 0, 0);
		mdiv = (tmp & PMU1_PLL0_PC1_M1DIV_MASK) >> PMU1_PLL0_PC1_M1DIV_SHIFT;
		break; /* default backplane frequency is 240MHz */

	CASE_BCM6715_CHIP: /* Uses CPU PLL instead of BB PLL to derive CPU clock */
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG9, 0, 0);
		mdiv = (tmp & PMU6715_PLL1_PC9_M1DIV_MASK) >> PMU6715_PLL1_PC9_M1DIV_SHIFT;
		break;

	case BCM4347_CHIP_GRPID:
		tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, 0, 0);
		mdiv = (tmp & PMU1_PLL0_PC1_M3DIV_MASK) >> PMU1_PLL0_PC1_M3DIV_SHIFT;
		break;

	default:
		PMU_MSG(("si_pmu1_cpuclk0: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		ASSERT(0);
		break;
	}

	ASSERT(mdiv != 0);

	/* Read p2div/p1div from pllcontrol[0] */
	tmp = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, 0, 0);
	p2div = (tmp & PMU1_PLL0_PC0_P2DIV_MASK) >> PMU1_PLL0_PC0_P2DIV_SHIFT;
	p1div = (tmp & PMU1_PLL0_PC0_P1DIV_MASK) >> PMU1_PLL0_PC0_P1DIV_SHIFT;

	/* Calculate fvco based on xtal freq and ndiv and pdiv */
	if (BCM4365_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG9;
	} else if (BCM43684_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG10;
	} else if (BCM6715_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG6;
	} else {
		tmp = PMU1_PLL0_PLLCTL2;
	}

	tmp = si_pmu_pllcontrol(sih, tmp, 0, 0);

	if (BCM4350_CHIP(sih->chip)) {
		p2div = 1;
		p1div = (tmp & PMU4335_PLL0_PC2_P1DIV_MASK) >> PMU4335_PLL0_PC2_P1DIV_SHIFT;
		ndiv_int = (tmp & PMU4335_PLL0_PC2_NDIV_INT_MASK) >>
		           PMU4335_PLL0_PC2_NDIV_INT_SHIFT;
	} else if (BCM4365_CHIP(sih->chip) || BCM43684_CHIP(sih->chip) ||
		BCM6715_CHIP(sih->chip)) {
		ndiv_int = (tmp >> 4) & 0x3ff;
		p2div = 1;
		p1div = (tmp & 7);
	} else if (BCM4347_CHIP(sih->chip)) {
		p2div = 1;
		p1div = (tmp & PMU4347_PLL0_PC2_P1DIV_MASK) >> PMU4347_PLL0_PC2_P1DIV_SHIFT;
		ndiv_int = (tmp & PMU4347_PLL0_PC2_NDIV_INT_MASK) >>
					PMU4347_PLL0_PC2_NDIV_INT_SHIFT;
	} else {
		ndiv_int = (tmp & PMU1_PLL0_PC2_NDIV_INT_MASK) >> PMU1_PLL0_PC2_NDIV_INT_SHIFT;
	}

	ASSERT(p1div != 0);

	if (BCM4365_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG10; /* contains CPU PLL ndiv_frac bitfield */
	} else if (BCM43684_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG11;
	} else if (BCM6715_CHIP(sih->chip)) {
		tmp = PMU_PLL_CTRL_REG7;
	} else {
		tmp = PMU1_PLL0_PLLCTL3;
	}

	tmp = si_pmu_pllcontrol(sih, tmp, 0, 0);

	if (BCM4347_CHIP(sih->chip) || BCM4369_CHIP(sih->chip) ||
	    BCM43684_CHIP(sih->chip)) {
	    if (BCM4369_CHIP(sih->chip)) {
			ndiv_frac =
				(tmp & PMU4369_PLL0_PC3_NDIV_FRAC_MASK) >>
				PMU4369_PLL0_PC3_NDIV_FRAC_SHIFT;
		} else {
			/* PLL has no second pdiv bitfield */
			ndiv_frac =
				(tmp & PMU4347_PLL0_PC3_NDIV_FRAC_MASK) >>
				PMU4347_PLL0_PC3_NDIV_FRAC_SHIFT;
		}
		fref = si_pmu1_alpclk0(sih, osh, pmu) / 1000; /* [KHz] */

		fvco = (fref * ndiv_int) << 8;
		fvco += (fref * ((ndiv_frac & 0xfffff) >> 4)) >> 8;
		fvco >>= 8;
		fvco *= p1div;
		fvco /= 1000;
		fvco *= 1000;
	} else if (BCM6715_CHIP(sih->chip)) {
		fref = si_pmu1_alpclk0(sih, osh, pmu) / 1000; /* [KHz] */

		p = ((tmp & PMU6715_PLL1_PC7_NDIV_P_MASK) >> PMU6715_PLL1_PC7_NDIV_P_SHIFT);
		q = ((tmp & PMU6715_PLL1_PC7_NDIV_Q_MASK) >> PMU6715_PLL1_PC7_NDIV_Q_SHIFT);

		/* check i_ndiv_frac_mode_sel */
		tmp1 = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG8, 0, 0);
		if (tmp1 & PMU6715_PLL1_PC8_NDIV_FRAC_MODE_SEL) {
			/* ndiv_frac_mode_sel 1, fractional value = p/q */
			ndiv_frac = p / q;
			fvco = (fref * ndiv_int);
			fvco += ((fref * p) / q);
		} else {
			/* ndiv_frac_mode_sel 0, fractional value= (p*2^10+q)/2^20 */
			ndiv_frac = ((p << 10) + q) >> 20;
			fvco =  fref * ndiv_int;
			fvco += (fref * ((p << 10) + q)) >> 20;
		}
		fvco /= p1div;
		fvco /= 1000;
		fvco *= 1000;
	} else {
		ndiv_frac =
			(tmp & PMU1_PLL0_PC3_NDIV_FRAC_MASK) >> PMU1_PLL0_PC3_NDIV_FRAC_SHIFT;

		fref = si_pmu1_alpclk0(sih, osh, pmu) / 1000;

		fvco = (fref * ndiv_int) << 8;
		fvco += (fref * (ndiv_frac >> 12)) >> 4;
		fvco += (fref * (ndiv_frac & 0xfff)) >> 12;
		fvco >>= 8;
		fvco *= p2div;
		fvco /= p1div;
		fvco /= 1000;
		fvco *= 1000;
	}

	PMU_MSG(("si_pmu1_cpuclk0: ndiv_int %u ndiv_frac %u p2div %u p1div %u fvco %u\n",
	         ndiv_int, ndiv_frac, p2div, p1div, fvco));

	if (BCM6715_CHIP(sih->chip)) {
		PMU_MSG(("%s: p %u q %u mdiv %u alpclk0 %u\n", __FUNCTION__,
			p, q, mdiv, fref));
	}

	FVCO = fvco;

	return FVCO / mdiv * 1000; /* Return CPU clock in [Hz] */
} /* si_pmu1_cpuclk0 */

/**
 * BCM4347/4369 specific function returning the CPU clock frequency. Does this by determining
 * current Fvco and the setting of the clock divider that leads up to the ARM.
 * Returns value in [Hz] units. For second pll for arm clock in 4347 - 420MHz
 */
static uint32
BCMINITFN(si_pmu1_cpuclk0_pll2)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 mdiv = 1;
	uint32 FVCO = si_pmu1_pllfvco0_pll2(sih);	/* in [khz] units */
#ifdef BCMDBG
	uint32 ndiv_int, ndiv_frac, p2div, p1div, fvco;
	uint32 fref;
	uint32 tmp;
	BCM_REFERENCE(ndiv_int);
	BCM_REFERENCE(ndiv_frac);
	BCM_REFERENCE(p2div);
	BCM_REFERENCE(p1div);
	BCM_REFERENCE(fvco);
	BCM_REFERENCE(fref);
	BCM_REFERENCE(tmp);
#endif // endif
	mdiv = 1;

	/* Return ARM/SB clock */
	return FVCO / mdiv * 1000;
} /* si_pmu1_cpuclk0_pll2 */

#if defined(DONGLEBUILD)
#define NDIV_INT_2000	0x251
#define NDIV_INT_2700	0x321
#define NDIV_INT_3000	0x371
#define NDIV_INT_3200	0x3b1
#define NDIV_INT_3300	0x3d1
#define NDIV_INT_3600	0x421
#define NDIV_FRAC_2000		0x9858
#define NDIV_FRAC_2700		0x8e396
#define NDIV_FRAC_3000		0x8e396
#define NDIV_FRAC_3200		0x42680
#define NDIV_FRAC_3300		0x1c746
#define NDIV_FRAC_3600		0xab086
#define NDIV_P_2880		0x3c
#define NDIV_Q_2880		0x64
#define NDIV_P_2907		0x2a
#define NDIV_Q_2907		0x12c
#define NDIV_P_3000		0x3fe
#define NDIV_Q_3000		0x3ff
#define NDIV_P_5760		0x3c
#define NDIV_Q_5760		0x32
#define NDIV_P_5814		0x2a
#define NDIV_Q_5814		0x96
#define NDIV_P_6000		0x0
#define NDIV_Q_6000		0x2

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
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= (1 << 14);
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
	W_REG(osh, &pmu->pmucontrol, (oldpmucontrol | (1<<10)));
	R_REG(osh, &pmu->pmucontrol);

	/* Change CPU PLL controlled by force_cpupll_pwrdn_ldo,
	 * force_cpupll_areset and  force_cpupll_dreset
	 */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= (1 << 1) | (1 << 2) | (1 << 3);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* De-Assert cpupll_i_areset */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	W_REG(osh, &pmu->chipcontrol_data, (R_REG(osh, &pmu->chipcontrol_data) & ~(0x1)));
	R_REG(osh, &pmu->chipcontrol_data);

	/* wait for HqClkRequired(HR) which means XTAL will be enabled for ALP request,
	 * or PLL/FLL will be powered up in close-loop mode for HT
	 */
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HQCLKREQ) == CCS_HQCLKREQ), PMU_MAX_TRANSITION_DLY);

	/* De-Assert cpupll_i_dreset */
	W_REG(osh, &pmu->chipcontrol_data, (R_REG(osh, &pmu->chipcontrol_data) & ~(0x2)));
	R_REG(osh, &pmu->pllcontrol_data);

	/* Enable CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	W_REG(osh, &pmu->chipcontrol_data, (R_REG(osh, &pmu->chipcontrol_data) & ~(1 << 14)));
	R_REG(osh, &pmu->chipcontrol_data);

	/* Recovery CPUPLL setting to default value */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	W_REG(osh, &pmu->chipcontrol_data, (R_REG(osh, &pmu->chipcontrol_data) & ~(1 << 3)));
	R_REG(osh, &pmu->chipcontrol_data);

} /* si_set_cpu_vcofreq_43684 */

static void
BCMATTACHFN(si_set_cpu_vcofreq_6715)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint32 freq)
{
	uint32 oldpmucontrol, val;

	uint32 tmp, tmp1, p, q;
	int ndiv_frac_mod_sel, change = 0;

	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG7);
	R_REG(osh, &pmu->pllcontrol_addr);
	tmp = R_REG(osh, &pmu->pllcontrol_data);

	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG8);
	R_REG(osh, &pmu->pllcontrol_addr);
	tmp1 = R_REG(osh, &pmu->pllcontrol_data);

	ndiv_frac_mod_sel =  (tmp1 & PMU6715_PLL1_PC8_NDIV_FRAC_MODE_SEL);

	p = ((tmp & PMU6715_PLL1_PC7_NDIV_P_MASK) >> PMU6715_PLL1_PC7_NDIV_P_SHIFT);
	q = ((tmp & PMU6715_PLL1_PC7_NDIV_Q_MASK) >> PMU6715_PLL1_PC7_NDIV_Q_SHIFT);

	if (!(p == NDIV_P_6000 && q == NDIV_Q_6000 && ndiv_frac_mod_sel == 1))
		change++;

	if ((!change) && (freq == FVCO_6000)) {
		return;
	}

	/* Stop CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= (1 << 14);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	switch (freq) {
		case FVCO_2880:
			p = NDIV_P_2880;
			q = NDIV_Q_2880;
			break;
		case FVCO_2907:
			p = NDIV_P_2907;
			q = NDIV_Q_2907;
			break;
		case FVCO_3000:
			p = NDIV_P_3000;
			q = NDIV_Q_3000;
			break;
		case FVCO_5760:
			p = NDIV_P_5760;
			q = NDIV_Q_5760;
			break;
		case FVCO_5814:
			p = NDIV_P_5814;
			q = NDIV_Q_5814;
			break;
		case FVCO_6000:
			p = NDIV_P_6000;
			q = NDIV_Q_6000;
			break;
	}
	/* Set ndiv_frac_mod_sel to 1 */
	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG8);
	R_REG(osh, &pmu->pllcontrol_addr);
	tmp1 = PMU6715_PLL1_PC8_NDIV_FRAC_MODE_SEL;
	W_REG(osh, &pmu->pllcontrol_data, tmp1);
	R_REG(osh, &pmu->pllcontrol_data);

	/* Set ndiv_p, ndiv_q  */
	W_REG(osh, &pmu->pllcontrol_addr, PMU_PLL_CTRL_REG7);
	R_REG(osh, &pmu->pllcontrol_addr);
	tmp = ((p << PMU6715_PLL1_PC7_NDIV_P_SHIFT) |
		(q << PMU6715_PLL1_PC7_NDIV_Q_SHIFT));
	W_REG(osh, &pmu->pllcontrol_data, tmp);
	R_REG(osh, &pmu->pllcontrol_data);

	/* Set pllCntlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, (oldpmucontrol | (1<<10)));
	R_REG(osh, &pmu->pmucontrol);

	/* Change CPU PLL controlled by force_cpupll_pwrdn_ldo,
	 * force_cpupll_areset and  force_cpupll_dreset
	 */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	val = R_REG(osh, &pmu->chipcontrol_data);
	val |= (1 << 1) | (1 << 2) | (1 << 3);
	W_REG(osh, &pmu->chipcontrol_data, val);
	R_REG(osh, &pmu->chipcontrol_data);

	/* De-Assert cpupll_i_areset */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	W_REG(osh, &pmu->chipcontrol_data, (R_REG(osh, &pmu->chipcontrol_data) & ~(0x1)));
	R_REG(osh, &pmu->chipcontrol_data);

	/* wait for HqClkRequired(HR) which means XTAL will be enabled for ALP request,
	 * or PLL/FLL will be powered up in close-loop mode for HT
	 */
	SPINWAIT(((si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
		& CCS_HQCLKREQ) == CCS_HQCLKREQ), PMU_MAX_TRANSITION_DLY);

	/* De-Assert cpupll_i_dreset */
	W_REG(osh, &pmu->chipcontrol_data, (R_REG(osh, &pmu->chipcontrol_data) & ~(0x2)));
	R_REG(osh, &pmu->pllcontrol_data);

	/* Enable CPU PLL clock output */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	W_REG(osh, &pmu->chipcontrol_data, (R_REG(osh, &pmu->chipcontrol_data) & ~(1 << 14)));
	R_REG(osh, &pmu->chipcontrol_data);

	/* Recovery CPUPLL setting to default value */
	W_REG(osh, &pmu->chipcontrol_addr, PMU_CHIPCTL1);
	R_REG(osh, &pmu->chipcontrol_addr);
	W_REG(osh, &pmu->chipcontrol_data, (R_REG(osh, &pmu->chipcontrol_data) & ~(1 << 3)));
	R_REG(osh, &pmu->chipcontrol_data);

} /* si_set_cpu_vcofreq_6715 */

#include <bcmnvram.h>
#define CCICLK	500
#define CCICLK_6715	750
#ifdef BCMQT
#define CPUCLK	500
#define CPUCLK_6715	750
#else /* !BCMQT */
#define CPUCLK	1500
#endif /* BCMQT */
#define CFG_CLK31_RATIO_SHIFT 17
#define CFG_CLK31_RATIO_MASK (1 << CFG_CLK31_RATIO_SHIFT)
#define CFG_CLK_RATIO_SHIFT 28

/** DONGLEBUILD specific */
static void
BCMATTACHFN(si_set_cpu_clock_43684)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 val, bp_on_ht, cpuclk = CPUCLK, cciclk = CCICLK, vcofreq = FVCO_3000;
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
	uint32 val, bp_on_ht, vcofreq, cpuclk = CPUCLK, cciclk = CCICLK_6715;
	uint32 saved_clk_st = 0, max_res_mask = 0, min_res_mask = 0;
	uint8 ratio = 1; /* 2:1 */
	uint8 mdiv = 2;
	char *nvstr;
	char *end;

#ifdef BCMQT
	cpuclk = CPUCLK_6715;
#endif // endif

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
		case 1500:
			if (cciclk == 750) {
				/* 2:1 */
				ratio = 1;
			} else if (cciclk == 500) {
				/* 3:1 */
				ratio = 2;
			} else {
				/* 4:1, 375 MHz */
				ratio = 4;
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
	val &= ~PMU6715_PLL1_PC9_M1DIV_MASK;
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

#endif /* defined(DONGLEBUILD) */

/**
 * Returns the MAC clock frequency. Called when e.g. MAC clk frequency has to change because of
 * interference mitigation. Only called for specific chips.
 */
extern uint32
si_mac_clk(si_t *sih, osl_t *osh)
{
	uint8 mdiv2 = 0;
	uint32 pll_reg, mac_clk = 0;
	chipcregs_t *cc;
	uint origidx, intr_val;
#ifdef BCMDBG
	char chn[8];
#endif // endif

	uint32 FVCO = si_pmu1_pllfvco0(sih);	/* in [khz] units */

	BCM_REFERENCE(osh);

	/* Remember original core before switch to chipc */
	cc = (chipcregs_t *)si_switch_core(sih, CC_CORE_ID, &origidx, &intr_val);
	ASSERT(cc != NULL);
	BCM_REFERENCE(cc);

	switch (CHIPID(sih->chip)) {
	case BCM53573_CHIP_GRPID:
		pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, 0, 0);

		mdiv2 = (pll_reg & PMU4335_PLL0_PC1_MDIV2_MASK) >>
				PMU4335_PLL0_PC1_MDIV2_SHIFT;
		if (!mdiv2) {
			PMU_ERROR(("mdiv2 calc returned 0! [%d]\n", __LINE__));
			ROMMABLE_ASSERT(0);
			break;
		}
		mac_clk = FVCO / mdiv2;
		break;
	default:
		PMU_MSG(("si_mac_clk: Unknown chipid %s\n",
			bcm_chipname(CHIPID(sih->chip), chn, 8)));
		ASSERT(0);
		break;
	}

	/* Return to original core */
	si_restore_core(sih, origidx, intr_val);

	return mac_clk;
} /* si_mac_clk */

/** Get chip's FVCO and PLLCTRL1 register value */
extern int
si_pmu_fvco_pllreg(si_t *sih, uint32 *fvco, uint32 *pllreg)
{
	chipcregs_t *cc;
	uint origidx, intr_val;
	int err = BCME_OK;
#ifdef BCMDBG
	char chn[8];
#endif // endif

	if (fvco)
		*fvco = si_pmu1_pllfvco0(sih)/1000;

	/* Remember original core before switch to chipc */
	cc = (chipcregs_t *)si_switch_core(sih, CC_CORE_ID, &origidx, &intr_val);
	ASSERT(cc != NULL);
	BCM_REFERENCE(cc);

	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
	case BCM53573_CHIP_GRPID:
		if (pllreg)
			*pllreg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, 0, 0);
		break;

	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
		if (pllreg)
			*pllreg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG12, 0, 0) &
				PMU1_PLL0_PC1_M1DIV_MASK;
		break;

	case BCM43602_CHIP_ID:
		if (pllreg) {
			*pllreg = (si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, 0, 0) &
				PMU1_PLL0_PC1_M3DIV_MASK) >> PMU1_PLL0_PC1_M3DIV_SHIFT;
		}
		break;
	case BCM4366_CHIP_ID:
	case BCM43684_CHIP_ID:
	case BCM63178_CHIP_ID:
		if (pllreg) {
			*pllreg = ((si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, 0, 0) &
				PMU1_PLL0_PC1_M4DIV_MASK) >> PMU1_PLL0_PC1_M4DIV_SHIFT);
		}
		break;
	CASE_BCM6715_CHIP:
		if (pllreg) {
			/* dot11mac_clk */
			*pllreg = ((si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, 0, 0) &
				PMU6715_PLL0_PC3_M3DIV_MASK) >> PMU6715_PLL0_PC3_M3DIV_SHIFT);
		}
		break;
	case BCM4347_CHIP_GRPID:
		if (pllreg) {
			*pllreg = (si_pmu_pllcontrol(sih, PMU1_PLL0_PLLCTL1, 0, 0)
			& PMU1_PLL0_PC1_M3DIV_MASK) >> PMU1_PLL0_PC1_M3DIV_SHIFT;
		}
		break;
	default:
		PMU_MSG(("si_mac_clk: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		err = BCME_ERROR;
	}

	/* Return to original core */
	si_restore_core(sih, origidx, intr_val);

	return err;
}

/** Return TRUE if scan retention memory's sleep/pm signal was asserted */
bool si_pmu_reset_ret_sleep_log(si_t *sih, osl_t *osh)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 ret_ctl;
	bool was_sleep = FALSE;

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	ret_ctl = R_REG(osh, &pmu->retention_ctl);
	if (ret_ctl & 0x20000000) {
		W_REG(osh, &pmu->retention_ctl, ret_ctl);
		was_sleep = TRUE;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return was_sleep;
}

/* For 43602a0 MCH2/MCH5 boards: power up PA Reference LDO */
void
si_pmu_switch_on_PARLDO(si_t *sih, osl_t *osh)
{
	uint32 mask;
	pmuregs_t *pmu;
	uint origidx;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	switch (CHIPID(sih->chip)) {
	CASE_BCM43602_CHIP:
		mask = R_REG(osh, &pmu->min_res_mask) | PMURES_BIT(RES43602_PARLDO_PU);
		W_REG(osh, &pmu->min_res_mask, mask);
		mask = R_REG(osh, &pmu->max_res_mask) | PMURES_BIT(RES43602_PARLDO_PU);
		W_REG(osh, &pmu->max_res_mask, mask);
		break;
	default:
		break;
	}
	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

/* For 43602a0 MCH2/MCH5 boards: power off PA Reference LDO */
void
si_pmu_switch_off_PARLDO(si_t *sih, osl_t *osh)
{
	uint32 mask;
	pmuregs_t *pmu;
	uint origidx;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM43602_CHIP_ID:
	case BCM43462_CHIP_ID:
		mask = R_REG(osh, &pmu->min_res_mask) & ~PMURES_BIT(RES43602_PARLDO_PU);
		W_REG(osh, &pmu->min_res_mask, mask);
		mask = R_REG(osh, &pmu->max_res_mask) & ~PMURES_BIT(RES43602_PARLDO_PU);
		W_REG(osh, &pmu->max_res_mask, mask);
		break;
	default:
		break;
	}
	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

#if defined(DONGLEBUILD)

/** DONGLEBUILD specific */
#define PMU_4365_CC1_CPUPLL_LOAD_SHIFT		6
#define PMU_4365_CC1_CPUPLL_LOAD_MASK		(0x7 << PMU_4365_CC1_CPUPLL_LOAD_SHIFT)

#define PMU_4365_CC3_CA7_CCI400_DIV_SHIFT    0
#define PMU_4365_CC3_CA7_CCI400_DIV_MASK   (0xf << PMU_4365_CC3_CA7_CCI400_DIV_SHIFT)
#define PMU_4365_CC3_UPDATE_EXT_DIV_SHIFT    8
#define PMU_4365_CC3_UPDATE_EXT_DIV_MASK   (0x1 << PMU_4365_CC3_UPDATE_EXT_DIV_SHIFT)
#define PMU_4365_CC3_PLL_DIV_ARM_SHIFT      11
#define PMU_4365_CC3_PLL_DIV_ARM_MASK      (0xf << PMU_4365_CC3_PLL_DIV_ARM_SHIFT)
#define PMU_4365_CC3_PLL_DIV_CCI400_SHIFT   15
#define PMU_4365_CC3_PLL_DIV_CCI400_MASK   (0xf << PMU_4365_CC3_PLL_DIV_CCI400_SHIFT)
#define PMU_4365_CC3_PLL_DIV_BP_SHIFT       19
#define PMU_4365_CC3_PLL_DIV_BP_MASK       (0xf << PMU_4365_CC3_PLL_DIV_BP_SHIFT)

/* Adjust 4365 cpu/cci/bp clock ratio */
static void
BCMATTACHFN(si_set_clock_ratio_4365)(si_t *sih, osl_t *osh, uint32 cpu_cci_ratio,
	uint32 cci_bp_ratio)
{
	volatile uint32 *ca7_regs;
	uint32 val;
	uint origidx;

	origidx = si_coreidx(sih);

	ca7_regs = si_setcore(sih, ARMCA7_CORE_ID, 0);
	if (ca7_regs) {
		/* Configured CPU/CCI/Backplane clocks ratio */
		val = R_REG(osh, ca7_regs);
		val &= ~(0x3 << 12);
		val |= (cci_bp_ratio << 12);
		W_REG(osh, ca7_regs, val);
		val = R_REG(osh, ca7_regs);
		val &= ~(0x7 << 9);
		val |= (cpu_cci_ratio << 9);
		W_REG(osh, ca7_regs, val);
	}

	si_setcoreidx(sih, origidx);
} /* si_set_clock_ratio_4365 */

/**
 * 4365 default cpu/cci/nic clock is 200/200/200MHz and need FW to boost it to 800/400/200.
 * sets the armca7, cci400 and backplane clocks
 */
static void
BCMATTACHFN(si_set_clocks_4365_c0)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 save_clk;
	uint origidx;
	chipcregs_t *cc;
	uint32 val;

	/* Force HT */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	save_clk = R_REG(osh, &cc->clk_ctl_st);
	/* Force HT Request */
	OR_REG(osh, &cc->clk_ctl_st, CCS_FORCEHT);
	/* Check BP clock is on HT */
	while (!(R_REG(osh, &cc->clk_ctl_st) & CCS_BP_ON_HT)) {
		OSL_DELAY(10);
	}
	si_setcoreidx(sih, origidx);

	printf("Setting clocks to 800/400/200\n");
	val = si_pmu_chipcontrol(sih, PMU_CHIPCTL3, 0, 0);
	val &= ~(PMU_4365_CC3_CA7_CCI400_DIV_MASK | PMU_4365_CC3_PLL_DIV_ARM_MASK |
		PMU_4365_CC3_PLL_DIV_CCI400_MASK | PMU_4365_CC3_PLL_DIV_BP_MASK);
	val |= ((0x2 << PMU_4365_CC3_CA7_CCI400_DIV_SHIFT) |
		(0x1 << PMU_4365_CC3_PLL_DIV_ARM_SHIFT) |
		(0x2 << PMU_4365_CC3_PLL_DIV_CCI400_SHIFT) |
		(0x4 << PMU_4365_CC3_PLL_DIV_BP_SHIFT));
	val |= PMU_4365_CC3_UPDATE_EXT_DIV_MASK;
	si_pmu_chipcontrol(sih, PMU_CHIPCTL3, ~0, val);
	si_pmu_chipcontrol(sih, PMU_CHIPCTL3, PMU_4365_CC3_UPDATE_EXT_DIV_MASK, 0);

	/* Restore the clk_ctl_st after changing the VCO */
	cc = si_setcoreidx(sih, SI_CC_IDX);
	W_REG(osh, &cc->clk_ctl_st, save_clk);
	si_setcoreidx(sih, origidx);
} /* si_set_clocks_4365_c0 */

/**
 * Make the CPU/CCI/backplane clocks configurable with NVRAM. Called for chiprev b1 and lower.
 * DONGLEBUILD specific.
 */
static void
BCMATTACHFN(si_set_clocks_4365)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 oldpmucontrol, save_clk;
	uint origidx;
	chipcregs_t *cc;
	int cpuclk;
	uint32 vco_val;

	cpuclk = getintvar(NULL, "cpuclk");
	/* Only allows 720/360/180 or 800/400/200 (default) for now. */
	if (cpuclk == 720) {
		vco_val = 0x241;
		printf("Setting clocks to 720/360/180\n");
	} else {
		vco_val = 0x281;
		printf("Setting clocks to 800/400/200\n");
	}

	/* Disable HT works on bp clock */
	W_REG(osh, &pmu->max_res_mask, 0xff);
	W_REG(osh, &pmu->min_res_mask, 0xff);

	/* save clk_ctl_st before changing VCO */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	save_clk = R_REG(osh, &cc->clk_ctl_st);
	/* Force ALP Request */
	OR_REG(osh, &cc->clk_ctl_st, CCS_FORCEALP);
	/* Check BP clock is on ALP */
	while (!(R_REG(osh, &cc->clk_ctl_st) & CCS_BP_ON_ALP)) {
		OSL_DELAY(10);
	}
	si_setcoreidx(sih, origidx);

	/* Set ndiv_int = 32, pdiv = 1 vco = 1600 or 1440 MHz */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG9, ~0, vco_val);
	/* Set ch0_mdiv = 2, ch1_mdiv =4, ch2_mdiv = 8 */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG11, ~0, 0x00080402);
	/* set areset[bit3] to 1, and dreset[bit4] to 1 */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, ~0, 0x53600e0c);
	/* Set pllCtrlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, oldpmucontrol | (1<<10));
	OSL_DELAY(10);

	/* set areset[bit3] to 0, and dreset[bit4] to 1 */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, ~0, 0x53600e08);
	/* Set pllCtrlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, oldpmucontrol | (1<<10));
	OSL_DELAY(50);

	/* set areset[bit3] to 0, and dreset[bit4] to 0 */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, ~0, 0x53600000);
	/* Set pllCtrlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, oldpmucontrol | (1<<10));

	/* max_res_mask, min_res_mask -> 0x3ff */
	W_REG(osh, &pmu->max_res_mask, 0x3ff);
	W_REG(osh, &pmu->min_res_mask, 0x3ff);

	/* Force HT */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	save_clk = R_REG(osh, &cc->clk_ctl_st);
	/* Force HT Request */
	OR_REG(osh, &cc->clk_ctl_st, CCS_FORCEHT);
	/* Check BP clock is on HT */
	while (!(R_REG(osh, &cc->clk_ctl_st) & CCS_BP_ON_HT)) {
		OSL_DELAY(10);
	}

	/* set CCI:BP clock ratio to 2:1(value 0x1) and CPU:CCI clock ratio to 2:1(value 0x1) */
	si_set_clock_ratio_4365(sih, osh, 1, 1);

	si_setcoreidx(sih, origidx);

	/* Restore the clk_ctl_st after changing the VCO */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	W_REG(osh, &cc->clk_ctl_st, save_clk);
	si_setcoreidx(sih, origidx);
} /* si_set_clocks_4365 */

#else /* !DONGLEBUILD */

/**
 * XXX: WAR for CRWLPCIEGEN2-326, called for PCIeRev15. Not called for 4365 rev > a0.
 * Change 4365A0 VCO frequency to 1280 MHz, and BP clock to 160 MHz
 */
static void
BCMATTACHFN(si_set_bb_vcofreq_4365)(si_t *sih, osl_t *osh, pmuregs_t *pmu)
{
	uint32 oldpmucontrol, save_clk;
	uint origidx;
	chipcregs_t *cc;

	/* Disable HT works on bp clock */
	W_REG(osh, &pmu->max_res_mask, 0xff);
	W_REG(osh, &pmu->min_res_mask, 0xff);

	/* save clk_ctl_st before changing VCO */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	save_clk = R_REG(osh, &cc->clk_ctl_st);
	/* Force ALP Request */
	OR_REG(osh, &cc->clk_ctl_st, CCS_FORCEALP);
	/* Check BP clock is on ALP */
	while (!(R_REG(osh, &cc->clk_ctl_st) & CCS_BP_ON_ALP)) {
		OSL_DELAY(10);
	}
	si_setcoreidx(sih, origidx);

	/* Set ndiv_int = 32, pdiv = 1 vco = 1280 MHz */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG9, ~0, 0x201);
	/* Set ch0_mdiv = 2, ch1_mdiv =4, ch2_mdiv = 8 */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG11, ~0, 0x00080402);
	/* set areset[bit3] to 1, and dreset[bit4] to 1 */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, ~0, 0x53600e0c);
	/* Set pllCtrlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, oldpmucontrol | (1<<10));
	OSL_DELAY(10);

	/* set areset[bit3] to 0, and dreset[bit4] to 1 */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, ~0, 0x53600e08);
	/* Set pllCtrlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, oldpmucontrol | (1<<10));
	OSL_DELAY(50);

	/* set areset[bit3] to 0, and dreset[bit4] to 0 */
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG7, ~0, 0x53600000);
	/* Set pllCtrlUpdate */
	oldpmucontrol = R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, oldpmucontrol | (1<<10));

	/* max_res_mask, min_res_mask -> 0x3ff */
	W_REG(osh, &pmu->max_res_mask, 0x3ff);
	W_REG(osh, &pmu->min_res_mask, 0x3ff);

	/* Restore the clk_ctl_st after changing the VCO */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	W_REG(osh, &cc->clk_ctl_st, save_clk);
	si_setcoreidx(sih, origidx);
} /* si_set_bb_vcofreq_4365 */

#endif /* !defined(DONGLEBUILD) */

/**
 * Change VCO frequency (slightly), e.g. to avoid PHY errors due to spurs.
 */
static void
BCMATTACHFN(si_set_bb_vcofreq_frac)(si_t *sih, osl_t *osh, int vcofreq, int frac, int xtalfreq)
{
	uint32 vcofreq_withfrac, p1div, ndiv_int, fraca, ndiv_mode, reg;
	/* shifts / masks for PMU PLL control register #2 : */
	uint32 ndiv_int_shift, ndiv_mode_shift, p1div_shift, pllctrl2_mask;
	/* shifts / masks for PMU PLL control register #3 : */
	uint32 pllctrl3_mask;
	BCM_REFERENCE(osh);

	if ((CHIPID(sih->chip) == BCM4360_CHIP_ID) ||
	    (CHIPID(sih->chip) == BCM43460_CHIP_ID) ||
	    (CHIPID(sih->chip) == BCM43526_CHIP_ID) ||
	    (CHIPID(sih->chip) == BCM4352_CHIP_ID) ||
	    BCM43602_CHIP(sih->chip)) {
		if (si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, clk_ctl_st), 0, 0)
			& CCS_HTAVAIL) {
			PMU_MSG(("HTAVAIL is set, so not updating BBPLL Frequency \n"));
			return;
		}

		ndiv_int_shift = 7;
		ndiv_mode_shift = 4;
		p1div_shift = 0;
		pllctrl2_mask = 0xffffffff;
		pllctrl3_mask = 0xffffffff;
	} else if (BCM4350_CHIP(sih->chip) &&
		(CST4350_IFC_MODE(sih->chipst) == CST4350_IFC_MODE_PCIE)) {
		ndiv_int_shift = 23;
		ndiv_mode_shift = 20;
		p1div_shift = 16;
		pllctrl2_mask = 0xffff0000;
		pllctrl3_mask = 0x00ffffff;
	} else {
		/* put more chips here */
		PMU_ERROR(("%s: only work on 4360, 4350\n", __FUNCTION__));
		return;
	}

	vcofreq_withfrac = vcofreq * 10000 + frac;
	p1div = 0x1;
	ndiv_int = vcofreq / xtalfreq;
	ndiv_mode = (vcofreq_withfrac % (xtalfreq * 10000)) ? 3 : 0;
	PMU_ERROR(("ChangeVCO => vco:%d, xtalF:%d, frac: %d, ndivMode: %d, ndivint: %d\n",
		vcofreq, xtalfreq, frac, ndiv_mode, ndiv_int));

	reg = (ndiv_int << ndiv_int_shift) |
	      (ndiv_mode << ndiv_mode_shift) |
	      (p1div << p1div_shift);
	PMU_ERROR(("Data written into the PLL_CNTRL_ADDR2: %08x\n", reg));
	si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, pllctrl2_mask, reg);

	if (ndiv_mode) {
		/* frac = (vcofreq_withfrac % (xtalfreq * 10000)) * 2^24) / (xtalfreq * 10000) */
		uint32 r1, r0;
		math_uint64_multiple_add(
			&r1, &r0, vcofreq_withfrac % (xtalfreq * 10000), 1 << 24, 0);
		math_uint64_divide(&fraca, r1, r0, xtalfreq * 10000);
		PMU_ERROR(("Data written into the PLL_CNTRL_ADDR3 (Fractional): %08x\n", fraca));
		si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, pllctrl3_mask, fraca);
	}

	si_pmu_pllupd(sih);
} /* si_set_bb_vcofreq_frac */

/**
 * Given x-tal frequency, returns BaseBand vcofreq with fraction in 100Hz. Only called for specific
 * chips.
 * @param   xtalfreq In [Mhz] units.
 * @return           In [100Hz] units.
 */
uint32
si_pmu_get_bb_vcofreq(si_t *sih, osl_t *osh, int xtalfreq)
{
	uint32  ndiv_int,	/* 9 bits integer divider */
		ndiv_mode,
		frac = 0,	/* 24 bits fractional divider */
		p1div;		/* predivider: divides x-tal freq */
	uint32 xtal1, vcofrac = 0, vcofreq;
	uint32 r1, r0, reg;

	BCM_REFERENCE(osh);

	if ((CHIPID(sih->chip) == BCM4360_CHIP_ID) ||
	    (CHIPID(sih->chip) == BCM43460_CHIP_ID) ||
	    (CHIPID(sih->chip) == BCM43526_CHIP_ID) ||
	    (CHIPID(sih->chip) == BCM4352_CHIP_ID) ||
	    BCM43602_CHIP(sih->chip)) {
		reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, 0, 0);
		ndiv_int = reg >> 7;
		ndiv_mode = (reg >> 4) & 7;
		p1div = 1; /* do not divide x-tal frequency */

		if (ndiv_mode)
			frac = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, 0, 0);
	} else if ((BCM4350_CHIP(sih->chip) &&
		(CST4350_IFC_MODE(sih->chipst) == CST4350_IFC_MODE_PCIE)) ||
		/* Include only these 4350 USB chips */
		((BCM4347_CHIP(sih->chip)) &&
		(CST4347_CHIPMODE_PCIE(sih->chipst))) ||
		(BCM53573_CHIP(sih->chip) &&
		(CST53573_CHIPMODE_PCIE(sih->chipst))) ||
		0) {
		reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, 0, 0);
		ndiv_int = reg >> 23;
		ndiv_mode = (reg >> 20) & 7;
		p1div = (reg >> 16) & 0xf;

		if (ndiv_mode)
			frac = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, 0, 0) & 0x00ffffff;

	} else if (BCM4369_CHIP(sih->chip) &&
			CST4369_CHIPMODE_PCIE(sih->chipst)) {
		reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, 0, 0);
		ndiv_int = reg >> 20;
		p1div = (reg >> 16) & 0xf;
		frac = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, 0, 0) & 0x00fffff;
		ndiv_mode = 1;

	} else {
		/* put more chips here */
		PMU_ERROR(("%s: only work on 4360, 4350\n", __FUNCTION__));
		ASSERT(FALSE);
		return 0;
	}

	xtal1 = 10000 * xtalfreq / p1div;		/* in [100Hz] units */

	if (ndiv_mode) {
		/* vcofreq fraction = (xtal1 * frac + (1 << 23)) / (1 << 24);
		 * handle overflow
		 */
		math_uint64_multiple_add(&r1, &r0, xtal1, frac, 1 << 23);
		vcofrac = (r1 << 8) | (r0 >> 24);
	}

	if (ndiv_int == 0) {
		ASSERT(0);
		return 0;
	}

	if ((int)xtal1 > (int)((0xffffffff - vcofrac) / ndiv_int)) {
		PMU_ERROR(("%s: xtalfreq is too big, %d\n", __FUNCTION__, xtalfreq));
		return 0;
	}

	vcofreq = xtal1 * ndiv_int + vcofrac;
	return vcofreq;
} /* si_pmu_get_bb_vcofreq */

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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	/* twiki PmuRev30, OneMhzToggleEn:31, AlpPeriod[23:0] */
	if (PMUREV(sih->pmurev) >= 30) {
		/* AlpPeriod = ROUND(POWER(2,26)/ALP_CLK_FREQ_IN_MHz,0) */
		/* Calculation will be accurate for only one decimal of xtal (like 37.4),
		* and will not be accurate for more than one decimal of xtal freq (like 37.43)
		* Also no rounding is done on final result
		*/
		ROMMABLE_ASSERT((xtalfreq/100)*100 == xtalfreq);
		val = (((1 << 26)*10)/(xtalfreq/100));
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

	W_REG(osh, &pmu->slowclkperiod, val);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

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
#endif // endif
	BCM_REFERENCE(pmu1_xtaltab0_880);
	BCM_REFERENCE(pmu1_xtaltab0_1760);
	BCM_REFERENCE(bcm4350_res_depend);

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM4352_CHIP_ID: {
		if (CHIPREV(sih->chiprev) > 2)
			si_set_bb_vcofreq_frac(sih, osh, 960, 98, 40);
		break;
	}
	CASE_BCM43602_CHIP:
		si_set_bb_vcofreq_frac(sih, osh, 960, 98, 40);
		break;
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
#if defined(DONGLEBUILD)
		/* Due to HW limitation, we need to set CCI:BP clock ratio to 1:1(value 0x0)
		 * and CPU:CCI clock ratio to 1:1(value 0x0), before boost to higher CPU:CCI:BP rate
		 */
		si_set_clock_ratio_4365(sih, osh, 0, 0);

		/* Set CPUPLL Load */
		si_pmu_chipcontrol(sih, PMU_CHIPCTL1, PMU_4365_CC1_CPUPLL_LOAD_MASK,
			PMU_4365_CC1_CPUPLL_LOAD_MASK);
		/* Clear CPUPLL Load */
		si_pmu_chipcontrol(sih, PMU_CHIPCTL1, PMU_4365_CC1_CPUPLL_LOAD_MASK, 0);

		if (CHIPREV(sih->chiprev) >= 4)
			si_set_clocks_4365_c0(sih, osh, pmu);
		else
			si_set_clocks_4365(sih, osh, pmu);
#else
		if (sih->buscorerev == 15) {
			si_set_bb_vcofreq_4365(sih, osh, pmu); /* WAR for a0 chips */
		}
#endif /* DONGLEBUILD */
		si_set_bb_vcofreq_frac(sih, osh, 1600, 0, 40);
		break;
	case BCM43217_CHIP_ID:
		break;
	case BCM43684_CHIP_ID:
#if defined(DONGLEBUILD)
		si_set_cpu_clock_43684(sih, osh, pmu);
#endif /* DONGLEBUILD */
		break;
	case BCM6715_CHIP_ID:
#if defined(DONGLEBUILD)
		si_set_cpu_clock_6715(sih, osh, pmu);
#endif /* DONGLEBUILD */
		break;
	case BCM4347_CHIP_GRPID:
		si_pmu1_pllinit1(sih, osh, pmu, xtalfreq); /* nvram PLL overrides + enables PLL */
		break;
	case BCM43570_CHIP_ID:
		si_pmu1_pllinit1(sih, osh, pmu, xtalfreq);
		if (xtalfreq == 40000)
			/* VCO 960.0098MHz does not produce spurs in 2G
				and it does not suffer from Tx FIFO underflows
			*/
			si_set_bb_vcofreq_frac(sih, osh, 960, 98, 40);
		break;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
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

#if defined(DONGLEBUILD) && defined(__ARM_ARCH_7A__)
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

/** get alp clock frequency in [Hz] units */
uint32
BCMINITFN(si_pmu_alp_clock)(si_t *sih, osl_t *osh)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 clock = ALP_CLOCK;
#ifdef BCMDBG
	char chn[8];
#endif // endif

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID:
		if (sih->chipst & CST4360_XTAL_40MZ)
			clock = 40000 * 1000;
		else
			clock = 20000 * 1000;
		break;

	CASE_BCM43602_CHIP:
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
	case BCM53573_CHIP_GRPID:
		/* always 40Mhz */
		clock = 40000 * 1000;
		break;
	case BCM43217_CHIP_ID:
		/* always 20Mhz */
		clock = 20000 * 1000;
		break;
	case BCM43570_CHIP_ID:
	case BCM4347_CHIP_GRPID:
	CASE_BCM43684_CHIP: /* alp clk == xtal clk */
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
	CASE_BCM6715_CHIP:
		clock = si_pmu1_alpclk0(sih, osh, pmu);
		break;
	CASE_BCM6710_CHIP:
		/* always 50Mhz */
		clock = 50000 * 1000;
		break;
	default:
		PMU_MSG(("No ALP clock specified "
			"for chip %s rev %d pmurev %d, using default %d Hz\n",
			bcm_chipname(
			CHIPID(sih->chip), chn, 8), CHIPREV(sih->chiprev),
			PMUREV(sih->pmurev), clock));
		break;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return clock; /* in [Hz] units */
} /* si_pmu_alp_clock */

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
	fc = si_pmu_alp_clock(sih, osh) / 1000000;
	fc = (p1 * ndiv * fc) / p2;

	PMU_NONE(("%s: p1=%d, p2=%d, ndiv=%d(0x%x), m%d=%d; fc=%d, clock=%d\n",
	          __FUNCTION__, p1, p2, ndiv, ndiv, m, div, fc, fc / div));

	/* Return clock in Hertz */
	return ((fc / div) * 1000000);
} /* si_pmu5_clock */

/**
 * Get backplane clock frequency, returns a value in [hz] units.
 * For designs that feed the same clock to both backplane and CPU just return the CPU clock speed.
 */
uint32
BCMINITFN(si_pmu_si_clock)(si_t *sih, osl_t *osh)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 clock = HT_CLOCK;	/* in [hz] units */
#ifdef BCMDBG
	char chn[8];
#endif // endif

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM6362_CHIP_ID:
		/* 96MHz backplane clock */
		clock = 96000 * 1000;
		break;
	case BCM4360_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM43526_CHIP_ID:
	case BCM4352_CHIP_ID:
		clock = si_pmu1_cpuclk0(sih, osh, pmu);
		break;
	CASE_BCM43602_CHIP: {
			uint32 mdiv;
			/* Ch3 is connected to backplane_clk. Read 'bbpll_i_m3div' from pllctl[4] */
			mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, 0, 0);
			mdiv = (mdiv & PMU1_PLL0_PC1_M3DIV_MASK) >> PMU1_PLL0_PC1_M3DIV_SHIFT;
			ASSERT(mdiv != 0);
			clock = si_pmu1_pllfvco0(sih) / mdiv * 1000;
			break;
		}

	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
		{ /* BP clock is derived from CPU PLL */
			uint32 mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG11, 0, 0);
			if (CHIPREV(sih->chiprev) < 4) {
				mdiv = (mdiv &
					PMU1_PLL0_PC1_M3DIV_MASK) >> PMU1_PLL0_PC1_M3DIV_SHIFT;
				clock = si_pmu1_pllfvco1(sih) / mdiv * 1000;
			} else {
				mdiv = (mdiv &
					PMU1_PLL0_PC1_M1DIV_MASK) >> PMU1_PLL0_PC1_M1DIV_SHIFT;
				clock = si_pmu1_pllfvco1(sih) / (mdiv * 4) * 1000;
			}
			break;
		}

	CASE_BCM43684_CHIP:
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
		{ /* BP clock is derived from BB PLL, channel 1 */
			uint32 mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, 0, 0);
			mdiv = (mdiv &
				PMU1_PLL0_PC1_M2DIV_MASK) >> PMU1_PLL0_PC1_M2DIV_SHIFT;
			ASSERT(mdiv != 0);
			clock = si_pmu1_pllfvco0(sih) / mdiv * 1000;
		}
		break;

	CASE_BCM6715_CHIP:
		{ /* BP clock is derived from BB PLL, channel 1 */
			uint32 mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, 0, 0);
			mdiv = (mdiv &
				PMU6715_PLL0_PC3_M2DIV_MASK) >> PMU6715_PLL0_PC3_M2DIV_SHIFT;
			ASSERT(mdiv != 0);
			clock = si_pmu1_pllfvco0(sih) / mdiv * 1000;
		}
		break;

	case BCM4347_CHIP_GRPID:
		clock = si_pmu1_cpuclk0(sih, osh, pmu);
		break;

	case BCM53573_CHIP_GRPID: {
		uint32 fvco, pdiv, pll_ctrl_20, mdiv, ndiv_nfrac, r_high, r_low;
		uint32 xtal_freq = si_alp_clock(sih);

		/* Read mdiv, pdiv from pllcontrol[13], ch2 for NIC400 */
		mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG13, 0, 0);
		mdiv = (mdiv >> 16) & 0xff;
		ASSERT(mdiv);
		pdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG13, 0, 0);
		pdiv = (pdiv >> 24) & 0x7;
		ASSERT(pdiv);

		/* Read ndiv[29:20], ndiv_frac[19:0] from pllcontrol[14] */
		ndiv_nfrac = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG14, 0, 0) & 0x3fffffff;

		/* Read pll_ctrl_20 from pllcontrol[15] */
		pll_ctrl_20 = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG15, 0, 0);
		pll_ctrl_20 = 1 << ((pll_ctrl_20 >> 20) & 0x1);

		math_uint64_multiple_add(&r_high, &r_low, ndiv_nfrac, (xtal_freq * pll_ctrl_20), 0);
		math_uint64_right_shift(&fvco, r_high, r_low, 20);

		clock = (fvco / pdiv)/ mdiv;
		break;
	}

	default:
		PMU_MSG(("No backplane clock specified "
			"for chip %s rev %d pmurev %d, using default %d Hz\n",
			bcm_chipname(
			CHIPID(sih->chip), chn, 8), CHIPREV(sih->chiprev),
			PMUREV(sih->pmurev), clock));
		break;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return clock;
} /* si_pmu_si_clock */

/** returns CPU clock frequency in [hz] units */
uint32
BCMINITFN(si_pmu_cpu_clock)(si_t *sih, osl_t *osh)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 clock;	/* in [hz] units */

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	if ((PMUREV(sih->pmurev) >= 5) &&
		!((CHIPID(sih->chip) == BCM4360_CHIP_ID) ||
		(CHIPID(sih->chip) == BCM4352_CHIP_ID) ||
		(CHIPID(sih->chip) == BCM43526_CHIP_ID) ||
		(CHIPID(sih->chip) == BCM43460_CHIP_ID) ||
		BCM4350_CHIP(sih->chip) ||
		(BCM4347_CHIP(sih->chip)) ||
		0)) {
		uint pll = PMU4716_MAINPLL_PLL0;

		if (BCM4365_CHIP(sih->chip) ||
			BCM43602_CHIP(sih->chip) ||
			BCM43684_CHIP(sih->chip) ||
			BCM6715_CHIP(sih->chip))
			clock = si_pmu1_cpuclk0(sih, osh, pmu);
		else
			clock = si_pmu5_clock(sih, osh, pmu, pll, PMU5_MAINPLL_CPU);
	} else if (BCM4347_CHIP(sih->chip) ||
		BCM4369_CHIP(sih->chip)) {
		clock = si_pmu1_cpuclk0_pll2(sih, osh, pmu); /* for chips with separate CPU PLL */
	} else {
		clock = si_pmu_si_clock(sih, osh);
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	return clock;
} /* si_pmu_cpu_clock */

/**
 * Gets memory clock frequency, which is the same as the HT clock for newer chips. Returns [Hz].
 * Only called for firmware builds.
 */
uint32
BCMINITFN(si_pmu_mem_clock)(si_t *sih, osl_t *osh)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 clock;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	if ((PMUREV(sih->pmurev) >= 5) &&
		!(BCM4365_CHIP(sih->chip) ||
		(BCM4347_CHIP(sih->chip)) ||
		BCM43684_CHIP(sih->chip) ||
		BCM6715_CHIP(sih->chip) ||
		BCM43602_CHIP(sih->chip) ||
		BCM4350_CHIP(sih->chip))) {
		uint pll = PMU4716_MAINPLL_PLL0;

		clock = si_pmu5_clock(sih, osh, pmu, pll, PMU5_MAINPLL_MEM);
	} else if (BCM4365_CHIP(sih->chip)) {
		uint32 mdiv = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG11, 0, 0);
		ASSERT(mdiv != 0);
		if (CHIPREV(sih->chiprev) < 4) {
			mdiv = (mdiv &
				PMU1_PLL0_PC1_M2DIV_MASK) >> PMU1_PLL0_PC1_M2DIV_SHIFT;
			clock = si_pmu1_pllfvco1(sih) / mdiv * 1000;
		} else {
			mdiv = (mdiv &
				PMU1_PLL0_PC1_M1DIV_MASK) >> PMU1_PLL0_PC1_M1DIV_SHIFT;
			clock = si_pmu1_pllfvco1(sih) / (mdiv * 2) * 1000;
		}
	} else {
		clock = si_pmu_si_clock(sih, osh); /* mem clk same as backplane clk */
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	return clock;
} /* si_pmu_mem_clock */

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

/**
 * Measure ILP clock frequency. Returns a value in [Hz] units.
 *
 * XXX The variable ilpcycles_per_sec is used to store the ILP clock speed. The value
 * is calculated when the function is called the first time and then cached.
 * The change in PMU timer count is measured across a delay of ILP_CALC_DUR msec.
 * Before the first time the function is called, one must make sure the HT clock is
 * turned on and used to feed the CPU and that OSL_DELAY() is calibrated.
 */
uint32
BCMINITFN(si_pmu_ilp_clock)(si_t *sih, osl_t *osh)
{
	if (ISSIM_ENAB(sih))
		return ILP_CLOCK;

	if (ilpcycles_per_sec == 0) {
		uint32 start, end, delta;
		pmuregs_t *pmu;
		uint origidx = si_coreidx(sih);

		if (AOB_ENAB(sih)) {
			pmu = si_setcore(sih, PMU_CORE_ID, 0);
		} else {
			pmu = si_setcoreidx(sih, SI_CC_IDX);
		}
		ASSERT(pmu != NULL);
		start = R_REG(osh, &pmu->pmutimer);
		if (start != R_REG(osh, &pmu->pmutimer))
			start = R_REG(osh, &pmu->pmutimer);
		OSL_DELAY(ILP_CALC_DUR * 1000);
		end = R_REG(osh, &pmu->pmutimer);
		if (end != R_REG(osh, &pmu->pmutimer))
			end = R_REG(osh, &pmu->pmutimer);
		delta = end - start;
		ilpcycles_per_sec = delta * (1000 / ILP_CALC_DUR);
		/* Return to original core */
		si_setcoreidx(sih, origidx);
	}

	ASSERT(ilpcycles_per_sec != 0);
	return ilpcycles_per_sec;
}
#endif /* !defined(BCMDONGLEHOST) */

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

#if !defined(BCMDONGLEHOST)
/** initialize PMU */
void
BCMATTACHFN(si_pmu_init)(si_t *sih, osl_t *osh)
{
	pmuregs_t *pmu;
	uint origidx;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	if (PMUREV(sih->pmurev) == 1)
		AND_REG(osh, &pmu->pmucontrol, ~PCTL_NOILP_ON_WAIT);
	else if (PMUREV(sih->pmurev) >= 2)
		OR_REG(osh, &pmu->pmucontrol, PCTL_NOILP_ON_WAIT);

	/* Changes from PMU revision 26 are not included in revision 27 */
	if ((PMUREV(sih->pmurev) >= 26) && (PMUREV(sih->pmurev) != 27)) {
		uint32 val = PMU_INTC_ALP_REQ | PMU_INTC_HT_REQ | PMU_INTC_HQ_REQ;
		pmu_corereg(sih, SI_CC_IDX, pmuintctrl0, val, val);

		val = RSRC_INTR_MASK_TIMER_INT_0;
		pmu_corereg(sih, SI_CC_IDX, pmuintmask0, val, val);
		(void)pmu_corereg(sih, SI_CC_IDX, pmuintmask0, 0, 0);
	}

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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}

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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}

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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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

/**
 * Return worst case up time in [ILP cycles] for the given resource.
 *
 * Example use case: the d11 core needs to be programmed with the max time it
 * takes to make the HT clock available.
 *
 * XXX need to check circular dependancies and prevent dead recursion.
 */
static uint
BCMINITFN(si_pmu_res_uptime)(si_t *sih, osl_t *osh, pmuregs_t *pmu, uint8 rsrc)
{
	uint32 deps;
	uint uptime, i, dup, dmax;
	uint32 min_mask = 0;
#ifndef SR_DEBUG
	uint32 max_mask = 0;
#endif /* SR_DEBUG */

	/* uptime of resource 'rsrc' */
	W_REG(osh, &pmu->res_table_sel, (uint32)rsrc);
	if (PMUREV(sih->pmurev) >= 30)
		uptime = (R_REG(osh, &pmu->res_updn_timer) >> 16) & 0x7fff;
	else if (PMUREV(sih->pmurev) >= 13)
		uptime = (R_REG(osh, &pmu->res_updn_timer) >> 16) & 0x3ff;
	else
		uptime = (R_REG(osh, &pmu->res_updn_timer) >> 8) & 0xff;

	/* direct dependencies of resource 'rsrc' */
	deps = si_pmu_res_deps(sih, osh, pmu, PMURES_BIT(rsrc), FALSE);
	for (i = 0; i <= PMURES_MAX_RESNUM; i ++) {
		if (!(deps & PMURES_BIT(i)))
			continue;
		deps &= ~si_pmu_res_deps(sih, osh, pmu, PMURES_BIT(i), TRUE);
	}
#ifndef SR_DEBUG
	si_pmu_res_masks(sih, &min_mask, &max_mask);
#else
	/* Recalculate fast pwr up delay if min res mask/max res mask has changed */
	min_mask = R_REG(osh, &pmu->min_res_mask);
#endif /* SR_DEBUG */
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
 */
void
si_pmu_otp_power(si_t *sih, osl_t *osh, bool on, uint32* min_res_mask)
{
	pmuregs_t *pmu;
	uint origidx;
	uint32 rsrcs = 0;	/* rsrcs to turn on/off OTP power */
	rsc_per_chip_t *rsc;	/* chip specific resource bit positions */

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Don't do anything if OTP is disabled */
	if (si_is_otp_disabled(sih)) {
		PMU_MSG(("si_pmu_otp_power: OTP is disabled\n"));
		return;
	}

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	/*
	 * OTP can't be power cycled by toggling OTP_PU for always on OTP chips. For now
	 * corerev 45 is the only one that has always on OTP.
	 * Instead, the chipc register OTPCtrl1 (Offset 0xF4) bit 25 (forceOTPpwrDis) is used.
	 * Please refer to http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/ChipcommonRev45
	 */
	if (CCREV(sih->ccrev) == 45) {
		uint32 otpctrl1;
		otpctrl1 = si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, otpcontrol1), 0, 0);
		if (on)
			otpctrl1 &= ~OTPC_FORCE_PWR_OFF;
		else
			otpctrl1 |= OTPC_FORCE_PWR_OFF;
		si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, otpcontrol1), ~0, otpctrl1);
		/* Return to original core */
		si_setcoreidx(sih, origidx);
		return;
	}

	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM43526_CHIP_ID:
	case BCM53573_CHIP_GRPID:
		rsc = si_pmu_get_rsc_positions(sih);
		rsrcs = PMURES_BIT(rsc->otp_pu);
		break;
	default:
		break;
	}

	if (rsrcs != 0) {
		uint32 otps;
		bool on_check = FALSE; /* Stores otp_ready state */
		uint32 min_mask = 0;

		/* Turn on/off the power */
		if (on) {
			min_mask = R_REG(osh, &pmu->min_res_mask);
			*min_res_mask = min_mask;

			min_mask |= rsrcs;
			min_mask |= si_pmu_res_deps(sih, osh, pmu, min_mask, TRUE);
			on_check = TRUE;
			/* Assuming max rsc mask defines OTP_PU, so not programming max */
			PMU_MSG(("Adding rsrc 0x%x to min_res_mask\n", min_mask));
			W_REG(osh, &pmu->min_res_mask, min_mask);
			si_pmu_wait_for_steady_state(sih, osh, pmu);
			OSL_DELAY(1000);
			SPINWAIT(!(R_REG(osh, &pmu->res_state) & rsrcs),
				PMU_MAX_TRANSITION_DLY);
			ASSERT(R_REG(osh, &pmu->res_state) & rsrcs);
		} else {
			/*
			 * Restore back the min_res_mask,
			 * but keep OTP powered off if allowed by dependencies
			 */
			if (*min_res_mask)
				min_mask = *min_res_mask;
			else
				min_mask = R_REG(osh, &pmu->min_res_mask);

			min_mask &= ~rsrcs;
			/*
			 * OTP rsrc can be cleared only if its not
			 * in the dependency of any "higher" rsrc in min_res_mask
			 */
			min_mask |= si_pmu_res_deps(sih, osh, pmu, min_mask, TRUE);
			on_check = ((min_mask & rsrcs) != 0);

			PMU_MSG(("Removing rsrc 0x%x from min_res_mask\n", min_mask));
			W_REG(osh, &pmu->min_res_mask, min_mask);
			si_pmu_wait_for_steady_state(sih, osh, pmu);
		}

		if (AOB_ENAB(sih)) {
			SPINWAIT((((otps =
				si_corereg(sih, si_findcoreidx(sih, GCI_CORE_ID, 0),
				OFFSETOF(gciregs_t, otpstatus), 0, 0))
				& OTPS_READY) != (on_check ? OTPS_READY : 0)), 3000);
		} else {
			SPINWAIT((((otps =
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, otpstatus), 0, 0))
			& OTPS_READY) != (on_check ? OTPS_READY : 0)), 3000);
		}
		ASSERT((otps & OTPS_READY) == (on_check ? OTPS_READY : 0));
		if ((otps & OTPS_READY) != (on_check ? OTPS_READY : 0))
			PMU_MSG(("OTP ready bit not %s after wait\n", (on_check ? "ON" : "OFF")));
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
} /* si_pmu_otp_power */

void
si_pmu_spuravoid(si_t *sih, osl_t *osh, uint8 spuravoid)
{
	pmuregs_t *pmu;
	uint origidx, intr_val = 0;
	uint32 min_res_mask = 0, max_res_mask = 0, clk_ctl_st = 0;
	bool pll_off_on = FALSE;

	if (BCM53573_CHIP(sih->chip))
	{
		pll_off_on = TRUE;
	}

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	/* force the HT off  */
	if (pll_off_on)
		si_pmu_pll_off(sih, osh, pmu, &min_res_mask, &max_res_mask, &clk_ctl_st);

	/* update the pll changes */
	si_pmu_spuravoid_pllupdate(sih, pmu, osh, spuravoid);

	/* enable HT back on  */
	if (pll_off_on)
		si_pmu_pll_on(sih, osh, pmu, min_res_mask, max_res_mask, clk_ctl_st);

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
	bool pll_off_on = FALSE;

	if (BCM53573_CHIP(sih->chip)) {
		pll_off_on = TRUE;
	}
	/* Block ints and save current core */
	intr_val = si_introff(sih);
	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	if (pll_off_on)
		si_pmu_pll_off_isdone(sih, osh, pmu);
	/* update the pll changes */
	si_pmu_spuravoid_pllupdate(sih, pmu, osh, spuravoid);

	/* enable HT back on  */
	if (pll_off_on)
		si_pmu_pll_on(sih, osh, pmu, min_res_mask, max_res_mask, clk_ctl_st);

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
#endif // endif

	switch (CHIPID(sih->chip)) {
	case BCM43217_CHIP_ID:
		/* LCNXN */
		/* PLL Settings for spur avoidance on/off mode, no on2 support for 43228A0 */
		if (spuravoid == 1) {
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, ~0, 0x01100014);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, ~0, 0x040C0C06);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, ~0, 0x03140A08);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x00333333);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x202C2820);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG5, ~0, 0x88888815);
		} else {
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG0, ~0, 0x11100014);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG1, ~0, 0x040c0c06);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, ~0, 0x03000a08);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, ~0, 0x00000000);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG4, ~0, 0x200005c0);
			si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG5, ~0, 0x88888815);
		}
		tmp = 1 << 10;
		break;
	case BCM53573_CHIP_GRPID:
		printf("Error. spur table not populated\n");
		break;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
		/* 63178 / 47622 */
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

	default:
		PMU_ERROR(("%s: unknown spuravoidance settings for chip %s, not changing PLL\n",
		           __FUNCTION__, bcm_chipname(CHIPID(sih->chip), chn, 8)));
		break;
	}

	tmp |= R_REG(osh, &pmu->pmucontrol);
	W_REG(osh, &pmu->pmucontrol, tmp);
} /* si_pmu_spuravoid_pllupdate */

extern uint32
si_pmu_cal_fvco(si_t *sih, osl_t *osh)
{
	uint32 xf, ndiv_int, ndiv_frac, fvco, pll_reg, p1_div_scale;
	uint32 r_high, r_low, int_part, frac_part, rounding_const;
	uint8 p1_div;
	chipcregs_t *cc;
	uint origidx, intr_val;
#ifdef BCMDBG
	char chn[8];
#endif // endif

	/* Remember original core before switch to chipc */
	cc = (chipcregs_t *)si_switch_core(sih, CC_CORE_ID, &origidx, &intr_val);
	ASSERT(cc != NULL);
	BCM_REFERENCE(cc);

	xf = si_pmu_alp_clock(sih, osh)/1000;

	switch (CHIPID(sih->chip)) {
		case BCM53573_CHIP_GRPID:
			pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG2, 0, 0);

			p1_div = (pll_reg & PMU4335_PLL0_PC2_P1DIV_MASK) >>
					PMU4335_PLL0_PC2_P1DIV_SHIFT;

			ndiv_int = (pll_reg & PMU4335_PLL0_PC2_NDIV_INT_MASK) >>
					PMU4335_PLL0_PC2_NDIV_INT_SHIFT;

			pll_reg = si_pmu_pllcontrol(sih, PMU_PLL_CTRL_REG3, 0, 0);

			ndiv_frac = (pll_reg & PMU1_PLL0_PC3_NDIV_FRAC_MASK) >>
					PMU1_PLL0_PC3_NDIV_FRAC_SHIFT;
		break;

	default:
		PMU_MSG(("si_pmu_cal_fvco: Unknown chipid %s\n", bcm_chipname(sih->chip, chn, 8)));
		ASSERT(0);
		return 0;
	}

	/* Actual expression is as below */
	/* fvco1 = (100 * (xf * 1/p1_div) * (ndiv_int + (ndiv_frac * 1/(1 << 24)))) */
	/* * 1/(1000 * 100); */

	/* Representing 1/p1_div as a 12 bit number */
	/* Reason for the choice of 12: */
	/* ndiv_int is represented by 9 bits */
	/* so (ndiv_int << 24) needs 33 bits */
	/* xf needs 16 bits for the worst case of 52MHz clock */
	/* So (xf * (ndiv << 24)) needs atleast 49 bits */
	/* So remaining bits for uint64 : 64 - 49 = 15 bits */
	/* So, choosing 12 bits, with 3 bits of headroom */
	int_part = xf * ndiv_int;

	rounding_const = 1 << (BBPLL_NDIV_FRAC_BITS - 1);
	math_uint64_multiple_add(&r_high, &r_low, ndiv_frac, xf, rounding_const);
	math_uint64_right_shift(&frac_part, r_high, r_low, BBPLL_NDIV_FRAC_BITS);

	if (!p1_div) {
		PMU_ERROR(("p1_div calc returned 0! [%d]\n", __LINE__));
		ROMMABLE_ASSERT(0);
		fvco = 0;
		goto done;
	}

	p1_div_scale = (1 << P1_DIV_SCALE_BITS) / p1_div;
	rounding_const = 1 << (P1_DIV_SCALE_BITS - 1);

	math_uint64_multiple_add(&r_high, &r_low, (int_part + frac_part),
		p1_div_scale, rounding_const);
	math_uint64_right_shift(&fvco, r_high, r_low, P1_DIV_SCALE_BITS);
done:
	/* Return to original core */
	si_restore_core(sih, origidx, intr_val);
	return fvco;
} /* si_pmu_cal_fvco */

bool
si_pmu_is_otp_powered(si_t *sih, osl_t *osh)
{
	uint idx;
	pmuregs_t *pmu;
	bool st;
	rsc_per_chip_t *rsc;		/* chip specific resource bit positions */

	/* Remember original core before switch to chipc/pmu */
	idx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	si_pmu_wait_for_steady_state(sih, osh, pmu);

	switch (CHIPID(sih->chip)) {
	case BCM43570_CHIP_ID:
	case BCM4360_CHIP_ID:
	case BCM43460_CHIP_ID:
	case BCM43526_CHIP_ID:
	case BCM4352_CHIP_ID:
	case BCM4347_CHIP_GRPID:
	case BCM53573_CHIP_GRPID:
		rsc = si_pmu_get_rsc_positions(sih);
		st = (R_REG(osh, &pmu->res_state) & PMURES_BIT(rsc->otp_pu)) != 0;
		break;
	/* These chip doesn't use PMU bit to power up/down OTP. OTP always on.
	 * Use OTP_INIT command to reset/refresh state.
	 */
	CASE_BCM43602_CHIP:
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
	CASE_BCM43684_CHIP:
	CASE_BCM6715_CHIP:
		st = TRUE;
		break;
	CASE_EMBEDDED_2x2AX_CORE:
	CASE_BCM6878_CHIP:
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

/**
 * Some chip/boards can be optionally fitted with an external 32Khz clock source for increased power
 * savings (due to more accurate sleep intervals).
 */
static void
BCMATTACHFN(si_pmu_set_lpoclk)(si_t *sih, osl_t *osh)
{
	uint32 ext_lpo_sel, int_lpo_sel, timeout = 0,
		ext_lpo_avail = 0, lpo_sel = 0;
	uint32 ext_lpo_isclock; /* On e.g. 43602a0, either x-tal or clock can be on LPO pins */
	pmuregs_t *pmu;
	uint origidx;

	if (!(getintvar(NULL, "boardflags3")))
		return;

	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	ext_lpo_sel = getintvar(NULL, "boardflags3") & BFL3_FORCE_EXT_LPO_SEL;
	int_lpo_sel = getintvar(NULL, "boardflags3") & BFL3_FORCE_INT_LPO_SEL;
	ext_lpo_isclock = getintvar(NULL, "boardflags3") & BFL3_EXT_LPO_ISCLOCK;

	BCM_REFERENCE(ext_lpo_isclock);

	if (ext_lpo_sel != 0) {
		switch (CHIPID(sih->chip)) {
		CASE_BCM43602_CHIP:
			/* External LPO is POR default enabled */
			si_pmu_chipcontrol(sih, PMU_CHIPCTL2, PMU43602_CC2_XTAL32_SEL,
				ext_lpo_isclock ? 0 : PMU43602_CC2_XTAL32_SEL);
			break;
		default:
			/* Force External LPO Power Up */
			si_pmu_chipcontrol(sih, PMU_CHIPCTL0, CC_EXT_LPO_PU, CC_EXT_LPO_PU);
			si_gci_chipcontrol(sih, CHIPCTRLREG6, GC_EXT_LPO_PU, GC_EXT_LPO_PU);
			break;
		}

		ext_lpo_avail = R_REG(osh, &pmu->pmustatus) & EXT_LPO_AVAIL;
		while (ext_lpo_avail == 0 && timeout < LPO_SEL_TIMEOUT) {
			OSL_DELAY(1000);
			ext_lpo_avail = R_REG(osh, &pmu->pmustatus) & EXT_LPO_AVAIL;
			timeout++;
		}

		if (timeout >= LPO_SEL_TIMEOUT) {
			PMU_ERROR(("External LPO is not available\n"));
		} else {
			/* External LPO is available, lets use (=select) it */
			OSL_DELAY(1000);
			timeout = 0;

			switch (CHIPID(sih->chip)) {
			CASE_BCM43602_CHIP:
				si_pmu_chipcontrol(sih, PMU_CHIPCTL2, PMU43602_CC2_FORCE_EXT_LPO,
					PMU43602_CC2_FORCE_EXT_LPO); /* switches to external LPO */
				break;
			default:
				/* Force External LPO Sel up */
				si_gci_chipcontrol(sih, CHIPCTRLREG6, EXT_LPO_SEL, EXT_LPO_SEL);
				/* Clear Force Internal LPO Sel */
				si_gci_chipcontrol(sih, CHIPCTRLREG6, INT_LPO_SEL, 0x0);
				OSL_DELAY(1000);

				lpo_sel = R_REG(osh, &pmu->pmucontrol) & LPO_SEL;
				while (lpo_sel != 0 && timeout < LPO_SEL_TIMEOUT) {
					OSL_DELAY(1000);
					lpo_sel = R_REG(osh, &pmu->pmucontrol) & LPO_SEL;
					timeout++;
				}
			}

			if (timeout >= LPO_SEL_TIMEOUT) {
				PMU_ERROR(("External LPO is not set\n"));
				/* Clear Force External LPO Sel */
				switch (CHIPID(sih->chip)) {
				CASE_BCM43602_CHIP:
					si_pmu_chipcontrol(sih, PMU_CHIPCTL2,
						PMU43602_CC2_FORCE_EXT_LPO, 0);
					break;
				default:
					si_gci_chipcontrol(sih, CHIPCTRLREG6, EXT_LPO_SEL, 0x0);
					break;
				}
			} else {
				/* Clear Force Internal LPO Power Up */
				switch (CHIPID(sih->chip)) {
				CASE_BCM43602_CHIP:
					break;
				default:
					si_pmu_chipcontrol(sih, PMU_CHIPCTL0, CC_INT_LPO_PU, 0x0);
					si_gci_chipcontrol(sih, CHIPCTRLREG6, GC_INT_LPO_PU, 0x0);
					break;
				}
			} /* if (timeout) */
		} /* if (timeout) */
	} else if (int_lpo_sel != 0) {
		switch (CHIPID(sih->chip)) {
		CASE_BCM43602_CHIP:
			break; /* do nothing, internal LPO is POR default powered and selected */
		default:
			/* Force Internal LPO Power Up */
			si_pmu_chipcontrol(sih, PMU_CHIPCTL0, CC_INT_LPO_PU, CC_INT_LPO_PU);
			si_gci_chipcontrol(sih, CHIPCTRLREG6, GC_INT_LPO_PU, GC_INT_LPO_PU);

			OSL_DELAY(1000);

			/* Force Internal LPO Sel up */
			si_gci_chipcontrol(sih, CHIPCTRLREG6, INT_LPO_SEL, INT_LPO_SEL);
			/* Clear Force External LPO Sel */
			si_gci_chipcontrol(sih, CHIPCTRLREG6, EXT_LPO_SEL, 0x0);

			OSL_DELAY(1000);

			lpo_sel = R_REG(osh, &pmu->pmucontrol) & LPO_SEL;
			timeout = 0;
			while (lpo_sel == 0 && timeout < LPO_SEL_TIMEOUT) {
				OSL_DELAY(1000);
				lpo_sel = R_REG(osh, &pmu->pmucontrol) & LPO_SEL;
				timeout++;
			}
			if (timeout >= LPO_SEL_TIMEOUT) {
				PMU_ERROR(("Internal LPO is not set\n"));
				/* Clear Force Internal LPO Sel */
				si_gci_chipcontrol(sih, CHIPCTRLREG6, INT_LPO_SEL, 0x0);
			} else {
				/* Clear Force External LPO Power Up */
				si_pmu_chipcontrol(sih, PMU_CHIPCTL0, CC_EXT_LPO_PU, 0x0);
				si_gci_chipcontrol(sih, CHIPCTRLREG6, GC_EXT_LPO_PU, 0x0);
			}
			break;
		}
		if ((PMUREV(sih->pmurev) >= 33)) {
			/* Enabling FAST_SEQ */
			PMU_REG(sih, pmucontrol_ext, PCTL_EXT_FASTSEQ_ENAB, PCTL_EXT_FASTSEQ_ENAB);
		}
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
} /* si_pmu_set_lpoclk */

/* Turn ON FAST LPO FLL (1MHz) for PCIE */
bool
BCMATTACHFN(si_pmu_fast_lpo_enable_pcie)(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
	case BCM4347_CHIP_GRPID: {
		uint8	fastlpo_pcie_dis = fastlpo_pcie_dis_get();

		if (!fastlpo_pcie_dis) {
			PMU_REG(sih, pmucontrol_ext,
				PCTL_EXT_FASTLPO_PCIE_SWENAB, PCTL_EXT_FASTLPO_PCIE_SWENAB);
			OSL_DELAY(1000);
			PMU_MSG(("pcie fast lpo enabled\n"));
			return TRUE;
		}
		break;
	}
	default:
		PMU_MSG(("%s: LPO enable: unsupported chip!\n", __FUNCTION__));
	}

	return FALSE;
}

/* Turn ON FAST LPO FLL (1MHz) for PMU */
bool
BCMATTACHFN(si_pmu_fast_lpo_enable_pmu)(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
	case BCM4347_CHIP_GRPID: {
		uint8	fastlpo_dis = fastlpo_dis_get();

		if (!fastlpo_dis) {
			PMU_MSG(("pmu fast lpo enabled\n"));
			return TRUE;
		}
		break;
	}
	default:
		PMU_MSG(("%s: LPO enable: unsupported chip!\n", __FUNCTION__));
	}

	return FALSE;
}

static uint8
BCMATTACHFN(fastlpo_dis_get)(void)
{
	uint8 fastlpo_dis = 1;

	if (FASTLPO_ENAB()) {
		fastlpo_dis = 0;
		if (getvar(NULL, rstr_fastlpo_dis) != NULL) {
			fastlpo_dis = (uint8)getintvar(NULL, rstr_fastlpo_dis);
		}
	}
	return fastlpo_dis;
}

static uint8
BCMATTACHFN(fastlpo_pcie_dis_get)(void)
{
	uint8 fastlpo_pcie_dis = 1;

	if (FASTLPO_ENAB()) {
		fastlpo_pcie_dis = 0;
		if (getvar(NULL, rstr_fastlpo_pcie_dis) != NULL) {
			fastlpo_pcie_dis = (uint8)getintvar(NULL, rstr_fastlpo_pcie_dis);
		}
	}
	return fastlpo_pcie_dis;
}

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
	case BCM4347_CHIP_GRPID:
	{
		pmuregs_t *pmu = si_setcore(sih, PMU_CORE_ID, 0);
		uint32 lpo = LHL_LPO_AUTO;
		uint32 lhl_tmr_sel = 0;

#ifdef USE_LHL_TIMER
		lhl_tmr_sel = CC4_4347_LHL_TIMER_SELECT;
#endif // endif
		si_pmu_chipcontrol(sih, PMU_CHIPCTL4, CC4_4347_LHL_TIMER_SELECT, lhl_tmr_sel);

		if (R_REG(osh, &pmu->pmustatus) & PST_EXTLPOAVAIL) {
			lpo = LHL_EXT_LPO_ENAB;
		}

		if (!ISSIM_ENAB(sih)) {
			si_lhl_set_lpoclk(sih, osh, lpo);
		}

		if (getintvar(NULL, rstr_btldo3p3pu)) {
			si_pmu_regcontrol(sih, 4,
				PMU_28NM_VREG4_WL_LDO_CNTL_EN,
				PMU_28NM_VREG4_WL_LDO_CNTL_EN);
			si_pmu_regcontrol(sih, 6,
				PMU_28NM_VREG6_BTLDO3P3_PU,
				PMU_28NM_VREG6_BTLDO3P3_PU);
		}

		/* Updating xtal pmu registers to combat slow powerup issue */
		si_pmu_chipcontrol(sih, PMU_CHIPCTL3,
			PMUCCTL03_4347_XTAL_CORESIZE_PMOS_NORMAL_MASK,
			(PMUCCTL03_4347_XTAL_CORESIZE_PMOS_NORMAL_VAL <<
			PMUCCTL03_4347_XTAL_CORESIZE_PMOS_NORMAL_SHIFT));

		si_pmu_chipcontrol(sih, PMU_CHIPCTL3,
			PMUCCTL03_4347_XTAL_CORESIZE_NMOS_NORMAL_MASK,
			(PMUCCTL03_4347_XTAL_CORESIZE_NMOS_NORMAL_VAL <<
			PMUCCTL03_4347_XTAL_CORESIZE_NMOS_NORMAL_SHIFT));

		si_pmu_chipcontrol(sih, PMU_CHIPCTL3,
			PMUCCTL03_4347_XTAL_SEL_BIAS_RES_NORMAL_MASK,
			(PMUCCTL03_4347_XTAL_SEL_BIAS_RES_NORMAL_VAL <<
			PMUCCTL03_4347_XTAL_SEL_BIAS_RES_NORMAL_SHIFT));

		si_pmu_chipcontrol(sih, PMU_CHIPCTL0,
			PMUCCTL00_4347_XTAL_CORESIZE_BIAS_ADJ_NORMAL_MASK,
			(PMUCCTL00_4347_XTAL_CORESIZE_BIAS_ADJ_NORMAL_VAL <<
			PMUCCTL00_4347_XTAL_CORESIZE_BIAS_ADJ_NORMAL_SHIFT));

		si_pmu_chipcontrol(sih, PMU_CHIPCTL0,
			PMUCCTL00_4347_XTAL_RES_BYPASS_NORMAL_MASK,
			(PMUCCTL00_4347_XTAL_RES_BYPASS_NORMAL_VAL <<
			PMUCCTL00_4347_XTAL_RES_BYPASS_NORMAL_SHIFT));

		if (LHL_IS_PSMODE_1(sih)) {
			si_gci_chipcontrol(sih, CC_GCI_CHIPCTRL_07,
				((1 << GCI_CC7_AAON_BYPASS_PWRSW_SEL) |
				(1 << GCI_CC7_AAON_BYPASS_PWRSW_SEQ_ON)),
				0);
		}

		si_lhl_setup(sih, osh);

		/* Setting MemLPLDO voltage */
		if (getvar(NULL, rstr_memlpldo_volt) != NULL) {
			int memlpldo_volt = getintvar(NULL, rstr_memlpldo_volt);

			if (memlpldo_volt >= PMU_VREG5_LPLDO_VOLT_0_90 &&
				memlpldo_volt <= PMU_VREG5_LPLDO_VOLT_0_88) {
				si_pmu_regcontrol(sih, PMU_VREG_5, VREG5_4347_MEMLPLDO_ADJ_MASK,
					memlpldo_volt << VREG5_4347_MEMLPLDO_ADJ_SHIFT);
			} else {
				PMU_MSG(("Invalid memlpldo value: %d\n", memlpldo_volt));
			}
		}

		/* Setting LPLDO voltage */
		if (getvar(NULL, rstr_lpldo_volt) != NULL) {
			int lpldo_volt = getintvar(NULL, rstr_lpldo_volt);

			if (lpldo_volt >= PMU_VREG5_LPLDO_VOLT_0_90 &&
				lpldo_volt <= PMU_VREG5_LPLDO_VOLT_0_88) {
				si_pmu_regcontrol(sih, PMU_VREG_5, VREG5_4347_LPLDO_ADJ_MASK,
					lpldo_volt << VREG5_4347_LPLDO_ADJ_SHIFT);
			} else {
				PMU_MSG(("Invalid lpldo value: %d\n", lpldo_volt));
			}
		}
	}
		break;
	case BCM43570_CHIP_ID:
	{
		if (CST4350_IFC_MODE(sih->chipst) == CST4350_IFC_MODE_PCIE) {
			si_pmu_chipcontrol(sih, PMU_CHIPCTL1,
				PMU_CC1_ENABLE_BBPLL_PWR_DOWN, PMU_CC1_ENABLE_BBPLL_PWR_DOWN);
			si_pmu_vreg_control(sih, PMU_VREG_0, ~0, 1);

			si_pmu_chipcontrol(sih, PMU_CHIPCTL5, CC5_4350_PMU_EN_ASSERT_MASK,
				CC5_4350_PMU_EN_ASSERT_MASK);
		}
		/* Set internal/external LPO */
		si_pmu_set_lpoclk(sih, osh);
		break;
	}
	CASE_BCM43602_CHIP: /* fall through */
		/* Set internal/external LPO */
		si_pmu_set_lpoclk(sih, osh);
		break;
	case BCM53573_CHIP_GRPID:
		if (CST53573_CHIPMODE_PCIE(sih->chipst)) {
			/* Refer 4349 code above - BBPLL is pwred down */
			si_pmu_chipcontrol(sih, PMU_53573_CHIPCTL3,
				PMU_53573_CC3_ENABLE_BBPLL_PWRDOWN_MASK,
				PMU_53573_CC3_ENABLE_BBPLL_PWRDOWN);
		}
		/* HW4349-248 Use clock requests from cores directly in clkrst to control clocks to
		 * turn on clocks faster
		 */
		si_pmu_chipcontrol(sih, PMU_53573_CHIPCTL1,
			PMU_53573_CC1_HT_CLK_REQ_CTRL_MASK, PMU_53573_CC1_HT_CLK_REQ_CTRL);
		/* Set internal/external LPO */
		si_pmu_set_lpoclk(sih, osh);
		break;
	CASE_BCM6710_CHIP:
#ifdef BCA_HNDROUTER
		/* XXX try to WAR BCAWLAN-217960 with the way below
		 * change the reset option to Reset the backplane, PMU to Power Up state.
		 */
		si_pmu_set_resetcontrol(sih, PCTL_RESETCTL_PU);
#endif /* BCA_HNDROUTER */
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
#if !defined(BCMDONGLEHOST)
	chipcregs_t *cc;
	uint origidx;
	uint32 xtalfreq, mode;

	/* PMU specific initializations */
	if (!PMUCTL_ENAB(sih))
		return;
	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);
	if (cc == NULL)
		return;

	xtalfreq = getintvar(NULL, rstr_xtalfreq);
	/*
	 * workaround for chips that don't support external LPO, thus ALP clock
	 * can not be measured accurately:
	 */
	switch (CHIPID(sih->chip)) {
		CASE_BCM43602_CHIP:
		case BCM4363_CHIP_ID:
		case BCM4365_CHIP_ID:
		case BCM4366_CHIP_ID:
		case BCM43664_CHIP_ID:
	        case BCM43666_CHIP_ID:
			xtalfreq = 40000;
			break;
		case BCM43570_CHIP_ID:
			if (xtalfreq == 0) {
				mode = CST4350_IFC_MODE(sih->chipst);
				if ((mode == CST4350_IFC_MODE_USB20D) ||
				    (mode == CST4350_IFC_MODE_USB30D) ||
				    (mode == CST4350_IFC_MODE_USB30D_WL))
					xtalfreq = 40000;
				else {
					xtalfreq = 37400;
					if (mode == CST4350_IFC_MODE_HSIC20D ||
					    mode == CST4350_IFC_MODE_HSIC30D) {
						/* HSIC sprom_present_strap=1:40 mHz xtal */
						if (((CHIPREV(sih->chiprev) >= 3) ||
						     (CHIPID(sih->chip) == BCM43570_CHIP_ID)) &&
						    CST4350_PKG_USB_40M(sih->chipst) &&
						    CST4350_PKG_USB(sih->chipst)) {
							xtalfreq = 40000;
						}
					}
				}
			}
			break;
		default:
			break;
	}
	/* If xtalfreq var not available, try to measure it */
	if (xtalfreq == 0)
		xtalfreq = si_pmu_measure_alpclk(sih, osh);
	si_pmu_enb_slow_clk(sih, osh, xtalfreq);
	/* Return to original core */
	si_setcoreidx(sih, origidx);
#endif /* !BCMDONGLEHOST */
}

/** initialize PMU registers in case default values proved to be suboptimal */
void
BCMATTACHFN(si_pmu_swreg_init)(si_t *sih, osl_t *osh)
{
	int gpio_drive_strength;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	switch (CHIPID(sih->chip)) {
	case BCM53573_CHIP_GRPID:
		break;
	CASE_BCM43602_CHIP:
		/* adjust PA Vref to 2.80V */
		si_pmu_set_ldo_voltage(sih, osh, SET_LDO_VOLTAGE_PAREF, 0x0c);
		break;
	case BCM4347_CHIP_GRPID:
		/* add option to change gpio out drive strength, required for some
		* Olympic modules
		*/
		if (getvar(NULL, rstr_gpio_drive_strength)) {
			gpio_drive_strength = getintvar(NULL, rstr_gpio_drive_strength);
			si_gci_chipcontrol(sih, CC_GCI_CHIPCTRL_04, 0x3f<<0,
			gpio_drive_strength | gpio_drive_strength<<3);
		}
		break;
	default:
		break;
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	if (BCM53573_CHIP(sih->chip) ||
		BCM4365_CHIP(sih->chip) ||
		(BCM4347_CHIP(sih->chip)) ||
		BCM43602_CHIP(sih->chip) ||
		BCM4350_CHIP(sih->chip) ||
		EMBEDDED_2x2AX_CORE(sih->chip) ||
		BCM6715_CHIP(sih->chip) ||
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
	} else
		alp_khz = 0;

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return alp_khz;
} /* si_pmu_measure_alpclk */

/** Update min/max resources after SR-ASM download to d11 txfifo */
void
si_pmu_res_minmax_update(si_t *sih, osl_t *osh)
{
	uint32 min_mask = 0, max_mask = 0;
	pmuregs_t *pmu;
	uint origidx, intr_val = 0;

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	/* Remember original core before switch to chipc/pmu */
	origidx = si_coreidx(sih);
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4363_CHIP_ID:
	case BCM4365_CHIP_ID:
	case BCM4366_CHIP_ID:
	case BCM43664_CHIP_ID:
	case BCM43666_CHIP_ID:
	CASE_BCM43602_CHIP:
	case BCM43570_CHIP_ID:
		max_mask = 0; /* Only care about min_mask for now */
		break;

	case BCM53573_CHIP_GRPID:
		break; /* Not sure what to do in this case */
	case BCM4347_CHIP_GRPID:
		si_pmu_res_masks(sih, &min_mask, &max_mask);
		max_mask = 0; /* Don't need to update max */
		break;
	default:
		break;
	}
	if (min_mask) {
		/* Add min mask dependencies */
		min_mask |= si_pmu_res_deps(sih, osh, pmu, min_mask, FALSE);
		W_REG(osh, &pmu->min_res_mask, min_mask);
	}
	if (max_mask) {
		max_mask |= si_pmu_res_deps(sih, osh, pmu, max_mask, FALSE);
		W_REG(osh, &pmu->max_res_mask, max_mask);
	}

	si_pmu_wait_for_steady_state(sih, osh, pmu);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	si_intrrestore(sih, intr_val);
} /* si_pmu_res_minmax_update */

#ifdef DONGLEBUILD

#define PMUCAP_DUMP_TAG_SIZE_BYTES	4

/* XXX:
 * this is not declared as static const, although that is the right thing to do
 * reason being if declared as static const, compile/link process would that in
 * read only section...
 * currently this code/array is used to identify the registers which are dumped
 * during trap processing
 * and usually for the trap buffer, .rodata buffer is reused,  so for now just static
*/
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

	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	}
	else {
		pmu = si_setcore(sih, SI_CC_IDX, 0);
	}
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
			addr = (uint32 *)(SI_ENUM_BASE(sih) + chipc_regs_to_dump[i]);
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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

#ifdef LDO3P3_MIN_RES_MASK

/** Set ldo 3.3V mask in the min resources mask register */
int
si_pmu_min_res_ldo3p3_set(si_t *sih, osl_t *osh, bool on)
{
	uint32 min_mask = 0;

	return BCME_UNSUPPORTED;
}

int
si_pmu_min_res_ldo3p3_get(si_t *sih, osl_t *osh, int *res)
{
	uint32 min_mask = 0;

	return BCME_UNSUPPORTED;
}

#endif /* LDO3P3_MIN_RES_MASK */

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
#endif /* !defined(BCMDONGLEHOST) */

#if defined(BCMULP)

int
si_pmu_ulp_register(si_t *sih)
{
	return ulp_p1_module_register(ULP_MODULE_ID_PMU, &ulp_pmu_ctx, (void *)sih);
}

static uint
si_pmu_ulp_get_retention_size_cb(void *handle, ulp_ext_info_t *einfo)
{
	ULP_DBG(("%s: sz: %d\n", __FUNCTION__, sizeof(si_pmu_ulp_cr_dat_t)));
	return sizeof(si_pmu_ulp_cr_dat_t);
}

static int
si_pmu_ulp_enter_cb(void *handle, ulp_ext_info_t *einfo, uint8 *cache_data)
{
	si_pmu_ulp_cr_dat_t crinfo = {0};
	crinfo.ilpcycles_per_sec = ilpcycles_per_sec;
	ULP_DBG(("%s: ilpcycles_per_sec: %x\n", __FUNCTION__, ilpcycles_per_sec));
	memcpy(cache_data, (void*)&crinfo, sizeof(crinfo));
	return BCME_OK;
}

static int
si_pmu_ulp_exit_cb(void *handle, uint8 *cache_data,
	uint8 *p2_cache_data)
{
	si_pmu_ulp_cr_dat_t *crinfo = (si_pmu_ulp_cr_dat_t *)cache_data;

	ilpcycles_per_sec = crinfo->ilpcycles_per_sec;
	ULP_DBG(("%s: ilpcycles_per_sec: %x, cache_data: %p\n", __FUNCTION__,
		ilpcycles_per_sec, cache_data));
	return BCME_OK;
}

void
si_pmu_ulp_ilp_config(si_t *sih, osl_t *osh, uint32 ilp_period)
{
	pmuregs_t *pmu;
	pmu = si_setcoreidx(sih, si_findcoreidx(sih, PMU_CORE_ID, 0));
	W_REG(osh, &pmu->ILPPeriod, ilp_period);
	si_lhl_ilp_config(sih, osh, ilp_period);
}

#endif /* defined(BCMULP) */

uint32
si_pmu_wake_bit_offset(si_t *sih)
{
	uint32 wakebit;

	switch (CHIPID(sih->chip)) {
	case BCM4347_CHIP_GRPID:
		wakebit = CC2_4347_GCI2WAKE_MASK;
		break;
	default:
		wakebit = 0;
		ASSERT(0);
		break;
	}

	return wakebit;
}

#ifdef ATE_BUILD
void hnd_pmu_clr_int_sts_req_active(osl_t *hnd_osh, si_t *sih)
{
	uint32 res_req_timer;
	pmuregs_t *pmu;
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	}
	else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	W_REG(osh, &pmu->min_res_mask, min_res_mask);
	OSL_DELAY(100);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

#if !defined(BCMDONGLEHOST)
void si_pmu_chipcontrol_xtal_settings_4369(si_t *sih)
{

/* 4369 XTAL Bias settings */
/*
	Reg name			startup		Normal
	xtal_bias_adj		0xFF				0x20
	xtal_coresize_nmos	0x3f				0x3f
	xtal_coresize_pmos	0x3f				0x3f
	xtal_sel_bias_res	0x2				0x5
	xt_res_bypass		0x0				0x2
*/
	uint32 u32Val;
	uint32 u32Mask;

	u32Val = (PMU_CC0_4369_XTALCORESIZE_BIAS_ADJ_START_VAL |
		PMU_CC0_4369_XTALCORESIZE_BIAS_ADJ_NORMAL_VAL |
		PMU_CC0_4369_XTAL_RES_BYPASS_START_VAL |
		PMU_CC0_4369_XTAL_RES_BYPASS_NORMAL_VAL);

	u32Mask = (PMU_CC0_4369_XTALCORESIZE_BIAS_ADJ_START_MASK |
		PMU_CC0_4369_XTALCORESIZE_BIAS_ADJ_NORMAL_MASK |
		PMU_CC0_4369_XTAL_RES_BYPASS_START_MASK |
		PMU_CC0_4369_XTAL_RES_BYPASS_NORMAL_MASK);

	si_pmu_chipcontrol(sih, PMU_CHIPCTL0, u32Mask, u32Val);

	u32Val = (PMU_CC2_4369_XTALCORESIZE_BIAS_ADJ_START_VAL |
		PMU_CC2_4369_XTALCORESIZE_BIAS_ADJ_NORMAL_VAL);

	u32Mask = (PMU_CC2_4369_XTALCORESIZE_BIAS_ADJ_START_MASK |
		PMU_CC2_4369_XTALCORESIZE_BIAS_ADJ_NORMAL_MASK);

	si_pmu_chipcontrol(sih, PMU_CHIPCTL2, u32Mask, u32Val);

	u32Val = (PMU_CC3_4369_XTALCORESIZE_PMOS_START_VAL |
		PMU_CC3_4369_XTALCORESIZE_PMOS_NORMAL_VAL |
		PMU_CC3_4369_XTALCORESIZE_NMOS_START_VAL |
		PMU_CC3_4369_XTALCORESIZE_NMOS_NORMAL_VAL |
		PMU_CC3_4369_XTALSEL_BIAS_RES_START_VAL |
		PMU_CC3_4369_XTALSEL_BIAS_RES_NORMAL_VAL);

	u32Mask = (PMU_CC3_4369_XTALCORESIZE_PMOS_START_MASK |
		PMU_CC3_4369_XTALCORESIZE_PMOS_NORMAL_MASK |
		PMU_CC3_4369_XTALCORESIZE_NMOS_START_MASK |
		PMU_CC3_4369_XTALCORESIZE_NMOS_NORMAL_MASK |
		PMU_CC3_4369_XTALSEL_BIAS_RES_START_MASK |
		PMU_CC3_4369_XTALSEL_BIAS_RES_NORMAL_MASK);

	si_pmu_chipcontrol(sih, PMU_CHIPCTL3, u32Mask, u32Val);

}

/*
 * si_pmu_set_resetcontrol
 * Set the PMU's reset behavior for NIC-mode devices
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
#endif /* !BCMDONGLEHOST */

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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
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
	if (AOB_ENAB(sih)) {
		pmu = si_setcore(sih, PMU_CORE_ID, 0);
	} else {
		pmu = si_setcoreidx(sih, SI_CC_IDX);
	}
	ASSERT(pmu != NULL);

	pmustatstimer[timerid].cnt_mode = cnt_mode;
	si_pmustatstimer_update(osh, pmu, timerid);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}
#endif /* BCMPMU_STATS */
