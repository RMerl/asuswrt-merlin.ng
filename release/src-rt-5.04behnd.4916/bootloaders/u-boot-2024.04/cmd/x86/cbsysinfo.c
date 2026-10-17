// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <asm/cb_sysinfo.h>
#include <command.h>
#include <console.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static void cbprompt(const char *name)
{
	for (; *name == '>'; name++)
		puts("   ");
	printf("%-12s: ", name);
}

static void print_dec(const char *name, int value)
{
	cbprompt(name);
	printf(value > 9 ? "0d%d\n" : "%d\n", value);
}

static void print_hex(const char *name, int value)
{
	cbprompt(name);
	printf("%x\n", value);
}

static void print_addr(const char *name, ulong value)
{
	cbprompt(name);
	printf("%08lx\n", value);
}

static void print_addr64(const char *name, u64 value)
{
	cbprompt(name);
	printf("%16llx\n", value);
}

static void print_ptr(const char *name, const void *value)
{
	cbprompt(name);
	printf("%p\n", value);
}

static void print_str(const char *name, const char *value)
{
	if (value) {
		cbprompt(name);
		printf("%s\n", value);
	}
}

static void print_idx(const char *name, uint idx, const u8 *strings)
{
	const char *ptr;

	cbprompt(name);
	ptr = (char *)strings + idx;
	printf("%d: %s\n", idx, ptr ? ptr : "(unknown)");
}

static const char *const cb_mem_name[] = {
	NULL,
	"ram",
	"reserved",
	"acpi",
	"nvs",
	"unusable",
	"vendor",
};

static const char *get_mem_name(int tag)
{
	if (tag >= CB_MEM_RAM && tag <= CB_MEM_VENDOR_RSVD)
		return cb_mem_name[tag];

	if (tag == CB_MEM_TABLE)
		return "table";

	return "(unknown)";
}

