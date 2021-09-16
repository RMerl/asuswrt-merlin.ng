/* hogweed-internal.h

   Bignum operations that are missing from gmp.

   Copyright (C) 2001 Niels Möller

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

#ifndef NETTLE_HOGWEED_INTERNAL_H_INCLUDED
#define NETTLE_HOGWEED_INTERNAL_H_INCLUDED

void
_nettle_generate_pocklington_prime (mpz_t p, mpz_t r,
				    unsigned bits, int top_bits_set,
				    void *ctx, nettle_random_func *random,
				    const mpz_t p0,
				    const mpz_t q,
				    const mpz_t p0q);

#define _pkcs1_signature_prefix _nettle_pkcs1_signature_prefix

uint8_t *
_pkcs1_signature_prefix(unsigned key_size,
			uint8_t *buffer,
			unsigned id_size,
			const uint8_t *id,
			unsigned digest_size);

#endif /* NETTLE_HOGWEED_INTERNAL_H_INCLUDED */
