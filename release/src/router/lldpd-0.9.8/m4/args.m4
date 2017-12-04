#
# lldp_ARG_WITH
#

dnl lldp_AC_EXPAND(var)

AC_DEFUN([lldp_AC_EXPAND], [
  dnl first expand prefix and exec_prefix if necessary
  prefix_save=$prefix
  exec_prefix_save=$exec_prefix

  dnl if no prefix given, then use /usr/local, the default prefix
  if test "x$prefix" = "xNONE"; then
    prefix="$ac_default_prefix"
  fi
  dnl if no exec_prefix given, then use prefix
  if test "x$exec_prefix" = "xNONE"; then
    exec_prefix=$prefix
  fi

  full_var="$1"
  dnl loop until it doesn't change anymore
  while true; do
    dnl echo DEBUG: full_var: $full_var
    new_full_var="`eval echo $full_var`"
    if test "x$new_full_var" = "x$full_var"; then break; fi
    full_var=$new_full_var
  done

  dnl clean up
  full_var=$new_full_var
  eval $2="$full_var"

  dnl restore prefix and exec_prefix
  prefix=$prefix_save
  exec_prefix=$exec_prefix_save
])

dnl lldp_ARG_WITH_UNQUOTED(name, help1, default)

AC_DEFUN([lldp_ARG_WITH_UNQUOTED],[
  AC_ARG_WITH([$1],
	AS_HELP_STRING([--with-$1],
		[$2 @<:@default=$3@:>@]),[
        AC_DEFINE_UNQUOTED(AS_TR_CPP([$1]), [$withval], [$2])
        AC_SUBST(AS_TR_CPP([$1]), [$withval])],[
	AC_DEFINE_UNQUOTED(AS_TR_CPP([$1]), [$3], [$2])
        AC_SUBST(AS_TR_CPP([$1]), [$3])
        eval with_[]m4_translit([$1], [-+.], [___])=$3
])])

dnl lldp_ARG_WITH(name, help1, default)

AC_DEFUN([lldp_ARG_WITH],[
  AC_ARG_WITH([$1],
	AS_HELP_STRING([--with-$1],
		[$2 @<:@default=$3@:>@]),[
        lldp_AC_EXPAND("$withval", expanded)
        AC_DEFINE_UNQUOTED(AS_TR_CPP([$1]), ["$expanded"], [$2])
        AC_SUBST(AS_TR_CPP([$1]), [$expanded])],[
        lldp_AC_EXPAND("$3", expanded)
	AC_DEFINE_UNQUOTED(AS_TR_CPP([$1]), ["$expanded"], [$2])
        AC_SUBST(AS_TR_CPP([$1]), [$expanded])
        eval with_[]m4_translit([$1], [-+.], [___])="$expanded"
])])

dnl lldp_ARG_ENABLE(name, help1, default)

AC_DEFUN([lldp_ARG_ENABLE],[
  AC_ARG_ENABLE([$1],
	AS_HELP_STRING([--enable-$1],
		[Enable $2 @<:@default=$3@:>@]),
	[enable_$1=$enableval], [enable_$1=$3])
  AC_MSG_CHECKING(whether to enable $2)
  if test x"$enable_$1" = x"yes"; then
     AC_MSG_RESULT(yes)
     AC_DEFINE([ENABLE_]AS_TR_CPP([$1]),, [$2])
  else
     AC_MSG_RESULT(no)
  fi
])
