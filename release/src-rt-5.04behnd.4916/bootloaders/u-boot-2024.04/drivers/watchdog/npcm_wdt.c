// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology, Inc
 */

#include <dm.h>
#include <errno.h>
#include <log.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/err.h>

#define NPCM_WTCLK	(BIT(10) | BIT(11))	/* Clock divider */
#define NPCM_WTE	BIT(7)			/* Enable */
#define NPCM_WTIE	BIT(6)			/* Enable irq */
#define NPCM_WTIS	(BIT(4) | BIT(5))	/* Interval selection */
#define NPCM_WTIF	BIT(3)			/* Interrupt flag*/
#define NPCM_WTRF	BIT(2)			/* Reset flag */
#define NPCM_WTRE	BIT(1)			/* Reset enable */
#define NPCM_WTR	BIT(0)			/* Reset counter */

struct npcm_wdt_priv {
	void __iomem *regs;
};

static int npcm_wdt_start(struct udevice *dev, u64 timeout_ms, ulong flags)
{
	struct npcm_wdt_priv *priv = dev_get_priv(dev);
	u32 time_out, val;

	time_out = (u32)(timeout_ms) / 1000;
	if (time_out < 2)
		val = 0x800;
	else if (time_out < 3)
		val = 0x420;
	else if (time_out < 6)
		val = 0x810;
	else if (time_out < 11)
		val = 0x430;
	else if (time_out < 22)
		val = 0x820;
	else if (time_out < 44)
		val = 0xc00;
	else if (time_out < 87)
		val = 0x830;
	else if (time_out < 173)
		val = 0xc10;
	else if (time_out < 688)
		val = 0xc20;
	else
		val = 0xc30;

	val |= NPCM_WTRE | NPCM_WTE | NPCM_WTR | NPCM_WTIE;
	writel(val, priv->regs);

	return 0;
}

static int npcm_wdt_stop(struct udevice *dev)
{
	struct npcm_wdt_priv *priv = dev_get_priv(dev);

	writel(0, priv->regs);

	return 0;
}

static int npcm_wdt_reset(struct udevice *dev)
{
	struct npcm_wdt_priv *priv = dev_get_priv(dev);
	u32 val;

	val = readl(priv->regs);
	writel(val | NPCM_WTR, priv->regs);

	return 0;
}

static int npcm_wdt_expire_now(struct udevice *dev, ulong flags)
{
	struct npcm_wdt_priv *priv = dev_get_priv(dev);

	writel(NPCM_WTR | NPCM_WTRE | NPCM_WTE, priv->regs);

	return 0;
}

static int npcm_wdt_of_to_plat(struct udevice *dev)
{
	struct npcm_wdt_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

static const struct wdt_ops npcm_wdt_ops = {
	.expire_now = npcm_wdt_expire_now,
	.start = npcm_wdt_start,
	.reset = npcm_wdt_reset,
	.stop = npcm_wdt_stop,
};

static const struct udevice_id npcm_wdt_ids[] = {
	{ .compatible = "nuvoton,npcm750-wdt" },
	{ }
};

static int npcm_wdt_probe(struct udevice *dev)
{
	debug("%s() wdt%u\n", __func__, dev_seq(dev));
	npcm_wdt_stop(dev);

	return 0;
}

U_BOOT_DRIVER(npcm_wdt) = {
	.name = "npcm_wdt",
	.id = UCLASS_WDT,
	.of_match = npcm_wdt_ids,
	.probe = npcm_wdt_probe,
	.priv_auto = sizeof(struct npcm_wdt_priv),
	.of_to_plat = npcm_wdt_of_to_plat,
	.ops = &npcm_wdt_ops,
};
