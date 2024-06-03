/*
 * HND SiliconBackplane PMU support.
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
 * $Id: hndpmu.h 828363 2023-08-08 09:20:08Z $
 */

#ifndef _hndpmu_h_
#define _hndpmu_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <siutils.h>
#include <sbchipc.h>

#define PMUREGADDR(pmur, member) (&(pmur)->member)

/* prevents backplane stall caused by subsequent writes to 'ilp domain' PMU registers */
#define HND_PMU_SYNC_WR(sih, pmur, osh, r, v) do { \
	if ((sih) && (sih)->pmurev >= 22) { \
		while (R_REG(osh, PMUREGADDR(pmur, pmustatus)) & PST_SLOW_WR_PENDING) { \
			; /* empty */ \
		} \
	} \
	W_REG(osh, r, v); \
	(void)R_REG(osh, r); \
} while (0)

/* uses this enum instead of 'magic' values when writing to register pllcontrol_addr */
enum pmu_pll_ctrl_reg_e {
	PMU_PLL_CTRL_REG0	= 0,
	PMU_PLL_CTRL_REG1	= 1,
	PMU_PLL_CTRL_REG2	= 2,
	PMU_PLL_CTRL_REG3	= 3,
	PMU_PLL_CTRL_REG4	= 4,
	PMU_PLL_CTRL_REG5	= 5,
	PMU_PLL_CTRL_REG6	= 6,
	PMU_PLL_CTRL_REG7	= 7,
	PMU_PLL_CTRL_REG8	= 8,
	PMU_PLL_CTRL_REG9	= 9,
	PMU_PLL_CTRL_REG10	= 10,
	PMU_PLL_CTRL_REG11	= 11,
	PMU_PLL_CTRL_REG12	= 12,
	PMU_PLL_CTRL_REG13	= 13,
	PMU_PLL_CTRL_REG14	= 14,
	PMU_PLL_CTRL_REG15	= 15
};

enum si_pmu_clk_e {
	SI_PMU_CLK_XTAL = 0,
	SI_PMU_CLK_ILP,
	SI_PMU_CLK_ALP,
	SI_PMU_CLK_CPU,
	SI_PMU_CLK_MEM,
	SI_PMU_CLK_BACKPLANE
};

/* PMU Stat Timer */

/* for count mode */
enum {
	PMU_STATS_LEVEL_HIGH = 0,
	PMU_STATS_LEVEL_LOW,
	PMU_STATS_EDGE_RISE,
	PMU_STATS_EDGE_FALL
};

typedef struct {
	uint8 src_num; /* predefined source hw signal num to map timer */
	bool  enable;  /* timer enable/disable */
	bool  int_enable; /* overflow interrupts enable/disable */
	uint8 cnt_mode;
} pmu_stats_timer_t;

/* internal hw signal source number for Timer  */
#define SRC_PMU_RESRC_OFFSET 0x40

#define SRC_LINK_IN_L12 0
#define SRC_LINK_IN_L23 1
#define SRC_PM_ST_IN_D0 2
#define SRC_PM_ST_IN_D3 3

#define SRC_XTAL_PU (SRC_PMU_RESRC_OFFSET + RES4347_XTAL_PU)
#define SRC_CORE_RDY_MAIN (SRC_PMU_RESRC_OFFSET + RES4347_CORE_RDY_MAIN)
#define SRC_CORE_RDY_AUX (SRC_PMU_RESRC_OFFSET + RES4347_CORE_RDY_AUX)

#ifdef BCMPMU_STATS
extern bool _pmustatsenab;
	#if defined(ROM_ENAB_RUNTIME_CHECK)
		#define PMU_STATS_ENAB()	(_pmustatsenab)
	#elif defined(BCMPMU_STATS_DISABLED)
		#define PMU_STATS_ENAB()	(0)
	#else
		#define PMU_STATS_ENAB()	(1)
	#endif
#else
	#define PMU_STATS_ENAB()	(0)
#endif /* BCMPMU_STATS */

/* 1MHz lpo enable */
#ifdef BCM_FASTLPO
	#define FASTLPO_ENAB()	(TRUE)
#else
	#define FASTLPO_ENAB()	(FALSE)
#endif

extern void si_pmu_init(si_t *sih, osl_t *osh);
extern void si_pmu_chip_init(si_t *sih, osl_t *osh);
extern void si_pmu_pll_init(si_t *sih, osl_t *osh, uint32 xtalfreq);
extern void si_pmu_res_init(si_t *sih, osl_t *osh);
extern void si_pmu_swreg_init(si_t *sih, osl_t *osh);

