/*
 * Copyright (C) 2011-2018 Tobias Brunner
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2005 Jan Hutter
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

#include <stddef.h>
#include <string.h>

#include "encrypted_payload.h"
#include "encrypted_fragment_payload.h"

#include <daemon.h>
#include <encoding/payloads/encodings.h>
#include <collections/linked_list.h>
#include <encoding/parser.h>

typedef struct private_encrypted_payload_t private_encrypted_payload_t;
typedef struct private_encrypted_fragment_payload_t private_encrypted_fragment_payload_t;

struct private_encrypted_payload_t {

	/**
	 * Public encrypted_payload_t interface.
	 */
	encrypted_payload_t public;

	/**
	 * There is no next payload for an encrypted payload,
	 * since encrypted payload MUST be the last one.
	 * next_payload means here the first payload of the
	 * contained, encrypted payload.
	 */
	uint8_t next_payload;

	/**
	 * Flags, including reserved bits
	 */
	uint8_t flags;

	/**
	 * Length of this payload
	 */
	uint16_t payload_length;

	/**
	 * Chunk containing the IV, plain, padding and ICV.
	 */
	chunk_t encrypted;

	/**
	 * AEAD transform to use
	 */
	aead_t *aead;

	/**
	 * Contained payloads
	 */
	linked_list_t *payloads;

	/**
	 * Type of payload, PLV2_ENCRYPTED or PLV1_ENCRYPTED
	 */
	payload_type_t type;
};

struct private_encrypted_fragment_payload_t {

	/**
	 * Public interface.
	 */
	encrypted_fragment_payload_t public;

	/**
	 * The first fragment contains the type of the first payload contained in
	 * the original encrypted payload, for all other fragments it MUST be set
	 * to zero.
	 */
	uint8_t next_payload;

	/**
	 * Flags, including reserved bits
	 */
	uint8_t flags;

	/**
	 * Length of this payload
	 */
	uint16_t payload_length;

	/**
	 * Chunk containing the IV, plain, padding and ICV.
	 */
	chunk_t encrypted;

	/**
	 * Fragment number
	 */
	uint16_t fragment_number;

	/**
	 * Total fragments
	 */
	uint16_t total_fragments;

	/**
	 * AEAD transform to use
	 */
	aead_t *aead;

	/**
	 * Chunk containing the plain packet data.
	 */
	chunk_t plain;
};

/**
 * Encoding rules to parse or generate a IKEv2-Encrypted Payload.
 *
 * The defined offsets are the positions in a object of type
 * private_encrypted_payload_t.
 */
static encoding_rule_t encodings_v2[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_encrypted_payload_t, next_payload)	},
	/* Critical and 7 reserved bits, all stored for reconstruction */
	{ U_INT_8,			offsetof(private_encrypted_payload_t, flags)			},
	/* Length of the whole encrypted payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_encrypted_payload_t, payload_length)	},
	/* encrypted data, stored in a chunk. contains iv, data, padding */
	{ CHUNK_DATA,		offsetof(private_encrypted_payload_t, encrypted)		},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                     Initialization Vector                     !
      !         (length is block size for encryption algorithm)       !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                    Encrypted IKE Payloads                     !
      +               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !               !             Padding (0-255 octets)            !
      +-+-+-+-+-+-+-+-+                               +-+-+-+-+-+-+-+-+
      !                                               !  Pad Length   !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ~                    Integrity Checksum Data                    ~
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/**
 * Encoding rules to parse or generate a complete encrypted IKEv1 message.
 *
 * The defined offsets are the positions in a object of type
 * private_encrypted_payload_t.
 */
