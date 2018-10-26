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

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_ANDROID_OS
#include <linux/capability.h>
#else
#include <private/android_filesystem_capability.h>
#endif

#define XATTR_SELINUX_SUFFIX "selinux"
#define XATTR_CAPS_SUFFIX "capability"

#include "ext4_utils.h"
#include "ext4.h"
#include "make_ext4fs.h"
#include "allocate.h"
#include "contents.h"
#include "extent.h"
#include "indirect.h"
#include "xattr.h"

#ifdef USE_MINGW
#define S_IFLNK 0  /* used by make_link, not needed under mingw */
#endif

static u32 dentry_size(u32 entries, struct dentry *dentries)
{
	u32 len = 24;
	unsigned int i;
	unsigned int dentry_len;

	for (i = 0; i < entries; i++) {
		dentry_len = 8 + ALIGN(strlen(dentries[i].filename), 4);
		if (len % info.block_size + dentry_len > info.block_size)
			len += info.block_size - (len % info.block_size);
		len += dentry_len;
	}

	return len;
}

static struct ext4_dir_entry_2 *add_dentry(u8 *data, u32 *offset,
		struct ext4_dir_entry_2 *prev, u32 inode, const char *name,
		u8 file_type)
{
	u8 name_len = strlen(name);
	u16 rec_len = 8 + ALIGN(name_len, 4);
	struct ext4_dir_entry_2 *dentry;

	u32 start_block = *offset / info.block_size;
	u32 end_block = (*offset + rec_len - 1) / info.block_size;
	if (start_block != end_block) {
		/* Adding this dentry will cross a block boundary, so pad the previous
		   dentry to the block boundary */
		if (!prev)
			critical_error("no prev");
		prev->rec_len += end_block * info.block_size - *offset;
		*offset = end_block * info.block_size;
	}

	dentry = (struct ext4_dir_entry_2 *)(data + *offset);
	dentry->inode = inode;
	dentry->rec_len = rec_len;
	dentry->name_len = name_len;
	dentry->file_type = file_type;
	memcpy(dentry->name, name, name_len);

	*offset += rec_len;
	return dentry;
}

/* Creates a directory structure for an array of directory entries, dentries,
   and stores the location of the structure in an inode.  The new inode's
   .. link is set to dir_inode_num.  Stores the location of the inode number
   of each directory entry into dentries[i].inode, to be filled in later
   when the inode for the entry is allocated.  Returns the inode number of the
   new directory */
u32 make_directory(u32 dir_inode_num, u32 entries, struct dentry *dentries,
	u32 dirs)
{
	struct ext4_inode *inode;
	u32 blocks;
	u32 len;
	u32 offset = 0;
	u32 inode_num;
	u8 *data;
	unsigned int i;
	struct ext4_dir_entry_2 *dentry;

	blocks = DIV_ROUND_UP(dentry_size(entries, dentries), info.block_size);
	len = blocks * info.block_size;

	if (dir_inode_num) {
		inode_num = allocate_inode(info);
	} else {
		dir_inode_num = EXT4_ROOT_INO;
		inode_num = EXT4_ROOT_INO;
	}

	if (inode_num == EXT4_ALLOCATE_FAILED) {
		error("failed to allocate inode\n");
		return EXT4_ALLOCATE_FAILED;
	}

	add_directory(inode_num);

	inode = get_inode(inode_num);
	if (inode == NULL) {
		error("failed to get inode %u", inode_num);
		return EXT4_ALLOCATE_FAILED;
	}

	data = inode_allocate_data_extents(inode, len, len);
	if (data == NULL) {
		error("failed to allocate %u extents", len);
		return EXT4_ALLOCATE_FAILED;
	}

	inode->i_mode = S_IFDIR;
	inode->i_links_count = dirs + 2;
	inode->i_flags |= aux_info.default_i_flags;

	dentry = NULL;

	dentry = add_dentry(data, &offset, NULL, inode_num, ".", EXT4_FT_DIR);
	if (!dentry) {
		error("failed to add . directory");
		return EXT4_ALLOCATE_FAILED;
	}

	dentry = add_dentry(data, &offset, dentry, dir_inode_num, "..", EXT4_FT_DIR);
	if (!dentry) {
		error("failed to add .. directory");
		return EXT4_ALLOCATE_FAILED;
	}

	for (i = 0; i < entries; i++) {
		dentry = add_dentry(data, &offset, dentry, 0,
				dentries[i].filename, dentries[i].file_type);
		if (offset > len || (offset == len && i != entries - 1))
			critical_error("internal error: dentry for %s ends at %d, past %d\n",
				dentries[i].filename, offset, len);
		dentries[i].inode = &dentry->inode;
		if (!dentry) {
			error("failed to add directory");
			return EXT4_ALLOCATE_FAILED;
		}
	}

	/* pad the last dentry out to the end of the block */
	dentry->rec_len += len - offset;

	return inode_num;
}

