dnl Checks for keyutils library and headers
dnl
AC_DEFUN([AC_KEYUTILS], [

  dnl Check for libkeyutils; do not add to LIBS if found
  AC_CHECK_LIB([keyutils], [keyctl_instantiate], [LIBKEYUTILS=-lkeyutils], ,)
  AC_SUBST(LIBKEYUTILS)

  AC_CHECK_HEADERS([keyutils.h])

  AC_CHECK_LIB([keyutils], [find_key_by_type_and_desc],
		[AC_DEFINE([HAVE_FIND_KEY_BY_TYPE_AND_DESC], [1],
			[Define to 1 if you have the `find_key_by_type_and_desc' function.])],)

])dnl
