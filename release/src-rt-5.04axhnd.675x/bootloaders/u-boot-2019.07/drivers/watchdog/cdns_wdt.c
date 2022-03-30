// SPDX-License-Identifier: GPL-2.0
/*
 * Cadence WDT driver - Used by Xilinx Zynq
 * Reference: Linux kernel Cadence watchdog driver.
 *
 * Author(s):	Shreenidhi Shedi <yesshedi@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <clk.h>
#include <div64.h>
#include <linux/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct cdns_regs {
	u32 zmr;	/* WD Zero mode register, offset - 0x0 */
	u32 ccr;	/* Counter Control Register offset - 0x4 */
	u32 restart;	/* Restart key register, offset - 0x8 */
	u32 status;	/* Status Register, offset - 0xC */
};

struct cdns_wdt_priv {
	bool rst;
	struct cdns_regs *regs;
};

#define CDNS_WDT_DEFAULT_TIMEOUT	10

/* Supports 1 - 516 sec */
#define CDNS_WDT_MIN_TIMEOUT		1
#define CDNS_WDT_MAX_TIMEOUT		516

/* Restart key */
#define CDNS_WDT_RESTART_KEY		0x00001999

/* Counter register access key */
#define CDNS_WDT_REGISTER_ACCESS_KEY	0x00920000

/* Counter value divisor */
#define CDNS_WDT_COUNTER_VALUE_DIVISOR	0x1000

/* Clock prescaler value and selection */
#define CDNS_WDT_PRESCALE_64		64
#define CDNS_WDT_PRESCALE_512		512
#define CDNS_WDT_PRESCALE_4096		4096
#define CDNS_WDT_PRESCALE_SELECT_64	1
#define CDNS_WDT_PRESCALE_SELECT_512	2
#define CDNS_WDT_PRESCALE_SELECT_4096	3

/* Input clock frequency */
#define CDNS_WDT_CLK_75MHZ	75000000

/* Counter maximum value */
#define CDNS_WDT_COUNTER_MAX	0xFFF

/*********************    Register Map    **********************************/

/*
 * Zero Mode Register - This register controls how the time out is indicated
 * and also contains the access code to allow writes to the register (0xABC).
 */
#define CDNS_WDT_ZMR_WDEN_MASK	0x00000001 /* Enable the WDT */
#define CDNS_WDT_ZMR_RSTEN_MASK	0x00000002 /* Enable the reset output */
#define CDNS_WDT_ZMR_IRQEN_MASK	0x00000004 /* Enable IRQ output */
#define CDNS_WDT_ZMR_RSTLEN_16	0x00000030 /* Reset pulse of 16 pclk cycles */
#define CDNS_WDT_ZMR_ZKEY_VAL	0x00ABC000 /* Access key, 0xABC << 12 */

/*
 * Counter Control register - This register controls how fast the timer runs
 * and the reset value and also contains the access code to allow writes to
 * the register.
 */
#define CDNS_WDT_CCR_CRV_MASK	0x00003FFC /* Counter reset value */

/* Write access to Registers */
static inline void cdns_wdt_writereg(u32 *addr, u32 val)
{
	writel(val, addr);
}

/**
 * cdns_wdt_reset - Reload the watchdog timer (i.e. pat the watchdog).
 *
 * @dev: Watchdog device
 *
 * Write the restart key value (0x00001999) to the restart register.
 *
 * Return: Always 0
 */
static int cdns_wdt_reset(struct udevice *dev)
{
	struct cdns_wdt_priv *priv = dev_get_priv(dev);

	debug("%s\n", __func__);

	cdns_wdt_writereg(&priv->regs->restart, CDNS_WDT_RESTART_KEY);

	return 0;
}

/**
 * cdns_wdt_start - Enable and start the watchdog.
 *
 * @dev: Watchdog device
 * @timeout: Timeout value
 * @flags: Driver flags
 *
 * The counter value is calculated according to the formula:
 *		count = (timeout * clock) / prescaler + 1.
 *
 * The calculated count is divided by 0x1000 to obtain the field value
 * to write to counter control register.
 *
 * Clears the contents of prescaler and counter reset value. Sets the
 * prescaler to 4096 and the calculated count and access key
 * to write to CCR Register.
 *
 * Sets the WDT (WDEN bit) and either the Reset signal(RSTEN bit)
 * or Interrupt signal(IRQEN) with a specified cycles and the access
 * key to write to ZMR Register.
 *
 * Return: Upon success 0, failure -1.
 */
