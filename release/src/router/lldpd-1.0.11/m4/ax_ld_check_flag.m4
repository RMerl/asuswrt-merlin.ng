# ===========================================================================
#     http://www.gnu.org/software/autoconf-archive/ax_ld_check_flag.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_LDFLAGS_OPTION(FLAG-TO-CHECK,[VAR],[NOTFOUND])
#
# DESCRIPTION
#
#   This macro tests if the C compiler supports the flag FLAG-TO-CHECK. If
#   successfull add it to VAR.
#
#   This code is inspired from KDE_CHECK_COMPILER_FLAG macro. Thanks to
#   Bogdan Drozdowski <bogdandr@op.pl> for testing and bug fixes.
#
#   This version has been (heavily) modified by Vincent Bernat
#   <bernat@luffy.cx> to match AX_CFLAGS_GCC_OPTION.
#
# LICENSE
#
#   Copyright (c) 2008 Francesco Salvestrini <salvestrini@users.sourceforge.net>
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

#serial 6

AC_DEFUN([AX_LDFLAGS_OPTION],[
  AC_PREREQ([2.61])
  AC_REQUIRE([AC_PROG_SED])

  flag=`echo "$1" | $SED 'y% .=/+-(){}<>:*,%_______________%'`

  AC_CACHE_CHECK([whether the linker accepts the $1 flag],
    [ax_cv_ld_check_flag_$flag],[

    AC_LANG_SAVE
    AC_LANG_C

    save_LDFLAGS="$LDFLAGS"
    LDFLAGS="-Werror $LDFLAGS $[]m4_ifval($2,$2,) $1"
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([],[])
    ],[
      eval "ax_cv_ld_check_flag_$flag=yes"
    ],[
      eval "ax_cv_ld_check_flag_$flag=no"
    ])

    LDFLAGS="$save_LDFLAGS"

    AC_LANG_RESTORE

  ])

  AS_IF([eval "test \"`echo '$ax_cv_ld_check_flag_'$flag`\" = yes"],[
    m4_ifval($2,$2,LDFLAGS)="$[]m4_ifval($2,$2,LDFLAGS) $1"
  ],[
    :; $3
  ])
])
