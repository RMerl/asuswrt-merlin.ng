// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <pch.h>
#include <pci.h>
#include <asm/cpu.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/gpio.h>
#include <dt-bindings/gpio/x86-gpio.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	MAX_GPIOS	= 95,
};

#define PIRQ_SHIFT	16
#define CONF_MASK	0xffff

struct pin_info {
	int node;
	int phandle;
	bool mode_gpio;
	bool dir_input;
	bool invert;
	bool trigger_level;
	bool output_high;
	bool sense_disable;
	bool owner_gpio;
	bool route_smi;
	bool irq_enable;
	bool reset_rsmrst;
	bool pirq_apic_route;
};

static int broadwell_pinctrl_read_configs(struct udevice *dev,
					  struct pin_info *conf, int max_pins)
{
	const void *blob = gd->fdt_blob;
	int count = 0;
	int node;

	debug("%s: starting\n", __func__);
	for (node = fdt_first_subnode(blob, dev_of_offset(dev));
	     node > 0;
	     node = fdt_next_subnode(blob, node)) {
		int phandle = fdt_get_phandle(blob, node);

		if (!phandle)
			continue;
		if (count == max_pins)
			return -ENOSPC;

		/* We've found a new configuration */
		memset(conf, '\0', sizeof(*conf));
		conf->node = node;
		conf->phandle = phandle;
		conf->mode_gpio = fdtdec_get_bool(blob, node, "mode-gpio");
		if (fdtdec_get_int(blob, node, "direction", -1) == PIN_INPUT)
			conf->dir_input = true;
		conf->invert = fdtdec_get_bool(blob, node, "invert");
		if (fdtdec_get_int(blob, node, "trigger", -1) == TRIGGER_LEVEL)
			conf->trigger_level = true;
		if (fdtdec_get_int(blob, node, "output-value", -1) == 1)
			conf->output_high = true;
		conf->sense_disable = fdtdec_get_bool(blob, node,
						      "sense-disable");
		if (fdtdec_get_int(blob, node, "owner", -1) == OWNER_GPIO)
			conf->owner_gpio = true;
		if (fdtdec_get_int(blob, node, "route", -1) == ROUTE_SMI)
			conf->route_smi = true;
		conf->irq_enable = fdtdec_get_bool(blob, node, "irq-enable");
		conf->reset_rsmrst = fdtdec_get_bool(blob, node,
						     "reset-rsmrst");
		if (fdtdec_get_int(blob, node, "pirq-apic", -1) ==
				PIRQ_APIC_ROUTE)
			conf->pirq_apic_route = true;
		debug("config: phandle=%d\n", phandle);
		count++;
		conf++;
	}
	debug("%s: Found %d configurations\n", __func__, count);

	return count;
}

static int broadwell_pinctrl_lookup_phandle(struct pin_info *conf,
					    int conf_count, int phandle)
{
	int i;

	for (i = 0; i < conf_count; i++) {
		if (conf[i].phandle == phandle)
			return i;
	}

	return -ENOENT;
}

static int broadwell_pinctrl_read_pins(struct udevice *dev,
		struct pin_info *conf, int conf_count, int gpio_conf[],
		int num_gpios)
{
	const void *blob = gd->fdt_blob;
	int count = 0;
	int node;

	for (node = fdt_first_subnode(blob, dev_of_offset(dev));
	     node > 0;
	     node = fdt_next_subnode(blob, node)) {
		int len, i;
		const u32 *prop = fdt_getprop(blob, node, "config", &len);

		if (!prop)
			continue;

		/* There are three cells per pin */
		count = len / (sizeof(u32) * 3);
		debug("Found %d GPIOs to configure\n", count);
		for (i = 0; i < count; i++) {
			uint gpio = fdt32_to_cpu(prop[i * 3]);
			uint phandle = fdt32_to_cpu(prop[i * 3 + 1]);
			int val;

			if (gpio >= num_gpios) {
				debug("%s: GPIO %d out of range\n", __func__,
				      gpio);
				return -EDOM;
			}
			val = broadwell_pinctrl_lookup_phandle(conf, conf_count,
							       phandle);
			if (val < 0) {
				debug("%s: Cannot find phandle %d\n", __func__,
				      phandle);
				return -EINVAL;
			}
			gpio_conf[gpio] = val |
				fdt32_to_cpu(prop[i * 3 + 2]) << PIRQ_SHIFT;
		}
	}

	return 0;
}

