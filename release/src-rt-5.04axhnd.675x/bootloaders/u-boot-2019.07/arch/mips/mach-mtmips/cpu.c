// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <wdt.h>
#include <asm/io.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include "mt76xx.h"

#define STR_LEN			6

#ifdef CONFIG_BOOT_ROM
int mach_cpu_init(void)
{
	ddr_calibrate();

	return 0;
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, SZ_256M);

	return 0;
}

int print_cpuinfo(void)
{
	static const char * const boot_str[] = { "PLL (3-Byte SPI Addr)",
						 "PLL (4-Byte SPI Addr)",
						 "XTAL (3-Byte SPI Addr)",
						 "XTAL (4-Byte SPI Addr)" };
	const void *blob = gd->fdt_blob;
	void __iomem *sysc_base;
	char buf[STR_LEN + 1];
	fdt_addr_t base;
	fdt_size_t size;
	char *str;
	int node;
	u32 val;

	/* Get system controller base address */
	node = fdt_node_offset_by_compatible(blob, -1, "ralink,mt7620a-sysc");
	if (node < 0)
		return -FDT_ERR_NOTFOUND;

	base = fdtdec_get_addr_size_auto_noparent(blob, node, "reg",
						  0, &size, true);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	sysc_base = ioremap_nocache(base, size);

	str = (char *)sysc_base + MT76XX_CHIPID_OFFS;
	snprintf(buf, STR_LEN + 1, "%s", str);
	val = readl(sysc_base + MT76XX_CHIP_REV_ID_OFFS);
	printf("CPU:   %-*s Rev %ld.%ld - ", STR_LEN, buf,
	       (val & GENMASK(11, 8)) >> 8, val & GENMASK(3, 0));

	val = (readl(sysc_base + MT76XX_SYSCFG0_OFFS) & GENMASK(3, 1)) >> 1;
	printf("Boot from %s\n", boot_str[val]);

	return 0;
}

int last_stage_init(void)
{
	void *src, *dst;

	src = malloc(SZ_64K);
	dst = malloc(SZ_64K);
	if (!src || !dst) {
		printf("Can't allocate buffer for cache cleanup copy!\n");
		return 0;
	}

	/*
	 * It has been noticed, that sometimes the d-cache is not in a
	 * "clean-state" when U-Boot is running on MT7688. This was
	 * detected when using the ethernet driver (which uses d-cache)
	 * and a TFTP command does not complete. Copying an area of 64KiB
	 * in DDR at a very late bootup time in U-Boot, directly before
	 * calling into the prompt, seems to fix this issue.
	 */
	memcpy(dst, src, SZ_64K);
	free(src);
	free(dst);

	return 0;
}
