/*
 * Copyright (C) 2006 Martin Will
 * Copyright (C) 2000-2016 Andreas Steffen
 *
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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <utils/debug.h>

#include "oid.h"
#include "asn1.h"
#include "asn1_parser.h"

/**
 * Commonly used ASN1 values.
 */
const chunk_t ASN1_INTEGER_0 = chunk_from_chars(0x02, 0x01, 0x00);
const chunk_t ASN1_INTEGER_1 = chunk_from_chars(0x02, 0x01, 0x01);
const chunk_t ASN1_INTEGER_2 = chunk_from_chars(0x02, 0x01, 0x02);

/*
 * Described in header
 */
chunk_t asn1_algorithmIdentifier_params(int oid, chunk_t params)
{
	return asn1_wrap(ASN1_SEQUENCE, "mm", asn1_build_known_oid(oid), params);
}

/*
 * Described in header
 */
chunk_t asn1_algorithmIdentifier(int oid)
{
	chunk_t parameters;

	/* some algorithmIdentifiers have a NULL parameters field and some do not */
	switch (oid)
	{
		case OID_ECDSA_WITH_SHA1:
		case OID_ECDSA_WITH_SHA224:
		case OID_ECDSA_WITH_SHA256:
		case OID_ECDSA_WITH_SHA384:
		case OID_ECDSA_WITH_SHA512:
		case OID_ED25519:
		case OID_ED448:
			parameters = chunk_empty;
			break;
		default:
			parameters = asn1_simple_object(ASN1_NULL, chunk_empty);
			break;
	}
	return asn1_algorithmIdentifier_params(oid, parameters);
}

/*
 * Defined in header.
 */
int asn1_known_oid(chunk_t object)
{
	int oid = 0;

	while (object.len)
	{
		if (oid_names[oid].octet == *object.ptr)
		{
			if (--object.len == 0 || oid_names[oid].down == 0)
			{
				return oid;		  /* found terminal symbol */
			}
			else
			{
				object.ptr++; oid++; /* advance to next hex octet */
			}
		}
		else
		{
			if (oid_names[oid].next)
			{
				oid = oid_names[oid].next;
			}
			else
			{
				return OID_UNKNOWN;
			}
		}
	}
	return OID_UNKNOWN;
}

/*
 * Defined in header.
 */
chunk_t asn1_build_known_oid(int n)
{
	chunk_t oid;
	int i;

	if (n < 0 || n >= OID_MAX)
	{
		return chunk_empty;
	}

	i = oid_names[n].level + 1;
	oid = chunk_alloc(2 + i);
	oid.ptr[0] = ASN1_OID;
	oid.ptr[1] = i;

	do
	{
		if (oid_names[n].level >= i)
		{
			n--;
			continue;
		}
		oid.ptr[--i + 2] = oid_names[n--].octet;
	}
	while (i > 0);

	return oid;
}

/**
 * Returns the number of bytes required to encode the given OID node
 */
static int bytes_required(u_int val)
{
	int shift, required = 1;

	/* sufficient to handle 32 bit node numbers */
	for (shift = 28; shift; shift -= 7)
	{
		if (val >> shift)
		{	/* do not encode leading zeroes */
			required++;
		}
	}
	return required;
}

/*
 * Defined in header.
 */
chunk_t asn1_oid_from_string(char *str)
{
	enumerator_t *enumerator;
	size_t buf_len = 64;
	u_char buf[buf_len];
	char *end;
	int i = 0, pos = 0, req, shift;
	u_int val, first = 0;

	enumerator = enumerator_create_token(str, ".", "");
	while (enumerator->enumerate(enumerator, &str))
	{
		val = strtoul(str, &end, 10);
		req = bytes_required(val);
		if (end == str || pos + req > buf_len)
		{
			pos = 0;
			break;
		}
		switch (i++)
		{
			case 0:
				first = val;
				break;
			case 1:
				buf[pos++] = first * 40 + val;
				break;
			default:
				for (shift = (req - 1) * 7; shift; shift -= 7)
				{
					buf[pos++] = 0x80 | ((val >> shift) & 0x7F);
				}
				buf[pos++] = val & 0x7F;
		}
	}
	enumerator->destroy(enumerator);

	return chunk_clone(chunk_create(buf, pos));
}

/*
 * Defined in header.
 */
