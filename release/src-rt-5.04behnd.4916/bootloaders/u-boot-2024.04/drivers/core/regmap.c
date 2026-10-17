// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_DM

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <mapmem.h>
#include <regmap.h>
#include <asm/io.h>
#include <dm/of_addr.h>
#include <dm/devres.h>
#include <linux/ioport.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <linux/bitops.h>

/*
 * Internal representation of a regmap field. Instead of storing the MSB and
 * LSB, store the shift and mask. This makes the code a bit cleaner and faster
 * because the shift and mask don't have to be calculated every time.
 */
struct regmap_field {
	struct regmap *regmap;
	unsigned int mask;
	/* lsb */
	unsigned int shift;
	unsigned int reg;
};

DECLARE_GLOBAL_DATA_PTR;

/**
 * do_range_check() - Control whether range checks are done
 *
 * Returns: true to do range checks, false to skip
 *
 * This is used to reduce code size on SPL where range checks are known not to
 * be needed
 *
 * Add this to the top of the file to enable them: #define LOG_DEBUG
 */
static inline bool do_range_check(void)
{
	return _LOG_DEBUG || !IS_ENABLED(CONFIG_SPL);

}

/**
 * regmap_alloc() - Allocate a regmap with a given number of ranges.
 *
 * @count: Number of ranges to be allocated for the regmap.
 *
 * The default regmap width is set to REGMAP_SIZE_32. Callers can override it
 * if they need.
 *
 * Return: A pointer to the newly allocated regmap, or NULL on error.
 */
static struct regmap *regmap_alloc(int count)
{
	struct regmap *map;
	size_t size = sizeof(*map) + sizeof(map->ranges[0]) * count;

	map = calloc(1, size);
	if (!map)
		return NULL;
	map->range_count = count;
	map->width = REGMAP_SIZE_32;

	return map;
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
int regmap_init_mem_plat(struct udevice *dev, void *reg, int size, int count,
			 struct regmap **mapp)
{
	struct regmap_range *range;
	struct regmap *map;

	map = regmap_alloc(count);
	if (!map)
		return -ENOMEM;

