/*
 * Copyright (C) 2011 Fujitsu.  All rights reserved.
 * Written by Miao Xie <miaox@cn.fujitsu.com>
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

#include <linux/slab.h>
#include "delayed-inode.h"
#include "disk-io.h"
#include "transaction.h"
#include "ctree.h"

#define BTRFS_DELAYED_WRITEBACK		512
#define BTRFS_DELAYED_BACKGROUND	128
#define BTRFS_DELAYED_BATCH		16

static struct kmem_cache *delayed_node_cache;

int __init btrfs_delayed_inode_init(void)
{
	delayed_node_cache = kmem_cache_create("btrfs_delayed_node",
					sizeof(struct btrfs_delayed_node),
					0,
					SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD,
					NULL);
	if (!delayed_node_cache)
		return -ENOMEM;
	return 0;
}

void btrfs_delayed_inode_exit(void)
{
	if (delayed_node_cache)
		kmem_cache_destroy(delayed_node_cache);
}

static inline void btrfs_init_delayed_node(
				struct btrfs_delayed_node *delayed_node,
				struct btrfs_root *root, u64 inode_id)
{
	delayed_node->root = root;
	delayed_node->inode_id = inode_id;
	atomic_set(&delayed_node->refs, 0);
	delayed_node->count = 0;
	delayed_node->flags = 0;
	delayed_node->ins_root = RB_ROOT;
	delayed_node->del_root = RB_ROOT;
	mutex_init(&delayed_node->mutex);
	delayed_node->index_cnt = 0;
	INIT_LIST_HEAD(&delayed_node->n_list);
	INIT_LIST_HEAD(&delayed_node->p_list);
	delayed_node->bytes_reserved = 0;
	memset(&delayed_node->inode_item, 0, sizeof(delayed_node->inode_item));
}

static inline int btrfs_is_continuous_delayed_item(
					struct btrfs_delayed_item *item1,
					struct btrfs_delayed_item *item2)
{
	if (item1->key.type == BTRFS_DIR_INDEX_KEY &&
	    item1->key.objectid == item2->key.objectid &&
	    item1->key.type == item2->key.type &&
	    item1->key.offset + 1 == item2->key.offset)
		return 1;
	return 0;
}

static inline struct btrfs_delayed_root *btrfs_get_delayed_root(
							struct btrfs_root *root)
{
	return root->fs_info->delayed_root;
}

static struct btrfs_delayed_node *btrfs_get_delayed_node(struct inode *inode)
{
	struct btrfs_inode *btrfs_inode = BTRFS_I(inode);
	struct btrfs_root *root = btrfs_inode->root;
	u64 ino = btrfs_ino(inode);
	struct btrfs_delayed_node *node;

	node = ACCESS_ONCE(btrfs_inode->delayed_node);
	if (node) {
		atomic_inc(&node->refs);
		return node;
	}

	spin_lock(&root->inode_lock);
	node = radix_tree_lookup(&root->delayed_nodes_tree, ino);
	if (node) {
		if (btrfs_inode->delayed_node) {
			atomic_inc(&node->refs);	/* can be accessed */
			BUG_ON(btrfs_inode->delayed_node != node);
			spin_unlock(&root->inode_lock);
			return node;
		}
		btrfs_inode->delayed_node = node;
		/* can be accessed and cached in the inode */
		atomic_add(2, &node->refs);
		spin_unlock(&root->inode_lock);
		return node;
	}
	spin_unlock(&root->inode_lock);

	return NULL;
}

/* Will return either the node or PTR_ERR(-ENOMEM) */
static struct btrfs_delayed_node *btrfs_get_or_create_delayed_node(
							struct inode *inode)
{
	struct btrfs_delayed_node *node;
	struct btrfs_inode *btrfs_inode = BTRFS_I(inode);
	struct btrfs_root *root = btrfs_inode->root;
	u64 ino = btrfs_ino(inode);
	int ret;

again:
	node = btrfs_get_delayed_node(inode);
	if (node)
		return node;

	node = kmem_cache_alloc(delayed_node_cache, GFP_NOFS);
	if (!node)
		return ERR_PTR(-ENOMEM);
	btrfs_init_delayed_node(node, root, ino);

	/* cached in the btrfs inode and can be accessed */
	atomic_add(2, &node->refs);

	ret = radix_tree_preload(GFP_NOFS & ~__GFP_HIGHMEM);
	if (ret) {
		kmem_cache_free(delayed_node_cache, node);
		return ERR_PTR(ret);
	}

	spin_lock(&root->inode_lock);
	ret = radix_tree_insert(&root->delayed_nodes_tree, ino, node);
	if (ret == -EEXIST) {
		spin_unlock(&root->inode_lock);
		kmem_cache_free(delayed_node_cache, node);
		radix_tree_preload_end();
		goto again;
	}
	btrfs_inode->delayed_node = node;
	spin_unlock(&root->inode_lock);
	radix_tree_preload_end();

	return node;
}

/*
 * Call it when holding delayed_node->mutex
 *
 * If mod = 1, add this node into the prepared list.
 */
static void btrfs_queue_delayed_node(struct btrfs_delayed_root *root,
				     struct btrfs_delayed_node *node,
				     int mod)
{
	spin_lock(&root->lock);
	if (test_bit(BTRFS_DELAYED_NODE_IN_LIST, &node->flags)) {
		if (!list_empty(&node->p_list))
			list_move_tail(&node->p_list, &root->prepare_list);
		else if (mod)
			list_add_tail(&node->p_list, &root->prepare_list);
	} else {
		list_add_tail(&node->n_list, &root->node_list);
		list_add_tail(&node->p_list, &root->prepare_list);
		atomic_inc(&node->refs);	/* inserted into list */
		root->nodes++;
		set_bit(BTRFS_DELAYED_NODE_IN_LIST, &node->flags);
	}
	spin_unlock(&root->lock);
}

/* Call it when holding delayed_node->mutex */
static void btrfs_dequeue_delayed_node(struct btrfs_delayed_root *root,
				       struct btrfs_delayed_node *node)
{
	spin_lock(&root->lock);
	if (test_bit(BTRFS_DELAYED_NODE_IN_LIST, &node->flags)) {
		root->nodes--;
		atomic_dec(&node->refs);	/* not in the list */
		list_del_init(&node->n_list);
		if (!list_empty(&node->p_list))
			list_del_init(&node->p_list);
		clear_bit(BTRFS_DELAYED_NODE_IN_LIST, &node->flags);
	}
	spin_unlock(&root->lock);
}

static struct btrfs_delayed_node *btrfs_first_delayed_node(
			struct btrfs_delayed_root *delayed_root)
{
	struct list_head *p;
	struct btrfs_delayed_node *node = NULL;

	spin_lock(&delayed_root->lock);
	if (list_empty(&delayed_root->node_list))
		goto out;

	p = delayed_root->node_list.next;
	node = list_entry(p, struct btrfs_delayed_node, n_list);
	atomic_inc(&node->refs);
out:
	spin_unlock(&delayed_root->lock);

	return node;
}

static struct btrfs_delayed_node *btrfs_next_delayed_node(
						struct btrfs_delayed_node *node)
{
	struct btrfs_delayed_root *delayed_root;
	struct list_head *p;
	struct btrfs_delayed_node *next = NULL;

	delayed_root = node->root->fs_info->delayed_root;
	spin_lock(&delayed_root->lock);
	if (!test_bit(BTRFS_DELAYED_NODE_IN_LIST, &node->flags)) {
		/* not in the list */
		if (list_empty(&delayed_root->node_list))
			goto out;
		p = delayed_root->node_list.next;
	} else if (list_is_last(&node->n_list, &delayed_root->node_list))
		goto out;
	else
		p = node->n_list.next;

	next = list_entry(p, struct btrfs_delayed_node, n_list);
	atomic_inc(&next->refs);
out:
	spin_unlock(&delayed_root->lock);

	return next;
}

