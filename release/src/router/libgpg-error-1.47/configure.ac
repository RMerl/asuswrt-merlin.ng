# configure.ac for libgpg-error
# Copyright (C) 2003, 2004, 2006, 2010, 2013-2022 g10 Code GmbH
#
# This file is part of libgpg-error.
#
# libgpg-error is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# libgpg-error is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <https://www.gnu.org/licenses/>.
# SPDX-License-Identifier: LGPL-2.1+

# (Process this file with autoconf to produce a configure script.)
# The following lines are used by ./autogen.sh.
AC_PREREQ([2.69])
min_automake_version="1.14"

# To build a release you need to create a tag with the version number
# (git tag -s libgpg-error-n.m) and run "./autogen.sh --force".
# Please bump the version number immediately after the release, do
# another commit, and a push so that the git magic is able to work.
# See below for the LT versions.
m4_define([mym4_package],[libgpg-error])
m4_define([mym4_major], [1])
m4_define([mym4_minor], [47])

# Below is m4 magic to extract and compute the revision number, the
# decimalized short revision number, a beta version string, and a flag
# indicating a development version (mym4_isgit). Note that the m4
# processing is done by autoconf and not during the configure run.
m4_define([mym4_verslist], m4_split(m4_esyscmd([./autogen.sh --find-version] \
                           mym4_package mym4_major mym4_minor),[:]))
m4_define([mym4_isbeta],       m4_argn(2, mym4_verslist))
m4_define([mym4_version],      m4_argn(4, mym4_verslist))
m4_define([mym4_revision],     m4_argn(7, mym4_verslist))
m4_define([mym4_revision_dec], m4_argn(8, mym4_verslist))
m4_esyscmd([echo ]mym4_version[>VERSION])
AC_INIT([mym4_package],[mym4_version],[https://bugs.gnupg.org])
# LT Version numbers, remember to change them just *before* a release.
#   (Code changed:			REVISION++)
#   (Interfaces added/removed/changed:	CURRENT++, REVISION=0)
#   (Interfaces added:			AGE++)
#   (Interfaces removed:		AGE=0)
# Note that added error codes don't constitute an interface change.
LIBGPG_ERROR_LT_CURRENT=34
LIBGPG_ERROR_LT_AGE=34
LIBGPG_ERROR_LT_REVISION=0
################################################

AC_SUBST(LIBGPG_ERROR_LT_CURRENT)
AC_SUBST(LIBGPG_ERROR_LT_AGE)
AC_SUBST(LIBGPG_ERROR_LT_REVISION)

VERSION_NUMBER=m4_esyscmd(printf "0x%02x%02x00" mym4_major mym4_minor)
AC_SUBST(VERSION_NUMBER)

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([serial-tests dist-bzip2])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/err-sources.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES

# We need to know about the host architecture to avoid spurious
# warnings.
AC_CANONICAL_HOST
AB_INIT

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_PROG_AWK
AC_CHECK_TOOL(AR, ar, :)
AC_USE_SYSTEM_EXTENSIONS

AM_LANGINFO_CODESET

# Taken from mpfr-4.0.1, then modified for LDADD_FOR_TESTS_KLUDGE
dnl Under Linux, make sure that the old dtags are used if LD_LIBRARY_PATH
dnl is defined. The issue is that with the new dtags, LD_LIBRARY_PATH has
dnl the precedence over the run path, so that if a compatible MPFR library
dnl is installed in some directory from $LD_LIBRARY_PATH, then the tested
dnl MPFR library will be this library instead of the MPFR library from the
dnl build tree. Other OS with the same issue might be added later.
dnl
dnl References:
dnl   https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=859732
dnl   http://lists.gnu.org/archive/html/libtool/2017-05/msg00000.html
dnl
dnl We need to check whether --disable-new-dtags is supported as alternate
dnl linkers may be used (e.g., with tcc: CC=tcc LD=tcc).
dnl
case $host in
  *-*-linux*)
    if test -n "$LD_LIBRARY_PATH"; then
      saved_LDFLAGS="$LDFLAGS"
      LDADD_FOR_TESTS_KLUDGE="-Wl,--disable-new-dtags"
      LDFLAGS="$LDFLAGS $LDADD_FOR_TESTS_KLUDGE"
      AC_MSG_CHECKING(whether --disable-new-dtags is supported by the linker)
      AC_LINK_IFELSE([AC_LANG_SOURCE([[
int main (void) { return 0; }
      ]])],
      [AC_MSG_RESULT(yes (use it since LD_LIBRARY_PATH is set))],
      [AC_MSG_RESULT(no)
       LDADD_FOR_TESTS_KLUDGE=""
      ])
      LDFLAGS="$saved_LDFLAGS"
    fi
    ;;
esac
AC_SUBST([LDADD_FOR_TESTS_KLUDGE])

# Set some variables depending on the platform for later use.
have_w32_system=no
have_w64_system=no
case "${host}" in
    x86_64-*mingw32*)
        have_w32_system=yes
        have_w64_system=yes
        ;;
    *-mingw32*)
        have_w32_system=yes
        ;;
    *-apple-darwin*)
        # This is the equivalent of the _GNU_SOURCE feature-test-macro
        # on GNU libc systems.
        AC_DEFINE(_DARWIN_C_SOURCE, 1,
                  Expose all libc features (__DARWIN_C_FULL).)
        ;;
    *)
       ;;
