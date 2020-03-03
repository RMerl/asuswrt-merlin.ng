/*
 * linux/kernel/irq/msi.c
 *
 * Copyright (C) 2014 Intel Corp.
 * Author: Jiang Liu <jiang.liu@linux.intel.com>
 *
 * This file is licensed under GPLv2.
 *
 * This file contains common code to support Message Signalled Interrupt for
 * PCI compatible and non PCI compatible devices.
 */
#include <linux/types.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/msi.h>

/* Temparory solution for building, will be removed later */
#include <linux/pci.h>

void __get_cached_msi_msg(struct msi_desc *entry, struct msi_msg *msg)
{
	*msg = entry->msg;
}

void get_cached_msi_msg(unsigned int irq, struct msi_msg *msg)
{
	struct msi_desc *entry = irq_get_msi_desc(irq);

	__get_cached_msi_msg(entry, msg);
}
EXPORT_SYMBOL_GPL(get_cached_msi_msg);

#ifdef CONFIG_GENERIC_MSI_IRQ_DOMAIN
static inline void irq_chip_write_msi_msg(struct irq_data *data,
					  struct msi_msg *msg)
{
	data->chip->irq_write_msi_msg(data, msg);
}

/**
 * msi_domain_set_affinity - Generic affinity setter function for MSI domains
 * @irq_data:	The irq data associated to the interrupt
 * @mask:	The affinity mask to set
 * @force:	Flag to enforce setting (disable online checks)
 *
 * Intended to be used by MSI interrupt controllers which are
 * implemented with hierarchical domains.
 */
int msi_domain_set_affinity(struct irq_data *irq_data,
			    const struct cpumask *mask, bool force)
{
	struct irq_data *parent = irq_data->parent_data;
	struct msi_msg msg;
	int ret;

	ret = parent->chip->irq_set_affinity(parent, mask, force);
	if (ret >= 0 && ret != IRQ_SET_MASK_OK_DONE) {
		BUG_ON(irq_chip_compose_msi_msg(irq_data, &msg));
		irq_chip_write_msi_msg(irq_data, &msg);
	}

	return ret;
}

static void msi_domain_activate(struct irq_domain *domain,
				struct irq_data *irq_data)
{
	struct msi_msg msg;

	BUG_ON(irq_chip_compose_msi_msg(irq_data, &msg));
	irq_chip_write_msi_msg(irq_data, &msg);
}

static void msi_domain_deactivate(struct irq_domain *domain,
				  struct irq_data *irq_data)
{
	struct msi_msg msg;

	memset(&msg, 0, sizeof(msg));
	irq_chip_write_msi_msg(irq_data, &msg);
}

static int msi_domain_alloc(struct irq_domain *domain, unsigned int virq,
			    unsigned int nr_irqs, void *arg)
{
	struct msi_domain_info *info = domain->host_data;
	struct msi_domain_ops *ops = info->ops;
	irq_hw_number_t hwirq = ops->get_hwirq(info, arg);
	int i, ret;

	if (irq_find_mapping(domain, hwirq) > 0)
		return -EEXIST;

	ret = irq_domain_alloc_irqs_parent(domain, virq, nr_irqs, arg);
	if (ret < 0)
		return ret;

	for (i = 0; i < nr_irqs; i++) {
		ret = ops->msi_init(domain, info, virq + i, hwirq + i, arg);
		if (ret < 0) {
			if (ops->msi_free) {
				for (i--; i > 0; i--)
					ops->msi_free(domain, info, virq + i);
			}
			irq_domain_free_irqs_top(domain, virq, nr_irqs);
			return ret;
		}
	}

	return 0;
}

static void msi_domain_free(struct irq_domain *domain, unsigned int virq,
			    unsigned int nr_irqs)
{
	struct msi_domain_info *info = domain->host_data;
	int i;

	if (info->ops->msi_free) {
		for (i = 0; i < nr_irqs; i++)
			info->ops->msi_free(domain, info, virq + i);
	}
	irq_domain_free_irqs_top(domain, virq, nr_irqs);
}

static struct irq_domain_ops msi_domain_ops = {
	.alloc		= msi_domain_alloc,
	.free		= msi_domain_free,
	.activate	= msi_domain_activate,
	.deactivate	= msi_domain_deactivate,
};

#ifdef GENERIC_MSI_DOMAIN_OPS
static irq_hw_number_t msi_domain_ops_get_hwirq(struct msi_domain_info *info,
						msi_alloc_info_t *arg)
{
	return arg->hwirq;
}

static int msi_domain_ops_prepare(struct irq_domain *domain, struct device *dev,
				  int nvec, msi_alloc_info_t *arg)
{
	memset(arg, 0, sizeof(*arg));
	return 0;
}

static void msi_domain_ops_set_desc(msi_alloc_info_t *arg,
				    struct msi_desc *desc)
{
	arg->desc = desc;
}
#else
#define msi_domain_ops_get_hwirq	NULL
#define msi_domain_ops_prepare		NULL
#define msi_domain_ops_set_desc		NULL
#endif /* !GENERIC_MSI_DOMAIN_OPS */

static int msi_domain_ops_init(struct irq_domain *domain,
			       struct msi_domain_info *info,
			       unsigned int virq, irq_hw_number_t hwirq,
			       msi_alloc_info_t *arg)
{
	irq_domain_set_hwirq_and_chip(domain, virq, hwirq, info->chip,
				      info->chip_data);
	if (info->handler && info->handler_name) {
		__irq_set_handler(virq, info->handler, 0, info->handler_name);
		if (info->handler_data)
			irq_set_handler_data(virq, info->handler_data);
	}
	return 0;
}

