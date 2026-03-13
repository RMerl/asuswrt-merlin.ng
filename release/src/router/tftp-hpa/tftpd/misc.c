/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2001-2007 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software available under the same license
 *   as the "OpenBSD" operating system, distributed at
 *   http://www.openbsd.org/.
 *
 * ----------------------------------------------------------------------- */

/*
 * misc.c
 *
 * Minor help routines.
 */

#include "config.h"             /* Must be included first! */
#include <syslog.h>
#include "tftpd.h"

/*
 * Set the signal handler and flags.  Basically a user-friendly
 * wrapper around sigaction().
 */
void set_signal(int signum, void (*handler) (int), int flags)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = flags;

    if (sigaction(signum, &sa, NULL)) {
        syslog(LOG_ERR, "sigaction: %m");
        exit(EX_OSERR);
    }
}

/*
 * malloc() that syslogs an error message and bails if it fails.
 */
void *tfmalloc(size_t size)
{
    void *p = malloc(size);

    if (!p) {
        syslog(LOG_ERR, "malloc: %m");
        exit(EX_OSERR);
    }

    return p;
}

/*
 * strdup() that does the equivalent
 */
char *tfstrdup(const char *str)
{
    char *p = strdup(str);

    if (!p) {
        syslog(LOG_ERR, "strdup: %m");
        exit(EX_OSERR);
    }

    return p;
}
