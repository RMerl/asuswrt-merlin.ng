// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_DT

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <log.h>
#include <malloc.h>
#include <of_live.h>
#include <linux/libfdt.h>
#include <dm/of_access.h>
#include <dm/of_addr.h>
#include <dm/ofnode.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(OFNODE_MULTI_TREE)
static void *oftree_list[CONFIG_OFNODE_MULTI_TREE_MAX];
static int oftree_count;

void oftree_reset(void)
{
	if (gd->flags & GD_FLG_RELOC) {
		oftree_count = 0;
		oftree_list[oftree_count++] = (void *)gd->fdt_blob;
	}
}

static int oftree_find(const void *fdt)
{
	int i;

	for (i = 0; i < oftree_count; i++) {
		if (fdt == oftree_list[i])
			return i;
	}

	return -1;
}

static int check_tree_count(void)
{
	if (oftree_count == CONFIG_OFNODE_MULTI_TREE_MAX) {
		log_warning("Too many registered device trees (max %d)\n",
			    CONFIG_OFNODE_MULTI_TREE_MAX);
		return -E2BIG;
	}

	return 0;
}

static oftree oftree_ensure(void *fdt)
{
	oftree tree;
	int i;

	if (of_live_active()) {
		struct device_node *root;
		int ret;

		ret = unflatten_device_tree(fdt, &root);
		if (ret) {
			log_err("Failed to create live tree: err=%d\n", ret);
			return oftree_null();
		}
		tree = oftree_from_np(root);

		return tree;
	}

	if (gd->flags & GD_FLG_RELOC) {
		i = oftree_find(fdt);
		if (i == -1) {
			if (check_tree_count())
				return oftree_null();

			if (fdt_check_header(fdt)) {
				log_err("Invalid device tree blob header\n");
				return oftree_null();
			}

			/* register the new tree */
			i = oftree_count++;
			oftree_list[i] = fdt;
			log_debug("oftree: registered tree %d: %p\n", i, fdt);
		}
	} else {
		if (fdt != gd->fdt_blob) {
			log_debug("Only the control FDT can be accessed before relocation\n");
			return oftree_null();
		}
	}

	tree.fdt = fdt;

	return tree;
}

int oftree_new(oftree *treep)
{
	oftree tree = oftree_null();
	int ret;

	if (of_live_active()) {
		struct device_node *root;

		ret = of_live_create_empty(&root);
		if (ret)
			return log_msg_ret("liv", ret);
		tree = oftree_from_np(root);
	} else {
		const int size = 1024;
		void *fdt;

		ret = check_tree_count();
		if (ret)
			return log_msg_ret("fla", ret);

		/* register the new tree with a small size */
		fdt = malloc(size);
		if (!fdt)
			return log_msg_ret("fla", -ENOMEM);
		ret = fdt_create_empty_tree(fdt, size);
		if (ret)
			return log_msg_ret("fla", -EINVAL);
		oftree_list[oftree_count++] = fdt;
		tree.fdt = fdt;
	}
	*treep = tree;

	return 0;
}

void oftree_dispose(oftree tree)
{
	if (of_live_active())
		of_live_free(tree.np);
}

void *ofnode_lookup_fdt(ofnode node)
{
	if (gd->flags & GD_FLG_RELOC) {
		uint i = OFTREE_TREE_ID(node.of_offset);

		if (i >= oftree_count) {
			log_debug("Invalid tree ID %x\n", i);
			return NULL;
		}

		return oftree_list[i];
	} else {
		return (void *)gd->fdt_blob;
	}
}

void *ofnode_to_fdt(ofnode node)
{
#ifdef OF_CHECKS
	if (of_live_active())
		return NULL;
#endif
	if (CONFIG_IS_ENABLED(OFNODE_MULTI_TREE) && ofnode_valid(node))
		return ofnode_lookup_fdt(node);

	/* Use the control FDT by default */
	return (void *)gd->fdt_blob;
}

/**
 * ofnode_to_offset() - convert an ofnode to a flat DT offset
 *
 * This cannot be called if the reference contains a node pointer.
 *
 * @node: Reference containing offset (possibly invalid)
 * Return: DT offset (can be -1)
 */
int ofnode_to_offset(ofnode node)
{
#ifdef OF_CHECKS
	if (of_live_active())
		return -1;
#endif
	if (CONFIG_IS_ENABLED(OFNODE_MULTI_TREE) && node.of_offset >= 0)
		return OFTREE_OFFSET(node.of_offset);

	return node.of_offset;
}

oftree oftree_from_fdt(void *fdt)
{
	oftree tree;

	if (CONFIG_IS_ENABLED(OFNODE_MULTI_TREE))
		return oftree_ensure(fdt);

#ifdef OF_CHECKS
	if (of_live_active())
		return oftree_null();
#endif
	tree.fdt = fdt;

	return tree;
}

/**
 * noffset_to_ofnode() - convert a DT offset to an ofnode
 *
 * @other_node: Node in the same tree to use as a reference
 * @of_offset: DT offset (either valid, or -1)
 * Return: reference to the associated DT offset
 */
ofnode noffset_to_ofnode(ofnode other_node, int of_offset)
{
	ofnode node;

	if (of_live_active())
		node.np = NULL;
	else if (!CONFIG_IS_ENABLED(OFNODE_MULTI_TREE) || of_offset < 0 ||
		 !ofnode_valid(other_node))
		node.of_offset = of_offset;
	else
		node.of_offset = OFTREE_MAKE_NODE(other_node.of_offset,
						  of_offset);

	return node;
}

#else /* !OFNODE_MULTI_TREE */

static inline int oftree_find(const void *fdt)
{
	return 0;
}

int oftree_new(oftree *treep)
{
	return -ENOSYS;
}

#endif /* OFNODE_MULTI_TREE */

int oftree_to_fdt(oftree tree, struct abuf *buf)
{
	int ret;

	if (of_live_active()) {
		ret = of_live_flatten(ofnode_to_np(oftree_root(tree)), buf);
		if (ret)
			return log_msg_ret("flt", ret);
	} else {
		void *fdt = oftree_lookup_fdt(tree);

		abuf_init(buf);
		abuf_set(buf, fdt, fdt_totalsize(fdt));
	}

	return 0;
}

/**
 * ofnode_from_tree_offset() - get an ofnode from a tree offset (flat tree)
 *
 * Looks up the tree and returns an ofnode with the correct of_offset (i.e.
 * containing the tree ID).
 *
 * If @offset is < 0 then this returns an ofnode with that offset and no tree
 * ID.
 *
 * @tree: tree to check
 * @offset: offset within that tree (can be < 0)
 * @return node for that offset, with the correct ID
 */
