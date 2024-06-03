// SPDX-License-Identifier: GPL-2.0+
/*
 * Pinctrl driver for STMicroelectronics STi SoCs
 *
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <bitfield.h>
#include <dm.h>
#include <errno.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/pinctrl.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAX_STI_PINCONF_ENTRIES		7
/* Output enable */
#define OE			(1 << 27)
/* Pull Up */
#define PU			(1 << 26)
/* Open Drain */
#define OD			(1 << 25)

/* User-frendly defines for Pin Direction */
		/* oe = 0, pu = 0, od = 0 */
#define IN			(0)
		/* oe = 0, pu = 1, od = 0 */
#define IN_PU			(PU)
		/* oe = 1, pu = 0, od = 0 */
#define OUT			(OE)
		/* oe = 1, pu = 1, od = 0 */
#define OUT_PU			(OE | PU)
		/* oe = 1, pu = 0, od = 1 */
#define BIDIR			(OE | OD)
		/* oe = 1, pu = 1, od = 1 */
#define BIDIR_PU		(OE | PU | OD)

struct sti_pinctrl_platdata {
	struct regmap *regmap;
};

struct sti_pin_desc {
	unsigned char bank;
	unsigned char pin;
	unsigned char alt;
	int dir;
};

/*
 * PIO alternative Function selector
 */
void sti_alternate_select(struct udevice *dev, struct sti_pin_desc *pin_desc)
{
	struct sti_pinctrl_platdata *plat = dev_get_platdata(dev);
	unsigned long sysconf, *sysconfreg;
	int alt = pin_desc->alt;
	int bank = pin_desc->bank;
	int pin = pin_desc->pin;

	sysconfreg = (unsigned long *)plat->regmap->ranges[0].start;

	switch (bank) {
	case 0 ... 5:		/* in "SBC Bank" */
		sysconfreg += bank;
		break;
	case 10 ... 20:		/* in "FRONT Bank" */
		sysconfreg += bank - 10;
		break;
	case 30 ... 35:		/* in "REAR Bank" */
		sysconfreg += bank - 30;
		break;
	case 40 ... 42:		/* in "FLASH Bank" */
		sysconfreg += bank - 40;
		break;
	default:
		BUG();
		return;
	}

	sysconf = readl(sysconfreg);
	sysconf = bitfield_replace(sysconf, pin * 4, 3, alt);
	writel(sysconf, sysconfreg);
}

/* pin configuration */
void sti_pin_configure(struct udevice *dev, struct sti_pin_desc *pin_desc)
{
	struct sti_pinctrl_platdata *plat = dev_get_platdata(dev);
	int bit;
	int oe = 0, pu = 0, od = 0;
	unsigned long *sysconfreg;
	int bank = pin_desc->bank;

	sysconfreg = (unsigned long *)plat->regmap->ranges[0].start + 40;

	/*
	 * NOTE: The PIO configuration for the PIO pins in the
	 * "FLASH Bank" are different from all the other banks!
	 * Specifically, the output-enable pin control register
	 * (SYS_CFG_3040) and the pull-up pin control register
	 * (SYS_CFG_3050), are both classed as being "reserved".
	 * Hence, we do not write to these registers to configure
	 * the OE and PU features for PIOs in this bank. However,
	 * the open-drain pin control register (SYS_CFG_3060)
	 * follows the style of the other banks, and so we can
	 * treat that register normally.
	 *
	 * Being pedantic, we should configure the PU and PD features
	 * in the "FLASH Bank" explicitly instead using the four
	 * SYS_CFG registers: 3080, 3081, 3085, and 3086. However, this
	 * would necessitate passing in the alternate function number
	 * to this function, and adding some horrible complexity here.
	 * Alternatively, we could just perform 4 32-bit "pokes" to
	 * these four SYS_CFG registers early in the initialization.
	 * In practice, these four SYS_CFG registers are correct
	 * after a reset, and U-Boot does not need to change them, so
	 * we (cheat and) rely on these registers being correct.
	 * WARNING: Please be aware of this (pragmatic) behaviour!
	 */
	int flashss = 0;	/* bool: PIO in the Flash Sub-System ? */

	switch (pin_desc->dir) {
	case IN:
		oe = 0; pu = 0; od = 0;
		break;
	case IN_PU:
		oe = 0; pu = 1; od = 0;
		break;
	case OUT:
		oe = 1; pu = 0; od = 0;
		break;
	case BIDIR:
		oe = 1; pu = 0; od = 1;
		break;
	case BIDIR_PU:
		oe = 1; pu = 1; od = 1;
		break;

	default:
		pr_err("%s invalid direction value: 0x%x\n",
		      __func__, pin_desc->dir);
		BUG();
		break;
	}

	switch (bank) {
	case 0 ... 5:		/* in "SBC Bank" */
		sysconfreg += bank / 4;
		break;
	case 10 ... 20:		/* in "FRONT Bank" */
		bank -= 10;
		sysconfreg += bank / 4;
		break;
	case 30 ... 35:		/* in "REAR Bank" */
		bank -= 30;
		sysconfreg += bank / 4;
		break;
	case 40 ... 42:		/* in "FLASH Bank" */
		bank -= 40;
		sysconfreg += bank / 4;
		flashss = 1;	/* pin is in the Flash Sub-System */
		break;
	default:
		BUG();
		return;
	}

	bit = ((bank * 8) + pin_desc->pin) % 32;

	/*
	 * set the "Output Enable" pin control
	 * but, do nothing if in the flashSS
	 */
	if (!flashss) {
		if (oe)
			generic_set_bit(bit, sysconfreg);
		else
			generic_clear_bit(bit, sysconfreg);
	}

	sysconfreg += 10;	/* skip to next set of syscfg registers */

	/*
	 * set the "Pull Up" pin control
	 * but, do nothing if in the FlashSS
	 */

	if (!flashss) {
		if (pu)
			generic_set_bit(bit, sysconfreg);
		else
			generic_clear_bit(bit, sysconfreg);
	}

	sysconfreg += 10;	/* skip to next set of syscfg registers */

	/* set the "Open Drain Enable" pin control */
	if (od)
		generic_set_bit(bit, sysconfreg);
	else
		generic_clear_bit(bit, sysconfreg);
}


