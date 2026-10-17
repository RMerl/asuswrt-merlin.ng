// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_ARCH

#include <common.h>
#include <handoff.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <acpi/acpi_s3.h>
#include <asm/arch/cpu.h>
#include <asm/fsp/fsp_support.h>
#include <asm/fsp2/fsp_api.h>
#include <asm/fsp2/fsp_internal.h>
#include <asm/global_data.h>
#include <linux/sizes.h>

int dram_init(void)
{
	int ret;

	if (!ll_boot_init()) {
		/* Use a small and safe amount of 1GB */
		gd->ram_size = SZ_1G;

		return 0;
	}
	if (spl_phase() == PHASE_SPL) {
		bool s3wake = false;

		s3wake = IS_ENABLED(CONFIG_HAVE_ACPI_RESUME) &&
			gd->arch.prev_sleep_state == ACPI_S3;

		ret = fsp_memory_init(s3wake,
			      IS_ENABLED(CONFIG_APL_BOOT_FROM_FAST_SPI_FLASH));
		if (ret) {
			log_debug("Memory init failed (err=%x)\n", ret);
			return ret;
		}

		/* The FSP has already set up DRAM, so grab the info we need */
		ret = fsp_scan_for_ram_size();
		if (ret)
			return ret;

#ifdef CONFIG_ENABLE_MRC_CACHE
		gd->arch.mrc[MRC_TYPE_NORMAL].buf =
			fsp_get_nvs_data(gd->arch.hob_list,
					 &gd->arch.mrc[MRC_TYPE_NORMAL].len);
		gd->arch.mrc[MRC_TYPE_VAR].buf =
			fsp_get_var_nvs_data(gd->arch.hob_list,
					     &gd->arch.mrc[MRC_TYPE_VAR].len);
		log_debug("normal %x, var %x\n",
			  gd->arch.mrc[MRC_TYPE_NORMAL].len,
			  gd->arch.mrc[MRC_TYPE_VAR].len);
#endif
	} else {
#if CONFIG_IS_ENABLED(HANDOFF)
		struct spl_handoff *ho = gd->spl_handoff;

		if (!ho) {
			log_debug("No SPL handoff found\n");
			return -ESTRPIPE;
		}
		gd->ram_size = ho->ram_size;
		handoff_load_dram_banks(ho);
#endif
		ret = arch_fsps_preinit();
		if (ret)
			return log_msg_ret("fsp_s_preinit", ret);
	}

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	if (!ll_boot_init())
		return gd->ram_size;

#if CONFIG_IS_ENABLED(HANDOFF)
	struct spl_handoff *ho = gd->spl_handoff;

	log_debug("usable_ram_top = %lx\n", ho->arch.usable_ram_top);

	return ho->arch.usable_ram_top;
#endif

	return gd->ram_top;
}