static encoding_rule_t encodings_v1[] = {
	/* encrypted data, stored in a chunk */
	{ ENCRYPTED_DATA,	offsetof(private_encrypted_payload_t, encrypted)		},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                    Encrypted IKE Payloads                     !
      +               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !               !             Padding (0-255 octets)            !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/**
 * Encoding rules to parse or generate an IKEv2-Encrypted Fragment Payload.
 *
 * The defined offsets are the positions in a object of type
 * private_encrypted_payload_t.
 */
static encoding_rule_t encodings_fragment[] = {
	/* 1 Byte next payload type, stored in the field next_payload */
	{ U_INT_8,			offsetof(private_encrypted_fragment_payload_t, next_payload)	},
	/* Critical and 7 reserved bits, all stored for reconstruction */
	{ U_INT_8,			offsetof(private_encrypted_fragment_payload_t, flags)			},
	/* Length of the whole encryption payload*/
	{ PAYLOAD_LENGTH,	offsetof(private_encrypted_fragment_payload_t, payload_length)	},
	/* Fragment number */
	{ U_INT_16,			offsetof(private_encrypted_fragment_payload_t, fragment_number)	},
	/* Total number of fragments */
	{ U_INT_16,			offsetof(private_encrypted_fragment_payload_t, total_fragments)	},
	/* encrypted data, stored in a chunk. contains iv, data, padding */
	{ CHUNK_DATA,		offsetof(private_encrypted_fragment_payload_t, encrypted)		},
};

/*
                           1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ! Next Payload  !C!  RESERVED   !         Payload Length        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !        Fragment Number        |        Total Fragments        !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                     Initialization Vector                     !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !                    Encrypted IKE Payloads                     !
      +               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      !               !             Padding (0-255 octets)            !
      +-+-+-+-+-+-+-+-+                               +-+-+-+-+-+-+-+-+
      !                                               !  Pad Length   !
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      ~                    Integrity Checksum Data                    ~
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

METHOD(payload_t, verify, status_t,
	private_encrypted_payload_t *this)
{
	return SUCCESS;
}

METHOD(payload_t, get_encoding_rules, int,
	private_encrypted_payload_t *this, encoding_rule_t **rules)
{
	if (this->type == PLV2_ENCRYPTED)
	{
		*rules = encodings_v2;
		return countof(encodings_v2);
	}
	*rules = encodings_v1;
	return countof(encodings_v1);
}

METHOD(payload_t, get_header_length, int,
	private_encrypted_payload_t *this)
{
	if (this->type == PLV2_ENCRYPTED)
	{
		return 4;
	}
	return 0;
}

METHOD(payload_t, get_type, payload_type_t,
	private_encrypted_payload_t *this)
{
	return this->type;
}

METHOD(payload_t, get_next_type, payload_type_t,
	private_encrypted_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, set_next_type, void,
	private_encrypted_payload_t *this, payload_type_t type)
{
	/* the next payload is set during add, still allow this for IKEv1 */
	this->next_payload = type;
}

/**
 * Get length of encryption/integrity overhead for the given plaintext length
 */
static size_t compute_overhead(aead_t *aead, size_t len)
{
	size_t bs, overhead;

	/* padding */
	bs = aead->get_block_size(aead);
	overhead = bs - (len % bs);
	/* add iv */
	overhead += aead->get_iv_size(aead);
	/* add icv */
	overhead += aead->get_icv_size(aead);
	return overhead;
}

/**
 * Compute the length of the whole payload
 */
static void compute_length(private_encrypted_payload_t *this)
{
	enumerator_t *enumerator;
	payload_t *payload;
	size_t length = 0;

	if (this->encrypted.len)
	{
		length = this->encrypted.len;
	}
	else
	{
		enumerator = this->payloads->create_enumerator(this->payloads);
		while (enumerator->enumerate(enumerator, &payload))
		{
			length += payload->get_length(payload);
		}
		enumerator->destroy(enumerator);

		if (this->aead)
		{
			length += compute_overhead(this->aead, length);
		}
	}
	length += get_header_length(this);
	this->payload_length = length;
}

METHOD2(payload_t, encrypted_payload_t, get_length, size_t,
	private_encrypted_payload_t *this)
{
	compute_length(this);
	return this->payload_length;
}

METHOD2(payload_t, encrypted_payload_t, get_length_plain, size_t,
	private_encrypted_payload_t *this)
{
	/* contains only the decrypted payload data, no IV, padding or ICV */
	this->payload_length = this->encrypted.len;

	if (this->aead)
	{
		this->payload_length += compute_overhead(this->aead,
												 this->payload_length);
	}
	this->payload_length += get_header_length(this);
	return this->payload_length;
}

METHOD(encrypted_payload_t, add_payload, void,
	private_encrypted_payload_t *this, payload_t *payload)
{
	payload_t *last_payload;

	if (this->payloads->get_count(this->payloads) > 0)
	{
		this->payloads->get_last(this->payloads, (void **)&last_payload);
		last_payload->set_next_type(last_payload, payload->get_type(payload));
	}
	else
	{
		this->next_payload = payload->get_type(payload);
	}
	payload->set_next_type(payload, PL_NONE);
	this->payloads->insert_last(this->payloads, payload);
	compute_length(this);
}