esac


if test "$have_w32_system" != yes; then
  gl_THREADLIB_EARLY
fi

# We build libgpg-error with large file support so that we have a 64
# bit off_t.  Our external interface uses the gpgrt_off_t which is
# anyway specified as 64 bit.  Thus the same libgpg-error can be used
# by software which is not build with large file support.
AC_SYS_LARGEFILE


LT_PREREQ([2.2.6])
LT_INIT([win32-dll disable-static])
LT_LANG([Windows Resource])


# We need to compile and run a program on the build machine.
AX_CC_FOR_BUILD

AH_BOTTOM([
/* Force using of NLS for W32 even if no libintl has been found.  This is
   okay because we have our own gettext implementation for W32.  */
#if defined(HAVE_W32_SYSTEM) && !defined(ENABLE_NLS)
#define ENABLE_NLS 1
#endif

/* Connect the generic estream-printf.c to our framework.  */
#define _ESTREAM_PRINTF_REALLOC _gpgrt_realloc
#define _ESTREAM_PRINTF_EXTRA_INCLUDE "gpgrt-int.h"

/* For building we need to define these macro.  */
#define GPG_ERR_ENABLE_GETTEXT_MACROS 1
#define GPG_ERR_ENABLE_ERRNO_MACROS 1
#define GPGRT_ENABLE_ES_MACROS 1
#define GPGRT_ENABLE_LOG_MACROS 1
#define GPGRT_ENABLE_ARGPARSE_MACROS 1
])

# We used to provide gpg-error-config command always.  Now, it's
# gpgrt-config command with gpg-error.pc configuration file, which
# does same thing.  gpg-error-config command is deprecated.
AC_ARG_ENABLE(install-gpg-error-config,
  AS_HELP_STRING([--enable-install-gpg-error-config],
                 [install gpg-error-config command]),
  install_gpg_error_config=$enableval)
AM_CONDITIONAL(INSTALL_GPG_ERROR_CONFIG,
               test "$install_gpg_error_config" = "yes")

# Note, that autogen.sh greps for the next line.
AM_GNU_GETTEXT_VERSION([0.19.3])
AM_GNU_GETTEXT([external])

# Checks for header files.
AC_CHECK_HEADERS([locale.h stdint.h sys/select.h sys/time.h \
                  signal.h poll.h pwd.h])

