/* gost28147-internal.h

   The GOST 28147-89 cipher function, described in RFC 5831.

   Copyright (C) 2019 Dmitry Eremin-Solenikov

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

#ifndef NETTLE_GOST28147_INTERNAL_H_INCLUDED
#define NETTLE_GOST28147_INTERNAL_H_INCLUDED

#include <stdint.h>

extern const struct gost28147_param _nettle_gost28147_param_test_3411;
extern const struct gost28147_param _nettle_gost28147_param_CryptoPro_3411;

struct gost28147_param
{
  uint32_t sbox[4][256];
};

void _nettle_gost28147_encrypt_block (const uint32_t *key,
				      const uint32_t sbox[4][256],
				      const uint32_t *in, uint32_t *out);

#endif /* NETTLE_GOST28147_INTERNAL_H_INCLUDED */
