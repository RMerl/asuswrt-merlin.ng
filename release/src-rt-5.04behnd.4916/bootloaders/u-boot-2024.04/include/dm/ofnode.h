/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _DM_OFNODE_H
#define _DM_OFNODE_H

/* TODO(sjg@chromium.org): Drop fdtdec.h include */
#include <fdtdec.h>
#include <dm/of.h>
#include <dm/of_access.h>
#include <log.h>
#include <phy_interface.h>

/* Enable checks to protect against invalid calls */
#undef OF_CHECKS

struct abuf;
struct resource;

#include <dm/ofnode_decl.h>
#include <linux/errno.h>

struct ofnode_phandle_args {
	ofnode node;
	int args_count;
	uint32_t args[OF_MAX_PHANDLE_ARGS];
};

#if CONFIG_IS_ENABLED(OFNODE_MULTI_TREE)
/**
 * oftree_reset() - reset the state of the oftree list
 *
 * Reset the oftree list so it can be started again. This should be called
 * once the control FDT is in place, but before the ofnode interface is used.
 */
void oftree_reset(void);

/**
 * ofnode_to_fdt() - convert an ofnode to a flat DT pointer
 *
 * This cannot be called if the reference contains a node pointer.
 *
 * @node: Reference containing offset (possibly invalid)
 * Return: DT offset (can be NULL)
 */
__attribute_const__ void *ofnode_to_fdt(ofnode node);

/**
 * ofnode_to_offset() - convert an ofnode to a flat DT offset
 *
 * This cannot be called if the reference contains a node pointer.
 *
 * @node: Reference containing offset (possibly invalid)
 * Return: DT offset (can be -1)
 */
__attribute_const__ int ofnode_to_offset(ofnode node);

/**
 * oftree_from_fdt() - Returns an oftree from a flat device tree pointer
 *
 * If @fdt is not already registered in the list of current device trees, it is
 * added to the list.
 *
 * @fdt: Device tree to use
 *
 * Returns: reference to the given node
 */
oftree oftree_from_fdt(void *fdt);

/**
 * noffset_to_ofnode() - convert a DT offset to an ofnode
 *
 * @other_node: Node in the same tree to use as a reference
 * @of_offset: DT offset (either valid, or -1)
 * Return: reference to the associated DT offset
 */
ofnode noffset_to_ofnode(ofnode other_node, int of_offset);

#else /* !OFNODE_MULTI_TREE */
static inline void oftree_reset(void) {}

static inline void *ofnode_to_fdt(ofnode node)
{
#ifdef OF_CHECKS
	if (of_live_active())
		return NULL;
#endif
	/* Use the control FDT by default */
	return (void *)gd->fdt_blob;
}

static inline __attribute_const__ int ofnode_to_offset(ofnode node)
{
#ifdef OF_CHECKS
	if (of_live_active())
		return -1;
#endif
	return node.of_offset;
}

static inline oftree oftree_from_fdt(void *fdt)
{
	oftree tree;

	/* we cannot access other trees without OFNODE_MULTI_TREE */
	if (fdt == gd->fdt_blob)
		tree.fdt = fdt;
	else
		tree.fdt = NULL;

	return tree;
}

static inline ofnode noffset_to_ofnode(ofnode other_node, int of_offset)
{
	ofnode node;

	if (of_live_active())
		node.np = NULL;
	else
		node.of_offset = of_offset;

	return node;
}

#endif /* OFNODE_MULTI_TREE */

/**
 * oftree_new() - Create a new, empty tree
 *
 * @treep: Returns a pointer to the tree, on success
 * Returns: 0 on success, -ENOMEM if out of memory, -E2BIG if !OF_LIVE and
 * there are too many (flattrees) already
 */
int oftree_new(oftree *treep);

/**
 * oftree_to_fdt() - Convert an oftree to a flat FDT
 *
 * @tree: tree to flatten (if livetree) or copy (if not)
 * @buf: Returns inited buffer containing the newly created flat tree. Note
 * that for flat tree the buffer is not allocated. In either case the caller
 * must call abut_uninit() to free any memory used by @buf
 * Return: 0 on success, -ENOMEM if out of memory, other -ve value for any other
 * error
 */
int oftree_to_fdt(oftree tree, struct abuf *buf);

/**
 * ofnode_to_np() - convert an ofnode to a live DT node pointer
 *
 * This cannot be called if the reference contains an offset.
 *
 * @node: Reference containing struct device_node * (possibly invalid)
 * Return: pointer to device node (can be NULL)
 */
static inline struct device_node *ofnode_to_np(ofnode node)
{
#ifdef OF_CHECKS
	if (!of_live_active())
		return NULL;
#endif
	return node.np;
}

/**
 * ofnode_valid() - check if an ofnode is valid
 *
 * @node: Reference containing offset (possibly invalid)
 * Return: true if the reference contains a valid ofnode, false if not
 */
static inline bool ofnode_valid(ofnode node)
{
	if (of_live_active())
		return node.np != NULL;
	else
		return node.of_offset >= 0;
}

/**
 * oftree_lookup_fdt() - obtain the FDT pointer from an oftree
 *
 * This can only be called when flat tree is enabled
 *
 * @tree: Tree to look at
 * @return FDT pointer from the tree
 */
static inline void *oftree_lookup_fdt(oftree tree)
{
	if (of_live_active())
		return NULL;
	else
		return tree.fdt;
}

/**
 * offset_to_ofnode() - convert a DT offset to an ofnode
 *
 * @of_offset: DT offset (either valid, or -1)
 * Return: reference to the associated DT offset
 */
static inline ofnode offset_to_ofnode(int of_offset)
{
	ofnode node;

	if (of_live_active())
		node.np = NULL;
	else
		node.of_offset = of_offset >= 0 ? of_offset : -1;

	return node;
}

/**
 * np_to_ofnode() - convert a node pointer to an ofnode
 *
 * @np: Live node pointer (can be NULL)
 * Return: reference to the associated node pointer
 */
static inline ofnode np_to_ofnode(struct device_node *np)
{
	ofnode node;

	node.np = np;

	return node;
}

