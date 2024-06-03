/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Function to read values from the device tree node attached to a udevice.
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _DM_READ_H
#define _DM_READ_H

#include <dm/fdtaddr.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>

struct resource;

#if CONFIG_IS_ENABLED(OF_LIVE)
static inline const struct device_node *dev_np(struct udevice *dev)
{
	return ofnode_to_np(dev->node);
}
#else
static inline const struct device_node *dev_np(struct udevice *dev)
{
	return NULL;
}
#endif

/**
 * dev_ofnode() - get the DT node reference associated with a udevice
 *
 * @dev:	device to check
 * @return reference of the the device's DT node
 */
static inline ofnode dev_ofnode(struct udevice *dev)
{
	return dev->node;
}

static inline bool dev_of_valid(struct udevice *dev)
{
	return ofnode_valid(dev_ofnode(dev));
}

#ifndef CONFIG_DM_DEV_READ_INLINE
/**
 * dev_read_u32() - read a 32-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * @return 0 if OK, -ve on error
 */
int dev_read_u32(struct udevice *dev, const char *propname, u32 *outp);

/**
 * dev_read_u32_default() - read a 32-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * @return property value, or @def if not found
 */
int dev_read_u32_default(struct udevice *dev, const char *propname, int def);

/**
 * dev_read_s32() - read a signed 32-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * @return 0 if OK, -ve on error
 */
int dev_read_s32(struct udevice *dev, const char *propname, s32 *outp);

/**
 * dev_read_s32_default() - read a signed 32-bit int from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * @return property value, or @def if not found
 */
int dev_read_s32_default(struct udevice *dev, const char *propname, int def);

/**
 * dev_read_u32u() - read a 32-bit integer from a device's DT property
 *
 * This version uses a standard uint type.
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * @return 0 if OK, -ve on error
 */
int dev_read_u32u(struct udevice *dev, const char *propname, uint *outp);

/**
 * dev_read_string() - Read a string from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read
 * @return string from property value, or NULL if there is no such property
 */
const char *dev_read_string(struct udevice *dev, const char *propname);

/**
 * dev_read_bool() - read a boolean value from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of property to read
 * @return true if property is present (meaning true), false if not present
 */
bool dev_read_bool(struct udevice *dev, const char *propname);

/**
 * dev_read_subnode() - find a named subnode of a device
 *
 * @dev:	device whose DT node contains the subnode
 * @subnode_name: name of subnode to find
 * @return reference to subnode (which can be invalid if there is no such
 * subnode)
 */
ofnode dev_read_subnode(struct udevice *dev, const char *subbnode_name);

/**
 * dev_read_size() - read the size of a property
 *
 * @dev: device to check
 * @propname: property to check
 * @return size of property if present, or -EINVAL if not
 */
int dev_read_size(struct udevice *dev, const char *propname);

/**
 * dev_read_addr_index() - Get the indexed reg property of a device
 *
 * @dev: Device to read from
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * @return address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr_index(struct udevice *dev, int index);

/**
 * dev_remap_addr_index() - Get the indexed reg property of a device
 *                               as a memory-mapped I/O pointer
 *
 * @dev: Device to read from
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * @return pointer or NULL if not found
 */
void *dev_remap_addr_index(struct udevice *dev, int index);

/**
 * dev_read_addr_name() - Get the reg property of a device, indexed by name
 *
 * @dev: Device to read from
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @index
 *	  indicates the value to search for in 'reg-names'.
 *
 * @return address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr_name(struct udevice *dev, const char* name);

/**
 * dev_remap_addr_name() - Get the reg property of a device, indexed by name,
 *                         as a memory-mapped I/O pointer
 *
 * @dev: Device to read from
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @index
 *	  indicates the value to search for in 'reg-names'.
 *
 * @return pointer or NULL if not found
 */
void *dev_remap_addr_name(struct udevice *dev, const char* name);

/**
 * dev_read_addr() - Get the reg property of a device
 *
 * @dev: Device to read from
 *
 * @return address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr(struct udevice *dev);

/**
 * dev_read_addr_ptr() - Get the reg property of a device
 *                       as a pointer
 *
 * @dev: Device to read from
 *
 * @return pointer or NULL if not found
 */
