/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Functions to convert BTRFS structures from disk to CPU endianness and back.
 *
 * 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 */

#ifndef __BTRFS_CONV_FUNCS_H__
#define __BTRFS_CONV_FUNCS_H__

#include "ctree.h"
#include <u-boot/variadic-macro.h>
#include <asm/byteorder.h>

/* We are using variadic macros and C11 _Generic to achieve compact code.

   We want to define macro DEFINE_CONV(x, ...), where the first argument is the
   name of the structure for which it shall define conversion functions (the
   names of the functions shall be x_to_cpu and x_to_disk), and the other
   arguments are names of the members on which the functions shall do
   endianness conversion. */

#if defined(__LITTLE_ENDIAN)

/* If the target machine is little endian, the conversion functions do
   nothing, since the on disk format is little endian. */

# define DEFINE_CONV(n,...)					\
	static inline struct n *n##_to_disk(struct n * r)	\
	{							\
		return r;					\
	}							\
	static inline struct n *n##_to_cpu(struct n * r)	\
	{							\
		return r;					\
	}

# define DEFINE_CONV_ALT(n,a,...)				\
	static inline struct n *n##_to_disk_##a(struct n * r)	\
	{							\
		return r;					\
	}							\
	static inline struct n *n##_to_cpu_##a(struct n * r)	\
	{							\
		return r;					\
	}

#else /* !defined(__LITTLE_ENDIAN) */

/* Some structures contain not only scalar members, but compound types as well
   (for example, struct btrfs_inode_item contains members of type struct
   btrfs_timespec.

   For these members we want to call the conversion function recursively, so
   first we declare the functions taking pointers to this types (these function
   will be defined later by the DEFINE_CONV macro) and then we define
   correspond functions taking non-pointers, so that they can be used in the
   expansion of the _Generic. */
# define DEFINE_CONV_FOR_STRUCT(n)				\
	static inline struct n * n##_to_disk(struct n *);	\
	static inline struct n * n##_to_cpu(struct n *);	\
	static inline struct n n##_to_disk_v(struct n x) {	\
		return *n##_to_disk(&x);			\
	}							\
	static inline struct n n##_to_cpu_v(struct n x) {	\
		return *n##_to_cpu(&x);				\
	}

DEFINE_CONV_FOR_STRUCT(btrfs_key)
DEFINE_CONV_FOR_STRUCT(btrfs_stripe)
DEFINE_CONV_FOR_STRUCT(btrfs_timespec)
DEFINE_CONV_FOR_STRUCT(btrfs_inode_item)
DEFINE_CONV_FOR_STRUCT(btrfs_root_backup)
DEFINE_CONV_FOR_STRUCT(btrfs_dev_item)

/* Now define the _Generic for both CPU to LE and LE to CPU */
# define DEFINE_CONV_CPU_TO_LE(x)					\
	(d->x) = _Generic((d->x),					\
		__u16: cpu_to_le16,					\
		__u32: cpu_to_le32,					\
		__u64: cpu_to_le64,					\
		struct btrfs_key: btrfs_key_to_disk_v,			\
		struct btrfs_stripe: btrfs_stripe_to_disk_v,		\
		struct btrfs_timespec: btrfs_timespec_to_disk_v,	\
		struct btrfs_inode_item: btrfs_inode_item_to_disk_v,	\
		struct btrfs_root_backup: btrfs_root_backup_to_disk_v,	\
		struct btrfs_dev_item: btrfs_dev_item_to_disk_v		\
		)((d->x));

# define DEFINE_CONV_LE_TO_CPU(x)					\
	(d->x) = _Generic((d->x),					\
		__u16: le16_to_cpu,					\
		__u32: le32_to_cpu,					\
		__u64: le64_to_cpu,					\
		struct btrfs_key: btrfs_key_to_cpu_v,			\
		struct btrfs_stripe: btrfs_stripe_to_cpu_v,		\
		struct btrfs_timespec: btrfs_timespec_to_cpu_v,		\
		struct btrfs_inode_item: btrfs_inode_item_to_cpu_v,	\
		struct btrfs_root_backup: btrfs_root_backup_to_cpu_v,	\
		struct btrfs_dev_item: btrfs_dev_item_to_cpu_v		\
		)((d->x));

