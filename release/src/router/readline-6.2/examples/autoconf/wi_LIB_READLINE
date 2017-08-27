dnl Borut Razem
dnl
dnl This macro checks for the presence of the readline library.
dnl It works also in cross-compilation environment.
dnl
dnl To get it into the aclocal.m4 dnl file, do this:
dnl   aclocal -I . --verbose
dnl
dnl The --verbose will show all of the files that are searched
dnl for .m4 macros.

AC_DEFUN([wi_LIB_READLINE], [
  dnl check for the readline.h header file

  AC_CHECK_HEADER(readline/readline.h)

  if test "$ac_cv_header_readline_readline_h" = yes; then
    dnl check the readline version

    cat > conftest.$ac_ext <<EOF
#include <stdio.h>
#include <readline/readline.h>
wi_LIB_READLINE_VERSION RL_VERSION_MAJOR RL_VERSION_MINOR
EOF

    wi_READLINE_VERSION=$($CPP $CPPFLAGS conftest.$ac_ext | sed -n -e "s/^wi_LIB_READLINE_VERSION  *\([[0-9\]][[0-9\]]*\)  *\([[0-9\]][[0-9\]]*\)$/\1.\2/p")
    rm -rf conftest*

    if test -n "$wi_READLINE_VERSION"; then
      wi_MAJOR=$(expr $wi_READLINE_VERSION : '\([[0-9]][[0-9]]*\)\.')
      wi_MINOR=$(expr $wi_READLINE_VERSION : '[[0-9]][[0-9]]*\.\([[0-9]][[0-9]]*$\)')
      if test $wi_MINOR -lt 10; then
        wi_MINOR=$(expr $wi_MINOR \* 10)
      fi
      wi_READLINE_VERSION=$(expr $wi_MAJOR \* 100 + $wi_MINOR)
    else
      wi_READLINE_VERSION=-1
    fi

    dnl check for the readline library

    ac_save_LIBS="$LIBS"
    # Note: $LIBCURSES is permitted to be empty.

    for LIBREADLINE in "-lreadline.dll" "-lreadline" "-lreadline $LIBCURSES" "-lreadline -ltermcap" "-lreadline -lncurses" "-lreadline -lcurses"
    do
      AC_MSG_CHECKING([for GNU Readline library $LIBREADLINE])

      LIBS="$ac_save_LIBS $LIBREADLINE"

      AC_TRY_LINK([
        /* includes */
        #include <stdio.h>
        #include <readline/readline.h>
      ],[
        /* function-body */
        int dummy = rl_completion_append_character; /* rl_completion_append_character appeared in version 2.1 */
        readline(NULL);
      ],[
        wi_cv_lib_readline=yes
        AC_MSG_RESULT(yes)
      ],[
        wi_cv_lib_readline=no
        AC_MSG_RESULT(no)
      ])

      if test "$wi_cv_lib_readline" = yes; then
        AC_SUBST(LIBREADLINE)
        AC_DEFINE_UNQUOTED(HAVE_LIBREADLINE, $wi_READLINE_VERSION, [Readline])
        break
      fi
    done

    LIBS="$ac_save_LIBS"
  fi
])
