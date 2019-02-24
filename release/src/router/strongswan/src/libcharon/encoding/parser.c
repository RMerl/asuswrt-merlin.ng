/*
 * Copyright (C) 2005-2009 Martin Willi
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

#include <stdlib.h>
#include <string.h>

#include "parser.h"

#include <library.h>
#include <daemon.h>
#include <collections/linked_list.h>
#include <encoding/payloads/encodings.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/proposal_substructure.h>
#include <encoding/payloads/transform_substructure.h>
#include <encoding/payloads/transform_attribute.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/id_payload.h>
#include <encoding/payloads/notify_payload.h>
#include <encoding/payloads/encrypted_payload.h>
#include <encoding/payloads/auth_payload.h>
#include <encoding/payloads/cert_payload.h>
#include <encoding/payloads/certreq_payload.h>
#include <encoding/payloads/ts_payload.h>
#include <encoding/payloads/delete_payload.h>
#include <encoding/payloads/vendor_id_payload.h>
#include <encoding/payloads/cp_payload.h>
#include <encoding/payloads/configuration_attribute.h>
#include <encoding/payloads/eap_payload.h>
#include <encoding/payloads/unknown_payload.h>


typedef struct private_parser_t private_parser_t;

/**
 * Private data stored in a context.
 *
 * Contains pointers and counters to store current state.
 */
struct private_parser_t {
	/**
	 * Public members, see parser_t.
	 */
	parser_t public;

	/**
	 * major IKE version
	 */
	uint8_t major_version;

	/**
	 * Current bit for reading in input data.
	 */
	uint8_t bit_pos;

	/**
	 * Current byte for reading in input data.
	 */
	uint8_t *byte_pos;

	/**
	 * Input data to parse.
	 */
	uint8_t *input;

	/**
	 * Roof of input, used for length-checking.
	 */
	uint8_t *input_roof;

	/**
	 * Set of encoding rules for this parsing session.
	 */
	encoding_rule_t *rules;
};

/**
 * Log invalid length error
 */
static bool short_input(private_parser_t *this, int number)
{
	DBG1(DBG_ENC, "  not enough input to parse rule %d %N",
		 number, encoding_type_names, this->rules[number].type);
	return FALSE;
}

/**
 * Log unaligned rules
 */
static bool bad_bitpos(private_parser_t *this, int number)
{
	DBG1(DBG_ENC, "  found rule %d %N on bitpos %d",
		 number, encoding_type_names, this->rules[number].type, this->bit_pos);
	return FALSE;
}

/**
 * Parse a 4-Bit unsigned integer from the current parsing position.
 */
static bool parse_uint4(private_parser_t *this, int rule_number,
						uint8_t *output_pos)
{
	if (this->byte_pos + sizeof(uint8_t) > this->input_roof)
	{
		return short_input(this, rule_number);
	}
	switch (this->bit_pos)
	{
		case 0:
			if (output_pos)
			{
				*output_pos = *(this->byte_pos) >> 4;
			}
			this->bit_pos = 4;
			break;
		case 4:
			if (output_pos)
			{
				*output_pos = *(this->byte_pos) & 0x0F;
			}
			this->bit_pos = 0;
			this->byte_pos++;
			break;
		default:
			return bad_bitpos(this, rule_number);
	}
	if (output_pos)
	{
		DBG3(DBG_ENC, "   => %hhu", *output_pos);
	}
	return TRUE;
}

/**
 * Parse a 8-Bit unsigned integer from the current parsing position.
 */
static bool parse_uint8(private_parser_t *this, int rule_number,
						uint8_t *output_pos)
{
	if (this->byte_pos + sizeof(uint8_t) > this->input_roof)
	{
		return short_input(this, rule_number);
	}
	if (this->bit_pos)
	{
		return bad_bitpos(this, rule_number);
	}
	if (output_pos)
	{
		*output_pos = *(this->byte_pos);
		DBG3(DBG_ENC, "   => %hhu", *output_pos);
	}
	this->byte_pos++;
	return TRUE;
}

