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


#include "esp_packet.h"

#include <library.h>
#include <utils/debug.h>
#include <crypto/crypters/crypter.h>
#include <crypto/signers/signer.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

#ifndef WIN32
#include <netinet/in.h>
#endif

typedef struct private_esp_packet_t private_esp_packet_t;

/**
 * Private additions to esp_packet_t.
 */
struct private_esp_packet_t {

	/**
	 * Public members
	 */
	esp_packet_t public;

	/**
	 * Raw ESP packet
	 */
	packet_t *packet;

	/**
	 * Payload of this packet
	 */
	ip_packet_t *payload;

	/**
	 * Next Header info (e.g. IPPROTO_IPIP)
	 */
	uint8_t next_header;

};

/**
 * Forward declaration for clone()
 */
static private_esp_packet_t *esp_packet_create_internal(packet_t *packet);

METHOD(packet_t, set_source, void,
	private_esp_packet_t *this, host_t *src)
{
	return this->packet->set_source(this->packet, src);
}

METHOD2(esp_packet_t, packet_t, get_source, host_t*,
	private_esp_packet_t *this)
{
	return this->packet->get_source(this->packet);
}

METHOD(packet_t, set_destination, void,
	private_esp_packet_t *this, host_t *dst)
{
	return this->packet->set_destination(this->packet, dst);
}

METHOD2(esp_packet_t, packet_t, get_destination, host_t*,
	private_esp_packet_t *this)
{
	return this->packet->get_destination(this->packet);
}

METHOD(packet_t, get_data, chunk_t,
	private_esp_packet_t *this)
{
	return this->packet->get_data(this->packet);
}

METHOD(packet_t, set_data, void,
	private_esp_packet_t *this, chunk_t data)
{
	return this->packet->set_data(this->packet, data);
}

METHOD(packet_t, get_dscp, uint8_t,
	private_esp_packet_t *this)
{
	return this->packet->get_dscp(this->packet);
}

METHOD(packet_t, set_dscp, void,
	private_esp_packet_t *this, uint8_t value)
{
	this->packet->set_dscp(this->packet, value);
}

METHOD(packet_t, skip_bytes, void,
	private_esp_packet_t *this, size_t bytes)
{
	return this->packet->skip_bytes(this->packet, bytes);
}

METHOD(packet_t, clone_, packet_t*,
	private_esp_packet_t *this)
{
	private_esp_packet_t *pkt;

	pkt = esp_packet_create_internal(this->packet->clone(this->packet));
	pkt->payload = this->payload ? this->payload->clone(this->payload) : NULL;
	pkt->next_header = this->next_header;
	return &pkt->public.packet;
}

METHOD(esp_packet_t, parse_header, bool,
	private_esp_packet_t *this, uint32_t *spi)
{
	bio_reader_t *reader;
	uint32_t seq;

	reader = bio_reader_create(this->packet->get_data(this->packet));
	if (!reader->read_uint32(reader, spi) ||
		!reader->read_uint32(reader, &seq))
	{
		DBG1(DBG_ESP, "failed to parse ESP header: invalid length");
		reader->destroy(reader);
		return FALSE;
	}
	reader->destroy(reader);

	DBG2(DBG_ESP, "parsed ESP header with SPI %.8x [seq %u]", *spi, seq);
	*spi = htonl(*spi);
	return TRUE;
}

/**
 * Check padding as specified in RFC 4303
 */
