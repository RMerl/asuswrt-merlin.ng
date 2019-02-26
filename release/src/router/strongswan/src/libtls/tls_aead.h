/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup tls_aead tls_aead
 * @{ @ingroup tls
 */

#ifndef TLS_AEAD_H_
#define TLS_AEAD_H_

typedef struct tls_aead_t tls_aead_t;

#include "tls.h"

/**
 * TLS specific AEAD interface, includes padding.
 *
 * As TLS uses sign-then-encrypt instead of the more modern encrypt-then-sign,
 * we can't directly abstract traditional transforms using our aead_t interface.
 * With traditional transforms, the AEAD operation has to manage padding, as
 * the MAC is calculated over unpadded data.
 */
struct tls_aead_t {

	/**
	 * Encrypt and sign a TLS record.
	 *
	 * The plain data chunk gets freed on success, and the data chunk
	 * gets updated with a new allocation of the encrypted data.
	 * If next_iv is given, it must contain the IV for this operation. It
	 * gets updated to the IV for the next record.
	 *
	 * @param version		TLS version
	 * @param type			TLS content type
	 * @param seq			record sequence number
	 * @param data			data to encrypt, encryption result
	 * @return				TRUE if successfully encrypted
	 */
	bool (*encrypt)(tls_aead_t *this, tls_version_t version,
					tls_content_type_t type, uint64_t seq, chunk_t *data);

	/**
	 * Decrypt and verify a TLS record.
	 *
	 * The passed encrypted data chunk gets updated to the decrypted record
	 * length, decryption is done inline.
	 *
	 * @param version		TLS version
	 * @param type			TLS content type
	 * @param seq			record sequence number
	 * @param data			data to decrypt, decrypted result
	 * @return				TRUE if successfully decrypted
	 */
	bool (*decrypt)(tls_aead_t *this, tls_version_t version,
					tls_content_type_t type, uint64_t seq, chunk_t *data);

	/**
	 * Get the authentication key size.
	 *
	 * @return		key size, in bytes, 0 if not used
	 */
	size_t (*get_mac_key_size)(tls_aead_t *this);

	/**
	 * Get the encryption key size, if used.
	 *
	 * @return		key size, in bytes, 0 if not used
	 */
	size_t (*get_encr_key_size)(tls_aead_t *this);

	/**
	 * Get the size of implicit IV (or AEAD salt), if used.
	 *
	 * @return		IV/salt size, in bytes, 0 if not used
	 */
	size_t (*get_iv_size)(tls_aead_t *this);

	/**
	 * Set the keys used by an AEAD transform.
	 *
	 * @param mac		authentication key, if used
	 * @param encr		encryption key, if used
	 * @param iv		initial implicit IV or AEAD salt, if any
	 * @return			TRUE if key valid and set
	 */
	bool (*set_keys)(tls_aead_t *this, chunk_t mac, chunk_t ecnr, chunk_t iv);

	/**
	 * Destroy a tls_aead_t.
	 */
	void (*destroy)(tls_aead_t *this);
};

/**
 * Create a tls_aead instance using traditional transforms, explicit IV.
 *
 * An explicit IV means that the IV is prepended to each TLS record. This is
 * the mechanism used in TLS 1.1 and newer.
 *
 * @param mac			integrity protection algorithm
 * @param encr			encryption algorithm
 * @param encr_size		encryption key size, in bytes
 * @return				TLS AEAD transform
 */
tls_aead_t *tls_aead_create_explicit(integrity_algorithm_t mac,
								encryption_algorithm_t encr, size_t encr_size);

/**
 * Create a tls_aead instance using traditional transforms, implicit IV.
 *
 * An implicit IV uses a first IV derived from the TLS keymat, which then
 * gets replaced by the last encrypted records tail. This is the mechanism
 * used for TLS 1.0 and older.
 *
 * @param mac			integrity protection algorithm
 * @param encr			encryption algorithm
 * @param encr_size		encryption key size, in bytes
 * @return				TLS AEAD transform
 */
tls_aead_t *tls_aead_create_implicit(integrity_algorithm_t mac,
								encryption_algorithm_t encr, size_t encr_size);

/**
 * Create a tls_aead instance using NULL encryption.
 *
 * As no IV is involved with null encryption, this AEAD works with any
 * version of TLS.
 *
 * @param mac			integrity protection algorithm
 * @return				TLS AEAD transform
 */
tls_aead_t *tls_aead_create_null(integrity_algorithm_t mac);

/**
 * Create a tls_aead instance using real a AEAD cipher.
 *
 * @param encr			AEAD encryption algorithm
 * @param encr_size		encryption key size, in bytes
 * @return				TLS AEAD transform
 */
tls_aead_t *tls_aead_create_aead(encryption_algorithm_t encr, size_t encr_size);

#endif /** TLS_AEAD_H_ @}*/