METHOD(encrypted_payload_t, remove_payload, payload_t *,
	private_encrypted_payload_t *this)
{
	payload_t *payload;

	if (this->payloads->remove_first(this->payloads,
									 (void**)&payload) == SUCCESS)
	{
		return payload;
	}
	return NULL;
}

/**
 * Generate payload before encryption
 */
static chunk_t generate(private_encrypted_payload_t *this,
						generator_t *generator)
{
	payload_t *current, *next;
	enumerator_t *enumerator;
	uint32_t *lenpos;
	chunk_t chunk = chunk_empty;

	enumerator = this->payloads->create_enumerator(this->payloads);
	if (enumerator->enumerate(enumerator, &current))
	{
		this->next_payload = current->get_type(current);

		while (enumerator->enumerate(enumerator, &next))
		{
			current->set_next_type(current, next->get_type(next));
			generator->generate_payload(generator, current);
			current = next;
		}
		current->set_next_type(current, PL_NONE);
		generator->generate_payload(generator, current);

		chunk = generator->get_chunk(generator, &lenpos);
		DBG2(DBG_ENC, "generated content in encrypted payload");
	}
	enumerator->destroy(enumerator);
	return chunk;
}

METHOD(encrypted_payload_t, generate_payloads, void,
	private_encrypted_payload_t *this, generator_t *generator)
{
	generate(this, generator);
}

/**
 * Append the encrypted payload header to the associated data
 */
static chunk_t append_header(private_encrypted_payload_t *this, chunk_t assoc)
{
	struct {
		uint8_t next_payload;
		uint8_t flags;
		uint16_t length;
	} __attribute__((packed)) header = {
		.next_payload = this->next_payload,
		.flags = this->flags,
		.length = htons(get_length(this)),
	};
	return chunk_cat("cc", assoc, chunk_from_thing(header));
}

/**
 * Encrypts the data in plain and returns it in an allocated chunk.
 */
static status_t encrypt_content(char *label, aead_t *aead, uint64_t mid,
							chunk_t plain, chunk_t assoc, chunk_t *encrypted)
{
	chunk_t iv, padding, icv, crypt;
	iv_gen_t *iv_gen;
	rng_t *rng;
	size_t bs;

	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng)
	{
		DBG1(DBG_ENC, "encrypting %s failed, no RNG found", label);
		return NOT_SUPPORTED;
	}

	iv_gen = aead->get_iv_gen(aead);
	if (!iv_gen)
	{
		DBG1(DBG_ENC, "encrypting %s failed, no IV generator", label);
		return NOT_SUPPORTED;
	}

	bs = aead->get_block_size(aead);
	/* we need at least one byte padding to store the padding length */
	padding.len = bs - (plain.len % bs);
	iv.len = aead->get_iv_size(aead);
	icv.len = aead->get_icv_size(aead);

	/* prepare data to authenticate-encrypt:
	 * | IV | plain | padding | ICV |
	 *       \____crypt______/   ^
	 *              |           /
	 *              v          /
	 *     assoc -> + ------->/
	 */
	*encrypted = chunk_alloc(iv.len + plain.len + padding.len + icv.len);
	iv.ptr = encrypted->ptr;
	memcpy(iv.ptr + iv.len, plain.ptr, plain.len);
	plain.ptr = iv.ptr + iv.len;
	padding.ptr = plain.ptr + plain.len;
	icv.ptr = padding.ptr + padding.len;
	crypt = chunk_create(plain.ptr, plain.len + padding.len);

	if (!iv_gen->get_iv(iv_gen, mid, iv.len, iv.ptr) ||
		!rng->get_bytes(rng, padding.len - 1, padding.ptr))
	{
		DBG1(DBG_ENC, "encrypting %s failed, no IV or padding", label);
		rng->destroy(rng);

		return FAILED;
	}
	padding.ptr[padding.len - 1] = padding.len - 1;
	rng->destroy(rng);

	DBG3(DBG_ENC, "%s encryption:", label);
	DBG3(DBG_ENC, "IV %B", &iv);
	DBG3(DBG_ENC, "plain %B", &plain);
	DBG3(DBG_ENC, "padding %B", &padding);
	DBG3(DBG_ENC, "assoc %B", &assoc);

	if (!aead->encrypt(aead, crypt, assoc, iv, NULL))
	{
		return FAILED;
	}
	DBG3(DBG_ENC, "encrypted %B", &crypt);
	DBG3(DBG_ENC, "ICV %B", &icv);
	return SUCCESS;
}

