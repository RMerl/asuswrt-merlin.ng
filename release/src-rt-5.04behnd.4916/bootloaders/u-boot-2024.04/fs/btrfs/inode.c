// SPDX-License-Identifier: GPL-2.0+
/*
 * BTRFS filesystem implementation for U-Boot
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#include <linux/kernel.h>
#include <malloc.h>
#include <memalign.h>
#include "btrfs.h"
#include "disk-io.h"
#include "volumes.h"

/*
 * Read the content of symlink inode @ino of @root, into @target.
 * NOTE: @target will not be \0 termiated, caller should handle it properly.
 *
 * Return the number of read data.
 * Return <0 for error.
 */
int btrfs_readlink(struct btrfs_root *root, u64 ino, char *target)
{
	struct btrfs_path path;
	struct btrfs_key key;
	struct btrfs_file_extent_item *fi;
	int ret;

	key.objectid = ino;
	key.type = BTRFS_EXTENT_DATA_KEY;
	key.offset = 0;
	btrfs_init_path(&path);

	ret = btrfs_search_slot(NULL, root, &key, &path, 0, 0);
	if (ret < 0)
		return ret;
	if (ret > 0) {
		ret = -ENOENT;
		goto out;
	}
	fi = btrfs_item_ptr(path.nodes[0], path.slots[0],
			    struct btrfs_file_extent_item);
	if (btrfs_file_extent_type(path.nodes[0], fi) !=
	    BTRFS_FILE_EXTENT_INLINE) {
		ret = -EUCLEAN;
		error("Extent for symlink %llu must be INLINE type!", ino);
		goto out;
	}
	if (btrfs_file_extent_compression(path.nodes[0], fi) !=
	    BTRFS_COMPRESS_NONE) {
		ret = -EUCLEAN;
		error("Extent for symlink %llu must not be compressed!", ino);
		goto out;
	}
	if (btrfs_file_extent_ram_bytes(path.nodes[0], fi) >=
	    root->fs_info->sectorsize) {
		ret = -EUCLEAN;
		error("Symlink %llu extent data too large (%llu)!\n",
			ino, btrfs_file_extent_ram_bytes(path.nodes[0], fi));
		goto out;
	}
	read_extent_buffer(path.nodes[0], target,
			btrfs_file_extent_inline_start(fi),
			btrfs_file_extent_ram_bytes(path.nodes[0], fi));
	ret = btrfs_file_extent_ram_bytes(path.nodes[0], fi);
out:
	btrfs_release_path(&path);
	return ret;
}

static int lookup_root_ref(struct btrfs_fs_info *fs_info,
			   u64 rootid, u64 *root_ret, u64 *dir_ret)
{
	struct btrfs_root *root = fs_info->tree_root;
	struct btrfs_root_ref *root_ref;
	struct btrfs_path path;
	struct btrfs_key key;
	int ret;

	btrfs_init_path(&path);
	key.objectid = rootid;
	key.type = BTRFS_ROOT_BACKREF_KEY;
	key.offset = (u64)-1;

	ret = btrfs_search_slot(NULL, root, &key, &path, 0, 0);
	if (ret < 0)
		return ret;
	/* Should not happen */
	if (ret == 0) {
		ret = -EUCLEAN;
		goto out;
	}
	ret = btrfs_previous_item(root, &path, rootid, BTRFS_ROOT_BACKREF_KEY);
	if (ret < 0)
		goto out;
	if (ret > 0) {
		ret = -ENOENT;
		goto out;
	}
	btrfs_item_key_to_cpu(path.nodes[0], &key, path.slots[0]);
	root_ref = btrfs_item_ptr(path.nodes[0], path.slots[0],
				  struct btrfs_root_ref);
	*root_ret = key.offset;
	*dir_ret = btrfs_root_ref_dirid(path.nodes[0], root_ref);
out:
	btrfs_release_path(&path);
	return ret;
}

/*
 * To get the parent inode of @ino of @root.
 *
 * @root_ret and @ino_ret will be filled.
 *
 * NOTE: This function is not reliable. It can only get one parent inode.
 * The get the proper parent inode, we need a full VFS inodes stack to
 * resolve properly.
 */
