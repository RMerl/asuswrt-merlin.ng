/*
 * Copyright (C) 2008-2013 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup chunk chunk
 * @{ @ingroup utils
 */

#ifndef CHUNK_H_
#define CHUNK_H_

#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <utils/utils.h>

typedef struct chunk_t chunk_t;

/**
 * General purpose pointer/length abstraction.
 */
struct chunk_t {
	/** Pointer to start of data */
	u_char *ptr;
	/** Length of data in bytes */
	size_t len;
};

#include "utils.h"

/**
 * A { NULL, 0 }-chunk handy for initialization.
 */
extern chunk_t chunk_empty;

/**
 * Create a new chunk pointing to "ptr" with length "len"
 */
static inline chunk_t chunk_create(u_char *ptr, size_t len)
{
	chunk_t chunk = {ptr, len};
	return chunk;
}

/**
 * Create a clone of a chunk pointing to "ptr"
 */
chunk_t chunk_create_clone(u_char *ptr, chunk_t chunk);

/**
 * Calculate length of multiple chunks
 */
size_t chunk_length(const char *mode, ...);

/**
 * Concatenate chunks into a chunk pointing to "ptr".
 *
 * The mode string specifies the number of chunks, and how to handle each of
 * them with a single character: 'c' for copy (allocate new chunk), 'm' for move
 * (free given chunk) or 's' for sensitive-move (clear given chunk, then free).
 */
chunk_t chunk_create_cat(u_char *ptr, const char* mode, ...);

/**
 * Split up a chunk into parts, "mode" is a string of "a" (alloc),
 * "c" (copy) and "m" (move). Each letter say for the corresponding chunk if
 * it should get allocated on heap, copied into existing chunk, or the chunk
 * should point into "chunk". The length of each part is an argument before
 * each target chunk. E.g.:
 * chunk_split(chunk, "mcac", 3, &a, 7, &b, 5, &c, d.len, &d);
 */
void chunk_split(chunk_t chunk, const char *mode, ...);

/**
 * Write the binary contents of a chunk_t to a file
 *
 * If the write fails, errno is set appropriately.
 *
 * @param chunk			contents to write to file
 * @param path			path where file is written to
 * @param mask			file mode creation mask
 * @param force			overwrite existing file by force
 * @return				TRUE if write operation was successful
 */
bool chunk_write(chunk_t chunk, char *path, mode_t mask, bool force);

/**
 * Store data read from FD into a chunk
 *
 * On error, errno is set appropriately.
 *
 * @param fd			file descriptor to read from
 * @param chunk			chunk receiving allocated buffer
 * @return				TRUE if successful, FALSE on failure
 */
bool chunk_from_fd(int fd, chunk_t *chunk);

/**
 * mmap() a file to a chunk
 *
 * The returned chunk structure is allocated from heap, but it must be freed
 * through chunk_unmap(). A user may alter the chunk ptr or len, but must pass
 * the chunk pointer returned from chunk_map() to chunk_unmap() after use.
 *
 * On error, errno is set appropriately.
 *
 * @param path			path of file to map
 * @param wr			TRUE to sync writes to disk
 * @return				mapped chunk, NULL on error
 */
chunk_t *chunk_map(char *path, bool wr);

/**
 * munmap() a chunk previously mapped with chunk_map()
 *
 * When unmapping a writeable map, the return value should be checked to
 * ensure changes landed on disk.
 *
 * @param chunk			pointer returned from chunk_map()
 * @return				TRUE of changes written back to file
 */
bool chunk_unmap(chunk_t *chunk);

/**
 * Convert a chunk of data to hex encoding.
 *
 * The resulting string is '\\0' terminated, but the chunk does not include
 * the '\\0'. If buf is supplied, it must hold at least (chunk.len * 2 + 1).
 *
 * @param chunk			data to convert to hex encoding
 * @param buf			buffer to write to, NULL to malloc
 * @param uppercase		TRUE to use uppercase letters
 * @return				chunk of encoded data
 */
chunk_t chunk_to_hex(chunk_t chunk, char *buf, bool uppercase);

/**
 * Convert a hex encoded in a binary chunk.
 *
 * If buf is supplied, it must hold at least (hex.len / 2) + (hex.len % 2)
 * bytes. It is filled by the right to give correct values for short inputs.
 *
 * @param hex			hex encoded input data
 * @param buf			buffer to write decoded data, NULL to malloc
 * @return				converted data
 */
