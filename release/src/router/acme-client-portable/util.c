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

#include <sys/wait.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

static	volatile sig_atomic_t sig;

static	const char *const comps[COMP__MAX] = {
	"netproc", /* COMP_NET */
	"keyproc", /* COMP_KEY */
	"certproc", /* COMP_CERT */
	"acctproc", /* COMP_ACCOUNT */
	"challengeproc", /* COMP_CHALLENGE */
	"fileproc", /* COMP_FILE */
	"dnsproc", /* COMP_DNS */
	"revokeproc", /* COMP_REVOKE */
};

static	const char *const comms[COMM__MAX] = {
	"req", /* COMM_REQ */
	"thumbprint", /* COMM_THUMB */
	"cert", /* COMM_CERT */
	"payload", /* COMM_PAY */
	"nonce", /* COMM_NONCE */
	"token", /* COMM_TOK */
	"challenge-op", /* COMM_CHNG_OP */
	"challenge-ack", /* COMM_CHNG_ACK */
	"account", /* COMM_ACCT */
	"acctpro-status", /* COMM_ACCT_STAT */
	"csr", /* COMM_CSR */
	"csr-op", /* COMM_CSR_OP */
	"issuer", /* COMM_ISSUER */
	"chain", /* COMM_CHAIN */
	"chain-op", /* COMM_CHAIN_OP */
	"dns", /* COMM_DNS */
	"dnsq", /* COMM_DNSQ */
	"dns-address", /* COMM_DNSA */
	"dns-family", /* COMM_DNSF */
	"dns-length", /* COMM_DNSLEN */
	"keyproc-status", /* COMM_KEY_STAT */
	"revoke-op", /* COMM_REVOKE_OP */
	"revoke-check", /* COMM_REVOKE_CHECK */
	"revoke-response", /* COMM_REVOKE_RESP */
	"challenge-type", /* COMM_CHNG_TYPE */
};

static void
sigpipe(int code)
{

	(void)code;
	sig = 1;
}

/*
 * This will read a long-sized operation.
 * Operations are usually enums, so this should be alright.
 * We return 0 on EOF and LONG_MAX on failure.
 */
long
readop(int fd, enum comm comm)
{
	ssize_t		 ssz;
	long		 op;

	ssz = read(fd, &op, sizeof(long));
	if (ssz < 0) {
		warn("read: %s", comms[comm]);
		return (LONG_MAX);
	} else if (ssz && ssz != sizeof(long)) {
		warnx("short read: %s", comms[comm]);
		return (LONG_MAX);
	} else if (0 == ssz)
		return (0);

	return (op);
}

char *
readstr(int fd, enum comm comm)
{
	size_t	 sz;

	return (readbuf(fd, comm, &sz));
}

/*
 * Read a buffer from the sender.
 * This consists of two parts: the lenght of the buffer, and the buffer
 * itself.
 * We allow the buffer to be binary, but nil-terminate it anyway.
 */
char *
readbuf(int fd, enum comm comm, size_t *sz)
{
	ssize_t		 ssz;
	size_t		 rsz, lsz;
	char		*p = NULL;

	if ((ssz = read(fd, sz, sizeof(size_t))) < 0) {
		warn("read: %s length", comms[comm]);
		return (NULL);
	} else if ((size_t)ssz != sizeof(size_t)) {
		warnx("short read: %s length", comms[comm]);
		return (NULL);
	} else if (*sz > SIZE_MAX - 1) {
		warnx("integer overflow");
		return (NULL);
	} else if (NULL == (p = calloc(1, *sz + 1))) {
		warn("malloc");
		return (NULL);
	}

	/* Catch this over several reads. */

	rsz = 0;
	lsz = *sz;
	while (lsz) {
		if ((ssz = read(fd, p + rsz, lsz)) < 0) {
			warn("read: %s", comms[comm]);
			break;
		} else if (ssz > 0) {
			assert((size_t)ssz <= lsz);
			rsz += (size_t)ssz;
			lsz -= (size_t)ssz;
		}
	}

	if (lsz) {
		warnx("couldn't read buffer: %s", comms[comm]);
		free(p);
		return (NULL);
	}

	return (p);
}

/*
 * Wring a long-value to a communication pipe.
 * Returns 0 if the reader has terminated, -1 on error, 1 on success.
 */
