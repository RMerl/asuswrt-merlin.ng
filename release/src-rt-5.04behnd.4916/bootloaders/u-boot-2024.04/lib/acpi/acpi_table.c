// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic code used to generate ACPI tables
 *
 * Copyright 2019 Google LLC
 */

#include <dm.h>
#include <cpu.h>
#include <log.h>
#include <mapmem.h>
#include <tables_csum.h>
#include <version_string.h>
#include <acpi/acpi_table.h>
#include <asm/global_data.h>
#include <dm/acpi.h>

/*
 * OEM_REVISION is 32-bit unsigned number. It should be increased only when
 * changing software version. Therefore it should not depend on build time.
 * U-Boot calculates it from U-Boot version and represent it in hexadecimal
 * notation. As U-Boot version is in form year.month set low 8 bits to 0x01
 * to have valid date. So for U-Boot version 2021.04 OEM_REVISION is set to
 * value 0x20210401.
 */
#define OEM_REVISION ((((version_num / 1000) % 10) << 28) | \
		      (((version_num / 100) % 10) << 24) | \
		      (((version_num / 10) % 10) << 20) | \
		      ((version_num % 10) << 16) | \
		      (((version_num_patch / 10) % 10) << 12) | \
		      ((version_num_patch % 10) << 8) | \
		      0x01)

int acpi_create_dmar(struct acpi_dmar *dmar, enum dmar_flags flags)
{
	struct acpi_table_header *header = &dmar->header;
	struct cpu_info info;
	struct udevice *cpu;
	int ret;

	ret = uclass_first_device_err(UCLASS_CPU, &cpu);
	if (ret)
		return log_msg_ret("cpu", ret);
	ret = cpu_get_info(cpu, &info);
	if (ret)
		return log_msg_ret("info", ret);
	memset((void *)dmar, 0, sizeof(struct acpi_dmar));

	/* Fill out header fields. */
	acpi_fill_header(&dmar->header, "DMAR");
	header->length = sizeof(struct acpi_dmar);
	header->revision = acpi_get_table_revision(ACPITAB_DMAR);

	dmar->host_address_width = info.address_width - 1;
	dmar->flags = flags;

	return 0;
}

int acpi_get_table_revision(enum acpi_tables table)
{
	switch (table) {
	case ACPITAB_FADT:
		return ACPI_FADT_REV_ACPI_3_0;
	case ACPITAB_MADT:
		return ACPI_MADT_REV_ACPI_3_0;
	case ACPITAB_MCFG:
		return ACPI_MCFG_REV_ACPI_3_0;
	case ACPITAB_TCPA:
		/* This version and the rest are open-coded */
		return 2;
	case ACPITAB_TPM2:
		return 4;
	case ACPITAB_SSDT: /* ACPI 3.0 upto 6.3: 2 */
		return 2;
	case ACPITAB_SRAT: /* ACPI 2.0: 1, ACPI 3.0: 2, ACPI 4.0 to 6.3: 3 */
		return 1; /* TODO Should probably be upgraded to 2 */
	case ACPITAB_DMAR:
		return 1;
	case ACPITAB_SLIT: /* ACPI 2.0 upto 6.3: 1 */
		return 1;
	case ACPITAB_SPMI: /* IMPI 2.0 */
		return 5;
	case ACPITAB_HPET: /* Currently 1. Table added in ACPI 2.0 */
		return 1;
	case ACPITAB_VFCT: /* ACPI 2.0/3.0/4.0: 1 */
		return 1;
	case ACPITAB_IVRS:
		return IVRS_FORMAT_FIXED;
	case ACPITAB_DBG2:
		return 0;
	case ACPITAB_FACS: /* ACPI 2.0/3.0: 1, ACPI 4.0 to 6.3: 2 */
		return 1;
	case ACPITAB_RSDT: /* ACPI 1.0 upto 6.3: 1 */
		return 1;
	case ACPITAB_XSDT: /* ACPI 2.0 upto 6.3: 1 */
		return 1;
	case ACPITAB_RSDP: /* ACPI 2.0 upto 6.3: 2 */
		return 2;
	case ACPITAB_HEST:
		return 1;
	case ACPITAB_NHLT:
		return 5;
	case ACPITAB_BERT:
		return 1;
	case ACPITAB_SPCR:
		return 2;
	default:
		return -EINVAL;
	}
}

void acpi_fill_header(struct acpi_table_header *header, char *signature)
{
	memcpy(header->signature, signature, 4);
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, OEM_TABLE_ID, 8);
	header->oem_revision = OEM_REVISION;
	memcpy(header->aslc_id, ASLC_ID, 4);
}