chunk_t chunk_from_hex(chunk_t hex, char *buf);

/**
 * Convert a chunk of data to its base64 encoding.
 *
 * The resulting string is '\\0' terminated, but the chunk does not include
 * the '\\0'. If buf is supplied, it must hold at least (chunk.len * 4 / 3 + 1).
 *
 * @param chunk			data to convert
 * @param buf			buffer to write to, NULL to malloc
 * @return				chunk of encoded data
 */
chunk_t chunk_to_base64(chunk_t chunk, char *buf);

/**
 * Convert a base64 in a binary chunk.
 *
 * If buf is supplied, it must hold at least (base64.len / 4 * 3).
 *
 * @param base64		base64 encoded input data
 * @param buf			buffer to write decoded data, NULL to malloc
 * @return				converted data
 */
chunk_t chunk_from_base64(chunk_t base64, char *buf);

/**
 * Convert a chunk of data to its base32 encoding.
 *
 * The resulting string is '\\0' terminated, but the chunk does not include
 * the '\\0'. If buf is supplied, it must hold (chunk.len * 8 / 5 + 1) bytes.
 *
 * @param chunk			data to convert
 * @param buf			buffer to write to, NULL to malloc
 * @return				chunk of encoded data
 */
chunk_t chunk_to_base32(chunk_t chunk, char *buf);

/**
 * Free contents of a chunk
 */
static inline void chunk_free(chunk_t *chunk)
{
	free(chunk->ptr);
	*chunk = chunk_empty;
}

/**
 * Overwrite the contents of a chunk and free it
 */
static inline void chunk_clear(chunk_t *chunk)
{
	if (chunk->ptr)
	{
		memwipe(chunk->ptr, chunk->len);
		chunk_free(chunk);
	}
}

/**
 * Initialize a chunk using a char array
 */
#define chunk_from_chars(...) ((chunk_t){(u_char[]){__VA_ARGS__}, sizeof((u_char[]){__VA_ARGS__})})

/**
 * Initialize a chunk to point to a thing
 */
#define chunk_from_thing(thing) chunk_create((u_char*)&(thing), sizeof(thing))

/**
 * Initialize a chunk from a string, not containing 0-terminator
 */
#define chunk_from_str(str) ({char *x = (str); chunk_create((u_char*)x, strlen(x));})

/**
 * Allocate a chunk on the heap
 */
#define chunk_alloc(bytes) ({size_t x = (bytes); chunk_create(x ? malloc(x) : NULL, x);})

/**
 * Allocate a chunk on the stack
 */
#define chunk_alloca(bytes) ({size_t x = (bytes); chunk_create(x ? alloca(x) : NULL, x);})

/**
 * Clone a chunk on heap
 */
#define chunk_clone(chunk) ({chunk_t x = (chunk); chunk_create_clone(x.len ? malloc(x.len) : NULL, x);})

/**
 * Clone a chunk on stack
 */
#define chunk_clonea(chunk) ({chunk_t x = (chunk); chunk_create_clone(x.len ? alloca(x.len) : NULL, x);})

/**
 * Concatenate chunks into a chunk on heap
 */
#define chunk_cat(mode, ...) chunk_create_cat(malloc(chunk_length(mode, __VA_ARGS__)), mode, __VA_ARGS__)

/**
 * Concatenate chunks into a chunk on stack
 */
#define chunk_cata(mode, ...) chunk_create_cat(alloca(chunk_length(mode, __VA_ARGS__)), mode, __VA_ARGS__)

/**
 * Skip n bytes in chunk (forward pointer, shorten length)
 */
static inline chunk_t chunk_skip(chunk_t chunk, size_t bytes)
{
	if (chunk.len > bytes)
	{
		chunk.ptr += bytes;
		chunk.len -= bytes;
		return chunk;
	}
	return chunk_empty;
}

/**
 * Skip a leading zero-valued byte
 */
static inline chunk_t chunk_skip_zero(chunk_t chunk)
{
	if (chunk.len > 1 && *chunk.ptr == 0x00)
	{
		chunk.ptr++;
		chunk.len--;
	}
	return chunk;
}


/**
 *  Compare two chunks, returns zero if a equals b
 *  or negative/positive if a is small/greater than b
 */
int chunk_compare(chunk_t a, chunk_t b);

/**
 * Compare two chunks for equality,
 * NULL chunks are never equal.
 */
