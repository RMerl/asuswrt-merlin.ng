/* chacha-internal.h

   The ChaCha stream cipher.

   Copyright (C) 2013 Joachim Strömbergson
   Copyright (C) 2012 Simon Josefsson
   Copyright (C) 2014 Niels Möller

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

#ifndef NETTLE_CHACHA_INTERNAL_H_INCLUDED
#define NETTLE_CHACHA_INTERNAL_H_INCLUDED

#include "nettle-types.h"
#include "chacha.h"

void
_nettle_chacha_core(uint32_t *dst, const uint32_t *src, unsigned rounds);

/* Functions available only in some configurations */
void
_nettle_chacha_2core(uint32_t *dst, const uint32_t *src, unsigned rounds);

void
_nettle_chacha_2core32(uint32_t *dst, const uint32_t *src, unsigned rounds);

void
_nettle_chacha_3core(uint32_t *dst, const uint32_t *src, unsigned rounds);

void
_nettle_chacha_3core32(uint32_t *dst, const uint32_t *src, unsigned rounds);

void
_nettle_chacha_4core(uint32_t *dst, const uint32_t *src, unsigned rounds);

void
_nettle_chacha_4core32(uint32_t *dst, const uint32_t *src, unsigned rounds);

void
_nettle_chacha_crypt_1core(struct chacha_ctx *ctx,
			   size_t length,
			   uint8_t *dst,
			   const uint8_t *src);

void
_nettle_chacha_crypt_3core(struct chacha_ctx *ctx,
			   size_t length,
			   uint8_t *dst,
			   const uint8_t *src);

void
_nettle_chacha_crypt_4core(struct chacha_ctx *ctx,
			   size_t length,
			   uint8_t *dst,
			   const uint8_t *src);

void
_nettle_chacha_crypt32_1core(struct chacha_ctx *ctx,
			     size_t length,
			     uint8_t *dst,
			     const uint8_t *src);

void
_nettle_chacha_crypt32_3core(struct chacha_ctx *ctx,
			     size_t length,
			     uint8_t *dst,
			     const uint8_t *src);

void
_nettle_chacha_crypt32_4core(struct chacha_ctx *ctx,
			     size_t length,
			     uint8_t *dst,
			     const uint8_t *src);

#endif /* NETTLE_CHACHA_INTERNAL_H_INCLUDED */
