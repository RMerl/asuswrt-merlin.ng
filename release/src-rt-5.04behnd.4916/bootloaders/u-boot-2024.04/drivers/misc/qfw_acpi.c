// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#define LOG_CATEGORY UCLASS_QFW

#include <acpi/acpi_table.h>
#include <errno.h>
#include <malloc.h>
#include <mapmem.h>
#include <qfw.h>
#include <tables_csum.h>
#include <stdio.h>
#include <linux/sizes.h>
#include <asm/byteorder.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * This function allocates memory for ACPI tables
 *
 * @entry : BIOS linker command entry which tells where to allocate memory
 *          (either high memory or low memory)
 * @addr  : The address that should be used for low memory allcation. If the
 *          memory allocation request is 'ZONE_HIGH' then this parameter will
 *          be ignored.
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_allocate(struct udevice *dev,
				struct bios_linker_entry *entry, ulong *addr)
{
	uint32_t size, align;
	struct fw_file *file;
	unsigned long aligned_addr;

	align = le32_to_cpu(entry->alloc.align);
	/* align must be power of 2 */
	if (align & (align - 1)) {
		printf("error: wrong alignment %u\n", align);
		return -EINVAL;
	}

	file = qfw_find_file(dev, entry->alloc.file);
	if (!file) {
		printf("error: can't find file %s\n", entry->alloc.file);
		return -ENOENT;
	}

	size = be32_to_cpu(file->cfg.size);

	/*
	 * ZONE_HIGH means we need to allocate from high memory, since
	 * malloc space is already at the end of RAM, so we directly use it.
	 * If allocation zone is ZONE_FSEG, then we use the 'addr' passed
	 * in which is low memory
	 */
	if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_HIGH) {
		aligned_addr = (unsigned long)memalign(align, size);
		if (!aligned_addr) {
			printf("error: allocating resource\n");
			return -ENOMEM;
		}
		if (aligned_addr < gd->arch.table_start_high)
			gd->arch.table_start_high = aligned_addr;
		if (aligned_addr + size > gd->arch.table_end_high)
			gd->arch.table_end_high = aligned_addr + size;

	} else if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_FSEG) {
		aligned_addr = ALIGN(*addr, align);
	} else {
		printf("error: invalid allocation zone\n");
		return -EINVAL;
	}

	debug("bios_linker_allocate: allocate file %s, size %u, zone %d, align %u, addr 0x%lx\n",
	      file->cfg.name, size, entry->alloc.zone, align, aligned_addr);

	qfw_read_entry(dev, be16_to_cpu(file->cfg.select), size,
		       (void *)aligned_addr);
	file->addr = aligned_addr;

	/* adjust address for low memory allocation */
	if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_FSEG)
		*addr = (aligned_addr + size);

	return 0;
}

