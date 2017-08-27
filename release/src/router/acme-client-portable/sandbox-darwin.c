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
#include <sandbox.h>
#include <stdlib.h>

#include "extern.h"

int
sandbox_before(void)
{

	switch (proccomp) {
	case (COMP_ACCOUNT):
	case (COMP_CERT):
	case (COMP_CHALLENGE):
	case (COMP_FILE):
	case (COMP_KEY):
	case (COMP_REVOKE):
	case (COMP__MAX):
		if (-1 == sandbox_init
		    (kSBXProfileNoNetwork, SANDBOX_NAMED, NULL)) {
			warnx("sandbox_init");
			return(0);
		}
		break;
	case (COMP_DNS):
	case (COMP_NET):
		if (-1 == sandbox_init
		    (kSBXProfileNoWrite, SANDBOX_NAMED, NULL)) {
			warnx("sandbox_init");
			return(0);
		}
		break;
	}

	return(1);
}

int
sandbox_after(int arg)
{

	(void)arg;
	return(1);
}
