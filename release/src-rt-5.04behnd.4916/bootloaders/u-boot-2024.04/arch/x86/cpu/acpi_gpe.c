// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google, LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	UCLASS_IRQ

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <log.h>
#include <acpi/acpi_device.h>
#include <asm/io.h>
#include <dt-bindings/interrupt-controller/irq.h>
#include <dt-bindings/interrupt-controller/x86-irq.h>

/**
 * struct acpi_gpe_priv - private driver information
 *
 * @acpi_base: Base I/O address of ACPI registers
 */
struct acpi_gpe_priv {
	ulong acpi_base;
};

#define GPE0_STS(x)		(0x20 + ((x) * 4))

static int acpi_gpe_read_and_clear(struct irq *irq)
{
	struct acpi_gpe_priv *priv = dev_get_priv(irq->dev);
	u32 mask, sts;
	ulong start;
	int ret = 0;
	int bank;

	bank = irq->id / 32;
	mask = 1 << (irq->id % 32);

	/* Wait up to 1ms for GPE status to clear */
	start = get_timer(0);
	do {
		if (get_timer(start) > 1)
			return ret;

		sts = inl(priv->acpi_base + GPE0_STS(bank));
		if (sts & mask) {
			outl(mask, priv->acpi_base + GPE0_STS(bank));
			ret = 1;
		}
	} while (sts & mask);

	return ret;
}

static int acpi_gpe_of_to_plat(struct udevice *dev)
{
	struct acpi_gpe_priv *priv = dev_get_priv(dev);

	priv->acpi_base = dev_read_addr(dev);
	if (!priv->acpi_base || priv->acpi_base == FDT_ADDR_T_NONE)
		return log_msg_ret("acpi_base", -EINVAL);

	return 0;
}

static int acpi_gpe_of_xlate(struct irq *irq, struct ofnode_phandle_args *args)
{
	irq->id = args->args[0];
	irq->flags = args->args[1];

	return 0;
}

#if CONFIG_IS_ENABLED(ACPIGEN)
static int acpi_gpe_get_acpi(const struct irq *irq, struct acpi_irq *acpi_irq)
{
	memset(acpi_irq, '\0', sizeof(*acpi_irq));
	acpi_irq->pin = irq->id;
	acpi_irq->mode = irq->flags & IRQ_TYPE_EDGE_BOTH ?
		ACPI_IRQ_EDGE_TRIGGERED : ACPI_IRQ_LEVEL_TRIGGERED;
	acpi_irq->polarity = irq->flags &
		 (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_LEVEL_LOW) ?
		 ACPI_IRQ_ACTIVE_LOW : ACPI_IRQ_ACTIVE_HIGH;
	acpi_irq->shared = irq->flags & X86_IRQ_TYPE_SHARED ?
		ACPI_IRQ_SHARED : ACPI_IRQ_EXCLUSIVE;
	acpi_irq->wake = irq->flags & X86_IRQ_TYPE_WAKE ? ACPI_IRQ_WAKE :
		ACPI_IRQ_NO_WAKE;

	return 0;
}
#endif

static const struct irq_ops acpi_gpe_ops = {
	.read_and_clear		= acpi_gpe_read_and_clear,
	.of_xlate		= acpi_gpe_of_xlate,
#if CONFIG_IS_ENABLED(ACPIGEN)
	.get_acpi		= acpi_gpe_get_acpi,
#endif
};

static const struct udevice_id acpi_gpe_ids[] = {
	{ .compatible = "intel,acpi-gpe", .data = X86_IRQT_ACPI_GPE },
	{ }
};

U_BOOT_DRIVER(intel_acpi_gpe) = {
	.name		= "intel_acpi_gpe",
	.id		= UCLASS_IRQ,
	.of_match	= acpi_gpe_ids,
	.ops		= &acpi_gpe_ops,
	.of_to_plat	= acpi_gpe_of_to_plat,
	.priv_auto	= sizeof(struct acpi_gpe_priv),
};
