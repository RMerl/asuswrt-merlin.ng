/**************************************************************************
 *   chars.c  --  This file is part of GNU nano.                          *
 *                                                                        *
 *   Copyright (C) 2001-2011, 2013-2023 Free Software Foundation, Inc.    *
 *   Copyright (C) 2016-2021 Benno Schulenberg                            *
 *                                                                        *
 *   GNU nano is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published    *
 *   by the Free Software Foundation, either version 3 of the License,    *
 *   or (at your option) any later version.                               *
 *                                                                        *
 *   GNU nano is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty          *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.              *
 *   See the GNU General Public License for more details.                 *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program.  If not, see http://www.gnu.org/licenses/.  *
 *                                                                        *
 **************************************************************************/

#include "prototypes.h"

#include <ctype.h>
#include <string.h>

#ifdef ENABLE_UTF8
#include <wchar.h>
#include <wctype.h>

static bool use_utf8 = FALSE;
		/* Whether we've enabled UTF-8 support. */

/* Enable UTF-8 support. */
void utf8_init(void)
{
	use_utf8 = TRUE;
}

/* Is UTF-8 support enabled? */
bool using_utf8(void)
{
	return use_utf8;
}
#endif /* ENABLE_UTF8 */

#ifdef ENABLE_SPELLER
/* Return TRUE when the given character is some kind of letter. */
bool is_alpha_char(const char *c)
{
#ifdef ENABLE_UTF8
	wchar_t wc;

	if (mbtowide(&wc, c) < 0)
		return FALSE;

	return iswalpha(wc);
#else
	return isalpha((unsigned char)*c);
#endif
}
#endif /* ENABLE_SPELLER */

/* Return TRUE when the given character is some kind of letter or a digit. */
bool is_alnum_char(const char *c)
{
#ifdef ENABLE_UTF8
	wchar_t wc;

	if (mbtowide(&wc, c) < 0)
		return FALSE;

	return iswalnum(wc);
#else
	return isalnum((unsigned char)*c);
#endif
}

/* Return TRUE when the given character is space or tab or other whitespace. */
bool is_blank_char(const char *c)
{
#ifdef ENABLE_UTF8
	wchar_t wc;

	if ((signed char)*c >= 0)
		return (*c == ' ' || *c == '\t');

	if (mbtowide(&wc, c) < 0)
		return FALSE;

	return iswblank(wc);
#else
	return isblank((unsigned char)*c);
#endif
}

/* Return TRUE when the given character is a control character. */
bool is_cntrl_char(const char *c)
{
#ifdef ENABLE_UTF8
	if (use_utf8) {
		return ((c[0] & 0xE0) == 0 || c[0] == DEL_CODE ||
				((signed char)c[0] == -62 && (signed char)c[1] < -96));
	} else
#endif
		return ((*c & 0x60) == 0 || *c == DEL_CODE);
}

/* Return TRUE when the given character is a punctuation character. */
bool is_punct_char(const char *c)
{
#ifdef ENABLE_UTF8
	wchar_t wc;

	if (mbtowide(&wc, c) < 0)
		return FALSE;

	return iswpunct(wc);
#else
	return ispunct((unsigned char)*c);
#endif
}

/* Return TRUE when the given character is word-forming (it is alphanumeric or
 * specified in 'wordchars', or it is punctuation when allow_punct is TRUE). */
bool is_word_char(const char *c, bool allow_punct)
{
	if (*c == '\0')
		return FALSE;

	if (is_alnum_char(c))
		return TRUE;

	if (allow_punct && is_punct_char(c))
		return TRUE;

	if (word_chars != NULL && *word_chars != '\0') {
		char symbol[MAXCHARLEN + 1];
		int symlen = collect_char(c, symbol);

		symbol[symlen] = '\0';
		return (strstr(word_chars, symbol) != NULL);
	} else
		return FALSE;
}

/* Return the visible representation of control character c. */
char control_rep(const signed char c)
{
	if (c == DEL_CODE)
		return '?';
	else if (c == -97)
		return '=';
	else if (c < 0)
		return c + 224;
	else
		return c + 64;
}

/* Return the visible representation of multibyte control character c. */
char control_mbrep(const char *c, bool isdata)
{
	/* An embedded newline is an encoded NUL if it is data. */
	if (*c == '\n' && (isdata || as_an_at))
		return '@';

#ifdef ENABLE_UTF8
	if (use_utf8) {
		if ((unsigned char)c[0] < 128)
			return control_rep(c[0]);
		else
			return control_rep(c[1]);
	} else
#endif
		return control_rep(*c);
}

