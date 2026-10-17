/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Function to read values from the device tree node attached to a udevice.
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _DM_READ_H
#define _DM_READ_H

#include <linux/errno.h>

#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>

struct resource;

#if CONFIG_IS_ENABLED(OF_LIVE)
static inline const struct device_node *dev_np(const struct udevice *dev)
{
	return ofnode_to_np(dev_ofnode(dev));
}
#else
static inline const struct device_node *dev_np(const struct udevice *dev)
{
	return NULL;
}
#endif

#if !defined(CONFIG_DM_DEV_READ_INLINE) || CONFIG_IS_ENABLED(OF_PLATDATA)
/**
 * dev_read_u8() - read a 8-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int dev_read_u8(const struct udevice *dev, const char *propname, u8 *outp);

/**
 * dev_read_u8_default() - read a 8-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u8 dev_read_u8_default(const struct udevice *dev, const char *propname, u8 def);

/**
 * dev_read_u16() - read a 16-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int dev_read_u16(const struct udevice *dev, const char *propname, u16 *outp);

/**
 * dev_read_u16_default() - read a 16-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u16 dev_read_u16_default(const struct udevice *dev, const char *propname,
			 u16 def);

/**
 * dev_read_u32() - read a 32-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int dev_read_u32(const struct udevice *dev, const char *propname, u32 *outp);

/**
 * dev_read_u32_default() - read a 32-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
int dev_read_u32_default(const struct udevice *dev, const char *propname,
			 int def);

/**
 * dev_read_u32_index() - read an indexed 32-bit integer from a device's DT
 *                        property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @index:	index of the integer to return
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int dev_read_u32_index(struct udevice *dev, const char *propname, int index,
		       u32 *outp);

/**
 * dev_read_u32_index_default() - read an indexed 32-bit integer from a device's
 *                                DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @index:	index of the integer to return
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u32 dev_read_u32_index_default(struct udevice *dev, const char *propname,
			       int index, u32 def);

/**
 * dev_read_s32() - read a signed 32-bit integer from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int dev_read_s32(const struct udevice *dev, const char *propname, s32 *outp);

/**
 * dev_read_s32_default() - read a signed 32-bit int from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @def:	default value to return if the property has no value
 * Return: property value, or @def if not found
 */
int dev_read_s32_default(const struct udevice *dev, const char *propname,
			 int def);

/**
 * dev_read_u32u() - read a 32-bit integer from a device's DT property
 *
 * This version uses a standard uint type.
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read from
 * @outp:	place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int dev_read_u32u(const struct udevice *dev, const char *propname, uint *outp);

/**
 * dev_read_u64() - read a 64-bit integer from a device's DT property
 *
 * @dev:        device to read DT property from
 * @propname:   name of the property to read from
 * @outp:       place to put value (if found)
 * Return: 0 if OK, -ve on error
 */
int dev_read_u64(const struct udevice *dev, const char *propname, u64 *outp);

/**
 * dev_read_u64_default() - read a 64-bit integer from a device's DT property
 *
 * @dev:        device to read DT property from
 * @propname:   name of the property to read from
 * @def:        default value to return if the property has no value
 * Return: property value, or @def if not found
 */
u64 dev_read_u64_default(const struct udevice *dev, const char *propname,
			 u64 def);

/**
 * dev_read_string() - Read a string from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of the property to read
 * Return: string from property value, or NULL if there is no such property
 */
const char *dev_read_string(const struct udevice *dev, const char *propname);

/**
 * dev_read_bool() - read a boolean value from a device's DT property
 *
 * @dev:	device to read DT property from
 * @propname:	name of property to read
 * Return: true if property is present (meaning true), false if not present
 */
bool dev_read_bool(const struct udevice *dev, const char *propname);

/**
 * dev_read_subnode() - find a named subnode of a device
 *
 * @dev:	device whose DT node contains the subnode
 * @subnode_name: name of subnode to find
 * Return: reference to subnode (which can be invalid if there is no such
 * subnode)
 */
ofnode dev_read_subnode(const struct udevice *dev, const char *subnode_name);

/**
 * dev_read_size() - read the size of a property
 *
 * @dev: device to check
 * @propname: property to check
 * Return: size of property if present, or -EINVAL if not
 */
int dev_read_size(const struct udevice *dev, const char *propname);

