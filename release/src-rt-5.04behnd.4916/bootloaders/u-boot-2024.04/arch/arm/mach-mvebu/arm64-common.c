// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <init.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Not all memory is mapped in the MMU. So we need to restrict the
 * memory size so that U-Boot does not try to access it. Also, the
 * internal registers are located at 0xf000.0000 - 0xffff.ffff.
 * Currently only 2GiB are mapped for system memory. This is what
 * we pass to the U-Boot subsystem here.
 */
#define USABLE_RAM_SIZE		0x80000000ULL

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	unsigned long top = CFG_SYS_SDRAM_BASE + min(gd->ram_size, USABLE_RAM_SIZE);

	return (gd->ram_top > top) ? top : gd->ram_top;
}

/*
 * On ARMv8, MBus is not configured in U-Boot. To enable compilation
 * of the already implemented drivers, lets add a dummy version of
 * this function so that linking does not fail.
 */
const struct mbus_dram_target_info *mvebu_mbus_dram_info(void)
{
	return NULL;
}

__weak int dram_init_banksize(void)
{
	if (IS_ENABLED(CONFIG_ARMADA_8K))
		return a8k_dram_init_banksize();
	else if (IS_ENABLED(CONFIG_ARMADA_3700))
		return a3700_dram_init_banksize();
	else if (IS_ENABLED(CONFIG_ALLEYCAT_5))
		return alleycat5_dram_init_banksize();
	else
		return fdtdec_setup_memory_banksize();
}

__weak int dram_init(void)
{
	if (IS_ENABLED(CONFIG_ARMADA_8K)) {
		gd->ram_size = a8k_dram_scan_ap_sz();
		if (gd->ram_size != 0)
			return 0;
	}

	if (IS_ENABLED(CONFIG_ARMADA_3700))
		return a3700_dram_init();

	if (IS_ENABLED(CONFIG_ALLEYCAT_5))
		return alleycat5_dram_init();

	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int arch_cpu_init(void)
{
	/* Nothing to do (yet) */
	return 0;
}

int arch_early_init_r(void)
{
	struct udevice *dev;
	int ret;
	int i;

	/*
	 * Loop over all MISC uclass drivers to call the comphy code
	 * and init all CP110 devices enabled in the DT
	 */
	i = 0;
	while (1) {
		/* Call the comphy code via the MISC uclass driver */
		ret = uclass_get_device(UCLASS_MISC, i++, &dev);

		/* We're done, once no further CP110 device is found */
		if (ret)
			break;
	}

	/* Cause the SATA device to do its early init */
	uclass_first_device(UCLASS_AHCI, &dev);

	/* Trigger PCIe devices detection */
	if (IS_ENABLED(CONFIG_PCI))
		pci_init();

	return 0;
}