	if (size == sizeof(fdt32_t)) {
		fdt32_t *ptr = (fdt32_t *)reg;

		for (range = map->ranges; count > 0;
		     ptr += 2, range++, count--) {
			range->start = *ptr;
			range->size = ptr[1];
		}
	} else if (size == sizeof(fdt64_t)) {
		fdt64_t *ptr = (fdt64_t *)reg;

		for (range = map->ranges; count > 0;
		     ptr += 2, range++, count--) {
			range->start = *ptr;
			range->size = ptr[1];
		}
	} else {
		return -EINVAL;
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

int regmap_init_mem_index(ofnode node, struct regmap **mapp, int index)
{
	struct regmap *map;
	int addr_len, size_len;
	int ret;

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

	map = regmap_alloc(1);
	if (!map)
		return -ENOMEM;

	ret = init_range(node, map->ranges, addr_len, size_len, index);
	if (ret)
		goto err;

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
err:
	regmap_uninit(map);

	return ret;
}

int regmap_init_mem_range(ofnode node, ulong r_start, ulong r_size,
			  struct regmap **mapp)
{
	struct regmap *map;
	struct regmap_range *range;

	map = regmap_alloc(1);
	if (!map)
		return -ENOMEM;

	range = &map->ranges[0];
	range->start = r_start;
	range->size = r_size;

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

int regmap_init_mem(ofnode node, struct regmap **mapp)
{
	struct regmap_range *range;
	struct regmap *map;
	int count;
	int addr_len, size_len, both_len;
	int len;
	int index;
	int ret;

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
		ret = init_range(node, range, addr_len, size_len, index);
		if (ret)
			goto err;
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
err:
	regmap_uninit(map);

	return ret;
}

static void devm_regmap_release(struct udevice *dev, void *res)
{
	regmap_uninit(*(struct regmap **)res);
}

struct regmap *devm_regmap_init(struct udevice *dev,
				const struct regmap_bus *bus,
				void *bus_context,
				const struct regmap_config *config)
{
	int rc;
	struct regmap **mapp, *map;

	/* this looks like a leak, but devres takes care of it */
	mapp = devres_alloc(devm_regmap_release, sizeof(struct regmap *),
			    __GFP_ZERO);
	if (unlikely(!mapp))
		return ERR_PTR(-ENOMEM);

	if (config && config->r_size != 0)
		rc = regmap_init_mem_range(dev_ofnode(dev), config->r_start,
					   config->r_size, mapp);
	else
		rc = regmap_init_mem(dev_ofnode(dev), mapp);
	if (rc)
		return ERR_PTR(rc);

	map = *mapp;
	if (config) {
		map->width = config->width;
		map->reg_offset_shift = config->reg_offset_shift;
	}

	devres_add(dev, mapp);
	return *mapp;
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

	if (do_range_check() && range_num >= map->range_count) {
		debug("%s: range index %d larger than range count\n",
		      __func__, range_num);
		return -ERANGE;
	}
	range = &map->ranges[range_num];

	offset <<= map->reg_offset_shift;
	if (do_range_check() &&
	    (offset + val_len > range->size || offset + val_len < offset)) {
		debug("%s: offset/size combination invalid\n", __func__);
		return -ERANGE;
	}

	ptr = map_physmem(range->start + offset, val_len, MAP_NOCACHE);

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
	union {
		u8 v8;
		u16 v16;
		u32 v32;
		u64 v64;
	} u;
	int res;

	res = regmap_raw_read(map, offset, &u, map->width);
	if (res)
		return res;

	switch (map->width) {
	case REGMAP_SIZE_8:
		*valp = u.v8;
		break;
	case REGMAP_SIZE_16:
		*valp = u.v16;
		break;
	case REGMAP_SIZE_32:
		*valp = u.v32;
		break;
	case REGMAP_SIZE_64:
		*valp = u.v64;
		break;
	default:
		unreachable();
	}

	return 0;
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

	offset <<= map->reg_offset_shift;
	if (offset + val_len > range->size || offset + val_len < offset) {
		debug("%s: offset/size combination invalid\n", __func__);
		return -ERANGE;
	}

	ptr = map_physmem(range->start + offset, val_len, MAP_NOCACHE);

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
	union {
		u8 v8;
		u16 v16;
		u32 v32;
		u64 v64;
	} u;

	switch (map->width) {
	case REGMAP_SIZE_8:
		u.v8 = val;
		break;
	case REGMAP_SIZE_16:
		u.v16 = val;
		break;
	case REGMAP_SIZE_32:
		u.v32 = val;
		break;
	case REGMAP_SIZE_64:
		u.v64 = val;
		break;
	default:
		debug("%s: regmap size %zu unknown\n", __func__,
		      (size_t)map->width);
		return -EINVAL;
	}

	return regmap_raw_write(map, offset, &u, map->width);
}

int regmap_update_bits(struct regmap *map, uint offset, uint mask, uint val)
{
	uint reg;
	int ret;

	ret = regmap_read(map, offset, &reg);
	if (ret)
		return ret;

	reg &= ~mask;

	return regmap_write(map, offset, reg | (val & mask));
}

int regmap_field_read(struct regmap_field *field, unsigned int *val)
{
	int ret;
	unsigned int reg_val;

	ret = regmap_read(field->regmap, field->reg, &reg_val);
	if (ret != 0)
		return ret;

	reg_val &= field->mask;
	reg_val >>= field->shift;
	*val = reg_val;

	return ret;
}

int regmap_field_write(struct regmap_field *field, unsigned int val)
{
	return regmap_update_bits(field->regmap, field->reg, field->mask,
				  val << field->shift);
}

static void regmap_field_init(struct regmap_field *rm_field,
			      struct regmap *regmap,
			      struct reg_field reg_field)
{
	rm_field->regmap = regmap;
	rm_field->reg = reg_field.reg;
	rm_field->shift = reg_field.lsb;
	rm_field->mask = GENMASK(reg_field.msb, reg_field.lsb);
}

struct regmap_field *devm_regmap_field_alloc(struct udevice *dev,
					     struct regmap *regmap,
					     struct reg_field reg_field)
{
	struct regmap_field *rm_field = devm_kzalloc(dev, sizeof(*rm_field),
						     GFP_KERNEL);
	if (!rm_field)
		return ERR_PTR(-ENOMEM);

	regmap_field_init(rm_field, regmap, reg_field);

	return rm_field;
}

void devm_regmap_field_free(struct udevice *dev, struct regmap_field *field)
{
	devm_kfree(dev, field);
}

struct regmap_field *regmap_field_alloc(struct regmap *regmap,
					struct reg_field reg_field)
{
	struct regmap_field *rm_field = kzalloc(sizeof(*rm_field), GFP_KERNEL);

	if (!rm_field)
		return ERR_PTR(-ENOMEM);

	regmap_field_init(rm_field, regmap, reg_field);

	return rm_field;
}

void regmap_field_free(struct regmap_field *field)
{
	kfree(field);
}
