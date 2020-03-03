/*
 * Copyright IBM Corp. 2005, 2011
 *
 * Author(s): Rolf Adelsberger,
 *	      Heiko Carstens <heiko.carstens@de.ibm.com>
 *	      Michael Holzheu <holzheu@linux.vnet.ibm.com>
 */

#include <linux/device.h>
#include <linux/mm.h>
#include <linux/kexec.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/ftrace.h>
#include <linux/debug_locks.h>
#include <linux/suspend.h>
#include <asm/cio.h>
#include <asm/setup.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/smp.h>
#include <asm/reset.h>
#include <asm/ipl.h>
#include <asm/diag.h>
#include <asm/elf.h>
#include <asm/asm-offsets.h>
#include <asm/os_info.h>
#include <asm/switch_to.h>

typedef void (*relocate_kernel_t)(kimage_entry_t *, unsigned long);

extern const unsigned char relocate_kernel[];
extern const unsigned long long relocate_kernel_len;

#ifdef CONFIG_CRASH_DUMP

/*
 * Create ELF notes for one CPU
 */
static void add_elf_notes(int cpu)
{
	struct save_area *sa = (void *) 4608 + store_prefix();
	void *ptr;

	memcpy((void *) (4608UL + sa->pref_reg), sa, sizeof(*sa));
	ptr = (u64 *) per_cpu_ptr(crash_notes, cpu);
	ptr = fill_cpu_elf_notes(ptr, sa, NULL);
	memset(ptr, 0, sizeof(struct elf_note));
}

/*
 * Initialize CPU ELF notes
 */
static void setup_regs(void)
{
	unsigned long sa = S390_lowcore.prefixreg_save_area + SAVE_AREA_BASE;
	struct _lowcore *lc;
	int cpu, this_cpu;

	/* Get lowcore pointer from store status of this CPU (absolute zero) */
	lc = (struct _lowcore *)(unsigned long)S390_lowcore.prefixreg_save_area;
	this_cpu = smp_find_processor_id(stap());
	add_elf_notes(this_cpu);
	for_each_online_cpu(cpu) {
		if (cpu == this_cpu)
			continue;
		if (smp_store_status(cpu))
			continue;
		add_elf_notes(cpu);
	}
	if (MACHINE_HAS_VX)
		save_vx_regs_safe((void *) lc->vector_save_area_addr);
	/* Copy dump CPU store status info to absolute zero */
	memcpy((void *) SAVE_AREA_BASE, (void *) sa, sizeof(struct save_area));
}

/*
 * PM notifier callback for kdump
 */
static int machine_kdump_pm_cb(struct notifier_block *nb, unsigned long action,
			       void *ptr)
{
	switch (action) {
	case PM_SUSPEND_PREPARE:
	case PM_HIBERNATION_PREPARE:
		if (crashk_res.start)
			crash_map_reserved_pages();
		break;
	case PM_POST_SUSPEND:
	case PM_POST_HIBERNATION:
		if (crashk_res.start)
			crash_unmap_reserved_pages();
		break;
	default:
		return NOTIFY_DONE;
	}
	return NOTIFY_OK;
}

static int __init machine_kdump_pm_init(void)
{
	pm_notifier(machine_kdump_pm_cb, 0);
	return 0;
}
arch_initcall(machine_kdump_pm_init);

/*
 * Start kdump: We expect here that a store status has been done on our CPU
 */
static void __do_machine_kdump(void *image)
{
	int (*start_kdump)(int) = (void *)((struct kimage *) image)->start;

	__load_psw_mask(PSW_MASK_BASE | PSW_DEFAULT_KEY | PSW_MASK_EA | PSW_MASK_BA);
	start_kdump(1);
}
#endif

/*
 * Check if kdump checksums are valid: We call purgatory with parameter "0"
 */
static int kdump_csum_valid(struct kimage *image)
{
#ifdef CONFIG_CRASH_DUMP
	int (*start_kdump)(int) = (void *)image->start;
	int rc;

	__arch_local_irq_stnsm(0xfb); /* disable DAT */
	rc = start_kdump(0);
	__arch_local_irq_stosm(0x04); /* enable DAT */
	return rc ? 0 : -EINVAL;
#else
	return -EINVAL;
#endif
}

