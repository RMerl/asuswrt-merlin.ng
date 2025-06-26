EXTRA_DIST = CMakeLists.txt

EXAMPLES =			\
	jwtgen			\
	jwtauth

bin_PROGRAMS = $(EXAMPLES)

jwtgen_SOURCES = main-gen.c
jwtauth_SOURCES = main-auth.c

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -Wall -DKEYDIR="\"$(srcdir)/keys\"" -D_GNU_SOURCE
AM_LDFLAGS = -L$(top_builddir)/libjwt
LDADD = -ljwt

