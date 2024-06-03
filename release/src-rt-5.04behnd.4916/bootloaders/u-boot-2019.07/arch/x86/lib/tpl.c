// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Google, Inc
 */

#include <common.h>
#include <debug_uart.h>
#include <spl.h>
#include <asm/cpu.h>
#include <asm/mtrr.h>
#include <asm/processor.h>
#include <asm-generic/sections.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int arch_cpu_init_dm(void)
{
	return 0;
}

static int x86_tpl_init(void)
{
	int ret;

	debug("%s starting\n", __func__);
	ret = spl_init();
	if (ret) {
		debug("%s: spl_init() failed\n", __func__);
		return ret;
	}
	ret = arch_cpu_init();
	if (ret) {
		debug("%s: arch_cpu_init() failed\n", __func__);
		return ret;
	}
	ret = arch_cpu_init_dm();
	if (ret) {
		debug("%s: arch_cpu_init_dm() failed\n", __func__);
		return ret;
	}
	preloader_console_init();
	ret = print_cpuinfo();
	if (ret) {
		debug("%s: print_cpuinfo() failed\n", __func__);
		return ret;
	}

	return 0;
}

void board_init_f(ulong flags)
{
	int ret;

	ret = x86_tpl_init();
	if (ret) {
		debug("Error %d\n", ret);
		hang();
	}

	/* Uninit CAR and jump to board_init_f_r() */
	board_init_r(gd, 0);
}

void board_init_f_r(void)
{
	/* Not used since we never call board_init_f_r_trampoline() */
	while (1);
}

u32 spl_boot_device(void)
{
	return IS_ENABLED(CONFIG_CHROMEOS) ? BOOT_DEVICE_CROS_VBOOT :
		BOOT_DEVICE_BOARD;
}

int spl_start_uboot(void)
{
	return 0;
}

void spl_board_announce_boot_device(void)
{
	printf("SPI flash");
}

static int spl_board_load_image(struct spl_image_info *spl_image,
				struct spl_boot_device *bootdev)
{
	spl_image->size = CONFIG_SYS_MONITOR_LEN;  /* We don't know SPL size */
	spl_image->entry_point = CONFIG_SPL_TEXT_BASE;
	spl_image->load_addr = CONFIG_SPL_TEXT_BASE;
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";

	debug("Loading to %lx\n", spl_image->load_addr);

	return 0;
}
SPL_LOAD_IMAGE_METHOD("SPI", 0, BOOT_DEVICE_BOARD, spl_board_load_image);

int spl_spi_load_image(void)
{
	return -EPERM;
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	printf("Jumping to U-Boot SPL at %lx\n", (ulong)spl_image->entry_point);
	jump_to_spl(spl_image->entry_point);
	while (1)
		;
}

void spl_board_init(void)
{
	preloader_console_init();
}
