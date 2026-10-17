// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <spl.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <mach/tzc.h>
#include <linux/libfdt.h>

u32 spl_boot_device(void)
{
	u32 boot_mode;

	boot_mode = get_bootmode();

	switch (boot_mode) {
	case BOOT_FLASH_SD_1:
	case BOOT_FLASH_EMMC_1:
		return BOOT_DEVICE_MMC1;
	case BOOT_FLASH_SD_2:
	case BOOT_FLASH_EMMC_2:
		return BOOT_DEVICE_MMC2;
	case BOOT_SERIAL_UART_1:
	case BOOT_SERIAL_UART_2:
	case BOOT_SERIAL_UART_3:
	case BOOT_SERIAL_UART_4:
	case BOOT_SERIAL_UART_5:
	case BOOT_SERIAL_UART_6:
	case BOOT_SERIAL_UART_7:
	case BOOT_SERIAL_UART_8:
		return BOOT_DEVICE_UART;
	case BOOT_SERIAL_USB_OTG:
		return BOOT_DEVICE_DFU;
	case BOOT_FLASH_NAND_FMC:
		return BOOT_DEVICE_NAND;
	case BOOT_FLASH_NOR_QSPI:
		return BOOT_DEVICE_SPI;
	case BOOT_FLASH_SPINAND_1:
		return BOOT_DEVICE_NONE; /* SPINAND not supported in SPL */
	}

	return BOOT_DEVICE_MMC1;
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	return MMCSD_MODE_RAW;
}

#ifdef CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_PARTITION
int spl_mmc_boot_partition(const u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION;
	case BOOT_DEVICE_MMC2:
		return CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION_MMC2;
	default:
		return -EINVAL;
	}
}
#endif

#ifdef CONFIG_SPL_DISPLAY_PRINT
void spl_display_print(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	const char *model;

	/* same code than show_board_info() but not compiled for SPL
	 * see CONFIG_DISPLAY_BOARDINFO & common/board_info.c
	 */
	model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
	if (model)
		log_info("Model: %s\n", model);
}
#endif

__weak int board_early_init_f(void)
{
	return 0;
}

uint32_t stm32mp_get_dram_size(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	if (uclass_get_device(UCLASS_RAM, 0, &dev))
		return 0;

	ret = ram_get_info(dev, &ram);
	if (ret)
		return 0;

	return ram.size;
}

static int optee_get_reserved_memory(uint32_t *start, uint32_t *size)
{
	fdt_addr_t fdt_mem_size;
	fdt_addr_t fdt_start;
	ofnode node;

	node = ofnode_path("/reserved-memory/optee");
	if (!ofnode_valid(node))
		return 0;

	fdt_start = ofnode_get_addr_size(node, "reg", &fdt_mem_size);
	*start = fdt_start;
	*size = fdt_mem_size;
	return (fdt_start < 0) ? fdt_start : 0;
}

#define CFG_SHMEM_SIZE			0x200000
#define STM32_TZC_NSID_ALL		0xffff
#define STM32_TZC_FILTER_ALL		3

void stm32_init_tzc_for_optee(void)
{
	const uint32_t dram_size = stm32mp_get_dram_size();
	const uintptr_t dram_top = STM32_DDR_BASE + (dram_size - 1);
	uint32_t optee_base, optee_size, tee_shmem_base;
	const uintptr_t tzc = STM32_TZC_BASE;
	int ret;

	if (dram_size == 0)
		panic("Cannot determine DRAM size from devicetree\n");

	ret = optee_get_reserved_memory(&optee_base, &optee_size);
	if (ret < 0 || optee_size <= CFG_SHMEM_SIZE)
		panic("Invalid OPTEE reserved memory in devicetree\n");

	tee_shmem_base = optee_base + optee_size - CFG_SHMEM_SIZE;

	const struct tzc_region optee_config[] = {
		{
			.base = STM32_DDR_BASE,
			.top = optee_base - 1,
			.sec_mode = TZC_ATTR_SEC_NONE,
			.nsec_id = STM32_TZC_NSID_ALL,
			.filters_mask = STM32_TZC_FILTER_ALL,
		}, {
			.base = optee_base,
			.top = tee_shmem_base - 1,
			.sec_mode = TZC_ATTR_SEC_RW,
			.nsec_id = 0,
			.filters_mask = STM32_TZC_FILTER_ALL,
		}, {
			.base = tee_shmem_base,
			.top = dram_top,
			.sec_mode = TZC_ATTR_SEC_NONE,
			.nsec_id = STM32_TZC_NSID_ALL,
			.filters_mask = STM32_TZC_FILTER_ALL,
		}, {
			.top = 0,
		}
	};

	flush_dcache_all();

	tzc_configure(tzc, optee_config);
	tzc_dump_config(tzc);

	dcache_disable();
}

void spl_board_prepare_for_optee(void *fdt)
{
	stm32_init_tzc_for_optee();
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	arch_cpu_init();
	mach_cpu_init();

	ret = spl_early_init();
	if (ret) {
		log_debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_CLK, 0, &dev);
	if (ret) {
		log_debug("Clock init failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_RESET, 0, &dev);
	if (ret) {
		log_debug("Reset init failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_PINCTRL, 0, &dev);
	if (ret) {
		log_debug("%s: Cannot find pinctrl device\n", __func__);
		hang();
	}

	/* enable console uart printing */
	preloader_console_init();

	ret = board_early_init_f();
	if (ret) {
		log_debug("board_early_init_f() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		log_err("DRAM init failed: %d\n", ret);
		hang();
	}

	/*
	 * activate cache on DDR only when DDR is fully initialized
	 * to avoid speculative access and issue in get_ram_size()
	 */
	if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		mmu_set_region_dcache_behaviour(STM32_DDR_BASE,
						CONFIG_DDR_CACHEABLE_SIZE,
						DCACHE_DEFAULT_OPTION);
}

void spl_board_prepare_for_boot(void)
{
	dcache_disable();
}

void spl_board_prepare_for_linux(void)
{
	dcache_disable();
}
