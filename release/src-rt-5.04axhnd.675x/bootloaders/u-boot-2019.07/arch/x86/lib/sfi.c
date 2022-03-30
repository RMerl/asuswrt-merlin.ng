// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

/*
 * Intel Simple Firmware Interface (SFI)
 *
 * Yet another way to pass information to the Linux kernel.
 *
 * See https://simplefirmware.org/ for details
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <asm/cpu.h>
#include <asm/ioapic.h>
#include <asm/sfi.h>
#include <asm/tables.h>
#include <dm/uclass-internal.h>

struct table_info {
	u32 base;
	int ptr;
	u32 entry_start;
	u64 table[SFI_TABLE_MAX_ENTRIES];
	int count;
};

static void *get_entry_start(struct table_info *tab)
{
	if (tab->count == SFI_TABLE_MAX_ENTRIES)
		return NULL;
	tab->entry_start = tab->base + tab->ptr;
	tab->table[tab->count] = tab->entry_start;
	tab->entry_start += sizeof(struct sfi_table_header);

	return (void *)(uintptr_t)tab->entry_start;
}

static void finish_table(struct table_info *tab, const char *sig, void *entry)
{
	struct sfi_table_header *hdr;

	hdr = (struct sfi_table_header *)(uintptr_t)(tab->base + tab->ptr);
	strcpy(hdr->sig, sig);
	hdr->len = sizeof(*hdr) + ((ulong)entry - tab->entry_start);
	hdr->rev = 1;
	strncpy(hdr->oem_id, "U-Boot", SFI_OEM_ID_SIZE);
	strncpy(hdr->oem_table_id, "Table v1", SFI_OEM_TABLE_ID_SIZE);
	hdr->csum = 0;
	hdr->csum = table_compute_checksum(hdr, hdr->len);
	tab->ptr += hdr->len;
	tab->ptr = ALIGN(tab->ptr, 16);
	tab->count++;
}

static int sfi_write_system_header(struct table_info *tab)
{
	u64 *entry = get_entry_start(tab);
	int i;

	if (!entry)
		return -ENOSPC;

	for (i = 0; i < tab->count; i++)
		*entry++ = tab->table[i];
	finish_table(tab, SFI_SIG_SYST, entry);

	return 0;
}

static int sfi_write_cpus(struct table_info *tab)
{
	struct sfi_cpu_table_entry *entry = get_entry_start(tab);
	struct udevice *dev;
	int count = 0;

	if (!entry)
		return -ENOSPC;

	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct cpu_platdata *plat = dev_get_parent_platdata(dev);

		if (!device_active(dev))
			continue;
		entry->apic_id = plat->cpu_id;
		entry++;
		count++;
	}

	/* Omit the table if there is only one CPU */
	if (count > 1)
		finish_table(tab, SFI_SIG_CPUS, entry);

	return 0;
}

static int sfi_write_apic(struct table_info *tab)
{
	struct sfi_apic_table_entry *entry = get_entry_start(tab);

	if (!entry)
		return -ENOSPC;

	entry->phys_addr = IO_APIC_ADDR;
	entry++;
	finish_table(tab, SFI_SIG_APIC, entry);

	return 0;
}

static int sfi_write_xsdt(struct table_info *tab)
{
	struct sfi_xsdt_header *entry = get_entry_start(tab);

	if (!entry)
		return -ENOSPC;

	entry->oem_revision = 1;
	entry->creator_id = 1;
	entry->creator_revision = 1;
	entry++;
	finish_table(tab, SFI_SIG_XSDT, entry);

	return 0;
}

ulong write_sfi_table(ulong base)
{
	struct table_info table;

	table.base = base;
	table.ptr = 0;
	table.count = 0;
	sfi_write_cpus(&table);
	sfi_write_apic(&table);

	/*
	 * The SFI specification marks the XSDT table as option, but Linux 4.0
	 * crashes on start-up when it is not provided.
	 */
	sfi_write_xsdt(&table);

	/* Finally, write out the system header which points to the others */
	sfi_write_system_header(&table);

	return base + table.ptr;
}