static ofnode ofnode_from_tree_offset(oftree tree, int offset)
{
	ofnode node;

	if (CONFIG_IS_ENABLED(OFNODE_MULTI_TREE) && offset >= 0) {
		int tree_id = oftree_find(tree.fdt);

		if (tree_id == -1)
			return ofnode_null();
		node.of_offset = OFTREE_NODE(tree_id, offset);
	} else {
		node.of_offset = offset;
	}

	return node;
}

bool ofnode_name_eq(ofnode node, const char *name)
{
	const char *node_name;
	size_t len;

	assert(ofnode_valid(node));

	node_name = ofnode_get_name(node);
	len = strchrnul(node_name, '@') - node_name;

	return (strlen(name) == len) && !strncmp(node_name, name, len);
}

int ofnode_read_u8(ofnode node, const char *propname, u8 *outp)
{
	const u8 *cell;
	int len;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node))
		return of_read_u8(ofnode_to_np(node), propname, outp);

	cell = fdt_getprop(gd->fdt_blob, ofnode_to_offset(node), propname,
			   &len);
	if (!cell || len < sizeof(*cell)) {
		debug("(not found)\n");
		return -EINVAL;
	}
	*outp = *cell;
	debug("%#x (%d)\n", *outp, *outp);

	return 0;
}

u8 ofnode_read_u8_default(ofnode node, const char *propname, u8 def)
{
	assert(ofnode_valid(node));
	ofnode_read_u8(node, propname, &def);

	return def;
}

int ofnode_read_u16(ofnode node, const char *propname, u16 *outp)
{
	const fdt16_t *cell;
	int len;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node))
		return of_read_u16(ofnode_to_np(node), propname, outp);

	cell = fdt_getprop(gd->fdt_blob, ofnode_to_offset(node), propname,
			   &len);
	if (!cell || len < sizeof(*cell)) {
		debug("(not found)\n");
		return -EINVAL;
	}
	*outp = be16_to_cpup(cell);
	debug("%#x (%d)\n", *outp, *outp);

	return 0;
}

u16 ofnode_read_u16_default(ofnode node, const char *propname, u16 def)
{
	assert(ofnode_valid(node));
	ofnode_read_u16(node, propname, &def);

	return def;
}

int ofnode_read_u32(ofnode node, const char *propname, u32 *outp)
{
	return ofnode_read_u32_index(node, propname, 0, outp);
}

u32 ofnode_read_u32_default(ofnode node, const char *propname, u32 def)
{
	assert(ofnode_valid(node));
	ofnode_read_u32_index(node, propname, 0, &def);

	return def;
}

int ofnode_read_u32_index(ofnode node, const char *propname, int index,
			  u32 *outp)
{
	const fdt32_t *cell;
	int len;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node))
		return of_read_u32_index(ofnode_to_np(node), propname, index,
					 outp);

	cell = fdt_getprop(ofnode_to_fdt(node), ofnode_to_offset(node),
			   propname, &len);
	if (!cell) {
		debug("(not found)\n");
		return -EINVAL;
	}

	if (len < (sizeof(int) * (index + 1))) {
		debug("(not large enough)\n");
		return -EOVERFLOW;
	}

	*outp = fdt32_to_cpu(cell[index]);
	debug("%#x (%d)\n", *outp, *outp);

	return 0;
}

int ofnode_read_u64_index(ofnode node, const char *propname, int index,
			  u64 *outp)
{
	const fdt64_t *cell;
	int len;

	assert(ofnode_valid(node));

	if (ofnode_is_np(node))
		return of_read_u64_index(ofnode_to_np(node), propname, index,
					 outp);

	cell = fdt_getprop(ofnode_to_fdt(node), ofnode_to_offset(node),
			   propname, &len);
	if (!cell) {
		debug("(not found)\n");
		return -EINVAL;
	}

	if (len < (sizeof(u64) * (index + 1))) {
		debug("(not large enough)\n");
		return -EOVERFLOW;
	}

	*outp = fdt64_to_cpu(cell[index]);
	debug("%#llx (%lld)\n", *outp, *outp);

	return 0;
}

u32 ofnode_read_u32_index_default(ofnode node, const char *propname, int index,
				  u32 def)
{
	assert(ofnode_valid(node));
	ofnode_read_u32_index(node, propname, index, &def);

	return def;
}

int ofnode_read_s32_default(ofnode node, const char *propname, s32 def)
{
	assert(ofnode_valid(node));
	ofnode_read_u32(node, propname, (u32 *)&def);

	return def;
}

int ofnode_read_u64(ofnode node, const char *propname, u64 *outp)
{
	const unaligned_fdt64_t *cell;
	int len;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node))
		return of_read_u64(ofnode_to_np(node), propname, outp);

	cell = fdt_getprop(ofnode_to_fdt(node), ofnode_to_offset(node),
			   propname, &len);
	if (!cell || len < sizeof(*cell)) {
		debug("(not found)\n");
		return -EINVAL;
	}
	*outp = fdt64_to_cpu(cell[0]);
	debug("%#llx (%lld)\n", (unsigned long long)*outp,
	      (unsigned long long)*outp);

	return 0;
}

u64 ofnode_read_u64_default(ofnode node, const char *propname, u64 def)
{
	assert(ofnode_valid(node));
	ofnode_read_u64(node, propname, &def);

	return def;
}

bool ofnode_read_bool(ofnode node, const char *propname)
{
	bool prop;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	prop = ofnode_has_property(node, propname);

	debug("%s\n", prop ? "true" : "false");

	return prop ? true : false;
}

const void *ofnode_read_prop(ofnode node, const char *propname, int *sizep)
{
	const char *val = NULL;
	int len;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node)) {
		struct property *prop = of_find_property(
				ofnode_to_np(node), propname, &len);

		if (prop) {
			val = prop->value;
			len = prop->length;
		}
	} else {
		val = fdt_getprop(ofnode_to_fdt(node), ofnode_to_offset(node),
				  propname, &len);
	}
	if (!val) {
		debug("<not found>\n");
		if (sizep)
			*sizep = -FDT_ERR_NOTFOUND;
		return NULL;
	}
	if (sizep)
		*sizep = len;

	return val;
}

const char *ofnode_read_string(ofnode node, const char *propname)
{
	const char *str;
	int len;

	str = ofnode_read_prop(node, propname, &len);
	if (!str)
		return NULL;

	if (strnlen(str, len) >= len) {
		debug("<invalid>\n");
		return NULL;
	}
	debug("%s\n", str);

	return str;
}

int ofnode_read_size(ofnode node, const char *propname)
{
	int len;

	if (!ofnode_read_prop(node, propname, &len))
		return -EINVAL;

	return len;
}

