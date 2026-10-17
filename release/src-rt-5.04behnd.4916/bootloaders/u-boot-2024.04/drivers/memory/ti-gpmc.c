// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments GPMC Driver
 *
 * Copyright (C) 2021 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/io.h>
#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <linux/mtd/omap_gpmc.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include "ti-gpmc.h"

enum gpmc_clk_domain {
	GPMC_CD_FCLK,
	GPMC_CD_CLK
};

struct gpmc_cs_data {
	const char *name;
#define GPMC_CS_RESERVED	BIT(0)
	u32 flags;
};

struct ti_gpmc {
	void __iomem *base;
	u32 cs_num;
	u32 nr_waitpins;
	struct clk *l3_clk;
	u32 capability_flags;
	struct resource data;
};

static struct gpmc_cs_data gpmc_cs[GPMC_CS_NUM];
static unsigned int gpmc_cs_num = GPMC_CS_NUM;
static unsigned int gpmc_nr_waitpins;
static unsigned int gpmc_capability;
static void __iomem *gpmc_base;
static struct clk *gpmc_l3_clk;

/* Public, as required by nand/raw/omap_gpmc.c */
const struct gpmc *gpmc_cfg;

/*
 * The first 1MB of GPMC address space is typically mapped to
 * the internal ROM. Never allocate the first page, to
 * facilitate bug detection; even if we didn't boot from ROM.
 * As GPMC minimum partition size is 16MB we can only start from
 * there.
 */
#define GPMC_MEM_START		0x1000000
#define GPMC_MEM_END		0x3FFFFFFF

static void gpmc_write_reg(int idx, u32 val)
{
	writel_relaxed(val, gpmc_base + idx);
}

static u32 gpmc_read_reg(int idx)
{
	return readl_relaxed(gpmc_base + idx);
}

static void gpmc_cs_write_reg(int cs, int idx, u32 val)
{
	void __iomem *reg_addr;

	reg_addr = gpmc_base + GPMC_CS0_OFFSET + (cs * GPMC_CS_SIZE) + idx;
	writel_relaxed(val, reg_addr);
}

static u32 gpmc_cs_read_reg(int cs, int idx)
{
	void __iomem *reg_addr;

	reg_addr = gpmc_base + GPMC_CS0_OFFSET + (cs * GPMC_CS_SIZE) + idx;
	return readl_relaxed(reg_addr);
}

static unsigned long gpmc_get_fclk_period(void)
{
	unsigned long rate = clk_get_rate(gpmc_l3_clk);

	rate /= 1000;
	rate = 1000000000 / rate;	/* In picoseconds */

	return rate;
}

/**
 * gpmc_get_clk_period - get period of selected clock domain in ps
 * @cs: Chip Select Region.
 * @cd: Clock Domain.
 *
 * GPMC_CS_CONFIG1 GPMCFCLKDIVIDER for cs has to be setup
 * prior to calling this function with GPMC_CD_CLK.
 */
static unsigned long gpmc_get_clk_period(int cs, enum gpmc_clk_domain cd)
{
	unsigned long tick_ps = gpmc_get_fclk_period();
	u32 l;
	int div;

	switch (cd) {
	case GPMC_CD_CLK:
		/* get current clk divider */
		l = gpmc_cs_read_reg(cs, GPMC_CS_CONFIG1);
		div = (l & 0x03) + 1;
		/* get GPMC_CLK period */
		tick_ps *= div;
		break;
	case GPMC_CD_FCLK:
	default:
		break;
	}

	return tick_ps;
}

static unsigned int gpmc_ns_to_clk_ticks(unsigned int time_ns, int cs,
					 enum gpmc_clk_domain cd)
{
	unsigned long tick_ps;

	/* Calculate in picosecs to yield more exact results */
	tick_ps = gpmc_get_clk_period(cs, cd);

	return (time_ns * 1000 + tick_ps - 1) / tick_ps;
}

static unsigned int gpmc_ns_to_ticks(unsigned int time_ns)
{
	return gpmc_ns_to_clk_ticks(time_ns, /* any CS */ 0, GPMC_CD_FCLK);
}

static unsigned int gpmc_ps_to_ticks(unsigned int time_ps)
{
	unsigned long tick_ps;

	/* Calculate in picosecs to yield more exact results */
	tick_ps = gpmc_get_fclk_period();

	return (time_ps + tick_ps - 1) / tick_ps;
}

static __maybe_unused unsigned int gpmc_clk_ticks_to_ns(unsigned int ticks, int cs,
							enum gpmc_clk_domain cd)
{
	return ticks * gpmc_get_clk_period(cs, cd) / 1000;
}

static inline void gpmc_cs_modify_reg(int cs, int reg, u32 mask, bool value)
{
	u32 l;

	l = gpmc_cs_read_reg(cs, reg);
	if (value)
		l |= mask;
	else
		l &= ~mask;
	gpmc_cs_write_reg(cs, reg, l);
}

static void gpmc_cs_bool_timings(int cs, const struct gpmc_bool_timings *p)
{
	gpmc_cs_modify_reg(cs, GPMC_CS_CONFIG1,
			   GPMC_CONFIG1_TIME_PARA_GRAN,
			   p->time_para_granularity);
	gpmc_cs_modify_reg(cs, GPMC_CS_CONFIG2,
			   GPMC_CONFIG2_CSEXTRADELAY, p->cs_extra_delay);
	gpmc_cs_modify_reg(cs, GPMC_CS_CONFIG3,
			   GPMC_CONFIG3_ADVEXTRADELAY, p->adv_extra_delay);
	gpmc_cs_modify_reg(cs, GPMC_CS_CONFIG4,
			   GPMC_CONFIG4_OEEXTRADELAY, p->oe_extra_delay);
	gpmc_cs_modify_reg(cs, GPMC_CS_CONFIG4,
			   GPMC_CONFIG4_WEEXTRADELAY, p->we_extra_delay);
	gpmc_cs_modify_reg(cs, GPMC_CS_CONFIG6,
			   GPMC_CONFIG6_CYCLE2CYCLESAMECSEN,
			   p->cycle2cyclesamecsen);
	gpmc_cs_modify_reg(cs, GPMC_CS_CONFIG6,
			   GPMC_CONFIG6_CYCLE2CYCLEDIFFCSEN,
			   p->cycle2cyclediffcsen);
}