/**
 * Parse a 15-Bit unsigned integer from the current parsing position.
 */
static bool parse_uint15(private_parser_t *this, int rule_number,
						 uint16_t *output_pos)
{
	if (this->byte_pos + sizeof(uint16_t) > this->input_roof)
	{
		return short_input(this, rule_number);
	}
	if (this->bit_pos != 1)
	{
		return bad_bitpos(this, rule_number);
	}
	if (output_pos)
	{
		memcpy(output_pos, this->byte_pos, sizeof(uint16_t));
		*output_pos = ntohs(*output_pos) & ~0x8000;
		DBG3(DBG_ENC, "   => %hu", *output_pos);
	}
	this->byte_pos += sizeof(uint16_t);
	this->bit_pos = 0;
	return TRUE;
}

/**
 * Parse a 16-Bit unsigned integer from the current parsing position.
 */
static bool parse_uint16(private_parser_t *this, int rule_number,
						 uint16_t *output_pos)
{
	if (this->byte_pos + sizeof(uint16_t) > this->input_roof)
	{
		return short_input(this, rule_number);
	}
	if (this->bit_pos)
	{
		return bad_bitpos(this, rule_number);
	}
	if (output_pos)
	{
		memcpy(output_pos, this->byte_pos, sizeof(uint16_t));
		*output_pos = ntohs(*output_pos);
		DBG3(DBG_ENC, "   => %hu", *output_pos);
	}
	this->byte_pos += sizeof(uint16_t);
	return TRUE;
}
/**
 * Parse a 32-Bit unsigned integer from the current parsing position.
 */
static bool parse_uint32(private_parser_t *this, int rule_number,
						 uint32_t *output_pos)
{
	if (this->byte_pos + sizeof(uint32_t) > this->input_roof)
	{
		return short_input(this, rule_number);
	}
	if (this->bit_pos)
	{
		return bad_bitpos(this, rule_number);
	}
	if (output_pos)
	{
		memcpy(output_pos, this->byte_pos, sizeof(uint32_t));
		*output_pos = ntohl(*output_pos);
		DBG3(DBG_ENC, "   => %u", *output_pos);
	}
	this->byte_pos += sizeof(uint32_t);
	return TRUE;
}

/**
 * Parse a given amount of bytes and writes them to a specific location
 */
static bool parse_bytes(private_parser_t *this, int rule_number,
						uint8_t *output_pos, int bytes)
{
	if (this->byte_pos + bytes > this->input_roof)
	{
		return short_input(this, rule_number);
	}
	if (this->bit_pos)
	{
		return bad_bitpos(this, rule_number);
	}
	if (output_pos)
	{
		memcpy(output_pos, this->byte_pos, bytes);
		DBG3(DBG_ENC, "   %b", output_pos, bytes);
	}
	this->byte_pos += bytes;
	return TRUE;
}

/**
 * Parse a single Bit from the current parsing position
 */
static bool parse_bit(private_parser_t *this, int rule_number,
					  bool *output_pos)
{
	if (this->byte_pos + sizeof(uint8_t) > this->input_roof)
	{
		return short_input(this, rule_number);
	}
	if (output_pos)
	{
		uint8_t mask;
		mask = 0x01 << (7 - this->bit_pos);
		*output_pos = *this->byte_pos & mask;

		if (*output_pos)
		{	/* set to a "clean", comparable true */
			*output_pos = TRUE;
		}
		DBG3(DBG_ENC, "   => %d", *output_pos);
	}
	this->bit_pos = (this->bit_pos + 1) % 8;
	if (this->bit_pos == 0)
	{
		this->byte_pos++;
	}
	return TRUE;
}

/**
 * Parse substructures in a list.
 */
