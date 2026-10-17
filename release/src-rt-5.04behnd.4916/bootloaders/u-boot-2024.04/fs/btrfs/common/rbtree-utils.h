/*
 * Copyright (C) 2014 Facebook.  All rights reserved.
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

#ifndef __RBTREE_UTILS__
#define __RBTREE_UTILS__

#include <linux/rbtree.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The common insert/search/free functions */
typedef int (*rb_compare_nodes)(struct rb_node *node1, struct rb_node *node2);
typedef int (*rb_compare_keys)(struct rb_node *node, void *key);
typedef void (*rb_free_node)(struct rb_node *node);

int rb_insert(struct rb_root *root, struct rb_node *node,
	      rb_compare_nodes comp);
/*
 * In some cases, we need return the next node if we don't find the node we
 * specify. At this time, we can use next_ret.
 */
struct rb_node *rb_search(struct rb_root *root, void *key, rb_compare_keys comp,
			  struct rb_node **next_ret);
void rb_free_nodes(struct rb_root *root, rb_free_node free_node);

#define FREE_RB_BASED_TREE(name, free_func)		\
static void free_##name##_tree(struct rb_root *root)	\
{							\
	rb_free_nodes(root, free_func);			\
}

#ifdef __cplusplus
}
#endif

#endif