static void __btrfs_release_delayed_node(
				struct btrfs_delayed_node *delayed_node,
				int mod)
{
	struct btrfs_delayed_root *delayed_root;

	if (!delayed_node)
		return;

	delayed_root = delayed_node->root->fs_info->delayed_root;

	mutex_lock(&delayed_node->mutex);
	if (delayed_node->count)
		btrfs_queue_delayed_node(delayed_root, delayed_node, mod);
	else
		btrfs_dequeue_delayed_node(delayed_root, delayed_node);
	mutex_unlock(&delayed_node->mutex);

	if (atomic_dec_and_test(&delayed_node->refs)) {
		bool free = false;
		struct btrfs_root *root = delayed_node->root;
		spin_lock(&root->inode_lock);
		if (atomic_read(&delayed_node->refs) == 0) {
			radix_tree_delete(&root->delayed_nodes_tree,
					  delayed_node->inode_id);
			free = true;
		}
		spin_unlock(&root->inode_lock);
		if (free)
			kmem_cache_free(delayed_node_cache, delayed_node);
	}
}

static inline void btrfs_release_delayed_node(struct btrfs_delayed_node *node)
{
	__btrfs_release_delayed_node(node, 0);
}

static struct btrfs_delayed_node *btrfs_first_prepared_delayed_node(
					struct btrfs_delayed_root *delayed_root)
{
	struct list_head *p;
	struct btrfs_delayed_node *node = NULL;

	spin_lock(&delayed_root->lock);
	if (list_empty(&delayed_root->prepare_list))
		goto out;

	p = delayed_root->prepare_list.next;
	list_del_init(p);
	node = list_entry(p, struct btrfs_delayed_node, p_list);
	atomic_inc(&node->refs);
out:
	spin_unlock(&delayed_root->lock);

	return node;
}

static inline void btrfs_release_prepared_delayed_node(
					struct btrfs_delayed_node *node)
{
	__btrfs_release_delayed_node(node, 1);
}

static struct btrfs_delayed_item *btrfs_alloc_delayed_item(u32 data_len)
{
	struct btrfs_delayed_item *item;
	item = kmalloc(sizeof(*item) + data_len, GFP_NOFS);
	if (item) {
		item->data_len = data_len;
		item->ins_or_del = 0;
		item->bytes_reserved = 0;
		item->delayed_node = NULL;
		atomic_set(&item->refs, 1);
	}
	return item;
}

/*
 * __btrfs_lookup_delayed_item - look up the delayed item by key
 * @delayed_node: pointer to the delayed node
 * @key:	  the key to look up
 * @prev:	  used to store the prev item if the right item isn't found
 * @next:	  used to store the next item if the right item isn't found
 *
 * Note: if we don't find the right item, we will return the prev item and
 * the next item.
 */
static struct btrfs_delayed_item *__btrfs_lookup_delayed_item(
				struct rb_root *root,
				struct btrfs_key *key,
				struct btrfs_delayed_item **prev,
				struct btrfs_delayed_item **next)
{
	struct rb_node *node, *prev_node = NULL;
	struct btrfs_delayed_item *delayed_item = NULL;
	int ret = 0;

	node = root->rb_node;

	while (node) {
		delayed_item = rb_entry(node, struct btrfs_delayed_item,
					rb_node);
		prev_node = node;
		ret = btrfs_comp_cpu_keys(&delayed_item->key, key);
		if (ret < 0)
			node = node->rb_right;
		else if (ret > 0)
			node = node->rb_left;
		else
			return delayed_item;
	}

	if (prev) {
		if (!prev_node)
			*prev = NULL;
		else if (ret < 0)
			*prev = delayed_item;
		else if ((node = rb_prev(prev_node)) != NULL) {
			*prev = rb_entry(node, struct btrfs_delayed_item,
					 rb_node);
		} else
			*prev = NULL;
	}

	if (next) {
		if (!prev_node)
			*next = NULL;
		else if (ret > 0)
			*next = delayed_item;
		else if ((node = rb_next(prev_node)) != NULL) {
			*next = rb_entry(node, struct btrfs_delayed_item,
					 rb_node);
		} else
			*next = NULL;
	}
	return NULL;
}

static struct btrfs_delayed_item *__btrfs_lookup_delayed_insertion_item(
					struct btrfs_delayed_node *delayed_node,
					struct btrfs_key *key)
{
	struct btrfs_delayed_item *item;

	item = __btrfs_lookup_delayed_item(&delayed_node->ins_root, key,
					   NULL, NULL);
	return item;
}

static int __btrfs_add_delayed_item(struct btrfs_delayed_node *delayed_node,
				    struct btrfs_delayed_item *ins,
				    int action)
{
	struct rb_node **p, *node;
	struct rb_node *parent_node = NULL;
	struct rb_root *root;
	struct btrfs_delayed_item *item;
	int cmp;

	if (action == BTRFS_DELAYED_INSERTION_ITEM)
		root = &delayed_node->ins_root;
	else if (action == BTRFS_DELAYED_DELETION_ITEM)
		root = &delayed_node->del_root;
	else
		BUG();
	p = &root->rb_node;
	node = &ins->rb_node;

	while (*p) {
		parent_node = *p;
		item = rb_entry(parent_node, struct btrfs_delayed_item,
				 rb_node);

		cmp = btrfs_comp_cpu_keys(&item->key, &ins->key);
		if (cmp < 0)
			p = &(*p)->rb_right;
		else if (cmp > 0)
			p = &(*p)->rb_left;
		else
			return -EEXIST;
	}

	rb_link_node(node, parent_node, p);
	rb_insert_color(node, root);
	ins->delayed_node = delayed_node;
	ins->ins_or_del = action;

	if (ins->key.type == BTRFS_DIR_INDEX_KEY &&
	    action == BTRFS_DELAYED_INSERTION_ITEM &&
	    ins->key.offset >= delayed_node->index_cnt)
			delayed_node->index_cnt = ins->key.offset + 1;

	delayed_node->count++;
	atomic_inc(&delayed_node->root->fs_info->delayed_root->items);
	return 0;
}

static int __btrfs_add_delayed_insertion_item(struct btrfs_delayed_node *node,
					      struct btrfs_delayed_item *item)
{
	return __btrfs_add_delayed_item(node, item,
					BTRFS_DELAYED_INSERTION_ITEM);
}

static int __btrfs_add_delayed_deletion_item(struct btrfs_delayed_node *node,
					     struct btrfs_delayed_item *item)
{
	return __btrfs_add_delayed_item(node, item,
					BTRFS_DELAYED_DELETION_ITEM);
}

static void finish_one_item(struct btrfs_delayed_root *delayed_root)
{
	int seq = atomic_inc_return(&delayed_root->items_seq);
	if ((atomic_dec_return(&delayed_root->items) <
	    BTRFS_DELAYED_BACKGROUND || seq % BTRFS_DELAYED_BATCH == 0) &&
	    waitqueue_active(&delayed_root->wait))
		wake_up(&delayed_root->wait);
}

static void __btrfs_remove_delayed_item(struct btrfs_delayed_item *delayed_item)
{
	struct rb_root *root;
	struct btrfs_delayed_root *delayed_root;

	delayed_root = delayed_item->delayed_node->root->fs_info->delayed_root;

	BUG_ON(!delayed_root);
	BUG_ON(delayed_item->ins_or_del != BTRFS_DELAYED_DELETION_ITEM &&
	       delayed_item->ins_or_del != BTRFS_DELAYED_INSERTION_ITEM);

	if (delayed_item->ins_or_del == BTRFS_DELAYED_INSERTION_ITEM)
		root = &delayed_item->delayed_node->ins_root;
	else
		root = &delayed_item->delayed_node->del_root;

	rb_erase(&delayed_item->rb_node, root);
	delayed_item->delayed_node->count--;

	finish_one_item(delayed_root);
}

static void btrfs_release_delayed_item(struct btrfs_delayed_item *item)
{
	if (item) {
		__btrfs_remove_delayed_item(item);
		if (atomic_dec_and_test(&item->refs))
			kfree(item);
	}
}

static struct btrfs_delayed_item *__btrfs_first_delayed_insertion_item(
					struct btrfs_delayed_node *delayed_node)
{
	struct rb_node *p;
	struct btrfs_delayed_item *item = NULL;

	p = rb_first(&delayed_node->ins_root);
	if (p)
		item = rb_entry(p, struct btrfs_delayed_item, rb_node);

