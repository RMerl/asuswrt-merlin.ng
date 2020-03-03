/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * lustre/include/interval_tree.h
 *
 * Author: Huang Wei <huangwei@clusterfs.com>
 * Author: Jay Xiong <jinshan.xiong@sun.com>
 */

#ifndef _INTERVAL_H__
#define _INTERVAL_H__

#include "../../include/linux/libcfs/libcfs.h"	/* LASSERT. */

struct interval_node {
	struct interval_node   *in_left;
	struct interval_node   *in_right;
	struct interval_node   *in_parent;
	unsigned		in_color:1,
				in_intree:1, /** set if the node is in tree */
				in_res1:30;
	__u8		    in_res2[4];  /** tags, 8-bytes aligned */
	__u64		   in_max_high;
	struct interval_node_extent {
		__u64 start;
		__u64 end;
	} in_extent;
};

enum interval_iter {
	INTERVAL_ITER_CONT = 1,
	INTERVAL_ITER_STOP = 2
};

static inline int interval_is_intree(struct interval_node *node)
{
	return node->in_intree == 1;
}

static inline __u64 interval_low(struct interval_node *node)
{
	return node->in_extent.start;
}

static inline __u64 interval_high(struct interval_node *node)
{
	return node->in_extent.end;
}

static inline void interval_set(struct interval_node *node,
				__u64 start, __u64 end)
{
	LASSERT(start <= end);
	node->in_extent.start = start;
	node->in_extent.end = end;
	node->in_max_high = end;
}

/* Rules to write an interval callback.
 *  - the callback returns INTERVAL_ITER_STOP when it thinks the iteration
 *    should be stopped. It will then cause the iteration function to return
 *    immediately with return value INTERVAL_ITER_STOP.
 *  - callbacks for interval_iterate and interval_iterate_reverse: Every
 *    nodes in the tree will be set to @node before the callback being called
 *  - callback for interval_search: Only overlapped node will be set to @node
 *    before the callback being called.
 */
typedef enum interval_iter (*interval_callback_t)(struct interval_node *node,
						  void *args);

struct interval_node *interval_insert(struct interval_node *node,
				      struct interval_node **root);
void interval_erase(struct interval_node *node, struct interval_node **root);

/* Search the extents in the tree and call @func for each overlapped
 * extents. */
enum interval_iter interval_search(struct interval_node *root,
				   struct interval_node_extent *ex,
				   interval_callback_t func, void *data);

/* Iterate every node in the tree - by reverse order or regular order. */
enum interval_iter interval_iterate(struct interval_node *root,
				    interval_callback_t func, void *data);
enum interval_iter interval_iterate_reverse(struct interval_node *root,
				    interval_callback_t func, void *data);

void interval_expand(struct interval_node *root,
		     struct interval_node_extent *ext,
		     struct interval_node_extent *limiter);
int interval_is_overlapped(struct interval_node *root,
			   struct interval_node_extent *ex);
struct interval_node *interval_find(struct interval_node *root,
				    struct interval_node_extent *ex);
#endif
