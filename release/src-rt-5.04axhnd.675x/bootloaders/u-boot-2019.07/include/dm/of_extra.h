/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef _DM_OF_EXTRA_H
#define _DM_OF_EXTRA_H

#include <dm/ofnode.h>

enum fmap_compress_t {
	FMAP_COMPRESS_NONE,
	FMAP_COMPRESS_LZ4,
};

enum fmap_hash_t {
	FMAP_HASH_NONE,
	FMAP_HASH_SHA1,
	FMAP_HASH_SHA256,
};

/* A flash map entry, containing an offset and length */
struct fmap_entry {
	uint32_t offset;
	uint32_t length;
	uint32_t used;			/* Number of bytes used in region */
	enum fmap_compress_t compress_algo;	/* Compression type */
	uint32_t unc_length;			/* Uncompressed length */
	enum fmap_hash_t hash_algo;		/* Hash algorithm */
	const uint8_t *hash;			/* Hash value */
	int hash_size;				/* Hash size */
};

/**
 * Read a flash entry from the fdt
 *
 * @param node		Reference to node to read
 * @param entry		Place to put offset and size of this node
 * @return 0 if ok, -ve on error
 */
int ofnode_read_fmap_entry(ofnode node, struct fmap_entry *entry);

/**
 * ofnode_decode_region() - Decode a memory region from a node
 *
 * Look up a property in a node which contains a memory region address and
 * size. Then return a pointer to this address.
 *
 * The property must hold one address with a length. This is only tested on
 * 32-bit machines.
 *
 * @param node		ofnode to examine
 * @param prop_name	name of property to find
 * @param basep		Returns base address of region
 * @param size		Returns size of region
 * @return 0 if ok, -1 on error (property not found)
 */
int ofnode_decode_region(ofnode node, const char *prop_name, fdt_addr_t *basep,
			 fdt_size_t *sizep);

/**
 * ofnode_decode_memory_region()- Decode a named region within a memory bank
 *
 * This function handles selection of a memory region. The region is
 * specified as an offset/size within a particular type of memory.
 *
 * The properties used are:
 *
 *	<mem_type>-memory<suffix> for the name of the memory bank
 *	<mem_type>-offset<suffix> for the offset in that bank
 *
 * The property value must have an offset and a size. The function checks
 * that the region is entirely within the memory bank.5
 *
 * @param node		ofnode containing the properties (-1 for /config)
 * @param mem_type	Type of memory to use, which is a name, such as
 *			"u-boot" or "kernel".
 * @param suffix	String to append to the memory/offset
 *			property names
 * @param basep		Returns base of region
 * @param sizep		Returns size of region
 * @return 0 if OK, -ive on error
 */
int ofnode_decode_memory_region(ofnode config_node, const char *mem_type,
				const char *suffix, fdt_addr_t *basep,
				fdt_size_t *sizep);

#endif
