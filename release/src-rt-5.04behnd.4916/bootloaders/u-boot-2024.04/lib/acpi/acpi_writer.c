// SPDX-License-Identifier: GPL-2.0+
/*
 * Handles writing the declared ACPI tables
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY LOGC_ACPI

#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <acpi/acpi_table.h>
#include <asm/global_data.h>
#include <dm/acpi.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

int acpi_write_one(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	int ret;

	log_debug("%s: writing table '%s'\n", entry->name,
		  entry->table);
	ctx->tab_start = ctx->current;
	ret = entry->h_write(ctx, entry);
	if (ret == -ENOENT) {
		log_debug("%s: Omitted due to being empty\n",
			  entry->name);
		ret = 0;
		ctx->current = ctx->tab_start;	/* drop the table */
		return ret;
	}
	if (ret)
		return log_msg_ret("write", ret);

	if (entry->flags & ACPIWF_ALIGN64)
		acpi_align64(ctx);
	else
		acpi_align(ctx);

	/* Add the item to the internal list */
	ret = acpi_add_other_item(ctx, entry, ctx->tab_start);
	if (ret)
		return log_msg_ret("add", ret);

	return 0;
}

#ifndef CONFIG_QFW_ACPI
static int acpi_write_all(struct acpi_ctx *ctx)
{
	const struct acpi_writer *writer =
		 ll_entry_start(struct acpi_writer, acpi_writer);
	const int n_ents = ll_entry_count(struct acpi_writer, acpi_writer);
	const struct acpi_writer *entry;
	int ret;

	for (entry = writer; entry != writer + n_ents; entry++) {
		ret = acpi_write_one(ctx, entry);
		if (ret && ret != -ENOENT)
			return log_msg_ret("one", ret);
	}

	return 0;
}

/*
 * QEMU's version of write_acpi_tables is defined in drivers/misc/qfw.c
 */
ulong write_acpi_tables(ulong start_addr)
{
	struct acpi_ctx *ctx;
	ulong addr;
	int ret;

	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		return log_msg_ret("mem", -ENOMEM);

	log_debug("ACPI: Writing ACPI tables at %lx\n", start_addr);

	acpi_reset_items();
	acpi_setup_ctx(ctx, start_addr);

	ret = acpi_write_all(ctx);
	if (ret) {
		log_err("Failed to write ACPI tables (err=%d)\n", ret);
		return log_msg_ret("write", -ENOMEM);
	}

	addr = map_to_sysmem(ctx->current);
	log_debug("ACPI current = %lx\n", addr);

	return addr;
}

int write_dev_tables(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	int ret;

	ret = acpi_write_dev_tables(ctx);
	if (ret)
		return log_msg_ret("write", ret);

	return 0;
}
ACPI_WRITER(8dev, NULL, write_dev_tables, 0);

ulong acpi_get_rsdp_addr(void)
{
	if (!gd->acpi_ctx)
		return 0;

	return map_to_sysmem(gd->acpi_ctx->rsdp);
}
#endif /* QFW_ACPI */

void acpi_setup_ctx(struct acpi_ctx *ctx, ulong start)
{
	gd->acpi_ctx = ctx;
	memset(ctx, '\0', sizeof(*ctx));

	/* Align ACPI tables to 16-byte boundary */
	start = ALIGN(start, 16);
	ctx->base = map_sysmem(start, 0);
	ctx->current = ctx->base;

	gd_set_acpi_start(start);
}