AC_FUNC_STRERROR_R
case "${host_os}" in
     mingw32*)
     INSTALLSHELLPATH=/bin/sh
     AC_CHECK_FUNC([strerror_s])
     ;;
     solaris*)
     INSTALLSHELLPATH=/usr/xpg4/bin/sh
     # All versions of Solaris from 2.4 have a thread-safe strerror().
     # Since Solaris 10, in addition strerror_r() exists.
     ;;
     *)
     INSTALLSHELLPATH=/bin/sh
     AC_CHECK_FUNC([strerror_r], [],
AC_MSG_WARN([[Without strerror_r, gpg_strerror_r might not be thread-safe]]))
     ;;
esac
AC_SUBST(INSTALLSHELLPATH)

# We don't check 'fork' by configure script.  Use of fork is inferred from by
# the target system (POSIX or Windows).
# AC_FUNC_FORK

AC_CHECK_FUNCS([flockfile vasprintf mmap rand strlwr stpcpy setenv stat \
                getrlimit getpwnam getpwuid getpwnam_r getpwuid_r inet_pton])


#
# Checks for typedefs, structures, and compiler characteristics.
#
AC_C_CONST

AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

GNUPG_FUNC_MKDIR_TAKES_ONE_ARG


# Find a 64 bit integer type to be used instead of off_t.  We prefer
# the standard integer types over int64_t and finally try long long.
if test "$ac_cv_sizeof_int" = "8"; then
   replacement_for_off_t="int"
elif test "$ac_cv_sizeof_long" = "8"; then
   replacement_for_off_t="long"
elif test "$ac_cv_header_stdint_h" = yes; then
   replacement_for_off_t="int64_t"
elif test "$ac_cv_sizeof_long_long" = "8"; then
   replacement_for_off_t="long long"
else
   AC_MSG_ERROR([[
***
*** No 64 bit signed integer type found.  Can't build this library.
***]])
fi
AC_DEFINE_UNQUOTED(REPLACEMENT_FOR_OFF_T, "$replacement_for_off_t",
                   [Used by mkheader to insert the replacement type.])


#
# Setup gcc specific options
#
AC_MSG_NOTICE([checking for cc features])
if test "$GCC" = yes; then
    # Check whether gcc does not emit a diagnostic for unknown -Wno-*
    # options.  This is the case for gcc >= 4.6
    AC_MSG_CHECKING([if gcc ignores unknown -Wno-* options])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6 )
#kickerror
#endif]],[])],[_gcc_silent_wno=yes],[_gcc_silent_wno=no])
    AC_MSG_RESULT($_gcc_silent_wno)

    # Note that it is okay to use CFLAGS here because these are just
    # warning options and the user should have a chance of overriding
    # them.
    if test "$USE_MAINTAINER_MODE" = "yes"; then
        M_CFLAGS="-O3 -Wall -Wcast-align -Wshadow -Wstrict-prototypes"
        M_CFLAGS="$M_CFLAGS -Wformat -Wno-format-y2k -Wformat-security"
        if test x"$_gcc_silent_wno" = xyes ; then
          _gcc_wopt=yes
        else
          AC_MSG_CHECKING([if gcc supports -Wno-missing-field-initializers])
          _gcc_cflags_save=$CFLAGS
          CFLAGS="-Wno-missing-field-initializers"
          AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
                            [_gcc_wopt=yes],[_gcc_wopt=no])
          AC_MSG_RESULT($_gcc_wopt)
          CFLAGS=$_gcc_cflags_save;
        fi
        if test x"$_gcc_wopt" = xyes ; then
          M_CFLAGS="$M_CFLAGS -W -Wno-sign-compare -Wno-missing-field-initializers"
        fi

        AC_MSG_CHECKING([if gcc supports -Wdeclaration-after-statement])
        _gcc_cflags_save=$CFLAGS
        CFLAGS="-Wdeclaration-after-statement"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],_gcc_wopt=yes,_gcc_wopt=no)
        AC_MSG_RESULT($_gcc_wopt)
        CFLAGS=$_gcc_cflags_save;
        if test x"$_gcc_wopt" = xyes ; then
          M_CFLAGS="$M_CFLAGS -Wdeclaration-after-statement"
        fi

	# Prepend the maintainer-cflags so that the user can override
	# them, e.g. to override the optimization flags for debugging.
	CFLAGS="$M_CFLAGS $CFLAGS"
    else
        CFLAGS="$CFLAGS -Wall"
    fi

    AC_MSG_CHECKING([if gcc supports -Wpointer-arith])
    _gcc_cflags_save=$CFLAGS
    CFLAGS="-Wpointer-arith"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],_gcc_psign=yes,_gcc_psign=no)
    AC_MSG_RESULT($_gcc_psign)
    CFLAGS=$_gcc_cflags_save;
    if test x"$_gcc_psign" = xyes ; then
       CFLAGS="$CFLAGS -Wpointer-arith"
    fi

    # The undocumented option -Wno-psabi suppresses the annoying
    #   "the ABI of passing union with long double has changed in GCC 4.4"
    # which is emitted in estream-printf.c but entirely irrelvant
    # because that union is local to the file.
    if test x"$_gcc_silent_wno" = xyes ; then
       CFLAGS="$CFLAGS -Wno-psabi"
    fi
