/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
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
#include <stdarg.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <sys/utsname.h>
#ifdef HAVE_PRCTL
# include <sys/prctl.h>
#endif
#if defined(IA64)
# include <asm/ptrace_offsets.h>
#endif
/* In some libc, these aren't declared. Do it ourself: */
extern char **environ;
extern int optind;
extern char *optarg;


#if defined __NR_tkill
# define my_tkill(tid, sig) syscall(__NR_tkill, (tid), (sig))
#else
   /* kill() may choose arbitrarily the target task of the process group
      while we later wait on a that specific TID.  PID process waits become
      TID task specific waits for a process under ptrace(2).  */
# warning "tkill(2) not available, risk of strace hangs!"
# define my_tkill(tid, sig) kill((tid), (sig))
#endif

/* Glue for systems without a MMU that cannot provide fork() */
#if !defined(HAVE_FORK)
# undef NOMMU_SYSTEM
# define NOMMU_SYSTEM 1
#endif
#if NOMMU_SYSTEM
# define fork() vfork()
#endif

cflag_t cflag = CFLAG_NONE;
unsigned int followfork = 0;
unsigned int ptrace_setoptions = 0;
unsigned int xflag = 0;
bool need_fork_exec_workarounds = 0;
bool debug_flag = 0;
bool Tflag = 0;
unsigned int qflag = 0;
/* Which WSTOPSIG(status) value marks syscall traps? */
static unsigned int syscall_trap_sig = SIGTRAP;
static unsigned int tflag = 0;
static bool iflag = 0;
static bool rflag = 0;
static bool print_pid_pfx = 0;

/* -I n */
enum {
    INTR_NOT_SET        = 0,
    INTR_ANYWHERE       = 1, /* don't block/ignore any signals */
    INTR_WHILE_WAIT     = 2, /* block fatal signals while decoding syscall. default */
    INTR_NEVER          = 3, /* block fatal signals. default if '-o FILE PROG' */
    INTR_BLOCK_TSTP_TOO = 4, /* block fatal signals and SIGTSTP (^Z) */
    NUM_INTR_OPTS
};
static int opt_intr;
/* We play with signal mask only if this mode is active: */
#define interactive (opt_intr == INTR_WHILE_WAIT)

/*
 * daemonized_tracer supports -D option.
 * With this option, strace forks twice.
 * Unlike normal case, with -D *grandparent* process exec's,
 * becoming a traced process. Child exits (this prevents traced process
 * from having children it doesn't expect to have), and grandchild
 * attaches to grandparent similarly to strace -p PID.
 * This allows for more transparent interaction in cases
 * when process and its parent are communicating via signals,
 * wait() etc. Without -D, strace process gets lodged in between,
 * disrupting parent<->child link.
 */
static bool daemonized_tracer = 0;

#if USE_SEIZE
static int post_attach_sigstop = TCB_IGNORE_ONE_SIGSTOP;
# define use_seize (post_attach_sigstop == 0)
#else
# define post_attach_sigstop TCB_IGNORE_ONE_SIGSTOP
# define use_seize 0
#endif

/* Sometimes we want to print only succeeding syscalls. */
bool not_failing_only = 0;

/* Show path associated with fd arguments */
bool show_fd_path = 0;

static bool detach_on_execve = 0;
/* Are we "strace PROG" and need to skip detach on first execve? */
static bool skip_one_b_execve = 0;
/* Are we "strace PROG" and need to hide everything until execve? */
bool hide_log_until_execve = 0;

static int exit_code = 0;
static int strace_child = 0;
static int strace_tracer_pid = 0;

static char *username = NULL;
static uid_t run_uid;
static gid_t run_gid;

unsigned int max_strlen = DEFAULT_STRLEN;
static int acolumn = DEFAULT_ACOLUMN;
static char *acolumn_spaces;

static char *outfname = NULL;
/* If -ff, points to stderr. Else, it's our common output log */
static FILE *shared_log;

struct tcb *printing_tcp = NULL;
static struct tcb *current_tcp;

static struct tcb **tcbtab;
static unsigned int nprocs, tcbtabsize;
static const char *progname;

unsigned os_release; /* generated from uname()'s u.release */

static int detach(struct tcb *tcp);
static int trace(void);
static void cleanup(void);
static void interrupt(int sig);
static sigset_t empty_set, blocked_set;

#ifdef HAVE_SIG_ATOMIC_T
static volatile sig_atomic_t interrupted;
#else
static volatile int interrupted;
#endif

#ifndef HAVE_STRERROR

#if !HAVE_DECL_SYS_ERRLIST
extern int sys_nerr;
extern char *sys_errlist[];
#endif

const char *
strerror(int err_no)
{
	static char buf[sizeof("Unknown error %d") + sizeof(int)*3];

	if (err_no < 1 || err_no >= sys_nerr) {
		sprintf(buf, "Unknown error %d", err_no);
		return buf;
	}
	return sys_errlist[err_no];
}

#endif /* HAVE_STERRROR */

static void
usage(FILE *ofp, int exitval)
{
	fprintf(ofp, "\
usage: strace [-CdffhiqrtttTvVxxy] [-I n] [-e expr]...\n\
              [-a column] [-o file] [-s strsize] [-P path]...\n\
              -p pid... / [-D] [-E var=val]... [-u username] PROG [ARGS]\n\
   or: strace -c[df] [-I n] [-e expr]... [-O overhead] [-S sortby]\n\
              -p pid... / [-D] [-E var=val]... [-u username] PROG [ARGS]\n\
-c -- count time, calls, and errors for each syscall and report summary\n\
-C -- like -c but also print regular output\n\
-d -- enable debug output to stderr\n\
-D -- run tracer process as a detached grandchild, not as parent\n\
-f -- follow forks, -ff -- with output into separate files\n\
-i -- print instruction pointer at time of syscall\n\
-q -- suppress messages about attaching, detaching, etc.\n\
-r -- print relative timestamp, -t -- absolute timestamp, -tt -- with usecs\n\
-T -- print time spent in each syscall\n\
-v -- verbose mode: print unabbreviated argv, stat, termios, etc. args\n\
-x -- print non-ascii strings in hex, -xx -- print all strings in hex\n\
-y -- print paths associated with file descriptor arguments\n\
-h -- print help message, -V -- print version\n\
-a column -- alignment COLUMN for printing syscall results (default %d)\n\
-b execve -- detach on this syscall\n\
-e expr -- a qualifying expression: option=[!]all or option=[!]val1[,val2]...\n\
   options: trace, abbrev, verbose, raw, signal, read, write\n\
-I interruptible --\n\
   1: no signals are blocked\n\
   2: fatal signals are blocked while decoding syscall (default)\n\
   3: fatal signals are always blocked (default if '-o FILE PROG')\n\
   4: fatal signals and SIGTSTP (^Z) are always blocked\n\
      (useful to make 'strace -o FILE PROG' not stop on ^Z)\n\
-o file -- send trace output to FILE instead of stderr\n\
-O overhead -- set overhead for tracing syscalls to OVERHEAD usecs\n\
-p pid -- trace process with process id PID, may be repeated\n\
-s strsize -- limit length of print strings to STRSIZE chars (default %d)\n\
-S sortby -- sort syscall counts by: time, calls, name, nothing (default %s)\n\
-u username -- run command as username handling setuid and/or setgid\n\
-E var=val -- put var=val in the environment for command\n\
-E var -- remove var from the environment for command\n\
-P path -- trace accesses to path\n\
"
/* ancient, no one should use it
-F -- attempt to follow vforks (deprecated, use -f)\n\
 */
/* this is broken, so don't document it
-z -- print only succeeding syscalls\n\
 */
, DEFAULT_ACOLUMN, DEFAULT_STRLEN, DEFAULT_SORTBY);
	exit(exitval);
}

static void die(void) __attribute__ ((noreturn));
static void die(void)
{
	if (strace_tracer_pid == getpid()) {
		cflag = 0;
		cleanup();
	}
	exit(1);
}

static void verror_msg(int err_no, const char *fmt, va_list p)
{
	char *msg;

	fflush(NULL);

	/* We want to print entire message with single fprintf to ensure
	 * message integrity if stderr is shared with other programs.
	 * Thus we use vasprintf + single fprintf.
	 */
	msg = NULL;
	if (vasprintf(&msg, fmt, p) >= 0) {
		if (err_no)
			fprintf(stderr, "%s: %s: %s\n", progname, msg, strerror(err_no));
		else
			fprintf(stderr, "%s: %s\n", progname, msg);
		free(msg);
	} else {
		/* malloc in vasprintf failed, try it without malloc */
		fprintf(stderr, "%s: ", progname);
		vfprintf(stderr, fmt, p);
		if (err_no)
			fprintf(stderr, ": %s\n", strerror(err_no));
		else
			putc('\n', stderr);
	}
	/* We don't switch stderr to buffered, thus fprintf(stderr)
	 * always flushes its output and this is not necessary: */
	/* fflush(stderr); */
}

void error_msg(const char *fmt, ...)
{
	va_list p;
	va_start(p, fmt);
	verror_msg(0, fmt, p);
	va_end(p);
}