void *dev_read_addr_ptr(struct udevice *dev);

/**
 * dev_remap_addr() - Get the reg property of a device as a
 *                         memory-mapped I/O pointer
 *
 * @dev: Device to read from
 *
 * @return pointer or NULL if not found
 */
void *dev_remap_addr(struct udevice *dev);

/**
 * dev_read_addr_size() - get address and size from a device property
 *
 * This does no address translation. It simply reads an property that contains
 * an address and a size value, one after the other.
 *
 * @dev: Device to read from
 * @propname: property to read
 * @sizep: place to put size value (on success)
 * @return address value, or FDT_ADDR_T_NONE on error
 */
fdt_addr_t dev_read_addr_size(struct udevice *dev, const char *propname,
				fdt_size_t *sizep);

/**
 * dev_read_name() - get the name of a device's node
 *
 * @node: valid node to look up
 * @return name of node
 */
const char *dev_read_name(struct udevice *dev);

/**
 * dev_read_stringlist_search() - find string in a string list and return index
 *
 * Note that it is possible for this function to succeed on property values
 * that are not NUL-terminated. That's because the function will stop after
 * finding the first occurrence of @string. This can for example happen with
 * small-valued cell properties, such as #address-cells, when searching for
 * the empty string.
 *
 * @dev: device to check
 * @propname: name of the property containing the string list
 * @string: string to look up in the string list
 *
 * @return:
 *   the index of the string in the list of strings
 *   -ENODATA if the property is not found
 *   -EINVAL on some other error
 */
int dev_read_stringlist_search(struct udevice *dev, const char *property,
			  const char *string);

/**
 * dev_read_string_index() - obtain an indexed string from a string list
 *
 * @dev: device to examine
 * @propname: name of the property containing the string list
 * @index: index of the string to return
 * @out: return location for the string
 *
 * @return:
 *   length of string, if found or -ve error value if not found
 */
int dev_read_string_index(struct udevice *dev, const char *propname, int index,
			  const char **outp);

/**
 * dev_read_string_count() - find the number of strings in a string list
 *
 * @dev: device to examine
 * @propname: name of the property containing the string list
 * @return:
 *   number of strings in the list, or -ve error value if not found
 */
int dev_read_string_count(struct udevice *dev, const char *propname);
/**
 * dev_read_phandle_with_args() - Find a node pointed by phandle in a list
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
 * dev_read_phandle_with_args(dev, "list", "#list-cells", 0, 1, &args);
 *
 * @dev:	device whose node containing a list
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
int dev_read_phandle_with_args(struct udevice *dev, const char *list_name,
				const char *cells_name, int cell_count,
				int index,
				struct ofnode_phandle_args *out_args);

/**
 * dev_count_phandle_with_args() - Return phandle number in a list
 *
 * This function is usefull to get phandle number contained in a property list.
 * For example, this allows to allocate the right amount of memory to keep
 * clock's reference contained into the "clocks" property.
 *
 *
 * @dev:	device whose node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @Returns number of phandle found on success, on error returns appropriate
 * errno value.
 */

int dev_count_phandle_with_args(struct udevice *dev, const char *list_name,
				const char *cells_name);

/**
 * dev_read_addr_cells() - Get the number of address cells for a device's node
 *
 * This walks back up the tree to find the closest #address-cells property
 * which controls the given node.
 *
 * @dev: device to check
 * @return number of address cells this node uses
 */
int dev_read_addr_cells(struct udevice *dev);

/**
 * dev_read_size_cells() - Get the number of size cells for a device's node
 *
 * This walks back up the tree to find the closest #size-cells property
 * which controls the given node.
 *
 * @dev: device to check
 * @return number of size cells this node uses
 */
int dev_read_size_cells(struct udevice *dev);

/**
 * dev_read_addr_cells() - Get the address cells property in a node
 *
 * This function matches fdt_address_cells().
 *
 * @dev: device to check
 * @return number of address cells this node uses
 */
int dev_read_simple_addr_cells(struct udevice *dev);