static int get_parent_inode(struct btrfs_root *root, u64 ino,
			    struct btrfs_root **root_ret, u64 *ino_ret)
{
	struct btrfs_fs_info *fs_info = root->fs_info;
	struct btrfs_path path;
	struct btrfs_key key;
	int ret;

	if (ino == BTRFS_FIRST_FREE_OBJECTID) {
		u64 parent_root = -1;

		/* It's top level already, no more parent */
		if (root->root_key.objectid == BTRFS_FS_TREE_OBJECTID) {
			*root_ret = fs_info->fs_root;
			*ino_ret = BTRFS_FIRST_FREE_OBJECTID;
			return 0;
		}

		ret = lookup_root_ref(fs_info, root->root_key.objectid,
				      &parent_root, ino_ret);
		if (ret < 0)
			return ret;

		key.objectid = parent_root;
		key.type = BTRFS_ROOT_ITEM_KEY;
		key.offset = (u64)-1;
		*root_ret = btrfs_read_fs_root(fs_info, &key);
		if (IS_ERR(*root_ret))
			return PTR_ERR(*root_ret);

		return 0;
	}

	btrfs_init_path(&path);
	key.objectid = ino;
	key.type = BTRFS_INODE_REF_KEY;
	key.offset = (u64)-1;

	ret = btrfs_search_slot(NULL, root, &key, &path, 0, 0);
	if (ret < 0)
		return ret;
	/* Should not happen */
	if (ret == 0) {
		ret = -EUCLEAN;
		goto out;
	}
	ret = btrfs_previous_item(root, &path, ino, BTRFS_INODE_REF_KEY);
	if (ret < 0)
		goto out;
	if (ret > 0) {
		ret = -ENOENT;
		goto out;
	}
	btrfs_item_key_to_cpu(path.nodes[0], &key, path.slots[0]);
	*root_ret = root;
	*ino_ret = key.offset;
out:
	btrfs_release_path(&path);
	return ret;
}

static inline int next_length(const char *path)
{
	int res = 0;
	while (*path != '\0' && *path != '/') {
		++res;
		++path;
		if (res > BTRFS_NAME_LEN)
			break;
	}
	return res;
}

static inline const char *skip_current_directories(const char *cur)
{
	while (1) {
		if (cur[0] == '/')
			++cur;
		else if (cur[0] == '.' && cur[1] == '/')
			cur += 2;
		else
			break;
	}

	return cur;
}

/*
 * Resolve one filename of @ino of @root.
 *
 * key_ret:	The child key (either INODE_ITEM or ROOT_ITEM type)
 * type_ret:	BTRFS_FT_* of the child inode.
 *
 * Return 0 with above members filled.
 * Return <0 for error.
 */
static int resolve_one_filename(struct btrfs_root *root, u64 ino,
				const char *name, int namelen,
				struct btrfs_key *key_ret, u8 *type_ret)
{
	struct btrfs_dir_item *dir_item;
	struct btrfs_path path;
	int ret = 0;

	btrfs_init_path(&path);

	dir_item = btrfs_lookup_dir_item(NULL, root, &path, ino, name,
					 namelen, 0);
	if (IS_ERR(dir_item)) {
		ret = PTR_ERR(dir_item);
		goto out;
	}

	btrfs_dir_item_key_to_cpu(path.nodes[0], dir_item, key_ret);
	*type_ret = btrfs_dir_type(path.nodes[0], dir_item);
out:
	btrfs_release_path(&path);
	return ret;
}

/*
 * Resolve a full path @filename. The start point is @ino of @root.
 *
 * The result will be filled into @root_ret, @ino_ret and @type_ret.
 */
