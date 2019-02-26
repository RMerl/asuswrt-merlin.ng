/*
 * Copyright (C) 2012-2013 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
 * HSR Hochschule fuer Technik Rapperswil
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

#include <limits.h>
#include <stdint.h>

#include "esp_context.h"

#include <library.h>
#include <utils/debug.h>

/**
 * Should be a multiple of 8
 */
#define ESP_DEFAULT_WINDOW_SIZE 128

typedef struct private_esp_context_t private_esp_context_t;

/**
 * Private additions to esp_context_t.
 */
struct private_esp_context_t {

	/**
	 * Public members
	 */
	esp_context_t public;

	/**
	 * AEAD wrapper or method to encrypt/decrypt/authenticate ESP packets
	 */
	aead_t *aead;

	/**
	 * The highest sequence number that was successfully verified
	 * and authenticated, or assigned in an outbound context
	 */
	uint32_t last_seqno;

	/**
	 * The bit in the window of the highest authenticated sequence number
	 */
	u_int seqno_index;

	/**
	 * The size of the anti-replay window (in bits)
	 */
	u_int window_size;

	/**
	 * The anti-replay window buffer
	 */
	chunk_t window;

	/**
	 * TRUE in case of an inbound ESP context
	 */
	bool inbound;
};

/**
 * Set or unset a bit in the window.
 */
static inline void set_window_bit(private_esp_context_t *this,
								  u_int index, bool set)
{
	u_int i = index / CHAR_BIT;

	if (set)
	{
		this->window.ptr[i] |= 1 << (index % CHAR_BIT);
	}
	else
	{
		this->window.ptr[i] &= ~(1 << (index % CHAR_BIT));
	}
}

/**
 * Get a bit from the window.
 */
static inline bool get_window_bit(private_esp_context_t *this, u_int index)
{
	u_int i = index / CHAR_BIT;

	return this->window.ptr[i] & (1 << index % CHAR_BIT);
}

/**
 * Returns TRUE if the supplied seqno is not already marked in the window
 */
static bool check_window(private_esp_context_t *this, uint32_t seqno)
{
	u_int offset;

	offset = this->last_seqno - seqno;
	offset = (this->seqno_index - offset) % this->window_size;
	return !get_window_bit(this, offset);
}

METHOD(esp_context_t, verify_seqno, bool,
	private_esp_context_t *this, uint32_t seqno)
{
	if (!this->inbound)
	{
		return FALSE;
	}

	if (seqno > this->last_seqno)
	{	/*       |----------------------------------------|
		 *  <---------^   ^   or    <---------^     ^
		 *     WIN    H   S            WIN    H     S
		 */
		return TRUE;
	}
	else if (seqno > 0 && this->window_size > this->last_seqno - seqno)
	{	/*       |----------------------------------------|
		 *  <---------^      or     <---------^
		 *     WIN ^  H                WIN ^  H
		 *         S                       S
		 */
		return check_window(this, seqno);
	}
	else
	{	/*       |----------------------------------------|
		 *                       ^  <---------^
		 *                       S     WIN    H
		 */
		return FALSE;
	}
}

METHOD(esp_context_t, set_authenticated_seqno, void,
	private_esp_context_t *this, uint32_t seqno)
{
	u_int i, shift;

	if (!this->inbound)
	{
		return;
	}

	if (seqno > this->last_seqno)
	{	/* shift the window to the new highest authenticated seqno */
		shift = seqno - this->last_seqno;
		shift = shift < this->window_size ? shift : this->window_size;
		for (i = 0; i < shift; ++i)
		{
			this->seqno_index = (this->seqno_index + 1) % this->window_size;
			set_window_bit(this, this->seqno_index, FALSE);
		}
		set_window_bit(this, this->seqno_index, TRUE);
		this->last_seqno = seqno;
	}
	else
	{	/* seqno is inside the window, set the corresponding window bit */
		i = this->last_seqno - seqno;
		set_window_bit(this, (this->seqno_index - i) % this->window_size, TRUE);
	}
}

METHOD(esp_context_t, get_seqno, uint32_t,
	private_esp_context_t *this)
{
	return this->last_seqno;
}

METHOD(esp_context_t, next_seqno, bool,
	private_esp_context_t *this, uint32_t *seqno)
{
	if (this->inbound || this->last_seqno == UINT32_MAX)
	{	/* inbound or segno would cycle */
		return FALSE;
	}
	*seqno = ++this->last_seqno;
	return TRUE;
}

METHOD(esp_context_t, get_aead, aead_t*,
	private_esp_context_t *this)
{
	return this->aead;
}

