# $Id$
ACLOCAL_AMFLAGS = -I m4 -I libopts/m4

if NEED_LIBOPTS
SUBDIRS = scripts lib $(LIBOPTS_DIR) src
else
SUBDIRS = scripts lib src
endif

DIST_SUBDIRS = scripts lib libopts src docs test
.PHONY: manpages docs test man2html


dist-hook: version manpages

DOCS_DIR = $(top_builddir)/docs

manpages:
	cd src && make manpages

docs: manpages
	echo Making docs in $(DOCS_DIR)
	cd $(DOCS_DIR) && make docs

clean-docs:
	cd $(DOCS_DIR) && make clean-docs

postweb: docs doxygen
	cd $(DOCS_DIR) && make postweb

TEST_DIR = $(top_builddir)/test

autoopts:
	cd src && make autoopts

test:
	echo Making test in $(TEST_DIR)
	cd $(TEST_DIR) && make test

dlt_names:
	cat @SAVEFILE_C@ | $(top_builddir)/scripts/dlt2name.pl src/dlt_names.h

version:
	-rm -f src/common/git_version.c
	cd src/common && make git_version.c

distclean-local:
	-rm -rf autom4te.cache doxygen

doxygen: version
	doxygen doxygen.cfg

ncc:
	CC=ncc make
	
MOSTLYCLEANFILES = tcpreplay.spec *~

DISTCLEANFILES = .tm_project.cache stamp-h1 *.tar.*

MAINTAINERCLEANFILES = Makefile.in configure *.bak

EXTRA_DIST = doxygen.cfg.in \
	m4/libtool.m4 m4/ltoptions.m4 m4/ltsugar.m4 \
	m4/ltversion.m4 m4/lt~obsolete.m4 acinclude.m4
