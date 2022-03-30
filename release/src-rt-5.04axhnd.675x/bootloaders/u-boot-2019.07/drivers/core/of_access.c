// SPDX-License-Identifier: GPL-2.0+
/*
 * Originally from Linux v4.9
 * Paul Mackerras	August 1996.
 * Copyright (C) 1996-2005 Paul Mackerras.
 *
 * Adapted for 64bit PowerPC by Dave Engebretsen and Peter Bergner.
 *   {engebret|bergner}@us.ibm.com
 *
 * Adapted for sparc and sparc64 by David S. Miller davem@davemloft.net
 *
 * Reconsolidated from arch/x/kernel/prom.c by Stephen Rothwell and
 * Grant Likely.
 *
 * Modified for U-Boot
 * Copyright (c) 2017 Google, Inc
 *
 * This file follows drivers/of/base.c with functions in the same order as the
 * Linux version.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <dm/of_access.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/ioport.h>

DECLARE_GLOBAL_DATA_PTR;

/* list of struct alias_prop aliases */
LIST_HEAD(aliases_lookup);

/* "/aliaes" node */
static struct device_node *of_aliases;

/* "/chosen" node */
static struct device_node *of_chosen;

/* node pointed to by the stdout-path alias */
static struct device_node *of_stdout;

/* pointer to options given after the alias (separated by :) or NULL if none */
static const char *of_stdout_options;

/**
 * struct alias_prop - Alias property in 'aliases' node
 *
 * The structure represents one alias property of 'aliases' node as
 * an entry in aliases_lookup list.
 *
 * @link:	List node to link the structure in aliases_lookup list
 * @alias:	Alias property name
 * @np:		Pointer to device_node that the alias stands for
 * @id:		Index value from end of alias name
 * @stem:	Alias string without the index
 */
struct alias_prop {
	struct list_head link;
	const char *alias;
	struct device_node *np;
	int id;
	char stem[0];
};

int of_n_addr_cells(const struct device_node *np)
{
	const __be32 *ip;

	do {
		if (np->parent)
			np = np->parent;
		ip = of_get_property(np, "#address-cells", NULL);
		if (ip)
			return be32_to_cpup(ip);
	} while (np->parent);

	/* No #address-cells property for the root node */
	return OF_ROOT_NODE_ADDR_CELLS_DEFAULT;
}

int of_n_size_cells(const struct device_node *np)
{
	const __be32 *ip;

	do {
		if (np->parent)
			np = np->parent;
		ip = of_get_property(np, "#size-cells", NULL);
		if (ip)
			return be32_to_cpup(ip);
	} while (np->parent);

	/* No #size-cells property for the root node */
	return OF_ROOT_NODE_SIZE_CELLS_DEFAULT;
}

int of_simple_addr_cells(const struct device_node *np)
{
	const __be32 *ip;

	ip = of_get_property(np, "#address-cells", NULL);
	if (ip)
		return be32_to_cpup(ip);

	/* Return a default of 2 to match fdt_address_cells()*/
	return 2;
}

int of_simple_size_cells(const struct device_node *np)
{
	const __be32 *ip;

	ip = of_get_property(np, "#size-cells", NULL);
	if (ip)
		return be32_to_cpup(ip);

	/* Return a default of 2 to match fdt_size_cells()*/
	return 2;
}

struct property *of_find_property(const struct device_node *np,
				  const char *name, int *lenp)
{
	struct property *pp;

	if (!np)
		return NULL;

	for (pp = np->properties; pp; pp = pp->next) {
		if (strcmp(pp->name, name) == 0) {
			if (lenp)
				*lenp = pp->length;
			break;
		}
	}
	if (!pp && lenp)
		*lenp = -FDT_ERR_NOTFOUND;

	return pp;
}

struct device_node *of_find_all_nodes(struct device_node *prev)
{
	struct device_node *np;

	if (!prev) {
		np = gd->of_root;
	} else if (prev->child) {
		np = prev->child;
	} else {
		/*
		 * Walk back up looking for a sibling, or the end of the
		 * structure
		 */
		np = prev;
		while (np->parent && !np->sibling)
			np = np->parent;
		np = np->sibling; /* Might be null at the end of the tree */
	}

