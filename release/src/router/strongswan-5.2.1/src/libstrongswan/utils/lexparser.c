/*
 * Copyright (C) 2001-2006 Andreas Steffen
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

#include "lexparser.h"

/**
 * eat whitespace
 */
bool eat_whitespace(chunk_t *src)
{
	while (src->len > 0 && (*src->ptr == ' ' || *src->ptr == '\t'))
	{
		src->ptr++;  src->len--;
	}
	return  src->len > 0 && *src->ptr != '#';
}

/**
 * compare string with chunk
 */
bool match(const char *pattern, const chunk_t *ch)
{
	return ch->len == strlen(pattern) && strncmp(pattern, ch->ptr, ch->len) == 0;
}

/**
 * extracts a token ending with the first occurrence of a given termination symbol
 */
bool extract_token(chunk_t *token, const char termination, chunk_t *src)
{
	u_char *eot = memchr(src->ptr, termination, src->len);

	if (termination == ' ')
	{
		u_char *eot_tab = memchr(src->ptr, '\t', src->len);

		/* check if a tab instead of a space terminates the token */
		eot = ( eot_tab == NULL || (eot && eot < eot_tab) ) ? eot : eot_tab;
	}

	/* initialize empty token */
	*token = chunk_empty;

	if (eot == NULL) /* termination symbol not found */
	{
		return FALSE;
	}

	/* extract token */
	token->ptr = src->ptr;
	token->len = (u_int)(eot - src->ptr);

	/* advance src pointer after termination symbol */
	src->ptr = eot + 1;
	src->len -= (token->len + 1);

	return TRUE;
}

/**
 * extracts a token ending with the first occurrence of a given null-terminated string
 */
bool extract_token_str(chunk_t *token, const char *termination, chunk_t *src)
{
	u_char *eot = memstr(src->ptr, termination, src->len);
	size_t l = strlen(termination);

	/* initialize empty token */
	*token = chunk_empty;

	if (eot == NULL) /* termination string not found */
	{
		return FALSE;
	}

	/* extract token */
	token->ptr = src->ptr;
	token->len = (u_int)(eot - src->ptr);

	/* advance src pointer after termination string */
	src->ptr = eot + l;
	src->len -= (token->len + l);

	return TRUE;
}

/**
 *  fetches a new line terminated by \n or \r\n
 */
bool fetchline(chunk_t *src, chunk_t *line)
{
	if (src->len == 0) /* end of src reached */
		return FALSE;

	if (extract_token(line, '\n', src))
	{
		if (line->len > 0 && *(line->ptr + line->len -1) == '\r')
			line->len--;  /* remove optional \r */
	}
	else /*last line ends without newline */
	{
		*line = *src;
		src->ptr += src->len;
		src->len = 0;
	}
	return TRUE;
}

err_t extract_value(chunk_t *value, chunk_t *line)
{
	char delimiter = ' ';

	if (!eat_whitespace(line))
	{
		*value = chunk_empty;
		return NULL;
	}
	if (*line->ptr == '\'' || *line->ptr == '"')
	{
		delimiter = *line->ptr;
		line->ptr++;  line->len--;
	}
	if (!extract_token(value, delimiter, line))
	{
		if (delimiter == ' ')
		{
			*value = *line;
			line->len = 0;
		}
		else
		{
			return "missing second delimiter";
		}
	}
	return NULL;
}

/**
 * extracts a parameter: value pair
 */
err_t extract_parameter_value(chunk_t *name, chunk_t *value, chunk_t *line)
{
	/* extract name */
	if (!extract_token(name,':', line))
	{
		return "missing ':'";
	}

	/* extract value */
	return extract_value(value, line);
}
