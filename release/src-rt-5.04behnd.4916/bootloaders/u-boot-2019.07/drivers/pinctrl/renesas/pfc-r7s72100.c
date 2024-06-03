// SPDX-License-Identifier: GPL-2.0+
/*
 * R7S72100 processor support
 *
 * Copyright (C) 2019 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <linux/io.h>
#include <linux/err.h>

#define P(bank)			(0x0000 + (bank) * 4)
#define PSR(bank)		(0x0100 + (bank) * 4)
#define PPR(bank)		(0x0200 + (bank) * 4)
#define PM(bank)		(0x0300 + (bank) * 4)
#define PMC(bank)		(0x0400 + (bank) * 4)
#define PFC(bank)		(0x0500 + (bank) * 4)
#define PFCE(bank)		(0x0600 + (bank) * 4)
#define PNOT(bank)		(0x0700 + (bank) * 4)
#define PMSR(bank)		(0x0800 + (bank) * 4)
#define PMCSR(bank)		(0x0900 + (bank) * 4)
#define PFCAE(bank)		(0x0A00 + (bank) * 4)
#define PIBC(bank)		(0x4000 + (bank) * 4)
#define PBDC(bank)		(0x4100 + (bank) * 4)
#define PIPC(bank)		(0x4200 + (bank) * 4)

#define RZA1_PINS_PER_PORT	16

DECLARE_GLOBAL_DATA_PTR;

struct r7s72100_pfc_platdata {
	void __iomem	*base;
};

static void r7s72100_pfc_set_function(struct udevice *dev, u16 bank, u16 line,
				      u16 func, u16 inbuf, u16 bidir)
{
	struct r7s72100_pfc_platdata *plat = dev_get_platdata(dev);

	clrsetbits_le16(plat->base + PFCAE(bank), BIT(line),
			(func & BIT(2)) ? BIT(line) : 0);
	clrsetbits_le16(plat->base + PFCE(bank), BIT(line),
			(func & BIT(1)) ? BIT(line) : 0);
	clrsetbits_le16(plat->base + PFC(bank), BIT(line),
			(func & BIT(0)) ? BIT(line) : 0);

	clrsetbits_le16(plat->base + PIBC(bank), BIT(line),
			inbuf ? BIT(line) : 0);
	clrsetbits_le16(plat->base + PBDC(bank), BIT(line),
			bidir ? BIT(line) : 0);

	setbits_le32(plat->base + PMCSR(bank), BIT(line + 16) | BIT(line));

	setbits_le16(plat->base + PIPC(bank), BIT(line));
}

static int r7s72100_pfc_set_state(struct udevice *dev, struct udevice *config)
{
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(config);
	u32 cells[32];
	u16 bank, line, func;
	int i, count, bidir;

	count = fdtdec_get_int_array_count(blob, node, "pinmux",
					   cells, ARRAY_SIZE(cells));
	if (count < 0) {
		printf("%s: bad pinmux array %d\n", __func__, count);
		return -EINVAL;
	}

	if (count > ARRAY_SIZE(cells)) {
		printf("%s: unsupported pinmux array count %d\n",
		       __func__, count);
		return -EINVAL;
	}

	for (i = 0 ; i < count; i++) {
		func = (cells[i] >> 16) & 0xf;
		if (func == 0 || func > 8) {
			printf("Invalid cell %i in node %s!\n",
			       count, ofnode_get_name(dev_ofnode(config)));
			continue;
		}

		func = (func - 1) & 0x7;

		bank = (cells[i] / RZA1_PINS_PER_PORT) & 0xff;
		line = cells[i] % RZA1_PINS_PER_PORT;

		bidir = 0;
		if (bank == 3 && line == 3 && func == 1)
			bidir = 1;

		r7s72100_pfc_set_function(dev, bank, line, func, 0, bidir);
	}

	return 0;
}

const struct pinctrl_ops r7s72100_pfc_ops  = {
	.set_state = r7s72100_pfc_set_state,
};

static int r7s72100_pfc_probe(struct udevice *dev)
{
	struct r7s72100_pfc_platdata *plat = dev_get_platdata(dev);
	fdt_addr_t addr_base;
	ofnode node;

	addr_base = devfdt_get_addr(dev);
	if (addr_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = (void __iomem *)addr_base;

	dev_for_each_subnode(node, dev) {
		struct udevice *cdev;

		if (!ofnode_read_bool(node, "gpio-controller"))
			continue;

		device_bind_driver_to_node(dev, "r7s72100-gpio",
					   ofnode_get_name(node),
					   node, &cdev);
	}

	return 0;
}

static const struct udevice_id r7s72100_pfc_match[] = {
	{ .compatible = "renesas,r7s72100-ports" },
	{}
};

U_BOOT_DRIVER(r7s72100_pfc) = {
	.name		= "r7s72100_pfc",
	.id		= UCLASS_PINCTRL,
	.of_match	= r7s72100_pfc_match,
	.probe		= r7s72100_pfc_probe,
	.platdata_auto_alloc_size = sizeof(struct r7s72100_pfc_platdata),
	.ops		= &r7s72100_pfc_ops,
};
