dnl Checks for matching sqlite3 header and library, and
dnl sufficient sqlite3 version.
dnl
AC_DEFUN([AC_SQLITE3_VERS], [
  AC_CHECK_HEADERS([sqlite3.h], ,)

  dnl look for the library; do not add to LIBS if found
  AC_CHECK_LIB([sqlite3], [sqlite3_libversion_number], [LIBSQLITE=-lsqlite3], ,)
  AC_SUBST(LIBSQLITE)

  AC_MSG_CHECKING(for suitable sqlite3 version)

  AC_CACHE_VAL([libsqlite3_cv_is_recent],
   [
    saved_LIBS="$LIBS"
    LIBS=-lsqlite3
    AC_TRY_RUN([
	#include <stdio.h>
	#include <sqlite3.h>
	int main()
	{
		int vers = sqlite3_libversion_number();

		return vers != SQLITE_VERSION_NUMBER ||
			vers < 3003000;
	}
       ], [libsqlite3_cv_is_recent=yes], [libsqlite3_cv_is_recent=no],
       [libsqlite3_cv_is_recent=unknown])
    LIBS="$saved_LIBS"])

  AC_MSG_RESULT($libsqlite3_cv_is_recent)
])dnl
