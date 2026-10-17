// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <irq_func.h>
#include <log.h>
#include <status_led.h>
#include <sysinfo.h>
#include <time.h>
#include <timer.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CFG_SYS_WATCHDOG_FREQ
#define CFG_SYS_WATCHDOG_FREQ (CONFIG_SYS_HZ / 2)
#endif

/**
 * struct mpc83xx_timer_priv - Private data structure for MPC83xx timer driver
 * @decrementer_count: Value to which the decrementer register should be re-set
 *		       to when a timer interrupt occurs, thus determines the
 *		       interrupt frequency (value for 1e6/HZ microseconds)
 * @timestamp:         Counter for the number of timer interrupts that have
 *		       occurred (i.e. can be used to trigger events
 *		       periodically in the timer interrupt)
 */
struct mpc83xx_timer_priv {
	uint decrementer_count;
	ulong timestamp;
};

/*
 * Bitmask for enabling the time base in the SPCR (System Priority
 * Configuration Register)
 */
static const u32 SPCR_TBEN_MASK = BIT(31 - 9);

/**
 * get_dec() - Get the value of the decrementer register
 *
 * Return: The value of the decrementer register
 */
static inline unsigned long get_dec(void)
{
	unsigned long val;

	asm volatile ("mfdec %0" : "=r" (val) : );

	return val;
}

/**
 * set_dec() - Set the value of the decrementer register
 * @val: The value of the decrementer register to be set
 */
static inline void set_dec(unsigned long val)
{
	if (val)
		asm volatile ("mtdec %0"::"r" (val));
}

/**
 * mftbu() - Get value of TBU (upper time base) register
 *
 * Return: Value of the TBU register
 */
static inline u32 mftbu(void)
{
	u32 rval;

	asm volatile("mftbu %0" : "=r" (rval));
	return rval;
}

/**
 * mftb() - Get value of TBL (lower time base) register
 *
 * Return: Value of the TBL register
 */
static inline u32 mftb(void)
{
	u32 rval;

	asm volatile("mftb %0" : "=r" (rval));
	return rval;
}

/*
 * TODO(mario.six@gdsys.cc): This should really be done by timer_init, and the
 * interrupt init should go into a interrupt driver.
 */
int interrupt_init(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	struct udevice *csb;
	struct udevice *sysinfo;
	struct udevice *timer;
	struct mpc83xx_timer_priv *timer_priv;
	struct clk clock;
	int ret;

	ret = uclass_first_device_err(UCLASS_TIMER, &timer);
	if (ret) {
		debug("%s: Could not find timer device (error: %d)",
		      __func__, ret);
		return ret;
	}

	timer_priv = dev_get_priv(timer);

	if (sysinfo_get(&sysinfo)) {
		debug("%s: sysinfo device could not be fetched.\n", __func__);
		return -ENOENT;
	}

	ret = uclass_get_device_by_phandle(UCLASS_SIMPLE_BUS, sysinfo,
					   "csb", &csb);
	if (ret) {
		debug("%s: Could not retrieve CSB device (error: %d)",
		      __func__, ret);
		return ret;
	}

	ret = clk_get_by_index(csb, 0, &clock);
	if (ret) {
		debug("%s: Could not retrieve clock (error: %d)",
		      __func__, ret);
		return ret;
	}

	timer_priv->decrementer_count = (clk_get_rate(&clock) / 4)
					/ CONFIG_SYS_HZ;
	/* Enable e300 time base */
	setbits_be32(&immr->sysconf.spcr, SPCR_TBEN_MASK);

	set_dec(timer_priv->decrementer_count);

	/* Switch on interrupts */
	set_msr(get_msr() | MSR_EE);

	return 0;
}

/**
 * timer_interrupt() - Handler for the timer interrupt
 * @regs: Array of register values
 */
void timer_interrupt(struct pt_regs *regs)
{
	struct udevice *timer = gd->timer;
	struct mpc83xx_timer_priv *priv;

	/*
	 * During initialization, gd->timer might not be set yet, but the timer
	 * interrupt may already be enabled. In this case, wait for the
	 * initialization to complete
	 */
	if (!timer)
		return;

	priv = dev_get_priv(timer);

	/* Restore Decrementer Count */
	set_dec(priv->decrementer_count);

	priv->timestamp++;

#if defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
	if (CFG_SYS_WATCHDOG_FREQ && (priv->timestamp % (CFG_SYS_WATCHDOG_FREQ)) == 0)
		schedule();
#endif    /* CONFIG_WATCHDOG || CONFIG_HW_WATCHDOG */

#ifdef CONFIG_LED_STATUS
	status_led_tick(priv->timestamp);
#endif /* CONFIG_LED_STATUS */
}

void wait_ticks(ulong ticks)
{
	ulong end = get_ticks() + ticks;

	while (end > get_ticks())
		schedule();
}

static u64 mpc83xx_timer_get_count(struct udevice *dev)
{
	u32 tbu, tbl;

	/*
	 * To make sure that no tbl overflow occurred between reading tbl and
	 * tbu, read tbu again, and compare it with the previously read tbu
	 * value: If they're different, a tbl overflow has occurred.
	 */
	do {
		tbu = mftbu();
		tbl = mftb();
	} while (tbu != mftbu());

	return (tbu * 0x10000ULL) + tbl;
}

static int mpc83xx_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct clk clock;
	int ret;

	ret = interrupt_init();
	if (ret) {
		debug("%s: interrupt_init failed (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	ret = clk_get_by_index(dev, 0, &clock);
	if (ret) {
		debug("%s: Could not retrieve clock (err = %d)\n",
		      dev->name, ret);
		return ret;
	}

	uc_priv->clock_rate = (clk_get_rate(&clock) + 3L) / 4L;

	return 0;
}

static const struct timer_ops mpc83xx_timer_ops = {
	.get_count = mpc83xx_timer_get_count,
};

static const struct udevice_id mpc83xx_timer_ids[] = {
	{ .compatible = "fsl,mpc83xx-timer" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mpc83xx_timer) = {
	.name	= "mpc83xx_timer",
	.id	= UCLASS_TIMER,
	.of_match = mpc83xx_timer_ids,
	.probe = mpc83xx_timer_probe,
	.ops	= &mpc83xx_timer_ops,
	.priv_auto	= sizeof(struct mpc83xx_timer_priv),
};
