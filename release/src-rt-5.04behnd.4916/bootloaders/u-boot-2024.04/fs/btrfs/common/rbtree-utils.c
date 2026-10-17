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

#include <linux/errno.h>
#include "rbtree-utils.h"

int rb_insert(struct rb_root *root, struct rb_node *node,
	      rb_compare_nodes comp)
{
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	int ret;

	while(*p) {
		parent = *p;

		ret = comp(parent, node);
		if (ret < 0)
			p = &(*p)->rb_left;
		else if (ret > 0)
			p = &(*p)->rb_right;
		else
			return -EEXIST;
	}

	rb_link_node(node, parent, p);
	rb_insert_color(node, root);
	return 0;
}

struct rb_node *rb_search(struct rb_root *root, void *key, rb_compare_keys comp,
			  struct rb_node **next_ret)
{
	struct rb_node *n = root->rb_node;
	struct rb_node *parent = NULL;
	int ret = 0;

	while(n) {
		parent = n;

		ret = comp(n, key);
		if (ret < 0)
			n = n->rb_left;
		else if (ret > 0)
			n = n->rb_right;
		else
			return n;
	}

	if (!next_ret)
		return NULL;

	if (parent && ret > 0)
		parent = rb_next(parent);

	*next_ret = parent;
	return NULL;
}

void rb_free_nodes(struct rb_root *root, rb_free_node free_node)
{
	struct rb_node *node;

	while ((node = rb_first(root))) {
		rb_erase(node, root);
		free_node(node);
	}
}
