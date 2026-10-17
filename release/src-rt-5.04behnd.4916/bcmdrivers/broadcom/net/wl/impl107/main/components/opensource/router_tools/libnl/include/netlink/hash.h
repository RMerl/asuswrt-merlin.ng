/*
 * This file was taken from http://ccodearchive.net/info/hash.html
 * Changes to the original file include cleanups and removal of unwanted code
 * and also code that depended on build_asert
 */
#ifndef CCAN_HASH_H
#define CCAN_HASH_H
#include <stdint.h>
#include <stdlib.h>
#include <endian.h>

/* Stolen mostly from: lookup3.c, by Bob Jenkins, May 2006, Public Domain.
 *
 * http://burtleburtle.net/bob/c/lookup3.c
 */

#ifdef __LITTLE_ENDIAN
#   define HAVE_LITTLE_ENDIAN 1
#elif __BIG_ENDIAN
#   define HAVE_BIG_ENDIAN 1
#else
#error Unknown endianness.  Failure in endian.h
#endif

/**
 * hash - fast hash of an array for internal use
 * @p: the array or pointer to first element
 * @num: the number of elements to hash
 * @base: the base number to roll into the hash (usually 0)
 *
 * The memory region pointed to by p is combined with the base to form
 * a 32-bit hash.
 *
 * This hash will have different results on different machines, so is
 * only useful for internal hashes (ie. not hashes sent across the
 * network or saved to disk).
 *
 * It may also change with future versions: it could even detect at runtime
 * what the fastest hash to use is.
 *
 * See also: hash64, hash_stable.
 *
 * Example:
 *	#include <ccan/hash/hash.h>
 *	#include <err.h>
 *	#include <stdio.h>
 *	#include <string.h>
 *
 *	// Simple demonstration: idential strings will have the same hash, but
 *	// two different strings will probably not.
 *	int main(int argc, char *argv[])
 *	{
 *		uint32_t hash1, hash2;
 *
 *		if (argc != 3)
 *			err(1, "Usage: %s <string1> <string2>", argv[0]);
 *
 *		hash1 = __nl_hash(argv[1], strlen(argv[1]), 0);
 *		hash2 = __nl_hash(argv[2], strlen(argv[2]), 0);
 *		printf("Hash is %s\n", hash1 == hash2 ? "same" : "different");
 *		return 0;
 *	}
 */
#define __nl_hash(p, num, base) nl_hash_any((p), (num)*sizeof(*(p)), (base))

/* Our underlying operations. */
uint32_t nl_hash_any(const void *key, size_t length, uint32_t base);

#endif /* HASH_H */
