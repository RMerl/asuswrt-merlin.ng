// SPDX-License-Identifier: GPL-2.0+
/*
 *  charset conversion utils
 *
 *  Copyright (c) 2017 Rob Clark
 */

#include <charset.h>
#include <capitalization.h>
#include <cp437.h>
#include <efi_loader.h>
#include <errno.h>
#include <malloc.h>

/**
 * codepage_437 - Unicode to codepage 437 translation table
 */
const u16 codepage_437[128] = CP437;

static struct capitalization_table capitalization_table[] =
#ifdef CONFIG_EFI_UNICODE_CAPITALIZATION
	UNICODE_CAPITALIZATION_TABLE;
#elif CONFIG_FAT_DEFAULT_CODEPAGE == 1250
	CP1250_CAPITALIZATION_TABLE;
#else
	CP437_CAPITALIZATION_TABLE;
#endif

/**
 * get_code() - read Unicode code point from UTF-8 stream
 *
 * @read_u8:	- stream reader
 * @src:	- string buffer passed to stream reader, optional
 * Return:	- Unicode code point, or -1
 */
static int get_code(u8 (*read_u8)(void *data), void *data)
{
	s32 ch = 0;

	ch = read_u8(data);
	if (!ch)
		return 0;
	if (ch >= 0xc2 && ch <= 0xf4) {
		int code = 0;

		if (ch >= 0xe0) {
			if (ch >= 0xf0) {
				/* 0xf0 - 0xf4 */
				ch &= 0x07;
				code = ch << 18;
				ch = read_u8(data);
				if (ch < 0x80 || ch > 0xbf)
					goto error;
				ch &= 0x3f;
			} else {
				/* 0xe0 - 0xef */
				ch &= 0x0f;
			}
			code += ch << 12;
			if ((code >= 0xD800 && code <= 0xDFFF) ||
			    code >= 0x110000)
				goto error;
			ch = read_u8(data);
			if (ch < 0x80 || ch > 0xbf)
				goto error;
		}
		/* 0xc0 - 0xdf or continuation byte (0x80 - 0xbf) */
		ch &= 0x3f;
		code += ch << 6;
		ch = read_u8(data);
		if (ch < 0x80 || ch > 0xbf)
			goto error;
		ch &= 0x3f;
		ch += code;
	} else if (ch >= 0x80) {
		goto error;
	}
	return ch;
error:
	return -1;
}

/**
 * read_string() - read byte from character string
 *
 * @data:	- pointer to string
 * Return:	- byte read
 *
 * The string pointer is incremented if it does not point to '\0'.
 */
static u8 read_string(void *data)

{
	const char **src = (const char **)data;
	u8 c;

	if (!src || !*src || !**src)
		return 0;
	c = **src;
	++*src;
	return c;
}

/**
 * read_console() - read byte from console
 *
 * @data	- not used, needed to match interface
 * Return:	- byte read or 0 on error
 */
static u8 read_console(void *data)
{
	int ch;

	ch = getchar();
	if (ch < 0)
		ch = 0;
	return ch;
}

int console_read_unicode(s32 *code)
{
	for (;;) {
		s32 c;

		if (!tstc()) {
			/* No input available */
			return 1;
		}

		/* Read Unicode code */
		c = get_code(read_console, NULL);
		if (c > 0) {
			*code = c;
			return 0;
		}
	}
}

s32 utf8_get(const char **src)
{
	return get_code(read_string, src);
}

int utf8_put(s32 code, char **dst)
{
	if (!dst || !*dst)
		return -1;
	if ((code >= 0xD800 && code <= 0xDFFF) || code >= 0x110000)
		return -1;
	if (code <= 0x007F) {
		**dst = code;
	} else {
		if (code <= 0x07FF) {
			**dst = code >> 6 | 0xC0;
		} else {
			if (code < 0x10000) {
				**dst = code >> 12 | 0xE0;
			} else {
				**dst = code >> 18 | 0xF0;
				++*dst;
				**dst = (code >> 12 & 0x3F) | 0x80;
			}
			++*dst;
			**dst = (code >> 6 & 0x3F) | 0x80;
		}
		++*dst;
		**dst = (code & 0x3F) | 0x80;
	}
	++*dst;
	return 0;
}

size_t utf8_utf16_strnlen(const char *src, size_t count)
{
	size_t len = 0;

	for (; *src && count; --count)  {
		s32 code = utf8_get(&src);

		if (!code)
			break;
		if (code < 0) {
			/* Reserve space for a replacement character */
			len += 1;
		} else if (code < 0x10000) {
			len += 1;
		} else {
			len += 2;
		}
	}
	return len;
}

