/* Replacement in case utimensat(2) is missing
 *
 * Copyright (C) 2017-2021  Joachim Wiberg <troglobit@gmail.com>
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

#include "config.h"

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <sys/time.h>		/* lutimes(), utimes(), utimensat() */

int utimensat(int dirfd, const char *pathname, const struct timespec ts[2], int flags)
{
	int ret = -1;
	struct timeval tv[2];

	if (dirfd != 0) {
		errno = ENOTSUP;
		return -1;
	}

	TIMESPEC_TO_TIMEVAL(&tv[0], &ts[0]);
	TIMESPEC_TO_TIMEVAL(&tv[1], &ts[1]);

#ifdef AT_SYMLINK_NOFOLLOW
	if ((flags & AT_SYMLINK_NOFOLLOW) == AT_SYMLINK_NOFOLLOW)
		ret = lutimes(pathname, tv);
	else
#endif
		ret = utimes(pathname, tv);

	return ret;
}
