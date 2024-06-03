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

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <syscon.h>
#include <asm/acpi.h>
#include <asm/acpi_s3.h>
#include <asm/acpi_table.h>
#include <asm/control_regs.h>
#include <asm/coreboot_tables.h>
#include <asm/cpu.h>
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

int __weak x86_cleanup_before_linux(void)
{
#ifdef CONFIG_BOOTSTAGE_STASH
	bootstage_stash((void *)CONFIG_BOOTSTAGE_STASH_ADDR,
			CONFIG_BOOTSTAGE_STASH_SIZE);
#endif

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

const char *cpu_vendor_name(int vendor)
{
	const char *name;
	name = "<invalid cpu vendor>";
	if (vendor < ARRAY_SIZE(x86_vendor_name) &&
	    x86_vendor_name[vendor])
		name = x86_vendor_name[vendor];

	return name;
}

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

#ifdef CONFIG_HAVE_ACPI_RESUME
	debug("ACPI previous sleep state: %s\n",
	      acpi_ss_string(gd->arch.prev_sleep_state));
#endif

	return 0;
}

void show_boot_progress(int val)
{
	outb(val, POST_PORT);
}

#if !defined(CONFIG_SYS_COREBOOT) && !defined(CONFIG_EFI_STUB)
/*
 * Implement a weak default function for boards that optionally
 * need to clean up the system before jumping to the kernel.
 */
__weak void board_final_cleanup(void)
{
}

int last_stage_init(void)
{
	struct acpi_fadt __maybe_unused *fadt;

	board_final_cleanup();

#ifdef CONFIG_HAVE_ACPI_RESUME
	fadt = acpi_find_fadt();

	if (fadt && gd->arch.prev_sleep_state == ACPI_S3)
		acpi_resume(fadt);
#endif

	write_tables();

#ifdef CONFIG_GENERATE_ACPI_TABLE
	fadt = acpi_find_fadt();

	/* Don't touch ACPI hardware on HW reduced platforms */
	if (fadt && !(fadt->flags & ACPI_FADT_HW_REDUCED_ACPI)) {
		/*
		 * Other than waiting for OSPM to request us to switch to ACPI
		 * mode, do it by ourselves, since SMI will not be triggered.
		 */
		enter_acpi_mode(fadt->pm1a_cnt_blk);
	}
#endif

	return 0;
}
#endif

static int x86_init_cpus(void)
{
#ifdef CONFIG_SMP
	debug("Init additional CPUs\n");
	x86_mp_init();
#else
	struct udevice *dev;

	/*
	 * This causes the cpu-x86 driver to be probed.
	 * We don't check return value here as we want to allow boards
	 * which have not been converted to use cpu uclass driver to boot.
	 */
	uclass_first_device(UCLASS_CPU, &dev);
#endif

	return 0;
}

int cpu_init_r(void)
{
	struct udevice *dev;
	int ret;

	if (!ll_boot_init())
		return 0;

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
#ifdef CONFIG_ENABLE_MRC_CACHE
	mrccache_reserve();
#endif

#ifdef CONFIG_SEABIOS
	high_table_reserve();
#endif

#ifdef CONFIG_HAVE_ACPI_RESUME
	acpi_s3_reserve();

#ifdef CONFIG_HAVE_FSP
	/*
	 * Save stack address to CMOS so that at next S3 boot,
	 * we can use it as the stack address for fsp_contiue()
	 */
	fsp_save_s3_stack();
#endif /* CONFIG_HAVE_FSP */
#endif /* CONFIG_HAVE_ACPI_RESUME */

	return 0;
}
#endif
