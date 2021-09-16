/* blowfish-internal.h

   Blowfish block cipher.

   Copyright (C) 2014 Niels Möller
   Copyright (C) 1998, 2001 FSF, Ray Dassen, Niels Möller

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

#ifndef NETTLE_BLOWFISH_INTERNAL_H_INCLUDED
#define NETTLE_BLOWFISH_INTERNAL_H_INCLUDED

#include "nettle-types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const struct blowfish_ctx _nettle_blowfish_initial_ctx;
extern void _nettle_blowfish_encround (const struct blowfish_ctx *ctx,
                                       uint32_t * ret_xl, uint32_t * ret_xr);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_BLOWFISH_INTERNAL_H_INCLUDED */
