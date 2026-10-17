// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 Google, Inc
 */

#define LOG_CATEGORY	LOGC_BOOT

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/cpu.h>
#include <asm/global_data.h>
#include <asm/mtrr.h>
#include <asm/processor.h>
#include <asm-generic/sections.h>

DECLARE_GLOBAL_DATA_PTR;

static int x86_tpl_init(void)
{
	int ret;

	debug("%s starting\n", __func__);
	ret = x86_cpu_init_tpl();
	if (ret) {
		debug("%s: x86_cpu_init_tpl() failed\n", __func__);
		return ret;
	}
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
	preloader_console_init();

	return 0;
}

void board_init_f(ulong flags)
{
	int ret;

	ret = x86_tpl_init();
	if (ret) {
		debug("Error %d\n", ret);
		panic("x86_tpl_init fail");
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
	return IS_ENABLED(CONFIG_CHROMEOS_VBOOT) ? BOOT_DEVICE_CROS_VBOOT :
		BOOT_DEVICE_SPI_MMAP;
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
SPL_LOAD_IMAGE_METHOD("SPI", 5, BOOT_DEVICE_SPI_MMAP, spl_board_load_image);

int spl_spi_load_image(void)
{
	return -EPERM;
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	debug("Jumping to %s at %lx\n", spl_phase_name(spl_next_phase()),
	      (ulong)spl_image->entry_point);
#ifdef DEBUG
	print_buffer(spl_image->entry_point, (void *)spl_image->entry_point, 1,
		     0x20, 0);
#endif
	jump_to_spl(spl_image->entry_point);
	hang();
}

void spl_board_init(void)
{
	preloader_console_init();
}

#if !CONFIG_IS_ENABLED(PCI)
/*
 * This is a fake PCI bus for TPL when it doesn't have proper PCI. It is enough
 * to bind the devices on the PCI bus, some of which have early-regs properties
 * providing fixed BARs. Individual drivers program these BARs themselves so
 * that they can access the devices. The BARs are allocated statically in the
 * device tree.
 *
 * Once SPL is running it enables PCI properly, but does not auto-assign BARs
 * for devices, so the TPL BARs continue to be used. Once U-Boot starts it does
 * the auto allocation (after relocation).
 */
#if CONFIG_IS_ENABLED(OF_REAL)
static const struct udevice_id tpl_fake_pci_ids[] = {
	{ .compatible = "pci-x86" },
	{ }
};
#endif

U_BOOT_DRIVER(pci_x86) = {
	.name	= "pci_x86",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = of_match_ptr(tpl_fake_pci_ids),
	DM_PHASE(tpl)
};
#endif
