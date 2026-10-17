// SPDX-License-Identifier: GPL-2.0
/*
 * Sandbox driver for interrupts
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <acpi/acpi_device.h>
#include <asm/irq.h>
#include <asm/test.h>

static int sandbox_set_polarity(struct udevice *dev, uint irq, bool active_low)
{
	if (irq > 10)
		return -EINVAL;

	return 0;
}

static int sandbox_route_pmc_gpio_gpe(struct udevice *dev, uint pmc_gpe_num)
{
	if (pmc_gpe_num > 10)
		return -ENOENT;

	return pmc_gpe_num + 1;
}

static int sandbox_snapshot_polarities(struct udevice *dev)
{
	return 0;
}

static int sandbox_restore_polarities(struct udevice *dev)
{
	return 0;
}

static int sandbox_irq_read_and_clear(struct irq *irq)
{
	struct sandbox_irq_priv *priv = dev_get_priv(irq->dev);

	if (irq->id != SANDBOX_IRQN_PEND)
		return -EINVAL;
	priv->count++;
	if (priv->pending) {
		priv->pending = false;
		return 1;
	}

	if (!(priv->count % 3))
		priv->pending = true;

	return 0;
}

static int sandbox_irq_of_xlate(struct irq *irq,
				struct ofnode_phandle_args *args)
{
	irq->id = args->args[0];

	return 0;
}

static __maybe_unused int sandbox_get_acpi(const struct irq *irq,
					   struct acpi_irq *acpi_irq)
{
	acpi_irq->pin = irq->id;
	acpi_irq->mode = ACPI_IRQ_LEVEL_TRIGGERED;
	acpi_irq->polarity = ACPI_IRQ_ACTIVE_HIGH;
	acpi_irq->shared = ACPI_IRQ_SHARED;
	acpi_irq->wake = ACPI_IRQ_WAKE;

	return 0;
}

static const struct irq_ops sandbox_irq_ops = {
	.route_pmc_gpio_gpe	= sandbox_route_pmc_gpio_gpe,
	.set_polarity		= sandbox_set_polarity,
	.snapshot_polarities	= sandbox_snapshot_polarities,
	.restore_polarities	= sandbox_restore_polarities,
	.read_and_clear		= sandbox_irq_read_and_clear,
	.of_xlate		= sandbox_irq_of_xlate,
#if CONFIG_IS_ENABLED(ACPIGEN)
	.get_acpi		= sandbox_get_acpi,
#endif
};

static const struct udevice_id sandbox_irq_ids[] = {
	{ .compatible = "sandbox,irq", SANDBOX_IRQT_BASE },
	{ }
};

U_BOOT_DRIVER(sandbox_irq) = {
	.name		= "sandbox_irq",
	.id		= UCLASS_IRQ,
	.of_match	= sandbox_irq_ids,
	.ops		= &sandbox_irq_ops,
	.priv_auto	= sizeof(struct sandbox_irq_priv),
	DM_HEADER(<asm/irq.h>)
};
