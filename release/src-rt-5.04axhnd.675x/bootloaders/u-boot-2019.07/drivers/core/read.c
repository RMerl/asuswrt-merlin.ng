// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <asm/types.h>
#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <dm/of_access.h>

int dev_read_u32(struct udevice *dev, const char *propname, u32 *outp)
{
	return ofnode_read_u32(dev_ofnode(dev), propname, outp);
}

int dev_read_u32_default(struct udevice *dev, const char *propname, int def)
{
	return ofnode_read_u32_default(dev_ofnode(dev), propname, def);
}

int dev_read_s32(struct udevice *dev, const char *propname, s32 *outp)
{
	return ofnode_read_u32(dev_ofnode(dev), propname, (u32 *)outp);
}

int dev_read_s32_default(struct udevice *dev, const char *propname, int def)
{
	return ofnode_read_u32_default(dev_ofnode(dev), propname, def);
}

int dev_read_u32u(struct udevice *dev, const char *propname, uint *outp)
{
	u32 val;
	int ret;

	ret = ofnode_read_u32(dev_ofnode(dev), propname, &val);
	if (ret)
		return ret;
	*outp = val;

	return 0;
}

const char *dev_read_string(struct udevice *dev, const char *propname)
{
	return ofnode_read_string(dev_ofnode(dev), propname);
}

bool dev_read_bool(struct udevice *dev, const char *propname)
{
	return ofnode_read_bool(dev_ofnode(dev), propname);
}

ofnode dev_read_subnode(struct udevice *dev, const char *subnode_name)
{
	return ofnode_find_subnode(dev_ofnode(dev), subnode_name);
}

ofnode dev_read_first_subnode(struct udevice *dev)
{
	return ofnode_first_subnode(dev_ofnode(dev));
}

ofnode dev_read_next_subnode(ofnode node)
{
	return ofnode_next_subnode(node);
}

int dev_read_size(struct udevice *dev, const char *propname)
{
	return ofnode_read_size(dev_ofnode(dev), propname);
}

fdt_addr_t dev_read_addr_index(struct udevice *dev, int index)
{
	if (ofnode_is_np(dev_ofnode(dev)))
		return ofnode_get_addr_index(dev_ofnode(dev), index);
	else
		return devfdt_get_addr_index(dev, index);
}

void *dev_remap_addr_index(struct udevice *dev, int index)
{
	fdt_addr_t addr = dev_read_addr_index(dev, index);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_physmem(addr, 0, MAP_NOCACHE);
}

fdt_addr_t dev_read_addr_name(struct udevice *dev, const char *name)
{
	int index = dev_read_stringlist_search(dev, "reg-names", name);

	if (index < 0)
		return FDT_ADDR_T_NONE;
	else
		return dev_read_addr_index(dev, index);
}

void *dev_remap_addr_name(struct udevice *dev, const char *name)
{
	fdt_addr_t addr = dev_read_addr_name(dev, name);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_physmem(addr, 0, MAP_NOCACHE);
}

fdt_addr_t dev_read_addr(struct udevice *dev)
{
	return dev_read_addr_index(dev, 0);
}

void *dev_read_addr_ptr(struct udevice *dev)
{
	fdt_addr_t addr = dev_read_addr(dev);

	return (addr == FDT_ADDR_T_NONE) ? NULL : map_sysmem(addr, 0);
}

void *dev_remap_addr(struct udevice *dev)
{
	return dev_remap_addr_index(dev, 0);
}

fdt_addr_t dev_read_addr_size(struct udevice *dev, const char *property,
			      fdt_size_t *sizep)
{
	return ofnode_get_addr_size(dev_ofnode(dev), property, sizep);
}

const char *dev_read_name(struct udevice *dev)
{
	return ofnode_get_name(dev_ofnode(dev));
}

int dev_read_stringlist_search(struct udevice *dev, const char *property,
			       const char *string)
{
	return ofnode_stringlist_search(dev_ofnode(dev), property, string);
}

int dev_read_string_index(struct udevice *dev, const char *propname, int index,
			  const char **outp)
{
	return ofnode_read_string_index(dev_ofnode(dev), propname, index, outp);
}

int dev_read_string_count(struct udevice *dev, const char *propname)
{
	return ofnode_read_string_count(dev_ofnode(dev), propname);
}

int dev_read_phandle_with_args(struct udevice *dev, const char *list_name,
			       const char *cells_name, int cell_count,
			       int index, struct ofnode_phandle_args *out_args)
{
	return ofnode_parse_phandle_with_args(dev_ofnode(dev), list_name,
					      cells_name, cell_count, index,
					      out_args);
}

int dev_count_phandle_with_args(struct udevice *dev, const char *list_name,
				const char *cells_name)
{
	return ofnode_count_phandle_with_args(dev_ofnode(dev), list_name,
					      cells_name);
}

int dev_read_addr_cells(struct udevice *dev)
{
	return ofnode_read_addr_cells(dev_ofnode(dev));
}

int dev_read_size_cells(struct udevice *dev)
{
	return ofnode_read_size_cells(dev_ofnode(dev));
}

int dev_read_simple_addr_cells(struct udevice *dev)
{
	return ofnode_read_simple_addr_cells(dev_ofnode(dev));
}

int dev_read_simple_size_cells(struct udevice *dev)
{
	return ofnode_read_simple_size_cells(dev_ofnode(dev));
}

int dev_read_phandle(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);

	if (ofnode_is_np(node))
		return ofnode_to_np(node)->phandle;
	else
		return fdt_get_phandle(gd->fdt_blob, ofnode_to_offset(node));
}

const void *dev_read_prop(struct udevice *dev, const char *propname, int *lenp)
{
	return ofnode_get_property(dev_ofnode(dev), propname, lenp);
}

int dev_read_alias_seq(struct udevice *dev, int *devnump)
{
	ofnode node = dev_ofnode(dev);
	const char *uc_name = dev->uclass->uc_drv->name;
	int ret;

	if (ofnode_is_np(node)) {
		ret = of_alias_get_id(ofnode_to_np(node), uc_name);
		if (ret >= 0)
			*devnump = ret;
	} else {
		ret = fdtdec_get_alias_seq(gd->fdt_blob, uc_name,
					   ofnode_to_offset(node), devnump);
	}

	return ret;
}

int dev_read_u32_array(struct udevice *dev, const char *propname,
		       u32 *out_values, size_t sz)
{
	return ofnode_read_u32_array(dev_ofnode(dev), propname, out_values, sz);
}

const uint8_t *dev_read_u8_array_ptr(struct udevice *dev, const char *propname,
				     size_t sz)
{
	return ofnode_read_u8_array_ptr(dev_ofnode(dev), propname, sz);
}

int dev_read_enabled(struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);

	if (ofnode_is_np(node))
		return of_device_is_available(ofnode_to_np(node));
	else
		return fdtdec_get_is_enabled(gd->fdt_blob,
					     ofnode_to_offset(node));
}

int dev_read_resource(struct udevice *dev, uint index, struct resource *res)
{
	return ofnode_read_resource(dev_ofnode(dev), index, res);
}

int dev_read_resource_byname(struct udevice *dev, const char *name,
			     struct resource *res)
{
	return ofnode_read_resource_byname(dev_ofnode(dev), name, res);
}

u64 dev_translate_address(struct udevice *dev, const fdt32_t *in_addr)
{
	return ofnode_translate_address(dev_ofnode(dev), in_addr);
}

int dev_read_alias_highest_id(const char *stem)
{
	if (of_live_active())
		return of_alias_get_highest_id(stem);

	return fdtdec_get_alias_highest_id(gd->fdt_blob, stem);
}
