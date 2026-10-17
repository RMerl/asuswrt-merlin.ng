/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Header file for SHA hardware acceleration
 *
 * Copyright (c) 2012  Samsung Electronics
 */
#ifndef __HW_SHA_H
#define __HW_SHA_H
#include <hash.h>

/**
 * Computes hash value of input pbuf using h/w acceleration
 *
 * @param in_addr	A pointer to the input buffer
 * @param bufleni	Byte length of input buffer
 * @param out_addr	A pointer to the output buffer. When complete
 *			64 bytes are copied to pout[0]...pout[63]. Thus, a user
 *			should allocate at least 64 bytes at pOut in advance.
 * @param chunk_size	chunk size for sha512
 */
void hw_sha512(const uchar *in_addr, uint buflen, uchar *out_addr,
	       uint chunk_size);

/**
 * Computes hash value of input pbuf using h/w acceleration
 *
 * @param in_addr	A pointer to the input buffer
 * @param bufleni	Byte length of input buffer
 * @param out_addr	A pointer to the output buffer. When complete
 *			48 bytes are copied to pout[0]...pout[47]. Thus, a user
 *			should allocate at least 48 bytes at pOut in advance.
 * @param chunk_size	chunk size for sha384
 */
void hw_sha384(const uchar *in_addr, uint buflen, uchar *out_addr,
	       uint chunk_size);

/**
 * Computes hash value of input pbuf using h/w acceleration
 *
 * @param in_addr	A pointer to the input buffer
 * @param bufleni	Byte length of input buffer
 * @param out_addr	A pointer to the output buffer. When complete
 *			32 bytes are copied to pout[0]...pout[31]. Thus, a user
 *			should allocate at least 32 bytes at pOut in advance.
 * @param chunk_size	chunk size for sha256
 */
void hw_sha256(const uchar *in_addr, uint buflen, uchar *out_addr,
	       uint chunk_size);

/**
 * Computes hash value of input pbuf using h/w acceleration
 *
 * @param in_addr	A pointer to the input buffer
 * @param bufleni	Byte length of input buffer
 * @param out_addr	A pointer to the output buffer. When complete
 *			32 bytes are copied to pout[0]...pout[31]. Thus, a user
 *			should allocate at least 32 bytes at pOut in advance.
 * @param chunk_size	chunk_size for sha1
 */
void hw_sha1(const uchar *in_addr, uint buflen, uchar *out_addr,
	     uint chunk_size);

/*
 * Create the context for sha progressive hashing using h/w acceleration
 *
 * @algo: Pointer to the hash_algo struct
 * @ctxp: Pointer to the pointer of the context for hashing
 * Return: 0 if ok, -ve on error
 */
int hw_sha_init(struct hash_algo *algo, void **ctxp);

/*
 * Update buffer for sha progressive hashing using h/w acceleration
 *
 * The context is freed by this function if an error occurs.
 *
 * @algo: Pointer to the hash_algo struct
 * @ctx: Pointer to the context for hashing
 * @buf: Pointer to the buffer being hashed
 * @size: Size of the buffer being hashed
 * @is_last: 1 if this is the last update; 0 otherwise
 * Return: 0 if ok, -ve on error
 */
int hw_sha_update(struct hash_algo *algo, void *ctx, const void *buf,
		  unsigned int size, int is_last);

/*
 * Copy sha hash result at destination location
 *
 * The context is freed after completion of hash operation or after an error.
 *
 * @algo: Pointer to the hash_algo struct
 * @ctx: Pointer to the context for hashing
 * @dest_buf: Pointer to the destination buffer where hash is to be copied
 * @size: Size of the buffer being hashed
 * Return: 0 if ok, -ve on error
 */
int hw_sha_finish(struct hash_algo *algo, void *ctx, void *dest_buf,
		  int size);

#endif