/**
 * dev_read_size_cells() - Get the size cells property in a node
 *
 * This function matches fdt_size_cells().
 *
 * @dev: device to check
 * @return number of size cells this node uses
 */
int dev_read_simple_size_cells(struct udevice *dev);

/**
 * dev_read_phandle() - Get the phandle from a device
 *
 * @dev: device to check
 * @return phandle (1 or greater), or 0 if no phandle or other error
 */
int dev_read_phandle(struct udevice *dev);

/**
 * dev_read_prop()- - read a property from a device's node
 *
 * @dev: device to check
 * @propname: property to read
 * @lenp: place to put length on success
 * @return pointer to property, or NULL if not found
 */
const void *dev_read_prop(struct udevice *dev, const char *propname, int *lenp);

/**
 * dev_read_alias_seq() - Get the alias sequence number of a node
 *
 * This works out whether a node is pointed to by an alias, and if so, the
 * sequence number of that alias. Aliases are of the form <base><num> where
 * <num> is the sequence number. For example spi2 would be sequence number 2.
 *
 * @dev: device to look up
 * @devnump: set to the sequence number if one is found
 * @return 0 if a sequence was found, -ve if not
 */
int dev_read_alias_seq(struct udevice *dev, int *devnump);

/**
 * dev_read_u32_array() - Find and read an array of 32 bit integers
 *
 * Search for a property in a device node and read 32-bit value(s) from
 * it.
 *
 * The out_values is modified only if a valid u32 value can be decoded.
 *
 * @dev: device to look up
 * @propname:	name of the property to read
 * @out_values:	pointer to return value, modified only if return value is 0
 * @sz:		number of array elements to read
 * @return 0 on success, -EINVAL if the property does not exist, -ENODATA if
 * property does not have a value, and -EOVERFLOW if the property data isn't
 * large enough.
 */
int dev_read_u32_array(struct udevice *dev, const char *propname,
		       u32 *out_values, size_t sz);

/**
 * dev_read_first_subnode() - find the first subnode of a device's node
 *
 * @dev: device to look up
 * @return reference to the first subnode (which can be invalid if the device's
 * node has no subnodes)
 */
ofnode dev_read_first_subnode(struct udevice *dev);

/**
 * ofnode_next_subnode() - find the next sibling of a subnode
 *
 * @node:	valid reference to previous node (sibling)
 * @return reference to the next subnode (which can be invalid if the node
 * has no more siblings)
 */
ofnode dev_read_next_subnode(ofnode node);

/**
 * dev_read_u8_array_ptr() - find an 8-bit array
 *
 * Look up a device's node property and return a pointer to its contents as a
 * byte array of given length. The property must have at least enough data
 * for the array (count bytes). It may have more, but this will be ignored.
 * The data is not copied.
 *
 * @dev: device to look up
 * @propname: name of property to find
 * @sz: number of array elements
 * @return pointer to byte array if found, or NULL if the property is not
 *		found or there is not enough data
 */
const uint8_t *dev_read_u8_array_ptr(struct udevice *dev, const char *propname,
				     size_t sz);

/**
 * dev_read_enabled() - check whether a node is enabled
 *
 * This looks for a 'status' property. If this exists, then returns 1 if
 * the status is 'ok' and 0 otherwise. If there is no status property,
 * it returns 1 on the assumption that anything mentioned should be enabled
 * by default.
 *
 * @dev: device to examine
 * @return integer value 0 (not enabled) or 1 (enabled)
 */
int dev_read_enabled(struct udevice *dev);

/**
 * dev_read_resource() - obtain an indexed resource from a device.
 *
 * @dev: device to examine
 * @index index of the resource to retrieve (0 = first)
 * @res returns the resource
 * @return 0 if ok, negative on error
 */
int dev_read_resource(struct udevice *dev, uint index, struct resource *res);

/**
 * dev_read_resource_byname() - obtain a named resource from a device.
 *
 * @dev: device to examine
 * @name: name of the resource to retrieve
 * @res: returns the resource
 * @return 0 if ok, negative on error
 */
int dev_read_resource_byname(struct udevice *dev, const char *name,
			     struct resource *res);

