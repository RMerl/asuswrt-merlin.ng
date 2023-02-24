dnl Process this file with autoconf to create configure.

AC_INIT(libnfnetlink, 1.0.2)
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([-Wall foreign subdir-objects
	tar-pax no-dist-gzip dist-bzip2 1.6])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

dnl kernel style compile messages
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AM_PROG_CC_C_O
AC_DISABLE_STATIC
AM_PROG_LIBTOOL

case "$host" in 
*-*-linux* | *-*-uclinux*) ;;
*) AC_MSG_ERROR([Linux only, dude!]);;
esac


dnl--------------------------------
dnl--------------------------------


dnl Output the makefile
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile doxygen.cfg
	include/libnfnetlink/Makefile utils/Makefile libnfnetlink.pc])
AC_OUTPUT
