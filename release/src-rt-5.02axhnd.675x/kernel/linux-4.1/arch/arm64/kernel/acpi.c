/*
 *  ARM64 Specific Low-Level ACPI Boot Support
 *
 *  Copyright (C) 2013-2014, Linaro Ltd.
 *	Author: Al Stone <al.stone@linaro.org>
 *	Author: Graeme Gregory <graeme.gregory@linaro.org>
 *	Author: Hanjun Guo <hanjun.guo@linaro.org>
 *	Author: Tomasz Nowicki <tomasz.nowicki@linaro.org>
 *	Author: Naresh Bhat <naresh.bhat@linaro.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "ACPI: " fmt

#include <linux/acpi.h>
#include <linux/bootmem.h>
#include <linux/cpumask.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/memblock.h>
#include <linux/of_fdt.h>
#include <linux/smp.h>

#include <asm/cputype.h>
#include <asm/cpu_ops.h>
#include <asm/smp_plat.h>

int acpi_noirq = 1;		/* skip ACPI IRQ initialization */
int acpi_disabled = 1;
EXPORT_SYMBOL(acpi_disabled);

int acpi_pci_disabled = 1;	/* skip ACPI PCI scan and IRQ initialization */
EXPORT_SYMBOL(acpi_pci_disabled);

/* Processors with enabled flag and sane MPIDR */
static int enabled_cpus;

/* Boot CPU is valid or not in MADT */
static bool bootcpu_valid  __initdata;

static bool param_acpi_off __initdata;
static bool param_acpi_force __initdata;

static int __init parse_acpi(char *arg)
{
	if (!arg)
		return -EINVAL;

	/* "acpi=off" disables both ACPI table parsing and interpreter */
	if (strcmp(arg, "off") == 0)
		param_acpi_off = true;
	else if (strcmp(arg, "force") == 0) /* force ACPI to be enabled */
		param_acpi_force = true;
	else
		return -EINVAL;	/* Core will print when we return error */

	return 0;
}
early_param("acpi", parse_acpi);

static int __init dt_scan_depth1_nodes(unsigned long node,
				       const char *uname, int depth,
				       void *data)
{
	/*
	 * Return 1 as soon as we encounter a node at depth 1 that is
	 * not the /chosen node.
	 */
	if (depth == 1 && (strcmp(uname, "chosen") != 0))
		return 1;
	return 0;
}

/*
 * __acpi_map_table() will be called before page_init(), so early_ioremap()
 * or early_memremap() should be called here to for ACPI table mapping.
 */
char *__init __acpi_map_table(unsigned long phys, unsigned long size)
{
	if (!size)
		return NULL;

	return early_memremap(phys, size);
}

void __init __acpi_unmap_table(char *map, unsigned long size)
{
	if (!map || !size)
		return;

	early_memunmap(map, size);
}

/**
 * acpi_map_gic_cpu_interface - generates a logical cpu number
 * and map to MPIDR represented by GICC structure
 */
static void __init
acpi_map_gic_cpu_interface(struct acpi_madt_generic_interrupt *processor)
{
	int i;
	u64 mpidr = processor->arm_mpidr & MPIDR_HWID_BITMASK;
	bool enabled = !!(processor->flags & ACPI_MADT_ENABLED);

	if (mpidr == INVALID_HWID) {
		pr_info("Skip MADT cpu entry with invalid MPIDR\n");
		return;
	}

	total_cpus++;
	if (!enabled)
		return;

	if (enabled_cpus >=  NR_CPUS) {
		pr_warn("NR_CPUS limit of %d reached, Processor %d/0x%llx ignored.\n",
			NR_CPUS, total_cpus, mpidr);
		return;
	}

	/* Check if GICC structure of boot CPU is available in the MADT */
	if (cpu_logical_map(0) == mpidr) {
		if (bootcpu_valid) {
			pr_err("Firmware bug, duplicate CPU MPIDR: 0x%llx in MADT\n",
			       mpidr);
			return;
		}

		bootcpu_valid = true;
	}

	/*
	 * Duplicate MPIDRs are a recipe for disaster. Scan
	 * all initialized entries and check for
	 * duplicates. If any is found just ignore the CPU.
	 */
	for (i = 1; i < enabled_cpus; i++) {
		if (cpu_logical_map(i) == mpidr) {
			pr_err("Firmware bug, duplicate CPU MPIDR: 0x%llx in MADT\n",
			       mpidr);
			return;
		}
	}

	if (!acpi_psci_present())
		return;

	cpu_ops[enabled_cpus] = cpu_get_ops("psci");
	/* CPU 0 was already initialized */
	if (enabled_cpus) {
		if (!cpu_ops[enabled_cpus])
			return;

		if (cpu_ops[enabled_cpus]->cpu_init(NULL, enabled_cpus))
			return;

		/* map the logical cpu id to cpu MPIDR */
		cpu_logical_map(enabled_cpus) = mpidr;
	}

	enabled_cpus++;
}

static int __init
acpi_parse_gic_cpu_interface(struct acpi_subtable_header *header,
				const unsigned long end)
{
	struct acpi_madt_generic_interrupt *processor;

	processor = (struct acpi_madt_generic_interrupt *)header;

	if (BAD_MADT_ENTRY(processor, end))
		return -EINVAL;

	acpi_table_print_madt_entry(header);
	acpi_map_gic_cpu_interface(processor);
	return 0;
}

