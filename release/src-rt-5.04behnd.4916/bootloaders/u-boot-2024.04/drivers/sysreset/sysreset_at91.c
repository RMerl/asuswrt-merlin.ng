// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Microchip Technology, Inc. and its subsidiaries
 */

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <asm/arch/at91_rstc.h>
#include <clk.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <sysreset.h>

static int at91_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	at91_rstc_t *rstc = (at91_rstc_t *)dev_get_priv(dev);

	writel(AT91_RSTC_KEY
		| AT91_RSTC_CR_PROCRST  /* Processor Reset */
		| AT91_RSTC_CR_PERRST   /* Peripheral Reset */
#ifdef CONFIG_AT91RESET_EXTRST
		| AT91_RSTC_CR_EXTRST   /* External Reset (assert nRST pin) */
#endif
		, &rstc->cr);

	return -EINPROGRESS;
}

static int at91_sysreset_probe(struct udevice *dev)
{
	struct clk slck;
	void *priv;
	int ret;

	priv = dev_remap_addr(dev);
	if (!priv)
		return -EINVAL;

	dev_set_priv(dev, priv);

	ret = clk_get_by_index(dev, 0, &slck);
	if (ret)
		return ret;

	ret = clk_prepare_enable(&slck);
	if (ret)
		return ret;

	return 0;
}

static struct sysreset_ops at91_sysreset = {
	.request = at91_sysreset_request,
};

U_BOOT_DRIVER(sysreset_at91) = {
	.id	= UCLASS_SYSRESET,
	.name	= "at91_sysreset",
	.ops	= &at91_sysreset,
	.probe  = at91_sysreset_probe,
};
