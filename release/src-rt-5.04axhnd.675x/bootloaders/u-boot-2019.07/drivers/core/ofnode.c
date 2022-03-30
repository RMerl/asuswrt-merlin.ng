// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <dm/of_access.h>
#include <dm/of_addr.h>
#include <dm/ofnode.h>
#include <linux/err.h>
#include <linux/ioport.h>

int ofnode_read_u32(ofnode node, const char *propname, u32 *outp)
{
	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node)) {
		return of_read_u32(ofnode_to_np(node), propname, outp);
	} else {
		const fdt32_t *cell;
		int len;

		cell = fdt_getprop(gd->fdt_blob, ofnode_to_offset(node),
				   propname, &len);
		if (!cell || len < sizeof(int)) {
			debug("(not found)\n");
			return -EINVAL;
		}
		*outp = fdt32_to_cpu(cell[0]);
	}
	debug("%#x (%d)\n", *outp, *outp);

	return 0;
}

u32 ofnode_read_u32_default(ofnode node, const char *propname, u32 def)
{
	assert(ofnode_valid(node));
	ofnode_read_u32(node, propname, &def);

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
	const fdt64_t *cell;
	int len;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node))
		return of_read_u64(ofnode_to_np(node), propname, outp);

	cell = fdt_getprop(gd->fdt_blob, ofnode_to_offset(node), propname,
			   &len);
	if (!cell || len < sizeof(*cell)) {
		debug("(not found)\n");
		return -EINVAL;
	}
	*outp = fdt64_to_cpu(cell[0]);
	debug("%#llx (%lld)\n", (unsigned long long)*outp,
	      (unsigned long long)*outp);

	return 0;
}

int ofnode_read_u64_default(ofnode node, const char *propname, u64 def)
{
	assert(ofnode_valid(node));
	ofnode_read_u64(node, propname, &def);

	return def;
}

bool ofnode_read_bool(ofnode node, const char *propname)
{
	const void *prop;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	prop = ofnode_get_property(node, propname, NULL);

	debug("%s\n", prop ? "true" : "false");

	return prop ? true : false;
}

const char *ofnode_read_string(ofnode node, const char *propname)
{
	const char *str = NULL;
	int len = -1;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, propname);

	if (ofnode_is_np(node)) {
		struct property *prop = of_find_property(
				ofnode_to_np(node), propname, NULL);

		if (prop) {
			str = prop->value;
			len = prop->length;
		}
	} else {
		str = fdt_getprop(gd->fdt_blob, ofnode_to_offset(node),
				  propname, &len);
	}
	if (!str) {
		debug("<not found>\n");
		return NULL;
	}
	if (strnlen(str, len) >= len) {
		debug("<invalid>\n");
		return NULL;
	}
	debug("%s\n", str);

	return str;
}

ofnode ofnode_find_subnode(ofnode node, const char *subnode_name)
{
	ofnode subnode;

	assert(ofnode_valid(node));
	debug("%s: %s: ", __func__, subnode_name);

	if (ofnode_is_np(node)) {
		const struct device_node *np = ofnode_to_np(node);

		for (np = np->child; np; np = np->sibling) {
			if (!strcmp(subnode_name, np->name))
				break;
		}
		subnode = np_to_ofnode(np);
	} else {
		int ooffset = fdt_subnode_offset(gd->fdt_blob,
				ofnode_to_offset(node), subnode_name);
		subnode = offset_to_ofnode(ooffset);
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
		return fdtdec_get_int_array(gd->fdt_blob,
					    ofnode_to_offset(node), propname,
					    out_values, sz);
	}
}

ofnode ofnode_first_subnode(ofnode node)
{
	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		return np_to_ofnode(node.np->child);

	return offset_to_ofnode(
		fdt_first_subnode(gd->fdt_blob, ofnode_to_offset(node)));
}

ofnode ofnode_next_subnode(ofnode node)
{
	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		return np_to_ofnode(node.np->sibling);

	return offset_to_ofnode(
		fdt_next_subnode(gd->fdt_blob, ofnode_to_offset(node)));
}

ofnode ofnode_get_parent(ofnode node)
{
	ofnode parent;

	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		parent = np_to_ofnode(of_get_parent(ofnode_to_np(node)));
	else
		parent.of_offset = fdt_parent_offset(gd->fdt_blob,
						     ofnode_to_offset(node));

	return parent;
}

const char *ofnode_get_name(ofnode node)
{
	assert(ofnode_valid(node));
	if (ofnode_is_np(node))
		return strrchr(node.np->full_name, '/') + 1;

	return fdt_get_name(gd->fdt_blob, ofnode_to_offset(node), NULL);
}