void error_msg_and_die(const char *fmt, ...)
{
	va_list p;
	va_start(p, fmt);
	verror_msg(0, fmt, p);
	die();
}

void perror_msg(const char *fmt, ...)
{
	va_list p;
	va_start(p, fmt);
	verror_msg(errno, fmt, p);
	va_end(p);
}

void perror_msg_and_die(const char *fmt, ...)
{
	va_list p;
	va_start(p, fmt);
	verror_msg(errno, fmt, p);
	die();
}

void die_out_of_memory(void)
{
	static bool recursed = 0;
	if (recursed)
		exit(1);
	recursed = 1;
	error_msg_and_die("Out of memory");
}

static void
error_opt_arg(int opt, const char *arg)
{
	error_msg_and_die("Invalid -%c argument: '%s'", opt, arg);
}

#if USE_SEIZE
static int
ptrace_attach_or_seize(int pid)
{
	int r;
	if (!use_seize)
		return ptrace(PTRACE_ATTACH, pid, 0, 0);
	r = ptrace(PTRACE_SEIZE, pid, 0, 0);
	if (r)
		return r;
	r = ptrace(PTRACE_INTERRUPT, pid, 0, 0);
	return r;
}
#else
# define ptrace_attach_or_seize(pid) ptrace(PTRACE_ATTACH, (pid), 0, 0)
#endif

/*
 * Used when we want to unblock stopped traced process.
 * Should be only used with PTRACE_CONT, PTRACE_DETACH and PTRACE_SYSCALL.
 * Returns 0 on success or if error was ESRCH
 * (presumably process was killed while we talk to it).
 * Otherwise prints error message and returns -1.
 */
static int
ptrace_restart(int op, struct tcb *tcp, int sig)
{
	int err;
	const char *msg;

	errno = 0;
	ptrace(op, tcp->pid, (void *) 0, (long) sig);
	err = errno;
	if (!err)
		return 0;

	msg = "SYSCALL";
	if (op == PTRACE_CONT)
		msg = "CONT";
	if (op == PTRACE_DETACH)
		msg = "DETACH";
#ifdef PTRACE_LISTEN
	if (op == PTRACE_LISTEN)
		msg = "LISTEN";
#endif
	/*
	 * Why curcol != 0? Otherwise sometimes we get this:
	 *
	 * 10252 kill(10253, SIGKILL)              = 0
	 *  <ptrace(SYSCALL,10252):No such process>10253 ...next decode...
	 *
	 * 10252 died after we retrieved syscall exit data,
	 * but before we tried to restart it. Log looks ugly.
	 */
	if (current_tcp && current_tcp->curcol != 0) {
		tprintf(" <ptrace(%s):%s>\n", msg, strerror(err));
		line_ended();
	}
	if (err == ESRCH)
		return 0;
	errno = err;
	perror_msg("ptrace(PTRACE_%s,pid:%d,sig:%d)", msg, tcp->pid, sig);
	return -1;
}

static void
set_cloexec_flag(int fd)
{
	int flags, newflags;

	flags = fcntl(fd, F_GETFD);
	if (flags < 0) {
		/* Can happen only if fd is bad.
		 * Should never happen: if it does, we have a bug
		 * in the caller. Therefore we just abort
		 * instead of propagating the error.
		 */
		perror_msg_and_die("fcntl(%d, F_GETFD)", fd);
	}

	newflags = flags | FD_CLOEXEC;
	if (flags == newflags)
		return;

	fcntl(fd, F_SETFD, newflags); /* never fails */
}

static void kill_save_errno(pid_t pid, int sig)
{
	int saved_errno = errno;

	(void) kill(pid, sig);
	errno = saved_errno;
}

/*
 * When strace is setuid executable, we have to swap uids
 * before and after filesystem and process management operations.
 */
static void
swap_uid(void)
{
	int euid = geteuid(), uid = getuid();

	if (euid != uid && setreuid(euid, uid) < 0) {
		perror_msg_and_die("setreuid");
	}
}

#if _LFS64_LARGEFILE
# define fopen_for_output fopen64
# define struct_stat struct stat64
# define stat_file stat64
# define struct_dirent struct dirent64
# define read_dir readdir64
# define struct_rlimit struct rlimit64
# define set_rlimit setrlimit64
#else
# define fopen_for_output fopen
# define struct_stat struct stat
# define stat_file stat
# define struct_dirent struct dirent
# define read_dir readdir
# define struct_rlimit struct rlimit
# define set_rlimit setrlimit
#endif

static FILE *
strace_fopen(const char *path)
{
	FILE *fp;

	swap_uid();
	fp = fopen_for_output(path, "w");
	if (!fp)
		perror_msg_and_die("Can't fopen '%s'", path);
	swap_uid();
	set_cloexec_flag(fileno(fp));
	return fp;
}

static int popen_pid = 0;

#ifndef _PATH_BSHELL
# define _PATH_BSHELL "/bin/sh"
#endif

/*
 * We cannot use standard popen(3) here because we have to distinguish
 * popen child process from other processes we trace, and standard popen(3)
 * does not export its child's pid.
 */
static FILE *
strace_popen(const char *command)
{
	FILE *fp;
	int fds[2];

	swap_uid();
	if (pipe(fds) < 0)
		perror_msg_and_die("pipe");

	set_cloexec_flag(fds[1]); /* never fails */

	popen_pid = vfork();
	if (popen_pid == -1)
		perror_msg_and_die("vfork");

	if (popen_pid == 0) {
		/* child */
		close(fds[1]);
		if (fds[0] != 0) {
			if (dup2(fds[0], 0))
				perror_msg_and_die("dup2");
			close(fds[0]);
		}
		execl(_PATH_BSHELL, "sh", "-c", command, NULL);
		perror_msg_and_die("Can't execute '%s'", _PATH_BSHELL);
	}

	/* parent */
	close(fds[0]);
	swap_uid();
	fp = fdopen(fds[1], "w");
	if (!fp)
		die_out_of_memory();
	return fp;
}

void
tprintf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (current_tcp) {
		int n = strace_vfprintf(current_tcp->outf, fmt, args);
		if (n < 0) {
			if (current_tcp->outf != stderr)
				perror_msg("%s", outfname);
		} else
			current_tcp->curcol += n;
	}
	va_end(args);
}

void
tprints(const char *str)
{
	if (current_tcp) {
		int n = fputs_unlocked(str, current_tcp->outf);
		if (n >= 0) {
			current_tcp->curcol += strlen(str);
			return;
		}
		if (current_tcp->outf != stderr)
			perror_msg("%s", outfname);
	}
}

void
line_ended(void)
{
	if (current_tcp) {
		current_tcp->curcol = 0;
		fflush(current_tcp->outf);
	}
	if (printing_tcp) {
		printing_tcp->curcol = 0;
		printing_tcp = NULL;
	}
}

void
printleader(struct tcb *tcp)
{
	/* If -ff, "previous tcb we printed" is always the same as current,
	 * because we have per-tcb output files.
	 */
	if (followfork >= 2)
		printing_tcp = tcp;

	if (printing_tcp) {
		current_tcp = printing_tcp;
		if (printing_tcp->curcol != 0 && (followfork < 2 || printing_tcp == tcp)) {
			/*
			 * case 1: we have a shared log (i.e. not -ff), and last line
			 * wasn't finished (same or different tcb, doesn't matter).
			 * case 2: split log, we are the same tcb, but our last line
			 * didn't finish ("SIGKILL nuked us after syscall entry" etc).
			 */
			tprints(" <unfinished ...>\n");
			printing_tcp->curcol = 0;
		}
	}

	printing_tcp = tcp;
	current_tcp = tcp;
	current_tcp->curcol = 0;

	if (print_pid_pfx)
		tprintf("%-5d ", tcp->pid);
	else if (nprocs > 1 && !outfname)
		tprintf("[pid %5u] ", tcp->pid);

	if (tflag) {
		char str[sizeof("HH:MM:SS")];
		struct timeval tv, dtv;
		static struct timeval otv;

		gettimeofday(&tv, NULL);
		if (rflag) {
			if (otv.tv_sec == 0)
				otv = tv;
			tv_sub(&dtv, &tv, &otv);
			tprintf("%6ld.%06ld ",
				(long) dtv.tv_sec, (long) dtv.tv_usec);
			otv = tv;
		}
		else if (tflag > 2) {
			tprintf("%ld.%06ld ",
				(long) tv.tv_sec, (long) tv.tv_usec);
		}
		else {
			time_t local = tv.tv_sec;
			strftime(str, sizeof(str), "%T", localtime(&local));
			if (tflag > 1)
				tprintf("%s.%06ld ", str, (long) tv.tv_usec);
			else
				tprintf("%s ", str);
		}
	}
	if (iflag)
		printcall(tcp);
}

void
tabto(void)
{
	if (current_tcp->curcol < acolumn)
		tprints(acolumn_spaces + current_tcp->curcol);
}