#if IS_ENABLED(CONFIG_TI_GPMC_DEBUG)
/**
 * get_gpmc_timing_reg - read a timing parameter and print DTS settings for it.
 * @cs:      Chip Select Region
 * @reg:     GPMC_CS_CONFIGn register offset.
 * @st_bit:  Start Bit
 * @end_bit: End Bit. Must be >= @st_bit.
 * @max:     Maximum parameter value (before optional @shift).
 *           If 0, maximum is as high as @st_bit and @end_bit allow.
 * @name:    DTS node name, w/o "gpmc,"
 * @cd:      Clock Domain of timing parameter.
 * @shift:   Parameter value left shifts @shift, which is then printed instead of value.
 * @raw:     Raw Format Option.
 *           raw format:  gpmc,name = <value>
 *           tick format: gpmc,name = <value> /&zwj;* x ns -- y ns; x ticks *&zwj;/
 *           Where x ns -- y ns result in the same tick value.
 *           When @max is exceeded, "invalid" is printed inside comment.
 * @noval:   Parameter values equal to 0 are not printed.
 * @return:  Specified timing parameter (after optional @shift).
 *
 */
static int get_gpmc_timing_reg(/* timing specifiers */
	int cs, int reg, int st_bit, int end_bit, int max,
	const char *name, const enum gpmc_clk_domain cd,
	/* value transform */
	int shift,
	/* format specifiers */
	bool raw, bool noval)
{
	u32 l;
	int nr_bits;
	int mask;
	bool invalid;

	l = gpmc_cs_read_reg(cs, reg);
	nr_bits = end_bit - st_bit + 1;
	mask = (1 << nr_bits) - 1;
	l = (l >> st_bit) & mask;
	if (!max)
		max = mask;
	invalid = l > max;
	if (shift)
		l = (shift << l);
	if (noval && l == 0)
		return 0;
	if (!raw) {
		/* DTS tick format for timings in ns */
		unsigned int time_ns;
		unsigned int time_ns_min = 0;

		if (l)
			time_ns_min = gpmc_clk_ticks_to_ns(l - 1, cs, cd) + 1;
		time_ns = gpmc_clk_ticks_to_ns(l, cs, cd);
		pr_info("gpmc,%s = <%u>; /* %u ns - %u ns; %i ticks%s*/\n",
			name, time_ns, time_ns_min, time_ns, l,
			invalid ? "; invalid " : " ");
	} else {
		/* raw format */
		pr_info("gpmc,%s = <%u>;%s\n", name, l,
			invalid ? " /* invalid */" : "");
	}

	return l;
}

#define GPMC_PRINT_CONFIG(cs, config) \
	pr_info("CS%i %s: 0x%08x\n", cs, #config, \
		gpmc_cs_read_reg(cs, config))
#define GPMC_GET_RAW(reg, st, end, field) \
	get_gpmc_timing_reg(cs, (reg), (st), (end), 0, field, GPMC_CD_FCLK, 0, 1, 0)
#define GPMC_GET_RAW_MAX(reg, st, end, max, field) \
	get_gpmc_timing_reg(cs, (reg), (st), (end), (max), field, GPMC_CD_FCLK, 0, 1, 0)
#define GPMC_GET_RAW_BOOL(reg, st, end, field) \
	get_gpmc_timing_reg(cs, (reg), (st), (end), 0, field, GPMC_CD_FCLK, 0, 1, 1)
#define GPMC_GET_RAW_SHIFT_MAX(reg, st, end, shift, max, field) \
	get_gpmc_timing_reg(cs, (reg), (st), (end), (max), field, GPMC_CD_FCLK, (shift), 1, 1)
#define GPMC_GET_TICKS(reg, st, end, field) \
	get_gpmc_timing_reg(cs, (reg), (st), (end), 0, field, GPMC_CD_FCLK, 0, 0, 0)
#define GPMC_GET_TICKS_CD(reg, st, end, field, cd) \
	get_gpmc_timing_reg(cs, (reg), (st), (end), 0, field, (cd), 0, 0, 0)
#define GPMC_GET_TICKS_CD_MAX(reg, st, end, max, field, cd) \
	get_gpmc_timing_reg(cs, (reg), (st), (end), (max), field, (cd), 0, 0, 0)

static void gpmc_show_regs(int cs, const char *desc)
{
	pr_info("gpmc cs%i %s:\n", cs, desc);
	GPMC_PRINT_CONFIG(cs, GPMC_CS_CONFIG1);
	GPMC_PRINT_CONFIG(cs, GPMC_CS_CONFIG2);
	GPMC_PRINT_CONFIG(cs, GPMC_CS_CONFIG3);
	GPMC_PRINT_CONFIG(cs, GPMC_CS_CONFIG4);
	GPMC_PRINT_CONFIG(cs, GPMC_CS_CONFIG5);
	GPMC_PRINT_CONFIG(cs, GPMC_CS_CONFIG6);
}

/*
 * Note that gpmc,wait-pin handing wrongly assumes bit 8 is available,
 * see commit c9fb809.
 */
