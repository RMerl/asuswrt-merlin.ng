/* cipher-selftest.h - Helper functions for bulk encryption selftests.
 * Copyright (C) 2013,2020 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
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

#ifndef G10_SELFTEST_HELP_H
#define G10_SELFTEST_HELP_H

#include <config.h>
#include "types.h"
#include "g10lib.h"
#include "cipher.h"

typedef void (*gcry_cipher_bulk_cbc_dec_t)(void *context, unsigned char *iv,
					   void *outbuf_arg,
					   const void *inbuf_arg,
					   size_t nblocks);

typedef void (*gcry_cipher_bulk_cfb_dec_t)(void *context, unsigned char *iv,
					   void *outbuf_arg,
					   const void *inbuf_arg,
					   size_t nblocks);

typedef void (*gcry_cipher_bulk_ctr_enc_t)(void *context, unsigned char *iv,
					   void *outbuf_arg,
					   const void *inbuf_arg,
					   size_t nblocks);

/* Helper function to allocate an aligned context for selftests.  */
void *_gcry_cipher_selftest_alloc_ctx (const int context_size,
                                       unsigned char **r_mem);


/* Helper function for bulk CBC decryption selftest */
const char *
_gcry_selftest_helper_cbc (const char *cipher, gcry_cipher_setkey_t setkey,
			   gcry_cipher_encrypt_t encrypt_one,
			   const int nblocks, const int blocksize,
			   const int context_size);

/* Helper function for bulk CFB decryption selftest */
const char *
_gcry_selftest_helper_cfb (const char *cipher, gcry_cipher_setkey_t setkey,
			   gcry_cipher_encrypt_t encrypt_one,
			   const int nblocks, const int blocksize,
			   const int context_size);

/* Helper function for bulk CTR encryption selftest */
const char *
_gcry_selftest_helper_ctr (const char *cipher, gcry_cipher_setkey_t setkey,
			   gcry_cipher_encrypt_t encrypt_one,
			   const int nblocks, const int blocksize,
			   const int context_size);

#endif /*G10_SELFTEST_HELP_H*/