/**
 * ofnode_is_np() - check if a reference is a node pointer
 *
 * This function associated that if there is a valid live tree then all
 * references will use it. This is because using the flat DT when the live tree
 * is valid is not permitted.
 *
 * @node: reference to check (possibly invalid)
 * Return: true if the reference is a live node pointer, false if it is a DT
 * offset
 */
static inline bool ofnode_is_np(ofnode node)
{
#ifdef OF_CHECKS
	/*
	 * Check our assumption that flat tree offsets are not used when a
	 * live tree is in use.
	 */
	assert(!ofnode_valid(node) ||
	       (of_live_active() ? ofnode_to_np(node)
				  : ofnode_to_np(node)));
#endif
	return of_live_active() && ofnode_valid(node);
}

/**
 * ofnode_equal() - check if two references are equal
 *
 * @ref1: first reference to check (possibly invalid)
 * @ref2: second reference to check (possibly invalid)
 * Return: true if equal, else false
 */
static inline bool ofnode_equal(ofnode ref1, ofnode ref2)
{
	/* We only need to compare the contents */
	return ref1.of_offset == ref2.of_offset;
}

/**
 * oftree_valid() - check if an oftree is valid
 *
 * @tree: Reference containing oftree
 * Return: true if the reference contains a valid oftree, false if node
 */
static inline bool oftree_valid(oftree tree)
{
	if (of_live_active())
		return tree.np;
	else
		return tree.fdt;
}

/**
 * oftree_null() - Obtain a null oftree
 *
 * This returns an oftree which points to no tree. It works both with the flat
 * tree and livetree.
 */
static inline oftree oftree_null(void)
{
	oftree tree;

	if (of_live_active())
		tree.np = NULL;
	else
		tree.fdt = NULL;

	return tree;
}

/**
 * ofnode_null() - Obtain a null ofnode
 *
 * This returns an ofnode which points to no node. It works both with the flat
 * tree and livetree.
 */
static inline ofnode ofnode_null(void)
{
	ofnode node;

	if (of_live_active())
		node.np = NULL;
	else
		node.of_offset = -1;

	return node;
}

static inline ofnode ofnode_root(void)
{
	ofnode node;

	if (of_live_active())
		node.np = gd_of_root();
	else
		node.of_offset = 0;

	return node;
}

/**
 * ofprop_valid() - check if an ofprop is valid
 *
 * @prop: Pointer to ofprop to check
 * Return: true if the reference contains a valid ofprop, false if not
 */
static inline bool ofprop_valid(struct ofprop *prop)
{
	if (of_live_active())
		return prop->prop;
	else
		return prop->offset >= 0;
}

/**
 * oftree_default() - Returns the default device tree (U-Boot's control FDT)
 *
 * Returns: reference to the control FDT
 */
static inline oftree oftree_default(void)
{
	oftree tree;

	if (of_live_active())
		tree.np = gd_of_root();
	else
		tree.fdt = (void *)gd->fdt_blob;

	return tree;
}

/**
 * oftree_from_np() - Returns an oftree from a node pointer
 *
 * @root: Root node of the tree
 * Returns: reference to the given node
 */
static inline oftree oftree_from_np(struct device_node *root)
{
	oftree tree;

	tree.np = root;

	return tree;
}

/**
 * oftree_dispose() - Dispose of an oftree
 *
 * This can be used to dispose of a tree that has been created (other than
 * the control FDT which must not be disposed)
 *
 * @tree: Tree to dispose
 */
void oftree_dispose(oftree tree);

/**
 * ofnode_name_eq() - Check if the node name is equivalent to a given name
 *                    ignoring the unit address
 *
 * @node:	valid node reference that has to be compared
 * @name:	name that has to be compared with the node name
 * Return: true if matches, false if it doesn't match
 */
bool ofnode_name_eq(ofnode node, const char *name);

/**
 * ofnode_read_u8() - Read a 8-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int ofnode_read_u8(ofnode node, const char *propname, u8 *outp);

/**
 * ofnode_read_u8_default() - Read a 8-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u8 ofnode_read_u8_default(ofnode node, const char *propname, u8 def);

/**
 * ofnode_read_u16() - Read a 16-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int ofnode_read_u16(ofnode node, const char *propname, u16 *outp);

/**
 * ofnode_read_u16_default() - Read a 16-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u16 ofnode_read_u16_default(ofnode node, const char *propname, u16 def);

/**
 * ofnode_read_u32() - Read a 32-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int ofnode_read_u32(ofnode node, const char *propname, u32 *outp);

/**
 * ofnode_read_u32_index() - Read a 32-bit integer from a multi-value property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @index:	index of the integer to return
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int ofnode_read_u32_index(ofnode node, const char *propname, int index,
			  u32 *outp);

/**
 * ofnode_read_u64_index() - Read a 64-bit integer from a multi-value property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @index:	index of the integer to return
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int ofnode_read_u64_index(ofnode node, const char *propname, int index,
			  u64 *outp);

/**
 * ofnode_read_s32() - Read a 32-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
static inline int ofnode_read_s32(ofnode node, const char *propname,
				  s32 *outp)
{
	return ofnode_read_u32(node, propname, (u32 *)outp);
}

/**
 * ofnode_read_u32_default() - Read a 32-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u32 ofnode_read_u32_default(ofnode node, const char *propname, u32 def);

/**
 * ofnode_read_u32_index_default() - Read a 32-bit integer from a multi-value
 *                                   property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @index:	index of the integer to return
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u32 ofnode_read_u32_index_default(ofnode node, const char *propname, int index,
				  u32 def);

/**
 * ofnode_read_s32_default() - Read a 32-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
int ofnode_read_s32_default(ofnode node, const char *propname, s32 def);

/**
 * ofnode_read_u64() - Read a 64-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int ofnode_read_u64(ofnode node, const char *propname, u64 *outp);

/**
 * ofnode_read_u64_default() - Read a 64-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u64 ofnode_read_u64_default(ofnode node, const char *propname, u64 def);

/**
 * ofnode_read_prop() - Read a property from a node
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read
 * @sizep:	if non-NULL, returns the size of the property, or an error code
 *              if not found
 * Return: property value, or NULL if there is no such property
 */
const void *ofnode_read_prop(ofnode node, const char *propname, int *sizep);

/**
 * ofnode_read_string() - Read a string from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read
 * Return: string from property value, or NULL if there is no such property
 */