static void gpmc_cs_show_timings(int cs, const char *desc)
{
	gpmc_show_regs(cs, desc);

	pr_info("gpmc cs%i access configuration:\n", cs);
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG1,  4,  4, "time-para-granularity");
	GPMC_GET_RAW(GPMC_CS_CONFIG1,  8,  9, "mux-add-data");
	GPMC_GET_RAW_SHIFT_MAX(GPMC_CS_CONFIG1, 12, 13, 1,
			       GPMC_CONFIG1_DEVICESIZE_MAX, "device-width");
	GPMC_GET_RAW(GPMC_CS_CONFIG1, 16, 17, "wait-pin");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG1, 21, 21, "wait-on-write");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG1, 22, 22, "wait-on-read");
	GPMC_GET_RAW_SHIFT_MAX(GPMC_CS_CONFIG1, 23, 24, 4,
			       GPMC_CONFIG1_ATTACHEDDEVICEPAGELENGTH_MAX,
			       "burst-length");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG1, 27, 27, "sync-write");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG1, 28, 28, "burst-write");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG1, 29, 29, "gpmc,sync-read");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG1, 30, 30, "burst-read");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG1, 31, 31, "burst-wrap");

	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG2,  7,  7, "cs-extra-delay");

	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG3,  7,  7, "adv-extra-delay");

	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG4, 23, 23, "we-extra-delay");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG4,  7,  7, "oe-extra-delay");

	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG6,  7,  7, "cycle2cycle-samecsen");
	GPMC_GET_RAW_BOOL(GPMC_CS_CONFIG6,  6,  6, "cycle2cycle-diffcsen");

	pr_info("gpmc cs%i timings configuration:\n", cs);
	GPMC_GET_TICKS(GPMC_CS_CONFIG2,  0,  3, "cs-on-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG2,  8, 12, "cs-rd-off-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG2, 16, 20, "cs-wr-off-ns");

	GPMC_GET_TICKS(GPMC_CS_CONFIG3,  0,  3, "adv-on-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG3,  8, 12, "adv-rd-off-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG3, 16, 20, "adv-wr-off-ns");
	if (gpmc_capability & GPMC_HAS_MUX_AAD) {
		GPMC_GET_TICKS(GPMC_CS_CONFIG3, 4, 6, "adv-aad-mux-on-ns");
		GPMC_GET_TICKS(GPMC_CS_CONFIG3, 24, 26,
			       "adv-aad-mux-rd-off-ns");
		GPMC_GET_TICKS(GPMC_CS_CONFIG3, 28, 30,
			       "adv-aad-mux-wr-off-ns");
	}

	GPMC_GET_TICKS(GPMC_CS_CONFIG4,  0,  3, "oe-on-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG4,  8, 12, "oe-off-ns");
	if (gpmc_capability & GPMC_HAS_MUX_AAD) {
		GPMC_GET_TICKS(GPMC_CS_CONFIG4,  4,  6, "oe-aad-mux-on-ns");
		GPMC_GET_TICKS(GPMC_CS_CONFIG4, 13, 15, "oe-aad-mux-off-ns");
	}
	GPMC_GET_TICKS(GPMC_CS_CONFIG4, 16, 19, "we-on-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG4, 24, 28, "we-off-ns");

	GPMC_GET_TICKS(GPMC_CS_CONFIG5,  0,  4, "rd-cycle-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG5,  8, 12, "wr-cycle-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG5, 16, 20, "access-ns");

	GPMC_GET_TICKS(GPMC_CS_CONFIG5, 24, 27, "page-burst-access-ns");

	GPMC_GET_TICKS(GPMC_CS_CONFIG6, 0, 3, "bus-turnaround-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG6, 8, 11, "cycle2cycle-delay-ns");

	GPMC_GET_TICKS_CD_MAX(GPMC_CS_CONFIG1, 18, 19,
			      GPMC_CONFIG1_WAITMONITORINGTIME_MAX,
			      "wait-monitoring-ns", GPMC_CD_CLK);
	GPMC_GET_TICKS_CD_MAX(GPMC_CS_CONFIG1, 25, 26,
			      GPMC_CONFIG1_CLKACTIVATIONTIME_MAX,
			      "clk-activation-ns", GPMC_CD_FCLK);

	GPMC_GET_TICKS(GPMC_CS_CONFIG6, 16, 19, "wr-data-mux-bus-ns");
	GPMC_GET_TICKS(GPMC_CS_CONFIG6, 24, 28, "wr-access-ns");
}
#else
static inline void gpmc_cs_show_timings(int cs, const char *desc)
{
}
#endif

/**
 * set_gpmc_timing_reg - set a single timing parameter for Chip Select Region.
 * Caller is expected to have initialized CONFIG1 GPMCFCLKDIVIDER
 * prior to calling this function with @cd equal to GPMC_CD_CLK.
 *
 * @cs:      Chip Select Region.
 * @reg:     GPMC_CS_CONFIGn register offset.
 * @st_bit:  Start Bit
 * @end_bit: End Bit. Must be >= @st_bit.
 * @max:     Maximum parameter value.
 *           If 0, maximum is as high as @st_bit and @end_bit allow.
 * @time:    Timing parameter in ns.
 * @cd:      Timing parameter clock domain.
 * @name:    Timing parameter name.
 * @return:  0 on success, -1 on error.
 */
static int set_gpmc_timing_reg(int cs, int reg, int st_bit, int end_bit, int max,
			       int time, enum gpmc_clk_domain cd, const char *name)
{
	u32 l;
	int ticks, mask, nr_bits;

	if (time == 0)
		ticks = 0;
	else
		ticks = gpmc_ns_to_clk_ticks(time, cs, cd);
	nr_bits = end_bit - st_bit + 1;
	mask = (1 << nr_bits) - 1;

	if (!max)
		max = mask;

	if (ticks > max) {
		pr_err("%s: GPMC CS%d: %s %d ns, %d ticks > %d ticks\n",
		       __func__, cs, name, time, ticks, max);

		return -1;
	}

	l = gpmc_cs_read_reg(cs, reg);
	if (IS_ENABLED(CONFIG_TI_GPMC_DEBUG)) {
		pr_info("GPMC CS%d: %-17s: %3d ticks, %3lu ns (was %3i ticks) %3d ns\n",
			cs, name, ticks, gpmc_get_clk_period(cs, cd) * ticks / 1000,
				(l >> st_bit) & mask, time);
	}

	l &= ~(mask << st_bit);
	l |= ticks << st_bit;
	gpmc_cs_write_reg(cs, reg, l);

	return 0;
}

/**
 * gpmc_calc_waitmonitoring_divider - calculate proper GPMCFCLKDIVIDER based on WAITMONITORINGTIME
 * WAITMONITORINGTIME will be _at least_ as long as desired, i.e.
 * read  --> don't sample bus too early
 * write --> data is longer on bus
 *
 * Formula:
 * gpmc_clk_div + 1 = ceil(ceil(waitmonitoringtime_ns / gpmc_fclk_ns)
 *                    / waitmonitoring_ticks)
 * WAITMONITORINGTIME resulting in 0 or 1 tick with div = 1 are caught by
 * div <= 0 check.
 *
 * @wait_monitoring: WAITMONITORINGTIME in ns.
 * @return:          -1 on failure to scale, else proper divider > 0.
 */
static int gpmc_calc_waitmonitoring_divider(unsigned int wait_monitoring)
{
	int div = gpmc_ns_to_ticks(wait_monitoring);

	div += GPMC_CONFIG1_WAITMONITORINGTIME_MAX - 1;
	div /= GPMC_CONFIG1_WAITMONITORINGTIME_MAX;

	if (div > 4)
		return -1;
	if (div <= 0)
		div = 1;

	return div;
}

/**
 * gpmc_calc_divider - calculate GPMC_FCLK divider for sync_clk GPMC_CLK period.
 * @sync_clk: GPMC_CLK period in ps.
 * @return:   Returns at least 1 if GPMC_FCLK can be divided to GPMC_CLK.
 *            Else, returns -1.
 */
static int gpmc_calc_divider(unsigned int sync_clk)
{
	int div = gpmc_ps_to_ticks(sync_clk);

	if (div > 4)
		return -1;
	if (div <= 0)
		div = 1;

	return div;
}

/**
 * gpmc_cs_set_timings - program timing parameters for Chip Select Region.
 * @cs:     Chip Select Region.
 * @t:      GPMC timing parameters.
 * @s:      GPMC timing settings.
 * @return: 0 on success, -1 on error.
 */
static int gpmc_cs_set_timings(int cs, const struct gpmc_timings *t,
			       const struct gpmc_settings *s)
{
	int div, ret;
	u32 l;

	div = gpmc_calc_divider(t->sync_clk);
	if (div < 0)
		return -EINVAL;

	/*
	 * See if we need to change the divider for waitmonitoringtime.
	 *
	 * Calculate GPMCFCLKDIVIDER independent of gpmc,sync-clk-ps in DT for
	 * pure asynchronous accesses, i.e. both read and write asynchronous.
	 * However, only do so if WAITMONITORINGTIME is actually used, i.e.
	 * either WAITREADMONITORING or WAITWRITEMONITORING is set.
	 *
	 * This statement must not change div to scale async WAITMONITORINGTIME
	 * to protect mixed synchronous and asynchronous accesses.
	 *
	 * We raise an error later if WAITMONITORINGTIME does not fit.
	 */
	if (!s->sync_read && !s->sync_write &&
	    (s->wait_on_read || s->wait_on_write)
	   ) {
		div = gpmc_calc_waitmonitoring_divider(t->wait_monitoring);
		if (div < 0) {
			pr_err("%s: waitmonitoringtime %3d ns too large for greatest gpmcfclkdivider.\n",
			       __func__,
			       t->wait_monitoring
			       );
			return -ENXIO;
		}
	}

	ret = 0;
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG2, 0, 3, 0, t->cs_on,
				   GPMC_CD_FCLK, "cs_on");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG2, 8, 12, 0, t->cs_rd_off,
				   GPMC_CD_FCLK, "cs_rd_off");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG2, 16, 20, 0, t->cs_wr_off,
				   GPMC_CD_FCLK, "cs_wr_off");
	if (ret)
		return -ENXIO;

	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG3, 0, 3, 0, t->adv_on,
				   GPMC_CD_FCLK, "adv_on");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG3, 8, 12, 0, t->adv_rd_off,
				   GPMC_CD_FCLK, "adv_rd_off");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG3, 16, 20, 0, t->adv_wr_off,
				   GPMC_CD_FCLK, "adv_wr_off");
	if (ret)
		return -ENXIO;

	if (gpmc_capability & GPMC_HAS_MUX_AAD) {
		ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG3, 4, 6, 0,
					   t->adv_aad_mux_on, GPMC_CD_FCLK,
					   "adv_aad_mux_on");
		ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG3, 24, 26, 0,
					   t->adv_aad_mux_rd_off, GPMC_CD_FCLK,
					   "adv_aad_mux_rd_off");
		ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG3, 28, 30, 0,
					   t->adv_aad_mux_wr_off, GPMC_CD_FCLK,
					   "adv_aad_mux_wr_off");
		if (ret)
			return -ENXIO;
	}

	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG4, 0, 3, 0, t->oe_on,
				   GPMC_CD_FCLK, "oe_on");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG4, 8, 12, 0, t->oe_off,
				   GPMC_CD_FCLK, "oe_off");
	if (gpmc_capability & GPMC_HAS_MUX_AAD) {
		ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG4, 4, 6, 0,
					   t->oe_aad_mux_on, GPMC_CD_FCLK,
					   "oe_aad_mux_on");
		ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG4, 13, 15, 0,
					   t->oe_aad_mux_off, GPMC_CD_FCLK,
					   "oe_aad_mux_off");
	}
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG4, 16, 19, 0, t->we_on,
				   GPMC_CD_FCLK, "we_on");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG4, 24, 28, 0, t->we_off,
				   GPMC_CD_FCLK, "we_off");
	if (ret)
		return -ENXIO;

	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG5, 0, 4, 0, t->rd_cycle,
				   GPMC_CD_FCLK, "rd_cycle");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG5, 8, 12, 0, t->wr_cycle,
				   GPMC_CD_FCLK, "wr_cycle");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG5, 16, 20, 0, t->access,
				   GPMC_CD_FCLK, "access");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG5, 24, 27, 0,
				   t->page_burst_access, GPMC_CD_FCLK,
				   "page_burst_access");
	if (ret)
		return -ENXIO;

	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG6, 0, 3, 0,
				   t->bus_turnaround, GPMC_CD_FCLK,
				   "bus_turnaround");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG6, 8, 11, 0,
				   t->cycle2cycle_delay, GPMC_CD_FCLK,
				   "cycle2cycle_delay");
	if (ret)
		return -ENXIO;

	if (gpmc_capability & GPMC_HAS_WR_DATA_MUX_BUS) {
		ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG6, 16, 19, 0,
					   t->wr_data_mux_bus, GPMC_CD_FCLK,
					   "wr_data_mux_bus");
		if (ret)
			return -ENXIO;
	}
	if (gpmc_capability & GPMC_HAS_WR_ACCESS) {
		ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG6, 24, 28, 0,
					   t->wr_access, GPMC_CD_FCLK,
					   "wr_access");
		if (ret)
			return -ENXIO;
	}

	l = gpmc_cs_read_reg(cs, GPMC_CS_CONFIG1);
	l &= ~0x03;
	l |= (div - 1);
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG1, l);

	ret = 0;
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG1, 18, 19,
				   GPMC_CONFIG1_WAITMONITORINGTIME_MAX,
				   t->wait_monitoring, GPMC_CD_CLK,
				   "wait_monitoring");
	ret |= set_gpmc_timing_reg(cs, GPMC_CS_CONFIG1, 25, 26,
				   GPMC_CONFIG1_CLKACTIVATIONTIME_MAX,
				   t->clk_activation, GPMC_CD_FCLK,
				   "clk_activation");
	if (ret)
		return -ENXIO;

	if (IS_ENABLED(CONFIG_TI_GPMC_DEBUG)) {
		pr_info("GPMC CS%d CLK period is %lu ns (div %d)\n",
			cs, (div * gpmc_get_fclk_period()) / 1000, div);
	}

	gpmc_cs_bool_timings(cs, &t->bool_timings);
	gpmc_cs_show_timings(cs, "after gpmc_set_timings");

	return 0;
}