	return np;
}

const void *of_get_property(const struct device_node *np, const char *name,
			    int *lenp)
{
	struct property *pp = of_find_property(np, name, lenp);

	return pp ? pp->value : NULL;
}

static const char *of_prop_next_string(struct property *prop, const char *cur)
{
	const void *curv = cur;

	if (!prop)
		return NULL;

	if (!cur)
		return prop->value;

	curv += strlen(cur) + 1;
	if (curv >= prop->value + prop->length)
		return NULL;

	return curv;
}

int of_device_is_compatible(const struct device_node *device,
			    const char *compat, const char *type,
			    const char *name)
{
	struct property *prop;
	const char *cp;
	int index = 0, score = 0;

	/* Compatible match has highest priority */
	if (compat && compat[0]) {
		prop = of_find_property(device, "compatible", NULL);
		for (cp = of_prop_next_string(prop, NULL); cp;
		     cp = of_prop_next_string(prop, cp), index++) {
			if (of_compat_cmp(cp, compat, strlen(compat)) == 0) {
				score = INT_MAX/2 - (index << 2);
				break;
			}
		}
		if (!score)
			return 0;
	}

	/* Matching type is better than matching name */
	if (type && type[0]) {
		if (!device->type || of_node_cmp(type, device->type))
			return 0;
		score += 2;
	}

	/* Matching name is a bit better than not */
	if (name && name[0]) {
		if (!device->name || of_node_cmp(name, device->name))
			return 0;
		score++;
	}

	return score;
}

bool of_device_is_available(const struct device_node *device)
{
	const char *status;
	int statlen;

	if (!device)
		return false;

	status = of_get_property(device, "status", &statlen);
	if (status == NULL)
		return true;

	if (statlen > 0) {
		if (!strcmp(status, "okay"))
			return true;
	}

	return false;
}

struct device_node *of_get_parent(const struct device_node *node)
{
	const struct device_node *np;

	if (!node)
		return NULL;

	np = of_node_get(node->parent);

	return (struct device_node *)np;
}

static struct device_node *__of_get_next_child(const struct device_node *node,
					       struct device_node *prev)
{
	struct device_node *next;

	if (!node)
		return NULL;

	next = prev ? prev->sibling : node->child;
	/*
	 * coverity[dead_error_line : FALSE]
	 * Dead code here since our current implementation of of_node_get()
	 * always returns NULL (Coverity CID 163245). But we leave it as is
	 * since we may want to implement get/put later.
	 */
	for (; next; next = next->sibling)
		if (of_node_get(next))
			break;
	of_node_put(prev);
	return next;
}

#define __for_each_child_of_node(parent, child) \
	for (child = __of_get_next_child(parent, NULL); child != NULL; \
	     child = __of_get_next_child(parent, child))

static struct device_node *__of_find_node_by_path(struct device_node *parent,
						  const char *path)
{
	struct device_node *child;
	int len;

	len = strcspn(path, "/:");
	if (!len)
		return NULL;

	__for_each_child_of_node(parent, child) {
		const char *name = strrchr(child->full_name, '/');

		name++;
		if (strncmp(path, name, len) == 0 && (strlen(name) == len))
			return child;
	}
	return NULL;
}

#define for_each_property_of_node(dn, pp) \
	for (pp = dn->properties; pp != NULL; pp = pp->next)

struct device_node *of_find_node_opts_by_path(const char *path,
					      const char **opts)
{
	struct device_node *np = NULL;
	struct property *pp;
	const char *separator = strchr(path, ':');

	if (opts)
		*opts = separator ? separator + 1 : NULL;

	if (strcmp(path, "/") == 0)
		return of_node_get(gd->of_root);

	/* The path could begin with an alias */
	if (*path != '/') {
		int len;
		const char *p = separator;

		if (!p)
			p = strchrnul(path, '/');
		len = p - path;

		/* of_aliases must not be NULL */
		if (!of_aliases)
			return NULL;

		for_each_property_of_node(of_aliases, pp) {
			if (strlen(pp->name) == len && !strncmp(pp->name, path,
								len)) {
				np = of_find_node_by_path(pp->value);
				break;
			}
		}
		if (!np)
			return NULL;
		path = p;
	}

	/* Step down the tree matching path components */
	if (!np)
		np = of_node_get(gd->of_root);
	while (np && *path == '/') {
		struct device_node *tmp = np;

		path++; /* Increment past '/' delimiter */
		np = __of_find_node_by_path(np, path);
		of_node_put(tmp);
		path = strchrnul(path, '/');
		if (separator && separator < path)
			break;
	}

	return np;
}

