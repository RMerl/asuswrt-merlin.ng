/* gcm-internal.h

   Copyright (C) 2024 Niels Möller

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

#ifndef NETTLE_GCM_INTERNAL_H_INCLUDED
#define NETTLE_GCM_INTERNAL_H_INCLUDED

#include "gcm.h"

#if HAVE_NATIVE_gcm_aes_encrypt

/* Name mangling */
#define _gcm_aes_encrypt _nettle_gcm_aes_encrypt
#define _gcm_aes_decrypt _nettle_gcm_aes_decrypt

/* To reduce the number of arguments (e.g., maximum of 6 register
   arguments on x86_64), pass a pointer to gcm_key, which really is a
   pointer to the first member of the appropriate gcm_aes*_ctx
   struct. */
size_t
_gcm_aes_encrypt (struct gcm_key *key,
		  unsigned rounds,
		  size_t size, uint8_t *dst, const uint8_t *src);

size_t
_gcm_aes_decrypt (struct gcm_key *CTX,
		  unsigned rounds,
		  size_t size, uint8_t *dst, const uint8_t *src);
#else /* !HAVE_NATIVE_gcm_aes_encrypt */
#define _gcm_aes_encrypt(key, rounds, size, dst, src) 0
#define _gcm_aes_decrypt(key, rounds, size, dst, src) 0
#endif /* !HAVE_NATIVE_gcm_aes_encrypt */

#endif /* NETTLE_GCM_INTERNAL_H_INCLUDED */
