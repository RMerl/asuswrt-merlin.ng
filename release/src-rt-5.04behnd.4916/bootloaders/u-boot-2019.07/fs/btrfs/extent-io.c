// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
 */

#include "btrfs.h"
#include <malloc.h>
#include <memalign.h>

u64 btrfs_read_extent_inline(struct btrfs_path *path,
			     struct btrfs_file_extent_item *extent, u64 offset,
			     u64 size, char *out)
{
	u32 clen, dlen, orig_size = size, res;
	const char *cbuf;
	char *dbuf;
	const int data_off = offsetof(struct btrfs_file_extent_item,
				      disk_bytenr);

	clen = btrfs_path_item_size(path) - data_off;
	cbuf = (const char *) extent + data_off;
	dlen = extent->ram_bytes;

	if (offset > dlen)
		return -1ULL;

	if (size > dlen - offset)
		size = dlen - offset;

	if (extent->compression == BTRFS_COMPRESS_NONE) {
		memcpy(out, cbuf + offset, size);
		return size;
	}

	if (dlen > orig_size) {
		dbuf = malloc(dlen);
		if (!dbuf)
			return -1ULL;
	} else {
		dbuf = out;
	}

	res = btrfs_decompress(extent->compression, cbuf, clen, dbuf, dlen);
	if (res == -1 || res != dlen)
		goto err;

	if (dlen > orig_size) {
		memcpy(out, dbuf + offset, size);
		free(dbuf);
	} else if (offset) {
		memmove(out, dbuf + offset, size);
	}

	return size;

err:
	if (dlen > orig_size)
		free(dbuf);
	return -1ULL;
}

u64 btrfs_read_extent_reg(struct btrfs_path *path,
			  struct btrfs_file_extent_item *extent, u64 offset,
			  u64 size, char *out)
{
	u64 physical, clen, dlen, orig_size = size;
	u32 res;
	char *cbuf, *dbuf;

	clen = extent->disk_num_bytes;
	dlen = extent->num_bytes;

	if (offset > dlen)
		return -1ULL;

	if (size > dlen - offset)
		size = dlen - offset;

	physical = btrfs_map_logical_to_physical(extent->disk_bytenr);
	if (physical == -1ULL)
		return -1ULL;

	if (extent->compression == BTRFS_COMPRESS_NONE) {
		physical += extent->offset + offset;
		if (!btrfs_devread(physical, size, out))
			return -1ULL;

		return size;
	}

	cbuf = malloc_cache_aligned(dlen > size ? clen + dlen : clen);
	if (!cbuf)
		return -1ULL;

	if (dlen > orig_size)
		dbuf = cbuf + clen;
	else
		dbuf = out;

	if (!btrfs_devread(physical, clen, cbuf))
		goto err;

	res = btrfs_decompress(extent->compression, cbuf, clen, dbuf, dlen);
	if (res == -1)
		goto err;

	if (dlen > orig_size)
		memcpy(out, dbuf + offset, size);
	else
		memmove(out, dbuf + offset, size);

	free(cbuf);
	return res;

err:
	free(cbuf);
	return -1ULL;
}
