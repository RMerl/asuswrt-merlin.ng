/* Rijndael (AES) for GnuPG - PowerPC Vector Crypto AES implementation
 * Copyright 2021- IBM Inc. All rights reserved
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
 *
 * Alternatively, this code may be used in OpenSSL from The OpenSSL Project,
 * and Cryptogams by Andy Polyakov, and if made part of a release of either
 * or both projects, is thereafter dual-licensed under the license said project
 * is released under.
 */

#include <config.h>

#include "rijndael-internal.h"
#include "cipher-internal.h"
#include "bufhelp.h"

#ifdef USE_PPC_CRYPTO_WITH_PPC9LE


extern size_t _gcry_ppc10_aes_gcm_encrypt (const void *inp, void *out,
                                           size_t len,
                                           const unsigned char *key,
                                           unsigned char iv[16], void *Xip);
extern size_t _gcry_ppc10_aes_gcm_decrypt (const void *inp, void *out,
                                           size_t len,
                                           const unsigned char *key,
                                           unsigned char iv[16], void *Xip);

size_t
_gcry_aes_p10le_gcm_crypt(gcry_cipher_hd_t c, void *outbuf_arg,
                          const void *inbuf_arg, size_t nblocks, int encrypt)
{
  RIJNDAEL_context *ctx = (RIJNDAEL_context *) &c->context.c;
  unsigned char *rk = (unsigned char *) ctx->u1.keyschedule;
  unsigned char *gcm_table = (unsigned char *) c->u_mode.gcm.gcm_table;
  unsigned char *iv = c->u_ctr.ctr;
  unsigned char *Xi = c->u_mode.gcm.u_tag.tag;
  int s = 0;
  int ndone = 0;
  int ctr_reset = 0;
  size_t len = nblocks * GCRY_GCM_BLOCK_LEN;
  u64 blocks_unused;
  u64 nb = nblocks;
  u64 next_ctr = 0;
  unsigned char ctr_saved[12];
  unsigned char *inp = (unsigned char *) inbuf_arg;
  unsigned char *out = (unsigned char *) outbuf_arg;

  /*
   * This is what the aes-gcm asembly code expects some input parameters.
   *
   *   - Number of rounds is at 480 offset from rk (rk->rounds)
   *   - Xi at 256 offset from gcm_table
   */
  gcry_assert (sizeof(c->u_mode.gcm.gcm_table) >= 256 + 16);
  buf_cpy (gcm_table+256, Xi, 16);
  buf_cpy (ctr_saved, c->u_ctr.ctr, 12);

  while (nb)
    {
      blocks_unused = (u64) 0xffffffffU + 1 - (u64) buf_get_be32 (iv + 12);
      if (nb > blocks_unused)
        {
          len = blocks_unused * GCRY_GCM_BLOCK_LEN;
          nb -= blocks_unused;
          next_ctr = blocks_unused;
          ctr_reset = 1;
        }
      else
        {
          len = nb * GCRY_GCM_BLOCK_LEN;
          next_ctr = nb;
          nb = 0;
        }

      if (encrypt)
        s = _gcry_ppc10_aes_gcm_encrypt((const void *) inp, (void *) out, len,
                                        (const unsigned char *) rk, iv,
                                        (void *) gcm_table);
      else
        s = _gcry_ppc10_aes_gcm_decrypt((const void *) inp, (void *) out, len,
                                        (const unsigned char *) rk, iv,
                                        (void *) gcm_table);

      cipher_block_add(c->u_ctr.ctr, next_ctr, GCRY_GCM_BLOCK_LEN);
      if (ctr_reset)
        {
          ctr_reset = 0;
          inp += len;
          out += len;
        }
      buf_cpy (c->u_ctr.ctr, ctr_saved, 12);
      ndone += s;
    }
  buf_cpy (Xi, gcm_table+256, 16);

  /*
   * Return number of blocks done.
   */
  s = ndone / GCRY_GCM_BLOCK_LEN;
  s = nblocks - s;
  return ( s );
}

#endif /* USE_PPC_CRYPTO_WITH_PPC9LE */