static int gpmc_cs_set_memconf(int cs, resource_size_t base, u32 size)
{
	u32 l;
	u32 mask;

	/*
	 * Ensure that base address is aligned on a
	 * boundary equal to or greater than size.
	 */
	if (base & (size - 1))
		return -EINVAL;

	base >>= GPMC_CHUNK_SHIFT;
	mask = (1 << GPMC_SECTION_SHIFT) - size;
	mask >>= GPMC_CHUNK_SHIFT;
	mask <<= GPMC_CONFIG7_MASKADDRESS_OFFSET;

	l = gpmc_cs_read_reg(cs, GPMC_CS_CONFIG7);
	l &= ~GPMC_CONFIG7_MASK;
	l |= base & GPMC_CONFIG7_BASEADDRESS_MASK;
	l |= mask & GPMC_CONFIG7_MASKADDRESS_MASK;
	l |= GPMC_CONFIG7_CSVALID;
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG7, l);

	return 0;
}

static void gpmc_cs_enable_mem(int cs)
{
	u32 l;

	l = gpmc_cs_read_reg(cs, GPMC_CS_CONFIG7);
	l |= GPMC_CONFIG7_CSVALID;
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG7, l);
}

static void gpmc_cs_disable_mem(int cs)
{
	u32 l;

	l = gpmc_cs_read_reg(cs, GPMC_CS_CONFIG7);
	l &= ~GPMC_CONFIG7_CSVALID;
	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG7, l);
}