/**
 * dev_read_addr_index() - Get the indexed reg property of a device
 *
 * @dev: Device to read from
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * Return: address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr_index(const struct udevice *dev, int index);

/**
 * dev_read_addr_index_ptr() - Get the indexed reg property of a device
 *                             as a pointer
 *
 * @dev: Device to read from
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * Return: pointer or NULL if not found
 */
void *dev_read_addr_index_ptr(const struct udevice *dev, int index);

/**
 * dev_read_addr_size_index() - Get the indexed reg property of a device
 *
 * @dev: Device to read from
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 * @size: place to put size value (on success)
 *
 * Return: address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr_size_index(const struct udevice *dev, int index,
				    fdt_size_t *size);

/**
 * dev_read_addr_size_index_ptr() - Get the indexed reg property of a device
 *                                  as a pointer
 *
 * @dev: Device to read from
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 * @size: place to put size value (on success)
 *
 * Return: pointer or NULL if not found
 */
void *dev_read_addr_size_index_ptr(const struct udevice *dev, int index,
				   fdt_size_t *size);

/**
 * dev_remap_addr_index() - Get the indexed reg property of a device
 *                               as a memory-mapped I/O pointer
 *
 * @dev: Device to read from
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * Return: pointer or NULL if not found
 */
void *dev_remap_addr_index(const struct udevice *dev, int index);

/**
 * dev_read_addr_name() - Get the reg property of a device, indexed by name
 *
 * @dev: Device to read from
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 *
 * Return: address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr_name(const struct udevice *dev, const char *name);

/**
 * dev_read_addr_name_ptr() - Get the reg property of a device as a pointer,
 *                            indexed by name
 *
 * @dev: Device to read from
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 *
 * Return: pointer or NULL if not found
 */
void *dev_read_addr_name_ptr(const struct udevice *dev, const char *name);

/**
 * dev_read_addr_size_name() - Get the reg property of a device, indexed by name
 *
 * @dev: Device to read from
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 *  @size: place to put size value (on success)
 *
 * Return: address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr_size_name(const struct udevice *dev, const char *name,
				   fdt_size_t *size);

/**
 * dev_read_addr_size_name_ptr() - Get the reg property of a device as a pointer,
 *                                 indexed by name
 *
 * @dev: Device to read from
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 *  @size: place to put size value (on success)
 *
 * Return: pointer or NULL if not found
 */
void *dev_read_addr_size_name_ptr(const struct udevice *dev, const char *name,
				  fdt_size_t *size);

/**
 * dev_remap_addr_name() - Get the reg property of a device, indexed by name,
 *                         as a memory-mapped I/O pointer
 *
 * @dev: Device to read from
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @name
 *	  indicates the value to search for in 'reg-names'.
 *
 * Return: pointer or NULL if not found
 */
void *dev_remap_addr_name(const struct udevice *dev, const char *name);

/**
 * dev_read_addr() - Get the reg property of a device
 *
 * @dev: Device to read from
 *
 * Return: address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr(const struct udevice *dev);

/**
 * dev_read_addr_ptr() - Get the reg property of a device
 *                       as a pointer
 *
 * @dev: Device to read from
 *
 * Return: pointer or NULL if not found
 */
void *dev_read_addr_ptr(const struct udevice *dev);

/**
 * dev_read_addr_pci() - Read an address and handle PCI address translation
 *
 * At present U-Boot does not have address translation logic for PCI in the
 * livetree implementation (of_addr.c). This special function supports this for
 * the flat tree implementation.
 *
 * This function should be removed (and code should use dev_read() instead)
 * once:
 *
 * 1. PCI address translation is added; and either
 * 2. everything uses livetree where PCI translation is used (which is feasible
 *    in SPL and U-Boot proper) or PCI address translation is added to
 *    fdtdec_get_addr() and friends.
 *
 * @dev: Device to read from
 * @sizep: If non-NULL, returns size of address space found
 * Return: address or FDT_ADDR_T_NONE if not found
 */
fdt_addr_t dev_read_addr_pci(const struct udevice *dev, fdt_size_t *sizep);

/**
 * dev_remap_addr() - Get the reg property of a device as a
 *                         memory-mapped I/O pointer
 *
 * @dev: Device to read from
 *
 * Return: pointer or NULL if not found
 */
void *dev_remap_addr(const struct udevice *dev);

/**
 * dev_read_addr_size() - Get the reg property of a device
 *
 * @dev: Device to read from
 * @sizep: place to put size value (on success)
 * Return: address value, or FDT_ADDR_T_NONE on error
 */
