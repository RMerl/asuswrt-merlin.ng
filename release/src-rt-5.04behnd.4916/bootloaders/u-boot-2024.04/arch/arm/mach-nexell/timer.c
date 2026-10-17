// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * Hyunseok, Jung <hsjung@nexell.co.kr>
 */

#include <common.h>
#include <log.h>

#include <asm/io.h>
#include <asm/arch/nexell.h>
#include <asm/arch/clk.h>
#if defined(CONFIG_ARCH_S5P4418)
#include <asm/arch/reset.h>
#endif

#if (CONFIG_TIMER_SYS_TICK_CH > 3)
#error Not support timer channel. Please use "0~3" channels.
#endif

/* global variables to save timer count
 *
 * Section ".data" must be used because BSS is not available before relocation,
 * in board_init_f(), respectively! I.e. global variables can not be used!
 */
static unsigned long timestamp __section(".data");
static unsigned long lastdec __section(".data");
static int	timerinit __section(".data");

/* macro to hw timer tick config */
static long	TIMER_FREQ  = 1000000;
static long	TIMER_HZ    = 1000000 / CONFIG_SYS_HZ;
static long	TIMER_COUNT = 0xFFFFFFFF;

#define	REG_TCFG0			(0x00)
#define	REG_TCFG1			(0x04)
#define	REG_TCON			(0x08)
#define	REG_TCNTB0			(0x0C)
#define	REG_TCMPB0			(0x10)
#define	REG_TCNT0			(0x14)
#define	REG_CSTAT			(0x44)

#define	TCON_BIT_AUTO			(1 << 3)
#define	TCON_BIT_INVT			(1 << 2)
#define	TCON_BIT_UP			(1 << 1)
#define	TCON_BIT_RUN			(1 << 0)
#define TCFG0_BIT_CH(ch)		((ch) == 0 || (ch) == 1 ? 0 : 8)
#define TCFG1_BIT_CH(ch)		((ch) * 4)
#define TCON_BIT_CH(ch)			((ch) ? (ch) * 4  + 4 : 0)
#define TINT_CH(ch)			(ch)
#define TINT_CSTAT_BIT_CH(ch)		((ch) + 5)
#define	TINT_CSTAT_MASK			(0x1F)
#define TIMER_TCNT_OFFS			(0xC)

void reset_timer_masked(void);
unsigned long get_timer_masked(void);

/*
 * Timer HW
 */
static inline void timer_clock(void __iomem *base, int ch, int mux, int scl)
{
	u32 val = readl(base + REG_TCFG0) & ~(0xFF << TCFG0_BIT_CH(ch));

	writel(val | ((scl - 1) << TCFG0_BIT_CH(ch)), base + REG_TCFG0);
	val = readl(base + REG_TCFG1) & ~(0xF << TCFG1_BIT_CH(ch));
	writel(val | (mux << TCFG1_BIT_CH(ch)), base + REG_TCFG1);
}

static inline void timer_count(void __iomem *base, int ch, unsigned int cnt)
{
	writel((cnt - 1), base + REG_TCNTB0 + (TIMER_TCNT_OFFS * ch));
	writel((cnt - 1), base + REG_TCMPB0 + (TIMER_TCNT_OFFS * ch));
}

static inline void timer_start(void __iomem *base, int ch)
{
	int on = 0;
	u32 val = readl(base + REG_CSTAT) & ~(TINT_CSTAT_MASK << 5 | 0x1 << ch);

	writel(val | (0x1 << TINT_CSTAT_BIT_CH(ch) | on << ch),
	       base + REG_CSTAT);
	val = readl(base + REG_TCON) & ~(0xE << TCON_BIT_CH(ch));
	writel(val | (TCON_BIT_UP << TCON_BIT_CH(ch)), base + REG_TCON);

	val &= ~(TCON_BIT_UP << TCON_BIT_CH(ch));
	val |= ((TCON_BIT_AUTO | TCON_BIT_RUN) << TCON_BIT_CH(ch));
	writel(val, base + REG_TCON);
	dmb();
}

static inline void timer_stop(void __iomem *base, int ch)
{
	int on = 0;
	u32 val = readl(base + REG_CSTAT) & ~(TINT_CSTAT_MASK << 5 | 0x1 << ch);

	writel(val | (0x1 << TINT_CSTAT_BIT_CH(ch) | on << ch),
	       base + REG_CSTAT);
	val = readl(base + REG_TCON) & ~(TCON_BIT_RUN << TCON_BIT_CH(ch));
	writel(val, base + REG_TCON);
}

static inline unsigned long timer_read(void __iomem *base, int ch)
{
	unsigned long ret;

	ret = TIMER_COUNT - readl(base + REG_TCNT0 + (TIMER_TCNT_OFFS * ch));
	return ret;
}

