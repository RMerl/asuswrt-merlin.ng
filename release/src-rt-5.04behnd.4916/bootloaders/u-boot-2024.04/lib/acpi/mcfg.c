// SPDX-License-Identifier: GPL-2.0+
/*
 * Write an ACPI MCFG table
 *
 * Copyright 2022 Google LLC
 */

#define LOG_CATEGORY LOGC_ACPI

#include <mapmem.h>
#include <tables_csum.h>
#include <acpi/acpi_table.h>
#include <dm/acpi.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>

int acpi_create_mcfg_mmconfig(struct acpi_mcfg_mmconfig *mmconfig, u32 base,
			      u16 seg_nr, u8 start, u8 end)
{
	memset(mmconfig, 0, sizeof(*mmconfig));
	mmconfig->base_address_l = base;
	mmconfig->base_address_h = 0;
	mmconfig->pci_segment_group_number = seg_nr;
	mmconfig->start_bus_number = start;
	mmconfig->end_bus_number = end;

	return sizeof(struct acpi_mcfg_mmconfig);
}

__weak int acpi_fill_mcfg(struct acpi_ctx *ctx)
{
	return -ENOENT;
}

/* MCFG is defined in the PCI Firmware Specification 3.0 */
int acpi_write_mcfg(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_mcfg *mcfg;
	int ret;

	mcfg = ctx->current;
	header = &mcfg->header;

	memset(mcfg, '\0', sizeof(struct acpi_mcfg));

	/* Fill out header fields */
	acpi_fill_header(header, "MCFG");
	header->length = sizeof(struct acpi_mcfg);
	header->revision = 1;
	acpi_inc(ctx, sizeof(*mcfg));

	ret = acpi_fill_mcfg(ctx);
	if (ret)
		return log_msg_ret("fill", ret);

	/* (Re)calculate length and checksum */
	header->length = (ulong)ctx->current - (ulong)mcfg;
	header->checksum = table_compute_checksum(mcfg, header->length);

	acpi_add_table(ctx, mcfg);

	return 0;
}
ACPI_WRITER(5mcfg, "MCFG", acpi_write_mcfg, 0);
