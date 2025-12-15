/* ocb-aes128.c

   Copyright (C) 2022 Niels Möller

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

#include "ocb.h"

void
ocb_aes128_set_encrypt_key (struct ocb_aes128_encrypt_key *ocb_key, const uint8_t *key)
{
  aes128_set_encrypt_key (&ocb_key->encrypt, key);
  ocb_set_key (&ocb_key->ocb, &ocb_key->encrypt, (nettle_cipher_func *) aes128_encrypt);
}

void
ocb_aes128_set_decrypt_key (struct ocb_aes128_encrypt_key *ocb_key, struct aes128_ctx *decrypt,
			    const uint8_t *key)
{
  ocb_aes128_set_encrypt_key (ocb_key, key);
  aes128_invert_key (decrypt, &ocb_key->encrypt);
}

void
ocb_aes128_set_nonce (struct ocb_ctx *ctx, const struct ocb_aes128_encrypt_key *key,
		      size_t tag_length, size_t nonce_length, const uint8_t *nonce)
{
  ocb_set_nonce (ctx, &key->encrypt, (nettle_cipher_func *) aes128_encrypt,
		 tag_length, nonce_length, nonce);
}

void
ocb_aes128_update (struct ocb_ctx *ctx, const struct ocb_aes128_encrypt_key *key,
		   size_t length, const uint8_t *data)
{
  ocb_update (ctx, &key->ocb, &key->encrypt, (nettle_cipher_func *) aes128_encrypt,
	      length, data);
}

void
ocb_aes128_encrypt(struct ocb_ctx *ctx, const struct ocb_aes128_encrypt_key *key,
		   size_t length, uint8_t *dst, const uint8_t *src)
{
  ocb_encrypt (ctx, &key->ocb, &key->encrypt, (nettle_cipher_func *) aes128_encrypt,
	       length, dst, src);
}

void
ocb_aes128_decrypt(struct ocb_ctx *ctx, const struct ocb_aes128_encrypt_key *key,
		   const struct aes128_ctx *decrypt,
		   size_t length, uint8_t *dst, const uint8_t *src)
{
  ocb_decrypt (ctx, &key->ocb, &key->encrypt, (nettle_cipher_func *) aes128_encrypt,
	       decrypt, (nettle_cipher_func *) aes128_decrypt,
	       length, dst, src);
}

void
ocb_aes128_digest(struct ocb_ctx *ctx, const struct ocb_aes128_encrypt_key *key,
		  size_t length, uint8_t *digest)
{
  ocb_digest (ctx, &key->ocb, &key->encrypt, (nettle_cipher_func *) aes128_encrypt,
	      length, digest);
}

void
ocb_aes128_encrypt_message (const struct ocb_aes128_encrypt_key *key,
			    size_t nlength, const uint8_t *nonce,
			    size_t alength, const uint8_t *adata,
			    size_t tlength,
			    size_t clength, uint8_t *dst, const uint8_t *src)
{
  ocb_encrypt_message (&key->ocb, &key->encrypt, (nettle_cipher_func *) aes128_encrypt,
		       nlength, nonce, alength, adata, tlength, clength, dst, src);
}

int
ocb_aes128_decrypt_message (const struct ocb_aes128_encrypt_key *key,
			    const struct aes128_ctx *decrypt,
			    size_t nlength, const uint8_t *nonce,
			    size_t alength, const uint8_t *adata,
			    size_t tlength,
			    size_t mlength, uint8_t *dst, const uint8_t *src)
{
  return ocb_decrypt_message (&key->ocb, &key->encrypt, (nettle_cipher_func *) aes128_encrypt,
			      &decrypt, (nettle_cipher_func *) aes128_decrypt,
			      nlength, nonce, alength, adata,
			      tlength, mlength, dst, src);
}
