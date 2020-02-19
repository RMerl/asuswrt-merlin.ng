m4_pattern_allow([^AU_])

# AU_HAVE_GNU_ERRNO_H
#--------------------
# Check for GNU extensions of errno.h.
AC_DEFUN([AU_HAVE_GNU_ERRNO_H], [
  AC_CACHE_CHECK([for GNU extensions of errno.h], [ac_cv_have_gnu_errno_h],[
    AC_TRY_COMPILE([
#define _GNU_SOURCE
#include <errno.h>
    ], [char *testvar = program_invocation_name;],
    [ac_cv_have_gnu_errno_h=yes], [ac_cv_have_gnu_errno_h=no])])
  AS_IF([test x"$ac_cv_have_gnu_errno_h" = x"yes"], [
    AC_DEFINE([HAVE_GNU_ERRNO_H], [1], [Define to 1 if you have the <errno.h> header file with GNU extensions.])
    AC_DEFINE([_GNU_SOURCE], [], [Force _GNU_SOURCE from AC_HAVE[]_GNU_ERRNO_H.])
    ])
])