	return item;
}

static struct btrfs_delayed_item *__btrfs_first_delayed_deletion_item(
					struct btrfs_delayed_node *delayed_node)
{
	struct rb_node *p;
	struct btrfs_delayed_item *item = NULL;

	p = rb_first(&delayed_node->del_root);
	if (p)
		item = rb_entry(p, struct btrfs_delayed_item, rb_node);

	return item;
}

static struct btrfs_delayed_item *__btrfs_next_delayed_item(
						struct btrfs_delayed_item *item)
{
	struct rb_node *p;
	struct btrfs_delayed_item *next = NULL;

	p = rb_next(&item->rb_node);
	if (p)
		next = rb_entry(p, struct btrfs_delayed_item, rb_node);

	return next;
}

static int btrfs_delayed_item_reserve_metadata(struct btrfs_trans_handle *trans,
					       struct btrfs_root *root,
					       struct btrfs_delayed_item *item)
{
	struct btrfs_block_rsv *src_rsv;
	struct btrfs_block_rsv *dst_rsv;
	u64 num_bytes;
	int ret;

	if (!trans->bytes_reserved)
		return 0;

	src_rsv = trans->block_rsv;
	dst_rsv = &root->fs_info->delayed_block_rsv;

	num_bytes = btrfs_calc_trans_metadata_size(root, 1);
	ret = btrfs_block_rsv_migrate(src_rsv, dst_rsv, num_bytes);
	if (!ret) {
		trace_btrfs_space_reservation(root->fs_info, "delayed_item",
					      item->key.objectid,
					      num_bytes, 1);
		item->bytes_reserved = num_bytes;
	}

	return ret;
}

static void btrfs_delayed_item_release_metadata(struct btrfs_root *root,
						struct btrfs_delayed_item *item)
{
	struct btrfs_block_rsv *rsv;

	if (!item->bytes_reserved)
		return;

	rsv = &root->fs_info->delayed_block_rsv;
	trace_btrfs_space_reservation(root->fs_info, "delayed_item",
				      item->key.objectid, item->bytes_reserved,
				      0);
	btrfs_block_rsv_release(root, rsv,
				item->bytes_reserved);
}

static int btrfs_delayed_inode_reserve_metadata(
					struct btrfs_trans_handle *trans,
					struct btrfs_root *root,
					struct inode *inode,
					struct btrfs_delayed_node *node)
{
	struct btrfs_block_rsv *src_rsv;
	struct btrfs_block_rsv *dst_rsv;
	u64 num_bytes;
	int ret;
	bool release = false;

	src_rsv = trans->block_rsv;
	dst_rsv = &root->fs_info->delayed_block_rsv;

	num_bytes = btrfs_calc_trans_metadata_size(root, 1);

	/*
	 * btrfs_dirty_inode will update the inode under btrfs_join_transaction
	 * which doesn't reserve space for speed.  This is a problem since we
	 * still need to reserve space for this update, so try to reserve the
	 * space.
	 *
	 * Now if src_rsv == delalloc_block_rsv we'll let it just steal since
	 * we're accounted for.
	 */
	if (!src_rsv || (!trans->bytes_reserved &&
			 src_rsv->type != BTRFS_BLOCK_RSV_DELALLOC)) {
		ret = btrfs_block_rsv_add(root, dst_rsv, num_bytes,
					  BTRFS_RESERVE_NO_FLUSH);
		/*
		 * Since we're under a transaction reserve_metadata_bytes could
		 * try to commit the transaction which will make it return
		 * EAGAIN to make us stop the transaction we have, so return
		 * ENOSPC instead so that btrfs_dirty_inode knows what to do.
		 */
		if (ret == -EAGAIN)
			ret = -ENOSPC;
		if (!ret) {
			node->bytes_reserved = num_bytes;
			trace_btrfs_space_reservation(root->fs_info,
						      "delayed_inode",
						      btrfs_ino(inode),
						      num_bytes, 1);
		}
		return ret;
	} else if (src_rsv->type == BTRFS_BLOCK_RSV_DELALLOC) {
		spin_lock(&BTRFS_I(inode)->lock);
		if (test_and_clear_bit(BTRFS_INODE_DELALLOC_META_RESERVED,
				       &BTRFS_I(inode)->runtime_flags)) {
			spin_unlock(&BTRFS_I(inode)->lock);
			release = true;
			goto migrate;
		}
		spin_unlock(&BTRFS_I(inode)->lock);

		/* Ok we didn't have space pre-reserved.  This shouldn't happen
		 * too often but it can happen if we do delalloc to an existing
		 * inode which gets dirtied because of the time update, and then
		 * isn't touched again until after the transaction commits and
		 * then we try to write out the data.  First try to be nice and
		 * reserve something strictly for us.  If not be a pain and try
		 * to steal from the delalloc block rsv.
		 */
		ret = btrfs_block_rsv_add(root, dst_rsv, num_bytes,
					  BTRFS_RESERVE_NO_FLUSH);
		if (!ret)
			goto out;

		ret = btrfs_block_rsv_migrate(src_rsv, dst_rsv, num_bytes);
		if (!WARN_ON(ret))
			goto out;

		/*
		 * Ok this is a problem, let's just steal from the global rsv
		 * since this really shouldn't happen that often.
		 */
		ret = btrfs_block_rsv_migrate(&root->fs_info->global_block_rsv,
					      dst_rsv, num_bytes);
		goto out;
	}

migrate:
	ret = btrfs_block_rsv_migrate(src_rsv, dst_rsv, num_bytes);

out:
	/*
	 * Migrate only takes a reservation, it doesn't touch the size of the
	 * block_rsv.  This is to simplify people who don't normally have things
	 * migrated from their block rsv.  If they go to release their
	 * reservation, that will decrease the size as well, so if migrate
	 * reduced size we'd end up with a negative size.  But for the
	 * delalloc_meta_reserved stuff we will only know to drop 1 reservation,
	 * but we could in fact do this reserve/migrate dance several times
	 * between the time we did the original reservation and we'd clean it
	 * up.  So to take care of this, release the space for the meta
	 * reservation here.  I think it may be time for a documentation page on
	 * how block rsvs. work.
	 */
	if (!ret) {
		trace_btrfs_space_reservation(root->fs_info, "delayed_inode",
					      btrfs_ino(inode), num_bytes, 1);
		node->bytes_reserved = num_bytes;
	}

	if (release) {
		trace_btrfs_space_reservation(root->fs_info, "delalloc",
					      btrfs_ino(inode), num_bytes, 0);
		btrfs_block_rsv_release(root, src_rsv, num_bytes);
	}

	return ret;
}

static void btrfs_delayed_inode_release_metadata(struct btrfs_root *root,
						struct btrfs_delayed_node *node)
{
	struct btrfs_block_rsv *rsv;

	if (!node->bytes_reserved)
		return;

	rsv = &root->fs_info->delayed_block_rsv;
	trace_btrfs_space_reservation(root->fs_info, "delayed_inode",
				      node->inode_id, node->bytes_reserved, 0);
	btrfs_block_rsv_release(root, rsv,
				node->bytes_reserved);
	node->bytes_reserved = 0;
}

/*
 * This helper will insert some continuous items into the same leaf according
 * to the free space of the leaf.
 */
static int btrfs_batch_insert_items(struct btrfs_root *root,
				    struct btrfs_path *path,
				    struct btrfs_delayed_item *item)
{
	struct btrfs_delayed_item *curr, *next;
	int free_space;
	int total_data_size = 0, total_size = 0;
	struct extent_buffer *leaf;
	char *data_ptr;
	struct btrfs_key *keys;
	u32 *data_size;
	struct list_head head;
	int slot;
	int nitems;
	int i;
	int ret = 0;

	BUG_ON(!path->nodes[0]);

	leaf = path->nodes[0];
	free_space = btrfs_leaf_free_space(root, leaf);
	INIT_LIST_HEAD(&head);

	next = item;
	nitems = 0;

