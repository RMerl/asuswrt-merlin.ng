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
	 * traditional signer
	 */
	signer_t *signer;
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
	private_tls_aead_t *this, tls_version_t version,
	tls_content_type_t type, uint64_t seq, chunk_t *data)
{
	chunk_t assoc, mac;
	sigheader_t hdr;

	hdr.type = type;
	htoun64(&hdr.seq, seq);
	htoun16(&hdr.version, version);
	htoun16(&hdr.length, data->len);

	assoc = chunk_from_thing(hdr);
	if (!this->signer->get_signature(this->signer, assoc, NULL) ||
		!this->signer->allocate_signature(this->signer, *data, &mac))
	{
		return FALSE;
	}
	*data = chunk_cat("mm", *data, mac);
	return TRUE;
}

METHOD(tls_aead_t, decrypt, bool,
	private_tls_aead_t *this, tls_version_t version,
	tls_content_type_t type, uint64_t seq, chunk_t *data)
{
	chunk_t assoc, mac;
	sigheader_t hdr;

	mac.len = this->signer->get_block_size(this->signer);
	if (data->len < mac.len)
	{
		return FALSE;
	}
	mac = chunk_skip(*data, data->len - mac.len);
	data->len -= mac.len;

	hdr.type = type;
	htoun64(&hdr.seq, seq);
	htoun16(&hdr.version, version);
	htoun16(&hdr.length, data->len);

	assoc = chunk_from_thing(hdr);
	if (!this->signer->get_signature(this->signer, assoc, NULL) ||
		!this->signer->verify_signature(this->signer, *data, mac))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(tls_aead_t, get_mac_key_size, size_t,
	private_tls_aead_t *this)
{
	return this->signer->get_key_size(this->signer);
}

METHOD(tls_aead_t, get_encr_key_size, size_t,
	private_tls_aead_t *this)
{
	return 0;
}

METHOD(tls_aead_t, get_iv_size, size_t,
	private_tls_aead_t *this)
{
	return 0;
}

METHOD(tls_aead_t, set_keys, bool,
	private_tls_aead_t *this, chunk_t mac, chunk_t encr, chunk_t iv)
{
	if (iv.len || encr.len)
	{
		return FALSE;
	}
	return this->signer->set_key(this->signer, mac);
}

METHOD(tls_aead_t, destroy, void,
	private_tls_aead_t *this)
{
	this->signer->destroy(this->signer);
	free(this);
}

/**
 * See header
 */
tls_aead_t *tls_aead_create_null(integrity_algorithm_t alg)
{
	private_tls_aead_t *this;

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
		.signer = lib->crypto->create_signer(lib->crypto, alg),
	);

	if (!this->signer)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