static bool check_padding(chunk_t padding)
{
	size_t i;

	for (i = 0; i < padding.len; ++i)
	{
		if (padding.ptr[i] != (uint8_t)(i + 1))
		{
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Remove the padding from the payload and set the next header info
 */
static bool remove_padding(private_esp_packet_t *this, chunk_t plaintext)
{
	uint8_t next_header, pad_length;
	chunk_t padding, payload;
	bio_reader_t *reader;

	reader = bio_reader_create(plaintext);
	if (!reader->read_uint8_end(reader, &next_header) ||
		!reader->read_uint8_end(reader, &pad_length))
	{
		DBG1(DBG_ESP, "parsing ESP payload failed: invalid length");
		goto failed;
	}
	if (!reader->read_data_end(reader, pad_length, &padding) ||
		!check_padding(padding))
	{
		DBG1(DBG_ESP, "parsing ESP payload failed: invalid padding");
		goto failed;
	}
	this->payload = ip_packet_create(reader->peek(reader));
	reader->destroy(reader);
	if (!this->payload)
	{
		DBG1(DBG_ESP, "parsing ESP payload failed: unsupported payload");
		return FALSE;
	}
	this->next_header = next_header;
	payload = this->payload->get_encoding(this->payload);

	DBG3(DBG_ESP, "ESP payload:\n  payload %B\n  padding %B\n  "
		 "padding length = %hhu, next header = %hhu", &payload, &padding,
		 pad_length, this->next_header);
	return TRUE;

failed:
	reader->destroy(reader);
	chunk_free(&plaintext);
	return FALSE;
}

METHOD(esp_packet_t, decrypt, status_t,
	private_esp_packet_t *this, esp_context_t *esp_context)
{
	bio_reader_t *reader;
	uint32_t spi, seq;
	chunk_t data, iv, icv, aad, ciphertext, plaintext;
	aead_t *aead;

	DESTROY_IF(this->payload);
	this->payload = NULL;

	data = this->packet->get_data(this->packet);
	aead = esp_context->get_aead(esp_context);

	reader = bio_reader_create(data);
	if (!reader->read_uint32(reader, &spi) ||
		!reader->read_uint32(reader, &seq) ||
		!reader->read_data(reader, aead->get_iv_size(aead), &iv) ||
		!reader->read_data_end(reader, aead->get_icv_size(aead), &icv) ||
		reader->remaining(reader) % aead->get_block_size(aead))
	{
		DBG1(DBG_ESP, "ESP decryption failed: invalid length");
		return PARSE_ERROR;
	}
	ciphertext = reader->peek(reader);
	reader->destroy(reader);

	if (!esp_context->verify_seqno(esp_context, seq))
	{
		DBG1(DBG_ESP, "ESP sequence number verification failed:\n  "
			 "src %H, dst %H, SPI %.8x [seq %u]",
			 get_source(this), get_destination(this), spi, seq);
		return VERIFY_ERROR;
	}
	DBG3(DBG_ESP, "ESP decryption:\n  SPI %.8x [seq %u]\n  IV %B\n  "
		 "encrypted %B\n  ICV %B", spi, seq, &iv, &ciphertext, &icv);

	/* include ICV in ciphertext for decryption/verification */
	ciphertext.len += icv.len;
	/* aad = spi + seq */
	aad = chunk_create(data.ptr, 8);

	if (!aead->decrypt(aead, ciphertext, aad, iv, &plaintext))
	{
		DBG1(DBG_ESP, "ESP decryption or ICV verification failed");
		return FAILED;
	}
	esp_context->set_authenticated_seqno(esp_context, seq);

	if (!remove_padding(this, plaintext))
	{
		return PARSE_ERROR;
	}
	return SUCCESS;
}

/**
 * Generate the padding as specified in RFC4303
 */
static void generate_padding(chunk_t padding)
{
	size_t i;

	for (i = 0; i < padding.len; ++i)
	{
		padding.ptr[i] = (uint8_t)(i + 1);
	}
}

METHOD(esp_packet_t, encrypt, status_t,
	private_esp_packet_t *this, esp_context_t *esp_context, uint32_t spi)
{
	chunk_t iv, icv, aad, padding, payload, ciphertext;
	bio_writer_t *writer;
	uint32_t next_seqno;
	size_t blocksize, plainlen;
	aead_t *aead;
	iv_gen_t *iv_gen;

	this->packet->set_data(this->packet, chunk_empty);

	if (!esp_context->next_seqno(esp_context, &next_seqno))
	{
		DBG1(DBG_ESP, "ESP encapsulation failed: sequence numbers cycled");
		return FAILED;
	}

	aead = esp_context->get_aead(esp_context);
	iv_gen = aead->get_iv_gen(aead);
	if (!iv_gen)
	{
		DBG1(DBG_ESP, "ESP encryption failed: no IV generator");
		return NOT_FOUND;
	}

	blocksize = aead->get_block_size(aead);
	iv.len = aead->get_iv_size(aead);
	icv.len = aead->get_icv_size(aead);

	/* plaintext = payload, padding, pad_length, next_header */
	payload = this->payload ? this->payload->get_encoding(this->payload)
							: chunk_empty;
	plainlen = payload.len + 2;
	padding.len = pad_len(plainlen, blocksize);
	/* ICV must be on a 4-byte boundary */
	padding.len += pad_len(iv.len + plainlen + padding.len, 4);
	plainlen += padding.len;

	/* len = spi, seq, IV, plaintext, ICV */
	writer = bio_writer_create(2 * sizeof(uint32_t) + iv.len + plainlen +
							   icv.len);
	writer->write_uint32(writer, ntohl(spi));
	writer->write_uint32(writer, next_seqno);

	iv = writer->skip(writer, iv.len);
	if (!iv_gen->get_iv(iv_gen, next_seqno, iv.len, iv.ptr))
	{
		DBG1(DBG_ESP, "ESP encryption failed: could not generate IV");
		writer->destroy(writer);
		return FAILED;
	}

	/* plain-/ciphertext will start here */
	ciphertext = writer->get_buf(writer);
	ciphertext.ptr += ciphertext.len;
	ciphertext.len = plainlen;

	writer->write_data(writer, payload);

	padding = writer->skip(writer, padding.len);
	generate_padding(padding);

	writer->write_uint8(writer, padding.len);
	writer->write_uint8(writer, this->next_header);

	/* aad = spi + seq */
	aad = writer->get_buf(writer);
	aad.len = 8;
	icv = writer->skip(writer, icv.len);

	DBG3(DBG_ESP, "ESP before encryption:\n  payload = %B\n  padding = %B\n  "
		 "padding length = %hhu, next header = %hhu", &payload, &padding,
		 (uint8_t)padding.len, this->next_header);

	/* encrypt/authenticate the content inline */
	if (!aead->encrypt(aead, ciphertext, aad, iv, NULL))
	{
		DBG1(DBG_ESP, "ESP encryption or ICV generation failed");
		writer->destroy(writer);
		return FAILED;
	}

	DBG3(DBG_ESP, "ESP packet:\n  SPI %.8x [seq %u]\n  IV %B\n  "
		 "encrypted %B\n  ICV %B", ntohl(spi), next_seqno, &iv,
		 &ciphertext, &icv);

	this->packet->set_data(this->packet, writer->extract_buf(writer));
	writer->destroy(writer);
	return SUCCESS;
}

METHOD(esp_packet_t, get_next_header, uint8_t,
	private_esp_packet_t *this)
{
	return this->next_header;
}

METHOD(esp_packet_t, get_payload, ip_packet_t*,
	private_esp_packet_t *this)
{
	return this->payload;
}

METHOD(esp_packet_t, extract_payload, ip_packet_t*,
	private_esp_packet_t *this)
{
	ip_packet_t *payload;

	payload = this->payload;
	this->payload = NULL;
	return payload;
}

METHOD2(esp_packet_t, packet_t, destroy, void,
	private_esp_packet_t *this)
{
	DESTROY_IF(this->payload);
	this->packet->destroy(this->packet);
	free(this);
}

static private_esp_packet_t *esp_packet_create_internal(packet_t *packet)
{
	private_esp_packet_t *this;

	INIT(this,
		.public = {
			.packet = {
				.set_source = _set_source,
				.get_source = _get_source,
				.set_destination = _set_destination,
				.get_destination = _get_destination,
				.get_data = _get_data,
				.set_data = _set_data,
				.get_dscp = _get_dscp,
				.set_dscp = _set_dscp,
				.skip_bytes = _skip_bytes,
				.clone = _clone_,
				.destroy = _destroy,
			},
			.get_source = _get_source,
			.get_destination = _get_destination,
			.get_next_header = _get_next_header,
			.parse_header = _parse_header,
			.decrypt = _decrypt,
			.encrypt = _encrypt,
			.get_payload = _get_payload,
			.extract_payload = _extract_payload,
			.destroy = _destroy,
		},
		.packet = packet,
		.next_header = IPPROTO_NONE,
	);
	return this;
}

/**
 * Described in header.
 */
esp_packet_t *esp_packet_create_from_packet(packet_t *packet)
{
	private_esp_packet_t *this;

	this = esp_packet_create_internal(packet);

	return &this->public;
}

/**
 * Described in header.
 */
esp_packet_t *esp_packet_create_from_payload(host_t *src, host_t *dst,
											 ip_packet_t *payload)
{
	private_esp_packet_t *this;
	packet_t *packet;

	packet = packet_create_from_data(src, dst, chunk_empty);
	this = esp_packet_create_internal(packet);
	this->payload = payload;
	if (payload)
	{
		this->next_header = payload->get_version(payload) == 4 ? IPPROTO_IPIP
															   : IPPROTO_IPV6;
	}
	else
	{
		this->next_header = IPPROTO_NONE;
	}
	return &this->public;
}