	/*
	 * count the number of the continuous items that we can insert in batch
	 */
	while (total_size + next->data_len + sizeof(struct btrfs_item) <=
	       free_space) {
		total_data_size += next->data_len;
		total_size += next->data_len + sizeof(struct btrfs_item);
		list_add_tail(&next->tree_list, &head);
		nitems++;

		curr = next;
		next = __btrfs_next_delayed_item(curr);
		if (!next)
			break;

		if (!btrfs_is_continuous_delayed_item(curr, next))
			break;
	}

	if (!nitems) {
		ret = 0;
		goto out;
	}

	/*
	 * we need allocate some memory space, but it might cause the task
	 * to sleep, so we set all locked nodes in the path to blocking locks
	 * first.
	 */
	btrfs_set_path_blocking(path);

	keys = kmalloc_array(nitems, sizeof(struct btrfs_key), GFP_NOFS);
	if (!keys) {
		ret = -ENOMEM;
		goto out;
	}

	data_size = kmalloc_array(nitems, sizeof(u32), GFP_NOFS);
	if (!data_size) {
		ret = -ENOMEM;
		goto error;
	}

	/* get keys of all the delayed items */
	i = 0;
	list_for_each_entry(next, &head, tree_list) {
		keys[i] = next->key;
		data_size[i] = next->data_len;
		i++;
	}

	/* reset all the locked nodes in the patch to spinning locks. */
	btrfs_clear_path_blocking(path, NULL, 0);

	/* insert the keys of the items */
	setup_items_for_insert(root, path, keys, data_size,
			       total_data_size, total_size, nitems);

	/* insert the dir index items */
	slot = path->slots[0];
	list_for_each_entry_safe(curr, next, &head, tree_list) {
		data_ptr = btrfs_item_ptr(leaf, slot, char);
		write_extent_buffer(leaf, &curr->data,
				    (unsigned long)data_ptr,
				    curr->data_len);
		slot++;

		btrfs_delayed_item_release_metadata(root, curr);

		list_del(&curr->tree_list);
		btrfs_release_delayed_item(curr);
	}

error:
	kfree(data_size);
	kfree(keys);
out:
	return ret;
}

/*
 * This helper can just do simple insertion that needn't extend item for new
 * data, such as directory name index insertion, inode insertion.
 */
static int btrfs_insert_delayed_item(struct btrfs_trans_handle *trans,
				     struct btrfs_root *root,
				     struct btrfs_path *path,
				     struct btrfs_delayed_item *delayed_item)
{
	struct extent_buffer *leaf;
	char *ptr;
	int ret;

	ret = btrfs_insert_empty_item(trans, root, path, &delayed_item->key,
				      delayed_item->data_len);
	if (ret < 0 && ret != -EEXIST)
		return ret;

	leaf = path->nodes[0];

	ptr = btrfs_item_ptr(leaf, path->slots[0], char);

	write_extent_buffer(leaf, delayed_item->data, (unsigned long)ptr,
			    delayed_item->data_len);
	btrfs_mark_buffer_dirty(leaf);

	btrfs_delayed_item_release_metadata(root, delayed_item);
	return 0;
}

/*
 * we insert an item first, then if there are some continuous items, we try
 * to insert those items into the same leaf.
 */
static int btrfs_insert_delayed_items(struct btrfs_trans_handle *trans,
				      struct btrfs_path *path,
				      struct btrfs_root *root,
				      struct btrfs_delayed_node *node)
{
	struct btrfs_delayed_item *curr, *prev;
	int ret = 0;

do_again:
	mutex_lock(&node->mutex);
	curr = __btrfs_first_delayed_insertion_item(node);
	if (!curr)
		goto insert_end;

	ret = btrfs_insert_delayed_item(trans, root, path, curr);
	if (ret < 0) {
		btrfs_release_path(path);
		goto insert_end;
	}

	prev = curr;
	curr = __btrfs_next_delayed_item(prev);
	if (curr && btrfs_is_continuous_delayed_item(prev, curr)) {
		/* insert the continuous items into the same leaf */
		path->slots[0]++;
		btrfs_batch_insert_items(root, path, curr);
	}
	btrfs_release_delayed_item(prev);
	btrfs_mark_buffer_dirty(path->nodes[0]);

	btrfs_release_path(path);
	mutex_unlock(&node->mutex);
	goto do_again;

insert_end:
	mutex_unlock(&node->mutex);
	return ret;
}

static int btrfs_batch_delete_items(struct btrfs_trans_handle *trans,
				    struct btrfs_root *root,
				    struct btrfs_path *path,
				    struct btrfs_delayed_item *item)
{
	struct btrfs_delayed_item *curr, *next;
	struct extent_buffer *leaf;
	struct btrfs_key key;
	struct list_head head;
	int nitems, i, last_item;
	int ret = 0;

	BUG_ON(!path->nodes[0]);

	leaf = path->nodes[0];

	i = path->slots[0];
	last_item = btrfs_header_nritems(leaf) - 1;
	if (i > last_item)
		return -ENOENT;	/* FIXME: Is errno suitable? */

	next = item;
	INIT_LIST_HEAD(&head);
	btrfs_item_key_to_cpu(leaf, &key, i);
	nitems = 0;
	/*
	 * count the number of the dir index items that we can delete in batch
	 */
	while (btrfs_comp_cpu_keys(&next->key, &key) == 0) {
		list_add_tail(&next->tree_list, &head);
		nitems++;

		curr = next;
		next = __btrfs_next_delayed_item(curr);
		if (!next)
			break;

		if (!btrfs_is_continuous_delayed_item(curr, next))
			break;

		i++;
		if (i > last_item)
			break;
		btrfs_item_key_to_cpu(leaf, &key, i);
	}

	if (!nitems)
		return 0;

	ret = btrfs_del_items(trans, root, path, path->slots[0], nitems);
	if (ret)
		goto out;

	list_for_each_entry_safe(curr, next, &head, tree_list) {
		btrfs_delayed_item_release_metadata(root, curr);
		list_del(&curr->tree_list);
		btrfs_release_delayed_item(curr);
	}

out:
	return ret;
}

static int btrfs_delete_delayed_items(struct btrfs_trans_handle *trans,
				      struct btrfs_path *path,
				      struct btrfs_root *root,
				      struct btrfs_delayed_node *node)
{
	struct btrfs_delayed_item *curr, *prev;
	int ret = 0;

do_again:
	mutex_lock(&node->mutex);
	curr = __btrfs_first_delayed_deletion_item(node);
	if (!curr)
		goto delete_fail;

	ret = btrfs_search_slot(trans, root, &curr->key, path, -1, 1);
	if (ret < 0)
		goto delete_fail;
	else if (ret > 0) {
		/*
		 * can't find the item which the node points to, so this node
		 * is invalid, just drop it.
		 */
		prev = curr;
		curr = __btrfs_next_delayed_item(prev);
		btrfs_release_delayed_item(prev);
		ret = 0;
		btrfs_release_path(path);
		if (curr) {
			mutex_unlock(&node->mutex);
			goto do_again;
		} else
			goto delete_fail;
	}

	btrfs_batch_delete_items(trans, root, path, curr);
	btrfs_release_path(path);
	mutex_unlock(&node->mutex);
	goto do_again;

delete_fail:
	btrfs_release_path(path);
	mutex_unlock(&node->mutex);
	return ret;
}

static void btrfs_release_delayed_inode(struct btrfs_delayed_node *delayed_node)
{
	struct btrfs_delayed_root *delayed_root;

	if (delayed_node &&
	    test_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &delayed_node->flags)) {
		BUG_ON(!delayed_node->root);
		clear_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &delayed_node->flags);
		delayed_node->count--;

		delayed_root = delayed_node->root->fs_info->delayed_root;
		finish_one_item(delayed_root);
	}
}

static void btrfs_release_delayed_iref(struct btrfs_delayed_node *delayed_node)
{
	struct btrfs_delayed_root *delayed_root;

	ASSERT(delayed_node->root);
	clear_bit(BTRFS_DELAYED_NODE_DEL_IREF, &delayed_node->flags);
	delayed_node->count--;

	delayed_root = delayed_node->root->fs_info->delayed_root;
	finish_one_item(delayed_root);
}

