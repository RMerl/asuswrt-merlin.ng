// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google, LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_IRQ

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <irq.h>
#include <log.h>
#include <dm/device-internal.h>

int irq_route_pmc_gpio_gpe(struct udevice *dev, uint pmc_gpe_num)
{
	const struct irq_ops *ops = irq_get_ops(dev);

	if (!ops->route_pmc_gpio_gpe)
		return -ENOSYS;

	return ops->route_pmc_gpio_gpe(dev, pmc_gpe_num);
}

int irq_set_polarity(struct udevice *dev, uint irq, bool active_low)
{
	const struct irq_ops *ops = irq_get_ops(dev);

	if (!ops->set_polarity)
		return -ENOSYS;

	return ops->set_polarity(dev, irq, active_low);
}

int irq_snapshot_polarities(struct udevice *dev)
{
	const struct irq_ops *ops = irq_get_ops(dev);

	if (!ops->snapshot_polarities)
		return -ENOSYS;

	return ops->snapshot_polarities(dev);
}

int irq_restore_polarities(struct udevice *dev)
{
	const struct irq_ops *ops = irq_get_ops(dev);

	if (!ops->restore_polarities)
		return -ENOSYS;

	return ops->restore_polarities(dev);
}

int irq_read_and_clear(struct irq *irq)
{
	const struct irq_ops *ops = irq_get_ops(irq->dev);

	if (!ops->read_and_clear)
		return -ENOSYS;

	return ops->read_and_clear(irq);
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
int irq_get_by_phandle(struct udevice *dev, const struct phandle_2_arg *cells,
		       struct irq *irq)
{
	int ret;

	ret = device_get_by_ofplat_idx(cells->idx, &irq->dev);
	if (ret)
		return ret;
	irq->id = cells->arg[0];

	/*
	 * Note: we could call irq_of_xlate_default() here to do this properly.
	 * For now, this is good enough for existing cases.
	 */
	irq->flags = cells->arg[1];

	return 0;
}
#else
static int irq_of_xlate_default(struct irq *irq,
				struct ofnode_phandle_args *args)
{
	log_debug("(irq=%p)\n", irq);

	if (args->args_count > 1) {
		log_debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		irq->id = args->args[0];
	else
		irq->id = 0;

	return 0;
}

static int irq_get_by_index_tail(int ret, ofnode node,
				 struct ofnode_phandle_args *args,
				 const char *list_name, int index,
				 struct irq *irq)
{
	struct udevice *dev_irq;
	const struct irq_ops *ops;

	assert(irq);
	irq->dev = NULL;
	if (ret)
		goto err;

	ret = uclass_get_device_by_ofnode(UCLASS_IRQ, args->node, &dev_irq);
	if (ret) {
		log_debug("uclass_get_device_by_ofnode failed: err=%d\n", ret);
		return ret;
	}

	irq->dev = dev_irq;

	ops = irq_get_ops(dev_irq);

	if (ops->of_xlate)
		ret = ops->of_xlate(irq, args);
	else
		ret = irq_of_xlate_default(irq, args);
	if (ret) {
		log_debug("of_xlate() failed: %d\n", ret);
		return ret;
	}

	return irq_request(dev_irq, irq);
err:
	log_debug("Node '%s', property '%s', failed to request IRQ index %d: %d\n",
		  ofnode_get_name(node), list_name, index, ret);
	return ret;
}

int irq_get_by_index(struct udevice *dev, int index, struct irq *irq)
{
	struct ofnode_phandle_args args;
	int ret;

	ret = dev_read_phandle_with_args(dev, "interrupts-extended",
					 "#interrupt-cells", 0, index, &args);

	return irq_get_by_index_tail(ret, dev_ofnode(dev), &args,
				     "interrupts-extended", index > 0, irq);
}
#endif /* OF_PLATDATA */

int irq_request(struct udevice *dev, struct irq *irq)
{
	const struct irq_ops *ops;

	log_debug("(dev=%p, irq=%p)\n", dev, irq);
	ops = irq_get_ops(dev);

	irq->dev = dev;

	if (!ops->request)
		return 0;

	return ops->request(irq);
}

int irq_first_device_type(enum irq_dev_t type, struct udevice **devp)
{
	int ret;

	ret = uclass_first_device_drvdata(UCLASS_IRQ, type, devp);
	if (ret)
		return ret;

	return 0;
}

#if CONFIG_IS_ENABLED(ACPIGEN)
int irq_get_acpi(const struct irq *irq, struct acpi_irq *acpi_irq)
{
	struct irq_ops *ops;

	if (!irq_is_valid(irq))
		return -EINVAL;

	ops = irq_get_ops(irq->dev);
	if (!ops->get_acpi)
		return -ENOSYS;

	return ops->get_acpi(irq, acpi_irq);
}
#endif

UCLASS_DRIVER(irq) = {
	.id		= UCLASS_IRQ,
	.name		= "irq",
};
