// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <mapmem.h>
#include <stdio_dev.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <exports.h>
#include <fdtdec.h>

/**
 * fdt_getprop_u32_default_node - Return a node's property or a default
 *
 * @fdt: ptr to device tree
 * @off: offset of node
 * @cell: cell offset in property
 * @prop: property name
 * @dflt: default value if the property isn't found
 *
 * Convenience function to return a node's property or a default value if
 * the property doesn't exist.
 */
u32 fdt_getprop_u32_default_node(const void *fdt, int off, int cell,
				const char *prop, const u32 dflt)
{
	const fdt32_t *val;
	int len;

	val = fdt_getprop(fdt, off, prop, &len);

	/* Check if property exists */
	if (!val)
		return dflt;

	/* Check if property is long enough */
	if (len < ((cell + 1) * sizeof(uint32_t)))
		return dflt;

	return fdt32_to_cpu(*val);
}

/**
 * fdt_getprop_u32_default - Find a node and return it's property or a default
 *
 * @fdt: ptr to device tree
 * @path: path of node
 * @prop: property name
 * @dflt: default value if the property isn't found
 *
 * Convenience function to find a node and return it's property or a
 * default value if it doesn't exist.
 */
u32 fdt_getprop_u32_default(const void *fdt, const char *path,
				const char *prop, const u32 dflt)
{
	int off;

	off = fdt_path_offset(fdt, path);
	if (off < 0)
		return dflt;

	return fdt_getprop_u32_default_node(fdt, off, 0, prop, dflt);
}

/**
 * fdt_find_and_setprop: Find a node and set it's property
 *
 * @fdt: ptr to device tree
 * @node: path of node
 * @prop: property name
 * @val: ptr to new value
 * @len: length of new property value
 * @create: flag to create the property if it doesn't exist
 *
 * Convenience function to directly set a property given the path to the node.
 */
int fdt_find_and_setprop(void *fdt, const char *node, const char *prop,
			 const void *val, int len, int create)
{
	int nodeoff = fdt_path_offset(fdt, node);

	if (nodeoff < 0)
		return nodeoff;

	if ((!create) && (fdt_get_property(fdt, nodeoff, prop, NULL) == NULL))
		return 0; /* create flag not set; so exit quietly */

	return fdt_setprop(fdt, nodeoff, prop, val, len);
}

/**
 * fdt_find_or_add_subnode() - find or possibly add a subnode of a given node
 *
 * @fdt: pointer to the device tree blob
 * @parentoffset: structure block offset of a node
 * @name: name of the subnode to locate
 *
 * fdt_subnode_offset() finds a subnode of the node with a given name.
 * If the subnode does not exist, it will be created.
 */
int fdt_find_or_add_subnode(void *fdt, int parentoffset, const char *name)
{
	int offset;

	offset = fdt_subnode_offset(fdt, parentoffset, name);

	if (offset == -FDT_ERR_NOTFOUND)
		offset = fdt_add_subnode(fdt, parentoffset, name);

	if (offset < 0)
		printf("%s: %s: %s\n", __func__, name, fdt_strerror(offset));

	return offset;
}

/* rename to CONFIG_OF_STDOUT_PATH ? */
#if defined(OF_STDOUT_PATH)
static int fdt_fixup_stdout(void *fdt, int chosenoff)
{
	return fdt_setprop(fdt, chosenoff, "linux,stdout-path",
			      OF_STDOUT_PATH, strlen(OF_STDOUT_PATH) + 1);
}
#elif defined(CONFIG_OF_STDOUT_VIA_ALIAS) && defined(CONFIG_CONS_INDEX)
static int fdt_fixup_stdout(void *fdt, int chosenoff)
{
	int err;
	int aliasoff;
	char sername[9] = { 0 };
	const void *path;
	int len;
	char tmp[256]; /* long enough */

	sprintf(sername, "serial%d", CONFIG_CONS_INDEX - 1);

	aliasoff = fdt_path_offset(fdt, "/aliases");
	if (aliasoff < 0) {
		err = aliasoff;
		goto noalias;
	}

	path = fdt_getprop(fdt, aliasoff, sername, &len);
	if (!path) {
		err = len;
		goto noalias;
	}

	/* fdt_setprop may break "path" so we copy it to tmp buffer */
	memcpy(tmp, path, len);

	err = fdt_setprop(fdt, chosenoff, "linux,stdout-path", tmp, len);
	if (err < 0)
		printf("WARNING: could not set linux,stdout-path %s.\n",
		       fdt_strerror(err));

	return err;

noalias:
	printf("WARNING: %s: could not read %s alias: %s\n",
	       __func__, sername, fdt_strerror(err));

	return 0;
}
#else
static int fdt_fixup_stdout(void *fdt, int chosenoff)
{
	return 0;
}
#endif

static inline int fdt_setprop_uxx(void *fdt, int nodeoffset, const char *name,
				  uint64_t val, int is_u64)
{
	if (is_u64)
		return fdt_setprop_u64(fdt, nodeoffset, name, val);
	else
		return fdt_setprop_u32(fdt, nodeoffset, name, (uint32_t)val);
}

int fdt_root(void *fdt)
{
	char *serial;
	int err;

	err = fdt_check_header(fdt);
	if (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		return err;
	}

	serial = env_get("serial#");
	if (serial) {
		err = fdt_setprop(fdt, 0, "serial-number", serial,
				  strlen(serial) + 1);

		if (err < 0) {
			printf("WARNING: could not set serial-number %s.\n",
			       fdt_strerror(err));
			return err;
		}
	}

	return 0;
}

int fdt_initrd(void *fdt, ulong initrd_start, ulong initrd_end)
{
	int   nodeoffset;
	int   err, j, total;
	int is_u64;
	uint64_t addr, size;

	/* just return if the size of initrd is zero */
	if (initrd_start == initrd_end)
		return 0;

	/* find or create "/chosen" node. */
	nodeoffset = fdt_find_or_add_subnode(fdt, 0, "chosen");
	if (nodeoffset < 0)
		return nodeoffset;

	total = fdt_num_mem_rsv(fdt);

	/*
	 * Look for an existing entry and update it.  If we don't find
	 * the entry, we will j be the next available slot.
	 */
	for (j = 0; j < total; j++) {
		err = fdt_get_mem_rsv(fdt, j, &addr, &size);
		if (addr == initrd_start) {
			fdt_del_mem_rsv(fdt, j);
			break;
		}
	}

	err = fdt_add_mem_rsv(fdt, initrd_start, initrd_end - initrd_start);
	if (err < 0) {
		printf("fdt_initrd: %s\n", fdt_strerror(err));
		return err;
	}

	is_u64 = (fdt_address_cells(fdt, 0) == 2);

	err = fdt_setprop_uxx(fdt, nodeoffset, "linux,initrd-start",
			      (uint64_t)initrd_start, is_u64);

	if (err < 0) {
		printf("WARNING: could not set linux,initrd-start %s.\n",
		       fdt_strerror(err));
		return err;
	}

	err = fdt_setprop_uxx(fdt, nodeoffset, "linux,initrd-end",
			      (uint64_t)initrd_end, is_u64);

	if (err < 0) {
		printf("WARNING: could not set linux,initrd-end %s.\n",
		       fdt_strerror(err));

		return err;
	}

	return 0;
}

