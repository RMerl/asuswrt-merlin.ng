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
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

int
sandbox_before(void)
{

	return(1);
}

/*
 * Use pledge(2) to sandbox based which process we're in.
 */
int
sandbox_after(int arg)
{

	switch (proccomp) {
	case (COMP_ACCOUNT):
	case (COMP_CERT):
	case (COMP_KEY):
	case (COMP_REVOKE):
	case (COMP__MAX):
		if (-1 == pledge("stdio", NULL)) {
			warn("pledge");
			return(0);
		}
		break;
	case (COMP_CHALLENGE):
		/*
		 * If "arg" is set, that means that our challenge is
		 * going to be exported to the caller and we don't need
		 * to touch any files.
		 */
		if (arg) {
			if (-1 == pledge("stdio", NULL)) {
				warn("pledge");
				return(0);
			}
		} else {
			if (-1 == pledge("stdio cpath wpath", NULL)) {
				warn("pledge");
				return(0);
			}
		}
		break;
	case (COMP_DNS):
		if (-1 == pledge("stdio dns", NULL)) {
			warn("pledge");
			return(0);
		}
		break;
	case (COMP_FILE):
		/* 
		 * Rpath and cpath for rename, wpath and cpath for
		 * writing to the temporary.
		 */
		if (-1 == pledge("stdio cpath wpath rpath", NULL)) {
			warn("pledge");
			return(0);
		}
		break;
	case (COMP_NET):
		/*
		 * Prior to tls_config.c version 1.19, the CA file was
		 * lazy-loaded during configuration, which will crash
		 * the pledge unless rpath is enabled.
		 */
#if TLS_API < 20160801
		if (-1 == pledge("stdio inet rpath", NULL)) {
			warn("pledge");
			return(0);
		}
#else
		if (-1 == pledge("stdio inet", NULL)) {
			warn("pledge");
			return(0);
		}
#endif
		break;
	}
	return(1);
}
