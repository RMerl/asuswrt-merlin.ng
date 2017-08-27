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

#include <sys/types.h>

#include <err.h>
#include <pwd.h>
#ifndef OPSSL
#include <tls.h>
#endif
#include <unistd.h>

#include "extern.h"

static	uid_t uid;
static	gid_t gid;

int
dropfs(const char *path)
{

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
	struct passwd	 *passent;

	/* We need root for our chroots. */

	if (0 != getuid())
		return(0);

	/* We need this for our privdropping. */

	passent = getpwnam(NOBODY_USER);
	if (NULL == passent) {
		warnx("%s: unknown user", NOBODY_USER);
		return(0);
	}

	uid = passent->pw_uid;
	gid = passent->pw_gid;
	return(1);
}

int
dropprivs(void)
{

	/*
	 * Safely drop privileges into the given credentials.
	 */

	if (setgroups(1, &gid) ||
	    setresgid(gid, gid, gid) ||
	    setresuid(uid, uid, uid)) {
		warnx("drop privileges");
		return(0);
	}

	if (getgid() != gid || getegid() != gid) {
		warnx("failed to drop gid");
		return(0);
	}
	if (getuid() != uid || geteuid() != uid) {
		warnx("failed to drop uid");
		return(0);
	}

	return(1);
}
