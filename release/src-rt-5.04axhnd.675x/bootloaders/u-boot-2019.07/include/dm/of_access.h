/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Originally from Linux v4.9
 * Copyright (C) 1996-2005 Paul Mackerras.
 *
 * Updates for PPC64 by Peter Bergner & David Engebretsen, IBM Corp.
 * Updates for SPARC64 by David S. Miller
 * Derived from PowerPC and Sparc prom.h files by Stephen Rothwell, IBM Corp.
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Modified for U-Boot
 * Copyright (c) 2017 Google, Inc
 */

#ifndef _DM_OF_ACCESS_H
#define _DM_OF_ACCESS_H

#include <dm/of.h>

/**
 * of_find_all_nodes - Get next node in global list
 * @prev:	Previous node or NULL to start iteration
 *		of_node_put() will be called on it
 *
 * Returns a node pointer with refcount incremented, use
 * of_node_put() on it when done.
 */
struct device_node *of_find_all_nodes(struct device_node *prev);

#define for_each_of_allnodes_from(from, dn) \
	for (dn = of_find_all_nodes(from); dn; dn = of_find_all_nodes(dn))
#define for_each_of_allnodes(dn) for_each_of_allnodes_from(NULL, dn)

/* Dummy functions to mirror Linux. These are not used in U-Boot */
#define of_node_get(x) (x)
static inline void of_node_put(const struct device_node *np) { }

/**
 * of_n_addr_cells() - Get the number of address cells for a node
 *
 * This walks back up the tree to find the closest #address-cells property
 * which controls the given node.
 *
 * @np: Node pointer to check
 * @return number of address cells this node uses
 */
int of_n_addr_cells(const struct device_node *np);

/**
 * of_n_size_cells() - Get the number of size cells for a node
 *
 * This walks back up the tree to find the closest #size-cells property
 * which controls the given node.
 *
 * @np: Node pointer to check
 * @return number of size cells this node uses
 */
int of_n_size_cells(const struct device_node *np);

/**
 * of_simple_addr_cells() - Get the address cells property in a node
 *
 * This function matches fdt_address_cells().
 *
 * @np: Node pointer to check
 * @return value of #address-cells property in this node, or 2 if none
 */
int of_simple_addr_cells(const struct device_node *np);

/**
 * of_simple_size_cells() - Get the size cells property in a node
 *
 * This function matches fdt_size_cells().
 *
 * @np: Node pointer to check
 * @return value of #size-cells property in this node, or 2 if none
 */
int of_simple_size_cells(const struct device_node *np);

/**
 * of_find_property() - find a property in a node
 *
 * @np: Pointer to device node holding property
 * @name: Name of property
 * @lenp: If non-NULL, returns length of property
 * @return pointer to property, or NULL if not found
 */
struct property *of_find_property(const struct device_node *np,
				  const char *name, int *lenp);

/**
 * of_get_property() - get a property value
 *
 * Find a property with a given name for a given node and return the value.
 *
 * @np: Pointer to device node holding property
 * @name: Name of property
 * @lenp: If non-NULL, returns length of property
 * @return pointer to property value, or NULL if not found
 */
const void *of_get_property(const struct device_node *np, const char *name,
			    int *lenp);

/**
 * of_device_is_compatible() - Check if the node matches given constraints
 * @device: pointer to node
 * @compat: required compatible string, NULL or "" for any match
 * @type: required device_type value, NULL or "" for any match
 * @name: required node name, NULL or "" for any match
 *
 * Checks if the given @compat, @type and @name strings match the
 * properties of the given @device. A constraints can be skipped by
 * passing NULL or an empty string as the constraint.
 *
 * @return 0 for no match, and a positive integer on match. The return
 * value is a relative score with larger values indicating better
 * matches. The score is weighted for the most specific compatible value
 * to get the highest score. Matching type is next, followed by matching
 * name. Practically speaking, this results in the following priority
 * order for matches:
 *
 * 1. specific compatible && type && name
 * 2. specific compatible && type
 * 3. specific compatible && name
 * 4. specific compatible
 * 5. general compatible && type && name
 * 6. general compatible && type
 * 7. general compatible && name
 * 8. general compatible
 * 9. type && name
 * 10. type
 * 11. name
 */
int of_device_is_compatible(const struct device_node *np, const char *compat,
			    const char *type, const char *name);

/**
 * of_device_is_available() - check if a device is available for use
 *
 * @device: Node to check for availability
 *
 * @return true if the status property is absent or set to "okay", false
 * otherwise
 */