const char *ofnode_read_string(ofnode node, const char *propname);

/**
 * ofnode_read_u32_array() - Find and read an array of 32 bit integers
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read
 * @out_values:	pointer to return value, modified only if return value is 0
 * @sz:		number of array elements to read
 * Return: 0 on success, -EINVAL if the property does not exist,
 * -ENODATA if property does not have a value, and -EOVERFLOW if the
 * property data isn't large enough
 *
 * Search for a property in a device node and read 32-bit value(s) from
 * it.
 *
 * The out_values is modified only if a valid u32 value can be decoded.
 */
int ofnode_read_u32_array(ofnode node, const char *propname,
			  u32 *out_values, size_t sz);

/**
 * ofnode_read_bool() - read a boolean value from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of property to read
 * Return: true if property is present (meaning true), false if not present
 */
bool ofnode_read_bool(ofnode node, const char *propname);

/**
 * ofnode_find_subnode() - find a named subnode of a parent node
 *
 * @node:	valid reference to parent node
 * @subnode_name: name of subnode to find
 * Return: reference to subnode (which can be invalid if there is no such
 * subnode)
 */
ofnode ofnode_find_subnode(ofnode node, const char *subnode_name);

#if CONFIG_IS_ENABLED(DM_INLINE_OFNODE)
#include <asm/global_data.h>

static inline bool ofnode_is_enabled(ofnode node)
{
	if (ofnode_is_np(node)) {
		return of_device_is_available(ofnode_to_np(node));
	} else {
		return fdtdec_get_is_enabled(gd->fdt_blob,
					     ofnode_to_offset(node));
	}
}

static inline ofnode ofnode_first_subnode(ofnode node)
{
	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		return np_to_ofnode(node.np->child);

	return offset_to_ofnode(
		fdt_first_subnode(gd->fdt_blob, ofnode_to_offset(node)));
}

static inline ofnode ofnode_next_subnode(ofnode node)
{
	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		return np_to_ofnode(node.np->sibling);

	return offset_to_ofnode(
		fdt_next_subnode(gd->fdt_blob, ofnode_to_offset(node)));
}
#else
/**
 * ofnode_is_enabled() - Checks whether a node is enabled.
 * This looks for a 'status' property. If this exists, then returns true if
 * the status is 'okay' and false otherwise. If there is no status property,
 * it returns true on the assumption that anything mentioned should be enabled
 * by default.
 *
 * @node: node to examine
 * Return: false (not enabled) or true (enabled)
 */
bool ofnode_is_enabled(ofnode node);

/**
 * ofnode_first_subnode() - find the first subnode of a parent node
 *
 * @node:	valid reference to a valid parent node
 * Return: reference to the first subnode (which can be invalid if the parent
 * node has no subnodes)
 */
ofnode ofnode_first_subnode(ofnode node);

/**
 * ofnode_next_subnode() - find the next sibling of a subnode
 *
 * @node:	valid reference to previous node (sibling)
 * Return: reference to the next subnode (which can be invalid if the node
 * has no more siblings)
 */
ofnode ofnode_next_subnode(ofnode node);
#endif /* DM_INLINE_OFNODE */

/**
 * ofnode_get_parent() - get the ofnode's parent (enclosing ofnode)
 *
 * @node: valid node to look up
 * Return: ofnode reference of the parent node
 */
ofnode ofnode_get_parent(ofnode node);

/**
 * ofnode_get_name() - get the name of a node
 *
 * @node: valid node to look up
 * Return: name of node (for the root node this is "")
 */
const char *ofnode_get_name(ofnode node);

/**
 * ofnode_get_path() - get the full path of a node
 *
 * @node: valid node to look up
 * @buf: buffer to write the node path into
 * @buflen: buffer size
 * Return: 0 if OK, -ve on error
 */
int ofnode_get_path(ofnode node, char *buf, int buflen);

/**
 * ofnode_get_by_phandle() - get ofnode from phandle
 *
 * This uses the default (control) device tree
 *
 * @phandle:	phandle to look up
 * Return: ofnode reference to the phandle
 */
ofnode ofnode_get_by_phandle(uint phandle);

/**
 * oftree_get_by_phandle() - get ofnode from phandle
 *
 * @tree:	tree to use
 * @phandle:	phandle to look up
 * Return: ofnode reference to the phandle
 */
ofnode oftree_get_by_phandle(oftree tree, uint phandle);

/**
 * ofnode_read_size() - read the size of a property
 *
 * @node: node to check
 * @propname: property to check
 * Return: size of property if present, or -EINVAL if not
 */
int ofnode_read_size(ofnode node, const char *propname);

/**
 * ofnode_get_addr_size_index() - get an address/size from a node
 *				  based on index
 *
 * This reads the register address/size from a node based on index
 *
 * @node: node to read from
 * @index: Index of address to read (0 for first)
 * @size: Pointer to size of the address
 * Return: address, or FDT_ADDR_T_NONE if not present or invalid
 */
fdt_addr_t ofnode_get_addr_size_index(ofnode node, int index,
				      fdt_size_t *size);

/**
 * ofnode_get_addr_size_index_notrans() - get an address/size from a node
 *					  based on index, without address
 *					  translation
 *
 * This reads the register address/size from a node based on index.
 * The resulting address is not translated. Useful for example for on-disk
 * addresses.
 *
 * @node: node to read from
 * @index: Index of address to read (0 for first)
 * @size: Pointer to size of the address
 * Return: address, or FDT_ADDR_T_NONE if not present or invalid
 */
fdt_addr_t ofnode_get_addr_size_index_notrans(ofnode node, int index,
					      fdt_size_t *size);

/**
 * ofnode_get_addr_index() - get an address from a node
 *
 * This reads the register address from a node
 *
 * @node: node to read from
 * @index: Index of address to read (0 for first)
 * Return: address, or FDT_ADDR_T_NONE if not present or invalid
 */
fdt_addr_t ofnode_get_addr_index(ofnode node, int index);

/**
 * ofnode_get_addr() - get an address from a node
 *
 * This reads the register address from a node
 *
 * @node: node to read from
 * Return: address, or FDT_ADDR_T_NONE if not present or invalid
 */
fdt_addr_t ofnode_get_addr(ofnode node);