int btrfs_lookup_path(struct btrfs_root *root, u64 ino, const char *filename,
			struct btrfs_root **root_ret, u64 *ino_ret,
			u8 *type_ret, int symlink_limit)
{
	struct btrfs_fs_info *fs_info = root->fs_info;
	struct btrfs_root *next_root;
	struct btrfs_key key;
	const char *cur = filename;
	u64 next_ino;
	u8 next_type;
	u8 type = BTRFS_FT_UNKNOWN;
	int len;
	int ret = 0;

	/* If the path is absolute path, also search from fs root */
	if (*cur == '/') {
		root = fs_info->fs_root;
		ino = btrfs_root_dirid(&root->root_item);
		type = BTRFS_FT_DIR;
	}

	while (*cur != '\0') {
		cur = skip_current_directories(cur);

		len = next_length(cur);
		if (len > BTRFS_NAME_LEN) {
			error("%s: Name too long at \"%.*s\"", __func__,
			       BTRFS_NAME_LEN, cur);
			return -ENAMETOOLONG;
		}

		if (len == 1 && cur[0] == '.')
			break;

		if (len == 2 && cur[0] == '.' && cur[1] == '.') {
			/* Go one level up */
			ret = get_parent_inode(root, ino, &next_root, &next_ino);
			if (ret < 0)
				return ret;
			root = next_root;
			ino = next_ino;
			goto next;
		}

		if (!*cur)
			break;

		ret = resolve_one_filename(root, ino, cur, len, &key, &type);
		if (ret < 0)
			return ret;

		if (key.type == BTRFS_ROOT_ITEM_KEY) {
			/* Child inode is a subvolume */

			next_root = btrfs_read_fs_root(fs_info, &key);
			if (IS_ERR(next_root))
				return PTR_ERR(next_root);
			root = next_root;
			ino = btrfs_root_dirid(&root->root_item);
		} else if (type == BTRFS_FT_SYMLINK && symlink_limit >= 0) {
			/* Child inode is a symlink */

			char *target;

			if (symlink_limit == 0) {
				error("%s: Too much symlinks!", __func__);
				return -EMLINK;
			}
			target = malloc(fs_info->sectorsize);
			if (!target)
				return -ENOMEM;
			ret = btrfs_readlink(root, key.objectid, target);
			if (ret < 0) {
				free(target);
				return ret;
			}
			target[ret] = '\0';

			ret = btrfs_lookup_path(root, ino, target, &next_root,
						&next_ino, &next_type,
						symlink_limit);
			if (ret < 0)
				return ret;
			root = next_root;
			ino = next_ino;
			type = next_type;
		} else {
			/* Child inode is an inode */
			ino = key.objectid;
		}
next:
		cur += len;
	}

	/* We haven't found anything, but still get no error? */
	if (type == BTRFS_FT_UNKNOWN && !ret)
		ret = -EUCLEAN;

	if (!ret) {
		*root_ret = root;
		*ino_ret = ino;
		*type_ret = type;
	}

	return ret;
}

/*
 * Read out inline extent.
 *
 * Since inline extent should only exist for offset 0, no need for extra
 * parameters.
 * Truncating should be handled by the caller.
 *
 * Return the number of bytes read.
 * Return <0 for error.
 */
int btrfs_read_extent_inline(struct btrfs_path *path,
			     struct btrfs_file_extent_item *fi, char *dest)
{
	struct extent_buffer *leaf = path->nodes[0];
	int slot = path->slots[0];
	char *cbuf = NULL;
	char *dbuf = NULL;
	u32 csize;
	u32 dsize;
	int ret;

	csize = btrfs_file_extent_inline_item_len(leaf, btrfs_item_nr(slot));
	if (btrfs_file_extent_compression(leaf, fi) == BTRFS_COMPRESS_NONE) {
		/* Uncompressed, just read it out */
		read_extent_buffer(leaf, dest,
				btrfs_file_extent_inline_start(fi),
				csize);
		return csize;
	}

	/* Compressed extent, prepare the compressed and data buffer */
	dsize = btrfs_file_extent_ram_bytes(leaf, fi);
	cbuf = malloc(csize);
	dbuf = malloc(dsize);
	if (!cbuf || !dbuf) {
		ret = -ENOMEM;
		goto out;
	}
	read_extent_buffer(leaf, cbuf, btrfs_file_extent_inline_start(fi),
			   csize);
	ret = btrfs_decompress(btrfs_file_extent_compression(leaf, fi),
			       cbuf, csize, dbuf, dsize);
	if (ret < 0) {
		ret = -EIO;
		goto out;
	}
	/*
	 * The compressed part ends before sector boundary, the remaining needs
	 * to be zeroed out.
	 */
	if (ret < dsize)
		memset(dbuf + ret, 0, dsize - ret);
	memcpy(dest, dbuf, dsize);
	ret = dsize;
out:
	free(cbuf);
	free(dbuf);
	return ret;
}