fdt_addr_t dev_read_addr_size(const struct udevice *dev, fdt_size_t *sizep);

/**
 * dev_read_name() - get the name of a device's node
 *
 * @dev: Device to read from
 * Return: name of node
 */
const char *dev_read_name(const struct udevice *dev);

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
 * Return:
 *   the index of the string in the list of strings
 *   -ENODATA if the property is not found
 *   -EINVAL on some other error
 */
int dev_read_stringlist_search(const struct udevice *dev, const char *propname,
			       const char *string);

/**
 * dev_read_string_index() - obtain an indexed string from a string list
 *
 * @dev: device to examine
 * @propname: name of the property containing the string list
 * @index: index of the string to return
 * @outp: return location for the string
 *
 * Return:
 *   length of string, if found or -ve error value if not found
 */
int dev_read_string_index(const struct udevice *dev, const char *propname,
			  int index, const char **outp);

/**
 * dev_read_string_count() - find the number of strings in a string list
 *
 * @dev: device to examine
 * @propname: name of the property containing the string list
 * Return:
 *   number of strings in the list, or -ve error value if not found
 */
int dev_read_string_count(const struct udevice *dev, const char *propname);

/**
 * dev_read_string_list() - read a list of strings
 *
 * This produces a list of string pointers with each one pointing to a string
 * in the string list. If the property does not exist, it returns {NULL}.
 *
 * The data is allocated and the caller is reponsible for freeing the return
 * value (the list of string pointers). The strings themselves may not be
 * changed as they point directly into the devicetree property.
 *
 * @dev: device to examine
 * @propname: name of the property containing the string list
 * @listp: returns an allocated, NULL-terminated list of strings if the return
 *	value is > 0, else is set to NULL
 * Return:
 * number of strings in list, 0 if none, -ENOMEM if out of memory,
 * -ENOENT if no such property
 */
int dev_read_string_list(const struct udevice *dev, const char *propname,
			 const char ***listp);

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
 * dev_read_phandle_with_args(dev, "list", "#list-cells", 0, 1, &args);
 *
 * @dev:	device whose node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @cell_count: Cell count to use if @cells_name is NULL
 * @index:	index of a phandle to parse out
 * @out_args:	optional pointer to output arguments structure (will be filled)
 * Return: 0 on success (with @out_args filled out if not NULL), -ENOENT if
 *	@list_name does not exist, -EINVAL if a phandle was not found,
 *	@cells_name could not be found, the arguments were truncated or there
 *	were too many arguments.
 */
int dev_read_phandle_with_args(const struct udevice *dev, const char *list_name,
			       const char *cells_name, int cell_count,
			       int index, struct ofnode_phandle_args *out_args);

/**
 * dev_count_phandle_with_args() - Return phandle number in a list
 *
 * This function is usefull to get phandle number contained in a property list.
 * For example, this allows to allocate the right amount of memory to keep
 * clock's reference contained into the "clocks" property.
 *
 * @dev:	device whose node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @cell_count: Cell count to use if @cells_name is NULL
 * Return: number of phandle found on success, on error returns appropriate
 * errno value.
 */

int dev_count_phandle_with_args(const struct udevice *dev,
				const char *list_name, const char *cells_name,
				int cell_count);

/**
 * dev_read_addr_cells() - Get the number of address cells for a device's node
 *
 * This walks back up the tree to find the closest #address-cells property
 * which controls the given node.
 *
 * @dev: device to check
 * Return: number of address cells this node uses
 */
int dev_read_addr_cells(const struct udevice *dev);

/**
 * dev_read_size_cells() - Get the number of size cells for a device's node
 *
 * This walks back up the tree to find the closest #size-cells property
 * which controls the given node.
 *
 * @dev: device to check
 * Return: number of size cells this node uses
 */
int dev_read_size_cells(const struct udevice *dev);

/**
 * dev_read_addr_cells() - Get the address cells property in a node
 *
 * This function matches fdt_address_cells().
 *
 * @dev: device to check
 * Return: number of address cells this node uses
 */
int dev_read_simple_addr_cells(const struct udevice *dev);

/**
 * dev_read_size_cells() - Get the size cells property in a node
 *
 * This function matches fdt_size_cells().
 *
 * @dev: device to check
 * Return: number of size cells this node uses
 */
int dev_read_simple_size_cells(const struct udevice *dev);

