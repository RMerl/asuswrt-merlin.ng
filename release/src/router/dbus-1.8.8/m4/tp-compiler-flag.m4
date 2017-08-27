dnl A version of AS_COMPILER_FLAG that supports both C and C++.
dnl Based on:

dnl as-compiler-flag.m4 0.1.0
dnl autostars m4 macro for detection of compiler flags
dnl David Schleef <ds@schleef.org>
dnl $Id: as-compiler-flag.m4,v 1.1 2005/06/18 18:02:46 burgerman Exp $

dnl TP_COMPILER_FLAG(CFLAGS, ACTION-IF-ACCEPTED, [ACTION-IF-NOT-ACCEPTED])
dnl Tries to compile with the given CFLAGS and CXXFLAGS.
dnl
dnl Runs ACTION-IF-ACCEPTED if the compiler for the currently selected
dnl AC_LANG can compile with the flags, and ACTION-IF-NOT-ACCEPTED otherwise.

AC_DEFUN([TP_COMPILER_FLAG],
[
  AC_MSG_CHECKING([to see if compiler understands $1])

  save_CFLAGS="$CFLAGS"
  save_CXXFLAGS="$CXXFLAGS"
  CFLAGS="$CFLAGS $1"
  CXXFLAGS="$CXXFLAGS $1"

  AC_TRY_COMPILE([ ], [], [flag_ok=yes], [flag_ok=no])
  CFLAGS="$save_CFLAGS"
  CXXFLAGS="$save_CXXFLAGS"

  if test "X$flag_ok" = Xyes ; then
    $2
    true
  else
    $3
    true
  fi
  AC_MSG_RESULT([$flag_ok])
])

dnl TP_ADD_COMPILER_FLAG(VARIABLE, CFLAGS)
dnl Append CFLAGS to VARIABLE if the compiler supports them.
AC_DEFUN([TP_ADD_COMPILER_FLAG],
[
  TP_COMPILER_FLAG([$2], [$1="[$]$1 $2"])
])