/* Creates a file on disk.  Returns the inode number of the new file */
u32 make_file(const char *filename, u64 len)
{
	struct ext4_inode *inode;
	u32 inode_num;

	inode_num = allocate_inode(info);
	if (inode_num == EXT4_ALLOCATE_FAILED) {
		error("failed to allocate inode\n");
		return EXT4_ALLOCATE_FAILED;
	}

	inode = get_inode(inode_num);
	if (inode == NULL) {
		error("failed to get inode %u", inode_num);
		return EXT4_ALLOCATE_FAILED;
	}

	if (len > 0)
		inode_allocate_file_extents(inode, len, filename);

	inode->i_mode = S_IFREG;
	inode->i_links_count = 1;
	inode->i_flags |= aux_info.default_i_flags;

	return inode_num;
}

/* Creates a file on disk.  Returns the inode number of the new file */
u32 make_link(const char *link)
{
	struct ext4_inode *inode;
	u32 inode_num;
	u32 len = strlen(link);

	inode_num = allocate_inode(info);
	if (inode_num == EXT4_ALLOCATE_FAILED) {
		error("failed to allocate inode\n");
		return EXT4_ALLOCATE_FAILED;
	}

	inode = get_inode(inode_num);
	if (inode == NULL) {
		error("failed to get inode %u", inode_num);
		return EXT4_ALLOCATE_FAILED;
	}

	inode->i_mode = S_IFLNK;
	inode->i_links_count = 1;
	inode->i_flags |= aux_info.default_i_flags;
	inode->i_size_lo = len;

	if (len + 1 <= sizeof(inode->i_block)) {
		/* Fast symlink */
		memcpy((char*)inode->i_block, link, len);
	} else {
		u8 *data = inode_allocate_data_indirect(inode, info.block_size, info.block_size);
		memcpy(data, link, len);
		inode->i_blocks_lo = info.block_size / 512;
	}

	return inode_num;
}

int inode_set_permissions(u32 inode_num, u16 mode, u16 uid, u16 gid, u32 mtime)
{
	struct ext4_inode *inode = get_inode(inode_num);

	if (!inode)
		return -1;

	inode->i_mode |= mode;
	inode->i_uid = uid;
	inode->i_gid = gid;
	inode->i_mtime = mtime;
	inode->i_atime = mtime;
	inode->i_ctime = mtime;

	return 0;
}

/*
 * Returns the amount of free space available in the specified
 * xattr region
 */
static size_t xattr_free_space(struct ext4_xattr_entry *entry, char *end)
{
	while(!IS_LAST_ENTRY(entry) && (((char *) entry) < end)) {
		end   -= EXT4_XATTR_SIZE(le32_to_cpu(entry->e_value_size));
		entry  = EXT4_XATTR_NEXT(entry);
	}

	if (((char *) entry) > end) {
		error("unexpected read beyond end of xattr space");
		return 0;
	}

	return end - ((char *) entry);
}

