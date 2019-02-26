/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#include <utils/utils.h>
#include <utils/chunk.h>

/**
 * Described in header.
 */
void memxor(uint8_t dst[], const uint8_t src[], size_t n)
{
	int m, i;

	/* byte wise XOR until dst aligned */
	for (i = 0; (uintptr_t)&dst[i] % sizeof(long) && i < n; i++)
	{
		dst[i] ^= src[i];
	}
	/* try to use words if src shares an alignment with dst */
	switch (((uintptr_t)&src[i] % sizeof(long)))
	{
		case 0:
			for (m = n - sizeof(long); i <= m; i += sizeof(long))
			{
				*(long*)&dst[i] ^= *(long*)&src[i];
			}
			break;
		case sizeof(int):
			for (m = n - sizeof(int); i <= m; i += sizeof(int))
			{
				*(int*)&dst[i] ^= *(int*)&src[i];
			}
			break;
		case sizeof(short):
			for (m = n - sizeof(short); i <= m; i += sizeof(short))
			{
				*(short*)&dst[i] ^= *(short*)&src[i];
			}
			break;
		default:
			break;
	}
	/* byte wise XOR of the rest */
	for (; i < n; i++)
	{
		dst[i] ^= src[i];
	}
}

/**
 * Described in header.
 */
void memwipe_noinline(void *ptr, size_t n)
{
	memwipe_inline(ptr, n);
}

/**
 * Described in header.
 */
bool memeq_const(const void *x, const void *y, size_t len)
{
	const u_char *a, *b;
	u_int bad = 0;
	size_t i;

	a = (const u_char*)x;
	b = (const u_char*)y;

	for (i = 0; i < len; i++)
	{
		bad |= a[i] != b[i];
	}
	return !bad;
}

/**
 * Described in header.
 */
void *memstr(const void *haystack, const char *needle, size_t n)
{
	const u_char *pos = haystack;
	size_t l;

	if (!haystack || !needle || (l = strlen(needle)) == 0)
	{
		return NULL;
	}
	for (; n >= l; ++pos, --n)
	{
		if (memeq(pos, needle, l))
		{
			return (void*)pos;
		}
	}
	return NULL;
}

/**
 * Described in header.
 */
void *utils_memrchr(const void *s, int c, size_t n)
{
	const u_char *pos;

	if (!s || !n)
	{
		return NULL;
	}

	for (pos = s + n - 1; pos >= (u_char*)s; pos--)
	{
		if (*pos == (u_char)c)
		{
			return (void*)pos;
		}
	}
	return NULL;
}

#ifdef HAVE_FMEMOPEN_FALLBACK

static int fmemread(chunk_t *cookie, char *buf, int size)
{
	int len;

	len = min(size, cookie->len);
	memcpy(buf, cookie->ptr, len);
	*cookie = chunk_skip(*cookie, len);

	return len;
}

static int fmemwrite(chunk_t *cookie, const char *buf, int size)
{
	int len;

	len = min(size, cookie->len);
	memcpy(cookie->ptr, buf, len);
	*cookie = chunk_skip(*cookie, len);

	return len;
}

static int fmemclose(void *cookie)
{
	free(cookie);
	return 0;
}

FILE *fmemopen(void *buf, size_t size, const char *mode)
{
	chunk_t *cookie;

	INIT(cookie,
		.ptr = buf,
		.len = size,
	);

	return funopen(cookie, (void*)fmemread, (void*)fmemwrite, NULL, fmemclose);
}

#endif /* FMEMOPEN fallback*/

/**
 * Number of bytes per line to dump raw data
 */
#define BYTES_PER_LINE 16

static char hexdig_upper[] = "0123456789ABCDEF";

/**
 * Described in header.
 */
int mem_printf_hook(printf_hook_data_t *data,
					printf_hook_spec_t *spec, const void *const *args)
{
	char *bytes = *((void**)(args[0]));
	u_int len = *((int*)(args[1]));

	char buffer[BYTES_PER_LINE * 3];
	char ascii_buffer[BYTES_PER_LINE + 1];
	char *buffer_pos = buffer;
	char *bytes_pos  = bytes;
	char *bytes_roof = bytes + len;
	int line_start = 0;
	int i = 0;
	int written = 0;

	written += print_in_hook(data, "=> %u bytes @ %p", len, bytes);

	while (bytes_pos < bytes_roof)
	{
		*buffer_pos++ = hexdig_upper[(*bytes_pos >> 4) & 0xF];
		*buffer_pos++ = hexdig_upper[ *bytes_pos       & 0xF];

		ascii_buffer[i++] =
				(*bytes_pos > 31 && *bytes_pos < 127) ? *bytes_pos : '.';

		if (++bytes_pos == bytes_roof || i == BYTES_PER_LINE)
		{
			int padding = 3 * (BYTES_PER_LINE - i);

			while (padding--)
			{
				*buffer_pos++ = ' ';
			}
			*buffer_pos++ = '\0';
			ascii_buffer[i] = '\0';

			written += print_in_hook(data, "\n%4d: %s  %s",
									 line_start, buffer, ascii_buffer);

			buffer_pos = buffer;
			line_start += BYTES_PER_LINE;
			i = 0;
		}
		else
		{
			*buffer_pos++ = ' ';
		}
	}
	return written;
}