ofnode ofnode_get_by_phandle(uint phandle)
{
	ofnode node;

	if (of_live_active())
		node = np_to_ofnode(of_find_node_by_phandle(phandle));
	else
		node.of_offset = fdt_node_offset_by_phandle(gd->fdt_blob,
							    phandle);

	return node;
}

int ofnode_read_size(ofnode node, const char *propname)
{
	int len;

	if (ofnode_is_np(node)) {
		struct property *prop = of_find_property(
				ofnode_to_np(node), propname, NULL);

		if (prop)
			return prop->length;
	} else {
		if (fdt_getprop(gd->fdt_blob, ofnode_to_offset(node), propname,
				&len))
			return len;
	}

	return -EINVAL;
}

fdt_addr_t ofnode_get_addr_size_index(ofnode node, int index, fdt_size_t *size)
{
	int na, ns;

	if (ofnode_is_np(node)) {
		const __be32 *prop_val;
		uint flags;

		prop_val = of_get_address(ofnode_to_np(node), index,
					  (u64 *)size, &flags);
		if (!prop_val)
			return FDT_ADDR_T_NONE;

		ns = of_n_size_cells(ofnode_to_np(node));

		if (IS_ENABLED(CONFIG_OF_TRANSLATE) && ns > 0) {
			return of_translate_address(ofnode_to_np(node), prop_val);
		} else {
			na = of_n_addr_cells(ofnode_to_np(node));
			return of_read_number(prop_val, na);
		}
	} else {
		na = ofnode_read_simple_addr_cells(ofnode_get_parent(node));
		ns = ofnode_read_simple_size_cells(ofnode_get_parent(node));
		return fdtdec_get_addr_size_fixed(gd->fdt_blob,
						  ofnode_to_offset(node), "reg",
						  index, na, ns, size, true);
	}

	return FDT_ADDR_T_NONE;
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

int ofnode_stringlist_search(ofnode node, const char *property,
			     const char *string)
{
	if (ofnode_is_np(node)) {
		return of_property_match_string(ofnode_to_np(node),
						property, string);
	} else {
		int ret;

		ret = fdt_stringlist_search(gd->fdt_blob,
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

		*outp = fdt_stringlist_get(gd->fdt_blob, ofnode_to_offset(node),
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
		return fdt_stringlist_count(gd->fdt_blob,
					    ofnode_to_offset(node), property);
	}
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
						 list_name, cells_name, index,
						 &args);
		if (ret)
			return ret;
		ofnode_from_of_phandle_args(&args, out_args);
	} else {
		struct fdtdec_phandle_args args;
		int ret;

		ret = fdtdec_parse_phandle_with_args(gd->fdt_blob,
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
				   const char *cells_name)
{
	if (ofnode_is_np(node))
		return of_count_phandle_with_args(ofnode_to_np(node),
				list_name, cells_name);
	else
		return fdtdec_parse_phandle_with_args(gd->fdt_blob,
				ofnode_to_offset(node), list_name, cells_name,
				0, -1, NULL);
}

ofnode ofnode_path(const char *path)
{
	if (of_live_active())
		return np_to_ofnode(of_find_node_by_path(path));
	else
		return offset_to_ofnode(fdt_path_offset(gd->fdt_blob, path));
}

const char *ofnode_get_chosen_prop(const char *name)
{
	ofnode chosen_node;

	chosen_node = ofnode_path("/chosen");

	return ofnode_read_string(chosen_node, name);
}

ofnode ofnode_get_chosen_node(const char *name)
{
	const char *prop;

	prop = ofnode_get_chosen_prop(name);
	if (!prop)
		return ofnode_null();

	return ofnode_path(prop);
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

const void *ofnode_get_property(ofnode node, const char *propname, int *lenp)
{
	if (ofnode_is_np(node))
		return of_get_property(ofnode_to_np(node), propname, lenp);
	else
		return fdt_getprop(gd->fdt_blob, ofnode_to_offset(node),
				   propname, lenp);
}

bool ofnode_is_available(ofnode node)
{
	if (ofnode_is_np(node))
		return of_device_is_available(ofnode_to_np(node));
	else
		return fdtdec_get_is_enabled(gd->fdt_blob,
					     ofnode_to_offset(node));
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
		return fdtdec_get_addr_size(gd->fdt_blob,
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
		return fdtdec_locate_byte_array(gd->fdt_blob,
				ofnode_to_offset(node), propname, sz);
	}
}

int ofnode_read_pci_addr(ofnode node, enum fdt_pci_space type,
			 const char *propname, struct fdt_pci_addr *addr)
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
				addr->phys_hi = fdt32_to_cpu(cell[0]);
				addr->phys_mid = fdt32_to_cpu(cell[1]);
				addr->phys_lo = fdt32_to_cpu(cell[1]);
				break;
			}

			cell += (FDT_PCI_ADDR_CELLS +
				 FDT_PCI_SIZE_CELLS);
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

int ofnode_read_addr_cells(ofnode node)
{
	if (ofnode_is_np(node))
		return of_n_addr_cells(ofnode_to_np(node));
	else  /* NOTE: this call should walk up the parent stack */
		return fdt_address_cells(gd->fdt_blob, ofnode_to_offset(node));
}

int ofnode_read_size_cells(ofnode node)
{
	if (ofnode_is_np(node))
		return of_n_size_cells(ofnode_to_np(node));
	else  /* NOTE: this call should walk up the parent stack */
		return fdt_size_cells(gd->fdt_blob, ofnode_to_offset(node));
}

int ofnode_read_simple_addr_cells(ofnode node)
{
	if (ofnode_is_np(node))
		return of_simple_addr_cells(ofnode_to_np(node));
	else
		return fdt_address_cells(gd->fdt_blob, ofnode_to_offset(node));
}

int ofnode_read_simple_size_cells(ofnode node)
{
	if (ofnode_is_np(node))
		return of_simple_size_cells(ofnode_to_np(node));
	else
		return fdt_size_cells(gd->fdt_blob, ofnode_to_offset(node));
}

bool ofnode_pre_reloc(ofnode node)
{
#if defined(CONFIG_SPL_BUILD) || defined(CONFIG_TPL_BUILD)
	/* for SPL and TPL the remaining nodes after the fdtgrep 1st pass
	 * had property dm-pre-reloc or u-boot,dm-spl/tpl.
	 * They are removed in final dtb (fdtgrep 2nd pass)
	 */
	return true;
#else
	if (ofnode_read_bool(node, "u-boot,dm-pre-reloc"))
		return true;
	if (ofnode_read_bool(node, "u-boot,dm-pre-proper"))
		return true;

	/*
	 * In regular builds individual spl and tpl handling both
	 * count as handled pre-relocation for later second init.
	 */
	if (ofnode_read_bool(node, "u-boot,dm-spl") ||
	    ofnode_read_bool(node, "u-boot,dm-tpl"))
		return true;

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

		ret = fdt_get_resource(gd->fdt_blob, ofnode_to_offset(node),
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
		return fdt_translate_address(gd->fdt_blob, ofnode_to_offset(node), in_addr);
}

int ofnode_device_is_compatible(ofnode node, const char *compat)
{
	if (ofnode_is_np(node))
		return of_device_is_compatible(ofnode_to_np(node), compat,
					       NULL, NULL);
	else
		return !fdt_node_check_compatible(gd->fdt_blob,
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
		return offset_to_ofnode(fdt_node_offset_by_compatible(
				gd->fdt_blob, ofnode_to_offset(from), compat));
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
		return offset_to_ofnode(fdt_node_offset_by_prop_value(
				gd->fdt_blob, ofnode_to_offset(from),
				propname, propval, proplen));
	}
}

int ofnode_write_prop(ofnode node, const char *propname, int len,
		      const void *value)
{
	const struct device_node *np = ofnode_to_np(node);
	struct property *pp;
	struct property *pp_last = NULL;
	struct property *new;

	if (!of_live_active())
		return -ENOSYS;

	if (!np)
		return -EINVAL;

	for (pp = np->properties; pp; pp = pp->next) {
		if (strcmp(pp->name, propname) == 0) {
			/* Property exists -> change value */
			pp->value = (void *)value;
			pp->length = len;
			return 0;
		}
		pp_last = pp;
	}

	if (!pp_last)
		return -ENOENT;

	/* Property does not exist -> append new property */
	new = malloc(sizeof(struct property));
	if (!new)
		return -ENOMEM;

	new->name = strdup(propname);
	if (!new->name) {
		free(new);
		return -ENOMEM;
	}

	new->value = (void *)value;
	new->length = len;
	new->next = NULL;

	pp_last->next = new;

	return 0;
}

int ofnode_write_string(ofnode node, const char *propname, const char *value)
{
	if (!of_live_active())
		return -ENOSYS;

	assert(ofnode_valid(node));

	debug("%s: %s = %s", __func__, propname, value);

	return ofnode_write_prop(node, propname, strlen(value) + 1, value);
}

int ofnode_set_enabled(ofnode node, bool value)
{
	if (!of_live_active())
		return -ENOSYS;

	assert(ofnode_valid(node));

	if (value)
		return ofnode_write_string(node, "status", "okay");
	else
		return ofnode_write_string(node, "status", "disable");
}
