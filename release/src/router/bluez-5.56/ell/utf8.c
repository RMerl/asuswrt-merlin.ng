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
#include <wchar.h>

#include "util.h"
#include "strv.h"
#include "utf8.h"
#include "private.h"

/**
 * SECTION:utf8
 * @short_description: UTF-8 utility function
 *
 * UTF-8 string handling support
 */

LIB_EXPORT unsigned char l_ascii_table[256] = {
	[0x00 ... 0x08] = L_ASCII_CNTRL,
	[0x09 ... 0x0D] = L_ASCII_CNTRL | L_ASCII_SPACE,
	[0x0E ... 0x1F] = L_ASCII_CNTRL,
	[0x20]		= L_ASCII_PRINT | L_ASCII_SPACE,
	[0x21 ... 0x2F] = L_ASCII_PRINT | L_ASCII_PUNCT,
	[0x30 ... 0x39] = L_ASCII_DIGIT | L_ASCII_XDIGIT | L_ASCII_PRINT,
	[0x3A ... 0x40] = L_ASCII_PRINT | L_ASCII_PUNCT,
	[0x41 ... 0x46] = L_ASCII_PRINT | L_ASCII_XDIGIT | L_ASCII_UPPER,
	[0x47 ... 0x5A] = L_ASCII_PRINT | L_ASCII_UPPER,
	[0x5B ... 0x60] = L_ASCII_PRINT | L_ASCII_PUNCT,
	[0x61 ... 0x66] = L_ASCII_PRINT | L_ASCII_XDIGIT | L_ASCII_LOWER,
	[0x67 ... 0x7A] = L_ASCII_PRINT | L_ASCII_LOWER,
	[0x7B ... 0x7E] = L_ASCII_PRINT | L_ASCII_PUNCT,
	[0x7F]		= L_ASCII_CNTRL,
	[0x80 ... 0xFF] = 0,
};

static inline bool __attribute__ ((always_inline))
			valid_unicode(wchar_t c)
{
	if (c <= 0xd7ff)
		return true;

	if (c < 0xe000 || c > 0x10ffff)
		return false;

	if (c >= 0xfdd0 && c <= 0xfdef)
		return false;

	if ((c & 0xfffe) == 0xfffe)
		return false;

	return true;
}

/**
 * l_utf8_get_codepoint
 * @str: a pointer to codepoint data
 * @len: maximum bytes to read
 * @cp: destination for codepoint
 *
 * Returns: number of bytes read, or -1 for invalid coddepoint
 **/
LIB_EXPORT int l_utf8_get_codepoint(const char *str, size_t len, wchar_t *cp)
{
	static const wchar_t mins[3] = { 1 << 7, 1 << 11, 1 << 16 };
	unsigned int expect_bytes;
	wchar_t val;
	size_t i;

	if (len == 0)
		return 0;

	if ((signed char) str[0] > 0) {
		*cp = str[0];
		return 1;
	}

	expect_bytes = __builtin_clz(~((unsigned char)str[0] << 24));

	if (expect_bytes < 2 || expect_bytes > 4)
		goto error;

	if (expect_bytes > len)
		goto error;

	val = str[0] & (0xff >> (expect_bytes + 1));

	for (i = 1; i < expect_bytes; i++) {
		if ((str[i] & 0xc0) != 0x80)
			goto error;

		val <<= 6;
		val |= str[i] & 0x3f;
	}

	if (val < mins[expect_bytes - 2])
		goto error;

	if (valid_unicode(val) == false)
		goto error;

	*cp = val;
	return expect_bytes;

error:
	return -1;
}

/**
 * l_utf8_validate:
 * @str: a pointer to character data
 * @len: max bytes to validate
 * @end: return location for end of valid data
 *
 * Validates UTF-8 encoded text. If @end is non-NULL, then the end of
 * the valid range will be stored there (i.e. the start of the first
 * invalid character if some bytes were invalid, or the end of the text
 * being validated otherwise).
 *
 * Returns: Whether the text was valid UTF-8
 **/
LIB_EXPORT bool l_utf8_validate(const char *str, size_t len, const char **end)
{
	size_t pos = 0;
	int ret;
	wchar_t val;

	while (pos < len && str[pos]) {
		ret = l_utf8_get_codepoint(str + pos, len - pos, &val);

		if (ret < 0)
			goto error;

		pos += ret;
	}

error:
	if (end)
		*end = str + pos;

	if (pos != len)
		return false;

	return true;
}