/**
 * ofnode_get_size() - get size from a node
 *
 * This reads the register size from a node
 *
 * @node: node to read from
 * Return: size of the address, or FDT_SIZE_T_NONE if not present or invalid
 */
fdt_size_t ofnode_get_size(ofnode node);

/**
 * ofnode_stringlist_search() - find a string in a string list and return index
 *
 * Note that it is possible for this function to succeed on property values
 * that are not NUL-terminated. That's because the function will stop after
 * finding the first occurrence of @string. This can for example happen with
 * small-valued cell properties, such as #address-cells, when searching for
 * the empty string.
 *
 * @node: node to check
 * @propname: name of the property containing the string list
 * @string: string to look up in the string list
 *
 * Return:
 *   the index of the string in the list of strings
 *   -ENODATA if the property is not found
 *   -EINVAL on some other error
 */
int ofnode_stringlist_search(ofnode node, const char *propname,
			     const char *string);

/**
 * ofnode_read_string_index() - obtain an indexed string from a string list
 *
 * Note that this will successfully extract strings from properties with
 * non-NUL-terminated values. For example on small-valued cell properties
 * this function will return the empty string.
 *
 * If non-NULL, the length of the string (on success) or a negative error-code
 * (on failure) will be stored in the integer pointer to by lenp.
 *
 * @node: node to check
 * @propname: name of the property containing the string list
 * @index: index of the string to return (cannot be negative)
 * @outp: return location for the string
 *
 * Return:
 *   0 if found or -ve error value if not found
 */
int ofnode_read_string_index(ofnode node, const char *propname, int index,
			     const char **outp);

/**
 * ofnode_read_string_count() - find the number of strings in a string list
 *
 * @node: node to check
 * @property: name of the property containing the string list
 * Return:
 *   number of strings in the list, or -ve error value if not found
 */
int ofnode_read_string_count(ofnode node, const char *property);

/**
 * ofnode_read_string_list() - read a list of strings
 *
 * This produces a list of string pointers with each one pointing to a string
 * in the string list. If the property does not exist, it returns {NULL}.
 *
 * The data is allocated and the caller is reponsible for freeing the return
 * value (the list of string pointers). The strings themselves may not be
 * changed as they point directly into the devicetree property.
 *
 * @node: node to check
 * @property: name of the property containing the string list
 * @listp: returns an allocated, NULL-terminated list of strings if the return
 *	value is > 0, else is set to NULL
 * Return:
 * number of strings in list, 0 if none, -ENOMEM if out of memory,
 * -EINVAL if no such property, -EENODATA if property is empty
 */
int ofnode_read_string_list(ofnode node, const char *property,
			    const char ***listp);

/**
 * ofnode_parse_phandle_with_args() - Find a node pointed by phandle in a list
 *
 * This function is useful to parse lists of phandles and their arguments.
 * Returns 0 on success and fills out_args, on error returns appropriate
 * errno value.
 *
 * Caller is responsible to call of_node_put() on the returned out_args->np
 * pointer.
 *
 * Example:
 *
 * .. code-block::
 *
 *   phandle1: node1 {
 *       #list-cells = <2>;
 *   };
 *   phandle2: node2 {
 *       #list-cells = <1>;
 *   };
 *   node3 {
 *       list = <&phandle1 1 2 &phandle2 3>;
 *   };
 *
 * To get a device_node of the `node2' node you may call this:
 * ofnode_parse_phandle_with_args(node3, "list", "#list-cells", 0, 1, &args);
 *
 * @node:	device tree node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @cell_count: Cell count to use if @cells_name is NULL
 * @index:	index of a phandle to parse out
 * @out_args:	optional pointer to output arguments structure (will be filled)
 * Return:
 *   0 on success (with @out_args filled out if not NULL), -ENOENT if
 *   @list_name does not exist, -EINVAL if a phandle was not found,
 *   @cells_name could not be found, the arguments were truncated or there
 *   were too many arguments.
 */
int ofnode_parse_phandle_with_args(ofnode node, const char *list_name,
				   const char *cells_name, int cell_count,
				   int index,
				   struct ofnode_phandle_args *out_args);

/**
 * ofnode_count_phandle_with_args() - Count number of phandle in a list
 *
 * This function is useful to count phandles into a list.
 * Returns number of phandle on success, on error returns appropriate
 * errno value.
 *
 * @node:	device tree node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @cell_count: Cell count to use if @cells_name is NULL
 * Return:
 *   number of phandle on success, -ENOENT if @list_name does not exist,
 *   -EINVAL if a phandle was not found, @cells_name could not be found.
 */
int ofnode_count_phandle_with_args(ofnode node, const char *list_name,
				   const char *cells_name, int cell_count);

/**
 * ofnode_path() - find a node by full path
 *
 * This uses the control FDT.
 *
 * @path: Full path to node, e.g. "/bus/spi@1"
 * Return: reference to the node found. Use ofnode_valid() to check if it exists
 */
ofnode ofnode_path(const char *path);

/**
 * oftree_path() - find a node by full path from a root node
 *
 * @tree: Device tree to use
 * @path: Full path to node, e.g. "/bus/spi@1"
 * Return: reference to the node found. Use ofnode_valid() to check if it exists
 */
ofnode oftree_path(oftree tree, const char *path);

/**
 * oftree_root() - get the root node of a tree
 *
 * @tree: Device tree to use
 * Return: reference to the root node
 */
ofnode oftree_root(oftree tree);

/**
 * ofnode_read_chosen_prop() - get the value of a chosen property
 *
 * This looks for a property within the /chosen node and returns its value.
 *
 * This only works with the control FDT.
 *
 * @propname: Property name to look for
 * @sizep: Returns size of property, or  `FDT_ERR_...` error code if function
 *	returns NULL
 * Return: property value if found, else NULL
 */
const void *ofnode_read_chosen_prop(const char *propname, int *sizep);

/**
 * ofnode_read_chosen_string() - get the string value of a chosen property
 *
 * This looks for a property within the /chosen node and returns its value,
 * checking that it is a valid nul-terminated string
 *
 * This only works with the control FDT.
 *
 * @propname: Property name to look for
 * Return: string value if found, else NULL
 */
const char *ofnode_read_chosen_string(const char *propname);