int timer_init(void)
{
	struct clk *clk = NULL;
	char name[16] = "pclk";
	int ch = CONFIG_TIMER_SYS_TICK_CH;
	unsigned long rate, tclk = 0;
	unsigned long mout, thz, cmp = -1UL;
	int tcnt, tscl = 0, tmux = 0;
	int mux = 0, scl = 0;
	void __iomem *base = (void __iomem *)PHY_BASEADDR_TIMER;

	if (timerinit)
		return 0;

	/* get with PCLK */
	clk  = clk_get(name);
	rate = clk_get_rate(clk);
	for (mux = 0; mux < 5; mux++) {
		mout = rate / (1 << mux), scl = mout / TIMER_FREQ,
		thz = mout / scl;
		if (!(mout % TIMER_FREQ) && 256 > scl) {
			tclk = thz, tmux = mux, tscl = scl;
			break;
		}
		if (scl > 256)
			continue;
		if (abs(thz - TIMER_FREQ) >= cmp)
			continue;
		tclk = thz, tmux = mux, tscl = scl;
		cmp = abs(thz - TIMER_FREQ);
	}
	tcnt = tclk;	/* Timer Count := 1 Mhz counting */

	TIMER_FREQ = tcnt;	/* Timer Count := 1 Mhz counting */
	TIMER_HZ = TIMER_FREQ / CONFIG_SYS_HZ;
	tcnt = TIMER_COUNT == 0xFFFFFFFF ? TIMER_COUNT + 1 : tcnt;

	timer_stop(base, ch);
	timer_clock(base, ch, tmux, tscl);
	timer_count(base, ch, tcnt);
	timer_start(base, ch);

	reset_timer_masked();
	timerinit = 1;

	return 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

unsigned long get_timer(unsigned long base)
{
	long ret;
	unsigned long time = get_timer_masked();
	unsigned long hz = TIMER_HZ;

	ret = time / hz - base;
	return ret;
}

void set_timer(unsigned long t)
{
	timestamp = (unsigned long)t;
}

void reset_timer_masked(void)
{
	void __iomem *base = (void __iomem *)PHY_BASEADDR_TIMER;
	int ch = CONFIG_TIMER_SYS_TICK_CH;

	/* reset time */
	/* capure current decrementer value time */
	lastdec = timer_read(base, ch);
	/* start "advancing" time stamp from 0 */
	timestamp = 0;
}

unsigned long get_timer_masked(void)
{
	void __iomem *base = (void __iomem *)PHY_BASEADDR_TIMER;
	int ch = CONFIG_TIMER_SYS_TICK_CH;

	unsigned long now = timer_read(base, ch); /* current tick value */

	if (now >= lastdec) {			  /* normal mode (non roll) */
		/* move stamp fordward with absolute diff ticks */
		timestamp += now - lastdec;
	} else {
		/* we have overflow of the count down timer */
		/* nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...
		 * it could also roll and cause problems.
		 */
		timestamp += now + TIMER_COUNT - lastdec;
	}
	/* save last */
	lastdec = now;

	debug("now=%lu, last=%lu, timestamp=%lu\n", now, lastdec, timestamp);
	return (unsigned long)timestamp;
}

void __udelay(unsigned long usec)
{
	unsigned long tmo, tmp;

	debug("+udelay=%ld\n", usec);

	if (!timerinit)
		timer_init();

	/* if "big" number, spread normalization to seconds */
	if (usec >= 1000) {
		/* start to normalize for usec to ticks per sec */
		tmo  = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= TIMER_FREQ;
		/* finish normalize. */
		tmo /= 1000;
	/* else small number, don't kill it prior to HZ multiply */
	} else {
		tmo = usec * TIMER_FREQ;
		tmo /= (1000 * 1000);
	}

	tmp = get_timer_masked();	/* get current timestamp */
	debug("A. tmo=%ld, tmp=%ld\n", tmo, tmp);

	/* if setting this fordward will roll time stamp */
	if (tmp > (tmo + tmp + 1))
		/* reset "advancing" timestamp to 0, set lastdec value */
		reset_timer_masked();
	else
		/* set advancing stamp wake up time */
		tmo += tmp;

	debug("B. tmo=%ld, tmp=%ld\n", tmo, tmp);

	/* loop till event */
	do {
		tmp = get_timer_masked();
	} while (tmo > tmp);
	debug("-udelay=%ld\n", usec);
}

void udelay_masked(unsigned long usec)
{
	unsigned long tmo, endtime;
	signed long diff;

	/* if "big" number, spread normalization to seconds */
	if (usec >= 1000) {
		/* start to normalize for usec to ticks per sec */
		tmo = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= TIMER_FREQ;
		/* finish normalize. */
		tmo /= 1000;
	} else { /* else small number, don't kill it prior to HZ multiply */
		tmo = usec * TIMER_FREQ;
		tmo /= (1000 * 1000);
	}

	endtime = get_timer_masked() + tmo;

	do {
		unsigned long now = get_timer_masked();

		diff = endtime - now;
	} while (diff >= 0);
}

unsigned long long get_ticks(void)
{
	return get_timer_masked();
}

#if defined(CONFIG_ARCH_S5P4418)
ulong get_tbclk(void)
{
	ulong  tbclk = TIMER_FREQ;
	return tbclk;
}
#endif
