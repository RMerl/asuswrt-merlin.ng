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

#include <sys/stat.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

int
chngproc(int netsock, const char *root, const struct config *cfg)
{
	char		 *alt = NULL, *tok = NULL, *th = NULL, *fmt = NULL, 
			 *fmtbuf = NULL;
	char		**fs = NULL;
	size_t		  i, fsz = 0, sz;
	ssize_t		  ssz;
	int		  rc = 0, fd = -1, cc;
	long		  lval;
	enum chngop	  op;
	void		 *pp;

	/* File-system and sandbox jailing. */

	if ( ! sandbox_before())
		goto out;
	else if ( ! dropfs(NULL != cfg->challenge ? PATH_VAR_EMPTY : root))
		goto out;
	else if ( ! sandbox_after(NULL != cfg->challenge))
		goto out;

	/*
	 * Make sure that the created file can be read by all parties,
	 * but not written to.
	 * (Our default umask for root is sometimes to disallow others
	 * from reading the file.)
	 */

	(void)umask(S_IWGRP | S_IWOTH);

	/*
	 * Loop while we wait to get a thumbprint and token.
	 * We'll get this for each SAN request.
	 */

	for (;;) {
		op = CHNG__MAX;
		if (0 == (lval = readop(netsock, COMM_CHNG_OP)))
			op = CHNG_STOP;
		else if (CHNG_SYN == lval)
			op = lval;

		if (CHNG__MAX == op) {
			warnx("unknown operation from netproc");
			goto out;
		} else if (CHNG_STOP == op)
			break;

		assert(CHNG_SYN == op);

		/*
		 * Read the alt, thumbprint, and token.
		 * The token is the filename, so store that in a vector
		 * of tokens that we'll later clean up.
		 */

		if (NULL == (alt = readstr(netsock, COMM_DNSA)))
			goto out;
		else if (NULL == (th = readstr(netsock, COMM_THUMB)))
			goto out;
		else if (NULL == (tok = readstr(netsock, COMM_TOK)))
			goto out;

		/* Vector appending... */

		pp = realloc(fs, (fsz + 1) * sizeof(char *));
		if (NULL == pp) {
			warn("realloc");
			goto out;
		}
		fs = pp;
		fs[fsz] = tok;
		tok = NULL;
		fsz++;

		if (NULL != cfg->challenge) {
			/*
			 * If we have a specific challenge request, then
			 * we write the request and thumbprint to stdout
			 * and wait for a reply (which must be an echo
			 * of the output) to indicate that all's well.
			 */
			fmt = doasprintf("%s %s %s.%s\n", 
				cfg->challenge, alt, fs[fsz - 1], th);
			if (NULL == fmt) {
				warn("asprintf");
				goto out;
			} else if (NULL == (fmtbuf = strdup(fmt))) {
				warn("strdup");
				goto out;
			}

			sz = strlen(fmt);
			ssz = write(STDOUT_FILENO, fmt, sz);
			if (-1 == ssz) {
				warn("<stdout>");
				goto out;
			} else if ((size_t)ssz != sz) {
				warnx("<stdout>: short write");
				goto out;
			}
			doddbg("%s: challenge written", fs[fsz - 1]);
			ssz = read(STDIN_FILENO, fmt, sz);
			if (-1 == ssz) {
				warn("<stdin>");
				goto out;
			} else if ((size_t)ssz != sz) {
				warnx("<stdin>: short read");
				goto out;
			} else if (strcmp(fmt, fmtbuf)) {
				warnx("<stdin>: token mismatch");
				goto out;
			}
			doddbg("%s: challenge read", fs[fsz - 1]);
		} else { 
			/* 
			 * Create and write to our challenge file.
			 * Note: we use file descriptors instead of FILE
			 * because we want to minimise our pledges.
			 */
			fmt = doasprintf("%s.%s", fs[fsz - 1], th);
			if (NULL == fmt) {
				warn("asprintf");
				goto out;
			}

			fd = open(fs[fsz - 1], 
				O_WRONLY|O_EXCL|O_CREAT, 0444);
			if (-1 == fd) {
				warn("%s", fs[fsz - 1]);
				goto out;
			} if (-1 == write(fd, fmt, strlen(fmt))) {
				warn("%s", fs[fsz - 1]);
				goto out;
			} else if (-1 == close(fd)) {
				warn("%s", fs[fsz - 1]);
				goto out;
			}
			fd = -1;
			dodbg("%s/%s: created", root, fs[fsz - 1]);
		}

		free(th);
		free(fmt);
		free(fmtbuf);
		th = fmt = fmtbuf = NULL;

		/*
		 * Write our acknowledgement.
		 * Ignore reader failure.
		 */

		cc = writeop(netsock, COMM_CHNG_ACK, CHNG_ACK);
		if (0 == cc)
			break;
		if (cc < 0)
			goto out;
	}

	rc = 1;
out:
	close(netsock);
	if (-1 != fd)
		close(fd);
	if (NULL == cfg->challenge) 
		for (i = 0; i < fsz; i++) {
			if (-1 == unlink(fs[i]) && ENOENT != errno)
				warn("%s", fs[i]);
			free(fs[i]);
		}
	free(fs);
	free(fmt);
	free(fmtbuf);
	free(th);
	free(tok);
	return(rc);
}
