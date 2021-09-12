#
# lldp_CHECK___PROGNAME
#
AC_DEFUN([lldp_CHECK___PROGNAME],[
  AC_CACHE_CHECK([whether libc defines __progname], lldp_cv_check___progname, [
    AC_TRY_LINK([],
	[ extern char *__progname; printf("%s", __progname); ],
	[ lldp_cv_check___progname="yes" ],
	[ lldp_cv_check___progname="no" ])
  ])
  if test x"$lldp_cv_check___progname" = x"yes"; then
     AC_DEFINE([HAVE___PROGNAME], [1], [Define if libc defines __progname])
  fi
])