static int msi_domain_ops_check(struct irq_domain *domain,
				struct msi_domain_info *info,
				struct device *dev)
{
	return 0;
}

static struct msi_domain_ops msi_domain_ops_default = {
	.get_hwirq	= msi_domain_ops_get_hwirq,
	.msi_init	= msi_domain_ops_init,
	.msi_check	= msi_domain_ops_check,
	.msi_prepare	= msi_domain_ops_prepare,
	.set_desc	= msi_domain_ops_set_desc,
};

static void msi_domain_update_dom_ops(struct msi_domain_info *info)
{
	struct msi_domain_ops *ops = info->ops;

	if (ops == NULL) {
		info->ops = &msi_domain_ops_default;
		return;
	}

	if (ops->get_hwirq == NULL)
		ops->get_hwirq = msi_domain_ops_default.get_hwirq;
	if (ops->msi_init == NULL)
		ops->msi_init = msi_domain_ops_default.msi_init;
	if (ops->msi_check == NULL)
		ops->msi_check = msi_domain_ops_default.msi_check;
	if (ops->msi_prepare == NULL)
		ops->msi_prepare = msi_domain_ops_default.msi_prepare;
	if (ops->set_desc == NULL)
		ops->set_desc = msi_domain_ops_default.set_desc;
}

static void msi_domain_update_chip_ops(struct msi_domain_info *info)
{
	struct irq_chip *chip = info->chip;

	BUG_ON(!chip);
	if (!chip->irq_mask)
		chip->irq_mask = pci_msi_mask_irq;
	if (!chip->irq_unmask)
		chip->irq_unmask = pci_msi_unmask_irq;
	if (!chip->irq_set_affinity)
		chip->irq_set_affinity = msi_domain_set_affinity;
}

/**
 * msi_create_irq_domain - Create a MSI interrupt domain
 * @of_node:	Optional device-tree node of the interrupt controller
 * @info:	MSI domain info
 * @parent:	Parent irq domain
 */
struct irq_domain *msi_create_irq_domain(struct device_node *node,
					 struct msi_domain_info *info,
					 struct irq_domain *parent)
{
	if (info->flags & MSI_FLAG_USE_DEF_DOM_OPS)
		msi_domain_update_dom_ops(info);
	if (info->flags & MSI_FLAG_USE_DEF_CHIP_OPS)
		msi_domain_update_chip_ops(info);

	return irq_domain_add_hierarchy(parent, 0, 0, node, &msi_domain_ops,
					info);
}

/**
 * msi_domain_alloc_irqs - Allocate interrupts from a MSI interrupt domain
 * @domain:	The domain to allocate from
 * @dev:	Pointer to device struct of the device for which the interrupts
 *		are allocated
 * @nvec:	The number of interrupts to allocate
 *
 * Returns 0 on success or an error code.
 */
int msi_domain_alloc_irqs(struct irq_domain *domain, struct device *dev,
			  int nvec)
{
	struct msi_domain_info *info = domain->host_data;
	struct msi_domain_ops *ops = info->ops;
	msi_alloc_info_t arg;
	struct msi_desc *desc;
	int i, ret, virq = -1;

	ret = ops->msi_check(domain, info, dev);
	if (ret == 0)
		ret = ops->msi_prepare(domain, dev, nvec, &arg);
	if (ret)
		return ret;

	for_each_msi_entry(desc, dev) {
		ops->set_desc(&arg, desc);
		if (info->flags & MSI_FLAG_IDENTITY_MAP)
			virq = (int)ops->get_hwirq(info, &arg);
		else
			virq = -1;

		virq = __irq_domain_alloc_irqs(domain, virq, desc->nvec_used,
					       dev_to_node(dev), &arg, false);
		if (virq < 0) {
			ret = -ENOSPC;
			if (ops->handle_error)
				ret = ops->handle_error(domain, desc, ret);
			if (ops->msi_finish)
				ops->msi_finish(&arg, ret);
			return ret;
		}

		for (i = 0; i < desc->nvec_used; i++)
			irq_set_msi_desc_off(virq, i, desc);
	}

	if (ops->msi_finish)
		ops->msi_finish(&arg, 0);

	for_each_msi_entry(desc, dev) {
		if (desc->nvec_used == 1)
			dev_dbg(dev, "irq %d for MSI\n", virq);
		else
			dev_dbg(dev, "irq [%d-%d] for MSI\n",
				virq, virq + desc->nvec_used - 1);
	}

	return 0;
}

/**
 * msi_domain_free_irqs - Free interrupts from a MSI interrupt @domain associated tp @dev
 * @domain:	The domain to managing the interrupts
 * @dev:	Pointer to device struct of the device for which the interrupts
 *		are free
 */
void msi_domain_free_irqs(struct irq_domain *domain, struct device *dev)
{
	struct msi_desc *desc;

	for_each_msi_entry(desc, dev) {
		/*
		 * We might have failed to allocate an MSI early
		 * enough that there is no IRQ associated to this
		 * entry. If that's the case, don't do anything.
		 */
		if (desc->irq) {
			irq_domain_free_irqs(desc->irq, desc->nvec_used);
			desc->irq = 0;
		}
	}
}

/**
 * msi_get_domain_info - Get the MSI interrupt domain info for @domain
 * @domain:	The interrupt domain to retrieve data from
 *
 * Returns the pointer to the msi_domain_info stored in
 * @domain->host_data.
 */
struct msi_domain_info *msi_get_domain_info(struct irq_domain *domain)
{
	return (struct msi_domain_info *)domain->host_data;
}

#endif /* CONFIG_GENERIC_MSI_IRQ_DOMAIN */