METHOD(esp_context_t, destroy, void,
	private_esp_context_t *this)
{
	chunk_free(&this->window);
	DESTROY_IF(this->aead);
	free(this);
}

/**
 * Create an AEAD algorithm
 */
static bool create_aead(private_esp_context_t *this, int alg,
						chunk_t key)
{
	size_t salt = 0;

	switch (alg)
	{
		case ENCR_AES_GCM_ICV8:
		case ENCR_AES_GCM_ICV12:
		case ENCR_AES_GCM_ICV16:
		case ENCR_CHACHA20_POLY1305:
			salt = 4;
			break;
		case ENCR_AES_CCM_ICV8:
		case ENCR_AES_CCM_ICV12:
		case ENCR_AES_CCM_ICV16:
		case ENCR_CAMELLIA_CCM_ICV8:
		case ENCR_CAMELLIA_CCM_ICV12:
		case ENCR_CAMELLIA_CCM_ICV16:
			salt = 3;
			break;
		default:
			break;
	}
	if (salt)
	{
		this->aead = lib->crypto->create_aead(lib->crypto, alg,
											  key.len - salt, salt);
	}
	if (!this->aead)
	{
		DBG1(DBG_ESP, "failed to create ESP context: unsupported AEAD "
			 "algorithm %N", encryption_algorithm_names, alg);
		return FALSE;
	}
	if (!this->aead->set_key(this->aead, key))
	{
		DBG1(DBG_ESP, "failed to create ESP context: setting AEAD key failed");
		return FALSE;
	}
	return TRUE;
}

/**
 * Create AEAD wrapper around traditional encryption/integrity algorithms
 */
static bool create_traditional(private_esp_context_t *this, int enc_alg,
							   chunk_t enc_key, int int_alg, chunk_t int_key)
{
	crypter_t *crypter = NULL;
	signer_t *signer = NULL;
	iv_gen_t *ivg;

	switch (enc_alg)
	{
		case ENCR_AES_CTR:
		case ENCR_CAMELLIA_CTR:
			/* the key includes a 4 byte salt */
			crypter = lib->crypto->create_crypter(lib->crypto, enc_alg,
												  enc_key.len - 4);
			break;
		default:
			crypter = lib->crypto->create_crypter(lib->crypto, enc_alg,
												  enc_key.len);
			break;
	}
	if (!crypter)
	{
		DBG1(DBG_ESP, "failed to create ESP context: unsupported encryption "
			 "algorithm %N", encryption_algorithm_names, enc_alg);
		goto failed;
	}
	if (!crypter->set_key(crypter, enc_key))
	{
		DBG1(DBG_ESP, "failed to create ESP context: setting encryption key "
			 "failed");
		goto failed;
	}

	signer = lib->crypto->create_signer(lib->crypto, int_alg);
	if (!signer)
	{
		DBG1(DBG_ESP, "failed to create ESP context: unsupported integrity "
			 "algorithm %N", integrity_algorithm_names, int_alg);
		goto failed;
	}
	if (!signer->set_key(signer, int_key))
	{
		DBG1(DBG_ESP, "failed to create ESP context: setting signature key "
			 "failed");
		goto failed;
	}
	ivg = iv_gen_create_for_alg(enc_alg);
	if (!ivg)
	{
		DBG1(DBG_ESP, "failed to create ESP context: creating iv gen failed");
		goto failed;
	}
	this->aead = aead_create(crypter, signer, ivg);
	return TRUE;

failed:
	DESTROY_IF(crypter);
	DESTROY_IF(signer);
	return FALSE;
}

/**
 * Described in header.
 */
esp_context_t *esp_context_create(int enc_alg, chunk_t enc_key,
								  int int_alg, chunk_t int_key, bool inbound)
{
	private_esp_context_t *this;

	INIT(this,
		.public = {
			.get_aead = _get_aead,
			.get_seqno = _get_seqno,
			.next_seqno = _next_seqno,
			.verify_seqno = _verify_seqno,
			.set_authenticated_seqno = _set_authenticated_seqno,
			.destroy = _destroy,
		},
		.inbound = inbound,
		.window_size = ESP_DEFAULT_WINDOW_SIZE,
	);

	if (encryption_algorithm_is_aead(enc_alg))
	{
		if (!create_aead(this, enc_alg, enc_key))
		{
			destroy(this);
			return NULL;
		}
	}
	else
	{
		if (!create_traditional(this, enc_alg, enc_key, int_alg, int_key))
		{
			destroy(this);
			return NULL;
		}
	}

	if (inbound)
	{
		this->window = chunk_alloc(this->window_size / CHAR_BIT + 1);
		memset(this->window.ptr, 0, this->window.len);
	}
	return &this->public;
}
