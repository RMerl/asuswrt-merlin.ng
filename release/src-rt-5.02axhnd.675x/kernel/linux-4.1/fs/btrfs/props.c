/*
 * Copyright (C) 2014 Filipe David Borba Manana <fdmanana@gmail.com>
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

#include <linux/hashtable.h>
#include "props.h"
#include "btrfs_inode.h"
#include "hash.h"
#include "transaction.h"
#include "xattr.h"

#define BTRFS_PROP_HANDLERS_HT_BITS 8
static DEFINE_HASHTABLE(prop_handlers_ht, BTRFS_PROP_HANDLERS_HT_BITS);

struct prop_handler {
	struct hlist_node node;
	const char *xattr_name;
	int (*validate)(const char *value, size_t len);
	int (*apply)(struct inode *inode, const char *value, size_t len);
	const char *(*extract)(struct inode *inode);
	int inheritable;
};

static int prop_compression_validate(const char *value, size_t len);
static int prop_compression_apply(struct inode *inode,
				  const char *value,
				  size_t len);
static const char *prop_compression_extract(struct inode *inode);

static struct prop_handler prop_handlers[] = {
	{
		.xattr_name = XATTR_BTRFS_PREFIX "compression",
		.validate = prop_compression_validate,
		.apply = prop_compression_apply,
		.extract = prop_compression_extract,
		.inheritable = 1
	},
	{
		.xattr_name = NULL
	}
};

void __init btrfs_props_init(void)
{
	struct prop_handler *p;

	hash_init(prop_handlers_ht);

	for (p = &prop_handlers[0]; p->xattr_name; p++) {
		u64 h = btrfs_name_hash(p->xattr_name, strlen(p->xattr_name));

		hash_add(prop_handlers_ht, &p->node, h);
	}
}

static const struct hlist_head *find_prop_handlers_by_hash(const u64 hash)
{
	struct hlist_head *h;

	h = &prop_handlers_ht[hash_min(hash, BTRFS_PROP_HANDLERS_HT_BITS)];
	if (hlist_empty(h))
		return NULL;

	return h;
}

static const struct prop_handler *
find_prop_handler(const char *name,
		  const struct hlist_head *handlers)
{
	struct prop_handler *h;

	if (!handlers) {
		u64 hash = btrfs_name_hash(name, strlen(name));

		handlers = find_prop_handlers_by_hash(hash);
		if (!handlers)
			return NULL;
	}

	hlist_for_each_entry(h, handlers, node)
		if (!strcmp(h->xattr_name, name))
			return h;

	return NULL;
}

static int __btrfs_set_prop(struct btrfs_trans_handle *trans,
			    struct inode *inode,
			    const char *name,
			    const char *value,
			    size_t value_len,
			    int flags)
{
	const struct prop_handler *handler;
	int ret;

	if (strlen(name) <= XATTR_BTRFS_PREFIX_LEN)
		return -EINVAL;

	handler = find_prop_handler(name, NULL);
	if (!handler)
		return -EINVAL;

	if (value_len == 0) {
		ret = __btrfs_setxattr(trans, inode, handler->xattr_name,
				       NULL, 0, flags);
		if (ret)
			return ret;

		ret = handler->apply(inode, NULL, 0);
		ASSERT(ret == 0);

		return ret;
	}

	ret = handler->validate(value, value_len);
	if (ret)
		return ret;
	ret = __btrfs_setxattr(trans, inode, handler->xattr_name,
			       value, value_len, flags);
	if (ret)
		return ret;
	ret = handler->apply(inode, value, value_len);
	if (ret) {
		__btrfs_setxattr(trans, inode, handler->xattr_name,
				 NULL, 0, flags);
		return ret;
	}

	set_bit(BTRFS_INODE_HAS_PROPS, &BTRFS_I(inode)->runtime_flags);

	return 0;
}

int btrfs_set_prop(struct inode *inode,
		   const char *name,
		   const char *value,
		   size_t value_len,
		   int flags)
{
	return __btrfs_set_prop(NULL, inode, name, value, value_len, flags);
}

static int iterate_object_props(struct btrfs_root *root,
				struct btrfs_path *path,
				u64 objectid,
				void (*iterator)(void *,
						 const struct prop_handler *,
						 const char *,
						 size_t),
				void *ctx)
{
	int ret;
	char *name_buf = NULL;
	char *value_buf = NULL;
	int name_buf_len = 0;
	int value_buf_len = 0;

	while (1) {
		struct btrfs_key key;
		struct btrfs_dir_item *di;
		struct extent_buffer *leaf;
		u32 total_len, cur, this_len;
		int slot;
		const struct hlist_head *handlers;

		slot = path->slots[0];
		leaf = path->nodes[0];

		if (slot >= btrfs_header_nritems(leaf)) {
			ret = btrfs_next_leaf(root, path);
			if (ret < 0)
				goto out;
			else if (ret > 0)
				break;
			continue;
		}

		btrfs_item_key_to_cpu(leaf, &key, slot);
		if (key.objectid != objectid)
			break;
		if (key.type != BTRFS_XATTR_ITEM_KEY)
			break;

		handlers = find_prop_handlers_by_hash(key.offset);
		if (!handlers)
			goto next_slot;

		di = btrfs_item_ptr(leaf, slot, struct btrfs_dir_item);
		cur = 0;
		total_len = btrfs_item_size_nr(leaf, slot);

		while (cur < total_len) {
			u32 name_len = btrfs_dir_name_len(leaf, di);
			u32 data_len = btrfs_dir_data_len(leaf, di);
			unsigned long name_ptr, data_ptr;
			const struct prop_handler *handler;

			this_len = sizeof(*di) + name_len + data_len;
			name_ptr = (unsigned long)(di + 1);
			data_ptr = name_ptr + name_len;

			if (name_len <= XATTR_BTRFS_PREFIX_LEN ||
			    memcmp_extent_buffer(leaf, XATTR_BTRFS_PREFIX,
						 name_ptr,
						 XATTR_BTRFS_PREFIX_LEN))
				goto next_dir_item;

			if (name_len >= name_buf_len) {
				kfree(name_buf);
				name_buf_len = name_len + 1;
				name_buf = kmalloc(name_buf_len, GFP_NOFS);
				if (!name_buf) {
					ret = -ENOMEM;
					goto out;
				}
			}
			read_extent_buffer(leaf, name_buf, name_ptr, name_len);
			name_buf[name_len] = '\0';

			handler = find_prop_handler(name_buf, handlers);
			if (!handler)
				goto next_dir_item;

			if (data_len > value_buf_len) {
				kfree(value_buf);
				value_buf_len = data_len;
				value_buf = kmalloc(data_len, GFP_NOFS);
				if (!value_buf) {
					ret = -ENOMEM;
					goto out;
				}
			}
			read_extent_buffer(leaf, value_buf, data_ptr, data_len);

			iterator(ctx, handler, value_buf, data_len);
next_dir_item:
			cur += this_len;
			di = (struct btrfs_dir_item *)((char *) di + this_len);
		}

next_slot:
		path->slots[0]++;
	}

	ret = 0;
out:
	btrfs_release_path(path);
	kfree(name_buf);
	kfree(value_buf);

	return ret;
}

static void inode_prop_iterator(void *ctx,
				const struct prop_handler *handler,
				const char *value,
				size_t len)
{
	struct inode *inode = ctx;
	struct btrfs_root *root = BTRFS_I(inode)->root;
	int ret;

	ret = handler->apply(inode, value, len);
	if (unlikely(ret))
		btrfs_warn(root->fs_info,
			   "error applying prop %s to ino %llu (root %llu): %d",
			   handler->xattr_name, btrfs_ino(inode),
			   root->root_key.objectid, ret);
	else
		set_bit(BTRFS_INODE_HAS_PROPS, &BTRFS_I(inode)->runtime_flags);
}

int btrfs_load_inode_props(struct inode *inode, struct btrfs_path *path)
{
	struct btrfs_root *root = BTRFS_I(inode)->root;
	u64 ino = btrfs_ino(inode);
	int ret;

	ret = iterate_object_props(root, path, ino, inode_prop_iterator, inode);

	return ret;
}

static int inherit_props(struct btrfs_trans_handle *trans,
			 struct inode *inode,
			 struct inode *parent)
{
	const struct prop_handler *h;
	struct btrfs_root *root = BTRFS_I(inode)->root;
	int ret;

	if (!test_bit(BTRFS_INODE_HAS_PROPS,
		      &BTRFS_I(parent)->runtime_flags))
		return 0;

	for (h = &prop_handlers[0]; h->xattr_name; h++) {
		const char *value;
		u64 num_bytes;

		if (!h->inheritable)
			continue;

		value = h->extract(parent);
		if (!value)
			continue;

		num_bytes = btrfs_calc_trans_metadata_size(root, 1);
		ret = btrfs_block_rsv_add(root, trans->block_rsv,
					  num_bytes, BTRFS_RESERVE_NO_FLUSH);
		if (ret)
			goto out;
		ret = __btrfs_set_prop(trans, inode, h->xattr_name,
				       value, strlen(value), 0);
		btrfs_block_rsv_release(root, trans->block_rsv, num_bytes);
		if (ret)
			goto out;
	}
	ret = 0;
out:
	return ret;
}

int btrfs_inode_inherit_props(struct btrfs_trans_handle *trans,
			      struct inode *inode,
			      struct inode *dir)
{
	if (!dir)
		return 0;

	return inherit_props(trans, inode, dir);
}

int btrfs_subvol_inherit_props(struct btrfs_trans_handle *trans,
			       struct btrfs_root *root,
			       struct btrfs_root *parent_root)
{
	struct btrfs_key key;
	struct inode *parent_inode, *child_inode;
	int ret;

	key.objectid = BTRFS_FIRST_FREE_OBJECTID;
	key.type = BTRFS_INODE_ITEM_KEY;
	key.offset = 0;

	parent_inode = btrfs_iget(parent_root->fs_info->sb, &key,
				  parent_root, NULL);
	if (IS_ERR(parent_inode))
		return PTR_ERR(parent_inode);

	child_inode = btrfs_iget(root->fs_info->sb, &key, root, NULL);
	if (IS_ERR(child_inode)) {
		iput(parent_inode);
		return PTR_ERR(child_inode);
	}

	ret = inherit_props(trans, child_inode, parent_inode);
	iput(child_inode);
	iput(parent_inode);

	return ret;
}

static int prop_compression_validate(const char *value, size_t len)
{
	if (!strncmp("lzo", value, len))
		return 0;
	else if (!strncmp("zlib", value, len))
		return 0;

	return -EINVAL;
}

static int prop_compression_apply(struct inode *inode,
				  const char *value,
				  size_t len)
{
	int type;

	if (len == 0) {
		BTRFS_I(inode)->flags |= BTRFS_INODE_NOCOMPRESS;
		BTRFS_I(inode)->flags &= ~BTRFS_INODE_COMPRESS;
		BTRFS_I(inode)->force_compress = BTRFS_COMPRESS_NONE;

		return 0;
	}

	if (!strncmp("lzo", value, len))
		type = BTRFS_COMPRESS_LZO;
	else if (!strncmp("zlib", value, len))
		type = BTRFS_COMPRESS_ZLIB;
	else
		return -EINVAL;

	BTRFS_I(inode)->flags &= ~BTRFS_INODE_NOCOMPRESS;
	BTRFS_I(inode)->flags |= BTRFS_INODE_COMPRESS;
	BTRFS_I(inode)->force_compress = type;

	return 0;
}

static const char *prop_compression_extract(struct inode *inode)
{
	switch (BTRFS_I(inode)->force_compress) {
	case BTRFS_COMPRESS_ZLIB:
		return "zlib";
	case BTRFS_COMPRESS_LZO:
		return "lzo";
	}

	return NULL;
}