static void gpmc_cs_set_reserved(int cs, int reserved)
{
	struct gpmc_cs_data *gpmc = &gpmc_cs[cs];

	gpmc->flags |= GPMC_CS_RESERVED;
}

static bool gpmc_cs_reserved(int cs)
{
	struct gpmc_cs_data *gpmc = &gpmc_cs[cs];

	return gpmc->flags & GPMC_CS_RESERVED;
}

static unsigned long gpmc_mem_align(unsigned long size)
{
	int order;

	size = (size - 1) >> (GPMC_CHUNK_SHIFT - 1);
	order = GPMC_CHUNK_SHIFT - 1;
	do {
		size >>= 1;
		order++;
	} while (size);
	size = 1 << order;
	return size;
}

static int gpmc_cs_request(ofnode node, int cs, struct resource *res)
{
	int r = -1;
	u32 size;
	resource_size_t addr_base = res->start;

	if (cs >= gpmc_cs_num) {
		pr_err("%s: requested chip-select is disabled\n", __func__);
		return -ENODEV;
	}

	size = gpmc_mem_align(resource_size(res));
	if (size > (1 << GPMC_SECTION_SHIFT))
		return -ENOMEM;

	if (gpmc_cs_reserved(cs)) {
		r = -EBUSY;
		goto out;
	}

	if (addr_base & (SZ_16M - 1)) {
		pr_err("CS region should be aligned to 16M boundary\n");
		goto out;
	}

	/* Disable CS while changing base address and size mask */
	gpmc_cs_disable_mem(cs);

	r = gpmc_cs_set_memconf(cs, addr_base, size);
	if (r < 0)
		goto out;

	/* Enable CS */
	gpmc_cs_enable_mem(cs);
	gpmc_cs_set_reserved(cs, 1);
out:
	return r;
}

static void gpmc_cs_free(int cs)
{
	if (cs >= gpmc_cs_num || cs < 0 || !gpmc_cs_reserved(cs)) {
		pr_warn("Trying to free non-reserved GPMC CS%d\n", cs);
		return;
	}

	gpmc_cs_disable_mem(cs);
	gpmc_cs_set_reserved(cs, 0);
}

/**
 * gpmc_configure - write request to configure gpmc
 * @cmd: command type
 * @wval: value to write
 * @return status of the operation
 */
static int gpmc_configure(int cmd, int wval)
{
	u32 regval;

	switch (cmd) {
	case GPMC_CONFIG_WP:
		regval = gpmc_read_reg(GPMC_CONFIG);
		if (wval)
			regval &= ~GPMC_CONFIG_WRITEPROTECT; /* WP is ON */
		else
			regval |= GPMC_CONFIG_WRITEPROTECT;  /* WP is OFF */
		gpmc_write_reg(GPMC_CONFIG, regval);
		break;

	default:
		pr_err("%s: command not supported\n", __func__);
		return -EINVAL;
	}

	return 0;
}

