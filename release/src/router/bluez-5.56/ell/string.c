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

#include <stdio.h>

#include "util.h"
#include "strv.h"
#include "string.h"
#include "private.h"

/**
 * SECTION:string
 * @short_description: Growable string buffer
 *
 * Growable string buffer support
 */

/**
 * l_string:
 *
 * Opague object representing the string buffer.
 */
struct l_string {
	size_t max;
	size_t len;
	char *str;
};

static inline size_t next_power(size_t len)
{
	size_t n = 1;

	if (len > SIZE_MAX / 2)
		return SIZE_MAX;

	while (n < len)
		n = n << 1;

	return n;
}

static void grow_string(struct l_string *str, size_t extra)
{
	if (str->len + extra < str->max)
		return;

	str->max = next_power(str->len + extra + 1);
	str->str = l_realloc(str->str, str->max);
}

/**
 * l_string_new:
 * @initial_length: Initial length of the groable string
 *
 * Create new growable string.  If the @initial_length is 0, then a safe
 * default is chosen.
 *
 * Returns: a newly allocated #l_string object.
 **/
LIB_EXPORT struct l_string *l_string_new(size_t initial_length)
{
	static const size_t DEFAULT_INITIAL_LENGTH = 127;
	struct l_string *ret;

	ret = l_new(struct l_string, 1);

	if (initial_length == 0)
		initial_length = DEFAULT_INITIAL_LENGTH;

	grow_string(ret, initial_length);
	ret->str[0] = '\0';

	return ret;
}

/**
 * l_string_free:
 * @string: growable string object
 *
 * Free the growable string object and all associated data
 **/
LIB_EXPORT void l_string_free(struct l_string *string)
{
	if (unlikely(!string))
		return;

	l_free(string->str);
	l_free(string);
}

/**
 * l_string_unwrap:
 * @string: growable string object
 *
 * Free the growable string object and return the internal string data.
 * The caller is responsible for freeing the string data using l_free(),
 * and the string object is no longer usable.
 *
 * Returns: @string's internal buffer
 **/
LIB_EXPORT char *l_string_unwrap(struct l_string *string)
{
	char *result;

	if (unlikely(!string))
		return NULL;

	result = string->str;

	l_free(string);

	return result;
}

/**
 * l_string_append:
 * @dest: growable string object
 * @src: C-style string to copy
 *
 * Appends the contents of @src to @dest.  The internal buffer of @dest is
 * grown if necessary.
 *
 * Returns: @dest
 **/
LIB_EXPORT struct l_string *l_string_append(struct l_string *dest,
						const char *src)
{
	size_t size;

	if (unlikely(!dest || !src))
		return NULL;

	size = strlen(src);

	grow_string(dest, size);

	memcpy(dest->str + dest->len, src, size);
	dest->len += size;
	dest->str[dest->len] = '\0';

	return dest;
}

/**
 * l_string_append_c:
 * @dest: growable string object
 * @c: Character
 *
 * Appends character given by @c to @dest.  The internal buffer of @dest is
 * grown if necessary.
 *
 * Returns: @dest
 **/
LIB_EXPORT struct l_string *l_string_append_c(struct l_string *dest,
						const char c)
{
	if (unlikely(!dest))
		return NULL;

	grow_string(dest, 1);
	dest->str[dest->len++] = c;
	dest->str[dest->len] = '\0';

	return dest;
}

/**
 * l_string_append_fixed:
 * @dest: growable string object
 * @src: Character array to copy from
 * @max: Maximum number of characters to copy
 *
 * Appends the contents of a fixed size string array @src to @dest.
 * The internal buffer of @dest is grown if necessary.  Up to a maximum of
 * @max characters are copied.  If a null is encountered in the first @max
 * characters, the string is copied only up to the NULL character.
 *
 * Returns: @dest
 **/
LIB_EXPORT struct l_string *l_string_append_fixed(struct l_string *dest,
							const char *src,
							size_t max)
{
	const char *nul;

	if (unlikely(!dest || !src || !max))
		return NULL;

	nul = memchr(src, 0, max);
	if (nul)
		max = nul - src;

	grow_string(dest, max);

	memcpy(dest->str + dest->len, src, max);
	dest->len += max;
	dest->str[dest->len] = '\0';

	return dest;
}

/**
 * l_string_append_vprintf:
 * @dest: growable string object
 * @format: the string format.  See the sprintf() documentation
 * @args: the parameters to insert
 *
 * Appends a formatted string to the growable string buffer.  This function
 * is equivalent to l_string_append_printf except that the arguments are
 * passed as a va_list.
 **/
LIB_EXPORT void l_string_append_vprintf(struct l_string *dest,
					const char *format, va_list args)
{
	size_t len;
	size_t have_space;
	va_list args_copy;

	if (unlikely(!dest))
		return;

#if __STDC_VERSION__ > 199409L
	va_copy(args_copy, args);
#else
	__va_copy(args_copy, args);
#endif

	have_space = dest->max - dest->len;
	len = vsnprintf(dest->str + dest->len, have_space, format, args);

	if (len >= have_space) {
		grow_string(dest, len);
		len = vsprintf(dest->str + dest->len, format, args_copy);
	}

	dest->len += len;

	va_end(args_copy);
}

/**
 * l_string_append_printf:
 * @dest: growable string object
 * @format: the string format.  See the sprintf() documentation
 * @...: the parameters to insert
 *
 * Appends a formatted string to the growable string buffer, growing it as
 * necessary.
 **/