static const struct timestamp_id_to_name {
	uint id;
	const char *name;
} timestamp_ids[] = {
	/* Marker to report base_time */
	{ 0,			"1st timestamp" },
	{ TS_START_ROMSTAGE,	"start of romstage" },
	{ TS_BEFORE_INITRAM,	"before ram initialization" },
	{ TS_AFTER_INITRAM,	"after ram initialization" },
	{ TS_END_ROMSTAGE,	"end of romstage" },
	{ TS_START_VBOOT,	"start of verified boot" },
	{ TS_END_VBOOT,		"end of verified boot" },
	{ TS_START_COPYRAM,	"starting to load ramstage" },
	{ TS_END_COPYRAM,	"finished loading ramstage" },
	{ TS_START_RAMSTAGE,	"start of ramstage" },
	{ TS_START_BOOTBLOCK,	"start of bootblock" },
	{ TS_END_BOOTBLOCK,	"end of bootblock" },
	{ TS_START_COPYROM,	"starting to load romstage" },
	{ TS_END_COPYROM,	"finished loading romstage" },
	{ TS_START_ULZMA,	"starting LZMA decompress (ignore for x86)" },
	{ TS_END_ULZMA,		"finished LZMA decompress (ignore for x86)" },
	{ TS_START_ULZ4F,	"starting LZ4 decompress (ignore for x86)" },
	{ TS_END_ULZ4F,		"finished LZ4 decompress (ignore for x86)" },
	{ TS_DEVICE_ENUMERATE,	"device enumeration" },
	{ TS_DEVICE_CONFIGURE,	"device configuration" },
	{ TS_DEVICE_ENABLE,	"device enable" },
	{ TS_DEVICE_INITIALIZE,	"device initialization" },
	{ TS_DEVICE_DONE,	"device setup done" },
	{ TS_CBMEM_POST,	"cbmem post" },
	{ TS_WRITE_TABLES,	"write tables" },
	{ TS_FINALIZE_CHIPS,	"finalize chips" },
	{ TS_LOAD_PAYLOAD,	"load payload" },
	{ TS_ACPI_WAKE_JUMP,	"ACPI wake jump" },
	{ TS_SELFBOOT_JUMP,	"selfboot jump" },

	{ TS_START_COPYVER,	"starting to load verstage" },
	{ TS_END_COPYVER,	"finished loading verstage" },
	{ TS_START_TPMINIT,	"starting to initialize TPM" },
	{ TS_END_TPMINIT,	"finished TPM initialization" },
	{ TS_START_VERIFY_SLOT,	"starting to verify keyblock/preamble (RSA)" },
	{ TS_END_VERIFY_SLOT,	"finished verifying keyblock/preamble (RSA)" },
	{ TS_START_HASH_BODY,	"starting to verify body (load+SHA2+RSA) " },
	{ TS_DONE_LOADING,	"finished loading body (ignore for x86)" },
	{ TS_DONE_HASHING,	"finished calculating body hash (SHA2)" },
	{ TS_END_HASH_BODY,	"finished verifying body signature (RSA)" },

	{ TS_START_COPYVPD,	"starting to load Chrome OS VPD" },
	{ TS_END_COPYVPD_RO,	"finished loading Chrome OS VPD (RO)" },
	{ TS_END_COPYVPD_RW,	"finished loading Chrome OS VPD (RW)" },

	{ TS_U_BOOT_INITTED,	"U-Boot start" },
	{ TS_RO_PARAMS_INIT,	"RO parameter init" },
	{ TS_RO_VB_INIT,	"RO vboot init" },
	{ TS_RO_VB_SELECT_FIRMWARE,		"RO vboot select firmware" },
	{ TS_RO_VB_SELECT_AND_LOAD_KERNEL,	"RO vboot select&load kernel" },
	{ TS_RW_VB_SELECT_AND_LOAD_KERNEL,	"RW vboot select&load kernel" },
	{ TS_VB_SELECT_AND_LOAD_KERNEL,		"vboot select&load kernel" },
	{ TS_VB_EC_VBOOT_DONE,	"finished EC verification" },
	{ TS_VB_STORAGE_INIT_DONE, "finished storage device initialization" },
	{ TS_VB_READ_KERNEL_DONE, "finished reading kernel from disk" },
	{ TS_VB_VBOOT_DONE,	"finished vboot kernel verification" },
	{ TS_KERNEL_DECOMPRESSION, "starting kernel decompression/relocation" },
	{ TS_START_KERNEL,	"jumping to kernel" },
	{ TS_U_BOOT_START_KERNEL,	"just before jump to kernel" },

	/* Intel ME-related timestamps */
	{ TS_ME_INFORM_DRAM_WAIT, "waiting for ME acknowledgment of raminit"},
	{ TS_ME_INFORM_DRAM_DONE, "finished waiting for ME response"},

	/* FSP-related timestamps */
	{ TS_FSP_MEMORY_INIT_START, "calling FspMemoryInit" },
	{ TS_FSP_MEMORY_INIT_END, "returning from FspMemoryInit" },
	{ TS_FSP_TEMP_RAM_EXIT_START, "calling FspTempRamExit" },
	{ TS_FSP_TEMP_RAM_EXIT_END, "returning from FspTempRamExit" },
	{ TS_FSP_SILICON_INIT_START, "calling FspSiliconInit" },
	{ TS_FSP_SILICON_INIT_END, "returning from FspSiliconInit" },
	{ TS_FSP_BEFORE_ENUMERATE, "calling FspNotify(AfterPciEnumeration)" },
	{ TS_FSP_AFTER_ENUMERATE,
		 "returning from FspNotify(AfterPciEnumeration)" },
	{ TS_FSP_BEFORE_FINALIZE, "calling FspNotify(ReadyToBoot)" },
	{ TS_FSP_AFTER_FINALIZE, "returning from FspNotify(ReadyToBoot)" },
	{ TS_FSP_BEFORE_END_OF_FIRMWARE, "calling FspNotify(EndOfFirmware)" },
	{ TS_FSP_AFTER_END_OF_FIRMWARE,
		"returning from FspNotify(EndOfFirmware)" },
};

static const char *timestamp_name(uint32_t id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(timestamp_ids); i++) {
		if (timestamp_ids[i].id == id)
			return timestamp_ids[i].name;
	}

	return "<unknown>";
}