/**
 * ofnode_get_chosen_node() - get a referenced node from the chosen node
 *
 * This looks up a named property in the chosen node and uses that as a path to
 * look up a code.
 *
 * This only works with the control FDT.
 *
 * @propname: Property name to look for
 * Return: the referenced node if present, else ofnode_null()
 */
ofnode ofnode_get_chosen_node(const char *propname);

/**
 * ofnode_read_baud() - get the baudrate from string value of chosen property
 *
 * This looks for stdout-path property within the /chosen node and parses its
 * value to return baudrate.
 *
 * This only works with the control FDT.
 *
 * Return: baudrate value if found, else -ve error code
 */
int ofnode_read_baud(void);

/**
 * ofnode_read_aliases_prop() - get the value of a aliases property
 *
 * This looks for a property within the /aliases node and returns its value
 *
 * @propname: Property name to look for
 * @sizep: Returns size of property, or `FDT_ERR_...` error code if function
 *	returns NULL
 * Return: property value if found, else NULL
 */
const void *ofnode_read_aliases_prop(const char *propname, int *sizep);

/**
 * ofnode_get_aliases_node() - get a referenced node from the aliases node
 *
 * This looks up a named property in the aliases node and uses that as a path to
 * look up a code.
 *
 * This only works with the control FDT.
 *
 * @propname: Property name to look for
 * Return: the referenced node if present, else ofnode_null()
 */
ofnode ofnode_get_aliases_node(const char *propname);

struct display_timing;
/**
 * ofnode_decode_display_timing() - decode display timings
 *
 * Decode display timings from the supplied 'display-timings' node.
 * See doc/device-tree-bindings/video/display-timing.txt for binding
 * information.
 *
 * @node:	'display-timing' node containing the timing subnodes
 * @index:	Index number to read (0=first timing subnode)
 * @config:	Place to put timings
 * Return: 0 if OK, -FDT_ERR_NOTFOUND if not found
 */
int ofnode_decode_display_timing(ofnode node, int index,
				 struct display_timing *config);

/**
 * ofnode_decode_panel_timing() - decode display timings
 *
 * Decode panel timings from the supplied 'panel-timings' node.
 *
 * @node:	'display-timing' node containing the timing subnodes
 * @config:	Place to put timings
 * Return: 0 if OK, -FDT_ERR_NOTFOUND if not found
 */
int ofnode_decode_panel_timing(ofnode node,
			       struct display_timing *config);

/**
 * ofnode_get_property() - get a pointer to the value of a node property
 *
 * @node: node to read
 * @propname: property to read
 * @lenp: place to put length on success
 * Return: pointer to property value, or NULL if not found or empty
 */
const void *ofnode_get_property(ofnode node, const char *propname, int *lenp);

/**
 * ofnode_has_property() - check if a node has a named property
 *
 * @node: node to read
 * @propname: property to read
 * Return: true if the property exists in the node, false if not
 */
bool ofnode_has_property(ofnode node, const char *propname);

/**
 * ofnode_first_property()- get the reference of the first property
 *
 * Get reference to the first property of the node, it is used to iterate
 * and read all the property with ofprop_get_property().
 *
 * @node: node to read
 * @prop: place to put argument reference
 * Return: 0 if OK, -ve on error. -FDT_ERR_NOTFOUND if not found
 */
int ofnode_first_property(ofnode node, struct ofprop *prop);

/**
 * ofnode_next_property() - get the reference of the next property
 *
 * Get reference to the next property of the node, it is used to iterate
 * and read all the property with ofprop_get_property().
 *
 * @prop: reference of current argument and place to put reference of next one
 * Return: 0 if OK, -ve on error. -FDT_ERR_NOTFOUND if not found
 */
int ofnode_next_property(struct ofprop *prop);

/**
 * ofnode_for_each_prop() - iterate over all properties of a node
 *
 * @prop:	struct ofprop
 * @node:	node (lvalue, ofnode)
 *
 * This is a wrapper around a for loop and is used like this::
 *
 *   ofnode node;
 *   struct ofprop prop;
 *
 *   ofnode_for_each_prop(prop, node) {
 *       ...use prop...
 *   }
 *
 * Note that this is implemented as a macro and @prop is used as
 * iterator in the loop. The parent variable can be a constant or even a
 * literal.
 */
#define ofnode_for_each_prop(prop, node) \
	for (ofnode_first_property(node, &prop); \
	     ofprop_valid(&prop); \
	     ofnode_next_property(&prop))

/**
 * ofprop_get_property() - get a pointer to the value of a property
 *
 * Get value for the property identified by the provided reference.
 *
 * @prop: reference on property
 * @propname: If non-NULL, place to property name on success,
 * @lenp: If non-NULL, place to put length on success, or error code on failure
 * Return: pointer to property, or NULL if not found
 */
const void *ofprop_get_property(const struct ofprop *prop,
				const char **propname, int *lenp);

/**
 * ofnode_get_addr_size() - get address and size from a property
 *
 * This does no address translation. It simply reads an property that contains
 * an address and a size value, one after the other.
 *
 * @node: node to read from
 * @propname: property to read
 * @sizep: place to put size value (on success)
 * Return: address value, or FDT_ADDR_T_NONE on error
 */
fdt_addr_t ofnode_get_addr_size(ofnode node, const char *propname,
				fdt_size_t *sizep);

/**
 * ofnode_read_u8_array_ptr() - find an 8-bit array
 *
 * Look up a property in a node and return a pointer to its contents as a
 * byte array of given length. The property must have at least enough data
 * for the array (count bytes). It may have more, but this will be ignored.
 * The data is not copied.
 *
 * @node:	node to examine
 * @propname:	name of property to find
 * @sz:		number of array elements
 * Return:
 * pointer to byte array if found, or NULL if the property is not found or
 * there is not enough data
 */
const uint8_t *ofnode_read_u8_array_ptr(ofnode node, const char *propname,
					size_t sz);