/**
 * l_utf8_strlen:
 * @str: a pointer to character data
 *
 * Computes the number of UTF-8 characters (not bytes) in the string given
 * by @str.
 *
 * Returns: The number of UTF-8 characters in the string
 **/
LIB_EXPORT size_t l_utf8_strlen(const char *str)
{
	size_t l = 0;
	size_t i;
	unsigned char b;

	for (i = 0; str[i]; i++) {
		b = str[i];

		if ((b >> 6) == 2)
			l += 1;
	}

	return i - l;
}

static inline int __attribute__ ((always_inline))
			utf8_length(wchar_t c)
{
	if (c <= 0x7f)
		return 1;

	if (c <= 0x7ff)
		return 2;

	if (c <= 0xffff)
		return 3;

	return 4;
}

static inline uint16_t __attribute__ ((always_inline))
			surrogate_value(uint16_t h, uint16_t l)
{
	return 0x10000 + (h - 0xd800) * 0x400 + l - 0xdc00;
}

/*
 * l_utf8_from_wchar:
 * @c: a wide-character to convert
 * @out_buf: Buffer to write out to
 *
 * Assumes c is valid unicode and out_buf contains enough space for a single
 * utf8 character (maximum 4 bytes)
 * Returns: number of characters written
 */
LIB_EXPORT size_t l_utf8_from_wchar(wchar_t c, char *out_buf)
{
	int len = utf8_length(c);
	int i;

	if (len == 1) {
		out_buf[0] = c;
		return 1;
	}

	for (i = len - 1; i; i--) {
		out_buf[i] = (c & 0x3f) | 0x80;
		c >>= 6;
	}

	out_buf[0] = (0xff << (8 - len)) | c;
	return len;
}

/**
 * l_utf8_from_utf16:
 * @utf16: Array of UTF16 characters
 * @utf16_size: The size of the @utf16 array in bytes.  Must be a multiple of 2.
 *
 * Returns: A newly-allocated buffer containing UTF16 encoded string converted
 * to UTF8.  The UTF8 string will always be null terminated, even if the
 * original UTF16 string was not.
 **/
LIB_EXPORT char *l_utf8_from_utf16(const void *utf16, ssize_t utf16_size)
{
	char *utf8;
	size_t utf8_len = 0;
	wchar_t high_surrogate = 0;
	ssize_t i = 0;
	uint16_t in;
	wchar_t c;

	if (unlikely(utf16_size % 2))
		return NULL;

	while (utf16_size < 0 || i < utf16_size) {
		in = l_get_u16(utf16 + i);

		if (!in)
			break;

		if (in >= 0xdc00 && in < 0xe000) {
			if (high_surrogate)
				c = surrogate_value(high_surrogate, in);
			else
				return NULL;

			high_surrogate = 0;
		} else {
			if (high_surrogate)
				return NULL;

			if (in >= 0xd800 && in < 0xdc00) {
				high_surrogate = in;
				goto next;
			}

			c = in;
		}

		if (!valid_unicode(c))
			return NULL;

		utf8_len += utf8_length(c);
next:
		i += 2;
	}

	if (high_surrogate)
		return NULL;

	utf8 = l_malloc(utf8_len + 1);
	utf8_len = 0;
	i = 0;

	while (utf16_size < 0 || i < utf16_size) {
		in = l_get_u16(utf16 + i);

		if (!in)
			break;

		if (in >= 0xd800 && in < 0xdc00) {
			high_surrogate = in;
			i += 2;
			in = l_get_u16(utf16 + i);
			c = surrogate_value(high_surrogate, in);
		} else
			c = in;

		utf8_len += l_utf8_from_wchar(c, utf8 + utf8_len);
		i += 2;
	}

	utf8[utf8_len] = '\0';

	return utf8;
}

/**
 * l_utf8_to_utf16:
 * @utf8: UTF8 formatted string
 * @out_size: The size in bytes of the converted utf16 string
 *
 * Converts a UTF8 formatted string to UTF16.  It is assumed that the string
 * is valid UTF8 and no sanity checking is performed.
 *
 * Returns: A newly-allocated buffer containing UTF8 encoded string converted
 * to UTF16.  The UTF16 string will always be null terminated.
 **/
