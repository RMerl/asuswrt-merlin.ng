dnl Checks for pthreads library and headers
dnl
AC_DEFUN([AC_LIBPTHREAD], [

    dnl Check for library, but do not add -lpthreads to LIBS
    AC_CHECK_LIB([pthread], [pthread_create], [LIBPTHREAD=-lpthread],
                 [AC_MSG_ERROR([libpthread not found.])])
  AC_SUBST(LIBPTHREAD)

  AC_CHECK_HEADERS([pthread.h], ,
                   [AC_MSG_ERROR([libpthread headers not found.])])

])dnl
