/*
 * Copyright (C) 2007 Oracle.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#include <linux/highmem.h>
#include <asm/unaligned.h>

#include "ctree.h"

static inline u8 get_unaligned_le8(const void *p)
{
       return *(u8 *)p;
}

static inline void put_unaligned_le8(u8 val, void *p)
{
       *(u8 *)p = val;
}

/*
 * this is some deeply nasty code.
 *
 * The end result is that anyone who #includes ctree.h gets a
 * declaration for the btrfs_set_foo functions and btrfs_foo functions,
 * which are wappers of btrfs_set_token_#bits functions and
 * btrfs_get_token_#bits functions, which are defined in this file.
 *
 * These setget functions do all the extent_buffer related mapping
 * required to efficiently read and write specific fields in the extent
 * buffers.  Every pointer to metadata items in btrfs is really just
 * an unsigned long offset into the extent buffer which has been
 * cast to a specific type.  This gives us all the gcc type checking.
 *
 * The extent buffer api is used to do the page spanning work required to
 * have a metadata blocksize different from the page size.
 */

#define DEFINE_BTRFS_SETGET_BITS(bits)					\
u##bits btrfs_get_token_##bits(struct extent_buffer *eb, void *ptr,	\
			       unsigned long off,			\
			       struct btrfs_map_token *token)		\
{									\
	unsigned long part_offset = (unsigned long)ptr;			\
	unsigned long offset = part_offset + off;			\
	void *p;							\
	int err;							\
	char *kaddr;							\
	unsigned long map_start;					\
	unsigned long map_len;						\
	int size = sizeof(u##bits);					\
	u##bits res;							\
									\
	if (token && token->kaddr && token->offset <= offset &&		\
	    token->eb == eb &&						\
	   (token->offset + PAGE_CACHE_SIZE >= offset + size)) {	\
		kaddr = token->kaddr;					\
		p = kaddr + part_offset - token->offset;		\
		res = get_unaligned_le##bits(p + off);			\
		return res;						\
	}								\
	err = map_private_extent_buffer(eb, offset, size,		\
					&kaddr, &map_start, &map_len);	\
	if (err) {							\
		__le##bits leres;					\
									\
		read_extent_buffer(eb, &leres, offset, size);		\
		return le##bits##_to_cpu(leres);			\
	}								\
	p = kaddr + part_offset - map_start;				\
	res = get_unaligned_le##bits(p + off);				\
	if (token) {							\
		token->kaddr = kaddr;					\
		token->offset = map_start;				\
		token->eb = eb;						\
	}								\
	return res;							\
}									\
void btrfs_set_token_##bits(struct extent_buffer *eb,			\
			    void *ptr, unsigned long off, u##bits val,	\
			    struct btrfs_map_token *token)		\
{									\
	unsigned long part_offset = (unsigned long)ptr;			\
	unsigned long offset = part_offset + off;			\
	void *p;							\
	int err;							\
	char *kaddr;							\
	unsigned long map_start;					\
	unsigned long map_len;						\
	int size = sizeof(u##bits);					\
									\
	if (token && token->kaddr && token->offset <= offset &&		\
	    token->eb == eb &&						\
	   (token->offset + PAGE_CACHE_SIZE >= offset + size)) {	\
		kaddr = token->kaddr;					\
		p = kaddr + part_offset - token->offset;		\
		put_unaligned_le##bits(val, p + off);			\
		return;							\
	}								\
	err = map_private_extent_buffer(eb, offset, size,		\
			&kaddr, &map_start, &map_len);			\
	if (err) {							\
		__le##bits val2;					\
									\
		val2 = cpu_to_le##bits(val);				\
		write_extent_buffer(eb, &val2, offset, size);		\
		return;							\
	}								\
	p = kaddr + part_offset - map_start;				\
	put_unaligned_le##bits(val, p + off);				\
	if (token) {							\
		token->kaddr = kaddr;					\
		token->offset = map_start;				\
		token->eb = eb;						\
	}								\
}

DEFINE_BTRFS_SETGET_BITS(8)
DEFINE_BTRFS_SETGET_BITS(16)
DEFINE_BTRFS_SETGET_BITS(32)
DEFINE_BTRFS_SETGET_BITS(64)

void btrfs_node_key(struct extent_buffer *eb,
		    struct btrfs_disk_key *disk_key, int nr)
{
	unsigned long ptr = btrfs_node_key_ptr_offset(nr);
	read_eb_member(eb, (struct btrfs_key_ptr *)ptr,
		       struct btrfs_key_ptr, key, disk_key);
}
