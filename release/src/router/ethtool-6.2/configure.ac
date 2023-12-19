dnl Process this file with autoconf to produce a configure script.
AC_INIT(ethtool, 6.2, netdev@vger.kernel.org)
AC_PREREQ(2.52)
AC_CONFIG_SRCDIR([ethtool.c])
AM_INIT_AUTOMAKE([gnu subdir-objects])
AC_CONFIG_HEADERS([ethtool-config.h])

AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_GCC_TRADITIONAL
AM_PROG_CC_C_O
PKG_PROG_PKG_CONFIG

dnl Checks for libraries.

dnl Checks for header files.

dnl Checks for typedefs, structures, and compiler characteristics.
AC_MSG_CHECKING([whether <linux/types.h> defines big-endian types])
AC_TRY_COMPILE([#include <linux/types.h>],
	       [__be16 foo;__be32 bar;],
	       [AC_MSG_RESULT(yes)
		AC_DEFINE([HAVE_BE_TYPES], [1],
			  [Define to 1 if <linux/types.h> defines big-endian types])],
	       [AC_MSG_RESULT(no)])

dnl Checks for library functions.
AC_HEADER_STDC
AC_CHECK_FUNCS(socket strtol)

dnl Check for options
AC_ARG_ENABLE(pretty-dump,
	      [  --enable-pretty-dump	  enable registers, EEPROM and SFP pretty dumps (enabled by default)],
	      ,
	      enable_pretty_dump=yes)
if test x$enable_pretty_dump = xyes; then
    AC_DEFINE(ETHTOOL_ENABLE_PRETTY_DUMP, 1,
	      [Define this to enable register, EEPROM and SFP pretty dumps.])
fi
AM_CONDITIONAL([ETHTOOL_ENABLE_PRETTY_DUMP], [test x$enable_pretty_dump = xyes])

AC_ARG_WITH([bash-completion-dir],
	    AS_HELP_STRING([--with-bash-completion-dir[=PATH]],
	                   [Install the bash-completion script in this directory. @<:@default=yes@:>@]),
	    [],
	    [with_bash_completion_dir=yes])
AS_IF([test "x$with_bash_completion_dir" = xyes],
      [AC_MSG_CHECKING([for bash-completion directory])
       dnl Attempt to use pkg-config completionsdir variable with given $prefix.
       dnl This matches distcheck expectation that all files install to $prefix.
       dnl It works with /usr and /usr/local (for default $XDG_DATA_DIRS) but
       dnl may install to directory not used by bash-completion in other cases.
       dnl See: https://lore.kernel.org/netdev/20190417025333.GA28674@kevinolos/
       AS_IF([test "x$PKG_CONFIG" != x \
	      && bash_completion_prefix=`"$PKG_CONFIG" --print-errors --variable=prefix bash-completion 2>&AS_MESSAGE_LOG_FD` \
	      && bash_completion_dir=`"$PKG_CONFIG" --print-errors --variable=completionsdir bash-completion 2>&AS_MESSAGE_LOG_FD`],
	     [bash_completion_dir="${bash_completion_dir#"$bash_completion_prefix"}"
	      bash_completion_dir="${bash_completion_dir#/}"
	      BASH_COMPLETION_DIR='${prefix}'/"$bash_completion_dir"],
	     [BASH_COMPLETION_DIR='${datadir}/bash-completion/completions'])
       AC_MSG_RESULT([$BASH_COMPLETION_DIR])],
      [BASH_COMPLETION_DIR="$with_bash_completion_dir"])
AC_SUBST([BASH_COMPLETION_DIR])
AM_CONDITIONAL([ENABLE_BASH_COMPLETION],
	       [test "x$with_bash_completion_dir" != xno])

AC_ARG_ENABLE(netlink,
	      [  --enable-netlink	  enable netlink interface (enabled by default)],
	      ,
	      enable_netlink=yes)
if test x$enable_netlink = xyes; then
    PKG_PROG_PKG_CONFIG
    PKG_CHECK_MODULES([MNL], [libmnl])
    AC_DEFINE(ETHTOOL_ENABLE_NETLINK, 1,
	      Define this to support netlink interface to talk to kernel.)
fi
AM_CONDITIONAL([ETHTOOL_ENABLE_NETLINK], [test x$enable_netlink = xyes])

AC_CONFIG_FILES([Makefile ethtool.spec ethtool.8])
AC_OUTPUT
