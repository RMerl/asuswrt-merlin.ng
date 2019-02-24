/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup string_i string
 * @{ @ingroup utils_i
 */

#ifndef STRING_H_
#define STRING_H_

/**
 * Helper function that compares two strings for equality
 */
static inline bool streq(const char *x, const char *y)
{
	return (x == y) || (x && y && strcmp(x, y) == 0);
}

/**
 * Helper function that compares two strings for equality, length limited
 */
static inline bool strneq(const char *x, const char *y, size_t len)
{
	return (x == y) || (x && y && strncmp(x, y, len) == 0);
}

/**
 * Helper function that checks if a string starts with a given prefix
 */
static inline bool strpfx(const char *x, const char *prefix)
{
	return strneq(x, prefix, strlen(prefix));
}

/**
 * Helper function that compares two strings for equality ignoring case
 */
static inline bool strcaseeq(const char *x, const char *y)
{
	return (x == y) || (x && y && strcasecmp(x, y) == 0);
}

/**
 * Helper function that compares two strings for equality ignoring case, length limited
 */
static inline bool strncaseeq(const char *x, const char *y, size_t len)
{
	return (x == y) || (x && y && strncasecmp(x, y, len) == 0);
}

/**
 * Helper function that checks if a string starts with a given prefix
 */
static inline bool strcasepfx(const char *x, const char *prefix)
{
	return strncaseeq(x, prefix, strlen(prefix));
}

/**
 * NULL-safe strdup variant
 */
static inline char *strdupnull(const char *s)
{
	return s ? strdup(s) : NULL;
}

/**
 * Translates the characters in the given string, searching for characters
 * in 'from' and mapping them to characters in 'to'.
 * The two characters sets 'from' and 'to' must contain the same number of
 * characters.
 */
char *translate(char *str, const char *from, const char *to);

/**
 * Replaces all occurrences of search in the given string with replace.
 *
 * Allocates memory only if anything is replaced in the string.  The original
 * string is also returned if any of the arguments are invalid (e.g. if search
 * is empty or any of them are NULL).
 *
 * @param str		original string
 * @param search	string to search for and replace
 * @param replace	string to replace found occurrences with
 * @return			allocated string, if anything got replaced, str otherwise
 */
char *strreplace(const char *str, const char *search, const char *replace);

#endif /** STRING_H_ @} */
