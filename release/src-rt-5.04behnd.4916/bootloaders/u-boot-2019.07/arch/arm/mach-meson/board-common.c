// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 */

#include <common.h>
#include <asm/arch/boot.h>
#include <linux/libfdt.h>
#include <linux/err.h>
#include <asm/arch/mem.h>
#include <asm/arch/sm.h>
#include <asm/armv8/mmu.h>
#include <asm/unaligned.h>
#include <efi_loader.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	const fdt64_t *val;
	int offset;
	int len;

	offset = fdt_path_offset(gd->fdt_blob, "/memory");
	if (offset < 0)
		return -EINVAL;

	val = fdt_getprop(gd->fdt_blob, offset, "reg", &len);
	if (len < sizeof(*val) * 2)
		return -EINVAL;

	/* Use unaligned access since cache is still disabled */
	gd->ram_size = get_unaligned_be64(&val[1]);

	return 0;
}

__weak int meson_ft_board_setup(void *blob, bd_t *bd)
{
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	meson_init_reserved_memory(blob);

	return meson_ft_board_setup(blob, bd);
}

void meson_board_add_reserved_memory(void *fdt, u64 start, u64 size)
{
	int ret;

	ret = fdt_add_mem_rsv(fdt, start, size);
	if (ret)
		printf("Could not reserve zone @ 0x%llx\n", start);

	if (IS_ENABLED(CONFIG_EFI_LOADER)) {
		efi_add_memory_map(start,
				   ALIGN(size, EFI_PAGE_SIZE) >> EFI_PAGE_SHIFT,
				   EFI_RESERVED_MEMORY_TYPE, false);
	}
}

static void meson_set_boot_source(void)
{
	const char *source;

	switch (meson_get_boot_device()) {
	case BOOT_DEVICE_EMMC:
		source = "emmc";
		break;

	case BOOT_DEVICE_NAND:
		source = "nand";
		break;

	case BOOT_DEVICE_SPI:
		source = "spi";
		break;

	case BOOT_DEVICE_SD:
		source = "sd";
		break;

	case BOOT_DEVICE_USB:
		source = "usb";
		break;

	default:
		source = "unknown";
	}

	env_set("boot_source", source);
}

__weak int meson_board_late_init(void)
{
	return 0;
}

int board_late_init(void)
{
	meson_set_boot_source();

	return meson_board_late_init();
}

void reset_cpu(ulong addr)
{
	psci_system_reset();
}
