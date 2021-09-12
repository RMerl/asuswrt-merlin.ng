/* -*- mode: c; c-file-style: "openbsd" -*- */

#include <stdlib.h>
#include <string.h>

/*
 * Similar to `strdup()` but copies at most n bytes.
 */
char*
strndup(const char *string, size_t maxlen)
{
	char *result;
	/* We may use `strnlen()` but it may be unavailable. */
	const char *end = memchr(string, '\0', maxlen);
	size_t len = end?(size_t)(end - string):maxlen;

	result = malloc(len + 1);
	if (!result) return 0;

	memcpy(result, string, len);
	result[len] = '\0';
	return result;
}
