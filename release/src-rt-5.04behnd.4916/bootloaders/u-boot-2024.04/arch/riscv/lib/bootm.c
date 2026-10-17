// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <bootstage.h>
#include <bootm.h>
#include <command.h>
#include <dm.h>
#include <fdt_support.h>
#include <hang.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/root.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/csr.h>
#include <asm/smp.h>
#include <dm/device.h>
#include <dm/root.h>
#include <u-boot/zlib.h>

DECLARE_GLOBAL_DATA_PTR;

__weak void board_quiesce_devices(void)
{
}

/**
 * announce_and_cleanup() - Print message and prepare for kernel boot
 *
 * @fake: non-zero to do everything except actually boot
 */
static void announce_and_cleanup(int fake)
{
	printf("\nStarting kernel ...%s\n\n", fake ?
		"(fake run for tracing)" : "");
	bootstage_mark_name(BOOTSTAGE_ID_BOOTM_HANDOFF, "start_kernel");
#ifdef CONFIG_BOOTSTAGE_FDT
	bootstage_fdt_add_report();
#endif
#if CONFIG_IS_ENABLED(BOOTSTAGE_REPORT)
	bootstage_report();
#endif

#ifdef CONFIG_USB_DEVICE
	udc_disconnect();
#endif

	board_quiesce_devices();

	/*
	 * Call remove function of all devices with a removal flag set.
	 * This may be useful for last-stage operations, like cancelling
	 * of DMA operation or releasing device internal buffers.
	 */
	dm_remove_devices_flags(DM_REMOVE_ACTIVE_ALL);

	cleanup_before_linux();
}

static void boot_prep_linux(struct bootm_headers *images)
{
	if (CONFIG_IS_ENABLED(OF_LIBFDT) && IS_ENABLED(CONFIG_LMB) && images->ft_len) {
		debug("using: FDT\n");
		if (image_setup_linux(images)) {
			printf("FDT creation failed! hanging...");
			hang();
		}
	} else {
		printf("Device tree not found or missing FDT support\n");
		hang();
	}
}

static void boot_jump_linux(struct bootm_headers *images, int flag)
{
	void (*kernel)(ulong hart, void *dtb);
	int fake = (flag & BOOTM_STATE_OS_FAKE_GO);
#ifdef CONFIG_SMP
	int ret;
#endif

	kernel = (void (*)(ulong, void *))images->ep;

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	debug("## Transferring control to kernel (at address %08lx) ...\n",
	      (ulong)kernel);

	announce_and_cleanup(fake);

	if (!fake) {
		if (CONFIG_IS_ENABLED(OF_LIBFDT) && images->ft_len) {
#ifdef CONFIG_SMP
			ret = smp_call_function(images->ep,
						(ulong)images->ft_addr, 0, 0);
			if (ret)
				hang();
#endif
			kernel(gd->arch.boot_hart, images->ft_addr);
		}
	}
}

int do_bootm_linux(int flag, struct bootm_info *bmi)
{
	struct bootm_headers *images = bmi->images;

	/* No need for those on RISC-V */
	if (flag & BOOTM_STATE_OS_BD_T || flag & BOOTM_STATE_OS_CMDLINE)
		return -1;

	if (flag & BOOTM_STATE_OS_PREP) {
		boot_prep_linux(images);
		return 0;
	}

	if (flag & (BOOTM_STATE_OS_GO | BOOTM_STATE_OS_FAKE_GO)) {
		boot_jump_linux(images, flag);
		return 0;
	}

	boot_prep_linux(images);
	boot_jump_linux(images, flag);
	return 0;
}

int do_bootm_vxworks(int flag, struct bootm_info *bmi)
{
	return do_bootm_linux(flag, bmi);
}

static ulong get_sp(void)
{
	ulong ret;

	asm("mv %0, sp" : "=r"(ret) : );
	return ret;
}

void arch_lmb_reserve(struct lmb *lmb)
{
	arch_lmb_reserve_generic(lmb, get_sp(), gd->ram_top, 4096);
}
