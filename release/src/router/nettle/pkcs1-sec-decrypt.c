/* pkcs1-sec-decrypt.c

   The RSA publickey algorithm. Side channel resistant PKCS#1 decryption.

   Copyright (C) 2001, 2012 Niels Möller
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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

#include <string.h>

#include "memops.h"

#include "gmp-glue.h"
#include "pkcs1-internal.h"

/* Inputs are always cast to uint32_t values. But all values used in this
 * function should never exceed the maximum value of a uint32_t anyway.
 * these macros returns 1 on success, 0 on failure */
#define NOT_EQUAL(a, b) \
    ((0U - ((uint32_t)(a) ^ (uint32_t)(b))) >> 31)
#define EQUAL(a, b) \
    ((((uint32_t)(a) ^ (uint32_t)(b)) - 1U) >> 31)
#define GREATER_OR_EQUAL(a, b) \
    (1U - (((uint32_t)(a) - (uint32_t)(b)) >> 31))

int
_pkcs1_sec_decrypt (size_t length, uint8_t *message,
                    size_t padded_message_length,
                    const volatile uint8_t *padded_message)
{
  volatile int ok;
  size_t i, t;

  /* Message independent branch */
  if (length + 11 > padded_message_length)
    return 0;

  t = padded_message_length - length - 1;

  /* Check format, padding, message_size */
  ok = EQUAL(padded_message[0], 0);       /* ok if padded_message[0] == 0 */
  ok &= EQUAL(padded_message[1], 2);      /* ok if padded_message[1] == 2 */
  for (i = 2; i < t; i++)      /* check padding has no zeros */
    {
      ok &= NOT_EQUAL(padded_message[i], 0);
    }
  ok &= EQUAL(padded_message[t], 0);      /* ok if terminator == 0 */

  /* fill destination buffer regardless of outcome */
  cnd_memcpy(ok, message, padded_message + t + 1, length);

  return ok;
}

int
_pkcs1_sec_decrypt_variable(size_t *length, uint8_t *message,
                            size_t padded_message_length,
                            const volatile uint8_t *padded_message)
{
  volatile int not_found = 1;
  volatile int ok;
  volatile size_t offset;
  size_t buflen, msglen;
  size_t shift, i;

  /* Check format, padding, message_size */
  ok = EQUAL(padded_message[0], 0);
  ok &= EQUAL(padded_message[1], 2);

  /* length is discovered in a side-channel silent way.
   * not_found goes to 0 when the terminator is found.
   * offset starts at 3 as it includes the terminator and
   * the format bytes already */
  offset = 3;
  for (i = 2; i < padded_message_length; i++)
    {
      not_found &= NOT_EQUAL(padded_message[i], 0);
      offset += not_found;
    }
  /* check if we ran out of buffer */
  ok &= NOT_EQUAL(not_found, 1);
  /* padding must be >= 11 (2 format bytes + 8 pad bytes min. + terminator) */
  ok &= GREATER_OR_EQUAL(offset, 11);

  /* offset can vary between 3 and padded_message_length, due to the loop
   * above, therefore msglen can't underflow */
  msglen = padded_message_length - offset;

  /* we always fill the whole buffer but only up to
   * padded_message_length length */
  buflen = *length;
  if (buflen > padded_message_length) { /* input independent branch */
    buflen = padded_message_length;
  }

  /* if the message length is larger than the buffer we must fail */
  ok &= GREATER_OR_EQUAL(buflen, msglen);

  /* fill destination buffer fully regardless of outcome. Copies the message
   * in a memory access independent way. The destination message buffer will
   * be clobbered past the message length. */
  shift = padded_message_length - buflen;
  cnd_memcpy(ok, message, padded_message + shift, buflen);
  offset -= shift;
  /* In this loop, the bits of the 'offset' variable are used as shifting
   * conditions, starting from the least significant bit. The end result is
   * that the buffer is shifted left exactly 'offset' bytes. */
  for (shift = 1; shift < buflen; shift <<= 1, offset >>= 1)
    {
      /* 'ok' is both a least significant bit mask and a condition */
      cnd_memcpy(offset & ok, message, message + shift, buflen - shift);
    }

  /* update length only if we succeeded, otherwise leave unchanged */
  *length = (msglen & (-(size_t) ok)) + (*length & ((size_t) ok - 1));

  return ok;
}