#ifdef ENABLE_UTF8
/* Convert the given multibyte sequence c to wide character wc, and return
 * the number of bytes in the sequence, or -1 for an invalid sequence. */
int mbtowide(wchar_t *wc, const char *c)
{
	if ((signed char)*c < 0 && use_utf8) {
		unsigned char v1 = (unsigned char)c[0];
		unsigned char v2 = (unsigned char)c[1] ^ 0x80;

		if (v2 > 0x3F || v1 < 0xC2)
			return -1;

		if (v1 < 0xE0) {
			*wc = (((unsigned int)(v1 & 0x1F) << 6) | (unsigned int)v2);
			return 2;
		}

		unsigned char v3 = (unsigned char)c[2] ^ 0x80;

		if (v3 > 0x3F)
			return -1;

		if (v1 < 0xF0) {
			if ((v1 > 0xE0 || v2 >= 0x20) && (v1 != 0xED || v2 < 0x20)) {
				*wc = (((unsigned int)(v1 & 0x0F) << 12) |
							((unsigned int)v2 << 6) | (unsigned int)v3);
				return 3;
			} else
				return -1;
		}

		unsigned char v4 = (unsigned char)c[3] ^ 0x80;

		if (v4 > 0x3F || v1 > 0xF4)
			return -1;

		if ((v1 > 0xF0 || v2 >= 0x10) && (v1 != 0xF4 || v2 < 0x10)) {
			*wc = (((unsigned int)(v1 & 0x07) << 18) | ((unsigned int)v2 << 12) |
							((unsigned int)v3 << 6) | (unsigned int)v4);
			return 4;
		} else
			return -1;
	}

	*wc = (unsigned int)*c;
	return 1;
}

/* Return TRUE when the given character occupies two cells. */
bool is_doublewidth(const char *ch)
{
	wchar_t wc;

	/* Only from U+1100 can code points have double width. */
	if ((unsigned char)*ch < 0xE1 || !use_utf8)
		return FALSE;

	if (mbtowide(&wc, ch) < 0)
		return FALSE;

	return (wcwidth(wc) == 2);
}

/* Return TRUE when the given character occupies zero cells. */
bool is_zerowidth(const char *ch)
{
	wchar_t wc;

	/* Only from U+0300 can code points have zero width. */
	if ((unsigned char)*ch < 0xCC || !use_utf8)
		return FALSE;

	if (mbtowide(&wc, ch) < 0)
		return FALSE;

#if defined(__OpenBSD__)
	/* Work around an OpenBSD bug -- see https://sv.gnu.org/bugs/?60393. */
	if (wc >= 0xF0000)
		return FALSE;
#endif

	return (wcwidth(wc) == 0);
}
#endif /* ENABLE_UTF8 */

/* Return the number of bytes in the character that starts at *pointer. */
int char_length(const char *pointer)
{
#ifdef ENABLE_UTF8
	if ((unsigned char)*pointer > 0xC1 && use_utf8) {
		unsigned char c1 = (unsigned char)pointer[0];
		unsigned char c2 = (unsigned char)pointer[1];

		if ((c2 ^ 0x80) > 0x3F)
			return 1;

		if (c1 < 0xE0)
			return 2;

		if (((unsigned char)pointer[2] ^ 0x80) > 0x3F)
			return 1;

		if (c1 < 0xF0) {
			if ((c1 > 0xE0 || c2 >= 0xA0) && (c1 != 0xED || c2 < 0xA0))
				return 3;
			else
				return 1;
		}

		if (((unsigned char)pointer[3] ^ 0x80) > 0x3F)
			return 1;

		if (c1 > 0xF4)
			return 1;

		if ((c1 > 0xF0 || c2 >= 0x90) && (c1 != 0xF4 || c2 < 0x90))
			return 4;
	}
#endif
		return 1;
}

/* Return the number of (multibyte) characters in the given string. */
size_t mbstrlen(const char *pointer)
{
	size_t count = 0;

	while (*pointer != '\0') {
		pointer += char_length(pointer);
		count++;
	}

	return count;
}

/* Return the length (in bytes) of the character at the start of the
 * given string, and return a copy of this character in *thechar. */
int collect_char(const char *string, char *thechar)
{
	int charlen = char_length(string);

	for (int i = 0; i < charlen; i++)
		thechar[i] = string[i];

	return charlen;
}

/* Return the length (in bytes) of the character at the start of
 * the given string, and add this character's width to *column. */
