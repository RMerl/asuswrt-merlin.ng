# Makefile.am for libgpg-error documentation
# Copyright (C) 2014 g10 Code GmbH
#
# This file is part of libgpg-error.
#
# libgpg-error is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# libgpg-error is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, see <https://www.gnu.org/licenses/>.


EXTRA_DIST = HACKING errorref.txt

bin_PROGRAMS = yat2m
yat2m_SOURCES = yat2m.c
yat2m_CFLAGS= -DPACKAGE_VERSION="\"$(PACKAGE_VERSION)\""

DISTCLEANFILES = gpgrt.cps yat2m-stamp.tmp yat2m-stamp $(myman_pages)
CLEANFILES = errorref.txt.x

info_TEXINFOS = gpgrt.texi
gpgrt_TEXINFOS = lgpl.texi gpl.texi

YAT2M_OPTIONS = -I $(srcdir) \
	--release "Libgpg-error @PACKAGE_VERSION@" --source "GnuPG"

myman_sources = gpgrt.texi
if INSTALL_GPG_ERROR_CONFIG
myman_pages   = gpg-error-config.1 gpgrt-config.1
else
myman_pages   = gpgrt-config.1
endif

man_MANS = $(myman_pages)

if CROSS_COMPILING
YAT2M_CMD = ./yat2m-for-build$(EXEEXT_FOR_BUILD)
YAT2M_DEP = yat2m-for-build$(EXEEXT_FOR_BUILD)
CLEANFILES += yat2m-for-build$(EXEEXT_FOR_BUILD)

yat2m-for-build$(EXEEXT_FOR_BUILD): yat2m.c
	$(CC_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(LDFLAGS_FOR_BUILD) \
	$(CPPFLAGS_FOR_BUILD) -o $@ $(srcdir)/yat2m.c
else
YAT2M_CMD = ./yat2m$(EXEEXT)
YAT2M_DEP = yat2m$(EXEEXT)
endif

yat2m-stamp: $(myman_sources) $(srcdir)/version.texi
	@rm -f yat2m-stamp.tmp
	@touch yat2m-stamp.tmp
	for file in $(myman_sources) ; do \
              $(YAT2M_CMD) $(YAT2M_OPTIONS) --store \
	          `test -f '$$file' || echo '$(srcdir)/'`$$file ; done
	@mv -f yat2m-stamp.tmp $@

yat2m-stamp: $(YAT2M_DEP)

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

# Make sure that gpgrt.texi is touched if any other texi file has
# been modified.  This is required so that the version.texi magic
# updates the release date.
gpgrt.texi : $(gpgrt_TEXINFOS)
	touch $(srcdir)/gpgrt.texi

errorref.txt.x : errorref.txt
	sed '/^##/ d' $(srcdir)/errorref.txt >$@
	echo "# Installed by $(PACKAGE_NAME) $(PACKAGE_VERSION)" >>$@

install-data-local: errorref.txt.x
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(INSTALL_DATA) errorref.txt.x $(DESTDIR)$(pkgdatadir)/errorref.txt

uninstall-local:
	-@rm $(DESTDIR)$(pkgdatadir)/errorref.txt
