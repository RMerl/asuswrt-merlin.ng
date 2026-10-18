/*
 * Nexus interrupt(s) resolution API
 *
 * Copyright (C) 2015-2016, Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/irqdomain.h>
#include <linux/interrupt.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>

#include <linux/brcmstb/irq_api.h>

#define BUILD_IRQ_NAME(_name)			\
	case brcmstb_l2_irq_##_name:		\
		return __stringify(_name);	\

static const char *brcmstb_l2_irq_to_name(brcmstb_l2_irq irq)
{
	switch (irq) {
	BUILD_IRQ_NAME(gio);
	BUILD_IRQ_NAME(gio_aon);
	BUILD_IRQ_NAME(iica);
	BUILD_IRQ_NAME(iicb);
	BUILD_IRQ_NAME(iicc);
	BUILD_IRQ_NAME(iicd);
	BUILD_IRQ_NAME(iice);
	BUILD_IRQ_NAME(iicf);
	BUILD_IRQ_NAME(iicg);
	BUILD_IRQ_NAME(irb);
	BUILD_IRQ_NAME(icap);
	BUILD_IRQ_NAME(kbd1);
	BUILD_IRQ_NAME(kbd2);
	BUILD_IRQ_NAME(kbd3);
	BUILD_IRQ_NAME(ldk);
	BUILD_IRQ_NAME(spi);
	BUILD_IRQ_NAME(ua);
	BUILD_IRQ_NAME(ub);
	BUILD_IRQ_NAME(uc);
	BUILD_IRQ_NAME(bicap_fifo_inact_intr);
	BUILD_IRQ_NAME(bicap_fifo_lvl_intr);
	BUILD_IRQ_NAME(bicap_fifo_of_intr);
	BUILD_IRQ_NAME(bicap_timeout0_intr);
	BUILD_IRQ_NAME(bicap_timeout1_intr);
	BUILD_IRQ_NAME(bicap_timeout2_intr);
	BUILD_IRQ_NAME(bicap_timeout3_intr);
	BUILD_IRQ_NAME(wktmr_alarm_intr);
	default:
		return NULL;
	}

	return NULL;
}

static int brcmstb_resolve_l2_irq(struct device_node *np, brcmstb_l2_irq irq)
{
	int i, num_elems, ret;
	const char *int_name, *irq_name;
	u32 hwirq;

	irq_name = brcmstb_l2_irq_to_name(irq);
	if (!irq_name)
		return -EINVAL;

	/* Special case for AON interrupt names, search the other node for the
	 * same name
	 */
	if (strstr(irq_name, "aon") && !strstr(np->name, "aon"))
		return -EAGAIN;

	pr_debug("%s: resolving on node %s\n", __func__, np->full_name);

	num_elems = of_property_count_strings(np, "interrupt-names");
	if (num_elems <= 0) {
		pr_err("Unable to find an interrupt-names property, check DT\n");
		return -EINVAL;
	}

	for (i = 0; i < num_elems; i++) {
		ret = of_property_read_u32_index(np, "interrupts", i,
						 &hwirq);
		if (ret < 0)
			return ret;

		ret = of_property_read_string_index(np,
						    "interrupt-names",
						    i, &int_name);
		if (ret < 0)
			return ret;

		/* We may be requesting to match, eg: "gio" with "gio_aon" */
		if (!strncasecmp(int_name, irq_name, strlen(int_name)))
			break;
	}

	if (i == num_elems) {
		pr_debug("%s: exceeded search for %s\n", __func__, irq_name);
		return -ENOENT;
	}

	pr_debug("%s IRQ name: %s Node: %s @%d mapped to: %d\n", __func__,
		 irq_name, np->full_name, i, hwirq);

	return of_irq_get(np, i);
}

static const char *nexus_irq0_node_names[] = { "nexus-irq0", "nexus-irq0_aon",
					       "nexus-upg_main_irq",
					       "nexus-upg_main_aon_irq",
					       "nexus-upg_bsc_irq",
					       "nexus-upg_bsc_aon_irq",
					       "nexus-upg_spi_aon_irq",
					       "nexus-upg_aux_aon_intr2" };