static void broadwell_pinctrl_commit(struct pch_lp_gpio_regs *regs,
				     struct pin_info *pin_info,
				     int gpio_conf[], int count)
{
	u32 owner_gpio[GPIO_BANKS] = {0};
	u32 route_smi[GPIO_BANKS] = {0};
	u32 irq_enable[GPIO_BANKS] = {0};
	u32 reset_rsmrst[GPIO_BANKS] = {0};
	u32 pirq2apic = 0;
	int set, bit, gpio = 0;

	for (gpio = 0; gpio < MAX_GPIOS; gpio++) {
		int confnum = gpio_conf[gpio] & CONF_MASK;
		struct pin_info *pin = &pin_info[confnum];
		u32 val;

		val = pin->mode_gpio << CONFA_MODE_SHIFT |
			pin->dir_input << CONFA_DIR_SHIFT |
			pin->invert << CONFA_INVERT_SHIFT |
			pin->trigger_level << CONFA_TRIGGER_SHIFT |
			pin->output_high << CONFA_OUTPUT_SHIFT;
		outl(val, &regs->config[gpio].conf_a);
		outl(pin->sense_disable << CONFB_SENSE_SHIFT,
		     &regs->config[gpio].conf_b);

		/* Determine set and bit based on GPIO number */
		set = gpio / GPIO_PER_BANK;
		bit = gpio % GPIO_PER_BANK;

		/* Apply settings to set specific bits */
		owner_gpio[set] |= pin->owner_gpio << bit;
		route_smi[set] |= pin->route_smi << bit;
		irq_enable[set] |= pin->irq_enable << bit;
		reset_rsmrst[set] |= pin->reset_rsmrst << bit;

		/* PIRQ to IO-APIC map */
		if (pin->pirq_apic_route)
			pirq2apic |= gpio_conf[gpio] >> PIRQ_SHIFT;
		debug("gpio %d: conf %d, mode_gpio %d, dir_input %d, output_high %d\n",
		      gpio, confnum, pin->mode_gpio, pin->dir_input,
		      pin->output_high);
	}

	for (set = 0; set < GPIO_BANKS; set++) {
		outl(owner_gpio[set], &regs->own[set]);
		outl(route_smi[set], &regs->gpi_route[set]);
		outl(irq_enable[set], &regs->gpi_ie[set]);
		outl(reset_rsmrst[set], &regs->rst_sel[set]);
	}

	outl(pirq2apic, &regs->pirq_to_ioxapic);
}

static int broadwell_pinctrl_probe(struct udevice *dev)
{
	struct pch_lp_gpio_regs *regs;
	struct pin_info conf[12];
	int gpio_conf[MAX_GPIOS];
	struct udevice *pch;
	int conf_count;
	u32 gpiobase;
	int ret;

	ret = uclass_find_first_device(UCLASS_PCH, &pch);
	if (ret)
		return ret;
	if (!pch)
		return -ENODEV;
	debug("%s: start\n", __func__);

	/* Only init once, before relocation */
	if (gd->flags & GD_FLG_RELOC)
		return 0;

	/*
	 * Get the memory/io base address to configure every pins.
	 * IOBASE is used to configure the mode/pads
	 * GPIOBASE is used to configure the direction and default value
	 */
	ret = pch_get_gpio_base(pch, &gpiobase);
	if (ret) {
		debug("%s: invalid GPIOBASE address (%08x)\n", __func__,
		      gpiobase);
		return -EINVAL;
	}

	conf_count = broadwell_pinctrl_read_configs(dev, conf,
						    ARRAY_SIZE(conf));
	if (conf_count < 0) {
		debug("%s: Cannot read configs: err=%d\n", __func__, ret);
		return conf_count;
	}

	/*
	 * Assume that pin settings are provided for every pin. Pins not
	 * mentioned will get the first config mentioned in the list.
	 */
	ret = broadwell_pinctrl_read_pins(dev, conf, conf_count, gpio_conf,
					  MAX_GPIOS);
	if (ret) {
		debug("%s: Cannot read pin settings: err=%d\n", __func__, ret);
		return ret;
	}

	regs = (struct pch_lp_gpio_regs *)gpiobase;
	broadwell_pinctrl_commit(regs, conf, gpio_conf, ARRAY_SIZE(conf));

	debug("%s: done\n", __func__);

	return 0;
}

static const struct udevice_id broadwell_pinctrl_match[] = {
	{ .compatible = "intel,x86-broadwell-pinctrl",
		.data = X86_SYSCON_PINCONF },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(broadwell_pinctrl) = {
	.name = "broadwell_pinctrl",
	.id = UCLASS_SYSCON,
	.of_match = broadwell_pinctrl_match,
	.probe = broadwell_pinctrl_probe,
};