/*
 * Map or unmap crashkernel memory
 */
static void crash_map_pages(int enable)
{
	unsigned long size = resource_size(&crashk_res);

	BUG_ON(crashk_res.start % KEXEC_CRASH_MEM_ALIGN ||
	       size % KEXEC_CRASH_MEM_ALIGN);
	if (enable)
		vmem_add_mapping(crashk_res.start, size);
	else {
		vmem_remove_mapping(crashk_res.start, size);
		if (size)
			os_info_crashkernel_add(crashk_res.start, size);
		else
			os_info_crashkernel_add(0, 0);
	}
}

/*
 * Map crashkernel memory
 */
void crash_map_reserved_pages(void)
{
	crash_map_pages(1);
}

/*
 * Unmap crashkernel memory
 */
void crash_unmap_reserved_pages(void)
{
	crash_map_pages(0);
}

/*
 * Give back memory to hypervisor before new kdump is loaded
 */
static int machine_kexec_prepare_kdump(void)
{
#ifdef CONFIG_CRASH_DUMP
	if (MACHINE_IS_VM)
		diag10_range(PFN_DOWN(crashk_res.start),
			     PFN_DOWN(crashk_res.end - crashk_res.start + 1));
	return 0;
#else
	return -EINVAL;
#endif
}

int machine_kexec_prepare(struct kimage *image)
{
	void *reboot_code_buffer;

	/* Can't replace kernel image since it is read-only. */
	if (ipl_flags & IPL_NSS_VALID)
		return -EOPNOTSUPP;

	if (image->type == KEXEC_TYPE_CRASH)
		return machine_kexec_prepare_kdump();

	/* We don't support anything but the default image type for now. */
	if (image->type != KEXEC_TYPE_DEFAULT)
		return -EINVAL;

	/* Get the destination where the assembler code should be copied to.*/
	reboot_code_buffer = (void *) page_to_phys(image->control_code_page);

	/* Then copy it */
	memcpy(reboot_code_buffer, relocate_kernel, relocate_kernel_len);
	return 0;
}

void machine_kexec_cleanup(struct kimage *image)
{
}

void arch_crash_save_vmcoreinfo(void)
{
	VMCOREINFO_SYMBOL(lowcore_ptr);
	VMCOREINFO_SYMBOL(high_memory);
	VMCOREINFO_LENGTH(lowcore_ptr, NR_CPUS);
}

void machine_shutdown(void)
{
}

void machine_crash_shutdown(struct pt_regs *regs)
{
}

/*
 * Do normal kexec
 */
static void __do_machine_kexec(void *data)
{
	relocate_kernel_t data_mover;
	struct kimage *image = data;

	data_mover = (relocate_kernel_t) page_to_phys(image->control_code_page);

	/* Call the moving routine */
	(*data_mover)(&image->head, image->start);
}

/*
 * Reset system and call either kdump or normal kexec
 */
static void __machine_kexec(void *data)
{
	__arch_local_irq_stosm(0x04); /* enable DAT */
	pfault_fini();
	tracing_off();
	debug_locks_off();
#ifdef CONFIG_CRASH_DUMP
	if (((struct kimage *) data)->type == KEXEC_TYPE_CRASH) {

		lgr_info_log();
		s390_reset_system(setup_regs, __do_machine_kdump, data);
	} else
#endif
		s390_reset_system(NULL, __do_machine_kexec, data);
	disabled_wait((unsigned long) __builtin_return_address(0));
}

/*
 * Do either kdump or normal kexec. In case of kdump we first ask
 * purgatory, if kdump checksums are valid.
 */
void machine_kexec(struct kimage *image)
{
	if (image->type == KEXEC_TYPE_CRASH && !kdump_csum_valid(image))
		return;
	tracer_disable();
	smp_send_stop();
	smp_call_ipl_cpu(__machine_kexec, image);
}
