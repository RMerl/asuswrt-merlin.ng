/*
 * Copyright (C) 2011 Tobias Brunner
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
#include <stdio.h>

#include "generator.h"

#include <library.h>
#include <daemon.h>
#include <collections/linked_list.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/proposal_substructure.h>
#include <encoding/payloads/transform_substructure.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/notify_payload.h>
#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/id_payload.h>
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

/**
 * Generating is done in a data buffer.
 * This is the start size of this buffer in bytes.
 */
#define GENERATOR_DATA_BUFFER_SIZE 500

/**
 * Number of bytes to increase the buffer, if it is too small.
 */
#define GENERATOR_DATA_BUFFER_INCREASE_VALUE 500

typedef struct private_generator_t private_generator_t;

/**
 * Private part of a generator_t object.
 */
struct private_generator_t {
	/**
	 * Public part of a generator_t object.
	 */
	 generator_t public;

	/**
	 * Buffer used to generate the data into.
	 */
	uint8_t *buffer;

	/**
	 * Current write position in buffer (one byte aligned).
	 */
	uint8_t *out_position;

	/**
	 * Position of last byte in buffer.
	 */
	uint8_t *roof_position;

	/**
	 * Current bit writing to in current byte (between 0 and 7).
	 */
	uint8_t current_bit;

	/**
	 * Associated data struct to read information from.
	 */
	void *data_struct;

	/**
	 * Offset of the header length field in the buffer.
	 */
	uint32_t header_length_offset;

	/**
	 * Attribute format of the last generated transform attribute.
	 *
	 * Used to check if a variable value field is used or not for
	 * the transform attribute value.
	 */
	bool attribute_format;

	/**
	 * Depending on the value of attribute_format this field is used
	 * to hold the length of the transform attribute in bytes.
	 */
	uint16_t attribute_length;

	/**
	 * TRUE, if debug messages should be logged during generation.
	 */
	bool debug;
};

/**
 * Get size of current buffer in bytes.
 */
static int get_size(private_generator_t *this)
{
	return this->roof_position - this->buffer;
}

/**
 * Get free space of current buffer in bytes.
 */
static int get_space(private_generator_t *this)
{
	return this->roof_position - this->out_position;
}

/**
 * Get length of data in buffer (in bytes).
 */
static int get_length(private_generator_t *this)
{
	return this->out_position - this->buffer;
}

/**
 * Get current offset in buffer (in bytes).
 */
static uint32_t get_offset(private_generator_t *this)
{
	return this->out_position - this->buffer;
}

/**
 * Makes sure enough space is available in buffer to store amount of bits.
 */
static void make_space_available(private_generator_t *this, int bits)
{
	while ((get_space(this) * 8 - this->current_bit) < bits)
	{
		int old_buffer_size, new_buffer_size, out_position_offset;

		old_buffer_size = get_size(this);
		new_buffer_size = old_buffer_size + GENERATOR_DATA_BUFFER_INCREASE_VALUE;
		out_position_offset = this->out_position - this->buffer;

		if (this->debug)
		{
			DBG2(DBG_ENC, "increasing gen buffer from %d to %d byte",
				 old_buffer_size, new_buffer_size);
		}

		this->buffer = realloc(this->buffer,new_buffer_size);
		this->out_position = (this->buffer + out_position_offset);
		this->roof_position = (this->buffer + new_buffer_size);
	}
}

/**
 * Writes a specific amount of byte into the buffer.
 */
static void write_bytes_to_buffer(private_generator_t *this, void *bytes,
								  int number_of_bytes)
{
	int i;
	uint8_t *read_position = (uint8_t *)bytes;

	make_space_available(this, number_of_bytes * 8);

	for (i = 0; i < number_of_bytes; i++)
	{
		*(this->out_position) = *(read_position);
		read_position++;
		this->out_position++;
	}
}

/**
 * Generates a U_INT-Field type and writes it to buffer.
 */
