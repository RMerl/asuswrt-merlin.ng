dnl Process this file with autoconf to create configure.  Use autoreconf.
AC_PREREQ(2.57)
AC_INIT([strace],
	m4_esyscmd([./git-version-gen .tarball-version]),
	[strace-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([strace.c])
AC_CONFIG_AUX_DIR([.])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign dist-xz no-dist-gzip silent-rules])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS

AC_MSG_CHECKING([for supported architecture])
case "$host_cpu" in
bfin)
	arch=bfin
	AC_DEFINE([BFIN], 1, [Define for the Blackfin architecture.])
	;;
i[[3456]]86|pentium)
	arch=i386
	AC_DEFINE([I386], 1, [Define for the i386 architecture.])
	;;
ia64)
	arch=ia64
	AC_DEFINE([IA64], 1, [Define for the IA64 architecture.])
	;;
m68k)
	arch=m68k
	AC_DEFINE([M68K], 1, [Define for the m68k architecture.])
	;;
sparc64*)
	arch=sparc64
	AC_DEFINE([SPARC64], 1, [Define for the SPARC64 architecture.])
	;;
sparc*)
	arch=sparc
	AC_DEFINE([SPARC], 1, [Define for the SPARC architecture.])
	;;
metag*)
	arch=metag
	AC_DEFINE([METAG], 1, [Define for the Meta architecture.])
	;;
mips*)
	arch=mips
	AC_DEFINE([MIPS], 1, [Define for the MIPS architecture.])
	;;
alpha*)
	arch=alpha
	AC_DEFINE([ALPHA], 1, [Define for the Alpha architecture.])
	;;
powerpc*)
	arch=powerpc
	AC_DEFINE([POWERPC], 1, [Define for the PowerPC architecture.])
	if test $host_cpu = powerpc64; then
		AC_DEFINE([POWERPC64], 1, [Define for the PowerPC64 architecture.])
	fi
	;;
arm*)
	arch=arm
	AC_DEFINE([ARM], 1, [Define for the ARM architecture.])
	;;
aarch64*)
	arch=aarch64
	AC_DEFINE([AARCH64], 1, [Define for the AArch64 architecture.])
	;;
avr32*)
	arch=avr32
	AC_DEFINE([AVR32], 1, [Define for the AVR32 architecture.])
	;;
s390)
	arch=s390
	AC_DEFINE([S390], 1, [Define for the S390 architecture.])
	;;
s390x)
	arch=s390x
	AC_DEFINE([S390X], 1, [Define for the S390x architecture.])
	;;
hppa*|parisc*)
	arch=hppa
	AC_DEFINE([HPPA], 1, [Define for the HPPA architecture.])
	;;
sh64*)
	arch=sh64
	AC_DEFINE([SH64], 1, [Define for the SH64 architecture.])
	;;
sh*)
	arch=sh
	AC_DEFINE([SH], 1, [Define for the SH architecture.])
	;;
x86?64*)
	AC_TRY_COMPILE(
[#ifndef __ILP32__
# error not x32
#endif], [], arch=x32, arch=x86_64)
	if test "$arch" = "x86_64"; then
		AC_DEFINE([X86_64], 1, [Define for the 64bit AMD x86-64 architecture.])
	else
		AC_DEFINE([X32], 1, [Define for the 32bit AMD x86-64 architecture.])
	fi
	;;
cris|crisv10)
	arch=crisv10
	AC_DEFINE([CRISV10], 1, [Define for the CRISv10 architecture.])
	;;
crisv32)
	arch=crisv32
	AC_DEFINE([CRISV32], 1, [Define for the CRISv32 architecture.])
	;;
tile*)
	arch=tile
	AC_DEFINE([TILE], 1, [Define for the Tile architecture])
	;;
microblaze*)
	arch=microblaze
	AC_DEFINE([MICROBLAZE], 1, [Define for the MicroBlaze architecture.])
	;;
or1k*)
	arch=or1k
	AC_DEFINE([OR1K], 1, [Define for the OpenRISC 1000 architecture.])
	;;

xtensa*)
	arch=xtensa
	AC_DEFINE([XTENSA], 1, [Define for the Xtensa architecture])
	;;

*)
	AC_MSG_RESULT([NO!])
	AC_MSG_ERROR([architecture $host_cpu is not supported by strace])
	;;
esac
AC_MSG_RESULT($arch)

AC_SUBST(arch)

AM_CONDITIONAL([I386], [test x$arch = xi386])
AM_CONDITIONAL([X86_64], [test x$arch = xx86_64])
AM_CONDITIONAL([X32], [test x$arch = xx32])

AC_INCLUDEDIR

gl_WARN_ADD([-Wall])
gl_WARN_ADD([-Wwrite-strings])
AC_ARG_ENABLE([gcc-Werror],
  [AS_HELP_STRING([--enable-gcc-Werror], [turn on gcc's -Werror option])],
  [case $enableval in
     yes) gl_WARN_ADD([-Werror]) ;;
     no)  ;;
     *)   AC_MSG_ERROR([bad value $enableval for gcc-Werror option]) ;;
   esac]
)
AC_SUBST([WARN_CFLAGS])

AC_PROG_CPP
AC_PROG_INSTALL
AC_C_CONST
AC_C_BIGENDIAN
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_DIRENT
AC_HEADER_STAT
AC_CHECK_MEMBERS(m4_normalize([
	struct stat.st_aclcnt,
	struct stat.st_blksize,
	struct stat.st_blocks,
	struct stat.st_flags,
	struct stat.st_fstype,
	struct stat.st_gen,
	struct stat.st_level,
	struct stat.st_rdev
]))
AC_STAT64
AC_STATFS64