/**
 * dev_read_phandle() - Get the phandle from a device
 *
 * @dev: device to check
 * Return: phandle (1 or greater), or 0 if no phandle or other error
 */
int dev_read_phandle(const struct udevice *dev);

/**
 * dev_read_prop()- - read a property from a device's node
 *
 * @dev: device to check
 * @propname: property to read
 * @lenp: place to put length on success
 * Return: pointer to property, or NULL if not found
 */
const void *dev_read_prop(const struct udevice *dev, const char *propname,
			  int *lenp);

/**
 * dev_read_first_prop()- get the reference of the first property
 *
 * Get reference to the first property of the node, it is used to iterate
 * and read all the property with dev_read_prop_by_prop().
 *
 * @dev: device to check
 * @prop: place to put argument reference
 * Return: 0 if OK, -ve on error. -FDT_ERR_NOTFOUND if not found
 */
int dev_read_first_prop(const struct udevice *dev, struct ofprop *prop);

/**
 * ofnode_next_property() - get the reference of the next property
 *
 * Get reference to the next property of the node, it is used to iterate
 * and read all the property with dev_read_prop_by_prop().
 *
 * @prop: reference of current argument and place to put reference of next one
 * Return: 0 if OK, -ve on error. -FDT_ERR_NOTFOUND if not found
 */
int dev_read_next_prop(struct ofprop *prop);

/**
 * dev_read_prop_by_prop() - get a pointer to the value of a property
 *
 * Get value for the property identified by the provided reference.
 *
 * @prop: reference on property
 * @propname: If non-NULL, place to property name on success,
 * @lenp: If non-NULL, place to put length on success
 * Return: 0 if OK, -ve on error. -FDT_ERR_NOTFOUND if not found
 */
const void *dev_read_prop_by_prop(struct ofprop *prop,
				  const char **propname, int *lenp);

/**
 * dev_read_alias_seq() - Get the alias sequence number of a node
 *
 * This works out whether a node is pointed to by an alias, and if so, the
 * sequence number of that alias. Aliases are of the form <base><num> where
 * <num> is the sequence number. For example spi2 would be sequence number 2.
 *
 * @dev: device to look up
 * @devnump: set to the sequence number if one is found
 * Return: 0 if a sequence was found, -ve if not
 */
int dev_read_alias_seq(const struct udevice *dev, int *devnump);

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
 * Return: 0 on success, -EINVAL if the property does not exist, -ENODATA if
 * property does not have a value, and -EOVERFLOW if the property data isn't
 * large enough.
 */
int dev_read_u32_array(const struct udevice *dev, const char *propname,
		       u32 *out_values, size_t sz);

/**
 * dev_read_first_subnode() - find the first subnode of a device's node
 *
 * @dev: device to look up
 * Return: reference to the first subnode (which can be invalid if the device's
 * node has no subnodes)
 */
ofnode dev_read_first_subnode(const struct udevice *dev);

/**
 * ofnode_next_subnode() - find the next sibling of a subnode
 *
 * @node:	valid reference to previous node (sibling)
 * Return: reference to the next subnode (which can be invalid if the node
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
 * Return:
 * pointer to byte array if found, or NULL if the property is not found or
 * there is not enough data
 */
const uint8_t *dev_read_u8_array_ptr(const struct udevice *dev,
				     const char *propname, size_t sz);

/**
 * dev_read_enabled() - check whether a node is enabled
 *
 * This looks for a 'status' property. If this exists, then returns 1 if
 * the status is 'ok' and 0 otherwise. If there is no status property,
 * it returns 1 on the assumption that anything mentioned should be enabled
 * by default.
 *
 * @dev: device to examine
 * Return: integer value 0 (not enabled) or 1 (enabled)
 */
int dev_read_enabled(const struct udevice *dev);

/**
 * dev_read_resource() - obtain an indexed resource from a device.
 *
 * @dev: device to examine
 * @index: index of the resource to retrieve (0 = first)
 * @res: returns the resource
 * Return: 0 if ok, negative on error
 */
int dev_read_resource(const struct udevice *dev, uint index,
		      struct resource *res);

/**
 * dev_read_resource_byname() - obtain a named resource from a device.
 *
 * @dev: device to examine
 * @name: name of the resource to retrieve
 * @res: returns the resource
 * Return: 0 if ok, negative on error
 */
int dev_read_resource_byname(const struct udevice *dev, const char *name,
			     struct resource *res);

