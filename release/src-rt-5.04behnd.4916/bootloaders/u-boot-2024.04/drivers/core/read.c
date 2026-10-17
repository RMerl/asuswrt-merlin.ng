// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/of_access.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/ioport.h>

int dev_read_u8(const struct udevice *dev, const char *propname, u8 *outp)
{
	return ofnode_read_u8(dev_ofnode(dev), propname, outp);
}

u8 dev_read_u8_default(const struct udevice *dev, const char *propname, u8 def)
{
	return ofnode_read_u8_default(dev_ofnode(dev), propname, def);
}

int dev_read_u16(const struct udevice *dev, const char *propname, u16 *outp)
{
	return ofnode_read_u16(dev_ofnode(dev), propname, outp);
}

u16 dev_read_u16_default(const struct udevice *dev, const char *propname,
			 u16 def)
{
	return ofnode_read_u16_default(dev_ofnode(dev), propname, def);
}

int dev_read_u32(const struct udevice *dev, const char *propname, u32 *outp)
{
	return ofnode_read_u32(dev_ofnode(dev), propname, outp);
}

int dev_read_u32_default(const struct udevice *dev, const char *propname,
			 int def)
{
	return ofnode_read_u32_default(dev_ofnode(dev), propname, def);
}

int dev_read_u32_index(struct udevice *dev, const char *propname, int index,
		       u32 *outp)
{
	return ofnode_read_u32_index(dev_ofnode(dev), propname, index, outp);
}

u32 dev_read_u32_index_default(struct udevice *dev, const char *propname,
			       int index, u32 def)
{
	return ofnode_read_u32_index_default(dev_ofnode(dev), propname, index,
					     def);
}

int dev_read_s32(const struct udevice *dev, const char *propname, s32 *outp)
{
	return ofnode_read_u32(dev_ofnode(dev), propname, (u32 *)outp);
}

int dev_read_s32_default(const struct udevice *dev, const char *propname,
			 int def)
{
	return ofnode_read_u32_default(dev_ofnode(dev), propname, def);
}

int dev_read_u32u(const struct udevice *dev, const char *propname, uint *outp)
{
	u32 val;
	int ret;

	ret = ofnode_read_u32(dev_ofnode(dev), propname, &val);
	if (ret)
		return ret;
	*outp = val;

	return 0;
}

int dev_read_u64(const struct udevice *dev, const char *propname, u64 *outp)
{
	return ofnode_read_u64(dev_ofnode(dev), propname, outp);
}

u64 dev_read_u64_default(const struct udevice *dev, const char *propname,
			 u64 def)
{
	return ofnode_read_u64_default(dev_ofnode(dev), propname, def);
}

const char *dev_read_string(const struct udevice *dev, const char *propname)
{
	return ofnode_read_string(dev_ofnode(dev), propname);
}

bool dev_read_bool(const struct udevice *dev, const char *propname)
{
	return ofnode_read_bool(dev_ofnode(dev), propname);
}

ofnode dev_read_subnode(const struct udevice *dev, const char *subnode_name)
{
	return ofnode_find_subnode(dev_ofnode(dev), subnode_name);
}

ofnode dev_read_first_subnode(const struct udevice *dev)
{
	return ofnode_first_subnode(dev_ofnode(dev));
}

ofnode dev_read_next_subnode(ofnode node)
{
	return ofnode_next_subnode(node);
}

int dev_read_size(const struct udevice *dev, const char *propname)
{
	return ofnode_read_size(dev_ofnode(dev), propname);
}

fdt_addr_t dev_read_addr_index(const struct udevice *dev, int index)
{
	if (ofnode_is_np(dev_ofnode(dev)))
		return ofnode_get_addr_index(dev_ofnode(dev), index);
	else
		return devfdt_get_addr_index(dev, index);
}

void *dev_read_addr_index_ptr(const struct udevice *dev, int index)
{
	fdt_addr_t addr = dev_read_addr_index(dev, index);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_sysmem(addr, 0);
}

fdt_addr_t dev_read_addr_size_index(const struct udevice *dev, int index,
				    fdt_size_t *size)
{
	if (ofnode_is_np(dev_ofnode(dev)))
		return ofnode_get_addr_size_index(dev_ofnode(dev), index, size);
	else
		return devfdt_get_addr_size_index(dev, index, size);
}

void *dev_read_addr_size_index_ptr(const struct udevice *dev, int index,
				   fdt_size_t *size)
{
	fdt_addr_t addr = dev_read_addr_size_index(dev, index, size);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_sysmem(addr, 0);
}

