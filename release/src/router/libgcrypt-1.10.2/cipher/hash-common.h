/* hash-common.h - Declarations of common code for hash algorithms.
 * Copyright (C) 2008 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GCRY_HASH_COMMON_H
#define GCRY_HASH_COMMON_H

#include "types.h"


const char * _gcry_hash_selftest_check_one
/**/         (int algo,
              int datamode, const void *data, size_t datalen,
              const void *expect, size_t expectlen);

/* Type for the md_write helper function.  */
typedef unsigned int (*_gcry_md_block_write_t) (void *c,
						const unsigned char *blks,
						size_t nblks);

#if (defined(USE_SHA512) || defined(USE_WHIRLPOOL))
/* SHA-512 and Whirlpool needs u64. SHA-512 needs larger buffer. */
# define MD_BLOCK_MAX_BLOCKSIZE 128
# define MD_NBLOCKS_TYPE u64
#else
# define MD_BLOCK_MAX_BLOCKSIZE 64
# define MD_NBLOCKS_TYPE u32
#endif

/* SHA1 needs 2x64 bytes and SHA-512 needs 128 bytes. */
#define MD_BLOCK_CTX_BUFFER_SIZE 128

typedef struct gcry_md_block_ctx
{
    byte buf[MD_BLOCK_CTX_BUFFER_SIZE];
    MD_NBLOCKS_TYPE nblocks;
    MD_NBLOCKS_TYPE nblocks_high;
    int count;
    unsigned int blocksize_shift;
    _gcry_md_block_write_t bwrite;
} gcry_md_block_ctx_t;


void
_gcry_md_block_write( void *context, const void *inbuf_arg, size_t inlen);

#endif /*GCRY_HASH_COMMON_H*/