fi

#
# Check whether the compiler supports the GCC style aligned attribute
#
AC_CACHE_CHECK([whether the GCC style aligned attribute is supported],
       [gcry_cv_gcc_attribute_aligned],
       [gcry_cv_gcc_attribute_aligned=no
        AC_COMPILE_IFELSE([AC_LANG_SOURCE(
          [[struct { int a; } foo __attribute__ ((aligned (16)));]])],
          [gcry_cv_gcc_attribute_aligned=yes])])
if test "$gcry_cv_gcc_attribute_aligned" = "yes" ; then
   AC_DEFINE(HAVE_GCC_ATTRIBUTE_ALIGNED,1,
     [Defined if a GCC style "__attribute__ ((aligned (n))" is supported])
fi

#
# Check for ELF visibility support.
#
AC_CACHE_CHECK(whether the visibility attribute is supported,
       gcry_cv_visibility_attribute,
       [gcry_cv_visibility_attribute=no
        AC_LANG_CONFTEST([AC_LANG_SOURCE(
          [[int foo __attribute__ ((visibility ("hidden"))) = 1;
            int bar __attribute__ ((visibility ("protected"))) = 1;
          ]])])

        if ${CC-cc} -Werror -S conftest.c -o conftest.s \
                  1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD ; then
            if grep '\.hidden.*foo' conftest.s >/dev/null 2>&1 ; then
                if grep '\.protected.*bar' conftest.s >/dev/null 2>&1; then
                    gcry_cv_visibility_attribute=yes
                fi
            fi
        fi
       ])
if test "$gcry_cv_visibility_attribute" = "yes"; then
    AC_CACHE_CHECK(for broken visibility attribute,
       gcry_cv_broken_visibility_attribute,
       [gcry_cv_broken_visibility_attribute=yes
        AC_LANG_CONFTEST([AC_LANG_SOURCE(
          [[int foo (int x);
            int bar (int x) __asm__ ("foo")
                            __attribute__ ((visibility ("hidden")));
            int bar (int x) { return x; }
          ]])])

        if ${CC-cc} -Werror -S conftest.c -o conftest.s \
                  1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD ; then
           if grep '\.hidden@<:@ 	_@:>@foo' conftest.s >/dev/null 2>&1;
            then
               gcry_cv_broken_visibility_attribute=no
           fi
        fi
       ])
