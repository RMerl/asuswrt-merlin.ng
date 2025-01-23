# Makefile.am - for gcrypt/src
# Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
#               2006, 2007  Free Software Foundation, Inc.
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
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.


## Process this file with automake to produce Makefile.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgcrypt.pc

EXTRA_DIST = libgcrypt-config.in libgcrypt.m4 libgcrypt.vers \
             gcrypt.h.in libgcrypt.def libgcrypt.pc.in gen-note-integrity.sh

bin_SCRIPTS = libgcrypt-config
m4datadir = $(datadir)/aclocal
m4data_DATA = libgcrypt.m4
nodist_include_HEADERS = gcrypt.h

lib_LTLIBRARIES = libgcrypt.la
bin_PROGRAMS = dumpsexp hmac256 mpicalc
if ENABLE_RANDOM_DAEMON
sbin_PROGRAMS = gcryptrnd
bin_PROGRAMS += getrandom
endif ENABLE_RANDOM_DAEMON

# Depending on the architecture some targets require libgpg-error.
if HAVE_W32CE_SYSTEM
arch_gpg_error_cflags = $(GPG_ERROR_CFLAGS)
arch_gpg_error_libs   = $(GPG_ERROR_LIBS)
else
arch_gpg_error_cflags =
arch_gpg_error_libs   =
endif

AM_CFLAGS = $(GPG_ERROR_CFLAGS)
AM_CCASFLAGS = $(NOEXECSTACK_FLAGS)

if HAVE_LD_VERSION_SCRIPT
  libgcrypt_version_script_cmd = -Wl,--version-script=$(srcdir)/libgcrypt.vers
else
  libgcrypt_version_script_cmd =
endif

libgcrypt_la_CFLAGS = $(GPG_ERROR_CFLAGS) @DEF_HMAC_BINARY_CHECK@
libgcrypt_la_SOURCES = \
        gcrypt-int.h g10lib.h visibility.c visibility.h types.h \
	gcrypt-testapi.h cipher.h cipher-proto.h \
	misc.c global.c sexp.c hwfeatures.c hwf-common.h \
	stdmem.c stdmem.h secmem.c secmem.h \
	mpi.h missing-string.c fips.c \
	context.c context.h \
	ec-context.h

EXTRA_libgcrypt_la_SOURCES = hwf-x86.c hwf-arm.c hwf-ppc.c hwf-s390x.c
gcrypt_hwf_modules = @GCRYPT_HWF_MODULES@


if HAVE_W32_SYSTEM

RCCOMPILE = $(RC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(libgcrypt_la_CPPFLAGS) $(AM_CPPFLAGS) $(CPPFLAGS)
LTRCCOMPILE = $(LIBTOOL) --mode=compile --tag=RC $(RCCOMPILE)

SUFFIXES = .rc .lo

.rc.lo:
	$(LTRCCOMPILE) -i "$<" -o "$@"

gcrypt_res = versioninfo.lo
no_undefined = -no-undefined
export_symbols = -export-symbols $(srcdir)/libgcrypt.def
extra_ltoptions = -XCClinker -static-libgcc

install-def-file:
	-$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL) $(srcdir)/libgcrypt.def $(DESTDIR)$(libdir)/libgcrypt.def

uninstall-def-file:
	-rm $(DESTDIR)$(libdir)/libgcrypt.def

gcrypt_deps = $(gcrypt_res) libgcrypt.def

else !HAVE_W32_SYSTEM

gcrypt_res =
gcrypt_res_ldflag =
no_undefined =
export_symbols =
extra_ltoptions =
install-def-file:
uninstall-def-file:

gcrypt_deps =

endif !HAVE_W32_SYSTEM


libgcrypt_la_LDFLAGS = $(no_undefined) $(export_symbols) $(extra_ltoptions) \
	$(libgcrypt_version_script_cmd) -version-info \
	@LIBGCRYPT_LT_CURRENT@:@LIBGCRYPT_LT_REVISION@:@LIBGCRYPT_LT_AGE@
libgcrypt_la_DEPENDENCIES = \
         $(gcrypt_hwf_modules) \
	../cipher/libcipher.la \
	../random/librandom.la \
	../mpi/libmpi.la \
	../compat/libcompat.la \
	$(srcdir)/libgcrypt.vers $(gcrypt_deps)
libgcrypt_la_LIBADD = $(gcrypt_res) \
        $(gcrypt_hwf_modules) \
	../cipher/libcipher.la \
	../random/librandom.la \
	../mpi/libmpi.la \
	../compat/libcompat.la $(DL_LIBS) $(GPG_ERROR_LIBS)


dumpsexp_SOURCES = dumpsexp.c
dumpsexp_CFLAGS = $(arch_gpg_error_cflags)
dumpsexp_LDADD = $(arch_gpg_error_libs)

mpicalc_SOURCES = mpicalc.c
mpicalc_CFLAGS = $(GPG_ERROR_CFLAGS)
mpicalc_LDADD = libgcrypt.la $(GPG_ERROR_LIBS)
EXTRA_mpicalc_DEPENDENCIES = libgcrypt.la.done

hmac256_SOURCES = hmac256.c hmac256.h
hmac256_CFLAGS = -DSTANDALONE @DEF_HMAC_BINARY_CHECK@ \
	$(arch_gpg_error_cflags)
hmac256_LDADD = $(arch_gpg_error_libs)

if ENABLE_RANDOM_DAEMON
gcryptrnd_SOURCES = gcryptrnd.c
gcryptrnd_CFLAGS = $(GPG_ERROR_CFLAGS) $(PTH_CFLAGS)
gcryptrnd_LDADD = libgcrypt.la $(GPG_ERROR_LIBS) $(PTH_LIBS)

getrandom_SOURCES = getrandom.c
endif ENABLE_RANDOM_DAEMON

CLEANFILES = libgcrypt.la.done
if USE_HMAC_BINARY_CHECK
CLEANFILES += libgcrypt.so.hmac

libgcrypt.la.done: libgcrypt.so.hmac
	$(OBJCOPY) --add-section .note.fdo.integrity=libgcrypt.so.hmac \
	  --set-section-flags .note.fdo.integrity=noload,readonly \
	  .libs/libgcrypt.so .libs/libgcrypt.so.new
	mv -f .libs/libgcrypt.so.new .libs/libgcrypt.so.*.*
	@touch libgcrypt.la.done

libgcrypt.so.hmac: hmac256 libgcrypt.la
	ECHO_N=$(ECHO_N) READELF=$(READELF) AWK=$(AWK) \
	$(srcdir)/gen-note-integrity.sh > $@
else !USE_HMAC_BINARY_CHECK
libgcrypt.la.done: libgcrypt.la
	@touch libgcrypt.la.done
endif !USE_HMAC_BINARY_CHECK

install-data-local: install-def-file

uninstall-local: uninstall-def-file
