// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Partially based on acpi.c for other x86 platforms
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <asm/acpi_table.h>
#include <asm/ioapic.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <asm/arch/global_nvs.h>

void acpi_create_fadt(struct acpi_fadt *fadt, struct acpi_facs *facs,
		      void *dsdt)
{
	struct acpi_table_header *header = &(fadt->header);

	memset((void *)fadt, 0, sizeof(struct acpi_fadt));

	acpi_fill_header(header, "FACP");
	header->length = sizeof(struct acpi_fadt);
	header->revision = 6;

	fadt->firmware_ctrl = (u32)facs;
	fadt->dsdt = (u32)dsdt;
	fadt->preferred_pm_profile = ACPI_PM_UNSPECIFIED;

	fadt->iapc_boot_arch = ACPI_FADT_VGA_NOT_PRESENT |
			       ACPI_FADT_NO_PCIE_ASPM_CONTROL;
	fadt->flags =
		ACPI_FADT_WBINVD |
		ACPI_FADT_POWER_BUTTON | ACPI_FADT_SLEEP_BUTTON |
		ACPI_FADT_SEALED_CASE | ACPI_FADT_HEADLESS |
		ACPI_FADT_HW_REDUCED_ACPI;

	fadt->minor_revision = 2;

	fadt->x_firmware_ctl_l = (u32)facs;
	fadt->x_firmware_ctl_h = 0;
	fadt->x_dsdt_l = (u32)dsdt;
	fadt->x_dsdt_h = 0;

	header->checksum = table_compute_checksum(fadt, header->length);
}

u32 acpi_fill_madt(u32 current)
{
	current += acpi_create_madt_lapics(current);

	current += acpi_create_madt_ioapic((struct acpi_madt_ioapic *)current,
			io_apic_read(IO_APIC_ID) >> 24, IO_APIC_ADDR, 0);

	return current;
}

u32 acpi_fill_mcfg(u32 current)
{
	/* TODO: Derive parameters from SFI MCFG table */
	current += acpi_create_mcfg_mmconfig
		((struct acpi_mcfg_mmconfig *)current,
		0x3f500000, 0x0, 0x0, 0x0);

	return current;
}

void acpi_create_gnvs(struct acpi_global_nvs *gnvs)
{
	struct udevice *dev;
	int ret;

	/* at least we have one processor */
	gnvs->pcnt = 1;

	/* override the processor count with actual number */
	ret = uclass_find_first_device(UCLASS_CPU, &dev);
	if (ret == 0 && dev != NULL) {
		ret = cpu_get_count(dev);
		if (ret > 0)
			gnvs->pcnt = ret;
	}
}
