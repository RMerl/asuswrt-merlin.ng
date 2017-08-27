/*
 * Copyright (C) 2006 Martin Will
 * Copyright (C) 2000-2008 Andreas Steffen
 *
 * Hochschule fuer Technik Rapperswil
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

/**
 * @defgroup asn1i asn1
 * @{ @ingroup asn1
 */

#ifndef ASN1_H_
#define ASN1_H_

#include <stdarg.h>

#include <library.h>

/**
 * Definition of some primitive ASN1 types
 */
typedef enum {
	ASN1_EOC =				0x00,
	ASN1_BOOLEAN =			0x01,
	ASN1_INTEGER =			0x02,
	ASN1_BIT_STRING =		0x03,
	ASN1_OCTET_STRING =		0x04,
	ASN1_NULL =				0x05,
	ASN1_OID =				0x06,
	ASN1_ENUMERATED =		0x0A,
	ASN1_UTF8STRING =		0x0C,
	ASN1_NUMERICSTRING =	0x12,
	ASN1_PRINTABLESTRING =	0x13,
	ASN1_T61STRING =		0x14,
	ASN1_VIDEOTEXSTRING =	0x15,
	ASN1_IA5STRING =		0x16,
	ASN1_UTCTIME =			0x17,
	ASN1_GENERALIZEDTIME =	0x18,
	ASN1_GRAPHICSTRING =	0x19,
	ASN1_VISIBLESTRING =	0x1A,
	ASN1_GENERALSTRING =	0x1B,
	ASN1_UNIVERSALSTRING =	0x1C,
	ASN1_BMPSTRING =		0x1E,

	ASN1_CONSTRUCTED =		0x20,

	ASN1_SEQUENCE =			0x30,
	ASN1_SET =				0x31,

	ASN1_CONTEXT_S_0 =		0x80,
	ASN1_CONTEXT_S_1 =		0x81,
	ASN1_CONTEXT_S_2 =		0x82,
	ASN1_CONTEXT_S_3 =		0x83,
	ASN1_CONTEXT_S_4 =		0x84,
	ASN1_CONTEXT_S_5 =		0x85,
	ASN1_CONTEXT_S_6 =		0x86,
	ASN1_CONTEXT_S_7 =		0x87,
	ASN1_CONTEXT_S_8 =		0x88,

	ASN1_CONTEXT_C_0 =		0xA0,
	ASN1_CONTEXT_C_1 =		0xA1,
	ASN1_CONTEXT_C_2 =		0xA2,
	ASN1_CONTEXT_C_3 =		0xA3,
	ASN1_CONTEXT_C_4 =		0xA4,
	ASN1_CONTEXT_C_5 =		0xA5,

	ASN1_INVALID =			0x100,
} asn1_t;

#define ASN1_INVALID_LENGTH	0xffffffff

/**
 * Some common prefabricated ASN.1 constants
 */
extern const chunk_t ASN1_INTEGER_0;
extern const chunk_t ASN1_INTEGER_1;
extern const chunk_t ASN1_INTEGER_2;


/** Some ASN.1 analysis functions */

/**
 * Build an algorithmIdentifier from a known OID.
 *
 * @param oid		known OID index
 * @return			body of the corresponding OID, allocated
 */
chunk_t asn1_algorithmIdentifier(int oid);

/**
 * Converts an ASN.1 OID into a known OID index
 *
 * @param object	body of an OID
 * @return			index into the oid_names[] table or OID_UNKNOWN
 */
int asn1_known_oid(chunk_t object);

/**
 * Converts a known OID index to an ASN.1 OID
 *
 * @param n			index into the oid_names[] table
 * @return			allocated OID chunk, chunk_empty if index out of range
 */
chunk_t asn1_build_known_oid(int n);

/**
 * Convert human readable OID to ASN.1 DER encoding, without OID header.
 *
 * @param str		OID string (e.g. 1.2.345.67.8)
 * @return			allocated ASN.1 encoded OID, chunk_empty on error
 */
chunk_t asn1_oid_from_string(char *str);

/**
 * Convert a DER encoded ASN.1 OID to a human readable string.
 *
 * @param oid		DER encoded OID, without header
 * @return			human readable OID string, allocated, NULL on error
 */
char* asn1_oid_to_string(chunk_t oid);

/**
 * Returns the length of an ASN.1 object
 * The blob pointer is advanced past the tag length fields
 *
 * @param blob		pointer to an ASN.1 coded blob
 * @return			length of ASN.1 object
 */
size_t asn1_length(chunk_t *blob);

/**
 * Unwrap the inner content of an ASN.1 type/length wrapped object.
 *
 * @param blob		blob to parse header from, moved behind parsed content
 * @param content	inner content
 * @return			parsed type, ASN1_INVALID if length parsing failed
 */
int asn1_unwrap(chunk_t *blob, chunk_t *content);

