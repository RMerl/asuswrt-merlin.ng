// SPDX-License-Identifier: GPL-2.0
/*
 * From coreboot src/southbridge/intel/bd82x6x/mrccache.c
 *
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2015 Bin Meng <bmeng.cn@gmail.com>
 */

#define LOG_CATEGORY	UCLASS_RAM

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <net.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/global_data.h>
#include <asm/mrccache.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

static uint mrc_block_size(uint data_size)
{
	uint mrc_size = sizeof(struct mrc_data_container) + data_size;

	return ALIGN(mrc_size, MRC_DATA_ALIGN);
}

static struct mrc_data_container *next_mrc_block(
	struct mrc_data_container *cache)
{
	/* MRC data blocks are aligned within the region */
	u8 *region_ptr = (u8 *)cache;

	region_ptr += mrc_block_size(cache->data_size);

	return (struct mrc_data_container *)region_ptr;
}

static int is_mrc_cache(struct mrc_data_container *cache)
{
	return cache && (cache->signature == MRC_DATA_SIGNATURE);
}

struct mrc_data_container *mrccache_find_current(struct mrc_region *entry)
{
	struct mrc_data_container *cache, *next;
	ulong base_addr, end_addr;
	uint id;

	base_addr = entry->base + entry->offset;
	end_addr = base_addr + entry->length;
	cache = NULL;

	/* Search for the last filled entry in the region */
	for (id = 0, next = (struct mrc_data_container *)base_addr;
	     is_mrc_cache(next);
	     id++) {
		cache = next;
		next = next_mrc_block(next);
		if ((ulong)next >= end_addr)
			break;
	}

	if (id-- == 0) {
		debug("%s: No valid MRC cache found.\n", __func__);
		return NULL;
	}

	/* Verify checksum */
	if (cache->checksum != compute_ip_checksum(cache->data,
						   cache->data_size)) {
		printf("%s: MRC cache checksum mismatch\n", __func__);
		return NULL;
	}

	debug("%s: picked entry %u from cache block\n", __func__, id);

	return cache;
}

/**
 * find_next_mrc_cache() - get next cache entry
 *
 * This moves to the next cache entry in the region, making sure it has enough
 * space to hold data of size @data_size.
 *
 * @entry:	MRC cache flash area
 * @cache:	Entry to start from
 * @data_size:	Required data size of the new entry. Note that we assume that
 *	all cache entries are the same size
 *
 * Return: next cache entry if found, NULL if we got to the end
 */
static struct mrc_data_container *find_next_mrc_cache(struct mrc_region *entry,
		struct mrc_data_container *prev, int data_size)
{
	struct mrc_data_container *cache;
	ulong base_addr, end_addr;

	base_addr = entry->base + entry->offset;
	end_addr = base_addr + entry->length;

	/*
	 * We assume that all cache entries are the same size, but let's use
	 * data_size here for clarity.
	 */
	cache = next_mrc_block(prev);
	if ((ulong)cache + mrc_block_size(data_size) > end_addr) {
		/* Crossed the boundary */
		cache = NULL;
		debug("%s: no available entries found\n", __func__);
	} else {
		debug("%s: picked next entry from cache block at %p\n",
		      __func__, cache);
	}

	return cache;
}

/**
 * mrccache_update() - update the MRC cache with a new record
 *
 * This writes a new record to the end of the MRC cache region. If the new
 * record is the same as the latest record then the write is skipped
 *
 * @sf:		SPI flash to write to
 * @entry:	Position and size of MRC cache in SPI flash
 * @cur:	Record to write
 * Return: 0 if updated, -EEXIST if the record is the same as the latest
 * record, -EINVAL if the record is not valid, other error if SPI write failed
 */
static int mrccache_update(struct udevice *sf, struct mrc_region *entry,
			   struct mrc_data_container *cur)
{
	struct mrc_data_container *cache;
	ulong offset;
	ulong base_addr;
	int ret;

	if (!is_mrc_cache(cur)) {
		debug("%s: Cache data not valid\n", __func__);
		return -EINVAL;
	}

	/* Find the last used block */
	base_addr = entry->base + entry->offset;
	debug("Updating MRC cache data\n");
	cache = mrccache_find_current(entry);
	if (cache && (cache->data_size == cur->data_size) &&
	    (!memcmp(cache, cur, cache->data_size + sizeof(*cur)))) {
		debug("MRC data in flash is up to date. No update\n");
		return -EEXIST;
	}

	/* Move to the next block, which will be the first unused block */
	if (cache)
		cache = find_next_mrc_cache(entry, cache, cur->data_size);

	/*
	 * If we have got to the end, erase the entire mrc-cache area and start
	 * again at block 0.
	 */
	if (!cache) {
		debug("Erasing the MRC cache region of %x bytes at %x\n",
		      entry->length, entry->offset);

		ret = spi_flash_erase_dm(sf, entry->offset, entry->length);
		if (ret) {
			debug("Failed to erase flash region\n");
			return ret;
		}
		cache = (struct mrc_data_container *)base_addr;
	}

	/* Write the data out */
	offset = (ulong)cache - base_addr + entry->offset;
	debug("Write MRC cache update to flash at %lx\n", offset);
	ret = spi_flash_write_dm(sf, offset, cur->data_size + sizeof(*cur),
				 cur);
	if (ret) {
		debug("Failed to write to SPI flash\n");
		return log_msg_ret("Cannot update mrccache", ret);
	}

