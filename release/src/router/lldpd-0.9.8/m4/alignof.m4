#
# lldp_CHECK_ALIGNOF
#
AC_DEFUN([lldp_CHECK_ALIGNOF],[
  AC_CACHE_CHECK([whether compiler understands __alignof__], lldp_cv_check_alignof, [
    AC_TRY_COMPILE([],
	[ return __alignof__(long); ],
	[ lldp_cv_check_alignof="yes" ],
	[ lldp_cv_check_alignof="no" ])
  ])
  if test x"$lldp_cv_check_alignof" = x"yes"; then
     AC_DEFINE([HAVE_ALIGNOF], [1], [Define if __alignof__ operator is available])
  fi
])