AC_TYPE_SIGNAL
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_GETGROUPS
AC_HEADER_MAJOR
AC_CHECK_TYPES([sig_atomic_t, siginfo_t],,, [#include <signal.h>])
AC_CHECK_MEMBERS([struct sockaddr_in6.sin6_scope_id],,,
[#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>])
AC_CHECK_TYPES([long long])
AC_LITTLE_ENDIAN_LONG_LONG
AC_OFF_T_IS_LONG_LONG
AC_CHECK_TYPES([struct opthdr],,, [#include <sys/socket.h>])
AC_CHECK_TYPES([struct t_opthdr],,, [#include <sys/tiuser.h>])

AC_CHECK_FUNCS(m4_normalize([
	fork
	if_indextoname
	inet_ntop
	prctl
	process_vm_readv
	sendmsg
	sigaction
	stpcpy
	strerror
	strsignal
	_sys_siglist
	sys_siglist
]))
AC_CHECK_HEADERS(m4_normalize([
	asm/cachectl.h
	asm/sysmips.h
	elf.h
	inttypes.h
	ioctls.h
	linux/capability.h
	linux/perf_event.h
	linux/ptrace.h
	linux/utsname.h
	mqueue.h
	netinet/sctp.h
	poll.h
	stropts.h
	sys/acl.h
	sys/asynch.h
	sys/conf.h
	sys/epoll.h
	sys/filio.h
	sys/ioctl.h
	sys/poll.h
	sys/ptrace.h
	sys/reg.h
	sys/uio.h
	sys/vfs.h
]))
AC_CHECK_HEADERS([linux/icmp.h linux/in6.h linux/netlink.h linux/if_packet.h],
                 [], [], [#include <stddef.h>
#include <sys/socket.h>
#include <asm/types.h>])
AC_CHECK_HEADERS([asm/sigcontext.h], [], [], [#include <signal.h>])
AC_CHECK_TYPES([struct sigcontext_struct,
		struct sigcontext],,, [#include <signal.h>])
AC_CHECK_HEADERS([netinet/tcp.h netinet/udp.h],,, [#include <netinet/in.h>])

AC_CHECK_MEMBERS([struct msghdr.msg_control],,, [#include <sys/socket.h>])

AC_CHECK_MEMBERS([struct T_conn_res.QUEUE_ptr,
                  struct T_conn_res.ACCEPTOR_id],,,
[#include <sys/stream.h>
#include <sys/tihdr.h>])

AC_CHECK_TYPES([struct __old_kernel_stat],,, [#include <asm/stat.h>])

AC_CHECK_TYPES([struct pt_all_user_regs, struct ia64_fpreg],,,
	       [#include <sys/ptrace.h>])

AC_CHECK_TYPES([struct user_desc],,, [#include <asm/ldt.h>])

AC_CHECK_MEMBERS([struct dqblk.dqb_curblocks],,, [#include <sys/quota.h>])

AC_CHECK_MEMBERS([struct sigcontext.sc_hi2],,, [#include <signal.h>
#ifdef HAVE_ASM_SIGCONTEXT_H
# include <asm/sigcontext.h>
#endif])

AC_CHECK_MEMBERS([struct utsname.domainname],,, [#include <sys/utsname.h>])

AC_CHECK_DECLS([sys_errlist])
AC_CHECK_DECLS([sys_siglist, _sys_siglist],,, [#include <signal.h>])
AC_CHECK_DECLS(m4_normalize([
	PTRACE_EVENT_CLONE,
	PTRACE_EVENT_EXEC,
	PTRACE_EVENT_EXIT,
	PTRACE_EVENT_FORK,
	PTRACE_EVENT_VFORK,
	PTRACE_EVENT_VFORK_DONE,
	PTRACE_GETEVENTMSG,
	PTRACE_GETSIGINFO,
	PTRACE_O_TRACECLONE,
	PTRACE_O_TRACEEXEC,
	PTRACE_O_TRACEEXIT,
	PTRACE_O_TRACEFORK,
	PTRACE_O_TRACESYSGOOD,
	PTRACE_O_TRACEVFORK,
	PTRACE_SETOPTIONS
]),,, [#include <sys/ptrace.h>])

AC_CHECK_DECLS(m4_normalize([
	LO_FLAGS_AUTOCLEAR,
	LO_FLAGS_PARTSCAN
]),,, [#include <linux/loop.h>])

AC_CACHE_CHECK([for BLKGETSIZE64], [ac_cv_have_blkgetsize64],
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
#include <stdlib.h>
#include <linux/fs.h>], [return !BLKGETSIZE64;])],
			   [ac_cv_have_blkgetsize64=yes], [ac_cv_have_blkgetsize64=no])]
	if test $ac_cv_have_blkgetsize64 = yes; then
		AC_DEFINE([HAVE_BLKGETSIZE64], [1], [Define to 1 if you have BLKGETSIZE64.])
	fi)

AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([rlim_t],,[#include <sys/resource.h>])

AC_CHECK_HEADERS([libaio.h], [
	AC_CHECK_MEMBERS([struct iocb.u.c.flags],,, [#include <libaio.h>])
	AC_CHECK_DECLS([IO_CMD_PWRITE, IO_CMD_PWRITEV],,, [#include <libaio.h>])
])

AC_PATH_PROG([PERL], [perl])

AC_CONFIG_FILES([Makefile tests/Makefile])
AC_OUTPUT