/*
 * Returns a pointer to the free space immediately after the
 * last xattr element
 */
static struct ext4_xattr_entry* xattr_get_last(struct ext4_xattr_entry *entry)
{
	for (; !IS_LAST_ENTRY(entry); entry = EXT4_XATTR_NEXT(entry)) {
		// skip entry
	}
	return entry;
}

/*
 * assert that the elements in the ext4 xattr section are in sorted order
 *
 * The ext4 filesystem requires extended attributes to be sorted when
 * they're not stored in the inode. The kernel ext4 code uses the following
 * sorting algorithm:
 *
 * 1) First sort extended attributes by their name_index. For example,
 *    EXT4_XATTR_INDEX_USER (1) comes before EXT4_XATTR_INDEX_SECURITY (6).
 * 2) If the name_indexes are equal, then sorting is based on the length
 *    of the name. For example, XATTR_SELINUX_SUFFIX ("selinux") comes before
 *    XATTR_CAPS_SUFFIX ("capability") because "selinux" is shorter than "capability"
 * 3) If the name_index and name_length are equal, then memcmp() is used to determine
 *    which name comes first. For example, "selinux" would come before "yelinux".
 *
 * This method is intended to implement the sorting function defined in
 * the Linux kernel file fs/ext4/xattr.c function ext4_xattr_find_entry().
 */
static void xattr_assert_sane(struct ext4_xattr_entry *entry)
{
	for( ; !IS_LAST_ENTRY(entry); entry = EXT4_XATTR_NEXT(entry)) {
		struct ext4_xattr_entry *next = EXT4_XATTR_NEXT(entry);
		if (IS_LAST_ENTRY(next)) {
			return;
		}

		int cmp = next->e_name_index - entry->e_name_index;
		if (cmp == 0)
			cmp = next->e_name_len - entry->e_name_len;
		if (cmp == 0)
			cmp = memcmp(next->e_name, entry->e_name, next->e_name_len);
		if (cmp < 0) {
			error("BUG: extended attributes are not sorted\n");
			return;
		}
		if (cmp == 0) {
			error("BUG: duplicate extended attributes detected\n");
			return;
		}
	}
}

#define NAME_HASH_SHIFT 5
#define VALUE_HASH_SHIFT 16

static void ext4_xattr_hash_entry(struct ext4_xattr_header *header,
		struct ext4_xattr_entry *entry)
{
	__u32 hash = 0;
	char *name = entry->e_name;
	int n;

	for (n = 0; n < entry->e_name_len; n++) {
		hash = (hash << NAME_HASH_SHIFT) ^
			(hash >> (8*sizeof(hash) - NAME_HASH_SHIFT)) ^
			*name++;
	}

	if (entry->e_value_block == 0 && entry->e_value_size != 0) {
		__le32 *value = (__le32 *)((char *)header +
			le16_to_cpu(entry->e_value_offs));
		for (n = (le32_to_cpu(entry->e_value_size) +
			EXT4_XATTR_ROUND) >> EXT4_XATTR_PAD_BITS; n; n--) {
			hash = (hash << VALUE_HASH_SHIFT) ^
				(hash >> (8*sizeof(hash) - VALUE_HASH_SHIFT)) ^
				le32_to_cpu(*value++);
		}
	}
	entry->e_hash = cpu_to_le32(hash);
}

#undef NAME_HASH_SHIFT
#undef VALUE_HASH_SHIFT

