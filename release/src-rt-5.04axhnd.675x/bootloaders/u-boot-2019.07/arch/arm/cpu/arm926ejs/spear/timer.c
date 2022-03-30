// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_gpt.h>
#include <asm/arch/spr_misc.h>

#define GPT_RESOLUTION	(CONFIG_SPEAR_HZ_CLOCK / CONFIG_SPEAR_HZ)
#define READ_TIMER()	(readl(&gpt_regs_p->count) & GPT_FREE_RUNNING)

static struct gpt_regs *const gpt_regs_p =
    (struct gpt_regs *)CONFIG_SPEAR_TIMERBASE;

static struct misc_regs *const misc_regs_p =
    (struct misc_regs *)CONFIG_SPEAR_MISCBASE;

DECLARE_GLOBAL_DATA_PTR;

static ulong get_timer_masked(void);

#define timestamp gd->arch.tbl
#define lastdec gd->arch.lastinc

int timer_init(void)
{
	u32 synth;

	/* Prescaler setting */
#if defined(CONFIG_SPEAR3XX)
	writel(MISC_PRSC_CFG, &misc_regs_p->prsc2_clk_cfg);
	synth = MISC_GPT4SYNTH;
#elif defined(CONFIG_SPEAR600)
	writel(MISC_PRSC_CFG, &misc_regs_p->prsc1_clk_cfg);
	synth = MISC_GPT3SYNTH;
#else
# error Incorrect config. Can only be SPEAR{600|300|310|320}
#endif

	writel(readl(&misc_regs_p->periph_clk_cfg) | synth,
	       &misc_regs_p->periph_clk_cfg);

	/* disable timers */
	writel(GPT_PRESCALER_1 | GPT_MODE_AUTO_RELOAD, &gpt_regs_p->control);

	/* load value for free running */
	writel(GPT_FREE_RUNNING, &gpt_regs_p->compare);

	/* auto reload, start timer */
	writel(readl(&gpt_regs_p->control) | GPT_ENABLE, &gpt_regs_p->control);

	/* Reset the timer */
	lastdec = READ_TIMER();
	timestamp = 0;

	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer(ulong base)
{
	return (get_timer_masked() / GPT_RESOLUTION) - base;
}

void __udelay(unsigned long usec)
{
	ulong tmo;
	ulong start = get_timer_masked();
	ulong tenudelcnt = CONFIG_SPEAR_HZ_CLOCK / (1000 * 100);
	ulong rndoff;

	rndoff = (usec % 10) ? 1 : 0;

	/* tenudelcnt timer tick gives 10 microsecconds delay */
	tmo = ((usec / 10) + rndoff) * tenudelcnt;

	while ((ulong) (get_timer_masked() - start) < tmo)
		;
}

static ulong get_timer_masked(void)
{
	ulong now = READ_TIMER();

	if (now >= lastdec) {
		/* normal mode */
		timestamp += now - lastdec;
	} else {
		/* we have an overflow ... */
		timestamp += now + GPT_FREE_RUNNING - lastdec;
	}
	lastdec = now;

	return timestamp;
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SPEAR_HZ;
}
