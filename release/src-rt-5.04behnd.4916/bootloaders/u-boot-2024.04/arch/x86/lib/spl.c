// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 */

#define LOG_CATEGORY	LOGC_BOOT

#include <common.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <dm.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <irq_func.h>
#include <log.h>
#include <malloc.h>
#include <spl.h>
#include <syscon.h>
#include <vesa.h>
#include <asm/cpu.h>
#include <asm/cpu_common.h>
#include <asm/fsp2/fsp_api.h>
#include <asm/global_data.h>
#include <asm/mp.h>
#include <asm/mrccache.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/processor.h>
#include <asm/qemu.h>
#include <asm/spl.h>
#include <asm-generic/sections.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int fsp_setup_pinctrl(void *ctx, struct event *event)
{
	return 0;
}

#ifdef CONFIG_TPL

static int set_max_freq(void)
{
	if (cpu_get_burst_mode_state() == BURST_MODE_UNAVAILABLE) {
		/*
		 * Burst Mode has been factory-configured as disabled and is not
		 * available in this physical processor package
		 */
		debug("Burst Mode is factory-disabled\n");
		return -ENOENT;
	}

	/* Enable burst mode */
	cpu_set_burst_mode(true);

	/* Enable speed step */
	cpu_set_eist(true);

	/* Set P-State ratio */
	cpu_set_p_state_to_turbo_ratio();

	return 0;
}
#endif

static int x86_spl_init(void)
{
	struct udevice *dev;

#ifndef CONFIG_TPL
	/*
	 * TODO(sjg@chromium.org): We use this area of RAM for the stack
	 * and global_data in SPL. Once U-Boot starts up and releocates it
	 * is not needed. We could make this a CONFIG option or perhaps
	 * place it immediately below CONFIG_TEXT_BASE.
	 */
	__maybe_unused char *ptr = (char *)0x110000;
#else
	struct udevice *punit;
#endif
	int ret;

	log_debug("x86 spl starting\n");
	if (IS_ENABLED(TPL))
		ret = x86_cpu_reinit_f();
	else
		ret = x86_cpu_init_f();
	ret = spl_init();
	if (ret) {
		log_debug("spl_init() failed (err=%d)\n", ret);
		return ret;
	}
	ret = arch_cpu_init();
	if (ret) {
		log_debug("arch_cpu_init() failed (err=%d)\n", ret);
		return ret;
	}
#ifndef CONFIG_TPL
	ret = fsp_setup_pinctrl(NULL, NULL);
	if (ret) {
		log_debug("fsp_setup_pinctrl() failed (err=%d)\n", ret);
		return ret;
	}
#endif
	/*
	 * spl_board_init() below sets up the console if enabled. If it isn't,
	 * do it here. We cannot call this twice since it results in a double
	 * banner and CI tests fail.
	 */
	if (!IS_ENABLED(CONFIG_SPL_BOARD_INIT))
		preloader_console_init();
#if !defined(CONFIG_TPL) && !CONFIG_IS_ENABLED(CPU)
	ret = print_cpuinfo();
	if (ret) {
		log_debug("print_cpuinfo() failed (err=%d)\n", ret);
		return ret;
	}
#endif
	/* probe the LPC so we get the GPIO_BASE set up correctly */
	ret = uclass_first_device_err(UCLASS_LPC, &dev);
	if (ret && ret != -ENODEV) {
		log_debug("lpc probe failed\n");
		return ret;
	}

	ret = dram_init();
	if (ret) {
		log_debug("dram_init() failed (err=%d)\n", ret);
		return ret;
	}
	log_debug("mrc\n");
	if (IS_ENABLED(CONFIG_ENABLE_MRC_CACHE)) {
		ret = mrccache_spl_save();
		if (ret)
			log_debug("Failed to write to mrccache (err=%d)\n",
				  ret);
	}

#ifndef CONFIG_SYS_COREBOOT
	debug("BSS clear from %lx to %lx len %lx\n", (ulong)__bss_start,
	      (ulong)__bss_end, (ulong)__bss_end - (ulong)__bss_start);
	memset(__bss_start, 0, (ulong)__bss_end - (ulong)__bss_start);
# ifndef CONFIG_TPL

	/* TODO(sjg@chromium.org): Consider calling cpu_init_r() here */
	ret = interrupt_init();
	if (ret) {
		debug("%s: interrupt_init() failed\n", __func__);
		return ret;
	}

	/*
	 * The stack grows down from ptr. Put the global data at ptr. This
	 * will only be used for SPL. Once SPL loads U-Boot proper it will
	 * set up its own stack.
	 */
	gd->new_gd = (struct global_data *)ptr;
	memcpy(gd->new_gd, gd, sizeof(*gd));

	log_debug("logging\n");
	/*
	 * Make sure logging is disabled when we switch, since the log system
	 * list head will move
	 */
	gd->new_gd->flags &= ~GD_FLG_LOG_READY;
	arch_setup_gd(gd->new_gd);
	gd->start_addr_sp = (ulong)ptr;

	/* start up logging again, with the new list-head location */
	ret = log_init();
	if (ret) {
		log_debug("Log setup failed (err=%d)\n", ret);
		return ret;
	}

	if (_LOG_DEBUG) {
		ret = mtrr_list(mtrr_get_var_count(), MP_SELECT_BSP);
		if (ret)
			printf("mtrr_list failed\n");
	}

	/* Cache the SPI flash. Otherwise copying the code to RAM takes ages */
	ret = mtrr_add_request(MTRR_TYPE_WRBACK,
			       (1ULL << 32) - CONFIG_XIP_ROM_SIZE,
			       CONFIG_XIP_ROM_SIZE);
	if (ret) {
		debug("%s: SPI cache setup failed (err=%d)\n", __func__, ret);
		return ret;
	}
# else
	ret = syscon_get_by_driver_data(X86_SYSCON_PUNIT, &punit);
	if (ret)
		debug("Could not find PUNIT (err=%d)\n", ret);

	ret = set_max_freq();
	if (ret)
		debug("Failed to set CPU frequency (err=%d)\n", ret);
# endif
#endif
	log_debug("done\n");

	return 0;
}