static int __btrfs_update_delayed_inode(struct btrfs_trans_handle *trans,
					struct btrfs_root *root,
					struct btrfs_path *path,
					struct btrfs_delayed_node *node)
{
	struct btrfs_key key;
	struct btrfs_inode_item *inode_item;
	struct extent_buffer *leaf;
	int mod;
	int ret;

	key.objectid = node->inode_id;
	key.type = BTRFS_INODE_ITEM_KEY;
	key.offset = 0;

	if (test_bit(BTRFS_DELAYED_NODE_DEL_IREF, &node->flags))
		mod = -1;
	else
		mod = 1;

	ret = btrfs_lookup_inode(trans, root, path, &key, mod);
	if (ret > 0) {
		btrfs_release_path(path);
		return -ENOENT;
	} else if (ret < 0) {
		return ret;
	}

	leaf = path->nodes[0];
	inode_item = btrfs_item_ptr(leaf, path->slots[0],
				    struct btrfs_inode_item);
	write_extent_buffer(leaf, &node->inode_item, (unsigned long)inode_item,
			    sizeof(struct btrfs_inode_item));
	btrfs_mark_buffer_dirty(leaf);

	if (!test_bit(BTRFS_DELAYED_NODE_DEL_IREF, &node->flags))
		goto no_iref;

	path->slots[0]++;
	if (path->slots[0] >= btrfs_header_nritems(leaf))
		goto search;
again:
	btrfs_item_key_to_cpu(leaf, &key, path->slots[0]);
	if (key.objectid != node->inode_id)
		goto out;

	if (key.type != BTRFS_INODE_REF_KEY &&
	    key.type != BTRFS_INODE_EXTREF_KEY)
		goto out;

	/*
	 * Delayed iref deletion is for the inode who has only one link,
	 * so there is only one iref. The case that several irefs are
	 * in the same item doesn't exist.
	 */
	btrfs_del_item(trans, root, path);
out:
	btrfs_release_delayed_iref(node);
no_iref:
	btrfs_release_path(path);
err_out:
	btrfs_delayed_inode_release_metadata(root, node);
	btrfs_release_delayed_inode(node);

	return ret;

search:
	btrfs_release_path(path);

	key.type = BTRFS_INODE_EXTREF_KEY;
	key.offset = -1;
	ret = btrfs_search_slot(trans, root, &key, path, -1, 1);
	if (ret < 0)
		goto err_out;
	ASSERT(ret);

	ret = 0;
	leaf = path->nodes[0];
	path->slots[0]--;
	goto again;
}

static inline int btrfs_update_delayed_inode(struct btrfs_trans_handle *trans,
					     struct btrfs_root *root,
					     struct btrfs_path *path,
					     struct btrfs_delayed_node *node)
{
	int ret;

	mutex_lock(&node->mutex);
	if (!test_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &node->flags)) {
		mutex_unlock(&node->mutex);
		return 0;
	}

	ret = __btrfs_update_delayed_inode(trans, root, path, node);
	mutex_unlock(&node->mutex);
	return ret;
}

static inline int
__btrfs_commit_inode_delayed_items(struct btrfs_trans_handle *trans,
				   struct btrfs_path *path,
				   struct btrfs_delayed_node *node)
{
	int ret;

	ret = btrfs_insert_delayed_items(trans, path, node->root, node);
	if (ret)
		return ret;

	ret = btrfs_delete_delayed_items(trans, path, node->root, node);
	if (ret)
		return ret;

	ret = btrfs_update_delayed_inode(trans, node->root, path, node);
	return ret;
}

/*
 * Called when committing the transaction.
 * Returns 0 on success.
 * Returns < 0 on error and returns with an aborted transaction with any
 * outstanding delayed items cleaned up.
 */
static int __btrfs_run_delayed_items(struct btrfs_trans_handle *trans,
				     struct btrfs_root *root, int nr)
{
	struct btrfs_delayed_root *delayed_root;
	struct btrfs_delayed_node *curr_node, *prev_node;
	struct btrfs_path *path;
	struct btrfs_block_rsv *block_rsv;
	int ret = 0;
	bool count = (nr > 0);

	if (trans->aborted)
		return -EIO;

	path = btrfs_alloc_path();
	if (!path)
		return -ENOMEM;
	path->leave_spinning = 1;

	block_rsv = trans->block_rsv;
	trans->block_rsv = &root->fs_info->delayed_block_rsv;

	delayed_root = btrfs_get_delayed_root(root);

	curr_node = btrfs_first_delayed_node(delayed_root);
	while (curr_node && (!count || (count && nr--))) {
		ret = __btrfs_commit_inode_delayed_items(trans, path,
							 curr_node);
		if (ret) {
			btrfs_release_delayed_node(curr_node);
			curr_node = NULL;
			btrfs_abort_transaction(trans, root, ret);
			break;
		}

		prev_node = curr_node;
		curr_node = btrfs_next_delayed_node(curr_node);
		btrfs_release_delayed_node(prev_node);
	}

	if (curr_node)
		btrfs_release_delayed_node(curr_node);
	btrfs_free_path(path);
	trans->block_rsv = block_rsv;

	return ret;
}

int btrfs_run_delayed_items(struct btrfs_trans_handle *trans,
			    struct btrfs_root *root)
{
	return __btrfs_run_delayed_items(trans, root, -1);
}

int btrfs_run_delayed_items_nr(struct btrfs_trans_handle *trans,
			       struct btrfs_root *root, int nr)
{
	return __btrfs_run_delayed_items(trans, root, nr);
}

int btrfs_commit_inode_delayed_items(struct btrfs_trans_handle *trans,
				     struct inode *inode)
{
	struct btrfs_delayed_node *delayed_node = btrfs_get_delayed_node(inode);
	struct btrfs_path *path;
	struct btrfs_block_rsv *block_rsv;
	int ret;

	if (!delayed_node)
		return 0;

	mutex_lock(&delayed_node->mutex);
	if (!delayed_node->count) {
		mutex_unlock(&delayed_node->mutex);
		btrfs_release_delayed_node(delayed_node);
		return 0;
	}
	mutex_unlock(&delayed_node->mutex);

	path = btrfs_alloc_path();
	if (!path) {
		btrfs_release_delayed_node(delayed_node);
		return -ENOMEM;
	}
	path->leave_spinning = 1;

	block_rsv = trans->block_rsv;
	trans->block_rsv = &delayed_node->root->fs_info->delayed_block_rsv;

	ret = __btrfs_commit_inode_delayed_items(trans, path, delayed_node);

	btrfs_release_delayed_node(delayed_node);
	btrfs_free_path(path);
	trans->block_rsv = block_rsv;

	return ret;
}

int btrfs_commit_inode_delayed_inode(struct inode *inode)
{
	struct btrfs_trans_handle *trans;
	struct btrfs_delayed_node *delayed_node = btrfs_get_delayed_node(inode);
	struct btrfs_path *path;
	struct btrfs_block_rsv *block_rsv;
	int ret;

	if (!delayed_node)
		return 0;

	mutex_lock(&delayed_node->mutex);
	if (!test_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &delayed_node->flags)) {
		mutex_unlock(&delayed_node->mutex);
		btrfs_release_delayed_node(delayed_node);
		return 0;
	}
	mutex_unlock(&delayed_node->mutex);

	trans = btrfs_join_transaction(delayed_node->root);
	if (IS_ERR(trans)) {
		ret = PTR_ERR(trans);
		goto out;
	}

	path = btrfs_alloc_path();
	if (!path) {
		ret = -ENOMEM;
		goto trans_out;
	}
	path->leave_spinning = 1;

	block_rsv = trans->block_rsv;
	trans->block_rsv = &delayed_node->root->fs_info->delayed_block_rsv;

	mutex_lock(&delayed_node->mutex);
	if (test_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &delayed_node->flags))
		ret = __btrfs_update_delayed_inode(trans, delayed_node->root,
						   path, delayed_node);
	else
		ret = 0;
	mutex_unlock(&delayed_node->mutex);

	btrfs_free_path(path);
	trans->block_rsv = block_rsv;
trans_out:
	btrfs_end_transaction(trans, delayed_node->root);
	btrfs_btree_balance_dirty(delayed_node->root);
out:
	btrfs_release_delayed_node(delayed_node);

	return ret;
}