/**
 * dev_translate_address() - Translate a device-tree address
 *
 * Translate an address from the device-tree into a CPU physical address.  This
 * function walks up the tree and applies the various bus mappings along the
 * way.
 *
 * @dev: device giving the context in which to translate the address
 * @in_addr: pointer to the address to translate
 * Return: the translated address; OF_BAD_ADDR on error
 */
u64 dev_translate_address(const struct udevice *dev, const fdt32_t *in_addr);

/**
 * dev_translate_dma_address() - Translate a device-tree DMA address
 *
 * Translate a DMA address from the device-tree into a CPU physical address.
 * This function walks up the tree and applies the various bus mappings along
 * the way.
 *
 * @dev: device giving the context in which to translate the DMA address
 * @in_addr: pointer to the DMA address to translate
 * Return: the translated DMA address; OF_BAD_ADDR on error
 */
u64 dev_translate_dma_address(const struct udevice *dev,
			      const fdt32_t *in_addr);

/**
 * dev_get_dma_range() - Get a device's DMA constraints
 *
 * Provide the address bases and size of the linear mapping between the CPU and
 * a device's BUS address space.
 *
 * @dev: device giving the context in which to translate the DMA address
 * @cpu: base address for CPU's view of memory
 * @bus: base address for BUS's view of memory
 * @size: size of the address space
 * Return: 0 if ok, negative on error
 */
int dev_get_dma_range(const struct udevice *dev, phys_addr_t *cpu,
		      dma_addr_t *bus, u64 *size);

/**
 * dev_read_alias_highest_id - Get highest alias id for the given stem
 * @stem:	Alias stem to be examined
 *
 * The function travels the lookup table to get the highest alias id for the
 * given alias stem.
 * Return: alias ID, if found, else -1
 */
int dev_read_alias_highest_id(const char *stem);

/**
 * dev_get_child_count() - get the child count of a device
 *
 * @dev: device to use for interation (`struct udevice *`)
 * Return: the count of child subnode
 */
int dev_get_child_count(const struct udevice *dev);

/**
 * dev_read_pci_bus_range - Read PCI bus-range resource
 *
 * Look at the bus range property of a device node and return the pci bus
 * range for this node.
 *
 * @dev: device to examine
 * @res: returns the resource
 * Return: 0 if ok, negative on error
 */
int dev_read_pci_bus_range(const struct udevice *dev, struct resource *res);

/**
 * dev_decode_display_timing() - decode display timings
 *
 * Decode display timings from the supplied 'display-timings' node.
 * See doc/device-tree-bindings/video/display-timing.txt for binding
 * information.
 *
 * @dev: device to read DT display timings from. The node linked to the device
 *       contains a child node called 'display-timings' which in turn contains
 *       one or more display timing nodes.
 * @index: index number to read (0=first timing subnode)
 * @config: place to put timings
 * Return: 0 if OK, -FDT_ERR_NOTFOUND if not found
 */
int dev_decode_display_timing(const struct udevice *dev, int index,
			      struct display_timing *config);

/**
 * dev_decode_panel_timing() - decode panel timings
 *
 * Decode display timings from the supplied 'panel-timings' node.
 *
 * @dev: device to read DT display timings from. The node linked to the device
 *       contains a child node called 'display-timings' which in turn contains
 *       one or more display timing nodes.
 * @config: place to put timings
 * Return: 0 if OK, -FDT_ERR_NOTFOUND if not found
 */
int dev_decode_panel_timing(const struct udevice *dev,
			    struct display_timing *config);

/**
 * dev_get_phy_node() - Get PHY node for a MAC (if not fixed-link)
 *
 * This function parses PHY handle from the Ethernet controller's ofnode
 * (trying all possible PHY handle property names), and returns the PHY ofnode.
 *
 * Before this is used, ofnode_phy_is_fixed_link() should be checked first, and
 * if the result to that is true, this function should not be called.
 *
 * @dev: device representing the MAC
 * Return: ofnode of the PHY, if it exists, otherwise an invalid ofnode
 */
ofnode dev_get_phy_node(const struct udevice *dev);

/**
 * dev_read_phy_mode() - Read PHY connection type from a MAC
 *
 * This function parses the "phy-mode" / "phy-connection-type" property and
 * returns the corresponding PHY interface type.
 *
 * @dev: device representing the MAC
 * Return: one of PHY_INTERFACE_MODE_* constants, PHY_INTERFACE_MODE_NA on
 *	   error
 */