static bool parse_list(private_parser_t *this, int rule_number,
			linked_list_t **output_pos, payload_type_t payload_type, int length)
{
	linked_list_t *list = *output_pos;

	if (length < 0)
	{
		return short_input(this, rule_number);
	}
	if (this->bit_pos)
	{
		return bad_bitpos(this, rule_number);
	}
	while (length > 0)
	{
		uint8_t *pos_before = this->byte_pos;
		payload_t *payload;

		DBG2(DBG_ENC, "  %d bytes left, parsing recursively %N",
			 length, payload_type_names, payload_type);

		if (this->public.parse_payload(&this->public, payload_type,
									   &payload) != SUCCESS)
		{
			DBG1(DBG_ENC, "  parsing of a %N substructure failed",
				 payload_type_names, payload_type);
			return FALSE;
		}
		list->insert_last(list, payload);
		length -= this->byte_pos - pos_before;
	}
	if (length != 0)
	{	/* must yield exactly to zero */
		DBG1(DBG_ENC, "  length of %N substructure list invalid",
			 payload_type_names, payload_type);
		return FALSE;
	}
	*output_pos = list;
	return TRUE;
}

/**
 * Parse data from current parsing position in a chunk.
 */
static bool parse_chunk(private_parser_t *this, int rule_number,
						chunk_t *output_pos, int length)
{
	if (this->byte_pos + length > this->input_roof)
	{
		return short_input(this, rule_number);
	}
	if (this->bit_pos)
	{
		return bad_bitpos(this, rule_number);
	}
	if (output_pos)
	{
		*output_pos = chunk_alloc(length);
		memcpy(output_pos->ptr, this->byte_pos, length);
		DBG3(DBG_ENC, "   %b", output_pos->ptr, length);
	}
	this->byte_pos += length;
	return TRUE;
}