/*
 * Read out regular extent.
 *
 * Truncating should be handled by the caller.
 *
 * @offset and @len should not cross the extent boundary.
 * Return the number of bytes read.
 * Return <0 for error.
 */
int btrfs_read_extent_reg(struct btrfs_path *path,
			  struct btrfs_file_extent_item *fi, u64 offset,
			  int len, char *dest)
{
	struct extent_buffer *leaf = path->nodes[0];
	struct btrfs_fs_info *fs_info = leaf->fs_info;
	struct btrfs_key key;
	u64 extent_num_bytes;
	u64 disk_bytenr;
	u64 read;
	char *cbuf = NULL;
	char *dbuf = NULL;
	u32 csize;
	u32 dsize;
	bool finished = false;
	int num_copies;
	int i;
	int slot = path->slots[0];
	int ret;

	btrfs_item_key_to_cpu(leaf, &key, slot);
	extent_num_bytes = btrfs_file_extent_num_bytes(leaf, fi);
	ASSERT(IS_ALIGNED(offset, fs_info->sectorsize) &&
	       IS_ALIGNED(len, fs_info->sectorsize));
	ASSERT(offset >= key.offset &&
	       offset + len <= key.offset + extent_num_bytes);

	/* Preallocated or hole , fill @dest with zero */
	if (btrfs_file_extent_type(leaf, fi) == BTRFS_FILE_EXTENT_PREALLOC ||
	    btrfs_file_extent_disk_bytenr(leaf, fi) == 0) {
		memset(dest, 0, len);
		return len;
	}

	if (btrfs_file_extent_compression(leaf, fi) == BTRFS_COMPRESS_NONE) {
		u64 logical;

		logical = btrfs_file_extent_disk_bytenr(leaf, fi) +
			  btrfs_file_extent_offset(leaf, fi) +
			  offset - key.offset;
		read = len;

		num_copies = btrfs_num_copies(fs_info, logical, len);
		for (i = 1; i <= num_copies; i++) {
			ret = read_extent_data(fs_info, dest, logical, &read, i);
			if (ret < 0 || read != len)
				continue;
			finished = true;
			break;
		}
		if (!finished)
			return -EIO;
		return len;
	}

	csize = btrfs_file_extent_disk_num_bytes(leaf, fi);
	dsize = btrfs_file_extent_ram_bytes(leaf, fi);
	disk_bytenr = btrfs_file_extent_disk_bytenr(leaf, fi);
	num_copies = btrfs_num_copies(fs_info, disk_bytenr, csize);

	cbuf = malloc_cache_aligned(csize);
	dbuf = malloc_cache_aligned(dsize);
	if (!cbuf || !dbuf) {
		ret = -ENOMEM;
		goto out;
	}
	/* For compressed extent, we must read the whole on-disk extent */
	for (i = 1; i <= num_copies; i++) {
		read = csize;
		ret = read_extent_data(fs_info, cbuf, disk_bytenr,
				       &read, i);
		if (ret < 0 || read != csize)
			continue;
		finished = true;
		break;
	}
	if (!finished) {
		ret = -EIO;
		goto out;
	}

	ret = btrfs_decompress(btrfs_file_extent_compression(leaf, fi), cbuf,
			       csize, dbuf, dsize);
	if (ret < 0) {
		ret = -EIO;
		goto out;
	}
	/*
	 * The compressed part ends before sector boundary, the remaining needs
	 * to be zeroed out.
	 */
	if (ret < dsize)
		memset(dbuf + ret, 0, dsize - ret);
	/* Then copy the needed part */
	memcpy(dest,
	       dbuf + btrfs_file_extent_offset(leaf, fi) + offset - key.offset,
	       len);
	ret = len;
out:
	free(cbuf);
	free(dbuf);
	return ret;
}

/*
 * Get the first file extent that covers bytenr @file_offset.
 *
 * @file_offset must be aligned to sectorsize.
 *
 * return 0 for found, and path points to the file extent.
 * return >0 for not found, and fill @next_offset.
 * @next_offset can be 0 if there is no next file extent.
 * return <0 for error.
 */
