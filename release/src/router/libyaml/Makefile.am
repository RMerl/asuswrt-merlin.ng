## Run `./bootstrap` to generate the "Makefile.in" files in this directory and
## the "$SUBDIRS" subdirectories.

SUBDIRS = include src . tests

EXTRA_DIST = README LICENSE CMakeLists.txt doc/doxygen.cfg

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = yaml-0.1.pc

maintainer-clean-local:
	rm -f aclocal.m4 config.h.in configure config/*
	-find ${builddir} -name Makefile.in -exec rm -f '{}' ';'

distclean-local:
	rm -fr tests/run-test-suite
	-git worktree prune

.PHONY: bootstrap
bootstrap: maintainer-clean
	./bootstrap
	./configure
	make

test: all
	make -C tests check-TESTS

test-suite: tests/run-test-suite
	make -C $< test

test-all: test test-suite

tests/run-test-suite:
	-git branch --track run-test-suite origin/run-test-suite
	-git worktree prune
	git worktree add $@ run-test-suite
