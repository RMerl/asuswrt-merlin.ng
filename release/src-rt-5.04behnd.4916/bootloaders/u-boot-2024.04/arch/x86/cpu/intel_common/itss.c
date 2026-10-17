// SPDX-License-Identifier: GPL-2.0
/*
 * Interrupt Timer Subsystem
 *
 * Copyright (C) 2017 Intel Corporation.
 * Copyright (C) 2017 Siemens AG
 * Copyright 2019 Google LLC
 *
 * Taken from coreboot itss.c
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <irq.h>
#include <log.h>
#include <malloc.h>
#include <p2sb.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/itss.h>

static int set_polarity(struct udevice *dev, uint irq, bool active_low)
{
	u32 mask;
	uint reg;

	if (irq > ITSS_MAX_IRQ)
		return -EINVAL;

	reg = PCR_ITSS_IPC0_CONF + sizeof(u32) * (irq / IRQS_PER_IPC);
	mask = 1 << (irq % IRQS_PER_IPC);

	pcr_clrsetbits32(dev, reg, mask, active_low ? mask : 0);

	return 0;
}

#ifndef CONFIG_TPL_BUILD
static int snapshot_polarities(struct udevice *dev)
{
	struct itss_priv *priv = dev_get_priv(dev);
	const int start = GPIO_IRQ_START;
	const int end = GPIO_IRQ_END;
	int reg_start;
	int reg_end;
	int i;

	reg_start = start / IRQS_PER_IPC;
	reg_end = DIV_ROUND_UP(end, IRQS_PER_IPC);

	log_debug("ITSS IRQ Polarities snapshot %p\n", priv->irq_snapshot);
	for (i = reg_start; i < reg_end; i++) {
		uint reg = PCR_ITSS_IPC0_CONF + sizeof(u32) * i;

		priv->irq_snapshot[i] = pcr_read32(dev, reg);
		log_debug("   - %d, reg %x: irq_snapshot[i] %x\n", i, reg,
			  priv->irq_snapshot[i]);
	}

	/* Save the snapshot for use after relocation */
	gd->start_addr_sp -= sizeof(*priv);
	gd->start_addr_sp &= ~0xf;
	gd->arch.itss_priv = (void *)gd->start_addr_sp;
	memcpy(gd->arch.itss_priv, priv, sizeof(*priv));

	return 0;
}

static void show_polarities(struct udevice *dev, const char *msg)
{
	int i;

	log_debug("ITSS IRQ Polarities %s:\n", msg);
	for (i = 0; i < NUM_IPC_REGS; i++) {
		uint reg = PCR_ITSS_IPC0_CONF + sizeof(u32) * i;

		log_debug("IPC%d: 0x%08x\n", i, pcr_read32(dev, reg));
	}
}

static int restore_polarities(struct udevice *dev)
{
	struct itss_priv *priv = dev_get_priv(dev);
	struct itss_priv *old_priv;
	const int start = GPIO_IRQ_START;
	const int end = GPIO_IRQ_END;
	int reg_start;
	int reg_end;
	int i;

	/* Get the snapshot which was stored by the pre-reloc device */
	old_priv = gd->arch.itss_priv;
	if (!old_priv)
		return log_msg_ret("priv", -EFAULT);
	memcpy(priv->irq_snapshot, old_priv->irq_snapshot,
	       sizeof(priv->irq_snapshot));

	show_polarities(dev, "Before");
	log_debug("priv->irq_snapshot %p\n", priv->irq_snapshot);

	reg_start = start / IRQS_PER_IPC;
	reg_end = DIV_ROUND_UP(end, IRQS_PER_IPC);


	for (i = reg_start; i < reg_end; i++) {
		u32 mask;
		u16 reg;
		int irq_start;
		int irq_end;

		irq_start = i * IRQS_PER_IPC;
		irq_end = min(irq_start + IRQS_PER_IPC - 1, ITSS_MAX_IRQ);

		if (start > irq_end)
			continue;
		if (end < irq_start)
			break;

		/* Track bits within the bounds of of the register */
		irq_start = max(start, irq_start) % IRQS_PER_IPC;
		irq_end = min(end, irq_end) % IRQS_PER_IPC;

		/* Create bitmask of the inclusive range of start and end */
		mask = (((1U << irq_end) - 1) | (1U << irq_end));
		mask &= ~((1U << irq_start) - 1);

		reg = PCR_ITSS_IPC0_CONF + sizeof(u32) * i;
		log_debug("   - %d, reg %x: mask %x, irq_snapshot[i] %x\n",
			  i, reg, mask, priv->irq_snapshot[i]);
		pcr_clrsetbits32(dev, reg, mask, mask & priv->irq_snapshot[i]);
	}

	show_polarities(dev, "After");

	return 0;
}
#endif