char *asn1_oid_to_string(chunk_t oid)
{
	size_t len = 64;
	char buf[len], *pos = buf;
	int written;
	u_int val;

	if (!oid.len)
	{
		return NULL;
	}
	val = oid.ptr[0] / 40;
	written = snprintf(buf, len, "%u.%u", val, oid.ptr[0] - val * 40);
	oid = chunk_skip(oid, 1);
	if (written < 0 || written >= len)
	{
		return NULL;
	}
	pos += written;
	len -= written;
	val = 0;

	while (oid.len)
	{
		val = (val << 7) + (u_int)(oid.ptr[0] & 0x7f);

		if (oid.ptr[0] < 128)
		{
			written = snprintf(pos, len, ".%u", val);
			if (written < 0 || written >= len)
			{
				return NULL;
			}
			pos += written;
			len -= written;
			val = 0;
		}
		oid = chunk_skip(oid, 1);
	}
	return (val == 0) ? strdup(buf) : NULL;
}

/*
 * Defined in header.
 */
size_t asn1_length(chunk_t *blob)
{
	u_char n;
	size_t len;

	if (blob->len < 2)
	{
		DBG2(DBG_ASN, "insufficient number of octets to parse ASN.1 length");
		return ASN1_INVALID_LENGTH;
	}

	/* read length field, skip tag and length */
	n = blob->ptr[1];
	blob->ptr += 2;
	blob->len -= 2;

	if ((n & 0x80) == 0)
	{	/* single length octet */
		if (n > blob->len)
		{
			DBG2(DBG_ASN, "length is larger than remaining blob size");
			return ASN1_INVALID_LENGTH;
		}
		return n;
	}

	/* composite length, determine number of length octets */
	n &= 0x7f;

	if (n == 0 || n > blob->len)
	{
		DBG2(DBG_ASN, "number of length octets invalid");
		return ASN1_INVALID_LENGTH;
	}

	if (n > sizeof(len))
	{
		DBG2(DBG_ASN, "number of length octets is larger than limit of"
			 " %d octets", (int)sizeof(len));
		return ASN1_INVALID_LENGTH;
	}

	len = 0;

	while (n-- > 0)
	{
		len = 256*len + *blob->ptr++;
		blob->len--;
	}
	if (len > blob->len)
	{
		DBG2(DBG_ASN, "length is larger than remaining blob size");
		return ASN1_INVALID_LENGTH;
	}
	return len;
}

/*
 * See header.
 */
int asn1_unwrap(chunk_t *blob, chunk_t *inner)
{
	chunk_t res;
	u_char len;
	int type;

	if (blob->len < 2)
	{
		return ASN1_INVALID;
	}
	type = blob->ptr[0];
	len = blob->ptr[1];
	*blob = chunk_skip(*blob, 2);

	if ((len & 0x80) == 0)
	{	/* single length octet */
		res.len = len;
	}
	else
	{	/* composite length, determine number of length octets */
		len &= 0x7f;
		if (len == 0 || len > blob->len || len > sizeof(res.len))
		{
			return ASN1_INVALID;
		}
		res.len = 0;
		while (len-- > 0)
		{
			res.len = 256 * res.len + blob->ptr[0];
			*blob = chunk_skip(*blob, 1);
		}
	}
	if (res.len > blob->len)
	{
		return ASN1_INVALID;
	}
	res.ptr = blob->ptr;
	*blob = chunk_skip(*blob, res.len);
	/* updating inner not before we are finished allows a caller to pass
	 * blob = inner */
	*inner = res;
	return type;
}

