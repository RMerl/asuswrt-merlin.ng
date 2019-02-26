/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008 Andreas Steffen
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

#include <library.h>
#include <utils/debug.h>

#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <collections/linked_list.h>

#include "pkcs7_attributes.h"

typedef struct private_pkcs7_attributes_t private_pkcs7_attributes_t;
typedef struct attribute_t attribute_t;

/**
 * Private data of a pkcs7_attributes_t attribute list.
 */
struct private_pkcs7_attributes_t {
	/**
	 * Public interface
	 */
	pkcs7_attributes_t public;

	/**
	 * DER encoding of PKCS#9 attributes
	 */
	chunk_t encoding;

	/**
	 * Linked list of PKCS#9 attributes
	 */
	linked_list_t *attributes;
};

/**
 * Definition of an attribute_t object.
 */
struct attribute_t {

	/**
	 * Object Identifier (OID)
	 */
	int oid;

	/**
	 * Attribute value
	 */
	chunk_t value;

	/**
	 * ASN.1 encoding
	 */
	chunk_t encoding;
};

/**
 * Destroy an attribute_t object.
 */
static void attribute_destroy(attribute_t *this)
{
	free(this->value.ptr);
	free(this);
}

/**
 * Create an attribute_t object.
 */
static attribute_t *attribute_create(int oid, chunk_t value)
{
	attribute_t *this;

	INIT(this,
		.oid = oid,
		.value = chunk_clone(value),
	);

	return this;
}

/**
 * Build encoding of the attribute list
 */
static void build_encoding(private_pkcs7_attributes_t *this)
{
	enumerator_t *enumerator;
	attribute_t *attribute;
	u_int len = 0, count, i = 0;
	chunk_t *chunks;
	u_char *pos;

	count = this->attributes->get_count(this->attributes);
	chunks = malloc(sizeof(chunk_t) * count);

	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &attribute))
	{
		chunks[i] = asn1_wrap(ASN1_SEQUENCE, "mm",
								asn1_build_known_oid(attribute->oid),
								asn1_wrap(ASN1_SET, "c", attribute->value));
		len += chunks[i].len;
		i++;
	}
	enumerator->destroy(enumerator);

	pos = asn1_build_object(&this->encoding, ASN1_SET, len);
	for (i = 0; i < count; i++)
	{
		memcpy(pos, chunks[i].ptr, chunks[i].len);
		pos += chunks[i].len;
		free(chunks[i].ptr);
	}
	free(chunks);
}

METHOD(pkcs7_attributes_t, get_encoding, chunk_t,
	private_pkcs7_attributes_t *this)
{
	if (!this->encoding.len)
	{
		build_encoding(this);
	}
	return this->encoding;
}

METHOD(pkcs7_attributes_t, get_attribute, chunk_t,
	private_pkcs7_attributes_t *this, int oid)
{
	enumerator_t *enumerator;
	chunk_t value = chunk_empty;
	attribute_t *attribute;

	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &attribute))
	{
		if (attribute->oid == oid)
		{
			value = attribute->value;
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (value.len && asn1_unwrap(&value, &value) != ASN1_INVALID)
	{
		return value;
	}
	return chunk_empty;
}

METHOD(pkcs7_attributes_t, add_attribute, void,
	private_pkcs7_attributes_t *this, int oid, chunk_t value)
{
	this->attributes->insert_last(this->attributes,
								  attribute_create(oid, value));
	chunk_free(&value);

	/* rebuild encoding when adding attributes */
	chunk_free(&this->encoding);
}

METHOD(pkcs7_attributes_t, destroy, void,
	private_pkcs7_attributes_t *this)
{
	this->attributes->destroy_function(this->attributes,
									   (void*)attribute_destroy);
	free(this->encoding.ptr);
	free(this);
}

/*
 * Described in header.
 */
pkcs7_attributes_t *pkcs7_attributes_create(void)
{
	private_pkcs7_attributes_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_attribute = _get_attribute,
			.add_attribute = _add_attribute,
			.destroy = _destroy,
		},
		.attributes = linked_list_create(),
	);

	return &this->public;
}

/**
 * ASN.1 definition of the X.501 attribute type
 */
static const asn1Object_t attributesObjects[] = {
	{ 0, "attributes",		ASN1_SET,		ASN1_LOOP }, /* 0 */
	{ 1,   "attribute",		ASN1_SEQUENCE,	ASN1_NONE }, /* 1 */
	{ 2,     "type",		ASN1_OID,		ASN1_BODY }, /* 2 */
	{ 2,     "values",		ASN1_SET,		ASN1_LOOP }, /* 3 */
	{ 3,       "value",		ASN1_EOC,		ASN1_RAW  }, /* 4 */
	{ 2,     "end loop",	ASN1_EOC,		ASN1_END  }, /* 5 */
	{ 0, "end loop",		ASN1_EOC,		ASN1_END  }, /* 6 */
	{ 0, "exit",			ASN1_EOC,		ASN1_EXIT }
};
#define ATTRIBUTE_OBJ_TYPE 	2
#define ATTRIBUTE_OBJ_VALUE	4

/**
 * Parse a PKCS#9 attribute list
 */
static bool parse_attributes(chunk_t chunk, int level0,
							 private_pkcs7_attributes_t* this)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	int oid = OID_UNKNOWN;
	bool success = FALSE;

	parser = asn1_parser_create(attributesObjects, chunk);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case ATTRIBUTE_OBJ_TYPE:
				oid = asn1_known_oid(object);
				break;
			case ATTRIBUTE_OBJ_VALUE:
				if (oid != OID_UNKNOWN)
				{
					this->attributes->insert_last(this->attributes,
												  attribute_create(oid, object));
				}
				break;
		}
	}
	success = parser->success(parser);

	parser->destroy(parser);
	return success;
}

 /*
 * Described in header.
 */
pkcs7_attributes_t *pkcs7_attributes_create_from_chunk(chunk_t chunk,
													   u_int level)
{
	private_pkcs7_attributes_t *this;

	this = (private_pkcs7_attributes_t*)pkcs7_attributes_create();
	this->encoding = chunk_clone(chunk);
	if (!parse_attributes(chunk, level, this))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}