static void show_table(struct sysinfo_t *info, bool verbose)
{
	struct cb_serial *ser = info->serial;
	int i;

	printf("Coreboot table at %lx, size %x, records %x (dec %d), decoded to %p",
	       gd->arch.coreboot_table, info->table_size, info->rec_count,
	       info->rec_count, info);
	if (info->header)
		printf(", forwarded to %p\n", info->header);
	printf("\n");

	print_dec("CPU KHz", info->cpu_khz);

	print_addr("Serial I/O port", info->ser_ioport);
	print_addr(">base", info->ser_base);
	print_ptr(">pointer", ser);
	if (ser) {
		print_hex(">type", ser->type);
		print_addr(">base", ser->baseaddr);
		print_dec(">baud", ser->baud);
		print_hex(">regwidth", ser->regwidth);
		print_dec(">input_hz", ser->input_hertz);
		print_addr(">PCI addr", ser->uart_pci_addr);
	}

	print_dec("Mem ranges", info->n_memranges);
	printf("%12s: %-11s        ||   base        ||   size\n", "id", "type");
	for (i = 0; i < info->n_memranges; i++) {
		const struct memrange *mr = &info->memrange[i];

		printf("%12d: %02x:%-8s %016llx %016llx\n", i, mr->type,
		       get_mem_name(mr->type), mr->base, mr->size);
	}
	print_ptr("option_table", info->option_table);

	print_hex("CMOS start", info->cmos_range_start);
	if (info->cmos_range_start) {
		print_hex(">CMOS end", info->cmos_range_end);
		print_hex(">CMOS csum loc", info->cmos_checksum_location);
	}

	print_hex("VBNV start", info->vbnv_start);
	print_hex("VBNV size", info->vbnv_size);

	print_str("CB version", info->cb_version);
	print_str(">Extra", info->extra_version);
	print_str(">Build", info->build);
	print_str(">Time", info->compile_time);
	print_str(">By", info->compile_by);
	print_str(">Host", info->compile_host);
	print_str(">Domain", info->compile_domain);
	print_str(">Compiler", info->compiler);
	print_str(">Linker", info->linker);
	print_str(">Assembler", info->assembler);

	print_ptr("Framebuffer", info->framebuffer);
	if (info->framebuffer) {
		struct cb_framebuffer *fb = info->framebuffer;

		print_addr64(">Phys addr", fb->physical_address);
		print_dec(">X res", fb->x_resolution);
		print_dec(">X res", fb->y_resolution);
		print_hex(">Bytes / line", fb->bytes_per_line);
		print_dec(">Bpp", fb->bits_per_pixel);
		printf("   %-12s  red %d/%d, green %d/%d, blue %d/%d, reserved %d/%d\n",
		       "pos/size", fb->red_mask_pos, fb->red_mask_size,
		       fb->green_mask_pos, fb->green_mask_size,
		       fb->blue_mask_pos, fb->blue_mask_size,
		       fb->reserved_mask_pos, fb->reserved_mask_size);
	}

	print_dec("GPIOs", info->num_gpios);
	printf("%12s: %4s %12s %3s %s\n", "id", "port", "polarity", "val",
	       "name");
	for (i = 0; i < info->num_gpios; i++) {
		const struct cb_gpio *gpio = &info->gpios[i];
		char portstr[4];

		if (gpio->port == 0xffffffff)
			strcpy(portstr, "-");
		else
			sprintf(portstr, "%x", gpio->port);
		printf("%12d: %4s %12s %3d %s\n", i, portstr,
		       gpio->polarity == CB_GPIO_ACTIVE_LOW ? "active-low" :
		       "active-high", gpio->value, gpio->name);
	}
	print_dec("MACs", info->num_macs);
	for (i = 0; i < info->num_macs; i++) {
		const struct mac_address *mac = &info->macs[i];
		int j;

		printf("%12d: ", i);
		for (j = 0; j < sizeof(mac->mac_addr); j++)
			printf("%s%02x", j ? ":" : "", mac->mac_addr[j]);
		printf("\n");
	}
	print_str(">Serial #", info->serialno);
	print_ptr("Multiboot tab", info->mbtable);
	print_ptr("CB header", info->header);
	print_ptr("CB mainboard", info->mainboard);
	if (info->mainboard) {
		struct cb_mainboard *mb = info->mainboard;

		print_idx(">vendor", mb->vendor_idx, mb->strings);
		print_idx(">part_number", mb->part_number_idx, mb->strings);
	}
	print_ptr("vboot handoff", info->vboot_handoff);
	print_hex(">size", info->vboot_handoff_size);
	print_ptr(">vdat addr", info->vdat_addr);
	print_hex(">size", info->vdat_size);

	print_addr64("SMBIOS", info->smbios_start);
	print_hex(">size", info->smbios_size);
	print_hex("ROM MTRR", info->x86_rom_var_mtrr_index);

	print_ptr("Tstamp table", info->tstamp_table);
	if (verbose && info->tstamp_table) {
		struct timestamp_table *ts = info->tstamp_table;

		printf("%-12s", "Base_time");
		print_grouped_ull(ts->base_time, 12);
		printf("\n");
		print_dec("Tick MHz", ts->tick_freq_mhz);
		for (i = 0; i < ts->num_entries; i++) {
			const struct timestamp_entry *tse;

			tse = &ts->entries[i];
			printf("   ");
			print_grouped_ull(tse->entry_stamp, 12);
			printf("  %s\n", timestamp_name(tse->entry_id));
		}
	}

	print_ptr("CBmem cons", info->cbmem_cons);
	if (info->cbmem_cons) {
		struct cbmem_console *cons = info->cbmem_cons;
		int i;

		print_hex("Size", cons->size);
		print_hex("Cursor", cons->cursor);
		if (verbose) {
			for (i = 0; i < cons->cursor; i++) {
				int ch = cons->body[i];

				putc(ch);

				if (ch == '\n') {
					/* check for ctrl-c to abort... */
					if (ctrlc()) {
						puts("Abort\n");
						return;
					}
					printf("   ");
				}
			}
			printf("\n");
		}
	}

	print_ptr("MRC cache", info->mrc_cache);
	print_ptr("ACPI GNVS", info->acpi_gnvs);
	print_hex("Board ID", info->board_id);
	print_hex("RAM code", info->ram_code);
	print_ptr("WiFi calib", info->wifi_calibration);
	print_addr64("Ramoops buff", info->ramoops_buffer);
	print_hex(">size", info->ramoops_buffer_size);
	print_hex("SF size", info->spi_flash.size);
	print_hex("SF sector", info->spi_flash.sector_size);
	print_hex("SF erase cmd", info->spi_flash.erase_cmd);

	print_addr64("FMAP offset", info->fmap_offset);
	print_addr64("CBFS offset", info->cbfs_offset);
	print_addr64("CBFS size", info->cbfs_size);
	print_addr64("Boot media size", info->boot_media_size);
	print_addr64("MTC start", info->mtc_start);
	print_hex("MTC size", info->mtc_size);

	print_ptr("Chrome OS VPD", info->chromeos_vpd);
	print_ptr("RSDP", info->rsdp);
	printf("%-12s: ", "Unimpl.");
	if (info->unimpl_count) {
		for (i = 0; i < info->unimpl_count; i++)
			printf("%02x ", info->unimpl[i]);
		printf("\n");
	} else {
		printf("(none)\n");
	}
}

static int do_cbsysinfo(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	bool verbose = false;

	if (argc > 1) {
		if (!strcmp("-v", argv[1]))
			verbose = true;
		else
			return CMD_RET_USAGE;
	}

	if (!gd->arch.coreboot_table) {
		printf("No coreboot sysinfo table found\n");
		return CMD_RET_FAILURE;
	}
	show_table(&lib_sysinfo, verbose);

	return 0;
}

U_BOOT_CMD(
	cbsysinfo,	2,	1,	do_cbsysinfo,
	"Show coreboot sysinfo table",
	"[-v]         Dumps out the contents of the sysinfo table. This only\n"
	"works if U-Boot is booted from coreboot"
);
