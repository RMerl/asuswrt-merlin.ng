// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Emlid Limited
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/scu.h>
#include <linux/io.h>

#define BUFCFG_OFFSET				0x100

#define MRFLD_FAMILY_LEN			0x400

/* These are taken from Linux kernel */
#define MRFLD_PINMODE_MASK			0x07

#define pin_to_bufno(f, p)			((p) - (f)->pin_base)

struct mrfld_family {
	unsigned int family_number;
	unsigned int pin_base;
	size_t npins;
	void __iomem *regs;
};

#define MRFLD_FAMILY(b, s, e)				\
	{						\
		.family_number = (b),			\
		.pin_base = (s),			\
		.npins = (e) - (s) + 1,			\
	}

/* Now we only support I2C family of pins */
static struct mrfld_family mrfld_families[] = {
	MRFLD_FAMILY(7, 101, 114),
};

struct mrfld_pinctrl {
	const struct mrfld_family *families;
	size_t nfamilies;
};

static const struct mrfld_family *
mrfld_get_family(struct mrfld_pinctrl *mp, unsigned int pin)
{
	const struct mrfld_family *family;
	unsigned int i;

	for (i = 0; i < mp->nfamilies; i++) {
		family = &mp->families[i];
		if (pin >= family->pin_base &&
		    pin < family->pin_base + family->npins)
			return family;
	}

	pr_err("failed to find family for pin %u\n", pin);
	return NULL;
}

static void __iomem *
mrfld_get_bufcfg(struct mrfld_pinctrl *pinctrl, unsigned int pin)
{
	const struct mrfld_family *family;
	unsigned int bufno;

	family =  mrfld_get_family(pinctrl, pin);
	if (!family)
		return NULL;

	bufno = pin_to_bufno(family, pin);

	return family->regs + BUFCFG_OFFSET + bufno * 4;
}

static void
mrfld_setup_families(void *base_addr,
		     struct mrfld_family *families, unsigned int nfam)
{
	for (int i = 0; i < nfam; i++) {
		struct mrfld_family *family = &families[i];

		family->regs = base_addr +
			       family->family_number * MRFLD_FAMILY_LEN;
	}
}

static int mrfld_pinconfig_protected(unsigned int pin, u32 mask, u32 bits)
{
	struct mrfld_pinctrl *pinctrl;
	struct udevice *dev;
	void __iomem *bufcfg;
	u32 v, value;
	int ret;

	ret = syscon_get_by_driver_data(X86_SYSCON_PINCONF, &dev);
	if (ret)
		return ret;

	pinctrl = dev_get_priv(dev);

	bufcfg = mrfld_get_bufcfg(pinctrl, pin);
	if (!bufcfg)
		return -EINVAL;

	value = readl(bufcfg);

	v = (value & ~mask) | (bits & mask);

	debug("scu: v: 0x%x p: 0x%x bits: %d, mask: %d bufcfg: 0x%p\n",
	      v, (u32)bufcfg, bits, mask, bufcfg);

	ret = scu_ipc_raw_command(IPCMSG_INDIRECT_WRITE, 0, &v, 4,
				  NULL, 0, (u32)bufcfg, 0);
	if (ret)
		pr_err("Failed to set mode via SCU for pin %u (%d)\n",
		       pin, ret);

	return ret;
}

static int mrfld_pinctrl_cfg_pin(ofnode pin_node)
{
	bool is_protected;
	int pad_offset;
	int mode;
	u32 mask;
	int ret;

	/* For now we only support just protected Family of pins */
	is_protected = ofnode_read_bool(pin_node, "protected");
	if (!is_protected)
		return -ENOTSUPP;

	pad_offset = ofnode_read_s32_default(pin_node, "pad-offset", -1);
	if (pad_offset == -1)
		return -EINVAL;

	mode = ofnode_read_s32_default(pin_node, "mode-func", -1);
	if (mode == -1)
		return -EINVAL;

	mask = MRFLD_PINMODE_MASK;

	/* We don't support modes not in range 0..7 */
	if (mode & ~mask)
		return -ENOTSUPP;

	ret = mrfld_pinconfig_protected(pad_offset, mask, mode);

	return ret;
}

static int tangier_pinctrl_probe(struct udevice *dev)
{
	void *base_addr = syscon_get_first_range(X86_SYSCON_PINCONF);
	struct mrfld_pinctrl *pinctrl = dev_get_priv(dev);
	ofnode pin_node;
	int ret;

	mrfld_setup_families(base_addr, mrfld_families,
			     ARRAY_SIZE(mrfld_families));

	pinctrl->families = mrfld_families;
	pinctrl->nfamilies = ARRAY_SIZE(mrfld_families);

	ofnode_for_each_subnode(pin_node, dev_ofnode(dev)) {
		ret = mrfld_pinctrl_cfg_pin(pin_node);
		if (ret) {
			pr_err("%s: invalid configuration for the pin %ld\n",
			       __func__, pin_node.of_offset);
		}
	}

	return 0;
}

static const struct udevice_id tangier_pinctrl_match[] = {
	{ .compatible = "intel,pinctrl-tangier", .data = X86_SYSCON_PINCONF },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(tangier_pinctrl) = {
	.name = "tangier_pinctrl",
	.id = UCLASS_SYSCON,
	.of_match = tangier_pinctrl_match,
	.probe = tangier_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct mrfld_pinctrl),
};