void *dev_remap_addr_index(const struct udevice *dev, int index)
{
	fdt_addr_t addr = dev_read_addr_index(dev, index);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_physmem(addr, 0, MAP_NOCACHE);
}

fdt_addr_t dev_read_addr_name(const struct udevice *dev, const char *name)
{
	int index = dev_read_stringlist_search(dev, "reg-names", name);

	if (index < 0)
		return FDT_ADDR_T_NONE;
	else
		return dev_read_addr_index(dev, index);
}

void *dev_read_addr_name_ptr(const struct udevice *dev, const char *name)
{
	fdt_addr_t addr = dev_read_addr_name(dev, name);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_sysmem(addr, 0);
}

fdt_addr_t dev_read_addr_size_name(const struct udevice *dev, const char *name,
				   fdt_size_t *size)
{
	int index = dev_read_stringlist_search(dev, "reg-names", name);

	if (index < 0)
		return FDT_ADDR_T_NONE;
	else
		return dev_read_addr_size_index(dev, index, size);
}

void *dev_read_addr_size_name_ptr(const struct udevice *dev, const char *name,
				  fdt_size_t *size)
{
	fdt_addr_t addr = dev_read_addr_size_name(dev, name, size);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_sysmem(addr, 0);
}

void *dev_remap_addr_name(const struct udevice *dev, const char *name)
{
	fdt_addr_t addr = dev_read_addr_name(dev, name);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_physmem(addr, 0, MAP_NOCACHE);
}

fdt_addr_t dev_read_addr(const struct udevice *dev)
{
	return dev_read_addr_index(dev, 0);
}

void *dev_read_addr_ptr(const struct udevice *dev)
{
	fdt_addr_t addr = dev_read_addr(dev);

	if (addr == FDT_ADDR_T_NONE)
		return NULL;

	return map_sysmem(addr, 0);
}

void *dev_remap_addr(const struct udevice *dev)
{
	return dev_remap_addr_index(dev, 0);
}

fdt_addr_t dev_read_addr_size(const struct udevice *dev, fdt_size_t *sizep)
{
	return dev_read_addr_size_index(dev, 0, sizep);
}

const char *dev_read_name(const struct udevice *dev)
{
	return ofnode_get_name(dev_ofnode(dev));
}

int dev_read_stringlist_search(const struct udevice *dev, const char *property,
			       const char *string)
{
	return ofnode_stringlist_search(dev_ofnode(dev), property, string);
}

int dev_read_string_index(const struct udevice *dev, const char *propname,
			  int index, const char **outp)
{
	return ofnode_read_string_index(dev_ofnode(dev), propname, index, outp);
}

int dev_read_string_count(const struct udevice *dev, const char *propname)
{
	return ofnode_read_string_count(dev_ofnode(dev), propname);
}

int dev_read_string_list(const struct udevice *dev, const char *propname,
			 const char ***listp)
{
	return ofnode_read_string_list(dev_ofnode(dev), propname, listp);
}

int dev_read_phandle_with_args(const struct udevice *dev, const char *list_name,
			       const char *cells_name, int cell_count,
			       int index, struct ofnode_phandle_args *out_args)
{
	return ofnode_parse_phandle_with_args(dev_ofnode(dev), list_name,
					      cells_name, cell_count, index,
					      out_args);
}

int dev_count_phandle_with_args(const struct udevice *dev,
				const char *list_name, const char *cells_name,
				int cell_count)
{
	return ofnode_count_phandle_with_args(dev_ofnode(dev), list_name,
					      cells_name, cell_count);
}

int dev_read_addr_cells(const struct udevice *dev)
{
	return ofnode_read_addr_cells(dev_ofnode(dev));
}

int dev_read_size_cells(const struct udevice *dev)
{
	return ofnode_read_size_cells(dev_ofnode(dev));
}

int dev_read_simple_addr_cells(const struct udevice *dev)
{
	return ofnode_read_simple_addr_cells(dev_ofnode(dev));
}

int dev_read_simple_size_cells(const struct udevice *dev)
{
	return ofnode_read_simple_size_cells(dev_ofnode(dev));
}

int dev_read_phandle(const struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);

	if (ofnode_is_np(node))
		return ofnode_to_np(node)->phandle;
	else
		return fdt_get_phandle(gd->fdt_blob, ofnode_to_offset(node));
}

const void *dev_read_prop(const struct udevice *dev, const char *propname,
			  int *lenp)
{
	return ofnode_get_property(dev_ofnode(dev), propname, lenp);
}

