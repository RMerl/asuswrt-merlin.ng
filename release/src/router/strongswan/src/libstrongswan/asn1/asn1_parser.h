/*
 * Copyright (C) 2006 Martin Will
 * Copyright (C) 2000-2017 Andreas Steffen
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

/**
 * @defgroup asn1_parser asn1_parser
 * @{ @ingroup asn1
 */

#ifndef ASN1_PARSER_H_
#define ASN1_PARSER_H_

#include <stdarg.h>

#include <library.h>

#include "asn1.h"

/**
 * Definition of ASN.1 flags
 */
#define ASN1_NONE    0x0000
#define ASN1_DEF     0x0001
#define ASN1_OPT     0x0002
#define ASN1_LOOP    0x0004
#define ASN1_CHOICE  0x0008
#define ASN1_CH      0x0010
#define ASN1_END     0x0020
#define ASN1_OBJ     0x0040
#define ASN1_BODY    0x0080
#define ASN1_RAW     0x0100
#define ASN1_EXIT    0x0200

typedef struct asn1Object_t asn1Object_t;

/**
 * Syntax definition of an ASN.1 object
 */
struct asn1Object_t{
	u_int level;
	const u_char *name;
	asn1_t type;
	uint16_t flags;
};

typedef struct asn1_parser_t asn1_parser_t;

/**
 * Public interface of an ASN.1 parser
 */
struct asn1_parser_t {

	/**
	 * Parse the next ASN.1 object in the hierarchy and return it
	 *
	 * @param objectID	current line in the object syntax definition
	 * @param object	current object
	 * @return 			- FALSE if end of object syntax definition was reached
	 *							or a parsing error occurred
	 *					- TRUE	otherwise
	 */
	bool (*iterate)(asn1_parser_t *this, int *objectID, chunk_t *object);

	/**
	 * Get the current parsing level
	 *
	 * @return 			current level
	 */
	u_int (*get_level)(asn1_parser_t *this);

	/**
	 * Set the top-most level
	 *
	 * @param level		top-most level
	 */
	void (*set_top_level)(asn1_parser_t *this, u_int level0);

	/**
	 * Set implicit and private flags
	 *
	 * @param implicit	top-most type of object is implicit
	 * @param private	object data is private (use debug level 4)
	 */
	void (*set_flags)(asn1_parser_t *this, bool implicit, bool private);

	/**
	 * Show final parsing status
	 *
	 * @return			TRUE if parsing was successful, FALSE otherwise
	 */
	bool (*success)(asn1_parser_t *this);

	/**
	 * Destroy the ASN.1 parser
	 */
	void (*destroy)(asn1_parser_t *this);
};

/**
 * Create an ASN.1 parser
 *
 * @param objects	syntax definition of the ASN.1 object to be parsed
 * @param blob		ASN.1 coded binary blob
 * @return			ASN.1 context
 */
asn1_parser_t* asn1_parser_create(asn1Object_t const *objects, chunk_t blob);

#endif /** ASN1_PARSER_H_ @}*/
