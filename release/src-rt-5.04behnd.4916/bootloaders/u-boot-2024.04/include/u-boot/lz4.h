/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Google LLC
 */

#ifndef __LZ4_H
#define __LZ4_H

/**
 * ulz4fn() - Decompress LZ4 data
 *
 * @src: Source data to decompress
 * @srcn: Length of source data
 * @dst: Destination for uncompressed data
 * @dstn: Returns length of uncompressed data
 * Return: 0 if OK, -EPROTONOSUPPORT if the magic number or version number are
 *	not recognised or independent blocks are used, -EINVAL if the reserved
 *	fields are non-zero, or input is overrun, -EENOBUFS if the destination
 *	buffer is overrun, -EEPROTO if the compressed data causes an error in
 *	the decompression algorithm
 */
int ulz4fn(const void *src, size_t srcn, void *dst, size_t *dstn);

/**
 * LZ4_decompress_safe() - Decompression protected against buffer overflow
 * @source: source address of the compressed data
 * @dest: output buffer address of the uncompressed data
 *	which must be already allocated
 * @compressedSize: is the precise full size of the compressed block
 * @maxDecompressedSize: is the size of 'dest' buffer
 *
 * Decompresses data from 'source' into 'dest'.
 * If the source stream is detected malformed, the function will
 * stop decoding and return a negative result.
 * This function is protected against buffer overflow exploits,
 * including malicious data packets. It never writes outside output buffer,
 * nor reads outside input buffer.
 *
 * Return: number of bytes decompressed into destination buffer
 *	(necessarily <= maxDecompressedSize)
 *	or a negative result in case of error
 */
int LZ4_decompress_safe(const char *source, char *dest,
	int compressedSize, int maxDecompressedSize);

/**
 * LZ4_decompress_safe_partial() - Decompress a block of size 'compressedSize'
 *	at position 'source' into buffer 'dest'
 * @source: source address of the compressed data
 * @dest: output buffer address of the decompressed data which must be
 *	already allocated
 * @compressedSize: is the precise full size of the compressed block.
 * @targetOutputSize: the decompression operation will try
 *	to stop as soon as 'targetOutputSize' has been reached
 * @maxDecompressedSize: is the size of destination buffer
 *
 * This function decompresses a compressed block of size 'compressedSize'
 * at position 'source' into destination buffer 'dest'
 * of size 'maxDecompressedSize'.
 * The function tries to stop decompressing operation as soon as
 * 'targetOutputSize' has been reached, reducing decompression time.
 * This function never writes outside of output buffer,
 * and never reads outside of input buffer.
 * It is therefore protected against malicious data packets.
 *
 * Return: the number of bytes decoded in the destination buffer
 *	(necessarily <= maxDecompressedSize)
 *	or a negative result in case of error
 *
 */
int LZ4_decompress_safe_partial(const char *src, char *dst,
	int compressedSize, int targetOutputSize, int dstCapacity);
#endif
