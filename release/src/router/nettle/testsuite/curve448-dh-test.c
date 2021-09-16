/* curve448-dh-test.c

   Copyright (C) 2017 Daiki Ueno
   Copyright (C) 2017 Red Hat, Inc.

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

#include "curve448.h"

static void
test_g (const uint8_t *s, const uint8_t *r)
{
  uint8_t p[CURVE448_SIZE];
  curve448_mul_g (p, s);
  if (!MEMEQ (CURVE448_SIZE, p, r))
    {
      printf ("curve448_mul_g failure:\ns = ");
      print_hex (CURVE448_SIZE, s);
      printf ("\np = ");
      print_hex (CURVE448_SIZE, p);
      printf (" (bad)\nr = ");
      print_hex (CURVE448_SIZE, r);
      printf (" (expected)\n");
      abort ();
    }
}

static void
test_a (const uint8_t *s, const uint8_t *b, const uint8_t *r)
{
  uint8_t p[CURVE448_SIZE];
  curve448_mul (p, s, b);
  if (!MEMEQ (CURVE448_SIZE, p, r))
    {
      printf ("curve448_mul failure:\ns = ");
      print_hex (CURVE448_SIZE, s);
      printf ("\nb = ");
      print_hex (CURVE448_SIZE, b);
      printf ("\np = ");
      print_hex (CURVE448_SIZE, p);
      printf (" (bad)\nr = ");
      print_hex (CURVE448_SIZE, r);
      printf (" (expected)\n");
      abort ();
    }
}

void
test_main (void)
{
  /* From RFC 7748. */
  test_g (H("9a8f4925d1519f5775cf46b04b5800d4ee9ee8bae8bc5565d498c28d"
	    "d9c9baf574a9419744897391006382a6f127ab1d9ac2d8c0a598726b"),
	  H("9b08f7cc31b7e3e67d22d5aea121074a273bd2b83de09c63faa73d2c"
	    "22c5d9bbc836647241d953d40c5b12da88120d53177f80e532c41fa0"));
  test_g (H("1c306a7ac2a0e2e0990b294470cba339e6453772b075811d8fad0d1d"
	    "6927c120bb5ee8972b0d3e21374c9c921b09d1b0366f10b65173992d"),
	  H("3eb7a829b0cd20f5bcfc0b599b6feccf6da4627107bdb0d4f345b430"
	    "27d8b972fc3e34fb4232a13ca706dcb57aec3dae07bdc1c67bf33609"));

  test_a (H("9a8f4925d1519f5775cf46b04b5800d4ee9ee8bae8bc5565d498c28d"
	    "d9c9baf574a9419744897391006382a6f127ab1d9ac2d8c0a598726b"),
	  H("3eb7a829b0cd20f5bcfc0b599b6feccf6da4627107bdb0d4f345b430"
	    "27d8b972fc3e34fb4232a13ca706dcb57aec3dae07bdc1c67bf33609"),
	  H("07fff4181ac6cc95ec1c16a94a0f74d12da232ce40a77552281d282b"
	    "b60c0b56fd2464c335543936521c24403085d59a449a5037514a879d"));
  test_a (H("1c306a7ac2a0e2e0990b294470cba339e6453772b075811d8fad0d1d"
	    "6927c120bb5ee8972b0d3e21374c9c921b09d1b0366f10b65173992d"),
	  H("9b08f7cc31b7e3e67d22d5aea121074a273bd2b83de09c63faa73d2c"
	    "22c5d9bbc836647241d953d40c5b12da88120d53177f80e532c41fa0"),
	  H("07fff4181ac6cc95ec1c16a94a0f74d12da232ce40a77552281d282b"
	    "b60c0b56fd2464c335543936521c24403085d59a449a5037514a879d"));
}