METHOD(encrypted_payload_t, encrypt, status_t,
	private_encrypted_payload_t *this, uint64_t mid, chunk_t assoc)
{
	generator_t *generator;
	chunk_t plain;
	status_t status;

	if (this->aead == NULL)
	{
		DBG1(DBG_ENC, "encrypting encrypted payload failed, transform missing");
		return INVALID_STATE;
	}

	free(this->encrypted.ptr);
	generator = generator_create();
	plain = generate(this, generator);
	assoc = append_header(this, assoc);
	/* lower 32-bits are for fragment number, if used */
	mid <<= 32;
	status = encrypt_content("encrypted payload", this->aead, mid, plain, assoc,
							 &this->encrypted);
	generator->destroy(generator);
	free(assoc.ptr);
	return status;
}

METHOD(encrypted_payload_t, encrypt_v1, status_t,
	private_encrypted_payload_t *this, uint64_t mid, chunk_t iv)
{
	generator_t *generator;
	chunk_t plain, padding;
	size_t bs;

	if (this->aead == NULL)
	{
		DBG1(DBG_ENC, "encryption failed, transform missing");
		return INVALID_STATE;
	}

	generator = generator_create();
	plain = generate(this, generator);
	bs = this->aead->get_block_size(this->aead);
	padding.len = bs - (plain.len % bs);

	/* prepare data to encrypt:
	 * | plain | padding | */
	free(this->encrypted.ptr);
	this->encrypted = chunk_alloc(plain.len + padding.len);
	memcpy(this->encrypted.ptr, plain.ptr, plain.len);
	plain.ptr = this->encrypted.ptr;
	padding.ptr = plain.ptr + plain.len;
	memset(padding.ptr, 0, padding.len);
	generator->destroy(generator);

	DBG3(DBG_ENC, "encrypting payloads:");
	DBG3(DBG_ENC, "IV %B", &iv);
	DBG3(DBG_ENC, "plain %B", &plain);
	DBG3(DBG_ENC, "padding %B", &padding);

	if (!this->aead->encrypt(this->aead, this->encrypted, chunk_empty, iv, NULL))
	{
		return FAILED;
	}

	DBG3(DBG_ENC, "encrypted %B", &this->encrypted);

	return SUCCESS;
}

/**
 * Parse the payloads after decryption.
 */
static status_t parse(private_encrypted_payload_t *this, chunk_t plain)
{
	parser_t *parser;
	payload_type_t type;

	parser = parser_create(plain);
	parser->set_major_version(parser, this->type == PLV1_ENCRYPTED ? 1 : 2);
	type = this->next_payload;
	while (type != PL_NONE)
	{
		payload_t *payload;

		if (plain.len < 4 || untoh16(plain.ptr + 2) > plain.len)
		{
			DBG1(DBG_ENC, "invalid %N payload length, decryption failed?",
				 payload_type_names, type);
			parser->destroy(parser);
			return PARSE_ERROR;
		}
		if (parser->parse_payload(parser, type, &payload) != SUCCESS)
		{
			parser->destroy(parser);
			return PARSE_ERROR;
		}
		if (payload->verify(payload) != SUCCESS)
		{
			DBG1(DBG_ENC, "%N verification failed",
				 payload_type_names, payload->get_type(payload));
			payload->destroy(payload);
			parser->destroy(parser);
			return VERIFY_ERROR;
		}
		type = payload->get_next_type(payload);
		this->payloads->insert_last(this->payloads, payload);
	}
	parser->destroy(parser);
	DBG2(DBG_ENC, "parsed content of encrypted payload");
	return SUCCESS;
}

/**
 * Decrypts the given data in-place and returns a chunk pointing to the
 * resulting plaintext.
 */