/**
 * dev_translate_address() - Tranlate a device-tree address
 *
 * Translate an address from the device-tree into a CPU physical address.  This
 * function walks up the tree and applies the various bus mappings along the
 * way.
 *
 * @dev: device giving the context in which to translate the address
 * @in_addr: pointer to the address to translate
 * @return the translated address; OF_BAD_ADDR on error
 */
u64 dev_translate_address(struct udevice *dev, const fdt32_t *in_addr);

/**
 * dev_read_alias_highest_id - Get highest alias id for the given stem
 * @stem:	Alias stem to be examined
 *
 * The function travels the lookup table to get the highest alias id for the
 * given alias stem.
 * @return alias ID, if found, else -1
 */
int dev_read_alias_highest_id(const char *stem);

#else /* CONFIG_DM_DEV_READ_INLINE is enabled */

static inline int dev_read_u32(struct udevice *dev,
			       const char *propname, u32 *outp)
{
	return ofnode_read_u32(dev_ofnode(dev), propname, outp);
}

static inline int dev_read_u32_default(struct udevice *dev,
				       const char *propname, int def)
{
	return ofnode_read_u32_default(dev_ofnode(dev), propname, def);
}

static inline int dev_read_s32(struct udevice *dev,
			       const char *propname, s32 *outp)
{
	return ofnode_read_s32(dev_ofnode(dev), propname, outp);
}

static inline int dev_read_s32_default(struct udevice *dev,
				       const char *propname, int def)
{
	return ofnode_read_s32_default(dev_ofnode(dev), propname, def);
}

static inline int dev_read_u32u(struct udevice *dev,
				const char *propname, uint *outp)
{
	u32 val;
	int ret;

	ret = ofnode_read_u32(dev_ofnode(dev), propname, &val);
	if (ret)
		return ret;
	*outp = val;

	return 0;
}

static inline const char *dev_read_string(struct udevice *dev,
					  const char *propname)
{
	return ofnode_read_string(dev_ofnode(dev), propname);
}

static inline bool dev_read_bool(struct udevice *dev, const char *propname)
{
	return ofnode_read_bool(dev_ofnode(dev), propname);
}

static inline ofnode dev_read_subnode(struct udevice *dev,
				      const char *subbnode_name)
{
	return ofnode_find_subnode(dev_ofnode(dev), subbnode_name);
}

static inline int dev_read_size(struct udevice *dev, const char *propname)
{
	return ofnode_read_size(dev_ofnode(dev), propname);
}

static inline fdt_addr_t dev_read_addr_index(struct udevice *dev, int index)
{
	return devfdt_get_addr_index(dev, index);
}

static inline fdt_addr_t dev_read_addr_name(struct udevice *dev,
					    const char *name)
{
	return devfdt_get_addr_name(dev, name);
}

static inline fdt_addr_t dev_read_addr(struct udevice *dev)
{
	return devfdt_get_addr(dev);
}

static inline void *dev_read_addr_ptr(struct udevice *dev)
{
	return devfdt_get_addr_ptr(dev);
}

static inline void *dev_remap_addr(struct udevice *dev)
{
	return devfdt_remap_addr(dev);
}

static inline void *dev_remap_addr_index(struct udevice *dev, int index)
{
	return devfdt_remap_addr_index(dev, index);
}

static inline void *dev_remap_addr_name(struct udevice *dev, const char *name)
{
	return devfdt_remap_addr_name(dev, name);
}

static inline fdt_addr_t dev_read_addr_size(struct udevice *dev,
					    const char *propname,
					    fdt_size_t *sizep)
{
	return ofnode_get_addr_size(dev_ofnode(dev), propname, sizep);
}

static inline const char *dev_read_name(struct udevice *dev)
{
	return ofnode_get_name(dev_ofnode(dev));
}

static inline int dev_read_stringlist_search(struct udevice *dev,
					     const char *propname,
					     const char *string)
{
	return ofnode_stringlist_search(dev_ofnode(dev), propname, string);
}

static inline int dev_read_string_index(struct udevice *dev,
					const char *propname, int index,
					const char **outp)
{
	return ofnode_read_string_index(dev_ofnode(dev), propname, index, outp);
}

