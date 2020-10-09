# from http://autoconf-archive.cryp.to/ax_tls.html
#
# This was licensed under the GPL with the following exception:
#
# As a special exception, the respective Autoconf Macro's copyright
# owner gives unlimited permission to copy, distribute and modify the
# configure scripts that are the output of Autoconf when processing
# the Macro. You need not follow the terms of the GNU General Public
# License when using or distributing such scripts, even though
# portions of the text of the Macro appear in them. The GNU General
# Public License (GPL) does govern all other use of the material that
# constitutes the Autoconf Macro.
#
# This special exception to the GPL applies to versions of the
# Autoconf Macro released by the Autoconf Macro Archive. When you make
# and distribute a modified version of the Autoconf Macro, you may
# extend this special exception to the GPL to apply to your modified
# version as well.
#
AC_DEFUN([AX_TLS], [
  AC_MSG_CHECKING(for thread local storage (TLS) class)
  AC_CACHE_VAL(ac_cv_tls, [
    ax_tls_keywords="__thread __declspec(thread) none"
    for ax_tls_keyword in $ax_tls_keywords; do
       case $ax_tls_keyword in
          none) ac_cv_tls=none ; break ;;
          *)
             AC_TRY_COMPILE(
                [#include <stdlib.h>
                 static void
                 foo(void) {
                 static ] $ax_tls_keyword [ int bar;
                 exit(1);
                 }],
                 [],
                 [ac_cv_tls=$ax_tls_keyword ; break],
                 ac_cv_tls=none
             )
          esac
    done
])

  if test "$ac_cv_tls" != "none"; then
    dnl AC_DEFINE([TLS], [], [If the compiler supports a TLS storage class define it to that here])
    AC_DEFINE_UNQUOTED([TLS], $ac_cv_tls, [If the compiler supports a TLS storage class define it to that here])
  fi
  AC_MSG_RESULT($ac_cv_tls)
])

# ===========================================================================
#         http://www.nongnu.org/autoconf-archive/check_gnu_make.html
# ===========================================================================
#
# SYNOPSIS
#
#   CHECK_GNU_MAKE()
#
# DESCRIPTION
#
#   This macro searches for a GNU version of make. If a match is found, the
#   makefile variable `ifGNUmake' is set to the empty string, otherwise it
#   is set to "#". This is useful for including a special features in a
#   Makefile, which cannot be handled by other versions of make. The
#   variable _cv_gnu_make_command is set to the command to invoke GNU make
#   if it exists, the empty string otherwise.
#
#   Here is an example of its use:
#
#   Makefile.in might contain:
#
#       # A failsafe way of putting a dependency rule into a makefile
#       $(DEPEND):
#               $(CC) -MM $(srcdir)/*.c > $(DEPEND)
#
#       @ifGNUmake@ ifeq ($(DEPEND),$(wildcard $(DEPEND)))
#       @ifGNUmake@ include $(DEPEND)
#       @ifGNUmake@ endif
#
#   Then configure.in would normally contain:
#
#       CHECK_GNU_MAKE()
#       AC_OUTPUT(Makefile)
#
#   Then perhaps to cause gnu make to override any other make, we could do
#   something like this (note that GNU make always looks for GNUmakefile
#   first):
#
#       if  ! test x$_cv_gnu_make_command = x ; then
#               mv Makefile GNUmakefile
#               echo .DEFAULT: > Makefile ;
#               echo \  $_cv_gnu_make_command \$@ >> Makefile;
#       fi
#
#   Then, if any (well almost any) other make is called, and GNU make also
#   exists, then the other make wraps the GNU make.
#
# LICENSE
#
#   Copyright (c) 2008 John Darrington <j.darrington@elvis.murdoch.edu.au>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.
#
# Note: Modified by Ted Ts'o to add @ifNotGNUMake@

AC_DEFUN(
        [CHECK_GNU_MAKE], [ AC_CACHE_CHECK( for GNU make,_cv_gnu_make_command,
                _cv_gnu_make_command='' ;
dnl Search all the common names for GNU make
                if test -n "$FORCE_NATIVE_MAKE" ; then
                   MAKES="make"
                else
                   MAKES="make gmake gnumake"
                fi
                for a in "$MAKE" $MAKES ; do
                        if test -z "$a" ; then continue ; fi ;
                        if  ( sh -c "$a --version" 2> /dev/null | grep GNU  2>&1 > /dev/null ) ;  then
                                _cv_gnu_make_command=$a ;
                                break;
                        fi
                done ;
        ) ;
dnl If there was a GNU version, then set @ifGNUmake@ to the empty string, '#' otherwise
        if test  "x$_cv_gnu_make_command" != "x"  ; then
                ifGNUmake='' ;
                ifNotGNUmake='#' ;
        else
                ifGNUmake='#' ;
                ifNotGNUmake='' ;
                AC_MSG_RESULT("Not found");
        fi
        AC_SUBST(ifGNUmake)
        AC_SUBST(ifNotGNUmake)
] )

# AX_CHECK_MOUNT_OPT: an autoconf macro to check for generic filesystem-
# agnostic 'mount' options. Written by Nicholas Clark. Looks for constants in
# sys/mount.h to predict whether the 'mount' utility will support a specific
# mounting option.
#
# This macro can be used to check for the presence of 'nodev' (or other mount
# options), which isn't uniformly implemented in the BSD family at the time of
# this writing. Tested on FreeBSD, NetBSD, OpenBSD, and Linux.
#
# Usage:
#
# AX_CHECK_MOUNT_OPT(option)
#
# Defines HAVE_MOUNT_$OPTION (in uppercase) if the option exists, and sets
# ac_cv_mount_$option (in original case) otherwise.
#
# Copyright (c) 2018 Nicholas Clark <nicholas.clark@gmail.com>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty or attribution requirement.

AC_DEFUN([AX_CHECK_MOUNT_OPT], [__AX_CHECK_MOUNT_OPT(m4_tolower([$1]),m4_toupper([$1]))])
AC_DEFUN([__AX_CHECK_MOUNT_OPT],
[
    AS_ECHO_N("checking for mount '$1' option... ")
    AC_TRY_COMPILE(
        [#include <sys/mount.h>],
        [void *temp = (void *)(MS_$2); (void) temp;],
        [AC_DEFINE(HAVE_MOUNT_$2, 1, [Define to 1 if mount supports $1.])
         AS_VAR_SET(ac_cv_mount_$1, yes)
         AS_ECHO("yes")],
        [AC_TRY_COMPILE(
            [#include <sys/mount.h>],
            [void *temp = (void *)(MNT_$2); (void) temp;],
            [AC_DEFINE(HAVE_MOUNT_$2, 1, [Define to 1 if mount supports $1.])
             AS_VAR_SET(ac_cv_mount_$1, yes)
             AS_ECHO("yes")],
            [AS_VAR_SET(ac_cv_mount_$1, no)
             AS_ECHO("no")]
        )]
    )
])
