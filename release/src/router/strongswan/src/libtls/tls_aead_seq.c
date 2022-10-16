/*
 * Copyright (C) 2020 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
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

#include <bio/bio_writer.h>

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
	 * AEAD transform.
	 */
	aead_t *aead;

	/**
	 * IV derived from key material.
	 */
	chunk_t iv;

	/**
	 * Size of the salt that's internally used by the AEAD implementation.
	 */
	size_t salt;
};

/**
 * Additional data for AEAD (record header)
 */
typedef struct __attribute__((__packed__)) {
	uint8_t type;
	uint16_t version;
	uint16_t length;
} sigheader_t;

/**
 * Generate the IV from the given sequence number.
 */
static bool generate_iv(private_tls_aead_t *this, uint64_t seq, chunk_t iv)
{
	if (iv.len < sizeof(uint64_t) ||
		iv.len < this->iv.len)
	{
		return FALSE;
	}
	memset(iv.ptr, 0, iv.len);
	htoun64(iv.ptr + iv.len - sizeof(uint64_t), seq);
	memxor(iv.ptr + iv.len - this->iv.len, this->iv.ptr, this->iv.len);
	return TRUE;
}

METHOD(tls_aead_t, encrypt, bool,
	private_tls_aead_t *this, tls_version_t version, tls_content_type_t *type,
	uint64_t seq, chunk_t *data)
{
	bio_writer_t *writer;
	chunk_t assoc, encrypted, iv, padding, plain;
	uint8_t icvlen;
	sigheader_t hdr;

	iv = chunk_alloca(this->aead->get_iv_size(this->aead));
	if (!generate_iv(this, seq, iv))
	{
		return FALSE;
	}

	/* no padding for now */
	padding = chunk_empty;
	icvlen = this->aead->get_icv_size(this->aead);

	writer = bio_writer_create(data->len + 1 + padding.len + icvlen);
	writer->write_data(writer, *data);
	writer->write_uint8(writer, *type);
	writer->write_data(writer, padding);
	writer->skip(writer, icvlen);
	encrypted = writer->extract_buf(writer);
	writer->destroy(writer);

	plain = encrypted;
	plain.len -= icvlen;

	hdr.type = TLS_APPLICATION_DATA;
	htoun16(&hdr.version, TLS_1_2);
	htoun16(&hdr.length, encrypted.len);

	assoc = chunk_from_thing(hdr);
	if (!this->aead->encrypt(this->aead, plain, assoc, iv, NULL))
	{
		chunk_free(&encrypted);
		return FALSE;
	}
	chunk_free(data);
	*type = TLS_APPLICATION_DATA;
	*data = encrypted;
	return TRUE;
}

METHOD(tls_aead_t, decrypt, bool,
	private_tls_aead_t *this, tls_version_t version, tls_content_type_t *type,
	uint64_t seq, chunk_t *data)
{
	chunk_t assoc, iv;
	uint8_t icvlen;
	sigheader_t hdr;

	iv = chunk_alloca(this->aead->get_iv_size(this->aead));
	if (!generate_iv(this, seq, iv))
	{
		return FALSE;
	}

	icvlen = this->aead->get_icv_size(this->aead);
	if (data->len < icvlen)
	{
		return FALSE;
	}

	hdr.type = TLS_APPLICATION_DATA;
	htoun16(&hdr.version, TLS_1_2);
	htoun16(&hdr.length, data->len);

	assoc = chunk_from_thing(hdr);
	if (!this->aead->decrypt(this->aead, *data, assoc, iv, NULL))
	{
		return FALSE;
	}
	data->len -= icvlen;

	while (data->len && !data->ptr[data->len-1])
	{	/* ignore any padding */
		data->len--;
	}
	if (data->len < 1)
	{
		return FALSE;
	}
	*type = data->ptr[data->len-1];
	data->len--;
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
	/* our AEAD implementations add the salt length here, so subtract it */
	return this->aead->get_key_size(this->aead) - this->salt;
}

METHOD(tls_aead_t, get_iv_size, size_t,
	private_tls_aead_t *this)
{
	/* analogous to the change above, we add the salt length here */
	return this->aead->get_iv_size(this->aead) + this->salt;
}

METHOD(tls_aead_t, set_keys, bool,
	private_tls_aead_t *this, chunk_t mac, chunk_t encr, chunk_t iv)
{
	chunk_t key, salt;
	bool success;

	if (mac.len || iv.len < this->salt)
	{
		return FALSE;
	}

	/* we have to recombine the keys as our AEAD implementations expect the
	 * salt as part of the key */
	chunk_clear(&this->iv);
	chunk_split(iv, "ma", this->salt, &salt, iv.len - this->salt, &this->iv);
	key = chunk_cata("cc", encr, salt);
	success = this->aead->set_key(this->aead, key);
	memwipe(key.ptr, key.len);
	return success;
}

METHOD(tls_aead_t, destroy, void,
	private_tls_aead_t *this)
{
	this->aead->destroy(this->aead);
	chunk_clear(&this->iv);
	free(this);
}

/*
 * Described in header
 */
tls_aead_t *tls_aead_create_seq(encryption_algorithm_t encr, size_t encr_size)
{
	private_tls_aead_t *this;
	size_t salt;

	switch (encr)
	{
		case ENCR_AES_GCM_ICV16:
		case ENCR_CHACHA20_POLY1305:
			salt = 4;
			break;
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV16:
			salt = 3;
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
