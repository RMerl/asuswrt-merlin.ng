## Process this file with automake to produce Makefile.in
# Copyright (C) 1992, 1999, 2000, 2002 Free Software Foundation, Inc.
#
# This file is part of Libgcrypt.
#
# Libgcrypt is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of
# the License, or (at your option) any later version.
#
# Libgcrypt is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, see <http://www.gnu.org/licenses/>.
# SPDX-License-Identifier: LGPL-2.1-or-later

# Location of the released tarball archives.  This is prefixed by
# the variable RELEASE_ARCHIVE in ~/.gnupg-autogen.rc.  For example:
# RELEASE_ARCHIVE=wk@somehost:archive/tarballs
RELEASE_ARCHIVE_SUFFIX  = libgcrypt/v1.10
# The variable RELEASE_SIGNING_KEY in ~/.gnupg-autogen.rc is used
# to specify the key for signing.  For example:
# RELEASE_SIGNKEY=D8692123C4065DEA5E0F3AB5249B39D24F25E3B6


ACLOCAL_AMFLAGS = -I m4
AM_DISTCHECK_DVI_TARGET = pdf
DISTCHECK_CONFIGURE_FLAGS = --disable-random-daemon --enable-doc \
                            --enable-random=auto

# (A suitable gitlog-to-changelog script can be found in GnuPG master.)
GITLOG_TO_CHANGELOG=gitlog-to-changelog

if BUILD_DOC
doc = doc
else
doc =
endif


DIST_SUBDIRS = m4 compat mpi cipher random src doc tests
SUBDIRS =         compat mpi cipher random src $(doc) tests

EXTRA_DIST = autogen.sh autogen.rc README.GIT LICENSES      	           \
             ChangeLog-2011 build-aux/ChangeLog-2011 doc/ChangeLog-2011    \
             m4/ChangeLog-2011 cipher/ChangeLog-2011 src/ChangeLog-2011    \
             random/ChangeLog-2011 tests/ChangeLog-2011 mpi/ChangeLog-2011 \
             build-aux/git-log-footer build-aux/git-log-fix VERSION

DISTCLEANFILES =


# Add all the files listed in "distfiles" files to the distribution
dist-hook: gen-ChangeLog
	@set -e; \
	 for file in `cd $(top_srcdir); \
	      find mpi -type f -name distfiles`; do \
	    dir=`dirname $$file` ; $(mkinstalldirs) $(distdir)/$$dir ; \
	    for i in distfiles `cat $(top_srcdir)/$$file` ; do \
		ln $(top_srcdir)/$$dir/$$i $(distdir)/$$dir/$$i 2> /dev/null \
		|| cp -p $(top_srcdir)/$$dir/$$i $(distdir)/$$dir/$$i; \
	    done ; \
	done

distcheck-hook:
	set -e; ( \
	pref="#+macro: $$(echo $(PACKAGE_NAME)|tr '-' '_')_" ;\
	reldate="$$(date -u +%Y-%m-%d)" ;\
        echo "$${pref}ver  $(PACKAGE_VERSION)"  ;\
        echo "$${pref}date $${reldate}" ;\
        list='$(DIST_ARCHIVES)'; for i in $$list; do \
	  case "$$i" in *.tar.bz2) \
            echo "$${pref}size $$(wc -c <$$i|awk '{print int($$1/1024)}')k" ;\
	    echo "$${pref}sha1 $$(sha1sum <$$i|cut -d' ' -f1)" ;\
	    echo "$${pref}sha2 $$(sha256sum <$$i|cut -d' ' -f1)" ;;\
	  esac;\
	done ) | tee $(distdir).swdb



gen_start_date = 2011-12-01T14:00:00
.PHONY: gen-ChangeLog
gen-ChangeLog:
	if test -e $(top_srcdir)/.git; then				\
	  (cd $(top_srcdir) &&                      			\
	    $(GITLOG_TO_CHANGELOG) --append-dot --tear-off		\
	    --amend=build-aux/git-log-fix	 			\
	    --since=$(gen_start_date) ) > $(distdir)/cl-t;		\
          cat $(top_srcdir)/build-aux/git-log-footer >> $(distdir)/cl-t;\
	  rm -f $(distdir)/ChangeLog;					\
	  mv $(distdir)/cl-t $(distdir)/ChangeLog;			\
	fi



stowinstall:
	$(MAKE) $(AM_MAKEFLAGS) install prefix=/usr/local/stow/libgcrypt

# Macro to help the release target.
RELEASE_NAME = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)

release:
	+(set -e;\
	 if [ "$(abs_top_builddir)" = "$(abs_top_srcdir)" ]; then \
           echo "error: build directory must not be the source directory" >&2;\
	   exit 2;\
         fi ;\
	 echo "/* Build started at $$(date -uIseconds) */" ;\
	 cd $(top_srcdir); \
	 ./autogen.sh --force; \
	 cd $(abs_top_builddir); \
	 rm -rf dist; mkdir dist ; cd dist ; \
	 $(abs_top_srcdir)/configure --enable-maintainer-mode; \
	 $(MAKE) distcheck; \
	 echo "/* Build finished at $$(date -uIseconds) */" ;\
         echo "/*" ;\
	 echo " * Please run the final step interactivly:" ;\
	 echo " *   make sign-release" ;\
	 echo " */" ;\
	) 2>&1 | tee "$(RELEASE_NAME).buildlog"

sign-release:
	 +(set -e; \
	  test $$(pwd | sed 's,.*/,,') = dist || cd dist; \
	  x=$$(grep '^RELEASE_ARCHIVE=' $$HOME/.gnupg-autogen.rc|cut -d= -f2);\
          if [ -z "$$x" ]; then \
             echo "error: RELEASE_ARCHIVE missing in ~/.gnupg-autogen.rc">&2; \
             exit 2;\
          fi;\
          myarchive="$$x/$(RELEASE_ARCHIVE_SUFFIX)";\
	  x=$$(grep '^RELEASE_SIGNKEY=' $$HOME/.gnupg-autogen.rc|cut -d= -f2);\
          if [ -z "$$x" ]; then \
             echo "error: RELEASE_SIGNKEY missing in ~/.gnupg-autogen.rc">&2; \
             exit 2;\
          fi;\
	  mysignkey="$$x";\
	  files1="$(RELEASE_NAME).tar.bz2 \
	          $(RELEASE_NAME).tar.gz" ; \
	  files2="$(RELEASE_NAME).tar.bz2.sig \
	          $(RELEASE_NAME).tar.gz.sig  \
		  $(RELEASE_NAME).swdb \
		  $(RELEASE_NAME).buildlog" ;\
	  echo "/* Signing the source tarball ..." ;\
	  gpg -sbu $$mysignkey $(RELEASE_NAME).tar.bz2 ;\
	  gpg -sbu $$mysignkey $(RELEASE_NAME).tar.gz  ;\
	  cat $(RELEASE_NAME).swdb >swdb.snippet;\
	  echo >>swdb.snippet ;\
	  sha1sum $${files1} >>swdb.snippet ;\
          cat "../$(RELEASE_NAME).buildlog" swdb.snippet \
               | gzip >$(RELEASE_NAME).buildlog ;\
          echo "Copying to archive $$myarchive ..." ;\
	  scp -p $${files1} $${files2} $${myarchive}/ || true;\
	  echo '/*' ;\
	  echo ' * All done; for checksums see dist/swdb.snippet' ;\
	  echo ' */' ;\
         )