fi
if test "$gcry_cv_visibility_attribute" = "yes"; then
    AC_CACHE_CHECK(for broken alias attribute,
       gcry_cv_broken_alias_attribute,
       [gcry_cv_broken_alias_attribute=yes
        AC_LANG_CONFTEST([AC_LANG_SOURCE(
          [[extern int foo (int x) __asm ("xyzzy");
            int bar (int x) { return x; }
            extern __typeof (bar) foo __attribute ((weak, alias ("bar")));
            extern int dfoo;
            extern __typeof (dfoo) dfoo __asm ("abccb");
            int dfoo = 1;
          ]])])

        if ${CC-cc} -Werror -S conftest.c -o conftest.s \
                  1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD ; then
           if grep 'xyzzy' conftest.s >/dev/null 2>&1 && \
              grep 'abccb' conftest.s >/dev/null 2>&1; then
              gcry_cv_broken_alias_attribute=no
           fi
        fi
        ])
fi
if test "$gcry_cv_visibility_attribute" = "yes"; then
    AC_CACHE_CHECK(if gcc supports -fvisibility=hidden,
       gcry_cv_gcc_has_f_visibility,
       [gcry_cv_gcc_has_f_visibility=no
        _gcc_cflags_save=$CFLAGS
        CFLAGS="-fvisibility=hidden"
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
                          gcry_cv_gcc_has_f_visibility=yes)
        CFLAGS=$_gcc_cflags_save;
       ])
fi
if test "$gcry_cv_visibility_attribute" = "yes" \
   && test "$gcry_cv_broken_visibility_attribute" != "yes" \
   && test "$gcry_cv_broken_alias_attribute" != "yes" \
   && test "$gcry_cv_gcc_has_f_visibility" = "yes"
 then
   AC_DEFINE(GPGRT_USE_VISIBILITY, 1,
             [Define to use the GNU C visibility attribute.])
   CFLAGS="$CFLAGS -fvisibility=hidden"
fi

#
# Check whether ld supports a version script.
# (Actually not a check but a list of systems which are known to support it.)
#
have_ld_version_script=no
case "${host}" in
    *-*-linux*)
	have_ld_version_script=yes
        ;;
    *-*-gnu*)
	have_ld_version_script=yes
        ;;
esac
AC_ARG_ENABLE([ld-version-script],
              AS_HELP_STRING([--enable-ld-version-script],
                             [enable/disable use of linker version script.
                              (default is system dependent)]),
              [have_ld_version_script=$enableval],
              [ : ] )
AM_CONDITIONAL(HAVE_LD_VERSION_SCRIPT, test "$have_ld_version_script" = "yes")


#
# Check for thread library.
#
# Windows has always thread support; thus we don't bother to test for
# it as it may lead to false results when cross building.
if test "$have_w32_system" = yes; then
  GPG_ERROR_CONFIG_LIBS_PRIVATE=-lws2_32
  AC_DEFINE([USE_WINDOWS_THREADS], [1])
  LIBTHREAD=
  LTLIBTHREAD=
  LIBMULTITHREAD=
  LTLIBMULTITHREAD=
  THREADLIB_CPPFLAGS=""
  AC_SUBST([LIBTHREAD])
  AC_SUBST([LTLIBTHREAD])
  AC_SUBST([LIBMULTITHREAD])
  AC_SUBST([LTLIBMULTITHREAD])
