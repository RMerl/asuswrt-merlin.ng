/* -*- mode: c; c-file-style: "openbsd" -*- */

#include <string.h>

/*
 * Determine the length of a fixed-size string. This is really a
 * wrapper around `memchr()`.
 */
size_t
strnlen(const char *string, size_t maxlen)
{
	const char *end = memchr(string, '\0', maxlen);
	return end?(size_t)(end - string):maxlen;
}