static int cdns_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	ulong clk_f;
	u32 count, prescaler, ctrl_clksel, data = 0;
	struct clk clock;
	struct cdns_wdt_priv *priv = dev_get_priv(dev);

	if (clk_get_by_index(dev, 0, &clock) < 0) {
		dev_err(dev, "failed to get clock\n");
		return -1;
	}

	clk_f = clk_get_rate(&clock);
	if (IS_ERR_VALUE(clk_f)) {
		dev_err(dev, "failed to get rate\n");
		return -1;
	}

	/* Calculate timeout in seconds and restrict to min and max value */
	do_div(timeout, 1000);
	timeout = max_t(u64, timeout, CDNS_WDT_MIN_TIMEOUT);
	timeout = min_t(u64, timeout, CDNS_WDT_MAX_TIMEOUT);

	debug("%s: CLK_FREQ %ld, timeout %lld\n", __func__, clk_f, timeout);

	if (clk_f <= CDNS_WDT_CLK_75MHZ) {
		prescaler = CDNS_WDT_PRESCALE_512;
		ctrl_clksel = CDNS_WDT_PRESCALE_SELECT_512;
	} else {
		prescaler = CDNS_WDT_PRESCALE_4096;
		ctrl_clksel = CDNS_WDT_PRESCALE_SELECT_4096;
	}

	/*
	 * Counter value divisor to obtain the value of
	 * counter reset to be written to control register.
	 */
	count = (timeout * (clk_f / prescaler)) /
		CDNS_WDT_COUNTER_VALUE_DIVISOR + 1;

	if (count > CDNS_WDT_COUNTER_MAX)
		count = CDNS_WDT_COUNTER_MAX;

	cdns_wdt_writereg(&priv->regs->zmr, CDNS_WDT_ZMR_ZKEY_VAL);

	count = (count << 2) & CDNS_WDT_CCR_CRV_MASK;

	/* Write counter access key first to be able write to register */
	data = count | CDNS_WDT_REGISTER_ACCESS_KEY | ctrl_clksel;
	cdns_wdt_writereg(&priv->regs->ccr, data);

	data = CDNS_WDT_ZMR_WDEN_MASK | CDNS_WDT_ZMR_RSTLEN_16 |
		CDNS_WDT_ZMR_ZKEY_VAL;

	/* Reset on timeout if specified in device tree. */
	if (priv->rst) {
		data |= CDNS_WDT_ZMR_RSTEN_MASK;
		data &= ~CDNS_WDT_ZMR_IRQEN_MASK;
	} else {
		data &= ~CDNS_WDT_ZMR_RSTEN_MASK;
		data |= CDNS_WDT_ZMR_IRQEN_MASK;
	}

	cdns_wdt_writereg(&priv->regs->zmr, data);
	cdns_wdt_writereg(&priv->regs->restart, CDNS_WDT_RESTART_KEY);

	return 0;
}

/**
 * cdns_wdt_stop - Stop the watchdog.
 *
 * @dev: Watchdog device
 *
 * Read the contents of the ZMR register, clear the WDEN bit in the register
 * and set the access key for successful write.
 *
 * Return: Always 0
 */
static int cdns_wdt_stop(struct udevice *dev)
{
	struct cdns_wdt_priv *priv = dev_get_priv(dev);

	cdns_wdt_writereg(&priv->regs->zmr,
			  CDNS_WDT_ZMR_ZKEY_VAL & (~CDNS_WDT_ZMR_WDEN_MASK));

	return 0;
}

/**
 * cdns_wdt_probe - Probe call for the device.
 *
 * @dev: Handle to the udevice structure.
 *
 * Return: Always 0.
 */
static int cdns_wdt_probe(struct udevice *dev)
{
	debug("%s: Probing wdt%u\n", __func__, dev->seq);

	return 0;
}

static int cdns_wdt_ofdata_to_platdata(struct udevice *dev)
{
	struct cdns_wdt_priv *priv = dev_get_priv(dev);

	priv->regs = (struct cdns_regs *)dev_read_addr(dev);
	if (IS_ERR(priv->regs))
		return PTR_ERR(priv->regs);

	priv->rst = dev_read_bool(dev, "reset-on-timeout");

	debug("%s: reset %d\n", __func__, priv->rst);

	return 0;
}

static const struct wdt_ops cdns_wdt_ops = {
	.start = cdns_wdt_start,
	.reset = cdns_wdt_reset,
	.stop = cdns_wdt_stop,
	/* There is no bit/reg/support in IP for expire_now functionality */
};

static const struct udevice_id cdns_wdt_ids[] = {
	{ .compatible = "cdns,wdt-r1p2" },
	{}
};

U_BOOT_DRIVER(cdns_wdt) = {
	.name = "cdns_wdt",
	.id = UCLASS_WDT,
	.of_match = cdns_wdt_ids,
	.probe = cdns_wdt_probe,
	.priv_auto_alloc_size = sizeof(struct cdns_wdt_priv),
	.ofdata_to_platdata = cdns_wdt_ofdata_to_platdata,
	.ops = &cdns_wdt_ops,
};