static int sti_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct fdtdec_phandle_args args;
	const void *blob = gd->fdt_blob;
	const char *prop_name;
	int node = dev_of_offset(config);
	int property_offset, prop_len;
	int pinconf_node, ret, count;
	const char *bank_name;
	u32 cells[MAX_STI_PINCONF_ENTRIES];

	struct sti_pin_desc pin_desc;

	/* go to next node "st,pins" which contains the pins configuration */
	pinconf_node = fdt_subnode_offset(blob, node, "st,pins");

	/*
	 * parse each pins configuration which looks like :
	 *	pin_name = <bank_phandle pin_nb alt dir rt_type rt_delay rt_clk>
	 */

	fdt_for_each_property_offset(property_offset, blob, pinconf_node) {
		fdt_getprop_by_offset(blob, property_offset, &prop_name,
				      &prop_len);

		/* extract the bank of the pin description */
		ret = fdtdec_parse_phandle_with_args(blob, pinconf_node,
						     prop_name, "#gpio-cells",
						     0, 0, &args);
		if (ret < 0) {
			pr_err("Can't get the gpio bank phandle: %d\n", ret);
			return ret;
		}

		bank_name = fdt_getprop(blob, args.node, "st,bank-name",
					&count);
		if (count < 0) {
			pr_err("Can't find bank-name property %d\n", count);
			return -EINVAL;
		}

		pin_desc.bank = trailing_strtoln(bank_name, NULL);

		count = fdtdec_get_int_array_count(blob, pinconf_node,
						   prop_name, cells,
						   ARRAY_SIZE(cells));
		if (count < 0) {
			pr_err("Bad pin configuration array %d\n", count);
			return -EINVAL;
		}

		if (count > MAX_STI_PINCONF_ENTRIES) {
			pr_err("Unsupported pinconf array count %d\n", count);
			return -EINVAL;
		}

		pin_desc.pin = cells[1];
		pin_desc.alt = cells[2];
		pin_desc.dir = cells[3];

		sti_alternate_select(dev, &pin_desc);
		sti_pin_configure(dev, &pin_desc);
	};

	return 0;
}

static int sti_pinctrl_probe(struct udevice *dev)
{
	struct sti_pinctrl_platdata *plat = dev_get_platdata(dev);
	struct udevice *syscon;
	int err;

	/* get corresponding syscon phandle */
	err = uclass_get_device_by_phandle(UCLASS_SYSCON, dev,
					   "st,syscfg", &syscon);
	if (err) {
		pr_err("unable to find syscon device\n");
		return err;
	}

	plat->regmap = syscon_get_regmap(syscon);
	if (!plat->regmap) {
		pr_err("unable to find regmap\n");
		return -ENODEV;
	}

	return 0;
}

static const struct udevice_id sti_pinctrl_ids[] = {
	{ .compatible = "st,stih407-sbc-pinctrl" },
	{ .compatible = "st,stih407-front-pinctrl" },
	{ .compatible = "st,stih407-rear-pinctrl" },
	{ .compatible = "st,stih407-flash-pinctrl" },
	{ }
};

const struct pinctrl_ops sti_pinctrl_ops = {
	.set_state = sti_pinctrl_set_state,
};

U_BOOT_DRIVER(pinctrl_sti) = {
	.name = "pinctrl_sti",
	.id = UCLASS_PINCTRL,
	.of_match = sti_pinctrl_ids,
	.ops = &sti_pinctrl_ops,
	.probe = sti_pinctrl_probe,
	.platdata_auto_alloc_size = sizeof(struct sti_pinctrl_platdata),
	.ops = &sti_pinctrl_ops,
};