static int route_pmc_gpio_gpe(struct udevice *dev, uint pmc_gpe_num)
{
	struct itss_priv *priv = dev_get_priv(dev);
	struct pmc_route *route;
	int i;

	for (i = 0, route = priv->route; i < priv->route_count; i++, route++) {
		if (pmc_gpe_num == route->pmc)
			return route->gpio;
	}

	return -ENOENT;
}

static int itss_bind(struct udevice *dev)
{
	/* This is not set with basic of-platdata, so set it manually */
	if (CONFIG_IS_ENABLED(OF_PLATDATA) &&
	    !CONFIG_IS_ENABLED(OF_PLATDATA_INST))
		dev->driver_data = X86_IRQT_ITSS;

	return 0;
}

static int itss_of_to_plat(struct udevice *dev)
{
	struct itss_priv *priv = dev_get_priv(dev);
	int ret;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct itss_plat *plat = dev_get_plat(dev);
	struct dtd_intel_itss *dtplat = &plat->dtplat;

	/*
	 * It would be nice to do this in the bind() method, but with
	 * of-platdata binding happens in the order that DM finds things in the
	 * linker list (i.e. alphabetical order by driver name). So the GPIO
	 * device may well be bound before its parent (p2sb), and this call
	 * will fail if p2sb is not bound yet.
	 *
	 * TODO(sjg@chromium.org): Add a parent pointer to child devices in dtoc
	 */
	ret = p2sb_set_port_id(dev, dtplat->intel_p2sb_port_id);
	if (ret)
		return log_msg_ret("Could not set port id", ret);
	priv->route = (struct pmc_route *)dtplat->intel_pmc_routes;
	priv->route_count = ARRAY_SIZE(dtplat->intel_pmc_routes) /
		 sizeof(struct pmc_route);
#else
	int size;

	size = dev_read_size(dev, "intel,pmc-routes");
	if (size < 0)
		return size;
	priv->route = malloc(size);
	if (!priv->route)
		return -ENOMEM;
	ret = dev_read_u32_array(dev, "intel,pmc-routes", (u32 *)priv->route,
				 size / sizeof(fdt32_t));
	if (ret)
		return log_msg_ret("Cannot read pmc-routes", ret);
	priv->route_count = size / sizeof(struct pmc_route);
#endif

	return 0;
}

static const struct irq_ops itss_ops = {
	.route_pmc_gpio_gpe	= route_pmc_gpio_gpe,
	.set_polarity	= set_polarity,
#ifndef CONFIG_TPL_BUILD
	.snapshot_polarities = snapshot_polarities,
	.restore_polarities = restore_polarities,
#endif
};

#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id itss_ids[] = {
	{ .compatible = "intel,itss", .data = X86_IRQT_ITSS },
	{ }
};
#endif

U_BOOT_DRIVER(intel_itss) = {
	.name		= "intel_itss",
	.id		= UCLASS_IRQ,
	.of_match	= of_match_ptr(itss_ids),
	.ops		= &itss_ops,
	.bind		= itss_bind,
	.of_to_plat = itss_of_to_plat,
	.plat_auto	= sizeof(struct itss_plat),
	.priv_auto	= sizeof(struct itss_priv),
};
