# SPLIT_PACKAGE_VERSION
# ---------------------------
# Split package version in four parts
AC_DEFUN([SPLIT_PACKAGE_VERSION],
	[
	AC_REQUIRE([AC_PROG_SED])
	PACKAGE_VERSION_MAJOR=`echo "$PACKAGE_VERSION" | $SED 's/\([[0-9]]\+\).*/\1/'`
	PACKAGE_VERSION_MINOR=`echo "$PACKAGE_VERSION" | $SED 's/[[0-9]]\+\.\([[0-9]]\+\).*/\1/'`
	PACKAGE_VERSION_BUILD=`echo "$PACKAGE_VERSION" | $SED 's/[[0-9]]\+\.[[0-9]]\+\.\([[0-9]]\+\).*/\1/'`
	PACKAGE_VERSION_REVIEW=`echo "$PACKAGE_VERSION" | $SED 's/[[0-9]]\+\.[[0-9]]\+\.[[0-9]]\+\(.*\)/\1/'`
	AC_SUBST([PACKAGE_VERSION_MAJOR])
	AC_SUBST([PACKAGE_VERSION_MINOR])
	AC_SUBST([PACKAGE_VERSION_BUILD])
	AC_SUBST([PACKAGE_VERSION_REVIEW])
	]
)