static struct ext4_xattr_entry* xattr_addto_range(
		void *block_start,
		void *block_end,
		struct ext4_xattr_entry *first,
		int name_index,
		const char *name,
		const void *value,
		size_t value_len)
{
	size_t name_len = strlen(name);
	if (name_len > 255)
		return NULL;

	size_t available_size = xattr_free_space(first, block_end);
	size_t needed_size = EXT4_XATTR_LEN(name_len) + EXT4_XATTR_SIZE(value_len);

	if (needed_size > available_size)
		return NULL;

	struct ext4_xattr_entry *new_entry = xattr_get_last(first);
	memset(new_entry, 0, EXT4_XATTR_LEN(name_len));

	new_entry->e_name_len = name_len;
	new_entry->e_name_index = name_index;
	memcpy(new_entry->e_name, name, name_len);
	new_entry->e_value_block = 0;
	new_entry->e_value_size = cpu_to_le32(value_len);

	char *val = (char *) new_entry + available_size - EXT4_XATTR_SIZE(value_len);
	size_t e_value_offs = val - (char *) block_start;

	new_entry->e_value_offs = cpu_to_le16(e_value_offs);
	memset(val, 0, EXT4_XATTR_SIZE(value_len));
	memcpy(val, value, value_len);

	xattr_assert_sane(first);
	return new_entry;
}

static int xattr_addto_inode(struct ext4_inode *inode, int name_index,
		const char *name, const void *value, size_t value_len)
{
	struct ext4_xattr_ibody_header *hdr = (struct ext4_xattr_ibody_header *) (inode + 1);
	struct ext4_xattr_entry *first = (struct ext4_xattr_entry *) (hdr + 1);
	char *block_end = ((char *) inode) + info.inode_size;

	struct ext4_xattr_entry *result =
		xattr_addto_range(first, block_end, first, name_index, name, value, value_len);

	if (result == NULL)
		return -1;

	hdr->h_magic = cpu_to_le32(EXT4_XATTR_MAGIC);
	inode->i_extra_isize = cpu_to_le16(sizeof(struct ext4_inode) - EXT4_GOOD_OLD_INODE_SIZE);

	return 0;
}

static int xattr_addto_block(struct ext4_inode *inode, int name_index,
		const char *name, const void *value, size_t value_len)
{
	struct ext4_xattr_header *header = get_xattr_block_for_inode(inode);
	if (!header)
		return -1;

	struct ext4_xattr_entry *first = (struct ext4_xattr_entry *) (header + 1);
	char *block_end = ((char *) header) + info.block_size;

	struct ext4_xattr_entry *result =
		xattr_addto_range(header, block_end, first, name_index, name, value, value_len);

	if (result == NULL)
		return -1;

	ext4_xattr_hash_entry(header, result);
	return 0;
}


static int xattr_add(u32 inode_num, int name_index, const char *name,
		const void *value, size_t value_len)
{
	if (!value)
		return 0;

	struct ext4_inode *inode = get_inode(inode_num);

	if (!inode)
		return -1;

	int result = xattr_addto_inode(inode, name_index, name, value, value_len);
	if (result != 0) {
		result = xattr_addto_block(inode, name_index, name, value, value_len);
	}
	return result;
}

int inode_set_selinux(u32 inode_num, const char *secon)
{
	if (!secon)
		return 0;

	return xattr_add(inode_num, EXT4_XATTR_INDEX_SECURITY,
		XATTR_SELINUX_SUFFIX, secon, strlen(secon) + 1);
}

int inode_set_capabilities(u32 inode_num, uint64_t capabilities) {
	if (capabilities == 0)
		return 0;

	struct vfs_cap_data cap_data;
	memset(&cap_data, 0, sizeof(cap_data));

	cap_data.magic_etc = VFS_CAP_REVISION | VFS_CAP_FLAGS_EFFECTIVE;
	cap_data.data[0].permitted = (uint32_t) (capabilities & 0xffffffff);
	cap_data.data[0].inheritable = 0;
	cap_data.data[1].permitted = (uint32_t) (capabilities >> 32);
	cap_data.data[1].inheritable = 0;

	return xattr_add(inode_num, EXT4_XATTR_INDEX_SECURITY,
		XATTR_CAPS_SUFFIX, &cap_data, sizeof(cap_data));
}

