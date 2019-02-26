/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2002-2008 Andreas Steffen
 * Copyright (C) 2005 Jan Hutter, Martin Willi
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

#include "pkcs7_generic.h"
#include "pkcs7_data.h"
#include "pkcs7_signed_data.h"
#include "pkcs7_encrypted_data.h"
#include "pkcs7_enveloped_data.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>

/**
 * ASN.1 definition of the PKCS#7 ContentInfo type
 */
static const asn1Object_t contentInfoObjects[] = {
	{ 0, "contentInfo",		ASN1_SEQUENCE,		ASN1_NONE }, /* 0 */
	{ 1,   "contentType",	ASN1_OID,			ASN1_BODY }, /* 1 */
	{ 1,   "content",		ASN1_CONTEXT_C_0,	ASN1_OPT |
												ASN1_BODY }, /* 2 */
	{ 1,   "end opt",		ASN1_EOC,			ASN1_END  }, /* 3 */
	{ 0, "exit",			ASN1_EOC,			ASN1_EXIT }
};
#define PKCS7_INFO_TYPE		1
#define PKCS7_INFO_CONTENT	2

/**
 * Parse PKCS#7 contentInfo object
 */
static pkcs7_t* parse_contentInfo(chunk_t blob)
{
	asn1_parser_t *parser;
	chunk_t object, content = chunk_empty;
	int objectID, type = OID_UNKNOWN;
	bool success = FALSE;

	parser = asn1_parser_create(contentInfoObjects, blob);
	parser->set_top_level(parser, 0);

	while (parser->iterate(parser, &objectID, &object))
	{
		if (objectID == PKCS7_INFO_TYPE)
		{
			type = asn1_known_oid(object);
			if (type < OID_PKCS7_DATA || type > OID_PKCS7_ENCRYPTED_DATA)
			{
				DBG1(DBG_ASN, "unknown pkcs7 content type");
				goto end;
			}
		}
		else if (objectID == PKCS7_INFO_CONTENT)
		{
			content = object;
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);

	if (success)
	{
		switch (type)
		{
			case OID_PKCS7_DATA:
				return pkcs7_data_load(blob, content);
			case OID_PKCS7_SIGNED_DATA:
				return pkcs7_signed_data_load(blob, content);
			case OID_PKCS7_ENVELOPED_DATA:
				return pkcs7_enveloped_data_load(blob, content);
			case OID_PKCS7_ENCRYPTED_DATA:
				return pkcs7_encrypted_data_load(blob, content);
			default:
				DBG1(DBG_ASN, "pkcs7 content type %d not supported", type);
				return NULL;
		}
	}
	return NULL;
}


pkcs7_t *pkcs7_generic_load(container_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (blob.len)
	{
		if (blob.len >= 2 &&
			blob.ptr[0] == ASN1_SEQUENCE && blob.ptr[1] == 0x80)
		{	/* looks like infinite length BER encoding, but we can't handle it.
			 * ignore silently, our openssl backend can handle it */
			return NULL;
		}
		return parse_contentInfo(blob);
	}
	return NULL;
}
