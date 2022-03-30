// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <mapmem.h>
#include <regmap.h>
#include <asm/io.h>
#include <dm/of_addr.h>
#include <linux/ioport.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * regmap_alloc() - Allocate a regmap with a given number of ranges.
 *
 * @count: Number of ranges to be allocated for the regmap.
 * Return: A pointer to the newly allocated regmap, or NULL on error.
 */
static struct regmap *regmap_alloc(int count)
{
	struct regmap *map;

	map = malloc(sizeof(*map) + sizeof(map->ranges[0]) * count);
	if (!map)
		return NULL;
	map->range_count = count;

	return map;
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
int regmap_init_mem_platdata(struct udevice *dev, fdt_val_t *reg, int count,
			     struct regmap **mapp)
{
	struct regmap_range *range;
	struct regmap *map;

	map = regmap_alloc(count);
	if (!map)
		return -ENOMEM;

	for (range = map->ranges; count > 0; reg += 2, range++, count--) {
		range->start = *reg;
		range->size = reg[1];
	}

	*mapp = map;

	return 0;
}
#else
/**
 * init_range() - Initialize a single range of a regmap
 * @node:     Device node that will use the map in question
 * @range:    Pointer to a regmap_range structure that will be initialized
 * @addr_len: The length of the addr parts of the reg property
 * @size_len: The length of the size parts of the reg property
 * @index:    The index of the range to initialize
 *
 * This function will read the necessary 'reg' information from the device tree
 * (the 'addr' part, and the 'length' part), and initialize the range in
 * quesion.
 *
 * Return: 0 if OK, -ve on error
 */
static int init_range(ofnode node, struct regmap_range *range, int addr_len,
		      int size_len, int index)
{
	fdt_size_t sz;
	struct resource r;

	if (of_live_active()) {
		int ret;

		ret = of_address_to_resource(ofnode_to_np(node),
					     index, &r);
		if (ret) {
			debug("%s: Could not read resource of range %d (ret = %d)\n",
			      ofnode_get_name(node), index, ret);
			return ret;
		}

		range->start = r.start;
		range->size = r.end - r.start + 1;
	} else {
		int offset = ofnode_to_offset(node);

		range->start = fdtdec_get_addr_size_fixed(gd->fdt_blob, offset,
							  "reg", index,
							  addr_len, size_len,
							  &sz, true);
		if (range->start == FDT_ADDR_T_NONE) {
			debug("%s: Could not read start of range %d\n",
			      ofnode_get_name(node), index);
			return -EINVAL;
		}

		range->size = sz;
	}

	return 0;
}

int regmap_init_mem(ofnode node, struct regmap **mapp)
{
	struct regmap_range *range;
	struct regmap *map;
	int count;
	int addr_len, size_len, both_len;
	int len;
	int index;

	addr_len = ofnode_read_simple_addr_cells(ofnode_get_parent(node));
	if (addr_len < 0) {
		debug("%s: Error while reading the addr length (ret = %d)\n",
		      ofnode_get_name(node), addr_len);
		return addr_len;
	}

	size_len = ofnode_read_simple_size_cells(ofnode_get_parent(node));
	if (size_len < 0) {
		debug("%s: Error while reading the size length: (ret = %d)\n",
		      ofnode_get_name(node), size_len);
		return size_len;
	}

	both_len = addr_len + size_len;
	if (!both_len) {
		debug("%s: Both addr and size length are zero\n",
		      ofnode_get_name(node));
		return -EINVAL;
	}

	len = ofnode_read_size(node, "reg");
	if (len < 0) {
		debug("%s: Error while reading reg size (ret = %d)\n",
		      ofnode_get_name(node), len);
		return len;
	}
	len /= sizeof(fdt32_t);
	count = len / both_len;
	if (!count) {
		debug("%s: Not enough data in reg property\n",
		      ofnode_get_name(node));
		return -EINVAL;
	}

	map = regmap_alloc(count);
	if (!map)
		return -ENOMEM;

	for (range = map->ranges, index = 0; count > 0;
	     count--, range++, index++) {
		int ret = init_range(node, range, addr_len, size_len, index);

		if (ret)
			return ret;
	}

	if (ofnode_read_bool(node, "little-endian"))
		map->endianness = REGMAP_LITTLE_ENDIAN;
	else if (ofnode_read_bool(node, "big-endian"))
		map->endianness = REGMAP_BIG_ENDIAN;
	else if (ofnode_read_bool(node, "native-endian"))
		map->endianness = REGMAP_NATIVE_ENDIAN;
	else /* Default: native endianness */
		map->endianness = REGMAP_NATIVE_ENDIAN;

	*mapp = map;

	return 0;
}
#endif

void *regmap_get_range(struct regmap *map, unsigned int range_num)
{
	struct regmap_range *range;

	if (range_num >= map->range_count)
		return NULL;
	range = &map->ranges[range_num];

	return map_sysmem(range->start, range->size);
}

int regmap_uninit(struct regmap *map)
{
	free(map);

	return 0;
}

static inline u8 __read_8(u8 *addr, enum regmap_endianness_t endianness)
{
	return readb(addr);
}

static inline u16 __read_16(u16 *addr, enum regmap_endianness_t endianness)
{
	switch (endianness) {
	case REGMAP_LITTLE_ENDIAN:
		return in_le16(addr);
	case REGMAP_BIG_ENDIAN:
		return in_be16(addr);
	case REGMAP_NATIVE_ENDIAN:
		return readw(addr);
	}

	return readw(addr);
}

static inline u32 __read_32(u32 *addr, enum regmap_endianness_t endianness)
{
	switch (endianness) {
	case REGMAP_LITTLE_ENDIAN:
		return in_le32(addr);
	case REGMAP_BIG_ENDIAN:
		return in_be32(addr);
	case REGMAP_NATIVE_ENDIAN:
		return readl(addr);
	}

	return readl(addr);
}

#if defined(in_le64) && defined(in_be64) && defined(readq)
static inline u64 __read_64(u64 *addr, enum regmap_endianness_t endianness)
{
	switch (endianness) {
	case REGMAP_LITTLE_ENDIAN:
		return in_le64(addr);
	case REGMAP_BIG_ENDIAN:
		return in_be64(addr);
	case REGMAP_NATIVE_ENDIAN:
		return readq(addr);
	}

	return readq(addr);
}
#endif

int regmap_raw_read_range(struct regmap *map, uint range_num, uint offset,
			  void *valp, size_t val_len)
{
	struct regmap_range *range;
	void *ptr;

	if (range_num >= map->range_count) {
		debug("%s: range index %d larger than range count\n",
		      __func__, range_num);
		return -ERANGE;
	}
	range = &map->ranges[range_num];

	ptr = map_physmem(range->start + offset, val_len, MAP_NOCACHE);

	if (offset + val_len > range->size) {
		debug("%s: offset/size combination invalid\n", __func__);
		return -ERANGE;
	}

	switch (val_len) {
	case REGMAP_SIZE_8:
		*((u8 *)valp) = __read_8(ptr, map->endianness);
		break;
	case REGMAP_SIZE_16:
		*((u16 *)valp) = __read_16(ptr, map->endianness);
		break;
	case REGMAP_SIZE_32:
		*((u32 *)valp) = __read_32(ptr, map->endianness);
		break;
#if defined(in_le64) && defined(in_be64) && defined(readq)
	case REGMAP_SIZE_64:
		*((u64 *)valp) = __read_64(ptr, map->endianness);
		break;
#endif
	default:
		debug("%s: regmap size %zu unknown\n", __func__, val_len);
		return -EINVAL;
	}

	return 0;
}

int regmap_raw_read(struct regmap *map, uint offset, void *valp, size_t val_len)
{
	return regmap_raw_read_range(map, 0, offset, valp, val_len);
}

int regmap_read(struct regmap *map, uint offset, uint *valp)
{
	return regmap_raw_read(map, offset, valp, REGMAP_SIZE_32);
}

static inline void __write_8(u8 *addr, const u8 *val,
			     enum regmap_endianness_t endianness)
{
	writeb(*val, addr);
}

static inline void __write_16(u16 *addr, const u16 *val,
			      enum regmap_endianness_t endianness)
{
	switch (endianness) {
	case REGMAP_NATIVE_ENDIAN:
		writew(*val, addr);
		break;
	case REGMAP_LITTLE_ENDIAN:
		out_le16(addr, *val);
		break;
	case REGMAP_BIG_ENDIAN:
		out_be16(addr, *val);
		break;
	}
}

static inline void __write_32(u32 *addr, const u32 *val,
			      enum regmap_endianness_t endianness)
{
	switch (endianness) {
	case REGMAP_NATIVE_ENDIAN:
		writel(*val, addr);
		break;
	case REGMAP_LITTLE_ENDIAN:
		out_le32(addr, *val);
		break;
	case REGMAP_BIG_ENDIAN:
		out_be32(addr, *val);
		break;
	}
}

#if defined(out_le64) && defined(out_be64) && defined(writeq)
static inline void __write_64(u64 *addr, const u64 *val,
			      enum regmap_endianness_t endianness)
{
	switch (endianness) {
	case REGMAP_NATIVE_ENDIAN:
		writeq(*val, addr);
		break;
	case REGMAP_LITTLE_ENDIAN:
		out_le64(addr, *val);
		break;
	case REGMAP_BIG_ENDIAN:
		out_be64(addr, *val);
		break;
	}
}
#endif

int regmap_raw_write_range(struct regmap *map, uint range_num, uint offset,
			   const void *val, size_t val_len)
{
	struct regmap_range *range;
	void *ptr;

	if (range_num >= map->range_count) {
		debug("%s: range index %d larger than range count\n",
		      __func__, range_num);
		return -ERANGE;
	}
	range = &map->ranges[range_num];

	ptr = map_physmem(range->start + offset, val_len, MAP_NOCACHE);

	if (offset + val_len > range->size) {
		debug("%s: offset/size combination invalid\n", __func__);
		return -ERANGE;
	}

	switch (val_len) {
	case REGMAP_SIZE_8:
		__write_8(ptr, val, map->endianness);
		break;
	case REGMAP_SIZE_16:
		__write_16(ptr, val, map->endianness);
		break;
	case REGMAP_SIZE_32:
		__write_32(ptr, val, map->endianness);
		break;
#if defined(out_le64) && defined(out_be64) && defined(writeq)
	case REGMAP_SIZE_64:
		__write_64(ptr, val, map->endianness);
		break;
#endif
	default:
		debug("%s: regmap size %zu unknown\n", __func__, val_len);
		return -EINVAL;
	}

	return 0;
}

int regmap_raw_write(struct regmap *map, uint offset, const void *val,
		     size_t val_len)
{
	return regmap_raw_write_range(map, 0, offset, val, val_len);
}

int regmap_write(struct regmap *map, uint offset, uint val)
{
	return regmap_raw_write(map, offset, &val, REGMAP_SIZE_32);
}

int regmap_update_bits(struct regmap *map, uint offset, uint mask, uint val)
{
	uint reg;
	int ret;

	ret = regmap_read(map, offset, &reg);
	if (ret)
		return ret;

	reg &= ~mask;

	return regmap_write(map, offset, reg | val);
}