ofnode ofnode_find_subnode(ofnode node, const char *subnode_name)
{
	ofnode subnode;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, subnode_name);

	if (ofnode_is_np(node)) {
		struct device_node *np = ofnode_to_np(node);

		for (np = np->child; np; np = np->sibling) {
			if (!strcmp(subnode_name, np->name))
				break;
		}
		subnode = np_to_ofnode(np);
	} else {
		int ooffset = fdt_subnode_offset(ofnode_to_fdt(node),
				ofnode_to_offset(node), subnode_name);
		subnode = noffset_to_ofnode(node, ooffset);
	}
	debug("%s\n", ofnode_valid(subnode) ?
	      ofnode_get_name(subnode) : "<none>");

	return subnode;
}

int ofnode_read_u32_array(ofnode node, const char *propname,
			  u32 *out_values, size_t sz)
{
	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node)) {
		return of_read_u32_array(ofnode_to_np(node), propname,
					 out_values, sz);
	} else {
		int ret;

		ret = fdtdec_get_int_array(ofnode_to_fdt(node),
					   ofnode_to_offset(node), propname,
					   out_values, sz);

		/* get the error right, but space is more important in SPL */
		if (!IS_ENABLED(CONFIG_SPL_BUILD)) {
			if (ret == -FDT_ERR_NOTFOUND)
				return -EINVAL;
			else if (ret == -FDT_ERR_BADLAYOUT)
				return -EOVERFLOW;
		}
		return ret;
	}
}

#if !CONFIG_IS_ENABLED(DM_INLINE_OFNODE)
bool ofnode_is_enabled(ofnode node)
{
	if (ofnode_is_np(node)) {
		return of_device_is_available(ofnode_to_np(node));
	} else {
		return fdtdec_get_is_enabled(ofnode_to_fdt(node),
					     ofnode_to_offset(node));
	}
}

ofnode ofnode_first_subnode(ofnode node)
{
	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		return np_to_ofnode(node.np->child);

	return noffset_to_ofnode(node,
		fdt_first_subnode(ofnode_to_fdt(node), ofnode_to_offset(node)));
}

ofnode ofnode_next_subnode(ofnode node)
{
	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		return np_to_ofnode(node.np->sibling);

	return noffset_to_ofnode(node,
		fdt_next_subnode(ofnode_to_fdt(node), ofnode_to_offset(node)));
}
#endif /* !DM_INLINE_OFNODE */

ofnode ofnode_get_parent(ofnode node)
{
	ofnode parent;

	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		parent = np_to_ofnode(of_get_parent(ofnode_to_np(node)));
	else
		parent.of_offset = fdt_parent_offset(ofnode_to_fdt(node),
						     ofnode_to_offset(node));

	return parent;
}

const char *ofnode_get_name(ofnode node)
{
	if (!ofnode_valid(node)) {
		debug("%s node not valid\n", __func__);
		return NULL;
	}

	if (ofnode_is_np(node))
		return node.np->name;

	return fdt_get_name(ofnode_to_fdt(node), ofnode_to_offset(node), NULL);
}

int ofnode_get_path(ofnode node, char *buf, int buflen)
{
	assert(ofnode_valid(node));

	if (ofnode_is_np(node)) {
		if (strlen(node.np->full_name) >= buflen)
			return -ENOSPC;

		strcpy(buf, node.np->full_name);

		return 0;
	} else {
		int res;

		res = fdt_get_path(ofnode_to_fdt(node), ofnode_to_offset(node), buf,
				   buflen);
		if (!res)
			return res;
		else if (res == -FDT_ERR_NOSPACE)
			return -ENOSPC;
		else
			return -EINVAL;
	}
}

ofnode ofnode_get_by_phandle(uint phandle)
{
	ofnode node;

	if (of_live_active())
		node = np_to_ofnode(of_find_node_by_phandle(NULL, phandle));
	else
		node.of_offset = fdt_node_offset_by_phandle(gd->fdt_blob,
							    phandle);

	return node;
}

ofnode oftree_get_by_phandle(oftree tree, uint phandle)
{
	ofnode node;

	if (of_live_active())
		node = np_to_ofnode(of_find_node_by_phandle(tree.np, phandle));
	else
		node = ofnode_from_tree_offset(tree,
			fdt_node_offset_by_phandle(oftree_lookup_fdt(tree),
						   phandle));

	return node;
}

static fdt_addr_t __ofnode_get_addr_size_index(ofnode node, int index,
					       fdt_size_t *size, bool translate)
{
	int na, ns;

	if (size)
		*size = FDT_SIZE_T_NONE;

	if (ofnode_is_np(node)) {
		const __be32 *prop_val;
		u64 size64;
		uint flags;

		prop_val = of_get_address(ofnode_to_np(node), index, &size64,
					  &flags);
		if (!prop_val)
			return FDT_ADDR_T_NONE;

		if (size)
			*size = size64;

		ns = of_n_size_cells(ofnode_to_np(node));

		if (translate && IS_ENABLED(CONFIG_OF_TRANSLATE) && ns > 0) {
			return of_translate_address(ofnode_to_np(node), prop_val);
		} else {
			na = of_n_addr_cells(ofnode_to_np(node));
			return of_read_number(prop_val, na);
		}
	} else {
		na = ofnode_read_simple_addr_cells(ofnode_get_parent(node));
		ns = ofnode_read_simple_size_cells(ofnode_get_parent(node));
		return fdtdec_get_addr_size_fixed(ofnode_to_fdt(node),
						  ofnode_to_offset(node), "reg",
						  index, na, ns, size,
						  translate);
	}
}

fdt_addr_t ofnode_get_addr_size_index(ofnode node, int index, fdt_size_t *size)
{
	return __ofnode_get_addr_size_index(node, index, size, true);
}

fdt_addr_t ofnode_get_addr_size_index_notrans(ofnode node, int index,
					      fdt_size_t *size)
{
	return __ofnode_get_addr_size_index(node, index, size, false);
}

fdt_addr_t ofnode_get_addr_index(ofnode node, int index)
{
	fdt_size_t size;

	return ofnode_get_addr_size_index(node, index, &size);
}

fdt_addr_t ofnode_get_addr(ofnode node)
{
	return ofnode_get_addr_index(node, 0);
}

fdt_size_t ofnode_get_size(ofnode node)
{
	fdt_size_t size;

	ofnode_get_addr_size_index(node, 0, &size);

	return size;
}