/* Should be only called directly *after successful attach* to a tracee.
 * Otherwise, "strace -oFILE -ff -p<nonexistant_pid>"
 * may create bogus empty FILE.<nonexistant_pid>, and then die.
 */
static void
newoutf(struct tcb *tcp)
{
	tcp->outf = shared_log; /* if not -ff mode, the same file is for all */
	if (followfork >= 2) {
		char name[520 + sizeof(int) * 3];
		sprintf(name, "%.512s.%u", outfname, tcp->pid);
		tcp->outf = strace_fopen(name);
	}
}

static void
expand_tcbtab(void)
{
	/* Allocate some more TCBs and expand the table.
	   We don't want to relocate the TCBs because our
	   callers have pointers and it would be a pain.
	   So tcbtab is a table of pointers.  Since we never
	   free the TCBs, we allocate a single chunk of many.  */
	int i = tcbtabsize;
	struct tcb *newtcbs = calloc(tcbtabsize, sizeof(newtcbs[0]));
	struct tcb **newtab = realloc(tcbtab, tcbtabsize * 2 * sizeof(tcbtab[0]));
	if (!newtab || !newtcbs)
		die_out_of_memory();
	tcbtabsize *= 2;
	tcbtab = newtab;
	while (i < tcbtabsize)
		tcbtab[i++] = newtcbs++;
}

static struct tcb *
alloctcb(int pid)
{
	int i;
	struct tcb *tcp;

	if (nprocs == tcbtabsize)
		expand_tcbtab();

	for (i = 0; i < tcbtabsize; i++) {
		tcp = tcbtab[i];
		if ((tcp->flags & TCB_INUSE) == 0) {
			memset(tcp, 0, sizeof(*tcp));
			tcp->pid = pid;
			tcp->flags = TCB_INUSE;
#if SUPPORTED_PERSONALITIES > 1
			tcp->currpers = current_personality;
#endif
			nprocs++;
			if (debug_flag)
				fprintf(stderr, "new tcb for pid %d, active tcbs:%d\n", tcp->pid, nprocs);
			return tcp;
		}
	}
	error_msg_and_die("bug in alloctcb");
}

static void
droptcb(struct tcb *tcp)
{
	if (tcp->pid == 0)
		return;

	nprocs--;
	if (debug_flag)
		fprintf(stderr, "dropped tcb for pid %d, %d remain\n", tcp->pid, nprocs);

	if (tcp->outf) {
		if (followfork >= 2) {
			if (tcp->curcol != 0)
				fprintf(tcp->outf, " <detached ...>\n");
			fclose(tcp->outf);
		} else {
			if (printing_tcp == tcp && tcp->curcol != 0)
				fprintf(tcp->outf, " <detached ...>\n");
			fflush(tcp->outf);
		}
	}

	if (current_tcp == tcp)
		current_tcp = NULL;
	if (printing_tcp == tcp)
		printing_tcp = NULL;

	memset(tcp, 0, sizeof(*tcp));
}

/* detach traced process; continue with sig
 * Never call DETACH twice on the same process as both unattached and
 * attached-unstopped processes give the same ESRCH.  For unattached process we
 * would SIGSTOP it and wait for its SIGSTOP notification forever.
 */
static int
detach(struct tcb *tcp)
{
	int error;
	int status, sigstop_expected;

	if (tcp->flags & TCB_BPTSET)
		clearbpt(tcp);

	/*
	 * Linux wrongly insists the child be stopped
	 * before detaching.  Arghh.  We go through hoops
	 * to make a clean break of things.
	 */
#if defined(SPARC)
# undef PTRACE_DETACH
# define PTRACE_DETACH PTRACE_SUNDETACH
#endif

	error = 0;
	sigstop_expected = 0;
	if (tcp->flags & TCB_ATTACHED) {
		/*
		 * We attached but possibly didn't see the expected SIGSTOP.
		 * We must catch exactly one as otherwise the detached process
		 * would be left stopped (process state T).
		 */
		sigstop_expected = (tcp->flags & TCB_IGNORE_ONE_SIGSTOP);
		error = ptrace(PTRACE_DETACH, tcp->pid, (char *) 1, 0);
		if (error == 0) {
			/* On a clear day, you can see forever. */
		}
		else if (errno != ESRCH) {
			/* Shouldn't happen. */
			perror_msg("detach: ptrace(PTRACE_DETACH, ...)");
		}
		else if (my_tkill(tcp->pid, 0) < 0) {
			if (errno != ESRCH)
				perror_msg("detach: checking sanity");
		}
		else if (!sigstop_expected && my_tkill(tcp->pid, SIGSTOP) < 0) {
			if (errno != ESRCH)
				perror_msg("detach: stopping child");
		}
		else
			sigstop_expected = 1;
	}

	if (sigstop_expected) {
		for (;;) {
#ifdef __WALL
			if (waitpid(tcp->pid, &status, __WALL) < 0) {
				if (errno == ECHILD) /* Already gone.  */
					break;
				if (errno != EINVAL) {
					perror_msg("detach: waiting");
					break;
				}
#endif /* __WALL */
				/* No __WALL here.  */
				if (waitpid(tcp->pid, &status, 0) < 0) {
					if (errno != ECHILD) {
						perror_msg("detach: waiting");
						break;
					}
#ifdef __WCLONE
					/* If no processes, try clones.  */
					if (waitpid(tcp->pid, &status, __WCLONE) < 0) {
						if (errno != ECHILD)
							perror_msg("detach: waiting");
						break;
					}
#endif /* __WCLONE */
				}
#ifdef __WALL
			}
#endif
			if (!WIFSTOPPED(status)) {
				/* Au revoir, mon ami. */
				break;
			}
			if (WSTOPSIG(status) == SIGSTOP) {
				ptrace_restart(PTRACE_DETACH, tcp, 0);
				break;
			}
			error = ptrace_restart(PTRACE_CONT, tcp,
					WSTOPSIG(status) == syscall_trap_sig ? 0
					: WSTOPSIG(status));
			if (error < 0)
				break;
		}
	}

	if (!qflag && (tcp->flags & TCB_ATTACHED))
		fprintf(stderr, "Process %u detached\n", tcp->pid);

	droptcb(tcp);

	return error;
}

static void
process_opt_p_list(char *opt)
{
	while (*opt) {
		/*
		 * We accept -p PID,PID; -p "`pidof PROG`"; -p "`pgrep PROG`".
		 * pidof uses space as delim, pgrep uses newline. :(
		 */
		int pid;
		char *delim = opt + strcspn(opt, ", \n\t");
		char c = *delim;

		*delim = '\0';
		pid = string_to_uint(opt);
		if (pid <= 0) {
			error_msg_and_die("Invalid process id: '%s'", opt);
		}
		if (pid == strace_tracer_pid) {
			error_msg_and_die("I'm sorry, I can't let you do that, Dave.");
		}
		*delim = c;
		alloctcb(pid);
		if (c == '\0')
			break;
		opt = delim + 1;
	}
}

static void
startup_attach(void)
{
	int tcbi;
	struct tcb *tcp;

	/*
	 * Block user interruptions as we would leave the traced
	 * process stopped (process state T) if we would terminate in
	 * between PTRACE_ATTACH and wait4() on SIGSTOP.
	 * We rely on cleanup() from this point on.
	 */
	if (interactive)
		sigprocmask(SIG_BLOCK, &blocked_set, NULL);

	if (daemonized_tracer) {
		pid_t pid = fork();
		if (pid < 0) {
			perror_msg_and_die("fork");
		}
		if (pid) { /* parent */
			/*
			 * Wait for grandchild to attach to straced process
			 * (grandparent). Grandchild SIGKILLs us after it attached.
			 * Grandparent's wait() is unblocked by our death,
			 * it proceeds to exec the straced program.
			 */
			pause();
			_exit(0); /* paranoia */
		}
		/* grandchild */
		/* We will be the tracer process. Remember our new pid: */
		strace_tracer_pid = getpid();
	}

	for (tcbi = 0; tcbi < tcbtabsize; tcbi++) {
		tcp = tcbtab[tcbi];

		if (!(tcp->flags & TCB_INUSE))
			continue;

		/* Is this a process we should attach to, but not yet attached? */
		if (tcp->flags & TCB_ATTACHED)
			continue; /* no, we already attached it */

		if (followfork && !daemonized_tracer) {
			char procdir[sizeof("/proc/%d/task") + sizeof(int) * 3];
			DIR *dir;

			sprintf(procdir, "/proc/%d/task", tcp->pid);
			dir = opendir(procdir);
			if (dir != NULL) {
				unsigned int ntid = 0, nerr = 0;
				struct_dirent *de;

				while ((de = read_dir(dir)) != NULL) {
					struct tcb *cur_tcp;
					int tid;

					if (de->d_fileno == 0)
						continue;
					/* we trust /proc filesystem */
					tid = atoi(de->d_name);
					if (tid <= 0)
						continue;
					++ntid;
					if (ptrace_attach_or_seize(tid) < 0) {
						++nerr;
						if (debug_flag)
							fprintf(stderr, "attach to pid %d failed\n", tid);
						continue;
					}
					if (debug_flag)
						fprintf(stderr, "attach to pid %d succeeded\n", tid);
					cur_tcp = tcp;
					if (tid != tcp->pid)
						cur_tcp = alloctcb(tid);
					cur_tcp->flags |= TCB_ATTACHED | TCB_STARTUP | post_attach_sigstop;
					newoutf(cur_tcp);
				}
				closedir(dir);
				if (interactive) {
					sigprocmask(SIG_SETMASK, &empty_set, NULL);
					if (interrupted)
						goto ret;
					sigprocmask(SIG_BLOCK, &blocked_set, NULL);
				}
				ntid -= nerr;
				if (ntid == 0) {
					perror_msg("attach: ptrace(PTRACE_ATTACH, ...)");
					droptcb(tcp);
					continue;
				}
				if (!qflag) {
					fprintf(stderr, ntid > 1
? "Process %u attached with %u threads\n"
: "Process %u attached\n",
						tcp->pid, ntid);
				}
				if (!(tcp->flags & TCB_ATTACHED)) {
					/* -p PID, we failed to attach to PID itself
					 * but did attach to some of its sibling threads.
					 * Drop PID's tcp.
					 */
					droptcb(tcp);
				}
				continue;
			} /* if (opendir worked) */
		} /* if (-f) */
		if (ptrace_attach_or_seize(tcp->pid) < 0) {
			perror_msg("attach: ptrace(PTRACE_ATTACH, ...)");
			droptcb(tcp);
			continue;
		}
		tcp->flags |= TCB_ATTACHED | TCB_STARTUP | post_attach_sigstop;
		newoutf(tcp);
		if (debug_flag)
			fprintf(stderr, "attach to pid %d (main) succeeded\n", tcp->pid);

		if (daemonized_tracer) {
			/*
			 * Make parent go away.
			 * Also makes grandparent's wait() unblock.
			 */
			kill(getppid(), SIGKILL);
		}

		if (!qflag)
			fprintf(stderr,
				"Process %u attached\n",
				tcp->pid);
	} /* for each tcbtab[] */

 ret:
	if (interactive)
		sigprocmask(SIG_SETMASK, &empty_set, NULL);
}

