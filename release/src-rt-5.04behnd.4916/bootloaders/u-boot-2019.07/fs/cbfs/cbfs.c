// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 */

#include <common.h>
#include <cbfs.h>
#include <malloc.h>
#include <asm/byteorder.h>

enum cbfs_result file_cbfs_result;

const char *file_cbfs_error(void)
{
	switch (file_cbfs_result) {
	case CBFS_SUCCESS:
		return "Success";
	case CBFS_NOT_INITIALIZED:
		return "CBFS not initialized";
	case CBFS_BAD_HEADER:
		return "Bad CBFS header";
	case CBFS_BAD_FILE:
		return "Bad CBFS file";
	case CBFS_FILE_NOT_FOUND:
		return "File not found";
	default:
		return "Unknown";
	}
}


static const u32 good_magic = 0x4f524243;
static const u8 good_file_magic[] = "LARCHIVE";


static int initialized;
static struct cbfs_header cbfs_header;
static struct cbfs_cachenode *file_cache;

/* Do endian conversion on the CBFS header structure. */
static void swap_header(struct cbfs_header *dest, struct cbfs_header *src)
{
	dest->magic = be32_to_cpu(src->magic);
	dest->version = be32_to_cpu(src->version);
	dest->rom_size = be32_to_cpu(src->rom_size);
	dest->boot_block_size = be32_to_cpu(src->boot_block_size);
	dest->align = be32_to_cpu(src->align);
	dest->offset = be32_to_cpu(src->offset);
}

/* Do endian conversion on a CBFS file header. */
static void swap_file_header(struct cbfs_fileheader *dest,
			     const struct cbfs_fileheader *src)
{
	memcpy(&dest->magic, &src->magic, sizeof(dest->magic));
	dest->len = be32_to_cpu(src->len);
	dest->type = be32_to_cpu(src->type);
	dest->checksum = be32_to_cpu(src->checksum);
	dest->offset = be32_to_cpu(src->offset);
}

/*
 * Given a starting position in memory, scan forward, bounded by a size, and
 * find the next valid CBFS file. No memory is allocated by this function. The
 * caller is responsible for allocating space for the new file structure.
 *
 * @param start		The location in memory to start from.
 * @param size		The size of the memory region to search.
 * @param align		The alignment boundaries to check on.
 * @param newNode	A pointer to the file structure to load.
 * @param used		A pointer to the count of of bytes scanned through,
 *			including the file if one is found.
 *
 * @return 1 if a file is found, 0 if one isn't.
 */
static int file_cbfs_next_file(u8 *start, u32 size, u32 align,
			       struct cbfs_cachenode *newNode, u32 *used)
{
	struct cbfs_fileheader header;

	*used = 0;

	while (size >= align) {
		const struct cbfs_fileheader *fileHeader =
			(const struct cbfs_fileheader *)start;
		u32 name_len;
		u32 step;

		/* Check if there's a file here. */
		if (memcmp(good_file_magic, &(fileHeader->magic),
				sizeof(fileHeader->magic))) {
			*used += align;
			size -= align;
			start += align;
			continue;
		}

		swap_file_header(&header, fileHeader);
		if (header.offset < sizeof(struct cbfs_fileheader)) {
			file_cbfs_result = CBFS_BAD_FILE;
			return -1;
		}
		newNode->next = NULL;
		newNode->type = header.type;
		newNode->data = start + header.offset;
		newNode->data_length = header.len;
		name_len = header.offset - sizeof(struct cbfs_fileheader);
		newNode->name = (char *)fileHeader +
				sizeof(struct cbfs_fileheader);
		newNode->name_length = name_len;
		newNode->checksum = header.checksum;

		step = header.len;
		if (step % align)
			step = step + align - step % align;

		*used += step;
		return 1;
	}
	return 0;
}

/* Look through a CBFS instance and copy file metadata into regular memory. */
static void file_cbfs_fill_cache(u8 *start, u32 size, u32 align)
{
	struct cbfs_cachenode *cache_node;
	struct cbfs_cachenode *newNode;
	struct cbfs_cachenode **cache_tail = &file_cache;

	/* Clear out old information. */
	cache_node = file_cache;
	while (cache_node) {
		struct cbfs_cachenode *oldNode = cache_node;
		cache_node = cache_node->next;
		free(oldNode);
	}
	file_cache = NULL;

	while (size >= align) {
		int result;
		u32 used;

		newNode = (struct cbfs_cachenode *)
				malloc(sizeof(struct cbfs_cachenode));
		result = file_cbfs_next_file(start, size, align,
			newNode, &used);

		if (result < 0) {
			free(newNode);
			return;
		} else if (result == 0) {
			free(newNode);
			break;
		}
		*cache_tail = newNode;
		cache_tail = &newNode->next;

		size -= used;
		start += used;
	}
	file_cbfs_result = CBFS_SUCCESS;
}

