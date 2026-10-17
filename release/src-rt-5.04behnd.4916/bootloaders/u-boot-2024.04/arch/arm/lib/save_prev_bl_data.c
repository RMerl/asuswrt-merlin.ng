// SPDX-License-Identifier: GPL-2.0+
/*
 * save_prev_bl_data - saving previous bootloader data
 * to environment variables.
 *
 * Copyright (c) 2022 Dzmitry Sankouski (dsankouski@gmail.com)
 */
#include <init.h>
#include <env.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <fdt.h>
#include <common.h>
#include <linux/errno.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>

static ulong reg0 __section(".data");

/**
 * Save x0 register value, assuming previous bootloader set it to
 * point on loaded fdt or (for older linux kernels)atags.
 */
void save_boot_params(ulong r0)
{
	reg0 = r0;
	save_boot_params_ret();
}

bool is_addr_accessible(phys_addr_t addr)
{
	struct mm_region *mem = mem_map;
	phys_addr_t bank_start;
	phys_addr_t bank_end;

	while (mem->size) {
		bank_start = mem->phys;
		bank_end = bank_start + mem->size;
		debug("check if block %pap - %pap includes %pap\n", &bank_start, &bank_end, &addr);
		if (addr > bank_start && addr < bank_end)
			return true;
		mem++;
	}

	return false;
}

int save_prev_bl_data(void)
{
	struct fdt_header *fdt_blob;
	int node;
	u64 initrd_start_prop;

	if (!is_addr_accessible((phys_addr_t)reg0))
		return -ENODATA;

	fdt_blob = (struct fdt_header *)reg0;
	if (!fdt_valid(&fdt_blob)) {
		pr_warn("%s: address 0x%lx is not a valid fdt\n", __func__, reg0);
		return -ENODATA;
	}

	if (IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		env_set_addr("prevbl_fdt_addr", (void *)reg0);
	if (!IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_START_ADDR))
		return 0;

	node = fdt_path_offset(fdt_blob, "/chosen");
	if (!node) {
		pr_warn("%s: chosen node not found in device tree at addr: 0x%lx\n",
					__func__, reg0);
		return -ENODATA;
	}
	/*
	 * linux,initrd-start property might be either 64 or 32 bit,
	 * depending on primary bootloader implementation.
	 */
	initrd_start_prop = fdtdec_get_uint64(fdt_blob, node, "linux,initrd-start", 0);
	if (!initrd_start_prop) {
		debug("%s: attempt to get uint64 linux,initrd-start property failed, trying uint\n",
				__func__);
		initrd_start_prop = fdtdec_get_uint(fdt_blob, node, "linux,initrd-start", 0);
		if (!initrd_start_prop) {
			debug("%s: attempt to get uint failed, too\n", __func__);
			return -ENODATA;
		}
	}
	env_set_addr("prevbl_initrd_start_addr", (void *)initrd_start_prop);

	return 0;
}
