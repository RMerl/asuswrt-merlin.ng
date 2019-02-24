/*
 * Copyright (C) 2007-2008 Andreas Steffen
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

#include <stdio.h>
#include <errno.h>

#include <library.h>
#include <utils/debug.h>
#include <utils/lexparser.h>

#include "optionsfrom.h"

#define	MAX_USES	 20		/* loop-detection limit */
#define	MORE_ARGS	 10		/* first guess at how many arguments we'll need */

/*
 * Defined in header.
 */

typedef struct private_options_t private_options_t;

/**
 * Private data of a options_t object.
 */
struct private_options_t {
	/**
	 * Public interface
	 */
	options_t public;

	/**
	 * reallocated argv array
	 */
	char **newargv;

	/**
	 * number of free arguments in newargv
	 */
	int room;

	/**
	 * number of included option files
	*/
	int nuses;

	/**
	 * allocated space for option files
	 */
	char *buffers[MAX_USES];
};

METHOD(options_t, from, bool,
	private_options_t *this, char *filename, int *argcp, char **argvp[], int optind)
{
	int newargc;
	int next;			/* place for next argument */
	char **newargv;
	chunk_t src, line, token;
	bool good = TRUE;
	int linepos = 0;
	FILE *fd;

	/* avoid endless loops with recursive --optionsfrom arguments */
	this->nuses++;
	if (this->nuses >= MAX_USES)
	{
		DBG1(DBG_LIB, "optionsfrom called %d times by \"%s\" - looping?",
			 this->nuses + 1, (*argvp)[0]);
		return FALSE;
	}

	fd = fopen(filename, "r");
	if (fd == NULL)
	{
		DBG1(DBG_LIB, "optionsfrom: unable to open file '%s': %s",
			 filename, strerror(errno));
		return FALSE;
	}

	/* determine the file size */
	if (fseek(fd, 0, SEEK_END) == -1 || (src.len = ftell(fd)) == -1)
	{
		DBG1(DBG_LIB, "optionsfrom: unable to determine size of '%s': %s",
			 filename, strerror(errno));
		fclose(fd);
		return FALSE;
	}
	rewind(fd);

	/* allocate one byte more just in case of a missing final newline */
	src.ptr = this->buffers[this->nuses] = malloc(src.len + 1);

	/* read the whole file into a chunk */
	if (fread(src.ptr, 1, src.len, fd) != src.len)
	{
		DBG1(DBG_LIB, "optionsfrom: unable to read file '%s': %s",
			 filename, strerror(errno));
		free(src.ptr);
		fclose(fd);
		return FALSE;
	}
	fclose(fd);

	if (this->room)
	{
		newargc = *argcp;
		newargv = malloc((newargc + 1 + this->room) * sizeof(char *));
	}
	else
	{
		newargc = *argcp + MORE_ARGS;
		this->room = MORE_ARGS;
		newargv = malloc((newargc + 1) * sizeof(char *));
	}
	memcpy(newargv, *argvp, optind * sizeof(char *));
	next = optind;
	newargv[next] = NULL;

	while (fetchline(&src, &line) && good)
	{
		linepos++;
		while (eat_whitespace(&line))
		{
			if (*line.ptr == '"'|| *line.ptr == '\'')
			{
				char delimiter = *line.ptr;

				line.ptr++;
				line.len--;
				if (!extract_token(&token, delimiter, &line))
				{
					DBG1(DBG_LIB, "optionsfrom: missing terminator at %s:%d",
						 filename, linepos);
					good = FALSE;
					break;
				}
			}
			else
			{
				if (!extract_token(&token, ' ', &line))
				{
					/* last token in a line */
					token = line;
					line.len = 0;
				}
			}

			/* do we have to allocate more memory for additional arguments? */
			if (this->room == 0)
			{
				newargc += MORE_ARGS;
				newargv = realloc(newargv, (newargc + 1) * sizeof(char *));
				this->room = MORE_ARGS;
			}

			/* terminate the token by replacing the delimiter with a null character */
			*(token.ptr + token.len) = '\0';

			/* assign the token to the next argument */
			newargv[next] = token.ptr;
			next++;
			this->room--;
		}
	}

	/* assign newargv to argv */
	if (good)
	{
		memcpy(newargv + next, *argvp + optind, (*argcp + 1 - optind) * sizeof(char *));
		*argcp += next - optind;
		*argvp = newargv;
	}

	/* keep a pointer to the latest newargv and free any earlier version */
	free(this->newargv);
	this->newargv = newargv;

	return good;
}

METHOD(options_t, destroy, void,
	private_options_t *this)
{
	while (this->nuses >= 0)
	{
		free(this->buffers[this->nuses--]);
	}
	free(this->newargv);
	free(this);
}

/*
 * Defined in header
 */
options_t *options_create(void)
{
	private_options_t *this;

	INIT(this,
		.public = {
			.from = _from,
			.destroy = _destroy,

		},
		.nuses = -1,
	);

	return &this->public;
}
