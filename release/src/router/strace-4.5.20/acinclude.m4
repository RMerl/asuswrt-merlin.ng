dnl
dnl This file contains macros used in configure.ac.
dnl automake uses this file to generate aclocal.m4, which is used by autoconf.
dnl

dnl ### A macro to find the include directory, useful for cross-compiling.
AC_DEFUN([AC_INCLUDEDIR],
[AC_REQUIRE([AC_PROG_AWK])dnl
AC_SUBST(includedir)
AC_MSG_CHECKING(for primary include directory)
includedir=/usr/include
if test -n "$GCC"
then
	>conftest.c
	new_includedir=`
		$CC -v -E conftest.c 2>&1 | $AWK '
			/^End of search list/ { print last; exit }
			{ last = [$]1 }
		'
	`
	rm -f conftest.c
	if test -n "$new_includedir" && test -d "$new_includedir"
	then
		includedir=$new_includedir
	fi
fi
AC_MSG_RESULT($includedir)
])

dnl ### A macro to set gcc warning flags.
define(AC_WARNFLAGS,
[AC_SUBST(WARNFLAGS)
if test -z "$WARNFLAGS"
then
	if test -n "$GCC"
	then
		# If we're using gcc we want warning flags.
		WARNFLAGS=-Wall
	fi
fi
])

dnl ### A macro to determine if we have a "MP" type procfs
AC_DEFUN([AC_MP_PROCFS],
[AC_MSG_CHECKING(for MP procfs)
AC_CACHE_VAL(ac_cv_mp_procfs,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <signal.h>
#include <sys/procfs.h>

main()
{
	int pid;
	char proc[32];
	FILE *ctl;
	FILE *status;
	int cmd;
	struct pstatus pstatus;

	if ((pid = fork()) == 0) {
		pause();
		exit(0);
	}
	sprintf(proc, "/proc/%d/ctl", pid);
	if ((ctl = fopen(proc, "w")) == NULL)
		goto fail;
	sprintf(proc, "/proc/%d/status", pid);
	if ((status = fopen (proc, "r")) == NULL)
		goto fail;
	cmd = PCSTOP;
	if (write (fileno (ctl), &cmd, sizeof cmd) < 0)
		goto fail;
	if (read (fileno (status), &pstatus, sizeof pstatus) < 0)
		goto fail;
	kill(pid, SIGKILL);
	exit(0);
fail:
	kill(pid, SIGKILL);
	exit(1);
}
]])],[ac_cv_mp_procfs=yes],[ac_cv_mp_procfs=no],[
# Guess or punt.
case "$host_os" in
svr4.2*|svr5*)
	ac_cv_mp_procfs=yes
	;;
*)
	ac_cv_mp_procfs=no
	;;
esac
])])
AC_MSG_RESULT($ac_cv_mp_procfs)
if test "$ac_cv_mp_procfs" = yes
then
	AC_DEFINE([HAVE_MP_PROCFS], 1,
[Define if you have a SVR4 MP type procfs.
I.E. /dev/xxx/ctl, /dev/xxx/status.
Also implies that you have the pr_lwp member in prstatus.])
fi
])

dnl ### A macro to determine if procfs is pollable.
AC_DEFUN([AC_POLLABLE_PROCFS],
[AC_MSG_CHECKING(for pollable procfs)
AC_CACHE_VAL(ac_cv_pollable_procfs,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <signal.h>
#include <sys/procfs.h>
#include <sys/stropts.h>
#include <poll.h>

#ifdef HAVE_MP_PROCFS
#define PIOCSTOP	PCSTOP
#define POLLWANT	POLLWRNORM
#define PROC		"/proc/%d/ctl"
#define PROC_MODE	"w"
int IOCTL (int fd, int cmd, int arg) {
	return write (fd, &cmd, sizeof cmd);
}
#else
#define POLLWANT	POLLPRI
#define	PROC		"/proc/%d"
#define PROC_MODE	"r+"
#define IOCTL		ioctl
#endif

main()
{
	int pid;
	char proc[32];
	FILE *pfp;
	struct pollfd pfd;

	if ((pid = fork()) == 0) {
		pause();
		exit(0);
	}
	sprintf(proc, PROC, pid);
	if ((pfp = fopen(proc, PROC_MODE)) == NULL)
		goto fail;
	if (IOCTL(fileno(pfp), PIOCSTOP, NULL) < 0)
		goto fail;
	pfd.fd = fileno(pfp);
	pfd.events = POLLWANT;
	if (poll(&pfd, 1, 0) < 0)
		goto fail;
	if (!(pfd.revents & POLLWANT))
		goto fail;
	kill(pid, SIGKILL);
	exit(0);
fail:
	kill(pid, SIGKILL);
	exit(1);
}
]])],[ac_cv_pollable_procfs=yes],[ac_cv_pollable_procfs=no],[
# Guess or punt.
case "$host_os" in
solaris2*|irix5*|svr4.2uw*|svr5*)
	ac_cv_pollable_procfs=yes
	;;
*)
	ac_cv_pollable_procfs=no
	;;
esac
])])
AC_MSG_RESULT($ac_cv_pollable_procfs)
if test "$ac_cv_pollable_procfs" = yes
then
	AC_DEFINE([HAVE_POLLABLE_PROCFS], 1,
[Define if you have SVR4 and the poll system call works on /proc files.])
fi
])