METHOD(parser_t, parse_payload, status_t,
	private_parser_t *this, payload_type_t payload_type, payload_t **payload)
{
	payload_t *pld;
	void *output;
	int payload_length = 0, spi_size = 0, attribute_length = 0, header_length;
	uint16_t ts_type = 0;
	bool attribute_format = FALSE;
	int rule_number, rule_count;
	encoding_rule_t *rule;

	/* create instance of the payload to parse */
	if (payload_is_known(payload_type, this->major_version))
	{
		pld = payload_create(payload_type);
	}
	else
	{
		pld = (payload_t*)unknown_payload_create(payload_type);
	}

	DBG2(DBG_ENC, "parsing %N payload, %d bytes left",
		 payload_type_names, payload_type, this->input_roof - this->byte_pos);

	DBG3(DBG_ENC, "parsing payload from %b",
		 this->byte_pos, (u_int)(this->input_roof - this->byte_pos));

	/* base pointer for output, avoids casting in every rule */
	output = pld;
	/* parse the payload with its own rulse */
	rule_count = pld->get_encoding_rules(pld, &this->rules);
	for (rule_number = 0; rule_number < rule_count; rule_number++)
	{
		/* update header length for each rule, as it is dynamic (SPIs) */
		header_length = pld->get_header_length(pld);

		rule = &(this->rules[rule_number]);
		DBG2(DBG_ENC, "  parsing rule %d %N",
			 rule_number, encoding_type_names, rule->type);
		switch ((int)rule->type)
		{
			case U_INT_4:
			{
				if (!parse_uint4(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case U_INT_8:
			case RESERVED_BYTE:
			{
				if (!parse_uint8(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case U_INT_16:
			{
				if (!parse_uint16(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case U_INT_32:
			case HEADER_LENGTH:
			{
				if (!parse_uint32(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case IKE_SPI:
			{
				if (!parse_bytes(this, rule_number, output + rule->offset, 8))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case RESERVED_BIT:
			case FLAG:
			{
				if (!parse_bit(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case PAYLOAD_LENGTH:
			{
				if (!parse_uint16(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				/* parsed u_int16 should be aligned */
				payload_length = *(uint16_t*)(output + rule->offset);
				/* all payloads must have at least 4 bytes header */
				if (payload_length < 4)
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case SPI_SIZE:
			{
				if (!parse_uint8(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				spi_size = *(uint8_t*)(output + rule->offset);
				break;
			}
			case SPI:
			{
				if (!parse_chunk(this, rule_number, output + rule->offset,
								 spi_size))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case PAYLOAD_LIST + PLV2_PROPOSAL_SUBSTRUCTURE:
			case PAYLOAD_LIST + PLV1_PROPOSAL_SUBSTRUCTURE:
			case PAYLOAD_LIST + PLV2_TRANSFORM_SUBSTRUCTURE:
			case PAYLOAD_LIST + PLV1_TRANSFORM_SUBSTRUCTURE:
			case PAYLOAD_LIST + PLV2_TRANSFORM_ATTRIBUTE:
			case PAYLOAD_LIST + PLV1_TRANSFORM_ATTRIBUTE:
			case PAYLOAD_LIST + PLV2_CONFIGURATION_ATTRIBUTE:
			case PAYLOAD_LIST + PLV1_CONFIGURATION_ATTRIBUTE:
			case PAYLOAD_LIST + PLV2_TRAFFIC_SELECTOR_SUBSTRUCTURE:
			{
				if (payload_length < header_length ||
					!parse_list(this, rule_number, output + rule->offset,
								rule->type - PAYLOAD_LIST,
								payload_length - header_length))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case CHUNK_DATA:
			{
				if (payload_length < header_length ||
					!parse_chunk(this, rule_number, output + rule->offset,
								 payload_length - header_length))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case ENCRYPTED_DATA:
			{
				if (!parse_chunk(this, rule_number, output + rule->offset,
								 this->input_roof - this->byte_pos))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case ATTRIBUTE_FORMAT:
			{
				if (!parse_bit(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				attribute_format = *(bool*)(output + rule->offset);
				break;
			}
			case ATTRIBUTE_TYPE:
			{
				if (!parse_uint15(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case ATTRIBUTE_LENGTH:
			{
				if (!parse_uint16(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				attribute_length = *(uint16_t*)(output + rule->offset);
				break;
			}
			case ATTRIBUTE_LENGTH_OR_VALUE:
			{
				if (!parse_uint16(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				attribute_length = *(uint16_t*)(output + rule->offset);
				break;
			}
			case ATTRIBUTE_VALUE:
			{
				if (attribute_format == FALSE &&
					!parse_chunk(this, rule_number, output + rule->offset,
								 attribute_length))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			case TS_TYPE:
			{
				if (!parse_uint8(this, rule_number, output + rule->offset))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				ts_type = *(uint8_t*)(output + rule->offset);
				break;
			}
			case ADDRESS:
			{
				int address_length = (ts_type == TS_IPV4_ADDR_RANGE) ? 4 : 16;

				if (!parse_chunk(this, rule_number, output + rule->offset,
								 address_length))
				{
					pld->destroy(pld);
					return PARSE_ERROR;
				}
				break;
			}
			default:
			{
				DBG1(DBG_ENC, "  no rule to parse rule %d %N",
					 rule_number, encoding_type_names, rule->type);
				pld->destroy(pld);
				return PARSE_ERROR;
			}
		}
		/* process next rulue */
		rule++;
	}

	*payload = pld;
	DBG2(DBG_ENC, "parsing %N payload finished",
		 payload_type_names, payload_type);
	return SUCCESS;
}

METHOD(parser_t, get_remaining_byte_count, int,
	private_parser_t *this)
{
	return this->input_roof - this->byte_pos;
}

METHOD(parser_t, reset_context, void,
	private_parser_t *this)
{
	this->byte_pos = this->input;
	this->bit_pos = 0;
}

METHOD(parser_t, set_major_version, void,
	private_parser_t *this, uint8_t major_version)
{
	this->major_version = major_version;
}

METHOD(parser_t, destroy, void,
	private_parser_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
parser_t *parser_create(chunk_t data)
{
	private_parser_t *this;

	INIT(this,
		.public = {
			.parse_payload = _parse_payload,
			.reset_context = _reset_context,
			.set_major_version = _set_major_version,
			.get_remaining_byte_count = _get_remaining_byte_count,
			.destroy = _destroy,
		},
		.input = data.ptr,
		.byte_pos = data.ptr,
		.input_roof = data.ptr + data.len,
	);

	return &this->public;
}

