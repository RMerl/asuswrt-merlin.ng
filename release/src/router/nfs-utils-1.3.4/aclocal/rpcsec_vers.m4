dnl Checks librpcsec version
AC_DEFUN([AC_RPCSEC_VERSION], [

  AC_ARG_WITH([gssglue],
	[AC_HELP_STRING([--with-gssglue], [Use libgssglue for GSS support])])
  if test x"$with_gssglue" = x"yes"; then
    PKG_CHECK_MODULES([GSSGLUE], [libgssglue >= 0.3])
    AC_CHECK_LIB([gssglue], [gss_set_allowable_enctypes])
  fi

  dnl TI-RPC replaces librpcsecgss
  if test "$enable_tirpc" = no; then
    PKG_CHECK_MODULES([RPCSECGSS], [librpcsecgss >= 0.16])
  fi

])dnl