int ofnode_stringlist_search(ofnode node, const char *property,
			     const char *string)
{
	if (ofnode_is_np(node)) {
		return of_property_match_string(ofnode_to_np(node),
						property, string);
	} else {
		int ret;

		ret = fdt_stringlist_search(ofnode_to_fdt(node),
					    ofnode_to_offset(node), property,
					    string);
		if (ret == -FDT_ERR_NOTFOUND)
			return -ENODATA;
		else if (ret < 0)
			return -EINVAL;

		return ret;
	}
}

int ofnode_read_string_index(ofnode node, const char *property, int index,
			     const char **outp)
{
	if (ofnode_is_np(node)) {
		return of_property_read_string_index(ofnode_to_np(node),
						     property, index, outp);
	} else {
		int len;

		*outp = fdt_stringlist_get(ofnode_to_fdt(node),
					   ofnode_to_offset(node),
					   property, index, &len);
		if (len < 0)
			return -EINVAL;
		return 0;
	}
}

int ofnode_read_string_count(ofnode node, const char *property)
{
	if (ofnode_is_np(node)) {
		return of_property_count_strings(ofnode_to_np(node), property);
	} else {
		return fdt_stringlist_count(ofnode_to_fdt(node),
					    ofnode_to_offset(node), property);
	}
}

int ofnode_read_string_list(ofnode node, const char *property,
			    const char ***listp)
{
	const char **prop;
	int count;
	int i;

	*listp = NULL;
	count = ofnode_read_string_count(node, property);
	if (count < 0)
		return count;
	if (!count)
		return 0;

	prop = calloc(count + 1, sizeof(char *));
	if (!prop)
		return -ENOMEM;

	for (i = 0; i < count; i++)
		ofnode_read_string_index(node, property, i, &prop[i]);
	prop[count] = NULL;
	*listp = prop;

	return count;
}

static void ofnode_from_fdtdec_phandle_args(struct fdtdec_phandle_args *in,
					    struct ofnode_phandle_args *out)
{
	assert(OF_MAX_PHANDLE_ARGS == MAX_PHANDLE_ARGS);
	out->node = offset_to_ofnode(in->node);
	out->args_count = in->args_count;
	memcpy(out->args, in->args, sizeof(out->args));
}

static void ofnode_from_of_phandle_args(struct of_phandle_args *in,
					struct ofnode_phandle_args *out)
{
	assert(OF_MAX_PHANDLE_ARGS == MAX_PHANDLE_ARGS);
	out->node = np_to_ofnode(in->np);
	out->args_count = in->args_count;
	memcpy(out->args, in->args, sizeof(out->args));
}

int ofnode_parse_phandle_with_args(ofnode node, const char *list_name,
				   const char *cells_name, int cell_count,
				   int index,
				   struct ofnode_phandle_args *out_args)
{
	if (ofnode_is_np(node)) {
		struct of_phandle_args args;
		int ret;

		ret = of_parse_phandle_with_args(ofnode_to_np(node),
						 list_name, cells_name,
						 cell_count, index,
						 &args);
		if (ret)
			return ret;
		ofnode_from_of_phandle_args(&args, out_args);
	} else {
		struct fdtdec_phandle_args args;
		int ret;

		ret = fdtdec_parse_phandle_with_args(ofnode_to_fdt(node),
						     ofnode_to_offset(node),
						     list_name, cells_name,
						     cell_count, index, &args);
		if (ret)
			return ret;
		ofnode_from_fdtdec_phandle_args(&args, out_args);
	}

	return 0;
}

int ofnode_count_phandle_with_args(ofnode node, const char *list_name,
				   const char *cells_name, int cell_count)
{
	if (ofnode_is_np(node))
		return of_count_phandle_with_args(ofnode_to_np(node),
				list_name, cells_name, cell_count);
	else
		return fdtdec_parse_phandle_with_args(ofnode_to_fdt(node),
				ofnode_to_offset(node), list_name, cells_name,
				cell_count, -1, NULL);
}

ofnode ofnode_path(const char *path)
{
	if (of_live_active())
		return np_to_ofnode(of_find_node_by_path(path));
	else
		return offset_to_ofnode(fdt_path_offset(gd->fdt_blob, path));
}

ofnode oftree_root(oftree tree)
{
	if (of_live_active()) {
		return np_to_ofnode(tree.np);
	} else {
		return ofnode_from_tree_offset(tree, 0);
	}
}

ofnode oftree_path(oftree tree, const char *path)
{
	if (of_live_active()) {
		return np_to_ofnode(of_find_node_opts_by_path(tree.np, path,
							      NULL));
	} else if (*path != '/' && tree.fdt != gd->fdt_blob) {
		return ofnode_null();  /* Aliases only on control FDT */
	} else {
		int offset = fdt_path_offset(tree.fdt, path);

		return ofnode_from_tree_offset(tree, offset);
	}
}

const void *ofnode_read_chosen_prop(const char *propname, int *sizep)
{
	ofnode chosen_node;

	chosen_node = ofnode_path("/chosen");

	return ofnode_read_prop(chosen_node, propname, sizep);
}

const char *ofnode_read_chosen_string(const char *propname)
{
	return ofnode_read_chosen_prop(propname, NULL);
}

ofnode ofnode_get_chosen_node(const char *name)
{
	const char *prop;

	prop = ofnode_read_chosen_prop(name, NULL);
	if (!prop)
		return ofnode_null();

	return ofnode_path(prop);
}

int ofnode_read_baud(void)
{
	const char *str, *p;
	u32 baud;

	str = ofnode_read_chosen_string("stdout-path");
	if (!str)
		return -EINVAL;

	/* Parse string serial0:115200n8 */
	p = strchr(str, ':');
	if (!p)
		return -EINVAL;

	baud = dectoul(p + 1, NULL);
	return baud;
}

const void *ofnode_read_aliases_prop(const char *propname, int *sizep)
{
	ofnode node;

	node = ofnode_path("/aliases");

	return ofnode_read_prop(node, propname, sizep);
}

ofnode ofnode_get_aliases_node(const char *name)
{
	const char *prop;

	prop = ofnode_read_aliases_prop(name, NULL);
	if (!prop)
		return ofnode_null();

	debug("%s: node_path: %s\n", __func__, prop);

	return ofnode_path(prop);
}

int ofnode_get_child_count(ofnode parent)
{
	ofnode child;
	int num = 0;

	ofnode_for_each_subnode(child, parent)
		num++;

	return num;
}

static int decode_timing_property(ofnode node, const char *name,
				  struct timing_entry *result)
{
	int length, ret = 0;

	length = ofnode_read_size(node, name);
	if (length < 0) {
		debug("%s: could not find property %s\n",
		      ofnode_get_name(node), name);
		return length;
	}

	if (length == sizeof(u32)) {
		result->typ = ofnode_read_u32_default(node, name, 0);
		result->min = result->typ;
		result->max = result->typ;
	} else {
		ret = ofnode_read_u32_array(node, name, &result->min, 3);
	}

