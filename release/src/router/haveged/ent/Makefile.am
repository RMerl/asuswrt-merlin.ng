## Process this file with automake to produce Makefile.in.

if ENABLE_ENT_TEST
check_PROGRAMS = entest
endif

AM_CFLAGS=-Wall

entest_LDADD = -lm

entest_SOURCES = entest.c iso8859.c randtest.c chisq.c iso8859.h randtest.h 

CLEANFILES = sample *log
clean-local:
	rm -rf chi_square

MAINTAINERCLEANFILES = Makefile.in

if ENABLE_ENT_TEST
check-local:
	./entest -t ${srcdir}/entitle.gif
	./test.sh
endif
