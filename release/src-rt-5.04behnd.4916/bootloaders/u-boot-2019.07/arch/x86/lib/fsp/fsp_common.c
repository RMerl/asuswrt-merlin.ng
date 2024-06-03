// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <rtc.h>
#include <asm/acpi_s3.h>
#include <asm/cmos_layout.h>
#include <asm/early_cmos.h>
#include <asm/io.h>
#include <asm/mrccache.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/fsp/fsp_support.h>

DECLARE_GLOBAL_DATA_PTR;

int checkcpu(void)
{
	return 0;
}

int print_cpuinfo(void)
{
	post_code(POST_CPU_INFO);
	return default_print_cpuinfo();
}

int fsp_init_phase_pci(void)
{
	u32 status;

	/* call into FspNotify */
	debug("Calling into FSP (notify phase INIT_PHASE_PCI): ");
	status = fsp_notify(NULL, INIT_PHASE_PCI);
	if (status)
		debug("fail, error code %x\n", status);
	else
		debug("OK\n");

	return status ? -EPERM : 0;
}

void board_final_cleanup(void)
{
	u32 status;

	/* call into FspNotify */
	debug("Calling into FSP (notify phase INIT_PHASE_BOOT): ");
	status = fsp_notify(NULL, INIT_PHASE_BOOT);
	if (status)
		debug("fail, error code %x\n", status);
	else
		debug("OK\n");

	return;
}

static __maybe_unused void *fsp_prepare_mrc_cache(void)
{
	struct mrc_data_container *cache;
	struct mrc_region entry;
	int ret;

	ret = mrccache_get_region(NULL, &entry);
	if (ret)
		return NULL;

	cache = mrccache_find_current(&entry);
	if (!cache)
		return NULL;

	debug("%s: mrc cache at %p, size %x checksum %04x\n", __func__,
	      cache->data, cache->data_size, cache->checksum);

	return cache->data;
}

#ifdef CONFIG_HAVE_ACPI_RESUME
int fsp_save_s3_stack(void)
{
	struct udevice *dev;
	int ret;

	if (gd->arch.prev_sleep_state == ACPI_S3)
		return 0;

	ret = uclass_get_device(UCLASS_RTC, 0, &dev);
	if (ret) {
		debug("Cannot find RTC: err=%d\n", ret);
		return -ENODEV;
	}

	/* Save the stack address to CMOS */
	ret = rtc_write32(dev, CMOS_FSP_STACK_ADDR, gd->start_addr_sp);
	if (ret) {
		debug("Save stack address to CMOS: err=%d\n", ret);
		return -EIO;
	}

	return 0;
}
#endif

int arch_fsp_init(void)
{
	void *nvs;
	int stack = CONFIG_FSP_TEMP_RAM_ADDR;
	int boot_mode = BOOT_FULL_CONFIG;
#ifdef CONFIG_HAVE_ACPI_RESUME
	int prev_sleep_state = chipset_prev_sleep_state();
	gd->arch.prev_sleep_state = prev_sleep_state;
#endif

	if (!gd->arch.hob_list) {
#ifdef CONFIG_ENABLE_MRC_CACHE
		nvs = fsp_prepare_mrc_cache();
#else
		nvs = NULL;
#endif

#ifdef CONFIG_HAVE_ACPI_RESUME
		if (prev_sleep_state == ACPI_S3) {
			if (nvs == NULL) {
				/* If waking from S3 and no cache then */
				debug("No MRC cache found in S3 resume path\n");
				post_code(POST_RESUME_FAILURE);
				/* Clear Sleep Type */
				chipset_clear_sleep_state();
				/* Reboot */
				debug("Rebooting..\n");
				outb(SYS_RST | RST_CPU, IO_PORT_RESET);
				/* Should not reach here.. */
				panic("Reboot System");
			}

			/*
			 * DM is not available yet at this point, hence call
			 * CMOS access library which does not depend on DM.
			 */
			stack = cmos_read32(CMOS_FSP_STACK_ADDR);
			boot_mode = BOOT_ON_S3_RESUME;
		}
#endif
		/*
		 * The first time we enter here, call fsp_init().
		 * Note the execution does not return to this function,
		 * instead it jumps to fsp_continue().
		 */
		fsp_init(stack, boot_mode, nvs);
	} else {
		/*
		 * The second time we enter here, adjust the size of malloc()
		 * pool before relocation. Given gd->malloc_base was adjusted
		 * after the call to board_init_f_init_reserve() in arch/x86/
		 * cpu/start.S, we should fix up gd->malloc_limit here.
		 */
		gd->malloc_limit += CONFIG_FSP_SYS_MALLOC_F_LEN;
	}

	return 0;
}