else
  GPG_ERROR_CONFIG_LIBS_PRIVATE=""
  gl_LOCK
  if test "$gl_threads_api" = posix; then
    AC_CHECK_SIZEOF(pthread_mutex_t,,[AC_INCLUDES_DEFAULT
                                      #include <pthread.h>])
    AC_CHECK_HEADERS([sys/single_threaded.h])
  fi
fi

# Default value for GPG_ERROR_CONFIG_LIBS
config_libs="-lgpg-error"

#
# Check for other libraries
#
LIB_SCHED_YIELD=
AC_SUBST([LIB_SCHED_YIELD])
AC_SEARCH_LIBS([sched_yield], [rt posix4],
               [if test "$ac_cv_search_sched_yield" != "none required"; then
                  LIB_SCHED_YIELD=$ac_cv_search_sched_yield
                  config_libs="$config_libs $LIB_SCHED_YIELD"
                fi])

LIB_NETWORK=
AC_SUBST([LIB_NETWORK])
AC_SEARCH_LIBS([inet_addr], [nsl],
               [if test "$ac_cv_search_inet_addr" != "none required"; then
                  LIB_NETWORK=$ac_cv_search_inet_addr
                fi])
AC_SEARCH_LIBS([socket], [socket],
               [if test "$ac_cv_search_socket" != "none required"; then
                  LIB_NETWORK="$ac_cv_search_socket $LIB_NETWORK"
                fi], [], [$LIB_NETWORK])
if test "x$LIB_NETWORK" != x; then
  config_libs="$config_libs $LIB_NETWORK"
fi

# Check for optional readline support
GNUPG_CHECK_READLINE


#
# Prepare building of estream
#
estream_INIT

#
# Substitution used for gpg-error-config and gpg-error.pc
#
GPG_ERROR_CONFIG_LIBS="$config_libs"
if test "x$LIBTHREAD" != x; then
  GPG_ERROR_CONFIG_LIBS="${GPG_ERROR_CONFIG_LIBS} ${LIBTHREAD}"
fi
if test "x$LIBMULTITHREAD" != x; then
  GPG_ERROR_CONFIG_MT_LIBS="${LIBMULTITHREAD}"
else
  GPG_ERROR_CONFIG_MT_LIBS=""
fi
GPG_ERROR_CONFIG_CFLAGS=""
if test "x$THREADLIB_CPPFLAGS" != x; then
  GPG_ERROR_CONFIG_MT_CFLAGS="${THREADLIB_CPPFLAGS}"
else
  GPG_ERROR_CONFIG_MT_CFLAGS=""
fi
GPG_ERROR_CONFIG_HOST="$host"

case "$includedir" in
  /usr/include|/include) ;;
  '${prefix}/include')
    if test "$prefix" != / -a "$prefix" != /usr; then
      if test -z "$GPG_ERROR_CONFIG_CFLAGS"; then
        GPG_ERROR_CONFIG_CFLAGS="-I\${includedir}"
      else
        GPG_ERROR_CONFIG_CFLAGS="-I\${includedir} $GPG_ERROR_CONFIG_CFLAGS"
      fi
    fi
    ;;
  *)
    if test -z "$GPG_ERROR_CONFIG_CFLAGS"; then
      GPG_ERROR_CONFIG_CFLAGS="-I\${includedir}"
    else
      GPG_ERROR_CONFIG_CFLAGS="-I\${includedir} $GPG_ERROR_CONFIG_CFLAGS"
    fi
    ;;
esac
case "$libdir" in
  /usr/lib|/usr/lib64|/lib|/lib64) ;;
  '${exec_prefix}/lib'|'${exec_prefix}/lib64')
    if test "$exec_prefix" = "NONE"; then
      if test "$prefix" != / -a "$prefix" != /usr; then
        GPG_ERROR_CONFIG_LIBS="-L\${libdir} $GPG_ERROR_CONFIG_LIBS"
      fi
    elif test "$exec_prefix" != / -a "$exec_prefix" != /usr; then
      GPG_ERROR_CONFIG_LIBS="-L\${libdir} $GPG_ERROR_CONFIG_LIBS"
    fi
    ;;
  *) GPG_ERROR_CONFIG_LIBS="-L\${libdir} $GPG_ERROR_CONFIG_LIBS" ;;
esac

AC_SUBST(GPG_ERROR_CONFIG_LIBS)
AC_SUBST(GPG_ERROR_CONFIG_CFLAGS)
AC_SUBST(GPG_ERROR_CONFIG_MT_LIBS)
AC_SUBST(GPG_ERROR_CONFIG_MT_CFLAGS)
AC_SUBST(GPG_ERROR_CONFIG_LIBS_PRIVATE)
AC_SUBST(GPG_ERROR_CONFIG_HOST)


#
# Special defines for certain platforms
#
force_use_syscfg=no
if test "$have_w32_system" = yes; then
    AC_DEFINE(HAVE_W32_SYSTEM,1,[Defined if we run on a W32 API based system])
    if test "$have_w64_system" = yes; then
      AC_DEFINE(HAVE_W64_SYSTEM,1,[Defined if we run on 64 bit W32 API system])
    fi
    force_use_syscfg=yes
