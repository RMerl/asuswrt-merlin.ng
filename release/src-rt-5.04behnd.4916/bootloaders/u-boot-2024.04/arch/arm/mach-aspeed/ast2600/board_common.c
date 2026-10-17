// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#include <common.h>
#include <dm.h>
#include <ram.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <dm/uclass.h>
#include <asm/arch/scu_ast2600.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/* Memory Control registers */
#define MCR_BASE			0x1e6e0000
#define MCR_CONF			(MCR_BASE + 0x004)

/* bit fields of MCR_CONF */
#define MCR_CONF_ECC_EN			BIT(7)
#define MCR_CONF_VGA_MEMSZ_MASK		GENMASK(3, 2)
#define MCR_CONF_VGA_MEMSZ_SHIFT	2
#define MCR_CONF_MEMSZ_MASK		GENMASK(1, 0)
#define MCR_CONF_MEMSZ_SHIFT		0

int dram_init(void)
{
	int ret;
	struct udevice *dev;
	struct ram_info ram;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("cannot get DRAM driver\n");
		return ret;
	}

	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("cannot get DRAM information\n");
		return ret;
	}

	gd->ram_size = ram.size;
	return 0;
}

int board_init(void)
{
	int i = 0, rc;
	struct udevice *dev;

	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	while (1) {
		rc = uclass_get_device(UCLASS_MISC, i++, &dev);
		if (rc)
			break;
	}

	return 0;
}

void board_add_ram_info(int use_default)
{
	int rc;
	uint32_t conf;
	uint32_t ecc, act_size, vga_rsvd;
	struct udevice *scu_dev;
	struct ast2600_scu *scu;

	rc = uclass_get_device_by_driver(UCLASS_CLK,
					 DM_DRIVER_GET(aspeed_ast2600_scu), &scu_dev);
	if (rc) {
		debug("%s: cannot find SCU device, rc=%d\n", __func__, rc);
		return;
	}

	scu = devfdt_get_addr_ptr(scu_dev);
	if (IS_ERR_OR_NULL(scu)) {
		debug("%s: cannot get SCU address pointer\n", __func__);
		return;
	}

	conf = readl(MCR_CONF);

	ecc = conf & MCR_CONF_ECC_EN;
	act_size = 0x100 << ((conf & MCR_CONF_MEMSZ_MASK) >> MCR_CONF_MEMSZ_SHIFT);
	vga_rsvd = 0x8 << ((conf & MCR_CONF_VGA_MEMSZ_MASK) >> MCR_CONF_VGA_MEMSZ_SHIFT);

	/* no VGA reservation if efuse VGA disable bit is set */
	if (readl(scu->efuse) & SCU_EFUSE_DIS_VGA)
		vga_rsvd = 0;

	printf(" (capacity:%d MiB, VGA:%d MiB), ECC %s", act_size,
	       vga_rsvd, (ecc) ? "on" : "off");
}

void enable_caches(void)
{
	/* get rid of the warning message */
}
