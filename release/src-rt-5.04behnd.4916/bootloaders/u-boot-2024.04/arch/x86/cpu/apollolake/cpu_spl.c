// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 *
 * Portions taken from coreboot
 */

#include <common.h>
#include <dm.h>
#include <ec_commands.h>
#include <init.h>
#include <log.h>
#include <spi_flash.h>
#include <spl.h>
#include <syscon.h>
#include <acpi/acpi_s3.h>
#include <asm/cpu.h>
#include <asm/cpu_common.h>
#include <asm/cpu_x86.h>
#include <asm/fast_spi.h>
#include <asm/global_data.h>
#include <asm/intel_pinctrl.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/iomap.h>
#include <asm/arch/lpc.h>
#include <asm/arch/pch.h>
#include <asm/arch/systemagent.h>
#include <asm/fsp2/fsp_api.h>
#include <linux/sizes.h>
#include <power/acpi_pmc.h>

static int fast_spi_cache_bios_region(void)
{
	uint map_size, offset;
	ulong map_base, base;
	int ret;

	ret = fast_spi_early_init(PCH_DEV_SPI, IOMAP_SPI_BASE);
	if (ret)
		return log_msg_ret("early_init", ret);

	ret = fast_spi_get_bios_mmap(PCH_DEV_SPI, &map_base, &map_size,
				     &offset);
	if (ret)
		return log_msg_ret("get_mmap", ret);

	base = SZ_4G - map_size;
	mtrr_set_next_var(MTRR_TYPE_WRPROT, base, map_size);
	log_debug("BIOS cache base=%lx, size=%x\n", base, (uint)map_size);

	return 0;
}

static void google_chromeec_ioport_range(uint *out_basep, uint *out_sizep)
{
	uint base;
	uint size;

	if (IS_ENABLED(CONFIG_EC_GOOGLE_CHROMEEC_MEC)) {
		base = MEC_EMI_BASE;
		size = MEC_EMI_SIZE;
	} else {
		base = EC_HOST_CMD_REGION0;
		size = 2 * EC_HOST_CMD_REGION_SIZE;
		/* Make sure MEMMAP region follows host cmd region */
		assert(base + size == EC_LPC_ADDR_MEMMAP);
		size += EC_MEMMAP_SIZE;
	}

	*out_basep = base;
	*out_sizep = size;
}

static void early_ec_init(void)
{
	uint base, size;

	/*
	 * Set up LPC decoding for the Chrome OS EC I/O port ranges:
	 * - Ports 62/66, 60/64, and 200->208
	 * - Chrome OS EC communication I/O ports
	 */
	lpc_enable_fixed_io_ranges(LPC_IOE_EC_62_66 | LPC_IOE_KBC_60_64 |
				   LPC_IOE_LGE_200);
	google_chromeec_ioport_range(&base, &size);
	lpc_open_pmio_window(base, size);
}

static int arch_cpu_init_tpl(void)
{
	struct udevice *pmc, *sa, *p2sb, *serial, *spi, *lpc;
	int ret;

	ret = uclass_first_device_err(UCLASS_ACPI_PMC, &pmc);
	if (ret)
		return log_msg_ret("PMC", ret);

	/* Clear global reset promotion bit */
	ret = pmc_global_reset_set_enable(pmc, false);
	if (ret)
		return log_msg_ret("disable global reset", ret);

	enable_pm_timer_emulation(pmc);

	ret = uclass_first_device_err(UCLASS_P2SB, &p2sb);
	if (ret)
		return log_msg_ret("p2sb", ret);
	ret = uclass_first_device_err(UCLASS_NORTHBRIDGE, &sa);
	if (ret)
		return log_msg_ret("northbridge", ret);
	gd->baudrate = CONFIG_BAUDRATE;
	ret = uclass_first_device_err(UCLASS_SERIAL, &serial);
	if (ret)
		return log_msg_ret("serial", ret);
	if (CONFIG_IS_ENABLED(SPI_FLASH_SUPPORT)) {
		ret = uclass_first_device_err(UCLASS_SPI, &spi);
		if (ret)
			return log_msg_ret("SPI", ret);
	} else {
		/* Alternative code if we don't have SPI in TPL */
		if (IS_ENABLED(CONFIG_APL_BOOT_FROM_FAST_SPI_FLASH))
			printf("Warning: Enable APL_SPI_FLASHBOOT to use SPI-flash driver in TPL");
		ret = fast_spi_cache_bios_region();
		if (ret)
			return log_msg_ret("BIOS cache", ret);
	}
	ret = pmc_disable_tco(pmc);
	if (ret)
		return log_msg_ret("disable TCO", ret);
	ret = pmc_gpe_init(pmc);
	if (ret)
		return log_msg_ret("pmc_gpe", ret);
	ret = uclass_first_device_err(UCLASS_LPC, &lpc);
	if (ret)
		return log_msg_ret("lpc", ret);

	early_ec_init();

	return 0;
}

/*
 * Enables several BARs and devices which are needed for memory init
 * - MCH_BASE_ADDR is needed in order to talk to the memory controller
 * - HPET is enabled because FSP wants to store a pointer to global data in the
 *   HPET comparator register
 */
static int arch_cpu_init_spl(void)
{
	struct udevice *pmc, *p2sb;
	int ret;

	ret = uclass_first_device_err(UCLASS_ACPI_PMC, &pmc);
	if (ret)
		return log_msg_ret("Could not probe PMC", ret);
	ret = uclass_first_device_err(UCLASS_P2SB, &p2sb);
	if (ret)
		return log_msg_ret("Cannot set up p2sb", ret);

	lpc_io_setup_comm_a_b();

	/* TODO(sjg@chromium.org): Enable upper RTC bank here */

	ret = pmc_init(pmc);
	if (ret < 0)
		return log_msg_ret("Could not init PMC", ret);
	if (IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)) {
		ret = pmc_prev_sleep_state(pmc);
		if (ret < 0)
			return log_msg_ret("Could not get PMC sleep state",
					   ret);
		gd->arch.prev_sleep_state = ret;
	}

	return 0;
}

int arch_cpu_init(void)
{
	int ret = 0;

	if (spl_phase() == PHASE_TPL)
		ret = arch_cpu_init_tpl();
	else if (spl_phase() == PHASE_SPL)
		ret = arch_cpu_init_spl();
	if (ret)
		printf("%s: Error %d\n", __func__, ret);

	return ret;
}