phy_interface_t dev_read_phy_mode(const struct udevice *dev);

#else /* CONFIG_DM_DEV_READ_INLINE is enabled */
#include <asm/global_data.h>

static inline int dev_read_u8(const struct udevice *dev,
			      const char *propname, u8 *outp)
{
	return ofnode_read_u8(dev_ofnode(dev), propname, outp);
}

static inline int dev_read_u8_default(const struct udevice *dev,
				      const char *propname, u8 def)
{
	return ofnode_read_u8_default(dev_ofnode(dev), propname, def);
}

static inline int dev_read_u16(const struct udevice *dev,
			       const char *propname, u16 *outp)
{
	return ofnode_read_u16(dev_ofnode(dev), propname, outp);
}

static inline int dev_read_u16_default(const struct udevice *dev,
				       const char *propname, u16 def)
{
	return ofnode_read_u16_default(dev_ofnode(dev), propname, def);
}

static inline int dev_read_u32(const struct udevice *dev,
			       const char *propname, u32 *outp)
{
	return ofnode_read_u32(dev_ofnode(dev), propname, outp);
}

static inline int dev_read_u32_default(const struct udevice *dev,
				       const char *propname, int def)
{
	return ofnode_read_u32_default(dev_ofnode(dev), propname, def);
}

static inline int dev_read_u32_index(struct udevice *dev,
				     const char *propname, int index, u32 *outp)
{
	return ofnode_read_u32_index(dev_ofnode(dev), propname, index, outp);
}

static inline u32 dev_read_u32_index_default(struct udevice *dev,
					     const char *propname, int index,
					     u32 def)
{
	return ofnode_read_u32_index_default(dev_ofnode(dev), propname, index,
					     def);
}

static inline int dev_read_s32(const struct udevice *dev,
			       const char *propname, s32 *outp)
{
	return ofnode_read_s32(dev_ofnode(dev), propname, outp);
}

static inline int dev_read_s32_default(const struct udevice *dev,
				       const char *propname, int def)
{
	return ofnode_read_s32_default(dev_ofnode(dev), propname, def);
}

