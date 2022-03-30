// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Intel Corporation
 */

#include <common.h>
#include <asm/e820.h>
#include <asm/global_data.h>
#include <asm/sfi.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * SFI tables are part of the first stage bootloader.
 *
 * U-Boot finds the System Table by searching 16-byte boundaries between
 * physical address 0x000E0000 and 0x000FFFFF. U-Boot shall search this region
 * starting at the low address and shall stop searching when the 1st valid SFI
 * System Table is found.
 */
#define SFI_BASE_ADDR		0x000E0000
#define SFI_LENGTH		0x00020000
#define SFI_TABLE_LENGTH	16

static int sfi_table_check(struct sfi_table_header *sbh)
{
	char chksum = 0;
	char *pos = (char *)sbh;
	u32 i;

	if (sbh->len < SFI_TABLE_LENGTH)
		return -ENXIO;

	if (sbh->len > SFI_LENGTH)
		return -ENXIO;

	for (i = 0; i < sbh->len; i++)
		chksum += *pos++;

	if (chksum)
		pr_err("sfi: Invalid checksum\n");

	/* Checksum is OK if zero */
	return chksum ? -EILSEQ : 0;
}

static int sfi_table_is_type(struct sfi_table_header *sbh, const char *signature)
{
	return !strncmp(sbh->sig, signature, SFI_SIGNATURE_SIZE) &&
	       !sfi_table_check(sbh);
}

static struct sfi_table_simple *sfi_get_table_by_sig(unsigned long addr,
						     const char *signature)
{
	struct sfi_table_simple *sb;
	u32 i;

	for (i = 0; i < SFI_LENGTH; i += SFI_TABLE_LENGTH) {
		sb = (struct sfi_table_simple *)(addr + i);
		if (sfi_table_is_type(&sb->header, signature))
			return sb;
	}

	return NULL;
}

static struct sfi_table_simple *sfi_search_mmap(void)
{
	struct sfi_table_header *sbh;
	struct sfi_table_simple *sb;
	u32 sys_entry_cnt;
	u32 i;

	/* Find SYST table */
	sb = sfi_get_table_by_sig(SFI_BASE_ADDR, SFI_SIG_SYST);
	if (!sb) {
		pr_err("sfi: failed to locate SYST table\n");
		return NULL;
	}

	sys_entry_cnt = (sb->header.len - sizeof(*sbh)) / 8;

	/* Search through each SYST entry for MMAP table */
	for (i = 0; i < sys_entry_cnt; i++) {
		sbh = (struct sfi_table_header *)(unsigned long)sb->pentry[i];

		if (sfi_table_is_type(sbh, SFI_SIG_MMAP))
			return (struct sfi_table_simple *)sbh;
	}

	pr_err("sfi: failed to locate SFI MMAP table\n");
	return NULL;
}

#define sfi_for_each_mentry(i, sb, mentry)				\
	for (i = 0, mentry = (struct sfi_mem_entry *)sb->pentry;	\
	     i < SFI_GET_NUM_ENTRIES(sb, struct sfi_mem_entry);		\
	     i++, mentry++)						\

static unsigned int sfi_setup_e820(unsigned int max_entries,
				   struct e820_entry *entries)
{
	struct sfi_table_simple *sb;
	struct sfi_mem_entry *mentry;
	unsigned long long start, end, size;
	int type, total = 0;
	u32 i;

	sb = sfi_search_mmap();
	if (!sb)
		return 0;

	sfi_for_each_mentry(i, sb, mentry) {
		start = mentry->phys_start;
		size = mentry->pages << 12;
		end = start + size;

		if (start > end)
			continue;

		/* translate SFI mmap type to E820 map type */
		switch (mentry->type) {
		case SFI_MEM_CONV:
			type = E820_RAM;
			break;
		case SFI_MEM_UNUSABLE:
		case SFI_RUNTIME_SERVICE_DATA:
			continue;
		default:
			type = E820_RESERVED;
		}

		if (total == E820MAX)
			break;
		entries[total].addr = start;
		entries[total].size = size;
		entries[total].type = type;

		total++;
	}

	return total;
}

static int sfi_get_bank_size(void)
{
	struct sfi_table_simple *sb;
	struct sfi_mem_entry *mentry;
	int bank = 0;
	u32 i;

	sb = sfi_search_mmap();
	if (!sb)
		return 0;

	sfi_for_each_mentry(i, sb, mentry) {
		if (mentry->type != SFI_MEM_CONV)
			continue;

		gd->bd->bi_dram[bank].start = mentry->phys_start;
		gd->bd->bi_dram[bank].size = mentry->pages << 12;
		bank++;
	}

	return bank;
}

static phys_size_t sfi_get_ram_size(void)
{
	struct sfi_table_simple *sb;
	struct sfi_mem_entry *mentry;
	phys_size_t ram = 0;
	u32 i;

	sb = sfi_search_mmap();
	if (!sb)
		return 0;

	sfi_for_each_mentry(i, sb, mentry) {
		if (mentry->type != SFI_MEM_CONV)
			continue;

		ram += mentry->pages << 12;
	}

	debug("sfi: RAM size %llu\n", ram);
	return ram;
}

unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *entries)
{
	return sfi_setup_e820(max_entries, entries);
}

int dram_init_banksize(void)
{
	sfi_get_bank_size();
	return 0;
}

int dram_init(void)
{
	gd->ram_size = sfi_get_ram_size();
	return 0;
}
