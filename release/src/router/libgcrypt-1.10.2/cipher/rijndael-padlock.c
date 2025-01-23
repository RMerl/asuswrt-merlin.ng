/* Padlock accelerated AES for Libgcrypt
 * Copyright (C) 2000, 2001, 2002, 2003, 2007,
 *               2008, 2011, 2012 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memcmp() */

#include "types.h"  /* for byte and u32 typedefs */
#include "g10lib.h"
#include "cipher.h"
#include "bufhelp.h"
#include "cipher-selftest.h"
#include "rijndael-internal.h"

#ifdef USE_PADLOCK

/* Encrypt or decrypt one block using the padlock engine.  A and B may
   be the same. */
static unsigned int
do_padlock (const RIJNDAEL_context *ctx, unsigned char *bx,
            const unsigned char *ax, int decrypt_flag)
{
  /* BX and AX are not necessary correctly aligned.  Thus we need to
     copy them here. */
  unsigned char a[16] __attribute__ ((aligned (16)));
  unsigned char b[16] __attribute__ ((aligned (16)));
  unsigned int cword[4] __attribute__ ((aligned (16)));
  unsigned char *pa = a;
  unsigned char *pb = b;
  int blocks;

  /* The control word fields are:
      127:12   11:10 9     8     7     6     5     4     3:0
      RESERVED KSIZE CRYPT INTER KEYGN CIPHR ALIGN DGEST ROUND  */
  cword[0] = (ctx->rounds & 15);  /* (The mask is just a safeguard.)  */
  cword[1] = 0;
  cword[2] = 0;
  cword[3] = 0;
  if (decrypt_flag)
    cword[0] |= 0x00000200;

  memcpy (a, ax, 16);

  blocks = 1; /* Init counter for just one block.  */
#ifdef __x86_64__
  asm volatile
    ("pushfq\n\t"          /* Force key reload.  */
     "popfq\n\t"
     ".byte 0xf3, 0x0f, 0xa7, 0xc8\n\t" /* REP XCRYPT ECB. */
     : "+S" (pa), "+D" (pb), "+c" (blocks)
     : "d" (cword), "b" (ctx->padlockkey)
     : "cc", "memory"
     );
#else
  asm volatile
    ("pushfl\n\t"          /* Force key reload.  */
     "popfl\n\t"
     "xchg %4, %%ebx\n\t"  /* Load key.  */
     ".byte 0xf3, 0x0f, 0xa7, 0xc8\n\t" /* REP XCRYPT ECB. */
     "xchg %4, %%ebx\n"    /* Restore GOT register.  */
     : "+S" (pa), "+D" (pb), "+c" (blocks)
     : "d" (cword), "r" (ctx->padlockkey)
     : "cc", "memory"
     );
#endif

  memcpy (bx, b, 16);

  return (48 + 15 /* possible padding for alignment */);
}

unsigned int
_gcry_aes_padlock_encrypt (const RIJNDAEL_context *ctx,
                           unsigned char *bx, const unsigned char *ax)
{
  return do_padlock(ctx, bx, ax, 0);
}

unsigned int
_gcry_aes_padlock_decrypt (const RIJNDAEL_context *ctx,
                           unsigned char *bx, const unsigned char *ax)
{
  return do_padlock(ctx, bx, ax, 1);
}

void
_gcry_aes_padlock_prepare_decryption (RIJNDAEL_context *ctx)
{
  /* Padlock does not need decryption subkeys. */
  (void)ctx;
}
#endif /* USE_PADLOCK */
