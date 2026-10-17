// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <init.h>
#include <log.h>
#include <rtc.h>
#include <acpi/acpi_s3.h>
#include <asm/cmos_layout.h>
#include <asm/early_cmos.h>
#include <asm/global_data.h>
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

void board_final_init(void)
{
	u32 status;

	/* call into FspNotify */
	debug("Calling into FSP (notify phase INIT_PHASE_BOOT): ");
	status = fsp_notify(NULL, INIT_PHASE_BOOT);
	if (status)
		debug("fail, error code %x\n", status);
	else
		debug("OK\n");
}

#if CONFIG_IS_ENABLED(DM_RTC)
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