static const int days[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static const int tm_leap_1970 = 477;

/**
 * Converts ASN.1 UTCTIME or GENERALIZEDTIME into calendar time
 */
time_t asn1_to_time(const chunk_t *utctime, asn1_t type)
{
	int tm_year, tm_mon, tm_day, tm_hour, tm_min, tm_sec;
	int tm_leap_4, tm_leap_100, tm_leap_400, tm_leap;
	int tz_hour, tz_min, tz_offset;
	time_t tm_days, tm_secs;
	char buf[BUF_LEN], *eot = NULL;

	snprintf(buf, sizeof(buf), "%.*s", (int)utctime->len, utctime->ptr);

	if ((eot = strchr(buf, 'Z')) != NULL)
	{
		tz_offset = 0; /* Zulu time with a zero time zone offset */
	}
	else if ((eot = strchr(buf, '+')) != NULL)
	{
		if (sscanf(eot+1, "%2d%2d", &tz_hour, &tz_min) != 2)
		{
			return 0; /* error in positive timezone offset format */
		}
		tz_offset = 3600*tz_hour + 60*tz_min;  /* positive time zone offset */
	}
	else if ((eot = strchr(buf, '-')) != NULL)
	{
		if (sscanf(eot+1, "%2d%2d", &tz_hour, &tz_min) != 2)
		{
			return 0; /* error in negative timezone offset format */
		}
		tz_offset = -3600*tz_hour - 60*tz_min;  /* negative time zone offset */
	}
	else
	{
		return 0; /* error in time format */
	}

	/* parse ASN.1 time string */
	{
		const char* format = (type == ASN1_UTCTIME)? "%2d%2d%2d%2d%2d":
													 "%4d%2d%2d%2d%2d";

		if (sscanf(buf, format, &tm_year, &tm_mon, &tm_day,
								&tm_hour, &tm_min) != 5)
		{
			return 0; /* error in [yy]yymmddhhmm time format */
		}
	}

	/* is there a seconds field? */
	if ((eot - buf) == ((type == ASN1_UTCTIME)?12:14))
	{
		if (sscanf(eot-2, "%2d", &tm_sec) != 1)
		{
			return 0; /* error in ss seconds field format */
		}
	}
	else
	{
		tm_sec = 0;
	}

	/* representation of two-digit years */
	if (type == ASN1_UTCTIME)
	{
		tm_year += (tm_year < 50) ? 2000 : 1900;
	}

	/* prevent obvious 32 bit integer overflows */
	if (sizeof(time_t) == 4 && (tm_year > 2038 || tm_year < 1901))
	{
		return TIME_32_BIT_SIGNED_MAX;
	}

	/* representation of months as 0..11*/
	if (tm_mon < 1 || tm_mon > 12)
	{
		return 0;
	}
	tm_mon--;

	/* representation of days as 0..30 */
	if (tm_day < 1 || tm_day > 31)
	{	/* we don't actually validate the day in relation to tm_year/tm_mon */
		return 0;
	}
	tm_day--;

	if (tm_hour < 0 || tm_hour > 23 ||
		tm_min < 0 || tm_min > 59 ||
		tm_sec < 0 || tm_sec > 60 /* allow leap seconds */)
	{
		return 0;
	}

	/* number of leap years between last year and 1970? */
	tm_leap_4 = (tm_year - 1) / 4;
	tm_leap_100 = tm_leap_4 / 25;
	tm_leap_400 = tm_leap_100 / 4;
	tm_leap = tm_leap_4 - tm_leap_100 + tm_leap_400 - tm_leap_1970;

	/* if date later then February, is the current year a leap year? */
	if (tm_mon > 1 && (tm_year % 4 == 0) &&
		(tm_year % 100 != 0 || tm_year % 400 == 0))
	{
		tm_leap++;
	}
	tm_days = 365 * (tm_year - 1970) + days[tm_mon] + tm_day + tm_leap;
	tm_secs = 60 * (60 * (24 * tm_days + tm_hour) + tm_min) + tm_sec - tz_offset;

	if (sizeof(time_t) == 4)
	{	/* has a 32 bit signed integer overflow occurred? */
		if (tm_year > 1970 && tm_secs < 0)
		{	/* depending on the time zone, the first days in 1970 may result in
			 * a negative value, but dates after 1970 never will */
			return TIME_32_BIT_SIGNED_MAX;
		}
		if (tm_year < 1969 && tm_secs > 0)
		{	/* similarly, tm_secs is not positive for dates before 1970, except
			 * for the last days in 1969, depending on the time zone */
			return TIME_32_BIT_SIGNED_MAX;
		}
	}
	return tm_secs;
}

/**
 *  Convert a date into ASN.1 UTCTIME or GENERALIZEDTIME format
 */
chunk_t asn1_from_time(const time_t *time, asn1_t type)
{
	int offset;
	const char *format;
	char buf[BUF_LEN];
	chunk_t formatted_time;
	struct tm t = {};

	gmtime_r(time, &t);
	/* RFC 5280 says that dates through the year 2049 MUST be encoded as UTCTIME
	 * and dates in 2050 or later MUST be encoded as GENERALIZEDTIME. We only
	 * enforce the latter to avoid overflows but allow callers to force the
	 * encoding to GENERALIZEDTIME */
	type = (t.tm_year >= 150) ? ASN1_GENERALIZEDTIME : type;
	if (type == ASN1_GENERALIZEDTIME)
	{
		format = "%04d%02d%02d%02d%02d%02dZ";
		offset = 1900;
	}
	else /* ASN1_UTCTIME */
	{
		format = "%02d%02d%02d%02d%02d%02dZ";
		offset = (t.tm_year < 100) ? 0 : -100;
	}
	snprintf(buf, BUF_LEN, format, t.tm_year + offset,
			 t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	formatted_time.ptr = buf;
	formatted_time.len = strlen(buf);
	return asn1_simple_object(type, formatted_time);
}

/*
 * Defined in header.
 */
void asn1_debug_simple_object(chunk_t object, asn1_t type, bool private)
{
	int oid;

	switch (type)
	{
		case ASN1_OID:
			oid = asn1_known_oid(object);
			if (oid == OID_UNKNOWN)
			{
				char *oid_str = asn1_oid_to_string(object);

				if (!oid_str)
				{
					break;
				}
				DBG2(DBG_ASN, "  %s", oid_str);
				free(oid_str);
			}
			else
			{
				DBG2(DBG_ASN, "  '%s'", oid_names[oid].name);
			}
			return;
		case ASN1_UTF8STRING:
		case ASN1_IA5STRING:
		case ASN1_PRINTABLESTRING:
		case ASN1_T61STRING:
		case ASN1_VISIBLESTRING:
			DBG2(DBG_ASN, "  '%.*s'", (int)object.len, object.ptr);
			return;
		case ASN1_UTCTIME:
		case ASN1_GENERALIZEDTIME:
			{
				time_t time = asn1_to_time(&object, type);

				DBG2(DBG_ASN, "  '%T'", &time, TRUE);
			}
			return;
		default:
			break;
	}
	if (private)
	{
		DBG4(DBG_ASN, "%B", &object);
	}
	else
	{
		DBG3(DBG_ASN, "%B", &object);
	}
}

/**
 * parse an ASN.1 simple type
 */
bool asn1_parse_simple_object(chunk_t *object, asn1_t type, u_int level, const char* name)
{
	size_t len;

	/* an ASN.1 object must possess at least a tag and length field */
	if (object->len < 2)
	{
		DBG2(DBG_ASN, "L%d - %s:  ASN.1 object smaller than 2 octets", level,
			 name);
		return FALSE;
	}

	if (*object->ptr != type)
	{
		DBG2(DBG_ASN, "L%d - %s: ASN1 tag 0x%02x expected, but is 0x%02x",
			 level, name, type, *object->ptr);
		return FALSE;
	}

	len = asn1_length(object);

	if (len == ASN1_INVALID_LENGTH)
	{
		DBG2(DBG_ASN, "L%d - %s:  length of ASN.1 object invalid or too large",
			 level, name);
		return FALSE;
	}

	DBG2(DBG_ASN, "L%d - %s:", level, name);
	asn1_debug_simple_object(*object, type, FALSE);
	return TRUE;
}

/*
 * Described in header
 */
uint64_t asn1_parse_integer_uint64(chunk_t blob)
{
	uint64_t val = 0;
	int i;

	for (i = 0; i < blob.len; i++)
	{	/* if it is longer than 8 bytes, we just use the 8 LSBs */
		val <<= 8;
		val |= (uint64_t)blob.ptr[i];
	}
	return val;
}

/*
 * Described in header
 */
chunk_t asn1_integer_from_uint64(uint64_t val)
{
	u_char buf[sizeof(val)];
	chunk_t enc = chunk_empty;

	if (val < 0x100)
	{
		buf[0] = (u_char)val;
		return chunk_clone(chunk_create(buf, 1));
	}
	for (enc.ptr = buf + sizeof(val); val; enc.len++, val >>= 8)
	{	/* fill the buffer from the end */
		*(--enc.ptr) = val & 0xff;
	}
	return chunk_clone(enc);
}

/**
 * ASN.1 definition of an algorithmIdentifier
 */
static const asn1Object_t algorithmIdentifierObjects[] = {
	{ 0, "algorithmIdentifier",	ASN1_SEQUENCE,		ASN1_NONE			}, /* 0 */
	{ 1,   "algorithm",			ASN1_OID,			ASN1_BODY			}, /* 1 */
	{ 1,   "parameters",		ASN1_OID,			ASN1_RAW|ASN1_OPT	}, /* 2 */
	{ 1,   "end opt",			ASN1_EOC,			ASN1_END			}, /* 3 */
	{ 1,   "parameters",		ASN1_SEQUENCE,		ASN1_RAW|ASN1_OPT	}, /* 4 */
	{ 1,   "end opt",			ASN1_EOC,			ASN1_END			}, /* 5 */
	{ 1,   "parameters",		ASN1_OCTET_STRING,	ASN1_RAW|ASN1_OPT	}, /* 6 */
	{ 1,   "end opt",			ASN1_EOC,			ASN1_END			}, /* 7 */
	{ 0, "exit",				ASN1_EOC,			ASN1_EXIT			}
};
#define ALGORITHM_ID_ALG				1
#define ALGORITHM_ID_PARAMETERS_OID		2
#define ALGORITHM_ID_PARAMETERS_SEQ		4
#define ALGORITHM_ID_PARAMETERS_OCT		6

/*
 * Defined in header
 */
int asn1_parse_algorithmIdentifier(chunk_t blob, int level0, chunk_t *parameters)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	int alg = OID_UNKNOWN;

	parser = asn1_parser_create(algorithmIdentifierObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case ALGORITHM_ID_ALG:
				alg = asn1_known_oid(object);
				break;
			case ALGORITHM_ID_PARAMETERS_OID:
			case ALGORITHM_ID_PARAMETERS_SEQ:
			case ALGORITHM_ID_PARAMETERS_OCT:
				if (parameters != NULL)
				{
					*parameters = object;
				}
				break;
			default:
				break;
		}
	}
	parser->destroy(parser);
	return alg;
}

