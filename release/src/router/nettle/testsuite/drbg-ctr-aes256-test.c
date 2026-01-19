/* drbg-ctr-aes256-test.c

   Copyright (C) 2023 Simon Josefsson

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

#include "testutils.h"

#include "drbg-ctr.h"

void
test_main (void)
{
  struct drbg_ctr_aes256_ctx rng;
  uint8_t seed_material[DRBG_CTR_AES256_SEED_SIZE];
  uint8_t tmp[DRBG_CTR_AES256_SEED_SIZE];
  size_t i;

  /* https://ntruprime.cr.yp.to/nist/ntruprime-20201007/Reference_Implementation/kem/sntrup761/nist/kat_kem.c.html
     https://ntruprime.cr.yp.to/nist/ntruprime-20201007/KAT/kem/sntrup761/kat_kem.req.html
   */

  for (i = 0; i < DRBG_CTR_AES256_SEED_SIZE; i++)
    seed_material[i] = i;

  drbg_ctr_aes256_init (&rng, seed_material);

  drbg_ctr_aes256_random (&rng, DRBG_CTR_AES256_SEED_SIZE, tmp);
  if (!MEMEQ (DRBG_CTR_AES256_SEED_SIZE,
	      tmp,
	      H ("061550234D158C5EC95595FE04EF7A25767F2E24CC2BC479"
		 "D09D86DC9ABCFDE7056A8C266F9EF97ED08541DBD2E1FFA1")))
    {
      printf ("drbg_ctr_aes256 = ");
      print_hex (DRBG_CTR_AES256_SEED_SIZE, tmp);
      abort ();
    }

  drbg_ctr_aes256_random (&rng, DRBG_CTR_AES256_SEED_SIZE, tmp);
  if (!MEMEQ (DRBG_CTR_AES256_SEED_SIZE,
	      tmp,
	      H ("D81C4D8D734FCBFBEADE3D3F8A039FAA2A2C9957E835AD55"
		 "B22E75BF57BB556AC81ADDE6AEEB4A5A875C3BFCADFA958F")))
    {
      printf ("drbg_ctr_aes256 = ");
      print_hex (DRBG_CTR_AES256_SEED_SIZE, tmp);
      abort ();
    }
}