int advance_over(const char *string, size_t *column)
{
#ifdef ENABLE_UTF8
	if ((signed char)*string < 0 && use_utf8) {
		/* A UTF-8 upper control code has two bytes and takes two columns. */
		if (((unsigned char)string[0] == 0xC2 && (signed char)string[1] < -96)) {
			*column += 2;
			return 2;
		} else {
			wchar_t wc;
			int charlen = mbtowide(&wc, string);

			if (charlen < 0) {
				*column += 1;
				return 1;
			}

			int width = wcwidth(wc);

#if defined(__OpenBSD__)
			*column += (width < 0 || wc >= 0xF0000) ? 1 : width;
#else
			*column += (width < 0) ? 1 : width;
#endif
			return charlen;
		}
	}
#endif

	if ((unsigned char)*string < 0x20) {
		if (*string == '\t')
			*column += tabsize - *column % tabsize;
		else
			*column += 2;
	} else if (0x7E < (unsigned char)*string && (unsigned char)*string < 0xA0)
		*column += 2;
	else
		*column += 1;

	return 1;
}

/* Return the index in buf of the beginning of the multibyte character
 * before the one at pos. */
size_t step_left(const char *buf, size_t pos)
{
#ifdef ENABLE_UTF8
	if (use_utf8) {
		size_t before, charlen = 0;

		if (pos < 4)
			before = 0;
		else {
			const char *ptr = buf + pos;

			/* Probe for a valid starter byte in the preceding four bytes. */
			if ((signed char)*(--ptr) > -65)
				before = pos - 1;
			else if ((signed char)*(--ptr) > -65)
				before = pos - 2;
			else if ((signed char)*(--ptr) > -65)
				before = pos - 3;
			else if ((signed char)*(--ptr) > -65)
				before = pos - 4;
			else
				before = pos - 1;
		}

		/* Move forward again until we reach the original character,
		 * so we know the length of its preceding character. */
		while (before < pos) {
			charlen = char_length(buf + before);
			before += charlen;
		}

		return before - charlen;
	} else
#endif
		return (pos == 0 ? 0 : pos - 1);
}

/* Return the index in buf of the beginning of the multibyte character
 * after the one at pos. */
size_t step_right(const char *buf, size_t pos)
{
	return pos + char_length(buf + pos);
}

/* This function is equivalent to strcasecmp() for multibyte strings. */
int mbstrcasecmp(const char *s1, const char *s2)
{
	return mbstrncasecmp(s1, s2, HIGHEST_POSITIVE);
}

/* This function is equivalent to strncasecmp() for multibyte strings. */
int mbstrncasecmp(const char *s1, const char *s2, size_t n)
{
#ifdef ENABLE_UTF8
	if (use_utf8) {
		wchar_t wc1, wc2;

		while (*s1 != '\0' && *s2 != '\0' && n > 0) {
			if ((signed char)*s1 >= 0 && (signed char)*s2 >= 0) {
				if ('A' <= (*s1 & 0x5F) && (*s1 & 0x5F) <= 'Z') {
					if ('A' <= (*s2 & 0x5F) && (*s2 & 0x5F) <= 'Z') {
						if ((*s1 & 0x5F) != (*s2 & 0x5F))
							return ((*s1 & 0x5F) - (*s2 & 0x5F));
					} else
						return ((*s1 | 0x20) - *s2);
				} else if ('A' <= (*s2 & 0x5F) && (*s2 & 0x5F) <= 'Z')
					return (*s1 - (*s2 | 0x20));
				else if (*s1 != *s2)
					return (*s1 - *s2);

				s1++; s2++; n--;
				continue;
			}

			bool bad1 = (mbtowide(&wc1, s1) < 0);
			bool bad2 = (mbtowide(&wc2, s2) < 0);

			if (bad1 || bad2) {
				if (*s1 != *s2)
					return (unsigned char)*s1 - (unsigned char)*s2;

				if (bad1 != bad2)
					return (bad1 ? 1 : -1);
			} else {
				int difference = towlower(wc1) - towlower(wc2);

				if (difference != 0)
					return difference;
			}

			s1 += char_length(s1);
			s2 += char_length(s2);
			n--;
		}

		return (n > 0) ? ((unsigned char)*s1 - (unsigned char)*s2) : 0;
	} else
#endif
		return strncasecmp(s1, s2, n);
}

/* This function is equivalent to strcasestr() for multibyte strings. */
char *mbstrcasestr(const char *haystack, const char *needle)
{
#ifdef ENABLE_UTF8
	if (use_utf8) {
		size_t needle_len = mbstrlen(needle);

		while (*haystack != '\0') {
			if (mbstrncasecmp(haystack, needle, needle_len) == 0)
				return (char *)haystack;

			haystack += char_length(haystack);
		}

		return NULL;
	} else
#endif
		return (char *)strcasestr(haystack, needle);
}

