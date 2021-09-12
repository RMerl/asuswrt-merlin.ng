#
# lldp_CHECK_LIBCAP
#

AC_DEFUN([lldp_CHECK_LIBCAP], [
    PKG_CHECK_MODULES([libcap], [libcap >= 2], [
       AC_DEFINE([HAVE_LINUX_CAPABILITIES], 1, [Define to indicate support of linux capabilities])
    ], [
      libcap_LIBS=-lcap
      libcap_CFLAGS=
      _save_libs="$LIBS"
      LIBS="$LIBS ${libcap_LIBS}"
      AC_MSG_CHECKING([libcap (without pkg-config)])
      AC_TRY_LINK_FUNC([cap_set_proc], [
         AC_DEFINE([HAVE_LINUX_CAPABILITIES], 1, [Define to indicate support of linux capabilities])
         AC_MSG_RESULT(yes)
      ], [
         libcap_LIBS=
         AC_MSG_RESULT(no)
      ])
      LIBS="$_save_libs"
    ])
    AC_SUBST([libcap_LIBS])
    AC_SUBST([libcap_CFLAGS])
])
