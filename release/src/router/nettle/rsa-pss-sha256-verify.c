/* rsa-pss-sha256-verify.c

   Verifying signatures created with RSA and SHA-256, with PSS padding.

   Copyright (C) 2017 Daiki Ueno

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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "rsa.h"

#include "bignum.h"
#include "pss.h"

int
rsa_pss_sha256_verify_digest(const struct rsa_public_key *key,
			     size_t salt_length,
			     const uint8_t *digest,
			     const mpz_t signature)
{
  int res;
  mpz_t m;

  mpz_init (m);

  res = (_rsa_verify_recover(key, m, signature) &&
	 pss_verify_mgf1(m, mpz_sizeinbase(key->n, 2) - 1, &nettle_sha256,
			 salt_length, digest));

  mpz_clear (m);
  return res;
}