int dev_read_first_prop(const struct udevice *dev, struct ofprop *prop)
{
	return ofnode_first_property(dev_ofnode(dev), prop);
}

int dev_read_next_prop(struct ofprop *prop)
{
	return ofnode_next_property(prop);
}

const void *dev_read_prop_by_prop(struct ofprop *prop,
				  const char **propname, int *lenp)
{
	return ofprop_get_property(prop, propname, lenp);
}

int dev_read_alias_seq(const struct udevice *dev, int *devnump)
{
	ofnode node = dev_ofnode(dev);
	const char *uc_name = dev->uclass->uc_drv->name;
	int ret = -ENOTSUPP;

	if (ofnode_is_np(node)) {
		ret = of_alias_get_id(ofnode_to_np(node), uc_name);
		if (ret >= 0) {
			*devnump = ret;
			ret = 0;
		}
	} else {
#if CONFIG_IS_ENABLED(OF_CONTROL)
		ret = fdtdec_get_alias_seq(gd->fdt_blob, uc_name,
					   ofnode_to_offset(node), devnump);
#endif
	}

	return ret;
}

int dev_read_u32_array(const struct udevice *dev, const char *propname,
		       u32 *out_values, size_t sz)
{
	return ofnode_read_u32_array(dev_ofnode(dev), propname, out_values, sz);
}

const uint8_t *dev_read_u8_array_ptr(const struct udevice *dev,
				     const char *propname, size_t sz)
{
	return ofnode_read_u8_array_ptr(dev_ofnode(dev), propname, sz);
}

int dev_read_enabled(const struct udevice *dev)
{
	ofnode node = dev_ofnode(dev);

	if (ofnode_is_np(node))
		return of_device_is_available(ofnode_to_np(node));
	else
		return fdtdec_get_is_enabled(gd->fdt_blob,
					     ofnode_to_offset(node));
}

int dev_read_resource(const struct udevice *dev, uint index,
		      struct resource *res)
{
	return ofnode_read_resource(dev_ofnode(dev), index, res);
}

int dev_read_resource_byname(const struct udevice *dev, const char *name,
			     struct resource *res)
{
	return ofnode_read_resource_byname(dev_ofnode(dev), name, res);
}

u64 dev_translate_address(const struct udevice *dev, const fdt32_t *in_addr)
{
	return ofnode_translate_address(dev_ofnode(dev), in_addr);
}

u64 dev_translate_dma_address(const struct udevice *dev, const fdt32_t *in_addr)
{
	return ofnode_translate_dma_address(dev_ofnode(dev), in_addr);
}

int dev_get_dma_range(const struct udevice *dev, phys_addr_t *cpu,
		      dma_addr_t *bus, u64 *size)
{
	return ofnode_get_dma_range(dev_ofnode(dev), cpu, bus, size);
}

int dev_read_alias_highest_id(const char *stem)
{
	if (of_live_active())
		return of_alias_get_highest_id(stem);

	return fdtdec_get_alias_highest_id(gd->fdt_blob, stem);
}

fdt_addr_t dev_read_addr_pci(const struct udevice *dev, fdt_size_t *sizep)
{
	ulong addr;

	addr = dev_read_addr(dev);
	if (sizep)
		*sizep = 0;
	if (addr == FDT_ADDR_T_NONE && !of_live_active())
		addr = devfdt_get_addr_pci(dev, sizep);

	return addr;
}

int dev_get_child_count(const struct udevice *dev)
{
	return ofnode_get_child_count(dev_ofnode(dev));
}

int dev_read_pci_bus_range(const struct udevice *dev,
			   struct resource *res)
{
	const u32 *values;
	int len;

	values = dev_read_prop(dev, "bus-range", &len);
	if (!values || len < sizeof(*values) * 2)
		return -EINVAL;

	res->start = *values++;
	res->end = *values;

	return 0;
}

int dev_decode_display_timing(const struct udevice *dev, int index,
			      struct display_timing *config)
{
	return ofnode_decode_display_timing(dev_ofnode(dev), index, config);
}

int dev_decode_panel_timing(const struct udevice *dev,
			    struct display_timing *config)
{
	return ofnode_decode_panel_timing(dev_ofnode(dev), config);
}

ofnode dev_get_phy_node(const struct udevice *dev)
{
	return ofnode_get_phy_node(dev_ofnode(dev));
}

phy_interface_t dev_read_phy_mode(const struct udevice *dev)
{
	return ofnode_read_phy_mode(dev_ofnode(dev));
}
