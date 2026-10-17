// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <efi.h>
#include <efi_loader.h>
#include <errno.h>
#include <fsl_ddr.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/kernel.h>
#include <env_internal.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <i2c.h>
#include <asm/arch/soc.h>
#include <fsl_immap.h>
#include <netdev.h>
#include <wdt.h>

#include <sl28cpld.h>
#include <fdtdec.h>
#include <miiphy.h>

#include "sl28.h"

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_EFI_HAVE_CAPSULE_SUPPORT)
struct efi_fw_image fw_images[] = {
	{
		.image_type_id = KONTRON_SL28_FIT_IMAGE_GUID,
		.fw_name = u"KONTRON-SL28-FIT",
		.image_index = 1,
	},
};

struct efi_capsule_update_info update_info = {
	.dfu_string = "sf 0:0=u-boot-bin raw 0x210000 0x1d0000;"
			"u-boot-env raw 0x3e0000 0x20000",
	.num_images = ARRAY_SIZE(fw_images),
	.images = fw_images,
};

#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_early_init_f(void)
{
	fsl_lsch3_early_init_f();
	return 0;
}

int board_init(void)
{
	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return pci_eth_init(bis);
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	enum boot_source src = sl28_boot_source();

	if (prio)
		return ENVL_UNKNOWN;

	if (!CONFIG_IS_ENABLED(ENV_IS_IN_SPI_FLASH))
		return ENVL_NOWHERE;

	/* write and erase always operate on the environment */
	if (op == ENVOP_SAVE || op == ENVOP_ERASE)
		return ENVL_SPI_FLASH;

	/* failsafe boot will always use the compiled-in default environment */
	if (src == BOOT_SOURCE_SPI)
		return ENVL_NOWHERE;

	return ENVL_SPI_FLASH;
}

static int __sl28cpld_read(uint reg)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_NOP,
					  DM_DRIVER_GET(sl28cpld), &dev);
	if (ret)
		return ret;

	return sl28cpld_read(dev, reg);
}

static void print_cpld_version(void)
{
	int version = __sl28cpld_read(SL28CPLD_VERSION);

	if (version < 0)
		printf("CPLD:  error reading version (%d)\n", version);
	else
		printf("CPLD:  v%d\n", version);
}

int checkboard(void)
{
	printf("EL:    %d\n", current_el());
	if (IS_ENABLED(CONFIG_SL28CPLD))
		print_cpld_version();

	return 0;
}

static void stop_recovery_watchdog(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_WDT,
					  DM_DRIVER_GET(sl28cpld_wdt), &dev);
	if (!ret)
		wdt_stop(dev);
}

static void sl28_set_prompt(void)
{
	enum boot_source src = sl28_boot_source();

	switch (src) {
	case BOOT_SOURCE_SPI:
		env_set("PS1", "[FAILSAFE] => ");
		break;
	case BOOT_SOURCE_SDHC:
		env_set("PS1", "[SDHC] => ");
		break;
	default:
		env_set("PS1", NULL);
		break;
	}
}

int fsl_board_late_init(void)
{
	if (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT))
		sl28_set_prompt();

	/*
	 * Usually, the after a board reset, the watchdog is enabled by
	 * default. This is to supervise the bootloader boot-up. Therefore,
	 * to prevent a watchdog reset if we don't actively kick it, we have
	 * to disable it.
	 *
	 * If the watchdog isn't enabled at reset (which is a configuration
	 * option) disabling it doesn't hurt either.
	 */
	if (!IS_ENABLED(CONFIG_WATCHDOG_AUTOSTART))
		stop_recovery_watchdog();

	return 0;
}

void detail_board_ddr_info(void)
{
	print_ddr_info(0);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int nbanks = CONFIG_NR_DRAM_BANKS;
	int node;
	int i;

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
	for (i = 0; i < nbanks; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

	fdt_fixup_memory_banks(blob, base, size, nbanks);

	fdt_fixup_icid(blob);

	if (IS_ENABLED(CONFIG_SL28_SPL_LOADS_OPTEE_BL32)) {
		node = fdt_node_offset_by_compatible(blob, -1, "linaro,optee-tz");
		if (node)
			fdt_set_node_status(blob, node, FDT_STATUS_OKAY);
	}

	return 0;
}