static int lookup_data_extent(struct btrfs_root *root, struct btrfs_path *path,
			      u64 ino, u64 file_offset, u64 *next_offset)
{
	struct btrfs_key key;
	struct btrfs_file_extent_item *fi;
	u8 extent_type;
	int ret = 0;

	ASSERT(IS_ALIGNED(file_offset, root->fs_info->sectorsize));
	key.objectid = ino;
	key.type = BTRFS_EXTENT_DATA_KEY;
	key.offset = file_offset;

	ret = btrfs_search_slot(NULL, root, &key, path, 0, 0);
	/* Error or we're already at the file extent */
	if (ret <= 0)
		return ret;
	/* Check previous file extent */
	ret = btrfs_previous_item(root, path, ino, BTRFS_EXTENT_DATA_KEY);
	if (ret < 0)
		return ret;
	if (ret > 0)
		goto check_next;
	/* Now the key.offset must be smaller than @file_offset */
	btrfs_item_key_to_cpu(path->nodes[0], &key, path->slots[0]);
	if (key.objectid != ino ||
	    key.type != BTRFS_EXTENT_DATA_KEY)
		goto check_next;

	fi = btrfs_item_ptr(path->nodes[0], path->slots[0],
			    struct btrfs_file_extent_item);
	extent_type = btrfs_file_extent_type(path->nodes[0], fi);
	if (extent_type == BTRFS_FILE_EXTENT_INLINE) {
		if (file_offset == 0)
			return 0;
		/* Inline extent should be the only extent, no next extent. */
		*next_offset = 0;
		return 1;
	}

	/* This file extent covers @file_offset */
	if (key.offset <= file_offset && key.offset +
	    btrfs_file_extent_num_bytes(path->nodes[0], fi) > file_offset)
		return 0;
check_next:
	ret = btrfs_next_item(root, path);
	if (ret < 0)
		return ret;
	if (ret > 0) {
		*next_offset = 0;
		return 1;
	}

	btrfs_item_key_to_cpu(path->nodes[0], &key, path->slots[0]);
	fi = btrfs_item_ptr(path->nodes[0], path->slots[0],
			    struct btrfs_file_extent_item);
	/* Next next data extent */
	if (key.objectid != ino ||
	    key.type != BTRFS_EXTENT_DATA_KEY) {
		*next_offset = 0;
		return 1;
	}
	/* Current file extent already beyond @file_offset */
	if (key.offset > file_offset) {
		*next_offset = key.offset;
		return 1;
	}
	/* This file extent covers @file_offset */
	if (key.offset <= file_offset && key.offset +
	    btrfs_file_extent_num_bytes(path->nodes[0], fi) > file_offset)
		return 0;
	/* This file extent ends before @file_offset, check next */
	ret = btrfs_next_item(root, path);
	if (ret < 0)
		return ret;
	if (ret > 0) {
		*next_offset = 0;
		return 1;
	}
	btrfs_item_key_to_cpu(path->nodes[0], &key, path->slots[0]);
	if (key.type != BTRFS_EXTENT_DATA_KEY || key.objectid != ino) {
		*next_offset = 0;
		return 1;
	}
	*next_offset = key.offset;
	return 1;
}

static int read_and_truncate_page(struct btrfs_path *path,
				  struct btrfs_file_extent_item *fi,
				  int start, int len, char *dest)
{
	struct extent_buffer *leaf = path->nodes[0];
	struct btrfs_fs_info *fs_info = leaf->fs_info;
	u64 aligned_start = round_down(start, fs_info->sectorsize);
	u8 extent_type;
	char *buf;
	int page_off = start - aligned_start;
	int page_len = fs_info->sectorsize - page_off;
	int ret;

	ASSERT(start + len <= aligned_start + fs_info->sectorsize);
	buf = malloc_cache_aligned(fs_info->sectorsize);
	if (!buf)
		return -ENOMEM;

	extent_type = btrfs_file_extent_type(leaf, fi);
	if (extent_type == BTRFS_FILE_EXTENT_INLINE) {
		ret = btrfs_read_extent_inline(path, fi, buf);
		memcpy(dest, buf + page_off, min(page_len, ret));
		free(buf);
		return len;
	}

	ret = btrfs_read_extent_reg(path, fi,
			round_down(start, fs_info->sectorsize),
			fs_info->sectorsize, buf);
	if (ret < 0) {
		free(buf);
		return ret;
	}
	memcpy(dest, buf + page_off, page_len);
	free(buf);
	return len;
}

