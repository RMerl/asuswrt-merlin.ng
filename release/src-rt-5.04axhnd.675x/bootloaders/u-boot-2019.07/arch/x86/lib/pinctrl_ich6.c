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
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPIO_USESEL_OFFSET(x)	(x)
#define GPIO_IOSEL_OFFSET(x)	(x + 4)
#define GPIO_LVL_OFFSET(x)	((x) ? (x) + 8 : 0xc)
#define GPI_INV			0x2c

#define IOPAD_MODE_MASK			0x7
#define IOPAD_PULL_ASSIGN_SHIFT		7
#define IOPAD_PULL_ASSIGN_MASK		(0x3 << IOPAD_PULL_ASSIGN_SHIFT)
#define IOPAD_PULL_STRENGTH_SHIFT	9
#define IOPAD_PULL_STRENGTH_MASK	(0x3 << IOPAD_PULL_STRENGTH_SHIFT)

static int ich6_pinctrl_set_value(uint16_t base, unsigned offset, int value)
{
	if (value)
		setio_32(base, 1UL << offset);
	else
		clrio_32(base, 1UL << offset);

	return 0;
}

static int ich6_pinctrl_set_function(uint16_t base, unsigned offset, int func)
{
	if (func)
		setio_32(base, 1UL << offset);
	else
		clrio_32(base, 1UL << offset);

	return 0;
}

static int ich6_pinctrl_set_direction(uint16_t base, unsigned offset, int dir)
{
	if (!dir)
		setio_32(base, 1UL << offset);
	else
		clrio_32(base, 1UL << offset);

	return 0;
}

static int ich6_pinctrl_cfg_pin(s32 gpiobase, s32 iobase, int pin_node)
{
	bool is_gpio, invert;
	u32 gpio_offset[2];
	int pad_offset;
	int dir, val;
	int ret;

	/*
	 * GPIO node is not mandatory, so we only do the pinmuxing if the
	 * node exists.
	 */
	ret = fdtdec_get_int_array(gd->fdt_blob, pin_node, "gpio-offset",
				   gpio_offset, 2);
	if (!ret) {
		/* Do we want to force the GPIO mode? */
		is_gpio = fdtdec_get_bool(gd->fdt_blob, pin_node, "mode-gpio");
		if (is_gpio)
			ich6_pinctrl_set_function(GPIO_USESEL_OFFSET(gpiobase) +
						gpio_offset[0], gpio_offset[1],
						1);

		dir = fdtdec_get_int(gd->fdt_blob, pin_node, "direction", -1);
		if (dir != -1)
			ich6_pinctrl_set_direction(GPIO_IOSEL_OFFSET(gpiobase) +
						 gpio_offset[0], gpio_offset[1],
						 dir);

		val = fdtdec_get_int(gd->fdt_blob, pin_node, "output-value",
				     -1);
		if (val != -1)
			ich6_pinctrl_set_value(GPIO_LVL_OFFSET(gpiobase) +
					     gpio_offset[0], gpio_offset[1],
					     val);

		invert = fdtdec_get_bool(gd->fdt_blob, pin_node, "invert");
		if (invert)
			setio_32(gpiobase + GPI_INV, 1 << gpio_offset[1]);
		debug("gpio %#x bit %d, is_gpio %d, dir %d, val %d, invert %d\n",
		      gpio_offset[0], gpio_offset[1], is_gpio, dir, val,
		      invert);
	}

	/* if iobase is present, let's configure the pad */
	if (iobase != -1) {
		ulong iobase_addr;

		/*
		 * The offset for the same pin for the IOBASE and GPIOBASE are
		 * different, so instead of maintaining a lookup table,
		 * the device tree should provide directly the correct
		 * value for both mapping.
		 */
		pad_offset = fdtdec_get_int(gd->fdt_blob, pin_node,
					    "pad-offset", -1);
		if (pad_offset == -1)
			return 0;

		/* compute the absolute pad address */
		iobase_addr = iobase + pad_offset;

		/*
		 * Do we need to set a specific function mode?
		 * If someone put also 'mode-gpio', this option will
		 * be just ignored by the controller
		 */
		val = fdtdec_get_int(gd->fdt_blob, pin_node, "mode-func", -1);
		if (val != -1)
			clrsetbits_le32(iobase_addr, IOPAD_MODE_MASK, val);

		/* Configure the pull-up/down if needed */
		val = fdtdec_get_int(gd->fdt_blob, pin_node, "pull-assign", -1);
		if (val != -1)
			clrsetbits_le32(iobase_addr,
					IOPAD_PULL_ASSIGN_MASK,
					val << IOPAD_PULL_ASSIGN_SHIFT);

		val = fdtdec_get_int(gd->fdt_blob, pin_node, "pull-strength",
				     -1);
		if (val != -1)
			clrsetbits_le32(iobase_addr,
					IOPAD_PULL_STRENGTH_MASK,
					val << IOPAD_PULL_STRENGTH_SHIFT);

		debug("%s: pad cfg [0x%x]: %08x\n", __func__, pad_offset,
		      readl(iobase_addr));
	}

	return 0;
}

static int ich6_pinctrl_probe(struct udevice *dev)
{
	struct udevice *pch;
	int pin_node;
	int ret;
	u32 gpiobase;
	u32 iobase = -1;

	debug("%s: start\n", __func__);
	ret = uclass_first_device(UCLASS_PCH, &pch);
	if (ret)
		return ret;
	if (!pch)
		return -ENODEV;

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

	/*
	 * Get the IOBASE, this is not mandatory as this is not
	 * supported by all the CPU
	 */
	ret = pch_get_io_base(pch, &iobase);
	if (ret && ret != -ENOSYS) {
		debug("%s: invalid IOBASE address (%08x)\n", __func__, iobase);
		return -EINVAL;
	}

	for (pin_node = fdt_first_subnode(gd->fdt_blob, dev_of_offset(dev));
	     pin_node > 0;
	     pin_node = fdt_next_subnode(gd->fdt_blob, pin_node)) {
		/* Configure the pin */
		ret = ich6_pinctrl_cfg_pin(gpiobase, iobase, pin_node);
		if (ret != 0) {
			debug("%s: invalid configuration for the pin %d\n",
			      __func__, pin_node);
			return ret;
		}
	}
	debug("%s: done\n", __func__);

	return 0;
}

static const struct udevice_id ich6_pinctrl_match[] = {
	{ .compatible = "intel,x86-pinctrl", .data = X86_SYSCON_PINCONF },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(ich6_pinctrl) = {
	.name = "ich6_pinctrl",
	.id = UCLASS_SYSCON,
	.of_match = ich6_pinctrl_match,
	.probe = ich6_pinctrl_probe,
};
