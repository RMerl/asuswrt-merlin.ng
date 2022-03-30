/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _DM_OF_H
#define _DM_OF_H

#include <asm/u-boot.h>
#include <asm/global_data.h>

/* integer value within a device tree property which references another node */
typedef u32 phandle;

/**
 * struct property: Device tree property
 *
 * @name: Property name
 * @length: Length of property in bytes
 * @value: Pointer to property value
 * @next: Pointer to next property, or NULL if none
 */
struct property {
	char *name;
	int length;
	void *value;
	struct property *next;
};

/**
 * struct device_node: Device tree node
 *
 * @name: Node name
 * @type: Node type (value of device_type property) or "<NULL>" if none
 * @phandle: Phandle value of this none, or 0 if none
 * @full_name: Full path to node, e.g. "/bus@1/spi@1100"
 * @properties: Pointer to head of list of properties, or NULL if none
 * @parent: Pointer to parent node, or NULL if this is the root node
 * @child: Pointer to head of child node list, or NULL if no children
 * @sibling: Pointer to the next sibling node, or NULL if this is the last
 */
struct device_node {
	const char *name;
	const char *type;
	phandle phandle;
	const char *full_name;

	struct property *properties;
	struct device_node *parent;
	struct device_node *child;
	struct device_node *sibling;
};

#define OF_MAX_PHANDLE_ARGS 16

/**
 * struct of_phandle_args - structure to hold phandle and arguments
 *
 * This is used when decoding a phandle in a device tree property. Typically
 * these look like this:
 *
 * wibble {
 *    phandle = <5>;
 * };
 *
 * ...
 * some-prop = <&wibble 1 2 3>
 *
 * Here &node is the phandle of the node 'wibble', i.e. 5. There are three
 * arguments: 1, 2, 3.
 *
 * So when decoding the phandle in some-prop, np will point to wibble,
 * args_count will be 3 and the three arguments will be in args.
 *
 * @np: Node that the phandle refers to
 * @args_count: Number of arguments
 * @args: Argument values
 */
struct of_phandle_args {
	struct device_node *np;
	int args_count;
	uint32_t args[OF_MAX_PHANDLE_ARGS];
};

DECLARE_GLOBAL_DATA_PTR;

/**
 * of_live_active() - check if livetree is active
 *
 * @returns true if livetree is active, false it not
 */
#ifdef CONFIG_OF_LIVE
static inline bool of_live_active(void)
{
	return gd->of_root != NULL;
}
#else
static inline bool of_live_active(void)
{
	return false;
}
#endif

#define OF_BAD_ADDR	((u64)-1)

static inline const char *of_node_full_name(const struct device_node *np)
{
	return np ? np->full_name : "<no-node>";
}

/* Default #address and #size cells */
#if !defined(OF_ROOT_NODE_ADDR_CELLS_DEFAULT)
#define OF_ROOT_NODE_ADDR_CELLS_DEFAULT 1
#define OF_ROOT_NODE_SIZE_CELLS_DEFAULT 1
#endif

/* Default string compare functions */
#if !defined(of_compat_cmp)
#define of_compat_cmp(s1, s2, l)	strcasecmp((s1), (s2))
#define of_prop_cmp(s1, s2)		strcmp((s1), (s2))
#define of_node_cmp(s1, s2)		strcasecmp((s1), (s2))
#endif

/* Helper to read a big number; size is in cells (not bytes) */
static inline u64 of_read_number(const __be32 *cell, int size)
{
	u64 r = 0;
	while (size--)
		r = (r << 32) | be32_to_cpu(*(cell++));
	return r;
}

/* Like of_read_number, but we want an unsigned long result */
static inline unsigned long of_read_ulong(const __be32 *cell, int size)
{
	/* toss away upper bits if unsigned long is smaller than u64 */
	return of_read_number(cell, size);
}

#endif
