/*
 * Suspend support specific for i386/x86-64.
 *
 * Distribute under GPLv2
 *
 * Copyright (c) 2007 Rafael J. Wysocki <rjw@sisk.pl>
 * Copyright (c) 2002 Pavel Machek <pavel@ucw.cz>
 * Copyright (c) 2001 Patrick Mochel <mochel@osdl.org>
 */

#include <linux/suspend.h>
#include <linux/export.h>
#include <linux/smp.h>
#include <linux/perf_event.h>

#include <asm/pgtable.h>
#include <asm/proto.h>
#include <asm/mtrr.h>
#include <asm/page.h>
#include <asm/mce.h>
#include <asm/xcr.h>
#include <asm/suspend.h>
#include <asm/debugreg.h>
#include <asm/fpu-internal.h> /* pcntxt_mask */
#include <asm/cpu.h>
#include <asm/mmu_context.h>

#ifdef CONFIG_X86_32
__visible unsigned long saved_context_ebx;
__visible unsigned long saved_context_esp, saved_context_ebp;
__visible unsigned long saved_context_esi, saved_context_edi;
__visible unsigned long saved_context_eflags;
#endif
struct saved_context saved_context;

/**
 *	__save_processor_state - save CPU registers before creating a
 *		hibernation image and before restoring the memory state from it
 *	@ctxt - structure to store the registers contents in
 *
 *	NOTE: If there is a CPU register the modification of which by the
 *	boot kernel (ie. the kernel used for loading the hibernation image)
 *	might affect the operations of the restored target kernel (ie. the one
 *	saved in the hibernation image), then its contents must be saved by this
 *	function.  In other words, if kernel A is hibernated and different
 *	kernel B is used for loading the hibernation image into memory, the
 *	kernel A's __save_processor_state() function must save all registers
 *	needed by kernel A, so that it can operate correctly after the resume
 *	regardless of what kernel B does in the meantime.
 */
static void __save_processor_state(struct saved_context *ctxt)
{
#ifdef CONFIG_X86_32
	mtrr_save_fixed_ranges(NULL);
#endif
	kernel_fpu_begin();

	/*
	 * descriptor tables
	 */
#ifdef CONFIG_X86_32
	store_idt(&ctxt->idt);
#else
/* CONFIG_X86_64 */
	store_idt((struct desc_ptr *)&ctxt->idt_limit);
#endif
	/*
	 * We save it here, but restore it only in the hibernate case.
	 * For ACPI S3 resume, this is loaded via 'early_gdt_desc' in 64-bit
	 * mode in "secondary_startup_64". In 32-bit mode it is done via
	 * 'pmode_gdt' in wakeup_start.
	 */
	ctxt->gdt_desc.size = GDT_SIZE - 1;
	ctxt->gdt_desc.address = (unsigned long)get_cpu_gdt_table(smp_processor_id());

	store_tr(ctxt->tr);

	/* XMM0..XMM15 should be handled by kernel_fpu_begin(). */
	/*
	 * segment registers
	 */
#ifdef CONFIG_X86_32
	savesegment(es, ctxt->es);
	savesegment(fs, ctxt->fs);
	savesegment(gs, ctxt->gs);
	savesegment(ss, ctxt->ss);
#else
/* CONFIG_X86_64 */
	asm volatile ("movw %%ds, %0" : "=m" (ctxt->ds));
	asm volatile ("movw %%es, %0" : "=m" (ctxt->es));
	asm volatile ("movw %%fs, %0" : "=m" (ctxt->fs));
	asm volatile ("movw %%gs, %0" : "=m" (ctxt->gs));
	asm volatile ("movw %%ss, %0" : "=m" (ctxt->ss));

	rdmsrl(MSR_FS_BASE, ctxt->fs_base);
	rdmsrl(MSR_GS_BASE, ctxt->gs_base);
	rdmsrl(MSR_KERNEL_GS_BASE, ctxt->gs_kernel_base);
	mtrr_save_fixed_ranges(NULL);

	rdmsrl(MSR_EFER, ctxt->efer);
#endif

	/*
	 * control registers
	 */
	ctxt->cr0 = read_cr0();
	ctxt->cr2 = read_cr2();
	ctxt->cr3 = read_cr3();
	ctxt->cr4 = __read_cr4_safe();
#ifdef CONFIG_X86_64
	ctxt->cr8 = read_cr8();
#endif
	ctxt->misc_enable_saved = !rdmsrl_safe(MSR_IA32_MISC_ENABLE,
					       &ctxt->misc_enable);
}

/* Needed by apm.c */
void save_processor_state(void)
{
	__save_processor_state(&saved_context);
	x86_platform.save_sched_clock_state();
}
#ifdef CONFIG_X86_32
EXPORT_SYMBOL(save_processor_state);
#endif

static void do_fpu_end(void)
{
	/*
	 * Restore FPU regs if necessary.
	 */
	kernel_fpu_end();
}

static void fix_processor_context(void)
{
	int cpu = smp_processor_id();
	struct tss_struct *t = &per_cpu(cpu_tss, cpu);
#ifdef CONFIG_X86_64
	struct desc_struct *desc = get_cpu_gdt_table(cpu);
	tss_desc tss;
#endif
	set_tss_desc(cpu, t);	/*
				 * This just modifies memory; should not be
				 * necessary. But... This is necessary, because
				 * 386 hardware has concept of busy TSS or some
				 * similar stupidity.
				 */

#ifdef CONFIG_X86_64
	memcpy(&tss, &desc[GDT_ENTRY_TSS], sizeof(tss_desc));
	tss.type = 0x9; /* The available 64-bit TSS (see AMD vol 2, pg 91 */
	write_gdt_entry(desc, GDT_ENTRY_TSS, &tss, DESC_TSS);

	syscall_init();				/* This sets MSR_*STAR and related */
#endif
	load_TR_desc();				/* This does ltr */
	load_mm_ldt(current->active_mm);	/* This does lldt */
}

