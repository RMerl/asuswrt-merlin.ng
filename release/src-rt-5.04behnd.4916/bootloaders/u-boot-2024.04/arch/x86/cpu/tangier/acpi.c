// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Partially based on acpi.c for other x86 platforms
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <mapmem.h>
#include <acpi/acpi_table.h>
#include <asm/ioapic.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <asm/arch/global_nvs.h>
#include <asm/arch/iomap.h>
#include <dm/uclass-internal.h>

static int tangier_write_fadt(struct acpi_ctx *ctx,
			      const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_fadt *fadt;

	fadt = ctx->current;
	header = &fadt->header;

	memset(fadt, '\0', sizeof(struct acpi_fadt));

	acpi_fill_header(header, "FACP");
	header->length = sizeof(struct acpi_fadt);
	header->revision = 6;

	fadt->preferred_pm_profile = ACPI_PM_UNSPECIFIED;

	fadt->iapc_boot_arch = ACPI_FADT_VGA_NOT_PRESENT |
			       ACPI_FADT_NO_PCIE_ASPM_CONTROL;
	fadt->flags =
		ACPI_FADT_WBINVD |
		ACPI_FADT_POWER_BUTTON | ACPI_FADT_SLEEP_BUTTON |
		ACPI_FADT_SEALED_CASE | ACPI_FADT_HEADLESS |
		ACPI_FADT_HW_REDUCED_ACPI;

	fadt->minor_revision = 2;

	fadt->x_firmware_ctrl = map_to_sysmem(ctx->facs);
	fadt->x_dsdt = map_to_sysmem(ctx->dsdt);

	header->checksum = table_compute_checksum(fadt, header->length);

	return acpi_add_fadt(ctx, fadt);
}
ACPI_WRITER(5fadt, "FADT", tangier_write_fadt, 0);

u32 acpi_fill_madt(u32 current)
{
	current += acpi_create_madt_lapics(current);

	current += acpi_create_madt_ioapic((struct acpi_madt_ioapic *)current,
			io_apic_read(IO_APIC_ID) >> 24, IO_APIC_ADDR, 0);

	return current;
}

int acpi_fill_mcfg(struct acpi_ctx *ctx)
{
	size_t size;

	/* TODO: Derive parameters from SFI MCFG table */
	size = acpi_create_mcfg_mmconfig
		((struct acpi_mcfg_mmconfig *)ctx->current,
		MCFG_BASE_ADDRESS, 0x0, 0x0, 0x0);
	acpi_inc(ctx, size);

	return 0;
}

static u32 acpi_fill_csrt_dma(struct acpi_csrt_group *grp)
{
	struct acpi_csrt_shared_info *si = (struct acpi_csrt_shared_info *)&grp[1];

	/* Fill the Resource Group with Shared Information attached */
	memset(grp, 0, sizeof(*grp));
	grp->shared_info_length = sizeof(struct acpi_csrt_shared_info);
	grp->length = sizeof(struct acpi_csrt_group) + grp->shared_info_length;
	/* TODO: All values below should come from U-Boot DT somehow */
	sprintf((char *)&grp->vendor_id, "%04X", 0x8086);
	grp->device_id = 0x11a2;

	/* Fill the Resource Group Shared Information */
	memset(si, 0, sizeof(*si));
	si->major_version = 1;
	si->minor_version = 0;
	/* TODO: All values below should come from U-Boot DT somehow */
	si->mmio_base_low = 0xff192000;
	si->mmio_base_high = 0;
	si->gsi_interrupt = 32;
	si->interrupt_polarity = 0;	/* Active High */
	si->interrupt_mode = 0;		/* Level triggered */
	si->num_channels = 8;
	si->dma_address_width = 32;
	si->base_request_line = 0;
	si->num_handshake_signals = 16;
	si->max_block_size = 0x1ffff;

	return grp->length;
}

int acpi_fill_csrt(struct acpi_ctx *ctx)
{
	int size;

	size = acpi_fill_csrt_dma(ctx->current);
	acpi_inc(ctx, size);

	return 0;
}

int acpi_create_gnvs(struct acpi_global_nvs *gnvs)
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

	return 0;
}