/* Parse GIC cpu interface entries in MADT for SMP init */
void __init acpi_init_cpus(void)
{
	int count, i;

	/*
	 * do a partial walk of MADT to determine how many CPUs
	 * we have including disabled CPUs, and get information
	 * we need for SMP init
	 */
	count = acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_INTERRUPT,
			acpi_parse_gic_cpu_interface, 0);

	if (!count) {
		pr_err("No GIC CPU interface entries present\n");
		return;
	} else if (count < 0) {
		pr_err("Error parsing GIC CPU interface entry\n");
		return;
	}

	if (!bootcpu_valid) {
		pr_err("MADT missing boot CPU MPIDR, not enabling secondaries\n");
		return;
	}

	for (i = 0; i < enabled_cpus; i++)
		set_cpu_possible(i, true);

	/* Make boot-up look pretty */
	pr_info("%d CPUs enabled, %d CPUs total\n", enabled_cpus, total_cpus);
}

/*
 * acpi_fadt_sanity_check() - Check FADT presence and carry out sanity
 *			      checks on it
 *
 * Return 0 on success,  <0 on failure
 */
static int __init acpi_fadt_sanity_check(void)
{
	struct acpi_table_header *table;
	struct acpi_table_fadt *fadt;
	acpi_status status;
	acpi_size tbl_size;
	int ret = 0;

	/*
	 * FADT is required on arm64; retrieve it to check its presence
	 * and carry out revision and ACPI HW reduced compliancy tests
	 */
	status = acpi_get_table_with_size(ACPI_SIG_FADT, 0, &table, &tbl_size);
	if (ACPI_FAILURE(status)) {
		const char *msg = acpi_format_exception(status);

		pr_err("Failed to get FADT table, %s\n", msg);
		return -ENODEV;
	}

	fadt = (struct acpi_table_fadt *)table;

	/*
	 * Revision in table header is the FADT Major revision, and there
	 * is a minor revision of FADT which was introduced by ACPI 5.1,
	 * we only deal with ACPI 5.1 or newer revision to get GIC and SMP
	 * boot protocol configuration data.
	 */
	if (table->revision < 5 ||
	   (table->revision == 5 && fadt->minor_revision < 1)) {
		pr_err("Unsupported FADT revision %d.%d, should be 5.1+\n",
		       table->revision, fadt->minor_revision);
		ret = -EINVAL;
		goto out;
	}

	if (!(fadt->flags & ACPI_FADT_HW_REDUCED)) {
		pr_err("FADT not ACPI hardware reduced compliant\n");
		ret = -EINVAL;
	}

out:
	/*
	 * acpi_get_table_with_size() creates FADT table mapping that
	 * should be released after parsing and before resuming boot
	 */
	early_acpi_os_unmap_memory(table, tbl_size);
	return ret;
}

/*
 * acpi_boot_table_init() called from setup_arch(), always.
 *	1. find RSDP and get its address, and then find XSDT
 *	2. extract all tables and checksums them all
 *	3. check ACPI FADT revision
 *	4. check ACPI FADT HW reduced flag
 *
 * We can parse ACPI boot-time tables such as MADT after
 * this function is called.
 *
 * On return ACPI is enabled if either:
 *
 * - ACPI tables are initialized and sanity checks passed
 * - acpi=force was passed in the command line and ACPI was not disabled
 *   explicitly through acpi=off command line parameter
 *
 * ACPI is disabled on function return otherwise
 */
void __init acpi_boot_table_init(void)
{
	/*
	 * Enable ACPI instead of device tree unless
	 * - ACPI has been disabled explicitly (acpi=off), or
	 * - the device tree is not empty (it has more than just a /chosen node)
	 *   and ACPI has not been force enabled (acpi=force)
	 */
	if (param_acpi_off ||
	    (!param_acpi_force && of_scan_flat_dt(dt_scan_depth1_nodes, NULL)))
		return;

	/*
	 * ACPI is disabled at this point. Enable it in order to parse
	 * the ACPI tables and carry out sanity checks
	 */
	enable_acpi();

	/*
	 * If ACPI tables are initialized and FADT sanity checks passed,
	 * leave ACPI enabled and carry on booting; otherwise disable ACPI
	 * on initialization error.
	 * If acpi=force was passed on the command line it forces ACPI
	 * to be enabled even if its initialization failed.
	 */
	if (acpi_table_init() || acpi_fadt_sanity_check()) {
		pr_err("Failed to init ACPI tables\n");
		if (!param_acpi_force)
			disable_acpi();
	}
}

void __init acpi_gic_init(void)
{
	struct acpi_table_header *table;
	acpi_status status;
	acpi_size tbl_size;
	int err;

	if (acpi_disabled)
		return;

	status = acpi_get_table_with_size(ACPI_SIG_MADT, 0, &table, &tbl_size);
	if (ACPI_FAILURE(status)) {
		const char *msg = acpi_format_exception(status);

		pr_err("Failed to get MADT table, %s\n", msg);
		return;
	}

	err = gic_v2_acpi_init(table);
	if (err)
		pr_err("Failed to initialize GIC IRQ controller");

	early_acpi_os_unmap_memory((char *)table, tbl_size);
}
