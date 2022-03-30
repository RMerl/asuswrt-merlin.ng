// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <malloc.h>
#include <qfw.h>
#include <asm/io.h>
#ifdef CONFIG_GENERATE_ACPI_TABLE
#include <asm/tables.h>
#endif
#include <linux/list.h>

static bool fwcfg_present;
static bool fwcfg_dma_present;
static struct fw_cfg_arch_ops *fwcfg_arch_ops;

static LIST_HEAD(fw_list);

#ifdef CONFIG_GENERATE_ACPI_TABLE
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
static int bios_linker_allocate(struct bios_linker_entry *entry, ulong *addr)
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

	file = qemu_fwcfg_find_file(entry->alloc.file);
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
	} else if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_FSEG) {
		aligned_addr = ALIGN(*addr, align);
	} else {
		printf("error: invalid allocation zone\n");
		return -EINVAL;
	}

	debug("bios_linker_allocate: allocate file %s, size %u, zone %d, align %u, addr 0x%lx\n",
	      file->cfg.name, size, entry->alloc.zone, align, aligned_addr);

	qemu_fwcfg_read_entry(be16_to_cpu(file->cfg.select),
			      size, (void *)aligned_addr);
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
static int bios_linker_add_pointer(struct bios_linker_entry *entry)
{
	struct fw_file *dest, *src;
	uint32_t offset = le32_to_cpu(entry->pointer.offset);
	uint64_t pointer = 0;

	dest = qemu_fwcfg_find_file(entry->pointer.dest_file);
	if (!dest || !dest->addr)
		return -ENOENT;
	src = qemu_fwcfg_find_file(entry->pointer.src_file);
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
static int bios_linker_add_checksum(struct bios_linker_entry *entry)
{
	struct fw_file *file;
	uint8_t *data, cksum = 0;
	uint8_t *cksum_start;

	file = qemu_fwcfg_find_file(entry->cksum.file);
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
	int i, ret = 0;
	struct fw_file *file;
	struct bios_linker_entry *table_loader;
	struct bios_linker_entry *entry;
	uint32_t size;

	/* make sure fw_list is loaded */
	ret = qemu_fwcfg_read_firmware_list();
	if (ret) {
		printf("error: can't read firmware file list\n");
		return addr;
	}

	file = qemu_fwcfg_find_file("etc/table-loader");
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

	qemu_fwcfg_read_entry(be16_to_cpu(file->cfg.select),
			      size, table_loader);

	for (i = 0; i < (size / sizeof(*entry)); i++) {
		entry = table_loader + i;
		switch (le32_to_cpu(entry->command)) {
		case BIOS_LINKER_LOADER_COMMAND_ALLOCATE:
			ret = bios_linker_allocate(entry, &addr);
			if (ret)
				goto out;
			break;
		case BIOS_LINKER_LOADER_COMMAND_ADD_POINTER:
			ret = bios_linker_add_pointer(entry);
			if (ret)
				goto out;
			break;
		case BIOS_LINKER_LOADER_COMMAND_ADD_CHECKSUM:
			ret = bios_linker_add_checksum(entry);
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
		for (file = qemu_fwcfg_file_iter_init(&iter);
		     !qemu_fwcfg_file_iter_end(&iter);
		     file = qemu_fwcfg_file_iter_next(&iter)) {
			if (file->addr) {
				free((void *)file->addr);
				file->addr = 0;
			}
		}
	}

	free(table_loader);
	return addr;
}

ulong acpi_get_rsdp_addr(void)
{
	struct fw_file *file;

	file = qemu_fwcfg_find_file("etc/acpi/rsdp");
	return file->addr;
}
#endif

/* Read configuration item using fw_cfg PIO interface */
static void qemu_fwcfg_read_entry_pio(uint16_t entry,
		uint32_t size, void *address)
{
	debug("qemu_fwcfg_read_entry_pio: entry 0x%x, size %u address %p\n",
	      entry, size, address);

	return fwcfg_arch_ops->arch_read_pio(entry, size, address);
}

/* Read configuration item using fw_cfg DMA interface */
static void qemu_fwcfg_read_entry_dma(uint16_t entry,
		uint32_t size, void *address)
{
	struct fw_cfg_dma_access dma;

	dma.length = cpu_to_be32(size);
	dma.address = cpu_to_be64((uintptr_t)address);
	dma.control = cpu_to_be32(FW_CFG_DMA_READ);

	/*
	 * writting FW_CFG_INVALID will cause read operation to resume at
	 * last offset, otherwise read will start at offset 0
	 */
	if (entry != FW_CFG_INVALID)
		dma.control |= cpu_to_be32(FW_CFG_DMA_SELECT | (entry << 16));

	barrier();

	debug("qemu_fwcfg_read_entry_dma: entry 0x%x, size %u address %p, control 0x%x\n",
	      entry, size, address, be32_to_cpu(dma.control));

	fwcfg_arch_ops->arch_read_dma(&dma);
}

bool qemu_fwcfg_present(void)
{
	return fwcfg_present;
}

bool qemu_fwcfg_dma_present(void)
{
	return fwcfg_dma_present;
}

void qemu_fwcfg_read_entry(uint16_t entry, uint32_t length, void *address)
{
	if (fwcfg_dma_present)
		qemu_fwcfg_read_entry_dma(entry, length, address);
	else
		qemu_fwcfg_read_entry_pio(entry, length, address);
}

int qemu_fwcfg_online_cpus(void)
{
	uint16_t nb_cpus;

	if (!fwcfg_present)
		return -ENODEV;

	qemu_fwcfg_read_entry(FW_CFG_NB_CPUS, 2, &nb_cpus);

	return le16_to_cpu(nb_cpus);
}

int qemu_fwcfg_read_firmware_list(void)
{
	int i;
	uint32_t count;
	struct fw_file *file;
	struct list_head *entry;

	/* don't read it twice */
	if (!list_empty(&fw_list))
		return 0;

	qemu_fwcfg_read_entry(FW_CFG_FILE_DIR, 4, &count);
	if (!count)
		return 0;

	count = be32_to_cpu(count);
	for (i = 0; i < count; i++) {
		file = malloc(sizeof(*file));
		if (!file) {
			printf("error: allocating resource\n");
			goto err;
		}
		qemu_fwcfg_read_entry(FW_CFG_INVALID,
				      sizeof(struct fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &fw_list);
	}

	return 0;

err:
	list_for_each(entry, &fw_list) {
		file = list_entry(entry, struct fw_file, list);
		free(file);
	}

	return -ENOMEM;
}

struct fw_file *qemu_fwcfg_find_file(const char *name)
{
	struct list_head *entry;
	struct fw_file *file;

	list_for_each(entry, &fw_list) {
		file = list_entry(entry, struct fw_file, list);
		if (!strcmp(file->cfg.name, name))
			return file;
	}

	return NULL;
}

struct fw_file *qemu_fwcfg_file_iter_init(struct fw_cfg_file_iter *iter)
{
	iter->entry = fw_list.next;
	return list_entry((struct list_head *)iter->entry,
			  struct fw_file, list);
}

struct fw_file *qemu_fwcfg_file_iter_next(struct fw_cfg_file_iter *iter)
{
	iter->entry = ((struct list_head *)iter->entry)->next;
	return list_entry((struct list_head *)iter->entry,
			  struct fw_file, list);
}

bool qemu_fwcfg_file_iter_end(struct fw_cfg_file_iter *iter)
{
	return iter->entry == &fw_list;
}

void qemu_fwcfg_init(struct fw_cfg_arch_ops *ops)
{
	uint32_t qemu;
	uint32_t dma_enabled;

	fwcfg_present = false;
	fwcfg_dma_present = false;
	fwcfg_arch_ops = NULL;

	if (!ops || !ops->arch_read_pio || !ops->arch_read_dma)
		return;
	fwcfg_arch_ops = ops;

	qemu_fwcfg_read_entry_pio(FW_CFG_SIGNATURE, 4, &qemu);
	if (be32_to_cpu(qemu) == QEMU_FW_CFG_SIGNATURE)
		fwcfg_present = true;

	if (fwcfg_present) {
		qemu_fwcfg_read_entry_pio(FW_CFG_ID, 1, &dma_enabled);
		if (dma_enabled & FW_CFG_DMA_ENABLED)
			fwcfg_dma_present = true;
	}
}
