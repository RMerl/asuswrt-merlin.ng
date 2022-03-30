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

/* Enable checks to protect against invalid calls */
#undef OF_CHECKS

struct resource;

/**
 * ofnode - reference to a device tree node
 *
 * This union can hold either a straightforward pointer to a struct device_node
 * in the live device tree, or an offset within the flat device tree. In the
 * latter case, the pointer value is just the integer offset within the flat DT.
 *
 * Thus we can reference nodes in both the live tree (once available) and the
 * flat tree (until then). Functions are available to translate between an
 * ofnode and either an offset or a struct device_node *.
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
 * For now these points use constant types, since we don't allow writing
 * the DT.
 *
 * @np: Pointer to device node, used for live tree
 * @of_offset: Pointer into flat device tree, used for flat tree. Note that this
 *	is not a really a pointer to a node: it is an offset value. See above.
 */
typedef union ofnode_union {
	const struct device_node *np;	/* will be used for future live tree */
	long of_offset;
} ofnode;

struct ofnode_phandle_args {
	ofnode node;
	int args_count;
	uint32_t args[OF_MAX_PHANDLE_ARGS];
};

/**
 * _ofnode_to_np() - convert an ofnode to a live DT node pointer
 *
 * This cannot be called if the reference contains an offset.
 *
 * @node: Reference containing struct device_node * (possibly invalid)
 * @return pointer to device node (can be NULL)
 */
static inline const struct device_node *ofnode_to_np(ofnode node)
{
#ifdef OF_CHECKS
	if (!of_live_active())
		return NULL;
#endif
	return node.np;
}

/**
 * ofnode_to_offset() - convert an ofnode to a flat DT offset
 *
 * This cannot be called if the reference contains a node pointer.
 *
 * @node: Reference containing offset (possibly invalid)
 * @return DT offset (can be -1)
 */
static inline int ofnode_to_offset(ofnode node)
{
#ifdef OF_CHECKS
	if (of_live_active())
		return -1;
#endif
	return node.of_offset;
}

/**
 * ofnode_valid() - check if an ofnode is valid
 *
 * @return true if the reference contains a valid ofnode, false if it is NULL
 */
static inline bool ofnode_valid(ofnode node)
{
	if (of_live_active())
		return node.np != NULL;
	else
		return node.of_offset != -1;
}

/**
 * offset_to_ofnode() - convert a DT offset to an ofnode
 *
 * @of_offset: DT offset (either valid, or -1)
 * @return reference to the associated DT offset
 */
static inline ofnode offset_to_ofnode(int of_offset)
{
	ofnode node;

	if (of_live_active())
		node.np = NULL;
	else
		node.of_offset = of_offset;

	return node;
}

/**
 * np_to_ofnode() - convert a node pointer to an ofnode
 *
 * @np: Live node pointer (can be NULL)
 * @return reference to the associated node pointer
 */
static inline ofnode np_to_ofnode(const struct device_node *np)
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
 * @return true if the reference is a live node pointer, false if it is a DT
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
	       (of_live_active() ? _ofnode_to_np(node)
				  : _ofnode_to_np(node)));
#endif
	return of_live_active() && ofnode_valid(node);
}

/**
 * ofnode_equal() - check if two references are equal
 *
 * @return true if equal, else false
 */