static status_t decrypt_content(char *label, aead_t *aead, chunk_t encrypted,
								chunk_t assoc, chunk_t *plain)
{
	chunk_t iv, padding, icv, crypt;
	size_t bs;

	/* prepare data to authenticate-decrypt:
	 * | IV | plain | padding | ICV |
	 *       \____crypt______/   ^
	 *              |           /
	 *              v          /
	 *     assoc -> + ------->/
	 */
	bs = aead->get_block_size(aead);
	iv.len = aead->get_iv_size(aead);
	iv.ptr = encrypted.ptr;
	icv.len = aead->get_icv_size(aead);
	icv.ptr = encrypted.ptr + encrypted.len - icv.len;
	crypt.ptr = iv.ptr + iv.len;
	crypt.len = encrypted.len - iv.len;

	if (iv.len + icv.len > encrypted.len ||
		(crypt.len - icv.len) % bs)
	{
		DBG1(DBG_ENC, "decrypting %s payload failed, invalid length", label);
		return FAILED;
	}

	DBG3(DBG_ENC, "%s decryption:", label);
	DBG3(DBG_ENC, "IV %B", &iv);
	DBG3(DBG_ENC, "encrypted %B", &crypt);
	DBG3(DBG_ENC, "ICV %B", &icv);
	DBG3(DBG_ENC, "assoc %B", &assoc);

	if (!aead->decrypt(aead, crypt, assoc, iv, NULL))
	{
		DBG1(DBG_ENC, "verifying %s integrity failed", label);
		return FAILED;
	}

	*plain = chunk_create(crypt.ptr, crypt.len - icv.len);
	padding.len = plain->ptr[plain->len - 1] + 1;
	if (padding.len > plain->len)
	{
		DBG1(DBG_ENC, "decrypting %s failed, padding invalid %B", label,
			 &crypt);
		return PARSE_ERROR;
	}
	plain->len -= padding.len;
	padding.ptr = plain->ptr + plain->len;

	DBG3(DBG_ENC, "plain %B", plain);
	DBG3(DBG_ENC, "padding %B", &padding);
	return SUCCESS;
}

METHOD(encrypted_payload_t, decrypt, status_t,
	private_encrypted_payload_t *this, chunk_t assoc)
{
	chunk_t plain;
	status_t status;

	if (this->aead == NULL)
	{
		DBG1(DBG_ENC, "decrypting encrypted payload failed, transform missing");
		return INVALID_STATE;
	}

	assoc = append_header(this, assoc);
	status = decrypt_content("encrypted payload", this->aead, this->encrypted,
							 assoc, &plain);
	free(assoc.ptr);

	if (status != SUCCESS)
	{
		return status;
	}
	return parse(this, plain);
}

METHOD(encrypted_payload_t, decrypt_plain, status_t,
	private_encrypted_payload_t *this, chunk_t assoc)
{
	if (!this->encrypted.ptr)
	{
		return FAILED;
	}
	return parse(this, this->encrypted);
}

METHOD(encrypted_payload_t, decrypt_v1, status_t,
	private_encrypted_payload_t *this, chunk_t iv)
{
	if (this->aead == NULL)
	{
		DBG1(DBG_ENC, "decryption failed, transform missing");
		return INVALID_STATE;
	}

	/* data must be a multiple of block size */
	if (iv.len != this->aead->get_block_size(this->aead) ||
		this->encrypted.len < iv.len || this->encrypted.len % iv.len)
	{
		DBG1(DBG_ENC, "decryption failed, invalid length");
		return FAILED;
	}

	DBG3(DBG_ENC, "decrypting payloads:");
	DBG3(DBG_ENC, "encrypted %B", &this->encrypted);

	if (!this->aead->decrypt(this->aead, this->encrypted, chunk_empty, iv, NULL))
	{
		return FAILED;
	}

	DBG3(DBG_ENC, "plain %B", &this->encrypted);

	return parse(this, this->encrypted);
}

METHOD(encrypted_payload_t, set_transform, void,
	private_encrypted_payload_t *this, aead_t* aead)
{
	this->aead = aead;
}

METHOD(encrypted_payload_t, get_transform, aead_t*,
	private_encrypted_payload_t *this)
{
	return this->aead;
}

METHOD2(payload_t, encrypted_payload_t, destroy, void,
	private_encrypted_payload_t *this)
{
	this->payloads->destroy_offset(this->payloads, offsetof(payload_t, destroy));
	free(this->encrypted.ptr);
	free(this);
}

/*
 * Described in header
 */