static void generate_u_int_type(private_generator_t *this,
								encoding_type_t int_type,uint32_t offset)
{
	int number_of_bits = 0;

	/* find out number of bits of each U_INT type to check for enough space */
	switch (int_type)
	{
		case U_INT_4:
			number_of_bits = 4;
			break;
		case TS_TYPE:
		case RESERVED_BYTE:
		case SPI_SIZE:
		case U_INT_8:
			number_of_bits = 8;
			break;
		case U_INT_16:
		case PAYLOAD_LENGTH:
		case ATTRIBUTE_LENGTH:
			number_of_bits = 16;
			break;
		case U_INT_32:
			number_of_bits = 32;
			break;
		case ATTRIBUTE_TYPE:
			number_of_bits = 15;
			break;
		case IKE_SPI:
			number_of_bits = 64;
			break;
		default:
			DBG1(DBG_ENC, "U_INT Type %N is not supported",
				 encoding_type_names, int_type);
			return;
	}
	if ((number_of_bits % 8) == 0 && this->current_bit != 0)
	{
		DBG1(DBG_ENC, "U_INT Type %N is not 8 Bit aligned",
			 encoding_type_names, int_type);
		return;
	}

	make_space_available(this, number_of_bits);
	switch (int_type)
	{
		case U_INT_4:
		{
			uint8_t high, low;

			if (this->current_bit == 0)
			{
				/* high of current byte in buffer has to be set to the new value*/
				high = *((uint8_t *)(this->data_struct + offset)) << 4;
				/* low in buffer is not changed */
				low = *(this->out_position) & 0x0F;
				/* high is set, low_val is not changed */
				*(this->out_position) = high | low;
				if (this->debug)
				{
					DBG3(DBG_ENC, "   => %d", *(this->out_position));
				}
				/* write position is not changed, just bit position is moved */
				this->current_bit = 4;
			}
			else if (this->current_bit == 4)
			{
				/* high in buffer is not changed */
				high = *(this->out_position) & 0xF0;
				/* low of current byte in buffer has to be set to the new value*/
				low = *((uint8_t *)(this->data_struct + offset)) & 0x0F;
				*(this->out_position) = high | low;
				if (this->debug)
				{
					DBG3(DBG_ENC, "   => %d", *(this->out_position));
				}
				this->out_position++;
				this->current_bit = 0;
			}
			else
			{
				DBG1(DBG_ENC, "U_INT_4 Type is not 4 Bit aligned");
				/* 4 Bit integers must have a 4 bit alignment */
				return;
			}
			break;
		}
		case TS_TYPE:
		case RESERVED_BYTE:
		case SPI_SIZE:
		case U_INT_8:
		{
			/* 8 bit values are written as they are */
			*this->out_position = *((uint8_t *)(this->data_struct + offset));
			if (this->debug)
			{
				DBG3(DBG_ENC, "   => %d", *(this->out_position));
			}
			this->out_position++;
			break;
		}
		case ATTRIBUTE_TYPE:
		{
			uint8_t attribute_format_flag;
			uint16_t val;

			/* attribute type must not change first bit of current byte */
			if (this->current_bit != 1)
			{
				DBG1(DBG_ENC, "ATTRIBUTE FORMAT flag is not set");
				return;
			}
			attribute_format_flag = *(this->out_position) & 0x80;
			/* get attribute type value as 16 bit integer*/
			val = *((uint16_t*)(this->data_struct + offset));
			/* unset most significant bit */
			val &= 0x7FFF;
			if (attribute_format_flag)
			{
				val |= 0x8000;
			}
			val = htons(val);
			if (this->debug)
			{
				DBG3(DBG_ENC, "   => %d", val);
			}
			/* write bytes to buffer (set bit is overwritten) */
			write_bytes_to_buffer(this, &val, sizeof(uint16_t));
			this->current_bit = 0;
			break;

		}
		case U_INT_16:
		case PAYLOAD_LENGTH:
		case ATTRIBUTE_LENGTH:
		{
			uint16_t val = htons(*((uint16_t*)(this->data_struct + offset)));
			if (this->debug)
			{
				DBG3(DBG_ENC, "   %b", &val, sizeof(uint16_t));
			}
			write_bytes_to_buffer(this, &val, sizeof(uint16_t));
			break;
		}
		case U_INT_32:
		{
			uint32_t val = htonl(*((uint32_t*)(this->data_struct + offset)));
			if (this->debug)
			{
				DBG3(DBG_ENC, "   %b", &val, sizeof(uint32_t));
			}
			write_bytes_to_buffer(this, &val, sizeof(uint32_t));
			break;
		}
		case IKE_SPI:
		{
			/* 64 bit are written as-is, no host order conversion */
			write_bytes_to_buffer(this, this->data_struct + offset,
								  sizeof(uint64_t));
			if (this->debug)
			{
				DBG3(DBG_ENC, "   %b", this->data_struct + offset,
					 sizeof(uint64_t));
			}
			break;
		}
		default:
		{
			DBG1(DBG_ENC, "U_INT Type %N is not supported",
				 encoding_type_names, int_type);
			return;
		}
	}
}

/**
 * Generate a FLAG filed
 */
static void generate_flag(private_generator_t *this, uint32_t offset)
{
	uint8_t flag_value;
	uint8_t flag;

	flag_value = (*((bool *) (this->data_struct + offset))) ? 1 : 0;
	/* get flag position */
	flag = (flag_value << (7 - this->current_bit));

	/* make sure one bit is available in buffer */
	make_space_available(this, 1);
	if (this->current_bit == 0)
	{
		/* memory must be zero */
		*(this->out_position) = 0x00;
	}

	*(this->out_position) = *(this->out_position) | flag;
	if (this->debug)
	{
		DBG3(DBG_ENC, "   => %d", *this->out_position);
	}

	this->current_bit++;
	if (this->current_bit >= 8)
	{
		this->current_bit = this->current_bit % 8;
		this->out_position++;
	}
}

/**
 * Generates a bytestream from a chunk_t.
 */
