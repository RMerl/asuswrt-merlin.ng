// SPDX-License-Identifier: GPL-2.0+
/*
 * Write the ACPI Differentiated System Description Table (DSDT)
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY LOGC_ACPI

#include <acpi/acpi_table.h>
#include <dm/acpi.h>
#include <tables_csum.h>
#include <linux/string.h>

/*
 * IASL compiles the dsdt entries and writes the hex values
 * to a C array AmlCode[] (see dsdt.c).
 */
extern const unsigned char AmlCode[];

int acpi_write_dsdt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	const int thl = sizeof(struct acpi_table_header);
	struct acpi_table_header *dsdt = ctx->current;
	int aml_len;

	/* Put the table header first */
	memcpy(dsdt, &AmlCode, thl);
	acpi_inc(ctx, thl);
	log_debug("DSDT starts at %p, hdr ends at %p\n", dsdt, ctx->current);

	/* If the table is not empty, allow devices to inject things */
	aml_len = dsdt->length - thl;
	if (aml_len) {
		void *base = ctx->current;
		int ret;

		ret = acpi_inject_dsdt(ctx);
		if (ret)
			return log_msg_ret("inject", ret);
		log_debug("Added %lx bytes from inject_dsdt, now at %p\n",
			  (ulong)(ctx->current - base), ctx->current);
		log_debug("Copy AML code size %x to %p\n", aml_len,
			  ctx->current);
		memcpy(ctx->current, AmlCode + thl, aml_len);
		acpi_inc(ctx, aml_len);
	}

	ctx->dsdt = dsdt;
	dsdt->length = ctx->current - (void *)dsdt;
	log_debug("Updated DSDT length to %x\n", dsdt->length);

	return 0;
}
ACPI_WRITER(3dsdt, "DSDT", acpi_write_dsdt, 0);
