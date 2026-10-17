// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023 SiFive
 */

#include <cache.h>
#include <dm.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/device-internal.h>

#define	SIFIVE_PL2CHICKENBIT_OFFSET			0x1000
#define	SIFIVE_PL2CHICKENBIT_REGIONCLOCKDISABLE_MASK	BIT(3)

static int sifive_pl2_probe(struct udevice *dev)
{
	fdt_addr_t base;
	u32 val;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Enable regionClockDisable bit */
	val = readl((void __iomem *)(base + SIFIVE_PL2CHICKENBIT_OFFSET));
	writel(val & ~SIFIVE_PL2CHICKENBIT_REGIONCLOCKDISABLE_MASK,
	       (void __iomem *)(base + SIFIVE_PL2CHICKENBIT_OFFSET));

	return 0;
}

static const struct udevice_id sifive_pl2_ids[] = {
	{ .compatible = "sifive,pl2cache0" },
	{ .compatible = "sifive,pl2cache1" },
	{}
};

U_BOOT_DRIVER(sifive_pl2) = {
	.name = "sifive_pl2",
	.id = UCLASS_CACHE,
	.of_match = sifive_pl2_ids,
	.probe = sifive_pl2_probe,
};