fi
if test x"$gl_use_threads" = xno; then
  lock_obj_h_generated=yes
  if test ! -d src; then mkdir src; fi
  host=$host $srcdir/src/gen-lock-obj.sh --disable-threads \
    >src/lock-obj-pub.native.h
  AC_MSG_NOTICE([generated src/lock-obj-pub.native.h for $host])
elif test x$cross_compiling = xyes; then
  case $host in
    *-*-linux-gnu* | *-*-linux-musl*)
    AC_CHECK_TOOL(OBJDUMP, [objdump])
    if test -n "$OBJDUMP"; then
      lock_obj_h_generated=yes
      if test ! -d src; then mkdir src; fi
      LOCK_ABI_VERSION=1 host=$host host_alias=$host_alias \
          CC=$CC OBJDUMP=$OBJDUMP \
          ac_ext=$ac_ext ac_objext=$ac_objext \
          AWK=$AWK $srcdir/src/gen-lock-obj.sh \
          >src/lock-obj-pub.native.h
      AC_MSG_NOTICE([generated src/lock-obj-pub.native.h using $host_alias-objdump and $AWK])
    else
      force_use_syscfg=yes
    fi
    ;;
    *)
    force_use_syscfg=yes
    ;;
  esac
fi

AM_CONDITIONAL(HAVE_W32_SYSTEM, test "$have_w32_system" = yes)
AM_CONDITIONAL(HAVE_W64_SYSTEM, test "$have_w64_system" = yes)
AM_CONDITIONAL(CROSS_COMPILING, test x$cross_compiling = xyes)
AM_CONDITIONAL(FORCE_USE_SYSCFG, test x$force_use_syscfg = xyes)
AM_CONDITIONAL(HAVE_GENERATED_LOCK_OBJ_H, test x$lock_obj_h_generated = xyes)

have_lock_optimization=no
if test "$gl_threads_api" = posix; then
  if test x$ac_cv_header_sys_single_threaded_h = xyes; then
    have_lock_optimization=yes
  else
    case "${host_os}" in
      *-gnu*) have_lock_optimization=yes ;;
      * ) ;;
    esac
  fi
fi
AM_CONDITIONAL(HAVE_LOCK_OPTIMIZATION, test "$have_lock_optimization" = yes)

AC_DEFINE_UNQUOTED(HOST_TRIPLET_STRING, "$host", [The host triplet])


#
# gpgrt_log_clock may require linking with extra libaries.  As long as
# we don't have a good test for this we require the use of this
# configure option to enabling printing of a timestamp.
#
AC_MSG_CHECKING([whether to enable log_clock])
AC_ARG_ENABLE(log_clock,
              AS_HELP_STRING([--enable-log-clock],
                             [enable log_clock timestamps]),
              enable_log_clock=$enableval, enable_log_clock=no)
AC_MSG_RESULT($enable_log_clock)
if test "$enable_log_clock" = yes ; then
  AC_DEFINE(ENABLE_LOG_CLOCK,1,[Defined to use log_clock timestamps])
fi

#
# For now we do not build gpgscm by default.
# Eventually we will reverse the meaning of that option.
#
build_gpgscm=no
#AC_MSG_CHECKING([whether to build gpgscm])
#AC_ARG_ENABLE(gpgscm,
#              AS_HELP_STRING([--enable-gpgscm],
#                             [build the gpgscm tool]),
#              build_gpgscm=$enableval, build_gpgscm=no)
#AC_MSG_RESULT($build_gpgscm)
AM_CONDITIONAL([BUILD_GPGSCM], [test "x$build_gpgscm" != xno])


