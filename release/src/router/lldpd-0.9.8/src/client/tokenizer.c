/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2013 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "client.h"

#include <string.h>
/**
 * Tokenize the given line. We support quoted strings and escaped characters
 * with backslash.
 *
 * @param line Line to tokenize.
 * @param argv Will get an array of arguments tokenized.
 * @param argc Will get the number of tokenized arguments.
 * @return 0 on success, -1 on internal error, 1 on unmatched quotes
 */
int
tokenize_line(const char *line, int *argc, char ***argv)
{
	int iargc = 0; char **iargv = NULL;
	char *ifs     = " \n\t";
	char *quotes  = "'\"";
	char *escapes = "\\";
	char empty = 2;		/* Empty character, will be removed from output
				 * but will mark a word. */

	/* Escape handle. Also escape quoted characters. */
	int escaped = 0;
	int ipos = 0;
	char quote = 0;
	char input[2*strlen(line) + 3]; /* 3 = 2 for '\n ' and 1 for \0 */
	memset(input, 0, 2*strlen(line) + 3);
	for (int pos = 0; line[pos]; pos++) {
		if (line[pos] == '#' && !escaped && !quote)
			break;
		if (!escaped && strchr(escapes, line[pos]))
			escaped = 1;
		else if (!escaped && strchr(quotes, line[pos]) && !quote) {
			input[ipos++] = empty;
			input[ipos++] = '!';
			quote = line[pos];
		} else if (!escaped && quote == line[pos])
			quote = 0;
		else {
			input[ipos++] = line[pos];
			input[ipos++] = (escaped || quote)?'!':' ';
			escaped = 0;
		}
	}
	if (escaped || quote) return 1;
	/* Trick to not have to handle \0 in a special way */
	input[ipos++] = ifs[0];
	input[ipos++] = ' ';

	/* Tokenize, we don't have to handle quotes anymore */
	int wbegin  = -1;      /* Offset of the beginning of the current word */

#define CURRENT (input[2*pos])
#define ESCAPED (input[2*pos+1] != ' ')
	for (int pos = 0; CURRENT; pos++) {
		if (wbegin == -1) {
			if (!ESCAPED && strchr(ifs, CURRENT))
				/* IFS while not in a word, continue. */
				continue;
			/* Start a word. */
			wbegin = pos;
			continue;
		}
		if (ESCAPED || !strchr(ifs, CURRENT))
			/* Regular character in a word. */
			continue;

		/* End of word. */
		char *word = calloc(1, pos - wbegin + 1);
		if (!word) goto error;
		int i,j;
		for (i = wbegin, j = 0;
		     i != pos;
		     i++)
			if (input[2*i] != empty) word[j++] = input[2*i];
		char **nargv = realloc(iargv, sizeof(char*) * (iargc + 1));
		if (!nargv) {
			free(word);
			goto error;
		}
		nargv[iargc++] = word;
		iargv  = nargv;
		wbegin = -1;
	}

	*argc = iargc;
	*argv = iargv;
	return 0;

error:
	tokenize_free(iargc, iargv);
	return -1;
}

void
tokenize_free(int argc, char **argv)
{
	while (argc) free(argv[--argc]);
	free(argv);
}