int brcmstb_get_l2_irq_id(brcmstb_l2_irq irq)
{
	struct device_node *np;
	int ret = -ENOENT;
	int i = 0;

	for (i = 0; i < ARRAY_SIZE(nexus_irq0_node_names); i++) {
		np = of_find_node_by_name(NULL, nexus_irq0_node_names[i]);
		if (!np)
			continue;

		ret = brcmstb_resolve_l2_irq(np, irq);
		if (ret < 0)
			continue;

		if (ret == 0)
			return -EBUSY;

		if (ret > 0)
			break;
	}

	return ret;
}
EXPORT_SYMBOL(brcmstb_get_l2_irq_id);

int brcmstb_get_l1_irq_id(int phys_irq)
{
	struct irq_fwspec spec;
	struct device_node *np;
	struct irq_domain *gic;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "arm,cortex-a15-gic");
	if (!np)
		np = of_find_compatible_node(NULL, NULL, "arm,gic-400");
	if (!np)
		np = of_find_compatible_node(NULL, NULL, "arm,gic-v3");

	gic = irq_find_host(np);
	if (!gic)
		return -1;

	spec.fwnode = &np->fwnode;
	spec.param_count = 3;
	spec.param[0] = GIC_SPI;
	spec.param[1] = phys_irq;
	spec.param[2] = IRQ_TYPE_LEVEL_HIGH;

	ret = irq_create_fwspec_mapping(&spec);
	if (ret >= 0)
		pr_debug("%s IRQ on node: %s @ %d mapped to: %d\n",
			 __func__, np->full_name, ret, phys_irq);
	return ret;
}
EXPORT_SYMBOL(brcmstb_get_l1_irq_id);

struct ipi_handler {
	void (*handler)(void);
	int virq;
};

static struct ipi_handler ipi_handlers[16];

static irqreturn_t set_ipi_handler_compat(int irq, void *dev_id)
{
	struct ipi_handler *ipi = dev_id;

	(*ipi->handler)();

	return IRQ_HANDLED;
}

/*
 * set_ipi_handler:
 * Interface provided for a kernel module to specify an IPI handler function.
 */
int set_ipi_handler(int ipinr, void *handler, char *desc)
{
	struct ipi_handler *ipi = &ipi_handlers[ipinr];
	unsigned int cpu = smp_processor_id();
	struct device_node *np = NULL;
	struct irq_domain *gic;
	unsigned int irq;

	pr_warn("%s() is deprecated, do not use! (called by %pS)\n",
		__func__, (void *)_RET_IP_);

	if (ipi->handler) {
		pr_crit("CPU%u: IPI handler 0x%x already registered to %pf\n",
			cpu, ipinr, ipi->handler);
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "arm,cortex-a15-gic");
	if (!np)
		np = of_find_compatible_node(NULL, NULL, "arm,gic-400");

	gic = irq_find_host(np);
	if (!gic)
		return -1;

	irq = irq_create_mapping(gic, ipinr);

	ipi->handler = handler;
	ipi->virq = irq;

	return request_irq(irq, set_ipi_handler_compat, IRQF_NO_SUSPEND,
			   desc, ipi);
}
EXPORT_SYMBOL(set_ipi_handler);

/*
 * clear_ipi_handler:
 * Interface provided for a kernel module to clear an IPI handler function.
 */
void clear_ipi_handler(int ipinr)
{
	struct ipi_handler *ipi = &ipi_handlers[ipinr];

	free_irq(ipi->virq, ipi);
	irq_dispose_mapping(ipi->virq);
	pr_warn("%s() is deprecated, do not use! (called by %pS)\n",
		__func__, (void *)_RET_IP_);
	ipi->handler = NULL;
}
EXPORT_SYMBOL(clear_ipi_handler);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom STB Nexus IRQ APIs");
