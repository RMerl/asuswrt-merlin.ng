/*
 * Copyright (C) 2009 Thomas Gleixner <tglx@linutronix.de>
 *
 *  For licencing details see kernel-base/COPYING
 */
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/pci.h>

#include <asm/bios_ebda.h>
#include <asm/paravirt.h>
#include <asm/pci_x86.h>
#include <asm/pci.h>
#include <asm/mpspec.h>
#include <asm/setup.h>
#include <asm/apic.h>
#include <asm/e820.h>
#include <asm/time.h>
#include <asm/irq.h>
#include <asm/io_apic.h>
#include <asm/hpet.h>
#include <asm/pat.h>
#include <asm/tsc.h>
#include <asm/iommu.h>
#include <asm/mach_traps.h>

void x86_init_noop(void) { }
void __init x86_init_uint_noop(unsigned int unused) { }
int __init iommu_init_noop(void) { return 0; }
void iommu_shutdown_noop(void) { }

/*
 * The platform setup functions are preset with the default functions
 * for standard PC hardware.
 */
struct x86_init_ops x86_init __initdata = {

	.resources = {
		.probe_roms		= probe_roms,
		.reserve_resources	= reserve_standard_io_resources,
		.memory_setup		= default_machine_specific_memory_setup,
	},

	.mpparse = {
		.mpc_record		= x86_init_uint_noop,
		.setup_ioapic_ids	= x86_init_noop,
		.mpc_apic_id		= default_mpc_apic_id,
		.smp_read_mpc_oem	= default_smp_read_mpc_oem,
		.mpc_oem_bus_info	= default_mpc_oem_bus_info,
		.find_smp_config	= default_find_smp_config,
		.get_smp_config		= default_get_smp_config,
	},

	.irqs = {
		.pre_vector_init	= init_ISA_irqs,
		.intr_init		= native_init_IRQ,
		.trap_init		= x86_init_noop,
	},

	.oem = {
		.arch_setup		= x86_init_noop,
		.banner			= default_banner,
	},

	.paging = {
		.pagetable_init		= native_pagetable_init,
	},

	.timers = {
		.setup_percpu_clockev	= setup_boot_APIC_clock,
		.tsc_pre_init		= x86_init_noop,
		.timer_init		= hpet_time_init,
		.wallclock_init		= x86_init_noop,
	},

	.iommu = {
		.iommu_init		= iommu_init_noop,
	},

	.pci = {
		.init			= x86_default_pci_init,
		.init_irq		= x86_default_pci_init_irq,
		.fixup_irqs		= x86_default_pci_fixup_irqs,
	},
};

struct x86_cpuinit_ops x86_cpuinit = {
	.early_percpu_clock_init	= x86_init_noop,
	.setup_percpu_clockev		= setup_secondary_APIC_clock,
};

static void default_nmi_init(void) { };
static int default_i8042_detect(void) { return 1; };

struct x86_platform_ops x86_platform = {
	.calibrate_tsc			= native_calibrate_tsc,
	.get_wallclock			= mach_get_cmos_time,
	.set_wallclock			= mach_set_rtc_mmss,
	.iommu_shutdown			= iommu_shutdown_noop,
	.is_untracked_pat_range		= is_ISA_range,
	.nmi_init			= default_nmi_init,
	.get_nmi_reason			= default_get_nmi_reason,
	.i8042_detect			= default_i8042_detect,
	.save_sched_clock_state 	= tsc_save_sched_clock_state,
	.restore_sched_clock_state 	= tsc_restore_sched_clock_state,
};

EXPORT_SYMBOL_GPL(x86_platform);

#if defined(CONFIG_PCI_MSI)
struct x86_msi_ops x86_msi = {
	.setup_msi_irqs		= native_setup_msi_irqs,
	.compose_msi_msg	= native_compose_msi_msg,
	.teardown_msi_irq	= native_teardown_msi_irq,
	.teardown_msi_irqs	= default_teardown_msi_irqs,
	.restore_msi_irqs	= default_restore_msi_irqs,
	.setup_hpet_msi		= default_setup_hpet_msi,
};

/* MSI arch specific hooks */
int arch_setup_msi_irqs(struct pci_dev *dev, int nvec, int type)
{
	return x86_msi.setup_msi_irqs(dev, nvec, type);
}

void arch_teardown_msi_irqs(struct pci_dev *dev)
{
	x86_msi.teardown_msi_irqs(dev);
}

void arch_teardown_msi_irq(unsigned int irq)
{
	x86_msi.teardown_msi_irq(irq);
}

void arch_restore_msi_irqs(struct pci_dev *dev)
{
	x86_msi.restore_msi_irqs(dev);
}
#endif

struct x86_io_apic_ops x86_io_apic_ops = {
	.init			= native_io_apic_init_mappings,
	.read			= native_io_apic_read,
	.write			= native_io_apic_write,
	.modify			= native_io_apic_modify,
	.disable		= native_disable_io_apic,
	.print_entries		= native_io_apic_print_entries,
	.set_affinity		= native_ioapic_set_affinity,
	.setup_entry		= native_setup_ioapic_entry,
	.eoi_ioapic_pin		= native_eoi_ioapic_pin,
};
