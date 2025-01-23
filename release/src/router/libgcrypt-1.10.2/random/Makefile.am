# Makefile for cipher modules
# Copyright (C) 2008 Free Software Foundation, Inc.
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

# Process this file with automake to produce Makefile.in

# Need to include ../src in addition to top_srcdir because gcrypt.h is
# a built header.
AM_CPPFLAGS = -I../src -I$(top_srcdir)/src
AM_CFLAGS = $(GPG_ERROR_CFLAGS)

noinst_LTLIBRARIES = librandom.la

GCRYPT_MODULES = @GCRYPT_RANDOM@

librandom_la_DEPENDENCIES = $(GCRYPT_MODULES)
librandom_la_LIBADD = $(GCRYPT_MODULES)

librandom_la_SOURCES = \
random.c random.h \
rand-internal.h \
random-csprng.c \
random-drbg.c \
random-system.c \
rndjent.c \
rndhw.c

EXTRA_librandom_la_SOURCES = \
rndgetentropy.c \
rndoldlinux.c \
rndegd.c \
rndunix.c \
rndw32.c  \
rndw32ce.c \
jitterentropy-gcd.c jitterentropy-gcd.h \
jitterentropy-health.c jitterentropy-health.h \
jitterentropy-noise.c jitterentropy-noise.h \
jitterentropy-sha3.c jitterentropy-sha3.h \
jitterentropy-timer.c jitterentropy-timer.h \
jitterentropy-base.h \
jitterentropy-base.c jitterentropy.h jitterentropy-base-user.h

# The rndjent module needs to be compiled without optimization.  */
if ENABLE_O_FLAG_MUNGING
o_flag_munging = sed -e 's/-O\([1-9sgz][1-9sgz]*\)/-O0/g' -e 's/-Ofast/-O0/g'
else
o_flag_munging = cat
endif

rndjent.o: $(srcdir)/rndjent.c jitterentropy-base-user.h \
           $(srcdir)/jitterentropy-gcd.c $(srcdir)/jitterentropy-gcd.h \
           $(srcdir)/jitterentropy-health.c $(srcdir)/jitterentropy-health.h \
           $(srcdir)/jitterentropy-noise.c $(srcdir)/jitterentropy-noise.h \
           $(srcdir)/jitterentropy-sha3.c $(srcdir)/jitterentropy-sha3.h \
           $(srcdir)/jitterentropy-timer.c $(srcdir)/jitterentropy-timer.h \
           $(srcdir)/jitterentropy-base.c $(srcdir)/jitterentropy.h
	`echo $(COMPILE) -c $(srcdir)/rndjent.c | $(o_flag_munging) `

rndjent.lo: $(srcdir)/rndjent.c jitterentropy-base-user.h \
           $(srcdir)/jitterentropy-gcd.c $(srcdir)/jitterentropy-gcd.h \
           $(srcdir)/jitterentropy-health.c $(srcdir)/jitterentropy-health.h \
           $(srcdir)/jitterentropy-noise.c $(srcdir)/jitterentropy-noise.h \
           $(srcdir)/jitterentropy-sha3.c $(srcdir)/jitterentropy-sha3.h \
           $(srcdir)/jitterentropy-timer.c $(srcdir)/jitterentropy-timer.h \
            $(srcdir)/jitterentropy-base.c $(srcdir)/jitterentropy.h
	`echo $(LTCOMPILE) -c $(srcdir)/rndjent.c | $(o_flag_munging) `