/**
 * Parses an ASN.1 algorithmIdentifier object
 *
 * @param blob		ASN.1 coded blob
 * @param level0	top-most level offset
 * @param params	returns optional [ASN.1 coded] parameters
 * @return			known OID index or OID_UNKNOWN
 */
int asn1_parse_algorithmIdentifier(chunk_t blob, int level0, chunk_t *params);

/**
 * Parse the top-most level of an ASN.1 object
 *
 * @param object	ASN.1 coded object
 * @param type		Expected ASN.1 type
 * @param level0	top-most level offset
 * @param name		descriptive name of object
 * @return			TRUE if parsing successful
 */
bool asn1_parse_simple_object(chunk_t *object, asn1_t type, u_int level0,
							  const char* name);

/**
 * Converts an ASN.1 INTEGER object to an u_int64_t. If the INTEGER is longer
 * than 8 bytes only the 8 LSBs are returned.
 *
 * @param blob		body of an ASN.1 coded integer object
 * @return			converted integer
 */
u_int64_t asn1_parse_integer_uint64(chunk_t blob);

/**
 * Print the value of an ASN.1 simple object
 *
 * @param object	ASN.1 object to be printed
 * @param type		asn1_t type
 * @param private	ASN.1 data is confidential (use debug level 4)
 */
void asn1_debug_simple_object(chunk_t object, asn1_t type, bool private);

/**
 * Converts an ASN.1 UTCTIME or GENERALIZEDTIME string to time_t
 *
 * On systems where sizeof(time_t) == 4 there will be an overflow
 * for dates
 *   > Tue, 19 Jan 2038 03:14:07 UTC (0x7fffffff)
 * and
 *   < Fri, 13 Dec 1901 20:45:52 UTC (0x80000000)
 * in both cases TIME_32_BIT_SIGNED_MAX is returned.
 *
 * @param utctime	body of an ASN.1 coded time object
 * @param type		ASN1_UTCTIME or ASN1_GENERALIZEDTIME
 * @return			time_t in UTC
 */
time_t asn1_to_time(const chunk_t *utctime, asn1_t type);

/**
 * Converts time_t to an ASN.1 UTCTIME or GENERALIZEDTIME string
 *
 * @note The type is automatically changed to GENERALIZEDTIME if needed
 *
 * @param time		time_t in UTC
 * @param type		ASN1_UTCTIME or ASN1_GENERALIZEDTIME
 * @return			body of an ASN.1 code time object
 */
chunk_t asn1_from_time(const time_t *time, asn1_t type);

/**
 * Parse an ASN.1 UTCTIME or GENERALIZEDTIME object
 *
 * @param blob		ASN.1 coded time object
 * @param level0	top-most level offset
 * @return			time_t in UTC
 */
time_t asn1_parse_time(chunk_t blob, int level0);

/**
 * Determines if a binary blob is ASN.1 coded
 *
 * @param blob		blob to be tested
 * @return			TRUE if blob is ASN.1 coded (SEQUENCE or SET)
 */
bool is_asn1(chunk_t blob);

/**
 * Determines if a character string can be coded as PRINTABLESTRING
 *
 * @param str		character string to be tested
 * @return			TRUE if no special characters are contained
 */
bool asn1_is_printablestring(chunk_t str);


/** some ASN.1 synthesis functions */

/**
 * Build an empty ASN.1 object with tag and length fields already filled in
 *
 * @param object	returned object - memory is allocated by function
 * @param type		ASN.1 type to be created
 * @param datalen	size of the body to be created
 * @return			points to the first position in the body
 */
u_char* asn1_build_object(chunk_t *object, asn1_t type, size_t datalen);

/**
 * Build a simple ASN.1 object
 *
 * @param tag		ASN.1 type to be created
 * @param content	content of the ASN.1 object
 * @return			chunk containing the ASN.1 coded object
 */
chunk_t asn1_simple_object(asn1_t tag, chunk_t content);

/**
 * Build an ASN.1 BITSTRING object
 *
 * @param mode		'c' for copy or 'm' for move
 * @param content	content of the BITSTRING
 * @return			chunk containing the ASN.1 coded BITSTRING
 */
chunk_t asn1_bitstring(const char *mode, chunk_t content);

/**
 * Build an ASN.1 INTEGER object
 *
 * @param mode		'c' for copy or 'm' for move
 * @param content	content of the INTEGER
 * @return			chunk containing the ASN.1 coded INTEGER
 */
chunk_t asn1_integer(const char *mode, chunk_t content);

/**
 * Build an ASN.1 object from a variable number of individual chunks
 *
 * The mode string specifies the number of chunks, and how to handle each of
 * them with a single character: 'c' for copy (allocate new chunk), 'm' for move
 * (free given chunk) or 's' for sensitive-copy (clear given chunk, then free).
 *
 * @param type		ASN.1 type to be created
 * @param mode		for each list member: 'c', 'm' or 's'
 * @return			chunk containing the ASN.1 coded object
 */
chunk_t asn1_wrap(asn1_t type, const char *mode, ...);

#endif /** ASN1_H_ @}*/
