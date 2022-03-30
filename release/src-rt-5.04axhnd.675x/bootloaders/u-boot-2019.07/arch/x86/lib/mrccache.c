// SPDX-License-Identifier: GPL-2.0
/*
 * From coreboot src/southbridge/intel/bd82x6x/mrccache.c
 *
 * Copyright (C) 2014 Google Inc.
 * Copyright (C) 2015 Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <net.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/mrccache.h>

DECLARE_GLOBAL_DATA_PTR;

static struct mrc_data_container *next_mrc_block(
	struct mrc_data_container *cache)
{
	/* MRC data blocks are aligned within the region */
	u32 mrc_size = sizeof(*cache) + cache->data_size;
	u8 *region_ptr = (u8 *)cache;

	if (mrc_size & (MRC_DATA_ALIGN - 1UL)) {
		mrc_size &= ~(MRC_DATA_ALIGN - 1UL);
		mrc_size += MRC_DATA_ALIGN;
	}

	region_ptr += mrc_size;

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
 * @entry:	MRC cache flash area
 * @cache:	Entry to start from
 *
 * @return next cache entry if found, NULL if we got to the end
 */
static struct mrc_data_container *find_next_mrc_cache(struct mrc_region *entry,
		struct mrc_data_container *cache)
{
	ulong base_addr, end_addr;

	base_addr = entry->base + entry->offset;
	end_addr = base_addr + entry->length;

	cache = next_mrc_block(cache);
	if ((ulong)cache >= end_addr) {
		/* Crossed the boundary */
		cache = NULL;
		debug("%s: no available entries found\n", __func__);
	} else {
		debug("%s: picked next entry from cache block at %p\n",
		      __func__, cache);
	}

	return cache;
}

int mrccache_update(struct udevice *sf, struct mrc_region *entry,
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
		cache = find_next_mrc_cache(entry, cache);

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
		return ret;
	}

	return 0;
}

static void mrccache_setup(void *data)
{
	struct mrc_data_container *cache = data;
	u16 checksum;

	cache->signature = MRC_DATA_SIGNATURE;
	cache->data_size = gd->arch.mrc_output_len;
	checksum = compute_ip_checksum(gd->arch.mrc_output, cache->data_size);
	debug("Saving %d bytes for MRC output data, checksum %04x\n",
	      cache->data_size, checksum);
	cache->checksum = checksum;
	cache->reserved = 0;
	memcpy(cache->data, gd->arch.mrc_output, cache->data_size);

	/* gd->arch.mrc_output now points to the container */
	gd->arch.mrc_output = (char *)cache;
}

int mrccache_reserve(void)
{
	if (!gd->arch.mrc_output_len)
		return 0;

	/* adjust stack pointer to store pure cache data plus the header */
	gd->start_addr_sp -= (gd->arch.mrc_output_len + MRC_DATA_HEADER_SIZE);
	mrccache_setup((void *)gd->start_addr_sp);

	gd->start_addr_sp &= ~0xf;

	return 0;
}

int mrccache_get_region(struct udevice **devp, struct mrc_region *entry)
{
	const void *blob = gd->fdt_blob;
	int node, mrc_node;
	u32 reg[2];
	int ret;

	/* Find the flash chip within the SPI controller node */
	node = fdtdec_next_compatible(blob, 0, COMPAT_GENERIC_SPI_FLASH);
	if (node < 0) {
		debug("%s: Cannot find SPI flash\n", __func__);
		return -ENOENT;
	}

	if (fdtdec_get_int_array(blob, node, "memory-map", reg, 2)) {
		debug("%s: Cannot find memory map\n", __func__);
		return -EINVAL;
	}
	entry->base = reg[0];

	/* Find the place where we put the MRC cache */
	mrc_node = fdt_subnode_offset(blob, node, "rw-mrc-cache");
	if (mrc_node < 0) {
		debug("%s: Cannot find node\n", __func__);
		return -EPERM;
	}

	if (fdtdec_get_int_array(blob, mrc_node, "reg", reg, 2)) {
		debug("%s: Cannot find address\n", __func__);
		return -EINVAL;
	}
	entry->offset = reg[0];
	entry->length = reg[1];

	if (devp) {
		ret = uclass_get_device_by_of_offset(UCLASS_SPI_FLASH, node,
						     devp);
		debug("ret = %d\n", ret);
		if (ret)
			return ret;
	}

	return 0;
}

int mrccache_save(void)
{
	struct mrc_data_container *data;
	struct mrc_region entry;
	struct udevice *sf;
	int ret;

	if (!gd->arch.mrc_output_len)
		return 0;
	debug("Saving %d bytes of MRC output data to SPI flash\n",
	      gd->arch.mrc_output_len);

	ret = mrccache_get_region(&sf, &entry);
	if (ret)
		goto err_entry;
	data  = (struct mrc_data_container *)gd->arch.mrc_output;
	ret = mrccache_update(sf, &entry, data);
	if (!ret) {
		debug("Saved MRC data with checksum %04x\n", data->checksum);
	} else if (ret == -EEXIST) {
		debug("MRC data is the same as last time, skipping save\n");
		ret = 0;
	}

err_entry:
	if (ret)
		debug("%s: Failed: %d\n", __func__, ret);
	return ret;
}

int mrccache_spl_save(void)
{
	void *data;
	int size;

	size = gd->arch.mrc_output_len + MRC_DATA_HEADER_SIZE;
	data = malloc(size);
	if (!data)
		return log_msg_ret("Allocate MRC cache block", -ENOMEM);
	mrccache_setup(data);
	gd->arch.mrc_output = data;

	return mrccache_save();
}
