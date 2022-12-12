/* nettle-internal.h

   Things that are used only by the testsuite and benchmark, and
   not included in the library.

   Copyright (C) 2002, 2014 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#ifndef NETTLE_INTERNAL_H_INCLUDED
#define NETTLE_INTERNAL_H_INCLUDED

#include <assert.h>

#include "nettle-meta.h"

/* For definition of NETTLE_MAX_HASH_CONTEXT_SIZE. */
#include "sha3.h"

/* Temporary allocation, for systems that don't support alloca. Note
 * that the allocation requests should always be reasonably small, so
 * that they can fit on the stack. For non-alloca systems, we use a
 * fix maximum size + an assert.
 *
 * TMP_DECL and TMP_ALLOC allocate an array of the given type, and
 * take the array size (not byte size) as argument.
 *
 * TMP_DECL_ALIGN and TMP_ALLOC_ALIGN are intended for context
 * structs, which need proper alignment. They take the size in bytes,
 * and produce a void *. On systems without alloca, implemented as an
 * array of uint64_t, to ensure alignment. Since it is used as void *
 * argument, no type casts are needed.
 */

#if HAVE_ALLOCA
# define TMP_DECL(name, type, max) type *name
# define TMP_ALLOC(name, size) (name = alloca(sizeof (*name) * (size)))
# define TMP_DECL_ALIGN(name, max) void *name
# define TMP_ALLOC_ALIGN(name, size) (name = alloca(size))
#else /* !HAVE_ALLOCA */
# define TMP_DECL(name, type, max) type name[max]
# define TMP_ALLOC(name, size) \
  do { assert((size_t)(size) <= (sizeof(name) / sizeof(name[0]))); } while (0)
# define TMP_DECL_ALIGN(name, max) \
  uint64_t name[((max) + (sizeof(uint64_t) - 1))/ sizeof(uint64_t)]
# define TMP_ALLOC_ALIGN(name, size) \
  do { assert((size_t)(size) <= (sizeof(name))); } while (0)
#endif 

/* Arbitrary limits which apply to systems that don't have alloca */
#define NETTLE_MAX_HASH_BLOCK_SIZE 128
#define NETTLE_MAX_HASH_DIGEST_SIZE 64
#define NETTLE_MAX_HASH_CONTEXT_SIZE (sizeof(struct sha3_224_ctx))
#define NETTLE_MAX_SEXP_ASSOC 17
#define NETTLE_MAX_CIPHER_BLOCK_SIZE 32

/* Doesn't quite fit with the other algorithms, because of the weak
 * keys. Weak keys are not reported, the functions will simply crash
 * if you try to use a weak key. */

extern const struct nettle_cipher nettle_des;
extern const struct nettle_cipher nettle_des3;

extern const struct nettle_cipher nettle_blowfish128;

extern const struct nettle_cipher nettle_unified_aes128;
extern const struct nettle_cipher nettle_unified_aes192;
extern const struct nettle_cipher nettle_unified_aes256;

/* Stream ciphers treated as aead algorithms with no authentication. */
extern const struct nettle_aead nettle_arcfour128;
extern const struct nettle_aead nettle_chacha;
extern const struct nettle_aead nettle_salsa20;
extern const struct nettle_aead nettle_salsa20r12;

/* All-in-one CBC encrypt fucntinos treated as AEAD with no
   authentication and no decrypt method. */
extern const struct nettle_aead nettle_cbc_aes128;
extern const struct nettle_aead nettle_cbc_aes192;
extern const struct nettle_aead nettle_cbc_aes256;

extern const struct nettle_aead nettle_openssl_gcm_aes128;
extern const struct nettle_aead nettle_openssl_gcm_aes192;
extern const struct nettle_aead nettle_openssl_gcm_aes256;


/* Glue to openssl, for comparative benchmarking. Code in
 * examples/nettle-openssl.c. */
extern void nettle_openssl_init(void);
extern const struct nettle_cipher nettle_openssl_aes128;
extern const struct nettle_cipher nettle_openssl_aes192;
extern const struct nettle_cipher nettle_openssl_aes256;
extern const struct nettle_cipher nettle_openssl_blowfish128;
extern const struct nettle_cipher nettle_openssl_des;
extern const struct nettle_cipher nettle_openssl_cast128;

extern const struct nettle_hash nettle_openssl_md5;
extern const struct nettle_hash nettle_openssl_sha1;

extern const struct nettle_hash * const _nettle_hashes[];

#endif /* NETTLE_INTERNAL_H_INCLUDED */
