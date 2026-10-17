// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <addr_map.h>
#include <config.h>
#include <cpu_func.h>
#include <cros_ec.h>
#include <dm.h>
#include <efi.h>
#include <efi_loader.h>
#include <env_internal.h>
#include <extension_board.h>
#include <init.h>
#include <led.h>
#include <malloc.h>
#include <mapmem.h>
#include <os.h>
#include <acpi/acpi_table.h>
#include <asm/global_data.h>
#include <asm/test.h>
#include <asm/u-boot-sandbox.h>
#include <linux/kernel.h>
#include <linux/sizes.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it.
 */
gd_t *gd;

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
/* GUIDs for capsule updatable firmware images */
#define SANDBOX_UBOOT_IMAGE_GUID \
	EFI_GUID(0x09d7cf52, 0x0720, 0x4710, 0x91, 0xd1, \
		 0x08, 0x46, 0x9b, 0x7f, 0xe9, 0xc8)

#define SANDBOX_UBOOT_ENV_IMAGE_GUID \
	EFI_GUID(0x5a7021f5, 0xfef2, 0x48b4, 0xaa, 0xba, \
		 0x83, 0x2e, 0x77, 0x74, 0x18, 0xc0)

#define SANDBOX_FIT_IMAGE_GUID \
	EFI_GUID(0x3673b45d, 0x6a7c, 0x46f3, 0x9e, 0x60, \
		 0xad, 0xab, 0xb0, 0x3f, 0x79, 0x37)

struct efi_fw_image fw_images[] = {
#if defined(CONFIG_EFI_CAPSULE_FIRMWARE_RAW)
	{
		.image_type_id = SANDBOX_UBOOT_IMAGE_GUID,
		.fw_name = u"SANDBOX-UBOOT",
		.image_index = 1,
	},
	{
		.image_type_id = SANDBOX_UBOOT_ENV_IMAGE_GUID,
		.fw_name = u"SANDBOX-UBOOT-ENV",
		.image_index = 2,
	},
#elif defined(CONFIG_EFI_CAPSULE_FIRMWARE_FIT)
	{
		.image_type_id = SANDBOX_FIT_IMAGE_GUID,
		.fw_name = u"SANDBOX-FIT",
		.image_index = 1,
	},
#endif
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "sf 0:0=u-boot-bin raw 0x100000 0x50000;"
		"u-boot-env raw 0x150000 0x200000",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

#if !CONFIG_IS_ENABLED(OF_PLATDATA)
/*
 * Add a simple GPIO device (don't use with of-platdata as it interferes with
 * the auto-generated devices)
 */
U_BOOT_DRVINFO(gpio_sandbox) = {
	.name = "sandbox_gpio",
};
#endif

#ifndef CONFIG_TIMER
/* system timer offset in ms */
static unsigned long sandbox_timer_offset;

void timer_test_add_offset(unsigned long offset)
{
	sandbox_timer_offset += offset;
}

unsigned long timer_read_counter(void)
{
	return os_get_nsec() / 1000 + sandbox_timer_offset * 1000;
}
#endif

/* specific order for sandbox: nowhere is the first value, used by default */
static enum env_location env_locations[] = {
	ENVL_NOWHERE,
	ENVL_EXT4,
	ENVL_FAT,
};

enum env_location env_get_location(enum env_operation op, int prio)
{
	if (prio >= ARRAY_SIZE(env_locations))
		return ENVL_UNKNOWN;

	return env_locations[prio];
}

int dram_init(void)
{
	gd->ram_size = CFG_SYS_SDRAM_SIZE;
	return 0;
}

int board_init(void)
{
	return 0;
}

int ft_board_setup(void *fdt, struct bd_info *bd)
{
	/* Create an arbitrary reservation to allow testing OF_BOARD_SETUP.*/
	return fdt_add_mem_rsv(fdt, 0x00d02000, 0x4000);
}

#ifdef CONFIG_CMD_EXTENSION
int extension_board_scan(struct list_head *extension_list)
{
	struct extension *extension;
	int i;

	for (i = 0; i < 2; i++) {
		extension = calloc(1, sizeof(struct extension));
		snprintf(extension->overlay, sizeof(extension->overlay), "overlay%d.dtbo", i);
		snprintf(extension->name, sizeof(extension->name), "extension board %d", i);
		snprintf(extension->owner, sizeof(extension->owner), "sandbox");
		snprintf(extension->version, sizeof(extension->version), "1.1");
		snprintf(extension->other, sizeof(extension->other), "Fictional extension board");
		list_add_tail(&extension->list, extension_list);
	}

	return i;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	struct udevice *dev;
	ulong addr, end;
	void *ptr;
	int ret;

	ret = uclass_first_device_err(UCLASS_CROS_EC, &dev);
	if (ret && ret != -ENODEV) {
		/* Force console on */
		gd->flags &= ~GD_FLG_SILENT;

		printf("cros-ec communications failure %d\n", ret);
		puts("\nPlease reset with Power+Refresh\n\n");
		panic("Cannot init cros-ec device");
		return -1;
	}

	if (IS_ENABLED(CONFIG_GENERATE_ACPI_TABLE)) {
		/* Reserve 64K for ACPI tables, aligned to a 4K boundary */
		ptr = memalign(SZ_4K, SZ_64K);
		addr = map_to_sysmem(ptr);

		/* Generate ACPI tables */
		end = write_acpi_tables(addr);
		gd->arch.table_start = addr;
		gd->arch.table_end = addr;
	}

	return 0;
}
#endif

int init_addr_map(void)
{
	if (IS_ENABLED(CONFIG_ADDR_MAP))
		addrmap_set_entry(0, 0, CFG_SYS_SDRAM_SIZE, 0);

	return 0;
}

#if defined(CONFIG_FWU_MULTI_BANK_UPDATE)
void fwu_plat_get_bootidx(uint *boot_idx)
{
	/* Dummy value */
	*boot_idx = 0;
}
#endif