/**
 * gpmc_cs_program_settings - programs non-timing related settings
 * @cs:		GPMC chip-select to program
 * @p:		pointer to GPMC settings structure
 *
 * Programs non-timing related settings for a GPMC chip-select, such as
 * bus-width, burst configuration, etc. Function should be called once
 * for each chip-select that is being used and must be called before
 * calling gpmc_cs_set_timings() as timing parameters in the CONFIG1
 * register will be initialised to zero by this function. Returns 0 on
 * success and appropriate negative error code on failure.
 */
static int gpmc_cs_program_settings(int cs, struct gpmc_settings *p)
{
	u32 config1;

	if (!p->device_width || p->device_width > GPMC_DEVWIDTH_16BIT) {
		pr_err("%s: invalid width %d!", __func__, p->device_width);
		return -EINVAL;
	}

	/* Address-data multiplexing not supported for NAND devices */
	if (p->device_nand && p->mux_add_data) {
		pr_err("%s: invalid configuration!\n", __func__);
		return -EINVAL;
	}

	if (p->mux_add_data > GPMC_MUX_AD ||
	    (p->mux_add_data == GPMC_MUX_AAD &&
	     !(gpmc_capability & GPMC_HAS_MUX_AAD))) {
		pr_err("%s: invalid multiplex configuration!\n", __func__);
		return -EINVAL;
	}

	/* Page/burst mode supports lengths of 4, 8 and 16 bytes */
	if (p->burst_read || p->burst_write) {
		switch (p->burst_len) {
		case GPMC_BURST_4:
		case GPMC_BURST_8:
		case GPMC_BURST_16:
			break;
		default:
			pr_err("%s: invalid page/burst-length (%d)\n",
			       __func__, p->burst_len);
			return -EINVAL;
		}
	}

	if (p->wait_pin > gpmc_nr_waitpins) {
		pr_err("%s: invalid wait-pin (%d)\n", __func__, p->wait_pin);
		return -EINVAL;
	}

	config1 = GPMC_CONFIG1_DEVICESIZE((p->device_width - 1));

	if (p->sync_read)
		config1 |= GPMC_CONFIG1_READTYPE_SYNC;
	if (p->sync_write)
		config1 |= GPMC_CONFIG1_WRITETYPE_SYNC;
	if (p->wait_on_read)
		config1 |= GPMC_CONFIG1_WAIT_READ_MON;
	if (p->wait_on_write)
		config1 |= GPMC_CONFIG1_WAIT_WRITE_MON;
	if (p->wait_on_read || p->wait_on_write)
		config1 |= GPMC_CONFIG1_WAIT_PIN_SEL(p->wait_pin);
	if (p->device_nand)
		config1	|= GPMC_CONFIG1_DEVICETYPE(GPMC_DEVICETYPE_NAND);
	if (p->mux_add_data)
		config1	|= GPMC_CONFIG1_MUXTYPE(p->mux_add_data);
	if (p->burst_read)
		config1 |= GPMC_CONFIG1_READMULTIPLE_SUPP;
	if (p->burst_write)
		config1 |= GPMC_CONFIG1_WRITEMULTIPLE_SUPP;
	if (p->burst_read || p->burst_write) {
		config1 |= GPMC_CONFIG1_PAGE_LEN(p->burst_len >> 3);
		config1 |= p->burst_wrap ? GPMC_CONFIG1_WRAPBURST_SUPP : 0;
	}

	gpmc_cs_write_reg(cs, GPMC_CS_CONFIG1, config1);

	return 0;
}

static void gpmc_cs_set_name(int cs, const char *name)
{
	struct gpmc_cs_data *gpmc = &gpmc_cs[cs];

	gpmc->name = name;
}

static const char *gpmc_cs_get_name(int cs)
{
	struct gpmc_cs_data *gpmc = &gpmc_cs[cs];

	return gpmc->name;
}

/**
 * gpmc_read_settings_dt - read gpmc settings from device-tree
 * @np:		pointer to device-tree node for a gpmc child device
 * @p:		pointer to gpmc settings structure
 *
 * Reads the GPMC settings for a GPMC child device from device-tree and
 * stores them in the GPMC settings structure passed. The GPMC settings
 * structure is initialised to zero by this function and so any
 * previously stored settings will be cleared.
 */
static void gpmc_read_settings_dt(ofnode np, struct gpmc_settings *p)
{
	memset(p, 0, sizeof(struct gpmc_settings));

	p->sync_read = ofnode_read_bool(np, "gpmc,sync-read");
	p->sync_write = ofnode_read_bool(np, "gpmc,sync-write");
	ofnode_read_u32(np, "gpmc,device-width", &p->device_width);
	ofnode_read_u32(np, "gpmc,mux-add-data", &p->mux_add_data);

	if (!ofnode_read_u32(np, "gpmc,burst-length", &p->burst_len)) {
		p->burst_wrap = ofnode_read_bool(np, "gpmc,burst-wrap");
		p->burst_read = ofnode_read_bool(np, "gpmc,burst-read");
		p->burst_write = ofnode_read_bool(np, "gpmc,burst-write");
		if (!p->burst_read && !p->burst_write)
			pr_warn("%s: page/burst-length set but not used!\n",
				__func__);
	}

	if (!ofnode_read_u32(np, "gpmc,wait-pin", &p->wait_pin)) {
		p->wait_on_read = ofnode_read_bool(np,
						   "gpmc,wait-on-read");
		p->wait_on_write = ofnode_read_bool(np,
						    "gpmc,wait-on-write");
		if (!p->wait_on_read && !p->wait_on_write)
			pr_debug("%s: rd/wr wait monitoring not enabled!\n",
				 __func__);
	}
}

static void gpmc_read_timings_dt(ofnode np,
				 struct gpmc_timings *gpmc_t)
{
	struct gpmc_bool_timings *p;

	if (!gpmc_t)
		return;

	memset(gpmc_t, 0, sizeof(*gpmc_t));

	/* minimum clock period for syncronous mode */
	ofnode_read_u32(np, "gpmc,sync-clk-ps", &gpmc_t->sync_clk);

	/* chip select timtings */
	ofnode_read_u32(np, "gpmc,cs-on-ns", &gpmc_t->cs_on);
	ofnode_read_u32(np, "gpmc,cs-rd-off-ns", &gpmc_t->cs_rd_off);
	ofnode_read_u32(np, "gpmc,cs-wr-off-ns", &gpmc_t->cs_wr_off);