/**
 *	__restore_processor_state - restore the contents of CPU registers saved
 *		by __save_processor_state()
 *	@ctxt - structure to load the registers contents from
 */
static void notrace __restore_processor_state(struct saved_context *ctxt)
{
	if (ctxt->misc_enable_saved)
		wrmsrl(MSR_IA32_MISC_ENABLE, ctxt->misc_enable);
	/*
	 * control registers
	 */
	/* cr4 was introduced in the Pentium CPU */
#ifdef CONFIG_X86_32
	if (ctxt->cr4)
		__write_cr4(ctxt->cr4);
#else
/* CONFIG X86_64 */
	wrmsrl(MSR_EFER, ctxt->efer);
	write_cr8(ctxt->cr8);
	__write_cr4(ctxt->cr4);
#endif
	write_cr3(ctxt->cr3);
	write_cr2(ctxt->cr2);
	write_cr0(ctxt->cr0);

	/*
	 * now restore the descriptor tables to their proper values
	 * ltr is done i fix_processor_context().
	 */
#ifdef CONFIG_X86_32
	load_idt(&ctxt->idt);
#else
/* CONFIG_X86_64 */
	load_idt((const struct desc_ptr *)&ctxt->idt_limit);
#endif

	/*
	 * segment registers
	 */
#ifdef CONFIG_X86_32
	loadsegment(es, ctxt->es);
	loadsegment(fs, ctxt->fs);
	loadsegment(gs, ctxt->gs);
	loadsegment(ss, ctxt->ss);

	/*
	 * sysenter MSRs
	 */
	if (boot_cpu_has(X86_FEATURE_SEP))
		enable_sep_cpu();
#else
/* CONFIG_X86_64 */
	asm volatile ("movw %0, %%ds" :: "r" (ctxt->ds));
	asm volatile ("movw %0, %%es" :: "r" (ctxt->es));
	asm volatile ("movw %0, %%fs" :: "r" (ctxt->fs));
	load_gs_index(ctxt->gs);
	asm volatile ("movw %0, %%ss" :: "r" (ctxt->ss));

	wrmsrl(MSR_FS_BASE, ctxt->fs_base);
	wrmsrl(MSR_GS_BASE, ctxt->gs_base);
	wrmsrl(MSR_KERNEL_GS_BASE, ctxt->gs_kernel_base);
#endif

	/*
	 * restore XCR0 for xsave capable cpu's.
	 */
	if (cpu_has_xsave)
		xsetbv(XCR_XFEATURE_ENABLED_MASK, pcntxt_mask);

	fix_processor_context();

	do_fpu_end();
	x86_platform.restore_sched_clock_state();
	mtrr_bp_restore();
	perf_restore_debug_store();
}

/* Needed by apm.c */
void notrace restore_processor_state(void)
{
	__restore_processor_state(&saved_context);
}
#ifdef CONFIG_X86_32
EXPORT_SYMBOL(restore_processor_state);
#endif

/*
 * When bsp_check() is called in hibernate and suspend, cpu hotplug
 * is disabled already. So it's unnessary to handle race condition between
 * cpumask query and cpu hotplug.
 */
static int bsp_check(void)
{
	if (cpumask_first(cpu_online_mask) != 0) {
		pr_warn("CPU0 is offline.\n");
		return -ENODEV;
	}

	return 0;
}

static int bsp_pm_callback(struct notifier_block *nb, unsigned long action,
			   void *ptr)
{
	int ret = 0;

	switch (action) {
	case PM_SUSPEND_PREPARE:
	case PM_HIBERNATION_PREPARE:
		ret = bsp_check();
		break;
#ifdef CONFIG_DEBUG_HOTPLUG_CPU0
	case PM_RESTORE_PREPARE:
		/*
		 * When system resumes from hibernation, online CPU0 because
		 * 1. it's required for resume and
		 * 2. the CPU was online before hibernation
		 */
		if (!cpu_online(0))
			_debug_hotplug_cpu(0, 1);
		break;
	case PM_POST_RESTORE:
		/*
		 * When a resume really happens, this code won't be called.
		 *
		 * This code is called only when user space hibernation software
		 * prepares for snapshot device during boot time. So we just
		 * call _debug_hotplug_cpu() to restore to CPU0's state prior to
		 * preparing the snapshot device.
		 *
		 * This works for normal boot case in our CPU0 hotplug debug
		 * mode, i.e. CPU0 is offline and user mode hibernation
		 * software initializes during boot time.
		 *
		 * If CPU0 is online and user application accesses snapshot
		 * device after boot time, this will offline CPU0 and user may
		 * see different CPU0 state before and after accessing
		 * the snapshot device. But hopefully this is not a case when
		 * user debugging CPU0 hotplug. Even if users hit this case,
		 * they can easily online CPU0 back.
		 *
		 * To simplify this debug code, we only consider normal boot
		 * case. Otherwise we need to remember CPU0's state and restore
		 * to that state and resolve racy conditions etc.
		 */
		_debug_hotplug_cpu(0, 0);
		break;
#endif
	default:
		break;
	}
	return notifier_from_errno(ret);
}

static int __init bsp_pm_check_init(void)
{
	/*
	 * Set this bsp_pm_callback as lower priority than
	 * cpu_hotplug_pm_callback. So cpu_hotplug_pm_callback will be called
	 * earlier to disable cpu hotplug before bsp online check.
	 */
	pm_notifier(bsp_pm_callback, -INT_MAX);
	return 0;
}

core_initcall(bsp_pm_check_init);
