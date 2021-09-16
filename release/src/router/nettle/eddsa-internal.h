/* eddsa.h

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

#ifndef NETTLE_EDDSA_INTERNAL_H
#define NETTLE_EDDSA_INTERNAL_H

#include "nettle-types.h"
#include "bignum.h"

#define _eddsa_compress _nettle_eddsa_compress
#define _eddsa_compress_itch _nettle_eddsa_compress_itch
#define _eddsa_decompress _nettle_eddsa_decompress
#define _eddsa_decompress_itch _nettle_eddsa_decompress_itch
#define _eddsa_hash _nettle_eddsa_hash
#define _eddsa_expand_key _nettle_eddsa_expand_key
#define _eddsa_sign _nettle_eddsa_sign
#define _eddsa_sign_itch _nettle_eddsa_sign_itch
#define _eddsa_verify _nettle_eddsa_verify
#define _eddsa_verify_itch _nettle_eddsa_verify_itch
#define _eddsa_public_key_itch _nettle_eddsa_public_key_itch
#define _eddsa_public_key _nettle_eddsa_public_key

/* Low-level internal functions */

struct ecc_curve;
struct ecc_modulo;

typedef void nettle_eddsa_dom_func(void *ctx);

struct ecc_eddsa
{
  /* Hash function to use */
  nettle_hash_update_func *update;
  nettle_hash_digest_func *digest;
  nettle_eddsa_dom_func *dom;
  /* For generating the secret scalar */
  mp_limb_t low_mask;
  mp_limb_t high_bit;
};

extern const struct ecc_eddsa _nettle_ed25519_sha512;
extern const struct ecc_eddsa _nettle_ed448_shake256;

mp_size_t
_eddsa_compress_itch (const struct ecc_curve *ecc);
void
_eddsa_compress (const struct ecc_curve *ecc, uint8_t *r, mp_limb_t *p,
		 mp_limb_t *scratch);

mp_size_t
_eddsa_decompress_itch (const struct ecc_curve *ecc);
int
_eddsa_decompress (const struct ecc_curve *ecc, mp_limb_t *p,
		   const uint8_t *cp,
		   mp_limb_t *scratch);

void
_eddsa_hash (const struct ecc_modulo *m,
	     mp_limb_t *rp, size_t digest_size, const uint8_t *digest);

mp_size_t
_eddsa_sign_itch (const struct ecc_curve *ecc);

void
_eddsa_sign (const struct ecc_curve *ecc,
	     const struct ecc_eddsa *eddsa,
	     void *ctx,
	     const uint8_t *pub,
	     const uint8_t *k1,
	     const mp_limb_t *k2,
	     size_t length,
	     const uint8_t *msg,
	     uint8_t *signature,
	     mp_limb_t *scratch);

mp_size_t
_eddsa_verify_itch (const struct ecc_curve *ecc);

int
_eddsa_verify (const struct ecc_curve *ecc,
	       const struct ecc_eddsa *eddsa,
	       const uint8_t *pub,
	       const mp_limb_t *A,
	       void *ctx,
	       size_t length,
	       const uint8_t *msg,
	       const uint8_t *signature,
	       mp_limb_t *scratch);

void
_eddsa_expand_key (const struct ecc_curve *ecc,
		   const struct ecc_eddsa *eddsa,
		   void *ctx,
		   const uint8_t *key,
		   uint8_t *digest,
		   mp_limb_t *k2);

mp_size_t
_eddsa_public_key_itch (const struct ecc_curve *ecc);

void
_eddsa_public_key (const struct ecc_curve *ecc,
		   const mp_limb_t *k, uint8_t *pub, mp_limb_t *scratch);

#endif /* NETTLE_EDDSA_INTERNAL_H */