int utf8_utf16_strncpy(u16 **dst, const char *src, size_t count)
{
	if (!src || !dst || !*dst)
		return -1;

	for (; count && *src; --count) {
		s32 code = utf8_get(&src);

		if (code < 0)
			code = '?';
		utf16_put(code, dst);
	}
	**dst = 0;
	return 0;
}

s32 utf16_get(const u16 **src)
{
	s32 code, code2;

	if (!src || !*src)
		return -1;
	if (!**src)
		return 0;
	code = **src;
	++*src;
	if (code >= 0xDC00 && code <= 0xDFFF)
		return -1;
	if (code >= 0xD800 && code <= 0xDBFF) {
		if (!**src)
			return -1;
		code &= 0x3ff;
		code <<= 10;
		code += 0x10000;
		code2 = **src;
		++*src;
		if (code2 <= 0xDC00 || code2 >= 0xDFFF)
			return -1;
		code2 &= 0x3ff;
		code += code2;
	}
	return code;
}

int utf16_put(s32 code, u16 **dst)
{
	if (!dst || !*dst)
		return -1;
	if ((code >= 0xD800 && code <= 0xDFFF) || code >= 0x110000)
		return -1;
	if (code < 0x10000) {
		**dst = code;
	} else {
		code -= 0x10000;
		**dst = code >> 10 | 0xD800;
		++*dst;
		**dst = (code & 0x3ff) | 0xDC00;
	}
	++*dst;
	return 0;
}

size_t utf16_strnlen(const u16 *src, size_t count)
{
	size_t len = 0;

	for (; *src && count; --count)  {
		s32 code = utf16_get(&src);

		if (!code)
			break;
		/*
		 * In case of an illegal sequence still reserve space for a
		 * replacement character.
		 */
		++len;
	}
	return len;
}

size_t utf16_utf8_strnlen(const u16 *src, size_t count)
{
	size_t len = 0;

	for (; *src && count; --count)  {
		s32 code = utf16_get(&src);

		if (!code)
			break;
		if (code < 0)
			/* Reserve space for a replacement character */
			len += 1;
		else if (code < 0x80)
			len += 1;
		else if (code < 0x800)
			len += 2;
		else if (code < 0x10000)
			len += 3;
		else
			len += 4;
	}
	return len;
}

int utf16_utf8_strncpy(char **dst, const u16 *src, size_t count)
{
	if (!src || !dst || !*dst)
		return -1;

	for (; count && *src; --count) {
		s32 code = utf16_get(&src);

		if (code < 0)
			code = '?';
		utf8_put(code, dst);
	}
	**dst = 0;
	return 0;
}

s32 utf_to_lower(const s32 code)
{
	struct capitalization_table *pos = capitalization_table;
	s32 ret = code;

	if (code <= 0x7f) {
		if (code >= 'A' && code <= 'Z')
			ret += 0x20;
		return ret;
	}
	for (; pos->upper; ++pos) {
		if (pos->upper == code) {
			ret = pos->lower;
			break;
		}
	}
	return ret;
}

s32 utf_to_upper(const s32 code)
{
	struct capitalization_table *pos = capitalization_table;
	s32 ret = code;

	if (code <= 0x7f) {
		if (code >= 'a' && code <= 'z')
			ret -= 0x20;
		return ret;
	}
	for (; pos->lower; ++pos) {
		if (pos->lower == code) {
			ret = pos->upper;
			break;
		}
	}
	return ret;
}

/*
 * u16_strcasecmp() - compare two u16 strings case insensitively
 *
 * @s1:		first string to compare
 * @s2:		second string to compare
 * @n:		maximum number of u16 to compare
 * Return:	0  if the first n u16 are the same in s1 and s2
 *		< 0 if the first different u16 in s1 is less than the
 *		corresponding u16 in s2
 *		> 0 if the first different u16 in s1 is greater than the
 */
int u16_strcasecmp(const u16 *s1, const u16 *s2)
{
	int ret = 0;
	s32 c1, c2;

	for (;;) {
		c1 = utf_to_upper(utf16_get(&s1));
		c2 = utf_to_upper(utf16_get(&s2));
		ret = c1 - c2;
		if (ret || !c1 || c1 == -1 || c2 == -1)
			break;
	}
	return ret;
}

/*
 * u16_strncmp() - compare two u16 string
 *
 * @s1:		first string to compare
 * @s2:		second string to compare
 * @n:		maximum number of u16 to compare
 * Return:	0  if the first n u16 are the same in s1 and s2
 *		< 0 if the first different u16 in s1 is less than the
 *		corresponding u16 in s2
 *		> 0 if the first different u16 in s1 is greater than the
 *		corresponding u16 in s2
 */
