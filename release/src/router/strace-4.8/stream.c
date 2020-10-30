/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_SYS_POLL_H
# include <sys/poll.h>
#endif
#ifdef HAVE_SYS_CONF_H
# include <sys/conf.h>
#endif

/* Who has STREAMS syscalls?
 * Linux hasn't. Solaris has (had?).
 * Just in case I miss something, retain in for Sparc...
 */
#if defined(SPARC) || defined(SPARC64)

# ifdef HAVE_STROPTS_H
#  include <stropts.h>
# else
#  define RS_HIPRI 1
struct strbuf {
	int     maxlen;                 /* no. of bytes in buffer */
	int     len;                    /* no. of bytes returned */
	const char *buf;                /* pointer to data */
};
#  define MORECTL 1
#  define MOREDATA 2
# endif

static const struct xlat msgflags[] = {
	{ RS_HIPRI,	"RS_HIPRI"	},
	{ 0,		NULL		},
};

static void
printstrbuf(struct tcb *tcp, struct strbuf *sbp, int getting)
{
	if (sbp->maxlen == -1 && getting)
		tprints("{maxlen=-1}");
	else {
		tprints("{");
		if (getting)
			tprintf("maxlen=%d, ", sbp->maxlen);
		tprintf("len=%d, buf=", sbp->len);
		printstr(tcp, (unsigned long) sbp->buf, sbp->len);
		tprints("}");
	}
}

static void
printstrbufarg(struct tcb *tcp, long arg, int getting)
{
	struct strbuf buf;

	if (arg == 0)
		tprints("NULL");
	else if (umove(tcp, arg, &buf) < 0)
		tprints("{...}");
	else
		printstrbuf(tcp, &buf, getting);
	tprints(", ");
}

int
sys_putmsg(struct tcb *tcp)
{
	int i;

	if (entering(tcp)) {
		/* fd */
		tprintf("%ld, ", tcp->u_arg[0]);
		/* control and data */
		for (i = 1; i < 3; i++)
			printstrbufarg(tcp, tcp->u_arg[i], 0);
		/* flags */
		printflags(msgflags, tcp->u_arg[3], "RS_???");
	}
	return 0;
}

int
sys_getmsg(struct tcb *tcp)
{
	int i, flags;

	if (entering(tcp)) {
		/* fd */
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx, %#lx",
				tcp->u_arg[1], tcp->u_arg[2], tcp->u_arg[3]);
			return 0;
		}
		/* control and data */
		for (i = 1; i < 3; i++)
			printstrbufarg(tcp, tcp->u_arg[i], 1);
		/* pointer to flags */
		if (tcp->u_arg[3] == 0)
			tprints("NULL");
		else if (umove(tcp, tcp->u_arg[3], &flags) < 0)
			tprints("[?]");
		else {
			tprints("[");
			printflags(msgflags, flags, "RS_???");
			tprints("]");
		}
		/* decode return value */
		switch (tcp->u_rval) {
		case MORECTL:
			tcp->auxstr = "MORECTL";
			break;
		case MORECTL|MOREDATA:
			tcp->auxstr = "MORECTL|MOREDATA";
			break;
		case MOREDATA:
			tcp->auxstr = "MORECTL";
			break;
		default:
			tcp->auxstr = NULL;
			break;
		}
	}
	return RVAL_HEX | RVAL_STR;
}

# if defined SYS_putpmsg || defined SYS_getpmsg
static const struct xlat pmsgflags[] = {
#  ifdef MSG_HIPRI
	{ MSG_HIPRI,	"MSG_HIPRI"	},
#  endif
#  ifdef MSG_AND
	{ MSG_ANY,	"MSG_ANY"	},
#  endif
#  ifdef MSG_BAND
	{ MSG_BAND,	"MSG_BAND"	},
#  endif
	{ 0,		NULL		},
};
#  ifdef SYS_putpmsg
int
sys_putpmsg(struct tcb *tcp)
{
	int i;

	if (entering(tcp)) {
		/* fd */
		tprintf("%ld, ", tcp->u_arg[0]);
		/* control and data */
		for (i = 1; i < 3; i++)
			printstrbufarg(tcp, tcp->u_arg[i], 0);
		/* band */
		tprintf("%ld, ", tcp->u_arg[3]);
		/* flags */
		printflags(pmsgflags, tcp->u_arg[4], "MSG_???");
	}
	return 0;
}
#  endif
#  ifdef SYS_getpmsg
int
sys_getpmsg(struct tcb *tcp)
{
	int i, flags;

	if (entering(tcp)) {
		/* fd */
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx, %#lx, %#lx", tcp->u_arg[1],
				tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[4]);
			return 0;
		}
		/* control and data */
		for (i = 1; i < 3; i++)
			printstrbufarg(tcp, tcp->u_arg[i], 1);
		/* pointer to band */
		printnum(tcp, tcp->u_arg[3], "%d");
		tprints(", ");
		/* pointer to flags */
		if (tcp->u_arg[4] == 0)
			tprints("NULL");
		else if (umove(tcp, tcp->u_arg[4], &flags) < 0)
			tprints("[?]");
		else {
			tprints("[");
			printflags(pmsgflags, flags, "MSG_???");
			tprints("]");
		}
		/* decode return value */
		switch (tcp->u_rval) {
		case MORECTL:
			tcp->auxstr = "MORECTL";
			break;
		case MORECTL|MOREDATA:
			tcp->auxstr = "MORECTL|MOREDATA";
			break;
		case MOREDATA:
			tcp->auxstr = "MORECTL";
			break;
		default:
			tcp->auxstr = NULL;
			break;
		}
	}
	return RVAL_HEX | RVAL_STR;
}
#  endif
# endif /* getpmsg/putpmsg */