int fdt_chosen(void *fdt)
{
	int   nodeoffset;
	int   err;
	char  *str;		/* used to set string properties */

	err = fdt_check_header(fdt);
	if (err < 0) {
		printf("fdt_chosen: %s\n", fdt_strerror(err));
		return err;
	}

	/* find or create "/chosen" node. */
	nodeoffset = fdt_find_or_add_subnode(fdt, 0, "chosen");
	if (nodeoffset < 0)
		return nodeoffset;

	str = env_get("bootargs");
	if (str) {
		err = fdt_setprop(fdt, nodeoffset, "bootargs", str,
				  strlen(str) + 1);
		if (err < 0) {
			printf("WARNING: could not set bootargs %s.\n",
			       fdt_strerror(err));
			return err;
		}
	}

	return fdt_fixup_stdout(fdt, nodeoffset);
}

void do_fixup_by_path(void *fdt, const char *path, const char *prop,
		      const void *val, int len, int create)
{
#if defined(DEBUG)
	int i;
	debug("Updating property '%s/%s' = ", path, prop);
	for (i = 0; i < len; i++)
		debug(" %.2x", *(u8*)(val+i));
	debug("\n");
#endif
	int rc = fdt_find_and_setprop(fdt, path, prop, val, len, create);
	if (rc)
		printf("Unable to update property %s:%s, err=%s\n",
			path, prop, fdt_strerror(rc));
}

void do_fixup_by_path_u32(void *fdt, const char *path, const char *prop,
			  u32 val, int create)
{
	fdt32_t tmp = cpu_to_fdt32(val);
	do_fixup_by_path(fdt, path, prop, &tmp, sizeof(tmp), create);
}

void do_fixup_by_prop(void *fdt,
		      const char *pname, const void *pval, int plen,
		      const char *prop, const void *val, int len,
		      int create)
{
	int off;
#if defined(DEBUG)
	int i;
	debug("Updating property '%s' = ", prop);
	for (i = 0; i < len; i++)
		debug(" %.2x", *(u8*)(val+i));
	debug("\n");
#endif
	off = fdt_node_offset_by_prop_value(fdt, -1, pname, pval, plen);
	while (off != -FDT_ERR_NOTFOUND) {
		if (create || (fdt_get_property(fdt, off, prop, NULL) != NULL))
			fdt_setprop(fdt, off, prop, val, len);
		off = fdt_node_offset_by_prop_value(fdt, off, pname, pval, plen);
	}
}

void do_fixup_by_prop_u32(void *fdt,
			  const char *pname, const void *pval, int plen,
			  const char *prop, u32 val, int create)
{
	fdt32_t tmp = cpu_to_fdt32(val);
	do_fixup_by_prop(fdt, pname, pval, plen, prop, &tmp, 4, create);
}

void do_fixup_by_compat(void *fdt, const char *compat,
			const char *prop, const void *val, int len, int create)
{
	int off = -1;
#if defined(DEBUG)
	int i;
	debug("Updating property '%s' = ", prop);
	for (i = 0; i < len; i++)
		debug(" %.2x", *(u8*)(val+i));
	debug("\n");
#endif
	off = fdt_node_offset_by_compatible(fdt, -1, compat);
	while (off != -FDT_ERR_NOTFOUND) {
		if (create || (fdt_get_property(fdt, off, prop, NULL) != NULL))
			fdt_setprop(fdt, off, prop, val, len);
		off = fdt_node_offset_by_compatible(fdt, off, compat);
	}
}

void do_fixup_by_compat_u32(void *fdt, const char *compat,
			    const char *prop, u32 val, int create)
{
	fdt32_t tmp = cpu_to_fdt32(val);
	do_fixup_by_compat(fdt, compat, prop, &tmp, 4, create);
}

#ifdef CONFIG_ARCH_FIXUP_FDT_MEMORY
/*
 * fdt_pack_reg - pack address and size array into the "reg"-suitable stream
 */
static int fdt_pack_reg(const void *fdt, void *buf, u64 *address, u64 *size,
			int n)
{
	int i;
	int address_cells = fdt_address_cells(fdt, 0);
	int size_cells = fdt_size_cells(fdt, 0);
	char *p = buf;

	for (i = 0; i < n; i++) {
		if (address_cells == 2)
			*(fdt64_t *)p = cpu_to_fdt64(address[i]);
		else
			*(fdt32_t *)p = cpu_to_fdt32(address[i]);
		p += 4 * address_cells;

		if (size_cells == 2)
			*(fdt64_t *)p = cpu_to_fdt64(size[i]);
		else
			*(fdt32_t *)p = cpu_to_fdt32(size[i]);
		p += 4 * size_cells;
	}

	return p - (char *)buf;
}

#if CONFIG_NR_DRAM_BANKS > 4
#define MEMORY_BANKS_MAX CONFIG_NR_DRAM_BANKS
#else
#define MEMORY_BANKS_MAX 4
#endif
int fdt_fixup_memory_banks(void *blob, u64 start[], u64 size[], int banks)
{
	int err, nodeoffset;
	int len, i;
	u8 tmp[MEMORY_BANKS_MAX * 16]; /* Up to 64-bit address + 64-bit size */

	if (banks > MEMORY_BANKS_MAX) {
		printf("%s: num banks %d exceeds hardcoded limit %d."
		       " Recompile with higher MEMORY_BANKS_MAX?\n",
		       __FUNCTION__, banks, MEMORY_BANKS_MAX);
		return -1;
	}

	err = fdt_check_header(blob);
	if (err < 0) {
		printf("%s: %s\n", __FUNCTION__, fdt_strerror(err));
		return err;
	}

	/* find or create "/memory" node. */
	nodeoffset = fdt_find_or_add_subnode(blob, 0, "memory");
	if (nodeoffset < 0)
			return nodeoffset;

	err = fdt_setprop(blob, nodeoffset, "device_type", "memory",
			sizeof("memory"));
	if (err < 0) {
		printf("WARNING: could not set %s %s.\n", "device_type",
				fdt_strerror(err));
		return err;
	}

	for (i = 0; i < banks; i++) {
		if (start[i] == 0 && size[i] == 0)
			break;
	}

	banks = i;

	if (!banks)
		return 0;

	len = fdt_pack_reg(blob, tmp, start, size, banks);

	err = fdt_setprop(blob, nodeoffset, "reg", tmp, len);
	if (err < 0) {
		printf("WARNING: could not set %s %s.\n",
				"reg", fdt_strerror(err));
		return err;
	}
	return 0;
}
#endif

