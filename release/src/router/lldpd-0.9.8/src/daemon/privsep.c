/* -*- mode: c; c-file-style: "openbsd" -*- */

#include "lldpd.h"

static int privileged, unprivileged;
void
priv_privileged_fd(int fd)
{
	privileged = fd;
}
void
priv_unprivileged_fd(int fd)
{
	unprivileged = fd;
}
int
priv_fd(enum priv_context ctx)
{
	switch (ctx) {
	case PRIV_PRIVILEGED: return privileged;
	case PRIV_UNPRIVILEGED: return unprivileged;
	}
	return -1;		/* Not possible */
}
