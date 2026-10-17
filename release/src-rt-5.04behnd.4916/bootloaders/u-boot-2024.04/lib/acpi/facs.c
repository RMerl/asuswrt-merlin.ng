// SPDX-License-Identifier: GPL-2.0+
/*
 * Write an ACPI Firmware ACPI Control Structure (FACS) table
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY LOGC_ACPI

#include <acpi/acpi_table.h>
#include <dm/acpi.h>
#include <linux/string.h>

int acpi_write_facs(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_facs *facs = ctx->current;

	memset((void *)facs, '\0', sizeof(struct acpi_facs));

	memcpy(facs->signature, "FACS", 4);
	facs->length = sizeof(struct acpi_facs);
	facs->hardware_signature = 0;
	facs->firmware_waking_vector = 0;
	facs->global_lock = 0;
	facs->flags = 0;
	facs->x_firmware_waking_vector_l = 0;
	facs->x_firmware_waking_vector_h = 0;
	facs->version = 1;

	ctx->facs = facs;
	acpi_inc(ctx, sizeof(struct acpi_facs));

	return 0;
}
ACPI_WRITER(1facs, "FACS", acpi_write_facs, 0);
