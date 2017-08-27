/*
 * Copyright (C) 2001-2008 Andreas Steffen
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
 * @defgroup lexparser lexparser
 * @{ @ingroup utils
 */

#ifndef LEXPARSER_H_
#define LEXPARSER_H_

#include <library.h>

/**
 * Eats whitespace
 */
bool eat_whitespace(chunk_t *src);

/**
 * Compare null-terminated pattern with chunk
 */
bool match(const char *pattern, const chunk_t *ch);

/**
 * Extracts a token ending with the first occurrence of a given termination symbol
 */
bool extract_token(chunk_t *token, const char termination, chunk_t *src);

/**
 * Extracts a token ending with the first occurrence of a given null-terminated string
 */
bool extract_token_str(chunk_t *token, const char *termination, chunk_t *src);

/**
 *  Fetches a new text line terminated by \\n or \\r\\n
 */
bool fetchline(chunk_t *src, chunk_t *line);

/**
 * Extracts a value that might be single or double quoted
 */
err_t extract_value(chunk_t *value, chunk_t *line);

/**
 * extracts a name: value pair from a text line
 */
err_t extract_name_value(chunk_t *name, chunk_t *value, chunk_t *line);

/**
 * extracts a parameter: value from a text line
 */
err_t extract_parameter_value(chunk_t *name, chunk_t *value, chunk_t *line);

#endif /** LEXPARSER_H_ @}*/
