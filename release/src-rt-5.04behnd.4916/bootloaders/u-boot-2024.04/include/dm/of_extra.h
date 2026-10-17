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
	FMAP_COMPRESS_LZMA,
	FMAP_COMPRESS_LZ4,

	FMAP_COMPRESS_COUNT,
	FMAP_COMPRESS_UNKNOWN,
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
	/* Node pointer if CBFS, else NULL */
	const struct cbfs_cachenode *cbfs_node;
	/* Hash node pointer if CBFS, else NULL */
	const struct cbfs_cachenode *cbfs_hash_node;
};

/**
 * ofnode_read_fmap_entry() - Read a flash entry from the fdt
 *
 * @node:	Reference to node to read
 * @entry:	Place to put offset and size of this node
 * Return: 0 if ok, -ve on error
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
 * @node:	ofnode to examine
 * @prop_name:	name of property to find
 * @basep:	Returns base address of region
 * @sizep:	Returns size of region
 * Return: 0 if ok, -1 on error (property not found)
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
 * @config_node:	ofnode containing the properties (invalid for "/config")
 * @mem_type:	Type of memory to use, which is a name, such as
 *		"u-boot" or "kernel".
 * @suffix:	String to append to the memory/offset
 *		property names
 * @basep:	Returns base of region
 * @sizep:	Returns size of region
 * Return: 0 if OK, -ive on error
 */
int ofnode_decode_memory_region(ofnode config_node, const char *mem_type,
				const char *suffix, fdt_addr_t *basep,
				fdt_size_t *sizep);

/**
 * ofnode_phy_is_fixed_link() - Detect fixed-link pseudo-PHY device
 *
 * This function detects whether the ethernet controller connects to a
 * fixed-link pseudo-PHY device.
 *
 * This function supports the following two DT bindings:
 * - the new DT binding, where 'fixed-link' is a sub-node of the
 * Ethernet device
 * - the old DT binding, where 'fixed-link' is a property with 5
 * cells encoding various information about the fixed PHY
 *
 * If both new and old bindings exist, the new one is preferred.
 *
 * @eth_node:	ofnode containing the fixed-link subnode/property
 * @phy_node:	if fixed-link PHY detected, containing the PHY ofnode
 * Return: true if a fixed-link pseudo-PHY device exists, false otherwise
 */
bool ofnode_phy_is_fixed_link(ofnode eth_node, ofnode *phy_node);

/**
 * ofnode_eth_uses_inband_aneg() - Detect whether MAC should use in-band autoneg
 *
 * This function detects whether the Ethernet controller should use IEEE 802.3
 * clause 37 in-band autonegotiation for serial protocols such as 1000base-x,
 * SGMII, USXGMII, etc. The property is relevant when the Ethernet controller
 * is connected to an on-board PHY or an SFP cage, and is not relevant when it
 * has a fixed link (in that case, in-band autoneg should not be used).
 *
 * @eth_node:	ofnode belonging to the Ethernet controller
 * Return: true if in-band autoneg should be used, false otherwise
 */
bool ofnode_eth_uses_inband_aneg(ofnode eth_node);

#endif
