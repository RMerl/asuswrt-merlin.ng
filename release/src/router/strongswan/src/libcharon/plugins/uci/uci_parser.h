/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Thomas Kallenberg
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
 * @defgroup uci_parser_t uci_parser
 * @{ @ingroup uci
 */

#ifndef UCI_PARSER_H_
#define UCI_PARSER_H_

#include <collections/enumerator.h>

typedef struct uci_parser_t uci_parser_t;

/**
 * Wrapper to parse UCI sections with an enumerator.
 */
struct uci_parser_t {

	/**
	 * Create an enumerator over a section.
	 *
	 * The enumerator returns a section name followed by values for the keywords
	 * specified in the variable argument list of this function.
	 *
	 * @param ...		variable argument list with keywords, NULL terminated
	 * @return			enumerator over sections
	 */
	enumerator_t* (*create_section_enumerator)(uci_parser_t *this, ...);

	/**
	 * Destroy the parser.
	 */
	void (*destroy)(uci_parser_t *this);
};

/**
 * Create a UCI parser.
 *
 * @param package	UCI package this parser should read
 * @return			parser context
 */
uci_parser_t *uci_parser_create(char *package);

#endif /** UCI_PARSER_H_ @}*/