int fdt_fixup_memory(void *blob, u64 start, u64 size)
{
	return fdt_fixup_memory_banks(blob, &start, &size, 1);
}

void fdt_fixup_ethernet(void *fdt)
{
	int i = 0, j, prop;
	char *tmp, *end;
	char mac[16];
	const char *path;
	unsigned char mac_addr[ARP_HLEN];
	int offset;
#ifdef FDT_SEQ_MACADDR_FROM_ENV
	int nodeoff;
	const struct fdt_property *fdt_prop;
#endif

	if (fdt_path_offset(fdt, "/aliases") < 0)
		return;

	/* Cycle through all aliases */
	for (prop = 0; ; prop++) {
		const char *name;

		/* FDT might have been edited, recompute the offset */
		offset = fdt_first_property_offset(fdt,
			fdt_path_offset(fdt, "/aliases"));
		/* Select property number 'prop' */
		for (j = 0; j < prop; j++)
			offset = fdt_next_property_offset(fdt, offset);

		if (offset < 0)
			break;

		path = fdt_getprop_by_offset(fdt, offset, &name, NULL);
		if (!strncmp(name, "ethernet", 8)) {
			/* Treat plain "ethernet" same as "ethernet0". */
			if (!strcmp(name, "ethernet")
#ifdef FDT_SEQ_MACADDR_FROM_ENV
			 || !strcmp(name, "ethernet0")
#endif
			)
				i = 0;
#ifndef FDT_SEQ_MACADDR_FROM_ENV
			else
				i = trailing_strtol(name);
#endif
			if (i != -1) {
				if (i == 0)
					strcpy(mac, "ethaddr");
				else
					sprintf(mac, "eth%daddr", i);
			} else {
				continue;
			}
#ifdef FDT_SEQ_MACADDR_FROM_ENV
			nodeoff = fdt_path_offset(fdt, path);
			fdt_prop = fdt_get_property(fdt, nodeoff, "status",
						    NULL);
			if (fdt_prop && !strcmp(fdt_prop->data, "disabled"))
				continue;
			i++;
#endif
			tmp = env_get(mac);
			if (!tmp)
				continue;

			for (j = 0; j < 6; j++) {
				mac_addr[j] = tmp ?
					      simple_strtoul(tmp, &end, 16) : 0;
				if (tmp)
					tmp = (*end) ? end + 1 : end;
			}

			do_fixup_by_path(fdt, path, "mac-address",
					 &mac_addr, 6, 0);
			do_fixup_by_path(fdt, path, "local-mac-address",
					 &mac_addr, 6, 1);
		}
	}
}

int fdt_record_loadable(void *blob, u32 index, const char *name,
			uintptr_t load_addr, u32 size, uintptr_t entry_point,
			const char *type, const char *os)
{
	int err, node;

	err = fdt_check_header(blob);
	if (err < 0) {
		printf("%s: %s\n", __func__, fdt_strerror(err));
		return err;
	}

	/* find or create "/fit-images" node */
	node = fdt_find_or_add_subnode(blob, 0, "fit-images");
	if (node < 0)
		return node;

	/* find or create "/fit-images/<name>" node */
	node = fdt_find_or_add_subnode(blob, node, name);
	if (node < 0)
		return node;

	/*
	 * We record these as 32bit entities, possibly truncating addresses.
	 * However, spl_fit.c is not 64bit safe either: i.e. we should not
	 * have an issue here.
	 */
	fdt_setprop_u32(blob, node, "load-addr", load_addr);
	if (entry_point != -1)
		fdt_setprop_u32(blob, node, "entry-point", entry_point);
	fdt_setprop_u32(blob, node, "size", size);
	if (type)
		fdt_setprop_string(blob, node, "type", type);
	if (os)
		fdt_setprop_string(blob, node, "os", os);

	return node;
}

/* Resize the fdt to its actual size + a bit of padding */
int fdt_shrink_to_minimum(void *blob, uint extrasize)
{
	int i;
	uint64_t addr, size;
	int total, ret;
	uint actualsize;
	int fdt_memrsv = 0;

	if (!blob)
		return 0;

	total = fdt_num_mem_rsv(blob);
	for (i = 0; i < total; i++) {
		fdt_get_mem_rsv(blob, i, &addr, &size);
		if (addr == (uintptr_t)blob) {
			fdt_del_mem_rsv(blob, i);
			fdt_memrsv = 1;
			break;
		}
	}

	/*
	 * Calculate the actual size of the fdt
	 * plus the size needed for 5 fdt_add_mem_rsv, one
	 * for the fdt itself and 4 for a possible initrd
	 * ((initrd-start + initrd-end) * 2 (name & value))
	 */
	actualsize = fdt_off_dt_strings(blob) +
		fdt_size_dt_strings(blob) + 5 * sizeof(struct fdt_reserve_entry);

	actualsize += extrasize;
	/* Make it so the fdt ends on a page boundary */
	actualsize = ALIGN(actualsize + ((uintptr_t)blob & 0xfff), 0x1000);
	actualsize = actualsize - ((uintptr_t)blob & 0xfff);

	/* Change the fdt header to reflect the correct size */
	fdt_set_totalsize(blob, actualsize);

	if (fdt_memrsv) {
		/* Add the new reservation */
		ret = fdt_add_mem_rsv(blob, map_to_sysmem(blob), actualsize);
		if (ret < 0)
			return ret;
	}

	return actualsize;
}

#ifdef CONFIG_PCI
#define CONFIG_SYS_PCI_NR_INBOUND_WIN 4

#define FDT_PCI_PREFETCH	(0x40000000)
#define FDT_PCI_MEM32		(0x02000000)
#define FDT_PCI_IO		(0x01000000)
#define FDT_PCI_MEM64		(0x03000000)

