/* kdf-internal.h  - Internal defs for kdf.c
 * Copyright (C) 2013 g10 Code GmbH
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

#ifndef GCRY_KDF_INTERNAL_H
#define GCRY_KDF_INTERNAL_H

/*-- kdf.c --*/
gpg_err_code_t
_gcry_kdf_pkdf2 (const void *passphrase, size_t passphraselen,
                 int hashalgo,
                 const void *salt, size_t saltlen,
                 unsigned long iterations,
                 size_t keysize, void *keybuffer);

/*-- scrypt.c --*/
gcry_err_code_t
_gcry_kdf_scrypt (const unsigned char *passwd, size_t passwdlen,
                  int algo, int subalgo,
                  const unsigned char *salt, size_t saltlen,
                  unsigned long iterations,
                  size_t dklen, unsigned char *dk);

/*-- blake2.c --*/
gcry_err_code_t
blake2b_vl_hash (const void *in, size_t inlen, size_t outputlen, void *output);

#endif /*GCRY_KDF_INTERNAL_H*/
