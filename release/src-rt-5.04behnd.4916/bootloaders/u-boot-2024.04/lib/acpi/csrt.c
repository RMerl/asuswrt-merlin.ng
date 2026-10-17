// SPDX-License-Identifier: GPL-2.0+
/*
 * Write an ACPI Core System Resource Table (CSRT)
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY LOGC_ACPI

#include <mapmem.h>
#include <tables_csum.h>
#include <acpi/acpi_table.h>
#include <dm/acpi.h>
#include <linux/string.h>

__weak int acpi_fill_csrt(struct acpi_ctx *ctx)
{
	return 0;
}

int acpi_write_csrt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_csrt *csrt;
	int ret;

	csrt = ctx->current;
	header = &csrt->header;

	memset(csrt, '\0', sizeof(struct acpi_csrt));

	/* Fill out header fields */
	acpi_fill_header(header, "CSRT");
	header->revision = 0;
	acpi_inc(ctx, sizeof(*header));

	ret = acpi_fill_csrt(ctx);
	if (ret)
		return log_msg_ret("fill", ret);

	/* (Re)calculate length and checksum */
	header->length = (ulong)ctx->current - (ulong)csrt;
	header->checksum = table_compute_checksum(csrt, header->length);

	acpi_add_table(ctx, csrt);

	return 0;
}
ACPI_WRITER(5csrt, "CSRT", acpi_write_csrt, 0);