static inline bool ofnode_equal(ofnode ref1, ofnode ref2)
{
	/* We only need to compare the contents */
	return ref1.of_offset == ref2.of_offset;
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

/**
 * ofnode_read_u32() - Read a 32-bit integer from a property
 *
 * @ref:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * @return 0 if OK, -ve on error
 */
int ofnode_read_u32(ofnode node, const char *propname, u32 *outp);

/**
 * ofnode_read_s32() - Read a 32-bit integer from a property
 *
 * @ref:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * @return 0 if OK, -ve on error
 */
static inline int ofnode_read_s32(ofnode node, const char *propname,
				  s32 *out_value)
{
	return ofnode_read_u32(node, propname, (u32 *)out_value);
}

/**
 * ofnode_read_u32_default() - Read a 32-bit integer from a property
 *
 * @ref:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * @return property value, or @def if not found
 */
u32 ofnode_read_u32_default(ofnode ref, const char *propname, u32 def);

/**
 * ofnode_read_s32_default() - Read a 32-bit integer from a property
 *
 * @ref:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * @return property value, or @def if not found
 */
int ofnode_read_s32_default(ofnode node, const char *propname, s32 def);

/**
 * ofnode_read_u64() - Read a 64-bit integer from a property
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * @return 0 if OK, -ve on error
 */
int ofnode_read_u64(ofnode node, const char *propname, u64 *outp);

/**
 * ofnode_read_u64_default() - Read a 64-bit integer from a property
 *
 * @ref:	valid node reference to read property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * @return property value, or @def if not found
 */
int ofnode_read_u64_default(ofnode node, const char *propname, u64 def);

/**
 * ofnode_read_string() - Read a string from a property
 *
 * @ref:	valid node reference to read property from
 * @propname:	name of the property to read
 * @return string from property value, or NULL if there is no such property
 */
const char *ofnode_read_string(ofnode node, const char *propname);

/**
 * ofnode_read_u32_array() - Find and read an array of 32 bit integers
 *
 * @node:	valid node reference to read property from
 * @propname:	name of the property to read
 * @out_values:	pointer to return value, modified only if return value is 0
 * @sz:		number of array elements to read
 * @return 0 if OK, -ve on error
 *
 * Search for a property in a device node and read 32-bit value(s) from
 * it. Returns 0 on success, -EINVAL if the property does not exist,
 * -ENODATA if property does not have a value, and -EOVERFLOW if the
 * property data isn't large enough.
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
 * @return true if property is present (meaning true), false if not present
 */
bool ofnode_read_bool(ofnode node, const char *propname);

/**
 * ofnode_find_subnode() - find a named subnode of a parent node
 *
 * @node:	valid reference to parent node
 * @subnode_name: name of subnode to find
 * @return reference to subnode (which can be invalid if there is no such
 * subnode)
 */
ofnode ofnode_find_subnode(ofnode node, const char *subnode_name);

/**
 * ofnode_first_subnode() - find the first subnode of a parent node
 *
 * @node:	valid reference to a valid parent node
 * @return reference to the first subnode (which can be invalid if the parent
 * node has no subnodes)
 */
ofnode ofnode_first_subnode(ofnode node);

/**
 * ofnode_next_subnode() - find the next sibling of a subnode
 *
 * @node:	valid reference to previous node (sibling)
 * @return reference to the next subnode (which can be invalid if the node
 * has no more siblings)
 */
ofnode ofnode_next_subnode(ofnode node);

/**
 * ofnode_get_parent() - get the ofnode's parent (enclosing ofnode)
 *
 * @node: valid node to look up
 * @return ofnode reference of the parent node
 */
ofnode ofnode_get_parent(ofnode node);

/**
 * ofnode_get_name() - get the name of a node
 *
 * @node: valid node to look up
 * @return name of node
 */
const char *ofnode_get_name(ofnode node);

/**
 * ofnode_get_by_phandle() - get ofnode from phandle
 *
 * @phandle:	phandle to look up
 * @return ofnode reference to the phandle
 */
ofnode ofnode_get_by_phandle(uint phandle);

/**
 * ofnode_read_size() - read the size of a property
 *
 * @node: node to check
 * @propname: property to check
 * @return size of property if present, or -EINVAL if not
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
 * @return address, or FDT_ADDR_T_NONE if not present or invalid
 */
phys_addr_t ofnode_get_addr_size_index(ofnode node, int index,
				       fdt_size_t *size);

/**
 * ofnode_get_addr_index() - get an address from a node
 *
 * This reads the register address from a node
 *
 * @node: node to read from
 * @index: Index of address to read (0 for first)
 * @return address, or FDT_ADDR_T_NONE if not present or invalid
 */
phys_addr_t ofnode_get_addr_index(ofnode node, int index);

/**
 * ofnode_get_addr() - get an address from a node
 *
 * This reads the register address from a node
 *
 * @node: node to read from
 * @return address, or FDT_ADDR_T_NONE if not present or invalid
 */
phys_addr_t ofnode_get_addr(ofnode node);

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
 * @return:
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
 * @index: index of the string to return
 * @lenp: return location for the string length or an error code on failure
 *
 * @return:
 *   length of string, if found or -ve error value if not found
 */
int ofnode_read_string_index(ofnode node, const char *propname, int index,
			     const char **outp);

/**
 * ofnode_read_string_count() - find the number of strings in a string list
 *
 * @node: node to check
 * @propname: name of the property containing the string list
 * @return:
 *   number of strings in the list, or -ve error value if not found
 */
int ofnode_read_string_count(ofnode node, const char *property);

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
 * phandle1: node1 {
 *	#list-cells = <2>;
 * }
 *
 * phandle2: node2 {
 *	#list-cells = <1>;
 * }
 *
 * node3 {
 *	list = <&phandle1 1 2 &phandle2 3>;
 * }
 *
 * To get a device_node of the `node2' node you may call this:
 * ofnode_parse_phandle_with_args(node3, "list", "#list-cells", 0, 1, &args);
 *
 * @node:	device tree node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @cells_count: Cell count to use if @cells_name is NULL
 * @index:	index of a phandle to parse out
 * @out_args:	optional pointer to output arguments structure (will be filled)
 * @return 0 on success (with @out_args filled out if not NULL), -ENOENT if
 *	@list_name does not exist, -EINVAL if a phandle was not found,
 *	@cells_name could not be found, the arguments were truncated or there
 *	were too many arguments.
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
 * @return number of phandle on success, -ENOENT if @list_name does not
 *      exist, -EINVAL if a phandle was not found, @cells_name could not
 *      be found.
 */
int ofnode_count_phandle_with_args(ofnode node, const char *list_name,
				   const char *cells_name);

/**
 * ofnode_path() - find a node by full path
 *
 * @path: Full path to node, e.g. "/bus/spi@1"
 * @return reference to the node found. Use ofnode_valid() to check if it exists
 */
ofnode ofnode_path(const char *path);

/**
 * ofnode_get_chosen_prop() - get the value of a chosen property
 *
 * This looks for a property within the /chosen node and returns its value
 *
 * @propname: Property name to look for
 * @return property value if found, else NULL
 */
const char *ofnode_get_chosen_prop(const char *propname);

/**
 * ofnode_get_chosen_node() - get the chosen node
 *
 * @return the chosen node if present, else ofnode_null()
 */
ofnode ofnode_get_chosen_node(const char *name);

struct display_timing;
/**
 * ofnode_decode_display_timing() - decode display timings
 *
 * Decode display timings from the supplied 'display-timings' node.
 * See doc/device-tree-bindings/video/display-timing.txt for binding
 * information.
 *
 * @node	'display-timing' node containing the timing subnodes
 * @index	Index number to read (0=first timing subnode)
 * @config	Place to put timings
 * @return 0 if OK, -FDT_ERR_NOTFOUND if not found
 */
int ofnode_decode_display_timing(ofnode node, int index,
				 struct display_timing *config);

/**
 * ofnode_get_property()- - get a pointer to the value of a node property
 *
 * @node: node to read
 * @propname: property to read
 * @lenp: place to put length on success
 * @return pointer to property, or NULL if not found
 */
const void *ofnode_get_property(ofnode node, const char *propname, int *lenp);

/**
 * ofnode_is_available() - check if a node is marked available
 *
 * @node: node to check
 * @return true if node's 'status' property is "okay" (or is missing)
 */
bool ofnode_is_available(ofnode node);

/**
 * ofnode_get_addr_size() - get address and size from a property
 *
 * This does no address translation. It simply reads an property that contains
 * an address and a size value, one after the other.
 *
 * @node: node to read from
 * @propname: property to read
 * @sizep: place to put size value (on success)
 * @return address value, or FDT_ADDR_T_NONE on error
 */
phys_addr_t ofnode_get_addr_size(ofnode node, const char *propname,
				 phys_size_t *sizep);

/**
 * ofnode_read_u8_array_ptr() - find an 8-bit array
 *
 * Look up a property in a node and return a pointer to its contents as a
 * byte array of given length. The property must have at least enough data
 * for the array (count bytes). It may have more, but this will be ignored.
 * The data is not copied.
 *
 * @node	node to examine
 * @propname	name of property to find
 * @sz		number of array elements
 * @return pointer to byte array if found, or NULL if the property is not
 *		found or there is not enough data
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
 * @node	node to examine
 * @type	pci address type (FDT_PCI_SPACE_xxx)
 * @propname	name of property to find
 * @addr	returns pci address in the form of fdt_pci_addr
 * @return 0 if ok, -ENOENT if the property did not exist, -EINVAL if the
 *		format of the property was invalid, -ENXIO if the requested
 *		address type was not found
 */
int ofnode_read_pci_addr(ofnode node, enum fdt_pci_space type,
			 const char *propname, struct fdt_pci_addr *addr);

/**
 * ofnode_read_pci_vendev() - look up PCI vendor and device id
 *
 * Look at the compatible property of a device node that represents a PCI
 * device and extract pci vendor id and device id from it.
 *
 * @param node		node to examine
 * @param vendor	vendor id of the pci device
 * @param device	device id of the pci device
 * @return 0 if ok, negative on error
 */
int ofnode_read_pci_vendev(ofnode node, u16 *vendor, u16 *device);

/**
 * ofnode_read_addr_cells() - Get the number of address cells for a node
 *
 * This walks back up the tree to find the closest #address-cells property
 * which controls the given node.
 *
 * @node: Node to check
 * @return number of address cells this node uses
 */
int ofnode_read_addr_cells(ofnode node);

/**
 * ofnode_read_size_cells() - Get the number of size cells for a node
 *
 * This walks back up the tree to find the closest #size-cells property
 * which controls the given node.
 *
 * @node: Node to check
 * @return number of size cells this node uses
 */
int ofnode_read_size_cells(ofnode node);

/**
 * ofnode_read_simple_addr_cells() - Get the address cells property in a node
 *
 * This function matches fdt_address_cells().
 *
 * @np: Node pointer to check
 * @return value of #address-cells property in this node, or 2 if none
 */
int ofnode_read_simple_addr_cells(ofnode node);

/**
 * ofnode_read_simple_size_cells() - Get the size cells property in a node
 *
 * This function matches fdt_size_cells().
 *
 * @np: Node pointer to check
 * @return value of #size-cells property in this node, or 2 if none
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
 * There are 3 settings currently in use
 * -
 * - u-boot,dm-pre-reloc: legacy and indicates any of TPL or SPL
 *   Existing platforms only use it to indicate nodes needed in
 *   SPL. Should probably be replaced by u-boot,dm-spl for
 *   new platforms.
 *
 * @node: node to check
 * @return true if node is needed in SPL/TL, false otherwise
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
 * @return 0 if OK, -ve on error
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
 * @return 0 if OK, -ve on error
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
 * @return ofnode found, or ofnode_null() if none
 */
ofnode ofnode_by_compatible(ofnode from, const char *compat);

/**
 * ofnode_by_prop_value() - Find the next node with given property value
 *
 * Find the next node after @from that has a @propname with a value
 * @propval and a length @proplen.
 *
 * @from: ofnode to start from (use ofnode_null() to start at the
 * beginning) @propname: property name to check @propval: property value to
 * search for @proplen: length of the value in propval @return ofnode
 * found, or ofnode_null() if none
 */
ofnode ofnode_by_prop_value(ofnode from, const char *propname,
			    const void *propval, int proplen);

/**
 * ofnode_for_each_subnode() - iterate over all subnodes of a parent
 *
 * @node:       child node (ofnode, lvalue)
 * @parent:     parent node (ofnode)
 *
 * This is a wrapper around a for loop and is used like so:
 *
 *	ofnode node;
 *
 *	ofnode_for_each_subnode(node, parent) {
 *		Use node
 *		...
 *	}
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
 * ofnode_translate_address() - Tranlate a device-tree address
 *
 * Translate an address from the device-tree into a CPU physical address. This
 * function walks up the tree and applies the various bus mappings along the
 * way.
 *
 * @ofnode: Device tree node giving the context in which to translate the
 *          address
 * @in_addr: pointer to the address to translate
 * @return the translated address; OF_BAD_ADDR on error
 */
u64 ofnode_translate_address(ofnode node, const fdt32_t *in_addr);

/**
 * ofnode_device_is_compatible() - check if the node is compatible with compat
 *
 * This allows to check whether the node is comaptible with the compat.
 *
 * @node:	Device tree node for which compatible needs to be verified.
 * @compat:	Compatible string which needs to verified in the given node.
 * @return true if OK, false if the compatible is not found
 */
int ofnode_device_is_compatible(ofnode node, const char *compat);

/**
 * ofnode_write_prop() - Set a property of a ofnode
 *
 * Note that the value passed to the function is *not* allocated by the
 * function itself, but must be allocated by the caller if necessary.
 *
 * @node:	The node for whose property should be set
 * @propname:	The name of the property to set
 * @len:	The length of the new value of the property
 * @value:	The new value of the property (must be valid prior to calling
 *		the function)
 * @return 0 if successful, -ve on error
 */
int ofnode_write_prop(ofnode node, const char *propname, int len,
		      const void *value);

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
 * @return 0 if successful, -ve on error
 */
int ofnode_write_string(ofnode node, const char *propname, const char *value);

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
 * @return 0 if successful, -ve on error
 */
int ofnode_set_enabled(ofnode node, bool value);

#endif