void btrfs_remove_delayed_node(struct inode *inode)
{
	struct btrfs_delayed_node *delayed_node;

	delayed_node = ACCESS_ONCE(BTRFS_I(inode)->delayed_node);
	if (!delayed_node)
		return;

	BTRFS_I(inode)->delayed_node = NULL;
	btrfs_release_delayed_node(delayed_node);
}

struct btrfs_async_delayed_work {
	struct btrfs_delayed_root *delayed_root;
	int nr;
	struct btrfs_work work;
};

static void btrfs_async_run_delayed_root(struct btrfs_work *work)
{
	struct btrfs_async_delayed_work *async_work;
	struct btrfs_delayed_root *delayed_root;
	struct btrfs_trans_handle *trans;
	struct btrfs_path *path;
	struct btrfs_delayed_node *delayed_node = NULL;
	struct btrfs_root *root;
	struct btrfs_block_rsv *block_rsv;
	int total_done = 0;

	async_work = container_of(work, struct btrfs_async_delayed_work, work);
	delayed_root = async_work->delayed_root;

	path = btrfs_alloc_path();
	if (!path)
		goto out;

again:
	if (atomic_read(&delayed_root->items) < BTRFS_DELAYED_BACKGROUND / 2)
		goto free_path;

	delayed_node = btrfs_first_prepared_delayed_node(delayed_root);
	if (!delayed_node)
		goto free_path;

	path->leave_spinning = 1;
	root = delayed_node->root;

	trans = btrfs_join_transaction(root);
	if (IS_ERR(trans))
		goto release_path;

	block_rsv = trans->block_rsv;
	trans->block_rsv = &root->fs_info->delayed_block_rsv;

	__btrfs_commit_inode_delayed_items(trans, path, delayed_node);

	trans->block_rsv = block_rsv;
	btrfs_end_transaction(trans, root);
	btrfs_btree_balance_dirty_nodelay(root);

release_path:
	btrfs_release_path(path);
	total_done++;

	btrfs_release_prepared_delayed_node(delayed_node);
	if ((async_work->nr == 0 && total_done < BTRFS_DELAYED_WRITEBACK) ||
	    total_done < async_work->nr)
		goto again;

free_path:
	btrfs_free_path(path);
out:
	wake_up(&delayed_root->wait);
	kfree(async_work);
}


static int btrfs_wq_run_delayed_node(struct btrfs_delayed_root *delayed_root,
				     struct btrfs_fs_info *fs_info, int nr)
{
	struct btrfs_async_delayed_work *async_work;

	if (atomic_read(&delayed_root->items) < BTRFS_DELAYED_BACKGROUND ||
	    btrfs_workqueue_normal_congested(fs_info->delayed_workers))
		return 0;

	async_work = kmalloc(sizeof(*async_work), GFP_NOFS);
	if (!async_work)
		return -ENOMEM;

	async_work->delayed_root = delayed_root;
	btrfs_init_work(&async_work->work, btrfs_delayed_meta_helper,
			btrfs_async_run_delayed_root, NULL, NULL);
	async_work->nr = nr;

	btrfs_queue_work(fs_info->delayed_workers, &async_work->work);
	return 0;
}

void btrfs_assert_delayed_root_empty(struct btrfs_root *root)
{
	struct btrfs_delayed_root *delayed_root;
	delayed_root = btrfs_get_delayed_root(root);
	WARN_ON(btrfs_first_delayed_node(delayed_root));
}

static int could_end_wait(struct btrfs_delayed_root *delayed_root, int seq)
{
	int val = atomic_read(&delayed_root->items_seq);

	if (val < seq || val >= seq + BTRFS_DELAYED_BATCH)
		return 1;

	if (atomic_read(&delayed_root->items) < BTRFS_DELAYED_BACKGROUND)
		return 1;

	return 0;
}

void btrfs_balance_delayed_items(struct btrfs_root *root)
{
	struct btrfs_delayed_root *delayed_root;
	struct btrfs_fs_info *fs_info = root->fs_info;

	delayed_root = btrfs_get_delayed_root(root);

	if (atomic_read(&delayed_root->items) < BTRFS_DELAYED_BACKGROUND)
		return;

	if (atomic_read(&delayed_root->items) >= BTRFS_DELAYED_WRITEBACK) {
		int seq;
		int ret;

		seq = atomic_read(&delayed_root->items_seq);

		ret = btrfs_wq_run_delayed_node(delayed_root, fs_info, 0);
		if (ret)
			return;

		wait_event_interruptible(delayed_root->wait,
					 could_end_wait(delayed_root, seq));
		return;
	}

	btrfs_wq_run_delayed_node(delayed_root, fs_info, BTRFS_DELAYED_BATCH);
}

/* Will return 0 or -ENOMEM */
int btrfs_insert_delayed_dir_index(struct btrfs_trans_handle *trans,
				   struct btrfs_root *root, const char *name,
				   int name_len, struct inode *dir,
				   struct btrfs_disk_key *disk_key, u8 type,
				   u64 index)
{
	struct btrfs_delayed_node *delayed_node;
	struct btrfs_delayed_item *delayed_item;
	struct btrfs_dir_item *dir_item;
	int ret;

	delayed_node = btrfs_get_or_create_delayed_node(dir);
	if (IS_ERR(delayed_node))
		return PTR_ERR(delayed_node);

	delayed_item = btrfs_alloc_delayed_item(sizeof(*dir_item) + name_len);
	if (!delayed_item) {
		ret = -ENOMEM;
		goto release_node;
	}

	delayed_item->key.objectid = btrfs_ino(dir);
	delayed_item->key.type = BTRFS_DIR_INDEX_KEY;
	delayed_item->key.offset = index;

	dir_item = (struct btrfs_dir_item *)delayed_item->data;
	dir_item->location = *disk_key;
	btrfs_set_stack_dir_transid(dir_item, trans->transid);
	btrfs_set_stack_dir_data_len(dir_item, 0);
	btrfs_set_stack_dir_name_len(dir_item, name_len);
	btrfs_set_stack_dir_type(dir_item, type);
	memcpy((char *)(dir_item + 1), name, name_len);

	ret = btrfs_delayed_item_reserve_metadata(trans, root, delayed_item);
	/*
	 * we have reserved enough space when we start a new transaction,
	 * so reserving metadata failure is impossible
	 */
	BUG_ON(ret);


	mutex_lock(&delayed_node->mutex);
	ret = __btrfs_add_delayed_insertion_item(delayed_node, delayed_item);
	if (unlikely(ret)) {
		btrfs_err(root->fs_info, "err add delayed dir index item(name: %.*s) "
				"into the insertion tree of the delayed node"
				"(root id: %llu, inode id: %llu, errno: %d)",
				name_len, name, delayed_node->root->objectid,
				delayed_node->inode_id, ret);
		BUG();
	}
	mutex_unlock(&delayed_node->mutex);

release_node:
	btrfs_release_delayed_node(delayed_node);
	return ret;
}

static int btrfs_delete_delayed_insertion_item(struct btrfs_root *root,
					       struct btrfs_delayed_node *node,
					       struct btrfs_key *key)
{
	struct btrfs_delayed_item *item;

	mutex_lock(&node->mutex);
	item = __btrfs_lookup_delayed_insertion_item(node, key);
	if (!item) {
		mutex_unlock(&node->mutex);
		return 1;
	}

	btrfs_delayed_item_release_metadata(root, item);
	btrfs_release_delayed_item(item);
	mutex_unlock(&node->mutex);
	return 0;
}

