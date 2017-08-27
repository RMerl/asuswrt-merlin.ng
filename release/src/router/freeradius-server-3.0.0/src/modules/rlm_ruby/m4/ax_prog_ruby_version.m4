# ===========================================================================
#   http://www.gnu.org/software/autoconf-archive/ax_prog_ruby_version.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_PROG_RUBY_VERSION([VERSION],[ACTION-IF-TRUE],[ACTION-IF-FALSE])
#
# DESCRIPTION
#
#   Makes sure that ruby supports the version indicated. If true the shell
#   commands in ACTION-IF-TRUE are executed. If not the shell commands in
#   ACTION-IF-FALSE are run. Note if $RUBY is not set (for example by
#   running AC_CHECK_PROG or AC_PATH_PROG) the macro will fail.
#
#   Example:
#
#     AC_PATH_PROG([RUBY],[ruby])
#     AC_PROG_RUBY_VERSION([1.8.0],[ ... ],[ ... ])
#
#   This will check to make sure that the ruby you have supports at least
#   version 1.6.0.
#
#   NOTE: This macro uses the $RUBY variable to perform the check.
#   AX_WITH_PROG([RUBY],[ruby],[VALUE-IF-NOT-FOUND],[PATH]) can be used to
#   set that variable prior to running this macro. The $RUBY_VERSION
#   variable will be valorized with the detected version.
#
# LICENSE
#
#   Copyright (c) 2009 Francesco Salvestrini <salvestrini@users.sourceforge.net>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 11

AC_DEFUN([AX_PROG_RUBY_VERSION],[
    AC_REQUIRE([AC_PROG_SED])
    AC_REQUIRE([AC_PROG_GREP])

    AS_IF([test -n "$RUBY"],[
        ax_ruby_version="$1"

        AC_MSG_CHECKING([for ruby version])
        changequote(<<,>>)
        ruby_version=`$RUBY --version 2>&1 | $GREP "^ruby " | $SED -e 's/^.* \([0-9]*\.[0-9]*\.[0-9]*\) .*/\1/'`
        changequote([,])
        AC_MSG_RESULT($ruby_version)

	AC_SUBST([RUBY_VERSION],[$ruby_version])

        AX_COMPARE_VERSION([$ax_ruby_version],[le],[$ruby_version],[
	    :
            $2
        ],[
	    :
            $3
        ])
    ],[
        AC_MSG_WARN([could not find the ruby interpreter])
        $3
    ])
])