	return ret;
}

int ofnode_decode_display_timing(ofnode parent, int index,
				 struct display_timing *dt)
{
	int i;
	ofnode timings, node;
	u32 val = 0;
	int ret = 0;

	timings = ofnode_find_subnode(parent, "display-timings");
	if (!ofnode_valid(timings))
		return -EINVAL;

	i = 0;
	ofnode_for_each_subnode(node, timings) {
		if (i++ == index)
			break;
	}

	if (!ofnode_valid(node))
		return -EINVAL;

	memset(dt, 0, sizeof(*dt));

	ret |= decode_timing_property(node, "hback-porch", &dt->hback_porch);
	ret |= decode_timing_property(node, "hfront-porch", &dt->hfront_porch);
	ret |= decode_timing_property(node, "hactive", &dt->hactive);
	ret |= decode_timing_property(node, "hsync-len", &dt->hsync_len);
	ret |= decode_timing_property(node, "vback-porch", &dt->vback_porch);
	ret |= decode_timing_property(node, "vfront-porch", &dt->vfront_porch);
	ret |= decode_timing_property(node, "vactive", &dt->vactive);
	ret |= decode_timing_property(node, "vsync-len", &dt->vsync_len);
	ret |= decode_timing_property(node, "clock-frequency", &dt->pixelclock);

	dt->flags = 0;
	val = ofnode_read_u32_default(node, "vsync-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_VSYNC_HIGH :
				DISPLAY_FLAGS_VSYNC_LOW;
	}
	val = ofnode_read_u32_default(node, "hsync-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_HSYNC_HIGH :
				DISPLAY_FLAGS_HSYNC_LOW;
	}
	val = ofnode_read_u32_default(node, "de-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_DE_HIGH :
				DISPLAY_FLAGS_DE_LOW;
	}
	val = ofnode_read_u32_default(node, "pixelclk-active", -1);
	if (val != -1) {
		dt->flags |= val ? DISPLAY_FLAGS_PIXDATA_POSEDGE :
				DISPLAY_FLAGS_PIXDATA_NEGEDGE;
	}

	if (ofnode_read_bool(node, "interlaced"))
		dt->flags |= DISPLAY_FLAGS_INTERLACED;
	if (ofnode_read_bool(node, "doublescan"))
		dt->flags |= DISPLAY_FLAGS_DOUBLESCAN;
	if (ofnode_read_bool(node, "doubleclk"))
		dt->flags |= DISPLAY_FLAGS_DOUBLECLK;

	return ret;
}

int ofnode_decode_panel_timing(ofnode parent,
			       struct display_timing *dt)
{
	ofnode timings;
	u32 val = 0;
	int ret = 0;

	timings = ofnode_find_subnode(parent, "panel-timing");
	if (!ofnode_valid(timings))
		return -EINVAL;
	memset(dt, 0, sizeof(*dt));
	ret |= decode_timing_property(timings, "hback-porch", &dt->hback_porch);
	ret |= decode_timing_property(timings, "hfront-porch", &dt->hfront_porch);
	ret |= decode_timing_property(timings, "hactive", &dt->hactive);
	ret |= decode_timing_property(timings, "hsync-len", &dt->hsync_len);
	ret |= decode_timing_property(timings, "vback-porch", &dt->vback_porch);
	ret |= decode_timing_property(timings, "vfront-porch", &dt->vfront_porch);
	ret |= decode_timing_property(timings, "vactive", &dt->vactive);
	ret |= decode_timing_property(timings, "vsync-len", &dt->vsync_len);
	ret |= decode_timing_property(timings, "clock-frequency", &dt->pixelclock);
	dt->flags = 0;
	if (!ofnode_read_u32(timings, "vsync-active", &val)) {
		dt->flags |= val ? DISPLAY_FLAGS_VSYNC_HIGH :
		    DISPLAY_FLAGS_VSYNC_LOW;
	}
	if (!ofnode_read_u32(timings, "hsync-active", &val)) {
		dt->flags |= val ? DISPLAY_FLAGS_HSYNC_HIGH :
		    DISPLAY_FLAGS_HSYNC_LOW;
	}
	if (!ofnode_read_u32(timings, "de-active", &val)) {
		dt->flags |= val ? DISPLAY_FLAGS_DE_HIGH :
		    DISPLAY_FLAGS_DE_LOW;
	}
	if (!ofnode_read_u32(timings, "pixelclk-active", &val)) {
		dt->flags |= val ? DISPLAY_FLAGS_PIXDATA_POSEDGE :
		DISPLAY_FLAGS_PIXDATA_NEGEDGE;
	}
	if (ofnode_read_bool(timings, "interlaced"))
		dt->flags |= DISPLAY_FLAGS_INTERLACED;
	if (ofnode_read_bool(timings, "doublescan"))
		dt->flags |= DISPLAY_FLAGS_DOUBLESCAN;
	if (ofnode_read_bool(timings, "doubleclk"))
		dt->flags |= DISPLAY_FLAGS_DOUBLECLK;

	return ret;
}

const void *ofnode_get_property(ofnode node, const char *propname, int *lenp)
{
	if (ofnode_is_np(node))
		return of_get_property(ofnode_to_np(node), propname, lenp);
	else
		return fdt_getprop(ofnode_to_fdt(node), ofnode_to_offset(node),
				   propname, lenp);
}

bool ofnode_has_property(ofnode node, const char *propname)
{
	if (ofnode_is_np(node))
		return of_find_property(ofnode_to_np(node), propname, NULL);
	else
		return ofnode_get_property(node, propname, NULL);
}

int ofnode_first_property(ofnode node, struct ofprop *prop)
{
	prop->node = node;

	if (ofnode_is_np(node)) {
		prop->prop = of_get_first_property(ofnode_to_np(prop->node));
		if (!prop->prop)
			return -FDT_ERR_NOTFOUND;
	} else {
		prop->offset =
			fdt_first_property_offset(ofnode_to_fdt(node),
						  ofnode_to_offset(prop->node));
		if (prop->offset < 0)
			return prop->offset;
	}

	return 0;
}

int ofnode_next_property(struct ofprop *prop)
{
	if (ofnode_is_np(prop->node)) {
		prop->prop = of_get_next_property(ofnode_to_np(prop->node),
						  prop->prop);
		if (!prop->prop)
			return -FDT_ERR_NOTFOUND;
	} else {
		prop->offset =
			fdt_next_property_offset(ofnode_to_fdt(prop->node),
						 prop->offset);
		if (prop->offset  < 0)
			return prop->offset;
	}

	return 0;
}

