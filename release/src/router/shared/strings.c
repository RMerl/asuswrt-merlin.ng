/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <bcmnvram.h>
#include "shutils.h"
#include "shared.h"

#if defined(RTCONFIG_UTF8_SSID)
int is_utf8(const char * string)
{
	if(!string)
		return 0;

	const unsigned char * bytes = (const unsigned char *)string;
	while(*bytes)
		{
		if( (// ASCII
			 (0x20 <= bytes[0] && bytes[0] <= 0x7F)
			// use bytes[0] <= 0x7F to allow ASCII control characters
			//bytes[0] == 0x09 ||
			//bytes[0] == 0x0A ||
			//bytes[0] == 0x0D ||
			//(0x20 <= bytes[0] && bytes[0] <= 0x7E)
		)
		) {
			bytes += 1;
			continue;
		}

		if( (// non-overlong 2-byte
			(0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
			(0x80 <= bytes[1] && bytes[1] <= 0xBF)
		)
		) {
			bytes += 2;
			continue;
		}

		if( (// excluding overlongs
			bytes[0] == 0xE0 &&
			(0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
			(0x80 <= bytes[2] && bytes[2] <= 0xBF)
			) ||
			(// straight 3-byte
			((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
			bytes[0] == 0xEE ||
			bytes[0] == 0xEF) &&
			(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
			(0x80 <= bytes[2] && bytes[2] <= 0xBF)
			) ||
			(// excluding surrogates
			bytes[0] == 0xED &&
			(0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
			(0x80 <= bytes[2] && bytes[2] <= 0xBF)
		)
		) {
			bytes += 3;
			continue;
		}

		if( (// planes 1-3
			bytes[0] == 0xF0 &&
			(0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
			(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
			(0x80 <= bytes[3] && bytes[3] <= 0xBF)
			) ||
			(// planes 4-15
			(0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
			(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
			(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
			(0x80 <= bytes[3] && bytes[3] <= 0xBF)
			) ||
			(// plane 16
			bytes[0] == 0xF4 &&
			(0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
			(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
			(0x80 <= bytes[3] && bytes[3] <= 0xBF)
		)
		) {
			bytes += 4;
			continue;
		}
		return 0;
	}
	return 1;
}

/* Transfer Char to UTF-8 */
int char_to_ascii_safe_with_utf8(const char *output, const char *input, int outsize)
{
	char *src = (char *)input;
	char *dst = (char *)output;
	char *end = (char *)output + outsize - 1;

	if (src == NULL || dst == NULL || outsize <= 0)
		return 0;

	for ( ; *src && dst < end; src++) {
		if ((*src >='0' && *src <='9') ||
		    (*src >='A' && *src <='Z') ||
		    (*src >='a' && *src <='z')) {
			*dst++ = *src;
		} else {
			if (dst + 3 > end)
				break;
			if( (unsigned char)*src >= 32 && (unsigned char)*src <= 127 )
			{
				dst += sprintf(dst, "%%%.02X", (unsigned char)*src);
			}else if(// non-overlong 2-byte
				(0xC2 <= (unsigned char)*src && (unsigned char)*src <= 0xDF) &&
				(0x80 <= (unsigned char)*(src+1) && (unsigned char)*(src+1) <= 0xBF)
			){
				dst += sprintf(dst, "%%%.02X", (unsigned char)*src);
				dst += sprintf(dst, "%%%.02X", (unsigned char)*(src+1));
				src += 1;
			}else if( (// excluding overlongs
					(unsigned char)*src == 0xE0 &&
					(0xA0 <= (unsigned char)*(src+1)&& (unsigned char)*(src+1) <= 0xBF) &&
					(0x80 <= (unsigned char)*(src+2) && (unsigned char)*(src+2) <= 0xBF)
					) ||
					(// straight 3-byte
					((0xE1 <= (unsigned char)*src && (unsigned char)*src <= 0xEC) ||
					(unsigned char)*src == 0xEE ||
					(unsigned char)*src == 0xEF) &&
					(0x80 <= (unsigned char)*(src+1) && (unsigned char)*(src+1) <= 0xBF) &&
					(0x80 <= (unsigned char)*(src+2) && (unsigned char)*(src+2) <= 0xBF)
					) ||
					(// excluding surrogates
					(unsigned char)*src == 0xED &&
					(0x80 <= (unsigned char)*(src+1) && (unsigned char)*(src+1) <= 0x9F) &&
					(0x80 <= (unsigned char)*(src+2) && (unsigned char)*(src+2) <= 0xBF)
				)
			) {
					dst += sprintf(dst, "%%%.02X", (unsigned char)*src);
					dst += sprintf(dst, "%%%.02X", (unsigned char)*(src+1));
					dst += sprintf(dst, "%%%.02X", (unsigned char)*(src+2));
					src += 2;
			}else if( (// planes 1-3
				(unsigned char)*src == 0xF0 &&
				(0x90 <= (unsigned char)*(src+1) && (unsigned char)*(src+1) <= 0xBF) &&
				(0x80 <= (unsigned char)*(src+2) && (unsigned char)*(src+2) <= 0xBF) &&
				(0x80 <= (unsigned char)*(src+3) && (unsigned char)*(src+3) <= 0xBF)
				) ||
				(// planes 4-15
				(0xF1 <= (unsigned char)*src && (unsigned char)*src <= 0xF3) &&
				(0x80 <= (unsigned char)*(src+1) && (unsigned char)*(src+1) <= 0xBF) &&
				(0x80 <= (unsigned char)*(src+2) && (unsigned char)*(src+2) <= 0xBF) &&
				(0x80 <= (unsigned char)*(src+3) && (unsigned char)*(src+3) <= 0xBF)
				) ||
				(// plane 16
				(unsigned char)*src == 0xF4 &&
				(0x80 <= (unsigned char)*(src+1) && (unsigned char)*(src+1) <= 0x8F) &&
				(0x80 <= (unsigned char)*(src+2) && (unsigned char)*(src+2) <= 0xBF) &&
				(0x80 <= (unsigned char)*(src+3) && (unsigned char)*(src+3) <= 0xBF)
			)
			) {
				dst += sprintf(dst, "%%%.02X", (unsigned char)*src);
				dst += sprintf(dst, "%%%.02X", (unsigned char)*(src+1));
				dst += sprintf(dst, "%%%.02X", (unsigned char)*(src+2));
				dst += sprintf(dst, "%%%.02X", (unsigned char)*(src+3));
				src += 3;
			}
		}
	}
	if (dst <= end)
		*dst = '\0';

	return dst - output;
}

void char_to_ascii_with_utf8(const char *output, const char *input)
{
	int outlen = strlen(input)*3 + 1;
	char_to_ascii_safe_with_utf8(output, input, outlen);
}
#endif

/* Transfer Char to ASCII */
int char_to_ascii_safe(const char *output, const char *input, int outsize)
{
	char *src = (char *)input;
	char *dst = (char *)output;
	char *end = (char *)output + outsize - 1;

	if (src == NULL || dst == NULL || outsize <= 0)
		return 0;

	for ( ; *src && dst < end; src++) {
		if ((*src >='0' && *src <='9') ||
		    (*src >='A' && *src <='Z') ||
		    (*src >='a' && *src <='z')) {
			*dst++ = *src;
		} else {
			if (dst + 3 > end)
				break;
			if( (unsigned char)*src >= 32 && (unsigned char)*src <= 127) {
				dst += sprintf(dst, "%%%.02X", (unsigned char)*src);
			}
		}
	}
	if (dst <= end)
		*dst = '\0';

	return dst - output;
}

void char_to_ascii(const char *output, const char *input)
{
	int outlen = strlen(input)*3 + 1;
	char_to_ascii_safe(output, input, outlen);
}

/* Transfer ASCII to Char */
int ascii_to_char_safe(const char *output, const char *input, int outsize){
	char *src = (char *)input;
	char *dst = (char *)output;
	char *end = (char *)output+outsize-1;
	char char_array[3];
	unsigned int char_code;

	if(src == NULL || dst == NULL || outsize <= 0)
		return 0;

	for(; *src && dst < end; ++src, ++dst){
		if((*src >= '0' && *src <= '9')
				|| (*src >= 'A' && *src <= 'Z')
				|| (*src >= 'a' && *src <= 'z')
				){
			*dst = *src;
		}
		else if(*src == '\\'){
			++src;
			if(!(*src))
				break;

			*dst = *src;
		}
		else{
			++src;
			if(!(*src))
				break;
			memset(char_array, 0, 3);
			strncpy(char_array, src, 2);
			++src;

			char_code = strtol(char_array, NULL, 16);

			*dst = (char)char_code;
		}
	}

	if(dst <= end)
		*dst = '\0';

	return (dst-output);
}

void ascii_to_char(const char *output, const char *input){
	int outlen = strlen(input)+1;
	ascii_to_char_safe(output, input, outlen);
}

const char *find_word(const char *buffer, const char *word)
{
	const char *p, *q;
	int n;

	n = strlen(word);
	p = buffer;
	while ((p = strstr(p, word)) != NULL) {
		if ((p == buffer) || (*(p - 1) == ' ') || (*(p - 1) == ',')) {
			q = p + n;
			if ((*q == ' ') || (*q == ',') || (*q == 0)) {
				return p;
			}
		}
		++p;
	}
	return NULL;
}

/*
static void add_word(char *buffer, const char *word, int max)
{
	if ((*buffer != 0) && (buffer[strlen(buffer) - 1] != ' '))
		strlcat(buffer, " ", max);
	strlcat(buffer, word, max);
}
*/

int remove_word(char *buffer, const char *word)
{
	char *p;
	char *q;

	if ((p = strstr(buffer, word)) == NULL) return 0;
	q = p;
	p += strlen(word);
	while (*p == ' ') ++p;
	while ((q > buffer) && (*(q - 1) == ' ')) --q;
	if (*p != '\0' && q != buffer) *q++ = ' ';		/* add ' ' if have remain string and not in the head of buffer */
	if(p != q)
		memmove(q, p, strlen(p)+1);			/* including '\0' */

	return 1;
}

void replace_char(char *str, char find, char replace) {
	char *p;

	for(p = str; *p != '\0'; p++)
		if(*p == find) *p = replace;
}

/* Escape characters that could break a Javascript array */
int str_escape_quotes(const char *output, const char *input, int outsize)
{
	char *src = (char *)input;
	char *dst = (char *)output;
	char *end = (char *)output + outsize - 1;
	char *escape = "'\"\\";

	if (src == NULL || dst == NULL || outsize <= 0)
		return 0;

	for ( ; *src && dst < end; src++) {
		if (strchr(escape, *src)) {
			if (dst + 2 > end)
				break;
			*dst++ = '\\';
			*dst++ = *src;
		} else {
			*dst++ = *src;
		}
	}
	if (dst <= end)
		*dst = '\0';

	return dst - output;
}

void trim_space(char *str)
{
	char *pt;
	if (!str) return;
	pt = str+strlen(str)-1;
	while (pt > str) {
		if (*pt==' ')
			*pt--='\0';
		else
			break;
	}
}

void trim_colon(char *str)
{
	int i=0, len=0, j=0;
	if (!str) return;
	len=strlen(str);
	for(i=0; i<len; i++)
	{
		if(str[i]==':')
		{
			for(j=i; j<len; j++)
			{
				str[j]=str[j+1];
			}
		len--;
		}
	}
}

void trim_char(char *str, char c)
{
	int in = 0;
	int out = 0;

	if (!str)
		return;

	while (str[in])
	{
		if (str[in] != c)
			str[out++] = str[in];
		in++;
	}

	str[out] = '\0';
}

void toLowerCase(char *str) {
	char *p;

	for(p = str; *p != '\0'; p++)
		if(*p >= 'A' && *p <='Z') *p += 32;
}

void toUpperCase(char *str) {
	char *p;

	for(p = str; *p != '\0'; p++)
		if(*p >= 'a' && *p <='z') *p -= 32;
}