dnl ### A macro to determine if the prstatus structure has a pr_syscall member.
AC_DEFUN([AC_STRUCT_PR_SYSCALL],
[AC_MSG_CHECKING(for pr_syscall in struct prstatus)
AC_CACHE_VAL(ac_cv_struct_pr_syscall,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/procfs.h>]], [[#ifdef HAVE_MP_PROCFS
pstatus_t s;
s.pr_lwp.pr_syscall
#else
prstatus_t s;
s.pr_syscall
#endif]])],[ac_cv_struct_pr_syscall=yes],[ac_cv_struct_pr_syscall=no])])
AC_MSG_RESULT($ac_cv_struct_pr_syscall)
if test "$ac_cv_struct_pr_syscall" = yes
then
	AC_DEFINE([HAVE_PR_SYSCALL], 1,
[Define if the prstatus structure in sys/procfs.h has a pr_syscall member.])
fi
])

dnl ### A macro to determine whether stat64 is defined.
AC_DEFUN([AC_STAT64],
[AC_MSG_CHECKING(for stat64 in (asm|sys)/stat.h)
AC_CACHE_VAL(ac_cv_type_stat64,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#ifdef LINUX
#include <linux/types.h>
#include <asm/stat.h>
#else
#include <sys/stat.h>
#endif]], [[struct stat64 st;]])],[ac_cv_type_stat64=yes],[ac_cv_type_stat64=no])])
AC_MSG_RESULT($ac_cv_type_stat64)
if test "$ac_cv_type_stat64" = yes
then
	AC_DEFINE([HAVE_STAT64], 1,
[Define if stat64 is available in asm/stat.h.])
fi
])

dnl ### A macro to determine whether statfs64 is defined.
AC_DEFUN([AC_STATFS64],
[AC_MSG_CHECKING(for statfs64 in sys/vfs.h)
AC_CACHE_VAL(ac_cv_type_statfs64,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#ifdef LINUX
#include <linux/types.h>
#include <sys/vfs.h>
#endif]], [[struct statfs64 st;]])],[ac_cv_type_statfs64=yes],[ac_cv_type_statfs64=no])])
AC_MSG_RESULT($ac_cv_type_statfs64)
if test "$ac_cv_type_statfs64" = yes
then
	AC_DEFINE([HAVE_STATFS64], 1,
[Define if statfs64 is available in sys/vfs.h.])
fi
])


dnl ### A macro to determine if off_t is a long long
AC_DEFUN([AC_OFF_T_IS_LONG_LONG],
[AC_MSG_CHECKING(for long long off_t)
AC_CACHE_VAL(ac_cv_have_long_long_off_t,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <sys/types.h>
main () {
	if (sizeof (off_t) == sizeof (long long) &&
	    sizeof (off_t) > sizeof (long))
	    return 0;
	return 1;
}
]])],[ac_cv_have_long_long_off_t=yes],[ac_cv_have_long_long_off_t=no],[# Should try to guess here
ac_cv_have_long_long_off_t=no
])])
AC_MSG_RESULT($ac_cv_have_long_long_off_t)
if test "$ac_cv_have_long_long_off_t" = yes
then
	AC_DEFINE([HAVE_LONG_LONG_OFF_T], 1, [Define if off_t is a long long.])
fi
])

dnl ### A macro to determine if rlim_t is a long long
AC_DEFUN([AC_RLIM_T_IS_LONG_LONG],
[AC_MSG_CHECKING(for long long rlim_t)
AC_CACHE_VAL(ac_cv_have_long_long_rlim_t,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
main () {
	if (sizeof (rlim_t) == sizeof (long long) &&
	    sizeof (rlim_t) > sizeof (long))
	    return 0;
	return 1;
}
]])],[ac_cv_have_long_long_rlim_t=yes],[ac_cv_have_long_long_rlim_t=no],[# Should try to guess here
ac_cv_have_long_long_rlim_t=no
])])
AC_MSG_RESULT($ac_cv_have_long_long_rlim_t)
if test "$ac_cv_have_long_long_rlim_t" = yes
then
	AC_DEFINE([HAVE_LONG_LONG_RLIM_T], 1, [Define if rlim_t is a long long.])
fi
])

dnl ### A macro to determine endianness of long long
AC_DEFUN([AC_LITTLE_ENDIAN_LONG_LONG],
[AC_MSG_CHECKING(for little endian long long)
AC_CACHE_VAL(ac_cv_have_little_endian_long_long,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
int main () {
	union {
		long long ll;
		long l [2];
	} u;
	u.ll = 0x12345678;
	if (u.l[0] == 0x12345678)
		return 0;
	return 1;
}
]])],[ac_cv_have_little_endian_long_long=yes],[ac_cv_have_little_endian_long_long=no],[# Should try to guess here
ac_cv_have_little_endian_long_long=no
])])
AC_MSG_RESULT($ac_cv_have_little_endian_long_long)
if test "$ac_cv_have_little_endian_long_long" = yes
then
	AC_DEFINE([HAVE_LITTLE_ENDIAN_LONG_LONG], 1,
[Define if long long is little-endian.])
fi
])