/** returns [Hz] units */
extern uint32 si_pmu_get_clock(si_t *sih, osl_t *osh, enum si_pmu_clk_e clk_type);

extern void si_pmu_ilp_clock_set(uint32 cycles);

extern void si_pmu_set_ldo_voltage(si_t *sih, osl_t *osh, uint8 ldo, uint8 voltage);
extern uint16 si_pmu_fast_pwrup_delay(si_t *sih, osl_t *osh);
extern void si_pmu_pllupd(si_t *sih);
extern void si_pmu_spuravoid(si_t *sih, osl_t *osh, uint8 spuravoid);
extern void si_pmu_spuravoid_isdone(si_t *sih, osl_t *osh, uint32 min_res_mask,
	uint32 max_res_mask, uint32 clk_ctl_st, uint8 spuravoid);
extern bool si_pmu_is_otp_powered(si_t *sih, osl_t *osh);
extern uint32 si_pmu_measure_alpclk(si_t *sih, osl_t *osh);

extern uint32 si_pmu_chipcontrol(si_t *sih, uint reg, uint32 mask, uint32 val);
#define si_pmu_regcontrol si_pmu_vreg_control /* prevents build err because of usage in PHY */
extern uint32 si_pmu_vreg_control(si_t *sih, uint reg, uint32 mask, uint32 val);

extern uint32 si_pmu_waitforclk_on_backplane(si_t *sih, osl_t *osh, uint32 clk, uint32 delay);

extern bool si_pmu_reset_ret_sleep_log(si_t *sih, osl_t *osh);
extern int si_pmu_openloop_cal(si_t *sih, uint16 currtemp);

#ifndef WLTEST
extern int si_pmu_min_res_otp_pu_set(si_t *sih, osl_t *osh, bool on);
#endif /* !WLTEST */

extern void si_pmu_set_resetcontrol(si_t *sih, uint32 value);
extern uint32 si_pmu_get_resetcontrol(si_t *sih);

extern void si_pmu_6715_prevent_on_the_fly_arm_vco_change(si_t *sih);

extern uint32 si_pmu_pllcontrol(si_t *sih, enum pmu_pll_ctrl_reg_e reg, uint32 mask, uint32 val);
extern uint32 si_pmu_rsrc_macphy_clk_deps(si_t *sih, osl_t *osh, int maccore_index);
extern uint32 si_pmu_rsrc_ht_avail_clk_deps(si_t *sih, osl_t *osh);

extern void si_pmu_otp_power(si_t *sih, osl_t *osh, bool on, uint32* min_res_mask);
extern void si_sdiod_drive_strength_init(si_t *sih, osl_t *osh, uint32 drivestrength);

extern void si_pmu_slow_clk_reinit(si_t *sih, osl_t *osh);
extern void si_pmu_avbtimer_enable(si_t *sih, osl_t *osh, bool set_flag);
extern uint32 si_pmu_dump_pmucap_binary(si_t *sih, uchar *p);
extern uint32 si_pmu_dump_buf_size_pmucap(si_t *sih);
extern int si_pmu_wait_for_steady_state(si_t *sih, osl_t *osh, pmuregs_t *pmu);
#ifdef ATE_BUILD
extern void hnd_pmu_clr_int_sts_req_active(osl_t *hnd_osh, si_t *hnd_sih);
#endif
extern uint32 si_pmu_get_pmutimer(si_t *sih);
extern void si_switch_pmu_dependency(si_t *sih, uint mode);
extern void si_pmu_set_min_res_mask(si_t *sih, osl_t *osh, uint min_res_mask);
extern void si_pmu_set_mac_rsrc_req(si_t *sih, int macunit);
#ifdef BCMPMU_STATS
extern void si_pmustatstimer_init(si_t *sih);
extern void si_pmustatstimer_dump(si_t *sih);
extern void si_pmustatstimer_start(si_t *sih, uint8 timerid);
extern void si_pmustatstimer_stop(si_t *sih, uint8 timerid);
extern void si_pmustatstimer_clear(si_t *sih, uint8 timerid);
extern void si_pmustatstimer_clear_overflow(si_t *sih);
extern uint32 si_pmustatstimer_read(si_t *sih, uint8 timerid);
extern void si_pmustatstimer_cfg_src_num(si_t *sih, uint8 src_num, uint8 timerid);
extern void si_pmustatstimer_cfg_cnt_mode(si_t *sih, uint8 cnt_mode, uint8 timerid);
extern void si_pmustatstimer_int_enable(si_t *sih);
extern void si_pmustatstimer_int_disable(si_t *sih);
#endif /* BCMPMU_STATS */
#endif /* _hndpmu_h_ */
