# year2038.m4 serial 7
dnl Copyright (C) 2017-2022 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Attempt to ensure that 'time_t' can go past the year 2038 and that
dnl the functions 'time', 'stat', etc. work with post-2038 timestamps.

AC_DEFUN([gl_YEAR2038_EARLY],
[
  AC_REQUIRE([AC_CANONICAL_HOST])
  case "$host_os" in
    mingw*)
      AC_DEFINE([__MINGW_USE_VC2005_COMPAT], [1],
        [For 64-bit time_t on 32-bit mingw.])
      ;;
  esac
])

# gl_YEAR2038_TEST_INCLUDES
# -------------------------
AC_DEFUN([gl_YEAR2038_TEST_INCLUDES],
[[
  #include <time.h>
  /* Check that time_t can represent 2**32 - 1 correctly.  */
  #define LARGE_TIME_T \\
    ((time_t) (((time_t) 1 << 30) - 1 + 3 * ((time_t) 1 << 30)))
  int verify_time_t_range[(LARGE_TIME_T / 65537 == 65535
                           && LARGE_TIME_T % 65537 == 0)
                          ? 1 : -1];
]])

# gl_YEAR2038_BODY(REQUIRE-YEAR2038-SAFE)
-----------------------------------------
AC_DEFUN([gl_YEAR2038_BODY],
[
 AC_ARG_ENABLE([year2038],
   [  --disable-year2038      omit support for timestamps past the year 2038])
 AS_IF([test "$enable_year2038" != no],
 [
  dnl On many systems, time_t is already a 64-bit type.
  dnl On those systems where time_t is still 32-bit, it requires kernel
  dnl and libc support to make it 64-bit. For glibc 2.34 and later on Linux,
  dnl defining _TIME_BITS=64 and _FILE_OFFSET_BITS=64 is needed on x86 and ARM.
  dnl
  dnl On native Windows, the system include files define types __time32_t
  dnl and __time64_t. By default, time_t is an alias of
  dnl   - __time32_t on 32-bit mingw,
  dnl   - __time64_t on 64-bit mingw and on MSVC (since MSVC 8).
  dnl But when compiling with -D__MINGW_USE_VC2005_COMPAT, time_t is an
  dnl alias of __time64_t.
  dnl And when compiling with -D_USE_32BIT_TIME_T, time_t is an alias of
  dnl __time32_t.
  AC_CACHE_CHECK([for time_t past the year 2038], [gl_cv_type_time_t_y2038],
    [AC_COMPILE_IFELSE(
       [AC_LANG_SOURCE([gl_YEAR2038_TEST_INCLUDES])],
       [gl_cv_type_time_t_y2038=yes], [gl_cv_type_time_t_y2038=no])
    ])
  if test "$gl_cv_type_time_t_y2038" = no; then
    AC_CACHE_CHECK([for 64-bit time_t with _TIME_BITS=64],
      [gl_cv_type_time_t_bits_macro],
      [AC_COMPILE_IFELSE(
         [AC_LANG_SOURCE([[#define _TIME_BITS 64
                           #define _FILE_OFFSET_BITS 64
                           ]gl_YEAR2038_TEST_INCLUDES])],
         [gl_cv_type_time_t_bits_macro=yes],
         [gl_cv_type_time_t_bits_macro=no])
      ])
    if test "$gl_cv_type_time_t_bits_macro" = yes; then
      AC_DEFINE([_TIME_BITS], [64],
        [Number of bits in a timestamp, on hosts where this is settable.])
      dnl AC_SYS_LARGFILE also defines this; it's OK if we do too.
      AC_DEFINE([_FILE_OFFSET_BITS], [64],
        [Number of bits in a file offset, on hosts where this is settable.])
      gl_cv_type_time_t_y2038=yes
    fi
  fi
  if test $gl_cv_type_time_t_y2038 = no; then
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE(
         [[#ifdef _USE_32BIT_TIME_T
             int ok;
           #else
             error fail
           #endif
         ]])],
      [AC_MSG_FAILURE(
         [The 'time_t' type stops working after January 2038.
          Remove _USE_32BIT_TIME_T from the compiler flags.])],
      [# If not cross-compiling and $1 says we should check,
       # and 'touch' works with a large timestamp, then evidently wider time_t
       # is desired and supported, so fail and ask the builder to fix the
       # problem.  Otherwise, just warn the builder.
       m4_ifval([$1],
         [if test $cross_compiling = no \
             && TZ=UTC0 touch -t 210602070628.15 conftest.time 2>/dev/null; then
            case `TZ=UTC0 LC_ALL=C ls -l conftest.time 2>/dev/null` in
              *'Feb  7  2106'* | *'Feb  7 17:10'*)
                AC_MSG_FAILURE(
                  [The 'time_t' type stops working after January 2038,
                   and your system appears to support a wider 'time_t'.
                   Try configuring with 'CC="${CC} -m64"'.
                   To build with a 32-bit time_t anyway (not recommended),
                   configure with '--disable-year2038'.]);;
            esac
            rm -f conftest.time
          fi])
       if test "$gl_warned_about_y2038" != yes; then
         AC_MSG_WARN(
           [The 'time_t' type stops working after January 2038,
            and this package needs a wider 'time_t' type
            if there is any way to access timestamps after that.
            Configure with 'CC="${CC} -m64"' perhaps?])
         gl_warned_about_y2038=yes
       fi
      ])
  fi])
])

AC_DEFUN([gl_YEAR2038],
[
  gl_YEAR2038_BODY([require-year2038-safe])
])