/* Stack-o-phobic exec helper, in the hope to work around
 * NOMMU + "daemonized tracer" difficulty.
 */
struct exec_params {
	int fd_to_close;
	uid_t run_euid;
	gid_t run_egid;
	char **argv;
	char *pathname;
};
static struct exec_params params_for_tracee;
static void __attribute__ ((noinline, noreturn))
exec_or_die(void)
{
	struct exec_params *params = &params_for_tracee;

	if (params->fd_to_close >= 0)
		close(params->fd_to_close);
	if (!daemonized_tracer && !use_seize) {
		if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0) {
			perror_msg_and_die("ptrace(PTRACE_TRACEME, ...)");
		}
	}

	if (username != NULL) {
		/*
		 * It is important to set groups before we
		 * lose privileges on setuid.
		 */
		if (initgroups(username, run_gid) < 0) {
			perror_msg_and_die("initgroups");
		}
		if (setregid(run_gid, params->run_egid) < 0) {
			perror_msg_and_die("setregid");
		}
		if (setreuid(run_uid, params->run_euid) < 0) {
			perror_msg_and_die("setreuid");
		}
	}
	else if (geteuid() != 0)
		if (setreuid(run_uid, run_uid) < 0) {
			perror_msg_and_die("setreuid");
		}

	if (!daemonized_tracer) {
		/*
		 * Induce a ptrace stop. Tracer (our parent)
		 * will resume us with PTRACE_SYSCALL and display
		 * the immediately following execve syscall.
		 * Can't do this on NOMMU systems, we are after
		 * vfork: parent is blocked, stopping would deadlock.
		 */
		if (!NOMMU_SYSTEM)
			kill(getpid(), SIGSTOP);
	} else {
		alarm(3);
		/* we depend on SIGCHLD set to SIG_DFL by init code */
		/* if it happens to be SIG_IGN'ed, wait won't block */
		wait(NULL);
		alarm(0);
	}

	execv(params->pathname, params->argv);
	perror_msg_and_die("exec");
}

static void
startup_child(char **argv)
{
	struct_stat statbuf;
	const char *filename;
	char pathname[MAXPATHLEN];
	int pid;
	struct tcb *tcp;

	filename = argv[0];
	if (strchr(filename, '/')) {
		if (strlen(filename) > sizeof pathname - 1) {
			errno = ENAMETOOLONG;
			perror_msg_and_die("exec");
		}
		strcpy(pathname, filename);
	}
#ifdef USE_DEBUGGING_EXEC
	/*
	 * Debuggers customarily check the current directory
	 * first regardless of the path but doing that gives
	 * security geeks a panic attack.
	 */
	else if (stat_file(filename, &statbuf) == 0)
		strcpy(pathname, filename);
#endif /* USE_DEBUGGING_EXEC */
	else {
		const char *path;
		int m, n, len;

		for (path = getenv("PATH"); path && *path; path += m) {
			const char *colon = strchr(path, ':');
			if (colon) {
				n = colon - path;
				m = n + 1;
			}
			else
				m = n = strlen(path);
			if (n == 0) {
				if (!getcwd(pathname, MAXPATHLEN))
					continue;
				len = strlen(pathname);
			}
			else if (n > sizeof pathname - 1)
				continue;
			else {
				strncpy(pathname, path, n);
				len = n;
			}
			if (len && pathname[len - 1] != '/')
				pathname[len++] = '/';
			strcpy(pathname + len, filename);
			if (stat_file(pathname, &statbuf) == 0 &&
			    /* Accept only regular files
			       with some execute bits set.
			       XXX not perfect, might still fail */
			    S_ISREG(statbuf.st_mode) &&
			    (statbuf.st_mode & 0111))
				break;
		}
	}
	if (stat_file(pathname, &statbuf) < 0) {
		perror_msg_and_die("Can't stat '%s'", filename);
	}

	params_for_tracee.fd_to_close = (shared_log != stderr) ? fileno(shared_log) : -1;
	params_for_tracee.run_euid = (statbuf.st_mode & S_ISUID) ? statbuf.st_uid : run_uid;
	params_for_tracee.run_egid = (statbuf.st_mode & S_ISGID) ? statbuf.st_gid : run_gid;
	params_for_tracee.argv = argv;
	/*
	 * On NOMMU, can be safely freed only after execve in tracee.
	 * It's hard to know when that happens, so we just leak it.
	 */
	params_for_tracee.pathname = NOMMU_SYSTEM ? strdup(pathname) : pathname;

#if defined HAVE_PRCTL && defined PR_SET_PTRACER && defined PR_SET_PTRACER_ANY
	if (daemonized_tracer)
		prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
#endif

	strace_child = pid = fork();
	if (pid < 0) {
		perror_msg_and_die("fork");
	}
	if ((pid != 0 && daemonized_tracer)
	 || (pid == 0 && !daemonized_tracer)
	) {
		/* We are to become the tracee. Two cases:
		 * -D: we are parent
		 * not -D: we are child
		 */
		exec_or_die();
	}

	/* We are the tracer */

	if (!daemonized_tracer) {
		if (!use_seize) {
			/* child did PTRACE_TRACEME, nothing to do in parent */
		} else {
			if (!NOMMU_SYSTEM) {
				/* Wait until child stopped itself */
				int status;
				while (waitpid(pid, &status, WSTOPPED) < 0) {
					if (errno == EINTR)
						continue;
					perror_msg_and_die("waitpid");
				}
				if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP) {
					kill_save_errno(pid, SIGKILL);
					perror_msg_and_die("Unexpected wait status %x", status);
				}
			}
			/* Else: NOMMU case, we have no way to sync.
			 * Just attach to it as soon as possible.
			 * This means that we may miss a few first syscalls...
			 */

			if (ptrace_attach_or_seize(pid)) {
				kill_save_errno(pid, SIGKILL);
				perror_msg_and_die("Can't attach to %d", pid);
			}
			if (!NOMMU_SYSTEM)
				kill(pid, SIGCONT);
		}
		tcp = alloctcb(pid);
		if (!NOMMU_SYSTEM)
			tcp->flags |= TCB_ATTACHED | TCB_STRACE_CHILD | TCB_STARTUP | post_attach_sigstop;
		else
			tcp->flags |= TCB_ATTACHED | TCB_STRACE_CHILD | TCB_STARTUP;
		newoutf(tcp);
	}
	else {
		/* With -D, we are *child* here, IOW: different pid. Fetch it: */
		strace_tracer_pid = getpid();
		/* The tracee is our parent: */
		pid = getppid();
		alloctcb(pid);
		/* attaching will be done later, by startup_attach */
		/* note: we don't do newoutf(tcp) here either! */

		/* NOMMU BUG! -D mode is active, we (child) return,
		 * and we will scribble over parent's stack!
		 * When parent later unpauses, it segfaults.
		 *
		 * We work around it
		 * (1) by declaring exec_or_die() NORETURN,
		 * hopefully compiler will just jump to it
		 * instead of call (won't push anything to stack),
		 * (2) by trying very hard in exec_or_die()
		 * to not use any stack,
		 * (3) having a really big (MAXPATHLEN) stack object
		 * in this function, which creates a "buffer" between
		 * child's and parent's stack pointers.
		 * This may save us if (1) and (2) failed
		 * and compiler decided to use stack in exec_or_die() anyway
		 * (happens on i386 because of stack parameter passing).
		 *
		 * A cleaner solution is to use makecontext + setcontext
		 * to create a genuine separate stack and execute on it.
		 */
	}
}

