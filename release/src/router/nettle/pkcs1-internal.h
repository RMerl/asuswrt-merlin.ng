/* pkcs1-internal.h

   Copyright (C) 2018 Niels Möller
   Copyright (C) 2018 Red Hat, Inc.

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

#ifndef NETTLE_PKCS1_INTERNAL_H_INCLUDED
#define NETTLE_PKCS1_INTERNAL_H_INCLUDED

#include "nettle-types.h"

#define _pkcs1_sec_decrypt _nettle_pkcs1_sec_decrypt
#define _pkcs1_sec_decrypt_variable _nettle_pkcs1_sec_decrypt_variable

/* additional resistance to memory access side-channel attacks.
 * Note: message buffer is returned unchanged on error */
int
_pkcs1_sec_decrypt (size_t length, uint8_t *message,
                    size_t padded_message_length,
                    const volatile uint8_t *padded_message);

int
_pkcs1_sec_decrypt_variable(size_t *length, uint8_t *message,
                            size_t padded_message_length,
                            const volatile uint8_t *padded_message);

#endif /* NETTLE_PKCS1_INTERNAL_H_INCLUDED */
