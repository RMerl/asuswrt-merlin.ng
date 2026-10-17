/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _DM_OFNODE_DECL_H
#define _DM_OFNODE_DECL_H

/**
 * typedef union ofnode_union ofnode - reference to a device tree node
 *
 * This union can hold either a straightforward pointer to a struct device_node
 * in the live device tree, or an offset within the flat device tree. In the
 * latter case, the pointer value is just the integer offset within the flat DT.
 *
 * Thus we can reference nodes in both the live tree (once available) and the
 * flat tree (until then). Functions are available to translate between an
 * ofnode and either an offset or a `struct device_node *`.
 *
 * The reference can also hold a null offset, in which case the pointer value
 * here is NULL. This corresponds to a struct device_node * value of
 * NULL, or an offset of -1.
 *
 * There is no ambiguity as to whether ofnode holds an offset or a node
 * pointer: when the live tree is active it holds a node pointer, otherwise it
 * holds an offset. The value itself does not need to be unique and in theory
 * the same value could point to a valid device node or a valid offset. We
 * could arrange for a unique value to be used (e.g. by making the pointer
 * point to an offset within the flat device tree in the case of an offset) but
 * this increases code size slightly due to the subtraction. Since it offers no
 * real benefit, the approach described here seems best.
 *
 * Where multiple trees are in use, this works without any trouble with live
 * tree, except for aliases, such as ofnode_path("mmc0"), which only work on the
 * control FDT. When the flat tree is in use, the trees are registered and a
 * 'tree ID' is encoded into the top bits of @of_offset - see immediately below
 * for the associated macro definitions. Note that 64-bit machines use the same
 * encoding, even though there is more space available. This is partly because
 * the FDT format contains 32-bit values for things like the string-table
 * offset, therefore 64-bit offsets cannot be supported anyway.
 *
 * For the multiple-tree case, an invalid offset (i.e. with of_offset < 0) is
 * still invalid. It does not contain a tree ID. So there is no way of knowing
 * which tree produced the invalid offset.
 *
 * @np: Pointer to device node, used for live tree
 * @of_offset: Pointer into flat device tree, used for flat tree. Note that this
 *	is not a really a pointer to a node: it is an offset value. See above.
 */
typedef union ofnode_union {
	struct device_node *np;
	long of_offset;
} ofnode;

/* shift for the tree ID within of_offset */
#define OF_TREE_SHIFT 28

/* mask to obtain the device tree offset from of_offset */
#define OF_TREE_MASK ((1 << OF_TREE_SHIFT) - 1)

/* encode a tree ID and node offset into an of_offset value */
#define OFTREE_NODE(tree_id, offs)	((tree_id) << OF_TREE_SHIFT | (offs))

/* decode the node offset from an of_offset value */
#define OFTREE_OFFSET(of_offs)		((of_offs) & OF_TREE_MASK)

/* decode the tree ID from an of_offset value */
#define OFTREE_TREE_ID(of_offs)		((of_offs) >> OF_TREE_SHIFT)

/* encode a node offset in the tree given by another node's of_offset value */
#define OFTREE_MAKE_NODE(other_of_offset, offs)	\
		(((offs) & OF_TREE_MASK) | ((other_of_offset) & ~OF_TREE_MASK))

/**
 * struct ofprop - reference to a property of a device tree node
 *
 * This struct hold the reference on one property of one node,
 * using struct ofnode and an offset within the flat device tree or either
 * a pointer to a struct property in the live device tree.
 *
 * Thus we can reference arguments in both the live tree and the flat tree.
 *
 * The property reference can also hold a null reference. This corresponds to
 * a struct property NULL pointer or an offset of -1.
 *
 * @node: Pointer to device node
 * @offset: Pointer into flat device tree, used for flat tree.
 * @prop: Pointer to property, used for live tree.
 */

struct ofprop {
	ofnode node;
	union {
		int offset;
		const struct property *prop;
	};
};

/**
 * union oftree_union - reference to a tree of device tree nodes
 *
 * One or other of the members is used, depending on of_live_active()
 *
 * @np: Pointer to roott device node, used for live tree
 * @fdt: Pointer to the flat device tree, used for flat tree
 */
typedef union oftree_union {
	struct device_node *np;
	void *fdt;
} oftree;

#endif