/*
 *  tests if a blob contains a valid ASN.1 set or sequence
 */
bool is_asn1(chunk_t blob)
{
	u_int len;
	u_char tag;

	if (!blob.len || !blob.ptr)
	{
		return FALSE;
	}

	tag = *blob.ptr;
	if (tag != ASN1_SEQUENCE && tag != ASN1_SET && tag != ASN1_OCTET_STRING)
	{
		DBG2(DBG_ASN, "  file content is not binary ASN.1");
		return FALSE;
	}

	len = asn1_length(&blob);

	if (len == ASN1_INVALID_LENGTH)
	{
		return FALSE;
	}

	/* exact match */
	if (len == blob.len)
	{
		return TRUE;
	}

	/* some websites append a surplus newline character to the blob */
	if (len + 1 == blob.len && *(blob.ptr + len) == '\n')
	{
		return TRUE;
	}

	DBG2(DBG_ASN, "  file size does not match ASN.1 coded length");
	return FALSE;
}

/*
 * Defined in header.
 */
bool asn1_is_printablestring(chunk_t str)
{
	const char printablestring_charset[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 '()+,-./:=?";
	u_int i;

	for (i = 0; i < str.len; i++)
	{
		if (strchr(printablestring_charset, str.ptr[i]) == NULL)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * codes ASN.1 lengths up to a size of 16'777'215 bytes
 */
static void asn1_code_length(size_t length, chunk_t *code)
{
	if (length < 128)
	{
		code->ptr[0] = length;
		code->len = 1;
	}
	else if (length < 256)
	{
		code->ptr[0] = 0x81;
		code->ptr[1] = (u_char) length;
		code->len = 2;
	}
	else if (length < 65536)
	{
		code->ptr[0] = 0x82;
		code->ptr[1] = length >> 8;
		code->ptr[2] = length & 0x00ff;
		code->len = 3;
	}
	else
	{
		code->ptr[0] = 0x83;
		code->ptr[1] = length >> 16;
		code->ptr[2] = (length >> 8) & 0x00ff;
		code->ptr[3] = length & 0x0000ff;
		code->len = 4;
	}
}

/**
 * build an empty asn.1 object with tag and length fields already filled in
 */
u_char* asn1_build_object(chunk_t *object, asn1_t type, size_t datalen)
{
	u_char length_buf[4];
	chunk_t length = { length_buf, 0 };
	u_char *pos;

	/* code the asn.1 length field */
	asn1_code_length(datalen, &length);

	/* allocate memory for the asn.1 TLV object */
	object->len = 1 + length.len + datalen;
	object->ptr = malloc(object->len);

	/* set position pointer at the start of the object */
	pos = object->ptr;

	/* copy the asn.1 tag field and advance the pointer */
	*pos++ = type;

	/* copy the asn.1 length field and advance the pointer */
	memcpy(pos, length.ptr, length.len);
	pos += length.len;

	return pos;
}

/**
 * Build a simple ASN.1 object
 */
chunk_t asn1_simple_object(asn1_t tag, chunk_t content)
{
	chunk_t object;

	u_char *pos = asn1_build_object(&object, tag, content.len);
	memcpy(pos, content.ptr, content.len);

	return object;
}

/**
 * Build an ASN.1 BIT_STRING object
 */
chunk_t asn1_bitstring(const char *mode, chunk_t content)
{
	chunk_t object;
	u_char *pos = asn1_build_object(&object, ASN1_BIT_STRING, 1 + content.len);

	*pos++ = 0x00;
	memcpy(pos, content.ptr, content.len);
	if (*mode == 'm')
	{
		free(content.ptr);
	}
	return object;
}

/**
 * Build an ASN.1 INTEGER object
 */
chunk_t asn1_integer(const char *mode, chunk_t content)
{
	chunk_t object;
	size_t len;
	u_char *pos;
	bool move;


	if (content.len == 0)
	{	/* make sure 0 is encoded properly */
		content = chunk_from_chars(0x00);
		move = FALSE;
	}
	else
	{
		move = (*mode == 'm');
	}

	/* ASN.1 integers must be positive numbers in two's complement */
	len = content.len + ((*content.ptr & 0x80) ? 1 : 0);
	pos = asn1_build_object(&object, ASN1_INTEGER, len);
	if (len > content.len)
	{
		*pos++ = 0x00;
	}
	memcpy(pos, content.ptr, content.len);

	if (move)
	{
		free(content.ptr);
	}
	return object;
}

/**
 * Build an ASN.1 object from a variable number of individual chunks.
 * Depending on the mode, chunks either are moved ('m') or copied ('c').
 */
chunk_t asn1_wrap(asn1_t type, const char *mode, ...)
{
	chunk_t construct;
	va_list chunks;
	u_char *pos;
	int i;
	int count = strlen(mode);

	/* sum up lengths of individual chunks */
	va_start(chunks, mode);
	construct.len = 0;
	for (i = 0; i < count; i++)
	{
		chunk_t ch = va_arg(chunks, chunk_t);
		construct.len += ch.len;
	}
	va_end(chunks);

	/* allocate needed memory for construct */
	pos = asn1_build_object(&construct, type, construct.len);

	/* copy or move the chunks */
	va_start(chunks, mode);
	for (i = 0; i < count; i++)
	{
		chunk_t ch = va_arg(chunks, chunk_t);

		memcpy(pos, ch.ptr, ch.len);
		pos += ch.len;

		switch (*mode++)
		{
			case 's':
				chunk_clear(&ch);
				break;
			case 'm':
				free(ch.ptr);
				break;
			default:
				break;
		}
	}
	va_end(chunks);

	return construct;
}

/**
 * ASN.1 definition of time
 */
static const asn1Object_t timeObjects[] = {
	{ 0, "utcTime",			ASN1_UTCTIME,			ASN1_OPT|ASN1_BODY	}, /* 0 */
	{ 0, "end opt",			ASN1_EOC,				ASN1_END			}, /* 1 */
	{ 0, "generalizeTime",	ASN1_GENERALIZEDTIME,	ASN1_OPT|ASN1_BODY	}, /* 2 */
	{ 0, "end opt",			ASN1_EOC,				ASN1_END			}, /* 3 */
	{ 0, "exit",			ASN1_EOC,				ASN1_EXIT			}
};
#ifdef TIME_UTC
/* used by C11 timespec_get(), <time.h> */
# undef TIME_UTC
#endif
#define TIME_UTC			0
#define TIME_GENERALIZED	2

/**
 * extracts and converts a UTCTIME or GENERALIZEDTIME object
 */
time_t asn1_parse_time(chunk_t blob, int level0)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	time_t utc_time = 0;

	parser= asn1_parser_create(timeObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		if (objectID == TIME_UTC || objectID == TIME_GENERALIZED)
		{
			utc_time = asn1_to_time(&object, (objectID == TIME_UTC)
									? ASN1_UTCTIME : ASN1_GENERALIZEDTIME);
		}
	}
	parser->destroy(parser);
	return utc_time;
}
