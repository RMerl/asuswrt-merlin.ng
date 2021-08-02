## Process this file with automake to produce Makefile.in.


if ENABLE_NIST_TEST
check_PROGRAMS = nist
endif

AM_CFLAGS=-Wall

nist_LDADD = -lm

nist_SOURCES = cephes.c dfft.c matrix.c nist.c packtest.c special-functions.c cephes-protos.h matrix.h mconf.h special-functions.h nist.h

CLEANFILES = sample nist.out

MAINTAINERCLEANFILES = Makefile.in

if ENABLE_NIST_TEST
check-local:
	../src/haveged -n 16M $*
	./nist sample ${srcdir}
endif