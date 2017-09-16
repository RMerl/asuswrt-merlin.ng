dnl Checks for libcap.so
dnl
AC_DEFUN([AC_LIBCAP], [

  dnl look for prctl
  AC_CHECK_FUNC([prctl], , AC_MSG_ERROR([prctl syscall is not available]))

  AC_ARG_ENABLE([caps],
    [AS_HELP_STRING([--disable-caps], [Disable capabilities support])])

  LIBCAP=

  if test "x$enable_caps" != "xno" ; then
    dnl look for the library; do not add to LIBS if found
    AC_CHECK_LIB([cap], [cap_get_proc], [LIBCAP=-lcap], ,)

    AC_CHECK_HEADERS([sys/capability.h], ,
      [test "x$enable_caps" = "xyes" && AC_MSG_ERROR([libcap headers not found.])])
  fi

  AC_SUBST(LIBCAP)

])dnl
