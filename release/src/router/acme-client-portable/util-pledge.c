/*	$Id$ */
/*
 * Copyright (c) 2016 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <err.h>
#include <unistd.h>

#include "extern.h"

int
dropfs(const char *path)
{

	/*
	 * Only the challenge and file processes touch files within the
	 * pledge, so only these need to be chrooted.
	 */
	
	if (COMP_CHALLENGE != proccomp &&
	    COMP_FILE != proccomp)
		return(1);

	if (-1 == chroot(path))
		warn("%s: chroot", path);
	else if (-1 == chdir("/")) 
		warn("/: chdir");
	else
		return(1);

	return(0);
}

int
checkprivs(void)
{

	/* Needed for chroot(2) calls in dropfs(). */

	return(0 == getuid());
}

int
dropprivs(void)
{

	/* Don't need to drop privileges like this. */

	return(1);
}