/**
 * ofnode_read_pci_addr() - look up a PCI address
 *
 * Look at an address property in a node and return the PCI address which
 * corresponds to the given type in the form of fdt_pci_addr.
 * The property must hold one fdt_pci_addr with a lengh.
 *
 * @node:	node to examine
 * @type:	pci address type (FDT_PCI_SPACE_xxx)
 * @propname:	name of property to find
 * @addr:	returns pci address in the form of fdt_pci_addr
 * @size:	if non-null, returns register-space size
 * Return:
 * 0 if ok, -ENOENT if the property did not exist, -EINVAL if the
 * format of the property was invalid, -ENXIO if the requested
 * address type was not found
 */
int ofnode_read_pci_addr(ofnode node, enum fdt_pci_space type,
			 const char *propname, struct fdt_pci_addr *addr,
			 fdt_size_t *size);

/**
 * ofnode_read_pci_vendev() - look up PCI vendor and device id
 *
 * Look at the compatible property of a device node that represents a PCI
 * device and extract pci vendor id and device id from it.
 *
 * @node:	node to examine
 * @vendor:	vendor id of the pci device
 * @device:	device id of the pci device
 * Return: 0 if ok, negative on error
 */
int ofnode_read_pci_vendev(ofnode node, u16 *vendor, u16 *device);

/**
 * ofnode_read_eth_phy_id() - look up eth phy vendor and device id
 *
 * Look at the compatible property of a device node that represents a eth phy
 * device and extract phy vendor id and device id from it.
 *
 * @node:	node to examine
 * @vendor:	vendor id of the eth phy device
 * @device:	device id of the eth phy device
 * Return:	 0 if ok, negative on error
 */
int ofnode_read_eth_phy_id(ofnode node, u16 *vendor, u16 *device);

/**
 * ofnode_read_addr_cells() - Get the number of address cells for a node
 *
 * This walks back up the tree to find the closest #address-cells property
 * which controls the given node.
 *
 * @node: Node to check
 * Return: number of address cells this node uses
 */
int ofnode_read_addr_cells(ofnode node);

/**
 * ofnode_read_size_cells() - Get the number of size cells for a node
 *
 * This walks back up the tree to find the closest #size-cells property
 * which controls the given node.
 *
 * @node: Node to check
 * Return: number of size cells this node uses
 */
int ofnode_read_size_cells(ofnode node);

/**
 * ofnode_read_simple_addr_cells() - Get the address cells property in a node
 *
 * This function matches fdt_address_cells().
 *
 * @node: Node to check
 * Return: value of #address-cells property in this node, or 2 if none
 */
int ofnode_read_simple_addr_cells(ofnode node);

/**
 * ofnode_read_simple_size_cells() - Get the size cells property in a node
 *
 * This function matches fdt_size_cells().
 *
 * @node: Node to check
 * Return: value of #size-cells property in this node, or 2 if none
 */
int ofnode_read_simple_size_cells(ofnode node);

/**
 * ofnode_pre_reloc() - check if a node should be bound before relocation
 *
 * Device tree nodes can be marked as needing-to-be-bound in the loader stages
 * via special device tree properties.
 *
 * Before relocation this function can be used to check if nodes are required
 * in either SPL or TPL stages.
 *
 * After relocation and jumping into the real U-Boot binary it is possible to
 * determine if a node was bound in one of SPL/TPL stages.
 *
 * There are 4 settings currently in use
 * - bootph-some-ram: U-Boot proper pre-relocation phase
 * - bootph-all: all phases
 * Existing platforms only use it to indicate nodes needed in
 * SPL. Should probably be replaced by bootph-pre-ram for new platforms.
 * - bootph-pre-ram: SPL phase
 * - bootph-pre-sram: TPL phase
 *
 * @node: node to check
 * Return: true if node should be or was bound, false otherwise
 */
bool ofnode_pre_reloc(ofnode node);

/**
 * ofnode_read_resource() - Read a resource from a node
 *
 * Read resource information from a node at the given index
 *
 * @node: Node to read from
 * @index: Index of resource to read (0 = first)
 * @res: Returns resource that was read, on success
 * Return: 0 if OK, -ve on error
 */
int ofnode_read_resource(ofnode node, uint index, struct resource *res);

/**
 * ofnode_read_resource_byname() - Read a resource from a node by name
 *
 * Read resource information from a node matching the given name. This uses a
 * 'reg-names' string list property with the names matching the associated
 * 'reg' property list.
 *
 * @node: Node to read from
 * @name: Name of resource to read
 * @res: Returns resource that was read, on success
 * Return: 0 if OK, -ve on error
 */
int ofnode_read_resource_byname(ofnode node, const char *name,
				struct resource *res);

/**
 * ofnode_by_compatible() - Find the next compatible node
 *
 * Find the next node after @from that is compatible with @compat
 *
 * @from: ofnode to start from (use ofnode_null() to start at the beginning)
 * @compat: Compatible string to match
 * Return: ofnode found, or ofnode_null() if none
 */
ofnode ofnode_by_compatible(ofnode from, const char *compat);

/**
 * ofnode_by_prop_value() - Find the next node with given property value
 *
 * Find the next node after @from that has a @propname with a value
 * @propval and a length @proplen.
 *
 * @from: ofnode to start from. Use ofnode_null() to start at the
 * beginning, or the return value from oftree_root() to start at the first
 * child of the root
 * @propname: property name to check
 * @propval: property value to search for
 * @proplen: length of the value in propval
 * Return: ofnode found, or ofnode_null() if none
 */
ofnode ofnode_by_prop_value(ofnode from, const char *propname,
			    const void *propval, int proplen);

/**
 * ofnode_for_each_subnode() - iterate over all subnodes of a parent
 *
 * @node:       child node (ofnode, lvalue)
 * @parent:     parent node (ofnode)
 *
 * This is a wrapper around a for loop and is used like so::
 *
 *   ofnode node;
 *   ofnode_for_each_subnode(node, parent) {
 *       Use node
 *       ...
 *   }
 *
 * Note that this is implemented as a macro and @node is used as
 * iterator in the loop. The parent variable can be a constant or even a
 * literal.
 */
#define ofnode_for_each_subnode(node, parent) \
	for (node = ofnode_first_subnode(parent); \
	     ofnode_valid(node); \
	     node = ofnode_next_subnode(node))

