/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ext4_utils.h"
#include "extent.h"

#include <sparse/sparse.h>

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>


/* Creates data buffers for the first backing_len bytes of a block allocation
   and queues them to be written */
static u8 *extent_create_backing(struct block_allocation *alloc,
	u64 backing_len)
{
	u8 *data = calloc(backing_len, 1);
	if (!data)
		critical_error_errno("calloc");

	u8 *ptr = data;
	for (; alloc != NULL && backing_len > 0; get_next_region(alloc)) {
		u32 region_block;
		u32 region_len;
		u32 len;
		get_region(alloc, &region_block, &region_len);

		len = min(region_len * info.block_size, backing_len);

		sparse_file_add_data(ext4_sparse_file, ptr, len, region_block);
		ptr += len;
		backing_len -= len;
	}

	return data;
}

/* Queues each chunk of a file to be written to contiguous data block
   regions */
static void extent_create_backing_file(struct block_allocation *alloc,
	u64 backing_len, const char *filename)
{
	off64_t offset = 0;
	for (; alloc != NULL && backing_len > 0; get_next_region(alloc)) {
		u32 region_block;
		u32 region_len;
		u32 len;
		get_region(alloc, &region_block, &region_len);

		len = min(region_len * info.block_size, backing_len);

		sparse_file_add_file(ext4_sparse_file, filename, offset, len,
				region_block);
		offset += len;
		backing_len -= len;
	}
}

static struct block_allocation *do_inode_allocate_extents(
	struct ext4_inode *inode, u64 len)
{
	u32 block_len = DIV_ROUND_UP(len, info.block_size);
	struct block_allocation *alloc = allocate_blocks(block_len + 1);
	u32 extent_block = 0;
	u32 file_block = 0;
	struct ext4_extent *extent;
	u64 blocks;

	if (alloc == NULL) {
		error("Failed to allocate %d blocks\n", block_len + 1);
		return NULL;
	}

	int allocation_len = block_allocation_num_regions(alloc);
	if (allocation_len <= 3) {
		reduce_allocation(alloc, 1);
	} else {
		reserve_oob_blocks(alloc, 1);
		extent_block = get_oob_block(alloc, 0);
	}

	if (!extent_block) {
		struct ext4_extent_header *hdr =
			(struct ext4_extent_header *)&inode->i_block[0];
		hdr->eh_magic = EXT4_EXT_MAGIC;
		hdr->eh_entries = allocation_len;
		hdr->eh_max = 3;
		hdr->eh_generation = 0;
		hdr->eh_depth = 0;

		extent = (struct ext4_extent *)&inode->i_block[3];
	} else {
		struct ext4_extent_header *hdr =
			(struct ext4_extent_header *)&inode->i_block[0];
		hdr->eh_magic = EXT4_EXT_MAGIC;
		hdr->eh_entries = 1;
		hdr->eh_max = 3;
		hdr->eh_generation = 0;
		hdr->eh_depth = 1;

		struct ext4_extent_idx *idx =
			(struct ext4_extent_idx *)&inode->i_block[3];
		idx->ei_block = 0;
		idx->ei_leaf_lo = extent_block;
		idx->ei_leaf_hi = 0;
		idx->ei_unused = 0;

		u8 *data = calloc(info.block_size, 1);
		if (!data)
			critical_error_errno("calloc");

		sparse_file_add_data(ext4_sparse_file, data, info.block_size,
				extent_block);

		if (((int)(info.block_size - sizeof(struct ext4_extent_header) /
				sizeof(struct ext4_extent))) < allocation_len) {
			error("File size %"PRIu64" is too big to fit in a single extent block\n",
					len);
			return NULL;
		}

		hdr = (struct ext4_extent_header *)data;
		hdr->eh_magic = EXT4_EXT_MAGIC;
		hdr->eh_entries = allocation_len;
		hdr->eh_max = (info.block_size - sizeof(struct ext4_extent_header)) /
			sizeof(struct ext4_extent);
		hdr->eh_generation = 0;
		hdr->eh_depth = 0;

		extent = (struct ext4_extent *)(data +
			sizeof(struct ext4_extent_header));
	}

	for (; !last_region(alloc); extent++, get_next_region(alloc)) {
		u32 region_block;
		u32 region_len;

		get_region(alloc, &region_block, &region_len);
		extent->ee_block = file_block;
		extent->ee_len = region_len;
		extent->ee_start_hi = 0;
		extent->ee_start_lo = region_block;
		file_block += region_len;
	}

	if (extent_block)
		block_len += 1;

	blocks = (u64)block_len * info.block_size / 512;

	inode->i_flags |= EXT4_EXTENTS_FL;
	inode->i_size_lo = len;
	inode->i_size_high = len >> 32;
	inode->i_blocks_lo = blocks;
	inode->osd2.linux2.l_i_blocks_high = blocks >> 32;

	rewind_alloc(alloc);

	return alloc;
}

/* Allocates enough blocks to hold len bytes, with backing_len bytes in a data
   buffer, and connects them to an inode.  Returns a pointer to the data
   buffer. */
u8 *inode_allocate_data_extents(struct ext4_inode *inode, u64 len,
	u64 backing_len)
{
	struct block_allocation *alloc;
	u8 *data = NULL;

	alloc = do_inode_allocate_extents(inode, len);
	if (alloc == NULL) {
		error("failed to allocate extents for %"PRIu64" bytes", len);
		return NULL;
	}

	if (backing_len) {
		data = extent_create_backing(alloc, backing_len);
		if (!data)
			error("failed to create backing for %"PRIu64" bytes", backing_len);
	}

	free_alloc(alloc);

	return data;
}

/* Allocates enough blocks to hold len bytes, queues them to be written
   from a file, and connects them to an inode. */
struct block_allocation* inode_allocate_file_extents(struct ext4_inode *inode, u64 len,
	const char *filename)
{
	struct block_allocation *alloc;

	alloc = do_inode_allocate_extents(inode, len);
	if (alloc == NULL) {
		error("failed to allocate extents for %"PRIu64" bytes", len);
		return NULL;
	}

	extent_create_backing_file(alloc, len, filename);
	return alloc;
}

/* Allocates enough blocks to hold len bytes and connects them to an inode */
void inode_allocate_extents(struct ext4_inode *inode, u64 len)
{
	struct block_allocation *alloc;

	alloc = do_inode_allocate_extents(inode, len);
	if (alloc == NULL) {
		error("failed to allocate extents for %"PRIu64" bytes", len);
		return;
	}

	free_alloc(alloc);
}
