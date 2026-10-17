// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <init.h>
#include <image.h>
#include <vsprintf.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <asm/addrspace.h>
#include <asm/byteorder.h>
#include <asm/global_data.h>
#include <linux/sizes.h>
#include <linux/types.h>
#include <mach/serial.h>
#include "../mt7621.h"
#include "dram.h"
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

struct tpl_info {
	u32 magic;
	u32 size;
};

void set_timer_freq_simple(void)
{
	u32 div = get_xtal_mhz();

	/* Round down cpu freq */
	gd->arch.timer_freq = rounddown(CONFIG_MT7621_CPU_FREQ, div) * 500000;
}

void __noreturn board_init_f(ulong dummy)
{
	spl_init();

#ifdef CONFIG_SPL_SERIAL
	/*
	 * mtmips_spl_serial_init() is useful if debug uart is enabled,
	 * or DM based serial is not enabled.
	 */
	mtmips_spl_serial_init();
	preloader_console_init();
#endif

	board_init_r(NULL, 0);
}

void board_boot_order(u32 *spl_boot_list)
{
#ifdef CONFIG_MT7621_BOOT_FROM_NAND
	spl_boot_list[0] = BOOT_DEVICE_NAND;
#else
	spl_boot_list[0] = BOOT_DEVICE_NOR;
#endif
}

unsigned long spl_nor_get_uboot_base(void)
{
	const struct tpl_info *tpli;
	const struct legacy_img_hdr *hdr;
	u32 addr;

	addr = FLASH_MMAP_BASE + TPL_INFO_OFFSET;
	tpli = (const struct tpl_info *)KSEG1ADDR(addr);

	if (tpli->magic == TPL_INFO_MAGIC) {
		addr = FLASH_MMAP_BASE + tpli->size;
		hdr = (const struct legacy_img_hdr *)KSEG1ADDR(addr);

		if (image_get_magic(hdr) == IH_MAGIC) {
			addr += sizeof(*hdr) + image_get_size(hdr);
			return KSEG1ADDR(addr);
		}
	}

	panic("Unable to locate SPL payload\n");
	return 0;
}

uint32_t spl_nand_get_uboot_raw_page(void)
{
	const struct stage_header *sh = (const struct stage_header *)_start;
	u32 addr;

	addr = image_get_header_size() + be32_to_cpu(sh->stage_size);
	addr = ALIGN(addr, SZ_4K);

	return addr;
}