struct device_node *of_find_compatible_node(struct device_node *from,
		const char *type, const char *compatible)
{
	struct device_node *np;

	for_each_of_allnodes_from(from, np)
		if (of_device_is_compatible(np, compatible, type, NULL) &&
		    of_node_get(np))
			break;
	of_node_put(from);

	return np;
}

static int of_device_has_prop_value(const struct device_node *device,
				    const char *propname, const void *propval,
				    int proplen)
{
	struct property *prop = of_find_property(device, propname, NULL);

	if (!prop || !prop->value || prop->length != proplen)
		return 0;
	return !memcmp(prop->value, propval, proplen);
}

struct device_node *of_find_node_by_prop_value(struct device_node *from,
					       const char *propname,
					       const void *propval, int proplen)
{
	struct device_node *np;

	for_each_of_allnodes_from(from, np) {
		if (of_device_has_prop_value(np, propname, propval, proplen) &&
		    of_node_get(np))
			break;
	}
	of_node_put(from);

	return np;
}

struct device_node *of_find_node_by_phandle(phandle handle)
{
	struct device_node *np;

	if (!handle)
		return NULL;

	for_each_of_allnodes(np)
		if (np->phandle == handle)
			break;
	(void)of_node_get(np);

	return np;
}

/**
 * of_find_property_value_of_size() - find property of given size
 *
 * Search for a property in a device node and validate the requested size.
 *
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @len:	requested length of property value
 *
 * @return the property value on success, -EINVAL if the property does not
 * exist, -ENODATA if property does not have a value, and -EOVERFLOW if the
 * property data isn't large enough.
 */
static void *of_find_property_value_of_size(const struct device_node *np,
					    const char *propname, u32 len)
{
	struct property *prop = of_find_property(np, propname, NULL);

	if (!prop)
		return ERR_PTR(-EINVAL);
	if (!prop->value)
		return ERR_PTR(-ENODATA);
	if (len > prop->length)
		return ERR_PTR(-EOVERFLOW);

	return prop->value;
}

int of_read_u32(const struct device_node *np, const char *propname, u32 *outp)
{
	const __be32 *val;

	debug("%s: %s: ", __func__, propname);
	if (!np)
		return -EINVAL;
	val = of_find_property_value_of_size(np, propname, sizeof(*outp));
	if (IS_ERR(val)) {
		debug("(not found)\n");
		return PTR_ERR(val);
	}

	*outp = be32_to_cpup(val);
	debug("%#x (%d)\n", *outp, *outp);

	return 0;
}

int of_read_u32_array(const struct device_node *np, const char *propname,
		      u32 *out_values, size_t sz)
{
	const __be32 *val;

	debug("%s: %s: ", __func__, propname);
	val = of_find_property_value_of_size(np, propname,
					     sz * sizeof(*out_values));

	if (IS_ERR(val))
		return PTR_ERR(val);

	debug("size %zd\n", sz);
	while (sz--)
		*out_values++ = be32_to_cpup(val++);

	return 0;
}

int of_read_u64(const struct device_node *np, const char *propname, u64 *outp)
{
	const __be64 *val;

	debug("%s: %s: ", __func__, propname);
	if (!np)
		return -EINVAL;
	val = of_find_property_value_of_size(np, propname, sizeof(*outp));
	if (IS_ERR(val)) {
		debug("(not found)\n");
		return PTR_ERR(val);
	}

	*outp = be64_to_cpup(val);
	debug("%#llx (%lld)\n", (unsigned long long)*outp,
              (unsigned long long)*outp);

	return 0;
}

