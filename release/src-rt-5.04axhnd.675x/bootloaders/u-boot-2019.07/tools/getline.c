// SPDX-License-Identifier: GPL-2.0+
/* getline.c -- Replacement for GNU C library function getline
 *
 * Copyright (C) 1993, 1996, 2001, 2002 Free Software Foundation, Inc.
 */

/* Written by Jan Brittenson, bson@gnu.ai.mit.edu.  */

#include <assert.h>
#include <stdio.h>

/* Always add at least this many bytes when extending the buffer.  */
#define MIN_CHUNK 64

/* Read up to (and including) a TERMINATOR from STREAM into *LINEPTR
   + OFFSET (and null-terminate it). *LINEPTR is a pointer returned from
   malloc (or NULL), pointing to *N characters of space.  It is realloc'd
   as necessary.  Return the number of characters read (not including the
   null terminator), or -1 on error or EOF.
   NOTE: There is another getstr() function declared in <curses.h>.  */
static int getstr(char **lineptr, size_t *n, FILE *stream,
		  char terminator, size_t offset)
{
	int nchars_avail;	/* Allocated but unused chars in *LINEPTR.  */
	char *read_pos;		/* Where we're reading into *LINEPTR. */
	int ret;

	if (!lineptr || !n || !stream)
		return -1;

	if (!*lineptr) {
		*n = MIN_CHUNK;
		*lineptr = malloc(*n);
		if (!*lineptr)
			return -1;
	}

	nchars_avail = *n - offset;
	read_pos = *lineptr + offset;

	for (;;) {
		register int c = getc(stream);

		/* We always want at least one char left in the buffer, since we
		   always (unless we get an error while reading the first char)
		   NUL-terminate the line buffer.  */

		assert(*n - nchars_avail == read_pos - *lineptr);
		if (nchars_avail < 2) {
			if (*n > MIN_CHUNK)
				*n *= 2;
			else
				*n += MIN_CHUNK;

			nchars_avail = *n + *lineptr - read_pos;
			*lineptr = realloc(*lineptr, *n);
			if (!*lineptr)
				return -1;
			read_pos = *n - nchars_avail + *lineptr;
			assert(*n - nchars_avail == read_pos - *lineptr);
		}

		if (c == EOF || ferror (stream)) {
			/* Return partial line, if any.  */
			if (read_pos == *lineptr)
				return -1;
			else
				break;
		}

		*read_pos++ = c;
		nchars_avail--;

		if (c == terminator)
			/* Return the line.  */
			break;
	}

	/* Done - NUL terminate and return the number of chars read.  */
	*read_pos = '\0';

	ret = read_pos - (*lineptr + offset);
	return ret;
}

int getline (char **lineptr, size_t *n, FILE *stream)
{
	return getstr(lineptr, n, stream, '\n', 0);
}
