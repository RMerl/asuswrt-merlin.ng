// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 CS Systemes d'Information
 */

#include <common.h>
#include <env.h>
#include <dm.h>
#include <wdt.h>
#include <clock_legacy.h>
#include <asm/io.h>

struct mpc8xxx_wdt {
	__be32 res0;
	__be32 swcrr;	/* System watchdog control register */
#define SWCRR_SWTC 0xFFFF0000 /* Software Watchdog Time Count. */
#define SWCRR_BME  0x00000080 /* Bus monitor enable (mpc8xx) */
#define SWCRR_SWF  0x00000008 /* Software Watchdog Freeze (mpc8xx). */
#define SWCRR_SWEN 0x00000004 /* Watchdog Enable bit. */
#define SWCRR_SWRI 0x00000002 /* Software Watchdog Reset/Interrupt Select bit.*/
#define SWCRR_SWPR 0x00000001 /* Software Watchdog Counter Prescale bit. */
	__be32 swcnr;	/* System watchdog count register */
	u8 res1[2];
	__be16 swsrr;	/* System watchdog service register */
	u8 res2[0xf0];
};

struct mpc8xxx_wdt_priv {
	struct mpc8xxx_wdt __iomem *base;
};

static int mpc8xxx_wdt_reset(struct udevice *dev)
{
	struct mpc8xxx_wdt_priv *priv = dev_get_priv(dev);

	out_be16(&priv->base->swsrr, 0x556c);	/* write magic1 */
	out_be16(&priv->base->swsrr, 0xaa39);	/* write magic2 */

	return 0;
}

static int mpc8xxx_wdt_start(struct udevice *dev, u64 timeout, ulong flags)
{
	struct mpc8xxx_wdt_priv *priv = dev_get_priv(dev);
	const char *mode = env_get("watchdog_mode");
	ulong prescaler = dev_get_driver_data(dev);
	u16 swtc = min_t(u16, timeout * get_board_sys_clk() / 1000 / prescaler, U16_MAX);
	u32 val;

	mpc8xxx_wdt_reset(dev);

	if (strcmp(mode, "off") == 0)
		val = (swtc << 16) | SWCRR_SWPR;
	else if (strcmp(mode, "nmi") == 0)
		val = (swtc << 16) | SWCRR_SWPR | SWCRR_SWEN;
	else
		val = (swtc << 16) | SWCRR_SWPR | SWCRR_SWEN | SWCRR_SWRI;

	if (IS_ENABLED(CONFIG_WDT_MPC8xxx_BME))
		val |= (CONFIG_WDT_MPC8xxx_BMT << 8) | SWCRR_BME;

	out_be32(&priv->base->swcrr, val);

	if (!(in_be32(&priv->base->swcrr) & SWCRR_SWEN))
		return -EBUSY;
	return 0;

}

static int mpc8xxx_wdt_stop(struct udevice *dev)
{
	struct mpc8xxx_wdt_priv *priv = dev_get_priv(dev);

	clrbits_be32(&priv->base->swcrr, SWCRR_SWEN);

	if (in_be32(&priv->base->swcrr) & SWCRR_SWEN)
		return -EBUSY;
	return 0;
}

static int mpc8xxx_wdt_of_to_plat(struct udevice *dev)
{
	struct mpc8xxx_wdt_priv *priv = dev_get_priv(dev);

	priv->base = (void __iomem *)devfdt_remap_addr(dev);

	if (!priv->base)
		return -EINVAL;

	return 0;
}

static const struct wdt_ops mpc8xxx_wdt_ops = {
	.start = mpc8xxx_wdt_start,
	.reset = mpc8xxx_wdt_reset,
	.stop = mpc8xxx_wdt_stop,
};

static const struct udevice_id mpc8xxx_wdt_ids[] = {
	{ .compatible = "fsl,pq1-wdt", .data = 0x800 },
	{ .compatible = "fsl,pq2pro-wdt", .data = 0x10000 },
	{}
};

U_BOOT_DRIVER(wdt_mpc8xxx) = {
	.name = "wdt_mpc8xxx",
	.id = UCLASS_WDT,
	.of_match = mpc8xxx_wdt_ids,
	.ops = &mpc8xxx_wdt_ops,
	.of_to_plat = mpc8xxx_wdt_of_to_plat,
	.priv_auto	= sizeof(struct mpc8xxx_wdt_priv),
};