int btrfs_delete_delayed_dir_index(struct btrfs_trans_handle *trans,
				   struct btrfs_root *root, struct inode *dir,
				   u64 index)
{
	struct btrfs_delayed_node *node;
	struct btrfs_delayed_item *item;
	struct btrfs_key item_key;
	int ret;

	node = btrfs_get_or_create_delayed_node(dir);
	if (IS_ERR(node))
		return PTR_ERR(node);

	item_key.objectid = btrfs_ino(dir);
	item_key.type = BTRFS_DIR_INDEX_KEY;
	item_key.offset = index;

	ret = btrfs_delete_delayed_insertion_item(root, node, &item_key);
	if (!ret)
		goto end;

	item = btrfs_alloc_delayed_item(0);
	if (!item) {
		ret = -ENOMEM;
		goto end;
	}

	item->key = item_key;

	ret = btrfs_delayed_item_reserve_metadata(trans, root, item);
	/*
	 * we have reserved enough space when we start a new transaction,
	 * so reserving metadata failure is impossible.
	 */
	BUG_ON(ret);

	mutex_lock(&node->mutex);
	ret = __btrfs_add_delayed_deletion_item(node, item);
	if (unlikely(ret)) {
		btrfs_err(root->fs_info, "err add delayed dir index item(index: %llu) "
				"into the deletion tree of the delayed node"
				"(root id: %llu, inode id: %llu, errno: %d)",
				index, node->root->objectid, node->inode_id,
				ret);
		BUG();
	}
	mutex_unlock(&node->mutex);
end:
	btrfs_release_delayed_node(node);
	return ret;
}

int btrfs_inode_delayed_dir_index_count(struct inode *inode)
{
	struct btrfs_delayed_node *delayed_node = btrfs_get_delayed_node(inode);

	if (!delayed_node)
		return -ENOENT;

	/*
	 * Since we have held i_mutex of this directory, it is impossible that
	 * a new directory index is added into the delayed node and index_cnt
	 * is updated now. So we needn't lock the delayed node.
	 */
	if (!delayed_node->index_cnt) {
		btrfs_release_delayed_node(delayed_node);
		return -EINVAL;
	}

	BTRFS_I(inode)->index_cnt = delayed_node->index_cnt;
	btrfs_release_delayed_node(delayed_node);
	return 0;
}

void btrfs_get_delayed_items(struct inode *inode, struct list_head *ins_list,
			     struct list_head *del_list)
{
	struct btrfs_delayed_node *delayed_node;
	struct btrfs_delayed_item *item;

	delayed_node = btrfs_get_delayed_node(inode);
	if (!delayed_node)
		return;

	mutex_lock(&delayed_node->mutex);
	item = __btrfs_first_delayed_insertion_item(delayed_node);
	while (item) {
		atomic_inc(&item->refs);
		list_add_tail(&item->readdir_list, ins_list);
		item = __btrfs_next_delayed_item(item);
	}

	item = __btrfs_first_delayed_deletion_item(delayed_node);
	while (item) {
		atomic_inc(&item->refs);
		list_add_tail(&item->readdir_list, del_list);
		item = __btrfs_next_delayed_item(item);
	}
	mutex_unlock(&delayed_node->mutex);
	/*
	 * This delayed node is still cached in the btrfs inode, so refs
	 * must be > 1 now, and we needn't check it is going to be freed
	 * or not.
	 *
	 * Besides that, this function is used to read dir, we do not
	 * insert/delete delayed items in this period. So we also needn't
	 * requeue or dequeue this delayed node.
	 */
	atomic_dec(&delayed_node->refs);
}

void btrfs_put_delayed_items(struct list_head *ins_list,
			     struct list_head *del_list)
{
	struct btrfs_delayed_item *curr, *next;

	list_for_each_entry_safe(curr, next, ins_list, readdir_list) {
		list_del(&curr->readdir_list);
		if (atomic_dec_and_test(&curr->refs))
			kfree(curr);
	}

	list_for_each_entry_safe(curr, next, del_list, readdir_list) {
		list_del(&curr->readdir_list);
		if (atomic_dec_and_test(&curr->refs))
			kfree(curr);
	}
}

int btrfs_should_delete_dir_index(struct list_head *del_list,
				  u64 index)
{
	struct btrfs_delayed_item *curr, *next;
	int ret;

	if (list_empty(del_list))
		return 0;

	list_for_each_entry_safe(curr, next, del_list, readdir_list) {
		if (curr->key.offset > index)
			break;

		list_del(&curr->readdir_list);
		ret = (curr->key.offset == index);

		if (atomic_dec_and_test(&curr->refs))
			kfree(curr);

		if (ret)
			return 1;
		else
			continue;
	}
	return 0;
}

/*
 * btrfs_readdir_delayed_dir_index - read dir info stored in the delayed tree
 *
 */
int btrfs_readdir_delayed_dir_index(struct dir_context *ctx,
				    struct list_head *ins_list, bool *emitted)
{
	struct btrfs_dir_item *di;
	struct btrfs_delayed_item *curr, *next;
	struct btrfs_key location;
	char *name;
	int name_len;
	int over = 0;
	unsigned char d_type;

	if (list_empty(ins_list))
		return 0;

	/*
	 * Changing the data of the delayed item is impossible. So
	 * we needn't lock them. And we have held i_mutex of the
	 * directory, nobody can delete any directory indexes now.
	 */
	list_for_each_entry_safe(curr, next, ins_list, readdir_list) {
		list_del(&curr->readdir_list);

		if (curr->key.offset < ctx->pos) {
			if (atomic_dec_and_test(&curr->refs))
				kfree(curr);
			continue;
		}

		ctx->pos = curr->key.offset;

		di = (struct btrfs_dir_item *)curr->data;
		name = (char *)(di + 1);
		name_len = btrfs_stack_dir_name_len(di);

		d_type = btrfs_filetype_table[di->type];
		btrfs_disk_key_to_cpu(&location, &di->location);

		over = !dir_emit(ctx, name, name_len,
			       location.objectid, d_type);

		if (atomic_dec_and_test(&curr->refs))
			kfree(curr);

		if (over)
			return 1;
		*emitted = true;
	}
	return 0;
}

static void fill_stack_inode_item(struct btrfs_trans_handle *trans,
				  struct btrfs_inode_item *inode_item,
				  struct inode *inode)
{
	btrfs_set_stack_inode_uid(inode_item, i_uid_read(inode));
	btrfs_set_stack_inode_gid(inode_item, i_gid_read(inode));
	btrfs_set_stack_inode_size(inode_item, BTRFS_I(inode)->disk_i_size);
	btrfs_set_stack_inode_mode(inode_item, inode->i_mode);
	btrfs_set_stack_inode_nlink(inode_item, inode->i_nlink);
	btrfs_set_stack_inode_nbytes(inode_item, inode_get_bytes(inode));
	btrfs_set_stack_inode_generation(inode_item,
					 BTRFS_I(inode)->generation);
	btrfs_set_stack_inode_sequence(inode_item, inode->i_version);
	btrfs_set_stack_inode_transid(inode_item, trans->transid);
	btrfs_set_stack_inode_rdev(inode_item, inode->i_rdev);
	btrfs_set_stack_inode_flags(inode_item, BTRFS_I(inode)->flags);
	btrfs_set_stack_inode_block_group(inode_item, 0);

	btrfs_set_stack_timespec_sec(&inode_item->atime,
				     inode->i_atime.tv_sec);
	btrfs_set_stack_timespec_nsec(&inode_item->atime,
				      inode->i_atime.tv_nsec);

	btrfs_set_stack_timespec_sec(&inode_item->mtime,
				     inode->i_mtime.tv_sec);
	btrfs_set_stack_timespec_nsec(&inode_item->mtime,
				      inode->i_mtime.tv_nsec);

	btrfs_set_stack_timespec_sec(&inode_item->ctime,
				     inode->i_ctime.tv_sec);
	btrfs_set_stack_timespec_nsec(&inode_item->ctime,
				      inode->i_ctime.tv_nsec);

	btrfs_set_stack_timespec_sec(&inode_item->otime,
				     BTRFS_I(inode)->i_otime.tv_sec);
	btrfs_set_stack_timespec_nsec(&inode_item->otime,
				     BTRFS_I(inode)->i_otime.tv_nsec);
}