const void *ofprop_get_property(const struct ofprop *prop,
				const char **propname, int *lenp)
{
	if (ofnode_is_np(prop->node))
		return of_get_property_by_prop(ofnode_to_np(prop->node),
					       prop->prop, propname, lenp);
	else
		return fdt_getprop_by_offset(ofnode_to_fdt(prop->node),
					     prop->offset,
					     propname, lenp);
}

fdt_addr_t ofnode_get_addr_size(ofnode node, const char *property,
				fdt_size_t *sizep)
{
	if (ofnode_is_np(node)) {
		int na, ns;
		int psize;
		const struct device_node *np = ofnode_to_np(node);
		const __be32 *prop = of_get_property(np, property, &psize);

		if (!prop)
			return FDT_ADDR_T_NONE;
		na = of_n_addr_cells(np);
		ns = of_n_size_cells(np);
		*sizep = of_read_number(prop + na, ns);

		if (CONFIG_IS_ENABLED(OF_TRANSLATE) && ns > 0)
			return of_translate_address(np, prop);
		else
			return of_read_number(prop, na);
	} else {
		return fdtdec_get_addr_size(ofnode_to_fdt(node),
					    ofnode_to_offset(node), property,
					    sizep);
	}
}

const uint8_t *ofnode_read_u8_array_ptr(ofnode node, const char *propname,
					size_t sz)
{
	if (ofnode_is_np(node)) {
		const struct device_node *np = ofnode_to_np(node);
		int psize;
		const __be32 *prop = of_get_property(np, propname, &psize);

		if (!prop || sz != psize)
			return NULL;
		return (uint8_t *)prop;

	} else {
		return fdtdec_locate_byte_array(ofnode_to_fdt(node),
				ofnode_to_offset(node), propname, sz);
	}
}

int ofnode_read_pci_addr(ofnode node, enum fdt_pci_space type,
			 const char *propname, struct fdt_pci_addr *addr,
			 fdt_size_t *size)
{
	const fdt32_t *cell;
	int len;
	int ret = -ENOENT;

	debug("%s: %s: ", __func__, propname);

	/*
	 * If we follow the pci bus bindings strictly, we should check
	 * the value of the node's parent node's #address-cells and
	 * #size-cells. They need to be 3 and 2 accordingly. However,
	 * for simplicity we skip the check here.
	 */
	cell = ofnode_get_property(node, propname, &len);
	if (!cell)
		goto fail;

	if ((len % FDT_PCI_REG_SIZE) == 0) {
		int num = len / FDT_PCI_REG_SIZE;
		int i;

		for (i = 0; i < num; i++) {
			debug("pci address #%d: %08lx %08lx %08lx\n", i,
			      (ulong)fdt32_to_cpu(cell[0]),
			      (ulong)fdt32_to_cpu(cell[1]),
			      (ulong)fdt32_to_cpu(cell[2]));
			if ((fdt32_to_cpu(*cell) & type) == type) {
				const unaligned_fdt64_t *ptr;

				addr->phys_hi = fdt32_to_cpu(cell[0]);
				addr->phys_mid = fdt32_to_cpu(cell[1]);
				addr->phys_lo = fdt32_to_cpu(cell[2]);
				ptr = (const unaligned_fdt64_t *)(cell + 3);
				if (size)
					*size = fdt64_to_cpu(*ptr);
				break;
			}

			cell += FDT_PCI_ADDR_CELLS + FDT_PCI_SIZE_CELLS;
		}

		if (i == num) {
			ret = -ENXIO;
			goto fail;
		}

		return 0;
	}

	ret = -EINVAL;

fail:
	debug("(not found)\n");
	return ret;
}

int ofnode_read_pci_vendev(ofnode node, u16 *vendor, u16 *device)
{
	const char *list, *end;
	int len;

	list = ofnode_get_property(node, "compatible", &len);
	if (!list)
		return -ENOENT;

	end = list + len;
	while (list < end) {
		len = strlen(list);
		if (len >= strlen("pciVVVV,DDDD")) {
			char *s = strstr(list, "pci");

			/*
			 * check if the string is something like pciVVVV,DDDD.RR
			 * or just pciVVVV,DDDD
			 */
			if (s && s[7] == ',' &&
			    (s[12] == '.' || s[12] == 0)) {
				s += 3;
				*vendor = simple_strtol(s, NULL, 16);

				s += 5;
				*device = simple_strtol(s, NULL, 16);

				return 0;
			}
		}
		list += (len + 1);
	}

	return -ENOENT;
}

int ofnode_read_eth_phy_id(ofnode node, u16 *vendor, u16 *device)
{
	const char *list, *end;
	int len;

	list = ofnode_get_property(node, "compatible", &len);

	if (!list)
		return -ENOENT;

	end = list + len;
	while (list < end) {
		len = strlen(list);

		if (len >= strlen("ethernet-phy-idVVVV.DDDD")) {
			char *s = strstr(list, "ethernet-phy-id");

			/*
			 * check if the string is something like
			 * ethernet-phy-idVVVV.DDDD
			 */
			if (s && s[19] == '.') {
				s += strlen("ethernet-phy-id");
				*vendor = simple_strtol(s, NULL, 16);
				s += 5;
				*device = simple_strtol(s, NULL, 16);

				return 0;
			}
		}
		list += (len + 1);
	}

	return -ENOENT;
}

int ofnode_read_addr_cells(ofnode node)
{
	if (ofnode_is_np(node)) {
		return of_n_addr_cells(ofnode_to_np(node));
	} else {
		int parent = fdt_parent_offset(ofnode_to_fdt(node),
					       ofnode_to_offset(node));

		return fdt_address_cells(ofnode_to_fdt(node), parent);
	}
}

int ofnode_read_size_cells(ofnode node)
{
	if (ofnode_is_np(node)) {
		return of_n_size_cells(ofnode_to_np(node));
	} else {
		int parent = fdt_parent_offset(ofnode_to_fdt(node),
					       ofnode_to_offset(node));

		return fdt_size_cells(ofnode_to_fdt(node), parent);
	}
}

int ofnode_read_simple_addr_cells(ofnode node)
{
	if (ofnode_is_np(node))
		return of_simple_addr_cells(ofnode_to_np(node));
	else
		return fdt_address_cells(ofnode_to_fdt(node),
					 ofnode_to_offset(node));
}

int ofnode_read_simple_size_cells(ofnode node)
{
	if (ofnode_is_np(node))
		return of_simple_size_cells(ofnode_to_np(node));
	else
		return fdt_size_cells(ofnode_to_fdt(node),
				      ofnode_to_offset(node));
}