/*
 * Test whether the kernel support PTRACE_O_TRACECLONE et al options.
 * First fork a new child, call ptrace with PTRACE_SETOPTIONS on it,
 * and then see which options are supported by the kernel.
 */
static int
test_ptrace_setoptions_followfork(void)
{
	int pid, expected_grandchild = 0, found_grandchild = 0;
	const unsigned int test_options = PTRACE_O_TRACECLONE |
					  PTRACE_O_TRACEFORK |
					  PTRACE_O_TRACEVFORK;

	/* Need fork for test. NOMMU has no forks */
	if (NOMMU_SYSTEM)
		goto worked; /* be bold, and pretend that test succeeded */

	pid = fork();
	if (pid < 0)
		perror_msg_and_die("fork");
	if (pid == 0) {
		pid = getpid();
		if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0)
			perror_msg_and_die("%s: PTRACE_TRACEME doesn't work",
					   __func__);
		kill_save_errno(pid, SIGSTOP);
		if (fork() < 0)
			perror_msg_and_die("fork");
		_exit(0);
	}

	while (1) {
		int status, tracee_pid;

		errno = 0;
		tracee_pid = wait(&status);
		if (tracee_pid <= 0) {
			if (errno == EINTR)
				continue;
			if (errno == ECHILD)
				break;
			kill_save_errno(pid, SIGKILL);
			perror_msg_and_die("%s: unexpected wait result %d",
					   __func__, tracee_pid);
		}
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status)) {
				if (tracee_pid != pid)
					kill_save_errno(pid, SIGKILL);
				error_msg_and_die("%s: unexpected exit status %u",
						  __func__, WEXITSTATUS(status));
			}
			continue;
		}
		if (WIFSIGNALED(status)) {
			if (tracee_pid != pid)
				kill_save_errno(pid, SIGKILL);
			error_msg_and_die("%s: unexpected signal %u",
					  __func__, WTERMSIG(status));
		}
		if (!WIFSTOPPED(status)) {
			if (tracee_pid != pid)
				kill_save_errno(tracee_pid, SIGKILL);
			kill_save_errno(pid, SIGKILL);
			error_msg_and_die("%s: unexpected wait status %x",
					  __func__, status);
		}
		if (tracee_pid != pid) {
			found_grandchild = tracee_pid;
			if (ptrace(PTRACE_CONT, tracee_pid, 0, 0) < 0) {
				kill_save_errno(tracee_pid, SIGKILL);
				kill_save_errno(pid, SIGKILL);
				perror_msg_and_die("PTRACE_CONT doesn't work");
			}
			continue;
		}
		switch (WSTOPSIG(status)) {
		case SIGSTOP:
			if (ptrace(PTRACE_SETOPTIONS, pid, 0, test_options) < 0
			    && errno != EINVAL && errno != EIO)
				perror_msg("PTRACE_SETOPTIONS");
			break;
		case SIGTRAP:
			if (status >> 16 == PTRACE_EVENT_FORK) {
				long msg = 0;

				if (ptrace(PTRACE_GETEVENTMSG, pid,
					   NULL, (long) &msg) == 0)
					expected_grandchild = msg;
			}
			break;
		}
		if (ptrace(PTRACE_SYSCALL, pid, 0, 0) < 0) {
			kill_save_errno(pid, SIGKILL);
			perror_msg_and_die("PTRACE_SYSCALL doesn't work");
		}
	}
	if (expected_grandchild && expected_grandchild == found_grandchild) {
 worked:
		ptrace_setoptions |= test_options;
		if (debug_flag)
			fprintf(stderr, "ptrace_setoptions = %#x\n",
				ptrace_setoptions);
		return 0;
	}
	error_msg("Test for PTRACE_O_TRACECLONE failed, "
		  "giving up using this feature.");
	return 1;
}

/*
 * Test whether the kernel support PTRACE_O_TRACESYSGOOD.
 * First fork a new child, call ptrace(PTRACE_SETOPTIONS) on it,
 * and then see whether it will stop with (SIGTRAP | 0x80).
 *
 * Use of this option enables correct handling of user-generated SIGTRAPs,
 * and SIGTRAPs generated by special instructions such as int3 on x86:
 * _start:	.globl	_start
 *		int3
 *		movl	$42, %ebx
 *		movl	$1, %eax
 *		int	$0x80
 * (compile with: "gcc -nostartfiles -nostdlib -o int3 int3.S")
 */
static int
test_ptrace_setoptions_for_all(void)
{
	const unsigned int test_options = PTRACE_O_TRACESYSGOOD |
					  PTRACE_O_TRACEEXEC;
	int pid;
	int it_worked = 0;

	/* Need fork for test. NOMMU has no forks */
	if (NOMMU_SYSTEM)
		goto worked; /* be bold, and pretend that test succeeded */

	pid = fork();
	if (pid < 0)
		perror_msg_and_die("fork");

	if (pid == 0) {
		pid = getpid();
		if (ptrace(PTRACE_TRACEME, 0L, 0L, 0L) < 0)
			/* Note: exits with exitcode 1 */
			perror_msg_and_die("%s: PTRACE_TRACEME doesn't work",
					   __func__);
		kill(pid, SIGSTOP);
		_exit(0); /* parent should see entry into this syscall */
	}

	while (1) {
		int status, tracee_pid;

		errno = 0;
		tracee_pid = wait(&status);
		if (tracee_pid <= 0) {
			if (errno == EINTR)
				continue;
			kill_save_errno(pid, SIGKILL);
			perror_msg_and_die("%s: unexpected wait result %d",
					   __func__, tracee_pid);
		}
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0)
				break;
			error_msg_and_die("%s: unexpected exit status %u",
					  __func__, WEXITSTATUS(status));
		}
		if (WIFSIGNALED(status)) {
			error_msg_and_die("%s: unexpected signal %u",
					  __func__, WTERMSIG(status));
		}
		if (!WIFSTOPPED(status)) {
			kill(pid, SIGKILL);
			error_msg_and_die("%s: unexpected wait status %x",
					  __func__, status);
		}
		if (WSTOPSIG(status) == SIGSTOP) {
			/*
			 * We don't check "options aren't accepted" error.
			 * If it happens, we'll never get (SIGTRAP | 0x80),
			 * and thus will decide to not use the option.
			 * IOW: the outcome of the test will be correct.
			 */
			if (ptrace(PTRACE_SETOPTIONS, pid, 0L, test_options) < 0
			    && errno != EINVAL && errno != EIO)
				perror_msg("PTRACE_SETOPTIONS");
		}
		if (WSTOPSIG(status) == (SIGTRAP | 0x80)) {
			it_worked = 1;
		}
		if (ptrace(PTRACE_SYSCALL, pid, 0L, 0L) < 0) {
			kill_save_errno(pid, SIGKILL);
			perror_msg_and_die("PTRACE_SYSCALL doesn't work");
		}
	}

	if (it_worked) {
 worked:
		syscall_trap_sig = (SIGTRAP | 0x80);
		ptrace_setoptions |= test_options;
		if (debug_flag)
			fprintf(stderr, "ptrace_setoptions = %#x\n",
				ptrace_setoptions);
		return 0;
	}

	error_msg("Test for PTRACE_O_TRACESYSGOOD failed, "
		  "giving up using this feature.");
	return 1;
}

#if USE_SEIZE
static void
test_ptrace_seize(void)
{
	int pid;

	/* Need fork for test. NOMMU has no forks */
	if (NOMMU_SYSTEM) {
		post_attach_sigstop = 0; /* this sets use_seize to 1 */
		return;
	}

	pid = fork();
	if (pid < 0)
		perror_msg_and_die("fork");

	if (pid == 0) {
		pause();
		_exit(0);
	}

	/* PTRACE_SEIZE, unlike ATTACH, doesn't force tracee to trap.  After
	 * attaching tracee continues to run unless a trap condition occurs.
	 * PTRACE_SEIZE doesn't affect signal or group stop state.
	 */
	if (ptrace(PTRACE_SEIZE, pid, 0, 0) == 0) {
		post_attach_sigstop = 0; /* this sets use_seize to 1 */
	} else if (debug_flag) {
		fprintf(stderr, "PTRACE_SEIZE doesn't work\n");
	}

	kill(pid, SIGKILL);

	while (1) {
		int status, tracee_pid;

		errno = 0;
		tracee_pid = waitpid(pid, &status, 0);
		if (tracee_pid <= 0) {
			if (errno == EINTR)
				continue;
			perror_msg_and_die("%s: unexpected wait result %d",
					 __func__, tracee_pid);
		}
		if (WIFSIGNALED(status)) {
			return;
		}
		error_msg_and_die("%s: unexpected wait status %x",
				__func__, status);
	}
}
#else /* !USE_SEIZE */
# define test_ptrace_seize() ((void)0)
#endif