int fdt_pci_dma_ranges(void *blob, int phb_off, struct pci_controller *hose) {

	int addrcell, sizecell, len, r;
	u32 *dma_range;
	/* sized based on pci addr cells, size-cells, & address-cells */
	u32 dma_ranges[(3 + 2 + 2) * CONFIG_SYS_PCI_NR_INBOUND_WIN];

	addrcell = fdt_getprop_u32_default(blob, "/", "#address-cells", 1);
	sizecell = fdt_getprop_u32_default(blob, "/", "#size-cells", 1);

	dma_range = &dma_ranges[0];
	for (r = 0; r < hose->region_count; r++) {
		u64 bus_start, phys_start, size;

		/* skip if !PCI_REGION_SYS_MEMORY */
		if (!(hose->regions[r].flags & PCI_REGION_SYS_MEMORY))
			continue;

		bus_start = (u64)hose->regions[r].bus_start;
		phys_start = (u64)hose->regions[r].phys_start;
		size = (u64)hose->regions[r].size;

		dma_range[0] = 0;
		if (size >= 0x100000000ull)
			dma_range[0] |= FDT_PCI_MEM64;
		else
			dma_range[0] |= FDT_PCI_MEM32;
		if (hose->regions[r].flags & PCI_REGION_PREFETCH)
			dma_range[0] |= FDT_PCI_PREFETCH;
#ifdef CONFIG_SYS_PCI_64BIT
		dma_range[1] = bus_start >> 32;
#else
		dma_range[1] = 0;
#endif
		dma_range[2] = bus_start & 0xffffffff;

		if (addrcell == 2) {
			dma_range[3] = phys_start >> 32;
			dma_range[4] = phys_start & 0xffffffff;
		} else {
			dma_range[3] = phys_start & 0xffffffff;
		}

		if (sizecell == 2) {
			dma_range[3 + addrcell + 0] = size >> 32;
			dma_range[3 + addrcell + 1] = size & 0xffffffff;
		} else {
			dma_range[3 + addrcell + 0] = size & 0xffffffff;
		}

		dma_range += (3 + addrcell + sizecell);
	}

	len = dma_range - &dma_ranges[0];
	if (len)
		fdt_setprop(blob, phb_off, "dma-ranges", &dma_ranges[0], len*4);

	return 0;
}
#endif

int fdt_increase_size(void *fdt, int add_len)
{
	int newlen;

	newlen = fdt_totalsize(fdt) + add_len;

	/* Open in place with a new len */
	return fdt_open_into(fdt, fdt, newlen);
}

#ifdef CONFIG_FDT_FIXUP_PARTITIONS
#include <jffs2/load_kernel.h>
#include <mtd_node.h>

static int fdt_del_subnodes(const void *blob, int parent_offset)
{
	int off, ndepth;
	int ret;

	for (ndepth = 0, off = fdt_next_node(blob, parent_offset, &ndepth);
	     (off >= 0) && (ndepth > 0);
	     off = fdt_next_node(blob, off, &ndepth)) {
		if (ndepth == 1) {
			debug("delete %s: offset: %x\n",
				fdt_get_name(blob, off, 0), off);
			ret = fdt_del_node((void *)blob, off);
			if (ret < 0) {
				printf("Can't delete node: %s\n",
					fdt_strerror(ret));
				return ret;
			} else {
				ndepth = 0;
				off = parent_offset;
			}
		}
	}
	return 0;
}

static int fdt_del_partitions(void *blob, int parent_offset)
{
	const void *prop;
	int ndepth = 0;
	int off;
	int ret;

	off = fdt_next_node(blob, parent_offset, &ndepth);
	if (off > 0 && ndepth == 1) {
		prop = fdt_getprop(blob, off, "label", NULL);
		if (prop == NULL) {
			/*
			 * Could not find label property, nand {}; node?
			 * Check subnode, delete partitions there if any.
			 */
			return fdt_del_partitions(blob, off);
		} else {
			ret = fdt_del_subnodes(blob, parent_offset);
			if (ret < 0) {
				printf("Can't remove subnodes: %s\n",
					fdt_strerror(ret));
				return ret;
			}
		}
	}
	return 0;
}

int fdt_node_set_part_info(void *blob, int parent_offset,
			   struct mtd_device *dev)
{
	struct list_head *pentry;
	struct part_info *part;
	int off, ndepth = 0;
	int part_num, ret;
	int sizecell;
	char buf[64];

	ret = fdt_del_partitions(blob, parent_offset);
	if (ret < 0)
		return ret;

	/*
	 * Check if size/address is 1 or 2 cells.
	 * We assume #address-cells and #size-cells have same value.
	 */
	sizecell = fdt_getprop_u32_default_node(blob, parent_offset,
						0, "#size-cells", 1);

	/*
	 * Check if it is nand {}; subnode, adjust
	 * the offset in this case
	 */
	off = fdt_next_node(blob, parent_offset, &ndepth);
	if (off > 0 && ndepth == 1)
		parent_offset = off;

	part_num = 0;
	list_for_each_prev(pentry, &dev->parts) {
		int newoff;

		part = list_entry(pentry, struct part_info, link);

		debug("%2d: %-20s0x%08llx\t0x%08llx\t%d\n",
			part_num, part->name, part->size,
			part->offset, part->mask_flags);

		sprintf(buf, "partition@%llx", part->offset);
add_sub:
		ret = fdt_add_subnode(blob, parent_offset, buf);
		if (ret == -FDT_ERR_NOSPACE) {
			ret = fdt_increase_size(blob, 512);
			if (!ret)
				goto add_sub;
			else
				goto err_size;
		} else if (ret < 0) {
			printf("Can't add partition node: %s\n",
				fdt_strerror(ret));
			return ret;
		}
		newoff = ret;

		/* Check MTD_WRITEABLE_CMD flag */
		if (part->mask_flags & 1) {
add_ro:
			ret = fdt_setprop(blob, newoff, "read_only", NULL, 0);
			if (ret == -FDT_ERR_NOSPACE) {
				ret = fdt_increase_size(blob, 512);
				if (!ret)
					goto add_ro;
				else
					goto err_size;
			} else if (ret < 0)
				goto err_prop;
		}

add_reg:
		if (sizecell == 2) {
			ret = fdt_setprop_u64(blob, newoff,
					      "reg", part->offset);
			if (!ret)
				ret = fdt_appendprop_u64(blob, newoff,
							 "reg", part->size);
		} else {
			ret = fdt_setprop_u32(blob, newoff,
					      "reg", part->offset);
			if (!ret)
				ret = fdt_appendprop_u32(blob, newoff,
							 "reg", part->size);
		}

		if (ret == -FDT_ERR_NOSPACE) {
			ret = fdt_increase_size(blob, 512);
			if (!ret)
				goto add_reg;
			else
				goto err_size;
		} else if (ret < 0)
			goto err_prop;

add_label:
		ret = fdt_setprop_string(blob, newoff, "label", part->name);
		if (ret == -FDT_ERR_NOSPACE) {
			ret = fdt_increase_size(blob, 512);
			if (!ret)
				goto add_label;
			else
				goto err_size;
		} else if (ret < 0)
			goto err_prop;

		part_num++;
	}
	return 0;
err_size:
	printf("Can't increase blob size: %s\n", fdt_strerror(ret));
	return ret;
err_prop:
	printf("Can't add property: %s\n", fdt_strerror(ret));
	return ret;
}

