/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <string.h>

#include "util.h"
#include "strv.h"
#include "private.h"

/**
 * SECTION:strv
 * @short_description: String array functions
 *
 * String array functions
 */

/**
 * l_strfreev:
 * @strlist: String list to free
 *
 * Frees a list of strings
 **/
LIB_EXPORT void l_strfreev(char **strlist)
{
	l_strv_free(strlist);
}

/**
 * l_strsplit:
 * @str: String to split
 * @sep: The delimiter character
 *
 * Splits a string into pieces which do not contain the delimiter character.
 * As a special case, an empty string is returned as an empty array, e.g.
 * an array with just the NULL element.
 *
 * Note that this function only works with ASCII delimiters.
 *
 * Returns: A newly allocated %NULL terminated string array.  This array
 * should be freed using l_strfreev().
 **/
LIB_EXPORT char **l_strsplit(const char *str, const char sep)
{
	int len;
	int i;
	const char *p;
	char **ret;

	if (unlikely(!str))
		return NULL;

	if (str[0] == '\0')
		return l_new(char *, 1);

	for (p = str, len = 1; *p; p++)
		if (*p == sep)
			len += 1;

	ret = l_new(char *, len + 1);

	i = 0;
	p = str;
	len = 0;

	while (p[len]) {
		if (p[len] != sep) {
			len += 1;
			continue;
		}

		ret[i++] = l_strndup(p, len);
		p += len + 1;
		len = 0;
	}

	ret[i++] = l_strndup(p, len);

	return ret;
}

/**
 * l_strsplit_set:
 * @str: String to split
 * @separators: A set of delimiters
 *
 * Splits a string into pieces which do not contain the delimiter characters
 * that can be found in @separators.
 * As a special case, an empty string is returned as an empty array, e.g.
 * an array with just the NULL element.
 *
 * Note that this function only works with ASCII delimiters.
 *
 * Returns: A newly allocated %NULL terminated string array.  This array
 * should be freed using l_strfreev().
 **/
LIB_EXPORT char **l_strsplit_set(const char *str, const char *separators)
{
	int len;
	int i;
	const char *p;
	char **ret;
	bool sep_table[256];

	if (unlikely(!str))
		return NULL;

	if (str[0] == '\0')
		return l_new(char *, 1);

	memset(sep_table, 0, sizeof(sep_table));

	for (p = separators; *p; p++)
		sep_table[(unsigned char) *p] = true;

	for (p = str, len = 1; *p; p++)
		if (sep_table[(unsigned char) *p] == true)
			len += 1;

	ret = l_new(char *, len + 1);

	i = 0;
	p = str;
	len = 0;

	while (p[len]) {
		if (sep_table[(unsigned char) p[len]] != true) {
			len += 1;
			continue;
		}

		ret[i++] = l_strndup(p, len);
		p += len + 1;
		len = 0;
	}

	ret[i++] = l_strndup(p, len);

	return ret;
}

/**
 * l_strjoinv:
 * @str_array: a %NULL terminated array of strings to join
 * @delim: Delimiting character
 *
 * Joins strings contanied in the @str_array into one long string delimited
 * by @delim.
 *
 * Returns: A newly allocated string that should be freed using l_free()
 */
LIB_EXPORT char *l_strjoinv(char **str_array, const char delim)
{
	size_t len = 0;
	unsigned int i;
	char *ret;
	char *p;

	if (unlikely(!str_array))
		return NULL;

	if (!str_array[0])
		return l_strdup("");

	for (i = 0; str_array[i]; i++)
		len += strlen(str_array[i]);

	len += 1 + i - 1;

	ret = l_malloc(len);

	p = stpcpy(ret, str_array[0]);

	for (i = 1; str_array[i]; i++) {
		*p++ = delim;
		p = stpcpy(p, str_array[i]);
	}

	return ret;
}

/**
 * l_strv_new:
 *
 * Returns: new emptry string array
 **/
LIB_EXPORT char **l_strv_new(void)
{
	return l_new(char *, 1);
}

/**
 * l_strv_free:
 * @str_array: a %NULL terminated array of strings
 *
 * Frees strings in @str_array and @str_array itself
 **/
LIB_EXPORT void l_strv_free(char **str_array)
{
	if (likely(str_array)) {
		int i;

		for (i = 0; str_array[i]; i++)
			l_free(str_array[i]);

		l_free(str_array);
	}
}

/**
 * l_strv_length:
 * @str_array: a %NULL terminated array of strings
 *
 * Returns: the number of strings in @str_array
 */
LIB_EXPORT unsigned int l_strv_length(char **str_array)
{
	unsigned int i = 0;

	if (unlikely(!str_array))
		return 0;

	while (str_array[i])
		i += 1;

	return i;
}

/**
 * l_strv_contains:
 * @str_array: a %NULL terminated array of strings
 * @item: An item to search for, must be not %NULL
 *
 * Returns: #true if @str_array contains item
 */
LIB_EXPORT bool l_strv_contains(char **str_array, const char *item)
{
	unsigned int i = 0;

	if (unlikely(!str_array || !item))
		return false;

	while (str_array[i]) {
		if (!strcmp(str_array[i], item))
			return true;

		i += 1;
	}

	return false;
}

/**
 * l_strv_append:
 * @str_array: a %NULL terminated array of strings or %NULL
 * @str: A string to be appened at the end of @str_array
 *
 * Returns: New %NULL terminated array of strings with @str added
 */
LIB_EXPORT char **l_strv_append(char **str_array, const char *str)
{
	char **ret;
	unsigned int i, len;

	if (unlikely(!str))
		return str_array;

	len = l_strv_length(str_array);
	ret = l_new(char *, len + 2);

	for (i = 0; i < len; i++)
		ret[i] = str_array[i];

	ret[i] = l_strdup(str);

	l_free(str_array);

	return ret;
}

LIB_EXPORT char **l_strv_append_printf(char **str_array,
						const char *format, ...)
{
	va_list args;
	char **ret;

	va_start(args, format);
	ret = l_strv_append_vprintf(str_array, format, args);
	va_end(args);

	return ret;
}

LIB_EXPORT char **l_strv_append_vprintf(char **str_array,
					const char *format, va_list args)
{
	char **ret;
	unsigned int i, len;

	if (unlikely(!format))
		return str_array;

	len = l_strv_length(str_array);
	ret = l_new(char *, len + 2);

	for (i = 0; i < len; i++)
		ret[i] = str_array[i];

	ret[i] = l_strdup_vprintf(format, args);

	l_free(str_array);

	return ret;
}

/**
 * l_strv_copy:
 * @str_array: a %NULL terminated array of strings or %NULL
 *
 * Returns: An independent copy of @str_array.
 */
LIB_EXPORT char **l_strv_copy(char **str_array)
{
	int i, len;
	char **copy;

	if (unlikely(!str_array))
		return NULL;

	for (len = 0; str_array[len]; len++);

	copy = l_malloc(sizeof(char *) * (len + 1));

	for (i = len; i >= 0; i--)
		copy[i] = l_strdup(str_array[i]);

	return copy;
}
