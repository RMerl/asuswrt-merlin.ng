## Process this file with automake to create Makefile.in
# Copyright (C) 2002 Free Software Foundation, Inc.
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
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

EXTRA_DIST = README.apichanges HACKING DCO \
	     libgcrypt-modules.png fips-fsm.png \
             libgcrypt-modules.pdf fips-fsm.pdf \
	     yat2m.c

DISTCLEANFILES = gcrypt.cps yat2m-stamp.tmp yat2m-stamp $(myman_pages)
CLEANFILES = yat2m

BUILT_SOURCES = libgcrypt-modules.png fips-fsm.png \
                libgcrypt-modules.pdf fips-fsm.pdf

info_TEXINFOS = gcrypt.texi
gcrypt_TEXINFOS = lgpl.texi gpl.texi libgcrypt-modules.fig fips-fsm.fig

YAT2M_OPTIONS = -I $(srcdir) \
	--release "Libgcrypt @PACKAGE_VERSION@" --source "Libgcrypt"

myman_sources = gcrypt.texi
myman_pages   = hmac256.1

man_MANS = $(myman_pages)

yat2m: yat2m.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(LDFLAGS_FOR_BUILD) \
	    $(CPPFLAGS_FOR_BUILD) -o $@ $(srcdir)/yat2m.c

.fig.png:
	fig2dev -L png `test -f '$<' || echo '$(srcdir)/'`$< $@

.fig.jpg:
	fig2dev -L jpg `test -f '$<' || echo '$(srcdir)/'`$< $@

.fig.eps:
	fig2dev -L eps `test -f '$<' || echo '$(srcdir)/'`$< $@

.fig.pdf:
	fig2dev -L pdf `test -f '$<' || echo '$(srcdir)/'`$< $@

yat2m-stamp: $(myman_sources)
	@rm -f yat2m-stamp.tmp
	@touch yat2m-stamp.tmp
	for file in $(myman_sources) ; do \
              ./yat2m $(YAT2M_OPTIONS) --store \
	          `test -f '$$file' || echo '$(srcdir)/'`$$file ; done
	@mv -f yat2m-stamp.tmp $@

yat2m-stamp: yat2m

$(myman_pages) : yat2m-stamp
	@if test -f $@; then :; else \
            trap 'rm -rf yat2m-stamp yat2m-lock' 1 2 13 15; \
               if mkdir yat2m-lock 2>/dev/null; then \
                 rm -f yat2m-stamp; \
                 $(MAKE) $(AM_MAKEFLAGS) yat2m-stamp; \
                 rmdir yat2m-lock; \
               else \
                 while test -d yat2m-lock; do sleep 1; done; \
                 test -f yat2m-stamp; exit $$?; \
               fi; \
             fi


# Make sure that gcrypt.texi is touched if any other source file has
# been modified.  This is required so that the version.texi magic
# updates the release date.
gcrypt.texi : $(gcrypt_TEXINFOS)
	touch $(srcdir)/gcrypt.texi

online: gcrypt.html gcrypt.pdf gcrypt.info
	set -e; \
	echo "Uploading current manuals to www.gnupg.org ..."; \
	cp libgcrypt-modules.png gcrypt.html/; \
	cp fips-fsm.png gcrypt.html/; \
        user=werner ; dashdevel="" ; \
        if echo "@PACKAGE_VERSION@" | grep -- "-svn" >/dev/null; then \
	  dashdevel="-devel" ; \
	  cp gcrypt.pdf gcrypt.html/; \
	  cp gcrypt.info gcrypt.html/; \
	else \
          rsync -v gcrypt.pdf gcrypt.info \
               $${user}@trithemius.gnupg.org:webspace/manuals/ ; \
        fi ; \
	cd gcrypt.html ; \
        rsync -vr --exclude='.svn' .  \
	  $${user}@trithemius.gnupg.org:webspace/manuals/gcrypt$${dashdevel}/