	/* ADV signal timings */
	ofnode_read_u32(np, "gpmc,adv-on-ns", &gpmc_t->adv_on);
	ofnode_read_u32(np, "gpmc,adv-rd-off-ns", &gpmc_t->adv_rd_off);
	ofnode_read_u32(np, "gpmc,adv-wr-off-ns", &gpmc_t->adv_wr_off);
	ofnode_read_u32(np, "gpmc,adv-aad-mux-on-ns",
			&gpmc_t->adv_aad_mux_on);
	ofnode_read_u32(np, "gpmc,adv-aad-mux-rd-off-ns",
			&gpmc_t->adv_aad_mux_rd_off);
	ofnode_read_u32(np, "gpmc,adv-aad-mux-wr-off-ns",
			&gpmc_t->adv_aad_mux_wr_off);

	/* WE signal timings */
	ofnode_read_u32(np, "gpmc,we-on-ns", &gpmc_t->we_on);
	ofnode_read_u32(np, "gpmc,we-off-ns", &gpmc_t->we_off);

	/* OE signal timings */
	ofnode_read_u32(np, "gpmc,oe-on-ns", &gpmc_t->oe_on);
	ofnode_read_u32(np, "gpmc,oe-off-ns", &gpmc_t->oe_off);
	ofnode_read_u32(np, "gpmc,oe-aad-mux-on-ns",
			&gpmc_t->oe_aad_mux_on);
	ofnode_read_u32(np, "gpmc,oe-aad-mux-off-ns",
			&gpmc_t->oe_aad_mux_off);

	/* access and cycle timings */
	ofnode_read_u32(np, "gpmc,page-burst-access-ns",
			&gpmc_t->page_burst_access);
	ofnode_read_u32(np, "gpmc,access-ns", &gpmc_t->access);
	ofnode_read_u32(np, "gpmc,rd-cycle-ns", &gpmc_t->rd_cycle);
	ofnode_read_u32(np, "gpmc,wr-cycle-ns", &gpmc_t->wr_cycle);
	ofnode_read_u32(np, "gpmc,bus-turnaround-ns",
			&gpmc_t->bus_turnaround);
	ofnode_read_u32(np, "gpmc,cycle2cycle-delay-ns",
			&gpmc_t->cycle2cycle_delay);
	ofnode_read_u32(np, "gpmc,wait-monitoring-ns",
			&gpmc_t->wait_monitoring);
	ofnode_read_u32(np, "gpmc,clk-activation-ns",
			&gpmc_t->clk_activation);

	/* only applicable to OMAP3+ */
	ofnode_read_u32(np, "gpmc,wr-access-ns", &gpmc_t->wr_access);
	ofnode_read_u32(np, "gpmc,wr-data-mux-bus-ns",
			&gpmc_t->wr_data_mux_bus);

	/* bool timing parameters */
	p = &gpmc_t->bool_timings;

	p->cycle2cyclediffcsen =
		ofnode_read_bool(np, "gpmc,cycle2cycle-diffcsen");
	p->cycle2cyclesamecsen =
		ofnode_read_bool(np, "gpmc,cycle2cycle-samecsen");
	p->we_extra_delay = ofnode_read_bool(np, "gpmc,we-extra-delay");
	p->oe_extra_delay = ofnode_read_bool(np, "gpmc,oe-extra-delay");
	p->adv_extra_delay = ofnode_read_bool(np, "gpmc,adv-extra-delay");
	p->cs_extra_delay = ofnode_read_bool(np, "gpmc,cs-extra-delay");
	p->time_para_granularity =
		ofnode_read_bool(np, "gpmc,time-para-granularity");
}

/**
 * gpmc_probe_generic_child - configures the gpmc for a child device
 * @dev:	pointer to gpmc platform device
 * @child:	pointer to device-tree node for child device
 *
 * Allocates and configures a GPMC chip-select for a child device.
 * Returns 0 on success and appropriate negative error code on failure.
 */
static int gpmc_probe_generic_child(struct udevice *dev,
				    ofnode child)
{
	struct gpmc_settings gpmc_s;
	struct gpmc_timings gpmc_t;
	struct resource res;
	const char *name;
	int ret;
	u32 val, cs;

	if (ofnode_read_u32(child, "reg", &cs) < 0) {
		dev_err(dev, "can't get reg property of child %s\n",
			ofnode_get_name(child));
		return -ENODEV;
	}

	if (ofnode_read_resource(child, 0, &res) < 0) {
		dev_err(dev, "%s has malformed 'reg' property\n",
			ofnode_get_name(child));
		return -ENODEV;
	}

	/*
	 * Check if we have multiple instances of the same device
	 * on a single chip select. If so, use the already initialized
	 * timings.
	 */
	name = gpmc_cs_get_name(cs);
	if (name && !strcmp(name, ofnode_get_name(child)))
		goto no_timings;

	ret = gpmc_cs_request(child, cs, &res);
	if (ret < 0) {
		dev_err(dev, "cannot request GPMC CS %d\n", cs);
		return ret;
	}
	gpmc_cs_set_name(cs, ofnode_get_name(child));

	gpmc_read_settings_dt(child, &gpmc_s);
	gpmc_read_timings_dt(child, &gpmc_t);

	/*
	 * For some GPMC devices we still need to rely on the bootloader
	 * timings because the devices can be connected via FPGA.
	 * REVISIT: Add timing support from slls644g.pdf.
	 */
	if (!gpmc_t.cs_rd_off) {
		pr_warn("enable GPMC debug to configure .dts timings for CS%i\n",
			cs);
		gpmc_cs_show_timings(cs,
				     "please add GPMC bootloader timings to .dts");
		goto no_timings;
	}

	/* CS must be disabled while making changes to gpmc configuration */
	gpmc_cs_disable_mem(cs);

	if (!ofnode_read_u32(child, "nand-bus-width", &val)) {
		/* NAND specific setup */
		ofnode_read_u32(child, "nand-bus-width", &val);
		switch (val) {
		case 8:
			gpmc_s.device_width = GPMC_DEVWIDTH_8BIT;
			break;
		case 16:
			gpmc_s.device_width = GPMC_DEVWIDTH_16BIT;
			break;
		default:
			dev_err(dev, "%s: invalid 'nand-bus-width'\n",
				ofnode_get_name(child));
			ret = -EINVAL;
			goto err;
		}

		/* disable write protect */
		gpmc_configure(GPMC_CONFIG_WP, 0);
		gpmc_s.device_nand = true;
	} else {
		ret = ofnode_read_u32(child, "bank-width",
				      &gpmc_s.device_width);
		if (ret < 0 && !gpmc_s.device_width) {
			dev_err(dev,
				"%s has no 'gpmc,device-width' property\n",
				ofnode_get_name(child));
			goto err;
		}
	}

	gpmc_cs_show_timings(cs, "before gpmc_cs_program_settings");

	ret = gpmc_cs_program_settings(cs, &gpmc_s);
	if (ret < 0)
		goto err;

	ret = gpmc_cs_set_timings(cs, &gpmc_t, &gpmc_s);
	if (ret) {
		dev_err(dev, "failed to set gpmc timings for: %s\n",
			ofnode_get_name(child));
		goto err;
	}

	/* Clear limited address i.e. enable A26-A11 */
	val = gpmc_read_reg(GPMC_CONFIG);
	val &= ~GPMC_CONFIG_LIMITEDADDRESS;
	gpmc_write_reg(GPMC_CONFIG, val);

	/* Enable CS region */
	gpmc_cs_enable_mem(cs);

no_timings:

	return 0;

err:
	gpmc_cs_free(cs);

	return ret;
}