void board_init_f(ulong flags)
{
	int ret;

	ret = x86_spl_init();
	if (ret) {
		printf("x86_spl_init: error %d\n", ret);
		hang();
	}
#if IS_ENABLED(CONFIG_TPL) || IS_ENABLED(CONFIG_SYS_COREBOOT)
	gd->bd = malloc(sizeof(*gd->bd));
	if (!gd->bd) {
		printf("Out of memory for bd_info size %x\n", sizeof(*gd->bd));
		hang();
	}
	board_init_r(gd, 0);
#else
	/* Uninit CAR and jump to board_init_f_r() */
	board_init_f_r_trampoline(gd->start_addr_sp);
#endif
}

void board_init_f_r(void)
{
	mtrr_commit(false);
	init_cache();
	gd->flags &= ~GD_FLG_SERIAL_READY;

	/* make sure driver model is not accessed from now on */
	gd->flags |= GD_FLG_DM_DEAD;
	debug("cache status %d\n", dcache_status());
	board_init_r(gd, 0);
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_SPI_MMAP;
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
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
	spl_image->entry_point = CONFIG_TEXT_BASE;
	spl_image->load_addr = CONFIG_TEXT_BASE;
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = "U-Boot";

	if (spl_image->load_addr != spl_get_image_pos()) {
		/* Copy U-Boot from ROM */
		memcpy((void *)spl_image->load_addr,
		       (void *)spl_get_image_pos(), spl_get_image_size());
	}

	debug("Loading to %lx\n", spl_image->load_addr);

	return 0;
}
SPL_LOAD_IMAGE_METHOD("SPI", 5, BOOT_DEVICE_SPI_MMAP, spl_board_load_image);

int spl_spi_load_image(void)
{
	return -EPERM;
}

#ifdef CONFIG_X86_RUN_64BIT
void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	int ret;

	printf("Jumping to 64-bit U-Boot: Note many features are missing\n");
	ret = cpu_jump_to_64bit_uboot(spl_image->entry_point);
	debug("ret=%d\n", ret);
	hang();
}
#endif

void spl_board_init(void)
{
#ifndef CONFIG_TPL
	preloader_console_init();
#endif
	if (IS_ENABLED(CONFIG_QEMU))
		qemu_chipset_init();

	if (CONFIG_IS_ENABLED(VIDEO)) {
		struct udevice *dev;

		/* Set up PCI video in SPL if required */
		uclass_first_device_err(UCLASS_PCI, &dev);
		uclass_first_device_err(UCLASS_VIDEO, &dev);
	}
}
