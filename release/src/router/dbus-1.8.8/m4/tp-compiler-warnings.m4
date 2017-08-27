dnl TP_COMPILER_WARNINGS(VARIABLE, WERROR_BY_DEFAULT, DESIRABLE, UNDESIRABLE)
dnl $1 (VARIABLE): the variable to put flags into
dnl $2 (WERROR_BY_DEFAULT): a command returning true if -Werror should be the
dnl     default
dnl $3 (DESIRABLE): warning flags we want (e.g. all extra shadow)
dnl $4 (UNDESIRABLE): warning flags we don't want (e.g.
dnl   missing-field-initializers unused-parameter)
AC_DEFUN([TP_COMPILER_WARNINGS],
[
  AC_REQUIRE([AC_ARG_ENABLE])dnl
  AC_REQUIRE([AC_HELP_STRING])dnl
  AC_REQUIRE([TP_COMPILER_FLAG])dnl

  tp_warnings=""
  for tp_flag in $3; do
    TP_COMPILER_FLAG([-W$tp_flag], [tp_warnings="$tp_warnings -W$tp_flag"])
  done

  tp_error_flags="-Werror"
  TP_COMPILER_FLAG([-Werror], [tp_werror=yes], [tp_werror=no])

  for tp_flag in $4; do
    TP_COMPILER_FLAG([-Wno-$tp_flag],
      [tp_warnings="$tp_warnings -Wno-$tp_flag"])
dnl Yes, we do need to use both -Wno-foo and -Wno-error=foo. Simon says:
dnl     some warnings we explicitly don't want, like unused-parameter, but
dnl     they're in -Wall. when a distro using cdbs compiles us, we have:
dnl       -Werror -Wno-unused-parameter      -Wall
dnl         ^ from us                         ^ from cdbs
dnl     which turns -Wunused-parameter back on, in effect
    TP_COMPILER_FLAG([-Wno-error=$tp_flag],
      [tp_error_flags="$tp_error_flags -Wno-error=$tp_flag"], [tp_werror=no])
  done

  AC_ARG_ENABLE([Werror],
    AC_HELP_STRING([--disable-Werror],
      [compile without -Werror (normally enabled in development builds)]),
    tp_werror=$enableval, :)

  if test "x$tp_werror" = xyes && $2; then
dnl We put -Wno-error=foo before -Wno-foo because clang interprets -Wall
dnl -Werror -Wno-foo -Wno-error=foo as “make foo a non-fatal warning”, but does
dnl what we want if you reverse them.
    $1="$tp_error_flags $tp_warnings"
  else
    $1="$tp_warnings"
  fi

])
