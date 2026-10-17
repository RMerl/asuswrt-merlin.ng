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

/*
 * codepage_437 - Unicode to codepage 437 translation table
 */
extern const u16 codepage_437[128];

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
 * @a:			utf-8 string
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
 * @d:			destination buffer
 * @s:			source buffer
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
 * @a:			utf-16 string
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
 * @d:			destination buffer
 * @s:			source buffer
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
 * u16_strcasecmp() - compare two u16 strings case insensitively
 *
 * @s1:		first string to compare
 * @s2:		second string to compare
 * Return:	0  if the first n u16 are the same in s1 and s2
 *		< 0 if the first different u16 in s1 is less than the
 *		corresponding u16 in s2
 *		> 0 if the first different u16 in s1 is greater than the
 */
int u16_strcasecmp(const u16 *s1, const u16 *s2);

/**
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
int u16_strncmp(const u16 *s1, const u16 *s2, size_t n);

/**
 * u16_strcmp() - compare two u16 string
 *
 * @s1:		first string to compare
 * @s2:		second string to compare
 * Return:	0  if the first n u16 are the same in s1 and s2
 *		< 0 if the first different u16 in s1 is less than the
 *		corresponding u16 in s2
 *		> 0 if the first different u16 in s1 is greater than the
 *		corresponding u16 in s2
 */
#define u16_strcmp(s1, s2)	u16_strncmp((s1), (s2), SIZE_MAX)

/**
 * u16_strsize() - count size of u16 string in bytes including the null
 *		   character
 *
 * Counts the number of bytes occupied by a u16 string
 *
 * @in:			null terminated u16 string
 * Return:		bytes in a u16 string
 */
size_t u16_strsize(const void *in);

/**
 * u16_strnlen() - count non-zero words
 *
 * This function matches wscnlen_s() if the -fshort-wchar compiler flag is set.
 * In the EFI context we explicitly need a function handling u16 strings.
 *
 * @in:			null terminated u16 string
 * @count:		maximum number of words to count
 * Return:		number of non-zero words.
 *			This is not the number of utf-16 letters!
 */
size_t u16_strnlen(const u16 *in, size_t count);

/**
 * u16_strlen - count non-zero words
 *
 * This function matches wsclen() if the -fshort-wchar compiler flag is set.
 * In the EFI context we explicitly need a function handling u16 strings.
 *
 * @in:			null terminated u16 string
 * Return:		number of non-zero words.
 *			This is not the number of utf-16 letters!
 */
size_t u16_strlen(const void *in);

#define u16_strlen(in) u16_strnlen(in, SIZE_MAX)

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
u16 *u16_strdup(const void *src);

/**
 * u16_strlcat() - Append a length-limited, %NUL-terminated string to another
 *
 * Append the source string @src to the destination string @dest, overwriting
 * null word at the end of @dest adding  a terminating null word.
 *
 * @dest:		zero terminated u16 destination string
 * @src:		zero terminated u16 source string
 * @count:		size of buffer in u16 words including taling 0x0000
 * Return:		required size including trailing 0x0000 in u16 words
 *			If return value >= count, truncation occurred.
 */
size_t u16_strlcat(u16 *dest, const u16 *src, size_t count);

/**
 * utf16_to_utf8() - Convert an utf16 string to utf8
 *
 * Converts 'size' characters of the utf16 string 'src' to utf8
 * written to the 'dest' buffer.
 *
 * NOTE that a single utf16 character can generate up to 3 utf8
 * characters.  See MAX_UTF8_PER_UTF16.
 *
 * @dest:	the destination buffer to write the utf8 characters
 * @src:	the source utf16 string
 * @size:	the number of utf16 characters to convert
 * Return:	the pointer to the first unwritten byte in 'dest'
 */
uint8_t *utf16_to_utf8(uint8_t *dest, const uint16_t *src, size_t size);

/**
 * utf_to_cp() - translate Unicode code point to 8bit codepage
 *
 * Codepoints that do not exist in the codepage are rendered as question mark.
 *
 * @c:		pointer to Unicode code point to be translated
 * @codepage:	Unicode to codepage translation table
 * Return:	0 on success, -ENOENT if codepoint cannot be translated
 */
int utf_to_cp(s32 *c, const u16 *codepage);

/**
 * utf8_to_cp437_stream() - convert UTF-8 stream to codepage 437
 *
 * @c:		next UTF-8 character to convert
 * @buffer:	buffer, at least 5 characters
 * Return:	next codepage 437 character or 0
 */
int utf8_to_cp437_stream(u8 c, char *buffer);

/**
 * utf8_to_utf32_stream() - convert UTF-8 byte stream to Unicode code points
 *
 * The function is called for each byte @c in a UTF-8 stream. The byte is
 * appended to the temporary storage @buffer until the UTF-8 stream in
 * @buffer describes a Unicode code point.
 *
 * When a new code point has been decoded it is returned and buffer[0] is
 * set to '\0', otherwise the return value is 0.
 *
 * The buffer must be at least 5 characters long. Before the first function
 * invocation buffer[0] must be set to '\0'."
 *
 * @c:		next UTF-8 character to convert
 * @buffer:	buffer, at least 5 characters
 * Return:	Unicode code point or 0
 */
int utf8_to_utf32_stream(u8 c, char *buffer);

#endif /* __CHARSET_H_ */
