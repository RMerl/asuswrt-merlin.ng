#
# lldp_SYSTEMTAP
#
# Check for DTrace/Systemtap support

AC_DEFUN([lldp_SYSTEMTAP], [
  # Enable systemtap support
  lldp_ARG_ENABLE([dtrace], [systemtap/DTrace trace support], [no])
  AM_CONDITIONAL([ENABLE_SYSTEMTAP], [test x"$enable_dtrace" = x"yes"])
  if test x"$enable_dtrace" = x"yes"; then
     AC_CHECK_PROGS(DTRACE, dtrace)
     if test -z "$DTRACE"; then
       AC_MSG_ERROR([*** dtrace command not found])
     fi
     AC_CHECK_HEADER([sys/sdt.h],,[AC_MSG_ERROR([*** no sys/sdt.h header found])])
  fi
])
