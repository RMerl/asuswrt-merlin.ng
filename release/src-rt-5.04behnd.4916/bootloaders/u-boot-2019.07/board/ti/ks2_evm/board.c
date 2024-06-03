// SPDX-License-Identifier: GPL-2.0+
/*
 * Keystone : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include "board.h"
#include <spl.h>
#include <exports.h>
#include <fdt_support.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/psc_defs.h>
#include <asm/arch/clock.h>
#include <asm/ti-common/ti-aemif.h>
#include <asm/ti-common/keystone_net.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_TI_AEMIF)
static struct aemif_config aemif_configs[] = {
	{			/* CS0 */
		.mode		= AEMIF_MODE_NAND,
		.wr_setup	= 0xf,
		.wr_strobe	= 0x3f,
		.wr_hold	= 7,
		.rd_setup	= 0xf,
		.rd_strobe	= 0x3f,
		.rd_hold	= 7,
		.turn_around	= 3,
		.width		= AEMIF_WIDTH_8,
	},
};
#endif

int dram_init(void)
{
	u32 ddr3_size;

	ddr3_size = ddr3_init();

	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_MAX_RAM_BANK_SIZE);
#if defined(CONFIG_TI_AEMIF)
	if (!board_is_k2g_ice())
		aemif_init(ARRAY_SIZE(aemif_configs), aemif_configs);
#endif

	if (!board_is_k2g_ice()) {
		if (ddr3_size)
			ddr3_init_ecc(KS2_DDR3A_EMIF_CTRL_BASE, ddr3_size);
		else
			ddr3_init_ecc(KS2_DDR3A_EMIF_CTRL_BASE,
				      gd->ram_size >> 30);
	}

	return 0;
}

struct image_header *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (struct image_header *)(CONFIG_SYS_TEXT_BASE);
}

int board_init(void)
{
#if CONFIG_IS_ENABLED(DM_USB)
	int rc = psc_enable_module(KS2_LPSC_USB);

	if (rc)
		puts("Cannot enable USB0 module");
#ifdef KS2_LPSC_USB_1
	rc = psc_enable_module(KS2_LPSC_USB_1);
	if (rc)
		puts("Cannot enable USB1 module");
#endif
#endif

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
	spl_init_keystone_plls();
	preloader_console_init();
}

u32 spl_boot_device(void)
{
#if defined(CONFIG_SPL_SPI_LOAD)
	return BOOT_DEVICE_SPI;
#else
	puts("Unknown boot device\n");
	hang();
#endif
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	int lpae;
	char *env;
	char *endp;
	int nbanks;
	u64 size[2];
	u64 start[2];
	u32 ddr3a_size;

	env = env_get("mem_lpae");
	lpae = env && simple_strtol(env, NULL, 0);

	ddr3a_size = 0;
	if (lpae) {
		ddr3a_size = ddr3_get_size();
		if ((ddr3a_size != 8) && (ddr3a_size != 4))
			ddr3a_size = 0;
	}

	nbanks = 1;
	start[0] = bd->bi_dram[0].start;
	size[0]  = bd->bi_dram[0].size;

	/* adjust memory start address for LPAE */
	if (lpae) {
		start[0] -= CONFIG_SYS_SDRAM_BASE;
		start[0] += CONFIG_SYS_LPAE_SDRAM_BASE;
	}

	if ((size[0] == 0x80000000) && (ddr3a_size != 0)) {
		size[1] = ((u64)ddr3a_size - 2) << 30;
		start[1] = 0x880000000;
		nbanks++;
	}

	/* reserve memory at start of bank */
	env = env_get("mem_reserve_head");
	if (env) {
		start[0] += ustrtoul(env, &endp, 0);
		size[0] -= ustrtoul(env, &endp, 0);
	}

	env = env_get("mem_reserve");
	if (env)
		size[0] -= ustrtoul(env, &endp, 0);

	fdt_fixup_memory_banks(blob, start, size, nbanks);

	return 0;
}

void ft_board_setup_ex(void *blob, bd_t *bd)
{
	int lpae;
	u64 size;
	char *env;
	u64 *reserve_start;
	int unitrd_fixup = 0;

	env = env_get("mem_lpae");
	lpae = env && simple_strtol(env, NULL, 0);
	env = env_get("uinitrd_fixup");
	unitrd_fixup = env && simple_strtol(env, NULL, 0);

	/* Fix up the initrd */
	if (lpae && unitrd_fixup) {
		int nodeoffset;
		int err;
		u64 *prop1, *prop2;
		u64 initrd_start, initrd_end;

		nodeoffset = fdt_path_offset(blob, "/chosen");
		if (nodeoffset >= 0) {
			prop1 = (u64 *)fdt_getprop(blob, nodeoffset,
					    "linux,initrd-start", NULL);
			prop2 = (u64 *)fdt_getprop(blob, nodeoffset,
					    "linux,initrd-end", NULL);
			if (prop1 && prop2) {
				initrd_start = __be64_to_cpu(*prop1);
				initrd_start -= CONFIG_SYS_SDRAM_BASE;
				initrd_start += CONFIG_SYS_LPAE_SDRAM_BASE;
				initrd_start = __cpu_to_be64(initrd_start);
				initrd_end = __be64_to_cpu(*prop2);
				initrd_end -= CONFIG_SYS_SDRAM_BASE;
				initrd_end += CONFIG_SYS_LPAE_SDRAM_BASE;
				initrd_end = __cpu_to_be64(initrd_end);

				err = fdt_delprop(blob, nodeoffset,
						  "linux,initrd-start");
				if (err < 0)
					puts("error deleting initrd-start\n");

				err = fdt_delprop(blob, nodeoffset,
						  "linux,initrd-end");
				if (err < 0)
					puts("error deleting initrd-end\n");

				err = fdt_setprop(blob, nodeoffset,
						  "linux,initrd-start",
						  &initrd_start,
						  sizeof(initrd_start));
				if (err < 0)
					puts("error adding initrd-start\n");

				err = fdt_setprop(blob, nodeoffset,
						  "linux,initrd-end",
						  &initrd_end,
						  sizeof(initrd_end));
				if (err < 0)
					puts("error adding linux,initrd-end\n");
			}
		}
	}

	if (lpae) {
		/*
		 * the initrd and other reserved memory areas are
		 * embedded in in the DTB itslef. fix up these addresses
		 * to 36 bit format
		 */
		reserve_start = (u64 *)((char *)blob +
				       fdt_off_mem_rsvmap(blob));
		while (1) {
			*reserve_start = __cpu_to_be64(*reserve_start);
			size = __cpu_to_be64(*(reserve_start + 1));
			if (size) {
				*reserve_start -= CONFIG_SYS_SDRAM_BASE;
				*reserve_start +=
					CONFIG_SYS_LPAE_SDRAM_BASE;
				*reserve_start =
					__cpu_to_be64(*reserve_start);
			} else {
				break;
			}
			reserve_start += 2;
		}
	}

	ddr3_check_ecc_int(KS2_DDR3A_EMIF_CTRL_BASE);
}
#endif /* CONFIG_OF_BOARD_SETUP */

#if defined(CONFIG_DTB_RESELECT)
int __weak embedded_dtb_select(void)
{
	return 0;
}
#endif