static inline int dev_read_string_count(struct udevice *dev,
					const char *propname)
{
	return ofnode_read_string_count(dev_ofnode(dev), propname);
}

static inline int dev_read_phandle_with_args(struct udevice *dev,
		const char *list_name, const char *cells_name, int cell_count,
		int index, struct ofnode_phandle_args *out_args)
{
	return ofnode_parse_phandle_with_args(dev_ofnode(dev), list_name,
					      cells_name, cell_count, index,
					      out_args);
}

static inline int dev_count_phandle_with_args(struct udevice *dev,
		const char *list_name, const char *cells_name)
{
	return ofnode_count_phandle_with_args(dev_ofnode(dev), list_name,
					      cells_name);
}

static inline int dev_read_addr_cells(struct udevice *dev)
{
	/* NOTE: this call should walk up the parent stack */
	return fdt_address_cells(gd->fdt_blob, dev_of_offset(dev));
}

static inline int dev_read_size_cells(struct udevice *dev)
{
	/* NOTE: this call should walk up the parent stack */
	return fdt_size_cells(gd->fdt_blob, dev_of_offset(dev));
}

static inline int dev_read_simple_addr_cells(struct udevice *dev)
{
	return fdt_address_cells(gd->fdt_blob, dev_of_offset(dev));
}

static inline int dev_read_simple_size_cells(struct udevice *dev)
{
	return fdt_size_cells(gd->fdt_blob, dev_of_offset(dev));
}

static inline int dev_read_phandle(struct udevice *dev)
{
	return fdt_get_phandle(gd->fdt_blob, dev_of_offset(dev));
}

static inline const void *dev_read_prop(struct udevice *dev,
					const char *propname, int *lenp)
{
	return ofnode_get_property(dev_ofnode(dev), propname, lenp);
}

static inline int dev_read_alias_seq(struct udevice *dev, int *devnump)
{
	return fdtdec_get_alias_seq(gd->fdt_blob, dev->uclass->uc_drv->name,
				    dev_of_offset(dev), devnump);
}

static inline int dev_read_u32_array(struct udevice *dev, const char *propname,
				     u32 *out_values, size_t sz)
{
	return ofnode_read_u32_array(dev_ofnode(dev), propname, out_values, sz);
}

static inline ofnode dev_read_first_subnode(struct udevice *dev)
{
	return ofnode_first_subnode(dev_ofnode(dev));
}

static inline ofnode dev_read_next_subnode(ofnode node)
{
	return ofnode_next_subnode(node);
}

static inline const uint8_t *dev_read_u8_array_ptr(struct udevice *dev,
					const char *propname, size_t sz)
{
	return ofnode_read_u8_array_ptr(dev_ofnode(dev), propname, sz);
}

static inline int dev_read_enabled(struct udevice *dev)
{
	return fdtdec_get_is_enabled(gd->fdt_blob, dev_of_offset(dev));
}

static inline int dev_read_resource(struct udevice *dev, uint index,
				    struct resource *res)
{
	return ofnode_read_resource(dev_ofnode(dev), index, res);
}

static inline int dev_read_resource_byname(struct udevice *dev,
					   const char *name,
					   struct resource *res)
{
	return ofnode_read_resource_byname(dev_ofnode(dev), name, res);
}

static inline u64 dev_translate_address(struct udevice *dev, const fdt32_t *in_addr)
{
	return ofnode_translate_address(dev_ofnode(dev), in_addr);
}

static inline int dev_read_alias_highest_id(const char *stem)
{
	return fdtdec_get_alias_highest_id(gd->fdt_blob, stem);
}

#endif /* CONFIG_DM_DEV_READ_INLINE */

/**
 * dev_for_each_subnode() - Helper function to iterate through subnodes
 *
 * This creates a for() loop which works through the subnodes in a device's
 * device-tree node.
 *
 * @subnode: ofnode holding the current subnode
 * @dev: device to use for interation (struct udevice *)
 */
#define dev_for_each_subnode(subnode, dev) \
	for (subnode = dev_read_first_subnode(dev); \
	     ofnode_valid(subnode); \
	     subnode = ofnode_next_subnode(subnode))

#endif