int btrfs_fill_inode(struct inode *inode, u32 *rdev)
{
	struct btrfs_delayed_node *delayed_node;
	struct btrfs_inode_item *inode_item;

	delayed_node = btrfs_get_delayed_node(inode);
	if (!delayed_node)
		return -ENOENT;

	mutex_lock(&delayed_node->mutex);
	if (!test_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &delayed_node->flags)) {
		mutex_unlock(&delayed_node->mutex);
		btrfs_release_delayed_node(delayed_node);
		return -ENOENT;
	}

	inode_item = &delayed_node->inode_item;

	i_uid_write(inode, btrfs_stack_inode_uid(inode_item));
	i_gid_write(inode, btrfs_stack_inode_gid(inode_item));
	btrfs_i_size_write(inode, btrfs_stack_inode_size(inode_item));
	inode->i_mode = btrfs_stack_inode_mode(inode_item);
	set_nlink(inode, btrfs_stack_inode_nlink(inode_item));
	inode_set_bytes(inode, btrfs_stack_inode_nbytes(inode_item));
	BTRFS_I(inode)->generation = btrfs_stack_inode_generation(inode_item);
        BTRFS_I(inode)->last_trans = btrfs_stack_inode_transid(inode_item);

	inode->i_version = btrfs_stack_inode_sequence(inode_item);
	inode->i_rdev = 0;
	*rdev = btrfs_stack_inode_rdev(inode_item);
	BTRFS_I(inode)->flags = btrfs_stack_inode_flags(inode_item);

	inode->i_atime.tv_sec = btrfs_stack_timespec_sec(&inode_item->atime);
	inode->i_atime.tv_nsec = btrfs_stack_timespec_nsec(&inode_item->atime);

	inode->i_mtime.tv_sec = btrfs_stack_timespec_sec(&inode_item->mtime);
	inode->i_mtime.tv_nsec = btrfs_stack_timespec_nsec(&inode_item->mtime);

	inode->i_ctime.tv_sec = btrfs_stack_timespec_sec(&inode_item->ctime);
	inode->i_ctime.tv_nsec = btrfs_stack_timespec_nsec(&inode_item->ctime);

	BTRFS_I(inode)->i_otime.tv_sec =
		btrfs_stack_timespec_sec(&inode_item->otime);
	BTRFS_I(inode)->i_otime.tv_nsec =
		btrfs_stack_timespec_nsec(&inode_item->otime);

	inode->i_generation = BTRFS_I(inode)->generation;
	BTRFS_I(inode)->index_cnt = (u64)-1;

	mutex_unlock(&delayed_node->mutex);
	btrfs_release_delayed_node(delayed_node);
	return 0;
}

int btrfs_delayed_update_inode(struct btrfs_trans_handle *trans,
			       struct btrfs_root *root, struct inode *inode)
{
	struct btrfs_delayed_node *delayed_node;
	int ret = 0;

	delayed_node = btrfs_get_or_create_delayed_node(inode);
	if (IS_ERR(delayed_node))
		return PTR_ERR(delayed_node);

	mutex_lock(&delayed_node->mutex);
	if (test_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &delayed_node->flags)) {
		fill_stack_inode_item(trans, &delayed_node->inode_item, inode);
		goto release_node;
	}

	ret = btrfs_delayed_inode_reserve_metadata(trans, root, inode,
						   delayed_node);
	if (ret)
		goto release_node;

	fill_stack_inode_item(trans, &delayed_node->inode_item, inode);
	set_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &delayed_node->flags);
	delayed_node->count++;
	atomic_inc(&root->fs_info->delayed_root->items);
release_node:
	mutex_unlock(&delayed_node->mutex);
	btrfs_release_delayed_node(delayed_node);
	return ret;
}

int btrfs_delayed_delete_inode_ref(struct inode *inode)
{
	struct btrfs_delayed_node *delayed_node;

	/*
	 * we don't do delayed inode updates during log recovery because it
	 * leads to enospc problems.  This means we also can't do
	 * delayed inode refs
	 */
	if (BTRFS_I(inode)->root->fs_info->log_root_recovering)
		return -EAGAIN;

	delayed_node = btrfs_get_or_create_delayed_node(inode);
	if (IS_ERR(delayed_node))
		return PTR_ERR(delayed_node);

	/*
	 * We don't reserve space for inode ref deletion is because:
	 * - We ONLY do async inode ref deletion for the inode who has only
	 *   one link(i_nlink == 1), it means there is only one inode ref.
	 *   And in most case, the inode ref and the inode item are in the
	 *   same leaf, and we will deal with them at the same time.
	 *   Since we are sure we will reserve the space for the inode item,
	 *   it is unnecessary to reserve space for inode ref deletion.
	 * - If the inode ref and the inode item are not in the same leaf,
	 *   We also needn't worry about enospc problem, because we reserve
	 *   much more space for the inode update than it needs.
	 * - At the worst, we can steal some space from the global reservation.
	 *   It is very rare.
	 */
	mutex_lock(&delayed_node->mutex);
	if (test_bit(BTRFS_DELAYED_NODE_DEL_IREF, &delayed_node->flags))
		goto release_node;

	set_bit(BTRFS_DELAYED_NODE_DEL_IREF, &delayed_node->flags);
	delayed_node->count++;
	atomic_inc(&BTRFS_I(inode)->root->fs_info->delayed_root->items);
release_node:
	mutex_unlock(&delayed_node->mutex);
	btrfs_release_delayed_node(delayed_node);
	return 0;
}

static void __btrfs_kill_delayed_node(struct btrfs_delayed_node *delayed_node)
{
	struct btrfs_root *root = delayed_node->root;
	struct btrfs_delayed_item *curr_item, *prev_item;

	mutex_lock(&delayed_node->mutex);
	curr_item = __btrfs_first_delayed_insertion_item(delayed_node);
	while (curr_item) {
		btrfs_delayed_item_release_metadata(root, curr_item);
		prev_item = curr_item;
		curr_item = __btrfs_next_delayed_item(prev_item);
		btrfs_release_delayed_item(prev_item);
	}

	curr_item = __btrfs_first_delayed_deletion_item(delayed_node);
	while (curr_item) {
		btrfs_delayed_item_release_metadata(root, curr_item);
		prev_item = curr_item;
		curr_item = __btrfs_next_delayed_item(prev_item);
		btrfs_release_delayed_item(prev_item);
	}

	if (test_bit(BTRFS_DELAYED_NODE_DEL_IREF, &delayed_node->flags))
		btrfs_release_delayed_iref(delayed_node);

	if (test_bit(BTRFS_DELAYED_NODE_INODE_DIRTY, &delayed_node->flags)) {
		btrfs_delayed_inode_release_metadata(root, delayed_node);
		btrfs_release_delayed_inode(delayed_node);
	}
	mutex_unlock(&delayed_node->mutex);
}

void btrfs_kill_delayed_inode_items(struct inode *inode)
{
	struct btrfs_delayed_node *delayed_node;

	delayed_node = btrfs_get_delayed_node(inode);
	if (!delayed_node)
		return;

	__btrfs_kill_delayed_node(delayed_node);
	btrfs_release_delayed_node(delayed_node);
}

void btrfs_kill_all_delayed_nodes(struct btrfs_root *root)
{
	u64 inode_id = 0;
	struct btrfs_delayed_node *delayed_nodes[8];
	int i, n;

	while (1) {
		spin_lock(&root->inode_lock);
		n = radix_tree_gang_lookup(&root->delayed_nodes_tree,
					   (void **)delayed_nodes, inode_id,
					   ARRAY_SIZE(delayed_nodes));
		if (!n) {
			spin_unlock(&root->inode_lock);
			break;
		}

		inode_id = delayed_nodes[n - 1]->inode_id + 1;

		for (i = 0; i < n; i++)
			atomic_inc(&delayed_nodes[i]->refs);
		spin_unlock(&root->inode_lock);

		for (i = 0; i < n; i++) {
			__btrfs_kill_delayed_node(delayed_nodes[i]);
			btrfs_release_delayed_node(delayed_nodes[i]);
		}
	}
}

void btrfs_destroy_delayed_inodes(struct btrfs_root *root)
{
	struct btrfs_delayed_root *delayed_root;
	struct btrfs_delayed_node *curr_node, *prev_node;

	delayed_root = btrfs_get_delayed_root(root);

	curr_node = btrfs_first_delayed_node(delayed_root);
	while (curr_node) {
		__btrfs_kill_delayed_node(curr_node);

		prev_node = curr_node;
		curr_node = btrfs_next_delayed_node(curr_node);
		btrfs_release_delayed_node(prev_node);
	}
}