#endif /* STREAMS syscalls support */


#ifdef HAVE_SYS_POLL_H

static const struct xlat pollflags[] = {
# ifdef POLLIN
	{ POLLIN,	"POLLIN"	},
	{ POLLPRI,	"POLLPRI"	},
	{ POLLOUT,	"POLLOUT"	},
#  ifdef POLLRDNORM
	{ POLLRDNORM,	"POLLRDNORM"	},
#  endif
#  ifdef POLLWRNORM
	{ POLLWRNORM,	"POLLWRNORM"	},
#  endif
#  ifdef POLLRDBAND
	{ POLLRDBAND,	"POLLRDBAND"	},
#  endif
#  ifdef POLLWRBAND
	{ POLLWRBAND,	"POLLWRBAND"	},
#  endif
	{ POLLERR,	"POLLERR"	},
	{ POLLHUP,	"POLLHUP"	},
	{ POLLNVAL,	"POLLNVAL"	},
# endif
	{ 0,		NULL		},
};

static int
decode_poll(struct tcb *tcp, long pts)
{
	struct pollfd fds;
	unsigned nfds;
	unsigned long size, start, cur, end, abbrev_end;
	int failed = 0;

	if (entering(tcp)) {
		nfds = tcp->u_arg[1];
		size = sizeof(fds) * nfds;
		start = tcp->u_arg[0];
		end = start + size;
		if (nfds == 0 || size / sizeof(fds) != nfds || end < start) {
			tprintf("%#lx, %d, ",
				tcp->u_arg[0], nfds);
			return 0;
		}
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(fds);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}
		tprints("[");
		for (cur = start; cur < end; cur += sizeof(fds)) {
			if (cur > start)
				tprints(", ");
			if (cur >= abbrev_end) {
				tprints("...");
				break;
			}
			if (umoven(tcp, cur, sizeof fds, (char *) &fds) < 0) {
				tprints("?");
				failed = 1;
				break;
			}
			if (fds.fd < 0) {
				tprintf("{fd=%d}", fds.fd);
				continue;
			}
			tprints("{fd=");
			printfd(tcp, fds.fd);
			tprints(", events=");
			printflags(pollflags, fds.events, "POLL???");
			tprints("}");
		}
		tprints("]");
		if (failed)
			tprintf(" %#lx", start);
		tprintf(", %d, ", nfds);
		return 0;
	} else {
		static char outstr[1024];
		char *outptr;
#define end_outstr (outstr + sizeof(outstr))
		const char *flagstr;

		if (syserror(tcp))
			return 0;
		if (tcp->u_rval == 0) {
			tcp->auxstr = "Timeout";
			return RVAL_STR;
		}

		nfds = tcp->u_arg[1];
		size = sizeof(fds) * nfds;
		start = tcp->u_arg[0];
		end = start + size;
		if (nfds == 0 || size / sizeof(fds) != nfds || end < start)
			return 0;
		if (abbrev(tcp)) {
			abbrev_end = start + max_strlen * sizeof(fds);
			if (abbrev_end < start)
				abbrev_end = end;
		} else {
			abbrev_end = end;
		}

		outptr = outstr;

		for (cur = start; cur < end; cur += sizeof(fds)) {
			if (umoven(tcp, cur, sizeof fds, (char *) &fds) < 0) {
				if (outptr < end_outstr - 2)
					*outptr++ = '?';
				failed = 1;
				break;
			}
			if (!fds.revents)
				continue;
			if (outptr == outstr) {
				*outptr++ = '[';
			} else {
				if (outptr < end_outstr - 3)
					outptr = stpcpy(outptr, ", ");
			}
			if (cur >= abbrev_end) {
				if (outptr < end_outstr - 4)
					outptr = stpcpy(outptr, "...");
				break;
			}
			if (outptr < end_outstr - (sizeof("{fd=%d, revents=") + sizeof(int)*3) + 1)
				outptr += sprintf(outptr, "{fd=%d, revents=", fds.fd);
			flagstr = sprintflags("", pollflags, fds.revents);
			if (outptr < end_outstr - (strlen(flagstr) + 2)) {
				outptr = stpcpy(outptr, flagstr);
				*outptr++ = '}';
			}
		}
		if (failed)
			return 0;

		if (outptr != outstr /* && outptr < end_outstr - 1 (always true)*/)
			*outptr++ = ']';

		*outptr = '\0';
		if (pts) {
			if (outptr < end_outstr - (10 + TIMESPEC_TEXT_BUFSIZE)) {
				outptr = stpcpy(outptr, outptr == outstr ? "left " : ", left ");
				sprint_timespec(outptr, tcp, pts);
			}
		}

		if (outptr == outstr)
			return 0;

		tcp->auxstr = outstr;
		return RVAL_STR;
#undef end_outstr
	}
}

int
sys_poll(struct tcb *tcp)
{
	int rc = decode_poll(tcp, 0);
	if (entering(tcp)) {
# ifdef INFTIM
		if (tcp->u_arg[2] == INFTIM)
			tprints("INFTIM");
		else
# endif
			tprintf("%ld", tcp->u_arg[2]);
	}
	return rc;
}

int
sys_ppoll(struct tcb *tcp)
{
	int rc = decode_poll(tcp, tcp->u_arg[2]);
	if (entering(tcp)) {
		print_timespec(tcp, tcp->u_arg[2]);
		tprints(", ");
		print_sigset(tcp, tcp->u_arg[3], 0);
		tprintf(", %lu", tcp->u_arg[4]);
	}
	return rc;
}

#else /* !HAVE_SYS_POLL_H */
int
sys_poll(struct tcb *tcp)
{
	return 0;
}
#endif