int
writeop(int fd, enum comm comm, long op)
{
	void	(*sigfp)(int);
	ssize_t	 ssz;
	int	 er;

	sigfp = signal(SIGPIPE, sigpipe);

	if ((ssz = write(fd, &op, sizeof(long))) < 0) {
		if (EPIPE != (er = errno))
			warn("write: %s", comms[comm]);
		signal(SIGPIPE, sigfp);
		return (EPIPE == er ? 0 : -1);
	}

	signal(SIGPIPE, sigfp);

	if ((size_t)ssz != sizeof(long)) {
		warnx("short write: %s", comms[comm]);
		return (-1);
	}

	return (1);
}

/*
 * Fully write the given buffer.
 * Returns 0 if the reader has terminated, -1 on error, 1 on success.
 */
int
writebuf(int fd, enum comm comm, const void *v, size_t sz)
{
	ssize_t	 ssz;
	int	 er, rc = -1;
	void	(*sigfp)(int);

	/*
	 * First, try to write the length.
	 * If the other end of the pipe has closed, we allow the short
	 * write to propogate as a return value of zero.
	 * To detect this, catch SIGPIPE.
	 */

	sigfp = signal(SIGPIPE, sigpipe);

	if ((ssz = write(fd, &sz, sizeof(size_t))) < 0) {
		if (EPIPE != (er = errno))
			warn("write: %s length", comms[comm]);
		signal(SIGPIPE, sigfp);
		return (EPIPE == er ? 0 : -1);
	}

	/* Now write errors cause us to bail. */

	if ((size_t)ssz != sizeof(size_t))
		warnx("short write: %s length", comms[comm]);
	else if ((ssz = write(fd, v, sz)) < 0)
		warn("write: %s", comms[comm]);
	else if ((size_t)ssz != sz)
		warnx("short write: %s", comms[comm]);
	else
		rc = 1;

	signal(SIGPIPE, sigfp);
	return (rc);
}

int
writestr(int fd, enum comm comm, const char *v)
{

	return (writebuf(fd, comm, v, strlen(v)));
}

/*
 * Make sure that the given process exits properly, i.e., properly
 * exiting with EXIT_SUCCESS.
 * Returns non-zero on success and zero on failure.
 */
int
checkexit(pid_t pid, enum comp comp)
{
	int		 c, cc;
	const char	*cp;

	if (-1 == waitpid(pid, &c, 0)) {
		warn("waitpid");
		return (0);
	} else if ( ! WIFEXITED(c) && WIFSIGNALED(c)) {
		cp = strsignal(WTERMSIG(c));
		warnx("signal: %s(%u): %s", comps[comp], pid, cp);
		return (0);
	} else if ( ! WIFEXITED(c)) {
		warnx("did not exit: %s(%u)", comps[comp], pid);
		return (0);
	} else if (EXIT_SUCCESS != WEXITSTATUS(c)) {
		cc = WEXITSTATUS(c);
		dodbg("bad exit: %s(%u): %d", comps[comp], pid, cc);
		return (0);
	}

	return (1);
}

/*
 * Make sure that the given process exits properly, i.e., properly
 * exiting with EXIT_SUCCESS *or* 2.
 * Returns non-zero on success and zero on failure and sets the "rc"
 * value to be the exit status.
 */
int
checkexit_ext(int *rc, pid_t pid, enum comp comp)
{
	int		 c;
	const char	*cp;

	*rc = EXIT_FAILURE;

	if (-1 == waitpid(pid, &c, 0)) {
		warn("waitpid");
		return (0);
	}

	if ( ! WIFEXITED(c) && WIFSIGNALED(c)) {
		cp = strsignal(WTERMSIG(c));
		warnx("signal: %s(%u): %s", comps[comp], pid, cp);
		return (0);
	} else if ( ! WIFEXITED(c)) {
		warnx("did not exit: %s(%u)", comps[comp], pid);
		return (0);
	}

	/* Now check extended status. */

	if (EXIT_SUCCESS != (*rc = WEXITSTATUS(c)) && 2 != *rc) {
		dodbg("bad exit: %s(%u): %d", comps[comp], pid, *rc);
		return (0);
	}
	return (1);
}

/*
 * Wrap around asprintf(3), which sometimes nullifies the input values,
 * sometimes not, but always returns <0 on error.
 * Returns NULL on failure or the pointer on success.
 */
char *
doasprintf(const char *fmt, ...)
{
	int	 c;
	char	*cp;
	va_list	 ap;

	va_start(ap, fmt);
	c = vasprintf(&cp, fmt, ap);
	va_end(ap);
	return(c < 0 ? NULL : cp);
}

