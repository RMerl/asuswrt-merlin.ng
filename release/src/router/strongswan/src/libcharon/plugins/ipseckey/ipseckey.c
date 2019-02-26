/*
 * Copyright (C) 2012 Reto Guadagnini
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

#include "ipseckey.h"

#include <library.h>
#include <utils/debug.h>
#include <bio/bio_reader.h>

typedef struct private_ipseckey_t private_ipseckey_t;

/**
* private data of the ipseckey
*/
struct private_ipseckey_t {

	/**
	 * public functions
	 */
	ipseckey_t public;

	/**
	 * Precedence
	 */
	uint8_t precedence;

	/**
	 * Gateway type
	 */
	uint8_t gateway_type;

	/**
	 * Algorithm
	 */
	uint8_t algorithm;

	/**
	 * Gateway
	 */
	chunk_t gateway;

	/**
	 * Public key
	 */
	chunk_t public_key;
};

METHOD(ipseckey_t, get_precedence, uint8_t,
	private_ipseckey_t *this)
{
	return this->precedence;
}

METHOD(ipseckey_t, get_gateway_type, ipseckey_gw_type_t,
	private_ipseckey_t *this)
{
	return this->gateway_type;
}

METHOD(ipseckey_t, get_algorithm, ipseckey_algorithm_t,
	private_ipseckey_t *this)
{
	return this->algorithm;
}

METHOD(ipseckey_t, get_gateway, chunk_t,
	private_ipseckey_t *this)
{
	return this->gateway;
}

METHOD(ipseckey_t, get_public_key, chunk_t,
	private_ipseckey_t *this)
{
	return this->public_key;
}

METHOD(ipseckey_t, destroy, void,
	private_ipseckey_t *this)
{
	chunk_free(&this->gateway);
	chunk_free(&this->public_key);
	free(this);
}

/*
 * See header
 */
ipseckey_t *ipseckey_create_frm_rr(rr_t *rr)
{
	private_ipseckey_t *this;
	bio_reader_t *reader = NULL;
	uint8_t label;
	chunk_t tmp;

	INIT(this,
			.public = {
				.get_precedence = _get_precedence,
				.get_gateway_type = _get_gateway_type,
				.get_algorithm = _get_algorithm,
				.get_gateway = _get_gateway,
				.get_public_key = _get_public_key,
				.destroy = _destroy,
			},
	);

	if (rr->get_type(rr) != RR_TYPE_IPSECKEY)
	{
		DBG1(DBG_CFG, "unable to create an ipseckey out of an RR "
					  "whose type is not IPSECKEY");
		free(this);
		return NULL;
	}

	/** Parse the content (RDATA field) of the RR */
	reader = bio_reader_create(rr->get_rdata(rr));
	if (!reader->read_uint8(reader, &this->precedence) ||
		!reader->read_uint8(reader, &this->gateway_type) ||
		!reader->read_uint8(reader, &this->algorithm))
	{
		DBG1(DBG_CFG, "ipseckey RR has a wrong format");
		reader->destroy(reader);
		free(this);
		return NULL;
	}

	switch (this->gateway_type)
	{
		case IPSECKEY_GW_TP_NOT_PRESENT:
			break;

		case IPSECKEY_GW_TP_IPV4:
			if (!reader->read_data(reader, 4, &this->gateway))
			{
				DBG1(DBG_CFG, "ipseckey gateway field does not contain an "
							  "IPv4 address as expected");
				reader->destroy(reader);
				free(this);
				return NULL;
			}
			this->gateway = chunk_clone(this->gateway);
			break;

		case IPSECKEY_GW_TP_IPV6:
			if (!reader->read_data(reader, 16, &this->gateway))
			{
				DBG1(DBG_CFG, "ipseckey gateway field does not contain an "
							  "IPv6 address as expected");
				reader->destroy(reader);
				free(this);
				return NULL;
			}
			this->gateway = chunk_clone(this->gateway);
			break;

		case IPSECKEY_GW_TP_WR_ENC_DNAME:
			/**
			 * Uncompressed domain name as defined in RFC 1035 chapter 3.
			 *
			 * TODO: Currently we ignore wire encoded domain names.
			 *
			 */
			while (reader->read_uint8(reader, &label) &&
				   label != 0 && label < 192)
			{
				if (!reader->read_data(reader, label, &tmp))
				{
					DBG1(DBG_CFG, "wrong wire encoded domain name format "
								  "in ipseckey gateway field");
					reader->destroy(reader);
					free(this);
					return NULL;
				}
			}
			break;

		default:
			DBG1(DBG_CFG, "unable to parse ipseckey gateway field");
			reader->destroy(reader);
			free(this);
			return NULL;
	}

	if (!reader->read_data(reader, reader->remaining(reader),
						   &this->public_key))
	{
		DBG1(DBG_CFG, "failed to read ipseckey public key field");
		reader->destroy(reader);
		chunk_free(&this->gateway);
		free(this);
		return NULL;
	}
	this->public_key = chunk_clone(this->public_key);
	reader->destroy(reader);
	return &this->public;
}

