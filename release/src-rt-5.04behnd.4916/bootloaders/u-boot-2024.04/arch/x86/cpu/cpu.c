// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Part of this file is adapted from coreboot
 * src/arch/x86/lib/cpu.c
 */

#define LOG_CATEGORY	UCLASS_CPU

#include <common.h>
#include <bootstage.h>
#include <command.h>
#include <cpu_func.h>
#include <dm.h>
#include <errno.h>
#include <event.h>
#include <init.h>
#include <irq.h>
#include <log.h>
#include <malloc.h>
#include <syscon.h>
#include <acpi/acpi_s3.h>
#include <acpi/acpi_table.h>
#include <asm/acpi.h>
#include <asm/control_regs.h>
#include <asm/coreboot_tables.h>
#include <asm/cpu.h>
#include <asm/global_data.h>
#include <asm/lapic.h>
#include <asm/microcode.h>
#include <asm/mp.h>
#include <asm/mrccache.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <asm/post.h>
#include <asm/processor.h>
#include <asm/processor-flags.h>
#include <asm/interrupt.h>
#include <asm/tables.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_TPL_BUILD
static const char *const x86_vendor_name[] = {
	[X86_VENDOR_INTEL]     = "Intel",
	[X86_VENDOR_CYRIX]     = "Cyrix",
	[X86_VENDOR_AMD]       = "AMD",
	[X86_VENDOR_UMC]       = "UMC",
	[X86_VENDOR_NEXGEN]    = "NexGen",
	[X86_VENDOR_CENTAUR]   = "Centaur",
	[X86_VENDOR_RISE]      = "Rise",
	[X86_VENDOR_TRANSMETA] = "Transmeta",
	[X86_VENDOR_NSC]       = "NSC",
	[X86_VENDOR_SIS]       = "SiS",
};
#endif

int __weak x86_cleanup_before_linux(void)
{
	int ret;

	ret = mp_park_aps();
	if (ret)
		return log_msg_ret("park", ret);
	bootstage_stash((void *)CONFIG_BOOTSTAGE_STASH_ADDR,
			CONFIG_BOOTSTAGE_STASH_SIZE);

	return 0;
}

int x86_init_cache(void)
{
	enable_caches();

	return 0;
}
int init_cache(void) __attribute__((weak, alias("x86_init_cache")));

void  flush_cache(unsigned long dummy1, unsigned long dummy2)
{
	asm("wbinvd\n");
}

/* Define these functions to allow ehch-hcd to function */
void flush_dcache_range(unsigned long start, unsigned long stop)
{
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
}

void dcache_enable(void)
{
	enable_caches();
}

void dcache_disable(void)
{
	disable_caches();
}

void icache_enable(void)
{
}

void icache_disable(void)
{
}

int icache_status(void)
{
	return 1;
}

#ifndef CONFIG_TPL_BUILD
const char *cpu_vendor_name(int vendor)
{
	const char *name;
	name = "<invalid cpu vendor>";
	if (vendor < ARRAY_SIZE(x86_vendor_name) &&
	    x86_vendor_name[vendor])
		name = x86_vendor_name[vendor];

	return name;
}
#endif

char *cpu_get_name(char *name)
{
	unsigned int *name_as_ints = (unsigned int *)name;
	struct cpuid_result regs;
	char *ptr;
	int i;

	/* This bit adds up to 48 bytes */
	for (i = 0; i < 3; i++) {
		regs = cpuid(0x80000002 + i);
		name_as_ints[i * 4 + 0] = regs.eax;
		name_as_ints[i * 4 + 1] = regs.ebx;
		name_as_ints[i * 4 + 2] = regs.ecx;
		name_as_ints[i * 4 + 3] = regs.edx;
	}
	name[CPU_MAX_NAME_LEN - 1] = '\0';

	/* Skip leading spaces. */
	ptr = name;
	while (*ptr == ' ')
		ptr++;

	return ptr;
}

int default_print_cpuinfo(void)
{
	printf("CPU: %s, vendor %s, device %xh\n",
	       cpu_has_64bit() ? "x86_64" : "x86",
	       cpu_vendor_name(gd->arch.x86_vendor), gd->arch.x86_device);

	if (IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)) {
		debug("ACPI previous sleep state: %s\n",
		      acpi_ss_string(gd->arch.prev_sleep_state));
	}

	return 0;
}

#if CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
void show_boot_progress(int val)
{
	outb(val, POST_PORT);
}
#endif

#if !defined(CONFIG_SYS_COREBOOT) && !defined(CONFIG_EFI_STUB) && \
	!defined(CONFIG_SPL_BUILD)
/*
 * Implement a weak default function for boards that need to do some final init
 * before the system is ready.
 */
