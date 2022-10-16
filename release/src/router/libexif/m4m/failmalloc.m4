dnl Search for libfailmalloc to use for testing
AC_DEFUN([CHECK_FAILMALLOC],[dnl
  dnl Libtool sets the default library paths
  LT_INIT([win32-dll])
  path_provided=
  failmalloc_requested=  dnl Either implicitly or explicitly
  AC_ARG_WITH(failmalloc, [  --with-failmalloc=PATH  use Failmalloc for tests], [
      failmalloc_requested=1
      if test x"$withval" = "x" -o x"$withval" = x"yes"; then
        failmalloc_search_path="$sys_lib_search_path_spec"
      elif test x"$withval" = x"no"; then
        failmalloc_search_path=""
        failmalloc_requested=
      else
        failmalloc_search_path="$withval"
        path_provided=1
      fi
    ], [failmalloc_search_path="$sys_lib_search_path_spec"]
  )
  libfailmalloc_file=libfailmalloc.so.0
  FAILMALLOC_PATH=

  dnl Skip the check if we're cross-compiling, unless the user explicitly requested it
  if test x"$cross_compiling" = x"no" -o x"$failmalloc_requested" = x"1"; then
    dnl Check if the argument is a directory
    for d in $failmalloc_search_path; do
        AC_CHECK_FILE([$d/$libfailmalloc_file], [
          FAILMALLOC_PATH="$d/$libfailmalloc_file"
          break
        ], [])
    done
    if test -z "$FAILMALLOC_PATH" -a -n "$path_provided"; then
      dnl Check if the argument is a file
      AC_CHECK_FILE([$failmalloc_search_path], [FAILMALLOC_PATH="$failmalloc_search_path"], [])
    fi
  fi

  AC_MSG_CHECKING([for failmalloc])
  dnl Make sure AC_CHECK_FILE didn't find a directory by mistake
  if test -n "$FAILMALLOC_PATH" -a -f "$FAILMALLOC_PATH"; then
    AC_MSG_RESULT([yes])
  else
    if test -n "$path_provided"; then
      AC_MSG_ERROR([$libfailmalloc_file was not found at $failmalloc_search_path])
    else
      if test x"$cross_compiling" != x"no"; then
        AC_MSG_RESULT([no (cross compiling)])
      else
        AC_MSG_RESULT([no])
      fi
    fi
  fi
  AC_SUBST(FAILMALLOC_PATH)
  AM_CONDITIONAL(USE_FAILMALLOC, [test x"$FAILMALLOC_PATH" != x])
])

