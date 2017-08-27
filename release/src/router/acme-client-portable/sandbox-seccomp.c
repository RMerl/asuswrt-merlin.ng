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

#include <sys/syscall.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <seccomp.h>

#include "extern.h"

static void
sandbox_violation(int signum, siginfo_t *info, void *ctx)
{
	char cp[256];

	(void)signum;
	(void)ctx;

	/*
	 * This isn't signal-safe, but it's just for debugging.
	 */

	snprintf(cp, sizeof(cp), 
		"The seccomp sandbox has failed, error: %d:%d.\n"
		"Please report this number pair to the author.\n", 
		proccomp, info->si_syscall);
	write(STDERR_FILENO, cp, strlen(cp));
	exit(1);
}

static void
sandbox_child_debugging(void)
{
	struct sigaction act;
	sigset_t mask;

	memset(&act, 0, sizeof(act));
	sigemptyset(&mask);
	sigaddset(&mask, SIGSYS);

	act.sa_sigaction = &sandbox_violation;
	act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGSYS, &act, NULL) == -1)
		fprintf(stderr, "%s: sigaction(SIGSYS): %s\n", 
			__func__, strerror(errno));
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
		fprintf(stderr, "%s: sigprocmask(SIGSYS): %s\n",
			__func__, strerror(errno));
}

static int
sandbox_allow(scmp_filter_ctx ctx, int call)
{

	if (0 == seccomp_rule_add(ctx, SCMP_ACT_ALLOW, call, 0))
		return(1);
	warn("seccomp_rule_add");
	return(0);
}

static int
sandbox_allow_inet(scmp_filter_ctx ctx)
{

	if ( ! sandbox_allow(ctx, SCMP_SYS(socket)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(wait4)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(connect)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(nanosleep)))
		return(0);
	return(1);
}

static int
sandbox_allow_dns(scmp_filter_ctx ctx)
{

	if ( ! sandbox_allow(ctx, SCMP_SYS(bind)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(getsockname)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(recvfrom)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(sendto)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(fcntl)))
		return(0);
	return(1);
}

static int
sandbox_allow_cpath(scmp_filter_ctx ctx)
{

	if ( ! sandbox_allow(ctx, SCMP_SYS(open)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(unlink)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(rename)))
		return(0);
	return(1);
}

static int
sandbox_allow_stdio(scmp_filter_ctx ctx)
{

	if ( ! sandbox_allow(ctx, SCMP_SYS(getpid)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(gettimeofday)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(clock_gettime)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(close)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(time)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(fstat)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(read)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(readv)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(write)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(writev)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(close)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(brk)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(poll)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(select)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(madvise)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(mmap)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(mremap)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(munmap)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(exit_group)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(rt_sigaction)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(rt_sigprocmask)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(rt_sigreturn)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(getrandom)) ||
	     ! sandbox_allow(ctx, SCMP_SYS(sigprocmask)))
		return(0);
	return(1);
}

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
	scmp_filter_ctx	 ctx;

	switch (proccomp) {
	case (COMP_ACCOUNT):
	case (COMP_CERT):
	case (COMP_KEY):
	case (COMP_REVOKE):
	case (COMP__MAX):
		sandbox_child_debugging();
		ctx = seccomp_init(SCMP_ACT_TRAP);
		if (NULL == ctx) {
			warn("seccomp_init");
			return(0);
		}
		if ( ! sandbox_allow_stdio(ctx)) {
			seccomp_release(ctx);
			return(0);
		}
		if (0 != seccomp_load(ctx)) {
			warn("seccomp_load");
			seccomp_release(ctx);
			return(0);
		}
		seccomp_release(ctx);
		break;
	case (COMP_CHALLENGE):
		sandbox_child_debugging();
		ctx = seccomp_init(SCMP_ACT_TRAP);
		if (NULL == ctx) {
			warn("seccomp_init");
			return(0);
		}
		if ( ! sandbox_allow_stdio(ctx)) {
			seccomp_release(ctx);
			return(0);
		}
		if ( ! arg && ! sandbox_allow_cpath(ctx)) {
			seccomp_release(ctx);
			return(0);
		}
		if (0 != seccomp_load(ctx)) {
			warn("seccomp_load");
			seccomp_release(ctx);
			return(0);
		}
		seccomp_release(ctx);
		break;
	case (COMP_DNS):
		sandbox_child_debugging();
		ctx = seccomp_init(SCMP_ACT_TRAP);
		if (NULL == ctx) {
			warn("seccomp_init");
			return(0);
		}
		if ( ! sandbox_allow_stdio(ctx) ||
		     ! sandbox_allow_cpath(ctx) ||
		     ! sandbox_allow_dns(ctx) ||
		     ! sandbox_allow_inet(ctx)) {
			seccomp_release(ctx);
			return(0);
		}
		if (0 != seccomp_load(ctx)) {
			warn("seccomp_load");
			seccomp_release(ctx);
			return(0);
		}
		seccomp_release(ctx);
		break;
	case (COMP_FILE):
		sandbox_child_debugging();
		ctx = seccomp_init(SCMP_ACT_TRAP);
		if (NULL == ctx) {
			warn("seccomp_init");
			return(0);
		}
		if ( ! sandbox_allow_stdio(ctx)) {
			seccomp_release(ctx);
			return(0);
		}
		if ( ! arg && ! sandbox_allow_cpath(ctx)) {
			seccomp_release(ctx);
			return(0);
		}
		if (0 != seccomp_load(ctx)) {
			warn("seccomp_load");
			seccomp_release(ctx);
			return(0);
		}
		seccomp_release(ctx);
		break;
	case (COMP_NET):
		sandbox_child_debugging();
		ctx = seccomp_init(SCMP_ACT_TRAP);
		if (NULL == ctx) {
			warn("seccomp_init");
			return(0);
		}
		if ( ! sandbox_allow_stdio(ctx) ||
		     ! sandbox_allow_inet(ctx)) {
			seccomp_release(ctx);
			return(0);
		}
		if (0 != seccomp_load(ctx)) {
			warn("seccomp_load");
			seccomp_release(ctx);
			return(0);
		}
		seccomp_release(ctx);
		break;
	}
	return(1);
}