bool of_device_is_available(const struct device_node *np);

/**
 * of_get_parent() - Get a node's parent, if any
 *
 * @node: Node to check
 * @eturns a node pointer, or NULL if none
 */
struct device_node *of_get_parent(const struct device_node *np);

/**
 * of_find_node_opts_by_path() - Find a node matching a full OF path
 *
 * @path: Either the full path to match, or if the path does not start with
 *	'/', the name of a property of the /aliases node (an alias). In the
 *	case of an alias, the node matching the alias' value will be returned.
 * @opts: Address of a pointer into which to store the start of an options
 *	string appended to the end of the path with a ':' separator. Can be NULL
 *
 * Valid paths:
 *	/foo/bar	Full path
 *	foo		Valid alias
 *	foo/bar		Valid alias + relative path
 *
 * @return a node pointer or NULL if not found
 */
struct device_node *of_find_node_opts_by_path(const char *path,
					      const char **opts);

static inline struct device_node *of_find_node_by_path(const char *path)
{
	return of_find_node_opts_by_path(path, NULL);
}

/**
 * of_find_compatible_node() - find a node based on its compatible string
 *
 * Find a node based on type and one of the tokens in its "compatible" property
 * @from: Node to start searching from or NULL. the node you pass will not be
 *	searched, only the next one will; typically, you pass what the previous
 *	call returned.
 * @type: The type string to match "device_type" or NULL to ignore
 * @compatible:	The string to match to one of the tokens in the device
 *	"compatible" list.
 * @return node pointer or NULL if not found
 */
struct device_node *of_find_compatible_node(struct device_node *from,
				const char *type, const char *compatible);

/**
 * of_find_node_by_prop_value() - find a node with a given property value
 *
 * Find a node based on a property value.
 * @from: Node to start searching from or NULL. the node you pass will not be
 *	searched, only the next one will; typically, you pass what the previous
 *	call returned.
 * @propname: property name to check
 * @propval: property value to search for
 * @proplen: length of the value in propval
 * @return node pointer or NULL if not found
 */
struct device_node *of_find_node_by_prop_value(struct device_node *from,
					       const char *propname,
					       const void *propval,
					       int proplen);
/**
 * of_find_node_by_phandle() - Find a node given a phandle
 *
 * @handle:	phandle of the node to find
 *
 * @return node pointer, or NULL if not found
 */
struct device_node *of_find_node_by_phandle(phandle handle);

/**
 * of_read_u32() - Find and read a 32-bit integer from a property
 *
 * Search for a property in a device node and read a 32-bit value from
 * it.
 *
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @outp:	pointer to return value, modified only if return value is 0.
 *
 * @return 0 on success, -EINVAL if the property does not exist,
 * -ENODATA if property does not have a value, and -EOVERFLOW if the
 * property data isn't large enough.
 */
int of_read_u32(const struct device_node *np, const char *propname, u32 *outp);

/**
 * of_read_u64() - Find and read a 64-bit integer from a property
 *
 * Search for a property in a device node and read a 64-bit value from
 * it.
 *
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @outp:	pointer to return value, modified only if return value is 0.
 *
 * @return 0 on success, -EINVAL if the property does not exist,
 * -ENODATA if property does not have a value, and -EOVERFLOW if the
 * property data isn't large enough.
 */
int of_read_u64(const struct device_node *np, const char *propname, u64 *outp);

/**
 * of_read_u32_array() - Find and read an array of 32 bit integers
 *
 * Search for a property in a device node and read 32-bit value(s) from
 * it.
 *
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @out_values:	pointer to return value, modified only if return value is 0.
 * @sz:		number of array elements to read
 * @return 0 on success, -EINVAL if the property does not exist, -ENODATA
 * if property does not have a value, and -EOVERFLOW is longer than sz.
 */
int of_read_u32_array(const struct device_node *np, const char *propname,
		      u32 *out_values, size_t sz);

/**
 * of_property_match_string() - Find string in a list and return index
 *
 * This function searches a string list property and returns the index
 * of a specific string value.
 *
 * @np: pointer to node containing string list property
 * @propname: string list property name
 * @string: pointer to string to search for in string list
 * @return 0 on success, -EINVAL if the property does not exist, -ENODATA
 * if property does not have a value, and -EOVERFLOW is longer than sz.
 */
int of_property_match_string(const struct device_node *np, const char *propname,
			     const char *string);