/*
 * Update partitions in nor/nand nodes using info from
 * mtdparts environment variable. The nodes to update are
 * specified by node_info structure which contains mtd device
 * type and compatible string: E. g. the board code in
 * ft_board_setup() could use:
 *
 *	struct node_info nodes[] = {
 *		{ "fsl,mpc5121-nfc",    MTD_DEV_TYPE_NAND, },
 *		{ "cfi-flash",          MTD_DEV_TYPE_NOR,  },
 *	};
 *
 *	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
 */
void fdt_fixup_mtdparts(void *blob, const struct node_info *node_info,
			int node_info_size)
{
	struct mtd_device *dev;
	int i, idx;
	int noff;

	if (mtdparts_init() != 0)
		return;

	for (i = 0; i < node_info_size; i++) {
		idx = 0;
		noff = fdt_node_offset_by_compatible(blob, -1,
						     node_info[i].compat);
		while (noff != -FDT_ERR_NOTFOUND) {
			debug("%s: %s, mtd dev type %d\n",
				fdt_get_name(blob, noff, 0),
				node_info[i].compat, node_info[i].type);
			dev = device_find(node_info[i].type, idx++);
			if (dev) {
				if (fdt_node_set_part_info(blob, noff, dev))
					return; /* return on error */
			}

			/* Jump to next flash node */
			noff = fdt_node_offset_by_compatible(blob, noff,
							     node_info[i].compat);
		}
	}
}
#endif

void fdt_del_node_and_alias(void *blob, const char *alias)
{
	int off = fdt_path_offset(blob, alias);

	if (off < 0)
		return;

	fdt_del_node(blob, off);

	off = fdt_path_offset(blob, "/aliases");
	fdt_delprop(blob, off, alias);
}

/* Max address size we deal with */
#define OF_MAX_ADDR_CELLS	4
#define OF_BAD_ADDR	FDT_ADDR_T_NONE
#define OF_CHECK_COUNTS(na, ns)	((na) > 0 && (na) <= OF_MAX_ADDR_CELLS && \
			(ns) > 0)

/* Debug utility */
#ifdef DEBUG
static void of_dump_addr(const char *s, const fdt32_t *addr, int na)
{
	printf("%s", s);
	while(na--)
		printf(" %08x", *(addr++));
	printf("\n");
}
#else
static void of_dump_addr(const char *s, const fdt32_t *addr, int na) { }
#endif

/**
 * struct of_bus - Callbacks for bus specific translators
 * @name:	A string used to identify this bus in debug output.
 * @addresses:	The name of the DT property from which addresses are
 *		to be read, typically "reg".
 * @match:	Return non-zero if the node whose parent is at
 *		parentoffset in the FDT blob corresponds to a bus
 *		of this type, otherwise return zero. If NULL a match
 *		is assumed.
 * @count_cells:Count how many cells (be32 values) a node whose parent
 *		is at parentoffset in the FDT blob will require to
 *		represent its address (written to *addrc) & size
 *		(written to *sizec).
 * @map:	Map the address addr from the address space of this
 *		bus to that of its parent, making use of the ranges
 *		read from DT to an array at range. na and ns are the
 *		number of cells (be32 values) used to hold and address
 *		or size, respectively, for this bus. pna is the number
 *		of cells used to hold an address for the parent bus.
 *		Returns the address in the address space of the parent
 *		bus.
 * @translate:	Update the value of the address cells at addr within an
 *		FDT by adding offset to it. na specifies the number of
 *		cells used to hold the address being translated. Returns
 *		zero on success, non-zero on error.
 *
 * Each bus type will include a struct of_bus in the of_busses array,
 * providing implementations of some or all of the functions used to
 * match the bus & handle address translation for its children.
 */
struct of_bus {
	const char	*name;
	const char	*addresses;
	int		(*match)(const void *blob, int parentoffset);
	void		(*count_cells)(const void *blob, int parentoffset,
				int *addrc, int *sizec);
	u64		(*map)(fdt32_t *addr, const fdt32_t *range,
				int na, int ns, int pna);
	int		(*translate)(fdt32_t *addr, u64 offset, int na);
};

/* Default translator (generic bus) */
void fdt_support_default_count_cells(const void *blob, int parentoffset,
					int *addrc, int *sizec)
{
	const fdt32_t *prop;

	if (addrc)
		*addrc = fdt_address_cells(blob, parentoffset);

	if (sizec) {
		prop = fdt_getprop(blob, parentoffset, "#size-cells", NULL);
		if (prop)
			*sizec = be32_to_cpup(prop);
		else
			*sizec = 1;
	}
}

static u64 of_bus_default_map(fdt32_t *addr, const fdt32_t *range,
		int na, int ns, int pna)
{
	u64 cp, s, da;

	cp = fdt_read_number(range, na);
	s  = fdt_read_number(range + na + pna, ns);
	da = fdt_read_number(addr, na);

	debug("OF: default map, cp=%llx, s=%llx, da=%llx\n", cp, s, da);

	if (da < cp || da >= (cp + s))
		return OF_BAD_ADDR;
	return da - cp;
}

static int of_bus_default_translate(fdt32_t *addr, u64 offset, int na)
{
	u64 a = fdt_read_number(addr, na);
	memset(addr, 0, na * 4);
	a += offset;
	if (na > 1)
		addr[na - 2] = cpu_to_fdt32(a >> 32);
	addr[na - 1] = cpu_to_fdt32(a & 0xffffffffu);

	return 0;
}

#ifdef CONFIG_OF_ISA_BUS

/* ISA bus translator */
static int of_bus_isa_match(const void *blob, int parentoffset)
{
	const char *name;

	name = fdt_get_name(blob, parentoffset, NULL);
	if (!name)
		return 0;

	return !strcmp(name, "isa");
}

static void of_bus_isa_count_cells(const void *blob, int parentoffset,
				   int *addrc, int *sizec)
{
	if (addrc)
		*addrc = 2;
	if (sizec)
		*sizec = 1;
}

static u64 of_bus_isa_map(fdt32_t *addr, const fdt32_t *range,
			  int na, int ns, int pna)
{
	u64 cp, s, da;

	/* Check address type match */
	if ((addr[0] ^ range[0]) & cpu_to_be32(1))
		return OF_BAD_ADDR;

	cp = fdt_read_number(range + 1, na - 1);
	s  = fdt_read_number(range + na + pna, ns);
	da = fdt_read_number(addr + 1, na - 1);

	debug("OF: ISA map, cp=%llx, s=%llx, da=%llx\n", cp, s, da);

	if (da < cp || da >= (cp + s))
		return OF_BAD_ADDR;
	return da - cp;
}

static int of_bus_isa_translate(fdt32_t *addr, u64 offset, int na)
{
	return of_bus_default_translate(addr + 1, offset, na - 1);
}

#endif /* CONFIG_OF_ISA_BUS */

