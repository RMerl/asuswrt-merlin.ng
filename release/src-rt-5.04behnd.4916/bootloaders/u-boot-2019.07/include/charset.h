/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  charset conversion utils
 *
 *  Copyright (c) 2017 Rob Clark
 */

#ifndef __CHARSET_H_
#define __CHARSET_H_

#include <linux/kernel.h>
#include <linux/types.h>

#define MAX_UTF8_PER_UTF16 3

/**
 * console_read_unicode() - read Unicode code point from console
 *
 * @code:	pointer to store Unicode code point
 * Return:	0 = success
 */
int console_read_unicode(s32 *code);

/**
 * utf8_get() - get next UTF-8 code point from buffer
 *
 * @src:		pointer to current byte, updated to point to next byte
 * Return:		code point, or 0 for end of string, or -1 if no legal
 *			code point is found. In case of an error src points to
 *			the incorrect byte.
 */
s32 utf8_get(const char **src);

/**
 * utf8_put() - write UTF-8 code point to buffer
 *
 * @code:		code point
 * @dst:		pointer to destination buffer, updated to next position
 * Return:		-1 if the input parameters are invalid
 */
int utf8_put(s32 code, char **dst);

/**
 * utf8_utf16_strnlen() - length of a truncated utf-8 string after conversion
 *			  to utf-16
 *
 * @src:		utf-8 string
 * @count:		maximum number of code points to convert
 * Return:		length in u16 after conversion to utf-16 without the
 *			trailing \0. If an invalid UTF-8 sequence is hit one
 *			u16 will be reserved for a replacement character.
 */
size_t utf8_utf16_strnlen(const char *src, size_t count);

/**
 * utf8_utf16_strlen() - length of a utf-8 string after conversion to utf-16
 *
 * @src:		utf-8 string
 * Return:		length in u16 after conversion to utf-16 without the
 *			trailing \0. If an invalid UTF-8 sequence is hit one
 *			u16 will be reserved for a replacement character.
 */
#define utf8_utf16_strlen(a) utf8_utf16_strnlen((a), SIZE_MAX)

/**
 * utf8_utf16_strncpy() - copy utf-8 string to utf-16 string
 *
 * @dst:		destination buffer
 * @src:		source buffer
 * @count:		maximum number of code points to copy
 * Return:		-1 if the input parameters are invalid
 */
int utf8_utf16_strncpy(u16 **dst, const char *src, size_t count);

/**
 * utf8_utf16_strcpy() - copy utf-8 string to utf-16 string
 *
 * @dst:		destination buffer
 * @src:		source buffer
 * Return:		-1 if the input parameters are invalid
 */
#define utf8_utf16_strcpy(d, s) utf8_utf16_strncpy((d), (s), SIZE_MAX)

/**
 * utf16_get() - get next UTF-16 code point from buffer
 *
 * @src:		pointer to current word, updated to point to next word
 * Return:		code point, or 0 for end of string, or -1 if no legal
 *			code point is found. In case of an error src points to
 *			the incorrect word.
 */
s32 utf16_get(const u16 **src);

/**
 * utf16_put() - write UTF-16 code point to buffer
 *
 * @code:		code point
 * @dst:		pointer to destination buffer, updated to next position
 * Return:		-1 if the input parameters are invalid
 */
int utf16_put(s32 code, u16 **dst);

/**
 * utf16_strnlen() - length of a truncated utf-16 string
 *
 * @src:		utf-16 string
 * @count:		maximum number of code points to convert
 * Return:		length in code points. If an invalid UTF-16 sequence is
 *			hit one position will be reserved for a replacement
 *			character.
 */
size_t utf16_strnlen(const u16 *src, size_t count);

/**
 * utf16_utf8_strnlen() - length of a truncated utf-16 string after conversion
 *			  to utf-8
 *
 * @src:		utf-16 string
 * @count:		maximum number of code points to convert
 * Return:		length in bytes after conversion to utf-8 without the
 *			trailing \0. If an invalid UTF-16 sequence is hit one
 *			byte will be reserved for a replacement character.
 */
