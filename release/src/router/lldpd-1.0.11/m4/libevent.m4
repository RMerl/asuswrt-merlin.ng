#
# lldp_CHECK_LIBEVENT
#

AC_DEFUN([lldp_CHECK_LIBEVENT], [
  # Do we require embedded libevent?
  AC_ARG_WITH([embedded-libevent],
    AS_HELP_STRING(
      [--with-embedded-libevent],
      [Use embedded libevent @<:@default=auto@:>@]
  ), [], [with_embedded_libevent=auto])
  if test x"$with_embedded_libevent" = x"yes"; then
     LIBEVENT_EMBEDDED=1
  else
    # If not forced, check first with pkg-config
    PKG_CHECK_MODULES([libevent], [libevent >= 2.0.5], [
       # Check if we have a working libevent
       AC_MSG_CHECKING([if system libevent works as expected])
       _save_CFLAGS="$CFLAGS"
       _save_LIBS="$LIBS"
       CFLAGS="$CFLAGS $libevent_CFLAGS"
       LIBS="$LIBS $libevent_LIBS"
       AC_TRY_LINK([
@%:@include <sys/time.h>
@%:@include <sys/types.h>
@%:@include <event2/event.h>], [ struct event_base *base = event_base_new(); event_new(base, -1, 0, NULL, NULL); ],
       [
         AC_MSG_RESULT([yes])
       ], [
         if test x"$with_embedded_libevent" = x"auto"; then
           AC_MSG_RESULT([no, using shipped libevent])
           LIBEVENT_EMBEDDED=1
         else
           AC_MSG_ERROR([*** unusable system libevent])
         fi
       ])
       CFLAGS="$_save_CFLAGS"
       LIBS="$_save_LIBS"
    ], [
      # No appropriate version, let's use the shipped copy if possible
      if test x"$with_embedded_libevent" = x"auto"; then
        AC_MSG_NOTICE([using shipped libevent])
        LIBEVENT_EMBEDDED=1
      else
        AC_MSG_ERROR([*** libevent not found])
      fi
    ])
  fi

  if test x"$LIBEVENT_EMBEDDED" != x; then
    unset libevent_LIBS
    libevent_CFLAGS="-I\$(top_srcdir)/libevent/include -I\$(top_builddir)/libevent/include"
    libevent_LDFLAGS="\$(top_builddir)/libevent/libevent.la"
  fi

  # Call ./configure in libevent. Need it for make dist...
  libevent_configure_args="$libevent_configure_args --disable-libevent-regress"
  libevent_configure_args="$libevent_configure_args --disable-thread-support"
  libevent_configure_args="$libevent_configure_args --disable-openssl"
  libevent_configure_args="$libevent_configure_args --disable-malloc-replacement"
  libevent_configure_args="$libevent_configure_args --disable-debug-mode"
  libevent_configure_args="$libevent_configure_args --enable-function-sections"
  libevent_configure_args="$libevent_configure_args --disable-shared"
  libevent_configure_args="$libevent_configure_args --with-pic"
  libevent_configure_args="$libevent_configure_args --enable-static"
  lldp_CONFIG_SUBDIRS([libevent], [$libevent_configure_args])

  AM_CONDITIONAL([LIBEVENT_EMBEDDED], [test x"$LIBEVENT_EMBEDDED" != x])
  AC_SUBST([libevent_LIBS])
  AC_SUBST([libevent_CFLAGS])
  AC_SUBST([libevent_LDFLAGS])
])