#
# Add -Werror to CFLAGS.  This hack can be used to avoid problems with
# misbehaving autoconf tests in case the user supplied -Werror.
#
AC_ARG_ENABLE(werror,
   AS_HELP_STRING([--enable-werror],
                  [append -Werror to CFLAGS]),
                  [if test $enableval = yes ; then
                      CFLAGS="$CFLAGS -Werror"
                   fi])


#
# Provide information about the build.
#
BUILD_REVISION="mym4_revision"
AC_SUBST(BUILD_REVISION)
AC_DEFINE_UNQUOTED(BUILD_REVISION, "$BUILD_REVISION",
                   [GIT commit id revision used to build this package])

changequote(,)dnl
BUILD_VERSION=`echo "$PACKAGE_VERSION"|sed 's/\([0-9.]*\).*/\1./'`
changequote([,])dnl
BUILD_VERSION="${BUILD_VERSION}0.mym4_revision_dec"
BUILD_FILEVERSION=`echo "${BUILD_VERSION}" | tr . ,`
AC_SUBST(BUILD_VERSION)
AC_SUBST(BUILD_FILEVERSION)

AC_ARG_ENABLE([build-timestamp],
  AS_HELP_STRING([--enable-build-timestamp],
                 [set an explicit build timestamp for reproducibility.
                  (default is the current time in ISO-8601 format)]),
     [if test "$enableval" = "yes"; then
        BUILD_TIMESTAMP=`date -u +%Y-%m-%dT%H:%M+0000 2>/dev/null || date`
      else
        BUILD_TIMESTAMP="$enableval"
      fi],
     [BUILD_TIMESTAMP="<none>"])
AC_SUBST(BUILD_TIMESTAMP)
AC_DEFINE_UNQUOTED(BUILD_TIMESTAMP, "$BUILD_TIMESTAMP",
                   [The time this package was configured for a build])



AC_ARG_ENABLE(languages, AS_HELP_STRING([--disable-languages],
                        [do not build support for other languages than C]))
AM_CONDITIONAL([LANGUAGES_SOME], [test "x$enable_languages" != xno])

build_doc=yes
AC_ARG_ENABLE([doc], AS_HELP_STRING([--disable-doc],
                                    [do not build the documentation]),
                     build_doc=$enableval, build_doc=yes)
AM_CONDITIONAL([BUILD_DOC], [test "x$build_doc" != xno])

build_tests=yes
AC_ARG_ENABLE([tests], AS_HELP_STRING([--disable-tests],
                                    [do not build the tests]),
                     build_tests=$enableval, build_tests=yes)
AM_CONDITIONAL([BUILD_TESTS], [test "x$build_tests" != xno])

#
# Substitution
#
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([doc/Makefile po/Makefile.in m4/Makefile])
AC_CONFIG_FILES([src/Makefile tests/Makefile])
AC_CONFIG_FILES([lang/Makefile lang/cl/Makefile lang/cl/gpg-error.asd])
AC_CONFIG_FILES([src/versioninfo.rc src/gpg-error.w32-manifest])
AC_CONFIG_FILES([src/gpg-error.pc])
AC_CONFIG_FILES([src/gpg-error-config-old:src/gpg-error-config.in], [chmod +x src/gpg-error-config-old])
AC_CONFIG_FILES([src/gpgrt-config], [chmod +x src/gpgrt-config])
AC_CONFIG_FILES([src/gpg-error-config-test.sh], [chmod +x src/gpg-error-config-test.sh])

AC_OUTPUT

tmp=
if test "$have_w32_system" != yes; then
   if test x"$gl_use_threads" = xno; then
     tmp=" NO-THREADS"
   fi
fi
echo "
        $PACKAGE_NAME v$PACKAGE_VERSION has been configured as follows:

        Revision: mym4_revision  (mym4_revision_dec)
        Platform: $host$tmp
"
if test "$gcry_cv_gcc_attribute_aligned" != "yes" ; then
cat <<G10EOF
***
***  Please note that your compiler does not support the GCC style
***  aligned attribute.  Using this software may evoke bus errors.
***
G10EOF
fi