bool ofnode_pre_reloc(ofnode node)
{
#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_TPL_BUILD)
	/* for SPL and TPL the remaining nodes after the fdtgrep 1st pass
	 * had property bootph-all or bootph-pre-sram/bootph-pre-ram.
	 * They are removed in final dtb (fdtgrep 2nd pass)
	 */
	return true;
#else
	if (ofnode_read_bool(node, "bootph-all"))
		return true;
	if (ofnode_read_bool(node, "bootph-some-ram"))
		return true;

	/*
	 * In regular builds individual spl and tpl handling both
	 * count as handled pre-relocation for later second init.
	 */
	if (ofnode_read_bool(node, "bootph-pre-ram") ||
	    ofnode_read_bool(node, "bootph-pre-sram"))
		return gd->flags & GD_FLG_RELOC;

	if (IS_ENABLED(CONFIG_OF_TAG_MIGRATE)) {
		/* detect and handle old tags */
		if (ofnode_read_bool(node, "u-boot,dm-pre-reloc") ||
		    ofnode_read_bool(node, "u-boot,dm-pre-proper") ||
		    ofnode_read_bool(node, "u-boot,dm-spl") ||
		    ofnode_read_bool(node, "u-boot,dm-tpl") ||
		    ofnode_read_bool(node, "u-boot,dm-vpl")) {
			gd->flags |= GD_FLG_OF_TAG_MIGRATE;
			return true;
		}
	}

	return false;
#endif
}

int ofnode_read_resource(ofnode node, uint index, struct resource *res)
{
	if (ofnode_is_np(node)) {
		return of_address_to_resource(ofnode_to_np(node), index, res);
	} else {
		struct fdt_resource fres;
		int ret;

		ret = fdt_get_resource(ofnode_to_fdt(node),
				       ofnode_to_offset(node),
				       "reg", index, &fres);
		if (ret < 0)
			return -EINVAL;
		memset(res, '\0', sizeof(*res));
		res->start = fres.start;
		res->end = fres.end;

		return 0;
	}
}

int ofnode_read_resource_byname(ofnode node, const char *name,
				struct resource *res)
{
	int index;

	index = ofnode_stringlist_search(node, "reg-names", name);
	if (index < 0)
		return index;

	return ofnode_read_resource(node, index, res);
}

u64 ofnode_translate_address(ofnode node, const fdt32_t *in_addr)
{
	if (ofnode_is_np(node))
		return of_translate_address(ofnode_to_np(node), in_addr);
	else
		return fdt_translate_address(ofnode_to_fdt(node),
					     ofnode_to_offset(node), in_addr);
}

u64 ofnode_translate_dma_address(ofnode node, const fdt32_t *in_addr)
{
	if (ofnode_is_np(node))
		return of_translate_dma_address(ofnode_to_np(node), in_addr);
	else
		return fdt_translate_dma_address(ofnode_to_fdt(node),
						 ofnode_to_offset(node), in_addr);
}

int ofnode_get_dma_range(ofnode node, phys_addr_t *cpu, dma_addr_t *bus, u64 *size)
{
	if (ofnode_is_np(node))
		return of_get_dma_range(ofnode_to_np(node), cpu, bus, size);
	else
		return fdt_get_dma_range(ofnode_to_fdt(node),
					 ofnode_to_offset(node),
					 cpu, bus, size);
}

int ofnode_device_is_compatible(ofnode node, const char *compat)
{
	if (ofnode_is_np(node))
		return of_device_is_compatible(ofnode_to_np(node), compat,
					       NULL, NULL);
	else
		return !fdt_node_check_compatible(ofnode_to_fdt(node),
						  ofnode_to_offset(node),
						  compat);
}

ofnode ofnode_by_compatible(ofnode from, const char *compat)
{
	if (of_live_active()) {
		return np_to_ofnode(of_find_compatible_node(
			(struct device_node *)ofnode_to_np(from), NULL,
			compat));
	} else {
		return noffset_to_ofnode(from,
			fdt_node_offset_by_compatible(ofnode_to_fdt(from),
					ofnode_to_offset(from), compat));
	}
}

ofnode ofnode_by_prop_value(ofnode from, const char *propname,
			    const void *propval, int proplen)
{
	if (of_live_active()) {
		return np_to_ofnode(of_find_node_by_prop_value(
			(struct device_node *)ofnode_to_np(from), propname,
			propval, proplen));
	} else {
		return noffset_to_ofnode(from,
			 fdt_node_offset_by_prop_value(ofnode_to_fdt(from),
				ofnode_to_offset(from), propname, propval,
				proplen));
	}
}

int ofnode_write_prop(ofnode node, const char *propname, const void *value,
		      int len, bool copy)
{
	if (of_live_active()) {
		void *newval;
		int ret;

		if (copy) {
			newval = malloc(len);
			if (!newval)
				return log_ret(-ENOMEM);
			memcpy(newval, value, len);
			value = newval;
		}
		ret = of_write_prop(ofnode_to_np(node), propname, len, value);
		if (ret && copy)
			free(newval);
		return ret;
	} else {
		return fdt_setprop(ofnode_to_fdt(node), ofnode_to_offset(node),
				   propname, value, len);
	}
}

int ofnode_write_string(ofnode node, const char *propname, const char *value)
{
	assert(ofnode_valid(node));

	debug("%s: %s = %s", __func__, propname, value);

	return ofnode_write_prop(node, propname, value, strlen(value) + 1,
				 false);
}

int ofnode_write_u32(ofnode node, const char *propname, u32 value)
{
	fdt32_t *val;

	assert(ofnode_valid(node));

	log_debug("%s = %x", propname, value);
	val = malloc(sizeof(*val));
	if (!val)
		return -ENOMEM;
	*val = cpu_to_fdt32(value);

	return ofnode_write_prop(node, propname, val, sizeof(value), true);
}

int ofnode_write_u64(ofnode node, const char *propname, u64 value)
{
	fdt64_t *val;

	assert(ofnode_valid(node));

	log_debug("%s = %llx", propname, (unsigned long long)value);
	val = malloc(sizeof(*val));
	if (!val)
		return -ENOMEM;
	*val = cpu_to_fdt64(value);

	return ofnode_write_prop(node, propname, val, sizeof(value), true);
}

int ofnode_write_bool(ofnode node, const char *propname, bool value)
{
	if (value)
		return ofnode_write_prop(node, propname, NULL, 0, false);
	else
		return ofnode_delete_prop(node, propname);
}

int ofnode_delete_prop(ofnode node, const char *propname)
{
	if (ofnode_is_np(node)) {
		struct property *prop;
		int len;

		prop = of_find_property(ofnode_to_np(node), propname, &len);
		if (prop)
			return of_remove_property(ofnode_to_np(node), prop);
		return 0;
	} else {
		return fdt_delprop(ofnode_to_fdt(node), ofnode_to_offset(node),
				   propname);
	}
}

