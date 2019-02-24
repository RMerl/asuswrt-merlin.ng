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

#include "tls_aead.h"

#include <crypto/iv/iv_gen_rand.h>

typedef struct private_tls_aead_t private_tls_aead_t;

/**
 * Private data of an tls_aead_t object.
 */
struct private_tls_aead_t {

	/**
	 * Public tls_aead_t interface.
	 */
	tls_aead_t public;

	/**
	 * AEAD transform
	 */
	aead_t *aead;

	/**
	 * Size of salt, the implicit nonce
	 */
	size_t salt;
};

/**
 * Associated header data to create signature over
 */
typedef struct __attribute__((__packed__)) {
	uint64_t seq;
	uint8_t type;
	uint16_t version;
	uint16_t length;
} sigheader_t;

METHOD(tls_aead_t, encrypt, bool,
	private_tls_aead_t *this, tls_version_t version, tls_content_type_t type,
	uint64_t seq, chunk_t *data)
{
	chunk_t assoc, encrypted, iv, plain;
	uint8_t icvlen;
	sigheader_t hdr;
	iv_gen_t *gen;

	gen = this->aead->get_iv_gen(this->aead);
	iv.len = this->aead->get_iv_size(this->aead);
	icvlen = this->aead->get_icv_size(this->aead);

	encrypted = chunk_alloc(iv.len + data->len + icvlen);
	iv.ptr = encrypted.ptr;
	if (!gen->get_iv(gen, seq, iv.len, iv.ptr))
	{
		chunk_free(&encrypted);
		return FALSE;
	}
	memcpy(encrypted.ptr + iv.len, data->ptr, data->len);
	plain = chunk_skip(encrypted, iv.len);
	plain.len -= icvlen;

	hdr.type = type;
	htoun64(&hdr.seq, seq);
	htoun16(&hdr.version, version);
	htoun16(&hdr.length, plain.len);

	assoc = chunk_from_thing(hdr);
	if (!this->aead->encrypt(this->aead, plain, assoc, iv, NULL))
	{
		chunk_free(&encrypted);
		return FALSE;
	}
	chunk_free(data);
	*data = encrypted;
	return TRUE;
}

METHOD(tls_aead_t, decrypt, bool,
	private_tls_aead_t *this, tls_version_t version, tls_content_type_t type,
	uint64_t seq, chunk_t *data)
{
	chunk_t assoc, iv;
	uint8_t icvlen;
	sigheader_t hdr;

	iv.len = this->aead->get_iv_size(this->aead);
	if (data->len < iv.len)
	{
		return FALSE;
	}
	iv.ptr = data->ptr;
	*data = chunk_skip(*data, iv.len);
	icvlen = this->aead->get_icv_size(this->aead);
	if (data->len < icvlen)
	{
		return FALSE;
	}

	hdr.type = type;
	htoun64(&hdr.seq, seq);
	htoun16(&hdr.version, version);
	htoun16(&hdr.length, data->len - icvlen);

	assoc = chunk_from_thing(hdr);
	if (!this->aead->decrypt(this->aead, *data, assoc, iv, NULL))
	{
		return FALSE;
	}
	data->len -= icvlen;
	return TRUE;
}

METHOD(tls_aead_t, get_mac_key_size, size_t,
	private_tls_aead_t *this)
{
	return 0;
}

METHOD(tls_aead_t, get_encr_key_size, size_t,
	private_tls_aead_t *this)
{
	return this->aead->get_key_size(this->aead) - this->salt;
}

METHOD(tls_aead_t, get_iv_size, size_t,
	private_tls_aead_t *this)
{
	return this->salt;
}

METHOD(tls_aead_t, set_keys, bool,
	private_tls_aead_t *this, chunk_t mac, chunk_t encr, chunk_t iv)
{
	chunk_t key;

	if (mac.len)
	{
		return FALSE;
	}
	key = chunk_cata("cc", encr, iv);
	return this->aead->set_key(this->aead, key);
}

METHOD(tls_aead_t, destroy, void,
	private_tls_aead_t *this)
{
	this->aead->destroy(this->aead);
	free(this);
}

/**
 * See header
 */
tls_aead_t *tls_aead_create_aead(encryption_algorithm_t encr, size_t encr_size)
{
	private_tls_aead_t *this;
	size_t salt;

	switch (encr)
	{
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
		case ENCR_CAMELLIA_CCM_ICV8:
		case ENCR_CAMELLIA_CCM_ICV12:
		case ENCR_CAMELLIA_CCM_ICV16:
			salt = 4;
			break;
		default:
			return NULL;
	}

	INIT(this,
		.public = {
			.encrypt = _encrypt,
			.decrypt = _decrypt,
			.get_mac_key_size = _get_mac_key_size,
			.get_encr_key_size = _get_encr_key_size,
			.get_iv_size = _get_iv_size,
			.set_keys = _set_keys,
			.destroy = _destroy,
		},
		.aead = lib->crypto->create_aead(lib->crypto, encr, encr_size, salt),
		.salt = salt,
	);

	if (!this->aead)
	{
		free(this);
		return NULL;
	}

	if (this->aead->get_block_size(this->aead) != 1)
	{	/* TLS does not define any padding scheme for AEAD */
		destroy(this);
		return NULL;
	}

	return &this->public;
}
