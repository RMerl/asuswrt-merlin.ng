EXTRA_DIST = CMakeLists.txt keys

TESTS =			\
	jwt_new		\
	jwt_grant	\
	jwt_header	\
	jwt_dump	\
	jwt_encode	\
	jwt_rsa		\
	jwt_ec		\
	jwt_validate

if !HAVE_OPENSSL
TESTS += jwt_rsa_pss
endif

check_PROGRAMS = $(TESTS)

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -Wall $(CHECK_CFLAGS) -DKEYDIR="\"$(srcdir)/keys\"" -D_GNU_SOURCE
AM_LDFLAGS = -L$(top_builddir)/libjwt
LDADD = -ljwt $(CHECK_LIBS)

@CODE_COVERAGE_RULES@
@VALGRIND_CHECK_RULES@
