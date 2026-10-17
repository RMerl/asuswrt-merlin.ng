// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2023 Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 */

#define LOG_CATEGORY UCLASS_QFW

#include <efi_loader.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <qfw.h>
#include <smbios.h>
#include <tables_csum.h>
#include <linux/sizes.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * qfw_load_smbios_table() - load a QEMU firmware file
 *
 * @dev:	QEMU firmware device
 * @size:	parameter to return the size of the loaded table
 * @name:	name of the table to load
 * Return:	address of the loaded table, NULL on error
 */
static void *qfw_load_smbios_table(struct udevice *dev, uint32_t *size,
				   char *name)
{
	struct fw_file *file;
	struct bios_linker_entry *table;

	file = qfw_find_file(dev, name);
	if (!file) {
		log_debug("Can't find %s\n", name);
		return NULL;
	}

	*size = be32_to_cpu(file->cfg.size);

	table = malloc(*size);
	if (!table) {
		log_err("Out of memory\n");
		return NULL;
	}

	qfw_read_entry(dev, be16_to_cpu(file->cfg.select), *size, table);

	return table;
}

/**
 * qfw_parse_smbios_anchor() - parse QEMU's SMBIOS anchor
 *
 * @dev:	QEMU firmware device
 * @entry:	SMBIOS 3 structure to be filled from QEMU's anchor
 * Return:	0 for success, -ve on error
 */
static int qfw_parse_smbios_anchor(struct udevice *dev,
				   struct smbios3_entry *entry)
{
	void *table;
	uint32_t size;
	struct smbios_entry *entry2;
	struct smbios3_entry *entry3;
	const char smbios_sig[] = "_SM_";
	const char smbios3_sig[] = "_SM3_";
	int ret = 0;

	table = qfw_load_smbios_table(dev, &size, "etc/smbios/smbios-anchor");
	if (!table)
		return -ENOMEM;
	if (!memcmp(table, smbios3_sig, sizeof(smbios3_sig) - 1)) {
		entry3 = table;
		if (entry3->length != sizeof(struct smbios3_entry)) {
			ret = -ENOENT;
			goto out;
		}
		memcpy(entry, entry3, sizeof(struct smbios3_entry));
	} else if (!memcmp(table, smbios_sig, sizeof(smbios_sig) - 1)) {
		entry2 = table;
		if (entry2->length != sizeof(struct smbios_entry)) {
			ret = -ENOENT;
			goto out;
		}
		memset(entry, 0, sizeof(struct smbios3_entry));
		memcpy(entry, smbios3_sig, sizeof(smbios3_sig));
		entry->length = sizeof(struct smbios3_entry);
		entry->major_ver = entry2->major_ver;
		entry->minor_ver = entry2->minor_ver;
		entry->table_maximum_size = entry2->struct_table_length;
	} else {
		ret = -ENOENT;
		goto out;
	}
	ret = 0;
out:
	free(table);

	return ret;
}

/**
 * qfw_write_smbios_tables() - copy SMBIOS tables from QEMU
 *
 * @addr:	target buffer
 * @size:	size of target buffer
 * Return:	0 for success, -ve on error
 */
static int qfw_write_smbios_tables(u8 *addr, uint32_t size)
{
	int ret;
	struct udevice *dev;
	struct smbios3_entry *entry = (void *)addr;
	void *table;
	uint32_t table_size;

	ret = qfw_get_dev(&dev);
	if (ret) {
		log_err("No QEMU firmware device\n");
		return ret;
	}

	ret = qfw_read_firmware_list(dev);
	if (ret) {
		log_err("Can't read firmware file list\n");
		return ret;
	}

	ret = qfw_parse_smbios_anchor(dev, entry);
	if (ret) {
		log_debug("Can't parse anchor\n");
		return ret;
	}

	addr += entry->length;
	entry->struct_table_address = (uintptr_t)addr;
	entry->checksum = 0;
	entry->checksum = table_compute_checksum(entry,
						 sizeof(struct smbios3_entry));

	table = qfw_load_smbios_table(dev, &table_size,
				      "etc/smbios/smbios-tables");
	if (table_size + sizeof(struct smbios3_entry) > size) {
		free(table);
		return -ENOMEM;
	}
	memcpy(addr, table, table_size);
	free(table);

	return 0;
}

/**
 * qfw_evt_write_smbios_tables() - event handler for copying QEMU SMBIOS tables
 *
 * Return:	0 on success, -ve on error (only out of memory)
 */
static int qfw_evt_write_smbios_tables(void)
{
	phys_addr_t addr;
	void *ptr;
	int ret;
	/*
	 * TODO:
	 * This size is currently hard coded in lib/efi_loader/efi_smbios.c.
	 * We need a field in global data for the size.
	 */
	uint32_t size = SZ_4K;

	/* Reserve 64K for SMBIOS tables, aligned to a 4K boundary */
	ptr = memalign(SZ_4K, size);
	if (!ptr) {
		log_err("Out of memory\n");
		return -ENOMEM;
	}
	addr = map_to_sysmem(ptr);

	/* Generate SMBIOS tables */
	ret = qfw_write_smbios_tables(ptr, size);
	if (ret) {
		if (CONFIG_IS_ENABLED(GENERATE_SMBIOS_TABLE)) {
			log_info("Falling back to U-Boot generated SMBIOS tables\n");
			write_smbios_table(addr);
		}
	} else {
		log_debug("SMBIOS tables copied from QEMU\n");
	}

	gd_set_smbios_start(addr);

	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, qfw_evt_write_smbios_tables);
