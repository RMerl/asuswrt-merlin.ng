/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2008 Vincent Bernat <bernat@luffy.cx>
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

#include "lldpd.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "syscall-names.h"
#include <seccomp.h>

#ifndef SYS_SECCOMP
# define SYS_SECCOMP 1
#endif

#if defined(__i386__)
# define REG_SYSCALL	REG_EAX
# define ARCH_NR	AUDIT_ARCH_I386
#elif defined(__x86_64__)
# define REG_SYSCALL	REG_RAX
# define ARCH_NR	AUDIT_ARCH_X86_64
#else
# error "Platform does not support seccomp filter yet"
# define REG_SYSCALL	0
# define ARCH_NR	0
#endif

/* If there is no privilege separation, seccomp is currently useless */
#ifdef ENABLE_PRIVSEP
static int monitored = -1;
static int trapped = 0;
/**
 * SIGSYS signal handler
 * @param nr the signal number
 * @param info siginfo_t pointer
 * @param void_context handler context
 *
 * Simple signal handler for SIGSYS displaying the error, killing the child and
 * exiting.
 *
 */
static void
priv_seccomp_trap_handler(int signal, siginfo_t *info, void *vctx)
{
	ucontext_t *ctx = (ucontext_t *)(vctx);
	unsigned int syscall;

	if (trapped)
		_exit(161);	/* Avoid loops */

	/* Get details */
	if (info->si_code != SYS_SECCOMP)
		return;
	if (!ctx)
		_exit(161);
	syscall = ctx->uc_mcontext.gregs[REG_SYSCALL];
	trapped = 1;

	/* Log them. Technically, `log_warnx()` is not signal safe, but we are
	 * unlikely to reenter here. */
	log_warnx("seccomp", "invalid syscall attempted: %s(%d)",
	    (syscall < sizeof(syscall_names))?syscall_names[syscall]:"unknown",
	    syscall);

	/* Kill children and exit */
	kill(monitored, SIGTERM);
	fatalx("seccomp", "invalid syscall not allowed: stop here");
	_exit(161);
}

/**
 * Install a TRAP action signal handler
 *
 * This function installs the TRAP action signal handler and is based on
 * examples from Will Drewry and Kees Cook.  Returns zero on success, negative
 * values on failure.
 *
 */
static int
priv_seccomp_trap_install()
{
	struct sigaction signal_handler = {};
	sigset_t signal_mask;

	sigemptyset(&signal_mask);
	sigaddset(&signal_mask, SIGSYS);

	signal_handler.sa_sigaction = &priv_seccomp_trap_handler;
	signal_handler.sa_flags = SA_SIGINFO;
	if (sigaction(SIGSYS, &signal_handler, NULL) < 0)
		return -errno;
	if (sigprocmask(SIG_UNBLOCK, &signal_mask, NULL))
		return -errno;

	return 0;
}

/**
 * Initialize seccomp.
 *
 * @param remote file descriptor to talk with the unprivileged process
 * @param monitored monitored child
 * @return negative on failures or 0 if everything was setup
 */
int
priv_seccomp_init(int remote, int child)
{
	int rc = -1;
	scmp_filter_ctx ctx = NULL;

	log_debug("seccomp", "initialize libseccomp filter");
	monitored = child;
	if (priv_seccomp_trap_install() < 0) {
		log_warn("seccomp", "unable to install SIGSYS handler");
		goto failure_scmp;
	}

	if ((ctx = seccomp_init(SCMP_ACT_TRAP)) == NULL) {
		log_warnx("seccomp", "unable to initialize libseccomp subsystem");
		goto failure_scmp;
	}

	if ((rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW,
		    SCMP_SYS(read), 1, SCMP_CMP(0, SCMP_CMP_EQ, remote))) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW,
		SCMP_SYS(write), 1, SCMP_CMP(0, SCMP_CMP_EQ, remote))) < 0) {
		errno = -rc;
		log_warn("seccomp", "unable to allow read/write on remote socket");
		goto failure_scmp;
	}

	/* We are far more generic from here. */
	if ((rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0)) < 0 || /* write needed for */
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fcntl), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(kill), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(socket), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(bind), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(setsockopt), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getsockname), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(uname), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(unlink), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(sendmsg), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(sendmmsg), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(wait4), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(stat), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0)) < 0 || /* brk needed for newer libc */
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getpid), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(sendto), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(poll), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(recvmsg), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(recvfrom), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(readv), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(sendmmsg), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(clock_gettime), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(gettimeofday), 0)) < 0 ||
	    /* The following are for resolving addresses */
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(connect), 0)) < 0 ||
	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex), 0)) < 0 ||

	    (rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0)) < 0) {
		errno = -rc;
		log_warn("seccomp", "unable to build seccomp rules");
		goto failure_scmp;
	}

	if ((rc = seccomp_load(ctx)) < 0) {
		errno = -rc;
		log_warn("seccomp", "unable to load libseccomp filter");
		goto failure_scmp;
	}

failure_scmp:
	seccomp_release(ctx);
	return rc;
}
#endif