/* Array of bus specific translators */
static struct of_bus of_busses[] = {
#ifdef CONFIG_OF_ISA_BUS
	/* ISA */
	{
		.name = "isa",
		.addresses = "reg",
		.match = of_bus_isa_match,
		.count_cells = of_bus_isa_count_cells,
		.map = of_bus_isa_map,
		.translate = of_bus_isa_translate,
	},
#endif /* CONFIG_OF_ISA_BUS */
	/* Default */
	{
		.name = "default",
		.addresses = "reg",
		.count_cells = fdt_support_default_count_cells,
		.map = of_bus_default_map,
		.translate = of_bus_default_translate,
	},
};

static struct of_bus *of_match_bus(const void *blob, int parentoffset)
{
	struct of_bus *bus;

	if (ARRAY_SIZE(of_busses) == 1)
		return of_busses;

	for (bus = of_busses; bus; bus++) {
		if (!bus->match || bus->match(blob, parentoffset))
			return bus;
	}

	/*
	 * We should always have matched the default bus at least, since
	 * it has a NULL match field. If we didn't then it somehow isn't
	 * in the of_busses array or something equally catastrophic has
	 * gone wrong.
	 */
	assert(0);
	return NULL;
}

static int of_translate_one(const void *blob, int parent, struct of_bus *bus,
			    struct of_bus *pbus, fdt32_t *addr,
			    int na, int ns, int pna, const char *rprop)
{
	const fdt32_t *ranges;
	int rlen;
	int rone;
	u64 offset = OF_BAD_ADDR;

	/* Normally, an absence of a "ranges" property means we are
	 * crossing a non-translatable boundary, and thus the addresses
	 * below the current not cannot be converted to CPU physical ones.
	 * Unfortunately, while this is very clear in the spec, it's not
	 * what Apple understood, and they do have things like /uni-n or
	 * /ht nodes with no "ranges" property and a lot of perfectly
	 * useable mapped devices below them. Thus we treat the absence of
	 * "ranges" as equivalent to an empty "ranges" property which means
	 * a 1:1 translation at that level. It's up to the caller not to try
	 * to translate addresses that aren't supposed to be translated in
	 * the first place. --BenH.
	 */
	ranges = fdt_getprop(blob, parent, rprop, &rlen);
	if (ranges == NULL || rlen == 0) {
		offset = fdt_read_number(addr, na);
		memset(addr, 0, pna * 4);
		debug("OF: no ranges, 1:1 translation\n");
		goto finish;
	}

	debug("OF: walking ranges...\n");

	/* Now walk through the ranges */
	rlen /= 4;
	rone = na + pna + ns;
	for (; rlen >= rone; rlen -= rone, ranges += rone) {
		offset = bus->map(addr, ranges, na, ns, pna);
		if (offset != OF_BAD_ADDR)
			break;
	}
	if (offset == OF_BAD_ADDR) {
		debug("OF: not found !\n");
		return 1;
	}
	memcpy(addr, ranges + na, 4 * pna);

 finish:
	of_dump_addr("OF: parent translation for:", addr, pna);
	debug("OF: with offset: %llu\n", offset);

	/* Translate it into parent bus space */
	return pbus->translate(addr, offset, pna);
}

/*
 * Translate an address from the device-tree into a CPU physical address,
 * this walks up the tree and applies the various bus mappings on the
 * way.
 *
 * Note: We consider that crossing any level with #size-cells == 0 to mean
 * that translation is impossible (that is we are not dealing with a value
 * that can be mapped to a cpu physical address). This is not really specified
 * that way, but this is traditionally the way IBM at least do things
 */
static u64 __of_translate_address(const void *blob, int node_offset,
				  const fdt32_t *in_addr, const char *rprop)
{
	int parent;
	struct of_bus *bus, *pbus;
	fdt32_t addr[OF_MAX_ADDR_CELLS];
	int na, ns, pna, pns;
	u64 result = OF_BAD_ADDR;

	debug("OF: ** translation for device %s **\n",
		fdt_get_name(blob, node_offset, NULL));

	/* Get parent & match bus type */
	parent = fdt_parent_offset(blob, node_offset);
	if (parent < 0)
		goto bail;
	bus = of_match_bus(blob, parent);

	/* Cound address cells & copy address locally */
	bus->count_cells(blob, parent, &na, &ns);
	if (!OF_CHECK_COUNTS(na, ns)) {
		printf("%s: Bad cell count for %s\n", __FUNCTION__,
		       fdt_get_name(blob, node_offset, NULL));
		goto bail;
	}
	memcpy(addr, in_addr, na * 4);

	debug("OF: bus is %s (na=%d, ns=%d) on %s\n",
	    bus->name, na, ns, fdt_get_name(blob, parent, NULL));
	of_dump_addr("OF: translating address:", addr, na);

	/* Translate */
	for (;;) {
		/* Switch to parent bus */
		node_offset = parent;
		parent = fdt_parent_offset(blob, node_offset);

		/* If root, we have finished */
		if (parent < 0) {
			debug("OF: reached root node\n");
			result = fdt_read_number(addr, na);
			break;
		}

		/* Get new parent bus and counts */
		pbus = of_match_bus(blob, parent);
		pbus->count_cells(blob, parent, &pna, &pns);
		if (!OF_CHECK_COUNTS(pna, pns)) {
			printf("%s: Bad cell count for %s\n", __FUNCTION__,
				fdt_get_name(blob, node_offset, NULL));
			break;
		}

		debug("OF: parent bus is %s (na=%d, ns=%d) on %s\n",
		    pbus->name, pna, pns, fdt_get_name(blob, parent, NULL));

		/* Apply bus translation */
		if (of_translate_one(blob, node_offset, bus, pbus,
					addr, na, ns, pna, rprop))
			break;

		/* Complete the move up one level */
		na = pna;
		ns = pns;
		bus = pbus;

		of_dump_addr("OF: one level translation:", addr, na);
	}
 bail:

	return result;
}

u64 fdt_translate_address(const void *blob, int node_offset,
			  const fdt32_t *in_addr)
{
	return __of_translate_address(blob, node_offset, in_addr, "ranges");
}

/**
 * fdt_node_offset_by_compat_reg: Find a node that matches compatiable and
 * who's reg property matches a physical cpu address
 *
 * @blob: ptr to device tree
 * @compat: compatiable string to match
 * @compat_off: property name
 *
 */
int fdt_node_offset_by_compat_reg(void *blob, const char *compat,
					phys_addr_t compat_off)
{
	int len, off = fdt_node_offset_by_compatible(blob, -1, compat);
	while (off != -FDT_ERR_NOTFOUND) {
		const fdt32_t *reg = fdt_getprop(blob, off, "reg", &len);
		if (reg) {
			if (compat_off == fdt_translate_address(blob, off, reg))
				return off;
		}
		off = fdt_node_offset_by_compatible(blob, off, compat);
	}

	return -FDT_ERR_NOTFOUND;
}