/**
 * ofnode_for_each_compatible_node() - iterate over all nodes with a given
 *				       compatible string
 *
 * @node:       child node (ofnode, lvalue)
 * @compat:     compatible string to match
 *
 * This is a wrapper around a for loop and is used like so::
 *
 *   ofnode node;
 *   ofnode_for_each_compatible_node(node, parent, compatible) {
 *      Use node
 *      ...
 *   }
 *
 * Note that this is implemented as a macro and @node is used as
 * iterator in the loop.
 */
#define ofnode_for_each_compatible_node(node, compat) \
	for (node = ofnode_by_compatible(ofnode_null(), compat); \
	     ofnode_valid(node); \
	     node = ofnode_by_compatible(node, compat))

/**
 * ofnode_get_child_count() - get the child count of a ofnode
 *
 * @parent: valid node to get its child count
 * Return: the number of subnodes
 */
int ofnode_get_child_count(ofnode parent);

/**
 * ofnode_translate_address() - Translate a device-tree address
 *
 * Translate an address from the device-tree into a CPU physical address. This
 * function walks up the tree and applies the various bus mappings along the
 * way.
 *
 * @node: Device tree node giving the context in which to translate the address
 * @in_addr: pointer to the address to translate
 * Return: the translated address; OF_BAD_ADDR on error
 */
u64 ofnode_translate_address(ofnode node, const fdt32_t *in_addr);

/**
 * ofnode_translate_dma_address() - Translate a device-tree DMA address
 *
 * Translate a DMA address from the device-tree into a CPU physical address.
 * This function walks up the tree and applies the various bus mappings along
 * the way.
 *
 * @node: Device tree node giving the context in which to translate the
 *        DMA address
 * @in_addr: pointer to the DMA address to translate
 * Return: the translated DMA address; OF_BAD_ADDR on error
 */
u64 ofnode_translate_dma_address(ofnode node, const fdt32_t *in_addr);

/**
 * ofnode_get_dma_range() - get dma-ranges for a specific DT node
 *
 * Get DMA ranges for a specifc node, this is useful to perform bus->cpu and
 * cpu->bus address translations
 *
 * @node: Device tree node
 * @cpu: Pointer to variable storing the range's cpu address
 * @bus: Pointer to variable storing the range's bus address
 * @size: Pointer to variable storing the range's size
 * Return: translated DMA address or OF_BAD_ADDR on error
 */
int ofnode_get_dma_range(ofnode node, phys_addr_t *cpu, dma_addr_t *bus,
			 u64 *size);

/**
 * ofnode_device_is_compatible() - check if the node is compatible with compat
 *
 * This allows to check whether the node is comaptible with the compat.
 *
 * @node:	Device tree node for which compatible needs to be verified.
 * @compat:	Compatible string which needs to verified in the given node.
 * Return: true if OK, false if the compatible is not found
 */
int ofnode_device_is_compatible(ofnode node, const char *compat);

/**
 * ofnode_write_prop() - Set a property of a ofnode
 *
 * Note that if @copy is false, the value passed to the function is *not*
 * allocated by the function itself, but must be allocated by the caller if
 * necessary. However it does allocate memory for the property struct and name.
 *
 * @node:	The node for whose property should be set
 * @propname:	The name of the property to set
 * @value:	The new value of the property (must be valid prior to calling
 *		the function)
 * @len:	The length of the new value of the property
 * @copy: true to allocate memory for the value. This only has any effect with
 *	live tree, since flat tree handles this automatically. It allows a
 *	node's value to be written to the tree, without requiring that the
 *	caller allocate it
 * Return: 0 if successful, -ve on error
 */
int ofnode_write_prop(ofnode node, const char *propname, const void *value,
		      int len, bool copy);

/**
 * ofnode_write_string() - Set a string property of a ofnode
 *
 * Note that the value passed to the function is *not* allocated by the
 * function itself, but must be allocated by the caller if necessary.
 *
 * @node:	The node for whose string property should be set
 * @propname:	The name of the string property to set
 * @value:	The new value of the string property (must be valid prior to
 *		calling the function)
 * Return: 0 if successful, -ve on error
 */
int ofnode_write_string(ofnode node, const char *propname, const char *value);

/**
 * ofnode_write_u32() - Set an integer property of an ofnode
 *
 * @node:	The node for whose string property should be set
 * @propname:	The name of the string property to set
 * @value:	The new value of the 32-bit integer property
 * Return: 0 if successful, -ve on error
 */
int ofnode_write_u32(ofnode node, const char *propname, u32 value);

/**
 * ofnode_write_u64() - Set an integer property of an ofnode
 *
 * @node:	The node for whose string property should be set
 * @propname:	The name of the string property to set
 * @value:	The new value of the 64-bit integer property
 * Return: 0 if successful, -ve on error
 */
int ofnode_write_u64(ofnode node, const char *propname, u64 value);

/**
 * ofnode_write_bool() - Set a boolean property of an ofnode
 *
 * This either adds or deleted a property with a zero-length value
 *
 * @node:	The node for whose string property should be set
 * @propname:	The name of the string property to set
 * @value:	The new value of the boolean property
 * Return: 0 if successful, -ve on error
 */
int ofnode_write_bool(ofnode node, const char *propname, bool value);

/**
 * ofnode_delete_prop() - Delete a property
 *
 * @node:	Node containing the property to delete
 * @propname:	Name of property to delete
 * Return: 0 if successful, -ve on error
 */
int ofnode_delete_prop(ofnode node, const char *propname);

/**
 * ofnode_set_enabled() - Enable or disable a device tree node given by its
 *			  ofnode
 *
 * This function effectively sets the node's "status" property to either "okay"
 * or "disable", hence making it available for driver model initialization or
 * not.
 *
 * @node:	The node to enable
 * @value:	Flag that tells the function to either disable or enable the
 *		node
 * Return: 0 if successful, -ve on error
 */
int ofnode_set_enabled(ofnode node, bool value);

/**
 * ofnode_get_phy_node() - Get PHY node for a MAC (if not fixed-link)
 *
 * This function parses PHY handle from the Ethernet controller's ofnode
 * (trying all possible PHY handle property names), and returns the PHY ofnode.
 *
 * Before this is used, ofnode_phy_is_fixed_link() should be checked first, and
 * if the result to that is true, this function should not be called.
 *
 * @eth_node:	ofnode belonging to the Ethernet controller
 * Return: ofnode of the PHY, if it exists, otherwise an invalid ofnode
 */
