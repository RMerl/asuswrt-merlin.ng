# strndup.m4
# serial 24
dnl Copyright (C) 2002-2003, 2005-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl This file is offered as-is, without any warranty.

AC_DEFUN([gl_FUNC_STRNDUP],
[
  dnl Persuade glibc <string.h> to declare strndup().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  AC_REQUIRE([AC_CANONICAL_HOST]) dnl for cross-compiles
  AC_REQUIRE([gl_STRING_H_DEFAULTS])
  AC_CHECK_DECLS_ONCE([strndup])
  AC_CHECK_FUNCS_ONCE([strndup])
  if test $ac_cv_have_decl_strndup = no; then
    HAVE_DECL_STRNDUP=0
  fi

  if test $ac_cv_func_strndup = yes; then
    HAVE_STRNDUP=1
    dnl AIX 5.1 has a function that fails to add the terminating '\0'.
    dnl AIX 7.3 has a function that does not support a zero length.
    AC_CACHE_CHECK([for working strndup], [gl_cv_func_strndup_works],
      [AC_RUN_IFELSE([
         AC_LANG_PROGRAM([[#include <string.h>
                           #include <stdlib.h>]], [[
#if !HAVE_DECL_STRNDUP
  extern
  #ifdef __cplusplus
  "C"
  #endif
  char *strndup (const char *, size_t);
#endif
  int result = 0;
  {
    char *s = strndup ("some longer string", 15);
    free (s);
    s = strndup ("shorter string", 13);
    if (s[13] != '\0')
      result |= 1;
    free (s);
  }
  if (strndup (NULL, 0) == NULL)
    result |= 2;
  return result;]])],
         [gl_cv_func_strndup_works=yes],
         [gl_cv_func_strndup_works=no],
         [case $host_os in
            aix*) gl_cv_func_strndup_works="guessing no";;
            *)    gl_cv_func_strndup_works="guessing yes";;
          esac
         ])])
    case $gl_cv_func_strndup_works in
      *no) REPLACE_STRNDUP=1 ;;
    esac
  else
    HAVE_STRNDUP=0
  fi
])