# define DEFINE_CONV_ONE(t,n,m,...)			\
	static inline struct t * n(struct t * d) {	\
		CALL_MACRO_FOR_EACH(m, ##__VA_ARGS__)	\
		return d;				\
	}

/* Finally define the DEFINE_CONV macro */
# define DEFINE_CONV(n,...) \
	DEFINE_CONV_ONE(n,n##_to_disk,DEFINE_CONV_CPU_TO_LE,##__VA_ARGS__) \
	DEFINE_CONV_ONE(n,n##_to_cpu,DEFINE_CONV_LE_TO_CPU,##__VA_ARGS__)

# define DEFINE_CONV_ALT(n,a,...) \
	DEFINE_CONV_ONE(n,n##_to_disk_##a,DEFINE_CONV_CPU_TO_LE, \
		##__VA_ARGS__) \
	DEFINE_CONV_ONE(n,n##_to_cpu_##a,DEFINE_CONV_LE_TO_CPU,##__VA_ARGS__)

#endif /* !defined(__LITTLE_ENDIAN) */

DEFINE_CONV(btrfs_key, objectid, offset)
DEFINE_CONV(btrfs_dev_item, devid, total_bytes, bytes_used, io_align, io_width,
	    sector_size, type, generation, start_offset, dev_group)
DEFINE_CONV(btrfs_stripe, devid, offset)
DEFINE_CONV(btrfs_chunk, length, owner, stripe_len, type, io_align, io_width,
	    sector_size, num_stripes, sub_stripes)
DEFINE_CONV(btrfs_free_space_entry, offset, bytes)
DEFINE_CONV(btrfs_free_space_header, location, generation, num_entries,
	    num_bitmaps)
DEFINE_CONV(btrfs_extent_item, refs, generation, flags)
DEFINE_CONV(btrfs_tree_block_info, key)
DEFINE_CONV(btrfs_extent_data_ref, root, objectid, offset, count)
DEFINE_CONV(btrfs_shared_data_ref, count)
DEFINE_CONV(btrfs_extent_inline_ref, offset)
DEFINE_CONV(btrfs_dev_extent, chunk_tree, chunk_objectid, chunk_offset, length)
DEFINE_CONV(btrfs_inode_ref, index, name_len)
DEFINE_CONV(btrfs_inode_extref, parent_objectid, index, name_len)
DEFINE_CONV(btrfs_timespec, sec, nsec)
DEFINE_CONV(btrfs_inode_item, generation, transid, size, nbytes, block_group,
	    nlink, uid, gid, mode, rdev, flags, sequence, atime, ctime, mtime,
	    otime)
DEFINE_CONV(btrfs_dir_log_item, end)
DEFINE_CONV(btrfs_dir_item, location, transid, data_len, name_len)
DEFINE_CONV(btrfs_root_item, inode, generation, root_dirid, bytenr, byte_limit,
	    bytes_used, last_snapshot, flags, refs, drop_progress,
	    generation_v2, ctransid, otransid, stransid, rtransid, ctime,
	    otime, stime, rtime)
DEFINE_CONV(btrfs_root_ref, dirid, sequence, name_len)
DEFINE_CONV(btrfs_file_extent_item, generation, ram_bytes, other_encoding,
	    disk_bytenr, disk_num_bytes, offset, num_bytes)
DEFINE_CONV_ALT(btrfs_file_extent_item, inl, generation, ram_bytes,
		other_encoding)
DEFINE_CONV(btrfs_dev_replace_item, src_devid, cursor_left, cursor_right,
	    cont_reading_from_srcdev_mode, replace_state, time_started,
	    time_stopped, num_write_errors, num_uncorrectable_read_errors)
DEFINE_CONV(btrfs_block_group_item, used, chunk_objectid, flags)
DEFINE_CONV(btrfs_free_space_info, extent_count, flags)

DEFINE_CONV(btrfs_header, bytenr, flags, generation, owner, nritems)
DEFINE_CONV(btrfs_root_backup, tree_root, tree_root_gen, chunk_root,
	    chunk_root_gen, extent_root, extent_root_gen, fs_root, fs_root_gen,
	    dev_root, dev_root_gen, csum_root, csum_root_gen, total_bytes,
	    bytes_used, num_devices)
DEFINE_CONV(btrfs_super_block, bytenr, flags, magic, generation, root,
	    chunk_root, log_root, log_root_transid, total_bytes, bytes_used,
	    root_dir_objectid, num_devices, sectorsize, nodesize,
	    __unused_leafsize, stripesize, sys_chunk_array_size,
	    chunk_root_generation, compat_flags, compat_ro_flags,
	    incompat_flags, csum_type, dev_item, cache_generation,
	    uuid_tree_generation, super_roots[0], super_roots[1], 
	    super_roots[2], super_roots[3])
DEFINE_CONV(btrfs_item, key, offset, size)
DEFINE_CONV(btrfs_key_ptr, key, blockptr, generation)

#endif /* __BTRFS_CONV_FUNCS_H__ */
