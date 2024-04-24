/*
 * Copyright (C) 2019 Sean Parkinson, wolfSSL Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup wolfssl_util wolfssl_util
 * @{ @ingroup wolfssl_p
 */

#ifndef WOLFSSL_PLUGIN_UTIL_H_
#define WOLFSSL_PLUGIN_UTIL_H_

#include <wolfssl/wolfcrypt/integer.h>
#include <wolfssl/wolfcrypt/hash.h>

/**
 * Creates a hash of a given type of a chunk of data.
 *
 * Note: this function allocates memory for the hash
 *
 * @param hash_type Hash enumeration
 * @param data		the chunk of data to hash
 * @param hash		chunk that contains the hash
 * @return			TRUE on success, FALSE otherwise
 */
bool wolfssl_hash_chunk(int hash_type, chunk_t data, chunk_t *hash);

/**
 * Exports the given integer (assumed to be a positive number) to a chunk in
 * two's complement format (i.e. a zero byte is added if the MSB is set).
 *
 * @param mp		the integer to export
 * @param chunk		the chunk (data gets allocated)
 * @return			TRUE on success, FALSE otherwise
 */
bool wolfssl_mp2chunk(mp_int *mp, chunk_t *chunk);

/**
 * Splits a chunk into two mp_ints of equal binary length.
 *
 * @param chunk		a chunk that contains the two integers
 * @param a			first mp_int
 * @param b			second mp_int
 * @return			TRUE on success, FALSE otherwise
 */
bool wolfssl_mp_split(chunk_t chunk, mp_int *a, mp_int *b);

/**
 * Concatenates two integers into a chunk, thereby enforcing the length of
 * a single integer, if necessary, by prepending it with zeros.
 *
 * Note: this function allocates memory for the chunk
 *
 * @param len		the length of a single integer
 * @param a			first integer
 * @param b			second integer
 * @param chunk		resulting chunk
 * @return			TRUE on success, FALSE otherwise
 */
bool wolfssl_mp_cat(int len, mp_int *a, mp_int *b, chunk_t *chunk);

/**
 * Convert the hash algorithm to a wolfSSL hash type.
 *
 * @param hash		hash algorithm
 * @param type		Hash enumeration from wolfSSL
 * @return			TRUE on success, FALSE otherwise
 */
bool wolfssl_hash2type(hash_algorithm_t hash, enum wc_HashType *type);

/**
 * Convert the mgf1 hash algorithm to a wolfSSL mgf1 type.
 *
 * @param hash		hash algorithm
 * @param mgf1		MGF1 algorithm from wolfSSL
 * @return			TRUE on success, FALSE otherwise
 */
bool wolfssl_hash2mgf1(hash_algorithm_t hash, int *mgf1);

#endif /** WOLFSSL_PLUGIN_UTIL_H_ @}*/