int of_property_match_string(const struct device_node *np, const char *propname,
			     const char *string)
{
	const struct property *prop = of_find_property(np, propname, NULL);
	size_t l;
	int i;
	const char *p, *end;

	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;

	p = prop->value;
	end = p + prop->length;

	for (i = 0; p < end; i++, p += l) {
		l = strnlen(p, end - p) + 1;
		if (p + l > end)
			return -EILSEQ;
		debug("comparing %s with %s\n", string, p);
		if (strcmp(string, p) == 0)
			return i; /* Found it; return index */
	}
	return -ENODATA;
}

/**
 * of_property_read_string_helper() - Utility helper for parsing string properties
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @out_strs:	output array of string pointers.
 * @sz:		number of array elements to read.
 * @skip:	Number of strings to skip over at beginning of list.
 *
 * Don't call this function directly. It is a utility helper for the
 * of_property_read_string*() family of functions.
 */
int of_property_read_string_helper(const struct device_node *np,
				   const char *propname, const char **out_strs,
				   size_t sz, int skip)
{
	const struct property *prop = of_find_property(np, propname, NULL);
	int l = 0, i = 0;
	const char *p, *end;

	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;
	p = prop->value;
	end = p + prop->length;

	for (i = 0; p < end && (!out_strs || i < skip + sz); i++, p += l) {
		l = strnlen(p, end - p) + 1;
		if (p + l > end)
			return -EILSEQ;
		if (out_strs && i >= skip)
			*out_strs++ = p;
	}
	i -= skip;
	return i <= 0 ? -ENODATA : i;
}

static int __of_parse_phandle_with_args(const struct device_node *np,
					const char *list_name,
					const char *cells_name,
					int cell_count, int index,
					struct of_phandle_args *out_args)
{
	const __be32 *list, *list_end;
	int rc = 0, cur_index = 0;
	uint32_t count = 0;
	struct device_node *node = NULL;
	phandle phandle;
	int size;

	/* Retrieve the phandle list property */
	list = of_get_property(np, list_name, &size);
	if (!list)
		return -ENOENT;
	list_end = list + size / sizeof(*list);

	/* Loop over the phandles until all the requested entry is found */
	while (list < list_end) {
		rc = -EINVAL;
		count = 0;

		/*
		 * If phandle is 0, then it is an empty entry with no
		 * arguments.  Skip forward to the next entry.
		 */
		phandle = be32_to_cpup(list++);
		if (phandle) {
			/*
			 * Find the provider node and parse the #*-cells
			 * property to determine the argument length.
			 *
			 * This is not needed if the cell count is hard-coded
			 * (i.e. cells_name not set, but cell_count is set),
			 * except when we're going to return the found node
			 * below.
			 */
			if (cells_name || cur_index == index) {
				node = of_find_node_by_phandle(phandle);
				if (!node) {
					debug("%s: could not find phandle\n",
					      np->full_name);
					goto err;
				}
			}

			if (cells_name) {
				if (of_read_u32(node, cells_name, &count)) {
					debug("%s: could not get %s for %s\n",
					      np->full_name, cells_name,
					      node->full_name);
					goto err;
				}
			} else {
				count = cell_count;
			}

			/*
			 * Make sure that the arguments actually fit in the
			 * remaining property data length
			 */
			if (list + count > list_end) {
				debug("%s: arguments longer than property\n",
				      np->full_name);
				goto err;
			}
		}

		/*
		 * All of the error cases above bail out of the loop, so at
		 * this point, the parsing is successful. If the requested
		 * index matches, then fill the out_args structure and return,
		 * or return -ENOENT for an empty entry.
		 */
		rc = -ENOENT;
		if (cur_index == index) {
			if (!phandle)
				goto err;

			if (out_args) {
				int i;
				if (WARN_ON(count > OF_MAX_PHANDLE_ARGS))
					count = OF_MAX_PHANDLE_ARGS;
				out_args->np = node;
				out_args->args_count = count;
				for (i = 0; i < count; i++)
					out_args->args[i] =
							be32_to_cpup(list++);
			} else {
				of_node_put(node);
			}

			/* Found it! return success */
			return 0;
		}

		of_node_put(node);
		node = NULL;
		list += count;
		cur_index++;
	}

