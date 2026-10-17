/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __GZIP_H
#define __GZIP_H

struct blk_desc;

/**
 * gzip_parse_header() - Parse a header from a gzip file
 *
 * This returns the length of the header.
 *
 * @src: Pointer to gzip file
 * @len: Length of data
 * Return: length of header in bytes, or -1 if not enough data
 */
int gzip_parse_header(const unsigned char *src, unsigned long len);

/**
 * gunzip() - Decompress gzipped data
 *
 * @dst: Destination for uncompressed data
 * @dstlen: Size of destination buffer
 * @src: Source data to decompress
 * @lenp: Returns length of uncompressed data
 * Return: 0 if OK, -1 on error
 */
int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp);

/**
 * zunzip() - Uncompress blocks compressed with zlib without headers
 *
 * @dst: Destination for uncompressed data
 * @dstlen: Size of destination buffer
 * @src: Source data to decompress
 * @lenp: On entry, length data at @src. On exit, number of bytes used from @src
 * @stoponerr: 0 to continue when a decode error is found, 1 to stop
 * @offset: start offset within the src buffer
 * Return: 0 if OK, -1 on error
 */
int zunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp,
	   int stoponerr, int offset);

/**
 * gzwrite progress indicators: defined weak to allow board-specific
 * overrides:
 *
 *	gzwrite_progress_init called on startup
 *	gzwrite_progress called during decompress/write loop
 *	gzwrite_progress_finish called at end of loop to
 *		indicate success (retcode=0) or failure
 */
void gzwrite_progress_init(ulong expected_size);

void gzwrite_progress(int iteration, ulong bytes_written, ulong total_bytes);

void gzwrite_progress_finish(int retcode, ulong totalwritten, ulong totalsize,
			     u32 expected_crc, u32 calculated_crc);

/**
 * gzwrite() - decompress and write gzipped image from memory to block device
 *
 * @src:	compressed image address
 * @len:	compressed image length in bytes
 * @dev:	block device descriptor
 * @szwritebuf:	bytes per write (pad to erase size)
 * @startoffs:	offset in bytes of first write
 * @szexpected:	expected uncompressed length, may be zero to use gzip trailer
 *		for files under 4GiB
 * Return: 0 if OK, -1 on error
 */
int gzwrite(unsigned char *src, int len, struct blk_desc *dev, ulong szwritebuf,
	    ulong startoffs, ulong szexpected);

/**
 * gzip()- Compress data into a buffer using the gzip algorithm
 *
 * @dst: Destination buffer for compressed data
 * @lenp: On entry, space available in destination buffer (in bytes). On exit,
 *	number of bytes used in the buffer
 * @src: Source data to compress
 * @srclen: Size of source data
 * Return: 0 if OK, -1 on error
 */
int gzip(void *dst, unsigned long *lenp, unsigned char *src, ulong srclen);

/**
 * zzip() - Compress blocks with zlib
 *
 * @dst: Destination for compressed data
 * @lenp: On entry, length data at @dst. On exit, number of bytes written to
 *	@dst
 * @src: Source data to compress
 * @srclen: Size of source data
 * @stoponerr: 0 to continue when a decode error is found, 1 to stop
 * @func: Some sort of function that is called to do something. !ADD DOCS HERE!
 */
int zzip(void *dst, ulong *lenp, unsigned char *src, ulong srclen,
	 int stoponerr, int (*func)(ulong, ulong));

#endif
