# wget_manywarnings.m4 serial 1
dnl Copyright (C) 2016-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Tim Ruehsen

# wget_WORD_SET(RESULT, SET, WORDS)
# --------------------------------------------------
# Add each word in WORDS to SET if not already there and store in RESULT.
# Words separated by whitespace.
AC_DEFUN([wget_WORD_SET], [
  ret=$2
  words=" $2 "
  for word in $3; do
    if test "${words#*" $word "}" = "$words"; then ret="$ret $word"; fi
  done
  $1=$ret
])

# wget_WORD_REMOVE(RESULT, SET, WORDS)
# --------------------------------------------------
# Remove each word in WORDS from SET and store in RESULT.
# Words separated by whitespace.
AC_DEFUN([wget_WORD_REMOVE], [
  ret=
  words=" $3 "
  for word in $2; do
     if test "${words#*" $word "}" = "$words"; then ret="$ret $word"; fi
  done
  $1=$ret
])

# wget_MANYWARNINGS(VARIABLE, LANGUAGE)
# -----------------------------
# Add LANGUAGE related GCC warnings to VARIABLE.
# This only works for gcc >= 4.3.
AC_DEFUN([wget_MANYWARNINGS], [
  #
  # check if manywarnings is requested
  #
  AC_ARG_ENABLE([manywarnings],
    [AS_HELP_STRING([--enable-manywarnings], [Turn on extra compiler warnings (for developers)])],
    [case $enableval in
       yes|no) ;;
       *) AC_MSG_ERROR([Bad value $enableval for --enable-manywarnings option]) ;;
     esac
       wget_manywarnings=$enableval
    ], [
      test -f .manywarnings && wget_manywarnings=yes || wget_manywarnings=no
    ]
  )

  $1=""

  if test "$wget_manywarnings" = yes; then
    # AC_PROG_CC sets $GCC to 'yes' if compiler is gcc
    # AC_REQUIRE([AC_PROG_CC])

    case $CC in
      *gcc*) CCNAME="gcc";;
      *clang*) CCNAME="clang";;
    esac

    if test "$CCNAME" = "gcc"; then
      test -n "$2" && wget_LANGUAGE=$2 || wget_LANGUAGE=C

      # add -Wall -Wextra to reduce number of warn flags
      wget_WORD_SET([wget_WARN_CFLAGS], [$CFLAGS], ["-Wall -Wextra -Wformat=2"])

      # collect all disabled warn flags in $WARN_CFLAGS
      wget_WARN_CFLAGS=$wget_WARN_CFLAGS" "$($CC $wget_WARN_CFLAGS -Q --help=warning,$wget_LANGUAGE|\
        awk '{ if (([$]2 == "[[disabled]]" || [$]2 == "")\
          && [$]1!~/=/ && [$]1~/^-W/ && [$]1!~/</ && [$]1!="-Wall") print [$]1 }')

      GCC_VERSION=$($CC -dumpversion | cut -f1 -d.)
      if test $GCC_VERSION -ge 6; then
        wget_WARN_CFLAGS=$wget_WARN_CFLAGS" -Warray-bounds=2 -Wnormalized=nfc -Wshift-overflow=2 -Wunused-const-variable=2"
      fi
      if test $GCC_VERSION -ge 7; then
        wget_WARN_CFLAGS=$wget_WARN_CFLAGS" -Wformat-overflow=2 -Wformat-truncation=1 -Wstringop-overflow=2"
      fi

    elif test "$CCNAME" = "clang"; then
      # set all warn flags on
      wget_WORD_SET([wget_WARN_CFLAGS], [$CFLAGS], ["-Weverything"])
    fi

    $1=$wget_WARN_CFLAGS
  fi
])
