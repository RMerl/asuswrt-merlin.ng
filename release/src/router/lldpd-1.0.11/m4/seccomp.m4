#
# lldp_CHECK_SECCOMP
#

AC_DEFUN([lldp_CHECK_SECCOMP], [
   if test x"$with_seccomp" != x"no"; then
      PKG_CHECK_MODULES([libseccomp], [libseccomp >= 1], [
         AC_SUBST([libseccomp_LIBS])
         AC_SUBST([libseccomp_CFLAGS])
         AC_DEFINE_UNQUOTED([USE_SECCOMP], 1, [Define to indicate to enable seccomp support])
         with_seccomp=yes
      ], [
         if test x"$with_seccomp" = x"yes"; then
            AC_MSG_FAILURE([*** no seccomp support found])
         fi
         with_seccomp=no
      ])
   fi
])