int u16_strncmp(const u16 *s1, const u16 *s2, size_t n)
{
	int ret = 0;

	for (; n; --n, ++s1, ++s2) {
		ret = *s1 - *s2;
		if (ret || !*s1)
			break;
	}

	return ret;
}

size_t __efi_runtime u16_strnlen(const u16 *in, size_t count)
{
	size_t i;
	for (i = 0; count-- && in[i]; i++);
	return i;
}

size_t u16_strsize(const void *in)
{
	return (u16_strlen(in) + 1) * sizeof(u16);
}

u16 *u16_strcpy(u16 *dest, const u16 *src)
{
	u16 *tmp = dest;

	for (;; dest++, src++) {
		*dest = *src;
		if (!*src)
			break;
	}

	return tmp;
}

u16 *u16_strdup(const void *src)
{
	u16 *new;
	size_t len;

	if (!src)
		return NULL;
	len = u16_strsize(src);
	new = malloc(len);
	if (!new)
		return NULL;
	memcpy(new, src, len);

	return new;
}

size_t u16_strlcat(u16 *dest, const u16 *src, size_t count)
{
	size_t destlen = u16_strnlen(dest, count);
	size_t srclen = u16_strlen(src);
	size_t ret = destlen + srclen;

	if (destlen >= count)
		return ret;
	if (ret >= count)
		srclen -= (ret - count + 1);
	memcpy(&dest[destlen], src, 2 * srclen);
	dest[destlen + srclen] = 0x0000;

	return ret;
}

/* Convert UTF-16 to UTF-8.  */
uint8_t *utf16_to_utf8(uint8_t *dest, const uint16_t *src, size_t size)
{
	uint32_t code_high = 0;

	while (size--) {
		uint32_t code = *src++;

		if (code_high) {
			if (code >= 0xDC00 && code <= 0xDFFF) {
				/* Surrogate pair.  */
				code = ((code_high - 0xD800) << 10) + (code - 0xDC00) + 0x10000;

				*dest++ = (code >> 18) | 0xF0;
				*dest++ = ((code >> 12) & 0x3F) | 0x80;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			} else {
				/* Error...  */
				*dest++ = '?';
				/* *src may be valid. Don't eat it.  */
				src--;
			}

			code_high = 0;
		} else {
			if (code <= 0x007F) {
				*dest++ = code;
			} else if (code <= 0x07FF) {
				*dest++ = (code >> 6) | 0xC0;
				*dest++ = (code & 0x3F) | 0x80;
			} else if (code >= 0xD800 && code <= 0xDBFF) {
				code_high = code;
				continue;
			} else if (code >= 0xDC00 && code <= 0xDFFF) {
				/* Error... */
				*dest++ = '?';
			} else if (code < 0x10000) {
				*dest++ = (code >> 12) | 0xE0;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			} else {
				*dest++ = (code >> 18) | 0xF0;
				*dest++ = ((code >> 12) & 0x3F) | 0x80;
				*dest++ = ((code >> 6) & 0x3F) | 0x80;
				*dest++ = (code & 0x3F) | 0x80;
			}
		}
	}

	return dest;
}

int utf_to_cp(s32 *c, const u16 *codepage)
{
	if (*c >= 0x80) {
		int j;

		/* Look up codepage translation */
		for (j = 0; j < 0x80; ++j) {
			if (*c == codepage[j]) {
				*c = j + 0x80;
				return 0;
			}
		}
		*c = '?';
		return -ENOENT;
	}
	return 0;
}

int utf8_to_cp437_stream(u8 c, char *buffer)
{
	char *end;
	const char *pos;
	s32 s;
	int ret;

	for (;;) {
		pos = buffer;
		end = buffer + strlen(buffer);
		*end++ = c;
		*end = 0;
		s = utf8_get(&pos);
		if (s > 0) {
			*buffer = 0;
			ret = utf_to_cp(&s, codepage_437);
			return s;
			}
		if (pos == end)
			return 0;
		*buffer = 0;
	}
}

int utf8_to_utf32_stream(u8 c, char *buffer)
{
	char *end;
	const char *pos;
	s32 s;

	for (;;) {
		pos = buffer;
		end = buffer + strlen(buffer);
		*end++ = c;
		*end = 0;
		s = utf8_get(&pos);
		if (s > 0) {
			*buffer = 0;
			return s;
		}
		if (pos == end)
			return 0;
		/*
		 * Appending the byte lead to an invalid UTF-8 byte sequence.
		 * Consider it as the start of a new code sequence.
		 */
		*buffer = 0;
	}
}