	return 0;
}

static void mrccache_setup(struct mrc_output *mrc, void *data)
{
	struct mrc_data_container *cache = data;
	u16 checksum;

	cache->signature = MRC_DATA_SIGNATURE;
	cache->data_size = mrc->len;
	checksum = compute_ip_checksum(mrc->buf, cache->data_size);
	log_debug("Saving %d bytes for MRC output data, checksum %04x\n",
		  cache->data_size, checksum);
	cache->checksum = checksum;
	cache->reserved = 0;
	memcpy(cache->data, mrc->buf, cache->data_size);

	mrc->cache = cache;
}

int mrccache_reserve(void)
{
	int i;

	for (i = 0; i < MRC_TYPE_COUNT; i++) {
		struct mrc_output *mrc = &gd->arch.mrc[i];

		if (!mrc->len)
			continue;

		/* adjust stack pointer to store pure cache data plus header */
		gd->start_addr_sp -= (mrc->len + MRC_DATA_HEADER_SIZE);
		mrccache_setup(mrc, (void *)gd->start_addr_sp);

		gd->start_addr_sp &= ~0xf;
	}

	return 0;
}

int mrccache_get_region(enum mrc_type_t type, struct udevice **devp,
			struct mrc_region *entry)
{
	struct udevice *dev;
	ofnode mrc_node;
	ulong map_base;
	uint map_size;
	uint offset;
	ofnode node;
	u32 reg[2];
	int ret;

	/*
	 * Find the flash chip within the SPI controller node. Avoid probing
	 * the device here since it may put it into a strange state where the
	 * memory map cannot be read.
	 */
	ret = uclass_find_first_device(UCLASS_SPI_FLASH, &dev);
	if (ret || !dev) {
		/*
		 * Fall back to searching the device tree since driver model
		 * may not be ready yet (e.g. with FSPv1)
		 */
		node = ofnode_by_compatible(ofnode_null(), "jedec,spi-nor");
		if (!ofnode_valid(node))
			return log_msg_ret("Cannot find SPI flash\n", -ENOENT);
		ret = -ENODEV;
	} else {
		ret = dm_spi_get_mmap(dev, &map_base, &map_size, &offset);
		if (!ret)
			entry->base = map_base;
		node = dev_ofnode(dev);
	}

	/*
	 * At this point we have entry->base if ret == 0. If not, then we have
	 * the node and can look for memory-map
	 */
	if (ret) {
		ret = ofnode_read_u32_array(node, "memory-map", reg, 2);
		if (ret)
			return log_msg_ret("Cannot find memory map\n", ret);
		entry->base = reg[0];
	}

	/* Find the place where we put the MRC cache */
	mrc_node = ofnode_find_subnode(node, type == MRC_TYPE_NORMAL ?
				       "rw-mrc-cache" : "rw-var-mrc-cache");
	if (!ofnode_valid(mrc_node))
		return log_msg_ret("Cannot find node", -EPERM);

	ret = ofnode_read_u32_array(mrc_node, "reg", reg, 2);
	if (ret)
		return log_msg_ret("Cannot find address", ret);
	entry->offset = reg[0];
	entry->length = reg[1];

	if (devp)
		*devp = dev;
	debug("MRC cache type %d in '%s', offset %x, len %x, base %x\n",
	      type, dev ? dev->name : ofnode_get_name(node), entry->offset,
	      entry->length, entry->base);

	return 0;
}

static int mrccache_save_type(enum mrc_type_t type)
{
	struct mrc_data_container *cache;
	struct mrc_output *mrc;
	struct mrc_region entry;
	struct udevice *sf;
	int ret;

	mrc = &gd->arch.mrc[type];
	if (!mrc->len)
		return 0;
	log_debug("Saving %x bytes of MRC output data type %d to SPI flash\n",
		  mrc->len, type);
	ret = mrccache_get_region(type, &sf, &entry);
	if (ret)
		return log_msg_ret("Cannot get region", ret);
	ret = device_probe(sf);
	if (ret)
		return log_msg_ret("Cannot probe device", ret);
	cache = mrc->cache;

	ret = mrccache_update(sf, &entry, cache);
	if (!ret)
		debug("Saved MRC data with checksum %04x\n", cache->checksum);
	else if (ret == -EEXIST)
		debug("MRC data is the same as last time, skipping save\n");

	return 0;
}

int mrccache_save(void)
{
	int i;

	for (i = 0; i < MRC_TYPE_COUNT; i++) {
		int ret;

		ret = mrccache_save_type(i);
		if (ret)
			return ret;
	}

	return 0;
}

int mrccache_spl_save(void)
{
	int i;

	for (i = 0; i < MRC_TYPE_COUNT; i++) {
		struct mrc_output *mrc = &gd->arch.mrc[i];
		void *data;
		int size;

		size = mrc->len + MRC_DATA_HEADER_SIZE;
		data = malloc(size);
		if (!data)
			return log_msg_ret("Allocate MRC cache block", -ENOMEM);
		mrccache_setup(mrc, data);
	}

	return mrccache_save();
}