ofnode ofnode_get_phy_node(ofnode eth_node);

/**
 * ofnode_read_phy_mode() - Read PHY connection type from a MAC node
 *
 * This function parses the "phy-mode" / "phy-connection-type" property and
 * returns the corresponding PHY interface type.
 *
 * @mac_node:	ofnode containing the property
 * Return: one of PHY_INTERFACE_MODE_* constants, PHY_INTERFACE_MODE_NA on
 *	   error
 */
phy_interface_t ofnode_read_phy_mode(ofnode mac_node);

#if CONFIG_IS_ENABLED(DM)
/**
 * ofnode_conf_read_bool() - Read a boolean value from the U-Boot config
 *
 * This reads a property from the /config node of the devicetree.
 *
 * This only works with the control FDT.
 *
 * See doc/device-tree-bindings/config.txt for bindings
 *
 * @prop_name:	property name to look up
 * Return: true, if it exists, false if not
 */
bool ofnode_conf_read_bool(const char *prop_name);

/**
 * ofnode_conf_read_int() - Read an integer value from the U-Boot config
 *
 * This reads a property from the /config node of the devicetree.
 *
 * See doc/device-tree-bindings/config.txt for bindings
 *
 * @prop_name: property name to look up
 * @default_val: default value to return if the property is not found
 * Return: integer value, if found, or @default_val if not
 */
int ofnode_conf_read_int(const char *prop_name, int default_val);

/**
 * ofnode_conf_read_str() - Read a string value from the U-Boot config
 *
 * This reads a property from the /config node of the devicetree.
 *
 * This only works with the control FDT.
 *
 * See doc/device-tree-bindings/config.txt for bindings
 *
 * @prop_name: property name to look up
 * Return: string value, if found, or NULL if not
 */
const char *ofnode_conf_read_str(const char *prop_name);

/**
 * ofnode_read_bootscript_address() - Read bootscr-address or bootscr-ram-offset
 *
 * @bootscr_address: pointer to 64bit address where bootscr-address property value
 * is stored
 * @bootscr_offset:  pointer to 64bit offset address where bootscr-ram-offset
 * property value is stored
 *
 * This reads a bootscr-address or bootscr-ram-offset property from
 * the /options/u-boot/ node of the devicetree. bootscr-address holds the full
 * address of the boot script file. bootscr-ram-offset holds the boot script
 * file offset from the start of the ram base address. When bootscr-address is
 * defined, bootscr-ram-offset property is ignored.
 *
 * This only works with the control FDT.
 *
 * Return: 0 if OK, -EINVAL if property is not found.
 */
int ofnode_read_bootscript_address(u64 *bootscr_address, u64 *bootscr_offset);

/**
 * ofnode_read_bootscript_flash() - Read bootscr-flash-offset/size
 *
 * @bootscr_flash_offset: pointer to 64bit offset where bootscr-flash-offset
 * property value is stored
 * @bootscr_flash_size: pointer to 64bit size where bootscr-flash-size property
 * value is stored
 *
 * This reads a bootscr-flash-offset and bootscr-flash-size properties from
 * the /options/u-boot/ node of the devicetree. bootscr-flash-offset holds
 * the offset of the boot script file from start of flash. bootscr-flash-size
 * holds the boot script size in flash. When bootscr-flash-size is not defined,
 * bootscr-flash-offset property is cleaned.
 *
 * This only works with the control FDT.
 *
 * Return: 0 if OK, -EINVAL if property is not found or incorrect.
 */
int ofnode_read_bootscript_flash(u64 *bootscr_flash_offset,
				 u64 *bootscr_flash_size);

#else /* CONFIG_DM */
static inline bool ofnode_conf_read_bool(const char *prop_name)
{
	return false;
}

static inline int ofnode_conf_read_int(const char *prop_name, int default_val)
{
	return default_val;
}

static inline const char *ofnode_conf_read_str(const char *prop_name)
{
	return NULL;
}

static inline int ofnode_read_bootscript_address(u64 *bootscr_address, u64 *bootscr_offset)
{
	return -EINVAL;
}

static inline int ofnode_read_bootscript_flash(u64 *bootscr_flash_offset,
					       u64 *bootscr_flash_size)
{
	return -EINVAL;
}

#endif /* CONFIG_DM */

/**
 * of_add_subnode() - add a new subnode to a node
 *
 * @parent:	parent node to add to
 * @name:	name of subnode
 * @nodep:	returns pointer to new subnode (valid if the function returns 0
 *	or -EEXIST)
 * Returns 0 if OK, -EEXIST if already exists, -ENOMEM if out of memory, other
 * -ve on other error
 */
int ofnode_add_subnode(ofnode parent, const char *name, ofnode *nodep);

/**
 * ofnode_copy_props() - copy all properties from one node to another
 *
 * Makes a copy of all properties from the source node to the destination node.
 * Existing properties in the destination node remain unchanged, except that
 * any with the same name are overwritten, including changing the size of the
 * property.
 *
 * For livetree, properties are copied / allocated, so the source tree does not
 * need to be present afterwards.
 *
 * @dst: Destination node to write properties to
 * @src: Source node to read properties from
 */
int ofnode_copy_props(ofnode dst, ofnode src);

/**
 * ofnode_copy_node() - Copy a node to another place
 *
 * If a node with this name already exists in dst_parent, this returns an
 * .error
 *
 * @dst_parent: Parent of the newly copied node
 * @name: Name to give the new node
 * @src: Source node to copy
 * @nodep: Returns the new node, or the existing node if there is one
 * Return: 0 if OK, -EEXIST if dst_parent already has a node with this parent
 */
int ofnode_copy_node(ofnode dst_parent, const char *name, ofnode src,
		     ofnode *nodep);

/**
 * ofnode_delete() - Delete a node
 *
 * Delete a node from the tree
 *
 * @nodep: Pointer to node to delete (set to ofnode_null() on success)
 * Return: 0 if OK, -ENOENT if the node does not exist, -EPERM if it is the root
 * node (wWhich cannot be removed), -EFAULT if the tree is broken (to_remove is
 * not a child of its parent),
 *
 */
int ofnode_delete(ofnode *nodep);

#endif