LIB_EXPORT void *l_utf8_to_utf16(const char *utf8, size_t *out_size)
{
	const char *c;
	wchar_t wc;
	int len;
	uint16_t *utf16;
	size_t n_utf16;

	if (unlikely(!utf8))
		return NULL;

	c = utf8;
	n_utf16 = 0;

	while (*c) {
		len = l_utf8_get_codepoint(c, 4, &wc);
		if (len < 0)
			return NULL;

		if (wc < 0x10000)
			n_utf16 += 1;
		else
			n_utf16 += 2;

		c += len;
	}

	utf16 = l_malloc((n_utf16 + 1) * 2);
	c = utf8;
	n_utf16 = 0;

	while (*c) {
		len = l_utf8_get_codepoint(c, 4, &wc);

		if (wc >= 0x10000) {
			utf16[n_utf16++] = (wc - 0x1000) / 0x400 + 0xd800;
			utf16[n_utf16++] = (wc - 0x1000) % 0x400 + 0xdc00;
		} else
			utf16[n_utf16++] = wc;

		c += len;
	}

	utf16[n_utf16] = 0;

	if (out_size)
		*out_size = (n_utf16 + 1) * 2;

	return utf16;
}

/**
 * l_utf8_from_ucs2be:
 * @ucs2be: Array of UCS2 characters in big-endian format
 * @ucs2be_size: The size of the @ucs2 array in bytes.  Must be a multiple of 2.
 *
 * Returns: A newly-allocated buffer containing UCS2BE encoded string converted
 * to UTF8.  The UTF8 string will always be null terminated, even if the
 * original UCS2BE string was not.
 **/
LIB_EXPORT char *l_utf8_from_ucs2be(const void *ucs2be, ssize_t ucs2be_size)
{
	char *utf8;
	size_t utf8_len = 0;
	ssize_t i = 0;
	uint16_t in;

	if (unlikely(ucs2be_size % 2))
		return NULL;

	while (ucs2be_size < 0 || i < ucs2be_size) {
		in = l_get_be16(ucs2be + i);

		if (!in)
			break;

		if (in >= 0xd800 && in < 0xe000)
			return NULL;

		if (!valid_unicode(in))
			return NULL;

		utf8_len += utf8_length(in);
		i += 2;
	}

	utf8 = l_malloc(utf8_len + 1);
	utf8_len = 0;
	i = 0;

	while (ucs2be_size < 0 || i < ucs2be_size) {
		in = l_get_be16(ucs2be + i);

		if (!in)
			break;

		utf8_len += l_utf8_from_wchar(in, utf8 + utf8_len);
		i += 2;
	}

	utf8[utf8_len] = '\0';

	return utf8;
}

/**
 * l_utf8_to_ucs2be:
 * @utf8: UTF8 formatted string
 * @out_size: The size in bytes of the converted ucs2be string
 *
 * Converts a UTF8 formatted string to UCS2BE.  It is assumed that the string
 * is valid UTF8 and no sanity checking is performed.
 *
 * Returns: A newly-allocated buffer containing UTF8 encoded string converted
 * to UCS2BE.  The UCS2BE string will always be null terminated.
 **/
LIB_EXPORT void *l_utf8_to_ucs2be(const char *utf8, size_t *out_size)
{
	const char *c;
	wchar_t wc;
	int len;
	uint16_t *ucs2be;
	size_t n_ucs2be;

	if (unlikely(!utf8))
		return NULL;

	c = utf8;
	n_ucs2be = 0;

	while (*c) {
		len = l_utf8_get_codepoint(c, 4, &wc);
		if (len < 0)
			return NULL;

		if (wc >= 0x10000)
			return NULL;

		n_ucs2be += 1;
		c += len;
	}

	ucs2be = l_malloc((n_ucs2be + 1) * 2);
	c = utf8;
	n_ucs2be = 0;

	while (*c) {
		len = l_utf8_get_codepoint(c, 4, &wc);
		ucs2be[n_ucs2be++] = L_CPU_TO_BE16(wc);
		c += len;
	}

	ucs2be[n_ucs2be] = 0;

	if (out_size)
		*out_size = (n_ucs2be + 1) * 2;

	return ucs2be;
}