	/*
	 * Unlock node before returning result; will be one of:
	 * -ENOENT : index is for empty phandle
	 * -EINVAL : parsing error on data
	 * [1..n]  : Number of phandle (count mode; when index = -1)
	 */
	rc = index < 0 ? cur_index : -ENOENT;
 err:
	if (node)
		of_node_put(node);
	return rc;
}

struct device_node *of_parse_phandle(const struct device_node *np,
				     const char *phandle_name, int index)
{
	struct of_phandle_args args;

	if (index < 0)
		return NULL;

	if (__of_parse_phandle_with_args(np, phandle_name, NULL, 0, index,
					 &args))
		return NULL;

	return args.np;
}

int of_parse_phandle_with_args(const struct device_node *np,
			       const char *list_name, const char *cells_name,
			       int index, struct of_phandle_args *out_args)
{
	if (index < 0)
		return -EINVAL;

	return __of_parse_phandle_with_args(np, list_name, cells_name, 0,
					    index, out_args);
}

int of_count_phandle_with_args(const struct device_node *np,
			       const char *list_name, const char *cells_name)
{
	return __of_parse_phandle_with_args(np, list_name, cells_name, 0,
					    -1, NULL);
}

static void of_alias_add(struct alias_prop *ap, struct device_node *np,
			 int id, const char *stem, int stem_len)
{
	ap->np = np;
	ap->id = id;
	strncpy(ap->stem, stem, stem_len);
	ap->stem[stem_len] = 0;
	list_add_tail(&ap->link, &aliases_lookup);
	debug("adding DT alias:%s: stem=%s id=%i node=%s\n",
	      ap->alias, ap->stem, ap->id, of_node_full_name(np));
}

int of_alias_scan(void)
{
	struct property *pp;

	of_aliases = of_find_node_by_path("/aliases");
	of_chosen = of_find_node_by_path("/chosen");
	if (of_chosen == NULL)
		of_chosen = of_find_node_by_path("/chosen@0");

	if (of_chosen) {
		const char *name;

		name = of_get_property(of_chosen, "stdout-path", NULL);
		if (name)
			of_stdout = of_find_node_opts_by_path(name,
							&of_stdout_options);
	}

	if (!of_aliases)
		return 0;

	for_each_property_of_node(of_aliases, pp) {
		const char *start = pp->name;
		const char *end = start + strlen(start);
		struct device_node *np;
		struct alias_prop *ap;
		ulong id;
		int len;

		/* Skip those we do not want to proceed */
		if (!strcmp(pp->name, "name") ||
		    !strcmp(pp->name, "phandle") ||
		    !strcmp(pp->name, "linux,phandle"))
			continue;

		np = of_find_node_by_path(pp->value);
		if (!np)
			continue;

		/*
		 * walk the alias backwards to extract the id and work out
		 * the 'stem' string
		 */
		while (isdigit(*(end-1)) && end > start)
			end--;
		len = end - start;

		if (strict_strtoul(end, 10, &id) < 0)
			continue;

		/* Allocate an alias_prop with enough space for the stem */
		ap = malloc(sizeof(*ap) + len + 1);
		if (!ap)
			return -ENOMEM;
		memset(ap, 0, sizeof(*ap) + len + 1);
		ap->alias = start;
		of_alias_add(ap, np, id, start, len);
	}

	return 0;
}

int of_alias_get_id(const struct device_node *np, const char *stem)
{
	struct alias_prop *app;
	int id = -ENODEV;

	mutex_lock(&of_mutex);
	list_for_each_entry(app, &aliases_lookup, link) {
		if (strcmp(app->stem, stem) != 0)
			continue;

		if (np == app->np) {
			id = app->id;
			break;
		}
	}
	mutex_unlock(&of_mutex);

	return id;
}

int of_alias_get_highest_id(const char *stem)
{
	struct alias_prop *app;
	int id = -1;

	mutex_lock(&of_mutex);
	list_for_each_entry(app, &aliases_lookup, link) {
		if (strcmp(app->stem, stem) != 0)
			continue;

		if (app->id > id)
			id = app->id;
	}
	mutex_unlock(&of_mutex);

	return id;
}

struct device_node *of_get_stdout(void)
{
	return of_stdout;
}