static void generate_from_chunk(private_generator_t *this, uint32_t offset)
{
	chunk_t *value;

	if (this->current_bit != 0)
	{
		DBG1(DBG_ENC, "can not generate a chunk at bitpos %d",
			 this->current_bit);
		return ;
	}

	value = (chunk_t *)(this->data_struct + offset);
	if (this->debug)
	{
		DBG3(DBG_ENC, "   %B", value);
	}

	write_bytes_to_buffer(this, value->ptr, value->len);
}

METHOD(generator_t, get_chunk, chunk_t,
	private_generator_t *this, uint32_t **lenpos)
{
	chunk_t data;

	*lenpos = (uint32_t*)(this->buffer + this->header_length_offset);
	data = chunk_create(this->buffer, get_length(this));
	if (this->debug)
	{
		DBG3(DBG_ENC, "generated data of this generator %B", &data);
	}
	return data;
}

METHOD(generator_t, generate_payload, void,
	private_generator_t *this, payload_t *payload)
{
	int i, offset_start, rule_count;
	encoding_rule_t *rules;
	payload_type_t payload_type;

	this->data_struct = payload;
	payload_type = payload->get_type(payload);

	offset_start = this->out_position - this->buffer;

	if (this->debug)
	{
		DBG2(DBG_ENC, "generating payload of type %N",
			 payload_type_names, payload_type);
	}

	/* each payload has its own encoding rules */
	rule_count = payload->get_encoding_rules(payload, &rules);

	for (i = 0; i < rule_count;i++)
	{
		if (this->debug)
		{
			DBG2(DBG_ENC, "  generating rule %d %N",
				 i, encoding_type_names, rules[i].type);
		}
		switch ((int)rules[i].type)
		{
			case U_INT_4:
			case U_INT_8:
			case U_INT_16:
			case U_INT_32:
			case PAYLOAD_LENGTH:
			case IKE_SPI:
			case RESERVED_BYTE:
			case SPI_SIZE:
			case TS_TYPE:
			case ATTRIBUTE_TYPE:
			case ATTRIBUTE_LENGTH:
				generate_u_int_type(this, rules[i].type, rules[i].offset);
				break;
			case RESERVED_BIT:
			case FLAG:
				generate_flag(this, rules[i].offset);
				break;
			case HEADER_LENGTH:
				this->header_length_offset = get_offset(this);
				generate_u_int_type(this, U_INT_32, rules[i].offset);
				break;
			case ADDRESS:
			case SPI:
			case CHUNK_DATA:
			case ENCRYPTED_DATA:
				generate_from_chunk(this, rules[i].offset);
				break;
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
				linked_list_t *proposals;
				enumerator_t *enumerator;
				payload_t *proposal;

				proposals = *((linked_list_t **)
										(this->data_struct + rules[i].offset));
				enumerator = proposals->create_enumerator(proposals);
				while (enumerator->enumerate(enumerator, &proposal))
				{
					generate_payload(this, proposal);
				}
				enumerator->destroy(enumerator);
				break;
			}
			case ATTRIBUTE_FORMAT:
				generate_flag(this, rules[i].offset);
				/* Attribute format is a flag which is stored in context*/
				this->attribute_format =
							*((bool *)(this->data_struct + rules[i].offset));
				break;
			case ATTRIBUTE_LENGTH_OR_VALUE:
				if (this->attribute_format)
				{
					generate_u_int_type(this, U_INT_16, rules[i].offset);
				}
				else
				{
					generate_u_int_type(this, U_INT_16, rules[i].offset);
					/* this field hold the length of the attribute */
					this->attribute_length =
						*((uint16_t *)(this->data_struct + rules[i].offset));
				}
				break;
			case ATTRIBUTE_VALUE:
			{
				if (!this->attribute_format)
				{
					if (this->debug)
					{
						DBG2(DBG_ENC, "attribute value has not fixed size");
					}
					/* the attribute value is generated */
					generate_from_chunk(this, rules[i].offset);
				}
				break;
			}
			default:
				DBG1(DBG_ENC, "field type %N is not supported",
					 encoding_type_names, rules[i].type);
				return;
		}
	}
	if (this->debug)
	{
		DBG2(DBG_ENC, "generating %N payload finished",
			 payload_type_names, payload_type);
		DBG3(DBG_ENC, "generated data for this payload %b",
			 this->buffer + offset_start,
			 (u_int)(this->out_position - this->buffer - offset_start));
	}
}

METHOD(generator_t, destroy, void,
	private_generator_t *this)
{
	free(this->buffer);
	free(this);
}

/*
 * Described in header
 */
generator_t *generator_create()
{
	private_generator_t *this;

	INIT(this,
		.public = {
			.get_chunk = _get_chunk,
			.generate_payload = _generate_payload,
			.destroy = _destroy,
		},
		.buffer = malloc(GENERATOR_DATA_BUFFER_SIZE),
		.debug = TRUE,
	);

	this->out_position = this->buffer;
	this->roof_position = this->buffer + GENERATOR_DATA_BUFFER_SIZE;

	return &this->public;
}

/*
 * Described in header
 */
generator_t *generator_create_no_dbg()
{
	private_generator_t *this = (private_generator_t*)generator_create();

	this->debug = FALSE;

	return &this->public;
}
