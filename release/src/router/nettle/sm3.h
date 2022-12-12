/* sm3.h

   The SM3 hash function.

   Copyright (C) 2017 Jia Zhang
   Copyright (C) 2021 Tianjia Zhang <tianjia.zhang@linux.alibaba.com>

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

#ifndef NETTLE_SM3_H_INCLUDED
#define NETTLE_SM3_H_INCLUDED

#include "nettle-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define sm3_init nettle_sm3_init
#define sm3_update nettle_sm3_update
#define sm3_digest nettle_sm3_digest

#define SM3_DIGEST_SIZE 32
#define SM3_BLOCK_SIZE 64

/* Digest is kept internally as 8 32-bit words. */
#define _SM3_DIGEST_LENGTH 8

struct sm3_ctx
{
  uint32_t state[_SM3_DIGEST_LENGTH];
  uint64_t count;               /* Block count */
  unsigned index;               /* Into buffer */
  uint8_t block[SM3_BLOCK_SIZE]; /* Block buffer */
};

void
sm3_init(struct sm3_ctx *ctx);

void
sm3_update(struct sm3_ctx *ctx,
	   size_t length,
	   const uint8_t *data);

void
sm3_digest(struct sm3_ctx *ctx,
	   size_t length,
	   uint8_t *digest);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_SM3_H_INCLUDED */
