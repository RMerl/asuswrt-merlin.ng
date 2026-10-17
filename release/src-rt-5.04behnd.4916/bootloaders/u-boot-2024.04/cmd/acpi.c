// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */
#include <common.h>
#include <command.h>
#include <display_options.h>
#include <log.h>
#include <mapmem.h>
#include <acpi/acpi_table.h>
#include <asm/acpi_table.h>
#include <asm/global_data.h>
#include <dm/acpi.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * dump_hdr() - Dump an ACPI header
 *
 * Except for the Firmware ACPI Control Structure (FACS)
 * additionally show the revision information.
 *
 * @hdr: ACPI header to dump
 */
static void dump_hdr(struct acpi_table_header *hdr)
{
	bool has_hdr = memcmp(hdr->signature, "FACS", ACPI_NAME_LEN);

	printf("%.*s  %16lx  %5x", ACPI_NAME_LEN, hdr->signature,
	       (ulong)map_to_sysmem(hdr), hdr->length);
	if (has_hdr) {
		printf("  v%02d %.6s %.8s %x %.4s %x\n", hdr->revision,
		       hdr->oem_id, hdr->oem_table_id, hdr->oem_revision,
		       hdr->aslc_id, hdr->aslc_revision);
	} else {
		printf("\n");
	}
}

static int dump_table_name(const char *sig)
{
	struct acpi_table_header *hdr;

	hdr = acpi_find_table(sig);
	if (!hdr)
		return -ENOENT;
	printf("%.*s @ %16lx\n", ACPI_NAME_LEN, hdr->signature,
	       (ulong)nomap_to_sysmem(hdr));
	print_buffer(0, hdr, 1, hdr->length, 0);

	return 0;
}

static void list_fadt(struct acpi_fadt *fadt)
{
	if (fadt->header.revision >= 3 && fadt->x_dsdt)
		dump_hdr(nomap_sysmem(fadt->x_dsdt, 0));
	else if (fadt->dsdt)
		dump_hdr(nomap_sysmem(fadt->dsdt, 0));
	if (!IS_ENABLED(CONFIG_X86) &&
	    !(fadt->flags & ACPI_FADT_HW_REDUCED_ACPI))
		log_err("FADT not ACPI-hardware-reduced-compliant\n");
	if (fadt->header.revision >= 3 && fadt->x_firmware_ctrl)
		dump_hdr(nomap_sysmem(fadt->x_firmware_ctrl, 0));
	else if (fadt->firmware_ctrl)
		dump_hdr(nomap_sysmem(fadt->firmware_ctrl, 0));
}

static void list_rsdt(struct acpi_rsdp *rsdp)
{
	int len, i, count;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt;

	if (rsdp->rsdt_address) {
		rsdt = nomap_sysmem(rsdp->rsdt_address, 0);
		dump_hdr(&rsdt->header);
	}
	if (rsdp->xsdt_address) {
		xsdt = nomap_sysmem(rsdp->xsdt_address, 0);
		dump_hdr(&xsdt->header);
		len = xsdt->header.length - sizeof(xsdt->header);
		count = len / sizeof(u64);
	} else if (rsdp->rsdt_address) {
		len = rsdt->header.length - sizeof(rsdt->header);
		count = len / sizeof(u32);
	} else {
		return;
	}

	for (i = 0; i < count; i++) {
		struct acpi_table_header *hdr;
		u64 entry;

		if (rsdp->xsdt_address)
			entry = xsdt->entry[i];
		else
			entry = rsdt->entry[i];
		if (!entry)
			break;
		hdr = nomap_sysmem(entry, 0);
		dump_hdr(hdr);
		if (!memcmp(hdr->signature, "FACP", ACPI_NAME_LEN))
			list_fadt((struct acpi_fadt *)hdr);
	}
}

static void list_rsdp(struct acpi_rsdp *rsdp)
{
	printf("RSDP  %16lx  %5x  v%02d %.6s\n", (ulong)map_to_sysmem(rsdp),
	       rsdp->length, rsdp->revision, rsdp->oem_id);
	list_rsdt(rsdp);
}

static int do_acpi_list(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct acpi_rsdp *rsdp;

	rsdp = map_sysmem(gd_acpi_start(), 0);
	if (!rsdp) {
		printf("No ACPI tables present\n");
		return 0;
	}
	printf("Name              Base   Size  Detail\n"
	       "----  ----------------  -----  ----------------------------\n");
	list_rsdp(rsdp);

	return 0;
}

static int do_acpi_set(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	ulong val;

	if (argc < 2) {
		printf("ACPI pointer: %lx\n", gd_acpi_start());
	} else {
		val = hextoul(argv[1], NULL);
		printf("Setting ACPI pointer to %lx\n", val);
		gd_set_acpi_start(val);
	}

	return 0;
}

static int do_acpi_items(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	bool dump_contents;

	dump_contents = argc >= 2 && !strcmp("-d", argv[1]);
	if (!IS_ENABLED(CONFIG_ACPIGEN)) {
		printf("Not supported (enable ACPIGEN)\n");
		return CMD_RET_FAILURE;
	}
	acpi_dump_items(dump_contents ? ACPI_DUMP_CONTENTS : ACPI_DUMP_LIST);

	return 0;
}

static int do_acpi_dump(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	const char *name;
	char sig[ACPI_NAME_LEN];
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	name = argv[1];
	if (strlen(name) != ACPI_NAME_LEN) {
		printf("Table name '%s' must be four characters\n", name);
		return CMD_RET_FAILURE;
	}
	str_to_upper(name, sig, ACPI_NAME_LEN);
	ret = dump_table_name(sig);
	if (ret) {
		printf("Table '%.*s' not found\n", ACPI_NAME_LEN, sig);
		return CMD_RET_FAILURE;
	}

	return 0;
}

U_BOOT_LONGHELP(acpi,
	"list  - list ACPI tables\n"
	"acpi items [-d]   - List/dump each piece of ACPI data from devices\n"
	"acpi set [<addr>] - Set or show address of ACPI tables\n"
	"acpi dump <name>  - Dump ACPI table");

U_BOOT_CMD_WITH_SUBCMDS(acpi, "ACPI tables", acpi_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_acpi_list),
	U_BOOT_SUBCMD_MKENT(items, 2, 1, do_acpi_items),
	U_BOOT_SUBCMD_MKENT(set, 2, 1, do_acpi_set),
	U_BOOT_SUBCMD_MKENT(dump, 2, 1, do_acpi_dump));