/**
 * fdt_alloc_phandle: Return next free phandle value
 *
 * @blob: ptr to device tree
 */
int fdt_alloc_phandle(void *blob)
{
	int offset;
	uint32_t phandle = 0;

	for (offset = fdt_next_node(blob, -1, NULL); offset >= 0;
	     offset = fdt_next_node(blob, offset, NULL)) {
		phandle = max(phandle, fdt_get_phandle(blob, offset));
	}

	return phandle + 1;
}

/*
 * fdt_set_phandle: Create a phandle property for the given node
 *
 * @fdt: ptr to device tree
 * @nodeoffset: node to update
 * @phandle: phandle value to set (must be unique)
 */
int fdt_set_phandle(void *fdt, int nodeoffset, uint32_t phandle)
{
	int ret;

#ifdef DEBUG
	int off = fdt_node_offset_by_phandle(fdt, phandle);

	if ((off >= 0) && (off != nodeoffset)) {
		char buf[64];

		fdt_get_path(fdt, nodeoffset, buf, sizeof(buf));
		printf("Trying to update node %s with phandle %u ",
		       buf, phandle);

		fdt_get_path(fdt, off, buf, sizeof(buf));
		printf("that already exists in node %s.\n", buf);
		return -FDT_ERR_BADPHANDLE;
	}
#endif

	ret = fdt_setprop_cell(fdt, nodeoffset, "phandle", phandle);
	if (ret < 0)
		return ret;

	/*
	 * For now, also set the deprecated "linux,phandle" property, so that we
	 * don't break older kernels.
	 */
	ret = fdt_setprop_cell(fdt, nodeoffset, "linux,phandle", phandle);

	return ret;
}

/*
 * fdt_create_phandle: Create a phandle property for the given node
 *
 * @fdt: ptr to device tree
 * @nodeoffset: node to update
 */
unsigned int fdt_create_phandle(void *fdt, int nodeoffset)
{
	/* see if there is a phandle already */
	int phandle = fdt_get_phandle(fdt, nodeoffset);

	/* if we got 0, means no phandle so create one */
	if (phandle == 0) {
		int ret;

		phandle = fdt_alloc_phandle(fdt);
		ret = fdt_set_phandle(fdt, nodeoffset, phandle);
		if (ret < 0) {
			printf("Can't set phandle %u: %s\n", phandle,
			       fdt_strerror(ret));
			return 0;
		}
	}

	return phandle;
}

/*
 * fdt_set_node_status: Set status for the given node
 *
 * @fdt: ptr to device tree
 * @nodeoffset: node to update
 * @status: FDT_STATUS_OKAY, FDT_STATUS_DISABLED,
 *	    FDT_STATUS_FAIL, FDT_STATUS_FAIL_ERROR_CODE
 * @error_code: optional, only used if status is FDT_STATUS_FAIL_ERROR_CODE
 */
int fdt_set_node_status(void *fdt, int nodeoffset,
			enum fdt_status status, unsigned int error_code)
{
	char buf[16];
	int ret = 0;

	if (nodeoffset < 0)
		return nodeoffset;

	switch (status) {
	case FDT_STATUS_OKAY:
		ret = fdt_setprop_string(fdt, nodeoffset, "status", "okay");
		break;
	case FDT_STATUS_DISABLED:
		ret = fdt_setprop_string(fdt, nodeoffset, "status", "disabled");
		break;
	case FDT_STATUS_FAIL:
		ret = fdt_setprop_string(fdt, nodeoffset, "status", "fail");
		break;
	case FDT_STATUS_FAIL_ERROR_CODE:
		sprintf(buf, "fail-%d", error_code);
		ret = fdt_setprop_string(fdt, nodeoffset, "status", buf);
		break;
	default:
		printf("Invalid fdt status: %x\n", status);
		ret = -1;
		break;
	}

	return ret;
}

/*
 * fdt_set_status_by_alias: Set status for the given node given an alias
 *
 * @fdt: ptr to device tree
 * @alias: alias of node to update
 * @status: FDT_STATUS_OKAY, FDT_STATUS_DISABLED,
 *	    FDT_STATUS_FAIL, FDT_STATUS_FAIL_ERROR_CODE
 * @error_code: optional, only used if status is FDT_STATUS_FAIL_ERROR_CODE
 */
int fdt_set_status_by_alias(void *fdt, const char* alias,
			    enum fdt_status status, unsigned int error_code)
{
	int offset = fdt_path_offset(fdt, alias);

	return fdt_set_node_status(fdt, offset, status, error_code);
}

#if defined(CONFIG_VIDEO) || defined(CONFIG_LCD)
int fdt_add_edid(void *blob, const char *compat, unsigned char *edid_buf)
{
	int noff;
	int ret;

	noff = fdt_node_offset_by_compatible(blob, -1, compat);
	if (noff != -FDT_ERR_NOTFOUND) {
		debug("%s: %s\n", fdt_get_name(blob, noff, 0), compat);
add_edid:
		ret = fdt_setprop(blob, noff, "edid", edid_buf, 128);
		if (ret == -FDT_ERR_NOSPACE) {
			ret = fdt_increase_size(blob, 512);
			if (!ret)
				goto add_edid;
			else
				goto err_size;
		} else if (ret < 0) {
			printf("Can't add property: %s\n", fdt_strerror(ret));
			return ret;
		}
	}
	return 0;
err_size:
	printf("Can't increase blob size: %s\n", fdt_strerror(ret));
	return ret;
}
#endif

/*
 * Verify the physical address of device tree node for a given alias
 *
 * This function locates the device tree node of a given alias, and then
 * verifies that the physical address of that device matches the given
 * parameter.  It displays a message if there is a mismatch.
 *
 * Returns 1 on success, 0 on failure
 */
int fdt_verify_alias_address(void *fdt, int anode, const char *alias, u64 addr)
{
	const char *path;
	const fdt32_t *reg;
	int node, len;
	u64 dt_addr;

	path = fdt_getprop(fdt, anode, alias, NULL);
	if (!path) {
		/* If there's no such alias, then it's not a failure */
		return 1;
	}

	node = fdt_path_offset(fdt, path);
	if (node < 0) {
		printf("Warning: device tree alias '%s' points to invalid "
		       "node %s.\n", alias, path);
		return 0;
	}

	reg = fdt_getprop(fdt, node, "reg", &len);
	if (!reg) {
		printf("Warning: device tree node '%s' has no address.\n",
		       path);
		return 0;
	}

	dt_addr = fdt_translate_address(fdt, node, reg);
	if (addr != dt_addr) {
		printf("Warning: U-Boot configured device %s at address %llu,\n"
		       "but the device tree has it address %llx.\n",
		       alias, addr, dt_addr);
		return 0;
	}

	return 1;
}

/*
 * Returns the base address of an SOC or PCI node
 */