int btrfs_file_read(struct btrfs_root *root, u64 ino, u64 file_offset, u64 len,
		    char *dest)
{
	struct btrfs_fs_info *fs_info = root->fs_info;
	struct btrfs_file_extent_item *fi;
	struct btrfs_path path;
	struct btrfs_key key;
	u64 aligned_start = round_down(file_offset, fs_info->sectorsize);
	u64 aligned_end = round_down(file_offset + len, fs_info->sectorsize);
	u64 next_offset;
	u64 cur = aligned_start;
	int ret = 0;

	btrfs_init_path(&path);

	/* Set the whole dest all zero, so we won't need to bother holes */
	memset(dest, 0, len);

	/* Read out the leading unaligned part */
	if (aligned_start != file_offset) {
		ret = lookup_data_extent(root, &path, ino, aligned_start,
					 &next_offset);
		if (ret < 0)
			goto out;
		if (ret == 0) {
			/* Read the unaligned part out*/
			fi = btrfs_item_ptr(path.nodes[0], path.slots[0],
					struct btrfs_file_extent_item);
			ret = read_and_truncate_page(&path, fi, file_offset,
					round_up(file_offset, fs_info->sectorsize) -
					file_offset, dest);
			if (ret < 0)
				goto out;
			cur += fs_info->sectorsize;
		} else {
			/* The whole file is a hole */
			if (!next_offset) {
				memset(dest, 0, len);
				return len;
			}
			cur = next_offset;
		}
	}

	/* Read the aligned part */
	while (cur < aligned_end) {
		u64 extent_num_bytes;
		u8 type;

		btrfs_release_path(&path);
		ret = lookup_data_extent(root, &path, ino, cur, &next_offset);
		if (ret < 0)
			goto out;
		if (ret > 0) {
			/* No next, direct exit */
			if (!next_offset) {
				ret = 0;
				goto out;
			}
			/*
			 * Find a extent gap, mostly caused by NO_HOLE feature.
			 * Just to next offset directly.
			 */
			if (next_offset > cur) {
				cur = next_offset;
				continue;
			}
		}
		fi = btrfs_item_ptr(path.nodes[0], path.slots[0],
				    struct btrfs_file_extent_item);
		btrfs_item_key_to_cpu(path.nodes[0], &key, path.slots[0]);
		type = btrfs_file_extent_type(path.nodes[0], fi);
		if (type == BTRFS_FILE_EXTENT_INLINE) {
			ret = btrfs_read_extent_inline(&path, fi, dest);
			goto out;
		}
		/* Skip holes, as we have zeroed the dest */
		if (type == BTRFS_FILE_EXTENT_PREALLOC ||
		    btrfs_file_extent_disk_bytenr(path.nodes[0], fi) == 0) {
			cur = key.offset + btrfs_file_extent_num_bytes(
					path.nodes[0], fi);
			continue;
		}

		/* Read the remaining part of the extent */
		extent_num_bytes = btrfs_file_extent_num_bytes(path.nodes[0],
							       fi);
		ret = btrfs_read_extent_reg(&path, fi, cur,
				min(extent_num_bytes, aligned_end - cur),
				dest + cur - file_offset);
		if (ret < 0)
			goto out;
		cur += min(extent_num_bytes, aligned_end - cur);
	}

	/* Read the tailing unaligned part*/
	if (file_offset + len != aligned_end) {
		btrfs_release_path(&path);
		ret = lookup_data_extent(root, &path, ino, aligned_end,
					 &next_offset);
		/* <0 is error, >0 means no extent */
		if (ret)
			goto out;
		fi = btrfs_item_ptr(path.nodes[0], path.slots[0],
				    struct btrfs_file_extent_item);
		ret = read_and_truncate_page(&path, fi, aligned_end,
				file_offset + len - aligned_end,
				dest + aligned_end - file_offset);
	}
out:
	btrfs_release_path(&path);
	if (ret < 0)
		return ret;
	return len;
}