static inline int dev_read_u32u(const struct udevice *dev,
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

static inline int dev_read_u64(const struct udevice *dev,
			       const char *propname, u64 *outp)
{
	return ofnode_read_u64(dev_ofnode(dev), propname, outp);
}

static inline u64 dev_read_u64_default(const struct udevice *dev,
				       const char *propname, u64 def)
{
	return ofnode_read_u64_default(dev_ofnode(dev), propname, def);
}

static inline const char *dev_read_string(const struct udevice *dev,
					  const char *propname)
{
	return ofnode_read_string(dev_ofnode(dev), propname);
}

static inline bool dev_read_bool(const struct udevice *dev,
				 const char *propname)
{
	return ofnode_read_bool(dev_ofnode(dev), propname);
}

static inline ofnode dev_read_subnode(const struct udevice *dev,
				      const char *subbnode_name)
{
	return ofnode_find_subnode(dev_ofnode(dev), subbnode_name);
}

static inline int dev_read_size(const struct udevice *dev, const char *propname)
{
	return ofnode_read_size(dev_ofnode(dev), propname);
}

static inline fdt_addr_t dev_read_addr_index(const struct udevice *dev,
					     int index)
{
	return devfdt_get_addr_index(dev, index);
}

static inline void *dev_read_addr_index_ptr(const struct udevice *dev,
					    int index)
{
	return devfdt_get_addr_index_ptr(dev, index);
}

static inline fdt_addr_t dev_read_addr_size_index(const struct udevice *dev,
						  int index,
						  fdt_size_t *size)
{
	return devfdt_get_addr_size_index(dev, index, size);
}

static inline void *dev_read_addr_size_index_ptr(const struct udevice *dev,
						 int index,
						 fdt_size_t *size)
{
	return devfdt_get_addr_size_index_ptr(dev, index, size);
}

static inline fdt_addr_t dev_read_addr_name(const struct udevice *dev,
					    const char *name)
{
	return devfdt_get_addr_name(dev, name);
}

static inline void *dev_read_addr_name_ptr(const struct udevice *dev,
					   const char *name)
{
	return devfdt_get_addr_name_ptr(dev, name);
}

static inline fdt_addr_t dev_read_addr_size_name(const struct udevice *dev,
						 const char *name,
						 fdt_size_t *size)
{
	return devfdt_get_addr_size_name(dev, name, size);
}

static inline void *dev_read_addr_size_name_ptr(const struct udevice *dev,
						const char *name,
						fdt_size_t *size)
{
	return devfdt_get_addr_size_name_ptr(dev, name, size);
}

static inline fdt_addr_t dev_read_addr(const struct udevice *dev)
{
	return devfdt_get_addr(dev);
}

static inline void *dev_read_addr_ptr(const struct udevice *dev)
{
	return devfdt_get_addr_ptr(dev);
}

static inline fdt_addr_t dev_read_addr_pci(const struct udevice *dev,
					   fdt_size_t *sizep)
{
	return devfdt_get_addr_pci(dev, sizep);
}

static inline void *dev_remap_addr(const struct udevice *dev)
{
	return devfdt_remap_addr(dev);
}

static inline void *dev_remap_addr_index(const struct udevice *dev, int index)
{
	return devfdt_remap_addr_index(dev, index);
}

static inline void *dev_remap_addr_name(const struct udevice *dev,
					const char *name)
{
	return devfdt_remap_addr_name(dev, name);
}

static inline fdt_addr_t dev_read_addr_size(const struct udevice *dev,
					    fdt_size_t *sizep)
{
	return dev_read_addr_size_index(dev, 0, sizep);
}

static inline const char *dev_read_name(const struct udevice *dev)
{
	return ofnode_get_name(dev_ofnode(dev));
}

static inline int dev_read_stringlist_search(const struct udevice *dev,
					     const char *propname,
					     const char *string)
{
	return ofnode_stringlist_search(dev_ofnode(dev), propname, string);
}

static inline int dev_read_string_index(const struct udevice *dev,
					const char *propname, int index,
					const char **outp)
{
	return ofnode_read_string_index(dev_ofnode(dev), propname, index, outp);
}

static inline int dev_read_string_count(const struct udevice *dev,
					const char *propname)
{
	return ofnode_read_string_count(dev_ofnode(dev), propname);
}

static inline int dev_read_string_list(const struct udevice *dev,
				       const char *propname,
				       const char ***listp)
{
	return ofnode_read_string_list(dev_ofnode(dev), propname, listp);
}

static inline int dev_read_phandle_with_args(const struct udevice *dev,
		const char *list_name, const char *cells_name, int cell_count,
		int index, struct ofnode_phandle_args *out_args)
{
	return ofnode_parse_phandle_with_args(dev_ofnode(dev), list_name,
					      cells_name, cell_count, index,
					      out_args);
}

static inline int dev_count_phandle_with_args(const struct udevice *dev,
		const char *list_name, const char *cells_name, int cell_count)
{
	return ofnode_count_phandle_with_args(dev_ofnode(dev), list_name,
					      cells_name, cell_count);
}

static inline int dev_read_addr_cells(const struct udevice *dev)
{
	int parent = fdt_parent_offset(gd->fdt_blob, dev_of_offset(dev));

	return fdt_address_cells(gd->fdt_blob, parent);
}

static inline int dev_read_size_cells(const struct udevice *dev)
{
	int parent = fdt_parent_offset(gd->fdt_blob, dev_of_offset(dev));

	return fdt_size_cells(gd->fdt_blob, parent);
}

static inline int dev_read_simple_addr_cells(const struct udevice *dev)
{
	return fdt_address_cells(gd->fdt_blob, dev_of_offset(dev));
}

static inline int dev_read_simple_size_cells(const struct udevice *dev)
{
	return fdt_size_cells(gd->fdt_blob, dev_of_offset(dev));
}

static inline int dev_read_phandle(const struct udevice *dev)
{
	return fdt_get_phandle(gd->fdt_blob, dev_of_offset(dev));
}

static inline const void *dev_read_prop(const struct udevice *dev,
					const char *propname, int *lenp)
{
	return ofnode_get_property(dev_ofnode(dev), propname, lenp);
}

static inline int dev_read_first_prop(const struct udevice *dev, struct ofprop *prop)
{
	return ofnode_first_property(dev_ofnode(dev), prop);
}

static inline int dev_read_next_prop(struct ofprop *prop)
{
	return ofnode_next_property(prop);
}

static inline const void *dev_read_prop_by_prop(struct ofprop *prop,
						const char **propname,
						int *lenp)
{
	return ofprop_get_property(prop, propname, lenp);
}

static inline int dev_read_alias_seq(const struct udevice *dev, int *devnump)
{
#if CONFIG_IS_ENABLED(OF_CONTROL)
	return fdtdec_get_alias_seq(gd->fdt_blob, dev->uclass->uc_drv->name,
				    dev_of_offset(dev), devnump);
#else
	return -ENOTSUPP;
#endif
}

static inline int dev_read_u32_array(const struct udevice *dev,
				     const char *propname, u32 *out_values,
				     size_t sz)
{
	return ofnode_read_u32_array(dev_ofnode(dev), propname, out_values, sz);
}

static inline ofnode dev_read_first_subnode(const struct udevice *dev)
{
	return ofnode_first_subnode(dev_ofnode(dev));
}

static inline ofnode dev_read_next_subnode(ofnode node)
{
	return ofnode_next_subnode(node);
}

static inline const uint8_t *dev_read_u8_array_ptr(const struct udevice *dev,
						   const char *propname,
						   size_t sz)
{
	return ofnode_read_u8_array_ptr(dev_ofnode(dev), propname, sz);
}

static inline int dev_read_enabled(const struct udevice *dev)
{
	return fdtdec_get_is_enabled(gd->fdt_blob, dev_of_offset(dev));
}

static inline int dev_read_resource(const struct udevice *dev, uint index,
				    struct resource *res)
{
	return ofnode_read_resource(dev_ofnode(dev), index, res);
}

static inline int dev_read_resource_byname(const struct udevice *dev,
					   const char *name,
					   struct resource *res)
{
	return ofnode_read_resource_byname(dev_ofnode(dev), name, res);
}

static inline u64 dev_translate_address(const struct udevice *dev,
					const fdt32_t *in_addr)
{
	return ofnode_translate_address(dev_ofnode(dev), in_addr);
}

static inline u64 dev_translate_dma_address(const struct udevice *dev,
					    const fdt32_t *in_addr)
{
	return ofnode_translate_dma_address(dev_ofnode(dev), in_addr);
}

static inline int dev_get_dma_range(const struct udevice *dev, phys_addr_t *cpu,
				    dma_addr_t *bus, u64 *size)
{
	return ofnode_get_dma_range(dev_ofnode(dev), cpu, bus, size);
}

static inline int dev_read_alias_highest_id(const char *stem)
{
	if (!CONFIG_IS_ENABLED(OF_LIBFDT) || !gd->fdt_blob)
		return -1;
	return fdtdec_get_alias_highest_id(gd->fdt_blob, stem);
}

static inline int dev_get_child_count(const struct udevice *dev)
{
	return ofnode_get_child_count(dev_ofnode(dev));
}

static inline int dev_decode_display_timing(const struct udevice *dev,
					    int index,
					    struct display_timing *config)
{
	return ofnode_decode_display_timing(dev_ofnode(dev), index, config);
}

static inline int dev_decode_panel_timing(const struct udevice *dev,
					  struct display_timing *config)
{
	return ofnode_decode_panel_timing(dev_ofnode(dev), config);
}

static inline ofnode dev_get_phy_node(const struct udevice *dev)
{
	return ofnode_get_phy_node(dev_ofnode(dev));
}

static inline phy_interface_t dev_read_phy_mode(const struct udevice *dev)
{
	return ofnode_read_phy_mode(dev_ofnode(dev));
}

#endif /* CONFIG_DM_DEV_READ_INLINE */

/**
 * dev_for_each_subnode() - Helper function to iterate through subnodes
 *
 * This creates a for() loop which works through the subnodes in a device's
 * device-tree node.
 *
 * @subnode: ofnode holding the current subnode
 * @dev: device to use for interation (`struct udevice *`)
 */
#define dev_for_each_subnode(subnode, dev) \
	for (subnode = dev_read_first_subnode(dev); \
	     ofnode_valid(subnode); \
	     subnode = ofnode_next_subnode(subnode))

/**
 * dev_for_each_property() - Helper function to iterate through property
 *
 * This creates a for() loop which works through the property in a device's
 * device-tree node.
 *
 * @prop: struct ofprop holding the current property
 * @dev: device to use for interation (`struct udevice *`)
 */
#define dev_for_each_property(prop, dev) \
	for (int ret_prop = dev_read_first_prop(dev, &prop); \
	     !ret_prop; \
	     ret_prop = dev_read_next_prop(&prop))

#endif
