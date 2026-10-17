// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022 ARM Limited
 * (C) Copyright 2022 Linaro
 * Rui Miguel Silva <rui.silva@linaro.org>
 */

#include <blk.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <env.h>
#include <fwu.h>
#include <netdev.h>
#include <nvmxip.h>
#include <part.h>
#include <dm/platform_data/serial_pl01x.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>

#define CORSTONE1000_KERNEL_PARTS 2
#define CORSTONE1000_KERNEL_PRIMARY "kernel_primary"
#define CORSTONE1000_KERNEL_SECONDARY "kernel_secondary"

static int corstone1000_boot_idx;

static struct mm_region corstone1000_mem_map[] = {
	{
		/* CVM */
		.virt = 0x02000000UL,
		.phys = 0x02000000UL,
		.size = 0x02000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	}, {
		/* QSPI */
		.virt = 0x08000000UL,
		.phys = 0x08000000UL,
		.size = 0x08000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	}, {
		/* Host Peripherals */
		.virt = 0x1A000000UL,
		.phys = 0x1A000000UL,
		.size = 0x26000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* USB */
		.virt = 0x40200000UL,
		.phys = 0x40200000UL,
		.size = 0x00100000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* ethernet */
		.virt = 0x40100000UL,
		.phys = 0x40100000UL,
		.size = 0x00100000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* OCVM */
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = corstone1000_mem_map;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

void fwu_plat_get_bootidx(uint *boot_idx)
{
	int ret;

	/*
	 * in our platform, the Secure Enclave is the one who controls
	 * all the boot tries and status, so, every time we get here
	 * we know that the we are booting from the active index
	 */
	ret = fwu_get_active_index(boot_idx);
	if (ret < 0) {
		*boot_idx = CONFIG_FWU_NUM_BANKS;
		log_err("corstone1000: failed to read active index\n");
	}
}

int board_late_init(void)
{
	struct disk_partition part_info;
	struct udevice *dev, *bdev;
	struct nvmxip_plat *plat;
	struct blk_desc *desc;
	int ret;

	ret = uclass_first_device_err(UCLASS_NVMXIP, &dev);
	if (ret < 0) {
		log_err("Cannot find kernel device\n");
		return ret;
	}

	plat = dev_get_plat(dev);
	device_find_first_child(dev, &bdev);
	desc = dev_get_uclass_plat(bdev);
	ret = fwu_get_active_index(&corstone1000_boot_idx);
	if (ret < 0) {
		log_err("corstone1000: failed to read boot index\n");
		return ret;
	}

	if (!corstone1000_boot_idx)
		ret = part_get_info_by_name(desc, CORSTONE1000_KERNEL_PRIMARY,
					    &part_info);
	else
		ret = part_get_info_by_name(desc, CORSTONE1000_KERNEL_SECONDARY,
					    &part_info);

	if (ret < 0) {
		log_err("failed to fetch kernel partition index: %d\n",
			corstone1000_boot_idx);
		return ret;
	}

	ret = 0;

	ret |= env_set_hex("kernel_addr", plat->phys_base +
			   (part_info.start * part_info.blksz));
	ret |= env_set_hex("kernel_size", part_info.size * part_info.blksz);

	if (ret < 0)
		log_err("failed to setup kernel addr and size\n");

	return ret;
}