static unsigned
get_os_release(void)
{
	unsigned rel;
	const char *p;
	struct utsname u;
	if (uname(&u) < 0)
		perror_msg_and_die("uname");
	/* u.release has this form: "3.2.9[-some-garbage]" */
	rel = 0;
	p = u.release;
	for (;;) {
		if (!(*p >= '0' && *p <= '9'))
			error_msg_and_die("Bad OS release string: '%s'", u.release);
		/* Note: this open-codes KERNEL_VERSION(): */
		rel = (rel << 8) | atoi(p);
		if (rel >= KERNEL_VERSION(1,0,0))
			break;
		while (*p >= '0' && *p <= '9')
			p++;
		if (*p != '.') {
			if (rel >= KERNEL_VERSION(0,1,0)) {
				/* "X.Y-something" means "X.Y.0" */
				rel <<= 8;
				break;
			}
			error_msg_and_die("Bad OS release string: '%s'", u.release);
		}
		p++;
	}
	return rel;
}

/*
 * Initialization part of main() was eating much stack (~0.5k),
 * which was unused after init.
 * We can reuse it if we move init code into a separate function.
 *
 * Don't want main() to inline us and defeat the reason
 * we have a separate function.
 */
static void __attribute__ ((noinline))
init(int argc, char *argv[])
{
	struct tcb *tcp;
	int c, i;
	int optF = 0;
	struct sigaction sa;

	progname = argv[0] ? argv[0] : "strace";

	/* Make sure SIGCHLD has the default action so that waitpid
	   definitely works without losing track of children.  The user
	   should not have given us a bogus state to inherit, but he might
	   have.  Arguably we should detect SIG_IGN here and pass it on
	   to children, but probably noone really needs that.  */
	signal(SIGCHLD, SIG_DFL);

	strace_tracer_pid = getpid();

	os_release = get_os_release();

	/* Allocate the initial tcbtab.  */
	tcbtabsize = argc;	/* Surely enough for all -p args.  */
	tcbtab = calloc(tcbtabsize, sizeof(tcbtab[0]));
	if (!tcbtab)
		die_out_of_memory();
	tcp = calloc(tcbtabsize, sizeof(*tcp));
	if (!tcp)
		die_out_of_memory();
	for (c = 0; c < tcbtabsize; c++)
		tcbtab[c] = tcp++;

	shared_log = stderr;
	set_sortby(DEFAULT_SORTBY);
	set_personality(DEFAULT_PERSONALITY);
	qualify("trace=all");
	qualify("abbrev=all");
	qualify("verbose=all");
#if DEFAULT_QUAL_FLAGS != (QUAL_TRACE | QUAL_ABBREV | QUAL_VERBOSE)
# error Bug in DEFAULT_QUAL_FLAGS
#endif
	qualify("signal=all");
	while ((c = getopt(argc, argv,
		"+b:cCdfFhiqrtTvVxyz"
		"D"
		"a:e:o:O:p:s:S:u:E:P:I:")) != EOF) {
		switch (c) {
		case 'b':
			if (strcmp(optarg, "execve") != 0)
				error_msg_and_die("Syscall '%s' for -b isn't supported",
					optarg);
			detach_on_execve = 1;
			break;
		case 'c':
			if (cflag == CFLAG_BOTH) {
				error_msg_and_die("-c and -C are mutually exclusive");
			}
			cflag = CFLAG_ONLY_STATS;
			break;
		case 'C':
			if (cflag == CFLAG_ONLY_STATS) {
				error_msg_and_die("-c and -C are mutually exclusive");
			}
			cflag = CFLAG_BOTH;
			break;
		case 'd':
			debug_flag = 1;
			break;
		case 'D':
			daemonized_tracer = 1;
			break;
		case 'F':
			optF = 1;
			break;
		case 'f':
			followfork++;
			break;
		case 'h':
			usage(stdout, 0);
			break;
		case 'i':
			iflag = 1;
			break;
		case 'q':
			qflag++;
			break;
		case 'r':
			rflag = 1;
			/* fall through to tflag++ */
		case 't':
			tflag++;
			break;
		case 'T':
			Tflag = 1;
			break;
		case 'x':
			xflag++;
			break;
		case 'y':
			show_fd_path = 1;
			break;
		case 'v':
			qualify("abbrev=none");
			break;
		case 'V':
			printf("%s -- version %s\n", PACKAGE_NAME, VERSION);
			exit(0);
			break;
		case 'z':
			not_failing_only = 1;
			break;
		case 'a':
			acolumn = string_to_uint(optarg);
			if (acolumn < 0)
				error_opt_arg(c, optarg);
			break;
		case 'e':
			qualify(optarg);
			break;
		case 'o':
			outfname = strdup(optarg);
			break;
		case 'O':
			i = string_to_uint(optarg);
			if (i < 0)
				error_opt_arg(c, optarg);
			set_overhead(i);
			break;
		case 'p':
			process_opt_p_list(optarg);
			break;
		case 'P':
			pathtrace_select(optarg);
			break;
		case 's':
			i = string_to_uint(optarg);
			if (i < 0)
				error_opt_arg(c, optarg);
			max_strlen = i;
			break;
		case 'S':
			set_sortby(optarg);
			break;
		case 'u':
			username = strdup(optarg);
			break;
		case 'E':
			if (putenv(optarg) < 0)
				die_out_of_memory();
			break;
		case 'I':
			opt_intr = string_to_uint(optarg);
			if (opt_intr <= 0 || opt_intr >= NUM_INTR_OPTS)
				error_opt_arg(c, optarg);
			break;
		default:
			usage(stderr, 1);
			break;
		}
	}
	argv += optind;
	/* argc -= optind; - no need, argc is not used below */

	acolumn_spaces = malloc(acolumn + 1);
	if (!acolumn_spaces)
		die_out_of_memory();
	memset(acolumn_spaces, ' ', acolumn);
	acolumn_spaces[acolumn] = '\0';

	/* Must have PROG [ARGS], or -p PID. Not both. */
	if (!argv[0] == !nprocs)
		usage(stderr, 1);

	if (nprocs != 0 && daemonized_tracer) {
		error_msg_and_die("-D and -p are mutually exclusive");
	}

	if (!followfork)
		followfork = optF;

	if (followfork >= 2 && cflag) {
		error_msg_and_die("(-c or -C) and -ff are mutually exclusive");
	}

	/* See if they want to run as another user. */
	if (username != NULL) {
		struct passwd *pent;

		if (getuid() != 0 || geteuid() != 0) {
			error_msg_and_die("You must be root to use the -u option");
		}
		pent = getpwnam(username);
		if (pent == NULL) {
			error_msg_and_die("Cannot find user '%s'", username);
		}
		run_uid = pent->pw_uid;
		run_gid = pent->pw_gid;
	}
	else {
		run_uid = getuid();
		run_gid = getgid();
	}

	/*
	 * On any reasonably recent Linux kernel (circa about 2.5.46)
	 * need_fork_exec_workarounds should stay 0 after these tests:
	 */
	/*need_fork_exec_workarounds = 0; - already is */
	if (followfork)
		need_fork_exec_workarounds = test_ptrace_setoptions_followfork();
	need_fork_exec_workarounds |= test_ptrace_setoptions_for_all();
	test_ptrace_seize();

	/* Check if they want to redirect the output. */
	if (outfname) {
		/* See if they want to pipe the output. */
		if (outfname[0] == '|' || outfname[0] == '!') {
			/*
			 * We can't do the <outfname>.PID funny business
			 * when using popen, so prohibit it.
			 */
			if (followfork >= 2)
				error_msg_and_die("Piping the output and -ff are mutually exclusive");
			shared_log = strace_popen(outfname + 1);
		}
		else if (followfork < 2)
			shared_log = strace_fopen(outfname);
	} else {
		/* -ff without -o FILE is the same as single -f */
		if (followfork >= 2)
			followfork = 1;
	}

	if (!outfname || outfname[0] == '|' || outfname[0] == '!') {
		char *buf = malloc(BUFSIZ);
		if (!buf)
			die_out_of_memory();
		setvbuf(shared_log, buf, _IOLBF, BUFSIZ);
	}
	if (outfname && argv[0]) {
		if (!opt_intr)
			opt_intr = INTR_NEVER;
		qflag = 1;
	}
	if (!opt_intr)
		opt_intr = INTR_WHILE_WAIT;

	/* argv[0]	-pPID	-oFILE	Default interactive setting
	 * yes		0	0	INTR_WHILE_WAIT
	 * no		1	0	INTR_WHILE_WAIT
	 * yes		0	1	INTR_NEVER
	 * no		1	1	INTR_WHILE_WAIT
	 */

	sigemptyset(&empty_set);
	sigemptyset(&blocked_set);

	/* startup_child() must be called before the signal handlers get
	 * installed below as they are inherited into the spawned process.
	 * Also we do not need to be protected by them as during interruption
	 * in the startup_child() mode we kill the spawned process anyway.
	 */
	if (argv[0]) {
		if (!NOMMU_SYSTEM || daemonized_tracer)
			hide_log_until_execve = 1;
		skip_one_b_execve = 1;
		startup_child(argv);
	}

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGTTOU, &sa, NULL); /* SIG_IGN */
	sigaction(SIGTTIN, &sa, NULL); /* SIG_IGN */
	if (opt_intr != INTR_ANYWHERE) {
		if (opt_intr == INTR_BLOCK_TSTP_TOO)
			sigaction(SIGTSTP, &sa, NULL); /* SIG_IGN */
		/*
		 * In interactive mode (if no -o OUTFILE, or -p PID is used),
		 * fatal signals are blocked while syscall stop is processed,
		 * and acted on in between, when waiting for new syscall stops.
		 * In non-interactive mode, signals are ignored.
		 */
		if (opt_intr == INTR_WHILE_WAIT) {
			sigaddset(&blocked_set, SIGHUP);
			sigaddset(&blocked_set, SIGINT);
			sigaddset(&blocked_set, SIGQUIT);
			sigaddset(&blocked_set, SIGPIPE);
			sigaddset(&blocked_set, SIGTERM);
			sa.sa_handler = interrupt;
		}
		/* SIG_IGN, or set handler for these */
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		sigaction(SIGPIPE, &sa, NULL);
		sigaction(SIGTERM, &sa, NULL);
	}
	if (nprocs != 0 || daemonized_tracer)
		startup_attach();

	/* Do we want pids printed in our -o OUTFILE?
	 * -ff: no (every pid has its own file); or
	 * -f: yes (there can be more pids in the future); or
	 * -p PID1,PID2: yes (there are already more than one pid)
	 */
	print_pid_pfx = (outfname && followfork < 2 && (followfork == 1 || nprocs > 1));
}