static void gpmc_probe_dt_children(struct udevice *dev)
{
	int ret;
	ofnode child;

	ofnode_for_each_subnode(child, dev_ofnode(dev)) {
		ret = gpmc_probe_generic_child(dev, child);
		if (ret) {
			dev_err(dev, "Cannot parse child %s:%d",
				ofnode_get_name(child), ret);
		}
	}
}

static int gpmc_parse_dt(struct udevice *dev, struct ti_gpmc *gpmc)
{
	int ret;
	u32 val;

	ret = ofnode_read_u32(dev_ofnode(dev), "gpmc,num-cs",
			      &val);
	if (ret < 0) {
		pr_err("%s: number of chip-selects not defined\n", __func__);
		return ret;
	} else if (val < 1) {
		pr_err("%s: all chip-selects are disabled\n", __func__);
		return -EINVAL;
	} else if (val > GPMC_CS_NUM) {
		pr_err("%s: number of supported chip-selects cannot be > %d\n",
		       __func__, GPMC_CS_NUM);
		return -EINVAL;
	}

	gpmc->cs_num = val;
	gpmc_cs_num = val;

	ret = ofnode_read_u32(dev_ofnode(dev), "gpmc,num-waitpins",
			      &gpmc->nr_waitpins);
	if (ret < 0) {
		pr_err("%s: number of wait pins not found!\n", __func__);
		return ret;
	}

	gpmc_nr_waitpins = gpmc->nr_waitpins;

	return 0;
}

static int gpmc_probe(struct udevice *dev)
{
	struct ti_gpmc *priv = dev_get_priv(dev);
	int ret;
	struct resource res;

	ret = dev_read_resource_byname(dev, "cfg", &res);
	if (ret) {
		/* Legacy DT */
		dev_read_resource(dev, 0, &res);
		priv->base = devm_ioremap(dev, res.start, resource_size(&res));

		priv->data.start = GPMC_MEM_START;
		priv->data.end = GPMC_MEM_END;
	} else {
		priv->base = devm_ioremap(dev, res.start, resource_size(&res));
		ret = dev_read_resource_byname(dev, "data", &res);
		if (ret)
			return -ENOENT;

		priv->data = res;
	}

	if (!priv->base)
		return -ENOMEM;

	gpmc_cfg = (struct gpmc *)priv->base;
	gpmc_base = priv->base;

	/*
	 * Disable all IRQs as some bootroms might leave them enabled
	 * and that will cause a lock-up later
	 */
	gpmc_write_reg(GPMC_IRQENABLE, 0);

	priv->l3_clk = devm_clk_get(dev, "fck");
	if (IS_ERR(priv->l3_clk))
		return PTR_ERR(priv->l3_clk);

	if (!clk_get_rate(priv->l3_clk))
		return -EINVAL;

	gpmc_l3_clk = priv->l3_clk;

	ret = gpmc_parse_dt(dev, priv);
	if (ret)
		return ret;

	priv->capability_flags = dev->driver->of_match->data;
	gpmc_capability = priv->capability_flags;

	gpmc_probe_dt_children(dev);

	return 0;
}

#define GPMC_DATA_REV2_4 0
#define GPMC_DATA_REV5 (GPMC_HAS_WR_ACCESS | GPMC_HAS_WR_DATA_MUX_BUS)
#define GPMC_DATA_REV6 (GPMC_HAS_WR_ACCESS | GPMC_HAS_WR_DATA_MUX_BUS | GPMC_HAS_MUX_AAD)

static const struct udevice_id gpmc_dt_ids[] = {
	{ .compatible = "ti,am64-gpmc", .data = GPMC_DATA_REV6, },
	{ .compatible = "ti,am3352-gpmc", .data = GPMC_DATA_REV5, },
	{ .compatible = "ti,omap2420-gpmc", .data = GPMC_DATA_REV2_4, },
	{ .compatible = "ti,omap2430-gpmc", .data = GPMC_DATA_REV2_4, },
	{ .compatible = "ti,omap3430-gpmc", .data = GPMC_DATA_REV5, },
	{ .compatible = "ti,omap4430-gpmc", .data = GPMC_DATA_REV6, },
	{ } /* sentinel */
};

U_BOOT_DRIVER(ti_gpmc) = {
	.name   = "ti-gpmc",
	.id     = UCLASS_MEMORY,
	.of_match = gpmc_dt_ids,
	.probe  = gpmc_probe,
	.flags  = DM_FLAG_ALLOC_PRIV_DMA,
};