encrypted_payload_t *encrypted_payload_create(payload_type_t type)
{
	private_encrypted_payload_t *this;

	INIT(this,
		.public = {
			.payload_interface = {
				.verify = _verify,
				.get_encoding_rules = _get_encoding_rules,
				.get_header_length = _get_header_length,
				.get_length = _get_length,
				.get_next_type = _get_next_type,
				.set_next_type = _set_next_type,
				.get_type = _get_type,
				.destroy = _destroy,
			},
			.get_length = _get_length,
			.add_payload = _add_payload,
			.remove_payload = _remove_payload,
			.generate_payloads = _generate_payloads,
			.set_transform = _set_transform,
			.get_transform = _get_transform,
			.encrypt = _encrypt,
			.decrypt = _decrypt,
			.destroy = _destroy,
		},
		.next_payload = PL_NONE,
		.payloads = linked_list_create(),
		.type = type,
	);
	this->payload_length = get_header_length(this);

	if (type == PLV1_ENCRYPTED)
	{
		this->public.encrypt = _encrypt_v1;
		this->public.decrypt = _decrypt_v1;
	}

	return &this->public;
}

/*
 * Described in header
 */
encrypted_payload_t *encrypted_payload_create_from_plain(payload_type_t next,
														 chunk_t plain)
{
	private_encrypted_payload_t *this;

	this = (private_encrypted_payload_t*)encrypted_payload_create(PLV2_ENCRYPTED);
	this->public.payload_interface.get_length = _get_length_plain;
	this->public.get_length = _get_length_plain;
	this->public.decrypt = _decrypt_plain;
	this->next_payload = next;
	this->encrypted = plain;

	return &this->public;
}

METHOD(payload_t, frag_verify, status_t,
	private_encrypted_fragment_payload_t *this)
{
	if (!this->fragment_number || !this->total_fragments ||
		this->fragment_number > this->total_fragments)
	{
		DBG1(DBG_ENC, "invalid fragment number (%u) or total fragments (%u)",
			 this->fragment_number, this->total_fragments);
		return FAILED;
	}
	if (this->fragment_number > 1 && this->next_payload != 0)
	{
		DBG1(DBG_ENC, "invalid next payload (%u) for fragment %u, ignored",
			 this->next_payload, this->fragment_number);
		this->next_payload = 0;
	}
	return SUCCESS;
}

METHOD(payload_t, frag_get_encoding_rules, int,
	private_encrypted_fragment_payload_t *this, encoding_rule_t **rules)
{
	*rules = encodings_fragment;
	return countof(encodings_fragment);
}

METHOD(payload_t, frag_get_header_length, int,
	private_encrypted_fragment_payload_t *this)
{
	return 8;
}

METHOD(payload_t, frag_get_type, payload_type_t,
	private_encrypted_fragment_payload_t *this)
{
	return PLV2_FRAGMENT;
}

METHOD(payload_t, frag_get_next_type, payload_type_t,
	private_encrypted_fragment_payload_t *this)
{
	return this->next_payload;
}

METHOD(payload_t, frag_set_next_type, void,
	private_encrypted_fragment_payload_t *this, payload_type_t type)
{
	if (this->fragment_number == 1 && this->next_payload == PL_NONE)
	{
		this->next_payload = type;
	}
}

METHOD2(payload_t, encrypted_payload_t, frag_get_length, size_t,
	private_encrypted_fragment_payload_t *this)
{
	if (this->encrypted.len)
	{
		this->payload_length = this->encrypted.len;
	}
	else
	{
		this->payload_length = this->plain.len;

		if (this->aead)
		{
			this->payload_length += compute_overhead(this->aead,
													 this->payload_length);
		}
	}
	this->payload_length += frag_get_header_length(this);
	return this->payload_length;
}

METHOD(encrypted_fragment_payload_t, get_fragment_number, uint16_t,
	private_encrypted_fragment_payload_t *this)
{
	return this->fragment_number;
}

METHOD(encrypted_fragment_payload_t, get_total_fragments, uint16_t,
	private_encrypted_fragment_payload_t *this)
{
	return this->total_fragments;
}

METHOD(encrypted_fragment_payload_t, frag_get_content, chunk_t,
	private_encrypted_fragment_payload_t *this)
{
	return this->plain;
}

METHOD(encrypted_payload_t, frag_add_payload, void,
	private_encrypted_fragment_payload_t *this, payload_t* payload)
{
	payload->destroy(payload);
}