size_t utf16_utf8_strnlen(const u16 *src, size_t count);

/**
 * utf16_utf8_strlen() - length of a utf-16 string after conversion to utf-8
 *
 * @src:		utf-16 string
 * Return:		length in bytes after conversion to utf-8 without the
 *			trailing \0. If an invalid UTF-16 sequence is hit one
 *			byte will be reserved for a replacement character.
 */
#define utf16_utf8_strlen(a) utf16_utf8_strnlen((a), SIZE_MAX)

/**
 * utf16_utf8_strncpy() - copy utf-16 string to utf-8 string
 *
 * @dst:		destination buffer
 * @src:		source buffer
 * @count:		maximum number of code points to copy
 * Return:		-1 if the input parameters are invalid
 */
int utf16_utf8_strncpy(char **dst, const u16 *src, size_t count);

/**
 * utf16_utf8_strcpy() - copy utf-16 string to utf-8 string
 *
 * @dst:		destination buffer
 * @src:		source buffer
 * Return:		-1 if the input parameters are invalid
 */
#define utf16_utf8_strcpy(d, s) utf16_utf8_strncpy((d), (s), SIZE_MAX)

/**
 * utf_to_lower() - convert a Unicode letter to lower case
 *
 * @code:		letter to convert
 * Return:		lower case letter or unchanged letter
 */
s32 utf_to_lower(const s32 code);

/**
 * utf_to_upper() - convert a Unicode letter to upper case
 *
 * @code:		letter to convert
 * Return:		upper case letter or unchanged letter
 */
s32 utf_to_upper(const s32 code);

/**
 * u16_strlen - count non-zero words
 *
 * This function matches wsclen() if the -fshort-wchar compiler flag is set.
 * In the EFI context we explicitly need a function handling u16 strings.
 *
 * @in:			null terminated u16 string
 * ReturnValue:		number of non-zero words.
 *			This is not the number of utf-16 letters!
 */
size_t u16_strlen(const u16 *in);

/**
 * u16_strlen - count non-zero words
 *
 * This function matches wscnlen_s() if the -fshort-wchar compiler flag is set.
 * In the EFI context we explicitly need a function handling u16 strings.
 *
 * @in:			null terminated u16 string
 * @count:		maximum number of words to count
 * ReturnValue:		number of non-zero words.
 *			This is not the number of utf-16 letters!
 */
size_t u16_strnlen(const u16 *in, size_t count);

/**
 * u16_strcpy() - copy u16 string
 *
 * Copy u16 string pointed to by src, including terminating null word, to
 * the buffer pointed to by dest.
 *
 * @dest:		destination buffer
 * @src:		source buffer (null terminated)
 * Return:		'dest' address
 */
u16 *u16_strcpy(u16 *dest, const u16 *src);

/**
 * u16_strdup() - duplicate u16 string
 *
 * Copy u16 string pointed to by src, including terminating null word, to a
 * newly allocated buffer.
 *
 * @src:		source buffer (null terminated)
 * Return:		allocated new buffer on success, NULL on failure
 */
u16 *u16_strdup(const u16 *src);

/**
 * utf16_to_utf8() - Convert an utf16 string to utf8
 *
 * Converts 'size' characters of the utf16 string 'src' to utf8
 * written to the 'dest' buffer.
 *
 * NOTE that a single utf16 character can generate up to 3 utf8
 * characters.  See MAX_UTF8_PER_UTF16.
 *
 * @dest   the destination buffer to write the utf8 characters
 * @src    the source utf16 string
 * @size   the number of utf16 characters to convert
 * @return the pointer to the first unwritten byte in 'dest'
 */
uint8_t *utf16_to_utf8(uint8_t *dest, const uint16_t *src, size_t size);

#endif /* __CHARSET_H_ */