/* Get the CBFS header out of the ROM and do endian conversion. */
static int file_cbfs_load_header(uintptr_t end_of_rom,
				 struct cbfs_header *header)
{
	struct cbfs_header *header_in_rom;
	int32_t offset = *(u32 *)(end_of_rom - 3);

	header_in_rom = (struct cbfs_header *)(end_of_rom + offset + 1);
	swap_header(header, header_in_rom);

	if (header->magic != good_magic || header->offset >
			header->rom_size - header->boot_block_size) {
		file_cbfs_result = CBFS_BAD_HEADER;
		return 1;
	}
	return 0;
}

void file_cbfs_init(uintptr_t end_of_rom)
{
	u8 *start_of_rom;
	initialized = 0;

	if (file_cbfs_load_header(end_of_rom, &cbfs_header))
		return;

	start_of_rom = (u8 *)(end_of_rom + 1 - cbfs_header.rom_size);

	file_cbfs_fill_cache(start_of_rom, cbfs_header.rom_size,
			     cbfs_header.align);
	if (file_cbfs_result == CBFS_SUCCESS)
		initialized = 1;
}

const struct cbfs_header *file_cbfs_get_header(void)
{
	if (initialized) {
		file_cbfs_result = CBFS_SUCCESS;
		return &cbfs_header;
	} else {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	}
}

const struct cbfs_cachenode *file_cbfs_get_first(void)
{
	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	} else {
		file_cbfs_result = CBFS_SUCCESS;
		return file_cache;
	}
}

void file_cbfs_get_next(const struct cbfs_cachenode **file)
{
	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		file = NULL;
		return;
	}

	if (*file)
		*file = (*file)->next;
	file_cbfs_result = CBFS_SUCCESS;
}

const struct cbfs_cachenode *file_cbfs_find(const char *name)
{
	struct cbfs_cachenode *cache_node = file_cache;

	if (!initialized) {
		file_cbfs_result = CBFS_NOT_INITIALIZED;
		return NULL;
	}

	while (cache_node) {
		if (!strcmp(name, cache_node->name))
			break;
		cache_node = cache_node->next;
	}
	if (!cache_node)
		file_cbfs_result = CBFS_FILE_NOT_FOUND;
	else
		file_cbfs_result = CBFS_SUCCESS;

	return cache_node;
}

const struct cbfs_cachenode *file_cbfs_find_uncached(uintptr_t end_of_rom,
						     const char *name)
{
	u8 *start;
	u32 size;
	u32 align;
	static struct cbfs_cachenode node;

	if (file_cbfs_load_header(end_of_rom, &cbfs_header))
		return NULL;

	start = (u8 *)(end_of_rom + 1 - cbfs_header.rom_size);
	size = cbfs_header.rom_size;
	align = cbfs_header.align;

	while (size >= align) {
		int result;
		u32 used;

		result = file_cbfs_next_file(start, size, align, &node, &used);

		if (result < 0)
			return NULL;
		else if (result == 0)
			break;

		if (!strcmp(name, node.name))
			return &node;

		size -= used;
		start += used;
	}
	file_cbfs_result = CBFS_FILE_NOT_FOUND;
	return NULL;
}

const char *file_cbfs_name(const struct cbfs_cachenode *file)
{
	file_cbfs_result = CBFS_SUCCESS;
	return file->name;
}

u32 file_cbfs_size(const struct cbfs_cachenode *file)
{
	file_cbfs_result = CBFS_SUCCESS;
	return file->data_length;
}

u32 file_cbfs_type(const struct cbfs_cachenode *file)
{
	file_cbfs_result = CBFS_SUCCESS;
	return file->type;
}

long file_cbfs_read(const struct cbfs_cachenode *file, void *buffer,
		    unsigned long maxsize)
{
	u32 size;

	size = file->data_length;
	if (maxsize && size > maxsize)
		size = maxsize;

	memcpy(buffer, file->data, size);

	file_cbfs_result = CBFS_SUCCESS;
	return size;
}
