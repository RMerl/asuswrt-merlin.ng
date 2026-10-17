// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 * Sharma Bhupesh <bhupesh.sharma@freescale.com>
 */
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <init.h>
#include <malloc.h>
#include <errno.h>
#include <net.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/sizes.h>
#include <dm/platform_data/serial_pl01x.h>
#include "pcie.h"
#include <asm/armv8/mmu.h>
#ifdef CONFIG_VIRTIO_NET
#include <virtio_types.h>
#include <virtio.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static const struct pl01x_serial_plat serial_plat = {
	.base = V2M_UART0,
	.type = TYPE_PL011,
	.clock = CFG_PL011_CLOCK,
};

U_BOOT_DRVINFO(vexpress_serials) = {
	.name = "serial_pl01x",
	.plat = &serial_plat,
};

static struct mm_region vexpress64_mem_map[] = {
	{
		.virt = V2M_PA_BASE,
		.phys = V2M_PA_BASE,
		.size = SZ_2G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = V2M_DRAM_BASE,
		.phys = V2M_DRAM_BASE,
		.size = SZ_2G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/*
		 * DRAM beyond 2 GiB is located high. Let's map just some
		 * of it, although U-Boot won't realistically use it, and
		 * the actual available amount might be smaller on the model.
		 */
		.virt = 0x880000000UL,		/* 32 + 2 GiB */
		.phys = 0x880000000UL,
		.size = 6UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = vexpress64_mem_map;

/* This function gets replaced by platforms supporting PCIe.
 * The replacement function, eg. on Juno, initialises the PCIe bus.
 */
__weak void vexpress64_pcie_init(void)
{
}

int board_init(void)
{
	vexpress64_pcie_init();
#ifdef CONFIG_VIRTIO_NET
	virtio_init();
#endif
	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

/* Assigned in lowlevel_init.S
 * Push the variable into the .data section so that it
 * does not get cleared later.
 */
unsigned long __section(".data") prior_stage_fdt_address[2];

#ifdef CONFIG_OF_BOARD

#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
#define JUNO_FLASH_SEC_SIZE	(256 * 1024)
static phys_addr_t find_dtb_in_nor_flash(const char *partname)
{
	phys_addr_t sector = CFG_SYS_FLASH_BASE;
	int i;

	for (i = 0;
	     i < CONFIG_SYS_MAX_FLASH_SECT;
	     i++, sector += JUNO_FLASH_SEC_SIZE) {
		int len = strlen(partname) + 1;
		int offs;
		phys_addr_t imginfo;
		u32 reg;

		reg = readl(sector + JUNO_FLASH_SEC_SIZE - 0x04);
                /* This makes up the string "HSLFTOOF" flash footer */
		if (reg != 0x464F4F54U)
			continue;
		reg = readl(sector + JUNO_FLASH_SEC_SIZE - 0x08);
                if (reg != 0x464C5348U)
			continue;

		for (offs = 0; offs < 32; offs += 4, len -= 4) {
			reg = readl(sector + JUNO_FLASH_SEC_SIZE - 0x30 + offs);
			if (strncmp(partname + offs, (char *)&reg,
			            len > 4 ? 4 : len))
				break;

			if (len > 4)
				continue;

			reg = readl(sector + JUNO_FLASH_SEC_SIZE - 0x10);
			imginfo = sector + JUNO_FLASH_SEC_SIZE - 0x30 - reg;
			reg = readl(imginfo + 0x54);

			return CFG_SYS_FLASH_BASE +
			       reg * JUNO_FLASH_SEC_SIZE;
		}
	}

	printf("No DTB found\n");

	return ~0;
}
#endif

/*
 * Filter for a valid DTB, as TF-A happens to provide a pointer to some
 * data structure using the DTB format, which we cannot use.
 * The address of the DTB cannot be 0, in fact this is the reserved value
 * for x1 in the kernel boot protocol.
 * And while the nt_fw_config.dtb used by TF-A is a valid DTB structure, it
 * does not contain the typical nodes and properties, which we test for by
 * probing for the mandatory /memory node.
 */
static bool is_valid_dtb(uintptr_t dtb_ptr)
{
	if (dtb_ptr == 0 || fdt_magic(dtb_ptr) != FDT_MAGIC)
		return false;

	return fdt_subnode_offset((void *)dtb_ptr, 0, "memory") >= 0;
}

void *board_fdt_blob_setup(int *err)
{
#ifdef CONFIG_TARGET_VEXPRESS64_JUNO
	phys_addr_t fdt_rom_addr = find_dtb_in_nor_flash(CONFIG_JUNO_DTB_PART);

	*err = 0;
	if (fdt_rom_addr == ~0UL) {
		*err = -ENXIO;
		return NULL;
	}

	return (void *)fdt_rom_addr;
#endif

#ifdef VEXPRESS_FDT_ADDR
	if (fdt_magic(VEXPRESS_FDT_ADDR) == FDT_MAGIC) {
		*err = 0;
		return (void *)VEXPRESS_FDT_ADDR;
	}
#endif

	if (is_valid_dtb(prior_stage_fdt_address[1])) {
		*err = 0;
		return (void *)prior_stage_fdt_address[1];
	} else if (is_valid_dtb(prior_stage_fdt_address[0])) {
		*err = 0;
		return (void *)prior_stage_fdt_address[0];
	}

	if (fdt_magic(gd->fdt_blob) == FDT_MAGIC) {
		*err = 0;
		return (void *)gd->fdt_blob;
	}

	*err = -ENXIO;
	return NULL;
}
#endif

/* Actual reset is done via PSCI. */
void reset_cpu(void)
{
}

/*
 * Board specific ethernet initialization routine.
 */
int board_eth_init(struct bd_info *bis)
{
	int rc = 0;
#ifndef CONFIG_DM_ETH
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CFG_SMC911X_BASE);
#endif
#endif
	return rc;
}
