/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __CVMX_FUSE_H__
#define __CVMX_FUSE_H__

/**
 * Read a byte of fuse data
 * @param node		node to read from
 * @param byte_addr	address to read
 *
 * Return: fuse value: 0 or 1
 */
static inline u8 cvmx_fuse_read_byte_node(u8 node, int byte_addr)
{
	u64 val;

	val = FIELD_PREP(MIO_FUS_RCMD_ADDR, byte_addr) | MIO_FUS_RCMD_PEND;
	csr_wr_node(node, CVMX_MIO_FUS_RCMD, val);

	do {
		val = csr_rd_node(node, CVMX_MIO_FUS_RCMD);
	} while (val & MIO_FUS_RCMD_PEND);

	return FIELD_GET(MIO_FUS_RCMD_DAT, val);
}

/**
 * Read a byte of fuse data
 * @param byte_addr   address to read
 *
 * Return: fuse value: 0 or 1
 */
static inline u8 cvmx_fuse_read_byte(int byte_addr)
{
	return cvmx_fuse_read_byte_node(0, byte_addr);
}

/**
 * Read a single fuse bit
 *
 * @param node   Node number
 * @param fuse   Fuse number (0-1024)
 *
 * Return: fuse value: 0 or 1
 */
static inline int cvmx_fuse_read_node(u8 node, int fuse)
{
	return (cvmx_fuse_read_byte_node(node, fuse >> 3) >> (fuse & 0x7)) & 1;
}

/**
 * Read a single fuse bit
 *
 * @param fuse   Fuse number (0-1024)
 *
 * Return: fuse value: 0 or 1
 */
static inline int cvmx_fuse_read(int fuse)
{
	return cvmx_fuse_read_node(0, fuse);
}

static inline int cvmx_octeon_fuse_locked(void)
{
	return cvmx_fuse_read(123);
}

#endif /* __CVMX_FUSE_H__ */
