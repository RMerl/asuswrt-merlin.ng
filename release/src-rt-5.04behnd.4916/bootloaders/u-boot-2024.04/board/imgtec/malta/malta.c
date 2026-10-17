// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2013 Imagination Technologies
 */

#include <config.h>
#include <fdt_support.h>
#include <ide.h>
#include <init.h>
#include <net.h>
#include <netdev.h>
#include <pci.h>
#include <pci_gt64120.h>
#include <pci_msc01.h>
#include <rtc.h>
#include <asm/global_data.h>
#include <linux/delay.h>

#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/malta.h>

#include "superio.h"

DECLARE_GLOBAL_DATA_PTR;

#define MALTA_GT_PATH	 "/pci0@1be00000"
#define MALTA_MSC_PATH	 "/pci0@1bd00000"

enum core_card {
	CORE_UNKNOWN,
	CORE_LV,
	CORE_FPGA6,
};

enum sys_con {
	SYSCON_UNKNOWN,
	SYSCON_GT64120,
	SYSCON_MSC01,
};

static void malta_lcd_puts(const char *str)
{
	int i;
	void *reg = (void *)CKSEG1ADDR(MALTA_ASCIIPOS0);

	/* print up to 8 characters of the string */
	for (i = 0; i < min((int)strlen(str), 8); i++) {
		__raw_writel(str[i], reg);
		reg += MALTA_ASCIIPOS1 - MALTA_ASCIIPOS0;
	}

	/* fill the rest of the display with spaces */
	for (; i < 8; i++) {
		__raw_writel(' ', reg);
		reg += MALTA_ASCIIPOS1 - MALTA_ASCIIPOS0;
	}
}

static enum core_card malta_core_card(void)
{
	u32 corid, rev;
	const void *reg = (const void *)CKSEG1ADDR(MALTA_REVISION);

	rev = __raw_readl(reg);
	corid = (rev & MALTA_REVISION_CORID_MSK) >> MALTA_REVISION_CORID_SHF;

	switch (corid) {
	case MALTA_REVISION_CORID_CORE_LV:
		return CORE_LV;

	case MALTA_REVISION_CORID_CORE_FPGA6:
		return CORE_FPGA6;

	default:
		return CORE_UNKNOWN;
	}
}

static enum sys_con malta_sys_con(void)
{
	switch (malta_core_card()) {
	case CORE_LV:
		return SYSCON_GT64120;

	case CORE_FPGA6:
		return SYSCON_MSC01;

	default:
		return SYSCON_UNKNOWN;
	}
}

int dram_init(void)
{
	gd->ram_size = CFG_SYS_SDRAM_SIZE;

	return 0;
}

int checkboard(void)
{
	enum core_card core;

	malta_lcd_puts("U-Boot");
	puts("Board: MIPS Malta");

	core = malta_core_card();
	switch (core) {
	case CORE_LV:
		puts(" CoreLV");
		break;

	case CORE_FPGA6:
		puts(" CoreFPGA6");
		break;

	default:
		puts(" CoreUnknown");
	}

	putc('\n');
	return 0;
}

#if !IS_ENABLED(CONFIG_DM_ETH)
int board_eth_init(struct bd_info *bis)
{
	return pci_eth_init(bis);
}
#endif

void _machine_restart(void)
{
	void __iomem *reset_base;

	reset_base = (void __iomem *)CKSEG1ADDR(MALTA_RESET_BASE);
	__raw_writel(GORESET, reset_base);
	mdelay(1000);
}

int board_early_init_f(void)
{
	ulong io_base;

	/* choose correct PCI I/O base */
	switch (malta_sys_con()) {
	case SYSCON_GT64120:
		io_base = CKSEG1ADDR(MALTA_GT_PCIIO_BASE);
		break;

	case SYSCON_MSC01:
		io_base = CKSEG1ADDR(MALTA_MSC01_PCIIO_BASE);
		break;

	default:
		return -1;
	}

	set_io_port_base(io_base);

	/* setup FDC37M817 super I/O controller */
	malta_superio_init();

	return 0;
}

int misc_init_r(void)
{
	rtc_reset();

	return 0;
}

#if IS_ENABLED(CONFIG_OF_BOARD_FIXUP)
/*
 * TODO: currently doesn't work because rw_fdt_blob points to a
 * NOR flash address. This needs some changes in board_init_f.
 */
int board_fix_fdt(void *rw_fdt_blob)
{
	int node = -1;

	switch (malta_sys_con()) {
	case SYSCON_GT64120:
		node =  fdt_path_offset(rw_fdt_blob, MALTA_GT_PATH);
		break;
	default:
	case SYSCON_MSC01:
		node =  fdt_path_offset(rw_fdt_blob, MALTA_MSC_PATH);
		break;
	}

	return fdt_status_okay(rw_fdt_blob, node);
}
#endif

int board_early_init_r(void)
{
	struct udevice *dev;
	int ret;

	pci_init();

	ret = dm_pci_find_device(PCI_VENDOR_ID_INTEL,
				 PCI_DEVICE_ID_INTEL_82371AB_0, 0, &dev);
	if (ret)
		panic("Failed to find PIIX4 PCI bridge\n");

	/* setup PCI interrupt routing */
	dm_pci_write_config8(dev, PCI_CFG_PIIX4_PIRQRCA, 10);
	dm_pci_write_config8(dev, PCI_CFG_PIIX4_PIRQRCB, 10);
	dm_pci_write_config8(dev, PCI_CFG_PIIX4_PIRQRCC, 11);
	dm_pci_write_config8(dev, PCI_CFG_PIIX4_PIRQRCD, 11);

	/* mux SERIRQ onto SERIRQ pin */
	dm_pci_clrset_config32(dev, PCI_CFG_PIIX4_GENCFG, 0,
			       PCI_CFG_PIIX4_GENCFG_SERIRQ);

	/* enable SERIRQ - Linux currently depends upon this */
	dm_pci_clrset_config8(dev, PCI_CFG_PIIX4_SERIRQC, 0,
			      PCI_CFG_PIIX4_SERIRQC_EN | PCI_CFG_PIIX4_SERIRQC_CONT);

	ret = dm_pci_find_device(PCI_VENDOR_ID_INTEL,
				 PCI_DEVICE_ID_INTEL_82371AB, 0, &dev);
	if (ret)
		panic("Failed to find PIIX4 IDE controller\n");

	/* enable bus master & IO access */
	dm_pci_clrset_config32(dev, PCI_COMMAND, 0,
			       PCI_COMMAND_MASTER | PCI_COMMAND_IO);

	/* set latency */
	dm_pci_write_config8(dev, PCI_LATENCY_TIMER, 0x40);

	/* enable IDE/ATA */
	dm_pci_write_config32(dev, PCI_CFG_PIIX4_IDETIM_PRI,
			      PCI_CFG_PIIX4_IDETIM_IDE);
	dm_pci_write_config32(dev, PCI_CFG_PIIX4_IDETIM_SEC,
			      PCI_CFG_PIIX4_IDETIM_IDE);

	return 0;
}