u64 fdt_get_base_address(const void *fdt, int node)
{
	int size;
	const fdt32_t *prop;

	prop = fdt_getprop(fdt, node, "reg", &size);

	return prop ? fdt_translate_address(fdt, node, prop) : 0;
}

/*
 * Read a property of size <prop_len>. Currently only supports 1 or 2 cells.
 */
static int fdt_read_prop(const fdt32_t *prop, int prop_len, int cell_off,
			 uint64_t *val, int cells)
{
	const fdt32_t *prop32 = &prop[cell_off];
	const fdt64_t *prop64 = (const fdt64_t *)&prop[cell_off];

	if ((cell_off + cells) > prop_len)
		return -FDT_ERR_NOSPACE;

	switch (cells) {
	case 1:
		*val = fdt32_to_cpu(*prop32);
		break;
	case 2:
		*val = fdt64_to_cpu(*prop64);
		break;
	default:
		return -FDT_ERR_NOSPACE;
	}

	return 0;
}

/**
 * fdt_read_range - Read a node's n'th range property
 *
 * @fdt: ptr to device tree
 * @node: offset of node
 * @n: range index
 * @child_addr: pointer to storage for the "child address" field
 * @addr: pointer to storage for the CPU view translated physical start
 * @len: pointer to storage for the range length
 *
 * Convenience function that reads and interprets a specific range out of
 * a number of the "ranges" property array.
 */
int fdt_read_range(void *fdt, int node, int n, uint64_t *child_addr,
		   uint64_t *addr, uint64_t *len)
{
	int pnode = fdt_parent_offset(fdt, node);
	const fdt32_t *ranges;
	int pacells;
	int acells;
	int scells;
	int ranges_len;
	int cell = 0;
	int r = 0;

	/*
	 * The "ranges" property is an array of
	 * { <child address> <parent address> <size in child address space> }
	 *
	 * All 3 elements can span a diffent number of cells. Fetch their size.
	 */
	pacells = fdt_getprop_u32_default_node(fdt, pnode, 0, "#address-cells", 1);
	acells = fdt_getprop_u32_default_node(fdt, node, 0, "#address-cells", 1);
	scells = fdt_getprop_u32_default_node(fdt, node, 0, "#size-cells", 1);

	/* Now try to get the ranges property */
	ranges = fdt_getprop(fdt, node, "ranges", &ranges_len);
	if (!ranges)
		return -FDT_ERR_NOTFOUND;
	ranges_len /= sizeof(uint32_t);

	/* Jump to the n'th entry */
	cell = n * (pacells + acells + scells);

	/* Read <child address> */
	if (child_addr) {
		r = fdt_read_prop(ranges, ranges_len, cell, child_addr,
				  acells);
		if (r)
			return r;
	}
	cell += acells;

	/* Read <parent address> */
	if (addr)
		*addr = fdt_translate_address(fdt, node, ranges + cell);
	cell += pacells;

	/* Read <size in child address space> */
	if (len) {
		r = fdt_read_prop(ranges, ranges_len, cell, len, scells);
		if (r)
			return r;
	}

	return 0;
}

/**
 * fdt_setup_simplefb_node - Fill and enable a simplefb node
 *
 * @fdt: ptr to device tree
 * @node: offset of the simplefb node
 * @base_address: framebuffer base address
 * @width: width in pixels
 * @height: height in pixels
 * @stride: bytes per line
 * @format: pixel format string
 *
 * Convenience function to fill and enable a simplefb node.
 */
int fdt_setup_simplefb_node(void *fdt, int node, u64 base_address, u32 width,
			    u32 height, u32 stride, const char *format)
{
	char name[32];
	fdt32_t cells[4];
	int i, addrc, sizec, ret;

	fdt_support_default_count_cells(fdt, fdt_parent_offset(fdt, node),
					&addrc, &sizec);
	i = 0;
	if (addrc == 2)
		cells[i++] = cpu_to_fdt32(base_address >> 32);
	cells[i++] = cpu_to_fdt32(base_address);
	if (sizec == 2)
		cells[i++] = 0;
	cells[i++] = cpu_to_fdt32(height * stride);

	ret = fdt_setprop(fdt, node, "reg", cells, sizeof(cells[0]) * i);
	if (ret < 0)
		return ret;

	snprintf(name, sizeof(name), "framebuffer@%llx", base_address);
	ret = fdt_set_name(fdt, node, name);
	if (ret < 0)
		return ret;

	ret = fdt_setprop_u32(fdt, node, "width", width);
	if (ret < 0)
		return ret;

	ret = fdt_setprop_u32(fdt, node, "height", height);
	if (ret < 0)
		return ret;

	ret = fdt_setprop_u32(fdt, node, "stride", stride);
	if (ret < 0)
		return ret;

	ret = fdt_setprop_string(fdt, node, "format", format);
	if (ret < 0)
		return ret;

	ret = fdt_setprop_string(fdt, node, "status", "okay");
	if (ret < 0)
		return ret;

	return 0;
}

/*
 * Update native-mode in display-timings from display environment variable.
 * The node to update are specified by path.
 */
int fdt_fixup_display(void *blob, const char *path, const char *display)
{
	int off, toff;

	if (!display || !path)
		return -FDT_ERR_NOTFOUND;

	toff = fdt_path_offset(blob, path);
	if (toff >= 0)
		toff = fdt_subnode_offset(blob, toff, "display-timings");
	if (toff < 0)
		return toff;

	for (off = fdt_first_subnode(blob, toff);
	     off >= 0;
	     off = fdt_next_subnode(blob, off)) {
		uint32_t h = fdt_get_phandle(blob, off);
		debug("%s:0x%x\n", fdt_get_name(blob, off, NULL),
		      fdt32_to_cpu(h));
		if (strcasecmp(fdt_get_name(blob, off, NULL), display) == 0)
			return fdt_setprop_u32(blob, toff, "native-mode", h);
	}
	return toff;
}

#ifdef CONFIG_OF_LIBFDT_OVERLAY
/**
 * fdt_overlay_apply_verbose - Apply an overlay with verbose error reporting
 *
 * @fdt: ptr to device tree
 * @fdto: ptr to device tree overlay
 *
 * Convenience function to apply an overlay and display helpful messages
 * in the case of an error
 */
int fdt_overlay_apply_verbose(void *fdt, void *fdto)
{
	int err;
	bool has_symbols;

	err = fdt_path_offset(fdt, "/__symbols__");
	has_symbols = err >= 0;

	err = fdt_overlay_apply(fdt, fdto);
	if (err < 0) {
		printf("failed on fdt_overlay_apply(): %s\n",
				fdt_strerror(err));
		if (!has_symbols) {
			printf("base fdt does did not have a /__symbols__ node\n");
			printf("make sure you've compiled with -@\n");
		}
	}
	return err;
}
#endif
