# ===========================================================================
#          http://autoconf-archive.cryp.to/ax_cflags_gcc_option.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CFLAGS_GCC_OPTION (optionflag [,[shellvar][,[A][,[NA]]])
#
# DESCRIPTION
#
#   AX_CFLAGS_GCC_OPTION(-fvomit-frame) would show a message as like
#   "checking CFLAGS for gcc -fvomit-frame ... yes" and adds the optionflag
#   to CFLAGS if it is understood. You can override the shellvar-default of
#   CFLAGS of course. The order of arguments stems from the explicit macros
#   like AX_CFLAGS_WARN_ALL.
#
#   The cousin AX_CXXFLAGS_GCC_OPTION would check for an option to add to
#   CXXFLAGS - and it uses the autoconf setup for C++ instead of C (since it
#   is possible to use different compilers for C and C++).
#
#   The macro is a lot simpler than any special AX_CFLAGS_* macro (or
#   ac_cxx_rtti.m4 macro) but allows to check for arbitrary options.
#   However, if you use this macro in a few places, it would be great if you
#   would make up a new function-macro and submit it to the ac-archive.
#
#     - $1 option-to-check-for : required ("-option" as non-value)
#     - $2 shell-variable-to-add-to : CFLAGS (or CXXFLAGS in the other case)
#     - $3 action-if-found : add value to shellvariable
#     - $4 action-if-not-found : nothing
#
#   There are other variants that emerged from the original macro variant
#   which did just test an option to be possibly added. However, some
#   compilers accept an option silently, or possibly for just another option
#   that was not intended. Therefore, we have to do a generic test for a
#   compiler family. For gcc we check "-pedantic" being accepted which is
#   also understood by compilers who just want to be compatible with gcc
#   even when not being made from gcc sources.
#
#   This version has been modified by Vincent Bernat <bernat@luffy.cx>
#   to ensure that the shell variable the option is added to is
#   prepended to the test variable (like a regular CFLAGS).
#
#   See also: AX_CFLAGS_SUN_OPTION, AX_CFLAGS_HPUX_OPTION,
#   AX_CFLAGS_AIX_OPTION, and AX_CFLAGS_IRIX_OPTION.
#
# LICENSE
#
#   Copyright (c) 2008 Guido U. Draheim <guidod@gmx.de>
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

dnl -------------------------------------------------------------------------

AC_DEFUN([AX_CFLAGS_GCC_OPTION], [dnl
AS_VAR_PUSHDEF([FLAGS],[CFLAGS])dnl
AS_VAR_PUSHDEF([VAR],[ac_cv_cflags_gcc_option_$1])dnl
AC_CACHE_CHECK([FLAGS for gcc m4_ifval($1,$1,-option)],
VAR,[AS_VAR_SET([VAR],["no, unknown"])
 AC_LANG_SAVE
 AC_LANG_C
 ac_save_[]FLAGS="$[]FLAGS"
for ac_arg dnl
in "-pedantic -Werror % m4_ifval($1,$1,-option)"  dnl   GCC
   "-pedantic % m4_ifval($1,$1,-option) %% no, obsolete"  dnl new GCC
   #
do FLAGS="$ac_save_[]FLAGS $[]m4_ifval($2,$2,) "`echo $ac_arg | sed -e 's,%%.*,,' -e 's,%,,'`
   AC_TRY_COMPILE([],[return 0;],
   [AS_VAR_SET([VAR],[`echo $ac_arg | sed -e 's,.*% *,,'`]) ; break])
done
 FLAGS="$ac_save_[]FLAGS"
 AC_LANG_RESTORE
])
AS_VAR_COPY([ac_res], [VAR])
case ".${ac_res}" in
     .ok|.ok,*) m4_ifvaln($3,$3) ;;
   .|.no|.no,*) m4_ifvaln($4,$4) ;;
   *) m4_ifvaln($3,$3,[
   if echo " $[]m4_ifval($2,$2,FLAGS) " | grep " ${ac_res} " 2>&1 >/dev/null
   then AC_RUN_LOG([: m4_ifval($2,$2,FLAGS) does contain ${ac_res}])
   else AC_RUN_LOG([: m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) ${ac_res}"])
                      m4_ifval($2,$2,FLAGS)="$m4_ifval($2,$2,FLAGS) ${ac_res}"
   fi ]) ;;
esac
AS_VAR_POPDEF([VAR])dnl
AS_VAR_POPDEF([FLAGS])dnl
])
