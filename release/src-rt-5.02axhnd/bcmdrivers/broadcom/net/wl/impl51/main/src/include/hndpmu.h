/*
 * HND SiliconBackplane PMU support.
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
 * $Id: hndpmu.h 742769 2018-01-23 15:23:27Z $
 */

#ifndef _hndpmu_h_
#define _hndpmu_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <siutils.h>
#include <sbchipc.h>

#if !defined(BCMDONGLEHOST)

#define SET_LDO_VOLTAGE_LDO1		1
#define SET_LDO_VOLTAGE_LDO2		2
#define SET_LDO_VOLTAGE_LDO3		3
#define SET_LDO_VOLTAGE_PAREF		4
#define SET_LDO_VOLTAGE_CLDO_PWM	5
#define SET_LDO_VOLTAGE_CLDO_BURST	6
#define SET_LDO_VOLTAGE_CBUCK_PWM	7
#define SET_LDO_VOLTAGE_CBUCK_BURST	8
#define SET_LDO_VOLTAGE_LNLDO1		9
#define SET_LDO_VOLTAGE_LNLDO2_SEL	10
#define SET_LNLDO_PWERUP_LATCH_CTRL	11
#define SET_LDO_VOLTAGE_LDO3P3		12

#define BBPLL_NDIV_FRAC_BITS		24
#define P1_DIV_SCALE_BITS			12

#define PMUREQTIMER (1 << 0)

#define XTAL_FREQ_40MHZ		40000
#define XTAL_FREQ_54MHZ		54000

/* selects core based on AOB_ENAB() */
#define PMUREGADDR(sih, pmur, ccr, member) \
	(AOB_ENAB(sih) ? (&(pmur)->member) : (&(ccr)->member))

/* prevents backplane stall caused by subsequent writes to 'ilp domain' PMU registers */
#define HND_PMU_SYNC_WR(sih, pmur, ccr, osh, r, v) do { \
	if ((sih) && (sih)->pmurev >= 22) { \
		while (R_REG(osh, PMUREGADDR(sih, pmur, ccr, pmustatus)) & \
		       PST_SLOW_WR_PENDING) { \
			; /* empty */ \
		} \
	} \
	W_REG(osh, r, v); \
	(void)R_REG(osh, r); \
} while (0)

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
#endif // endif

extern void si_pmu_init(si_t *sih, osl_t *osh);
extern void si_pmu_chip_init(si_t *sih, osl_t *osh);
extern void si_pmu_pll_init(si_t *sih, osl_t *osh, uint32 xtalfreq);
extern void si_pmu_res_init(si_t *sih, osl_t *osh);
extern void si_pmu_swreg_init(si_t *sih, osl_t *osh);
extern void si_pmu_res_minmax_update(si_t *sih, osl_t *osh);

extern uint32 si_pmu_si_clock(si_t *sih, osl_t *osh);   /* returns [Hz] units */
extern uint32 si_pmu_cpu_clock(si_t *sih, osl_t *osh);  /* returns [hz] units */
extern uint32 si_pmu_mem_clock(si_t *sih, osl_t *osh);  /* returns [Hz] units */
extern uint32 si_pmu_alp_clock(si_t *sih, osl_t *osh);  /* returns [Hz] units */
extern void si_pmu_ilp_clock_set(uint32 cycles);
extern uint32 si_pmu_ilp_clock(si_t *sih, osl_t *osh);  /* returns [Hz] units */

extern void si_pmu_set_ldo_voltage(si_t *sih, osl_t *osh, uint8 ldo, uint8 voltage);
extern uint16 si_pmu_fast_pwrup_delay(si_t *sih, osl_t *osh);
extern void si_pmu_pllupd(si_t *sih);
extern void si_pmu_spuravoid(si_t *sih, osl_t *osh, uint8 spuravoid);
extern void si_pmu_spuravoid_isdone(si_t *sih, osl_t *osh, uint32 min_res_mask,
	uint32 max_res_mask, uint32 clk_ctl_st, uint8 spuravoid);
extern void si_pmu_pll_4364_macfreq_switch(si_t *sih, osl_t *osh, uint8 mode);
extern void si_pmu_pll_off_PARR(si_t *sih, osl_t *osh, uint32 *min_res_mask,
	uint32 *max_res_mask, uint32 *clk_ctl_st);
/* below function are only for BBPLL parallel purpose */
extern void si_pmu_gband_spurwar(si_t *sih, osl_t *osh);
extern uint32 si_pmu_cal_fvco(si_t *sih, osl_t *osh);

extern bool si_pmu_is_otp_powered(si_t *sih, osl_t *osh);
extern uint32 si_pmu_measure_alpclk(si_t *sih, osl_t *osh);

extern uint32 si_pmu_chipcontrol(si_t *sih, uint reg, uint32 mask, uint32 val);
#define si_pmu_regcontrol si_pmu_vreg_control /* prevents build err because of usage in PHY */
extern uint32 si_pmu_vreg_control(si_t *sih, uint reg, uint32 mask, uint32 val);
extern uint32 si_pmu_pllcontrol(si_t *sih, uint reg, uint32 mask, uint32 val);
extern void si_pmu_pllupd(si_t *sih);

extern uint32 si_pmu_waitforclk_on_backplane(si_t *sih, osl_t *osh, uint32 clk, uint32 delay);
extern uint32 si_pmu_get_bb_vcofreq(si_t *sih, osl_t *osh, int xtalfreq);
typedef void (*si_pmu_callback_t)(void* arg);

extern uint32 si_mac_clk(si_t *sih, osl_t *osh);
extern void si_pmu_switch_on_PARLDO(si_t *sih, osl_t *osh);
extern void si_pmu_switch_off_PARLDO(si_t *sih, osl_t *osh);
extern int si_pmu_fvco_pllreg(si_t *sih, uint32 *fvco, uint32 *pllreg);

extern bool si_pmu_reset_ret_sleep_log(si_t *sih, osl_t *osh);
extern int si_pmu_openloop_cal(si_t *sih, uint16 currtemp);

#ifdef LDO3P3_MIN_RES_MASK
extern int si_pmu_min_res_ldo3p3_set(si_t *sih, osl_t *osh, bool on);
extern int si_pmu_min_res_ldo3p3_get(si_t *sih, osl_t *osh, int *res);
#endif /* LDO3P3_MIN_RES_MASK */

#ifndef WLTEST
extern int si_pmu_min_res_otp_pu_set(si_t *sih, osl_t *osh, bool on);
#endif /* !WLTEST */
#endif /* !defined(BCMDONGLEHOST) */

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
#endif // endif
extern uint32 si_pmu_wake_bit_offset(si_t *sih);
#if defined(BCMULP)
int si_pmu_ulp_register(si_t *sih);
extern void si_pmu_ulp_ilp_config(si_t *sih, osl_t *osh, uint32 ilp_period);
#endif /* BCMULP */
extern uint32 si_pmu_get_pmutimer(si_t *sih);
extern void si_switch_pmu_dependency(si_t *sih, uint mode);
extern void si_pmu_set_min_res_mask(si_t *sih, osl_t *osh, uint min_res_mask);
extern void si_pmu_set_mac_rsrc_req(si_t *sih, int macunit);
extern bool si_pmu_fast_lpo_enable_pcie(si_t *sih);
extern bool si_pmu_fast_lpo_enable_pmu(si_t *sih);
extern void si_pmu_chipcontrol_xtal_settings_4369(si_t *sih);
extern uint32 si_cur_pmu_time(si_t *sih);
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