/*
 * This function patches ACPI tables previously loaded
 * by bios_linker_allocate()
 *
 * @entry : BIOS linker command entry which tells how to patch
 *          ACPI tables
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_add_pointer(struct udevice *dev,
				   struct bios_linker_entry *entry)
{
	struct fw_file *dest, *src;
	uint32_t offset = le32_to_cpu(entry->pointer.offset);
	uint64_t pointer = 0;

	dest = qfw_find_file(dev, entry->pointer.dest_file);
	if (!dest || !dest->addr)
		return -ENOENT;
	src = qfw_find_file(dev, entry->pointer.src_file);
	if (!src || !src->addr)
		return -ENOENT;

	debug("bios_linker_add_pointer: dest->addr 0x%lx, src->addr 0x%lx, offset 0x%x size %u, 0x%llx\n",
	      dest->addr, src->addr, offset, entry->pointer.size, pointer);

	memcpy(&pointer, (char *)dest->addr + offset, entry->pointer.size);
	pointer	= le64_to_cpu(pointer);
	pointer += (unsigned long)src->addr;
	pointer	= cpu_to_le64(pointer);
	memcpy((char *)dest->addr + offset, &pointer, entry->pointer.size);

	return 0;
}

/*
 * This function updates checksum fields of ACPI tables previously loaded
 * by bios_linker_allocate()
 *
 * @entry : BIOS linker command entry which tells where to update ACPI table
 *          checksums
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_add_checksum(struct udevice *dev,
				    struct bios_linker_entry *entry)
{
	struct fw_file *file;
	uint8_t *data, cksum = 0;
	uint8_t *cksum_start;

	file = qfw_find_file(dev, entry->cksum.file);
	if (!file || !file->addr)
		return -ENOENT;

	data = (uint8_t *)(file->addr + le32_to_cpu(entry->cksum.offset));
	cksum_start = (uint8_t *)(file->addr + le32_to_cpu(entry->cksum.start));
	cksum = table_compute_checksum(cksum_start,
				       le32_to_cpu(entry->cksum.length));
	*data = cksum;

	return 0;
}

/* This function loads and patches ACPI tables provided by QEMU */
ulong write_acpi_tables(ulong addr)
{
	int i, ret;
	struct fw_file *file;
	struct bios_linker_entry *table_loader;
	struct bios_linker_entry *entry;
	uint32_t size;
	struct udevice *dev;

	ret = qfw_get_dev(&dev);
	if (ret) {
		printf("error: no qfw\n");
		return addr;
	}

	/* make sure fw_list is loaded */
	ret = qfw_read_firmware_list(dev);
	if (ret) {
		printf("error: can't read firmware file list\n");
		return addr;
	}

	file = qfw_find_file(dev, "etc/table-loader");
	if (!file) {
		printf("error: can't find etc/table-loader\n");
		return addr;
	}

	size = be32_to_cpu(file->cfg.size);
	if ((size % sizeof(*entry)) != 0) {
		printf("error: table-loader maybe corrupted\n");
		return addr;
	}

	table_loader = malloc(size);
	if (!table_loader) {
		printf("error: no memory for table-loader\n");
		return addr;
	}

	/* QFW always puts tables at high addresses */
	gd->arch.table_start_high = (ulong)table_loader;
	gd->arch.table_end_high = (ulong)table_loader;

	qfw_read_entry(dev, be16_to_cpu(file->cfg.select), size, table_loader);

	for (i = 0; i < (size / sizeof(*entry)); i++) {
		entry = table_loader + i;
		switch (le32_to_cpu(entry->command)) {
		case BIOS_LINKER_LOADER_COMMAND_ALLOCATE:
			ret = bios_linker_allocate(dev, entry, &addr);
			if (ret)
				goto out;
			break;
		case BIOS_LINKER_LOADER_COMMAND_ADD_POINTER:
			ret = bios_linker_add_pointer(dev, entry);
			if (ret)
				goto out;
			break;
		case BIOS_LINKER_LOADER_COMMAND_ADD_CHECKSUM:
			ret = bios_linker_add_checksum(dev, entry);
			if (ret)
				goto out;
			break;
		default:
			break;
		}
	}

out:
	if (ret) {
		struct fw_cfg_file_iter iter;
		for (file = qfw_file_iter_init(dev, &iter);
		     !qfw_file_iter_end(&iter);
		     file = qfw_file_iter_next(&iter)) {
			if (file->addr) {
				free((void *)file->addr);
				file->addr = 0;
			}
		}
	}

	free(table_loader);

	gd_set_acpi_start(acpi_get_rsdp_addr());

	return addr;
}

ulong acpi_get_rsdp_addr(void)
{
	int ret;
	struct fw_file *file;
	struct udevice *dev;

	ret = qfw_get_dev(&dev);
	if (ret) {
		printf("error: no qfw\n");
		return 0;
	}

	file = qfw_find_file(dev, "etc/acpi/rsdp");
	return file->addr;
}

#ifndef CONFIG_X86
static int evt_write_acpi_tables(void)
{
	ulong addr, end;
	void *ptr;

	/* Reserve 64K for ACPI tables, aligned to a 4K boundary */
	ptr = memalign(SZ_4K, SZ_64K);
	if (!ptr)
		return -ENOMEM;
	addr = map_to_sysmem(ptr);

	/* Generate ACPI tables */
	end = write_acpi_tables(addr);
	gd->arch.table_start = addr;
	gd->arch.table_end = addr;

	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, evt_write_acpi_tables);
#endif
