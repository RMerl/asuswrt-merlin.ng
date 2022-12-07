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

#ifndef __ELL_UTF8_H
#define __ELL_UTF8_H

#include <stdbool.h>
#include <wchar.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char l_ascii_table[];

enum l_ascii {
	L_ASCII_CNTRL	= 0x80,
	L_ASCII_PRINT	= 0x40,
	L_ASCII_PUNCT	= 0x20,
	L_ASCII_SPACE	= 0x10,
	L_ASCII_XDIGIT	= 0x08,
	L_ASCII_UPPER	= 0x04,
	L_ASCII_LOWER	= 0x02,
	L_ASCII_DIGIT	= 0x01,
	L_ASCII_ALPHA	= L_ASCII_LOWER | L_ASCII_UPPER,
	L_ASCII_ALNUM	= L_ASCII_ALPHA | L_ASCII_DIGIT,
	L_ASCII_GRAPH	= L_ASCII_ALNUM | L_ASCII_PUNCT,
};

#define l_ascii_isalnum(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_ALNUM) != 0)

#define l_ascii_isalpha(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_ALPHA) != 0)

#define l_ascii_iscntrl(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_CNTRL) != 0)

#define l_ascii_isdigit(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_DIGIT) != 0)

#define l_ascii_isgraph(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_GRAPH) != 0)

#define l_ascii_islower(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_LOWER) != 0)

#define l_ascii_isprint(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_PRINT) != 0)

#define l_ascii_ispunct(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_PUNCT) != 0)

#define l_ascii_isspace(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_SPACE) != 0)

#define l_ascii_isupper(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_UPPER) != 0)

#define l_ascii_isxdigit(c) \
	((l_ascii_table[(unsigned char) (c)] & L_ASCII_XDIGIT) != 0)

#if __STDC_VERSION__ <= 199409L
#define inline __inline__
#endif

static inline __attribute__ ((always_inline))
					bool l_ascii_isblank(unsigned char c)
{
	if (c == ' ' || c == '\t')
		return true;

	return false;
}

static inline __attribute__ ((always_inline)) bool l_ascii_isascii(int c)
{
	if (c <= 127)
		return true;

	return false;
}

bool l_utf8_validate(const char *src, size_t len, const char **end);
size_t l_utf8_strlen(const char *str);

int l_utf8_get_codepoint(const char *str, size_t len, wchar_t *cp);
size_t l_utf8_from_wchar(wchar_t c, char *out_buf);

char *l_utf8_from_utf16(const void *utf16, ssize_t utf16_size);
void *l_utf8_to_utf16(const char *utf8, size_t *out_size);

char *l_utf8_from_ucs2be(const void *ucs2be, ssize_t ucs2be_size);
void *l_utf8_to_ucs2be(const char *utf8, size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_UTF8_H */