static struct tcb *
pid2tcb(int pid)
{
	int i;

	if (pid <= 0)
		return NULL;

	for (i = 0; i < tcbtabsize; i++) {
		struct tcb *tcp = tcbtab[i];
		if (tcp->pid == pid && (tcp->flags & TCB_INUSE))
			return tcp;
	}

	return NULL;
}

static void
cleanup(void)
{
	int i;
	struct tcb *tcp;
	int fatal_sig;

	/* 'interrupted' is a volatile object, fetch it only once */
	fatal_sig = interrupted;
	if (!fatal_sig)
		fatal_sig = SIGTERM;

	for (i = 0; i < tcbtabsize; i++) {
		tcp = tcbtab[i];
		if (!(tcp->flags & TCB_INUSE))
			continue;
		if (debug_flag)
			fprintf(stderr,
				"cleanup: looking at pid %u\n", tcp->pid);
		if (tcp->flags & TCB_STRACE_CHILD) {
			kill(tcp->pid, SIGCONT);
			kill(tcp->pid, fatal_sig);
		}
		detach(tcp);
	}
	if (cflag)
		call_summary(shared_log);
}

static void
interrupt(int sig)
{
	interrupted = sig;
}

static int
trace(void)
{
	struct rusage ru;
	struct rusage *rup = cflag ? &ru : NULL;
#ifdef __WALL
	static int wait4_options = __WALL;
#endif

	while (nprocs != 0) {
		int pid;
		int wait_errno;
		int status, sig;
		int stopped;
		struct tcb *tcp;
		unsigned event;

		if (interrupted)
			return 0;
		if (interactive)
			sigprocmask(SIG_SETMASK, &empty_set, NULL);
#ifdef __WALL
		pid = wait4(-1, &status, wait4_options, rup);
		if (pid < 0 && (wait4_options & __WALL) && errno == EINVAL) {
			/* this kernel does not support __WALL */
			wait4_options &= ~__WALL;
			pid = wait4(-1, &status, wait4_options, rup);
		}
		if (pid < 0 && !(wait4_options & __WALL) && errno == ECHILD) {
			/* most likely a "cloned" process */
			pid = wait4(-1, &status, __WCLONE, rup);
			if (pid < 0) {
				perror_msg("wait4(__WCLONE) failed");
			}
		}
#else
		pid = wait4(-1, &status, 0, rup);
#endif /* __WALL */
		wait_errno = errno;
		if (interactive)
			sigprocmask(SIG_BLOCK, &blocked_set, NULL);

		if (pid < 0) {
			switch (wait_errno) {
			case EINTR:
				continue;
			case ECHILD:
				/*
				 * We would like to verify this case
				 * but sometimes a race in Solbourne's
				 * version of SunOS sometimes reports
				 * ECHILD before sending us SIGCHILD.
				 */
				return 0;
			default:
				errno = wait_errno;
				perror_msg("wait");
				return -1;
			}
		}
		if (pid == popen_pid) {
			if (WIFEXITED(status) || WIFSIGNALED(status))
				popen_pid = 0;
			continue;
		}

		event = ((unsigned)status >> 16);
		if (debug_flag) {
			char buf[sizeof("WIFEXITED,exitcode=%u") + sizeof(int)*3 /*paranoia:*/ + 16];
			char evbuf[sizeof(",PTRACE_EVENT_?? (%u)") + sizeof(int)*3 /*paranoia:*/ + 16];
			strcpy(buf, "???");
			if (WIFSIGNALED(status))
#ifdef WCOREDUMP
				sprintf(buf, "WIFSIGNALED,%ssig=%s",
						WCOREDUMP(status) ? "core," : "",
						signame(WTERMSIG(status)));
#else
				sprintf(buf, "WIFSIGNALED,sig=%s",
						signame(WTERMSIG(status)));
#endif
			if (WIFEXITED(status))
				sprintf(buf, "WIFEXITED,exitcode=%u", WEXITSTATUS(status));
			if (WIFSTOPPED(status))
				sprintf(buf, "WIFSTOPPED,sig=%s", signame(WSTOPSIG(status)));
#ifdef WIFCONTINUED
			if (WIFCONTINUED(status))
				strcpy(buf, "WIFCONTINUED");
#endif
			evbuf[0] = '\0';
			if (event != 0) {
				static const char *const event_names[] = {
					[PTRACE_EVENT_CLONE] = "CLONE",
					[PTRACE_EVENT_FORK]  = "FORK",
					[PTRACE_EVENT_VFORK] = "VFORK",
					[PTRACE_EVENT_VFORK_DONE] = "VFORK_DONE",
					[PTRACE_EVENT_EXEC]  = "EXEC",
					[PTRACE_EVENT_EXIT]  = "EXIT",
				};
				const char *e;
				if (event < ARRAY_SIZE(event_names))
					e = event_names[event];
				else {
					sprintf(buf, "?? (%u)", event);
					e = buf;
				}
				sprintf(evbuf, ",PTRACE_EVENT_%s", e);
			}
			fprintf(stderr, " [wait(0x%04x) = %u] %s%s\n", status, pid, buf, evbuf);
		}

		/* Look up 'pid' in our table. */
		tcp = pid2tcb(pid);

		if (!tcp) {
			if (followfork) {
				tcp = alloctcb(pid);
				tcp->flags |= TCB_ATTACHED | TCB_STARTUP | post_attach_sigstop;
				newoutf(tcp);
				if (!qflag)
					fprintf(stderr, "Process %d attached\n",
						pid);
			} else {
				/* This can happen if a clone call used
				   CLONE_PTRACE itself.  */
				if (WIFSTOPPED(status))
					ptrace(PTRACE_CONT, pid, (char *) 0, 0);
				error_msg_and_die("Unknown pid: %u", pid);
			}
		}

		clear_regs();
		if (WIFSTOPPED(status))
			get_regs(pid);

		/* Under Linux, execve changes pid to thread leader's pid,
		 * and we see this changed pid on EVENT_EXEC and later,
		 * execve sysexit. Leader "disappears" without exit
		 * notification. Let user know that, drop leader's tcb,
		 * and fix up pid in execve thread's tcb.
		 * Effectively, execve thread's tcb replaces leader's tcb.
		 *
		 * BTW, leader is 'stuck undead' (doesn't report WIFEXITED
		 * on exit syscall) in multithreaded programs exactly
		 * in order to handle this case.
		 *
		 * PTRACE_GETEVENTMSG returns old pid starting from Linux 3.0.
		 * On 2.6 and earlier, it can return garbage.
		 */
		if (event == PTRACE_EVENT_EXEC && os_release >= KERNEL_VERSION(3,0,0)) {
			FILE *fp;
			struct tcb *execve_thread;
			long old_pid = 0;

			if (ptrace(PTRACE_GETEVENTMSG, pid, NULL, (long) &old_pid) < 0)
				goto dont_switch_tcbs;
			if (old_pid <= 0 || old_pid == pid)
				goto dont_switch_tcbs;
			execve_thread = pid2tcb(old_pid);
			/* It should be !NULL, but I feel paranoid */
			if (!execve_thread)
				goto dont_switch_tcbs;

			if (execve_thread->curcol != 0) {
				/*
				 * One case we are here is -ff:
				 * try "strace -oLOG -ff test/threaded_execve"
				 */
				fprintf(execve_thread->outf, " <pid changed to %d ...>\n", pid);
				/*execve_thread->curcol = 0; - no need, see code below */
			}
			/* Swap output FILEs (needed for -ff) */
			fp = execve_thread->outf;
			execve_thread->outf = tcp->outf;
			tcp->outf = fp;
			/* And their column positions */
			execve_thread->curcol = tcp->curcol;
			tcp->curcol = 0;
			/* Drop leader, but close execve'd thread outfile (if -ff) */
			droptcb(tcp);
			/* Switch to the thread, reusing leader's outfile and pid */
			tcp = execve_thread;
			tcp->pid = pid;
			if (cflag != CFLAG_ONLY_STATS) {
				printleader(tcp);
				tprintf("+++ superseded by execve in pid %lu +++\n", old_pid);
				line_ended();
				tcp->flags |= TCB_REPRINT;
			}
		}
 dont_switch_tcbs:

		if (event == PTRACE_EVENT_EXEC) {
			if (detach_on_execve && !skip_one_b_execve)
				detach(tcp); /* do "-b execve" thingy */
			skip_one_b_execve = 0;
		}

		/* Set current output file */
		current_tcp = tcp;

		if (cflag) {
			tv_sub(&tcp->dtime, &ru.ru_stime, &tcp->stime);
			tcp->stime = ru.ru_stime;
		}

		if (WIFSIGNALED(status)) {
			if (pid == strace_child)
				exit_code = 0x100 | WTERMSIG(status);
			if (cflag != CFLAG_ONLY_STATS
			 && (qual_flags[WTERMSIG(status)] & QUAL_SIGNAL)
			) {
				printleader(tcp);
#ifdef WCOREDUMP
				tprintf("+++ killed by %s %s+++\n",
					signame(WTERMSIG(status)),
					WCOREDUMP(status) ? "(core dumped) " : "");
#else
				tprintf("+++ killed by %s +++\n",
					signame(WTERMSIG(status)));
#endif
				line_ended();
			}
			droptcb(tcp);
			continue;
		}
		if (WIFEXITED(status)) {
			if (pid == strace_child)
				exit_code = WEXITSTATUS(status);
			if (cflag != CFLAG_ONLY_STATS &&
			    qflag < 2) {
				printleader(tcp);
				tprintf("+++ exited with %d +++\n", WEXITSTATUS(status));
				line_ended();
			}
			droptcb(tcp);
			continue;
		}
		if (!WIFSTOPPED(status)) {
			fprintf(stderr, "PANIC: pid %u not stopped\n", pid);
			droptcb(tcp);
			continue;
		}

		/* Is this the very first time we see this tracee stopped? */
		if (tcp->flags & TCB_STARTUP) {
			if (debug_flag)
				fprintf(stderr, "pid %d has TCB_STARTUP, initializing it\n", tcp->pid);
			tcp->flags &= ~TCB_STARTUP;
			if (tcp->flags & TCB_BPTSET) {
				/*
				 * One example is a breakpoint inherited from
				 * parent through fork().
				 */
				if (clearbpt(tcp) < 0) {
					/* Pretty fatal */
					droptcb(tcp);
					cleanup();
					return -1;
				}
			}
			if (ptrace_setoptions) {
				if (debug_flag)
					fprintf(stderr, "setting opts %x on pid %d\n", ptrace_setoptions, tcp->pid);
				if (ptrace(PTRACE_SETOPTIONS, tcp->pid, NULL, ptrace_setoptions) < 0) {
					if (errno != ESRCH) {
						/* Should never happen, really */
						perror_msg_and_die("PTRACE_SETOPTIONS");
					}
				}
			}
		}

		sig = WSTOPSIG(status);

		if (event != 0) {
			/* Ptrace event */
#if USE_SEIZE
			if (event == PTRACE_EVENT_STOP) {
				/*
				 * PTRACE_INTERRUPT-stop or group-stop.
				 * PTRACE_INTERRUPT-stop has sig == SIGTRAP here.
				 */
				if (sig == SIGSTOP
				 || sig == SIGTSTP
				 || sig == SIGTTIN
				 || sig == SIGTTOU
				) {
					stopped = 1;
					goto show_stopsig;
				}
			}
#endif
			goto restart_tracee_with_sig_0;
		}

		/* Is this post-attach SIGSTOP?
		 * Interestingly, the process may stop
		 * with STOPSIG equal to some other signal
		 * than SIGSTOP if we happend to attach
		 * just before the process takes a signal.
		 */
		if (sig == SIGSTOP && (tcp->flags & TCB_IGNORE_ONE_SIGSTOP)) {
			if (debug_flag)
				fprintf(stderr, "ignored SIGSTOP on pid %d\n", tcp->pid);
			tcp->flags &= ~TCB_IGNORE_ONE_SIGSTOP;
			goto restart_tracee_with_sig_0;
		}

		if (sig != syscall_trap_sig) {
			siginfo_t si;

			/* Nonzero (true) if tracee is stopped by signal
			 * (as opposed to "tracee received signal").
			 * TODO: shouldn't we check for errno == EINVAL too?
			 * We can get ESRCH instead, you know...
			 */
			stopped = (ptrace(PTRACE_GETSIGINFO, pid, 0, (long) &si) < 0);
#if USE_SEIZE
 show_stopsig:
#endif
			if (cflag != CFLAG_ONLY_STATS
			    && !hide_log_until_execve
			    && (qual_flags[sig] & QUAL_SIGNAL)
			   ) {
#if defined(PT_CR_IPSR) && defined(PT_CR_IIP)
				long pc = 0;
				long psr = 0;

				upeek(tcp, PT_CR_IPSR, &psr);
				upeek(tcp, PT_CR_IIP, &pc);

# define PSR_RI	41
				pc += (psr >> PSR_RI) & 0x3;
# define PC_FORMAT_STR	" @ %lx"
# define PC_FORMAT_ARG	, pc
#else
# define PC_FORMAT_STR	""
# define PC_FORMAT_ARG	/* nothing */
#endif
				printleader(tcp);
				if (!stopped) {
					tprintf("--- %s ", signame(sig));
					printsiginfo(&si, verbose(tcp));
					tprintf(PC_FORMAT_STR " ---\n"
						PC_FORMAT_ARG);
				} else
					tprintf("--- stopped by %s" PC_FORMAT_STR " ---\n",
						signame(sig)
						PC_FORMAT_ARG);
				line_ended();
			}

			if (!stopped)
				/* It's signal-delivery-stop. Inject the signal */
				goto restart_tracee;

			/* It's group-stop */
#if USE_SEIZE
			if (use_seize) {
				/*
				 * This ends ptrace-stop, but does *not* end group-stop.
				 * This makes stopping signals work properly on straced process
				 * (that is, process really stops. It used to continue to run).
				 */
				if (ptrace_restart(PTRACE_LISTEN, tcp, 0) < 0) {
					cleanup();
					return -1;
				}
				continue;
			}
			/* We don't have PTRACE_LISTEN support... */
#endif
			goto restart_tracee;
		}

		/* We handled quick cases, we are permitted to interrupt now. */
		if (interrupted)
			return 0;

		/* This should be syscall entry or exit.
		 * (Or it still can be that pesky post-execve SIGTRAP!)
		 * Handle it.
		 */
		if (trace_syscall(tcp) < 0) {
			/* ptrace() failed in trace_syscall().
			 * Likely a result of process disappearing mid-flight.
			 * Observed case: exit_group() or SIGKILL terminating
			 * all processes in thread group.
			 * We assume that ptrace error was caused by process death.
			 * We used to detach(tcp) here, but since we no longer
			 * implement "detach before death" policy/hack,
			 * we can let this process to report its death to us
			 * normally, via WIFEXITED or WIFSIGNALED wait status.
			 */
			continue;
		}
 restart_tracee_with_sig_0:
		sig = 0;
 restart_tracee:
		if (ptrace_restart(PTRACE_SYSCALL, tcp, sig) < 0) {
			cleanup();
			return -1;
		}
	}
	return 0;
}

int
main(int argc, char *argv[])
{
	init(argc, argv);

	/* Run main tracing loop */
	if (trace() < 0)
		return 1;

	cleanup();
	fflush(NULL);
	if (shared_log != stderr)
		fclose(shared_log);
	if (popen_pid) {
		while (waitpid(popen_pid, NULL, 0) < 0 && errno == EINTR)
			;
	}
	if (exit_code > 0xff) {
		/* Avoid potential core file clobbering.  */
		struct_rlimit rlim = {0, 0};
		set_rlimit(RLIMIT_CORE, &rlim);

		/* Child was killed by a signal, mimic that.  */
		exit_code &= 0xff;
		signal(exit_code, SIG_DFL);
		raise(exit_code);
		/* Paranoia - what if this signal is not fatal?
		   Exit with 128 + signo then.  */
		exit_code += 128;
	}

	return exit_code;
}