void acpi_align(struct acpi_ctx *ctx)
{
	ctx->current = (void *)ALIGN((ulong)ctx->current, 16);
}

void acpi_align64(struct acpi_ctx *ctx)
{
	ctx->current = (void *)ALIGN((ulong)ctx->current, 64);
}

void acpi_inc(struct acpi_ctx *ctx, uint amount)
{
	ctx->current += amount;
}

void acpi_inc_align(struct acpi_ctx *ctx, uint amount)
{
	ctx->current += amount;
	acpi_align(ctx);
}

/**
 * Add an ACPI table to the RSDT (and XSDT) structure, recalculate length
 * and checksum.
 */
int acpi_add_table(struct acpi_ctx *ctx, void *table)
{
	int i, entries_num;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt;

	/* The RSDT is mandatory while the XSDT is not */
	rsdt = ctx->rsdt;

	/* This should always be MAX_ACPI_TABLES */
	entries_num = ARRAY_SIZE(rsdt->entry);

	for (i = 0; i < entries_num; i++) {
		if (rsdt->entry[i] == 0)
			break;
	}

	if (i >= entries_num) {
		log_err("ACPI: Error: too many tables\n");
		return -E2BIG;
	}

	/* Add table to the RSDT */
	rsdt->entry[i] = nomap_to_sysmem(table);

	/* Fix RSDT length or the kernel will assume invalid entries */
	rsdt->header.length = sizeof(struct acpi_table_header) +
				(sizeof(u32) * (i + 1));

	/* Re-calculate checksum */
	rsdt->header.checksum = 0;
	rsdt->header.checksum = table_compute_checksum((u8 *)rsdt,
						       rsdt->header.length);

	/*
	 * And now the same thing for the XSDT. We use the same index as for
	 * now we want the XSDT and RSDT to always be in sync in U-Boot
	 */
	xsdt = ctx->xsdt;

	/* Add table to the XSDT */
	xsdt->entry[i] = nomap_to_sysmem(table);

	/* Fix XSDT length */
	xsdt->header.length = sizeof(struct acpi_table_header) +
				(sizeof(u64) * (i + 1));

	/* Re-calculate checksum */
	xsdt->header.checksum = 0;
	xsdt->header.checksum = table_compute_checksum((u8 *)xsdt,
						       xsdt->header.length);

	return 0;
}

void acpi_create_dbg2(struct acpi_dbg2_header *dbg2,
		      int port_type, int port_subtype,
		      struct acpi_gen_regaddr *address, u32 address_size,
		      const char *device_path)
{
	uintptr_t current;
	struct acpi_dbg2_device *device;
	u32 *dbg2_addr_size;
	struct acpi_table_header *header;
	size_t path_len;
	const char *path;
	char *namespace;

	/* Fill out header fields. */
	current = (uintptr_t)dbg2;
	memset(dbg2, '\0', sizeof(struct acpi_dbg2_header));
	header = &dbg2->header;

	header->revision = acpi_get_table_revision(ACPITAB_DBG2);
	acpi_fill_header(header, "DBG2");
	header->aslc_revision = ASL_REVISION;

	/* One debug device defined */
	dbg2->devices_offset = sizeof(struct acpi_dbg2_header);
	dbg2->devices_count = 1;
	current += sizeof(struct acpi_dbg2_header);

	/* Device comes after the header */
	device = (struct acpi_dbg2_device *)current;
	memset(device, 0, sizeof(struct acpi_dbg2_device));
	current += sizeof(struct acpi_dbg2_device);

	device->revision = 0;
	device->address_count = 1;
	device->port_type = port_type;
	device->port_subtype = port_subtype;

	/* Base Address comes after device structure */
	memcpy((void *)current, address, sizeof(struct acpi_gen_regaddr));
	device->base_address_offset = current - (uintptr_t)device;
	current += sizeof(struct acpi_gen_regaddr);

	/* Address Size comes after address structure */
	dbg2_addr_size = (uint32_t *)current;
	device->address_size_offset = current - (uintptr_t)device;
	*dbg2_addr_size = address_size;
	current += sizeof(uint32_t);

	/* Namespace string comes last, use '.' if not provided */
	path = device_path ? : ".";
	/* Namespace string length includes NULL terminator */
	path_len = strlen(path) + 1;
	namespace = (char *)current;
	device->namespace_string_length = path_len;
	device->namespace_string_offset = current - (uintptr_t)device;
	strncpy(namespace, path, path_len);
	current += path_len;

	/* Update structure lengths and checksum */
	device->length = current - (uintptr_t)device;
	header->length = current - (uintptr_t)dbg2;
	header->checksum = table_compute_checksum(dbg2, header->length);
}