static inline bool chunk_equals(chunk_t a, chunk_t b)
{
	return a.ptr != NULL  && b.ptr != NULL &&
			a.len == b.len && memeq(a.ptr, b.ptr, a.len);
}

/**
 * Compare two chunks (given as pointers) for equality (useful as callback),
 * NULL chunks are never equal.
 */
static inline bool chunk_equals_ptr(chunk_t *a, chunk_t *b)
{
	return a != NULL && b != NULL && chunk_equals(*a, *b);
}

/**
 * Increment a chunk, as it would reprensent a network order integer.
 *
 * @param chunk			chunk to increment
 * @return				TRUE if an overflow occurred
 */
bool chunk_increment(chunk_t chunk);

/**
 * Check if a chunk has printable characters only.
 *
 * If sane is given, chunk is cloned into sane and all non printable characters
 * get replaced by "replace".
 *
 * @param chunk			chunk to check for printability
 * @param sane			pointer where sane version is allocated, or NULL
 * @param replace		character to use for replaceing unprintable characters
 * @return				TRUE if all characters in chunk are printable
 */
bool chunk_printable(chunk_t chunk, chunk_t *sane, char replace);

/**
 * Seed initial key for chunk_hash().
 *
 * This call should get invoked once during startup. This is usually done
 * by calling library_init(). Calling it multiple times is safe, it gets
 * executed just once.
 */
void chunk_hash_seed();

/**
 * Computes a 32 bit hash of the given chunk.
 *
 * @note The output of this function is randomized, that is, it will only
 * produce the same output for the same input when calling it from the same
 * process.  For a more predictable hash function use chunk_hash_static()
 * instead.
 *
 * @note This hash is only intended for hash tables not for cryptographic
 * purposes.
 *
 * @param chunk			data to hash
 * @return				hash value
 */
u_int32_t chunk_hash(chunk_t chunk);

/**
 * Incremental version of chunk_hash. Use this to hash two or more chunks.
 *
 * @param chunk			data to hash
 * @param hash			previous hash value
 * @return				hash value
 */
u_int32_t chunk_hash_inc(chunk_t chunk, u_int32_t hash);

/**
 * Computes a 32 bit hash of the given chunk.
 *
 * Compared to chunk_hash() this will always calculate the same output for the
 * same input.  Therefore, it should not be used for hash tables (to prevent
 * hash flooding).
 *
 * @note This hash is not intended for cryptographic purposes.
 *
 * @param chunk			data to hash
 * @return				hash value
 */
u_int32_t chunk_hash_static(chunk_t chunk);

/**
 * Incremental version of chunk_hash_static(). Use this to hash two or more
 * chunks in a predictable way.
 *
 * @param chunk			data to hash
 * @param hash			previous hash value
 * @return				hash value
 */
u_int32_t chunk_hash_static_inc(chunk_t chunk, u_int32_t hash);

/**
 * Computes a quick MAC from the given chunk and key using SipHash.
 *
 * The key must have a length of 128-bit (16 bytes).
 *
 * @note While SipHash has strong features using it for cryptographic purposes
 * is not recommended (in particular because of the rather short output size).
 *
 * @param chunk			data to process
 * @param key			key to use
 * @return				MAC for given input and key
 */
u_int64_t chunk_mac(chunk_t chunk, u_char *key);

/**
 * Calculate the Internet Checksum according to RFC 1071 for the given chunk.
 *
 * If the result is used with chunk_internet_checksum_inc() and the data length
 * is not a multiple of 16 bit the checksum bytes have to be swapped to
 * compensate the even/odd alignment.
 *
 * @param data			data to process
 * @return				checksum (one's complement, network order)
 */
u_int16_t chunk_internet_checksum(chunk_t data);

/**
 * Extend the given Internet Checksum (one's complement, in network byte order)
 * with the given data.
 *
 * If data is not a multiple of 16 bits the checksum may have to be swapped to
 * compensate even/odd alignment (see chunk_internet_checksum()).
 *
 * @param data			data to process
 * @param checksum		previous checksum (one's complement, network order)
 * @return				checksum (one's complement, network order)
 */
u_int16_t chunk_internet_checksum_inc(chunk_t data, u_int16_t checksum);

/**
 * printf hook function for chunk_t.
 *
 * Arguments are:
 *	chunk_t *chunk
 * Use #-modifier to print a compact version
 * Use +-modifier to print a compact version without separator
 */
int chunk_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					  const void *const *args);

#endif /** CHUNK_H_ @}*/
