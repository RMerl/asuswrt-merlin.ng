/*
 * sleep -- sleep for fractions of a second
 *
 * usage: sleep seconds[.fraction]
 */

/*
   Copyright (C) 1999-2009 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#include "bashtypes.h"

#if defined (TIME_WITH_SYS_TIME)
#  include <sys/time.h>
#  include <time.h>
#else
#  if defined (HAVE_SYS_TIME_H)
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

#if defined (HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include <stdio.h>
#include "chartypes.h"

#include "shell.h"
#include "builtins.h"
#include "common.h"

#define RETURN(x) \
	do { \
		if (sp) *sp = sec; \
		if (usp) *usp = usec; \
		return (x); \
	} while (0)

int
sleep_builtin (list)
WORD_LIST	*list;
{
	long	sec, usec;

	if (list == 0) {
		builtin_usage();
		return(EX_USAGE);
	}

	/* Skip over `--' */
	if (list->word && ISOPTION (list->word->word, '-'))
		list = list->next;

	if (*list->word->word == '-' || list->next) {
		builtin_usage ();
		return (EX_USAGE);
	}

    	if (uconvert(list->word->word, &sec, &usec)) {
		fsleep(sec, usec);
		return(EXECUTION_SUCCESS);
    	}

	builtin_error("%s: bad sleep interval", list->word->word);
	return (EXECUTION_FAILURE);
}

static char *sleep_doc[] = {
	"Suspend execution for specified period.",
	""
	"sleep suspends execution for a minimum of SECONDS[.FRACTION] seconds.",
	(char *)NULL
};

struct builtin sleep_struct = {
	"sleep",
	sleep_builtin,
	BUILTIN_ENABLED,
	sleep_doc,
	"sleep seconds[.fraction]",
	0
};
