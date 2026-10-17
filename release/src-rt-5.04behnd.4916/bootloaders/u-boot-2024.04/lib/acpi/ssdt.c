// SPDX-License-Identifier: GPL-2.0+
/*
 * Write an ACPI Secondary System Descriptor Table (SSDT) table
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY LOGC_ACPI

#include <acpi/acpi_table.h>
#include <dm/acpi.h>
#include <tables_csum.h>
#include <linux/errno.h>
#include <linux/string.h>

int acpi_write_ssdt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *ssdt;
	int ret;

	ssdt = ctx->current;
	memset(ssdt, '\0', sizeof(struct acpi_table_header));

	acpi_fill_header(ssdt, "SSDT");
	ssdt->revision = acpi_get_table_revision(ACPITAB_SSDT);
	ssdt->aslc_revision = 1;
	ssdt->length = sizeof(struct acpi_table_header);

	acpi_inc(ctx, sizeof(struct acpi_table_header));

	ret = acpi_fill_ssdt(ctx);
	if (ret) {
		ctx->current = ssdt;
		return log_msg_ret("fill", ret);
	}

	/* (Re)calculate length and checksum */
	ssdt->length = ctx->current - (void *)ssdt;
	ssdt->checksum = table_compute_checksum((void *)ssdt, ssdt->length);
	log_debug("SSDT at %p, length %x\n", ssdt, ssdt->length);

	/* Drop the table if it is empty */
	if (ssdt->length == sizeof(struct acpi_table_header))
		return log_msg_ret("fill", -ENOENT);
	acpi_add_table(ctx, ssdt);

	return 0;
}
ACPI_WRITER(6ssdt, "SSDT", acpi_write_ssdt, 0);
