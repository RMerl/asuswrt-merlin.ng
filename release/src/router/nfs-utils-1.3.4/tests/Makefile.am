## Process this file with automake to produce Makefile.in

check_PROGRAMS = statdb_dump
statdb_dump_SOURCES = statdb_dump.c

statdb_dump_LDADD = ../support/nfs/libnfs.a \
		    ../support/nsm/libnsm.a $(LIBCAP)

SUBDIRS = nsm_client

MAINTAINERCLEANFILES = Makefile.in

TESTS = t0001-statd-basic-mon-unmon.sh
EXTRA_DIST = test-lib.sh $(TESTS)