__weak void board_final_init(void)
{
}

/*
 * Implement a weak default function for boards that need to do some final
 * processing before booting the OS.
 */
__weak void board_final_cleanup(void)
{
}

static int last_stage_init(void)
{
	struct acpi_fadt __maybe_unused *fadt;
	int ret;

	board_final_init();

	if (IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)) {
		fadt = acpi_find_fadt();

		if (fadt && gd->arch.prev_sleep_state == ACPI_S3)
			acpi_resume(fadt);
	}

	ret = write_tables();
	if (ret) {
		log_err("Failed to write tables\n");
		return log_msg_ret("table", ret);
	}

	if (IS_ENABLED(CONFIG_GENERATE_ACPI_TABLE)) {
		fadt = acpi_find_fadt();

		/* Don't touch ACPI hardware on HW reduced platforms */
		if (fadt && !(fadt->flags & ACPI_FADT_HW_REDUCED_ACPI)) {
			/*
			 * Other than waiting for OSPM to request us to switch
			 * to ACPI * mode, do it by ourselves, since SMI will
			 * not be triggered.
			 */
			enter_acpi_mode(fadt->pm1a_cnt_blk);
		}
	}

	/*
	 * TODO(sjg@chromium.org): Move this to bootm_announce_and_cleanup()
	 * once APL FSP-S at 0x200000 does not overlap with the bzimage at
	 * 0x100000.
	 */
	board_final_cleanup();

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);

#endif  /* !SYS_COREBOOT && !EFI_STUB && !SPL_BUILD */

static int x86_init_cpus(void)
{
	if (IS_ENABLED(CONFIG_SMP)) {
		debug("Init additional CPUs\n");
		x86_mp_init();
	} else {
		struct udevice *dev;

		/*
		 * This causes the cpu-x86 driver to be probed.
		 * We don't check return value here as we want to allow boards
		 * which have not been converted to use cpu uclass driver to
		 * boot.
		 */
		uclass_first_device(UCLASS_CPU, &dev);
	}

	return 0;
}

int cpu_init_r(void)
{
	struct udevice *dev;
	int ret;

	if (!ll_boot_init()) {
		uclass_first_device(UCLASS_PCI, &dev);
		return 0;
	}

	ret = x86_init_cpus();
	if (ret)
		return ret;

	/*
	 * Set up the northbridge, PCH and LPC if available. Note that these
	 * may have had some limited pre-relocation init if they were probed
	 * before relocation, but this is post relocation.
	 */
	uclass_first_device(UCLASS_NORTHBRIDGE, &dev);
	uclass_first_device(UCLASS_PCH, &dev);
	uclass_first_device(UCLASS_LPC, &dev);

	/* Set up pin control if available */
	ret = syscon_get_by_driver_data(X86_SYSCON_PINCONF, &dev);
	debug("%s, pinctrl=%p, ret=%d\n", __func__, dev, ret);

	return 0;
}

#ifndef CONFIG_EFI_STUB
int reserve_arch(void)
{
	struct udevice *itss;
	int ret;

	if (IS_ENABLED(CONFIG_ENABLE_MRC_CACHE))
		mrccache_reserve();

	if (IS_ENABLED(CONFIG_SEABIOS))
		high_table_reserve();

	if (IS_ENABLED(CONFIG_HAVE_ACPI_RESUME)) {
		acpi_s3_reserve();

		if (IS_ENABLED(CONFIG_HAVE_FSP)) {
			/*
			 * Save stack address to CMOS so that at next S3 boot,
			 * we can use it as the stack address for fsp_continue()
			 */
			fsp_save_s3_stack();
		}
	}
	ret = irq_first_device_type(X86_IRQT_ITSS, &itss);
	if (!ret) {
		/*
		 * Snapshot the current GPIO IRQ polarities. FSP-S is about to
		 * run and will set a default policy that doesn't honour boards'
		 * requirements
		 */
		irq_snapshot_polarities(itss);
	}

	return 0;
}
#endif

long detect_coreboot_table_at(ulong start, ulong size)
{
	u32 *ptr, *end;

	size /= 4;
	for (ptr = (void *)start, end = ptr + size; ptr < end; ptr += 4) {
		if (*ptr == 0x4f49424c) /* "LBIO" */
			return (long)ptr;
	}

	return -ENOENT;
}

long locate_coreboot_table(void)
{
	long addr;

	/* We look for LBIO from addresses 1K-4K and again at 960KB */
	addr = detect_coreboot_table_at(0x400, 0xc00);
	if (addr < 0)
		addr = detect_coreboot_table_at(0xf0000, 0x1000);

	return addr;
}
