// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <image.h>
#include <asm/system.h>
#include <asm/sections.h>
#include <asm/cacheops.h>
#include <asm/mipsregs.h>
#include <asm/cm.h>

#define INDEX_STORE_DATA_SD	0x0f

typedef void __noreturn (*image_entry_noargs_t)(void);

/*
 * Lock L2 cache and fill data
 * Assume that data is 4-byte aligned and start_addr/size is 32-byte aligned
 */
static void fill_lock_l2cache(uintptr_t dataptr, ulong start_addr, ulong size)
{
	ulong slsize = CONFIG_SYS_DCACHE_LINE_SIZE;
	ulong end_addr = start_addr + size;
	const u32 *data = (u32 *)dataptr;
	ulong i, addr;
	u32 val;

	/* Clear WSC & SPR bit in ErrCtl */
	val = read_c0_ecc();
	val &= 0xcfffffff;
	write_c0_ecc(val);
	execution_hazard_barrier();

	for (addr = start_addr; addr < end_addr; addr += slsize) {
		/* Set STagLo to lock cache line */
		write_c0_staglo((addr & 0x1ffff800) | 0xa0);
		mips_cache(INDEX_STORE_TAG_SD, (void *)addr);

		/* Fill data */
		for (i = 0; i < slsize; i += 8) {
			val = *data++;
			__write_32bit_c0_register($28, 5, val); /* sdtaglo */
			val = *data++;
			__write_32bit_c0_register($29, 5, val); /* sdtaghi */
			mips_cache(INDEX_STORE_DATA_SD, (void *)(addr + i));
		}
	}

	sync();
}

/* A simple function to initialize MT7621's cache */
static void mt7621_cache_init(void)
{
	void __iomem *cm_base = (void *)KSEG1ADDR(CONFIG_MIPS_CM_BASE);
	ulong lsize = CONFIG_SYS_DCACHE_LINE_SIZE;
	ulong addr;
	u32 val;

	/* Enable CCA override. Set to uncached */
	val = readl(cm_base + GCR_BASE);
	val &= ~CCA_DEFAULT_OVR_MASK;
	val |= CCA_DEFAULT_OVREN | (2 << CCA_DEFAULT_OVR_SHIFT);
	writel(val, cm_base + GCR_BASE);

	/* Initialize L1 I-Cache */
	write_c0_taglo(0);
	write_c0_taghi(0);

	for (addr = 0; addr < CONFIG_SYS_ICACHE_SIZE; addr += lsize)
		mips_cache(INDEX_STORE_TAG_I, (void *)addr);

	/* Initialize L1 D-Cache */
	write_c0_dtaglo(0);
	__write_32bit_c0_register($29, 2, 0); /* dtaghi */

	for (addr = 0; addr < CONFIG_SYS_DCACHE_SIZE; addr += lsize)
		mips_cache(INDEX_STORE_TAG_D, (void *)addr);

	/* Initialize L2 Cache */
	write_c0_staglo(0);
	__write_32bit_c0_register($29, 4, 0); /* staghi */

	for (addr = 0; addr < (256 << 10); addr += lsize)
		mips_cache(INDEX_STORE_TAG_SD, (void *)addr);

	/* Dsiable CCA override */
	val = readl(cm_base + GCR_BASE);
	val &= ~(CCA_DEFAULT_OVR_MASK | CCA_DEFAULT_OVREN);
	writel(val, cm_base + GCR_BASE);

	/* Set KSEG0 to non-coherent cached (important!) */
	val = read_c0_config();
	val &= ~CONF_CM_CMASK;
	val |= CONF_CM_CACHABLE_NONCOHERENT;
	write_c0_config(val);
	execution_hazard_barrier();

	/* Again, invalidate L1 D-Cache */
	for (addr = 0; addr < CONFIG_SYS_DCACHE_SIZE; addr += lsize)
		mips_cache(INDEX_WRITEBACK_INV_D, (void *)addr);

	/* Invalidate L1 I-Cache */
	for (addr = 0; addr < CONFIG_SYS_ICACHE_SIZE; addr += lsize)
		mips_cache(INDEX_INVALIDATE_I, (void *)addr);

	/* Disable L2 cache bypass */
	val = read_c0_config2();
	val &= ~MIPS_CONF_IMPL;
	write_c0_config2(val);
	execution_hazard_barrier();
}

void __noreturn tpl_main(void)
{
	const struct legacy_img_hdr *hdr = (const struct legacy_img_hdr *)__image_copy_end;
	image_entry_noargs_t image_entry;
	u32 loadaddr, size;
	uintptr_t data;

	/* Initialize the cache first */
	mt7621_cache_init();

	if (image_get_magic(hdr) != IH_MAGIC)
		goto failed;

	loadaddr = image_get_load(hdr);
	size = image_get_size(hdr);
	image_entry = (image_entry_noargs_t)image_get_ep(hdr);

	/* Load TPL image to L2 cache */
	data = (uintptr_t)__image_copy_end + sizeof(struct legacy_img_hdr);
	fill_lock_l2cache(data, loadaddr, size);

	/* Jump to SPL */
	image_entry();

failed:
	for (;;)
		;
}