int of_property_read_string_helper(const struct device_node *np,
				   const char *propname, const char **out_strs,
				   size_t sz, int index);

/**
 * of_property_read_string_index() - Find and read a string from a multiple
 * strings property.
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @index:	index of the string in the list of strings
 * @out_string:	pointer to null terminated return string, modified only if
 *		return value is 0.
 *
 * Search for a property in a device tree node and retrieve a null
 * terminated string value (pointer to data, not a copy) in the list of strings
 * contained in that property.
 * Returns 0 on success, -EINVAL if the property does not exist, -ENODATA if
 * property does not have a value, and -EILSEQ if the string is not
 * null-terminated within the length of the property data.
 *
 * The out_string pointer is modified only if a valid string can be decoded.
 */
static inline int of_property_read_string_index(const struct device_node *np,
						const char *propname,
						int index, const char **output)
{
	int rc = of_property_read_string_helper(np, propname, output, 1, index);
	return rc < 0 ? rc : 0;
}

/**
 * of_property_count_strings() - Find and return the number of strings from a
 * multiple strings property.
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 *
 * Search for a property in a device tree node and retrieve the number of null
 * terminated string contain in it. Returns the number of strings on
 * success, -EINVAL if the property does not exist, -ENODATA if property
 * does not have a value, and -EILSEQ if the string is not null-terminated
 * within the length of the property data.
 */
static inline int of_property_count_strings(const struct device_node *np,
					    const char *propname)
{
	return of_property_read_string_helper(np, propname, NULL, 0, 0);
}

/**
 * of_parse_phandle - Resolve a phandle property to a device_node pointer
 * @np: Pointer to device node holding phandle property
 * @phandle_name: Name of property holding a phandle value
 * @index: For properties holding a table of phandles, this is the index into
 *         the table
 *
 * Returns the device_node pointer with refcount incremented.  Use
 * of_node_put() on it when done.
 */
struct device_node *of_parse_phandle(const struct device_node *np,
				     const char *phandle_name, int index);

/**
 * of_parse_phandle_with_args() - Find a node pointed by phandle in a list
 *
 * @np:		pointer to a device tree node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @index:	index of a phandle to parse out
 * @out_args:	optional pointer to output arguments structure (will be filled)
 * @return 0 on success (with @out_args filled out if not NULL), -ENOENT if
 *	@list_name does not exist, -EINVAL if a phandle was not found,
 *	@cells_name could not be found, the arguments were truncated or there
 *	were too many arguments.
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
 * of_parse_phandle_with_args(node3, "list", "#list-cells", 1, &args);
 */
int of_parse_phandle_with_args(const struct device_node *np,
			       const char *list_name, const char *cells_name,
			       int index, struct of_phandle_args *out_args);

/**
 * of_count_phandle_with_args() - Count the number of phandle in a list
 *
 * @np:		pointer to a device tree node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @return number of phandle found, -ENOENT if
 *	@list_name does not exist, -EINVAL if a phandle was not found,
 *	@cells_name could not be found, the arguments were truncated or there
 *	were too many arguments.
 *
 * Returns number of phandle found on success, on error returns appropriate
 * errno value.
 *
 */
int of_count_phandle_with_args(const struct device_node *np,
			       const char *list_name, const char *cells_name);

/**
 * of_alias_scan() - Scan all properties of the 'aliases' node
 *
 * The function scans all the properties of the 'aliases' node and populates
 * the lookup table with the properties.  It returns the number of alias
 * properties found, or an error code in case of failure.
 *
 * @return 9 if OK, -ENOMEM if not enough memory
 */
int of_alias_scan(void);

/**
 * of_alias_get_id - Get alias id for the given device_node
 *
 * Travels the lookup table to get the alias id for the given device_node and
 * alias stem.
 *
 * @np:		Pointer to the given device_node
 * @stem:	Alias stem of the given device_node
 * @return alias ID, if found, else -ENODEV
 */
int of_alias_get_id(const struct device_node *np, const char *stem);

/**
 * of_alias_get_highest_id - Get highest alias id for the given stem
 * @stem:	Alias stem to be examined
 *
 * The function travels the lookup table to get the highest alias id for the
 * given alias stem.
 * @return alias ID, if found, else -1
 */
int of_alias_get_highest_id(const char *stem);

/**
 * of_get_stdout() - Get node to use for stdout
 *
 * @return node referred to by stdout-path alias, or NULL if none
 */
struct device_node *of_get_stdout(void);

#endif