/* This function is equivalent to strstr(), except in that it scans the
 * string in reverse, starting at pointer. */
char *revstrstr(const char *haystack, const char *needle,
		const char *pointer)
{
	size_t needle_len = strlen(needle);
	size_t tail_len = strlen(pointer);

	if (tail_len < needle_len)
		pointer -= (needle_len - tail_len);

	while (pointer >= haystack) {
		if (strncmp(pointer, needle, needle_len) == 0)
			return (char *)pointer;
		pointer--;
	}

	return NULL;
}

/* This function is equivalent to strcasestr(), except in that it scans
 * the string in reverse, starting at pointer. */
char *revstrcasestr(const char *haystack, const char *needle,
		const char *pointer)
{
	size_t needle_len = strlen(needle);
	size_t tail_len = strlen(pointer);

	if (tail_len < needle_len)
		pointer -= (needle_len - tail_len);

	while (pointer >= haystack) {
		if (strncasecmp(pointer, needle, needle_len) == 0)
			return (char *)pointer;
		pointer--;
	}

	return NULL;
}

/* This function is equivalent to strcasestr() for multibyte strings,
 * except in that it scans the string in reverse, starting at pointer. */
char *mbrevstrcasestr(const char *haystack, const char *needle,
		const char *pointer)
{
#ifdef ENABLE_UTF8
	if (use_utf8) {
		size_t needle_len = mbstrlen(needle);
		size_t tail_len = mbstrlen(pointer);

		if (tail_len < needle_len)
			pointer -= (needle_len - tail_len);

		if (pointer < haystack)
			return NULL;

		while (TRUE) {
			if (mbstrncasecmp(pointer, needle, needle_len) == 0)
				return (char *)pointer;

			if (pointer == haystack)
				return NULL;

			pointer = haystack + step_left(haystack, pointer - haystack);
		}
	} else
#endif
		return revstrcasestr(haystack, needle, pointer);
}

#if !defined(NANO_TINY) || defined(ENABLE_JUSTIFY)
/* This function is equivalent to strchr() for multibyte strings. */
char *mbstrchr(const char *string, const char *chr)
{
#ifdef ENABLE_UTF8
	if (use_utf8) {
		bool bad_s = FALSE, bad_c = FALSE;
		wchar_t ws, wc;

		if (mbtowide(&wc, chr) < 0) {
			wc = (unsigned char)*chr;
			bad_c = TRUE;
		}

		while (*string != '\0') {
			int symlen = mbtowide(&ws, string);

			if (symlen < 0) {
				ws = (unsigned char)*string;
				bad_s = TRUE;
			}

			if (ws == wc && bad_s == bad_c)
				break;

			string += symlen;
		}

		if (*string == '\0')
			return NULL;

		return (char *)string;
	} else
#endif
		return strchr(string, *chr);
}
#endif /* !NANO_TINY || ENABLE_JUSTIFY */

#ifndef NANO_TINY
/* Locate, in the given string, the first occurrence of any of
 * the characters in accept, searching forward. */
char *mbstrpbrk(const char *string, const char *accept)
{
	while (*string != '\0') {
		if (mbstrchr(accept, string) != NULL)
			return (char *)string;

		string += char_length(string);
	}

	return NULL;
}

/* Locate, in the string that starts at head, the first occurrence of any of
 * the characters in accept, starting from pointer and searching backwards. */
char *mbrevstrpbrk(const char *head, const char *accept, const char *pointer)
{
	if (*pointer == '\0') {
		if (pointer == head)
			return NULL;
		pointer = head + step_left(head, pointer - head);
	}

	while (TRUE) {
		if (mbstrchr(accept, pointer) != NULL)
			return (char *)pointer;

		/* If we've reached the head of the string, we found nothing. */
		if (pointer == head)
			return NULL;

		pointer = head + step_left(head, pointer - head);
	}
}
#endif /* !NANO_TINY */

#if defined(ENABLE_NANORC) && (!defined(NANO_TINY) || defined(ENABLE_JUSTIFY))
/* Return TRUE if the given string contains at least one blank character. */
bool has_blank_char(const char *string)
{
	while (*string != '\0' && !is_blank_char(string))
		string += char_length(string);

	return *string;
}
#endif

/* Return TRUE when the given string is empty or consists of only blanks. */
bool white_string(const char *string)
{
	while (*string != '\0' && (is_blank_char(string) || *string == '\r'))
		string += char_length(string);

	return !*string;
}
