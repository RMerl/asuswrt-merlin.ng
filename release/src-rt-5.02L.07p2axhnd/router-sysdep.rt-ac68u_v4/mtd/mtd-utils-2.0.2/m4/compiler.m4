dnl Copyright (C) 2008-2011 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Simon Josefsson
dnl -- derivated from coreutils m4/warnings.m4

# UL_AS_VAR_APPEND(VAR, VALUE)
# ----------------------------
# Provide the functionality of AS_VAR_APPEND if Autoconf does not have it.
m4_ifdef([AS_VAR_APPEND],
[m4_copy([AS_VAR_APPEND], [UL_AS_VAR_APPEND])],
[m4_define([UL_AS_VAR_APPEND],
[AS_VAR_SET([$1], [AS_VAR_GET([$1])$2])])])

# UL_ADD_WARN(COMPILER_OPTION [, VARNAME])
# ------------------------
# Adds parameter to WARN_CFLAGS (or to $VARNAME) if the compiler supports it.
AC_DEFUN([UL_WARN_ADD], [
  m4_define([warnvarname], m4_default([$2],WARN_CFLAGS))
  AS_VAR_PUSHDEF([ul_Warn], [ul_cv_warn_$1])dnl
  AC_CACHE_CHECK([whether compiler handles $1], m4_defn([ul_Warn]), [
    # store AC_LANG_WERROR status, then turn it on
    save_ac_[]_AC_LANG_ABBREV[]_werror_flag="${ac_[]_AC_LANG_ABBREV[]_werror_flag}"
    AC_LANG_WERROR

    ul_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="-Werror ${CPPFLAGS} $1"
    AC_PREPROC_IFELSE([AC_LANG_PROGRAM([])],
                      [AS_VAR_SET(ul_Warn, [yes])],
                      [AS_VAR_SET(ul_Warn, [no])])
    # restore AC_LANG_WERROR
    ac_[]_AC_LANG_ABBREV[]_werror_flag="${save_ac_[]_AC_LANG_ABBREV[]_werror_flag}"

    CPPFLAGS="$ul_save_CPPFLAGS"
  ])
  AS_VAR_IF(ul_Warn, [yes], [UL_AS_VAR_APPEND(warnvarname, [" $1"])])
])