int ofnode_set_enabled(ofnode node, bool value)
{
	assert(ofnode_valid(node));

	if (value)
		return ofnode_write_string(node, "status", "okay");
	else
		return ofnode_write_string(node, "status", "disabled");
}

bool ofnode_conf_read_bool(const char *prop_name)
{
	ofnode node;

	node = ofnode_path("/config");
	if (!ofnode_valid(node))
		return false;

	return ofnode_read_bool(node, prop_name);
}

int ofnode_conf_read_int(const char *prop_name, int default_val)
{
	ofnode node;

	node = ofnode_path("/config");
	if (!ofnode_valid(node))
		return default_val;

	return ofnode_read_u32_default(node, prop_name, default_val);
}

const char *ofnode_conf_read_str(const char *prop_name)
{
	ofnode node;

	node = ofnode_path("/config");
	if (!ofnode_valid(node))
		return NULL;

	return ofnode_read_string(node, prop_name);
}

int ofnode_read_bootscript_address(u64 *bootscr_address, u64 *bootscr_offset)
{
	int ret;
	ofnode uboot;

	*bootscr_address = 0;
	*bootscr_offset = 0;

	uboot = ofnode_path("/options/u-boot");
	if (!ofnode_valid(uboot)) {
		debug("%s: Missing /u-boot node\n", __func__);
		return -EINVAL;
	}

	ret = ofnode_read_u64(uboot, "bootscr-address", bootscr_address);
	if (ret) {
		ret = ofnode_read_u64(uboot, "bootscr-ram-offset",
				      bootscr_offset);
		if (ret)
			return -EINVAL;
	}

	return 0;
}

int ofnode_read_bootscript_flash(u64 *bootscr_flash_offset,
				 u64 *bootscr_flash_size)
{
	int ret;
	ofnode uboot;

	*bootscr_flash_offset = 0;
	*bootscr_flash_size = 0;

	uboot = ofnode_path("/options/u-boot");
	if (!ofnode_valid(uboot)) {
		debug("%s: Missing /u-boot node\n", __func__);
		return -EINVAL;
	}

	ret = ofnode_read_u64(uboot, "bootscr-flash-offset",
			      bootscr_flash_offset);
	if (ret)
		return -EINVAL;

	ret = ofnode_read_u64(uboot, "bootscr-flash-size",
			      bootscr_flash_size);
	if (ret)
		return -EINVAL;

	if (!bootscr_flash_size) {
		debug("bootscr-flash-size is zero. Ignoring properties!\n");
		*bootscr_flash_offset = 0;
		return -EINVAL;
	}

	return 0;
}

ofnode ofnode_get_phy_node(ofnode node)
{
	/* DT node properties that reference a PHY node */
	static const char * const phy_handle_str[] = {
		"phy-handle", "phy", "phy-device",
	};
	struct ofnode_phandle_args args = {
		.node = ofnode_null()
	};
	int i;

	assert(ofnode_valid(node));

	for (i = 0; i < ARRAY_SIZE(phy_handle_str); i++)
		if (!ofnode_parse_phandle_with_args(node, phy_handle_str[i],
						    NULL, 0, 0, &args))
			break;

	return args.node;
}

phy_interface_t ofnode_read_phy_mode(ofnode node)
{
	const char *mode;
	int i;

	assert(ofnode_valid(node));

	mode = ofnode_read_string(node, "phy-mode");
	if (!mode)
		mode = ofnode_read_string(node, "phy-connection-type");

	if (!mode)
		return PHY_INTERFACE_MODE_NA;

	for (i = 0; i < PHY_INTERFACE_MODE_MAX; i++)
		if (!strcmp(mode, phy_interface_strings[i]))
			return i;

	debug("%s: Invalid PHY interface '%s'\n", __func__, mode);

	return PHY_INTERFACE_MODE_NA;
}

int ofnode_add_subnode(ofnode node, const char *name, ofnode *subnodep)
{
	ofnode subnode;
	int ret = 0;

	assert(ofnode_valid(node));

	if (ofnode_is_np(node)) {
		struct device_node *np, *child;

		np = (struct device_node *)ofnode_to_np(node);
		ret = of_add_subnode(np, name, -1, &child);
		if (ret && ret != -EEXIST)
			return ret;
		subnode = np_to_ofnode(child);
	} else {
		void *fdt = ofnode_to_fdt(node);
		int poffset = ofnode_to_offset(node);
		int offset;

		offset = fdt_add_subnode(fdt, poffset, name);
		if (offset == -FDT_ERR_EXISTS) {
			offset = fdt_subnode_offset(fdt, poffset, name);
			ret = -EEXIST;
		}
		if (offset < 0)
			return -EINVAL;
		subnode = noffset_to_ofnode(node, offset);
	}

	*subnodep = subnode;

	return ret;	/* 0 or -EEXIST */
}

int ofnode_delete(ofnode *nodep)
{
	ofnode node = *nodep;
	int ret;

	assert(ofnode_valid(node));
	if (ofnode_is_np(node)) {
		ret = of_remove_node(ofnode_to_np(node));
	} else {
		void *fdt = ofnode_to_fdt(node);
		int offset = ofnode_to_offset(node);

		ret = fdt_del_node(fdt, offset);
		if (ret)
			ret = -EFAULT;
	}
	if (ret)
		return ret;
	*nodep = ofnode_null();

	return 0;
}

int ofnode_copy_props(ofnode dst, ofnode src)
{
	struct ofprop prop;

	ofnode_for_each_prop(prop, src) {
		const char *name;
		const char *val;
		int len, ret;

		val = ofprop_get_property(&prop, &name, &len);
		if (!val) {
			log_debug("Cannot read prop (err=%d)\n", len);
			return log_msg_ret("get", -EINVAL);
		}
		ret = ofnode_write_prop(dst, name, val, len, true);
		if (ret) {
			log_debug("Cannot write prop (err=%d)\n", ret);
			return log_msg_ret("wr", -EINVAL);
		}
	}

	return 0;
}

int ofnode_copy_node(ofnode dst_parent, const char *name, ofnode src,
		     ofnode *nodep)
{
	ofnode node;
	int ret;

	ret = ofnode_add_subnode(dst_parent, name, &node);
	if (ret) {
		if (ret == -EEXIST)
			*nodep = node;
		return log_msg_ret("add", ret);
	}
	ret = ofnode_copy_props(node, src);
	if (ret)
		return log_msg_ret("cpy", ret);
	*nodep = node;

	return 0;
}