LIB_EXPORT void l_string_append_printf(struct l_string *dest,
					const char *format, ...)
{
	va_list args;

	if (unlikely(!dest))
		return;

	va_start(args, format);
	l_string_append_vprintf(dest, format, args);
	va_end(args);
}

/**
 * l_string_length:
 * @string: growable string object
 *
 * Returns: bytes used in the string.
 **/
LIB_EXPORT unsigned int l_string_length(struct l_string *string)
{
	if (unlikely(!string))
		return 0;

	return string->len;
}

LIB_EXPORT struct l_string *l_string_truncate(struct l_string *string,
							size_t new_size)
{
	if (unlikely(!string))
		return NULL;

	if (new_size >= string->len)
		return string;

	string->len = new_size;
	string->str[new_size] = '\0';

	return string;
}

struct arg {
	size_t max_len;
	size_t cur_len;
	char *chars;
};

static inline void arg_init(struct arg *arg)
{
	arg->max_len = 0;
	arg->cur_len = 0;
	arg->chars = NULL;
}

static void arg_putchar(struct arg *arg, char ch)
{
	if (arg->cur_len == arg->max_len) {
		arg->max_len += 32; /* Grow by at least 32 bytes */
		arg->chars = l_realloc(arg->chars, 1 + arg->max_len);
	}

	arg->chars[arg->cur_len++] = ch;
	arg->chars[arg->cur_len] = '\0';
}

static void arg_putmem(struct arg *arg, const void *mem, size_t len)
{
	if (len == 0)
		return;

	if (arg->cur_len + len > arg->max_len) {
		size_t growby = len * 2;

		if (growby < 32)
			growby = 32;

		arg->max_len += growby;
		arg->chars = l_realloc(arg->chars, 1 + arg->max_len);
	}

	memcpy(arg->chars + arg->cur_len, mem, len);
	arg->cur_len += len;
	arg->chars[arg->cur_len] = '\0';
}

static bool parse_backslash(struct arg *arg, const char *args, size_t *pos)
{
	/* We're at the backslash, not within double quotes */
	char c = args[*pos + 1];

	switch (c) {
	case 0:
		return false;
	case '\n':
		break;
	default:
		arg_putchar(arg, c);
		break;
	}

	*pos += 1;
	return true;
}

static bool parse_quoted_backslash(struct arg *arg,
						const char *args, size_t *pos)
{
	/* We're at the backslash, within double quotes */
	char c = args[*pos + 1];

	switch (c) {
	case 0:
		return false;
	case '\n':
		break;
	case '"':
	case '\\':
		arg_putchar(arg, c);
		break;
	default:
		arg_putchar(arg, '\\');
		arg_putchar(arg, c);
		break;
	}

	*pos += 1;
	return true;
}

static bool parse_single_quote(struct arg *arg, const char *args, size_t *pos)
{
	/* We're just past the single quote */
	size_t start = *pos;

	for (; args[*pos]; *pos += 1) {
		if (args[*pos] != '\'')
			continue;

		arg_putmem(arg, args + start, *pos - start);
		return true;
	}

	/* Unterminated ' */
	return false;
}

static bool parse_double_quote(struct arg *arg, const char *args, size_t *pos)
{
	/* We're just past the double quote */
	for (; args[*pos]; *pos += 1) {
		char c = args[*pos];

		switch (c) {
		case '"':
			return true;
		case '\\':
			if (!parse_quoted_backslash(arg, args, pos))
				return false;

			break;
		default:
			arg_putchar(arg, c);
			break;
		}
	}

	/* Unterminated */
	return false;
}

static void add_arg(char ***args, char *arg, int *n_args)
{
	*args = l_realloc(*args, sizeof(char *) * (2 + *n_args));
	(*args)[*n_args] = arg;
	(*args)[*n_args + 1] = NULL;

	*n_args += 1;
}

LIB_EXPORT char **l_parse_args(const char *args, int *out_n_args)
{
	size_t i;
	struct arg arg;
	char **ret = l_realloc(NULL, sizeof(char *));
	int n_args = 0;

	ret[0] = NULL;
	arg_init(&arg);

	for (i = 0; args[i]; i++) {
		switch (args[i]) {
		case '\\':
			if (!parse_backslash(&arg, args, &i))
				goto error;
			break;
		case '"':
			i += 1;
			if (!parse_double_quote(&arg, args, &i))
				goto error;

			/* Add an empty string */
			if (!arg.cur_len)
				add_arg(&ret, l_strdup(""), &n_args);

			break;
		case '\'':
			i += 1;
			if (!parse_single_quote(&arg, args, &i))
				goto error;

			/* Add an empty string */
			if (!arg.cur_len)
				add_arg(&ret, l_strdup(""), &n_args);

			break;
		default:
			if (!strchr(" \t", args[i])) {
				if (args[i] == '\n')
					goto error;

				arg_putchar(&arg, args[i]);
				continue;
			}

			if (arg.cur_len)
				add_arg(&ret, arg.chars, &n_args);

			arg_init(&arg);
			break;
		}
	}

	if (arg.cur_len)
		add_arg(&ret, arg.chars, &n_args);

	if (out_n_args)
		*out_n_args = n_args;

	return ret;

error:
	l_free(arg.chars);
	l_strfreev(ret);
	return NULL;
}
