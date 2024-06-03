// SPDX-License-Identifier: GPL-2.0+
/*
 * Marvell Armada 37xx SoC Watchdog Driver
 *
 * Marek Behun <marek.behun@nic.cz>
 */

#include <common.h>
#include <dm.h>
#include <wdt.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

struct a37xx_wdt {
	void __iomem *sel_reg;
	void __iomem *reg;
	ulong clk_rate;
	u64 timeout;
};

/*
 * We use Counter 1 as watchdog timer, and Counter 0 for re-triggering Counter 1
 */

#define CNTR_CTRL(id)			((id) * 0x10)
#define CNTR_CTRL_ENABLE		0x0001
#define CNTR_CTRL_ACTIVE		0x0002
#define CNTR_CTRL_MODE_MASK		0x000c
#define CNTR_CTRL_MODE_ONESHOT		0x0000
#define CNTR_CTRL_MODE_HWSIG		0x000c
#define CNTR_CTRL_TRIG_SRC_MASK		0x00f0
#define CNTR_CTRL_TRIG_SRC_PREV_CNTR	0x0050
#define CNTR_CTRL_PRESCALE_MASK		0xff00
#define CNTR_CTRL_PRESCALE_MIN		2
#define CNTR_CTRL_PRESCALE_SHIFT	8

#define CNTR_COUNT_LOW(id)		(CNTR_CTRL(id) + 0x4)
#define CNTR_COUNT_HIGH(id)		(CNTR_CTRL(id) + 0x8)

static void set_counter_value(struct a37xx_wdt *priv, int id, u64 val)
{
	writel(val & 0xffffffff, priv->reg + CNTR_COUNT_LOW(id));
	writel(val >> 32, priv->reg + CNTR_COUNT_HIGH(id));
}

static void counter_enable(struct a37xx_wdt *priv, int id)
{
	setbits_le32(priv->reg + CNTR_CTRL(id), CNTR_CTRL_ENABLE);
}

static void counter_disable(struct a37xx_wdt *priv, int id)
{
	clrbits_le32(priv->reg + CNTR_CTRL(id), CNTR_CTRL_ENABLE);
}

static int init_counter(struct a37xx_wdt *priv, int id, u32 mode, u32 trig_src)
{
	u32 reg;

	reg = readl(priv->reg + CNTR_CTRL(id));
	if (reg & CNTR_CTRL_ACTIVE)
		return -EBUSY;

	reg &= ~(CNTR_CTRL_MODE_MASK | CNTR_CTRL_PRESCALE_MASK |
		 CNTR_CTRL_TRIG_SRC_MASK);

	/* set mode */
	reg |= mode;

	/* set prescaler to the min value */
	reg |= CNTR_CTRL_PRESCALE_MIN << CNTR_CTRL_PRESCALE_SHIFT;

	/* set trigger source */
	reg |= trig_src;

	writel(reg, priv->reg + CNTR_CTRL(id));

	return 0;
}

static int a37xx_wdt_reset(struct udevice *dev)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);

	if (!priv->timeout)
		return -EINVAL;

	/* counter 1 is retriggered by forcing end count on counter 0 */
	counter_disable(priv, 0);
	counter_enable(priv, 0);

	return 0;
}

static int a37xx_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);

	/* first we set timeout to 0 */
	counter_disable(priv, 1);
	set_counter_value(priv, 1, 0);
	counter_enable(priv, 1);

	/* and then we start counter 1 by forcing end count on counter 0 */
	counter_disable(priv, 0);
	counter_enable(priv, 0);

	return 0;
}

static int a37xx_wdt_start(struct udevice *dev, u64 ms, ulong flags)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);
	int err;

	err = init_counter(priv, 0, CNTR_CTRL_MODE_ONESHOT, 0);
	if (err < 0)
		return err;

	err = init_counter(priv, 1, CNTR_CTRL_MODE_HWSIG,
			   CNTR_CTRL_TRIG_SRC_PREV_CNTR);
	if (err < 0)
		return err;

	priv->timeout = ms * priv->clk_rate / 1000 / CNTR_CTRL_PRESCALE_MIN;

	set_counter_value(priv, 0, 0);
	set_counter_value(priv, 1, priv->timeout);
	counter_enable(priv, 1);

	/* we have to force end count on counter 0 to start counter 1 */
	counter_enable(priv, 0);

	return 0;
}

static int a37xx_wdt_stop(struct udevice *dev)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);

	counter_disable(priv, 1);
	counter_disable(priv, 0);
	writel(0, priv->sel_reg);

	return 0;
}

static int a37xx_wdt_probe(struct udevice *dev)
{
	struct a37xx_wdt *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE)
		goto err;
	priv->sel_reg = (void __iomem *)addr;

	addr = dev_read_addr_index(dev, 1);
	if (addr == FDT_ADDR_T_NONE)
		goto err;
	priv->reg = (void __iomem *)addr;

	priv->clk_rate = (ulong)get_ref_clk() * 1000000;

	/*
	 * We use counter 1 as watchdog timer, therefore we only set bit
	 * TIMER1_IS_WCHDOG_TIMER. Counter 0 is only used to force re-trigger on
	 * counter 1.
	 */
	writel(1 << 1, priv->sel_reg);

	return 0;
err:
	dev_err(dev, "no io address\n");
	return -ENODEV;
}

static const struct wdt_ops a37xx_wdt_ops = {
	.start = a37xx_wdt_start,
	.reset = a37xx_wdt_reset,
	.stop = a37xx_wdt_stop,
	.expire_now = a37xx_wdt_expire_now,
};

static const struct udevice_id a37xx_wdt_ids[] = {
	{ .compatible = "marvell,armada-3700-wdt" },
	{}
};

U_BOOT_DRIVER(a37xx_wdt) = {
	.name = "armada_37xx_wdt",
	.id = UCLASS_WDT,
	.of_match = a37xx_wdt_ids,
	.probe = a37xx_wdt_probe,
	.priv_auto_alloc_size = sizeof(struct a37xx_wdt),
	.ops = &a37xx_wdt_ops,
};