METHOD(encrypted_payload_t, frag_set_transform, void,
	private_encrypted_fragment_payload_t *this, aead_t* aead)
{
	this->aead = aead;
}

METHOD(encrypted_payload_t, frag_get_transform, aead_t*,
	private_encrypted_fragment_payload_t *this)
{
	return this->aead;
}

/**
 * Append the encrypted fragment payload header to the associated data
 */
static chunk_t append_header_frag(private_encrypted_fragment_payload_t *this,
								  chunk_t assoc)
{
	struct {
		uint8_t next_payload;
		uint8_t flags;
		uint16_t length;
		uint16_t fragment_number;
		uint16_t total_fragments;
	} __attribute__((packed)) header = {
		.next_payload = this->next_payload,
		.flags = this->flags,
		.length = htons(frag_get_length(this)),
		.fragment_number = htons(this->fragment_number),
		.total_fragments = htons(this->total_fragments),
	};
	return chunk_cat("cc", assoc, chunk_from_thing(header));
}

METHOD(encrypted_payload_t, frag_encrypt, status_t,
	private_encrypted_fragment_payload_t *this, uint64_t mid, chunk_t assoc)
{
	status_t status;

	if (!this->aead)
	{
		DBG1(DBG_ENC, "encrypting encrypted fragment payload failed, "
			 "transform missing");
		return INVALID_STATE;
	}
	free(this->encrypted.ptr);
	assoc = append_header_frag(this, assoc);
	/* IKEv2 message IDs are not unique if fragmentation is used, hence include
	 * the fragment number to make it unique */
	mid = mid << 32 | this->fragment_number;
	status = encrypt_content("encrypted fragment payload", this->aead, mid,
							 this->plain, assoc, &this->encrypted);
	free(assoc.ptr);
	return status;
}

METHOD(encrypted_payload_t, frag_decrypt, status_t,
	private_encrypted_fragment_payload_t *this, chunk_t assoc)
{
	status_t status;

	if (!this->aead)
	{
		DBG1(DBG_ENC, "decrypting encrypted fragment payload failed, "
			 "transform missing");
		return INVALID_STATE;
	}
	free(this->plain.ptr);
	assoc = append_header_frag(this, assoc);
	status = decrypt_content("encrypted fragment payload", this->aead,
							 this->encrypted, assoc, &this->plain);
	this->plain = chunk_clone(this->plain);
	free(assoc.ptr);
	return status;
}

METHOD2(payload_t, encrypted_payload_t, frag_destroy, void,
	private_encrypted_fragment_payload_t *this)
{
	free(this->encrypted.ptr);
	free(this->plain.ptr);
	free(this);
}

/*
 * Described in header
 */
encrypted_fragment_payload_t *encrypted_fragment_payload_create()
{
	private_encrypted_fragment_payload_t *this;

	INIT(this,
		.public = {
			.encrypted = {
				.payload_interface = {
					.verify = _frag_verify,
					.get_encoding_rules = _frag_get_encoding_rules,
					.get_header_length = _frag_get_header_length,
					.get_length = _frag_get_length,
					.get_next_type = _frag_get_next_type,
					.set_next_type = _frag_set_next_type,
					.get_type = _frag_get_type,
					.destroy = _frag_destroy,
				},
				.get_length = _frag_get_length,
				.add_payload = _frag_add_payload,
				.remove_payload = (void*)return_null,
				.generate_payloads = nop,
				.set_transform = _frag_set_transform,
				.get_transform = _frag_get_transform,
				.encrypt = _frag_encrypt,
				.decrypt = _frag_decrypt,
				.destroy = _frag_destroy,
			},
			.get_fragment_number = _get_fragment_number,
			.get_total_fragments = _get_total_fragments,
			.get_content = _frag_get_content,
		},
		.next_payload = PL_NONE,
	);
	this->payload_length = frag_get_header_length(this);

	return &this->public;
}

/*
 * Described in header
 */
encrypted_fragment_payload_t *encrypted_fragment_payload_create_from_data(
								uint16_t num, uint16_t total, chunk_t plain)
{
	private_encrypted_fragment_payload_t *this;

	this = (private_encrypted_fragment_payload_t*)encrypted_fragment_payload_create();
	this->fragment_number = num;
	this->total_fragments = total;
	this->plain = chunk_clone(plain);

	return &this->public;
}
